/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file halNv.c

    \brief Nv facilities

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */
//#include "halInternal.h"

#include "string.h"
#include "aniGlobal.h"
#include "ani_assert.h"
#include "halDebug.h"
#include "halNvApi.h"
#include <halPhyUtil.h>
//#include <vos_nvitem.h>
#include "phyApi.h"

extern const sHalNv nvDefaults;

//static eHalStatus CacheTables(tHalHandle hMac);

eHalStatus halNvOpen(tHalHandle hMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    memcpy(&pMac->hphy.nvCache, &nvDefaults, sizeof(sHalNv));   //start with defaults
/*Once tested for other platform android specific flag can be removed*/

#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_AMSS))
{
    v_BOOL_t itemIsValid = VOS_FALSE;

    if (vos_nv_getValidity(VNV_FIELD_IMAGE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE) {

            if(vos_nv_read( VNV_FIELD_IMAGE, (v_VOID_t *)&pMac->hphy.nvCache.fields, NULL, sizeof(sNvFields) ) != VOS_STATUS_SUCCESS)
              return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_RATE_TO_POWER_TABLE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
             if(vos_nv_read( VNV_RATE_TO_POWER_TABLE, (v_VOID_t *)&pMac->hphy.nvCache.tables.pwrOptimum[0], NULL, sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_REGULARTORY_DOMAIN_TABLE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {

        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_REGULARTORY_DOMAIN_TABLE, (v_VOID_t *)&pMac->hphy.nvCache.tables.regDomains[0], NULL, sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS ) != VOS_STATUS_SUCCESS)
                return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_DEFAULT_LOCATION, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_DEFAULT_LOCATION, (v_VOID_t *)&pMac->hphy.nvCache.tables.defaultCountryTable, NULL, sizeof(sDefaultCountry) ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_RSSI_OFFSETS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
    	{
            if(vos_nv_read( VNV_RSSI_OFFSETS, (v_VOID_t *)&pMac->hphy.nvCache.tables.rssiOffset[0], NULL, sizeof(tANI_S16) * PHY_MAX_RX_CHAINS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

#ifdef ANI_OS_TYPE_ANDROID
    if (vos_nv_getValidity(VNV_RSSI_CHANNEL_OFFSETS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_RSSI_CHANNEL_OFFSETS, (v_VOID_t *)&pMac->hphy.nvCache.tables.rssiChanOffsets[0], NULL, sizeof(sRssiChannelOffsets) * PHY_MAX_RX_CHAINS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }
#endif
}
#endif
    pMac->hphy.nvTables[NV_FIELDS_IMAGE              ] = &pMac->hphy.nvCache.fields;
    pMac->hphy.nvTables[NV_TABLE_QFUSE               ] = &pMac->hphy.nvCache.tables.qFuseData;
    pMac->hphy.nvTables[NV_TABLE_RATE_POWER_SETTINGS ] = &pMac->hphy.nvCache.tables.pwrOptimum[0];
    pMac->hphy.nvTables[NV_TABLE_REGULATORY_DOMAINS  ] = &pMac->hphy.nvCache.tables.regDomains[0];
    pMac->hphy.nvTables[NV_TABLE_DEFAULT_COUNTRY     ] = &pMac->hphy.nvCache.tables.defaultCountryTable;
    pMac->hphy.nvTables[NV_TABLE_TPC_CONFIG          ] = &pMac->hphy.nvCache.tables.tpcConfig[0];
    pMac->hphy.nvTables[NV_TABLE_RF_CAL_VALUES       ] = &pMac->hphy.nvCache.tables.rfCalValues;
    pMac->hphy.nvTables[NV_TABLE_RSSI_OFFSETS        ] = &pMac->hphy.nvCache.tables.rssiOffset[0];
    pMac->hphy.nvTables[NV_TABLE_RSSI_CHANNEL_OFFSETS] = &pMac->hphy.nvCache.tables.rssiChanOffsets[0];

    return status;
}

/*
 * free NV cache memory
 */
eHalStatus halNvClose(tHalHandle hMac)
{
    //tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    return (retVal);
}


// static eHalStatus CacheTables(tHalHandle hMac)
// {
//     eHalStatus retVal = eHAL_STATUS_SUCCESS;
//     tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
//
//     if (halIsQFuseBlown(hMac) == eHAL_STATUS_FAILURE)
//     {
// #ifndef ANI_MANF_DIAG
// #ifdef ANI_PHY_DEBUG
//         HALLOGE(halLog(pMac, LOGE, "ERROR: qFuse is empty. Continuing with uncalibrated transmit power\n"));
//         pMac->hphy.nvTables[NV_TABLE_QFUSE] = NULL;
// #endif
// #endif
//     }
//     else
//     {
//         pMac->hphy.nvTables[NV_TABLE_QFUSE] = &pMac->hphy.nvCache.tables.qFuseData;
//     }
//
//     return retVal;
// }


static eHalStatus GetFieldLoc(tpAniSirGlobal pMac, eNvField field, void **cacheData, size_t *dataSize)
{

    switch (field)
    {
        case NV_COMMON_PRODUCT_ID:
            *cacheData = &pMac->hphy.nvCache.fields.productId;
            *dataSize = sizeof(tANI_U16);
            break;
        case NV_COMMON_PRODUCT_BANDS:
            *cacheData = &pMac->hphy.nvCache.fields.productBands;
            *dataSize = sizeof(tANI_U8);
            break;
        case NV_COMMON_NUM_OF_TX_CHAINS:
            *cacheData = &pMac->hphy.nvCache.fields.numOfTxChains;
            *dataSize = sizeof(tANI_U8);
            break;
        case NV_COMMON_NUM_OF_RX_CHAINS:
            *cacheData = &pMac->hphy.nvCache.fields.numOfRxChains;
            *dataSize = sizeof(tANI_U8);
            break;
        case NV_COMMON_MAC_ADDR:
            *cacheData = &pMac->hphy.nvCache.fields.macAddr[0];
            *dataSize = NV_FIELD_MAC_ADDR_SIZE;
            break;
        case NV_COMMON_MFG_SERIAL_NUMBER:
            *cacheData = &pMac->hphy.nvCache.fields.mfgSN[0];
            *dataSize = NV_FIELD_MFG_SN_SIZE;
            break;

        default:
            return (eHAL_STATUS_FAILURE);
            break;
    }

    return (eHAL_STATUS_SUCCESS);
}


eHalStatus halGetNvFieldSize(tHalHandle hMac, eNvField field, tANI_U32 *fieldSize)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_FAILURE;
    void *cacheData = NULL;

    retVal = GetFieldLoc(pMac, field, &cacheData, (size_t *)fieldSize);

    return (retVal);
}


eHalStatus halReadNvField(tHalHandle hMac, eNvField field, uNvFields *fieldData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    void *cacheData = NULL;
    size_t dataSize;

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


eHalStatus halWriteNvField(tHalHandle hMac, eNvField field, uNvFields *fieldData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    void *cacheData = NULL;
    size_t dataSize;

    if (GetFieldLoc(pMac, field, &cacheData, &dataSize) == eHAL_STATUS_SUCCESS)
    {
        //copy fieldData to cacheData
        assert(cacheData != NULL);

        if (dataSize == 1)
        {
            //HALLOGE(halLog(hMac, LOGE, "Setting NV field %d = %d at location %p", field, fieldData->productBands, cacheData));   //product bands happens to be one byte
        }

        memcpy(cacheData, fieldData, dataSize);
        return (eHAL_STATUS_SUCCESS);
    }
    else
    {
        return (eHAL_STATUS_FAILURE);
    }
}

#ifndef WLAN_FTM_STUB

eHalStatus halStoreTableToNv(tHalHandle hMac, eNvTable tableID)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    VOS_STATUS vosStatus;


    if ((tableID == NV_TABLE_QFUSE) && (halIsQFuseBlown(hMac) == eHAL_STATUS_FAILURE))
    {
        return (halQFuseWrite(hMac));    //blow QFuse permanently
    }
    else
    {
        switch (tableID)
        {
            case NV_FIELDS_IMAGE:
                if ((vosStatus = vos_nv_write(VNV_FIELD_IMAGE, (void *)&pMac->hphy.nvCache.fields, sizeof(sNvFields))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_RATE_POWER_SETTINGS:
                if ((vosStatus = vos_nv_write(VNV_RATE_TO_POWER_TABLE, (void *)&pMac->hphy.nvCache.tables.pwrOptimum[0], sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_REGULATORY_DOMAINS:
                if ((vosStatus = vos_nv_write(VNV_REGULARTORY_DOMAIN_TABLE, (void *)&pMac->hphy.nvCache.tables.regDomains[0], sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_DEFAULT_COUNTRY:
                if ((vosStatus = vos_nv_write(VNV_DEFAULT_LOCATION, (void *)&pMac->hphy.nvCache.tables.defaultCountryTable, sizeof(sDefaultCountry))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_RSSI_OFFSETS:
                if ((vosStatus = vos_nv_write(VNV_RSSI_OFFSETS, (void *)&pMac->hphy.nvCache.tables.rssiOffset[0], sizeof(tANI_S16) * PHY_MAX_RX_CHAINS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

#ifdef ANI_OS_TYPE_ANDROID
            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                if ((vosStatus = vos_nv_write(VNV_RSSI_CHANNEL_OFFSETS, (void *)&pMac->hphy.nvCache.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * PHY_MAX_RX_CHAINS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;
#endif

            case NV_TABLE_TPC_CONFIG:       //stored through QFUSE
            case NV_TABLE_RF_CAL_VALUES:    //stored through QFUSE
            default:
                return (eHAL_STATUS_FAILURE);
                break;
        }
    }
    return eHAL_STATUS_SUCCESS;
}
#endif


eHalStatus halGetNvTableLoc(tHalHandle hMac, eNvTable nvTable, uNvTables **tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    if (nvTable < NUM_NV_TABLE_IDS)
    {
        *tableData = (uNvTables *)pMac->hphy.nvTables[nvTable];
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

eHalStatus halIsTableInNv(tHalHandle hMac, eNvTable nvTable)
{
    //tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    //VOS_STATUS vosStatus;
    //v_BOOL_t pItemIsValid;

    assert(nvTable < NUM_NV_TABLE_IDS);

    switch (nvTable)
    {
/*          TODO: Awaiting vos nv functionality to be checked in by Simon Ho
            case NV_FIELDS_IMAGE:
                if ((vosStatus = vos_nv_getValidity(VNV_FIELD_IMAGE, &pItemIsValid)) == VOS_STATUS_SUCCESS)
                {
                    if (pItemIsValid == VOS_TRUE)
                    {
                        return (eHAL_STATUS_SUCCESS);   //success means it is stored
                    }
                }

                return (eHAL_STATUS_FAILURE);

            case NV_TABLE_RATE_POWER_SETTINGS:
                if ((vosStatus = vos_nv_getValidity(VNV_RATE_TO_POWER_TABLE, &pItemIsValid)) == VOS_STATUS_SUCCESS)
                {
                    if (pItemIsValid == VOS_TRUE)
                    {
                        return (eHAL_STATUS_SUCCESS);   //success means it is stored
                    }
                }

                return (eHAL_STATUS_FAILURE);

            case NV_TABLE_REGULATORY_DOMAINS:
                if ((vosStatus = vos_nv_getValidity(VNV_REGULARTORY_DOMAIN_TABLE, &pItemIsValid)) == VOS_STATUS_SUCCESS)
                {
                    if (pItemIsValid == VOS_TRUE)
                    {
                        return (eHAL_STATUS_SUCCESS);   //success means it is stored
                    }
                }

                return (eHAL_STATUS_FAILURE);

            case NV_TABLE_DEFAULT_COUNTRY:
                if ((vosStatus = vos_nv_getValidity(VNV_DEFAULT_LOCATION, &pItemIsValid)) == VOS_STATUS_SUCCESS)
                {
                    if (pItemIsValid == VOS_TRUE)
                    {
                        return (eHAL_STATUS_SUCCESS);   //success means it is stored
                    }
                }

                return (eHAL_STATUS_FAILURE);
*/

        case NV_TABLE_QFUSE:
        case NV_TABLE_TPC_CONFIG:
        case NV_TABLE_RF_CAL_VALUES:
            return halIsQFuseBlown(hMac);

        default:
            return (eHAL_STATUS_FAILURE);
    }
}


eHalStatus halReadNvTable(tHalHandle hMac, eNvTable nvTable, uNvTables *tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    switch (nvTable)
    {
        case NV_FIELDS_IMAGE:
            memcpy(tableData, &pMac->hphy.nvCache.fields, sizeof(sNvFields));
            break;

        case NV_TABLE_QFUSE:
            memcpy(tableData, &pMac->hphy.nvCache.tables.qFuseData, sizeof(sQFuseConfig));
            retVal = halIsQFuseBlown(pMac);
            break;

        case NV_TABLE_RATE_POWER_SETTINGS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.pwrOptimum[0], sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS);
            break;

        case NV_TABLE_REGULATORY_DOMAINS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.regDomains[0], sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS);
            break;

        case NV_TABLE_DEFAULT_COUNTRY:
            memcpy(tableData, &pMac->hphy.nvCache.tables.defaultCountryTable, sizeof(sDefaultCountry));
            break;

        case NV_TABLE_TPC_CONFIG:
            memcpy(tableData, &pMac->hphy.nvCache.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS);
            break;

        case NV_TABLE_RF_CAL_VALUES:
            memcpy(tableData, &pMac->hphy.nvCache.tables.rfCalValues, sizeof(sRfNvCalValues));
            break;

        case NV_TABLE_RSSI_OFFSETS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.rssiOffset[0], sizeof(tANI_S16) * PHY_MAX_RX_CHAINS);
            break;

        case NV_TABLE_RSSI_CHANNEL_OFFSETS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * PHY_MAX_RX_CHAINS);
            break;

        default:
            return (eHAL_STATUS_FAILURE);
            break;
    }

    return (retVal);
} // halReadNvTable


eHalStatus halWriteNvTable(tHalHandle hMac, eNvTable nvTable, uNvTables *tableData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    tANI_U16 numOfEntries;
    tANI_U16 sizeOfEntry;

    if (nvTable == NV_TABLE_QFUSE)
    {
        if (halIsQFuseBlown(hMac) == eHAL_STATUS_SUCCESS)
        {
            //qFuse has already been written
            HALLOGE(halLog(pMac, LOGE, "WARNING: overwriting qfuse cache which was already blown\n"));
        }
        numOfEntries = 1;
        sizeOfEntry = sizeof(sQFuseConfig);
    }
    else
    {
        switch (nvTable)
        {
            case NV_FIELDS_IMAGE:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sNvFields);
                break;

            case NV_TABLE_RATE_POWER_SETTINGS:
                numOfEntries = NUM_RF_SUBBANDS;
                sizeOfEntry = sizeof(tRateGroupPwr);
                break;

            case NV_TABLE_REGULATORY_DOMAINS:
                numOfEntries = NUM_REG_DOMAINS;
                sizeOfEntry = sizeof(sRegulatoryDomains);
                break;

            case NV_TABLE_DEFAULT_COUNTRY:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sDefaultCountry);
                break;

            case NV_TABLE_TPC_CONFIG:
                numOfEntries = 2;
                sizeOfEntry = sizeof(tTpcConfig);
                break;

            case NV_TABLE_RF_CAL_VALUES:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sRfNvCalValues);
                break;

            case NV_TABLE_RSSI_OFFSETS:
                numOfEntries = 2;
                sizeOfEntry = sizeof(tANI_S16);
                break;

            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                numOfEntries = 2;
                sizeOfEntry = sizeof(sRssiChannelOffsets);
                break;

            default:
                return (eHAL_STATUS_FAILURE);
                break;
        }
    }
    retVal = palCopyMemory(pMac->hHdd,
                           (tANI_U8 *)pMac->hphy.nvTables[nvTable],
                           tableData,
                           (numOfEntries * sizeOfEntry)
                          );
    if (retVal == eHAL_STATUS_SUCCESS)
    {
        switch (nvTable)
        {
            case NV_TABLE_TPC_CONFIG:
            case NV_TABLE_RF_CAL_VALUES:
                //if either of these change from previous values, then reconfigure TPC so it stays up to date.
                if ((retVal = ConfigureTpcFromNv(pMac)) != eHAL_STATUS_SUCCESS) { return (retVal); }

                break;
            default:
                break;
        }
    }



    return retVal;
}

#ifndef WLAN_FTM_STUB

eHalStatus halRemoveNvTable(tHalHandle hMac, eNvTable nvTable)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus;

    switch (nvTable)
    {
        case NV_TABLE_QFUSE:
            if (halIsQFuseBlown(hMac) == eHAL_STATUS_SUCCESS)
            {
                //qFuse has already been written
                HALLOGE(halLog(pMac, LOGE, "ERROR: QFUSE already blown, can't remove\n"));
            }
            else
            {
                memcpy(&pMac->hphy.nvCache.tables.tpcConfig[0], &nvDefaults.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS);
                memcpy(&pMac->hphy.nvCache.tables.rfCalValues, &nvDefaults.tables.rfCalValues, sizeof(sRfNvCalValues));
                halQFusePackBits(hMac);
            }

            break;

            case NV_FIELDS_IMAGE:
                if ((vosStatus = vos_nv_setValidity(VNV_FIELD_IMAGE, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.fields, &nvDefaults.fields, sizeof(sNvFields));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_RATE_POWER_SETTINGS:
                if ((vosStatus = vos_nv_setValidity(VNV_RATE_TO_POWER_TABLE, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.pwrOptimum[0], &nvDefaults.tables.pwrOptimum[0], sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_REGULATORY_DOMAINS:
                if ((vosStatus = vos_nv_setValidity(VNV_REGULARTORY_DOMAIN_TABLE, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.regDomains[0], &nvDefaults.tables.regDomains[0], sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_DEFAULT_COUNTRY:
                if ((vosStatus = vos_nv_setValidity(VNV_DEFAULT_LOCATION, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.defaultCountryTable, &nvDefaults.tables.defaultCountryTable, sizeof(sDefaultCountry));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

        case NV_TABLE_RSSI_OFFSETS:
            if ((vosStatus = vos_nv_setValidity(VNV_RSSI_OFFSETS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
            {
                memcpy(&pMac->hphy.nvCache.tables.rssiOffset[0], &nvDefaults.tables.rssiOffset[0], sizeof(tANI_S16) * PHY_MAX_RX_CHAINS);
            }
            else
            {
                return (eHAL_STATUS_FAILURE);
            }

            break;

#ifdef ANI_OS_TYPE_ANDROID
        case NV_TABLE_RSSI_CHANNEL_OFFSETS:
            if ((vosStatus = vos_nv_setValidity(VNV_RSSI_CHANNEL_OFFSETS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
            {
                memcpy(&pMac->hphy.nvCache.tables.rssiChanOffsets[0], &nvDefaults.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * PHY_MAX_RX_CHAINS);
            }
            else
            {
                return (eHAL_STATUS_FAILURE);
            }

            break;
#endif

        case NV_TABLE_TPC_CONFIG:
            memcpy(&pMac->hphy.nvCache.tables.tpcConfig[0], &nvDefaults.tables.tpcConfig[0], sizeof(tTpcConfig) * MAX_TPC_CHANNELS);
            halQFusePackBits(hMac);
            break;

        case NV_TABLE_RF_CAL_VALUES:
            memcpy(&pMac->hphy.nvCache.tables.rfCalValues, &nvDefaults.tables.rfCalValues, sizeof(sRfNvCalValues));
            halQFusePackBits(hMac);
            break;

        default:
            return (eHAL_STATUS_FAILURE);
            break;
    }


    return (retVal);
}


eHalStatus halBlankNv(tHalHandle hMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    //tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    halRemoveNvTable(hMac, NV_FIELDS_IMAGE              );
    halRemoveNvTable(hMac, NV_TABLE_QFUSE               );
    halRemoveNvTable(hMac, NV_TABLE_RATE_POWER_SETTINGS );
    halRemoveNvTable(hMac, NV_TABLE_REGULATORY_DOMAINS  );
    halRemoveNvTable(hMac, NV_TABLE_DEFAULT_COUNTRY     );
    halRemoveNvTable(hMac, NV_TABLE_TPC_CONFIG          );
    halRemoveNvTable(hMac, NV_TABLE_RF_CAL_VALUES       );
    halRemoveNvTable(hMac, NV_TABLE_RSSI_OFFSETS        );
    halRemoveNvTable(hMac, NV_TABLE_RSSI_CHANNEL_OFFSETS);

    return (retVal);
}
#endif

void halByteSwapNvTable(tHalHandle hMac, eNvTable tableID, uNvTables *tableData)
{

    switch (tableID)
    {
        case NV_TABLE_RF_CAL_VALUES:
        case NV_TABLE_RATE_POWER_SETTINGS:
        case NV_TABLE_DEFAULT_COUNTRY:
        {
            //only single bytes - nothing to reorder
            break;
        }

        case NV_TABLE_REGULATORY_DOMAINS:
        {
            tANI_U32 dom, i, subband;

            for (dom = 0; dom < NUM_REG_DOMAINS; dom++)
            {
                //channels array only has single bytes

                for (subband = RF_SUBBAND_2_4_GHZ; subband < NUM_RF_SUBBANDS; subband++)
                {
                        BYTE_SWAP_S(tableData->regDomains[dom].antennaGain[subband].reported);
                }
                for (i = 0; i < NUM_2_4GHZ_CHANNELS; i++)
                {
                    BYTE_SWAP_S(tableData->regDomains[dom].bRatePowerOffset[i].reported);
                }
            }

            break;
        }


        case NV_TABLE_TPC_CONFIG:
        {
            tANI_U32 i;

            for (i = 0; i < MAX_TPC_CHANNELS; i++)
            {
                BYTE_SWAP_S(tableData->tpcConfig[i].freq);
                BYTE_SWAP_S(tableData->tpcConfig[i].reserved);
                BYTE_SWAP_S(tableData->tpcConfig[i].absPower.min);
                BYTE_SWAP_S(tableData->tpcConfig[i].absPower.max);

                //empirical array is all single bytes
            }

            break;
        }

        case NV_FIELDS_IMAGE:
        {
            sHalNv *nv = (sHalNv *)tableData;

            BYTE_SWAP_S(nv->fields.productId);
            break;
        }

        case NV_TABLE_RSSI_OFFSETS:
        {
            tANI_U8 i;

            for (i = 0; i < PHY_MAX_RX_CHAINS; i++)
            {
                BYTE_SWAP_S(tableData->rssiOffset[i]);
            }
            break;
        }

        case NV_TABLE_RSSI_CHANNEL_OFFSETS:
        {
            tANI_U8 i, j;

            for (i = 0; i < PHY_MAX_RX_CHAINS; i++)
            {
                for (j = 0; j < NUM_2_4GHZ_CHANNELS; j++)
                {
                    BYTE_SWAP_S(tableData->rssiChanOffsets[i].bRssiOffset[j]);
                    BYTE_SWAP_S(tableData->rssiChanOffsets[i].gnRssiOffset[j]);
                }
            }
            break;
        }

        default:
            break;
    }
}

void halDumpNVtable(tHalHandle hMac, eNvTable tableID)
{
#ifdef WLAN_DEBUG
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
#endif
    switch (tableID)
    {
        case NV_TABLE_RF_CAL_VALUES:
        {
            break;
        }
        case NV_TABLE_RATE_POWER_SETTINGS:
        {
            int i;
            for(i = 0; i < NUM_RATE_POWER_GROUPS; i++)
            {
                HALLOGE(halLog(pMac, LOGE, "[%d]: Rate2Pwr: %d\n", i, pMac->hphy.nvCache.tables.pwrOptimum[0][i]));
            }
            break;
        }
        case NV_TABLE_DEFAULT_COUNTRY:
        {
            HALLOGE(halLog(pMac, LOGE, "%x %x %x\n", pMac->hphy.nvCache.tables.defaultCountryTable.countryCode[0], pMac->hphy.nvCache.tables.defaultCountryTable.countryCode[1], pMac->hphy.nvCache.tables.defaultCountryTable.countryCode[2]));
            break;
        }

        case NV_TABLE_REGULATORY_DOMAINS:
        {
            tANI_U32 dom, subband, chan;

            HALLOGE(halLog(pMac, LOGE, "Dumping NV_TABLE_REGULATORY_DOMAINS table...\n"));
            for (dom = 0; dom < NUM_REG_DOMAINS; dom++)
            {
                HALLOGE(halLog(pMac, LOGE, "regDomain: %d\n", dom));
                //channels array only has single bytes
                for(chan=0; chan<NUM_RF_CHANNELS;chan++)
                {
                    //dump sRegulatoryChannel
                    HALLOGE(halLog(pMac, LOGE, "[%d]: enabled: %d pwrLimit: %d\n", chan, pMac->hphy.nvCache.tables.regDomains[dom].channels[chan].enabled, pMac->hphy.nvCache.tables.regDomains[dom].channels[chan].pwrLimit));
                }
                for(subband = 0;subband < NUM_RF_SUBBANDS; subband++)
                {
                    //dump antenna gain
                    HALLOGE(halLog(pMac, LOGE, "[%d]: antennaGain: %d \n", subband, pMac->hphy.nvCache.tables.regDomains[dom].antennaGain[subband]));
                }
                for(chan=0; chan<NUM_2_4GHZ_CHANNELS;chan++)
                {
                    //dump bRatePowerOffset, gnRatePowerOffset
                    HALLOGE(halLog(pMac, LOGE, "[%d]: bRatePowerOffset: %d  gnRatePowerOffset: %d\n", chan, pMac->hphy.nvCache.tables.regDomains[dom].bRatePowerOffset[chan], pMac->hphy.nvCache.tables.regDomains[dom].gnRatePowerOffset[chan]));
                }
            }

            break;
        }


        case NV_TABLE_TPC_CONFIG:
        {

                //empirical array is all single bytes
            break;
        }

        case NV_FIELDS_IMAGE:
        {
            int i;
            HALLOGE(halLog(pMac, LOGE, "productId: %d\n",pMac->hphy.nvCache.fields.productId));
            HALLOGE(halLog(pMac, LOGE, "productBands: %d\n",pMac->hphy.nvCache.fields.productBands));
            HALLOGE(halLog(pMac, LOGE, "numOfTxChains: %d\n",pMac->hphy.nvCache.fields.numOfTxChains));
            HALLOGE(halLog(pMac, LOGE, "numOfRxChains: %d\n",pMac->hphy.nvCache.fields.numOfRxChains));
            HALLOGE(halLog(pMac, LOGE, "macAddr: %x %x %x %x %x %x\n",
                            pMac->hphy.nvCache.fields.macAddr[0],
                            pMac->hphy.nvCache.fields.macAddr[1],
                            pMac->hphy.nvCache.fields.macAddr[2],
                            pMac->hphy.nvCache.fields.macAddr[3],
                            pMac->hphy.nvCache.fields.macAddr[4],
                            pMac->hphy.nvCache.fields.macAddr[5]));

            HALLOGE(halLog(pMac, LOGE, "mfgSN: "));
            for(i = 0; i < NV_FIELD_MFG_SN_SIZE; i++ )
            {
                HALLOGE(halLog(pMac, LOGE, "%x\n",pMac->hphy.nvCache.fields.mfgSN[i]));
            }
            break;
        }

        case NV_TABLE_RSSI_OFFSETS:
        {
            tANI_U8 i;

            for (i = 0; i < PHY_MAX_RX_CHAINS; i++)
            {
                HALLOGE(halLog(pMac, LOGE, "[%d]: rssi: %d\n", i, pMac->hphy.nvCache.tables.rssiOffset[i]));
            }
            break;
        }

        case NV_TABLE_RSSI_CHANNEL_OFFSETS:
        {
            tANI_U8 i, j;

            for (i = 0; i < PHY_MAX_RX_CHAINS; i++)
            {
                for (j = 0; j < NUM_2_4GHZ_CHANNELS; j++)
                {
                    HALLOGE(halLog(pMac, LOGE, "[%d]: rssi bOffset: %d, gnOffset: %d\n", i, pMac->hphy.nvCache.tables.rssiChanOffsets[i].bRssiOffset[j], pMac->hphy.nvCache.tables.rssiChanOffsets[i].gnRssiOffset[j]));
                }
            }
            break;
        }

        default:
            break;
    }
}
