/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef __TL_DEBUG_H__
#define __TL_DEBUG_H__
#if  !defined (ANI_OS_TYPE_ANDROID)
#include <stdio.h>
#endif
#include <stdarg.h>
#include "vos_trace.h"
#ifdef WLAN_DEBUG
#ifdef WLAN_MDM_DATAPATH_OPT
#define TLLOGP(x0)  x0
#define TLLOGE(x0)  x0
#define TLLOGW(x0)  x0
#define TLLOG1(x)  {}
#define TLLOG2(x)  {}
#define TLLOG3(x)  {}
#define TLLOG4(x)  {}

#else /*WLAN_MDM_DATAPATH_OPT*/

#define TLLOGP(x0)  x0
#define TLLOGE(x0)  x0
#define TLLOGW(x0)  x0
#define TLLOG1(x0)  x0

#ifdef TL_DEBUG_LOG2
#define TLLOG2(x0)  x0
#else
 #define TLLOG2(x0)
#endif

#ifdef TL_DEBUG_LOG3
#define TLLOG3(x0)  x0
#else
 #define TLLOG3(x0)
#endif

#ifdef TL_DEBUG_LOG4
#define TLLOG4(x0)  x0
#else
 #define TLLOG4(x0)
#endif


#endif /*WLAN_MDM_DATAPATH_OPT*/

#else /* WLAN DEBUG */

#define TLLOGP(x)  x
#define TLLOGE(x)  x
#define TLLOGW(x)  x
#define TLLOG1(x)  {}
#define TLLOG2(x)  {}
#define TLLOG3(x)  {}
#define TLLOG4(x)  {}
#endif /* WLAN DEBUG */


#endif // __TL_DEBUG_H__

