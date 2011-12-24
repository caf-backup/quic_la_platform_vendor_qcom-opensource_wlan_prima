/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMacUtilsApi.h This is the header file for the hal mac utils
 *
 * Author:      Viji Alagarsamy
 * Date:        04/07/05
 * History:-
 * --------------------------------------------------------------------
 *
 */

#ifndef __HAL_MAC_UTILS_API_H__
#define __HAL_MAC_UTILS_API_H__

#include <sirCommon.h>
#include "aniGlobal.h"
//#include "halApi.h"

// Offset between Titan "Hardware Units" & true dBm.  This was
// determined empirically 10.26.2005 by James & Adib.
#define HAL_HW_RSSI_OFFSET   41


// After *much* discussion, it was decided to normalize to the
// range [-200, -10]
#define HAL_MIN_RSSI               ( -60 )
#define HAL_MAX_RSSI               ( 130 )

#define HAL_NOISE_INVALID          0

/// Maximum times noise register is read before returning invalid
#define HAL_MAX_NOISE_ITER         5

/// Default value of noise floor
#define HAL_DEFAULT_NOISE          25

/// Number of extra precision bits used in maintaining average
#define HAL_EXTRA_PRECISION_BITS   8


//RSSI in dBm = rssi_stat – max_gain_db – 41
#define HAL_CONVERT_HW_RSSI_UNIT_2_DB(rssi, maxgaindb) ((rssi) - maxgaindb - HAL_HW_RSSI_OFFSET)


/// Convert percent to hardware unit
#define HAL_CONVERT_PERCENT_TO_HW_UNIT(rssi) (HAL_MIN_RSSI + (rssi) * (HAL_MAX_RSSI - HAL_MIN_RSSI) / 100)

/// Convert hardware unit to polaris
#define HAL_CONVERT_HW_UNIT_2_PERCENT(rssi) (((rssi) > HAL_MAX_RSSI) ? 100: (\
                 ((((rssi)-HAL_MIN_RSSI)*100 + (HAL_MAX_RSSI-HAL_MIN_RSSI-1))/(HAL_MAX_RSSI-HAL_MIN_RSSI))))

/// Convert percent to dB
#define HAL_CONVERT_PERCENT_TO_DB(rssi) (HAL_CONVERT_HW_RSSI_UNIT_2_DB(HAL_CONVERT_PERCENT_TO_HW_UNIT((rssi))))


/// Compute max of 3 unsigned values
#define HAL_MAX_OF_THREE_UINT(val0, val1, val2) (((val0) > (val1)) ? (((val0) > (val2)) ? (val0) : (val2)) : (((val1) > (val2)) ? (val1) : (val2)))




#ifdef GEN6_FIXME

// -----------------------------------------------------------------------
// Following are for external use
// -----------------------------------------------------------------------

/// Get RSSI from BD in polaris units
static inline tANI_U8 halGetRssi(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
#if 0
    tANI_U8 rssi[3];

    rssi[0] = (tANI_U8) (pBd->phyStatsLo) & 0xff;
    rssi[1] = (tANI_U8) (pBd->phyStatsLo >> 8) & 0xff;
    rssi[2] = (tANI_U8) (pBd->phyStatsLo >> 16) & 0xff;

    return ((tANI_U8) HAL_MAX_OF_THREE_UINT(rssi[0], rssi[1], rssi[2]));
#else
    return 0;
#endif
}

/// Get RSSI from BD in dB
static inline tANI_S8 halGetRssiDb(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_HW_UNIT_2_DB(halGetRssi(pMac, pBd));
#else
    return 0;
#endif /* if 0 */
}

/// Get RSSI from BD in percentage
static inline tANI_U8 halGetRssiPercent(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_HW_UNIT_2_PERCENT(halGetRssi(pMac, pBd));
#else
    return 0;
#endif /* if 0 */
}

/// Get current noise floor in polaris units - return invalid if not read
static inline tANI_U8 halGetNoiseRaw(tpAniSirGlobal pMac, tANI_U32 attempts)
{
// FIXME
#if 0 // Titan BD specific operation
    tANI_U8 noise[3];
    tANI_U32 iter = 0;

    do
    {
        if (iter++ >= attempts)
            return HAL_NOISE_INVALID;
        asicSetPmuPhy1Enable(pMac, PMU_AGC_CLK);
        noise[0] = (tANI_U8) halGetReg(pMac, TIT_AGC_RSSI0_REG);
        noise[1] = (tANI_U8) halGetReg(pMac, TIT_AGC_RSSI1_REG);
        noise[2] = (tANI_U8) halGetReg(pMac, TIT_AGC_RSSI2_REG);
        asicSetPmuPhy1Disable(pMac, PMU_AGC_CLK);
    }
    while (noise[0] >> 7 || noise[1] >> 7 || noise[2] >> 7);

    return ((tANI_U8) HAL_MAX_OF_THREE_UINT(noise[0], noise[1], noise[2]));
#else 
    return 0;
#endif /* if 0 */
}

/// Get current noise floor in polaris units
static inline tANI_U8 halGetNoise(tpAniSirGlobal pMac)
{
// FIXME
#if 0 // Titan BD specific operation
    tANI_U8 noise = halGetNoiseRaw(pMac, HAL_MAX_NOISE_ITER);

    return (noise == HAL_NOISE_INVALID ? HAL_DEFAULT_NOISE : noise);
#else
    return 0;
#endif /* if 0 */
}

/// Get current noise floor in dB
static inline tANI_S8 halGetNoiseDb(tpAniSirGlobal pMac)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_HW_UNIT_2_DB(halGetNoise(pMac));
#else
    return 0;
#endif /* if 0 */
}

/// Get SNR from BD
static inline tANI_U8 halGetSnr(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
// FIXME
#if 0 // Titan BD specific operation
    tANI_U8 noise = halGetNoise(pMac);
    tANI_U8 rssi = halGetRssi(pMac, pBd);

    return((rssi > noise) ? (rssi - noise) : 0);
#else
    return 0;
#endif /* if 0 */
}

/// Get SNR from BD in percentage
static inline tANI_U8 halGetSinrPercent(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_SNR_2_PERCENT(halGetSnr(pMac, pBd));
#else
    return 0;
#endif /* if 0 */
}

/// Get signal quality from BD
static inline tANI_U8 halGetSignalQuality(tpAniSirGlobal pMac, tpHalBufDesc pBd)
{
// FIXME
#if 0 // Titan BD specific operation
    return((tANI_U8) ((pBd->phyStatsLo >> 28) & 0xf) |
           (tANI_U8) ((pBd->phyStatsHi & 0xf) << 4));
#else
    return 0;
#endif /* if 0 */
}

/*
 * Usage of exponential averaging macros
 * -------------------------------------
 * Define the average value as tANI_U32, initialize it to 0 (IMPORTANT)
 *
 * Each time you get a new RSSI value, call the macro
 * halExpAvg[/Db/Percent] depending on whether the new value is
 * in polaris units, dB or percentage respectively
 *
 * To report the RSSI avg, call the macro halConvertAvg[/Db/Percent]
 * dependign on whether it needs to be reported in polaris units,
 * dB or percentage respectively
 *
 */

/// Update exponential average with newVal in dB
static inline tANI_U32 halExpAvgDb(tANI_S8 newVal, tANI_U32 oldAvg, tANI_U8 alpha)
{
// FIXME
#if 0 // Titan BD specific operation
    return halExpAvg(HAL_CONVERT_DB_2_HW_UNIT(newVal), oldAvg, alpha);
#else
    return 0;
#endif /* if 0 */
}

/// Update exponential average with newVal in percent
static inline tANI_U32 halExpAvgPercent(tANI_U8 newVal, tANI_U32 oldAvg, tANI_U8 alpha)
{
// FIXME
#if 0 // Titan BD specific operation
    return halExpAvg((tANI_U8)HAL_CONVERT_PERCENT_TO_HW_UNIT(newVal), oldAvg, alpha);
#else
    return 0;
#endif /* if 0 */
}

/// Convert avg value to reportable RSSI in polaris units
static inline tANI_U8 halConvertAvg(tANI_U32 avg)
{
// FIXME
#if 0 // Titan BD specific operation
    return((tANI_U8) (avg >> HAL_EXTRA_PRECISION_BITS));
#else
    return 0;
#endif /* if 0 */
}

/// Convert avg value to reportable RSSI in dB
static inline tANI_S8 halConvertAvgDb(tANI_U32 avg)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_HW_UNIT_2_DB(halConvertAvg(avg));
#else
    return 0;
#endif /* if 0 */
}

/// Convert avg value to reportable RSSI in percent
static inline tANI_U8 halConvertAvgPercent(tANI_U32 avg)
{
// FIXME
#if 0 // Titan BD specific operation
    return HAL_CONVERT_HW_UNIT_2_PERCENT(halConvertAvg(avg));
#else
    return 0;
#endif /* if 0 */
}

/// Compute percentage utilization
static inline tANI_U8 halComputeUtilization(tANI_U32 oldTsf, tANI_U32 newTsf, tANI_U32 oldBusy, tANI_U32 newBusy)
{
// FIXME
#if 0 // Titan BD specific operation
    tANI_U32 diffTsf = newTsf - oldTsf;
    tANI_U32 diffBusy = newBusy - oldBusy;

    return((tANI_U8) (diffBusy * 100 / diffTsf));
#else
    return 0;
#endif /* if 0 */
}

#endif

/**
 * Just for the successful compilation of the code
 */
static inline tANI_U8 halGetNoise(tpAniSirGlobal pMac)
{
    return 0;
}

void halGetTxTSFtimer(tpAniSirGlobal pMac, tSirMacTimeStamp *pTime);

#endif
