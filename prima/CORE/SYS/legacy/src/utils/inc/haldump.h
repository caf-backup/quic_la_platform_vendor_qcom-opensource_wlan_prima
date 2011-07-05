/*===========================================================================

         haldump.h


DESCRIPTION
  This file contains the API for hal related dump command functionality


  Copyright (c) 2010-2011 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when        who      what, where, why
--------    ---      ----------------------------------------------------------
05/2011    spuligil

===========================================================================*/



/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/

#ifndef _HALDUMP_H_
#define _HALDUMP_H_

#ifdef FEATURE_WLAN_INTEGRATED_SOC

#include "aniGlobal.h"
#include "logDump.h"

/*WDA Hal Dump Command implmentationin is featurised with WLAN_FEATURE_LOG_DUMP.
so to maintain consistency with WDA/WDI the below flag is added */
#ifdef WLAN_FEATURE_LOG_DUMP

void halDumpInit(tpAniSirGlobal pMac);

#endif /*WLAN_FEATURE_LOG_DUMP*/

#endif /* FEATURE_WLAN_INTEGRATED_SOC*/

#endif
