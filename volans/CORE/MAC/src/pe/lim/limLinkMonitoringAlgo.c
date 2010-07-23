/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limLinkMonitoringAlgo.cc contains the code for
 * Link monitoring algorithm on AP and heart beat failure
 * handling on STA.
 * Author:        Chandra Modumudi
 * Date:          03/01/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "aniGlobal.h"
#include "wniCfgAp.h"
#include "halDataStruct.h"
#include "cfgApi.h"
#include "halCommonApi.h"

#include "schApi.h"
#include "pmmApi.h"
#include "utilsApi.h"
#include "limAssocUtils.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limPropExtsUtils.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_log.h"
#endif //FEATURE_WLAN_DIAG_SUPPORT

/**
 * limSendKeepAliveToPeer()
 *
 *FUNCTION:
 * This function is called to send Keep alive message to peer
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * NA
 *
 * @param  pMac        - Pointer to Global MAC structure
 * @return None
 */

void
limSendKeepAliveToPeer(tpAniSirGlobal pMac)
{
    tpDphHashNode   pStaDs;

    // If keep live has been disabled, exit
    if (pMac->sch.keepAlive == 0)
        return;

    if ( (limIsSystemInScanState(pMac) == false) && 
          (pMac->lim.gLimSystemRole == eLIM_AP_ROLE))
    {
        tANI_U16 i;
        tANI_U32        len = SIR_MAC_MAX_SSID_LENGTH;
        tAniSSID   ssId;

        /*
        ** send keepalive NULL data frame for each
        ** associated STA;
        */

        for (i=2; i<pMac->lim.maxStation; i++)
        {
            pStaDs = dphGetHashEntry(pMac, i);

            if (pStaDs && pStaDs->added &&
                (pStaDs->mlmStaContext.mlmState == eLIM_MLM_LINK_ESTABLISHED_STATE))
            {
                // SP-Tx hangs at times when a zero-lenght packet is transmitted
                // To avoid any interoperability issue with third party clinet
                // instead of sending a non-zero data-null packet, AP sends a
                // probe response as a keep alive packet.
                if (wlan_cfgGetStr(pMac, WNI_CFG_SSID,
                                (tANI_U8 *) &ssId.ssId,
                                (tANI_U32 *) &len) != eSIR_SUCCESS)
                {
                        /// Could not get SSID from CFG. Log error.
                    limLog(pMac, LOGP, FL("could not retrieve SSID\n"));
                }
                ssId.length = (tANI_U8) len;
 
                PELOG2(limLog(pMac, LOG2,  FL("Sending keepalive Probe Rsp Msg to "));
                limPrintMacAddr(pMac, pStaDs->staAddr, LOG2);)
                limSendProbeRspMgmtFrame(pMac,
                                         pStaDs->staAddr,
                                         &ssId,
                                         i,
                                         DPH_KEEPALIVE_FRAME);
            }
        }
    }
} /*** limSendKeepAliveToPeer() ***/


/** ---------------------------------------------------------
\fn      limDeleteStaContext
\brief   This function handles the message from HAL:
\        SIR_HAL_DELETE_STA_CONTEXT_IND. This function
\        validates that the given station id exist, and if so,
\        deletes the station by calling limTriggerSTAdeletion. 
\param   tpAniSirGlobal pMac
\param   tpSirMsgQ      limMsg
\return  none
  -----------------------------------------------------------*/
void
limDeleteStaContext(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
    tpDeleteStaContext  pMsg = (tpDeleteStaContext)limMsg->bodyptr;
    tpDphHashNode       pStaDs;

    if (NULL != pMsg)
    {
        pStaDs = dphGetHashEntry(pMac, pMsg->assocId);
        if (! pStaDs)
        {
            PELOGW(limLog(pMac, LOGW, FL("Skip STA deletion (invalid STA)\n"));)
            palFreeMemory(pMac->hHdd, limMsg);
            return;
        }

        /* check and see if same staId. This is to avoid the scenario 
         * where we're trying to delete a staId we just added. 
         */
        if (pStaDs->staIndex != pMsg->staId)
        {
            PELOGW(limLog(pMac, LOGW, FL("staid mismatch: %d vs %d \n"), pStaDs->staIndex, pMsg->staId);)
            palFreeMemory(pMac->hHdd, limMsg);
            return;
        }

        PELOG1(limLog(pMac, LOG1, FL("lim Delete Station Context (staId: %d, assocId: %d) \n"),pMsg->staId, pMsg->assocId);)
        limTriggerSTAdeletion(pMac, pStaDs);
    }

    palFreeMemory(pMac->hHdd, limMsg);
    return;
}


/**
 * limTriggerSTAdeletion()
 *
 *FUNCTION:
 * This function is called to trigger STA context deletion
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * NA
 *
 * @param  pMac   - Pointer to global MAC structure
 * @param  pStaDs - Pointer to internal STA Datastructure
 * @return None
 */

void
limTriggerSTAdeletion(tpAniSirGlobal pMac, tpDphHashNode pStaDs)
{
    tSirSmeDeauthReq    smeDeauthReq;

    if (! pStaDs)
    {
        PELOGW(limLog(pMac, LOGW, FL("Skip STA deletion (invalid STA)\n"));)
        return;
    }
    /**
     * MAC based Authentication was used. Trigger
     * Deauthentication frame to peer since it will
     * take care of disassociation as well.
     */
    sirStoreU16N((tANI_U8 *) &(smeDeauthReq.messageType),
                 eWNI_SME_DEAUTH_REQ);
    sirStoreU16N((tANI_U8 *) &(smeDeauthReq.length),
                 sizeof(tSirSmeDeauthReq));
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &smeDeauthReq.peerMacAddr,
                  (tANI_U8 *) pStaDs->staAddr,
                  sizeof(tSirMacAddr));
    sirStoreU16N((tANI_U8 *) &(smeDeauthReq.reasonCode),
                 (tANI_U16) eLIM_LINK_MONITORING_DEAUTH);
#if (WNI_POLARIS_FW_PRODUCT == AP)
    sirStoreU16N((tANI_U8 *) &(smeDeauthReq.aid), pStaDs->assocId);
#endif

    limPostSmeMessage(pMac,
                      eWNI_SME_DEAUTH_REQ,
                      (tANI_U32 *) &smeDeauthReq);
} /*** end limTriggerSTAdeletion() ***/



/**
 * limTearDownLinkWithAp()
 *
 *FUNCTION:
 * This function is called when heartbeat (beacon reception)
 * fails on STA
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limTearDownLinkWithAp(tpAniSirGlobal pMac)
{
    tpDphHashNode pStaDs;

    /**
     * Heart beat failed for upto threshold value
     * and AP did not respond for Probe request.
     * Trigger link tear down.
     */

    limLog(pMac, LOGW,
       FL("No ProbeRsp from AP after HB failure. Tearing down link\n"));

    // Deactivate heartbeat timer
    limDeactivateAndChangeTimer(pMac, eLIM_HEART_BEAT_TIMER);

    // Announce loss of link to Roaming algorithm
    // and cleanup by sending SME_DISASSOC_REQ to SME

    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
    limTriggerSTAdeletion(pMac, pStaDs);
} /*** limTearDownLinkWithAp() ***/




/**
 * limHandleHeartBeatFailure()
 *
 *FUNCTION:
 * This function is called when heartbeat (beacon reception)
 * fails on STA
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limHandleHeartBeatFailure(tpAniSirGlobal pMac)
{
    tSirMacSSid    ssId;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    vos_log_beacon_update_pkt_type *log_ptr = NULL;
#endif //FEATURE_WLAN_DIAG_SUPPORT 

	/* If gLimHeartBeatTimer fires between the interval of sending SIR_HAL_ENTER_BMPS_REQUEST 
	 * to the HAL and receiving SIR_HAL_ENTER_BMPS_RSP from the HAL, then LIM (PE) tries to Process the
	 * SIR_LIM_HEAR_BEAT_TIMEOUT message but The PE state is ePMM_STATE_BMPS_SLEEP so PE dont
	 * want to handle heartbeat timeout in the BMPS, because Firmware handles it in BMPS.
	 * So just return from heartbeatfailure handler
	 */

	if(!limIsSystemInActiveState(pMac))
			return ;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_beacon_update_pkt_type, LOG_WLAN_BEACON_UPDATE_C);
    if(log_ptr)
        log_ptr->bcn_rx_cnt = pMac->lim.gLimRxedBeaconCntDuringHB;
    WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif //FEATURE_WLAN_DIAG_SUPPORT

	/** Re Activate Timer if the system is Waiting for ReAssoc Response*/
	if(LIM_IS_CONNECTION_ACTIVE(pMac) || limIsReassocInProgress(pMac))
	{
		if(pMac->lim.gLimRxedBeaconCntDuringHB < MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL)
			pMac->lim.gLimHeartBeatBeaconStats[pMac->lim.gLimRxedBeaconCntDuringHB]++;
  		else
			pMac->lim.gLimHeartBeatBeaconStats[0]++;
			limReactivateTimer(pMac, eLIM_HEART_BEAT_TIMER);

		// Reset number of beacons received
		limResetHBPktCount(pMac);

		return;
	}

    if ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pMac->lim.gLimMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE))
    {
        if (!pMac->sys.gSysEnableLinkMonitorMode)
            return;

        /**
         * Beacon frame not received within heartbeat timeout.
         */
        PELOGW(limLog(pMac, LOGW, FL("Heartbeat Failure\n"));)
        pMac->lim.gLimHBfailureCntInLinkEstState++;

      /**
             * Send Probe Request frame to AP to see if
             * it is still around. Wait until certain
             * timeout for Probe Response from AP.
             */
            ssId.length = 0;
            PELOGW(limLog(pMac, LOGW, FL("Heart Beat missed from AP. Sending Probe Req\n"));)

            limSendProbeReqMgmtFrame(pMac, &ssId, pMac->lim.gLimCurrentBssId,
                                     pMac->lim.gLimCurrentChannelId);
            limDeactivateAndChangeTimer(pMac, eLIM_PROBE_AFTER_HB_TIMER);
	     MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_PROBE_AFTER_HB_TIMER));
            if (tx_timer_activate(&pMac->lim.limTimers.gLimProbeAfterHBTimer) != TX_SUCCESS)
            {
                /// Could not activate Probe-after-heartbeat timer.
                // Log error
                limLog(pMac, LOGP, FL("could not start Probe-after-heartbeat timer\n"));
                limReactivateTimer(pMac, eLIM_HEART_BEAT_TIMER);
            }
    }
    else
    {
        /**
         * Heartbeat timer may have timed out
         * while we're doing background scanning/learning
         * or in states other than link-established state.
         * Log error.
         */
        PELOG1(limLog(pMac, LOG1, FL("received heartbeat timeout in state %X\n"),
               pMac->lim.gLimMlmState);)
        limPrintMlmState(pMac, LOG1, pMac->lim.gLimMlmState);
        pMac->lim.gLimHBfailureCntInOtherStates++;
        limReactivateTimer(pMac, eLIM_HEART_BEAT_TIMER);
    }
} /*** limHandleHeartBeatFailure() ***/
