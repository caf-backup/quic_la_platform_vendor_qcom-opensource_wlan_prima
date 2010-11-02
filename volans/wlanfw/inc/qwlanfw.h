#ifndef __Q_WLAN_FW_H
#define __Q_WLAN_FW_H

/*===========================================================================

FILE: 
  qwlanfw.h

BRIEF DESCRIPTION:
   Header file containing the definitions used both by Host and Fw

DESCRIPTION:
  qwlanfw.h contains: 
  ** definitions of Ctrl Messages understood by Gen6 FW. 
  ** Dot11 definitions understood by Gen6 FW.
  ** May include HW related definitions.
  This file should be used for Host side development.

                Copyright (c) 2008 QUALCOMM Incorporated.
                All Right Reserved.
                Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

$Header$
$DateTime$

when       who            what, where, why
--------   ---          -----------------------------------------------------
02/08/08   chaitanya      Created

===========================================================================*/

/*===========================================================================
   ARCHITECTURE HEADER FILES
===========================================================================*/

#include <qwlanhw.h>

/*===========================================================================
   WLAN FIRMWARE ERROR REPORT
===========================================================================*/

#ifndef __ASSEMBLER__

/**
   When firmware fails, it reports error code to host. There are two types
   of error codes. 

   - System errors : These are caused by hardware exception.
   - Software errors : Software code determines that it cannot proceed
     due to software problems. 
 */
enum {
   /** Not an error. */
   QWLANFW_ERROR_OK = 0,

   /*
      System exceptions
    */
   QWLANFW_ERROR_SYSTEM = 0x100,

   /** This type of error happens when software code accesses an address
      which is not valid. This could be instruction address or data address
    */
   QWLANFW_ERROR_SYSTEM_ADDR_TRANSLATE,
   /** When operand of load or store instruction violates alignment requirement
      hardware may generate an alignment exception.
    */
   QWLANFW_ERROR_SYSTEM_ALIGNMENT,
   /** This type of error happens when an access to bus fails due to timeout
      or NACK response. For example, accessing an hardware register which does
      not physically exists may cause a bus error. 
    */
   QWLANFW_ERROR_SYSTEM_BUS,
   /** This type of error happens when an access to virtual address failed by 
      permission denied. For example, the page is marked read only, but the
      program tried to write to it.
    */
   QWLANFW_ERROR_SYSTEM_PERMISSION,
   /** This error happens when CPU tries to execute invalid instruction. 
      This may happen when CPU program counter is located not in code space.
    */
   QWLANFW_ERROR_SYSTEM_INVALID_INSTRUCTION,
   /** Other type of exceptions is reported as GENERAL error. This includes
      "DIVIDE BY ZERO", or system calls.
    */
   QWLANFW_ERROR_SYSTEM_GENERAL,

   /*
      Initialization errors.
    */
   QWLANFW_ERROR_INIT = 0x200,

   /** Firmware may abort initialization if configuration is out of range, and
      it can not handle the configuration.
    */
   QWLANFW_ERROR_INIT_CONFIG,
   /** Firmware may abort initialization if it can not allocate memory for its
      operations.
    */
   QWLANFW_ERROR_INIT_OUT_OF_MEMORY,

   /*
      Run-time software errors.
    */
   QWLANFW_ERROR_SW = 0x300,
};

#endif
/*=========================================================================
   MEMORY MAP
===========================================================================*/

/* WLAN firmware informatin block, including signature */
#define QWLANFW_MEMMAP_INFO_START            0x40
#define QWLANFW_MEMMAP_INFO_END              0x180

#define QWLANFW_MEMMAP_HW_BPS_CYCLES_ADDR    0x2404
#define QWLANFW_MEMMAP_ADU_REINIT_DUMMY_ADDR 0x2408
#define QWLANFW_MEMMAP_TIMESTAMP_BASE_ADDR   0x2400
#define QWLANFW_MEMMAP_TIMESTAMP1_ADDR       0x240c
#define QWLANFW_MEMMAP_TIMESTAMP2_ADDR       0x2410
#define QWLANFW_MEMMAP_TIMESTAMP3_ADDR       0x2414

/*===========================================================================
   INFORMATION BLOCK
===========================================================================*/

#define QWLANFW_SIGNATURE_WORD0              0x5155414c /* QUAL */
#define QWLANFW_SIGNATURE_WORD1              0x434f4d4d /* COMM */
#define QWLANFW_SIGNATURE_WORD2              0x20574c41 /*  WLA */
#define QWLANFW_SIGNATURE_WORD3              0x4e204657 /* N FW */

#endif /*__Q_WLAN_FW_H*/ 
