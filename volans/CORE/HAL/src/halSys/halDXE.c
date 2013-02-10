/**
 *
 *  @file:      halDXE.c
 *
 *  @brief:     Provides all the APIs to configure DXE.
 *
 *  Copyright (C) 2008, Qualcomm Technologies, Inc. All rights reserved.
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
#ifdef WLAN_HAL_VOLANS
    pDxeCCB->sdioRxHiDescStart    = pMac->hal.pHalDxe->sdioRxHiDescStart;
#endif
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
#ifdef WLAN_HAL_VOLANS
        case DMA_CHANNEL_RX_HI:
            // No need to swap the Rx descriptor as the Rx descriptors are read from SRAM
            // While the Tx descriptors are read from the UIF
            pDxeCCB->sdio_ch_desc            = pDxeCCB->sdioRxHiDescStart;

            if (pDxeCCB->use_short_desc_fmt)
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL    = (pDxeCCB->refWQ);
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL    = SIF_RX_HI_FIFO_BASE | QUE_SIF_RX_HI;
                pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.phyNextL       = (pDxeCCB->sdio_ch_desc);    // Make it a ring
            }
            else
            {
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.srcMemAddrL        = (pDxeCCB->refWQ);
                pDxeCCB->sdioDesc.dxedesc.dxe_long_desc.dstMemAddrL        = SIF_RX_HI_FIFO_BASE | QUE_SIF_RX_HI;
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
#endif

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

    /* Save the DXE Control register value for this channel. This is needed for later use */
    halReadRegister(pMac, pDxeCCB->chDXECtrlRegAddr, &pDxeCCB->chDXECtrlRegValue);
    
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
#ifdef WLAN_HAL_VOLANS
    pHalDxe->sdioRxHiDescStart = pMac->hal.memMap.dxeRxHiDescInfo_offset;
#endif
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

    //Configure Tx channel 

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

    //Configure Rx channel 

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


    //Configure Rx channel 
#ifdef WLAN_HAL_VOLANS
    DXEChCfgInfo.refWQ = BMUWQ_DXE_RX_HI;
    DXEChCfgInfo.xfrType = DXE_XFR_BMU_TO_HOST;

    status = halDXE_ConfigChannel(hHalHandle, DMA_CHANNEL_RX_HI, &DXEChCfgInfo);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW( halLog(pMac, LOGW, FL("DMA Channel Configuration for Rx failed\n")));
        return status;
    }

     pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX_HI];
     dxeHdrCfg.RXHiChannel.descriptorControl = pDxeCCB->sdioDesc.cw.ctrl;
     dxeHdrCfg.RXHiChannel.destinationAddress = 
        pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.dstMemAddrL;
     dxeHdrCfg.RXHiChannel.sourceAddress = dxeHdrCfg.RXHiChannel.nextAddress = 
         pDxeCCB->sdioDesc.dxedesc.dxe_short_desc.srcMemAddrL;
#endif


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
#ifdef FEATURE_WLAN_CCX
    halDxe_SetTimeStamp(pMac);
#endif
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
 * Ensure DXE IDLE state by stopping the Rx channel
 */
eHalStatus halDxe_EnsureDXEIdleState(tHalHandle hHalHandle) 
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
	sDxeCCB    *pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX];
	tANI_U32	dwCtrl = pDxeCCB->sdioDesc.cw.ctrl | DXE_DESC_CTRL_STOP;

    halWriteDeviceMemory(pMac, pDxeCCB->sdio_ch_desc,
                        (tANI_U8 *)&dwCtrl, 4);

    /* Volans has two DxE RX channels, DXE_RX and DXE_RX_HI. 
     * Both channels have to be stopped before entering into IMPS mode
     */
	pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX_HI];
	dwCtrl  = pDxeCCB->sdioDesc.cw.ctrl | DXE_DESC_CTRL_STOP;

    halWriteDeviceMemory(pMac, pDxeCCB->sdio_ch_desc,
                        (tANI_U8 *)&dwCtrl, 4);

    vos_sleep(2);

	return eHAL_STATUS_SUCCESS;
}

/* 
 * Enable/Disable the DXE 
 */
eHalStatus halDxe_EnableDisableDXE(tHalHandle hHalHandle, tANI_U8 enable) 
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32       regValue=0; 
	sDxeCCB        *pDxeCCB;

	// If DXE is being disabled ensure that it is either in the stopped or MASKED state
	if (!enable) {

		// Remove the STOP bit from the DXE descriptor CTRL DWORD for CHANNEL 1 (RX Channel)
        pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX];
		halWriteDeviceMemory(pMac, pDxeCCB->sdio_ch_desc,
                             (tANI_U8 *)&pDxeCCB->sdioDesc.cw.ctrl, 4);
        /* Stopping the channel will disable the channel */
        /* Update the DXE Channel Control register and explicitly enable the channel */
        halWriteRegister(pMac, pDxeCCB->chDXECtrlRegAddr, pDxeCCB->chDXECtrlRegValue);

        pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX_HI];
        // Remove the STOP bit from the DXE descriptor CTRL DWORD for CHANNEL 3 (RX HI Channel)
		halWriteDeviceMemory(pMac, pDxeCCB->sdio_ch_desc,
                             (tANI_U8 *)&pDxeCCB->sdioDesc.cw.ctrl, 4);
        /* Stopping the channel will disable the channel */
        /* Update the DXE Channel Control register and explicitly enable the channel */
        halWriteRegister(pMac, pDxeCCB->chDXECtrlRegAddr, pDxeCCB->chDXECtrlRegValue);
	}

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

/**
 * Get DXE channel status
 **/
VOS_STATUS halDxe_DxeChannelIdleStatus(tANI_U32 *pStatus, v_PVOID_t pMacContext)
{

#define DXE_0_BMU_SB_QDAT_WQ_2_OFFSET 0x2
    
    VOS_STATUS     status;
    tpAniSirGlobal pMac    = PMAC_STRUCT(pMacContext);
    tANI_U32       bmuSB=0, dxeRxChStatus=0, dxeRxHiChStatus=0;
	sDxeCCB        *pDxeCCB;

    /*
     * DxE Channel status for DXE_RX and DXE_RX_HI channels
     */    
    pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX];
	halReadRegister(pMac, pDxeCCB->chDXEStatusRegAddr, &dxeRxChStatus);
	
    pDxeCCB = &pMac->hal.pHalDxe->DxeCCB[DMA_CHANNEL_RX_HI];
	halReadRegister(pMac, pDxeCCB->chDXEStatusRegAddr, &dxeRxHiChStatus);
    
    /**
     * DxE has side band register status from BMU
     */
    halReadRegister(pMac, QWLAN_DXE_0_BMU_SB_QDAT_AVAIL_REG, &bmuSB);

    status = (((dxeRxChStatus & QWLAN_DXE_0_CH_STATUS_STOPD_MASK) || 
              ((dxeRxChStatus & QWLAN_DXE_0_CH_STATUS_MSKD_MASK) && !(bmuSB & (1 << (BMUWQ_DXE_RX - DXE_0_BMU_SB_QDAT_WQ_2_OFFSET))))) && 
             ((dxeRxHiChStatus & QWLAN_DXE_0_CH_STATUS_STOPD_MASK) || 
              ((dxeRxHiChStatus & QWLAN_DXE_0_CH_STATUS_MSKD_MASK) && !(bmuSB & (1 << (BMUWQ_DXE_RX_HI - DXE_0_BMU_SB_QDAT_WQ_2_OFFSET)))))) 
             ? VOS_STATUS_SUCCESS : VOS_STATUS_E_FAILURE;

    if(VOS_STATUS_E_FAILURE == status)
    {
        HALLOGE( halLog(pMac, LOGE, FL("dxeRxChStatus=0x%x, dxeRxHiChStatus=0x%x, bmuSB=0x%x\n"), dxeRxChStatus, dxeRxHiChStatus, bmuSB));
    }
   
    *pStatus = status; 
    return (status);
}

#ifdef FEATURE_WLAN_CCX

eHalStatus halDxe_EnableTimeStamp(tHalHandle hHalHandle, tANI_U8 enable)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32       regVal;
    eHalStatus status=eHAL_STATUS_SUCCESS;

    status = halReadRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, &regVal);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halReadRegister failed\n")));
        return status;
    }

    if(enable)
    {
        if(!(regVal & QWLAN_DXE_0_DMA_CSR_TSTMP_EN_MASK))
            regVal |=QWLAN_DXE_0_DMA_CSR_TSTMP_EN_MASK;
    }
    else
    {
        if((regVal & QWLAN_DXE_0_DMA_CSR_TSTMP_EN_MASK))
            regVal &=~QWLAN_DXE_0_DMA_CSR_TSTMP_EN_MASK;
    }

    status = halWriteRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, regVal);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halReadRegister failed\n")));
        return status;
    }

    return status;
}

eHalStatus halDxe_SetTimeStampOffset(tHalHandle hHalHandle, tANI_U8 tx_offset, tANI_U8 rx_offset)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U32       regVal;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    status = halReadRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, &regVal);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halReadRegister failed\n")));
        return status;
    }
    /*First clear before set*/
    regVal &= ~QWLAN_DXE_0_DMA_CSR_H2B_TSTMP_OFF_MASK;
    regVal |= (tANI_U32)(tx_offset << QWLAN_DXE_0_DMA_CSR_H2B_TSTMP_OFF_OFFSET);
    
    regVal &= ~QWLAN_DXE_0_DMA_CSR_B2H_TSTMP_OFF_MASK;
    regVal |= (tANI_U32)(rx_offset << QWLAN_DXE_0_DMA_CSR_B2H_TSTMP_OFF_OFFSET);

    status = halWriteRegister(pMac, QWLAN_DXE_0_DMA_CSR_REG, regVal);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halReadRegister failed\n")));
        return status;
    }
    return status;
}

eHalStatus halDxe_SetTimeStamp(tHalHandle hHalHandle)
{
    eHalStatus status;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);
    tANI_U8 txBdTsOffset,rxBdTsOffset;

    txBdTsOffset = offsetof(halTxBd_type,dxeH2BStartTimestamp)/sizeof(tANI_U32);
    rxBdTsOffset = offsetof(halRxBd_type,mclkRxTimestamp)/sizeof(tANI_U32);

    status = halDxe_SetTimeStampOffset(pMac,txBdTsOffset,rxBdTsOffset);

    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog(pMac, LOGE, 
               FL("halDxe_SetTimeStampOffset failed!!!\n")));
        return eHAL_STATUS_FAILURE;
    }

    /*Enable the time stamp in the dxe_0 DMA Control and Status Register*/
    status = halDxe_EnableTimeStamp(pMac,VOS_TRUE);

    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog(pMac, LOGE, 
                     FL("halDxe_EnableTimeStamp failed!!!\n")));
        return eHAL_STATUS_FAILURE;
    }

    return status;
}

#endif

