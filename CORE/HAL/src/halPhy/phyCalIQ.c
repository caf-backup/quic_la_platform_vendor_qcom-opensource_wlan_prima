/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyIQCal.cc: Calibration algorithm for RX IQ amp/phase imbalance
   Author:  Mark Nelson
   Date:    3/12/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#include "ani_assert.h"
#include "sys_api.h"

#define NUM_CAL_IQ_SAMPLES 1023         //must be >= 64

#define NUM_CAL_TONES 14
const tANI_S8 calTones[NUM_CAL_TONES] = { -28, -24, -20, -16, -12, -8, -4, 4, 8, 12, 16, 20, 24, 28 };

static tANI_S16 ComputePhase(tpAniSirGlobal pMac, tToneResponse phaseMeas);
static tANI_U8 checkClippedS16( tANI_S16 x );
static tANI_U8 checkClippedComplexInteger( tComplexInteger x );
static tANI_U8 checkClippedPhaseMeasurement(tToneResponse phaseMeas);

void phyIQCal(tpAniSirGlobal pMac)
{

     phyDcoCal(pMac, MAX_AGC_GAIN);       // DC Offset Calibration

    {
        ePhyChainSelect activeChains = halPhyGetActiveChainSelect(pMac);

        switch (activeChains)
        {

            case PHY_CHAIN_SEL_NO_RX_TX:
                break;
            case PHY_CHAIN_SEL_R0_T0_ON:
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_0);
                break;
            case PHY_CHAIN_SEL_R0R1_T0_ON: //1x2
                 phyIQCalRxChain(pMac, PHY_RX_CHAIN_0);
                 phyIQCalRxChain(pMac, PHY_RX_CHAIN_1);
                 break;
            case PHY_CHAIN_SEL_R0R1_T0T1_ON:
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_0);
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_1);
                break;
            case PHY_CHAIN_SEL_R0R1R2_T0T1_ON:
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_0);
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_1);    //Mars chain is not correctly calibrating - don't do this one
                phyIQCalRxChain(pMac, PHY_RX_CHAIN_2);
                break;
            default:
                assert(0);
                break;
        }
    }

}

void phyIQCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain)
{
    // state variables to be restored upon completion
    ePhyRxDisabledPktTypes agcDisModeState = asicGetDisabledRxPacketTypes(pMac);

    tANI_S8 tone;
    tANI_S16 phaseCorrAccum = 0;
    tANI_S8 phaseCorrection;
    tANI_S16 ampCorrection = 0;    
    tANI_U16 calLength = asicGetPhyCalLength( pMac );
    tANI_U32 cbState;
    ePhyChainSelect currentChains = halPhyGetActiveChainSelect(pMac);

    if (halPhyGetChannelBondState(pMac) != PHY_SINGLE_CHANNEL_CENTERED)
    {
        cbState = TIT_SYS_MODE_CB_MASK;
    }
    
    // put AGC into reset
    halWriteRegister(pMac, TIT_AGC_AGC_RESET_REG , 1 );
    
    // set cb bit to zero
    //TODO: Need to use AGC interface to set the cb_enable bit in bandwidth config register
    // halWriteRegister(pMac, TIT_SYS_MODE_REG, (tANI_U32) ( halGetReg( pMac, TIT_SYS_MODE_REG ) & ~( TIT_SYS_MODE_CB_MASK ) ) );
    
    // Disable Packet Detection
    asicSetDisabledRxPacketTypes(pMac, PHY_RX_DISABLE_ALL_TYPES);

    // enable all rx chains - This could be just the chain of interest
    // note: this does not change the power-down fields in the rx_override register
    asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_CALIBRATING);

    asicSetPMUManualEnMode(pMac, PMU_CAL_IQ_EN);

    asicPrepareRxIQCal(pMac, rxChain);   //Override CAL AIC & PIC regs to 0

    // select a single Rx RF chain because we can only put a tone on one chain
    switch (rxChain)
    {
        case PHY_RX_CHAIN_0:
            rfSetChainSelectionMode(pMac, PHY_CHAIN_SEL_R0_ONLY);
            break;
        case PHY_RX_CHAIN_1:
            rfSetChainSelectionMode(pMac, PHY_CHAIN_SEL_R1_ONLY);
            break;
        case PHY_MAX_RX_CHAINS:
        case PHY_ALL_RX_CHAINS:
            break;

        default:
            break;
    }

    halWriteRegister(pMac, TIT_AGC_AGC_RESET_REG , 0 );
    //For all calibration tones, -28 to 28, in .3125MHz/tone steps
    for (tone = 0; tone < NUM_CAL_TONES; tone++)
    {
        tANI_S16 phase;
        tToneResponse phaseMeas;
        tANI_U8 init_or_clip = 1;
        tANI_U8 rxGain = MAX_AGC_GAIN;

        rfSetCalibrationTone(pMac, calTones[tone]);

        while (init_or_clip && rxGain > 0)
        {
            asicOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS, rxGain);

            phyForceResidualDCOMeasurement(pMac); //final correction before IQ cal

            asicSetPhyCalLength(pMac, NUM_CAL_IQ_SAMPLES);
            asicSetPhyCalMode(pMac, PHY_CAL_MODE_IQCAL);

            rfSetCalMode(pMac, RF_CAL_RX_I_Q);
            phaseMeas = asicGetPhaseMeasurement(pMac, calTones[tone], rxChain);

            init_or_clip = checkClippedPhaseMeasurement( phaseMeas );

            rxGain -= 1; // step down the rx gain

            if (init_or_clip)
            {
               phyLog(pMac, LOG1, "phyIQCalRxChain(): Phase Measurement Clipped\n");
               phyLog(pMac, LOG1, "phyIQCalRxChain(): Re-Measure with rx gain = %d\n", rxGain);

            }

            rfSetCalMode(pMac, RF_CAL_NORMAL);
        }
        //Compute polynomial divide and use LUT to get phase per calibration tone.
        phase = ComputePhase(pMac, phaseMeas);

        phyLog(pMac, LOG1, "phyIQCalRxChain(): Phase Measurement, tone index = %d, rx chain = %d\n", tone, rxChain);
        phyLog(pMac, LOG1, "phyIQCalRxChain(): tone = %d + %dj, image = %d + %dj\n", 
                        phaseMeas.desired.real, 
                        phaseMeas.desired.imaginary, 
                        phaseMeas.image.real, 
                        phaseMeas.image.imaginary
              );

        phaseCorrAccum += phase;
        phyLog(pMac, LOG1, "phaseCorrAccum = %d\n", phaseCorrAccum);
        
        
        ampCorrection += phaseMeas.ampCorrect;
    }   //end tone loop

    phaseCorrection = (tANI_S8)( phaseCorrAccum / NUM_CAL_TONES );


    phyLog(pMac, LOG1, "phyIQCalRxChain(): rx chain = %d, phase Correction = %d amp Correct = %d\n", 
                        rxChain, phaseCorrection, (tANI_S8)(ampCorrection / NUM_CAL_TONES));

    asicWriteRxPhaseCorrection(pMac, rxChain, phaseCorrection);
    asicWriteRxAmpCorrection(pMac, rxChain, (tANI_S8)(ampCorrection / NUM_CAL_TONES));

    // put AGC into reset
    halWriteRegister(pMac, TIT_AGC_AGC_RESET_REG , 1 );
    
    //restore normal settings
    asicSetPhaAccess(pMac, PHA_INTERNAL_ACCESS);
    asicSetPhyCalMode(pMac, PHY_CAL_MODE_NORMAL);

    rfSetCalMode(pMac, RF_CAL_NORMAL);

    asicSetPMUManualEnMode(pMac, PMU_AUTO_EN);

    rfSetChainSelectionMode(pMac, currentChains);   //restore current chain selection
    
    // restore AGC Gain to 'Auto'
    asicCeaseOverrideAGCRxChainGain(pMac, PHY_ALL_RX_CHAINS );
    
    // restore cal length to 
    asicSetPhyCalLength( pMac, calLength );
    
    // return rx chains to auto-enable
    // note: this does not change the power-down fields in the rx_override register
    asicOverrideAGCRxEnable(pMac, AGC_ALL_RX, AGC_RX_ON_AUTO);

    // restore packet detection mode
    asicSetDisabledRxPacketTypes(pMac, agcDisModeState);
    
    // restore CB
    //TODO: Need to use AGC interface to set the cb_enable bit in bandwidth config register
    // halWriteRegister(pMac, TIT_SYS_MODE_REG, (tANI_U32)(halGetReg( pMac, TIT_SYS_MODE_REG ) | cbState ) );
    
    // pull AGC out of reset
    halWriteRegister(pMac, TIT_AGC_AGC_RESET_REG , 0 );
}

static tANI_S16 ComputePhase(tpAniSirGlobal pMac, tToneResponse phaseMeas)
{
    tANI_S32 x0, x1, y0, y1;
    tANI_S32 retVal;

    x0 = phaseMeas.desired.real;
    y0 = phaseMeas.desired.imaginary;
    x1 = phaseMeas.image.real;
    y1 = phaseMeas.image.imaginary;

    retVal = ((x0 * y1) + (x1 * y0));
    phyLog(pMac, LOG2, "step1: %d\n", retVal);
    retVal <<= 10;
    phyLog(pMac, LOG2, "step2: %d\n", retVal);
    retVal /= ((x0*x0) + (y0*y0) + 1);
    phyLog(pMac, LOG2, "step3: %d\n", retVal);
    retVal = -retVal;
    phyLog(pMac, LOG2, "step4: %d\n", retVal);

    if (retVal > 32767)
        retVal = 32767;

    if (retVal < -32767)
        retVal = -32767;
    
    
    return((tANI_S16)retVal);
}

static tANI_U8 checkClippedS16( tANI_S16 x )
{
    return ((tANI_U8)(x >= 2047 || x <= -2047));
}

static tANI_U8 checkClippedComplexInteger( tComplexInteger x )
{
    return ((tANI_U8)( checkClippedS16( x.real ) || checkClippedS16( x.imaginary ) ));
}

static tANI_U8 checkClippedPhaseMeasurement(tToneResponse phaseMeas)
{
    return ((tANI_U8)( checkClippedComplexInteger( phaseMeas.desired ) || checkClippedComplexInteger( phaseMeas.image ) ));
}
