/*===========================================================================


                     W L A N _ Q C T _ WDA _ DS _ VOLANS . C

  OVERVIEW:

  This software unit holds the implementation of WLAN Data Abtraction APIs
  for the WLAN Transport Layer.

  DEPENDENCIES:

  Are listed for each API below.


  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when        who          what, where, why
--------    ---         ----------------------------------------------
12/08/2010  seokyoun    Created. Move down HAL interfaces from TL to WDA
                        for UMAC convergence btween Volans/Libra and Prima
=========================================================================== */

#include "wlan_qct_wda.h"
#include "wlan_qct_tl.h"
#include "wlan_qct_tli.h"
#include "wlan_qct_wdi_ds.h"

#ifdef WLAN_PERF
/*==========================================================================
  FUNCTION    WDA_TLI_FastHwFwdDataFrame

  DESCRIPTION 
    For NON integrated SOC, this function is called by TL.

    Fast path function to quickly forward a data frame if HAL determines BD 
    signature computed here matches the signature inside current VOSS packet. 
    If there is a match, HAL and TL fills in the swapped packet length into 
    BD header and DxE header, respectively. Otherwise, packet goes back to 
    normal (slow) path and a new BD signature would be tagged into BD in this
    VOSS packet later by the WLANHAL_FillTxBd() function.

  TODO  For integrated SOC, this function does nothing yet. Pima SLM/HAL 
        should provide the equivelant functionality.

  DEPENDENCIES 
     
  PARAMETERS 

   IN
        pvosGCtx    VOS context
        vosDataBuff Ptr to VOSS packet
        pMetaInfo   For getting frame's TID
        pStaInfo    For checking STA type
    
   OUT
        pvosStatus  returned status
        puFastFwdOK Flag to indicate whether frame could be fast forwarded
   
  RETURN VALUE
    No return.   

  SIDE EFFECTS 
  
============================================================================*/
void WDA_TLI_FastHwFwdDataFrame
(
  v_PVOID_t     pvosGCtx,
  vos_pkt_t*    vosDataBuff,
  VOS_STATUS*   pvosStatus,
  v_U32_t*       puFastFwdOK,
  WLANTL_MetaInfoType*  pMetaInfo,
  WLAN_STADescType*  pStaInfo
)
{
    v_PVOID_t   pvPeekData;
    v_U8_t      ucDxEBDWLANHeaderLen = WLANTL_BD_HEADER_LEN(0) + sizeof(WLANBAL_sDXEHeaderType); 
    v_U8_t      ucIsUnicast;
    WLANBAL_sDXEHeaderType  *pDxEHeader;
    v_PVOID_t   pvBDHeader;
    v_PVOID_t   pucBuffPtr;
    v_U16_t      usPktLen;

   /*-----------------------------------------------------------------------
    Extract packet length
    -----------------------------------------------------------------------*/

    vos_pkt_get_packet_length( vosDataBuff, &usPktLen);

   /*-----------------------------------------------------------------------
    Extract MAC address
    -----------------------------------------------------------------------*/
    *pvosStatus = vos_pkt_peek_data( vosDataBuff, 
                                 WLANTL_MAC_ADDR_ALIGN(0), 
                                 (v_PVOID_t)&pvPeekData, 
                                 VOS_MAC_ADDR_SIZE );

    if ( VOS_STATUS_SUCCESS != *pvosStatus ) 
    {
       TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                  "WLAN TL:Failed while attempting to extract MAC Addr %d", 
                  *pvosStatus));
       *pvosStatus = VOS_STATUS_E_INVAL;
       return;
    }

   /*-----------------------------------------------------------------------
    Reserve head room for DxE header, BD, and WLAN header
    -----------------------------------------------------------------------*/

    vos_pkt_reserve_head( vosDataBuff, &pucBuffPtr, 
                        ucDxEBDWLANHeaderLen );
    if ( NULL == pucBuffPtr )
    {
        TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                    "WLAN TL:No enough space in VOSS packet %p for DxE/BD/WLAN header", vosDataBuff));
       *pvosStatus = VOS_STATUS_E_INVAL;
        return;
    }
    pDxEHeader = (WLANBAL_sDXEHeaderType  *)pucBuffPtr;
    pvBDHeader = (v_PVOID_t) &pDxEHeader[1];

    /* UMA Tx acceleration is enabled. 
     * UMA would help convert frames to 802.11, fill partial BD fields and 
     * construct LLC header. To further accelerate this kind of frames,
     * HAL would attempt to reuse the BD descriptor if the BD signature 
     * matches to the saved BD descriptor.
     */
     if(pStaInfo->wSTAType == WLAN_STA_IBSS)
        ucIsUnicast = !(((tANI_U8 *)pvPeekData)[0] & 0x01);
     else
        ucIsUnicast = 1;
 
     *puFastFwdOK = (v_U32_t) WLANHAL_TxBdFastFwd(pvosGCtx, pvPeekData, pMetaInfo->ucTID, ucIsUnicast, pvBDHeader, usPktLen );
    
      /* Can't be fast forwarded. Trim the VOS head back to original location. */
      if(! *puFastFwdOK){
          vos_pkt_trim_head(vosDataBuff, ucDxEBDWLANHeaderLen);
      }else{
        /* could be fast forwarded. Now notify BAL DxE header filling could be completely skipped
         */
        v_U32_t uPacketSize = WLANTL_BD_HEADER_LEN(0) + usPktLen;
        vos_pkt_set_user_data_ptr( vosDataBuff, VOS_PKT_USER_DATA_ID_BAL, 
                       (v_PVOID_t)uPacketSize);
        pDxEHeader->size  = SWAP_ENDIAN_UINT32(uPacketSize);
      }
     *pvosStatus = VOS_STATUS_SUCCESS;
      return;
}
#endif /*WLAN_PERF*/


/*==========================================================================
  FUNCTION    WDA_DS_Register

  DESCRIPTION 
    Register TL client to WDA. This function registers TL RX/TX functions
    to WDI by calling WDI_DS_Register.


    For NON integrated SOC, this function calls WLANBAL_RegTlCbFunctions
    to register TL's RX/TX functions to BAL

  TODO 
    For Prima, pfnResourceCB gets called in WDTS_OOResourceNotification.
    The uCount parameter is AC mask. It should be redefined to use the
    same resource callback function.

  DEPENDENCIES 
     
  PARAMETERS 

   IN
        pvosGCtx    VOS context
        pfnTxCompleteCallback       TX complete callback upon TX completion
        pfnRxPacketCallback         RX callback
        pfnTxPacketCallback         TX callback
        pfnResourceCB               gets called when updating TX PDU number
        uResTheshold                minimum TX PDU size for a packet
        pCallbackContext            WDI calls callback function with it
                                    VOS global context pointer
   OUT
        uAvailableTxBuf       available TX PDU numbder. 
                              BAL returns it for NON integrated SOC
   
  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS WDA_DS_Register 
( 
  v_PVOID_t                 pvosGCtx, 
  WDA_DS_TxCompleteCallback pfnTxCompleteCallback,
  WDA_DS_RxPacketCallback   pfnRxPacketCallback, 
  WDA_DS_TxPacketCallback   pfnTxPacketCallback,
  WDA_DS_TxFlowControlCallback  pfnTxFlowCtrlCallback,
  WDA_DS_ResourceCB         pfnResourceCB,
  v_U32_t                   uResTheshold,
  v_PVOID_t                 pCallbackContext,
  v_U32_t                   *uAvailableTxBuf
)
{
  VOS_STATUS          vosStatus;
  WLANBAL_TlRegType   tlReg;

  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "WLAN WDA: WDA_DS_Register" );

  /*------------------------------------------------------------------------
    Sanity check
   ------------------------------------------------------------------------*/
  if ( ( NULL == pvosGCtx ) ||
       ( NULL == pfnTxPacketCallback ) ||
       ( NULL == pfnTxCompleteCallback ) ||
       ( NULL == pfnRxPacketCallback ) ||
       ( NULL == pfnResourceCB ) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Invalid pointers on WDA_DS_Register" );
    return VOS_STATUS_E_FAULT;
  }

  /*------------------------------------------------------------------------
    Register with BAL as transport layer client
  ------------------------------------------------------------------------*/
  tlReg.receiveFrameCB = pfnRxPacketCallback;
  tlReg.getTXFrameCB   = pfnTxPacketCallback;
  tlReg.txCompleteCB   = pfnTxCompleteCallback;
  tlReg.txResourceCB   = pfnResourceCB;
  tlReg.txResourceThreashold = uResTheshold;
  tlReg.tlUsrData      = pvosGCtx;

  vosStatus = WLANBAL_RegTlCbFunctions( pvosGCtx, &tlReg );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
               "WLAN WDA: WDA failed to register with BAL, Err: %d", vosStatus );
    return vosStatus;
  }

  /*------------------------------------------------------------------------
    Request resources for tx from bus
  ------------------------------------------------------------------------*/
  vosStatus = WLANBAL_GetTxResources( pvosGCtx, &uResCount );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:WDA failed to get resources from BAL, Err: %d",
               vosStatus );
    return vosStatus;
  }

  return vosStatus;
}


/*==========================================================================
  FUNCTION    WDA_DS_StartXmit

  DESCRIPTION 
    Serialize TX transmit reques to TX thread. 

  TODO This sends TX transmit request to TL. It should send to WDI for
         abstraction.

    For NON integrated SOC, this function calls WLANBAL_StartXmit

  DEPENDENCIES 
     
  PARAMETERS 

   IN
        pvosGCtx    VOS context
   
  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WDA_DS_StartXmit
(
  v_PVOID_t pvosGCtx
)
{
   return WLANBAL_StartXmit( pvosGCtx );
}



/*==========================================================================
   FUNCTION    WDA_DS_BuildTxPacketInfo

  DESCRIPTION
    Build TX meta info for integrated SOC.
    
    Same function calls HAL for reserve BD header space into VOS packet and
    HAL function to fill it.
    
  DEPENDENCIES

  PARAMETERS

   IN
    pvosGCtx         VOS context
    vosDataBuff      vos data buffer
    pvDestMacAdddr   destination MAC address ponter
    ucDisableFrmXtl  Is frame xtl disabled?
    ucQosEnabled     Is QoS enabled?
    ucWDSEnabled     Is WDS enabled?
    extraHeadSpace   Extra head bytes. If it's not 0 due to 4 bytes align
                     of BD header.
    typeSubtype      typeSubtype from MAC header or TX metainfo/BD
    pAddr2           address 2
    uTid             tid
    txFlag
    timeStamp
    ucIsEapol
    ucUP

   OUT
    *pusPktLen       Packet length

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_BuildTxPacketInfo
(
  v_PVOID_t       pvosGCtx,
  vos_pkt_t*      vosDataBuff,
  v_MACADDR_t*    pvDestMacAdddr,
  v_U8_t          ucDisableFrmXtl,
  v_U16_t*        pusPktLen,
  v_U8_t          ucQosEnabled,
  v_U8_t          ucWDSEnabled,
  v_U8_t          extraHeadSpace,
  v_U8_t          typeSubtype,
  v_PVOID_t       pAddr2,
  v_U8_t          uTid,
  v_U8_t          txFlag,
  v_U32_t         timeStamp,
  v_U8_t          ucIsEapol,
  v_U8_t          ucUP
)
{
  VOS_STATUS   vosStatus;
  v_PVOID_t    pvBDHeader,

  WDA_DS_PrepareBDHeader( vosDataBuff, &pvBDHeader, pvDestMacAdddr, ucDisableFrmXtl,
                  &vosStatus, pusPktLen, ucQosEnabled, ucWDSEnabled, extraHeadSpace)

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Failed while attempting to prepare BD %d", vosStatus );
    *pvosDataBuff = NULL;
    return vosStatus;
  }

  vosStatus = WLANHAL_FillTxBd( pVosGCtx, typeSubtype, pvDestMacAddr, pAddr2,
                    uTid, ucDisableFrmXtl, pvBDHeader, txFlag, timeStamp );

  if ( VOS_STATUS_SUCCESS != vosStatus )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:Failed while attempting to fill BD %d", vosStatus );
    return vosStatus;
  }


}



/*==========================================================================
   FUNCTION    WDS_DS_TrimRxPacketInfo

  DESCRIPTION
    Trim/Remove RX BD header for NON integrated SOC.
    It does nothing for integrated SOC.
    
  DEPENDENCIES

  PARAMETERS

   IN
    vosDataBuff      vos data buffer

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDS_DS_TrimRxPacketInfo
( 
  vos_pkt_t *vosDataBuff
)
{
  VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
  v_U16_t  usPktLen;
  v_U8_t   ucMPDUHOffset;
  v_U16_t  usMPDUDOffset;
  v_U16_t  usMPDULen;
  v_U8_t   ucMPDUHLen = 0;
  v_U16_t  usActualHLen = 0;
  v_U8_t   aucBDHeader[WLANHAL_RX_BD_HEADER_SIZE];

  vos_pkt_pop_head( vosDataBuff, aucBDHeader, WLANHAL_RX_BD_HEADER_SIZE);

  ucMPDUHOffset = (v_U8_t)WLANHAL_RX_BD_GET_MPDU_H_OFFSET(aucBDHeader);
  usMPDUDOffset = (v_U16_t)WLANHAL_RX_BD_GET_MPDU_D_OFFSET(aucBDHeader);
  usMPDULen     = (v_U16_t)WLANHAL_RX_BD_GET_MPDU_LEN(aucBDHeader);
  ucMPDUHLen    = (v_U8_t)WLANHAL_RX_BD_GET_MPDU_H_LEN(aucBDHeader);
  
  VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO_HIGH,
       "WLAN WDA:BD header processing data: HO %d DO %d Len %d HLen %d"
       " Tid %d BD %d",
       ucMPDUHOffset, usMPDUDOffset, usMPDULen, ucMPDUHLen,
       WLANHAL_RX_BD_HEADER_SIZE );

  vos_pkt_get_packet_length( vosTempBuff, &usPktLen);

  if (( ucMPDUHOffset >= WLANHAL_RX_BD_HEADER_SIZE ) &&
      ( usMPDUDOffset >  ucMPDUHOffset ) &&
      ( usMPDULen     >= ucMPDUHLen ) &&
      ( usPktLen >= usMPDULen ))
  {
    if((ucMPDUHOffset - WLANHAL_RX_BD_HEADER_SIZE) > 0)
    {
      vos_pkt_trim_head( vosDataBuff, ucMPDUHOffset - WLANHAL_RX_BD_HEADER_SIZE);
    }
    else
    {
      /* Nothing to trim
       * Do Nothing */
    }
    vos_pkt_trim_head( vosTempBuff, ucMPDUHOffset - WLANHAL_RX_BD_HEADER_SIZE);
    vosStatus = VOS_STATUS_SUCCESS;
  }
  else
  {
    vosStatus = VOS_STATUS_E_FAILURE;
  }

  return vosStatus;
}



/*==========================================================================
   FUNCTION    WDA_DS_PeekRxPacketInfo

  DESCRIPTION
    Return RX metainfo pointer for for integrated SOC.
    
    Same function will return BD header pointer.
    
  DEPENDENCIES

  PARAMETERS

   IN
    vosDataBuff      vos data buffer

    pvDestMacAdddr   destination MAC address ponter
    bSwap            Want to swap BD header? For backward compatability
                     It does nothing for integrated SOC
   OUT
    *ppRxHeader      RX metainfo pointer

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_PeekRxPacketInfo
(
  vos_pkt_t *vosDataBuff,
  v_PVOID_t *ppRxHeader,
  v_BOOL_t  bSwap
)
{
  VOS_STATUS vosStatus;

  vosStatus = vos_pkt_peek_data( vosDataBuff, 0, (v_PVOID_t)ppRxHeader,
                                   WLANHAL_RX_BD_HEADER_SIZE);

  if ( NULL == *ppRxHeader )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WDA :Cannot extract BD header" );
    return VOS_STATUS_E_FAILURE;
  }

  if ( VOS_TRUE == bSwap )
  {
    WLANHAL_SwapRxBd( *ppRxHeader );
  }

  return VOS_STATUS_SUCCESS;
}



/*==========================================================================
   FUNCTION    WDA_DS_GetFrameTypeSubType

  DESCRIPTION
    Get typeSubtype from the packet. The BD header should have this.
    But some reason, Libra/Volans read it from 802.11 header and save it
    back to BD header. So for NON integrated SOC, this function does
    the same.

    For integrated SOC, WDI does the same, not TL. 
    It does return typeSubtype from RX meta info for integrated SOC.

  DEPENDENCIES

  PARAMETERS

   IN
    pvosGCtx         VOS context
    vosDataBuff      vos data buffer
    pRxHeader        RX meta info or BD header pointer

   OUT
    ucTypeSubtype    typeSubtype

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_GetFrameTypeSubType
(
  v_PVOID_t pvosGCtx,
  vos_pkt_t *vosDataBuff,
  v_PVOID_t pRxHeader,
  v_U8_t    *ucTypeSubtype
)
{
{
  v_PVOID_t           pvBDHeader = pRxHeader;
  v_U16_t             usFrmCtrl  = 0; 
  v_SIZE_t            usFrmCtrlSize = sizeof(usFrmCtrl); 

  /*---------------------------------------------------------------------
    Extract frame control field from 802.11 header if present 
    (frame translation not done) 
  ---------------------------------------------------------------------*/
  vosStatus = vos_pkt_extract_data( vosDataBuff, 
                       ( 0 == WLANHAL_RX_BD_GET_FT(pvBDHeader) ) ?
                       WLANHAL_RX_BD_GET_MPDU_H_OFFSET(pvBDHeader):
                       WLANHAL_RX_BD_HEADER_SIZE,
                       &usFrmCtrl, &usFrmCtrlSize );

  if (( VOS_STATUS_SUCCESS != vosStatus ) || 
      ( sizeof(usFrmCtrl) != usFrmCtrlSize ))
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WLAN WDA:Cannot extract Frame Control Field" );
    return VOS_STATUS_E_FAILURE;
  }


  ucFrmType = (v_U8_t)WLANHAL_RxBD_GetFrameTypeSubType( pvBDHeader, 
                                                        usFrmCtrl);
  WLANHAL_RX_BD_SET_TYPE_SUBTYPE(pvBDHeader, ucFrmType);

  *ucTypeSubtype = ucFrmType;
}



/*==========================================================================
   FUNCTION    WDA_DS_RxAmsduBdFix

  DESCRIPTION
    For backward compatability with Libra/Volans. Need to call HAL function
    for HW BD bug fix

    It does nothing for integrated SOC.

  DEPENDENCIES

  PARAMETERS

   IN
    pvosGCtx         VOS context
    vosDataBuff      vos data buffer

   OUT

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_RxAmsduBdFix
(
  v_PVOID_t pvosGCtx,
  vos_pkt_t *vosDataBuff
)
{
  /* AMSDU HW bug fix
   * After 2nd AMSDU subframe HW could not handle BD correctly
   * HAL workaround is needed */
  WLANHAL_RxAmsduBdFix(pvosGCtx, pvBDHeader);
  return VOS_STATUS_SUCCESS;
}


/*==========================================================================
   FUNCTION    WDA_DS_GetRssi

  DESCRIPTION
    Get RSSI 

  TODO It returns hardcoded value in the meantime since WDA/WDI does nothing
       support it yet for Prima.

  DEPENDENCIES

  PARAMETERS

   IN
    vosDataBuff      vos data buffer

   OUT
    puRssi           RSSI

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_GetRssi
(
  v_PVOID_t pvosGCtx,
  v_S7_t*   puRssi
)
{
  halPS_GetRssi(vos_get_context(VOS_MODULE_ID_SME, pvosGCtx), puRssi);
  return VOS_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    WDA_DS_GetTxResources

  DESCRIPTION
    It does return hardcoded value for Prima. It should bigger number than 0.
    Returning 0 will put TL in out-of-resource condition for TX.

    Return current PDU resources from BAL for NON integrated SOC.
    
  DEPENDENCIES

  PARAMETERS

   IN
    vosDataBuff      vos data buffer
   
   OUT
    puResCount        available PDU number for TX

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/
VOS_STATUS
WDA_DS_GetTxResources
( 
  v_PVOID_t pvosGCtx,
  v_U32_t*  puResCount
)
{
  return WLANBAL_GetTxResources( pvosGCtx, puResCount );
}



/*==========================================================================
   FUNCTION    WDA_DS_GetReplayCounter

  DESCRIPTION
    Return replay counter from BD header or RX meta info

  DEPENDENCIES

  PARAMETERS

   IN
    pRxHeader        RX meta info or BD header pointer

   OUT

  RETURN VALUE
    Replay Counter

  SIDE EFFECTS

============================================================================*/
v_U64_t
WDA_DS_GetReplayCounter
(
  v_PVOID_t pRxHeader
)
{
   v_U8_t *pucRxBDHeader = pRxHeader;

/* 48-bit replay counter is created as follows
   from RX BD 6 byte PMI command:
   Addr : AES/TKIP
   0x38 : pn3/tsc3
   0x39 : pn2/tsc2
   0x3a : pn1/tsc1
   0x3b : pn0/tsc0

   0x3c : pn5/tsc5
   0x3d : pn4/tsc4 */

#ifdef ANI_BIG_BYTE_ENDIAN
    v_U64_t ullcurrentReplayCounter = 0;
    /* Getting 48-bit replay counter from the RX BD */
    ullcurrentReplayCounter = WLANHAL_RX_BD_GET_PMICMD_20TO23(pucRxBDHeader); 
    ullcurrentReplayCounter <<= 16;
    ullcurrentReplayCounter |= (( WLANHAL_RX_BD_GET_PMICMD_24TO25(pucRxBDHeader) & 0xFFFF0000) >> 16);
    return ullcurrentReplayCounter;
#else
    v_U64_t ullcurrentReplayCounter = 0;
    /* Getting 48-bit replay counter from the RX BD */
    ullcurrentReplayCounter = (WLANHAL_RX_BD_GET_PMICMD_24TO25(pucRxBDHeader) & 0x0000FFFF); 
    ullcurrentReplayCounter <<= 32; 
    ullcurrentReplayCounter |= WLANHAL_RX_BD_GET_PMICMD_20TO23(pucRxBDHeader); 
    return ullcurrentReplayCounter;
#endif
}



/*==========================================================================
   FUNCTION    WDA_DS_TxFrames

  DESCRIPTION
    Pull packets from TL and push them to WDI. It gets invoked upon
    WDA_DS_TX_START_XMIT.

    This function is equivelant of WLANSSC_Transmit in Libra/Volans.

  TODO
    This function should be implemented and moved in WDI.

  DEPENDENCIES

  PARAMETERS

   IN
    pvosGCtx         VOS context

   OUT

  RETURN VALUE
    VOS_STATUS_E_FAULT:  pointer is NULL and other errors 
    VOS_STATUS_SUCCESS:  Everything is good :)

  SIDE EFFECTS

============================================================================*/

VOS_STATUS
WDA_DS_TxFrames
( 
  v_PVOID_t pvosGCtx 
)
{
  VOS_STATUS vosStatus;
  vos_pkt_t  *pTxChain = NULL;
  v_BOOL_t   bUrgent;
  WDI_Status wdiStatus;
  tWDA_CbContext *wdaContext = NULL;

  wdaContext = (tWDA_CbContext *)vos_get_context(VOS_MODULE_ID_WDA, pvosGCtx);
  if ( NULL == wdaContext )
  {
    VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "WDA:Invalid wda context pointer from pvosGCtx on WDA_DS_TxFrames" );
    return VOS_STATUS_E_FAULT;
  }

  do
  {
    if ( VOS_FALSE == WLANTL_GetFrames( pvosGCtx, &pTxChain, 
                       5000/*FIXME resource size 5000 is large enough*/, &bUrgent ) )
    {
      // No more data frame
      vosStatus = VOS_STATUS_SUCCESS;
      break;
    }
      
    wdiStatus = WDI_DS_TxPacket( wdaContext->pWdiContext, 
                                 pTxChain, 0 );
    if ( WDI_STATUS_SUCCESS != wdiStatus )
    {
      VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                   "WLAN TL: Pushing a FC packet to DAL failed.");
      vosStatus = VOS_STATUS_E_FAILURE;
      break;
    }
  } while ( 1 );

  return vosStatus;
}

/*==========================================================================
   FUNCTION    WDA_DS_PrepareBDHeader

  DESCRIPTION
    Inline function for preparing BD header before HAL processing.

  DEPENDENCIES
    Just notify HAL that suspend in TL is complete.

  PARAMETERS

   IN
    vosDataBuff:      vos data buffer
    ucDisableFrmXtl:  is frame xtl disabled

   OUT
    ppvBDHeader:      it will contain the BD header
    pvDestMacAdddr:   it will contain the destination MAC address
    pvosStatus:       status of the combined processing
    pusPktLen:        packet len.

  RETURN VALUE
    No return.

  SIDE EFFECTS

============================================================================*/
void
WDA_DS_PrepareBDHeader
(
  vos_pkt_t*      vosDataBuff,
  v_PVOID_t*      ppvBDHeader,
  v_MACADDR_t*    pvDestMacAdddr,
  v_U8_t          ucDisableFrmXtl,
  VOS_STATUS*     pvosStatus,
  v_U16_t*        pusPktLen,
  v_U8_t          ucQosEnabled,
  v_U8_t          ucWDSEnabled,
  v_U8_t          extraHeadSpace
)
{
  v_U8_t      ucHeaderOffset;
  v_U8_t      ucHeaderLen;
#ifndef WLAN_SOFTAP_FEATURE
  v_PVOID_t   pvPeekData;
#endif
  v_U8_t      ucBDHeaderLen = WLANTL_BD_HEADER_LEN(ucDisableFrmXtl);

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /*-------------------------------------------------------------------------
    Get header pointer from VOSS
    !!! make sure reserve head zeros out the memory
   -------------------------------------------------------------------------*/
  vos_pkt_get_packet_length( vosDataBuff, pusPktLen);

  if ( WLANTL_MAC_HEADER_LEN(ucDisableFrmXtl) > *pusPktLen )
  {
    TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL: Length of the packet smaller than expected network"
               " header %d", *pusPktLen ));

    *pvosStatus = VOS_STATUS_E_INVAL;
    return;
  }

  vos_pkt_reserve_head( vosDataBuff, ppvBDHeader,
                        ucBDHeaderLen );
  if ( NULL == *ppvBDHeader )
  {
    TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                "WLAN TL:VOSS packet corrupted on Attach BD header"));
    *pvosStatus = VOS_STATUS_E_INVAL;
    return;
  }

  /*-----------------------------------------------------------------------
    Extract MAC address
   -----------------------------------------------------------------------*/
#ifdef WLAN_SOFTAP_FEATURE
  {
   v_SIZE_t usMacAddrSize = VOS_MAC_ADDR_SIZE;
   *pvosStatus = vos_pkt_extract_data( vosDataBuff,
                                     ucBDHeaderLen +
                                     WLANTL_MAC_ADDR_ALIGN(ucDisableFrmXtl),
                                     (v_PVOID_t)pvDestMacAdddr,
                                     &usMacAddrSize );
  }
#else
  *pvosStatus = vos_pkt_peek_data( vosDataBuff,
                                     ucBDHeaderLen +
                                     WLANTL_MAC_ADDR_ALIGN(ucDisableFrmXtl),
                                     (v_PVOID_t)&pvPeekData,
                                     VOS_MAC_ADDR_SIZE );

  /*Fix me*/
  vos_copy_macaddr(pvDestMacAdddr, (v_MACADDR_t*)pvPeekData);
#endif
  if ( VOS_STATUS_SUCCESS != *pvosStatus )
  {
     TLLOGE(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
                "WLAN TL:Failed while attempting to extract MAC Addr %d",
                *pvosStatus));
  }
  else
  {
    /*---------------------------------------------------------------------
        Fill MPDU info fields:
          - MPDU data start offset
          - MPDU header start offset
          - MPDU header length
          - MPDU length - this is a 16b field - needs swapping
    --------------------------------------------------------------------*/
    ucHeaderOffset = ucBDHeaderLen;
    ucHeaderLen    = WLANTL_MAC_HEADER_LEN(ucDisableFrmXtl);

    if ( 0 != ucDisableFrmXtl )
    {
      if ( 0 != ucQosEnabled )
      {
        ucHeaderLen += WLANTL_802_11_HEADER_QOS_CTL;
      }

      // Similar to Qos we need something for WDS format !
      if ( ucWDSEnabled != 0 )
      {
        // If we have frame translation enabled
        ucHeaderLen    += WLANTL_802_11_HEADER_ADDR4_LEN;
      }
      if ( extraHeadSpace != 0 )
      {
        // Decrease the packet length with the extra padding after the header
        *pusPktLen = *pusPktLen - extraHeadSpace;
      }
    }

    WLANHAL_TX_BD_SET_MPDU_HEADER_LEN( *ppvBDHeader, ucHeaderLen);
    WLANHAL_TX_BD_SET_MPDU_HEADER_OFFSET( *ppvBDHeader, ucHeaderOffset);
    WLANHAL_TX_BD_SET_MPDU_DATA_OFFSET( *ppvBDHeader,
                                          ucHeaderOffset + ucHeaderLen + extraHeadSpace);
    WLANHAL_TX_BD_SET_MPDU_LEN( *ppvBDHeader, *pusPktLen);

    TLLOG2(VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
                "WLAN TL: VALUES ARE HLen=%x Hoff=%x doff=%x len=%x ex=%d",
                ucHeaderLen, ucHeaderOffset, 
		(ucHeaderOffset + ucHeaderLen + extraHeadSpace), 
		*pusPktLen, extraHeadSpace));
  }/* if peek MAC success*/

}/* WLANTL_PrepareBDHeader */
