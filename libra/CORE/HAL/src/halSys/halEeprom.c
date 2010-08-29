/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halEeprom.c

    \brief Eeprom facilities

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */
//#include "halInternal.h"

#include "palTypes.h"
#include "string.h"
#include "aniGlobal.h"
#include "ani_assert.h"
#include "halDebug.h"


const sEepromFields defCommonFields =
{
    0,                                                              // tANI_U16  productId;
    0,                                                              // tANI_U8   eepImageVersion;
    32,                                                             // tANI_U8   eepSize;
    CARD_NOT_SPECIFIED,                                             // tANI_U8   cardType;
    2,                                                              // tANI_U8   numOfTxChains;
    3,                                                              // tANI_U8   numOfRxChains;
    0,                                                              // tANI_U8   productBands;
    2,                                                              // tANI_U8   padcGain_24ghz;
    { 0, 0, 0 },                                                    // tANI_U8   unused1[3];
    SDRAM_NOT_PRESENT,                                              // tANI_U8   sdramInfo;
    { 0, 0, 0 },                                                    // tANI_U8   sdramReserved[3];
    { 0x00, 0xDE, 0xAD, 0xBE, 0xEF, 0x00 },                         // tANI_U8   macAddr[EEPROM_FIELD_MAC_ADDR_SIZE];
    { 0, 0 },                                                        // tANI_U8   unused2[2]
    { 'U', 'S', 'I' },                                              // tANI_U8   countryCode[EEPROM_FIELD_COUNTRY_CODE_SIZE];        // Country Code
    REG_DOMAIN_FCC,                                                 // tANI_U8   defaultRegDomain;      // must match the country code
    16,                                                             // tANI_U8   numStations;
    { 0, 0, 0 },                                                    // tANI_U8   unused3[3];
    0                                                               // tANI_U32  cksumPreceding;
};


eHalStatus halEepromOpen(tHalHandle hMac, sHalEeprom *pEepromEntries)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    pMac->hphy.eepromSize = pEepromEntries->fields.eepSize;

    if (pMac->hphy.eepromSize == 0)
        pMac->hphy.eepromSize = (tANI_U16)((HAL_EEPROM_SIZE / 1024) * 1024);
    
    if ((status = palAllocateMemory(pMac->hHdd, (void **)&pMac->hphy.pEepromCache,
        pMac->hphy.eepromSize)) == eHAL_STATUS_SUCCESS)
    {
        //HALLOGE(halLog(pMac, LOGE, FL("eeprom open alloc mem %d failed\n"),pMac->hphy.eepromSize));
        return status;
    }

    if (!pEepromEntries || (pEepromEntries->fields.eepSize == 0))
    {
        HALLOGE(halLog(pMac, LOGE, FL("Wrong eeprom size, filling the default values...\n")));

        if ((status = palCopyMemory(pMac->hHdd, (void *)&pMac->hphy.pEepromCache->fields,
            (void *)&defCommonFields, sizeof(sEepromFields))) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, FL("eeprom open palCopyMemory failed1\n")));
            return status;
        }
    }
    else
    {
        if ((status = palCopyMemory(pMac->hHdd, (void *)&pMac->hphy.pEepromCache->fields,
            (void *)&pEepromEntries->fields, sizeof(sEepromFields)))
            != eHAL_STATUS_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, FL("eeprom open palCopyMemory failed2\n")));
            return status;
        }
    }
    return status;
}

/*
 * free EEPROM cache memory
 */
eHalStatus halEepromClose(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    if (pMac->hphy.pEepromCache)
    {
        retVal = palFreeMemory(pMac->hHdd, pMac->hphy.pEepromCache);
        pMac->hphy.pEepromCache = NULL;
    }
    else
    {
        //assert(0);
        HALLOGE(halLog(pMac, LOGE, FL("WARN: attempting to free NULL ptr in halEepromClose")));
    }

    return (retVal);
}




static eHalStatus GetFieldLoc(tpAniSirGlobal pMac, eEepromField field, void **cacheData, size_t *dataSize)
{
    sEepromFields *commonFields = &pMac->hphy.pEepromCache->fields;

    assert(pMac->hphy.pEepromCache != NULL);

    switch (field)
    {
        case EEPROM_COMMON_SDRAM_INFO:
            *cacheData = &commonFields->sdramInfo;
            *dataSize = sizeof(commonFields->sdramInfo);
            break;

        case EEPROM_COMMON_MAC_ADDR:
            *cacheData = &commonFields->macAddr[0];
            *dataSize = sizeof(commonFields->macAddr);
            break;

        case EEPROM_COMMON_COUNTRY_CODE:
            *cacheData = &commonFields->countryCode[0];
            *dataSize = sizeof(commonFields->countryCode);
            break;

        case EEPROM_COMMON_CARD_TYPE:
            *cacheData = &commonFields->cardType;
            *dataSize = sizeof(commonFields->cardType);
            break;

        case EEPROM_COMMON_NUM_OF_TX_CHAINS:
            *cacheData = &commonFields->numOfTxChains;
            *dataSize = sizeof(commonFields->numOfTxChains);
            break;

        case EEPROM_COMMON_NUM_OF_RX_CHAINS:
            *cacheData = &commonFields->numOfRxChains;
            *dataSize = sizeof(commonFields->numOfRxChains);
            break;

        case EEPROM_COMMON_PRODUCT_BANDS:
            *cacheData = &commonFields->productBands;
            *dataSize = sizeof(commonFields->productBands);
            break;

        case EEPROM_COMMON_PDADC_GAIN_2_4_GHZ:
            *cacheData = &commonFields->padcGain_24ghz;
            *dataSize = sizeof(commonFields->padcGain_24ghz);
            break;

        case EEPROM_COMMON_DEFAULT_REG_DOMAIN:
            *cacheData = &commonFields->defaultRegDomain;
            *dataSize = sizeof(commonFields->defaultRegDomain);
            break;

        case EEPROM_COMMON_NUM_STATIONS:
            *cacheData = &commonFields->numStations;
            *dataSize = sizeof(commonFields->numStations);
            break;

        default:
            return (eHAL_STATUS_FAILURE);
    }

    return (eHAL_STATUS_SUCCESS);
}


eHalStatus halGetEepromFieldSize(tHalHandle hMac, eEepromField field, tANI_U32 *fieldSize)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_FAILURE;
    void *cacheData = NULL;

    if (pMac->hphy.pEepromCache == NULL)
    {
        return (eHAL_STATUS_FAILURE);
    }

    retVal = GetFieldLoc(pMac, field, &cacheData, (size_t *)fieldSize);

    return (retVal);
}


eHalStatus halReadEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    void *cacheData = NULL;
    size_t dataSize;

    if (pMac->hphy.pEepromCache == NULL)
    {
        return (eHAL_STATUS_FAILURE);
    }

    if (GetFieldLoc(pMac, field, &cacheData, &dataSize) == eHAL_STATUS_SUCCESS)
    {
        //copy cached value to pointer
        assert(cacheData != NULL);

        memcpy(fieldData, cacheData, dataSize);
        return (eHAL_STATUS_SUCCESS);
    }
    else
    {
        return (eHAL_STATUS_FAILURE);
    }
}


eHalStatus halWriteEepromField(tHalHandle hMac, eEepromField field, uEepromFields *fieldData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    void *cacheData = NULL;
    size_t dataSize;

    if (pMac->hphy.pEepromCache == NULL)
    {
        return (eHAL_STATUS_FAILURE);
    }

    if (GetFieldLoc(pMac, field, &cacheData, &dataSize) == eHAL_STATUS_SUCCESS)
    {
        //copy fieldData to cacheData
        assert(cacheData != NULL);

        if (dataSize == 1)
        {
            HALLOG1( halLog(hMac, LOG1, FL("Setting EEPROM field %d = %d at location %p"),  field, *(tANI_U8 *)fieldData, cacheData ));
        }

        memcpy(cacheData, fieldData, dataSize);
        return (eHAL_STATUS_SUCCESS);
    }
    else
    {
        return (eHAL_STATUS_FAILURE);
    }
}


eHalStatus halStoreTableToEeprom(tHalHandle hMac, eEepromTable tableID)
{
    return eHAL_STATUS_SUCCESS;
}



eHalStatus halGetEepromTableLoc(tHalHandle hMac, eEepromTable eepromTable, uEepromTables **tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    if (eepromTable < NUM_EEPROM_TABLE_IDS)
    {
        *tableData = (uEepromTables *)pMac->hphy.eepromTables[eepromTable];
        if (*tableData == NULL)
        {
            return (eHAL_STATUS_FAILURE);
        }
        else
        {
            return (eHAL_STATUS_SUCCESS);
        }
    }
    else
    {
        *tableData = NULL;
        return (eHAL_STATUS_FAILURE);
    }
}

#if 0
static void logEepromCommonFields(tpAniSirGlobal pMac, sEepromFields *pFields)
{
    HALLOGE( halLog(pMac, LOGE, FL("%d = productId\n"),  pFields->productId ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = eepImageVersion\n"),  pFields->eepImageVersion ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = eepSize\n"),  pFields->eepSize ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = cardType\n"),  pFields->cardType ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = numOfTxChains\n"),  pFields->numOfTxChains ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = numOfRxChains\n"),  pFields->numOfRxChains ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = productBands\n"),  pFields->productBands ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = padcGain_24ghz\n"),  pFields->padcGain_24ghz ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = sdramInfo\n"),  pFields->sdramInfo ));
    HALLOGE( halLog(pMac, LOGE, FL("%X %X %X %X %X %X = macAddr\n"),
                            pFields->macAddr[0],
                            pFields->macAddr[1],
                            pFields->macAddr[2],
                            pFields->macAddr[3],
                            pFields->macAddr[4],
                            pFields->macAddr[5]
          ));
    HALLOGE( halLog(pMac, LOGE, FL("%s = countryCode\n"),  &pFields->countryCode[0] ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = defaultRegDomain\n"),  pFields->defaultRegDomain ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = numStations\n"),  pFields->numStations ));
    HALLOGE( halLog(pMac, LOGE, FL("%d = cksumPreceding\n"),  pFields->cksumPreceding ));
}
#endif
