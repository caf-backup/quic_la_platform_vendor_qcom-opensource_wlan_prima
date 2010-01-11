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




#include "sys_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(ANI_PHY_DEBUG) || defined(ANI_MANF_DIAG)
#define MAX_RF_STR_SIZE 70
extern const char rfFieldStr[NUM_RF_FIELDS][MAX_RF_STR_SIZE];
#define DUMP_RF_FIELD(fieldId) \
{                                                                                                                               \
    if (fieldId < NUM_RF_FIELDS)                                                                                                \
    {                                                                                                                           \
        tANI_U32 _val0 = 0, _val1 = 0;                                                                                          \
        rfReadField(pMac, fieldId, &_val0, GEMINI_CHIP);                                                                        \
        phyLog(pMac, LOGE, "RF Field %d:%80s = 0x%08X\n", fieldId, &rfFieldStr[fieldId][0], _val0);                             \
    }                                                                                                                           \
    else                                                                                                                        \
    {                                                                                                                           \
        phyLog(pMac, LOGE, "fieldId %d not found in RF fields\n", fieldId);                                                     \
    }                                                                                                                           \
}


#else
#define DUMP_RF_FIELD(fieldId)

#endif


eHalStatus rfWriteField(tpAniSirGlobal pMac, eRfFields geminiField, tANI_U32 value, eRfChipSelect chipSel);
eHalStatus rfReadField(tpAniSirGlobal pMac, eRfFields geminiField, tANI_U32 *value, eRfChipSelect chipSel);
eRfSubBand rfGetBand(tpAniSirGlobal pMac, eRfChannels chan);
eRfSubBand rfGetAGBand(tpAniSirGlobal pMac);
eRfChannels rfGetCurChannel(tpAniSirGlobal pMac);
tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum);
eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState);
tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex);


#if defined(ANI_PHY_DEBUG) || defined(ANI_MANF_DIAG)
eHalStatus rfSetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset);
eHalStatus rfGetDCOffset(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset);
eHalStatus rfGetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect *corr);
eHalStatus rfSetTxLoCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect corr);
eHalStatus rfTakeTemp(tpAniSirGlobal pMac, tANI_U8 nSamples, tTempADCVal *retTemperature);

void dump_all_rf_fields(tpAniSirGlobal pMac);
#endif


#ifdef __cplusplus
}
#endif


#endif /* RFAPI_H */
