/*============================================================================
Copyright (c) 2007 QUALCOMM Incorporated.
All Rights Reserved.
Qualcomm Confidential and Proprietary

dvtLogDump.c

Implements the dump commands specific to the dvt module. 
============================================================================*/

#include "dvtModuleApi.h"
#include "logDump.h"


//#ifdef ANI_DVT_DEBUG

static char *
dump_dvt_frames_create( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg3; (void) arg4;
    testDvtCreateFrames(pMac, arg1, arg2);
    return p;
}

static char *
dump_dvt_heap_cleanup( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    testForceDvtHeapGarbageCleanup(pMac);
    return p;
}

static char *
dump_dvt_dxe_histogram( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
    (void) arg1; (void) arg2; (void) arg3; (void) arg4;
    dvtDumpDxePktTimeHistogram(pMac);
    return p;
}

//#endif


static tDumpFuncEntry dvtMenuDumpTable[] = {
	//#ifdef ANI_DVT_DEBUG
    //----------------------------
    {0,     "DVT (500-699)",                                          NULL},
    {500,   "DVT: create frames",                                     dump_dvt_frames_create},
    {501,   "DVT: Heap garbage cleanup",                              dump_dvt_heap_cleanup},
    {502,   "DVT: Dxe pkt time histogram dump",                        dump_dvt_dxe_histogram}
	//#endif
};
	
void dvtDumpInit(tHalHandle hHal)
{
	logDumpRegisterTable( (tpAniSirGlobal) hHal, &dvtMenuDumpTable[0], 
						  sizeof(dvtMenuDumpTable)/sizeof(dvtMenuDumpTable[0]));
}


