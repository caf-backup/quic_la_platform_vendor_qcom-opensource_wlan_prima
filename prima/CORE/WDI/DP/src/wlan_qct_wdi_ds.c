/**=========================================================================
 *     
 *       \file  wlan_qct_wdi_ds.c
 *          
 *       \brief define Dataservice API 
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
#include "wlan_qct_wdi_i.h"
#include "wlan_qct_wdi_ds.h"
#include "wlan_qct_wdi_ds_i.h"
#include "wlan_qct_wdi_dts.h"
#include "wlan_qct_wdi_dp.h"




/* DAL registration function. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along 
 *  with the callback.
 *  pfnTxCompleteCallback:Callback function that is to be invoked to return 
 *  packets which have been transmitted.
 *  pfnRxPacketCallback:Callback function that is to be invoked to deliver 
 *  packets which have been received
 *  pfnTxFlowControlCallback:Callback function that is to be invoked to 
 *  indicate/clear congestion. 
 *
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
WDI_Status WDI_DS_Register( void *pContext, 
  WDI_DS_TxCompleteCallback pfnTxCompleteCallback,
  WDI_DS_RxPacketCallback pfnRxPacketCallback, 
  WDI_DS_TxFlowControlCallback pfnTxFlowControlCallback,
  void *pCallbackContext)
{
  WDI_DS_ClientDataType *pClientData = 
    (WDI_DS_ClientDataType *)WDI_DS_GetDatapathContext(pContext);
  WDI_Status sWdiStatus = WDI_STATUS_SUCCESS;

  if(!pClientData)
    return WDI_STATUS_MEM_FAILURE;

  // Do Sanity checks
  if(NULL == pContext ||
     NULL == pCallbackContext || 
     NULL == pfnTxCompleteCallback || 
     NULL == pfnRxPacketCallback || 
     NULL == pfnTxFlowControlCallback) {
    return WDI_STATUS_E_FAILURE;
  }
  

  // Store callbacks in client structure
  pClientData->pcontext = pContext;
  pClientData->receiveFrameCB = pfnRxPacketCallback;
  pClientData->txCompleteCB = pfnTxCompleteCallback;
  pClientData->txResourceCB = pfnTxFlowControlCallback;
  pClientData->pCallbackContext = pCallbackContext;
    
  // rerurn status
  return sWdiStatus;  
}



/* DAL Transmit function. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along with the callback.
 *  pFrame:Refernce to PAL frame.
 *  more: Does the invokee have more than one packet pending?
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */


WDI_Status WDI_DS_TxPacket(void *pContext,
  wpt_packet *pFrame,
  wpt_boolean more)
{
  WDI_DS_ClientDataType *pClientData =  
    (WDI_DS_ClientDataType *) WDI_DS_GetDatapathContext(pContext);
  wpt_uint8      ucSwFrameTXXlation;
  wpt_uint8      ucUP;
  wpt_uint8      ucTypeSubtype;
  wpt_uint8      alignment;
  wpt_uint8      ucTxFlag;
  wpt_uint8*     pSTAMACAddress;
  wpt_uint8*     pAddr2MACAddress;
  WDI_DS_TxMetaInfoType     *pTxMetadata;
  void *physBDHeader, *pvBDHeader;
  wpt_uint8      ucType;
  WDI_DS_BdMemPoolType *pMemPool;
  wpt_uint8      ucBdPoolType;

  // Do Sanity checks
  if(NULL == pContext || pClientData->suspend){
    return WDI_STATUS_E_FAILURE;
  }

  // extract metadata from PAL packet
  pTxMetadata = WDI_DS_ExtractTxMetaData(pFrame);
  ucSwFrameTXXlation = pTxMetadata->fdisableFrmXlt;
  ucTypeSubtype = pTxMetadata->typeSubtype;
  ucUP = pTxMetadata->fUP;
  ucTxFlag = pTxMetadata->txFlags;
  pSTAMACAddress = &(pTxMetadata->fSTAMACAddress[0]);
  pAddr2MACAddress = &(pTxMetadata->addr2MACAddress[0]);

  /*------------------------------------------------------------------------
     Get type and subtype of the frame first 
  ------------------------------------------------------------------------*/
  ucType = (ucTypeSubtype & WDI_FRAME_TYPE_MASK) >> WDI_FRAME_TYPE_OFFSET;
  switch(ucType)
  {
    case WDI_MAC_DATA_FRAME:
       pMemPool = &(pClientData->dataMemPool);
       ucBdPoolType = WDI_DATA_POOL_ID;
    break;
    case WDI_MAC_MGMT_FRAME:
       pMemPool = &(pClientData->mgmtMemPool);
       ucBdPoolType = WDI_MGMT_POOL_ID;
    break;
    default:
      return WDI_STATUS_E_FAILURE;;
  }

  // Allocate BD header from pool
  pvBDHeader = WDI_DS_MemPoolAlloc(pMemPool, &physBDHeader, ucBdPoolType);
  if(NULL == pvBDHeader)
    return WDI_STATUS_E_FAILURE;
      
  WDI_SetBDPointers(pFrame, pvBDHeader, physBDHeader);

  alignment = 0;
  WDI_DS_PrepareBDHeader(pFrame, ucSwFrameTXXlation, alignment);

  if(WDI_STATUS_SUCCESS != 
      WDI_FillTxBd( pContext, ucTypeSubtype, pSTAMACAddress, pAddr2MACAddress, 
        &ucUP, 1, pvBDHeader, ucTxFlag /* No ACK */, 0 )){
    WDI_DS_MemPoolFree(pMemPool, pvBDHeader, physBDHeader);
    return WDI_STATUS_E_FAILURE;
  }
  // Send packet to transport layer.
  if(eWLAN_PAL_STATUS_SUCCESS !=WDTS_TxPacket(pContext, pFrame)){
    WDI_DS_MemPoolFree(pMemPool, pvBDHeader, physBDHeader);
    return WDI_STATUS_E_FAILURE;
  }  

  return WDI_STATUS_SUCCESS;  
}
 
 
/* DAL Transmit Complete function. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along with the callback.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */


WDI_Status WDI_DS_TxComplete(void *pContext)
{
  // Do Sanity checks
  if(NULL == pContext)
    return WDI_STATUS_E_FAILURE;
  
  // Send notification to transport layer.
  if(eWLAN_PAL_STATUS_SUCCESS !=WDTS_CompleteTx(pContext))
  {
    return WDI_STATUS_E_FAILURE;
  }  

  return WDI_STATUS_SUCCESS;  
} 

/* DAL Suspend Transmit function. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along with the callback.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */


WDI_Status WDI_DS_TxSuspend(void *pContext)
{
  WDI_DS_ClientDataType *pClientData =  
    (WDI_DS_ClientDataType *) WDI_DS_GetDatapathContext(pContext);
  pClientData->suspend = 1;

  return WDI_STATUS_SUCCESS;  

}


/* DAL Resume Transmit function. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along with the callback.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */


WDI_Status WDI_DS_TxResume(void *pContext)
{
  WDI_DS_ClientDataType *pClientData =  
    (WDI_DS_ClientDataType *) WDI_DS_GetDatapathContext(pContext);

  pClientData->suspend = 0;

  return WDI_STATUS_SUCCESS;  
}

/* DAL Get Available Resource Count. 
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along with the callback.
 *  wdiResPool: - identifier of resource pool
 * Return Value: number of resources available
 *
 */

wpt_uint32 WDI_GetAvailableResCount(void *pContext,WDI_ResPoolType wdiResPool)
{
  WDI_DS_ClientDataType *pClientData =  
    (WDI_DS_ClientDataType *) WDI_DS_GetDatapathContext(pContext);
  switch(wdiResPool)
  {
    case WDI_MGMT_POOL_ID:
      return (WDI_DS_HI_PRI_RES_NUM - WDI_DS_GetAvailableResCount(&pClientData->mgmtMemPool));
    case WDI_DATA_POOL_ID:
      return (WDI_DS_LO_PRI_RES_NUM - WDI_DS_GetAvailableResCount(&pClientData->dataMemPool));
    default:
      return 0;
  }
}
