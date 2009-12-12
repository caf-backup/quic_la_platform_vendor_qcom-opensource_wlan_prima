/**
 *
   Airgo Networks, Inc proprietary.
   All Rights Reserved, Copyright 2006
   This program is the confidential and proprietary product of Airgo Networks Inc.
   Any Unauthorized use, reproduction or transfer of this program is strictly prohibited.

  
   testLibraRegs.h: test definitions
   Author:  Mark Nelson
   Date:    6/13/06

   History - 
   Date        Modified by              Modification Information
  --------------------------------------------------------------------------

 */

#ifndef TEST_LIBRA_REGS_H
#define TEST_LIBRA_REGS_H


#include "sys_defs.h"

#define NUM_ALL_LIBRA_REGS 1945
#define MAX_LIBRA_REG_STR_LEN 70

typedef struct
{
    const char regName[MAX_LIBRA_REG_STR_LEN];
    tANI_U32 regAddr;
}tRegDumpType;

extern const tRegDumpType regDumpList[NUM_ALL_LIBRA_REGS];

void logDumpAllLibraRegs(tpAniSirGlobal pMac, int range);
const char *GetLibraRegName(tANI_U32 regAddr);
void testLogPhyWatchList(tpAniSirGlobal pMac);
void testAddRegToPhyWatchList(tpAniSirGlobal pMac, tANI_U32 asicRegAddr);
void testClearPhyWatchList(tpAniSirGlobal pMac);



#endif
