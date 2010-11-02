#ifndef PHY_DEBUG_H
#define PHY_DEBUG_H
#if !defined ANI_OS_TYPE_OSX && !defined ANI_OS_TYPE_ANDROID
#include <stdio.h>
#endif
#include <stdarg.h>

#include "utilsApi.h"
#include "sirDebug.h"
#include "sirParams.h"

void phyLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...);

#endif  /* PHY_DEBUG_H */
