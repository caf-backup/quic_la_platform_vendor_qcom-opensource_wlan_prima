/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyCalService.cc: Calibration functionality extending from Baseband blocks to RF
   Author:  Mark Nelson
   Date:    3/10/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */
#include <string.h>
#include "ani_assert.h"
#include "sys_api.h"


#define DRV_INITIAL_DCO_CAL_LENGTH  1023
#define DRV_PERIODIC_DCO_CAL_LENGTH 255

#define CAL_GAIN_INDEX  QUASAR_GAIN_MAX_INDEX
extern const tANI_U8 agcQuasarGains[NUM_QUASAR_RX_GAIN_STEPS];

const tANI_U16 tenPowFracValues[10] =
{
    100,
    125,
    158,
    199,
    251,
    316,
    398,
    501,
    630,
    794
};

static tANI_U16 inline power(tANI_U16 x, tANI_U16 y)
{
    tANI_U16 i = 0, result = 1;

    for (i = 0; i < y; i++)
    {
        result *= x;
    }

    return result;
}

eHalStatus phyInitRxDcoCal(tpAniSirGlobal pMac)
{
    eHalStatus retVal;
    ePhyRxChains rxChain;
    tANI_U32 gain;
    tANI_U8 calGainIndex = CAL_GAIN_INDEX;
    tANI_U32 clipThresholds[2];
    
    
    // pMac->hphy.phy.test.testLogPhyRegisters = eANI_BOOLEAN_TRUE;
    // pMac->hphy.phy.test.testLogSpiRegs = eANI_BOOLEAN_TRUE;
    
    //GET_PHY_REG(pMac->hHdd, AGC_INIT_GAIN_REG, &calGainIndex);

    if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_ON, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    // if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX0_DCOC_EN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    // if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX1_DCOC_EN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    // if ((retVal = rfWriteQuasarField(pMac, QUASAR_FIELD_RX2_DCOC_EN, 1)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    //if ((retVal = asicSetPhyCalLength(pMac, DRV_INITIAL_DCO_CAL_LENGTH)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }


    //done in phyDcoCalRxChain if ((retVal = asicOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    clipThresholds[0] = (1024 + 512);
    clipThresholds[1] = (1024 - 512);
    SET_PHY_MEMORY(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, clipThresholds, 2);
    
    for (rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
    {
        phyLog(pMac, LOG1, "Performing Initial DCO cal on RX%d\n", rxChain);
        
        //zero corrections for initial cal
        if ((retVal = asicZeroFineDCOCorrection(pMac, rxChain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        {
            tRxDcoCorrect dco = { 32, 32, 1 };    //set starting point DCO values for initialization cal - I, Q, Range=1
            
            if ((retVal = rfSetDCOffset(pMac, rxChain, calGainIndex, dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            if ((retVal = phyDcoCalRxChain(pMac, rxChain, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            //now plug resultant dco at max gain into all other dco gain settings
            if ((retVal = rfGetDCOffset(pMac, rxChain, calGainIndex, &dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            
            for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
            {
                if ((retVal = rfSetDCOffset(pMac, rxChain, gain, dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            }
        }
    }

    if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_NO_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    clipThresholds[0] = AGC_TH_CLIP_HIGH_THRESHOLD_DEFAULT;
    clipThresholds[1] = AGC_TH_CLIP_LOW_THRESHOLD_DEFAULT;
    SET_PHY_MEMORY(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, clipThresholds, 2);

    if ((retVal = phyForceResidualDCOMeasurement(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    //if ((retVal = asicSetPhyCalLength(pMac, DRV_PERIODIC_DCO_CAL_LENGTH)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicCeaseOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    //if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_ON_AUTO)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    //if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_OFF, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    
    // pMac->hphy.phy.test.testLogPhyRegisters = eANI_BOOLEAN_FALSE;
    // pMac->hphy.phy.test.testLogSpiRegs = eANI_BOOLEAN_FALSE;
    return (retVal);
}

//assumes that Phy Cal block has been placed into Init DCO Cal mode
//assumes that the number of ADC samples has been set in the Cal block
eHalStatus phyDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain)
{
    eHalStatus retVal;
    tANI_U32 iteration = 0;
    tIQAdc dcoError;
    tRxGain curVal;
    tANI_U32 lnaEnables;
    tIQAdc minError = { MSK_10, MSK_10 };   //init to max error values
    tANI_S8 minICorrect = 0;
    tANI_S8 minQCorrect = 0;
    tRxGain tempGainVal = { agcQuasarGains[QUASAR_GAIN_MAX_INDEX], LNA_SW_EN_MSK };
    eRfSubBand  currBand;
    tANI_U8 bbRxGain;
    tANI_U32 bbGain;

    assert((rxChain == PHY_RX_CHAIN_0) ||
           (rxChain == PHY_RX_CHAIN_1) ||
           (rxChain == PHY_RX_CHAIN_2)
          );
    currBand = rfGetAGBand(pMac);

    pMac->hphy.phy.calTable[currBand]->useDcoCorrection = eANI_BOOLEAN_FALSE;
    
    //set gain for this calibration
    if ((retVal = asicOverrideAGCRxChainGain(pMac, rxChain, gain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    // take DC offset error measurements as tANI_S16 values
    if ((retVal = asicGetCalADCSamples(pMac, rxChain, &dcoError)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    {
        //temporarily flip TR switch to T for this chain & gain indes - provides better front end isolation
        asicGetAgcGainIndex(pMac, rxChain, (tANI_U8)gain, &curVal);
        // memcpy(&tempGainVal, &curVal, sizeof(tRxGain));
        tempGainVal.swEn |= T_SW_EN_MSK;
#ifdef ANI_BUS_TYPE_USB
        tempGainVal.swEn &= (~LNA_SW_EN_MSK); //R+LNA_OFF because there is an external LNA controlled independently through GPIO
#endif        
        asicSetAGCGainLut(pMac, rxChain, (tANI_U8)gain, (tANI_U8)gain, (const tRxGain *)&tempGainVal);
#ifdef FIXME_GEN5
        GET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, &lnaEnables);
        SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, 0);
#endif		
    }
    
    {        
        bbRxGain = (tANI_U8)(6 + 2*( agcQuasarGains[MAX_QUASAR_RX_GAIN_STEP] & 15)); // assuming it is same across chains for now 
        bbGain = (power(10, bbRxGain/20) * tenPowFracValues[((bbRxGain % 20)*10)/20])/100;
        pMac->hphy.rf.dcoStep = (1042*bbGain*2)/1000;
    }

    //Calibrate DCO for I & Q rails
    while (iteration++ < DCO_CORRECTION_RANGE)
    {
        tANI_S16 IMag = GET_MAG(dcoError.I);
        tANI_S16 QMag = GET_MAG(dcoError.Q);

        phyLog(pMac, LOG1, "gain=%d iter=%d   phy RX%d DCO_ERR:I=%d Q=%d\n", gain, iteration, rxChain, dcoError.I, dcoError.Q);

        //check to see if any rail is still out of tolerance
        if ((IMag <= DCO_ERROR_TOLERANCE) &&
            (QMag <= DCO_ERROR_TOLERANCE)
           )
        {
            if (iteration == 1) 
            {
                tRxDcoCorrect curOffset;
                rfGetDCOffset(pMac, rxChain, gain, &curOffset);
                minICorrect = curOffset.IDcoCorrect;    
                minQCorrect = curOffset.QDcoCorrect;
            }
            break;  //tolerances met, exit loop
        }

        {
            tRxDcoCorrect curOffset;
            rfGetDCOffset(pMac, rxChain, gain, &curOffset);

            if (IMag < GET_MAG(minError.I))
            {
                minError.I = dcoError.I;
                minICorrect = curOffset.IDcoCorrect;
            }
        
            if (QMag < GET_MAG(minError.Q))
            {
                minError.Q = dcoError.Q;
                minQCorrect = curOffset.QDcoCorrect;
            }
        }
        
        
        //this applies corrections based on input tANI_S16 dcoError
        if ((retVal = rfAdjustDCO(pMac, rxChain, gain, dcoError)) != eHAL_STATUS_SUCCESS) { return (retVal); }

        // Wait for minimum of 10 us; Settling time for RF gain setting
        // Actually 1 us is enough; for performance we will tweak it later
        sirBusyWait(10000);

        // take DC offset error measurements
        if ((retVal = asicGetCalADCSamples(pMac, rxChain, &dcoError)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }

    //now restore prior chain gain and lnas
    asicSetAGCGainLut(pMac, rxChain, (tANI_U8)gain, (tANI_U8)gain, (const tRxGain *)&curVal);
#ifdef FIXME_GEN5
    SET_PHY_REG(pMac->hHdd, GPIO_RFIF_EN_REG, lnaEnables);
#endif

    if (iteration >= DCO_CORRECTION_RANGE)
    {
        phyLog(pMac, LOGE, "DCO CAL FAILED TO CONVERGE");
        {
            tRxDcoCorrect setOffset;
            
            setOffset.IDcoCorrect = minICorrect;
            setOffset.QDcoCorrect = minQCorrect;
            setOffset.dcRange = 1;
            
            phyLog(pMac, LOGE, "Loading minError values: I=%d at error=%d  Q=%d at error %d\n",
                                minICorrect, minError.I, minQCorrect, minError.Q
                  );
            rfSetDCOffset(pMac, rxChain, gain, setOffset);
        }
    }
    else
    {
       //store cal values for this band for use on all channels
        pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].IDcoCorrect = (tDcoCorrect)minICorrect;
        pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].QDcoCorrect = (tDcoCorrect)minQCorrect;
        pMac->hphy.phy.calTable[currBand]->dcoCorrection[rxChain].dcRange = 1;

        pMac->hphy.phy.calTable[currBand]->useDcoCorrection = eANI_BOOLEAN_TRUE;
    }
    
    return (retVal);
}


eHalStatus phyForceResidualDCOMeasurement(tpAniSirGlobal pMac)
{
    eHalStatus retVal;

    //Set Cal mode to Residual DCO and take measurement to correct for any residual DCO that might affect the IQ cal
    if (halPhyGetChannelBondState(pMac) == PHY_SINGLE_CHANNEL_CENTERED)
    {
        //non-channel bonded
        if ((retVal = asicSetPhyCalLength(pMac, 0x0F)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }
    else
    {
        //channel bonded
        if ((retVal = asicSetPhyCalLength(pMac, 0x1F)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    }


    if ((retVal = asicSetPhyCalDCOTimers(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_RESDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    if ((retVal = asicPerformCalMeasurement(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    return (retVal);
}





eHalStatus phyPeriodicDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain)
{
    eHalStatus retVal;
    //ePhyRxChains rxChain;
    //tANI_U32 gain;
    tANI_U8 calGainIndex = CAL_GAIN_INDEX;
    

    if ((retVal = asicSetTxDACs(pMac, PHY_ALL_TX_CHAINS, eANI_BOOLEAN_ON, eANI_BOOLEAN_OFF)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_CALIBRATING)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_INITDCCAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, (1024 + 512));
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, (1024 - 512));

    if (rxChain == PHY_MAX_RX_CHAINS || rxChain == PHY_ALL_RX_CHAINS)
    {
        for (rxChain = 0; rxChain < PHY_MAX_RX_CHAINS; rxChain++)
        {
            phyLog(pMac, LOG1, "Performing Periodic DCO cal on RX%d\n", rxChain);

            //zero corrections for initial cal
            if ((retVal = asicZeroFineDCOCorrection(pMac, rxChain)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            {
                tRxDcoCorrect dco;

                if ((retVal = phyDcoCalRxChain(pMac, rxChain, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                //now plug resultant dco at max gain into all other dco gain settings
                if ((retVal = rfGetDCOffset(pMac, rxChain, calGainIndex, &dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                phyLog(pMac, LOG3, "rxChain: %d, dco.IDcoCorrect: %d, dco.QDcoCorrect: %d, dco.dcRange: %d\n", rxChain, dco.IDcoCorrect, dco.QDcoCorrect, dco.dcRange);

                for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
                {
                    if ((retVal = rfSetDCOffset(pMac, rxChain, gain, dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
                }
            }
        }

    }
    else if (rxChain == PHY_RX_CHAIN_0 || rxChain == PHY_RX_CHAIN_1 || rxChain == PHY_RX_CHAIN_2)
    {
        if ((retVal = asicZeroFineDCOCorrection(pMac, rxChain)) != eHAL_STATUS_SUCCESS) { return (retVal); }
        {
            tRxDcoCorrect dco;

            if ((retVal = phyDcoCalRxChain(pMac, rxChain, calGainIndex)) != eHAL_STATUS_SUCCESS) { return (retVal); }

            //now plug resultant dco at max gain into all other dco gain settings
            if ((retVal = rfGetDCOffset(pMac, rxChain, calGainIndex, &dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            phyLog(pMac, LOG3, "rxChain: %d, dco.IDcoCorrect: %d, dco.QDcoCorrect: %d, dco.dcRange: %d\n", rxChain, dco.IDcoCorrect, dco.QDcoCorrect, dco.dcRange);

            for (gain = 0; gain < NUM_QUASAR_RX_GAIN_STEPS; gain++)
            {
                if ((retVal = rfSetDCOffset(pMac, rxChain, gain, dco)) != eHAL_STATUS_SUCCESS) { return (retVal); }
            }
        }
    }
    else
    {
        phyLog(pMac, LOGE, "Invalid Rx Chain \n");
        retVal = eHAL_STATUS_FAILURE;
    }

    if ((retVal = asicZeroFineDCOCorrection(pMac, PHY_NO_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_HIGH_REG, AGC_TH_CLIP_HIGH_THRESHOLD_DEFAULT);
    SET_PHY_REG(pMac->hHdd, AGC_TH_CLIP_LOW_REG, AGC_TH_CLIP_LOW_THRESHOLD_DEFAULT);

    if ((retVal = phyForceResidualDCOMeasurement(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL)) != eHAL_STATUS_SUCCESS) { return (retVal); }
    if ((retVal = asicCeaseOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS)) != eHAL_STATUS_SUCCESS) { return (retVal); }

    return (retVal);
}
