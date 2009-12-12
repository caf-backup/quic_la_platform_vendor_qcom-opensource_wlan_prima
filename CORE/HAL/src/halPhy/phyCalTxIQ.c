/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file phyCalTxIQ.c

    \brief Tx IQ calibration

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include "string.h"
#include "sys_api.h"


eHalStatus phyInitTxIQCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U32 gain;
    
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetupTestWaveform(pMac, calWaveform, NUM_CAL_WFM_SAMPLES, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    //initialization for tx0 IQ cal
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_ON, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_RX_1, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    /* txCalClipDetect */
    {
        tANI_U8 rfDetGain = 5;     //5 is the max value of 30dBm
        tANI_U8 rxGainIndex = MAX_QUASAR_RX_GAIN_STEP;
        tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR};

        if ((retVal = phyTxIqClipDetect(pMac, PHY_TX_CHAIN_0, txGain, PHY_RX_CHAIN_1, &rxGainIndex, &rfDetGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    //loop through all gains for txChain 0
    for (gain = 0; gain < NUM_TX_GAIN_STEPS; gain ++)
    {
        if ((retVal = phyTxIQCal(pMac, PHY_TX_CHAIN_0, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }


    //initialization for tx1 IQ cal
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_RX_0, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    {
        tANI_U8 rfDetGain = 5;     //5 is the max value of 30dBm
        tANI_U8 rxGainIndex = MAX_QUASAR_RX_GAIN_STEP;
        tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR};

        if ((retVal = phyTxIqClipDetect(pMac, PHY_TX_CHAIN_1, txGain, PHY_RX_CHAIN_0, &rxGainIndex, &rfDetGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    //loop through all gains for txChain 1
    for (gain = 0; gain < NUM_TX_GAIN_STEPS; gain ++)
    {
        if ((retVal = phyTxIQCal(pMac, PHY_TX_CHAIN_1, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_OFF, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_ON_AUTO)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    return (retVal);

}


eHalStatus phyPeriodicIQCalTxChain(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;


    return (retVal);

}



eHalStatus phyTxIQCal(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tIQAdc txAmpA, txAmpB, txPhaseA8, txPhaseB8;
    sIQCalValues correct;
    tTxGain otherGain = { MIN_TPC_COARSE_TXPWR, MIN_TPC_FINE_TXPWR };
    tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR };

    assert(gain < NUM_TX_GAIN_STEPS);

    //assumes the calWaveform has been setup
    //assumes the proper tx & rx chains are enabled

    //set the transmit gain
    txGain.coarsePwr = gain;
    if (txChain == PHY_TX_CHAIN_0)
    {
        if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }
    else if (txChain == PHY_TX_CHAIN_1)
    {
        if ((retVal = asicTPCPowerOverride(pMac, otherGain, txGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    //start with current correction values
    if ((retVal = asicTxFirGetTxPhaseCorrection(pMac, gain, txChain, &correct)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    do
    {
        if ((retVal = asicTxFirSetTxPhaseCorrection(pMac, gain, PHY_TX_CHAIN_0, correct)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, calWfmIndices[CAL_WFM_TX_AMP_A].start, calWfmIndices[CAL_WFM_TX_AMP_A].end)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &txAmpA)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, calWfmIndices[CAL_WFM_TX_AMP_B].start, calWfmIndices[CAL_WFM_TX_AMP_B].end)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &txAmpB)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, calWfmIndices[CAL_WFM_TX_PHASE_TONE_A8].start, calWfmIndices[CAL_WFM_TX_PHASE_TONE_A8].end)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &txPhaseA8)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, calWfmIndices[CAL_WFM_TX_PHASE_TONE_B8].start, calWfmIndices[CAL_WFM_TX_PHASE_TONE_B8].end)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_1, &txPhaseB8)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        //TODO: error = ? from Q samples
        
        //adjust correct values for measured error
        
    }while (1 /* TODO: error > tolerance */);
    

    return (retVal);
}


#define DET_OFFSET_FOR_TXIQ_CLIP_DET    0

eHalStatus phyTxIqClipDetect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain, ePhyRxChains rxChain, tANI_U8 *rxGainIndex, tANI_U8 *rfDetGain)
{
    eHalStatus retVal;
    tTxGain otherGain = { MIN_TPC_COARSE_TXPWR, MIN_TPC_FINE_TXPWR };
    tANI_U32 clipReg = 0;
    sAgcGainLut rxGains;

    //assumes this to be used while a continuous waveform is being generated
    //assumes that the correct transmitters & receivers are enabled

    assert(*rxGainIndex < NUM_QUASAR_RX_GAIN_STEPS);
    assert(rxChain < PHY_MAX_RX_CHAINS);
    assert(txChain < PHY_MAX_TX_CHAINS);

    //We need to get the current AGC gain Lut in order to see where the TR & LNA bits change
    //We only want to use the R + LNA On portion of the gain Lut. 
    //Once we go below that, there is no point in continuing.
    if ((retVal = asicGetAgcGainLut(pMac, rxChain, rxGains)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    //start by initializing Quasar detector to initial gain
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_OFFSET, DET_OFFSET_FOR_TXIQ_CLIP_DET)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, *rfDetGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxChainGain(pMac, rxChain, *rxGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    switch (txChain)
    {
        case PHY_TX_CHAIN_0:
            if ((retVal = asicTPCPowerOverride(pMac, txGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            break;

        case PHY_TX_CHAIN_1:
            if ((retVal = asicTPCPowerOverride(pMac, otherGain, txGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            break;

        case PHY_ALL_TX_CHAINS:
            if ((retVal = asicTPCPowerOverride(pMac, txGain, txGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            break;

        default:
            assert(0);
            return (eHAL_STATUS_FAILURE);
            break;
    }

    //tx gains set to desired level
    //now loop from startRxGainIndex downward until the clip detection is not triggered
    do
    {
        SET_PHY_REG(pMac->hHdd, AGC_CLIP_COUNT_REG, 0);    //reset counter
        sirBusyWait(50000);  //wait 50 microseconds
        GET_PHY_REG(pMac->hHdd, AGC_CLIP_COUNT_REG, &clipReg);

        if (clipReg != 0)
        {
            if (*rfDetGain > 0)
            {
                //decrement the detector gain in 6dB steps
                *rfDetGain--;
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_DET_GAIN, *rfDetGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            }
            else if (*rxGainIndex > 0)
            {
                *rxGainIndex--;
                
                //make sure we are still in the correct range of LUT settings
                if ((rxGains[*rxGainIndex].swEn & (T_SW_EN_MSK | LNA_SW_EN_MSK)) == (LNA_SW_EN_MSK))
                {
                    if ((retVal = asicOverrideAGCRxChainGain(pMac, rxChain, *rxGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                }
                else
                {
                    //can't adjust any further downward
                    assert(0);
                    phyLog(pMac, LOGE, "ERROR: Unable to reduce gain sufficiently to avoid clipping of ADCs\n");
                    return (eHAL_STATUS_FAILURE);   //still clipping
                }
            }
            else
            {
                //can't adjust any further downward
                assert(0);
                phyLog(pMac, LOGE, "ERROR: Unable to reduce gain sufficiently to avoid clipping of ADCs\n");
                return (eHAL_STATUS_FAILURE);   //still clipping
             }
        }
    }while (clipReg != 0);

    return (retVal);
}


