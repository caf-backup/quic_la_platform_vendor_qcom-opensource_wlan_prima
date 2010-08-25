/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limAIDmgmt.cc contains the functions related to
 * AID pool management like initialization, assignment etc.
 * Author:        Chandra Modumudi
 * Date:          03/20/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
 
#include "palTypes.h"
#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "aniGlobal.h"
#include "cfgApi.h"
#include "sirParams.h"
#include "limUtils.h"
#include "limTimerUtils.h"
#include "limSession.h"
#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#endif

#define LIM_START_AID_AP    2
#define LIM_START_AID_STA   1

/* the algorithm makes use of pMac->lim.gLimAIDpool array and of
   6 static(for now) variables are used, everything can be tANI_U8 if MAX_NUMBER_OF_AID is 255 */

//unsigned int freeAidHead;
//unsigned int freeAidTail;

/* the to be released list is used in case delayedRelease is set.
   This list holds the aid that must be released upon release timer exipiry.
   Upon timer expiry the first "numReleaseLastCycle" Aids, i.e. the Aids at the head
   of the list, are released. Those are the Aid that have been released during the previous
   release timer cycle.

   Example of a to elements list:
   toBeReleasedHead => aid=12 released last cycle => aid=7 released this cycle => 0
   toBeReleasedTail ===============================^
*/


//numReleasedThisCycle /* number of Aid released since last timer expiry */
//numReleasedLastCycle /* number of Aid released before last timer expiry */

//delayedRelease /* if set then the aid are actually released after at least
/* one period of the release timer, otherwise they are released */
/* upon call to limReleaseAid() */



/**
 * limInitAIDpool()
 *
 *FUNCTION:
 * This function is called while starting a BSS at AP
 * to initialize AID pool. This may also be called while
 * starting/joining an IBSS if 'Association' is allowed
 * in IBSS.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limInitAIDpool(tpAniSirGlobal pMac,tpPESession sessionEntry)
{
    tANI_U8 i;
    tANI_U8 maxAssocSta = pMac->lim.maxStation;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (limGetSystemRole(sessionEntry) == eLIM_AP_ROLE)
    {
        pMac->lim.gpLimAIDpool[0]=pMac->lim.gpLimAIDpool[1]=0;
        pMac->lim.delayedRelease=1;
        pMac->lim.freeAidHead=LIM_START_AID_AP;
    }
#endif
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
#ifndef WLAN_SOFTAP_FEATURE
    if (limGetSystemRole(sessionEntry) != eLIM_AP_ROLE)
#endif		
    {
        pMac->lim.gpLimAIDpool[0]=0;
        pMac->lim.freeAidHead=LIM_START_AID_STA;
    }
#endif

#ifdef WLAN_SOFTAP_FEATURE
    maxAssocSta = MAX_NO_OF_ASSOC_STA;   
    for (i=pMac->lim.freeAidHead;i<maxAssocSta-1; i++)
#else    
    for (i=pMac->lim.freeAidHead;i<maxAssocSta-1; i++)
#endif
    {
        pMac->lim.gpLimAIDpool[i]         = i+1;
    }
    pMac->lim.gpLimAIDpool[i]         =  0;

    pMac->lim.freeAidTail=i;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (limGetSystemRole(sessionEntry) == eLIM_AP_ROLE)
    {
        pMac->lim.toBeReleasedHead=0;
        pMac->lim.toBeReleasedTail=0;
        pMac->lim.numReleasedThisCycle=pMac->lim.numReleasedLastCycle=0;

        if (pMac->lim.delayedRelease)
        {
            tANI_U32 cfgValue;

            if (wlan_cfgGetInt(pMac, WNI_CFG_RELEASE_AID_TIMEOUT,
                        &cfgValue) != eSIR_SUCCESS)
            {
                /**
                 * Could not get AID release Timeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                        FL("could not retrieve AID release Timeout value\n"));
            }
            cfgValue = SYS_MS_TO_TICKS(cfgValue);

            /// Create and start AID release timeout
            if (tx_timer_create(&pMac->lim.limTimers.gLimAIDreleaseTimer, "AIDmgmt",
                        limTimerHandler, SIR_LIM_AID_RELEASE_TIMEOUT,
                        cfgValue, cfgValue, 0) != TX_SUCCESS)
            {
                /// Could not create ReleaseAID timer.
                // Log error
                limLog(pMac, LOGP, FL("could not create ReleaseAID timer\n"));

                return;
            }
        }
    }
#endif
}


/**
 * limAssignAID()
 *
 *FUNCTION:
 * This function is called during Association/Reassociation
 * frame handling to assign association ID (aid) to a STA.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return aid  - assigned Association ID for STA
 */

tANI_U16
limAssignAID(tpAniSirGlobal pMac)
{
    tANI_U16 aid;

    /* return head of free list */

    if (pMac->lim.freeAidHead)
    {
        aid=pMac->lim.freeAidHead;
        pMac->lim.freeAidHead=pMac->lim.gpLimAIDpool[pMac->lim.freeAidHead];
        if (pMac->lim.freeAidHead==0)
            pMac->lim.freeAidTail=0;
        pMac->lim.gLimNumOfCurrentSTAs++;
        //PELOG2(limLog(pMac, LOG2,FL("Assign aid %d, numSta %d, head %d tail %d \n"),aid,pMac->lim.gLimNumOfCurrentSTAs,pMac->lim.freeAidHead,pMac->lim.freeAidTail);)
        return aid;
    }

    return 0; /* no more free aids */
}


/**
 * limReleaseAID()
 *
 *FUNCTION:
 * This function is called when a STA context is removed
 * at AP (or at a STA in IBSS mode) to return association ID (aid)
 * to free pool.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  aid - Association ID that need to return to free pool
 *
 * @return None
 */

void
limReleaseAID(tpAniSirGlobal pMac, tANI_U16 aid)
{
    pMac->lim.gLimNumOfCurrentSTAs--;

    /* insert at tail of free list */
    if (pMac->lim.freeAidTail)
    {
        pMac->lim.gpLimAIDpool[pMac->lim.freeAidTail]=(tANI_U8)aid;
        pMac->lim.freeAidTail=(tANI_U8)aid;
    }
    else
    {
        pMac->lim.freeAidTail=pMac->lim.freeAidHead=(tANI_U8)aid;
    }
    pMac->lim.gpLimAIDpool[(tANI_U8)aid]=0;
    //PELOG2(limLog(pMac, LOG2,FL("Release aid %d, numSta %d, head %d tail %d \n"),aid,pMac->lim.gLimNumOfCurrentSTAs,pMac->lim.freeAidHead,pMac->lim.freeAidTail);)

}


/**
 * limReleaseAIDtoTBRList()
 *
 *FUNCTION:
 * This function is called when a STA context is removed
 * at AP (or at a STA in IBSS mode) to return association ID (aid)
 * to free pool, the AID will be released only after AID_TIMER expiry
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  aid - Association ID that need to return to free pool
 *
 * @return None
 */
void limAddAIDtoTBRList(tpAniSirGlobal pMac, tANI_U16 aid) {
#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (pMac->lim.delayedRelease)
    {
        /* insert at tail of to be released list */
        if (pMac->lim.toBeReleasedTail)
        {
            pMac->lim.gpLimAIDpool[pMac->lim.toBeReleasedTail]=(tANI_U8)aid;
            pMac->lim.toBeReleasedTail=(tANI_U8)aid;
        }
        else
        {
            pMac->lim.toBeReleasedTail=pMac->lim.toBeReleasedHead=(tANI_U8)aid;

#ifdef GEN6_TODO
            /* revisit this piece to fetch the appropriate sessionId below
             * priority - MEDIUM
             */
            pMac->lim.limTimers.gLimAIDreleaseTimer.sessionId = sessionId;
#endif
            /* the list was empty: activate release timer */
            MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_RELEASE_AID_TIMER));
            tx_timer_activate(&pMac->lim.limTimers.gLimAIDreleaseTimer);
        }
        pMac->lim.gpLimAIDpool[(tANI_U8)aid]=0;
        pMac->lim.numReleasedThisCycle++;
        //PELOG2(limLog(pMac, LOG2,FL("Release aid to TBR aid%d num %d\n"),aid,pMac->lim.numReleasedThisCycle);)
    }
    else 
#endif
       limReleaseAID(pMac,aid);
}

#if (WNI_POLARIS_FW_PRODUCT == AP)
/**
 * limReleaseAIDHandler()
 *
 *FUNCTION:
 * This function is called upon periodic AIDrelease timeout
 * to return association ID (aid) to free pool from
 * to-be-released (tbr!) list.
 *
 *LOGIC:
 * numReleasedLastCycle is the number of Aid released in previous timer
 * cycle, so this is the number of Aid that we have to release now.
 * Those Aid are always at the begining of the to be released list.
 *
 *ASSUMPTIONS:
 * timer handler cannot be preempted by something calling limReleaseAid
 * or limAssignAid
 *
 *NOTE:
 * AID is placed in TBR (to-be-released) list upon Disassociation
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limReleaseAIDHandler(tpAniSirGlobal pMac)
{
    tANI_U8 i, aid;
    
#ifdef GEN6_TODO
    // use the following sessionEntry, if at all the AIDpool is maintained per session
    // priority - MEDIUM
    tpPESession sessionEntry;

    if(sessionEntry = (peFindSessionBySessionId(pMac, pMac->lim.limTimers.gLimAIDreleaseTimer.sessionId))== NULL) 
    {
        limLog(pMac, LOGP,FL("Session Does not exist for given sessionID\n"));
        return;
    }
#endif

    /* free the Aid released during previous cycle */
    for (i=0;i<pMac->lim.numReleasedLastCycle;i++)
    {
        /* get aid at head of to be released list, the list cannot be empty */
        aid=pMac->lim.toBeReleasedHead;
        pMac->lim.toBeReleasedHead=pMac->lim.gpLimAIDpool[pMac->lim.toBeReleasedHead];
        /* release it at tail of free list */
        if (pMac->lim.freeAidTail)
        {
            pMac->lim.gpLimAIDpool[pMac->lim.freeAidTail]=aid;
            pMac->lim.freeAidTail=aid;
        }
        else
        {
            pMac->lim.freeAidTail=pMac->lim.freeAidHead=aid;
        }
    }
    pMac->lim.gpLimAIDpool[pMac->lim.freeAidTail]=0;
    pMac->lim.gLimNumOfCurrentSTAs-=pMac->lim.numReleasedLastCycle;
    //PELOG2(limLog(pMac, LOG2,FL("Release aid timer: num %d, numSta %d, head %d tail %d \n"),pMac->lim.numReleasedLastCycle,pMac->lim.gLimNumOfCurrentSTAs,pMac->lim.freeAidHead,pMac->lim.freeAidTail);)
    pMac->lim.numReleasedLastCycle=pMac->lim.numReleasedThisCycle;

    if (pMac->lim.numReleasedThisCycle==0)
    {
        /* no more aid to release, the timer is periodic: deactivate it */
	MTRACE(macTrace(pMac, TRACE_CODE_TIMER_DEACTIVATE, 0, eLIM_RELEASE_AID_TIMER));
        tx_timer_deactivate(&pMac->lim.limTimers.gLimAIDreleaseTimer);
        pMac->lim.toBeReleasedTail=0;
        /* sanity check: if there is no more Aid to release, then the list must be empty */
        if (pMac->lim.toBeReleasedHead) limLog(pMac, LOGP,FL("AID management, toBeReleasedHead non null"));
    }
    else  /* start new cycle */ pMac->lim.numReleasedThisCycle=0;


} /****** end limReleaseAIDHandler() ******/

#endif

