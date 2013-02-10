/**
 *
 *  @file:         halDebug.c
 *
 *  @brief:        Log interface
 *
 *  @author:       Sanoop Kottontavida
 *
 *  Copyright (C) 2002 - 2008, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 */
#include "palTypes.h"
#include "halDebug.h"

void halLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) {
    va_list marker;
    
    if(loglevel > pMac->utils.gLogDbgLevel[HAL_DEBUG_LOGIDX])
        return;
   
    va_start( marker, pString );     /* Initialize variable arguments. */
    
    logDebug(pMac, SIR_HAL_MODULE_ID, loglevel, pString, marker);
    
    va_end( marker );              /* Reset variable arguments.      */
}

