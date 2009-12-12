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

/// convert the CW values into a tANI_U16
#define GET_CW(pCw) ((tANI_U16) ((*(pCw) << 8) + *((pCw) + 1)))


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

    // Start up in eSYSTEM_STA_ROLE
    // This should get updated appropriately at a later stage
    halSetSystemRole( pMac, eSYSTEM_STA_ROLE );
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
    eHalStatus retVal;

    switch (powerState) {
    case eSIR_HT_MIMO_PS_DYNAMIC:
        retVal =  halPhySetPowerSave(pMac, PHY_POWER_DYNAMIC_RX_SM) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Dynamic SM PS Mode \n")));
        break;

    case eSIR_HT_MIMO_PS_STATIC:
        retVal =  halPhySetPowerSave(pMac, PHY_POWER_STATIC_RX_SM) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Static SM PS Mode \n")));
        break;

    default:
        retVal =  halPhySetPowerSave(pMac, PHY_POWER_NORMAL) ;
        HALLOG1( halLog(pMac, LOG1, FL("Updated the Rxp Chains for Normal SM PS Mode \n")));
    break;
    }

    return retVal;
}


tSystemRole halGetSystemRole(tpAniSirGlobal pMac)
{
    return pMac->hal.halSystemRole;
}

void halSetSystemRole(tpAniSirGlobal pMac, tSystemRole role)
{
   pMac->hal.halSystemRole = role;
}

/**
 * \fn   halSetByteSwap
 *
 * \brief   Disable byte swapping (Set PCI BAR2 swap bit to 1)
 *            Enable byte swapping (Clear PCI Bar2 swap bit to 0)
 * 
 * \param  tpAniSirGlobal  pMac
 *
 * \param  eAniBoolean     swap01
 *
 * \param  eAniBoolean     swap2
 *
 * \param eAniBoolean     swapDma
 *
 * \return  eHalStatus
 */
eHalStatus halSetByteSwap(
    tpAniSirGlobal  pMac,
    eAniBoolean     swap01,
    eAniBoolean     swap2,
    eAniBoolean     swapDma )
{
    eHalStatus eRc = eHAL_STATUS_SUCCESS;
#if defined(ANI_BUS_TYPE_PCI)
#error "Libra doesn't support PCI interface"
#if 0 //FIXME_NO_VIRGO
    tANI_U32    value;
    // setup the T0/1 register properly first
    if (swap01)
        halWriteRegister(pMac, PIF_PCI_PIF_PCI_BYTE_SWAP_REG, 1);

    pMac->hal.intSwapPci = swap01;

    if ((eRc = halReadRegister(pMac, PIF_PCI_PIF_PCI_CONTROL_REG, &value)) != eHAL_STATUS_SUCCESS)
        return eRc;

    if (swap2)
        value |= PIF_PCI_PIF_PCI_CONTROL_TARGET2_CHANGE_ENDIAN_MASK;
    else
        value &= (~PIF_PCI_PIF_PCI_CONTROL_TARGET2_CHANGE_ENDIAN_MASK);

    if (swapDma)
        value |= PIF_PCI_PIF_PCI_CONTROL_OUTBOUND_CHANGE_ENDIAN_MASK;
    else
        value &= (~PIF_PCI_PIF_PCI_CONTROL_OUTBOUND_CHANGE_ENDIAN_MASK);

    eRc = halWriteRegister(pMac, PIF_PCI_PIF_PCI_CONTROL_REG, value);
#endif
#endif
    return eRc;
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
    return BMU_INTERNAL_MEMORY_SIZE_128K;
#else
    return BMU_INTERNAL_MEMORY_SIZE_256K;
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


/*
 * Compute the start address of the HW descriptors based on the FW image size
 * Initialize the start address of all the descriptors in the device memory
 */
eHalStatus halMemoryMap_Start(tHalHandle hHal, void *arg)
{
    tHalMacStartParameters *pStartParams = (tHalMacStartParameters*)arg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U32 fwSize = 0;
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

    // Compute the memory address based on the FW image size
    pMac->hal.memMap.descStartAddr = (((FW_IMAGE_MEMORY_BASE_ADDRESS+fwSize)+(HAL_MEM_BOUNDARY_ALIGN-1))/HAL_MEM_BOUNDARY_ALIGN) * HAL_MEM_BOUNDARY_ALIGN;

    HALLOGE(halLog(pMac, LOGE, FL("FW image size = %d (0x%x) bytes"), fwSize, fwSize));
    HALLOGE(halLog(pMac, LOGE, FL("HW Descriptor Start Addr = 0x%08x"), pMac->hal.memMap.descStartAddr));

    // Start the first HW descriptor (DPU) after the FW image
    pMac->hal.memMap.dpuDescriptor_offset = pMac->hal.memMap.descStartAddr;
    pMac->hal.memMap.dpuDescriptor_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuDescriptor);

    pMac->hal.memMap.keyDescriptor_offset =
        (pMac->hal.memMap.dpuDescriptor_offset + pMac->hal.memMap.dpuDescriptor_size);
    pMac->hal.memMap.keyDescriptor_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuKeyDescriptor);

    pMac->hal.memMap.micKey_offset =
        (pMac->hal.memMap.keyDescriptor_offset + pMac->hal.memMap.keyDescriptor_size);
    pMac->hal.memMap.micKey_size =
        pMac->hal.memMap.maxDpuEntries * sizeof(tDpuMicKeyDescriptor);

    pMac->hal.memMap.replayCounter_offset =
        (pMac->hal.memMap.micKey_offset + pMac->hal.memMap.micKey_size);
    pMac->hal.memMap.replayCounter_size =
        pMac->hal.memMap.maxDpuEntries * MAX_NUM_OF_TIDS * sizeof(tDpuReplayCounterDescriptor);

    pMac->hal.memMap.dxeRxDescInfo_offset =
        (pMac->hal.memMap.replayCounter_offset + pMac->hal.memMap.replayCounter_size);
    pMac->hal.memMap.dxeRxDescInfo_size = DXE_RXDESC_INFO_SIZE;

    pMac->hal.memMap.tpeStaDesc_offset =
       pMac->hal.memMap.dxeRxDescInfo_offset+pMac->hal.memMap.dxeRxDescInfo_size;
    pMac->hal.memMap.tpeStaDesc_size =
       pMac->hal.memMap.maxStations * TPE_STA_DESC_AND_STATS_SIZE;

    pMac->hal.memMap.rpeStaDesc_offset =
       pMac->hal.memMap.tpeStaDesc_offset+pMac->hal.memMap.tpeStaDesc_size;
    pMac->hal.memMap.rpeStaDesc_size =
       pMac->hal.memMap.maxStations * RPE_STA_DESC_ENTRY_SIZE;

    pMac->hal.memMap.rpePartialBitmap_offset =
       pMac->hal.memMap.rpeStaDesc_offset+pMac->hal.memMap.rpeStaDesc_size;
    pMac->hal.memMap.rpePartialBitmap_size = RPE_PARTIAL_BITMAP_SIZE;

    pMac->hal.memMap.btqmTxQueue_offset =
       pMac->hal.memMap.rpePartialBitmap_offset+pMac->hal.memMap.rpePartialBitmap_size;
    pMac->hal.memMap.btqmTxQueue_size =
       pMac->hal.memMap.maxStations * HW_MAX_QUEUES * BTQM_STA_QUEUE_ENTRY_SIZE;

    pMac->hal.memMap.hwTemplate_offset =
       pMac->hal.memMap.btqmTxQueue_offset+pMac->hal.memMap.btqmTxQueue_size;
    pMac->hal.memMap.hwTemplate_size = HW_TEMPLATE_SIZE;

    pMac->hal.memMap.swTemplate_offset =
       pMac->hal.memMap.hwTemplate_offset+pMac->hal.memMap.hwTemplate_size;
    pMac->hal.memMap.swTemplate_size = SW_TEMPLATE_SIZE;

    pMac->hal.memMap.beaconTemplate_offset =
       pMac->hal.memMap.swTemplate_offset+pMac->hal.memMap.swTemplate_size;
    pMac->hal.memMap.beaconTemplate_size = BEACON_TEMPLATE_SIZE;

    pMac->hal.memMap.aduUmaStaDesc_offset =
        pMac->hal.memMap.beaconTemplate_offset+
        (BEACON_TEMPLATE_SIZE * pMac->hal.memMap.maxBssids);
    pMac->hal.memMap.aduUmaStaDesc_size = HAL_NUM_UMA_DESC_ENTRIES *
                                            ADU_UMA_STA_DESC_ENTRY_SIZE;
    
    pMac->hal.memMap.aduRegRecfgTbl_offset =
       pMac->hal.memMap.aduUmaStaDesc_offset+
       pMac->hal.memMap.aduUmaStaDesc_size; 
    pMac->hal.memMap.aduRegRecfgTbl_size = ADU_REG_RECONFIG_TABLE_SIZE;
    pMac->hal.memMap.aduRegRecfgTbl_curPtr = pMac->hal.memMap.aduRegRecfgTbl_offset;

    pMac->hal.memMap.aduMimoPSprg_offset =
       pMac->hal.memMap.aduRegRecfgTbl_offset+pMac->hal.memMap.aduRegRecfgTbl_size;
    pMac->hal.memMap.aduMimoPSprg_size = ADU_MIMO_PS_PRG_SIZE;

    pMac->hal.memMap.packetMemory_offset =
       ((pMac->hal.memMap.aduMimoPSprg_offset + pMac->hal.memMap.aduMimoPSprg_size)/HAL_BD_SIZE + 1) * HAL_BD_SIZE;

        HALLOGW( halLog(pMac, LOGW, FL("DPU Desc offset 0x%x\n"), pMac->hal.memMap.dpuDescriptor_offset));
    HALLOGW( halLog(pMac, LOGW, FL("KEY Desc offset 0x%x\n"), pMac->hal.memMap.keyDescriptor_offset));
    HALLOGW( halLog(pMac, LOGW, FL("MIC KEY offset 0x%x\n"), pMac->hal.memMap.micKey_offset));
    HALLOGW( halLog(pMac, LOGW, FL("Replay Counters offset 0x%x\n"), pMac->hal.memMap.replayCounter_offset));
    HALLOGW( halLog(pMac, LOGW, FL("USB Info offset 0x%x\n"), pMac->hal.memMap.dxeRxDescInfo_offset));
    HALLOGW( halLog(pMac, LOGW, FL("USB Info size 0x%x\n"), pMac->hal.memMap.dxeRxDescInfo_size));
    HALLOGW( halLog(pMac, LOGW, FL("TPE STA Desc offset 0x%x\n"), pMac->hal.memMap.tpeStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("TPE STA Desc size 0x%x\n"), pMac->hal.memMap.tpeStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("RPE STA Desc Offset 0x%x\n"), pMac->hal.memMap.rpeStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("RPE STA Desc Size 0x%x\n"), pMac->hal.memMap.rpeStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("RPE Partial Bitmap Offset 0x%x\n"), pMac->hal.memMap.rpePartialBitmap_offset));
    HALLOGW( halLog(pMac, LOGW, FL("RPE Partial Bitmap Size 0x%x\n"), pMac->hal.memMap.rpePartialBitmap_size));
    HALLOGW( halLog(pMac, LOGW, FL("BTQM Tx WQ offset 0x%x\n"), pMac->hal.memMap.btqmTxQueue_offset));
    HALLOGW( halLog(pMac, LOGW, FL("BTQM Tx WQ size 0x%x\n"), pMac->hal.memMap.btqmTxQueue_size));
    HALLOGW( halLog(pMac, LOGW, FL("HW Template offset 0x%x\n"), pMac->hal.memMap.hwTemplate_offset));
    HALLOGW( halLog(pMac, LOGW, FL("HW Template size 0x%x\n"), pMac->hal.memMap.hwTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("SW Template size 0x%x\n"), pMac->hal.memMap.swTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("Beacon Template offset 0x%x\n"), pMac->hal.memMap.beaconTemplate_offset));
    HALLOGW( halLog(pMac, LOGW, FL("Beacon Template size 0x%x\n"), pMac->hal.memMap.beaconTemplate_size));
    HALLOGW( halLog(pMac, LOGW, FL("ADU UMA STA Desc offset 0x%x\n"), pMac->hal.memMap.aduUmaStaDesc_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU UMA STA Desc Size 0x%x\n"), pMac->hal.memMap.aduUmaStaDesc_size));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Reg Recfg Table offset 0x%x\n"), pMac->hal.memMap.aduRegRecfgTbl_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Reg Recfg Table size 0x%x\n"), pMac->hal.memMap.aduRegRecfgTbl_size));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Mimo PS Programming Offset 0x%x\n"), pMac->hal.memMap.aduMimoPSprg_offset));
    HALLOGW( halLog(pMac, LOGW, FL("ADU Mimo PS Programming size 0x%x\n"), pMac->hal.memMap.aduMimoPSprg_size));
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
    if (halTpe_UpdateEdcaTxOp(pMac, aEdcaTxop) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halTpe_UpdateEdcaTxOp() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // update Short retry and Long Retry limit
    if (halMsg_updateRetryLimit(pMac) != eSIR_SUCCESS)
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

