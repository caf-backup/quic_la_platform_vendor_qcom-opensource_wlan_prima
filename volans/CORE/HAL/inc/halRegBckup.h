/*
 * File:        halRegBckup.h
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
 * 08/28/2008 lawrie      Created
 *
 *
 */

#ifndef _HALREGBCKUP_H_
#define _HALREGBCKUP_H_


// Wait cycles after writing into the BMU registers, cycles required based on
// 80Mhz clock frequency
#if defined (LIBRA_RF) || defined (VOLANS_RF)
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


#ifdef ADU_MEM_OPT_ENABLED
typedef struct
{
  tANI_U32 cmdRegAddr;
  tANI_U32 numRegs;
} tAduBatchCommandType;

#define FRAME_ADU_BATCH_COMMAND(buffer, regAddress, numRegs) \
       *(buffer) = ((ADU_BATCH_REG_BKUP_CMD) | (regAddress)); \
       *(buffer + 1) = (numRegs & ADU_BATCH_REG_BKUP_CMD_NUM_REG_MASK)

eHalStatus halRegBckup_Memory(tHalHandle hHal, tANI_U32 *buffer, tANI_U32 size);
eHalStatus halRegBckup_Optimized(tpAniSirGlobal pMac, tANI_U32 *pAddr, tRegisterEntry *pRegList, tANI_U32 regCount);
#endif /* ADU_MEM_OPT_ENABLED */

/* Functions */
eHalStatus halRegBckup_Open(tHalHandle hHal, void* arg);
eHalStatus halRegBckup_Start(tHalHandle hHal, void* arg);
eHalStatus halRegBckup_WriteWaitCmd(tpAniSirGlobal pMac, tANI_U32 *memAddr,
        tANI_U32 cycles);
eHalStatus halRegBckup_WriteTableEndCmd(tpAniSirGlobal pMac, tANI_U32 memAddr);
void halRegBckup_StartRecord(tpAniSirGlobal pMac, tANI_U32 mode);
void halRegBckup_StopRecord(tpAniSirGlobal pMac);
eHalStatus halRegBckup_RFRegisters(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halRegBckup_PostBmuStartRegisters(tpAniSirGlobal pMac, tANI_U32 *memAddr);
eHalStatus halRegBckup_PreBmuStartRegisters(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_VolatileRegisters(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_InsertOverrideRegList(tpAniSirGlobal pMac, tANI_U32 *pAddr);
eHalStatus halRegBckup_InsertRegEntry(tpAniSirGlobal pMac, tANI_U32 index, 
        tANI_U32 regAddr, tANI_U32 regValue, tANI_U32 hostFilled);

#ifdef VOLANS_PHY_TX_OPT_ENABLED
eHalStatus halRegBckup_PhyRFTxRegisters(tpAniSirGlobal pMac, tANI_U32 *pMemAddr);
#endif /* VOLANS_PHY_TX_OPT_ENABLED */
#endif //_HALREGBCKUP_H_
