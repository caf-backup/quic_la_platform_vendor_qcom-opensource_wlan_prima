/**
 *
 *  @file:         halMtu.h
 *
 *  @brief:        Header file for the MTU Hardware Block.
 *
 *  @author:        Susan Tsao
 *
 *  Copyright (C) 2002 - 2007, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/09/2007  File created.
 * 11/27/2007  Virgo related changes.
 */
#ifndef _HALMTU_H_
#define _HALMTU_H_


#include "halTypes.h"
#include "palTypes.h"
#include "aniGlobal.h"


// FPGA with Virgo only for IBSS testing 0x7.   For  final setting this need to be 0x1f on the chip.
#ifdef VOLANS_FPGA
#define HAL_IBSS_CW_LIMIT 0x7 
#else
#define HAL_IBSS_CW_LIMIT 0x1f 
#endif

#define HAL_IBSS_CW_LIMIT_NON11b 0x1f 
#define HAL_IBSS_CW_LIMIT_11b    0x3f 

 /* Timer control : 0 <= n <= 11 for UP bit, 0 <= n <= 7 for UNIT mask */
#define MTU_TIMER_CONTROL_UP_BIT_N(n)       (n)
#define MTU_TIMER_CONTROL_UP_N(n)           (1u << MTU_TIMER_CONTROL_UP_BIT_N(n))
#define MTU_TIMER_CONTROL_UNIT_BIT_N(n)     ((n) * 2 + 16)
#define MTU_TIMER_CONTROL_UNIT_BIT_N_HI(n)  (((n)-8) * 2)

#define MTU_TIMER_CONTROL_STOP_MASK(n)      (3u << MTU_TIMER_CONTROL_UNIT_BIT_N(n))
#define MTU_TIMER_CONTROL_STOP_MASK_HI(n)      (3u << MTU_TIMER_CONTROL_UNIT_BIT_N_HI(n))

#define MTU_TIMER_CONTROL_UNIT_MASK(n)      (3u << MTU_TIMER_CONTROL_UNIT_BIT_N(n))
#define MTU_TIMER_CONTROL_UNIT_CLK(n)       (1u << MTU_TIMER_CONTROL_UNIT_BIT_N(n))
#define MTU_TIMER_CONTROL_UNIT_USEC(n)      (2u << MTU_TIMER_CONTROL_UNIT_BIT_N(n))
#define MTU_TIMER_CONTROL_UNIT_USER(n)      (3u << MTU_TIMER_CONTROL_UNIT_BIT_N(n))

//slot time in microseconds
#define MTU_SHORT_SLOT_TIME_US 9
#define MTU_LONG_SLOT_TIME_US 20

//RT Tick timer related definitions
#define RT_TICK_TIMER_MCU_SYS_GROUPED eHAL_INT_MCU_SYS_GROUPED_2_TIMER_1
#define RT_TICK_TIMER_PIF_GROUP eHAL_INT_PIF_PCI_MCU_SYS_GROUP_2
#define RT_TICK_MTU_TIMER_REG MTU_TIMER_1_REG

/** Adaptive Threshold Timer definitions.*/
#define ADAPTIVE_THRESHOLD_TIMER_MCU_SYS_GROUPED eHAL_INT_MCU_SYS_GROUPED_2_TIMER_0
#define ADAPTIVE_THRESHOLD_TIMER_PIF_GROUP eHAL_INT_PIF_PCI_MCU_SYS_GROUP_2
#define ADAPTIVE_THRESHOLD_MTU_TIMER_REG MTU_TIMER_0_REG

#define PRE_BEACON_INTERVAL  0x14
#define MAX_NUMBER_OF_MODES  4

#define TIME_TPE_PROGRAM_TXP	1250

typedef struct sMtuParams {
    tANI_U8     difs[8];
    tANI_U16    cwMin[8];
    tANI_U16    cwMax[8];
    tANI_U16    eifs;
    tANI_U8     pifs;
    tANI_U8     slot;
    tANI_U8     cca_miss_limit;
    tANI_U8     sifs;
    tANI_U8     one_usec_limit;
    tANI_U8     bcn_slot_limit;
    tANI_U8     rifs;
} tMtuParams;

typedef enum
{
    MODE_11A = 0,
    MODE_11B = 1,
    MODE_11G_MIXED = 2,
    MODE_11G_PURE = 3
} tMtuMode;



// Warn when polling a register.
#define	MCU_REG_POLLING_WARNING 2000


/*
 * Back-off assignment
 *
 * In MTU lower index has high priority. There are 8 basic backoff timers,
 * and later additional 4 timers are added. Note that these two sets of
 * backoff timer use different control register.
 */

typedef enum
{
    /** Basic BackOff timer.*/
    MTU_BKID_0,
    MTU_BKID_1,
    MTU_BKID_2,
    MTU_BKID_3,
    MTU_BKID_4,
    MTU_BKID_5,
    MTU_BKID_6,
    MTU_BKID_7,
    MTU_BKID_8,
    /** Additional timers.*/
    MTU_BKID_9,
    MTU_BKID_10,
    MTU_BKID_11,
    MTU_BKID_NUM,

    /* High priority backoff timers */
    MTU_BKID_HI_START = MTU_BKID_0,
    MTU_BKID_HI_END   = MTU_BKID_7,
    MTU_BKID_HI_NUM   = 8,

    /* PS-Poll response */
    MTU_BKID_PSPOLL   = MTU_BKID_1,
    /* EDCA data */
    MTU_BKID_AC_VO    = MTU_BKID_4,
    MTU_BKID_AC_VI    = MTU_BKID_5,
    MTU_BKID_AC_BK    = MTU_BKID_6,
    MTU_BKID_AC_BE    = MTU_BKID_7,
    MTU_BKID_MGMT_BCAST = MTU_BKID_3,
    MTU_BKID_MGMT_UCAST = MTU_BKID_2,

    /* Unused */
    MTU_BKID_RESERVED = MTU_BKID_11,
}tMtuBkId;


// In register QWLAN_MTU_BKOF_CONTROL_REG, bits indicating the Data backoffs.
// 3,4,5,6,7. These assignments are done below in the enum
#define SW_MTU_STALL_DATA_BKOF_MASK ((1<< MTU_BKID_AC_BE) | (1<< MTU_BKID_AC_BK) | (1<< MTU_BKID_AC_VI) | (1<< MTU_BKID_AC_VO)) 

typedef enum {
    /* Normal priority. For mCPU these timers generate level 3 interrupts. */
    MTUTIMER_0,
    MTUTIMER_1, //used for sys tick on AP
    MTUTIMER_2,
    MTUTIMER_3,
    MTUTIMER_4,
    MTUTIMER_5,
    MTUTIMER_6,
    MTUTIMER_7,
    /* Low priority. For mCPU this timer generates level 2 interrupt. */
    MTUTIMER_8,
    /* High priority. For mCPU these timers generate level 4 interrupts. */
    MTUTIMER_9,
    MTUTIMER_10,
    /* Highest priority. For mCPU this timer generates level 5 interrupt. */
    MTUTIMER_11,
    MTUTIMER_NUM,

    MTUTIMER_MID_START = MTUTIMER_0,
    MTUTIMER_MID_END = MTUTIMER_7,

    MTUTIMER_LO_START = MTUTIMER_8,
    MTUTIMER_LO_END = MTUTIMER_8,

    MTUTIMER_HI_START = MTUTIMER_9,
    MTUTIMER_HI_END = MTUTIMER_10,

    MTUTIMER_HIGHEST_START = MTUTIMER_11,
    MTUTIMER_HIGHEST_END = MTUTIMER_11,

    MTUTIMER_BEACON_PRE = MTUTIMER_5,  /** Used for Pre Beacon Interrupt.*/

    MTUTIMER_RT_TICK = MTUTIMER_1, /** Used as a RT Tick in Linux.*/
    MTUTIMER_ATH = MTUTIMER_0     /** Used for Adaptive threshold.*/
}tMtuTimer;

eHalStatus halMTU_initialize(tpAniSirGlobal pMac, tMtuParams *modeParams);
eHalStatus halMTU_Start(tHalHandle hHal, void *arg);

void halMTU_updateTimingParams(tpAniSirGlobal pMac, tMtuMode mode);
void halMTU_update11gSlotTimingParams(tpAniSirGlobal pMac, tMtuMode mode);
void halMTU_initTimingParams(tpAniSirGlobal pMac, tMtuMode mode);
void halMTU_updateCW(tpAniSirGlobal pMac, tANI_U8 tid, tANI_U16 ecwMin, tANI_U16 ecwMax);
void halMTU_updateIbssCW(tpAniSirGlobal pMac, tANI_U32 cwValue);
void halMTU_updateAIFS(tpAniSirGlobal pMac, tSirMacEdcaParamRecord*);
void halMTU_update11gSlotTime(tpAniSirGlobal pMac, tANI_U8 slotTime);

eHalStatus halMTU_TsfGet( tpAniSirGlobal pMac, tANI_U32 *pLow, tANI_U32 *pHigh);
eHalStatus halMTU_printMTUParams(tpAniSirGlobal pMac, tMtuMode  mode);
tMtuMode halMTU_getMode(tpAniSirGlobal pMac);
eHalStatus halMTU_setAdaptThreshTimer(tpAniSirGlobal pMac);
eHalStatus halMTU_ErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource);

eHalStatus halMTU_getRIFS(tpAniSirGlobal pMac, tMtuMode  mode, tANI_U8 *rifs);

void halMTU_GetActiveBss(tpAniSirGlobal pMac, tANI_U8 *activeBssCnt);
void halMTU_SetMtbttTimer(tpAniSirGlobal pMac, tANI_U32 mtbttLo, tANI_U32 mtbttHi);
void halMTU_SetTbttTimer(tpAniSirGlobal pMac, tANI_U32 tbttLo, tANI_U32 tbttHi);
void halMTU_GetTbttTimer(tpAniSirGlobal pMac, tANI_U32 *tbttLo, tANI_U32 *tbttHi);
void halMTU_GetTsfTimer(tpAniSirGlobal pMac, tANI_U32 *tsfTimerLo, tANI_U32 *tsfTimerHi);
void halMTU_SetTsfTimer(tpAniSirGlobal pMac, tANI_U32 tsfTimerLo, tANI_U32 tsfTimerHi);
void halMTU_SetIbssValid_And_BTAMPMode(tpAniSirGlobal pMac, 
        tANI_U8 btamp_flag);
eHalStatus halMTU_InitIntHanlder(tpAniSirGlobal pMac);
void halMTU_SetDtim(tpAniSirGlobal pMac, v_U32_t dtimPeriod,
                                              v_U32_t dtimThreshLimit);
void halMTU_GetDtimCount(tpAniSirGlobal pMac, tANI_U16 *dtimCnt);
void halMTU_UpdateMbssInterval(tpAniSirGlobal pMac, tANI_U32 mbssInterval);
void halMTU_DisableBeaconTransmission(tpAniSirGlobal pMac);
void halMTU_UpdateNumBSS(tpAniSirGlobal pMac, tANI_U8 numBSS);
void halMTU_UpdateBeaconInterval(tpAniSirGlobal pMac, tANI_U32 beaconInterval);

void halMTU_EnableDisableBssidTBTTBeaconTransmission(tpAniSirGlobal pMac, 
        tANI_U32 beaconInterval, tANI_U8 enable_flag);
eHalStatus halMTU_UpdateValidBssid(tpAniSirGlobal pMac, tANI_U16 bssIdx, tHalBitVal bitOp);

void halMTU_updateRetryLimit(tpAniSirGlobal pMac, tANI_U8 shortRetry, tANI_U8 longRetry);
eHalStatus halMTU_TimerInterrupt( tHalHandle hHalHandle, eHalIntSources intSource );
eHalStatus halMTU_DefInterruptHandler( tHalHandle hHalHandle, eHalIntSources intSource );

eHalStatus halMTU_DeactivateTimer(tpAniSirGlobal pMac, tMtuTimer timer);

#ifndef WLAN_SOFTAP_FEATURE
eHalStatus halInitPreBeaconTmr( tpAniSirGlobal pMac );
eHalStatus halIntMtuHandlePreBeaconTmr( tHalHandle hHal, eHalIntSources tsfTimerIntr );
#endif

eHalStatus halIntMtuHandlePreBeaconTmr( tHalHandle hHal, eHalIntSources tsfTimerIntr );
tANI_U32 __halMTU_ac2BkoffIndex(tpAniSirGlobal pMac, tANI_U32 ac);

void halMTU_stallBackoffs(tpAniSirGlobal pMac, tANI_U32 mask);
void halMTU_startBackoffs(tpAniSirGlobal pMac, tANI_U32 mask);

#endif /* _HALMTU_H_ */



