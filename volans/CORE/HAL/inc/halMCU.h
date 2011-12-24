/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMCU.c:  Provides all the MAC APIs to the MCU Hardware Block.
 * Author:    Satish Gune
 * Date:      02/09/2006
 *
 * --------------------------------------------------------------------------
 */

#ifndef _HALMCU_H_
#define _HALMCU_H_

#include "halTypes.h"
#include "aniGlobal.h"

eHalStatus halMcu_Start(tHalHandle hHal, void *arg);
eHalStatus halMcu_ResetModules(tpAniSirGlobal pMac, tANI_U32 bits);
eHalStatus halMcu_CBRErrorInterruptHandler(tHalHandle hHalHandle, eHalIntSources intSource);
void halMcu_ResetMutexCount(tpAniSirGlobal pMac, tANI_U8 mutexIdx, tANI_U8 count);

#endif /* _HALMCU_H_ */
