/*============================================================================
Copyright (c) 2007 QUALCOMM Incorporated.
All Rights Reserved.
Qualcomm Confidential and Proprietary

csrLogDump.c

Implements the dump commands specific to the csr module. 
============================================================================*/

#include "aniGlobal.h"
#include "csrApi.h"
#include "logDump.h"


#if defined(ANI_LOGDUMP)

static char *
dump_csr( tpAniSirGlobal pMac, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p)
{
	static tCsrRoamProfile x;
	static tSirMacSSid ssid;   //To be allocated for array of SSIDs
    palZeroMemory(pMac->hHdd, (void*)&x, sizeof(x)); 
    x.SSIDs.numOfSSIDs=1 ;
    x.SSIDs.SSIDList[0].SSID = ssid ;
    ssid.length=6 ;
    palCopyMemory(pMac->hHdd, ssid.ssId, "AniNet", 6);
    (void)csrRoamConnect(pMac, &x, NULL, NULL);
    return p;
}


static tDumpFuncEntry csrMenuDumpTable[] = {
	{0,     "CSR (850-860)",                                                                                NULL},
    {851,   "CSR: CSR testing ",                                                                            dump_csr}
};
	
void csrDumpInit(tHalHandle hHal)
{
	logDumpRegisterTable( (tpAniSirGlobal)hHal, &csrMenuDumpTable[0], 
						  sizeof(csrMenuDumpTable)/sizeof(csrMenuDumpTable[0]) );
}


#endif //#if defined(ANI_LOGDUMP)

