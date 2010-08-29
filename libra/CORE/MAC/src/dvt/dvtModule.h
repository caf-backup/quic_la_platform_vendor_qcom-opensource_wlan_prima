/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtModule.h

    \brief dvt module contains test functionality and harnesses the common driver architecture

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */

#ifndef DVT_MODULE_H
#define DVT_MODULE_H

#include "palTypes.h"   //all internal and external interfaces
#include "palPipes.h"

#include "sirApi.h"
#include "sirParams.h"

#include "dvtTxRxHeap.h"
#include "dvtMgmt.h"
#include "dvtFrames.h"
#include "halInterrupts.h"
#include "eeprom.h"

#include "asicDXE.h"

typedef enum
{
    //general success/failure values
    DVT_STATUS_SUCCESS = 0,
    DVT_STATUS_FAILURE = 1,

    //these values reserved to report to application layer about adapter
    DVT_STATUS_INVALID_PARM,
    DVT_STATUS_NO_ADAPTER,
    DVT_STATUS_ADAPTER_ALREADY_OPEN,
    DVT_STATUS_ADAPTER_ALREADY_DISABLED,
    DVT_STATUS_ADAPTER_ALREADY_ENABLED,
    DVT_STATUS_ADAPTER_NOT_OPEN,

    DVT_STATUS_MONITOR_NOT_OPEN,
    DVT_STATUS_MONITOR_NOT_SUPPORTED,
    DVT_STATUS_MONITOR_ALREADY_STARTED,
    DVT_STATUS_MONITOR_ALREADY_STOPPED,
    DVT_STATUS_MONITOR_NOT_SET,

    DVT_STATUS_NOTIFICATION_NOT_REGISTERED,
    DVT_STATUS_NOTIFICATION_ALREADY_REGISTERED,

    DVT_STATUS_INVALID_OUTPUT_BUFFER_SIZE,
    DVT_STATUS_NOT_CONNECTED,
    DVT_STATUS_RESOURCES,                  // out of resources.
    DVT_STATUS_OID_FAILURE,                // failed to issue OId to miniport...
    DVT_STATUS_API_UNSUPPORTED,            // API is not supported by the underlying drivers or hardware.

    //these values specific to DVT functionality
    DVT_STATUS_RANGE_MIN = 0x100,
    DVT_STATUS_NO_VALID_STA,               // Indicates that no valid STA descriptor was found,
                                           // using which the transmit parameters could be configured.
    DVT_STATUS_EEPROM_TABLE_NOT_PRESENT,
    DVT_STATUS_BAD_PARAM,
    DVT_STATUS_TIMEOUT,
    DVT_STATUS_RANGE_MAX,

    DVT_STATUS_BAD_FRAME_SPEC,          //returned if a parameter falls out of bounds or similar condition where the spec is not usable
    MAX_DVT_STATUS
}eANI_DVT_STATUS;


typedef struct
{
    tANI_U32 nDescs;                //number of URBs for USB or descriptors for DXE that can be queued for transfer at one time
    tANI_U32 nRxBuffers;            //maximum number of receive buffers from physical memory to use for this pipe
    tANI_U32 refWQ;                 //Reference WQ - for H2B and B2H only
    tANI_U32 xfrType;               //H2B(Tx), B2H(Rx), H2H(SRAM<->HostMem R/W)
    tANI_U32 chPriority;            //Channel Priority 7(Highest) - 0(Lowest)
    tANI_BOOLEAN preferCircular;    //1 = use circular descriptor handling if available, linear otherwise
    tANI_BOOLEAN bdPresent;         //1 = BD attached to frames for this pipe
}sDvtPipeCfg;

#define MAX_BUFSIZE 4096
#define MAX_NUM_PACKETS_PER_TRANSFER 32
#define MAX_BEACON_SIZE 128

typedef struct sWNIAPI_DOT11_SEND_PACKET {

    ePipes pipe;
    void *pPacketData;
    tANI_U32 CountBytes;
    tANI_BOOLEAN  bdPresent;

} tWNIAPI_DOT11_SEND_PACKET;

typedef struct
{
    ePipes pipe;
    sTxFrameTransfer txFrame;
}tDvtSendPacket;

#define DVT_DXE_MASK_DUMP_BD     0x10000000
#define DVT_DXE_MASK_DUMP_PDU    0x20000000
typedef struct
{
    tANI_U32   dumpMasknIndex;
    tANI_U8    data[HAL_BD_SIZE];
} tDvtDumpDxeBDPDU;

#define MAX_TX_PIPES    8
#define MAX_RX_PIPES    7
#define MAX_TIDS        8
typedef struct
{
    tANI_U32    cTxAttempCounts[MAX_TX_PIPES];
    tANI_U32    cTxOKCounts[MAX_TX_PIPES];
    tANI_U32    cTxErrCounts[MAX_TX_PIPES];
    //tANI_U32    cTxCountPerRate[NUM_HAL_PHY_RATES][MAX_TX_PIPES];
} sDvtTxInfo;

typedef struct
{
    tANI_U32    cRxOKCounts[MAX_RX_PIPES];
    tANI_U32    cRXErrCounts[MAX_RX_PIPES];
    tANI_U32    cRxCountPerTid[MAX_TIDS];
    tANI_U32    cRxErrCountPerTid[MAX_TIDS];
    tANI_U32    cRxAMSDUSubFrameDrpCounts[MAX_RX_PIPES];

} sDvtRxInfo;

typedef struct
{
    tANI_U32    headBdIndex;
    tANI_U32    tailBdIndex;
    tANI_U32    availBDs;
    tANI_U32    isEnabled;
    tANI_U32    isDpuWq;
} sBMUWQInfo;

typedef struct
{
    tANI_U32    bdThreshold;
    tANI_U32    pduThreshold;
    tANI_U32    bdReserved;
    tANI_U32    pduReserved;
} sBMUMasterInfo;

typedef struct
{
    tANI_U32    control;
    tANI_U32    freeBD;
    tANI_U32    freePDU;
    tANI_U32    errIntrStatus;
    tANI_U32    errIntrEnable;
    tANI_U32    errIntrAddr;
    tANI_U32    errIntrWData;
    sBMUWQInfo  wqInfo[25];
    sBMUMasterInfo  masterInfo[11];
    tANI_U32    bdPduBaseAddr;
} sDvtBMUInfo;

typedef struct
{
    tANI_U32    csr;
    tANI_U32    sz;
    tANI_U32    saddr;
    tANI_U32    daddr;
    tANI_U32    desc;
    tANI_U32    lstDesc;
    tANI_U32    bd;
    tANI_U32    head;
    tANI_U32    tail;
    tANI_U32    pdu;
    tANI_U32    tstmp;
} tDxeChannelInfo;

typedef struct
{
    tDxeChannelInfo channelInfo[MAX_DXE_CHANNEL];
} sDvtDxeInfo;

typedef struct
{
    tANI_U32 fcsEnCount;   //(1<<22)
    tANI_U32 navSetCount;   //(1<<21)
    tANI_U32 navClearedCount;   //(1<<20)
    tANI_U32 txWarmedupCount;   //(1<<19)
    tANI_U32 addr3InvalidCount;   //(1<<18)
    tANI_U32 addr2InvalidCount;   //(1<<17)
    tANI_U32 addr1InvalidCount;   //(1<<16)
    tANI_U32 plcpOverrideCount;   //(1<<15)
    tANI_U32 isAmpduCount;   //   (1<<14)
    tANI_U32 isAppduCount;   //   (1<<13)
    tANI_U32 isAppdu_lastCount;   //(1<<12) /* last psdu */
    tANI_U32 isAppdu_firstCount;   //(1<<11) /* first psdu */
    tANI_U32 isAmpdu_lastCount;   //(1<<10)
    tANI_U32 isAmpdu_firstCount;   //(1<<9)
    tANI_U32 hasPhycmdCount;   // (1<<8)
    tANI_U32 hasPhystatsCount;   // (1<<7)
    tANI_U32 hasDlmCount;   //(1<<6)
    tANI_U32 bypassDlmprocCount;   //(1<<5)
    tANI_U32 bypassMpduprocCount;   //(1<<4)
    tANI_U32 failFilterCount;   // (1<<3)
    tANI_U32 failMaxPktLenCount;   //(1<<2)
    tANI_U32 fcsErrorCount;   //  (1<<1)
    tANI_U32 exceptionCount;   //  (1<<0)
} tDvtRxpFlags;

#define MAX_NUM_DATA_FRAMES 16
typedef struct
{
    tANI_U32    cTxAttempCounts;
    tANI_U32    cTxOKCounts;
    tANI_U32    cRxOKCounts;
    tANI_U32    cRXErrCounts;
    tANI_U32    cRxCountPerTid[MAX_TIDS];
    tANI_U32    cRxErrCountPerTid[MAX_TIDS];
    tDvtRxpFlags rxpFlags[MAX_RX_PIPES];
    tANI_U32 cRxDataFrames[MAX_NUM_DATA_FRAMES];
    tANI_U32    cRxControlFrames;
    tANI_U32    cRxMgmtFrames;
    tANI_U32    cRxBeaconFrames;
	tANI_U32	cRxMulticastFrames;
	tANI_U32	cRxBroadcastFrames;
	tANI_U32	cRxUnicastFrames;
} tDvtRxCounters;

typedef struct
{
    tANI_U8 ppi;
    tANI_U8 pli;
    tANI_U16 txFragThreshold4B;
    tANI_U8 signature;
    tANI_U16 enablePerTidDecomp;
    tANI_U16 enablePerTidComp;
    tANI_U16 enableTxPerTidInsertConcatSeqNum;
    tANI_U16 enableRxPerTidRemoveConcatSeqNum;
    tANI_U8 replayCountSet;
    tANI_U8 mickeyIndex;
    tANI_U8 keyIndex;
    tANI_U8 txKeyId;
    tANI_U8 encryptMode;
    tANI_U32 idxPerTidReplayCount[4];
    tANI_U32 txSentBlocks;
    tANI_U32 rxRcvddBlocks;
    tANI_U8  wepRxKeyIdx0;
    tANI_U8  wepRxKeyIdx1;
    tANI_U8  wepRxKeyIdx2;
    tANI_U8  wepRxKeyIdx3;
    tANI_U8 micErrCount;
    tANI_U32 excludedCount;
    tANI_U16 formatErrorCount;
    tANI_U16 undecryptableCount;
    tANI_U32 decryptErrorCount;
    tANI_U32 decryptSuccessCount;
    tANI_U8 keyIdErr;
    tANI_U8 extIVerror;
} tDpuDescStats;

typedef struct
{
    tANI_U8 dpuDescIdx;
    tDpuDescStats dpuDescStats;
} sDvtDpuDescInfo;

typedef struct
{
    tANI_U32    txPktCount;
    tANI_U32    rxPktCount;
    tANI_U32    bDCheckCount;
    tANI_U32    control;
    tANI_U32    interruptMask;
    tANI_U32    interruptStatus;
    tANI_U32    eventfifo;
    tANI_U32    replayCntThrMSW;
    tANI_U32    rplayCntThrLSW;
    tANI_U32    pPIExpandThreshold;
    tANI_U32    maximumPktLen;
    tANI_U32    compressionTypeCnt;
    tANI_U32    compressionNumPkts;
    tANI_U32    decompressionNumPkts;
    tANI_U32    watchdog;
    tANI_U32    bDInCounts;
    tANI_U32    bDOutCounts;
} sDvtDpuInfo;

#define DVT_INFO_MASK_TX    0x00000001
#define DVT_INFO_MASK_RX    0x00000002
#define DVT_INFO_MASK_BMU   0x00000004
#define DVT_INFO_MASK_DXE   0x00000008
#define DVT_INFO_MASK_DPU   0x00000010
#define DVT_INFO_MASK_DPU_DESC   0x00000020

typedef struct
{
    tANI_U32    dbgInfoMask;    //indicate which members are valid
    sDvtTxInfo  txInfo;
    sDvtRxInfo  rxInfo;
    sDvtBMUInfo bmuInfo;
    sDvtDxeInfo dxeInfo;
    sDvtDpuInfo dpuInfo;
    sDvtDpuDescInfo dpuDescInfo;
} sDvtGatherInfo;

typedef enum
{
    DVT_MEM_TEST_8BIT_ACCESS,
    DVT_MEM_TEST_16BIT_ACCESS,
    DVT_MEM_TEST_32BIT_ACCESS,
    DVT_MEM_TEST_128BYTE_ACCESS,

    MAX_DVT_MEM_TEST_ACCESS
}eDvtMemTestAccessSize;

typedef enum
{
    DVT_MEM_TEST_WRITE_READBACK_RANDOM,
    DVT_MEM_TEST_WRITE_READBACK_VALUE,
    DVT_MEM_TEST_DXE_WRITE_READBACK_RANDOM,
    DVT_MEM_TEST_DXE_WRITE_READBACK_VALUE,

    MAX_DVT_MEM_TEST_TYPE
}eDvtMemTestType;

typedef struct
{
    eDvtMemTestType type;
    eDvtMemTestAccessSize accessSize;
    tANI_U32 startAddr;
    tANI_U32 length;
    tANI_U32 value;          //specific value to use
}tDvtMemTest;

#define MAX_WRITE_SIZE  128

#define MAX_DPU_REGS        38 // 0x98/4
#define MAX_BMU_REGS        60   // 0xEC/4
#define MAX_DXE_REGS        56
#define MAX_RXP_REGS        58
#define MAX_TXP_REGS        22
#define MAX_MTU_REGS        86   // 0xEC/4

typedef struct
{
    tANI_U32 dpuRegVals[MAX_DPU_REGS];
    tANI_U32 bmuRegVals[MAX_BMU_REGS];
    tANI_U32 dxeRegVals[MAX_DXE_REGS];
    tANI_U32 rxpRegVals[MAX_RXP_REGS];
    tANI_U32 txpRegVals[MAX_TXP_REGS];
    tANI_U32 mtuRegVals[MAX_MTU_REGS];

} sDvtReadRegs;

#define MAX_NUM_BSS 1

#define NUM_DEST_STATIONS   32

// ------------------------PMI RATE CMD BITS--------------------------
# define PMI_CMD_RATE_IDX_CONCAT_PKT_MASK           0x2
# define PMI_CMD_RATE_IDX_CONCAT_PKT_OFFSET         0x1

# define PMI_CMD_RATE_IDX_SHORT_GUARD_INTERVAL_MASK     0x8000
# define PMI_CMD_RATE_IDX_SHORT_GUARD_INTERVAL_OFFSET   0xf

# define PMI_CMD_RATE_IDX_BANDWIDTH_MODE_MASK       0x6000
# define PMI_CMD_RATE_IDX_BANDWIDTH_MODE_OFFSET     0xd

# define PMI_CMD_RATE_IDX_PKT_TYPE_MASK     0x700
# define PMI_CMD_RATE_IDX_PKT_TYPE_OFFSET   0x8

# define PMI_CMD_RATE_IDX_NSS_11B_MASK      0xC00000
# define PMI_CMD_RATE_IDX_NSS_11B_OFFSET    0x16

# define PMI_CMD_RATE_IDX_AIRGO_11N_RATES_MASK      0x100000
# define PMI_CMD_RATE_IDX_AIRGO_11N_RATES_OFFSET    0x14

# define PMI_CMD_RATE_IDX_PSDU_RATE_MASK        0xF0000
# define PMI_CMD_RATE_IDX_PSDU_RATE_OFFSET      0x10

# define PMI_CMD_RATE_IDX_PPDU_RATE_MASK        0xF0
# define PMI_CMD_RATE_IDX_PPDU_RATE_OFFSET      0x4
// -------------------------------------------------------------------

typedef struct
{
    tANI_U8 concat_packet;
    tANI_U8 short_guard_interval;
    tANI_U8 bandwidth_mode;
    tANI_U8 nss_11b;
    tANI_U8 psdu_rate;
    tANI_U8 ppdu_rate;

} tDvtPmiRateIdxBits;

typedef enum
{
    DVT_PHY_MODE_RATE_11_A_TITAN_NON_DUPLICATE,
    DVT_PHY_MODE_RATE_11_N_MCS_0_15_20MHZ,
    DVT_PHY_MODE_RATE_11_N_MCS_0_15_40MHZ,
    DVT_PHY_MODE_RATE_11_A_TITAN_LEGACY_DUPLICATE,
    DVT_PHY_MODE_RATE_11_N_MCS_32_40MHZ_DUPLICATE,
    DVT_PHY_MODE_RATE_11_N_AIRGO_PROPRIETARY,
    DVT_PHY_MODE_RATE_11_B
} eDVTPhyModeRates;

/*
 * defining the tSmacRuntimeStat structure without bit fields as the perl harness
 * does not support the bit fields.
 */
typedef struct sDvtSmacRuntimeStat {
    tANI_U32 reserved1[2];
    tANI_U32 signature;
    tANI_U32 arch;
    tANI_U32 version;
    tANI_U32 caps;
    tANI_U32 addInfo;
    tANI_U32 heartbeat;
    tANI_U32 irqStatus;
    tANI_U32 smacStatus;
    tANI_U32 errorAddr;
    tANI_U32 errorInfo;
    tANI_U32 smacReg1;
    tANI_U32 smacReg2;
    tANI_U32 smacReg3;
    tANI_U32 smacDiagInfoBlockOffset;
    tANI_U32 sysErrorCount;
    tANI_U32 checkedErrorCount;
    tANI_U32 mboxMsgTxCount;
    tANI_U32 mboxMsgTxFailedCount;
    tANI_U32 mboxMsgTxLastErrno;
    tANI_U32 mboxMsgRxCount;
    tANI_U32 mboxMsgRxIgnoredCount;
    tANI_U32 mboxMsgRxFailedCount;
    tANI_U32 beaconTxCount;
    tANI_U32 beaconRxCount;
    tANI_U32 txQueueCount;
    tANI_U32 txQueueDropCount;
    tANI_U32 txProgramCount;
    tANI_U32 txSendCount;
    tANI_U32 txAckedCount;
    tANI_U32 txSendFragCount ;
    tANI_U32 txSendAmpduCount;
    tANI_U32 txSendGroupCount;
    tANI_U32 txSendNoRespCount;
    tANI_U32 txSendCtrlCount;
    tANI_U32 txRxRespInvalidCount;
    tANI_U32 txRetryCount;
    tANI_U32 txRxRespCount;
    tANI_U32 txTxpErrorCount;
    tANI_U32 txSendBurstCount;
    tANI_U32 txDropCount;
    tANI_U32 txMissingRxCCAcount;
    tANI_U32 txMissingRxPktPushCount;
    tANI_U32 txRxPktdetTimeout;
    tANI_U32 txErrorCount;
    tANI_U32 txQueueRawCount;
    tANI_U32 txSendConcatCount;
    tANI_U32 txSendResv;
    tANI_U32 txBAretansmitFrameCount;
    tANI_U32 txUnackBACount;
    tANI_U32 txRetransmitByUnexpectedResp;
    tANI_U32 txRetransmitByTxpReset;
    tANI_U32 txBadStaSignature;
    tANI_U32 rxRcvdCount;
    tANI_U32 rxTxRespCount;
    tANI_U32 rxFragCount;
    tANI_U32 rxFragDropCount;
    tANI_U32 rxAmpduCount;
    tANI_U32 rxAmsduCount;
    tANI_U32 rxAmsduSubFrames;
    tANI_U32 rxDroppedCount;
    tANI_U32 rxFwdCount;
    tANI_U32 rxDropDupCount;
    tANI_U32 rxMcastCount;
    tANI_U32 rxConcatCount;
    tANI_U32 rxReserved1;
    tANI_U32 reserved[3];
} __ani_attr_packed tDvtSmacRuntimeStat;

typedef struct sDvtPktDxeTimestampStats
{
	tANI_U32 h2bEndTimeStamp;
	tANI_U32 b2hEndTimeStamp;
	tANI_U32 h2bStartDelayAfterDxeTrigger;
	tANI_U32 rxProcessingDelay;
	tANI_U32 h2bStartDelayFromPrevPkt;
	tANI_U32 b2hStartDelayFromPrevPkt;
	tANI_U32 h2bXferTime;
	tANI_U32 bmuIdleTime;
	tANI_U32 b2hXferTime;
} tDvtPktDxeTimestampStats;

#define DVT_MAX_DXE_PKT_HISTORY 125

/// DVT Global Definitions
typedef struct sAniSirDvt
{
    sDvtGatherInfo  dvtInfo;
    tANI_U32        cTxCompleteCount;
    tSirMsgQ        dvtMsg;
    tANI_U32        crcEnable;
    tANI_U32        rxcrcTable[256];
    tANI_U32        rxCrcTableInit;

	tANI_U32        dxeTimestampEnable;
	tDvtPktDxeTimestampStats dxePktTimeHistogram[MAX_RX_PIPES][DVT_MAX_DXE_PKT_HISTORY];
	int        		dxePktHeadIndx[MAX_RX_PIPES], dxePktTailIndx[MAX_RX_PIPES];

    sDvtHeapResources txHeap;
    sDvtHeapResources rxHeap;

    sDvtTxQueue txQueues[NUM_DVT_TX_QUEUES];
    sDvtRxQueue rxQueues[NUM_DVT_RX_QUEUES];

    sDvtTestBounds frameBounds[MAX_DVT_FRAME_OPTION];

    sDvtSimpleStationConfig simpleStaTable[NUM_DEST_STATIONS];
    sDvtSimpleMacConfig mac;
    sDvtSimpleBssConfig bss[MAX_NUM_BSS];
    tANI_U32    cTxCountPerRate[NUM_HAL_PHY_RATES][MAX_TX_PIPES];
    tANI_U32    cRxCountPerRate[NUM_HAL_PHY_RATES][MAX_RX_PIPES];
    tDvtRxpFlags rxpFlags[MAX_RX_PIPES];
    tANI_U32 cRxDataFrames[MAX_NUM_DATA_FRAMES];
    tANI_U32 cRxCountperSta[MAX_TIDS][NUM_DEST_STATIONS];
    tANI_U32 cTxCountperSta[MAX_TIDS][NUM_DEST_STATIONS];
    tANI_U32 cRxPMIRateIdxMisMatchCount[NUM_HAL_PHY_RATES];
    tANI_U32    cRxControlFrames;
    tANI_U32    cRxMgmtFrames;
    tANI_U32    cRxBeaconFrames;
	tANI_U32	cRxMulticastFrames;
	tANI_U32	cRxBroadcastFrames;
	tANI_U32	cRxUnicastFrames;

} tAniSirDvt, *tpAniSirDvt;




#endif // DVT_MODULE_H
