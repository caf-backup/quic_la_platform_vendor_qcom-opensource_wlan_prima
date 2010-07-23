/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *


    \file dvtMsgApi.h

    \brief Defines message types and union for DVT messages that are passed to the driver either from a socket or application.

    $Id$

    Copyright (C) 2006 Airgo Networks, Incorporated


   ========================================================================== */


#ifndef DVT_MSG_API_H
#define DVT_MSG_API_H

#include "dvtModule.h"
typedef struct sAniSirGlobal *tpAniSirGlobal;
#include "halMsgApi.h"

#define DVT_MSG_TYPES_BEGIN_40          (SIR_DVT_MODULE_ID << 8)

// All messages:
typedef enum
{
    DVT_MSG_TYPES_BEGIN               = (SIR_DVT_MODULE_ID << 8),

    DVT_MSG_WRITE_REG,
    DVT_MSG_READ_REG,
    DVT_MSG_WRITE_MEMORY,
    DVT_MSG_READ_MEMORY,
    DVT_MSG_CONFIG_PIPE,
    DVT_MSG_WRITE_PIPE_FRAME_TRANSFER,
    DVT_MSG_CREATE_FRAME,
    DVT_MSG_NVI_WRITE_DATA,
    DVT_MSG_NVI_READ_DATA,
    DVT_MSG_NVI_WRITE_BURST_DATA,
    DVT_MSG_NVI_READ_BURST_DATA,
    DVT_MSG_GET_EEPROM_FIELD_SIZE,
    DVT_MSG_GET_EEPROM_TABLE_SIZE,
    DVT_MSG_GET_EEPROM_TABLE_DIR,
    DVT_MSG_READ_EEPROM_FIELD,
    DVT_MSG_WRITE_EEPROM_FIELD,
    DVT_MSG_READ_EEPROM_TABLE,
    DVT_MSG_WRITE_EEPROM_TABLE,
    DVT_MSG_REMOVE_EEPROM_TABLE,

    DVT_MSG_START_STOP_TX_PACKET_GEN,
    DVT_MSG_GATHER_INFO,
    DVT_MSG_CRC_CHECK_ENABLE,

    DVT_MSG_INIT_ALL,

    DVT_MSG_RESET_COUNTERS,
    DVT_MSG_DUMP_BD_PDU,
    DVT_MSG_SIMPLE_MAC_CONFIG,
    DVT_MSG_SIMPLE_BSS_CONFIG,
    DVT_MSG_ADD_SIMPLE_STATION,
    DVT_MSG_DELETE_STATION,
    DVT_MSG_GET_STATION_TABLE,
    DVT_MSG_GET_DPU_SIGNATURE,
    DVT_MSG_SET_STA_KEY,
    DVT_MSG_SET_BSS_KEY,
    DVT_MSG_ADD_STATION,
    DVT_MSG_BSS_CONFIG,

    DVT_MSG_MEM_TEST,
    DVT_MSG_SET_EEPROM_BURST_PAGE_SIZE,
    DVT_MSG_GET_TX_COUNT_PER_RATE,
    DVT_MSG_GET_RX_COUNT_PER_RATE,
    DVT_MSG_GET_RX_COUNTERS,
    DVT_MSG_READ_REGS,
    DVT_MSG_GET_COUNT_PER_STA,
    DVT_MSG_GET_RX_PMI_RATE_IDX_MISMATCH_COUNT,
    DVT_MSG_WRITE_BEACON_TO_MEMORY,
    DVT_MSG_GET_SMAC_RUNTIME_STATS,
	DVT_MSG_DXE_TIMESTAMP_ENABLE,

    DVT_MAX_MSG_ID
} eDvtMsgId;

// Message parameter definitions:
typedef struct
{
    tANI_U32 addr;
    tANI_U32 value;
} tDvtMsgWriteReg;

typedef struct
{
    tANI_U32 addr;
    tANI_U32 value;
} tDvtMsgReadReg;

typedef struct
{
    tANI_U32 macDestAddr;
    tANI_U8 pBuf[MAX_BUFSIZE];
    tANI_U32 bufSize;
} tDvtMsgWriteMemory;

typedef struct
{
    tANI_U32 macSourceAddr;
    tANI_U8 pBuf[MAX_BUFSIZE];
    tANI_U32 bufSize;
} tDvtMsgReadMemory;

typedef struct
{
    ePipes pipe;
    sDvtPipeCfg pipeCfg;
} tDvtMsgConfigPipe;


typedef struct
{
    tANI_U32 countPackets;
    tWNIAPI_DOT11_SEND_PACKET pPacketArray[MAX_NUM_PACKETS_PER_TRANSFER];
} tDvtMsgWritePipeFrameTransfer4UserMode;

typedef struct
{
    tANI_U32 countPackets;
    tDvtSendPacket pPacketArray[MAX_NUM_PACKETS_PER_TRANSFER];
} tDvtMsgWritePipeFrameTransfer;

typedef struct
{
    sDvtFrameSpec spec;
} tDvtMsgCreateFrame;

typedef struct
{
    tANI_U32 eepromOffset;
    tANI_U8 pBuf[MAX_BUFSIZE];
    tANI_U32 nBytes;
} tDvtMsgNviWriteData;

typedef struct
{
    tANI_U32 eepromOffset;
    tANI_U8 pBuf[MAX_BUFSIZE];
    tANI_U32 nBytes;
} tDvtMsgNviReadData;

typedef struct
{
    tANI_U32 eepromOffset;
    tANI_U32 pBuf[MAX_BUFSIZE];
    tANI_U32 nDwords;
} tDvtMsgNviWriteBurstData;

typedef struct
{
    tANI_U32 eepromOffset;
    tANI_U32 pBuf[MAX_BUFSIZE];
    tANI_U32 nDwords;
} tDvtMsgNviReadBurstData;

typedef struct
{
    eEepromField field;
    tANI_U32 fieldSize;
} tDvtMsgGetEepromFieldSize;

typedef struct
{
    eEepromTable table;
    tANI_U32 tableSize;
} tDvtMsgGetEepromTableSize;

typedef struct
{
    eEepromTable table;
    sEepromTableDir dirEntry;
} tDvtMsgGetEepromTableDir;

typedef struct
{
    eEepromField field;
    uEepromFields fieldData;
} tDvtMsgReadEepromField;

typedef struct
{
    eEepromField field;
    uEepromFields fieldData;
} tDvtMsgWriteEepromField;

typedef struct
{
    eEepromTable eepromTable;
    uEepromTables tableData;
    tANI_U32 tableLen;
} tDvtMsgReadEepromTable;

typedef struct
{
    eEepromTable eepromTable;
    uEepromTables tableData;
} tDvtMsgWriteEepromTable;

typedef struct
{
    eEepromTable eepromTable;
} tDvtMsgRemoveEepromTable;

typedef struct
{
    tANI_BOOLEAN startStop;
} tDvtMsgStartStopTxPacketGen;

typedef struct
{
    sDvtGatherInfo dvtInfo;
    tANI_U32       dbgInfoMask;
} tDvtMsgGatherInfo;

typedef struct
{
    tANI_U16        staIdx;
    tANI_U8    dpuDescIndx;
    tANI_U8    dpuSignature;
} tDvtMsgGetDpuSignature;

typedef struct
{
    tSetStaKeyParams keyParams;
} tDvtMsgSetStaKeyParams;

typedef struct
{
    tSetBssKeyParams keyParams;
} tDvtMsgSetBssKeyParams;

typedef struct
{
    tANI_U32 pageSize; //should be less than 256 and multiples of 4
} tDvtMsgSetEepromBurstPageSize;

typedef struct
{
    tANI_U32        crcEnable;
} tDvtMsgCrcCheckEnable;

typedef struct
{
    tANI_U32        dxeTimestampEnable;
} tDvtMsgDxeTimestampEnable;


typedef struct
{
    tANI_U32    dumpMasknIndex;
    tANI_U8     pBuf[HAL_BD_SIZE];
} tDvtMsgDumpBdPdu;

typedef struct
{
    tANI_U32 dummy;
} tDvtMsgInitAll;

typedef struct
{
    sDvtSimpleMacConfig mac;
} tDvtMsgSimpleMacConfig;

typedef struct
{
    sDvtSimpleBssConfig bss;
    tANI_U32 bssIndex;
} tDvtMsgSimpleBssConfig;

typedef struct
{
    tAddBssParams bss;
} tDvtMsgBssConfig;


typedef struct
{
    sDvtSimpleStationConfig sta;
    tANI_U32 stationIndex;
} tDvtMsgAddSimpleStation;

typedef struct
{
    tAddStaParams sta;
} tDvtMsgAddStation;

typedef struct
{
    tANI_U32 stationIndex;
} tDvtMsgDeleteStation;

typedef struct
{
    tDvtMemTest params;
    tANI_U32 failedAddr;
} tDvtMsgMemTest;

typedef struct
{
    sDvtSimpleStationConfig entries[NUM_DEST_STATIONS];
} tDvtMsgGetStaTable;

typedef struct
{
    tANI_U32 cTxCountPerRate[NUM_HAL_PHY_RATES * MAX_TX_PIPES];
    tANI_U32 size;
} tDvtMsgGetTxCountPerRate;

typedef struct
{
    tANI_U32 cRxCountPerRate[NUM_HAL_PHY_RATES * MAX_RX_PIPES];
    tANI_U32 size;
} tDvtMsgGetRxCountPerRate;

typedef struct
{
    tANI_U32 cRxPMIRateIdxMisMatchCount[NUM_HAL_PHY_RATES];
    tANI_U32 size;
} tDvtMsgGetRxPMIRateIdxMisMatchCount;

typedef struct
{
    tDvtRxCounters rxCounters;
    tANI_U8       size;
} tDvtMsgGetRxCounters;

typedef struct
{
    sDvtReadRegs regs;
    tANI_U8 size;
} tDvtMsgReadRegs;

typedef struct
{
    tANI_U32 countPerSta[MAX_TIDS * NUM_DEST_STATIONS];
    tANI_BOOLEAN isTx;
} tDvtMsgGetCountPerSta;

typedef struct
{
    tANI_U8 beacon[MAX_BEACON_SIZE];
    tANI_U16 bssIndex;
    tANI_U32 length;
} tDvtMsgWriteBeaconToMemory;

typedef struct
{
    tDvtSmacRuntimeStat smacRuntimeStats;
    tANI_U8 size; //not used
}tDvtMsgGetSmacRuntimeStats;

typedef union
{
    tDvtMsgWriteReg                         WriteReg;
    tDvtMsgReadReg                          ReadReg;
    tDvtMsgWriteMemory                      WriteMemory;
    tDvtMsgReadMemory                       ReadMemory;
    tDvtMsgConfigPipe                       ConfigPipe;
    tDvtMsgWritePipeFrameTransfer4UserMode  WritePipeFrameTransfer4User; //for user mode only
    tDvtMsgWritePipeFrameTransfer           WritePipeFrameTransfer;
    tDvtMsgCreateFrame                      CreateFrame;
    tDvtMsgNviWriteData                     NviWriteData;
    tDvtMsgNviReadData                      NviReadData;
    tDvtMsgNviWriteBurstData                NviWriteBurstData;
    tDvtMsgNviReadBurstData                 NviReadBurstData;
    tDvtMsgGetEepromFieldSize               GetEepromFieldSize;
    tDvtMsgGetEepromTableSize               GetEepromTableSize;
    tDvtMsgGetEepromTableDir                GetEepromTableDir;
    tDvtMsgReadEepromField                  ReadEepromField;
    tDvtMsgWriteEepromField                 WriteEepromField;
    tDvtMsgReadEepromTable                  ReadEepromTable;
    tDvtMsgWriteEepromTable                 WriteEepromTable;
    tDvtMsgRemoveEepromTable                RemoveEepromTable;
    tDvtMsgStartStopTxPacketGen             StartStopTxPacketGen;
    tDvtMsgGatherInfo                       GatherInfo;
    tDvtMsgInitAll                          InitAll;
    tDvtMsgCrcCheckEnable                   CrcCheckEnable;
	tDvtMsgDxeTimestampEnable				DxeTimestampEnable;
    tDvtMsgDumpBdPdu                        DumpBdPdu;
    tDvtMsgSimpleMacConfig                  SimpleMacConfig;
    tDvtMsgSimpleBssConfig                  SimpleBssConfig;
    tDvtMsgBssConfig                        BssConfig;
    tDvtMsgAddSimpleStation                 AddSimpleStation;
    tDvtMsgAddStation                       AddStation;
    tDvtMsgDeleteStation                    DeleteStation;
    tDvtMsgMemTest                          MemTest;
    tDvtMsgGetStaTable                      GetStationTable;
    tDvtMsgGetDpuSignature                  GetDpuSignature;
    tDvtMsgSetStaKeyParams                  SetStaKeyParams;
    tDvtMsgSetBssKeyParams                  SetBssKeyParams;
    tDvtMsgSetEepromBurstPageSize           SetEepromBurstPageSize;
    tDvtMsgGetTxCountPerRate                GetTxCountPerRate;
    tDvtMsgGetRxCountPerRate                GetRxCountPerRate;
    tDvtMsgGetRxCounters                    GetRxCounters;
    tDvtMsgReadRegs                         ReadRegs;
    tDvtMsgGetCountPerSta                   GetCountPerSta;
    tDvtMsgGetRxPMIRateIdxMisMatchCount     GetRxPMIRateIdxMisMatchCount;
    tDvtMsgWriteBeaconToMemory              WriteBeaconToMemory;
    tDvtMsgGetSmacRuntimeStats              GetSmacRuntimeStats;
} uDvtMsgs;

typedef struct
{
    tANI_U16 msgId;              //eDvtMsgId
    tANI_U16 msgBodyLength;
    eANI_DVT_STATUS msgResponse;
    uDvtMsgs msgBody;
} tDvtMsgbuffer;



/* The following messages are used to queue events to report to the application layer when polled */

typedef enum
{
    DVT_REPORT_TX_XFR_COMPLETE  = 0x5000,

    DVT_MAX_REPORT_ID
}eDvtReportId;

typedef struct
{
    tANI_U32 payloadLength;
    tANI_U8 destAddr[6];
    tANI_U16 reserved;
}tDvtReportTxXfrComplete;

typedef union
{
    tDvtReportTxXfrComplete         TxXfrComplete;
}uDvtReports;

typedef struct
{
    tANI_U16 reportId;              //eDvtReportId
    tANI_U16 reportBodyLength;
    uDvtReports msgBody;
}sDvtReports;

#endif // DVT_MSG_API_H
