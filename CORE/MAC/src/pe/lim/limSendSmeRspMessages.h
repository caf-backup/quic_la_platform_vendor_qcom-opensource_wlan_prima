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
void limSendSmeRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes);
void limSendSmeStartBssRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes);
void limSendSmeScanRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes);
void limPostSmeScanRspMessage(tpAniSirGlobal, tANI_U16, tSirResultCodes); 
void limSendSmeAuthRsp(tpAniSirGlobal, tSirResultCodes,
                       tSirMacAddr, tAniAuthType, tANI_U16);

void limSendSmeJoinReassocRsp(tpAniSirGlobal, tANI_U16, tSirResultCodes, tANI_U16);
void limSendSmeDisassocNtf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes, tANI_U16, tANI_U16);
void limSendSmeDisassocInd(tpAniSirGlobal, tpDphHashNode);
void limSendSmeDeauthInd(tpAniSirGlobal, tpDphHashNode);
void limSendSmeDeauthNtf(tpAniSirGlobal, tSirMacAddr, tSirResultCodes, tANI_U16, tANI_U16);
void limSendSmeWmStatusChangeNtf(tpAniSirGlobal, tSirSmeStatusChangeCode, tANI_U32 *, tANI_U16);
void limSendSmeSetContextRsp(tpAniSirGlobal,
                             tSirMacAddr, tANI_U16, tSirResultCodes);
void limSendSmePromiscuousModeRsp(tpAniSirGlobal pMac);
void limSendSmeNeighborBssInd(tpAniSirGlobal,
                              tLimScanResultNode *);
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
void limSendSmeMeasurementInd(tpAniSirGlobal);
#endif

void limSendSmeAddtsRsp(tpAniSirGlobal pMac, tANI_U8 rspReqd, tANI_U32 status, tSirMacTspecIE tspec);
void limSendSmeAddtsInd(tpAniSirGlobal pMac, tpSirAddtsReqInfo addts);
void limSendSmeDeltsRsp(tpAniSirGlobal pMac, tpSirDeltsReq delts, tANI_U32 status);
void limSendSmeDeltsInd(tpAniSirGlobal pMac, tpSirDeltsReqInfo delts, tANI_U16 aid);
void limSendSmeStatsRsp(tpAniSirGlobal pMac, tANI_U16 msgtype, void * stats);
void limSendSmePEStatisticsRsp(tpAniSirGlobal pMac, tANI_U16 msgtype, void * stats);
void limSendSmeRemoveKeyRsp(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, tSirResultCodes resultCode);
void limSendSmeGetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 power, tANI_U32 status);
void limSendSmeGetNoiseRsp(tpAniSirGlobal pMac, tSirMacNoise noise);
void limSendSmeIBSSPeerInd(tpAniSirGlobal pMac,tSirMacAddr peerMacAddr,tANI_U16 staIndex,tANI_U8 *beacon,tANI_U16 beaconLen, tANI_U16 msgType);
void limSendExitBmpsInd(tpAniSirGlobal pMac, tExitBmpsReason reasonCode);

#endif /* __LIM_SEND_SME_RSP_H */

