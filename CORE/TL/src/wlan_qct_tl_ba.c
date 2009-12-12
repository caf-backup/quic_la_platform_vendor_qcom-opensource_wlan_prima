/*===========================================================================

                       W L A N _ Q C T _ T L _ B A. C
                                               
  OVERVIEW:
  
  This software unit holds the implementation of the WLAN Transport Layer
  Block Ack session support. Also included are the AMSDU de-aggregation 
  completion and MSDU re-ordering functionality. 
  
  The functions externalized by this module are to be called ONLY by the main
  TL module or the HAL layer.

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


   $Header$$DateTime$$Author$


  when        who     what, where, why
----------    ---    --------------------------------------------------------
2008-08-22    sch     Update based on unit test
2008-07-31    lti     Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_tl.h" 
#include "wlan_qct_tli.h" 
#include "wlan_qct_tli_ba.h" 
#include "wlan_qct_hal.h" 
#include "vos_list.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
//#define WLANTL_REORDER_DEBUG_MSG_ENABLE
#define WLANTL_BA_REORDERING_AGING_TIMER   30   /* 30 millisec */

#define BAMSGERROR(a, b, c, d)                                           \
        VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR, a, b, c, d)
#ifdef WLANTL_REORDER_DEBUG_MSG_ENABLE
#define BAMSGDEBUG(a, b, c, d)                                           \
        VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO, a, b, c, d)
#else
#define BAMSGDEBUG(a, b, c, d)          {}
#endif /* WLANTL_REORDER_DEBUG_MSG_ENABLE */

#ifdef WLANTL_REORDER_DEBUG_MSG_ENABLE
static char *opCodeSting[] = {
                                "INVALID, Just forward CUR Frame",
                                "QCU_FWBF, Q cur frame and FWD buffered frames",
                                "FWBF_FWCU, FWD buffered frames and cur frame",
                                "QCUR, Just put in cur frame into Q",
                                "FWBF_QCU, FWD buffered frames and input cur frame",
                                "FWBF_DCU, BAR frame, FWD buffered frames and drop BAR",
                                "FWA_DCU, FWD all buffered frames and drop cur BAR",
                                "FWA_QCU, FWD all buffered frames and Q cur",
                                "DCU, Drop cur frame",
                             };
#endif /* WLANTL_REORDER_DEBUG_MSG_ENABLE */

/*==========================================================================

   FUNCTION    tlReorderingAgingTimerExpierCB

   DESCRIPTION 
      After aging timer expiered, all Qed frames have to be routed to upper
      layer. Otherwise, there is possibilitied that ahng some frames
    
   PARAMETERS 
      v_PVOID_t  timerUdata    Timer callback user data
                               Has information about where frames should be
                               routed
   
   RETURN VALUE
      VOS_STATUS_SUCCESS       General success
      VOS_STATUS_E_INVAL       Invalid frame handle
  
============================================================================*/
v_VOID_t WLANTL_ReorderingAgingTimerExpierCB
(
   v_PVOID_t  timerUdata
)
{
   WLANTL_TIMER_EXPIER_UDATA_T *expireHanlde;
   WLANTL_BAReorderType        *ReorderInfo;
   WLANTL_CbType               *pTLHandle;
   vos_pkt_t                   *vosDataBuff;
   VOS_STATUS                   status = VOS_STATUS_SUCCESS;
   v_U8_t                       ucSTAID;
   v_U8_t                       ucTID;
   v_U8_t                       opCode;
   WLANTL_RxMetaInfoType        wRxMetaInfo;
   v_U32_t                      fwIdx = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   if(NULL == timerUdata)
   {
      BAMSGERROR("Timer Callback User data NULL", 0, 0, 0);
      return;
   }

   expireHanlde = (WLANTL_TIMER_EXPIER_UDATA_T *)timerUdata;
   ucSTAID      = (v_U8_t)expireHanlde->STAID;
   ucTID        = expireHanlde->TID;
   if(WLANTL_STA_ID_INVALID(ucSTAID) || WLANTL_TID_INVALID(ucTID))
   {
      BAMSGERROR("SID %d or TID %d is not valid",
                  ucSTAID, ucTID, 0);
      return;
   }

   pTLHandle    = (WLANTL_CbType *)expireHanlde->pTLHandle;
   if(NULL == pTLHandle)
   {
      BAMSGERROR("TL Controll block NULL", 0, 0, 0);
      return;
   }

   ReorderInfo = &pTLHandle->atlSTAClients[ucSTAID].atlBAReorderInfo[ucTID];
   if(NULL == ReorderInfo)
   {
      BAMSGERROR("Reorder data NULL, this could not happen SID %d, TID %d", 
                  ucSTAID, ucTID, 0);
      return;
   }

   opCode   = WLANTL_OPCODE_FWDALL_DROPCUR;
   vosDataBuff = NULL;
   ReorderInfo->timerStarted = VOS_FALSE;
   if(ReorderInfo->pendingFramesCount < 1)
   {
      BAMSGERROR("No pending frames, why triggered timer?", 0, 0, 0);
      return;
   }
   else
   {
      BAMSGERROR("There are %d pending frames, handle it %d", ReorderInfo->pendingFramesCount, 0, 0);
   }

   if(0 == ReorderInfo->ucCIndex)
   {
      fwIdx = ReorderInfo->winSize;
   }
   else
   {
      fwIdx = ReorderInfo->ucCIndex - 1;
   }

   status = WLANTL_ChainFrontPkts(fwIdx, opCode, 
                                  &vosDataBuff, ReorderInfo, NULL);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      BAMSGERROR("Make packet chain fail with Qed frames %d", status, 0, 0);
      return;
   }

   if(NULL == pTLHandle->atlSTAClients[ucSTAID].pfnSTARx)
   {
      BAMSGERROR("Callback function NULL with STAID %d", ucSTAID, 0, 0);
      return;
   }

   if(NULL == vosDataBuff)
   {
      BAMSGERROR("No pending frames, why triggered timer? ", 0, 0, 0);
      return;
   }

   wRxMetaInfo.ucUP = ucTID;
   pTLHandle->atlSTAClients[ucSTAID].pfnSTARx(expireHanlde->pAdapter,
                                            vosDataBuff, ucSTAID, &wRxMetaInfo);
   return;
}/*WLANTL_ReorderingAgingTimerExpierCB*/

/*==========================================================================

   FUNCTION    tlReorderDetectHole

   DESCRIPTION 
      Detect hole within frame Q
      If there is a hole, start aging timer
      If there is no hole, stop timer
    
   PARAMETERS 
      WLANTL_BAReorderType  *pwBaReorder
         Reorder information for this BA session
         Q and window timer context is here

      vos_pkt_t             *vosDataBuff
         New frame received
   
   RETURN VALUE
  
============================================================================*/
VOS_STATUS WLANTL_ReorderDetectHole
(
   WLANTL_BAReorderType  *pwBaReorder
)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   if((pwBaReorder->pendingFramesCount > 0) &&
      (VOS_FALSE == pwBaReorder->timerStarted))
   {
      BAMSGDEBUG("There is a new HOLE, Pending Frames Count %d",
                  pwBaReorder->pendingFramesCount, 0, 0);
      status = vos_timer_start(&pwBaReorder->agingTimer,
                               WLANTL_BA_REORDERING_AGING_TIMER);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BAMSGERROR("Timer start fail", status, 0, 0);
         return status;
      }
      pwBaReorder->timerStarted = VOS_TRUE;
   }
   else if((pwBaReorder->pendingFramesCount > 0) &&
           (VOS_TRUE == pwBaReorder->timerStarted))
   {
      BAMSGDEBUG("Still HOLE, Pending Frames Count %d",
                  pwBaReorder->pendingFramesCount, 0, 0);
   }
   else if((pwBaReorder->pendingFramesCount == 0) &&
           (VOS_TRUE == pwBaReorder->timerStarted))
   {
      BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
      status = vos_timer_stop(&pwBaReorder->agingTimer);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         BAMSGERROR("Timer stop fail", status, 0, 0);
         return status;
      }
      pwBaReorder->timerStarted = VOS_FALSE;
   }

   return status;
}

/*----------------------------------------------------------------------------
    INTERACTION WITH TL Main
 ---------------------------------------------------------------------------*/
/*==========================================================================

   FUNCTION    WLANTL_InitBAReorderBuffer

   DESCRIPTION 
      Init Reorder buffer array
    
   PARAMETERS 
      v_PVOID_t   pvosGCtx Global context

   RETURN VALUE
      NONE
  
============================================================================*/

void WLANTL_InitBAReorderBuffer
(
   v_PVOID_t   pvosGCtx
)
{
   WLANTL_CbType        *pTLCb = NULL; 
   v_U32_t              idx;
   v_U32_t              pIdx;

   pTLCb = VOS_GET_TL_CB(pvosGCtx);
   for(idx = 0; idx < WLANTL_MAX_BA_SESSION; idx++)
   {
      pTLCb->reorderBufferPool[idx].isAvailable = VOS_TRUE;
      for(pIdx = 0; pIdx < WLANTL_MAX_WINSIZE; pIdx++)
      {
         pTLCb->reorderBufferPool[idx].arrayBuffer[pIdx] = NULL;
      }
   }

   BAMSGDEBUG("BA reorder buffer init", 0, 0, 0);
   return;
}

/*==========================================================================

  FUNCTION    WLANTL_BaSessionAdd

  DESCRIPTION 
    HAL notifies TL when a new Block Ack session is being added. 
    
  DEPENDENCIES 
    A BA session on Rx needs to be added in TL before the response is 
    being sent out 
    
  PARAMETERS 

    IN
    pvosGCtx:       pointer to the global vos context; a handle to TL's 
                    control block can be extracted from its context 
    ucSTAId:        identifier of the station for which requested the BA 
                    session
    ucTid:          Tspec ID for the new BA session
    uSize:          size of the reordering window

   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_INVAL:      Input parameters are invalid 
    VOS_STATUS_E_FAULT:      Station ID is outside array boundaries or pointer 
                             to TL cb is NULL ; access would cause a page fault  
    VOS_STATUS_E_EXISTS:     Station was not registered or BA session already
                             exists
    VOS_STATUS_E_NOSUPPORT:  Not yet supported
    
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_BaSessionAdd 
( 
  v_PVOID_t   pvosGCtx, 
  v_U16_t     sessionID,
  v_U32_t     ucSTAId,
  v_U8_t      ucTid, 
  v_U32_t     uBufferSize,
  v_U32_t     winSize,
  v_U32_t     SSN
)
{
  WLANTL_CbType        *pTLCb = NULL; 
  WLANTL_BAReorderType *reorderInfo;
  v_U32_t               idx;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*------------------------------------------------------------------------
    Sanity check
   ------------------------------------------------------------------------*/
  if ( WLANTL_TID_INVALID(ucTid))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:Invalid parameter sent on WLANTL_BaSessionAdd");
    return VOS_STATUS_E_INVAL;
  }

  if ( WLANTL_STA_ID_INVALID( ucSTAId ) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:Invalid station id requested on WLANTL_BaSessionAdd");
    return VOS_STATUS_E_FAULT;
  }

  /*------------------------------------------------------------------------
    Extract TL control block and check existance
   ------------------------------------------------------------------------*/
  pTLCb = VOS_GET_TL_CB(pvosGCtx);
  if ( NULL == pTLCb ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
          "WLAN TL:Invalid TL pointer from pvosGCtx on WLANTL_BaSessionAdd");
    return VOS_STATUS_E_FAULT;
  }

  if ( 0 == pTLCb->atlSTAClients[ucSTAId].ucExists ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
          "WLAN TL:Station was not yet registered on WLANTL_BaSessionAdd");
    return VOS_STATUS_E_EXISTS;
  }

  /*------------------------------------------------------------------------
    Verify that BA session was not already added
   ------------------------------------------------------------------------*/
  if ( 0 != pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists ) 
  {
    pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists++;
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:BA session already exists on WLANTL_BaSessionAdd");
    return VOS_STATUS_E_EXISTS;
  }

  /*------------------------------------------------------------------------
    Initialize new BA session 
   ------------------------------------------------------------------------*/
  reorderInfo = &pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid];

  for(idx = 0; idx < WLANTL_MAX_BA_SESSION; idx++)
  {
    if(VOS_TRUE == pTLCb->reorderBufferPool[idx].isAvailable)
    {
      pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].reorderBuffer =
                                            &(pTLCb->reorderBufferPool[idx]);
      pTLCb->reorderBufferPool[idx].isAvailable = VOS_FALSE;
      BAMSGDEBUG("%dth buffer avaialable, buffer PTR 0x%p",
                  idx,
                  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].reorderBuffer,
                  0);
      break;
    }
  }

  reorderInfo->timerUdata.pAdapter     = pvosGCtx;
  reorderInfo->timerUdata.pTLHandle    = (v_PVOID_t)pTLCb;
  reorderInfo->timerUdata.STAID        = ucSTAId;
  reorderInfo->timerUdata.TID          = ucTid;
  reorderInfo->timerStarted = VOS_FALSE;
  vos_timer_init(&reorderInfo->agingTimer,
                 VOS_TIMER_TYPE_SW,
                 WLANTL_ReorderingAgingTimerExpierCB,
                 (v_PVOID_t)(&reorderInfo->timerUdata));

  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists++;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].usCount   = 0;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucCIndex  = 0;
  if(0 == winSize)
  {
    pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].winSize =
                                                   WLANTL_MAX_WINSIZE;
  }
  else
  {
    pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].winSize   = winSize;
  }
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].SSN       = SSN;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].sessionID = sessionID;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].pendingFramesCount = 0;
  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
             "WLAN TL:New BA session added for STA: %d TID: %d",
             ucSTAId, ucTid);

  return VOS_STATUS_SUCCESS;
}/* WLANTL_BaSessionAdd */

/*==========================================================================

  FUNCTION    WLANTL_BaSessionDel

  DESCRIPTION 
    HAL notifies TL when a new Block Ack session is being deleted. 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pvosGCtx:       pointer to the global vos context; a handle to TL's 
                    control block can be extracted from its context 
    ucSTAId:        identifier of the station for which requested the BA 
                    session
    ucTid:          Tspec ID for the new BA session
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_INVAL:      Input parameters are invalid 
    VOS_STATUS_E_FAULT:      Station ID is outside array boundaries or pointer 
                             to TL cb is NULL ; access would cause a page fault  
    VOS_STATUS_E_EXISTS:     Station was not registered or BA session already
                             exists
    VOS_STATUS_E_NOSUPPORT:  Not yet supported
    
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_BaSessionDel 
( 
  v_PVOID_t      pvosGCtx, 
  v_U16_t        ucSTAId,
  v_U8_t         ucTid
)
{
  WLANTL_CbType*          pTLCb       = NULL; 
  vos_pkt_t*              vosDataBuff = NULL;
  VOS_STATUS              vosStatus   = VOS_STATUS_E_FAILURE;
  WLANTL_BAReorderType*   reOrderInfo = NULL;
  WLANTL_RxMetaInfoType   wRxMetaInfo;
  v_U32_t                 fwIdx = 0;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
    Sanity check
   ------------------------------------------------------------------------*/
  if ( WLANTL_TID_INVALID(ucTid))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:Invalid parameter sent on WLANTL_BaSessionDel");
    return VOS_STATUS_E_INVAL;
  }

   if ( WLANTL_STA_ID_INVALID( ucSTAId ) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:Invalid station id requested on WLANTL_BaSessionDel");
    return VOS_STATUS_E_FAULT;
  }

  /*------------------------------------------------------------------------
    Extract TL control block and check existance
   ------------------------------------------------------------------------*/
  pTLCb = VOS_GET_TL_CB(pvosGCtx);
  if ( NULL == pTLCb ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
          "WLAN TL:Invalid TL pointer from pvosGCtx on WLANTL_BaSessionDel");
    return VOS_STATUS_E_FAULT;
  }

  if (( 0 == pTLCb->atlSTAClients[ucSTAId].ucExists ) &&
      ( 0 == pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists ))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
          "WLAN TL:Station was not yet registered on WLANTL_BaSessionDel");
    return VOS_STATUS_E_EXISTS;
  }
  else if(( 0 == pTLCb->atlSTAClients[ucSTAId].ucExists ) &&
          ( 0 != pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists ))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
          "STA was deleted but BA info is still there, just remove BA info");

    reOrderInfo = &pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid];
    reOrderInfo->reorderBuffer->isAvailable = VOS_TRUE;
    memset(&reOrderInfo->reorderBuffer->arrayBuffer[0],
           0,
           WLANTL_MAX_WINSIZE * sizeof(v_PVOID_t));
    vos_timer_destroy(&reOrderInfo->agingTimer);
    memset(reOrderInfo, 0, sizeof(WLANTL_BAReorderType));

    return VOS_STATUS_SUCCESS;
  }

  /*------------------------------------------------------------------------
    Verify that BA session was added
   ------------------------------------------------------------------------*/
  if ( 0 == pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:BA session does not exists on WLANTL_BaSessionDel");
    return VOS_STATUS_E_EXISTS;
  }

  
  /*------------------------------------------------------------------------
     Send all pending packets to HDD 
   ------------------------------------------------------------------------*/
  reOrderInfo = &pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid];

  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
               "WLAN TL: Fwd all packets to HDD on WLANTL_BaSessionDel");

  if(0 == reOrderInfo->ucCIndex)
  {
     fwIdx = reOrderInfo->winSize;
  }
  else
  {
     fwIdx = reOrderInfo->ucCIndex - 1;
  }

  if(0 != reOrderInfo->pendingFramesCount)
  {
    vosStatus = WLANTL_ChainFrontPkts(fwIdx,
                                      WLANTL_OPCODE_FWDALL_DROPCUR,
                                      &vosDataBuff, reOrderInfo, pTLCb);
  }

  if ((VOS_STATUS_SUCCESS == vosStatus) && (NULL != vosDataBuff))
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
             "WLAN TL: Chaining was successful sending all pkts to HDD : %x",
              vosDataBuff );

    wRxMetaInfo.ucUP = ucTid;
    pTLCb->atlSTAClients[ucSTAId].pfnSTARx( pvosGCtx, vosDataBuff, 
                                           (v_U8_t)ucSTAId, &wRxMetaInfo);
  }

  /*------------------------------------------------------------------------
     Delete reordering timer
   ------------------------------------------------------------------------*/
  if((VOS_TRUE == reOrderInfo->timerStarted) &&
     (VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState(&reOrderInfo->agingTimer)))
  {
    vosStatus = vos_timer_stop(&reOrderInfo->agingTimer);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus))
    { 
       BAMSGERROR("Timer stop fail", vosStatus, 0, 0);
       return vosStatus;
    }
    reOrderInfo->timerStarted = VOS_FALSE;
  }

  if(VOS_TIMER_STATE_STOPPED == vos_timer_getCurrentState(&reOrderInfo->agingTimer))
  {
  vosStatus = vos_timer_destroy(&reOrderInfo->agingTimer);
  }
  else
  {
    BAMSGERROR("Timer is not stopped state current state is %d",
                vos_timer_getCurrentState(&reOrderInfo->agingTimer), 0, 0);
  }
  if ( VOS_STATUS_SUCCESS != vosStatus ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
              "WLAN TL:Failed to destroy reorder timer on WLANTL_BaSessionAdd");
  }

  /*------------------------------------------------------------------------
    Delete session 
   ------------------------------------------------------------------------*/
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucExists = 0;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].usCount  = 0;
  pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid].ucCIndex = 0;
  reOrderInfo->winSize   = 0;
  reOrderInfo->SSN       = 0;
  reOrderInfo->sessionID = 0;

  VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
             "WLAN TL: BA session deleted for STA: %d TID: %d",
             ucSTAId, ucTid);

  memset((v_U8_t *)(&reOrderInfo->reorderBuffer->arrayBuffer[0]),
                    0,
                    WLANTL_MAX_WINSIZE * sizeof(v_PVOID_t));
  reOrderInfo->reorderBuffer->isAvailable = VOS_TRUE;

  return VOS_STATUS_SUCCESS;
}/* WLANTL_BaSessionDel */


/*----------------------------------------------------------------------------
    INTERACTION WITH TL main module
 ---------------------------------------------------------------------------*/

/*==========================================================================
      AMSDU sub-frame processing module
  ==========================================================================*/
/*==========================================================================
  FUNCTION    WLANTL_AMSDUProcess

  DESCRIPTION 
    Process A-MSDU sub-frame. Start of chain if marked as first frame. 
    Linked at the end of the existing AMSDU chain. 

  DEPENDENCIES 
         
  PARAMETERS 

   IN/OUT:
   vosDataBuff: vos packet for the received data
                 outgoing contains the root of the chain for the rx 
                 aggregated MSDU if the frame is marked as last; otherwise 
                 NULL
   
   IN
   pvosGCtx:     pointer to the global vos context; a handle to TL's 
                 control block can be extracted from its context 
   pvBDHeader:   pointer to the BD header
   ucSTAId:      Station ID 
   ucMPDUHLen:   length of the MPDU header
   usMPDULen:    length of the MPDU 
      
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_INVAL:   invalid input parameters
    VOS_STATUS_E_FAULT:   pointer to TL cb is NULL ; access would cause a 
                          page fault  
    VOS_STATUS_SUCCESS:   Everything is good :)

  Other values can be returned as a result of a function call, please check 
  corresponding API for more info. 
  
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_AMSDUProcess
( 
  v_PVOID_t   pvosGCtx,
  vos_pkt_t*  vosDataBuff, 
  v_PVOID_t   pvBDHeader,
  v_U8_t      ucSTAId,
  v_U8_t      ucMPDUHLen,
  v_U16_t     usMPDULen
)
{
  v_U8_t          ucFsf; /* First AMSDU sub frame */
  v_U8_t          ucAef; /* Error in AMSDU sub frame */
  WLANTL_CbType*  pTLCb = NULL; 
  v_U8_t          MPDUHeaderAMSDUHeader[WLANTL_MPDU_HEADER_LEN + TL_AMSDU_SUBFRM_HEADER_LEN];
  v_U16_t         subFrameLength;
  v_U16_t         paddingSize;
  VOS_STATUS      vStatus = VOS_STATUS_SUCCESS;
  v_U16_t         MPDUDataOffset;
  v_U16_t         packetLength; 
  static v_U32_t  numAMSDUFrames = 0;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /*------------------------------------------------------------------------
    Sanity check
   ------------------------------------------------------------------------*/
  if (( NULL == vosDataBuff ) || ( NULL == pvBDHeader ) || 
      ( WLANTL_STA_ID_INVALID(ucSTAId)) )
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
               "WLAN TL:Invalid parameter sent on WLANTL_AMSDUComplete");
    return VOS_STATUS_E_INVAL;
  }

  /*------------------------------------------------------------------------
    Extract TL control block 
   ------------------------------------------------------------------------*/
  pTLCb = VOS_GET_TL_CB(pvosGCtx);
  if ( NULL == pTLCb ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR,
         "WLAN TL:Invalid TL pointer from pvosGCtx on WLANTL_AMSDUComplete");
    return VOS_STATUS_E_FAULT;
  }

  /*------------------------------------------------------------------------
    Check frame
   ------------------------------------------------------------------------*/
  ucAef =  (v_U8_t)WLANHAL_RX_BD_GET_AEF( pvBDHeader );
  ucFsf =  (v_U8_t)WLANHAL_RX_BD_GET_ESF( pvBDHeader );
  MPDUDataOffset = (v_U16_t)WLANHAL_RX_BD_GET_MPDU_D_OFFSET(pvBDHeader) - WLANHAL_RX_BD_HEADER_SIZE;

  if ( WLANHAL_RX_BD_AEF_SET == ucAef ) 
  {
    VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO_HIGH,
               "WLAN TL:Error in AMSDU - dropping entire chain");

    vos_pkt_return_packet(vosDataBuff);
    vosDataBuff = NULL;
    return VOS_STATUS_SUCCESS; /*Not a transport error*/
  }

  if((0 != ucMPDUHLen) && ucFsf)
  {
    /*
     * This is first AMSDU sub frame
     * AMSDU Header should be removed
     * MPDU header should be stored into context to recover next frames
     */
    vStatus = vos_pkt_pop_head(vosDataBuff, MPDUHeaderAMSDUHeader, ucMPDUHLen + TL_AMSDU_SUBFRM_HEADER_LEN);
    if(!VOS_IS_STATUS_SUCCESS(vStatus))
    {
      BAMSGERROR("Pop MPDU AMSDU Header fail", 0, 0, 0);
      vos_pkt_return_packet(vosDataBuff);
      vosDataBuff = NULL;
      return VOS_STATUS_SUCCESS; /*Not a transport error*/
    }
    pTLCb->atlSTAClients[ucSTAId].ucMPDUHeaderLen = ucMPDUHLen;
    memcpy(pTLCb->atlSTAClients[ucSTAId].aucMPDUHeader, MPDUHeaderAMSDUHeader, ucMPDUHLen);
    /* AMSDU header stored to handle gabage data within next frame */
  }
  else
  {
    /* Trim gabage, size is frameLoop */
    if(MPDUDataOffset > 0)
    {
      vStatus = vos_pkt_trim_head(vosDataBuff, MPDUDataOffset);
    }
    if(!VOS_IS_STATUS_SUCCESS(vStatus))
    {
      BAMSGERROR("Trim Garbage Data fail", 0, 0, 0);
      vos_pkt_return_packet(vosDataBuff);
      vosDataBuff = NULL;
      return VOS_STATUS_SUCCESS; /*Not a transport error*/
    }

    /* Remove MPDU header and AMSDU header from the packet */
    vStatus = vos_pkt_pop_head(vosDataBuff, MPDUHeaderAMSDUHeader, ucMPDUHLen + TL_AMSDU_SUBFRM_HEADER_LEN);
    if(!VOS_IS_STATUS_SUCCESS(vStatus))
    {
      BAMSGERROR("AMSDU Header Pop fail", 0, 0, 0);
      vos_pkt_return_packet(vosDataBuff);
      vosDataBuff = NULL;
      return VOS_STATUS_SUCCESS; /*Not a transport error*/
    }
  } /* End of henalding not first sub frame specific */

  /* Put in MPDU header into all the frame */
  vStatus = vos_pkt_push_head(vosDataBuff, pTLCb->atlSTAClients[ucSTAId].aucMPDUHeader, pTLCb->atlSTAClients[ucSTAId].ucMPDUHeaderLen);
  if(!VOS_IS_STATUS_SUCCESS(vStatus))
  {
    BAMSGERROR("MPDU Header Push back fail", 0, 0, 0);
    vos_pkt_return_packet(vosDataBuff);
    vosDataBuff = NULL;
    return VOS_STATUS_SUCCESS; /*Not a transport error*/
  }

  /* Find Padding and remove */
  memcpy(&subFrameLength, MPDUHeaderAMSDUHeader + ucMPDUHLen + WLANTL_AMSDU_SUBFRAME_LEN_OFFSET, sizeof(v_U16_t));
  subFrameLength = vos_be16_to_cpu(subFrameLength);
  paddingSize = usMPDULen - ucMPDUHLen - subFrameLength - TL_AMSDU_SUBFRM_HEADER_LEN;

  vos_pkt_get_packet_length(vosDataBuff, &packetLength);
  if((paddingSize > 0) && (paddingSize < packetLength))
  {
    /* There is padding bits, remove it */
    vos_pkt_trim_tail(vosDataBuff, paddingSize);
  }
  else if(0 == paddingSize)
  {
    /* No Padding bits */
    /* Do Nothing */
  }
  else
  {
    /* Padding size is larger than Frame size, Actually negative */
    /* Not a valid case, not a valid frame, drop it */
    BAMSGERROR("Padding Size is negative, no possible %d", paddingSize, 0, 0);
    vos_pkt_return_packet(vosDataBuff);
    vosDataBuff = NULL;
    return VOS_STATUS_SUCCESS; /*Not a transport error*/
  }

  numAMSDUFrames++;
  if(0 == (numAMSDUFrames % 5000))
  {
    BAMSGERROR("%lu AMSDU frames arrived", numAMSDUFrames, 0, 0);
  }
  return VOS_STATUS_SUCCESS;
}/* WLANTL_AMSDUProcess */

/*==========================================================================
      Re-ordering module
  ==========================================================================*/

/*==========================================================================
  FUNCTION    WLANTL_MSDUReorder

  DESCRIPTION 
    MSDU reordering 

  DEPENDENCIES 
         
  PARAMETERS 

   IN
   
   vosDataBuff: vos packet for the received data
   pvBDHeader: pointer to the BD header
   ucSTAId:    Station ID 
      
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_SUCCESS:   Everything is good :)

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS WLANTL_MSDUReorder
( 
   WLANTL_CbType    *pTLCb,
   vos_pkt_t        **vosDataBuff, 
   v_PVOID_t        pvBDHeader,
   v_U8_t           ucSTAId,
   v_U8_t           ucTid
)
{
   WLANTL_BAReorderType *currentReorderInfo;
   vos_pkt_t            *vosPktIdx;
   v_U8_t               ucOpCode; 
   v_U8_t               ucSlotIdx;
   v_U8_t               ucFwdIdx;
   v_U8_t               CSN;
   VOS_STATUS           status = VOS_STATUS_SUCCESS; 

   if((NULL == pTLCb) || (*vosDataBuff == NULL))
   {
      BAMSGERROR("Invalid ARG pTLCb 0x%p, vosDataBuff 0x%p",
                  pTLCb, *vosDataBuff, 0);
      return VOS_STATUS_E_INVAL;
   }

   currentReorderInfo = &pTLCb->atlSTAClients[ucSTAId].atlBAReorderInfo[ucTid];
   ucOpCode  = (v_U8_t)WLANHAL_RX_BD_GET_BA_OPCODE(pvBDHeader);
   ucSlotIdx = (v_U8_t)WLANHAL_RX_BD_GET_BA_SI(pvBDHeader);
   ucFwdIdx  = (v_U8_t)WLANHAL_RX_BD_GET_BA_FI(pvBDHeader);
   CSN       = (v_U8_t)WLANHAL_RX_BD_GET_BA_CSN(pvBDHeader);
#ifdef WLANTL_REORDER_DEBUG_MSG_ENABLE
   BAMSGDEBUG("%s", opCodeSting[ucOpCode], 0, 0);
#endif /* WLANTL_REORDER_DEBUG_MSG_ENABLE */
   BAMSGDEBUG("SI %d, FI %d, CI %d", ucSlotIdx, ucFwdIdx, currentReorderInfo->ucCIndex);

   switch(ucOpCode) 
   {
      case WLANTL_OPCODE_INVALID:
         /* Do nothing just pass through current frame */
         break;

      case WLANTL_OPCODE_QCUR_FWDBUF:
            if(0 == currentReorderInfo->pendingFramesCount)
            {
               currentReorderInfo->ucCIndex = ucSlotIdx;
               return status;
            }
            status = WLANTL_QueueCurrent(currentReorderInfo,
                                         vosDataBuff,
                                         ucSlotIdx);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Input Q current frame fail %d", status, 0, 0);
               return status;
            }
            vosPktIdx = NULL;
            status = WLANTL_ChainFrontPkts(ucFwdIdx,
                                           WLANTL_OPCODE_QCUR_FWDBUF, 
                                           &vosPktIdx,
                                           currentReorderInfo,
                    pTLCb);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("IMake frame chain fail %d", status, 0, 0);
               return status;
            }
            *vosDataBuff = vosPktIdx;
         break;

      case WLANTL_OPCODE_FWDBUF_FWDCUR:
         vosPktIdx = NULL;
         status = WLANTL_ChainFrontPkts(ucFwdIdx,
                                        WLANTL_OPCODE_FWDBUF_FWDCUR, 
                                        &vosPktIdx,
                                        currentReorderInfo,
                    pTLCb);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make frame chain fail %d", status, 0, 0);
            return status;
         }

         status = vos_pkt_chain_packet(vosPktIdx, *vosDataBuff, 1);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make frame chain with CUR frame fail %d",
                        status, 0, 0);
            return status;
         }
         *vosDataBuff = vosPktIdx;
         if((currentReorderInfo->pendingFramesCount == 0) && (VOS_TRUE == currentReorderInfo->timerStarted))
         {
            BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
            status = vos_timer_stop(&currentReorderInfo->agingTimer);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Timer stop fail", status, 0, 0);
               return status;
            }
            currentReorderInfo->timerStarted = VOS_FALSE;
         }
         break;

      case WLANTL_OPCODE_QCUR:
         status = WLANTL_QueueCurrent(currentReorderInfo,
                                      vosDataBuff,
                                      ucSlotIdx);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Put current frame into Q fail %d", status, 0, 0);
            return status;
         }
         /* Since current Frame is Qed, no frame will be routed */
         *vosDataBuff = NULL; 
         break;

      case WLANTL_OPCODE_FWDBUF_QUEUECUR:
         vosPktIdx = NULL;
         status = WLANTL_ChainFrontPkts(ucFwdIdx,
                                        WLANTL_OPCODE_FWDBUF_QUEUECUR, 
                                        &vosPktIdx,
                                        currentReorderInfo,
                    pTLCb);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make cahin with buffered frame fail %d",
                        status, 0, 0);
            return status;
         }

         if((currentReorderInfo->pendingFramesCount == 0) && (VOS_TRUE == currentReorderInfo->timerStarted))
         {
            BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
            status = vos_timer_stop(&currentReorderInfo->agingTimer);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Timer stop fail", status, 0, 0);
               return status;
            }
            currentReorderInfo->timerStarted = VOS_FALSE;
         }

         status = WLANTL_QueueCurrent(currentReorderInfo,
                                      vosDataBuff,
                                      ucSlotIdx);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Put current frame into Q fail %d", status, 0, 0);
            return status;
         }
         *vosDataBuff = vosPktIdx;
         break;

      case WLANTL_OPCODE_FWDBUF_DROPCUR:
         vosPktIdx = NULL;
         status = WLANTL_ChainFrontPkts(ucFwdIdx,
                                        WLANTL_OPCODE_FWDBUF_DROPCUR, 
                                        &vosPktIdx,
                                        currentReorderInfo,
                     pTLCb);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make cahin with buffered frame fail %d",
                        status, 0, 0);
            return status;
         }

         /* Current frame has to be dropped, BAR frame */
         status = vos_pkt_return_packet(*vosDataBuff);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Drop BAR frame fail %d",
                        status, 0, 0);
            return status;
         }
         *vosDataBuff = vosPktIdx;

         if((currentReorderInfo->pendingFramesCount == 0) && (VOS_TRUE == currentReorderInfo->timerStarted))
         {
            BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
            status = vos_timer_stop(&currentReorderInfo->agingTimer);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Timer stop fail", status, 0, 0);
               return status;
            }
            currentReorderInfo->timerStarted = VOS_FALSE;
         }
         break;
 
      case WLANTL_OPCODE_FWDALL_DROPCUR:
         vosPktIdx = NULL;
         status = WLANTL_ChainFrontPkts(ucFwdIdx,
                                        WLANTL_OPCODE_FWDALL_DROPCUR, 
                                        &vosPktIdx,
                                        currentReorderInfo,
                     pTLCb);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make chain with buffered frame fail %d",
                        status, 0, 0);
            return status;
         }

         status = vos_pkt_return_packet(*vosDataBuff);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Drop BAR frame fail %d",
                        status, 0, 0);
            return status;
         }

         *vosDataBuff = vosPktIdx;
         if((currentReorderInfo->pendingFramesCount == 0) && (VOS_TRUE == currentReorderInfo->timerStarted))
         {
            BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
            status = vos_timer_stop(&currentReorderInfo->agingTimer);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Timer stop fail", status, 0, 0);
               return status;
            }
            currentReorderInfo->timerStarted = VOS_FALSE;
         }
         break;

      case WLANTL_OPCODE_FWDALL_QCUR:
         vosPktIdx = NULL;
         status = WLANTL_ChainFrontPkts(currentReorderInfo->winSize,
                                        WLANTL_OPCODE_FWDALL_DROPCUR, 
                                        &vosPktIdx,
                                        currentReorderInfo,
                    pTLCb);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Make cahin with buffered frame fail %d",
                        status, 0, 0);
            return status;
         }

         if((currentReorderInfo->pendingFramesCount == 0) && (VOS_TRUE == currentReorderInfo->timerStarted))
         {
            BAMSGDEBUG("There is no more HOLE, Stop aging timer", 0, 0, 0);
            status = vos_timer_stop(&currentReorderInfo->agingTimer);
            if(!VOS_IS_STATUS_SUCCESS(status))
            {
               BAMSGERROR("Timer stop fail", status, 0, 0);
               return status;
            }
            currentReorderInfo->timerStarted = VOS_FALSE;
         }

         status = WLANTL_QueueCurrent(currentReorderInfo,
                                      vosDataBuff,
                                      ucSlotIdx);
         if(!VOS_IS_STATUS_SUCCESS(status))
         {
            BAMSGERROR("Q Current frame fail %d",
                        status, 0, 0);
            return status;
         }
         currentReorderInfo->ucCIndex = ucSlotIdx;
         *vosDataBuff = vosPktIdx;
         break;

      case WLANTL_OPCODE_DROPCUR:
         vos_pkt_return_packet(*vosDataBuff);
         *vosDataBuff = NULL;
         break;

      default:
         break;
   }

   WLANTL_ReorderDetectHole(currentReorderInfo);
   return VOS_STATUS_SUCCESS;

}/* WLANTL_MSDUReorder */


/*==========================================================================
     Utility functions 
  ==========================================================================*/

/*==========================================================================

  FUNCTION    WLANTL_QueueCurrent

  DESCRIPTION 
    It will queue a packet at a given slot index in the MSDU reordering list. 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pwBaReorder:   pointer to the BA reordering session info 
    vosDataBuff:   data buffer to be queued
    ucSlotIndex:   slot index 
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_SUCCESS:     Everything is OK

    
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS WLANTL_QueueCurrent
(
   WLANTL_BAReorderType*  pwBaReorder,
   vos_pkt_t**            vosDataBuff,
   v_U8_t                 ucSlotIndex
)
{
   VOS_STATUS  status = VOS_STATUS_SUCCESS;

   BAMSGDEBUG("vos Packet has to be Qed 0x%p",
               *vosDataBuff, 0, 0);
   if(NULL != pwBaReorder->reorderBuffer->arrayBuffer[ucSlotIndex])
   {
      BAMSGERROR("Something wrong, buffer must be NULL", 0, 0, 0);
      return VOS_STATUS_E_FAILURE;
   }

   pwBaReorder->reorderBuffer->arrayBuffer[ucSlotIndex] =
                                           (v_PVOID_t)(*vosDataBuff);
   pwBaReorder->pendingFramesCount++;
   BAMSGDEBUG("Assigned, Pending Frames %d at slot %d, dataPtr 0x%x",
               pwBaReorder->pendingFramesCount,
               ucSlotIndex,
               pwBaReorder->reorderBuffer->arrayBuffer[ucSlotIndex]);

   return status;
}/*WLANTL_QueueCurrent*/

/*==========================================================================

  FUNCTION    WLANTL_ChainFrontPkts

  DESCRIPTION 
    It will remove all the packets from the front of a vos list and chain 
    them to a vos pkt . 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    ucCount:       number of packets to extract
    pwBaReorder:   pointer to the BA reordering session info 

    OUT
    vosDataBuff:   data buffer containing the extracted chain of packets
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_SUCCESS:     Everything is OK

    
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS WLANTL_ChainFrontPkts
( 
   v_U32_t                fwdIndex,
   v_U8_t                 opCode,
   vos_pkt_t              **vosDataBuff,
   WLANTL_BAReorderType   *pwBaReorder,
   WLANTL_CbType          *pTLCb
)
{
   VOS_STATUS          status = VOS_STATUS_SUCCESS;
   v_U32_t             idx;
   v_PVOID_t           currentDataPtr = NULL;

   if(pwBaReorder->ucCIndex >= fwdIndex)
   {
      fwdIndex += pwBaReorder->winSize;
   }

   if((WLANTL_OPCODE_FWDALL_DROPCUR == opCode) ||
      (WLANTL_OPCODE_FWDALL_QCUR == opCode))
   {
      fwdIndex = pwBaReorder->ucCIndex + pwBaReorder->winSize;
   }

   BAMSGDEBUG("Current Index %d, FWD Index %d, reorderBuffer 0x%p",
               pwBaReorder->ucCIndex % pwBaReorder->winSize,
               fwdIndex % pwBaReorder->winSize,
               pwBaReorder->reorderBuffer);

   for(idx = pwBaReorder->ucCIndex; idx < fwdIndex; idx++)
   {
      currentDataPtr = 
      pwBaReorder->reorderBuffer->arrayBuffer[idx % pwBaReorder->winSize];
      if(NULL != currentDataPtr)
      {
         BAMSGDEBUG("There is buffered frame %d",
                     idx % pwBaReorder->winSize, 0, 0);
         if(NULL == *vosDataBuff)
         {
            *vosDataBuff = (vos_pkt_t *)currentDataPtr;
            BAMSGDEBUG("This is new head %d",
                        idx % pwBaReorder->winSize, 0, 0);
         }
         else
         {
            BAMSGDEBUG("There is bufered Just add %d",
                        idx % pwBaReorder->winSize, 0, 0);
            vos_pkt_chain_packet(*vosDataBuff,
                                 (vos_pkt_t *)currentDataPtr,
                                 VOS_TRUE);
         }
         pwBaReorder->reorderBuffer->arrayBuffer[idx  % pwBaReorder->winSize]
                                                                       = NULL;
         pwBaReorder->pendingFramesCount--;
         BAMSGDEBUG("Slot Index %d, set as NULL, Pending Frames %d",
                     idx  % pwBaReorder->winSize,
                     pwBaReorder->pendingFramesCount,
                     0);
         pwBaReorder->ucCIndex = idx % pwBaReorder->winSize;
      }
      else
      {
         BAMSGDEBUG("Empty Array %d",
                     idx % pwBaReorder->winSize, 0, 0);
      }
      BAMSGDEBUG("Current Index %d, winSize %d",
                  pwBaReorder->ucCIndex,
                  pwBaReorder->winSize,
                  0);
   }
   return status; 
}/*WLANTL_ChainFrontPkts*/

