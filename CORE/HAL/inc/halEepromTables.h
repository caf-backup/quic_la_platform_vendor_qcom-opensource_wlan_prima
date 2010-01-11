/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halEepromTables.h

    \brief Union of table structures that will be stored in EEPROM

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALEEPROMTABLES_H
#define HALEEPROMTABLES_H

#include "halPhyCfg.h"




typedef enum
{
    EEPROM_FIELDS_IMAGE                 = 0,    //for dword access to the EEPROM fields from address 0 to, but not including, the fixed checksum
    EEPROM_TABLE_FIRST                  = 1,    //for checking use (first Id)
    //Do not change the values associated with an enumeration as they are recorded in EEPROMs this way

    EEPROM_TABLE_TX_IQ                  = 1,    //intended for initial card bringup, to be replaced by runtime cals
    EEPROM_TABLE_RX_IQ                  = 2,    //intended for initial card bringup, to be replaced by runtime cals
    EEPROM_TABLE_LNA_SW_GAIN            = 3,
    EEPROM_TABLE_TX_LO                  = 4,    //intended for initial card bringup, to be replaced by runtime cals
    EEPROM_TABLE_QUASAR_REGS            = 5,    //intended for initial card bringup, to be replaced by smaller table
    EEPROM_TABLE_REGULATORY_DOMAINS     = 6,    //obsolete, convert to EEPROM_TABLE_REGULATORY_DOMAINS_V2
    //EEPROM_TABLE_AGC_ADJUST             = 7,  //retiring this
    EEPROM_TABLE_2_4_TPC_PARAMS         = 8,
    EEPROM_TABLE_2_4_TPC_CONFIG         = 9,
    EEPROM_TABLE_5_TPC_PARAMS           = 10,
    EEPROM_TABLE_5_TPC_CONFIG           = 11,
    EEPROM_TABLE_RATE_POWER_SETTINGS    = 12,
    EEPROM_TABLE_MULT_BSSID             = 13,
    EEPROM_TABLE_DEMO_CHANNEL_LIST      = 14,   //intended for initial card bringup
    EEPROM_TABLE_RX_NOTCH_FILTER        = 15,
    EEPROM_TABLE_TPC_TEMP_COMP          = 16,   //correction power points per temperature
    EEPROM_TABLE_RX_DCO                 = 17,   //intended for initial card bringup, to be replaced by runtime cals
    //EEPROM_TABLE_INIT_CAL               = 18,   //obsolete
    EEPROM_TABLE_QUASAR_FILTERS         = 19,   //stored during initialization of mfg driver
    EEPROM_TABLE_REGULATORY_DOMAINS_V2  = 20,   //newer version of structure to replace the old

    //these replace the TPC tables for separate bands
    //a limit is placed on how many channels can be stored
    EEPROM_TABLE_TPC_PARAMS             = 21,
    EEPROM_TABLE_TPC_CONFIG             = 22,
    EEPROM_TABLE_CAL_TABLES             = 23,

    NUM_EEPROM_TABLE_IDS,
    EEPROM_TABLE_DIRECTORY              = 0xF000,
    EEPROM_EMPTY_TABLE_DIR              = 0xAAA,
    EEPROM_ALL_TABLES                   = 0xFFF,
    EEPROM_BINARY_IMAGE                 = 0x1000
}eEepromTable;




#define MAX_MULT_BSS_IDS 16

typedef struct
{
    tANI_U8 numBss;
    tANI_U8 bssID[MAX_MULT_BSS_IDS][6];
}sMultipleBssTable;

#ifndef ANI_MANF_DIAG
typedef union
{
    sTxIQChannel            txIQTable[NUM_DEMO_CAL_CHANNELS];               // EEPROM_TABLE_TX_IQ
    sRxIQChannel            rxIQTable[NUM_DEMO_CAL_CHANNELS];               // EEPROM_TABLE_RX_IQ
    sLnaSwGainTable         lnaSwGainTable;                                 // EEPROM_TABLE_LNA_SW_GAIN
    sTxLoCorrectChannel     txLoCorrectTable[NUM_DEMO_CAL_CHANNELS];        // EEPROM_TABLE_TX_LO
    sQuasarChannelRegs      quasarRegsTable[NUM_RF_CHANNELS];               // EEPROM_TABLE_QUASAR_REGS
    sRegulatoryDomains_old  regDomains_old[NUM_REG_DOMAINS];                // obsolete EEPROM_TABLE_REGULATORY_DOMAINS
    tTpcParams              pwrBgParams;                                    // EEPROM_TABLE_2_4_TPC_PARAMS
    tTpcConfig              pwrBgBand[NUM_TPC_2_4GHZ_CHANNELS];             // EEPROM_TABLE_2_4_TPC_CONFIG
    tTpcParams              pwrAParams;                                     // EEPROM_TABLE_5_TPC_PARAMS
    tTpcConfig              pwrABand[NUM_TPC_5GHZ_CHANNELS];                // EEPROM_TABLE_5_TPC_CONFIG
    tRateGroupPwr           pwrOptimum[NUM_RF_SUBBANDS];                    // EEPROM_TABLE_RATE_POWER_SETTINGS
    tANI_U32                demoChannelList[NUM_DEMO_CAL_CHANNELS];         // EEPROM_TABLE_DEMO_CHANNEL_LIST (freqs in MHz)
    tNotchSettings          rxNotch[NUM_RF_CHANNELS];                       // EEPROM_TABLE_RX_NOTCH_FILTER
    tTPCTempCompSubband     tpcTempCompTable[NUM_RF_SUBBANDS];              // EEPROM_TABLE_TPC_TEMP_COMP
    tRxDcoMatrix            rxDcoTable[NUM_DEMO_CAL_CHANNELS];              // EEPROM_TABLE_RX_DCO
    sMultipleBssTable       multBssidTable;                                 // EEPROM_TABLE_MULT_BSSID
    tQuasarFilterSettings   quasarFiltersTable;                             // EEPROM_TABLE_QUASAR_FILTERS
    sRegulatoryDomains      regDomains[NUM_REG_DOMAINS];                    // EEPROM_TABLE_REGULATORY_DOMAINS_V2
    tTpcParams              pwrParams;                                      // EEPROM_TABLE_TPC_PARAMS
    tTpcConfig              pwrCal[MAX_TPC_CHANNELS];                       // EEPROM_TABLE_TPC_CONFIG
    sCalTable               calTable[NUM_RF_BANDS];                         // EEPROM_TABLE_CAL_TABLES
    
}uEepromTables;
#endif



#endif
