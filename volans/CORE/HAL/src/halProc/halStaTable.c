/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halStaTable.c:  implements STA Table inside HAL and APIs
 * Author:    Satish Gune
 * Date:      03/07/2006
 *
 * --------------------------------------------------------------------------
 */
#include "halInternal.h"
#include "halTypes.h"
#include "halMsgApi.h"
#include "halDebug.h"
#include "halMacSecurityApi.h"
#include "halMacBA.h"
#include "halUtils.h"

// Local defines

//////////////////////////////////////////////////////////////////////////////
// halInitTables - Initialize the BSS and STA tables. Allocate the memory
//
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_Open(tHalHandle hHal, void *arg)
{
    eHalStatus status;
    tANI_U8 max_sta, max_bssid, staIdx;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;

    max_sta     = (tANI_U8) pMac->hal.memMap.maxStations;
    max_bssid   = (tANI_U8) pMac->hal.memMap.maxBssids;

    // Allocate the memory for sta and BSS table
    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.bssTable,
            max_bssid * sizeof(tBssStruct));
    if (status != eHAL_STATUS_SUCCESS)
        goto out;
    else
        palZeroMemory( pMac->hHdd,
                (void *) pMac->hal.halMac.bssTable,
                max_bssid * sizeof( tBssStruct ));

    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.staTable,
            max_sta * sizeof(tStaStruct));
    if (status != eHAL_STATUS_SUCCESS)
        goto out;
    else
        palZeroMemory( pMac->hHdd,
                (void *) pMac->hal.halMac.staTable,
                max_sta * sizeof( tStaStruct ));

    //Set the BssIdx to Invalid one
    for(staIdx = 0;staIdx < HAL_NUM_STA;staIdx++)
    {
        tpStaStruct pSta = &(((tpStaStruct)(pMac->hal.halMac.staTable))[staIdx]);
        pSta->bssIdx = HAL_INVALID_BSSIDX;
    }
	

    // zero out the uma descriptor table
    palZeroMemory( pMac->hHdd,
                   (void *) pMac->hal.halMac.aduUmaDesc,
                   sizeof( pMac->hal.halMac.aduUmaDesc ));

    // Store number of bssids and stas
    pMac->hal.halMac.maxBssId = max_bssid;
    pMac->hal.halMac.maxSta = max_sta;
    pMac->hal.halMac.numOfValidSta= 0;

#ifndef HAL_SELF_STA_PER_BSS
    // Initialize the Self STAID to an invalid value
    pMac->hal.halMac.selfStaId = HAL_STA_INVALID_IDX;
#endif

    return eHAL_STATUS_SUCCESS;
out:
    halTable_Close(hHal, NULL);
    HALLOGE(halLog(pMac, LOGE, FL("HAL table Open failed\n")));

    return status;
}

eHalStatus halTable_Start(tHalHandle hHal, void *arg)
{
    tANI_U8 max_sta, max_bssid;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;

    max_sta     = (tANI_U8) pMac->hal.memMap.maxStations;
    max_bssid   = (tANI_U8) pMac->hal.memMap.maxBssids;

    // Store number of bssids and stas
    pMac->hal.halMac.maxBssId = max_bssid;
    pMac->hal.halMac.maxSta = max_sta;
    pMac->hal.halMac.numOfValidSta= 0;

#ifdef WLAN_SOFTAP_FEATURE
    /** Zero out the BSS Table */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.bssTable_offset,
                        pMac->hal.memMap.bssTable_size);

    /** Zero out the Sta Table */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.staTable_offset,
                        pMac->hal.memMap.staTable_size);
#endif

    return eHAL_STATUS_SUCCESS;
}


eHalStatus halTable_Stop(tHalHandle hHal, void *arg)
{
    tANI_U8 max_sta, max_bssid;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;

#ifndef HAL_SELF_STA_PER_BSS
    // Clean up the Self STAID
    pMac->hal.halMac.selfStaId = HAL_STA_INVALID_IDX;
#endif

    max_sta     = (tANI_U8) pMac->hal.memMap.maxStations;
    max_bssid   = (tANI_U8) pMac->hal.memMap.maxBssids;

    palZeroMemory( pMac->hHdd,
            (void *) pMac->hal.halMac.bssTable,
            max_bssid * sizeof( tBssStruct ));

    palZeroMemory( pMac->hHdd,
            (void *) pMac->hal.halMac.staTable,
            max_sta * sizeof( tStaStruct ));

    return eHAL_STATUS_SUCCESS;
}


eHalStatus halTable_Close(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    // Free memory
    if (pMac->hal.halMac.bssTable != NULL)
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.bssTable);
    if (pMac->hal.halMac.staTable != NULL)
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.staTable);
    pMac->hal.halMac.bssTable = NULL;
    pMac->hal.halMac.staTable = NULL;
    return status;
}

//////////////////////////////////////////////////////////////////////////////
// halTableDbg_dumpStaTable
// Display the station table
//
//////////////////////////////////////////////////////////////////////////////
void halTableDbg_dumpStaTable(tpAniSirGlobal pMac)
{
    int i;
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

    HALLOGW( halLog(pMac, LOGW, FL("Idx MAC-Address       Dpu Bss\n")));
    HALLOGW( halLog(pMac, LOGW, FL("--- ----------------- --- ---\n")));
    for (i=0; i < pMac->hal.halMac.maxSta; i++)
    {
        if (pSta[i].valid)
        {
            HALLOGW( halLog(pMac, LOGW, FL("%-3d %02x:%02x:%02x:%02x:%02x:%02x "
                    "%-3d %d (%02x:%02x:%02x:%02x:%02x:%02x)\n"),
                    i,
                    pSta[i].staAddr[0], pSta[i].staAddr[1], pSta[i].staAddr[2],
                    pSta[i].staAddr[3], pSta[i].staAddr[4], pSta[i].staAddr[5],
                    pSta[i].dpuIndex,
                    pSta[i].bssIdx,
                    pSta[i].bssId[0], pSta[i].bssId[1], pSta[i].bssId[2],
                    pSta[i].bssId[3], pSta[i].bssId[4], pSta[i].bssId[5]));
        }
    }
}

void halTableDbg_dumpBssTable(tpAniSirGlobal pMac)
{
    int i;
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;

    HALLOGW( halLog( pMac, LOGW, FL("Maximum BSS Table size - %d\n"), pMac->hal.halMac.maxBssId ));
    for( i=0; i < pMac->hal.halMac.maxBssId; i++ )
    {
        if( bssTable[i].valid )
        {
            HALLOGW( halLog( pMac, LOGW, FL("-------------------------\n" )));
            HALLOGW( halLog( pMac, LOGW, FL("%-4s (Idx%d - %02x:%02x:%02x:%02x:%02x:%02x) : BcnInt %d, DTIM %d\n"),
                bssTable[i].bssType? "IBSS":"BSS",
                    i,
                    bssTable[i].bssId[0], bssTable[i].bssId[1], bssTable[i].bssId[2],
                    bssTable[i].bssId[3], bssTable[i].bssId[4], bssTable[i].bssId[5],
                bssTable[i].tuBeaconInterval,
                bssTable[i].bcnDtimPeriod ));

            HALLOGW( halLog( pMac, LOGW, FL("\tSTAidx: %d, STA bmap: %x %x %x %x %x %x %x %x\n"),
                    bssTable[i].bssSelfStaIdx, bssTable[i].staIdBitmap[0], bssTable[i].staIdBitmap[1], bssTable[i].staIdBitmap[2],
                    bssTable[i].staIdBitmap[3],bssTable[i].staIdBitmap[4],bssTable[i].staIdBitmap[5],bssTable[i].staIdBitmap[6],
                    bssTable[i].staIdBitmap[7]));

            HALLOGW( halLog( pMac, LOGW, FL("\tShortPreamble: %d, 11gCoexist:%d  11bCoexist:%d  \n"), \
                    bssTable[i].bssRaInfo.u.bit.fShortPreamble, bssTable[i].bssRaInfo.u.bit.llgCoexist, bssTable[i].bssRaInfo.u.bit.llbCoexist));

            HALLOGW( halLog( pMac, LOGW, FL("\tNonGfPresent: %d RIFS:%d HT20Coexist:%d \n"), \
                    bssTable[i].bssRaInfo.u.bit.nonGfPresent, bssTable[i].bssRaInfo.u.bit.rifsMode, bssTable[i].bssRaInfo.u.bit.ht20Coexist));

            HALLOGW( halLog( pMac, LOGW, FL("\tEncType: %d [O/W40/W104/T/A]\n"),
                    bssTable[i].encryptMode ));
        }
    }
}

/* ------------------------------------------------------------
 * FUNCTION:  halTable_GetStaId
 *
 * NOTE:
 *   Allocate a new station index based on the given bssid
 *   and station mac address.
 *
 *   If we're operating in loopback mode, then don't check
 *   for duplicate station addr or bssid. Else if we're
 *   operating in non-loopback mode, then perform the
 *   following check:
 *
 *   If (adding self)
 *     - check if the station self entry already exist in the
 *       station table. If so, then return the existing staid.
 *   If (adding peer or other)
 *     - check if the bssid or the station address already
 *       exist in the station table. If so, then return error.
 * ------------------------------------------------------------
 */
eHalStatus halTable_GetStaId(tpAniSirGlobal pMac, tANI_U8 type, tSirMacAddr bssId, tSirMacAddr staAddr, tANI_U8 *id)
{
    tANI_U8       i;
    tpStaStruct   t = (tpStaStruct) pMac->hal.halMac.staTable;
    tpStaStruct   sta = 0;
    tANI_U8       found = HAL_STA_INVALID_IDX;
    eHalStatus    status;
    tANI_U8 minIndex, maxIndex;

    HALLOG1( halLog(pMac, LOG1, FL("bssId=%x:%x:%x:%x:%x:%x staMac=%x:%x:%x:%x:%x:%x\n"), 
                bssId[0], bssId[1], bssId[2], bssId[3], bssId[4], bssId[5],
                staAddr[0], staAddr[1], staAddr[2], staAddr[3], staAddr[4], staAddr[5] ));

#ifdef HAL_BCAST_STA_PER_BSS
    if (type == STA_ENTRY_BCAST) {
        if (HAL_MAX_NUM_BCAST_STATIONS > 0) {
            minIndex = HAL_MIN_BCAST_STA_INDEX;
            maxIndex = HAL_MAX_NUM_BCAST_STATIONS;
        } else {
            return eHAL_STATUS_STA_TABLE_FULL;
        }
    } else {
        minIndex = HAL_MIN_STA_INDEX;
        maxIndex = (HAL_NUM_STA);
    }
    t += minIndex;
#else
    minIndex = 0;
    maxIndex =  pMac->hal.halMac.maxSta;
#endif

    HALLOG4( halLog(pMac, LOG4, FL("MinIndex = %d, MaxIndex = %d\n"), minIndex, maxIndex));

    for (i=minIndex; (found == HAL_STA_INVALID_IDX) && (i < maxIndex); i++, t++)
    {
        if (! pMac->hal.loopback)
        {
            // For local loopback following check fails
            // because same sta is added twice; once as self by STA and later by AP as remote STA
            if (type == STA_ENTRY_SELF)
            {
                if ( (t->valid == 1) && (sirCompareMacAddr(t->staAddr, staAddr)) )
                {
                    HALLOGE( halLog(pMac, LOGE, FL("This SELF STA mac addr already exist in entry %d. \n"),i));
                    *id = i;
                    sta = t;
                    found = i;                    
                    return eHAL_STATUS_SUCCESS;
                }
            }
            else
            {
//#ifdef HAL_TABLE_API_DEBUG
                HALLOG4( halLog(pMac, LOG4, FL("Adding STA_ENTRY_PEER/OTHER, checking table entry[%d] \n"),i));
//#endif
                if ((t->valid == 1) &&
                        (sirCompareMacAddr(t->staAddr, staAddr) &&
                         (sirCompareMacAddr(t->bssId, bssId))))
                {
//#ifdef HAL_TABLE_API_DEBUG
                    HALLOG1( halLog(pMac, LOG1, FL("This PEER mac addr already exist in entry %d. It's duplicate. \n"), i));
//#endif
                    *id = i;
                    return eHAL_STATUS_DUPLICATE_STA;
                }
            }
        }
        else
        {
            if (t->valid == 1)
                continue;
        }

#ifdef WLAN_SOFTAP_VSTA_FEATURE
        if ((pMac->hal.useOnlyVstaIdx) && 
            (type == STA_ENTRY_PEER) && !IS_VSTA_IDX(i))
            continue;

        // make sure we never assign the General Purpose STAs used to
        // support the Virtual Station functionality
        if (IS_GPSTA_IDX(i))
            continue;
#endif

        /* Assign this free staid entry */
        if ( (t->valid == 0) && (!sta) )
        {
            sta = t;
            found = i;
            break;
        }
    }
//#ifdef HAL_TABLE_API_DEBUG
    HALLOG1( halLog(pMac, LOG1, FL("Got station index %d \n"), found));
//#endif

    if ((i == maxIndex) && !sta)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Station table full")));
        return eHAL_STATUS_STA_TABLE_FULL;
    }
	if (sta == NULL)
	{
    	HALLOGE( halLog(pMac, LOGE, FL("Station table full")));
		return eHAL_STATUS_STA_TABLE_FULL;
    }
    sta->valid = 1;
    palCopyMemory(pMac->hHdd, (void *) sta->bssId, (void *)bssId, 6);
    palCopyMemory(pMac->hHdd, (void *) sta->staAddr, (void *)staAddr, 6);
    *id = found;
    sta->staId = *id;

#ifdef HAL_SELF_STA_PER_BSS
    sta->bssSelfStaIdx = *id;
#endif

    /* let's fill in the bss index for use later */
    status = halTable_FindBssidByAddr(pMac, bssId, &sta->bssIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("GetBssIndex failed (error %d)!"), status));
    }

    // Invalidate all the dpu indices for this new sta entry.
    sta->dpuIndex = HAL_INVALID_KEYID_INDEX;
    sta->bcastDpuIndex = HAL_INVALID_KEYID_INDEX;
    sta->bcastMgmtDpuIndex = HAL_INVALID_KEYID_INDEX;
    sta->umaIdx = HAL_INVALID_KEYID_INDEX;
    sta->opRateMode = eSTA_INVALID_RATE_MODE;
    
    // Initialize BA Session ID entries in this STA Entry
    // to be invalid
    palFillMemory( pMac->hHdd,
            (void *) sta->baSessionID,
            sizeof( *sta->baSessionID ) * STACFG_MAX_TC,
            (tANI_BYTE) BA_SESSION_ID_INVALID );

    palZeroMemory( pMac->hHdd,
        (void *) sta->baBlocked,
        sizeof( *sta->baBlocked ) * STACFG_MAX_TC );

    return eHAL_STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_ClearSta - Allocate a new station index.
// Error if the station already exists in station table.
//
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_ClearSta(tpAniSirGlobal pMac, tANI_U8 id)
{
    tpStaStruct t = pMac->hal.halMac.staTable;

    if ((id < pMac->hal.halMac.maxSta) )
    {
        tANI_U32 staSig = t[id].staSig;
        // Now if this is a PEER STA entry, delete the context for this STA
        // from the corresponding BSS table
        if(STA_ENTRY_PEER == t[id].staType)
        {
            halTable_BssDelSta(pMac, t[id].bssIdx, id);
        }
#ifndef HAL_SELF_STA_PER_BSS
        else if(STA_ENTRY_SELF == t[id].staType)
        {
            // Check if this was same as the stored Self-STAID then reset the stored index
            if(id == (tANI_U8)pMac->hal.halMac.selfStaId)
                pMac->hal.halMac.selfStaId = HAL_STA_INVALID_IDX;
        }
#endif
        
        //clear the statistics maintained for this sta
        halMacClearStaStats(pMac, id);
        
        palZeroMemory(pMac->hHdd, (void *) &t[id], sizeof(tStaStruct));
        t[id].staSig = (tANI_U8)(staSig + 1) & (HAL_STA_SIG_AUTH_MASK-1);

        //Invalidate the BSS Index
        t[id].bssIdx = HAL_INVALID_BSSIDX;

        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_SearchStaidByAddr - search the sta by sta addr and bssid.
//
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_FindStaidByAddrAndBssid(tpAniSirGlobal pMac, tSirMacAddr bssId, tSirMacAddr staAddr, tANI_U8 *id)
{
    tANI_U8 i;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    eHalStatus status = eHAL_STATUS_STA_INVALID;

    for (i=0; i < pMac->hal.halMac.maxSta; i++)
    {
        if ((t->valid == 1) && (sirCompareMacAddr(t->staAddr, staAddr) && (sirCompareMacAddr(t->bssId, bssId))))
        {
            *id = i;
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_GetBssIndex - Allocate BSSID.
// Error if bssid already exists or bssid table is full.
//
// NOTE - This API will return a "zero-based" BSS Index
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_GetBssIndex(tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U8 *id)
{
    tANI_U8     i;
    tpBssStruct t       = (tpBssStruct) pMac->hal.halMac.bssTable;
    tpBssStruct bss     = NULL;
    tANI_U8     index   = 0;

    for (i=0; i < pMac->hal.halMac.maxBssId; i++, t++)
    {
        if ( ! pMac->hal.loopback)
        {
            // Following check will fail if BSS is added twice, once by AP (start BSS) and
            // later by STA (join BSS)
            if ((t->valid == 1) && (sirCompareMacAddr(t->bssId, bssId)))
            {
                // Duplicate
                *id = i;
                return eHAL_STATUS_DUPLICATE_BSSID;
            }
        }
        if (t->valid == 1)
            continue;

        /* Assign this free bssid entry */
        if (t->valid == 0)
        {
            bss = t;
            index = i;
            break;
        }
    }

    if ((i == pMac->hal.halMac.maxBssId) && (bss == 0))
        return eHAL_STATUS_BSSID_TABLE_FULL;
    if (bss == NULL)
    {
        HALLOGE(halLog(pMac, LOGE, FL("BSSID TABLE is full. \n")));   	
		return eHAL_STATUS_BSSID_TABLE_FULL;
	}
	bss->valid = 1;
    *id = index;
    palCopyMemory(pMac->hHdd, (void *) bss->bssId, (void *)bssId, 6);

    // Zero out the STAID bitmap for this BSS
    palFillMemory(pMac->hHdd, (void *) bss->staIdBitmap, sizeof(bss->staIdBitmap), 0);

    /* initialize for static WEP */
    bss->encryptMode = eSIR_ED_NONE;
    bss->wepKeyIds[0] = HAL_INVALID_KEYID_INDEX;
    bss->wepKeyIds[1] = HAL_INVALID_KEYID_INDEX;
    bss->wepKeyIds[2] = HAL_INVALID_KEYID_INDEX;
    bss->wepKeyIds[3] = HAL_INVALID_KEYID_INDEX;
    bss->dpuIdx = HAL_INVALID_KEYID_INDEX;
    bss->dpuIdxBcastMgmt = HAL_INVALID_KEYID_INDEX;

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halTable_BssAddSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U8 dwIndex, bitIndex;

    if(bssIdx >= pMac->hal.halMac.maxBssId)
        return eHAL_STATUS_INVALID_BSSIDX;

    if(staIdx >= pMac->hal.halMac.maxSta)
        return eHAL_STATUS_INVALID_STAIDX;

    dwIndex = staIdx / 32;
    bitIndex = staIdx % 32;
    t[bssIdx].staIdBitmap[dwIndex] |= 1 << bitIndex;

    return eHAL_STATUS_SUCCESS;
}

eHalStatus halTable_BssDelSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U8 dwIndex, bitIndex;

    if(bssIdx >= pMac->hal.halMac.maxBssId)
        return eHAL_STATUS_INVALID_BSSIDX;

    if(staIdx >= pMac->hal.halMac.maxSta)
        return eHAL_STATUS_INVALID_STAIDX;

    dwIndex = staIdx / 32;
    bitIndex = staIdx % 32;
    t[bssIdx].staIdBitmap[dwIndex] &= ~(1 << bitIndex);

    return eHAL_STATUS_SUCCESS;
}

/*
 * Set the DPU descriptor index for the IGTK used for 
 * multicast and broadcast Mgmt frames
 */
eHalStatus halTable_SetBssBcastMgmtDpuIdx(tpAniSirGlobal pMac, 
        tANI_U8 bssIdx, tANI_U8 mgmtDpuIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if(bssIdx < pMac->hal.halMac.maxBssId)
    {
        t[bssIdx].dpuIdxBcastMgmt = mgmtDpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}

/*
 * Get the DPU descriptor index for the IGTK used for 
 * multicast and broadcast Mgmt frames
 */
eHalStatus halTable_GetBssBcastMgmtDpuIdx(tpAniSirGlobal pMac, 
        tANI_U8 bssIdx, tANI_U8 *mgmtDpuIdx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if( (bssIdx < pMac->hal.halMac.maxBssId) && (t[bssIdx].valid))
    {
        *mgmtDpuIdx = t[bssIdx].dpuIdxBcastMgmt;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_SetBssDpuIdx - Set the DPU Descriptor index correspoding to a
// given BSS index
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_SetBssDpuIdx( tpAniSirGlobal pMac,
    tANI_U8 bssIdx,
    tANI_U8 dpuIdx )
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if(bssIdx < pMac->hal.halMac.maxBssId)
    {
        t[bssIdx].dpuIdx = dpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_GetBssDpuIdx - Get the DPU Descriptor index correspoding to a
// given BSS index
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_GetBssDpuIdx( tpAniSirGlobal pMac,
    tANI_U8 bssIdx,
    tANI_U8 *pDpuIdx )
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if((bssIdx < pMac->hal.halMac.maxBssId) && (t[bssIdx].valid))
    {
        *pDpuIdx = (tANI_U8) t[bssIdx].dpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}

//////////////////////////////////////////////////////////////////////////////
// halTable_ClearBss - Free BSSID entry bssid
//
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_ClearBss(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    tANI_U8 i;
    tANI_U8 DpukeyIdx;

    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;

    /*Freeing up the Dpukeytable*/
    for (i=0; i<4; i++)
    {
        DpukeyIdx = t[bssIdx].wepKeyIds[i];
        if ( DpukeyIdx == HAL_INVALID_KEYID_INDEX )
            continue;
        else
        {
            status = halDpu_ReleaseKeyId (pMac, DpukeyIdx );
            if( eHAL_STATUS_SUCCESS != status)
                HALLOGW( halLog( pMac, LOGW, FL("Unable to release the DPU descriptor for BSS %d!\n"),  bssIdx ));
        }
    }

    if(!t->bssRaInfo.u.bit.rifsMode){
        if(pMac->hal.halMac.nonRifsBssCount)
            pMac->hal.halMac.nonRifsBssCount --;
    }else{
        if(pMac->hal.halMac.rifsBssCount)
            pMac->hal.halMac.rifsBssCount --;
    }

    // Free all sta IDs with this bssid
    if ((bssIdx < pMac->hal.halMac.maxBssId))
    {
        palZeroMemory(pMac->hHdd, (void *) &t[bssIdx], sizeof(tBssStruct));
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/****************************************************************************/
/*                UTILITY FUNCTIONS                                         */
/****************************************************************************/
//
//////////////////////////////////////////////////////////////////////////////
// halTable_FindBssidByAddr - Search BSSID in BSSID table
//
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_FindBssidByAddr(tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U8 *id)
{
    tANI_U8 i;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    eHalStatus status = eHAL_STATUS_BSSID_INVALID;

    for (i=0; i < pMac->hal.halMac.maxBssId; i++, t++)
    {
        if ((t->valid == 1) && (sirCompareMacAddr(t->bssId, bssId)))
        {
            *id = i;
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }

    return status;

}


/* -----------------------------------------------
 * FUNCTION:  halTable_FindStaidByAddr()
 *
 * NOTE:
 *   Given a station mac address, search for the
 *   corresponding station index from the Station
 *   Table.
 * -----------------------------------------------
 */
eHalStatus
halTable_FindStaidByAddr(tHalHandle hHalHandle, tSirMacAddr staAddr, tANI_U8 *id)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
#if defined(ANI_OS_TYPE_LINUX)
    status = halTable_StaIdCacheFind(pMac, staAddr, id);
#ifdef HAL_TABLE_API_DEBUG
    if (status == eHAL_STATUS_SUCCESS)
        halLog(pMac, LOG2, "%s: macAdr 0x%x:%x:%x:%x:%x:%x Found in mac cache id %d\n",__FUNCTION__, 
        staAddr[0], staAddr[1], staAddr[2],
        staAddr[3], staAddr[4], staAddr[5], *id);
#endif
#else
    tANI_U8 i;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    for (i=0; i < pMac->hal.halMac.maxSta; i++, t++)
    {
        if ( (t->valid == 1) && (sirCompareMacAddr(t->staAddr, staAddr)) )
        {
            *id = i;
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }
#ifdef HAL_TABLE_API_DEBUG
    halLog( pMac, LOG2, FL( "For macAddr 0x%x:%x:%x:%x:%x:%x, staid=%d, status = %d\n" ),
        staAddr[0], staAddr[1], staAddr[2],
        staAddr[3], staAddr[4], staAddr[5], i, status );
#endif
#endif
    return status;
}


/** ------------------------------------------------------------
 * @function :  halTable_FindStaIdByAssocId
 * @brief       :     This Function gets traversals through the list of Store associd's in the 
 *                           station table and if the id matches, then returns the StationId
 * 
 * ------------------------------------------------------------
 */
eHalStatus halTable_FindStaIdByAssocId(tpAniSirGlobal pMac, tANI_U16 assocId, tANI_U8 *staId)
{
    tANI_U8       i;
    tpStaStruct   pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    eHalStatus    status = eHAL_STATUS_FAILURE;
    
    for (i=0; i < pMac->hal.halMac.maxSta; i++, pSta++) {
        if ( (pSta->valid) && (pSta->assocId == assocId) ) {
            *staId = pSta->staId;
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }

    return status;
}

/* -----------------------------------------------
 * FUNCTION:  halTable_FindAddrByStaid()
 *
 * NOTE:
 *   Given a staid, search for station mac address,
 * -----------------------------------------------
 */
eHalStatus
halTable_FindAddrByStaid(tpAniSirGlobal pMac, tANI_U8 id, tSirMacAddr staAddr)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;


    palCopyMemory(pMac->hHdd, (void *) staAddr, (void *) t[id].staAddr, 6);

    return eHAL_STATUS_SUCCESS;

}

/* -------------------------------------------------------------
 * FUNCTION:  halTable_FindAddrByBssid()
 *
 * NOTE:
 *   Given a BSS index, search for corresponding BSS MAC address
 * -------------------------------------------------------------
 */
eHalStatus halTable_FindAddrByBssid( tpAniSirGlobal pMac, tANI_U8 bssIdx, tSirMacAddr bssId )
{
tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
eHalStatus status = eHAL_STATUS_SUCCESS;

  if( bssIdx < pMac->hal.halMac.maxBssId )
    palCopyMemory( pMac->hHdd,
        (void *) bssId,
        (void *) t[bssIdx].bssId,
        SIR_MAC_ADDR_LENGTH );
  else
  {
    status = eHAL_STATUS_BSSID_INVALID;
    HALLOGW( halLog( pMac, LOGW, FL("Invalid BSS index [%d] specified\n"),
        bssIdx ));
  }

  return status;
}

/****************************************************************************/
/*                UTILITY FUNCTIONS                                         */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// halTable_SaveStaConfig - Save station entry in sta table.
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_SaveStaConfig(tpAniSirGlobal pMac, tHalCfgSta *staEntry, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *) &t[staIdx].staParam, (tANI_U8 *)staEntry, sizeof(tHalCfgSta));
        t[staIdx].staId = staIdx;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

///////////////////////////////////////////////////////////////////////////////////////
// halTable_GetStaConfig - Get a pointer to the HAL station config from STA table.
//////////////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_GetStaConfig(tpAniSirGlobal pMac, tHalCfgSta **staEntry, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        *staEntry = &t[staIdx].staParam;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}


//////////////////////////////////////////////////////////////////////////////
// halTable_RestoreStaConfig - Restore station entry from STA table.
//////////////////////////////////////////////////////////////////////////////
eHalStatus halTable_RestoreStaConfig(tpAniSirGlobal pMac, tHalCfgSta *staEntry, tANI_U8 staIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if (staIdx < pMac->hal.halMac.maxSta)
    {
        palCopyMemory(pMac->hHdd, (tANI_U8 *)staEntry, (tANI_U8 *) &t[staIdx].staParam, sizeof(tHalCfgSta));
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

eHalStatus halTable_SaveBssConfig(tpAniSirGlobal pMac, 
         tANI_U8 rfBand, bssRaParam config, tANI_U8 bssIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U32     u32Update=0;

    if ((bssIdx < pMac->hal.halMac.maxBssId) && (t[bssIdx].valid))
    {
        u32Update = t[bssIdx].bssRaInfo.u.dword ^ config.dword;
        if(u32Update) {
           t[bssIdx].bssRaInfo.u.dword = config.dword;
           halMacRaBssInfoToFW(pMac, &t[bssIdx].bssRaInfo, (tANI_U8)bssIdx);
        }
        if(!config.bit.rifsMode){
            pMac->hal.halMac.nonRifsBssCount ++;
        }else{
            pMac->hal.halMac.rifsBssCount ++;
        }

        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

eHalStatus halTable_GetBssRaConfig(tpAniSirGlobal pMac, 
         tANI_U8 rfBand, bssRaParam *config, tANI_U8 bssIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if ((bssIdx < pMac->hal.halMac.maxBssId) && (t[bssIdx].valid))
    {
        config->dword = t[bssIdx].bssRaInfo.u.dword;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

#ifdef WLAN_FEATURE_VOWIFI
eHalStatus halTable_GetTxPowerLimitIndex(tpAniSirGlobal pMac, 
         tANI_U8 rfBand, tPwrTemplateIndex *maxTxPwrIndex)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U8      bssIdx = 0;;

    *maxTxPwrIndex = 0;
    
    while (bssIdx < pMac->hal.halMac.maxBssId)
    {
        if (t[bssIdx].valid)
        {
            if (t[bssIdx].bssRaInfo.u.bit.maxPwrIndex > *maxTxPwrIndex)
            {
                *maxTxPwrIndex = (tPwrTemplateIndex)t[bssIdx].bssRaInfo.u.bit.maxPwrIndex;
                status = eHAL_STATUS_SUCCESS;
            }
        }
        bssIdx++;
    }

    return status;
}
#endif  /* WLAN_FEATURE_VOWIFI */

eHalStatus
halTable_SaveEncMode(tpAniSirGlobal pMac, tANI_U8 staIdx, tAniEdType encMode)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if(staIdx < pMac->hal.halMac.maxSta)
    {
    t[staIdx].encMode = encMode;
        status = eHAL_STATUS_SUCCESS;
    }
    return status;
}

eHalStatus halTable_GetStaTidToAcMap(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U32 *pTidToAcMap)
{
  tpStaStruct   t = (tpStaStruct) pMac->hal.halMac.staTable;

  if((staIdx < pMac->hal.halMac.maxSta) && (0 != t[staIdx].valid))
  {
    *pTidToAcMap = t[staIdx].staParam.txTidAcMap;

    if ( *pTidToAcMap == 0)
        *pTidToAcMap = HAL_WMM_MAP_ALL_TID_TO_BE_MAP;

    return eHAL_STATUS_SUCCESS;
  }
  else
    return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaTidToAcMap(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U32 tidToAcMap)
{
  tpStaStruct   t = (tpStaStruct) pMac->hal.halMac.staTable;

  if((staIdx < pMac->hal.halMac.maxSta) && (0 != t[staIdx].valid))
  {
    t[staIdx].staParam.txTidAcMap = tidToAcMap;
    return eHAL_STATUS_SUCCESS;
  }
  else
    return eHAL_STATUS_INVALID_STAIDX;
}
/** Nobody is using this function*/
#ifdef FIXME_GEN6 
eHalStatus halTable_GetBssType(tpAniSirGlobal pMac, tANI_U8 bssId, tANI_U8 *bssType)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if ((bssId < pMac->hal.halMac.maxBssId) && (t[bssId].valid))
    {
        // return
        if (t[bssId].addBssParam.fApMode)
            *bssType = 1;
        else
            *bssType = 0;

        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}
#endif

eHalStatus halTable_ValidateStaIndex(tpAniSirGlobal pMac, tANI_U8 staId)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staId < pMac->hal.halMac.maxSta) && (pSta[staId].valid))
        return eHAL_STATUS_SUCCESS;
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_ValidateBssIndex(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid))
        return eHAL_STATUS_SUCCESS;
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}

eHalStatus halTable_SetBeaconIntervalForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U16 bcnInterval)
{
     tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;

     if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid))
     {
        pBss[bssIdx].tuBeaconInterval = bcnInterval;
        return eHAL_STATUS_SUCCESS;
     }
     else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetBeaconIntervalForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U16 *pbcnInterval)
{
     tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;

     if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid))
     {
        *pbcnInterval = pBss[bssIdx].tuBeaconInterval;
        return eHAL_STATUS_SUCCESS;
     }
     else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetSsidForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tSirMacSSid ssId)
{
     tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;

     if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid))
     {
        vos_mem_copy(&(pBss[bssIdx].ssId), &ssId, sizeof(tSirMacSSid));
        return eHAL_STATUS_SUCCESS;
     }
     else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetSsidForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tSirMacSSid *ssId)
{
     tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;

     if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid))
     {
        vos_mem_copy(ssId, &(pBss[bssIdx].ssId), sizeof(tSirMacSSid));
        return eHAL_STATUS_SUCCESS;
     }
     else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaType(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pStaType)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pStaType = t[staIdx].staType;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaType(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 staType)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].staType = staType;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaAddr(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 **pStaAddr)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pStaAddr = t[staIdx].staAddr;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}


eHalStatus halTable_SetStaAddr(tpAniSirGlobal pMac, tANI_U8 staIdx, tSirMacAddr staAddr)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        palCopyMemory(pMac->hHdd, t[staIdx].staAddr, staAddr, 6);
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 dpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].dpuIndex = dpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pDpuIdx = t[staIdx].dpuIndex ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaBcastDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 dpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].bcastDpuIndex = dpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaBcastDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pDpuIdx = t[staIdx].bcastDpuIndex ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 dpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].bcastMgmtDpuIndex = dpuIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaBcastMgmtDpuIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDpuIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pDpuIdx = t[staIdx].bcastMgmtDpuIndex ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaAssocId(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U16 assocId)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].assocId = assocId;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaAssocId(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U16 *pAssocId)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pAssocId = t[staIdx].assocId ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

/* note for Rate Adaption in FW:
at the time of porting, halTable_SetStaMaxAmpduDensity() is called only before halMacRaStaInit().
That's why it currently doesn't call _updateFwRaInfo() here, to reduce the number of transaction.
halMacRaStaInit() will call _updateFwRaInfo().
If it is not true, and it can be changed any time, it should call _updateFwRaInfo() before return
*/
eHalStatus halTable_SetStaMaxAmpduDensity(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 density)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].halRaInfo.maxAmpduDensity = density;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

/* firmware never modify this, so host can read this value whatever it wrotes  */
eHalStatus halTable_GetStaMaxAmpduDensity(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pDensity)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pDensity = t[staIdx].halRaInfo.maxAmpduDensity;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}



eHalStatus halTable_SetStaSignature(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 staSignature)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].staSig = staSignature;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_SIGNATURE;
}

eHalStatus halTable_GetStaSignature(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pStaSignature)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pStaSignature = t[staIdx].staSig;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_SIGNATURE;
}

eHalStatus halTable_SetStaUMAIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 umaIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].umaIdx = umaIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaUMAIdx(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *pUMAIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *pUMAIdx = t[staIdx].umaIdx ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

// Save the oprateMode passed to us from PE or when we init it.
eHalStatus halTable_SetStaopRateMode(tpAniSirGlobal pMac, 
        tANI_U8 staIdx, tStaRateMode  opRateMode)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].opRateMode = opRateMode;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

// Get the oprateMode 
eHalStatus halTable_GetStaopRateMode(tpAniSirGlobal pMac, 
        tANI_U8 staIdx, tStaRateMode *popRateMode)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        *popRateMode = t[staIdx].opRateMode ;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaAuthState(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_BOOLEAN authenticated)
{
    tANI_U8 staSignature;
    eHalStatus status;

#if defined(ANI_DVT_DEBUG)
    return eHAL_STATUS_SUCCESS;
#endif

    do
    {
        status = halTable_GetStaSignature(pMac, staIdx, &staSignature);
        if(eHAL_STATUS_SUCCESS != status)
        {
            HALLOGW( halLog(pMac, LOGW, FL("halTable_GetStaSignature() invoked for non-existant STAID %d.\n"), staIdx));
            break;
        }

        if(authenticated)
        {
            staSignature |= HAL_STA_SIG_AUTH_MASK;
        }
        else
        {
            staSignature &= ~HAL_STA_SIG_AUTH_MASK;
        }

        status = halTable_SetStaSignature(pMac, staIdx, staSignature);
        if(eHAL_STATUS_SUCCESS != status)
        {
            HALLOGW( halLog(pMac, LOGW, FL("halTable_SetStaSignature() invoked for non-existant STAID %d.\n"), staIdx));
        }

    } while(0);

    return status;
}

__DP_SRC_TX tANI_BOOLEAN halTable_IsStaAuthenticated(tHalHandle hHalHandle, tANI_U8 staIdx)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    t = &t[staIdx];
    if ((staIdx < pMac->hal.halMac.maxSta) && (t->valid) && (HAL_STA_SIG_AUTH_MASK & t->staSig))
        return eANI_BOOLEAN_TRUE;
    else
        return eANI_BOOLEAN_FALSE;
}

eHalStatus halTable_GetStaQosEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *qosEnabled)
{
     tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
     if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid) && qosEnabled)
     {
         *qosEnabled = t[staIdx].qosEnabled;
         return eHAL_STATUS_SUCCESS;
     }
     else
         return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetStaQosEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 qosEnabled)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].qosEnabled = qosEnabled;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}

/* note for Rate Adaption in FW:
at the time of porting, halTable_SetStaHtEnabled() is called only once before halMacRaStaInit().
That's why it currently doesn't call _updateFwRaInfo() here, to reduce the number of transaction.
halMacRaStaInit() will call _updateFwRaInfo().
If it is not true, and it can be changed any time, it should call _updateFwRaInfo() before return
*/
eHalStatus halTable_SetStaHtEnabled(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 htEnabled, tANI_U8 gfEnabled)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        t[staIdx].htEnabled= htEnabled;
        t[staIdx].halRaInfo.gfEnabled = gfEnabled;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;
}


#ifdef HAL_SELF_STA_PER_BSS
eHalStatus halTable_GetBssSelfStaIdxForSta(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 *idx)
{
     tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
     if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid)) {
         *idx = t[staIdx].bssSelfStaIdx;
         return eHAL_STATUS_SUCCESS;
     } else {
         return eHAL_STATUS_INVALID_STAIDX;
     }
}

eHalStatus halTable_SetBssSelfStaIdxForSta(tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 idx)
{
     tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
     if ((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid)) {
         t[staIdx].bssSelfStaIdx = idx;
         return eHAL_STATUS_SUCCESS;
     } else {
         return eHAL_STATUS_INVALID_STAIDX;
     }
}

eHalStatus halTable_GetBssSelfStaIdxForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *idx)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid)) {
        *idx = pBss[bssIdx].bssSelfStaIdx;
        return eHAL_STATUS_SUCCESS;
    } else {
        return eHAL_STATUS_INVALID_STAIDX;
    }
}

eHalStatus halTable_SetBssSelfStaIdxForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 idx)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid)) {
        pBss[bssIdx].bssSelfStaIdx = idx;
        return eHAL_STATUS_SUCCESS;
    } else {
        return eHAL_STATUS_INVALID_STAIDX;
    }
}
#endif

#ifdef HAL_BCAST_STA_PER_BSS
eHalStatus halTable_SetBssBcastStaIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 idx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    if(bssIdx < pMac->hal.halMac.maxBssId)
    {
        t[bssIdx].bcastStaIdx = idx;
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_INVALID_BSSIDX;
}

eHalStatus halTable_GetBssBcastStaIdx(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *idx)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    if(bssIdx < pMac->hal.halMac.maxBssId)
    {
        *idx = t[bssIdx].bcastStaIdx;
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_INVALID_BSSIDX;
}
#endif

// Find the staId for a station
eHalStatus halTable_FindStaidByAddrAndType(tpAniSirGlobal pMac, tSirMacAddr staAddr, tANI_U8 *id, tANI_U8 type)
{
    tANI_U8 i;
    tpStaStruct t = pMac->hal.halMac.staTable;
    eHalStatus status = eHAL_STATUS_STA_INVALID;

    for (i=0; i < pMac->hal.halMac.maxSta; i++)
    {
        if ((t->valid == 1) && sirCompareMacAddr(t->staAddr, staAddr) && (t->staType == type))
        {
            *id = i;
            status = eHAL_STATUS_SUCCESS;
            break;
        }
    }

    return status;
}

/*
 * Save the DTIM period as advertised by the AP in the bss table
 */
eHalStatus halTable_SetDtimPeriod( tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 dtim)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if(bssIdx < pMac->hal.halMac.maxBssId)
    {
        t[bssIdx].bcnDtimPeriod = dtim;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}


/*
 * Get the DTIM period for the specified BSS
 */
eHalStatus halTable_GetDtimPeriod( tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 *pDtim)
{
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if((bssIdx < pMac->hal.halMac.maxBssId) && (t[bssIdx].valid))
    {
        *pDtim = t[bssIdx].bcnDtimPeriod;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_BSSIDX;
}


/*
 * Following are defined as static inline for use in data path
 */
eHalStatus halTable_GetBssIndexForSta(tpAniSirGlobal pMac, tANI_U8 *bssIdx, tANI_U8 staIdx)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staIdx < pMac->hal.halMac.maxSta) && (pSta[staIdx].valid))
    {
        *bssIdx = pSta[staIdx].bssIdx;
        HALLOGW( halLog(pMac, LOGW, FL(" BssIdx = %d for Sta = %d\n"), *bssIdx, staIdx));
        /* TODO: This is a hack */
        if (staIdx == 0) {
            *bssIdx = 0;
        }
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_SetBssIndexForSta(tpAniSirGlobal pMac, tANI_U8 staId, tANI_U8 bssIdx)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    if ((staId < pMac->hal.halMac.maxSta) && (t[staId].valid))
    {
        t[staId].bssIdx = bssIdx;
        return eHAL_STATUS_SUCCESS;
    }
        return eHAL_STATUS_INVALID_STAIDX;
}

eHalStatus halTable_GetStaIndexForBss(tpAniSirGlobal pMac, tANI_U8 bssIndex, tANI_U8 *staIndex)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if ((bssIndex < pMac->hal.halMac.maxBssId) && (t[bssIndex].valid))
    {
        *staIndex = t[bssIndex].staIdForBss;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/*
 * Save the sta Index for bssid in BSS entry.
 */
eHalStatus halTable_SetStaIndexForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 staIdx)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    if (((staIdx < pMac->hal.halMac.maxSta) && (pSta[staIdx].valid)) &&
        ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid)))
    {
        pBss[bssIdx].staIdForBss = staIdx;
        return eHAL_STATUS_SUCCESS;
    }
    else
        return eHAL_STATUS_INVALID_STAIDX;

}

/*
 * Get the OBSS protection for the bssidx in BSS entry.
 */
eHalStatus halTable_GetObssProtForBss(tpAniSirGlobal pMac, tANI_U8 bssIndex, tANI_U8 *obssProt)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if ((bssIndex < pMac->hal.halMac.maxBssId) && (t[bssIndex].valid)) {
        *obssProt = t[bssIndex].obssProtEnabled;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

eHalStatus halTable_GetBcastStaIndexForBss(tpAniSirGlobal pMac, tANI_U8 bssIndex, tANI_U8 *staIndex)
{
    eHalStatus status = eHAL_STATUS_INVALID_BSSIDX;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;

    if ((bssIndex < pMac->hal.halMac.maxBssId) && (t[bssIndex].valid))
    {
        *staIndex = t[bssIndex].bcastStaIdx;
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/*
 * Save the OBSS protection for the bssidx in BSS entry.
 */
eHalStatus halTable_SetObssProtForBss(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 obssProt)
{
    tpBssStruct pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
    if ((bssIdx < pMac->hal.halMac.maxBssId) && (pBss[bssIdx].valid)) {
        pBss[bssIdx].obssProtEnabled = obssProt;
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_INVALID_BSSIDX;
}



/* -------------------------------------------------------
 * FUNCTION:  halIsBssTableEmpty()
 *
 * NOTE:
 *   Return FAILURE if the BSS table is NOT empty.
 *   Return SUCCESS if the BSS table is empty.
 * -------------------------------------------------------
 */
eHalStatus halIsBssTableEmpty(tpAniSirGlobal pMac)
{
    tANI_U16    i;
    tpBssStruct table = (tpBssStruct) pMac->hal.halMac.bssTable;

    for (i=0; i < pMac->hal.halMac.maxBssId; i++)
    {
        if (table->valid == 1)
        {
            HALLOGW(halLog(pMac, LOGW, FL("BSS Table is not Empty. \n")));
            return eHAL_STATUS_FAILURE;
        }
        table++;
   }
#ifdef HAL_TABLE_API_DEBUG
   HALLOG1(halLog(pMac, LOG1, FL("BSS Table is Empty. \n")));
#endif
   return eHAL_STATUS_SUCCESS;
}


/*************************************************
 * FUNCTION: halSta_getDefaultRCDescriptorFields
 *
 * NOTE:
 * This API can be used to determine the default
 * configuration that should be applied to the
 * Nova DPU Replay Counter Descriptor entries.
 *
 * Primarily, the RC Fields of interest are:
 * RCE - Replay Check Enable
 * WCE - Window Check Enable
 * winChkSize - Window Check Size, applicable if
 *              WCE is enabled
 *
 * ASSUMPTIONS:
 * The parameter winChkSize can hold a maximum of
 * MAX_NUM_OF_TIDS
 *
 ************************************************/
eHalStatus halSta_getDefaultRCDescriptorFields( tpAniSirGlobal pMac,
    tHalCfgSta staEntry,
    tANI_U16 *rce,
    tANI_U16 *wce,
    tANI_U8  *winChkSize )
{
#ifdef FIXME_GEN5
  if( NULL == rce ||
      NULL == wce ||
      NULL == winChkSize )
    return eHAL_STATUS_INVALID_PARAMETER;

  switch( staEntry.encMode )
  {
    case eSIR_ED_NONE:
    case eSIR_ED_WEP40:
    case eSIR_ED_WEP104:
      *rce = HAL_DPU_DEFAULT_RCE_OFF;
      break;

    case eSIR_ED_TKIP:
    case eSIR_ED_CCMP:
    default:
      *rce = HAL_DPU_DEFAULT_RCE_ON;
      break;
  }

  // Determine if we need to turn WCE ON
  palZeroMemory( pMac->hHdd, winChkSize, MAX_NUM_OF_TIDS );

  //
  // Now, look inside "this STA config" to determine the following:
  // a) This STA is 11n capable - cap11nHT
  // b) A-MPDU is enabled (?)
  // c) A-MPDU parameters are setup appropriately (?)
  // d) In the Per TCID configuration, check for -
  //    --> fUseBARx
  //    --> fRxBApolicy (Immediate/Delayed)
  //
  // FIXME - Should we even check for cap11nHT or for just BA?
  if( staEntry.cap11nHT &&
      eSIR_ED_CCMP == staEntry.encMode )
  {
  tANI_U16 tid = 0;

    // FIXME - SoftMAC TID range is 0..7 but
    //         DPU TID range is 0..15 :-(
    for( tid = 0; tid < STACFG_MAX_TC; tid++ )
    {
      if( staEntry.tcCfg[tid].fUseBARx |
          staEntry.tcCfg[tid].fRxBApolicy ) // Immediate BA
      {
        *wce |= (0x1 << tid);
        winChkSize[tid] = (tANI_U8) staEntry.tcCfg[tid].rxBufSize;
      }
    }
  }
  else
    *wce = HAL_DPU_DEFAULT_WCE_OFF;
#endif
  return eHAL_STATUS_SUCCESS;
}

/**
 * \brief In the HAL STA Table, if a new Rx BA session
 * gets established (via a peer-triggered ADDBA REQ),
 * then we need to update the BA Session ID for the
 * corresponding TID
 *
 * \sa halTable_SetStaBASessionID
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staIdx The STA index for which a new BA
 * session is being established
 *
 * \param baTID The TID for which the BA session is setup
 *
 * \param baSessionID The BA Session ID
 *
 * \return retStatus Indicates whether we were able to
 * successfully update the STA entry or not
 *
 */
eHalStatus halTable_SetStaBASessionID( tpAniSirGlobal pMac,
    tANI_U8 staIdx,
    tANI_U8 baTID,
    tANI_U16 baSessionID )
{
tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

  if((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
  {
    t[staIdx].baSessionID[baTID] = baSessionID;
    return eHAL_STATUS_SUCCESS;
  }
  else
    return eHAL_STATUS_INVALID_STAIDX;
}

/** -------------------------------------------------------------
\fn halTable_SetStaAddBAReqParams
\brief stored addBA related information to process response from HDD
\      and softmac.
\param     tpAniSirGlobal    pMac
\param     tANI_U16 staIdx
\param     tANI_U8 tid
\param     tSavedAddBAReqParamsStruct addBAReqParamsStruct : info to be stored.
\return    eHalStatus : success/failure
  -------------------------------------------------------------*/
eHalStatus halTable_SetStaAddBAReqParams( tpAniSirGlobal pMac,
    tANI_U16 staIdx,
    tANI_U8 tid,
    tSavedAddBAReqParamsStruct addBAReqParamsStruct)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid))
    {
        if(tid < STACFG_MAX_TC)
        {
            t[staIdx].addBAReqParams[tid] = addBAReqParamsStruct;
            status = eHAL_STATUS_SUCCESS;
        }
    }
    else
        status = eHAL_STATUS_INVALID_STAIDX;

    return status;
}

/** -------------------------------------------------------------
\fn halTable_GetStaAddBAParams
\brief retrieves the addBAReq related information from sta table 
\      after getting the response back from smac.
\param     tpAniSirGlobal    pMac
\param     pAddBAParams - the info to be retrieved.
\return    eHalStatus : success/failure
  -------------------------------------------------------------*/
eHalStatus halTable_GetStaAddBAParams( tpAniSirGlobal pMac,
    tSavedAddBAReqParamsStruct* pStaAddBAParams)
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U16 staIdx;
    tANI_U8 tid;
    for(staIdx = 0; staIdx < pMac->hal.halMac.maxSta; staIdx++)
    {
        if(t[staIdx].valid)
        {
            for(tid = 0; tid < STACFG_MAX_TC; tid++)
            {
                *pStaAddBAParams = t[staIdx].addBAReqParams[tid];            
                if(pStaAddBAParams->pAddBAReqParams)
                {
                    return eHAL_STATUS_SUCCESS;
                }
            }
        }
    }
    return eHAL_STATUS_FAILURE;
}

/*
 * find the index of the first STA in the sta table which belongs to the
 * specified BSS
 */
eHalStatus
halTable_FindStaInBss(
    tpAniSirGlobal  pMac,
    tANI_U8         bssIdx,
    tANI_U8         *pStaIdx)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U32    numSta;
    for (numSta = 0; (numSta < pMac->hal.halMac.maxSta); numSta++, pSta++)
    {
        // look only for a valid peer in the same bss
        if ( (! pSta->valid)             ||
             ( pSta->bssIdx  != bssIdx ) ||
             ( pSta->staType != STA_ENTRY_PEER ) )
            continue;
        *pStaIdx = pSta->staId;
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_FAILURE;
}

eHalStatus halTable_SetStaTxConfig( tpAniSirGlobal pMac,
    tANI_U32 staIdx,
    tANI_U32 txConfig) 
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid)) {
        t[staIdx].txConfig = (tBmuStaTxCfgCmd)txConfig;
        return eHAL_STATUS_SUCCESS;
    } else {
        return eHAL_STATUS_INVALID_STAIDX;
    }
}

eHalStatus halTable_GetStaTxConfig( tpAniSirGlobal pMac,
    tANI_U32 staIdx,
    tANI_U32 *pTxConfig) 
{
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    if((staIdx < pMac->hal.halMac.maxSta) && (t[staIdx].valid)) {
        *pTxConfig = (tANI_U32)t[staIdx].txConfig;
        return eHAL_STATUS_SUCCESS;
    } else {
        return eHAL_STATUS_INVALID_STAIDX;
    }
}

tANI_U8 halTable_UpdateStaRefCount ( tpAniSirGlobal pMac, tANI_U8 staIdx, tANI_U8 add )
{
   tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

   if( add )
      return ++t[staIdx].refCount;

   return --t[staIdx].refCount;
}
#if defined(ANI_OS_TYPE_LINUX)
/**
 * \brief Adds station the the cache table. If the staId already exists
 *  delete entry before adding a new one. The only case when the station mac 
 *  can already be in the table is when we add AP mac.
 *
 * \sa halTable_AddToStaCache
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staAddr Sta mac address
 *
 * \param staId Sta id
 *
 * \return void
 *
 */

eHalStatus halTable_AddToStaCache(tpAniSirGlobal pMac, tANI_U8 *staAddr, tANI_U8 staId)
{
    tStaCacheEntry *pStaCacheInfo = (tStaCacheEntry *) pMac->hal.halMac.pStaCacheInfo;
    tANI_U8 *pStaCacheHashTab = pMac->hal.halMac.staCache;
    tANI_U8 staFreeIdx;
    if(pStaCacheInfo[0].staId == staId)
    {
        // This can be only for BSSID staID == 0
#ifdef HAL_STATABLE_CACHE_DEBUG            
        tANI_U8 *mac2 = (tANI_U8 *)&pStaCacheInfo[0].macAddressHi;
        tANI_U8 *mac1 = (tANI_U8 *)&pStaCacheInfo[0].macAddressLo;
                    
        // Remove the old entry in pStaCacheHashTab
        HALLOGW( halLog(pMac, LOGW, FL(" staID [%d] already in the table\n"), staId));
        HALLOGW( halLog(pMac, LOGW, FL(" Cached Sta Mac Entry [%d]: %02x:%02x:%02x:%02x:%02x:%02x\n"),
         staId, mac1[0], mac1[1], mac1[2],mac1[3],mac2[0], mac2[1])); 
#endif        
        halTable_RemoveFromStaCache(pMac,(tANI_U8 *)&pStaCacheInfo[0].macAddressLo);
    }
    staFreeIdx = pMac->hal.halMac.staCacheInfoFreeHeadIndex;
    if (staFreeIdx != 0xff)
    {
#ifdef HAL_STATABLE_CACHE_DEBUG            
        tANI_U8 *mac1 = (tANI_U8 *)staAddr;
        
        HALLOGE( halLog(pMac, LOGE, FL("Mac added [%d]: %02x:%02x:%02x:%02x:%02x:%02x\n"),
         staId, mac1[0], mac1[1], mac1[2],mac1[3],mac1[4], mac1[5])); 
#endif        
        pStaCacheInfo[staFreeIdx].staId = staId;
        sirCopyMacAddr((tANI_U8 *)&(pStaCacheInfo[staFreeIdx].macAddressLo), staAddr);
        pMac->hal.halMac.staCacheInfoFreeHeadIndex = pStaCacheInfo[staFreeIdx].nextCacheIdx;
        pStaCacheInfo[staFreeIdx].nextCacheIdx = pStaCacheHashTab[ STA_ADDR_HASH(staAddr) ];
        pStaCacheHashTab[ STA_ADDR_HASH(staAddr) ] = staFreeIdx;
        return eHAL_STATUS_SUCCESS;
    }
    return eHAL_STATUS_FAILURE;
}


/**
 * \brief Remove station the the cache table. 
 *
 * \sa halTable_RemoveFromStaCache
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staAddr Sta mac address
 *
 * \param staId Sta id
 *
 * \return void
 *
 */

void halTable_RemoveFromStaCache(tpAniSirGlobal pMac,tSirMacAddr staAddr)

{

    tStaCacheEntry *pStaCacheInfo = (tStaCacheEntry *) pMac->hal.halMac.pStaCacheInfo;    
    tANI_U8 *pStaCacheHashTab = pMac->hal.halMac.staCache;
    tANI_U8 *prev = &pStaCacheHashTab[STA_ADDR_HASH(staAddr)];
    tANI_U8 curr;

    for (curr = *prev; curr != 0xff; prev = &pStaCacheInfo[curr].nextCacheIdx, curr = pStaCacheInfo[curr].nextCacheIdx)
    {            
        if (sirCompareMacAddr((tANI_U8 *)&(pStaCacheInfo[curr].macAddressLo), staAddr) && pStaCacheInfo[curr].staId != 0xff)
        {
#ifdef HAL_STATABLE_CACHE_DEBUG     
            tANI_U8 *mac2 = (tANI_U8 *)&pStaCacheInfo[curr].macAddressHi;
            tANI_U8 *mac1 = (tANI_U8 *)&pStaCacheInfo[curr].macAddressLo;
            HALLOGE( halLog(pMac, LOGE, FL(" Cached Sta [%d] Mac: %02x:%02x:%02x:%02x:%02x:%02x\n"),
                          pStaCacheInfo[curr].staId, mac1[0], mac1[1], mac1[2],mac1[3],mac2[0], mac2[1])); 
#endif         
            *prev = pStaCacheInfo[curr].nextCacheIdx;
            pStaCacheInfo[curr].nextCacheIdx = pMac->hal.halMac.staCacheInfoFreeHeadIndex;
            pMac->hal.halMac.staCacheInfoFreeHeadIndex = curr;
            pStaCacheInfo[curr].staId = 0xff;
            break;
        }      
    }
}

/**
 * \brief Search station cache table. 
 *
 * \sa halTable_StaIdCacheFind
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param staAddr Sta mac address
 *
 * \param staId Sta id
 *
 * \return eHAL_STATUS_SUCCESS if mac is found and staID contains station index
 *         eHAL_STATUS_INVALID_STAIDX if mac in not found in the cache
 *
 */

eHalStatus  halTable_StaIdCacheFind(tpAniSirGlobal  pMac, tSirMacAddr mac, tANI_U8 *staID)
{

    tStaCacheEntry *pStaCacheInfo = (tStaCacheEntry *) pMac->hal.halMac.pStaCacheInfo;    
    tANI_U8 *pStaCacheHashTab = pMac->hal.halMac.staCache;
    tStaCacheEntry *hashCacheEntry;
    tANI_U8  hashTabEntry = pStaCacheHashTab[STA_ADDR_HASH(mac)]; //get hash line index
    
    if(hashTabEntry != 0xff)
    {

        if((((unsigned int)mac) & 3) == 0)
        { 
            // MAC is 4 byte aligned, do a 4-byte then 2-byte compare.
            tANI_U32 *pktMac4 = (tANI_U32 *)mac;
            tANI_U16 *pktMac2 = (tANI_U16 *)&mac[4];
            do
            {
            
                hashCacheEntry = &pStaCacheInfo[hashTabEntry]; 
                if((*pktMac2 == hashCacheEntry->macAddressHi) &&
                   (*pktMac4 == hashCacheEntry->macAddressLo))
                {
                    *staID = (tANI_U8)hashCacheEntry->staId;
                    return eHAL_STATUS_SUCCESS; //found in cache
                }
            }
            while ((hashTabEntry = hashCacheEntry->nextCacheIdx) != 0xff);
        }
        else if((((tANI_U32)mac) & 1) == 0)
        { 
            // MAC is 2 byte aligned, do three 2-byte compares.
            tANI_U16 *pktMac = (tANI_U16 *)mac;
            tANI_U16 *cached;
        
            do
            {
                hashCacheEntry = &pStaCacheInfo[hashTabEntry];
                cached = (tANI_U16 *)&(hashCacheEntry->macAddressLo);
                          
                if((pktMac[2] == cached[2]) &&
                   (pktMac[0] == cached[0]) &&
                   (pktMac[1] == cached[1]))
                {
               
                   *staID = (tANI_U8)hashCacheEntry->staId ;
                   return eHAL_STATUS_SUCCESS; //found in cache
                }           
            
            }
            while ((hashTabEntry = hashCacheEntry->nextCacheIdx) != 0xff);
        }
        else
        {
            // MAC is unaligned, do byte-by-byte comparison.        
            do
            {
                tANI_U8 *mac1; 
                hashCacheEntry = &pStaCacheInfo[hashTabEntry];
                mac1 = (tANI_U8 *)hashCacheEntry->macAddressLo;
            
                if ( (mac1[5]==mac[5])&&
                     (mac1[4]==mac[4])&&
                     (mac1[3]==mac[3])&&
                     (mac1[2]==mac[2])&&
                     (mac1[1]==mac[1])&&
                     (mac1[0]==mac[0]) ) 
                {
                
                    *staID = (tANI_U8)hashCacheEntry->staId;
                    return eHAL_STATUS_SUCCESS; //found in cache
                }            
            }
            while ((hashTabEntry = hashCacheEntry->nextCacheIdx) != 0xff);
        }
    }
    return eHAL_STATUS_INVALID_STAIDX; // not found in cache
}

eHalStatus halTable_StaCacheOpen(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U8 max_sta= (tANI_U8) pMac->hal.memMap.maxStations;

    (void) arg;
#if defined(RTL8652)&& !defined(CONFIG_RTL865XC_DMEM_STACK_RELOCATION)
    extern void * rtlglue_alloc_data_scratchpad_memory(tANI_U32 size,  char *);
    pMac->hal.halMac.pStaCacheInfo = 
            (void *)rtlglue_alloc_data_scratchpad_memory(max_sta * sizeof(tStaCacheEntry),"pStaCacheInfo");
    
    if (pMac->hal.halMac.pStaCacheInfo == NULL)    
         status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.pStaCacheInfo,
                                     max_sta * sizeof(tStaCacheEntry));

    pMac->hal.halMac.staCache = (tANI_U8 *)rtlglue_alloc_data_scratchpad_memory(STA_CACHE_SIZE, "staCache");

    if (pMac->hal.halMac.staCache == NULL)            
         status = palAllocateMemory(pMac->hHdd,(void **) &pMac->hal.halMac.staCache,
                                         STA_CACHE_SIZE);

#else    
    status = palAllocateMemory(pMac->hHdd, &pMac->hal.halMac.pStaCacheInfo,
                                    max_sta * sizeof(tStaCacheEntry));
    
    status = palAllocateMemory(pMac->hHdd,(void **) &pMac->hal.halMac.staCache,
                                    STA_CACHE_SIZE);
#endif    
    pMac->hal.halMac.staCacheInfoFreeHeadIndex = 0;
    return status;
}

eHalStatus halTable_StaCacheStart(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tANI_U8 max_sta = (tANI_U8) pMac->hal.memMap.maxStations;
    tStaCacheEntry *pStaCacheInfo = (tStaCacheEntry *) pMac->hal.halMac.pStaCacheInfo;
    int i;
    
    (void) arg;
    
    pMac->hal.halMac.staCacheInfoFreeHeadIndex = 0;
     /* Initialize staCache */      
    palFillMemory(pMac->hHdd, pMac->hal.halMac.pStaCacheInfo, 
        max_sta * sizeof( tStaCacheEntry ), 0xff);
    palFillMemory(pMac->hHdd, pMac->hal.halMac.staCache, 
        STA_CACHE_SIZE, 0xff);
    for(i=0; i<max_sta-1; i++)
    {
        pStaCacheInfo[i].nextCacheIdx = i+1;
    }
    pStaCacheInfo[i].nextCacheIdx = 0xff;

    return status;
}

eHalStatus halTable_StaCacheStop(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    (void) arg;
    
    return status;
}

eHalStatus halTable_StaCacheClose(tHalHandle hHal, void *arg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    (void) arg;
#if defined(RTL8652)
    extern void * rtlglue_is_data_scratchpad_memory(void *);
    if(! rtlglue_is_data_scratchpad_memory(pMac->hal.halMac.pStaCacheInfo )) 
         status = palFreeMemory(pMac->hHdd, pMac->hal.halMac.pStaCacheInfo);
    
    if(! rtlglue_is_data_scratchpad_memory(pMac->hal.halMac.staCache )) 
        status = palFreeMemory(pMac->hHdd, pMac->hal.halMac.staCache);

#else    
    if ((pMac->hal.halMac.pStaCacheInfo != NULL))
         status = palFreeMemory(pMac->hHdd, pMac->hal.halMac.pStaCacheInfo);
    if (pMac->hal.halMac.staCache) status = 
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.staCache);
#endif
    return status;
}

void halTable_StaCacheDump(tHalHandle hMac)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac; 
    tANI_U8 *pStaCacheInfoTab = pMac->hal.halMac.staCache;
    tANI_U8 max_sta = (tANI_U8) pMac->hal.memMap.maxStations;

    tStaCacheEntry *pMacCache = (tStaCacheEntry *) pMac->hal.halMac.pStaCacheInfo;    
    tStaCacheEntry *thisCache;
    tANI_U8 *mac1, *mac2;
    int i;
    HALLOGW( halLog(pMac, LOGW, FL("Maximum number of stations supported: %03d\n"), max_sta));
    for(i=0;i<max_sta;i++)
    {
        thisCache = &pMacCache[i]; 
        mac1 = (tANI_U8 *)&thisCache->macAddressLo;
        mac2 = (tANI_U8 *)&thisCache->macAddressHi;
        HALLOGW( halLog(pMac, LOGW, FL("Cached Sta Mac Entry [%03d]: %02x:%02x:%02x:%02x:%02x:%02x nextCacheIdx [%03d]\n"),
            thisCache->staId, mac1[0], mac1[1], mac1[2],mac1[3],mac2[0], mac2[1],thisCache->nextCacheIdx )); 
     } 
    for(i=0; i< STA_CACHE_SIZE; i++)
    {
        if (pStaCacheInfoTab[i] != 0xff)
            HALLOGW( halLog(pMac, LOGW, FL("pStaCacheInfoTab[%03d] = %03d\n"),i,  pStaCacheInfoTab[i]));
    }
    
}
#endif
