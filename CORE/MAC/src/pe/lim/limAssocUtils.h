/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limAssocUtils.h contains the utility definitions
 * LIM uses while processing Re/Association messages.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef __LIM_ASSOC_UTILS_H
#define __LIM_ASSOC_UTILS_H

#include "sirApi.h"
#include "sirDebug.h"
#include "cfgApi.h"

#include "limTypes.h"


tANI_U8         limCmpSSid(tpAniSirGlobal, tSirMacSSid *);
tANI_U8         limCompareCapabilities(tpAniSirGlobal,
                                       tSirAssocReq *,
                                       tSirMacCapabilityInfo *);
tANI_U8         limCheckRxBasicRates(tpAniSirGlobal, tSirMacRateSet);
tANI_U8         limCheckMCSSet(tpAniSirGlobal pMac, tANI_U8* supportedMCSSet);
void            limPostDummyToTmRing(tpAniSirGlobal, tpDphHashNode);
void            limPostPacketToTdRing(tpAniSirGlobal,
                                      tpDphHashNode,
                                      tANI_U8);
tSirRetStatus   limCleanupRxPath(tpAniSirGlobal, tpDphHashNode);
void            limRejectAssociation(tpAniSirGlobal , tSirMacAddr, tANI_U8,
                                     tANI_U8 , tAniAuthType,
                                     tANI_U16, tANI_U8, tSirResultCodes);

tSirRetStatus limPopulateOwnRateSet(tpAniSirGlobal pMac,
                                                                tpSirSupportedRates pRates,
                                                                tANI_U8* pSupportedMCSSet,
                                                                tANI_U8 basicOnly);

tSirRetStatus   limPopulateMatchingRateSet(tpAniSirGlobal,
                                           tpDphHashNode,
                                           tSirMacRateSet *,
                                           tSirMacRateSet *,
                                           tANI_U8* pSupportedMCSSet,
                                           tSirMacPropRateSet *);
tSirRetStatus   limAddSta(tpAniSirGlobal, tpDphHashNode);
tSirRetStatus   limDelBss(tpAniSirGlobal, tpDphHashNode, tANI_U16);
tSirRetStatus   limDelSta(tpAniSirGlobal, tpDphHashNode, tANI_BOOLEAN);
tSirRetStatus   limAddStaSelf(tpAniSirGlobal, tANI_U16, tANI_U8);
tStaRateMode limGetStaRateMode(tANI_U8 dot11Mode);

void            limTeardownInfraBss(tpAniSirGlobal);
void            limRestorePreReassocState(tpAniSirGlobal,
                                          tSirResultCodes,
                                          tANI_U16); 
eAniBoolean     limIsReassocInProgress(tpAniSirGlobal);
void
limSendDelStaCnf(tpAniSirGlobal pMac, tSirMacAddr staDsAddr,
       tANI_U16 staDsAssocId, tLimMlmStaContext mlmStaContext, tSirResultCodes statusCode);

void            limHandleCnfWaitTimeout(tpAniSirGlobal pMac, tANI_U16 staId);
void            limDeleteDphHashEntry(tpAniSirGlobal, tSirMacAddr, tANI_U16);
void            limCheckAndAnnounceJoinSuccess(tpAniSirGlobal,
                                               tSirProbeRespBeacon *,
                                               tpSirMacMgmtHdr);
void limUpdateReAssocGlobals(tpAniSirGlobal pMac,
                                    tpSirAssocRsp pAssocRsp);

void limUpdateAssocStaDatas(tpAniSirGlobal pMac, 
                                tpDphHashNode pStaDs,tpSirAssocRsp pAssocRsp);
void
limFillSupportedRatesInfo(
    tpAniSirGlobal          pMac,
    tpDphHashNode           pSta,
    tpSirSupportedRates   pRates);

#ifdef ANI_PRODUCT_TYPE_CLIENT
//make non-conditional until the caller is #ifdefed
tSirRetStatus limStaSendAddBss(tpAniSirGlobal pMac, tpSirAssocRsp pAssocRsp, 
                                    tpSchBeaconStruct pBeaconStruct, tpSirBssDescription bssDescription, tANI_U8 updateEntry);
tSirRetStatus limStaSendAddBssPreAssoc( tpAniSirGlobal pMac, tANI_U8 updateEntry);


#elif defined(ANI_AP_CLIENT_SDK)
tSirRetStatus limStaSendAddBss(tpAniSirGlobal pMac, tpSirAssocRsp pAssocRsp, 
                                    tpSirNeighborBssInfo neighborBssInfo, tANI_U8 updateEntry);
#endif

void limPrepareAndSendDelStaCnf(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tSirResultCodes statusCode);
tSirRetStatus limExtractApCapabilities(tpAniSirGlobal pMac, tANI_U8 * pIE, tANI_U16 ieLen, tpSirProbeRespBeacon beaconStruct);
void limInitPreAuthTimerTable(tpAniSirGlobal pMac, tpLimPreAuthTable pPreAuthTimerTable);
tpLimPreAuthNode limAcquireFreePreAuthNode(tpAniSirGlobal pMac, tpLimPreAuthTable pPreAuthTimerTable);
tpLimPreAuthNode limGetPreAuthNodeFromIndex(tpAniSirGlobal pMac, tpLimPreAuthTable pAuthTable, tANI_U32 authNodeIdx);

/* Util API to check if the channels supported by STA is within range */
tSirRetStatus limIsDot11hSupportedChannelsValid(tpAniSirGlobal pMac, tSirAssocReq *assoc);

/* Util API to check if the txpower supported by STA is within range */
tSirRetStatus limIsDot11hPowerCapabilitiesInRange(tpAniSirGlobal pMac, tSirAssocReq *assoc);

/* API to re-add the same BSS during re-association */
void limHandleAddBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs);

/* API to fill in RX Highest Supported data Rate */
void limFillRxHighestSupportedRate(tpAniSirGlobal pMac, tANI_U16 *rxHighestRate, tANI_U8* pSupportedMCSSet);


#endif /* __LIM_ASSOC_UTILS_H */

