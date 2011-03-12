#if !defined( __VOS_NV_DATA_STRUCTS_H )
#define __VOS_NV_DATA_STRUCTS_H

/**=========================================================================
  
  \file  vos_nv_data_structs.h
  
  \brief virtual Operating System Services (vOSS) NV data structures
               
   Header file that to contain all the data structures required for NV 
   to function without including HAL & FW header files within VoSS.
   These data structures have been moved up from HAL and FW header 
   files and those header files have been modified to use this one.
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/
 /*=========================================================================== 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$ $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  02/10/10    rnair   Creating a new header file to contain all NV data 
                      structs
===========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/


#include "vos_types.h"

//From wlanfw/inc/halPhyTypes.h

typedef v_S7_t tPowerdBm;   //power in signed 8-bit integer, no decimal places

typedef union
{
    v_U32_t measurement;      //measured values can be passed to pttApi, but are maintained to 2 decimal places internally
    v_U16_t reported;         //used internally only - reported values only maintain 2 decimals places
}uAbsPwrPrecision;

typedef enum
{
    PHY_TX_CHAIN_0 = 0,

    PHY_MAX_TX_CHAINS = 1,
    PHY_ALL_TX_CHAINS,

    //possible tx chain combinations
    PHY_NO_TX_CHAINS,
    PHY_TX_CHAIN_MAX = 0xFFFFFFFF  /* define as 4 bytes data */
}ePhyTxChains;

//From wlanfw/inc/halRfTypes.h

typedef enum
{
    REG_DOMAIN_FCC,
    REG_DOMAIN_ETSI,
    REG_DOMAIN_JAPAN,
    REG_DOMAIN_WORLD,
    REG_DOMAIN_N_AMER_EXC_FCC,
    REG_DOMAIN_APAC,
    REG_DOMAIN_KOREA,
    REG_DOMAIN_HI_5GHZ,
    REG_DOMAIN_NO_5GHZ,

    NUM_REG_DOMAINS
}eRegDomainId;

typedef enum
{
    RF_SUBBAND_2_4_GHZ      = 0,

    NUM_RF_SUBBANDS,

    MAX_RF_SUBBANDS,
    INVALID_RF_SUBBAND,

    RF_BAND_2_4_GHZ = 0,
    NUM_RF_BANDS
}eRfSubBand;

typedef enum
{
    //2.4GHz Band
    RF_CHAN_1                 = 0,
    RF_CHAN_2                 = 1,
    RF_CHAN_3                 = 2,
    RF_CHAN_4                 = 3,
    RF_CHAN_5                 = 4,
    RF_CHAN_6                 = 5,
    RF_CHAN_7                 = 6,
    RF_CHAN_8                 = 7,
    RF_CHAN_9                 = 8,
    RF_CHAN_10                = 9,
    RF_CHAN_11                = 10,
    RF_CHAN_12                = 11,
    RF_CHAN_13                = 12,
    RF_CHAN_14                = 13,

    NUM_RF_CHANNELS,

    MIN_2_4GHZ_CHANNEL = RF_CHAN_1,
    MAX_2_4GHZ_CHANNEL = RF_CHAN_14,
    NUM_2_4GHZ_CHANNELS = (MAX_2_4GHZ_CHANNEL - MIN_2_4GHZ_CHANNEL + 1),

    MIN_20MHZ_RF_CHANNEL = RF_CHAN_1,
    MAX_20MHZ_RF_CHANNEL = RF_CHAN_14,
    NUM_20MHZ_RF_CHANNELS = (MAX_20MHZ_RF_CHANNEL - MIN_20MHZ_RF_CHANNEL + 1),

    NUM_TPC_2_4GHZ_CHANNELS = 14,

    INVALID_RF_CHANNEL = 0xBAD
}eRfChannels;

typedef struct
{
    v_BOOL_t enabled;
    tPowerdBm pwrLimit;
}sRegulatoryChannel;

typedef struct
{
    sRegulatoryChannel channels[NUM_RF_CHANNELS];
    uAbsPwrPrecision antennaGain[NUM_RF_SUBBANDS];
    uAbsPwrPrecision bRatePowerOffset[NUM_2_4GHZ_CHANNELS];
    uAbsPwrPrecision gnRatePowerOffset[NUM_2_4GHZ_CHANNELS];
}sRegulatoryDomains;

typedef struct
{
    v_S15_t bRssiOffset[NUM_2_4GHZ_CHANNELS];
    v_S15_t gnRssiOffset[NUM_2_4GHZ_CHANNELS];
}sRssiChannelOffsets;

typedef struct
{
    v_U16_t targetFreq;           //number in MHz
    v_U8_t channelNum;            //channel number as in the eRfChannels enumeration
    eRfSubBand band;               //band that this channel belongs to
}tRfChannelProps;

//From wlanfw/inc/halPhyCalMemory.h
typedef struct
{
    v_U16_t    process_monitor;
    v_U8_t     hdet_cal_code;
    v_U8_t     rxfe_gm_2;

    v_U8_t     tx_bbf_rtune;
    v_U8_t     pa_rtune_reg;
    v_U8_t     rt_code;
    v_U8_t     bias_rtune;

    v_U8_t     bb_bw1;
    v_U8_t     bb_bw2;
    v_U8_t     reserved0;
    v_U8_t     reserved1;

    v_U8_t     bb_bw3;
    v_U8_t     bb_bw4;
    v_U8_t     bb_bw5;
    v_U8_t     bb_bw6;

    v_U16_t    rcMeasured;
    v_U8_t     tx_bbf_ct;
    v_U8_t     tx_bbf_ctr;

    v_U8_t     csh_maxgain_reg;
    v_U8_t     csh_0db_reg;
    v_U8_t     csh_m3db_reg;
    v_U8_t     csh_m6db_reg;

    v_U8_t     cff_0db_reg;
    v_U8_t     cff_m3db_reg;
    v_U8_t     cff_m6db_reg;
    v_U8_t     rxfe_gpio_ctl_1;

    v_U8_t     mix_bal_cnt_2;
    v_S7_t     rxfe_lna_highgain_bias_ctl_delta;
    v_U8_t     rxfe_lna_load_ctune;
    v_U8_t     rxfe_lna_ngm_rtune;

    v_U8_t     rx_im2_spare0;
    v_U8_t     rx_im2_spare1;
    v_U8_t     reserved2;
    v_U8_t     reserved3;

    v_U8_t     pll_vfc_reg3_b0;
    v_U8_t     pll_vfc_reg3_b1;
    v_U8_t     pll_vfc_reg3_b2;
    v_U8_t     pll_vfc_reg3_b3;

    v_U16_t    tempStart;
    v_U16_t    tempFinish;

}sCalData;

typedef struct
{
    v_U32_t calStatus;  //use eNvCalID
    sCalData calData;
}sRFCalValues;

//From wlanfw/inc/halPhyCfg.h
typedef v_U8_t tTpcLutValue;

#define MAX_TPC_CAL_POINTS      (8)

typedef v_U8_t tPowerDetect;        //7-bit power detect reading

typedef struct
{
    tPowerDetect pwrDetAdc;            //= SENSED_PWR register, which reports the 8-bit ADC
                                       // the stored ADC value gets shifted to 7-bits as the index to the LUT
    tPowerDetect adjustedPwrDet;       //7-bit value that goes into the LUT at the LUT[pwrDet] location
                                       //MSB set if extraPrecision.hi8_adjustedPwrDet is used
}tTpcCaldPowerPoint;

typedef tTpcCaldPowerPoint tTpcCaldPowerTable[PHY_MAX_TX_CHAINS][MAX_TPC_CAL_POINTS];

typedef struct
{
    tTpcCaldPowerTable empirical;                      //calibrated power points
}tTpcConfig;

//From wlanfw/inc/phyTxPower.h
typedef tTpcLutValue tTpcPowerTable[PHY_MAX_TX_CHAINS][128];  

typedef struct
{
    tTpcConfig *pwrSampled;             //points to CLPC data in calMemory
}tPhyTxPowerBand;

//From halPhyRates.h
typedef enum
{
    //802.11b Rates
    HAL_PHY_RATE_11B_LONG_1_MBPS,
    HAL_PHY_RATE_11B_LONG_2_MBPS,
    HAL_PHY_RATE_11B_LONG_5_5_MBPS,
    HAL_PHY_RATE_11B_LONG_11_MBPS,
    HAL_PHY_RATE_11B_SHORT_2_MBPS,
    HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
    HAL_PHY_RATE_11B_SHORT_11_MBPS,

    //SLR Rates
    HAL_PHY_RATE_SLR_0_25_MBPS,
    HAL_PHY_RATE_SLR_0_5_MBPS,

    //Spica_Virgo 11A 20MHz Rates
    HAL_PHY_RATE_11A_6_MBPS,
    HAL_PHY_RATE_11A_9_MBPS,
    HAL_PHY_RATE_11A_12_MBPS,
    HAL_PHY_RATE_11A_18_MBPS,
    HAL_PHY_RATE_11A_24_MBPS,
    HAL_PHY_RATE_11A_36_MBPS,
    HAL_PHY_RATE_11A_48_MBPS,
    HAL_PHY_RATE_11A_54_MBPS,

    //MCS Index #0-15 (20MHz)
    HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
    HAL_PHY_RATE_MCS_1NSS_13_MBPS,
    HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
    HAL_PHY_RATE_MCS_1NSS_26_MBPS,
    HAL_PHY_RATE_MCS_1NSS_39_MBPS,
    HAL_PHY_RATE_MCS_1NSS_52_MBPS,
    HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
    HAL_PHY_RATE_MCS_1NSS_65_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
    HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS,

    NUM_HAL_PHY_RATES,
    HAL_PHY_RATE_INVALID,
    MIN_RATE_INDEX                                    = 0,
    MAX_RATE_INDEX = HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS
}eHalPhyRates;

#define NUM_RATE_POWER_GROUPS           NUM_HAL_PHY_RATES  //total number of rate power groups including the CB_RATE_POWER_OFFSET
typedef uAbsPwrPrecision tRateGroupPwr[NUM_RATE_POWER_GROUPS];

//From halNvTables.h
#define NV_FIELD_COUNTRY_CODE_SIZE  3
typedef struct
{
    v_U8_t regDomain;                                  //from eRegDomainId
    v_U8_t countryCode[NV_FIELD_COUNTRY_CODE_SIZE];    // string identifier
}sDefaultCountry;

typedef union
{
    tRateGroupPwr           pwrOptimum[NUM_RF_SUBBANDS];             // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains      regDomains[NUM_REG_DOMAINS];             // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry         defaultCountryTable;                     // NV_TABLE_DEFAULT_COUNTRY
    tTpcPowerTable          plutCharacterized[NUM_2_4GHZ_CHANNELS];  // NV_TABLE_TPC_POWER_TABLE
    v_U16_t                 plutPdadcOffset[NUM_2_4GHZ_CHANNELS];    // NV_TABLE_TPC_PDADC_OFFSETS
    //sCalFlashMemory       calFlashMemory;                          // NV_TABLE_CAL_MEMORY
    //sCalStatus            calStatus;                               // NV_TABLE_CAL_STATUS
    sRssiChannelOffsets     rssiChanOffsets[2];                      // NV_TABLE_RSSI_CHANNEL_OFFSETS
    sRFCalValues            rFCalValues;                             // NV_TABLE_RF_CAL_VALUES
}uNvTables;

//From halPhy.h
typedef tPowerdBm tChannelPwrLimit;

typedef struct
{
    tANI_U8 chanId;
    tChannelPwrLimit pwr;
}tChannelListWithPower;

#endif
