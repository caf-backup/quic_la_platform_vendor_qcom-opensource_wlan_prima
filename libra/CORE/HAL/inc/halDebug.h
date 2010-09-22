#ifndef __HAL_DEBUG_H__
#define __HAL_DEBUG_H__
#if (!defined ANI_OS_TYPE_OSX && !defined (ANI_OS_TYPE_ANDROID))
#include <stdio.h>
#endif
#include <stdarg.h>

#include "utilsApi.h"
#include "sirDebug.h"
#include "sirParams.h"
#define HAL_DEBUG_LOGIDX  ( LOG_INDEX_FOR_MODULE(SIR_HAL_MODULE_ID) )



#ifdef WLAN_DEBUG
#ifdef WLAN_SAP_MEM_OPT
#define HALLOGP(x0)  x0
#define HALLOGE(x0)  x0
#define HALLOGW(x0)  x0
#define HALLOG1(x)  {}
#define HALLOG2(x)  {}
#define HALLOG3(x)  {}
#define HALLOG4(x)  {}
#define STR(x)  x

#else /*WLAN_SAP_MEM_OPT*/

#define HALLOGP(x0)  x0
#define HALLOGE(x0)  x0
#define HALLOGW(x0)  x0
#define HALLOG1(x0)  x0

#ifdef HAL_DEBUG_LOG2
#define HALLOG2(x0)  x0
#else
 #define HALLOG2(x0)
#endif

#ifdef HAL_DEBUG_LOG3
#define HALLOG3(x0)  x0
#else
 #define HALLOG3(x0)
#endif

#ifdef HAL_DEBUG_LOG4
#define HALLOG4(x0)  x0
#else
 #define HALLOG4(x0)
#endif

#define STR(x)  x

#endif /*WLAN_SAP_MEM_OPT*/

#else

#define HALLOGP(x)  x
#define HALLOGE(x)  {}
#define HALLOGW(x)  {}
#define HALLOG1(x)  {}
#define HALLOG2(x)  {}
#define HALLOG3(x)  {}
#define HALLOG4(x)  {}
#define STR(x)      ""
#endif

void halLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...);

#endif // __HAL_DEBUG_H__

