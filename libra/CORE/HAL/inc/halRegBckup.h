/*
 * File:        halRegBckup.h
 * Description: This file contains the routines to backup the a list of
 *              registers before going to any of the power save modes.
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
 * 08/28/2008 lawrie      Created
 *
 *
 */

#ifndef _HALREGBCKUP_H_
#define _HALREGBCKUP_H_


// Wait cycles after writing into the BMU registers, cycles required based on
// 80Mhz clock frequency
#ifdef LIBRA_RF
#define HAL_REG_REINIT_BMU_WAIT_CYCLES               1600   // 20 usec
#define HAL_REG_REINIT_BMU_CONTROL1_WAIT_CYCLES      100    // 1.25 usec
#define HAL_REG_REINIT_BTQM_STA_EN_DIS_WAIT_CYCLES   100    // 1.25 usec
#else 
#define HAL_REG_REINIT_BMU_WAIT_CYCLES               1600
#define HAL_REG_REINIT_BMU_CONTROL1_WAIT_CYCLES      1600
#define HAL_REG_REINIT_BTQM_STA_EN_DIS_WAIT_CYCLES   1600
#endif

typedef struct sRegisterEntry
{
    tANI_U32    address;    // address of the register
    tANI_U32    value;      // value to be held by the register
} tRegisterEntry;

typedef struct sSpecialRegEntry
{
    tANI_U32    address;    // address of the register
    tANI_U32    inSeqNum;   // number of registers in sequence
    tANI_U32    value;      // value to be held by the register(s)
} tSpecialRegEntry;

/* Functions */
eHalStatus halRegBckup_Open(tHalHandle hHal, void* arg);
eHalStatus halRegBckup_Start(tHalHandle hHal, void* arg);
eHalStatus halRegBckup_WriteWaitCmd(tpAniSirGlobal pMac, tANI_U32 *memAddr,
        tANI_U32 cycles);
eHalStatus halRegBckup_WriteTableEndCmd(tpAniSirGlobal pMac, tANI_U32 memAddr);
void halRegBckup_StartRecord(tpAniSirGlobal pMac, tANI_U32 mode);
void halRegBckup_StopRecord(tpAniSirGlobal pMac);
eHalStatus halRegBckup_PARegisters(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halRegBckup_RFRegisters(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halRegBckup_BBRegisters(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halRegBckup_PreBBRegisters(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_VolatileRegisters(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_InsertOverrideRegList(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_InsertRegEntry(tpAniSirGlobal pMac, tANI_U32 index, 
        tANI_U32 regAddr, tANI_U32 regValue, tANI_U32 hostFilled);
#endif //_HALREGBCKUP_H_
