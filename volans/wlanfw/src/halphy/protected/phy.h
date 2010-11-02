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

#include <halPhy.h>
#include <asicTypes.h>
#include <phyTxPower.h>
#include <halPhyCalMemory.h>
#include <asicPhyDbg.h>



/* Currently this structure holds the information about the current calibration mode.
In future, if anymore info is needed, that can be added here */
typedef struct
{
    tANI_U8 currentCalibration;
} sCalibrationInfo;

typedef tIQAdc sTxLoCorrectBB[PHY_MAX_TX_CHAINS][NUM_TX_GAIN_STEPS];

// typedef struct
// {
//     tANI_BOOLEAN    useDcoCorrection;
//     tANI_BOOLEAN    useTxLoCorrection;
//     tRxDcoCorrect   txloDcoCorrect[PHY_MAX_RX_CHAINS];
//     tRxDcoCorrect   dcoCorrection[PHY_MAX_RX_CHAINS];
//     tIQAdc          txloCorrection[PHY_MAX_TX_CHAINS];
//     sTxLoCorrectBB  txloBasebandCorrection;
// }sCalTable;

typedef struct
{
    ePhyChanBondState chanBondState;            //current channel bonded state
    ePhyChainSelect activeChains;               //chains configured through halPhySetChainSelect
    ePhyChainSelect cfgChains;                  //chains configured in EEPROM
    eRegDomainId curRegDomain;                  //index to regulatory domain table

    tANI_U8    IDcoCorr[PHY_MAX_RX_CHAINS];
    tANI_U8    QDcoCorr[PHY_MAX_RX_CHAINS];
    tIQAdc     txLoCorr;
    sIQCalValues rxIQCorr[PHY_MAX_RX_CHAINS];
    sIQCalValues txIQCorr[PHY_ALL_TX_CHAINS];
    
// #ifdef DUAL_BAND_BUILD
//     sCalTable           calTable[NUM_RF_BANDS];
// 
//     eRfSubBand          prevBandIndex;
// #endif //DUAL_BAND_BUILD

    CorexTimer_DescType sTempMeasurementTimer;

}tPhy;

#endif /* PHY_H */
