/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyApi.h: Includes physical layer functional interface
   Author:  Mark Nelson
   Date:    3/7/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PHYAPI_H
#define PHYAPI_H

#include "sys_defs.h"
#include "asicApi.h"
#include "rfApi.h"
#include "ani_assert.h"
#ifdef __cplusplus
extern "C"
{
#endif


static inline tANI_S32 InterpolateBetweenPoints(tANI_S32 x1,  tANI_S32 y1, tANI_S32 x2, tANI_S32 y2, tANI_S32 x)
{
    tANI_S32 retVal;

    if ((x2 - x1) == 0)
    {
        //divide by 0 problem
        if (y1 < y2)
            return (y1);
        else
            return (y2);
    }
    else if (y1 != y2)
    {
        retVal = (((x - x1) * (y2 - y1)) / (x2 - x1)) + y1 + GET_ROUND(((x - x1) * (y2 - y1)), (y2 - y1));    //returns the closest integer
    }
    else
    {
        retVal = (((x - x1) * (y2 - y1)) / (x2 - x1)) + y1;    //returns the closest integer
    }

    return(retVal);
}


//looks for the frequencies in the sorted list that are closest(above and below) to the specified frequency
//  so that we can interpolate something based on the frequency
//returns the lo & hi indexes of the members with the encompassing frequencies
static inline eHalStatus FindEncompassingFreqs(tANI_U16 freq, void *list, tANI_U8 offsetToFreq, tANI_U8 sizeStruct, tANI_U8 nMembers, tANI_U8 *loIndex, tANI_U8 *hiIndex)
{
    tANI_U8 *member = list;
    tANI_U8 i;

    //assert(list != NULL);
    //assert((tANI_U32)list % 4 == 0);
    //assert((tANI_U32)offsetToFreq % 2 == 0);  //freq must be on 16-bit boundary

    {
        tANI_U16 minFreq = *(tANI_U16 *)(member + offsetToFreq);
        tANI_U16 maxFreq = *(tANI_U16 *)(member + offsetToFreq + ((nMembers - 1) * sizeStruct));

        if (freq < minFreq)
        {
            *loIndex = 0;
            *hiIndex = 0;
            return (eHAL_STATUS_SUCCESS);
        }

        if (freq > maxFreq)
        {
            *loIndex = nMembers - 1;
            *hiIndex = nMembers - 1;
            return (eHAL_STATUS_SUCCESS);
        }
    }

    //find the two cal channels that encompass the requested frequency
    for (i = 0; i < (nMembers - 1); i++)
    {
        tANI_U16 lowFreq = *(tANI_U16 *)(member + offsetToFreq + (i * sizeStruct));
        tANI_U16 highFreq = *(tANI_U16 *)(member + offsetToFreq + ((i + 1) * sizeStruct));

        if ((freq >= lowFreq) && (freq <= highFreq))
        {
            if (freq == lowFreq)
            {
                *loIndex = i;
                *hiIndex = i;
                return (eHAL_STATUS_SUCCESS);
            }
            else if (freq == highFreq)
            {
                *loIndex = i + 1;
                *hiIndex = i + 1;
                return (eHAL_STATUS_SUCCESS);
            }
            else
            {
                *loIndex = i;
                *hiIndex = i + 1;
                return (eHAL_STATUS_SUCCESS);
            }
        }
    }

    return (eHAL_STATUS_FAILURE);
}

#ifdef VERIFY_HALPHY_SIMV_MODEL //To get rid of multiple definition error, in eazy way.
#define phyInitTxCarrierSuppression host_phyInitTxCarrierSuppression
#define phyTxIQCal                  host_phyTxIQCal
#define phyRxIQCal                  host_phyRxIQCal
#define phyTxPowerInit              host_phyTxPowerInit
#endif

//phyCalService
eHalStatus phyInitialCalStart(tpAniSirGlobal pMac);
eHalStatus RunInitialCal(tpAniSirGlobal pMac, eCalSelection calId);
eHalStatus phyCalFromBringupTables(tpAniSirGlobal pMac, tANI_U16 freq);
eHalStatus phyPeriodicCal(tpAniSirGlobal pMac, eCalSelection calId);


//phyCalRxIQ
eHalStatus phyInitRxIQCal(tpAniSirGlobal pMac);
eHalStatus phyPeriodicIQCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain);
eHalStatus phyRxIQCal(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain);
//eHalStatus phyRxIqClipDetect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain *txGain, eQuasarLoopbackGain *lbGains);

//phyCalTxIQ
eHalStatus phyInitTxIQCal(tpAniSirGlobal pMac);
eHalStatus phyPeriodicIQCalTxChain(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps gain);
eHalStatus phyTxIQCal(tpAniSirGlobal pMac, ePhyTxChains txChain, eGainSteps gain);
eHalStatus phyTxIqClipDetect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain, ePhyRxChains rxChain, tANI_U8 *rxGainIndex, tANI_U8 *rfDetGain);


//phyCalCarrier
eHalStatus phyInitTxCarrierSuppression(tpAniSirGlobal pMac);
eHalStatus phyCarrierSuppressTxChain(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain, tANI_BOOLEAN useBasebandCorrection);
eHalStatus phyPeriodicTxCarrierSuppression(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain);
//eHalStatus phyTxLoClipDetect(tpAniSirGlobal pMac, ePhyTxChains txChain, tTxGain txGain, ePhyRxChains rxChain, tANI_U8 *rxGainIndex, eQuasarLoopbackGain *lbGains);
eHalStatus setupTxCarrierSuppression(tpAniSirGlobal pMac, tANI_U32 txChain, tANI_BOOLEAN isBasebandCorrection, tTxGain txGain);
eHalStatus restoreFromTxCarrierSuppression(tpAniSirGlobal pMac);

//phyCalDCO
eHalStatus phyInitRxDcoCal(tpAniSirGlobal pMac);
eHalStatus phyDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain);
eHalStatus phyForceResidualDCOMeasurement(tpAniSirGlobal pMac);
eHalStatus phyPeriodicDcoCalRxChain(tpAniSirGlobal pMac, ePhyRxChains rxChain, eGainSteps gain);


//phyTxPower
//eHalStatus ConfigureTpcFromNv(tpAniSirGlobal pMac);
eHalStatus phyTxPowerInit(tpAniSirGlobal pMac);
//eHalStatus phyTxPowerConfig(tpAniSirGlobal pMac, tTpcConfig *tpcConfig, tANI_U8 numFreqs, tANI_U8 numCalPointsPerFreq, eRfSubBand bandCfg);
//eHalStatus phyTxPowerClose(tpAniSirGlobal pMac);
//eHalStatus phySetTxPower(tpAniSirGlobal pMac, tANI_U32 freq, eRfSubBand bandCfg);
tPowerdBmRange InterpolateAbsPowerPerFreq(tpAniSirGlobal pMac, tANI_U16 freq);
tPowerDetect phyGetTxPowerLutValForAbsPower(tpAniSirGlobal pMac, ePhyTxChains txChain, t2Decimal absPwr);
t2Decimal phyGetAbsTxPowerForLutValue(tpAniSirGlobal pMac, ePhyTxChains txChain, tPowerDetect lutValue);
eHalStatus LoadRecentCalValues(tpAniSirGlobal pMac, eCalSelection calId, eRfSubBand bandIndex);


eHalStatus phyLoadCharacterizedPowerLut(tpAniSirGlobal pMac, eRfChannels chan);


#ifdef __cplusplus
}
#endif

#endif /* PHYAPI_H */

