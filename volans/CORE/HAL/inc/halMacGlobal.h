/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file halMacGlobal.h contains the driver APIs for Polaris and Cygnus.
 * Author:      V. K. Kandarpa
 * Date:        07/10/2003
 * History:-
 * Date         Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef __HAL_MAC_GLOBAL_H__
#define __HAL_MAC_GLOBAL_H__

#include "palTypes.h"
#include "halCommonApi.h"
#include "halMacBA.h"
#include "halAdu.h"
#include "halTpe.h"

// ----------------------------------
// Station Concatenation statistics
// ----------------------------------

typedef struct sHalMacStaConcatStat
{
    tANI_U32     numPktsTransThr0;
    tANI_U32     numPktsTransThr1;
    tANI_U32     ackTimeOutPerTCcnt[HAL_MAX_NUM_TC];
    tANI_U32     pktsSuccPerTCcnt[HAL_MAX_NUM_TC];
} tHalMacStaConcatStat, *tpHalMacStaConcatStat;

// ------------------------------
// Global Compression statistics
// ------------------------------
typedef struct sHalMacCompStat
{
    tANI_U32     compPktCnt;
    tANI_U32     decompPktCnt;
    tANI_U32     compExpansionCnt;
    tANI_U32     comp50PercentPktCnt;
    tANI_U32     compExcepCnt;
    tANI_U32     compPktExceedsMaxOutputLen;
} tHalMacCompStat, *tpHalMacCompStat;

// --------------------------------------
// Station Compression statistics
// --------------------------------------
typedef struct sHalMacStaCompStat
{
    tANI_U32     compPktLtMarker1Cnt;
    tANI_U32     compPktLtMarker2Cnt;
    tANI_U32     compPktLtMarker3Cnt;
    tANI_U32     compPktGtMarker3Cnt;
} tHalMacStaCompStat, *tpHalMacStaCompStat;

// ----------------------------------------------
// Global Channel Bonding / Escort Pkt statistics
// ----------------------------------------------
typedef struct sHalMacCbEscStat
{
    tANI_U32     cbEscPktCnt;
} tHalMacCbEscStat, *tpHalMacCbEscStat;

// --------------------------------------
// Station Rate Adaptation statistics
// --------------------------------------
typedef struct sHalMacStaRaStat
{
    tANI_U32     priTxPktsSucc;    /* packets successfully sent at primary rate */
    tANI_U32     priTxPktsFail;    /* packets failed at primary rate - each failed Tx
                               * attempt is counted and included here */
    tANI_U32     secTxPktsSucc;    /* packets successfully sent at secondary rate */
    tANI_U32     secTxPktsFail;    /* packets failed at secondary rate - each failed Tx
                               * attempt is counted and included here */
    tANI_U32     terTxPktsSucc;    /* packets successfully sent at tertiary rate */
    tANI_U32     terTxPktsFail;    /* packets failed at tertiary rate - each failed Tx
                               * attempt is counted and included here */
} tHalMacStaRaStat, *tpHalMacStaRaStat;

typedef struct
{
    tAniStaStatStruct staStat;
    tAniTxRxCounters     staStatCntrs;
    tAniSecStats         staBcStat; //Used to hold per station group security statistics. (valid only in IBSS scenario)
    tAniTxRxCounters     bcCntrs;
    tAniTxRxCounters     mcCntrs;
}tAniStaStats, *tpAniStaStats;

typedef struct sAniSirStats
{
    tANI_U32          periodicStatCnt;

    tANI_U8           periodicStats;
    tANI_U32          txCount; //Aggregrate count for quick reference.
    tANI_U32          rxCount; //Aggregrate count for quick reference.
    tANI_U32          prevTxCount;
    tANI_U32          prevRxCount;
    
    tAniStaStats      *pPerStaStats;

    tAniGlobalStatStruct globalStat;

    /* Following stats are updated whenever a station disassociates (updated in DELSTA).
     * Deleting stations' stats are added to below stats. Effectively this contains stats of all previous associations.
     * By adding these to the current station stats, we get the overall count from this interface is enabled. */
    tAniTxRxStats        globalUCStats; //Holds UC tx/rx frames and bytes across association/disassociation.
    tAniTxRxStats        globalBCStats; //Holds BC tx/rx frames and bytes across association/disassociation.
    tAniTxRxStats        globalMCStats; //Holds MC tx/rx frames and bytes across association/disassociation.

    TX_TIMER             statTimer;
    tANI_U32             statTmrVal;

    tANI_U8              lastStatStaId;  
}tAniSirStats, *tpAniSirStats;

typedef struct sAniSirRAStats{
    tANI_U32 txFrmRetryCnt[4];
    tANI_U32 txFrmMultiRetryCnt[4];
    tANI_U32 txFrmSuccCnt[4];
    tANI_U32 txFrmFailCnt[4];
    tANI_U32 rtsFailCnt[4];
    tANI_U32 ackFailCnt[4];
    tANI_U32 rtsSuccCnt[4];
    tANI_U32 txFragCnt[4];
    tANI_U32 tot20MTxPpduDataFrms1;
    tANI_U32 tot20MTxPpduDataFrms2;
    tANI_U32 tot20MTxPpduDataFrms3;
    
    tANI_U32 tot20MTxMpduDataFrms1;
    tANI_U32 tot20MTxMpduDataFrms2;
    tANI_U32 tot20MTxMpduDataFrms3;
    
    tANI_U32 tot20MMpduInAmPdu1;
    tANI_U32 tot20MMpduInAmPdu2;
    tANI_U32 tot20MMpduInAmPdu3;

}tAniSirRABckOffStats, *tpAniSirRABckOffStats;


typedef struct sAniSirDpuStats{
    tANI_U32 micErrCount; //need to clear in hw once the wrap around is 
    tANI_U32 extIVerror;  //need to clear in hw once the wrap around is 
    tANI_U32 formatErrorCount; //need to clear in hw once the wrap around is
    tANI_U32 undecryptableCount; //need to clear in hw once the wrap around is
}tAniSirDpuStats, *tpAniSirDpuStats;

typedef struct sAniSirWrapAroundStats
{
    tAniSirRABckOffStats       *pPrevRaBckOffStats;
    tAniSirRABckOffStats       *pRaBckOffWrappedCount;
    tAniSirDpuStats            *pDpuWrappedCount;
    TX_TIMER                   statTimer;
    tANI_U32                   statTmrVal;
    
    tANI_U8                    lastStatStaId;  
}tAniSirWrapAroundStats, *tpAniWrapAroundStats;

// HAL Global Definitions
typedef struct sAniSirHalMac
{
    /***************************************
    / PMU related fields
    ****************************************/

   /****************************************
   /STA BSS table related field
   *****************************************/
    // Following max are read from CFG.
    tANI_U16 maxSta;              // Max supported STA
    tANI_U16 maxBssId;            // Max BSSIDs supported
    tANI_U16 selfStaId;              // Store the Self Sta ID here -
                                  // Used during self MAC address updates
    tANI_U16 selfStaDpuId;        // Self Sta DPU Idx store 
    tANI_U16 numOfValidSta;       // Current number of STA's connected or valid in the StaTable
    tANI_U8  fShortSlot:1;          // this flag is for per BSS shortSlot setting..
    tANI_U8  rsvd1_sAniSirHalMac:7;  //reserved.

    tANI_U32 beaconInterval;

    tANI_U32 activeBss;

    tANI_U16 BeaconRateIndex;        //Beacon rate index for beacon frames
    tANI_U16 NonBeaconRateIndex;     //Rate index for non-beacon frames
    tANI_U16 MulticastRateIndex;     //Rate index for multicast frames 

    // Global BSS and STA table
    // Memory is allocated when needed.
    void  *bssTable;
    void  *staTable;
#if defined(ANI_OS_TYPE_LINUX)
    void *pStaCacheInfo;
    tANI_U8 *staCache;
    tANI_U8 staCacheInfoFreeHeadIndex;
#endif
   /****************************************
   /Mail box related field
   *****************************************/

    void  *mboxInfo;  /* mailbox management module private info */
    void  *mboxDeferQueue;  /* defer queue for HOST->FW messages */

   /****************************************
   /MTU related field
   *****************************************/

    tANI_U32   lastMtuMode;


   /****************************************
   /RXP related field
   *****************************************/

    void  *rxpInfo;   /* rxp module private info */

    // Place holder for system wide RXP filter mode
    tRxpMode     systemRxpMode;

   /****************************************
   /DPU related field
   *****************************************/
   
    void  *dpuInfo;   /* dpu module private info */
    tANI_U8 dpuRF;    /* dpu routing flag */  

   /****************************************
   /WMM related field
   *****************************************/

    // TSPEC info table
    tTspecTblEntry tspecInfo[LIM_NUM_TSPEC_MAX];

    tANI_BOOLEAN tsActivityChkTmrStarted;

    // Flags indicating which frame classifier is enabled
    // b0: DSCP
    // b1: 802.1P
    // (See PAL_PKT_FLD_DSCP_OFFSET and PAL_PKT_FLD_8021P_OFFSET)
    tANI_U16        frameClassifierEnabled;


   /****************************************
   /STAT related field
   *****************************************/
   
    tAniSirStats     macStats; 
    tAniSirWrapAroundStats wrapStats;
  
    tANI_U8              maxGainIndex;
    tANI_U8              topGainDb;


    /*************************************************
    /BLOCK ACK related field
    *************************************************/

    //BA activity global timer.
    TX_TIMER  baActivityChkTmr;

    // A global object to keep track of the maximum
    // number of buffers available for future BA sessions.
    // Initially, it is set to WNI_CFG_MAX_BA_BUFFERS
    tANI_U16 baRxMaxAvailBuffers;

    // A global object to keep track of the number of
    // Block ACK sessions that are currently active.
    // The maximum number of BA sessions allowed is
    // maintained by WNI_CFG_MAX_BA_SESSIONS
    tANI_U16 baNumActiveSessions;

    // A global object used by HAL to terminate a BA
    // session due to timeout
    tANI_U16 baTimeout;

    //This flag is controlled by config WNI_CFG_BA_AUTO_SETUP
    tANI_U8 baAutoSetupEnabled;

    // A global object used by HAL to trigger the setup
    // of a BA session. Basically, when traffic threshold
    // on a certain <STA/TID> tuple exceeds this value,
    // then this will trigger HAL to notify PE to start
    // a new BA session
    tANI_U32 baSetupThresholdHigh;

    // Global BA Session Table
    tRxBASessionTable baSessionTable[BA_MAX_SESSIONS];

    /*************************************************
    /RIFS related field
    *************************************************/
    tANI_U8 nonRifsBssCount;
    tANI_U8 rifsBssCount;

    /*************************************************
    /BMU related field
    *************************************************/
    tANI_BOOLEAN halDynamicBdPduEnabled;
    tANI_U32 bdPduExchangeable;
    tANI_U32    halMaxBdPduAvail;

    /*************************************************
    /SCAN LEARN related field
    *************************************************/

    TX_TIMER tempMeasTimer;
    tANI_U32 tempMeasTmrVal;

    /*************************************************
    /Softmac related field
    *************************************************/
    tANI_BOOLEAN isFwInitialized; /* Flag to indicate whether Firmware has been initialized or not */
    // Taurus Chip Monitor Timer 
    TX_TIMER  chipMonitorTimer;
    tANI_U32  fwMonitorthr;
    tANI_U32  fwHeartBeatPrev;

    tANI_U32  mpiTxSent;    //pkts count sent to TX
    tANI_U32  mpiTxAbort;   //pkts aborted in TX
    tANI_U8   phyHangThr;   //max threshold to reset on phy hang
    // Frame translation support by ADU/UMA
    tANI_U8     frameTransEnabled;
#ifdef WLAN_PERF
    tANI_U8     uBdSigSerialNum;
#endif

    /** Maintain a copy of ADU UMA STA Desc */
    tAduUmaStaDesc aduUmaDesc[HAL_NUM_UMA_DESC_ENTRIES];

    tHalRxBd rxAmsduBdFixMask;
    tHalRxBd rxAmsduFirstBdCache;

#ifdef FEATURE_ON_CHIP_REORDERING
    /* Number of BA sessions for which reordering is done on-chip */
    tANI_U8 numOfOnChipReorderSessions;
    /* Max Number on-chip reordering sessions*/
    tANI_U8 maxNumOfOnChipReorderSessions;
#endif
} tAniSirHalMac, *tpAniSirHalMac;

#endif
