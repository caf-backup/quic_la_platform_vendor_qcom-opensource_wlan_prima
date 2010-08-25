/**
 *
 *  @file:      halDXE.c
 *
 *  @brief:     Provides all the APIs to configure DXE.
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------------------------------
 * 04/26/2007  Naveen G            File created.
 * 06/18/2008  Sanoop Kottontavida    Added halDXE_Open/Close/Start/Stop
 */


#include "palApi.h"
#include "halDebug.h"
#include "halDXE.h"
#include "wlan_qct_bal.h"
#ifdef ANI_LOGDUMP
#include "halLogDump.h"
#endif // ANI_DEBUG

/** Configuration MACROS */
#define DXE_USE_SHORT_DESC_FORMAT        1
#define HAL_DXE_BMU_THRLD_SEL_VALUE        3

static eHalStatus
halDXE_ConfigChannel(tHalHandle hHalHandle, eDMAChannel channel, sDXEChannelCfg *pCfg);


static eHalStatus halDXE_EnableErrorInterrupts(tHalHandle hHalHandle);


eHalStatus halIntDXEErrorHandler(tHalHandle hHalHandle, eHalIntSources intSource)
{
    tANI_U32 channel;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    channel = intSource - eHAL_INT_DXE_ERR_CHANNEL_0;
    HALLOGE( halLog(pMac, LOGE, FL("DXE Error Interrupt Received for Channel %d!!\n"), (int)channel));

    /* Fatal Issue -- reset chip */
    HALLOGE( halLog(pMac, LOGE, FL("DXE Fatal Error at channel %d!\n"), channel));
    macSysResetReq(pMac, eSIR_DXE_EXCEPTION);

    return (eHAL_STATUS_SUCCESS);
}

/**
 *    Enables error interrupts in DXE for channel 0 and 1.
 */
static eHalStatus halDXE_EnableErrorInterrupts(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 regValue = 0;

    regValue = QWLAN_DXE_0_INT_MSK_CH0_INTEN_MASK |
        QWLAN_DXE_0_INT_MSK_CH1_INTEN_MASK;

    status = halWriteRegister(pMac, QWLAN_DXE_0_INT_MSK_REG, regValue);

    return status;
}


/**
 *    @fn         halDXE_ConfigChannel
 *    @brief    Configures 1 channel at a time. Gen6 uses only 2 channels.
 *    
 *    @param    Channel number. 0 for Tx and 1 for Rx.
 *
 *    @return    eHalStatus    Success or Failure.
 */
static eHalStatus
halDXE_ConfigChannel(tHalHandle hHalHandle, eDMAChannel channel, sDXEChannelCfg *pCfg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    sDxeCCB    *pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[channel];
    tANI_U32    regVal;

    pDxeCCB->bdPresent          = pCfg->bdPresent;
    pDxeCCB->use_short_desc_fmt    = pCfg->useshortdescfmt;
    pDxeCCB->channel            = channel;

    pDxeCCB->chDXEBaseAddr        = QWLAN_DXE_0_CH_CTRL_REG + DXE_CH_REG_SIZE * channel;
    pDxeCCB->chDXECtrlRegAddr    = pDxeCCB->chDXEBaseAddr + DXE_CH_CTRL_REG;
    
    pDxeCCB->bmuThdSel        = pCfg->bmuThdSel;
    pDxeCCB->bmuThdSel_mask = pCfg->bmuThdSel << QWLAN_DXE_0_CH_CTRL_BTHLD_SEL_OFFSET;

    pDxeCCB->chPriority     = pCfg->chPriority;
    pDxeCCB->xfrType        = pCfg->xfrType;
    
    pDxeCCB->chDXEStatusRegAddr        = pDxeCCB->chDXEBaseAddr + DXE_CH_STATUS_REG;
    pDxeCCB->chDXEDesclRegAddr        = pDxeCCB->chDXEBaseAddr + DXE_CH_DESCL_REG;
    pDxeCCB->chDXEDeschRegAddr        = pDxeCCB->chDXEBaseAddr + DXE_CH_DESCH_REG;
    pDxeCCB->chDXELstDesclRegAddr    = pDxeCCB->chDXEBaseAddr + DXE_CH_LST_DESCL_REG;
    pDxeCCB->chDXESzRegAddr         = pDxeCCB->chDXEBaseAddr + DXE_CH_SZ_REG;
    pDxeCCB->chDXEDadrlRegAddr         = pDxeCCB->chDXEBaseAddr + DXE_CH_DADRL_REG;
    pDxeCCB->chDXEDadrhRegAddr         = pDxeCCB->chDXEBaseAddr + DXE_CH_DADRH_REG;

    pDxeCCB->chk_size        = pCfg->chk_size;
    pDxeCCB->chk_size_mask    = pDxeCCB->chk_size << QWLAN_DXE_0_CH_SZ_CHK_SZ_OFFSET;
    
    pDxeCCB->cw_ctrl_read    =  ani_cpu_to_be32(DXE_DESC_CTRL_SIQ |
                                DXE_DESC_CTRL_XTYPE_B2H|
                                DXE_DESC_CTRL_EOP |
                                ((pDxeCCB->bdPresent)? 0 : DXE_DESC_CTRL_BDH) |
                                (pDxeCCB->chPriority << QWLAN_DXE_0_CH_CTRL_PRIO_OFFSET) |
                                (pDxeCCB->bmuThdSel_mask) |
                                DXE_DESC_CTRL_PDU_REL |
                                ((pDxeCCB->use_short_desc_fmt) ? 0 : DXE_DESC_CTRL_DFMT) |
                                DXE_DESC_CTRL_INT );
                                        
    pDxeCCB->cw_ctrl_write_eop_int = pDxeCCB->cw_ctrl_write_valid = pDxeCCB->cw_ctrl_write = 
                                ani_cpu_to_be32( (pDxeCCB->xfrType << QWLAN_DXE_0_CH_CTRL_XTYPE_OFFSET) |
                                   ((DXE_XFR_HOST_TO_HOST == pDxeCCB->xfrType)? 0 : DXE_DESC_CTRL_DIQ) |
                                ((pDxeCCB->bdPresent) ? 0 : DXE_DESC_CTRL_BDH) |
                                ((pDxeCCB->chPriority) << QWLAN_DXE_0_CH_CTRL_PRIO_OFFSET) | 
                                   ((pDxeCCB->use_short_desc_fmt) ? 0 : DXE_DESC_CTRL_DFMT) | 
                                   (pDxeCCB->bmuThdSel_mask));
    
    pDxeCCB->cw_ctrl_write_valid |= DXE_DESC_CTRL_VALID;

    pDxeCCB->cw_ctrl_write_eop_int |= ( DXE_DESC_CTRL_EOP | DXE_DESC_CTRL_INT);

    pDxeCCB->chan_mask         =    QWLAN_DXE_0_CH_CTRL_EDVEN_MASK |
                                (channel << QWLAN_DXE_0_CH_CTRL_CTR_SEL_OFFSET) |
                                QWLAN_DXE_0_CH_CTRL_EDEN_MASK |
                                QWLAN_DXE_0_CH_CTRL_INE_ED_MASK |
                                ((pDxeCCB->use_short_desc_fmt) ? 0 : DXE_DESC_CTRL_DFMT) |
                                QWLAN_DXE_0_CH_CTRL_EN_MASK;

    pDxeCCB->refWQ                = pCfg->refWQ;
    pDxeCCB->refWQ_swapped        = ani_cpu_to_be32(pDxeCCB->refWQ);
    pDxeCCB->bmuThdSel_mask        = pCfg->bmuThdSel << QWLAN_DXE_0_CH_CTRL_BTHLD_SEL_OFFSET;
    pDxeCCB->sdioRxDescStart    = pMac->hal.pHalDxe->sdioRxDescStart;

    switch (channel)
    {
        case DMA_CHANNEL_TX:
            pDxeCCB->sdio_ch_desc            = SIF_TX_FIFO_BASE;

            if (pDxeCCB->use_short_desc_fmt)
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL    = ani_cpu_to_be32(SIF_TX_FIFO_BASE);
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL    = pDxeCCB->refWQ_swapped;
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.phyNextL       = ani_cpu_to_be32(SIF_TX_FIFO_BASE);
            }
            else
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.srcMemAddrL        = ani_cpu_to_be32(SIF_TX_FIFO_BASE);
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.dstMemAddrL        = pDxeCCB->refWQ_swapped;
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.phyNextL           = ani_cpu_to_be32(SIF_TX_FIFO_BASE);
            }
            
            pDxeCCB->sdioDesc.cw.ctrl        =   /* DXE_DESC_CTRL_INT | */
                        (pDxeCCB->chPriority << QWLAN_DXE_0_CH_CTRL_PRIO_OFFSET) |
                        (pDxeCCB->bmuThdSel_mask) |
                        DXE_DESC_CTRL_PDU_REL |
                        (pDxeCCB->xfrType << QWLAN_DXE_0_CH_CTRL_XTYPE_OFFSET) |
                        ((DXE_XFR_HOST_TO_HOST == pDxeCCB->xfrType)? 0 : DXE_DESC_CTRL_DIQ) |
                        DXE_DESC_CTRL_PIQ |
                        DXE_DESC_CTRL_SIQ |
                        DXE_DESC_CTRL_EOP |
                        DXE_DESC_CTRL_VALID |
                        (channel << QWLAN_DXE_0_CH_CTRL_CTR_SEL_OFFSET) |
                        (pDxeCCB->use_short_desc_fmt ? 0: DXE_DESC_CTRL_DFMT);
            pDxeCCB->sdioDesc.cw.ctrl = ani_cpu_to_be32(pDxeCCB->sdioDesc.cw.ctrl);
    
            regVal = QWLAN_DXE_0_CH_CTRL_PIQ_MASK;
            break;

            
        case DMA_CHANNEL_RX:
            // No need to swap the Rx descriptor as the Rx descriptors are read from SRAM
            // While the Tx descriptors are read from the UIF
            pDxeCCB->sdio_ch_desc            = pDxeCCB->sdioRxDescStart;

            if (pDxeCCB->use_short_desc_fmt)
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL    = (pDxeCCB->refWQ);
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL    = SIF_RX_FIFO_BASE | QUE_SIF_RX;
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.phyNextL       = (pDxeCCB->sdio_ch_desc);    // Make it a ring
            }
            else
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.srcMemAddrL        = (pDxeCCB->refWQ);
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.dstMemAddrL        = SIF_RX_FIFO_BASE | QUE_SIF_RX;
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.phyNextL           = (pDxeCCB->sdio_ch_desc);    // Make it a ring
            }
            
            pDxeCCB->sdioDesc.cw.ctrl        = ( /* DXE_DESC_CTRL_INT | */
                        (pDxeCCB->chPriority << QWLAN_DXE_0_CH_CTRL_PRIO_OFFSET) |
                        (pDxeCCB->bmuThdSel_mask) |
                        DXE_DESC_CTRL_PDU_REL |
                        DXE_DESC_CTRL_DIQ |
                        DXE_DESC_CTRL_SIQ |
                        DXE_DESC_CTRL_EOP |
                        DXE_DESC_CTRL_XTYPE_B2H |
                        DXE_DESC_CTRL_VALID |
                        (channel << QWLAN_DXE_0_CH_CTRL_CTR_SEL_OFFSET) |
                        (pDxeCCB->use_short_desc_fmt ? 0: DXE_DESC_CTRL_DFMT));
            
            halWriteDeviceMemory(pMac, pDxeCCB->sdio_ch_desc,
                        (tANI_U8 *)(&(pDxeCCB->sdioDesc)), sizeof(sDXEDesc));
            
            regVal = 0;
            break;

        default:
            HALLOGW( halLog(pMac, LOGW, FL("Wrong DMA channel number\n")));
            return eHAL_STATUS_FAILURE;
    }

    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_DESCL_REG,
                            pDxeCCB->sdio_ch_desc);
    
    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_SADRH_REG, 0);
    
    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_DADRH_REG, 0);
    
    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_DESCH_REG, 0);

    regVal |=   QWLAN_DXE_0_CH_CTRL_EDEN_MASK | /*DXE_0_CH_CTRL_INE_ED_MASK | */ 
                QWLAN_DXE_0_CH_CTRL_EN_MASK |
                QWLAN_DXE_0_CH_CTRL_INE_ERR_MASK | /** Enabling error interrupt */
                (channel << QWLAN_DXE_0_CH_CTRL_CTR_SEL_OFFSET) |
                (pDxeCCB->use_short_desc_fmt ? 0: DXE_DESC_CTRL_DFMT); 

    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_CTRL_REG, regVal);

    // Configure the correct BD/PDU threshold for this channel
    halReadRegister(pMac, 
                        pDxeCCB->chDXEBaseAddr + DXE_CH_CTRL_REG, 
                        &regVal);

    halWriteRegister(pMac, 
                    pDxeCCB->chDXEBaseAddr + DXE_CH_CTRL_REG, 
                    (regVal & ~QWLAN_DXE_0_CH_CTRL_BTHLD_SEL_MASK) | 
                    (regVal & QWLAN_DXE_0_CH_CTRL_CTR_SEL_MASK) |
                    (pDxeCCB->bmuThdSel_mask));
    
    regVal = pDxeCCB->chk_size_mask;
    halWriteRegister(pMac, pDxeCCB->chDXEBaseAddr + DXE_CH_SZ_REG, regVal);

    pDxeCCB->chConfigured   = eANI_BOOLEAN_TRUE;
    pDxeCCB->chEnabled      = eANI_BOOLEAN_TRUE;     // Channel enabled

    return (eHAL_STATUS_SUCCESS);
}


/**
 * Allocates the resources required for DXE module.
 */
eHalStatus halDXE_Open(tHalHandle hHalHandle,  void *arg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus status = eHAL_STATUS_FAILURE;
    
    /* Allocate HalDxe */
    status = palAllocateMemory(pMac->hHdd, ((void **)&pMac->hal.pHalDxe), sizeof(tAniHalDxe));
    if (eHAL_STATUS_SUCCESS != status){
        HALLOGW( halLog(pMac, LOGW, FL("DxE Open failed\n")));
        return status;
    }

    /* Initialize the HalDxe structure */
    palZeroMemory(pMac->hHdd, pMac->hal.pHalDxe, sizeof(tAniHalDxe));

    return eHAL_STATUS_SUCCESS;
}

/**
 * Deallocates the resources used by DXE module.
 */
eHalStatus halDXE_Close(tHalHandle hHalHandle,  void *arg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus status = eHAL_STATUS_FAILURE;
    
    /* De-allocate HalDxe */
    status = palFreeMemory(pMac->hHdd, pMac->hal.pHalDxe);

    return status;
}


/**
 * Initializes the DXE module.
 */
eHalStatus halDXE_Start(tHalHandle hHalHandle,  void *arg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniHalDxe    pHalDxe = pMac->hal.pHalDxe;
    sDXEChannelCfg DXEChCfgInfo;
    tANI_U32 regVal = 0;
    sDxeCCB    *pDxeCCB;
    WLANBAL_SDIODXEHeaderConfigType dxeHdrCfg;

    palZeroMemory(pMac->hHdd, pHalDxe, sizeof(*pHalDxe));

    pHalDxe->sdioRxDescStart = pMac->hal.memMap.dxeRxDescInfo_offset;

    palZeroMemory(pMac->hHdd, &DXEChCfgInfo, sizeof(sDXEChannelCfg));

    //Is this information coming from caller???
    DXEChCfgInfo.bdPresent = eANI_BOOLEAN_TRUE;
    DXEChCfgInfo.chPriority = 0;
    DXEChCfgInfo.bmuThdSel = QWLAN_HAL_DXE0_MASTERID;
    DXEChCfgInfo.chk_size = 0;    /** 128 bytes */
    
    DXEChCfgInfo.useshortdescfmt = DXE_USE_SHORT_DESC_FORMAT;

    regVal = QWLAN_DXE_0_DMA_CSR_RESET_MASK;         // Reset DXE
    halWriteRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, regVal);
    
    // De-assert RESET, enable DXE, enable channel counter 
    // This is not enough. we need to enable per channel counters. Pay attention to to destroy CTRL_SEL bits in CH_CTRL registers
    regVal = QWLAN_DXE_0_DMA_CSR_EN_MASK | QWLAN_DXE_0_DMA_CSR_ECTR_EN_MASK;     
    halWriteRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, regVal);

    // DXE will send it to WQ 23, which is the ADU/UMA.. 
    DXEChCfgInfo.refWQ = BMUWQ_ADU_UMA_TX;
    DXEChCfgInfo.xfrType = DXE_XFR_HOST_TO_BMU;

    status = halDXE_ConfigChannel(hHalHandle, DMA_CHANNEL_TX, &DXEChCfgInfo);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW( halLog(pMac, LOGW, FL("DMA Channel Configuration for Tx failed\n")));
        return status;
    }

     pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_TX];
     
     dxeHdrCfg.TXChannel.descriptorControl = pDxeCCB->sdioDesc.cw.ctrl;
     dxeHdrCfg.TXChannel.destinationAddress = 
         pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL;
     dxeHdrCfg.TXChannel.sourceAddress = dxeHdrCfg.TXChannel.nextAddress =  
         pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL;


    DXEChCfgInfo.refWQ = BMUWQ_DXE_RX;
    DXEChCfgInfo.xfrType = DXE_XFR_BMU_TO_HOST;

    status = halDXE_ConfigChannel(hHalHandle, DMA_CHANNEL_RX, &DXEChCfgInfo);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW( halLog(pMac, LOGW, FL("DMA Channel Configuration for Rx failed\n")));
        return status;
    }

     pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX];
     dxeHdrCfg.RXChannel.descriptorControl = pDxeCCB->sdioDesc.cw.ctrl;
     dxeHdrCfg.RXChannel.destinationAddress = 
        pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL;
     dxeHdrCfg.RXChannel.sourceAddress = dxeHdrCfg.RXChannel.nextAddress = 
         pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL;

    status = halDXE_EnableErrorInterrupts(hHalHandle);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW( halLog(pMac, LOGW, FL("DMA Channel Configuration for Rx failed\n")));
        return status;
    }

    pHalDxe->Configured = eANI_BOOLEAN_TRUE;
    
    {   
        v_VOID_t * pVosContext = vos_get_global_context(VOS_MODULE_ID_HDD, pMac->hHdd);
        WLANBAL_DXEHeaderConfig(pVosContext, &dxeHdrCfg );
    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * Uninitializes the DXE module.
 */
eHalStatus halDXE_Stop(tHalHandle hHalHandle,  void *arg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tpAniHalDxe    pHalDxe = pMac->hal.pHalDxe;
    sDxeCCB    *pDxeCCB = &pHalDxe->DxeCCB[0];
    tANI_U32    channel;

    for (channel = 0; channel < DMA_CHANNEL_MAX; channel++)
    {
        pDxeCCB[channel].chConfigured = eANI_BOOLEAN_FALSE;
    }

    pHalDxe->Configured = eANI_BOOLEAN_FALSE;

    return eHAL_STATUS_SUCCESS;
}

/* 
 * Enable/Disable the DXE 
 */
eHalStatus halDxe_EnableDisableDXE(tHalHandle hHalHandle, tANI_U8 enable) 
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32 regValue = 0;

    // Enable DXE engine, reset the enable bit in the DXE CSR register
    halReadRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, &regValue); 

    if (enable) {
        regValue |= QWLAN_DXE_0_DMA_CSR_EN_MASK;
    } else {
        regValue &= ~QWLAN_DXE_0_DMA_CSR_EN_MASK;
    }

    halWriteRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, regValue);

    return eHAL_STATUS_SUCCESS;
}

