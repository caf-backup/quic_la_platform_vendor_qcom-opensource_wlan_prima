/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

   asicApi.h: All ASIC accesses encapsulated here
   Author:  Mark Nelson
   Date:    4/1/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICAPI_H
#define ASICAPI_H

#include <sys_defs.h>
#include <phyDebug.h>

//use these macros to set/get register valuse and handle failures by returning immediately

/*  The following SET/GET_PHY_REG is used for debug purposes to record all phy register access.
    It was reported that this method was taking too much time on some platforms, so it was reduced to the
    basic minimum. Leaving it #if 0'd, so we can use it when we need to compare register differences.

    We are leaving the debug version to use the syncronous version of register writing, but moving to asyncronous
    register writes, primarily to facilitate USB driver efficiency.
*/

#define DELAY_MICROSECONDS(time) CorexTime_CpuUDelay(time) //this is in microseconds

#define palFillMemory(pDest, len, ch) \
{                                                                                                               \
    Lib_MemSet(pDest, ch, len);                                                                                  \
}                                                                                                               \

#define SET_PHY_REG(regAddr, regVal) \
{                                                                                                               \
    CorexIo_Write(regAddr, regVal);                                                                             \
}

#define GET_PHY_REG(regAddr, pVal) \
{                                                                                                               \
    *pVal = CorexIo_Read(regAddr);                                                                              \
}


#define SET_PHY_MEMORY(memOffset, pBuf, numWords) Lib_MemCpy4((void *)memOffset, pBuf, (4 * numWords))

#define GET_PHY_MEMORY(memOffset, pBuf, numWords) Lib_MemCpy4(pBuf, (void *)memOffset, (4 * numWords))

#define WAIT_PHY_REG_VAL(reg, mask, waitRegVal, perIterWaitInNanoSec, numIter, pReadRegVal)


#ifdef FEATURE_WLANFW_PHY_DEBUG

#define DUMP_PHY_REG(addr)
#else
#define DUMP_PHY_REG(addr)
#endif


//pTimestamp and timestamp are tANI_U32
#define GET_TIMESTAMP(pTimestamp) { *pTimestamp = CorexTime_GetUs(); }
#define ELAPSED_MICROSECONDS(timestamp) CorexTime_GetPassedUs(timestamp)

#define EXTEND_SIGNED_VALUE_TO_S16(value, input_sign_bit) (((tANI_S16)((value) << (15 - (input_sign_bit)))) >> (15 - (input_sign_bit)))

#define ABS(x)      (((tANI_S32)(x) < 0) ? -(x) : (x))
#define SIGN(x)     (((tANI_S32)(x) < 0) ? -1 : (((tANI_S32)(x) > 0) ? 1 : 0))

#ifdef VERIFY_HALPHY_SIMV_MODEL
#define CONST
#else
#define CONST const
#endif

extern CONST sCalControlBitmask calControl;

tANI_U32 EndianFlip(tANI_U32 data, tANI_U8 msbDataBitNumber);
tANI_U32 SignExtend(tANI_U32 data, tANI_U8 nBits);
tANI_U32 power(tANI_U16 x, tANI_U16 y);
tANI_S16 TwosComp(tANI_U16 val, tANI_U16 nbits);
tANI_U16 signMagnitude(tANI_S16 val, tANI_U8 nBits);



/*
    rdModWrNovaField perform a read-modify-write operation for use with registers that contain bits that need to change independently of others
    The regAddr, mask, and shift parameters come from virgo.h
    The val parameter is the bit values to put in the mask location of the register. This will be shifted to the mask location.
*/
void rdModWrAsicField(tANI_U32 regAddr, tANI_U32 mask, tANI_U32 offset, tANI_U32 val);
/*
    converts a signed integer to unsigned.
    for eg: -5(1000) is converted to (10101)
*/
tANI_S16 convert_sign_mag(tANI_S16 value, tANI_U16 nbits);


void asicEnablePhyClocks(void);
void phyAsicInit();
void phyAsicSetFreq(tANI_U16 freq);
void phyAsicRfEnBand(eRfSubBand band);
void phyAsicRfEnChains(ePhyChainSelect txChains);



//asicCal functions
#ifdef FEATURE_WLANFW_RF_ACCESS
void asicGetPhyCalMode(eCalMode *mode);
void asicSetPhyCalMode(eCalMode mode);
#endif
void asicSetPhyCalLength(tANI_U16 numSamples);
void asicGetPhyCalLength( tANI_U32 *length);
void asicSetPhyCalDCOTimers();
void asicZeroFineDCOCorrection(ePhyRxChains rxChain);
void asicPerformCalMeasurement();
void asicInitCalMem();
void asicWriteRxPhaseCorrection(eGainSteps gain, ePhyRxChains rxChain, sIQCalValues iqCorrect);
void asicReadRxPhaseCorrection(eGainSteps gain, ePhyRxChains rxChain, sIQCalValues *iqCorrect);
void asicGetCalADCSamples(tIQAdc *dco);
void asicPrepareRxIQCal(ePhyRxChains rxChain);
void asicFinishIQCal();


//asicAGC functions
void asicAGCHoldInReset();
void asicAGCReset();
void asicAGCInit();
void asicAGCCalcGainLutsForChannel(eRfChannels chan);
void asicSetAGCGainLut(ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRxGain *agcTable);
void asicGetAgcGainLut(ePhyRxChains rxChain, tRxGain *agcTable /* NUM_AGC_GAINS */);
void asicSetAgcCCAMode(ePhyCCAMode primaryCcaMode, ePhyCCAMode secondaryCcaMode);
void asicSetAGCChannelBondState(ePhyChanBondState chBondState);
void asicOverrideAGCRxChainGain(ePhyRxChains rxChain, tANI_U8 gain);
void asicCeaseOverrideAGCRxChainGain(ePhyRxChains rxChain);
void asicOverrideAGCRxEnable(eAGCRxChainMask rxMask, eAGCRxMode rxMode);
void asicSetDisabledRxPacketTypes(ePhyRxDisabledPktTypes modTypes);
void asicGetDisabledRxPacketTypes(ePhyRxDisabledPktTypes *disabledTypes);
void asicSetAGCRxChains(tANI_U8 numActiveRx, tANI_U8 numListenRx);
void asicSetAGCCrossbar(eAGCRxChainMask rxMaskEnabled);
void asicAGCSetDensity(tANI_BOOLEAN densityOn, ePhyNwDensity density20MHz, ePhyNwDensity density40MHz);

//asicAGCRadar functions
void asicAgcInitRadar();


//asicTPC functions
void asicTPCInit();
void asicTPCPowerOverride(tTxGain tx0);
void asicTPCAutomatic();
void asicLoadTPCPowerLUT(ePhyTxChains txChain, tANI_U8 *tpcPowerLUT);
void asicLoadTPCGainLUT(ePhyTxChains txChain, tTxGain *tpcGainLUT);
void asicTPCGetADCReading(tANI_U16 *pADC);

//asicTXCTL functions
void asicTxCtlInit();
void asicEnableTxDACs(ePhyTxChains txChainsOff, tANI_BOOLEAN override, tANI_BOOLEAN wfm);

//asicTxFir functions
void asicTxFirInit();
void asicTxFirInitMem();
void asicTxFirAdjustCarrierCorrection(ePhyTxChains txChain, tTxCarrierError error, tIQAdc *correction);
void asicTxFirSetChainBypass(ePhyTxChains txChain, tANI_BOOLEAN chainBypassEnable);
void asicTxFirSetPaOverride(tANI_BOOLEAN overrideEnable, ePhyTxChains chainPaEnables);
void asicTxFirSetLoLeakageBypass(tANI_BOOLEAN bypassEnable);
void asicTxFirSetIqImbalanceBypass(tANI_BOOLEAN bypassEnable);
void asicTxFirSetLoLeakageGainShift(ePhyTxChains txChain, tANI_U32 shiftVal2Bits);
void asicTxFirSetPostIqCalGainAdjust(tANI_U32 ch0Adjust3bits);
void asicTxFirSetTxCarrierCorrection(eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect correct);
void asicTxFirGetTxCarrierCorrection(eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect *correct);
void asicTxFirSetTxPhaseCorrection(eGainSteps gain, ePhyTxChains txChain, sIQCalValues correct);
void asicTxFirGetTxPhaseCorrection(eGainSteps gain, ePhyTxChains txChain, sIQCalValues *correct);
void asicTxFirApplyCoef(eRfChannels channel, eRegDomainId regDomain);
void asicTxFirApply20MHzOFDMCoef(eRfChannels channel, eRegDomainId regDomain);
void asicTxFirApply11bCoef(eRfChannels channel);


//asicFft functions
void asicFftGetToneData(tANI_U8 numTones, tANI_S8 toneNumbers[MAX_TONES_DATA_COLLECTION], sToneDataCollection *toneData);



//asicPMU.c
// void asicPMUInit();
// void asicPMURadioControl(ePMURadioControl radio);
// void asicSetPMUManualEnMode(ePMUEnableStatus pmuMode);
// tANI_BOOLEAN asicSetPowerDisable(tANI_BOOLEAN onOff);

//asicWfm.c
void asicSetupTestWaveform(const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80);
void asicStartTestWaveform(eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex, tANI_BOOLEAN useDbgMem);
void asicStopTestWaveform();
tANI_U8 limitDcoCorr(tANI_S16 dcVal);

typedef enum
{
    TEST_PAYLOAD_NONE,
    TEST_PAYLOAD_FILL_BYTE,
    TEST_PAYLOAD_RANDOM,
    TEST_PAYLOAD_RAMP,
    TEST_PAYLOAD_TEMPLATE,
    TEST_PAYLOAD_MAX = 0XFFFFFFFF, //dummy value to set enum to 4 bytes
}ePayloadContents;


void asicGrabAdcSamples(tANI_U32 startSample, tANI_U32 numSamples, tGrabRamSample *sampleBuffer);

#endif /* ASICAPI_H */
