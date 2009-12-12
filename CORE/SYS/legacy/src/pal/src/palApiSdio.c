
/**

    \file palApiPci.c

    \brief Implemenation of the Platform Abstracion Layer functions

    $Id$


    Copyright (C) 2006 Airgo Networks, Incorporated


    This file contains function implementations for the Platform Abstration Layer.

 */

#include "wlan_qct_bal.h"
#include "palApi.h"


eHalStatus palReadRegister( tHddHandle hHdd, tANI_U32 regAddress, tANI_U32 *pRegValue )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, hHdd);


    status = WLANBAL_ReadRegister((v_PVOID_t) pVosContext, (v_U32_t)regAddress, (v_U32_t *) pRegValue);
    if (!VOS_IS_STATUS_SUCCESS(status)){
        VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_FATAL, "Register %p Read FAILED!!", regAddress );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS(status) );
    }        
    return eHAL_STATUS_SUCCESS;
}

eHalStatus palWriteRegister( tHddHandle hHdd, tANI_U32 regAddress, tANI_U32 regValue )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, hHdd);

    status = WLANBAL_WriteRegister(pVosContext, (v_U32_t) regAddress, (v_U32_t) regValue);
    if (!VOS_IS_STATUS_SUCCESS(status)){
        VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_FATAL, "Register %p Write FAILED!!", regAddress );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS(status) );
    }        
    return eHAL_STATUS_SUCCESS;
}

eHalStatus palAsyncWriteRegister( tHddHandle hHdd, tANI_U32 regAddress, tANI_U32 regValue )
{
   v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, hHdd);

    return palWriteRegister(pVosContext, regAddress, regValue);
}

eHalStatus palWriteDeviceMemory( tHddHandle hHdd, tANI_U32 memOffset, tANI_U8 *pBuffer, tANI_U32 numBytes )
{

    VOS_STATUS status = VOS_STATUS_SUCCESS;

    v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, hHdd);


    status = WLANBAL_WriteMemory(pVosContext, (v_U32_t) memOffset, (v_PVOID_t) pBuffer, (v_U32_t) numBytes);
    if (!VOS_IS_STATUS_SUCCESS(status)){
        VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_FATAL, "DeviceMemory %p Write %d bytes FAILED!!", memOffset, numBytes );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS(status) );
    }        
    return eHAL_STATUS_SUCCESS;
}

eHalStatus palReadDeviceMemory( tHddHandle hHdd, tANI_U32 memOffset, tANI_U8 *pBuffer, tANI_U32 numBytes )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, hHdd);

    status = WLANBAL_ReadMemory(pVosContext, (v_U32_t) memOffset, (v_PVOID_t) pBuffer, (v_U32_t) numBytes);
    if (!VOS_IS_STATUS_SUCCESS(status)){
        VOS_TRACE( VOS_MODULE_ID_BAL, VOS_TRACE_LEVEL_FATAL, "DeviceMemory %p Read %d bytes FAILED!!", memOffset, numBytes );
        VOS_ASSERT( VOS_IS_STATUS_SUCCESS(status) );
    }        
    return eHAL_STATUS_SUCCESS;
}

eHalStatus palFillDeviceMemory( tHddHandle hHdd, tANI_U32 memOffset, tANI_U32 numBytes, tANI_BYTE fillValue )
{
    eHalStatus halStatus = eHAL_STATUS_SUCCESS;
    tANI_U32 iteration;
    
    tANI_U32 fullSegments;
    tANI_U32 remainingBytes;
    
    tANI_U32 segmentOffset;

#define cbFillSegment ( 1024 )
    
    static char fill[ cbFillSegment ];
    
        // memory accesses must be a multiple of 4 bytes on Taurus...
        if ( 0 != ( numBytes % 4 ) )
        {
            halStatus = eHAL_STATUS_DEVICE_MEMORY_LENGTH_ERROR;
      return halStatus;
    }

        // memory accesses must be 4 byte aligned on Libra...
        if ( 0 != ( memOffset & 0x3 ) )
        {
            halStatus = eHAL_STATUS_DEVICE_MEMORY_MISALIGNED;
      return halStatus;
    }
    
        // fill the local segment with the fill value.  this segment gets copied to device
        // memory 
    memset( fill, fillValue, cbFillSegment );
        
        // calculate how many full segments we have to fill along with the remining bytes
        // that follow the full segments.
        fullSegments = ( numBytes / cbFillSegment );
        
        remainingBytes = numBytes % cbFillSegment;
        
        // iterate through a 'full segment' at a time and write a block (segemnt) to the 
        // device each time. 
        for ( iteration = 0, segmentOffset = memOffset; 
              iteration < fullSegments && ( eHAL_STATUS_SUCCESS == halStatus ); 
              iteration++, segmentOffset += cbFillSegment )
        {
            halStatus = palWriteDeviceMemory( hHdd, segmentOffset, (tANI_U8*)fill, cbFillSegment );
        }
        
        // if there was a failure to write memory, leave with this error code....
    if ( eHAL_STATUS_SUCCESS != halStatus )
    {
      return halStatus;
    }
    
        // now write the remaining bytes if there are any
        if ( remainingBytes )
        {
            halStatus = palWriteDeviceMemory( hHdd, segmentOffset, (tANI_U8*)fill, remainingBytes );
        }
    
    return( halStatus );
}


