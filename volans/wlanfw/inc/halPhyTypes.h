/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halPhyTypes.h

    \brief This header holds exposed types that are in common to configuration/Eeprom and
            run-time interfaces in halPhy.

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALPHYTYPES_H
#define HALPHYTYPES_H

#include <halTypes.h>

typedef tANI_S8  tANI_S6;
typedef tANI_S16 tANI_S9;
typedef tANI_S16 tANI_S10;
typedef tANI_S16 tANI_S12;
typedef tANI_U16 tANI_U10;


#ifdef VERIFY_HALPHY_SIMV_MODEL
typedef tANI_U8 tANI_BOOLEAN;
#endif


//convert float to a format that preserves enough accuracy to be used by driver
typedef tANI_S16 t2Decimal;
#define CONVERT_TO_2DECIMAL_PLACES(x)   (x * 100)
#define CONVERT_FROM_2DECIMAL_PLACES(x) (x / 100)

#ifndef PTT_FLOAT
#define PTT_FLOAT tANI_U32  // driver code can't include float, 
                            // so this reserves space in our structures to allow customer measurements in float
                            // driver code always uses t2decimal, whereas the pttApi always expects float.
#endif
typedef union
{
    PTT_FLOAT measurement;      //measured values can be passed to pttApi, but are maintained to 2 decimal places internally
    t2Decimal reported;         //used internally only - reported values only maintain 2 decimals places
}uAbsPwrPrecision;


typedef enum
{
    PHY_RX_CHAIN_0 = 0,

    PHY_MAX_RX_CHAINS = 1,
    PHY_ALL_RX_CHAINS,
    PHY_NO_RX_CHAINS
}ePhyRxChains;

typedef enum
{
    PHY_TX_CHAIN_0 = 0,

    PHY_MAX_TX_CHAINS = 1,
    PHY_ALL_TX_CHAINS,

    //possible tx chain combinations
    PHY_NO_TX_CHAINS,
}ePhyTxChains;

typedef enum
{
    PHY_I_RAIL = 0,
    PHY_Q_RAIL = 1,
    PHY_NUM_IQ_RAILS
}ePhyIQ;


typedef enum
{
    TX_GAIN_STEP_0,
    TX_GAIN_STEP_1,
    TX_GAIN_STEP_2,
    TX_GAIN_STEP_3,
    TX_GAIN_STEP_4,
    TX_GAIN_STEP_5,
    TX_GAIN_STEP_6,
    TX_GAIN_STEP_7,
    TX_GAIN_STEP_8,
    TX_GAIN_STEP_9,
    TX_GAIN_STEP_10,
    TX_GAIN_STEP_11,
    TX_GAIN_STEP_12,
    TX_GAIN_STEP_13,
    TX_GAIN_STEP_14,
    TX_GAIN_STEP_15,

    RX_GAIN_STEP_0   = 0,
    RX_GAIN_STEP_1,
    RX_GAIN_STEP_2,
    RX_GAIN_STEP_3,
    RX_GAIN_STEP_4,
    RX_GAIN_STEP_5,
    RX_GAIN_STEP_6,
    RX_GAIN_STEP_7,
    RX_GAIN_STEP_8,
    RX_GAIN_STEP_9,
    RX_GAIN_STEP_10,
    RX_GAIN_STEP_11,
    RX_GAIN_STEP_12,
    RX_GAIN_STEP_13,
    RX_GAIN_STEP_14,
    RX_GAIN_STEP_15,

    NUM_TX_GAIN_STEPS = 16,
    MAX_TX_GAIN_STEP = TX_GAIN_STEP_15,

    NUM_RX_GAIN_STEPS = 16,
    MAX_RX_GAIN_STEP = RX_GAIN_STEP_15,

    INVALID_GAIN_STEP
}eGainSteps;

// Keeping these definitions in case we need to revive them later
// #define ABS_GAIN_RANGE_DB       ((NUM_RF_RX_GAIN_STEPS - 1) * 2)
//
// #define DB_PER_INDEX            1
// #define RF_MAX_GAIN_DB          83      //defines highest gain
// #define RF_MIN_GAIN_DB          25
// #define RF_GAIN_RANGE_DB        (RF_MAX_GAIN_DB - RF_MIN_GAIN_DB)
//
// #define RF_GAIN_INDEX_DELTA     (RF_GAIN_RANGE_DB/DB_PER_INDEX)
// #define RF_GAIN_MAX_INDEX       (NUM_RF_RX_GAIN_STEPS - 1)
// #define RF_GAIN_MIN_INDEX       (RF_GAIN_MAX_INDEX - RF_GAIN_INDEX_DELTA)   //the index where the minimum gain falls,
//                                                                                     // all lower indexes are filled with the minimum gain
//
// #define SPEC_MIN_GAIN           10
// #define AGC_MEASUREMENT_INDEX   29  // index between range of RF_GAIN_MIN_INDEX to RF_GAIN_MAX_INDEX
                                    // where AGC gain measurements should be made


typedef struct
{
    tANI_S9 center;
    tANI_S9 offCenter;
    tANI_S9 imbalance;
}sIQCalValues;

typedef struct
{
    sIQCalValues iq[PHY_MAX_RX_CHAINS];
}sRxChainsIQCalValues;

typedef struct
{
    sIQCalValues iq[PHY_MAX_TX_CHAINS];
}sTxChainsIQCalValues;


typedef tANI_S8 tPowerdBm;   //power in signed 8-bit integer, no decimal places
typedef tANI_U16 t_mW;       //milliWatts
typedef tANI_U8 tPwrTemplateIndex;   //5-bit number used as the index into the tx gain tables

typedef struct
{
    tANI_U8 txPowerAdc[PHY_MAX_TX_CHAINS];
}sTxChainsPowerAdcReadings;

typedef struct
{
    tANI_U8 agcGain;
}tRxGain;

typedef struct
{
    tANI_U8 rx[PHY_MAX_RX_CHAINS];
}sRxChainsData;


typedef sRxChainsData sRxChainsRssi;
typedef sRxChainsData sRxChainsAgcDisable;

typedef struct
{
    tANI_BOOLEAN rx[PHY_MAX_RX_CHAINS];
}sRxChainsBoolean;

typedef sRxChainsBoolean sRxChainsAgcEnable;


#define NUM_AGC_GAINS   64
typedef tRxGain sAgcGainLut[NUM_AGC_GAINS];


typedef struct
{
    tANI_S6 iLo;
    tANI_S6 qLo;
}sTxFirLoCorrect;


#endif

