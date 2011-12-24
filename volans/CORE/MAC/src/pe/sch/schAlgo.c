/**
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * This file schAlgo.cc contains the functions used to implement the
 * scheduling algorithm.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#if (WNI_POLARIS_FW_PRODUCT == AP)
#ifdef WMM_SA

#ifndef NS_SIM
#include "aniGlobal.h"
#include "limApi.h"

#endif

#include "schClass.h"
#include "schDebug.h"

#ifdef NS_SIM
#include "mac.h"
#include "mac-802_11.h"
#endif

// --------------------------------------------------------------------
/**
 * initTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline void
initTIB(tpAniSirGlobal pMac)
{
    pMac->sch.TIB.next = 0;
    pMac->sch.TIB.totTxop = 0;
}

// --------------------------------------------------------------------
/**
 * lookupTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline tANI_U8
lookupTIB(tpAniSirGlobal pMac, tANI_U16 sta, tANI_U8 pri, tANI_U8 dir)
{
    for (tANI_U8 i=0; i<pMac->sch.TIB.next; i++)
        if (pMac->sch.TIB.entry[i].sta == sta && pMac->sch.TIB.entry[i].pri == pri
            && pMac->sch.TIB.entry[i].dir == dir)
            return i;
    return MAX_INST_PER_SCHEDULE;
}

// --------------------------------------------------------------------
/**
 * insertTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline tANI_U16
insertTIB(tpAniSirGlobal pMac, tANI_U8 sta, tANI_U8 pri, tANI_U8 dir, tANI_U16 txop, tANI_U16 *pTxop)
{
    tANI_U16 remTxop = 0;

    pMac->sch.TIB.entry[pMac->sch.TIB.next].sta = sta;
    pMac->sch.TIB.entry[pMac->sch.TIB.next].pri = pri;
    pMac->sch.TIB.entry[pMac->sch.TIB.next].dir = dir;
    pMac->sch.TIB.entry[pMac->sch.TIB.next].txop = txop;

    if (pMac->sch.TIB.entry[pMac->sch.TIB.next].txop >
        MAX_TXOP_PER_INSTRUCTION)
    {
        remTxop = pMac->sch.TIB.entry[pMac->sch.TIB.next].txop -
                  MAX_TXOP_PER_INSTRUCTION;
        pMac->sch.TIB.entry[pMac->sch.TIB.next].txop =
        MAX_TXOP_PER_INSTRUCTION;
    }

    *pTxop = pMac->sch.TIB.entry[pMac->sch.TIB.next].txop;

    pMac->sch.TIB.totTxop += txop - remTxop;
    pMac->sch.TIB.next++;
    return remTxop;
}

// --------------------------------------------------------------------
/**
 * addTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline tANI_U16
addTIB(tpAniSirGlobal pMac, tANI_U8 index, tANI_U16 txop, tANI_U16 *pTxop)
{
    tANI_U16 remTxop = 0;

    pMac->sch.TIB.entry[index].txop += txop;
    if (pMac->sch.TIB.entry[index].txop > MAX_TXOP_PER_INSTRUCTION)
    {
        remTxop = pMac->sch.TIB.entry[index].txop - MAX_TXOP_PER_INSTRUCTION;
        pMac->sch.TIB.entry[index].txop = MAX_TXOP_PER_INSTRUCTION;
    }
    *pTxop = pMac->sch.TIB.entry[index].txop;
    pMac->sch.TIB.totTxop += txop - remTxop;
    return remTxop;
}

// --------------------------------------------------------------------
/**
 * fullTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline bool
fullTIB(tpAniSirGlobal pMac)
{
    return(pMac->sch.TIB.next == MAX_INST_PER_SCHEDULE ||
           pMac->sch.TIB.totTxop >= MAX_TXOP_PER_SCHEDULE);
}

// --------------------------------------------------------------------
/**
 * emptyTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline bool
emptyTIB(tpAniSirGlobal pMac)
{
    return(pMac->sch.TIB.next == 0);
}

// --------------------------------------------------------------------
/**
 * subMinTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline bool
subMinTIB(tpAniSirGlobal pMac)
{
    if (pMac->sch.TIB.next == 0)
        return true;
    else
        return(pMac->sch.TIB.totTxop/pMac->sch.TIB.next <
               MIN_TXOP_PER_INSTRUCTION &&
               pMac->sch.TIB.totTxop < MAX_TXOP_PER_SCHEDULE);
}

// --------------------------------------------------------------------
/**
 * schPrintTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

void
schPrintTIB(tpAniSirGlobal pMac)
{
    schLog(pMac, LOG2, "\n********* pMac->sch.TIB tot %d txop %d\n",
           pMac->sch.TIB.next, pMac->sch.TIB.totTxop);

    for (int i=0; i<pMac->sch.TIB.next; i++)
        schLog(pMac, LOG2, "\t[%d] %d (%d,%d)\n", i, pMac->sch.TIB.entry[i].dir,
               pMac->sch.TIB.entry[i].sta, pMac->sch.TIB.entry[i].pri);
}

// --------------------------------------------------------------------
/**
 * convertTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params index Index of the scheduling buffer to write to
 * @return None
 */

void
convertTIB(tANI_U8 index)
{
    tpSchSchedule s = &schedule[index];

#if 1
    // TFP Workaround
    s->totTxopLo = 0xff;
    s->totTxopHi = 0xff;
#else
    s->totTxopLo = (pMac->sch.TIB.totTxop * TXOP_UNIT_IN_USEC) & 0xff;
    s->totTxopHi = (pMac->sch.TIB.totTxop * TXOP_UNIT_IN_USEC) >> 8;
#endif
    s->totInst = pMac->sch.TIB.next;
    s->curInst = 0;

    for (int i=0; i<pMac->sch.TIB.next; i++)
    {
        if (pMac->sch.TIB.entry[i].dir == SCH_DL_QUEUE)
        {
            pMac->sch.numData++;
            createDataInstruction(&(s->inst[i]), pMac->sch.TIB.entry[i].sta,
                                  pMac->sch.TIB.entry[i].pri,
                                  pMac->sch.TIB.entry[i].txop);
        }
        else
        {
            pMac->sch.numPoll++;
            createPollInstruction(&(s->inst[i]), pMac->sch.TIB.entry[i].sta,
                                  pMac->sch.TIB.entry[i].pri,
                                  pMac->sch.TIB.entry[i].txop);
            schLog(pMac, LOG2, FL("POLL sta %d tc %d\n"),
                   pMac->sch.TIB.entry[i].sta,
                   pMac->sch.TIB.entry[i].pri);
        }

        tANI_U32 arg = pMac->sch.TIB.entry[i].txop;
        arg <<= 16;
        arg |= (pMac->sch.TIB.entry[i].sta << 4);
        arg |= (pMac->sch.TIB.entry[i].dir << 3);
        arg |= (pMac->sch.TIB.entry[i].pri);
        SCH_TRACE(SCH_TRACE_INST, arg);
    }
}

// --------------------------------------------------------------------
/**
 * addQuantumQueue
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

void
//addQuantumQueue(tpSchQueueState q, tANI_U32 /*quantum*/)
addQuantumQueue(tpSchQueueState q)
{
    if (pMac->sch.defaultTxop)
        q->credits = (tANI_U16) pMac->sch.defaultTxop;
    else
    {
        q->credits += q->quantum;
        if (q->credits > MAX_ACCUMULATED_QUANTUM)
            q->credits = MAX_ACCUMULATED_QUANTUM;
    }
}

// --------------------------------------------------------------------
/**
 * SCH_CF_POLL_TIME
 *
 * FUNCTION:
 * Time taken to send CF-Poll to staId (units of TXOP)
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline tANI_U16
SCH_CF_POLL_TIME(tANI_U16 staId)
{
#if (WNI_POLARIS_FW_OS == SIR_RTAI)
  tpAniSirGlobal pMac = getPMac();
#endif
    tpDphHashNode staPtr = dphGetHashEntry(pMac, staId);

    if (staPtr == NULL || staPtr->valid == 0)
    {
        schLog(pMac, LOGE, FL("SCH_CF_POLL_TIME: Invalid staId %d staPtr 0x%x\n"), staId, (tANI_U32) staPtr);
        return 2;
    }
    else
    {
        tANI_U32 dataRateX2 = 72;
        tANI_U32 preamble = pMac->dph.gDphIFSValues[pMac->lim.].preamble;

        return((tANI_U16) ((preamble + (30 * 8 * 2 / dataRateX2) + 31)/32));
    }
}

// --------------------------------------------------------------------
/**
 * grantUnsolicited
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

static inline void
grantUnsolicited(tpAniSirGlobal pMac)
{
    tpSchQueueState q;

    while ((q= pMac->sch.unsolicited.getNext()) != NULL)
    {
        // give the grant
        tANI_U16 i = q->staId;
        tANI_U8  j = q->pri;
        tANI_U8  k = q->dir;
        tANI_U8 indexTIB = lookupTIB(pMac, i, j, k);
        tANI_U16 instTxop = 0;

        if (indexTIB == MAX_INST_PER_SCHEDULE)
            insertTIB(pMac, (tANI_U8) i, j, k, (tANI_U16) pMac->sch.defaultTxop, &instTxop);

#ifdef SCH_DEBUG_STATS
        // 0-11: staid, 12-14: pri, 15: direction, 16-31: txop
        pMac->sch.schObject.schTrace(SCH_TRACE_UNSOLICITED_GRANT,
                  (i & 0x03FF) | ((j & 7) << 12) | ((k & 1) << 15) | (instTxop << 16));
#endif

        // remove the queue from service list
        pMac->sch.unsolicited.schMakeInactive(q);

        // if schedule is full, exit
        if (fullTIB(pMac)) break;
    }
}

// --------------------------------------------------------------------
/**
 * computeTIB
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params serviceServiced Whether to service the serviced queues or not
 * @params pExistServiced pointer to the boolean indicating whether
 *         serviced queues exist or not
 * @params classId The class to be serviced
 * @params quantum Maximum quanta to be allocated before returning from
 *         this function
 * @return Unused quantum
 */
#define SCH_NEW_ALGO
#ifdef SCH_NEW_ALGO
tANI_U32
computeTIB(bool serviceServiced, bool *pExistServiced,
                     tANI_U8 classId, tANI_U32 quantum)
{
    tpSchQueueState q = qosClass[classId].getNext();
    if (q == NULL) return (quantum);
    tpSchQueueState qFirst = q;
    do
    {
        tANI_U16 i = q->staId;
        tANI_U8  j = q->pri;
        tANI_U8  k = q->dir;
        if (k == SCH_UL_QUEUE)
            j |= (q->ts << 3);

        if (q->serviced)
            *pExistServiced = true;

        if (q->serviced == 0 || serviceServiced)
        {
            tANI_U8 indexTIB = lookupTIB(pMac, i, j, k);
            tANI_U16 instTxop = 0;

            if (indexTIB == MAX_INST_PER_SCHEDULE)
                insertTIB(pMac, (tANI_U8) i, j, k, (tANI_U16) pMac->sch.defaultTxop, &instTxop);

            q->serviced++;
        }
        else if (q->serviced)
        {
            SCH_TRACE(SCH_TRACE_SKIP_SERVICED, (i << 4) | (k << 3) | j);
        }

        q = qosClass[classId].moveNext();
    }
    while (!fullTIB(pMac) && q != qFirst);

    return quantum;
}
#else
tANI_U32
computeTIB(bool serviceServiced, bool *pExistServiced,
                     tANI_U8 classId, tANI_U32 quantum)
{
    tpSchQueueState q = qosClass[classId].getNext();
    if (q == NULL)
        return(quantum+1);

    again:

    bool existUnserviced = false;
    bool ulFlowActive = false;
    tpSchQueueState qFirst = q;
    do
    {
        //      assert(q->active);

        tANI_U16 i = q->staId;
        tANI_U8  j = q->pri;
        tANI_U8  k = q->dir;

        if (q->serviced == 1)
            *pExistServiced = true;

        addQuantumQueue(q, SCH_QUANTUM_QUEUE);
        //      quantum -= SCH_QUANTUM_QUEUE;

        if (q->serviced == 0 || serviceServiced)
        {
            if (k == SCH_DL_QUEUE ||
                q->credits >= (q->txop + SCH_CF_POLL_TIME(i)) ||
                q->credits >= SCH_MIN_UL_ALLOC)
            {
                tANI_U8 indexTIB = lookupTIB((tANI_U8)i, j, k);
                tANI_U16 instTxop = 0;

                if (indexTIB < MAX_INST_PER_SCHEDULE)
                    q->credits = addTIB(indexTIB, q->credits, &instTxop);
                else
                    q->credits = insertTIB((tANI_U8)i, j, k, q->credits, &instTxop);

                if (k == SCH_UL_QUEUE)
                    schLog(pMac, LOG2, "queue[%d,%d,%d] txop %d credit %d\n",
                           i, j, k, q->txop, q->credits);

                if (q->credits > 0 || q->txop <= instTxop)
                    q->serviced = 1;
            }
            else
                ulFlowActive = true;
        }
        else if (q->serviced)
        {
            SCH_TRACE(SCH_TRACE_SKIP_SERVICED, (i << 4) | (k << 3) | j);
        }

        if (q->serviced == 0)
            existUnserviced = true;

        q = qosClass[classId].moveNext();
    }
    while (!fullTIB() && q != qFirst && quantum >= SCH_QUANTUM_QUEUE);

    if (!fullTIB() && quantum >= SCH_QUANTUM_QUEUE &&
        ((!emptyTIB() && subMinTIB() &&
          (existUnserviced || serviceServiced)) ||
         (emptyTIB() && ulFlowActive)))
        goto again;

    return quantum;
}
#endif

// --------------------------------------------------------------------
/**
 * computeSchedule
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params index Index of the scheduling buffer to write to
 * @return None
 */

void
computeSchedule(tANI_U8 index)
{
    // dumpQueueSizes();

    // Initialize TIB
    initTIB(pMac);

    // Service unsolicited grants
    grantUnsolicited(pMac);

    // Service queues in the order of strict priority
    for (tANI_U8 classId = 0; classId < SCH_NUM_QOS_CLASSES; classId++)
    {
        if (!emptyTIB(pMac)) break;

        /*
         * Compute a schedule,
         * but don't service the flows that were serviced in
         * the most recent schedule
         */
        bool existServiced = false;
        bool serviceServiced = false;
        computeTIB(serviceServiced, &existServiced, classId,
                   MAX_TXOP_PER_SCHEDULE);

        if (emptyTIB(pMac) && existServiced)
        {
            /*
             * All non-empty flows in this class are serviced
             * Therefore assign txop to serviced flows
             */

            SCH_TRACE(SCH_TRACE_TIB_PASS2, existServiced ? 1 : 0);

            /*
             * Compute a schedule again,
             * this time service the flows that were serviced in the most
             * recent schedule
             */
            serviceServiced = true;
            computeTIB(serviceServiced, &existServiced, classId,
                       MAX_TXOP_PER_SCHEDULE);
        }
    }

    // Convert the TIB into formatted schedule
    convertTIB(index);
}

// --------------------------------------------------------------------
/**
 * createDataInstruction
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

void
createDataInstruction(tpSchInstruction t, tANI_U16 staid,
                                tANI_U16 pri, tANI_U16 txop)
{
    t->type         = SCH_INST_DATA;
    t->instExecuted = 0;
    t->feedback     = 0;
    t->txopLo       = txop & 0xff;
    t->txopHi       = txop  >> 8;
    t->staidLo      = staid & 0xf;
    t->staidHi      = staid >> 4;
    t->pri          = pri;
}

// --------------------------------------------------------------------
/**
 * createPollInstruction
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @params None
 * @return None
 */

void
createPollInstruction(tpSchInstruction t, tANI_U16 staid, tANI_U16
                                pri, tANI_U16 txop)
{
    t->type         = SCH_INST_POLL;
    t->instExecuted = 0;
    t->feedback     = 0;
    t->txopLo       = txop & 0xff;
    t->txopHi       = txop  >> 8;
    t->staidLo      = staid & 0xf;
    t->staidHi      = staid >> 4;
    t->pri          = pri;
#ifdef NS_SIM
    t->staAddr[0]   = (tANI_U8)staid;
#else
    tANI_U8 *staAddr = dphGetMacAddress(pMac, staid);
    if (!staAddr)
    {
        schLog(pMac, LOGE, FL("MAC addr not found for POLL inst STA %d TC %d\n"),
               staid, pri);
        return;
    }
    sirCopyMacAddr(t->staAddr, staAddr);
#endif
}
#endif /* WMM_SA */

#endif
// --------------------------------------------------------------------
