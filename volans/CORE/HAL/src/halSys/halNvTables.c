/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halNvTables.c

    \brief Contains collection of table default values to use in case a table is not found in NV

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALNVTABLES_C
#define HALNVTABLES_C


#include "halNv.h"

const sHalNv nvDefaults =
{
    {
        0,                                                              // tANI_U16  productId;
        0,                                                              // tANI_U8   productBands;
        { 0 },                                                          // tANI_U8   unused1[0];
        1,                                                              // tANI_U8   numOfTxChains;
        2,                                                              // tANI_U8   numOfRxChains;
        { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00 },                         // tANI_U8   macAddr[NV_FIELD_MAC_ADDR_SIZE];
        { "\0" }
    }, //fields

    {
        // NV_TABLE_RATE_POWER_SETTINGS
        {
            // typedef tANI_S8 tPowerdBm;
            //typedef tPowerdBm tRateGroupPwr[NUM_HAL_PHY_RATES];
            //tRateGroupPwr       pwrOptimum[NUM_RF_SUBBANDS];
            {
                    //802.11b Rates
                {1900},    // HAL_PHY_RATE_11B_LONG_1_MBPS,
                {1900},    // HAL_PHY_RATE_11B_LONG_2_MBPS,
                {1900},    // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
                {1900},    // HAL_PHY_RATE_11B_LONG_11_MBPS,
                {1900},    // HAL_PHY_RATE_11B_SHORT_2_MBPS,
                {1900},    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
                {1900},    // HAL_PHY_RATE_11B_SHORT_11_MBPS,

                    //SLR Rates
                {1900},    // HAL_PHY_RATE_SLR_0_25_MBPS,
                {1900},    // HAL_PHY_RATE_SLR_0_5_MBPS,

                    //11A 20MHz Rates
                {1500},    // HAL_PHY_RATE_11A_6_MBPS,
                {1500},    // HAL_PHY_RATE_11A_9_MBPS,
                {1500},    // HAL_PHY_RATE_11A_12_MBPS,
                {1500},    // HAL_PHY_RATE_11A_18_MBPS,
                {1500},    // HAL_PHY_RATE_11A_24_MBPS,
                {1500},    // HAL_PHY_RATE_11A_36_MBPS,
                {1500},    // HAL_PHY_RATE_11A_48_MBPS,
                {1500},    // HAL_PHY_RATE_11A_54_MBPS,

                    //MCS Index #0-15 (20MHz)
                {1500},    // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
                {1300},    // HAL_PHY_RATE_MCS_1NSS_65_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
                {1300}     // HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS,
            },  //    RF_SUBBAND_2_4_GHZ,
        },


        // NV_TABLE_REGULATORY_DOMAINS
        {
            // typedef struct
            // {
            //     tANI_BOOLEAN enabled;
            //     tPowerdBm pwrLimit;
            // }sRegulatoryChannel;

            // typedef struct
            // {
            //     sRegulatoryChannel channels[NUM_RF_CHANNELS];
            //     uAbsPwrPrecision antennaGain[NUM_RF_SUBBANDS];
            //     uAbsPwrPrecision bRatePowerOffset[NUM_2_4GHZ_CHANNELS];
            // }sRegulatoryDomains;

            //sRegulatoryDomains  regDomains[NUM_REG_DOMAINS];


            {   // REG_DOMAIN_FCC start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 23},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 22},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 22},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_FCC end

            {   // REG_DOMAIN_ETSI start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 19},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 19},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_ETSI end

            {   // REG_DOMAIN_JAPAN start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 20},           //RF_CHAN_13,
                    {eANI_BOOLEAN_TRUE, 18},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_JAPAN end

            {   // REG_DOMAIN_WORLD start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_WORLD end

            {   // REG_DOMAIN_N_AMER_EXC_FCC start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 16},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            },   // REG_DOMAIN_N_AMER_EXC_FCC end

            {   // REG_DOMAIN_APAC start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 26},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_APAC end

            {   // REG_DOMAIN_KOREA start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 15},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_KOREA end

            {   // REG_DOMAIN_HI_5GHZ start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 14},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            }, // REG_DOMAIN_HI_5GHZ end

            {   // REG_DOMAIN_NO_5GHZ start
                { //sRegulatoryChannel start
                    //enabled, pwrLimit
                                       //2.4GHz Band
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 12},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 0},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { 0 },                       //RF_CHAN_1,
                    { 0 },                       //RF_CHAN_2,
                    { 0 },                       //RF_CHAN_3,
                    { 0 },                       //RF_CHAN_4,
                    { 0 },                       //RF_CHAN_5,
                    { 0 },                       //RF_CHAN_6,
                    { 0 },                       //RF_CHAN_7,
                    { 0 },                       //RF_CHAN_8,
                    { 0 },                       //RF_CHAN_9,
                    { 0 },                       //RF_CHAN_10,
                    { 0 },                       //RF_CHAN_11,
                    { 0 },                       //RF_CHAN_12,
                    { 0 },                       //RF_CHAN_13,
                    { 0 },                       //RF_CHAN_14,
                } // bRatePowerOffset end
            } // REG_DOMAIN_NO_5GHZ end
        },

        // NV_TABLE_DEFAULT_COUNTRY
        {
            // typedef struct
            // {
            //     tANI_U8 regDomain;                                      //from eRegDomainId
            //     tANI_U8 countryCode[NV_FIELD_COUNTRY_CODE_SIZE];    // string identifier
            // }sDefaultCountry;

            0,                  // regDomain
            { 'U', 'S', 'I' }   // countryCode
        },
        
        //NV_TABLE_CAL_MEMORY
        {
            0x7FFF,      // tANI_U16    process_monitor;
            0x00,        // tANI_U8     hdet_cal_code;
            0x00,        // tANI_U8     rxfe_gm_2;
                    
            0x00,        // tANI_U8     tx_bbf_rtune;
            0x00,        // tANI_U8     pa_rtune_reg;
            0x00,        // tANI_U8     rt_code;
            0x00,        // tANI_U8     bias_rtune;

            0x00,        // tANI_U8     bb_bw1;
            0x00,        // tANI_U8     bb_bw2;
            { 0x00, 0x00 },        // tANI_U8     reserved[2];

            0x00,        // tANI_U8     bb_bw3;
            0x00,        // tANI_U8     bb_bw4;
            0x00,        // tANI_U8     bb_bw5;
            0x00,        // tANI_U8     bb_bw6;

            0x7FFF,      // tANI_U16    rcMeasured;
            0x00,        // tANI_U8     tx_bbf_ct;
            0x00,        // tANI_U8     tx_bbf_ctr;

            0x00,        // tANI_U8     csh_maxgain_reg;
            0x00,        // tANI_U8     csh_0db_reg;
            0x00,        // tANI_U8     csh_m3db_reg;
            0x00,        // tANI_U8     csh_m6db_reg;

            0x00,        // tANI_U8     cff_0db_reg;
            0x00,        // tANI_U8     cff_m3db_reg;
            0x00,        // tANI_U8     cff_m6db_reg;
            0x00,        // tANI_U8     rxfe_gpio_ctl_1;

            0x00,        // tANI_U8     mix_bal_cnt_2;
            0x00,        // tANI_S8     rxfe_lna_highgain_bias_ctl_delta;
            0x00,        // tANI_U8     rxfe_lna_load_ctune;
            0x00,        // tANI_U8     rxfe_lna_ngm_rtune;

            0x00,        // tANI_U8     rx_im2_i_cfg0;
            0x00,        // tANI_U8     rx_im2_i_cfg1;
            0x00,        // tANI_U8     rx_im2_q_cfg0;
            0x00,        // tANI_U8     rx_im2_q_cfg1;

            0x00,        // tANI_U8     pll_vfc_reg3_b0;
            0x00,        // tANI_U8     pll_vfc_reg3_b1;
            0x00,        // tANI_U8     pll_vfc_reg3_b2;
            0x00,        // tANI_U8     pll_vfc_reg3_b3;

            0x7FFF,        // tANI_U16    tempStart;
            0x7FFF,        // tANI_U16    tempFinish;

            { //txLoCorrections
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_1
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_2
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_3
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_4
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_5
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_6
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_7
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_8
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_9
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_10
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_11
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_12
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_13
                {
                    { 0x00, 0x00 }, // TX_GAIN_STEP_0
                    { 0x00, 0x00 }, // TX_GAIN_STEP_1
                    { 0x00, 0x00 }, // TX_GAIN_STEP_2
                    { 0x00, 0x00 }, // TX_GAIN_STEP_3
                    { 0x00, 0x00 }, // TX_GAIN_STEP_4
                    { 0x00, 0x00 }, // TX_GAIN_STEP_5
                    { 0x00, 0x00 }, // TX_GAIN_STEP_6
                    { 0x00, 0x00 }, // TX_GAIN_STEP_7
                    { 0x00, 0x00 }, // TX_GAIN_STEP_8
                    { 0x00, 0x00 }, // TX_GAIN_STEP_9
                    { 0x00, 0x00 }, // TX_GAIN_STEP_10
                    { 0x00, 0x00 }, // TX_GAIN_STEP_11
                    { 0x00, 0x00 }, // TX_GAIN_STEP_12
                    { 0x00, 0x00 }, // TX_GAIN_STEP_13
                    { 0x00, 0x00 }, // TX_GAIN_STEP_14
                    { 0x00, 0x00 }  // TX_GAIN_STEP_15
                }  //RF_CHAN_14
            },        // tTxLoCorrections    txLoValues;
            
            { //sTxIQChannel
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_1
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_2
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_3
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_4
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_5
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_6
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_7
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_8
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_9
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_10
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_11
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_12
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }, //RF_CHAN_13
                {
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // TX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // TX_GAIN_STEP_15
                }  //RF_CHAN_14
            },        // sTxIQChannel        txIqValues;
            
            { //sRxIQChannel
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_1
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_2
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_3
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_4
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_5
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_6
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_7
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_8
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_9
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_10
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_11
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_12
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }, //RF_CHAN_13
                {
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_0
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_1
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_2
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_3
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_4
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_5
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_6
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_7
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_8
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_9
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_10
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_11
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_12
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_13
                    { 0x0000, 0x0000, 0x0000 }, // RX_GAIN_STEP_14
                    { 0x0000, 0x0000, 0x0000 }  // RX_GAIN_STEP_15
                }  //RF_CHAN_14
            },        // sRxIQChannel        rxIqValues;
            
            { // tTpcConfig          clpcData[MAX_TPC_CHANNELS]
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_1
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_2
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_3
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_4
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_5
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_6
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_7
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_8
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_9
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_10
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_11
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }, // RF_CHAN_12
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                },  // RF_CHAN_13
                {
                    {
                        {
                            { 0x00, 0x00 }, //CAL_POINT_0
                            { 0x00, 0x00 }, //CAL_POINT_1
                            { 0x00, 0x00 }, //CAL_POINT_2
                            { 0x00, 0x00 }, //CAL_POINT_3
                            { 0x00, 0x00 }, //CAL_POINT_4
                            { 0x00, 0x00 }, //CAL_POINT_5
                            { 0x00, 0x00 }, //CAL_POINT_6
                            { 0x00, 0x00 }  //CAL_POINT_7
                        } // PHY_TX_CHAIN_0
                    } // empirical
                }  // RF_CHAN_14
            },        // tTpcConfig          clpcData[MAX_TPC_CHANNELS];
            
            {
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_1: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_2: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_3: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_4: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_5: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_6: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_7: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_8: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_9: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_10: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_11: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_12: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }, // RF_CHAN_13: pdadc_offset, reserved[2]
                { 0x0000, { 0x00, 0x00 } }  // RF_CHAN_14: pdadc_offset, reserved[2]
            }        // tTpcParams          clpcParams[MAX_TPC_CHANNELS];
            
        }, //NV_TABLE_CAL_MEMORY
        
        //NV_TABLE_CAL_STATUS
        {
            0xFF,        // tANI_U8     overall;
            0xFF,        // tANI_U8     fwInit;
            0xFF,        // tANI_U8     hdet_dco;
            0xFF,        // tANI_U8     rtuner;
            0xFF,        // tANI_U8     ctuner;
            0xFF,        // tANI_U8     insitu;
            0xFF,        // tANI_U8     process_monitor;
            0xFF,        // tANI_U8     pllVcoLinearity;
            0xFF,        // tANI_U8     txIQ;
            0xFF,        // tANI_U8     rxIQ;
            0xFF,        // tANI_U8     rxDco;
            0xFF,        // tANI_U8     txLo;
            0xFF,        // tANI_U8     lnaBias;
            0xFF,        // tANI_U8     lnaBandTuning;
            0xFF,        // tANI_U8     lnaGainAdjust;
            0xFF,        // tANI_U8     im2UsingNoisePwr;
            0xFF,        // tANI_U8     temperature;
            0xFF,        // tANI_U8     clpc;
            0xFF,        // tANI_U8     clpc_temp_adjust;
            0xFF,        // tANI_U8     txDpd;
            0xFF,        // tANI_U8     channelTune;
            0xFF,        // tANI_U8     rxGmStageLinearity;
            0xFF,        // tANI_U8     im2UsingToneGen;
            {
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00
            }        // tANI_U8     unused[9];
        }

    } // tables
};


#endif

