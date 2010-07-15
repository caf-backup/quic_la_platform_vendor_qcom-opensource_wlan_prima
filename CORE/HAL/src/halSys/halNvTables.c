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
        // NV_TABLE_QFUSE
        {   0x0, 0x0, 0x0, 0x0      //default to nothing blown
        },


        // NV_TABLE_RATE_POWER_SETTINGS
        {
            // typedef tANI_S8 tPowerdBm;
            //typedef tPowerdBm tRateGroupPwr[NUM_HAL_PHY_RATES];
            //tRateGroupPwr       pwrOptimum[NUM_RF_SUBBANDS];
            {
                    //802.11b Rates
                {1600},    // HAL_PHY_RATE_11B_LONG_1_MBPS,
                {1600},    // HAL_PHY_RATE_11B_LONG_2_MBPS,
                {1600},    // HAL_PHY_RATE_11B_LONG_5_5_MBPS,
                {1600},    // HAL_PHY_RATE_11B_LONG_11_MBPS,
                {1600},    // HAL_PHY_RATE_11B_SHORT_2_MBPS,
                {1600},    // HAL_PHY_RATE_11B_SHORT_5_5_MBPS,
                {1600},    // HAL_PHY_RATE_11B_SHORT_11_MBPS,

                    //SLR Rates
                {1550},    // HAL_PHY_RATE_SLR_0_25_MBPS,
                {1550},    // HAL_PHY_RATE_SLR_0_5_MBPS,

                    //11A 20MHz Rates
                {1300},    // HAL_PHY_RATE_11A_6_MBPS,
                {1300},    // HAL_PHY_RATE_11A_9_MBPS,
                {1250},    // HAL_PHY_RATE_11A_12_MBPS,
                {1200},    // HAL_PHY_RATE_11A_18_MBPS,
                {1150},    // HAL_PHY_RATE_11A_24_MBPS,
                {1100},    // HAL_PHY_RATE_11A_36_MBPS,
                {1050},    // HAL_PHY_RATE_11A_48_MBPS,
                {1000},    // HAL_PHY_RATE_11A_54_MBPS,

                    //MCS Index #0-15 (20MHz)
                {1300},    // HAL_PHY_RATE_MCS_1NSS_6_5_MBPS,
                {1250},    // HAL_PHY_RATE_MCS_1NSS_13_MBPS,
                {1200},    // HAL_PHY_RATE_MCS_1NSS_19_5_MBPS,
                {1150},    // HAL_PHY_RATE_MCS_1NSS_26_MBPS,
                {1100},    // HAL_PHY_RATE_MCS_1NSS_39_MBPS,
                {1050},    // HAL_PHY_RATE_MCS_1NSS_52_MBPS,
                {1000},    // HAL_PHY_RATE_MCS_1NSS_58_5_MBPS,
                { 900},    // HAL_PHY_RATE_MCS_1NSS_65_MBPS,
                {1300},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_7_2_MBPS,
                {1250},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_14_4_MBPS,
                {1200},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_21_7_MBPS,
                {1150},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_28_9_MBPS,
                {1100},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_43_3_MBPS,
                {1050},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_57_8_MBPS,
                {1000},    // HAL_PHY_RATE_MCS_1NSS_MM_SG_65_MBPS,
                { 900}     // HAL_PHY_RATE_MCS_1NSS_MM_SG_72_2_MBPS,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_1,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_2,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_3,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_4,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_5,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_6,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_7,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_8,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_9,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_10,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_11,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_12,
                    {eANI_BOOLEAN_TRUE, 30},           //RF_CHAN_13,
                    {eANI_BOOLEAN_FALSE, 30},           //RF_CHAN_14,

                }, //sRegulatoryChannel end
                {
                    { 0 },  // RF_SUBBAND_2_4_GHZ
                },

                { // bRatePowerOffset start
                    //2.4GHz Band
                    { -125 },                       //RF_CHAN_1,
                    { -125 },                       //RF_CHAN_2,
                    { -125 },                       //RF_CHAN_3,
                    { -125 },                       //RF_CHAN_4,
                    { -125 },                       //RF_CHAN_5,
                    { -125 },                       //RF_CHAN_6,
                    { -125 },                       //RF_CHAN_7,
                    { -125 },                       //RF_CHAN_8,
                    { -125 },                       //RF_CHAN_9,
                    { -125 },                       //RF_CHAN_10,
                    { -125 },                       //RF_CHAN_11,
                    { -125 },                       //RF_CHAN_12,
                    { -125 },                       //RF_CHAN_13,
                    { -125 },                       //RF_CHAN_14,
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

        // NV_TABLE_TPC_CONFIG
        {
            // #define MAX_TPC_CAL_POINTS      (4)
            // #define MAX_TPC_CHANNELS        (2)
            // #define START_TPC_CHANNEL       2412
            // #define END_TPC_CHANNEL         2472
            //
            // typedef tANI_U8 tPowerDetect;        //7-bit power detect reading
            // typedef struct
            // {
            //     tPowerDetect pwrDetAdc;            //= SENSED_PWR register, which reports the 8-bit ADC
            //                                        // the stored ADC value gets shifted to 7-bits as the index to the LUT
            //     tPowerDetect adjustedPwrDet;       //7-bit value that goes into the LUT at the LUT[pwrDet] location
            //                                        //MSB set if extraPrecision.hi8_adjustedPwrDet is used
            // }tTpcCaldPowerPoint;
            //
            // typedef tTpcCaldPowerPoint tTpcCaldPowerTable[PHY_MAX_TX_CHAINS][MAX_TPC_CAL_POINTS];
            //
            // typedef struct
            // {
            //     tANI_U16 freq;                                     //frequency in MHz
            //     tANI_U16 reserved;
            //     tPowerdBmRange absPower;                           //Power range common to all chains
            //     tTpcCaldPowerTable empirical;                      //calibrated power points
            // }tTpcConfig;

            {
                START_TPC_CHANNEL,
                0,
                { MIN_PWR_LUT_DBM_2DEC_PLACES, MAX_PWR_LUT_DBM_2DEC_PLACES },   //tPowerdBmRange absPower;
                {
                    { // pwrDetAdc, adjustedPwrDet
                        { 9,     6 },   //cal point 0
                        { 14,   38 },   //cal point 1
                        { 28,   70 },   //cal point 2
                        { 115, 105 }    //cal point 3
                    } //PHY_TX_CHAIN_0
                } //empirical
            }, //START_TPC_CHANNEL

            {
                END_TPC_CHANNEL,
                0,
                { MIN_PWR_LUT_DBM_2DEC_PLACES, MAX_PWR_LUT_DBM_2DEC_PLACES },   //tPowerdBmRange absPower;
                {
                    { // pwrDetAdc, adjustedPwrDet
                        { 9,     6 },   //cal point 0
                        { 14,   38 },   //cal point 1
                        { 28,   70 },   //cal point 2
                        { 115, 105 }    //cal point 3
                    } //PHY_TX_CHAIN_0
                } //empirical
            } //END_TPC_CHANNEL
        },

        // NV_TABLE_RF_CAL_VALUES
        {
            7,      // hdet_ctl_ext_atten;
            29,      // hdet_dcoc_code;
            1,      // hdet_dcoc_ib_rcal_en;
            1       // hdet_dcoc_ib_scal_en;
        },

        //NV_TABLE_RSSI_OFFSETS
        {
            0, 0    //rssiOffset[PHY_MAX_RX_CHAINS];
        }

    } // tables
};


#endif

