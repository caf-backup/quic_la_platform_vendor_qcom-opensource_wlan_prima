/*===========================================================================

  FILE:
     halMailbox.c

  BRIEF DESCRIPTION:
     MailBox interface file for HAL to Firmware. This file contains all the functions to support
     mailbox initialization and handling.

  DESCRIPTION:
     This file implements
     1. Mailbox Initialization function
     2. Mailbox0 Interrupt Handler (Mbox0_IntrHandler Host->FW)
     3. Mailbox1 Interrupt Handler (Mbox1_IntrHandler FW->Host)
     4. Mailbox0 message posting (halMbox_SendMsg)


                  Copyright (c) 2008 Qualcomm Technologies, Inc.
                  All Right Reserved.
                  Qualcomm Technologies Confidential and Proprietary
  ===========================================================================*/

#include "palTypes.h"
#include "halMailbox.h"   // my own protos
#include "libraDefs.h"
#include "palApi.h"       // register/mem access
#include "halMemoryMap.h" // mailbox offsets and addresses
#include "halDebug.h"     // halLog
#include "aniGlobal.h"
#include "schApi.h"
#include "ani_assert.h"
#include "halFw.h"

/* --------
 * Mailbox
 * -------- */
#define HOST_NUM_OF_MAILBOX 2
#define H2FW_MAILBOX    QWLAN_MCU_MAILBOX_H2F_CTRL
#define FW2H_MAILBOX    QWLAN_MCU_MAILBOX_F2H_CTRL

/* -----------------
 *  Local constants
 * ----------------- */
#define MCU_MB_CONTROL_REGISTER(n)          (QWLAN_MCU_MB0_CONTROL_REG + (n * 8))
#define MCU_MB_CONTROL_COUNTER_REGISTER(n)  (QWLAN_MCU_MB0_CONTROL_COUNTERS_REG + (n * 8))
#define FAILURE                             -1
#define INVALID                             -1

/* -------------------------------
 * FUNCTION:  halMbox_Start()
 * -------------------------------
 */
eHalStatus halMbox_Start(tHalHandle hHal, void *arg)
{
    tpMboxInfo pMbox;
    eHalIntSources intr;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U8 index;
    tpMboxDeferQueue pMboxDQ;

    pMbox = (tpMboxInfo) pMac->hal.halMac.mboxInfo;

    /* Initialize */
    /* Host to Firmware Mail box */
    pMbox[H2FW_MAILBOX].pMboxStartAddr      = HOST2MCPU_MSG_OFFSET;
    pMbox[H2FW_MAILBOX].uMboxMsgSize        = HOST2MCPU_MSG_SIZE;
    pMbox[H2FW_MAILBOX].hostMsgSerialNum    = 0;
    pMbox[H2FW_MAILBOX].bMboxBusy           = eANI_BOOLEAN_FALSE;

    /* Firmware to Host Mailbox */
    pMbox[FW2H_MAILBOX].pMboxStartAddr      = MCPU2HOST_MSG_OFFSET;
    pMbox[FW2H_MAILBOX].uMboxMsgSize        = MCPU2HOST_MSG_SIZE;
    pMbox[FW2H_MAILBOX].hostMsgSerialNum    = 0; // To use for statistics/debug
    pMbox[FW2H_MAILBOX].bMboxBusy           = 0;

    // Enroll INTR handlers for all of the Mail Boxes
    // MB 0 is used for Host to FW message passing
    // MB 1 is used for FW to Host message passing

    for( intr = eHAL_INT_MCU_HOST_INT_MBOX0; intr <= eHAL_INT_MCU_HOST_INT_MBOX1; intr++ )
    {
      if( (eHalStatus)eHAL_STATUS_SUCCESS !=
          halIntEnrollHandler( intr, &halMbox_HandleInterrupt ))
          HALLOGE( halLog( pMac, LOGE,
                  FL("Unable to enroll an INTR handler for MailBox #%d\n"),
                  intr ));

      if( (eHalStatus)eHAL_STATUS_SUCCESS != halIntEnable( hHal, intr ))
          HALLOGE( halLog( pMac, LOGE,
                  FL("Failed to ENABLE the (eHAL_INT_MCU_HOST_INT_MBOX0 + %d) INTR!\n"),
                  intr - eHAL_INT_MCU_HOST_INT_MBOX0 ));
    }

    /* initialize mbox defer queue */
    pMboxDQ = (tpMboxDeferQueue)pMac->hal.halMac.mboxDeferQueue;
    pMboxDQ->readIdx = 0;
    pMboxDQ->writeIdx = 0;
    for (index = 0; index < HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES; index++)
    {
        pMboxDQ->pMsgArray[index] = (void *)NULL;
    }

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halMbox_Stop(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tANI_U8 index;
    tpMboxDeferQueue pMboxDQ;

    // free any pending messages on the defer queue
    pMboxDQ = (tpMboxDeferQueue)pMac->hal.halMac.mboxDeferQueue;
    if (NULL != pMboxDQ)
    {
        for (index = 0; index < HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES; index++)
        {
            if (NULL != pMboxDQ->pMsgArray[index])
            {
                palFreeMemory(pMac->hHdd, pMboxDQ->pMsgArray[index]);
                pMboxDQ->pMsgArray[index] = NULL;
            }
        }

        // reset indices
        pMboxDQ->readIdx = 0;
        pMboxDQ->writeIdx = 0;
    }

    return (eHAL_STATUS_SUCCESS);
}

/* -------------------------------
 * FUNCTION:  halMbox_Open()
 * -------------------------------
 */
eHalStatus halMbox_Open(tHalHandle hHal, void *arg)
{
    eHalStatus status;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    VOS_STATUS vosStatus;
    tpMboxInfo pMbox;

    (void) arg;

    /* allocate the mbox info struct */
    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.mboxInfo,
                               (sizeof(tMboxInfo) * HOST_NUM_OF_MAILBOX));

    /* allocate the mbox defer queue */
    if (eHAL_STATUS_SUCCESS == status)
    {
        status = palAllocateMemory(pMac->hHdd,
                                   &pMac->hal.halMac.mboxDeferQueue,
                                   sizeof(tMboxDeferQueue));
        if (eHAL_STATUS_SUCCESS != status)
        {
            palFreeMemory(pMac->hHdd, pMac->hal.halMac.mboxInfo);
            pMac->hal.halMac.mboxInfo = NULL;
        }
    }

    /* initialize the mbox lock for HOST->FW */
    if (eHAL_STATUS_SUCCESS == status)
    {
        pMbox = (tpMboxInfo)pMac->hal.halMac.mboxInfo;
        vosStatus = vos_lock_init(&(pMbox[H2FW_MAILBOX].lock));
        if (!VOS_IS_STATUS_SUCCESS(vosStatus))
        {
            HALLOGE( halLog(pMac, LOGE, FL("Unable to initialize VOS lock 0x%x\n"), vosStatus));

            palFreeMemory(pMac->hHdd, pMac->hal.halMac.mboxInfo);
            pMac->hal.halMac.mboxInfo = NULL;

            palFreeMemory(pMac->hHdd, pMac->hal.halMac.mboxDeferQueue);
            pMac->hal.halMac.mboxDeferQueue = NULL;

            status = eHAL_STATUS_FAILURE;
        }
    }

    return status;
}

/* -------------------------------
 * FUNCTION:  halMbox_Close()
 * -------------------------------
 */
eHalStatus halMbox_Close(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    tpMboxInfo pMbox;
    VOS_STATUS vosStatus;

    (void) arg;

    /* destroy the mbox lock for HOST->FW */
    pMbox = (tpMboxInfo)pMac->hal.halMac.mboxInfo;
    vosStatus = vos_lock_destroy(&(pMbox[H2FW_MAILBOX].lock));
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        HALLOGE(halLog(pMac, LOGE, FL("Unable to destroy VOS lock 0x%x\n"), vosStatus));
        return eHAL_STATUS_FAILURE;
    }

    /* release the mbox defer queue */
    if (pMac->hal.halMac.mboxDeferQueue != NULL)
    {
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.mboxDeferQueue);
    }
    pMac->hal.halMac.mboxDeferQueue = NULL;

    /* release the mbox info struct */
    if (pMac->hal.halMac.mboxInfo != NULL)
    {
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.mboxInfo);
    }
    pMac->hal.halMac.mboxInfo = NULL;

    return eHAL_STATUS_SUCCESS;
}

/* ----------------------------------------------------------------
 * FUNCTION:  halMbox_SendMsg()
 *
 * NOTE: Calling routine must acquire mbox lock before calling,
 *       really should only be called from halMbox_SendMsgComplete()
 *       or halMbox_SendReliableMsg().
 * ----------------------------------------------------------------
 */
eHalStatus halMbox_SendMsg(tpAniSirGlobal pMac, void *msg)
{
    tpMBoxMsgHdr pHdr = (tpMBoxMsgHdr)msg;
    tpMboxInfo pMbox = (tpMboxInfo)pMac->hal.halMac.mboxInfo;

    HALLOGW( halLog(pMac, LOGW, FL("msgType %d, msgLen %d \n"), pHdr->MsgType, pHdr->MsgLen));

    // message must be 4byte aligned
    if((pHdr->MsgLen & 3) != 0)
    {
        pHdr->MsgLen = (pHdr->MsgLen + 3) & 0xFFFC;
        HALLOGW( halLog(pMac, LOGW, FL("Message not 4byte aligned. Rounding msgLen to %d\n"),
               pHdr->MsgLen));
    }

    halWriteDeviceMemory(pMac, pMbox[MCU_MAILBOX_HOST2FW].pMboxStartAddr, (tANI_U8 *)msg, pHdr->MsgLen);
    halWriteRegister(pMac,
                         MCU_MB_CONTROL_REGISTER(MCU_MAILBOX_HOST2FW),
                         QWLAN_MCU_MB0_CONTROL_MB_RESET_MASK);

    //  assert(0==((u32_t)msgBuf&3));

    halWriteRegister(pMac,
                         MCU_MB_CONTROL_REGISTER(MCU_MAILBOX_HOST2FW),
                         (pMbox[MCU_MAILBOX_HOST2FW].pMboxStartAddr|QWLAN_MCU_MB0_CONTROL_INT_DIRECTION_MASK));

    pMbox[MCU_MAILBOX_HOST2FW].hostMsgSerialNum++;
    pMbox[MCU_MAILBOX_HOST2FW].bMboxBusy = eANI_BOOLEAN_TRUE;

    return eHAL_STATUS_SUCCESS;
}

/* ----------------------------------------------------------------
 * FUNCTION:  halMbox_SendReliableMsg()
 *
 * NOTE:
 * ----------------------------------------------------------------
 */
eHalStatus halMbox_SendReliableMsg(tpAniSirGlobal pMac, void *msg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 readIdx;
    tANI_U32 writeIdx;
    tpMBoxMsgHdr pHdr = (tpMBoxMsgHdr)msg;
    tpMboxInfo pMbox = (tpMboxInfo)pMac->hal.halMac.mboxInfo;
    tpMboxDeferQueue pMboxDQ = (tpMboxDeferQueue)pMac->hal.halMac.mboxDeferQueue;
    void *pBuf;
    VOS_STATUS vosStatus;

    if(!pMac->hal.halMac.isFwInitialized)
    {
        HALLOGW(halLog(pMac, LOGE, FL("Firmware Not Initialized.\n")));
        return eHAL_STATUS_FAILURE;
    }

    if ((pHdr == NULL) || (pHdr->MsgLen > pMbox[MCU_MAILBOX_HOST2FW].uMboxMsgSize))
    {
        HALLOGE(halLog(pMac, LOGE, FL("fail: msg==NULL or msgLen>%d\n"),
               pMbox[MCU_MAILBOX_HOST2FW].uMboxMsgSize));
        return eHAL_STATUS_FAILURE;
    }

    /* acqure lock */
    vosStatus = vos_lock_acquire(&(pMbox[H2FW_MAILBOX].lock));
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        HALLOGE(halLog(pMac, LOGE, FL("failed to acquire VOS lock: 0x%lx\n"), (tANI_U32)vosStatus));
        VOS_ASSERT(0);
        return eHAL_STATUS_FAILURE;
    }

    /* sample the mbox defer queue info */
    readIdx = pMboxDQ->readIdx;
    writeIdx = pMboxDQ->writeIdx;

    /* check if this message needs defering */
    if ((eANI_BOOLEAN_TRUE == pMbox[MCU_MAILBOX_HOST2FW].bMboxBusy) ||
        (NULL != pMboxDQ->pMsgArray[readIdx]))
    {
        /* check for room on the defer queue */
        if ((writeIdx == readIdx) &&
            (NULL != pMboxDQ->pMsgArray[writeIdx]))
        {
            tpMBoxMsgHdr pHdr;
            tANI_U32 i;
            HALLOGE(halLog(pMac, LOGE, FL("Mbox defer Queue is full\n")));
            for(i = 0; i < HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES; i++)
            {
                pHdr = (tpMBoxMsgHdr)pMboxDQ->pMsgArray[i];
                HALLOGE(halLog(pMac, LOGE, FL("Mbox defer Queue idx = %u, msgType = %u, msgLen = %u\n"),
                    i, pHdr->MsgType, pHdr->MsgLen));
            }
            VOS_ASSERT(0); //should not hit this case.
        }
        else
        {
            /* allocate a buffer to hold the message */
            status = palAllocateMemory(pMac->hHdd, &pBuf, pHdr->MsgLen);
            if (eHAL_STATUS_SUCCESS == status)
            {
                /* copy the message into the buffer */
                palCopyMemory(pMac->hHdd, pBuf, msg, pHdr->MsgLen);

                /* attach the buffer to the defer queue */
                pMboxDQ->pMsgArray[writeIdx] = pBuf;

                /* increment the write index */
                writeIdx++;
                if (HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES == writeIdx)
                {
                    writeIdx = 0;
                }
                pMboxDQ->writeIdx = writeIdx;

                status = eHAL_STATUS_SUCCESS;
            }
        }
    }
    else
    {
        /* no need to defer */
        status = halMbox_SendMsg(pMac, msg);
    }

    /* release lock */
    vosStatus = vos_lock_release(&(pMbox[H2FW_MAILBOX].lock));
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        HALLOGE(halLog(pMac, LOGE, FL("failed to release VOS lock: 0x%lx\n"), (tANI_U32)vosStatus));
        VOS_ASSERT(0);
    }

    return status;
}

/** ----------------------------------------------------------
\fn      halMbox_RecvMsg
\brief   This function process messages received from mailbox
\param   tpAniSirGlobal  pMac
\return  status
\ ------------------------------------------------------------ */
eHalStatus halMbox_RecvMsg( tHalHandle hHal, tANI_BOOLEAN fProcessInContext )
{
    tANI_U32 readValue = 0;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tMBoxMsgHdr header, *pMsgHdr;
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    tANI_U8 *pMsg = NULL;

    tpMboxInfo  pMbox = (tpMboxInfo) pMac->hal.halMac.mboxInfo;
    pMsgHdr = (tMBoxMsgHdr *)&header;
    palZeroMemory( pMac->hHdd, (void *)pMsgHdr, sizeof(tMBoxMsgHdr));

    HALLOG3( halLog( pMac, LOG3,
            FL("Reading MailBox #%d, Control Register 0x%x\n"),
            FW2H_MAILBOX,
            MCU_MB_CONTROL_REGISTER( FW2H_MAILBOX )));

    // Increment Msg counter - for statistics
    pMbox[FW2H_MAILBOX].hostMsgSerialNum++;

    // Now, read "just" the message header
    // The entire message itself will be read and handled
    // by the corresponding handlers...
    retStatus = halReadDeviceMemory(pMac, pMbox[FW2H_MAILBOX].pMboxStartAddr, (tANI_U8 *) pMsgHdr, sizeof(tMBoxMsgHdr));

    if(retStatus != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE,
                FL("halReadDeviceMemory failed\n")));
        return eHAL_STATUS_FAILURE;
    }
	
    HALLOG3( halLog( pMac, LOG3, FL(" Type = %d, Len = %d\n"),
            pMsgHdr->MsgLen,
            pMsgHdr->MsgType ));

    // Allocate the memory for the message received
    retStatus = palAllocateMemory(pMac->hHdd, (void **)&pMsg,
                                  pMsgHdr->MsgLen);
    if(retStatus != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE,
                FL("Allocating memory for Mbox message failed\n")));
        return eHAL_STATUS_FAILED_ALLOC;
        }

    // Copy the entire message in to the buffer
    retStatus = halReadDeviceMemory(pMac, pMbox[FW2H_MAILBOX].pMboxStartAddr,
            pMsg, pMsgHdr->MsgLen);

    if(retStatus != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE,  FL("halReadDeviceMemory failed\n")));
        palFreeMemory(pMac->hHdd, pMsg);
        return eHAL_STATUS_FAILURE;
    }
	
    HALLOG1( halLog( pMac, LOG1,
            FL("Mbox %d interrupt received.\n"),
            FW2H_MAILBOX));

    if(fProcessInContext)
    {
        retStatus = halFW_HandleFwStatusMsg(pMac, pMsg);
        palFreeMemory(pMac->hHdd, pMsg);
    }
    else
    {
        // Defer the FW messages to MC thread
        if(pMac->gDriverType == eDRIVER_TYPE_MFG)
        {
            //posting a msg to halMsgQ does not work for FTM driver
            //invoke directly Fw msg handling
            retStatus = halFW_HandleFwMessages(pMac, pMsg);
        }
        else
        {
            retStatus = halFw_PostFwRspMsg(pMac, pMsg);
        }

        if(!HAL_STATUS_SUCCESS(retStatus))
            palFreeMemory(pMac->hHdd, pMsg);
    }

    // READ the "concerned" MB Control REG in order to increment the READ count again
    halReadRegister(pMac, MCU_MB_CONTROL_REGISTER( MCU_MAILBOX_FW2HOST ),
        &readValue );
    //write to the mailbox
    halWriteRegister(pMac,
                     MCU_MB_CONTROL_REGISTER(MCU_MAILBOX_FW2HOST),
                     QWLAN_MCU_MB1_CONTROL_INT_DIRECTION_MASK);

    return retStatus;
}

/** ----------------------------------------------------------
\fn      halMbox_SendMsgComplete
\brief   This function is handler for mailbox messages Send completion indication
\param   tpAniSirGlobal  pMac
\return  status
\ ------------------------------------------------------------ */
eHalStatus halMbox_SendMsgComplete( tHalHandle hHal )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tANI_U32 uRegVal;
    tpMboxInfo pMbox = (tpMboxInfo)pMac->hal.halMac.mboxInfo;
    tpMboxDeferQueue pMboxDQ = (tpMboxDeferQueue)pMac->hal.halMac.mboxDeferQueue;
    tANI_U32 readIdx;
    eHalStatus status;
    VOS_STATUS vosStatus;

    /* acquire lock */
    vosStatus = vos_lock_acquire(&(pMbox[H2FW_MAILBOX].lock));
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        HALLOGE(halLog(pMac, LOGE, FL("failed to acquire VOS lock: 0x%lx\n"), (tANI_U32)vosStatus));
        VOS_ASSERT(0);
        return eHAL_STATUS_FAILURE;
    }

    /* Reset Mailbox - Mark it as ready to send */
    pMbox[H2FW_MAILBOX].bMboxBusy = eANI_BOOLEAN_FALSE;

    //For FTM driver posing of the msg to MC thread does not work
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        halReadRegister(pMac, MCU_MB_CONTROL_REGISTER(MCU_MAILBOX_HOST2FW), &uRegVal);
    }

    /* send one message from the defer queue (if there are any) */
    readIdx = pMboxDQ->readIdx;
    if (NULL != pMboxDQ->pMsgArray[readIdx])
    {
        status = halMbox_SendMsg(pMac, pMboxDQ->pMsgArray[readIdx]);
        if (eHAL_STATUS_SUCCESS != status)
        {
            HALLOGE(halLog(pMac, LOGE, FL("failed to send off defer queue: 0x%lx\n"), (tANI_U32)status));
        }

        /* free the message buffer from the defer queue (only one try to send) */
        palFreeMemory(pMac->hHdd, pMboxDQ->pMsgArray[readIdx]);
        pMboxDQ->pMsgArray[readIdx] = NULL;

        /* increment the read index */
        readIdx++;
        if (HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES == readIdx)
        {
            readIdx = 0;
        }
        pMboxDQ->readIdx = readIdx;
    }

    /* release lock */
    vosStatus = vos_lock_release(&(pMbox[H2FW_MAILBOX].lock));
    if (!VOS_IS_STATUS_SUCCESS(vosStatus))
    {
        HALLOGE(halLog(pMac, LOGE, FL("failed to release VOS lock: 0x%lx\n"), (tANI_U32)vosStatus));
        VOS_ASSERT(0);
    }

    return eHAL_STATUS_SUCCESS;
}

/* ----------------------------------------------------------------
 * DEBUG FUNCTION:
 * ----------------------------------------------------------------
 */
eHalStatus halMboxDbg_writeTest(tpAniSirGlobal pMac, tANI_U32 nMboxNum, tANI_U32 fReset, tANI_U32 fIntrHost, tANI_U32 nValue){
    tANI_U32 value;
    if(nMboxNum >= MCU_MAX_NUM_OF_MAILBOX){
        HALLOGE(halLog(pMac, LOGE, FL("Mailbox %d not supported in Libra\n"), nMboxNum));

    }else{
      nValue &= ~3;
      if(fReset){
        //only reset the mailbox
        halWriteRegister(pMac,
                         MCU_MB_CONTROL_REGISTER(nMboxNum),
                         QWLAN_MCU_MB0_CONTROL_MB_RESET_MASK);
      }else{
        //write to the mailbox
        halWriteRegister(pMac,
                         MCU_MB_CONTROL_REGISTER(nMboxNum),
                         (nValue| (fIntrHost ? 0: QWLAN_MCU_MB0_CONTROL_INT_DIRECTION_MASK)));
      }
      //print the counter register
      halReadRegister(pMac, MCU_MB_CONTROL_COUNTER_REGISTER(nMboxNum), & value);
      HALLOGW( halLog(pMac, LOGW, FL("Mbox %d W-cnt: %d  R-cnt: %d\n"), nMboxNum, value >> 16, value & 0xffff));
    }
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halFw_PostSendMsgComplete(tpAniSirGlobal pMac)
{
    tSirMsgQ msg;
    tANI_U32 uRegVal;

    halReadRegister(pMac,
                    MCU_MB_CONTROL_REGISTER(MCU_MAILBOX_HOST2FW),
                    &uRegVal);

    msg.type     =  SIR_HAL_SEND_MSG_COMPLETE;
    msg.reserved = 0;
    msg.bodyptr  = NULL;
    msg.bodyval  = 0;

    if(halPostMsgApi(pMac,&msg) != eSIR_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Posting SIR_HAL_HANDLE_FW_MBOX_RSP msg failed")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGW(halLog(pMac, LOGW, FL("Posting SIR_HAL_HANDLE_FW_MBOX_RSP msg\n")));
    return eHAL_STATUS_SUCCESS;
}

eHalStatus halMbox_HandleInterrupt( tHalHandle hHal, eHalIntSources mbIntr )
{

    tANI_U32 MBoxNum = 0;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    MBoxNum = mbIntr - eHAL_INT_MCU_HOST_INT_MBOX0;
    HALLOG2( halLog( pMac, LOG2, FL("MB INTR [%d] triggered...\n"), MBoxNum ));

    switch (mbIntr) {
    case eHAL_INT_MCU_HOST_INT_MBOX1 : // FW2H_MAILBOX
        // Process Received mailbox message
        halMbox_RecvMsg( pMac, eANI_BOOLEAN_FALSE ); //The message will be processed in MC thread
        break;

    case eHAL_INT_MCU_HOST_INT_MBOX0 : // H2FW_MAILBOX
        //For FTM driver posing of the msg to MC thread does not work
        if(pMac->gDriverType == eDRIVER_TYPE_MFG)
        {
            halMbox_SendMsgComplete(pMac);
        }
        else
        {
            // Process Response received for mailbox Message send
            halFw_PostSendMsgComplete(pMac); //This message will be processed in MC thread
        }
        break;

    default:
        break;
        /* Not a valid interrupt */
    }

    return eHAL_STATUS_SUCCESS;
}



