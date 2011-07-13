/**========================================================================= 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$   $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  03/29/11    tbh    Created module. 

  ==========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <wlan_hdd_dev_pwr.h>
#ifdef ANI_BUS_TYPE_PLATFORM
#include <linux/wcnss_wlan.h>
#else
#include <wcnss_wlan.h>
#endif // ANI_BUS_TYP_PLATFORM

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Global variables.
 *-------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------

   @brief Function to suspend the wlan driver.

   @param HDD_ADAPTER_HANDLE


   @return None

----------------------------------------------------------------------------*/
static int wlan_suspend(hdd_adapter_t* pAdapter)
{
   int rc = 0;

   pVosSchedContext vosSchedContext = NULL;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   /* Get the global VOSS context */
   vosSchedContext = get_vos_sched_ctxt();

   if(!vosSchedContext) {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: Global VOS_SCHED context is Null",__func__);
      return 0;
   }

   /* Set the Station state as Suspended */
   pHddCtx->isWlanSuspended = TRUE;

   /*
     Suspending MC Thread, Rx Thread and Tx Thread as the platform driver is going to Suspend.     
   */
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Suspending Mc, Rx and Tx Threads",__func__);

   init_completion(&pHddCtx->tx_sus_event_var);

   /* Indicate Tx Thread to Suspend */
   set_bit(TX_SUSPEND_EVENT_MASK, &vosSchedContext->txEventFlag);

   wake_up_interruptible(&vosSchedContext->txWaitQueue);

   /* Wait for Suspend Confirmation from Tx Thread */
   rc = wait_for_completion_interruptible_timeout(&pHddCtx->tx_sus_event_var, msecs_to_jiffies(200));

   if(!rc)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: Not able to suspend TX thread timeout happened", __func__);
      clear_bit(TX_SUSPEND_EVENT_MASK, &vosSchedContext->txEventFlag);

      return -1;
   }

   init_completion(&pHddCtx->rx_sus_event_var);

   /* Indicate Rx Thread to Suspend */
   set_bit(RX_SUSPEND_EVENT_MASK, &vosSchedContext->rxEventFlag);

   wake_up_interruptible(&vosSchedContext->rxWaitQueue);

   /* Wait for Suspend Confirmation from Rx Thread */
   rc = wait_for_completion_interruptible_timeout(&pHddCtx->rx_sus_event_var, msecs_to_jiffies(200));

   if(!rc)
   {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: Not able to suspend Rx thread timeout happened", __func__);

       clear_bit(RX_SUSPEND_EVENT_MASK, &vosSchedContext->rxEventFlag);

       /* Indicate Tx Thread to Resume */
       complete(&vosSchedContext->ResumeTxEvent);

       return -1;
   }

   init_completion(&pHddCtx->mc_sus_event_var);

   /* Indicate MC Thread to Suspend */
   set_bit(MC_SUSPEND_EVENT_MASK, &vosSchedContext->mcEventFlag);

   wake_up_interruptible(&vosSchedContext->mcWaitQueue);

   /* Wait for Suspend Confirmation from MC Thread */
   rc = wait_for_completion_interruptible_timeout(&pHddCtx->mc_sus_event_var, msecs_to_jiffies(200));

   if(!rc)
   {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: Not able to suspend MC thread timeout happened", __func__);

       clear_bit(MC_SUSPEND_EVENT_MASK, &vosSchedContext->mcEventFlag);

       /* Indicate Rx Thread to Resume */
       complete(&vosSchedContext->ResumeRxEvent);

       /* Indicate Tx Thread to Resume */
       complete(&vosSchedContext->ResumeTxEvent);

       return -1;
   }
   /* Set the Station state as Suspended */
   pHddCtx->isWlanSuspended = TRUE;

   return 0;
}

/*----------------------------------------------------------------------------

   @brief Function to resume the wlan driver.

   @param HDD_ADAPTER_HANDLE


   @return None

----------------------------------------------------------------------------*/
static void wlan_resume(hdd_adapter_t* pAdapter)
{
   pVosSchedContext vosSchedContext = NULL;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   //Get the global VOSS context.
   vosSchedContext = get_vos_sched_ctxt();

   if(!vosSchedContext) {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: Global VOS_SCHED context is Null",__func__);
      return;
   }

   /*
     Resuming Mc, Rx and Tx Thread as platform Driver is resuming.
   */
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Resuming Mc, Rx and Tx Thread",__func__);

   /* Indicate MC Thread to Resume */
   complete(&vosSchedContext->ResumeMcEvent);

   /* Indicate Rx Thread to Resume */
   complete(&vosSchedContext->ResumeRxEvent);

   /* Indicate Tx Thread to Resume */
   complete(&vosSchedContext->ResumeTxEvent);

   /* Set the Station state as Suspended */
   pHddCtx->isWlanSuspended = FALSE;
}

/*----------------------------------------------------------------------------

   @brief Function to suspend the wlan driver.
   This function will get called by platform driver Suspend on System Suspend

   @param dev    platform_func_device


   @return None

----------------------------------------------------------------------------*/
int hddDevSuspendHdlr(struct device *dev)
{
   int ret = 0;
   hdd_adapter_t* pAdapter = NULL;
   hdd_context_t* pHddCtx = NULL;

   pAdapter =  (hdd_adapter_t*)wcnss_wlan_get_drvdata(dev);
   pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: WLAN suspended by platform driver",__func__);

   /* Get the HDD context */
   if(!pAdapter) {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return 0;
   }

   if(pHddCtx->isWlanSuspended == TRUE)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: WLAN is alredy in suspended state",__func__);
      return 0;
   }

   /* Suspend the wlan driver */
   ret = wlan_suspend(pAdapter);
   if(ret != 0)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: Not able to suspend wlan",__func__);
      return ret;
   }

   return 0;
}

/*----------------------------------------------------------------------------

   @brief Function to resume the wlan driver.
   This function will get called by platform driver Resume on System Resume 

   @param dev    platform_func_device


   @return None

----------------------------------------------------------------------------*/
int hddDevResumeHdlr(struct device *dev)
{
   hdd_adapter_t* pAdapter = NULL;
   hdd_context_t* pHddCtx = NULL;

   pAdapter =  (hdd_adapter_t*)wcnss_wlan_get_drvdata(dev);
   pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_INFO, "%s: WLAN being resumed by Android OS",__func__);

   if(pHddCtx->isWlanSuspended != TRUE)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD,VOS_TRACE_LEVEL_FATAL,"%s: WLAN is alredy in resumed state",__func__);
      return 0;
   }

   /* Resume the wlan driver */
   wlan_resume(pAdapter);

   return 0;
}



/*----------------------------------------------------------------------------

   @brief Registration function.
        Register the suspend, resume callabcks with platform driver

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration Success
        VOS_STATUS_E_FAILURE     Registration Failure

----------------------------------------------------------------------------*/
VOS_STATUS hddRegisterPmOps(hdd_adapter_t *pAdapter)
{
   struct dev_pm_ops pm_ops;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   // Set the private data for the device to our adapter.
   wcnss_wlan_set_drvdata(pHddCtx->parent_dev, pAdapter);

   pm_ops.suspend = hddDevSuspendHdlr;
   pm_ops.resume = hddDevResumeHdlr;
#ifndef FEATURE_R33D
   wcnss_wlan_register_pm_ops(pHddCtx->parent_dev, &pm_ops);
#endif /* FEATURE_R33D */
   return VOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------

   @brief Deregistration function.
        Deregister the suspend, resume callabcks with platform driver

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration Success
        VOS_STATUS_E_FAILURE     Registration Failure

----------------------------------------------------------------------------*/
VOS_STATUS hddDeregisterPmOps(hdd_adapter_t *pAdapter)
{
   struct dev_pm_ops pm_ops;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
   // Set the private data for the device to our adapter.
   wcnss_wlan_set_drvdata(pHddCtx->parent_dev, pAdapter);

   pm_ops.suspend = NULL;
   pm_ops.resume = NULL;
#ifndef FEATURE_R33D
   wcnss_wlan_register_pm_ops(pHddCtx->parent_dev, &pm_ops);
#endif /* FEATURE_R33D */
   return VOS_STATUS_SUCCESS;
}

