/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessProbeRspFrame.cc contains the code
 * for processing Probe Response Frame.
 * Author:        Chandra Modumudi
 * Date:          03/01/02
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
#include "aniGlobal.h"
#include "halCommonApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limPropExtsUtils.h"
#include "limSerDesUtils.h"
#include "limSendMessages.h"

#include "parserApi.h"

/**
 * limProcessProbeRspFrame
 *
 *FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Probe Response frame reception.
 *
 *LOGIC:
 * This function processes received Probe Response frame.
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * 1. Frames with out-of-order IEs are dropped.
 * 2. In case of IBSS, join 'success' makes MLM state machine
 *    transition into 'BSS started' state. This may have to change
 *    depending on supporting what kinda Authentication in IBSS.
 *
 * @param pMac   Pointer to Global MAC structure
 * @param  *pBd  A pointer to Buffer descriptor + associated PDUs
 * @return None
 */


void
limProcessProbeRspFrame(tpAniSirGlobal pMac, tANI_U32 *pBd)
{
    tANI_U8                    *pBody;
    tANI_U32                   frameLen = 0;
    tANI_U32                   cfg = sizeof(tSirMacAddr);
    tSirMacAddr           currentBssId;
    tpSirMacMgmtHdr       pHdr;
    tSirProbeRespBeacon   probeRsp;
    tANI_U8 qosEnabled = false;
    tANI_U8 wmeEnabled = false;


    probeRsp.ssId.length              = 0;
    probeRsp.wpa.length               = 0;
    probeRsp.propIEinfo.apName.length = 0;
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    probeRsp.propIEinfo.aniIndicator  = 0;
    probeRsp.propIEinfo.wdsLength     = 0;
#endif


    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);


   PELOG2(limLog(pMac, LOG2,
             FL("Received Probe Response frame with length=%d from "),
             SIR_MAC_BD_TO_MPDU_LEN(pBd));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);)

    if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
        return;

    /**
     * Expect Probe Response only when
     * 1. STA is in scan mode waiting for Beacon/Probe response or
     * 2. STA is waiting for Beacon/Probe Response to announce
     *    join success or
     * 3. STA is in IBSS mode in BSS started state or
     * 4. STA/AP is in learn mode
     * 5. STA in link established state. In this state, the probe response is
     *     expected for two scenarios:
     *     -- As part of heart beat mechanism, probe req is sent out
     *     -- If QoS Info IE in beacon has a different count for EDCA Params,
     *         and EDCA IE is not present in beacon,
     *         then probe req is sent out to get the EDCA params.
     *
     * Ignore Probe Response frame in all other states
     */
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE) ||
        ((pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
         (pMac->lim.gLimMlmState == eLIM_MLM_BSS_STARTED_STATE)))
    {
        frameLen = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd);

        // Get pointer to Probe Response frame body
        pBody = SIR_MAC_BD_TO_MPDUDATA(pBd);

        if (sirConvertProbeFrame2Struct(pMac, pBody, frameLen, &probeRsp)
                          ==eSIR_FAILURE)
        {
            PELOG1(limLog(pMac, LOG1,
               FL("PArse error ProbeResponse, length=%d\n"),
               frameLen);)
            return;
        }

        if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
            (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE))
            limCheckAndAddBssDescription(pMac, &probeRsp, pBd, ((pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE) ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE));
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)
        {
#if defined(ANI_PRODUCT_TYPE_AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            // STA/AP is in learn mode
            /* Not sure whether the below 2 lines are needed for the station. TODO If yes, this should be 
             * uncommented. Also when we tested enabling this, there is a crash as soon as the station
             * comes up which needs to be fixed*/
            //if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
              //  limCheckAndAddBssDescription(pMac, &probeRsp, pBd, eANI_BOOLEAN_TRUE);
            limCollectMeasurementData(pMac, pBd, &probeRsp);
           PELOG3(limLog(pMac, LOG3,
               FL("Parsed WDS info in ProbeRsp frames: wdsLength=%d\n"),
               probeRsp.propIEinfo.wdsLength);)
#endif
        }
        else if (pMac->lim.gLimMlmState ==
                                     eLIM_MLM_WT_JOIN_BEACON_STATE)
        {
            if( pMac->lim.beacon != NULL ) //Either Beacon/probe response is required. Hence store it in same buffer.
            {
                palFreeMemory(pMac->hHdd, pMac->lim.beacon);
                pMac->lim.beacon = NULL;
             }
             pMac->lim.bcnLen = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd);
             if( (palAllocateMemory(pMac->hHdd, (void**)&pMac->lim.beacon, pMac->lim.bcnLen)) != eSIR_SUCCESS)
             {
                PELOGE(limLog(pMac, LOGE, FL("Unable to allocate memory to store beacon"));)
              }
              else
              {
                //Store the Beacon/ProbeRsp. This is sent to csr/hdd in join cnf response. 
                palCopyMemory(pMac->hHdd, pMac->lim.beacon, SIR_MAC_BD_TO_MPDUDATA(pBd), pMac->lim.bcnLen);
               }
             
        
            // STA in WT_JOIN_BEACON_STATE
            limCheckAndAnnounceJoinSuccess(pMac, &probeRsp, pHdr);
        }
        else if(pMac->lim.gLimMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE)
        {
            tpDphHashNode pStaDs = NULL;
            /**
             * Check if this Probe Response is for
            * our Probe Request sent upon reaching
            * heart beat threshold
            */
            if (wlan_cfgGetStr(pMac,
                          WNI_CFG_BSSID,
                          currentBssId,
                          &cfg) != eSIR_SUCCESS)
            {
                /// Could not get BSSID from CFG. Log error.
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
            }

            if ( !palEqualMemory( pMac->hHdd,currentBssId, pHdr->bssId, sizeof(tSirMacAddr)) )
                return;

            if (!LIM_IS_CONNECTION_ACTIVE(pMac))
            {
                limLog(pMac, LOGW,
                    FL("Received Probe Resp from AP. So it is alive!!\n"));

                if (probeRsp.HTInfo.present)
                    limReceivedHBHandler(pMac, (tANI_U8)probeRsp.HTInfo.primaryChannel);
				else
                    limReceivedHBHandler(pMac, (tANI_U8)probeRsp.channelNumber);
            }

#if defined ANI_PRODUCT_TYPE_CLIENT || defined (ANI_AP_CLIENT_SDK)
            
            if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
            {
                if (probeRsp.quietIEPresent)
                {
                    limUpdateQuietIEFromBeacon(pMac, &(probeRsp.quietIE));
                }
                else if ((pMac->lim.gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN) ||
                     (pMac->lim.gLimSpecMgmt.quietState == eLIM_QUIET_RUNNING))
                {
                    PELOG1(limLog(pMac, LOG1, FL("Received a probe rsp without Quiet IE\n"));)
                    limCancelDot11hQuiet(pMac);
                }

                if (probeRsp.channelSwitchPresent ||
                    probeRsp.propIEinfo.propChannelSwitchPresent)
                {
                    limUpdateChannelSwitch(pMac, &probeRsp);
                }
                else if (pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
                {
                    limCancelDot11hChannelSwitch(pMac);
                }
            }
        
#endif
            
            /**
            * Now Process EDCA Parameters, if EDCAParamSet count is different.
            *     -- While processing beacons in link established state if it is determined that
            *         QoS Info IE has a different count for EDCA Params,
            *         and EDCA IE is not present in beacon,
            *         then probe req is sent out to get the EDCA params.
            */

            pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);

            limGetQosMode(pMac, &qosEnabled);
            limGetWmeMode(pMac, &wmeEnabled);
           PELOG2(limLog(pMac, LOG2,
                    FL("wmeEdcaPresent: %d wmeEnabled: %d, edcaPresent: %d, qosEnabled: %d,  edcaParams.qosInfo.count: %d schObject.gSchEdcaParamSetCount: %d\n"),
                          probeRsp.wmeEdcaPresent, wmeEnabled, probeRsp.edcaPresent, qosEnabled,
                          probeRsp.edcaParams.qosInfo.count, pMac->sch.schObject.gSchEdcaParamSetCount);)
            if (((probeRsp.wmeEdcaPresent && wmeEnabled) ||
                (probeRsp.edcaPresent && qosEnabled)) &&
                (probeRsp.edcaParams.qosInfo.count != pMac->sch.schObject.gSchEdcaParamSetCount))
            {
                if (schBeaconEdcaProcess(pMac, &probeRsp.edcaParams) != eSIR_SUCCESS)
                    PELOGE(limLog(pMac, LOGE, FL("EDCA parameter processing error\n"));)
                else if (pStaDs != NULL)
                {
                    // If needed, downgrade the EDCA parameters
                    limSetActiveEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParams); 

                    if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE)
                        limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
                    else
                        limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
                }
                else
                    PELOGE(limLog(pMac, LOGE, FL("Self Entry missing in Hash Table\n"));)

            }
        }
        else if ((pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
                 (pMac->lim.gLimMlmState == eLIM_MLM_BSS_STARTED_STATE))
                limHandleIBSScoalescing(pMac, &probeRsp, pBd);
    } // if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) || ...

    // Ignore Probe Response frame in all other states
    return;
} /*** end limProcessProbeRspFrame() ***/
