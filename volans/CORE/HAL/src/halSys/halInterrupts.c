/**
 *
 *  @file:         halInterrupts.c
 *
 *  @brief:        Implementation of Hal Interrupts.
 *
 *  @author:        Jeff
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 *  12/03/2007  Added Virgo specific changes.
 */

#include "halInterrupts.h"
#include "aniGlobal.h"
#include "halDebug.h"
#include "vos_api.h"
#include "vos_types.h"
#include "wlan_qct_bal.h"
#include "halTpe.h"
#include "halUtils.h"

#ifdef ANI_OS_TYPE_ANDROID
#include <linux/kernel.h>
#endif

//#define WLAN_PERF

#undef HAL_INT_DEBUG

// During software development we sometimes encounter coding issues
// with respect to interrupts.  HAL_INT_MAX_ITERATIONS is designed to
// be used during the software development phase to abort infinite
// loops cause by improperly serviced interrupts.  If set to zero, the
// code will loop until no interrupts are serviced in an iteration,
// which is how the production code should work

#define HAL_INT_MAX_ITERATIONS 5

// information about each interrupt register
typedef struct sHalIntRegisterInfo {
    char * name;
    tANI_U32 enableRegister;
    tANI_U32 statusRegister;
    tANI_U32 clearRegister;
} tHalIntRegisterInfo;

 // definition for unknown/unsupported register
#define HAL_INT_INVALID_HW_REGISTER ((tANI_U32) -1)

// map the register enumerations (eHalIntRegisters) to the chip
tHalIntRegisterInfo halIntRegisterInfo[eHAL_INT_MAX_REGISTER] = {

    // eHAL_INT_SIF_REGISTER
    {
        "SIF/Master",
        QWLAN_SIF_SIF_INT_EN_REG,
        QWLAN_SIF_SIF_INT_STATUS_REG,
        QWLAN_SIF_SIF_INT_CLR_REG
    },


    // eHAL_INT_MCU_HOST_INT_REGISTER
    {
        "HOST",
        QWLAN_MCU_MAC_HOST_INT_EN_REG,
        QWLAN_SIF_MCU_HOST_MAC_INT_STATUS_REG, /*SIF has a shadow intr status register*/
        QWLAN_MCU_MAC_HOST_INT_CLEAR_REG,
    },

    // eHAL_INT_MCU_MTU_INT_REGISTER
    {
        "MTU",
        QWLAN_MCU_MTU_HOST_INT_EN_REG,  /* dedicated enable reg for host */
        QWLAN_SIF_MCU_HOST_MTU_INT_STATUS_REG, /* SIF has a shadow intr status register */
        QWLAN_MCU_MTU_INT_CLEAR_REG  /* same clear register shared with ACPU */
    },
    
    // eHAL_INT_MCU_COMBINED_INT_REGISTER
    {
        "COMBINED",
        QWLAN_MCU_COMBINED_HOST_INT_EN_REG, /* dedicated enable reg for host */
        QWLAN_SIF_MCU_HOST_COMBINED_INT_STATUS_REG, /* SIF has a shadow intr status register */
        QWLAN_MCU_COMBINED_INT_CLEAR_REG /* same clear register shared with ACPU */
    },
    
    // eHAL_INT_MCU_BMU_WQ_INT_REGISTER
    {
        "BMUWQ",
        QWLAN_MCU_BMU_WQ_HOST_INT_EN_REG,  /* dedicated enable reg for host */
        QWLAN_SIF_MCU_HOST_BMU_WQ_INT_STATUS_REG,/* SIF has a shadow intr status register */
        QWLAN_MCU_BMU_WQ_INT_CLEAR_REG /* same clear register shared with ACPU */
    },

    // eHAL_INT_BMU_ERR_REGISTER
    {
        "BMUERR",
        QWLAN_BMU_ERR_INTR_ENABLE_REG,
        /* BMU's status and clear reg are the same. Read to get, write to clear */
        
        QWLAN_BMU_ERR_INTR_STATUS_REG,
        QWLAN_BMU_ERR_INTR_STATUS_REG
    },
    
    // eHAL_INT_BMU_IDLE_BD_PDU_REGISTER
    {
        "BMUBDPDU",
	QWLAN_BMU_BMU_IDLE_BD_PDU_STATUS_REG,
	QWLAN_BMU_BMU_IDLE_BD_PDU_STATUS_REG,
	QWLAN_BMU_BMU_IDLE_BD_PDU_STATUS_REG, 
    },
    
    // eHAL_INT_RPE_ERR_REGISTER
    {
        "RPEERR",
        QWLAN_RPE_ERR_INT_ENABLE_REG,
        /* RPE's status and clear reg are the same. Read to get, write to clear */
        
        QWLAN_RPE_ERR_INT_STATUS_REG,
        QWLAN_RPE_ERR_INT_STATUS_REG,
    },
    
    // eHAL_INT_DPU_REGISTER
    {
        "DPUERR",
        QWLAN_DPU_DPU_INTERRUPT_MASK_REG,
        /* DPU's status and clear reg are the same. Read to get, write to clear */
        
        QWLAN_DPU_DPU_INTERRUPT_STATUS_REG,
        QWLAN_DPU_DPU_INTERRUPT_STATUS_REG
    },

    // eHAL_INT_DXE_RAW_REGISTER
    {
        "DXE RAW",
        HAL_INT_INVALID_HW_REGISTER,
        QWLAN_DXE_0_INT_SRC_RAW_REG,
        // the "raw" clear register actually clears the underlying
        // ED, DONE, and ERR registers, which is something we never
        // want to do.  we clear those registers individually through
        // their separate clear registers
        HAL_INT_INVALID_HW_REGISTER
    },

    // eHAL_INT_DXE_ERR_REGISTER
    {
        "DXE ERR",
        HAL_INT_INVALID_HW_REGISTER,
        QWLAN_DXE_0_INT_ERR_SRC_REG,
        QWLAN_DXE_0_INT_ERR_CLR_REG
    },

    // eHAL_INT_MIF_REGISTER
    {
        "MIFERR",
        QWLAN_MIF_MIF_INT_EN_REG,
        QWLAN_MIF_MIF_INT_REG,
        QWLAN_MIF_MIF_INT_CLR_REG
    },

    // eHAL_INT_ADU_REGISTER
    {
        "ADUERR",
        QWLAN_ADU_INTERRUPT_ENABLE_REG,
        /* ADU's status and clear reg are the same. Read to get, write to clear */
        QWLAN_ADU_INTERRUPT_STATUS_REG,
        QWLAN_ADU_INTERRUPT_STATUS_REG  
    },

    // eHAL_INT_RXP_REGISTER
    {
        "RXP",
        QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_REG,
        /* RxP's status and clear reg are the same. Read to get, write to clear */
        QWLAN_RXP_RXP_GROUPED_INTERRUPT_STATUS_REG,
        QWLAN_RXP_RXP_GROUPED_INTERRUPT_STATUS_REG 
    },



};



// forward declarations
static int halIntServiceRegister(tHalHandle hHalHandle, eHalIntRegisters intReg);
static eHalStatus halIntServiceInterrupt(tHalHandle hHalHandle, eHalIntSources intReg);
static eHalStatus halIntDxeRawHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntMifGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntSifAsicGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntAduGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntDpuGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntRxpGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntRpeGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntBmuErrGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntBmuWqGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntDefaultHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntMtuGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);
static eHalStatus halIntMcuCombinedGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource);

tANI_U32 halIntGetStatus(tHalHandle hHalHandle, eHalIntRegisters intReg);

// per-interrupt statistics
static tANI_U32 halIntCounter[eHAL_INT_MAX_SOURCE] = {0};

// information about each interrupt source
typedef struct sHalIntSourceInfo {
    tANI_BOOLEAN isLeaf;
    eHalIntRegisters eRegister;
    tANI_U32 mask;
    pHalIntHandler pHandler;
} tHalIntSourceInfo;

// All supported interrupt sources. 
// Follows the enum order defined in eHalIntSources

tHalIntSourceInfo halIntInfo[eHAL_INT_MAX_SOURCE] = {

    // eHAL_INT_SIF_ASIC
    {
        0,
        eHAL_INT_SIF_REGISTER,
#ifdef WLAN_PERF
        QWLAN_SIF_INT_ASIC_INTERRUPT_MASK,
#else
        QWLAN_SIF_SIF_INT_CLR_ASIC_INTR_CLR_MASK,
#endif
        halIntSifAsicGroupHandler
    },

    //////////////////////////////////////////////////////


    //eHAL_INT_MCU_HOST_INT_WQ_DATA_AVAIL    
    {
        0, /* group interrupt for BMU WQs */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_WQ_DATA_AVAIL_INT_EN_MASK,
        halIntBmuWqGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_MIF    
    {
        0, /* group interrupt for MIF */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_MIF_INT_EN_MASK,
        halIntMifGroupHandler
    },

#ifndef WLAN_SOFTAP_FEATURE
    //eHAL_INT_MCU_HOST_INT_MTU_TIMER_5
    {
        1, /* interrupt for MTU_TIMER5 as pre-beacon interrupt */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_MTU_TIMER5_EN_MASK,
        halIntMtuHandlePreBeaconTmr
    },
#endif    
    
    //eHAL_INT_MCU_HOST_INT_ADU    
    {
        0, /* group interrupt for ADU */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_ADU_INT_EN_MASK,
        halIntAduGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_RPE    
    {
        0, /* group interrupt for RPE */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        0,//QWLAN_MCU_MAC_HOST_INT_EN_RPE_INT_EN_MASK,
        halIntRpeGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_COMBINED    
    {
        0, /* other combined interrupts to host*/
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_COMBINED_INT_EN_MASK,
        halIntMcuCombinedGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_RXP    
    {
        0, /* group interrupt for RxP */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        /* This is a workaround for interrupt
         * sharing between Host and FW. Currently
         * disable all RXP interrupt from the Host
         */
        0,//QWLAN_MCU_MAC_HOST_INT_EN_RXP_INT_EN_MASK,
        halIntRxpGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_DXE_CH0    
    {
        1,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_DXE_MXU_CHAIN_0_INT_EN_MASK,
        halIntDXEErrorHandler
    },

    //eHAL_INT_MCU_HOST_INT_DXE_CH1    
    {
        1,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_DXE_MXU_CHAIN_1_INT_EN_MASK,
        halIntDXEErrorHandler
    },

    //eHAL_INT_MCU_HOST_INT_DXE_CH2    
    {
        1,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_DXE_MXU_CHAIN_2_INT_EN_MASK,
        halIntDXEErrorHandler
    },

    //eHAL_INT_MCU_HOST_INT_MBOX0    
    {
        0,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_ACPU_TO_HOST_MB0_INT_EN_MASK,
        halMbox_HandleInterrupt
    },

    //eHAL_INT_MCU_HOST_INT_MBOX1    
    {
        0,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_ACPU_TO_HOST_MB1_INT_EN_MASK,
        halMbox_HandleInterrupt
    },

    //eHAL_INT_MCU_HOST_INT_MBOX2    
    {
        0,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_ACPU_TO_HOST_MB2_INT_EN_MASK,
        halMbox_HandleInterrupt
    },

    //eHAL_INT_MCU_HOST_INT_MBOX3    
    {
        0,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_ACPU_TO_HOST_MB3_INT_EN_MASK,
        halMbox_HandleInterrupt
    },


    //eHAL_INT_MCU_HOST_INT_MTU    
    {
        0, /* group interrupt for MTU */
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_MTU_INT_EN_MASK,
        halIntMtuGroupHandler
    },

    //eHAL_INT_MCU_HOST_INT_DEFAULT    
    {
        0,
        eHAL_INT_MCU_HOST_INT_REGISTER,
        QWLAN_MCU_MAC_HOST_INT_EN_DEFAULT_MASK, /* all other MCU_HOST interrupts handler */
        halIntDefaultHandler
    },

    /////////////////////////////////////////////////////

    //eHAL_INT_MCU_MTU_INT_ERROR    
    {
        1,
        eHAL_INT_MCU_MTU_INT_REGISTER,
        QWLAN_MCU_MTU_INT_EN_MTU_ERR_EN_MASK,
        halMTU_DefInterruptHandler
    },
    //eHAL_INT_MCU_MTU_INT_WD_ENABLE_DISABLE_ERROR    
    {
        1,
        eHAL_INT_MCU_MTU_INT_REGISTER,
        QWLAN_MCU_MTU_INT_EN_WD_ENABLE_DISABLE_ERROR_EN_MASK,
        halMTU_DefInterruptHandler
    },
    //eHAL_INT_MCU_MTU_INT_WD_PROTECTION_ERROR    
    {
        1,
        eHAL_INT_MCU_MTU_INT_REGISTER,
        QWLAN_MCU_MTU_INT_EN_WD_PROTECTION_ERROR_EN_MASK,
        halMTU_DefInterruptHandler
    },

    ///////////////////////////////////////////////////////

    //eHAL_INT_MCU_COMBINED_INT_TPE_MCU_BD_BASED_TX_INT_1_P_HOSTINT_EN    
    {
        1, /* group interrupt for TX complete interrrupt for bit 1 */
        eHAL_INT_MCU_COMBINED_INT_REGISTER,
        TPE_HOST_TX_COMPLETE_INT_EN_MASK,
        halIntTpeMcuBdBasedTxInt1PHostHandler
    },

    //eHAL_INT_MCU_COMBINED_INT_BMU_ERROR    
    {
        0, /* group interrupt for BMU error */
        eHAL_INT_MCU_COMBINED_INT_REGISTER,
        QWLAN_MCU_COMBINED_HOST_INT_EN_BMU_MCU_ERR_INT_HOSTINT_EN_MASK,
        halIntBmuErrGroupHandler
    },

    //eHAL_INT_MCU_COMBINED_INT_DPU_ERROR    
    {
        0,  /* group interrupt for DPU */
        eHAL_INT_MCU_COMBINED_INT_REGISTER,
        QWLAN_MCU_COMBINED_HOST_INT_EN_DPU_MCU_ERRINTR_P_HOSTINT_EN_MASK,
        halIntDpuGroupHandler
    },

    //eHAL_INT_MCU_COMBINED_INT_DEFAULT    
    {
        0,  /* default combined group interrupt handler. Not handled */
        eHAL_INT_MCU_COMBINED_INT_REGISTER,
        QWLAN_MCU_COMBINED_HOST_INT_EN_DEFAULT_MASK,
        halIntDefaultHandler
    },

    //eHAL_INT_MCU_BMU_WQ_INT_WQ_DEFAULT    
    {
        0,  /* default group interrupt for BMU WQ */
        eHAL_INT_MCU_BMU_WQ_INT_REGISTER,
        QWLAN_MCU_BMU_WQ_HOST_INT_EN_DEFAULT_MASK,
        halIntBmuWqHandler
    },

    // eHAL_INT_BMU_ERROR_DEFAULT
    {
        0, /* default group interrupt for BMU error */
        eHAL_INT_BMU_ERR_REGISTER,
        QWLAN_BMU_ERR_INTR_EN_DEFAULT_MASK,
        halIntBMUErrorHandler
    },
    
    // eHAL_INT_BMU_IDLE_BD_PDU_INT
    // The below interrupt is unlike the other interrupts where the interrupt status is read from
    // status registers. This interrupt basically allows host to configure BMU to interrupt whenever
    // the total number of idle BD/PDUs are greater than the threshold set. The interrupt enable bit 
    // and the threshold to be set resides in the same register i.e. bmu_idle_bd_pdu_status_reg. Here,
    // the value 0xc8 corresponds to setting 200 as the threshold.
    {
        1, /* default group interrupt for BMU error */
        eHAL_INT_BMU_IDLE_BD_PDU_REGISTER,
        QWLAN_BMU_BMU_IDLE_BD_PDU_STATUS_BMU_IDLE_BD_PDU_THRESHOLD_INTERRUPT_ENABLE_MASK | 0x21C,
        halIntBMUIdleBdPduHandler
    },

    // eHAL_INT_RPE_ERR_DEFAULT
    {
        0, /* default group interrupt for RPE error */
        eHAL_INT_RPE_ERR_REGISTER,
        QWLAN_RPE_ERR_INT_EN_DEFAULT_MASK,
        halRpe_ErrIntHandler
    },
       
    // eHAL_INT_DPU_ERR_DEFAULT
    {
        0,
        eHAL_INT_DPU_REGISTER,
        QWLAN_DPU_DPU_INTERRUPT_EN_DEFAULT_MASK,
        halIntDPUErrorHandler
    },

    // eHAL_INT_DXE_RAW_CHANNEL_0
    {
        0,
        eHAL_INT_DXE_RAW_REGISTER,
        QWLAN_DXE_0_INT_SRC_RAW_CH0_INT_MASK,
        halIntDxeRawHandler
    },
    // eHAL_INT_DXE_RAW_CHANNEL_1
    {
        0,
        eHAL_INT_DXE_RAW_REGISTER,
        QWLAN_DXE_0_INT_SRC_RAW_CH1_INT_MASK,
        halIntDxeRawHandler
    },
    // eHAL_INT_DXE_RAW_CHANNEL_2
    {
        0,
        eHAL_INT_DXE_RAW_REGISTER,
        QWLAN_DXE_0_INT_SRC_RAW_CH2_INT_MASK,
        halIntDxeRawHandler
    },
  
    // eHAL_INT_DXE_ERR_CHANNEL_0
    {
        1,
        eHAL_INT_DXE_ERR_REGISTER,
        QWLAN_DXE_0_INT_ERR_SRC_CH0_ERR_INT_MASK,
        halIntDXEErrorHandler
    },
    // eHAL_INT_DXE_ERR_CHANNEL_1
    {
        1,
        eHAL_INT_DXE_ERR_REGISTER,
        QWLAN_DXE_0_INT_ERR_SRC_CH1_ERR_INT_MASK,
        halIntDXEErrorHandler
    },
    // eHAL_INT_DXE_ERR_CHANNEL_2
    {
        1,
        eHAL_INT_DXE_ERR_REGISTER,
        QWLAN_DXE_0_INT_ERR_SRC_CH2_ERR_INT_MASK,
        halIntDXEErrorHandler
    },
#ifdef WLAN_HAL_VOLANS
    // QWLAN_MIF_MIF_INT_EN_MIF_AHB_INVALID_ADDR_INT
    {
        0,
        eHAL_INT_MIF_REGISTER,
        QWLAN_MIF_MIF_INT_EN_MIF_AHB_INVALID_ADDR_INT_EN_MASK,
        halIntMIFErrorHandler
    },
    // QWLAN_MIF_MIF_INT_EN_MIF_ACPU_INVALID_RADDR_INT
    {
        0,
        eHAL_INT_MIF_REGISTER,
        QWLAN_MIF_MIF_INT_EN_MIF_ACPU_INVALID_RADDR_INT_EN_MASK,
        halIntMIFErrorHandler
    },
    
    // QWLAN_MIF_MIF_INT_EN_MIF_ACPU_INVALID_WADDR_INT
    {
        0,
        eHAL_INT_MIF_REGISTER,
        QWLAN_MIF_MIF_INT_EN_MIF_ACPU_INVALID_WADDR_INT_EN_MASK,
        halIntMIFErrorHandler
    },
#else
    // eHAL_INT_MIF_ERR_DEFAULT
    {
        0,
        eHAL_INT_MIF_REGISTER,
        QWLAN_MIF_MIF_INT_EN_MIF_INVALID_ADDRESS_INT_EN_MASK,
        halIntMIFErrorHandler
    },
#endif
    
    // eHAL_INT_ADU_ERR_DEFAULT
    {
        0,
        eHAL_INT_ADU_REGISTER,
        QWLAN_ADU_INTERRUPT_ENABLE_DEFAULT_MASK,
        halAdu_ErrIntHandler
    },

    // eHAL_INT_RXP_ERR_DEFAULT
    {
        0,
        eHAL_INT_RXP_REGISTER,
        QWLAN_RXP_RXP_GROUPED_INTERRUPT_ENABLE_DEFAULT_MASK,
        halRxp_ErrIntHandler
    },
           
};

eHalIntSources intRegBmuIdleBdPduService[] = {
    eHAL_INT_BMU_IDLE_BD_PDU_INT,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegBmuService[] = {
    eHAL_INT_BMU_ERROR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegRpeService[] = {
    eHAL_INT_RPE_ERR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegDpuService[] = {
    eHAL_INT_DPU_ERR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegDxeRawService[] = {
    eHAL_INT_DXE_RAW_CHANNEL_0,
    eHAL_INT_DXE_RAW_CHANNEL_1,
    eHAL_INT_DXE_RAW_CHANNEL_2,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegDxeErrService[] = {
     eHAL_INT_DXE_ERR_CHANNEL_0,
     eHAL_INT_DXE_ERR_CHANNEL_1,
     eHAL_INT_DXE_ERR_CHANNEL_2,
     eHAL_INT_MAX_SOURCE
};

#ifdef WLAN_HAL_VOLANS
eHalIntSources intRegMifService[] = {
    eHAL_INT_MIF_ERR_AHB_INVALID_ADDR,
    eHAL_INT_MIF_ERR_ACPU_INVALID_RADDR,
    eHAL_INT_MIF_ERR_AHB_INVALID_WADDR,    
    eHAL_INT_MAX_SOURCE
};
#else
eHalIntSources intRegMifService[] = {
    eHAL_INT_MIF_ERR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};
#endif

eHalIntSources intRegSifService[] = {
    eHAL_INT_SIF_ASIC,
    eHAL_INT_MAX_SOURCE
};


eHalIntSources intRegAduService[] = {
    // eHAL_INT_ADU_REGISTER,
    eHAL_INT_ADU_ERR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};


eHalIntSources intRegRxpService[] = {

    // eHAL_INT_RXP_REGISTER,
    eHAL_INT_RXP_ERR_DEFAULT,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegMcuHostService[] = {

    // eHAL_INT_MCU_HOST_INT_REGISTER,

    eHAL_INT_MCU_HOST_INT_WQ_DATA_AVAIL,
    eHAL_INT_MCU_HOST_INT_MIF,
#ifndef WLAN_SOFTAP_FEATURE    
    eHAL_INT_MCU_HOST_INT_MTU_TIMER_5,
#endif    
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
    eHAL_INT_MCU_HOST_INT_DEFAULT,
    eHAL_INT_MAX_SOURCE
};



eHalIntSources intRegMcuMtuService[] = {
         
    // eHAL_INT_MCU_MTU_INT_REGISTER,
    eHAL_INT_MCU_MTU_INT_ERROR,
    eHAL_INT_MCU_MTU_INT_WD_ENABLE_DISABLE_ERROR,
    eHAL_INT_MCU_MTU_INT_WD_PROTECTION_ERROR,
    eHAL_INT_MAX_SOURCE
};

eHalIntSources intRegMcuCombinedService[] = {

    // eHAL_INT_MCU_COMBINED_INT_REGISTER,
    eHAL_INT_MCU_COMBINED_INT_TPE_MCU_BD_BASED_TX_INT_1_P_HOSTINT_EN,
    eHAL_INT_MCU_COMBINED_INT_BMU_ERROR, 
    eHAL_INT_MCU_COMBINED_INT_DPU_ERROR, 
    eHAL_INT_MCU_COMBINED_INT_DEFAULT, 
    eHAL_INT_MAX_SOURCE
    
};


eHalIntSources intRegMcuBmuWqService[] = {

    // eHAL_INT_MCU_BMU_WQ_INT_REGISTER,
    eHAL_INT_MCU_BMU_WQ_INT_WQ_DEFAULT,
    eHAL_INT_MAX_SOURCE
};


//should follow the order in eHalIntRegisters definition

eHalIntSources * intRegService[eHAL_INT_MAX_REGISTER] = {
    intRegSifService,                // eHAL_INT_SIF_REGISTER,
    intRegMcuHostService,           // eHAL_INT_MCU_HOST_INT_REGISTER
    intRegMcuMtuService,            // eHAL_INT_MCU_MTU_INT_REGISTER
    intRegMcuCombinedService,       // eHAL_INT_MCU_COMBINED_INT_REGISTER
    intRegMcuBmuWqService,           // eHAL_INT_MCU_BMU_WQ_INT_REGISTER
    intRegBmuService,               // eHAL_INT_BMU_ERR_REGISTER,
    intRegBmuIdleBdPduService,      // eHAL_INT_BMU_ERR_REGISTER,    
    intRegRpeService,               // eHAL_INT_RPE_ERR_REGISTER,
    intRegDpuService,               // eHAL_INT_DPU_REGISTER,
    intRegDxeRawService,            // eHAL_INT_DXE_RAW_REGISTER,
    intRegDxeErrService,            // eHAL_INT_DXE_ERR_REGISTER,
    intRegMifService,               // eHAL_INT_MIF_REGISTER,
    intRegAduService,               // eHAL_INT_ADU_REGISTER,
    intRegRxpService,               // eHAL_INT_RXP_REGISTER,
};

char * intRegServiceName[eHAL_INT_MAX_REGISTER] = {
   "SIF", "McuHost", "Mtu", "Combined", "Bmuwq" , "Bmu", "BmuIdleBdPdu", "Rpe", "Dpu", "DxeRaw", "DxeErr", "Mif", "Adu", "Rxp"
};



static eHalStatus
halIntReadRegister(tpAniSirGlobal pMac,
                   tANI_U32 regAddress,
                   tANI_U32 *pRegValue)
{
    eHalStatus status;

    status = halReadRegister(pMac, regAddress, pRegValue);

    return status;
}

static eHalStatus
halIntWriteRegister(tpAniSirGlobal pMac,
                    tANI_U32 regAddress,
                    tANI_U32 regValue)
{

    eHalStatus status;
	
    if(regAddress != QWLAN_SIF_SIF_INT_EN_REG){
        status = halWriteRegister(pMac, regAddress, regValue);
    }else{
        v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *)pMac);
#ifdef WLAN_PERF
        if(regValue)
            status = WLANBAL_EnableASICInterruptEx(pVosGCtx, QWLAN_SIF_INT_ASIC_INTERRUPT_MASK);
        else
            status = WLANBAL_DisableASICInterruptEx(pVosGCtx, QWLAN_SIF_INT_ASIC_INTERRUPT_MASK);
#else
        if(regValue)
            status = WLANBAL_EnableASICInterrupt(pVosGCtx);
        else
            status = WLANBAL_DisableASICInterrupt(pVosGCtx);
#endif
    }
    return status;

};



eHalStatus
halIntClearStatus(tHalHandle hHalHandle, eHalIntSources interrupt)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntRegisters eRegister;
    tANI_U32 hwRegister;
    tANI_U32 mask;

    eRegister = halIntInfo[interrupt].eRegister;
    hwRegister = halIntRegisterInfo[eRegister].clearRegister;
    if (HAL_INT_INVALID_HW_REGISTER == hwRegister)
    {
        // we return success because some interrupt status registers
        // legitimately don't need to be cleared (i.e. mailbox)
        return eHAL_STATUS_SUCCESS;
    }
    mask = halIntInfo[interrupt].mask;
    return (halIntWriteRegister(pMac, hwRegister, mask));
}



/** -------------------------------------------------------------
\fn halIntWriteCacheToEnableReg
\brief Write the cache value to the enable register.
\param   tHalHandle hHalHandle
\param   eHalIntSources timerIntr
\return eHalStatus - status
  -------------------------------------------------------------*/
  
eHalStatus halIntWriteCacheToEnableReg(tHalHandle hHal, eHalIntSources eHalIntSource)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    eHalIntRegisters eReg = halIntInfo[eHalIntSource].eRegister;
    tANI_U32 hwReg = halIntRegisterInfo[eReg].enableRegister;
    return halIntWriteRegister(pMac,
                                    hwReg,
                                    pMac->hal.intEnableCache[eReg].value);
}


static eHalStatus
halIntDxeRawHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{

    tANI_U32 channel = intSource - eHAL_INT_DXE_RAW_CHANNEL_0;

    // the DXE INT_SRC_RAW interrupt status register mirrors the "or"
    // of the per-channel bits in the DXE INT_ED_SRC, INT_DONE_SRC, and
    // INT_ERR_SRC interrupt status registers.

#if 0  //From Programmer's guide, Gen6 doesn't need to handle ED and DONE interrupts

    // is the ED interrupt pending?
    intSource = eHAL_INT_DXE_ED_CHANNEL_0 + channel;
    status = halIntServiceInterrupt(hHalHandle, intSource);

    // is the DONE interrupt pending
    intSource = eHAL_INT_DXE_DONE_CHANNEL_0 + channel;
    status = halIntServiceInterrupt(hHalHandle, intSource);
#endif

    // is the ERR interrupt pending?
    intSource = (eHalIntSources) (eHAL_INT_DXE_ERR_CHANNEL_0 + channel);
    halIntServiceInterrupt(hHalHandle, intSource);

    return (eHAL_STATUS_SUCCESS);
}

eHalStatus halIntGetErrorStatus(tHalHandle hHalHandle, eHalIntSources intSource, tANI_U32 *intRegStatus, tANI_U32 *intRegMask)
{
    eHalIntRegisters eReg;
    tANI_U32 intEnableRegister;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    eReg = halIntInfo[intSource].eRegister;
    if(intRegStatus)
        *(intRegStatus) = halIntGetStatus(hHalHandle, eReg);

    if(intRegMask){
        *intRegMask = 0;
        if( (intEnableRegister = halIntRegisterInfo[eReg].enableRegister) != HAL_INT_INVALID_HW_REGISTER)
            return halIntReadRegister(pMac, intEnableRegister, intRegMask);
    }
    return eHAL_STATUS_SUCCESS;
}


static eHalStatus
halIntDefaultHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
#ifdef WLAN_DEBUG
    tANI_U32 intReg = halIntRegisterInfo[halIntInfo[intSource].eRegister].enableRegister;
#endif
    tANI_U32 intRegMask, intRegStatus;

#ifdef CMD53_CLOCK_SWITCHING_HW_BUG_WORKAROUND
    tANI_U32 uStatus = 0;

    /* TO DO: Remove this workaround for Libra 2.0
     * FW trigers SW interrupt in the wakeup cases 
     * where Host activity is NOT present. To get 
     * SD clk running in all Exit/Enter SIF Freeze cases.
     */
    halReadRegister(pMac, QWLAN_MCU_SW_INT_1_REG, &uStatus);

    if (uStatus) {
        palWriteRegister(pMac, QWLAN_MCU_SW_INT_1_REG, 0x0); 
    } else 
#endif
        
    if( eHAL_STATUS_SUCCESS == halIntGetErrorStatus( hHalHandle, intSource, &intRegStatus, &intRegMask))
       HALLOGE( halLog(pMac, LOGE,
           FL("Unhandled HAL interrupt %d triggered (Reg 0x%08x = 0x%08x Status=0x%08x"), intSource, intReg , intRegMask, intRegStatus));
    return (eHAL_STATUS_SUCCESS);
}

static eHalStatus halIntSifAsicGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_MCU_HOST_INT_REGISTER);
    return eHAL_STATUS_SUCCESS;
}

static eHalStatus halIntMifGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_MIF_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntAduGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_ADU_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntRpeGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_RPE_ERR_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntRxpGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
     halIntServiceRegister(hHalHandle, eHAL_INT_RXP_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntDpuGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
     halIntServiceRegister(hHalHandle, eHAL_INT_DPU_REGISTER);
    return eHAL_STATUS_SUCCESS;
}

static eHalStatus halIntBmuWqGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_MCU_BMU_WQ_INT_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntBmuErrGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_BMU_ERR_REGISTER);
    /* The IDLE_BD_PDU interrupt is given to the host in the same way as BMU error interrupt.
       But the enable register for this interrupt is not similar to the other interrupts. Hence
       this interrupt should be checked whenever there is a BMU error interrupt. If there is no
       error, if idle BD/PDU interrupt is asserted, BMU error interrupt would be given to the host
       and host should handle both. This is a special case. */
    halIntServiceRegister(hHalHandle, eHAL_INT_BMU_IDLE_BD_PDU_REGISTER);
    return eHAL_STATUS_SUCCESS;
}
static eHalStatus halIntMtuGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_MCU_MTU_INT_REGISTER);
    return eHAL_STATUS_SUCCESS;
}

static eHalStatus halIntMcuCombinedGroupHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    halIntServiceRegister(hHalHandle, eHAL_INT_MCU_COMBINED_INT_REGISTER);
    return eHAL_STATUS_SUCCESS;
}

eHalStatus
halIntDumpRegister(tpAniSirGlobal pMac, tANI_U32 hwRegister, char * buff)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 hwValue;

    if (HAL_INT_INVALID_HW_REGISTER == hwRegister)
    {
        // blank string
        *buff = '\0';
    }
    else
    {
        status = halIntReadRegister(pMac, hwRegister, &hwValue);
        if (eHAL_STATUS_SUCCESS != status)
        {
            sprintf(buff, "%08x Read ERR", (int)hwRegister);
        }
        else
        {
            sprintf(buff, "%08x=%08x", (int)hwRegister, (int)hwValue);
        }
    }
    return (status);
}

eHalStatus
halIntDumpRegisters(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntRegisters eReg;
    tANI_U32 enableRegister;
    tANI_U32 statusRegister;
    char enableBuf[20];
    char statusBuf[20];

    HALLOGW( halLog(pMac, LOGW,
           FL("Name     Status             Enable             SW Mask")));
    HALLOGW( halLog(pMac, LOGW,
           FL("-------- -------- --------  -------- --------  --------")));

    for (eReg = eHAL_INT_SIF_REGISTER; eReg < eHAL_INT_MAX_REGISTER; eReg++)
    {
        enableRegister = halIntRegisterInfo[eReg].enableRegister;
        statusRegister = halIntRegisterInfo[eReg].statusRegister;
        halIntDumpRegister(pMac, enableRegister, enableBuf);
        halIntDumpRegister(pMac, statusRegister, statusBuf);
        HALLOGW( halLog(pMac, LOGW, FL("%-8s %17s  %17s  %08x%s"),
               halIntRegisterInfo[eReg].name, statusBuf, enableBuf,
               pMac->hal.intEnableCache[eReg].value,
               pMac->hal.intEnableCache[eReg].valid ? "" : "*"));
    }
    HALLOGW( halLog(pMac, LOGW, FL(" intEnabled[%d]"), pMac->hal.intEnabled));
    return (eHAL_STATUS_SUCCESS);
}



eHalStatus
halIntDumpInterrupts(tHalHandle hHalHandle)
{
    eHalIntSources eInt;
    HALLOGW( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    HALLOGW( halLog(pMac, LOGW,
           FL("Name                               Count")));
    HALLOGW( halLog(pMac, LOGW,
           FL("---------------------------------- --------")));

    for (eInt = eHAL_INT_SIF_ASIC; eInt < eHAL_INT_MAX_SOURCE; eInt++)
    {
        HALLOGW( halLog(pMac, LOGW, FL("%-d %08x"),
               eInt, 
               halIntCounter[eInt]));
    }
    return (eHAL_STATUS_SUCCESS);
}



eHalStatus
halIntFlushCache(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    return palZeroMemory(pMac->hHdd,
                         pMac->hal.intStatusCache,
                         sizeof(pMac->hal.intStatusCache));
}


tANI_U32
halIntGetStatus(tHalHandle hHalHandle, eHalIntRegisters intReg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32 intStatusRegister;
    eHalStatus status;

    // is the interrupt already cached?
    if (!pMac->hal.intStatusCache[intReg].valid)
    {
        // no, get the interrupt status register
        intStatusRegister = halIntRegisterInfo[intReg].statusRegister;

        // is this register supported?
        if (HAL_INT_INVALID_HW_REGISTER == intStatusRegister)
        {
            // no, just say nothing pending
            return 0;
        }

        // get the interrupt register value
        status = halIntReadRegister(pMac,
                                    intStatusRegister,
                                    &pMac->hal.intStatusCache[intReg].value);
        if (eHAL_STATUS_SUCCESS != status)
        {
            // just say nothing pending
            return 0;
        }

        // we have now cached the register
        pMac->hal.intStatusCache[intReg].valid = 1;
    }

    // return the cached value
    return pMac->hal.intStatusCache[intReg].value;
}




int
halIntServiceRegister(tHalHandle hHalHandle, eHalIntRegisters intReg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntSources * pIntRegService;
    eHalIntSources intSource;
    tANI_U32 intMask;
    tANI_U32 intRegStatus;
    int count = 0;

    // get ordered list of interrupts we want to service from this register
    pIntRegService = intRegService[intReg];

    // get the (possibly cached) value of the interrupt status register
    intRegStatus = halIntGetStatus(hHalHandle, intReg);

    // get the cached value of the interrupt enable mask
    intMask = pMac->hal.intEnableCache[intReg].value;

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s intr register status %08x mask %08x"),
           intRegServiceName[intReg],
           intRegStatus, intMask));
#endif // HAL_INT_DEBUG

    // only service interrupts that are enabled
    intRegStatus &= intMask;

    // loop until we service all the interrupts
    while (intRegStatus) {

        // select interrupt source to process
        intSource = *pIntRegService;
        pIntRegService++;


        // have we handled all sources for this register?
        if (intSource >= eHAL_INT_MAX_SOURCE) {
            // we've handled all that we were programmed to handle
#ifdef HAL_INT_DEBUG
            HALLOGW( halLog(pMac, LOGW, FL("EOL")));
#endif // HAL_INT_DEBUG
            break;
        }

        // retrieve the interrupt bit mask for the selected interrupt
        intMask = halIntInfo[intSource].mask;

#ifdef HAL_INT_DEBUG
        //halLog(pMac, LOGW, FL("testing int %d mask %08x"),
        //       intSource, intMask);
#endif // HAL_INT_DEBUG

        // is this interrupt pending?
        if (intRegStatus & intMask) {

            // yes

            // is it a leaf interrupt?
            if (halIntInfo[intSource].isLeaf) {
                // yes, so clear the interrupt status before processing
                halIntClearStatus(hHalHandle, intSource);
            }

            // process the interrupt
#ifdef HAL_INT_DEBUG
            HALLOGW( halLog(pMac, LOGW, FL("Handling intSource=%d"), intSource));
#endif // HAL_INT_DEBUG
            halIntCounter[intSource]++;
            (halIntInfo[intSource].pHandler)(hHalHandle, intSource);

            // was it a leaf interrupt?
            if (!halIntInfo[intSource].isLeaf) {
                // no, so clear the interrupt status after processing
                halIntClearStatus(hHalHandle, intSource);
            }

            // this interrupt was processed
            intRegStatus &= ~intMask;

#ifdef HAL_INT_DEBUG
            //halLog(pMac, LOGW, FL("status now %08x"),
            //       intRegStatus);
#endif // HAL_INT_DEBUG

            // we processed an interrupt
            count++;
        }
    }
    return (count);
}

eHalStatus
halIntServiceInterrupt(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntRegisters intReg;
    tANI_U32 intMask;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;

    // get register that corresponds to the interrupt
    intReg = halIntInfo[intSource].eRegister;

    // get the (possibly cached) value of the interrupt status register
    intRegStatus = halIntGetStatus(hHalHandle, intReg);

    // get the cached value of the interrupt enable mask
    intRegMask = pMac->hal.intEnableCache[intReg].value;

    // retrieve the interrupt bit mask for the selected interrupt
    intMask = halIntInfo[intSource].mask;

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW,
           FL("%s register intr status %08x mask %08x intSource=%d mask %08x"),
           halIntRegisterInfo[intReg].name,
           intRegStatus, intRegMask,
           intSource, intMask));
#endif // HAL_INT_DEBUG

    // is this interrupt enabled & pending?
    if (intRegStatus & intRegMask & intMask) {

        // yes

        // is it a leaf interrupt?
        if (halIntInfo[intSource].isLeaf) {
            // yes, so clear the interrupt status before processing
            halIntClearStatus(hHalHandle, intSource);
        }

        // process the interrupt
#ifdef HAL_INT_DEBUG
        HALLOGW( halLog(pMac, LOGW, FL("Handling intSOurce=%d"), intSource));
#endif // HAL_INT_DEBUG
        halIntCounter[intSource]++;
        (halIntInfo[intSource].pHandler)(hHalHandle, intSource);

        // was it a leaf interrupt?
        if (!halIntInfo[intSource].isLeaf) {
            // no, so clear the interrupt status after processing
            halIntClearStatus(hHalHandle, intSource);
        }

        return (eHAL_STATUS_SUCCESS);
    }
    return (eHAL_STATUS_NO_INTERRUPTS);
}

/////////////////////////////

eHalStatus
halIntSdioPopulateCache(tHalHandle hHalHandle, tANI_U32 sifIntStatus)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    halIntFlushCache(hHalHandle);
    pMac->hal.intStatusCache[eHAL_INT_SIF_REGISTER].value = sifIntStatus;
    pMac->hal.intStatusCache[eHAL_INT_SIF_REGISTER].valid = 1;

    return (eHAL_STATUS_SUCCESS);
}


// This halIntCheck is coming directly from Gen6 diagnostics branch.
// Once BAL/SIF becomes ready, it won't read the SIF status reg but the
// MCU 
eHalStatus
halIntCheck(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus regStatus;
    tANI_U32 mcuMacHostIntStatus;
    tANI_U32 mcuMacHostIntMask;
#if HAL_INT_MAX_ITERATIONS > 0
    static tANI_U32 counter = 0;
#endif // HAL_INT_MAX_ITERATIONS > 0
#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s"), __FUNCTION__));
#endif // HAL_INT_DEBUG


#if 0
    regStatus = halIntReadRegister(pMac,
                                    QWLAN_MCU_MAC_HOST_INT_STATUS_REG,
                                    &mcuMacHostIntStatus);
    if (regStatus != eHAL_STATUS_SUCCESS)
    {
        return (regStatus);
    }

    // once we have production code, we want to rely upon the cached
    // value of the register.  during bringup though we'll read the
    // hardware and verify the cache is valid
    regStatus = halIntReadRegister(pMac,
                                    QWLAN_MCU_MAC_HOST_INT_EN_REG,
                                    &mcuMacHostIntMask);
    if (regStatus != eHAL_STATUS_SUCCESS)
    {
        return (regStatus);
    }

#ifdef HAL_INT_DEBUG
    if (pMac->hal.intEnableCache[eHAL_INT_MCU_HOST_INT_REGISTER].valid && 
        (mcuMacHostIntMask != pMac->hal.intEnableCache[eHAL_INT_MCU_HOST_INT_REGISTER].value))
    {
        HALLOGE( halLog(pMac, LOGE, FL("%s: MASK MISMATCH:  HW=%08x  SW=%08x  valid=%d"),
               __FUNCTION__, mcuMacHostIntMask,
               pMac->hal.intEnableCache[eHAL_INT_MCU_HOST_INT_REGISTER].value,
               pMac->hal.intEnableCache[eHAL_INT_MCU_HOST_INT_REGISTER].valid));
        // pMac->hal.intEnableCache[eHAL_INT_PIF_PCI_REGISTER].value = sifIntMask;
    }
#endif    
#else

    regStatus = halIntReadRegister(pMac,
                                    QWLAN_SIF_MCU_HOST_MAC_INT_STATUS_REG,
                                    &mcuMacHostIntStatus);
    if (regStatus != eHAL_STATUS_SUCCESS)
    {
        return (regStatus);
    }
    
    mcuMacHostIntMask = pMac->hal.intEnableCache[eHAL_INT_MCU_HOST_INT_REGISTER].value;

#endif
    if (mcuMacHostIntStatus & mcuMacHostIntMask)
    {
        // there are one or more interrupts pending

        // invalidate the prior cache
        halIntFlushCache(hHalHandle);

        // cache the value just read
        pMac->hal.intStatusCache[eHAL_INT_MCU_HOST_INT_REGISTER].value = mcuMacHostIntStatus;
        pMac->hal.intStatusCache[eHAL_INT_MCU_HOST_INT_REGISTER].valid = 1;

#if HAL_INT_MAX_ITERATIONS > 0
        // reset "calls w/o interrupt" counter
        counter = 0;
#endif // HAL_INT_MAX_ITERATIONS > 0

        return (eHAL_STATUS_INTERRUPT_PRESENT);
    }

#if HAL_INT_MAX_ITERATIONS > 0
    // we were called but no interrupts were pending. if this happens
    // too many times in a row in a test environment, we have a
    // problem.  in that case, dump what we have
    if (++counter > HAL_INT_MAX_ITERATIONS)
    {
        counter = 0;

        HALLOGE( halLog(pMac, LOGE,
               FL("%s: Too many iterations, clear the SIF Int Status: SIF Status: %08x Cached Mask: %08x"),
               __FUNCTION__, mcuMacHostIntStatus, mcuMacHostIntMask));

        halIntClearStatus(hHalHandle, eHAL_INT_SIF_ASIC);   

    }
#endif // HAL_INT_MAX_ITERATIONS > 0

    return (eHAL_STATUS_NO_INTERRUPTS);
}

VOS_STATUS
halIntHandler(v_PVOID_t pVosGCtx)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)vos_get_context(VOS_MODULE_ID_HAL, pVosGCtx);
    tHalHandle hHalHandle = (tHalHandle) pMac;
    eHalStatus status = eHAL_STATUS_SUCCESS;


    if (IS_PWRSAVE_STATE_IN_BMPS) { 
        status = halPS_SetHostBusy(pMac, HAL_PS_BUSY_INTR_CONTEXT); 
    }

   if(eHAL_STATUS_SUCCESS != status) {
   	VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s halPS_SetHostBusy  failed", __func__);
       return VOS_STATUS_E_FAILURE;
    }

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s"), __FUNCTION__));
#endif // HAL_INT_DEBUG
    while(eHAL_STATUS_INTERRUPT_PRESENT == halIntCheck(hHalHandle))
    {
        //Clear the SIF ASIC interrupt.
        status = halIntClearStatus(hHalHandle, eHAL_INT_SIF_ASIC);    

	    if(eHAL_STATUS_SUCCESS != status)
	    {
            break;
	    }

        // the interrupt register cache should already be populated in halIntCheck
        // Now serve all pending ASIC interrupts.
        halIntServiceRegister(hHalHandle, eHAL_INT_MCU_HOST_INT_REGISTER);
    }
    
    if(IS_HOST_BUSY_INTR_CNTX) {
        halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_INTR_CONTEXT); 
    }

    return VOS_STATUS_SUCCESS;
}


eHalStatus
halIntEnrollHandler(eHalIntSources intSource, pHalIntHandler pHandler)
{
    halIntInfo[intSource].pHandler = pHandler;
    return (eHAL_STATUS_SUCCESS);
}

eHalStatus
halIntReset(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntRegisters eReg;
    tANI_U32 hwReg;

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s"), __FUNCTION__));
#endif // HAL_INT_DEBUG

    // when the chip is reset, all enable registers are presumed
    // cleared, so our enable cache no longer matches the chip
    for (eReg = eHAL_INT_SIF_REGISTER; eReg < eHAL_INT_MAX_REGISTER; eReg++)
    {
        // get the enable register address for this interrupt
        hwReg = halIntRegisterInfo[eReg].enableRegister;

        //does this interrupt have an enable register?
        if (HAL_INT_INVALID_HW_REGISTER != hwReg)
        {
            // yes, so the enable register no longer matches the cache
            pMac->hal.intEnableCache[eReg].valid = 0;
        }
    }
    halIntFlushCache(hHalHandle);
    //reset all the interrupt status counters
    palZeroMemory(pMac->hHdd, &pMac->hal.halIntErrStats, sizeof(tHalIntErrStat));
    return (eHAL_STATUS_SUCCESS);
}


eHalStatus
halIntRegServiceEnable(tHalHandle hHalHandle, eHalIntSources intRegService[], eAniBoolean bEnable, tANI_U32 *count)
{
    tANI_U16 i;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    /** Enable each Interrupt Sources under the top level Interrupt.*/
    if (bEnable == eANI_BOOLEAN_TRUE) {
        for (i=0; intRegService[i]!=eHAL_INT_MAX_SOURCE; i++) {
            status = halIntEnable(hHalHandle, intRegService[i]);
            if (status != eHAL_STATUS_SUCCESS) {
                break;
            }
        }
    } else {
        for (i=0; intRegService[i]!=eHAL_INT_MAX_SOURCE; i++) {
            status = halIntDisable(hHalHandle, intRegService[i]);
            if (status != eHAL_STATUS_SUCCESS) {
                break;
            }
        }
    }
    if(count)
        *count = i;
    return status;
}


eHalStatus
halIntDefaultRegServicesEnable(tHalHandle hHalHandle, eAniBoolean bEnable)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 index, count;
    HALLOG1( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));
    HALLOG1( halLog(pMac, LOG1, FL("%s All Default Interrupt Services.\n"), (bEnable == eANI_BOOLEAN_TRUE) ? "Enabling":"Disabling"));



    /** enable/disable interrupt service for all register service groups */
    for(index= (eHAL_INT_MAX_REGISTER-1); index >=1 ; index--){
    
        status = halIntRegServiceEnable(hHalHandle, 
                   intRegService[index], bEnable, &count);

        HALLOG1( halLog(pMac, LOG1, FL("%s %s %d %s Interrupt Services.  Status: %d\n"), (bEnable == eANI_BOOLEAN_TRUE) ? "Enabled":"Disabled",
            (status == eHAL_STATUS_SUCCESS)?"all":"", count, intRegServiceName[index], status)
        );

        if (status != eHAL_STATUS_SUCCESS) {
            HALLOG1( halLog(pMac, LOG1, FL(".....Abort at %s!!!!\n"), intRegServiceName[index]));

            return status;
        }

    }
    return status;
}

eHalStatus
halIntChipEnable(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalIntRegisters eReg;
    tANI_U32 enableRegister;
    tANI_U32 enableValue;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, hHalHandle);
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    WLANBAL_HalRegType halRegType;


#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s"), __FUNCTION__));
#endif // HAL_INT_DEBUG
    pMac->hal.intEnabled = 1;

    // make sure all interrupt enables are written back.  we start
    // with the leaves and work up to the top to make sure everything
    // is how it should be before we enable the bus interrupt
    eReg = eHAL_INT_MAX_REGISTER;
    while (eReg > 0)
    {
        eReg--;
        if (!pMac->hal.intEnableCache[eReg].valid)
        {
            // hardware will now match cache
            pMac->hal.intEnableCache[eReg].valid = 1;

            // write back this enable register
            enableRegister = halIntRegisterInfo[eReg].enableRegister;
            enableValue = pMac->hal.intEnableCache[eReg].value;

#ifdef HAL_INT_DEBUG
            HALLOGW( halLog(pMac, LOGW, FL("register %d enable %08x"), eReg, enableValue));
#endif // HAL_INT_DEBUG

            if (HAL_INT_INVALID_HW_REGISTER != enableRegister)
            {
                halIntWriteRegister(pMac,
                                    enableRegister,
                                    enableValue);
            }

        }
    }
        
    halRegType.asicInterruptCB = halIntHandler;
    halRegType.halUsrData = pMac;
    halRegType.fatalErrorCB = halFatalErrorHandler;

    status = WLANBAL_RegHalCBFunctions(pVosGCtx, &halRegType);
    if(!VOS_IS_STATUS_SUCCESS(status))
    {
        //FIXME : what do we need to do here.
        HALLOGP( halLog(pMac, LOGP, FL("failed to register HAL callback to BAL\n")));
        return eHAL_STATUS_FAILURE;
    }
#ifdef WLAN_PERF
    status = WLANBAL_EnableASICInterruptEx(pVosGCtx, QWLAN_SIF_INT_ASIC_INTERRUPT_MASK);
#else
    status = WLANBAL_EnableASICInterrupt(pVosGCtx);
#endif
    if(!VOS_IS_STATUS_SUCCESS(status))
    {
        //FIXME : what do we need to do here.
        HALLOGP( halLog(pMac, LOGP, FL("failed to enable ASIC interrupt from BAL\n")));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}



eHalStatus
halIntChipDisable(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);


    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, hHalHandle);
    VOS_STATUS status = VOS_STATUS_SUCCESS;

    // interrupts are disabled
    pMac->hal.intEnabled = 0;

#ifdef WLAN_PERF
    status = WLANBAL_DisableASICInterruptEx(pVosGCtx, QWLAN_SIF_INT_ASIC_INTERRUPT_MASK);
#else
    status = WLANBAL_DisableASICInterrupt(pVosGCtx);
#endif
    if(!VOS_IS_STATUS_SUCCESS(status))
    {
        //FIXME : what do we need to do here.
        return eHAL_STATUS_FAILURE;
    }
    return eHAL_STATUS_SUCCESS;
}

eHalStatus
halIntEnable(tHalHandle hHalHandle, eHalIntSources eHalIntSource)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    eHalIntRegisters eReg;
    tANI_U32 hwReg;
    tANI_U32 mask;

    eReg = halIntInfo[eHalIntSource].eRegister;

    hwReg = halIntRegisterInfo[eReg].enableRegister;

    // get the enable mask for this interrupt
    mask = halIntInfo[eHalIntSource].mask;

    // is the interrupt already enabled?
    if (pMac->hal.intEnableCache[eReg].value & mask)
    {
        return (eHAL_STATUS_SUCCESS);
    }
    // update the cached version of the register enable mask
    pMac->hal.intEnableCache[eReg].value |= mask;

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("(%d): mask is now %08x"),
           eHalIntSource,
           pMac->hal.intEnableCache[eReg].value));
#endif // HAL_INT_DEBUG

    //does this interrupt register have an enable register?
    if (HAL_INT_INVALID_HW_REGISTER == hwReg)
    {
        // no, so the soft enable is the only enable and is valid
        pMac->hal.intEnableCache[eReg].valid = 1;
    }
    else
  
    // are interrupts currently enabled?
    if (pMac->hal.intEnabled)
    {
        // yes, so we must update the chip now
        pMac->hal.intEnableCache[eReg].valid = 1;
        retStatus = halIntWriteRegister(pMac,
                                        hwReg,
                                        pMac->hal.intEnableCache[eReg].value);
    }
    else
    {
        // no, so we must update the chip later
        // flag the enable mask as invalid so it will be written out
        // when interrupts are enabled
        pMac->hal.intEnableCache[eReg].valid = 0;
    }

    return (retStatus);
}

eHalStatus
halIntDisable(tHalHandle hHalHandle, eHalIntSources eHalIntSource)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    eHalIntRegisters eReg;
    tANI_U32 hwReg;
    tANI_U32 mask;

    eReg = halIntInfo[eHalIntSource].eRegister;

    hwReg = halIntRegisterInfo[eReg].enableRegister;

    // get the disable mask for this interrupt
    mask = halIntInfo[eHalIntSource].mask;
    // is the interrupt already disabled?
    if (!(pMac->hal.intEnableCache[eReg].value & mask))
    {
        return (eHAL_STATUS_SUCCESS);
    }
    // update the cached version of the register enable mask
    pMac->hal.intEnableCache[eReg].value &= ~mask;

#ifdef HAL_INT_DEBUG
    HALLOGW( halLog(pMac, LOGW, FL("%s(%d): mask is now %08x"),
           __FUNCTION__, eHalIntSource,
           pMac->hal.intEnableCache[eReg].value));
#endif // HAL_INT_DEBUG

    //does this interrupt register have an enable register?
    if (HAL_INT_INVALID_HW_REGISTER == hwReg)
    {
        // no, so the soft enable is the only enable and is valid
        pMac->hal.intEnableCache[eReg].valid = 1;
    }
    else

    // are interrupts currently enabled?
    if (pMac->hal.intEnabled)
    {
        // yes, so we must update the chip now
        pMac->hal.intEnableCache[eReg].valid = 1;
        retStatus = halIntWriteRegister(pMac,
                                        hwReg,
                                        pMac->hal.intEnableCache[eReg].value);
    }
    else
    {
        // no, so we must update the chip later
        // flag the enable mask as invalid so it will be written out
        // when interrupts are enabled
        pMac->hal.intEnableCache[eReg].valid = 0;
    }

    return (retStatus);
}

