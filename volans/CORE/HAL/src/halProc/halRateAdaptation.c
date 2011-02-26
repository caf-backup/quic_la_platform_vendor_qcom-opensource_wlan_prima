/**
 *
 *  @file:         halRateAdaptation.c
 *
 *  @brief:       This file contains the core rate adpatation algorithm implementation.
 *
 *  @author:    Qualcomm Inc
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 02/15/2006  File created.
 * 01/15/2009  Virgo related chages made.
 * 08/12/2009  Move some parts in firmware
 */

#include <stdarg.h>
#include "palTypes.h"

#include "halDataStruct.h"
#include "sirMacProtDef.h"
#include "cfgApi.h"
#include "halRateAdaptation.h"  /* our private types/defs */
#include "halRateAdaptApi.h" /* our own api header */
#include "halRateTable.h"
#include "vos_trace.h"

/* ------------------------------------------------------------------- */
/* local protos */

static void
_raSetStaFixedRate(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8         staId,
    tHalMacRate     rate);

static void
_raStartStaAutoRate(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8         staId
);

static tSirRetStatus
_raRateIsSupportedAndValid(
    tpAniSirGlobal  pMac,
    tHalMacRate     rate,
    tANI_U32        *psRates,
    tANI_U32        *pvRates,
    tANI_U16        maxDataRate);

static tHalMacRate
_getLowestRateByNwType(
    eRfBandMode band,
    tStaRateMode operMode,
    tANI_U32 pure11g
);

static void
_updateAllStaRetryRateByCfg(
tpAniSirGlobal  pMac);

tSirRetStatus
halMacRaInitStaRate(
    tpAniSirGlobal  pMac,
    tpStaStruct   pSta);

static void
_raGetMaxSupportedValidRate(
    tpAniSirGlobal  pMac,
    tpStaStruct     pSta,
    tTpeRateIdx    *pMaxRate,
    tANI_U32        *pTput
);

static void
_fillSupportedPreTitanRatesByIeRates(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U16        *pSupportedIeRates,
    tANI_U8         numRates,
    tHalIeRateType  rateType);

#ifdef FEATURE_TX_PWR_CONTROL
static void
_fillSupportedHTRatesByMcsBitmap(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8        *pSupportedMcsRateBitmap,
    tANI_U8         numBytes,
    tANI_U8         isSelfSta);
#else
static void
_fillSupportedHTRatesByMcsBitmap(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8        *pSupportedMcsRateBitmap,
    tANI_U8         numBytes);
#endif

#define _raAddRecord(pMac,pSta,code,netTx,netFail,prevRate,newRate)

#ifdef RA_CB_ENABLED
static tHalMacRate _convert2NonCbRate(tHalMacRate halRate);
#endif
static tHalMacRate _convert2ShortPreambleRate(tHalMacRate halRate);
static void raLog(tpAniSirGlobal pMac, tANI_U32 level, const char *pStr,...);

/* only bssRaParam is writable */
eHalStatus 
halMacRaBssInfoToFW(
    tpAniSirGlobal pMac, 
    tpHalRaBssInfo pHalRaBssInfo, 
    tANI_U8 bssIdx)
{
    eHalStatus status;

    // Download the table to the target.
    status = halWriteDeviceMemory(pMac, QWLANFW_MEMMAP_RA_BSS_INFO_TABLE + sizeof(tpHalRaBssInfo)*bssIdx, \
            pHalRaBssInfo, sizeof(bssRaParam));

    halMacRaUpdateParamReq(pMac, RA_UPDATE_BSS_INFO, pHalRaBssInfo->u.dword);

    return status;
}
/* auto sample table readable */
eHalStatus 
halMacRaBssInfoFromFW(
    tpAniSirGlobal pMac, 
    tpHalRaBssInfo pHalRaBssInfo, 
    tANI_U8 bssIdx)
{
#ifdef ANI_LITTLE_BYTE_ENDIAN /* if host is little endian, firmware needs big endian */
    tANI_U8  beRaBssInfo[CEIL_ALIGN(sizeof(tHalRaBssInfo), 4)];
    tANI_U32 *src;
    tANI_U32 *dest;
    int i;
#endif    
    eHalStatus status;

   
#ifdef ANI_LITTLE_BYTE_ENDIAN /* if host is little endian, firmware needs big endian */
    /* initialze */

    memset((void *)(beRaBssInfo+sizeof(bssRaParam)), HALRATE_INVALID, sizeof(beRaBssInfo)-sizeof(bssRaParam));
    /* read from the firmware */
    status = halReadDeviceMemory(pMac, QWLANFW_MEMMAP_RA_BSS_INFO_TABLE+sizeof(tpHalRaBssInfo)*bssIdx, \
        &beRaBssInfo[0], sizeof(tHalRaBssInfo));
    
    /* copy bssRaParam because it doesn't require swap */
    memcpy((void *)pHalRaBssInfo, (const void *)beRaBssInfo, sizeof(bssRaParam));
    /* prepare swap */
    src = (tANI_U32 *)(beRaBssInfo+sizeof(bssRaParam));
    dest =(tANI_U32 *)&pHalRaBssInfo->bssAutoSampleRateTable[0];
        
    for(i=0; i<(sizeof(beRaBssInfo)-sizeof(bssRaParam))>>2;i++) {
        *dest = HTOFL(*src);
        src++; dest++;
    }
#else
     status = halReadDeviceMemory(pMac, QWLANFW_MEMMAP_RA_BSS_INFO_TABLE+sizeof(tpHalRaBssInfo)*bssIdx, \
                (void *)pHalRaBssInfo, sizeof(tHalRaBssInfo));
#endif
    return status;
}

eHalStatus
halMacRaGlobalInfoToFW(
    tpAniSirGlobal pMac,
    tpHalRaGlobalInfo pGlobInfo,
    tANI_U32 startOffset,
    tANI_U32 szLen)
{
    eHalStatus status;
    tANI_U32 swapNeededStartOffset = offsetof (tHalRaGlobalInfo, goodPerThreshBySensitivity);
//    tANI_U32 swapNeededEndOffset = sizeof (tHalRaGlobalInfo);
    tANI_U32 *pSwapPtr;
    tANI_BOOLEAN bSwapNeeded = 1;
    int i;

#ifdef ANI_LITTLE_BYTE_ENDIAN /* if host is little endian, firmware needs big endian */
    if(/*startOffset > swapNeededEndOffset ||*/ (startOffset + szLen < swapNeededStartOffset))
        bSwapNeeded = 0;
#endif

    startOffset &= ~3;
    szLen = CEIL_ALIGN(szLen+(startOffset & 3), 4);

#ifdef ANI_LITTLE_BYTE_ENDIAN /* if host is little endian, firmware needs big endian */
    if(bSwapNeeded) {
        pSwapPtr =(tANI_U32 *)&pGlobInfo->goodPerThreshBySensitivity[0];
        // Note that U8 array needs to be byte-swapped before sending palWriteDevice.
        for(i=0; i<CEIL_ALIGN(RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE,4)>>2;i++) {
            *pSwapPtr = HTOFL(*pSwapPtr);
            pSwapPtr++;
        }
    }
#endif
    status = halWriteDeviceMemory(pMac, QWLANFW_MEMMAP_RA_GLOBAL_CONFIG + startOffset, (void *)((tANI_U32)pGlobInfo + startOffset), szLen);

#ifdef ANI_LITTLE_BYTE_ENDIAN /* revert back if swapped */
    if(bSwapNeeded) {
        pSwapPtr =(tANI_U32 *)&pGlobInfo->goodPerThreshBySensitivity[0];

        for(i=0; i<CEIL_ALIGN(RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE,4)>>2;i++) {
            *pSwapPtr = HTOFL(*pSwapPtr);
            pSwapPtr++;
        }
    }
#endif    
    return status;
}

/* sRate/tRate is not writable */
static eHalStatus
halMacRaStaInfoToFW(
    tpAniSirGlobal pMac, 
    tpHalRaInfo    pRaInfo,
    tANI_U16       staId,
    tANI_U32       startOffset, 
    tANI_U32       szLen)
{
    eHalStatus status;

    startOffset &= ~3;
    szLen = CEIL_ALIGN(szLen+(startOffset & 3), 4);

    status = halWriteDeviceMemory(pMac, QWLANFW_MEMMAP_RA_STA_CONFIG+sizeof(tHalRaInfo)*staId + startOffset, \
                (void*)((tANI_U32)pRaInfo + startOffset), szLen);
    return status;
}

static eHalStatus
halMacRaStaInfoFromFW(
    tpAniSirGlobal pMac, 
    tpHalRaInfo    pRaInfo,
    tANI_U16       staId,
    tANI_U32       startOffset, 
    tANI_U32       szLen)
{
    eHalStatus status;
    tANI_U32 swapNeededStartOffset = offsetof (tHalRaInfo, sRateTable);
    tANI_U32 swapNeededEndOffset = sizeof (tHalRaInfo);
    tANI_U32 *pSwapPtr;
    tANI_BOOLEAN bSwapNeeded = 1;
    tANI_U32 i;

    if(/*startOffset > swapNeededEndOffset ||*/ (startOffset + szLen <= swapNeededStartOffset))
        bSwapNeeded = 0;

    
    startOffset &= ~3;
    szLen = CEIL_ALIGN(szLen+(startOffset & 3), 4);

    status = halReadDeviceMemory(pMac, QWLANFW_MEMMAP_RA_STA_CONFIG+sizeof(tHalRaInfo)*staId + startOffset, \
        (void *)((tANI_U32)pRaInfo + startOffset), szLen);

    if(bSwapNeeded) {
        pSwapPtr =(tANI_U32 *)&pRaInfo->sRateTable[0];
        // Note that U8 array needs to be byte-swapped after reading from firmware.
        for(i=0; i<(swapNeededEndOffset-swapNeededStartOffset)>>2;i++) {
            *pSwapPtr = HTOFL(*pSwapPtr);
            pSwapPtr++;
        }
    }
    return status;
}

eHalStatus
halMacRaUpdateReq(tpAniSirGlobal pMac, tFwRaUpdateEnum raUpdateEvent, tANI_U16 msgLen, tANI_U8 *msgBody)
{
   Qwlanfw_RaUpdateMsgType msg;
   eHalStatus status = eHAL_STATUS_FAILURE;
   
   /* send message to firmware */
   msg.raUpdateType = (tANI_U16)raUpdateEvent;
   msg.raUpdateMsgLen = msgLen;

   if(msgLen)
#ifdef WLAN_SOFTAP_FEATURE    
      vos_mem_copy(&msg.u.raUpdateParam, msgBody, msgLen);
//#else
//      vos_mem_copy(&msg.u.raAddStaMsg, msgBody, msgLen);
#endif
   
   // Send the RA start request to firmware
   status = halFW_SendMsg(pMac, HAL_MODULE_ID_RA, QWLANFW_HOST2FW_RA_UPDATE,
           0, sizeof(Qwlanfw_RaUpdateMsgType), &msg, FALSE, NULL);
   return status;
}

#ifndef FEATURE_RA_CHANGE
eHalStatus
halMacRaAddBssReq(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U8 selfStaIdx)
{
    Qwlanfw_RaAddBssMsgType msgBody;
    /*   tpBssStruct pThisBss; */
    tHalRaGlobalInfo *pGlobRaInfo = &pMac->hal.halRaInfo;
    eHalStatus status = eHAL_STATUS_FAILURE;

    /* Download the table to the target. */
    /* Note : As a workaround, update globalInfo here. This is supposed to be called
       after initializtaion of RaGlobalInfo in halMacRaStart(). 
       However, rtsThreshold and protPolicy is not initialized properly at that moment,
       because Cfg is not initialized yet. 
    */ 
    halMacRaGlobalInfoToFW(pMac, pGlobRaInfo, 0, sizeof(tHalRaGlobalInfo));
    /*
    pThisBss = ((tpBssStruct) pMac->hal.halMac.bssTable)+bssIdx;
    halMacRaBssInfoToFW(pMac, &pThisBss->bssRaInfo, (tANI_U8)bssIdx);
    */
    /* send message to firmware */
    msgBody.bssIdx = bssIdx;
    msgBody.selfStaIdx = selfStaIdx;
    // msgBody.bssIdx = (tANI_U8)bssIdx;

    // Send the RA start request to firmware
    status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_ADD_BSS, sizeof(Qwlanfw_RaAddBssMsgType), (tANI_U8 *)&msgBody);
    return status;

}

eHalStatus
halMacRaDelBssReq(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    Qwlanfw_RaDelBssMsgType msgBody;
    eHalStatus status = eHAL_STATUS_FAILURE;

    /* send message to firmware */
    msgBody.bssIdx = bssIdx;
    // msgBody.bssIdx = (tANI_U8)bssIdx;

    // Send the RA start request to firmware
    status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_DEL_BSS, sizeof(Qwlanfw_RaDelBssMsgType), (tANI_U8 *)&msgBody);
    return status;

}

eHalStatus
halMacRaAddStaReq(tpAniSirGlobal pMac, tANI_U32 staid, tANI_U8 staType)
{
   Qwlanfw_RaAddStaMsgType msgBody;
   eHalStatus status = eHAL_STATUS_FAILURE;
   
   halMacRaStaAdd(pMac, (tANI_U8) staid, staType);

   /* send message to firmware */
   msgBody.staIdx = staid;
// msgBody.staIdx = (tANI_U8)staid;
   
   // Send the RA start request to firmware
   status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_ADD_STA, sizeof(Qwlanfw_RaAddStaMsgType), (tANI_U8 *)&msgBody);
   return status;
}

eHalStatus
halMacRaDelStaReq(tpAniSirGlobal pMac, tANI_U32 staid)
{
   Qwlanfw_RaDelStaMsgType msgBody;
   eHalStatus status = eHAL_STATUS_FAILURE;
   
   halMacRaStaDel(pMac, (tANI_U8) staid);
   
   /* send message to firmware */
   msgBody.staIdx = (tANI_U8)staid;
   
   // Send the RA start request to firmware
   status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_DEL_STA, sizeof(Qwlanfw_RaDelStaMsgType), (tANI_U8 *)&msgBody);
   return status;
}
#endif

eHalStatus
halMacRaUpdateParamReq(tpAniSirGlobal pMac, tANI_U32 bmCode, tANI_U32 paramSpecific)
{
   Qwlanfw_RaUpdateParamMsgType msgBody; 
   eHalStatus status = eHAL_STATUS_FAILURE;
   
   /* send message to firmware */
   msgBody.bmCode = bmCode;
   msgBody.paramSpecific = paramSpecific;
   
   // Send the RA start request to firmware
   status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_PARAM, sizeof(Qwlanfw_RaUpdateParamMsgType), (tANI_U8 *)&msgBody);
   return status;
}
/* --------------------------------------------------------------------------- */
/**
 * raLog
 *
 * FUNCTION:
 * Debug logs for rate adaptation
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

static void raLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) 
{
#ifdef WLAN_DEBUG
    va_list marker;
    
    if(loglevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( SIR_HAL_MODULE_ID )])
        return;

    va_start( marker, pString );     /* Initialize variable arguments. */

    logDebug(pMac, SIR_HAL_MODULE_ID, loglevel, pString, marker);

    va_end( marker );              /* Reset variable arguments.      */
#endif
}


static tHalMacRate
_getLowestRateByNwType( eRfBandMode band, tStaRateMode operMode, tANI_U32 pure11g)
{
    tHalMacRate halRate = HALRATE_INVALID;

    switch(operMode)
    {
        case eSTA_11a:
            halRate = HALRATE_6;
            break;
        case eSTA_11bg:
        case eSTA_11n:
            if (band == eRF_BAND_5_GHZ || (pure11g)) {
                halRate = HALRATE_6;
            } else {
                halRate = HALRATE_1;
            }
            break;
        case eSTA_11b:
        default:
            halRate = HALRATE_1;
            break;
    }

    return halRate;
}

//
// _updateStaValidRateBitmap: Set valid rate mask used while sending frames to peer STA.
//  The bitmask created here is based on rates which both peer STA and self STA support.
//  This function relies on the following parameters be configured in STA descriptor
//    - opRateMode, cbMode, mimoMode, shortGI, htEnabled
//  Also requires following parameters in the BSS descriptor:
//    - shortPreamble
//
static void
_updateStaValidRateBitmap(tpAniSirGlobal  pMac, tpStaStruct   pSta,  tANI_U32 selfStaIdx)
{
    tpHalRaInfo   pRaInfo;
    tpBssStruct   pBss = NULL;
    tANI_U32 *vRates;
    tANI_U8 rIndex;

    /* Step 1: Construct rate bitmap for peer STA */

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    //reset the valid rate
    vRates = (tANI_U32 *)&pRaInfo->validRates[0];
    for (rIndex = 0; rIndex < HAL_NUM_U32_MAP_RATES; rIndex++)
        vRates[rIndex] = 0;

    /* if sta is non 11b, enable 11a rates.
     * if CB is supported, also enable 11a duplicate rates */
    if(pRaInfo->opRateMode != eSTA_11b)
    {
        SET_11A_RATES(vRates);
        if(pRaInfo->cbMode)
            SET_DUP_11A_RATES(vRates);
    }

    //check nwType, if 11a then don't enable 11b rates
    //otherwise always enable 11b rates

    //FIXME: for pure 11g, should not enable 11b rates!
    if(pMac->hal.currentRfBand != eRF_BAND_5_GHZ){

        /* not in 5Ghz, always enable 11b rates */
        SET_11B_RATES(vRates);

        /* if BSS allows short preamble and peer STA also supports short preamble then
         * add short preambles (+CB if supported) rates
         */
        if(HAL_IS_VALID_BSS_INDEX(pMac, pSta->bssIdx)){
            pBss = (tpBssStruct) pMac->hal.halMac.bssTable;
            pBss = &pBss[pSta->bssIdx];
        }
        if(pBss && pBss->bssRaInfo.u.bit.fShortPreamble && pRaInfo->shortPreamble){
            SET_SPREAM_11B_RATES(vRates);
            if(pRaInfo->cbMode == 0){
                CLEAR_DUP_ALL_11B_RATES(vRates);
            }else{
                SET_DUP_ALL_11B_RATES(vRates);
            }
        }else{
            /* short preamble rates not supported*/
            if(pRaInfo->cbMode == 0){
                CLEAR_DUP_ALL_11B_RATES(vRates);
            }else{
                /* although almost impossible that a STA doesn't support short preamble rates
                 * but support CB, still handle this case here
                 */
                SET_DUP_ALL_11B_RATES(vRates);
            }
            //clear short preamble rates. Must be here after dup rates are set
            CLEAR_SPREAM_11B_RATES(vRates);
        }
    }

    if(pRaInfo->opRateMode == eSTA_11n) {

        //Enable HT MCS rates
        if(pSta->htEnabled){

            //enable all MCS 0-7 rates
            SET_HT_SIMO_ALLGI_RATES(vRates);

#ifdef FEATURE_TX_PWR_CONTROL
            //enable the corresponding bits for virtual rates
           HALRATE_SETBIT(vRates, HALRATE_QUALCOMM_VIRTUAL_RATE_START, HALRATE_QUALCOMM_VIRTUAL_RATE_END);
#endif

            //enable all MCS 8-15 rates
            SET_HT_MIMO_ALLGI_RATES(vRates);

            //enable all MCS 32 rates
            SET_HT_MCS32_ALLGI_RATES(vRates);

            //disble MIMO rates if not supported
            if(!pRaInfo->mimoMode)
                CLEAR_HT_MIMO_ALLGI_RATES(vRates);

            //disable CB rate if not supported
            if(pRaInfo->cbMode == 0 )
                CLEAR_HT_ALL_CB_RATES(vRates);

            //if SGI not supported, disable all SGI rates
            if(pRaInfo->shortGI20 == 0 ){
                CLEAR_HT_ALL_SHORTGI20_RATES(vRates);
            }
            if(pRaInfo->cbMode == 0 || pRaInfo->shortGI40 == 0 ){
                CLEAR_HT_ALL_SHORTGI40_RATES(vRates);
                CLEAR_HT_MCS32_SGI_RATES(vRates);
            }

        }

    } else {
        //11b and 11bg

    }

    /* Step 2: Peer STA's valid rate bitmap creation done.
     * If self STA index not yet set, then leave the vRates as is, no more filtering.
     * otherwise,  filter the valid rates by local STA's supported rates and valid rate mask
     */
    if(HAL_STA_INVALID_IDX != selfStaIdx){
        tpStaStruct   pSelfSta = (tpStaStruct) pMac->hal.halMac.staTable;
        tpHalRaInfo   pSelfRaInfo;
        tANI_U32 *selfValidRates;
        tANI_U32 *selfSuppRates;

        tANI_U32 i;
        pSelfSta = &pSelfSta[selfStaIdx];
        pSelfRaInfo = HAL_RAINFO_PTR_GET(pSelfSta);
        selfValidRates = (tANI_U32 *)&pSelfRaInfo->validRates[0];
        selfSuppRates = (tANI_U32 *)&pSelfRaInfo->supportedRates[0];
        for(i=0; i<HAL_NUM_U32_MAP_RATES; i++){
            vRates[i] &= selfValidRates[i]; /* filter with Self valid rate mask which comes from self oprateMode */
            vRates[i] &= selfSuppRates[i];  /* filter with self supported rate mask. */
        }
    }

    raLog(pMac, RALOG_STATS, FL("RA[STA %d] CB%d, MIMO%d, SGI20:%d, SGI40:%d shrtPreamble: %d rateMode %d"),
          pSta->staId, pRaInfo->cbMode,  pRaInfo->mimoMode, pRaInfo->shortGI20,
          pRaInfo->shortGI40,pRaInfo->shortPreamble, pRaInfo->opRateMode);
    raLog(pMac, RALOG_STATS,
          FL("RA[STA%d] ValidRates = %08x %08x"),
          pSta->staId, pRaInfo->validRates[0],
          pRaInfo->validRates[1]
          );

    return;
}

static tSirRetStatus _fixedRateCfgGet( tpAniSirGlobal  pMac,  tHalMacRate* fixedHalRate)
{
    tANI_U32 userCfgFixedRate;
    tTpeRateIdx tpeRate;
    if(wlan_cfgGetInt(pMac, WNI_CFG_FIXED_RATE, &userCfgFixedRate) != eSIR_SUCCESS) {
        raLog(pMac, RALOG_ERROR, FL("FIXED_RATE CFG get failed"));
        return eSIR_FAILURE;
    }
    tpeRate = halRate_cfgFixedRate2TpeRate(userCfgFixedRate);
    *fixedHalRate = halRate_tpeRate2HalRate(tpeRate);

    raLog ( pMac, RALOG_STATS, 
            FL("Band:%d nwType:%d, CfgFixedRates:%d = TPE Rate %d = HAL rate %d\n"),
            pMac->hal.currentRfBand,
            pMac->hal.nwType, userCfgFixedRate, tpeRate, *fixedHalRate);

    return eSIR_SUCCESS;
}

tSirRetStatus
halMacRaInitStaRate(
    tpAniSirGlobal  pMac,
    tpStaStruct   pSta)
{
    tHalMacRate fixedRate;
    tpHalRaInfo   pRaInfo;
#ifdef HAL_SELF_STA_PER_BSS
    tANI_U8 selfStaIdx;
#endif    
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    /* first get the fixed rate config */
    if(_fixedRateCfgGet(pMac, &fixedRate) != eSIR_SUCCESS) {
        raLog(pMac, RALOG_ERROR, FL("Unable to get fixed rate cfg's\n"));
        fixedRate = HALRATE_INVALID;
    }

    // If fixed rate is 11B rate, check if short preamble is supported. 
    // If supported switch to its corresponding short preamble rate.
    if (fixedRate != HALRATE_INVALID) {
        if (HALRATE_IS_11B(fixedRate)) {
	        if (pRaInfo->shortPreamble) {
		        fixedRate = _convert2ShortPreambleRate(fixedRate);
	        }
        }
    }

    /* always recreate the valid rate bitmap for this sta based on its 
     * capability, BSS parameters and local STA's capability*/
#ifdef HAL_SELF_STA_PER_BSS
    halTable_GetBssSelfStaIdxForBss(pMac, pSta->bssIdx, &selfStaIdx);
    _updateStaValidRateBitmap(pMac, pSta, selfStaIdx);
#else
    _updateStaValidRateBitmap(pMac, pSta, pMac->hal.halMac.selfStaId);
#endif
    //if peer does not support the fixed rate, let RA run on this STA.
    if( (fixedRate != HALRATE_INVALID) &&
            (eSIR_SUCCESS !=  _raRateIsSupportedAndValid( pMac, fixedRate, 
                                                          (tANI_U32 *)&pRaInfo->supportedRates[0], 
                                                          (tANI_U32 *)&pRaInfo->validRates[0],
                                                          pRaInfo->maxDataRate)) )
    {
        raLog(pMac, RALOG_ERROR, FL("Peer does not support the fixed rate: S%d, so changing the mode to AUTO\n"),halRate_halRate2TpeRate(fixedRate));
        fixedRate = HALRATE_INVALID;
    }

    if(fixedRate == HALRATE_INVALID){
        //no fixed rate, start auto rate adaptation on this STA
        _raStartStaAutoRate(pMac, pRaInfo, pSta->staId);

    }else{

        //for fixed rate STAs, use primary rate as secondary/teritary retry rates
        _raSetStaFixedRate(pMac, pRaInfo, pSta->staId, fixedRate);
    }
    return eSIR_SUCCESS;

}

void
halSetFixedRateForAllStaByCfg(tpAniSirGlobal pMac)
{
    tANI_U8 staid;
    tHalMacRate fixedRate;

    /* From CFG, get all the fixed rates for different modes, this is the MAC encoded rate */
    if(_fixedRateCfgGet(pMac, &fixedRate) != eSIR_SUCCESS)  {
        raLog(pMac, RALOG_ERROR, FL("Could not get valid set of fixed rates"));
    }

    raLog(pMac, RALOG_INFO, FL("setFixedRate: %d\n"),fixedRate);

    /*
     * For all STAs, based on peer type, select appropriate fixed rate
     */
    for (staid = 0; staid < pMac->hal.halMac.maxSta; staid++) {

        tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
        if(RA_STA_IS_VALID(pSta)) {

            tpHalRaInfo pRaInfo;

            pRaInfo = HAL_RAINFO_PTR_GET(pSta);

            /* for each STA, select appropriate rate among given
             * fixed rates by its peer type
             */

            // If fixed rate is 11B rate, check if short preamble is supported. 
            // If supported switch to its corresponding short preamble rate.
            if (fixedRate != HALRATE_INVALID) {
                if (HALRATE_IS_11B(fixedRate)) {
	                if (pRaInfo->shortPreamble) {
		                fixedRate = _convert2ShortPreambleRate(fixedRate);
	                }
                }
            }

            //Set the mode to 'AUTO' if peer does not support the fixed rate.
            if(eSIR_SUCCESS !=  _raRateIsSupportedAndValid(pMac, fixedRate, 
                        (tANI_U32 *)&pRaInfo->supportedRates[0], 
                        (tANI_U32 *)&pRaInfo->validRates[0],
                        pRaInfo->maxDataRate)) {
                raLog( pMac, RALOG_INFO, FL
                        ("STA %d does not support the fixed rate: %d\n"), 
                        staid, fixedRate);
                fixedRate = HALRATE_INVALID;
            }

            if(fixedRate == HALRATE_INVALID) {
                raLog( pMac, RALOG_ERROR, 
                        FL("RA[STA%d]: Selected HAL rate is invalid %d, so changing the mode to AUTO"),
                        staid, fixedRate);
                //no fixed rate, restart auto rate adaptation on this STA
                _raStartStaAutoRate(pMac, pRaInfo, pSta->staId);
            } else {
                raLog( pMac, RALOG_STATS, FL("RA[STA%d]: Selected rate %d"),
                        staid, fixedRate);
                _raSetStaFixedRate(pMac, pRaInfo, staid, fixedRate);
            }

        } /* if the sta was valid */
    } /* for each staid */

}

/* Get retry policy and retry rates from CFG,
 * set global retry rates and set those retry rates to ALL STAs
 */
static void
_updateAllStaRetryRateByCfg(
    tpAniSirGlobal  pMac)
{
    tANI_U32            retryPolicy;
    tHalMacRate         sRate, tRate;

    tHalRetryMode   retryMode[WNI_CFG_RETRYRATE_POLICY_MAX] = {
        HAL_RETRY_USE_WHATEVER,
        HAL_RETRY_USE_MINRATE,
        HAL_RETRY_USE_PRIMARY,
        HAL_RETRY_USE_SPECIFIED,
        HAL_RETRY_USE_CLOSEST,
    };

    /** Intialize to Default values as the Configurations are not yet in during init*/
    retryPolicy = WNI_CFG_RETRYRATE_POLICY_AUTOSELECT ;
    sRate = (tHalMacRate)WNI_CFG_RETRYRATE_SECONDARY_STADEF;
    tRate = (tHalMacRate)WNI_CFG_RETRYRATE_TERTIARY_STADEF;

    (void) halMacRaSetAllStaRetryRates(pMac, retryMode[retryPolicy], sRate, tRate);
}


/*
 * From all IE rates given, set corresponding bit in thr supported rate bitmap.
 */
static void
_fillSupportedPreTitanRatesByIeRates(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U16        *pSupportedIeRates,
    tANI_U8         numRates,
    tHalIeRateType       rateType)
{
    tANI_U8  rateIdx;
    tANI_U16 *pIeRate = pSupportedIeRates;
    for (rateIdx = 0; rateIdx < numRates; rateIdx++, pIeRate++)
    {
        tHalMacRate halRate;
        if(rateType == IERATE_11B ){
            if((*pIeRate != 0) &&
                (halConvertIeToMacRate(pMac, *pIeRate, &halRate) == eSIR_SUCCESS)){
                //set supported  11b long preamble rate
                RA_BITVAL_SET((halRate), pRaInfo->supportedRates);

                switch(pRaInfo->cbMode<<1|pRaInfo->shortPreamble){
                    case 0:
                        break;
                    case 1:
                        halRate = _convert2ShortPreambleRate(halRate);
                        //set supported  11b short preamble rate
                        if(halRate < HALRATE_INVALID)
                            RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                        break;
                    case 2:
                        halRate = halMacRaConvert2DupRate(halRate);
                        //set supported  11b duplicate mode rate
                        if(halRate < HALRATE_INVALID)
                            RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                        break;
                    case 3:
                        //set supported  11b short preamble rate
                        RA_BITVAL_SET((_convert2ShortPreambleRate(halRate)), pRaInfo->supportedRates);
                        halRate = halMacRaConvert2DupRate(halRate);
                        if(halRate < HALRATE_INVALID)
                            break;
                        //set supported  11b duplicate mode rate
                        RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                        halRate = _convert2ShortPreambleRate(halRate);
                        //set supported 11b duplicate mode, short preamble rate
                        if(halRate < HALRATE_INVALID)
                           RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                        break;
                }
            }
        }else if(rateType == IERATE_11A){
            if((*pIeRate != 0) &&
                (halConvertIeToMacRate(pMac, *pIeRate, &halRate) == eSIR_SUCCESS)){
                //set supported 11a rate
                RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                if(pRaInfo->cbMode){
                    //set supported 11a duplicate mode rate
                    halRate = halMacRaConvert2DupRate(halRate);
                    if(halRate < HALRATE_INVALID)
                        RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
                }
            }
        }else if(rateType == IERATE_POLARIS){
            if((*pIeRate != 0) &&
                (halConvertIeToMacRate(pMac, *pIeRate, &halRate) == eSIR_SUCCESS)){
                if(halRate < HALRATE_INVALID)
                    RA_BITVAL_SET((halRate), pRaInfo->supportedRates);
            }
        }
    }
}

#ifdef FEATURE_TX_PWR_CONTROL
static void
_fillSupportedHTRatesByMcsBitmap(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8        *pSupportedMcsRates,
    tANI_U8         numBytes,
    tANI_U8         isSelfSta)
#else
static void
_fillSupportedHTRatesByMcsBitmap(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8        *pSupportedMcsRates,
    tANI_U8         numBytes)
#endif
{
    //tHalMacRate halRate;
    tANI_U8 mcsIdx;

#ifdef FEATURE_TX_PWR_CONTROL 
    tANI_U32 txPwrCtrlEnabled;

    if(wlan_cfgGetInt(pMac,WNI_CFG_TX_PWR_CTRL_ENABLE, &txPwrCtrlEnabled) != 
                                                                eSIR_SUCCESS) {
        raLog(pMac, RALOG_ERROR, FL("TX_PWR_ENABLED CFG get failed"));
        return ;
    }
#endif


    for(mcsIdx = 0; mcsIdx < numBytes*8; mcsIdx++){

        if(mcsIdx>=16){
            if(mcsIdx == 32 && (pSupportedMcsRates[4] & 1)/* bit 0 of Word 1 = 32th bit = MCS 32 */){
#ifdef RA_CB_ENABLED
                /* HT SIMO+CB+DUP rates MCS 32, Rxp rate index 200, 201 */
                if(pRaInfo->cbMode){
                    if(pRaInfo->shortGI40){
                        //SIMO+CB+SGI
                        RA_BITVAL_SET(HALRATE_HT_SIMO_CB_DUP_SGI_0067, pRaInfo->supportedRates);
                    }
                    //SIMO+CB
                    RA_BITVAL_SET(HALRATE_HT_SIMO_CB_DUP_0060, pRaInfo->supportedRates);

                }
#endif
                break;
            }
        }else if(mcsIdx<8){
            //MCS 0-7, SIMO rates
            if(pSupportedMcsRates[0] & (1<<mcsIdx) ){
#ifdef RA_CB_ENABLED
                //MCS index supported
                if(pRaInfo->cbMode){
                    if(pRaInfo->shortGI40){
                        //SIMO+CB+SGI
                        RA_BITVAL_SET((HALRATE_HT_SIMO_CB_SGI_START + mcsIdx), pRaInfo->supportedRates);
                    }
                    //SIMO+CB
                    RA_BITVAL_SET((HALRATE_HT_SIMO_CB_START + mcsIdx), pRaInfo->supportedRates);

                }
#endif
                if(pRaInfo->shortGI20){
                    //SIMO+SGI
                    RA_BITVAL_SET((HALRATE_HT_SIMO_SGI_START + mcsIdx), pRaInfo->supportedRates);
#ifdef FEATURE_TX_PWR_CONTROL
                    /*set virtual rate bit for the 72Mbps Rate if SGI and 
                    * TX power control feature is enabled*/
                    if( txPwrCtrlEnabled && mcsIdx == 7)
                        RA_BITVAL_SET((HALRATE_HT_SIMO_SGI_0722_VIRT_TPW1), pRaInfo->supportedRates);
#endif
                }
                //SIMO
                RA_BITVAL_SET((HALRATE_HT_SIMO_START + mcsIdx), pRaInfo->supportedRates);
#ifdef FEATURE_TX_PWR_CONTROL
                /*set the virtual rate bit for the 65Mbps Rate if TX power control feature is enabled
                *and if self STA add the rate directly since it used for 
                *updating the sampling table or If short GI is not supported 
                *then only add the long GI virtual rate to avoid RA moving to 
                *long GI virtual rate before selecting actual short GI rate*/
                if((txPwrCtrlEnabled) && ( isSelfSta || !pRaInfo->shortGI20 ) &&
                                                                  ( mcsIdx == 7 ))
                    RA_BITVAL_SET((HALRATE_HT_SIMO_0650_VIRT_TPW1), pRaInfo->supportedRates);
#endif                
            }
        }else{
            tANI_U32 idx = mcsIdx - 8;

            //MCS 8-15, MIMO rates
            if(pSupportedMcsRates[1] & (1<<idx) ){

#ifdef RA_CB_ENABLED
                //MCS index supported
                if(pRaInfo->cbMode){
                    if(pRaInfo->shortGI40){
                        //MIMO+CB+SGI
                        RA_BITVAL_SET((HALRATE_HT_MIMO_CB_SGI_START +idx), pRaInfo->supportedRates);
                    }
                    //MIMO+CB
                    RA_BITVAL_SET((HALRATE_HT_MIMO_CB_START + idx), pRaInfo->supportedRates);

                }
#endif
#ifdef RA_MIMO_ENABLED
                if(pRaInfo->shortGI20){
                    //MIMO+SGI
                    RA_BITVAL_SET((HALRATE_HT_MIMO_SGI_START + idx), pRaInfo->supportedRates);
                }
                //MIMO
                RA_BITVAL_SET((HALRATE_HT_MIMO_START + idx), pRaInfo->supportedRates);
#endif
            }
        }

    }
}

static void
_raSetStaFixedRate(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8         staId,
    tHalMacRate     rate)
{

    pRaInfo->rateAdaptMode = RATE_ADAPT_FIXED;
    pRaInfo->currentRate = rate;
    pRaInfo->fixedRate   = rate;

   // update shared memory
    if(pRaInfo->valid) {
        halMacRaStaInfoToFW(pMac, pRaInfo, (tANI_U16)staId, 0, sizeof(tHalRaInfo));
        halMacRaUpdateParamReq(pMac,RA_UPDATE_STA_INFO, staId);
    }
}

static void
_raStartStaAutoRate(
    tpAniSirGlobal  pMac,
    tpHalRaInfo     pRaInfo,
    tANI_U8         staId)
{

    tpStaStruct   pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+ staId;
    tpBssStruct bssTable = NULL;  
    tANI_U32 pur11g = true;    
    tHalMacRate selectedPrimaryRate;    
    if(HAL_IS_VALID_BSS_INDEX(pMac, pSta->bssIdx))
    {
        bssTable = ((tpBssStruct) pMac->hal.halMac.bssTable) + pSta->bssIdx;          
    }

    if(bssTable && bssTable->bssRaInfo.u.bit.llbCoexist)
    {
        pur11g = false;
    }
    selectedPrimaryRate = _getLowestRateByNwType(pMac->hal.currentRfBand, pRaInfo->opRateMode, pur11g);

    //non fixed rate, pick lowest rate and force restart sampling
    //Best rate would be selected by RA later
    pRaInfo->currentRate = selectedPrimaryRate;
    pRaInfo->lowestRateByNwType = selectedPrimaryRate;
    pRaInfo->rateAdaptMode = RATE_ADAPT_AUTO;

    // update shared memory
    if(pRaInfo->valid) 
    {
        halMacRaStaInfoToFW(pMac, pRaInfo, (tANI_U16)staId, 0, sizeof(tHalRaInfo));
        halMacRaUpdateParamReq(pMac, RA_UPDATE_STA_INFO, staId);
    }

    return;

}

eHalStatus
halMacRaGetStaTxRate(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tHalMacRate     *curTxRateIdx,
    tANI_U32        *curTxRate100Kbps,
    tHalMacRate     *maxTxRateIdx,
    tANI_U32        *maxTxRate100Kbps,
    tANI_U8         *pPacketType
    )
{

    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    tpHalRaInfo pRaInfo; 
    tTpeRateIdx maxTxTpeRate = TPE_RT_IDX_INVALID;
    if( staid >= pMac->hal.halMac.maxSta )
        return eHAL_STATUS_INVALID_PARAMETER;

    pSta = &pSta[staid];
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    /* it should access shared memory to get pRaInfo->currentRate */
    halGetCurrentRate(pMac, staid);

    if( curTxRateIdx )
        *curTxRateIdx = (tHalMacRate)pRaInfo->currentRate;

    if( curTxRate100Kbps )
        *curTxRate100Kbps = HAL_RA_THRUPUT_GET(pRaInfo->currentRate);

    if(  maxTxRate100Kbps )
        _raGetMaxSupportedValidRate(pMac, pSta, &maxTxTpeRate, maxTxRate100Kbps);

    if( maxTxRateIdx )
        *maxTxRateIdx = halRate_tpeRate2HalRate(maxTxTpeRate);

    return halRate_getPacketTypeFromHalRate( (tHalMacRate)pRaInfo->currentRate, pPacketType );
}

eHalStatus
halMacRaGetStaTxCount(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tANI_U32        *curTxAckPktCount
    )
{

    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    if( staid >= pMac->hal.halMac.maxSta )
        return eHAL_STATUS_INVALID_PARAMETER;

    pSta = &pSta[staid];

    if( curTxAckPktCount )
        *curTxAckPktCount = pSta->txAckPkts;
    return eHAL_STATUS_SUCCESS;
}


eHalStatus
halMacRaUpdateStaTxCount(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tANI_U32        txAckPktCount)
{
    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;

    if( staid >= pMac->hal.halMac.maxSta )
        return eHAL_STATUS_INVALID_PARAMETER;

    pSta = &pSta[staid];
    pSta->txAckPkts += txAckPktCount;

    return eHAL_STATUS_SUCCESS;

}

//read STA's supported and valid rate bitmap, if given mac rate's bit in both
//bitmaps are set, return TRUE otherwise return FAILURE
static tSirRetStatus
_raRateIsSupportedAndValid(
    tpAniSirGlobal  pMac,
    tHalMacRate     rate,
    tANI_U32        *psRates,
    tANI_U32        *pvRates,
    tANI_U16        maxDataRate)
{

    if(rate >=  HALRATE_INVALID) {
        return eSIR_FAILURE;
    }

    if ((RA_RATE_SUPPORTED(rate, psRates)) &&
        (RA_RATE_SUPPORTED(rate, pvRates)) &&
        (HALRATE_IS_DISABLED(rate)==0)     &&
        (HALRATE_IS_BELOW_MAX(rate, maxDataRate))) {
        return eSIR_SUCCESS;
    }

    return eSIR_FAILURE;
}

static void
_raGetMaxSupportedValidRate(
    tpAniSirGlobal  pMac,
    tpStaStruct     pSta,
    tTpeRateIdx    *pMaxRate,
    tANI_U32        *pTput )
{
    tpHalRaInfo 	pRaInfo = HAL_RAINFO_PTR_GET(pSta);
    tANI_U8         rIndex;
    tHalMacRate     halRate = HALRATE_INVALID;
    tTpeRateIdx     maxTpeRateIdx = TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET;
    tANI_U32        maxTput = 0;

    for (rIndex = 0; rIndex < HAL_MAC_MAX_TX_RATES; rIndex++)
    {
        halRate = (tHalMacRate)(rIndex);
        if(RA_RATE_SUPPORTED(halRate , pRaInfo->supportedRates) &&
            RA_RATE_SUPPORTED(halRate, pRaInfo->validRates))
        {
            if( maxTput < HAL_RA_THRUPUT_GET(halRate)){
                maxTput = HAL_RA_THRUPUT_GET(halRate);
                maxTpeRateIdx = HAL_RA_TPERATEIDX_GET(halRate);
            }
        }
    }
    *pMaxRate = maxTpeRateIdx;
    *pTput = maxTput;
    return;
}

#ifdef RA_CB_ENABLED
//if halRate is a 40Mhz rate index, get the corresponding 20Mhz rate index
static tHalMacRate _convert2NonCbRate(tHalMacRate halRate)
{
    if(HALRATE_IS_TYPE( CB , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( CB ,     11A , halRate);
    }else if(HALRATE_IS_TYPE( MIMO_CB , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( MIMO_CB , MIMO , halRate);
    }else if(HALRATE_IS_TYPE( HT_SIMO_CB , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( HT_SIMO_CB , HT_SIMO , halRate);
    }else if(HALRATE_IS_TYPE( HT_SIMO_CB_SGI , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( HT_SIMO_CB_SGI , HT_SIMO_SGI , halRate);
    }else if(HALRATE_IS_TYPE( HT_MIMO_CB , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( HT_MIMO_CB  ,HT_MIMO  , halRate);
    }else if(HALRATE_IS_TYPE( HT_MIMO_CB_SGI , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET( HT_MIMO_CB_SGI ,HT_MIMO_SGI , halRate);
    }else
        return HALRATE_INVALID;
}
#endif

//if halRate is non duplicate mode rate index, get the corresponding duplicate mode rate index
tHalMacRate halMacRaConvert2DupRate(tHalMacRate halRate)
{
#ifdef RA_DUP_ENABLED
    if(HALRATE_IS_TYPE(11A  , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET(11A  ,DUP_11A  , halRate);
    }else if(HALRATE_IS_TYPE(11B  , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET(11B  ,DUP_11B  , halRate);
    }else if(HALRATE_IS_TYPE(SPREAM_11B  , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET(SPREAM_11B  ,SPREAM_DUP_11B  , halRate);
    }
#endif
    return HALRATE_INVALID;
}

//if halRate is 11b long preamble rate index, get the corresponding short preamble rate index
static tHalMacRate _convert2ShortPreambleRate(tHalMacRate halRate)
{
    if(HALRATE_IS_TYPE(11B  , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET(11B  ,SPREAM_11B  , halRate);
#ifdef RA_DUP_ENABLED
    }else if(HALRATE_IS_TYPE(DUP_11B  , halRate)){
        return HALRATE_CONVERT_BY_TYPE_OFFSET(DUP_11B  ,SPREAM_DUP_11B  , halRate);
#endif
    }
    return HALRATE_INVALID;
}

/* --------------------------------------------------------------------------- */
/**
 * dphInitializeRateAdaptState
 *
 * FUNCTION:
 * Initialize rate adaptation state
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

eHalStatus halMacRaStart(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tpHalRaGlobalInfo pGlobInfo = &pMac->hal.halRaInfo;
    tANI_U8 goodLinkPerThresh[RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE]=
        { 10, 10, 9, 9, 8, 8, 7, 7, 6, 6};
    tANI_U32 idx;

    pGlobInfo->rMode                       = WNI_CFG_RETRYRATE_POLICY_AUTOSELECT;
    pGlobInfo->raPerAlgoSelection          = RA_PER_SELECT_HYBRID;
    pGlobInfo->raPeriod                    = RA_RATE_ADAPTATION_PERIOD;
    
    pGlobInfo->txFailExceptionThreshold    = RA_TX_FAIL_EXCEPTION_THRESHOLD;
    pGlobInfo->failThreshold               = RA_TOTAL_FAILURES_THRESHOLD;
    pGlobInfo->consecFailThreshold         = RA_SAMPLE_FAILURES_THRESHOLD;
    pGlobInfo->extraStayIncThreshold       = 2;
    pGlobInfo->perIgnoreThreshold          = RA_PER_IGNORE_THRESHOLD;

    pGlobInfo->perGoodLinkJumpThreshold    = RA_GOODLINK_JUMP_THRESHOLD;
    pGlobInfo->perGoodLinkSampleThreshold  = RA_PER_GOODLINK_SAMPLE_THRESHOLD;
    pGlobInfo->perBadLinkJumpThreshold     = RA_BADLINK_JUMP_THRESHOLD;
    pGlobInfo->perBadLinkJumpRetryRateThreshold = RA_PER_BADLINK_RETRYRATE_SAMPLE_THRESHOLD;

    pGlobInfo->goodLinkPersistencyThresh   = RA_GOODLINK_PERSISTENCY_THRESHOLD;
    pGlobInfo->badLinkPersistencyThresh    = RA_BADLINK_PERSISTENCY_THRESHOLD;

    pGlobInfo->linkIdleSamples             = RA_LINK_IDLE_LIMIT;
    pGlobInfo->minPeriodsPerSample         = RA_SAMPLE_PERIODS_MIN;
    pGlobInfo->minTxPerSample              = RA_SAMPLE_MIN_PKTS;

    pGlobInfo->quickSamplePeriod           = RA_QUICK_SAMPLE_PERIOD;
    pGlobInfo->sampleAllPeriod             = RA_SAMPLE_ALL_PERIOD;

    pGlobInfo->lowerSensSampleRates        = RA_LOWERSENSITIVITY_SAMPLE_RATES;
    pGlobInfo->higherSensSampleRates       = RA_HIGHERTHRUPUT_SAMPLE_RATES;

    pGlobInfo->retry1SensitivityDiff       = RA_RETRY1_SENSITIVITY_DIFF;
    pGlobInfo->retry2SensitivityDiff       = RA_RETRY2_SENSITIVITY_DIFF;

    pGlobInfo->betterRateMaxSensDiff       = RA_NEXTRATE_MAX_SENSITIVITY_DIFF;
    pGlobInfo->lowerRateMinSensDiff        = RA_LOWERTHRUPUT_SENSITIVITY_DIFF;

    pGlobInfo->min11bRateIdx               = HALRATE_INVALID;
    pGlobInfo->min11gRateIdx               = HALRATE_INVALID;
    pGlobInfo->min11nRateIdx               = HALRATE_INVALID;

/* This is needed due to byte swap */
    palFillMemory(pMac->hHdd,&pGlobInfo->goodPerThreshBySensitivity[0],sizeof(pGlobInfo->goodPerThreshBySensitivity),0);
    for(idx = 0 ; idx < RA_GOODPERTHRESH_SENSITIVITY_TABLE_SIZE; idx ++ )
        pGlobInfo->goodPerThreshBySensitivity[idx] = goodLinkPerThresh[idx];

    // Download the table to the target.
    /*  Note: protPolicy and rtsThreshold is not yet valid.
    At this moment, this is deffered to RA_ADD_BSS message */
//    halMacRaGlobalInfoToFW(pMac, pGlobInfo, 0, sizeof(tHalRaGlobalInfo));
    //enable rate adaptation by default
//    if(eHAL_STATUS_SUCCESS != halRateAdaptStart(pMac))
//        return eHAL_STATUS_FAILURE;

    return eHAL_STATUS_SUCCESS;
}


// Exit function to clear all the RA data, here we destroy the timer used for 
// periodic statistic collection
eHalStatus halMacRaStop(tHalHandle hHal, void *arg)
{
//    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
//    tpHalRaGlobalInfo pGlobInfo = &pMac->hal.halRaInfo;
//    tx_timer_delete(&pGlobInfo->raStatsTimer);
    return eHAL_STATUS_SUCCESS;
}


// Function to set the periodicity of collecting and performing the 
// rate adaptation
eHalStatus halMacRaSetRAPeriodicity(tpAniSirGlobal pMac, tANI_U32 raPeriod)
{
    /* send message to firmware */
    Qwlanfw_RaTimerMsgType msgBody; 
    eHalStatus status = eHAL_STATUS_FAILURE;

    /* RA_TODO: raPeriod validity check required 
    if(raPeriod < minimum || raPeriod > maximum)
       return;
    */   

    msgBody.raPeriod = raPeriod;
    msgBody.raTimerCntrl = RA_UPDATE_TIMER_PERIOD;

    // Send the RA start request to firmware
    status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_TIMER, sizeof(Qwlanfw_RaTimerMsgType), (tANI_U8 *)&msgBody);
    return status;
}


/* --------------------------------------------------------------------------- */
/**
 * dphMacRate2Index
 *
 * FUNCTION:
 * Given a MAC rate encoding, return the corresponding DPH rate index
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param halRate MAC rate encoding
 * @return DPH rate index
 */

tANI_U32
dphMacRate2Index(tpAniSirGlobal pMac, tANI_U32 halRate)
{
    switch (halRate)
    {
        case SIR_MAC_RATE_1:
            return DPH_PHY_RATE_1_INDEX;
        case SIR_MAC_RATE_2:
            return DPH_PHY_RATE_2_INDEX;
        case SIR_MAC_RATE_5_5:
            return DPH_PHY_RATE_5_5_INDEX;
        case SIR_MAC_RATE_11:
            return DPH_PHY_RATE_11_INDEX;
        case SIR_MAC_RATE_6:
            return DPH_PHY_RATE_6_INDEX;
        case SIR_MAC_RATE_9:
            return DPH_PHY_RATE_9_INDEX;
        case SIR_MAC_RATE_12:
            return DPH_PHY_RATE_12_INDEX;
        case SIR_MAC_RATE_18:
            return DPH_PHY_RATE_18_INDEX;
        case SIR_MAC_RATE_24:
            return DPH_PHY_RATE_24_INDEX;
        case SIR_MAC_RATE_36:
            return DPH_PHY_RATE_36_INDEX;
        case SIR_MAC_RATE_48:
            return DPH_PHY_RATE_48_INDEX;
        case SIR_MAC_RATE_54:
            return DPH_PHY_RATE_54_INDEX;
        case SIR_MAC_RATE_72:
            return DPH_PHY_RATE_72_INDEX;
        case SIR_MAC_RATE_96:
            return DPH_PHY_RATE_96_INDEX;
        case SIR_MAC_RATE_108:
            return DPH_PHY_RATE_108_INDEX;
        case SIR_MAC_RATE_144:
            return DPH_PHY_RATE_144_INDEX;
        default:
            raLog(pMac, RALOG_ERROR, FL("Invalid mac rate 0x%x"), halRate);
            return DPH_PHY_RATE_1_INDEX;
    }
}

/* --------------------------------------------------------------------------- */
/**
 * sirMacRate2PhyRate
 *
 * FUNCTION:
 * Given a MAC rate encoding, return the corresponding PHY rate encoding
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param halRate MAC rate encoding
 * @param erpEnabled 11g enabled
 * @param pPhyRate pointer to rate (PHY encoding) returned
 * @param pPhyMode pointer to mode (PHY encoding) returned
 * @return None
 */

void
sirMacRate2PhyRate(
    tpAniSirGlobal  pMac,
    tANI_U32             halRate,
    tANI_U8              erpEnabled,
    tANI_U8              shortPreamble,
    tANI_U32            *pPhyRate,
    tANI_U32            *pPhyMode)
{
    shortPreamble = (shortPreamble && pMac->lim.gLimShortPreamble);

    switch (halRate)
    {
        case SIR_MAC_RATE_1:
            *pPhyRate = HAL_PHY_RATE_1;
            *pPhyMode = (shortPreamble ? HAL_PHY_MODE_11B_SHORT :
                         HAL_PHY_MODE_11B_LONG);
            break;

        case SIR_MAC_RATE_2:
            *pPhyRate = HAL_PHY_RATE_2;
            *pPhyMode = (shortPreamble ? HAL_PHY_MODE_11B_SHORT :
                         HAL_PHY_MODE_11B_LONG);
            break;

        case SIR_MAC_RATE_5_5:
            *pPhyRate = HAL_PHY_RATE_5_5;
            *pPhyMode = (shortPreamble ? HAL_PHY_MODE_11B_SHORT :
                         HAL_PHY_MODE_11B_LONG);
            break;

        case SIR_MAC_RATE_11:
            *pPhyRate = HAL_PHY_RATE_11;
            *pPhyMode = (shortPreamble ? HAL_PHY_MODE_11B_SHORT :
                         HAL_PHY_MODE_11B_LONG);
            break;

        case SIR_MAC_RATE_6:
            *pPhyRate = HAL_PHY_RATE_6;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_9:
            *pPhyRate = HAL_PHY_RATE_9;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_12:
            *pPhyRate = HAL_PHY_RATE_12;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_18:
            *pPhyRate = HAL_PHY_RATE_18;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_24:
            *pPhyRate = HAL_PHY_RATE_24;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_36:
            *pPhyRate = HAL_PHY_RATE_36;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_48:
            *pPhyRate = HAL_PHY_RATE_48;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_54:
            *pPhyRate = HAL_PHY_RATE_54;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G : HAL_PHY_MODE_11A);
            break;

        case SIR_MAC_RATE_72:
            *pPhyRate = HAL_PHY_RATE_MIMO_72;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G_MIMO :
                         HAL_PHY_MODE_11A_MIMO);
            break;

        case SIR_MAC_RATE_96:
            *pPhyRate = HAL_PHY_RATE_MIMO_96;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G_MIMO :
                         HAL_PHY_MODE_11A_MIMO);
            break;

        case SIR_MAC_RATE_108:
            *pPhyRate = HAL_PHY_RATE_MIMO_108;
            *pPhyMode = (erpEnabled ? HAL_PHY_MODE_11G_MIMO :
                         HAL_PHY_MODE_11A_MIMO);
            break;

        default:
            logDbg(pMac, SIR_HAL_MODULE_ID, LOGP, FL("Invalid MAC rate encoding %d"), halRate);
    }
}


/* convert a given encoded rate (from the IE's) to an enum'd mac rate */
tSirRetStatus
halConvertIeToMacRate(
    tpAniSirGlobal  pMac,
    tANI_U16        ieRate,
    tpHalMacRate    pMacRate)
{
    tHalMacRate rateIdx;

    if(ieRate==0)
        return eSIR_FAILURE;

    for (rateIdx = HALRATE_MODE_START; rateIdx < (tHalMacRate)HAL_NUM_MACRATE_2_IERATE_ENTRIES; rateIdx++)
    {
        if((ieRate & IERATE_RATE_MASK) == HAL_RA_IERATEMCSIDX_GET(rateIdx))
        {
            *pMacRate = (rateIdx);
            return eSIR_SUCCESS;
        }
    }
    return eSIR_FAILURE;
}

/* creates the supported rates bitmap for RA use */
tSirRetStatus
halMacRaUpdateStaSuppRateBitmap_PeerType(
    tpAniSirGlobal      pMac,
    tANI_U16            staid,
    tpSirSupportedRates pRates)
{
    tpHalRaInfo   pRaInfo;
    tSirRetStatus retval;
    tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
#ifdef FEATURE_TX_PWR_CONTROL
    tANI_U8     isSelfSta = (staid == pMac->hal.halMac.selfStaId)?1:0;
#endif

    if((pSta == NULL) || (pRates == NULL))
    {
        raLog(pMac, RALOG_ERROR, FL("invalid sta/rate info"));
        return eSIR_FAILURE;
    }

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    retval = eSIR_SUCCESS;

    raLog(pMac, RALOG_INFO, FL("PeerType[%d]: peer %d"),
          staid, pRates->opRateMode);

    switch (pRaInfo->opRateMode)
    {
        case eSTA_11bg:
            _fillSupportedPreTitanRatesByIeRates(pMac, pRaInfo, &pRates->llaRates[0], (tANI_U8) HAL_NUM_11A_RATES, IERATE_11A);
            /* fall through */

        case eSTA_11b:
            if(pMac->hal.currentRfBand == eRF_BAND_5_GHZ)
                break;
            _fillSupportedPreTitanRatesByIeRates(pMac, pRaInfo, &pRates->llbRates[0], (tANI_U8) HAL_NUM_11B_RATES, IERATE_11B);
            break;

        case eSTA_11a:
            _fillSupportedPreTitanRatesByIeRates(pMac, pRaInfo, &pRates->llaRates[0], (tANI_U8) HAL_NUM_11A_RATES, IERATE_11A);
            break;

        case eSTA_11n:
#ifdef FEATURE_TX_PWR_CONTROL
            _fillSupportedHTRatesByMcsBitmap(pMac, pRaInfo, &pRates->supportedMCSSet[0], 
                                    sizeof(pRates->supportedMCSSet), isSelfSta);
#else
            _fillSupportedHTRatesByMcsBitmap(pMac, pRaInfo, &pRates->supportedMCSSet[0], 
                                       sizeof(pRates->supportedMCSSet));
#endif
            /*TODO: Here assume 11n is operating in 2.4Ghz and set 11b rates in supported rate bitmap.
              if band is 5Ghz, 11b rates would be filtered later when creating valid rate bitmap*/
            if(pMac->hal.currentRfBand != eRF_BAND_5_GHZ)
                _fillSupportedPreTitanRatesByIeRates(pMac, pRaInfo, &pRates->llbRates[0], (tANI_U8) HAL_NUM_11B_RATES, IERATE_11B);
            _fillSupportedPreTitanRatesByIeRates(pMac, pRaInfo, &pRates->llaRates[0], (tANI_U8) HAL_NUM_11A_RATES, IERATE_11A);
            break;

        default:
            raLog(pMac, RALOG_ERROR, FL("PeerType[%d]: invalid opRateMode %d"),
                  staid, pRates->opRateMode);
            retval = eSIR_FAILURE;
            break;
    }

    raLog(pMac, RALOG_STATS,
          FL("STA %2d, opRateMode=%d: SupportedRates = "
             "%08x %08x"),
          staid, pRates->opRateMode, pRaInfo->supportedRates[0],
          pRaInfo->supportedRates[1]
          );

    return retval;
}

tSirRetStatus halMacRaStaAdd(
    tpAniSirGlobal      pMac,
    tANI_U16            staid,
    tANI_U8             staType)
{
    tpStaStruct         pSta = ((tpStaStruct) pMac->hal.halMac.staTable);
    tpHalRaInfo         pRaInfo;

    if(pSta == NULL || (pMac->hal.memMap.maxStations <= staid))
    {
        raLog(pMac, RALOG_ERROR, FL("halMacRaStaAdd: invalid sta[%d]!"), staid);
        return eSIR_HAL_STA_DOES_NOT_EXIST;
    }
    pSta = &pSta[staid];
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    /* this will effectively pRaInfo->valid  */
    pRaInfo->valid = 1;
    pRaInfo->staType = staType;
    // update shared memory
    halMacRaStaInfoToFW(pMac, pRaInfo, staid, 0, sizeof(tHalRaInfo));

    return eSIR_SUCCESS;
}

tSirRetStatus halMacRaStaDel(
    tpAniSirGlobal      pMac,
    tANI_U16            staid)
{
    tpStaStruct         pSta = ((tpStaStruct) pMac->hal.halMac.staTable);
    tpHalRaInfo         pRaInfo;

    if(pSta == NULL || (pMac->hal.memMap.maxStations <= staid))
    {
        raLog(pMac, RALOG_ERROR, FL("halMacRaStaDel: invalid sta[%d]!"), staid);
        return eSIR_HAL_STA_DOES_NOT_EXIST;
    }
    pSta = &pSta[staid];
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    /* this will effectively pRaInfo->valid & prevStatsCache to zero */
    vos_mem_zero((void *)pRaInfo, sizeof(tpHalRaInfo));
    // update shared memory
    halMacRaStaInfoToFW(pMac, pRaInfo, staid, 0, sizeof(tHalRaInfo));

    return eSIR_SUCCESS;
}

/* initialze RA data for a new sta */
tSirRetStatus
halMacRaStaInit(
    tpAniSirGlobal      pMac,
    tANI_U16            staid,
    tpSirSupportedRates pRates,
    tpHalRaModeCfg      pMode)
{
    tpStaStruct         pSta = ((tpStaStruct) pMac->hal.halMac.staTable);
    tpBssStruct         pBss = ((tpBssStruct) pMac->hal.halMac.bssTable);
    tpHalRaInfo         pRaInfo;

    /*
     * if the hash node is available, go ahead and initialize even if it is not valid
     * this function may be called before the valid bit is set
     */
    if(pSta == NULL || (pMac->hal.memMap.maxStations <= staid))
    {
        raLog(pMac, RALOG_ERROR, FL("invalid sta[%d]!"), staid);
        return eSIR_SUCCESS;
    }
    if(pBss == NULL)
        return eSIR_FAILURE;
    pSta = &pSta[staid];
    if(HAL_IS_VALID_BSS_INDEX(pMac, pSta->bssIdx))
        pBss = &pBss[pSta->bssIdx];

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);
    pRaInfo->opRateMode = pRates->opRateMode;
    pRaInfo->bssIdx     = pSta->bssIdx;

    /* populate the station specific capability information */
    pRaInfo->cbMode   = pMode->channelBondingEnable;
    pRaInfo->mimoMode = pMode->mimoEnable;
    pRaInfo->shortGI20 = pMode->sgiEnable;
    pRaInfo->shortGI40 = pMode->sgiEnable40;
    pRaInfo->shortPreamble = pMode->shortPreambleEnable;
    pRaInfo->maxDataRate = pRates->rxHighestDataRate;
/* note for firmware RA 
 pRaInfo->gfEnabled was called in separately in HalMsg_AddSta(). So that's why it deosn't show up here
 if required, gfEnabled can be part of tpHalRaModeCfg, and set here.
 pRaInfo->maxAmpduDensity was called in separately in halMsg_AddSta() (by halMsg_addStaUpdateTPE()).
 if required, maxAmpduDensity can be part of tpHalRaModeCfg, and set here 
*/

    /*
     * copy the retry rate settings to local sta info
     * we will always use the local sta settings to figure out the retry rates
     * this allows us to set the retry rates either globally (thru CFG's) or
     * locally through other means (such as dump commands)
     */
    /* initially set to lowest rate, may be overriden later if fixed rate is configured */
    pRaInfo->currentRate = _getLowestRateByNwType(pMac->hal.currentRfBand, pRaInfo->opRateMode, (!pBss->bssRaInfo.u.bit.llbCoexist));

    raLog(pMac, RALOG_STATS, FL("RA[STA%d]  opRateMode %d"), staid, pRates->opRateMode);

    /* fill the supported rate bitmap and update peer type based on supported rates */
    (void) halMacRaUpdateStaSuppRateBitmap_PeerType(pMac, staid, pRates);

    /* update the valid rate bitmap for this sta based on its capability, BSS parameters and local STA's capability*/
    /* in halMacRaInitStaRate, it will be done */
    /* _updateStaValidRateBitmap(pMac, pSta, pMac->hal.halMac.selfStaId); */

    /* update the station rates based on the mode and rates */
    return (halMacRaInitStaRate(pMac, pSta));

}


tHalMacRate
halGetCurrentRate(
    tpAniSirGlobal pMac,
    tANI_U16       staid)
{

    tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
    tpHalRaInfo pRaInfo;
    if(RA_STA_IS_NOT_VALID(pSta))
        return ((tHalMacRate) HALRATE_INVALID);
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    halMacRaStaInfoFromFW(pMac, pRaInfo, staid, offsetof(tHalRaInfo, currentRate), sizeof(pRaInfo->currentRate));
    return ((tHalMacRate)pRaInfo->currentRate);
}

/** -------------------------------------------------------------
\fn halProcessForcePolicyProtCfgUpdate
\brief handler for force protection policy cfg update. 
\       traverses through the HAL sta table and initializes the rate table for each 
\       valid station entry.
\param   tpAniSirGlobal pMac
\return none
  -------------------------------------------------------------*/
static
void halProcessForcePolicyProtCfgUpdate(tpAniSirGlobal pMac)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    int i;

    for(i = 0; i < pMac->hal.halMac.maxSta; i++, pSta++)
    {
        if(RA_STA_IS_VALID(pSta))
            halMacRaInitStaRate(pMac, pSta);
    }
}

void halMacRaCfgChange( tpAniSirGlobal  pMac, tANI_U32 cfgId)
{
    raLog(pMac, RALOG_STATS, FL("cfg change (%d)"), cfgId);

    switch (cfgId)
    {
        case WNI_CFG_FIXED_RATE:
            halSetFixedRateForAllStaByCfg(pMac);
            break;
        case WNI_CFG_RETRYRATE_SECONDARY:
        case WNI_CFG_RETRYRATE_POLICY:
            _updateAllStaRetryRateByCfg(pMac);
            break;
        case WNI_CFG_FORCE_POLICY_PROTECTION:
            halProcessForcePolicyProtCfgUpdate(pMac);
            break;
        default:
            raLog(pMac, RALOG_ERROR, FL("Unsupported CFG %d"), cfgId);
            break;
    }
}

tSirRetStatus
halMacRaStaCapModeUpdate(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tpHalRaModeCfg  pMode)
{
    tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
    tpHalRaInfo     pRaInfo;

    if(RA_STA_IS_NOT_VALID(pSta))
        return eSIR_FAILURE;

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    /* update the mode information for this sta */
    pRaInfo->cbMode   = pMode->channelBondingEnable;
    pRaInfo->mimoMode = pMode->mimoEnable;
    pRaInfo->shortGI20 = pMode->sgiEnable;
    pRaInfo->shortGI40 = pMode->sgiEnable40;
    pRaInfo->shortPreamble = pMode->shortPreambleEnable;

    raLog(pMac, LOG2, FL("HAL-RA[%d]:  cb %d,  mimo %d sgi20 %d sgi40 %d spream %d"),
            staid, pMode->channelBondingEnable,
            pMode->mimoEnable,
            pMode->sgiEnable,
            pMode->sgiEnable40,
            pMode->shortPreambleEnable
         );
    return (halMacRaInitStaRate(pMac, pSta));
}

tSirRetStatus
halMacRaStaModeGet(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tpHalRaModeCfg  pMode,
    tRateAdaptMode  *pRateAdaptMode)
{
    tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
    tpHalRaInfo     pRaInfo;

    if(RA_STA_IS_NOT_VALID(pSta))
        return eSIR_FAILURE;

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    pMode->channelBondingEnable =   pRaInfo->cbMode;
    pMode->mimoEnable = pRaInfo->mimoMode;
    pMode->sgiEnable = pRaInfo->shortGI20;
    pMode->sgiEnable40 = pRaInfo->shortGI40;
    pMode->shortPreambleEnable = pRaInfo->shortPreamble;
    *pRateAdaptMode = (tRateAdaptMode)pRaInfo->rateAdaptMode;

    return eSIR_SUCCESS;
}


tSirRetStatus
halMacRaSetStaRetryMode(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tHalRetryMode   rMode, /* retry mode */
    tHalMacRate     sRate, /* specific rate to use (depends on retry mode) */
    tHalMacRate     tRate) /* tertiary retry rate */
{

    tpStaStruct     pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
    tpHalRaInfo     pRaInfo;

    if(RA_STA_IS_NOT_VALID(pSta))
        return eSIR_FAILURE;

    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    if(pRaInfo->valid)
        halMacRaUpdateParamReq(pMac,RA_UPDATE_STA_INFO, staid);

    return eSIR_SUCCESS;
}

tSirRetStatus
halMacRaSetAllStaRetryRates(
    tpAniSirGlobal  pMac,
    tHalRetryMode   rMode,
    tHalMacRate     sRate,  /* secondary rate */
    tHalMacRate     tRate)  /* tertiary rate */
{
    tANI_U8 staid ; //startStaid;
    tANI_U8  staType;
    tpHalRaGlobalInfo   pGlob   = &pMac->hal.halRaInfo;

    /* set global retry rate policy and rates
     * the policy and rate set here won't override stations whose retry policy is HAL_RETRY_USE_WHATEVER
     */
    pGlob->rMode = rMode;
    pGlob->sRate = sRate;
    pGlob->tRate = tRate;

    // update raGlobalInfo in the firmware memory
    halMacRaGlobalInfoToFW(pMac, pGlob, offsetof(tHalRaGlobalInfo, rMode), 
		    sizeof(pGlob->rMode));

    /* for each staid, force set the retry mode, then update STA rate */
    for (staid = 0; staid < pMac->hal.halMac.maxSta; staid++)
    {
        // Not required on selfSta.
        if ((halTable_GetStaType(pMac, staid, &staType) == eHAL_STATUS_SUCCESS) && 
            (staType == STA_ENTRY_SELF)) {
            continue;
        }

        if(eSIR_SUCCESS == halMacRaSetStaRetryMode(pMac, staid, rMode, sRate, tRate)) {
            halMacRaSetStaRetryMode(pMac, staid, rMode, sRate, tRate);
	}
    }
    return eSIR_SUCCESS;
}



eHalStatus halRateAdaptStart(tpAniSirGlobal pMac)
{
   // send a message START_TIMER_REQ to FW
    Qwlanfw_RaTimerMsgType msgBody; 
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    _updateAllStaRetryRateByCfg(pMac); // big question here. it uses always by Config
 
    msgBody.raTimerCntrl = RA_UPDATE_TIMER_START;

    status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_TIMER, sizeof(Qwlanfw_RaTimerMsgType), (tANI_U8 *)&msgBody);
    return status;
}

eHalStatus halRateAdaptStop(tpAniSirGlobal pMac)
{
   // send a message STOP_TIMER_REQ to FW
    Qwlanfw_RaTimerMsgType msgBody; 
    eHalStatus status = eHAL_STATUS_FAILURE;

    msgBody.raTimerCntrl = RA_UPDATE_TIMER_STOP;

    status = halMacRaUpdateReq(pMac, QWLANFW_RA_UPDATE_TIMER, sizeof(Qwlanfw_RaTimerMsgType), (tANI_U8 *)&msgBody);
    return status;
}

void  halMacRaDumpHalRateTable(tpAniSirGlobal pMac)
{
    tHalMacRate halRateIdx;
    tANI_U8 rateStr[9];
    tANI_U8 modulation[][8]= { "BPSK ", "QPSK ", "16QAM", "64QAM"};
    tANI_U8 codeRate[][8]= { "1/2", "2/3", "3/4", "5/6", "7/8", "?", "?", "?"};
    tANI_U32 halRateTxPktCount[MAX_LIBRA_RATE_NUM] = {0, };

    raLog(pMac, RALOG_CLI, "A- 11a/g rate   B-11b rate  D-duplicate mode   G- HT SGI");
    raLog(pMac, RALOG_CLI, "H- HT rate      M-MIMO rate S-ShortPreamble    t- TitanRate ");
    raLog(pMac, RALOG_CLI, "2- 20Mhz        4-40Mhz     X-DISABLED RATE\n");
    raLog(pMac, RALOG_CLI, "HalRate  TpeRate  Thput  Flags          CurTxPpdu# ");

    halMacRaTxPktCountFromFW(pMac, halRateTxPktCount);
    for( halRateIdx = HALRATE_MODE_START; halRateIdx < HALRATE_MODE_END; halRateIdx ++){
        tANI_U32 index = (halRateIdx);
        tANI_U32 tpeRateIdx = HAL_RA_TPERATEIDX_GET(halRateIdx);
        tANI_U32 modulationType = (gHalRateInfo[index].rateProperty & RA_MODULATION_MASK)>>RA_MODULATION_SHIFT;
        tANI_U32 codeRateType = (gHalRateInfo[index].rateProperty & RA_CODERATE_MASK)>>RA_CODERATE_SHIFT;
        rateStr[0] = HALRATE_IS_DISABLED(halRateIdx)?'X':' ';
        rateStr[1] = HALRATE_IS_HT(halRateIdx)?'H':(HALRATE_IS_11B(halRateIdx)?'B':'A');
        rateStr[2] = HALRATE_IS_CB(halRateIdx)?'4':'2';
        rateStr[3] = HALRATE_IS_MIMO(halRateIdx)?'M':' ';
        rateStr[4] = HALRATE_IS_HT_SGI(halRateIdx)?'G':(HALRATE_IS_11B_SHORT_PREAM(halRateIdx)?'S':' ');
        rateStr[5] = HALRATE_IS_DUP(halRateIdx)?'D':' ';
        rateStr[6] = 0;
        rateStr[7] = 0;
        raLog(pMac, RALOG_CLI, "%3d. S%3d(0x%2x) %4dM (Tx %8d) %s %s %s", halRateIdx, tpeRateIdx, tpeRateIdx,
                HAL_RA_THRUPUT_GET(halRateIdx), halRateTxPktCount[halRateIdx], rateStr, modulation[modulationType],codeRate[codeRateType] );
    }

}

void  halMacRaDumpStaRateInfo(tpAniSirGlobal pMac, tANI_U32 startStaIdx, tANI_U32 endStaIdx)
{
    tANI_U32 staIdx;
    tpHalRaInfo pRaInfo;
    tpStaStruct pSta;

    for( staIdx = startStaIdx; staIdx <= endStaIdx; staIdx ++){
        pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staIdx;
        pRaInfo = HAL_RAINFO_PTR_GET(pSta);

        halGetCurrentRate(pMac, (tANI_U16)staIdx);
        raLog(pMac, RALOG_CLI, "STA %3d  CurRate  %3d.%dMbps (HAL%d Tpe%d) %s mode",
                staIdx,HAL_RA_THRUPUT_GET(pRaInfo->currentRate)/10,
                HAL_RA_THRUPUT_GET(pRaInfo->currentRate)%10,
                pRaInfo->currentRate, HAL_RA_TPERATEIDX_GET(pRaInfo->currentRate),
                pRaInfo->rateAdaptMode == RATE_ADAPT_FIXED? "FIXED" :"AUTO"
             );

        raLog(pMac, RALOG_CLI, "        Cap: %s %s %s %s %s\n",
                pRaInfo->cbMode?        "40M":  "20M",
                pRaInfo->mimoMode?      "MIMO": "SIMO",
                pRaInfo->shortGI20?       "SGI20":  "NoSGI20",
                pRaInfo->shortGI40?       "SGI40":  "NoSGI40",
                pRaInfo->shortPreamble? "ShrtPreamble":"NoShrtPreamble "
             );

/*
        raLog(pMac, RALOG_CLI, "        STA cfg: RETRY-MODE: %s, Retry1: %d Retry2: %d\n",
                pRaInfo->rMode  == HAL_RETRY_USE_WHATEVER?  "RA-decide" :
                (pRaInfo->rMode == HAL_RETRY_USE_MINRATE?   "UseMinRate" :
                 (pRaInfo->rMode == HAL_RETRY_USE_PRIMARY?   "UsePrimary" :
                  (pRaInfo->rMode == HAL_RETRY_USE_SPECIFIED? "UseSpecified" :
                   (pRaInfo->rMode == HAL_RETRY_USE_CLOSEST?   "UseClosest" : "???")))),
                pRaInfo->sRate,
                pRaInfo->tRate);
*/
        raLog(pMac, RALOG_CLI, "        Supp  Rate: %08x %08x \n",
                pRaInfo->supportedRates[0],
                pRaInfo->supportedRates[1]
             );

        raLog(pMac, RALOG_CLI, "  AND   Valid Rate: %08x %08x \n",
                pRaInfo->validRates[0],
                pRaInfo->validRates[1]

             );

        raLog(pMac, RALOG_CLI, "  =     Candidate : %08x %08x \n",
                pRaInfo->validRates[0] & pRaInfo->supportedRates[0],
                pRaInfo->validRates[1] & pRaInfo->supportedRates[1]
             );
    }
}

eHalStatus halRateUpdateStaRateInfo(tpAniSirGlobal pMac, tANI_U32 staIdx)
{
    tpStaStruct pSta = ((tpStaStruct) pMac->hal.halMac.staTable);
    tpBssStruct pBss = (tpBssStruct)pMac->hal.halMac.bssTable;

    if((pSta == NULL) || (pBss==NULL) || staIdx >= pMac->hal.memMap.maxStations )
        return eHAL_STATUS_FAILURE;

    pSta = &pSta[staIdx];
 
    if(pSta->valid){
        if(HAL_IS_VALID_BSS_INDEX(pMac, pSta->bssIdx)) {
            pBss = &pBss[pSta->bssIdx];
        }
        halMacRaInitStaRate(pMac, pSta);
        return eHAL_STATUS_SUCCESS;

    }else {
        return eHAL_STATUS_FAILURE;
    }
}

void  halMacRaDumpStaAllSupporetedRates(tpAniSirGlobal pMac, tANI_U16 staidx)
{
    tpHalRaInfo pRaInfo;
    tpStaStruct pSta;
    tHalMacRate halRateIdx, sRate, tRate;
    /* sRate and tRate is determined by firmware */
    pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staidx;
    if(pSta->valid) {
        pRaInfo = HAL_RAINFO_PTR_GET(pSta);

        halMacRaStaInfoFromFW(pMac, pRaInfo, staidx, offsetof(tHalRaInfo, sRateTable), sizeof(tHalRaInfo)-offsetof(tHalRaInfo, sRateTable));

        for( halRateIdx = HALRATE_MODE_START; halRateIdx < HALRATE_MODE_TX_END; halRateIdx ++) {
            if( eSIR_SUCCESS != _raRateIsSupportedAndValid( pMac, halRateIdx, 
                        (tANI_U32 *)pRaInfo->supportedRates, 
                        (tANI_U32 *)pRaInfo->validRates,
                        pRaInfo->maxDataRate) )
                continue;
            sRate = (tHalMacRate)pRaInfo->sRateTable[halRateIdx];
            tRate = (tHalMacRate)pRaInfo->tRateTable[halRateIdx];
            
            raLog(pMac, RALOG_CLI, "%2d:%2d  P %4dM(S%3d) => S %4dM(S%3d) => T %4dM(S%3d)\n",
                    (halRateIdx) >> 5,
                    (halRateIdx) & 31,
                    HAL_RA_THRUPUT_GET(halRateIdx), HAL_RA_TPERATEIDX_GET(halRateIdx),
                    HAL_RA_THRUPUT_GET(sRate), HAL_RA_TPERATEIDX_GET(sRate),
                    HAL_RA_THRUPUT_GET(tRate), HAL_RA_TPERATEIDX_GET(tRate)
            );
        }
    } else { 
        raLog(pMac, RALOG_CLI, "staIdx %d invalid\n", staidx);
    }
    return ;
}

void halMacRaDumpHalSamplingRateTable(tpAniSirGlobal pMac, tANI_U32 bssIdx, 
        tANI_U32 maxLowerSamplingRates, tANI_U32 maxHigherSamplingRates)
{
    tHalMacRate halRateIdx;
    tpBssStruct pBss = (tpBssStruct)pMac->hal.halMac.bssTable;
    tHalMacRate startHalRateIdx=HALRATE_MODE_START;
    tHalMacRate endHalRateIdx=HALRATE_MODE_TX_END;
    if(!pBss || HAL_IS_VALID_BSS_INDEX(pMac, bssIdx)==0)
        return;
    pBss = &pBss[bssIdx];
    halMacRaBssInfoFromFW(pMac, &pBss->bssRaInfo, (tANI_U8)bssIdx);

    raLog(pMac, RALOG_CLI, "Dumping BSS %d HAL sampling rate table %d ~ %d\n", bssIdx, startHalRateIdx, endHalRateIdx);

    for( halRateIdx = startHalRateIdx; halRateIdx < endHalRateIdx; halRateIdx ++){
        tANI_U32 index = (halRateIdx);
        tANI_U32 tpeRateIdx = HAL_RA_TPERATEIDX_GET(halRateIdx);
        tANI_U8 *pSample = &pBss->bssRaInfo.bssAutoSampleRateTable[index].sampleRate[RA_SAMPLING_BASE_RATE];
        if(pSample[0] >= HALRATE_INVALID){
            raLog(pMac, RALOG_CLI, "%d %3dM(Tpe%3d) Non sampled candidate rate.  \n",
                    halRateIdx, HAL_RA_THRUPUT_GET(halRateIdx), tpeRateIdx);
        }else{
            tANI_U32 sampIdx = 0;
            raLog(pMac, RALOG_CLI, "%d %3dM(Tpe%3d) Sensitivity: %d MaxTput %dK\n",
                    halRateIdx, HAL_RA_THRUPUT_GET(halRateIdx), tpeRateIdx, HAL_RA_SENSITIVITY_GET(halRateIdx),
                    HAL_RA_ACTUALTPUT_GET(halRateIdx));
#if 0
            raLog(pMac, RALOG_CLI, "x100Kbps TpeRateIdx Sensitivity/10\n");
            for(sampIdx = RA_SAMPLING_BASE_RATE; sampIdx < RA_SAMPLING_RATES_MAX; sampIdx++){
                raLog(pMac, RALOG_CLI, "%8d %7d %12d\n", 
                    (pSample[sampIdx] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx]):0, 
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx]),
                    (pSample[sampIdx]< HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx]):0);
            }
#endif
            raLog(pMac, RALOG_CLI, "  %2d-%2d: %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d ...(x100Kbps) \n",
                    sampIdx, sampIdx+9,
                    (pSample[sampIdx] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx]):0,
                    (pSample[sampIdx+1] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+1]):0,
                    (pSample[sampIdx+2] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+2]):0,
                    (pSample[sampIdx+3] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+3]):0,
                    (pSample[sampIdx+4] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+4]):0,
                    (pSample[sampIdx+5] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+5]):0,
                    (pSample[sampIdx+6] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+6]):0,
                    (pSample[sampIdx+7] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+7]):0,
                    (pSample[sampIdx+8] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+8]):0,
                    (pSample[sampIdx+9] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+9]):0, 
                    (pSample[sampIdx+10] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+10]):0,                        
                    (pSample[sampIdx+11] < HALRATE_INVALID)?HAL_RA_THRUPUT_GET(pSample[sampIdx+11]):0
                 );
            raLog(pMac, RALOG_CLI, "         %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d ...(tpeRateIdx) \n",
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+1]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+2]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+3]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+4]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+5]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+6]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+7]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+8]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+9]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+10]),
                    HAL_RA_TPERATEIDX_GET(pSample[sampIdx+11])
                 );
            raLog(pMac, RALOG_CLI, "         %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d ...(Sensitivity/10) \n",
                    (pSample[sampIdx]< HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx]):0,
                    (pSample[sampIdx+1] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+1]):0,
                    (pSample[sampIdx+2] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+2]):0,
                    (pSample[sampIdx+3] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+3]):0,
                    (pSample[sampIdx+4] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+4]):0,
                    (pSample[sampIdx+5] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+5]):0,
                    (pSample[sampIdx+6] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+6]):0,
                    (pSample[sampIdx+7] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+7]):0,
                    (pSample[sampIdx+8] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+8]):0,
                    (pSample[sampIdx+9] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+9]):0,
                    (pSample[sampIdx+10] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+10]):0,
                    (pSample[sampIdx+11] < HALRATE_INVALID)?HAL_RA_SENSITIVITY_GET(pSample[sampIdx+11]):0
                 );
        }
    }
    return ;
}

eHalStatus halMacRaSetGlobalCfg(tpAniSirGlobal pMac, tANI_U32 id, tANI_U32 value, tANI_U32 value2)
{
    tHalRaGlobalCfg item = (tHalRaGlobalCfg) id;
    tpHalRaGlobalInfo pGlobInfo = &pMac->hal.halRaInfo;
    tANI_U32    offsetItem = 0, sizeItem = 0; // if sizeItem is zero, don't pass to firmware

    if(item != RA_GLOBCFG_NONE){
        raLog(pMac, RALOG_CLI, "Set item %d = %d \n", item, value);

        switch(item){

            case RA_GLOBCFG_PER_BADLINK_JMPTHRESH   :
                pGlobInfo->perBadLinkJumpThreshold = (tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   perBadLinkJumpThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_PER_GOODLINK_JMPTHRESH   :
                pGlobInfo->perGoodLinkJumpThreshold =(tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   perGoodLinkJumpThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_PER_GOODLINK_SAMPLETHRESH   :
                pGlobInfo->perGoodLinkSampleThreshold =(tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   perGoodLinkSampleThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_BADLINK_PERSIST_THRESH   :
                pGlobInfo->badLinkPersistencyThresh  = (tANI_U16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   badLinkPersistencyThresh);
                sizeItem = sizeof(tANI_U16);
                break;

            case RA_GLOBCFG_GOODLINK_PERSIST_THRESH   :
                pGlobInfo->goodLinkPersistencyThresh = (tANI_U16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   goodLinkPersistencyThresh);
                sizeItem = sizeof(tANI_U16);
                break;

            case RA_GLOBCFG_RA_PERIOD:
                pGlobInfo->raPeriod = (tANI_U16)value;
                sizeItem = 0; /* raPeriod passed in the message */
                halMacRaSetRAPeriodicity(pMac, pGlobInfo->raPeriod);
                break;

            case RA_GLOBCFG_TOTAL_FAILURES_THRESHOLD   :
                pGlobInfo->failThreshold = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   failThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_PER_SELECTION:
                pGlobInfo->raPerAlgoSelection = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   raPerAlgoSelection);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_SAMPLE_FAILURES_THRESHOLD   :
                pGlobInfo->consecFailThreshold = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   consecFailThreshold);
                sizeItem = sizeof(tANI_U8);
                break;
            case RA_GLOBCFG_SAMPLE_ALL_PERIOD   :
                pGlobInfo->sampleAllPeriod = (tANI_U16)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   sampleAllPeriod);
                sizeItem = sizeof(tANI_U16);
                break;
            case RA_GLOBCFG_QUICK_SAMPLE_PERIOD   :
                pGlobInfo->quickSamplePeriod = (tANI_U16)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   quickSamplePeriod);
                sizeItem = sizeof(tANI_U16);
                break;
            case RA_GLOBCFG_SAMPLE_MIN_PKTS   :
                pGlobInfo->minTxPerSample = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   minTxPerSample);
                sizeItem = sizeof(tANI_U8);
                break;
            case RA_GLOBCFG_SAMPLE_PERIODS_MIN   :
                pGlobInfo->minPeriodsPerSample =(tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   minPeriodsPerSample);
                sizeItem = sizeof(tANI_U8);
                break;
            case RA_GLOBCFG_RETRY1_SENSITIVITY_DIFF   :
                pGlobInfo->retry1SensitivityDiff  = (tANI_U16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   retry1SensitivityDiff);
                sizeItem = sizeof(tANI_U16);
                break;
            case RA_GLOBCFG_RETRY2_SENSITIVITY_DIFF   :
                pGlobInfo->retry2SensitivityDiff = (tANI_U16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   retry2SensitivityDiff);
                sizeItem = sizeof(tANI_U16);
                break;
            case RA_GLOBCFG_LOWER_SENSITIVITY_SAMPLE_RATES   :
                pGlobInfo->lowerSensSampleRates = (tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   lowerSensSampleRates);
                sizeItem = sizeof(tANI_U8);
                break;
            case RA_GLOBCFG_HIGHER_SENSITIVITY_SAMPLE_RATES   :
                pGlobInfo->higherSensSampleRates = (tANI_U8) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   higherSensSampleRates);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_BETTER_RATE_MAX_SENSITIVITY_DIFF:
                pGlobInfo->betterRateMaxSensDiff = (tANI_S16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   betterRateMaxSensDiff);
                sizeItem = sizeof(tANI_S16);
                break;
                
            case RA_GLOBCFG_LOWER_RATE_MIN_SENSITIVITY_DIFF:
                pGlobInfo->lowerRateMinSensDiff = (tANI_S16) value;
                offsetItem = offsetof(tHalRaGlobalInfo,   lowerRateMinSensDiff);
                sizeItem = sizeof(tANI_S16);
                break;

            case RA_GLOBCFG_BADLINK_RETRYJMP_THRESH:
                pGlobInfo->perBadLinkJumpRetryRateThreshold = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   perBadLinkJumpRetryRateThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_LINK_IDLE_SAMPLES:
                pGlobInfo->linkIdleSamples = (tANI_U16)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   linkIdleSamples);
                sizeItem = sizeof(tANI_U16);
                break;

            case RA_GLOBCFG_GOOD_SAMPLE_EXTRA_STAY_INC_THRESH:
                pGlobInfo->extraStayIncThreshold= (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   extraStayIncThreshold);
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_GOODLINK_PERTHRESH_BY_SENSITIVITY_DIFF:
                pGlobInfo->goodPerThreshBySensitivity[value]  = (tANI_U8)value2;
                offsetItem = offsetof(tHalRaGlobalInfo,   goodPerThreshBySensitivity)+value;
                sizeItem = sizeof(tANI_U8);
                break;

            case RA_GLOBCFG_PER_LOW_IGNORE_THRESH:
                pGlobInfo->perIgnoreThreshold  = (tANI_U8)value;
                offsetItem = offsetof(tHalRaGlobalInfo,   perIgnoreThreshold);
                sizeItem = sizeof(tANI_U8);
                break;
                
            case RA_GLOBCFG_TX_FAIL_EXCEPTION_THRESH:
               pGlobInfo->txFailExceptionThreshold = (tANI_U8)value;
               offsetItem = offsetof(tHalRaGlobalInfo,   txFailExceptionThreshold);
               sizeItem = sizeof(tANI_U8);
               break;               

            default    :
                raLog(pMac, RALOG_CLI, "Item %d value %d ignored\n", item, value);
                sizeItem = 0;

                break;
        }
    }

    raLog(pMac, RALOG_CLI, "%d\tShow this help message\n\n",RA_GLOBCFG_NONE);
    raLog(pMac, RALOG_CLI, "%d\tPER_BADLINK_JMPTHRESH       Current:%d\n",RA_GLOBCFG_PER_BADLINK_JMPTHRESH,pGlobInfo->perBadLinkJumpThreshold );
    raLog(pMac, RALOG_CLI, "%d\tPER_GOODLINK_JMPTHRESH      Current:%d\n",RA_GLOBCFG_PER_GOODLINK_JMPTHRESH,pGlobInfo->perGoodLinkJumpThreshold);
    raLog(pMac, RALOG_CLI, "%d\tPER_GOODLINK_SAMPLETHRESH   Current:%d\n",RA_GLOBCFG_PER_GOODLINK_SAMPLETHRESH,pGlobInfo->perGoodLinkSampleThreshold);
    raLog(pMac, RALOG_CLI, "%d\tBADLINK_PERSIST_THRESH      Current:%d\n",RA_GLOBCFG_BADLINK_PERSIST_THRESH,pGlobInfo->badLinkPersistencyThresh);
    raLog(pMac, RALOG_CLI, "%d\tGOODLINK_PERSIST_THRESH     Current:%d\n",RA_GLOBCFG_GOODLINK_PERSIST_THRESH,pGlobInfo->goodLinkPersistencyThresh);
    raLog(pMac, RALOG_CLI, "%d\tSTAY_SAMPLE_ALL_PERIODS     Current:%d\n",RA_GLOBCFG_SAMPLE_ALL_PERIOD,pGlobInfo->sampleAllPeriod);
    raLog(pMac, RALOG_CLI, "%d\tGOODLINK_PERTHRESH_BY_SENS: \n",RA_GLOBCFG_GOODLINK_PERTHRESH_BY_SENSITIVITY_DIFF);
    raLog(pMac, RALOG_CLI, "\t  0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d\n",
            pGlobInfo->goodPerThreshBySensitivity[0],
            pGlobInfo->goodPerThreshBySensitivity[1],
            pGlobInfo->goodPerThreshBySensitivity[2],
            pGlobInfo->goodPerThreshBySensitivity[3],
            pGlobInfo->goodPerThreshBySensitivity[4],
            pGlobInfo->goodPerThreshBySensitivity[5],
            pGlobInfo->goodPerThreshBySensitivity[6],
            pGlobInfo->goodPerThreshBySensitivity[7]
         );
    raLog(pMac, RALOG_CLI, "%d\tRA_PERIOD                  Current:%d\n",RA_GLOBCFG_RA_PERIOD,pGlobInfo->raPeriod);
    raLog(pMac, RALOG_CLI, "%d\tRA_PER_ALGO_SELECTION      Current:%d\n",RA_PER_SELECTION,pGlobInfo->raPerAlgoSelection);
    raLog(pMac, RALOG_CLI, "%d\tTOTAL_FAILURES_THRESHOLD   Current:%d\n",RA_GLOBCFG_TOTAL_FAILURES_THRESHOLD,pGlobInfo->failThreshold);
    raLog(pMac, RALOG_CLI, "%d\tSAMPLE_FAILURES_THRESHOLD  Current:%d\n",RA_GLOBCFG_SAMPLE_FAILURES_THRESHOLD,pGlobInfo->consecFailThreshold);
    raLog(pMac, RALOG_CLI, "%d\tQUICK_SAMPLE_PERIOD        Current:%d\n",RA_GLOBCFG_QUICK_SAMPLE_PERIOD,pGlobInfo->quickSamplePeriod);
    raLog(pMac, RALOG_CLI, "%d\tSAMPLE_MIN_PKTS            Current:%d\n",RA_GLOBCFG_SAMPLE_MIN_PKTS,pGlobInfo->minTxPerSample);
    raLog(pMac, RALOG_CLI, "%d\tRETRY1_SENSITIVITY_DIFF    Current:%d\n",RA_GLOBCFG_RETRY1_SENSITIVITY_DIFF,pGlobInfo->retry1SensitivityDiff);
    raLog(pMac, RALOG_CLI, "%d\tRETRY2_SENSITIVITY_DIFF    Current:%d\n",RA_GLOBCFG_RETRY2_SENSITIVITY_DIFF,pGlobInfo->retry2SensitivityDiff);
    raLog(pMac, RALOG_CLI, "%d\tHIGHER_SENS_SAMPLE_RATES   Current:%d\n",RA_GLOBCFG_HIGHER_SENSITIVITY_SAMPLE_RATES,pGlobInfo->higherSensSampleRates);
    raLog(pMac, RALOG_CLI, "%d\tBETTER_TPUT_MAX_SENS_DIFF  Current:%d\n",RA_GLOBCFG_BETTER_RATE_MAX_SENSITIVITY_DIFF,pGlobInfo->betterRateMaxSensDiff);
    raLog(pMac, RALOG_CLI, "%d\tLOWER_TPUT_MIN_SENS_DIFF   Current:%d\n",RA_GLOBCFG_LOWER_RATE_MIN_SENSITIVITY_DIFF,pGlobInfo->lowerRateMinSensDiff);
    raLog(pMac, RALOG_CLI, "%d\tBADLINK_RETRYJMP_THRESH    Current:%d\n",RA_GLOBCFG_BADLINK_RETRYJMP_THRESH,pGlobInfo->perBadLinkJumpRetryRateThreshold);
    raLog(pMac, RALOG_CLI, "%d\tLINK_IDLE_SAMPLES          Current:%d\n",RA_GLOBCFG_LINK_IDLE_SAMPLES,pGlobInfo->linkIdleSamples);
    raLog(pMac, RALOG_CLI, "%d\tGOOD_SAMPLE_EXTRA_STAY_INC Current:%d\n",RA_GLOBCFG_GOOD_SAMPLE_EXTRA_STAY_INC_THRESH,pGlobInfo->extraStayIncThreshold);
    raLog(pMac, RALOG_CLI, "%d\tPER_LOW_IGNORE_THRESH      Current:%d\n",RA_GLOBCFG_GOOD_SAMPLE_EXTRA_STAY_INC_THRESH,pGlobInfo->perIgnoreThreshold);
    raLog(pMac, RALOG_CLI, "%d\tTX_FAIL_EXCEPTION_THRESH   Current:%d\n",RA_GLOBCFG_TX_FAIL_EXCEPTION_THRESH,pGlobInfo->txFailExceptionThreshold);
    if(sizeItem)
    {
        halMacRaGlobalInfoToFW(pMac, pGlobInfo, offsetItem, sizeItem);
        halMacRaUpdateParamReq(pMac, RA_UPDATE_GLOBAL_INFO, item);
    }
    return eHAL_STATUS_SUCCESS;
}

typedef enum eHalRaStaCfg {
    RA_STACFG_NONE =0,
    RA_STACFG_RATEADAPTMODE,
    RA_STACFG_FIXEDRATE,
} tHalRaStaCfg;

eHalStatus halMacRaStaSetCfg(tpAniSirGlobal pMac, tANI_U32 staid, 
        tANI_U32 id, tANI_U32 value)
{
    tHalRaGlobalCfg item = (tHalRaGlobalCfg) id;
    tpHalRaInfo pRaInfo;
    tpStaStruct pSta;

    pSta = ((tpStaStruct) pMac->hal.halMac.staTable)+staid;
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);

    if(item != RA_STACFG_NONE){
        raLog(pMac, RALOG_CLI, "Set item %d = %d \n", item, value);

        switch(item){

            case RA_STACFG_RATEADAPTMODE:
                pRaInfo->rateAdaptMode  = (tRateAdaptMode) value;
                break;
            case RA_STACFG_FIXEDRATE:
                pRaInfo->fixedRate      = (tHalMacRate) value;
                break;

            default:
                raLog(pMac, RALOG_CLI, "Item %d value %d ignored\n", item, value);

                break;
        }
    }

    raLog(pMac, RALOG_CLI, "%d\tShow this help message\n\n",RA_GLOBCFG_NONE);
    raLog(pMac, RALOG_CLI, "%d\tRATEADAPTMODE       Current:%d\n",RA_STACFG_RATEADAPTMODE,pRaInfo->rateAdaptMode );
    raLog(pMac, RALOG_CLI, "%d\tFIXEDRATE           Current:%d\n",RA_STACFG_FIXEDRATE,pRaInfo->fixedRate);

    return eHAL_STATUS_SUCCESS;
}

// Function to dump the statistics related to the specific station
void halMacRaDumpStats(tpAniSirGlobal pMac, tANI_U32 staid)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tpHalRaInfo pRaInfo = NULL;
    tTxRateStat *pRaCache = NULL;
	tANI_U8 rateChl, retry;

    pSta = &pSta[staid];

    // Get the rate adaptation info for the sta
    pRaInfo = HAL_RAINFO_PTR_GET(pSta);
    /* update prevStatsChache from the shared memory */
    halMacRaStaInfoFromFW(pMac, pRaInfo, (tANI_U16) staid, offsetof(tHalRaInfo, prevStatsCache), sizeof(pRaInfo->prevStatsCache));

    // Get the STA's local RA statistics pointer
    for (rateChl=0; rateChl<HAL_RA_TXRATE_CHANNEL_NUM; rateChl++) {
        raLog(pMac, RALOG_ERROR, "RA Statistic for %dMHz", (rateChl==0)?20:40);
        for (retry=0; retry<HAL_RA_MAX_RATES; retry++) {
            pRaCache = &pRaInfo->prevStatsCache.rastats[retry][rateChl];
            raLog(pMac, RALOG_ERROR, "%s Rate stats", (retry>0)?((retry==2)?"Tertiary":"Secondary"):"Primary");
            raLog(pMac, RALOG_ERROR, "totalTxPpdus = %d, totalTxMpdus = %d,"
                    "totalAckTimeoutPpdus = %d, totalHybridTx = %d,"
                    "totalRespMpdus = %d, rateIndex = %d",
                    pRaCache->totalTxPpdus, pRaCache->totalTxMpdus, pRaCache->totalAckTimeoutPpdus,
                    pRaCache->totalHybridTx, pRaCache->totalRespMpdus, pRaCache->dataRateIndex);
        }
    }
}


