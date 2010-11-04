/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2008
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   rf.h defines common data types for RF chips.


   Author:  Mark Nelson
   Date:    3/28/08

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef RF_H
#define RF_H

#include <halRfTypes.h>

#if defined(RF_CHIP_MIDAS)
#include <rfMidas.h>
#endif


extern const tRfRxGain rfGainTable[NUM_RF_SUBBANDS][NUM_RF_RX_GAIN_STEPS];
extern const tRfTxGain rfTxGainTable[NUM_TEMPERATURE_BINS][NUM_TX_GAIN_STEPS];



#define TX_DCO_RANGE_SETTING 1
#define RX_DCO_RANGE_SETTING 1

typedef enum
{
    HDET_PA_TEMP_ADC,
    HDET_LNA_BIAS_ADC,
    NUM_HDET_ADC_SETUPS
}eHdetAdcSetup;

typedef struct
{
    eRfChannels curChannel;
    tANI_S32 dcoStep[NUM_RF_DCO_VALUES];
    tRfChipVer version;
    tANI_U32 xoMode;
}tRF;


extern tANI_U8 pllVfcReg3B0;
extern tANI_U8 pllVfcReg3B1;
extern tANI_U8 pllVfcReg3B2;
extern tANI_U8 pllVfcReg3B3;


#endif /* RF_H */
