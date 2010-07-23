/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicApi.h: All ASIC accesses encapsulated here
   Author:  Mark Nelson
   Date:    4/1/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICAPI_H
#define ASICAPI_H

#include "phyDebug.h"

//use these macros to set/get register valuse and handle failures by returning immediately
//requires eHalStatus retVal to be declared previously

/*  The following SET/GET_PHY_REG is used for debug purposes to record all phy register access.
    It was reported that this method was taking too much time on some platforms, so it was reduced to the
    basic minimum. Leaving it #if 0'd, so we can use it when we need to compare register differences.
    
    We are leaving the debug version to use the syncronous version of register writing, but moving to asyncronous
    register writes, primarily to facilitate USB driver efficiency.
*/

#if 0
#define SET_PHY_REG(hHdd, regAddr, regVal) \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                                          \
{                                                                                                               \
    retVal = palWriteRegister(hHdd, regAddr, regVal);                                                           \
    if (retVal != eHAL_STATUS_SUCCESS)                                                                          \
    {                                                                                                           \
        return (retVal);                                                                                        \
    }                                                                                                           \
    else if (pMac->hphy.phy.test.testLogPhyRegisters == eANI_BOOLEAN_TRUE)  /* for debug only */                \
    {                                                                                                           \
        switch (regAddr)                                                                                        \
        {   /*exceptions to logging first*/                                                                     \
            default:                                                                                            \
                phyLog(pMac, LOGE, "SET_PHY_REG:%s = %08X\n", GetLibraRegName(regAddr), regVal);                \
                break;                                                                                          \
        }                                                                                                       \
    }                                                                                                           \
    testAddRegToPhyWatchList(pMac, regAddr);                                                                    \
}                                                                                                               \
else                                                                                                            \
{                                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                                               \
}
#endif

#define SET_PHY_REG(hHdd, regAddr, regVal) \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                                          \
{                                                                                                               \
    retVal = palWriteRegister(hHdd, regAddr, regVal);                                                           \
    if (retVal != eHAL_STATUS_SUCCESS)                                                                          \
    {                                                                                                           \
        return (retVal);                                                                                        \
    }                                                                                                           \
}                                                                                                               \
else                                                                                                            \
{                                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                                               \
}


#if 0   //at this stage of development, we need the debug version of this to track phy register usage

#define GET_PHY_REG(hHdd, regAddr, pVal) \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                                          \
{                                                                                                               \
    retVal = palReadRegister(hHdd, regAddr, (tANI_U32 *)pVal);                                                  \
    if (retVal != eHAL_STATUS_SUCCESS)                                                                          \
    {                                                                                                           \
        return (retVal);                                                                                        \
    }                                                                                                           \
    else if (pMac->hphy.phy.test.testLogPhyRegisters == eANI_BOOLEAN_TRUE)  /* for debug only */                \
    {                                                                                                           \
        switch (regAddr)                                                                                        \
        {   /*exceptions to logging first*/                                                                     \
            default:                                                                                            \
                phyLog(pMac, LOGE, "GET_PHY_REG:%s = %08X\n", GetLibraRegName(regAddr), *(tANI_U32 *)pVal);      \
                break;                                                                                          \
        }                                                                                                       \
    }                                                                                                           \
}                                                                                                               \
else                                                                                                            \
{                                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                                               \
    *pVal = 0;                                                                                                  \
}
#endif

#define GET_PHY_REG(hHdd, regAddr, pVal) \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                                          \
{                                                                                                               \
    retVal = palReadRegister(hHdd, regAddr, (tANI_U32 *)pVal);                                                  \
    if (retVal != eHAL_STATUS_SUCCESS)                                                                          \
    {                                                                                                           \
        return (retVal);                                                                                        \
    }                                                                                                           \
}                                                                                                               \
else                                                                                                            \
{                                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                                               \
    *pVal = 0;                                                                                                  \
}



#define SET_PHY_MEMORY(hHdd, memOffset, pBuf, numWords)                                         \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                          \
{                                                                                               \
    retVal = palWriteRegMemory(hHdd, memOffset, (tANI_U8 *)pBuf, numWords * 4);                 \
    if (retVal != eHAL_STATUS_SUCCESS)                                                          \
    {                                                                                           \
        return (retVal);                                                                        \
    }                                                                                           \
}                                                                                               \
else                                                                                            \
{                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                               \
}


#define GET_PHY_MEMORY(hHdd, memOffset, pBuf, numWords)                                         \
if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)                          \
{                                                                                               \
    retVal = palReadRegMemory(hHdd, memOffset, (tANI_U8 *)pBuf, numWords * 4);                  \
    if (retVal != eHAL_STATUS_SUCCESS)                                                          \
        {                                                                                       \
            return (retVal);                                                                    \
        }                                                                                       \
}                                                                                               \
else                                                                                            \
{                                                                                               \
    retVal = eHAL_STATUS_SUCCESS;                                                               \
}


#ifdef ANI_PHY_DEBUG
#define DUMP_PHY_REG(addr)
#endif

/* TODO: we need to use memory access for efficiency's sake retVal = palReadDeviceMemory(hHdd, memOffset, pBuf, numBytes) */

/*
 *  Extends the sign bit of the number, Sign bit is located at 'nBits'
 */
static inline tANI_U32 SignExtend(tANI_U32 data, tANI_U8 nBits)
{
    tANI_U32 sign;

    sign = (data >> (nBits-1)) & 0x1;

    return (data | ~((sign << nBits) - 1));
}


/*
    EndianFlip flips the endianess of the given data's lower bits, up to the specified bitLimit.

    The flip occurs so that LSB becomes MSB, and MSB becomes LSB, for the number of bits desired.
    !It is expected that any bits beyond the specified bitLimit are 0.

    data - the data that we are flipping
    msbDataBitNumber - the highest data bit number(0-based), which would be swapped with BIT_0
*/
static inline tANI_U32 EndianFlip(tANI_U32 data, tANI_U8 msbDataBitNumber)
{
    tANI_U32 flipped = 0;
    tANI_U8 i;

    for (i = 1; i <= msbDataBitNumber; i++)
    {
        if (data & (BIT_0 << (msbDataBitNumber - i))) //test from highest to lowest bit
        {
            flipped |= (BIT_0 << (i - 1)); //flip from lowest to highest bit
        }
    }

    return(flipped);
}

static eHalStatus readModifyWriteReg( tpAniSirGlobal pMac, tANI_U32 reg, tANI_U32 andMask, tANI_U32 orMask )
{
    tANI_U32    regVal;
    eHalStatus  nStatus;

    if ((nStatus = halReadRegister(pMac, reg, &regVal)) == eHAL_STATUS_SUCCESS)
    {
        nStatus = halWriteRegister(pMac, reg, (regVal & andMask) | orMask);
    }

    return nStatus;
}

/*
    rdModWrAsicField perform a read-modify-write operation for use with registers that contain bits that need to change independently of others
    The regAddr, mask, and shift parameters come from hadware header file
    The val parameter is the bit values to put in the mask location of the register. This will be shifted to the mask location.
*/
static inline eHalStatus rdModWrAsicField(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 mask, tANI_U32 offset, tANI_U32 val)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)
    {
        retVal = readModifyWriteReg(pMac, regAddr, ~(mask), (val << offset));
    }
    return(retVal);
}


#ifdef VERIFY_HALPHY_SIMV_MODEL //To get rid of multiple definition error, in eazy way.
#define asicAGCReset                    host_asicAGCReset
#define asicOverrideAGCRxChainGain      host_asicOverrideAGCRxChainGain
#define asicSetAgcCCAMode               host_asicSetAgcCCAMode
#define asicCeaseOverrideAGCRxChainGain host_asicCeaseOverrideAGCRxChainGain
#define asicSetDisabledRxPacketTypes    host_asicSetDisabledRxPacketTypes
#define asicFftGetToneData              host_asicFftGetToneData 
#define phyAsicInit                     host_phyAsicInit
#define phyAsicSetFreq                  host_phyAsicSetFreq
#define phyAsicRfEnBand                 host_phyAsicRfEnBand
#define phyAsicRfEnChains               host_phyAsicRfEnChains
#define asicTPCPowerOverride            host_asicTPCPowerOverride
#define asicTPCAutomatic                host_asicTPCAutomatic
#define asicTPCGetADCReading            host_asicTPCGetADCReading
#define asicEnableTxDACs                host_asicEnableTxDACs
#define asicTxFirSetChainBypass         host_asicTxFirSetChainBypass
#define asicTxFirSetPaOverride          host_asicTxFirSetPaOverride
#define asicPhyDbgStartFrameGen         host_asicPhyDbgStartFrameGen
#define asicPhyDbgStopFrameGen          host_asicPhyDbgStopFrameGen
#define asicGrabAdcSamples              host_asicGrabAdcSamples
#define asicSetupTestWaveform           host_asicSetupTestWaveform
#define asicLoadTPCPowerLUT             host_asicLoadTPCPowerLUT
#define asicLoadTPCGainLUT              host_asicLoadTPCGainLUT
#define phySetTxPower                   host_phySetTxPower
#define phySetPowerLimit                host_phySetPowerLimit
#endif




eHalStatus phyAsicInit(tpAniSirGlobal pMac);
eHalStatus phyAsicSetFreq(tpAniSirGlobal pMac, tANI_U16 freq);
eHalStatus phyAsicRfEnBand(tpAniSirGlobal pMac, eRfSubBand band);
eHalStatus phyAsicRfEnChains(tpAniSirGlobal pMac, ePhyChainSelect txChains);

//asicCal functions
// eHalStatus asicGetPhyCalMode(tpAniSirGlobal pMac, eCalMode *mode);
// eHalStatus asicSetPhyCalMode(tpAniSirGlobal pMac, eCalMode mode);
// eHalStatus asicSetPhyCalLength(tpAniSirGlobal pMac, tANI_U16 numSamples);
// eHalStatus asicGetPhyCalLength( tpAniSirGlobal pMac, tANI_U32 *length);
// eHalStatus asicSetPhyCalDCOTimers(tpAniSirGlobal pMac);
// eHalStatus asicZeroFineDCOCorrection(tpAniSirGlobal pMac, ePhyRxChains rxChain);
// eHalStatus asicPerformCalMeasurement(tpAniSirGlobal pMac);
// eHalStatus asicInitCalMem(tpAniSirGlobal pMac);
#ifdef VERIFY_HALPHY_SIMV_MODEL
eHalStatus asicWriteRxPhaseCorrection(eGainSteps gain, ePhyRxChains rxChain, sIQCalValues iqCorrect);
eHalStatus asicReadRxPhaseCorrection(eGainSteps gain, ePhyRxChains rxChain, sIQCalValues *iqCorrect);
#else
eHalStatus asicWriteRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues iqCorrect);
eHalStatus asicReadRxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyRxChains rxChain, sIQCalValues *iqCorrect);
#endif
// eHalStatus asicGetCalADCSamples(tpAniSirGlobal pMac, ePhyRxChains rxChain, tIQAdc *dco);
// eHalStatus asicPrepareRxIQCal(tpAniSirGlobal pMac, ePhyRxChains rxChain);
// eHalStatus asicFinishIQCal(tpAniSirGlobal pMac);

//asicAGC functions
eHalStatus asicAGCHoldInReset(tpAniSirGlobal pMac);
eHalStatus asicAGCReset(tpAniSirGlobal pMac);
eHalStatus asicAGCInit(tpAniSirGlobal pMac);
eHalStatus asicAGCCalcGainLutsForChannel(tpAniSirGlobal pMac, eRfChannels chan);
eHalStatus asicSetAGCGainLut(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 minIndex, tANI_U8 maxIndex, const tRxGain *agcTable);
eHalStatus asicGetAgcGainLut(tpAniSirGlobal pMac, ePhyRxChains rxChain, tRxGain *agcTable /* NUM_AGC_GAINS */); 
eHalStatus asicSetAgcCCAMode(tpAniSirGlobal pMac, ePhyCCAMode primaryCcaMode, ePhyCCAMode secondaryCcaMode);
eHalStatus asicSetAGCChannelBondState(tpAniSirGlobal pMac, ePhyChanBondState chBondState);
eHalStatus asicOverrideAGCRxChainGain(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 gain);
eHalStatus asicCeaseOverrideAGCRxChainGain(tpAniSirGlobal pMac, ePhyRxChains rxChain);
eHalStatus asicOverrideAGCRxEnable(tpAniSirGlobal pMac, eAGCRxChainMask rxMask, eAGCRxMode rxMode);
eHalStatus asicSetDisabledRxPacketTypes(tpAniSirGlobal pMac, ePhyRxDisabledPktTypes modTypes);
eHalStatus asicGetDisabledRxPacketTypes(tpAniSirGlobal pMac, ePhyRxDisabledPktTypes *disabledTypes);
eHalStatus asicSetAGCRxChains(tpAniSirGlobal pMac, tANI_U8 numActiveRx, tANI_U8 numListenRx);
eHalStatus asicAGCSetDensity(tpAniSirGlobal pMac, tANI_BOOLEAN densityOn, ePhyNwDensity density20MHz, ePhyNwDensity density40MHz);

//asicAGCRadar functions
eHalStatus asicAgcInitRadar(tpAniSirGlobal pMac);


//asicTPC functions
eHalStatus asicTPCInit(tpAniSirGlobal pMac);
eHalStatus asicTPCPowerOverride(tpAniSirGlobal pMac, tTxGain tx0, tTxGain tx1, tTxGain tx2, tTxGain tx3);
eHalStatus asicTPCAutomatic(tpAniSirGlobal pMac);
eHalStatus asicLoadTPCPowerLUT(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *tpcPowerLUT);
eHalStatus asicLoadTPCGainLUT(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain *tpcGainLut);
eHalStatus asicGetTxGainAtIndex(tpAniSirGlobal pMac, ePhyTxChains txChain, tPwrTemplateIndex index, tTxGainCombo *retGain);
eHalStatus asicGetTxPowerLutAtIndex(tpAniSirGlobal pMac, ePhyTxChains txChain, tPowerDetect adcIndex, tPowerDetect *retPwr);
eHalStatus asicGetTxPowerMeasurement(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U8 *retAdc);
eHalStatus asicTPCGetADCReading(tpAniSirGlobal pMac, tANI_U8 *pADC);


//asicTXCTL functions
eHalStatus asicEnableTxDACs(tpAniSirGlobal pMac, ePhyTxChains txChainsOff, tANI_BOOLEAN override, tANI_BOOLEAN wfm);

//asicTxFir functions
// eHalStatus asicTxFirInit(tpAniSirGlobal pMac);
// eHalStatus asicTxFirInitMem(tpAniSirGlobal pMac);
// eHalStatus asicTxFirAdjustCarrierCorrection(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxCarrierError error, tIQAdc *correction);
eHalStatus asicTxFirSetChainBypass(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_BOOLEAN chainBypassEnable);
eHalStatus asicTxFirSetPaOverride(tpAniSirGlobal pMac, tANI_BOOLEAN overrideEnable, ePhyTxChains chainPaEnables);
// eHalStatus asicTxFirSetLoLeakageBypass(tpAniSirGlobal pMac, tANI_BOOLEAN bypassEnable);
// eHalStatus asicTxFirSetIqImbalanceBypass(tpAniSirGlobal pMac, tANI_BOOLEAN bypassEnable);
// eHalStatus asicTxFirSetLoLeakageGainShift(tpAniSirGlobal pMac, ePhyTxChains txChain, tANI_U32 shiftVal2Bits);
// eHalStatus asicTxFirSetPostIqCalGainAdjust(tpAniSirGlobal pMac, tANI_U32 ch0Adjust3bits, tANI_U32 ch1Adjust3bits);
#ifdef VERIFY_HALPHY_SIMV_MODEL
void asicTxFirSetTxCarrierCorrection(eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect correct);
void asicTxFirGetTxCarrierCorrection(eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect *correct);
void asicTxFirSetTxPhaseCorrection(eGainSteps gain, ePhyTxChains txChain, sIQCalValues correct);
void asicTxFirGetTxPhaseCorrection(eGainSteps gain, ePhyTxChains txChain, sIQCalValues *correct);
#else
eHalStatus asicTxFirSetTxCarrierCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect correct);
eHalStatus asicTxFirGetTxCarrierCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sTxFirLoCorrect *correct);
eHalStatus asicTxFirSetTxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sIQCalValues correct);
eHalStatus asicTxFirGetTxPhaseCorrection(tpAniSirGlobal pMac, eGainSteps gain, ePhyTxChains txChain, sIQCalValues *correct);
#endif
// eHalStatus asicTxFirApplyCoef(tpAniSirGlobal pMac, eRfChannels channel, eRegDomainId regDomain);


//asicFft functions
eHalStatus asicFftGetToneData(tpAniSirGlobal pMac, ePhyRxChains rxChain, tANI_U8 toneIndex, sTwoToneData *toneData);



//asicPMU.c
// void asicPMUInit(tpAniSirGlobal pMac);
// void asicPMURadioControl(tpAniSirGlobal pMac, ePMURadioControl radio);
// void asicSetPMUManualEnMode(tpAniSirGlobal pMac, ePMUEnableStatus pmuMode);
// tANI_BOOLEAN asicSetPowerDisable(tpAniSirGlobal pMac, tANI_BOOLEAN onOff);

//asicWfm.c
eHalStatus asicSetupTestWaveform(tpAniSirGlobal pMac, const tWaveformSample *pWave, tANI_U16 numSamples, tANI_BOOLEAN clk80);
#ifdef VERIFY_HALPHY_SIMV_MODEL
void asicStartTestWaveform(eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex);
void asicStopTestWaveform();
#else
eHalStatus asicStartTestWaveform(tpAniSirGlobal pMac, eWaveMode playback, tANI_U32 startIndex, tANI_U32 endIndex);
eHalStatus asicStopTestWaveform(tpAniSirGlobal pMac);
#endif


#ifdef ANI_MANF_DIAG
//asicPhyDbg.c
eHalStatus asicPhyDbgStartFrameGen(tpAniSirGlobal pMac, 
                                   eHalPhyRates rate, 
                                   tANI_U16 payloadLength, 
                                   ePayloadContents payloadContents, 
                                   tANI_U8 payloadFillByte,
                                   tANI_U8 *payload, 
                                   tANI_U32 numTestPackets, 
                                   tANI_U32 interFrameSpace, 
                                   tANI_BOOLEAN pktAutoSeqNum,
                                   tANI_U8 pktScramblerSeed,
                                   ePhyDbgPreamble preamble,
                                   tANI_U8 *addr1, tANI_U8 *addr2, tANI_U8 *addr3,
                                   tANI_BOOLEAN crc);
eHalStatus asicPhyDbgStopFrameGen(tpAniSirGlobal pMac);
eHalStatus asicPhyDbgQueryStatus(tpAniSirGlobal pMac, sTxFrameCounters *numFrames, ePhyDbgTxStatus *status);
eHalStatus asicGrabAdcSamples(tpAniSirGlobal pMac, tANI_U32 startSample, tANI_U32 numSamples, eGrabRamSampleType sampleType, tGrabRamSample *sampleBuffer);


#endif



#endif /* ASICAPI_H */
