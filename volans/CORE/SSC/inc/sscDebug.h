#ifndef __SSC_DEBUG_H__
#define __SSC_DEBUG_H__
#if (!defined ANI_OS_TYPE_OSX && !defined (ANI_OS_TYPE_ANDROID))
#include <stdio.h>
#endif
#include <stdarg.h>
#include "vos_trace.h"
#ifdef WLAN_DEBUG
#ifdef WLAN_MDM_DATAPATH_OPT
#define SSCLOGP(x0)  x0
#define SSCLOGE(x0)  x0
#define SSCLOGW(x0)  x0
#define SSCLOG1(x)  {}
#define SSCLOG2(x)  {}
#define SSCLOG3(x)  {}
#define SSCLOG4(x)  {}

#else /*WLAN_MDM_DATAPATH_OPT*/

#define SSCLOGP(x0)  x0
#define SSCLOGE(x0)  x0
#define SSCLOGW(x0)  x0
#define SSCLOG1(x0)  x0

#ifdef SSC_DEBUG_LOG2
#define SSCLOG2(x0)  x0
#else
 #define SSCLOG2(x0)
#endif

#ifdef SSC_DEBUG_LOG3
#define SSCLOG3(x0)  x0
#else
 #define SSCLOG3(x0)
#endif

#ifdef SSC_DEBUG_LOG4
#define SSCLOG4(x0)  x0
#else
 #define SSCLOG4(x0)
#endif


#endif /*WLAN_MDM_DATAPATH_OPT*/

#else /*WLAN DEBUG */

#define SSCLOGP(x)  x
#define SSCLOGE(x)  {}
#define SSCLOGW(x)  {}
#define SSCLOG1(x)  {}
#define SSCLOG2(x)  {}
#define SSCLOG3(x)  {}
#define SSCLOG4(x)  {}
#endif /* WLAN DEBUG */


#endif // __SSC_DEBUG_H__

