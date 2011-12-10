/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halMacStats.c:    All statistics handle.
 * Author:   Kiran
 * Date:    4/24/2007
 * History:-
 * Date      Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */



#include "halInternal.h"
#include "halDPU.h"
#include "halStaTableApi.h"
#include "sirApi.h"
#include "halRateAdaptApi.h"
#include "halMacUtilsApi.h"
#include "halMacStats.h"
#include "halUtils.h"

#include "wniApi.h"//DEBUG
#include "limApi.h"//DEBUG
#include "halDebug.h"
#include "halFw.h"

//***********Helper macros********************************************

#define HAL_STATS_UPDATE16BIT_ROLLOVER(a, b) do { \
    if (b < (a & 0xFFFF)) { \
       a += 0x10000; \
    } \
    a = (a & 0xFFFF0000) | b; \
} \
while (0)

#define HAL_UPDATE_STAT64(a, b, c, d) do { \
    if (((*a)+(*b)) < (*a)) (*c)++; \
    (*c) += (*d); \
    (*a) += (*b); \
} \
while(0)

#define HAL_UPDATE_STAT64HiLoOLD(a,b) HAL_UPDATE_STAT64(a##Lo, b##Lo, a##Hi, a##Lo)


//check whether src is wrap around, if so, increase hi. So hi and lo will have the total count of src.
//hi and lo are pointer to U32.
#define HAL_UPDATE_STAT32(lo, src, hi) do { \
    if((src) < *(lo)) (*(hi))++; \
    (*(lo)) = (src); \
    }\
    while(0)

#define HAL_UPDATE_STAT32HiLoOLD(cntr, src) HAL_UPDATE_STAT32(cntr##Lo, src, cntr##Hi)

//*************************************************************************************


//******For Debug***************************
#define HAL_PRINT_STAT32(a,b) do{\
    HALLOGW( halLog(pMac, LOGW, FL("Hi = %x Lo = %x\n"),a,b))));\
    }\
    while(0)

#define HAL_PRINT_STAT32HiLoOLD(a) HAL_PRINT_STAT32(a##Hi, a##Lo)

//*********************************************

/** -------------------------------------------------------------
\fn halMacStats_Open
\brief  This function allocates the memory for the per station stats.
\param   tHalHandle hHal
\param   void *arg
\return  eHalStatus
  -------------------------------------------------------------*/
eHalStatus halMacStats_Open(tHalHandle hHal, void *arg)
{
    eHalStatus status;
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    tpDpuInfo pDpuEntry = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    (void) arg;

    status = palAllocateMemory(pMac->hHdd,
        (void **) &pMac->hal.halMac.macStats.pPerStaStats, sizeof(tAniStaStats)*pMac->hal.halMac.maxSta);

    if(eHAL_STATUS_SUCCESS != status)
    {
        goto out;
    }

    status = palAllocateMemory(pMac->hHdd,
        (void **) &pMac->hal.halMac.wrapStats.pPrevRaBckOffStats, sizeof(tAniSirRABckOffStats)*pMac->hal.halMac.maxSta);

    if(eHAL_STATUS_SUCCESS != status)
    {
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pMac->hal.halMac.wrapStats.pPrevRaBckOffStats, sizeof(tAniSirRABckOffStats)*pMac->hal.halMac.maxSta );

    status = palAllocateMemory(pMac->hHdd,
        (void **) &pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount, sizeof(tAniSirRABckOffStats)*pMac->hal.halMac.maxSta);

    if(eHAL_STATUS_SUCCESS != status)
    {
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount, sizeof(tAniSirRABckOffStats)*pMac->hal.halMac.maxSta );

    status = palAllocateMemory(pMac->hHdd,
        (void **) &pMac->hal.halMac.wrapStats.pDpuWrappedCount, sizeof(tAniSirDpuStats)*pDpuEntry->maxEntries);

    if(eHAL_STATUS_SUCCESS != status)
    {
        goto out;
    }
    palZeroMemory( pMac->hHdd, (void *) pMac->hal.halMac.wrapStats.pDpuWrappedCount, sizeof(tAniSirDpuStats)*pDpuEntry->maxEntries );

    return status;

out:
    halMacStats_Close(hHal, NULL);
    HALLOGE( halLog(pMac, LOGE, FL("MAC STATS open failed\n")));
    return status;

}

/** -------------------------------------------------------------
\fn halMacStats_Close
\brief  This function free the memory for the per station stats
        during hal close.
\param   tHalHandle hHal
\param   void *arg
\return  eHalStatus
  -------------------------------------------------------------*/
eHalStatus halMacStats_Close(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    (void) arg;

    if(pMac->hal.halMac.macStats.pPerStaStats)
        palFreeMemory( pMac->hHdd, pMac->hal.halMac.macStats.pPerStaStats);
    pMac->hal.halMac.macStats.pPerStaStats = NULL;


    if(pMac->hal.halMac.wrapStats.pPrevRaBckOffStats)
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.wrapStats.pPrevRaBckOffStats);
    pMac->hal.halMac.wrapStats.pPrevRaBckOffStats = NULL;

    if(pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount)
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount);
    pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount = NULL;

    if(pMac->hal.halMac.wrapStats.pDpuWrappedCount)
        palFreeMemory(pMac->hHdd, pMac->hal.halMac.wrapStats.pDpuWrappedCount);
    pMac->hal.halMac.wrapStats.pDpuWrappedCount = NULL;

    return eHAL_STATUS_SUCCESS;
}

//***********Helper functions******************************************
static void inline __halUpdate32bitValue(tpAni64BitCounters dst, tANI_U32 val)
{
    HAL_UPDATE_STAT32(&dst->Lo, val, &dst->Hi);
}
static void inline __halCopy64bitCounters(tpAni64BitCounters dst, tpAni64BitCounters src)
{
    dst->Hi = src->Hi;
    dst->Lo = src->Lo;
}
static void inline __halAdd64bitCounters(tpAni64BitCounters dst, tpAni64BitCounters src)
{
    HAL_UPDATE_STAT64(&dst->Lo, &src->Lo, &dst->Hi, &src->Hi);
}

static void __halAddTxRxCounters ( tpAniTxRxStats pDstStats, tpAniTxRxStats pSrcStats )
{
    __halAdd64bitCounters(&pDstStats->nRcvBytes, &pSrcStats->nRcvBytes);
    __halAdd64bitCounters(&pDstStats->nXmitBytes, &pSrcStats->nXmitBytes);
    __halAdd64bitCounters(&pDstStats->txFrames, &pSrcStats->txFrames);
    __halAdd64bitCounters(&pDstStats->rxFrames, &pSrcStats->rxFrames);
}

static void __halUpdateTxRxStats ( tpAniTxRxStats pStats, tpAniTxRxCounters pStatCntrs )
{
    __halUpdate32bitValue( &pStats->nRcvBytes, pStatCntrs->nRcvBytes );
    __halUpdate32bitValue( &pStats->nXmitBytes, pStatCntrs->nXmitBytes );
    __halUpdate32bitValue( &pStats->txFrames, pStatCntrs->txFrames );
    __halUpdate32bitValue( &pStats->rxFrames, pStatCntrs->rxFrames );

}
//****************************************************************************

//****************************DEBUG Functions***********************************
static void inline halPrint64bitCounters(tpAniSirGlobal pMac, tpAni64BitCounters cntr)
{
    HALLOGW( halLog(pMac, LOGW, FL("Hi = %x Lo = %x\n"),cntr->Hi,cntr->Lo));
}
//Debug
tANI_U32 halIssueDebugStatsCmd( tpAniSirGlobal pMac, tANI_U16 type, tANI_U8 staId)
{
   tpAniGetStatsReq pReq;
   tSirMsgQ pMsg;

   if((palAllocateMemory(pMac->hHdd, (void**)&pReq, sizeof(tpAniGetStatsReq))) != eHAL_STATUS_SUCCESS)
   	return eHAL_STATUS_FAILURE;

   pReq->msgType = type;
   halTable_FindAddrByStaid(pMac, staId, pReq->macAddr);

   HALLOGW( halLog(pMac, LOGW, FL("*********\nsta stats req....MacAddr = %02X-%02X-%02X-%02X-%02X-%02X.\n************\n"),
                               pReq->macAddr[0],pReq->macAddr[1],pReq->macAddr[2],pReq->macAddr[3],pReq->macAddr[4],pReq->macAddr[5]));

   pMsg.type = type;
   pMsg.bodyptr = pReq;
   pMsg.bodyval = 0;

   return limPostMsgApi(pMac, &pMsg);

}

void halMacPrintPerStaStats(tpAniSirGlobal pMac, tpAniGetPerStaStatsRsp pRsp)
{
     halPrint64bitCounters(pMac, &pRsp->sta.ucStats.nRcvBytes);
     halPrint64bitCounters(pMac, &pRsp->sta.ucStats.nXmitBytes);
     halPrint64bitCounters(pMac, &pRsp->sta.ucStats.txFrames);
     halPrint64bitCounters(pMac, &pRsp->sta.ucStats.rxFrames);

     HALLOGW( halLog(pMac, LOGW, FL("Max Tx rate - %x Max Rx Rate %x curr Tx Rate = %x  cur Rx Rate = %x RSSI = %x %x %x"),
                    pRsp->sta.maxTxRate, pRsp->sta.maxRxRate, pRsp->sta.currentTxRate, pRsp->sta.currentRxRate,
                    pRsp->sta.rssi[0], pRsp->sta.rssi[1], pRsp->sta.rssi[2]));

}

void halMacPrintGlobalStats(tpAniSirGlobal pMac, tpAniGetGlobalStatsRsp pRsp)
{
       halPrint64bitCounters(pMac, &pRsp->global.bcStats.nRcvBytes);
       halPrint64bitCounters(pMac, &pRsp->global.bcStats.nXmitBytes);
       halPrint64bitCounters(pMac, &pRsp->global.bcStats.txFrames);
       halPrint64bitCounters(pMac, &pRsp->global.bcStats.rxFrames);

       halPrint64bitCounters(pMac, &pRsp->global.mcStats.nRcvBytes);
       halPrint64bitCounters(pMac, &pRsp->global.mcStats.nXmitBytes);
       halPrint64bitCounters(pMac, &pRsp->global.mcStats.txFrames);
       halPrint64bitCounters(pMac, &pRsp->global.mcStats.rxFrames);

       halPrint64bitCounters(pMac, &pRsp->global.securityStats.aes.decryptOkCnt);
       halPrint64bitCounters(pMac, &pRsp->global.securityStats.aes.decryptErr);
}

void halMacPrintStatsSummary(tpAniSirGlobal pMac, tpAniGetStatSummaryRsp pRsp)
{

       halPrint64bitCounters(pMac, &pRsp->stat.uc.nRcvBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.uc.nXmitBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.uc.txFrames);
       halPrint64bitCounters(pMac, &pRsp->stat.uc.rxFrames);

       halPrint64bitCounters(pMac, &pRsp->stat.bc.nRcvBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.bc.nXmitBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.bc.txFrames);
       halPrint64bitCounters(pMac, &pRsp->stat.bc.rxFrames);

       halPrint64bitCounters(pMac, &pRsp->stat.mc.nRcvBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.mc.nXmitBytes);
       halPrint64bitCounters(pMac, &pRsp->stat.mc.txFrames);
       halPrint64bitCounters(pMac, &pRsp->stat.mc.rxFrames);

       halPrint64bitCounters(pMac, &pRsp->stat.txError);
       halPrint64bitCounters(pMac, &pRsp->stat.rxError);


}

//End Debug
//*************************************************************************************************

static void __halGetPeriodicGlobalStats(tpAniSirGlobal pMac)
{
    /** Have to read the device memory from here */
}

static eHalStatus __halUpdateSecurityStats(tpAniSirGlobal pMac, tANI_U8 dpuIdx, tpAniSecStats pSecurityStats)
{
    tDpuStatsParams dpuStats;
    tpAniSecurityStat pSecStat = NULL;
    eHalStatus status = eHAL_STATUS_SUCCESS;

     status = halDpu_GetStatus( pMac, dpuIdx, &dpuStats );
     if(eHAL_STATUS_SUCCESS != status)
     {
         //log error and return;
         return status;
     }

     switch(dpuStats.encMode)
     {
         case eSIR_ED_NONE://Do Nothing
            break;
         case eSIR_ED_WEP40:
         case eSIR_ED_WEP104:
             //Update Wep related cfgs
            pSecStat = &pSecurityStats->wep;
             break;
         case eSIR_ED_TKIP:
            //Update TKIP related config
            pSecStat = &pSecurityStats->tkip;
            __halUpdate32bitValue(&pSecurityStats->tkipReplays, dpuStats.replays);
            __halUpdate32bitValue(&pSecurityStats->tkipMicError, dpuStats.micErrorCnt);
            break;
         case eSIR_ED_CCMP:
            //Update AES related config
            pSecStat = &pSecurityStats->aes;
            __halUpdate32bitValue(&pSecurityStats->aesReplays, dpuStats.replays);
            break;
        default:
            //Error!!!
            return eHAL_STATUS_FAILURE;
    }

    if(pSecStat)
    {
        __halUpdate32bitValue(&pSecStat->txBlks, dpuStats.sendBlocks);
        __halUpdate32bitValue(&pSecStat->rxBlks, dpuStats.recvBlocks);
        __halUpdate32bitValue(&pSecStat->protExclCnt, dpuStats.protExclCnt);
        __halUpdate32bitValue(&pSecStat->formatErrorCnt, dpuStats.formatErrCnt);
        __halUpdate32bitValue(&pSecStat->unDecryptableCnt, dpuStats.unDecryptableCnt);
        __halUpdate32bitValue(&pSecStat->decryptErr, dpuStats.decryptErrCnt);
        __halUpdate32bitValue(&pSecStat->decryptOkCnt, dpuStats.decryptOkCnt);

    }

    return status;

}

static eHalStatus __halUpdateStaSecStats(tpAniSirGlobal pMac, tANI_U8 staIdx)
{
    tANI_U8 dpuIdx;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    //Update Security Stats.
    status = halTable_GetStaDpuIdx( pMac, staIdx, &dpuIdx);
    if(eHAL_STATUS_SUCCESS != status)
     {
        //log error and return;
        return status;
     }

    status = __halUpdateSecurityStats( pMac, dpuIdx, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.securityStats);

    return status;

}

static eHalStatus __halUpdateStaBcSecStats(tpAniSirGlobal pMac, tANI_U8 staIdx)
{
    tANI_U8 dpuIdx, bssDpuIdx;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bssIdx;

    status = halTable_GetBssIndexForSta(pMac,&bssIdx,staIdx);

    if(eHAL_STATUS_SUCCESS != status) return status;

    status = halTable_GetBssDpuIdx( pMac, bssIdx, &bssDpuIdx);
    if(eHAL_STATUS_SUCCESS != status)
    {
        //log error and return;
        return status;
    }

    //Update Security Stats.
     status = halTable_GetStaBcastDpuIdx( pMac, staIdx, &dpuIdx);
     if(eHAL_STATUS_SUCCESS != status)
     {
         //log error and return;
         return status;
     }

    if(bssDpuIdx == dpuIdx) return status; //Nothing to be done here.

    status = __halUpdateSecurityStats( pMac, dpuIdx, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staBcStat );

         return status;

     }

static void __halUpdateStaFrameStatCounters( tpAniSirGlobal pMac, tANI_U8 staIdx )
     {
    //Update Unicast Counters
    __halUpdateTxRxStats(&pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.ucStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStatCntrs );

    //Update Broadcast Counters
    __halUpdateTxRxStats(&pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.bcStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].bcCntrs);

    //Update Multicast Counters
    __halUpdateTxRxStats( &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.mcStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].mcCntrs );

}

static eHalStatus __halGetPeriodicPerStaStats( tpAniSirGlobal pMac, tANI_U8 staIdx )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tBssSystemRole BssSystemRole;
    tANI_U8 bssIdx = eHAL_STATUS_INVALID_BSSIDX; 

    //Update Frame Counters.
    __halUpdateStaFrameStatCounters( pMac, staIdx );

    status = __halUpdateStaSecStats(pMac, staIdx);
    if (status != eHAL_STATUS_SUCCESS) return status;

    status = halTable_GetBssIndexForSta( pMac, &bssIdx, staIdx );
    if (eHAL_STATUS_SUCCESS != status) return status;

    halLog(pMac, LOG2, FL("**Bssidx=%d STaidx: %d\n"), 
        bssIdx, staIdx);

    BssSystemRole = halGetBssSystemRole(pMac, bssIdx);

    if ((BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
        (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE) ||
        (BssSystemRole == eSYSTEM_BTAMP_STA_ROLE))
     {
        //Update multicast/unicast stats.
        status = __halUpdateStaBcSecStats(pMac, staIdx);
        if(status != eHAL_STATUS_SUCCESS) return status;

     }

    return status;
}

static eHalStatus __halUpdateBcMcStats(tpAniSirGlobal pMac, tANI_U8 bssIdx)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 dpuIdx;

    status = halTable_GetBssDpuIdx( pMac, bssIdx, &dpuIdx);
    if(eHAL_STATUS_SUCCESS != status)
    {
        //log error and return;
        return status;
    }

    status = __halUpdateSecurityStats( pMac, dpuIdx, &pMac->hal.halMac.macStats.globalStat.securityStats );

    //Bc and Mc Frame Stats will be updated, while processing the Global stats request.

    return status;
}

void halMacPeriodicStatCollection(tpAniSirGlobal pMac)
{
     //Collect stats from softmac and hardware and Update it into stats in HAL.
    //
    tANI_U8 i;
    tANI_U8 halState;
    eHalStatus status;
    pMac->hal.halMac.macStats.periodicStatCnt++;
    HALLOG4( halLog(pMac, LOG4, FL("\n********\nInside Periodic routine\n**************\n")));


    // Collect periodic radio stats
    // As per current interface -- Post a request to read runtime stats.
    // Update data structures accordingly.
    __halGetPeriodicGlobalStats(pMac);

    halState = halStateGet(pMac);
    // Collect periodic per sta stats. - Read DPU descriptor for each station and update stats.
    // Further code requires CFG download to be completed.
    if ((eHAL_CFG == halState) ||
          (eHAL_STARTED == halState) ||
          (eHAL_SYS_READY == halState) ||
          (eHAL_NORMAL == halState))
    {
        for (i = 0; i < pMac->hal.halMac.maxSta; i++)
        {
            status = halTable_ValidateStaIndex(pMac, i);
            if ( status == (eHalStatus)eHAL_STATUS_SUCCESS )
            {
                if ((status = __halGetPeriodicPerStaStats(pMac, i)) != (eHalStatus)eSIR_SUCCESS)
                {
                    HALLOGE( halLog(pMac, LOGE, FL("halMacPeriodicStatCollection: DPH and HW are out of sync - HW invalid STA ID %d\n"), i));
                    break;
                }
            }
        }
        for (i = 0; i < pMac->hal.halMac.maxBssId; i++)
        {
            if ( (status = halTable_ValidateBssIndex(pMac, i)) == (eHalStatus)eHAL_STATUS_SUCCESS )
            {
                if ((status = __halUpdateBcMcStats(pMac, i)) != (eHalStatus)eSIR_SUCCESS)
                {
                    HALLOGE( halLog(pMac, LOGE, FL("halMacPeriodicStatCollection: DPH and HW are out of sync - HW invalid STA ID %d\n"), i));
                    break;
                }
                break;
            }
         }

    }

}

#define SET_WRAP_AROUND(_param1, _param2, _param3)  \
{                                                   \
    if(_param1 < _param2)                           \
        _param3++;                                  \
    _param2 = _param1;                              \
}

/** -------------------------------------------------------------
\fn halMacWrapAroundStatCollection
\brief  This function is invoked periodically to check the wrap around
        issues for the sta and dpu stats collection.
\param   tHalHandle hHal
\return  void
  -------------------------------------------------------------*/
void halMacWrapAroundStatCollection(tpAniSirGlobal pMac)
{
    tANI_U8 writeDpuDesc = 0;
    tANI_U32 dpuEntriesFound = 0, staEntriesFound = 0;
    void *pRaStats;
    void *pDpuDescStats;
    eHalStatus status;
    tANI_U32 i, k, address;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

    //no need to check for wrap around issues of hw counters in IMPS mode
    if(halPS_GetState(pMac) == HAL_PWR_SAVE_IMPS_STATE)
        return;

    //compute the total dpu entries found
    {
        for (i=0; i < pDpu->maxEntries; i++ )
        {
            if(pDpu->descTable[i].used == 1)
            {
                dpuEntriesFound++;
                // fill in the gaps
                if(dpuEntriesFound != i+1)
                    dpuEntriesFound = i+1;
            }
        }

    }

    //compute the total sta entries found
    {
        for (i=0; i < pMac->hal.halMac.maxSta; i++)
        {
            if (pSta[i].valid)
            {
                staEntriesFound++;
                // fill in the gaps
                if(staEntriesFound != i+1)
                    staEntriesFound = i+1;

            }
        }
    }

    status = palAllocateMemory(pMac->hHdd,
        (void **) &pRaStats, sizeof(tpTpeStaDescAndStats)*staEntriesFound);
    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("stats alloc mem failed1\n")));
        return;
    }
    palZeroMemory( pMac->hHdd, (void *) pRaStats, sizeof(tpTpeStaDescAndStats)*staEntriesFound );


    status = palAllocateMemory(pMac->hHdd,
        (void **) &pDpuDescStats, sizeof(tDpuDescriptor)*dpuEntriesFound);
    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("stats alloc mem failed2\n")));
        return;
    }
    palZeroMemory( pMac->hHdd, (void *) pDpuDescStats, sizeof(tDpuDescriptor)*dpuEntriesFound );

    //Read the TPE descriptors for staEntriesFound
    {
        address = pMac->hal.memMap.tpeStaDesc_offset;
        halReadDeviceMemory(pMac, address, (tANI_U8 *)pRaStats,
                            sizeof(tpTpeStaDescAndStats)*staEntriesFound);
    }

    //Read the DPU descriptors for dpuEntriesFound
    {
        address = pMac->hal.memMap.dpuDescriptor_offset;
        halReadDeviceMemory(pMac, address, (tANI_U8 *)pDpuDescStats,
                                 sizeof(tDpuDescriptor)*dpuEntriesFound);
    }

    // update TPE sta stats
    for(i = 0; i < staEntriesFound; i++)
    {

        tpeStaDot11Stats pDot11Stats;
        tpTpeStaRaStats pStaRaStats;
        tAniSirRABckOffStats prevRAStats;
        tAniSirRABckOffStats raWrappedCount;
        tANI_U8 *ptr = (tANI_U8 *)pRaStats;

        if (!pSta[i].valid)
        {
            continue;
        }
        ptr += (sizeof(tpTpeStaDescAndStats) * i) + sizeof(tTpeStaDesc);

        pStaRaStats = (tpTpeStaRaStats)ptr;
        pDot11Stats = (tpeStaDot11Stats)(ptr + sizeof(tTpeStaRaStats));

        prevRAStats = pMac->hal.halMac.wrapStats.pPrevRaBckOffStats[i];
        raWrappedCount = pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount[i];

        //DOT 11 stats
        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->txFrmRetryCnt < prevRAStats.txFrmRetryCnt[k])
                raWrappedCount.txFrmRetryCnt[k]++;
            prevRAStats.txFrmRetryCnt[k] = (pDot11Stats+7-k)->txFrmRetryCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->txFrmMultiRetryCnt < prevRAStats.txFrmMultiRetryCnt[k])
                raWrappedCount.txFrmMultiRetryCnt[k]++;
            prevRAStats.txFrmMultiRetryCnt[k] = (pDot11Stats+7-k)->txFrmMultiRetryCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->txFrmSuccCnt < prevRAStats.txFrmSuccCnt[k])
                raWrappedCount.txFrmSuccCnt[k]++;
            prevRAStats.txFrmSuccCnt[k] = (pDot11Stats+7-k)->txFrmSuccCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->txFrmFailCnt < prevRAStats.txFrmFailCnt[k])
                raWrappedCount.txFrmFailCnt[k]++;
            prevRAStats.txFrmFailCnt[k] = (pDot11Stats+7-k)->txFrmFailCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->rtsFailCnt < prevRAStats.rtsFailCnt[k])
                raWrappedCount.rtsFailCnt[k]++;
            prevRAStats.rtsFailCnt[k] = (pDot11Stats+7-k)->rtsFailCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->ackFailCnt < prevRAStats.ackFailCnt[k])
                raWrappedCount.ackFailCnt[k]++;
            prevRAStats.ackFailCnt[k] = (pDot11Stats+7-k)->ackFailCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->rtsSuccCnt < prevRAStats.rtsSuccCnt[k])
                raWrappedCount.rtsSuccCnt[k]++;
            prevRAStats.rtsSuccCnt[k] = (pDot11Stats+7-k)->rtsSuccCnt;
        }

        for(k = 0; k < 4; k++)
        {
            if((pDot11Stats+7-k)->txFragCnt < prevRAStats.txFragCnt[k])
                raWrappedCount.txFragCnt[k]++;
            prevRAStats.txFragCnt[k] = (pDot11Stats+7-k)->txFragCnt;
        }

        //RA stats
        SET_WRAP_AROUND(pStaRaStats->tot20MTxPpduDataFrms1, prevRAStats.tot20MTxPpduDataFrms1, raWrappedCount.tot20MTxPpduDataFrms1)
        SET_WRAP_AROUND(pStaRaStats->tot20MTxPpduDataFrms2, prevRAStats.tot20MTxPpduDataFrms2, raWrappedCount.tot20MTxPpduDataFrms2)
        SET_WRAP_AROUND(pStaRaStats->tot20MTxPpduDataFrms3, prevRAStats.tot20MTxPpduDataFrms3, raWrappedCount.tot20MTxPpduDataFrms3)

        SET_WRAP_AROUND(pStaRaStats->tot20MTxMpduDataFrms1, prevRAStats.tot20MTxMpduDataFrms1, raWrappedCount.tot20MTxMpduDataFrms1)
        SET_WRAP_AROUND(pStaRaStats->tot20MTxMpduDataFrms2, prevRAStats.tot20MTxMpduDataFrms2, raWrappedCount.tot20MTxMpduDataFrms2)
        SET_WRAP_AROUND(pStaRaStats->tot20MTxMpduDataFrms3, prevRAStats.tot20MTxMpduDataFrms3, raWrappedCount.tot20MTxMpduDataFrms3)

        SET_WRAP_AROUND(pStaRaStats->tot20MMpduInAmPdu1, prevRAStats.tot20MMpduInAmPdu1, raWrappedCount.tot20MMpduInAmPdu1)
        SET_WRAP_AROUND(pStaRaStats->tot20MMpduInAmPdu2, prevRAStats.tot20MMpduInAmPdu2, raWrappedCount.tot20MMpduInAmPdu2)
        SET_WRAP_AROUND(pStaRaStats->tot20MMpduInAmPdu3, prevRAStats.tot20MMpduInAmPdu3, raWrappedCount.tot20MMpduInAmPdu3)
    }

    // update dpu desc stats
    for(i = 0; i < dpuEntriesFound; i++)
    {
        tDpuDescriptor *pDpuDesc;
        tAniSirDpuStats dpuWrappedCount;
        tANI_U8 *ptr = (tANI_U8 *)pDpuDescStats;

        if(pDpu->descTable[i].used == 0)
            continue;

        ptr += (sizeof(tDpuDescriptor) * i);
        pDpuDesc = (tDpuDescriptor *)ptr;

        dpuWrappedCount = pMac->hal.halMac.wrapStats.pDpuWrappedCount[i];

        if(pDpuDesc->micErrCount == 0xFF)
        {
            dpuWrappedCount.micErrCount += 1; //need
            pDpuDesc->micErrCount = 0; //write it back to dpu
            writeDpuDesc = 1;
        }
        if(pDpuDesc->extIVerror == 0xFF)
        {
            dpuWrappedCount.extIVerror += 1; //need
            pDpuDesc->extIVerror = 0; //write it back to dpu
            writeDpuDesc = 1;
        }
        if(pDpuDesc->formatErrorCount == 0xFFFF)
        {
            dpuWrappedCount.formatErrorCount += 1; //need
            pDpuDesc->formatErrorCount = 0; //write it back to dpu
            writeDpuDesc = 1;
        }
        if(pDpuDesc->undecryptableCount == 0xFFFF)
        {
            dpuWrappedCount.undecryptableCount += 1; //need
            pDpuDesc->undecryptableCount = 0; //write it back to dpu
            writeDpuDesc = 1;
        }

        if(writeDpuDesc)
        {
            address = pMac->hal.memMap.dpuDescriptor_offset + (i * sizeof(tDpuDescriptor));
            halWriteDeviceMemory(pMac, address, (tANI_U8 *)pDpuDesc,
                                     sizeof(tDpuDescriptor));
        }

    }

    //Free the memory
    palFreeMemory(pMac->hHdd, pRaStats);
    palFreeMemory(pMac->hHdd, pDpuDescStats);
}



eHalStatus halMacCollectAndClearStaStats( tpAniSirGlobal pMac, tANI_U8 staIdx )
{
    if ( staIdx > pMac->hal.halMac.maxSta )
    {
        return eHAL_STATUS_FAILURE;
    }

    /* Update current counters before adding */
    __halUpdateStaFrameStatCounters( pMac, staIdx );

    __halAddTxRxCounters ( &pMac->hal.halMac.macStats.globalUCStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.ucStats );
    __halAddTxRxCounters ( &pMac->hal.halMac.macStats.globalBCStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.bcStats );
    __halAddTxRxCounters ( &pMac->hal.halMac.macStats.globalMCStats, &pMac->hal.halMac.macStats.pPerStaStats[staIdx].staStat.mcStats );

   return palZeroMemory ( pMac, &pMac->hal.halMac.macStats.pPerStaStats[ staIdx ], sizeof( pMac->hal.halMac.macStats.pPerStaStats[ 0 ]) );

}

/*************************************************************************************************

Request/Response handler functions

*************************************************************************************************/
void halMacGetRssi(tpAniSirGlobal pMac,tANI_U8 staId, tANI_S8 rssiDB[3])
{


    rssiDB[0] = (tANI_S8)HAL_GET_RSSI_DB(pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.phyStatLo);
    rssiDB[1] = 0;
    rssiDB[2] = 0;

    HALLOG2( halLog(pMac, LOG2, FL("**CurrentRate: %d rssi0: %d, rssi1: %d, rssi2: %d\n"),
                                    pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.currentRxRateIdx,
                                    rssiDB[0], rssiDB[1], rssiDB[2]));
}

static void __halMacHandlePerStaStatsReq( tpAniSirGlobal pMac, tpAniGetStatsReq pReq )
{
    tANI_U8 staId;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniGetPerStaStatsRsp pRsp;

    if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pRsp, sizeof(tAniGetPerStaStatsRsp))))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp));

    pRsp->msgLen = sizeof(tAniStaStatStruct);
    pRsp->staId = pReq->staId;
    palCopyMemory(pMac->hHdd, &pRsp->macAddr, &pReq->macAddr, 6);
    pRsp->transactionId = pReq->transactionId;

    //Use Mac Address to find the corresponding sta index.
    status = halTable_FindStaidByAddr(pMac, pReq->macAddr, &staId);

    if(status != eHAL_STATUS_SUCCESS){
        goto end;
    }

    //Update stats before sending response.
    if ((status = __halGetPeriodicPerStaStats(pMac, staId)) != eSIR_SUCCESS)
    {
        goto end;
    }
    halMacGetRssi(pMac, staId, pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.rssi);

    {
        tANI_S32 halrateId = halRate_tpeRate2HalRate(pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.currentRxRateIdx);
        if( halrateId != HALRATE_INVALID )
        {
            pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.currentRxRate =
                HAL_RA_THRUPUT_GET(halrateId) ;
        }
    }

    halMacRaGetStaTxRate(pMac, staId,
            (tHalMacRate*)&pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.currentTxRateIdx,
            &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.currentTxRate,
            NULL,
            &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.maxTxRate,
            NULL);

    pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.maxRxRate = pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.maxTxRate;

    palCopyMemory(pMac->hHdd, &pRsp->sta, &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat, sizeof(tAniStaStatStruct));


end:
    pRsp->rc = status;

    halMsg_GenerateRsp( pMac, SIR_HAL_STA_STAT_RSP, (tANI_U16) 0, pRsp, 0);
}

static void __halMacAddBcStats( tpAniSirGlobal pMac, tpAniSecurityStat pSecStatsDst, tpAniSecurityStat pSecStatsSrc)
{

    __halAdd64bitCounters(&pSecStatsDst->txBlks, &pSecStatsSrc->txBlks);
    __halAdd64bitCounters(&pSecStatsDst->rxBlks, &pSecStatsSrc->rxBlks);
    __halAdd64bitCounters(&pSecStatsDst->protExclCnt, &pSecStatsSrc->protExclCnt);
    __halAdd64bitCounters(&pSecStatsDst->formatErrorCnt, &pSecStatsSrc->formatErrorCnt);
    __halAdd64bitCounters(&pSecStatsDst->unDecryptableCnt, &pSecStatsSrc->unDecryptableCnt);
    __halAdd64bitCounters(&pSecStatsDst->decryptErr, &pSecStatsSrc->decryptErr);
    __halAdd64bitCounters(&pSecStatsDst->decryptOkCnt, &pSecStatsSrc->decryptOkCnt);

         }

static void __halMacHandleGlobalStatsReq( tpAniSirGlobal pMac, tpAniGetStatsReq pReq )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniGetGlobalStatsRsp pRsp;
    tANI_U8 i;
    tANI_U8 bssIdx;

    if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pRsp, sizeof(tAniGetGlobalStatsRsp))))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp));

    pRsp->msgLen = sizeof(tAniGlobalStatStruct);
    pRsp->transactionId = pReq->transactionId;

    __halUpdateBcMcStats( pMac, 0 );

    palCopyMemory(pMac->hHdd, &pRsp->global, &pMac->hal.halMac.macStats.globalStat, sizeof(tAniGlobalStatStruct));
    //Update Security stats from each of the stations bcast DPU in IBSS.
    for (i = 0; i < pMac->hal.halMac.maxSta; i++)
    {
        if (halTable_GetBssIndexForSta( pMac, &bssIdx, i ) !=
                eHAL_STATUS_SUCCESS) continue;
        if (halGetBssSystemRole(pMac, bssIdx) != eSYSTEM_STA_IN_IBSS_ROLE) continue;
        if ( (status = halTable_ValidateStaIndex(pMac, i)) == eHAL_STATUS_SUCCESS )
        {
            __halAddTxRxCounters ( &pRsp->global.bcStats, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.bcStats );
            __halAddTxRxCounters ( &pRsp->global.mcStats, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.mcStats );

            __halMacAddBcStats(pMac, &pRsp->global.securityStats.aes, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.aes);
            __halMacAddBcStats(pMac, &pRsp->global.securityStats.tkip, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.tkip);
            __halMacAddBcStats(pMac, &pRsp->global.securityStats.wep, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.wep);
            __halAdd64bitCounters(&pRsp->global.securityStats.tkipMicError, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.tkipMicError);
            __halAdd64bitCounters(&pRsp->global.securityStats.tkipReplays, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.tkipReplays);
            __halAdd64bitCounters(&pRsp->global.securityStats.aesReplays, &pMac->hal.halMac.macStats.pPerStaStats[i].staBcStat.aesReplays);
        }
    }

    __halAddTxRxCounters ( &pRsp->global.bcStats, &pMac->hal.halMac.macStats.globalBCStats );
    __halAddTxRxCounters ( &pRsp->global.mcStats, &pMac->hal.halMac.macStats.globalMCStats );

    pRsp->rc = status;

    halMsg_GenerateRsp( pMac, SIR_HAL_GLOBAL_STAT_RSP, (tANI_U16) 0, pRsp, 0);

}

static void __halMacUpdateAggrSecStat( tpAniSecurityStat pAggr, tpAniSecurityStat pSta)
{
    __halAdd64bitCounters(&pAggr->txBlks, &pSta->txBlks);
    __halAdd64bitCounters(&pAggr->rxBlks, &pSta->rxBlks);
    __halAdd64bitCounters(&pAggr->protExclCnt, &pSta->protExclCnt);
    __halAdd64bitCounters(&pAggr->formatErrorCnt, &pSta->formatErrorCnt);
    __halAdd64bitCounters(&pAggr->unDecryptableCnt, &pSta->unDecryptableCnt);
    __halAdd64bitCounters(&pAggr->decryptErr, &pSta->decryptErr);
    __halAdd64bitCounters(&pAggr->decryptOkCnt, &pSta->decryptOkCnt);
}

static void __halMacHandleAggrStatsReq( tpAniSirGlobal pMac, tpAniGetStatsReq pReq )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniGetAggrStaStatsRsp pRsp;
    tpAniStaStatStruct pSta;
    tANI_U8 i;

    if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pRsp, sizeof(tAniGetAggrStaStatsRsp))))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp));

    pRsp->msgLen = sizeof(tAniStaStatStruct);
    pRsp->transactionId = pReq->transactionId;
    pSta = &pRsp->sta;

    for (i = 0; i < pMac->hal.halMac.maxSta; i++)
    {
        if ( (status = halTable_ValidateStaIndex(pMac, i)) == eHAL_STATUS_SUCCESS )
        {
            if ((status = __halGetPeriodicPerStaStats(pMac, i)) != eSIR_SUCCESS) break;


            __halAddTxRxCounters ( &pSta->ucStats, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.ucStats );
            __halAddTxRxCounters ( &pSta->bcStats, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.bcStats );
            __halAddTxRxCounters ( &pSta->mcStats, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.mcStats );

            __halMacUpdateAggrSecStat( &pSta->securityStats.aes, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.aes);
            __halMacUpdateAggrSecStat( &pSta->securityStats.wep, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.wep);
            __halMacUpdateAggrSecStat( &pSta->securityStats.tkip, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.tkip);

            __halAdd64bitCounters( &pSta->securityStats.tkipReplays, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.tkipReplays);
            __halAdd64bitCounters( &pSta->securityStats.aesReplays, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.aesReplays);
            __halAdd64bitCounters( &pSta->securityStats.tkipMicError, &pMac->hal.halMac.macStats.pPerStaStats[i].staStat.securityStats.tkipMicError);
        }
    }

    pRsp->rc = status;

    halMsg_GenerateRsp( pMac, SIR_HAL_AGGR_STAT_RSP, (tANI_U16) 0, pRsp, 0);

}
#ifdef FIXME_GEN6
static tPacketType __halMacStatsConvertPacketType(tpAniSirGlobal pMac, tANI_U8 mpiPacketType)
{
    tPacketType packetType = ePACKET_TYPE_UNKNOWN;

    switch( mpiPacketType )
    {
        case PKT_TYPE_11a:
            if( eRF_BAND_5_GHZ == halUtil_GetRfBand(pMac, pMac->hal.currentChannel) )
                packetType = ePACKET_TYPE_11A;
            else
                packetType = ePACKET_TYPE_11G;
            break;
        case PKT_TYPE_11b:
            packetType = ePACKET_TYPE_11B;
            break;
        case PKT_TYPE_11N_GREENFIELD:
        case PKT_TYPE_11N_MIXEDMODE:
            packetType = ePACKET_TYPE_11N;
            break;
        default:
            packetType = ePACKET_TYPE_UNKNOWN;
            break;
    }

    return packetType;

}
#endif
static void __halMacHandleStatSummaryReq( tpAniSirGlobal pMac, tpAniGetStatsReq pReq )
{
    tANI_U8 staId;
    tANI_U8 lastValidStaId = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniGetStatSummaryRsp pRsp;
    tANI_S32 halRate;
#ifdef FIXME_GEN6
    tHalMacRate currTxRateId;
    tANI_U8 packetType;
#endif
    if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pRsp, sizeof(tAniGetStatSummaryRsp))))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp));

    pRsp->msgLen = sizeof(tAniGetStatSummaryRsp);
    pRsp->transactionId = pReq->transactionId;


    for (staId = 0; staId < pMac->hal.halMac.maxSta; staId++)
    {
        if ( (status = halTable_ValidateStaIndex(pMac, staId)) == eHAL_STATUS_SUCCESS )
        {
            __halUpdateStaFrameStatCounters( pMac, staId );

            __halAddTxRxCounters ( &pRsp->stat.uc, &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.ucStats );
            __halAddTxRxCounters ( &pRsp->stat.bc, &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.bcStats );
            __halAddTxRxCounters ( &pRsp->stat.mc, &pMac->hal.halMac.macStats.pPerStaStats[staId].staStat.mcStats );


            lastValidStaId = staId;

        }

    }

    //Add global Stats to aggregrated stats.
    __halAddTxRxCounters ( &pRsp->stat.uc, &pMac->hal.halMac.macStats.globalUCStats );
    __halAddTxRxCounters ( &pRsp->stat.bc, &pMac->hal.halMac.macStats.globalBCStats );
    __halAddTxRxCounters ( &pRsp->stat.mc, &pMac->hal.halMac.macStats.globalMCStats );


    __halCopy64bitCounters(&pRsp->stat.rxError, &pMac->hal.halMac.macStats.globalStat.rxError);
    __halCopy64bitCounters(&pRsp->stat.txError, &pMac->hal.halMac.macStats.globalStat.txError);

    if ( (status = halTable_ValidateStaIndex(pMac, pMac->hal.halMac.macStats.lastStatStaId)) == eHAL_STATUS_SUCCESS )
    {
        lastValidStaId = pMac->hal.halMac.macStats.lastStatStaId;
    }

    halMacGetRssi( pMac, lastValidStaId, pRsp->stat.rssi);
    halRate = halRate_tpeRate2HalRate(pMac->hal.halMac.macStats.pPerStaStats[lastValidStaId].staStat.currentRxRateIdx);
    if( halRate != HALRATE_INVALID )
    {
        pRsp->stat.rxRate = HAL_RA_THRUPUT_GET(halRate) ;
        pRsp->stat.rxMCSId = HAL_RA_IERATEMCSIDX_GET(halRate);
#ifdef FIXME_GEN6
        halRate_getPacketTypeFromHalRate( halRate, &packetType );
        pRsp->stat.rxPacketType = __halMacStatsConvertPacketType( pMac, packetType);
#endif

    }
#ifdef FIXME_GEN6
    halMacRaGetStaTxRate(pMac, lastValidStaId,
            &currTxRateId,
            &pRsp->stat.txRate,
            NULL,
            NULL,
            &packetType);
    pRsp->stat.txMCSId = HAL_RA_IERATEMCSIDX_GET(currTxRateId);

    pRsp->stat.txPacketType = __halMacStatsConvertPacketType( pMac, packetType);
#endif
    halTable_FindAddrByStaid( pMac, lastValidStaId, pRsp->stat.macAddr );

    pRsp->rc = status;
    halMsg_GenerateRsp( pMac, SIR_HAL_STAT_SUMM_RSP, (tANI_U16) 0, pRsp, 0);

}

void halHandleStatsReq(tpAniSirGlobal pMac, tANI_U16 msgType, tpAniGetStatsReq pMsg)
{

    if( NULL == pMsg )
    {
        HALLOGE( halLog(pMac, LOGE, FL("Statistics request pointer is NULL!!")));
        return;
    }

    switch(msgType)
    {
        case SIR_HAL_STA_STAT_REQ:
            //Send station statistics response.
            __halMacHandlePerStaStatsReq( pMac, pMsg);
            break;
        case SIR_HAL_AGGR_STAT_REQ:
            //Send aggregated statistics response for all stations.
            __halMacHandleAggrStatsReq(pMac, pMsg);
            break;
        case SIR_HAL_GLOBAL_STAT_REQ:
            //Send Global statistics response.
            __halMacHandleGlobalStatsReq(pMac, pMsg);
            break;
        case SIR_HAL_STAT_SUMM_REQ:
            //Send Summary stats response.
            __halMacHandleStatSummaryReq(pMac, pMsg);
            break;
        default:
            HALLOGE( halLog(pMac, LOGE, FL("Invalid stats request\n")));
    }

    //Free the request message.
    palFreeMemory( pMac, pMsg );

}

static void __halMacHandlePESummaryStatsReq( tpAniSirGlobal pMac, tANI_U8 *pBuff, tpTpeStaStats pTpeStaStats, tANI_U16 staId )
{
    //fill out SummaryStats
    tANI_U32 i;
    Qwlanfw_HwCntrType hwCounters;

    tpAniSummaryStatsInfo pSummaryStats = (tpAniSummaryStatsInfo)pBuff;
    tAniSirRABckOffStats raWrappedCount = pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount[staId];

    /** Read the Hw counters.*/
    halReadDeviceMemory(pMac, QWLANFW_MEM_HW_COUNTERS_ADDR_OFFSET,
                (tANI_U8 *)&hwCounters, sizeof(Qwlanfw_HwCntrType));

    //tANI_U32 retry_cnt[4]; ==> pTpeStaStats->txFrmRetryCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->retry_cnt[i] = (raWrappedCount.txFrmRetryCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].txFrmRetryCnt;

    //tANI_U32 multiple_retry_cnt[4]; ==> pTpeStaStats->txFrmMultiRetryCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->multiple_retry_cnt[i] = (raWrappedCount.txFrmMultiRetryCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].txFrmMultiRetryCnt;

    //tANI_U32 tx_frm_cnt[4];  ==> pTpeStaStats->txFrmSuccCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->tx_frm_cnt[i] = (raWrappedCount.txFrmSuccCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].txFrmSuccCnt;

    //tANI_U32 fail_cnt[4];  ==> pTpeStaStats->txFrmFailCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->fail_cnt[i] = (raWrappedCount.txFrmFailCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].txFrmFailCnt;

    //tANI_U32 rx_frm_cnt;
    halReadRegister(pMac, QWLAN_RXP_DMA_SEND_CNT_REG, &(pSummaryStats->rx_frm_cnt));
    pSummaryStats->rx_frm_cnt += hwCounters.uRxp_Dma_Send_Cnt;

    //tANI_U32 frm_dup_cnt;  ==> RPE: rpe_bitmap_duplicate_cntr
    halReadRegister(pMac, QWLAN_RXP_FLT_DUPL_CNT_REG, &(pSummaryStats->frm_dup_cnt));

    //tANI_U32 rts_fail_cnt; ==> pTpeStaStats->rtsFailCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->rts_fail_cnt += (raWrappedCount.rtsFailCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].rtsFailCnt;

    //tANI_U32 ack_fail_cnt; ==> pTpeStaStats->ackFailCnt //16bit
    for(i = 0; i < 4; i++)
        pSummaryStats->ack_fail_cnt += (raWrappedCount.ackFailCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].ackFailCnt;

    //tANI_U32 rts_succ_cnt; ==> pTpeStaStats->rtsSuccCnt //16 bit
    for(i = 0; i < 4; i++)
        pSummaryStats->rts_succ_cnt += (raWrappedCount.rtsSuccCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].rtsSuccCnt;

    //tANI_U32 rx_discard_cnt; ==> RXP: fcs_err_cnt + dma_get_bmu_fail_cnt
    {
        tANI_U32 value;
        halReadRegister(pMac, QWLAN_RXP_FCS_ERR_CNT_REG, &(pSummaryStats->rx_discard_cnt));
        pSummaryStats->rx_discard_cnt += hwCounters.uRxp_Fcs_Err_Cnt;

        halReadRegister(pMac, QWLAN_RXP_DMA_GET_BMU_FAIL_CNT_REG, &value);
        pSummaryStats->rx_discard_cnt += value;
    }

    //tANI_U32 rx_error_cnt; ==> RXP: fcs_err_cnt
    halReadRegister(pMac, QWLAN_RXP_FCS_ERR_CNT_REG, &(pSummaryStats->rx_error_cnt));
    pSummaryStats->rx_error_cnt += hwCounters.uRxp_Fcs_Err_Cnt;

    //tANI_U32 tx_byte_cnt; ==> TPE: unicast_bytes_lower+unicast_bytes_upper
    //                               multicast_bytes_lower+multicast_bytes_upper
    //                               broadcast_bytes_lower+broadcast_bytes_upper
    {
/*
        QWLAN_TPE_UNICAST_BYTES_LOWER_REG;
        QWLAN_TPE_UNICAST_BYTES_UPPER_REG;
        QWLAN_TPE_MULTICAST_BYTES_LOWER_REG;
        QWLAN_TPE_MULTICAST_BYTES_UPPER_REG;
        QWLAN_TPE_BROADCAST_BYTES_LOWER_REG;
        QWLAN_TPE_BROADCAST_BYTES_UPPER_REG;
*/
#if 0
        tANI_U32 value[6];

        halReadRegister(pMac, QWLAN_TPE_UNICAST_BYTES_LOWER_REG, &(value[0]));
        halReadRegister(pMac, QWLAN_TPE_UNICAST_BYTES_UPPER_REG, &(value[1]));
        halReadRegister(pMac, QWLAN_TPE_MULTICAST_BYTES_LOWER_REG, &(value[2]));
        halReadRegister(pMac, QWLAN_TPE_MULTICAST_BYTES_UPPER_REG, &(value[3]));
        halReadRegister(pMac, QWLAN_TPE_BROADCAST_BYTES_LOWER_REG, &(value[4]));
        halReadRegister(pMac, QWLAN_TPE_BROADCAST_BYTES_UPPER_REG, &(value[5]));

        pSummaryStats->tx_byte_cnt = value[0] + value[1] + value[2] + value[3] + value[4] + value[5];
        pSummaryStats->tx_unicast_lower_byte_cnt = value[0];
        pSummaryStats->tx_unicast_upper_byte_cnt = value[1];
        pSummaryStats->tx_multicast_lower_byte_cnt = value[2];
        pSummaryStats->tx_multicast_upper_byte_cnt = value[3];
        pSummaryStats->tx_broadcast_lower_byte_cnt = value[4];
        pSummaryStats->tx_broadcast_upper_byte_cnt = value[5];
#endif

    }
}

static void __halMacHandlePEGlobalClassAStatsReq( tpAniSirGlobal pMac, tANI_U8 *pBuff, tANI_U16 staId )
{
    //fill out GlobalClassAStats

    tpAniGlobalClassAStatsInfo pGlobalClassAStats = (tpAniGlobalClassAStatsInfo)pBuff;
    Qwlanfw_HwCntrType hwCounters;

    /** Read the Hw counters.*/
    halReadDeviceMemory(pMac, QWLANFW_MEM_HW_COUNTERS_ADDR_OFFSET,
                (tANI_U8 *)&hwCounters, sizeof(Qwlanfw_HwCntrType));

    //tANI_U32 rx_frag_cnt;
    {
        tANI_U32 value;
        halReadRegister(pMac, QWLAN_DPU_DPU_FRAG_COUNT_REG, &value);
        pGlobalClassAStats->rx_frag_cnt = ((value & QWLAN_DPU_DPU_FRAG_COUNT_RX_FRAG_CNT_MASK) >>
                                           QWLAN_DPU_DPU_FRAG_COUNT_RX_FRAG_CNT_OFFSET);
    }

    //tANI_U32 promiscuous_rx_frag_cnt; ==> is this same as rx_frm_cnt from above?
    halReadRegister(pMac, QWLAN_RXP_PHY_MPDU_CNT_REG, &(pGlobalClassAStats->promiscuous_rx_frag_cnt));
    pGlobalClassAStats->promiscuous_rx_frag_cnt += hwCounters.uRxp_Phy_Mpdu_Cnt;

    //tANI_U32 rx_input_sensitivity;

    //tANI_U32 max_pwr;
    {
        tANI_U32 tbidx;
        tpTpeStaDescRateInfo pTpeRateInfo;
        halTpe_GetStaDescRateInfo(pMac, staId, TPE_STA_20MHZ_RATE, &pTpeRateInfo);
        pGlobalClassAStats->tx_rate_flags = 0;
        tbidx = (tANI_U32)pTpeRateInfo->rate_index;
        if(HALRATE_IS_LEGACY(tbidx) || HALRATE_IS_11AG(tbidx))
        {
            pGlobalClassAStats->tx_rate_flags |= eHAL_TX_RATE_LEGACY;
        }
        else if(HALRATE_IS_HT20(tbidx))
        {
            pGlobalClassAStats->mcs_index =  HAL_RA_IERATEMCSIDX_GET(tbidx);
            pGlobalClassAStats->tx_rate_flags |= eHAL_TX_RATE_HT20;
        }
        else if(HALRATE_IS_HT20_SGI(tbidx))
        {
            pGlobalClassAStats->mcs_index =  HAL_RA_IERATEMCSIDX_GET(tbidx);
            pGlobalClassAStats->tx_rate_flags |= eHAL_TX_RATE_HT20;
            pGlobalClassAStats->tx_rate_flags |= eHAL_TX_RATE_SGI;
        }

        //Transmit rate, in units of 500 kbit/sec, for the most recently transmitted frame
        pGlobalClassAStats->tx_rate =  (gHalRateInfo[pTpeRateInfo->rate_index].thruputKbps) / 5;

        //The maximum transmit power in dBm. upto one decimal. for eg: if it is 10.5dBm, the value would be 105
        pGlobalClassAStats->max_pwr = (pTpeRateInfo->tx_power + 16) * 5;
    }

    //tANI_U32 sync_fail_cnt;
    {
        tANI_U32 value[3];
        halReadRegister(pMac, QWLAN_RBAPB_COUNT_SSFD_REG, &(value[0]));
        halReadRegister(pMac, QWLAN_RBAPB_COUNT_LSFD_REG, &(value[1]));
        halReadRegister(pMac, QWLAN_RBAPB_COUNT_PLCP_CRC_ERRORS_REG, &(value[2]));
        pGlobalClassAStats->sync_fail_cnt = value[0] + value[1] + value[2];
    }

}

static void __halMacFillDpuStats( tpAniSirGlobal pMac, tpAniGlobalSecurityStats pStats, tANI_U8 dpuIdx, tpAniSirDpuStats pDpuWrappedCount )
{
    tANI_U32 address;
    tDpuDescriptor dpuDesc;

    palFillMemory(pMac->hHdd, (void *)&dpuDesc, sizeof(tDpuDescriptor), 0);

    address = pMac->hal.memMap.dpuDescriptor_offset + (dpuIdx * sizeof(tDpuDescriptor));
    halReadDeviceMemory(pMac, address, (tANI_U8 *)&dpuDesc,
                             sizeof(tDpuDescriptor));

    //tANI_U32 rx_wep_unencrypted_frm_cnt; ==>tDpuDescriptor.excludedCount
    pStats->rx_wep_unencrypted_frm_cnt = dpuDesc.excludedCount;

    //tANI_U32 rx_mic_fail_cnt; ==>tDpuDescriptor.micErrCount
    pStats->rx_mic_fail_cnt = (pDpuWrappedCount->micErrCount << 8) + dpuDesc.micErrCount;

    //tANI_U32 tkip_icv_err;   ==>tDpuDescriptor.extIVerror
    pStats->tkip_icv_err = (pDpuWrappedCount->micErrCount << 8) + dpuDesc.extIVerror;

    //tANI_U32 aes_ccmp_format_err;  ==>tDpuDescriptor.formatErrorCount
    pStats->aes_ccmp_format_err = (pDpuWrappedCount->micErrCount << 16) + dpuDesc.formatErrorCount;

    //tANI_U32 aes_ccmp_replay_cnt;    ==>tDpuDescriptor.replayCheckFailCount
    pStats->aes_ccmp_replay_cnt = dpuDesc.replayCheckFailCount;

    //tANI_U32 aes_ccmp_decrpt_err;    ==>tDpuDescriptor.decryptErrorCount
    pStats->aes_ccmp_decrpt_err = dpuDesc.decryptErrorCount;

    //tANI_U32 wep_undecryptable_cnt;  ==>tDpuDescriptor.undecryptableCount
    pStats->wep_undecryptable_cnt = (pDpuWrappedCount->micErrCount << 16) + dpuDesc.undecryptableCount;

    //tANI_U32 wep_icv_err;            ==>tDpuDescriptor.extIVerror
    pStats->wep_icv_err = (pDpuWrappedCount->micErrCount << 8) + dpuDesc.extIVerror;

    //tANI_U32 rx_decrypt_succ_cnt;   ==>tDpuDescriptor.decryptSuccessCount
    pStats->rx_decrypt_succ_cnt = dpuDesc.decryptSuccessCount;

    //tANI_U32 rx_decrypt_fail_cnt;   ==>tDpuDescriptor.decryptErrorCount
    pStats->rx_decrypt_fail_cnt = dpuDesc.decryptErrorCount;

}

static void __halMacHandlePEGlobalClassBStatsReq( tpAniSirGlobal pMac, tANI_U8 *pBuff, tANI_U8 staId )
{
    //fill out GlobalClassBStats
    tANI_U8 dpuIdx, dpuBcIdx;
    tpAniGlobalClassBStatsInfo pGlobalClassBStats = (tpAniGlobalClassBStatsInfo)pBuff;
    tpAniSirDpuStats pDpuWrappedCount;

    if (halTable_GetStaDpuIdx(pMac, staId, &dpuIdx) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW(halLog( pMac, LOGW, FL("Cannot get the DPU index, STA index %d\n"), staId ));
        return;
    }

    pDpuWrappedCount = &(pMac->hal.halMac.wrapStats.pDpuWrappedCount[dpuIdx]);

    __halMacFillDpuStats( pMac, &(pGlobalClassBStats->ucStats), dpuIdx, pDpuWrappedCount );

    if (halTable_GetStaBcastDpuIdx(pMac, staId, &dpuBcIdx) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW(halLog( pMac, LOGW, FL("Cannot get the DPU index, STA index %d\n"), staId ));
        return;
    }

    pDpuWrappedCount = &(pMac->hal.halMac.wrapStats.pDpuWrappedCount[dpuBcIdx]);

    __halMacFillDpuStats( pMac, &(pGlobalClassBStats->mcbcStats), dpuBcIdx, pDpuWrappedCount );
}

static void __halMacHandlePEGlobalClassCStatsReq( tpAniSirGlobal pMac, tANI_U8 *pBuff, tpTpeStaStats pTpeStaStats, tANI_U16 staId )
{
    //fill out GlobalClassCStats
    tpAniGlobalClassCStatsInfo pGlobalClassCStats = (tpAniGlobalClassCStatsInfo)pBuff;
    tAniSirRABckOffStats raWrappedCount = pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount[staId];
    Qwlanfw_HwCntrType hwCounters;

    /** Read the Hw counters.*/
    halReadDeviceMemory(pMac, QWLANFW_MEM_HW_COUNTERS_ADDR_OFFSET,
                (tANI_U8 *)&hwCounters, sizeof(Qwlanfw_HwCntrType));

    //tANI_U32 rx_amsdu_cnt;
    {
        tANI_U32 value;
        halReadRegister(pMac, QWLAN_ADU_ADU_COUNTERS1_REG, &value);
        value += hwCounters.uAdu_Adu_Counters1;
        pGlobalClassCStats->rx_amsdu_cnt = ((value & QWLAN_ADU_ADU_COUNTERS1_NUMBER_OF_AMSDU_FRAMES_PROCESSED_MASK) >>
                                           QWLAN_ADU_ADU_COUNTERS1_NUMBER_OF_AMSDU_FRAMES_PROCESSED_OFFSET);
    }

    //tANI_U32 rx_ampdu_cnt;
    halReadRegister(pMac, QWLAN_RXP_PHY_AMPDU_CNT_REG, &(pGlobalClassCStats->rx_ampdu_cnt));

    //tANI_U32 tx_20_frm_cnt;    ==>pTpeStaStats->raStats.totTxPpduDataFrms1
    pGlobalClassCStats->tx_20_frm_cnt += (raWrappedCount.tot20MTxPpduDataFrms1 << 16) + pTpeStaStats->raStats.tot20MTxPpduDataFrms1 +
                                         (raWrappedCount.tot20MTxPpduDataFrms2 << 16) + pTpeStaStats->raStats.tot20MTxPpduDataFrms2 +
                                         (raWrappedCount.tot20MTxPpduDataFrms3 << 16) + pTpeStaStats->raStats.tot20MTxPpduDataFrms3;
    //tANI_U32 rx_20_frm_cnt;
    halReadRegister(pMac, QWLAN_RXP_PHY_MPDU_CNT_REG, &(pGlobalClassCStats->rx_20_frm_cnt));
    pGlobalClassCStats->rx_20_frm_cnt += hwCounters.uRxp_Phy_Mpdu_Cnt;

    //tANI_U32 rx_mpdu_in_ampdu_cnt;
    halReadRegister(pMac, QWLAN_RXP_MPDU_IN_AMPDU_CNT_REG, &(pGlobalClassCStats->rx_mpdu_in_ampdu_cnt));

    //tANI_U32 ampdu_delimiter_crc_err;
    halReadRegister(pMac, QWLAN_RXP_DLM_ERR_CNT_REG, &(pGlobalClassCStats->ampdu_delimiter_crc_err));

}

static void __halMacHandlePEPerStaStatsReq( tpAniSirGlobal pMac, tANI_U8 *pBuff, tpTpeStaStats pTpeStaStats, tANI_U16 staId )
{
    //fill out PerStaStats
    tANI_U32 i;
    tpAniPerStaStatsInfo pPerStaStats = (tpAniPerStaStatsInfo)pBuff;
    tAniSirRABckOffStats raWrappedCount = pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount[staId];

    //tANI_U32 tx_frag_cnt[4]; ==>pTpeStaStats->txFragCnt //16bit
    for(i = 0; i < 4; i++)
        pPerStaStats->tx_frag_cnt[i] = (raWrappedCount.txFragCnt[i] << 16) + pTpeStaStats->dot11Stats[7-i].txFragCnt;

    //tANI_U32 tx_ampdu_cnt;   ==>pTpeStaStats->raStats.totTxMpduDataFrms1
    pPerStaStats->tx_ampdu_cnt += (raWrappedCount.tot20MTxMpduDataFrms1 << 16) + pTpeStaStats->raStats.tot20MTxMpduDataFrms1 +
                                  (raWrappedCount.tot20MTxMpduDataFrms2 << 16) + pTpeStaStats->raStats.tot20MTxMpduDataFrms2 +
                                  (raWrappedCount.tot20MTxMpduDataFrms3 << 16) + pTpeStaStats->raStats.tot20MTxMpduDataFrms3;

    //tANI_U32 tx_mpdu_in_ampdu_cnt; ==>pTpeStaStats->raStats.totMpduInAmPdu1
    pPerStaStats->tx_mpdu_in_ampdu_cnt += (raWrappedCount.tot20MMpduInAmPdu1 << 16) + pTpeStaStats->raStats.tot20MMpduInAmPdu1 +
                                  (raWrappedCount.tot20MMpduInAmPdu2 << 16) + pTpeStaStats->raStats.tot20MMpduInAmPdu2 +
                                  (raWrappedCount.tot20MMpduInAmPdu3 << 16) + pTpeStaStats->raStats.tot20MMpduInAmPdu3;
}

/** -------------------------------------------------------------
\fn halHandlePEStatisticsReq
\brief  This function fetches the stats based on statsMask that is part of the message.
\param   tHalHandle hHal
\param   msgType
\param   tpAniGetPEStatsReq pMsg
\return  void
  -------------------------------------------------------------*/
void halHandlePEStatisticsReq(tpAniSirGlobal pMac, tANI_U16 msgType, tpAniGetPEStatsReq pMsg)
{
    tANI_U32 statsMask;
    tANI_U32 statsSize = 0;
    tANI_U8 *pStatsBuff;
    tpAniGetPEStatsRsp pRsp;
    tANI_U8 staId;
    tTpeStaStats tpeStaStats;
    tpTpeStaStats pTpeStaStats = &tpeStaStats;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8   psState = 0;

     /** Don't get the stats if the device is in a Idle mode power save.*/
    psState = halPS_GetState(pMac);
    if((psState == HAL_PWR_SAVE_IMPS_STATE) || (psState == HAL_PWR_SAVE_IMPS_REQUESTED)) {
        //Free the request message.
        palFreeMemory( pMac, pMsg );
        return;
    }

    if( NULL == pMsg )
    {
        HALLOGE(halLog(pMac, LOGE, FL("Statistics request pointer is NULL!!")));
        return;
    }
    statsMask = pMsg->statsMask;
    staId = (tANI_U8)(pMsg->staId);

    if(statsMask & PE_SUMMARY_STATS_INFO)
    {
        statsSize += sizeof(tAniSummaryStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_A_STATS_INFO)
    {
        statsSize += sizeof(tAniGlobalClassAStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_B_STATS_INFO)
    {
        statsSize += sizeof(tAniGlobalClassBStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_C_STATS_INFO)
    {
        statsSize += sizeof(tAniGlobalClassCStatsInfo);
    }
    if(statsMask & PE_PER_STA_STATS_INFO)
    {
        statsSize += sizeof(tAniPerStaStatsInfo);
    }

    //Free the request message.
    palFreeMemory( pMac, pMsg );

    if(statsSize != 0)
    {
        //allocate memory for total size of stats and sizeof(tAniGetPEStatsRsp)
        if( eHAL_STATUS_SUCCESS != (status = palAllocateMemory (pMac->hHdd, (void**) &pStatsBuff, (statsSize + sizeof(tAniGetPEStatsRsp)))))
        {
            HALLOGE(halLog(pMac,LOGE, FL("Unable to allocate memory")));
            return;
        }

        palZeroMemory( pMac, pStatsBuff, (statsSize + sizeof(tAniGetPEStatsRsp)));
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid stats request\n")));
        return;
    }

    pRsp = (tpAniGetPEStatsRsp) pStatsBuff;
    pRsp->msgType = SIR_HAL_GET_STATISTICS_RSP;
    pRsp->msgLen = (tANI_U16)(statsSize + sizeof(tAniGetPEStatsRsp));
    pRsp->statsMask = statsMask;
    pRsp->staId = staId;

    //stat collection start
    pStatsBuff += sizeof(tAniGetPEStatsRsp);

    //read the tpe sta stats
    {
        tANI_U32    address;

        address = pMac->hal.memMap.tpeStaDesc_offset + (staId * TPE_STA_DESC_AND_STATS_SIZE) + TPE_PER_STA_STATS_START_OFFSET;
        if (halReadDeviceMemory(pMac, address, (tANI_U8 *)pTpeStaStats,
                            sizeof(tTpeStaStats)) != eHAL_STATUS_SUCCESS) {
            return;
        }
    }

    if(statsMask & PE_SUMMARY_STATS_INFO)
    {
        __halMacHandlePESummaryStatsReq( pMac, pStatsBuff, pTpeStaStats, staId);
        pStatsBuff += sizeof(tAniSummaryStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_A_STATS_INFO)
    {
        __halMacHandlePEGlobalClassAStatsReq( pMac, pStatsBuff, staId);
        pStatsBuff += sizeof(tAniGlobalClassAStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_B_STATS_INFO)
    {
        __halMacHandlePEGlobalClassBStatsReq( pMac, pStatsBuff, staId);
        pStatsBuff += sizeof(tAniGlobalClassBStatsInfo);
    }
    if(statsMask & PE_GLOBAL_CLASS_C_STATS_INFO)
    {
        __halMacHandlePEGlobalClassCStatsReq( pMac, pStatsBuff, pTpeStaStats, staId);
        pStatsBuff += sizeof(tAniGlobalClassCStatsInfo);
    }
    if(statsMask & PE_PER_STA_STATS_INFO)
    {
        __halMacHandlePEPerStaStatsReq( pMac, pStatsBuff, pTpeStaStats, staId);
    }

    pRsp->rc  = status;

    halMsg_GenerateRsp( pMac, SIR_HAL_GET_STATISTICS_RSP, (tANI_U16) 0, pRsp, 0);

}

/** -------------------------------------------------------------
\fn halMacClearDpuStats
\brief  This function clears the DPU stats wrapped around counters
\param   tHalHandle hHal
\param   tANI_U8 id ==> dpuIdx
\return  void
  -------------------------------------------------------------*/
void halMacClearDpuStats(tpAniSirGlobal pMac, tANI_U8 id)
{
    tAniSirDpuStats dpuWrappedCount;
    tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

    if(pDpu->descTable[id].used == 0)
        return;

    dpuWrappedCount = pMac->hal.halMac.wrapStats.pDpuWrappedCount[id];
    palZeroMemory( pMac->hHdd, (void *) &dpuWrappedCount, sizeof(tAniSirDpuStats) );

}

/** -------------------------------------------------------------
\fn halMacClearStaStats
\brief  This function clears the sta stats wrapped around counters
\param   tHalHandle hHal
\param   staId
\return  void
  -------------------------------------------------------------*/
void halMacClearStaStats(tpAniSirGlobal pMac, tANI_U8 staId)
{
    tAniSirRABckOffStats prevRAStats, raWrappedCount;
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;

    if (!pSta[staId].valid)
    {
        return;
    }

    prevRAStats = pMac->hal.halMac.wrapStats.pPrevRaBckOffStats[staId];
    raWrappedCount = pMac->hal.halMac.wrapStats.pRaBckOffWrappedCount[staId];

    palZeroMemory( pMac->hHdd, (void *) &raWrappedCount, sizeof(tAniSirRABckOffStats) );
    palZeroMemory( pMac->hHdd, (void *) &prevRAStats, sizeof(tAniSirRABckOffStats) );
}

