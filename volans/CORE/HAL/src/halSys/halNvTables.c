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
                {2150},    // HAL_PHY_RATE_11B_LONG_1_MBPS,
                {2150},    // HAL_PHY_RATE_11B_LONG_2_MBPS,
                {2150},    // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
                {2150},    // HAL_PHY_RATE_11B_LONG_11_MBPS,
                {2150},    // HAL_PHY_RATE_11B_SHORT_2_MBPS,
                {2150},    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
                {2150},    // HAL_PHY_RATE_11B_SHORT_11_MBPS,

                    //SLR Rates
                {2150},    // HAL_PHY_RATE_SLR_0_25_MBPS,
                {2150},    // HAL_PHY_RATE_SLR_0_5_MBPS,

                    //11A 20MHz Rates
                {1750},    // HAL_PHY_RATE_11A_6_MBPS,
                {1750},    // HAL_PHY_RATE_11A_9_MBPS,
                {1750},    // HAL_PHY_RATE_11A_12_MBPS,
                {1750},    // HAL_PHY_RATE_11A_18_MBPS,
                {1750},    // HAL_PHY_RATE_11A_24_MBPS,
                {1750},    // HAL_PHY_RATE_11A_36_MBPS,
                {1750},    // HAL_PHY_RATE_11A_48_MBPS,
                {1750},    // HAL_PHY_RATE_11A_54_MBPS,

                    //MCS Index #0-15 (20MHz)
                {1750},    // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
                {1650},    // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
                {1550},    // HAL_PHY_RATE_MCS_1NSS_65_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
                {1750},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
                {1650},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
                {1550}     // HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS,
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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
                }, // bRatePowerOffset end

                { // gnRatePowerOffset start
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
                } // gnRatePowerOffset end
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

        //NV_TABLE_TPC_POWER_TABLE
        {
            {
                {
                    40 , //0
                    41 , //1
                    43 , //2
                    45 , //3
                    46 , //4
                    47 , //5
                    49 , //6
                    50 , //7
                    51 , //8
                    52 , //9
                    54 , //10
                    55 , //11
                    56 , //12
                    57 , //13
                    58 , //14
                    59 , //15
                    60 , //16
                    61 , //17
                    62 , //18
                    63 , //19
                    64 , //20
                    65 , //21
                    66 , //22
                    66 , //23
                    67 , //24
                    68 , //25
                    69 , //26
                    70 , //27
                    70 , //28
                    71 , //29
                    72 , //30
                    73 , //31
                    73 , //32
                    74 , //33
                    75 , //34
                    75 , //35
                    76 , //36
                    77 , //37
                    77 , //38
                    78 , //39
                    79 , //40
                    79 , //41
                    80 , //42
                    80 , //43
                    81 , //44
                    81 , //45
                    82 , //46
                    82 , //47
                    83 , //48
                    83 , //49
                    84 , //50
                    84 , //51
                    85 , //52
                    85 , //53
                    86 , //54
                    86 , //55
                    87 , //56
                    87 , //57
                    88 , //58
                    88 , //59
                    88 , //60
                    89 , //61
                    89 , //62
                    90 , //63
                    90 , //64
                    90 , //65
                    91 , //66
                    91 , //67
                    91 , //68
                    92 , //69
                    92 , //70
                    92 , //71
                    93 , //72
                    93 , //73
                    93 , //74
                    94 , //75
                    94 , //76
                    94 , //77
                    95 , //78
                    95 , //79
                    95 , //80
                    95 , //81
                    96 , //82
                    96 , //83
                    96 , //84
                    97 , //85
                    97 , //86
                    97 , //87
                    97 , //88
                    98 , //89
                    98 , //90
                    98 , //91
                    98 , //92
                    99 , //93
                    99 , //94
                    99 , //95
                    99 , //96
                    100, //97
                    100, //98
                    100, //99
                    100, //100
                    101, //101
                    101, //102
                    101, //103
                    101, //104
                    101, //105
                    102, //106
                    102, //107
                    102, //108
                    102, //109
                    103, //110
                    103, //111
                    103, //112
                    103, //113
                    104, //114
                    104, //115
                    104, //116
                    105, //117
                    105, //118
                    105, //119
                    106, //120
                    106, //121
                    107, //122
                    107, //123
                    108, //124
                    108, //125
                    109, //126
                    109, //127
                }
            }, //RF_CHAN_1
            {
                {
                    39 , //0
                    41 , //1
                    43 , //2
                    45 , //3
                    46 , //4
                    48 , //5
                    49 , //6
                    50 , //7
                    52 , //8
                    53 , //9
                    54 , //10
                    55 , //11
                    56 , //12
                    57 , //13
                    58 , //14
                    59 , //15
                    60 , //16
                    61 , //17
                    62 , //18
                    63 , //19
                    64 , //20
                    65 , //21
                    66 , //22
                    67 , //23
                    67 , //24
                    68 , //25
                    69 , //26
                    70 , //27
                    71 , //28
                    71 , //29
                    72 , //30
                    73 , //31
                    73 , //32
                    74 , //33
                    75 , //34
                    75 , //35
                    76 , //36
                    77 , //37
                    77 , //38
                    78 , //39
                    78 , //40
                    79 , //41
                    79 , //42
                    80 , //43
                    81 , //44
                    81 , //45
                    81 , //46
                    82 , //47
                    82 , //48
                    83 , //49
                    84 , //50
                    84 , //51
                    85 , //52
                    85 , //53
                    86 , //54
                    86 , //55
                    87 , //56
                    88 , //57
                    88 , //58
                    89 , //59
                    89 , //60
                    90 , //61
                    90 , //62
                    90 , //63
                    91 , //64
                    91 , //65
                    91 , //66
                    92 , //67
                    92 , //68
                    92 , //69
                    93 , //70
                    93 , //71
                    93 , //72
                    94 , //73
                    94 , //74
                    94 , //75
                    95 , //76
                    95 , //77
                    95 , //78
                    96 , //79
                    96 , //80
                    96 , //81
                    96 , //82
                    97 , //83
                    97 , //84
                    97 , //85
                    98 , //86
                    98 , //87
                    98 , //88
                    98 , //89
                    99 , //90
                    99 , //91
                    99 , //92
                    100, //93
                    100, //94
                    100, //95
                    100, //96
                    101, //97
                    101, //98
                    101, //99
                    101, //100
                    102, //101
                    102, //102
                    102, //103
                    102, //104
                    103, //105
                    103, //106
                    103, //107
                    103, //108
                    103, //109
                    104, //110
                    104, //111
                    104, //112
                    104, //113
                    105, //114
                    105, //115
                    106, //116
                    106, //117
                    106, //118
                    107, //119
                    107, //120
                    108, //121
                    108, //122
                    108, //123
                    109, //124
                    109, //125
                    109, //126
                    110, //127
                }
            }, //RF_CHAN_2
                {
                    {
                        39 , //0
                        41 , //1
                        43 , //2
                        44 , //3
                        46 , //4
                        47 , //5
                        49 , //6
                        50 , //7
                        51 , //8
                        53 , //9
                        54 , //10
                        55 , //11
                        56 , //12
                        57 , //13
                        58 , //14
                        59 , //15
                        60 , //16
                        61 , //17
                        62 , //18
                        63 , //19
                        64 , //20
                        65 , //21
                        65 , //22
                        66 , //23
                        67 , //24
                        68 , //25
                        69 , //26
                        69 , //27
                        70 , //28
                        71 , //29
                        72 , //30
                        72 , //31
                        73 , //32
                        74 , //33
                        74 , //34
                        75 , //35
                        76 , //36
                        76 , //37
                        77 , //38
                        78 , //39
                        78 , //40
                        79 , //41
                        79 , //42
                        80 , //43
                        80 , //44
                        81 , //45
                        81 , //46
                        82 , //47
                        82 , //48
                        83 , //49
                        83 , //50
                        84 , //51
                        85 , //52
                        85 , //53
                        86 , //54
                        86 , //55
                        87 , //56
                        87 , //57
                        88 , //58
                        88 , //59
                        89 , //60
                        89 , //61
                        90 , //62
                        90 , //63
                        90 , //64
                        91 , //65
                        91 , //66
                        92 , //67
                        92 , //68
                        92 , //69
                        93 , //70
                        93 , //71
                        93 , //72
                        94 , //73
                        94 , //74
                        94 , //75
                        95 , //76
                        95 , //77
                        95 , //78
                        96 , //79
                        96 , //80
                        96 , //81
                        96 , //82
                        97 , //83
                        97 , //84
                        97 , //85
                        97 , //86
                        98 , //87
                        98 , //88
                        98 , //89
                        98 , //90
                        99 , //91
                        99 , //92
                        99 , //93
                        99 , //94
                        100, //95
                        100, //96
                        100, //97
                        100, //98
                        101, //99
                        101, //100
                        101, //101
                        101, //102
                        102, //103
                        102, //104
                        102, //105
                        102, //106
                        103, //107
                        103, //108
                        103, //109
                        103, //110
                        104, //111
                        104, //112
                        104, //113
                        104, //114
                        105, //115
                        105, //116
                        106, //117
                        106, //118
                        106, //119
                        107, //120
                        107, //121
                        108, //122
                        108, //123
                        109, //124
                        109, //125
                        110, //126
                        111, //127
                    }
                }, //RF_CHAN_3
                {
                    {
                        40 , //0
                        41 , //1
                        43 , //2
                        45 , //3
                        46 , //4
                        48 , //5
                        49 , //6
                        50 , //7
                        52 , //8
                        53 , //9
                        54 , //10
                        55 , //11
                        56 , //12
                        57 , //13
                        58 , //14
                        59 , //15
                        60 , //16
                        61 , //17
                        62 , //18
                        63 , //19
                        64 , //20
                        65 , //21
                        66 , //22
                        67 , //23
                        68 , //24
                        68 , //25
                        69 , //26
                        70 , //27
                        71 , //28
                        72 , //29
                        73 , //30
                        73 , //31
                        74 , //32
                        75 , //33
                        75 , //34
                        76 , //35
                        77 , //36
                        77 , //37
                        78 , //38
                        78 , //39
                        79 , //40
                        79 , //41
                        80 , //42
                        81 , //43
                        81 , //44
                        82 , //45
                        82 , //46
                        83 , //47
                        83 , //48
                        84 , //49
                        84 , //50
                        85 , //51
                        85 , //52
                        86 , //53
                        86 , //54
                        87 , //55
                        87 , //56
                        88 , //57
                        88 , //58
                        89 , //59
                        89 , //60
                        89 , //61
                        90 , //62
                        90 , //63
                        91 , //64
                        91 , //65
                        91 , //66
                        92 , //67
                        92 , //68
                        92 , //69
                        93 , //70
                        93 , //71
                        93 , //72
                        94 , //73
                        94 , //74
                        94 , //75
                        95 , //76
                        95 , //77
                        95 , //78
                        96 , //79
                        96 , //80
                        96 , //81
                        96 , //82
                        97 , //83
                        97 , //84
                        97 , //85
                        98 , //86
                        98 , //87
                        98 , //88
                        98 , //89
                        99 , //90
                        99 , //91
                        99 , //92
                        99 , //93
                        100, //94
                        100, //95
                        100, //96
                        100, //97
                        101, //98
                        101, //99
                        101, //100
                        101, //101
                        101, //102
                        102, //103
                        102, //104
                        102, //105
                        102, //106
                        103, //107
                        103, //108
                        103, //109
                        103, //110
                        104, //111
                        104, //112
                        104, //113
                        105, //114
                        105, //115
                        105, //116
                        105, //117
                        106, //118
                        106, //119
                        106, //120
                        107, //121
                        107, //122
                        108, //123
                        109, //124
                        109, //125
                        110, //126
                        110, //127
                    }
                }, //RF_CHAN_4
                {
                    {
                        40 , //0
                        42 , //1
                        43 , //2
                        45 , //3
                        46 , //4
                        47 , //5
                        49 , //6
                        50 , //7
                        52 , //8
                        53 , //9
                        55 , //10
                        56 , //11
                        57 , //12
                        58 , //13
                        59 , //14
                        60 , //15
                        61 , //16
                        62 , //17
                        63 , //18
                        64 , //19
                        65 , //20
                        65 , //21
                        66 , //22
                        67 , //23
                        68 , //24
                        69 , //25
                        70 , //26
                        70 , //27
                        71 , //28
                        72 , //29
                        72 , //30
                        73 , //31
                        74 , //32
                        74 , //33
                        75 , //34
                        76 , //35
                        76 , //36
                        77 , //37
                        78 , //38
                        78 , //39
                        79 , //40
                        79 , //41
                        80 , //42
                        80 , //43
                        81 , //44
                        82 , //45
                        82 , //46
                        83 , //47
                        83 , //48
                        84 , //49
                        84 , //50
                        85 , //51
                        86 , //52
                        86 , //53
                        87 , //54
                        88 , //55
                        88 , //56
                        89 , //57
                        89 , //58
                        90 , //59
                        90 , //60
                        90 , //61
                        91 , //62
                        91 , //63
                        92 , //64
                        92 , //65
                        92 , //66
                        93 , //67
                        93 , //68
                        93 , //69
                        94 , //70
                        94 , //71
                        94 , //72
                        95 , //73
                        95 , //74
                        95 , //75
                        95 , //76
                        96 , //77
                        96 , //78
                        96 , //79
                        97 , //80
                        97 , //81
                        97 , //82
                        98 , //83
                        98 , //84
                        98 , //85
                        98 , //86
                        99 , //87
                        99 , //88
                        99 , //89
                        100, //90
                        100, //91
                        100, //92
                        100, //93
                        101, //94
                        101, //95
                        101, //96
                        101, //97
                        102, //98
                        102, //99
                        102, //100
                        102, //101
                        103, //102
                        103, //103
                        103, //104
                        103, //105
                        104, //106
                        104, //107
                        104, //108
                        104, //109
                        104, //110
                        105, //111
                        105, //112
                        105, //113
                        106, //114
                        106, //115
                        106, //116
                        106, //117
                        107, //118
                        107, //119
                        107, //120
                        108, //121
                        108, //122
                        108, //123
                        109, //124
                        110, //125
                        111, //126
                        111, //127
                    }
                }, //RF_CHAN_5
                {
                    {
                        40 , //0
                        41 , //1
                        43 , //2
                        45 , //3
                        46 , //4
                        47 , //5
                        49 , //6
                        50 , //7
                        52 , //8
                        53 , //9
                        54 , //10
                        55 , //11
                        57 , //12
                        58 , //13
                        59 , //14
                        60 , //15
                        61 , //16
                        62 , //17
                        63 , //18
                        63 , //19
                        64 , //20
                        65 , //21
                        66 , //22
                        67 , //23
                        68 , //24
                        69 , //25
                        69 , //26
                        70 , //27
                        71 , //28
                        72 , //29
                        73 , //30
                        73 , //31
                        74 , //32
                        75 , //33
                        75 , //34
                        76 , //35
                        77 , //36
                        77 , //37
                        78 , //38
                        78 , //39
                        79 , //40
                        80 , //41
                        80 , //42
                        81 , //43
                        81 , //44
                        82 , //45
                        83 , //46
                        83 , //47
                        84 , //48
                        85 , //49
                        85 , //50
                        86 , //51
                        86 , //52
                        87 , //53
                        88 , //54
                        88 , //55
                        89 , //56
                        89 , //57
                        90 , //58
                        90 , //59
                        90 , //60
                        91 , //61
                        91 , //62
                        92 , //63
                        92 , //64
                        92 , //65
                        93 , //66
                        93 , //67
                        93 , //68
                        94 , //69
                        94 , //70
                        94 , //71
                        95 , //72
                        95 , //73
                        95 , //74
                        95 , //75
                        96 , //76
                        96 , //77
                        96 , //78
                        97 , //79
                        97 , //80
                        97 , //81
                        98 , //82
                        98 , //83
                        98 , //84
                        98 , //85
                        99 , //86
                        99 , //87
                        99 , //88
                        99 , //89
                        100, //90
                        100, //91
                        100, //92
                        101, //93
                        101, //94
                        101, //95
                        101, //96
                        102, //97
                        102, //98
                        102, //99
                        102, //100
                        103, //101
                        103, //102
                        103, //103
                        103, //104
                        103, //105
                        104, //106
                        104, //107
                        104, //108
                        104, //109
                        105, //110
                        105, //111
                        105, //112
                        105, //113
                        106, //114
                        106, //115
                        106, //116
                        107, //117
                        107, //118
                        108, //119
                        108, //120
                        108, //121
                        109, //122
                        109, //123
                        110, //124
                        110, //125
                        110, //126
                        111, //127
                    }
                }, //RF_CHAN_6
                {
                    {
                        40 , //0
                        42 , //1
                        43 , //2
                        45 , //3
                        46 , //4
                        47 , //5
                        49 , //6
                        50 , //7
                        52 , //8
                        53 , //9
                        54 , //10
                        56 , //11
                        57 , //12
                        58 , //13
                        59 , //14
                        60 , //15
                        61 , //16
                        62 , //17
                        63 , //18
                        64 , //19
                        65 , //20
                        66 , //21
                        67 , //22
                        67 , //23
                        68 , //24
                        69 , //25
                        69 , //26
                        70 , //27
                        71 , //28
                        72 , //29
                        73 , //30
                        73 , //31
                        74 , //32
                        75 , //33
                        75 , //34
                        76 , //35
                        77 , //36
                        77 , //37
                        78 , //38
                        78 , //39
                        79 , //40
                        79 , //41
                        80 , //42
                        81 , //43
                        81 , //44
                        82 , //45
                        82 , //46
                        83 , //47
                        83 , //48
                        84 , //49
                        84 , //50
                        85 , //51
                        85 , //52
                        86 , //53
                        86 , //54
                        87 , //55
                        87 , //56
                        88 , //57
                        88 , //58
                        89 , //59
                        89 , //60
                        89 , //61
                        90 , //62
                        90 , //63
                        91 , //64
                        91 , //65
                        91 , //66
                        92 , //67
                        92 , //68
                        93 , //69
                        93 , //70
                        93 , //71
                        94 , //72
                        94 , //73
                        94 , //74
                        95 , //75
                        95 , //76
                        95 , //77
                        96 , //78
                        96 , //79
                        96 , //80
                        96 , //81
                        97 , //82
                        97 , //83
                        97 , //84
                        97 , //85
                        98 , //86
                        98 , //87
                        98 , //88
                        98 , //89
                        99 , //90
                        99 , //91
                        99 , //92
                        99 , //93
                        100, //94
                        100, //95
                        100, //96
                        100, //97
                        101, //98
                        101, //99
                        101, //100
                        101, //101
                        102, //102
                        102, //103
                        102, //104
                        102, //105
                        103, //106
                        103, //107
                        103, //108
                        103, //109
                        103, //110
                        104, //111
                        104, //112
                        104, //113
                        105, //114
                        105, //115
                        105, //116
                        106, //117
                        106, //118
                        107, //119
                        107, //120
                        107, //121
                        108, //122
                        108, //123
                        109, //124
                        109, //125
                        110, //126
                        110, //127
                    }
                }, //RF_CHAN_7
                {
                    {
                        40 , //0
                        41 , //1
                        43 , //2
                        45 , //3
                        46 , //4
                        48 , //5
                        49 , //6
                        51 , //7
                        52 , //8
                        53 , //9
                        54 , //10
                        55 , //11
                        57 , //12
                        58 , //13
                        59 , //14
                        60 , //15
                        61 , //16
                        62 , //17
                        63 , //18
                        64 , //19
                        64 , //20
                        65 , //21
                        66 , //22
                        67 , //23
                        68 , //24
                        69 , //25
                        70 , //26
                        71 , //27
                        71 , //28
                        72 , //29
                        73 , //30
                        73 , //31
                        74 , //32
                        75 , //33
                        75 , //34
                        76 , //35
                        76 , //36
                        77 , //37
                        78 , //38
                        78 , //39
                        79 , //40
                        79 , //41
                        80 , //42
                        80 , //43
                        81 , //44
                        82 , //45
                        82 , //46
                        83 , //47
                        84 , //48
                        84 , //49
                        85 , //50
                        86 , //51
                        86 , //52
                        87 , //53
                        87 , //54
                        88 , //55
                        88 , //56
                        89 , //57
                        89 , //58
                        90 , //59
                        90 , //60
                        90 , //61
                        91 , //62
                        91 , //63
                        91 , //64
                        92 , //65
                        92 , //66
                        93 , //67
                        93 , //68
                        93 , //69
                        94 , //70
                        94 , //71
                        94 , //72
                        95 , //73
                        95 , //74
                        95 , //75
                        96 , //76
                        96 , //77
                        96 , //78
                        96 , //79
                        97 , //80
                        97 , //81
                        97 , //82
                        98 , //83
                        98 , //84
                        98 , //85
                        98 , //86
                        99 , //87
                        99 , //88
                        99 , //89
                        100, //90
                        100, //91
                        100, //92
                        100, //93
                        101, //94
                        101, //95
                        101, //96
                        101, //97
                        101, //98
                        102, //99
                        102, //100
                        102, //101
                        102, //102
                        103, //103
                        103, //104
                        103, //105
                        103, //106
                        104, //107
                        104, //108
                        104, //109
                        104, //110
                        105, //111
                        105, //112
                        105, //113
                        106, //114
                        106, //115
                        106, //116
                        106, //117
                        107, //118
                        107, //119
                        107, //120
                        108, //121
                        108, //122
                        109, //123
                        109, //124
                        109, //125
                        110, //126
                        110, //127
                    }
                }, //RF_CHAN_8
                {
                    {
                        41 , //0
                        43 , //1
                        44 , //2
                        46 , //3
                        47 , //4
                        49 , //5
                        50 , //6
                        52 , //7
                        53 , //8
                        54 , //9
                        55 , //10
                        57 , //11
                        58 , //12
                        59 , //13
                        60 , //14
                        61 , //15
                        62 , //16
                        63 , //17
                        64 , //18
                        65 , //19
                        66 , //20
                        67 , //21
                        68 , //22
                        69 , //23
                        69 , //24
                        70 , //25
                        71 , //26
                        72 , //27
                        72 , //28
                        73 , //29
                        74 , //30
                        75 , //31
                        75 , //32
                        76 , //33
                        77 , //34
                        77 , //35
                        78 , //36
                        79 , //37
                        79 , //38
                        80 , //39
                        80 , //40
                        81 , //41
                        81 , //42
                        82 , //43
                        82 , //44
                        83 , //45
                        83 , //46
                        84 , //47
                        85 , //48
                        85 , //49
                        86 , //50
                        86 , //51
                        87 , //52
                        88 , //53
                        88 , //54
                        89 , //55
                        89 , //56
                        89 , //57
                        90 , //58
                        90 , //59
                        91 , //60
                        91 , //61
                        91 , //62
                        92 , //63
                        92 , //64
                        93 , //65
                        93 , //66
                        93 , //67
                        94 , //68
                        94 , //69
                        94 , //70
                        95 , //71
                        95 , //72
                        95 , //73
                        96 , //74
                        96 , //75
                        96 , //76
                        96 , //77
                        97 , //78
                        97 , //79
                        97 , //80
                        98 , //81
                        98 , //82
                        98 , //83
                        99 , //84
                        99 , //85
                        99 , //86
                        99 , //87
                        100, //88
                        100, //89
                        100, //90
                        100, //91
                        101, //92
                        101, //93
                        101, //94
                        101, //95
                        102, //96
                        102, //97
                        102, //98
                        102, //99
                        103, //100
                        103, //101
                        103, //102
                        103, //103
                        104, //104
                        104, //105
                        104, //106
                        104, //107
                        105, //108
                        105, //109
                        105, //110
                        105, //111
                        106, //112
                        106, //113
                        106, //114
                        107, //115
                        107, //116
                        107, //117
                        107, //118
                        108, //119
                        108, //120
                        109, //121
                        109, //122
                        109, //123
                        110, //124
                        110, //125
                        111, //126
                        111, //127
                    }
                }, //RF_CHAN_9
                {
                    {
                        42 , //0
                        43 , //1
                        45 , //2
                        47 , //3
                        48 , //4
                        50 , //5
                        51 , //6
                        52 , //7
                        54 , //8
                        55 , //9
                        56 , //10
                        57 , //11
                        59 , //12
                        60 , //13
                        61 , //14
                        62 , //15
                        62 , //16
                        63 , //17
                        64 , //18
                        65 , //19
                        66 , //20
                        67 , //21
                        68 , //22
                        69 , //23
                        70 , //24
                        71 , //25
                        72 , //26
                        72 , //27
                        73 , //28
                        74 , //29
                        75 , //30
                        75 , //31
                        76 , //32
                        76 , //33
                        77 , //34
                        78 , //35
                        78 , //36
                        79 , //37
                        79 , //38
                        80 , //39
                        81 , //40
                        81 , //41
                        82 , //42
                        82 , //43
                        83 , //44
                        83 , //45
                        84 , //46
                        84 , //47
                        85 , //48
                        86 , //49
                        86 , //50
                        87 , //51
                        88 , //52
                        88 , //53
                        89 , //54
                        89 , //55
                        90 , //56
                        90 , //57
                        91 , //58
                        91 , //59
                        92 , //60
                        92 , //61
                        92 , //62
                        93 , //63
                        93 , //64
                        94 , //65
                        94 , //66
                        94 , //67
                        95 , //68
                        95 , //69
                        95 , //70
                        96 , //71
                        96 , //72
                        96 , //73
                        97 , //74
                        97 , //75
                        97 , //76
                        98 , //77
                        98 , //78
                        98 , //79
                        98 , //80
                        99 , //81
                        99 , //82
                        99 , //83
                        100, //84
                        100, //85
                        100, //86
                        100, //87
                        101, //88
                        101, //89
                        101, //90
                        101, //91
                        102, //92
                        102, //93
                        102, //94
                        103, //95
                        103, //96
                        103, //97
                        103, //98
                        104, //99
                        104, //100
                        104, //101
                        104, //102
                        105, //103
                        105, //104
                        105, //105
                        105, //106
                        105, //107
                        106, //108
                        106, //109
                        106, //110
                        107, //111
                        107, //112
                        107, //113
                        107, //114
                        108, //115
                        108, //116
                        108, //117
                        109, //118
                        109, //119
                        109, //120
                        110, //121
                        110, //122
                        111, //123
                        111, //124
                        112, //125
                        112, //126
                        112, //127
                    }
                }, //RF_CHAN_10
                {
                    {
                        42 , //0
                        44 , //1
                        46 , //2
                        47 , //3
                        49 , //4
                        50 , //5
                        51 , //6
                        52 , //7
                        53 , //8
                        55 , //9
                        56 , //10
                        57 , //11
                        58 , //12
                        60 , //13
                        61 , //14
                        62 , //15
                        63 , //16
                        64 , //17
                        65 , //18
                        65 , //19
                        66 , //20
                        67 , //21
                        68 , //22
                        69 , //23
                        70 , //24
                        71 , //25
                        71 , //26
                        72 , //27
                        73 , //28
                        74 , //29
                        74 , //30
                        75 , //31
                        76 , //32
                        76 , //33
                        77 , //34
                        78 , //35
                        78 , //36
                        79 , //37
                        79 , //38
                        80 , //39
                        80 , //40
                        81 , //41
                        82 , //42
                        82 , //43
                        83 , //44
                        83 , //45
                        84 , //46
                        85 , //47
                        85 , //48
                        86 , //49
                        86 , //50
                        87 , //51
                        87 , //52
                        88 , //53
                        88 , //54
                        89 , //55
                        90 , //56
                        90 , //57
                        91 , //58
                        91 , //59
                        91 , //60
                        92 , //61
                        92 , //62
                        93 , //63
                        93 , //64
                        93 , //65
                        94 , //66
                        94 , //67
                        94 , //68
                        95 , //69
                        95 , //70
                        95 , //71
                        96 , //72
                        96 , //73
                        96 , //74
                        96 , //75
                        97 , //76
                        97 , //77
                        97 , //78
                        98 , //79
                        98 , //80
                        98 , //81
                        98 , //82
                        99 , //83
                        99 , //84
                        99 , //85
                        99 , //86
                        100, //87
                        100, //88
                        100, //89
                        100, //90
                        101, //91
                        101, //92
                        101, //93
                        101, //94
                        102, //95
                        102, //96
                        102, //97
                        102, //98
                        103, //99
                        103, //100
                        103, //101
                        103, //102
                        104, //103
                        104, //104
                        104, //105
                        104, //106
                        105, //107
                        105, //108
                        105, //109
                        106, //110
                        106, //111
                        106, //112
                        106, //113
                        107, //114
                        107, //115
                        107, //116
                        108, //117
                        108, //118
                        108, //119
                        109, //120
                        109, //121
                        109, //122
                        110, //123
                        110, //124
                        110, //125
                        111, //126
                        111, //127
                    }
                }, //RF_CHAN_11
                {
                    {
                        43 , //0
                        44 , //1
                        46 , //2
                        48 , //3
                        49 , //4
                        51 , //5
                        52 , //6
                        53 , //7
                        54 , //8
                        56 , //9
                        57 , //10
                        58 , //11
                        59 , //12
                        60 , //13
                        61 , //14
                        62 , //15
                        63 , //16
                        64 , //17
                        65 , //18
                        66 , //19
                        67 , //20
                        67 , //21
                        68 , //22
                        69 , //23
                        70 , //24
                        71 , //25
                        72 , //26
                        72 , //27
                        73 , //28
                        74 , //29
                        74 , //30
                        75 , //31
                        76 , //32
                        76 , //33
                        77 , //34
                        77 , //35
                        78 , //36
                        79 , //37
                        79 , //38
                        80 , //39
                        80 , //40
                        81 , //41
                        82 , //42
                        82 , //43
                        83 , //44
                        83 , //45
                        84 , //46
                        84 , //47
                        85 , //48
                        85 , //49
                        86 , //50
                        86 , //51
                        87 , //52
                        87 , //53
                        88 , //54
                        88 , //55
                        89 , //56
                        89 , //57
                        90 , //58
                        90 , //59
                        90 , //60
                        91 , //61
                        91 , //62
                        92 , //63
                        92 , //64
                        92 , //65
                        93 , //66
                        93 , //67
                        93 , //68
                        94 , //69
                        94 , //70
                        94 , //71
                        95 , //72
                        95 , //73
                        95 , //74
                        96 , //75
                        96 , //76
                        96 , //77
                        97 , //78
                        97 , //79
                        97 , //80
                        97 , //81
                        98 , //82
                        98 , //83
                        98 , //84
                        99 , //85
                        99 , //86
                        99 , //87
                        99 , //88
                        100, //89
                        100, //90
                        100, //91
                        100, //92
                        101, //93
                        101, //94
                        101, //95
                        101, //96
                        102, //97
                        102, //98
                        102, //99
                        102, //100
                        103, //101
                        103, //102
                        103, //103
                        103, //104
                        104, //105
                        104, //106
                        104, //107
                        104, //108
                        105, //109
                        105, //110
                        105, //111
                        105, //112
                        106, //113
                        106, //114
                        106, //115
                        106, //116
                        107, //117
                        107, //118
                        107, //119
                        108, //120
                        108, //121
                        109, //122
                        109, //123
                        109, //124
                        110, //125
                        110, //126
                        111, //127
                    }
                }, //RF_CHAN_12
                {
                    {
                        43 , //0
                        44 , //1
                        46 , //2
                        47 , //3
                        49 , //4
                        50 , //5
                        51 , //6
                        53 , //7
                        54 , //8
                        55 , //9
                        56 , //10
                        57 , //11
                        58 , //12
                        59 , //13
                        60 , //14
                        61 , //15
                        62 , //16
                        63 , //17
                        64 , //18
                        65 , //19
                        66 , //20
                        67 , //21
                        68 , //22
                        69 , //23
                        70 , //24
                        70 , //25
                        71 , //26
                        72 , //27
                        72 , //28
                        73 , //29
                        74 , //30
                        74 , //31
                        75 , //32
                        76 , //33
                        76 , //34
                        77 , //35
                        78 , //36
                        78 , //37
                        79 , //38
                        79 , //39
                        80 , //40
                        81 , //41
                        81 , //42
                        82 , //43
                        82 , //44
                        83 , //45
                        83 , //46
                        84 , //47
                        84 , //48
                        84 , //49
                        85 , //50
                        86 , //51
                        86 , //52
                        87 , //53
                        87 , //54
                        88 , //55
                        89 , //56
                        89 , //57
                        90 , //58
                        90 , //59
                        91 , //60
                        91 , //61
                        91 , //62
                        92 , //63
                        92 , //64
                        93 , //65
                        93 , //66
                        93 , //67
                        94 , //68
                        94 , //69
                        94 , //70
                        95 , //71
                        95 , //72
                        95 , //73
                        96 , //74
                        96 , //75
                        96 , //76
                        97 , //77
                        97 , //78
                        97 , //79
                        97 , //80
                        98 , //81
                        98 , //82
                        98 , //83
                        99 , //84
                        99 , //85
                        99 , //86
                        99 , //87
                        100, //88
                        100, //89
                        100, //90
                        100, //91
                        101, //92
                        101, //93
                        101, //94
                        101, //95
                        102, //96
                        102, //97
                        102, //98
                        102, //99
                        103, //100
                        103, //101
                        103, //102
                        103, //103
                        104, //104
                        104, //105
                        104, //106
                        104, //107
                        105, //108
                        105, //109
                        105, //110
                        105, //111
                        106, //112
                        106, //113
                        106, //114
                        107, //115
                        107, //116
                        107, //117
                        107, //118
                        108, //119
                        108, //120
                        108, //121
                        109, //122
                        109, //123
                        110, //124
                        110, //125
                        110, //126
                        111, //127
                    }
                }, //RF_CHAN_13
                {
                    {
                        42,  //0
                        44,  //1
                        45,  //2
                        47,  //3
                        48,  //4
                        49,  //5
                        51,  //6
                        52,  //7
                        53,  //8
                        54,  //9
                        55,  //10
                        56,  //11
                        57,  //12
                        58,  //13
                        59,  //14
                        60,  //15
                        61,  //16
                        62,  //17
                        63,  //18
                        64,  //19
                        64,  //20
                        65,  //21
                        66,  //22
                        67,  //23
                        68,  //24
                        69,  //25
                        69,  //26
                        70,  //27
                        71,  //28
                        71,  //29
                        72,  //30
                        73,  //31
                        73,  //32
                        74,  //33
                        75,  //34
                        75,  //35
                        76,  //36
                        76,  //37
                        77,  //38
                        77,  //39
                        78,  //40
                        78,  //41
                        79,  //42
                        79,  //43
                        80,  //44
                        80,  //45
                        81,  //46
                        82,  //47
                        82,  //48
                        83,  //49
                        83,  //50
                        84,  //51
                        84,  //52
                        85,  //53
                        85,  //54
                        86,  //55
                        86,  //56
                        87,  //57
                        87,  //58
                        88,  //59
                        88,  //60
                        89,  //61
                        89,  //62
                        89,  //63
                        90,  //64
                        90,  //65
                        90,  //66
                        91,  //67
                        91,  //68
                        91,  //69
                        92,  //70
                        92,  //71
                        93,  //72
                        93,  //73
                        93,  //74
                        93,  //75
                        94,  //76
                        94,  //77
                        94,  //78
                        95,  //79
                        95,  //80
                        95,  //81
                        95,  //82
                        96,  //83
                        96,  //84
                        96,  //85
                        97,  //86
                        97,  //87
                        97,  //88
                        97,  //89
                        98,  //90
                        98,  //91
                        98,  //92
                        98,  //93
                        99,  //94
                        99,  //95
                        99,  //96
                        99,  //97
                        100, //98
                        100, //99
                        100, //100
                        100, //101
                        101, //102
                        101, //103
                        101, //104
                        102, //105
                        102, //106
                        102, //107
                        102, //108
                        103, //109
                        103, //110
                        103, //111
                        103, //112
                        104, //113
                        104, //114
                        104, //115
                        104, //116
                        105, //117
                        105, //118
                        105, //119
                        106, //120
                        106, //121
                        107, //122
                        107, //123
                        107, //124
                        108, //125
                        108, //126
                        108, //127
                    }
                }, //RF_CHAN_14
        },

        //NV_TABLE_TPC_PDADC_OFFSETS
        {
            147,  // RF_CHAN_1
            147,  // RF_CHAN_2
            147,  // RF_CHAN_3
            144,  // RF_CHAN_4
            143,  // RF_CHAN_5
            144,  // RF_CHAN_6
            146,  // RF_CHAN_7
            140,  // RF_CHAN_8
            138,  // RF_CHAN_9
            143,  // RF_CHAN_10
            149,  // RF_CHAN_11
            152,  // RF_CHAN_12
            148,  // RF_CHAN_13
            164   // RF_CHAN_14
        },
#if 0
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
        },
#endif
        //NV_TABLE_RSSI_CHANNEL_OFFSETS
        {
            //PHY_RX_CHAIN_0
            {
                //bRssiOffset
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

                //gnRssiOffset
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            },
            //rsvd
            {
                //bRssiOffset
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

                //gnRssiOffset
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
            }
        },

        //NV_TABLE_RF_CAL_VALUES
        {
            //typedef struct
            //{
            //    tANI_U32 calStatus;  //use eNvCalID
            //    sCalData calData;
            //}sRFCalValues;

            0,               //calStatus
            {
                0,           //process_monitor;
                0,           //hdet_cal_code;
                0,           //rxfe_gm_2;

                0,           //tx_bbf_rtune;
                0,           //pa_rtune_reg;
                0,           //rt_code;
                0,           //bias_rtune;

                0,           //bb_bw1;
                0,           //bb_bw2;
                0,           //reserved0;
                0,           //reserved11;

                0,           //bb_bw3;
                0,           //bb_bw4;
                0,           //bb_bw5;
                0,           //bb_bw6;

                0,           //rcMeasured;
                0,           //tx_bbf_ct;
                0,           //tx_bbf_ctr;

                0,           //csh_maxgain_reg;
                0,           //csh_0db_reg;
                0,           //csh_m3db_reg;
                0,           //csh_m6db_reg;

                0,           //cff_0db_reg;
                0,           //cff_m3db_reg;
                0,           //cff_m6db_reg;
                0,           //rxfe_gpio_ctl_1;

                0,           //mix_bal_cnt_2;
                0,           //rxfe_lna_highgain_bias_ctl_delta;
                0,           //rxfe_lna_load_ctune;
                0,           //rxfe_lna_ngm_rtune;

                0,           //rx_im2_spare0;
                0,           //rx_im2_spare1;
                0,           //hdet_dco

                0,           //pll_vfc_reg3_b0;
                0,           //pll_vfc_reg3_b1;
                0,           //pll_vfc_reg3_b2;
                0,           //pll_vfc_reg3_b3;

                0,           //tempStart;
                0            //roomTemp;
            }
        },

        //NV_TABLE_ANTENNA_PATH_LOSS
        {
            280,  // RF_CHAN_1
            270,  // RF_CHAN_2
            270,  // RF_CHAN_3
            270,  // RF_CHAN_4
            270,  // RF_CHAN_5
            270,  // RF_CHAN_6
            280,  // RF_CHAN_7
            280,  // RF_CHAN_8
            290,  // RF_CHAN_9
            300,  // RF_CHAN_10
            300,  // RF_CHAN_11
            310,  // RF_CHAN_12
            310,  // RF_CHAN_13
            310   // RF_CHAN_14
        },

        //NV_TABLE_PACKET_TYPE_POWER_LIMITS
        {
            {
                2150,  // RF_CHAN_1
                2150,  // RF_CHAN_2
                2150,  // RF_CHAN_3
                2150,  // RF_CHAN_4
                2150,  // RF_CHAN_5
                2150,  // RF_CHAN_6
                2150,  // RF_CHAN_7
                2150,  // RF_CHAN_8
                2150,  // RF_CHAN_9
                2150,  // RF_CHAN_10
                2150,  // RF_CHAN_11
                2150,  // RF_CHAN_12
                2150,  // RF_CHAN_13
                2150   // RF_CHAN_14
            },//MODE_802_11B

            {
                1850,  // RF_CHAN_1
                1950,  // RF_CHAN_2
                1950,  // RF_CHAN_3
                1950,  // RF_CHAN_4
                1950,  // RF_CHAN_5
                1950,  // RF_CHAN_6
                1950,  // RF_CHAN_7
                1950,  // RF_CHAN_8
                1950,  // RF_CHAN_9
                1950,  // RF_CHAN_10
                1750,  // RF_CHAN_11
                1950,  // RF_CHAN_12
                1950,  // RF_CHAN_13
                1950   // RF_CHAN_14
            },//MODE_802_11AG

            {
                1750,  // RF_CHAN_1
                1750,  // RF_CHAN_2
                1750,  // RF_CHAN_3
                1750,  // RF_CHAN_4
                1750,  // RF_CHAN_5
                1750,  // RF_CHAN_6
                1750,  // RF_CHAN_7
                1750,  // RF_CHAN_8
                1750,  // RF_CHAN_9
                1750,  // RF_CHAN_10
                1750,  // RF_CHAN_11
                1750,  // RF_CHAN_12
                1750,  // RF_CHAN_13
                1750   // RF_CHAN_14
            },//MODE_802_11AG
        }

    } // tables
};


#endif

