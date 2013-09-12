/*
* Copyright (c) 2013 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

/*
 * This file limRMC.c contains the code
 * for processing Leader-based Protocol messages to support Reliable multicast
 *
 */
#include "wniApi.h"
#include "wniCfgSta.h"
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limUtils.h"
#include "limTimerUtils.h"
#include "limSendMessages.h"
#include "limSendMessages.h"
#include "limSession.h"
#include "limSessionUtils.h"
#include "wlan_qct_wda.h"
#include "wlan_qct_tli.h"
#include "limRMC.h"

/**
 * DOC: Leader Based Protocol for Reliable Multicast
 *
 * This protocol proposes to achieve reliability in multicast transmissions
 * by having a selected multicast receiver respond with 802.11 ACKs.
 * This is designed for a peer to peer application that uses the underlying
 * IBSS network. The STAs in the IBSS network perform the following different
 * roles to support this protocol -
 *
 * 1) Multicast Transmitter:
 *      A node that delivers MCAST packets to every nodes and performs Reliable
 *      Multicast algorithm as a transmitter.
 * 2) Multicast Receiver:
 *      All nodes that receive MCAST packets
 * 3) Multicast Receiver Leader:
 *      A node that receives MCAST packets and performs a Reliable Multicast
 *      algorithm by sending ACK to transmitter for every multicast frame
 *      received. Multicast Receiver Leader is appointed by the Multicast
 *      Transmitter.
 *
 * The implementation in this file supports the roles of both Multicast
 *  Transmitter and the Multicast Receiver Leader.
 *
 * The firmware performs the Leader Selection algorithm and provides a candidate
 * list. The implementation in this file, sends vendor specific 802.11 Action
 * frame to notify the selected Multicast leader.
 *
 * The leader sets up its data path to send 802.11 ACKs for any received
 * Multicast frames belonging to the specified Multicast Group. It then sends an
 * Action frame to the transmitter to acknowledge that it has accepted the
 * Leader role.
 *
 * On receiving an acknowledgement from the leader, the transmitter sets up its
 * data path to expect 802.11 ACKs for Multicast transmissions.
 *
 * The function limProcessRMCMessages handles messages from HDD to enable or
 * disable this protocol for a Multicast Group.  It handles 802.11 Action frame
 * receive events for this protocol.  It also responds to firmware generated
 * indications and events.
 */

#if defined WLAN_FEATURE_RELIABLE_MCAST

/*
 *  RMC utility routines
 */

/**
 * __rmcGroupHashFunction()
 *
 *FUNCTION:
 * This function is called during scan hash entry operations
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  groupAddr - multicast group address
 *         transmitter - address of multicast transmitter
 *
 * @return Hash index
 */

static tANI_U8
__rmcGroupHashFunction(tSirMacAddr mcastGroupAddr, tSirMacAddr transmitter)
{
    tANI_U16 hash;

    /*
     * Generate a hash using both group address and the
     * transmitter address
     */
    hash = mcastGroupAddr[3] + mcastGroupAddr[4] + mcastGroupAddr[5];
    hash += transmitter[3] + transmitter[4] + transmitter[5];

    return hash & (RMC_MCAST_GROUPS_HASH_SIZE - 1);
}

/**
 *  __rmcGroupLookupHashEntry()
 *
 *FUNCTION:
 * This function is called to lookup RMC group entries
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *  Should be called with lkRmcLock held.
 *
 *NOTE:
 * NA
 *
 * @param  groupAddr - multicast group address
 *         transmitter - address of multicast transmitter
 *         role - transmitter or leader
 *
 * @return pointer to tLimRmcGroupContext
 */

static tLimRmcGroupContext *
__rmcGroupLookupHashEntry(tpAniSirGlobal pMac, tSirMacAddr mcastGroupAddr,
                          tSirMacAddr transmitter, eRmcRole role)
{
    tANI_U8 index;
    tLimRmcGroupContext *entry;

    index = __rmcGroupHashFunction(mcastGroupAddr, transmitter);

    /* Pick the correct hash table based on role */
    entry = (eRMC_TRANSMITTER_ROLE == role) ?
                 pMac->rmcContext.rmcGroupTxHashTable[index] :
                 pMac->rmcContext.rmcGroupRxHashTable[index];

    PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Lookup:[%d] group " MAC_ADDRESS_STR
                         " transmitter " MAC_ADDRESS_STR " entry %p"), index,
                         MAC_ADDR_ARRAY(mcastGroupAddr),
                         MAC_ADDR_ARRAY(transmitter), entry);)
    while (entry)
    {
        if ((vos_mem_compare(mcastGroupAddr,
            entry->groupAddr, sizeof(v_MACADDR_t))) &&
            (vos_mem_compare(transmitter, entry->transmitter,
             sizeof(v_MACADDR_t))))
        {
            return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

/**
 *  __rmcGroupInsertHashEntry()
 *
 *FUNCTION:
 * This function is called to insert RMC group entry
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *  Should be called with lkRmcLock held.
 *
 *NOTE:
 * NA
 *
 * @param  groupAddr - multicast group address
 *         transmitter - address of multicast transmitter
 *         role - transmitter or leader
 *
 * @return pointer to tLimRmcGroupContext
 */
static tLimRmcGroupContext *
__rmcGroupInsertHashEntry(tpAniSirGlobal pMac, tSirMacAddr mcastGroupAddr,
                          tSirMacAddr transmitter, eRmcRole role)
{
    tANI_U8 index;
    tLimRmcGroupContext *entry;
    tLimRmcGroupContext **head;

    index = __rmcGroupHashFunction(mcastGroupAddr, transmitter);

    PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Insert:[%d] group " MAC_ADDRESS_STR
                             " transmitter " MAC_ADDRESS_STR), index,
                             MAC_ADDR_ARRAY(mcastGroupAddr),
                             MAC_ADDR_ARRAY(transmitter));)

    /* Pick the correct hash table based on role */
    head = (eRMC_TRANSMITTER_ROLE == role) ?
                 &pMac->rmcContext.rmcGroupTxHashTable[index] :
                 &pMac->rmcContext.rmcGroupRxHashTable[index];

    entry = __rmcGroupLookupHashEntry(pMac, mcastGroupAddr, transmitter, role);

    if (entry)
    {
        /* If the entry exists, return it at the end */
        PELOGE(limLog(pMac, LOGE, FL("RMC: Hash Insert:"
                 MAC_ADDRESS_STR "exists"), MAC_ADDR_ARRAY(mcastGroupAddr));)
    }
    else
    {
        entry = (tLimRmcGroupContext *)vos_mem_malloc(sizeof(*entry));

        PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Insert:new entry %p"), entry);)

        if (entry)
        {
            vos_mem_copy(entry->groupAddr, mcastGroupAddr, sizeof(tSirMacAddr));
            vos_mem_copy(entry->transmitter, transmitter, sizeof(tSirMacAddr));
            entry->state = eRMC_LEADER_NOT_SELECTED;
            entry->isLeader = eRMC_IS_NOT_A_LEADER;
            entry->leader_index = 0;

            /* chain this entry */
            entry->next = *head;
            *head = entry;
        }
        else
        {
            PELOGE(limLog(pMac, LOGE, FL("RMC: Hash Insert:" MAC_ADDRESS_STR
                             " alloc failed"), MAC_ADDR_ARRAY(mcastGroupAddr));)
        }
    }

    return entry;
}

/**
 *  __rmcGroupDeleteHashEntry()
 *
 *FUNCTION:
 * This function is called to delete a RMC group entry
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *  Should be called with lkRmcLock held.
 *
 *NOTE:
 * Make sure (for the transmitter role) that the entry is
 * not in the Pending Response queue.
 *
 * @param  groupAddr - multicast group address
 *         transmitter - address of multicast transmitter
 *         role - transmitter or leader
 *
 * @return status
 */
static tSirRetStatus
__rmcGroupDeleteHashEntry(tpAniSirGlobal pMac, tSirMacAddr mcastGroupAddr,
                          tSirMacAddr transmitter, eRmcRole role)
{
    tSirRetStatus status = eSIR_FAILURE;
    tANI_U8 index;
    tLimRmcGroupContext *entry, *prev, **head;

    index = __rmcGroupHashFunction(mcastGroupAddr, transmitter);

    /* Pick the correct hash table based on role */
    head = (eRMC_TRANSMITTER_ROLE == role) ?
                 &pMac->rmcContext.rmcGroupTxHashTable[index] :
                 &pMac->rmcContext.rmcGroupRxHashTable[index];
    entry = *head;
    prev = NULL;

    while (entry)
    {
        if ((vos_mem_compare(mcastGroupAddr,
            entry->groupAddr, sizeof(v_MACADDR_t))) &&
            (vos_mem_compare(transmitter, entry->transmitter,
             sizeof(v_MACADDR_t))))
        {
            if (*head == entry)
            {
                *head = entry->next;
            }
            else
            {
                prev->next = entry->next;
            }

            PELOG1(limLog(pMac, LOG1, FL("RMC: Hash Delete: entry %p group "
                         MAC_ADDRESS_STR " transmitter " MAC_ADDRESS_STR),
                             entry, MAC_ADDR_ARRAY(mcastGroupAddr),
                             MAC_ADDR_ARRAY(transmitter));)

            /* free the group entry */
            vos_mem_free(entry);

            status = eSIR_SUCCESS;
            break;
        }

        prev = entry;
        entry = entry->next;
    }

    return status;
}

/* End RMC utility routines */

/**
 * \brief Send WDA_RMC_LEADER_REQ to HAL, in order
 * to request for a Multicast Leader selection.
 *
 * \sa __limPostMsgLeaderReq
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param cmd SUGGEST leader or BECOME leader
 *
 * \param mcastGroup Multicast Group address
 *
 * \param mcastTransmitter Multicast Transmitter address

 * \return none
 *
 */
static void
__limPostMsgLeaderReq ( tpAniSirGlobal pMac,
                        tANI_U8 cmd,
                        tSirMacAddr mcastGroup,
                        tSirMacAddr mcastTransmitter)
{
    tSirMsgQ msg;
    tSirRmcLeaderReq *pLeaderReq;

    pLeaderReq = vos_mem_malloc(sizeof(*pLeaderReq));
    if (NULL == pLeaderReq)
    {
       limLog(pMac, LOGE, FL("AllocateMemory() failed"));
       return;
    }

    pLeaderReq->cmd = cmd;

    vos_mem_copy(pLeaderReq->mcastTransmitter, mcastTransmitter,
                 sizeof(tSirMacAddr));
    vos_mem_copy(pLeaderReq->mcastGroup, mcastGroup, sizeof(tSirMacAddr));

    /* Initialize black list */
    vos_mem_zero(pLeaderReq->blacklist, sizeof(pLeaderReq->blacklist));

    /*
     * If there are a list of STA receivers that we do not want to be
     * considered for Leader, send it here.
     */
    if (eRMC_SUGGEST_LEADER_CMD == cmd)
    {
        /* TODO - Set the black list. */
    }

    msg.type = WDA_RMC_LEADER_REQ;
    msg.bodyptr = pLeaderReq;
    msg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));
    if (eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
    {
        vos_mem_free(pLeaderReq);
        limLog(pMac, LOGE, FL("wdaPostCtrlMsg() failed"));
    }

    return;
}

/**
 * \brief Send WDA_RMC_UPDATE_IND to HAL, in order
 * to request for a Multicast Leader selection.
 *
 * \sa __limPostMsgUpdateInd
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param indication Accepted or Cancelled
 *
 * \param role Leader or Transmitter
 *
 * \param mcastGroup Multicast Group address
 *
 * \param mcastTransmitter Multicast Transmitter address
 *
 * \param mcastLeader Multicast Leader address
 *
 * \return none
 *
 */
static void
__limPostMsgUpdateInd ( tpAniSirGlobal pMac,
                        tANI_U8 indication,
                        tANI_U8 role,
                        tSirMacAddr mcastGroup,
                        tSirMacAddr mcastTransmitter,
                        tSirMacAddr mcastLeader)
{
    tSirMsgQ msg;
    tSirRmcUpdateInd *pUpdateInd;

    pUpdateInd = vos_mem_malloc(sizeof(*pUpdateInd));
    if ( NULL == pUpdateInd )
    {
       limLog(pMac, LOGE, FL("AllocateMemory() failed"));
       return;
    }

    vos_mem_zero(pUpdateInd, sizeof(*pUpdateInd));

    pUpdateInd->indication = indication;
    pUpdateInd->role = role;

    vos_mem_copy(pUpdateInd->mcastTransmitter,
            mcastTransmitter, sizeof(tSirMacAddr));

    vos_mem_copy(pUpdateInd->mcastGroup, mcastGroup, sizeof(tSirMacAddr));

    vos_mem_copy(pUpdateInd->mcastLeader, mcastLeader, sizeof(tSirMacAddr));


    msg.type = WDA_RMC_UPDATE_IND;
    msg.bodyptr = pUpdateInd;
    msg.bodyval = 0;

    MTRACE(macTraceMsgTx(pMac, NO_SESSION, msg.type));
    if (eSIR_SUCCESS != wdaPostCtrlMsg(pMac, &msg))
    {
        vos_mem_free(pUpdateInd);
        limLog(pMac, LOGE, FL("wdaPostCtrlMsg() failed"));
    }

    return;
}

static char *
__limLeaderMessageToString(eRmcMessageType msgType)
{
    switch (msgType)
    {
        default:
            return "Invalid";
        case eLIM_RMC_ENABLE_REQ:
            return "RMC_ENABLE_REQ";
        case eLIM_RMC_DISABLE_REQ:
            return "RMC_DISABLE_REQ";
        case eLIM_RMC_LEADER_SELECT_RESP:
            return "RMC_LEADER_SELECT_RESP";
        case eLIM_RMC_LEADER_PICK_NEW:
            return "RMC_LEADER_PICK_NEW";
        case eLIM_RMC_OTA_LEADER_INFORM_ACK:
            return "RMC_OTA_LEADER_INFORM_ACK";
        case eLIM_RMC_OTA_LEADER_INFORM_SELECTED:
            return "RMC_OTA_LEADER_INFORM_SELECTED";
        case eLIM_RMC_BECOME_LEADER_RESP:
            return "RMC_BECOME_LEADER_RESP";
        case eLIM_RMC_OTA_LEADER_INFORM_CANCELLED:
            return "RMC_OTA_LEADER_INFORM_CANCELLED";
    }
}

static char *
__limLeaderStateToString(eRmcLeaderState state)
{
    switch (state)
    {
        default:
            return "Invalid";
        case eRMC_IS_NOT_A_LEADER:
            return "Device Not a Leader";
        case eRMC_LEADER_PENDING:
            return "Pending firmware resp";
        case eRMC_IS_A_LEADER:
            return "Device is Leader";
    }
}

static char *
__limMcastTxStateToString(eRmcMcastTxState state)
{
    switch (state)
    {
        default:
            return "Invalid";
        case eRMC_LEADER_NOT_SELECTED:
            return "Not Selected";
        case eRMC_LEADER_ENABLE_REQUESTED:
            return "Enable Requested";
        case eRMC_LEADER_OTA_REQUEST_SENT:
            return "OTA Request Sent";
        case eRMC_LEADER_ACTIVE:
            return "Active";
    }
}

/*
 * RMC Pending Response routines
 *
 * The pending response queue maintains a queue of outstanding LEADER_INFORM
 * requests sent to candidate leaders.  If a LEADER_INFORM_ACK is not received
 * within the configured period, we attempt to request other leaders from the
 * firmware provided list of candidate leaders.
 * If all the receivers from the leader list are exhausted, we disable RMC
 * for the multicast group.
 *
 * The __rmcPendingRespQueueAdd is called when a LEADER_INFORM request is sent.
 * For normal operations, it is expected that the LEADER_INFORM_ACK will be
 * received, at which point we call __rmcPendingRespQueueRemove.
 */

/**
 * __rmcPendingRespQueueAdd()
 *
 *FUNCTION:
 * This function is called to add a new entry to the pending
 * queue.
 *
 *LOGIC:  If the list is empty, add the new item, and start the
 *        timer. If the list it not empty, simply add the new
 *        entry to the list.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  param - Message corresponding to the timer that expired
 *
 * @return None
 */
void
__rmcPendingRespQueueAdd(tpAniSirGlobal pMac, tSirMacAddr mcastGroupAddr,
                            tSirMacAddr transmitter)
{
    tLimRmcGroupContext *entry, *new_entry;

    /* Find the group entry */
    new_entry = __rmcGroupLookupHashEntry(pMac, mcastGroupAddr,
                            transmitter, eRMC_TRANSMITTER_ROLE);

    if (!new_entry)
    {
        limLog(pMac, LOGE,
            FL("__rmcPendingRespQueueAdd Group entry not found"));
        return;
    }

    if (pMac->rmcContext.pendingRespQueue)
    {
        /*
         * Add the new entry at the end of the queue.
         * The timer should already be active.
         */
        entry = pMac->rmcContext.pendingRespQueue;
        while (entry->next_pending)
        {
            entry = entry->next_pending;
        }
        entry->next_pending = new_entry;
        new_entry->next_pending = NULL;
    }
    else
    {
        /*
         * The list is empty, so add it to the pending queue and
         * activate the timer.
         */
        pMac->rmcContext.pendingRespQueue = new_entry;
        new_entry->next_pending = NULL;
        if (tx_timer_activate(&pMac->rmcContext.gRmcResponseTimer)!= TX_SUCCESS)
        {
            limLog(pMac, LOGE,
             FL("__rmcPendingRespQueueAdd:Activate RMC Response timer failed"));
        }
    }
}

/**
 * __rmcPendingRespQueueRemove()
 *
 *FUNCTION:
 * This function is called to remove a new entry to the pending
 * queue.
 *
 *LOGIC:  If the list is empty after removal, deactivate the timer.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  tpAniSirGlobal and tLimRmcGroupContext
 *
 * @return None
 */
void
__rmcPendingRespQueueRemove(tpAniSirGlobal pMac, tSirMacAddr mcastGroupAddr,
                            tSirMacAddr transmitter)
{
    tLimRmcGroupContext *entry, *prev = NULL;

    entry = pMac->rmcContext.pendingRespQueue;

    while (entry)
    {
        if (vos_mem_compare(mcastGroupAddr,
            entry->groupAddr, sizeof(v_MACADDR_t)))
        {
            /* Check if the first node is being removed.*/
            if (NULL == prev)
            {
                pMac->rmcContext.pendingRespQueue = entry->next_pending;
            }
            else
            {
                prev->next_pending = entry->next_pending;
            }

            break;
        }

        prev = entry;
        entry = entry->next_pending;
    }

    return;
}

/**
 * __rmcResponseTimerHandler()
 *
 *FUNCTION:
 * This function is called upon timer expiry.
 *
 *LOGIC:  This function handles unacked LEADER_INFORM messages.
 *        If a leader fails to respond, it tries the next one in
 *        the list.  If all potential leaders are exhausted, the
 *        multicast group is removed.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * Only one entry is processed for every invocation if this routine.
 * This allows us to use a single timer and makes sure we do not
 * timeout a request too early.
 *
 * @param  param - Message corresponding to the timer that expired
 *
 * @return None
 */

void
__rmcResponseTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;
    tLimRmcGroupContext *pending;
    tLimRmcGroupContext *new_head;
    tSirRetStatus status;

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("__rmcResponseTimerHandler lock acquire failed"));
        if (tx_timer_activate(&pMac->rmcContext.gRmcResponseTimer)!= TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not activate RMC Response timer"));
        }
        return;
    }

    pending = pMac->rmcContext.pendingRespQueue;

    /*
     * Check if there is anything to process.
     * Do not reschedule timer if nothing is pending.
     */
    if (NULL == pending)
    {
        if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
        {
            limLog(pMac, LOGE,
                FL("RMC: __rmcResponseTimerHandler lock release failed"));
        }
        return;
    }

    new_head = pending->next_pending;

    /*
     * Handle multicast groups that are waiting for LEADER_INFORM_ACK.
     */
    if (eRMC_LEADER_OTA_REQUEST_SENT == pending->state)
    {
        tANI_U8 next_leader_index =
                 (pending->leader_index + 1) & (SIR_RMC_NUM_MAX_LEADERS - 1);
        tSirMacAddr zeroMacAddr = { 0, 0, 0, 0, 0, 0 };
        tSirMacAddr *leader = &pending->leaderList[pending->leader_index];

        limLog(pMac, LOGE,
               FL("RMC Response timeout for" MAC_ADDRESS_STR
                   "leader" MAC_ADDRESS_STR),
                   MAC_ADDR_ARRAY(pending->groupAddr), MAC_ADDR_ARRAY(*leader));

        /*
         * If we have wrapped around on the leader_index, or we have
         * exhausted our leader list, we disable RMC for this mcast group.
         */
        if (next_leader_index !=  0 &&
            (VOS_FALSE == vos_mem_compare(&zeroMacAddr,
                            &pending->leaderList[next_leader_index],
                            sizeof(tSirMacAddr))))
        {
            tSirRMCInfo RMC;

            /*
             * Send Leader_Inform Action frame to the candidate leader.
             * Candidate leader is at leader_index.
             */
            RMC.dialogToken = 0;
            RMC.action = SIR_MAC_RMC_LEADER_INFORM_SELECTED;
            vos_mem_copy(&RMC.mcastGroup, &pending->groupAddr,
                         sizeof(tSirMacAddr));

            status = limSendRMCActionFrame(pMac,
                          pending->leaderList[next_leader_index],
                          &RMC,
                          pending->psessionEntry);

            if (eSIR_FAILURE == status)
            {
                PELOGE(limLog(pMac, LOGE,
                 FL("RMC:__rmcResponseTimerHandler Action frame send failed"));)
            }

            pending->leader_index = next_leader_index;
            pending->state = eRMC_LEADER_OTA_REQUEST_SENT;

            leader = &pending->leaderList[next_leader_index];
            limLog(pMac, LOGE,
               FL("RMC Response timeout trying new leader " MAC_ADDRESS_STR),
                     *leader[0], *leader[1], *leader[2],
                     *leader[3], *leader[4], *leader[5]);
            /*
             * Re-arm timer and keep the same pending head
             */
            if (tx_timer_activate(&pMac->rmcContext.gRmcResponseTimer)!=
                TX_SUCCESS)
            {
                limLog(pMac, LOGE, FL("could not activate RMC Response timer"));
            }

            if (!VOS_IS_STATUS_SUCCESS
                    (vos_lock_release(&pMac->rmcContext.lkRmcLock)))
            {
                limLog(pMac, LOGE,
                    FL("RMC: __rmcResponseTimerHandler lock release failed"));
            }

            return;
        }
    }


    limLog(pMac, LOGE,
           FL("RMC:All attempts exhausted for" MAC_ADDRESS_STR),
           pending->groupAddr[0], pending->groupAddr[1], pending->groupAddr[2],
           pending->groupAddr[3], pending->groupAddr[4], pending->groupAddr[5]);

    /*
     * Delete hash entry for this Group address.
     */
    status = __rmcGroupDeleteHashEntry(pMac, pending->groupAddr,
                         pending->transmitter, eRMC_TRANSMITTER_ROLE);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
                 FL("RMC: __rmcResponseTimerHandler delete failed"));)
    }

    /*
     * Update the pending queue
     */
    pMac->rmcContext.pendingRespQueue = new_head;

    /*
     * Re-arm timer if the queue has items
     */
    if (pMac->rmcContext.pendingRespQueue)
    {
        if (tx_timer_activate(&pMac->rmcContext.gRmcResponseTimer)!= TX_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not activate RMC Response timer"));
        }
    }

    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: __rmcResponseTimerHandler lock release failed"));
    }

    return;
}

/**
 * __limProcessRMCEnableRequest()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_ENABLE_REQ
 * message from SME.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCEnableRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSetRMCReq *setRmcReq = (tSirSetRMCReq *)pMsgBuf;
    tLimRmcGroupContext *entry;
    tSirMacAddr mcastGroup;

    if (!setRmcReq)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Enable:NULL message") );)
        return;
    }

    /*
     * Convert Multicast IP address to the Multicast Group MAC Address
     * (01-00-5E-xx-xx-xx).
     */
    mcastGroup[0] = 0x01;
    mcastGroup[1] = 0x00;
    mcastGroup[2] = 0x5e;
    mcastGroup[3] = setRmcReq->mcastGroupIpAddr[1] & 0x7f; // mask off bit-23
    mcastGroup[4] = setRmcReq->mcastGroupIpAddr[2];
    mcastGroup[5] = setRmcReq->mcastGroupIpAddr[3];

    /*
     * Insert an entry for this Multicast Group address
     */
    entry = __rmcGroupInsertHashEntry(pMac, mcastGroup,
                     setRmcReq->mcastTransmitter, eRMC_TRANSMITTER_ROLE);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Enable:Hash insert failed"));)
        return;
    }

    /* Send LBP_LEADER_REQ to f/w */
    __limPostMsgLeaderReq(pMac, eRMC_SUGGEST_LEADER_CMD, mcastGroup,
                        setRmcReq->mcastTransmitter);

    // TODO start timer

    entry->state = eRMC_LEADER_ENABLE_REQUESTED;
}

/**
 * __limProcessRMCDisableRequest()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_DISABLE_REQ
 * message from SME.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCDisableRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpPESession psessionEntry;
    tSirRMCInfo RMC;
    tSirSetRMCReq *setRmcReq = (tSirSetRMCReq *)pMsgBuf;
    tSirMacAddr mcastGroup;
    tSirRetStatus status;
    tLimRmcGroupContext *entry;
    v_PVOID_t pvosGCtx;
    VOS_STATUS vos_status;
    v_MACADDR_t vosMcast;

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Disable:No active IBSS"));)
        return;
    }

    if (!setRmcReq)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Disable:NULL message") );)
        return;
    }

    /*
     * Convert Multicast IP address to the Multicast Group MAC Address
     * (01-00-5E-xx-xx-xx).
     */
    mcastGroup[0] = 0x01;
    mcastGroup[1] = 0x00;
    mcastGroup[2] = 0x5e;
    mcastGroup[3] = setRmcReq->mcastGroupIpAddr[1] & 0x7f; // mask off bit-23
    mcastGroup[4] = setRmcReq->mcastGroupIpAddr[2];
    mcastGroup[5] = setRmcReq->mcastGroupIpAddr[3];

    /* Copy it into v_MACADDR_t format for TL */
    vosMcast.bytes[0] = mcastGroup[0];
    vosMcast.bytes[1] = mcastGroup[1];
    vosMcast.bytes[2] = mcastGroup[2];
    vosMcast.bytes[3] = mcastGroup[3];
    vosMcast.bytes[4] = mcastGroup[4];
    vosMcast.bytes[5] = mcastGroup[5];

    /* Disable RMC in TL */
    pvosGCtx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *) pMac);
    vos_status = WLANTL_DisableReliableMcast(pvosGCtx, &vosMcast);

    if (VOS_STATUS_SUCCESS != vos_status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Disable: TL disable failed"));)
    }

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Disable lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac, mcastGroup,
                        setRmcReq->mcastTransmitter, eRMC_TRANSMITTER_ROLE);

    if (!entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Disable: No entry"));)
        goto done;
    }

    if (entry->state == eRMC_LEADER_ACTIVE)
    {
        /*
         * Send Leader_Inform_Cancelled Action frame to the Leader.
         */
        RMC.dialogToken = 0;
        RMC.action = SIR_MAC_RMC_LEADER_INFORM_CANCELLED;
        vos_mem_copy(&RMC.mcastGroup, &mcastGroup, sizeof(tSirMacAddr));

        status = limSendRMCActionFrame(pMac, entry->leaderList[entry->leader_index],
                             &RMC, psessionEntry);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE, FL("RMC:Disable: Action frame send failed"));)
        }
    }

    /* send LBP_UPDATE_IND */
    __limPostMsgUpdateInd(pMac, eRMC_LEADER_CANCELLED, eRMC_TRANSMITTER_ROLE,
                         mcastGroup, setRmcReq->mcastTransmitter,
                         entry->leaderList[entry->leader_index]);

    /*
     * Remove  the groupfrom pending queue if present.
     */
    __rmcPendingRespQueueRemove(pMac, mcastGroup, setRmcReq->mcastTransmitter);

    /*
     * Delete hash entry for this Group address.
     */
    status = __rmcGroupDeleteHashEntry(pMac, mcastGroup,
                        setRmcReq->mcastTransmitter, eRMC_TRANSMITTER_ROLE);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Disable:hash delete failed"));)
    }

done:
    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC: Disable: lock release failed"));
    }
}

/**
 * __limProcessRMCLeaderSelectResponse()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_LEADER_SELECT_RESP
 * message from the firmware.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCLeaderSelectResponse(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcLeaderSelectInd *pRmcLeaderSelectInd;
    tLimRmcGroupContext *entry;
    tpPESession psessionEntry;
    tSirRetStatus status;
    tSirRMCInfo RMC;

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Select_Resp:NULL message"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Leader_Select_Resp:No active IBSS"));)
        return;
    }

    pRmcLeaderSelectInd = (tSirRmcLeaderSelectInd *)pMsgBuf;

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Leader_Select_Resp:lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac,
                       pRmcLeaderSelectInd->mcastGroup,
                       pRmcLeaderSelectInd->mcastTransmitter,
                       eRMC_TRANSMITTER_ROLE);
    if (!entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Leader_Select_Resp: No entry"));)
        /*
         * Not marking error because there is nothing to delete from hash.
         */
        goto done;
    }

    if (entry->state != eRMC_LEADER_ENABLE_REQUESTED)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Select_Resp:Bad state %s"),
                        __limMcastTxStateToString(entry->state) );)
        status = eSIR_FAILURE;
        goto done;
    }

    if (pRmcLeaderSelectInd->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Leader_Select_Resp:FW Status %d"),
                        pRmcLeaderSelectInd->status);)
        status = eSIR_FAILURE;
        goto done;
    }

    entry->psessionEntry = psessionEntry;

    RMC.dialogToken = 0;
    RMC.action = SIR_MAC_RMC_LEADER_INFORM_SELECTED;
    vos_mem_copy(&RMC.mcastGroup, &pRmcLeaderSelectInd->mcastGroup,
                 sizeof(tSirMacAddr));

    /* Cache the leader list for this multicast group */
    vos_mem_copy(entry->leaderList, pRmcLeaderSelectInd->leader,
                 sizeof(entry->leaderList));

    PELOG1(limLog(pMac, LOG1, FL("RMC: Leader_Select :leader " MAC_ADDRESS_STR),
             MAC_ADDR_ARRAY(pRmcLeaderSelectInd->leader[entry->leader_index]));)
    /*
     * Send Leader_Inform Action frame to the candidate leader.
     * Candidate leader is at leader_index.
     */
    status = limSendRMCActionFrame(pMac,
                          pRmcLeaderSelectInd->leader[entry->leader_index],
                          &RMC,
                          psessionEntry);

    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
         FL("RMC: Leader_Select_Resp: Action send failed"));)
    }

    entry->state = eRMC_LEADER_OTA_REQUEST_SENT;

    /* Add request to the pending response queue. */
    __rmcPendingRespQueueAdd(pMac, pRmcLeaderSelectInd->mcastGroup,
                       pRmcLeaderSelectInd->mcastTransmitter);

done:
    if (eSIR_FAILURE == status)
    {
        status = __rmcGroupDeleteHashEntry(pMac,
                       pRmcLeaderSelectInd->mcastGroup,
                       pRmcLeaderSelectInd->mcastTransmitter,
                       eRMC_TRANSMITTER_ROLE);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE,
                      FL("RMC: Leader_Select_Resp:hash delete failed"));)
        }
    }

    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Leader_Select_Resp: lock release failed"));
    }
}

/**
 * __limProcessRMCLeaderPickNew()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_LEADER_PICK_NEW
 * message from the firmware.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCLeaderPickNew(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcUpdateInd *pRmcUpdateInd;
    tLimRmcGroupContext *entry;
    tpPESession psessionEntry;
    tSirRetStatus status;
    tSirRMCInfo RMC;

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Pick_New:NULL message"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Pick_New:No active IBSS"));)
        return;
    }

    pRmcUpdateInd = (tSirRmcUpdateInd *)pMsgBuf;

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Leader_Pick_New:lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac,
                           pRmcUpdateInd->mcastGroup,
                           pRmcUpdateInd->mcastTransmitter,
                           eRMC_TRANSMITTER_ROLE);
    if (!entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Leader_Pick_New: No entry"));)
        goto done;
    }

    if (entry->state != eRMC_LEADER_ACTIVE)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Pick_New:Bad state %s"),
                        __limMcastTxStateToString(entry->state) );)
        goto done;
    }

    RMC.dialogToken = 0;
    vos_mem_copy(&RMC.mcastGroup, &pRmcUpdateInd->mcastGroup,
                 sizeof(tSirMacAddr));

    /*
     * Send Leader_Inform_Cancelled Action frame to the current leader.
     */
    RMC.action = SIR_MAC_RMC_LEADER_INFORM_CANCELLED;
    status = limSendRMCActionFrame(pMac, entry->leaderList[entry->leader_index],
                         &RMC, psessionEntry);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
           FL("RMC:Leader_Pick_New: Inform_Cancel Action send failed"));)
        goto done;
    }


    /* Cache the leader list for this multicast group */
    vos_mem_copy(entry->leaderList, pRmcUpdateInd->leader,
                 sizeof(entry->leaderList));

    /* Reset leader index */
    entry->leader_index = 0;

    /*
     * Send Leader_Inform Action frame to the new candidate leader.
     */

    RMC.action = SIR_MAC_RMC_LEADER_INFORM_SELECTED;
    status = limSendRMCActionFrame(pMac, entry->leaderList[entry->leader_index],
                         &RMC, psessionEntry);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
           FL("RMC:Leader_Pick_New: Inform_Selected Action send failed"));)
        goto done;
    }


    entry->state = eRMC_LEADER_OTA_REQUEST_SENT;

    /* Add request to the pending response queue. */
    __rmcPendingRespQueueAdd(pMac, pRmcUpdateInd->mcastGroup,
                           pRmcUpdateInd->mcastTransmitter);

done:
    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Leader_Pick_New: lock release failed"));
    }
}

/**
 * __limProcessRMCLeaderInformAck()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_OTA_LEADER_INFORM_ACK
 * message from the "Leader Inform Ack" Action frame from the Leader.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCLeaderInformAck(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirMacMgmtHdr pHdr;
    tLimRmcGroupContext *entry;
    tANI_U8 *pFrameData;
    tSirMacAddr mcastGroup;
    v_PVOID_t pvosGCtx;
    VOS_STATUS vos_status;
    v_MACADDR_t vosMcast;

    if (!pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Ack:NULL msg"));)
        return;
    }

    pHdr = WDA_GET_RX_MAC_HEADER((tANI_U8 *)pMsgBuf);
    pFrameData = WDA_GET_RX_MPDU_DATA((tANI_U8 *)pMsgBuf) +
                    sizeof(tSirMacOxygenNetworkFrameHdr);

    if (!pFrameData)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Ack:NULL data"));)
        return;
    }

    /* Copy the multicast group address from the Action frame payload. */
    vos_mem_copy(&mcastGroup, pFrameData, sizeof(tSirMacAddr));

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Leader_Inform_Ack lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac, mcastGroup, pHdr->da,
                                            eRMC_TRANSMITTER_ROLE);
    if (!entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Leader_Inform_Ack: No entry"));)
        goto done;
    }

    if (entry->state != eRMC_LEADER_OTA_REQUEST_SENT)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Ack:Bad state %s"),
                        __limMcastTxStateToString(entry->state) );)
        goto done;
    }

    /* send LBP_UPDATE_IND */
    __limPostMsgUpdateInd(pMac, eRMC_LEADER_ACCEPTED, eRMC_TRANSMITTER_ROLE,
                         mcastGroup, pHdr->da, pHdr->sa);

    /* Copy it into v_MACADDR_t format for TL */
    vosMcast.bytes[0] = mcastGroup[0];
    vosMcast.bytes[1] = mcastGroup[1];
    vosMcast.bytes[2] = mcastGroup[2];
    vosMcast.bytes[3] = mcastGroup[3];
    vosMcast.bytes[4] = mcastGroup[4];
    vosMcast.bytes[5] = mcastGroup[5];

    /* Enable TL */
    pvosGCtx = vos_get_global_context(VOS_MODULE_ID_PE, (v_VOID_t *) pMac);
    vos_status = WLANTL_EnableReliableMcast(pvosGCtx, &vosMcast);

    if (VOS_STATUS_SUCCESS != vos_status)
    {
        PELOGE(limLog(pMac, LOGE,
            FL("RMC:Leader_Inform_Ack: TL enable failed"));)
    }

    /* Set leader state to Active. */
    entry->state = eRMC_LEADER_ACTIVE;

    /*
     * Since we recevied the response from the leader, remove the entry
     * from the pending queue.
     */
    __rmcPendingRespQueueRemove(pMac, mcastGroup, pHdr->da);

done:
    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC: Leader_Inform_Ack: lock release failed"));
    }

    return;
}

/**
 * __limProcessRMCLeaderInformSelected()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_OTA_LEADER_INFORM_SELECTED
 * message from the "Leader Inform" Action frame from the
 * multicast transmitter.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCLeaderInformSelected(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirMacMgmtHdr pHdr;
    tANI_U8 *pFrameData;
    tSirMacAddr mcastGroup;
    tLimRmcGroupContext *entry;

    if (!pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform:NULL msg"));)
        return;
    }

    pHdr = WDA_GET_RX_MAC_HEADER((tANI_U8 *)pMsgBuf);
    pFrameData = WDA_GET_RX_MPDU_DATA((tANI_U8 *)pMsgBuf) +
                    sizeof(tSirMacOxygenNetworkFrameHdr);

    if (!pFrameData)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform:NULL data"));)
        return;
    }
    /* Copy the multicast group address from the Action frame payload. */
    vos_mem_copy(&mcastGroup, pFrameData, sizeof(tSirMacAddr));

    /* Add the group address to the hash */
    entry = __rmcGroupInsertHashEntry(pMac, mcastGroup, pHdr->sa,
                                 eRMC_LEADER_ROLE);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform:Hash insert failed"));)
        return;
    }

    /* Send LBP_LEADER_REQ to f/w */
    __limPostMsgLeaderReq(pMac, eRMC_BECOME_LEADER_CMD, mcastGroup, pHdr->sa);

    entry->isLeader = eRMC_LEADER_PENDING;
}

/**
 * __limProcessRMCBecomeLeaderResp()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_BECOME_LEADER_RESP
 * message from the firmware.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCBecomeLeaderResp(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirRmcBecomeLeaderInd *pRmcBecomeLeaderInd;
    tLimRmcGroupContext *entry;
    tpPESession psessionEntry;
    tSirRetStatus status;
    tSirRMCInfo RMC;

    if (NULL == pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Leader_Resp:NULL message"));)
        return;
    }

    /*
     * This API relies on a single active IBSS session.
     */
    psessionEntry = limIsIBSSSessionActive(pMac);
    if (NULL == psessionEntry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Become_Leader_Resp:No active IBSS"));)
        return;
    }

    pRmcBecomeLeaderInd = (tSirRmcBecomeLeaderInd *)pMsgBuf;
    RMC.dialogToken = 0;
    RMC.action = SIR_MAC_RMC_LEADER_INFORM_ACK;
    vos_mem_copy(&RMC.mcastGroup, &pRmcBecomeLeaderInd->mcastGroup,
                 sizeof(tSirMacAddr));

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Become_Leader_Resp:lock acquire failed"));
        return;
    }

    if (pRmcBecomeLeaderInd->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC:Become_Leader_Resp:FW Status %d"),
                        pRmcBecomeLeaderInd->status);)
        status = eSIR_FAILURE;
        goto done;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac,
                  pRmcBecomeLeaderInd->mcastGroup,
                  pRmcBecomeLeaderInd->mcastTransmitter, eRMC_LEADER_ROLE);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Leader_Resp: No entry"));)
        status = eSIR_FAILURE;
        goto done;
    }

    if (entry->isLeader != eRMC_LEADER_PENDING)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Become_Leader_Resp:Bad state: %s"),
                        __limLeaderStateToString(entry->isLeader) );)
        status = eSIR_FAILURE;
        goto done;
    }


    /*
     * Send Leader_Inform_Ack Action frame to mcast transmitter.
     */
    status = limSendRMCActionFrame(pMac, pRmcBecomeLeaderInd->mcastTransmitter,
                         &RMC, psessionEntry);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
           FL("RMC:Become_Leader_Resp: Action send failed"));)
        status = eSIR_FAILURE;
        goto done;
    }


    entry->isLeader = eRMC_IS_A_LEADER;

done:
    if (eSIR_FAILURE == status)
    {
        status = __rmcGroupDeleteHashEntry(pMac,
                       pRmcBecomeLeaderInd->mcastGroup,
                       pRmcBecomeLeaderInd->mcastTransmitter,
                       eRMC_LEADER_ROLE);
        if (eSIR_FAILURE == status)
        {
            PELOGE(limLog(pMac, LOGE,
                      FL("RMC: Become_Leader_Resp:hash delete failed"));)
        }
    }

    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Become_Leader_Resp: lock release failed"));
    }

    return;
}

/**
 * __limProcessRMCLeaderInformCancelled()
 *
 *FUNCTION:
 * This function is called to processes eLIM_RMC_OTA_LEADER_INFORM_CANCELLED
 * message from the "Leader Inform Cancelled" Action frame from the
 * multicast transmitter.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the RMC message buffer
 *
 * @return None
 */
static void
__limProcessRMCLeaderInformCancelled(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirMacMgmtHdr pHdr;
    tANI_U8 *pFrameData;
    tSirMacAddr mcastGroup;
    tSirRetStatus status;
    tLimRmcGroupContext *entry;

    if (!pMsgBuf)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Cancel:NULL msg"));)
        return;
    }

    pHdr = WDA_GET_RX_MAC_HEADER((tANI_U8 *)pMsgBuf);
    pFrameData = WDA_GET_RX_MPDU_DATA((tANI_U8 *)pMsgBuf) +
                    sizeof(tSirMacOxygenNetworkFrameHdr);

    if (!pFrameData)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Cancel:NULL data"));)
        return;
    }
    /* Copy the multicast group address from the Action frame payload. */
    vos_mem_copy(&mcastGroup, pFrameData, sizeof(tSirMacAddr));

    /* Acquire RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE, FL("RMC:Leader_Inform_Cancel lock acquire failed"));
        return;
    }

    /*
     * Find the entry for this Group Address.
     */
    entry = __rmcGroupLookupHashEntry(pMac, mcastGroup,
                                     pHdr->sa, eRMC_LEADER_ROLE);
    if (NULL == entry)
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC: Leader_Inform_Cancel: No entry"));)
        goto done;
    }

    /* send LBP_UPDATE_END */
    __limPostMsgUpdateInd(pMac, eRMC_LEADER_CANCELLED, eRMC_LEADER_ROLE,
                         mcastGroup, pHdr->sa, pHdr->da);

    /*
     * Delete hash entry for this Group address.
     */
    status = __rmcGroupDeleteHashEntry(pMac, mcastGroup, pHdr->sa,
                                       eRMC_LEADER_ROLE);
    if (eSIR_FAILURE == status)
    {
        PELOGE(limLog(pMac, LOGE,
                  FL("RMC: Leader_Inform_Cancel:hash delete failed"));)
    }

done:
    /* Release RMC lock */
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_release(&pMac->rmcContext.lkRmcLock)))
    {
        limLog(pMac, LOGE,
            FL("RMC: Leader_Inform_Cancel: lock release failed"));
    }
    return;
}

/**
 * limProcessRMCMessages()
 *
 *FUNCTION:
 * This function is called to processes various RMC messages.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param  msgType   Indicates the RMC message type
 * @param  *pMsgBuf  A pointer to the RMC message buffer
 *
 * @return None
 */
void
limProcessRMCMessages(tpAniSirGlobal pMac, eRmcMessageType msgType,
                      tANI_U32 *pMsgBuf)
{

   if (pMsgBuf == NULL)
    {
           PELOGE(limLog(pMac, LOGE, FL("Buffer is Pointing to NULL"));)
           return;
    }

    PELOGE(limLog(pMac, LOGE, FL("RMC: limProcessRMCMessages: %s"),
                        __limLeaderMessageToString(msgType) );)

    switch (msgType)
    {
        /*
         * Begin - messages processed by RMC multicast transmitter.
         */
        case eLIM_RMC_ENABLE_REQ:
            __limProcessRMCEnableRequest(pMac, pMsgBuf);
            break;

        case eLIM_RMC_DISABLE_REQ:
            __limProcessRMCDisableRequest(pMac, pMsgBuf);
            break;

        case eLIM_RMC_LEADER_SELECT_RESP:
            __limProcessRMCLeaderSelectResponse(pMac, pMsgBuf);
            break;

        case eLIM_RMC_LEADER_PICK_NEW:
            __limProcessRMCLeaderPickNew(pMac, pMsgBuf);
            break;

        case eLIM_RMC_OTA_LEADER_INFORM_ACK:
            __limProcessRMCLeaderInformAck(pMac, pMsgBuf);
            break;
        /*
         * End - messages processed by RMC multicast transmitter.
         */

        /*
         * Begin - messages processed by RMC Leader (receiver).
         */
        case eLIM_RMC_OTA_LEADER_INFORM_SELECTED:
            __limProcessRMCLeaderInformSelected(pMac, pMsgBuf);
            break;

        case eLIM_RMC_BECOME_LEADER_RESP:
            __limProcessRMCBecomeLeaderResp(pMac, pMsgBuf);
            break;

        case eLIM_RMC_OTA_LEADER_INFORM_CANCELLED:
            __limProcessRMCLeaderInformCancelled(pMac, pMsgBuf);
            break;

        /*
         * End - messages processed by RMC Leader (receiver).
         */

        default:
            break;
    } // switch (msgType)
    return;
} /*** end limProcessRMCMessages() ***/

void
limRmcInit(tpAniSirGlobal pMac)
{
    vos_mem_zero(&pMac->rmcContext, sizeof(pMac->rmcContext));

    if (!VOS_IS_STATUS_SUCCESS(vos_lock_init(&pMac->rmcContext.lkRmcLock)))
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC lock init failed!"));)
    }

    if (tx_timer_create(&pMac->rmcContext.gRmcResponseTimer,
                            "RMC RSP TIMEOUT",
                            __rmcResponseTimerHandler,
                            0 /* param */,
                            50 /* ms */, 0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        /*  Could not create RMC response timer. */
        limLog(pMac, LOGE, FL("could not create RMC response timer"));
    }
}

void
limRmcCleanup(tpAniSirGlobal pMac)
{
    if (!VOS_IS_STATUS_SUCCESS(vos_lock_destroy(&pMac->rmcContext.lkRmcLock)))
    {
        PELOGE(limLog(pMac, LOGE, FL("RMC lock destroy failed!"));)
    }

    tx_timer_delete(&pMac->rmcContext.gRmcResponseTimer);
}

#endif /* WLAN_FEATURE_RELIABLE_MCAST */
