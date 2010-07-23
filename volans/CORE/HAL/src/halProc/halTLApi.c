/**
 *
 *  @file:      halTLApi.c
 *
 *  @brief:     Provides all the APIs to interact with Transport Layer.
 *
 *  @author:    Lawrie Kurian
 *
 *  Copyright (C) 2008, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 05/21/2008  File created.
 */

#include "palTypes.h"
#include "vos_api.h"
#include "wlan_qct_tl.h"
#include "vos_event.h"
#include "vos_types.h" 
#include "halInternal.h"
#include "halTLApi.h"
#include "halFwApi.h"
#include "halDebug.h"
#include "halUtils.h"
#include "cfgApi.h"
#include "wlan_qct_hal.h"
#include "wlan_qct_tl.h"

//#define WLAN_PERF

/* 
 * DESCRIPTION:
 *      Intializes the parameters required to interact with TL
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      arg   :  dummy to match with the funcTable definition
 *
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLApiInit(tpAniSirGlobal pMac, void* arg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
	tpHalRxBd  pAmsduRxBdFixMask;

    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

    // Initialize the event
    vosStatus = vos_event_init(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event init failed - status = %d\n"),  vosStatus ));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

    // Init Mgmt Frame transfer event
    vosStatus = vos_event_init(&pMac->hal.TLParam.txMgmtFrameEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE(halLog(pMac, LOGE, FL("VOS Mgmt Frame Event init failed - status = %d\n"), vosStatus));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
    // WQ to be used for filling the TxBD
    pMac->hal.halMac.dpuRF = BMUWQ_BTQM_TX_MGMT;
#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

    // Initialize the TL suspend timed out flag to false.
    pMac->hal.TLParam.txSuspendTimedOut = FALSE;

#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum = 0;
#endif
	pAmsduRxBdFixMask = &pMac->hal.halMac.rxAmsduBdFixMask;

    vos_mem_set(pAmsduRxBdFixMask,sizeof(tHalRxBd), 0xff);
    pAmsduRxBdFixMask->penultimatePduIdx = 0;
    pAmsduRxBdFixMask->headPduIdx = 0;
    pAmsduRxBdFixMask->tailPduIdx = 0;
	pAmsduRxBdFixMask->mpduHeaderLength = 0;
	pAmsduRxBdFixMask->mpduHeaderOffset = 0;
	pAmsduRxBdFixMask->mpduDataOffset = 0;
	pAmsduRxBdFixMask->pduCount = 0;
	pAmsduRxBdFixMask->mpduLength = 0;
	pAmsduRxBdFixMask->asf = 0;
	pAmsduRxBdFixMask->esf = 0;
	pAmsduRxBdFixMask->lsf = 0;
	pAmsduRxBdFixMask->processOrder = 0;
	pAmsduRxBdFixMask->sybFrameIdx = 0;
	pAmsduRxBdFixMask->totalMsduSize = 0;
	pAmsduRxBdFixMask->aduFeedback = 0;
    return status;
}


/* 
 * DESCRIPTION:
 *      Destroy the parameters required to interact with TL
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      arg  :    dummy to match with functTable
 *
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLApiExit(tpAniSirGlobal pMac, void* arg)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

    // Destroy the event
    vosStatus = vos_event_destroy(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event init failed - status = %d\n"),  vosStatus ));
        return eHAL_STATUS_FAILURE;
    }

    // Destroy the event
    vosStatus = vos_event_destroy(&pMac->hal.TLParam.txMgmtFrameEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event init failed - status = %d\n"),  vosStatus ));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/* 
 * DESCRIPTION:
 *      Callback function provided for TL(transport layer), called after
 *      suspending transmission
 *
 * PARAMETERS:
 *      pVosGCtx:   Pointer to the global VOS contextt
 *      pStaId:  pointer to Station ID for which the TX will be suspended 
 *      statusCode:  VOS Status Code, specifying success or failure when suspending 
 *              transmission for the particular STA
 *
 * RETURN VALUE:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_FAILURE
 */
VOS_STATUS halTLSuspendTxCallBack(v_PVOID_t pVosGCtx, v_U8_t* pStaId, VOS_STATUS statusCode)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tpAniSirGlobal pMac = (tpAniSirGlobal)vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);

    if(VOS_IS_STATUS_SUCCESS(statusCode)) {
        pMac->hal.TLParam.txStatus = HAL_TL_TX_SUSPEND_SUCCESS;
    }
    else{
        pMac->hal.TLParam.txStatus = HAL_TL_TX_SUSPEND_FAILURE;        
    }

    // Trigger the event to bring the HAL TL suspend function to come out of wait
    vosStatus = vos_event_set(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event init failed - status = %d\n"),  vosStatus ));
    };

    // If TL suspended had timedout before this callback was called, resume back TL.
    if (pMac->hal.TLParam.txSuspendTimedOut) {
        HALLOGE( halLog(pMac, LOGE, FL("Late TLSuspendCallback, resmuing TL back again\n")));
        halTLResumeTx(pMac, pStaId);
        pMac->hal.TLParam.txSuspendTimedOut = FALSE;
    }
    
    return vosStatus;
}

/* 
 * DESCRIPTION:
 *      Function to suspend transmission in the transport layer
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      *pStaId:Pointer to the Station ID for which the TX will be suspended.
 *              Value of NULL signifies all STA's TX queue needs to be 
 *              suspended.
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLSuspendTx(tpAniSirGlobal pMac, tANI_U8* pStaId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U8 eventIdx = 0;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);

    pMac->hal.TLParam.txStatus = HAL_TL_TX_SUSPEND_FAILURE;

    if (pMac->hal.TLParam.txSuspendTimedOut) {
        HALLOGE( halLog(pMac, LOGE, FL("TL suspend timedout previously, CB not called yet\n")));
        return status;
    }

    // Reset the event to be not signalled
    vosStatus = vos_event_reset(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE(halLog(pMac, LOGE, FL("VOS Event reset failed - status = %d\n"), vosStatus));
		return status;
    }

    // Indicate TL to suspend transmission
    if(WLANTL_SuspendDataTx(pVosGCtx, (v_U8_t*) pStaId, halTLSuspendTxCallBack) != VOS_STATUS_SUCCESS) {
        return status;
    }

    // Wait for the event to be set by the TL, to get the response of suspending the 
    // TX queues, this event should be set by the Callback function called by TL
    vosStatus = vos_wait_events(&pMac->hal.TLParam.txEvent, 1, HAL_TL_SUSPEND_TIMEOUT, &eventIdx);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event wait failed - status = %d\n"),  vosStatus ));
        // Set this flag to true when TL suspend times out, so that when TL suspend 
        // eventually happens and calls the callback, TL can be resumed right away by looking at
        // this flag when true.
        pMac->hal.TLParam.txSuspendTimedOut = TRUE;
    } else {
        pMac->hal.TLParam.txSuspendTimedOut = FALSE;
    }

    if(pMac->hal.TLParam.txStatus == HAL_TL_TX_SUSPEND_SUCCESS) {
        status = eHAL_STATUS_SUCCESS;
    }

    return status;
}

/* 
 * DESCRIPTION:
 *      Function to resume transmission in the transport layer for a particular 
 *      station.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      *pStaId:Pointer to the Station ID for which the TX will be resumed.
 *              Value of NULL signifies all STA's TX queue needs to be 
 *              resumed.
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLResumeTx(tpAniSirGlobal pMac, tANI_U8* pStaId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;

    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
    // Indicate TL to resume the transmission
    if(WLANTL_ResumeDataTx(pVosGCtx, pStaId) == VOS_STATUS_SUCCESS) {
        status = eHAL_STATUS_SUCCESS;
    }

    return status; 
}

/* 
 * DESCRIPTION:
 *      Callback Function called by TL on completion if the transmitting the frame
 *
 * PARAMETERS:
 *      pVosGCtx:   Pointer to the global VOSS context
 *      void*:  Pointer to the frame to txmitted
 *      statusCode: Return status of the transmission
 *      
 * RETURN VALUE:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_FAILURE
 */
VOS_STATUS halTLTxCompleteCB(v_PVOID_t pVosGCtx, void* pData, VOS_STATUS statusCode)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tpAniSirGlobal pMac = (tpAniSirGlobal)vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);

    if(VOS_IS_STATUS_SUCCESS(statusCode)) {
        pMac->hal.TLParam.txStatus = HAL_TL_TX_SUCCESS;
    }
    else{
        pMac->hal.TLParam.txStatus = HAL_TL_TX_FAILURE;        
    }

    palPktFree( pMac->hHdd,
              HAL_TXRX_FRM_802_11_MGMT,
              (void *) NULL,           // this is ignored and will likely be removed from this API
              (void *) pData );        // lim passed in pPacket in the pData pointer that is given in this completion routine

    // Trigger the event to bring the HAL TL  send routine to come out of wait
    vosStatus = vos_event_set(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event set failed - status = %d\n"),  vosStatus ));
    };

    return vosStatus;
}

/* 
 * DESCRIPTION:
 *      Function to send frame through the TL module
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      void*:  Pointer to the frame to be txmitted
 *      txFlag : caller can request for TxComp intr and/or sta overwrite.
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLSend80211Frame(tpAniSirGlobal pMac, 
        void* pFrame, tANI_U8 frameType, tANI_U16 frameLen, tANI_U8 tid, 
        tANI_U8 txFlag)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    eHalStatus retval;
    tANI_U8 eventIdx = 0;	
    void       *pPacket, *pData;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    VOS_STATUS tlStatus = VOS_STATUS_E_FAILURE;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);

    pMac->hal.TLParam.txStatus = HAL_TL_TX_FAILURE;

    // Reset the event to be not signalled
    vosStatus = vos_event_reset(&pMac->hal.TLParam.txEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event reset failed - status = %d\n"),  vosStatus ));
        return eHAL_STATUS_FAILURE;
    }

    retval = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )frameLen, ( void** ) &pData,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( retval ) )
    {
        HALLOGP( halLog( pMac, LOGP, FL("Failed to allocate %d bytes\n"),  frameLen  ));
        return retval;
    }

    palCopyMemory( pMac->hHdd, ( tANI_U8* )pData, ( tANI_U8* )pFrame,
                       frameLen );

    if(  (tlStatus = WLANTL_TxMgmtFrm(pVosGCtx, pPacket, frameLen, frameType, tid, 
                (WLANTL_TxCompCBType)halTLTxCompleteCB, NULL, txFlag))!= VOS_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("Sending Mgmt Frame failed - status = %d\n"),  tlStatus ));
        return eHAL_STATUS_FAILURE;
    }

    // Wait for the event to be set by the TL, to get the response of TxComplete  
    // this event should be set by the Callback function called by TL
    vosStatus = vos_wait_events(&pMac->hal.TLParam.txEvent, 1, HAL_TL_TX_FRAME_TIMEOUT, &eventIdx);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Event wait failed - status = %d\n"),  vosStatus ));
        status = eHAL_STATUS_FAILURE;
    }
    else if(pMac->hal.TLParam.txStatus != HAL_TL_TX_SUCCESS) {
        status = eHAL_STATUS_FAILURE;
    }
    return status;
}

/** Swap Bytes */
static inline void swapBytes(tANI_U8 *pBd, tANI_U32 nbSwap)
{
    tANI_U32 *pU32;
    tANI_U32 nU32;
    tANI_U32 wc;

    nU32 = (((nbSwap) + 3)>>2);

    pU32 = (tANI_U32 *)pBd;
    for ( wc = 0; wc < nU32; wc++ )
    {
        pU32[ wc ] = vos_be32_to_cpu( pU32[ wc ] );
    }
}

#ifdef WLAN_PERF

static tANI_U32 computeTxBdSignature(tpAniSirGlobal  pMac, tANI_U8 *pDestMac, tANI_U8 tid, tANI_U8 unicastDst){
    tANI_U16 *pMacU16 = (tANI_U16 *) pDestMac;
    return ((pMacU16[0] ^ pMacU16[1] ^ pMacU16[2])<< HAL_TXBD_SIG_MACADDR_HASH_OFFSET |
        pMac->hal.halMac.uBdSigSerialNum << HAL_TXBD_SIG_SERIAL_OFFSET |
        tid << HAL_TXBD_SIG_TID_OFFSET |
        unicastDst << HAL_TXBD_SIG_UCAST_DATA_OFFSET);
}



tANI_U32 WLANHAL_TxBdFastFwd(void *pVosGCtx, tANI_U8 *pDestMac, tANI_U8 tid, tANI_U8 unicastDst, 
                             void *pTxBd, tANI_U16 mpduLength )
{
    tpAniSirGlobal  pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);
    tpHalTxBd       pBd = (tpHalTxBd) pTxBd;
    tANI_U32        retval = 0;
    tANI_U16        swapped, *pU16 = (tANI_U16 *) pTxBd;

    if( pBd->txBdSignature ==  computeTxBdSignature(pMac, pDestMac, tid, unicastDst)){

#ifdef ANI_LITTLE_BYTE_ENDIAN
       /* When swap to BE format, mpduLength field is at 8th WORD location(16th byte) */
       swapped = vos_cpu_to_be16(mpduLength);
       pU16[8] = swapped;
#else
        /* Remove the #error when ported to a real BIG ENDIAN machine */
       #error "Is host byte order really BIG endian?"
       /* When host is already in BE format, no swapping needed.*/
       pBd->mpduLength = mpduLength;
#endif
       retval = 1;
    }
    return retval ;
}

#endif


/** To swap the RxBD */
void WLANHAL_SwapRxBd(tANI_U8 *pBd)
{
#ifndef ANI_BIG_BYTE_ENDIAN
    swapBytes(pBd , WLANHAL_RX_BD_HEADER_SIZE);
#endif
}


/** To swap the TxBD */
static inline void WLANHAL_SwapTxBd(tANI_U8 *pBd)
{
#ifndef ANI_BIG_BYTE_ENDIAN
    swapBytes(pBd , WLANHAL_TX_BD_HEADER_SIZE);
#endif
}

void WLANHAL_RxAmsduBdFix(void *pVosGCtx, v_PVOID_t _pvBDHeader)
{
    tpHalRxBd pRxBd = (tpHalRxBd) _pvBDHeader;
    tpAniSirGlobal  pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);
    if(pRxBd->asf){
		if(pRxBd->sybFrameIdx == 0){
            //copy the BD of first AMSDU
			pMac->hal.halMac.rxAmsduFirstBdCache = *pRxBd;
		}else{
			tANI_U32 *pModBd, *pMaskBd, *pFirstBd, i;
			pModBd = (tANI_U32*)pRxBd;
			pMaskBd = (tANI_U32*)&pMac->hal.halMac.rxAmsduBdFixMask;
			pFirstBd = (tANI_U32*)&pMac->hal.halMac.rxAmsduFirstBdCache;
			for(i=0; i < sizeof(tHalRxBd)/sizeof(tANI_U32 *); i++){
				//modified BD = zero out non AMSDU related fields in this BD |
				//              non AMSDU related fields from the first BD.
				pModBd[i]= (pModBd[i] & ~pMaskBd[i])| (pFirstBd[i] & pMaskBd[i]);
			}
		}
    }
    return;
}



/*==========================================================================

  FUNCTION    WLANHAL_FillTxBd

  DESCRIPTION 
    Called by TL to fill in TxBD. 

    Following are the highlights of the function

    1. All unicast data packets are sent by data rate decided by TPE. (i.e BD rates are disabled).
    2. All u/mcast management packets would go in Broadcast Management Rates
    3. dpuNE would be disabled for all data packets
    4. dpuNE would be enabled for all management packets excluding packets when RMF is enabled
    5. QID8 at self STA is for broadcast data which uses no ACK policy. 
    6. QID9 at self STA, we use it for unicast mgmt and set ACK policy to normal ACK. 
    7. QID10 at self STA, we use it for b/mcast mgmt and set ACK policy to NO ACK.

  DEPENDENCIES 
    HAL must be initialized before this API can be called. 
    
  PARAMETERS 

    IN
    pvosGCtx:       pointer to the global vos context;a handle to HAL's 
                    control block can be extracted from its context 
    typeSubtype:    802.11 [5:4] type [3:0] subtype

    pDestMacAddr:   destination MAC address
    
    pTid:           ptr to TID of the frame

    disableFrmXtl:  When set, disables UMA HW frame translation and HAL needs to fill
                    in all BD fields. When not set, UMA performs BD filling and frame
                    translation

    pTxBd:          ptr to the TxBD

    txFlag:    different option setting for TX.

    timeStamp:      Timestamp when the frame was received from HDD. (usec)
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/

VOS_STATUS WLANHAL_FillTxBd(void *pVosGCtx, tANI_U8 typeSubtype, void *pDestMacAddr,
                    tANI_U8* pTid, tANI_U8 disableFrmXtl, void *pTxBd, tANI_U8 txFlag, tANI_U32 timeStamp)
{
    tANI_U8         tid = *pTid; 
    tpHalTxBd       pBd = (tpHalTxBd) pTxBd;
    tpAniSirGlobal  pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);
    eHalStatus      status = eHAL_STATUS_SUCCESS;
    tANI_U8         unicastDst = 0, dpuSig = 0;
    tANI_U8         type = 0, subType = 0, isRMF = 0;
    tANI_U32        cfgLen = 0;
#ifdef WLAN_PERF
    tANI_U32        txBdSignature = pBd->txBdSignature;
#endif
    tSirMacAddr     macAddr;
    tSystemRole     systemRole = halGetSystemRole(pMac);

    type = (typeSubtype & HAL_FRAME_TYPE_MASK) >> HAL_FRAME_TYPE_OFFSET;
    subType = (typeSubtype & HAL_FRAME_SUBTYPE_MASK);

    HALLOG1( halLog(pMac, LOG1, FL("Type: %d/%d, MAC: %08x., Tid=%d, frmXlat=%d, pTxBD=%08x txFlag 0x%X\n"), 
        type, subType, *((tANI_U32 *) pDestMacAddr), tid, !disableFrmXtl, pTxBd, txFlag 
    ));

    /* - Set common fields in TxBD
     *     bdt: always HWBD_TYPE_GENERIC
     *     dpuRF: This is not used in Gen6 since all WQs are explicitly programmed to each HW module
     *     tid: from caller, ignored if frame is MGMT frame
     *     fwTxComplete0: always set to 0
     *     txComplete1: If TxComp inrs  is requested, enable TxComplete interrupt
     *     dpuFeedback/aduFeedback/reserved2: Always set to 0
     *     ap: ACK policy to be placed in Qos ctrl field. Ignored by HW if non Qos type frames.
     *     u/b: If Addr1 of this frame in its 802.11 form is unicast, set to 0. Otherwise set to 1.
     *     dpuNE: always set to 0. DPU also uses the privacy bit in 802.11 hdr for encryption decision
     */
    
    pBd->bdt   = HWBD_TYPE_GENERIC;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
    // Route all trigger enabled frames to FW WQ, for FW to suspend trigger frame generation 
    // when no traffic is exists on trigger enabled ACs
    if(txFlag & HAL_TRIGGER_ENABLED_AC_MASK) {
        pBd->dpuRF = pMac->hal.halMac.dpuRF;
    } else 
#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
    {
        pBd->dpuRF = BMUWQ_BTQM_TX_MGMT;
    }


    pBd->tid   = tid; 
    pBd->fwTxComplete0 = 0;
    pBd->txComplete1 = (txFlag & HAL_TXCOMP_REQUESTED_MASK) ? 1 : 0; /* This bit is for host to register TxComplete Interrupt */

    pBd->ap     = HAL_ACKPOLICY_ACK_REQUIRED; 
    pBd->dpuNE = HAL_NO_ENCRYPTION_DISABLED;  
    
    *((tANI_U32 *)pBd + HAL_DPU_FEEDBACK_OFFSET) = 0;

    if (type == SIR_MAC_DATA_FRAME){

        /* - Set common fields for data frames (regardless FT enable/disable)
         *     bd_ssn: Let DPU auto generate seq # if QosData frame. All other frames DPU generates seq #
         *             using nonQos counter. For QosNull, don't occupy one Qos seq # to avoid a potential 
         *             hole seen in reorder buffer when BA is enabled.
         *     bd_rate:HW default or broadcast data rate
         *     rmf:    RMF doesn't apply for data frames. Always set to 0
         *     u/b: If Addr1 of this frame in its 802.11 form is unicast, set to 0. Otherwise set to 1.
         * - Sanity: Force disable HW frame translation if incoming frame is NULL data frame
         */

        if ((subType & SIR_MAC_DATA_QOS_DATA)&&(subType != SIR_MAC_DATA_QOS_NULL)){
            pBd->bd_ssn = HAL_TXBD_BD_SSN_FILL_DPU_QOS;
        }else{
            pBd->bd_ssn = HAL_TXBD_BD_SSN_FILL_DPU_NON_QOS;
        }

        /* Unicast/Mcast decision:
         *  In Infra STA role, all frames to AP are unicast frames.
         *  For IBSS, then check the actual DA MAC address 
         *  This implementation doesn't support multi BSS and AP case.
         */

        if(eSYSTEM_STA_IN_IBSS_ROLE == systemRole)
            unicastDst = !(((tANI_U8 *)pDestMacAddr)[0] & 0x01);
        else
            unicastDst = HAL_DEFAULT_UNICAST_ENABLED;

        pBd->bdRate = (unicastDst)? HAL_TXBD_BDRATE_DEFAULT : HAL_BDRATE_BCDATA_FRAME;
        pBd->rmf    = HAL_RMF_DISABLED;     

        /* sanity: Might already be set by caller, but enforce it here again */
        if( SIR_MAC_DATA_NULL == (subType & ~SIR_MAC_DATA_QOS_DATA)){
            disableFrmXtl = 1;
            if (txFlag & HAL_TXCOMP_REQUESTED_MASK) 
            {
                pBd->dpuRF = BMUWQ_FW_TRANSMIT; //Send to FW to transmit NULL frames.
            }
            else
            {
#ifdef LIBRA_WAPI_SUPPORT
                if (txFlag & HAL_WAPI_STA_MASK)
                {
                    pBd->dpuRF = BMUWQ_WAPI_DPU_TX;
                    //set NE bit to 1 for the null/qos null frames
                    pBd->dpuNE = HAL_NO_ENCRYPTION_ENABLED;
                }
#endif
            }
		}
#if defined(WLAN_PERF) || defined(FEATURE_WLAN_WAPI) || defined(LIBRA_WAPI_SUPPORT)
        //For not-NULL data frames
        else{
#if defined(FEATURE_WLAN_WAPI)
            //If caller doesn't want this frame to be encrypted, for example, WAI packets
            if( (txFlag & HAL_TX_NO_ENCRYPTION_MASK) )
            {
                pBd->dpuNE = HAL_NO_ENCRYPTION_ENABLED;
            }
#endif //defined(FEATURE_WLAN_WAPI)
#ifdef LIBRA_WAPI_SUPPORT
            if (txFlag & HAL_WAPI_STA_MASK)
            {
                pBd->dpuRF = BMUWQ_WAPI_DPU_TX;
            }
#endif //LIBRA_WAPI_SUPPORT
#if defined(WLAN_PERF)
            txBdSignature = computeTxBdSignature(pMac, pDestMacAddr, tid, unicastDst);
#endif //defined(WLAN_PERF)
        }
#endif        
    }
    else if (type == SIR_MAC_MGMT_FRAME)
    {

        /* - Set common fields for mgmt frames
         *     bd_ssn: Always let DPU auto generate seq # from the nonQos sequence number counter.
         *     bd_rate:Always use bcast mgmt rate for ucast/mcast mgmt frames
         *             Mgmt frames are relatively rare and so using a lower rate would have little or no 
         *             impact on anything. The result of losing a mgmt frame (ba or tspec or other action 
         *             frames) can be more severe than for losing a data frame, so it might be a good idea
         *             to send all mgmt frames at lower rates.
         *     rmf:    NOT SET here. would be set later after STA id lookup is done.
         * - Sanity: Force HW frame translation OFF for mgmt frames.
         */
         pBd->bdRate = HAL_BDRATE_BCMGMT_FRAME; /* apply to both ucast/mcast mgmt frames */
         pBd->bd_ssn = HAL_TXBD_BD_SSN_FILL_DPU_NON_QOS;
         if((subType == SIR_MAC_MGMT_ACTION) || (subType == SIR_MAC_MGMT_DEAUTH) || 
            (subType == SIR_MAC_MGMT_DISASSOC))
            isRMF = 1;
         disableFrmXtl = 1;
         
        /* 
         * Always use Addr1 byte0 to decide unicast/bcast 
         */
         unicastDst = !(((tANI_U8 *)pDestMacAddr)[0] & 0x01);

    } else {    // Control Packet
        /* We should never get a control packet, asserting here since something is wrong */
        VOS_ASSERT(0);
    }

    pBd->ub = !unicastDst;

    /* Fast path: Leverage UMA for BD filling/frame translation.
     * Must be a data frame to request for FT.
     * When HW frame translation is enabled, UMA fills in the following fields:
     *   DPU Sig 
     *   DPU descriptor index
     *   Updates MPDU header offset, data offset, MPDU length after translation
     *   STA id
     *   BTQM Queue ID
     */

    pBd->ft = pMac->hal.halMac.frameTransEnabled & !disableFrmXtl;


    if( !pBd->ft){

        /* - Slow path: Frame translation is disabled. Need to set the following fields:
         *    STA id
         *    DPU Sig 
         *    DPU descriptor index
         *    BTQM Queue ID
         * - For mgmt frames, also update rmf bits
         */
    
        tpStaStruct  pSta = (tpStaStruct) pMac->hal.halMac.staTable;
        tANI_U8 staId;

        pBd->ft = 0;/* Disable frame translation*/
#ifdef WLAN_PERF
        txBdSignature = HAL_TXBD_SIG_MGMT_MAGIC; /* Mark the BD could not be reused */
#endif
        
        /* If frame is a mgmt frame, always use self STA id.
         * Also if STA overwriting is requested, use self station id.
         */

        if((type == SIR_MAC_MGMT_FRAME) || (txFlag & HAL_USE_SELF_STA_REQUESTED_MASK))
        {
            staId = (tANI_U8)pMac->hal.halMac.selfStaId;
        }else{

            /* Look-up staId by data frame's DA
             * If incoming frame is 802.11 frame then the look-up is based on Addr1, otherwise it is 
             * DA field in 802.3/Ethernet frame.
             */
            status = halTable_FindStaidByAddr((tHalHandle)pMac, *(tSirMacAddr *)pDestMacAddr, &staId);

            /* Normally if incoming frame is 802.3/Ethernet, there won't be a match. 
             * If there is indeed a match, in Gen6 use cases, it means DA is an IBSS peer or BTAMP STA
             * In case there is no match, use peer MAC (BSSID) and search again.
             */
            if (eHAL_STATUS_SUCCESS != status)
            {
                cfgLen = SIR_MAC_ADDR_LENGTH;

                /* Look-up failed, get the Peer MAC from Config */
                status = wlan_cfgGetStr(pMac, WNI_CFG_BSSID, (tANI_U8 *)&macAddr, &cfgLen);

                if (eHAL_STATUS_SUCCESS != status)
                    return VOS_STATUS_E_FAILURE;

                /* Look-up BSSID from STA table */
                status = halTable_FindStaidByAddr((tHalHandle)pMac, macAddr, &staId);

                if (eHAL_STATUS_SUCCESS != status)
                    return VOS_STATUS_E_FAILURE;
            }
            /* else matched: IBSS or BTAMP STA*/
            if (staId >= pMac->hal.halMac.maxSta)
                return VOS_STATUS_E_FAILURE;
          
        }

        pBd->staIndex = staId;
        
        pSta += staId;  // Go to the curresponding station's station table

        if(type == SIR_MAC_MGMT_FRAME){

            if (unicastDst) {
                /* Assigning Queue Id configured to Ack */
                pBd->queueId = BTQM_QUEUE_SELF_STA_UCAST_MGMT;
            } else {
                /* Assigning to Queue Id configured to No Ack */
                pBd->queueId = BTQM_QUEUE_SELF_STA_BCAST_MGMT;
            }

            if(isRMF && pSta->rmfEnabled){
                pBd->rmf = 1;
                if(!unicastDst)
                    pBd->dpuDescIdx = pSta->bcastMgmtDpuIndex; /* IGTK */
                else
                    pBd->dpuDescIdx = pSta->dpuIndex; /* PTK */
            }else{
                pBd->dpuNE = HAL_NO_ENCRYPTION_ENABLED;  
                pBd->rmf = 0;
                pBd->dpuDescIdx = pSta->dpuIndex; /* PTK for both u/mcast mgmt frames */
            }
        }else{
            /* data frames */
            /* TID->QID is one-to-one mapping, the same way as followed in H/W */
            tANI_U8 queueId = 0;

            if(unicastDst && (staId == pMac->hal.halMac.selfStaId))
                pBd->queueId = BTQM_QUEUE_SELF_STA_UCAST_DATA;
            else if (pSta->qosEnabled) {
	        halBmu_get_qid_for_qos_tid(pMac, tid, &queueId);
                pBd->queueId = (tANI_U32) queueId;
	    }
            else
                pBd->queueId = BTQM_QUEUE_TX_nQOS;
            if(unicastDst){
                pBd->dpuDescIdx = pSta->dpuIndex; /*unicast data frames: PTK*/
            }else{
                pBd->dpuDescIdx = pSta->bcastDpuIndex; /* mcast data frames: GTK*/
            }
        }


        if(eHAL_STATUS_SUCCESS == halDpu_GetSignature(pMac, pSta->dpuIndex, &dpuSig))
            pBd->dpuSignature = dpuSig;
        else{   
            HALLOGE(halLog(pMac, LOGE, FL("halDpu_GetSignature() failed for dpuId = %d\n"), pBd->dpuDescIdx));
            return VOS_STATUS_E_FAILURE;
        }

    } 
    
    /* Over SDIO bus, SIF won't swap data bytes to/from data FIFO. 
     * In order for MAC modules to recognize BD in Libra's default endian format (Big endian)
     * All BD fields need to be swaped here
     */
    WLANHAL_SwapTxBd((tANI_U8 *)pBd);

#ifdef WLAN_PERF
    /* Save the BD signature. This field won't be swapped and remains in host byte order */
    pBd->txBdSignature = txBdSignature ;
#endif        
    
    if(HAL_STATUS_SUCCESS(status))
        return VOS_STATUS_SUCCESS;
    else
        return VOS_STATUS_SUCCESS;
}


/**
 * DESCRIPTION:
 *      Helper routine provide for TL to extract type, subtype from
 *      RxBD of the frame
 *
 * PARAMETERS:
 *      void*:  Pointer to the frame which starts with RxBD header.
 *
 * RETURN VALUE:
 *      8 bit value which contains both type/subtype info
 */
tANI_U8 WLANHAL_RxBD_GetFrameTypeSubType(v_PVOID_t _pvBDHeader, tANI_U16 usFrmCtrl)
{
    tpHalRxBd pRxBd = (tpHalRxBd) _pvBDHeader;
    tANI_U8    typeSubType;
    tSirMacFrameCtl  sirFrmCtl; 
    
    if (pRxBd->ft != WLANHAL_RX_BD_FT_DONE)
    {
      /*!!!! SHOULD NOT DO THIS, the input is only guaranteed to be the BH header 
           there's no guarantee that the packet will follow: 
             
        tpSirMacFrameCtl pFrmCtl = 
            (tpSirMacFrameCtl) (((tANI_U8 *)pRxBd) + pRxBd->mpduHeaderOffset);

        typeSubType = (pFrmCtl->type << HAL_FRAME_TYPE_OFFSET) |
                        pFrmCtl->subType;*/

	  if (pRxBd->asf)
      {
	      typeSubType = (SIR_MAC_DATA_FRAME << HAL_FRAME_TYPE_OFFSET) |
							  SIR_MAC_DATA_QOS_DATA;
	  }else{
		
        vos_mem_copy(&sirFrmCtl, &usFrmCtrl, sizeof(sirFrmCtl)); 
        typeSubType = (sirFrmCtl.type << HAL_FRAME_TYPE_OFFSET) |
                        sirFrmCtl.subType;
	  }
    }
    else
    {
        /* !!!! SHOULD NOT DO THIS, the input is only guaranteed to be the BH header 
           there's no guarantee that the packet will follow: 

        tpSirMacFrameCtl  psirFrmCtl = 
            (tpSirMacFrameCtl) (((tANI_U8 *)pRxBd) + WLANHAL_RX_BD_HEADER_SIZE);*/

        vos_mem_copy(&sirFrmCtl, &usFrmCtrl, sizeof(sirFrmCtl)); 
        typeSubType = (sirFrmCtl.type << HAL_FRAME_TYPE_OFFSET) |
                        sirFrmCtl.subType;
    }
    
    return typeSubType;
}

void halTLGetTxPktCount(tpAniSirGlobal pMac, tANI_U8 curSta, tANI_U8 tId, tANI_U32 *txPcktCount )
{
   v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);

   WLANTL_GetTxPktCount(pVosGCtx, curSta, tId, (v_U32_t*)txPcktCount); 
}

void halTLGetRxPktCount(tpAniSirGlobal pMac, tANI_U8 curSta, tANI_U8 tId, tANI_U32 *rxPcktCount )
{
   v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);

   WLANTL_GetRxPktCount(pVosGCtx, curSta, tId, (v_U32_t*)rxPcktCount); 
}

/**
 * DESCRIPTION:
 *      Notify TL whenever the RSSI threshold hits low/high.
 *    It is based on the system configuration programmed in the firmware
 *
 * PARAMETERS:
 *      pRSSINotification:  Pointer to the detected RSSI thresholds.
 *
 * RETURN VALUE:
 *      void
 */
void halTLRSSINotification(tpAniSirGlobal pMac, tpSirRSSINotification pRSSINotification)
{
    //Its upto TL to act accordinly based on this detection.
    //It has to use the halPS_SetRSSIThresholds interface to set the new RSSI thresholds
    //For more info, look into CR#: 165114 
#ifdef FEATURE_WLAN_GEN6_ROAMING
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
    WLANTL_BMPSRSSIRegionChangedNotification(pVosGCtx, pRSSINotification);
#endif
    return;
}

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
/*
 * DESCRIPTION:
 *      Update the FW system config with the paramters for the particular AC, 
 *      when UAPSD session is added for a given AC.
 *
 * PARAMETERS:
 *      pvosGCtx:   pointer to the global vos context;a handle to HAL's 
 *                  control block can be extracted from its context 
 *      staIdx:     Station index
 *      pUapsdInfo: pointer to the UAPSD paramters for a given AC.
 *
 * RETURN:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_E_FAILURE
 */
VOS_STATUS WLANHAL_EnableUapsdAcParams(void* pVosGCtx, tANI_U8 staIdx, tUapsdInfo *pUapsdInfo)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;
    eHalStatus status = eHAL_STATUS_FAILURE;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    pFwConfig->acParam[pUapsdInfo->ac].usSrvIntrMs  = pUapsdInfo->srvInterval;
    pFwConfig->acParam[pUapsdInfo->ac].ucUp         = pUapsdInfo->up;
    pFwConfig->acParam[pUapsdInfo->ac].uSuspIntrMs  = pUapsdInfo->susInterval;
    pFwConfig->acParam[pUapsdInfo->ac].uDelayIntrMs = pUapsdInfo->delayInterval;

    // Write the UAPSD params in the sysConfig
    status = halFW_UpdateSystemConfig(pMac,pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(*pFwConfig));
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("FW system config update FAILED!")));
        return VOS_STATUS_E_FAILURE;
    }

    return VOS_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Update the FW system config with the paramters for the particular AC,
 *      when UAPSD session is deleted.
 *
 * PARAMETERS:
 *      pvosGCtx:   pointer to the global vos context;a handle to HAL's 
 *                  control block can be extracted from its context 
 *      staIdx:     Station index
 *      ac:         access category for which UAPSD is being disabled.
 *
 * RETURN:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_E_FAILURE
 */
VOS_STATUS WLANHAL_DisableUapsdAcParams(void* pVosGCtx, tANI_U8 staIdx, tANI_U8 ac)
{
    return VOS_STATUS_SUCCESS;
}

#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

/*
 * DESCRIPTION:
 *      Routine to enable the Idle BD PDU interrupt on invocation from TL     
 *
 * PARAMETERS:
 *      pvosGCtx:   pointer to the global vos context;a handle to HAL's 
 *                  control block can be extracted from its context 
 *      idleBdPduThreshold:     Threshold to be set in BMU to get the interrupt
 *
 * RETURN:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_E_FAILURE
 */
VOS_STATUS WLANHAL_EnableIdleBdPduInterrupt(void* pVosGCtx, tANI_U8 idleBdPduThreshold)
{
    tpAniSirGlobal  pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, (v_CONTEXT_t) pVosGCtx);    
    return halBmu_EnableIdleBdPduInterrupt(pMac, idleBdPduThreshold);
}

/*
 * DESCRIPTION:
 *      Routing to handle the idle BD/PDU interrupt from BMU interrupt handler      
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *                  control block can be extracted from its context 
 *
 * RETURN:
 *      NONE
 */
void halTLHandleIdleBdPduInterrupt(tpAniSirGlobal pMac)
{
   v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
   WLANTL_PostResNeeded(pVosGCtx);
}
