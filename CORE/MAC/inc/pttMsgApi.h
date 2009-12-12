/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   pttMsgApi.h: Contains messages to PTT Module for physical layer testing
   Author:  Mark Nelson
   Date:    6/21/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PTT_MSG_API_H
#define PTT_MSG_API_H

#include "halEeprom.h"
#include "pttModule.h"
#include "rfQuasar.h"

#define PTT_MEM_ACCESS_MAX_SIZE 256

//Messages to/from socket or pttApi.c
typedef enum
{
    PTT_MSG_TYPES_BEGIN                          = 0x3000,

    // Init
    PTT_MSG_INIT                                 = PTT_MSG_TYPES_BEGIN,     //extra: internal only

//EEPROM Service
    PTT_MSG_GET_TPC_CAL_STATE                    = 0x3011,
    PTT_MSG_RESET_TPC_CAL_STATE                  = 0x3012,

    PTT_MSG_SET_EEPROM_CKSUM                     = 0x3013,
    PTT_MSG_GET_EEPROM_CKSUM                     = 0x3014,

    PTT_MSG_GET_EEPROM_TABLE                     = 0x3016,
    PTT_MSG_SET_EEPROM_TABLE                     = 0x3017,
    PTT_MSG_SET_EEPROM_IMAGE                     = 0x3018,  //extra: internal only
    PTT_MSG_BLANK_EEPROM                         = 0x3019,
    PTT_MSG_GET_EEPROM_IMAGE                     = 0x301E,
    PTT_MSG_DEL_EEPROM_TABLE                     = 0x301F,  //extra: update to new tables
    PTT_MSG_GET_EEPROM_FIELD                     = 0x3020,  //extra: update to new fields
    PTT_MSG_SET_EEPROM_FIELD                     = 0x3021,  //extra: update to new fields
    PTT_MSG_STORE_EEPROM_TABLE                   = 0x3022,
    PTT_MSG_SET_REG_DOMAIN                       = 0x3023,

//Device Register Access
    PTT_MSG_DBG_READ_REGISTER                    = 0x3040,  //extra: internal only
    PTT_MSG_DBG_WRITE_REGISTER                   = 0x3041,  //extra: internal only
    PTT_MSG_API_WRITE_REGISTER                   = 0x3042,
    PTT_MSG_API_READ_REGISTER                    = 0x3043,
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

//Tx Frame Gen Service
    PTT_MSG_CONFIG_TX_PACKET_GEN                 = 0x3081,
    PTT_MSG_START_STOP_TX_PACKET_GEN             = 0x3082,
    PTT_MSG_POLL_TX_PACKET_PROGRESS              = 0x3083,
    PTT_MSG_FRAME_GEN_STOP_IND                   = 0x3088,  //extra: internal only sent to application asyncronously
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
    PTT_MSG_SAVE_TX_PWR_CAL_TABLE                = 0x30A8,
    PTT_MSG_SET_POWER_LUT                        = 0x30A9,
    PTT_MSG_GET_POWER_LUT                        = 0x30AA,
    PTT_MSG_GET_PACKET_TX_GAIN_TABLE             = 0x30AB,
    PTT_MSG_SAVE_TX_PWR_FREQ_TABLE               = 0x30AC,
    PTT_MSG_CLPC_TEMP_COMPENSATION               = 0x30AD,

//Rx Gain Service
    PTT_MSG_DISABLE_AGC_TABLES                   = 0x30D0,
    PTT_MSG_ENABLE_AGC_TABLES                    = 0x30D1,
    PTT_MSG_SET_AGC_TABLES                       = 0x30D2,
    PTT_MSG_GET_RX_RSSI                          = 0x30D3,
    PTT_MSG_GET_AGC_TABLE                        = 0x30D5,

//Rx Frame Catcher Service
    PTT_MSG_SET_RX_DISABLE_MODE                  = 0x30D4,
    PTT_MSG_GET_RX_PKT_COUNTS                    = 0x30E0,
    PTT_MSG_RESET_RX_PACKET_STATISTICS           = 0x30E2,

//Rx Symbol Service
    PTT_MSG_GRAB_RAM                             = 0x30F0,
    PTT_MSG_GRAB_RAM_ONE_CHAIN                   = 0x30F1,

//Phy Calibration Service
    PTT_MSG_RX_IQ_CAL                            = 0x3100,
    PTT_MSG_RX_DCO_CAL                           = 0x3101,
    PTT_MSG_TX_CARRIER_SUPPRESS_CAL              = 0x3102,
    PTT_MSG_TX_IQ_CAL                            = 0x3103,
    PTT_MSG_EXECUTE_INITIAL_CALS                 = 0x3104,

//Phy Calibration Override Service
    PTT_MSG_SET_TX_CARRIER_SUPPRESS_CORRECT      = 0x3110,
    PTT_MSG_GET_TX_CARRIER_SUPPRESS_CORRECT      = 0x3111,
    PTT_MSG_SET_TX_IQ_CORRECT                    = 0x3112,  //changed
    PTT_MSG_GET_TX_IQ_CORRECT                    = 0x3113,  //changed
    PTT_MSG_SET_RX_IQ_CORRECT                    = 0x3114,  //changed
    PTT_MSG_GET_RX_IQ_CORRECT                    = 0x3115,  //changed
    PTT_MSG_SET_RX_DCO_CORRECT                   = 0x3116,
    PTT_MSG_GET_RX_DCO_CORRECT                   = 0x3117,
    PTT_MSG_SET_TX_IQ_PHASE_EEPROM_TABLE         = 0x3118,

//RF Chip Access
    PTT_MSG_GET_TEMP_ADC                         = 0x3202,
    PTT_MSG_READ_RF_REG                          = 0x3203,  //extra: internal only
    PTT_MSG_WRITE_RF_REG                         = 0x3204,  //extra: internal only


//Misc
    PTT_MSG_SYSTEM_RESET                         = 0x32A0,
    PTT_MSG_LOG_DUMP                             = 0x32A1,


//Messages for Socket App
    PTT_MSG_ADAPTER_DISABLED_RSP                 = 0x32A3,  //extra: ?
    PTT_MSG_ENABLE_ADAPTER                       = 0x32A4,  //extra: ?
    PTT_MSG_DISABLE_ADAPTER                      = 0x32A5,  //extra: ?
    PTT_MSG_PAUSE_RSP                            = 0x32A6,  //extra: ?
    PTT_MSG_CONTINUE_RSP                         = 0x32A7,  //extra: ?

    PTT_MSG_EXIT                                 = 0x32ff,  //extra: ?
    PTT_MAX_MSG_ID                               = PTT_MSG_EXIT
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
    eTpcCalState tpcCalState;
}tMsgPttGetTpcCalState;

typedef struct
{
    eTpcCalState tpcCalState;
}tMsgPttResetTpcCalState;


typedef struct
{
    tANI_U32 cksum;
    tANI_BOOLEAN isFixPart;
}tMsgPttSetEepromCksum;

typedef struct
{
    tANI_U32 cksum;
    tANI_U32 computedCksum;
    tANI_BOOLEAN isFixPart;
}tMsgPttGetEepromCksum;


typedef struct
{
    eEepromTable eepromTable;
    uEepromTables tableData;
}tMsgPttGetEepromTable;

typedef struct
{
    eEepromTable eepromTable;
    uEepromTables tableData;
}tMsgPttSetEepromTable;

typedef struct
{
    eEepromTable eepromTable;
}tMsgPttDelEepromTable;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttBlankEeprom;

typedef struct
{
    tANI_U32 imgOffset;
    tANI_U32 imgLen;
    tANI_U8 imgBuf[1];  // caller should allocate space
    tANI_BOOLEAN toCache;
}tMsgPttSetEepromImage;

typedef struct
{
    tANI_U32 imgOffset;
    tANI_U32 imgLen;
    tANI_U8 imgBuf[1];  // caller should allocate space
    tANI_BOOLEAN fromCache;
}tMsgPttGetEepromImage;

typedef struct
{
    eEepromField eepromField;
    uEepromFields fieldData;
}tMsgPttGetEepromField;

typedef struct
{
    eEepromField eepromField;
    uEepromFields fieldData;
}tMsgPttSetEepromField;

typedef struct
{
    eEepromTable eepromTable;
}tMsgPttStoreEepromTable;

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

// Register access
typedef struct
{
    eApiRegister regId;
    tANI_U32 regValue;
}tMsgPttApiWriteRegister;

typedef struct
{
    eApiRegister regId;
    tANI_U32 regValue;
}tMsgPttApiReadRegister;

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
    tANI_U8 tx0PowerAdc;
    tANI_U8 tx1PowerAdc;
    tANI_U8 reserved[2];
}tMsgPttGetWaveformPowerAdc;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttStartWaveform;

typedef struct
{
    tANI_U32 notUsed;
}tMsgPttStopWaveform;




//Tx Frame Gen Service
typedef struct
{
    sPttFrameGenParams frameParams;
}tMsgPttConfigTxPacketGen;

typedef struct
{
    tANI_BOOLEAN startStop;
}tMsgPttStartStopTxPacketGen;

typedef struct
{
    sTxFrameCounters numFrames;
    tANI_BOOLEAN status;
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


typedef struct
{
    tANI_U8 numTpcCalFreqs;
    tANI_U8 reserved[3];
    tTpcFreqData table[NUM_LEGIT_RF_CHANNELS];
}tMsgPttSaveTxPwrFreqTable;

typedef struct
{
    tANI_BOOLEAN enabled;
}tMsgPttClpcTempCompensation;




//Rx Gain Service
typedef struct
{
    tANI_U8 rx0Gain;
    tANI_U8 rx1Gain;
    tANI_U8 rx2Gain;
    tANI_U8 reserved[1];
}tMsgPttDisableAgcTables;


typedef struct
{
    tANI_BOOLEAN rx0;
    tANI_BOOLEAN rx1;
    tANI_BOOLEAN rx2;
    tANI_U8 reserved[1];
}tMsgPttEnableAgcTables;

typedef struct
{
    ePhyRxChains rxChain;

    tANI_U8 minIndex;
    tANI_U8 maxIndex;
    tANI_U8 reserved[2];

    tRxGain rxGainTable[NUM_AGC_GAINS];
}tMsgPttSetAgcTable;

typedef struct
{
    ePhyRxChains rxChain;
    tRxGain rxGainTable[NUM_AGC_GAINS];
}tMsgPttGetAgcTable;


typedef struct
{
    tANI_U8 rx0Rssi;
    tANI_U8 rx1Rssi;
    tANI_U8 rx2Rssi;
    tANI_U8 reserved[1];
}tMsgPttGetRxRssi;




//Rx Frame Catcher Service
typedef struct
{
    tANI_BOOLEAN aPktsDisabled;
    tANI_BOOLEAN bPktsDisabled;
    tANI_BOOLEAN chanBondPktsDisabled;
    tANI_U8 reserved[1];
}tMsgPttSetRxDisableMode;

typedef struct
{
    tANI_U32 numRxPackets;
}tMsgPttGetRxPktCounts;


typedef struct
{
    tANI_U32 notUsed;
}tMsgPttResetRxPacketStatistics;





//Rx Symbol Service
typedef struct
{
    tANI_U32 startSample;   //index of first requested sample, 0 causes new capture
    tANI_U32 numSamples;    //number of samples to transfer to host
    tGrabRamSample grabRam[MAX_REQUESTED_GRAB_RAM_SAMPLES];
}tMsgPttGrabRam;


//Phy Calibration Service
typedef struct
{
    sIQCalValues rx0;
    sIQCalValues rx1;
    sIQCalValues rx2;
    sIQCalValues reserved;
    eGainSteps gain;
}tMsgPttRxIqCal;

typedef struct
{
    tRxDcoCorrect rx0Dco;
    tRxDcoCorrect rx1Dco;
    tRxDcoCorrect rx2Dco;
    tRxDcoCorrect reserved;
    eGainSteps gain;
}tMsgPttRxDcoCal;

typedef struct
{
    tTxLoCorrect tx0;
    tTxLoCorrect tx1;
    eGainSteps gain;
}tMsgPttTxCarrierSuppressCal;

typedef struct
{
    sIQCalValues tx0;
    sIQCalValues tx1;
    eGainSteps gain;
}tMsgPttTxIqCal;

typedef struct
{
    tANI_U32 unused;
}tMsgPttExecuteInitialCals;

//Phy Calibration Override Service
typedef struct
{
    tTxLoCorrect tx0;
    tTxLoCorrect tx1;
    eGainSteps gain;
}tMsgPttSetTxCarrierSuppressCorrect;

typedef struct
{
    tTxLoCorrect tx0;
    tTxLoCorrect tx1;
    eGainSteps gain;
}tMsgPttGetTxCarrierSuppressCorrect;

typedef struct
{
    ePhyTxChains txChain;
    sIQCalValues correct;
    eGainSteps gain;
}tMsgPttSetTxIqCorrect;

typedef struct
{
    ePhyTxChains txChain;
    sIQCalValues correct;
    eGainSteps gain;
}tMsgPttGetTxIqCorrect;

typedef struct
{
    ePhyRxChains rxChain;
    sIQCalValues correct;
    eGainSteps gain;
}tMsgPttSetRxIqCorrect;

typedef struct
{
    ePhyRxChains rxChain;
    sIQCalValues correct;
    eGainSteps gain;
}tMsgPttGetRxIqCorrect;

typedef struct
{
    tRxDcoCorrect rx0Dco;
    tRxDcoCorrect rx1Dco;
    tRxDcoCorrect rx2Dco;
    tRxDcoCorrect reserved;
    eGainSteps gain;
}tMsgPttSetRxDcoCorrect;

typedef struct
{
    tRxDcoCorrect rx0Dco;
    tRxDcoCorrect rx1Dco;
    tRxDcoCorrect rx2Dco;
    tRxDcoCorrect reserved;
    eGainSteps gain;
}tMsgPttGetRxDcoCorrect;

typedef struct
{
    ePhyTxChains txChain;

    tANI_U8 tempAdc;
    tANI_U8 reserved[3];
}tMsgPttGetTempAdc;

typedef struct
{
    eQuasarFields quasarField;
    tANI_U32 value;
}tMsgPttReadQuasarField;

typedef struct
{
    eQuasarFields quasarField;
    tANI_U32 value;
}tMsgPttWriteQuasarField;

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


/******************************************************************************************************************
    END OF PTT MESSAGES
******************************************************************************************************************/

typedef union pttMsgUnion
{
    tMsgPttMsgInit                                   MsgInit;
    tMsgPttGetTpcCalState                            GetTpcCalState;
    tMsgPttResetTpcCalState                          ResetTpcCalState;
    tMsgPttSetEepromCksum                            SetEepromCksum;
    tMsgPttGetEepromCksum                            GetEepromCksum;
    tMsgPttGetEepromTable                            GetEepromTable;
    tMsgPttSetEepromTable                            SetEepromTable;
    tMsgPttDelEepromTable                            DelEepromTable;
    tMsgPttSetEepromImage                            SetEepromImage;
    tMsgPttGetEepromImage                            GetEepromImage;
    tMsgPttBlankEeprom                               BlankEeprom;
    tMsgPttStoreEepromTable                          StoreEepromTable;
    tMsgPttSetRegDomain                              SetRegDomain;   
    tMsgPttGetEepromField                            GetEepromField;
    tMsgPttSetEepromField                            SetEepromField;
    tMsgPttDbgReadRegister                           DbgReadRegister;
    tMsgPttDbgWriteRegister                          DbgWriteRegister;
    tMsgPttApiWriteRegister                          ApiWriteRegister;
    tMsgPttApiReadRegister                           ApiReadRegister;
    tMsgPttDbgReadMemory                             DbgReadMemory;
    tMsgPttDbgWriteMemory                            DbgWriteMemory;
    tMsgPttEnableChains                              EnableChains;
    tMsgPttSetChannel                                SetChannel;
    tMsgPttSetWaveform                               SetWaveform;
    tMsgPttSetTxWaveformGain                         SetTxWaveformGain;
    tMsgPttGetWaveformPowerAdc                       GetWaveformPowerAdc;
    tMsgPttStartWaveform                             StartWaveform;
    tMsgPttStopWaveform                              StopWaveform;
    tMsgPttConfigTxPacketGen                         ConfigTxPacketGen;
    tMsgPttStartStopTxPacketGen                      StartStopTxPacketGen;
    tMsgPttQueryTxStatus                             QueryTxStatus;
    tMsgPttCloseTpcLoop                              CloseTpcLoop;
    tMsgPttSetPacketTxGainTable                      SetPacketTxGainTable;
    tMsgPttGetPacketTxGainTable                      GetPacketTxGainTable;
    tMsgPttSetPacketTxGainIndex                      SetPacketTxGainIndex;
    tMsgPttForcePacketTxGain                         ForcePacketTxGain;
    tMsgPttSetPwrIndexSource                         SetPwrIndexSource;
    tMsgPttSetTxPower                                SetTxPower;
    tMsgPttGetTxPowerReport                          GetTxPowerReport;
    tMsgPttSetPowerLut                               SetPowerLut;
    tMsgPttGetPowerLut                               GetPowerLut;
    tMsgPttSaveTxPwrFreqTable                        SaveTxPwrFreqTable;
    tMsgPttDisableAgcTables                          DisableAgcTables;
    tMsgPttEnableAgcTables                           EnableAgcTables;
    tMsgPttSetAgcTable                               SetAgcTable;
    tMsgPttGetAgcTable                               GetAgcTable;
    tMsgPttGetRxRssi                                 GetRxRssi;
    tMsgPttSetRxDisableMode                          SetRxDisableMode;
    tMsgPttGetRxPktCounts                            GetRxPktCounts;
    tMsgPttResetRxPacketStatistics                   ResetRxPacketStatistics;
    tMsgPttGrabRam                                   GrabRam;
    tMsgPttRxIqCal                                   RxIqCal;
    tMsgPttRxDcoCal                                  RxDcoCal;
    tMsgPttTxCarrierSuppressCal                      TxCarrierSuppressCal;
    tMsgPttTxIqCal                                   TxIqCal;
    tMsgPttExecuteInitialCals                        ExecuteInitialCals;
    tMsgPttSetTxCarrierSuppressCorrect               SetTxCarrierSuppressCorrect;
    tMsgPttGetTxCarrierSuppressCorrect               GetTxCarrierSuppressCorrect;
    tMsgPttSetTxIqCorrect                            SetTxIqCorrect;
    tMsgPttGetTxIqCorrect                            GetTxIqCorrect;
    tMsgPttSetRxIqCorrect                            SetRxIqCorrect;
    tMsgPttGetRxIqCorrect                            GetRxIqCorrect;
    tMsgPttSetRxDcoCorrect                           SetRxDcoCorrect;
    tMsgPttGetRxDcoCorrect                           GetRxDcoCorrect;
    tMsgPttGetTempAdc                                GetTempAdc;
    tMsgPttReadQuasarField                           ReadQuasarField;
    tMsgPttWriteQuasarField                          WriteQuasarField;
    tMsgPttSystemReset                               SystemReset;
    tMsgPttLogDump                                   LogDump;

}uPttMsgs;





typedef struct
{
    tANI_U16 msgId;
    tANI_U16 msgBodyLength;
    ePttStatus msgResponse;
    uPttMsgs msgBody;   //msg body follows;
}tPttMsgbuffer;



#endif

