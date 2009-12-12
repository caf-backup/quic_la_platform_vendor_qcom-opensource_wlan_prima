/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessMlmRspMessages.cc contains the code
 * for processing response messages from MLM state machine.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "wniApi.h"
#ifdef ANI_PRODUCT_TYPE_AP
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limTimerUtils.h"
#include "limSendMessages.h"
#include "limAdmitControl.h"
#include "limSendMessages.h"
#include "limIbssPeerMgmt.h"

static void limHandleSmeJoinResult(tpAniSirGlobal, tSirResultCodes, tANI_U16);
void limProcessMlmScanCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmJoinCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAuthCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmStartCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAuthInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAssocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAssocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmReassocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmReassocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmSetKeysCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDisassocInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDisassocCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDeauthInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDeauthCnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmPurgeStaInd(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmAddBACnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmDelBACnf(tpAniSirGlobal, tANI_U32 *);
void limProcessMlmRemoveKeyCnf(tpAniSirGlobal pMac, tANI_U32 * pMsgBuf);
static void  limHandleDelBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs);


/**
 * limProcessMlmRspMessages()
 *
 *FUNCTION:
 * This function is called to processes various MLM response (CNF/IND
 * messages from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param  msgType   Indicates the MLM message type
 * @param  *pMsgBuf  A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmRspMessages(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    switch (msgType)
    {
        case LIM_MLM_SCAN_CNF:
            limProcessMlmScanCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_AUTH_CNF:
            limProcessMlmAuthCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_AUTH_IND:
            limProcessMlmAuthInd(pMac, pMsgBuf);

            break;

        case LIM_MLM_ASSOC_CNF:
            limProcessMlmAssocCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_START_CNF:
            limProcessMlmStartCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_JOIN_CNF:
            limProcessMlmJoinCnf(pMac, pMsgBuf);

            break;

#ifdef ANI_PRODUCT_TYPE_AP
        case LIM_MLM_ASSOC_IND:
            if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
                limProcessMlmAssocInd(pMac, pMsgBuf);

            break;
#endif

        case LIM_MLM_REASSOC_CNF:
            limProcessMlmReassocCnf(pMac, pMsgBuf);

            break;

#ifdef ANI_PRODUCT_TYPE_AP
        case LIM_MLM_REASSOC_IND:
            if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
                limProcessMlmReassocInd(pMac, pMsgBuf);

            break;
#endif

        case LIM_MLM_DISASSOC_CNF:
            limProcessMlmDisassocCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_DISASSOC_IND:
            limProcessMlmDisassocInd(pMac, pMsgBuf);

            break;

        case LIM_MLM_PURGE_STA_IND:
            limProcessMlmPurgeStaInd(pMac, pMsgBuf);

            break;

        case LIM_MLM_DEAUTH_CNF:
            limProcessMlmDeauthCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_DEAUTH_IND:
            limProcessMlmDeauthInd(pMac, pMsgBuf);

            break;

        case LIM_MLM_SETKEYS_CNF:
            limProcessMlmSetKeysCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_REMOVEKEY_CNF:
            limProcessMlmRemoveKeyCnf(pMac, pMsgBuf);

            break;

        case LIM_MLM_TSPEC_CNF:

            break;

        case LIM_MLM_ADDBA_CNF:
            limProcessMlmAddBACnf( pMac, pMsgBuf );
            break;

        case LIM_MLM_DELBA_CNF:
            limProcessMlmDelBACnf( pMac, pMsgBuf );
            break;

        default:

            break;
    } // switch (msgType)

    return;
} /*** end limProcessMlmRspMessages() ***/



/**
 * limProcessMlmScanCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_SCAN_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmScanCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    switch(pMac->lim.gLimSmeState)
    {
        case eLIM_SME_WT_SCAN_STATE:
        case eLIM_SME_LINK_EST_WT_SCAN_STATE:
        case eLIM_SME_NORMAL_CHANNEL_SCAN_STATE:
            pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
            pMac->lim.gLimSystemInScanLearnMode = 0;

            break;

        default:
            /**
             * Should not have received scan confirm
             * from MLM in other states.
             * Log error
             */
            limLog(pMac, LOGE,
               FL("received unexpected MLM_SCAN_CNF in state %X\n"),
               pMac->lim.gLimSmeState);

            return;
    }

    /// Process received scan confirm

    /// Increment length of cached scan results
    pMac->lim.gLimSmeScanResultLength +=
                    ((tLimMlmScanCnf *) pMsgBuf)->scanResultLength;

    if ((pMac->lim.gLimRspReqd) || pMac->lim.gLimReportBackgroundScanResults)
    {
        tANI_U16    scanRspLen = 0;

        /// Need to send response to Host
        pMac->lim.gLimRspReqd = false;

        if ((((tLimMlmScanCnf *) pMsgBuf)->resultCode ==
                                                eSIR_SME_SUCCESS) ||
            pMac->lim.gLimSmeScanResultLength)
        {
                    scanRspLen = sizeof(tSirSmeScanRsp) +
                                 pMac->lim.gLimSmeScanResultLength -
                                 sizeof(tSirBssDescription);
        }
        else
        {
            scanRspLen = 0x8;
        }

       if(pMac->lim.gLimReportBackgroundScanResults)
        {
            pMac->lim.gLimBackgroundScanTerminate = TRUE;
        }

        if (pMac->lim.gLimSmeScanResultLength == 0)
        {
            limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS);
        }
        else
        {
            limSendSmeScanRsp(pMac, scanRspLen,
                              eSIR_SME_SUCCESS);
        }
    } // if (pMac->lim.gLimRspReqd)
    
    //check to see whether we need to run bgScan timer
    if(pMac->lim.gLimBackgroundScanTerminate == FALSE)
    {
        if (tx_timer_activate(
            &pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
        {
            /// Could not activate background scan timer.
            // Log error
            limLog(pMac, LOGP,
            FL("could not activate background scan timer\n"));
            pMac->lim.gLimBackgroundScanStarted = FALSE;
        }
        else
        {
            pMac->lim.gLimBackgroundScanStarted = TRUE;
        }
    }
} /*** end limProcessMlmScanCnf() ***/



/**
 * limProcessMlmStartCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_START_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmStartCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    if (pMac->lim.gLimSmeState != eLIM_SME_WT_START_BSS_STATE)
    {
        /**
         * Should not have received Start confirm from MLM
         * in other states.
         * Log error
         */
        limLog(pMac, LOGE,
           FL("received unexpected MLM_START_CNF in state %X\n"),
           pMac->lim.gLimSmeState);

        return;
    }

    if (((tLimMlmStartCnf *) pMsgBuf)->resultCode ==
                                            eSIR_SME_SUCCESS)
    {
        /**
         * Update global SME state so that Beacon Generation
         * module starts writing Beacon frames into TFP's
         * Beacon file register.
         */
        pMac->lim.gLimSmeState = eLIM_SME_NORMAL_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        PELOG1(limLog(pMac, LOG1, FL("*** Started BSS ***\n"));)
    }
    else
    {
        /// Start BSS is a failure
        pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
        palFreeMemory( pMac->hHdd, pMac->lim.gpLimStartBssReq);
        pMac->lim.gpLimStartBssReq = NULL;
    }

    /// Send response to Host
    limSendSmeStartBssRsp(pMac, eWNI_SME_START_BSS_RSP,
                          ((tLimMlmStartCnf *) pMsgBuf)->resultCode);
} /*** end limProcessMlmStartCnf() ***/



/**
 * limProcessMlmJoinCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_JOIN_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmJoinCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes resultCode;

    if (pMac->lim.gLimSmeState != eLIM_SME_WT_JOIN_STATE)
    {
        /**
         * Should not have received Join confirm from MLM
         * in other states.
         * Log error
         */
        PELOGE(limLog(pMac, LOGE,   FL("received unexpected MLM_JOIN_CNF in state %X\n"),
                                  pMac->lim.gLimSmeState);)

        return;
    }

    resultCode = ((tLimMlmJoinCnf *) pMsgBuf)->resultCode ;
    /// Process Join confirm from MLM

    if (resultCode ==  eSIR_SME_SUCCESS)
    {
            PELOG1(limLog(pMac, LOG1, FL("*** Joined ESS ***\n"));)
                
            //Setup hardware upfront
           PELOGE(limLog(pMac, LOGE, FL("*** Starting to add BSS***\n"));)  //remove me
            if(limStaSendAddBssPreAssoc( pMac, false) == eSIR_SUCCESS)
                return;
            else
                resultCode = eSIR_SME_REFUSED;
    }

    {
        /// Join failure
        pMac->lim.gLimSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        /// Send Join response to Host
        limHandleSmeJoinResult(pMac, resultCode, ((tLimMlmJoinCnf *) pMsgBuf)->protStatusCode );
    }
                                                                          
} /*** end limProcessMlmJoinCnf() ***/



/**
 * limProcessMlmAuthCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_AUTH_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmAuthCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                caps;
    tANI_U32                val;
    tAniAuthType       cfgAuthType, authMode;
    tLimMlmAuthReq     *pMlmAuthReq;
    tLimMlmAssocReq    *pMlmAssocReq;

    if (((pMac->lim.gLimSmeState != eLIM_SME_WT_AUTH_STATE) &&
         (pMac->lim.gLimSmeState != eLIM_SME_WT_PRE_AUTH_STATE)) ||
        (pMac->lim.gLimSystemRole == eLIM_AP_ROLE))
    {
        /**
         * Should not have received AUTH confirm
         * from MLM in other states or on AP.
         * Log error
         */
        limLog(pMac, LOGE,
               FL("received unexpected MLM_AUTH_CNF in state %X\n"),
               pMac->lim.gLimSmeState);

        return;
    }

    /// Process AUTH confirm from MLM

    if (((tLimMlmAuthCnf *) pMsgBuf)->resultCode != eSIR_SME_SUCCESS)
    {
        if (pMac->lim.gLimSmeState == eLIM_SME_WT_AUTH_STATE)
                {
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATION_TYPE,
                          (tANI_U32 *) &cfgAuthType) != eSIR_SUCCESS)
            {
                /**
                 * Could not get AuthType value from CFG.
                 * Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve AuthType value\n"));
            }
                }
        else
            cfgAuthType = pMac->lim.gLimPreAuthType;

        if ((cfgAuthType == eSIR_AUTO_SWITCH) &&
            (((tLimMlmAuthCnf *) pMsgBuf)->authType == eSIR_SHARED_KEY))
        {
            /**
             * When Shared key authentication fails with
             * authType set to 'auto switch', revert to
             * Open System Authentication
             */
            authMode = eSIR_OPEN_SYSTEM;

            // Trigger MAC based Authentication
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAuthReq, sizeof(tLimMlmAuthReq)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmAuthReq\n"));
            }

            val = sizeof(tSirMacAddr);
            if (pMac->lim.gLimSmeState == eLIM_SME_WT_AUTH_STATE)
                        {
                if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID,
                              pMlmAuthReq->peerMacAddr,
                              &val) != eSIR_SUCCESS)
                {
                    /// Could not get BSSID from CFG. Log error.
                    limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
                }
                        }
            else
                palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmAuthReq->peerMacAddr,
                              (tANI_U8 *) &pMac->lim.gLimPreAuthPeerAddr,
                              sizeof(tSirMacAddr));

            pMlmAuthReq->authType = authMode;

            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAuthReq->authFailureTimeout)
                            != eSIR_SUCCESS)
            {
                /**
                 * Could not get AuthFailureTimeout value from CFG.
                 * Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
            }
            limPostMlmMessage(pMac,
                              LIM_MLM_AUTH_REQ,
                              (tANI_U32 *) pMlmAuthReq);

            return;
        }
        else
        {
            // MAC based authentication failure
            tSirMacAddr currentBssId;
            tANI_U32 cfg = sizeof(tSirMacAddr);

            if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                                eSIR_SUCCESS)
            {
                /// Could not get BSSID from CFG. Log error.
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
                return;
            }

            if (pMac->lim.gLimSmeState == eLIM_SME_WT_AUTH_STATE)
            {
                
                PELOGE(limLog(pMac, LOGE, FL("Auth Failure occurred.\n"));)
                pMac->lim.gLimSmeState = eLIM_SME_JOIN_FAILURE_STATE;
	         MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
                pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
		  MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
                //configure RXP to start filtering the frames.
                if(limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, currentBssId) != eSIR_SUCCESS)
                    PELOGE(limLog(pMac, LOGE,  FL("Failed to set the LinkState.\n"));)
                
#if defined(ANI_AP_CLIENT_SDK)
                if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
                {
                    tSirMacAddr nullMacAddr = {0, 0, 0, 0, 0, 0};
                    
                    PELOGE(limLog(pMac, LOGE, FL("Setting current BSSID as NULL in cfg\n"));)
                    palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));

                    if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
                    {
                        limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
                    }
                }
#endif

                /**
                 * Need to send Join response with
                 * auth failure to Host.
                 */
                limHandleSmeJoinResult(pMac,
                              ((tLimMlmAuthCnf *) pMsgBuf)->resultCode, ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode); 
            }
            else
            {
                /**
                 * Pre-authentication failure.
                 * Send Pre-auth failure response to host
                 */
                pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
		  MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

                limSendSmeAuthRsp(
                            pMac,
                            ((tLimMlmAuthCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAuthCnf *) pMsgBuf)->peerMacAddr,
                            ((tLimMlmAuthCnf *) pMsgBuf)->authType,
                            ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode );
            }
        } // end if (cfgAuthType == eAUTO_SWITCH)
    } // if (((tLimMlmAuthCnf *) pMsgBuf)->resultCode != ...
    else
    {
        if (pMac->lim.gLimSmeState == eLIM_SME_WT_AUTH_STATE)
        {
            /**
             * Successful MAC based authentication
             * Trigger Association with BSS
             */

            PELOG1(limLog(pMac, LOG1,
                   FL("*** Authenticated with BSS ***\n"));)

            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAssocReq, sizeof(tLimMlmAssocReq)))
            {
                // Log error
                limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmAssocReq\n"));

            }

            val = sizeof(tSirMacAddr);
            if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID,
                          pMlmAssocReq->peerMacAddr,
                          &val) != eSIR_SUCCESS)
            {
                /// Could not get BSSID from CFG. Log error.
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }

            if (wlan_cfgGetInt(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAssocReq->assocFailureTimeout)
                           != eSIR_SUCCESS)
            {
                /**
                 * Could not get AssocFailureTimeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AssocFailureTimeout value\n"));
            }

            if (cfgGetCapabilityInfo(pMac, &caps) != eSIR_SUCCESS)
            {
                /**
                 * Could not get Capabilities value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve Capabilities value\n"));
            }
            pMlmAssocReq->capabilityInfo = caps;
           PELOG3(limLog(pMac, LOG3,
               FL("Capabilities to be used in AssocReq=0x%X, privacy bit=%x\n"),
               caps,
               ((tpSirMacCapabilityInfo) &pMlmAssocReq->capabilityInfo)->privacy);)

            if (wlan_cfgGetInt(pMac, WNI_CFG_LISTEN_INTERVAL, &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get ListenInterval value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve ListenInterval\n"));
            }
            pMlmAssocReq->listenInterval = (tANI_U16)val;

            pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
            pMac->lim.gLimSmeState     = eLIM_SME_WT_ASSOC_STATE;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            limPostMlmMessage(pMac,
                              LIM_MLM_ASSOC_REQ,
                              (tANI_U32 *) pMlmAssocReq);
        }
        else
        {
            /**
             * Successful Pre-authentication.
             * Send Pre-auth response to host
             */
            pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            limSendSmeAuthRsp(
                        pMac,
                        ((tLimMlmAuthCnf *) pMsgBuf)->resultCode,
                        ((tLimMlmAuthCnf *) pMsgBuf)->peerMacAddr,
                        ((tLimMlmAuthCnf *) pMsgBuf)->authType,
                        ((tLimMlmAuthCnf *) pMsgBuf)->protStatusCode );
        }
    } // end if (((tLimMlmAuthCnf *) pMsgBuf)->resultCode != ...
} /*** end limProcessMlmAuthCnf() ***/



/**
 * limProcessMlmAssocCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_ASSOC_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmAssocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    if ((pMac->lim.gLimSmeState != eLIM_SME_WT_ASSOC_STATE) ||
        (pMac->lim.gLimSystemRole == eLIM_AP_ROLE))
    {
        /**
         * Should not have received Assocication confirm
         * from MLM in other states OR on AP.
         * Log error
         */
        limLog(pMac, LOGE,
               FL("received unexpected MLM_ASSOC_CNF in state %X\n"),
               pMac->lim.gLimSmeState);

        return;
    }

    if (((tLimMlmAssocCnf *) pMsgBuf)->resultCode != eSIR_SME_SUCCESS)
    {
        // Association failure

        PELOG1(limLog(pMac, LOG1, FL("*** Association failure ***\n"));)

        pMac->lim.gLimSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
        
#if defined(ANI_AP_CLIENT_SDK)
        if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
        {
            tSirMacAddr nullMacAddr = {0, 0, 0, 0, 0, 0};
            
            palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));

            if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
            }
        }
#endif
                
        /**
         * Need to send Join response with
         * Association failure to Host.
         */
        limHandleSmeJoinResult(pMac,
                            ((tLimMlmAssocCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAssocCnf *) pMsgBuf)->protStatusCode);
    } // if (((tLimMlmAssocCnf *) pMsgBuf)->resultCode != ...
    else
    {
        // Successful Association

        PELOG1(limLog(pMac, LOG1, FL("*** Associated with BSS ***\n"));)

        pMac->lim.gLimSmeState = eLIM_SME_LINK_EST_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        /**
         * Need to send Join response with
         * Association success to Host.
         */
        limHandleSmeJoinResult(pMac,
                            ((tLimMlmAssocCnf *) pMsgBuf)->resultCode,
                            ((tLimMlmAssocCnf *) pMsgBuf)->protStatusCode);
    } // end if (((tLimMlmAssocCnf *) pMsgBuf)->resultCode != ....
} /*** end limProcessMlmAssocCnf() ***/



/**
 * limProcessMlmReassocCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_REASSOC_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmReassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tLimMlmReassocCnf *pReassocCnf = (tLimMlmReassocCnf*) pMsgBuf;

    if ((pMac->lim.gLimSmeState != eLIM_SME_WT_REASSOC_STATE) ||
         (pMac->lim.gLimSystemRole == eLIM_AP_ROLE))
    {
        PELOGE(limLog(pMac, LOGE, 
                   FL("Rcv unexpected MLM_REASSOC_CNF in role %d, sme state 0x%X\n"), 
                   pMac->lim.gLimSystemRole, pMac->lim.gLimSmeState);)
        return;
    }

    palFreeMemory( pMac->hHdd, pMac->lim.gpLimReassocReq);
    pMac->lim.gpLimReassocReq = NULL;   
    PELOGE(limLog(pMac, LOGE, FL("Rcv MLM_REASSOC_CNF with result code %d\n"), pReassocCnf->resultCode);)

    pMac->lim.gLimSmeState = eLIM_SME_LINK_EST_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

    // Need to send Reassoc response to Host. 
    limSendSmeJoinReassocRsp( pMac, eWNI_SME_REASSOC_RSP,
                              pReassocCnf->resultCode, pReassocCnf->protStatusCode);

} /*** end limProcessMlmReassocCnf() ***/



#ifdef ANI_PRODUCT_TYPE_AP
/**
 * limProcessMlmReassocInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_REASSOC_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmReassocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32                len;
    tSirMsgQ           msgQ;
    tSirSmeReassocInd  *pSirSmeReassocInd;
    tpDphHashNode      pStaDs=0;

    /// Inform Host of STA reassociation
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    len = sizeof(tSirSmeReassocInd) -
          sizeof(tSirNeighborBssInfo) +
          (((tpLimMlmReassocInd) pMsgBuf)->numBss *
           sizeof(tSirNeighborBssInfo)) -
           SIR_MAC_MAX_SSID_LENGTH +
           ((tpLimMlmReassocInd) pMsgBuf)->ssId.length;
#else
    len = sizeof(tSirSmeReassocInd);
#endif
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeReassocInd, len))
    {
        // Log error
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_REASSOC_IND\n"));
    }
    sirStoreU16N((tANI_U8 *) &pSirSmeReassocInd->messageType,
                 eWNI_SME_REASSOC_IND);
    limReassocIndSerDes(pMac, (tpLimMlmReassocInd) pMsgBuf,
                        (tANI_U8 *) &(pSirSmeReassocInd->length));

    msgQ.type = eWNI_SME_REASSOC_IND;
    msgQ.bodyptr = pSirSmeReassocInd;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_REASSOC_IND_EVENT, pSirSmeReassocInd->peerMacAddr, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &msgQ,  ePROT);

    PELOG1(limLog(pMac, LOG1,
       FL("Create CNF_WAIT_TIMER after received LIM_MLM_REASSOC_IND\n"));)

    /*
     ** turn on a timer to detect the loss of REASSOC CNF
     **/

    pStaDs = dphGetHashEntry(pMac,
                             ((tpLimMlmReassocInd) pMsgBuf)->aid);

    if (pStaDs)
    {
        limActivateCnfTimer(pMac,
                            (tANI_U16) ((tpLimMlmReassocInd) pMsgBuf)->aid);
    }
    else
    {
        limLog(pMac, LOGW,
           FL("No STA context when try to create CNF_WAIT_TIMER to detect loss of REASSOC CNF\n"));
    }
} /*** end limProcessMlmReassocInd() ***/
#endif



/**
 * limProcessMlmAuthInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_AUTH_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmAuthInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ           msgQ;
    tSirSmeAuthInd     *pSirSmeAuthInd;

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeAuthInd, sizeof(tSirSmeAuthInd)))
    {
        // Log error
        limLog(pMac, LOGP,
           FL("call to palAllocateMemory failed for eWNI_SME_AUTH_IND\n"));
    }

    limCopyU16((tANI_U8 *) &pSirSmeAuthInd->messageType, eWNI_SME_AUTH_IND);

    limAuthIndSerDes(pMac, (tpLimMlmAuthInd) pMsgBuf,
                        (tANI_U8 *) &(pSirSmeAuthInd->length));

    msgQ.type = eWNI_SME_AUTH_IND;
    msgQ.bodyptr = pSirSmeAuthInd;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_AUTH_IND_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    limHalMmhPostMsgApi(pMac, &msgQ,  ePROT);
} /*** end limProcessMlmAuthInd() ***/



#ifdef ANI_PRODUCT_TYPE_AP
/**
 * limProcessMlmAssocInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_ASSOC_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmAssocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32                len;
    tSirMsgQ           msgQ;
    tSirSmeAssocInd    *pSirSmeAssocInd;
    tpDphHashNode      pStaDs=0;

    /// Inform Host of STA association
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    len = sizeof(tSirSmeAssocInd) -
          sizeof(tSirNeighborBssInfo) +
          (((tpLimMlmAssocInd) pMsgBuf)->numBss *
           sizeof(tSirNeighborBssInfo)) -
          SIR_MAC_MAX_SSID_LENGTH +
          ((tpLimMlmAssocInd) pMsgBuf)->ssId.length;
#else
    len = sizeof(tSirSmeAssocInd);
#endif
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeAssocInd, len))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for eWNI_SME_ASSOC_IND\n"));
    }
    sirStoreU16N((tANI_U8 *) &pSirSmeAssocInd->messageType,
                 eWNI_SME_ASSOC_IND);
    limAssocIndSerDes(pMac, (tpLimMlmAssocInd) pMsgBuf,
                      (tANI_U8 *) &(pSirSmeAssocInd->length));

    msgQ.type = eWNI_SME_ASSOC_IND;
    msgQ.bodyptr = pSirSmeAssocInd;
    msgQ.bodyval = 0;

    pStaDs = dphGetHashEntry(pMac,
                             ((tpLimMlmAssocInd) pMsgBuf)->aid);

    if (! pStaDs)
    {   // good time to panic...
        limLog(pMac, LOGP, FL("MLM AssocInd: Station context no longer valid (aid %d)\n"),
               ((tpLimMlmAssocInd) pMsgBuf)->aid);
    }

    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ASSOC_IND_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    if (limHalMmhPostMsgApi(pMac, &msgQ,  ePROT) != eSIR_SUCCESS)
    {
        limRejectAssociation(pMac,
                              pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType,
                             true,
                             pStaDs->mlmStaContext.authType,
                             ((tpLimMlmAssocInd) pMsgBuf)->aid,
                             true,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);
        return;
    }

    PELOG1(limLog(pMac, LOG1,
       FL("Create CNF_WAIT_TIMER after received LIM_MLM_ASSOC_IND\n"));)
    /*
     ** turn on a timer to detect the loss of ASSOC CNF
     **/
    limActivateCnfTimer(pMac, (tANI_U16) ((tpLimMlmAssocInd) pMsgBuf)->aid);
} /*** end limProcessMlmAssocInd() ***/
#endif



/**
 * limProcessMlmDisassocInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_DISASSOC_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmDisassocInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{

    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:

            break;

        case eLIM_STA_ROLE:
            pMac->lim.gLimSmeState = eLIM_SME_WT_DISASSOC_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
            break;

        default: // eLIM_AP_ROLE
            {
                PELOG1(
                tLimMlmDisassocInd  *pMlmDisassocInd;
                pMlmDisassocInd = (tLimMlmDisassocInd *) pMsgBuf;

                limLog(pMac, LOG1,
                       FL("*** Peer staId=%d Disassociated ***\n"),
                        pMlmDisassocInd->aid);)
            }

            // Send SME_DISASOC_IND after Polaris cleanup
            // (after receiving LIM_MLM_PURGE_STA_IND)

            break;
    } // end switch (pMac->lim.gLimSystemRole)
} /*** end limProcessMlmDisassocInd() ***/



/**
 * limProcessMlmDisassocCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_DISASSOC_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmDisassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes     resultCode;
    tLimMlmDisassocCnf  *pMlmDisassocCnf;
    pMlmDisassocCnf = (tLimMlmDisassocCnf *) pMsgBuf;
    
    resultCode = (tSirResultCodes)
                 (pMlmDisassocCnf->disassocTrigger ==
                  eLIM_LINK_MONITORING_DISASSOC) ?
                 eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE :
                 pMlmDisassocCnf->resultCode;

    if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
    {
#if defined(ANI_AP_CLIENT_SDK)
        tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
#endif
        // Disassociate Confirm from MLM
        if ( (pMac->lim.gLimSmeState != eLIM_SME_WT_DISASSOC_STATE) &&
             (pMac->lim.gLimSmeState != eLIM_SME_WT_DEAUTH_STATE) )
        {
            /**
             * Should not have received
             * Disassocate confirm
             * from MLM in other states.
             * Log error
             */
            limLog(pMac, LOGE,
               FL("received unexpected MLM_DISASSOC_CNF in state %X\n"),
               pMac->lim.gLimSmeState);

            return;
        }
#if defined(ANI_AP_CLIENT_SDK)
        // Whenever there is a disassoc notification, make sure the bssId is cleared so that
        // if the station finds the same AP to which it was associated, it can try to associate 
        // with it again. If this is not done, the beacons/probe rsp from this AP will not be given
        // up to WSM and it never see this is AP unless a cga/reboot is done.
        palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));

        if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
        }
#endif 
        if (pMac->lim.gLimRspReqd)
            pMac->lim.gLimRspReqd = false;

        if (pMlmDisassocCnf->disassocTrigger ==
                                    eLIM_PROMISCUOUS_MODE_DISASSOC)
        {
            if (pMlmDisassocCnf->resultCode != eSIR_SME_SUCCESS)
                pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
            else
                pMac->lim.gLimSmeState = eLIM_SME_OFFLINE_STATE;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            // Send Promiscuous mode response to host
            limSendSmePromiscuousModeRsp(pMac);
        }
        else
        {
            if (pMlmDisassocCnf->resultCode != eSIR_SME_SUCCESS)
                pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
            else
                pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            limSendSmeDisassocNtf(pMac, pMlmDisassocCnf->peerMacAddr,
                                  resultCode,
                                  pMlmDisassocCnf->disassocTrigger,
                                  pMlmDisassocCnf->aid);
        }
    }
    else if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        limSendSmeDisassocNtf(pMac, pMlmDisassocCnf->peerMacAddr,
                              resultCode,
                              pMlmDisassocCnf->disassocTrigger,
                              pMlmDisassocCnf->aid);
    }
} /*** end limProcessMlmDisassocCnf() ***/



/**
 * limProcessMlmDeauthInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_DEAUTH_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmDeauthInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{

    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:
            break;

        case eLIM_STA_ROLE:
            pMac->lim.gLimSmeState = eLIM_SME_WT_DEAUTH_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        default: // eLIM_AP_ROLE
            {
                PELOG1(
                tLimMlmDeauthInd  *pMlmDeauthInd;
                pMlmDeauthInd = (tLimMlmDeauthInd *) pMsgBuf;

                limLog(pMac, LOG1,
                   FL("*** Received Deauthentication from staId=%d ***\n"),
                   pMlmDeauthInd->aid);)
            }
            // Send SME_DEAUTH_IND after Polaris cleanup
            // (after receiving LIM_MLM_PURGE_STA_IND)

            break;
    } // end switch (pMac->lim.gLimSystemRole)
} /*** end limProcessMlmDeauthInd() ***/



/**
 * limProcessMlmDeauthCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_DEAUTH_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmDeauthCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16               aid;
    tSirResultCodes   resultCode;
    tLimMlmDeauthCnf  *pMlmDeauthCnf;
    
    pMlmDeauthCnf = (tLimMlmDeauthCnf *) pMsgBuf;

    resultCode = (tSirResultCodes)
                 (pMlmDeauthCnf->deauthTrigger ==
                  eLIM_LINK_MONITORING_DEAUTH) ?
                 eSIR_SME_LOST_LINK_WITH_PEER_RESULT_CODE :
                 pMlmDeauthCnf->resultCode;

    aid = (pMac->lim.gLimSystemRole == eLIM_AP_ROLE) ?
          pMlmDeauthCnf->aid : 1;

    if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
    {
#if defined(ANI_AP_CLIENT_SDK)
        tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
#endif
        // Deauth Confirm from MLM
        if (pMac->lim.gLimSmeState != eLIM_SME_WT_DEAUTH_STATE)
        {
            /**
             * Should not have received Deauth confirm
             * from MLM in other states.
             * Log error
             */
            limLog(pMac, LOGE,
               FL("received unexpected MLM_DEAUTH_CNF in state %X\n"),
               pMac->lim.gLimSmeState);

            return;
        }
#if defined(ANI_AP_CLIENT_SDK)
        // Whenever there is a disassoc notification, make sure the bssId is cleared so that
        // if the station finds the same AP to which it was associated, it can try to associate 
        // with it again. If this is not done, the beacons/probe rsp from this AP will not be given
        // up to WSM and it never see this is AP unless a cga/reboot is done.
        palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));

        if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
        }
#endif
        if (pMlmDeauthCnf->resultCode == eSIR_SME_SUCCESS)
        {
            pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;

            PELOG1(limLog(pMac, LOG1,
                   FL("*** Deauthenticated with BSS ***\n"));)
        }
        else
            pMac->lim.gLimSmeState = pMac->lim.gLimPrevSmeState;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        if (pMac->lim.gLimRspReqd)
            pMac->lim.gLimRspReqd = false;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    // BP deauthenticated by AP or vice versa
    // Send SME_DISASSOC_RSP to host.
    limSendSmeDisassocNtf(pMac, pMlmDeauthCnf->peerMacAddr,
                          resultCode,
                          pMlmDeauthCnf->deauthTrigger,
                          aid);
#else
    // On STA or on BASIC AP, send SME_DEAUTH_RSP to host
    limSendSmeDeauthNtf(pMac, pMlmDeauthCnf->peerMacAddr,
                        resultCode,
                        pMlmDeauthCnf->deauthTrigger,
                        aid);
#endif
} /*** end limProcessMlmDeauthCnf() ***/



/**
 * limProcessMlmPurgeStaInd()
 *
 *FUNCTION:
 * This function is called to processes MLM_PURGE_STA_IND
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmPurgeStaInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes      resultCode;
    tpLimMlmPurgeStaInd  pMlmPurgeStaInd;
#if defined(ANI_AP_CLIENT_SDK)
    tSirMacAddr         nullMacAddr = {0, 0, 0, 0, 0, 0};
#endif
    
    pMlmPurgeStaInd = (tpLimMlmPurgeStaInd) pMsgBuf;

    // Purge STA indication from MLM
    resultCode = (tSirResultCodes) pMlmPurgeStaInd->reasonCode;

    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_IN_IBSS_ROLE:
            break;

        case eLIM_STA_ROLE:
        default: // eLIM_AP_ROLE
            if ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
                (pMac->lim.gLimSmeState !=
                                       eLIM_SME_WT_DISASSOC_STATE) &&
                (pMac->lim.gLimSmeState != eLIM_SME_WT_DEAUTH_STATE))
            {
                /**
                 * Should not have received
                 * Purge STA indication
                 * from MLM in other states.
                 * Log error
                 */
                limLog(pMac, LOGE,
                   FL("received unexpected MLM_PURGE_STA_IND in state %X\n"),
                   pMac->lim.gLimSmeState);

                break;
            }

            PELOG1(limLog(pMac, LOG1,
               FL("*** Polaris cleanup completed for staId=%d ***\n"),
               pMlmPurgeStaInd->aid);)

            if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
            {
                pMac->lim.gLimSmeState = eLIM_SME_IDLE_STATE;
	         MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

#if defined(ANI_AP_CLIENT_SDK)
                // Whenever there is a disassoc notification, make sure the bssId is cleared so that
                // if the station finds the same AP to which it was associated, it can try to associate 
                // with it again. If this is not done, the beacons/probe rsp from this AP will not be given
                // up to WSM and it never see this is AP unless a cga/reboot is done.
                palCopyMemory(pMac->hHdd, pMac->lim.gLimBssid, nullMacAddr, sizeof(tSirMacAddr));

                if (cfgSetStr(pMac, WNI_CFG_BSSID, nullMacAddr, sizeof(tSirMacAddr)) != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP, FL("Could not update BSSID on CFG"));
                }
#endif
            }
 
            if (pMlmPurgeStaInd->reasonCode == eLIM_PEER_ENTITY_DEAUTH)
            {
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
                // BP deauthenticated by AP or vice versa
                // Send SME_DISASSOC_IND to host.
                limSendSmeDisassocNtf(pMac,
                                    pMlmPurgeStaInd->peerMacAddr,
                                    resultCode,
                                    pMlmPurgeStaInd->purgeTrigger,
                                    pMlmPurgeStaInd->aid);
#else

                limSendSmeDeauthNtf(pMac,
                            pMlmPurgeStaInd->peerMacAddr,
                            resultCode,
                            pMlmPurgeStaInd->purgeTrigger,
                            pMlmPurgeStaInd->aid);
#endif
            }
            else
                limSendSmeDisassocNtf(pMac,
                                    pMlmPurgeStaInd->peerMacAddr,
                                    resultCode,
                                    pMlmPurgeStaInd->purgeTrigger,
                                    pMlmPurgeStaInd->aid);
    } // end switch (pMac->lim.gLimSystemRole)
} /*** end limProcessMlmPurgeStaInd() ***/



/**
 * limProcessMlmSetKeysCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_SETKEYS_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmSetKeysCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    // Prepare and send SME_SETCONTEXT_RSP message
    tLimMlmSetKeysCnf  *pMlmSetKeysCnf;

    pMlmSetKeysCnf = (tLimMlmSetKeysCnf *) pMsgBuf;

    limLog( pMac, LOG1,
        FL("Received MLM_SETKEYS_CNF with resultCode = %d\n"),
        pMlmSetKeysCnf->resultCode );

    limSendSmeSetContextRsp(pMac,
                            pMlmSetKeysCnf->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                            pMlmSetKeysCnf->aid,
#else
                            1,
#endif
                            (tSirResultCodes) pMlmSetKeysCnf->resultCode);
} /*** end limProcessMlmSetKeysCnf() ***/

/**
 * limProcessMlmRemoveKeyCnf()
 *
 *FUNCTION:
 * This function is called to processes MLM_REMOVEKEY_CNF
 * message from MLM State machine.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param pMac       Pointer to Global MAC structure
 * @param pMsgBuf    A pointer to the MLM message buffer
 *
 * @return None
 */

void
limProcessMlmRemoveKeyCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    // Prepare and send SME_REMOVECONTEXT_RSP message
    tLimMlmRemoveKeyCnf  *pMlmRemoveKeyCnf;

    pMlmRemoveKeyCnf = (tLimMlmRemoveKeyCnf *) pMsgBuf;

    limLog( pMac, LOG1,
        FL("Received MLM_REMOVEKEYS_CNF with resultCode = %d\n"),
        pMlmRemoveKeyCnf->resultCode );

    limSendSmeRemoveKeyRsp(pMac,
                            pMlmRemoveKeyCnf->peerMacAddr,
                            (tSirResultCodes) pMlmRemoveKeyCnf->resultCode);
} /*** end limProcessMlmRemoveKeyCnf() ***/




/**
 * limHandleSmeJoinResult()
 *
 *FUNCTION:
 * This function is called to process join/auth/assoc failures
 * upon receiving MLM_JOIN/AUTH/ASSOC_CNF with a failure code or
 * MLM_ASSOC_CNF with a success code in case of STA role and
 * MLM_JOIN_CNF with success in case of STA in IBSS role.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  resultCode   Failure code to be sent
 *
 *
 * @return None
 */

static void
limHandleSmeJoinResult(tpAniSirGlobal pMac, tSirResultCodes resultCode, tANI_U16 protStatusCode)
{
    tpDphHashNode pStaDs = NULL;


    if (resultCode != eSIR_SME_SUCCESS)
    {
        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
        if (pStaDs != NULL)
        {
            pStaDs->mlmStaContext.disassocReason = eSIR_MAC_UNSPEC_FAILURE_REASON;
                                                   
            pStaDs->mlmStaContext.cleanupTrigger = eLIM_JOIN_FAILURE;
            pStaDs->mlmStaContext.resultCode = resultCode;
            pStaDs->mlmStaContext.protStatusCode = protStatusCode;

            limCleanupRxPath(pMac, pStaDs);
            return;

        }
    }

    palFreeMemory( pMac->hHdd, pMac->lim.gpLimJoinReq);
    pMac->lim.gpLimJoinReq = NULL;
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_JOIN_RSP, resultCode, protStatusCode);
} /*** end limHandleSmeJoinResult() ***/



/**
  * limProcessMlmAddStaRsp()
 *
 *FUNCTION:
 * This function is called to process a SIR_HAL_ADD_STA_RSP from HAL.
 * Upon receipt of this message from HAL, MLME -
 * > Determines the "state" in which this message was received
 * > Forwards it to the appropriate callback
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
void limProcessMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    //we need to process the deferred message since the initiating req. there might be nested request.
    //in the case of nested request the new request initiated from the response will take care of resetting
    //the deffered flag.
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

#ifdef ANI_PRODUCT_TYPE_AP
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        limProcessApMlmAddStaRsp(pMac, limMsgQ);
        return;
    }
#endif
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmAddStaRsp(pMac, limMsgQ);
#endif
}

void limProcessStaMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tLimMlmAssocCnf mlmAssocCnf;
    tANI_U32        mesgType = LIM_MLM_ASSOC_CNF;
    tpAddStaParams  pAddStaParams = (tpAddStaParams) limMsgQ->bodyptr;
    tpDphHashNode   pStaDs;

    if( eHAL_STATUS_SUCCESS == pAddStaParams->status )
    {
        if( eLIM_MLM_WT_ADD_STA_RSP_STATE != pMac->lim.gLimMlmState)
        {
            //TODO: any response to be sent out here ?
            limLog( pMac, LOGE,
                FL( "Received unexpected SIR_HAL_ADD_STA_RSP in state %X\n" ),
                pMac->lim.gLimMlmState);
            mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
            goto end;
        }

        if (pMac->lim.gLimSmeState == eLIM_SME_WT_REASSOC_STATE)
             mesgType = LIM_MLM_REASSOC_CNF;
        //
        // Update the DPH Hash Entry for this STA
        // with proper state info
        //
        pStaDs = dphGetHashEntry( pMac, DPH_STA_HASH_INDEX_PEER );
        if( NULL != pStaDs)
            pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
        else
            limLog( pMac, LOGW,
            FL( "Unable to get the DPH Hash Entry for AID - %d\n" ),
            DPH_STA_HASH_INDEX_PEER);

        pMac->lim.gLimMlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
        /*
        * Storing the self StaIndex(Generated by HAL) in global LIM context,
        * instead of storing it in DPH Hash entry for Self STA.
        * DPH entry for the self STA stores the sta index for the BSS entry
        * to which the STA is associated.
        */
        pMac->lim.gLimStaid      = pAddStaParams->staIdx;

        //if the AssocRsp frame is not acknowledged, then keep alive timer will take care of the state
        limReactivateTimer(pMac, eLIM_HEART_BEAT_TIMER);
	 MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_KEEPALIVE_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimKeepaliveTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("Cannot activate keepalive timer.\n"));

#ifdef WLAN_DEBUG    
        pMac->lim.gLimNumLinkEsts++;
#endif
        // Return Assoc confirm to SME with success
        // FIXME_GEN4 - Need the correct ASSOC RSP code to
        // be passed in here....
        //mlmAssocCnf.resultCode = (tSirResultCodes) assoc.statusCode;
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_SUCCESS;
    }
    else
    {
        limLog( pMac, LOGW, FL( "ADD_STA failed!\n" ));
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
    }

end:
    if( 0 != limMsgQ->bodyptr )
    {
      palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
    }
    limPostSmeMessage( pMac, mesgType, (tANI_U32 *) &mlmAssocCnf );
    return;
}

void limProcessMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
 //we need to process the deferred message since the initiating req. there might be nested request.
  //in the case of nested request the new request initiated from the response will take care of resetting
  //the deffered flag.
  SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

#ifdef ANI_PRODUCT_TYPE_AP
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        limProcessApMlmDelBssRsp(pMac, limMsgQ);
        return;
    }
#endif
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmDelBssRsp(pMac, limMsgQ);
#endif
}



void limProcessStaMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tpDeleteBssParams pDelBssParams = (tpDeleteBssParams) limMsgQ->bodyptr;
    tpDphHashNode pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
    tSirResultCodes statusCode = eSIR_SME_SUCCESS;
    tSirMacAddr currentBssId;
    tANI_U32 cfg = sizeof(tSirMacAddr);



    if( eHAL_STATUS_SUCCESS == pDelBssParams->status )
    {

        PELOGW(limLog( pMac, LOGW,
                      FL( "STA received the DEL_BSS_RSP for BSSID: %X.\n"),pDelBssParams->bssIdx);)
        if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                            eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            goto end;
        }

        if (limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, currentBssId) != eSIR_SUCCESS)
        {
            PELOGE(limLog( pMac, LOGE, FL( "Failure in setting link state to IDLE\n"));)
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }

        if(pStaDs == NULL) {
             PELOGE(limLog( pMac, LOGE, FL( "DPH Entry for STA 1 missing.\n"));)
             statusCode = eSIR_SME_REFUSED;
             goto end;
        }

         if( eLIM_MLM_WT_DEL_BSS_RSP_STATE != pStaDs->mlmStaContext.mlmState)
        {
            PELOGE(limLog( pMac, LOGE, FL( "Received unexpected SIR_HAL_DEL_BSS_RSP in state %X\n" ),
                           pStaDs->mlmStaContext.mlmState);)
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }

        PELOG1(limLog( pMac, LOG1, FL("STA AssocID %d MAC "), pStaDs->assocId );
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)
    }else {
        limLog( pMac, LOGP, FL( "DEL BSS failed!\n" ) );
        return;
    }

   end:
    
     if( 0 != limMsgQ->bodyptr ) {
        palFreeMemory( pMac->hHdd, (void *) pDelBssParams );
    }
     
    if(pStaDs == NULL)
          return;

    if ( (pMac->lim.gLimSystemRole  == eLIM_STA_ROLE) && 
                (pMac->lim.gLimSmeState != eLIM_SME_WT_DISASSOC_STATE  && 
                pMac->lim.gLimSmeState != eLIM_SME_WT_DEAUTH_STATE) &&
                pStaDs->mlmStaContext.cleanupTrigger != eLIM_JOIN_FAILURE) 
    {
        /** The Case where the DelBss is invoked from 
        *   context of other than normal DisAssoc / Deauth OR 
        *  as part of Join Failure.
        */
        limHandleDelBssInReAssocContext(pMac, pStaDs); 
        return;
    }

    limPrepareAndSendDelStaCnf(pMac, pStaDs, statusCode);
    return;

}




#ifdef ANI_PRODUCT_TYPE_AP
void limProcessApMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tSirResultCodes rc = eSIR_SME_SUCCESS;
    tpDeleteBssParams pDelBss = (tpDeleteBssParams) limMsgQ->bodyptr;

    if (pDelBss == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("BSS: DEL_BSS_RSP with no body!\n"));)
        rc = eSIR_SME_REFUSED;
        goto end;
    }

    if( eLIM_MLM_WT_DEL_BSS_RSP_STATE != pMac->lim.gLimMlmState)
    {
            limLog( pMac, LOGE,
                        FL( "Received unexpected SIR_HAL_DEL_BSS_RSP in state %X\n" ),pMac->lim.gLimMlmState);
            rc = eSIR_SME_REFUSED;
           goto end;
    }

    if (pDelBss->status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGP, FL("BSS: DEL_BSS_RSP error (%x) Bss %d "),
               pDelBss->status, pDelBss->bssIdx);
        rc = eSIR_SME_STOP_BSS_FAILURE;
        goto end;
    }

    rc = limSetLinkState(pMac, eSIR_LINK_IDLE_STATE);
    if( rc != eSIR_SUCCESS )
        goto end;

    /** Softmac may send all the buffered packets right after resuming the transmission hence
     * to occupy the medium during non channel occupancy period. So resume the transmission after
     * HAL gives back the response.
     */
    if (LIM_IS_RADAR_DETECTED(pMac))
    {
         limFrameTransmissionControl(pMac, eLIM_TX_BSS_BUT_BEACON, eLIM_RESUME_TX);
         LIM_SET_RADAR_DETECTED(pMac, eANI_BOOLEAN_FALSE);
    }

    dphHashTableClassInit(pMac);
    limDeletePreAuthList(pMac);
    //Is it ok to put LIM into IDLE state.
    pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

    end:
    limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, rc);
    if(pDelBss != NULL)
        palFreeMemory( pMac->hHdd, (void *) pDelBss );

}

#endif



void limProcessMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
 //we need to process the deferred message since the initiating req. there might be nested request.
  //in the case of nested request the new request initiated from the response will take care of resetting
  //the deffered flag.
  SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

#ifdef ANI_PRODUCT_TYPE_AP
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        limProcessApMlmDelStaRsp(pMac, limMsgQ);
        return;
    }
#endif
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    limProcessStaMlmDelStaRsp(pMac, limMsgQ);
#endif
}


#ifdef ANI_PRODUCT_TYPE_AP
void limProcessApMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tpDeleteStaParams pDelStaParams = (tpDeleteStaParams) limMsgQ->bodyptr;
    tpDphHashNode pStaDs = dphGetHashEntry(pMac, pDelStaParams->assocId);
    tSirResultCodes statusCode = eSIR_SME_SUCCESS;

    if( eHAL_STATUS_SUCCESS == pDelStaParams->status )
    {
           limLog( pMac, LOGW,
                      FL( "AP received the DEL_STA_RSP for assocID: %X.\n"), pDelStaParams->assocId);

        if(pStaDs == NULL)
        {
             limLog( pMac, LOGE,
                  FL( "DPH Entry for STA %X missing.\n"), pDelStaParams->assocId);
             statusCode = eSIR_SME_REFUSED;
             goto end;
        }

         if(( eLIM_MLM_WT_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState) &&
            ( eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState))
        {
            limLog( pMac, LOGE,
              FL( "Received unexpected SIR_HAL_DEL_STA_RSP in state %s for staId %d assocId %d \n" ),
               limMlmStateStr(pStaDs->mlmStaContext.mlmState), pStaDs->staIndex, pStaDs->assocId);
            statusCode = eSIR_SME_REFUSED;
           goto end;
        }


        PELOG1(limLog( pMac, LOG1,
            FL("Deleted STA AssocID %d staId %d MAC "),
            pStaDs->assocId, pStaDs->staIndex);
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)

       if(eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE == pStaDs->mlmStaContext.mlmState)
       {
            if( 0 != limMsgQ->bodyptr )
            {
              palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
            }

            if (limAddSta(pMac, pStaDs) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGE,
                       FL("could not Add STA with assocId=%d\n"),
                       pStaDs->assocId);

              // delete the TS if it has already been added.
               // send the response with error status.
                if(pStaDs->qos.addtsPresent)
                {
                  tpLimTspecInfo pTspecInfo;
                  if(eSIR_SUCCESS == limTspecFindByAssocId(pMac, pStaDs->assocId,
                            &pStaDs->qos.addts.tspec, &pMac->lim.tspecInfo[0], &pTspecInfo))
                  {
                    limAdmitControlDeleteTS(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec.tsinfo,
                                                            NULL, &pTspecInfo->idx);
                  }
                }

                limRejectAssociation(pMac,
                         pStaDs->staAddr,
                         pStaDs->mlmStaContext.subType,
                         true, pStaDs->mlmStaContext.authType,
                         pStaDs->assocId, true,
                         (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);

            }
            return;
        }
   }
    else
    {
        limLog( pMac, LOGW,
             FL( "DEL STA failed!\n" ));
        statusCode = eSIR_SME_REFUSED;
    }

    end:
    if( 0 != limMsgQ->bodyptr )
    {
      palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
    }

    if(eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState)
    {
       limPrepareAndSendDelStaCnf(pMac, pStaDs, statusCode);
    }

      return;

}

#endif



void limProcessStaMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tSirResultCodes   statusCode    = eSIR_SME_SUCCESS;
    tpDeleteStaParams pDelStaParams = (tpDeleteStaParams) limMsgQ->bodyptr;
    tpDphHashNode     pStaDs        = NULL;

    if( eHAL_STATUS_SUCCESS == pDelStaParams->status )
    {
        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
        if (pStaDs == NULL)
        {
            //TODO: any response to be sent out here ?
            limLog( pMac, LOGE, FL( "DPH Entry for STA %X missing.\n"),
                    pDelStaParams->assocId);
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }

        if( eLIM_MLM_WT_DEL_STA_RSP_STATE != pMac->lim.gLimMlmState)
        {
            //TODO: any response to be sent out here ?
            limLog( pMac, LOGE, FL( "Received unexpected SIR_HAL_DELETE_STA_RSP in state %s\n" ),
                  limMlmStateStr(pMac->lim.gLimMlmState));
            statusCode = eSIR_SME_REFUSED;
            goto end;
        }
        PELOG1(limLog( pMac, LOG1, FL("STA AssocID %d MAC "), pStaDs->assocId );
        limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)
        limLog( pMac, LOGW, FL( "DEL_STA_RSP received for assocID: %X\n"), pDelStaParams->assocId);

        //we must complete all cleanup related to delSta before calling limDelBSS.
        if( 0 != limMsgQ->bodyptr )
        {
            palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
        }
        statusCode = (tSirResultCodes) limDelBss(pMac, pStaDs, 0);
        return;
    }
    else
    {
        limLog( pMac, LOGW, FL( "DEL_STA failed!\n" ));
        statusCode = eSIR_SME_REFUSED;
    }
end:
    if( 0 != limMsgQ->bodyptr )
    {
        palFreeMemory( pMac->hHdd, (void *) pDelStaParams );
    }
    return;
}


#ifdef ANI_PRODUCT_TYPE_AP
void limProcessApMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tpAddStaParams pAddStaParams = (tpAddStaParams) limMsgQ->bodyptr;

    tpDphHashNode pStaDs = dphGetHashEntry(pMac, pAddStaParams->assocId);
    if(pStaDs == NULL)
    {
        //TODO: any response to be sent out here ?
        limLog( pMac, LOGE, FL( "DPH Entry for STA %X missing.\n"), pAddStaParams->assocId);
        goto end;
    }

    //
    // TODO & FIXME_GEN4
    // Need to inspect tSirMsgQ.reserved for a valid Dialog token!
    //
    //TODO: any check for pMac->lim.gLimMlmState ?

    if( eLIM_MLM_WT_ADD_STA_RSP_STATE != pStaDs->mlmStaContext.mlmState)
    {
        //TODO: any response to be sent out here ?
        limLog( pMac, LOGE,
                FL( "Received unexpected SIR_HAL_ADD_STA_RSP in state %X\n" ),
                pStaDs->mlmStaContext.mlmState);
        goto end;
    }

    if(eHAL_STATUS_SUCCESS != pAddStaParams->status)
    {
        PELOGE(limLog(pMac, LOGE, FL("Error! rcvd delSta rsp from HAL with status %d\n"),pAddStaParams->status);)
        limRejectAssociation(pMac, pStaDs->staAddr,
                 pStaDs->mlmStaContext.subType,
                 true, pStaDs->mlmStaContext.authType,
                 pStaDs->assocId, true,
                 (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);
        goto end;
    }

    pStaDs->bssId = pAddStaParams->bssIdx;
    pStaDs->staIndex = pAddStaParams->staIdx;

    if(pStaDs->qos.addtsPresent)
    {
        //need to send halMsg_AddTs to HAL.
        tpLimTspecInfo pTspecInfo;
        if(eSIR_SUCCESS ==
           limTspecFindByAssocId(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec,
                                 &pMac->lim.tspecInfo[0], &pTspecInfo))
        {
            if(eSIR_SUCCESS != limSendHalMsgAddTs(pMac, pStaDs->staIndex, pTspecInfo->idx, pStaDs->qos.addts.tspec))
            {
                // delete the TS that has already been added.
                // send the response with error status.
                limAdmitControlDeleteTS(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec.tsinfo, NULL, &pTspecInfo->idx);

                limLog(pMac, LOGE,
                 FL("limSendHalMsgAddTs failed for STA with assocId=%d\n"),
                 pStaDs->assocId);

                limRejectAssociation(pMac,
                   pStaDs->staAddr, pStaDs->mlmStaContext.subType,
                   true, pStaDs->mlmStaContext.authType,
                   pStaDs->assocId, false,
                   (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);

                if( 0 != limMsgQ->bodyptr )
                {
                    palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
                }
                limDelSta(pMac, pStaDs, true);
                return;
            }
        }
    }

    //if the AssocRsp frame is not acknowledged, then keep alive timer will take care of the state

    pStaDs->valid = 1;
    pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
    PELOG1(limLog( pMac, LOG1,
            FL("STA AssocID %d staId %d MAC "),
            pStaDs->assocId,
            pStaDs->staIndex);
    limPrintMacAddr(pMac, pStaDs->staAddr, LOG1);)

    // Send Re/Association Response with
    // status code to requesting STA.
    limSendAssocRspMgmtFrame(pMac, eSIR_SUCCESS, pStaDs->assocId, pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType, pStaDs);

end:
    if( 0 != limMsgQ->bodyptr )
    {
        palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
    }
    return;
}
#endif



/**
 * limProcessApMlmAddBssRsp()
 *
 *FUNCTION:
 * This function is called to process a SIR_HAL_ADD_BSS_RSP from HAL.
 * Upon receipt of this message from HAL, MLME -
 * > Validates the result of SIR_HAL_ADD_BSS_REQ
 * > Init other remaining LIM variables
 * > Init the AID pool, for that BSSID
 * > Init the Pre-AUTH list, for that BSSID
 * > Create LIM timers, specific to that BSSID
 * > Init DPH related parameters that are specific to that BSSID
 * > TODO - When do we do the actual change channel?
 *
 *LOGIC:
 * SME sends eWNI_SME_START_BSS_REQ to LIM
 * LIM sends LIM_MLM_START_REQ to MLME
 * MLME sends SIR_HAL_ADD_BSS_REQ to HAL
 * HAL responds with SIR_HAL_ADD_BSS_RSP to MLME
 * MLME responds with LIM_MLM_START_CNF to LIM
 * LIM responds with eWNI_SME_START_BSS_RSP to SME
 *
 *ASSUMPTIONS:
 * tSirMsgQ.body is allocated by MLME during limProcessMlmStartReq
 * tSirMsgQ.body will now be freed by this routine
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
static void
limProcessApMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tLimMlmStartCnf mlmStartCnf;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tANI_U32 val;
    tSirMacAddr currentBssId;
    tANI_U32 cfg = sizeof(tSirMacAddr);


    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        PELOG2(limLog(pMac, LOG2, FL("SIR_HAL_ADD_BSS_RSP returned with eHAL_STATUS_SUCCESS\n"));)
        if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                        eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            goto end;
        }

        if (limSetLinkState(pMac, eSIR_LINK_AP_STATE, currentBssId) != eSIR_SUCCESS )
            goto end;

        // Set MLME state
        pMac->lim.gLimMlmState = eLIM_MLM_BSS_STARTED_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        if( eSIR_IBSS_MODE == pAddBssParams->bssType )
        {
            /** IBSS is 'active' when we receive
             * Beacon frames from other STAs that are part of same IBSS.
             * Mark internal state as inactive until then.
             */
            pMac->lim.gLimIbssActive = false;

            limResetHBPktCount( pMac );
            limDeactivateAndChangeTimer( pMac, eLIM_HEART_BEAT_TIMER );
	     MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_HEART_BEAT_TIMER));
            if (tx_timer_activate(&pMac->lim.limTimers.gLimHeartBeatTimer) != TX_SUCCESS)
                limLog(pMac, LOGP, FL("could not activate Heartbeat timer\n"));

        }
        pMac->lim.gLimBssIdx     = (tANI_U8) pAddBssParams->bssIdx;

#ifdef ANI_PRODUCT_TYPE_AP
        pMac->lim.gLimSystemRole = eLIM_AP_ROLE;
#else
        pMac->lim.gLimSystemRole = eLIM_STA_IN_IBSS_ROLE;
#endif
        schEdcaProfileUpdate(pMac);
        limInitPreAuthList(pMac);
        limInitAIDpool(pMac);

        // Create timers used by LIM
        if (!pMac->lim.gLimTimersCreated)
            limCreateTimers(pMac);

        /* Update the lim global gLimTriggerBackgroundScanDuringQuietBss */
        if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val ))
            limLog( pMac, LOGP, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN!\n"));
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;

        // Apply previously set configuration at HW
        limApplyConfiguration(pMac);

        pMac->lim.gLimStaid     = pAddBssParams->staContext.staIdx;
        mlmStartCnf.resultCode  = eSIR_SME_SUCCESS;
    }
    else
    {
        limLog( pMac, LOGE, FL( "SIR_HAL_ADD_BSS_REQ failed with status %d\n" ),
            pAddBssParams->status );
        mlmStartCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
    }
    limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );

    end:

    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd, (void *) pAddBssParams );

}




/**
 * limProcessIbssMlmAddBssRsp()
 *
 *FUNCTION:
 * This function is called to process a SIR_HAL_ADD_BSS_RSP from HAL.
 * Upon receipt of this message from HAL, MLME -
 * > Validates the result of SIR_HAL_ADD_BSS_REQ
 * > Init other remaining LIM variables
 * > Init the AID pool, for that BSSID
 * > Init the Pre-AUTH list, for that BSSID
 * > Create LIM timers, specific to that BSSID
 * > Init DPH related parameters that are specific to that BSSID
 * > TODO - When do we do the actual change channel?
 *
 *LOGIC:
 * SME sends eWNI_SME_START_BSS_REQ to LIM
 * LIM sends LIM_MLM_START_REQ to MLME
 * MLME sends SIR_HAL_ADD_BSS_REQ to HAL
 * HAL responds with SIR_HAL_ADD_BSS_RSP to MLME
 * MLME responds with LIM_MLM_START_CNF to LIM
 * LIM responds with eWNI_SME_START_BSS_RSP to SME
 *
 *ASSUMPTIONS:
 * tSirMsgQ.body is allocated by MLME during limProcessMlmStartReq
 * tSirMsgQ.body will now be freed by this routine
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
static void
limProcessIbssMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tLimMlmStartCnf mlmStartCnf;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tANI_U32 val;
    tSirMacAddr currentBssId;
    tANI_U32 cfg = sizeof(tSirMacAddr);



    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        PELOG1(limLog(pMac, LOG1, FL("SIR_HAL_ADD_BSS_RSP returned with eHAL_STATUS_SUCCESS\n"));)
        if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                        eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            goto end;
        }

        if (limSetLinkState(pMac, eSIR_LINK_IBSS_STATE, currentBssId) != eSIR_SUCCESS )
            goto end;

        // Set MLME state
        pMac->lim.gLimMlmState = eLIM_MLM_BSS_STARTED_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        /** IBSS is 'active' when we receive
         * Beacon frames from other STAs that are part of same IBSS.
         * Mark internal state as inactive until then.
         */
        pMac->lim.gLimIbssActive = false;

        limResetHBPktCount( pMac );
        limDeactivateAndChangeTimer( pMac, eLIM_HEART_BEAT_TIMER );
	 MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_HEART_BEAT_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimHeartBeatTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("could not activate Heartbeat timer\n"));

        pMac->lim.gLimBssIdx     = (tANI_U8) pAddBssParams->bssIdx;

        pMac->lim.gLimSystemRole = eLIM_STA_IN_IBSS_ROLE;

        schEdcaProfileUpdate(pMac);
        limInitPreAuthList(pMac);
        limInitAIDpool(pMac);

        // Create timers used by LIM
        if (!pMac->lim.gLimTimersCreated)
            limCreateTimers(pMac);

        /* Update the lim global gLimTriggerBackgroundScanDuringQuietBss */
        if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val ))
            limLog( pMac, LOGP, FL("Failed to get WNI_CFG_TRIG_STA_BK_SCAN!\n"));
        pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;

        // Apply previously set configuration at HW
        limApplyConfiguration(pMac);

        pMac->lim.gLimStaid     = pAddBssParams->staContext.staIdx;
        mlmStartCnf.resultCode  = eSIR_SME_SUCCESS;
        //If ADD BSS was issued as part of IBSS coalescing, don't send the message to SME, as that is internal to LIM
        if(true == pMac->lim.gLimIbssCoalescingHappened)
        {
            limIbssAddBssRspWhenCoalescing(pMac, limMsgQ->bodyptr);
            goto end;
        }
    }
    else
    {
        limLog( pMac, LOGE, FL( "SIR_HAL_ADD_BSS_REQ failed with status %d\n" ),
            pAddBssParams->status );
        mlmStartCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
    }
    //Send this message to SME, when ADD_BSS is initiated by SME
    //If ADD_BSS is done as part of coalescing, this won't happen.
    limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );

    end:    
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd, (void *) pAddBssParams );

}


static void
limProcessStaMlmAddBssRspPreAssoc( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tANI_U32                val;
    tAniAuthType       cfgAuthType, authMode;
    tLimMlmAuthReq     *pMlmAuthReq;
    tpDphHashNode pStaDs = NULL;


    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {

            if ((pStaDs = dphAddHashEntry(pMac, pAddBssParams->staContext.staMac, DPH_STA_HASH_INDEX_PEER)) == NULL) 
            {
                // Could not add hash table entry
                PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for \n"));)
                limPrintMacAddr(pMac, pAddBssParams->staContext.staMac, LOGE);
                goto joinFailure;
            }

            pMac->lim.gLimBssIdx     = (tANI_U8) pAddBssParams->bssIdx;
     
            //Success, handle below
            pStaDs->bssId = pAddBssParams->bssIdx;
            //STA Index(genr by HAL) for the BSS entry is stored here
            pStaDs->staIndex = pAddBssParams->staContext.staIdx;
    

            // Trigger Authentication with AP
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATION_TYPE,
                          (tANI_U32 *) &cfgAuthType) != eSIR_SUCCESS)
            {
                /**
                 * Could not get AuthType from CFG.
                 * Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve AuthType\n"));
            }

            if (cfgAuthType == eSIR_AUTO_SWITCH)
                authMode = eSIR_SHARED_KEY; // Try Shared Key first
            else
                authMode = cfgAuthType;

            // Trigger MAC based Authentication
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAuthReq, sizeof(tLimMlmAuthReq)))
            {
                // Log error
                limLog(pMac, LOGP,
                       FL("call to palAllocateMemory failed for mlmAuthReq\n"));
                return;
            }

            val = sizeof(tSirMacAddr);
            if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID,
                          pMlmAuthReq->peerMacAddr,
                          &val) != eSIR_SUCCESS)
            {
                /// Could not get BSSID from CFG. Log error.
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }

            pMlmAuthReq->authType = authMode;

            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                          (tANI_U32 *) &pMlmAuthReq->authFailureTimeout)
                          != eSIR_SUCCESS)
            {
                /**
                 * Could not get AuthFailureTimeout
                 * value from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
            }

            pMac->lim.gLimMlmState = eLIM_MLM_JOINED_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, eLIM_MLM_JOINED_STATE));


            pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
            pMac->lim.gLimSmeState     = eLIM_SME_WT_AUTH_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            limPostMlmMessage(pMac,
                              LIM_MLM_AUTH_REQ,
                              (tANI_U32 *) pMlmAuthReq);
            return;
    }


joinFailure:
    {
        pMac->lim.gLimSmeState = eLIM_SME_JOIN_FAILURE_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        /// Send Join response to Host
        limHandleSmeJoinResult(pMac,  eSIR_SME_REFUSED, eSIR_MAC_UNSPEC_FAILURE_STATUS); 
                                                                          
    } 

}





/**
 * limProcessStaMlmAddBssRsp()
 *
 *FUNCTION:
 * This function is called to process a SIR_HAL_ADD_BSS_RSP from HAL.
 * Upon receipt of this message from HAL, MLME -
 * > Validates the result of SIR_HAL_ADD_BSS_REQ
 * > Now, send an ADD_STA to HAL and ADD the "local" STA itself
 *
 *LOGIC:
 * MLME had sent SIR_HAL_ADD_BSS_REQ to HAL
 * HAL responded with SIR_HAL_ADD_BSS_RSP to MLME
 * MLME now sends SIR_HAL_ADD_STA_REQ to HAL
 *
 *ASSUMPTIONS:
 * tSirMsgQ.body is allocated by MLME during limProcessMlmJoinReq
 * tSirMsgQ.body will now be freed by this routine
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
static void
limProcessStaMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
    tLimMlmAssocCnf mlmAssocCnf;
    tANI_U32 mesgType       = LIM_MLM_ASSOC_CNF;
    tANI_U32 subType        = LIM_ASSOC;
    tpDphHashNode pStaDs    = NULL;
    tANI_U16 staIdx = HAL_STA_INVALID_IDX;
    tANI_U8 updateSta = false;
    mlmAssocCnf.resultCode  = eSIR_SME_SUCCESS;


    if(eLIM_MLM_WT_ADD_BSS_RSP_PREASSOC_STATE == pMac->lim.gLimMlmState)
    {
        limProcessStaMlmAddBssRspPreAssoc(pMac, limMsgQ);
        goto end;
    }



    if( eLIM_MLM_WT_ADD_BSS_RSP_REASSOC_STATE == pMac->lim.gLimMlmState )
    {
        mesgType = LIM_MLM_REASSOC_CNF;
        subType = LIM_REASSOC;
	 //If Reassoc is happening for the same BSS, then use the existing StaId and indicate to HAL 
	 //to update the existing STA entry.
	 //If Reassoc is happening for the new BSS, then old BSS and STA entry would have been already deleted
	 //before PE tries to add BSS for the new BSS, so set the updateSta to false and pass INVALID STA Index.
	 if (sirCompareMacAddr( pMac->lim.gLimCurrentBssId, pMac->lim.gLimReassocBssId))
	{
		staIdx = pMac->lim.gLimStaid;
		updateSta = true;
	}
    }

    if( eHAL_STATUS_SUCCESS == pAddBssParams->status )
    {
        // Set MLME state
        pMac->lim.gLimMlmState = eLIM_MLM_WT_ADD_STA_RSP_STATE;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        // Now, send SIR_HAL_ADD_STA_REQ
        limLog( pMac, LOGW, FL( "On STA: ADD_BSS was successful\n" ));

        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
        if (pStaDs == NULL)
        {
            PELOGE(limLog(pMac, LOGE, FL("could not Add Self Entry for the station\n"));)
            mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
        }
        else
        {
            pMac->lim.gLimBssIdx     = (tANI_U8) pAddBssParams->bssIdx;
     
            //Success, handle below
            pStaDs->bssId = pAddBssParams->bssIdx;
            //STA Index(genr by HAL) for the BSS entry is stored here
            pStaDs->staIndex = pAddBssParams->staContext.staIdx;

            // Downgrade the EDCA parameters if needed
            limSetActiveEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParams); 

            // Send the active EDCA parameters to HAL
            if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE) {
                limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
            } else {
                limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
            }

            if (subType == LIM_REASSOC)
                limDeactivateAndChangeTimer(pMac, eLIM_KEEPALIVE_TIMER);
            if (limAddStaSelf(pMac, staIdx, updateSta) != eSIR_SUCCESS)
            {
                // Add STA context at HW
                PELOGE(limLog(pMac, LOGE, FL("could not Add Self Entry for the station\n"));)
                mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
            }
        }
    }
    else
    {
        limLog( pMac, LOGP, FL( "ADD_BSS failed!\n" ));

        // Return Assoc confirm to SME with failure
        mlmAssocCnf.resultCode = (tSirResultCodes) eSIR_SME_REFUSED;
    }


    if(mlmAssocCnf.resultCode != eSIR_SME_SUCCESS)
        limPostSmeMessage( pMac, mesgType, (tANI_U32 *) &mlmAssocCnf );

    end:
    if( 0 != limMsgQ->bodyptr )
        palFreeMemory( pMac->hHdd,(void *) pAddBssParams );
}







/**
 * limProcessMlmAddBssRsp()
 *
 *FUNCTION:
 * This function is called to process a SIR_HAL_ADD_BSS_RSP from HAL.
 * Upon receipt of this message from HAL, MLME -
 * > Determines the "state" in which this message was received
 * > Forwards it to the appropriate callback
 *
 *LOGIC:
 * SIR_HAL_ADD_BSS_RSP can be received by MLME while the LIM is
 * in the following two states:
 * 1) As AP, LIM state = eLIM_SME_WT_START_BSS_STATE
 * 2) As STA, LIM state = eLIM_SME_WT_JOIN_STATE
 * Based on these two states, this API will determine where to
 * route the message to
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
void limProcessMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
    tLimMlmStartCnf mlmStartCnf;
    tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;

    //
    // TODO & FIXME_GEN4
    // Need to inspect tSirMsgQ.reserved for a valid Dialog token!
    //

    //we need to process the deferred message since the initiating req. there might be nested request.
    //in the case of nested request the new request initiated from the response will take care of resetting
    //the deffered flag.
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    // Validate SME/LIM state
        // Validate MLME state

    if( eSIR_IBSS_MODE == pAddBssParams->bssType )
        limProcessIbssMlmAddBssRsp( pMac, limMsgQ );
    else
    {
        if( eLIM_SME_WT_START_BSS_STATE == pMac->lim.gLimSmeState )
        {
            if( eLIM_MLM_WT_ADD_BSS_RSP_STATE != pMac->lim.gLimMlmState )
            {
                // Mesg received from HAL in Invalid state!
                limLog( pMac, LOGE,
                  FL( "Received unexpected SIR_HAL_ADD_BSS_RSP in state %X\n" ),
                  pMac->lim.gLimMlmState );

                mlmStartCnf.resultCode = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;

                if( 0 != limMsgQ->bodyptr )
                    palFreeMemory( pMac->hHdd, (void *) pAddBssParams );
                limPostSmeMessage( pMac, LIM_MLM_START_CNF, (tANI_U32 *) &mlmStartCnf );
            }
            else
                limProcessApMlmAddBssRsp( pMac, limMsgQ );
        }
        else
            limProcessStaMlmAddBssRsp( pMac, limMsgQ );
    }
}

/**
 * limProcessMlmSetKeyRsp()
 *
 *FUNCTION:
 * This function is called to process the following two
 * messages from HAL:
 * 1) SIR_HAL_SET_BSSKEY_RSP
 * 2) SIR_HAL_SET_STAKEY_RSP
 * 3) SIR_HAL_SET_STA_BCASTKEY_RSP
 * Upon receipt of this message from HAL,
 * MLME -
 * > Determines the "state" in which this message was received
 * > Forwards it to the appropriate callback
 *
 *LOGIC:
 * SIR_HAL_SET_BSSKEY_RSP/SIR_HAL_SET_STAKEY_RSP can be
 * received by MLME while in the following state:
 * MLME state = eLIM_MLM_WT_SET_BSS_KEY_STATE --OR--
 * MLME state = eLIM_MLM_WT_SET_STA_KEY_STATE --OR--
 * MLME state = eLIM_MLM_WT_SET_STA_BCASTKEY_STATE
 * Based on this state, this API will determine where to
 * route the message to
 *
 *ASSUMPTIONS:
 * ONLY the MLME state is being taken into account for now.
 * This is because, it appears that the handling of the
 * SETKEYS REQ is handled symmetrically on both the AP & STA
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
void limProcessMlmSetKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
tANI_U8 respReqd = 1;
tLimMlmSetKeysCnf mlmSetKeysCnf;
tANI_U16 resultCode;

SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
  palZeroMemory( pMac->hHdd,
      (void *) &mlmSetKeysCnf,
      sizeof( tLimMlmSetKeysCnf ));

  if( eLIM_MLM_WT_SET_BSS_KEY_STATE == pMac->lim.gLimMlmState )
    resultCode = (tANI_U16) (((tpSetBssKeyParams) limMsgQ->bodyptr)->status);
  else if( eLIM_MLM_WT_SET_STA_KEY_STATE == pMac->lim.gLimMlmState)
    resultCode = (tANI_U16) (((tpSetStaKeyParams) limMsgQ->bodyptr)->status);
  else
    resultCode = (tANI_U16) (((tpSetStaKeyParams) limMsgQ->bodyptr)->status); //BCAST key also uses tpSetStaKeyParams. Done this way for readabilty.

  //
  // TODO & FIXME_GEN4
  // Need to inspect tSirMsgQ.reserved for a valid Dialog token!
  //

  // Validate SME/LIM state - Read the above "ASSUMPTIONS"
  //if( eLIM_SME_LINK_EST_STATE == pMac->lim.gLimSmeState )
  {
    // Validate MLME state
    if( eLIM_MLM_WT_SET_BSS_KEY_STATE != pMac->lim.gLimMlmState &&
         eLIM_MLM_WT_SET_STA_KEY_STATE != pMac->lim.gLimMlmState &&
         eLIM_MLM_WT_SET_STA_BCASTKEY_STATE != pMac->lim.gLimMlmState )
    {
      // Mesg received from HAL in Invalid state!
      limLog( pMac, LOGW,
          FL( "Received unexpected [Mesg Id - %d] in state %X\n" ),
          limMsgQ->type,
          pMac->lim.gLimMlmState );

      // There's not much that MLME can do at this stage...
      respReqd = 0;
    }
    else
      mlmSetKeysCnf.resultCode = resultCode;
  }

  if( 0 != limMsgQ->bodyptr )
    palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );

  // Restore MLME state
  pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;
  MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

  if( respReqd )
  {
  tpLimMlmSetKeysReq lpLimMlmSetKeysReq = (tpLimMlmSetKeysReq) pMac->lim.gpLimMlmSetKeysReq;

    // Prepare and Send LIM_MLM_SETKEYS_CNF
    if( NULL != lpLimMlmSetKeysReq )
    {
      palCopyMemory( pMac->hHdd,
          (tANI_U8 *) &mlmSetKeysCnf.peerMacAddr,
          (tANI_U8 *) lpLimMlmSetKeysReq->peerMacAddr,
          sizeof( tSirMacAddr ));

#ifdef ANI_PRODUCT_TYPE_AP
      mlmSetKeysCnf.aid = lpLimMlmSetKeysReq->aid;
#endif

      // Free the buffer cached for the global pMac->lim.gpLimMlmSetKeysReq
      palFreeMemory( pMac->hHdd, (tANI_U8 *) pMac->lim.gpLimMlmSetKeysReq);
      pMac->lim.gpLimMlmSetKeysReq = NULL;
    }

    limPostSmeMessage( pMac,
        LIM_MLM_SETKEYS_CNF,
        (tANI_U32 *) &mlmSetKeysCnf );
  }
}

/**
 * limProcessMlmRemoveKeyRsp()
 *
 *FUNCTION:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  tSirMsgQ  The MsgQ header, which contains the response buffer
 *
 * @return None
 */
void limProcessMlmRemoveKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ )
{
tLimMlmRemoveKeyCnf mlmRemoveCnf;
tANI_U16 resultCode;
tpLimMlmRemoveKeyReq lpLimMlmRemoveKeyReq = (tpLimMlmRemoveKeyReq) pMac->lim.gpLimMlmRemoveKeyReq;

  SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
  palZeroMemory( pMac->hHdd,
      (void *) &mlmRemoveCnf,
      sizeof( tLimMlmRemoveKeyCnf ));

  // Validate MLME state
    if( eLIM_MLM_WT_REMOVE_BSS_KEY_STATE != pMac->lim.gLimMlmState &&
        eLIM_MLM_WT_REMOVE_STA_KEY_STATE != pMac->lim.gLimMlmState )
    {
      // Mesg received from HAL in Invalid state!
      limLog( pMac, LOGW,
          FL( "Received unexpected [Mesg Id - %d] in state %X\n" ),
          limMsgQ->type,
          pMac->lim.gLimMlmState );
          return; //ignore the response.

    }


  if( eLIM_MLM_WT_REMOVE_BSS_KEY_STATE == pMac->lim.gLimMlmState )
    resultCode = (tANI_U16) (((tpRemoveBssKeyParams) limMsgQ->bodyptr)->status);
  else
    resultCode = (tANI_U16) (((tpRemoveStaKeyParams) limMsgQ->bodyptr)->status);

  //
  // TODO & FIXME_GEN4
  // Need to inspect tSirMsgQ.reserved for a valid Dialog token!
  //

 
  if( 0 != limMsgQ->bodyptr )
    palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );

  // Restore MLME state
  pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;
  MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

    // Prepare and Send LIM_MLM_REMOVEKEY_CNF
    if( NULL != lpLimMlmRemoveKeyReq )
    {
      palCopyMemory( pMac->hHdd,
          (tANI_U8 *) &mlmRemoveCnf.peerMacAddr,
          (tANI_U8 *) lpLimMlmRemoveKeyReq->peerMacAddr,
          sizeof( tSirMacAddr ));
      mlmRemoveCnf.resultCode = resultCode;

      // Free the buffer cached for the global pMac->lim.gpLimMlmRemoveKeyReq
      palFreeMemory( pMac->hHdd, (tANI_U8 *) pMac->lim.gpLimMlmRemoveKeyReq);
      pMac->lim.gpLimMlmRemoveKeyReq = NULL;
     }
     limPostSmeMessage( pMac,
        LIM_MLM_REMOVEKEY_CNF,
        (tANI_U32 *) &mlmRemoveCnf );
}


#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
/**----------------------------------------------------
\fn     __limProcessFinishLearnRsp
\brief  Handle finish learn rsp state, only for AP.

\param  pMac
\return NONE
-----------------------------------------------------*/
static void __limProcessFinishLearnRsp(tpAniSirGlobal pMac)
{
    PELOG2(limLog(pMac, LOG2, FL("System Role: %d\n"), pMac->lim.gLimSystemRole);)
    if (pMac->lim.gpLimMeasReq == NULL)
    {
        limRestorePreLearnState(pMac);
        return;
    }
    /**
      * Initial measurement -> periodic measurement should keep enabled, so that
      * if system Role is UNKNOWN LIM can send indication to WSM. Overloading
      * periodic measurement to distinguish initial measurement and radar
      * detect park basically to avoid sending measurement indication after
      * radar detect park.
      * Radar detect park -> periodic measurement should be disabled, so that
      * LIM wont send indication even when role is UNKNOWN.
      * Final measurement -> periodic measurement should be enabled, so that
      * LIM could start measurement indication timer.
      */
    if (pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled &&
                !pMac->lim.gLimMeasParams.isMeasIndTimerActive)
    {
        if (pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)
        {
            limSendSmeMeasurementInd(pMac);
            limCleanupMeasResources(pMac);
            limRestorePreLearnState(pMac);
            return;
        }
        else
        {
            // Activate periodic measurement indication timer
            if (tx_timer_activate(
               &pMac->lim.gLimMeasParams.measurementIndTimer)
               != TX_SUCCESS)
            {
                limLog(pMac, LOGP, FL("could not start Meas IND timer\n"));
                return;
            }
            pMac->lim.gLimMeasParams.isMeasIndTimerActive = 1;
        }
    }

    if (pMac->lim.gLimMeasParams.nextLearnChannelId >=
            pMac->lim.gpLimMeasReq->channelList.numChannels - 1)
    {
        // All channels in the channel set are learned.
        pMac->lim.gLimMeasParams.nextLearnChannelId = 0;
    }
    // Go back to previous state.
    limRestorePreLearnState(pMac);
    // Restart the learn interval timer.
    if (pMac->lim.gpLimMeasReq->measControl.periodicMeasEnabled)
        limReEnableLearnMode(pMac);
    return;
}
#endif


/** ---------------------------------------------------------------------
\fn      limProcessInitScanRsp
\brief   This function is called when LIM receives SIR_HAL_INIT_SCAN_RSP
\        message from HAL.  If status code is failure, then
\        update the gLimNumOfConsecutiveBkgndScanFailure count.
\param   tpAniSirGlobal  pMac
\param   tANI_U32        body
\return  none
\ ----------------------------------------------------------------------- */
void limProcessInitScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpInitScanParams pInitScanParam;
    eHalStatus status;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U8 channelNum;
#endif

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    
    pInitScanParam = (tpInitScanParams) body;
    status = pInitScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);

	if( pMac->lim.abortScan )
	{
		limLog( pMac, LOGW, FL(" finish scan\n") );
		pMac->lim.abortScan = 0;
        limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
        limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
        limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE);
	}

    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_INIT_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("InitScanRsp with failed status= %d\n"), status);)
               pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
               pMac->lim.gLimNumOfConsecutiveBkgndScanFailure += 1;
               /*
                * On Windows eSIR_SME_HAL_SCAN_INIT_FAILED message to CSR may trigger 
                * another Scan request in the same context (happens when 11d is enabled 
                * and first scan request with 11d channels fails for whatever reason, then CSR issues next init 
                * scan in the same context but with bigger channel list), so the state needs to be 
                * changed before this response message is sent.
                */
               limCompleteMlmScan(pMac, eSIR_SME_HAL_SCAN_INIT_FAILED);
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
                /* For handling the measurement request from WSM as scan request in LIM*/
#if 0
               if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE && pMac->lim.gpLimMeasReq != NULL)
                {
                    limRestorePreLearnState(pMac);
                    pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
                    limReEnableLearnMode(pMac);
                }
#endif
#endif                
                return;
            }
            else if (status == eHAL_STATUS_SUCCESS)
            {
                /* since we have successfully triggered a background scan, 
                 * reset the "consecutive bkgnd scan failure" count to 0
                 */ 
                pMac->lim.gLimNumOfConsecutiveBkgndScanFailure = 0;
                pMac->lim.gLimNumOfBackgroundScanSuccess += 1;
            }
            limContinueChannelScan(pMac);
            break;

#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_INIT_LEARN_WAIT_STATE:
//            if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            {
                if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
                {
                    limRestorePreLearnState(pMac);
                    pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
                    limReEnableLearnMode(pMac);
                    return;
                }
                channelNum = limGetCurrentLearnChannel(pMac);
                limSendHalStartScanReq(pMac, channelNum, eLIM_HAL_START_LEARN_WAIT_STATE);
            }
            break;

#endif
        default:
            limLog(pMac, LOGW, FL("limProcessInitScanRsp: Rcvd InitScanRsp not in WAIT State, state %d\n"),
                   pMac->lim.gLimHalScanState);
            break;
    }
    return;
}

/**
 * limProcessSwitchChannelReAssocReq()
 *
 *FUNCTION:
 * This function is called to send the reassoc req mgmt frame after the 
 * switchChannelRsp message is received from HAL. 
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac          - Pointer to Global MAC structure.
 * @param  psessionEntry - session related information.
 * @param  status        - channel switch success/failure.
 *
 * @return None
 */
static void limProcessSwitchChannelReAssocReq(tpAniSirGlobal pMac, eHalStatus status)
{
    tLimMlmReassocCnf        mlmReassocCnf;
    if(status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Change channel failed!!\n"));)
        mlmReassocCnf.resultCode = eSIR_SME_CHANNEL_SWITCH_FAIL;
        goto end;        
    }
    
    /// Start reassociation failure timer
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_REASSOC_FAIL_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimReassocFailureTimer)
                                           != TX_SUCCESS)
    {
        /// Could not start reassoc failure timer.
        // Log error
        limLog(pMac, LOGP,
           FL("could not start Reassociation failure timer\n"));

        // Return Reassoc confirm with
        // Resources Unavailable

        mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }
        
    /// Prepare and send Reassociation request frame
    limSendReassocReqMgmtFrame(pMac, (tpLimMlmReassocReq)(pMac->lim.gpLimMlmReassocReq));
    return;

end:
    /// Free up buffer allocated for reassocReq
    if (pMac->lim.gpLimMlmReassocReq)
    {
        palFreeMemory( pMac->hHdd, pMac->lim.gpLimMlmReassocReq);
        pMac->lim.gpLimMlmReassocReq = NULL;
    }
    mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;

    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);    
}

/**
 * limProcessSwitchChannelJoinReq()
 *
 *FUNCTION:
 * This function is called to send the probe req mgmt frame after the 
 * switchChannelRsp message is received from HAL. 
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac          - Pointer to Global MAC structure.
 * @param  psessionEntry - session related information.
 * @param  status        - channel switch success/failure.
 *
 * @return None
 */
static void limProcessSwitchChannelJoinReq(tpAniSirGlobal pMac, eHalStatus status)
{
    tANI_U32            val;
    tSirMacSSid         ssId;
    tLimMlmJoinCnf      mlmJoinCnf;
    tANI_U32            cfgLen;

    if(status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Change channel failed!!\n"));)
        mlmJoinCnf.resultCode = eSIR_SME_CHANNEL_SWITCH_FAIL;
        goto error;        
    }

    // Activate Join failure timer
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_JOIN_FAIL_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimJoinFailureTimer) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not activate Join failure timer\n"));

         MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
         mlmJoinCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        //memory is freed up below.
        goto error;
    }
        
    // Update BSSID at CFG database
    if (cfgSetStr(pMac, WNI_CFG_BSSID, pMac->lim.gpLimMlmJoinReq->bssDescription.bssId,
                  sizeof(tSirMacAddr))
        != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not update BSSID at CFG\n"));

    // By default STA is in Independent BSS mode
    if (SIR_MAC_GET_ESS( pMac->lim.gpLimMlmJoinReq->bssDescription.capabilityInfo))
        pMac->lim.gLimSystemRole = eLIM_STA_ROLE;
    else
        pMac->lim.gLimSystemRole = eLIM_STA_IN_IBSS_ROLE;

    /* Update the lim global gLimTriggerBackgroundScanDuringQuietBss */
    if(wlan_cfgGetInt(pMac, WNI_CFG_TRIG_STA_BK_SCAN, &val) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("failed to get WNI_CFG_TRIG_STA_BK_SCAN cfg value!\n"));
    pMac->lim.gLimTriggerBackgroundScanDuringQuietBss = (val) ? 1 : 0;

    // Apply previously set configuration at HW
    limApplyConfiguration(pMac);

    /// Wait for Beacon to announce join success
    cfgLen = SIR_MAC_MAX_SSID_LENGTH;
    if (wlan_cfgGetStr(pMac, WNI_CFG_SSID, ssId.ssId, &cfgLen) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not retrive SSID\n"));
    ssId.length = (tANI_U8) cfgLen;

    limSendProbeReqMgmtFrame( pMac, &ssId,
           pMac->lim.gpLimMlmJoinReq->bssDescription.bssId, pMac->lim.gpLimMlmJoinReq->bssDescription.channelId);
    return;                                        

error:
        /// Free up buffer allocated for pMac->lim.gLimMlmJoinReq
        palFreeMemory( pMac->hHdd, pMac->lim.gpLimMlmJoinReq);
        pMac->lim.gpLimMlmJoinReq = NULL;

        /// Return Join confirm with Resources Unavailable        
        mlmJoinCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
        limPostSmeMessage(pMac, LIM_MLM_JOIN_CNF, (tANI_U32 *) &mlmJoinCnf);
}

/**
 * limProcessSwitchChannelRsp()
 *
 *FUNCTION:
 * This function is called to process switchChannelRsp message from HAL. 
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  body - message body.
 *
 * @return None
 */
 
void limProcessSwitchChannelRsp(tpAniSirGlobal pMac,  void *body)
{
    tpSwitchChannelParams pChnlParams = NULL;
    eHalStatus status;
    tANI_U32 channelChangeReasonCode;
    
    //we need to process the deferred message since the initiating req. there might be nested request.
    //in the case of nested request the new request initiated from the response will take care of resetting
    //the deffered flag.
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    
    pChnlParams = (tpSwitchChannelParams) body;
    
    status = pChnlParams->status;

    palFreeMemory( pMac->hHdd, (tANI_U8 *)body);

    channelChangeReasonCode = pMac->lim.channelChangeReasonCode;    
    
    // initialize it back to invalid id
    pMac->lim.channelChangeReasonCode = 0xBAD;
        
    switch(channelChangeReasonCode)
    {
        case LIM_SWITCH_CHANNEL_REASSOC:
            limProcessSwitchChannelReAssocReq(pMac, status);
            break;
        
        case LIM_SWITCH_CHANNEL_JOIN:
            limProcessSwitchChannelJoinReq(pMac, status);
            break;
        
        default:
            break;
    }
}

/**
 * limProcessStartScanRsp()
 *
 *FUNCTION:
 * This function is called to process startScanRsp message from HAL. If scan/learn was successful
 *      then it will start scan/learn on the next channel.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  body - message body.
 *
 * @return None
 */


void limProcessStartScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpStartScanParams pStartScanParam;
    eHalStatus status;

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pStartScanParam = (tpStartScanParams) body;
    status = pStartScanParam->status;
    palFreeMemory( pMac->hHdd, (tANI_U8 *)body);

	if( pMac->lim.abortScan )
	{
		limLog( pMac, LOGW, FL(" finish scan\n") );
		pMac->lim.abortScan = 0;
        limDeactivateAndChangeTimer(pMac, eLIM_MIN_CHANNEL_TIMER);
        limDeactivateAndChangeTimer(pMac, eLIM_MAX_CHANNEL_TIMER);
        limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE);
	}

    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_START_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("StartScanRsp with failed status= %d\n"), status);)
               //
               // FIXME - With this, LIM will try and recover state, but
               // eWNI_SME_SCAN_CNF maybe reporting an incorrect
               // status back to the SME
               //
               limSendHalFinishScanReq( pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE );
               //limCompleteMlmScan(pMac, eSIR_SME_HAL_SCAN_INIT_FAILED);
            }
            else
            {
               pMac->lim.gLimHalScanState = eLIM_HAL_SCANNING_STATE;
               limContinuePostChannelScan(pMac);
            }
            break;

#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_START_LEARN_WAIT_STATE:
 //           if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            {
                if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
                {
                    limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
                }
                else
                {
                    limContinueChannelLearn(pMac);
                }
            }
            break;

#endif
        default:
            limLog(pMac, LOGW, FL("Rcvd StartScanRsp not in WAIT State, state %d\n"),
                     pMac->lim.gLimHalScanState);
            break;
    }

    return;
}

void limProcessEndScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpEndScanParams pEndScanParam;
    eHalStatus status;
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    tANI_U8 channelNum;
#endif

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pEndScanParam = (tpEndScanParams) body;
    status = pEndScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);

    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_END_SCAN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("EndScanRsp with failed status= %d\n"), status);)
               pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
               limCompleteMlmScan(pMac, eSIR_SME_HAL_SCAN_INIT_FAILED);
            }
            else
            {
               pMac->lim.gLimCurrentScanChannelId++;
               limContinueChannelScan(pMac);
            }
            break;

#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        case eLIM_HAL_END_LEARN_WAIT_STATE:
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
                limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
            }
            else if (pMac->lim.gLimSystemRole == eLIM_UNKNOWN_ROLE)
            {
               /** Before starting BSS, we do complete set of measurement before putting
                *   softmac back into normal mode.
                */
               channelNum = limGetCurrentLearnChannel(pMac);
               limSendHalStartScanReq(pMac, channelNum, eLIM_HAL_START_LEARN_WAIT_STATE);
            }
            else
            {
               limLog(pMac, LOGW, FL("ERROR! This state is set only when AP in UNKNOWN_ROLE\n"),
                        pMac->lim.gLimHalScanState);
                limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);
            }
            break;

#endif

        default:
            limLog(pMac, LOGW, FL("Rcvd endScanRsp not in WAIT State, state %d\n"),
                        pMac->lim.gLimHalScanState);
            break;
    }

    return;
}

void limProcessFinishScanRsp(tpAniSirGlobal pMac,  void *body)
{
    tpFinishScanParams pFinishScanParam;
    eHalStatus status;

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    pFinishScanParam = (tpFinishScanParams) body;
    status = pFinishScanParam->status;
    palFreeMemory( pMac->hHdd, (char *)body);

    switch(pMac->lim.gLimHalScanState)
    {
        case eLIM_HAL_FINISH_SCAN_WAIT_STATE:
            pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
            limCompleteMlmScan(pMac, eSIR_SME_SUCCESS);
            if (pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
            {
                /** Right time to stop tx and start the timer for channel switch */
                limStopTxAndSwitchChannel(pMac);
            }
            else if (pMac->lim.gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN)
            {
                /** Start the quieting */
                limStartQuietTimer(pMac);
            }
#ifdef ANI_PRODUCT_TYPE_AP 
            /* For handling the measurement request from WSM as scan request in LIM*/
#if 0            
            if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE && pMac->lim.gpLimMeasReq != NULL)
            {
                limSendSmeMeasurementInd(pMac);
                limCleanupMeasResources(pMac);
                limRestorePreLearnState(pMac);
            }
#endif
#endif            
            if (status != (tANI_U32) eHAL_STATUS_SUCCESS)
            {
               PELOGW(limLog(pMac, LOGW, FL("EndScanRsp with failed status= %d\n"), status);)
            }
            break;

#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))
        case eLIM_HAL_FINISH_LEARN_WAIT_STATE:
            pMac->lim.gLimHalScanState = eLIM_HAL_IDLE_SCAN_STATE;
            __limProcessFinishLearnRsp(pMac);
            break;

#endif //#if (defined(ANI_PRODUCT_TYPE_AP) || defined(ANI_PRODUCT_TYPE_AP_SDK))

        default:
            limLog(pMac, LOGW, FL("Rcvd endScanRsp not in WAIT State, state %d\n"),
                        pMac->lim.gLimHalScanState);
            break;
    }
    return;
}

/**
 * @function : limProcessMlmHalAddBARsp
 *  
 * @brief:     Process SIR_HAL_ADDBA_RSP coming from HAL
 *
 *
 * @param pMac The global tpAniSirGlobal object
 *
 * @param tSirMsgQ The MsgQ header containing the response buffer
 *
 * @return none
 */
void limProcessMlmHalAddBARsp( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ )
{
    // Send LIM_MLM_ADDBA_CNF to LIM
    tpLimMlmAddBACnf pMlmAddBACnf;
    tpAddBAParams pAddBAParams = (tpAddBAParams) limMsgQ->bodyptr;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_HAL_ADDBA_RSP_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    //now LIM can process any defer message.
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    //This is special case when HAL just wants LIM to start processing defer message.
    if(NULL == pAddBAParams)
        return;

    // Allocate for LIM_MLM_ADDBA_CNF
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
          (void **) &pMlmAddBACnf, sizeof( tLimMlmAddBACnf ))) {
        limLog( pMac, LOGP, FL(" palAllocateMemory failed with error code %d\n"));
        return;
    }
    
    palZeroMemory( pMac->hHdd, (void *) pMlmAddBACnf, sizeof( tLimMlmAddBACnf ));

     // Copy the peer MAC
     palCopyMemory( pMac->hHdd, pMlmAddBACnf->peerMacAddr, pAddBAParams->peerMacAddr,
                                                                                 sizeof( tSirMacAddr ));

     // Copy other ADDBA Rsp parameters
     pMlmAddBACnf->baDialogToken = pAddBAParams->baDialogToken;
     pMlmAddBACnf->baTID = pAddBAParams->baTID;
     pMlmAddBACnf->baPolicy = pAddBAParams->baPolicy;
     pMlmAddBACnf->baBufferSize = pAddBAParams->baBufferSize;
     pMlmAddBACnf->baTimeout = pAddBAParams->baTimeout;
     pMlmAddBACnf->baDirection = pAddBAParams->baDirection;

     if(eHAL_STATUS_SUCCESS == pAddBAParams->status)
        pMlmAddBACnf->addBAResultCode = eSIR_MAC_SUCCESS_STATUS;
     else
        pMlmAddBACnf->addBAResultCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;        

     palFreeMemory(pMac->hHdd, (void*)limMsgQ->bodyptr);
     
     // Send ADDBA CNF to LIM
     limPostSmeMessage( pMac, LIM_MLM_ADDBA_CNF, (tANI_U32 *) pMlmAddBACnf );

}

/**
 * \brief Process LIM_MLM_ADDBA_CNF
 *
 * \sa limProcessMlmAddBACnf
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tSirMsgQ The MsgQ header containing the response buffer
 *
 * \return none
 */
void limProcessMlmAddBACnf( tpAniSirGlobal pMac,
    tANI_U32 *pMsgBuf )
{
tpLimMlmAddBACnf pMlmAddBACnf;
tpDphHashNode pSta;
tANI_U16 aid;
tLimBAState curBaState;

  pMlmAddBACnf = (tpLimMlmAddBACnf) pMsgBuf;

  if(NULL == pMlmAddBACnf)
  {
    limLog( pMac, LOGE,
        FL( "pMlmAddBaCnf is NULL - ignoring ADDBA CNF from HAL\n" ));

    return;
  }
  // First, extract the DPH entry
  pSta = dphLookupHashEntry( pMac, pMlmAddBACnf->peerMacAddr, &aid );
  if( NULL == pSta )
  {
    limLog( pMac, LOGE,
        FL( "STA context not found - ignoring ADDBA CNF from HAL\n" ));

    return;
  }

  LIM_GET_STA_BA_STATE(pSta, pMlmAddBACnf->baTID, (tANI_U32*)&curBaState);

  // Need to validate SME state
  if( eLIM_BA_STATE_WT_ADD_RSP != curBaState)
  {
    limLog( pMac, LOGE,
        FL( "Received unexpected ADDBA CNF when STA BA state is %d\n" ),
        curBaState );
    return;
  }

  // Restore STA BA state
  LIM_SET_STA_BA_STATE(pSta, pMlmAddBACnf->baTID, eLIM_BA_STATE_IDLE);

  if( eSIR_SUCCESS == pMlmAddBACnf->addBAResultCode )
  {
    // Update LIM internal cache...
    if( eBA_RECIPIENT == pMlmAddBACnf->baDirection )
    {
      pSta->tcCfg[pMlmAddBACnf->baTID].fUseBARx = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fRxCompBA = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fRxBApolicy = pMlmAddBACnf->baPolicy;
      pSta->tcCfg[pMlmAddBACnf->baTID].rxBufSize = pMlmAddBACnf->baBufferSize;
      pSta->tcCfg[pMlmAddBACnf->baTID].tuRxBAWaitTimeout = pMlmAddBACnf->baTimeout;
    }
    else
    {
      pSta->tcCfg[pMlmAddBACnf->baTID].fUseBATx = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fTxCompBA = 1;
      pSta->tcCfg[pMlmAddBACnf->baTID].fTxBApolicy = pMlmAddBACnf->baPolicy;
      pSta->tcCfg[pMlmAddBACnf->baTID].txBufSize = pMlmAddBACnf->baBufferSize;
      pSta->tcCfg[pMlmAddBACnf->baTID].tuTxBAWaitTimeout = pMlmAddBACnf->baTimeout;
    }
  }

  if( eBA_RECIPIENT == pMlmAddBACnf->baDirection )
  {
    //
    // Package LIM_MLM_ADDBA_RSP to MLME, with proper
    // status code. MLME will then send an ADDBA RSP
    // over the air to the peer MAC entity
    //
    if( eSIR_SUCCESS != limPostMlmAddBARsp( pMac,
          pMlmAddBACnf->peerMacAddr,
          pMlmAddBACnf->addBAResultCode,
          pMlmAddBACnf->baDialogToken,
          (tANI_U8) pMlmAddBACnf->baTID,
          (tANI_U8) pMlmAddBACnf->baPolicy,
          pMlmAddBACnf->baBufferSize,
          pMlmAddBACnf->baTimeout))
    {
      limLog( pMac, LOGW,
          FL( "Failed to post LIM_MLM_ADDBA_RSP to " ));
      limPrintMacAddr( pMac, pMlmAddBACnf->peerMacAddr, LOGW );
    }
  }

  // Free the memory allocated for LIM_MLM_ADDBA_CNF
  palFreeMemory( pMac->hHdd, (void *) pMsgBuf );
}

/**
 * \brief Process LIM_MLM_DELBA_CNF
 *
 * \sa limProcessMlmDelBACnf
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tSirMsgQ The MsgQ header containing the response buffer
 *
 * \return none
 */
void limProcessMlmDelBACnf( tpAniSirGlobal pMac,
    tANI_U32 *pMsgBuf )
{
tpLimMlmDelBACnf pMlmDelBACnf;
tpDphHashNode pSta;
tANI_U16 aid;
tLimBAState curBaState;

  pMlmDelBACnf = (tpLimMlmDelBACnf) pMsgBuf;

  // First, extract the DPH entry
  pSta = dphLookupHashEntry( pMac, pMlmDelBACnf->peerMacAddr, &aid );
  if( NULL == pSta )
  {
    limLog( pMac, LOGE,
        FL( "STA context not found - ignoring DELBA CNF from HAL\n" ));

    return;
  }

  if(NULL == pMlmDelBACnf)
  {
    limLog( pMac, LOGE,
        FL( "pMlmDelBACnf is NULL - ignoring DELBA CNF from HAL\n" ));

    return;
  }

  // Need to validate baState
  LIM_GET_STA_BA_STATE(pSta, pMlmDelBACnf->baTID, (tANI_U32*)&curBaState);

  if( eLIM_BA_STATE_WT_DEL_RSP != curBaState )
  {
    limLog( pMac, LOGE,
        FL( "Received unexpected DELBA CNF when STA BA state is %d\n" ),
        curBaState );

    return;
  }

  // Restore STA BA state
  LIM_SET_STA_BA_STATE(pSta, pMlmDelBACnf->baTID, eLIM_BA_STATE_IDLE);

  // Free the memory allocated for LIM_MLM_DELBA_CNF
  palFreeMemory( pMac->hHdd, (void *) pMsgBuf );

}

/**
 * \brief Process SIR_LIM_DEL_BA_IND
 *
 * \sa limProcessMlmHalBADeleteInd
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param tSirMsgQ The MsgQ header containing the indication buffer
 *
 * \return none
 */
void limProcessMlmHalBADeleteInd( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ )
{
tSirRetStatus status = eSIR_SUCCESS;
tpBADeleteParams pBADeleteParams;
tpDphHashNode pSta;
tANI_U16 aid;
tLimBAState curBaState;

  pBADeleteParams = (tpBADeleteParams) limMsgQ->bodyptr;

  // First, extract the DPH entry
  pSta = dphLookupHashEntry( pMac, pBADeleteParams->peerMacAddr, &aid );
  if( NULL == pSta )
  {
    limLog( pMac, LOGE,
        FL( "STA context not found - ignoring BA Delete IND from HAL\n" ));

    goto returnAfterCleanup;
  }

  // Need to validate BA state

  LIM_GET_STA_BA_STATE(pSta, pBADeleteParams->baTID, (tANI_U32*)&curBaState);

  if( eLIM_BA_STATE_IDLE != curBaState )  
  {
    limLog( pMac, LOGE,
        FL( "Received unexpected BA Delete IND when STA BA state is %d\n" ),
        curBaState );

    goto returnAfterCleanup;
  }

  // Validate if a BA is active for the requested TID
  // AND in that desired direction
  if( eBA_INITIATOR == pBADeleteParams->baDirection )
  {
    if( 0 == pSta->tcCfg[pBADeleteParams->baTID].fUseBATx )
      status = eSIR_FAILURE;
  }
  else
  {
    if( 0 == pSta->tcCfg[pBADeleteParams->baTID].fUseBARx )
      status = eSIR_FAILURE;
  }

  if( eSIR_FAILURE == status )
  {
    limLog( pMac, LOGW,
        FL("Received an INVALID DELBA Delete Ind for TID %d...\n"),
        pBADeleteParams->baTID );
  }
  else
  {
    // Post DELBA REQ to MLME...
    if( eSIR_SUCCESS !=
        (status = limPostMlmDelBAReq( pMac,
                                      pSta,
                                      pBADeleteParams->baDirection,
                                      pBADeleteParams->baTID,
                                      eSIR_MAC_UNSPEC_FAILURE_REASON)))
    {
      limLog( pMac, LOGE,
          FL( "Attempt to post LIM_MLM_DELBA_REQ failed with status %d\n" ), status);
    }
    else
    {
      limLog( pMac, LOGE,
          FL( "BA Delete - Reason 0x%08x. Attempting to delete BA session for TID %d with peer STA "  ),
          pBADeleteParams->reasonCode, pBADeleteParams->baTID );
      limPrintMacAddr( pMac, pSta->staAddr, LOGW );
    }
  }

returnAfterCleanup:
  // Free the memory allocated for SIR_LIM_DEL_BA_IND
  palFreeMemory( pMac->hHdd, (void *) limMsgQ->bodyptr );
}

/**
 *  @function : limProcessSetMimoRsp()
 *
 *  @brief :  This function is called upon receiving the SIR_HAL_SET_MIMOPS_RSP from the HAL
 *                      after Processing the Req from the SME (PMC)
 *
 *      LOGIC:
 *
 *      ASSUMPTIONS:
 *          NA
 *
 *      NOTE:
 *          NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  limMsg - Lim Message structure object with the MimoPSparam in body
 * @return None
 */

void
limProcessSetMimoRsp(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
#if 0
    tSirRetStatus           retStatus;  
    tpSetMIMOPS          pMIMO_PSParams;  


    do {
        
        pMIMO_PSParams = (tpSetMIMOPS)limMsg->bodyptr;  
        if( NULL == pMIMO_PSParams ) {
            PELOGE(limLog(pMac, LOGE, "Received the SIR_HAL_SET_MIMOPS_RSP with NULL as the PS param");)
            return;
        }

        /** If Updation of the HAL Fail's*/
        if (pMIMO_PSParams->status != eSIR_SUCCESS) {  
            limLog(pMac, LOGP, FL("Update HAL / SW Mac for MIMO State has Failed\n")); 
            break;      
        } 
    
        if ((pMac->lim.gLimSystemRole != eSYSTEM_STA_ROLE) || 
                (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE) )
            break;

        pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;
        MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        /** In the Case of Exiting out of the Powersave (changing from Dynamic/Static mode to SM Enabled)
          *   send the action Frame to Peer to update the PS State of the STA , for the case of Entering PowerSave
          *   the Action Frame is being sent at first before setting the internal structures 
          */
        if (!isEnteringMimoPS(pMac->lim.gHTMIMOPSState, pMIMO_PSParams->htMIMOPSState)) {
            tSirMacAddr            macAddr;   
        
            /** Obtain the AP's Mac Address */  
            palCopyMemory(pMac -> hHdd, (tANI_U8 *)macAddr, pMac->lim.gLimBssid, sizeof(tSirMacAddr));  

            /** Send Action Frame with the corresponding mode */     
            retStatus = limSendSMPowerStateFrame(pMac, macAddr, pMIMO_PSParams->htMIMOPSState);           
            if (retStatus != eSIR_SUCCESS) {        
                PELOGE(limLog(pMac, LOGE, FL("Sending Action Frame has failed\n"));)           
                break;           
            }     
        }     
        PELOG1(limLog(pMac, LOG1, FL("The Setting up of LimGlobals is successful for MIMOPS"));)
    }while(0);

    palFreeMemory( pMac->hHdd, (void *) pMIMO_PSParams );
#endif
}

/**
  *     @function : limHandleDelBssInReAssocContext
  *     @brief      : While Processing the ReAssociation Response Frame in STA,
  *                         a. immediately after receiving the Reassoc Response the RxCleanUp is 
  *                         being issued and the end of DelBSS the new BSS is being added.
  *
  *                         b .If an AP rejects the ReAssociation (Disassoc / Deauth) with some context
  *                         change, We need to update CSR with ReAssocCNF Response with the 
  *                         ReAssoc Fail and the reason Code, that is also being handled in the DELBSS
  *                         context only
  *
  *     @param :   pMac - tpAniSirGlobal
  *                     pStaDs - Station Descriptor
  *     
  *     @return :  none
  */
static void
limHandleDelBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs)
{
    tLimMlmReassocCnf           mlmReassocCnf;
    
    /** Skipped the DeleteDPH Hash Entry as we need it for the new BSS*/
    /** Set the MlmState to IDLE*/
    pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
   MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
    
    switch (pMac->lim.gLimSmeState) 
    {

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        case eLIM_SME_WT_REASSOC_STATE : 
        {
            tpSirAssocRsp assocRsp;
            tpDphHashNode   pStaDs;
            tSirRetStatus       retStatus = eSIR_SUCCESS;
#ifdef ANI_PRODUCT_TYPE_CLIENT
            tSchBeaconStruct beaconStruct;
#endif
            /** Delete the older STA Table entry */
            limDeleteDphHashEntry(pMac, pMac->lim.gLimCurrentBssId, DPH_STA_HASH_INDEX_PEER);
           
            /**
             * Add an entry for AP to hash table
             * maintained by DPH module
             */
            if ((pStaDs = dphAddHashEntry(pMac, pMac->lim.gLimReassocBssId, DPH_STA_HASH_INDEX_PEER)) == NULL) 
            {
                // Could not add hash table entry
                PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for \n"));)
                limPrintMacAddr(pMac, pMac->lim.gLimReassocBssId, LOGE);
            
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_SME_SUCCESS;
            
                goto Error;
            }
            /** While Processing the ReAssoc Response Frame the ReAssocRsp Frame
            *   is being stored to be used here for sending ADDBSS
            */
            assocRsp = (tpSirAssocRsp)pMac->lim.gLimAssocResponseData;

            limUpdateAssocStaDatas(pMac, pStaDs, assocRsp);
            limUpdateReAssocGlobals(pMac, assocRsp);
#ifdef ANI_PRODUCT_TYPE_CLIENT
            limExtractApCapabilities( pMac,
                  (tANI_U8 *) pMac->lim.gpLimReassocReq->bssDescription.ieFields,
                  limGetIElenFromBssDescription( &pMac->lim.gpLimReassocReq->bssDescription ),
                    &beaconStruct );
            
            if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
                limDecideStaProtectionOnAssoc(pMac, beaconStruct);
    
            if(beaconStruct.erpPresent) {
                if (beaconStruct.erpIEInfo.barkerPreambleMode)
                    pMac->lim.gLimShortPreamble = 0;
                else
                    pMac->lim.gLimShortPreamble = 1;
            }

            //updateBss flag is false, as in this case, PE is first deleting the existing BSS and then adding a new one.
            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), beaconStruct, 
                                                    &pMac->lim.gpLimReassocReq->bssDescription, false))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#elif defined(ANI_AP_CLIENT_SDK)

            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), &pMac->lim.gpLimReassocReq->neighborBssList.bssList[0], false))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#endif            
            if (retStatus != eSIR_SUCCESS) 
            {
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                palFreeMemory(pMac->hHdd, assocRsp);
                pMac->lim.gLimAssocResponseData = NULL;
                goto Error;
            }
            palFreeMemory(pMac->hHdd, assocRsp);
            pMac->lim.gLimAssocResponseData = NULL;
        }
        break;
        
        case eLIM_SME_WT_REASSOC_LINK_FAIL_STATE: 
        {     
            /** Case wherein the DisAssoc / Deauth 
             *   being sent as response to ReAssoc Req*/
            /** Send the Reason code as the same received in Disassoc / Deauth Frame*/
            mlmReassocCnf.resultCode = pStaDs->mlmStaContext.disassocReason;
            mlmReassocCnf.protStatusCode = pStaDs->mlmStaContext.cleanupTrigger;

            /** Set the SME State back to WT_Reassoc State*/
            pMac->lim.gLimSmeState = eLIM_SME_WT_REASSOC_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
            
            limDeleteDphHashEntry(pMac, pStaDs->staAddr, pStaDs->assocId);

            if(pMac->lim.gLimSystemRole == eLIM_STA_ROLE) 
            {
                pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
                MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
            }
           
            limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
        }
        break;
#endif
        default:
            PELOGE(limLog(pMac, LOGE, FL("DelBss is being invoked in the wrong system Role /unhandled  SME State\n"));)
            mlmReassocCnf.resultCode = eSIR_SME_REFUSED;
            mlmReassocCnf.protStatusCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            goto Error;
    }

return;
Error:
    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);    
        
}



/**
  *     @function : limHandleAddBssInReAssocContext
  *     @brief      : While Processing the ReAssociation Response Frame in STA,
  *                         a. immediately after receiving the Reassoc Response the RxCleanUp is 
  *                         being issued and the end of DelBSS the new BSS is being added.
  *
  *                         b .If an AP rejects the ReAssociation (Disassoc / Deauth) with some context
  *                         change, We need to update CSR with ReAssocCNF Response with the 
  *                         ReAssoc Fail and the reason Code, that is also being handled in the DELBSS
  *                         context only
  *
  *     @param :   pMac - tpAniSirGlobal
  *                     pStaDs - Station Descriptor
  *     
  *     @return :  none
  */
void
limHandleAddBssInReAssocContext(tpAniSirGlobal pMac, tpDphHashNode pStaDs)
{
    tLimMlmReassocCnf           mlmReassocCnf;
    
    /** Skipped the DeleteDPH Hash Entry as we need it for the new BSS*/
    /** Set the MlmState to IDLE*/
    pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
    
    switch (pMac->lim.gLimSmeState) {

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        case eLIM_SME_WT_REASSOC_STATE : {
            tpSirAssocRsp assocRsp;
            tpDphHashNode   pStaDs;
            tSirRetStatus       retStatus = eSIR_SUCCESS;
#ifdef ANI_PRODUCT_TYPE_CLIENT
            tSchBeaconStruct beaconStruct;
#endif

            // Get the AP entry from DPH hash table 
            pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
            if (pStaDs == NULL )
            {
         		limLog(pMac, LOGE, FL("Fail to get STA PEER entry from hash\n"));         
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_SME_SUCCESS;
            
                goto Error;
            }
            /** While Processing the ReAssoc Response Frame the ReAssocRsp Frame
            *   is being stored to be used here for sending ADDBSS
            */
            assocRsp = (tpSirAssocRsp)pMac->lim.gLimAssocResponseData;

            limUpdateAssocStaDatas(pMac, pStaDs, assocRsp);
            limUpdateReAssocGlobals(pMac, assocRsp);
#ifdef ANI_PRODUCT_TYPE_CLIENT
            limExtractApCapabilities( pMac,
                  (tANI_U8 *) pMac->lim.gpLimReassocReq->bssDescription.ieFields,
                  limGetIElenFromBssDescription( &pMac->lim.gpLimReassocReq->bssDescription ),
                    &beaconStruct );
            
            if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
                limDecideStaProtectionOnAssoc(pMac, beaconStruct);
    
            if(beaconStruct.erpPresent) {
                if (beaconStruct.erpIEInfo.barkerPreambleMode)
                    pMac->lim.gLimShortPreamble = 0;
                else
                    pMac->lim.gLimShortPreamble = 1;
            }
    
            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), beaconStruct, 
                                                    &pMac->lim.gpLimReassocReq->bssDescription, true))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#elif defined(ANI_AP_CLIENT_SDK)

            if (eSIR_SUCCESS != limStaSendAddBss( pMac, (*assocRsp), &pMac->lim.gpLimReassocReq->neighborBssList.bssList[0], true))  {
                limLog( pMac, LOGE, FL( "Posting ADDBSS in the ReAssocContext has Failed \n"));
                retStatus = eSIR_FAILURE;
            }
#endif            
            if (retStatus != eSIR_SUCCESS) 
            {
                mlmReassocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                palFreeMemory(pMac->hHdd, assocRsp);
                pMac->lim.gLimAssocResponseData = NULL;
                goto Error;
            }
            palFreeMemory(pMac->hHdd, assocRsp);
            pMac->lim.gLimAssocResponseData = NULL;
        }
        break;
        
        case eLIM_SME_WT_REASSOC_LINK_FAIL_STATE: {     /** Case wherein the DisAssoc / Deauth 
                                                                                     *   being sent as response to ReAssoc Req*/
            /** Send the Reason code as the same received in Disassoc / Deauth Frame*/
            mlmReassocCnf.resultCode = pStaDs->mlmStaContext.disassocReason;
            mlmReassocCnf.protStatusCode = pStaDs->mlmStaContext.cleanupTrigger;

            /** Set the SME State back to WT_Reassoc State*/
            pMac->lim.gLimSmeState = eLIM_SME_WT_REASSOC_STATE;
            
            limDeleteDphHashEntry(pMac, pStaDs->staAddr, pStaDs->assocId);

            if(pMac->lim.gLimSystemRole == eLIM_STA_ROLE) 
              pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
           
            limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);
        }
        break;
#endif
        default:
            limLog(pMac, LOGE, FL("DelBss is being invoked in the wrong system Role /unhandled  SME State\n"));
            mlmReassocCnf.resultCode = eSIR_SME_REFUSED;
            mlmReassocCnf.protStatusCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            goto Error;
    }

return;
Error:
    limPostSmeMessage(pMac, LIM_MLM_REASSOC_CNF, (tANI_U32 *) &mlmReassocCnf);    
        
}
