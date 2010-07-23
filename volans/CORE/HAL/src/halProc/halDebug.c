/**
 *
 *  @file:         halDebug.c
 *
 *  @brief:        Log interface
 *
 *  @author:       Sanoop Kottontavida
 *
 *  Copyright (C) 2002 - 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 */
#include "palTypes.h"
#include "halDebug.h"

void halLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) {
    va_list marker;
    
    if(loglevel > pMac->utils.gLogDbgLevel[HAL_DEBUG_LOGIDX])
        return;
   
    va_start( marker, pString );     /* Initialize variable arguments. */
    
    logDebug(pMac, SIR_HAL_MODULE_ID, loglevel, pString, marker);
    
    va_end( marker );              /* Reset variable arguments.      */
}

#ifdef WLAN_DEBUG
/* -------------------------------------------------------------
 * FUNCTION: halLog_FwSystemCfgDump()
 *
 * NOTE:
 *   Dumps the firmware system config memory on the console
 * -------------------------------------------------------------
 */
void halLog_FwSystemCfgDump(tpAniSirGlobal pMac, tANI_U32 numLocations)
{
#ifdef WLAN_DEBUG
    tANI_U32 *fwBuffer;
    tANI_U32 fwMemOffset = pMac->hal.memMap.fwSystemConfig_offset;
    tANI_U32 i;

	fwBuffer = vos_mem_malloc(numLocations);
	if(fwBuffer != NULL)
	{
		vos_mem_zero(fwBuffer,numLocations);
	        
	    /* Read the device memory */
	    if((halReadDeviceMemory(pMac, fwMemOffset, fwBuffer, numLocations)) != eHAL_STATUS_SUCCESS)
	    {
	        HALLOGE( halLog(pMac, LOGE,
	           FL("%s: Could not read device memory for dumping\n"), __FUNCTION__));
			return;
	    }

	    HALLOGE( halLog(pMac, LOGE, FL("=======================================================================\n")));
	    HALLOGE( halLog(pMac, LOGE, FL("\t\t\tFirmware configuration space contents\t\t\t\n")));
	    HALLOGE( halLog(pMac, LOGE, FL("=======================================================================\n")));    

	    /* Dump the firmware configuration space contents as Address = Contents */
	    for (i = 0; i < numLocations; i++)
	    {
	        HALLOGE( halLog(pMac, LOGE, FL("0x%04x = 0x%08x\n"), pMac->hal.memMap.fwSystemConfig_offset + (i * 4), fwBuffer[i]));
	    }
	}
	else
	{
		/* Error condition */
		HALLOGE(halLog(pMac, LOGE, FL("\r\n ERROR: Memory Allocation Failed\r\n")));
	}
	
    return;
	
#endif
}


/* -------------------------------------------------------------
 * FUNCTION: halLog_DumpDeviceMemory()
 *
 * NOTE:
 *   Dumps device memory on the console
 * -------------------------------------------------------------
 */
void halLog_DumpDeviceMemory(tpAniSirGlobal pMac, 
                                          tANI_U32 startAddr, 
                                          tANI_U32 offset,
                                          tANI_U32 size)
{
#ifdef WLAN_DEBUG
    tANI_U32 *devBuffer;
    tANI_U32 i = 0x0;

	devBuffer = vos_mem_malloc(size);

	if(devBuffer != NULL)
	{
	    vos_mem_zero(devBuffer,size);
	    
	    /* Read the device memory */
	    if((halReadDeviceMemory(pMac, (startAddr + offset), devBuffer, size)) != eHAL_STATUS_SUCCESS)
	    {
	        HALLOGE( halLog(pMac, LOGE,
	           FL("%s: Could not read device memory for dumping\n"), __FUNCTION__));
			return;
	    }

	    HALLOGE( halLog(pMac, LOGE, FL("=======================================================================")));
	    HALLOGE( halLog(pMac, LOGE, FL("\tDevice Memory Contents\t")));
	    HALLOGE( halLog(pMac, LOGE, FL("=======================================================================")));    

	    /* Dump the firmware configuration space contents as Address = Contents */
	    for (i = 0; i < size; i++)
	    {
	        HALLOGE( halLog(pMac, LOGE, FL("0x%08x = 0x%08x\n"), (startAddr + (i * 4)), devBuffer[i]));
	    }
	}
	else
	{
	    HALLOGE( halLog(pMac, LOGE, FL("0x%08x = 0x%08x\n"), (startAddr + (i * 4)), devBuffer[i]));		
	}
	
    return;
	
#endif
}

#endif /* WLAN_DEBUG */
