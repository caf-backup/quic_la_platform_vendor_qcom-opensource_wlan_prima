/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file phyCalRxIQ.c

    \brief Rx IQ calibration

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#include "string.h"
#include "sys_api.h"


eHalStatus phyInitRxIQCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    eGainSteps gain;

    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_IQCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetupTestWaveform(pMac, calWaveform, NUM_CAL_WFM_SAMPLES, eANI_BOOLEAN_TRUE)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    //initialization for rx0 IQ cal
    //overrides DAC 0 disabled, DAC 1 enabled for waveform output
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_0, eANI_BOOLEAN_ON, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_RX_0, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicPrepareRxIQCal(pMac, PHY_RX_CHAIN_0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicStartTestWaveform(pMac, WAVE_CONTINUOUS, calWfmIndices[CAL_WFM_RX_TONE_8].start, calWfmIndices[CAL_WFM_RX_TONE_8].end)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    //loop through all gains for rxChain 0
    for (gain = 0; gain < NUM_RX_GAIN_STEPS; gain ++)
    {
        if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_0, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        {
            tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR};
            eQuasarLoopbackGain lbGains = QUASAR_LB_GAIN_0DB;
            if ((retVal = phyRxIqClipDetect(pMac, PHY_TX_CHAIN_1, &txGain, &lbGains)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }

        if ((retVal = phyRxIQCal(pMac, PHY_RX_CHAIN_0, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }


    //initialization for rx1 IQ cal
    //overrides DAC 1 disabled, DAC 0 enabled for waveform output
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_ON, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_RX_1, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicPrepareRxIQCal(pMac, PHY_RX_CHAIN_1)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    //loop through all gains for rxChain 1
    for (gain = 0; gain < NUM_RX_GAIN_STEPS; gain ++)
    {
        if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_1, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        {
            tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR};
            eQuasarLoopbackGain lbGains = QUASAR_LB_GAIN_0DB;
            if ((retVal = phyRxIqClipDetect(pMac, PHY_TX_CHAIN_0, &txGain, &lbGains)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }

        if ((retVal = phyRxIQCal(pMac, PHY_RX_CHAIN_1, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }


    //initialization for rx2 IQ cal
    //overrides DAC 1 disabled, DAC 0 enabled for waveform output
    if ((retVal = asicSetTxDACs(pMac, PHY_TX_CHAIN_1, eANI_BOOLEAN_ON, eANI_BOOLEAN_ON)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_RX_2, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicPrepareRxIQCal(pMac, PHY_RX_CHAIN_2)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    //loop through all gains for rxChain 2
    for (gain = 0; gain < NUM_RX_GAIN_STEPS; gain ++)
    {
        if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_RX_CHAIN_2, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        {
            tTxGain txGain = { MAX_TPC_COARSE_TXPWR, MAX_TPC_FINE_TXPWR};
            eQuasarLoopbackGain lbGains = QUASAR_LB_GAIN_0DB;
            if ((retVal = phyRxIqClipDetect(pMac, PHY_TX_CHAIN_0, &txGain, &lbGains)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        }

        if ((retVal = phyRxIQCal(pMac, PHY_RX_CHAIN_2, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    if ((retVal = asicStopTestWaveform(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetTxDACs(pMac, PHY_NO_TX_CHAINS, eANI_BOOLEAN_OFF, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_ON_AUTO)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    return (retVal);

}


eHalStatus phyPeriodicIQCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;


    return (retVal);

}



eHalStatus phyRxIQCal(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    sIQCalValues correct;
    tIQAdc rxTone8;

    //assumes tx & rx chains are enabled properly
    //assumes the waveform is playing
    //assumes the tx & rx gain is set correctly
    //assumes we are in the correct loopback and cal modes

    //start with current correction values
    if ((retVal = asicReadRxPhaseCorrection(pMac, gain, rxChain, &correct)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    do
    {
        if ((retVal = asicWriteRxPhaseCorrection(pMac, gain, rxChain, correct)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        //any delay here?
        if ((retVal = asicGetCalADCSamples(pMac, PHY_RX_CHAIN_0, &rxTone8)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        //TODO: error = ?
        //adjust correct values for measured error
    }while (1 /*TODO: error > tolerance */);

    return (retVal);
}



eHalStatus phyRxIqClipDetect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain *txGain, eQuasarLoopbackGain *lbGains)
{
    eHalStatus retVal;
    tANI_U32 clipReg = 0;
    tTxGain otherGain = { MIN_TPC_COARSE_TXPWR, MIN_TPC_FINE_TXPWR };
    tTxGain changingGain;
    tANI_U32 txLbGain, rxLbGain;   //init to some invalid value

    //assumes this to be used while a continuous waveform is being generated
    //assumes the proper loopback mode is selected in Quasar
    //assumes that the rx chain and txChains have been enabled and that the rx gain has been set

    switch (*lbGains)
    {
        case QUASAR_LB_GAIN_ALL_LO:
            txLbGain = 0;
            rxLbGain = 0;
            break;
        case QUASAR_LB_GAIN_RX_LO:
            txLbGain = 1;
            rxLbGain = 0;
            break;
        case QUASAR_LB_GAIN_TX_LO:
            txLbGain = 0;
            rxLbGain = 1;
            break;
        case QUASAR_LB_GAIN_0DB:
        default:
            txLbGain = 1;
            rxLbGain = 1;
            break;
    }

    changingGain.coarsePwr = txGain->coarsePwr;
    changingGain.finePwr = MAX_TPC_FINE_TXPWR;

    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, txLbGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, rxLbGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicTPCPowerOverride(pMac, changingGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    do
    {
        SET_PHY_REG(pMac->hHdd, AGC_CLIP_COUNT_REG, 0);    //reset counter

        sirBusyWait(50000);  //wait 50 microseconds

        GET_PHY_REG(pMac->hHdd, AGC_CLIP_COUNT_REG, &clipReg);

        if (clipReg != 0)
        {
            //still clipping
            if (txLbGain != 0)
            {
                //set -10dB pad in Quasar
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_TX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                {
                    txLbGain = 0;
                    *lbGains = QUASAR_LB_GAIN_TX_LO;
                }
            }
            else if (rxLbGain == 1)
            {
                //set -10dB pad in Quasar
                if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX_LB_GAIN, 0)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                {
                    rxLbGain = 0;
                    *lbGains = QUASAR_LB_GAIN_ALL_LO;
                }
            }
            else if ((tANI_U32)changingGain.coarsePwr > MIN_TPC_COARSE_TXPWR)
            {
                //decrement coarsePwr gain step
                changingGain.coarsePwr = (eTxCoarseGain)((tANI_U32)changingGain.coarsePwr - 1);

                switch (txChain)
                {
                    case PHY_TX_CHAIN_0:
                        if ((retVal = asicTPCPowerOverride(pMac, changingGain, otherGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                        break;

                    case PHY_TX_CHAIN_1:
                        if ((retVal = asicTPCPowerOverride(pMac, otherGain, changingGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                        break;

                    case PHY_ALL_TX_CHAINS:
                        if ((retVal = asicTPCPowerOverride(pMac, changingGain, changingGain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                        break;

                    default:
                        assert(0);
                        return (eHAL_STATUS_FAILURE);
                        break;
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
        else
        {
            //no longer clipping
            txGain->coarsePwr = changingGain.coarsePwr;
            txGain->finePwr = changingGain.finePwr;
            break;
        }
    }while (clipReg != 0);





    return (retVal);
}
