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

typedef enum
{
    NV_FIELDS_IMAGE                 = 0,    //contains all fields

    NV_TABLE_RATE_POWER_SETTINGS    = 1,
    NV_TABLE_REGULATORY_DOMAINS     = 2,
    NV_TABLE_DEFAULT_COUNTRY        = 3,
    NV_TABLE_CAL_MEMORY             = 4,    //cal memory structure from halPhyCalMemory.h preceded by status
    NV_TABLE_CAL_STATUS             = 5,

    NUM_NV_TABLE_IDS,
    NV_ALL_TABLES                   = 0xFFF,
    NV_BINARY_IMAGE                 = 0x1000
}eNvTable;

#define NV_FIELD_COUNTRY_CODE_SIZE  3
typedef struct
{
    tANI_U8 regDomain;                                      //from eRegDomainId
    tANI_U8 countryCode[NV_FIELD_COUNTRY_CODE_SIZE];    // string identifier
}sDefaultCountry;


typedef union
{
    tRateGroupPwr               pwrOptimum[NUM_RF_SUBBANDS];                    // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains          regDomains[NUM_REG_DOMAINS];                    // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry             defaultCountryTable;                            // NV_TABLE_DEFAULT_COUNTRY
    sCalFlashMemory             calFlashMemory;                                 // NV_TABLE_CAL_MEMORY
    sCalStatus                  calStatus;                                      // NV_TABLE_CAL_STATUS
}uNvTables;

typedef struct
{
    tRateGroupPwr               pwrOptimum[NUM_RF_SUBBANDS];                    // NV_TABLE_RATE_POWER_SETTINGS
    sRegulatoryDomains          regDomains[NUM_REG_DOMAINS];                    // NV_TABLE_REGULATORY_DOMAINS
    sDefaultCountry             defaultCountryTable;                            // NV_TABLE_DEFAULT_COUNTRY
    sCalFlashMemory             calFlashMemory;                                 // NV_TABLE_CAL_MEMORY
    sCalStatus                  calStatus;                                      // NV_TABLE_CAL_STATUS
}sNvTables;





#endif
