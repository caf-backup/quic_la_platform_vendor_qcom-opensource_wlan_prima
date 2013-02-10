/**
 *
 *  @file:         halFtmRx.h
 *
 *  @brief:        Header file for halFtmRx.c
 *
 *  @author:       Viral Modi
 *
 *  Copyright (C) 2002 - 2010, Qualcomm Technologies, Inc. All rights reserved.
 *
 */

#ifndef HALFTMRX_H
#define HALFTMRX_H

#include "halTypes.h"
#include "aniGlobal.h"

#define FTM_STA_ID 0

eHalStatus halFtmRx_Start(tHalHandle hHal, void *arg);
eHalStatus halFtm_AddStaSelf(tpAniSirGlobal     pMac);


#endif /* HALFTMRX_H */
