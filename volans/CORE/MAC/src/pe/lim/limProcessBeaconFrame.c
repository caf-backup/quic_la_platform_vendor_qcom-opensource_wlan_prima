/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessBeaconFrame.cc contains the code
 * for processing Received Beacon Frame.
 * Author:        Chandra Modumudi
 * Date:          03/01/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "aniGlobal.h"
#include "cfgApi.h"
#include "schApi.h"
#include "wniCfgAp.h"
#include "halCommonApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limPropExtsUtils.h"
#include "limSerDesUtils.h"

/**
 * limProcessBeaconFrame
 *
 *FUNCTION:
 * This function is called by limProcessMessageQueue() upon Beacon
 * frame reception.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * 1. Beacons received in 'normal' state in IBSS are handled by
 *    Beacon Processing module.
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pBd - A pointer to Buffer descriptor + associated PDUs
 * @return None
 */

void
limProcessBeaconFrame(tpAniSirGlobal pMac, tANI_U32 *pBd)
{
    tpSirMacMgmtHdr      pHdr;
    tSchBeaconStruct     beacon;

    pMac->lim.gLimNumBeaconsRcvd++;


    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);


   PELOG2(limLog(pMac, LOG2, FL("Received Beacon frame with length=%d from "),
           SIR_MAC_BD_TO_MPDU_LEN(pBd));
    limPrintMacAddr(pMac, pHdr->sa, LOG2);)

    if (limDeactivateMinChannelTimerDuringScan(pMac) != eSIR_SUCCESS)
        return;

    /**
     * Expect Beacon only when
     * 1. STA is in Scan mode waiting for Beacon/Probe response or
     * 2. STA is waiting for Beacon/Probe Respose Frame
     *    to announce join success.
     * 3. STA/AP is in Learn mode
     */
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE) ||
        (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE))
    {
        // Parse received Beacon
        if (sirConvertBeaconFrame2Struct(pMac, (tANI_U8 *) pBd,
                                         &beacon) != eSIR_SUCCESS)
        {
            // Received wrongly formatted/invalid Beacon.
            // Ignore it and move on.
            limLog(pMac, LOGW,
                   FL("Received invalid Beacon in state %X\n"),
                   pMac->lim.gLimMlmState);
            limPrintMlmState(pMac, LOGW, pMac->lim.gLimMlmState);
            return;
        }

       MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, beacon.timeStamp[0]);)
       MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, beacon.timeStamp[1]);)



        if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) ||
            (pMac->lim.gLimMlmState == eLIM_MLM_PASSIVE_SCAN_STATE))
            limCheckAndAddBssDescription(pMac, &beacon, pBd, ((pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE) ? eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE));
        else if (pMac->lim.gLimMlmState == eLIM_MLM_LEARN_STATE)
        {
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            // STA/AP is in learn mode
            /* Not sure whether the below 2 lines are needed for the station. TODO If yes, this should be 
             * uncommented. Also when we tested enabling this, there is a crash as soon as the station
             * comes up which needs to be fixed*/
            //if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
              //  limCheckAndAddBssDescription(pMac, &beacon, pBd, eANI_BOOLEAN_TRUE);
            limCollectMeasurementData(pMac, pBd, &beacon);
           PELOG3(limLog(pMac, LOG3, FL("Parsed WDS info in Beacon frames: wdsLength=%d\n"),
               beacon.propIEinfo.wdsLength);)
#endif
        }
        else
        {
            if( pMac->lim.beacon != NULL )
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
             
             // STA in WT_JOIN_BEACON_STATE (IBSS)
            limCheckAndAnnounceJoinSuccess(pMac, &beacon, pHdr);
        } // if (pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE)
    } // if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) || ...
    else
    {
        // Ignore Beacon frame in all other states
        if (pMac->lim.gLimMlmState == eLIM_MLM_JOINED_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_BSS_STARTED_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_AUTH_FRAME2_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_AUTH_FRAME3_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_AUTH_FRAME4_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_AUTH_RSP_TIMEOUT_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_AUTHENTICATED_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_ASSOC_RSP_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_REASSOC_RSP_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_ASSOCIATED_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_REASSOCIATED_STATE ||
            pMac->lim.gLimMlmState == eLIM_MLM_WT_ASSOC_CNF_STATE ||
            limIsReassocInProgress(pMac)) {
            // nothing unexpected about beacon in these states
            pMac->lim.gLimNumBeaconsIgnored++;
        }
        else
        {
            PELOG1(limLog(pMac, LOG1, FL("Received Beacon in unexpected state %d\n"),
                   pMac->lim.gLimMlmState);
            limPrintMlmState(pMac, LOG1, pMac->lim.gLimMlmState);)
#ifdef WLAN_DEBUG                    
            pMac->lim.gLimUnexpBcnCnt++;
#endif
        }
    }

    return;
} /*** end limProcessBeaconFrame() ***/

