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
#include <linux/wakelock.h>
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
#include <wlan_sal_misc.h>
#include <libra_sdioif.h>
#include <wlan_nlink_srv.h>
#include <wlan_hdd_misc.h>

#ifdef WLAN_SOFTAP_FEATURE
#include <wlan_hdd_hostapd.h>
#endif

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
static struct wake_lock wlan_wake_lock;
#endif
static eHalStatus g_full_pwr_status;
static eHalStatus g_standby_status;

extern VOS_STATUS hdd_post_voss_start_config(hdd_adapter_t* pAdapter);
extern VOS_STATUS vos_chipExitDeepSleepVREGHandler(
   vos_call_status_type* status,
   vos_power_cb_type callback,
   v_PVOID_t user_data);
extern void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);
void unregister_wlan_suspend(void);

extern struct notifier_block hdd_netdev_notifier;
#ifdef WLAN_SOFTAP_FEATURE
extern VOS_CON_MODE hdd_get_conparam ( void );
#endif

//Callback invoked by PMC to report status of standby request
void hdd_suspend_standby_cbk (void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Standby status = %d", __func__, status);
   g_standby_status = status; 

   if(eHAL_STATUS_SUCCESS == status)
   {
      pAdapter->hdd_ps_state = eHDD_SUSPEND_STANDBY;
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestStandby failed",__func__);
   }

   complete(&pAdapter->standby_comp_var);
}

//Callback invoked by PMC to report status of full power request
void hdd_suspend_full_pwr_callback(void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Full Power status = %d", __func__, status);
   g_full_pwr_status = status;

   if(eHAL_STATUS_SUCCESS == status)
   {
      pAdapter->hdd_ps_state = eHDD_SUSPEND_NONE;
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed",__func__);
   }

   complete(&pAdapter->full_pwr_comp_var);
}

eHalStatus hdd_exit_standby(hdd_adapter_t* pAdapter)
{  
   eHalStatus status = eHAL_STATUS_SUCCESS;

   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from standby",__func__);
   INIT_COMPLETION(pAdapter->full_pwr_comp_var);

   g_full_pwr_status = eHAL_STATUS_FAILURE;
    
   status = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, pAdapter,
                                  eSME_FULL_PWR_NEEDED_BY_HDD);

   if(status == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_FULL_PWR));
      status = g_full_pwr_status;
      if(g_full_pwr_status != eHAL_STATUS_SUCCESS)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed",__func__);
         VOS_ASSERT(0);
          goto failure;
      }
    }
    else if(status != eHAL_STATUS_SUCCESS)
    {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed - status %d",
         __func__, status);
      VOS_ASSERT(0);
      goto failure;
    }
    else
      pAdapter->hdd_ps_state = eHDD_SUSPEND_NONE;

failure:
    //No blocking to reduce latency. No other device should be depending on WLAN
    //to finish resume and WLAN won't be instantly on after resume
    return status;
}


//Helper routine to put the chip into standby
VOS_STATUS hdd_enter_standby(hdd_adapter_t* pAdapter)
{
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
         
   netif_tx_stop_all_queues(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

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
   INIT_COMPLETION(pAdapter->full_pwr_comp_var);
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
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower Failed",__func__);
         VOS_ASSERT(0);
         vosStatus = VOS_STATUS_E_FAILURE;
         goto failure;
      }
   }
   else if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed - status %d",
         __func__, halStatus);
      VOS_ASSERT(0);
      vosStatus = VOS_STATUS_E_FAILURE;
      goto failure;
   }

   if(pAdapter->hdd_mcastbcast_filter_set == TRUE) {
         hdd_conf_mcastbcast_filter(pAdapter, FALSE);
         pAdapter->hdd_mcastbcast_filter_set = FALSE;
   }

   //Request standby. Standby will cause the STA to disassociate first. TX queues
   //will be disabled (by HDD) when STA disconnects. You do not want to disable TX
   //queues here. Also do not assert if the failure code is eHAL_STATUS_PMC_NOT_NOW as PMC
   //will send this failure code in case of concurrent sessions. Power Save cannot be supported
   //when there are concurrent sessions.
  INIT_COMPLETION(pAdapter->standby_comp_var);
   g_standby_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestStandby(pAdapter->hHal, hdd_suspend_standby_cbk, pAdapter);

   if (halStatus == eHAL_STATUS_PMC_PENDING) 
   {
      //Wait till WLAN device enters standby mode
      wait_for_completion_timeout(&pAdapter->standby_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_STANDBY));
      if (g_standby_status != eHAL_STATUS_SUCCESS && g_standby_status != eHAL_STATUS_PMC_NOT_NOW)
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestStandby failed",__func__);
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
   else
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
   netif_tx_stop_all_queues(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Disable IMPS,BMPS as we do not want the device to enter any power
   //save mode on it own during suspend sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Ensure that device is in full power as we will touch H/W during vos_Stop
  INIT_COMPLETION(pAdapter->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, 
       pAdapter, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_FULL_PWR));
      if(g_full_pwr_status != eHAL_STATUS_SUCCESS){
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed",__func__);
         VOS_ASSERT(0);
      }
   }
   else if(halStatus != eHAL_STATUS_SUCCESS)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Request for Full Power failed",__func__);
      VOS_ASSERT(0);
   }

   //Issue a disconnect. This is required to inform the supplicant that
   //STA is getting disassociated and for GUI to be updated properly
   INIT_COMPLETION(pAdapter->disconnect_comp_var);
   halStatus = sme_RoamDisconnect(pAdapter->hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);

   //Success implies disconnect command got queued up successfully
   if(halStatus == eHAL_STATUS_SUCCESS)
   {
      //Block on a completion variable. Can't wait forever though.
      wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
   }


   //None of the steps should fail after this. Continue even in case of failure
   vosStatus = vos_stop( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANBAL_Stop( pAdapter->pvosContext );
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
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed in vos_chipVoteOnRFSupply",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipDeAssertDeepSleep",__func__);
   vosStatus = vos_chipDeAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed in vos_chipDeAssertDeepSleep",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipExitDeepSleepVREGHandler",__func__);
   vosStatus = vos_chipExitDeepSleepVREGHandler( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
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

   vosStatus = WLANBAL_Start(pAdapter->pvosContext);

   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start BAL",__func__);
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
      goto err_bal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_start",__func__);
   vosStatus = vos_start( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_start",__func__);
      goto err_bal_stop;
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
err_bal_stop:
   WLANBAL_Stop(pAdapter->pvosContext);
err_sal_stop:
   WLANSAL_Stop(pAdapter->pvosContext);
err_deep_sleep:
   return VOS_STATUS_E_FAILURE;

}

void hdd_conf_mcastbcast_filter(hdd_adapter_t* pAdapter, v_BOOL_t setfilter)
{
    eHalStatus halStatus = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_SME, pAdapter->pvosContext);
    hddLog(VOS_TRACE_LEVEL_INFO,
        "%s: Configuring Mcast/Bacst Filter Setting. setfilter %d", __func__, setfilter);
    if ( pMac ) 
    {
      halStatus = halRxp_configureRxpFilterMcstBcst( pMac, setfilter);
    }
    else
    {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: pMac is initialised to NULL",__func__ );
    }

    if(setfilter && (eHAL_STATUS_SUCCESS == halStatus))
       pAdapter->hdd_mcastbcast_filter_set = TRUE;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static inline void hdd_prevent_suspend(void)
{
    wake_lock(&wlan_wake_lock);
}

static inline void hdd_allow_suspend(void)
{
    wake_unlock(&wlan_wake_lock);
}
//Suspend routine registered with Android OS
void hdd_suspend_wlan(struct early_suspend *wlan_suspend)
{
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;
   struct sdio_func *sdio_func_dev = NULL;
   
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

   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
        /* Our card got removed */
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
        return;
   }

   sd_claim_host(sdio_func_dev);
   
   // Prevent touching the pMac while LOGP reset in progress, we should never get here
   // as the wake lock is already acquired and it would prevent from entering suspend 
   if (pAdapter->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore suspend wlan, LOGP in progress!", __func__);
      sd_release_host(sdio_func_dev);
      return;
   }

#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP
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
#endif
  
   if(pAdapter->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_MCAST_BCAST_FILTER) {
      if(eConnectionState_Associated == pAdapter->conn_info.connState) {
         hdd_conf_mcastbcast_filter(pAdapter, TRUE);
         halPSAppsCpuWakeupState(vos_get_context(VOS_MODULE_ID_SME, pAdapter->pvosContext), FALSE);
      }
   }
   
   sd_release_host(sdio_func_dev);
   
   return;
}

void hdd_resume_wlan(struct early_suspend *wlan_suspend)
{
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;
   struct sdio_func *sdio_func_dev = NULL;
   
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
   
   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
      /* Our card got removed */
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
      return;
   }

   sd_claim_host(sdio_func_dev);
   
   // Prevent touching the pMac while LOGP reset in progress, 
   if (pAdapter->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore resume wlan, LOGP in progress!", __func__);
      sd_release_host(sdio_func_dev);
      return;
   }

#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP
   if(pAdapter->hdd_ps_state == eHDD_SUSPEND_STANDBY) 
   {
       hdd_exit_standby(pAdapter);
   } 
   else if(pAdapter->hdd_ps_state == eHDD_SUSPEND_DEEP_SLEEP) 
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from deep sleep",__func__);
      hdd_exit_deep_sleep(pAdapter);
   }
#endif
	
   if(pAdapter->hdd_mcastbcast_filter_set == TRUE) {
         hdd_conf_mcastbcast_filter(pAdapter, FALSE);
         pAdapter->hdd_mcastbcast_filter_set = FALSE;
         halPSAppsCpuWakeupState(vos_get_context(VOS_MODULE_ID_SME, pAdapter->pvosContext), TRUE);
   }

   sd_release_host(sdio_func_dev);

   return;
}

#define LIBRA_CARD_INSERT_DETECT_MAX_COUNT	5
#define LIBRA_CARD_REMOVE_DETECT_MAX_COUNT	5
VOS_STATUS hdd_wlan_reset(void) 
{
   VOS_STATUS vosStatus;
   union iwreq_data wrqu;
   hdd_adapter_t *pAdapter = NULL;
   v_CONTEXT_t pVosContext = NULL;
   v_BOOL_t sendDisconnect = 0;
	
   pVosSchedContext vosSchedContext = NULL;
   struct sdio_func *sdio_func_dev_new = NULL;
   struct sdio_func *sdio_func_dev_current = NULL;
   unsigned int attempts = 0;

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

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing SAL Stop",__func__);
   //Invoke SAL stop
   vosStatus = WLANSAL_Stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to stop SAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Asserting Deep Sleep",__func__);
   //Assert Deep sleep signal now to put Libra HW in lowest power state
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Power Down Chip",__func__);   
   //Vote off any PMIC voltage supplies
   vos_chipPowerDown(NULL, NULL, NULL);

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Disabling TX queues ",__func__);
   
   //Stop the Interface TX queue.
   netif_tx_stop_all_queues(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Record whether STA is associated
   sendDisconnect = hdd_connIsConnected(pAdapter) ? VOS_TRUE : VOS_FALSE;
   
   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode on its own during reset sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_UAPSD_MODE_POWER_SAVE);

    //Kill all the threads first. We do not want any messages
   //to be a processed any more and the best way to ensure that
   //is to terminate the threads gracefully.
   vosSchedContext = get_vos_sched_ctxt();

   /* Wakeup Mc Thread if Suspended */
   if(TRUE == pAdapter->isMcThreadSuspended){
      complete(&vosSchedContext->ResumeMcEvent);
      pAdapter->isMcThreadSuspended= FALSE;
   }
   	
   /* Wakeup Tx Thread if Suspended */
   if(TRUE == pAdapter->isTxThreadSuspended){
      complete(&vosSchedContext->ResumeTxEvent);
      pAdapter->isTxThreadSuspended= FALSE;
   }
   	
   /* Reset the Suspend Variable */
   pAdapter->isWlanSuspended = FALSE;
   	
   //Wait for MC to exit
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Shutting down MC thread",__func__);
   set_bit(MC_SHUTDOWN_EVENT_MASK, &vosSchedContext->mcEventFlag);
   set_bit(MC_POST_EVENT_MASK, &vosSchedContext->mcEventFlag);
   wake_up_interruptible(&vosSchedContext->mcWaitQueue);
   wait_for_completion_interruptible(&vosSchedContext->McShutdown);

   //Wait for TX to exit
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Shutting down TX thread",__func__);
   set_bit(TX_SHUTDOWN_EVENT_MASK, &vosSchedContext->txEventFlag);
   set_bit(TX_POST_EVENT_MASK, &vosSchedContext->txEventFlag);
   wake_up_interruptible(&vosSchedContext->txWaitQueue);
   wait_for_completion_interruptible(&vosSchedContext->TxShutdown);

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing SME STOP",__func__);
   //Stop SME - Cannot invoke vos_stop as vos_stop relies
   //on threads being running to process the SYS Stop
   vosStatus = sme_Stop( pAdapter->hHal, TRUE );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing MAC STOP",__func__);
   //Stop MAC (PE and HAL)
   vosStatus = macStop( pAdapter->hHal, HAL_STOP_TYPE_SYS_RESET);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing TL STOP",__func__);
   //Stop TL
   vosStatus = WLANTL_Stop( pVosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing BAL STOP",__func__);
   vosStatus = WLANBAL_Stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: Failed to stop BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   //clean up HDD Data Path
   hdd_deinit_tx_rx(pAdapter);
   hdd_wmm_close(pAdapter);
  
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Flush Queues",__func__);
   //Clean up message queues of TX and MC thread
   vos_sched_flush_mc_mqs(vosSchedContext);
   vos_sched_flush_tx_mqs(vosSchedContext);

   //Deinit all the TX and MC queues
   vos_sched_deinit_mqs(vosSchedContext);

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing VOS Close",__func__);

   //Close VOSS
   vos_close(pVosContext);
   
   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing BAL Close",__func__);
   vosStatus = WLANBAL_Close(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, 
          "%s: Failed to close BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
   
   //Get the Current SDIO Func
   sdio_func_dev_current = libra_getsdio_funcdev();   

   if(NULL != sdio_func_dev_current) {
      libra_detect_card_change();
      attempts = 0;
      do {
         msleep(100);
         //Get the SDIO func device
         sdio_func_dev_current = libra_getsdio_funcdev();
         if(NULL == sdio_func_dev_current) {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Card Removed Successfully",__func__);
            break;
         }
         else {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to Remove the Card: Trying Again",__func__);
            attempts++;
         }
      } while (attempts < LIBRA_CARD_REMOVE_DETECT_MAX_COUNT);

      if(LIBRA_CARD_REMOVE_DETECT_MAX_COUNT == attempts) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to Remove the Card: Fatal",__func__);
         goto err_fail;
      }
   }

#ifdef TIMER_MANAGER
   vos_timer_exit();
#endif

#ifdef MEMORY_DEBUG
   vos_mem_clean();
#endif
   //Reinitialize the variable
   attempts = 0;

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Powering Up chip Again",__func__);
   //Power Up Libra WLAN card first if not already powered up
   vosStatus = vos_chipPowerUp(NULL,NULL,NULL);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not Powered Up."
             "exiting", __func__);
      goto err_pwr_fail;
   }

   // Trigger card detect
   libra_detect_card_change();

   //Reinitialize the variable
   attempts = 0;

   do {
      msleep(500);
   
      //Get the SDIO func device
      sdio_func_dev_new = libra_getsdio_funcdev();
      if(sdio_func_dev_new != NULL)
      {
         SET_NETDEV_DEV(pAdapter->dev, &sdio_func_dev_new->dev);
         libra_sdio_setprivdata (sdio_func_dev_new, pAdapter);
         atomic_set(&pAdapter->sdio_claim_count, 0);
         pAdapter->hsdio_func_dev = sdio_func_dev_new;
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
          "%s: Card Detected Successfully %p",__func__, 
          sdio_func_dev_new);
         break;
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
          "%s: Failed to detect card change %p",__func__, 
          sdio_func_dev_new);     
	  attempts++;
      }	
   }while (attempts < LIBRA_CARD_INSERT_DETECT_MAX_COUNT);
   
   if(LIBRA_CARD_INSERT_DETECT_MAX_COUNT == attempts){
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN fail to detect in reset"
             "exiting", __func__);
      goto err_fail;
   }
   
   vosStatus = WLANBAL_Open(pAdapter->pvosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if(!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
        "%s: Failed to open BAL",__func__);
     goto err_fail;
   }

   vosStatus = WLANSAL_Start(pAdapter->pvosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to start SAL",__func__);
      goto err_balclose;
   }

   /* Chip is reset so allow Lower MAC to start accessing WLAN registers. Note HDD is still blocked */
   vos_set_logp_in_progress(VOS_MODULE_ID_HDD, FALSE);

   /* Start BAL */
   vosStatus = WLANBAL_Start(pAdapter->pvosContext);
   
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to start BAL",__func__);
      goto err_salstop;
   }

   // Open VOSS 
   vosStatus = vos_open( &pVosContext, 0);
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_open failed",__func__);
      goto err_balstop;   
   }

   /* Save the hal context in Adapter */
   pAdapter->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_HAL, pVosContext );
      
   if ( NULL == pAdapter->hHal )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HAL context is null",__func__);
      goto err_vosclose;
   }
   
   //Set the Connection State to Not Connected
   pAdapter->conn_info.connState = eConnectionState_NotConnected;

   //Set the default operation channel
   pAdapter->conn_info.operationChannel = pAdapter->cfg_ini->OperatingChannel;

   /* Make the default Auth Type as OPEN*/
   pAdapter->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
  
   // Set the SME configuration parameters...
   vosStatus = hdd_set_sme_config( pAdapter );
   
   if ( VOS_STATUS_SUCCESS != vosStatus )
   {  
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed hdd_set_sme_config",__func__); 
      goto err_vosclose;
   } 
   
   //Initialize the data path module
   vosStatus = hdd_init_tx_rx(pAdapter);
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_init_tx_rx failed", __FUNCTION__);
      goto err_vosclose;
   }  

   //Initialize the WMM module
   vosStatus = hdd_wmm_init(pAdapter);
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_wmm_init failed", __FUNCTION__);
      goto err_vosclose;
   }

   /*Start VOSS which starts up the SME/MAC/HAL modules and everything else
     Note: Firmware image will be read and downloaded inside vos_start API */
   vosStatus = vos_start( pAdapter->pvosContext );
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_start failed",__func__);
      goto err_vosclose;
   }

   vosStatus = hdd_post_voss_start_config( pAdapter );
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_post_voss_start_config failed", 
         __func__);
      goto err_vosstop;
   }

   //Open a SME session for future operation
   vosStatus = sme_OpenSession( pAdapter->hHal, hdd_smeRoamCallback, pAdapter,
                                       (tANI_U8 *)&pAdapter->macAddressCurrent, &pAdapter->sessionId );
   if ( !HAL_STATUS_SUCCESS( vosStatus ) )
   {
       hddLog(VOS_TRACE_LEVEL_FATAL,"sme_OpenSession() failed with status code %08d [x%08lx]",vosStatus, vosStatus );
       goto err_vosstop;
   }

   /* Open the gates for HDD to receive Wext commands */
   pAdapter->isLogpInProgress = FALSE;
   pAdapter->isLinkUpSvcNeeded = FALSE; 
   pAdapter->hdd_mcastbcast_filter_set = FALSE;
   pAdapter->pWextState->mScanPending = FALSE;
   
   // Allow the phone to go to sleep
   hdd_allow_suspend();

   //Stop the Interface TX queue.
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Trigger the initial scan
   hdd_wlan_initial_scan(pAdapter);

   goto success;

err_vosstop:
   vos_stop(pVosContext);

err_vosclose:	
   vos_close(pVosContext ); 

err_balstop:
   wlan_hdd_enable_deepsleep(pAdapter->pvosContext);
   WLANBAL_Stop(pAdapter->pvosContext);
   WLANBAL_SuspendChip(pAdapter->pvosContext);

err_salstop:
   WLANSAL_Stop(pAdapter->pvosContext);

err_balclose:
   WLANBAL_Close(pAdapter->pvosContext);

err_fail:
   //Assert Deep sleep signal now to put Libra HW in lowest power state
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );

   //Vote off any PMIC voltage supplies
   vos_chipPowerDown(NULL, NULL, NULL);

err_pwr_fail:
   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   // Allow the phone to go to sleep
   hdd_allow_suspend();

#ifdef CONFIG_HAS_EARLYSUSPEND
   // unregister suspend/resume callbacks
   if(pAdapter->cfg_ini->nEnableSuspend)
      unregister_wlan_suspend();
#endif

   // Unregister the Net Device Notifier
   unregister_netdevice_notifier(&hdd_netdev_notifier);

  //Clean up HDD Nlink Service
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0); 
   nl_srv_exit();

#ifdef WLAN_SOFTAP_FEATURE
   //Deregister the hostapd device with the kernel
   if (VOS_STA_SAP_MODE == hdd_get_conparam())
         hdd_unregister_hostapd(pAdapter->pHostapd_dev);
#endif

   //Deregister the device with the kernel
   hdd_UnregisterWext(pAdapter->dev);

   if(test_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags)) 
   {
#ifdef WLAN_SOFTAP_FEATURE
      if (VOS_STA_SAP_MODE == hdd_get_conparam())
      {
           unregister_netdev(pAdapter->pHostapd_dev );
           //Free the hostapd net device
           free_netdev(pAdapter->pHostapd_dev);
      }
#endif
      unregister_netdev(pAdapter->dev);
      clear_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);
   }

  //Free up dynamically allocated members inside HDD Adapter
   kfree(pAdapter->cfg_ini);
   pAdapter->cfg_ini= NULL;

   //Free the net device
   free_netdev(pAdapter->dev);

   WLANSAL_Close(pVosContext);
   vos_preClose( &pVosContext );

#ifdef MEMORY_DEBUG
   vos_mem_exit(); 
#endif

   return -1;

success:
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


VOS_STATUS hdd_wlan_reset_initialization(void) 
{
      struct sdio_func *sdio_func_dev = NULL;
      v_U8_t  regValue = 0;
      int  err_ret = 0;
      v_CONTEXT_t pVosContext = NULL;
    
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: WLAN being reset",__func__);  
   
      //Get the global VOSS context.
      pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
      if(!pVosContext) {
          hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
         return VOS_STATUS_E_FAILURE;
      }
	  
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Preventing the phone from going to suspend",__func__);
   
      // Prevent the phone from going to sleep
      hdd_prevent_suspend();

      /* Clear pending interrupt and  disable Interrupts. Use only CMD52 */
           
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, 
             "%s LOGP in progress. Disabling Interrupt", __func__);

      sdio_func_dev = libra_getsdio_funcdev();

      if(sdio_func_dev == NULL)
      {
           /* Our card got removed before LOGP. Continue with reset anyways */
           hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
	    return VOS_STATUS_SUCCESS;
      }

      sd_claim_host(sdio_func_dev);
	  
      regValue = 0;
      libra_sdiocmd52(sdio_func_dev, QWLAN_SIF_SIF_INT_EN_REG,  
                                &regValue, 1, &err_ret);

      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, 
	  	"%s LOGP Cleared SIF_SIF_INT_EN_REG status:%d", __func__,err_ret);
	  
      regValue = 0;
      libra_sdiocmd52(sdio_func_dev, QWLAN_SIF_BAR4_INT_PEND_REG,  
                                &regValue, 1, &err_ret);
	  
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, 
	  	"%s LOGP Cleared SIF_BAR4_INT_PEND_REG status :%d", __func__,err_ret);	

      regValue = 0;
      libra_sdiocmd52(sdio_func_dev, QWLAN_SIF_BAR4_INT_ENABLE_REG,  
                                &regValue, 1, &err_ret);
	  
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, 
	  	"%s LOGP Cleared SIF_BAR4_INT_ENABLE_REG: Status:%d", __func__,err_ret);

      /* Disable SDIO IRQ */
      libra_enable_sdio_irq(sdio_func_dev, 0);
	  
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing Suspend Chip",__func__);

      //Put the chip is standby before asserting deep sleep
      WLANBAL_SuspendChip_NoLock( pVosContext );

      sd_release_host(sdio_func_dev);
	  
      return VOS_STATUS_SUCCESS;
}

void register_wlan_suspend(void)
{
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Register WLAN suspend/resume "
            "callbacks",__func__);

   wake_lock_init(&wlan_wake_lock, WAKE_LOCK_SUSPEND, "wlan");

   wlan_early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
   wlan_early_suspend.suspend = hdd_suspend_wlan;
   wlan_early_suspend.resume = hdd_resume_wlan;
   register_early_suspend(&wlan_early_suspend);
}

void unregister_wlan_suspend(void)
{
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unregister WLAN suspend/resume "
            "callbacks",__func__);

   wake_lock_destroy(&wlan_wake_lock);

   unregister_early_suspend(&wlan_early_suspend);
}

#endif
