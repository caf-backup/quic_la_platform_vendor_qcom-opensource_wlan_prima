/**
 *
 *  @brief:     Provides function to provide flush operation for 
 *  Transport Layer.
 *
 *  Copyright (C) 2008, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 09/3/2008  File created.
 */
#include "halTLFlush.h"

/*---------------------------------------------------------
 *
 * FUNCTION:
 *
 * This function is called to send the SIR_HAL_TL_FLUSH_AC_RSP 
 * back to TL, with the status of the flush operation.
 *
 *---------------------------------------------------------
 */
static tSirRetStatus halTLSendFlushResponse(tpAniSirGlobal pMac, 
    tANI_U8 ucSTAId, tANI_U8 ucTid, tSirRetStatus status)
{
  tpFlushACRsp pFlushACRspPtr = NULL;
  tSirMsgQ msg;

  // Allocate message buffer
  if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd,
        (void **) &pFlushACRspPtr,
        sizeof( tFlushACRsp )))
  {
    HALLOGE(halLog( pMac, LOGE, 
        FL("Unable to allocate PAL memory for Flush Rsp")));
    HALLOGE(halLog( pMac, LOGE, 
        FL("Unable send Flush Rsp from HAL to TL")));
    return eHAL_STATUS_FAILURE;
  }

  HALLOG1(halLog( pMac, LOG1, 
    FL("Flush structure allocated %p\n"), pFlushACRspPtr));
  palZeroMemory(pMac->hHdd, pFlushACRspPtr, sizeof(tFlushACRsp));

  pFlushACRspPtr->mesgType = SIR_HAL_TL_FLUSH_AC_RSP;
  pFlushACRspPtr->mesgLen = sizeof( tFlushACRsp );
  pFlushACRspPtr->ucSTAId = ucSTAId;
  pFlushACRspPtr->ucTid = ucTid;
  pFlushACRspPtr->status = status;

  // POST message to HDD
  msg.type = SIR_HAL_TL_FLUSH_AC_RSP;
  msg.bodyptr = (void *)pFlushACRspPtr;

  halTlPostMsgApi(pMac, &msg);

  return eSIR_SUCCESS;
}

/*---------------------------------------------------------
 *
 * FUNCTION:
 *
 *   This function is called by HAL when it receives a FLUSH 
 *   request message from TL.
 *
 *---------------------------------------------------------
 */
tSirRetStatus halTLProcessFlushReq(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    // Flush the BTQM queues
    tpFlushACReq FlushACReqPtr = (tpFlushACReq)(pMsg->bodyptr);
    tSirRetStatus status;

    assert(FlushACReqPtr != NULL);

    // Flush the Queues, cleanup the queues.
    status = halBmu_sta_enable_disable_control(pMac, 
        FlushACReqPtr->ucSTAId, eBMU_DIS_TX_QUE_DIS_TRANS_CLEANUP_QUE); 
    if ( eSIR_SUCCESS != status )
    {
        HALLOGE(halLog( pMac, LOGE, FL("Unable to update cleanup BMU queues")));
        // Send the response back to TL with the status
        halTLSendFlushResponse(pMac, FlushACReqPtr->ucSTAId, 
            FlushACReqPtr->ucTid, status);
        palFreeMemory( pMac->hHdd, FlushACReqPtr);
        return status;
    }

    // Enable the TX queues in the BMU
    status = halBmu_sta_enable_disable_control(pMac, 
        FlushACReqPtr->ucSTAId, eBMU_ENB_TX_QUE_ENB_TRANS); 
    if ( eSIR_SUCCESS != status )
    {
        HALLOGE(halLog( pMac, LOGE, FL("Unable to enable BMU queues")));
        halTLSendFlushResponse(pMac, FlushACReqPtr->ucSTAId, 
            FlushACReqPtr->ucTid, status);
        palFreeMemory( pMac->hHdd, FlushACReqPtr);
        return status;
    }

    //
    // Return the status of the flush operation in a response message
    halTLSendFlushResponse(pMac, FlushACReqPtr->ucSTAId, 
        FlushACReqPtr->ucTid, status);

    palFreeMemory(pMac->hHdd, FlushACReqPtr);
    return eSIR_SUCCESS;
}

/* End of File */
