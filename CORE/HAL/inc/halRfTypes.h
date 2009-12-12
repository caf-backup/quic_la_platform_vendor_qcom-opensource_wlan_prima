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

#ifndef RF_CHIP_GEMINI
#define RF_CHIP_GEMINI
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
}sRegulatoryDomains;


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



typedef tANI_U8 tTempADCVal;






typedef tANI_U8 tDcoCorrect;

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
    tDcoCorrect IDcoCorrect;
    tDcoCorrect QDcoCorrect;
    tANI_U8     dcRange;
}tTxLoCorrect;

typedef struct
{
    tTxLoCorrect txLo[PHY_MAX_TX_CHAINS];
}sTxChainsLoCorrections;


//tDcoCorrect is needed to define rf specific structures

#if defined(RF_CHIP_GEMINI)
#define NUM_RF_RX_GAIN_STEPS    (128)
#define MAX_RF_RX_GAIN_STEP     (NUM_RF_RX_GAIN_STEPS - 1)

#define NUM_RF_TX_GAIN_STEPS    (16)
#define MAX_RF_TX_GAIN_STEP     (NUM_RF_TX_GAIN_STEPS - 1)

#define NUM_RF_DCO_VALUES       91 //There are only 32 DCO values, but our algorithm it makes more sense for us to access these by AGC gain index
#define MAX_RF_DCO_VALUE        (NUM_RF_DCO_VALUES - 1)


typedef struct
{
    tANI_U16 gainReg1;   //GEMINI_REG_RX_GC_0 (lna + mix + tia + bq1 + bq2 + pga)
}tRfRxGain;


typedef struct
{
    tANI_U16 gainReg1;   //GEMINI_REG_PA_GC (ib_vtoi + g_vtoi + pga)
}tRfTxGain;


typedef tANI_U8 tGeminiReg;


typedef struct
{
    //TODO:define this struct for Gemini
    tANI_U8 rxIf;
    tANI_U8 txIf;
    tANI_U8 txRf;
    tANI_U8 reserved;
}sRfSpecificFilterSettings;

typedef sRfSpecificFilterSettings sRfChannelFilterSettings[NUM_RF_CHANNELS];


typedef struct
{
    tANI_U8 hdet_ctl_ext_atten;
    tANI_U8 hdet_dcoc_code;
    tANI_U8 hdet_dcoc_ib_rcal_en;
    tANI_U8 hdet_dcoc_ib_scal_en;
}sRfNvCalValues;  //stored in QFUSE

#endif

typedef tRxDcoCorrect tRxDcoMatrix[PHY_MAX_RX_CHAINS][NUM_RF_DCO_VALUES];

typedef enum
{
    SYNTH_UNLOCKED,
    SYNTH_LOCK
}eRfSynthLock;


#endif

