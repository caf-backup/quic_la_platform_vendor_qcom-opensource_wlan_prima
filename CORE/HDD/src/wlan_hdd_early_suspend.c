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
#include "halTypes.h"
#include "sme_Api.h"
#include <vos_api.h>
#include "vos_power.h"

/**-----------------------------------------------------------------------------
*   Preprocessor definitions and constants
* ----------------------------------------------------------------------------*/

/**-----------------------------------------------------------------------------
*   Type declarations
* ----------------------------------------------------------------------------*/

/**-----------------------------------------------------------------------------
*   Function and variables declarations
* ----------------------------------------------------------------------------*/

#define WLAN_MAP_SUSPEND_TO_STANDBY     1
#define WLAN_MAP_SUSPEND_TO_DEEP_SLEEP  2

static struct early_suspend wlan_early_suspend;
extern VOS_STATUS hdd_post_voss_start_config(hdd_adapter_t* pAdapter);
extern VOS_STATUS vos_chipExitDeepSleepVREGHandler(
   vos_call_status_type* status,
   vos_power_cb_type callback,
   v_PVOID_t user_data);
extern void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);

//Callback invoked by PMC to report status of standby request
static void hdd_suspend_standby_cbk (void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Standby status = %d", __func__, status);
   complete(&pAdapter->standby_comp_var);
}

//Callback invoked by PMC to report status of full power request
static void hdd_suspend_full_pwr_callback(void *callbackContext, eHalStatus status)
{
   hdd_adapter_t *pAdapter = (hdd_adapter_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Full Power status = %d", __func__, status);
   complete(&pAdapter->full_pwr_comp_var);
}

//Helper routine for Deep sleep entry
VOS_STATUS hdd_enter_deep_sleep(hdd_adapter_t* pAdapter)
{
   eHalStatus halStatus;
   VOS_STATUS vosStatus;
   vos_call_status_type callType;

   //Ensure that device is in full power as we will touch H/W during vos_Stop
   init_completion(&pAdapter->full_pwr_comp_var);
   halStatus = sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, 
       pAdapter, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus != eHAL_STATUS_SUCCESS)
   {
      if(halStatus == eHAL_STATUS_PMC_PENDING)
      {
         //Block on a completion variable. Can't wait forever though
         wait_for_completion_interruptible_timeout(&pAdapter->full_pwr_comp_var, 
         msecs_to_jiffies(1000));
      }
      else
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Request for Full Power failed\n", __func__);
         VOS_ASSERT(0);
         return VOS_STATUS_E_FAILURE;
      }
   }
   
   vosStatus = vos_stop( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANBAL_SuspendChip( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipVoteOffPASupply( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANSAL_SetCardStatusNotfPath( 
      pAdapter->pvosContext, WLANSAL_NOTF_PATH_SAL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANSAL_Stop(pAdapter->pvosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   pAdapter->hdd_ps_state = eHDD_SUSPEND_DEEP_SLEEP;

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_exit_deep_sleep(hdd_adapter_t* pAdapter)
{
   vos_call_status_type callType;
   VOS_STATUS vosStatus;

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipDeAssertDeepSleep",__func__);
   vosStatus = vos_chipDeAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipExitDeepSleepVREGHandler",__func__);
   vosStatus = vos_chipExitDeepSleepVREGHandler( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_chipVoteOnPASupply",__func__);
   vosStatus = vos_chipVoteOnPASupply( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANSAL_SDIOReInit",__func__);
   vosStatus = WLANSAL_SDIOReInit( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANSAL_SetCardStatusNotfPath",__func__);
   vosStatus = WLANSAL_SetCardStatusNotfPath( 
      pAdapter->pvosContext, WLANSAL_NOTF_PATH_SDBUS );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANBAL_ResumeChip",__func__);
   vosStatus = WLANBAL_ResumeChip( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling hdd_set_sme_config",__func__);
   vosStatus = hdd_set_sme_config( pAdapter );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_start",__func__);
   vosStatus = vos_start( pAdapter->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling hdd_post_voss_start_config",__func__);
   vosStatus = hdd_post_voss_start_config( pAdapter );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Trigger the initial scan
   hdd_wlan_initial_scan(pAdapter);

   return VOS_STATUS_SUCCESS;
}
//Suspend routine registered with Android OS
void hdd_suspend_wlan(struct early_suspend *wlan_suspend)
{
   eHalStatus halStatus;
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

   //Turn off carrier state
   netif_carrier_off(pAdapter->dev);
   
   //Stop the Interface TX queue.
   netif_tx_disable(pAdapter->dev);

   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode during suspend sequence
   sme_DisablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   if(pAdapter->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_STANDBY) 
   {
      //Execute standby procedure. Executing standby procedure will cause the STA to
      //disassociate first and then the chip will be put into standby.

      init_completion(&pAdapter->standby_comp_var);
      halStatus = sme_RequestStandby(pAdapter->hHal, hdd_suspend_standby_cbk, pAdapter);

      if (halStatus == eHAL_STATUS_SUCCESS) {
         //Already in standby mode
         pAdapter->hdd_ps_state = eHDD_SUSPEND_STANDBY;
      }
      else if (halStatus == eHAL_STATUS_PMC_PENDING) {
         //Wait till WLAN device enters standby mode
         wait_for_completion_interruptible_timeout(&pAdapter->standby_comp_var, 
            msecs_to_jiffies(1000));
         pAdapter->hdd_ps_state = eHDD_SUSPEND_STANDBY;
      }
      else {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestStandby failed - status %d",
            __func__, halStatus);
      }
   }
   else if(pAdapter->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_DEEP_SLEEP) {
      //Execute deep sleep procedure
      hdd_enter_deep_sleep(pAdapter);
   }
   else {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported suspend mapping %d",
         __func__, pAdapter->cfg_ini->nEnableSuspend);
   }

   //Restore IMPS config
   if(pAdapter->cfg_ini->fIsImpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_IDLE_MODE_POWER_SAVE);

   //Restore BMPS config
   if(pAdapter->cfg_ini->fIsBmpsEnabled)
      sme_EnablePowerSave(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE);

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
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from standby",__func__);
      init_completion(&pAdapter->full_pwr_comp_var);
      (void)sme_RequestFullPower(pAdapter->hHal, hdd_suspend_full_pwr_callback, pAdapter,
         eSME_FULL_PWR_NEEDED_BY_HDD);

      //Trigger an association to the last AP we were connected to
      (void)sme_RoamConnectToLastProfile(pAdapter->hHal);

      //No blocking to reduce latency. No other device should be depending on WLAN
      //to finish resume and WLAN won't be instantly on after resume
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
