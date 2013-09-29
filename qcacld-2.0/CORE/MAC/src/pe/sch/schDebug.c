/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file schDebug.cc contains some debug functions.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */


#include "schDebug.h"

void schLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...)
{
#ifdef WLAN_DEBUG
    // Verify against current log level
    if ( loglevel > pMac->utils.gLogDbgLevel[LOG_INDEX_FOR_MODULE( SIR_SCH_MODULE_ID )] )
        return;
    else
    {
        va_list marker;

        va_start( marker, pString );     /* Initialize variable arguments. */

        logDebug(pMac, SIR_SCH_MODULE_ID, loglevel, pString, marker);

        va_end( marker );              /* Reset variable arguments.      */
    }
#endif
}


// --------------------------------------------------------------------
