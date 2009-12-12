/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file schBeaconProcess.cc contains beacon processing related
 * functions
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "palTypes.h"
#include "wniCfgAp.h"

#include "cfgApi.h"
#include "pmmApi.h"
#include "limApi.h"
#include "utilsApi.h"
#include "schDebug.h"
#include "schApi.h"

#include "halCommonApi.h"
#include "limUtils.h"
#include "limSendMessages.h"
#include "limStaHashApi.h"

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_log.h"
#endif //FEATURE_WLAN_DIAG_SUPPORT 

/**
 * Number of bytes of variation in beacon length from the last beacon
 * to trigger reprogramming of rx delay register
 */
#define SCH_BEACON_LEN_DELTA       3

// calculate 2^cw - 1
#define CW_GET(cw) (((cw) == 0) ? 1 : ((1 << (cw)) - 1))

static void
ap_beacon_process(
    tpAniSirGlobal    pMac,
    tpHalBufDesc      pBD,
    tpSchBeaconStruct pBcnStruct,
    tpUpdateBeaconParams pBeaconParams)
{
    (void) pMac; (void) pBD; (void) pBcnStruct;

#ifdef ANI_PRODUCT_TYPE_AP
    tpSirMacMgmtHdr    pMh = SIR_MAC_BD_TO_MPDUHEADER(pBD);
    tANI_U32           phyMode;
    tSirRFBand          rfBand = SIR_BAND_UNKNOWN;
    limGetRfBand(pMac, &rfBand);
    limGetPhyMode(pMac, &phyMode);    
    if(SIR_BAND_5_GHZ == rfBand)
    {
        if(pMac->lim.htCapability)
        {
            if (pBcnStruct->channelNumber == pMac->lim.gLimCurrentChannelId)
            {
              //11a (non HT) AP  overlaps or
              //HT AP with HT op mode as mixed overlaps.              
              //HT AP with HT op mode as overlap legacy overlaps.                            
              if ((!pBcnStruct->HTInfo.present) ||
                  (eSIR_HT_OP_MODE_MIXED == pBcnStruct->HTInfo.opMode) ||
                  (eSIR_HT_OP_MODE_OVERLAP_LEGACY == pBcnStruct->HTInfo.opMode))
              {
                   limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11aParams));

                  if (pMac->lim.gLimOverlap11aParams.numSta &&
                      !pMac->lim.gLimOverlap11aParams.protectionEnabled)
                  {
                      limEnable11aProtection(pMac, true, true, pBeaconParams);
                  }
              }
              //HT AP with HT20 op mode overlaps.
              else if(eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT == pBcnStruct->HTInfo.opMode)
              {
                  limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlapHt20Params));

                  if (pMac->lim.gLimOverlapHt20Params.numSta &&
                      !pMac->lim.gLimOverlapHt20Params.protectionEnabled)
                  {
                      limEnableHT20Protection(pMac, true, true, pBeaconParams);
                  }
              }
            }
        }    
    }
    else if(SIR_BAND_2_4_GHZ == rfBand)
    {
        //We are 11G AP.
        if ((phyMode == WNI_CFG_PHY_MODE_11G) &&
              (false == pMac->lim.htCapability))
        {
            if (pBcnStruct->channelNumber == pMac->lim.gLimCurrentChannelId)        
            {
                if (pBcnStruct->erpPresent &&
                    (pBcnStruct->erpIEInfo.useProtection ||
                    pBcnStruct->erpIEInfo.nonErpPresent))
                {
                    limEnableOverlap11gProtection(pMac, pBeaconParams, pMh);
                }
            }
        }        
        // handling the case when HT AP has overlapping legacy BSS.
        else if(pMac->lim.htCapability)
        {
            if (pBcnStruct->channelNumber == pMac->lim.gLimCurrentChannelId)
            {
              if (pBcnStruct->erpPresent &&
                    (pBcnStruct->erpIEInfo.useProtection ||
                    pBcnStruct->erpIEInfo.nonErpPresent))
              {
                  limEnableOverlap11gProtection(pMac, pBeaconParams, pMh);
              }

              //11g device overlaps
              if (pBcnStruct->erpPresent &&
                  !(pBcnStruct->erpIEInfo.useProtection || 
                    pBcnStruct->erpIEInfo.nonErpPresent))
              {
                   limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11gParams));

                  if (pMac->lim.gLimOverlap11gParams.numSta &&
                      !pMac->lim.gLimOverlap11gParams.protectionEnabled)
                  {
                      limEnableHtProtectionFrom11g(pMac, true, true, pBeaconParams);
                  }
              }

              //ht device overlaps.
              //here we will check for HT related devices only which might need protection.
              //check for 11b and 11g is already done in the previous blocks.
              //so we will not check for HT operating mode as MIXED.
              if (pBcnStruct->HTInfo.present)
              {
                  //if we are not already in mixed mode or legacy mode as HT operating mode
                  //and received beacon has HT operating mode as legacy
                  //then we need to enable protection from 11g station. 
                  //we don't need protection from 11b because if that's needed then our operating
                  //mode would have already been set to legacy in the previous blocks.
                  if(eSIR_HT_OP_MODE_OVERLAP_LEGACY == pBcnStruct->HTInfo.opMode)
                  {
                      if((eSIR_HT_OP_MODE_MIXED != pMac->lim.gHTOperMode) &&
                          (eSIR_HT_OP_MODE_OVERLAP_LEGACY != pMac->lim.gHTOperMode))
                      {
                          limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlap11gParams));

                          if (pMac->lim.gLimOverlap11gParams.numSta &&
                              !pMac->lim.gLimOverlap11gParams.protectionEnabled)
                          {
                              limEnableHtProtectionFrom11g(pMac, true, true, pBeaconParams);
                          }
                      }
                  }           
                  else if(eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT == pBcnStruct->HTInfo.opMode)
                  {
                      limUpdateOverlapStaParam(pMac, pMh->bssId, &(pMac->lim.gLimOverlapHt20Params));

                      if (pMac->lim.gLimOverlapHt20Params.numSta &&
                          !pMac->lim.gLimOverlapHt20Params.protectionEnabled)
                      {
                          limEnableHT20Protection(pMac, true, true, pBeaconParams);
                      }
                  }
              }
              
            }
        }     
    }
    pMac->sch.gSchBcnIgnored++;
#endif
}
// --------------------------------------------------------------------
/**
 * schBeaconProcess
 *
 * FUNCTION:
 * Process the received beacon frame
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * When running on threadx, pBD is a pointer to BD in polaris memory,
 * hence everything is copied into CPU memory before processing
 * When running on RTAI, pBD is a pointer to a contiguous block of
 * memory containing the BD followed by beacon payload. After processing,
 * the packet buffer needs to be freed to the HDD.
 * When running on windows ???
 *
 * NOTE:
 *
 * @param pBD pointer to buffer descriptor
 * @return None
 */

void schBeaconProcess(tpAniSirGlobal pMac, tpHalBufDesc pBD)
{
    static tSchBeaconStruct beaconStruct;
    tANI_U32                     bi;
    tANI_U8 bssIdx = 0;
    tpSirMacMgmtHdr         pMh = SIR_MAC_BD_TO_MPDUHEADER(pBD);
    tANI_U8 bssid[sizeof(tSirMacAddr)];
    tUpdateBeaconParams beaconParams;
    tANI_U8 sendProbeReq = FALSE;

    PELOG4(schLog(pMac, LOG4, FL("beacon received\n"));)
    pMac->sch.gSchBcnRcvCnt++;

    beaconParams.paramChangeBitmap = 0;

    // Convert the beacon frame into a structure
    if (sirConvertBeaconFrame2Struct(pMac, (tANI_U8 *) pBD, &beaconStruct)
        != eSIR_SUCCESS)
        {
        PELOGE(schLog(pMac, LOGE, FL("beacon parsing failed\n"));)
        pMac->sch.gSchBcnParseErrorCnt++;
        goto fail;
    }

    if (beaconStruct.ssidPresent)
    {
        beaconStruct.ssId.ssId[beaconStruct.ssId.length] = 0;
       PELOG1(schLog(pMac, LOG1, FL("Bcn: [Channel %d] SSID.length %d (%s)\n"),
               beaconStruct.channelNumber,
               beaconStruct.ssId.length,
               beaconStruct.ssId.ssId);)
    }

    if (limGetSystemRole(pMac) == eLIM_AP_ROLE)
    {
        if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
            ap_beacon_process(pMac,  pBD, &beaconStruct, &beaconParams);
        goto updateBeaconFieldsAndParams;
    }

    /*
     * Check if IBSS coalescing happened. If so,
     * adopt to parameters received in Beacon.
     */
    if (limGetSystemRole(pMac) == eLIM_STA_IN_IBSS_ROLE)
    {
        if (limHandleIBSScoalescing(pMac, &beaconStruct, (tANI_U32 *) pBD)
            != eSIR_SUCCESS)
        {
            /*
             * Either SSID did not match or Beacon from different BSS has timestamp before
             * current IBSS TSF timer. * Ignore received Beacon Frame.
             */
            goto fail;
        }
    }
    else   // must be a STA role
    {
        //Always save the beacon into LIM's cached scan results
        limCheckAndAddBssDescription(pMac, &beaconStruct, (tANI_U32 *)pBD, eANI_BOOLEAN_FALSE);
        // If BSS id does not match, save beacon as to the scan result
        limGetBssid(pMac, bssid);
        if (palEqualMemory( pMac->hHdd,bssid, pMh->bssId, 6) == 0)
        {
            PELOG4(schLog(pMac, LOG4, FL("Rcvd beacon from different BSS\n"));)

            goto fail;
        }

        /**
        * This is the Beacon received from the AP  we're currently associated with. Check
        * if there are any changes in AP's capabilities 
       */
       limDetectChangeInApCapabilities(pMac, &beaconStruct);

        if(limGetStaHashBssidx(pMac, DPH_STA_HASH_INDEX_PEER, &bssIdx) != eSIR_SUCCESS)
            goto fail;
        beaconParams.bssIdx = bssIdx;
        palCopyMemory( pMac->hHdd, ( tANI_U8* )&pMac->lim.gLastBeaconTimeStamp, ( tANI_U8* )beaconStruct.timeStamp, sizeof(tANI_U64) );
        pMac->lim.gLastBeaconDtimCount = beaconStruct.tim.dtimCount;
        pMac->lim.gLastBeaconDtimPeriod= beaconStruct.tim.dtimPeriod;
        pMac->lim.gCurrentBssBeaconCnt++;


        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, beaconStruct.timeStamp[0]);)
        MTRACE(macTrace(pMac, TRACE_CODE_RX_MGMT_TSF, 0, beaconStruct.timeStamp[1]);)


        //pmmWakeupHandler(pMac);
    }
    // At this point, this is a beacon from our AP or IBSS, so we process it

#ifdef ANI_PRODUCT_TYPE_AP
#ifndef ANI_AP_SDK
    // check for WDS info
        limProcessWdsInfo(pMac, beaconStruct.propIEinfo);
#endif // !ANI_AP_SDK
#endif

    if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &bi) != eSIR_SUCCESS)
        schLog(pMac, LOGP, FL("Can't read beacon interval\n"));
    if (bi != beaconStruct.beaconInterval)
    {
       PELOG1(schLog(pMac, LOG1, FL("Beacon interval changed from %d to %d\n"),
               beaconStruct.beaconInterval, bi);)

        // Update beacon interval (if changed) at infra STAs
        if (limGetSystemRole(pMac) == eLIM_STA_ROLE)
        {
            bi = beaconStruct.beaconInterval;
            cfgSetInt(pMac, WNI_CFG_BEACON_INTERVAL, bi);
            beaconParams.paramChangeBitmap |= PARAM_BCN_INTERVAL_CHANGED;
            beaconParams.beaconInterval = (tANI_U16)bi;
        }
        else
            PELOGE(schLog(pMac, LOGE, FL("ERROR : Beacon interval can't change in IBSS!!\n"));)
    }

    if (limGetSystemRole(pMac) == eLIM_STA_ROLE)
    {
        tpDphHashNode pStaDs = NULL;

        /*
         * At the STA in infrastructure mode, write the DTIM/CFP period/count values
         * to the hardware and CFG
         */

        if (beaconStruct.cfPresent)
        {
            cfgSetInt(pMac, WNI_CFG_CFP_PERIOD, beaconStruct.cfParamSet.cfpPeriod);
            limSendCFParams(pMac, bssIdx, beaconStruct.cfParamSet.cfpCount, beaconStruct.cfParamSet.cfpPeriod);
        }

        if (beaconStruct.timPresent)
        {
            cfgSetInt(pMac, WNI_CFG_DTIM_PERIOD, beaconStruct.tim.dtimPeriod);
            //No need to send DTIM Period and Count to HAL/SMAC
            //SMAC already parses TIM bit.
        }

        
        if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
            limDecideStaProtection(pMac, beaconStruct, &beaconParams);
        if (beaconStruct.erpPresent)
        {
              if (beaconStruct.erpIEInfo.barkerPreambleMode)
                  limEnableShortPreamble(pMac, false, &beaconParams);
              else
                  limEnableShortPreamble(pMac, true, &beaconParams);
          }
        limUpdateShortSlot(pMac, &beaconStruct, &beaconParams);

        pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
        if ((beaconStruct.wmeEdcaPresent && (pMac->lim.gLimWmeEnabled)) ||
             (beaconStruct.edcaPresent    && (pMac->lim.gLimQosEnabled)))
        {
            if(beaconStruct.edcaParams.qosInfo.count != pMac->sch.schObject.gSchEdcaParamSetCount)
            {
                if (schBeaconEdcaProcess(pMac, &beaconStruct.edcaParams) != eSIR_SUCCESS)
                    PELOGE(schLog(pMac, LOGE, FL("EDCA parameter processing error\n"));)
                else if(pStaDs != NULL)
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
        else if( (beaconStruct.qosCapabilityPresent && pMac->lim.gLimQosEnabled) &&
            (beaconStruct.qosCapability.qosInfo.count != pMac->sch.schObject.gSchEdcaParamSetCount))
            sendProbeReq = TRUE;
    }

    // at this point, STA/BP received Beacon from its own AP.
    if ( pMac->lim.htCapability && beaconStruct.HTInfo.present )
    {
        limUpdateStaRunTimeHTSwitchChnlParams( pMac, &beaconStruct.HTInfo, bssIdx );
    }

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    if ( (pMac->lim.gLimSystemRole == eLIM_STA_ROLE) || 
          (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) )
    {
        if(beaconStruct.quietIEPresent)
        {
            limUpdateQuietIEFromBeacon(pMac, &(beaconStruct.quietIE));
        }
        else if ((pMac->lim.gLimSpecMgmt.quietState == eLIM_QUIET_BEGIN) ||
             (pMac->lim.gLimSpecMgmt.quietState == eLIM_QUIET_RUNNING))
        {
            PELOG1(limLog(pMac, LOG1, FL("Received a beacon without Quiet IE\n"));)
            limCancelDot11hQuiet(pMac);
        }

        /* Channel Switch information element updated */
        if(beaconStruct.channelSwitchPresent || 
            beaconStruct.propIEinfo.propChannelSwitchPresent)
        {
            limUpdateChannelSwitch(pMac, &beaconStruct);
        }
        else if (pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
        {
            limCancelDot11hChannelSwitch(pMac);
        }   
    }
#endif

    // Indicate to LIM that Beacon is received
    if (beaconStruct.HTInfo.present)
        limReceivedHBHandler(pMac, (tANI_U8)beaconStruct.HTInfo.primaryChannel);
	else
        limReceivedHBHandler(pMac, (tANI_U8)beaconStruct.channelNumber);

    if(sendProbeReq)
        limSendProbeReqMgmtFrame(pMac, &pMac->lim.gLimCurrentSSID,
            pMac->lim.gLimCurrentBssId, pMac->lim.gLimCurrentChannelId);

   PELOG2(schLog(pMac, LOG2, "Received Beacon's SeqNum=%d\n",
           (pMh->seqControl.seqNumHi << 4) | (pMh->seqControl.seqNumLo));)

updateBeaconFieldsAndParams:
    if(beaconParams.paramChangeBitmap)
    {
        if (eLIM_AP_ROLE == pMac->lim.gLimSystemRole)
        {
            schSetFixedBeaconFields(pMac);
        }
        limSendBeaconParams(pMac, &beaconParams);
    }

fail:
    return;
}

// --------------------------------------------------------------------
/**
 * schBeaconEdcaProcess
 *
 * FUNCTION:
 * Process the EDCA parameter set in the received beacon frame
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param edca reference to edca parameters in beacon struct
 * @return success
 */

tSirRetStatus schBeaconEdcaProcess(tpAniSirGlobal pMac, tSirMacEdcaParamSetIE *edca)
{
    tANI_U8 i;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    vos_log_qos_edca_pkt_type *log_ptr = NULL;
#endif //FEATURE_WLAN_DIAG_SUPPORT 

    PELOG1(schLog(pMac, LOG1, FL("Updating parameter set count: Old %d ---> new %d\n"),
           pMac->sch.schObject.gSchEdcaParamSetCount, edca->qosInfo.count);)

    pMac->sch.schObject.gSchEdcaParamSetCount = edca->qosInfo.count;
    pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BE] = edca->acbe;
    pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BK] = edca->acbk;
    pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VI] = edca->acvi;
    pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VO] = edca->acvo;
//log: LOG_WLAN_QOS_EDCA_C
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_qos_edca_pkt_type, LOG_WLAN_QOS_EDCA_C);
    if(log_ptr)
    {
       log_ptr->aci_be = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BE].aci.aci;
       log_ptr->cw_be  = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BE].cw.max << 4 |
          pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BE].cw.min;
       log_ptr->txoplimit_be = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BE].txoplimit;
       log_ptr->aci_bk = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BK].aci.aci;
       log_ptr->cw_bk  = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BK].cw.max << 4 |
          pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BK].cw.min;
       log_ptr->txoplimit_bk = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_BK].txoplimit;
       log_ptr->aci_vi = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VI].aci.aci;
       log_ptr->cw_vi  = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VI].cw.max << 4 |
          pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VI].cw.min;
       log_ptr->txoplimit_vi = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VI].txoplimit;
       log_ptr->aci_vo = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VO].aci.aci;
       log_ptr->cw_vo  = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VO].cw.max << 4 |
          pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VO].cw.min;
       log_ptr->txoplimit_vo = pMac->sch.schObject.gSchEdcaParams[EDCA_AC_VO].txoplimit;
    }
    WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	PELOG1(schLog(pMac, LOGE, FL("Updating Local EDCA Params(gSchEdcaParams) to: "));)
	for(i=0; i<MAX_NUM_AC; i++)
	{
        PELOG1(schLog(pMac, LOG1, FL("AC[%d]:  AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d\n"),
            i,
            pMac->sch.schObject.gSchEdcaParams[i].aci.aifsn, 
            pMac->sch.schObject.gSchEdcaParams[i].aci.acm,
            pMac->sch.schObject.gSchEdcaParams[i].cw.min,
            pMac->sch.schObject.gSchEdcaParams[i].cw.max,
            pMac->sch.schObject.gSchEdcaParams[i].txoplimit);)		
	}
	
    return eSIR_SUCCESS;
}
