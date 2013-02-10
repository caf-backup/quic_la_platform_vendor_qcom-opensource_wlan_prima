/**
 *
 *  @file:         halInterrupts.h
 *
 *  @brief:        Header file for Hal Interrupts.
 *
 *  @author:        Jeff
 *
 *  Copyright (C) 2002 - 2007, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 12/03/2007  Added Virgo specific changes.
 * 05/02/2008  Added Libra specific changes.
 */

#ifndef HALINTERRUPTS_H
#define HALINTERRUPTS_H

#include "halTypes.h"
#include "vos_types.h"

#define CMD53_CLOCK_SWITCHING_HW_BUG_WORKAROUND 1

// enumerate all of the interrupt registers on the chip.

typedef enum {
    // top-level interrupt register
    eHAL_INT_SIF_REGISTER,

    // grouped interrupt registers
    eHAL_INT_MCU_HOST_INT_REGISTER,
    eHAL_INT_MCU_MTU_INT_REGISTER,
    eHAL_INT_MCU_COMBINED_INT_REGISTER,
    eHAL_INT_MCU_BMU_WQ_INT_REGISTER,

    // source interrupt registers
    eHAL_INT_BMU_ERR_REGISTER,
    eHAL_INT_RPE_ERR_REGISTER,
    eHAL_INT_DPU_REGISTER,
    eHAL_INT_DXE_RAW_REGISTER,

    //By Gen6 Programmer's Guide, ED and DONE intr don't need to be handled.
    //eHAL_INT_DXE_ED_REGISTER,
    //eHAL_INT_DXE_DONE_REGISTER,

    eHAL_INT_DXE_ERR_REGISTER,
    eHAL_INT_MIF_REGISTER,
    eHAL_INT_ADU_REGISTER,
    eHAL_INT_RXP_REGISTER,

    // must be last
    eHAL_INT_MAX_REGISTER,
} eHalIntRegisters;


// enumerate all of the interrupt sources on the chip.

typedef enum {

    eHAL_INT_SIF_ASIC,
    eHAL_INT_MCU_HOST_INT_WQ_DATA_AVAIL, 
    eHAL_INT_MCU_HOST_INT_MIF,
    eHAL_INT_MCU_HOST_INT_MTU_TIMER_5,
    eHAL_INT_MCU_HOST_INT_ADU,
    eHAL_INT_MCU_HOST_INT_RPE,
    eHAL_INT_MCU_HOST_INT_COMBINED,
    eHAL_INT_MCU_HOST_INT_RXP,
    eHAL_INT_MCU_HOST_INT_DXE_CH0,    
    eHAL_INT_MCU_HOST_INT_DXE_CH1,
	eHAL_INT_MCU_HOST_INT_DXE_CH2,

    eHAL_INT_MCU_HOST_INT_MBOX0,
    eHAL_INT_MCU_HOST_INT_MBOX1,
    eHAL_INT_MCU_HOST_INT_MBOX2,
    eHAL_INT_MCU_HOST_INT_MBOX3,
    eHAL_INT_MCU_HOST_INT_MTU,
    eHAL_INT_MCU_HOST_INT_DEFAULT, /* all other unspecified interrupts in this register */
         
    // eHAL_INT_MCU_MTU_INT_REGISTER:  
    eHAL_INT_MCU_MTU_INT_ERROR,
    eHAL_INT_MCU_MTU_INT_WD_ENABLE_DISABLE_ERROR,
    eHAL_INT_MCU_MTU_INT_WD_PROTECTION_ERROR,

    // eHAL_INT_MCU_COMBINED_INT_REGISTER,
    // All other uninterested ones handled in eHAL_INT_MCU_COMBINED_INT_DEFAULT
    eHAL_INT_MCU_COMBINED_INT_TPE_MCU_BD_BASED_TX_INT_1_P_HOSTINT_EN,     
    eHAL_INT_MCU_COMBINED_INT_BMU_ERROR, 
    eHAL_INT_MCU_COMBINED_INT_DPU_ERROR, 
    eHAL_INT_MCU_COMBINED_INT_DEFAULT, /* all other unspecified combined interrupts */

    // eHAL_INT_MCU_BMU_WQ_INT_REGISTER,
    eHAL_INT_MCU_BMU_WQ_INT_WQ_DEFAULT, /* all other unspecified WQ interrupts */

    // eHAL_INT_BMU_ERR_REGISTER
    eHAL_INT_BMU_ERROR_DEFAULT,

    // eHAL_INT_RPE_ERR_REGISTER
    eHAL_INT_RPE_ERR_DEFAULT,

    // eHAL_INT_DPU_REGISTER
    eHAL_INT_DPU_ERR_DEFAULT,

    // eHAL_INT_DXE_RAW_REGISTER
     eHAL_INT_DXE_RAW_CHANNEL_0,
     eHAL_INT_DXE_RAW_CHANNEL_1,
     eHAL_INT_DXE_RAW_CHANNEL_2,

    // From Programmer's guide, Gen6 doesn't need to handle ED and DONE interrupts

    // eHAL_INT_DXE_ED_REGISTER
    //  eHAL_INT_DXE_ED_CHANNEL_0,
    //  eHAL_INT_DXE_ED_CHANNEL_1,
    //  eHAL_INT_DXE_ED_CHANNEL_2,
    // eHAL_INT_DXE_DONE_REGISTER
    //  eHAL_INT_DXE_DONE_CHANNEL_0,
    //  eHAL_INT_DXE_DONE_CHANNEL_1,
    //  eHAL_INT_DXE_DONE_CHANNEL_2,

    // eHAL_INT_DXE_ERR_REGISTER
     eHAL_INT_DXE_ERR_CHANNEL_0,
     eHAL_INT_DXE_ERR_CHANNEL_1,
     eHAL_INT_DXE_ERR_CHANNEL_2,

    // eHAL_INT_MIF_REGISTER
    eHAL_INT_MIF_ERR_DEFAULT,

    // eHAL_INT_ADU_REGISTER,
    eHAL_INT_ADU_ERR_DEFAULT,  // this intr handles all ADU errors

    // eHAL_INT_RXP_REGISTER,
    eHAL_INT_RXP_ERR_DEFAULT,  // this intr handles all Rxp errors

    // must be last
    eHAL_INT_MAX_SOURCE,
} eHalIntSources;


// Interrupt masks for each module to handle unregistered interrupt sources.
//
// In Gen4 we used to have a lot of unused interrupt entries declared in halIntInfo[] 
// and waste at least 16 bytes per interrupt source. Now try to combine them at module
// level and make them all use same interrupt source.

/* these interrupts are currently by default handler */
#ifdef CMD53_CLOCK_SWITCHING_HW_BUG_WORKAROUND
#define QWLAN_MCU_MAC_HOST_INT_EN_DEFAULT_MASK 0x80000000
#else             
#define QWLAN_MCU_MAC_HOST_INT_EN_DEFAULT_MASK 0
#endif

/* For combined interrupt, include everything but BMU & DPU errors */
// DEBUG - BTC enables TXP BTC aborts, causes many of these interrupts to occur
// #define QWLAN_MCU_COMBINED_HOST_INT_EN_DEFAULT_MASK (QWLAN_MCU_COMBINED_HOST_INT_EN_TXP_MCU_ERR_INT_HOSTINT_EN_MASK)
#define QWLAN_MCU_COMBINED_HOST_INT_EN_DEFAULT_MASK 0

/* default include all ASIC related interrupt bits in SIF interrupt status register */
#define QWLAN_SIF_INT_ASIC_INTERRUPT_MASK            0x3ffe0001
            
/* Don't register any WQs to the default handler */
/* except for unknown addr2 WQ */
#define QWLAN_MCU_BMU_WQ_HOST_INT_EN_DEFAULT_MASK (1 << BMUWQ_HOST_RX_UNKNOWN_ADDR2_FRAMES)

/* default BMU errors */
#define QWLAN_BMU_ERR_INTR_EN_DEFAULT_MASK ( \
             QWLAN_BMU_ERR_INTR_ENABLE_BTQM_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_DOUBLE_RELEASE_PDU_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_DOUBLE_RELEASE_BD_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_DOUBLE_PUSH_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_RELEASE_FRAG_BD_INDEX_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_MULTIPLE_READY_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_RELEASE_INTEGRITY_CHECK_PDU_LINK_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PUSH_INTEGRITY_CHECK_PDU_LINK_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PUSH_INTEGRITY_CHECK_HEAD_TAIL_PDU_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PUSH_INTEGRITY_CHECK_PREV_PDU_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PREV_PDU_INDEX_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_NR_OF_PDU_OVERFLOW_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_NR_OF_BD_OVERFLOW_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_WRGAM_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_RDGAM_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_TIMEOUT_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_IDLE_PDU_LIST_TAIL_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_IDLE_PDU_LIST_CNT_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PDU_INDEX_FROM_BD_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_RELEASE_FIFO_FULL_WARNING_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_BD_LINKED_LIST_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_PDU_LINKED_LIST_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_OUT_OF_MODULE_RANGE_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_OUT_OF_PDU_RANGE_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_OUT_OF_BD_RANGE_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_UNDEFINED_CMD_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_WQ_INVALID_ERR_ENABLE_MASK |\
             QWLAN_BMU_ERR_INTR_ENABLE_WQ_FULL_ERR_ENABLE_MASK)

/* default RPE errors */
#define QWLAN_RPE_ERR_INT_EN_DEFAULT_MASK 0x1ff 

/* default DPU errors */
#define QWLAN_DPU_DPU_INTERRUPT_EN_DEFAULT_MASK 0x800040

/* default ADU errors */
#define QWLAN_ADU_INTERRUPT_ENABLE_DEFAULT_MASK 0x1fff

/* default RxP errors */
#define QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_DEFAULT_MASK  (\
    QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_RPE_BITMAP_INTERFACE_TIMEOUT_INT_ENABLE_MASK |\
    QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_RPE_DUPL_INTERFACE_TIMEOUT_INT_ENABLE_MASK |\
    QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_RXP_ADDR2_MISS_INTERRUPT_MASK)

// the cache for a given interrupt register
typedef struct sHalIntRegisterCache {
    tANI_U32 value;
    tANI_U32 valid;
} tHalIntRegisterCache;


// define the type of an interrupt handler
typedef eHalStatus (*pHalIntHandler)(tHalHandle, eHalIntSources);

// external APIs
eHalStatus halIntDumpRegisters(tHalHandle);
eHalStatus halIntDumpInterrupts(tHalHandle);
eHalStatus halIntEnrollHandler(eHalIntSources, pHalIntHandler);
#if defined ANI_BUS_TYPE_SDIO
eHalStatus halIntSdioPopulateCache(tHalHandle hHalHandle, tANI_U32 sifIntStatus);
#endif

eHalStatus halIntReset(tHalHandle);
eHalStatus halIntChipEnable(tHalHandle);
eHalStatus halIntChipDisable(tHalHandle);
eHalStatus halIntEnable(tHalHandle, eHalIntSources);
eHalStatus halIntDisable(tHalHandle, eHalIntSources);
//void halIntDisableAllButMailboxIntInPifPciIntEnableCache(tHalHandle, tANI_U32*);
void halIntRestorePifPciIntEnableCache(tHalHandle, tANI_U32);
eHalStatus halIntCheck(tHalHandle hHalHandle);
VOS_STATUS halIntHandler(v_PVOID_t pVosGCtx);

//returns eHAL_STATUS_INTERRUPT_PRESENT or eHAL_STATUS_NO_INTERRUPTS
eHalStatus halIrqCheck(tHalHandle);

eHalStatus halIntDefaultRegServicesEnable(tHalHandle hHalHandle, eAniBoolean bEnable);

eHalStatus halIntGetErrorStatus(tHalHandle hHalHandle, 
    eHalIntSources intSource, tANI_U32 *intRegStatus, tANI_U32 *intRegMask);

//returns eHAL_STATUS_ALL_INTERRUPTS_PROCESSED or eHAL_STATUS_NO_INTERRUPTS
typedef eHalStatus (*tHalIsrFPtr)(tHddHandle);
 
eHalStatus halIntWriteCacheToEnableReg(tHalHandle hHal, eHalIntSources eHalIntSource);

eHalStatus
halIntClearStatus(tHalHandle hHalHandle, eHalIntSources interrupt);

#endif

