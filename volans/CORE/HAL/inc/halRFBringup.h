/*
 * File:        halRRBringup.h
 * Description: This file contains the routines to backup the a list of
 *              registers before going to any of the power save modes.
 *
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 08/28/2008 bharathp      Created
 *
 *
 */

#ifndef _HALRFBRINGUP_H_
#define _HALRFBRINGUP_H_

#ifdef LIBRA_RF
void halRF_InitRxDcoCal(tpAniSirGlobal pMac);
void halRF_SetChannel(tpAniSirGlobal pMac, tANI_U8 chan);
#endif

#endif //_HALREGBCKUP_H_
