/**=========================================================================

  @file  wlan_qct_sal.c

  @brief WLAN SDIO ABSTRACTION LAYER EXTERNAL API FOR LINUX SPECIFIC PLATFORM

   This file contains the external API exposed by the wlan SDIO abstraction layer module.
   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
   Qualcomm Confidential and Proprietary
========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when           who        what, where, why
--------    ---         ----------------------------------------------------------

===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <wlan_hdd_includes.h>
#include <wlan_hdd_power.h>
#include <wlan_qct_sal.h>
#include <wlan_sal_misc.h> // Linux specific includes
#include <wlan_qct_hal.h>
#include <wlan_bal_misc.h> // Linux specific includes
#include <linux/mmc/sdio.h>
#include <vos_power.h>
#include <vos_sched.h>
#include <vos_api.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
/* The below funxtion definintion is in librasdioif driver */
extern void libra_sdio_set_clock(struct sdio_func *func, unsigned int clk_freq);
extern void libra_sdio_get_card_id(struct sdio_func *func, unsigned short *card_id);

#ifdef MSM_PLATFORM /* Specific to Android Platform */
extern int sdio_reset_comm(struct mmc_card *card);
#endif /* MSM_PLATFORM */
/*-------------------------------------------------------------------------
 * Global variables.
 *-------------------------------------------------------------------------*/
static salHandleType *gpsalHandle;
static v_U8_t gSDCmdFailed = 0;

/*----------------------------------------------------------------------------

   @brief Function to suspend the wlan driver.

   @param HDD_ADAPTER_HANDLE


   @return None

----------------------------------------------------------------------------*/
static int wlan_suspend(hdd_adapter_t* pAdapter)
{
   int rc = 0;

   pVosSchedContext vosSchedContext = NULL;

   /* Get the global VOSS context */
   vosSchedContext = get_vos_sched_ctxt();

   if(!vosSchedContext) {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: Global VOS_SCHED context is Null",__func__);
      return -1;
   }

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Enter",__func__);

   /*
     Suspending MC Thread and Tx Thread as the SDIO driver is going to Suspend.
   */
   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO, "%s: Suspending Mc and Tx Threads",__func__);

     INIT_COMPLETION(pAdapter->mc_sus_event_var);

   /* Indicate Mc Thread to Suspend */
   set_bit(MC_SUSPEND_EVENT_MASK, &vosSchedContext->mcEventFlag);

   wake_up_interruptible(&vosSchedContext->mcWaitQueue);

   /* Wait for Suspend Confirmation from Mc Thread */
   rc = wait_for_completion_interruptible_timeout(&pAdapter->mc_sus_event_var, msecs_to_jiffies(200));

   if(!rc)
   {
       VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: Not able to suspend MC thread timeout happened", __func__);

       clear_bit(MC_SUSPEND_EVENT_MASK, &vosSchedContext->mcEventFlag);
  
       return -1;
   }

   /* If not associated, make sure that STA is in IMPS before suspend happens. 
    * Otherwise power consumption will be high in suspend mode */
   if(!hdd_is_apps_power_collapse_allowed(pAdapter))
   {
       /* Indicate MC Thread to Resume */
       complete(&vosSchedContext->ResumeMcEvent);
       hdd_abort_mac_scan(pAdapter);
       /* Fail this suspend */
       return -1;
   }

   /* Set the Mc Thread as Suspended */
   pAdapter->isMcThreadSuspended= TRUE;

     INIT_COMPLETION(pAdapter->tx_sus_event_var);

   /* Indicate Tx Thread to Suspend */
   set_bit(TX_SUSPEND_EVENT_MASK, &vosSchedContext->txEventFlag);

   wake_up_interruptible(&vosSchedContext->txWaitQueue);

   /* Wait for Suspend Confirmation from Tx Thread */
   rc = wait_for_completion_interruptible_timeout(&pAdapter->tx_sus_event_var, msecs_to_jiffies(200));

   if(!rc)
   {
       VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: Not able to suspend TX thread timeout happened", __func__);
       clear_bit(TX_SUSPEND_EVENT_MASK, &vosSchedContext->txEventFlag);

       /* Indicate Mc Thread to Resume */
       complete(&vosSchedContext->ResumeMcEvent);

       /* Set the MC Thread as Resumed */
       pAdapter->isMcThreadSuspended= FALSE;
       return -1;
   }

   /* Set the Tx Thread as Suspended */
   pAdapter->isTxThreadSuspended= TRUE;


   /* Set the Station state as Suspended */
   pAdapter->isWlanSuspended = TRUE;

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Exit",__func__);

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

   //Get the global VOSS context.
   vosSchedContext = get_vos_sched_ctxt();

   if(!vosSchedContext) {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: Global VOS_SCHED context is Null",__func__);
      return;
   }

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Enter",__func__);

   vos_sleep (10);

   /*
     Resuming Mc and Tx Thread as SDIO Driver is resuming.
   */
   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO, "%s: Resuming Mc and Tx Thread",__func__);

   /* Indicate MC Thread to Resume */
   complete(&vosSchedContext->ResumeMcEvent);

   /* Set the Mc Thread as Resumed */
   pAdapter->isMcThreadSuspended= FALSE;

   /* Indicate Tx Thread to Resume */
   complete(&vosSchedContext->ResumeTxEvent);

   /* Set the Tx Thread as Resumed */
   pAdapter->isTxThreadSuspended= FALSE;

   /* Set the Station state as Suspended */
   pAdapter->isWlanSuspended = FALSE;

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Exit",__func__);
}

/*----------------------------------------------------------------------------

   @brief Function to suspend the wlan driver.
   This function will get called by SDIO driver Suspend on System Suspend

   @param SD_DEVICE_HANDLE sdio_func_device


   @return None

----------------------------------------------------------------------------*/
int wlan_sdio_suspend_hdlr(struct sdio_func* sdio_func_dev)
{
   int ret = 0;
   hdd_adapter_t* pAdapter = NULL;
   pAdapter =  (hdd_adapter_t*)libra_sdio_getprivdata(sdio_func_dev);

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO, "%s: WLAN suspended by SDIO",__func__);

   /* Get the HDD context */
   if(!pAdapter) 
   {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
      return 0;
   }

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Enter",__func__);

   if(pAdapter->isWlanSuspended == TRUE)
   {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: WLAN is alredy in suspended state",__func__);
      return 0;
   }

   if(pAdapter->isLogpInProgress == TRUE) {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: Trying to Suspend While LogP is in progress",__func__);
      return -1;
   }
   
   /* Suspend the wlan driver */
   ret = wlan_suspend(pAdapter);
   if(ret != 0)
   {   
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: Not able to suspend wlan",__func__);
      return ret;
   }

   if(pAdapter->isLogpInProgress == TRUE) {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: After Suspend LogP is in progress",__func__);
      wlan_resume(pAdapter);
      return -1;
   }

   return 0;
}

/*----------------------------------------------------------------------------

   @brief Function to resume the wlan driver.
   This function will get called by SDIO driver Resume on System Resume 

   @param SD_DEVICE_HANDLE sdio_func_device 


   @return None

----------------------------------------------------------------------------*/
void wlan_sdio_resume_hdlr(struct sdio_func* sdio_func_dev)
{
   hdd_adapter_t* pAdapter = NULL;
   pAdapter =  (hdd_adapter_t*)libra_sdio_getprivdata(sdio_func_dev);

   if (!pAdapter)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s pAdapter is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(0);
      return;
   }
   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR, "%s: Enter",__func__);

   VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_INFO, "%s: WLAN being resumed by Android OS",__func__);

   if(pAdapter->isWlanSuspended != TRUE)
   {
      VOS_TRACE(VOS_MODULE_ID_SAL,VOS_TRACE_LEVEL_FATAL,"%s: WLAN is alredy in resumed state",__func__);
      return;
   }

   /* Resume the wlan driver */
   wlan_resume(pAdapter);
}

/*----------------------------------------------------------------------------

   @brief Hardware Interrupt handle

   @param SD_DEVICE_HANDLE  hDevice
   @param DWORD             pAdapter

   @return General status code
           VOS_STATUS_SUCCESS      Handle sunccess
           TBD

----------------------------------------------------------------------------*/
void salRxInterruptCB
(
   struct sdio_func *sdio_func_dev
)
{
   v_PVOID_t pAdapter = NULL;

   if (vos_is_logp_in_progress(VOS_MODULE_ID_SAL, NULL)) 
   {

      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, 
             "%s LOGP in progress. Ignore Interrupt", __func__);
#ifdef MSM_PLATFORM
      /* Disable SDIO IRQ since we are in LOGP state */
      libra_enable_sdio_irq(sdio_func_dev, 0);
#endif
      return;
   }
   
   VOS_ASSERT(sdio_func_dev);
   VOS_ASSERT(gpsalHandle);


   if (!gpsalHandle->sscCBs.interruptCB)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle->sscCBs.interruptCB is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle->sscCBs.interruptCB);
      return;
   }

   if (!gpsalHandle->sscCBs.sscUsrData)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle->sscCBs.sscUsrData is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle->sscCBs.sscUsrData);
      return;
   }

   pAdapter =  libra_sdio_getprivdata(sdio_func_dev);
   ((hdd_adapter_t *)pAdapter)->pid_sdio_claimed = current->pid;
   atomic_inc(&((hdd_adapter_t *)pAdapter)->sdio_claim_count);

   /* Release SDIO lock acquired by irq_sdio task before calling salRxInterruptCB. 
    * As we are anyway acquiring the same lock again while doing SDIO operation. 
    * Reason for releasing it here, to have synchronization of SDIO lock with other 
    * locks used in the driver. This is hack, we need to come up with proper soultion
    *  of this.
    */
   sd_release_host(sdio_func_dev);

   gpsalHandle->sscCBs.interruptCB(pAdapter, gpsalHandle->sscCBs.sscUsrData);

   /* Acquring the SDIO lock again as we release earlier. This is done so that
    * irq_sdio thread can release it after returning from here. 
    */
   sd_claim_host(sdio_func_dev);

   ((hdd_adapter_t *)pAdapter)->pid_sdio_claimed = 0; 
   atomic_dec(&((hdd_adapter_t *)pAdapter)->sdio_claim_count);
}

/*----------------------------------------------------------------------------

   @brief Function to workaround an ASIC CR.
   This is a worakound a Liba POR issue whe PMU comes up too early before SIF. This HW
   issue is slated to be fixed in Libra 2.0

   @param none

   @param v_U32_t   none

   @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
void PmuFixWorkAround(int *err_ret)
{
   v_U8_t                     regValue;
   int                        save_func_num = 0;

   VOS_ASSERT(NULL != gpsalHandle);

   SENTER();

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
   gpsalHandle->sdio_func_dev = libra_getsdio_funcdev();
   save_func_num = gpsalHandle->sdio_func_dev->num;
   gpsalHandle->sdio_func_dev->num = 0;

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO,"sdio_dev=%08x\n", (unsigned int)(gpsalHandle->sdio_func_dev));

   libra_sdiocmd52( gpsalHandle->sdio_func_dev, QWLAN_SIF_BAR4_WLAN_CONTROL_REG_REG, &regValue,
         0, err_ret);

   if (VOS_STATUS_SUCCESS != *err_ret)
   {
      VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR,"Err1 %d", *err_ret);
      gpsalHandle->sdio_func_dev->num = save_func_num;
      *err_ret = VOS_STATUS_E_FAILURE;
      return;
   }

   // When this bit is set to 0, WLAN will be placed in reset state.
   if ((regValue & QWLAN_SIF_BAR4_WLAN_STATUS_REG_PMU_BLOCKED_BIT_MASK) == 0)
   {
      // WLAN is somehow already awaken for whatever reason
      libra_sdiocmd52( gpsalHandle->sdio_func_dev, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &regValue,
         0, err_ret);
      if( VOS_STATUS_SUCCESS != *err_ret )
      {
         SMSGERROR("SAL_Start Read reg SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG Fail", 0, 0, 0);

         VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR,"Err2");
         gpsalHandle->sdio_func_dev->num = save_func_num;
         *err_ret = VOS_STATUS_E_FAILURE;
         return;
      }

      regValue |= 0x01;
      libra_sdiocmd52( gpsalHandle->sdio_func_dev, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &regValue,
         1, err_ret);
      if( VOS_STATUS_SUCCESS != *err_ret)
      {
         SMSGERROR("SAL_Start Write reg SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG Fail", 0, 0, 0);
         VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR,"Err3\n");
         gpsalHandle->sdio_func_dev->num = save_func_num;
         *err_ret = VOS_STATUS_E_FAILURE;
         return;
      }

      vos_sleep (2);

      regValue &= ~0x01;
      libra_sdiocmd52( gpsalHandle->sdio_func_dev, QWLAN_SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG, &regValue,
         1, err_ret);
      if( VOS_STATUS_SUCCESS != *err_ret)
      {
         SMSGERROR("SAL_Start Write reg SIF_BAR4_WLAN_PWR_SAVE_CONTROL_REG_REG Fail", 0, 0, 0);
         VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_ERROR,"Err4\n");
         gpsalHandle->sdio_func_dev->num = save_func_num;
         *err_ret = VOS_STATUS_E_FAILURE;
         return;
      }
   }
   gpsalHandle->sdio_func_dev->num = save_func_num;
   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_INFO,"Pmu: Success\n");
   *err_ret = 0;
   return;
}


/*----------------------------------------------------------------------------

   @brief Open SAL Module.
        Allocate internal resources, Initialize LOCK element,
        and allocate SDIO handle

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param v_U32_t   sdBusDCtxt
        Platform specific device context

   @return General status code
        VOS_STATUS_SUCCESS      Open Success
        VOS_STATUS_E_NOMEM      Open Fail, Resource alloc fail
        VOS_STATUS_E_FAILURE    Open Fail, Unknown reason

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Open
(
   v_PVOID_t pAdapter,
   v_U32_t   sdBusDCtxt
)
{
   VOS_STATUS        status    = VOS_STATUS_SUCCESS;

   if (!pAdapter)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s pAdapter is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(NULL != pAdapter);
      return VOS_STATUS_E_FAILURE;
   }

   SENTER();

   status = vos_alloc_context(pAdapter, VOS_MODULE_ID_SAL, (void *)&gpsalHandle, sizeof(salHandleType));

   VOS_ASSERT(VOS_STATUS_SUCCESS == status);
   if (!gpsalHandle)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle);
      return VOS_STATUS_E_FAILURE;
   }

   // Initialize the mutex
   mutex_init(&gpsalHandle->lock);
   spin_lock_init(&gpsalHandle->spinlock);

   /* Reset the CMD Failed variable here */
   gSDCmdFailed = 0;

   SEXIT();
   return VOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------

   @brief Start SAL module.
        Probing SDIO interface, get and store card information

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Start Success
        VOS_STATUS_E_FAILURE     Start Fail, BAL Not open yet
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Start
(
   v_PVOID_t pAdapter
)
{
   WLANSAL_CardInfoType       cardConfig;
#define VOS_GET_BAL_CTXT(a)            vos_get_context(VOS_MODULE_ID_BAL, a)
   balHandleType              *balHandle = (balHandleType *)VOS_GET_BAL_CTXT(pAdapter);

   memset( &cardConfig, 0, sizeof(cardConfig));
   if (!gpsalHandle)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(NULL != gpsalHandle);
      return VOS_STATUS_E_FAILURE;
   }

   /* Reset the CMD Failed variable here */
   gSDCmdFailed = 0;

   if (!balHandle)
   {
      VOS_ASSERT(0);
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s balHandle is dereferencing to NULL pointer\n", __func__);
      return VOS_STATUS_E_FAILURE;
   }
   SENTER();

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
   // Function for Rx handler and
   // 2nd function pointer which is a
   // workaround for PMU problems on ASIC CR: 185723.
   // Liba POR issue when PMU comes up too early before SIF. This HW
   // issue is slated to be fixed in Libra 2.0
   if (libra_sdio_configure( salRxInterruptCB, PmuFixWorkAround, LIBRA_FUNC_ENABLE_TIMEOUT, WLANSAL_MAX_BLOCK_SIZE))
   {
      return VOS_STATUS_E_FAILURE;
   }

   // Set the global sdio func dev handle
   gpsalHandle->sdio_func_dev = libra_getsdio_funcdev();
   balHandle->sdio_func_dev   = libra_getsdio_funcdev();

   cardConfig.blockSize = WLANSAL_MAX_BLOCK_SIZE;
   WLANSAL_CardInfoUpdate(pAdapter, &cardConfig);

   gpsalHandle->isINTEnabled = VOS_TRUE;

#ifndef LIBRA_LINUX_PC
   /* Register with SDIO driver as client for Suspend/Resume */
   libra_sdio_configure_suspend_resume(wlan_sdio_suspend_hdlr, wlan_sdio_resume_hdlr);
#endif /* LIBRA_LINUX_PC */
   SEXIT();

   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief Stop SAL module.
        Initialize internal resources

   @param  v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Stop Success
        VOS_STATUS_E_FAILURE     Stop Fail, BAL not started
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Stop
(
   v_PVOID_t pAdapter
)
{
   if (!gpsalHandle)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle);
      return VOS_STATUS_E_FAILURE;
   }


   SENTER();

#ifndef LIBRA_LINUX_PC
   /* Register with SDIO driver as client for Suspend/Resume */
   libra_sdio_configure_suspend_resume(NULL, NULL);
#endif /* LIBRA_LINUX_PC */

   // release sdio irq claim from our driver
   libra_sdio_deconfigure(gpsalHandle->sdio_func_dev);

   SEXIT();

   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief Close SAL module.
        Free internal resources already allocated.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Close Success
        VOS_STATUS_E_FAILURE     Close Fail, BAL not open
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Close
(
   v_PVOID_t pAdapter
)
{
   SENTER();

   // Just warn if mutex is still locked. 
   if (mutex_is_locked(&gpsalHandle->lock))
   {
      VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_WARN,"%s: Sal lock not released.\n");
   }

   vos_free_context(pAdapter, VOS_MODULE_ID_SAL, gpsalHandle);

   SEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief
      - TBD

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Reset
(
   v_PVOID_t pAdapter
)
{

   int  err_ret = 0;
   v_U8_t regValue = 0;
   
   SENTER();
   // TBD 

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, " Doing SDIO_CCCR_ABORT");
   
   // Get the lock, going native
   sd_claim_host(gpsalHandle->sdio_func_dev);

   gpsalHandle->sdio_func_dev->num = 0;
   libra_sdiocmd52( gpsalHandle->sdio_func_dev, SDIO_CCCR_ABORT, &regValue, 0, &err_ret);

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, " SDIO_CCCR_ABORT Read regValue = 0x%x err_ret = %d", regValue, err_ret);
   
   regValue |= 0x08;
   libra_sdiocmd52( gpsalHandle->sdio_func_dev, SDIO_CCCR_ABORT, &regValue, 1, &err_ret);
   gpsalHandle->sdio_func_dev->num = 1;

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, " SDIO_CCCR_ABORT Write regValue = 0x%x err_ret = %d", regValue, err_ret);

   // Release lock
   sd_release_host(gpsalHandle->sdio_func_dev);

   VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, " Done SDIO_CCCR_ABORT");
   
   //
   SEXIT();
   return VOS_STATUS_SUCCESS;
}

/*=========================================================================
 * END Interactions with BAL
 *=========================================================================*/

/*=========================================================================
 * General Functions
 *=========================================================================*/
/*----------------------------------------------------------------------------

   @brief SDIO CMD52 Read or write one byte at a time

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param  WLANSAL_Cmd52ReqType *cmd52Req
           WLANSAL_BUS_DIRECTION_TYPE BUS direction, Read or write direction
           v_U32_t                    Target address
           v_PVOID_t                  Data pointer
           v_PVOID_t                  SDIO internal handle
   @return General status code
        VOS_STATUS_SUCCESS       Read or write success
        VOS_STATUS_E_INVAL       cmd is not valid
        VOS_STATUS_E_FAILURE     SAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Cmd52
(
   v_PVOID_t             pAdapter,
   WLANSAL_Cmd52ReqType *cmd52Req
)
{
   int  err_ret = 0;
   int  save_function_num = 0;
   struct sdio_func *sdio_func_dev = NULL;
   
   VOS_ASSERT(gpsalHandle);
   VOS_ASSERT(cmd52Req);

   SENTER();

   //Get the SDIO func device
   sdio_func_dev = libra_getsdio_funcdev();

   if(NULL == sdio_func_dev) {   	
   	VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: sdio_func is null!!!",__func__);
   	return VOS_STATUS_E_FAILURE;
   }
  

   // Get the lock, going native
   sd_claim_host(gpsalHandle->sdio_func_dev);

   /* Block any more SD commands after the 1st failure */
   if (gSDCmdFailed)
   {
      sd_release_host(gpsalHandle->sdio_func_dev);
      return VOS_STATUS_E_FAILURE;
   } 
   
   save_function_num = gpsalHandle->sdio_func_dev->num;
   gpsalHandle->sdio_func_dev->num = 0; // Assign to 0.
   libra_sdiocmd52( gpsalHandle->sdio_func_dev, cmd52Req->address, cmd52Req->dataPtr,
         cmd52Req->busDirection, &err_ret);
   gpsalHandle->sdio_func_dev->num = save_function_num;

   VOS_ASSERT(0 == err_ret);

   if(err_ret)
   {
     VOS_TRACE(VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: addr 0x%x err = %d",__func__, cmd52Req->address, err_ret);
     gSDCmdFailed = 1;
   }
   
   // Release lock
   sd_release_host(gpsalHandle->sdio_func_dev);

   SEXIT();

   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief SDIO CMD53 Read or write multiple bytes at a time.
        Read or write can be happen with synchronous functions call or
        asynchronous function call, depends on clients request.
   Currently all calls are SYNC.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param  WLANSAL_Cmd53ReqType *cmd53Req
           WLANSAL_BUS_DIRECTION_TYPE BUS direction, Read or write direction
           WLANSAL_BUS_MODE_TYPE      Byte or block mode
           WLANSAL_BUS_EAPI_TYPE      Sync or Async call
           WLANSAL_BusCompleteCBType  If bus request is done,
                                      this callback have to be issued with
                                      asynchronous function call
           v_U32_t                    Target address
           v_U16_t                    Read or write data size
           v_PVOID_t                  Data pointer
           v_PVOID_t                  SDIO internal handle

   @return General status code
        VOS_STATUS_SUCCESS       Read or write success
        VOS_STATUS_E_INVAL       cmd is not valid
        VOS_STATUS_E_FAILURE     SAL is not ready

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_Cmd53
(
   v_PVOID_t             pAdapter,
   WLANSAL_Cmd53ReqType *cmd53Req
)
{
   v_U16_t              transferUnits;
   int                  err_ret=0;
   v_U32_t              numBlocks = 0;
   v_U32_t              numBytes = 0;
   v_U8_t               *temp_dataPtr;           // Data pointer
   v_U32_t              temp_address;           // Target address
   struct sdio_func *sdio_func_dev = NULL;   

   VOS_ASSERT(gpsalHandle);
   VOS_ASSERT(cmd53Req);
   VOS_ASSERT(cmd53Req->dataSize);

   SENTER();  
   
   if (0 == cmd53Req->dataSize) 
   {
      SMSGERROR("CMD53 Data size 0, direction %d, Mode %d, address 0x%x",
         cmd53Req->busDirection, cmd53Req->mode, cmd53Req->address);
      SEXIT();
      return VOS_STATUS_E_FAILURE;
   }
	
   // Get the lock, going native
   sd_claim_host(gpsalHandle->sdio_func_dev);

   /* Block any more SD commands after the 1st failure */
   if (gSDCmdFailed)
   {
      sd_release_host(gpsalHandle->sdio_func_dev);
      return VOS_STATUS_E_FAILURE;
   } 
   
   if (vos_is_logp_in_progress(VOS_MODULE_ID_SAL, NULL)) {
        VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: LOGP in progress. Ignore!!! direction %d, Mode %d, address 0x%x",
                        __func__,cmd53Req->busDirection, cmd53Req->mode, cmd53Req->address);
        // Release lock
        sd_release_host(gpsalHandle->sdio_func_dev);
        return VOS_STATUS_E_FAILURE;
   }
   
   //Get the SDIO func device
   sdio_func_dev = libra_getsdio_funcdev();

   if(NULL == sdio_func_dev) {   	
   	VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s: sdio_func is null Doing WatchDog Chip Reset!!!",__func__);
   	err_ret = VOS_STATUS_E_FAILURE;
   	goto watchdog_chip_reset;
   }

   // The host driver internally takes care of chopping it into blocks.
   transferUnits = (v_U16_t)cmd53Req->dataSize;

   if (WLANSAL_DIRECTION_READ == cmd53Req->busDirection)
   {
      if (WLANSAL_ADDRESS_FIXED == cmd53Req->addressHandle) {
         // Will only copy 1 block or less.
         err_ret = libra_sdio_readsb(gpsalHandle->sdio_func_dev,
                                  cmd53Req->dataPtr, cmd53Req->address, transferUnits);
      }
      else {
         err_ret = libra_sdio_memcpy_fromio(gpsalHandle->sdio_func_dev,
                                  cmd53Req->dataPtr, cmd53Req->address, transferUnits);
      }
   }
   else if(WLANSAL_DIRECTION_WRITE == cmd53Req->busDirection)
   {
      // For write we need to make sure that a Dummy CMD52 is sent for every CMD53.
      // So lets do some of the Bus Drivers work here.


      numBlocks = transferUnits / (gpsalHandle->sdio_func_dev->cur_blksize);
      numBytes  = transferUnits % (gpsalHandle->sdio_func_dev->cur_blksize);

      if (numBlocks != 0) {
         if (WLANSAL_ADDRESS_FIXED == cmd53Req->addressHandle) {
            err_ret = libra_sdio_writesb(gpsalHandle->sdio_func_dev,
               cmd53Req->address, cmd53Req->dataPtr, numBlocks*gpsalHandle->sdio_func_dev->cur_blksize);
         }
         else {
            err_ret = libra_sdio_memcpy_toio(gpsalHandle->sdio_func_dev, cmd53Req->address,
               cmd53Req->dataPtr, numBlocks*gpsalHandle->sdio_func_dev->cur_blksize);
         }
         if (err_ret)
         {
            SMSGFATAL("%s: Value of ERROR err_ret #1 = %d\n", __func__, err_ret, 0);
         }
         
      }

      if (numBytes != 0) {

         temp_address            = cmd53Req->address + (gpsalHandle->sdio_func_dev->cur_blksize * numBlocks);
         temp_dataPtr            = (v_U8_t *)(cmd53Req->dataPtr + ( gpsalHandle->sdio_func_dev->cur_blksize * numBlocks));

         if (WLANSAL_ADDRESS_FIXED == cmd53Req->addressHandle) {
            err_ret = libra_sdio_writesb(gpsalHandle->sdio_func_dev,
               temp_address, temp_dataPtr, numBytes);
         }
         else {
            err_ret = libra_sdio_memcpy_toio(gpsalHandle->sdio_func_dev, temp_address,
               temp_dataPtr, numBytes);
         }
         if (err_ret)
         {
            SMSGFATAL("%s: Value of ERROR err_ret #1 = %d\n", __func__, err_ret, 0);
         }
        
      }
   }

watchdog_chip_reset:
   if (err_ret)
   {
      SMSGFATAL("%s: Value of ERROR err_ret = %d, Data Size = %d\n", __func__, err_ret, cmd53Req->dataSize);
      SMSGFATAL("CMD53 direction %d, Mode %d, address 0x%x", 
         cmd53Req->busDirection, cmd53Req->mode, cmd53Req->address); 
      gSDCmdFailed = 1;
      vos_chipReset(NULL, VOS_FALSE, NULL, NULL, VOS_CHIP_RESET_CMD53_FAILURE);      
      // Release lock
      sd_release_host(gpsalHandle->sdio_func_dev);
      SEXIT();
      return err_ret;
   }

   // Release lock
   sd_release_host(gpsalHandle->sdio_func_dev);

   SEXIT();
   return VOS_STATUS_SUCCESS;
}


/*=========================================================================
 * END General Functions
 *=========================================================================*/
/*=========================================================================
 * Interactions with SSC
 *=========================================================================*/

/*----------------------------------------------------------------------------

   @brief Register SSC callback functions to SAL.
        Just after SAL open DONE, callback functions have to be registered.
        Registration functions are TX complete, RX complete and interrupt happen.
        Fatal error callback function is TBD yet.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param WLANSAL_SscRegType
           WLANSAL_InterruptCBType      interrupt CB function PTR
           WLANSAL_BusCompleteCBType    Bus complete CB function PTR
           v_PVOID_t                    SSC handle
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_RegSscCBFunctions
(
   v_PVOID_t           pAdapter,
   WLANSAL_SscRegType *sscReg
)
{

   if (!sscReg)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s sscReg is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(sscReg);
      return VOS_STATUS_E_FAILURE;
   }
   if (!gpsalHandle)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle);
      return VOS_STATUS_E_FAILURE;
   }
   SENTER();

   gpsalHandle->sscCBs.interruptCB   = sscReg->interruptCB;
   gpsalHandle->sscCBs.busCompleteCB = sscReg->busCompleteCB;
   gpsalHandle->sscCBs.sscUsrData    = sscReg->sscUsrData;

   SEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief De Register SSC callback functions from SAL.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @return General status code
        VOS_STATUS_SUCCESS       Registration success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_DeregSscCBFunctions
(
   v_PVOID_t           pAdapter
)
{

   if (!gpsalHandle)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s gpsalHandle is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(gpsalHandle); 
      return VOS_STATUS_E_FAILURE;
   }

   SENTER();

   gpsalHandle->sscCBs.interruptCB   = NULL;
   gpsalHandle->sscCBs.busCompleteCB = NULL;

   SEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief Query card information.
        Card information will be got during WLANSAL_Start.
        Card information is stored SAL internal structure,

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param WLANSAL_CardInfoType *cardInfo
           WLANSAL_CARD_INTERFACE_TYPE  1bit or 4 bit interface
           v_U32_t                      Card clock rate
           v_U32_t                      Card block size
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Query success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_CardInfoQuery
(
   v_PVOID_t             pAdapter,
   WLANSAL_CardInfoType *cardInfo
)
{

   VOS_ASSERT(gpsalHandle);

   if (!cardInfo)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s cardInfo is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(cardInfo);
      return VOS_STATUS_E_FAILURE;
   }
   SENTER();

   memcpy(cardInfo, &gpsalHandle->cardInfo, sizeof(WLANSAL_CardInfoType));

   SMSGINFO("%s: Query CR %dKHz, BS %d", __func__, cardInfo->clockRate, cardInfo->blockSize);

   SEXIT();
   return VOS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------

   @brief Update card information.

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param WLANSAL_CardInfoType *cardInfo
           WLANSAL_CARD_INTERFACE_TYPE  1bit or 4 bit interface
           v_U32_t                      Card clock rate
           v_U32_t                      Card block size
           v_PVOID_t                    SAL handle

   @return General status code
        VOS_STATUS_SUCCESS       Update success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_CardInfoUpdate
(
   v_PVOID_t             pAdapter,
   WLANSAL_CardInfoType *cardInfo
)
{
   VOS_ASSERT(gpsalHandle);

   if (!cardInfo)
   {
      VOS_TRACE( VOS_MODULE_ID_SAL, VOS_TRACE_LEVEL_FATAL, "%s cardInfo is dereferencing to NULL pointer\n", __func__);
      VOS_ASSERT(cardInfo);
      return VOS_STATUS_E_FAILURE;
   }

   SENTER();

   // Get the lock, going native
   SGETLOCK(__func__, &gpsalHandle->lock);

   if(gpsalHandle->cardInfo.clockRate != cardInfo->clockRate)
   {
      gpsalHandle->cardInfo.clockRate = cardInfo->clockRate;
   }

   if(gpsalHandle->cardInfo.blockSize != cardInfo->blockSize)
   {
      gpsalHandle->cardInfo.blockSize = cardInfo->blockSize;
   }

   // Release lock
   SRELEASELOCK(__func__, &gpsalHandle->lock);

   SEXIT();
   return VOS_STATUS_SUCCESS;
}

/*=========================================================================
 * END Interactions with SSC
 *=========================================================================*/

/*----------------------------------------------------------------------------

   @brief Set Card presence status path
          Card present status changed notification path
          If Card is removed from slot or put into slot
          Notification may routed to SAL or SDBUS

   @param v_PVOID_t pAdapter
        Global adapter handle

   @param WLANSAL_NOTF_PATH_T   path
        Notification Path, it may SAL or SDBUS

   @return General status code
        VOS_STATUS_SUCCESS       Update success
        VOS_STATUS_E_RESOURCES   SAL resources are not ready
        VOS_STATUS_E_INVAL       Invalid argument

----------------------------------------------------------------------------*/
VOS_STATUS WLANSAL_SetCardStatusNotfPath
(
   v_PVOID_t             pAdapter,
   WLANSAL_NOTF_PATH_T   path
)
{
   return VOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------

   @brief API exported from SAL to set the SD clock frequency. This needs to
          have the libra_sdio_set_clock API exported from librasdioif driver and
          mmc_set_clock API exported from the kernel
   @param hz - Frequency to be set

   @return void

----------------------------------------------------------------------------*/
void WLANSAL_SetSDIOClock
(
    v_PVOID_t    pAdapter,
    v_UINT_t     hz
)
{
  VOS_ASSERT(NULL != gpsalHandle);
  libra_sdio_set_clock(gpsalHandle->sdio_func_dev, hz);
}

/*----------------------------------------------------------------------------

   @brief API exported from SAL to get the vendor specific card ID. This needs to
          have the libra_sdio_get_card_id API exported from librasdioif driver
   @param *card_id - To receive the card id

   @return void

----------------------------------------------------------------------------*/
void WLANSAL_GetSDIOCardId
(
    v_PVOID_t   pAdapter,
    v_U16_t     *sdioCardId
)
{
  VOS_ASSERT(NULL != gpsalHandle);
  libra_sdio_get_card_id(gpsalHandle->sdio_func_dev, sdioCardId);
}
