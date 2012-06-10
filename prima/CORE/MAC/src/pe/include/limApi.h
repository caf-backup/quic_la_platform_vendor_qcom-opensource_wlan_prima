/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limApi.h contains the definitions exported by
 * LIM module.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_API_H
#define __LIM_API_H

#include "wniApi.h"
#include "sirApi.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h"
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
#include "sirMacPropExts.h"
#endif
#include "sirCommon.h"
#include "sirDebug.h"
#include "schGlobal.h"
#include "utilsApi.h"
#include "limGlobal.h"
#include "halMsgApi.h"
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "wlan_qct_wdi_ds.h"
#endif
#include "wlan_qct_wda.h"

#define LIM_POL_SYS_SCAN_MODE      0
#define LIM_POL_SYS_LEARN_MODE     1


/* Macro to count heartbeat */

#define limResetHBPktCount(psessionEntry)   (psessionEntry->LimRxedBeaconCntDuringHB = 0)



/* Useful macros for fetching various states in pMac->lim */
/* gLimSystemRole */
#define GET_LIM_SYSTEM_ROLE(psessionEntry)      (psessionEntry->limSystemRole)
#define LIM_IS_AP_ROLE(psessionEntry)           (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_AP_ROLE)
#define LIM_IS_STA_ROLE(psessionEntry)          (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_STA_ROLE)
#define LIM_IS_IBSS_ROLE(psessionEntry)         (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_STA_IN_IBSS_ROLE)
/* gLimSmeState */
#define GET_LIM_SME_STATE(pMac)                 (pMac->lim.gLimSmeState)
#define SET_LIM_SME_STATE(pMac, state)          (pMac->lim.gLimSmeState = state)
/* gLimMlmState */
#define GET_LIM_MLM_STATE(pMac)                 (pMac->lim.gLimMlmState)
#define SET_LIM_MLM_STATE(pMac, state)          (pMac->lim.gLimMlmState = state)
/*tpdphHashNode mlmStaContext*/
#define GET_LIM_STA_CONTEXT_MLM_STATE(pStaDs)   (pStaDs->mlmStaContext.mlmState)
#define SET_LIM_STA_CONTEXT_MLM_STATE(pStaDs, state)  (pStaDs->mlmStaContext.mlmState = state)
/* gLimQuietState */
#define GET_LIM_QUIET_STATE(pMac)               (pMac->lim.gLimSpecMgmt.quietState)
#define SET_LIM_QUIET_STATE(pMac, state)        (pMac->lim.gLimSpecMgmt.quietState = state)

#define LIM_IS_CONNECTION_ACTIVE(psessionEntry)  (psessionEntry->LimRxedBeaconCntDuringHB)

/*pMac->lim.gLimProcessDefdMsgs*/
#define GET_LIM_PROCESS_DEFD_MESGS(pMac) (pMac->lim.gLimProcessDefdMsgs)
#define SET_LIM_PROCESS_DEFD_MESGS(pMac, val) (pMac->lim.gLimProcessDefdMsgs = val)
// LIM exported function templates
//inline tANI_U16
//limGetNumAniPeersInBss(tpAniSirGlobal pMac)
//{ return pMac->lim.gLimNumOfAniSTAs; }
#define LIM_IS_RADAR_DETECTED(pMac)         (pMac->lim.gLimSpecMgmt.fRadarDetCurOperChan)
#define LIM_SET_RADAR_DETECTED(pMac, val)   (pMac->lim.gLimSpecMgmt.fRadarDetCurOperChan = val)
#define LIM_MIN_BCN_PR_LENGTH  12
#define LIM_BCN_PR_CAPABILITY_OFFSET 10

typedef enum eMgmtFrmDropReason
{
    eMGMT_DROP_NO_DROP,
    eMGMT_DROP_NOT_LAST_IBSS_BCN,
    eMGMT_DROP_INFRA_BCN_IN_IBSS,
    eMGMT_DROP_SCAN_MODE_FRAME,
    eMGMT_DROP_NON_SCAN_MODE_FRAME,
    eMGMT_DROP_INVALID_SIZE,
}tMgmtFrmDropReason;




/// During TD ring clean up at HDD in RTAI, will call this call back
extern void limPostTdDummyPktCallbak(void* pMacGlobals, unsigned int* pBd);

/**
 * Function to initialize LIM state machines.
 * This called upon LIM thread creation.
 */
extern tSirRetStatus limInitialize(tpAniSirGlobal);
tSirRetStatus peOpen(tpAniSirGlobal pMac, tMacOpenParameters *pMacOpenParam);
tSirRetStatus peClose(tpAniSirGlobal pMac);
#ifdef FEATURE_WLAN_INTEGRATED_SOC
tSirRetStatus limStart(tpAniSirGlobal pMac);
#endif
/**
 * Function to Initialize radar interrupts.
 */
void limRadarInit(tpAniSirGlobal pMac);

tSirRetStatus peStart(tpAniSirGlobal pMac);
void peStop(tpAniSirGlobal pMac);
tSirRetStatus pePostMsgApi(tpAniSirGlobal pMac, tSirMsgQ* pMsg);
tSirRetStatus peProcessMsg(tpAniSirGlobal pMac, tSirMsgQ* limMsg);
void limDumpInit(tpAniSirGlobal pMac);

/**
 * Function to cleanup LIM state.
 * This called upon reset/persona change etc
 */
extern void limCleanup(tpAniSirGlobal);

/// Function to post messages to LIM thread
extern tANI_U32  limPostMsgApi(tpAniSirGlobal, tSirMsgQ *);

/**
 * Function to fetch messages posted LIM thread
 */
extern void limProcessMessageQueue(tpAniSirGlobal);

/**
 * Function to process messages posted to LIM thread
 * and dispatch to various sub modules within LIM module.
 */
extern void limMessageProcessor(tpAniSirGlobal, tpSirMsgQ);
extern void limProcessMessages(tpAniSirGlobal, tpSirMsgQ); // DT test alt deferred 2

/**
 * Function to check the LIM state if system can be put in
 * Learn Mode.
 * This is called by SCH upon receiving SCH_START_LEARN_MODE
 * message from LIM.
 */
extern tSirRetStatus limCheckStateForLearnMode(tpAniSirGlobal);

/**
 * Function to check the LIM state if system is in Scan/Learn state.
 */
extern tANI_U8 limIsSystemInScanState(tpAniSirGlobal);

#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
/**
 * Function to setup Polaris into Learn mode.
 * This is also called by SCH upon receiving SCH_START_LEARN_MODE
 * message from LIM.
 */
extern void limSetLearnMode(tpAniSirGlobal);

/**
 * Function to re-enable Learn mode measurements
 */
extern void limReEnableLearnMode(tpAniSirGlobal);

#endif //#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))

/**
 * Function to handle IBSS coalescing.
 * Beacon Processing module to call this.
 */
extern tSirRetStatus limHandleIBSScoalescing(tpAniSirGlobal,
                                              tpSchBeaconStruct,
                                              tANI_U8 *,tpPESession);

/// Function used by other Sirius modules to read global SME state
 static inline tLimSmeStates
limGetSmeState(tpAniSirGlobal pMac) { return pMac->lim.gLimSmeState; }

/// Function used by other Sirius modules to read global system role
 static inline tLimSystemRole
limGetSystemRole(tpPESession psessionEntry) { return psessionEntry->limSystemRole; }

//limGetAID(tpPESession psessionEntry) { return psessionEntry->limAID; }

extern void limReceivedHBHandler(tpAniSirGlobal, tANI_U8, tpPESession);
//extern void limResetHBPktCount(tpPESession);

extern void limCheckAndQuietBSS(tpAniSirGlobal);

/// Function to send WDS info to WSM if needed
extern void limProcessWdsInfo(tpAniSirGlobal, tSirPropIEStruct);

/// Function to initialize WDS info params
extern void limInitWdsInfoParams(tpAniSirGlobal);

/// Function that triggers STA context deletion
extern void limTriggerSTAdeletion(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpPESession psessionEntry);


/// Function that checks for change in AP's capabilties on STA
extern void limDetectChangeInApCapabilities(tpAniSirGlobal,
                                             tpSirProbeRespBeacon,tpPESession);
tSirRetStatus limUpdateShortSlot(tpAniSirGlobal pMac, 
                                                            tpSirProbeRespBeacon pBeacon, 
                                                            tpUpdateBeaconParams pBeaconParams,tpPESession);


/// creates an addts request action frame and sends it out to staid
extern void limSendAddtsReq (tpAniSirGlobal pMac, tANI_U16 staid, tANI_U8 tsid, tANI_U8 userPrio, tANI_U8 wme);
/// creates a delts request action frame and sends it out to staid
extern void limSendDeltsReq (tpAniSirGlobal pMac, tANI_U16 staid, tANI_U8 tsid, tANI_U8 userPrio, tANI_U8 wme);
/// creates a SM Power State Mode update request action frame and sends it out to staid
extern void limPostStartLearnModeMsgToSch(tpAniSirGlobal pMac);

extern ePhyChanBondState limGetPhyCBState( tpAniSirGlobal pMac );
tSirMacHTSecondaryChannelOffset    limGetHTCBState(tAniCBSecondaryMode aniCBMode);
tAniCBSecondaryMode     limGetAniCBState( tSirMacHTSecondaryChannelOffset htCBMode) ;





tANI_U8 limIsSystemInActiveState(tpAniSirGlobal pMac);

#if 0 /* Currently, this function is not used but keep it around for when we do need it */
tSirRetStatus limUpdateGlobalChannelBonding(tpAniSirGlobal pMac, tHalBitVal cbBit);
#endif /* 0 */


#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
extern void setupQuietBss( tpAniSirGlobal pMac, tANI_U32 learnInterval );
extern tANI_BOOLEAN limUpdateQuietIEInBeacons( tpAniSirGlobal pMac );
#endif

#ifdef ANI_AP_SDK
extern void limConvertScanDuration(tpAniSirGlobal pMac);
#endif /* ANI_AP_SDK */

#if (WNI_POLARIS_FW_PRODUCT == AP)
tSirRetStatus limProcessCcaMonitorModeChangeNotification(tpAniSirGlobal pMac, tANI_U32 ccaCbMode);
#endif /* WNI_POLARIS_FW_PRODUCT == AP */

void limHandleLowRssiInd(tpAniSirGlobal pMac);
void limHandleBmpsStatusInd(tpAniSirGlobal pMac);
void limHandleMissedBeaconInd(tpAniSirGlobal pMac);
tMgmtFrmDropReason limIsPktCandidateForDrop(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo, tANI_U32 subType);



/* ----------------------------------------------------------------------- */
// These used to be in DPH
extern void limSetBssid(tpAniSirGlobal pMac, tANI_U8 *bssId);
extern void limGetBssid(tpAniSirGlobal pMac, tANI_U8 *bssId);
extern void limGetMyMacAddr(tpAniSirGlobal pMac, tANI_U8 *mac);
extern tSirRetStatus limCheckRxSeqNumber(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo);

#define limGetQosMode(psessionEntry, pVal) *(pVal) = (psessionEntry)->limQosEnabled
#define limGetWmeMode(psessionEntry, pVal) *(pVal) = (psessionEntry)->limWmeEnabled
#define limGetWsmMode(psessionEntry, pVal) *(pVal) = (psessionEntry)->limWsmEnabled
#define limGet11dMode(psessionEntry, pVal) *(pVal) = (psessionEntry)->lim11dEnabled
#define limGetAckPolicy(pMac, pVal)         *(pVal) = pMac->lim.ackPolicy

/* ----------------------------------------------------------------------- */
static inline void limGetPhyMode(tpAniSirGlobal pMac, tANI_U32 *phyMode, tpPESession psessionEntry)
{
   *phyMode = psessionEntry ? psessionEntry->gLimPhyMode : pMac->lim.gLimPhyMode;
}

/* ----------------------------------------------------------------------- */
static inline void limGetRfBand(tpAniSirGlobal pMac, tSirRFBand *band, tpPESession psessionEntry)
{
   *band = psessionEntry ? psessionEntry->limRFBand : SIR_BAND_UNKNOWN;
}

/*--------------------------------------------------------------------------
  
  \brief peProcessMessages() - Message Processor for PE
  
  Voss calls this function to dispatch the message to PE
  
  \param pMac - Pointer to Global MAC structure
  \param pMsg - Pointer to the message structure
  
  \return  tANI_U32 - TX_SUCCESS for success.
  
  --------------------------------------------------------------------------*/

tSirRetStatus peProcessMessages(tpAniSirGlobal pMac, tSirMsgQ* pMsg);

/** -------------------------------------------------------------
\fn peFreeMsg
\brief Called by VOS scheduler (function vos_sched_flush_mc_mqs)
\      to free a given PE message on the TX and MC thread.
\      This happens when there are messages pending in the PE 
\      queue when system is being stopped and reset. 
\param   tpAniSirGlobal pMac
\param   tSirMsgQ       pMsg
\return none
-----------------------------------------------------------------*/
v_VOID_t peFreeMsg( tpAniSirGlobal pMac, tSirMsgQ* pMsg);

/************************************************************/
#endif /* __LIM_API_H */

