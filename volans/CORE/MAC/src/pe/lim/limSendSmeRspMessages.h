/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limSendSmeRspMessages.h contains the definitions for
 * sending SME response/notification messages to applications above
 * MAC software.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 
 */
#ifndef __LIM_SEND_SME_RSP_H
#define __LIM_SEND_SME_RSP_H

#include "sirCommon.h"
#include "sirApi.h"
#include "sirMacProtDef.h"


// Functions for sending responses to Host
void limSendSmeRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes, tANI_U8 , tANI_U16);
void limSendSmeStartBssRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes,tpPESession,tANI_U8,tANI_U16);
void limSendSmeScanRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes,tANI_U8, tANI_U16);
void limPostSmeScanRspMessage(tpAniSirGlobal, tANI_U16, tSirResultCodes,tANI_U8,tANI_U16); 
void limSendSmeAuthRsp(tpAniSirGlobal, tSirResultCodes,
                       tSirMacAddr, tAniAuthType, tANI_U16,tpPESession,tANI_U8,tANI_U16);

void limSendSmeJoinReassocRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes, tANI_U16,tpPESession,tANI_U8,tANI_U16);
void limSendSmeDisassocNtf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes, tANI_U16, tANI_U16,tANI_U8,tANI_U16,tpPESession);
void limSendSmeDeauthNtf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes, tANI_U16, tANI_U16, tANI_U8, tANI_U16);
void limSendSmeDisassocInd(tpAniSirGlobal, tpDphHashNode,tpPESession);
void limSendSmeDeauthInd(tpAniSirGlobal, tpDphHashNode, tpPESession psessionEntry);



void limSendSmeWmStatusChangeNtf(tpAniSirGlobal, tSirSmeStatusChangeCode, tANI_U32 *, tANI_U16);
void limSendSmeSetContextRsp(tpAniSirGlobal,
                             tSirMacAddr, tANI_U16, tSirResultCodes,tpPESession,tANI_U8,tANI_U16);
void limSendSmePromiscuousModeRsp(tpAniSirGlobal pMac);
void limSendSmeNeighborBssInd(tpAniSirGlobal,
                              tLimScanResultNode *);
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
void limSendSmeMeasurementInd(tpAniSirGlobal);
#endif
void limHandleDeleteBssRsp(tpAniSirGlobal pMac,tpSirMsgQ MsgQ);


void limSendSmeAddtsRsp(tpAniSirGlobal pMac, tANI_U8 rspReqd, tANI_U32 status, tpPESession psessionEntry, tSirMacTspecIE tspec, tANI_U8 smesessionId, tANI_U16 smetransactionId);
void limSendSmeAddtsInd(tpAniSirGlobal pMac, tpSirAddtsReqInfo addts);
void limSendSmeDeltsRsp(tpAniSirGlobal pMac, tpSirDeltsReq delts, tANI_U32 status,tpPESession psessionEntry,tANI_U8 smessionId,tANI_U16 smetransactionId);
void limSendSmeDeltsInd(tpAniSirGlobal pMac, tpSirDeltsReqInfo delts, tANI_U16 aid,tpPESession);
void limSendSmeStatsRsp(tpAniSirGlobal pMac, tANI_U16 msgtype, void * stats);

void limSendSmePEStatisticsRsp(tpAniSirGlobal pMac, tANI_U16 msgtype, void * stats);
void limSendSmeRemoveKeyRsp(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, tSirResultCodes resultCode,tpPESession,tANI_U8,tANI_U16);


void limSendSmeGetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 power, tANI_U32 status);
void limSendSmeGetNoiseRsp(tpAniSirGlobal pMac, tSirMacNoise noise);
void limSendSmeIBSSPeerInd(tpAniSirGlobal pMac,tSirMacAddr peerMacAddr,tANI_U16 staIndex,tANI_U8 ucastIdx,tANI_U8 bcastIdx,tANI_U8 *beacon,tANI_U16 beaconLen, tANI_U16 msgType);
void limSendExitBmpsInd(tpAniSirGlobal pMac, tExitBmpsReason reasonCode);

#ifdef FEATURE_INNAV_SUPPORT
void limSendSmeInNavMeasRsp(tpAniSirGlobal pMac, tANI_U32* pMsgBuf, tSirResultCodes resultCode);
#endif

#endif /* __LIM_SEND_SME_RSP_H */

