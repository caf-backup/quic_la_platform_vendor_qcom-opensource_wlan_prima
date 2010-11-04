/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halPhyRates.h

    \brief halPhyRates.h

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALPHYRATES_H
#define HALPHYRATES_H

#include <halPhyTypes.h>


typedef enum
{
    POWER_MODE_HIGH_POWER = 0,  //Highest possible power for actual rates
    POWER_MODE_LOW_POWER,       //Reduced power for Virtual rates   
    POWER_MODE_WIFI_DIRECT,     //0dBm for Wifi direct
    POWER_MODE_INVALID          //Should not be used
} ePowerMode;

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


#define TEST_PHY_RATE_IS_11B(rate) \
( ((rate >= HAL_PHY_RATE_11B_LONG_1_MBPS) && (rate <= HAL_PHY_RATE_11B_SHORT_11_MBPS))                              \
                                                                                                                    \
  ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE                                                                          \
)

#define TEST_PHY_RATE_IS_SLR(rate)                                                            \
( (((rate >= HAL_PHY_RATE_SLR_0_25_MBPS) && (rate <= HAL_PHY_RATE_SLR_0_5_MBPS))              \
     )                                                                                        \
  ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE                                                    \
)


#ifdef CHANNEL_BONDED_CAPABLE
#define TEST_PHY_RATE_IS_DUP(rate)                                                                                  \
( (((rate >= HAL_PHY_RATE_11AG_DUP_6_MBPS ) && (rate <= HAL_PHY_RATE_11AG_DUP_54_MBPS)) ||                          \
   ((rate >= HAL_PHY_RATE_MCS_32_DUP_1NSS_6_MBPS) && (rate <= HAL_PHY_RATE_MCS_32_DUP_1NSS_SG_6_7_MBPS)) ||         \
   ((rate >= HAL_PHY_RATE_11B_DUP_LONGP_1_MBPS) && (rate <= HAL_PHY_RATE_11B_DUP_SHORTP_11_MBPS))                   \
  )                                                                                                                 \
  ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE                                                                          \
)


#define TEST_PHY_RATE_IS_CB(rate)                                                                                  \
( (TEST_PHY_RATE_IS_DUP(rate) ||                                                                                   \
   ((rate >= HAL_PHY_RATE_MCS_0_1NSS_CB_13_5_MBPS) && (rate <= HAL_PHY_RATE_MCS_15_2NSS_SG_CB_300_MBPS)) ||        \
   ((rate >= HAL_PHY_RATE_MCS_16_3NSS_CB_40_5_MBPS) && (rate <= HAL_PHY_RATE_MCS_31_4NSS_SG_CB_600_MBPS)) ||       \
   ((rate >= HAL_PHY_RATE_STBC_CB_1NSS_13_5_MBPS) && (rate <= HAL_PHY_RATE_STBC_CB_1NSS_SG_150_MBPS)) ||           \
   ((rate >= HAL_PHY_RATE_AIRGO_GF_CB_1NSS_141_7_MBPS) && (rate <= HAL_PHY_RATE_AIRGO_GF_CB_4NSS_SG_630_MBPS)) ||  \
   (rate == HAL_PHY_RATE_AIRGO_STBC_CB_1NSS_141_MBPS)                                                              \
  )                                                                                                                \
  ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE                                                                         \
)

#define TEST_PHY_RATE_IS_NON_CB(rate) \
(                                                                                                       \
    ( !(TEST_PHY_RATE_IS_CB(rate)))                                                                     \
    ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE                                                            \
)
#else
#define TEST_PHY_RATE_IS_NON_CB(rate)   eANI_BOOLEAN_TRUE
#define TEST_PHY_RATE_IS_CB(rate)       eANI_BOOLEAN_FALSE
#define TEST_PHY_RATE_IS_DUP(rate)      eANI_BOOLEAN_FALSE
#endif

#define NUM_RATE_POWER_GROUPS           NUM_HAL_PHY_RATES  //total number of rate power groups including the CB_RATE_POWER_OFFSET

typedef uAbsPwrPrecision tRateGroupPwr[NUM_RATE_POWER_GROUPS];

#endif

