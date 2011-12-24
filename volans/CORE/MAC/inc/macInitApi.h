/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 *
 * Copyright (C) 2006 Airgo Networks, Incorporated
 *
 * macInitApi.c - Header file for mac level init functions
 * Author:    Dinesh Upadhyay
 * Date:      04/23/2007
 * History:-
 * Date       Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */
#ifndef __MAC_INIT_API_H
#define __MAC_INIT_API_H

#include "halHddApis.h"

tSirRetStatus macStart(tHalHandle hHal, void* pHalMacStartParams);
tSirRetStatus macStop(tHalHandle hHal, tHalStopType stopType);
tSirRetStatus macOpen(tHalHandle * pHalHandle, tHddHandle hHdd, tMacOpenParameters * pMacOpenParms);
tSirRetStatus macClose(tHalHandle hHal);
#endif //__MAC_INIT_API_H

