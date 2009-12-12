/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

  
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
#include "asicWfm.h"
#include "halPhyCfg.h"  //this includes those types that are needed to store the associate EEPROM tables

#include "pttFrameGen.h"


typedef enum
{

    PTT_STATUS_SUCCESS = 0,
    PTT_STATUS_FAILURE = 1
    
}ePttStatus;



typedef enum
{
    TPC_NOT_CALIBRATED,
    TPC_2_4GHZ_CALIBRATION_STORED,
    TPC_5GHZ_CALIBRATED_STORED,
    TPC_CALIBRATION_COMPLETE        //both 2.4GHz and 5GHz calibrated
}eTpcCalState;




/* TX Power Calibration & Report Types */

#ifndef PTT_FLOAT
#define PTT_FLOAT tANI_U32
#endif
typedef union
{
    PTT_FLOAT measurement;   //measured values can be passed to the api, but are maintained to 2 decimal places internally
    t2Decimal reported;  //used internally only - reported values only maintain 2 decimals places
}uAbsPwrPrecision;


typedef struct
{
    tANI_U8  temperatureAdc;                //= 5 bit temperature measured at time sample was taken
    tANI_U8  txGain;                        //= 7 bit gain value used to get the power measurement
    tANI_U8  pwrDetAdc;                     //= 8 bit ADC power detect value
    tANI_U8  reserved;
    uAbsPwrPrecision absPowerMeasured;      //= dBm measurement, will be truncated to two decimal places
}tTpcCalPoint;


typedef struct
{
    tANI_U16 numTpcCalPoints;
    tANI_U16 reserved;
    tTpcCalPoint chain[MAX_TPC_CAL_POINTS];
}tTpcChainData;


typedef struct
{
    tANI_U16 freq;                                          //frequency in MHz
    tANI_U16 reserved;
    tTpcChainData empirical[PHY_MAX_TX_CHAINS];  //TPC samples passed in
}tTpcFreqData;


typedef struct
{
    tPowerDetect lut;                   //7-bit value in the power Lookup Table
    tANI_U8 reserved[3];
    
    uAbsPwrPrecision abs;               //LUT value conversion to absolute dBm
}tTxPowerLutOutput;

typedef struct
{
    tANI_U8      gain;                  //8-bit coarse(bits 4-7) & fine(bits 0-3) gain commanded for the current index
    tPowerDetect adc;                   //8-bit power ADC sampled during the packet preamble
    tANI_U8 reserved[2];
    
    tTxPowerLutOutput indexMinMatch;    //minimum LUT matching power that satisfies the power template index setting
    tTxPowerLutOutput indexMaxMatch;    //maximum LUT matching power that satisfies the power template index setting
    tTxPowerLutOutput output;           //output power values corresponding to power ADC index
}tTxChainPower;



typedef struct
{
    //common to both transmit chains
    eHalPhyRates rate;                  //current rate
    ePhyChanBondState cbState;          //current Channel bonded state
    
    tANI_U8 channelId;                  //current channel Id
    tANI_U8 pwrTemplateIndex;           //5-bit template index used for the current rate
    tANI_U8 reserved[2];

    //specific transmit chain power
    tTxChainPower tx0;                  //output power for Tx chain 0
    tTxChainPower tx1;                  //output power for Tx chain 1
}tTxPowerReport;


/* GRAB RAM types */
//TODO: Change Grab RAM interface as appropriate to Taurus
typedef enum
{
    GRAB_RAM_RXFIR,
    GRAB_RAM_ADC,
    GRAB_RAM_ADC_80
}eGrabRamType;

#define GRAB_RAM_SIZE 6000
#define GRAB_RAM_SIZE_80MHZ_1_CHAIN 12000

typedef struct
{
      tIQAdc rx;
      tANI_U8 reserved[2];
      
      tANI_U32 stats;  //holds AGC status and whatever else
}tGrabRamOneChain;


typedef struct

{
    tIQAdc rx0;
    tIQAdc rx1;
    tIQAdc rx2;
    tANI_U8 reserved[2];
      
    tANI_U32 stats;  //holds AGC status and whatever else

}tGrabRam;

#define GRAB_RAM_BUFFER_DEPTH   (12 * 1024)   //maximum grab ram size for channel bonding

/// Enum used to specify the trigger type for the aniGrabRam API
typedef enum eGramDumpTrigType
{
    eGRAM_DUMP_UNTRIGGERED,
    eGRAM_DUMP_TRIG_ON_11A,
    eGRAM_DUMP_TRIG_ON_11B,
    eGRAM_DUMP_TRIG_ON_11A_OR_11B
} tGramDumpTrigType;




typedef enum
{
    eREG_RX_INIT_GAIN,
    eREG_RX_COARSE_STEP,
    eREG_TARGET_BO_ACTIVE,
    eREG_TH_CD,
    eREG_CW_DETECT,
    eREG_MAX // max register numbers
}eApiRegister;


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
    tANI_BOOLEAN rx0AgcEnabled;
    tANI_BOOLEAN rx1AgcEnabled;
    tANI_BOOLEAN rx2AgcEnabled;
    tANI_BOOLEAN rx3AgcEnabled;    //for fourth Rx later
}tPttModuleVariables;


#endif /* PTTMODULE_H */
