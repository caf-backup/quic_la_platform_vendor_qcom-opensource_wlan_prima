#ifndef _BTCES_TYPES_H_
#define _BTCES_TYPES_H_

/*------------------------------------------------------------------------------
                 BTC-ES Bluetooth Coexistence Event Source
------------------------------------------------------------------------------*/

/**
   @file btces_types.h
   
   This file provides a mapping between platform-specific types and the BTC-ES
   platform-independent types. It also supplies a few definitions common to BTC-ES
   clients and to the hosting platform calling into BTC-ES.
*/

/*------------------------------------------------------------------------------
        Copyright (c) 2009 QUALCOMM Incorporated.
               All Rights Reserved.
        Qualcomm Proprietary and Confidential.
------------------------------------------------------------------------------*/
/*=============================================================================

                       EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //depot/asic/msmshared/sandbox/projects/bt/btces/btces/main/latest/inc/btces_types.h#2 $
  $DateTime: 2009/05/15 12:18:14 $
  $Author: tmonahan $

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2009-05-15  tam  Removed BTCES_STATUS_STILL_INITIALIZED; De-Init does not need De-Register.
  2009-04-24  tam  Added BTCES_STATUS_STILL_INITIALIZED.
  2009-04-01  tam  Added BTCES_STATUS_ALREADY_INITIALIZED.
  2009-03-31  tam  Moved common things here from btces.h .
  2009-03-25  dgh  Initial version.

=============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

typedef signed char     int8;
typedef signed short    int16;
typedef signed long     int32;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
typedef int             boolean;


/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Enumerated types
 * -------------------------------------------------------------------------*/
/** BTC-ES API and BTC-ES PFAL API failure codes */
typedef enum
{
  BTCES_OK                            = 0,    /**< The service was successful, alias for BTCES_SUCCESS */
  BTCES_SUCCESS                       = 0,    /**< The service was successful, alias for BTCES_OK */
  BTCES_FAIL                          = 101,  /**< The service was unsuccessful */
  BTCES_STATUS_OUT_OF_MEMORY          = 102,  /**< There was not enough memory to complete the operation. */
  BTCES_STATUS_NOT_IMPLEMENTED        = 103,  /**< The requested operation has not been implemented. */
  BTCES_STATUS_NOT_INITIALIZED        = 104,  /**< The subsystem is not ready to perform operations. */
  BTCES_STATUS_INITIALIZATION_FAILED  = 105,  /**< The request failed due to an initialization problem. */
  BTCES_STATUS_INVALID_PARAMETERS     = 106,  /**< The service was given one or more invalid parameters. */
  BTCES_STATUS_INTERNAL_ERROR         = 107,  /**< The service request found an internal inconsistency. */
  BTCES_STATUS_INVALID_STATE          = 108,  /**< The state of the system does not allow the requested operation. */
  BTCES_STATUS_ALREADY_REGISTERED     = 109,  /**< Registration has already been performed. */
  BTCES_STATUS_NOT_REGISTERED         = 110,  /**< Registration has not yet been performed. */
  BTCES_STATUS_ALREADY_INITIALIZED    = 111,  /**< The subsystem was already initialized, so no action occurred. */
} BTCES_STATUS;


/*----------------------------------------------------------------------------
 * Structure definitions
 * -------------------------------------------------------------------------*/

/**
  Bluetooth Device Address

  The device address is defined as an array of bytes.
  The array is big-endian, meaning that
  - addr[0] contains bits 47-40,
  - addr[1] contains bits 39-32,
  - addr[2] contains bits 31-24,
  - addr[3] contains bits 23-16,
  - addr[4] contains bits 15-8, and
  - addr[5] contains bits 7-0.
*/
typedef struct
{
  uint8 addr[6];        /**< The address is in this byte array. */
} btces_bt_addr_struct;


/*----------------------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Constant values
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function declarations
 * -------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /*_BTCES_TYPES_H_*/
