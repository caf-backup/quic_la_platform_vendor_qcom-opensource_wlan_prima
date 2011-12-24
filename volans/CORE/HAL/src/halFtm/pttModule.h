/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

   pttModule.h: global variable structure for pttModule
   Author:  Mark Nelson
   Date: 6/27/05

   History -
   Date         Modified by               Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PTTMODULE_H
#define PTTMODULE_H

#include "phyTest.h"
#include <asicWfm.h>
#include <asicTPC.h>
#include <halPhyCfg.h>  //this includes those types that are needed to store the associate NV tables

#include "pttFrameGen.h"


typedef enum
{

    PTT_STATUS_SUCCESS = 0,
    PTT_STATUS_FAILURE = 1,
    PTT_MAX_VAL = 0XFFFFFFFF, //dummy added to change enum to 4 bytes

}eQWPttStatus;

typedef struct
{
    tANI_U16 drvMjr;
    tANI_U16 drvMnr;
    tANI_U16 drvPtch;
    tANI_U16 drvBld;
    tANI_U16 pttMax;
    tANI_U16 pttMin;
    FwVersionInfo fwVer;
}sBuildReleaseParams;

typedef struct
{
    tANI_BOOLEAN agPktsDisabled;             //802.11ag
    tANI_BOOLEAN bPktsDisabled;             //802.11b
    tANI_BOOLEAN slrPktsDisabled;           //SLR rates
    tANI_BOOLEAN rsvd;
}sRxTypesDisabled;

typedef struct
{
    tANI_U32 totalRxPackets;
    tANI_U32 totalMacRxPackets;
    tANI_U32 totalMacFcsErrPackets;
}sRxFrameCounters;




/* GRAB RAM types */
//TODO: Change Grab RAM interface as appropriate to Taurus
typedef enum
{
    GRAB_RAM_RXFIR,
    GRAB_RAM_ADC,
    GRAB_RAM_ADC_80,
    GRAB_RAM_MAX_VAL = 0XFFFFFFFF, //dummy added to change enum to 4 bytes
}eGrabRamType;

#define GRAB_RAM_SIZE 6000
#define GRAB_RAM_SIZE_80MHZ_1_CHAIN 12000



/// Enum used to specify the trigger type for the aniGrabRam API
typedef enum eGramDumpTrigType
{
    eGRAM_DUMP_UNTRIGGERED,
    eGRAM_DUMP_TRIG_ON_11A,
    eGRAM_DUMP_TRIG_ON_11B,
    eGRAM_DUMP_TRIG_ON_11A_OR_11B
} tGramDumpTrigType;

typedef struct
{
    //common to both transmit chains
    eHalPhyRates rate;                  //current rate
    ePhyChanBondState cbState;          //current Channel bonded state

    tANI_U8 channelId;                  //current channel Id
    tANI_U8 pwrTemplateIndex;           //5-bit template index used for the current rate
    tANI_U8 reserved[2];

    //specific transmit chain power
    tTxChainPower txChains[PHY_MAX_TX_CHAINS];                  //output power for Tx chains
}tTxPowerReport;


typedef struct
{
    /*The idea here is to store only those things which cannot be
       handled directly within the individual function calls.
       Most things will go straight to registers or come from registers.
    */
    sPttFrameGenParams frameGenParams;
    tANI_U8 payload[MAX_PAYLOAD_SIZE];

    //Tx Waveform Gen Service
    tANI_U16 numWfmSamples;
    tANI_BOOLEAN wfmEnabled;
    tANI_BOOLEAN wfmStored;

    //Tx Frame Power Service
    tTxGain forcedTxGain[PHY_MAX_TX_CHAINS];      //use TXPWR_OVERRIDE for wfm, and fill gain table otherwise
    tANI_U8 tpcPowerLut[PHY_MAX_TX_CHAINS][TPC_MEM_POWER_LUT_DEPTH];
    tTxGain tpcGainLut[PHY_MAX_TX_CHAINS][TPC_MEM_GAIN_LUT_DEPTH];

    //Tx Frame Gen Service
    tANI_BOOLEAN frameGenEnabled;
    tANI_BOOLEAN phyDbgFrameGen;        //this says use phyDbg for frames - leave this in place until we know that PhyDbg will suffice
    tANI_U8 reserved[2];

    //Rx Gain Service
    sRxChainsAgcEnable agcEnables;

    tANI_U32 *pADCCaptureCache; //pointer to allocate ADC capture cache

    TX_TIMER  adcRssiStatsTimer; //Create adc rssi stat collection timer

    sRxChainsRssi rssi;
}tPttModuleVariables;


#endif /* PTTMODULE_H */
