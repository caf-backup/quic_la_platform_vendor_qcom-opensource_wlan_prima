/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halNvTables.h

    \brief Union of table structures that will be stored in NV

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef HALNVTABLES_H
#define HALNVTABLES_H

#include "halPhyCfg.h"
#include "halQFuse.h"

typedef enum
{
    NV_FIELDS_IMAGE                 = 0,    //contains all fields
    NV_TABLE_QFUSE                  = 1,    //this table contains NV_TABLE_TPC_CONFIG and NV_TABLE_RX_GAIN_SHIFT,
                                                // and can not be directly read or written, only stored
    NV_TABLE_RATE_POWER_SETTINGS    = 2,
    NV_TABLE_REGULATORY_DOMAINS     = 3,
    NV_TABLE_DEFAULT_COUNTRY        = 4,
    NV_TABLE_TPC_CONFIG             = 5,    //this table only exists in cache, and is stored with NV_TABLE_QFUSE
    NV_TABLE_RX_GAIN_SHIFT          = 6,    //OBSOLETE - James says not needed
    NV_TABLE_RF_CAL_VALUES          = 7,    //this table only exists in cache, and is stored with NV_TABLE_QFUSE
    NV_TABLE_RSSI_OFFSETS           = 8,
    NV_TABLE_RSSI_CHANNEL_OFFSETS   = 9,

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
    sQFuseConfig                qFuseData;                                      // NV_TABLE_QFUSE
    tRateGroupPwr               pwrOptimum[NUM_RF_SUBBANDS];                    // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains          regDomains[NUM_REG_DOMAINS];                    // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry             defaultCountryTable;                            // NV_TABLE_DEFAULT_COUNTRY
    tTpcConfig                  tpcConfig[MAX_TPC_CHANNELS];                    // NV_TABLE_TPC_CONFIG
    sTpcFreqCalTable            tpcFreqCal;                                     // NV_TABLE_TPC_CONFIG
                                 //tpcFreqCal is compiled into pttApi.h where the cal data is converted from float to t2decimal
                                 //it shares the same enum value, but the structure is converted in pttModule to tTpcConfig
    sRfNvCalValues              rfCalValues;                                    // NV_TABLE_RF_CAL_VALUES
    tANI_S16                    rssiOffset[PHY_MAX_RX_CHAINS];                  // NV_TABLE_RSSI_OFFSETS
    sRssiChannelOffsets         rssiChanOffsets[PHY_MAX_RX_CHAINS];             // NV_TABLE_RSSI_CHANNEL_OFFSETS
}uNvTables;

typedef struct
{
    sQFuseConfig                qFuseData;                                      // NV_TABLE_QFUSE
    tRateGroupPwr               pwrOptimum[NUM_RF_SUBBANDS];                    // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains          regDomains[NUM_REG_DOMAINS];                    // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry             defaultCountryTable;                            // NV_TABLE_DEFAULT_COUNTRY
    tTpcConfig                  tpcConfig[MAX_TPC_CHANNELS];                    // NV_TABLE_TPC_CONFIG
    sRfNvCalValues              rfCalValues;                                    // NV_TABLE_RF_CAL_VALUES
    tANI_S16                    rssiOffset[PHY_MAX_RX_CHAINS];                  // NV_TABLE_RSSI_OFFSETS
    sRssiChannelOffsets         rssiChanOffsets[PHY_MAX_RX_CHAINS];             // NV_TABLE_RSSI_CHANNEL_OFFSETS

}sNvTables;





#endif
