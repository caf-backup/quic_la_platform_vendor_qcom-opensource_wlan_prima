/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMIF.c:  Provides all the MAC APIs to the BMU Hardware Block.
 * Author:    Susan Tsao
 * Date:      03/13/2006
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALMIF_H_
#define _HALMIF_H_

#include "halTypes.h"
#include "aniGlobal.h"

eHalStatus halMif_Start(tHalHandle hHal, void *arg);
eHalStatus halMif_initSDR_forFpga(tpAniSirGlobal pMac);
eHalStatus halMif_initSDR_forChip(tpAniSirGlobal pMac);
eHalStatus halIntMIFErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource);
#endif /* _HALMIF_H_ */



