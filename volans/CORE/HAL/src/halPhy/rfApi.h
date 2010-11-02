/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   rfApi.h: All RF chip functions
   Author:  Mark Nelson
   Date:    2/18/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef RFAPI_H
#define RFAPI_H

#ifdef VERIFY_HALPHY_SIMV_MODEL //To get rid of multiple definition error, in eazy way.
#define rfGetBand               host_rfGetBand
#define rfGetAGBand             host_rfGetAGBand
#define rfGetCurChannel         host_rfGetCurChannel
#define rfChIdToFreqCoversion   host_rfChIdToFreqCoversion
#define rfGetChannelIndex       host_rfGetChannelIndex
#define rfGetChannelIdFromIndex host_rfGetChannelIdFromIndex
#endif


#include "sys_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define DUMP_RF_FIELD(fieldId)


eHalStatus rfWriteField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data);
eHalStatus rfReadField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData);
eHalStatus rfReadReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 *value);
eHalStatus rfWriteReg(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 value);
eRfSubBand rfGetBand(tpAniSirGlobal pMac, eRfChannels chan);
eRfSubBand rfGetAGBand(tpAniSirGlobal pMac);
eRfChannels rfGetCurChannel(tpAniSirGlobal pMac);
tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum);
eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState);
tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex);



#ifdef VERIFY_HALPHY_SIMV_MODEL
eHalStatus rfSetDCOffset(ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset);
eHalStatus rfGetDCOffset(ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset);
eHalStatus rfGetTxLoCorrect(ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect *corr);
eHalStatus rfSetTxLoCorrect(ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect corr);
void rfTakeTemp(tTempADCVal *retTemperature);
void rfGenerateTone(tANI_U8 lutIdx, tANI_U8 band);
#else
eHalStatus rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset);
eHalStatus rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset);
eHalStatus rfGetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect *corr);
eHalStatus rfSetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect corr);
eHalStatus rfTakeTemp(tpAniSirGlobal pMac, eRfTempSensor setup, tANI_U8 nSamples, tTempADCVal *retTemp);
#endif

void dump_all_rf_fields(tpAniSirGlobal pMac);



#ifdef __cplusplus
}
#endif


#endif /* RFAPI_H */
