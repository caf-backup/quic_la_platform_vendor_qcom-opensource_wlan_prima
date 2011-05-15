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

    if (vos_nv_getValidity(VNV_TPC_POWER_TABLE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_TPC_POWER_TABLE, (v_VOID_t *)&pMac->hphy.nvCache.tables.plutCharacterized[0], NULL, sizeof(tTpcPowerTable) * NUM_2_4GHZ_CHANNELS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_TPC_PDADC_OFFSETS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_TPC_PDADC_OFFSETS, (v_VOID_t *)&pMac->hphy.nvCache.tables.plutPdadcOffset[0], NULL, sizeof(tANI_U16) * NUM_2_4GHZ_CHANNELS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

 //   if (vos_nv_getValidity(VNV_CAL_MEMORY, &itemIsValid) == VOS_STATUS_SUCCESS)
 //   {
 //       if (itemIsValid == VOS_TRUE)
 //       {
 //           if(vos_nv_read( VNV_CAL_MEMORY, (v_VOID_t *)&pMac->hphy.nvCache.tables.calFlashMemory, NULL, sizeof(sCalFlashMemory) ) != VOS_STATUS_SUCCESS)
 //                return (eHAL_STATUS_FAILURE);
 //       }
 //   }

 //   if (vos_nv_getValidity(VNV_CAL_STATUS, &itemIsValid) == VOS_STATUS_SUCCESS)
 //   {
 //       if (itemIsValid == VOS_TRUE)
 //       {
 //           if(vos_nv_read( VNV_DEFAULT_LOCATION, (v_VOID_t *)&pMac->hphy.nvCache.tables.calStatus, NULL, sizeof(sCalStatus) ) != VOS_STATUS_SUCCESS)
 //                return (eHAL_STATUS_FAILURE);
 //       }
 //   }

    if (vos_nv_getValidity(VNV_RSSI_CHANNEL_OFFSETS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_RSSI_CHANNEL_OFFSETS, (v_VOID_t *)&pMac->hphy.nvCache.tables.rssiChanOffsets[0], NULL, sizeof(sRssiChannelOffsets) * 2 ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_RF_CAL_VALUES, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_RF_CAL_VALUES, (v_VOID_t *)&pMac->hphy.nvCache.tables.rFCalValues, NULL, sizeof(sRFCalValues) ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_ANTENNA_PATH_LOSS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_ANTENNA_PATH_LOSS, (v_VOID_t *)&pMac->hphy.nvCache.tables.antennaPathLoss[0], NULL, sizeof(t2Decimal) * NUM_2_4GHZ_CHANNELS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_PACKET_TYPE_POWER_LIMITS, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_PACKET_TYPE_POWER_LIMITS, (v_VOID_t *)&pMac->hphy.nvCache.tables.pktTypePwrLimits[0][0], NULL, sizeof(t2Decimal) * NUM_802_11_MODES * NUM_2_4GHZ_CHANNELS ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_OFDM_CMD_PWR_OFFSET, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_OFDM_CMD_PWR_OFFSET, (v_VOID_t *)&pMac->hphy.nvCache.tables.ofdmCmdPwrOffset, NULL, sizeof(sOfdmCmdPwrOffset) ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_TX_BB_FILTER_MODE, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_TX_BB_FILTER_MODE, (v_VOID_t *)&pMac->hphy.nvCache.tables.txbbFilterMode, NULL, sizeof(sTxBbFilterMode) ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }

    if (vos_nv_getValidity(VNV_FREQUENCY_FOR_1_3V_SUPPLY, &itemIsValid) == VOS_STATUS_SUCCESS)
    {
        if (itemIsValid == VOS_TRUE)
        {
            if(vos_nv_read( VNV_FREQUENCY_FOR_1_3V_SUPPLY, (v_VOID_t *)&pMac->hphy.nvCache.tables.freqFor1p3VSupply, NULL, sizeof(sFreqFor1p3VSupply) ) != VOS_STATUS_SUCCESS)
                 return (eHAL_STATUS_FAILURE);
        }
    }


}
#endif //(defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_AMSS))
    pMac->hphy.nvTables[NV_FIELDS_IMAGE             ] = &pMac->hphy.nvCache.fields;
    pMac->hphy.nvTables[NV_TABLE_RATE_POWER_SETTINGS] = &pMac->hphy.nvCache.tables.pwrOptimum[0];
    pMac->hphy.nvTables[NV_TABLE_REGULATORY_DOMAINS ] = &pMac->hphy.nvCache.tables.regDomains[0];
    pMac->hphy.nvTables[NV_TABLE_DEFAULT_COUNTRY    ] = &pMac->hphy.nvCache.tables.defaultCountryTable;
    pMac->hphy.nvTables[NV_TABLE_TPC_POWER_TABLE] = &pMac->hphy.nvCache.tables.plutCharacterized[0];
    pMac->hphy.nvTables[NV_TABLE_TPC_PDADC_OFFSETS  ] = &pMac->hphy.nvCache.tables.plutPdadcOffset[0];
    //pMac->hphy.nvTables[NV_TABLE_CAL_MEMORY         ] = &pMac->hphy.nvCache.tables.calFlashMemory;
    //pMac->hphy.nvTables[NV_TABLE_CAL_STATUS         ] = &pMac->hphy.nvCache.tables.calStatus;
    pMac->hphy.nvTables[NV_TABLE_RSSI_CHANNEL_OFFSETS] = &pMac->hphy.nvCache.tables.rssiChanOffsets[0];
    pMac->hphy.nvTables[NV_TABLE_RF_CAL_VALUES] = &pMac->hphy.nvCache.tables.rFCalValues;
    pMac->hphy.nvTables[NV_TABLE_ANTENNA_PATH_LOSS  ] = &pMac->hphy.nvCache.tables.antennaPathLoss[0];
    pMac->hphy.nvTables[NV_TABLE_PACKET_TYPE_POWER_LIMITS  ] = &pMac->hphy.nvCache.tables.pktTypePwrLimits[0][0];
    pMac->hphy.nvTables[NV_TABLE_OFDM_CMD_PWR_OFFSET  ] = &pMac->hphy.nvCache.tables.ofdmCmdPwrOffset;
    pMac->hphy.nvTables[NV_TABLE_TX_BB_FILTER_MODE  ] = &pMac->hphy.nvCache.tables.txbbFilterMode;
    pMac->hphy.nvTables[NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY  ] = &pMac->hphy.nvCache.tables.freqFor1p3VSupply;


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

        case NV_COMMON_WLAN_NV_REV_ID:
            *cacheData = &pMac->hphy.nvCache.fields.wlanNvRevId;
            *dataSize = sizeof(tANI_U8);
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
#ifndef VERIFY_HALPHY_SIMV_MODEL // SIMV doesn't compile vos_nv_items
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    VOS_STATUS vosStatus;


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

            case NV_TABLE_TPC_POWER_TABLE:
                if ((vosStatus = vos_nv_write(VNV_TPC_POWER_TABLE, (void *)&pMac->hphy.nvCache.tables.plutCharacterized[0], sizeof(tTpcPowerTable) * NUM_2_4GHZ_CHANNELS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_TPC_PDADC_OFFSETS:
                if ((vosStatus = vos_nv_write(VNV_TPC_PDADC_OFFSETS, (void *)&pMac->hphy.nvCache.tables.plutPdadcOffset[0], sizeof(tANI_U16) * NUM_2_4GHZ_CHANNELS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

      //      case NV_TABLE_CAL_MEMORY:
      //          if ((vosStatus = vos_nv_write(VNV_CAL_MEMORY, (void *)&pMac->hphy.nvCache.tables.calFlashMemory, sizeof(sCalFlashMemory))) != VOS_STATUS_SUCCESS)
      //          {
      //              return (eHAL_STATUS_FAILURE);
      //          }
      //          break;

      //      case NV_TABLE_CAL_STATUS:
      //          if ((vosStatus = vos_nv_write(VNV_CAL_STATUS, (void *)&pMac->hphy.nvCache.tables.calStatus, sizeof(sCalStatus))) != VOS_STATUS_SUCCESS)
      //          {
      //              return (eHAL_STATUS_FAILURE);
      //          }
      //          break;

            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                if ((vosStatus = vos_nv_write(VNV_RSSI_CHANNEL_OFFSETS, (void *)&pMac->hphy.nvCache.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * 2)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_RF_CAL_VALUES:
                if ((vosStatus = vos_nv_write(VNV_RF_CAL_VALUES, (void *)&pMac->hphy.nvCache.tables.rFCalValues, sizeof(sRFCalValues))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_ANTENNA_PATH_LOSS:
                if ((vosStatus = vos_nv_write(VNV_ANTENNA_PATH_LOSS, (void *)&pMac->hphy.nvCache.tables.antennaPathLoss[0], sizeof(t2Decimal) * NUM_2_4GHZ_CHANNELS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_PACKET_TYPE_POWER_LIMITS:
                if ((vosStatus = vos_nv_write(VNV_PACKET_TYPE_POWER_LIMITS, (void *)&pMac->hphy.nvCache.tables.pktTypePwrLimits[0][0], sizeof(t2Decimal) * NUM_802_11_MODES * NUM_2_4GHZ_CHANNELS)) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_OFDM_CMD_PWR_OFFSET:
                if ((vosStatus = vos_nv_write(VNV_OFDM_CMD_PWR_OFFSET, (void *)&pMac->hphy.nvCache.tables.ofdmCmdPwrOffset, sizeof(sOfdmCmdPwrOffset))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_TX_BB_FILTER_MODE:
                if ((vosStatus = vos_nv_write(VNV_TX_BB_FILTER_MODE, (void *)&pMac->hphy.nvCache.tables.txbbFilterMode, sizeof(sTxBbFilterMode))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;

            case NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY:
                if ((vosStatus = vos_nv_write(VNV_FREQUENCY_FOR_1_3V_SUPPLY, (void *)&pMac->hphy.nvCache.tables.freqFor1p3VSupply, sizeof(sFreqFor1p3VSupply))) != VOS_STATUS_SUCCESS)
                {
                    return (eHAL_STATUS_FAILURE);
                }
                break;



            default:
                return (eHAL_STATUS_FAILURE);
                break;
        }
    }
#endif
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

#ifndef VERIFY_HALPHY_SIMV_MODEL
    assert(nvTable < NUM_NV_TABLE_IDS);
#endif

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

            return halIsQFuseBlown(hMac);
*/

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

        case NV_TABLE_RATE_POWER_SETTINGS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.pwrOptimum[0], sizeof(tRateGroupPwr) * NUM_RF_SUBBANDS);
            break;

        case NV_TABLE_REGULATORY_DOMAINS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.regDomains[0], sizeof(sRegulatoryDomains) * NUM_REG_DOMAINS);
            break;

        case NV_TABLE_DEFAULT_COUNTRY:
            memcpy(tableData, &pMac->hphy.nvCache.tables.defaultCountryTable, sizeof(sDefaultCountry));
            break;

        case NV_TABLE_TPC_POWER_TABLE:
            memcpy(tableData, &pMac->hphy.nvCache.tables.plutCharacterized[0], sizeof(tTpcPowerTable) * NUM_2_4GHZ_CHANNELS);
            break;

        case NV_TABLE_TPC_PDADC_OFFSETS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.plutPdadcOffset[0], sizeof(tANI_U16) * NUM_2_4GHZ_CHANNELS);
            break;

    //    case NV_TABLE_CAL_MEMORY:
    //        memcpy(tableData, &pMac->hphy.nvCache.tables.calFlashMemory, sizeof(sCalFlashMemory));
    //        break;

    //    case NV_TABLE_CAL_STATUS:
    //        memcpy(tableData, &pMac->hphy.nvCache.tables.calStatus, sizeof(sCalStatus));
    //        break;

        case NV_TABLE_RSSI_CHANNEL_OFFSETS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * 2);
            break;

        case NV_TABLE_RF_CAL_VALUES:
            memcpy(tableData, &pMac->hphy.nvCache.tables.rFCalValues, sizeof(sRFCalValues));
            break;

        case NV_TABLE_ANTENNA_PATH_LOSS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.antennaPathLoss[0], sizeof(t2Decimal) * NUM_2_4GHZ_CHANNELS);
            break;

        case NV_TABLE_PACKET_TYPE_POWER_LIMITS:
            memcpy(tableData, &pMac->hphy.nvCache.tables.pktTypePwrLimits[0][0], sizeof(t2Decimal) * NUM_802_11_MODES * NUM_2_4GHZ_CHANNELS);
            break;

        case NV_TABLE_OFDM_CMD_PWR_OFFSET:
            memcpy(tableData, &pMac->hphy.nvCache.tables.ofdmCmdPwrOffset, sizeof(sOfdmCmdPwrOffset));
            break;

        case NV_TABLE_TX_BB_FILTER_MODE:
            memcpy(tableData, &pMac->hphy.nvCache.tables.txbbFilterMode, sizeof(sTxBbFilterMode));
            break;

        case NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY:
            memcpy(tableData, &pMac->hphy.nvCache.tables.freqFor1p3VSupply, sizeof(sFreqFor1p3VSupply));
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

            case NV_TABLE_TPC_POWER_TABLE:
                numOfEntries = NUM_2_4GHZ_CHANNELS;
                sizeOfEntry = sizeof(tTpcPowerTable);
                break;

            case NV_TABLE_TPC_PDADC_OFFSETS:
                numOfEntries = NUM_2_4GHZ_CHANNELS;
                sizeOfEntry = sizeof(tANI_U16);
                break;

      //      case NV_TABLE_CAL_MEMORY:
      //          numOfEntries = 1;
      //          sizeOfEntry = sizeof(sCalFlashMemory);
      //          break;

      //      case NV_TABLE_CAL_STATUS:
      //          numOfEntries = 1;
      //          sizeOfEntry = sizeof(sCalStatus);
      //          break;

            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                numOfEntries = 2;
                sizeOfEntry = sizeof(sRssiChannelOffsets);
                break;

            case NV_TABLE_RF_CAL_VALUES:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sRFCalValues);
                break;

            case NV_TABLE_ANTENNA_PATH_LOSS:
                numOfEntries = NUM_2_4GHZ_CHANNELS;
                sizeOfEntry = sizeof(t2Decimal);
                break;

            case NV_TABLE_PACKET_TYPE_POWER_LIMITS:
                numOfEntries = NUM_802_11_MODES * NUM_2_4GHZ_CHANNELS;
                sizeOfEntry = sizeof(t2Decimal);
                break;

            case NV_TABLE_OFDM_CMD_PWR_OFFSET:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sOfdmCmdPwrOffset);
                break;

            case NV_TABLE_TX_BB_FILTER_MODE:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sTxBbFilterMode);
                break;
            case NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY:
                numOfEntries = 1;
                sizeOfEntry = sizeof(sFreqFor1p3VSupply);
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


    return retVal;
}

#ifndef WLAN_FTM_STUB
eHalStatus halRemoveNvTable(tHalHandle hMac, eNvTable nvTable)
{
#ifndef VERIFY_HALPHY_SIMV_MODEL
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus;

    switch (nvTable)
    {
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

            case NV_TABLE_TPC_POWER_TABLE:
                if ((vosStatus = vos_nv_setValidity(VNV_TPC_POWER_TABLE, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.plutCharacterized[0], &nvDefaults.tables.plutCharacterized[0], sizeof(tTpcPowerTable) * NUM_2_4GHZ_CHANNELS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_TPC_PDADC_OFFSETS:
                if ((vosStatus = vos_nv_setValidity(VNV_TPC_PDADC_OFFSETS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.plutPdadcOffset[0], &nvDefaults.tables.plutPdadcOffset[0], sizeof(tANI_U16) * NUM_2_4GHZ_CHANNELS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

      //      case NV_TABLE_CAL_MEMORY:
      //          if ((vosStatus = vos_nv_setValidity(VNV_CAL_MEMORY, VOS_FALSE)) == VOS_STATUS_SUCCESS)
      //          {
      //              memcpy(&pMac->hphy.nvCache.tables.calFlashMemory, &nvDefaults.tables.calFlashMemory, sizeof(sCalFlashMemory));
      //          }
      //          else
      //          {
      //              return (eHAL_STATUS_FAILURE);
      //          }
      //
      //          break;

      //      case NV_TABLE_CAL_STATUS:
      //          if ((vosStatus = vos_nv_setValidity(VNV_CAL_STATUS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
      //          {
      //              memcpy(&pMac->hphy.nvCache.tables.calStatus, &nvDefaults.tables.calStatus, sizeof(sCalStatus));
      //          }
      //          else
      //          {
      //              return (eHAL_STATUS_FAILURE);
      //          }
      //
      //          break;

            case NV_TABLE_RSSI_CHANNEL_OFFSETS:
                if ((vosStatus = vos_nv_setValidity(VNV_RSSI_CHANNEL_OFFSETS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.rssiChanOffsets[0], &nvDefaults.tables.rssiChanOffsets[0], sizeof(sRssiChannelOffsets) * 2);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_RF_CAL_VALUES:
                if ((vosStatus = vos_nv_setValidity(VNV_RF_CAL_VALUES, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.rFCalValues, &nvDefaults.tables.rFCalValues, sizeof(sRFCalValues));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_ANTENNA_PATH_LOSS:
                if ((vosStatus = vos_nv_setValidity(VNV_ANTENNA_PATH_LOSS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.antennaPathLoss[0], &nvDefaults.tables.antennaPathLoss[0], sizeof(t2Decimal) * NUM_2_4GHZ_CHANNELS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_PACKET_TYPE_POWER_LIMITS:
                if ((vosStatus = vos_nv_setValidity(VNV_PACKET_TYPE_POWER_LIMITS, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.pktTypePwrLimits[0][0], &nvDefaults.tables.pktTypePwrLimits[0][0], sizeof(t2Decimal) * NUM_802_11_MODES * NUM_2_4GHZ_CHANNELS);
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_OFDM_CMD_PWR_OFFSET:
                if ((vosStatus = vos_nv_setValidity(VNV_OFDM_CMD_PWR_OFFSET, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.ofdmCmdPwrOffset, &nvDefaults.tables.ofdmCmdPwrOffset, sizeof(sOfdmCmdPwrOffset));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;

            case NV_TABLE_TX_BB_FILTER_MODE:
                if ((vosStatus = vos_nv_setValidity(VNV_TX_BB_FILTER_MODE, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.txbbFilterMode, &nvDefaults.tables.txbbFilterMode, sizeof(sTxBbFilterMode));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;
            case NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY:
                if ((vosStatus = vos_nv_setValidity(VNV_FREQUENCY_FOR_1_3V_SUPPLY, VOS_FALSE)) == VOS_STATUS_SUCCESS)
                {
                    memcpy(&pMac->hphy.nvCache.tables.freqFor1p3VSupply, &nvDefaults.tables.freqFor1p3VSupply, sizeof(sFreqFor1p3VSupply));
                }
                else
                {
                    return (eHAL_STATUS_FAILURE);
                }

                break;



        default:
            return (eHAL_STATUS_FAILURE);
            break;
    }


    return (retVal);
#else
    return eHAL_STATUS_SUCCESS;
#endif
}

eHalStatus halBlankNv(tHalHandle hMac)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;
    //tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;

    halRemoveNvTable(hMac, NV_FIELDS_IMAGE                );
    halRemoveNvTable(hMac, NV_TABLE_RATE_POWER_SETTINGS   );
    halRemoveNvTable(hMac, NV_TABLE_REGULATORY_DOMAINS    );
    halRemoveNvTable(hMac, NV_TABLE_DEFAULT_COUNTRY       );
    halRemoveNvTable(hMac, NV_TABLE_TPC_POWER_TABLE       );
    halRemoveNvTable(hMac, NV_TABLE_TPC_PDADC_OFFSETS     );
    //halRemoveNvTable(hMac, NV_TABLE_CAL_MEMORY            );
    //halRemoveNvTable(hMac, NV_TABLE_CAL_STATUS            );
    halRemoveNvTable(hMac, NV_TABLE_RSSI_CHANNEL_OFFSETS  );
    halRemoveNvTable(hMac, NV_TABLE_RF_CAL_VALUES  );
    halRemoveNvTable(hMac, NV_TABLE_ANTENNA_PATH_LOSS     );
    halRemoveNvTable(hMac, NV_TABLE_PACKET_TYPE_POWER_LIMITS     );
    halRemoveNvTable(hMac, NV_TABLE_OFDM_CMD_PWR_OFFSET   );
    halRemoveNvTable(hMac, NV_TABLE_TX_BB_FILTER_MODE     );
    halRemoveNvTable(hMac, NV_TABLE_FREQUENCY_FOR_1_3V_SUPPLY     );


    return (retVal);
}
#endif

void halByteSwapNvTable(tHalHandle hMac, eNvTable tableID, uNvTables *tableData)
{

    switch (tableID)
    {
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


        case NV_FIELDS_IMAGE:
        {
            sHalNv *nv = (sHalNv *)tableData;

            BYTE_SWAP_S(nv->fields.productId);
            break;
        }

        case NV_TABLE_TPC_PDADC_OFFSETS:
        {
            tANI_U8 j;

            for (j = 0; j < NUM_2_4GHZ_CHANNELS; j++)
            {
                BYTE_SWAP_S(tableData->plutPdadcOffset[j]);
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

