/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halRfTypes.h

    \brief Types that are common to configuration/Eeprom tables and halPhy interfaces

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALRFTYPES_H
#define HALRFTYPES_H

#include <halPhyTypes.h>

#ifndef RF_CHIP_MIDAS
#define RF_CHIP_MIDAS
#endif

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

typedef enum
{
    MODE_802_11B    = 0,
    MODE_802_11AG   = 1,
    MODE_802_11N    = 2,
    NUM_802_11_MODES
} e80211Modes;

typedef struct
{
    tANI_BOOLEAN enabled;
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
    tANI_S16 bRssiOffset[NUM_2_4GHZ_CHANNELS];
    tANI_S16 gnRssiOffset[NUM_2_4GHZ_CHANNELS];
}sRssiChannelOffsets;

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

typedef struct
{
    tANI_U16 targetFreq;           //number in MHz
    tANI_U8 channelNum;            //channel number as in the eRfChannels enumeration
    eRfSubBand band;               //band that this channel belongs to
}tRfChannelProps;

extern const tRfChannelProps rfChannels[NUM_RF_CHANNELS];

typedef enum
{
    RF_CAL_TONE_28NEG,
    RF_CAL_TONE_24NEG,
    RF_CAL_TONE_20NEG,
    RF_CAL_TONE_16NEG,
    RF_CAL_TONE_12NEG,
    RF_CAL_TONE_8NEG,
    RF_CAL_TONE_4NEG,
    RF_CAL_TONE_4POS,
    RF_CAL_TONE_8POS,
    RF_CAL_TONE_12POS,
    RF_CAL_TONE_16POS,
    RF_CAL_TONE_20POS,
    RF_CAL_TONE_24POS,
    RF_CAL_TONE_28POS,

    NUM_RF_TONES,

    MIN_RF_TONE = RF_CAL_TONE_28NEG,
    MAX_RF_TONE = RF_CAL_TONE_28POS
}eRfTones;



typedef tANI_U16 tRfADCVal;
typedef tRfADCVal tTempADCVal;


typedef tANI_U8 tDcoCorrect;
typedef tANI_S8 tIm2Correct;

typedef struct
{
    tDcoCorrect IDcoCorrect;
    tDcoCorrect QDcoCorrect;
    tANI_U8     dcRange;
}tRxDcoCorrect;

typedef struct
{
    tRxDcoCorrect dco[PHY_MAX_RX_CHAINS];
}tRxChainsDcoCorrections;

typedef struct
{
    tIm2Correct ICorrect;
    tIm2Correct QCorrect;
}tRxIm2Correct;

typedef struct
{
    tRxIm2Correct dco[PHY_MAX_RX_CHAINS];
}tRxChainsIm2Corrections;

typedef struct
{
    tDcoCorrect IDcoCorrect;
    tDcoCorrect QDcoCorrect;
}tTxLoCorrect;

typedef struct
{
    tTxLoCorrect txLo[PHY_MAX_TX_CHAINS];
}sTxChainsLoCorrections;


//tDcoCorrect is needed to define rf specific structures

#if defined(RF_CHIP_MIDAS)
#define NUM_RF_RX_GAIN_STEPS    (128)
#define MAX_RF_RX_GAIN_STEP     (NUM_RF_RX_GAIN_STEPS - 1)

#define NUM_RF_TX_GAIN_STEPS    (16)
#define MAX_RF_TX_GAIN_STEP     (NUM_RF_TX_GAIN_STEPS - 1)

#define RF_AGC_GAIN_LUT_DEPTH   (128)
#define NUM_RF_DCO_VALUES       (128) //There are only 32 DCO values, but our algorithm it makes more sense for us to access these by AGC gain index
#define MAX_RF_DCO_VALUE        (NUM_RF_DCO_VALUES - 1)


typedef struct
{
    tANI_U16 gainReg1;   //GEMINI_REG_RX_GC_0 (lna + mix + tia + bq1 + bq2 + pga)
}tRfRxGain;


typedef struct
{
    tANI_U16 bbf_gain_cnt;
    tANI_U16 bbf_lin_adj;
    tANI_U16 lo_mix_da_gain_cntl;
    tANI_U16 pa_gain_cntl;
    tANI_U16 da_pa_bias_1_cnt;
    tANI_U16 da_pa_bias_2_cntl;
}tRfTxGain;



#endif

typedef enum
{
    SYNTH_UNLOCKED,
    SYNTH_LOCK
}eRfSynthLock;

typedef enum
{
    TEMP_SENSOR_PA,
    TEMP_SENSOR_RX
}eRfTempSensor;

typedef enum
{
    TEMPERATURE_BIN_0,          //-30 to 5 C
    TEMPERATURE_BIN_1,          //5 to 45 C
    TEMPERATURE_BIN_2,          //45 to 85 C
    TEMPERATURE_BIN_3,          //85 to 125 C
    NUM_TEMPERATURE_BINS
}eTemperatureBins;

typedef struct
{
    tANI_U16 hdetDcocCode;
    tANI_U16 hdetDcoOffset;
}sRfHdetCalValues;

#endif

