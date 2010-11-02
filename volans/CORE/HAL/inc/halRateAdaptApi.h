/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * halRateAdaptApi.h: Provides all the MAC driver APIs in this file.
 * Author:    Viji Alagarsamy
 * Date:      03/07/2005
 * History:-
 * Date        Modified by            Modification Information
 * 09/14/2009  Hoonki Lee             Rate Adaptation to firmware
 * --------------------------------------------------------------------------
 *
 */

#ifndef __HAL_RATE_ADAPT_API_H
#define __HAL_RATE_ADAPT_API_H

#include "aniGlobal.h"
#include "halFw.h" /* ALIGN4 is defined to be used both host and target buid environment */

/* Per station rateSelect. Each STA is configured with three pre selected 
 * Tx rates by HAL after rate adaptation.
 */
#define HAL_RA_MAX_RATES           3

// Libra 1.0 HW bug:  CR-0000141920
// SW Workaround for MPDU Ack counters are not incremented after the first retry
#define LIBRA1_0_RA_STATS_HW_BUG_PRIM_RETRY_THRESHOLD   1
#define LIBRA1_0_RA_STATS_HW_BUG_SECD_RETRY_THRESHOLD   3
#define LIBRA1_0_RA_STATS_HW_BUG_TERT_RETRY_THRESHOLD   5

#define HALRATE_CONVERT_BY_TYPE_OFFSET(oldType, newType, halRate)\
    ((tHalMacRate)(((halRate) - HALRATE_ ## oldType ## _START) + HALRATE_ ## newType ## _START))


/*-- flavours of supported rates
 * valid types are
    11B
    SPREAM_11B
    DUP_11B
    SPREAM_DUP_11B
    11A
    DUP_11A
    MIMO
    CB
    MIMO_CB
    HT_SIMO
    HT_SIMO_SGI
    HT_MIMO
    HT_MIMO_SGI
    HT_SIMO_CB
    HT_SIMO_CB_SGI
    HT_MIMO_CB
    HT_MIMO_CB_SGI
 * example use: HAL_RATE_IS_TYPE(MIMO_CB, halRate)
 */


#define HALRATE_IS_TYPE(type, halRate)\
    (((halRate) <= HALRATE_ ## type ## _END) /*&& ((halRate) >= HALRATE_ ## type ## _START)*/)


/* -------------------------------------------------------------------------- */
/* rate adaptation private information */
#define HAL_RAINFO_PTR_GET(p) &(p->halRaInfo)

/*
 * supported modes for possible rates
 */
typedef struct sHalRaModeCfg {
    tANI_U8  channelBondingEnable:1;  /* 0 => CB rates not allowed */
    tANI_U8  mimoEnable:1;            /* 0 => mimo rates not allowed */
    tANI_U8  sgiEnable:1;             /* 0 => Short GI rates not allowed */
    tANI_U8  sgiEnable40:1;             /* 0 => Short GI rates not allowed */
    tANI_U8  shortPreambleEnable:1;   /* 0 => Short preamble not allowed */
} tHalRaModeCfg, *tpHalRaModeCfg;

/* ----------------------------------------------------------------------------
 * API functions available
 */

tSirRetStatus
halMacRaUpdateStaSuppRateBitmap_PeerType(
    tpAniSirGlobal          pMac,
    tANI_U16                     staid,
    tpSirSupportedRates   pRates);

extern void
halMacRaCfgChange(
    tpAniSirGlobal          pMac,
    tANI_U32                     cfgId);

extern tHalMacRate
halGetCurrentRate(
    tpAniSirGlobal          pMac,
    tANI_U16                     staid);

extern tSirRetStatus
halConvertIeToMacRate(
    tpAniSirGlobal          pMac,
    tANI_U16                     ieRate,
    tpHalMacRate            pMacRate);

extern tSirRetStatus
halMacRaStaInit(
    tpAniSirGlobal          pMac,
    tANI_U16                staid,
    tpSirSupportedRates   pRates,
    tpHalRaModeCfg          pMode
);

extern tSirRetStatus halMacRaStaAdd(
    tpAniSirGlobal      pMac,
    tANI_U16            staid,
    tANI_U8             staType);

extern tSirRetStatus halMacRaStaDel(
    tpAniSirGlobal      pMac,
    tANI_U16            staid);

extern tSirRetStatus halMacRaStaCapModeUpdate(
    tpAniSirGlobal  pMac,
    tANI_U16             staid,
    tpHalRaModeCfg  pMode);

extern tSirRetStatus
halMacRaStaModeGet(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tpHalRaModeCfg  pMode,
    tRateAdaptMode  *pRateAdaptMode);

extern tSirRetStatus
halMacRaSetAllStaRetryRates(
    tpAniSirGlobal  pMac,
    tHalRetryMode   rMode, /* retry mode */
    tHalMacRate     sRate, /* specific rate to use (depends on retry mode) */
    tHalMacRate     tRate); /* tertiary retry rate */

extern tSirRetStatus
halMacRaSetStaRetryMode(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tHalRetryMode   rMode, /* retry mode */
    tHalMacRate     sRate, /* specific rate to use (depends on retry mode) */
    tHalMacRate     tRate); /* tertiary retry rate */

extern eHalStatus
halMacRaGetStaTxRate(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tHalMacRate    *curTxRateIdx,
    tANI_U32        *curTxRate100Kbps,
    tHalMacRate     *maxTxRateIdx,
    tANI_U32        *maxTxRate100Kbps,
    tANI_U8         *pPacketType);


eHalStatus
halMacRaGetStaTxCount(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tANI_U32        *curTxAckPktCount);

eHalStatus
halMacRaUpdateStaTxCount(
    tpAniSirGlobal  pMac,
    tANI_U16        staid,
    tANI_U32        txAckPktCount);


void
halSetFixedRateForAllStaByCfg(
    tpAniSirGlobal  pMac);

eHalStatus
halRateUpdateStaRateInfo(
    tpAniSirGlobal pMac, 
    tANI_U32 staIdx);

eHalStatus
halMacRaStart(
    tHalHandle hHal, 
    void *arg);

eHalStatus
halMacRaStop(
    tHalHandle hHal, 
    void *arg);

eHalStatus
halMacRaSetGlobalCfg(
    tpAniSirGlobal pMac, 
    tANI_U32 id, 
    tANI_U32 value1, 
    tANI_U32 value2);

eHalStatus
halMacRaStaSetCfg(
    tpAniSirGlobal  pMac,
    tANI_U32 staid, 
    tANI_U32 id, 
    tANI_U32 value);

eHalStatus
halRateAdaptStart(
    tpAniSirGlobal pMac);

eHalStatus
halRateAdaptStop(
    tpAniSirGlobal pMac);

eHalStatus
halMacRaSetRAPeriodicity(
    tpAniSirGlobal  pMac,
    tANI_U32 raPeriod);

tHalMacRate
halMacRaConvert2DupRate(
    tHalMacRate halRate);

void
halMacRaDumpStats(
    tpAniSirGlobal pMac, 
    tANI_U32 staid);

eHalStatus
halMacRaBssInfoToFW(
    tpAniSirGlobal pMac, 
    tpHalRaBssInfo pHalRaBssInfo, 
    tANI_U8 bssIdx);

eHalStatus
halMacRaGlobalInfoToFW(
    tpAniSirGlobal pMac, 
    tpHalRaGlobalInfo pGlobInfo, 
    tANI_U32 startOffset, 
    tANI_U32 szLen);

eHalStatus
halMacRaTxPktCountFromFW(
    tpAniSirGlobal  pMac, 
    tANI_U32 *pktCntArray);

#ifndef FEATURE_RA_CHANGE
eHalStatus
halMacRaAddBssReq(
    tpAniSirGlobal pMac, 
    tANI_U8 bssIdx, 
    tANI_U8 selfStaIdx);

eHalStatus
halMacRaDelBssReq(
    tpAniSirGlobal pMac, 
    tANI_U8 bssIdx);

eHalStatus
halMacRaAddStaReq(
    tpAniSirGlobal pMac, 
    tANI_U32 staid, 
    tANI_U8 staType);

eHalStatus
halMacRaDelStaReq(
    tpAniSirGlobal pMac, 
    tANI_U32 staid);
#endif

eHalStatus
halMacRaUpdateParamReq(
    tpAniSirGlobal pMac, 
    tANI_U32 bmCode, 
    tANI_U32 paramSpecific);

eHalStatus
halMacRaUpdateReq(
    tpAniSirGlobal pMac, 
    tFwRaUpdateEnum raUpdateEvent, 
    tANI_U16 msgLen, 
    tANI_U8 *msgBody);

/* ---------------------------------------------------------------------------*/

#endif /* __HAL_RATE_ADAPT_API_H */
