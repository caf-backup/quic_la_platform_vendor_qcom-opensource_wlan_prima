/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halPhyRates.c

    \brief halPhyRates.c

    $Id$


    Copyright (c) 2011 Qualcomm Atheros, Inc. 
    All Rights Reserved. 
    Qualcomm Atheros Confidential and Proprietary. 
  
    Copyright (C) 2006 Airgo Networks, Incorporated

   ========================================================================== */

#include <halPhyRates.h>

#if defined(ANI_PHY_DEBUG)
const char rateStr[NUM_HAL_PHY_RATES][50]=
{
    //802.11b Rates
    "HAL_PHY_RATE_11B_LONG_1_MBPS",
    "HAL_PHY_RATE_11B_LONG_2_MBPS",
    "HAL_PHY_RATE_11B_LONG_5_5_MBPS",
    "HAL_PHY_RATE_11B_LONG_11_MBPS",
    "HAL_PHY_RATE_11B_SHORT_2_MBPS",
    "HAL_PHY_RATE_11B_SHORT_5_5_MBPS",
    "HAL_PHY_RATE_11B_SHORT_11_MBPS",

    //SLR Rates
    "HAL_PHY_RATE_SLR_0_25_MBPS",
    "HAL_PHY_RATE_SLR_0_5_MBPS",

    //Spica_Virgo 11A 20MHz Rates
    "HAL_PHY_RATE_11A_6_MBPS",
    "HAL_PHY_RATE_11A_9_MBPS",
    "HAL_PHY_RATE_11A_12_MBPS",
    "HAL_PHY_RATE_11A_18_MBPS",
    "HAL_PHY_RATE_11A_24_MBPS",
    "HAL_PHY_RATE_11A_36_MBPS",
    "HAL_PHY_RATE_11A_48_MBPS",
    "HAL_PHY_RATE_11A_54_MBPS",

    //MCS Index #0-15 (20MHz)
    "HAL_PHY_RATE_MCS_1NSS_6_5_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_13_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_19_5_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_26_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_39_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_52_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_58_5_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_65_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS",
    "HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS"
};
#endif

