/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   asicTPC.h:
   Author:  Mark Nelson
   Date:    3/25/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ASICTPC_H
#define ASICTPC_H

#define TPC_TXPWR_ENABLE_MASK                   QWLAN_TPC_TXPWR_ENABLE_EN_MASK


#define TPC_MEM_TX0_PWR_LUT_OFFSET              QWLAN_TPC_POWERDET0_RAM_MREG
#define TPC_MEM_TX1_PWR_LUT_OFFSET              QWLAN_TPC_POWERDET1_RAM_MREG
#define TPC_MEM_TX2_PWR_LUT_OFFSET              QWLAN_TPC_POWERDET2_RAM_MREG
#define TPC_MEM_TX3_PWR_LUT_OFFSET              QWLAN_TPC_POWERDET3_RAM_MREG
#define TPC_MEM_TX0_GAIN_LUT_OFFSET             QWLAN_TPC_GAIN_LUT0_MREG
#define TPC_MEM_TX1_GAIN_LUT_OFFSET             QWLAN_TPC_GAIN_LUT1_MREG
#define TPC_MEM_TX2_GAIN_LUT_OFFSET             QWLAN_TPC_GAIN_LUT2_MREG
#define TPC_MEM_TX3_GAIN_LUT_OFFSET             QWLAN_TPC_GAIN_LUT3_MREG

//these masks are the same for both chains
#define TPC_POWERDET_MASK                      QWLAN_TPC_POWERDET0_RAM_POWER_MASK
#define TPC_GAIN_RF_MASK                       QWLAN_TPC_GAIN_LUT0_RF_GAIN_MASK
#define TPC_GAIN_RF_OFFSET                     QWLAN_TPC_GAIN_LUT0_RF_GAIN_OFFSET
#define TPC_GAIN_DIG_MASK                      QWLAN_TPC_GAIN_LUT0_DIG_GAIN_MASK


#define TPC_MEM_POWER_LUT_DEPTH                 128
#define TPC_MEM_GAIN_LUT_DEPTH                  32


#define TPC_ADC_CTRL_REG                        QWLAN_TPC_ADC_CTRL_GET_ADC_REG
#define TPC_ADC_GET_MASK                        QWLAN_TPC_ADC_CTRL_GET_ADC_GET_ADC_MASK

#define TPC_ADC_FAILED_MASK                     QWLAN_TPC_ADC_STATUS_FAILED_MASK
#define TPC_ADC_BUSY_P_MASK                     QWLAN_TPC_ADC_STATUS_BUSY_P_MASK
#define TPC_ADC_BUSY_T_MASK                     QWLAN_TPC_ADC_STATUS_BUSY_T_MASK



#define COARSE_GAIN_MASK    MSK_4
#define COARSE_GAIN_OFFSET  4
#define FINE_GAIN_MASK      MSK_4   //the upper most bit overlaps the coarse gain and should not be used for TPC LUT data
#define FINE_GAIN_OFFSET    0

typedef enum
{
    TPC_COARSE_TXPWR_0_DBM      = 0,
    TPC_COARSE_TXPWR_POS_2_DBM  = 1,
    TPC_COARSE_TXPWR_POS_4_DBM  = 2,
    TPC_COARSE_TXPWR_POS_6_DBM  = 3,
    TPC_COARSE_TXPWR_POS_8_DBM  = 4,
    TPC_COARSE_TXPWR_POS_10_DBM = 5,
    TPC_COARSE_TXPWR_POS_12_DBM = 6,
    TPC_COARSE_TXPWR_POS_14_DBM = 7,
    TPC_COARSE_TXPWR_POS_16_DBM = 8,
    TPC_COARSE_TXPWR_POS_18_DBM = 9,
    TPC_COARSE_TXPWR_POS_20_DBM = 10,
    TPC_COARSE_TXPWR_POS_22_DBM = 11,
    TPC_COARSE_TXPWR_POS_24_DBM = 12,
    TPC_COARSE_TXPWR_POS_26_DBM = 13,
    TPC_COARSE_TXPWR_POS_28_DBM = 14,
    TPC_COARSE_TXPWR_POS_30_DBM = 15,
    NUM_TPC_COARSE_STEPS = TPC_COARSE_TXPWR_POS_30_DBM - TPC_COARSE_TXPWR_0_DBM + 1,
    MIN_TPC_COARSE_TXPWR = TPC_COARSE_TXPWR_0_DBM,
    MAX_TPC_COARSE_TXPWR = TPC_COARSE_TXPWR_POS_30_DBM
}eTxCoarseGain;                                 //refers to the external RF power adjustment

typedef enum
{
    TPC_FINE_TXPWR_0,
    TPC_FINE_TXPWR_1,
    TPC_FINE_TXPWR_2,
    TPC_FINE_TXPWR_3,
    TPC_FINE_TXPWR_4,
    TPC_FINE_TXPWR_5,
    TPC_FINE_TXPWR_6,
    TPC_FINE_TXPWR_7,
    TPC_FINE_TXPWR_8,
    TPC_FINE_TXPWR_9,
    TPC_FINE_TXPWR_10,
    TPC_FINE_TXPWR_11,
    TPC_FINE_TXPWR_12,
    TPC_FINE_TXPWR_13,
    TPC_FINE_TXPWR_14,
    TPC_FINE_TXPWR_15,
    MIN_TPC_FINE_TXPWR = TPC_FINE_TXPWR_0,
    MAX_TPC_FINE_TXPWR = TPC_FINE_TXPWR_7
}eTxFineGain;                                  //refers to the internal TxFIR power adjustment

typedef struct
{
    eTxCoarseGain coarsePwr;
    eTxFineGain finePwr;
}tTxGain;


#endif /* ASICTPC_H */
