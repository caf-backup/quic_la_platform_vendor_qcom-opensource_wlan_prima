/*===========================================================================

                       haldump.c

  OVERVIEW:

  This software unit holds the data structure holding the Dump command
  Interface (args..API's).

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


   $Header$$DateTime$$Author$


  when        who     what, where, why
----------    ---    --------------------------------------------------------
 05/2011    spuligil   Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "aniGlobal.h"
#include "logDump.h"
#include "wlan_qct_wda.h"

static tDumpFuncEntry halMenuDumpTable[] = {
   {0,     "HAL Specific (50-299)",                                    NULL},
   //----------------------------
   {0,     "HAL Basic Procedures (60-80)",                             NULL},
   {7,     "HAL:Read Register <address>",                              (tpFunc)WDA_HALDumpCmdReq},
   {8,     "HAL:Write Register <address> <value>",                     (tpFunc)WDA_HALDumpCmdReq},
   {9,     "HAL:Read Memory <address> <# of dwords> <1/2/4 block>",    (tpFunc)WDA_HALDumpCmdReq},
   {10,    "HAL.Basic: zero memory arg1 = address, arg2 = length",     (tpFunc)WDA_HALDumpCmdReq},
   {11,    "HAL:Set CFG <cfg> <value>",                                (tpFunc)WDA_HALDumpCmdReq},
   {12,    "HAL:Get CFG <cfg>",                                        (tpFunc)WDA_HALDumpCmdReq},
   {50,    "HAL.Basic: pMac size details",                             (tpFunc)WDA_HALDumpCmdReq},
#ifdef WLAN_SOFTAP_FEATURE    
   {55,    "send flow contorl frame to fw <staIdx> <memUsage Threshold> <fcConfig>", (tpFunc)WDA_HALDumpCmdReq},    
   {56,    "add sta with Uapsd <staType> <staNum> <bssIdx> <uapsdAcMask>",(tpFunc)WDA_HALDumpCmdReq},        
   {57,    "update probe response template <bssIdx>",                 (tpFunc)WDA_HALDumpCmdReq},    
//   {58,    "set probeRsp IE bitmap for FW: <flag: enable/disable the feature>, , <flag: enableDisableAllIes>",
//                                                                        WDA_HALDumpCmdReq},    
   {59,    "update UAPSD setting for a peer station(staIdx, uapsdACMask, maxSpLen)", (tpFunc)WDA_HALDumpCmdReq},    
#endif    
   {60,    "Test AddSta (staType, staId, bssId, qos_11n)",             (tpFunc)WDA_HALDumpCmdReq},
   {61,    "Test DelSta (staId)",                                      (tpFunc)WDA_HALDumpCmdReq},
   {62,    "Test AddBss (bssType, bssId)",                             (tpFunc)WDA_HALDumpCmdReq},
   {63,    "Test DelBss (bssId)",                                      (tpFunc)WDA_HALDumpCmdReq},
   {64,    "Show all descriptors",                                     (tpFunc)WDA_HALDumpCmdReq},
   {65,    "Show TL Tx/Rx counters",                                   (tpFunc)WDA_HALDumpCmdReq},    
   {66,    "update beacon template (bssIdx)",                          (tpFunc)WDA_HALDumpCmdReq},     
#if (defined(ANI_OS_TYPE_ANDROID) || defined(ANI_OS_TYPE_LINUX))    
   {67,    "transmit frame template (templateFile number < 10)",       (tpFunc)WDA_HALDumpCmdReq},         
   {68,    "transmit frame template using BTQM (staIdx, templateFile number < 10)",      (tpFunc)WDA_HALDumpCmdReq},  
#endif    
   {69,    "set link state of a bss (bssIdx, link sate <= 13)",        (tpFunc)WDA_HALDumpCmdReq},  


   {0,    "Multi-BSS Info (70-80)",        NULL},
   {70,   "Print current active Bss information", 
                                            (tpFunc)WDA_HALDumpCmdReq}, 

   {0,     "PowerSave (90-100)",                                       NULL},
   {90,    "Chip Power down Control <1/0>",                            (tpFunc)WDA_HALDumpCmdReq},
   {91,    "Set Max Ps Poll <0-255>",                                  (tpFunc)WDA_HALDumpCmdReq},
   {93,    "Set timeouts of Fw",                                       (tpFunc)WDA_HALDumpCmdReq},
   {94,    "Set Sleep Times of Fw",                                    (tpFunc)WDA_HALDumpCmdReq},
   {95,    "Enable BcnFilter <0/1> RssiMonitor <0/1>",                 (tpFunc)WDA_HALDumpCmdReq},
   {96,    "HAL.FW: Stop the FW heartbeat",                            (tpFunc)WDA_HALDumpCmdReq},
   {97,    "HAL.FW: toggle RfXo at FW",                                (tpFunc)WDA_HALDumpCmdReq},
   {98,    "HAL.FW: Insert ADU-reinit regEntry <index><Addr><value><hostfilled>",  (tpFunc)WDA_HALDumpCmdReq},
   {99,    "dump RSSI register values>",                               (tpFunc)WDA_HALDumpCmdReq},
   {100,   "Dump Power-save Counters>",                                (tpFunc)WDA_HALDumpCmdReq},
#ifdef WLAN_SOFTAP_FEATURE    
   {101,   "ap link monitor at FW <1/0>",                             (tpFunc)WDA_HALDumpCmdReq},   
   {102,   "ap unknown addr2 handling at FW <1/0>",                   (tpFunc)WDA_HALDumpCmdReq}, 
   {103,   "dump FW stat",                                            (tpFunc)WDA_HALDumpCmdReq},
#endif    
    
   {127,   "HAL: enable/disable BA activity check timer. arg1 = enable(1)/disable(0)",  (tpFunc)WDA_HALDumpCmdReq},

   {0,     "RateAdaptation (180-220)",                                                        NULL},
   {180,   "HAL.RATE: Set STA 20M Rate <staid> <PriRateIdx> <SecRateIdx> <TerRateIdx>", (tpFunc)WDA_HALDumpCmdReq},
   {181,   "HAL.RATE: Get STA 20M Rate <staid>", (tpFunc)WDA_HALDumpCmdReq},
   {182,   "HAL.RATE: Enable/Disable RA Global rates <0: disable, 1:enable>  <tpeRateStart> <tpeRateEnd>",(tpFunc)WDA_HALDumpCmdReq},
   {195,   "HAL.RATE: Set Global RA cfg <0: show help, [id] >  <value>", (tpFunc)WDA_HALDumpCmdReq},
   {200,   "HAL.RATE: Dump STA rate info <startStaIdx> <endStaIdx>",  (tpFunc)WDA_HALDumpCmdReq},
   {202,   "HAL.RATE: Dump HAL rate table & Tx pkt cnt",  (tpFunc)WDA_HALDumpCmdReq},
   {203,   "HAL.RATE: Dump STA supported/valid/retry rates <staid>", (tpFunc)WDA_HALDumpCmdReq },
   {204,   "HAL.RATE: Dump HAL sampling rate table <maxLowRate> <maxHighRate>", (tpFunc)WDA_HALDumpCmdReq},
   {205,   "HAL RATE: Dump the TPE rate entry <rateIndex>", (tpFunc)WDA_HALDumpCmdReq},
   {206,   "HAL.RATE: Enable/Disable periodic RA <1=Enable, 0=Disable>", (tpFunc)WDA_HALDumpCmdReq},
   {207,   "HAL.RATE: Get the stats <staid>", (tpFunc)WDA_HALDumpCmdReq},
   {208,   "HAL.RATE: force transmit power idx <pwrIdx>", (tpFunc)WDA_HALDumpCmdReq},
   {209,   "HAL.RATE: update rate command table", (tpFunc)WDA_HALDumpCmdReq},
   {210,   "HAL.RATE: dump rate to power",                             (tpFunc)WDA_HALDumpCmdReq},

   {0,     "RXP (220-229)",                                            NULL},
   {222,   "HAL.RXP: Print hardware's RXP Binary Search Table",        (tpFunc)WDA_HALDumpCmdReq},

   {0,     "WoWLAN (230-233)",                                         NULL},
   {230,   "send PE->HAL WoWL Enter Req (bcastEnable, disassoc, maxBeacons, maxSleep)", (tpFunc)WDA_HALDumpCmdReq},
   {231,   "send PE->HAL WoWL Exit Req",                               (tpFunc)WDA_HALDumpCmdReq},
   {232,   "send PE->HAL WoWL Add Ptrn (IPaddr[0] IPaddr[1] IPaddr[2] IPaddr[3])", (tpFunc)WDA_HALDumpCmdReq},
   {233,   "send PE->HAL WoWL Remove Ptrn (ptrn id)",                  (tpFunc)WDA_HALDumpCmdReq},

   {0,     "HAL Phy (234-249)",                                        NULL},
   {234,   "Tune RF Channel 1",                                        (tpFunc)WDA_HALDumpCmdReq},
   {235,   "Init Rx DCO cal",                                          (tpFunc)WDA_HALDumpCmdReq},
   {236,   "Firmware init cal",                                        (tpFunc)WDA_HALDumpCmdReq},
   {237,   "Firmware Periodic Channel tuning <1/0>",                   (tpFunc)WDA_HALDumpCmdReq},
   {238,   "update rate2Pwr table <rateIdx> <dBm>",                    (tpFunc)WDA_HALDumpCmdReq},
   {239,   "tpc config cal point <freqIdx><pointIdx><PADC><outputPwr upto two decimals>",  (tpFunc)WDA_HALDumpCmdReq},
   {240,   "modify start/end cal frequencies <freqIdx><chanIdx>",      (tpFunc)WDA_HALDumpCmdReq},
   {241,   "Configure Tpc",                                            (tpFunc)WDA_HALDumpCmdReq},
   {242,   "dump halPhy regs",                                         (tpFunc)WDA_HALDumpCmdReq},
   {243,   "halphySetChainSelect<numTx><numRx>",                       (tpFunc)WDA_HALDumpCmdReq},
   {244,   "getPwr <rateidx><pwrCap>",                                 (tpFunc)WDA_HALDumpCmdReq},
   {245,   "open TPC loop <1/0>",                                      (tpFunc)WDA_HALDumpCmdReq},
   {246,   "Set open loop gain<rfGgain><digGain>",                     (tpFunc)WDA_HALDumpCmdReq},

   {0,     "BTC (250-259)",                                            NULL},
   {250,   "change BTC paramters (WLAN interval, BT interval, mode, action)", (tpFunc)WDA_HALDumpCmdReq},
   {251,   "view BTC paramters",                                       (tpFunc)WDA_HALDumpCmdReq},
   {252,   "set FW log collection filters (module index, log level, event mask)", (tpFunc)WDA_HALDumpCmdReq},
   {253,   "view FW log collection filters",                           (tpFunc)WDA_HALDumpCmdReq},

   {255,   "dump FW Logs",                                             (tpFunc)WDA_HALDumpCmdReq},

#ifdef WLAN_SOFTAP_FEATURE
   {0,     "SAP (260-269)",                                            NULL},
   {260,   "SAP:Enable/Disable AGC Listen Mode <1/0>",                 (tpFunc)WDA_HALDumpCmdReq},
   {266,   "SAP:Config AGC Listen Mode <EDET threshold; 128-disable>", (tpFunc)WDA_HALDumpCmdReq},   
#endif
};


void halDumpInit(tpAniSirGlobal pMac)
{
   logDumpRegisterTable( pMac, &halMenuDumpTable[0],
                    sizeof(halMenuDumpTable)/sizeof(halMenuDumpTable[0]) );
}
#endif
