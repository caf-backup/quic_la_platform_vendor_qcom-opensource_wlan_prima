/*
* Copyright (c) 2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

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
#include <wlan_hdd_main.h>
#include <wlan_hdd_p2p.h>
#include <wlan_hdd_assoc.h>
#include <wlan_sal_misc.h>
#include <libra_sdioif.h>
#include <wlan_nlink_srv.h>
#include <wlan_hdd_misc.h>

#ifdef WLAN_SOFTAP_FEATURE
#include <linux/semaphore.h>
#include <wlan_hdd_hostapd.h>
#endif

#include <linux/inetdevice.h>
#include <wlan_hdd_cfg.h>
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
static struct wake_lock wlan_wake_lock;
#endif
#endif
static eHalStatus g_full_pwr_status;
static eHalStatus g_standby_status;

extern VOS_STATUS hdd_post_voss_start_config(hdd_context_t* pHddCtx);
extern VOS_STATUS vos_chipExitDeepSleepVREGHandler(
   vos_call_status_type* status,
   vos_power_cb_type callback,
   v_PVOID_t user_data);
extern void hdd_wlan_initial_scan(hdd_context_t *pHddCtx);
void unregister_wlan_suspend(void);

extern struct notifier_block hdd_netdev_notifier;
#ifdef WLAN_SOFTAP_FEATURE
extern tVOS_CON_MODE hdd_get_conparam ( void );
#endif
//Callback invoked by PMC to report status of standby request
void hdd_suspend_standby_cbk (void *callbackContext, eHalStatus status)
{
   hdd_context_t *pHddCtx = (hdd_context_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Standby status = %d", __func__, status);
   g_standby_status = status; 

   if(eHAL_STATUS_SUCCESS == status)
   {
      pHddCtx->hdd_ps_state = eHDD_SUSPEND_STANDBY;
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestStandby failed",__func__);
   }

   complete(&pHddCtx->standby_comp_var);
}

//Callback invoked by PMC to report status of full power request
void hdd_suspend_full_pwr_callback(void *callbackContext, eHalStatus status)
{
   hdd_context_t *pHddCtx = (hdd_context_t*)callbackContext;
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Full Power status = %d", __func__, status);
   g_full_pwr_status = status;

   if(eHAL_STATUS_SUCCESS == status)
   {
      pHddCtx->hdd_ps_state = eHDD_SUSPEND_NONE;
   }
   else
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: sme_RequestFullPower failed",__func__);
   }

   complete(&pHddCtx->full_pwr_comp_var);
}

eHalStatus hdd_exit_standby(hdd_context_t *pHddCtx)
{  
    eHalStatus status = VOS_STATUS_SUCCESS;

    hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from standby",__func__);
    INIT_COMPLETION(pHddCtx->full_pwr_comp_var);

   g_full_pwr_status = eHAL_STATUS_FAILURE;
    status = sme_RequestFullPower(pHddCtx->hHal, hdd_suspend_full_pwr_callback, pHddCtx,
      eSME_FULL_PWR_NEEDED_BY_HDD);

   if(status == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pHddCtx->full_pwr_comp_var, 
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
      pHddCtx->hdd_ps_state = eHDD_SUSPEND_NONE;

failure:
    //No blocking to reduce latency. No other device should be depending on WLAN
    //to finish resume and WLAN won't be instantly on after resume
    return status;
}


//Helper routine to put the chip into standby
VOS_STATUS hdd_enter_standby(hdd_context_t *pHddCtx)
{
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode on its own during suspend sequence
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Note we do not disable queues unnecessarily. Queues should already be disabled
   //if STA is disconnected or the queue will be disabled as and when disconnect
   //happens because of standby procedure.
   
   //Ensure that device is in full power first. There is scope for optimization
   //here especially in scenarios where PMC is already in IMPS or REQUEST_IMPS.
   //Core s/w needs to be optimized to handle this. Until then we request full
   //power before issuing request for standby.
   INIT_COMPLETION(pHddCtx->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pHddCtx->hHal, hdd_suspend_full_pwr_callback, 
       pHddCtx, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pHddCtx->full_pwr_comp_var, 
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

   if(pHddCtx->hdd_mcastbcast_filter_set == TRUE) {
         hdd_conf_mcastbcast_filter(pHddCtx, FALSE);
         pHddCtx->hdd_mcastbcast_filter_set = FALSE;
   }

   //Request standby. Standby will cause the STA to disassociate first. TX queues
   //will be disabled (by HDD) when STA disconnects. You do not want to disable TX
   //queues here. Also do not assert if the failure code is eHAL_STATUS_PMC_NOT_NOW as PMC
   //will send this failure code in case of concurrent sessions. Power Save cannot be supported
   //when there are concurrent sessions.
   INIT_COMPLETION(pHddCtx->standby_comp_var);
   g_standby_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestStandby(pHddCtx->hHal, hdd_suspend_standby_cbk, pHddCtx);

   if (halStatus == eHAL_STATUS_PMC_PENDING) 
   {
      //Wait till WLAN device enters standby mode
      wait_for_completion_timeout(&pHddCtx->standby_comp_var, 
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
      pHddCtx->hdd_ps_state = eHDD_SUSPEND_STANDBY;

failure:
   //Restore IMPS config
   if(pHddCtx->cfg_ini->fIsImpsEnabled)
      sme_EnablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);

   //Restore BMPS config
   if(pHddCtx->cfg_ini->fIsBmpsEnabled)
      sme_EnablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   return vosStatus;
}


//Helper routine for Deep sleep entry
VOS_STATUS hdd_enter_deep_sleep(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter)
{
   eHalStatus halStatus;
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_call_status_type callType;
   struct sdio_func *sdio_func_dev_current = NULL;
   int attempts = 0;

   //Stop the Interface TX queue.
   netif_tx_disable(pAdapter->dev);
   netif_carrier_off(pAdapter->dev);

   //Disable IMPS,BMPS as we do not want the device to enter any power
   //save mode on it own during suspend sequence
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);

   //Ensure that device is in full power as we will touch H/W during vos_Stop
   INIT_COMPLETION(pHddCtx->full_pwr_comp_var);
   g_full_pwr_status = eHAL_STATUS_FAILURE;
   halStatus = sme_RequestFullPower(pHddCtx->hHal, hdd_suspend_full_pwr_callback, 
       pHddCtx, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus == eHAL_STATUS_PMC_PENDING)
   {
      //Block on a completion variable. Can't wait forever though
      wait_for_completion_interruptible_timeout(&pHddCtx->full_pwr_comp_var, 
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
   halStatus = sme_RoamDisconnect(pHddCtx->hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);

   //Success implies disconnect command got queued up successfully
   if(halStatus == eHAL_STATUS_SUCCESS)
   {
      //Block on a completion variable. Can't wait forever though.
      wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var, 
         msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
   }


   //None of the steps should fail after this. Continue even in case of failure
   vosStatus = vos_stop( pHddCtx->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANBAL_Stop( pHddCtx->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANBAL_SuspendChip( pHddCtx->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = WLANSAL_Stop(pHddCtx->pvosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   vosStatus = vos_chipAssertDeepSleep( &callType, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Vote off any PMIC voltage supplies
   vosStatus = vos_chipPowerDown(NULL, NULL, NULL);
    //Get the Current SDIO Func
   sdio_func_dev_current = libra_getsdio_funcdev();
   if(NULL != sdio_func_dev_current) {
      libra_detect_card_change();
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

   pHddCtx->hdd_ps_state = eHDD_SUSPEND_DEEP_SLEEP;

   //Restore IMPS config
   if(pHddCtx->cfg_ini->fIsImpsEnabled)
      sme_EnablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);

   //Restore BMPS config
   if(pHddCtx->cfg_ini->fIsBmpsEnabled)
      sme_EnablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);

err_fail:

   return vosStatus;
}

VOS_STATUS hdd_exit_deep_sleep(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter)
{
   VOS_STATUS vosStatus;
   eHalStatus halStatus;
   int attempts = 0;
   struct sdio_func *sdio_func_dev = NULL;

   //Power Up Libra WLAN card first if not already powered up
   vosStatus = vos_chipPowerUp(NULL,NULL,NULL);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not Powered Up."
          "exiting", __func__);
      goto err_deep_sleep;
   }

   libra_detect_card_change();

   do {
      sdio_func_dev = libra_getsdio_funcdev();
      if (NULL == sdio_func_dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not detected yet.",__func__);
         attempts++;
      }
      else {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN detecton succeeded",__func__);
         break;
      }

      if(LIBRA_CARD_INSERT_DETECT_MAX_COUNT == attempts)
        break;

      msleep(250);

   }while (attempts < LIBRA_CARD_INSERT_DETECT_MAX_COUNT);

   //Retry to detect the card again by Powering Down the chip and Power up the chip
   //again. This retry is done to recover from CRC Error
   if (NULL == sdio_func_dev) {

      attempts = 0;

      //Vote off any PMIC voltage supplies
      vos_chipPowerDown(NULL, NULL, NULL);

      msleep(1000);

      //Power Up Libra WLAN card first if not already powered up
      vosStatus = vos_chipPowerUp(NULL,NULL,NULL);
      if (!VOS_IS_STATUS_SUCCESS(vosStatus))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not Powered Up."
             "exiting", __func__);
         goto err_deep_sleep;
      }

      do {
         sdio_func_dev = libra_getsdio_funcdev();
         if (NULL == sdio_func_dev) {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not detected yet.",__func__);
            attempts++;
         }
         else {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN detecton succeeded",__func__);
            break;
         }

         if(attempts == 2)
           break;

         msleep(1000);

      }while (attempts < 3);

   }

   if (NULL == sdio_func_dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not found!!",__func__);
         goto err_deep_sleep;
   }

   libra_sdio_setprivdata (sdio_func_dev, pHddCtx);
   atomic_set(&pHddCtx->sdio_claim_count, 0);
   pHddCtx->hsdio_func_dev = sdio_func_dev;

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling WLANSAL_Start",__func__);
   vosStatus = WLANSAL_Start(pHddCtx->pvosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed to start SAL",__func__);
      goto err_deep_sleep;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
      "%s: calling WLANBAL_ResumeChip",__func__);
   vosStatus = WLANBAL_ResumeChip( pHddCtx->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in WLANBAL_ResumeChip",__func__);
      goto err_sal_stop;
   }

   vosStatus = WLANBAL_Start(pHddCtx->pvosContext);

   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
               "%s: Failed to start BAL",__func__);
      goto err_sal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
      "%s: calling hdd_set_sme_config",__func__);
   vosStatus = hdd_set_sme_config( pHddCtx );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in hdd_set_sme_config",__func__);
      goto err_bal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling vos_start",__func__);
   vosStatus = vos_start( pHddCtx->pvosContext );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in vos_start",__func__);
      goto err_bal_stop;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
      "%s: calling hdd_post_voss_start_config",__func__);
   vosStatus = hdd_post_voss_start_config( pHddCtx );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: Failed in hdd_post_voss_start_config",__func__);
      goto err_voss_stop;
   }


   //Open a SME session for future operation
   halStatus = sme_OpenSession( pHddCtx->hHal, hdd_smeRoamCallback, pHddCtx,
                                (tANI_U8 *)&pAdapter->macAddressCurrent, &pAdapter->sessionId );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"sme_OpenSession() failed with status code %08d [x%08lx]",
                    halStatus, halStatus );
      goto err_voss_stop;

   }

   pHddCtx->hdd_ps_state = eHDD_SUSPEND_NONE;

   //Trigger the initial scan
   hdd_wlan_initial_scan(pHddCtx);

   return VOS_STATUS_SUCCESS;

err_voss_stop:
   vos_stop(pHddCtx->pvosContext);
err_bal_stop:
   WLANBAL_Stop(pHddCtx->pvosContext);
err_sal_stop:
   WLANSAL_Stop(pHddCtx->pvosContext);
err_deep_sleep:
   return VOS_STATUS_E_FAILURE;

}

VOS_STATUS hdd_conf_hostarpoffload(hdd_context_t* pHddCtx, v_BOOL_t fenable)
{
   struct in_ifaddr **ifap = NULL;
   struct in_ifaddr *ifa = NULL;
   struct in_device *in_dev;
   int i = 0;
   hdd_adapter_t *pAdapter = NULL;   
   tSirHostOffloadReq  offLoadRequest;

   pAdapter = hdd_get_adapter(pHddCtx,WLAN_HDD_INFRA_STATION);
   if(pAdapter == NULL)
   {
      pAdapter = hdd_get_adapter(pHddCtx,WLAN_HDD_P2P_CLIENT);
      if(pAdapter == NULL)
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }
   }

   

   if(fenable)
   {
       if ((in_dev = __in_dev_get_rtnl(pAdapter->dev)) != NULL)
       {
           for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL; 
                   ifap = &ifa->ifa_next)
           {
               if (!strcmp(pAdapter->dev->name, ifa->ifa_label))
               {
                   break; /* found */
               }
           }
       }
       
       if(ifa && ifa->ifa_local)
       {
           offLoadRequest.offloadType =  SIR_IPV4_ARP_REPLY_OFFLOAD;
           offLoadRequest.enableOrDisable = SIR_OFFLOAD_ENABLE;

           if(HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST ==
                   pHddCtx->cfg_ini->mcastBcastFilterSetting )
           {
               offLoadRequest.enableOrDisable |= 
                       SIR_OFFLOAD_BCAST_FILTER_ENABLE;
           }
           else if(HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST ==
                   pHddCtx->cfg_ini->mcastBcastFilterSetting )
           {
               offLoadRequest.enableOrDisable |= 
                       SIR_OFFLOAD_MCAST_FILTER_ENABLE;
           }
           else if(HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST ==
                   pHddCtx->cfg_ini->mcastBcastFilterSetting )
           {
               offLoadRequest.enableOrDisable |= 
                       SIR_OFFLOAD_BCAST_FILTER_ENABLE;
               offLoadRequest.enableOrDisable |= 
                       SIR_OFFLOAD_MCAST_FILTER_ENABLE;
           }
 
#if 0
           if((HDD_MCASTBCASTFILTER_FILTER_ALL_BROADCAST ==
                   pHddCtx->cfg_ini->mcastBcastFilterSetting )
                    || (HDD_MCASTBCASTFILTER_FILTER_ALL_MULTICAST_BROADCAST ==
                    pHddCtx->cfg_ini->mcastBcastFilterSetting))
           {
               //MCAST filter is set by hdd_conf_mcastbcast_filter fn call
               offLoadRequest.enableOrDisable = 
                       SIR_OFFLOAD_ARP_AND_BCAST_FILTER_ENABLE;
           }
#endif           
           //converting u32 to IPV4 address
           for(i = 0 ; i < 4; i++)
           {
              offLoadRequest.params.hostIpv4Addr[i] = 
                      (ifa->ifa_local >> (i*8) ) & 0xFF ;
           }

          if (eHAL_STATUS_SUCCESS != 
                    sme_SetHostOffload(WLAN_HDD_GET_HAL_CTX(pAdapter) , &offLoadRequest))
          {
              hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Failed to enable HostOffload \
                      feature\n", __func__);
              return VOS_STATUS_E_FAILURE;
          }
		  return VOS_STATUS_SUCCESS;
       }
       else
       {
           hddLog(VOS_TRACE_LEVEL_INFO, "%s:IP Address is not assigned \n", __func__);
           return VOS_STATUS_E_AGAIN;
       }
   }
   else
   {
       vos_mem_zero((void *)&offLoadRequest, sizeof(tSirHostOffloadReq));
       offLoadRequest.enableOrDisable = SIR_OFFLOAD_DISABLE;
       offLoadRequest.offloadType =  SIR_IPV4_ARP_REPLY_OFFLOAD;

       if (eHAL_STATUS_SUCCESS != sme_SetHostOffload(WLAN_HDD_GET_HAL_CTX(pAdapter), &offLoadRequest))
       {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Failure to disable host \
                             offload feature\n",__func__);
            return VOS_STATUS_E_FAILURE;
       }
	   return VOS_STATUS_SUCCESS;
   }
}

void hdd_conf_mcastbcast_filter(hdd_context_t* pHddCtx, v_BOOL_t setfilter)
{
    eHalStatus halStatus = eHAL_STATUS_FAILURE;
#if 0
    tpAniSirGlobal pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_SME, pHddCtx->pvosContext);
#endif
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

    hddLog(VOS_TRACE_LEVEL_INFO,
        "%s: Configuring Mcast/Bacst Filter Setting. setfilter %d", __func__, setfilter);

    if(pHddCtx->cfg_ini->fhostArpOffload)
    {
        vosStatus = hdd_conf_hostarpoffload(pHddCtx, setfilter);
		if (!VOS_IS_STATUS_SUCCESS(vosStatus))
        {
            pHddCtx->hdd_host_arpoffload_failed = TRUE;
            hddLog(VOS_TRACE_LEVEL_INFO, "%s:Failed to enable ARPOFFLOAD \
                    Feature %d\n", __func__, vosStatus) ;
        }
        else
        {
            pHddCtx->hdd_host_arpoffload_failed = FALSE;
        }
    }

#if 0
    if ( pMac ) 
    {
      halStatus = halRxp_configureRxpFilterMcstBcst( pMac, setfilter);
    }
    else
    {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: pMac is initialised to NULL",__func__ );
    }
#endif
    if(setfilter && (eHAL_STATUS_SUCCESS == halStatus))
       pHddCtx->hdd_mcastbcast_filter_set = TRUE;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static inline void hdd_prevent_suspend(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
    wake_lock(&wlan_wake_lock);
#endif
}

static inline void hdd_allow_suspend(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
    wake_unlock(&wlan_wake_lock);
#endif
}
//Suspend routine registered with Android OS
void hdd_suspend_wlan(struct early_suspend *wlan_suspend)
{
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext = NULL;

   hdd_adapter_t *pAdapter = NULL; 
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;

   struct sdio_func *sdio_func_dev = NULL;
   
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being suspended by Android OS",__func__);

   //Get the global VOSS context.
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if(!pVosContext) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return;
   }

   //Get the HDD context.
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );

   if(!pHddCtx) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return;
   }

   if (pHddCtx->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore suspend wlan,\
                                     LOGP in progress!", __func__);
      return;
   }

   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
        /* Our card got removed */
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
        return;
   }

   if(!sd_is_drvdata_available(sdio_func_dev))
   {
        /* Our card got removed */
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HDD context is not available\
                                       in sdio_func_dev!",__func__);
        return;
   }

   sd_claim_host(sdio_func_dev);
   
   // Prevent touching the pMac while LOGP reset in progress, we should never get here
   // as the wake lock is already acquired and it would prevent from entering suspend 
   if (pHddCtx->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore suspend wlan, LOGP in progress!", __func__);
      sd_release_host(sdio_func_dev);
      return;
   }

   /*We aquire the lock to check for logp so releasing it soon after the check*/
   sd_release_host(sdio_func_dev);
   
   /*loop through all adapters. TBD fix for Concurrency */
   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
       pAdapter = pAdapterNode->pAdapter;
       if ( (WLAN_HDD_INFRA_STATION != pAdapter->device_mode)
         && (WLAN_HDD_P2P_CLIENT != pAdapter->device_mode) )

       {  //just do for station interface
           status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
           pAdapterNode = pNext;
           continue;
       }

#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP
       if (pHddCtx->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_STANDBY)
       {
          //stop the interface before putting the chip to standby
          netif_tx_disable(pAdapter->dev);
          netif_carrier_off(pAdapter->dev);
       }
       else if (pHddCtx->cfg_ini->nEnableSuspend == 
               WLAN_MAP_SUSPEND_TO_DEEP_SLEEP)
       {
          //Execute deep sleep procedure
          hdd_enter_deep_sleep(pHddCtx, pAdapter);
       }
#endif

   if (pHddCtx->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_MCAST_BCAST_FILTER) 
   {
      if (eConnectionState_Associated == 
            (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState) 
      {
         hdd_conf_mcastbcast_filter(pHddCtx, TRUE);
         halPSAppsCpuWakeupState(vos_get_context(VOS_MODULE_ID_SME,
                                  pHddCtx->pvosContext), FALSE);
       }
   }
   pHddCtx->hdd_wlan_suspended = TRUE;
   status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
   pAdapterNode = pNext;
  }

#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP
  if(pHddCtx->cfg_ini->nEnableSuspend == WLAN_MAP_SUSPEND_TO_STANDBY)
  {
      hdd_enter_standby(pHddCtx);
  }
#endif

   return;
}

static void hdd_PowerStateChangedCB
(
   v_PVOID_t callbackContext,
   tPmcState newState
)
{
   hdd_context_t *pHddCtx = callbackContext;
   
   spin_lock(&pHddCtx->filter_lock);
   if((newState == BMPS) &&  pHddCtx->hdd_wlan_suspended
          && (pHddCtx->hdd_mcastbcast_filter_set != TRUE)) {
      spin_unlock(&pHddCtx->filter_lock);
      hdd_conf_mcastbcast_filter(pHddCtx, TRUE);
      halPSAppsCpuWakeupState(vos_get_context(VOS_MODULE_ID_SME, pHddCtx->pvosContext), FALSE);
      if(pHddCtx->hdd_mcastbcast_filter_set != TRUE)
         hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Not able to set mcast/bcast filter ", __func__);
   }
   else 
      spin_unlock(&pHddCtx->filter_lock);
}



void hdd_register_mcast_bcast_filter(hdd_context_t *pHddCtx)
{
   v_CONTEXT_t pVosContext;
   tHalHandle  smeContext;

   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if (NULL == pVosContext)
   {
      hddLog(LOGE, "%s: Invalid pContext", __FUNCTION__);
      return;
   }
   smeContext = vos_get_context(VOS_MODULE_ID_HAL, pVosContext);
   if (NULL == smeContext)
   {
      hddLog(LOGE, "%s: Invalid smeContext", __FUNCTION__);
      return;
   }

   spin_lock_init(&pHddCtx->filter_lock);
   if (WLAN_MAP_SUSPEND_TO_MCAST_BCAST_FILTER == 
             pHddCtx->cfg_ini->nEnableSuspend)
   {
      pmcRegisterDeviceStateUpdateInd( smeContext,
                    hdd_PowerStateChangedCB, pHddCtx );
   }
   
}

void hdd_unregister_mcast_bcast_filter(hdd_context_t *pHddCtx)
{
   v_CONTEXT_t pVosContext;
   tHalHandle smeContext;

   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if (NULL == pVosContext)
   {
      hddLog(LOGE, "%s: Invalid pContext", __FUNCTION__);
      return;
   }
   smeContext = vos_get_context(VOS_MODULE_ID_SME, pVosContext);
   if (NULL == smeContext)
   {
      hddLog(LOGE, "%s: Invalid smeContext", __FUNCTION__);
      return;
   }

   if (WLAN_MAP_SUSPEND_TO_MCAST_BCAST_FILTER == 
                                            pHddCtx->cfg_ini->nEnableSuspend)
   {
      pmcDeregisterDeviceStateUpdateInd(smeContext, hdd_PowerStateChangedCB);
   }
}

void hdd_resume_wlan(struct early_suspend *wlan_suspend)
{
   hdd_context_t *pHddCtx = NULL;
   hdd_adapter_t *pAdapter = NULL;
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
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
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );

   if(!pHddCtx) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return;
   }
   
   if (pHddCtx->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore resume wlan,\
                                     LOGP in progress!", __func__);
      return;
   }

   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
      /* Our card got removed */
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
      return;
   }

   if(!sd_is_drvdata_available(sdio_func_dev))
   {
        /* Our card got removed */
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: HDD context is not available\
                                       in sdio_func_dev!",__func__);
        return;
   }

   sd_claim_host(sdio_func_dev);
   
   // Prevent touching the pMac while LOGP reset in progress, 
   if (pHddCtx->isLogpInProgress) {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Ignore resume wlan, LOGP in progress!", __func__);
      sd_release_host(sdio_func_dev);
      return;
   }

    /*We aquire the lock to check for logp so releasing it soon after the check*/
   sd_release_host(sdio_func_dev);
   
   /*loop through all adapters. Concurrency */
   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
       pAdapter = pAdapterNode->pAdapter;
       if ( (WLAN_HDD_INFRA_STATION != pAdapter->device_mode)
         && (WLAN_HDD_P2P_CLIENT != pAdapter->device_mode) )
       {  //just do for station interface
            status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
            pAdapterNode = pNext;
            continue;
       }
#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP   
       if(pHddCtx->hdd_ps_state == eHDD_SUSPEND_DEEP_SLEEP) 
       {
          hddLog(VOS_TRACE_LEVEL_ERROR, "%s: WLAN being resumed from deep sleep",__func__);
          hdd_exit_deep_sleep(pAdapter);
       }
#endif
       if(pHddCtx->hdd_mcastbcast_filter_set == TRUE)
       {
          hdd_conf_mcastbcast_filter(pHddCtx, FALSE);
          pHddCtx->hdd_mcastbcast_filter_set = FALSE;
          halPSAppsCpuWakeupState(vos_get_context(VOS_MODULE_ID_SME,
                                  pHddCtx->pvosContext), TRUE);

      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

#ifdef SUPPORT_EARLY_SUSPEND_STANDBY_DEEPSLEEP   
   if(pHddCtx->hdd_ps_state == eHDD_SUSPEND_STANDBY) 
   {
       hdd_exit_standby(pHddCtx);
   }    
#endif

   return;
}

VOS_STATUS hdd_wlan_reset(vos_chip_reset_reason_type reset_reason) 
{
   VOS_STATUS vosStatus;
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext = NULL;
   pVosSchedContext vosSchedContext = NULL;
   struct sdio_func *sdio_func_dev = NULL;
   unsigned int attempts = 0;

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: WLAN being reset",__func__);

   //Get the global VOSS context.
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
   if(!pVosContext) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return VOS_STATUS_E_FAILURE;
   }

   //Get the HDD context.
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext);
   if(!pHddCtx) {
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


   /**
   EVM issue is observed with 1.6Mhz freq for 1.3V supply in wlan standalone case.
   During concurrent operation (e.g. WLAN and WCDMA) this issue is not observed. 
   To workaround, wlan will vote for 3.2Mhz during startup and will vote for 1.6Mhz
   during exit.
   */
   if (vos_chipVoteFreqFor1p3VSupply(NULL, NULL, NULL, VOS_NV_FREQUENCY_FOR_1_3V_SUPPLY_1P6MH) != VOS_STATUS_SUCCESS)
        VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to set the freq to 1.6Mhz for 1.3V Supply",__func__ );

#ifdef CONFIG_CFG80211
   hdd_p2p_cleanup(pHddCtx);
#endif

   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode on its own during reset sequence
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_UAPSD_MODE_POWER_SAVE);

   //Kill all the threads first. We do not want any messages
   //to be a processed any more and the best way to ensure that
   //is to terminate the threads gracefully.
   vosSchedContext = get_vos_sched_ctxt();

   /* Wakeup Mc Thread if Suspended */
   if(TRUE == pHddCtx->isMcThreadSuspended){
      complete(&vosSchedContext->ResumeMcEvent);
      pHddCtx->isMcThreadSuspended= FALSE;
   }

   /* Wakeup Tx Thread if Suspended */
   if(TRUE == pHddCtx->isTxThreadSuspended){
      complete(&vosSchedContext->ResumeTxEvent);
      pHddCtx->isTxThreadSuspended= FALSE;
   }

   /* Reset the Suspend Variable */
   pHddCtx->isWlanSuspended = FALSE;

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

   /* Cancel the vote for XO Core ON in LOGP since we are reinitializing our driver
    * This is done here to ensure there is no race condition since MC and TX thread have
    * exited at this point
    */
   hddLog(VOS_TRACE_LEVEL_WARN, "In LOGP: Cancel XO Core ON vote\n");
   if (vos_chipVoteXOCore(NULL, NULL, NULL, VOS_FALSE) != VOS_STATUS_SUCCESS)
   {
       hddLog(VOS_TRACE_LEVEL_ERROR, "Could not cancel XO Core ON vote. Not returning failure."
                                         "Power consumed will be high\n");
   }

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing SME STOP",__func__);
   //Stop SME - Cannot invoke vos_stop as vos_stop relies
   //on threads being running to process the SYS Stop
   vosStatus = sme_Stop( pHddCtx->hHal, TRUE );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Doing MAC STOP",__func__);
   //Stop MAC (PE and HAL)
   vosStatus = macStop( pHddCtx->hHal, HAL_STOP_TYPE_SYS_RESET);
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

#ifdef CONFIG_HAS_EARLYSUSPEND
   hdd_unregister_mcast_bcast_filter(pHddCtx);
#endif

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

   libra_sdio_notify_card_removal(NULL);
   WLANSAL_allow_card_removal();

#ifdef TIMER_MANAGER
   vos_timer_exit();
#endif

#ifdef MEMORY_DEBUG
   vos_mem_clean();
#endif

   if(reset_reason == VOS_CHIP_SHUTDOWN)
   {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Request for shutdown, "
              "exiting", __func__);
       goto shutdown;
   }

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
      sdio_func_dev = libra_getsdio_funcdev();
      if(sdio_func_dev != NULL)
      {
         libra_sdio_setprivdata (sdio_func_dev, pHddCtx);
         atomic_set(&pHddCtx->sdio_claim_count, 0);
         pHddCtx->hsdio_func_dev = sdio_func_dev;
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                 "%s: Card Detected Successfully %p",__func__, 
                 sdio_func_dev);
         break;
      }
      else
      {
         VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
                 "%s: Failed to detect card change %p",__func__, 
                 sdio_func_dev);     
         attempts++;
      }
   }while (attempts < LIBRA_CARD_INSERT_DETECT_MAX_COUNT);

   if(LIBRA_CARD_INSERT_DETECT_MAX_COUNT == attempts)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN fail to detect in reset"
              "exiting", __func__);
      goto err_fail;
   }

   /* Enable IRQ capabilities in host controller */
   sd_claim_host(sdio_func_dev);
   libra_disable_sdio_irq_capability(sdio_func_dev, 0);
   libra_enable_sdio_irq(sdio_func_dev, 1);
   sd_release_host(sdio_func_dev);

   vosStatus = WLANBAL_Open(pVosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
        "%s: Failed to open BAL",__func__);
     goto err_fail;
   }

   vosStatus = WLANSAL_Start(pVosContext);
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to start SAL",__func__);
      goto err_balclose;
   }

   /* Chip is reset so allow Lower MAC to start accessing WLAN registers. Note HDD is still blocked */
   vos_set_logp_in_progress(VOS_MODULE_ID_HDD, FALSE);

   /* Start BAL */
   vosStatus = WLANBAL_Start(pVosContext);

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
   pHddCtx->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_HAL, pVosContext );

   if ( NULL == pHddCtx->hHal )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HAL context is null",__func__);      
      goto err_vosclose;
   }
   // Set the SME configuration parameters...
   vosStatus = hdd_set_sme_config(pHddCtx);

   if ( VOS_STATUS_SUCCESS != vosStatus )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed hdd_set_sme_config",__func__); 
      goto err_vosclose;
   }

   //Initialize the WMM module
   vosStatus = hdd_wmm_init(pHddCtx);
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_wmm_init failed", __FUNCTION__);
      goto err_vosclose;
   }

   /*Start VOSS which starts up the SME/MAC/HAL modules and everything else
     Note: Firmware image will be read and downloaded inside vos_start API */
   vosStatus = vos_start( pVosContext );
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_start failed",__func__);
      goto err_vosclose;
   }

   vosStatus = hdd_post_voss_start_config( pHddCtx );
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_post_voss_start_config failed", 
         __func__);
      goto err_vosstop;
   }

   /* Restart all adapters */
   hdd_start_all_adapters(pHddCtx);
   pHddCtx->isLogpInProgress = FALSE;
   pHddCtx->hdd_mcastbcast_filter_set = FALSE;
#ifdef CONFIG_HAS_EARLYSUSPEND
   hdd_register_mcast_bcast_filter(pHddCtx);
#endif

   // Allow the phone to go to sleep
   hdd_allow_suspend();

   goto success;

err_vosstop:
   vos_stop(pVosContext);

err_vosclose:	
   vos_close(pVosContext ); 
   vos_sched_close(pVosContext);

err_balstop:

   if(pHddCtx->driver_type == eDRIVER_TYPE_PRODUCTION)
   {
     wlan_hdd_enable_deepsleep(pVosContext);
   }
   WLANBAL_Stop(pVosContext);
   WLANBAL_SuspendChip(pVosContext);

err_salstop:
   WLANSAL_Stop(pVosContext);

err_balclose:
   WLANBAL_Close(pVosContext);

err_fail:
   //Assert Deep sleep signal now to put Libra HW in lowest power state
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );

   //Vote off any PMIC voltage supplies
   vos_chipPowerDown(NULL, NULL, NULL);

shutdown:
err_pwr_fail:
   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   // Allow the phone to go to sleep
   hdd_allow_suspend();

#ifdef CONFIG_HAS_EARLYSUSPEND
   // unregister suspend/resume callbacks
   if(pHddCtx->cfg_ini->nEnableSuspend)
      unregister_wlan_suspend();
#endif

   // Unregister the Net Device Notifier
   unregister_netdevice_notifier(&hdd_netdev_notifier);

  //Clean up HDD Nlink Service
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0); 
   nl_srv_exit();

  hdd_close_all_adapters(pHddCtx);
  //Free up dynamically allocated members inside HDD Adapter
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

#ifdef CONFIG_CFG80211
   wiphy_unregister(pHddCtx->wiphy);
   wiphy_free(pHddCtx->wiphy);
#else
   vos_mem_free(pHddCtx);
#endif

   WLANSAL_Close(pVosContext);
   vos_preClose(&pVosContext);

#ifdef MEMORY_DEBUG
   vos_mem_exit();
#endif

   return -1;

success:
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
   if(!pVosContext) 
   {
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

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
   wake_lock_init(&wlan_wake_lock, WAKE_LOCK_SUSPEND, "wlan");
#endif

   wlan_early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
   wlan_early_suspend.suspend = hdd_suspend_wlan;
   wlan_early_suspend.resume = hdd_resume_wlan;
   register_early_suspend(&wlan_early_suspend);
}

void unregister_wlan_suspend(void)
{
   hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unregister WLAN suspend/resume "
            "callbacks",__func__);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,4,5))
   wake_lock_destroy(&wlan_wake_lock);
#endif

   unregister_early_suspend(&wlan_early_suspend);
}

#endif
