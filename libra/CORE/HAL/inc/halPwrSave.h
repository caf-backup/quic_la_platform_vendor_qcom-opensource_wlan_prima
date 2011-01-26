/*
 * File:        halPwrSave.h
 * Description: This file contains all the Power save related functions
 *              required for initialization, Idle Mode power save,
 *              Beacon Mode power save, Unscheduled automatic power
 *              save delivery and power save configuration.
 *
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 07/21/2008 lawrie      Created the functions for IMPS, BMPS & UAPSD
 *
 *
 */

#ifndef _HALPWRSAVE_H_
#define _HALPWRSAVE_H_

#include "halTypes.h"
#include "aniGlobal.h"
#include "sirApi.h"
#include "vos_event.h"
#include "vos_status.h"
#include "vos_types.h"
#include "sirMacProtDef.h"
#include "halPMU.h"

#define HAL_MAILBOX_VERSION             1

/* Various Power Save states */
#define HAL_PWR_SAVE_ACTIVE_STATE       0
#define HAL_PWR_SAVE_IMPS_REQUESTED     (1<<0)
#define HAL_PWR_SAVE_IMPS_STATE         (1<<1)
#define HAL_PWR_SAVE_BMPS_STATE         (1<<2)
#define HAL_PWR_SAVE_SUSPEND_BMPS_STATE (1<<3)
#define HAL_PWR_SAVE_UAPSD_STATE        (1<<4)

// TODO: this defines need to go into sirParams.h
#define SIR_HAL_SUSPEND_BMPS            1101
#define SIR_HAL_RESUME_BMPS             1102

//TO DO: This needs to be defined in CFG
#define HAL_PWR_SAVE_FW_PMU_SLEEP_TIMEOUT    3  //millisecond
#define HAL_PWR_SAVE_FW_FRAME_RETRIES         4
#define HAL_PWR_SAVE_FW_CHIP_PWR_DOWN_ENABLE  1
#define HAL_PWR_SAVE_FW_CHIP_PWR_DOWN_DISABLE  0
#define HAL_PWR_SAVE_FW_MAX_BCAST_DATA_RECEPTION_TIME_MS       12  //TO DO
#define HAL_PWR_SAVE_FW_MAX_UCAST_DATA_RECEPTION_TIME_MS       20  //TO DO
#define HAL_PWR_SAVE_FW_MAX_SIF_UNFREEZE_TIME_MS              100  //TO DO
#define HAL_PWR_SAVE_FW_MAX_BTQM_QUEUES_EMPTY_TIMEOUT_MS      250  //TO DO
#define HAL_PWR_SAVE_FW_BMPS_MINIMUM_SLEEP_TIME_US          10000  //TO DO
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_US        30000  //TO DO
#define HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_US      30000  //TO DO
/* Sum of all components of PMU power up staircase.
 * for 40Mhz clock: (30 + 50)*30.5 + ADU_Reinit(820us) = 3260us
 * for 19.2Mhz Clock: (30 + 194)*30.5 + ADU_reinit(820us) = 7652us
 */
#ifdef HAL_WLAN_1P2_AON_SW_STAIRCASE
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US    3460
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_WITHOUT_RFXO_SETTLING_US    1935
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US_19_2  7852
#else
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US    3260
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_WITHOUT_RFXO_SETTLING_US    1735
#define HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US_19_2  7652
#endif

#define HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_RFXO_US  4236
#define HAL_PWR_SAVE_FW_BMPS_BEACON_MODE_EARLY_TIMEOUT_US    2000  //TO DO
#define HAL_PWR_SAVE_FW_FIRST_BEACON_RECEPTION_TIMEOUT_MS      20  //TO DO
#define HAL_PWR_SAVE_FW_TX_PATH_MONITOR_TIME_MSEC               5
/* The total time required for XO setting time. By spec it is 
 * 1.5ms which is equivalent to 50 sleep clocks.
 */
#define HAL_PWR_SAVE_FW_BMPS_RF_SETTLING_TIME_CLKS             50
#define HAL_PWR_SAVE_SLP_CLK_PERIOD_US                         31
/* The total time required for TCXO power up time for 19.2Mhz operation. 
 * By spec it is 6ms which is equivalent to 197 sleep clocks. Since 
 * FW already adds 3 sleep clocks (default value) on top of this define, 
 * this is defined as 194.
 */
#define HAL_PWR_SAVE_FW_BMPS_RF_SETTLING_TIME_CLKS_19_2       194
#define HAL_PWR_SAVE_FW_UAPSD_DATA_RECEPTION_TIMEOUT_MS         3
#define HAL_PWR_SAVE_MAX_CONS_BCN_MISS                          5
#define HAL_PWR_SAVE_BCN_MISS_GRACE_PERIOD_US                 200
#define HAL_PWR_SAVE_BCN_MISS_WAIT_TU                          10

// Frames to be passed to host while in WOWL mode
#define HAL_PWR_SAVE_FW_WOWL_FRAMES_PASSED_TO_HOST        ((1<<SIR_MAC_MGMT_DISASSOC) |(1 << SIR_MAC_MGMT_DEAUTH))

// TO DO: These values have to be tuned.
#define QWLANFW_TRAFFIC_MONITOR_TIMEOUT_MSEC  5
//Greater than Rxp BD threshold and less DXE 0 threshold
#define QWLANFW_NUM_BDPDU_THRESHOLD           0x40

/*
 *  TBTT compensation = PIFS(SIFS+Slot)  +   Preamble   +   MPDU Header
 *  1Mbps(2.4Ghz):            30(20+10)      192(long)      192 (192 bit or 24 bytes)
 *  6Mbps(5Ghz):               25(9+16)         20             32
 */
#define TBTT_COMPENSATION_2_4_GHZ       414  //us
#define TBTT_COMPENSATION_5_GHZ         77   //us


// Macros for hardware mutex
#define HAL_PS_BUSY_GENERIC             0
#define HAL_PS_BUSY_INTR_CONTEXT        1
#define HAL_PS_MUTEX_ACQ_RETRY_COUNT    10
#define HAL_PS_MUTEX_MAX_COUNT          1

#define IS_PWRSAVE_STATE_IN_BMPS    (pMac->hal.PsParam.pwrSaveState.p.psState & HAL_PWR_SAVE_BMPS_STATE)
#define IS_HOST_BUSY_INTR_CNTX      (pMac->hal.PsParam.mutexIntrCount)
#define IS_HOST_BUSY_GENERIC_CNTX   (pMac->hal.PsParam.mutexCount)

#define ENABLE_HEART_BEAT_IN_PS        1
#define DISABLE_HEART_BEAT_IN_PS       0

#define HAL_XO_CLK_MODE_OFFSET          14
#define HAL_XO_CLK_MODE_MASK           0x3
#define HAL_XO_CLK_19_2MHZ             0x3
#define HAL_XO_CLK_40MHZ               0x1

/* Function pointer for Timer expiry */
typedef eHalStatus (*funcHalPsTimer)(tpAniSirGlobal, tANI_U16);

/* Call back function for Power Save Suspend/Resume */
typedef void (*funcHalPsCB)(tpAniSirGlobal, void*, tANI_U16, tANI_U32);

/* Structure to hold the various states in power save */
typedef union sHalPsStates {
    tANI_U8  state;
    struct sPsStateBits {
        tANI_U8 psState: 7;         // Power Save state
        tANI_U8 mutexProtect: 1;    // Mutex protection required between FW-HOST
    } p;  /* To avoid compile problems. AMSS RVCT compiler doesn't allow unnamed type. */
} tHalPsStates;

/* Parameters required for IMPS */
typedef struct sHalPsImps {
    tANI_U32 aduMemAddr;    // Addr of the register table in the ADU mem
    tANI_U32 regListSize;   // Size of the re-init register list
    tANI_U8  fwStatus;      // FW status message
    tANI_U16 token;
} tHalPsImps;

/* Parameters required for BMPS */
typedef struct sHalPsBmps {
    tANI_U32 aduMemAddr;    // Addr of the register table in the ADU mem
    tANI_U32 regListSize;   // Size of the re-init register list
    tANI_U8  fwStatus;      // FW status message
    tANI_U16 token;
} tHalPsBmps;

/* Parameters required for UAPSD */
typedef struct sHalPsUapsd {
    tANI_U32 aduMemAddr;    // Addr of the register table in the ADU mem
    tANI_U32 regListSize;   // Size of the re-init register list
    tANI_U8  fwStatus;      // FW status message
    tANI_U16 token;
} tHalPsUapsd;

/* Paramters required for the entire Power save module */
typedef struct sHalPwrSave
{
    tHalPsStates    pwrSaveState;   // Holds the power save state
    tANI_U16        dialogToken;    // Dialog Token used for the message
    tANI_U8         mutexCount;     // Count of mutex acquired.
    tANI_U8         mutexIntrCount;     // Count of mutex acquired.

    vos_event_t     fwRspEvent;     // VOSS event required for FW responses

    // Timer for FW response
    vos_timer_t     fwRspTimer;     // Timer for the FW response messages
    tANI_U32        fwRspTimeout;   // Timer timeout value

    // Function pointer called on the timer expiry
    funcHalPsTimer  fwRspTimeoutFunc;

    // Function pointer and data for callback during suspend/resume
    funcHalPsCB     psCbFunc;
    void*           psCbData;

    // Response Type to be send back to PE on timeout
    tANI_U16        rspType;

    // If DTIM period needs to be ignored for determining LI
    tANI_U8         ignoreDtim;

    // Reinit-register list parameters
    tANI_U32        regListStartAddr;   // Start of reg bckup list
    tANI_U32        regListCurrAddr;    // Current write addr in bckup list
    tANI_U32        regListSize;        // Size of the direct reg list
    tANI_U32        indirectRegListStartAddr;

    // Power Save mode contexts
    tHalPsImps      ImpsCtx;        // IMPS context
    tHalPsBmps      BmpsCtx;        // BMPS context
    tHalPsUapsd     UapsdCtx;        // UAPSD context;
} tHalPwrSave;

/* Beacon Filtering data structures */
typedef struct sBeaconFilterMsg
{
    tANI_U16    capabilityInfo;
    tANI_U16    capabilityMask;
    tANI_U16    beaconInterval;
    tANI_U16    ieNum;
} tBeaconFilterMsg, *tpBeaconFilterMsg;

typedef struct sEidByteInfo
{
    tANI_U8     offset;
    tANI_U8     value;
    tANI_U8     bitMask;
    tANI_U8     ref;
} tEidByteInfo, *tpEidByteInfo;


/* The above structure would be followed by multiple of below mentioned structure */
typedef struct sBeaconFilterIe
{
    tANI_U8         elementId;
    tANI_U8         checkIePresence;
    tEidByteInfo    byte;
} tBeaconFilterIe, *tpBeaconFilterIe;



/* Power Save Initialization/Exit functions */
eHalStatus halPS_Init(tHalHandle hHal, void *arg);
eHalStatus halPS_Exit(tHalHandle hHal, void *arg);

/* Get the Power Save state */
tANI_U8 halPS_GetState(tpAniSirGlobal pMac);

/* Power Save Configuration */
eHalStatus halPS_Config(tpAniSirGlobal pMac, tpSirPowerSaveCfg pPowerSaveConfig);
eHalStatus halPS_SetPeerParams(tpAniSirGlobal pMac, tANI_U8 staidx, tANI_U8 *pStaMacAddr, tANI_U8 *pApMacAddr);

/* Update the RSSI threshold values in the Sys config */
eHalStatus halPS_SetRSSIThresholds(tpAniSirGlobal pMac, tpSirRSSIThresholds pThresholds);

/* Get the RSSI value in BMPS mode */
eHalStatus halPS_GetRssi(tpAniSirGlobal pMac, tANI_S8 *pRssi);

/* Prepare PS-Poll template for FW to send during BMPS */
eHalStatus halPS_SetPsPollParam(tpAniSirGlobal pMac, tANI_U8 staIdx,
        tANI_U16 aid, tANI_U16 rateIndex, tANI_U8 txPower);

/* Update the beacon interval parameter into the Sys config */
eHalStatus halPS_SetBeaconInterval(tpAniSirGlobal pMac, tANI_U16 beaconInterval);

/* Update the listen interval parameter into the Sys config */
eHalStatus halPS_SetListenIntervalParam(tpAniSirGlobal pMac, tANI_U16 listenInterval);

/* Idle Mode Power Save (IMPS) functions */
/* Functions to handle IMPS request messages from upper layer PE */
eHalStatus halPS_HandleEnterImpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken);
eHalStatus halPS_HandleExitImpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken);
void halPS_ExecuteStandbyProcedure( tpAniSirGlobal pMac);

/* Functions to handle IMPS response messages from FW to the host */
eHalStatus halPS_PostponeFwEnterImpsRsp(tpAniSirGlobal pMac, void* pFwMsg);
eHalStatus halPS_HandleFwEnterImpsRsp(tpAniSirGlobal pMac, void* pFwMsg);
eHalStatus halPS_HandleFwImpsExited(tpAniSirGlobal pMac, void* pFwMsg);

/* Beacon Mode Power Save (BMPS) function */
/* Functions to handle BMPS request messages from upper layer PE */
eHalStatus halPS_HandleEnterBmpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpEnterBmpsParams pPeMsg);
eHalStatus halPS_HandleExitBmpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpExitBmpsParams pPeMsg);
eHalStatus halPS_SuspendBmps(tpAniSirGlobal pMac, tANI_U16 dialogToken,
      funcHalPsCB cbFunc, void* data);
eHalStatus halPS_ResumeBmps(tpAniSirGlobal pMac, tANI_U16 dialogToken,
      funcHalPsCB cbFunc, void* data, tANI_U8 rspReq);

/* Functions to handle BMPS response messages from FW to the host */
eHalStatus halPS_HandleFwEnterBmpsRsp(tpAniSirGlobal pMac, void* pFwMsg);
eHalStatus halPS_HandleFwExitBmpsRsp(tpAniSirGlobal pMac, void* pFwMsg);
eHalStatus halPS_HandleFwSuspendBmpsRsp(tpAniSirGlobal pMac, void *pfwMsg);
eHalStatus halPS_HandleFwResumeBmpsRsp(tpAniSirGlobal pMac, void *pfwMsg);
eHalStatus halPS_HandleFwBmpsStatusMsg(tpAniSirGlobal pMac, void* pFwMsg);

/* Unscheduled Automatic Power Save Delivery (UAPSD) functions */
/* Functions to handle UAPSD request messages from upper layer PE */
eHalStatus halPS_HandleEnterUapsdReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpUapsdParams pPeMsg);
eHalStatus halPS_HandleExitUapsdReq(tpAniSirGlobal pMac, tANI_U16 dialogToken);

/* Functions to handle UAPSD response messages from FW to the host */
eHalStatus halPS_HandleFwEnterUapsdRsp(tpAniSirGlobal pMac, void* pFwMsg);
eHalStatus halPS_HandleFwExitUapsdRsp(tpAniSirGlobal pMac, void* pFwMsg);

/* Beacon Filtering */
eHalStatus halPS_HandleAddBeaconFilter(tpAniSirGlobal, tANI_U16, void *);
eHalStatus halPS_HandleRemBeaconFilter(tpAniSirGlobal, tANI_U16, tANI_U8 *, tANI_U8);

/* Low/High RSSI Indication */
eHalStatus halPS_SendLowRssiInd(tpAniSirGlobal pMac);
eHalStatus halPS_HandleFwRssiNotification(tpAniSirGlobal pMac, void* pFwMsg);

/* Miss Beacon Indication */
eHalStatus halPS_SendBeaconMissInd(tpAniSirGlobal pMac);

/* Acquring/Releaseing Hardware mutex */
eHalStatus halPS_SetHostBusy(tpAniSirGlobal pMac, tANI_U8 ctx);
eHalStatus halPS_ReleaseHostBusy(tpAniSirGlobal pMac, tANI_U8 ctx);

/* Adds WOWL broadcast patterns to firmware when add msg is received from PE */
eHalStatus halPS_AddWowlPatternToFw(tpAniSirGlobal pMac, tpSirWowlAddBcastPtrn pBcastPat);

/* Removes WOWL broadcast pattern from firmware when remove msg is received from PE */
eHalStatus halPS_RemoveWowlPatternAtFw(tpAniSirGlobal pMac, tpSirWowlDelBcastPtrn pDelBcastPat);

/* Set and enable WOWL modes of operation */
eHalStatus halPS_EnterWowlReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpSirHalWowlEnterParams pWowParams);

/* Clear and disable WOWL modes of operation */
eHalStatus halPS_ExitWowlReq(tpAniSirGlobal pMac, tANI_U16 dialogToken);

/* Control chip power up/down during power save */
eHalStatus halPS_CtrlChipPowerDown(tpAniSirGlobal pMac, tANI_U8 enable);
void halPSDataInActivityTimeout( tpAniSirGlobal pMac, tANI_U32 cfgId );
void halPSFWHeartBeatCfg( tpAniSirGlobal pMac, tANI_U32 cfgId );
void halPSBcnFilterCfg( tpAniSirGlobal pMac, tANI_U32 cfgId );
void halPSRssiMonitorCfg( tpAniSirGlobal pMac, tANI_U32 cfgId );
void halPSRfSettlingTimeClk( tpAniSirGlobal pMac, tANI_U32 cfgId );

#endif //_HALPWRSAVE_H_
