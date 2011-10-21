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

#include <sys_defs.h>
#include <asicApi.h>
#include <rfApi.h>
#include <ani_assert.h>
#ifdef __cplusplus
extern "C" 
{
#endif




//looks for the frequencies in the sorted list that are closest(above and below) to the specified frequency
//  so that we can interpolate something based on the frequency
//returns the lo & hi indexes of the members with the encompassing frequencies
// static inline tANI_BOOLEAN FindEncompassingFreqs(tANI_U16 freq, void *list, tANI_U8 offsetToFreq, tANI_U8 sizeStruct, tANI_U8 nMembers, tANI_U8 *loIndex, tANI_U8 *hiIndex)
// {
//     tANI_U8 *member = list;
//     tANI_U8 i;
//     
//     //assert(list != NULL);
//     //assert((tANI_U32)list % 4 == 0);
//     //assert((tANI_U32)offsetToFreq % 2 == 0);  //freq must be on 16-bit boundary
//     
//     {
//         tANI_U16 minFreq = *(tANI_U16 *)(member + offsetToFreq);
//         tANI_U16 maxFreq = *(tANI_U16 *)(member + offsetToFreq + ((nMembers - 1) * sizeStruct));
//         
//         if (freq < minFreq)
//         {
//             *loIndex = 0;
//             *hiIndex = 0;
//                 return (eANI_BOOLEAN_TRUE);
//         }
// 
//         if (freq > maxFreq)
//         {
//             *loIndex = nMembers - 1;
//             *hiIndex = nMembers - 1;
//                 return (eANI_BOOLEAN_TRUE);
//         }
//     }
// 
//     //find the two cal channels that encompass the requested frequency
//     for (i = 0; i < (nMembers - 1); i++)
//     {
//         tANI_U16 lowFreq = *(tANI_U16 *)(member + offsetToFreq + (i * sizeStruct));
//         tANI_U16 highFreq = *(tANI_U16 *)(member + offsetToFreq + ((i + 1) * sizeStruct));
// 
//         if ((freq >= lowFreq) && (freq <= highFreq))
//         {
//             if (freq == lowFreq)
//             {
//                 *loIndex = i;
//                 *hiIndex = i;
//                 return (eANI_BOOLEAN_TRUE);
//             }
//             else if (freq == highFreq)
//             {
//                 *loIndex = i + 1;
//                 *hiIndex = i + 1;
//                 return (eANI_BOOLEAN_TRUE);
//             }
//             else
//             {
//                 *loIndex = i;
//                 *hiIndex = i + 1;
//                 return (eANI_BOOLEAN_TRUE);
//             }
//         }
//     }
//     
//     return (eANI_BOOLEAN_FALSE);
// }

//phyClpcProduction
// void phyTxPowerInit(void);
// void phySetTxPower(eRfChannels chan);
// tANI_S32 InterpolateBetweenPoints(tANI_S32 x1,  tANI_S32 y1, tANI_S32 x2, tANI_S32 y2, tANI_S32 x);

//phyClpcMfg
//void phyClpcCal(tTpcCaldPowerTable *tpcChainCfg, tRfADCVal *pdadc_offset);

//phyCalService
void phyInitCalCorrRegisters(void);
void phyInitialCalStart(void);
void phyPeriodicCal(eCalSelection calId);

//phyCalMemory
void PreProcessCalControlBitMask(void);
void ProcessCalControlBitMask(void);

//phyCalRxIQ
void phyRxIQCal(tANI_U8 txGainIndex, tANI_U8 rxGainIndex, tANI_BOOLEAN initCal);

//phyCalTxIQ
void phyTxIQCal(tANI_U8 gainIndex, tANI_BOOLEAN loopback, tANI_BOOLEAN initCal);

//phyCalCarrier
void phyInitTxCarrierSuppression(tANI_U8 gainIndex, tANI_BOOLEAN initCal);

//RF cals
void phyRtuneCal();
void phyBwCal();


//phyCalDCO
void phyRxDcoCal(uint8 );
void phyRxIm2Cal(tANI_U8 Im2CalOnly);

void phyLoadCharacterizedPowerLut(tANI_U32 pdAdcOffset, tANI_U8 *tpcPowerLUT);

#ifdef DUAL_BAND_BUILD
void LoadRecentCalValues(eInitCals calId, eRfSubBand bandIndex);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PHYAPI_H */

