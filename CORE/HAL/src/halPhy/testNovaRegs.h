/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2006
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

  
   testNovaRegs.h: test definitions
   Author:  Mark Nelson
   Date:    6/13/06

   History - 
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef ALL_REGS_H
#define ALL_REGS_H

#include "sys_defs.h"

#define NUM_ALL_REGS 1471
#define MAX_REG_STR_LEN 60

typedef struct
{
    const char regName[MAX_REG_STR_LEN];
    tANI_U32 regAddr;
}tRegDumpType;

extern const tRegDumpType regDumpList[NUM_ALL_REGS];

void logDumpAllNovaRegs(tpAniSirGlobal pMac, int range);
const char *GetNovaRegName(tANI_U32 regAddr);
void testLogPhyWatchList(tpAniSirGlobal pMac);
void testAddRegToPhyWatchList(tpAniSirGlobal pMac, tANI_U32 asicRegAddr);
void testClearPhyWatchList(tpAniSirGlobal pMac);




#endif
