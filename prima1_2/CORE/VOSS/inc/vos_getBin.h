#if !defined( __VOS_GETBIN_H )
#define __VOS_GETBIN_H

/**=========================================================================
  
  \file  vos_getBin.h
  
  \brief virtual Operating System Services (vOSS) binary APIs
               
   Binary retrieval definitions and APIs.  
   
   These APIs allow components to retrieve binary contents (firmware, 
   configuration data, etc.) from a storage medium on the platform.
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <vos_types.h>
#include <vos_status.h>

/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/
/// Binary IDs  
typedef enum
{
  /// Binary ID for firmware
  VOS_BINARY_ID_FIRMWARE,
  
  /// Binary ID for Configuration data
  VOS_BINARY_ID_CONFIG,

  /// Binary ID for country code to regulatory domain mapping
  VOS_BINARY_ID_COUNTRY_INFO,

  /// Binary ID for Handoff Configuration data
  VOS_BINARY_ID_HO_CONFIG

  
} VOS_BINARY_ID;



/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/
  

/**---------------------------------------------------------------------------
  
  \brief vos_get_binary_blob() - get binary data from platform

  This API allows components to get binary data from the platform independent
  of where the data is stored on the device.
  
  <ul>
    <li> Firmware
    <li> Configuration Data
  </ul> 
  
  \param binaryId - identifies the binary data to return to the caller.
         
  \param pBuffer - a pointer to the buffer where the binary data will be 
         retrieved.  Memory for this buffer is allocated by the caller 
         and free'd by the caller. vOSS will fill this buffer with 
         raw binary data and update the *pBufferSize with the exact
         size of the data that has been retreived.
         
         Input value of NULL is valid and will cause the API to return 
         the size of the binary data in *pBufferSize.
         
  \param pBufferSize - pointer to a variable that upon input contains the 
         size of the data buffer available at pBuffer.  Upon success, this
         variable is updated with the size of the binary data that was 
         retreived and written to the buffer at pBuffer.
         
         Input value of 0 is valid and will cause the API to return
         the size of the binary data in *pBufferSize.
         
  \return VOS_STATUS_SUCCESS - the binary data has been successfully 
          retreived and written to the buffer.
          
          VOS_STATUS_E_INVAL - The value specified by binaryId does not 
          refer to a valid VOS Binary ID.
          
          VOS_STATUS_E_FAULT - pBufferSize is not a valid pointer to a 
          variable that the API can write to.
          
          VOS_STATUS_E_NOMEM - the memory referred to by pBuffer and 
          *pBufferSize is not big enough to contain the binary.

  \sa
  
  --------------------------------------------------------------------------*/
VOS_STATUS vos_get_binary_blob( VOS_BINARY_ID binaryId, 
                                v_VOID_t *pBuffer, v_SIZE_t *pBufferSize );

#ifdef WLAN_SOFTAP_FEATURE
/**----------------------------------------------------------------------------
   \brief vos_get_conparam()- function to read the insmod parameters
-----------------------------------------------------------------------------*/
tVOS_CON_MODE vos_get_conparam( void );
#endif
tVOS_CONCURRENCY_MODE vos_get_concurrency_mode( void );
v_BOOL_t vos_concurrent_sessions_running(void);

#endif // !defined __VOS_GETBIN_H
