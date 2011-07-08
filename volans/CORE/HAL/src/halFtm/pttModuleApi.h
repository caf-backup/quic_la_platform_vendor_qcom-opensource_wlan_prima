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

#include "sys_defs.h"



#include "pttMsgApi.h"

//these are accessible for mfg and debug production drivers
void pttProcessMsg(tpAniSirGlobal pMac, tPttMsgbuffer *pttMsg);

eQWPttStatus pttDbgReadRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 *regValue);
eQWPttStatus pttDbgWriteRegister(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regValue);
eQWPttStatus pttDbgReadMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf);
eQWPttStatus pttDbgWriteMemory(tpAniSirGlobal pMac, tANI_U32 memAddr, tANI_U32 nBytes, tANI_U32 *pMemBuf);



void pttModuleInit(tpAniSirGlobal pMac);    //sets default values for state variables

//Init
eQWPttStatus pttMsgInit(tpAniSirGlobal pMac, tPttModuleVariables** ptt);

//NV Service
eQWPttStatus pttGetNvTable(tpAniSirGlobal pMac, eNvTable nvTable, uNvTables *tableData);
eQWPttStatus pttSetNvTable(tpAniSirGlobal pMac, eNvTable nvTable, uNvTables *tableData);
eQWPttStatus pttDelNvTable(tpAniSirGlobal pMac, eNvTable nvTable);
eQWPttStatus pttBlankNv(tpAniSirGlobal pMac);
eQWPttStatus pttGetNvField(tpAniSirGlobal pMac, eNvField nvField, uNvFields *fieldData);
eQWPttStatus pttSetNvField(tpAniSirGlobal pMac, eNvField nvField, uNvFields *fieldData);
eQWPttStatus pttStoreNvTable(tpAniSirGlobal pMac, eNvTable nvTable);
eQWPttStatus pttSetRegDomain(tpAniSirGlobal pMac, eRegDomainId regDomainId);


//Device MAC Test Setup
eQWPttStatus pttSetChannel(tpAniSirGlobal pMac, tANI_U32 chId, ePhyChanBondState cbState);
eQWPttStatus pttEnableChains(tpAniSirGlobal pMac, ePhyChainSelect chainSelection);


//Tx Waveform Gen Service
eQWPttStatus pttSetWaveform(tpAniSirGlobal pMac, tWaveformSample *waveform, tANI_U16 numSamples, tANI_BOOLEAN clk80);
eQWPttStatus pttSetTxWaveformGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain);
eQWPttStatus pttSetRxWaveformGain(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 gain);
eQWPttStatus pttGetWaveformPowerAdc(tpAniSirGlobal pMac, sTxChainsPowerAdcReadings *txPowerAdc);
//eQWPttStatus pttStartWaveform(tpAniSirGlobal pMac, tANI_U32 startIndex, tANI_U32 numSamples);
eQWPttStatus pttStartWaveform(tpAniSirGlobal pMac);
eQWPttStatus pttStopWaveform(tpAniSirGlobal pMac);



//Tx Frame Gen Service
eQWPttStatus pttConfigTxPacketGen(tpAniSirGlobal pMac, sPttFrameGenParams frameParams);
eQWPttStatus pttStartStopTxPacketGen(tpAniSirGlobal pMac, tANI_BOOLEAN startStop);
eQWPttStatus pttQueryTxStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, tANI_BOOLEAN *busy);

//Tx Frame Power Service
eQWPttStatus pttCloseTpcLoop(tpAniSirGlobal pMac, tANI_BOOLEAN tpcClose);

    //open loop service
eQWPttStatus pttSetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *gainTable);
eQWPttStatus pttGetPacketTxGainTable(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *gainTable);
eQWPttStatus pttSetPacketTxGainIndex(tpAniSirGlobal pMac, tANI_U8 index);
eQWPttStatus pttForcePacketTxGain(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 gain);

    //closed loop(CLPC) service
eQWPttStatus pttSetPwrIndexSource(tpAniSirGlobal pMac, ePowerTempIndexSource indexSource);
eQWPttStatus pttSetTxPower(tpAniSirGlobal pMac, t2Decimal dbmPwr);
eQWPttStatus pttGetTxPowerReport(tpAniSirGlobal pMac, tTxPowerReport *pwrTempIndex);
eQWPttStatus pttSetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 minIndex, tANI_U8 maxIndex, tANI_U8 *powerLut);
eQWPttStatus pttGetPowerLut(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *powerLut);
eQWPttStatus pttUpdateTpcSplitLut(tpAniSirGlobal pMac, ePhyTxPwrRange pwrRange, tANI_U32 splitIdx);


//Rx Gain Service
eQWPttStatus pttDisableAgcTables(tpAniSirGlobal pMac, sRxChainsAgcDisable gains);
eQWPttStatus pttEnableAgcTables(tpAniSirGlobal pMac, sRxChainsAgcEnable enables);
void pttGetRxRssi(tpAniSirGlobal pMac, sRxChainsRssi *rssi);
void pttGetUnicastMacPktRxRssi(tpAniSirGlobal pMac, sRxChainsRssi *rssi);
void pttCollectAdcRssiStats(tpAniSirGlobal pMac);


//Rx Frame Catcher Service
eQWPttStatus pttSetRxDisableMode(tpAniSirGlobal pMac, sRxTypesDisabled disabled);
eQWPttStatus pttGetRxPktCounts(tpAniSirGlobal pMac, sRxFrameCounters *counters);
eQWPttStatus pttResetRxPacketStatistics(tpAniSirGlobal pMac);


//Rx Symbol Service
eQWPttStatus pttGrabRam(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples,
                                eGrabRamSampleType sampleType, tGrabRamSample *grabRam);


//Phy Calibration Service
eQWPttStatus pttRxDcoCal(tpAniSirGlobal pMac, tRxChainsDcoCorrections *calValues, tANI_U8 gain);
eQWPttStatus pttRxIm2Cal(tpAniSirGlobal pMac, tRxChainsIm2Corrections *calValues, tANI_U8 im2CalOnly);
eQWPttStatus pttRxIqCal(tpAniSirGlobal pMac, sRxChainsIQCalValues *calValues, eGainSteps gain);
eQWPttStatus pttTxIqCal(tpAniSirGlobal pMac, sTxChainsIQCalValues *calValues, eGainSteps gain);
eQWPttStatus pttTxCarrierSuppressCal(tpAniSirGlobal pMac, sTxChainsLoCorrections *calValues, eGainSteps gain);
eQWPttStatus pttExecuteInitialCals(tpAniSirGlobal pMac);
eQWPttStatus pttHdetCal(tpAniSirGlobal pMac, sRfHdetCalValues *hdetCalValues);

//Phy Calibration Override Service
eQWPttStatus pttSetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, sTxChainsLoCorrections calValues, eGainSteps gain);
eQWPttStatus pttGetTxCarrierSuppressCorrect(tpAniSirGlobal pMac, sTxChainsLoCorrections *calValues, eGainSteps gain);
eQWPttStatus pttSetTxIqCorrect(tpAniSirGlobal pMac, sTxChainsIQCalValues calValues, eGainSteps gain);
eQWPttStatus pttGetTxIqCorrect(tpAniSirGlobal pMac, sTxChainsIQCalValues *calValues, eGainSteps gain);
eQWPttStatus pttSetRxIqCorrect(tpAniSirGlobal pMac, sRxChainsIQCalValues calValues, eGainSteps gain);
eQWPttStatus pttGetRxIqCorrect(tpAniSirGlobal pMac, sRxChainsIQCalValues *calValues, eGainSteps gain);
eQWPttStatus pttSetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections calValues, tANI_U8 gain);
eQWPttStatus pttGetRxDcoCorrect(tpAniSirGlobal pMac, tRxChainsDcoCorrections *calValues, tANI_U8 gain);
eQWPttStatus pttGetRxIm2Correct(tpAniSirGlobal pMac, tRxChainsIm2Corrections *calValues, tANI_U8 dummy);
eQWPttStatus pttSetRxIm2Correct(tpAniSirGlobal pMac, tRxChainsIm2Corrections calValues, tANI_U8 dummy);



//Rf Services
eQWPttStatus pttGetTempAdc(tpAniSirGlobal pMac, eRfTempSensor tempSensor, tTempADCVal *tempAdc);
eQWPttStatus pttReadRfField(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 mask, tANI_U32 shift, tANI_U32 *value);
eQWPttStatus pttWriteRfField(tpAniSirGlobal pMac, tANI_U32 addr, tANI_U32 mask, tANI_U32 shift, tANI_U32 value);

//Deep sleep
eQWPttStatus pttDeepSleep(tpAniSirGlobal pMac);

//Misc.
eQWPttStatus pttSystemReset(tpAniSirGlobal pMac);

#endif
