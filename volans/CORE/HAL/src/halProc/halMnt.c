/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMnt.c:    Maintenance message processing file.
 * Author:  V. K. Kandarpa
 * Date:    01/29/2002
 * History:-
 * Date     Modified by         Modification Information
 * 2/27/06  Satish G            Moved to Taurus branch as stubs.
 * --------------------------------------------------------------------------
 *
 */

/* Standard include files */

/* Application Specific include files */
#include "palTypes.h"
#include "sirCommon.h"
#include "halDataStruct.h"
#include "aniGlobal.h"
#include "halCommonApi.h"  //halMntPostMsgApi()
#include "halPhyApi.h"
#include "cfgApi.h"
#include "halDebug.h"

#define HAL_STATS_UPDATE16BIT_ROLLOVER(a, b) do { \
    if (b < (a & 0xFFFF)) { \
       a += 0x10000; \
    } \
    a = (a & 0xFFFF0000) | b; \
} \
while (0)

#define HAL_UPDATE_STAT64(a, b, c, d) do { \
    if (((*a)+(*b)) < (*a)) (*c)++; \
    (*c) += (*d); \
    (*a) += (*b); \
} \
while(0)

/// Get Radio stat request handler
void halMntProcessGetRadioStatsReq(tpAniSirGlobal, tpSirMsgQ);

/// Get Per STA stats request handler
void halMntProcessGetPerStaStatsReq(tpAniSirGlobal, tpSirMsgQ);

/// Get Aggregate sta stats request handler
void halMntProcessGetAggregateStaStatsReq(tpAniSirGlobal, tpSirMsgQ);

/// Handler clear stats request
void halMntProcessGetClearStatsReq(tpAniSirGlobal, tpSirMsgQ);

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
#define PERIODIC_CAL_ITER_LIMIT 10  //for 30second periodic interrupt, do this every 5 minutes

tSirRetStatus halMntProcessMsgs(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    tSirRetStatus rc = eSIR_SUCCESS;

    HALLOG4( halLog(pMac, LOG4, FL("msgType (%X)\n"), pMsg->type));

    cfgProcessMbMsg(pMac, (tSirMbMsg*)pMsg->bodyptr);

    return rc;

} // halMntProcessMsgs()

// ------------------------------------------------------------------
/**
 * halMntProcessMsgs
 *
 * FUNCTION:
 *   Gets radio stats and sends response to the requestor.
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
 * @return None
 */
void halMntProcessGetRadioStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    return;
}

// -----------------------------------------------------------------
/**
 * halMntProcessGetPerStaStatsReq
 *
 * FUNCTION:
 *   Processes Per STA request and send response with stats
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
 * @return None
 */
void halMntProcessGetPerStaStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    return;
}

// -----------------------------------------------------------------
/**
 * halMntProcessGetAggrStaStatsReq
 *
 * FUNCTION:
 *   Processes aggregate sta stats request and sends response with stats
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
 * @return None
 */
void halMntProcessGetAggregateStaStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    return;
}

// ----------------------------------------------------------------
/**
 * halMntProcessGetClearStatsReq
 *
 * FUNCTION:
 *  Processes clear stats request message
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
 * @return None
 */
void halMntProcessGetClearStatsReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    return;
}

// -----------------------------------------------------------------
/**
 * halCheckAndCorrectLossOfPduCondition
 *
 * FUNCTION:
 *   Check and correct for loss of PDU condition
 *
 * LOGIC
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param tpAniSirGlobal MAC parameters structure
 * @return None
 */
void halCheckAndCorrectLossOfPduCondition(tpAniSirGlobal pMac)
{
    return;
}

// -------------------------------------------------------------
/**
 * halMntTempCheck
 *
 * FUNCTION:
 *  Measures RF Chain#0 temperature and takes appropriate action.
 *
 * LOGIC
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *    If the temperature checks exceed the required CAL checks, then it
 *    will generate SIR_HAL_CAL_REQ_NTF to HAL.
 *
 * @param tpAniSirGlobal MAC parameter structure pointer
 * @return NONE
 */

void halMntTempCheck(tpAniSirGlobal pMac)
{
    return;
}
