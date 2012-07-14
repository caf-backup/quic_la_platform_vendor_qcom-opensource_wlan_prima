/**
 *
 *  @file:         halApi.c
 *
 *  @brief:       Provide HAL APIs.
 *
 *  @author:    Susan Tsao
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/06/2006  File created.
 * 01/24/2008  Virgo related chages.
 */
#include "halInternal.h"
#include "halMTU.h"
#include "halRpe.h"
#include "halDebug.h"
#include "cfgApi.h"
#include "halPhyApi.h"
#ifdef WLAN_SOFTAP_FEATURE
#include "halHddApis.h"
#endif

#include <ani_assert.h>

/// convert the CW values into a tANI_U16
#define GET_CW(pCw) ((tANI_U16) ((*(pCw) << 8) + *((pCw) + 1)))

#ifdef WLAN_SOFTAP_FEATURE
#define HAL_DEFAULT_BC_STA_ID (0x00)
#endif

/**
 * \fn   halGlobalInit
 *
 * \brief   Initializes all the HAL module global variables in this funciton.
 *            Reads from EEPROM, updates global informaiton of transmit power control
 *            interpolation and those interpolated will interpolated across the
 *            all the valid channels as well.
 *
 * \param pMac A pointer to the tAniSirGlobal structure
 *
 * \return Success or Failure
 */
eHalStatus halGlobalInit( tpAniSirGlobal pMac )
{
    eHalStatus  rc = eHAL_STATUS_SUCCESS;

    // Start up in eSYSTEM_STA_ROLE, we start with this as the default
    // even when we support Multi-Bss.
    pMac->hal.halGlobalSystemRole = eSYSTEM_STA_ROLE; 

    pMac->hal.currentRfBand = eRF_BAND_UNKNOWN;
    pMac->hal.currentChannel = 0;
    pMac->hal.currentCBState = PHY_SINGLE_CHANNEL_CENTERED;
    
#if defined(ANI_PRODUCT_TYPE_CLIENT)
    pMac->hal.cfgTxAntenna = WNI_CFG_CURRENT_TX_ANTENNA_STAMAX;
    pMac->hal.cfgRxAntenna = WNI_CFG_CURRENT_RX_ANTENNA_STAMAX;
#endif

    return rc;
}

/**
 * \fn      halSetPowerSaveMode
 *
 * \brief  This function Sets the Power Save State to the PHY Layer.
 *
 * \param powerState- State to which the PHY needs to be set
 *
 * \return Success or Failure
 */

eHalStatus halSetPowerSaveMode(tpAniSirGlobal  pMac, tSirMacHTMIMOPowerSaveState powerState)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    switch (powerState) {
    case eSIR_HT_MIMO_PS_DYNAMIC:
        //retVal =  halPhySetPowerSave(pMac, PHY_POWER_DYNAMIC_RX_SM) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Dynamic SM PS Mode \n")));
        break;

    case eSIR_HT_MIMO_PS_STATIC:
        //retVal =  halPhySetPowerSave(pMac, PHY_POWER_STATIC_RX_SM) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Static SM PS Mode \n")));
        break;

    default:
        //retVal =  halPhySetPowerSave(pMac, PHY_POWER_NORMAL) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Normal SM PS Mode \n")));
    break;
    }

    return retVal;
}

/* ---------------------------------------------------------
 * FUNCTION:  halGetBssSystemRole()
 *
 * Get the BSS specific HAL system role. 
 * ---------------------------------------------------------
 */
tBssSystemRole halGetBssSystemRole(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    return  (t[bssIdx].bssSystemRole);
}

/* ---------------------------------------------------------
 * FUNCTION:  halGetBssSystemRole()
 *
 * Get the BSS specific HAL system role. 
 * ---------------------------------------------------------
 */
tANI_U8 halGetHalBssPersona(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    return  (t[bssIdx].halBssPersona);
}


/* ---------------------------------------------------------
 * FUNCTION:  halSetBssSystemRole()
 *
 * Set the BSS specific HAL system role. Pass the 
 * bssIdx to the function to access the BSS specific table.
 * ---------------------------------------------------------
 */
void halSetBssSystemRole(tpAniSirGlobal pMac, tBssSystemRole role, 
    tANI_U8 bssIdx, tANI_U8 bssPersona)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

//    assert(i < pMac->hal.halMac.maxBssId);

    t[bssIdx].bssSystemRole = role;
    t[bssIdx].halBssPersona = bssPersona;
		

}

/* ---------------------------------------------------------
 * FUNCTION:  halGetBssLinkState()
 *
 * ---------------------------------------------------------
 */
tSirLinkState halGetBssLinkState(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

   // assert(i < pMac->hal.halMac.maxBssId);

    return  (t[bssIdx].bssLinkState);
}

/* ---------------------------------------------------------
 * FUNCTION:  halSetBssLinkState()
 *
 * ---------------------------------------------------------
 */
void halSetBssLinkState(tpAniSirGlobal pMac, tSirLinkState linkstate, 
    tANI_U8 bssIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    t[bssIdx].bssLinkState = linkstate;
}

/* ---------------------------------------------------------
 * FUNCTION:  halGetGlobalSystemRole()
 *
 * Get the global HAL system role. 
 * ---------------------------------------------------------
 */
tBssSystemRole halGetGlobalSystemRole(tpAniSirGlobal pMac)
{
    return  (pMac->hal.halGlobalSystemRole);
}

/* ---------------------------------------------------------
 * FUNCTION:  halSetGlobalSystemRole()
 *
 * Set the HAL global system role. This is the full system role.
 * When there are multiple BSS this reflects with a multi bss
 * system role.
 * ---------------------------------------------------------
 */
void halSetGlobalSystemRole(tpAniSirGlobal pMac, tBssSystemRole role)
{
    tANI_U8 activeBssCnt = 0;
    int i;
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;

    halMTU_GetActiveBss(pMac, &activeBssCnt);

    if (activeBssCnt == 0) 
    {
        // Ok set it to the default.
        pMac->hal.halGlobalSystemRole = eSYSTEM_STA_ROLE; 
        return;
    }
         
    if (activeBssCnt == 1) 
    {
        if (role != eSYSTEM_UNKNOWN_ROLE)
        {
            // BSS specific role when only 1 BSS active.
            pMac->hal.halGlobalSystemRole = role;
            return;
        }
        // Implies we are deleting the current one and 
        // there is still one more BSS active.
        HALLOG1( halLog( pMac, LOG1, "Maximum BSS Table size - %d\n", pMac->hal.halMac.maxBssId ));
        for( i=0; i < pMac->hal.halMac.maxBssId; i++ )
        {
            if( bssTable[i].valid == 0) continue;
            // OK so we have the entry we need.
            pMac->hal.halGlobalSystemRole = bssTable[i].bssSystemRole;
            HALLOG1( halLog( pMac, LOG1, "Set global system role - %d\n", 
                    pMac->hal.halGlobalSystemRole ));

            return;
        }
    }

    // Cant be > 2, we dont support it right now.
    //assert (activeBssCnt == 2);

    // If there are more than 1 active BSS, then set it to Multi.
    pMac->hal.halGlobalSystemRole = eSYSTEM_MULTI_BSS_ROLE;
}

/* ---------------------------------------------------------
 * FUNCTION:  halGetBssSystemRoleFromStaIdx()
 *
 * ---------------------------------------------------------
 */
tBssSystemRole halGetBssSystemRoleFromStaIdx(tpAniSirGlobal pMac, 
        tANI_U8 staIdx)
{
    tANI_U8     bssIdx;
    tBssSystemRole systemRole;
    eHalStatus status=eHAL_STATUS_SUCCESS;

    if (staIdx != HAL_STA_INVALID_IDX) {
        if ((status = halTable_GetBssIndexForSta(pMac, &bssIdx, 
                    staIdx)) == eHAL_STATUS_SUCCESS) {
            // Find the system role of the bss and return that.
            return (halGetBssSystemRole(pMac, bssIdx));
        }
    }

    // Ok so the BssIdx is not set as yet so we are not in multi bss
    // mode yet for sure. So return the global HAL system role.
    systemRole = halGetGlobalSystemRole(pMac);

    return (systemRole);
}


/* ---------------------------------------------------------
 * FUNCTION:  halGetHalPersonaFromStaIdx()
 *
 * ---------------------------------------------------------
 */
tANI_U8 halGetHalPersonaFromStaIdx(tpAniSirGlobal pMac, 
                                   tANI_U8 staIdx)
{
    tANI_U8     bssIdx;
    tANI_U8     halPersona = HAL_BSSPERSONA_INVALID_IDX;
    eHalStatus status=eHAL_STATUS_SUCCESS;

    if (staIdx != HAL_STA_INVALID_IDX) {
        if ((status = halTable_GetBssIndexForSta(pMac, &bssIdx, 
                    staIdx)) == eHAL_STATUS_SUCCESS) {
            // Find the BSS persona of the bss and return that.
            return (halGetHalBssPersona(pMac, bssIdx));
        }
    }

    return (halPersona);
}

static void
getMaxBssSta(tpAniSirGlobal pMac, tANI_U32 *maxBSS, tANI_U32 *maxSTA)
{
#ifdef FIXME_GEN6 //FIXME - do we need to read this value from the FLASH file?
    {
        //if EEPROM_TABLE_MULT_BSSID exists, multBssTable will not be NULL
        //returns eHAL_STATUS_FAILURE and multBssTable will be NULL if the table doesn't exist
        halGetEepromTableLoc(pMac, EEPROM_TABLE_MULT_BSSID, (uEepromTables **)&pMac->hal.multBssTable);

        if (pMac->hal.multBssTable)
        {
            if (pMac->hal.multBssTable->numBss <= HAL_NUM_BSSID)
            {
                *maxBSS = pMac->hal.multBssTable->numBss;
                halLog(pMac, LOG1, "Multiple BSS Table contains %d BSSIDs\n", pMac->hal.multBssTable->numBss);
            }
            else
            {
                halLog(pMac, LOGE, "ERROR: Multiple BSS Table has too many entries, %d\n", pMac->hal.multBssTable->numBss);
                halLog(pMac, LOGE, "Defaulting to 16 BSSID using mac address\n");
                *maxBSS = 16;            // Set to minimum
            }
        }
        else
        {
            *maxBSS = 16;            // Set to minimum
        }
    }

    {
        uEepromFields numStations;

        if (halReadEepromField(pMac, EEPROM_COMMON_NUM_STATIONS, &numStations) == eHAL_STATUS_SUCCESS)
        {
            tANI_U32 numSta;
            numSta = numStations.numStations;
#ifdef FIXME_GEN5
            if (numSta <= HAL_NUM_STA)
            {
                *maxSTA = numSta;
            }
            else
            {
                *maxSTA = HAL_NUM_STA;
            }
#endif
            *maxSTA = HAL_NUM_STA;
        }
    }
#else
    *maxSTA = HAL_NUM_STA;
    *maxBSS = HAL_NUM_BSSID;

#endif
}

tANI_U32 getInternalMemory(tpAniSirGlobal pMac)
{
#ifdef FIXME_GEN6 //FIXME - do we need to read this value from the FLASH file?
    uEepromFields sdramInfo;

    if (halReadEepromField(pMac, EEPROM_COMMON_INTERNAL_SRAM_SIZE, (uEepromFields *)&sdramInfo) == eHAL_STATUS_SUCCESS)
    {
        if (sdramInfo.sdramInfo)
            return BMU_INTERNAL_MEMORY_SIZE_128K;
        else
            return BMU_INTERNAL_MEMORY_SIZE_512K;
    }
    else // default to 512K
        return BMU_INTERNAL_MEMORY_SIZE_512K;
#endif
    
#ifdef DAFCA_TRACE_DEBUG_ENABLE
    /* If DAFCA trace debug is enabled, then 128K memory
     * is allocated for debug. Hence, that leaves only 128K 
     * remaining for internal memory.
     */
#ifdef WLAN_HAL_VOLANS
    return pMac->hal.memMap.packetMemory_endAddr;
#else
    return BMU_INTERNAL_MEMORY_SIZE_128K;
#endif
#else
#ifdef WLAN_HAL_VOLANS
    return pMac->hal.memMap.packetMemory_endAddr;
#else
    return BMU_INTERNAL_MEMORY_SIZE_256K;
#endif
#endif

}

static void
getTotalMemory(tpAniSirGlobal pMac, tANI_U32 *totalMem)
{
#ifdef FIXME_GEN6 //FIXME - do we need to read this value from the FLASH file?
    uEepromFields sdramInfo;

    if ((halReadEepromField(pMac, EEPROM_COMMON_SDRAM_INFO, &sdramInfo) == eHAL_STATUS_SUCCESS) && (sdramInfo.sdramInfo < NUM_SDRAM_OPTIONS))
    {
        switch (sdramInfo.sdramInfo)
        {
            case SDRAM_NOT_PRESENT:
                *totalMem = getInternalMemory(pMac);   // No external memory
                break;
            case SDRAM_FPGA:
            case SDRAM_AEVB:
                *totalMem = BMU_TOTAL_MEMORY_SIZE;
                break;
            default:
                halLog(pMac, LOGE, "ERROR: sdramInfo need to be programmed to a valid value\n");
                break;
        }
    }
    else
    {
        halLog(pMac, LOGE, "ERROR: Improper SDRAM option %d", sdramInfo.sdramInfo);
        *totalMem = getInternalMemory(pMac);   // No external memory
    }
#else
    *totalMem = BMU_TOTAL_MEMORY_SIZE;
#endif

}

static void
getMaxHwQueues(tpAniSirGlobal pMac, tANI_U32 *maxHwQueues)
{
    *maxHwQueues = HW_MAX_QUEUES;
}


/*
 * Initialize the Memory base address, the total available memory
 */
eHalStatus halMemoryMap_Open(tHalHandle hHal, void *arg)
{
    tANI_U32    maxBSS, maxSTA, totalMem, maxHwQueues;
    tMacOpenParameters *pMacOpenParams = (tMacOpenParameters *)arg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    getMaxBssSta(pMac, &maxBSS, &maxSTA);
    getMaxHwQueues(pMac, &maxHwQueues);

    pMac->hal.memMap.maxBssids      = maxBSS;
    pMac->hal.memMap.maxStations    = maxSTA;
    pMac->hal.memMap.maxDpuEntries  = pMac->hal.memMap.maxBssids+ pMac->hal.memMap.maxStations;
    pMac->hal.memMap.maxHwQueues = maxHwQueues;

    pMacOpenParams->maxBssId   = (tANI_U16) maxBSS;
    pMacOpenParams->maxStation = (tANI_U16) maxSTA;

    pMac->hal.memMap.memory_baseAddress = BMU_MEMORY_BASE_ADDRESS;
    pMac->hal.memMap.internalMemory_size = getInternalMemory(pMac);

    getTotalMemory(pMac, &totalMem);
    pMac->hal.memMap.totalMemory_size = totalMem;


    return eHAL_STATUS_SUCCESS;
}

static inline void halMemoryMap_ShowInfo(tpAniSirGlobal pMac, QWlanfw_MemMapInfo *pMemMapInfo)
{ 
    HALLOGW( halLog(pMac, LOGW, FL("fwState = 0x%x\n"), sirSwapU32(pMemMapInfo->fwState)));
    HALLOGW( halLog(pMac, LOGW, FL("fwVersion = 0x%x\n"), sirSwapU32(pMemMapInfo->fwVersion)));
    HALLOGW( halLog(pMac, LOGW, FL("fwStartAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->fwStartAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("fwEndAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->fwEndAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("fwTextStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwTextStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwTextEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwTextEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwBssStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwBssStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwBssEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwBssEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwDataStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwDataStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwDataEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwDataEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwInitStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwInitStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwInitEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwInitEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwBootImageStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwBootImageStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwBootImageEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwBootImageEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyInitStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyInitStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyInitEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyInitEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyDataStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyDataStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyDataEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyDataEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyPMTableStart = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyPMTableStart)));
    HALLOGW( halLog(pMac, LOGW, FL("fwHPhyPMTableEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->fwHPhyPMTableEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("gcStart = 0x%x\n"), sirSwapU32(pMemMapInfo->gcStart)));
    HALLOGW( halLog(pMac, LOGW, FL("gcEnd = 0x%x\n"), sirSwapU32(pMemMapInfo->gcEnd)));
    HALLOGW( halLog(pMac, LOGW, FL("mmapStartAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->mmapStartAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("h2FMsgAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->h2FMsgAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("f2HMsgAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->f2HMsgAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("fwLogAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->fwLogAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("errStatsAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->errStatsAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("phyCalStatusAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->phyCalStatusAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("phyCalCtrlBmapAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->phyCalCtrlBmapAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("phyCalCorrAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->phyCalCorrAddr)));
    /* Don't modify order of vars above this line, used for PDTE bench test */
    HALLOGW( halLog(pMac, LOGW, FL("hwCountersAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->hwCountersAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("sysCfgAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->sysCfgAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("fwPSCountersAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->fwPSCountersAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("phyFtmInfoAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->phyFtmInfoAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("raTableAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->raTableAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("uCodesAddr = 0x%x\n"), sirSwapU32(pMemMapInfo->uCodesAddr)));
    HALLOGW( halLog(pMac, LOGW, FL("reserved1 = 0x%x\n"), sirSwapU32(pMemMapInfo->reserved1)));
    HALLOGW( halLog(pMac, LOGW, FL("reserved2 = 0x%x\n"), sirSwapU32(pMemMapInfo->reserved2)));
}

static inline QWlanfw_MemMapInfo * halMemoryMap_ExtractFromFw(tANI_U8 *pFwImage)
{
    return (QWlanfw_MemMapInfo *)(pFwImage + QWLANFW_MEMINFO_START_ADDR);
}

/*
 * Compute the start address of the HW descriptors based on the FW image size
 * Initialize the start address of all the descriptors in the device memory
 */
eHalStatus halMemoryMap_Start(tHalHandle hHal, void *arg)
{
    tHalMacStartParameters *pStartParams = (tHalMacStartParameters*)arg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    QWlanfw_MemMapInfo  *pMemMapInfo = NULL;
    tANI_U32 fwSize = 0;
    tANI_U32 currentOffset;
    (void) arg;
    
    // Sanity check on the FW image pointer and the size
    if (pStartParams->FW.pImage != NULL) {
        fwSize = pStartParams->FW.cbImage;
    }

    if (fwSize > pMac->hal.memMap.totalMemory_size) {
        HALLOGE(halLog (pMac, LOGE, 
                FL("ERROR: FW image size 0x%0x exceeds available memory 0x%x"),
                fwSize, pMac->hal.memMap.totalMemory_size ));
        return eHAL_STATUS_FAILURE;
    }

    pMemMapInfo = halMemoryMap_ExtractFromFw((tANI_U8 *)pStartParams->FW.pImage);

    halMemoryMap_ShowInfo(pMac, pMemMapInfo);

    // In FTM mode, no HW desrciptors are required. Only packet memory needs 
    // to be mapped
    if (pMac->gDriverType == eDRIVER_TYPE_MFG) {
        if (fwSize > HAL_PHY_GRAB_RAM_CAPTURE_START_ADDR) {
            HALLOGE(halLog (pMac, LOGE, 
                        FL("ERROR: FW image size 0x%0x exceeds available memory 0x%x in FTM mode !!!"),
                        fwSize, HAL_PHY_GRAB_RAM_CAPTURE_START_ADDR));
            return eHAL_STATUS_FAILURE;
        }

        pMac->hal.memMap.packetMemory_offset = ((fwSize)/HAL_BD_SIZE + 1) * HAL_BD_SIZE; //((currentOffset)/HAL_BD_SIZE + 1) * HAL_BD_SIZE;
        pMac->hal.memMap.packetMemory_endAddr = HAL_PHY_GRAB_RAM_CAPTURE_START_ADDR; //BMU_INTERNAL_MEMORY_SIZE_208K; //

        HALLOGE(halLog (pMac, LOGE, 
                FL("FTM mode: FWsize = 0x%0x, PacketMem-StarAddr = 0x%x, PacketMem-EndAddr = 0x%x, FTMInfoAddr = 0x%x\n"),
                (unsigned int)fwSize, (unsigned int)pMac->hal.memMap.packetMemory_offset, 
                (unsigned int)pMac->hal.memMap.packetMemory_endAddr, (unsigned int)QWLANFW_MEM_PHY_FTM_INFO_ADDR_OFFSET));

        return eHAL_STATUS_SUCCESS;
    }

#if defined(DAFCA_TRACE_DEBUG_ENABLE) && defined(WLAN_HAL_VOLANS)
    pMac->hal.memMap.descStartAddr = BMU_INTERNAL_MEMORY_SIZE_208K - MIN_DAFCA_MEMORY;
#else
    pMac->hal.memMap.descStartAddr = BMU_INTERNAL_MEMORY_SIZE_208K; //(((FW_IMAGE_MEMORY_BASE_ADDRESS+fwSize)+(HAL_MEM_BOUNDARY_ALIGN-1))/HAL_MEM_BOUNDARY_ALIGN) * HAL_MEM_BOUNDARY_ALIGN;
#endif

    // Compute the memory address from the end of the available memory

    HALLOGE(halLog(pMac, LOGE, FL("FW image size = %d (0x%x) bytes"), fwSize, fwSize));
    HALLOGE(halLog(pMac, LOGE, FL("HW Descriptor Start Addr = 0x%08x"), pMac->hal.memMap.descStartAddr));

    // Start the first HW descriptor (DPU) at the bottom of the memory
    pMac->hal.memMap.dpuDescriptor_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuDescriptor);
    pMac->hal.memMap.dpuDescriptor_offset = 
        (pMac->hal.memMap.descStartAddr - pMac->hal.memMap.dpuDescriptor_size);

    pMac->hal.memMap.keyDescriptor_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuKeyDescriptor);
    pMac->hal.memMap.keyDescriptor_offset = 
        (pMac->hal.memMap.dpuDescriptor_offset - pMac->hal.memMap.keyDescriptor_size);

    pMac->hal.memMap.micKey_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuMicKeyDescriptor);
    pMac->hal.memMap.micKey_offset = 
        (pMac->hal.memMap.keyDescriptor_offset - pMac->hal.memMap.micKey_size);

    pMac->hal.memMap.replayCounter_size =
        pMac->hal.memMap.maxDpuEntries * MAX_NUM_OF_TIDS * sizeof(tDpuReplayCounterDescriptor);
    pMac->hal.memMap.replayCounter_offset =
        (pMac->hal.memMap.micKey_offset - pMac->hal.memMap.replayCounter_size);

    pMac->hal.memMap.dxeRxDescInfo_size = DXE_RXDESC_INFO_SIZE;
    pMac->hal.memMap.dxeRxDescInfo_offset =
        (pMac->hal.memMap.replayCounter_offset - pMac->hal.memMap.dxeRxDescInfo_size);

#ifdef WLAN_HAL_VOLANS
    pMac->hal.memMap.dxeRxHiDescInfo_size = DXE_RXDESC_INFO_SIZE;
    pMac->hal.memMap.dxeRxHiDescInfo_offset =
        (pMac->hal.memMap.dxeRxDescInfo_offset - pMac->hal.memMap.dxeRxHiDescInfo_size);

    pMac->hal.memMap.tpeStaDesc_size =
        pMac->hal.memMap.maxStations * TPE_STA_DESC_AND_STATS_SIZE;
    pMac->hal.memMap.tpeStaDesc_offset =
        (pMac->hal.memMap.dxeRxHiDescInfo_offset - pMac->hal.memMap.tpeStaDesc_size);
#else
    pMac->hal.memMap.tpeStaDesc_size =
       pMac->hal.memMap.maxStations * TPE_STA_DESC_AND_STATS_SIZE;
    pMac->hal.memMap.tpeStaDesc_offset =
       pMac->hal.memMap.dxeRxDescInfo_offset - pMac->hal.memMap.tpeStaDesc_size;
#endif

#ifdef WLAN_SOFTAP_VSTA_FEATURE
    pMac->hal.memMap.rpeStaDesc_size =
       HAL_NUM_HW_STA * RPE_STA_DESC_ENTRY_SIZE;
#else
    pMac->hal.memMap.rpeStaDesc_size =
       pMac->hal.memMap.maxStations * RPE_STA_DESC_ENTRY_SIZE;
#endif
    pMac->hal.memMap.rpeStaDesc_offset =
        (pMac->hal.memMap.tpeStaDesc_offset - pMac->hal.memMap.rpeStaDesc_size);

    pMac->hal.memMap.rpePartialBitmap_size = RPE_PARTIAL_BITMAP_SIZE;
    pMac->hal.memMap.rpePartialBitmap_offset =
        (pMac->hal.memMap.rpeStaDesc_offset - pMac->hal.memMap.rpePartialBitmap_size);
    currentOffset = pMac->hal.memMap.rpePartialBitmap_offset;

#ifdef FEATURE_ON_CHIP_REORDERING
        pMac->hal.memMap.rpeReOrderSTADataStructure_size =
              MAX_NUM_OF_ONCHIP_REORDER_SESSIONS * RPE_REORDER_STA_DS_SIZE;
        pMac->hal.memMap.rpeReOrderSTADataStructure_offset =
            (currentOffset - pMac->hal.memMap.rpeReOrderSTADataStructure_size);
        currentOffset = pMac->hal.memMap.rpeReOrderSTADataStructure_offset;
#endif

#ifdef WLAN_SOFTAP_VSTA_FEATURE
    pMac->hal.memMap.btqmTxQueue_size =
       HAL_NUM_HW_STA * HW_MAX_QUEUES * BTQM_STA_QUEUE_ENTRY_SIZE;
#else
    pMac->hal.memMap.btqmTxQueue_size =
       pMac->hal.memMap.maxStations * HW_MAX_QUEUES * BTQM_STA_QUEUE_ENTRY_SIZE;
#endif
    pMac->hal.memMap.btqmTxQueue_offset =
        (currentOffset - pMac->hal.memMap.btqmTxQueue_size);

    pMac->hal.memMap.hwTemplate_size = HW_TEMPLATE_SIZE;
    pMac->hal.memMap.hwTemplate_offset =
        (pMac->hal.memMap.btqmTxQueue_offset - pMac->hal.memMap.hwTemplate_size);

    pMac->hal.memMap.swTemplate_size = SW_TEMPLATE_SIZE;
    pMac->hal.memMap.swTemplate_offset =
        (pMac->hal.memMap.hwTemplate_offset - pMac->hal.memMap.swTemplate_size);

    pMac->hal.memMap.beaconTemplate_size = pMac->hal.memMap.maxBssids * BEACON_TEMPLATE_SIZE;
    pMac->hal.memMap.beaconTemplate_offset = (pMac->hal.memMap.swTemplate_offset -
        pMac->hal.memMap.beaconTemplate_size);

    pMac->hal.memMap.raBssTable_size   = pMac->hal.memMap.maxBssids * RA_BSS_INFO_SIZE;
    pMac->hal.memMap.raBssTable_offset = pMac->hal.memMap.beaconTemplate_offset - pMac->hal.memMap.raBssTable_size;

    pMac->hal.memMap.raStaTable_size   = pMac->hal.memMap.maxStations * RA_STA_INFO_SIZE;
    pMac->hal.memMap.raStaTable_offset = pMac->hal.memMap.raBssTable_offset - pMac->hal.memMap.raStaTable_size;

#ifdef WLAN_SOFTAP_FEATURE
    pMac->hal.memMap.probeRspTemplate_size = pMac->hal.memMap.maxBssids * PROBE_RSP_TEMPLATE_MAX_SIZE;
    pMac->hal.memMap.probeRspTemplate_offset =
       pMac->hal.memMap.raStaTable_offset - pMac->hal.memMap.probeRspTemplate_size;
    
    pMac->hal.memMap.bssTable_size   = pMac->hal.memMap.maxBssids * BSS_INFO_SIZE;
    pMac->hal.memMap.bssTable_offset = pMac->hal.memMap.probeRspTemplate_offset - pMac->hal.memMap.bssTable_size;
    
    pMac->hal.memMap.staTable_size   = pMac->hal.memMap.maxStations * STA_INFO_SIZE;
    pMac->hal.memMap.staTable_offset = pMac->hal.memMap.bssTable_offset - pMac->hal.memMap.staTable_size;

    pMac->hal.memMap.aduUmaStaDesc_size = HAL_NUM_UMA_DESC_ENTRIES * ADU_UMA_STA_DESC_ENTRY_SIZE;
    pMac->hal.memMap.aduUmaStaDesc_offset =
        pMac->hal.memMap.staTable_offset - pMac->hal.memMap.aduUmaStaDesc_size;

#else 

    pMac->hal.memMap.aduUmaStaDesc_size = HAL_NUM_UMA_DESC_ENTRIES *
                                            ADU_UMA_STA_DESC_ENTRY_SIZE;
    pMac->hal.memMap.aduUmaStaDesc_offset =
        (pMac->hal.memMap.beaconTemplate_offset - pMac->hal.memMap.aduUmaStaDesc_size);
#endif    
    pMac->hal.memMap.aduRegRecfgTbl_size = ADU_REG_RECONFIG_TABLE_SIZE;
    pMac->hal.memMap.aduRegRecfgTbl_offset =
       (pMac->hal.memMap.aduUmaStaDesc_offset - pMac->hal.memMap.aduRegRecfgTbl_size);
    pMac->hal.memMap.aduRegRecfgTbl_curPtr = pMac->hal.memMap.aduRegRecfgTbl_offset;
    currentOffset = pMac->hal.memMap.aduRegRecfgTbl_offset;

#ifdef VOLANS_PHY_TX_OPT_ENABLED
    pMac->hal.memMap.aduPhyTxRegRecfgTbl_size = ADU_PHY_TX_REG_RECONFIG_TABLE_SIZE;
    pMac->hal.memMap.aduPhyTxRegRecfgTbl_offset = 
        (pMac->hal.memMap.aduRegRecfgTbl_offset - pMac->hal.memMap.aduPhyTxRegRecfgTbl_size);
    pMac->hal.memMap.aduPhyTxRegRecfgTbl_curPtr = pMac->hal.memMap.aduPhyTxRegRecfgTbl_offset;
    currentOffset = pMac->hal.memMap.aduPhyTxRegRecfgTbl_offset;
#endif /* VOLANS_PHY_TX_OPT_ENABLED */

    pMac->hal.memMap.packetMemory_offset = ((sirSwapU32(pMemMapInfo->fwEndAddr))/HAL_BD_SIZE + 1) * HAL_BD_SIZE; //((currentOffset)/HAL_BD_SIZE + 1) * HAL_BD_SIZE;
    pMac->hal.memMap.packetMemory_endAddr = currentOffset;

    HALLOGW( halLog(pMac, LOGW, FL("maxBss %u , maxSta %u\n"), pMac->hal.memMap.maxBssids, pMac->hal.memMap.maxStations));
        HALLOGW( halLog(pMac, LOGW, FL("DPU Desc offset 0x%x\n"), pMac->hal.memMap.dpuDescriptor_offset));
    HALLOGW( halLog(pMac, LOGW, FL("KEY Desc offset 0x%x\n"), pMac->hal.memMap.keyDescriptor_offset));
    HALLOGW( halLog(pMac, LOGW, FL("MIC KEY offset 0x%x\n"), pMac->hal.memMap.micKey_offset));
    HALLOGW( halLog(pMac, LOGW, FL("Replay Counters offset 0x%x\n"), pMac->hal.memMap.replayCounter_offset));
    HALLOGW( halLog(pMac, LOGW, FL("DxE Rx Desc offset 0x%x\n"), pMac->hal.memMap.dxeRxDescInfo_offset));
    HALLOGW( halLog(pMac, LOGW, FL("DxE Rx Desc size 0x%x\n"), pMac->hal.memMap.dxeRxDescInfo_size));
    HALLOGW( halLog(pMac, LOGW, FL("TPE STA Desc offset 0x%x\n"), pMac->hal.memMap.tpeStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("TPE STA Desc size 0x%x\n"), pMac->hal.memMap.tpeStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("RPE STA Desc Offset 0x%x\n"), pMac->hal.memMap.rpeStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("RPE STA Desc Size 0x%x\n"), pMac->hal.memMap.rpeStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("RPE Partial Bitmap Offset 0x%x\n"), pMac->hal.memMap.rpePartialBitmap_offset));
    HALLOGW( halLog(pMac, LOGW, FL("RPE Partial Bitmap Size 0x%x\n"), pMac->hal.memMap.rpePartialBitmap_size));
#ifdef FEATURE_ON_CHIP_REORDERING
        HALLOGW( halLog(pMac, LOGW, FL("RPE Reorder STA Data Structure offset 0x%x\n"), pMac->hal.memMap.rpeReOrderSTADataStructure_offset));
        HALLOGW( halLog(pMac, LOGW, FL("RPE Reorder STA Data Structure Table size 0x%x\n"), pMac->hal.memMap.rpeReOrderSTADataStructure_size));
#endif
    HALLOGW( halLog(pMac, LOGW, FL("BTQM Tx WQ offset 0x%x\n"), pMac->hal.memMap.btqmTxQueue_offset));
    HALLOGW( halLog(pMac, LOGW, FL("BTQM Tx WQ size 0x%x\n"), pMac->hal.memMap.btqmTxQueue_size));
    HALLOGW( halLog(pMac, LOGW, FL("HW Template offset 0x%x\n"), pMac->hal.memMap.hwTemplate_offset));
    HALLOGW( halLog(pMac, LOGW, FL("HW Template size 0x%x\n"), pMac->hal.memMap.hwTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("SW Template size 0x%x\n"), pMac->hal.memMap.swTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("Beacon Template offset 0x%x\n"), pMac->hal.memMap.beaconTemplate_offset));
    HALLOGW( halLog(pMac, LOGW, FL("Beacon Template size 0x%x\n"), pMac->hal.memMap.beaconTemplate_size));
#ifdef WLAN_SOFTAP_FEATURE    
    HALLOGW( halLog(pMac, LOGW, FL("ProbeRsp Template offset 0x%x\n"), pMac->hal.memMap.probeRspTemplate_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ProbeRsp Template size 0x%x\n"), pMac->hal.memMap.probeRspTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("bssTable offset 0x%x\n"), pMac->hal.memMap.bssTable_offset));    
    HALLOGW( halLog(pMac, LOGW, FL("bssTable size 0x%x\n"), pMac->hal.memMap.bssTable_size));
    HALLOGW( halLog(pMac, LOGW, FL("staTable offset 0x%x\n"), pMac->hal.memMap.staTable_offset));    
    HALLOGW( halLog(pMac, LOGW, FL("staTable size 0x%x\n"), pMac->hal.memMap.staTable_size));    
#endif    
    HALLOGW( halLog(pMac, LOGW, FL("ADU UMA STA Desc offset 0x%x\n"), pMac->hal.memMap.aduUmaStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU UMA STA Desc Size 0x%x\n"), pMac->hal.memMap.aduUmaStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Reg Recfg Table offset 0x%x\n"), pMac->hal.memMap.aduRegRecfgTbl_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Reg Recfg Table size 0x%x\n"), pMac->hal.memMap.aduRegRecfgTbl_size));
#ifdef VOLANS_PHY_TX_OPT_ENABLED
    HALLOGW( halLog(pMac, LOGW, FL("ADU Phy TX Reg Recfg Table offset 0x%x\n"), pMac->hal.memMap.aduPhyTxRegRecfgTbl_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Phy TX Reg Recfg Table size 0x%x\n"), pMac->hal.memMap.aduPhyTxRegRecfgTbl_size));
#endif /* VOLANS_PHY_TX_OPT_ENABLED */
    HALLOGW( halLog(pMac, LOGW, FL("PACKET MEMORY offset 0x%x\n"), pMac->hal.memMap.packetMemory_offset));

    return eHAL_STATUS_SUCCESS;
}


/**
 *    \fn     halInitWmParam
 *
 *   \brief  This routine initializes the hal, MTU and Smac with
 *            WMM Wireless Medium parameter from WMM Default Broadcast Profile.
 *
 *    \param  hHal :  Handle to pMac structure.
 *
 *    \param  arg  :  Not used.
 *
 *    \note   Initializing the WMM param with WMM defaults is mandatory for stations
 *            BSS before Association and in IBSS mode.
 */
eHalStatus halInitWmParam(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U8  params[MAX_NUM_AC][WNI_CFG_EDCA_ANI_ACBK_LEN];
    tANI_U16 aEdcaTxop[MTU_BKID_HI_NUM];
    tANI_U32 cwminidx, cwmaxidx, txopidx;
    tANI_U32 mode;
    tANI_U32 wme[MAX_NUM_AC] = {WNI_CFG_EDCA_WME_ACBE, WNI_CFG_EDCA_WME_ACBK,
                   WNI_CFG_EDCA_WME_ACVI, WNI_CFG_EDCA_WME_ACVO};
    tANI_U8 i;

    (void) arg;

    /** Initialize the EDCA parameter structure */
    palZeroMemory(pMac, pMac->hal.edcaParam, (sizeof(tSirMacEdcaParamRecord) * MAX_NUM_AC) );

    for (i=0; i < MAX_NUM_AC; i++)
    {
        tANI_U32 len = WNI_CFG_EDCA_ANI_ACBK_LEN;
        if (wlan_cfgGetStr(pMac, (tANI_U16) wme[i], params[i], &len) != eSIR_SUCCESS)
        {
            return eHAL_STATUS_FAILURE;
        }
        if (len > WNI_CFG_EDCA_ANI_ACBK_LEN)
        {
            return eHAL_STATUS_FAILURE;
        }
    }

    mode = halMTU_getMode(pMac);
    switch (mode)
    {
        case MODE_11G_PURE:
        case MODE_11G_MIXED:
            cwminidx = WNI_CFG_EDCA_PROFILE_CWMING_IDX;
            cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXG_IDX;
            txopidx  = WNI_CFG_EDCA_PROFILE_TXOPG_IDX;
            break;
        case MODE_11B:
            cwminidx = WNI_CFG_EDCA_PROFILE_CWMINB_IDX;
            cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXB_IDX;
            txopidx  = WNI_CFG_EDCA_PROFILE_TXOPB_IDX;
            break;
        default: /**< This can happen if mode is not set yet, assume 11a mode.*/
            cwminidx = WNI_CFG_EDCA_PROFILE_CWMINA_IDX;
            cwmaxidx = WNI_CFG_EDCA_PROFILE_CWMAXA_IDX;
            txopidx  = WNI_CFG_EDCA_PROFILE_TXOPA_IDX;
            break;
    }


    for(i=0; i<MAX_NUM_AC; i++)
    {
        pMac->hal.edcaParam[i].aci.acm = params[i][WNI_CFG_EDCA_PROFILE_ACM_IDX];
        pMac->hal.edcaParam[i].aci.aifsn = params[i][WNI_CFG_EDCA_PROFILE_AIFSN_IDX];
        pMac->hal.edcaParam[i].cw.min =  convertCW(GET_CW(&params[i][cwminidx]));
        pMac->hal.edcaParam[i].cw.max =  convertCW(GET_CW(&params[i][cwmaxidx]));
        pMac->hal.edcaParam[i].txoplimit=  (tANI_U16) params[i][txopidx];
    }

    palZeroMemory(pMac->hHdd, aEdcaTxop, sizeof(aEdcaTxop));
    aEdcaTxop[MTU_BKID_AC_BE] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_BESTEFFORT].txoplimit);
    aEdcaTxop[MTU_BKID_AC_BK] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_BACKGROUND].txoplimit);
    aEdcaTxop[MTU_BKID_AC_VI] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_VIDEO].txoplimit);
    aEdcaTxop[MTU_BKID_AC_VO] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_VOICE].txoplimit);
                
    // update TXOP
    if (halTpe_UpdateEdcaTxOp(pMac, aEdcaTxop) !=eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halTpe_UpdateEdcaTxOp() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // update Short retry and Long Retry limit
    if (halMsg_updateRetryLimit(pMac) !=eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halMsg_updateRetryLimit() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    for (i=0;  i < MAX_NUM_AC; i++)
    {
        halMTU_updateCW(pMac, i, pMac->hal.edcaParam[i].cw.min, pMac->hal.edcaParam[i].cw.max);
    }

    halMTU_updateAIFS(pMac, pMac->hal.edcaParam);

    return eHAL_STATUS_SUCCESS;
}


/**
 * \fn halGetChipRevNum
 *
 * \brief   returns HAL stored value for chip rev num.
 *
 *  \param     tpAniSirGlobal    pMac
 *
 *  \return    revision number
 */
tANI_U32 halGetChipRevNum(tpAniSirGlobal pMac)
{
    return pMac->hal.chipRevNum;
}

/**
 * \fn halGetCardType
 *
 * \brief   returns HAL stored value for the type of Card type
 *
 *  \param     tpAniSirGlobal    pMac
 *
 *  \return    cardType
 */
tANI_U8 halGetCardType(tpAniSirGlobal pMac)
{
    return pMac->hal.cardType;
}

/**
 *  \brief  This API is used to get the current state of the link.
 *            this returns eANI_BOOLEAN_TRUE if busy or eANI_BOOLEAN_FALSE if Idle.
 *
 *  \param pMac - Mac Global Handle
 *
 *  \note  The sampeling interval for the link is 100 ms as its using the LED
 *            activity timer.
 *
 */
tANI_BOOLEAN halIsLinkBusy(tpAniSirGlobal pMac)
{
    tANI_U32 pktDiff;

    pktDiff = pMac->hal.halMac.macStats.txCount - pMac->hal.halMac.macStats.prevTxCount;
    pktDiff += (pMac->hal.halMac.macStats.rxCount - pMac->hal.halMac.macStats.prevRxCount);

    if (pktDiff > 0)
       return eANI_BOOLEAN_TRUE;

    return eANI_BOOLEAN_FALSE;
}

/*
 * Function to get the frame translation enabled/disabled state
 */
tANI_U8 halGetFrameTranslation(tpAniSirGlobal pMac)
{
    return (pMac->hal.halMac.frameTransEnabled);
}

