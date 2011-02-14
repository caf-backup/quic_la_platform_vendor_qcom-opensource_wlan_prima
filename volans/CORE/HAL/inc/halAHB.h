/**
 *
 *  @file:      halDXE.h
 *
 *  @brief:     Provides all the APIs to configure DXE.
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------------------------------
 * 04/26/2007  Naveen G			File created.
 * 06/18/2008  Sanoop Kottontavida	Added halDXE_Open/Close/Start/Stop
 * 								and required macros.
 */
#ifndef _HALAHB_H_
#define _HALAHB_H_
 
#include "aniGlobal.h"
#include "halTypes.h"


typedef struct
{
    tANI_U32 regAddr;
    tANI_U32 priOffset; //priority offset.
    tANI_U32 priMask;   //priority mask
    tANI_U32 pri;       //priority
}tAhbPri;

eHalStatus halAhb_Start(tHalHandle hHal, void *arg);

#endif /* _HALAHB_H_ */

