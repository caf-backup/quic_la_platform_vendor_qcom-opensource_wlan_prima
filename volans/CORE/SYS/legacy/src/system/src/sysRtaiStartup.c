/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Copyright (C) 2006 Airgo Networks, Incorporated
 *
 * sysRtaiStartup.cc: System startup file for RTAI platform.
 *
 * Author:  Kevin Nguyen
 * Date:    08/01/2002
 *
 * History:-
 * Date     Modified by         Modification Information
 * --------------------------------------------------------------------------
 *
 */
#include <stdarg.h>

#include <aniGlobal.h>
#include <sirCommon.h>
#include <sysDef.h>
#include <sysEntryFunc.h>
#include <halCommonApi.h>   /* halStateSet */
#include <utilsApi.h>
#include <cfgApi.h>
#include <limApi.h>


#include <schApi.h>
#include <sysRtaiStartup.h>
#include <sysStartup.h>
#include <aniParam.h>
#include "sysDebug.h"

extern void rtaiCancelAllTimer(struct rtLibApp * rt);
extern void (*rtaiMonitorInterrupts)(void);
extern void sysMonitorInterrupts(void);
extern void sysInitMonitorInterrupts(void);

// ---------------------------------------------------------------------
/*
 * sysMacModInit
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param  None
 * @return None
 */


void
sysMacModInit(tAniMacParam * pParam, struct rtLibApp * rt)
{
    tpAniSirGlobal pMac;

    pMac = (tpAniSirGlobal)pParam->hHalHandle;
    pMac->rt=rt;
    return;
}

// ---------------------------------------------------------------------
/*
 * sysMacModExit
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param  None
 * @return None
 */

void
sysMacModExit(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    // Delete HAL and MMH threads here
    tx_thread_delete(&pMac->sys.gSirMmhThread);
    tx_thread_delete(&pMac->sys.gSirHalThread);

    // Delete TX and RX Msg Qs here
    tx_queue_delete(&pMac->sys.gSirRxMsgQ);
    tx_queue_delete(&pMac->sys.gSirTxMsgQ);
}

// ---------------------------------------------------------------------
/*
 * sysSuspendThreads
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param  None
 * @return None
 */

void
sysSuspendThreads(tpAniSirGlobal pMac)
{
    if (pMac==0)
        return; /* really bad */

    tx_thread_suspend(&pMac->sys.gSirHalThread);
    tx_thread_suspend(&pMac->sys.gSirMntThread);
    tx_thread_suspend(&pMac->sys.gSirSchThread);
    tx_thread_suspend(&pMac->sys.gSirPmmThread);
    tx_thread_suspend(&pMac->sys.gSirLimThread);
#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
    tx_thread_suspend(&pMac->sys.gSirNimPttThread);
#endif
    rtaiCancelAllTimer(pMac->rt);
}

// ---------------------------------------------------------------------
/**
 * sysRecvPacket
 *
 * FUNCTION:
 *  Receive a new packet from RxDemux task.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *  This function is called by HDD to forward a non-data frame
 *  to the MAC.
 *
 * @param   pPacket:  Packet pointer
 * @return  None
 */

void
sysRecvPacket(tHalHandle hHal, void* pPacket)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tSirMsgQ   mmhMsg;

    mmhMsg.type = SIR_BB_XPORT_MGMT_MSG;
    mmhMsg.bodyptr = pPacket;
    mmhMsg.bodyval = 0;

    tpHalBufDesc pBD;

#ifndef GEN6_ONWARDS
    palGetPacketDataPtr( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, pPacket, (void **) &pBD );
#endif //GEN6_ONWARDS

    /* tpSirMacMgmtHdr mHdr = (tpSirMacMgmtHdr) (pBD->macHdr);
     * in the new world, th empdu header is located at an offset contianed
     * in the bd */
    tpSirMacMgmtHdr mHdr = SIR_MAC_BD_TO_MPDUHEADER(pBD);
#if defined (ANI_CHIPSET_TAURUS)
    if(pBD->swBdType != SMAC_SWBD_TYPE_CTLMSG)
#endif // defined (ANI_CHIPSET_TAURUS)
    {
        sysLog(pMac, LOG2, FL("** RX msg type %d sub %d rxpFlags 0x%x\n"),
               mHdr->fc.type, mHdr->fc.subType, pBD->rxpFlags);
        sirDumpBuf(pMac, SIR_SYS_MODULE_ID, LOG3, (tANI_U8 *) mHdr, sizeof(tSirMacMgmtHdr));
    }
    if (sysBbtProcessMessageCore(pMac, &mmhMsg, mHdr->fc.type,
                                 mHdr->fc.subType)
        != eSIR_SUCCESS)
    {
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, (void *) pBD, (void *) pPacket) ;
    }
}


struct rtLibApp * getRtLibApp(void* pMac) {
  return ((tpAniSirGlobal)pMac)->rt;
}

// ---------------------------------------------------------------------
/*
 * sysMailboxRead
 *
 * FUNCTION:
 *  API for HDD to read a message from MAC
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param  hHal HAL Handle of the radio instance
 * @param  message pointer to the location that will point to the message
 * @return eHAL_STATUS_SUCCESS on success
 */
eHalStatus sysMailboxRead(tHalHandle hHal, void * message)
{
    tpAniSirGlobal pMac;
    int status;
    int retStatus = eHAL_STATUS_SUCCESS;

    pMac = (tpAniSirGlobal) hHal;
    status = tx_queue_receive(&pMac->sys.gSirRxMsgQ, message, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        retStatus = eHAL_STATUS_FAILURE;
    }

    return retStatus;
}


// ---------------------------------------------------------------------
/*
 * sysMailboxWrite
 *
 * FUNCTION:
 *  API for HDD to write a message to MAC
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param  hHal HAL Handle of the radio instance
 * @param  message pointer to the location of the message
 * @return eHAL_STATUS_SUCCESS on success
 */
eHalStatus sysMailboxWrite(tHalHandle hHal, void * message)
{
    tpAniSirGlobal pMac;
    int status;
    int retStatus = eHAL_STATUS_SUCCESS;

    pMac = (tpAniSirGlobal) hHal;
    status = tx_queue_send(&pMac->sys.gSirTxMsgQ, message, TX_NO_WAIT);
    if (status != TX_SUCCESS) {
        retStatus = eHAL_STATUS_FAILURE;
    }

    return retStatus;
}


// ---------------------------------------------------------------------
/**
 * sysRtaiStartup
 *
 * FUNCTION:
 * Initializes all system wide resources.
 *
 * LOGIC:
 *
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 *
 * @param pParam pointer to HDD structure
 * @return None
 */

void
sysRtaiStartup(tAniMacParam * pParam)
{
    tANI_U32          status;
    tANI_U8          *pPtr = NULL;
    char         name[32];
    tpAniSirGlobal pMac;

    pMac = (tpAniSirGlobal)pParam->hHalHandle;

    pMac->sys.gSirRadioId  = pParam->radioId;
    pMac->sys.gSirThreadCount = 0;


    //-----------------------------------------------------------------------
    // Initialize system
    //-----------------------------------------------------------------------

    rtaiBufInit((unsigned int)(pParam->radioId),(t_mac_block_table*)(pParam->block_table));


    //-----------------------------------------------------------------------
    // Create all message queues
    //-----------------------------------------------------------------------

    /// HDD TX Msg Q creation
    sysLog(pMac, LOG1, "<SYS-%d> Create TX msgQ\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"HDDTxMsgQ%d",(int)pParam->radioId);

    status = tx_queue_create(pMac->rt,&pMac->sys.gSirTxMsgQ, name, SYS_TX_MSG_SIZE, (char*)pPtr,
                             SYS_TX_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// HDD RX Msg Q creation
    sysLog(pMac, LOG1, "<SYS-%d> Create RX msgQ\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"HDDRxMsgQ%d",(int)(int)pParam->radioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirRxMsgQ, name, SYS_RX_MSG_SIZE, (char*)pPtr,
                             SYS_RX_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// HAL Msg Q creation
    tx_sprintf(name,"HALMsgQ%d",(int)pParam->radioId);
    sysLog(pMac, LOG1, "<SYS-%d> Create HAL msgQ\n", pMac->sys.gSirRadioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirHalMsgQ, name, SYS_HAL_MSG_SIZE, (char*)pPtr,
                             SYS_HAL_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// Maintenance Msg Q creation
    tx_sprintf(name,"Maintenance%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create MNT msgQ\n", pMac->sys.gSirRadioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirMntMsgQ, name, SYS_MNT_MSG_SIZE, (char*)pPtr,
                             SYS_MNT_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// LIM Msg Q creation
    tx_sprintf(name,"LIMMsg%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create LIM msgQ\n", pMac->sys.gSirRadioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirLimMsgQ, name, SYS_LIM_MSG_SIZE, (char*)pPtr,
                             SYS_LIM_Q_SIZE, LIM_TIMER_EXPIRY_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    tx_sprintf(name,"LIMDefMsg%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create LIM deferred msgQ\n", pMac->sys.gSirRadioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirLimDeferredMsgQ, name,
                             SYS_LIM_MSG_SIZE, (char*)pPtr,
                             SYS_LIM_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// Scheduler Msg Q creation
    tx_sprintf(name,"scheduler%d",(int)pParam->radioId);
    sysLog(pMac, LOG1, "<SYS-%d> Create SCH msgQ\n", pMac->sys.gSirRadioId);
    status = tx_queue_create(pMac->rt,&pMac->sys.gSirSchMsgQ, name, SYS_SCH_MSG_SIZE, (char*)pPtr,
                             SYS_SCH_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

    /// Power Management Msg Q creation
    sysLog(pMac, LOG1, "<SYS-%d> Create PMM msgQ\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"PowerMgmt%d",(int)pParam->radioId);

    status = tx_queue_create(pMac->rt,&pMac->sys.gSirPmmMsgQ, name, SYS_PMM_MSG_SIZE, (char*)pPtr,
                             SYS_PMM_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create message queue\n", pMac->sys.gSirRadioId);

#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
    /// PTT Msg Q creation
    sysLog(pMac, LOG1, "<SYS-%d> Create NIM RD msgQ\n",
           pMac->sys.gSirRadioId);
    tx_sprintf(name,"NimRDMsgQ%d",(int)pParam->radioId);
    status = tx_queue_create(pMac->rt, &pMac->sys.gSirNimRDMsgQ, name,
                             SYS_NIM_PTT_MSG_SIZE, (char*)pPtr,
                             SYS_NIM_PTT_Q_SIZE, NO_LIST);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE,
               "<SYS-%d> Failed to create nim rtt message queue\n",
               pMac->sys.gSirRadioId);
#endif

    //-----------------------------------------------------------------------
    // Create all MAC tasks
    //-----------------------------------------------------------------------

    /// Mailbox Mechanism (MMH) thread creation
    ++pMac->sys.gSirThreadCount;
    sysLog(pMac, LOG1, "<SYS-%d> Create MMH task\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"MMH%d",(int)pParam->radioId);

    tx_thread_create(pMac->rt,&pMac->sys.gSirMmhThread,name, sysMmhEntry, 0, (char*)pPtr,
                     SYS_MMH_STACK_SIZE, SYS_MMH_THREAD_PRIORITY,
                     SYS_MMH_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

    /// HAL thread creation
    ++pMac->sys.gSirThreadCount;
    sysLog(pMac, LOG1, "<SYS-%d> Create HAL task\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"HAL%d",(int)pParam->radioId);
    status = tx_thread_create(pMac->rt,&pMac->sys.gSirHalThread,name, sysHalEntry, 0,
                              (char*)pPtr, SYS_HAL_STACK_SIZE,
                              SYS_HAL_THREAD_PRIORITY, SYS_HAL_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

    /// Maintenance (MNT) thread creation
    ++pMac->sys.gSirThreadCount;
    tx_sprintf(name,"Maintenance%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create MNT task\n", pMac->sys.gSirRadioId);
    status = tx_thread_create(pMac->rt,&pMac->sys.gSirMntThread,name, sysMntEntry, 0,
                              (char*)pPtr, SYS_MNT_STACK_SIZE,
                              SYS_MNT_THREAD_PRIORITY, SYS_MNT_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

    /// SCH thread creation
    ++pMac->sys.gSirThreadCount;
    tx_sprintf(name,"SCH%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create SCH task\n", pMac->sys.gSirRadioId);
    status = tx_thread_create(pMac->rt,&pMac->sys.gSirSchThread, name, sysSchEntry, 0, (char*)pPtr,
                     SYS_SCH_STACK_SIZE, SYS_SCH_THREAD_PRIORITY,
                     SYS_SCH_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

    /// PMM thread creation
    ++pMac->sys.gSirThreadCount;
    tx_sprintf(name,"PMM%d",(int)pParam->radioId);

    sysLog(pMac, LOG1, "<SYS-%d> Create PMM task\n", pMac->sys.gSirRadioId);
    status = tx_thread_create(pMac->rt,&pMac->sys.gSirPmmThread, name, sysPmmEntry, 0, (char*)pPtr,
                     SYS_PMM_STACK_SIZE, SYS_PMM_THREAD_PRIORITY,
                     SYS_PMM_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

    /// LIM thread creation
    ++pMac->sys.gSirThreadCount;
    sysLog(pMac, LOG1, "<SYS-%d> Create LIM task\n", pMac->sys.gSirRadioId);
    tx_sprintf(name,"LIM%d",(int)pParam->radioId);

    status = tx_thread_create(pMac->rt,&pMac->sys.gSirLimThread, name, sysLimEntry, 0, (char*)pPtr,
                     SYS_LIM_STACK_SIZE, SYS_LIM_THREAD_PRIORITY,
                     SYS_LIM_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);

#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
    /// NIM PTT thread
    ++pMac->sys.gSirThreadCount;
    sysLog(pMac, LOG1, "<SYS-%d> Create PKT GEN thread\n",
           pMac->sys.gSirRadioId);
    tx_sprintf(name,"PKT%d",(int)pParam->radioId);
    status = tx_thread_create(pMac->rt, &pMac->sys.gSirNimPttThread, name,
                              sysNimPttEntry, 0, (char*)pPtr,
                              SYS_NIM_PTT_THREAD_STACK_SIZE,
                              SYS_NIM_PTT_THREAD_PRIORITY,
                              SYS_NIM_PTT_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_DONT_START);
    if (status != TX_SUCCESS)
        sysLog(pMac, LOGE, "<SYS-%d> Failed to create task %s\n", pMac->sys.gSirRadioId, name);
#endif

    // At this point, we need to start MMH and HAL tasks.
    // HDD will send SIR_HAL_START_IND when the MAC device
    // is first opened on the host side.  Only then,  Hal
    // task will proceed with chipset HW initialization
    // and the rest of the tasks can be started.

    sysLog(pMac, LOG1, "<SYS-%d> Resume MMH task\n", pMac->sys.gSirRadioId);
    tx_thread_resume(&pMac->sys.gSirMmhThread);

    sysLog(pMac, LOG1, "<SYS-%d> Resume HAL task\n", pMac->sys.gSirRadioId);
    tx_thread_resume(&pMac->sys.gSirHalThread);

    sysLog(pMac, LOG1, "<SYS-%d> Resume LIM task\n", pMac->sys.gSirRadioId);
    tx_thread_resume(&pMac->sys.gSirLimThread);

    sysLog(pMac, LOG1, "<SYS-%d> Resume SCH task\n", pMac->sys.gSirRadioId);
    tx_thread_resume(&pMac->sys.gSirSchThread);

    sysLog(pMac, LOG1, "<SYS-%d> Resume PMM task\n", pMac->sys.gSirRadioId);
    tx_thread_resume(&pMac->sys.gSirPmmThread);

#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
//    printk("ptt before resume\n");
    sysLog(pMac, LOGW, "sysNimPttThreadStart: Thread resumed\n");
    tx_thread_resume(&pMac->sys.gSirNimPttThread);
//    printk("ptt after resuming status = %ud\n", status);
#endif

} // sysRtaiStartup();

// ---------------------------------------------------------------------
/**
 * sysRtaiCleanup
 *
 * FUNCTION:
 *  Initializes all system wide resources.
 *
 * LOGIC:
 *
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

void
sysRtaiCleanup(tHalHandle hHal)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    sysLog(pMac, LOG1, "<sysCleanup> Perform cleanup actions\n");

    // Delete tasks
    sysLog(pMac, LOG1, "<sysCleanup> Delete all MAC tasks\n");

    // cancel all timer before deleting any tasks
    rtaiCancelAllTimer(pMac->rt);

    tx_thread_delete(&pMac->sys.gSirMntThread);
    tx_thread_delete(&pMac->sys.gSirLimThread);
    tx_thread_delete(&pMac->sys.gSirPmmThread);
    tx_thread_delete(&pMac->sys.gSirSchThread);
#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
    tx_thread_delete(&pMac->sys.gSirNimPttThread);
#endif

    // Delete all message queues
    sysLog(pMac, LOG1, "<sysCleanup> Deleting Message queues\n");
    tx_queue_delete(&pMac->sys.gSirHalMsgQ);
    tx_queue_delete(&pMac->sys.gSirMntMsgQ);
    tx_queue_delete(&pMac->sys.gSirLimMsgQ);
    tx_queue_delete(&pMac->sys.gSirLimDeferredMsgQ);
    tx_queue_delete(&pMac->sys.gSirSchMsgQ);
    tx_queue_delete(&pMac->sys.gSirPmmMsgQ);
#if defined(ANI_MANF_DIAG) || defined(ANI_PHY_DEBUG)
    tx_queue_delete(&pMac->sys.gSirNimRDMsgQ);
#endif

    sysLog(pMac, LOG1, "<sysCleanup> Done cleaning up\n");
}

// ---------------------------------------------------------------------

