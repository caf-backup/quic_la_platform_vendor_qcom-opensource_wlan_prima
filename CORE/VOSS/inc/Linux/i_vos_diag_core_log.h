#if !defined( __I_VOS_DIAG_CORE_LOG_H )
#define __I_VOS_DIAG_CORE_LOG_H

/**=========================================================================
  
  \file  i_vos_diag_core_event.h
  
  \brief android-specific definitions for vOSS DIAG logs
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include <vos_types.h>
#include <vos_memory.h>

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include <log.h>
#include <log_codes.h>
#endif

/*-------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/
//FIXME To be removed when DIAG support is added. This definiton should be
//picked from log.h file above. 
typedef struct
{
  unsigned char header[12]; /* A log header is 12 bytes long */
}__attribute__((packed)) log_hdr_type;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/*---------------------------------------------------------------------------
  Allocate an event payload holder
---------------------------------------------------------------------------*/
#define WLAN_VOS_DIAG_LOG_ALLOC( payload_ptr, payload_type, log_code ) \
           do                                                                  \
           {                                                                   \
              if(log_status(log_code))                                         \
              {                                                                \
                 payload_ptr = ( payload_type *)vos_mem_malloc(sizeof(payload_type)); \
              }                                                                \
              else                                                             \
              {                                                                \
                 payload_ptr = NULL;                                           \
              }                                                                \
              if( payload_ptr )                                                \
              {                                                                \
                 vos_mem_zero(payload_ptr, sizeof(payload_type));              \
                 log_set_code(payload_ptr, log_code);                          \
                 log_set_length(payload_ptr, sizeof(payload_type));            \
                 log_set_timestamp(payload_ptr);                               \
              }                                                                \
           } while (0)

/*---------------------------------------------------------------------------
  Report the event
---------------------------------------------------------------------------*/
#define WLAN_VOS_DIAG_LOG_REPORT( payload_ptr ) \
    do                                          \
    {                               \
       if( payload_ptr)              \
       {                             \
          log_submit( payload_ptr);  \
          vos_mem_free(payload_ptr); \
       }                             \
    } while (0)
    
/*---------------------------------------------------------------------------
  Free the payload
---------------------------------------------------------------------------*/
#define WLAN_VOS_DIAG_LOG_FREE( payload_ptr ) \
    do                                          \
    {                               \
       if( payload_ptr)              \
       {                             \
          vos_mem_free(payload_ptr); \
       }                             \
    } while (0)
    

#else /* FEATURE_WLAN_DIAG_SUPPORT */

#define WLAN_VOS_DIAG_LOG_ALLOC( payload_ptr, payload_type, log_code ) 
#define WLAN_VOS_DIAG_LOG_REPORT( payload_ptr ) 
#define WLAN_VOS_DIAG_LOG_FREE( payload_ptr )

#endif /* FEATURE_WLAN_DIAG_SUPPORT */


/*------------------------------------------------------------------------- 
  Function declarations and documenation
  ------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __I_VOS_DIAG_CORE_LOG_H
