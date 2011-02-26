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




#include <sys_defs.h>
#ifdef __cplusplus
extern "C" 
{
#endif


#ifdef FEATURE_WLANFW_PHY_DEBUG
#define MAX_RF_STR_SIZE 70
extern const char rfFieldStr[NUM_RF_FIELDS][MAX_RF_STR_SIZE];
#define DUMP_RF_FIELD(fieldId)

#else
#define DUMP_RF_FIELD(fieldId)

#endif

#if defined(FEATURE_WLANFW_PHY_DEBUG) && defined(FEATURE_WLANFW_RF_ACCESS)

#define SET_RF_CHIP_REG(addr, regVal)                                                       \
{                                                                                           \
    CorexIo_Write((addrType)addr, (unsigned int)regVal);                                 \
}

#define GET_RF_CHIP_REG(addr, pVal)                                                      \
{                                                                                           \
    *pVal = CorexIo_Read((addrType)addr);                                                \
}

#elif defined(FEATURE_WLANFW_RF_ACCESS)
#define SET_RF_CHIP_REG(addr, regVal)                                                       \
{                                                                                           \
    CorexIo_Write((addrType)addr, (unsigned int)regVal);         \
}

#define GET_RF_CHIP_REG(addr, pVal)                                                       \
{                                                                                         \
    *pVal = CorexIo_Read((addrType)addr);                      \
}

#else
#define SET_RF_CHIP_REG(regAddr, regVal)

#define GET_RF_CHIP_REG(regAddr, pVal) { *pVal = 0; }

#endif //FEATURE_WLANFW_PHY_DEBUG



void rfWriteDataField(tANI_U32 regNum, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 data);
void rfReadDataField(tANI_U32 regAddr, tANI_U32 dataMask, tANI_U32 dataShift, tANI_U32 *pData);

#define SET_RF_FIELD(regAddr, mask, shift, regVal)                                    \
{                                                                                           \
    rfWriteDataField(regAddr, mask, shift, regVal);              \
}


#define GET_RF_FIELD(regAddr, mask, shift, pRegVal)                                    \
{                                                                                           \
    rfReadDataField(regAddr, mask, shift, pRegVal);              \
}






//initialization, enable, and chain selection
void rfInit();
void rfInitVolans2();
void rfGetVersion(tRfChipVer *chipVer);
void rfSetChainSelectionMode(ePhyChainSelect rfSysRxTxAntennaMode);
void rfSetModeSelectionControl(ePhyChainSelect phyChainSelections);
void rfSetRxGainLut(ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRfRxGain *gainTable);
void rfSetTxGainLut(ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRfTxGain *gainTable);

//channel/band/freq functions
eRfSubBand rfGetBand(eRfChannels chan);
eRfSubBand rfGetAGBand();
void rfSetCurChannel(eRfChannels chan);
eRfChannels rfGetCurChannel();
void rfSetChanBondMode(tANI_BOOLEAN onOff);
void rfRelockSynth();
void rfGetSynthLocks(eRfSynthLock *retSynthLock);
tANI_U16 rfChIdToFreqCoversion(tANI_U8 chanNum);
eRfChannels rfGetChannelIndex(tANI_U8 chanNum, ePhyChanBondState cbState);
eRfChannels rfGetIndexFromFreq(tANI_U16 chanFreq, ePhyChanBondState cbState);
tANI_U8 rfGetChannelIdFromIndex(eRfChannels chIndex);
eRfChannels rfGetClosest20MHzChannel(eRfChannels chIndex);


//calibration support functions
tTempADCVal rfTakeTemp(eRfTempSensor tempSensor);
tRfADCVal rfReadAdc(tANI_U8 nSamples, eHdetAdcSetup setup);
eTemperatureBins DetermineTemperatureBin(tTempADCVal temperature, tTempADCVal roomTemp);
eRfCalMode rfGetCalMode();
void rfSetCalMode(eRfCalMode calMode, ePhyRxChains rxChain, ePhyTxChains txChain, eGainSteps gain);

void rfSetDCOffset(ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect offset);
void rfGetDCOffset(ePhyRxChains rxChain, tANI_U8 dcoIndex, tRxDcoCorrect *offset);
void rfComputeDcoStep(tANI_U8 rfGainIndex, tANI_S32 *dcoStep);
void rfAdjustDCO(ePhyRxChains rxChain, tANI_U8 dcoIndex, tIQAdc dco);
void rfGetTxLoCorrect(ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect *corr);
void rfSetTxLoCorrect(ePhyTxChains txChain, eGainSteps txGain, tTxLoCorrect corr);
void rfSetIm2Correct(tRxIm2Correct im2Corr);
void rfGetIm2Correct(tRxIm2Correct *im2Corr);
void disableIm2ToneGen(tANI_BOOLEAN inBand);
void enableIm2ToneGen(tANI_BOOLEAN inBand);

tANI_U8 rfHdetDCOCal(void);
tANI_U16 getHdetDCOffset(void);
void rfRTuningCal(void);
tANI_U16 rfCTuningCal(void);
void rfInsituTuningCal(tANI_U16 rcMeas, tANI_U8 rTune);
tANI_U16 rfProcessMonitorCal(void);
tANI_S8 rfLNABiasSetting(tANI_U8 rTune);
tANI_U8 rfLNABandTuning(void);
tANI_U8 rfLNAGainAdjust(void);
tANI_U16 rfVcoFreqLinearityCal(void);
void rfSetNormalRxSensSettings(void);
void rfSetLowPwrRxSensSettings(void);


#ifdef FEATURE_WLANFW_PHY_DEBUG
void dumpRfCalCorrectionValues(eInitCals calId);
void dump_all_rf_fields();
#endif


#ifdef __cplusplus
}
#endif


#endif /* RFAPI_H */
