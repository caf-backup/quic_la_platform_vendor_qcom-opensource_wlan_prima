/**
 *

 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 

   pttMsgApi.h: Contains messages to PTT Module for physical layer testing
   Author:  Mark Nelson
   Date:    6/21/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PTT_MSG_API_H
#define PTT_MSG_API_H

#include "halNv.h"
#include "pttModule.h"
#include "wlan_rf.h"

typedef tANI_U8     tQWPTT_U8;
typedef tANI_S8     tQWPTT_S8;

typedef tANI_U16    tQWPTT_U16;
typedef tANI_S16    tQWPTT_S16;

typedef tANI_U32    tQWPTT_U32;
typedef tANI_S32    tQWPTT_S32;

typedef tANI_U8     tQWPTT_BYTE;
typedef tANI_S9     tQWPTT_S9;


typedef tANI_U8     tQWPTT_BOOLEAN;

#define PTT_MEM_ACCESS_MAX_SIZE 256

//Messages to/from socket or pttApi.c
typedef enum
{
    PTT_MSG_TYPES_BEGIN                          = 0x3000,

    // Init
    PTT_MSG_INIT                                 = PTT_MSG_TYPES_BEGIN,     //extra: internal only

//NV Service
    PTT_MSG_GET_TPC_CAL_STATE_OBSOLETE           = 0x3011,
    PTT_MSG_RESET_TPC_CAL_STATE_OBSOLETE         = 0x3012,

    PTT_MSG_SET_NV_CKSUM_OBSOLETE                = 0x3013,
    PTT_MSG_GET_NV_CKSUM_OBSOLETE                = 0x3014,
    PTT_MSG_GET_NV_TABLE                         = 0x3016,
    PTT_MSG_SET_NV_TABLE                         = 0x3017,
    PTT_MSG_SET_NV_IMAGE_OBSOLETE                = 0x3018,
    PTT_MSG_BLANK_NV                             = 0x3019,
    PTT_MSG_GET_NV_IMAGE_OBSOLETE                = 0x301E,
    PTT_MSG_DEL_NV_TABLE                         = 0x301F,
    PTT_MSG_GET_NV_FIELD                         = 0x3020,
    PTT_MSG_SET_NV_FIELD                         = 0x3021,
    PTT_MSG_STORE_NV_TABLE                       = 0x3022,
    PTT_MSG_SET_REG_DOMAIN                       = 0x3023,

//Device Register Access
    PTT_MSG_DBG_READ_REGISTER                    = 0x3040,
    PTT_MSG_DBG_WRITE_REGISTER                   = 0x3041,
    PTT_MSG_API_WRITE_REGISTER_OBSOLETE          = 0x3042,
    PTT_MSG_API_READ_REGISTER_OBSOLETE           = 0x3043,
    PTT_MSG_DBG_READ_MEMORY                      = 0x3044,
    PTT_MSG_DBG_WRITE_MEMORY                     = 0x3045,

//Device MAC Test Setup
    PTT_MSG_ENABLE_CHAINS                        = 0x304F,
    PTT_MSG_SET_CHANNEL                          = 0x3050,

//Tx Waveform Gen Service
    PTT_MSG_SET_WAVEFORM                         = 0x3071,
    PTT_MSG_SET_TX_WAVEFORM_GAIN                 = 0x3072,
    PTT_MSG_GET_WAVEFORM_POWER_ADC               = 0x3073,
    PTT_MSG_START_WAVEFORM                       = 0x3074,
    PTT_MSG_STOP_WAVEFORM                        = 0x3075,
    PTT_MSG_SET_RX_WAVEFORM_GAIN                 = 0x3076,

//Tx Frame Gen Service
    PTT_MSG_CONFIG_TX_PACKET_GEN                 = 0x3081,
    PTT_MSG_START_STOP_TX_PACKET_GEN             = 0x3082,
    PTT_MSG_POLL_TX_PACKET_PROGRESS_OBSOLETE     = 0x3083,
    PTT_MSG_FRAME_GEN_STOP_IND_OBSOLETE          = 0x3088,
    PTT_MSG_QUERY_TX_STATUS                      = 0x3089,


//Tx Frame Power Service
    PTT_MSG_CLOSE_TPC_LOOP                       = 0x30A0,

    //open loop service
    PTT_MSG_SET_PACKET_TX_GAIN_TABLE             = 0x30A1,
    PTT_MSG_SET_PACKET_TX_GAIN_INDEX             = 0x30A2,
    PTT_MSG_FORCE_PACKET_TX_GAIN                 = 0x30A3,

    //closed loop(CLPC) service
    PTT_MSG_SET_PWR_INDEX_SOURCE                 = 0x30A4,
    PTT_MSG_SET_TX_POWER                         = 0x30A5,
    PTT_MSG_GET_TX_POWER_REPORT                  = 0x30A7,
    PTT_MSG_SAVE_TX_PWR_CAL_TABLE_OBSOLETE       = 0x30A8,
    PTT_MSG_SET_POWER_LUT                        = 0x30A9,
    PTT_MSG_GET_POWER_LUT                        = 0x30AA,
    PTT_MSG_GET_PACKET_TX_GAIN_TABLE             = 0x30AB,
    PTT_MSG_SAVE_TX_PWR_FREQ_TABLE_OBSOLETE      = 0x30AC,
    PTT_MSG_CLPC_TEMP_COMPENSATION_OBSOLETE      = 0x30AD,
    PTT_MSG_UPDATE_TPC_SPLIT_LUT                 = 0x30AE,

//Rx Gain Service
    PTT_MSG_DISABLE_AGC_TABLES                   = 0x30D0,
    PTT_MSG_ENABLE_AGC_TABLES                    = 0x30D1,
    PTT_MSG_SET_AGC_TABLES_OBSOLETE              = 0x30D2,
    PTT_MSG_GET_RX_RSSI                          = 0x30D3,
    PTT_MSG_GET_AGC_TABLE_OBSOLETE               = 0x30D5,

//Rx Frame Catcher Service
    PTT_MSG_SET_RX_DISABLE_MODE                  = 0x30D4,
    PTT_MSG_GET_RX_PKT_COUNTS                    = 0x30E0,
    PTT_MSG_RESET_RX_PACKET_STATISTICS           = 0x30E2,
    PTT_MSG_GET_UNI_CAST_MAC_PKT_RX_RSSI         = 0x30E3,

//Rx Symbol Service
    PTT_MSG_GRAB_RAM                             = 0x30F0,
    PTT_MSG_GRAB_RAM_ONE_CHAIN_OBSOLETE          = 0x30F1,

//Phy Calibration Service
    PTT_MSG_RX_IQ_CAL                            = 0x3100,
    PTT_MSG_RX_DCO_CAL                           = 0x3101,
    PTT_MSG_TX_CARRIER_SUPPRESS_CAL              = 0x3102,
    PTT_MSG_TX_IQ_CAL                            = 0x3103,
    PTT_MSG_EXECUTE_INITIAL_CALS                 = 0x3104,
    PTT_MSG_HDET_CAL                             = 0x3105,
    PTT_MSG_VCO_LINEARITY_CAL_OBSOLETE           = 0x3106,

//Phy Calibration Override Service
    PTT_MSG_SET_TX_CARRIER_SUPPRESS_CORRECT      = 0x3110,
    PTT_MSG_GET_TX_CARRIER_SUPPRESS_CORRECT      = 0x3111,
    PTT_MSG_SET_TX_IQ_CORRECT                    = 0x3112,
    PTT_MSG_GET_TX_IQ_CORRECT                    = 0x3113,
    PTT_MSG_SET_RX_IQ_CORRECT                    = 0x3114,
    PTT_MSG_GET_RX_IQ_CORRECT                    = 0x3115,
    PTT_MSG_SET_RX_DCO_CORRECT                   = 0x3116,
    PTT_MSG_GET_RX_DCO_CORRECT                   = 0x3117,
    PTT_MSG_SET_TX_IQ_PHASE_NV_TABLE_OBSOLETE    = 0x3118,
    PTT_MSG_GET_HDET_CORRECT_OBSOLETE            = 0x3119,

//RF Chip Access
    PTT_MSG_GET_TEMP_ADC                         = 0x3202,
    PTT_MSG_READ_RF_REG                          = 0x3203,
    PTT_MSG_WRITE_RF_REG                         = 0x3204,
    PTT_MSG_GET_RF_VERSION                       = 0x3205,

//Deep sleep support
    PTT_MSG_DEEP_SLEEP                           = 0x3220,
    PTT_MSG_READ_SIF_BAR4_REGISTER               = 0x3221,
    PTT_MSG_WRITE_SIF_BAR4_REGISTER              = 0x3222,
    PTT_MSG_ENTER_FULL_POWER                     = 0x3223,

//Misc
    PTT_MSG_SYSTEM_RESET                         = 0x32A0,  //is there any meaning for this in Gen6?
    PTT_MSG_LOG_DUMP                             = 0x32A1,
    PTT_MSG_GET_BUILD_RELEASE_NUMBER             = 0x32A2,


//Messages for Socket App
    PTT_MSG_ADAPTER_DISABLED_RSP_OBSOLETE        = 0x32A3,
    PTT_MSG_ENABLE_ADAPTER                       = 0x32A4,
    PTT_MSG_DISABLE_ADAPTER                      = 0x32A5,
    PTT_MSG_PAUSE_RSP_OBSOLETE                   = 0x32A6,
    PTT_MSG_CONTINUE_RSP_OBSOLETE                = 0x32A7,

    PTT_MSG_HALPHY_INIT                          = 0x32A8,
    PTT_MSG_TEST_RXIQ_CAL                        = 0x32A9,
    PTT_MSG_START_TONE_GEN                       = 0x32AA,
    PTT_MSG_STOP_TONE_GEN                        = 0x32AB,
    PTT_MSG_RX_IM2_CAL                           = 0x32AC,
    PTT_MSG_SET_RX_IM2_CORRECT                   = 0x31AD,
    PTT_MSG_GET_RX_IM2_CORRECT                   = 0x31AE,
    PTT_MSG_TEST_DPD_CAL                         = 0x32AF,
    PTT_MSG_SET_CALCONTROL_BITMAP                = 0x32B0,

    PTT_MSG_EXIT                                 = 0x32ff,
    PTT_MAX_MSG_ID                               = PTT_MSG_EXIT,
    PTT_MSG_INVALID                              = 0x7fffffff
}ePttMsgId;


#define PTT_MSG_TYPES_BEGIN_30          PTT_MSG_TYPES_BEGIN
#define PTT_MSG_TYPES_BEGIN_31          PTT_MSG_TYPES_BEGIN + 0x100
#define PTT_MSG_TYPES_BEGIN_32          PTT_MSG_TYPES_BEGIN + 0x200


#ifndef tANI_BOOLEAN
#define tANI_BOOLEAN tANI_U8
#endif



/******************************************************************************************************************
    PTT MESSAGES
******************************************************************************************************************/
//Init
typedef struct
{
    tPttModuleVariables ptt;
}tMsgPttMsgInit;

typedef struct
{
    eNvTable nvTable;
    uNvTables tableData;
}tMsgPttGetNvTable;

typedef struct
{
    eNvTable nvTable;
    uNvTables tableData;
}tMsgPttSetNvTable;

typedef struct
{
    eNvTable nvTable;
}tMsgPttDelNvTable;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttBlankNv;

typedef struct
{
    eNvField nvField;
    uNvFields fieldData;
}tMsgPttGetNvField;

typedef struct
{
    eNvField nvField;
    uNvFields fieldData;
}tMsgPttSetNvField;

typedef struct
{
    eNvTable nvTable;
}tMsgPttStoreNvTable;

typedef struct
{
    eRegDomainId regDomainId;
}tMsgPttSetRegDomain;

//Device Register Access
typedef struct
{
    tANI_U32 regAddr;
    tANI_U32 regValue;
}tMsgPttDbgReadRegister;

typedef struct
{
    tANI_U32 regAddr;
    tANI_U32 regValue;
}tMsgPttDbgWriteRegister;

#define PTT_READ_MEM_MAX 1024
typedef struct
{
    tANI_U32 memAddr;
    tANI_U32 nBytes;
    tANI_U32 pMemBuf[PTT_READ_MEM_MAX];  //caller should allocate space
}tMsgPttDbgReadMemory;

typedef struct
{
    tANI_U32 memAddr;
    tANI_U32 nBytes;
    tANI_U32 pMemBuf[PTT_READ_MEM_MAX];
}tMsgPttDbgWriteMemory;

//Device MAC Test Setup
typedef struct
{
    tANI_U32 chId;
    ePhyChanBondState cbState;
}tMsgPttSetChannel;

typedef struct
{
    ePhyChainSelect chainSelect;
}tMsgPttEnableChains;



//Tx Waveform Gen Service
typedef struct
{
    tWaveformSample waveform[MAX_TEST_WAVEFORM_SAMPLES];
    tANI_U16 numSamples;
    tANI_BOOLEAN clk80;
    tANI_U8 reserved[1];
}tMsgPttSetWaveform;

typedef struct
{
    ePhyTxChains txChain;
    tANI_U8 gain;
}tMsgPttSetTxWaveformGain;

typedef struct
{
    ePhyRxChains rxChain;
    tANI_U8 gain;
}tMsgPttSetRxWaveformGain;

typedef struct
{
    sTxChainsPowerAdcReadings txPowerAdc;
}tMsgPttGetWaveformPowerAdc;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttStopWaveform;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttStartWaveform;



//Tx Frame Gen Service
typedef struct
{
    sPttFrameGenParams frameParams;
}tMsgPttConfigTxPacketGen;

typedef struct
{
    tANI_BOOLEAN startStop;
    tANI_U8 reserved[3];
}tMsgPttStartStopTxPacketGen;

typedef struct
{
    sTxFrameCounters numFrames;
    tANI_BOOLEAN status;
    tANI_U8 reserved[3];
}tMsgPttQueryTxStatus;

//Tx Frame Power Service
typedef struct
{
    tANI_BOOLEAN tpcClose;
    tANI_U8 reserved[3];
}tMsgPttCloseTpcLoop;


    //open loop service
typedef struct
{

    ePhyTxChains txChain;
    tANI_U8 minIndex;
    tANI_U8 maxIndex;
    tANI_U8 reserved[2];
    tANI_U8 gainTable[TPC_MEM_GAIN_LUT_DEPTH];
}tMsgPttSetPacketTxGainTable;

typedef struct
{
    ePhyTxChains txChain;
    tANI_U8 gainTable[TPC_MEM_GAIN_LUT_DEPTH];
}tMsgPttGetPacketTxGainTable;

typedef struct
{
    tANI_U8 index;
    tANI_U8 reserved[3];
}tMsgPttSetPacketTxGainIndex;

typedef struct
{
    ePhyTxChains txChain;
    tANI_U8 gain;
    tANI_U8 reserved[3];
}tMsgPttForcePacketTxGain;


typedef struct
{
    ePowerTempIndexSource indexSource;
}tMsgPttSetPwrIndexSource;

typedef struct
{
    ePhyTxPwrRange pwrRange;
    tANI_U32 splitIdx;
}tMsgPttUpdateTpcSplitLut;

typedef struct
{
    t2Decimal dbmPwr;
    tANI_U8 reserved[2];
}tMsgPttSetTxPower;

typedef tTxPowerReport tMsgPttGetTxPowerReport;

typedef struct
{
    ePhyTxChains txChain;

    tANI_U8 minIndex;
    tANI_U8 maxIndex;
    tANI_U8 reserved[2];

    tANI_U8 powerLut[TPC_MEM_POWER_LUT_DEPTH];
}tMsgPttSetPowerLut;

typedef struct
{
    ePhyTxChains txChain;

    tANI_U8 powerLut[TPC_MEM_POWER_LUT_DEPTH];
}tMsgPttGetPowerLut;




//Rx Gain Service
typedef struct
{
    sRxChainsAgcDisable gains;
}tMsgPttDisableAgcTables;


typedef struct
{
    sRxChainsAgcEnable enables;
}tMsgPttEnableAgcTables;

typedef struct
{
    sRxChainsRssi rssi;
}tMsgPttGetRxRssi;


typedef struct
{
    sRxChainsRssi rssi;
}tMsgPttGetUnicastMacPktRxRssi;


//Rx Frame Catcher Service
typedef struct
{
    sRxTypesDisabled disabled;
}tMsgPttSetRxDisableMode;

typedef struct
{
    sRxFrameCounters counters;
}tMsgPttGetRxPktCounts;


typedef struct
{
    tANI_U32 notUsed;
}tMsgPttResetRxPacketStatistics;





//ADC Sample Service
typedef struct
{
    tANI_U32 startSample;   //index of first requested sample, 0 causes new capture
    tANI_U32 numSamples;    //number of samples to transfer to host
    eGrabRamSampleType sampleType;
    tGrabRamSample grabRam[MAX_REQUESTED_GRAB_RAM_SAMPLES];
}tMsgPttGrabRam;


//Phy Calibration Service
typedef struct
{
    sRxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttRxIqCal;

typedef struct
{
    tRxChainsDcoCorrections calValues;
    tANI_U8 gain;
}tMsgPttRxDcoCal;

typedef struct
{
    tRxChainsIm2Corrections calValues;
    tANI_U8 im2CalOnly;
}tMsgPttRxIm2Cal;

typedef struct
{
    sTxChainsLoCorrections calValues;
    eGainSteps gain;
}tMsgPttTxCarrierSuppressCal;

typedef struct
{
    sTxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttTxIqCal;

typedef struct
{
    tANI_U32 unused;
}tMsgPttExecuteInitialCals;

typedef struct
{
    sRfHdetCalValues hdetCalValues;
}tMsgPttHdetCal;

//Phy Calibration Override Service
typedef struct
{
    sTxChainsLoCorrections calValues;
    eGainSteps gain;
}tMsgPttSetTxCarrierSuppressCorrect;

typedef struct
{
    sTxChainsLoCorrections calValues;
    eGainSteps gain;
}tMsgPttGetTxCarrierSuppressCorrect;

typedef struct
{
    sTxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttSetTxIqCorrect;

typedef struct
{
    sTxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttGetTxIqCorrect;

typedef struct
{
    sRxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttSetRxIqCorrect;

typedef struct
{
    sRxChainsIQCalValues calValues;
    eGainSteps gain;
}tMsgPttGetRxIqCorrect;

typedef struct
{
    tRxChainsDcoCorrections calValues;
    tANI_U8 gain;
}tMsgPttSetRxDcoCorrect;

typedef struct
{
    tRxChainsDcoCorrections calValues;
    tANI_U8 gain;
}tMsgPttGetRxDcoCorrect;

typedef struct
{
    tRxChainsIm2Corrections calValues;
    tANI_U8 dummy;
}tMsgPttSetRxIm2Correct;

typedef struct
{
    tRxChainsIm2Corrections calValues;
    tANI_U8 dummy;
}tMsgPttGetRxIm2Correct;

typedef struct
{
    eRfTempSensor tempSensor;
    tTempADCVal tempAdc;
    tANI_U8 reserved[4 - sizeof(tTempADCVal)];
}tMsgPttGetTempAdc;

typedef struct
{
    tANI_U32 addr;
    tANI_U32 mask;
    tANI_U32 shift;
    tANI_U32 value;
}tMsgPttReadRfField;

typedef struct
{
    tANI_U32 addr;
    tANI_U32 mask;
    tANI_U32 shift;
    tANI_U32 value;
}tMsgPttWriteRfField;

//SIF bar4 Register Access
typedef struct
{
    tANI_U32 sifRegAddr;
    tANI_U32 sifRegValue;
}tMsgPttReadSifBar4Register;

typedef struct
{
    tANI_U32 sifRegAddr;
    tANI_U32 sifRegValue;
}tMsgPttWriteSifBar4Register;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttDeepSleep;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttEnterFullPower;

//Misc.
typedef struct
{
    tANI_U32 notUsed;
}tMsgPttSystemReset;

typedef struct
{
    tANI_U32 cmd;
    tANI_U32 arg1;
    tANI_U32 arg2;
    tANI_U32 arg3;
    tANI_U32 arg4;
}tMsgPttLogDump;

typedef struct
{
    sBuildReleaseParams relParams;
}tMsgPttGetBuildReleaseNumber;

typedef struct
{
    tANI_U32 revId;
}tMsgPttGetRFVersion;

#ifdef VERIFY_HALPHY_SIMV_MODEL
typedef struct
{
    tANI_U32 option; //dummy variable
}tMsgPttCalControlBitmap;

typedef struct
{
    tANI_U32 option; //dummy variable
}tMsgPttHalPhyInit;

typedef struct
{
    tANI_U32 option; //dummy variable
}tMsgPttRxIQTest;

typedef struct
{
    tANI_U8 txGain;
}tMsgPttDpdTest;

typedef struct
{
    tANI_U8 lutIdx;
    tANI_U8 band;
} tMsgPttStartToneGen;

typedef struct
{
    tANI_U32 option; //dummy variable
}tMsgPttStopToneGen;
#endif

/******************************************************************************************************************
    END OF PTT MESSAGES
******************************************************************************************************************/

typedef union pttMsgUnion
{
    tMsgPttMsgInit                                  MsgInit;
    tMsgPttGetNvTable                               GetNvTable;
    tMsgPttSetNvTable                               SetNvTable;
    tMsgPttDelNvTable                               DelNvTable;
    tMsgPttBlankNv                                  BlankNv;
    tMsgPttStoreNvTable                             StoreNvTable;
    tMsgPttSetRegDomain                             SetRegDomain;
    tMsgPttGetNvField                               GetNvField;
    tMsgPttSetNvField                               SetNvField;
    tMsgPttDbgReadRegister                          DbgReadRegister;
    tMsgPttDbgWriteRegister                         DbgWriteRegister;
    tMsgPttDbgReadMemory                            DbgReadMemory;
    tMsgPttDbgWriteMemory                           DbgWriteMemory;
    tMsgPttEnableChains                             EnableChains;
    tMsgPttSetChannel                               SetChannel;
    tMsgPttSetWaveform                              SetWaveform;
    tMsgPttSetTxWaveformGain                        SetTxWaveformGain;
    tMsgPttGetWaveformPowerAdc                      GetWaveformPowerAdc;
    tMsgPttStartWaveform                            StartWaveform;
    tMsgPttStopWaveform                             StopWaveform;
    tMsgPttSetRxWaveformGain                        SetRxWaveformGain;
    tMsgPttConfigTxPacketGen                        ConfigTxPacketGen;
    tMsgPttStartStopTxPacketGen                     StartStopTxPacketGen;
    tMsgPttQueryTxStatus                            QueryTxStatus;
    tMsgPttCloseTpcLoop                             CloseTpcLoop;
    tMsgPttSetPacketTxGainTable                     SetPacketTxGainTable;
    tMsgPttGetPacketTxGainTable                     GetPacketTxGainTable;
    tMsgPttSetPacketTxGainIndex                     SetPacketTxGainIndex;
    tMsgPttForcePacketTxGain                        ForcePacketTxGain;
    tMsgPttSetPwrIndexSource                        SetPwrIndexSource;
    tMsgPttUpdateTpcSplitLut                        UpdateTpcSplitLut;
    tMsgPttSetTxPower                               SetTxPower;
    tMsgPttGetTxPowerReport                         GetTxPowerReport;
    tMsgPttSetPowerLut                              SetPowerLut;
    tMsgPttGetPowerLut                              GetPowerLut;
    tMsgPttDisableAgcTables                         DisableAgcTables;
    tMsgPttEnableAgcTables                          EnableAgcTables;
    tMsgPttGetRxRssi                                GetRxRssi;
    tMsgPttGetUnicastMacPktRxRssi                   GetUnicastMacPktRxRssi;
    tMsgPttSetRxDisableMode                         SetRxDisableMode;
    tMsgPttGetRxPktCounts                           GetRxPktCounts;
    tMsgPttResetRxPacketStatistics                  ResetRxPacketStatistics;
    tMsgPttGrabRam                                  GrabRam;
    tMsgPttRxIqCal                                  RxIqCal;
    tMsgPttRxDcoCal                                 RxDcoCal;
    tMsgPttRxIm2Cal                                 RxIm2Cal;
    tMsgPttTxCarrierSuppressCal                     TxCarrierSuppressCal;
    tMsgPttTxIqCal                                  TxIqCal;
    tMsgPttExecuteInitialCals                       ExecuteInitialCals;
    tMsgPttHdetCal                                  HdetCal;
    tMsgPttSetTxCarrierSuppressCorrect              SetTxCarrierSuppressCorrect;
    tMsgPttGetTxCarrierSuppressCorrect              GetTxCarrierSuppressCorrect;
    tMsgPttSetTxIqCorrect                           SetTxIqCorrect;
    tMsgPttGetTxIqCorrect                           GetTxIqCorrect;
    tMsgPttSetRxIqCorrect                           SetRxIqCorrect;
    tMsgPttGetRxIqCorrect                           GetRxIqCorrect;
    tMsgPttSetRxDcoCorrect                          SetRxDcoCorrect;
    tMsgPttGetRxDcoCorrect                          GetRxDcoCorrect;
    tMsgPttSetRxIm2Correct                          SetRxIm2Correct;
    tMsgPttGetRxIm2Correct                          GetRxIm2Correct;
    tMsgPttGetTempAdc                               GetTempAdc;
    tMsgPttReadRfField                              ReadRfField;
    tMsgPttWriteRfField                             WriteRfField;
#ifdef VERIFY_HALPHY_SIMV_MODEL
    tMsgPttCalControlBitmap                         SetCalControlBitmap;
    tMsgPttHalPhyInit                               InitOption;
    tMsgPttRxIQTest                                 RxIQTest;
    tMsgPttDpdTest                                  DpdTest;
    tMsgPttStartToneGen                             StartToneGen;
    tMsgPttStopToneGen                              StopToneGen;
#endif
    tMsgPttDeepSleep                                DeepSleep;
    tMsgPttReadSifBar4Register                      ReadSifBar4Register;
    tMsgPttWriteSifBar4Register                     WriteSifBar4Register;
    tMsgPttEnterFullPower                           EnterFullPower;
    tMsgPttSystemReset                              SystemReset;
    tMsgPttLogDump                                  LogDump;
    tMsgPttGetBuildReleaseNumber                    GetBuildReleaseNumber;
    tMsgPttGetRFVersion                             GetRFVersion;

}uPttMsgs;





typedef struct
{
    tANI_U16 msgId;
    tANI_U16 msgBodyLength;     //actually, the length of all the fields in this structure
    eQWPttStatus msgResponse;
    uPttMsgs msgBody;
}tPttMsgbuffer;

#endif

