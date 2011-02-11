#ifndef ANI_ASSERT_H
#define ANI_ASSERT_H


//#define NASSERT
#if defined(_X86_)
#if defined ANI_OS_TYPE_WINDOWS
#if !defined(NASSERT) && defined(ANI_PHY_DEBUG)
#define assert(x)   if (!(x)) { _asm int 3 }
#else
#define assert(x)
#endif
#else
#define assert(x)
#endif

#elif defined ANI_OS_TYPE_LINUX

#ifndef NASSERT
//I seem to be the only one who cares about using assertions, so I'm going to convert this to a phyLog statement - M.E.N.
#include "sys_defs.h"
#include "phyDebug.h"
//#define assert(x) { if (!(x) && (pMac != NULL)) phyLog(pMac, LOGE, "ERROR: Assertion: %s  FILE:%s  LINE:%i\n", #x, __FILE__, __LINE__); }
#define assert(x) { if (!(x)) phyLog(pMac, LOGE, "ERROR: Assertion: %s  FILE:%s  LINE:%i\n", #x, __FILE__, __LINE__); }
#else
#define assert(x)
#endif
#elif defined ANI_OS_TYPE_OSX
#include <kern/assert.h>

#elif (defined(ANI_OS_TYPE_AMSS) || defined(ANI_OS_TYPE_ANDROID))

#include "vos_trace.h"

#define assert(x)  VOS_ASSERT(x)

#else //#if defined(_X86_)

#define assert(x)

#endif //#if defined(_X86_)

#endif

