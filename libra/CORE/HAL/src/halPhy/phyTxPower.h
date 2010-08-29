/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2005
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.


   phyTxPower.h:
   Author:	Mark Nelson
   Date:	4/4/05

   History -
   Date	       Modified by	            Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PHYTXPOWER_H
#define PHYTXPOWER_H

#include "halPhyCfg.h"

#define INVALID_POWER_REQUEST 0


#ifndef TPC_MEM_POWER_LUT_DEPTH
#define TPC_MEM_POWER_LUT_DEPTH 128
#endif

typedef tTpcLutValue tTpcPowerTable[PHY_MAX_TX_CHAINS][TPC_MEM_POWER_LUT_DEPTH];  //holds the full interpolated power LUT values

typedef struct
{
    tTpcConfig *pwrSampled;             //points to NV cache
    tTpcPowerTable *pwrInterp;          //dynamically allocated
    tANI_U8 numTpcCalPointsPerFreq;
    tANI_U8 numTpcCalFreqs;
    tANI_U8 reserved[2];
    tTpcConfig pwrSampledAllocation[MAX_TPC_CHANNELS];
    tTpcPowerTable pwrInterpAllocation[MAX_TPC_CHANNELS];
}tPhyTxPowerBand;

typedef struct
{
    tTpcPowerTable curTpcPwrLUT;
    tPhyTxPowerBand combinedBands;
    tANI_BOOLEAN pwrCfgPresent;
    tANI_U8 reserved[2];
}tPhyTxPower;





#endif /* PHYTXPOWER_H */
