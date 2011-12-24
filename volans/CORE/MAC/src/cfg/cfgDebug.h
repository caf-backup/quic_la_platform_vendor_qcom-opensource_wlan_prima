/*
 *
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Copyright (C) 2006 Airgo Networks, Incorporated
 *
 * Author:      Kevin Nguyen    
 * Date:        04/09/02
 * History:-
 * 04/09/02        Created.
 * --------------------------------------------------------------------
 */

#ifndef __CFG_DEBUG_H__
#define __CFG_DEBUG_H__

#include "sirDebug.h"
#include "utilsApi.h"
#include "limTrace.h"


void cfgLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) ;

#endif
