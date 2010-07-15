#ifndef _HALMAILBOX_H_
#define _HALMAILBOX_H_

/*===========================================================================

FILE:
   halMailbox.h

BRIEF DESCRIPTION:
   MailBox interface definition file.

DESCRIPTION:
   Mail box definition file for libra Hal.

                Copyright (c) 2008 QUALCOMM Incorporated.
                All Right Reserved.
                Qualcomm Confidential and Proprietary
===========================================================================*/


#include "aniGlobal.h"    // tpAniSirGlobal
#include "halTypes.h"     // eHalStatus, etc.
#include "halInterrupts.h"   // mailbox definitions
#include "halMsgApi.h"

/* Mail Box definitions */
#define HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES 20

typedef struct sMboxInfo {
    vos_lock_t lock;
    tANI_U32 pMboxStartAddr;
    tANI_U32 hostMsgSerialNum;
    tANI_U16 uMboxMsgSize;
    tANI_U16 bMboxBusy : 1;
} tMboxInfo, *tpMboxInfo;

typedef struct sMboxDeferQueue {
    tANI_U32 readIdx;
    tANI_U32 writeIdx;
    void *pMsgArray[HAL_NUM_MBOX_DEFER_QUEUE_ENTRIES];
} tMboxDeferQueue, *tpMboxDeferQueue;

/* Message header */
typedef struct sMBoxMsgHdr {
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 Ver: 6;     //version. Current should always be SMAC_HOSTMESG_VER
    tANI_U32 MsgType:10;  //Message type
    tANI_U32 MsgLen: 16; //Length of message. Start from ver field to the end of message.
#else
    tANI_U32 MsgLen: 16;
    tANI_U32 MsgType:10;
    tANI_U32 Ver: 6;
#endif
#ifdef ANI_BIG_BYTE_ENDIAN
    tANI_U32 MsgSerialNum:16;
    tANI_U32 SenderID:15;
    tANI_U32 RespNeeded: 1;//Response needed
#else
    tANI_U32 RespNeeded: 1;
    tANI_U32 SenderID:15;
    tANI_U32 MsgSerialNum:16;
#endif
    tANI_U32 Callback0; /* callback func ptr[3:0] */
    tANI_U32 Callback1; /* callback func ptr[7:4] */
}*tpMBoxMsgHdr, tMBoxMsgHdr;

#define HAL_INVALID_OFFSET 0xffff

/* Call back function definition */
typedef void (*tHalFwMsgCallback)(tpAniSirGlobal pMac, tANI_U32 mesgId, tANI_U32 senderID, tMBoxMsgHdr *msgHdr);

/* mbox api functions */
eHalStatus halMbox_Open(tHalHandle hHal, void *arg);
eHalStatus halMbox_Start(tHalHandle hHal, void *arg);
eHalStatus halMbox_Stop(tHalHandle hHal, void *arg);
eHalStatus halMbox_Close(tHalHandle hHal, void *arg);
eHalStatus halMbox_SendMsg(tpAniSirGlobal pMac, void *msg);
eHalStatus halMbox_SendReliableMsg(tpAniSirGlobal pMac, void *msg);

#define HALMBOX_CHECKMSG_RESULT_UNKNOWN 0
#define HALMBOX_CHECKMSG_RESULT_SUCCESS 1
#define HALMBOX_CHECKMSG_RESULT_FAILED -1

eHalStatus halMbox_checkMsgProcessed(tpAniSirGlobal pMac, tANI_U32 msgSerialNum, tANI_S32 *result);
eHalStatus halMbox_RecvMsg( tHalHandle hHal, tANI_BOOLEAN fProcessInContext );
eHalStatus halMbox_PollMsg(tpAniSirGlobal  pMac, tANI_U32 mboxNum);
eHalStatus halMbox_HandleInterrupt( tHalHandle pMac, eHalIntSources mbIntr );

eHalStatus halMbox_SendMsgComplete( tHalHandle hHal );

/* Debug Functions */
eHalStatus halMboxDbg_writeTest(tpAniSirGlobal pMac, tANI_U32 nMboxNum, tANI_U32 fReset, tANI_U32 fIntrHost, tANI_U32 nValue);

// -------------------------------------------------------------------------
#endif /* _HALMAILBOX_H_ */


