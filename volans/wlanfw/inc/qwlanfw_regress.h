#ifndef QWLANFW_REGRESS_H
#define QWLANFW_REGRESS_H
/*===========================================================================

FILE: 
   qwlanfw_regress.h

BRIEF DESCRIPTION:
   Definitions for regression firmware.

DESCRIPTION:
   Regression firmware is special firmware to test hardware from the point
   of embedded CPU. Regression firmware can be downloaded to internal memory,
   and executed by embedded microprocessor. When the test is done, it will
   updated specific memory location with proper information. This file defines
   the where and what information can be found in the memory.

                Copyright (c) 2008 Qualcomm Technologies, Inc.
                All Right Reserved.
                Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header$
$DateTime$

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/12/08   holee   Created

===========================================================================*/

/*===========================================================================
   REGRESSION INFORMATION BLOCK

   Regression information block share space with firmware information block.
   That is, starting address of regression information block is defined as
   QWLANFW_MEMMAP_INFO_START, and first 4 words are reserved for signature.
===========================================================================*/

enum {
   /* Initial state before test begins */
   QWLANFW_REGRESS_STATE_INIT,
   /* Tests are running */
   QWLANFW_REGRESS_STATE_RUNNING,
   /* Tests have completed */
   QWLANFW_REGRESS_STATE_COMPLETED,
};

typedef struct {
   /* 4 word signatures : QWLANFW_SIGNATURE_WORD[0:3] */
   uint32 signatures[4];

   /* State flag. This will be non-zero if code is running */
   uint32 state;

   /* Current test number */
   uint32 count;
   /* Number of tests passed */
   uint32 passed;
   /* Number of tests failed */
   uint32 failed;
} Qwlanfw_RegressInfoBlockType;

#endif /* QWLANFW_REGRESS_H */
