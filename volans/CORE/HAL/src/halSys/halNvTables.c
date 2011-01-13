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
                    19, //0
                    21, //1
                    23, //2
                    24, //3
                    25, //4
                    26, //5
                    28, //6
                    29, //7
                    30, //8
                    31, //9
                    33, //10
                    34, //11
                    36, //12
                    37, //13
                    38, //14
                    39, //15
                    40, //16
                    41, //17
                    41, //18
                    42, //19
                    43, //20
                    44, //21
                    45, //22
                    46, //23
                    47, //24
                    48, //25
                    48, //26
                    49, //27
                    50, //28
                    50, //29
                    51, //30
                    52, //31
                    53, //32
                    53, //33
                    54, //34
                    54, //35
                    55, //36
                    56, //37
                    56, //38
                    57, //39
                    58, //40
                    58, //41
                    59, //42
                    59, //43
                    60, //44
                    61, //45
                    61, //46
                    62, //47
                    62, //48
                    63, //49
                    63, //50
                    64, //51
                    64, //52
                    65, //53
                    65, //54
                    65, //55
                    66, //56
                    66, //57
                    67, //58
                    67, //59
                    67, //60
                    68, //61
                    68, //62
                    68, //63
                    69, //64
                    69, //65
                    69, //66
                    70, //67
                    70, //68
                    70, //69
                    71, //70
                    71, //71
                    71, //72
                    72, //73
                    72, //74
                    73, //75
                    73, //76
                    73, //77
                    74, //78
                    74, //79
                    74, //80
                    74, //81
                    75, //82
                    75, //83
                    75, //84
                    76, //85
                    76, //86
                    76, //87
                    76, //88
                    77, //89
                    77, //90
                    77, //91
                    77, //92
                    78, //93
                    78, //94
                    78, //95
                    78, //96
                    79, //97
                    79, //98
                    79, //99
                    79, //100
                    80, //101
                    80, //102
                    80, //103
                    80, //104
                    81, //105
                    81, //106
                    81, //107
                    81, //108
                    82, //109
                    82, //110
                    82, //111
                    82, //112
                    83, //113
                    83, //114
                    83, //115
                    84, //116
                    84, //117
                    84, //118
                    85, //119
                    85, //120
                    85, //121
                    86, //122
                    86, //123
                    87, //124
                    87, //125
                    87, //126
                    88, //127
                }
            }, //RF_CHAN_1
            {
                {
                    19, //0
                    21, //1
                    23, //2
                    24, //3
                    25, //4
                    26, //5
                    28, //6
                    29, //7
                    30, //8
                    31, //9
                    33, //10
                    34, //11
                    36, //12
                    37, //13
                    38, //14
                    39, //15
                    40, //16
                    41, //17
                    41, //18
                    42, //19
                    43, //20
                    44, //21
                    45, //22
                    46, //23
                    47, //24
                    48, //25
                    48, //26
                    49, //27
                    50, //28
                    50, //29
                    51, //30
                    52, //31
                    53, //32
                    53, //33
                    54, //34
                    54, //35
                    55, //36
                    56, //37
                    56, //38
                    57, //39
                    58, //40
                    58, //41
                    59, //42
                    59, //43
                    60, //44
                    61, //45
                    61, //46
                    62, //47
                    62, //48
                    63, //49
                    63, //50
                    64, //51
                    64, //52
                    65, //53
                    65, //54
                    65, //55
                    66, //56
                    66, //57
                    67, //58
                    67, //59
                    67, //60
                    68, //61
                    68, //62
                    68, //63
                    69, //64
                    69, //65
                    69, //66
                    70, //67
                    70, //68
                    70, //69
                    71, //70
                    71, //71
                    71, //72
                    72, //73
                    72, //74
                    73, //75
                    73, //76
                    73, //77
                    74, //78
                    74, //79
                    74, //80
                    74, //81
                    75, //82
                    75, //83
                    75, //84
                    76, //85
                    76, //86
                    76, //87
                    76, //88
                    77, //89
                    77, //90
                    77, //91
                    77, //92
                    78, //93
                    78, //94
                    78, //95
                    78, //96
                    79, //97
                    79, //98
                    79, //99
                    79, //100
                    80, //101
                    80, //102
                    80, //103
                    80, //104
                    81, //105
                    81, //106
                    81, //107
                    81, //108
                    82, //109
                    82, //110
                    82, //111
                    82, //112
                    83, //113
                    83, //114
                    83, //115
                    84, //116
                    84, //117
                    84, //118
                    85, //119
                    85, //120
                    85, //121
                    86, //122
                    86, //123
                    87, //124
                    87, //125
                    87, //126
                    88, //127
                }
            }, //RF_CHAN_2
                {
                    {
                        19, //0
                        21, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        28, //6
                        29, //7
                        30, //8
                        31, //9
                        33, //10
                        34, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        41, //18
                        42, //19
                        43, //20
                        44, //21
                        45, //22
                        46, //23
                        47, //24
                        48, //25
                        48, //26
                        49, //27
                        50, //28
                        50, //29
                        51, //30
                        52, //31
                        53, //32
                        53, //33
                        54, //34
                        54, //35
                        55, //36
                        56, //37
                        56, //38
                        57, //39
                        58, //40
                        58, //41
                        59, //42
                        59, //43
                        60, //44
                        61, //45
                        61, //46
                        62, //47
                        62, //48
                        63, //49
                        63, //50
                        64, //51
                        64, //52
                        65, //53
                        65, //54
                        65, //55
                        66, //56
                        66, //57
                        67, //58
                        67, //59
                        67, //60
                        68, //61
                        68, //62
                        68, //63
                        69, //64
                        69, //65
                        69, //66
                        70, //67
                        70, //68
                        70, //69
                        71, //70
                        71, //71
                        71, //72
                        72, //73
                        72, //74
                        73, //75
                        73, //76
                        73, //77
                        74, //78
                        74, //79
                        74, //80
                        74, //81
                        75, //82
                        75, //83
                        75, //84
                        76, //85
                        76, //86
                        76, //87
                        76, //88
                        77, //89
                        77, //90
                        77, //91
                        77, //92
                        78, //93
                        78, //94
                        78, //95
                        78, //96
                        79, //97
                        79, //98
                        79, //99
                        79, //100
                        80, //101
                        80, //102
                        80, //103
                        80, //104
                        81, //105
                        81, //106
                        81, //107
                        81, //108
                        82, //109
                        82, //110
                        82, //111
                        82, //112
                        83, //113
                        83, //114
                        83, //115
                        84, //116
                        84, //117
                        84, //118
                        85, //119
                        85, //120
                        85, //121
                        86, //122
                        86, //123
                        87, //124
                        87, //125
                        87, //126
                        88, //127
                    }
                }, //RF_CHAN_3
                {
                    {
                        18, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        28, //5
                        30, //6
                        31, //7
                        33, //8
                        34, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        40, //15
                        41, //16
                        42, //17
                        43, //18
                        44, //19
                        45, //20
                        46, //21
                        46, //22
                        47, //23
                        48, //24
                        49, //25
                        50, //26
                        51, //27
                        52, //28
                        53, //29
                        53, //30
                        54, //31
                        55, //32
                        55, //33
                        56, //34
                        57, //35
                        57, //36
                        58, //37
                        58, //38
                        59, //39
                        59, //40
                        60, //41
                        61, //42
                        61, //43
                        62, //44
                        62, //45
                        63, //46
                        63, //47
                        64, //48
                        64, //49
                        65, //50
                        65, //51
                        66, //52
                        66, //53
                        67, //54
                        67, //55
                        68, //56
                        68, //57
                        68, //58
                        69, //59
                        69, //60
                        69, //61
                        70, //62
                        70, //63
                        71, //64
                        71, //65
                        71, //66
                        72, //67
                        72, //68
                        72, //69
                        73, //70
                        73, //71
                        74, //72
                        74, //73
                        74, //74
                        75, //75
                        75, //76
                        75, //77
                        76, //78
                        76, //79
                        76, //80
                        76, //81
                        77, //82
                        77, //83
                        77, //84
                        77, //85
                        78, //86
                        78, //87
                        78, //88
                        78, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        82, //101
                        82, //102
                        82, //103
                        82, //104
                        83, //105
                        83, //106
                        83, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        85, //112
                        85, //113
                        85, //114
                        86, //115
                        86, //116
                        86, //117
                        86, //118
                        87, //119
                        87, //120
                        87, //121
                        88, //122
                        88, //123
                        88, //124
                        88, //125
                        89, //126
                        89, //127
                    }
                }, //RF_CHAN_4
                {
                    {
                        18, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        28, //5
                        30, //6
                        31, //7
                        33, //8
                        34, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        40, //15
                        41, //16
                        42, //17
                        43, //18
                        44, //19
                        45, //20
                        46, //21
                        46, //22
                        47, //23
                        48, //24
                        49, //25
                        50, //26
                        51, //27
                        52, //28
                        53, //29
                        53, //30
                        54, //31
                        55, //32
                        55, //33
                        56, //34
                        57, //35
                        57, //36
                        58, //37
                        58, //38
                        59, //39
                        59, //40
                        60, //41
                        61, //42
                        61, //43
                        62, //44
                        62, //45
                        63, //46
                        63, //47
                        64, //48
                        64, //49
                        65, //50
                        65, //51
                        66, //52
                        66, //53
                        67, //54
                        67, //55
                        68, //56
                        68, //57
                        68, //58
                        69, //59
                        69, //60
                        69, //61
                        70, //62
                        70, //63
                        71, //64
                        71, //65
                        71, //66
                        72, //67
                        72, //68
                        72, //69
                        73, //70
                        73, //71
                        74, //72
                        74, //73
                        74, //74
                        75, //75
                        75, //76
                        75, //77
                        76, //78
                        76, //79
                        76, //80
                        76, //81
                        77, //82
                        77, //83
                        77, //84
                        77, //85
                        78, //86
                        78, //87
                        78, //88
                        78, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        82, //101
                        82, //102
                        82, //103
                        82, //104
                        83, //105
                        83, //106
                        83, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        85, //112
                        85, //113
                        85, //114
                        86, //115
                        86, //116
                        86, //117
                        86, //118
                        87, //119
                        87, //120
                        87, //121
                        88, //122
                        88, //123
                        88, //124
                        88, //125
                        89, //126
                        89, //127
                    }
                }, //RF_CHAN_5
                {
                    {
                        18, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        28, //5
                        30, //6
                        31, //7
                        33, //8
                        34, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        40, //15
                        41, //16
                        42, //17
                        43, //18
                        44, //19
                        45, //20
                        46, //21
                        46, //22
                        47, //23
                        48, //24
                        49, //25
                        50, //26
                        51, //27
                        52, //28
                        53, //29
                        53, //30
                        54, //31
                        55, //32
                        55, //33
                        56, //34
                        57, //35
                        57, //36
                        58, //37
                        58, //38
                        59, //39
                        59, //40
                        60, //41
                        61, //42
                        61, //43
                        62, //44
                        62, //45
                        63, //46
                        63, //47
                        64, //48
                        64, //49
                        65, //50
                        65, //51
                        66, //52
                        66, //53
                        67, //54
                        67, //55
                        68, //56
                        68, //57
                        68, //58
                        69, //59
                        69, //60
                        69, //61
                        70, //62
                        70, //63
                        71, //64
                        71, //65
                        71, //66
                        72, //67
                        72, //68
                        72, //69
                        73, //70
                        73, //71
                        74, //72
                        74, //73
                        74, //74
                        75, //75
                        75, //76
                        75, //77
                        76, //78
                        76, //79
                        76, //80
                        76, //81
                        77, //82
                        77, //83
                        77, //84
                        77, //85
                        78, //86
                        78, //87
                        78, //88
                        78, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        82, //101
                        82, //102
                        82, //103
                        82, //104
                        83, //105
                        83, //106
                        83, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        85, //112
                        85, //113
                        85, //114
                        86, //115
                        86, //116
                        86, //117
                        86, //118
                        87, //119
                        87, //120
                        87, //121
                        88, //122
                        88, //123
                        88, //124
                        88, //125
                        89, //126
                        89, //127
                    }
                }, //RF_CHAN_6
                {
                    {
                        18, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        28, //5
                        30, //6
                        31, //7
                        33, //8
                        34, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        40, //15
                        41, //16
                        42, //17
                        43, //18
                        44, //19
                        45, //20
                        46, //21
                        46, //22
                        47, //23
                        48, //24
                        49, //25
                        50, //26
                        51, //27
                        52, //28
                        53, //29
                        53, //30
                        54, //31
                        55, //32
                        55, //33
                        56, //34
                        57, //35
                        57, //36
                        58, //37
                        58, //38
                        59, //39
                        59, //40
                        60, //41
                        61, //42
                        61, //43
                        62, //44
                        62, //45
                        63, //46
                        63, //47
                        64, //48
                        64, //49
                        65, //50
                        65, //51
                        66, //52
                        66, //53
                        67, //54
                        67, //55
                        68, //56
                        68, //57
                        68, //58
                        69, //59
                        69, //60
                        69, //61
                        70, //62
                        70, //63
                        71, //64
                        71, //65
                        71, //66
                        72, //67
                        72, //68
                        72, //69
                        73, //70
                        73, //71
                        74, //72
                        74, //73
                        74, //74
                        75, //75
                        75, //76
                        75, //77
                        76, //78
                        76, //79
                        76, //80
                        76, //81
                        77, //82
                        77, //83
                        77, //84
                        77, //85
                        78, //86
                        78, //87
                        78, //88
                        78, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        82, //101
                        82, //102
                        82, //103
                        82, //104
                        83, //105
                        83, //106
                        83, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        85, //112
                        85, //113
                        85, //114
                        86, //115
                        86, //116
                        86, //117
                        86, //118
                        87, //119
                        87, //120
                        87, //121
                        88, //122
                        88, //123
                        88, //124
                        88, //125
                        89, //126
                        89, //127
                    }
                }, //RF_CHAN_7
                {
                    {
                        18, //0
                        20, //1
                        22, //2
                        24, //3
                        26, //4
                        28, //5
                        30, //6
                        31, //7
                        33, //8
                        34, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        40, //15
                        41, //16
                        42, //17
                        43, //18
                        44, //19
                        45, //20
                        46, //21
                        46, //22
                        47, //23
                        48, //24
                        49, //25
                        50, //26
                        51, //27
                        52, //28
                        53, //29
                        53, //30
                        54, //31
                        55, //32
                        55, //33
                        56, //34
                        57, //35
                        57, //36
                        58, //37
                        58, //38
                        59, //39
                        59, //40
                        60, //41
                        61, //42
                        61, //43
                        62, //44
                        62, //45
                        63, //46
                        63, //47
                        64, //48
                        64, //49
                        65, //50
                        65, //51
                        66, //52
                        66, //53
                        67, //54
                        67, //55
                        68, //56
                        68, //57
                        68, //58
                        69, //59
                        69, //60
                        69, //61
                        70, //62
                        70, //63
                        71, //64
                        71, //65
                        71, //66
                        72, //67
                        72, //68
                        72, //69
                        73, //70
                        73, //71
                        74, //72
                        74, //73
                        74, //74
                        75, //75
                        75, //76
                        75, //77
                        76, //78
                        76, //79
                        76, //80
                        76, //81
                        77, //82
                        77, //83
                        77, //84
                        77, //85
                        78, //86
                        78, //87
                        78, //88
                        78, //89
                        79, //90
                        79, //91
                        79, //92
                        79, //93
                        80, //94
                        80, //95
                        80, //96
                        81, //97
                        81, //98
                        81, //99
                        81, //100
                        82, //101
                        82, //102
                        82, //103
                        82, //104
                        83, //105
                        83, //106
                        83, //107
                        84, //108
                        84, //109
                        84, //110
                        84, //111
                        85, //112
                        85, //113
                        85, //114
                        86, //115
                        86, //116
                        86, //117
                        86, //118
                        87, //119
                        87, //120
                        87, //121
                        88, //122
                        88, //123
                        88, //124
                        88, //125
                        89, //126
                        89, //127
                    }
                }, //RF_CHAN_8
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_9
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_10
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_11
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_12
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_13
                {
                    {
                        18, //0
                        20, //1
                        23, //2
                        24, //3
                        25, //4
                        26, //5
                        27, //6
                        30, //7
                        31, //8
                        32, //9
                        34, //10
                        35, //11
                        36, //12
                        37, //13
                        38, //14
                        39, //15
                        40, //16
                        41, //17
                        42, //18
                        43, //19
                        44, //20
                        45, //21
                        46, //22
                        46, //23
                        47, //24
                        48, //25
                        49, //26
                        49, //27
                        50, //28
                        51, //29
                        52, //30
                        52, //31
                        53, //32
                        54, //33
                        55, //34
                        55, //35
                        56, //36
                        57, //37
                        58, //38
                        58, //39
                        59, //40
                        59, //41
                        60, //42
                        60, //43
                        61, //44
                        61, //45
                        62, //46
                        62, //47
                        63, //48
                        64, //49
                        64, //50
                        64, //51
                        65, //52
                        65, //53
                        66, //54
                        66, //55
                        66, //56
                        67, //57
                        67, //58
                        68, //59
                        68, //60
                        68, //61
                        69, //62
                        69, //63
                        70, //64
                        70, //65
                        70, //66
                        71, //67
                        71, //68
                        71, //69
                        72, //70
                        72, //71
                        73, //72
                        73, //73
                        73, //74
                        74, //75
                        74, //76
                        74, //77
                        74, //78
                        75, //79
                        75, //80
                        75, //81
                        75, //82
                        76, //83
                        76, //84
                        76, //85
                        77, //86
                        77, //87
                        77, //88
                        77, //89
                        78, //90
                        78, //91
                        78, //92
                        78, //93
                        79, //94
                        79, //95
                        79, //96
                        79, //97
                        80, //98
                        80, //99
                        80, //100
                        80, //101
                        81, //102
                        81, //103
                        81, //104
                        81, //105
                        82, //106
                        82, //107
                        82, //108
                        82, //109
                        83, //110
                        83, //111
                        83, //112
                        83, //113
                        84, //114
                        84, //115
                        84, //116
                        85, //117
                        85, //118
                        85, //119
                        85, //120
                        86, //121
                        86, //122
                        86, //123
                        86, //124
                        87, //125
                        87, //126
                        87, //127
                    }
                }, //RF_CHAN_14
        },

        //NV_TABLE_TPC_PDADC_OFFSETS
        {
            152,  // RF_CHAN_1
            152,  // RF_CHAN_2
            152,  // RF_CHAN_3
            138,  // RF_CHAN_4
            138,  // RF_CHAN_5
            138,  // RF_CHAN_6
            138,  // RF_CHAN_7
            138,  // RF_CHAN_8
            147,  // RF_CHAN_9
            147,  // RF_CHAN_10
            147,  // RF_CHAN_11
            147,  // RF_CHAN_12
            147,  // RF_CHAN_13
            147   // RF_CHAN_14
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
                0,           //reserved2;
                0,           //reserved3;

                0,           //pll_vfc_reg3_b0;
                0,           //pll_vfc_reg3_b1;
                0,           //pll_vfc_reg3_b2;
                0,           //pll_vfc_reg3_b3;

                0,           //tempStart;
                0            //tempFinish;
            }
        }

    } // tables
};


#endif

