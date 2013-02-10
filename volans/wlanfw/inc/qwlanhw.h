#ifndef QWLANHW_H
#define QWLANHW_H

/*===========================================================================

FILE: 
   qwlanhw.h

BRIEF DESCRIPTION:
   Definitions for Qualcomm WLAN architecture.

DESCRIPTION:
   Qualcomm has several generations of WLAN chipsets. This file includes
   right header file for current architecture.

                Copyright (c) 2008 Qualcomm Technologies, Inc.
                All Right Reserved.
                Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

$Header$
$DateTime$

when       who            what, where, why
--------   ---          -----------------------------------------------------
03/03/08   holee        Created

===========================================================================*/

/*===========================================================================
   ARCHITECTURE HEADER FILES

   LIBRA architecture is exposed to host interface. WLAN firmware can choose
   architecture between Libra and Virgo based on configuration.

   qwlanhw_$(arch).h : Register definition file.
   qwlanhw_$(arch)defs.h : Architecture specific definition file.
   qwlanhw_commondefs.h : Architecture common definition file.
===========================================================================*/

#if defined(FEATURE_WLANFW_ARCH_VIRGO)
#include <qwlanhw_virgo.h>
#include <qwlanhw_virgodefs.h>
#elif defined(FEATURE_WLANFW_ARCH_VIRGO2)
#include <qwlanhw_virgo2.h>
#include <qwlanhw_virgodefs.h>
#elif defined(FEATURE_WLANFW_ARCH_LIBRA)
#include <qwlanhw_libra.h>
#include <qwlanhw_libradefs.h>
#elif defined(FEATURE_WLANFW_ARCH_VOLANS)
#include <qwlanhw_volans.h>
#include <qwlanhw_volansdefs.h>
#endif
#include <qwlanhw_commondefs.h>

#endif /* QWLANHW_H */

