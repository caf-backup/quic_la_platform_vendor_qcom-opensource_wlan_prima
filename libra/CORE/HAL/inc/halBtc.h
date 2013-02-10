/*
 * File:        halBtc.h
 * Description: This file contains all the interface functions to
 *              interact with the firmware
 *
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 02/26/2008 davidliu    Created the functions for sending BTC related message to
 *                        FW, configuring FW sys config.
 */
#ifndef _HALBTC_H_
#define _HALBTC_H_

void halBtc_SetBtcCfg(tpAniSirGlobal pMac, void *pBuffer);
void halBtc_SendBtEventToFW(tpAniSirGlobal pMac, void *pBuffer);
#endif
