/**=========================================================================
 *     
 *       \file  wlan_qct_wdi_dts.c
 *          
 *       \brief  Data Transport Service API 
 *                               
 * WLAN Device Abstraction layer External API for Dataservice
 * DESCRIPTION
 *  This file contains the external API implemntation exposed by the 
 *   wlan device abstarction layer module.
 *
 *   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
 *   Qualcomm Confidential and Proprietary
 */


#include "wlan_qct_wdi.h"
#include "wlan_qct_dxe.h"
#include "wlan_qct_wdi_ds.h"
#include "wlan_qct_wdi_ds_i.h"
#include "wlan_qct_wdi_dts.h"
#include "wlan_qct_wdi_dp.h"

static WDTS_TransportDriverTrype gTransportDriver = {
  WLANDXE_Open, 
  WLANDXE_Start, 
  WLANDXE_ClientRegistration, 
  WLANDXE_TxFrame
};

/* DTS Tx packet complete function. 
 * This function should be invoked by the transport device to indicate 
 * transmit complete for a frame.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller 
 * pFrame:Refernce to PAL frame.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_TxPacketComplete(void *pContext, wpt_packet *pFrame, wpt_status status)
{
  WDI_DS_ClientDataType *pClientData = (WDI_DS_ClientDataType*)(pContext);
  WDI_DS_TxMetaInfoType     *pTxMetadata;
  void *pvBDHeader, *physBDHeader;

  // Do Sanity checks
  if(NULL == pContext || NULL == pFrame){
    return eWLAN_PAL_STATUS_E_FAILURE;
  }


  // extract metadata from PAL packet
  pTxMetadata = WDI_DS_ExtractTxMetaData(pFrame);
  pTxMetadata->txCompleteStatus = status;

  // Free BD header from pool
  WDI_GetBDPointers(pFrame, &pvBDHeader,  &physBDHeader);
  WDI_DS_MemPoolFree(&(pClientData->memPool), pvBDHeader, physBDHeader);
  WDI_SetBDPointers(pFrame, 0, 0);

  // Invoke Tx complete callback
  pClientData->txCompleteCB(pClientData->pCallbackContext, pFrame);  
  return eWLAN_PAL_STATUS_SUCCESS;

}




/* DTS Rx packet function. 
 * This function should be invoked by the transport device to indicate 
 * reception of a frame.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller 
 * pFrame:Refernce to PAL frame.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_RxPacket (void *pContext, wpt_packet *pFrame, WDTS_ChannelType channel)
{
  WDI_DS_ClientDataType *pClientData = 
    (WDI_DS_ClientDataType*)(pContext);
  wpt_boolean       bASF, bFSF, bLSF, bAEF;
  wpt_uint8                   ucMPDUHOffset, ucMPDUHLen, ucTid;
  wpt_uint8                   *pBDHeader;
  wpt_uint16                  usMPDUDOffset, usMPDULen;
  WDI_DS_RxMetaInfoType     *pRxMetadata;

  tpSirMacFrameCtl  pMacFrameCtl;

  // Do Sanity checks
  if(NULL == pContext || NULL == pFrame){
    return eWLAN_PAL_STATUS_E_FAILURE;
  }


  /*------------------------------------------------------------------------
    Extract BD header and check if valid
    ------------------------------------------------------------------------*/
  pBDHeader = (wpt_uint8*)wpalPacketGetRawBuf(pFrame);
  wpalPacketRawTrimHead(pFrame, WDI_RX_BD_HEADER_SIZE);
  WDI_SwapRxBd(pBDHeader);

  ucMPDUHOffset = (wpt_uint8)WDI_RX_BD_GET_MPDU_H_OFFSET(pBDHeader);
  usMPDUDOffset = (wpt_uint16)WDI_RX_BD_GET_MPDU_D_OFFSET(pBDHeader);
  usMPDULen     = (wpt_uint16)WDI_RX_BD_GET_MPDU_LEN(pBDHeader);
  ucMPDUHLen    = (wpt_uint8)WDI_RX_BD_GET_MPDU_H_LEN(pBDHeader);
  ucTid         = (wpt_uint8)WDI_RX_BD_GET_TID(pBDHeader);

  /*------------------------------------------------------------------------
    Gather AMSDU information 
    ------------------------------------------------------------------------*/
  bASF = WDI_RX_BD_GET_ASF(pBDHeader);
    bAEF = WDI_RX_BD_GET_AEF(pBDHeader);
    bFSF = WDI_RX_BD_GET_ESF(pBDHeader);
    bLSF = WDI_RX_BD_GET_LSF(pBDHeader);

  DTI_TRACE( DTI_TRACE_LEVEL_INFO,
      "WLAN TL:BD header processing data: HO %d DO %d Len %d HLen %d"
      " Tid %d BD %d",
      ucMPDUHOffset, usMPDUDOffset, usMPDULen, ucMPDUHLen, ucTid,
      WDI_RX_BD_HEADER_SIZE);

  if(usMPDUDOffset <= ucMPDUHOffset || usMPDULen < ucMPDUHLen) {
    DTI_TRACE( DTI_TRACE_LEVEL_ERROR,
        "WLAN TL:BD header corrupted - dropping packet");
    /* Drop packet ???? */ 
    wpalPacketFree(pFrame);
    return eWLAN_PAL_STATUS_SUCCESS;
  }

  if((ucMPDUHOffset < WDI_RX_BD_HEADER_SIZE) &&  (!(bASF && !bFSF))){
    /* AMSDU case, ucMPDUHOffset = 0  it should be hancdled seperatly */
    /* Drop packet ???? */ 
    wpalPacketFree(pFrame);
    return eWLAN_PAL_STATUS_SUCCESS;
  }


  if ( ucMPDUHOffset > WDI_RX_BD_HEADER_SIZE ) 
    wpalPacketRawTrimHead( pFrame, ucMPDUHOffset - WDI_RX_BD_HEADER_SIZE);


  pRxMetadata = WDI_DS_ExtractRxMetaData(pFrame);

  pRxMetadata->staId = WDI_RX_BD_GET_STA_ID(pBDHeader);
  pRxMetadata->addr3Idx = WDI_RX_BD_GET_ADDR3_IDX(pBDHeader);
  pRxMetadata->rxChannel = WDI_RX_BD_GET_RX_CHANNEL(pBDHeader);
  pRxMetadata->rtsf = WDI_RX_BD_GET_RTSF(pBDHeader);
  pRxMetadata->bsf = WDI_RX_BD_GET_BSF(pBDHeader);
  pRxMetadata->scan = WDI_RX_BD_GET_SCAN(pBDHeader);
  pRxMetadata->dpuSig = WDI_RX_BD_GET_DPU_SIG(pBDHeader);
  pRxMetadata->ft = WDI_RX_BD_GET_FT(pBDHeader);
  pRxMetadata->ne = WDI_RX_BD_GET_NE(pBDHeader);
  pRxMetadata->llcr = WDI_RX_BD_GET_LLCR(pBDHeader);
  pRxMetadata->bcast = WDI_RX_BD_GET_UB(pBDHeader);
  pRxMetadata->tid = ucTid;
  pRxMetadata->dpuFeedback = WDI_RX_BD_GET_DPU_FEEDBACK(pBDHeader);
  pRxMetadata->rateIndex = WDI_RX_BD_GET_RATEINDEX(pBDHeader);
  pRxMetadata->rxpFlags = WDI_RX_BD_GET_RXPFLAGS(pBDHeader);
  pRxMetadata->mclkRxTimestamp = WDI_RX_BD_GET_TIMESTAMP(pBDHeader);

  /* typeSubtype in BD doesn't look like correct. Fill from frame ctrl
     TL does it for Volans but TL does not know BD for Prima. WDI should do it */
  if ( 0 == WDI_RX_BD_GET_FT(pBDHeader) ) {
    if ( bASF ) {
      pRxMetadata->subtype = WDI_MAC_DATA_QOS_DATA;
      pRxMetadata->type    = WDI_MAC_DATA_FRAME;
    } else {
      pMacFrameCtl = (tpSirMacFrameCtl)(((wpt_uint8*)pBDHeader) + ucMPDUHOffset);
      pRxMetadata->subtype = pMacFrameCtl->subType;
      pRxMetadata->type    = pMacFrameCtl->type;
    }
  } else {
    pMacFrameCtl = (tpSirMacFrameCtl)(((wpt_uint8*)pBDHeader) + WDI_RX_BD_HEADER_SIZE);
    pRxMetadata->subtype = pMacFrameCtl->subType;
    pRxMetadata->type    = pMacFrameCtl->type;
  }

  pRxMetadata->mpduHeaderPtr = pBDHeader + ucMPDUHOffset;
  pRxMetadata->mpduDataPtr = pBDHeader + usMPDUDOffset;
  pRxMetadata->mpduLength = usMPDULen;
  pRxMetadata->mpduHeaderLength = ucMPDUHLen;

  /*------------------------------------------------------------------------
    Gather AMPDU information 
    ------------------------------------------------------------------------*/
    pRxMetadata->ampdu_reorderOpcode  = (wpt_uint8)WDI_RX_BD_GET_BA_OPCODE(pBDHeader);
    pRxMetadata->ampdu_reorderSlotIdx = (wpt_uint8)WDI_RX_BD_GET_BA_SI(pBDHeader);
    pRxMetadata->ampdu_reorderFwdIdx  = (wpt_uint8)WDI_RX_BD_GET_BA_FI(pBDHeader);
    pRxMetadata->currentPktSeqNo       = (wpt_uint8)WDI_RX_BD_GET_BA_CSN(pBDHeader);


  /*------------------------------------------------------------------------
    Gather AMSDU information 
    ------------------------------------------------------------------------*/
    pRxMetadata->amsdu_asf =  bASF;
    pRxMetadata->amsdu_aef =  bAEF;
    pRxMetadata->amsdu_esf =  bFSF;
    pRxMetadata->amsdu_lsf =  bLSF;
    pRxMetadata->amsdu_size =  WDI_RX_BD_GET_AMSDU_SIZE(pBDHeader);

    pRxMetadata->rssi0 = WDI_RX_BD_GET_RSSI0(pBDHeader);
    pRxMetadata->rssi1 = WDI_RX_BD_GET_RSSI1(pBDHeader);

  /* 
   * PAL BD pointer information needs to be populated 
   */ 
  WPAL_PACKET_SET_BD_POINTER(pFrame, pBDHeader);
  WPAL_PACKET_SET_BD_LENGTH(pFrame, sizeof(WDI_RxBdType));

  // Invoke Rx complete callback
  pClientData->receiveFrameCB(pClientData->pCallbackContext, pFrame);  
  return eWLAN_PAL_STATUS_SUCCESS;

}



/* DTS Out of Resource packet function. 
 * This function should be invoked by the transport device to indicate 
 * the device is out of resources.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller 
 * priority: indicates which channel is out of resource.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 */
wpt_status WDTS_OOResourceNotification(void *pContext, WDTS_ChannelType channel, wpt_boolean on)
{
  WDI_DS_ClientDataType *pClientData =
    (WDI_DS_ClientDataType *) pContext;
  static wpt_uint8 ac_mask = 0;

  // Do Sanity checks
  if(NULL == pContext){
    return eWLAN_PAL_STATUS_E_FAILURE;
  }
  
  if(on){
    ac_mask |=  channel == WDTS_CHANNEL_TX_LOW_PRI?  0x0f : 0x10;
  } else {
    ac_mask &=  channel == WDTS_CHANNEL_TX_LOW_PRI?  0x10 : 0x0f;
  }


  // Invoke OOR callback
  pClientData->txResourceCB(pClientData->pCallbackContext, ac_mask); 
  return eWLAN_PAL_STATUS_SUCCESS;

}

/* DTS open  function. 
 * On open the transport device should initialize itself.
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along 
 *  with the callback.
 *
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_openTransport( void *pContext)
{
  void *pDTDriverContext; 
  WDI_DS_ClientDataType *pClientData;

  pClientData = (WDI_DS_ClientDataType*) wpalMemoryAllocate(sizeof(WDI_DS_ClientDataType));
  if(!pClientData)
    return eWLAN_PAL_STATUS_E_NOMEM;

  pClientData->suspend = 0;
  WDI_DS_AssignDatapathContext(pContext, (void*)pClientData);

  pDTDriverContext = gTransportDriver.open(); 
  WDT_AssignTransportDriverContext(pContext, pDTDriverContext);
  gTransportDriver.register_client(pDTDriverContext, WDTS_RxPacket, WDTS_TxPacketComplete, 
    WDTS_OOResourceNotification, (void*)pClientData);
  return eWLAN_PAL_STATUS_SUCCESS;

}



/* DTS start  function. 
 * On start the transport device should start running.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller along 
 * with the callback.
 *
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_startTransport( void *pContext)
{
  void *pDTDriverContext = WDT_GetTransportDriverContext(pContext);
  gTransportDriver.start(pDTDriverContext); 
  return eWLAN_PAL_STATUS_SUCCESS;

}


/* DTS Tx packet function. 
 * This function should be invoked by the DAL Dataservice to schedule transmit frame through DXE/SDIO.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller along with the callback.
 * pFrame:Refernce to PAL frame.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_TxPacket(void *pContext, wpt_packet *pFrame)
{
  void *pDTDriverContext = WDT_GetTransportDriverContext(pContext);
  WDI_DS_TxMetaInfoType     *pTxMetadata;
  WDTS_ChannelType channel = WDTS_CHANNEL_TX_LOW_PRI;
  wpt_status status = eWLAN_PAL_STATUS_SUCCESS;

  // extract metadata from PAL packet
  pTxMetadata = WDI_DS_ExtractTxMetaData(pFrame);

  // assign MDPU to correct channel??
  channel =  (pTxMetadata->frmType & WDI_MAC_DATA_FRAME)? 
      WDTS_CHANNEL_TX_LOW_PRI : WDTS_CHANNEL_TX_HIGH_PRI;
  
  // Send packet to  Transport Driver. 
  status =  gTransportDriver.xmit(pDTDriverContext, pFrame, channel);

  return status;
}
