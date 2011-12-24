/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 * 
 */

#ifndef __PMM_DEBUG_H__
#define __PMM_DEBUG_H__

#include "utilsApi.h"
#include "sirDebug.h"

#define UL_HI( field ) ( *( ( (ULONG *)(&(field)) ) + 1 ) )
#define UL_LO( field ) ( *( ( (ULONG *)(&(field)) ) + 0 ) )


void pmmLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) ;

#endif

