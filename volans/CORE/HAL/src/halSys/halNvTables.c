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
        1,                                                              // tANI_U8   wlanNvRevId; //0: WCN1312, 1: WCN1314
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
                {1800},    // HAL_PHY_RATE_11B_LONG_1_MBPS,
                {1800},    // HAL_PHY_RATE_11B_LONG_2_MBPS,
                {1800},    // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
                {1800},    // HAL_PHY_RATE_11B_LONG_11_MBPS,
                {1800},    // HAL_PHY_RATE_11B_SHORT_2_MBPS,
                {1800},    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
                {1800},    // HAL_PHY_RATE_11B_SHORT_11_MBPS,

                    //SLR Rates
                {1800},    // HAL_PHY_RATE_SLR_0_25_MBPS,
                {1800},    // HAL_PHY_RATE_SLR_0_5_MBPS,

                    //11A 20MHz Rates
                {1700},    // HAL_PHY_RATE_11A_6_MBPS,
                {1700},    // HAL_PHY_RATE_11A_9_MBPS,
                {1700},    // HAL_PHY_RATE_11A_12_MBPS,
                {1700},    // HAL_PHY_RATE_11A_18_MBPS,
                {1600},    // HAL_PHY_RATE_11A_24_MBPS,
                {1600},    // HAL_PHY_RATE_11A_36_MBPS,
                {1500},    // HAL_PHY_RATE_11A_48_MBPS,
                {1500},    // HAL_PHY_RATE_11A_54_MBPS,

                    //MCS Index #0-15 (20MHz)
                {1500},    // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
                {1400},    // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
                {1300},    // HAL_PHY_RATE_MCS_1NSS_65_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
                {1500},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
                {1400},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
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
                     0, //0
                    19, //1
                    21, //2
                    23, //3
                    25, //4
                    27, //5
                    29, //6
                    31, //7
                    33, //8
                    34, //9
                    36, //10
                    37, //11
                    38, //12
                    40, //13
                    41, //14
                    42, //15
                    43, //16
                    45, //17
                    46, //18
                    47, //19
                    48, //20
                    49, //21
                    50, //22
                    51, //23
                    52, //24
                    53, //25
                    53, //26
                    54, //27
                    55, //28
                    56, //29
                    56, //30
                    57, //31
                    58, //32
                    59, //33
                    60, //34
                    60, //35
                    61, //36
                    61, //37
                    62, //38
                    63, //39
                    64, //40
                    64, //41
                    65, //42
                    66, //43
                    67, //44
                    67, //45
                    68, //46
                    69, //47
                    69, //48
                    70, //49
                    70, //50
                    71, //51
                    71, //52
                    72, //53
                    72, //54
                    73, //55
                    73, //56
                    73, //57
                    74, //58
                    74, //59
                    75, //60
                    75, //61
                    76, //62
                    76, //63
                    76, //64
                    77, //65
                    77, //66
                    77, //67
                    78, //68
                    78, //69
                    78, //70
                    79, //71
                    79, //72
                    80, //73
                    80, //74
                    80, //75
                    80, //76
                    81, //77
                    81, //78
                    81, //79
                    81, //80
                    82, //81
                    82, //82
                    82, //83
                    82, //84
                    83, //85
                    83, //86
                    83, //87
                    83, //88
                    83, //89
                    84, //90
                    84, //91
                    84, //92
                    84, //93
                    84, //94
                    84, //95
                    84, //96
                    84, //97
                    84, //98
                    84, //99
                    84, //100
                    84, //101
                    84, //102
                    84, //103
                    84, //104
                    84, //105
                    85, //106
                    85, //107
                    85, //108
                    85, //109
                    85, //110
                    85, //111
                    85, //112
                    85, //113
                    85, //114
                    85, //115
                    85, //116
                    85, //117
                    85, //118
                    85, //119
                    85, //120
                    85, //121
                    85, //122
                    85, //123
                    85, //124
                    85, //125
                    85, //126
                    85, //127
                }
            }, //RF_CHAN_1
            {
                {
                     0, //0
                    19, //1
                    21, //2
                    23, //3
                    25, //4
                    27, //5
                    29, //6
                    30, //7
                    32, //8
                    34, //9
                    35, //10
                    37, //11
                    38, //12
                    39, //13
                    40, //14
                    42, //15
                    43, //16
                    44, //17
                    45, //18
                    46, //19
                    47, //20
                    48, //21
                    49, //22
                    50, //23
                    51, //24
                    52, //25
                    53, //26
                    53, //27
                    54, //28
                    55, //29
                    56, //30
                    57, //31
                    57, //32
                    58, //33
                    59, //34
                    60, //35
                    60, //36
                    61, //37
                    62, //38
                    63, //39
                    63, //40
                    64, //41
                    65, //42
                    66, //43
                    66, //44
                    67, //45
                    67, //46
                    68, //47
                    69, //48
                    69, //49
                    70, //50
                    70, //51
                    71, //52
                    71, //53
                    72, //54
                    72, //55
                    73, //56
                    73, //57
                    74, //58
                    74, //59
                    74, //60
                    75, //61
                    75, //62
                    76, //63
                    76, //64
                    76, //65
                    77, //66
                    77, //67
                    77, //68
                    78, //69
                    78, //70
                    79, //71
                    79, //72
                    79, //73
                    79, //74
                    80, //75
                    80, //76
                    80, //77
                    81, //78
                    81, //79
                    81, //80
                    82, //81
                    82, //82
                    82, //83
                    82, //84
                    83, //85
                    83, //86
                    83, //87
                    83, //88
                    83, //89
                    84, //90
                    84, //91
                    84, //92
                    84, //93
                    84, //94
                    84, //95
                    84, //96
                    84, //97
                    84, //98
                    84, //99
                    84, //100
                    84, //101
                    84, //102
                    85, //103
                    85, //104
                    85, //105
                    85, //106
                    85, //107
                    85, //108
                    85, //109
                    85, //110
                    85, //111
                    85, //112
                    85, //113
                    85, //114
                    85, //115
                    85, //116
                    85, //117
                    85, //118
                    85, //119
                    85, //120
                    85, //121
                    85, //122
                    85, //123
                    85, //124
                    85, //125
                    85, //126
                    85, //127
                }
            }, //RF_CHAN_2
                {
                    {
                         0, //0
                        19, //1
                        21, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        30, //7
                        32, //8
                        33, //9
                        35, //10
                        36, //11
                        38, //12
                        39, //13
                        40, //14
                        42, //15
                        43, //16
                        44, //17
                        45, //18
                        46, //19
                        47, //20
                        48, //21
                        49, //22
                        50, //23
                        51, //24
                        52, //25
                        53, //26
                        53, //27
                        54, //28
                        55, //29
                        56, //30
                        56, //31
                        57, //32
                        58, //33
                        59, //34
                        60, //35
                        60, //36
                        61, //37
                        62, //38
                        62, //39
                        63, //40
                        64, //41
                        65, //42
                        65, //43
                        66, //44
                        67, //45
                        67, //46
                        68, //47
                        68, //48
                        69, //49
                        69, //50
                        70, //51
                        71, //52
                        71, //53
                        72, //54
                        72, //55
                        72, //56
                        73, //57
                        73, //58
                        74, //59
                        74, //60
                        75, //61
                        75, //62
                        75, //63
                        76, //64
                        76, //65
                        77, //66
                        77, //67
                        77, //68
                        78, //69
                        78, //70
                        78, //71
                        79, //72
                        79, //73
                        79, //74
                        80, //75
                        80, //76
                        80, //77
                        81, //78
                        81, //79
                        81, //80
                        81, //81
                        82, //82
                        82, //83
                        82, //84
                        82, //85
                        82, //86
                        83, //87
                        83, //88
                        83, //89
                        83, //90
                        83, //91
                        83, //92
                        83, //93
                        83, //94
                        83, //95
                        83, //96
                        83, //97
                        83, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        84, //112
                        84, //113
                        84, //114
                        84, //115
                        84, //116
                        84, //117
                        84, //118
                        84, //119
                        84, //120
                        84, //121
                        84, //122
                        84, //123
                        84, //124
                        84, //125
                        84, //126
                        84, //127
                    }
                }, //RF_CHAN_3
                {
                    {
                         0, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        27, //5
                        29, //6
                        31, //7
                        33, //8
                        35, //9
                        36, //10
                        38, //11
                        39, //12
                        40, //13
                        41, //14
                        42, //15
                        44, //16
                        45, //17
                        46, //18
                        47, //19
                        48, //20
                        49, //21
                        50, //22
                        51, //23
                        52, //24
                        53, //25
                        53, //26
                        54, //27
                        55, //28
                        56, //29
                        56, //30
                        57, //31
                        58, //32
                        59, //33
                        60, //34
                        60, //35
                        61, //36
                        62, //37
                        62, //38
                        63, //39
                        64, //40
                        65, //41
                        65, //42
                        66, //43
                        66, //44
                        67, //45
                        68, //46
                        68, //47
                        69, //48
                        69, //49
                        70, //50
                        70, //51
                        71, //52
                        71, //53
                        72, //54
                        72, //55
                        73, //56
                        73, //57
                        73, //58
                        74, //59
                        74, //60
                        75, //61
                        75, //62
                        76, //63
                        76, //64
                        76, //65
                        77, //66
                        77, //67
                        77, //68
                        78, //69
                        78, //70
                        78, //71
                        79, //72
                        79, //73
                        79, //74
                        80, //75
                        80, //76
                        80, //77
                        81, //78
                        81, //79
                        81, //80
                        81, //81
                        82, //82
                        82, //83
                        82, //84
                        82, //85
                        82, //86
                        82, //87
                        82, //88
                        82, //89
                        83, //90
                        83, //91
                        83, //92
                        83, //93
                        83, //94
                        83, //95
                        83, //96
                        83, //97
                        83, //98
                        83, //99
                        83, //100
                        83, //101
                        83, //102
                        83, //103
                        83, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        84, //112
                        84, //113
                        84, //114
                        84, //115
                        84, //116
                        84, //117
                        84, //118
                        84, //119
                        84, //120
                        84, //121
                        84, //122
                        84, //123
                        84, //124
                        84, //125
                        84, //126
                        84, //127
                    }
                }, //RF_CHAN_4
                {
                    {
                         0, //0
                        19, //1
                        21, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        31, //7
                        32, //8
                        34, //9
                        35, //10
                        37, //11
                        38, //12
                        40, //13
                        41, //14
                        43, //15
                        44, //16
                        45, //17
                        46, //18
                        47, //19
                        47, //20
                        49, //21
                        50, //22
                        50, //23
                        51, //24
                        52, //25
                        53, //26
                        54, //27
                        55, //28
                        56, //29
                        57, //30
                        57, //31
                        58, //32
                        59, //33
                        60, //34
                        61, //35
                        61, //36
                        62, //37
                        63, //38
                        64, //39
                        65, //40
                        65, //41
                        66, //42
                        67, //43
                        67, //44
                        68, //45
                        69, //46
                        69, //47
                        70, //48
                        70, //49
                        71, //50
                        71, //51
                        72, //52
                        72, //53
                        73, //54
                        73, //55
                        74, //56
                        74, //57
                        74, //58
                        75, //59
                        75, //60
                        76, //61
                        76, //62
                        76, //63
                        77, //64
                        77, //65
                        78, //66
                        78, //67
                        78, //68
                        79, //69
                        79, //70
                        79, //71
                        80, //72
                        80, //73
                        80, //74
                        81, //75
                        81, //76
                        81, //77
                        81, //78
                        82, //79
                        82, //80
                        82, //81
                        82, //82
                        83, //83
                        83, //84
                        83, //85
                        83, //86
                        83, //87
                        83, //88
                        83, //89
                        83, //90
                        83, //91
                        84, //92
                        84, //93
                        84, //94
                        84, //95
                        84, //96
                        84, //97
                        84, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        84, //112
                        84, //113
                        84, //114
                        84, //115
                        84, //116
                        84, //117
                        84, //118
                        84, //119
                        84, //120
                        84, //121
                        84, //122
                        84, //123
                        84, //124
                        84, //125
                        84, //126
                        84, //127
                    }
                }, //RF_CHAN_5
                {
                    {
                         0, //0
                        19, //1
                        21, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        31, //7
                        33, //8
                        34, //9
                        36, //10
                        37, //11
                        39, //12
                        40, //13
                        41, //14
                        42, //15
                        43, //16
                        44, //17
                        46, //18
                        47, //19
                        48, //20
                        49, //21
                        50, //22
                        51, //23
                        52, //24
                        53, //25
                        54, //26
                        55, //27
                        55, //28
                        56, //29
                        57, //30
                        58, //31
                        58, //32
                        59, //33
                        60, //34
                        61, //35
                        61, //36
                        62, //37
                        63, //38
                        64, //39
                        65, //40
                        65, //41
                        66, //42
                        67, //43
                        67, //44
                        68, //45
                        69, //46
                        69, //47
                        70, //48
                        70, //49
                        71, //50
                        71, //51
                        72, //52
                        72, //53
                        73, //54
                        73, //55
                        74, //56
                        74, //57
                        75, //58
                        75, //59
                        76, //60
                        76, //61
                        76, //62
                        77, //63
                        77, //64
                        78, //65
                        78, //66
                        78, //67
                        79, //68
                        79, //69
                        79, //70
                        80, //71
                        80, //72
                        80, //73
                        81, //74
                        81, //75
                        81, //76
                        81, //77
                        82, //78
                        82, //79
                        82, //80
                        82, //81
                        82, //82
                        83, //83
                        83, //84
                        83, //85
                        83, //86
                        83, //87
                        83, //88
                        83, //89
                        84, //90
                        84, //91
                        84, //92
                        84, //93
                        84, //94
                        84, //95
                        84, //96
                        84, //97
                        84, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        85, //111
                        85, //112
                        85, //113
                        85, //114
                        85, //115
                        85, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        85, //121
                        85, //122
                        85, //123
                        85, //124
                        85, //125
                        85, //126
                        85, //127
                    }
                }, //RF_CHAN_6
                {
                    {
                         0, //0
                        19, //1
                        21, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        31, //7
                        33, //8
                        34, //9
                        36, //10
                        38, //11
                        39, //12
                        40, //13
                        41, //14
                        42, //15
                        44, //16
                        45, //17
                        46, //18
                        47, //19
                        48, //20
                        49, //21
                        50, //22
                        51, //23
                        52, //24
                        53, //25
                        54, //26
                        55, //27
                        55, //28
                        56, //29
                        57, //30
                        58, //31
                        58, //32
                        59, //33
                        60, //34
                        61, //35
                        62, //36
                        62, //37
                        63, //38
                        64, //39
                        65, //40
                        65, //41
                        66, //42
                        66, //43
                        67, //44
                        68, //45
                        68, //46
                        69, //47
                        69, //48
                        70, //49
                        70, //50
                        71, //51
                        71, //52
                        72, //53
                        72, //54
                        73, //55
                        73, //56
                        74, //57
                        74, //58
                        75, //59
                        75, //60
                        75, //61
                        76, //62
                        76, //63
                        77, //64
                        77, //65
                        77, //66
                        78, //67
                        78, //68
                        78, //69
                        79, //70
                        79, //71
                        79, //72
                        80, //73
                        80, //74
                        80, //75
                        81, //76
                        81, //77
                        81, //78
                        81, //79
                        82, //80
                        82, //81
                        82, //82
                        82, //83
                        82, //84
                        82, //85
                        83, //86
                        83, //87
                        83, //88
                        83, //89
                        83, //90
                        83, //91
                        83, //92
                        83, //93
                        83, //94
                        83, //95
                        83, //96
                        83, //97
                        84, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        84, //112
                        84, //113
                        84, //114
                        84, //115
                        84, //116
                        84, //117
                        84, //118
                        84, //119
                        84, //120
                        84, //121
                        84, //122
                        84, //123
                        84, //124
                        84, //125
                        84, //126
                        84, //127
                    }
                }, //RF_CHAN_7
                {
                    {
                         0, //0
                        18, //1
                        20, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        30, //7
                        32, //8
                        34, //9
                        36, //10
                        37, //11
                        39, //12
                        40, //13
                        41, //14
                        43, //15
                        44, //16
                        45, //17
                        46, //18
                        47, //19
                        48, //20
                        49, //21
                        50, //22
                        51, //23
                        52, //24
                        53, //25
                        54, //26
                        55, //27
                        55, //28
                        56, //29
                        57, //30
                        58, //31
                        59, //32
                        59, //33
                        60, //34
                        61, //35
                        62, //36
                        63, //37
                        64, //38
                        64, //39
                        65, //40
                        66, //41
                        67, //42
                        67, //43
                        68, //44
                        69, //45
                        69, //46
                        70, //47
                        70, //48
                        71, //49
                        71, //50
                        72, //51
                        72, //52
                        73, //53
                        73, //54
                        74, //55
                        74, //56
                        75, //57
                        75, //58
                        75, //59
                        76, //60
                        76, //61
                        77, //62
                        77, //63
                        77, //64
                        78, //65
                        78, //66
                        78, //67
                        79, //68
                        79, //69
                        80, //70
                        80, //71
                        80, //72
                        81, //73
                        81, //74
                        81, //75
                        82, //76
                        82, //77
                        82, //78
                        82, //79
                        83, //80
                        83, //81
                        83, //82
                        83, //83
                        83, //84
                        83, //85
                        83, //86
                        83, //87
                        84, //88
                        84, //89
                        84, //90
                        84, //91
                        84, //92
                        84, //93
                        84, //94
                        84, //95
                        84, //96
                        84, //97
                        84, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        85, //105
                        85, //106
                        85, //107
                        85, //108
                        85, //109
                        85, //110
                        85, //111
                        85, //112
                        85, //113
                        85, //114
                        85, //115
                        85, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        85, //121
                        85, //122
                        85, //123
                        85, //124
                        85, //125
                        85, //126
                        85, //127
                    }
                }, //RF_CHAN_8
                {
                    {
                         0, //0
                        18, //1
                        21, //2
                        23, //3
                        25, //4
                        27, //5
                        29, //6
                        31, //7
                        33, //8
                        35, //9
                        36, //10
                        37, //11
                        39, //12
                        40, //13
                        41, //14
                        43, //15
                        44, //16
                        45, //17
                        46, //18
                        47, //19
                        48, //20
                        49, //21
                        50, //22
                        51, //23
                        52, //24
                        53, //25
                        54, //26
                        55, //27
                        56, //28
                        56, //29
                        57, //30
                        58, //31
                        59, //32
                        60, //33
                        60, //34
                        61, //35
                        62, //36
                        63, //37
                        64, //38
                        64, //39
                        65, //40
                        66, //41
                        66, //42
                        67, //43
                        68, //44
                        68, //45
                        69, //46
                        69, //47
                        70, //48
                        70, //49
                        71, //50
                        71, //51
                        72, //52
                        72, //53
                        73, //54
                        73, //55
                        74, //56
                        74, //57
                        75, //58
                        75, //59
                        75, //60
                        76, //61
                        76, //62
                        77, //63
                        77, //64
                        77, //65
                        78, //66
                        78, //67
                        78, //68
                        79, //69
                        79, //70
                        80, //71
                        80, //72
                        80, //73
                        81, //74
                        81, //75
                        81, //76
                        81, //77
                        82, //78
                        82, //79
                        82, //80
                        82, //81
                        82, //82
                        82, //83
                        83, //84
                        83, //85
                        83, //86
                        83, //87
                        83, //88
                        83, //89
                        83, //90
                        83, //91
                        83, //92
                        83, //93
                        83, //94
                        83, //95
                        83, //96
                        83, //97
                        83, //98
                        84, //99
                        84, //100
                        84, //101
                        84, //102
                        84, //103
                        84, //104
                        84, //105
                        84, //106
                        84, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        84, //112
                        84, //113
                        84, //114
                        84, //115
                        84, //116
                        84, //117
                        84, //118
                        84, //119
                        84, //120
                        84, //121
                        84, //122
                        84, //123
                        84, //124
                        84, //125
                        84, //126
                        84, //127
                    }
                }, //RF_CHAN_9
                {
                    {
                         0, //0
                        17, //1
                        19, //2
                        23, //3
                        24, //4
                        26, //5
                        28, //6
                        29, //7
                        31, //8
                        33, //9
                        34, //10
                        36, //11
                        38, //12
                        39, //13
                        40, //14
                        41, //15
                        43, //16
                        44, //17
                        45, //18
                        46, //19
                        47, //20
                        48, //21
                        49, //22
                        50, //23
                        51, //24
                        52, //25
                        53, //26
                        53, //27
                        54, //28
                        55, //29
                        56, //30
                        57, //31
                        58, //32
                        59, //33
                        60, //34
                        61, //35
                        61, //36
                        62, //37
                        63, //38
                        64, //39
                        65, //40
                        65, //41
                        66, //42
                        66, //43
                        67, //44
                        68, //45
                        68, //46
                        69, //47
                        70, //48
                        70, //49
                        71, //50
                        71, //51
                        72, //52
                        72, //53
                        72, //54
                        73, //55
                        73, //56
                        74, //57
                        74, //58
                        75, //59
                        75, //60
                        75, //61
                        76, //62
                        76, //63
                        77, //64
                        77, //65
                        78, //66
                        78, //67
                        78, //68
                        79, //69
                        79, //70
                        79, //71
                        80, //72
                        80, //73
                        80, //74
                        81, //75
                        81, //76
                        81, //77
                        81, //78
                        81, //79
                        82, //80
                        82, //81
                        82, //82
                        82, //83
                        82, //84
                        82, //85
                        82, //86
                        82, //87
                        82, //88
                        83, //89
                        83, //90
                        83, //91
                        83, //92
                        83, //93
                        83, //94
                        83, //95
                        83, //96
                        83, //97
                        83, //98
                        83, //99
                        83, //100
                        83, //101
                        83, //102
                        83, //103
                        83, //104
                        83, //105
                        83, //106
                        83, //107
                        83, //108
                        83, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        83, //114
                        83, //115
                        83, //116
                        83, //117
                        83, //118
                        83, //119
                        83, //120
                        83, //121
                        83, //122
                        83, //123
                        83, //124
                        83, //125
                        83, //126
                        83, //127
                    }
                }, //RF_CHAN_10
                {
                    {
                         0, //0
                        18, //1
                        20, //2
                        23, //3
                        25, //4
                        27, //5
                        28, //6
                        30, //7
                        31, //8
                        33, //9
                        34, //10
                        36, //11
                        37, //12
                        39, //13
                        40, //14
                        41, //15
                        42, //16
                        43, //17
                        45, //18
                        46, //19
                        47, //20
                        48, //21
                        49, //22
                        50, //23
                        51, //24
                        52, //25
                        53, //26
                        53, //27
                        54, //28
                        55, //29
                        56, //30
                        57, //31
                        58, //32
                        58, //33
                        59, //34
                        60, //35
                        61, //36
                        62, //37
                        62, //38
                        63, //39
                        64, //40
                        65, //41
                        66, //42
                        66, //43
                        67, //44
                        67, //45
                        68, //46
                        68, //47
                        69, //48
                        69, //49
                        70, //50
                        70, //51
                        71, //52
                        72, //53
                        72, //54
                        73, //55
                        73, //56
                        73, //57
                        74, //58
                        74, //59
                        75, //60
                        75, //61
                        76, //62
                        76, //63
                        76, //64
                        77, //65
                        77, //66
                        77, //67
                        78, //68
                        78, //69
                        78, //70
                        79, //71
                        79, //72
                        79, //73
                        79, //74
                        79, //75
                        79, //76
                        80, //77
                        80, //78
                        80, //79
                        80, //80
                        80, //81
                        80, //82
                        80, //83
                        80, //84
                        80, //85
                        80, //86
                        80, //87
                        81, //88
                        81, //89
                        81, //90
                        81, //91
                        81, //92
                        81, //93
                        81, //94
                        81, //95
                        81, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        81, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        81, //106
                        81, //107
                        81, //108
                        81, //109
                        81, //110
                        81, //111
                        81, //112
                        81, //113
                        81, //114
                        81, //115
                        81, //116
                        81, //117
                        81, //118
                        81, //119
                        81, //120
                        81, //121
                        81, //122
                        81, //123
                        81, //124
                        81, //125
                        81, //126
                        81, //127
                    }
                }, //RF_CHAN_11
                {
                    {
                         0, //0
                        16, //1
                        19, //2
                        21, //3
                        23, //4
                        25, //5
                        27, //6
                        29, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        38, //13
                        39, //14
                        40, //15
                        41, //16
                        42, //17
                        44, //18
                        45, //19
                        46, //20
                        47, //21
                        48, //22
                        49, //23
                        50, //24
                        51, //25
                        52, //26
                        52, //27
                        53, //28
                        54, //29
                        55, //30
                        55, //31
                        56, //32
                        57, //33
                        58, //34
                        58, //35
                        59, //36
                        60, //37
                        61, //38
                        61, //39
                        62, //40
                        63, //41
                        63, //42
                        64, //43
                        65, //44
                        65, //45
                        66, //46
                        67, //47
                        67, //48
                        68, //49
                        68, //50
                        69, //51
                        69, //52
                        70, //53
                        70, //54
                        71, //55
                        71, //56
                        71, //57
                        72, //58
                        72, //59
                        73, //60
                        73, //61
                        74, //62
                        74, //63
                        74, //64
                        75, //65
                        75, //66
                        75, //67
                        76, //68
                        76, //69
                        76, //70
                        77, //71
                        77, //72
                        77, //73
                        78, //74
                        78, //75
                        78, //76
                        78, //77
                        78, //78
                        78, //79
                        78, //80
                        79, //81
                        79, //82
                        79, //83
                        79, //84
                        79, //85
                        79, //86
                        79, //87
                        79, //88
                        79, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        80, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        80, //102
                        80, //103
                        80, //104
                        80, //105
                        80, //106
                        80, //107
                        80, //108
                        80, //109
                        80, //110
                        80, //111
                        80, //112
                        80, //113
                        80, //114
                        80, //115
                        80, //116
                        80, //117
                        80, //118
                        80, //119
                        80, //120
                        80, //121
                        80, //122
                        80, //123
                        80, //124
                        80, //125
                        80, //126
                        80, //127
                    }
                }, //RF_CHAN_12
                {
                    {
                         0, //0
                        17, //1
                        19, //2
                        21, //3
                        23, //4
                        25, //5
                        27, //6
                        29, //7
                        31, //8
                        33, //9
                        34, //10
                        35, //11
                        36, //12
                        38, //13
                        39, //14
                        40, //15
                        41, //16
                        43, //17
                        44, //18
                        45, //19
                        46, //20
                        47, //21
                        48, //22
                        49, //23
                        50, //24
                        50, //25
                        51, //26
                        52, //27
                        53, //28
                        54, //29
                        55, //30
                        55, //31
                        56, //32
                        57, //33
                        58, //34
                        58, //35
                        59, //36
                        60, //37
                        61, //38
                        61, //39
                        62, //40
                        63, //41
                        64, //42
                        64, //43
                        65, //44
                        66, //45
                        66, //46
                        67, //47
                        68, //48
                        68, //49
                        69, //50
                        69, //51
                        70, //52
                        70, //53
                        71, //54
                        71, //55
                        72, //56
                        72, //57
                        72, //58
                        73, //59
                        73, //60
                        74, //61
                        74, //62
                        75, //63
                        75, //64
                        75, //65
                        76, //66
                        76, //67
                        76, //68
                        77, //69
                        77, //70
                        77, //71
                        78, //72
                        78, //73
                        78, //74
                        78, //75
                        78, //76
                        78, //77
                        79, //78
                        79, //79
                        79, //80
                        79, //81
                        79, //82
                        79, //83
                        79, //84
                        79, //85
                        79, //86
                        79, //87
                        79, //88
                        79, //89
                        80, //90
                        80, //91
                        80, //92
                        80, //93
                        80, //94
                        80, //95
                        80, //96
                        80, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        80, //102
                        80, //103
                        80, //104
                        80, //105
                        80, //106
                        80, //107
                        80, //108
                        80, //109
                        80, //110
                        80, //111
                        80, //112
                        80, //113
                        80, //114
                        80, //115
                        80, //116
                        80, //117
                        80, //118
                        80, //119
                        80, //120
                        80, //121
                        80, //122
                        80, //123
                        80, //124
                        80, //125
                        80, //126
                        80, //127
                    }
                }, //RF_CHAN_13
                {
                    {
                        0, //0
                       15, //1
                       18, //2
                       20, //3
                       22, //4
                       24, //5
                       25, //6
                       27, //7
                       29, //8
                       31, //9
                       32, //10
                       33, //11
                       34, //12
                       35, //13
                       37, //14
                       38, //15
                       39, //16
                       40, //17
                       41, //18
                       42, //19
                       43, //20
                       44, //21
                       45, //22
                       46, //23
                       47, //24
                       48, //25
                       49, //26
                       49, //27
                       50, //28
                       51, //29
                       52, //30
                       53, //31
                       53, //32
                       54, //33
                       55, //34
                       56, //35
                       57, //36
                       58, //37
                       58, //38
                       59, //39
                       60, //40
                       60, //41
                       61, //42
                       62, //43
                       62, //44
                       63, //45
                       64, //46
                       64, //47
                       65, //48
                       65, //49
                       66, //50
                       66, //51
                       67, //52
                       67, //53
                       68, //54
                       68, //55
                       69, //56
                       69, //57
                       70, //58
                       70, //59
                       71, //60
                       71, //61
                       71, //62
                       72, //63
                       72, //64
                       72, //65
                       73, //66
                       73, //67
                       73, //68
                       73, //69
                       74, //70
                       74, //71
                       74, //72
                       74, //73
                       74, //74
                       74, //75
                       74, //76
                       74, //77
                       74, //78
                       74, //79
                       75,  //80
                       75,  //81
                       75,  //82
                       75,  //83
                       75,  //84
                       75,  //85
                       75,  //86
                       75,  //87
                       75,  //88
                       75,  //89
                       75,  //90
                       75,  //91
                       75,  //92
                       75,  //93
                       75,  //94
                       75,  //95
                       75,  //96
                       75,  //97
                       75, //98
                       75, //99
                       75, //100
                       75, //101
                       75, //102
                       75, //103
                       75, //104
                       75, //105
                       75, //106
                       75, //107
                       75, //108
                       75, //109
                       75, //110
                       75, //111
                       75, //112
                       75, //113
                       75, //114
                       75, //115
                       75, //116
                       75, //117
                       75, //118
                       75, //119
                       75, //120
                       75, //121
                       75, //122
                       75, //123
                       75, //124
                       75, //125
                       75, //126
                       75, //127
                    }
                }, //RF_CHAN_14
        },

        //NV_TABLE_TPC_PDADC_OFFSETS
        {
            98,  // RF_CHAN_1
            101,  // RF_CHAN_2
            101,  // RF_CHAN_3
            100,  // RF_CHAN_4
            98,  // RF_CHAN_5
            97,  // RF_CHAN_6
            94,  // RF_CHAN_7
            94,  // RF_CHAN_8
            92,  // RF_CHAN_9
            90,  // RF_CHAN_10
            94,  // RF_CHAN_11
            95,  // RF_CHAN_12
            97,  // RF_CHAN_13
            104   // RF_CHAN_14
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
                            { 0x00, 0x00 }, //CAL POINT_2
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
                {300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300},

                //gnRssiOffset
                {300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300}
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
                0,           //rxfe_gm_2;
                0,           //hdet_cal_code;
                0,           //process_monitor;

                0,           //bias_rtune;
                0,           //rt_code;
                0,           //pa_rtune_reg;
                0,           //tx_bbf_rtune;

                0,           //reserved1;
                0,           //pa_ctune_reg;
                0,           //bb_bw2;
                0,           //bb_bw1;

                0,           //bb_bw6;
                0,           //bb_bw5;
                0,           //bb_bw4;
                0,           //bb_bw3;

                0,           //tx_bbf_ctr;
                0,           //tx_bbf_ct;
                0,           //rcMeasured;

                0,           //csh_m6db_reg;
                0,           //csh_m3db_reg;
                0,           //csh_0db_reg;
                0,           //csh_maxgain_reg;

                0,           //rxfe_gpio_ctl_1;
                0,           //cff_m6db_reg;
                0,           //cff_m3db_reg;
                0,           //cff_0db_reg;

                0,           //rxfe_lna_ngm_rtune;
                0,           //rxfe_lna_load_ctune;
                0,           //rxfe_lna_highgain_bias_ctl_delta;
                0,           //mix_bal_cnt_2;

                0,           //hdet_dco
                0,           //rx_im2_spare1;
                0,           //rx_im2_spare0;

                0,           //pll_vfc_reg3_b3;
                0,           //pll_vfc_reg3_b2;
                0,           //pll_vfc_reg3_b1;
                0,           //pll_vfc_reg3_b0;

                0,           //roomTemp;
                0,           //tempStart;

                0,           //reserved2
                0,           //Ambient Cal Temp Valid
                2500         //Ambient Cal Temp
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
                1900,  // RF_CHAN_1
                1900,  // RF_CHAN_2
                1900,  // RF_CHAN_3
                1900,  // RF_CHAN_4
                1900,  // RF_CHAN_5
                1900,  // RF_CHAN_6
                1900,  // RF_CHAN_7
                1900,  // RF_CHAN_8
                1900,  // RF_CHAN_9
                1900,  // RF_CHAN_10
                1900,  // RF_CHAN_11
                1900,  // RF_CHAN_12
                1900,  // RF_CHAN_13
                1900   // RF_CHAN_14
            },//MODE_802_11B

            {
                1600,  // RF_CHAN_1
                1700,  // RF_CHAN_2
                1700,  // RF_CHAN_3
                1700,  // RF_CHAN_4
                1700,  // RF_CHAN_5
                1700,  // RF_CHAN_6
                1700,  // RF_CHAN_7
                1700,  // RF_CHAN_8
                1700,  // RF_CHAN_9
                1700,  // RF_CHAN_10
                1500,  // RF_CHAN_11
                1700,  // RF_CHAN_12
                1700,  // RF_CHAN_13
                1700   // RF_CHAN_14
            },//MODE_802_11AG

            {
                1500,  // RF_CHAN_1
                1500,  // RF_CHAN_2
                1500,  // RF_CHAN_3
                1500,  // RF_CHAN_4
                1500,  // RF_CHAN_5
                1500,  // RF_CHAN_6
                1500,  // RF_CHAN_7
                1500,  // RF_CHAN_8
                1500,  // RF_CHAN_9
                1500,  // RF_CHAN_10
                1500,  // RF_CHAN_11
                1500,  // RF_CHAN_12
                1500,  // RF_CHAN_13
                1500   // RF_CHAN_14
            },//MODE_802_11N
        },

        //NV_TABLE_OFDM_CMD_PWR_OFFSET
        {
            0, 0
        },

        //NV_TABLE_TX_BB_FILTER_MODE
        {
            0
        },
         //NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY
        {
            0  //3.2Mhz
        },
         //NV_TABLE_XO_WARMUP_US		
        3000



    } // tables
};


#endif

