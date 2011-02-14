#ifndef DVT_DEBUG_H
#define DVT_DEBUG_H

#include <stdarg.h>

#include "utilsApi.h"
#include "sirDebug.h"
#include "sirParams.h"


static inline void dvtLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) 
{	

    // Verify against current log level
    if (loglevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( SIR_DVT_MODULE_ID )])
        return;
    else
    {
        va_list marker;

        va_start( marker, pString );     /* Initialize variable arguments. */

        logDebug(pMac, SIR_DVT_MODULE_ID, loglevel, pString, marker);
        
        va_end( marker );              /* Reset variable arguments.      */
    }
}

#endif  /* DVT_DEBUG_H */
