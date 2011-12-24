
/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Copyright (C) 2006 Airgo Networks, Incorporated
 *
 * sysWinStartup.cpp: System startup file for Windows platform.
 * Author:         Rajesh Bhagwat
 * Date:           11/01/02
 * History:-
 * 11/01/02        Created.
 * --------------------------------------------------------------------
 *
 */

#include "limApi.h"

#include "utilsApi.h"
#include "sysEntryFunc.h"
#include "sysStartup.h"
#include "cfgApi.h"

// Routine used to retrieve the Winwrapper context pointer from the pMac structure
extern tpAniSirTxWrapper sysGetTxWrapperContext(void *);


tpAniSirTxWrapper
sysGetTxWrapperContext(void *pMac)
{
    return &((tpAniSirGlobal)(pMac))->txWrapper;
}
