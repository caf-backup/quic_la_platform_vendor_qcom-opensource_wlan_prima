/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

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


extern DATA_HPHY_INIT const tRfRxGain rfGainTable[NUM_RF_SUBBANDS][NUM_RF_RX_GAIN_STEPS];
extern DATA_HPHY_INIT const tRfTxGain rfTxGainTableVolans2[NUM_TX_GAIN_STEPS];
extern DATA_HPHY_INIT const tRfTxGain rfTxDAGainTable[NUM_TX_GAIN_STEPS];
extern DATA_HPHY_INIT const tRfTxGain rfTxGainTable[NUM_TX_GAIN_STEPS];


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
    tANI_U16    chanLockTemp;   /* Temperature at which channel is locked */
    tANI_S32 dcoStep[NUM_RF_DCO_VALUES];
    tRfChipVer version;
    tANI_U32 xoMode;
}tRF;


extern tANI_U8 pllVfcReg3B0;
extern tANI_U8 pllVfcReg3B1;
extern tANI_U8 pllVfcReg3B2;
extern tANI_U8 pllVfcReg3B3;

#define RF_CHIP_VERSION(x)      (hphy.rf.version.ver >= (x))

#endif /* RF_H */
