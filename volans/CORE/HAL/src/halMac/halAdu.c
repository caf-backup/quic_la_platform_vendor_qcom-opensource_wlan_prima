/**
 *
 *  @file:         halAdu.c
 *
 *  @brief:        Provides all the MAC APIs to the ADU Hardware Block.
 *
 *  @author:        Madhava Reddy S
 *
 *  Copyright (C) 2002 - 2007, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/09/2007  File created.
 * 12/04/2007  MIMO PS tables changes made.
 */
#include "palTypes.h"
#include "halTypes.h"
#include "aniGlobal.h"
#ifdef WLAN_HAL_VOLANS
#include "volansDefs.h"
#else
#include "libraDefs.h"
#endif
#include "halAdu.h"
#include "sirMacProtDef.h"
#include "halDebug.h"
#include "halRegBckup.h"

#ifdef ANI_SUPPORT_SMPS
#ifdef SPICA
static tHalRegCfg halAdu_mimoPScfg2to1[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x1e},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x1233},
    {BTCF_BTCF_CONFIG_REG, 0x370d},
    {AGC_N_ACTIVE_REG, 0x1},
    {AGC_N_LISTEN_REG, 0x1},
    {AGC_N_CAPTURE_REG, 0x1},
    {AGC_N_MEASURE_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x82},
    {AGC_TH_D0_11N_REG, 0x64},
    {AGC_TH_MAXCORN_REG, 0x8c},
    {AGC_TH_D0_B_REG, 0xa0},
    {AGC_TH_SIGNAL_LOW_REG, 0x19},
    {AGC_TH_D0_SLR_REG, 0x32},
    {AGC_TH_D0_SLRTFEST_REG, 0x28},
    {AGC_TH_D0_SLRWAITB_REG, 0x3c},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg1to2[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x20},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x2344},
    {BTCF_BTCF_CONFIG_REG, 0x3708},
    {AGC_N_ACTIVE_REG, 0x2},
    {AGC_N_LISTEN_REG, 0x2},
    {AGC_N_CAPTURE_REG, 0x2},
    {AGC_N_MEASURE_REG, 0x2},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x6e},
    {AGC_TH_D0_11N_REG, 0x5a},
    {AGC_TH_MAXCORN_REG, 0x78},
    {AGC_TH_D0_B_REG, 0x8c},
    {AGC_TH_SIGNAL_LOW_REG, 0x32},
    {AGC_TH_D0_SLR_REG, 0x28},
    {AGC_TH_D0_SLRTFEST_REG, 0x1e},
    {AGC_TH_D0_SLRWAITB_REG, 0x32},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

#else

static tHalRegCfg halAdu_mimoPScfg2to1[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x1e},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x1233},
    {BTCF_BTCF_CONFIG_REG, 0x370d},
    {AGC_N_ACTIVE_REG, 0x1},
    {AGC_N_LISTEN_REG, 0x1},
    {AGC_N_CAPTURE_REG, 0x1},
    {AGC_N_MEASURE_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x82},
    {AGC_TH_D0_11N_REG, 0x64},
    {AGC_TH_MAXCORN_REG, 0x8c},
    {AGC_TH_D0_B_REG, 0xa0},
    {AGC_TH_SIGNAL_LOW_REG, 0x19},
    {AGC_TH_D0_SLR_REG, 0x32},
    {AGC_TH_D0_SLRTFEST_REG, 0x28},
    {AGC_TH_D0_SLRWAITB_REG, 0x3c},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg3to1[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x1e},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x1233},
    {BTCF_BTCF_CONFIG_REG, 0x370d},
    {AGC_N_ACTIVE_REG, 0x1},
    {AGC_N_LISTEN_REG, 0x1},
    {AGC_N_CAPTURE_REG, 0x1},
    {AGC_N_MEASURE_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x82},
    {AGC_TH_D0_11N_REG, 0x64},
    {AGC_TH_MAXCORN_REG, 0x8c},
    {AGC_TH_D0_B_REG, 0xa0},
    {AGC_TH_D0_B_TF_EST_REG, 0x3c},
    {AGC_TH_SIGNAL_LOW_REG, 0x19},
    {AGC_TH_SIGNAL_HIGH_REG, 0xb4},
    {AGC_TH_D0_SLR_REG, 0x32},
    {AGC_TH_D0_SLRTFEST_REG, 0x28},
    {AGC_TH_D0_SLRWAITB_REG, 0x3c},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg4to1[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x1e},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x1233},
    {BTCF_BTCF_CONFIG_REG, 0x370d},
    {AGC_N_ACTIVE_REG, 0x1},
    {AGC_N_LISTEN_REG, 0x1},
    {AGC_N_CAPTURE_REG, 0x1},
    {AGC_N_MEASURE_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x82},
    {AGC_TH_D0_11N_REG, 0x64},
    {AGC_TH_MAXCORN_REG, 0x8c},
    {AGC_TH_D0_B_REG, 0xa0},
    {AGC_TH_D0_B_TF_EST_REG, 0x3c},
    {AGC_TH_SIGNAL_LOW_REG, 0x19},
    {AGC_TH_SIGNAL_HIGH_REG, 0xb4},
    {AGC_TH_D0_SLR_REG, 0x32},
    {AGC_TH_D0_SLRTFEST_REG, 0x28},
    {AGC_TH_D0_SLRWAITB_REG, 0x3c},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg1to2[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x20},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x2344},
    {BTCF_BTCF_CONFIG_REG, 0x3708},
    {AGC_N_ACTIVE_REG, 0x2},
    {AGC_N_LISTEN_REG, 0x2},
    {AGC_N_CAPTURE_REG, 0x2},
    {AGC_N_MEASURE_REG, 0x2},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x6e},
    {AGC_TH_D0_11N_REG, 0x5a},
    {AGC_TH_MAXCORN_REG, 0x78},
    {AGC_TH_D0_B_REG, 0x8c},
    {AGC_TH_SIGNAL_LOW_REG, 0x32},
    {AGC_TH_D0_SLR_REG, 0x28},
    {AGC_TH_D0_SLRTFEST_REG, 0x1e},
    {AGC_TH_D0_SLRWAITB_REG, 0x32},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg1to3[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x20},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x2344},
    {BTCF_BTCF_CONFIG_REG, 0x3712},
    {AGC_N_ACTIVE_REG, 0x3},
    {AGC_N_LISTEN_REG, 0x3},
    {AGC_N_CAPTURE_REG, 0x3},
    {AGC_N_MEASURE_REG, 0x3},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x64},
    {AGC_TH_D0_11N_REG, 0x46},
    {AGC_TH_MAXCORN_REG, 0x78},
    {AGC_TH_D0_B_REG, 0x6e},
    {AGC_TH_D0_B_TF_EST_REG, 0x32},
    {AGC_TH_SIGNAL_LOW_REG, 0x36},
    {AGC_TH_SIGNAL_HIGH_REG, 0x6e},
    {AGC_TH_D0_SLR_REG, 0x1e},
    {AGC_TH_D0_SLRTFEST_REG, 0x14},
    {AGC_TH_D0_SLRWAITB_REG, 0x28},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

static tHalRegCfg halAdu_mimoPScfg1to4[] = {
    {AGC_AGC_RESET_REG, 0x1},
    {RACTL_DAGC_TGTBO_REG, 0x20},
    {QMRC_LMAP_OPSCALE_STBC_REG, 0x2344},
    {BTCF_BTCF_CONFIG_REG, 0x371d},
    {AGC_N_ACTIVE_REG, 0x4},
    {AGC_N_LISTEN_REG, 0x4},
    {AGC_N_CAPTURE_REG, 0x4},
    {AGC_N_MEASURE_REG, 0x4},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xf8},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x77},
    {AGC_AGC_RESET_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x1},
    {CAL_CLR_DCOFFSET_REG, 0x1},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0xe0},
    {RXCLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_AGC_REG, 0x1},
    {AGC_TH_D0_A_REG, 0x64},
    {AGC_TH_D0_11N_REG, 0x46},
    {AGC_TH_MAXCORN_REG, 0x78},
    {AGC_TH_D0_B_REG, 0x5a},
    {AGC_TH_D0_B_TF_EST_REG, 0x32},
    {AGC_TH_SIGNAL_LOW_REG, 0x48},
    {AGC_TH_SIGNAL_HIGH_REG, 0x6e},
    {AGC_TH_D0_SLR_REG, 0x19},
    {AGC_TH_D0_SLRTFEST_REG, 0xa},
    {AGC_TH_D0_SLRWAITB_REG, 0x1e},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x2},
    {RXACLKCTRL_APB_BLOCK_DYN_CLKG_DISABLE_REG, 0x0},
    {AGC_AGC_RESET_REG, 0x0}
};

#endif

static tHalMimoPsCfg halAdu_mimoPScfg[eSIR_HT_MIMO_PS_MODE_MAX];
#endif

/**
 * \brief Initialize the ADU Error Interrupts
 *
 * \fn halAdu_initErrorInterrupts
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return eHalStatus ADU module Error interrupt initialization status
 */
eHalStatus halAdu_initErrorInterrupts(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    /** Enable the ADU interrupts */
    value = QWLAN_ADU_INTERRUPT_ENABLE_UMA_WRONG_WQ_ERROR_ENABLE_MASK|
            QWLAN_ADU_INTERRUPT_ENABLE_TIMEOUT_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_WRONG_WQ_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_INCORRECT_LINK_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_BAD_BD_TYPE_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_GAM_WRITE_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_DELIMITER_LENGTH_NOT_AVAIL_ERROR_ENABLE_MASK |
            QWLAN_ADU_INTERRUPT_ENABLE_GAM_READ_ERROR_ENABLE_MASK|
            QWLAN_ADU_INTERRUPT_ENABLE_GBI_PUSH_ERROR_ENABLE_MASK|
            QWLAN_ADU_INTERRUPT_ENABLE_MPDU_HEADER_NOT_IN_BD_ERROR_ENABLE_MASK|
            QWLAN_ADU_INTERRUPT_ENABLE_GBI_POP_ZERO_ERROR_ENABLE_MASK|
            QWLAN_ADU_INTERRUPT_ENABLE_GBI_POP_ERROR_ENABLE_MASK;

    HALLOGW( halLog( pMac, LOGW, FL("halAdu_initErrorInterrupts: value %dl\n"),  value ));

    halWriteRegister(pMac,  QWLAN_ADU_INTERRUPT_ENABLE_REG,  value );

    return eHAL_STATUS_SUCCESS;
}

// Init UMA Desc table base register.
static eHalStatus __halUma_InitStaDescBase(tpAniSirGlobal pMac)
{
    /** Zero out the UMA STA descriptor */
    palZeroDeviceMemory(pMac->hHdd, 
        pMac->hal.memMap.aduUmaStaDesc_offset, 
        pMac->hal.memMap.aduUmaStaDesc_size);

    halWriteRegister(pMac, QWLAN_ADU_UMA_DESP_TABLE_ADDR_REG,
        pMac->hal.memMap.aduUmaStaDesc_offset);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Initialize the ADU control
 *
 * \fn __halAdu_initControl
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return eHalStatus ADU control initialization status
 */
static eHalStatus __halAdu_initControl(tpAniSirGlobal pMac)
{
    tANI_U32 value;

    value = QWLAN_ADU_CONTROL_ADU_ENABLE_MASK |
            QWLAN_ADU_CONTROL_PUSH_WQ_SELECTION_MODE_MASK |
#ifndef WLAN_HAL_VOLANS //FIXME_VOLANS            
            QWLAN_ADU_CONTROL_REG_REINIT_TO_BOTH_AHB_EN_MASK |
#endif            
            (BMUWQ_DXE_RX << QWLAN_ADU_CONTROL_NEXT_WQ_OFFSET) |
            ((BMUWQ_SINK << QWLAN_ADU_CONTROL_WOW_WQ_OFFSET) & QWLAN_ADU_CONTROL_WOW_WQ_MASK);

    halWriteRegister(pMac,  QWLAN_ADU_CONTROL_REG,  value );

    halReadRegister(pMac, QWLAN_ADU_CONTROL_REG, &value) ;

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Update the ADU Control
 *
 * \fn halAdu_UpdateControl
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param mask Mask value to update
 *
 * \return eHalStatus ADU control update status
 */
eHalStatus halAdu_UpdateControl(tpAniSirGlobal pMac, tANI_U32 mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_ADU_CONTROL_REG, &value) ;

    /** Set the mask value */
    value |= mask;

    halWriteRegister(pMac, QWLAN_ADU_CONTROL_REG, value);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Update the ADU push Wq Control
 *
 * \fn halAdu_UpdateControlPushWq
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param workQueue WorkQueue to push
 *
 * \param mask Mask value to update the control bits
 *
 * \return eHalStatus ADU push Wq update status
 */
eHalStatus halAdu_UpdateControlPushWq(tpAniSirGlobal pMac, tANI_U32 workQueue, tANI_U32 mask)
{
    tANI_U32 value;

    halReadRegister(pMac, QWLAN_ADU_CONTROL_REG, &value) ;

    /** Set the mask value */
    value |= mask;

    if (!(value & QWLAN_ADU_CONTROL_PUSH_WQ_SELECTION_MODE_MASK)) {
        HALLOGW( halLog(pMac, LOGW, FL("halAdu_UpdateControlPushWq: Push Wq Control need to be set \n")));
        return eHAL_STATUS_FAILURE;
    }

    /** Clear the existing work queue mask */
    value &= ~QWLAN_ADU_CONTROL_NEXT_WQ_MASK;

    /** Set the Work Queue number */
    value |= (workQueue << QWLAN_ADU_CONTROL_NEXT_WQ_OFFSET);

    halWriteRegister(pMac, QWLAN_ADU_CONTROL_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/*
 * Function to enable the frame translation support from the UMA Upper MAC Accelerator 
 * module.  Which 1. Enables 802.3->802.11 conversion in Tx path and 802.11->802.3
 * conversion in Rx path. 2. Enables LLC insertion in Tx path and LLC removal in Rx path.
 */
eHalStatus halAdu_enableFrameTranslation(tpAniSirGlobal pMac) 
{
    tANI_U32 value = 0;

    halReadRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, &value);

    value |= (QWLAN_ADU_UMA_CONFIG_UMA_TX_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_LLC_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_LLC_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_CHECKSUM_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_CHECKSUM_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_FT_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_PUSH_WQ_SEL_MASK|
            (BMUWQ_DPU_TX<<QWLAN_ADU_UMA_CONFIG_UMA_TX_PUSH_WQ_OFFSET)|
            QWLAN_ADU_UMA_CONFIG_UMA_RX_FT_EN_MASK);

    halWriteRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, value);

    
    halWriteRegister(pMac, QWLAN_ADU_UMA_CTRL2_REG,
                     (
#ifdef WLAN_HAL_VOLANS
					 (1 << QWLAN_ADU_UMA_CTRL2_RX_PRIORITY_ROUTING_WQ_EN_OFFSET )|
                      (BMUWQ_DXE_RX_HI << QWLAN_ADU_UMA_CTRL2_RX_PRIORITY_ROUTING_WQ_OFFSET)|
#endif
                      (BMUWQ_SINK)));

    return eHAL_STATUS_SUCCESS;
}



/*
 * Function to disable the frame translation support from the UMA 
 * Upper MAC Accelerator module.  Which: 
 * 1. disables 802.3->802.11 conversion in Tx path and 802.11->802.3 
 *    conversion in Rx path. 
 * 2. disables LLC insertion in Tx path and LLC removal in Rx path.
 */
eHalStatus halAdu_disableFrameTranslation(tpAniSirGlobal pMac) 
{
    tANI_U32 value = 0;

    halReadRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, &value);

    value &= ~(QWLAN_ADU_UMA_CONFIG_UMA_TX_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_LLC_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_LLC_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_CHECKSUM_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_CHECKSUM_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_TX_FT_EN_MASK |
            QWLAN_ADU_UMA_CONFIG_UMA_RX_FT_EN_MASK);

    halWriteRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, value);

    return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Initialize the ADU module
 *
 * \fn halAdu_Start
 *
 * \param hHal The global tHalHandle object
 *
 * \param arg Pointer to arguments
 *
 * \return eHalStatus ADU module initialization status
 */
eHalStatus halAdu_Start(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal     pMac = (tpAniSirGlobal)hHal;

    /** Initialize the control register */
    if (__halAdu_initControl(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    /* Frame Translation support enable/disable */
    if(halGetFrameTranslation(pMac)) {
        HALLOG1( halLog(pMac, LOG1, FL("FT ON\n")));
        if (halAdu_enableFrameTranslation(pMac) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    } else {
        HALLOG1( halLog(pMac, LOG1, FL("FT OFF\n")));
        if(halAdu_disableFrameTranslation(pMac) != eHAL_STATUS_SUCCESS)
            return eHAL_STATUS_FAILURE;
    }

    /** Enable the error interrupts */
    if (halAdu_initErrorInterrupts(pMac) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    if (__halUma_InitStaDescBase(pMac) != eHAL_STATUS_SUCCESS) 
    {
        HALLOGE( halLog(pMac, LOGE, FL("ERROR setting UMA BASE\n")));
    }

#ifdef ANI_SUPPORT_SMPS
#ifdef SPICA
    halAdu_mimoPScfg[0].size         = sizeof(halAdu_mimoPScfg2to1);
    halAdu_mimoPScfg[0].halRegCfgPtr = halAdu_mimoPScfg2to1;
    halAdu_mimoPScfg[1].size         = sizeof(halAdu_mimoPScfg1to2);
    halAdu_mimoPScfg[1].halRegCfgPtr = halAdu_mimoPScfg1to2;
#else
    halAdu_mimoPScfg[0].size         = sizeof(halAdu_mimoPScfg2to1);
    halAdu_mimoPScfg[0].halRegCfgPtr = halAdu_mimoPScfg2to1;
    halAdu_mimoPScfg[1].size         = sizeof(halAdu_mimoPScfg3to1);
    halAdu_mimoPScfg[1].halRegCfgPtr = halAdu_mimoPScfg3to1;
    halAdu_mimoPScfg[2].size         = sizeof(halAdu_mimoPScfg4to1);
    halAdu_mimoPScfg[2].halRegCfgPtr = halAdu_mimoPScfg4to1;
    halAdu_mimoPScfg[3].size         = sizeof(halAdu_mimoPScfg1to2);
    halAdu_mimoPScfg[3].halRegCfgPtr = halAdu_mimoPScfg1to2;
    halAdu_mimoPScfg[4].size         = sizeof(halAdu_mimoPScfg1to3);
    halAdu_mimoPScfg[4].halRegCfgPtr = halAdu_mimoPScfg1to3;
    halAdu_mimoPScfg[5].size         = sizeof(halAdu_mimoPScfg1to4);
    halAdu_mimoPScfg[5].halRegCfgPtr = halAdu_mimoPScfg1to4;
#endif
#endif

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Save the Sta Descriptor Configuration
 *
 * \fn halAdu_SaveStaConfig
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tpeStaDesc Pointer to tpe sta desc configuration
 *
 * \param staIdx Station index
 *
 * \return eHalStatus save sta desc config status
 */
static eHalStatus halAdu_SaveUmaStaConfig(tpAniSirGlobal pMac, 
        tpAduUmaStaDesc pAduUmaStaDesc, tANI_U16 umaIdx)
{
    eHalStatus status = eHAL_STATUS_INVALID_STAIDX;

    if (umaIdx < HAL_NUM_UMA_DESC_ENTRIES) 
    {
        palCopyMemory(pMac->hHdd, 
            (tANI_U8 *) &(pMac->hal.halMac.aduUmaDesc[umaIdx]),
            (tANI_U8 *)pAduUmaStaDesc, ADU_UMA_STA_DESC_ENTRY_SIZE);
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/** 
 * \fn halAdu_WriteUmaDefaultMacAddr
 *
 * \brief Set the default mac address in UMA, which will be used once the mac address
 * look up in the UMA table is failed.
 *
 * \param macAddrLo Lower 4 bytes of the mac address
 *
 * \param macAddrHi Upper 2 bytes of the mac address
 * 
 * \return eHalStatus success or failure
 */
eHalStatus halAdu_WriteUmaDefaultMacAddr(tpAniSirGlobal pMac, 
        tANI_U32 macAddrLo, tANI_U32 macAddrHi, tANI_U8 umaIdx)
{
    
    halWriteRegister(pMac, QWLAN_ADU_UMA_TX_DEFAULT_WMACADDR_U_REG, macAddrHi);

    halWriteRegister(pMac, QWLAN_ADU_UMA_TX_DEFAULT_WMACADDR_L_REG, macAddrLo);

    // Update the umaIdx for the default UMA config.
    halAdu_UpdateUMADefaultstaIdx(pMac, umaIdx);

    return eHAL_STATUS_SUCCESS;
}

/**
 * \fn halAdu_AddToUmaSearchTable
 *
 * \brief Set the default mac address in UMA, which will be used once the mac address
 * look up in the UMA table is failed.
 *
 * \param macAddrLo Lower 4 bytes of the mac address
 *
 * \param macAddrHi Upper 2 bytes of the mac address
 * 
 * \param umaIdx : The UMA config entry index. 
 * \return eHalStatus success or failure
 */
eHalStatus halAdu_AddToUmaSearchTable(tpAniSirGlobal pMac, 
        tANI_U32 macAddrLo, tANI_U32 macAddrHi, tANI_U8 umaIdx)
{
    tANI_U32 val;
    
    halWriteRegister(pMac, QWLAN_ADU_UMA_INDEX_TABLE_WDATA_L_REG, macAddrLo);

    val = macAddrHi |
        (umaIdx << QWLAN_ADU_UMA_INDEX_TABLE_WDATA_U_UMA_SRCHTABLE_INDEX_OFFSET);
    
    halWriteRegister(pMac, QWLAN_ADU_UMA_INDEX_TABLE_WDATA_U_REG, val);

    val = (1 << QWLAN_ADU_UMA_INDEX_TABLE_ADDR_CTRL_UMA_SRCHTABLE_RW_OFFSET) |
        umaIdx; //same umaIdx is used for UMA search table indexing also
        
    halWriteRegister(pMac, QWLAN_ADU_UMA_INDEX_TABLE_ADDR_CTRL_REG, val);

    return eHAL_STATUS_SUCCESS;
}

/** Set WEP bit in frame ctl */
// This code has not been tested with security enabled.
eHalStatus halAdu_UpdateUmaDescForPrivacy(tpAniSirGlobal pMac, 
        tANI_U16 staIdx)
{
    tpAduUmaStaDesc pAduUmaStaDesc;
    eHalStatus status;
    tANI_U8 umaIdx;


    // Get the UMA index associated with this staIdx.
    if ((status=halTable_GetStaUMAIdx(pMac, (tANI_U8)staIdx, &umaIdx))
            != eHAL_STATUS_SUCCESS) 
    {
        HALLOGE( halLog( pMac, LOGE, 
                FL("Unable to get umaIdx for sta%d\n"), staIdx)); 
        return status; 
        
    }
    // Get the UMA descriptor table.
    if (umaIdx == HAL_INVALID_KEYID_INDEX) 
    {
        HALLOGE( halLog( pMac, LOGE, FL("UMA Index not available\n"))); 
        return eHAL_STATUS_FAILURE; 
    }

    pAduUmaStaDesc = &(pMac->hal.halMac.aduUmaDesc[umaIdx]);
    pAduUmaStaDesc->wep = 1;

    status = halAdu_SetUmaStaDesc(pMac, umaIdx, pAduUmaStaDesc);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
        return status; 
    }

    return status;
}

/**
 * \fn halAdu_SetUmaStaDesc
 *
 * \brief Configure the UMA Station Descriptor
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param uma descriptor index. 
 *
 * \param pAduUmaStaDesc Pointer to tpe station descriptor information
 *
 * \return eHalStatus Station descriptor configuration status
 */
eHalStatus halAdu_SetUmaStaDesc(tpAniSirGlobal pMac, 
        tANI_U16 umaIdx, tpAduUmaStaDesc pAduUmaStaDesc)
{
    tANI_U32    address;

    address = pMac->hal.memMap.aduUmaStaDesc_offset + (umaIdx * ADU_UMA_STA_DESC_ENTRY_SIZE);

    /** Save the STA config */
    if (halAdu_SaveUmaStaConfig(pMac, pAduUmaStaDesc, umaIdx) != eHAL_STATUS_SUCCESS)
        return eHAL_STATUS_FAILURE;

    HALLOGW( halLog( pMac, LOGW, FL("HAL ADU UMA STA Desc address 0x%x\n"),  address ));
    HALLOGW( halLog( pMac, LOGW, FL("HAL ADU UMA STA Desc size 0x%x\n"),  ADU_UMA_STA_DESC_ENTRY_SIZE ));

    halWriteDeviceMemory(pMac, address, (tANI_U8 *)pAduUmaStaDesc,
                        ADU_UMA_STA_DESC_ENTRY_SIZE);
    return eHAL_STATUS_SUCCESS;
}


/**
 * \brief Update ADU register reconfiguration table with the given commands
 *
 * \fn halAdu_UpdateRegRecfgTable
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param regAddr register address to be reinitialized
 *
 * \param regVal register value to reinitialize the register
 *
 * \param waitCycles wait time for the command completion
 *
 * \return eHalStatus ADU Reg Reconfig table update status
 */

#ifdef SPICA
eHalStatus halAdu_UpdateRegRecfgTable(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regVal, tANI_U32 waitCycles )
{
    tANI_U32   restoreCurPtr = pMac->hal.memMap.aduRegRecfgTbl_curPtr;
    tANI_U32   value;
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

    HALLOGW( halLog(pMac, LOGW, FL(" addr 0x%x value 0x%x wait %d\n"),
           regAddr, regVal, waitCycles);

    if (regAddr)
    {
        value = HAL_ADU_REG_RECFG_INIT_CMD | (regAddr & HAL_ADU_REG_RECFG_ADDR_MASK);
        /* Write register address first */
        halWriteDeviceMemory(pMac, pMac->hal.memMap.aduRegRecfgTbl_curPtr,
                                      (tANI_U8 *)&value, sizeof(tANI_U32));

        pMac->hal.memMap.aduRegRecfgTbl_curPtr += sizeof(tANI_U32);

        /* Write the value the above register is to be re-initialized with */
        halWriteDeviceMemory(pMac, pMac->hal.memMap.aduRegRecfgTbl_curPtr,
                                      (tANI_U8 *)&regVal, sizeof(tANI_U32));

        pMac->hal.memMap.aduRegRecfgTbl_curPtr += sizeof(tANI_U32);
    }

    /* If there is non-zero wait time, add the wait command */
    if (waitCycles)
    {
        value = HAL_ADU_REG_RECFG_WAIT_CMD | (regAddr & HAL_ADU_REG_RECFG_WAIT_TIME_MASK);
         halWriteDeviceMemory(pMac, pMac->hal.memMap.aduRegRecfgTbl_curPtr,
                                      (tANI_U8 *)&value, sizeof(tANI_U32));
        pMac->hal.memMap.aduRegRecfgTbl_curPtr += sizeof(tANI_U32);
    }

    /* Now write the end of table command */
    value = HAL_ADU_REG_RECFG_TBL_END_CMD & HAL_ADU_REG_RECFG_TBL_END_CMD_MASK;
     halWriteDeviceMemory(pMac, pMac->hal.memMap.aduRegRecfgTbl_curPtr,
                                  (tANI_U8 *)&value, sizeof(tANI_U32));

    /* current pointer always points to the next write location,
       hence the current pointer not updated after the table end command */

    return eHAL_STATUS_SUCCESS;

failure:
    /* Restore ADU Reg Recfg table current pointer */
    pMac->hal.memMap.aduRegRecfgTbl_curPtr = restoreCurPtr;
    return retVal;
}
#else
eHalStatus halAdu_UpdateRegRecfgTable(tpAniSirGlobal pMac, tANI_U32 regAddr, tANI_U32 regVal, tANI_U32 waitCycles )
{
    return eHAL_STATUS_SUCCESS;
}
#endif


#ifdef ANI_SUPPORT_SMPS

/**
 * \brief Set Mimo PS Config
 *
 * \fn halAdu_SetMimoPScfg
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param mode Mimo PS mode
 *
 * \return eHalStatus return status
 */
eHalStatus halAdu_SetMimoPScfg(tpAniSirGlobal pMac, eSirMacHTMIMOPowerSaveMode mode)
{

#ifdef SPICA
    tANI_U32 clearModeOffset = mode+1;
#else
    tANI_U32 clearModeOffset = mode+3;
#endif
    tANI_U32 offset=pMac->hal.memMap.aduMimoPSprg_offset;
    tANI_U32 endCmd = 0xFFFFFFFF;

    halWriteRegister(pMac, QWLAN_ADU_MIMO_CLEAR_INIT_ADDR_REG,
                                   offset);
    halWriteDeviceMemory(pMac, offset,
              (tANI_U8 *)halAdu_mimoPScfg[mode].halRegCfgPtr, halAdu_mimoPScfg[mode].size);

    offset += halAdu_mimoPScfg[mode].size;

    halWriteDeviceMemory(pMac, offset,
              (tANI_U8 *)&endCmd, sizeof(tANI_U32));

    offset += sizeof(tANI_U32);

    halWriteRegister(pMac, QWLAN_ADU_MIMO_SET_INIT_ADDR_REG,
                                   offset);

    halWriteDeviceMemory(pMac, offset,
              (tANI_U8 *)halAdu_mimoPScfg[clearModeOffset].halRegCfgPtr, halAdu_mimoPScfg[clearModeOffset].size);

    offset += halAdu_mimoPScfg[clearModeOffset].size;

    halWriteDeviceMemory(pMac, offset,
              (tANI_U8 *)&endCmd, sizeof(tANI_U32));

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Switch Mimo PS mode
 *
 * \fn halAdu_switchMimoPSmode
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param mode Mimo PS mode
 *
 * \return eHalStatus return status
 */
eHalStatus halAdu_switchMimoPSmode(tpAniSirGlobal pMac, eSirMacHTMIMOPowerSaveMode mode)
{
    eHalStatus retVal = eHAL_STATUS_SUCCESS;

#ifdef FIXME_GEN5
    if (!pMac->hal.halMac.psInfo.mimoPSenabled)
    {
        HALLOGW( halLog( pMac, LOGW, FL("%s : mimo PS not enabled\n"),  __FUNCTION__ ));
        return eHAL_STATUS_FAILURE;
    }
#endif

    if (mode >= eSIR_HT_MIMO_PS_MODE_MAX)
    {
        HALLOGW( halLog( pMac, LOGW, FL("%s : Invalid mimo PS mode %d\n"),  __FUNCTION__, mode ));
        return eHAL_STATUS_FAILURE;
    }

#ifdef SPICA
    if (mode <= eSIR_HT_MIMO_PS_2SS_1SS)
#else
    if (mode <= eSIR_HT_MIMO_PS_4SS_1SS)
#endif
    {
        if ((retVal = halAdu_SetMimoPScfg(pMac, mode)) != eHAL_STATUS_SUCCESS)
            return retVal;

        if ((retVal = halAdu_UpdateControl(pMac, QWLAN_ADU_CONTROL_SW_MIMO_CLEAR_TRIGGER_MASK))
                                   != eHAL_STATUS_SUCCESS)
            return retVal;
    }
    else
    {
        if ((retVal = halAdu_UpdateControl(pMac, QWLAN_ADU_CONTROL_SW_MIMO_SET_TRIGGER_MASK))
                                   != eHAL_STATUS_SUCCESS)
            return retVal;
    }

    return eHAL_STATUS_SUCCESS;
}
#endif

eHalStatus halAdu_ErrIntHandler(tHalHandle hHalHandle, eHalIntSources intSource){
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 intRegMask;
    tANI_U32 intRegStatus;
    HALLOGE( tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle));

    /** Read Interrupt Status.*/
    status = halIntGetErrorStatus(hHalHandle, intSource, &intRegStatus, &intRegMask);
    if (status != eHAL_STATUS_SUCCESS) {
        return status;
    }

    intRegStatus &= intRegMask;   

    if(intRegStatus){
        /** Display Error Information.*/
        HALLOGE( halLog( pMac, LOGE, FL("ADU Error Interrupt Status %x, enable %x\n"),  intRegStatus, intRegMask ));
        return status;
    } 
    

    return (eHAL_STATUS_SUCCESS);
}

/* UMA Search Table */
/* DESCRIPTION:
 *      Routine to write UMA search table into ADU memory.
 *      We try an regenerate the IBSS search table here.
 *      Only in IBSS we put something in the search table.
 *      Later we will also add the BTAMP peer address.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pAddr:  Starting address in the ADU memory from where the
 *              register/value tuple will be written
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halAdu_BckupUmaSearchTable(tpAniSirGlobal pMac, tANI_U32 *pAddr)
{
    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    tANI_U32 startAddr = *pAddr;
    tANI_U32 *pMemAddr = (tANI_U32*)startAddr;
    tANI_U32 macAddrLo, macAddrHi;
    tANI_U32 value = 0;
    tANI_U8 i;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    tANI_U8 umaIdx = 0;

    for (i=0; i < pMac->hal.halMac.maxSta; i++) {

        if (!pSta[i].valid) {
            continue;
        }

        if (pSta[i].staType != STA_ENTRY_PEER) continue;

        // If we are infra mode we do not have a search table
        // entry, we only have an entry in UMA default MAC address. 
        systemRole = halGetBssSystemRoleFromStaIdx(pMac, pSta[i].staId);
        if (eSYSTEM_STA_ROLE == systemRole) continue;

        // In other modes we need to make sure an UMA entry was allocated
        if (halTable_GetStaUMAIdx(pMac, i, &umaIdx) != eHAL_STATUS_SUCCESS)
            continue;
        if (HAL_INVALID_KEYID_INDEX == umaIdx)
            continue;

        // Get the Mac address
        macAddrHi = (((tANI_U32)pSta[i].staAddr[0]) << 8) |
            ((tANI_U32)pSta[i].staAddr[1]);

        macAddrLo = (((tANI_U32)pSta[i].staAddr[2]) << 24) |
            (((tANI_U32)pSta[i].staAddr[3]) << 16) |
            (((tANI_U32)pSta[i].staAddr[4]) << 8) |
            ((tANI_U32)pSta[i].staAddr[5]);

        // Write the Low Data register address and value
        value = QWLAN_ADU_UMA_INDEX_TABLE_WDATA_L_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, sizeof(tANI_U32));

        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&macAddrLo, sizeof(tANI_U32));

        // Write the High data register address and value
        value = QWLAN_ADU_UMA_INDEX_TABLE_WDATA_U_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, sizeof(tANI_U32));

        value = macAddrHi |
            (umaIdx << QWLAN_ADU_UMA_INDEX_TABLE_WDATA_U_UMA_SRCHTABLE_INDEX_OFFSET);

        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, sizeof(tANI_U32));

        // Write the ctrl register address and value
        value = QWLAN_ADU_UMA_INDEX_TABLE_ADDR_CTRL_REG | HAL_REG_RSVD_BIT | HAL_REG_HOST_FILLED;
        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, sizeof(tANI_U32));

        //same staIdx is used for UMA search table indexing also
        value = (1 << QWLAN_ADU_UMA_INDEX_TABLE_ADDR_CTRL_UMA_SRCHTABLE_RW_OFFSET) | umaIdx;

        halWriteDeviceMemory(pMac, (tANI_U32)pMemAddr++,
                    (tANI_U8*)&value, sizeof(tANI_U32));

    }

    // Update the start address pointer pointing to the
    // register re-init table
    *pAddr = (tANI_U32)pMemAddr;

    return eHAL_STATUS_SUCCESS;
}

// Write the Default staIdx for Infra case in the 
// UMA config register.
eHalStatus halAdu_UpdateUMADefaultstaIdx(tpAniSirGlobal pMac, 
    tANI_U8 umaIdx) 
{
    tANI_U32 value = 0;

    halReadRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, &value);

    HALLOG1( halLog( pMac, LOG1, FL("value 0x%x\n"),  value ));

    // Clear out the existing value at the STA offset
    value &= (~QWLAN_ADU_UMA_CONFIG_TX_DEFAULT_STAID_MASK);

    value |= (umaIdx << QWLAN_ADU_UMA_CONFIG_TX_DEFAULT_STAID_OFFSET);

    halWriteRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, value);

    halReadRegister(pMac, QWLAN_ADU_UMA_CONFIG_REG, &value);
    HALLOG1( halLog( pMac, LOG1, FL("value 0x%x\n"),  value ));

    return eHAL_STATUS_SUCCESS;
}

// Function to allocate an UMA Descriptor table index.
eHalStatus halUMA_AllocId(tpAniSirGlobal pMac, tANI_U8 *umaIdx)
{
    tANI_U8             i;
    tANI_U8             found = 0;
    tpAduUmaStaDesc     pAduUmaStaDesc = NULL;


    for ( i=0; i < HAL_NUM_UMA_DESC_ENTRIES; i++ )
    {
        if (pMac->hal.halMac.aduUmaDesc[i].valid == 0)
        {
            found = 1;
            break;
        }
    }

    if(found)
    {
        HALLOG1( halLog(pMac, LOG1, FL("Got UMA descriptor index %d \n"), i));
        *umaIdx = i;
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("UMA descriptor table full")));
        return eHAL_STATUS_UMA_DESCRIPTOR_TABLE_FULL;
    }

    pAduUmaStaDesc = &(pMac->hal.halMac.aduUmaDesc[i]);

    palZeroDeviceMemory(pMac->hHdd, (tANI_U32)pAduUmaStaDesc,
        ADU_UMA_STA_DESC_ENTRY_SIZE);

    return eHAL_STATUS_SUCCESS;
}
