
/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * hal.c:    HAL thread startup file.
 * Author:   V. K. Kandarpa
 * Date:     01/29/2002
 * Copy Right: AIRGO NETWORKS, PALO ALTO, CA
 * History:-
 * Date      Modified by            Modification Information
 * --------------------------------------------------------------------------
 * 10/30/2002   Rajesh Bhagwat        Moved halMmhForwardMBmsg() to API file
 * 10/30/2002   Rajesh Bhagwat        Modified halMmhPostMsgApi() to directly
 *                                    call host module which handles mailbox
 *                                    message receive for a Windows based MAC
 * --------------------------------------------------------------------------
 * */

/* Standard include files */
/* Application Specific include files */
#include "palTypes.h"
#include "sirApi.h"
#include "wniApi.h"
#include "sirCommon.h"
#include "palApi.h"
#include "limUtils.h"

#include "utilsApi.h"
#include "limApi.h"
#include "pmmApi.h"
#include "halCommonApi.h"
#include "halHddApis.h"
//#include "dphApi.h"
#include "cfgApi.h"
#include "halDebug.h"
//#include "arqApi.h"
#include "halGlobal.h"

/* Locally used Defines */

#define HAL_STAT_TIMER_VAL_IN_SECONDS 10 // 10 seconds

# define HAL_STAT_TIMER_VALUE \
         (HAL_STAT_TIMER_VAL_IN_SECONDS * SYS_TICKS_PER_SECOND)
# define HAL_TEMP_MEAS_TIMER_VALUE   (300 * SYS_TICKS_PER_SECOND) // 5 min.

/// Initial timer duration for the temperature monitoring timer, used for the open TPC
/// functionality.
#define HAL_OPEN_TPC_TEMP_MEAS_TIMER_VALUE   (120 * SYS_TICKS_PER_SECOND) // 120 secs.

# define HAL_AGC_RESET_TIMER_DEFAULT (20 / SYS_TICK_DUR_MS)       // 20 ms

# define HAL_2_MINUTE_COUNT (120 / HAL_STAT_TIMER_VAL_IN_SECONDS)


#ifdef VOSS_ENABLED
extern void SysProcessMmhMsg(tpAniSirGlobal pMac, tSirMsgQ* pMsg);
#else
void mbReceiveMBMsg( void *pAdapter, tSirMbMsg *pMsg );
#endif

// -------------------------------------------------------------
/**
 * halPostMsgApi
 *
 * FUNCTION:
 *     Posts HAL messages to gHAL thread
 *
 * LOGIC:
 *
 * ASSUMPTIONS:pl
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @param pMsg pointer with message
 * @return Success or Failure
 */

tSirRetStatus
halPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MQ_ID_HAL, (vos_msg_t *) pMsg))
        return eSIR_FAILURE;
    else
        return eSIR_SUCCESS;
} // halPostMsg()

// -------------------------------------------------------------
// MNT APIs
// -------------------------------------------------------------
/**
 * halMntPostMsgApi
 *
 * FUNCTION:
 *     Posts MNT messages to gSirMntMsgQ
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @param pMsg A pointer to the msg
 * @return Success or Failure
 */

tSirRetStatus
halMntPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tSirRetStatus rc = eSIR_SUCCESS;

    do
    {
#ifdef ANI_OS_TYPE_RTAI_LINUX

        // Posts message to the queue

        if (tx_queue_send(&pMac->sys.gSirMntMsgQ, pMsg,
                          TX_NO_WAIT) != TX_SUCCESS)
        {
            halLog(pMac, LOGP, FL("Queue send Failed! rc (%X)\n"),
                   eSIR_SYS_TX_Q_SEND_FAILED);
            rc = eSIR_SYS_TX_Q_SEND_FAILED;
            break;
        }

#else
        // For Windows based MAC, instead of posting message to different
        // queues we will call the handler routines directly

        halMntProcessMsgs(pMac, pMsg);
        rc = eSIR_SUCCESS;
#endif
    } while (0);

    return rc;
} // halMntPostMsg()

#ifndef WLAN_FTM_STUB
#include "pttModuleApi.h"
// -------------------------------------------------------------
/**
 * halNimPTTPostMsgApi
 *
 * FUNCTION:
 *     Posts NIM messages to gNIM thread
 *
 * LOGIC:
 *
 * ASSUMPTIONS:pl
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @param pMsg pointer with message
 * @return Success or Failure
 */

tSirRetStatus
halNimPTTPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tSirRetStatus rc = eSIR_SUCCESS;

    do
    {
#ifdef ANI_OS_TYPE_RTAI_LINUX

        // Posts message to the queue
        if (tx_queue_send(&pMac->sys.gSirNimRDMsgQ, pMsg,
                          TX_NO_WAIT) != TX_SUCCESS)
        {
            rc = eSIR_FAILURE;
            halLog(pMac, LOGP,
                   FL("Posting a Msg to nimMsgQ failed!\n"));
            break;
        }
#else
        // For Windows based MAC, instead of posting message to different
        // queues, we will call the handler routines directly
        halLog(pMac, LOGE, "ERROR: Received PTT message in obsolete code path.\n");
        halLog(pMac, LOGP, "This indicates that the wrong OID is being used - clean registry and previous inf files.\n");
/*
            tPttMsgbuffer *msgPtr = (tPttMsgbuffer *)(pMsg->body);  //for some reason, body is actually being used as if it were a void *
            pttProcessMsg(pMac, msgPtr);
*/

        //TODO: the resonse is now packaged in ((tPttMsgbuffer *)&pMsg->body)->msgResponse and needs to be sent back to the application


        rc = eSIR_SUCCESS;
#endif
    }
    while (0);

    return rc;
} // halNimPTTPostMsgApi()


#endif  //ANI_MANF_DIAG


// MMH APIs
// -------------------------------------------------------------
/**
 * halMmhPostMsgApi
 *
 * FUNCTION:
 *     Posts MMH messages to one of the three queues
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @param1 qType High priority, Protocol, and Debug/Log are the three types.
 * @param2 pMsg A pointer to the Message Queue
 * @return rc SUCCESS or Failure code
 */

tSirRetStatus
halMmhPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg, tANI_U8 qType)
{
  tSirRetStatus rc = eSIR_SUCCESS;
  HALLOG1( halLog(pMac, LOG1, FL("%s(%d)\n"), limMsgStr(pMsg->type),
         pMsg->type));
    do
    {

#ifdef ANI_OS_TYPE_RTAI_LINUX
        // RTAI
        if (tx_queue_send(&pMac->sys.gSirRxMsgQ, (void*)&pMsg->bodyptr,
                          TX_NO_WAIT) != TX_SUCCESS)
        {
            halLog(pMac, LOGP, FL("Queue send Failed!\n"));
            rc = eSIR_SYS_TX_Q_SEND_FAILED;
            break;
        }
        else
            tx_send_hdd_srq(pMac->rt);
#else
        // directly call the host module which handles mailbox receive

#ifdef VOSS_ENABLED
        /* On Gen6, VOSS does not copy over the message body
        ** so it should not be freed here
        */
        SysProcessMmhMsg(pMac, pMsg);
#else
        mbReceiveMBMsg( pMac->pAdapter, (tSirMbMsg *)pMsg->bodyptr );
        palFreeMemory( pMac->hHdd,  (tANI_U8*)(pMsg->bodyptr) );
#endif

        rc = eSIR_SUCCESS;
        break;
#endif  // == SIR_WINDOWS
    }
    while (0);

    return rc;

} // halMmhPostMsgApi


// -------------------------------------------------------------
/**
 * halMmhForwardMBmsg
 *
 * FUNCTION:
 *     Forwards the completely received message to the respective
 *    modules for further processing.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *    Freeing up of the message buffer is left to the destination module.
 *
 * NOTE:
 *  This function has been moved to the API file because for MAC running
 *  on Windows host, the host module will call this routine directly to
 *  send any mailbox messages. Making this function an API makes sure that
 *  outside world (any module outside MMH) only calls APIs to use MMH
 *  services and not an internal function.
 *
 * @param pMb A pointer to the maibox message
 * @return NONE
 */

tSirRetStatus halMmhForwardMBmsg(void* pSirGlobal, tSirMbMsg* pMb)
{
    tSirMsgQ msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pSirGlobal;

#ifdef ANI_OS_TYPE_RTAI_LINUX


    msg.type = pMb->type;
    msg.bodyptr = pMb;
    msg.bodyval = 0;
    HALLOG3( halLog(pMac, LOG3, FL("msgType %d, msgLen %d\n" ),
           pMb->type, pMb->msgLen));
#else

    tSirMbMsg* pMbLocal;
    msg.type = pMb->type;
    msg.bodyval = 0;

    HALLOG3(halLog(pMac, LOG3, FL("msgType %d, msgLen %d\n" ),
           pMb->type, pMb->msgLen));

    // copy the message from host buffer to firmware buffer
    // this will make sure that firmware allocates, uses and frees
    // it's own buffers for mailbox message instead of working on
    // host buffer

    // second parameter, 'wait option', to palAllocateMemory is ignored on Windows
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMbLocal, pMb->msgLen))
    {
        HALLOGE( halLog(pMac, LOGE, FL("Buffer Allocation failed!\n")));
        return eSIR_FAILURE;
    }

    palCopyMemory(pMac, (void *)pMbLocal, (void *)pMb, pMb->msgLen);
    msg.bodyptr = pMbLocal;
#endif


    switch (msg.type & HAL_MMH_MB_MSG_TYPE_MASK)
    {
        case SIR_HAL_MSG_TYPES_BEGIN:    // Posts a message to the HAL MsgQ
            halPostMsgApi(pMac, &msg);
            break;

        case SIR_LIM_MSG_TYPES_BEGIN:    // Posts a message to the LIM MsgQ
            limPostMsgApi(pMac, &msg);
            break;

        case SIR_CFG_MSG_TYPES_BEGIN:    // Posts a message to the CFG MsgQ
            halMntPostMsgApi(pMac, &msg);
            break;

        case SIR_PMM_MSG_TYPES_BEGIN:    // Posts a message to the PMM MsgQ
            pmmPostMessage(pMac, &msg);
            break;

#ifndef WLAN_FTM_STUB
        case SIR_PTT_MSG_TYPES_BEGIN:
            halNimPTTPostMsgApi(pMac, &msg); // Posts a message to the NIM PTT MsgQ
        break;

#endif

        default:
            HALLOGW( halLog(pMac, LOGW, FL("Unknown message type = " \
                   "0x%X\n"),
                   msg.type));

            // Release the memory.
            if (palFreeMemory( pMac->hHdd, (void*)(msg.bodyptr)) != eSIR_SUCCESS)
            {
                HALLOGE( halLog(pMac, LOGE, FL("Buffer Allocation failed!\n")));
                return eSIR_FAILURE;
            }
            break;
    }

    return eSIR_SUCCESS;

} // halMmhForwardMBmsg()

// -------------------------------------------------------------
/**
 * halMntProcessMsgs
 *
 * FUNCTION:
 *  Processes all the Maintenance messages.
 *
 * LOGIC
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @param pMsg A pointer to the message queue struct
 * @return Success or failure
 */
tSirRetStatus halMntProcessMsgs(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tSirRetStatus rc = eSIR_SUCCESS;

    HALLOG4( halLog(pMac, LOG4, FL("msgType (%X)\n"), pMsg->type));

    cfgProcessMbMsg(pMac, (tSirMbMsg*)pMsg->bodyptr);

    return rc;

} // halMntProcessMsgs()


