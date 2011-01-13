/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halNvTables.h

    \brief Union of table structures that will be stored in NV

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALNVTABLES_H
#define HALNVTABLES_H

#include <halPhyCalMemory.h>
#include <halPhyRates.h>
#include <phyTxPower.h>

typedef enum
{
    NV_FIELDS_IMAGE                 = 0,    //contains all fields

    NV_TABLE_RATE_POWER_SETTINGS    = 2,
    NV_TABLE_REGULATORY_DOMAINS     = 3,
    NV_TABLE_DEFAULT_COUNTRY        = 4,
    NV_TABLE_TPC_POWER_TABLE        = 5,
    NV_TABLE_TPC_PDADC_OFFSETS      = 6,
    NV_TABLE_RF_CAL_VALUES          = 7,
    NV_TABLE_RSSI_CHANNEL_OFFSETS   = 9,
    NV_TABLE_CAL_MEMORY             = 10,    //cal memory structure from halPhyCalMemory.h preceded by status
    NV_TABLE_CAL_STATUS             = 11,

    NUM_NV_TABLE_IDS,
    NV_ALL_TABLES                   = 0xFFF,
    NV_BINARY_IMAGE                 = 0x1000,
    NV_MAX_TABLE                    = 0xFFFFFFFF  /* define as 4 bytes data */
}eNvTable;

#define NV_FIELD_COUNTRY_CODE_SIZE  3
typedef struct
{
    tANI_U8 regDomain;                                      //from eRegDomainId
    tANI_U8 countryCode[NV_FIELD_COUNTRY_CODE_SIZE];    // string identifier
}sDefaultCountry;


typedef union
{
    tRateGroupPwr           pwrOptimum[NUM_RF_SUBBANDS];              // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains      regDomains[NUM_REG_DOMAINS];              // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry         defaultCountryTable;                      // NV_TABLE_DEFAULT_COUNTRY
    tTpcPowerTable          plutCharacterized[NUM_2_4GHZ_CHANNELS];   // NV_TABLE_TPC_POWER_TABLE
    tANI_U16                plutPdadcOffset[NUM_2_4GHZ_CHANNELS];     // NV_TABLE_TPC_PDADC_OFFSETS
    //sCalFlashMemory         calFlashMemory;                           // NV_TABLE_CAL_MEMORY
    //sCalStatus              calStatus;                                // NV_TABLE_CAL_STATUS
    sRssiChannelOffsets     rssiChanOffsets[2];                       // NV_TABLE_RSSI_CHANNEL_OFFSETS
    sRFCalValues            rFCalValues;                              // NV_TABLE_RF_CAL_VALUES
}uNvTables;

typedef struct
{
    tRateGroupPwr           pwrOptimum[NUM_RF_SUBBANDS];              // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains      regDomains[NUM_REG_DOMAINS];              // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry         defaultCountryTable;                      // NV_TABLE_DEFAULT_COUNTRY
    tTpcPowerTable          plutCharacterized[NUM_2_4GHZ_CHANNELS];   // NV_TABLE_TPC_POWER_TABLE
    tANI_U16                plutPdadcOffset[NUM_2_4GHZ_CHANNELS];     // NV_TABLE_TPC_PDADC_OFFSETS
    //sCalFlashMemory         calFlashMemory;                           // NV_TABLE_CAL_MEMORY
    //sCalStatus              calStatus;                                // NV_TABLE_CAL_STATUS
    sRssiChannelOffsets     rssiChanOffsets[2];                       // NV_TABLE_RSSI_CHANNEL_OFFSETS
    sRFCalValues            rFCalValues;                              // NV_TABLE_RF_CAL_VALUES
}sNvTables;





#endif
