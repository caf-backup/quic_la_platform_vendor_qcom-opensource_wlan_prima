/**=============================================================================
*     wlan_hdd_early_suspend.c
*
*     \brief      power management functions
*
*     Description
*                 Copyright 2009 (c) Qualcomm, Incorporated.
*                 All Rights Reserved.
*                 Qualcomm Confidential and Proprietary.
*
==============================================================================**/
/* $HEADER$ */

/**-----------------------------------------------------------------------------
*   Include files
* ----------------------------------------------------------------------------*/
#ifdef CONFIG_HAS_EARLYSUSPEND

#include <linux/pm.h>
#include <linux/wait.h>
#include <linux/earlysuspend.h>
#include <wlan_hdd_includes.h>
#include <wlan_qct_driver.h>
#endif

#include "halTypes.h"
#include "sme_Api.h"
#include <vos_api.h>
#include "vos_power.h"
#include <vos_sched.h>
#include <macInitApi.h>
#include <wlan_qct_sal.h>
#include <wlan_qct_bal.h>
#include <wlan_qct_sys.h>
#include <wlan_btc_svc.h>
#include <wlan_nlink_common.h>

/**-----------------------------------------------------------------------------
*   Preprocessor definitions and constants
* ----------------------------------------------------------------------------*/

/**-----------------------------------------------------------------------------
*   Type declarations
* ----------------------------------------------------------------------------*/

/**-----------------------------------------------------------------------------
*   Function and variables declarations
* ----------------------------------------------------------------------------*/
#include "wlan_hdd_power.h"

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend wlan_early_suspend;
#endif
static eHalStatus g_full_pwr_status;
static eHalStatus g_standby_status;

extern VOS_STATUS hdd_post_voss_start_config(hdd_adapter_t* pAdapter);
extern VOS_STATUS vos_chipExitDeepSleepVREGHandler(
   vos_call_status_type* status,
   vos_power_cb_type callback,
   v_PVOID_t user_data);
extern void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);

//Callback invoked by PMC to report status of standby request
void hdd_suspend_standby_cbk (void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Standby status = %d", __func__, status);
	g_standby_status = status; 
   complete(&pAdapter->standby_comp_var);
}

//Callback invoked by PMC to report status of full power request
void hdd_suspend_full_pwr_callback(void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Full Power status = %d", __func__, status);
	g_full_pwr_status = status;
   complete(&pAdapter->full_pwr_comp_var);
}

eHalStatus hdd_exit_standby(hdd_adapter_t* pAdapter)
{  
    eHalStatus status = VOS_STATUS_SUCCESS;

    hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from standby",__func__);
    init_completion(&pAdapter->full_pwr_comp_var);

    status = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, pAdapter,
      eSME_FULL_PWR_NEEDED_BY_HDD);

    if (status == eHAL_STATUS_FAILURE) {

        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: sme_RequestFullPower Failed!!!\n",__func__);
    } 
    else {
        status = eHAL_STATUS_SUCCESS;
        pAdapter->hdd_ps_state = eHDD_SUSPEND_NONE;
    }

    //No blocking to reduce latency. No other device should be depending on WLAN
    //to finish resume and WLAN won't be instantly on after resume
    return status;
}


//Helper routine to put the chip into standby
VOS_STATUS hdd_enter_standby(hdd_adapter_t* pAdapter)
{
   eHalStatus halStatus;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
         
   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode on its own during suspend sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Note we do not disable queues unnecessarily. Queues should already be disabled
   //if STA is disconnected or the queue will be disabled as and when disconnect
   //happens because of standby procedure.
   
   //Ensure that device is in full power first. There is scope for optimization
   //here especially in scenarios where PMC is already in IMPS or REQUEST_IMPS.
   //Core s/w needs to be optimized to handle this. Until then we request full
   //power before issuing request for standby.
   init_completion(&pAdapter->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, 
       pAdapter, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_FULL_PWR));
      if(g_full_pwr_status != eHAL_STATUS_SUCCESS)
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestFullPower failed",__func__);
         VOS_ASSERT(0);
         vosStatus = VOS_STATUS_E_FAILURE;
         goto failure;
      }
   }
   else if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestFullPower failed - status %d",
         __func__, halStatus);
      VOS_ASSERT(0);
      vosStatus = VOS_STATUS_E_FAILURE;
      goto failure;
   }

   //Request standby. Standby will cause the STA to disassociate first. TX queues
   //will be disabled (by HDD) when STA disconnects. You do not want to disable TX
   //queues here. Also do not assert if the failure code is eHAL_STATUS_PMC_NOT_NOW as PMC
   //will send this failure code in case of concurrent sessions. Power Save cannot be supported
   //when there are concurrent sessions.
   init_completion(&pAdapter->standby_comp_var);
   g_standby_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestStandby(pAdapter->hHal, hdd_suspend_standby_cbk, pAdapter);

   if (halStatus == eHAL_STATUS_PMC_PENDING) 
   {
      //Wait till WLAN device enters standby mode
      wait_for_completion_timeout(&pAdapter->standby_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_STANDBY));
      if (g_standby_status != eHAL_STATUS_SUCCESS && g_standby_status != eHAL_STATUS_PMC_NOT_NOW)
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestStandby failed",__func__);
         VOS_ASSERT(0);
         vosStatus = VOS_STATUS_E_FAILURE;
         goto failure;
      }
   }
   else if (halStatus != eHAL_STATUS_SUCCESS && halStatus != eHAL_STATUS_PMC_NOT_NOW) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestStandby failed - status %d",
         __func__, halStatus);
      VOS_ASSERT(0);
      vosStatus = VOS_STATUS_E_FAILURE;
      goto failure;
   }
   
   pAdapter->hdd_ps_state = eHDD_SUSPEND_STANDBY;

failure:
   //Restore IMPS config
   if(pAdapter->cfg_ini->fIsImpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);

   //Restore BMPS config
   if(pAdapter->cfg_ini->fIsBmpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   return vosStatus;
}


//Helper routine for Deep sleep entry
VOS_STATUS hdd_enter_deep_sleep(hdd_adapter_t* pAdapter)
{
   eHalStatus halStatus;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_call_status_type callType;

   //Stop the Interface TX queue.
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Disable IMPS,BMPS as we do not want the device to enter any power
   //save mode on it own during suspend sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Ensure that device is in full power as we will touch H/W during vos_Stop
   init_completion(&pAdapter->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, 
       pAdapter, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_FULL_PWR));
      if(g_full_pwr_status != eHAL_STATUS_SUCCESS){
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestFullPower failed",__func__);
         VOS_ASSERT(0);
      }
   }
   else if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Request for Full Power failed",__func__);
      VOS_ASSERT(0);
   }

   //Issue a disconnect. This is required to inform the supplicant that
   //STA is getting disassociated and for GUI to be updated properly
   init_completion(&pAdapter->disconnect_comp_var);
   halStatus = sme_RoamDisconnect(pAdapter->hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);

   //Success implies disconnect command got queued up successfully
   if(halStatus == eHAL_STATUS_SUCCESS)
   {
      //Block on a completion variable. Can't wait forever though.
      wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
   }

   halStatus = sme_CloseSession(pAdapter->hHal, pAdapter->sessionId);
   if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Unable to close session with sessionId %d",__func__, pAdapter->sessionId);
   }

   //None of the steps should fail after this. Continue even in case of failure
   vosStatus = vos_stop( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANBAL_SuspendChip( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANSAL_Stop(pAdapter->pvosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipVoteOffBBAnalogSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipVoteOffRFSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   pAdapter->hdd_ps_state = eHDD_SUSPEND_DEEP_SLEEP;

   //Restore IMPS config
   if(pAdapter->cfg_ini->fIsImpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);

   //Restore BMPS config
   if(pAdapter->cfg_ini->fIsBmpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   return vosStatus;
}

VOS_STATUS hdd_exit_deep_sleep(hdd_adapter_t* pAdapter)
{
   vos_call_status_type callType;
   VOS_STATUS vosStatus;
   eHalStatus halStatus;

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipVoteOnRFSupply",__func__);
   vosStatus = vos_chipVoteOnRFSupply(&callType, NULL, NULL);

   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_chipVoteOnRFSupply",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipDeAssertDeepSleep",__func__);
   vosStatus = vos_chipDeAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_chipDeAssertDeepSleep",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipExitDeepSleepVREGHandler",__func__);
   vosStatus = vos_chipExitDeepSleepVREGHandler( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_chipExitDeepSleepVREGHandler",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANSAL_SDIOReInit",__func__);
   vosStatus = WLANSAL_SDIOReInit( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in WLANSAL_SDIOReInit",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANSAL_Start",__func__);
   vosStatus = WLANSAL_Start(pAdapter->pvosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to start SAL",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
      "%s: calling WLANBAL_ResumeChip",__func__);
   vosStatus = WLANBAL_ResumeChip( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in WLANBAL_ResumeChip",__func__);
      goto err_sal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
      "%s: calling hdd_set_sme_config",__func__);
   vosStatus = hdd_set_sme_config( pAdapter );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in hdd_set_sme_config",__func__);
      goto err_sal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_start",__func__);
   vosStatus = vos_start( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_start",__func__);
      goto err_sal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling hdd_post_voss_start_config",__func__);
   vosStatus = hdd_post_voss_start_config( pAdapter );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in hdd_post_voss_start_config",__func__);
      goto err_voss_stop;
   }


   //Open a SME session for future operation
   halStatus = sme_OpenSession( pAdapter->hHal, hdd_smeRoamCallback, pAdapter,
                                (tANI_U8 *)&pAdapter->macAddressCurrent, &pAdapter->sessionId );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"sme_OpenSession() failed with status code %08d [x%08lx]",
                    halStatus, halStatus );
      goto err_voss_stop;

   }

   pAdapter->hdd_ps_state = eHDD_SUSPEND_NONE;

   //Trigger the initial scan
   hdd_wlan_initial_scan(pAdapter);

   return VOS_STATUS_SUCCESS;

err_voss_stop:
   vos_stop(pAdapter->pvosContext);
err_sal_stop:
   WLANSAL_Stop(pAdapter->pvosContext);
err_deep_sleep:
   return VOS_STATUS_E_FAILURE;

}
#ifdef CONFIG_HAS_EARLYSUSPEND
//Suspend routine registered with Android OS
void hdd_suspend_wlan(struct early_suspend *wlan_suspend)
{
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;

   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being suspended by Android OS",__func__);

   //Get the global VOSS context.
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if(!pVosContext) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return;
   }

   //Get the HDD context.
   pAdapter = (hdd_adapter_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
   if(!pAdapter) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return;
   }

   if(pAdapter->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_STANDBY) 
   {
      //Execute standby procedure. Executing standby procedure will cause the STA to
      //disassociate first and then the chip will be put into standby.
      hdd_enter_standby(pAdapter);
   }
   else if(pAdapter->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_DEEP_SLEEP) {
      //Execute deep sleep procedure
      hdd_enter_deep_sleep(pAdapter);
   }
   else {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported suspend mapping %d",
         __func__, pAdapter->cfg_ini->nEnableSuspend);
   }

   return;
}

void hdd_resume_wlan(struct early_suspend *wlan_suspend)
{
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;
   
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed by Android OS",__func__);

   //Get the global VOSS context.
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if(!pVosContext) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return;
   }

   //Get the HDD context.
   pAdapter = (hdd_adapter_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );
   if(!pAdapter) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return;
   }

   if(pAdapter->hdd_ps_state == eHDD_SUSPEND_STANDBY) 
   {
       hdd_exit_standby(pAdapter);
   } 
   else if(pAdapter->hdd_ps_state == eHDD_SUSPEND_DEEP_SLEEP) 
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from deep sleep",__func__);
      hdd_exit_deep_sleep(pAdapter);
   }
   else {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unknown WLAN PS state during resume %d",
         __func__, pAdapter->hdd_ps_state);
   }

   pAdapter->hdd_ps_state = eHDD_SUSPEND_NONE;

   return;
}

VOS_STATUS hdd_wlan_reset(void) 
{
   VOS_STATUS vosStatus;
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;
   pVosSchedContext vosSchedContext = NULL;
   pVosWatchdogContext vosWatchdogContext =NULL;
   vos_call_status_type callType;
   union iwreq_data wrqu;
   eHalStatus halStatus;
   v_BOOL_t sendDisconnect;
	
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: WLAN being reset",__func__);

   //Get the global VOSS context.
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if(!pVosContext) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   //Get the HDD context.
   pAdapter = (hdd_adapter_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext);
   if(!pAdapter) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return VOS_STATUS_E_FAILURE;
   }
	
   //Stop the Interface TX queue.
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Record whether STA is associated
   sendDisconnect = hdd_connIsConnected(pAdapter) ? VOS_TRUE : VOS_FALSE;
   
   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode on its own during reset sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Ensure that device is in full power first.
   init_completion(&pAdapter->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback,
       pAdapter, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var,
         msecs_to_jiffies(WLAN_WAIT_TIME_FULL_PWR));
      if(g_full_pwr_status != eHAL_STATUS_SUCCESS)
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestFullPower failed",__func__);
         VOS_ASSERT(0);
      }
   }
   else if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: sme_RequestFullPower failed - status %d",
         __func__, halStatus);
      VOS_ASSERT(0);
   }

   //Kill all the threads first. We do not want any messages
   //to be a processed any more and the best way to ensure that
   //is to terminate the threads gracefully.
   vosSchedContext = get_vos_sched_ctxt();
   vosWatchdogContext = get_vos_watchdog_ctxt();
	
   set_bit(MC_SHUTDOWN_EVENT_MASK, &vosSchedContext->mcEventFlag);
   set_bit(MC_POST_EVENT_MASK, &vosSchedContext->mcEventFlag);

   set_bit(TX_SHUTDOWN_EVENT_MASK, &vosSchedContext->txEventFlag);
   set_bit(TX_POST_EVENT_MASK, &vosSchedContext->txEventFlag);

   wake_up_interruptible(&vosSchedContext->mcWaitQueue);
   wake_up_interruptible(&vosSchedContext->txWaitQueue);

   //Wait for MC and TX to exit
   wait_for_completion_interruptible(&vosSchedContext->McShutdown);
   wait_for_completion_interruptible(&vosSchedContext->TxShutdown);

   //Stop SME - Cannot invoke vos_stop as vos_stop relies
   //on threads being running to process the SYS Stop
   vosStatus = sme_Stop( pAdapter->hHal, TRUE );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Stop MAC (PE and HAL)
   vosStatus = macStop( pAdapter->hHal, HAL_STOP_TYPE_SYS_RESET);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Stop TL
   vosStatus = WLANTL_Stop( pVosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Stop BAL
   vosStatus = WLANBAL_Stop( pVosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Suspend Chip
   vosStatus = WLANBAL_SuspendChip( pVosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Stop SAL
   vosStatus = WLANSAL_Stop(pVosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Enter Deep Sleep
   vosStatus = vos_chipAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipVoteOffBBAnalogSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipVoteOffRFSupply(&callType, NULL, NULL);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Clean up HDD TX/RX data path
   hdd_deinit_tx_rx(pAdapter);
   hdd_wmm_close(pAdapter);

   //Clean up message queues of TX and MC thread
   vos_sched_flush_mc_mqs(vosSchedContext);
   vos_sched_flush_tx_mqs(vosSchedContext);

   //Deinit all the TX and MC queues
   vos_sched_deinit_mqs(vosSchedContext);

   //Close VOSS -- this is not really necessary. But we are being
   //safe and restaring with new clean memory as well. Since this
   //scenario should be a rare occurence, we need not worry about
   //any latency or performance hits.
   vosStatus = vos_close(pVosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Open VOSS
   vosStatus = vos_open( &pVosContext, 0);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Reinit the HAL context in HDD Adapter
   pAdapter->hHal = (tHalHandle)vos_get_context(
      VOS_MODULE_ID_HAL, pVosContext );

   //Set the Connection State to Not Connected
   pAdapter->conn_info.connState = eConnectionState_NotConnected;

   //Set the default operation channel
   pAdapter->conn_info.operationChannel = pAdapter->cfg_ini->OperatingChannel;

   /* Make the default Auth Type as OPEN*/
   pAdapter->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

   vosStatus = hdd_init_tx_rx(pAdapter);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Initialize the WMM module
   vosStatus = hdd_wmm_init(pAdapter);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   
   vosStatus = hdd_exit_deep_sleep(pAdapter);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Indicate disconnect event to supplicant if associated previously
   if(sendDisconnect) {
      memset(&wrqu, '\0', sizeof(wrqu));
      wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	   memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
      wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
   }

   //Trigger replay of BTC events
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);

   return VOS_STATUS_SUCCESS;
	
}

void register_wlan_suspend(void)
{
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Register WLAN suspend/resume "
            "callbacks",__func__);
   wlan_early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
   wlan_early_suspend.suspend = hdd_suspend_wlan;
   wlan_early_suspend.resume = hdd_resume_wlan;
   register_early_suspend(&wlan_early_suspend);
}

void unregister_wlan_suspend(void)
{
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unregister WLAN suspend/resume "
            "callbacks",__func__);
   unregister_early_suspend(&wlan_early_suspend);
}

#endif
