/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * limSendMessages.h: Provides functions to send messages or Indications to HAL.
 * Author:    Sunit Bhatia
 * Date:       09/21/2006
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */
#ifndef __LIM_SEND_MESSAGES_H
#define __LIM_SEND_MESSAGES_H



#include "aniGlobal.h"
#include "limTypes.h"
#include "halMsgApi.h"
#include "sirParams.h"    

tSirRetStatus limSendCFParams(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 cfpCount, tANI_U8 cfpPeriod);
tSirRetStatus limSendBeaconParams(tpAniSirGlobal pMac, 
                                tpUpdateBeaconParams pUpdatedBcnParams,
								tpPESession  psessionEntry );

//tSirRetStatus limSendBeaconParams(tpAniSirGlobal pMac, tpUpdateBeaconParams pUpdatedBcnParams);
#if defined WLAN_FEATURE_VOWIFI  
tSirRetStatus limSendSwitchChnlParams(tpAniSirGlobal pMac, tANI_U8 chnlNumber, 
							   tSirMacHTSecondaryChannelOffset secondaryChnlOffset, 
                               tPowerdBm maxTxPower,tANI_U8 peSessionId);
#else
tSirRetStatus limSendSwitchChnlParams(tpAniSirGlobal pMac, tANI_U8 chnlNumber, 
							   tSirMacHTSecondaryChannelOffset secondaryChnlOffset, 
                               tANI_U8 localPwrConstraint,tANI_U8 peSessionId);
#endif
tSirRetStatus limSendEdcaParams(tpAniSirGlobal pMac, tSirMacEdcaParamRecord *pUpdatedEdcaParams, tANI_U16 bssIdx, tANI_BOOLEAN highPerformance);
tSirRetStatus limSetLinkState(tpAniSirGlobal pMac, tSirLinkState state,  tSirMacAddr bssId);
tSirRetStatus limSendSetTxPowerReq(tpAniSirGlobal pMac, tpSirSetTxPowerReq pTxPowerReq);
tSirRetStatus limSendGetTxPowerReq(tpAniSirGlobal pMac, tpSirGetTxPowerReq pTxPowerReq);

void limSetActiveEdcaParams(tpAniSirGlobal pMac, tSirMacEdcaParamRecord *plocalEdcaParams);

#define CAPABILITY_FILTER_MASK  0x73DF
#define ERP_FILTER_MASK         0xF8
#define EDCA_FILTER_MASK        0xF0
#define QOS_FILTER_MASK         0xF0
#define HT_BYTE0_FILTER_MASK    0x0
#define HT_BYTE2_FILTER_MASK    0xEB
#define HT_BYTE5_FILTER_MASK    0xFD
#define DS_PARAM_CHANNEL_MASK   0x0


tSirRetStatus limSendBeaconFilterInfo(tpAniSirGlobal pMac);


#endif
