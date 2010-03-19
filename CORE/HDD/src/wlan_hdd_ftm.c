/**========================================================================

  \file  wlan_hdd_ftm.c

  \brief This file contains the WLAN factory test mode implementation

   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

   Qualcomm Confidential and Proprietary.

  ========================================================================*/

/**=========================================================================

                       EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$   $DateTime: $ $Author: $


  when        who    what, where, why
  --------    ---    --------------------------------------------------------
  04/5/09     Shailender     Created module.

  ==========================================================================*/


#ifdef ANI_MANF_DIAG

#include <vos_mq.h>
#include "vos_sched.h"
#include <vos_api.h>
#include "sirTypes.h"
#include "halTypes.h"
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "sme_Api.h"
#include "macInitApi.h"
#include "wlan_qct_sal.h"
#include "wlan_qct_bal.h"
#include "wlan_qct_sys.h"
#include "wlan_qct_tl.h"
#include "wlan_hdd_misc.h"
#include "i_vos_packet.h"
#include "vos_nvitem.h"
#include "wlan_hdd_main.h"

v_VOID_t
ftm_vos_sys_probe_thread_cback
(
  v_VOID_t *pUserData
)
{
    pVosContextType pVosContext= (pVosContextType)pUserData;
    if (vos_event_set(&pVosContext->ProbeEvent)!= VOS_STATUS_SUCCESS)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
         "%s: vos_event_set failed", __FUNCTION__);
        return;
    }
} /* vos_sys_probe_thread_cback() */


void HEXDUMP(char *s0, char *s1, int len)
{
    int tmp;
    printk(KERN_EMERG "%s\n :", s0);

    for (tmp = 0; tmp< len; tmp++) {
        printk(KERN_EMERG "%02x ", *s1++);
    }
    printk("\n");
}


/**---------------------------------------------------------------------------

  \brief wlan_hdd_ftm_open() -

   The function hdd_wlan_sdio_probe calls this function to initialize the FTM specific modules.

  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

int wlan_hdd_ftm_open(hdd_adapter_t *pAdapter)
{
    VOS_STATUS vStatus       = VOS_STATUS_SUCCESS;
    v_U8_t iter               = 0;
    tSirRetStatus sirStatus = eSIR_SUCCESS;
    tMacOpenParameters macOpenParms;

    pVosContextType pVosContext= NULL;

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: Opening VOSS", __func__);

    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

    if (NULL == pVosContext)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Trying to open VOSS without a PreOpen",__func__);
        VOS_ASSERT(0);
        goto err_vos_status_failure;
    }

    /* Initialize the probe event */
    if (vos_event_init(&pVosContext->ProbeEvent) != VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Unable to init probeEvent",__func__);
        VOS_ASSERT(0);
        goto err_vos_status_failure;
    }

    /* Initialize the free message queue */
    vStatus = vos_mq_init(&pVosContext->freeVosMq);
    if (! VOS_IS_STATUS_SUCCESS(vStatus))
    {

        /* Critical Error ...  Cannot proceed further */
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to initialize VOS free message queue",__func__);
        VOS_ASSERT(0);
        goto err_probe_event;
    }

    for (iter =0; iter < VOS_CORE_MAX_MESSAGES; iter++)
    {
        (pVosContext->aMsgWrappers[iter]).pVosMsg =
         &(pVosContext->aMsgBuffers[iter]);
        INIT_LIST_HEAD(&pVosContext->aMsgWrappers[iter].msgNode);
        vos_mq_put(&pVosContext->freeVosMq, &(pVosContext->aMsgWrappers[iter]));
    }

    /* Now Open the VOS Scheduler */
    vStatus= vos_sched_open(pVosContext, &pVosContext->vosSched,
                           sizeof(VosSchedContext));

    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
       /* Critical Error ...  Cannot proceed further */
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to open VOS SCheduler", __func__);
       VOS_ASSERT(0);
       goto err_msg_queue;
    }

    /* Probe the MC thread */
    sysMcThreadProbe(pVosContext,
                    &ftm_vos_sys_probe_thread_cback,
                    pVosContext);

    if (vos_wait_single_event(&pVosContext->ProbeEvent, 0)!= VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Failed to probe MC Thread", __func__);
        VOS_ASSERT(0);
        goto err_sched_close;
    }
    
    /* initialize the NV module */
    vStatus = vos_nv_open();
    
    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        // NV module cannot be initialized, however the driver is allowed
        // to proceed
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
          "%s: Failed to initialize the NV module", __func__);
         goto err_sched_close;
    }
    /* If we arrive here, both threads dispacthing messages correctly */

    /* Now proceed to open the MAC */
    macOpenParms.driverType = eDRIVER_TYPE_MFG;

    /* UMA is supported in hardware for performing the
      frame translation 802.11 <-> 802.3 */
    macOpenParms.frameTransRequired = 1;
    sirStatus = macOpen(&(pVosContext->pMACContext), pVosContext->pHDDContext,
                         &macOpenParms);

    if (eSIR_SUCCESS != sirStatus)
    {
        /* Critical Error ...    Cannot proceed further */
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Failed to open MAC", __func__);
        VOS_ASSERT(0);
        goto err_nv_close;
    }

    vStatus = WLANBAL_Open(pVosContext);

    if(!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAL",__func__);
        goto err_mac_close;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
               "%s: VOSS successfully Opened",__func__);

       /* Save the hal context in Adapter */
    pAdapter->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_HAL, pVosContext );

    if ( NULL == pAdapter->hHal )
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: HAL context is null",__func__);
       goto err_mac_close;
    }
       //Initialize the nlink service
    if(nl_srv_init() != 0)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%S: nl_srv_init failed",__func__);
       goto err_mac_close;
    }

#ifdef PTT_SOCK_SVC_ENABLE
    //Initialize the PTT service
    if(ptt_sock_activate_svc(pAdapter) != 0)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: ptt_sock_activate_svc failed",__func__);
       goto err_nl_srv_init;
    }
#endif
       //Turn off carrier state
    netif_carrier_off(pAdapter->dev);

    //Stop the Interface TX queue. Just being safe
    netif_tx_disable(pAdapter->dev);

    pAdapter->ftm.ftm_state = WLAN_FTM_INITIALIZED;

    return VOS_STATUS_SUCCESS;

err_nl_srv_init:
nl_srv_exit();

err_mac_close:
macClose(pVosContext->pMACContext);

err_nv_close:
vos_nv_close();

err_sched_close:
vos_sched_close(pVosContext);

err_msg_queue:
vos_mq_deinit(&pVosContext->freeVosMq);

err_probe_event:
vos_event_destroy(&pVosContext->ProbeEvent);

err_vos_status_failure:

    return VOS_STATUS_E_FAILURE;
}



int wlan_hdd_ftm_close(hdd_adapter_t *pAdapter)
{
    ENTER();

    return 0;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_send_response() -

   The function sends the response to the ptt socket application running in user space.

  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS wlan_ftm_send_response(hdd_adapter_t *pAdapter){

   if( ptt_sock_send_msg_to_app(&pAdapter->ftm.wnl->wmsg, 0, ANI_NL_MSG_PUMAC, pAdapter->ftm.wnl->nlh.nlmsg_pid) < 0) {

       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Ptt Socket error sending message to the app!!\n"));
       return VOS_STATUS_E_FAILURE;
   }
   return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_hdd_ftm_start() -

   This function gets called when the FTM start commands received from the ptt socket application and
   it starts the following modules.
   1) SAL Start.
   2) BAL Start.
   3) MAC Start to download the firmware.


  \param  - pAdapter - Pointer HDD Context.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

int wlan_hdd_ftm_start(hdd_adapter_t *pAdapter)
{
    VOS_STATUS vStatus          = VOS_STATUS_SUCCESS;
    tSirRetStatus sirStatus      = eSIR_SUCCESS;
    pVosContextType pVosContext = (pVosContextType)pAdapter->pvosContext;
    tHalMacStartParameters halStartParams;

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
            "%s: Starting Libra SW", __func__);

    /* We support only one instance for now ...*/
    if (pVosContext == NULL)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
           "%s: mismatch in context",__FUNCTION__);
        goto err_status_failure;
    }

    if ((pVosContext->pBALContext == NULL) || ( pVosContext->pMACContext == NULL))
    {
        if (pVosContext->pBALContext == NULL)
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: BAL NULL context",__FUNCTION__);
        else if (pVosContext->pMACContext == NULL)
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: MAC NULL context",__FUNCTION__);
        else
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: TL NULL context",__FUNCTION__);

        goto err_status_failure;
    }

    /* Start SAL now */
    vStatus = WLANSAL_Start(pVosContext);
    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start SAL",__func__);
        goto err_status_failure;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
           "%s: SAL correctly started", __func__);

    /* Start BAL */
    vStatus = WLANBAL_Start(pVosContext);

    if (!VOS_IS_STATUS_SUCCESS(vStatus))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start BAL",__func__);
        goto err_sal_stop;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: BAL correctly started",__func__);

    /* Start the MAC */
    vos_mem_zero((v_PVOID_t)&halStartParams, sizeof(tHalMacStartParameters));

    /* Attempt to get the firmware binary through VOS.  We need to pass this
           to the MAC when starting. */
    vStatus = hdd_request_firmware(LIBRA_FW_FILE,pAdapter,
                               (v_VOID_t **)&halStartParams.FW.pImage,
                               (v_SIZE_t *)&halStartParams.FW.cbImage);

    if ( !VOS_IS_STATUS_SUCCESS( vStatus ) )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Failed to get firmware binary",__func__);
        printk(KERN_EMERG "***Failed to get firmware binary***\n");
        goto err_bal_stop;
    }

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: Firmware binary file found",__func__);

    /* Start the MAC */
    sirStatus = macStart(pVosContext->pMACContext,(v_PVOID_t)&halStartParams);

    /* Free uo the FW image no matter what */
    if( NULL != halStartParams.FW.pImage )
    {
        hdd_release_firmware(LIBRA_FW_FILE,pVosContext->pHDDContext);
        halStartParams.FW.pImage = NULL;
        halStartParams.FW.cbImage = 0;
    }

    if (eSIR_SUCCESS != sirStatus)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start MAC", __func__);

        printk(KERN_EMERG "***Failed to start MAC****\n");
        goto err_bal_stop;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
            "%s: MAC correctly started",__func__);


    printk(KERN_EMERG "*** FTM Start Successful****\n");


    /* START SYS. This will trigger the CFG download */
    sysMcStart(pVosContext, ftm_vos_sys_probe_thread_cback, pVosContext);

    /* Initialize the ftm vos event */
    if (vos_event_init(& pAdapter->ftm.ftm_vos_event) != VOS_STATUS_SUCCESS)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Unable to init probeEvent",__func__);
        VOS_ASSERT(0);
        goto err_mac_stop;
    }

    pAdapter->ftm.ftm_state = WLAN_FTM_STARTED;

    return VOS_STATUS_SUCCESS;

err_mac_stop:
macStop(pVosContext->pMACContext, HAL_STOP_TYPE_SYS_RESET);

err_bal_stop:
WLANBAL_Stop(pVosContext);

err_sal_stop:
WLANSAL_Stop(pVosContext);

err_status_failure:

    return VOS_STATUS_E_FAILURE;

}


int wlan_ftm_stop(hdd_adapter_t *pAdapter)
{
   return WLAN_FTM_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief wlan_ftm_postmsg() -

   The function used for sending the command to the halphy.

  \param  - cmd_ptr - Pointer command buffer.

  \param  - cmd_len - Command length.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

v_U32_t wlan_ftm_postmsg(v_U8_t *cmd_ptr, v_U16_t cmd_len)
{
    vos_msg_t   *ftmReqMsg;
    vos_msg_t    ftmMsg;
    ENTER();

    ftmReqMsg = (vos_msg_t *) cmd_ptr;

    ftmMsg.type = ftmReqMsg->type;
    ftmMsg.reserved = 0;
    ftmMsg.bodyptr = (v_U8_t*)cmd_ptr;
    ftmMsg.bodyval = 0;


    /* Use Vos messaging mechanism to send the command to halPhy */

    if (VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_HAL,
                                    (vos_msg_t *)&ftmMsg)) {
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: : Failed to post Msg to HAL\n",__func__);

        return VOS_STATUS_E_FAILURE;
    }

    EXIT();
    return VOS_STATUS_SUCCESS;
}


/**---------------------------------------------------------------------------

  \brief wlan_hdd_process_ftm_cmd() -

   This function process the commands received from the ptt socket application.

  \param  - pAdapter - Pointer HDD Context.

  \param  - wnl - Pointer to the ANI netlink header.

  \return - none

  --------------------------------------------------------------------------*/

void wlan_hdd_process_ftm_cmd
(
    hdd_adapter_t *pAdapter,
    tAniNlHdr *wnl
)
{
    wlan_hdd_ftm_request_t  *pRequestBuf = (wlan_hdd_ftm_request_t*)(((v_U8_t*)(&wnl->wmsg))+sizeof(tAniHdr)) ;
    v_U16_t   cmd_len;
    v_U8_t *pftm_data;

    ENTER();

    if (!pRequestBuf) {

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: request buffer is null\n",__func__);
        return ;
    }
    /*Save the received request*/
    pAdapter->ftm.pRequestBuf = pRequestBuf;

    pAdapter->ftm.pResponseBuf = (wlan_hdd_ftm_response_t*)pRequestBuf;

     /*Save the received request netlink header used for sending the response*/
    pAdapter->ftm.wnl = wnl;

    if (pRequestBuf->module_type != QUALCOMM_MODULE_TYPE) {

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Invalid Module Type =%d\n",__func__,pRequestBuf->module_type);

        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
        wlan_ftm_send_response(pAdapter);
        return ;
    }

    switch (pRequestBuf->ftmpkt.ftm_cmd_type)
    {
    case WLAN_FTM_START:
        if (pAdapter->ftm.ftm_state == WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s: FTM has already started =%d\n",__func__,pRequestBuf->ftmpkt.ftm_cmd_type);
            pAdapter->ftm.pResponseBuf->ftm_hdr.data_len -= 1;
            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;
        }
        if (wlan_hdd_ftm_start(pAdapter) != VOS_STATUS_SUCCESS) {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s: : Failed to start WLAN FTM\n",__func__);
            return;
        }
        /* Ptt application running on the host PC expects the length to be one byte less that what we have received*/
        pAdapter->ftm.pResponseBuf->ftm_hdr.data_len -= 1;
        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;
        pAdapter->ftm.pResponseBuf->ftmpkt.ftm_cmd_type = 0;


        wlan_ftm_send_response(pAdapter);

        break;

    case WLAN_FTM_STOP:

        if (pAdapter->ftm.ftm_state != WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM has not started\n",__func__);
            return;
        }

        if (VOS_STATUS_SUCCESS != wlan_ftm_stop(pAdapter)) {

            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;
        }

        /* This would send back the Command Success Status */
        pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;

        wlan_ftm_send_response(pAdapter);

        break;

    case WLAN_FTM_CMD:

        /* if it is regular FTM command, pass it to HAL PHY */
        if(pAdapter->ftm.IsCmdPending == TRUE) {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM command pending for process\n",__func__);
            return;
        }

        if (pAdapter->ftm.ftm_state != WLAN_FTM_STARTED) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM has not started\n",__func__);

            pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_FAILURE;
            wlan_ftm_send_response(pAdapter);
            return;

        }
        vos_event_reset(&pAdapter->ftm.ftm_vos_event);

        cmd_len = pRequestBuf->ftm_hdr.data_len;

        cmd_len -= (sizeof(wlan_hdd_ftm_request_t)- sizeof(pRequestBuf->ftmpkt.ftm_cmd_type));
        pftm_data = pRequestBuf->ftmpkt.pFtmCmd;

        //HEXDUMP("Request:",(char*)pftm_data,cmd_len);

        pAdapter->ftm.IsCmdPending = TRUE;

        /*Post the command to the HAL*/
        if (wlan_ftm_postmsg(pftm_data, cmd_len) != VOS_STATUS_SUCCESS) {

            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: FTM command failed\n",__func__);
            return;

        }
        /*Wait here until you get the response from HAL*/
        if (vos_wait_single_event(&pAdapter->ftm.ftm_vos_event, 0)!= VOS_STATUS_SUCCESS)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR,
               "%s: vos_wait_single_event failed",__func__);
            return;
        }

        cmd_len = be16_to_cpu(pAdapter->ftm.wnl->wmsg.length);

        //HEXDUMP("Response to QXDM:", (char *)&pAdapter->ftm.wnl->wmsg, cmd_len);

        wlan_ftm_send_response(pAdapter);
        pAdapter->ftm.IsCmdPending = FALSE;
        break;

    default:

        hddLog(VOS_TRACE_LEVEL_ERROR,"%s:: Command not supported \n",__func__);
        return;
    }

    EXIT();
    return;
} /* wlan_adp_ftm_cmd() */

/**---------------------------------------------------------------------------

  \brief WLANFTM_McProcessMsg() -

   Called by VOSS when a message was serialized for FTM through the
   main thread/task.

  \param  -  message:        type and content of the message.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS WLANFTM_McProcessMsg (v_VOID_t *message)
{

    ftm_rsp_msg_t   *pFtmMsgRsp;

    VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter;
    v_CONTEXT_t pVosContext= NULL;

    ENTER();

    pFtmMsgRsp = (ftm_rsp_msg_t *)message;

    if (!message )
    {
        VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                "WLAN FTM:Invalid parameter sent on WLANFTM_ProcessMainMessage");
        return VOS_STATUS_E_INVAL;
    }
    /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

     /*Get the Hdd Context */
    pAdapter = ((VosContextType*)(pVosContext))->pHDDContext;

    /*Response length to Ptt App*/
    pAdapter->ftm.wnl->wmsg.length = sizeof(tAniHdr)+ SIZE_OF_FTM_DIAG_HEADER_LEN + pFtmMsgRsp->msgBodyLength;

     /*Ptt App expects the response length in LE */
    pAdapter->ftm.wnl->wmsg.length = FTM_SWAP16(pAdapter->ftm.wnl->wmsg.length);

    /*Response expects the length to be in */
    pAdapter->ftm.pResponseBuf->ftm_hdr.data_len = pAdapter->ftm.pRequestBuf->ftm_hdr.data_len - 1;

    /*Copy the message*/
    memcpy((char*)&pAdapter->ftm.pResponseBuf->ftmpkt,(char*)message,pFtmMsgRsp->msgBodyLength);

    /*Update the error code*/
    pAdapter->ftm.pResponseBuf->ftm_err_code = WLAN_FTM_SUCCESS;

    vos_status = vos_event_set(&pAdapter->ftm.ftm_vos_event);

    if (!VOS_IS_STATUS_SUCCESS(vos_status))
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos_event_set failed!!\n"));
       return VOS_STATUS_E_FAILURE;
    }

    EXIT();
    return VOS_STATUS_SUCCESS;

}


VOS_STATUS wlan_write_to_efs (v_U8_t *pData, v_U16_t data_len)
{
    tAniHdr *wmsg = NULL;
    v_U8_t *pBuf;
    hdd_adapter_t *pAdapter;
    v_CONTEXT_t pVosContext= NULL;

    pBuf =  (v_U8_t*)vos_mem_malloc(sizeof(tAniHdr) + sizeof(v_U32_t)+ data_len);

    wmsg = (tAniHdr*)pBuf;
    wmsg->type = PTT_MSG_FTM_CMDS_TYPE;
    wmsg->length = data_len + sizeof(tAniHdr)+ sizeof(v_U32_t);
    wmsg->length = FTM_SWAP16(wmsg->length);
    pBuf += sizeof(tAniHdr);

     /*Get the global context */
    pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

     /*Get the Hdd Context */
    pAdapter = ((VosContextType*)(pVosContext))->pHDDContext;

    /* EfS command Code */
    *(v_U32_t*)pBuf = 0x000000EF;

    pBuf += sizeof(v_U32_t);

    memcpy(pBuf, pData,data_len);
   
    if( ptt_sock_send_msg_to_app(wmsg, 0, ANI_NL_MSG_PUMAC, pAdapter->ftm.wnl->nlh.nlmsg_pid) < 0) {

        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Ptt Socket error sending message to the app!!\n"));
        return VOS_STATUS_E_FAILURE;
    }

    vos_mem_free(pBuf);
    
    return VOS_STATUS_SUCCESS;
}

#endif /* ANI_MANF_DIAG */
