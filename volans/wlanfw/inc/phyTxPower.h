/**
 *

   Copyright (c) 2011 Qualcomm Atheros, Inc. 
   All Rights Reserved. 
   Qualcomm Atheros Confidential and Proprietary. 
  
   Copyright (C) 2006 Airgo Networks, Incorporated

   phyTxPower.h:
   Author:  Mark Nelson
   Date:    4/4/05

   History -
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef PHYTXPOWER_H
#define PHYTXPOWER_H

#include <halPhyCfg.h>

#define INVALID_POWER_REQUEST 0


#ifndef TPC_MEM_POWER_LUT_DEPTH
#define TPC_MEM_POWER_LUT_DEPTH 128
#endif

typedef tTpcLutValue tTpcPowerTable[PHY_MAX_TX_CHAINS][TPC_MEM_POWER_LUT_DEPTH];  //holds the full interpolated power LUT values


typedef struct
{
    tTpcConfig *pwrSampled;             //points to CLPC data in calMemory
}tPhyTxPowerBand;

typedef struct
{
    tANI_U32 curTpcPwrLUT[PHY_MAX_TX_CHAINS][TPC_MEM_POWER_LUT_DEPTH];
    tPhyTxPowerBand combinedBands;
}tPhyTxPower;





#endif /* PHYTXPOWER_H */
