/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

  
   pttModuleApi.h: Interface to change packet testing parameters used for phy layer testing
   Author:  Mark Nelson
   Date:    6/13/05

   History - 
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#ifndef PTT_MODULE_API_H
#define PTT_MODULE_API_H

#include "sys_api.h"


#if defined(ANI_PHY_DEBUG)  //always defined for ANI_MANF_DIAG
#include "pttMsgApi.h"

//these are accessible for mfg and debug production drivers
void pttProcessMsg(tpAniSirGlobal pMac, tPttMsgbuffer *pttMsg);

ePttStatus pttDbgReadRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 *regValue);
ePttStatus pttDbgWriteRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regValue);
ePttStatus pttDbgReadMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf);
ePttStatus pttDbgWriteMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf);

#endif



#ifdef ANI_MANF_DIAG



void pttModuleInit(tpAniSirGlobal pMac);    //sets default values for state variables

//Init
ePttStatus pttMsgInit(tpAniSirGlobal pMac, tPttModuleVariables** ptt);

//EEPROM Service
ePttStatus pttGetTpcCalState(tpAniSirGlobal pMac, eTpcCalState* calState);
ePttStatus pttResetTpcCalState(tpAniSirGlobal pMac, eTpcCalState* calState);

ePttStatus pttSetEepromCksum(tpAniSirGlobal pMac, tANI_U32 *cksum, tANI_BOOLEAN isFixPart);
ePttStatus pttGetEepromCksum(tpAniSirGlobal pMac, tANI_U32 *cksum, tANI_U32 *computedCksum, tANI_BOOLEAN isFixPart);

ePttStatus pttGetEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData);
ePttStatus pttSetEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable, uEepromTables *tableData);
ePttStatus pttDelEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable);
ePttStatus pttSetEepromImage(tpAniSirGlobal pMac, tANI_U32 offset, tANI_U32 len, tANI_U8 *pBuf, tANI_BOOLEAN toCache);
ePttStatus pttGetEepromImage(tpAniSirGlobal pMac, tANI_U32 offset, tANI_U32 len, tANI_U8 *pBuf, tANI_BOOLEAN fromCache);
ePttStatus pttBlankEeprom(tpAniSirGlobal pMac);
ePttStatus pttGetEepromField(tpAniSirGlobal pMac, eEepromField eepromField, uEepromFields *fieldData);
ePttStatus pttSetEepromField(tpAniSirGlobal pMac, eEepromField eepromField, uEepromFields *fieldData);
ePttStatus pttStoreEepromTable(tpAniSirGlobal pMac, eEepromTable eepromTable);
ePttStatus pttSetRegDomain(tpAniSirGlobal pMac, eRegDomainId regDomainId);


//Device Register Access
ePttStatus pttApiWriteRegister(tpAniSirGlobal pMac, eApiRegister regId, tANI_U32 regValue);
ePttStatus pttApiReadRegister(tpAniSirGlobal pMac, eApiRegister regId, tANI_U32 *regValue);


//Device MAC Test Setup
ePttStatus pttSetChannel(tpAniSirGlobal pMac, tANI_U32 chId, ePhyChanBondState cbState);
ePttStatus pttEnableChains(tpAniSirGlobal pMac, ePhyChainSelect chainSelection);


//Tx Waveform Gen Service
ePttStatus pttSetWaveform(tpAniSirGlobal pMac, tWaveformSample *waveform, tANI_U16 numSamples, tANI_BOOLEAN clk80);
ePttStatus pttSetTxWaveformGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain);
ePttStatus pttGetWaveformPowerAdc(tpAniSirGlobal pMac, tANI_U8 *tx0PowerAdc, tANI_U8 *tx1PowerAdc);
ePttStatus pttStartWaveform(tpAniSirGlobal pMac);
ePttStatus pttStopWaveform(tpAniSirGlobal pMac);



//Tx Frame Gen Service
ePttStatus pttConfigTxPacketGen(tpAniSirGlobal pMac, sPttFrameGenParams frameParams);
ePttStatus pttStartStopTxPacketGen(tpAniSirGlobal pMac, tANI_BOOLEAN startStop);
ePttStatus pttQueryTxStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, tANI_BOOLEAN *busy);

//Tx Frame Power Service
ePttStatus pttCloseTpcLoop(tpAniSirGlobal pMac, tANI_BOOLEAN tpcClose);

    //open loop service
ePttStatus pttSetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *gainTable);
ePttStatus pttGetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *gainTable);
ePttStatus pttSetPacketTxGainIndex(tpAniSirGlobal pMac, tANI_U8 index);
ePttStatus pttForcePacketTxGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain);

    //closed loop(CLPC) service
ePttStatus pttSetPwrIndexSource(tpAniSirGlobal pMac, ePowerTempIndexSource indexSource);
ePttStatus pttSetTxPower(tpAniSirGlobal pMac, t2Decimal dbmPwr);
ePttStatus pttGetTxPowerReport(tpAniSirGlobal pMac, tTxPowerReport *pwrTempIndex);
ePttStatus pttSetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *powerLut);
ePttStatus pttGetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *powerLut);
ePttStatus pttSaveTxPwrFreqTable(tpAniSirGlobal pMac, tANI_U8 numTpcCalFreqs, const tTpcFreqData *table);


//Rx Gain Service
ePttStatus pttDisableAgcTables(tpAniSirGlobal pMac, tANI_U8 rx0Gain, tANI_U8 rx1Gain, tANI_U8 rx2Gain);
ePttStatus pttEnableAgcTables(tpAniSirGlobal pMac, tANI_BOOLEAN rx0, tANI_BOOLEAN rx1, tANI_BOOLEAN rx2);
ePttStatus pttSetAgcTables(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRxGain *rxGainTable);
ePttStatus pttGetAgcTable(tpAniSirGlobal pMac, ePhyRxChains rxChain, tRxGain *agcTable /* NUM_AGC_GAINS */);
void pttGetRxRssi(tpAniSirGlobal pMac, tANI_U8 *rx0Rssi, tANI_U8 *rx1Rssi, tANI_U8 *rx2Rssi);


//Rx Frame Catcher Service
ePttStatus pttSetRxDisableMode(tpAniSirGlobal pMac, tANI_BOOLEAN aPktsDisabled, tANI_BOOLEAN bPktsDisabled, tANI_BOOLEAN chanBondPktsDisabled);
ePttStatus pttGetRxPktCounts(tpAniSirGlobal pMac, tANI_U32 *numRxPackets);
ePttStatus pttResetRxPacketStatistics(tpAniSirGlobal pMac);


//Rx Symbol Service
ePttStatus pttGrabRam(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples, tGrabRamSample *grabRam);


//Phy Calibration Service
ePttStatus pttRxDcoCal(tpAniSirGlobal pMac, tRxDcoCorrect *rx0Dco, tRxDcoCorrect *rx1Dco, tRxDcoCorrect *rx2Dco, eGainSteps gain);
ePttStatus pttRxIqCal(tpAniSirGlobal pMac, sIQCalValues* rx0, sIQCalValues* rx1, sIQCalValues* rx2, eGainSteps gain);
ePttStatus pttTxIqCal(tpAniSirGlobal pMac, sIQCalValues* tx0, sIQCalValues* tx1, eGainSteps gain);
ePttStatus pttTxCarrierSuppressCal(tpAniSirGlobal pMac, tTxLoCorrect* tx0, tTxLoCorrect *tx1, eGainSteps gain);
ePttStatus pttExecuteInitialCals(tpAniSirGlobal pMac);

//Phy Calibration Override Service
ePttStatus pttSetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, tTxLoCorrect tx0, tTxLoCorrect tx1, eGainSteps gain);
ePttStatus pttGetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, tTxLoCorrect* tx0, tTxLoCorrect *tx1, eGainSteps gain);
ePttStatus pttSetTxIqCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, sIQCalValues correct, eGainSteps gain);
ePttStatus pttGetTxIqCorrect(tpAniSirGlobal pMac, ePhyTxChains txChain, sIQCalValues *correct, eGainSteps gain);
ePttStatus pttSetRxIqCorrect(tpAniSirGlobal pMac, ePhyRxChains rxChain, sIQCalValues correct, eGainSteps gain);
ePttStatus pttGetRxIqCorrect(tpAniSirGlobal pMac, ePhyRxChains rxChain, sIQCalValues *correct, eGainSteps gain);
ePttStatus pttSetRxDcoCorrect(tpAniSirGlobal pMac, tRxDcoCorrect rx0Dco, tRxDcoCorrect rx1Dco, tRxDcoCorrect rx2Dco, eGainSteps gain);
ePttStatus pttGetRxDcoCorrect(tpAniSirGlobal pMac, tRxDcoCorrect* rx0Dco, tRxDcoCorrect *rx1Dco, tRxDcoCorrect *rx2Dco, eGainSteps gain);




//Rf Services
ePttStatus pttGetTempAdc(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *tempAdc);
ePttStatus pttReadQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 *value);
ePttStatus pttWriteQuasarField(tpAniSirGlobal pMac, eQuasarFields quasarField, tANI_U32 value);


//Misc.
ePttStatus pttSystemReset(tpAniSirGlobal pMac);
ePttStatus pttLogDump(tpAniSirGlobal pMac, tANI_U32 cmd, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4);

#endif
#endif
