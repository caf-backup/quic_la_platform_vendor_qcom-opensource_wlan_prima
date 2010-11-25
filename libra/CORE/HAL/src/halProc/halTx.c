/* Standard include files */

/* Application Specific include files */

#include "palTypes.h"
#include "palApi.h"
#include "halInternal.h"
#include "halTypes.h"
#include "halBmu.h"
#include <palApi.h>
#include "halDebug.h"
#include "vos_types.h"
#include "vos_api.h"
#include "wlan_qct_tl.h"
#include "halTLApi.h"
#include "halGlobal.h"
#include "limUtils.h" //this inclusion is temporary  in order to call limTxComplete directly from halTxComplete.


#define HAL_TL_TX_MGMT_FRAME_TIMEOUT  5000 // units in msec a very high upper limit of 5,000 msec

/* Constant Macros */
/* Function Macros */

/*

typedef VOS_STATUS (*WLANTL_TxCompCBType)( v_PVOID_t      pvosGCtx,
                                           vos_pkt_t*     vosDataBuff,
                                           VOS_STATUS     wTxSTAtus );
*/


// -------------------------------------------------------------
/**
 * halTxFrame
 *
 * FUNCTION:
 *     Transmits frames from higher level software.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @return tSirRetStatus SUCCESS or FAILURE
 */
__DP_SRC_TX  eHalStatus halTxFrame(tHalHandle hHal,
                      void *pFrmBuf,
                      tANI_U16 frmLen,
                      eFrameType frmType,
                      eFrameTxDir txDir,
                      tANI_U8 tid,
                      pHalTxRxCompFunc pCompFunc,
                      void *pData)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, hHal);
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    tpSirMacFrameCtl pFc = (tpSirMacFrameCtl ) pData;
    tANI_U8 ucTypeSubType = pFc->type <<4 | pFc->subType;
    tANI_U8 eventIdx = 0, txFlag = 0;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;

    HALLOG1( halLog(pMac, LOG1, FL("Tx Mgmt Frame Subtype: %d alloc(%x)\n"), pFc->subType, pFrmBuf));
    sirDumpBuf(pMac, SIR_HAL_MODULE_ID, LOG4, pData, frmLen);
    //MTRACE(macTrace(pMac, TRACE_CODE_TX_MGMT, 0, pFc->subType);)

    // Reset the event to be not signalled
    vosStatus = vos_event_reset(&pMac->hal.TLParam.txMgmtFrameEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE(halLog(pMac, LOGE, FL("VOS Event reset failed - status = %d\n"), 
                    vosStatus));
        halTxComplete(pVosGCtx, (vos_pkt_t *)pFrmBuf, vosStatus);
        return eHAL_STATUS_FAILURE;
    }

    pMac->hal.TLParam.txMgmtFrameStatus = HAL_TL_TX_FAILURE;
    // Get system role, use the self station if in unknown role or STA role
    systemRole = halGetGlobalSystemRole(pMac);
    HALLOG1(halLog(pMac, LOG1, FL("SystemRole=%d\n"),systemRole));
    if ((systemRole == eSYSTEM_UNKNOWN_ROLE) || (systemRole == eSYSTEM_STA_ROLE)) {
        txFlag = HAL_USE_SELF_STA_REQUESTED_MASK;     
    }

    // Divert Disassoc/Deauth frame thr self station, as by the time unicast 
    // disassoc frame reaches the HW, HAL has already deleted the peer station
    if ((pFc->type == SIR_MAC_MGMT_FRAME)) {
        if ((pFc->subType == SIR_MAC_MGMT_DISASSOC) || 
             (pFc->subType == SIR_MAC_MGMT_DEAUTH) || 
                (pFc->subType == SIR_MAC_MGMT_REASSOC_RSP)) {
        txFlag = HAL_USE_SELF_STA_REQUESTED_MASK;
        } 

        // Since we donot want probe responses to be retried, send probe responses
        // through the NO_ACK queues
        if (pFc->subType == SIR_MAC_MGMT_PROBE_RSP) {
            txFlag = HAL_USE_NO_ACK_REQUESTED_MASK;
        }
    }

    if(  (vosStatus = WLANTL_TxMgmtFrm(pVosGCtx, (vos_pkt_t *)pFrmBuf, frmLen, 
                    ucTypeSubType, tid, 
                    halTxComplete, NULL, txFlag)) != VOS_STATUS_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Sending Mgmt Frame failed - status = %d\n"), 
                    vosStatus));
        halTxComplete(pVosGCtx, (vos_pkt_t *)pFrmBuf, vosStatus);
        return eHAL_STATUS_FAILURE;
    }

    // Wait for the event to be set by the TL, to get the response of TX 
    // complete, this event should be set by the Callback function called by TL
    vosStatus = vos_wait_events(&pMac->hal.TLParam.txMgmtFrameEvent, 1, 
            HAL_TL_TX_MGMT_FRAME_TIMEOUT, &eventIdx);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE(halLog(pMac, LOGE, FL("VOS Event wait failed - status = %d\n"), 
                    vosStatus));
        return eHAL_STATUS_FAILURE;
    }
    if(pMac->hal.TLParam.txMgmtFrameStatus != HAL_TL_TX_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("MGMT SEND ---> FAILURE 1")));
        return eHAL_STATUS_FAILURE;
    }
    HALLOG1( halLog(pMac, LOG1, FL("MGMT SEND ---> SUCCESS %d"), eventIdx));
    return eHAL_STATUS_SUCCESS;

}

/**
 * halTxFrameWithTxComplete
 * This function will replace halTxFrame() when PE is ready to use this. 
 *
 * FUNCTION:
 *     Transmits frames from higher level software.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @param pCBackFnTxComp: NULL - tx complete not requested, otherwise tx complete requested.
 * @return tSirRetStatus SUCCESS or FAILURE
 */
__DP_SRC_TX  eHalStatus halTxFrameWithTxComplete(tHalHandle hHal,
                      void *pFrmBuf,
                      tANI_U16 frmLen,
                      eFrameType frmType,
                      eFrameTxDir txDir,
                      tANI_U8 tid,
                      pHalTxRxCompFunc pCompFunc,
                      void *pData,
                      tpCBackFnTxComp pCBackFnTxComp)
{
    eHalStatus retCode = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, hHal);
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
    tpSirMacFrameCtl pFc = (tpSirMacFrameCtl ) pData;
    tANI_U8 ucTypeSubType = pFc->type <<4 | pFc->subType;
    tANI_U8 ackRsp = 0;
    HALLOG1(halLog(pMac, LOG1, FL("Tx Mgmt Frame Subtype: %d\n"), pFc->subType));
    sirDumpBuf(pMac, SIR_HAL_MODULE_ID, LOG4, pData, frmLen);
    MTRACE(macTrace(pMac, TRACE_CODE_TX_MGMT, 0, pFc->subType);)
                        
    if(pCBackFnTxComp)
    {
        if(pMac->hal.pCBackFnTxComp == NULL)
        {
            ackRsp = 1;
            pMac->hal.pCBackFnTxComp = pCBackFnTxComp;        
            if(TX_SUCCESS != tx_timer_activate(&pMac->hal.txCompTimer)) //wait timer started. 
            {
                HALLOGE(halLog(pMac, LOGE, FL("could not activate txCompTimer\n")));
            }
        }
        else
        {
                retCode = eHAL_STATUS_FAILURE;
                HALLOGE(halLog(pMac, LOGE, FL("There is already one request pending for tx complete\n")));
        }
    }    
        

    if(eHAL_STATUS_SUCCESS == retCode)
    {
        vosStatus = WLANTL_TxMgmtFrm(pVosGCtx, (vos_pkt_t*)pFrmBuf, frmLen, ucTypeSubType, tid, (WLANTL_TxCompCBType)halTxComplete, NULL, ackRsp);
        if(!VOS_IS_STATUS_SUCCESS(vosStatus))
            retCode = eHAL_STATUS_FAILURE;
    } 
    return retCode;
}    

__DP_SRC_TX VOS_STATUS halTxComplete( v_PVOID_t pVosGCtx, vos_pkt_t *pData, VOS_STATUS status )
//__DP_SRC_TX VOS_STATUS halTxComplete( v_CONTEXT_t pVosGCtx, void *pData, VOS_STATUS status )
//#else
//__DP_SRC_TX void halTxComplete( tHalHandle hHal, tpHalFrameCb pFcb)
//#endif
{
    tHalHandle hHal = (tHalHandle) vos_get_context(VOS_MODULE_ID_HAL, pVosGCtx);
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

    if(VOS_IS_STATUS_SUCCESS(status)) {
        pMac->hal.TLParam.txMgmtFrameStatus = HAL_TL_TX_SUCCESS;
        HALLOG1(halLog(pMac, LOG1, FL("NEW VOS Event SUCCESS = %d\n"), vosStatus));
    }

    // Trigger the event to bring the HAL TL Tx complete function to come out of wait
    vosStatus = vos_event_set(&pMac->hal.TLParam.txMgmtFrameEvent);
    if(!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE(halLog(pMac, LOGE, FL("NEW VOS Event Set failed - status = %d\n"), vosStatus));
    }    

    limTxComplete(hHal, pData);
    return VOS_STATUS_SUCCESS;
}

