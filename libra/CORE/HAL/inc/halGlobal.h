/**
 *
 *  @file:         halGlobal.h
 *
 *  @brief:       This contains the HAL global variables and defines.
 *
 *  @author:    V. K. Kandarpa
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 07/10/2003  File created.
 * 01/24/2008  Virgo related chages.
 */

#ifndef __HAL_GLOBAL_H__
#define __HAL_GLOBAL_H__

#include   "halInternal.h"
#include   "sirMacProtDef.h"
#include   "halTypes.h"
#include   "halInterrupts.h"
#include   "halMacGlobal.h"
#include   "halTxRx.h"
#include   "halRateAdaptApi.h"
#include   "halRateTable.h"
#include   "halDXE.h"
#include   "halAdaptThrsh.h"
#include   "wlan_nv.h"
#include   "sirParams.h"
#include   "halLED.h"
#include   "halPwrSave.h"
#include   "halFwApi.h"
#include   "halRFBringup.h"
#include "vos_event.h"

#define TIME_UNIT_IN_USEC 	1024

typedef struct sAniHalNim
{
    tANI_U8                state;
    TX_TIMER               statTimer;
    tANI_U32               statTmrVal;
    tANI_U32               tempMeasTmrVal;
    tANI_U32               calAttnEvtCnt;
    tANI_U32               lowFPPduDetectCount;
    tANI_U32               prevTxCompleted;
    tANI_U8                resetDueToLossOfPdu;
    tANI_U32               resetCountDueToLossOfPdu;
    tANI_U8                forceFastResetFlag;
    tANI_U8                resetInpFlag;
    tANI_U32               eofAndSofFlag;
    tANI_U32               timeTakenToDoInitCal;
    tANI_U32               timeTakenToDoPeriodicCal;
    tANI_U32               ledTest;
    tANI_U8*               resetBuf;
    tANI_U8                tempCheckEnable;

    tANI_U8                testRadIndicator;
#if (WNI_POLARIS_FW_PRODUCT == AP)
    tANI_U8                rdetFlagEnable;
#endif
} tAniHalNim, *tpAniHalNim;


/*
 * Memory Map address definition
 */

typedef struct sHalMemMap
{
    tANI_U32   maxBssids;
    tANI_U32   maxStations;
    tANI_U32   maxDpuEntries;
    tANI_U32   maxRateEntries;

    tANI_U32   maxHwQueues;

    tANI_U32   memory_baseAddress;
    tANI_U32   internalMemory_size;
    tANI_U32   totalMemory_size;
    tANI_U32   descStartAddr;

    /* System Config space for FW */
    tANI_U32   fwSystemConfig_offset;
    tANI_U32   fwSystemConfig_size;

    tANI_U32   dpuDescriptor_offset;
    tANI_U32   dpuDescriptor_size;

    tANI_U32   keyDescriptor_offset;
    tANI_U32   keyDescriptor_size;

    tANI_U32   micKey_offset;
    tANI_U32   micKey_size;

    tANI_U32   replayCounter_offset;
    tANI_U32   replayCounter_size;

    tANI_U32   dxeRxDescInfo_offset;
    tANI_U32   dxeRxDescInfo_size;

    tANI_U32   tpeStaDesc_offset;
    tANI_U32   tpeStaDesc_size;

    tANI_U32   rpeStaDesc_offset;
    tANI_U32   rpeStaDesc_size;

    tANI_U32   rpePartialBitmap_offset;
    tANI_U32   rpePartialBitmap_size;

    tANI_U32   btqmTxQueue_offset;
    tANI_U32   btqmTxQueue_size;

    tANI_U32   hwTemplate_offset;
    tANI_U32   hwTemplate_size;

    tANI_U32   swTemplate_offset;
    tANI_U32   swTemplate_size;

    tANI_U32   beaconTemplate_offset;
    tANI_U32   beaconTemplate_size;

#ifdef WLAN_SOFTAP_FEATURE
    tANI_U32   bssTable_offset;
    tANI_U32   bssTable_size;

    tANI_U32   staTable_offset;
    tANI_U32   staTable_size;

#if WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE
    tANI_U32   probeRespTemplate_offset;
    tANI_U32   probeRespTemplate_size;
#endif //---> #if WLAN_SOFTAP_FW_PROCESS_PROBE_REQ_FEATURE
#endif	

    tANI_U32   aduUmaStaDesc_offset;
    tANI_U32   aduUmaStaDesc_size;

    tANI_U32   aduRegRecfgTbl_offset;
    tANI_U32   aduRegRecfgTbl_size;
    tANI_U32   aduRegRecfgTbl_curPtr;

    tANI_U32   aduMimoPSprg_offset;
    tANI_U32   aduMimoPSprg_size;

    tANI_U32   packetMemory_offset;

} tHalMemMap, *tpHalMemMap;


// Deferred Message Queue length
#define    HAL_MAX_DEFERRED_MSG_QUEUE_LEN  10

/* Structure used for HAL deferred messages. These messages
 * are deferred while system is in power save
 */
typedef struct sHalDeferredMsgQParams
{
    tSirMsgQ    deferredQueue[HAL_MAX_DEFERRED_MSG_QUEUE_LEN];
    tANI_U16    size;     //size of deferred queue
    tANI_U16    read;     //msg to be read from deferred queue
    tANI_U16    write;    //msg to be written to deferred queue
} tHalDeferredMsgQParams, *tpHalDeferredMsgQParams;

/** Here we maintain the Interupt error stat for non fatal errors.*/
typedef struct sAniIntErrStat
{
    tANI_U32 halIntBmuFifoFull;
    tANI_U32 halIntBmuNoBdPdu;
    tANI_U32 halIntDpuPktExpand;
    tANI_U32 halIntDpuReplayTh;
    tANI_U32 halIntDpuMicErr;
    tANI_U32 halIntDpuPktErr;
    tANI_U32 halIntDpuWatchDogErr;
    tANI_U32 halIntTxpErr;
    tANI_U32 halIntRxpGetBmuErr;
    tANI_U32 halIntRxpAsyncFifoFull;
    tANI_U32 halIntTxpTimeout;
    tANI_U32 halIntMtuErr;
    tANI_U32 halIntWdProtectionErr;
    tANI_U32 halIntWdEnableDisableErr;
}tHalIntErrStat, *tpHalIntErrStat;

typedef struct sHalScanParam{
    void                *pReqParam;
    tSirLinkState       linkState;
    tANI_BOOLEAN        isScanInProgress;
    tANI_U16            dialog_token;
}tHalScanParam;

/*
 * Structure that contains necessary parameters for HAL to interact with TL
 */
typedef struct sHalTlParam {
    vos_event_t txEvent;
    tANI_U8    txStatus;            // Status frm TL after suspend/resume Tx
    tANI_U8    txSuspendTimedOut;   // Flag set to true when TL suspend timesout.

    vos_event_t txMgmtFrameEvent;
    tANI_U8    txMgmtFrameStatus;   // Mgmt frame transfer, to fix TL loss of mgmt frames

} tHalTlParam, *tpHalTlParam;

// Register re-init in ADU related parameters
typedef struct sHalRegBckup {
    // Reinit-register list parameters
    tANI_U32    regListStartAddr;   // Start of reg bckup list
    tANI_U32    regListCurrAddr;    // Current write addr in bckup list
    tANI_U32    regListSize;        // Size of the direct reg list
    tANI_U32    indirectRegListStartAddr;

    tANI_U32    regListImpsStartAddr;       // Start addr of the reg list for IMPS
    tANI_U32    regListBmpsStartAddr;       // Start addr of the reg list for BMPS
    tANI_U32    regListUapsdStartAddr;      // Start addr of the reg list for UAPSD
    tANI_U32    regListTPCGainLutStartAddr; // Start addr of TPC gain lut reg list
    tANI_U32    overrideRegListStartAddr;   // Start addr for inserting extra register

    tANI_U32    volatileRegListStartAddr;   // Start addr of the volatile reg list
    tANI_U32    volatileRegListSize;        // Size of volatile reg list

    tANI_U32    mode;                       // FW backup or host backup required

} tHalRegBckup, tpHalRegBckup;

// Function pointer to the register write function
typedef eHalStatus (*fpWriteHwReg)(tHalHandle, tANI_U32, tANI_U32);

// Function pointer to the register read function
typedef eHalStatus (*fpReadHwReg)(tHalHandle, tANI_U32, tANI_U32*);

// Function pointer to the memory write function
typedef eHalStatus (*fpWriteHwMem)(tHalHandle, tANI_U32, void*, tANI_U32);

// Function pointer to the memory read function
typedef eHalStatus (*fpReadHwMem)(tHalHandle, tANI_U32, void*, tANI_U32);

//callback function for TX complete
//parameter 1 - global pMac pointer
//parameter 2 - txComplete status : 1- success, 0 - failure.
typedef eHalStatus (*tpCBackFnTxComp)(tpAniSirGlobal, tANI_U32);
// HAL Global Definitions
typedef struct sAniSirHal
{
    //multBssTable gets populated by Nv in open and start both sequences.
    //sMultipleBssTable *multBssTable;
    tBssSystemRole halGlobalSystemRole; // STA, AP, IBSS, MULTI-BSS etc.
    tANI_U8       currentChannel;
    eRfBandMode   currentRfBand;
    ePhyChanBondState currentCBState;
    tHalIntRegisterCache intStatusCache[eHAL_INT_MAX_REGISTER];
    tHalIntErrStat     halIntErrStats;
    tHalIntRegisterCache intEnableCache[eHAL_INT_MAX_REGISTER];
    tANI_BOOLEAN intEnabled;     // Are interrupts enabled on this device?
    //chip revision no.
    tANI_U32 chipRevNum;
    //CardType
    tANI_U8 cardType;
    tANI_U32 cfgTxAntenna;
    tANI_U32 cfgRxAntenna;
    tANI_U32 cfgPowerStatePerChain;
    tSirNwType  nwType;
    tHalMemMap               memMap;
    tSirMacEdcaParamRecord   edcaParam[MAX_NUM_AC];
    tHalMsgCallback pPECallBack;
    tHalDeferredMsgQParams   halDeferMsgQ;
    tAniSirHalMac      halMac;

    // Change from halDxe to pHalDxe. halDxe is allocated separately to leverage the fast
    // D-RAM available on Realtek 8652 platform
    tpAniHalDxe        pHalDxe;

    tHalRaGlobalInfo   halRaInfo;
    tAniHalAdaptThresh halAdaptThresh;
    tANI_U8            loopback; // for diags, sets loopback mode

    //HAL state
    tANI_U8 halState; //states are defined in tHAL_STATE

    tANI_BOOLEAN       powerSaveExitRequested;

    TX_TIMER           addBARspTimer; //timer for hdd/softmac addBA rsp

    // LED Related
    tHalLedParam       ledParam;

    // Traffic Activity Monitor timer
    TX_TIMER           trafficActivityTimer;

    /* Recovery mechanism for command-timeout */
    tANI_U8            retryMsgCount;
    tANI_U16           LastPsMsg;

    /* Local power constraint received from LIM as
     * part of channel switch info
     */
    tPowerdBm         gHalLocalPwrConstraint;
    tANI_BOOLEAN      prevCalibrationState;

    /** Used only for debugging, Dont overload */
    tANI_BOOLEAN       fTransmitFrozen;
    tHalScanParam      scanParam;

    /* Parameter for communicating with TL */
    tHalTlParam    TLParam;

    /* Power Save related parameters */
    tHalPwrSave     PsParam;

    /* Register re-init in ADU related parameters */
    tHalRegBckup    RegBckupParam;

    /* Function pointer to the read/write HW register function */
    fpWriteHwReg    funcWriteReg;
    fpReadHwReg     funcReadReg;

    /* Function pointer to the read/write HW memory function */
    fpWriteHwMem    funcWriteMem;
    fpReadHwMem     funcReadMem;

    /* Firmware parameters */
    tHalFwParams    FwParam;

    //callback function pointer for Tx Complete
    //There will be only one request pending. If this pointer is not NULL
    //then request for TxComplete will be rejected. This pointer will be
    //set to NULL when the request is serviced.
    tpCBackFnTxComp pCBackFnTxComp;

    TX_TIMER        txCompTimer; //Timer to wait for TX complete interrupt.

#ifdef WLAN_SOFTAP_FEATURE
    /* Listen mode configure parameters */
    tANI_U8    ghalPhyAgcListenMode;
#endif    
    tANI_U8         mcastBcastFilterSetting;
    tANI_U8         dynamicPsPollValue;

    tANI_U8         teleBcnWakeupEnable;
    tANI_U16        transListenInterval;
    tANI_U16        maxListenInterval;
    tANI_U16        uTransLiNumIdleBeacons;
    tANI_U16        uMaxLiNumIdleBeacons;

} tAniSirHal, *tpAniSirHal;

/* Invalid operating channel. Used during startup */
#define HAL_INVALID_OPERATING_CHANNEL 0
#define HAL_MAX_TXPOWER_INVALID       127

#define isChannelValid(channel) (((channel) > HAL_INVALID_OPERATING_CHANNEL) ? TRUE : FALSE)

#define FILTER_ALL_MULTICAST 0x01
#define FILTER_ALL_BROADCAST 0x02
#define FILTER_ALL_MULTICAST_BROADCAST 0x03

#endif /* __HAL_GLOBAL_H__ */
