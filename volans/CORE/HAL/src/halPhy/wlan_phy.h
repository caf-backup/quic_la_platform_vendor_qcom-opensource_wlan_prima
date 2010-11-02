/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

  
   phy.h: Types needed only for internal phy interfaces.
            Nothing in here is needed for configuration or by halPhy interface clients.
   Author:  Mark Nelson
   Date:    3/4/05

   History - 
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PHY_H
#define PHY_H

#include "halPhy.h"
#include "phyTest.h"
#include <phyTxPower.h>
#include <asicPhyDbg.h> //needed for access to grab ram for rfFilterCal




typedef struct
{
    ePhyChanBondState chanBondState;            //current channel bonded state
    ePhyChainSelect activeChains;               //chains configured through halPhySetChainSelect
    ePhyChainSelect cfgChains;                  //chains configured in NV
    eRegDomainId curRegDomain;                  //index to regulatory domain table

    tANI_BOOLEAN phyPeriodicCalEnable;          //ON when periodic cals are allowed
    
    tPhyTest test;
    
#ifdef ANI_PHY_DEBUG
    tANI_U32 phyDebugLogLevel;                  //Used to check whether debug logs in Calibrations to be printed or not
#endif
    tANI_U8 openLoopTxGain;
    tANI_U8 reserved;
    
    //global pointers to NV tables for use by the physical layer
    //tRxGainShiftChannel *rxGainShiftTable;      //rxGainShiftTable[NUM_20MHZ_RF_CHANNELS]
    sRegulatoryDomains  *regDomainInfo;         // pointer to regulatory domain table
    tRateGroupPwr       *pwrOptimal;            // pwrOptimum[NUM_RF_SUBBANDS]
}tPhy;


extern const char rateStr[NUM_HAL_PHY_RATES][50];   //for debug printing of rates
#endif /* PHY_H */
