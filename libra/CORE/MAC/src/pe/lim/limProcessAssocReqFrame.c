/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessAssocReqFrame.cc contains the code
 * for processing Re/Association Request Frame.
 * Author:        Chandra Modumudi
 * Date:          03/18/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 05/26/10       js             WPA handling in (Re)Assoc frames
 *
 */
#include "palTypes.h"
#include "aniGlobal.h"
#include "wniCfgAp.h"
#include "sirApi.h"
#include "cfgApi.h"

#include "schApi.h"
#include "pmmApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limStaHashApi.h"
#include "limAdmitControl.h"
#include "palApi.h"


#include "vos_types.h"
/**
 * limConvertSupportedChannels
 *
 *FUNCTION:
 * This function is called by limProcessAssocReqFrame() to
 * parse the channel support IE in the Assoc/Reassoc Request
 * frame, and send relevant information in the SME_ASSOC_IND
 *
 *NOTE:
 *
 * @param  pMac         - A pointer to Global MAC structure
 * @param  pMlmAssocInd - A pointer to SME ASSOC/REASSOC IND
 * @param  assocReq     - A pointer to ASSOC/REASSOC Request frame
 *
 * @return None
 */
static void
limConvertSupportedChannels(tpAniSirGlobal pMac,
                            tpLimMlmAssocInd pMlmAssocInd,
                            tSirAssocReq *assocReq)
{

    tANI_U16   i, j, index=0;
    tANI_U8    firstChannelNumber;
    tANI_U8    numberOfChannel;
    tANI_U8    nextChannelNumber;

    for(i=0; i < (assocReq->supportedChannels.length); i++)
    {
        // Get First Channel Number
        firstChannelNumber = assocReq->supportedChannels.supportedChannels[i];
        pMlmAssocInd->supportedChannels.channelList[index] = firstChannelNumber;
        i++;
        index++;
        if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
        {
            pMlmAssocInd->supportedChannels.numChnl = 0;
            return;
        }
        // Get Number of Channels in a Subband
        numberOfChannel = assocReq->supportedChannels.supportedChannels[i];
       PELOG2(limLog(pMac, LOG2, FL("Rcv AssocReq: chnl=%d, numOfChnl=%d \n"),
                             firstChannelNumber, numberOfChannel);)

        if (numberOfChannel > 1)
        {
            nextChannelNumber = firstChannelNumber;
            if(SIR_BAND_5_GHZ == limGetRFBand(firstChannelNumber))
            {
                for (j=1; j < numberOfChannel; j++)
                {
                    nextChannelNumber += SIR_11A_FREQUENCY_OFFSET;
                    pMlmAssocInd->supportedChannels.channelList[index] = nextChannelNumber;
                    index++;
                    if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
                    {
                        pMlmAssocInd->supportedChannels.numChnl = 0;
                        return;
                    }
                }
            }
            else if(SIR_BAND_2_4_GHZ == limGetRFBand(firstChannelNumber))
            {
                for (j=1; j < numberOfChannel; j++)
                {
                    nextChannelNumber += SIR_11B_FREQUENCY_OFFSET;
                    pMlmAssocInd->supportedChannels.channelList[index] = nextChannelNumber;
                    index++;
                    if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST)
                    {
                        pMlmAssocInd->supportedChannels.numChnl = 0;
                        return;
                    }
                }
            }
        }
    }

    pMlmAssocInd->supportedChannels.numChnl = (tANI_U8) index;
   PELOG2(limLog(pMac, LOG2,
        FL("Send AssocInd to WSM: spectrum ON, minPwr %d, maxPwr %d, numChnl %d\n"),
        pMlmAssocInd->powerCap.minTxPower,
        pMlmAssocInd->powerCap.maxTxPower,
        pMlmAssocInd->supportedChannels.numChnl);)
}

#if (WNI_POLARIS_FW_PRODUCT == AP)
/**
 * limProcessAssocReqFrame
 *
 *FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Re/Association Request frame reception.
 *
 *LOGIC:
 * This function processes received Re/Assoc Request frame and responds
 * with Re/Assoc Response.
 * Only AP or STA in IBSS mode will respond to Re/Association Request.
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  *pBd    - A pointer to Buffer descriptor + associated PDUs
 * @param  subType - Indicates whether it is Association Request (=0) or
 *                   Reassociation Request (=1) frame
 *
 * @return None
 */



void
limProcessAssocReqFrame(tpAniSirGlobal pMac, tANI_U32 *pBd, tANI_U8 subType,tpPESession psessionEntry)
{
    tANI_U8                 updateContext;
    tANI_U8                 *pBody;
    tANI_U16                aid, temp, statusCode;
    tANI_U16                i, j=0;
    tANI_U32                val;
    tANI_S32                framelen;
    tSirRetStatus           status;
    tpSirMacMgmtHdr         pHdr;
    tpLimMlmAssocInd        pMlmAssocInd;
    tpLimMlmReassocInd      pMlmReassocInd;
    struct tLimPreAuthNode         *pStaPreAuthContext;
    tAniAuthType            authType;
    tSirMacCapabilityInfo   localCapabilities;
    tpDphHashNode           pStaDs;
    tSirAssocReq            assoc;
#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tANI_U16                     numBlk=0;
#endif
    tANI_U32 phyMode;
    tHalBitVal qosMode;
    tHalBitVal wsmMode, wmeMode;
    tANI_U32   wpsApEnable=0, tmp;
    tANI_U32   wpsVersion;

    limGetPhyMode(pMac, &phyMode);
    limGetQosMode(pMac, &qosMode);

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    if ((numBlk=bufAvail(BUF_512)) < LIM_MIN_MEM_ASSOC)
    {
        // log error
        limLog(pMac, LOGE,
               FL("Memory is low to process ASSOC/REASSOC frame. Memory block left = %d\n"),numBlk);
               return;
    }
    else
    {
        PELOG1(limLog(pMac, LOG1,
               FL("Memory block left = %d while processing ASSOC/REASSOC frame.\n"),
               numBlk);)
    }
#endif

    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);
    framelen = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd);

   if (psessionEntry->limSystemRole == eLIM_STA_ROLE || psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE )
    {
        /// Should not have received Re/Assoc request frame on STA.

        // Log error
        limLog(pMac, LOGE, FL("received unexpected ASSOC REQ subType=%d for role=%d, radioId=%d from \n"),
                                            subType, psessionEntry->limSystemRole, pMac->sys.gSirRadioId);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);
        PELOG3(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3,
                      SIR_MAC_BD_TO_MPDUDATA(pBd), framelen);)

        return;
    }

    // Get pointer to Re/Association Request frame body
    pBody = SIR_MAC_BD_TO_MPDUDATA(pBd);

    if (limIsGroupAddr(pHdr->sa))
    {
        // Received Re/Assoc Req frame from a BC/MC address
        // Log error and ignore it
        if (subType == LIM_ASSOC)
            PELOG1(limLog(pMac, LOG1, FL("received Assoc frame from a BC/MC address\n"));)
        else
            PELOG1(limLog(pMac, LOG1, FL("received ReAssoc frame from a BC/MC address\n"));)
        PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

        return;
    }
    PELOG2(limLog(pMac, LOG2, FL("Received AssocReq Frame: "));
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG2, (tANI_U8 *) pBody, framelen);)
    
    if (subType == LIM_ASSOC)
        status = sirConvertAssocReqFrame2Struct(pMac, pBody, framelen, &assoc);
    else
        status = sirConvertReassocReqFrame2Struct(pMac, pBody, framelen, &assoc);

    if (status != eSIR_SUCCESS)
    {
        limLog(pMac, LOGW, FL("Parse error AssocRequest, length=%d from \n"),framelen);
        limPrintMacAddr(pMac, pHdr->sa, LOGW);

        limSendAssocRspMgmtFrame(pMac,
                                 eSIR_MAC_UNSPEC_FAILURE_STATUS,
                                 1,
                                 pHdr->sa,
                                 subType, 0,psessionEntry);

        goto error;
    }

    //
    // TODO: To be removed for newer revision of Titan
    // TITAN workaround for NULL data frames.
    // Block all non-Titan STA's in the interim
    //
    if( !assoc.propIEinfo.titanPresent &&  pMac->lim.gLimBlockNonTitanSta )
    {
      PELOG2(limLog( pMac, LOG2,
          FL("Sorry! Blocking all non-Titan STA's\n"));
      limPrintMacAddr(pMac, pHdr->sa, LOG2);)

      //
      // Currently responding with an error status of
      // eSIR_MAC_UNSPEC_FAILURE_STATUS. Not sure if
      // we need anything specific in the short term
      //
      limSendAssocRspMgmtFrame(pMac,
                               eSIR_MAC_UNSPEC_FAILURE_STATUS,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);
      goto error;
      
    }

    if (cfgGetCapabilityInfo(pMac, &temp,psessionEntry) != eSIR_SUCCESS)
    {
        /**
         * Could not get Capabilities
         * from CFG. Log error
         */
        limLog(pMac, LOGP, FL("could not retrieve Capabilities\n"));

        goto error;
    }
#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    *(tANI_U16*)&localCapabilities=(tANI_U16)(temp);
#else
    limCopyU16((tANI_U8 *) &localCapabilities, temp);
#endif

    if (limCompareCapabilities(pMac,
                               &assoc,
                               &localCapabilities,psessionEntry) == false)
    {
        /**
         * Capabilities of requesting STA does not match with
         * local capabilities. Respond with 'unsupported capabilities'
         * status code.
         */
        limSendAssocRspMgmtFrame(
                        pMac,
                        eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
                        1,
                        pHdr->sa,
                        subType, 0,psessionEntry);

        PELOG1(limLog(pMac, LOG1, FL("local caps 0x%x received 0x%x\n"),
               *((tANI_U16 *) &localCapabilities),
               *((tANI_U16 *) &assoc.capabilityInfo));)

        // Log error
        if (subType == LIM_ASSOC)
            PELOG1(limLog(pMac, LOG1,
               FL("received Assoc req with unsupported capabilities from\n"));)
        else
            PELOG1(limLog(pMac, LOG1,
               FL("received ReAssoc req with unsupported capabilities from\n"));)
        PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)
        goto error;
    }

    updateContext = false;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    // Check if multiple SSID feature is not enabled
    if (psessionEntry->pLimStartBssReq->ssId.length)
    {
        if (limCmpSSid(pMac, &(assoc.ssId),psessionEntry) == false)
        {
            /**
            * Received Re/Association Request with either
            * Broadcast SSID OR with SSID that does not
            * match with local one.
            * Respond with unspecified status code.
            */
            limSendAssocRspMgmtFrame(pMac,
                             eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             1,
                             pHdr->sa,
                             subType, 0,psessionEntry);

            // Log error
            if (subType == LIM_ASSOC)
                limLog(pMac, LOGW,
                   FL("received Assoc req with unmatched SSID from \n"));
            else
                limLog(pMac, LOGW,
                   FL("received ReAssoc req with unmatched SSID from \n"));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);

            goto error;
        }
    }
    else
        PELOG1(limLog(pMac, LOG1,
               FL("Suppressed SSID, App is going to check SSID\n"));)
#else
    if (limCmpSSid(pMac, &(assoc.ssId),psessionEntry) == false)
    {
        /**
         * Received Re/Association Request with either
         * Broadcast SSID OR with SSID that does not
         * match with local one.
         * Respond with unspecified status code.
         */
        limSendAssocRspMgmtFrame(pMac,
                             eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             1,
                             pHdr->sa,
                             subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
                   FL("received Assoc req with unmatched SSID from \n"));
        else
            limLog(pMac, LOGW,
                   FL("received ReAssoc req with unmatched SSID from \n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);

        goto error;
    }
#endif

    // Check if requested rates are among BSS basic rate set
    if (limCheckRxBasicRates(pMac, assoc.supportedRates,psessionEntry) == false)
    {
        /**
         * Requesting STA does not support ALL BSS basic
         * rates. Respond with 'basic rates not supported'
         * status code.
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported rates from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported rates from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);

        goto error;
    }

    /* Spectrum Management (11h) specific checks */
    if (localCapabilities.spectrumMgt)
    {
        tSirRetStatus status = eSIR_SUCCESS;

        /* If station is 11h capable, then it SHOULD send all mandatory 
                * IEs in assoc request frame. Let us verify that
                */
        if (assoc.capabilityInfo.spectrumMgt)
        {
            if (!((assoc.powerCapabilityPresent) && (assoc.supportedChannelsPresent)))
            {
                /* One or more required information elements are missing, log the peers error */
                if (!assoc.powerCapabilityPresent)
                {
                    if(subType == LIM_ASSOC)
                       PELOG1(limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in assoc request\n"));)
                    else
                       PELOG1(limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in Reassoc request\n"));)
                }
                if (!assoc.supportedChannelsPresent)
                {
                    if(subType == LIM_ASSOC)
                        PELOG1(limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in assoc request\n"));)
                    else
                        PELOG1(limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in Reassoc request\n"));)
                }
                PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)
            }
            else
            {
                /* Assoc request has mandatory fields */
                status = limIsDot11hPowerCapabilitiesInRange(pMac, &assoc,psessionEntry);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        PELOGW(limLog(pMac, LOGW, FL("LIM Info: Association MinTxPower(STA) > MaxTxPower(AP)\n"));)
                    else
                        PELOGW(limLog(pMac, LOGW, FL("LIM Info: Reassociation MinTxPower(STA) > MaxTxPower(AP)\n"));)
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                status = limIsDot11hSupportedChannelsValid(pMac, &assoc);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        PELOGW(limLog(pMac, LOGW, FL("LIM Info: Association wrong supported channels (STA)\n"));)
                    else
                        PELOGW(limLog(pMac, LOGW, FL("LIM Info: Rassociation wrong supported channels (STA)\n"));)
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                /* IEs are valid, use them if needed */
            }
        } //if(assoc.capabilityInfo.spectrumMgt)
        else
        {
            /* As per the capabiities, the spectrum management is not enabled on the station
                       * The AP may allow the associations to happen even if spectrum management is not
                       * allowed, if the transmit power of station is below the regulatory maximum
                       */

            /* TODO: presently, this is not handled. In the current implemetation, the AP would
                       * allow the station to associate even if it doesn't support spectrum management.
                       */
        }
    }// end of spectrum management related processing

    if ( (assoc.HTCaps.present) && (limCheckMCSSet(pMac, assoc.HTCaps.supportedMCSSet) == false))
    {
        /**
         * Requesting STA does not support ALL BSS MCS basic Rate set rates.
         * Spec does not define any status code for this scenario.
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported MCS Rate Set from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported MCS Rate Set from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);

        goto error;
    }



    //if (pMac->dph.gDphPhyMode == WNI_CFG_PHY_MODE_11G)
    if (phyMode == WNI_CFG_PHY_MODE_11G)
    {
        if (wlan_cfgGetInt(pMac, WNI_CFG_11G_ONLY_POLICY, &val) !=
                                                           eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve 11g-only flag\n"));
            goto error;
        }

        if (!assoc.extendedRatesPresent && val)
        {
            /**
             * Received Re/Association Request from
             * 11b STA when 11g only policy option
             * is set.
             * Reject with unspecified status code.
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW,
                   FL("Rejecting Re/Assoc req from 11b STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);

#ifdef WLAN_DEBUG    
            pMac->lim.gLim11bStaAssocRejectCount++;
#endif
            goto error;
        }
    }

#ifdef WMM_APSD
    // Save the QOS info element in assoc request..
    limGetWmeMode(pMac, &wmeMode);
    if (wmeMode == eHAL_SET)
    {
        tpSirMacQosInfoStation  qInfo;

        qInfo = &assoc.qosCapability.qosInfo;

        if ((pMac->lim.gUapsdEnable == 0) && (qInfo->acbe_uapsd || qInfo->acbk_uapsd || qInfo->acvo_uapsd || qInfo->acvi_uapsd))
        {

            /**
             * Received Re/Association Request from
             * 11b STA when 11g only policy option
             * is set.
             * Reject with unspecified status code.
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_WME_REFUSED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW,
                   FL("Rejecting Re/Assoc req from STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            PELOGE(limLog(pMac, LOGE, FL("APSD not enabled, qosInfo - 0x%x\n"), *qInfo);)
            goto error;
        }
    }
#endif

    // Check for 802.11n HT caps compatibility; are HT Capabilities
    // turned on in lim?
    if (psessionEntry->htCapabality)
    {
        // There are; are they turned on in the STA?
        if ( assoc.HTCaps.present )
        {
            // The station *does* support 802.11n HT capability...

            limLog( pMac, LOG1, FL( "AdvCodingCap:%d ChaWidthSet:%d "
                                    "PowerSave:%d greenField:%d "
                                    "shortGI20:%d shortGI40:%d\n"
                                    "txSTBC:%d rxSTBC:%d delayBA:%d"
                                    "maxAMSDUsize:%d DSSS/CCK:%d "
                                    "PSMP:%d stbcCntl:%d lsigTXProt:%d\n"),
                    assoc.HTCaps.advCodingCap,
                    assoc.HTCaps.supportedChannelWidthSet,
                    assoc.HTCaps.mimoPowerSave,
                    assoc.HTCaps.greenField,
                    assoc.HTCaps.shortGI20MHz,
                    assoc.HTCaps.shortGI40MHz,
                    assoc.HTCaps.txSTBC,
                    assoc.HTCaps.rxSTBC,
                    assoc.HTCaps.delayedBA,
                    assoc.HTCaps.maximalAMSDUsize,
                    assoc.HTCaps.dsssCckMode40MHz,
                    assoc.HTCaps.psmp,
                    assoc.HTCaps.stbcControlFrame,
                    assoc.HTCaps.lsigTXOPProtection );

                // Make sure the STA's caps are compatible with our own:
                //11.15.2 Support of DSSS/CCK in 40 MHz
                //the AP shall refuse association requests from an HT STA that has the DSSS/CCK 
                //Mode in 40 MHz subfield set to 1;

            if ( !pMac->lim.gHTDsssCckRate40MHzSupport && assoc.HTCaps.dsssCckMode40MHz )
            {
                statusCode = eSIR_MAC_DSSS_CCK_RATE_NOT_SUPPORT_STATUS;
                limLog( pMac, LOGW, FL( "limProcessAssocReqFrame: "
                                        "AP DSSS/CCK is disabled; "
                                        "STA rejected.\n" ) );
                // Reject association
                limSendAssocRspMgmtFrame( pMac, statusCode, 1, pHdr->sa,
                                          subType, 0,psessionEntry);

                goto error;
            }
        }
    } // End if on HT caps turned on in lim.

    /**
     * Extract 'associated' context for STA, if any.
     * This is maintained by DPH and created by LIM.
     */
    pStaDs = dphLookupHashEntry(pMac, pHdr->sa, &aid, &psessionEntry->dph.dphHashTable);

    /// Extract pre-auth context for the STA, if any.
    pStaPreAuthContext = limSearchPreAuthList(pMac, pHdr->sa);

    if (pStaDs == NULL)
    {
        /// Requesting STA is not currently associated

        if (pMac->lim.gLimNumOfCurrentSTAs == pMac->lim.maxStation)
        {
            /**
             * Maximum number of STAs that AP can handle reached.
             * Send Association response to peer MAC entity
             */
            limRejectAssociation(pMac, pHdr->sa,
                                 subType, false,
                                 (tAniAuthType) 0, 0,
                                 false,
                                 (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                                 psessionEntry);

            goto error;
        }

        /// Check if STA is pre-authenticated.
        if ((pStaPreAuthContext == NULL) ||
            (pStaPreAuthContext &&
             (pStaPreAuthContext->mlmState !=
                              eLIM_MLM_AUTHENTICATED_STATE)))
        {
            /**
             * STA is not pre-authenticated yet requesting
             * Re/Association before Authentication.
             * OR STA is in the process of getting authenticated
             * and sent Re/Association request.
             * Send Deauthentication frame with 'prior
             * authentication required' reason code.
             */
            limSendDeauthMgmtFrame(
                     pMac,
                     eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON, //=9
                     pHdr->sa,psessionEntry);

            // Log error
            if (subType == LIM_ASSOC)
                PELOG1(limLog(pMac, LOG1,
                       FL("received Assoc req from STA that does not have pre-auth context, MAC addr is: \n"));)
            else
                PELOG1(limLog(pMac, LOG1,
                       FL("received ReAssoc req from STA that does not have pre-auth context, MAC addr is: \n"));)
            PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

            goto error;
        }

        /// Delete 'pre-auth' context of STA
        authType = pStaPreAuthContext->authType;
        limDeletePreAuthNode(pMac, pHdr->sa);

        // All is well. Assign AID (after else part)

    } // if (pStaDs == NULL)
    else
    {
        // STA context does exist for this STA

        if (pStaDs->mlmStaContext.mlmState !=
                         eLIM_MLM_LINK_ESTABLISHED_STATE)
        {
            /**
             * Requesting STA is in some 'transient' state?
             * Ignore the Re/Assoc Req frame by incrementing
             * debug counter & logging error.
             */
            if (subType == LIM_ASSOC)
            {
#ifdef WLAN_DEBUG                
                pMac->lim.gLimNumAssocReqDropInvldState++;
#endif
                PELOG1(limLog(pMac, LOG1, FL("received Assoc req in state %X from "),
                       pStaDs->mlmStaContext.mlmState);)
            }
            else
            {
#ifdef WLAN_DEBUG                
                pMac->lim.gLimNumReassocReqDropInvldState++;
#endif
                PELOG1(limLog(pMac, LOG1, FL("received ReAssoc req in state %X from "),
                       pStaDs->mlmStaContext.mlmState);)
            }
            PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);
            limPrintMlmState(pMac, LOG1,
                             (tLimMlmStates) pStaDs->mlmStaContext.mlmState);)
            goto error;
        } // if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE)

        /**
         * STA sent Re/Association Request frame while already in
         * 'associated' state. Update STA capabilities and
         * send Association response frame with same AID
         */

        pStaDs->mlmStaContext.capabilityInfo = assoc.capabilityInfo;

        if (pStaPreAuthContext &&
            (pStaPreAuthContext->mlmState ==
                                       eLIM_MLM_AUTHENTICATED_STATE))
        {
            /// STA has triggered pre-auth again
            authType = pStaPreAuthContext->authType;
            limDeletePreAuthNode(pMac, pHdr->sa);
        }
        else
            authType = pStaDs->mlmStaContext.authType;

        updateContext = true;

        if (dphInitStaState(pMac, pHdr->sa, aid, true, &psessionEntry->dph.dphHashTable) == NULL)
        {
            PELOGE(limLog(pMac, LOGE, FL("could not Init STAid=%d\n"), aid);)
                    goto  error;
        }

        /*
         * Clear the power save state for this STA, in case this
         * was in power save before the (re)association
         */
        pmmUpdatePMMode(pMac, aid, 0);

        goto sendIndToSme;
    } // end if (lookup for STA in perStaDs fails)



    // check if sta is allowed per QoS AC rules
    //if (pMac->dph.gDphQosEnabled || pMac->dph.gDphWmeEnabled)
    limGetWmeMode(pMac, &wmeMode);
    if ((qosMode == eHAL_SET) || (wmeMode == eHAL_SET))
    {
        // for a qsta, check if the requested Traffic spec
        // is admissible
        // for a non-qsta check if the sta can be admitted
        if (assoc.addtsPresent)
        {
            tANI_U8 tspecIdx = 0; //index in the sch tspec table.
            if (limAdmitControlAddTS(pMac, pHdr->sa, &assoc.addtsReq,
                                     &assoc.qosCapability, 0, false, NULL, &tspecIdx, psessionEntry)
                != eSIR_SUCCESS)
            {
                PELOGW(limLog(pMac, LOGW, FL("AdmitControl: TSPEC rejected\n"));)
                limSendAssocRspMgmtFrame(
                               pMac,
                               eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                               1,
                               pHdr->sa,
                               subType, 0, psessionEntry);
#ifdef WLAN_DEBUG                    
                pMac->lim.gLimNumAssocReqDropACRejectTS++;
#endif
                goto error;
            }
        }
        else if (limAdmitControlAddSta(pMac, pHdr->sa, false)
                                               != eSIR_SUCCESS)
        {
            PELOGW(limLog(pMac, LOGW, FL("AdmitControl: Sta rejected\n"));)
            limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                    1,
                    pHdr->sa,
                    subType, 0, psessionEntry);
#ifdef WLAN_DEBUG                
            pMac->lim.gLimNumAssocReqDropACRejectSta++;
#endif
            goto error;
        }

        // else all ok
        PELOG1(limLog(pMac, LOG1, FL("AdmitControl: Sta OK!\n"));)
    }

    /**
     * STA is Associated !
     */
    if (subType == LIM_ASSOC)
        PELOG1(limLog(pMac, LOG1, FL("received Assoc req successful from "));)
    else
        PELOG1(limLog(pMac, LOG1, FL("received ReAssoc req successful from "));)
    PELOG1(limPrintMacAddr(pMac, pHdr->sa, LOG1);)

    /**
     * Assign unused/least recently used AID from perStaDs.
     * This will 12-bit STAid used by MAC HW.
     * NOTE: limAssignAID() assigns AID values ranging between 1 - 255
     *       so make MSB of AID 0xC0.
     */

    aid = limAssignAID(pMac);

    if (!aid)
    {
        // Could not assign AID
        // Reject association
        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType,
                             aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             psessionEntry);

        goto error;
    }

    /**
     * Add an entry to hash table maintained by DPH module
     */

    pStaDs = dphAddHashEntry(pMac, pHdr->sa, aid, &psessionEntry->dph.dphHashTable);
    if (pStaDs == NULL)
    {
        // Could not add hash table entry at DPH
        limLog(pMac, LOGE,
           FL("could not add hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

        // Release AID
        limReleaseAID(pMac, aid);

        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             psessionEntry);

        goto error;
    }


sendIndToSme:

    pStaDs->mlmStaContext.htCapability = assoc.HTCaps.present;
    pStaDs->qos.addtsPresent = (assoc.addtsPresent==0) ? false : true;
    pStaDs->qos.addts        = assoc.addtsReq;
    pStaDs->qos.capability   = assoc.qosCapability;
    pStaDs->versionPresent = 0;
    pStaDs->propCapability = 0;

    pStaDs->valid                  = 0;
    pStaDs->mlmStaContext.authType = authType;
    pStaDs->staType = STA_ENTRY_PEER;

    //TODO: If listen interval is more than certain limit, reject the association.
    //Need to check customer requirements and then implement.
    pStaDs->mlmStaContext.listenInterval = assoc.listenInterval;
    pStaDs->mlmStaContext.capabilityInfo = assoc.capabilityInfo;

    /* The following count will be used to knock-off the station if it doesn't
     * come back to receive the buffered data. The AP will wait for numTimSent number
     * of beacons after sending TIM information for the station, before assuming that 
     * the station is no more associated and disassociates it
     */

    /** timWaitCount is used by PMM for monitoring the STA's in PS for LINK*/
    pStaDs->timWaitCount = GET_TIM_WAIT_COUNT(assoc.listenInterval);
    
    /** Initialise the Current successful MPDU's tranfered to this STA count as 0 */
    pStaDs->curTxMpduCnt = 0;

    if(IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
      (assoc.HTCaps.present))
    {
        pStaDs->htGreenfield = assoc.HTCaps.greenField;
        pStaDs->htAMpduDensity = assoc.HTCaps.mpduDensity;
        pStaDs->htDsssCckRate40MHzSupport = assoc.HTCaps.dsssCckMode40MHz;
        pStaDs->htLsigTXOPProtection = assoc.HTCaps.lsigTXOPProtection;
        pStaDs->htMaxAmsduLength = assoc.HTCaps.maximalAMSDUsize;
        pStaDs->htMaxRxAMpduFactor = assoc.HTCaps.maxRxAMPDUFactor;
        pStaDs->htMIMOPSState = assoc.HTCaps.mimoPowerSave;
        pStaDs->htShortGI20Mhz = assoc.HTCaps.shortGI20MHz;
        pStaDs->htShortGI40Mhz = assoc.HTCaps.shortGI40MHz;
        pStaDs->htSupportedChannelWidthSet = assoc.HTCaps.supportedChannelWidthSet;
        pStaDs->baPolicyFlag = 0xFF;
    }



    if (limPopulateMatchingRateSet(pMac,
                                   pStaDs,
                                   &assoc.supportedRates,
                                   &assoc.extendedRates,
                                   assoc.HTCaps.supportedMCSSet,
                                   &assoc.propIEinfo.propRates,psessionEntry) != eSIR_SUCCESS)
    {
        // Could not update hash table entry at DPH with rateset
        limLog(pMac, LOGE,
           FL("could not update hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

                // Release AID
        limReleaseAID(pMac, aid);


        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, true,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pStaDs->mlmStaContext.propRateSet,
                  (tANI_U8 *) &(assoc.propIEinfo.propRates),
                  assoc.propIEinfo.propRates.numPropRates + 1);

    /// Add STA context at MAC HW (BMU, RHP & TFP)

    pStaDs->qosMode    = eANI_BOOLEAN_FALSE;
    pStaDs->lleEnabled = eANI_BOOLEAN_FALSE;
    if (assoc.capabilityInfo.qos && (qosMode == eHAL_SET))
    {
        pStaDs->lleEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode    = eANI_BOOLEAN_TRUE;
     }

    pStaDs->wmeEnabled = eANI_BOOLEAN_FALSE;
    pStaDs->wsmEnabled = eANI_BOOLEAN_FALSE;
    limGetWmeMode(pMac, &wmeMode);
    //if ((! pStaDs->lleEnabled) && assoc.wmeInfoPresent && pMac->dph.gDphWmeEnabled)
    if ((! pStaDs->lleEnabled) && assoc.wmeInfoPresent && (wmeMode == eHAL_SET))
    {
        pStaDs->wmeEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode = eANI_BOOLEAN_TRUE;
    limGetWsmMode(pMac, &wsmMode);
        /* WMM_APSD - TODO - WMM_SA related processing should be separate; WMM_SA and WMM_APSD
         can coexist */
        //if (assoc.wsmCapablePresent && pMac->dph.gDphWsmEnabled)
        if (assoc.wsmCapablePresent && (wsmMode == eHAL_SET))
            pStaDs->wsmEnabled = eANI_BOOLEAN_TRUE;

    }

        //Update pre-filled buffer descriptor at hal
    //dphSetStaQosMode(pMac, pStaDs->assocId);
    //Already done ABOVE...
    //halSetStaQosMode(pMac, pStaDs->assocId);

    if (!updateContext)
    {
        // Wait until Re/AssocCnf from WSM to add STA
        pStaDs->mlmStaContext.updateContext = 0;
    }
    else
    {
        // Wait until Re/AssocCnf from WSM to update STA
        pStaDs->mlmStaContext.updateContext = 1;
    }

    // Re/Assoc Response frame to requesting STA
    pStaDs->mlmStaContext.subType = subType;

    /// Send Re/Association indication to SME

    if (subType == LIM_ASSOC)
    {
        temp  = sizeof(tLimMlmAssocInd);

        temp += assoc.propIEinfo.numBss * sizeof(tSirNeighborBssInfo);

        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAssocInd, temp))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for pMlmAssocInd\n"));

            // Release AID
            limReleaseAID(pMac, aid);

            goto error;
        }

        palCopyMemory( pMac->hHdd, (tANI_U8 *) pMlmAssocInd->peerMacAddr,
                      (tANI_U8 *) pHdr->sa,
                      sizeof(tSirMacAddr));
        pMlmAssocInd->aid      = aid;
        pMlmAssocInd->authType = pStaDs->mlmStaContext.authType;		
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmAssocInd->ssId,
                      (tANI_U8 *) &(assoc.ssId), assoc.ssId.length + 1);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        pMlmAssocInd->seqNum       = (tANI_U16) (
                         ((tANI_U16) (pHdr->seqControl.seqNumHi << 4)) ||
                         pHdr->seqControl.seqNumLo);
        pMlmAssocInd->wniIndicator = (tAniBool) assoc.propIEinfo.aniIndicator;
        pMlmAssocInd->bpIndicator  = (tAniBool) assoc.propIEinfo.bpIndicator;
        pMlmAssocInd->bpType       = (tSirBpIndicatorType)
                                     assoc.propIEinfo.bpType;
        if (assoc.extendedRatesPresent)
        {
            pMlmAssocInd->nwType = eSIR_11G_NW_TYPE;
	     pStaDs->erpEnabled = eHAL_SET;
        }
        else
        {

            //if (pMac->dph.gDphPhyMode == WNI_CFG_PHY_MODE_11A)
            if (phyMode == WNI_CFG_PHY_MODE_11A)

                pMlmAssocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmAssocInd->nwType = eSIR_11B_NW_TYPE;
                pStaDs->erpEnabled = eHAL_CLEAR;

            }
        }
        pMlmAssocInd->assocType    = (tSirAssocType)
                                     assoc.propIEinfo.assocType;
        pMlmAssocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmAssocInd->load.channelUtilization =
                      (pMac->lim.gpLimMeasData) ?
                      pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmAssocInd->numBss       = (tANI_U32) assoc.propIEinfo.numBss;
        if (assoc.propIEinfo.numBss)
            palCopyMemory( pMac->hHdd, (tANI_U8 *) pMlmAssocInd->neighborList,
                          (tANI_U8 *) assoc.propIEinfo.pBssList,
                          (sizeof(tSirNeighborBssInfo) *
                           assoc.propIEinfo.numBss));
#else
        if (assoc.extendedRatesPresent)
        {
	     pStaDs->erpEnabled = eHAL_SET;
        }
        else
        {
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                ;
            else
            {
                pStaDs->erpEnabled = eHAL_CLEAR;
            }
        }
#endif
        if (assoc.capabilityInfo.shortPreamble)
        {
            pStaDs->shortPreambleEnabled = eHAL_SET;
        }
        else
        {
            pStaDs->shortPreambleEnabled = eHAL_CLEAR;
        }

        if (assoc.capabilityInfo.shortSlotTime)
        {
            pStaDs->shortSlotTimeEnabled = eHAL_SET;
        }
        else
        {
            pStaDs->shortSlotTimeEnabled = eHAL_CLEAR;
        }

        if (assoc.propIEinfo.aniIndicator)
            pStaDs->aniPeer = 1;



        pMlmAssocInd->capabilityInfo = assoc.capabilityInfo;

        pMlmAssocInd->rsnIE.length = 0;
        if (assoc.rsnPresent)
        {
           PELOG2(limLog(pMac, LOG2,
                   FL("Received RSN IE length in Assoc Req is %d\n"),
                   assoc.rsn.length);)

            pMlmAssocInd->rsnIE.length = 2 + assoc.rsn.length;
            pMlmAssocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[1] = assoc.rsn.length;
            palCopyMemory( pMac->hHdd, &pMlmAssocInd->rsnIE.rsnIEdata[2],
                          assoc.rsn.info,
                          assoc.rsn.length);
        }

        //FIXME: we need to have the cb information seprated between HT and Titan later. 
        if(assoc.HTCaps.present)
            limGetHtCbAdminState(pMac, assoc.HTCaps, &pMlmAssocInd->titanHtCaps);

        // 802.11h support
        if (assoc.powerCapabilityPresent && assoc.supportedChannelsPresent)
        {
            pMlmAssocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmAssocInd->powerCap.minTxPower = assoc.powerCapability.minTxPower;
            pMlmAssocInd->powerCap.maxTxPower = assoc.powerCapability.maxTxPower;

            limConvertSupportedChannels(pMac, pMlmAssocInd, &assoc);
        }
        else
            pMlmAssocInd->spectrumMgtIndicator = eSIR_FALSE;


        if (assoc.wpaPresent)
        {
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 1] = assoc.wpa.length;
            palCopyMemory( pMac->hHdd,
               &pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 2],
               assoc.wpa.info,
               assoc.wpa.length);
            pMlmAssocInd->rsnIE.length += 2 + assoc.wpa.length;
        }

        /* Initialize wscInfo. */
        vos_mem_set(&pMlmAssocInd->wscInfo, sizeof(tSirMacWscInfo), 0);
             
        if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
             limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
        
        wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

        if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_VERSION, &wpsVersion) != eSIR_SUCCESS)
             limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_VERSION );

        if (assoc.wscInfo.present && wpsApEnable)
        {
            /* Compare wps version of the received assoc req with AP's wps version */
            /* If not compatible, reject; else, copy into wscinfo into sLimMlmAssocInd */
            if (assoc.wscInfo.wpsVersion != wpsVersion)
            {
              /* Reject Association */
              statusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
              limSendAssocRspMgmtFrame( pMac, statusCode, 1, pHdr->sa, subType, 0 ,psessionEntry);
            }
            else
            {
                /* Copy sSirAssocReq.sSirMacWscInfo to sLimMlmAssocInd.sSirMacWscInfo */
                pMlmAssocInd->wscInfo.present           = 1;
                pMlmAssocInd->wscInfo.wpsVersion        = assoc.wscInfo.wpsVersion;
                pMlmAssocInd->wscInfo.wpsRequestType    = assoc.wscInfo.wpsRequestType;
        }
        }

        limPostSmeMessage(pMac,
                          LIM_MLM_ASSOC_IND,
                          (tANI_U32 *) pMlmAssocInd);

        palFreeMemory( pMac->hHdd, pMlmAssocInd);
    }
    else
    {
        temp  = sizeof(tLimMlmReassocInd);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        temp += assoc.propIEinfo.numBss * sizeof(tSirNeighborBssInfo);
#endif

        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmReassocInd, temp))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for pMlmReassocInd\n"));

            // Release AID
            limReleaseAID(pMac, aid);

            goto error;
        }

        palCopyMemory( pMac->hHdd, (tANI_U8 *) pMlmReassocInd->peerMacAddr,
                      (tANI_U8 *) pHdr->sa,
                      sizeof(tSirMacAddr));
        palCopyMemory( pMac->hHdd, (tANI_U8 *) pMlmReassocInd->currentApAddr,
                      (tANI_U8 *) &assoc.currentApAddr,
                      sizeof(tSirMacAddr));
        pMlmReassocInd->aid      = aid;
        pMlmReassocInd->authType = authType;
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmReassocInd->ssId,
                      (tANI_U8 *) &(assoc.ssId), assoc.ssId.length + 1);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        pMlmReassocInd->seqNum       = (tANI_U16) (
                         ((tANI_U16) (pHdr->seqControl.seqNumHi << 4)) ||
                         pHdr->seqControl.seqNumLo);
        pMlmReassocInd->wniIndicator = (tAniBool)
                                       assoc.propIEinfo.aniIndicator;
        pMlmReassocInd->bpIndicator  = (tAniBool)
                                       assoc.propIEinfo.bpIndicator;
        pMlmReassocInd->bpType       = (tSirBpIndicatorType)
                                       assoc.propIEinfo.bpType;
        if (assoc.extendedRatesPresent)
        {
            pMlmReassocInd->nwType = eSIR_11G_NW_TYPE;
	     pStaDs->erpEnabled = eHAL_SET;
        }
        else
        {
            //if (pMac->dph.gDphPhyMode == WNI_CFG_PHY_MODE_11A)
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                pMlmReassocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmReassocInd->nwType = eSIR_11B_NW_TYPE;
		pStaDs->erpEnabled = eHAL_CLEAR;
            }
        }

        pMlmReassocInd->reassocType  = (tSirAssocType)
                                       assoc.propIEinfo.assocType;
        pMlmReassocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmReassocInd->load.channelUtilization =
                      (pMac->lim.gpLimMeasData) ?
                      pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmReassocInd->numBss       = (tANI_U32) assoc.propIEinfo.numBss;
        if (assoc.propIEinfo.numBss)
            palCopyMemory( pMac->hHdd, (tANI_U8 *) pMlmReassocInd->neighborList,
                          (tANI_U8 *) assoc.propIEinfo.pBssList,
                          (sizeof(tSirNeighborBssInfo) *
                           assoc.propIEinfo.numBss));

#else
        if (assoc.extendedRatesPresent)
        {
            pStaDs->erpEnabled = eHAL_SET;
        }
        else
        {
            //if (pMac->dph.gDphPhyMode == WNI_CFG_PHY_MODE_11A)
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                ;
            else
            {
                pStaDs->erpEnabled = eHAL_CLEAR;
            }
        }
#endif
        if (assoc.capabilityInfo.shortPreamble)
            pStaDs->shortPreambleEnabled = eHAL_SET;
        else
            pStaDs->shortPreambleEnabled = eHAL_CLEAR;
        if (assoc.propIEinfo.aniIndicator)
            pStaDs->aniPeer = 1;

        pMlmReassocInd->capabilityInfo = assoc.capabilityInfo;

        pMlmReassocInd->rsnIE.length = 0;
        if (assoc.rsnPresent)
        {
           PELOG2(limLog(pMac, LOG2,
                   FL("Received RSN IE length in Assoc Req is %d\n"),
                   assoc.rsn.length);)

            pMlmReassocInd->rsnIE.length = 2 + assoc.rsn.length;
            pMlmReassocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[1] = assoc.rsn.length;
            palCopyMemory( pMac->hHdd, &pMlmReassocInd->rsnIE.rsnIEdata[2],
                          assoc.rsn.info,
                          assoc.rsn.length);
        }

        if(assoc.HTCaps.present)
              limGetHtCbAdminState(pMac, assoc.HTCaps,  &pMlmReassocInd->titanHtCaps );

        // 802.11h support
        if (assoc.powerCapabilityPresent && assoc.supportedChannelsPresent)
        {
            pMlmReassocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmReassocInd->powerCap.minTxPower = assoc.powerCapability.minTxPower;
            pMlmReassocInd->powerCap.maxTxPower = assoc.powerCapability.maxTxPower;
            pMlmReassocInd->supportedChannels.numChnl =
                                  (tANI_U8)(assoc.supportedChannels.length / 2);

            PELOG1(limLog(pMac, LOG1,
                FL("Sending Reassoc Ind: spectrum ON, minPwr %d, maxPwr %d, numChnl %d\n"),
                pMlmReassocInd->powerCap.minTxPower,
                pMlmReassocInd->powerCap.maxTxPower,
                pMlmReassocInd->supportedChannels.numChnl);)

            for(i=0; i < pMlmReassocInd->supportedChannels.numChnl; i++)
            {
                pMlmReassocInd->supportedChannels.channelList[i] =
                            assoc.supportedChannels.supportedChannels[j];

                PELOG1(limLog(pMac, LOG1, FL("Sending ReassocInd: chn[%d] = %d \n"),
                    i, pMlmReassocInd->supportedChannels.channelList[i]);)
                j+=2;
            }
        }
        else
            pMlmReassocInd->spectrumMgtIndicator = eSIR_FALSE;


        if (assoc.wpaPresent)
        {
            PELOG1(limLog(pMac, LOG2,
                   FL("Received WPA IE length in Assoc Req is %d\n"),
                   assoc.wpa.length);)

            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 1] = assoc.wpa.length;
            palCopyMemory( pMac->hHdd,
               &pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 2],
               assoc.wpa.info,
               assoc.wpa.length);
            pMlmReassocInd->rsnIE.length += 2 + assoc.wpa.length;
        }


        /* Initialize wscInfo. */
       vos_mem_set(&pMlmReassocInd->wscInfo, sizeof(tSirMacWscInfo), 0);
       if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
            limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
       
       wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

       if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_VERSION, &wpsVersion) != eSIR_SUCCESS)
            limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_VERSION );

        if (assoc.wscInfo.present && wpsApEnable)
        {
            /* Compare wps version of the received assoc req with AP's wps version */
            /* If not compatible, reject; else, copy into wscinfo into sLimMlmAssocInd */
                    if (assoc.wscInfo.wpsVersion != wpsVersion)
            {

              /* Reject Reassociation */
              statusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
              limSendAssocRspMgmtFrame( pMac, statusCode, 1, pHdr->sa, subType, 0 ,psessionEntry);
            }
            else
            {
                /* Copy sSirReassocReq.sSirMacWscInfo to sLimMlmReassocInd.sSirMacWscInfo */
                pMlmReassocInd->wscInfo.present         = 1;
                pMlmReassocInd->wscInfo.wpsVersion      = assoc.wscInfo.wpsVersion;
                pMlmReassocInd->wscInfo.wpsRequestType  = assoc.wscInfo.wpsRequestType;

            }
        }

        limPostSmeMessage(pMac,
                          LIM_MLM_REASSOC_IND,
                          (tANI_U32 *) pMlmReassocInd);

        palFreeMemory( pMac->hHdd, pMlmReassocInd);
    }

error:
    if ((assoc.propIEinfo.pBssList) && (assoc.propIEinfo.neighborListPresent))
        palFreeMemory( pMac->hHdd, assoc.propIEinfo.pBssList);

    return;

} /*** end limProcessAssocReqFrame() ***/
#endif



/**---------------------------------------------------------------
\fn     limBtAmpProcessAssocReqFrame
\brief  This function is called by limProcessMessageQueue()
\       upon Re/Association Request frame reception in 
\       BTAMP link.
\
\param pMac
\param *pBd    - A pointer to Buffer descriptor + associated PDUs
\param subType - Indicates whether it is Association Request(=0) 
\                or Reassociation Request(=1) frame
\return None
------------------------------------------------------------------*/
void
limBtAmpProcessAssocReqFrame(tpAniSirGlobal pMac, tANI_U32 *pBd, tANI_U8 subType,tpPESession psessionEntry)
{
    tANI_U8                 updateContext;
    tANI_U8                 *pBody;
    tANI_U16                aid, temp;
    tANI_U32                val;
    tANI_S32                framelen;
    tSirRetStatus           status;
    tpSirMacMgmtHdr         pHdr;
    struct tLimPreAuthNode  *pStaPreAuthContext;
    tAniAuthType            authType;
    tSirMacCapabilityInfo   localCapabilities;
    tpDphHashNode           pStaDs = NULL;
    tpSirAssocReq           pAssocReq;
#ifdef WLAN_SOFTAP_FEATURE
    tLimMlmStates           mlmPrevState;
    tDot11fIERSN            Dot11fIERSN;
    tDot11fIEWPA            Dot11fIEWPA;
#endif

//#if (WNI_POLARIS_FW_OS == SIR_RTAI)
//    tANI_U16                numBlk=0;
//#endif
    tANI_U32 phyMode;
    tHalBitVal qosMode;
    tHalBitVal wsmMode, wmeMode;

    limGetPhyMode(pMac, &phyMode);
    limGetQosMode(pMac, &qosMode);

//#if (WNI_POLARIS_FW_OS == SIR_RTAI)
//    if ((numBlk=bufAvail(BUF_512)) < LIM_MIN_MEM_ASSOC)
//    {
//        limLog(pMac, LOGE,
//               FL("Memory is low to process ASSOC/REASSOC frame. Memory block left = %d\n"),numBlk);
//        return;
//    }
//    else
//    {
//        limLog(pMac, LOG1, FL("Memory block left = %d while processing ASSOC/REASSOC frame.\n"),numBlk);
//    }
//#endif

    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);
    framelen = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd);

   if (psessionEntry->limSystemRole == eLIM_STA_ROLE || psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE )
   {
        limLog(pMac, LOGE, FL("received unexpected ASSOC REQ subType=%d for role=%d, radioId=%d from \n"),
                                            subType, psessionEntry->limSystemRole, pMac->sys.gSirRadioId);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);
        sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG3,
        SIR_MAC_BD_TO_MPDUDATA(pBd), framelen);
        return;
    }

    // Get pointer to Re/Association Request frame body
    pBody = SIR_MAC_BD_TO_MPDUDATA(pBd);

    if (limIsGroupAddr(pHdr->sa))
    {
        // Received Re/Assoc Req frame from a BC/MC address
        // Log error and ignore it
        if (subType == LIM_ASSOC)
            limLog(pMac, LOG1, FL("received Assoc frame from a BC/MC address\n"));
        else
            limLog(pMac, LOG1, FL("received ReAssoc frame from a BC/MC address\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOG1);
        return;
    }
    limLog(pMac, LOG2, FL("Received AssocReq Frame: "));
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG2, (tANI_U8 *) pBody, framelen);

#ifdef WLAN_SOFTAP_FEATURE
    // If TKIP counter measures enabled send Assoc Rsp frame to station with eSIR_MAC_MIC_FAILURE_REASON
    if ((psessionEntry->bTkipCntrMeasActive) && (psessionEntry->limSystemRole == eLIM_AP_ROLE))
    {
        limSendAssocRspMgmtFrame(pMac,
                                    eSIR_MAC_MIC_FAILURE_REASON,
                                    1,
                                    pHdr->sa,
                                    subType, 0, psessionEntry);
        return;
    }
#endif

    // BTAMP: Allocate memory for the Assoc Request frame
    if ( palAllocateMemory(pMac->hHdd, (void **)&pAssocReq, sizeof(*pAssocReq)) != eHAL_STATUS_SUCCESS) 
    {
        limLog(pMac, LOGP, FL("PAL Allocate Memory failed in AssocReq\n"));
        return;
    }
	palZeroMemory( pMac->hHdd, (void *)pAssocReq , sizeof(*pAssocReq));
   
    // Parse Assoc Request frame
	if (subType == LIM_ASSOC)
        status = sirConvertAssocReqFrame2Struct(pMac, pBody, framelen, pAssocReq);
    else
        status = sirConvertReassocReqFrame2Struct(pMac, pBody, framelen, pAssocReq);

    if (status != eSIR_SUCCESS)
    {
        limLog(pMac, LOGW, FL("Parse error AssocRequest, length=%d from \n"),framelen);
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        limSendAssocRspMgmtFrame(pMac, eSIR_MAC_UNSPEC_FAILURE_STATUS, 1, pHdr->sa, subType, 0, psessionEntry);
        goto error;
    }


    if (cfgGetCapabilityInfo(pMac, &temp,psessionEntry) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("could not retrieve Capabilities\n"));
        goto error;
    }
#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    *(tANI_U16*)&localCapabilities=(tANI_U16)(temp);
#else
    limCopyU16((tANI_U8 *) &localCapabilities, temp);
#endif

    if (limCompareCapabilities(pMac,
                               pAssocReq,
                               &localCapabilities,psessionEntry) == false)
    {
        /**
         * Capabilities of requesting STA does not match with
         * local capabilities. Respond with 'unsupported capabilities'
         * status code.
         */
        limSendAssocRspMgmtFrame(
                        pMac,
                        eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
                        1,
                        pHdr->sa,
                        subType, 0,psessionEntry);

        limLog(pMac, LOG1, FL("local caps 0x%x received 0x%x\n"), localCapabilities, pAssocReq->capabilityInfo);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOG1,
               FL("received Assoc req with unsupported capabilities from\n"));
        else
            limLog(pMac, LOG1,
               FL("received ReAssoc req with unsupported capabilities from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOG1);
        goto error;
    }

    updateContext = false;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    // Check if multiple SSID feature is not enabled
    if (psessionEntry->pLimStartBssReq->ssId.length)
    {
        if (limCmpSSid(pMac, &pAssocReq->ssId, psessionEntry) == false)
        {
            /*Temp hack for UPF35 - skip SSID check in order to be able to interop 
             with Marvel - they send their own SSID instead of ours*/
            if ( 0 != vos_get_skip_ssid_check())
            {
                limLog(pMac, LOG1, FL("Received unmatched SSID but cfg to supress - continuing\n"));
            }
            else
            {
              /**
              * Received Re/Association Request with either
              * Broadcast SSID OR with SSID that does not
              * match with local one.
              * Respond with unspecified status code.
              */
              limSendAssocRspMgmtFrame(pMac,
                               eSIR_MAC_UNSPEC_FAILURE_STATUS,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);
  
              // Log error
              if (subType == LIM_ASSOC)
                  limLog(pMac, LOGW, FL("received Assoc req with unmatched SSID from \n"));
              else
                  limLog(pMac, LOGW, FL("received ReAssoc req with unmatched SSID from \n"));
              limPrintMacAddr(pMac, pHdr->sa, LOGW);
              goto error;
          }
      }
    }
    else
        limLog(pMac, LOG1, FL("Suppressed SSID, App is going to check SSID\n"));
#else
    if (limCmpSSid(pMac, &pAssocReq->ssId, psessionEntry) == false)
    {
        /**
         * Received Re/Association Request with either
         * Broadcast SSID OR with SSID that does not
         * match with local one.
         * Respond with unspecified status code.
         */
        limSendAssocRspMgmtFrame(pMac,
                             eSIR_MAC_UNSPEC_FAILURE_STATUS,
                             1,
                             pHdr->sa,
                             subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
                   FL("received Assoc req with unmatched SSID from \n"));
        else
            limLog(pMac, LOGW,
                   FL("received ReAssoc req with unmatched SSID from \n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }
#endif

    // Check if requested rates are among BSS basic rate set
    if (limCheckRxBasicRates(pMac, pAssocReq->supportedRates, psessionEntry) == false)
    {
        /**
         * Requesting STA does not support ALL BSS basic
         * rates. Respond with 'basic rates not supported'
         * status code.
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported rates from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported rates from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }

#ifdef WLAN_SOFTAP_FEATURE

    if((psessionEntry->limSystemRole == eLIM_AP_ROLE ) &&
       (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11G_ONLY) &&
       ((!pAssocReq->extendedRatesPresent ) || (pAssocReq->HTCaps.present)))
    {
        limSendAssocRspMgmtFrame( pMac, eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS, 
                                  1, pHdr->sa, subType, 0, psessionEntry );
        limLog(pMac, LOGE, FL("SOFTAP was in 11G only mode, rejecting legacy STA's\n"));
        goto error;

    }//end if phyMode == 11G_only
 
    if((psessionEntry->limSystemRole == eLIM_AP_ROLE) && 
       (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11N_ONLY) && 
       (!pAssocReq->HTCaps.present))
    {
        limSendAssocRspMgmtFrame( pMac, eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS, 
                                  1, pHdr->sa, subType, 0, psessionEntry );
        limLog(pMac, LOGE, FL("SOFTAP was in 11N only mode, rejecting legacy STA's\n"));
        goto error;
    }//end if PhyMode == 11N_only

#endif

    /* Spectrum Management (11h) specific checks */
    if (localCapabilities.spectrumMgt)
    {
        tSirRetStatus status = eSIR_SUCCESS;

        /* If station is 11h capable, then it SHOULD send all mandatory 
         * IEs in assoc request frame. Let us verify that
         */
        if (pAssocReq->capabilityInfo.spectrumMgt)
        {
            if (!((pAssocReq->powerCapabilityPresent) && (pAssocReq->supportedChannelsPresent)))
            {
                /* One or more required information elements are missing, log the peers error */
                if (!pAssocReq->powerCapabilityPresent)
                {
                    if(subType == LIM_ASSOC)
                       limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in assoc request\n"));
                    else
                       limLog(pMac, LOG1, FL("LIM Info: Missing Power capability IE in Reassoc request\n"));
                }
                if (!pAssocReq->supportedChannelsPresent)
                {
                    if(subType == LIM_ASSOC)
                        limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in assoc request\n"));
                    else
                        limLog(pMac, LOG1, FL("LIM Info: Missing Supported channel IE in Reassoc request\n"));
                }
                limPrintMacAddr(pMac, pHdr->sa, LOG1);
            }
            else
            {
                /* Assoc request has mandatory fields */
                status = limIsDot11hPowerCapabilitiesInRange(pMac, pAssocReq, psessionEntry);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        limLog(pMac, LOGW, FL("LIM Info: Association MinTxPower(STA) > MaxTxPower(AP)\n"));
                    else
                        limLog(pMac, LOGW, FL("LIM Info: Reassociation MinTxPower(STA) > MaxTxPower(AP)\n"));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                status = limIsDot11hSupportedChannelsValid(pMac, pAssocReq);
                if (eSIR_SUCCESS != status)
                {
                    if (subType == LIM_ASSOC)
                        limLog(pMac, LOGW, FL("LIM Info: Association wrong supported channels (STA)\n"));
                    else
                        limLog(pMac, LOGW, FL("LIM Info: Rassociation wrong supported channels (STA)\n"));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                }
                /* IEs are valid, use them if needed */
            }
        } //if(assoc.capabilityInfo.spectrumMgt)
        else
        {
            /* As per the capabiities, the spectrum management is not enabled on the station
             * The AP may allow the associations to happen even if spectrum management is not
             * allowed, if the transmit power of station is below the regulatory maximum
             */

            /* TODO: presently, this is not handled. In the current implemetation, the AP would
             * allow the station to associate even if it doesn't support spectrum management.
             */
        }
    }// end of spectrum management related processing

    if ( (pAssocReq->HTCaps.present) && (limCheckMCSSet(pMac, pAssocReq->HTCaps.supportedMCSSet) == false))
    {
        /**
         * Requesting STA does not support ALL BSS MCS basic Rate set rates.
         * Spec does not define any status code for this scenario.
         */
        limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);

        // Log error
        if (subType == LIM_ASSOC)
            limLog(pMac, LOGW,
               FL("received Assoc req with unsupported MCS Rate Set from \n"));
        else
            limLog(pMac, LOGW,
               FL("received ReAssoc req with unsupported MCS Rate Set from\n"));
        limPrintMacAddr(pMac, pHdr->sa, LOGW);
        goto error;
    }

    //if (pMac->dph.gDphPhyMode == WNI_CFG_PHY_MODE_11G)
    if (phyMode == WNI_CFG_PHY_MODE_11G)
    {
		
        if (wlan_cfgGetInt(pMac, WNI_CFG_11G_ONLY_POLICY, &val) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("could not retrieve 11g-only flag\n"));
            goto error;
        }
		
        if (!pAssocReq->extendedRatesPresent && val)
        {
            /**
             * Received Re/Association Request from
             * 11b STA when 11g only policy option
             * is set.
             * Reject with unspecified status code.
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from 11b STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            
#ifdef WLAN_DEBUG    
            pMac->lim.gLim11bStaAssocRejectCount++;
#endif
            goto error;
        }
    }

#ifdef WMM_APSD
    // Save the QOS info element in assoc request..
    limGetWmeMode(pMac, &wmeMode);
    if (wmeMode == eHAL_SET)
    {
        tpQosInfoSta qInfo;

        qInfo = (tpQosInfoSta) (pAssocReq->qosCapability.qosInfo);

        if ((pMac->lim.gWmmApsd.apsdEnable == 0) && (qInfo->ac_be || qInfo->ac_bk || qInfo->ac_vo || qInfo->ac_vi))
        {

            /**
             * Received Re/Association Request from
             * 11b STA when 11g only policy option
             * is set.
             * Reject with unspecified status code.
             */
            limSendAssocRspMgmtFrame(
                           pMac,
                           eSIR_MAC_WME_REFUSED_STATUS,
                           1,
                           pHdr->sa,
                           subType, 0,psessionEntry);

            limLog(pMac, LOGW,
                   FL("Rejecting Re/Assoc req from STA: "));
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            limLog(pMac, LOGE, FL("APSD not enabled, qosInfo - 0x%x\n"), *qInfo);
            goto error;
        }
    }
#endif

    // Check for 802.11n HT caps compatibility; are HT Capabilities
    // turned on in lim?
    if ( psessionEntry->htCapabality )
    {
        // There are; are they turned on in the STA?
        if ( pAssocReq->HTCaps.present )
        {
            // The station *does* support 802.11n HT capability...

            limLog( pMac, LOG1, FL( "AdvCodingCap:%d ChaWidthSet:%d "
                                    "PowerSave:%d greenField:%d "
                                    "shortGI20:%d shortGI40:%d\n"
                                    "txSTBC:%d rxSTBC:%d delayBA:%d"
                                    "maxAMSDUsize:%d DSSS/CCK:%d "
                                    "PSMP:%d stbcCntl:%d lsigTXProt:%d\n"),
                    pAssocReq->HTCaps.advCodingCap,
                    pAssocReq->HTCaps.supportedChannelWidthSet,
                    pAssocReq->HTCaps.mimoPowerSave,
                    pAssocReq->HTCaps.greenField,
                    pAssocReq->HTCaps.shortGI20MHz,
                    pAssocReq->HTCaps.shortGI40MHz,
                    pAssocReq->HTCaps.txSTBC,
                    pAssocReq->HTCaps.rxSTBC,
                    pAssocReq->HTCaps.delayedBA,
                    pAssocReq->HTCaps.maximalAMSDUsize,
                    pAssocReq->HTCaps.dsssCckMode40MHz,
                    pAssocReq->HTCaps.psmp,
                    pAssocReq->HTCaps.stbcControlFrame,
                    pAssocReq->HTCaps.lsigTXOPProtection );

                // Make sure the STA's caps are compatible with our own:
                //11.15.2 Support of DSSS/CCK in 40 MHz
                //the AP shall refuse association requests from an HT STA that has the DSSS/CCK 
                //Mode in 40 MHz subfield set to 1;

		//FIXME_BTAMP_AP : Need to be enabled 
			/*
            if ( !pMac->lim.gHTDsssCckRate40MHzSupport && pAssocReq->HTCaps.dsssCckMode40MHz )
            {
                statusCode = eSIR_MAC_DSSS_CCK_RATE_NOT_SUPPORT_STATUS;
                limLog( pMac, LOGW, FL( "AP DSSS/CCK is disabled; "
                                        "STA rejected.\n" ) );
                // Reject association
                limSendAssocRspMgmtFrame( pMac, statusCode, 1, pHdr->sa, subType, 0,psessionEntry);
                goto error;
            }
            */
        }
    } // End if on HT caps turned on in lim.

#ifdef WLAN_SOFTAP_FEATURE

    /** check whether as RSN IE is present */
    if(psessionEntry->limSystemRole == eLIM_AP_ROLE 
        && psessionEntry->pLimStartBssReq->privacy 
        && psessionEntry->pLimStartBssReq->rsnIE.length)
    {
        limLog(pMac, LOGE,
               FL("AP supports RSN enabled authentication\n"));

        if(pAssocReq->rsnPresent)
        {
            if(pAssocReq->rsn.length)
            {
                // Unpack the RSN IE 
                dot11fUnpackIeRSN(pMac, 
                                    &pAssocReq->rsn.info[0], 
                                    pAssocReq->rsn.length, 
                                    &Dot11fIERSN);

                /* Check RSN version is supported or not */
                if(SIR_MAC_OUI_VERSION_1 == Dot11fIERSN.version)
                {
                    /* check the groupwise and pairwise cipher suites */
                    if(eSIR_SUCCESS != (status = limCheckRxRSNIeMatch(pMac,Dot11fIERSN,psessionEntry) ) )
                    {
                        /* some IE is not properly sent */
                        /* received Association req frame with RSN IE but length is 0 */
                        limSendAssocRspMgmtFrame(
                                       pMac,
                                       status,
                                       1,
                                       pHdr->sa,
                                       subType, 0,psessionEntry);

                        limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                        limPrintMacAddr(pMac, pHdr->sa, LOGW);
                        goto error;

                    }
                }
                else
                {
                    /* received Association req frame with RSN IE version wrong */
                    limSendAssocRspMgmtFrame(
                                   pMac,
                                   eSIR_MAC_UNSUPPORTED_RSN_IE_VERSION_STATUS,
                                   1,
                                   pHdr->sa,
                                   subType, 0,psessionEntry);

                    limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                    goto error;

                }
            }
            else
            {
                /* received Association req frame with RSN IE but length is 0 */
                limSendAssocRspMgmtFrame(
                               pMac,
                               eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);

                limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                limPrintMacAddr(pMac, pHdr->sa, LOGW);
                goto error;
                
            }
        } /* end - if(pAssocReq->rsnPresent) */
        if((!pAssocReq->rsnPresent) && pAssocReq->wpaPresent)
        {
            // Unpack the WPA IE 
            if(pAssocReq->wpa.length)
            {
                dot11fUnpackIeWPA(pMac, 
                                    &pAssocReq->wpa.info[4], //OUI is not taken care
                                    pAssocReq->wpa.length, 
                                    &Dot11fIEWPA);
                /* check the groupwise and pairwise cipher suites */
                if(eSIR_SUCCESS != (status = limCheckRxWPAIeMatch(pMac,Dot11fIEWPA,psessionEntry)))
                {
                    /* received Association req frame with WPA IE but mismatch */
                    limSendAssocRspMgmtFrame(
                                   pMac,
                                   status,
                                   1,
                                   pHdr->sa,
                                   subType, 0,psessionEntry);

                    limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                    limPrintMacAddr(pMac, pHdr->sa, LOGW);
                    goto error;

                }
            }
            else
            {
                /* received Association req frame with invalid WPA IE */
                limSendAssocRspMgmtFrame(
                               pMac,
                               eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);

                limLog(pMac, LOGW, FL("Rejecting Re/Assoc req from STA: "));
                limPrintMacAddr(pMac, pHdr->sa, LOGW);
                goto error;
            }/* end - if(pAssocReq->wpa.length) */
        } /* end - if(pAssocReq->wpaPresent) */
    } /* end of if(psessionEntry->pLimStartBssReq->privacy 
        && psessionEntry->pLimStartBssReq->rsnIE->length) */
#endif //WLAN_SOFTAP_FEATURE

    /**
     * Extract 'associated' context for STA, if any.
     * This is maintained by DPH and created by LIM.
     */
    pStaDs = dphLookupHashEntry(pMac, pHdr->sa, &aid, &psessionEntry->dph.dphHashTable);

    /// Extract pre-auth context for the STA, if any.
    pStaPreAuthContext = limSearchPreAuthList(pMac, pHdr->sa);

    if (pStaDs == NULL)
    {
        /// Requesting STA is not currently associated
        if (pMac->lim.gLimNumOfCurrentSTAs == pMac->lim.maxStation)
        {
            /**
             * Maximum number of STAs that AP can handle reached.
             * Send Association response to peer MAC entity
             */
            limRejectAssociation(pMac, pHdr->sa,
                                 subType, false,
                                 (tAniAuthType) 0, 0,
                                 false,
                                 (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

            goto error;
        }

        /// Check if STA is pre-authenticated.
        if ((pStaPreAuthContext == NULL) ||
            (pStaPreAuthContext &&
             (pStaPreAuthContext->mlmState !=
                              eLIM_MLM_AUTHENTICATED_STATE)))
        {
            /**
             * STA is not pre-authenticated yet requesting
             * Re/Association before Authentication.
             * OR STA is in the process of getting authenticated
             * and sent Re/Association request.
             * Send Deauthentication frame with 'prior
             * authentication required' reason code.
             */
            limSendDeauthMgmtFrame(
                     pMac,
                     eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON, //=9
                     pHdr->sa,psessionEntry);

            // Log error
            if (subType == LIM_ASSOC)
                limLog(pMac, LOG1,
                       FL("received Assoc req from STA that does not have pre-auth context, MAC addr is: \n"));
            else
                limLog(pMac, LOG1,
                       FL("received ReAssoc req from STA that does not have pre-auth context, MAC addr is: \n"));
            limPrintMacAddr(pMac, pHdr->sa, LOG1);
            goto error;
        }

        /// Delete 'pre-auth' context of STA
        authType = pStaPreAuthContext->authType;
        limDeletePreAuthNode(pMac, pHdr->sa);

        // All is well. Assign AID (after else part)

    } // if (pStaDs == NULL)
    else
    {
        // STA context does exist for this STA

        if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE)
        {
            /**
             * Requesting STA is in some 'transient' state?
             * Ignore the Re/Assoc Req frame by incrementing
             * debug counter & logging error.
             */
            if (subType == LIM_ASSOC)
            {
            
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumAssocReqDropInvldState++;
#endif
                limLog(pMac, LOG1, FL("received Assoc req in state %X from "), pStaDs->mlmStaContext.mlmState);
            }
            else
            {     
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumReassocReqDropInvldState++;
#endif
                limLog(pMac, LOG1, FL("received ReAssoc req in state %X from "), pStaDs->mlmStaContext.mlmState);
            }
            limPrintMacAddr(pMac, pHdr->sa, LOG1);
            limPrintMlmState(pMac, LOG1, (tLimMlmStates) pStaDs->mlmStaContext.mlmState);
            goto error;
        } // if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE)

        /**
         * STA sent Re/Association Request frame while already in
         * 'associated' state. Update STA capabilities and
         * send Association response frame with same AID
         */

        pStaDs->mlmStaContext.capabilityInfo = pAssocReq->capabilityInfo;

        if (pStaPreAuthContext &&
            (pStaPreAuthContext->mlmState ==
                                       eLIM_MLM_AUTHENTICATED_STATE))
        {
            /// STA has triggered pre-auth again
            authType = pStaPreAuthContext->authType;
            limDeletePreAuthNode(pMac, pHdr->sa);
        }
        else
            authType = pStaDs->mlmStaContext.authType;

        updateContext = true;

        if (dphInitStaState(pMac, pHdr->sa, aid, true, &psessionEntry->dph.dphHashTable) == NULL)   
        {
            limLog(pMac, LOGE, FL("could not Init STAid=%d\n"), aid);
            goto  error;
        }


        goto sendIndToSme;
    } // end if (lookup for STA in perStaDs fails)



    // check if sta is allowed per QoS AC rules
    //if (pMac->dph.gDphQosEnabled || pMac->dph.gDphWmeEnabled)
    limGetWmeMode(pMac, &wmeMode);
    if ((qosMode == eHAL_SET) || (wmeMode == eHAL_SET))
    {
        // for a qsta, check if the requested Traffic spec
        // is admissible
        // for a non-qsta check if the sta can be admitted
        if (pAssocReq->addtsPresent)
        {
            tANI_U8 tspecIdx = 0; //index in the sch tspec table.
            if (limAdmitControlAddTS(pMac, pHdr->sa, &(pAssocReq->addtsReq),
                                     &(pAssocReq->qosCapability), 0, false, NULL, &tspecIdx, psessionEntry) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGW, FL("AdmitControl: TSPEC rejected\n"));
                limSendAssocRspMgmtFrame(
                               pMac,
                               eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                               1,
                               pHdr->sa,
                               subType, 0,psessionEntry);
#ifdef WLAN_DEBUG    
                pMac->lim.gLimNumAssocReqDropACRejectTS++;
#endif
                goto error;
            }
        }
        else if (limAdmitControlAddSta(pMac, pHdr->sa, false)
                                               != eSIR_SUCCESS)
        {
            limLog(pMac, LOGW, FL("AdmitControl: Sta rejected\n"));
            limSendAssocRspMgmtFrame(
                    pMac,
                    eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
                    1,
                    pHdr->sa,
                    subType, 0,psessionEntry);
#ifdef WLAN_DEBUG    
            pMac->lim.gLimNumAssocReqDropACRejectSta++;
#endif
            goto error;
        }

        // else all ok
        limLog(pMac, LOG1, FL("AdmitControl: Sta OK!\n"));
    }

    /**
     * STA is Associated !
     */
    if (subType == LIM_ASSOC)
        limLog(pMac, LOG1, FL("received Assoc req successful from "));
    else
        limLog(pMac, LOG1, FL("received ReAssoc req successful from "));
    limPrintMacAddr(pMac, pHdr->sa, LOG1);

    /**
     * Assign unused/least recently used AID from perStaDs.
     * This will 12-bit STAid used by MAC HW.
     * NOTE: limAssignAID() assigns AID values ranging between 1 - 255
     *       so make MSB of AID 0xC0.
     */

    aid = limAssignAID(pMac);

    if (!aid)
    {
        // Could not assign AID
        // Reject association
        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType,
                             aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }

    /**
     * Add an entry to hash table maintained by DPH module
     */

    pStaDs = dphAddHashEntry(pMac, pHdr->sa, aid, &psessionEntry->dph.dphHashTable);

    if (pStaDs == NULL)
    {
        // Could not add hash table entry at DPH
        limLog(pMac, LOGE,
           FL("could not add hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

        // Release AID
        limReleaseAID(pMac, aid);

        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, false,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }


sendIndToSme:

    pStaDs->mlmStaContext.htCapability = pAssocReq->HTCaps.present;
    pStaDs->qos.addtsPresent = (pAssocReq->addtsPresent==0) ? false : true;
    pStaDs->qos.addts        = pAssocReq->addtsReq;
    pStaDs->qos.capability   = pAssocReq->qosCapability;
    pStaDs->versionPresent   = 0;
 
	if (pAssocReq->propIEinfo.versionPresent) //update STA version info
    {
        pStaDs->versionPresent = 1;
        pStaDs->version = pAssocReq->propIEinfo.version;
    }
    pStaDs->propCapability = 0;
    if (pAssocReq->propIEinfo.capabilityPresent)
    {
        if (sirGetCfgPropCaps(pMac, &pStaDs->propCapability))
            pStaDs->propCapability &= pAssocReq->propIEinfo.capability;
    }

    pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_CNF_STATE;
    pStaDs->valid                  = 0;
    pStaDs->mlmStaContext.authType = authType;
    pStaDs->staType = STA_ENTRY_PEER;

    //TODO: If listen interval is more than certain limit, reject the association.
    //Need to check customer requirements and then implement.
    pStaDs->mlmStaContext.listenInterval = pAssocReq->listenInterval;
    pStaDs->mlmStaContext.capabilityInfo = pAssocReq->capabilityInfo;

    /* The following count will be used to knock-off the station if it doesn't
     * come back to receive the buffered data. The AP will wait for numTimSent number
     * of beacons after sending TIM information for the station, before assuming that 
     * the station is no more associated and disassociates it
     */

    /** timWaitCount is used by PMM for monitoring the STA's in PS for LINK*/
    pStaDs->timWaitCount = GET_TIM_WAIT_COUNT(pAssocReq->listenInterval);
    
    /** Initialise the Current successful MPDU's tranfered to this STA count as 0 */
    pStaDs->curTxMpduCnt = 0;

    if(IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
      (pAssocReq->HTCaps.present))
    {
        pStaDs->htGreenfield = (tANI_U8)pAssocReq->HTCaps.greenField;
        pStaDs->htAMpduDensity = pAssocReq->HTCaps.mpduDensity;
        pStaDs->htDsssCckRate40MHzSupport = (tANI_U8)pAssocReq->HTCaps.dsssCckMode40MHz;
        pStaDs->htLsigTXOPProtection = (tANI_U8)pAssocReq->HTCaps.lsigTXOPProtection;
        pStaDs->htMaxAmsduLength = (tANI_U8)pAssocReq->HTCaps.maximalAMSDUsize;
        pStaDs->htMaxRxAMpduFactor = pAssocReq->HTCaps.maxRxAMPDUFactor;
        pStaDs->htMIMOPSState = pAssocReq->HTCaps.mimoPowerSave;
        pStaDs->htShortGI20Mhz = (tANI_U8)pAssocReq->HTCaps.shortGI20MHz;
        pStaDs->htShortGI40Mhz = (tANI_U8)pAssocReq->HTCaps.shortGI40MHz;
        pStaDs->htSupportedChannelWidthSet = (tANI_U8)pAssocReq->HTCaps.supportedChannelWidthSet;
        pStaDs->baPolicyFlag = 0xFF;
    }



    if (limPopulateMatchingRateSet(pMac,
                                   pStaDs,
                                   &(pAssocReq->supportedRates),
                                   &(pAssocReq->extendedRates),
                                   pAssocReq->HTCaps.supportedMCSSet,
                                   &(pAssocReq->propIEinfo.propRates), psessionEntry) != eSIR_SUCCESS)
    {
        // Could not update hash table entry at DPH with rateset
        limLog(pMac, LOGE,
           FL("could not update hash entry at DPH for aid=%d, MacAddr:\n"),
           aid);
        limPrintMacAddr(pMac, pHdr->sa, LOGE);

                // Release AID
        limReleaseAID(pMac, aid);


        limRejectAssociation(pMac, pHdr->sa,
                             subType, true, authType, aid, true,
                             (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

        goto error;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pStaDs->mlmStaContext.propRateSet,
                  (tANI_U8 *) &(pAssocReq->propIEinfo.propRates),
                  pAssocReq->propIEinfo.propRates.numPropRates + 1);

    /// Add STA context at MAC HW (BMU, RHP & TFP)

    pStaDs->qosMode    = eANI_BOOLEAN_FALSE;
    pStaDs->lleEnabled = eANI_BOOLEAN_FALSE;
    if (pAssocReq->capabilityInfo.qos && (qosMode == eHAL_SET))
    {
        pStaDs->lleEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode    = eANI_BOOLEAN_TRUE; 
    }

    pStaDs->wmeEnabled = eANI_BOOLEAN_FALSE;
    pStaDs->wsmEnabled = eANI_BOOLEAN_FALSE;
    limGetWmeMode(pMac, &wmeMode);
    //if ((! pStaDs->lleEnabled) && assoc.wmeInfoPresent && pMac->dph.gDphWmeEnabled)
    if ((! pStaDs->lleEnabled) && pAssocReq->wmeInfoPresent && (wmeMode == eHAL_SET))
    {
        pStaDs->wmeEnabled = eANI_BOOLEAN_TRUE;
        pStaDs->qosMode = eANI_BOOLEAN_TRUE;
        limGetWsmMode(pMac, &wsmMode);
        /* WMM_APSD - WMM_SA related processing should be separate; WMM_SA and WMM_APSD
         can coexist */
#ifdef WLAN_SOFTAP_FEATURE
        if( pAssocReq->WMMInfoStation.present)
        {
            /* check whether AP supports or not */
            if ((psessionEntry->limSystemRole == eLIM_AP_ROLE)
                 && (psessionEntry->apUapsdEnable == 0) && (pAssocReq->WMMInfoStation.acbe_uapsd 
                    || pAssocReq->WMMInfoStation.acbk_uapsd 
                    || pAssocReq->WMMInfoStation.acvo_uapsd 
                    || pAssocReq->WMMInfoStation.acvi_uapsd))
            {

                /**
                 * Received Re/Association Request from
                 * STA when UPASD is not supported.
                 */
                limLog( pMac, LOGE, FL( "AP do not support UPASD REASSOC Failed\n" ));
                limRejectAssociation(pMac, pHdr->sa,
                                     subType, true, authType, aid, true,
                                     (tSirResultCodes) eSIR_MAC_WME_REFUSED_STATUS, psessionEntry);

                goto error;
            }
            else
            {
                /* update UAPSD and send it to LIM to add STA */
                pStaDs->qos.capability.qosInfo.acbe_uapsd = pAssocReq->WMMInfoStation.acbe_uapsd;
                pStaDs->qos.capability.qosInfo.acbk_uapsd = pAssocReq->WMMInfoStation.acbk_uapsd;
                pStaDs->qos.capability.qosInfo.acvo_uapsd = pAssocReq->WMMInfoStation.acvo_uapsd;
                pStaDs->qos.capability.qosInfo.acvi_uapsd = pAssocReq->WMMInfoStation.acvi_uapsd;
                pStaDs->qos.capability.qosInfo.maxSpLen = pAssocReq->WMMInfoStation.max_sp_length;
            }
        }
#endif
        //if (assoc.wsmCapablePresent && pMac->dph.gDphWsmEnabled)
        if (pAssocReq->wsmCapablePresent && (wsmMode == eHAL_SET))
            pStaDs->wsmEnabled = eANI_BOOLEAN_TRUE;

    }

	// Re/Assoc Response frame to requesting STA
    pStaDs->mlmStaContext.subType = subType;

    if (pAssocReq->propIEinfo.aniIndicator)
        pStaDs->aniPeer = 1;

    // BTAMP: Storing the parsed assoc request in the psessionEntry array
    psessionEntry->parsedAssocReq[pStaDs->assocId] = pAssocReq;

    /* BTAMP: If STA context already exist (ie. updateContext = 1)  
     * for this STA, then we should delete the old one, and add 
     * the new STA. This is taken care of in the limDelSta() routine.   
     *
     * Prior to BTAMP, we were setting this flag so that when
     * PE receives SME_ASSOC_CNF, and if this flag is set, then 
     * PE shall delete the old station and then add. But now in
     * BTAMP, we're directly adding station before waiting for
     * SME_ASSOC_CNF, so we can do this now.  
     */
    if (!updateContext)
    {
        pStaDs->mlmStaContext.updateContext = 0;

        // BTAMP: Add STA context at HW - issue SIR_HAL_ADD_STA_REQ to HAL
        if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGE, FL("could not Add STA with assocId=%d\n"), pStaDs->assocId);
            limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType,
                                  true, pStaDs->mlmStaContext.authType, pStaDs->assocId, true,
                                  (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);
            goto error;
        }
    }
    else
    {
        pStaDs->mlmStaContext.updateContext = 1;

#ifdef WLAN_SOFTAP_FEATURE
        pStaDs->shortPreambleEnabled = pAssocReq->capabilityInfo.shortPreamble;
        mlmPrevState = pStaDs->mlmStaContext.mlmState;

        /* As per the HAL/FW needs the reassoc req need not be calling limDelSta */
        if(subType != LIM_REASSOC)
        {
            //we need to set the mlmState here in order differentiate in limDelSta.
            pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
            if(limDelSta(pMac, pStaDs, true, psessionEntry) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGE, FL("could not DEL STA with assocId=%d staId %d\n"), pStaDs->assocId, pStaDs->staIndex);
                limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType, true, pStaDs->mlmStaContext.authType,
                                      pStaDs->assocId, true,(tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS, psessionEntry);

                //Restoring the state back.
                pStaDs->mlmStaContext.mlmState = mlmPrevState;
                goto error;
            }
        }
        else
        {
            /* mlmState is changed in limAddSta context */
            /* use the same AID, already allocated */
            if (limAddSta(pMac, pStaDs,psessionEntry) != eSIR_SUCCESS)
            {
                    limLog( pMac, LOGE, FL( "AP do not support UPASD REASSOC Failed\n" ));
                    limRejectAssociation( pMac, pStaDs->staAddr, pStaDs->mlmStaContext.subType, true, pStaDs->mlmStaContext.authType,
                                          pStaDs->assocId, true,(tSirResultCodes) eSIR_MAC_WME_REFUSED_STATUS, psessionEntry);

                    //Restoring the state back.
                    pStaDs->mlmStaContext.mlmState = mlmPrevState;
                    goto error;
            }

        }
#endif

    }

    return;

error:
    if (pAssocReq != NULL)
    {
        if (palFreeMemory(pMac->hHdd, pAssocReq) != eHAL_STATUS_SUCCESS) 
        {
            limLog(pMac, LOGP, FL("PalFree Memory failed \n"));
            return;
        }
    }
		
    if(pStaDs!= NULL)
		psessionEntry->parsedAssocReq[pStaDs->assocId] = NULL;
    return;

} /*** end limBtAmpProcessAssocReqFrame() ***/



/**---------------------------------------------------------------
\fn     limSendMlmAssocInd
\brief  This function sends either LIM_MLM_ASSOC_IND  
\       or LIM_MLM_REASSOC_IND to SME.
\
\param  pMac
\param  *pStaDs - Station DPH hash entry
\param  psessionEntry - PE session entry
\return None

 * ?????? How do I get 
 *  - subtype   =====> psessionEntry->parsedAssocReq.reassocRequest
 *  - aid       =====> pStaDs->assocId
 *  - pHdr->sa  =====> pStaDs->staAddr
 *  - authType
 *  - pHdr->seqControl  =====> no longer needed
 *  - pStaDs
------------------------------------------------------------------*/
void limSendMlmAssocInd(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpPESession psessionEntry)
{
    tpLimMlmAssocInd        pMlmAssocInd;
    tpLimMlmReassocInd      pMlmReassocInd;
    tpSirAssocReq           pAssocReq;
    tANI_U16                temp;
    tANI_U32                phyMode;
    tANI_U8                 subType;
    tANI_U32                wpsApEnable=0;
    tANI_U32                tmp, wpsVersion;
    tANI_U16                statusCode;    
    tANI_U16                i, j=0;

    // Get a copy of the already parsed Assoc Request
    pAssocReq = (tpSirAssocReq) psessionEntry->parsedAssocReq[pStaDs->assocId];

    // Get the phyMode
    limGetPhyMode(pMac, &phyMode);
 
    // Extract pre-auth context for the peer BTAMP-STA, if any.
 
    // Determiine if its Assoc or ReAssoc Request
    if (pAssocReq->reassocRequest == 1)
        subType = LIM_REASSOC;
    else 
        subType = LIM_ASSOC;
#ifdef WLAN_SOFTAP_FEATURE
    if (subType == LIM_ASSOC || subType == LIM_REASSOC)
#else
    if (subType == LIM_ASSOC )
#endif
    {
        temp  = sizeof(tLimMlmAssocInd);
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)        
        temp += pAssocReq->propIEinfo.numBss * sizeof(tSirNeighborBssInfo);
#endif        

        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAssocInd, temp))
        {
            limReleaseAID(pMac, pStaDs->assocId);
            limLog(pMac, LOGP, FL("palAllocateMemory failed for pMlmAssocInd\n"));
            return;
        }
         palCopyMemory( pMac->hHdd,(tANI_U8 *)pMlmAssocInd->peerMacAddr,(tANI_U8 *)pStaDs->staAddr,sizeof(tSirMacAddr));	
		 
		 pMlmAssocInd->aid    = pStaDs->assocId;
		 palCopyMemory( pMac->hHdd, (tANI_U8 *)&pMlmAssocInd->ssId,(tANI_U8 *)&(pAssocReq->ssId), pAssocReq->ssId.length + 1);
		 pMlmAssocInd->sessionId = psessionEntry->peSessionId;
		 pMlmAssocInd->authType =  pStaDs->mlmStaContext.authType;
		 
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        // Note for BTAMP: no need to fill in pMlmAssocInd->seqNum
        pMlmAssocInd->wniIndicator = (tAniBool) pAssocReq->propIEinfo.aniIndicator;
        pMlmAssocInd->bpIndicator  = (tAniBool) pAssocReq->propIEinfo.bpIndicator;
        pMlmAssocInd->bpType       = (tSirBpIndicatorType) pAssocReq->propIEinfo.bpType;
        if (pAssocReq->extendedRatesPresent)
        {
            pMlmAssocInd->nwType = eSIR_11G_NW_TYPE;
            limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_SET);
        }
        else
        {
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                pMlmAssocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmAssocInd->nwType = eSIR_11B_NW_TYPE;
                limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_CLEAR);
            }
        }
        pMlmAssocInd->assocType = (tSirAssocType)pAssocReq->propIEinfo.assocType;
        pMlmAssocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmAssocInd->load.channelUtilization =(pMac->lim.gpLimMeasData) ? pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmAssocInd->numBss = (tANI_U32) pAssocReq->propIEinfo.numBss;
        if (pAssocReq->propIEinfo.numBss)
        {
            palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmAssocInd->neighborList,(tANI_U8 *)pAssocReq->propIEinfo.pBssList,
                           (sizeof(tSirNeighborBssInfo) * pAssocReq->propIEinfo.numBss));
        } 
#endif
        pMlmAssocInd->capabilityInfo = pAssocReq->capabilityInfo;

        // Fill in RSN IE information
        pMlmAssocInd->rsnIE.length = 0;
        if (pAssocReq->rsnPresent)
        {
            limLog(pMac, LOG2, FL("Assoc Req RSN IE len = %d\n"), pAssocReq->rsn.length);
            pMlmAssocInd->rsnIE.length = 2 + pAssocReq->rsn.length;
            pMlmAssocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[1] = pAssocReq->rsn.length;
            palCopyMemory( pMac->hHdd, 
                           &pMlmAssocInd->rsnIE.rsnIEdata[2],
                           pAssocReq->rsn.info,
                           pAssocReq->rsn.length);
        }

        //FIXME: we need to have the cb information seprated between HT and Titan later. 
        if(pAssocReq->HTCaps.present)
            limGetHtCbAdminState(pMac, pAssocReq->HTCaps, &pMlmAssocInd->titanHtCaps);

        // Fill in 802.11h related info
        if (pAssocReq->powerCapabilityPresent && pAssocReq->supportedChannelsPresent)
        {
            pMlmAssocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmAssocInd->powerCap.minTxPower = pAssocReq->powerCapability.minTxPower;
            pMlmAssocInd->powerCap.maxTxPower = pAssocReq->powerCapability.maxTxPower;
            limConvertSupportedChannels(pMac, pMlmAssocInd, pAssocReq);
        }
        else
            pMlmAssocInd->spectrumMgtIndicator = eSIR_FALSE;


        if (pAssocReq->wpaPresent)
        {
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 1] = pAssocReq->wpa.length;
            palCopyMemory( pMac->hHdd,
                           &pMlmAssocInd->rsnIE.rsnIEdata[pMlmAssocInd->rsnIE.length + 2],
                           pAssocReq->wpa.info,
                           pAssocReq->wpa.length);
            pMlmAssocInd->rsnIE.length += 2 + pAssocReq->wpa.length;
        }


        /* Initialize wscInfo. */
        vos_mem_set(&pMlmAssocInd->wscInfo, sizeof(tSirMacWscInfo), 0);
           
        if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
             limLog(pMac, LOGP, FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WPS_ENABLE );        
        wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

        if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_VERSION, &wpsVersion) != eSIR_SUCCESS)
             limLog(pMac, LOGP,FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WPS_VERSION );

        if (pAssocReq->wscInfo.present && wpsApEnable)
        {
            /* Compare wps version of the received assoc req with BTAMP-AP's wps version */
            /* If not compatible, reject; else, copy into wscinfo into sLimMlmAssocInd */
            if (pAssocReq->wscInfo.wpsVersion != wpsVersion)
            {
                /* Reject Association */
                statusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                limSendAssocRspMgmtFrame( pMac, statusCode, 1, pStaDs->staAddr, subType, 0 ,psessionEntry);
            }
            else
            {
                /* Copy sSirAssocReq.sSirMacWscInfo to sLimMlmAssocInd.sSirMacWscInfo */
                pMlmAssocInd->wscInfo.present         = 1;
                pMlmAssocInd->wscInfo.wpsVersion      = pAssocReq->wscInfo.wpsVersion;
                pMlmAssocInd->wscInfo.wpsRequestType  = pAssocReq->wscInfo.wpsRequestType;
            }
        }

#ifdef WLAN_SOFTAP_FEATURE
        if(pAssocReq->wmeInfoPresent)
        {

            if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WME_ENABLED, &tmp) != eSIR_SUCCESS)
                 limLog(pMac, LOGP, FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WME_ENABLED );

            /* check whether AP is enabled with WMM */
            if(tmp)
            {
                pMlmAssocInd->WmmStaInfoPresent = 1;
            }
            else
            {
                pMlmAssocInd->WmmStaInfoPresent= 0;
            }
            /* Note: we are not rejecting association here because IOT will fail */

        }
#endif

        limPostSmeMessage(pMac, LIM_MLM_ASSOC_IND, (tANI_U32 *) pMlmAssocInd);
        palFreeMemory( pMac->hHdd, pMlmAssocInd);
    }
    else
    {
        // If its of Reassociation Request, then post LIM_MLM_REASSOC_IND 
        temp  = sizeof(tLimMlmReassocInd);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        temp += pAssocReq->propIEinfo.numBss * sizeof(tSirNeighborBssInfo);
#endif
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmReassocInd, temp))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pMlmReassocInd\n"));
            limReleaseAID(pMac, pStaDs->assocId);
            return;
        }

        palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmReassocInd->peerMacAddr, (tANI_U8 *)pStaDs->staAddr, sizeof(tSirMacAddr));
        palCopyMemory( pMac->hHdd,(tANI_U8 *) pMlmReassocInd->currentApAddr, (tANI_U8 *)&(pAssocReq->currentApAddr), sizeof(tSirMacAddr));
        pMlmReassocInd->aid = pStaDs->assocId;
        pMlmReassocInd->authType = pStaDs->mlmStaContext.authType;
        palCopyMemory( pMac->hHdd,(tANI_U8 *)&pMlmReassocInd->ssId, (tANI_U8 *)&(pAssocReq->ssId), pAssocReq->ssId.length + 1);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        // Note for BTAMP: no need to fill in pMlmAssocInd->seqNum
        pMlmReassocInd->wniIndicator = (tAniBool) pAssocReq->propIEinfo.aniIndicator;
        pMlmReassocInd->bpIndicator  = (tAniBool) pAssocReq->propIEinfo.bpIndicator;
        pMlmReassocInd->bpType       = (tSirBpIndicatorType) pAssocReq->propIEinfo.bpType;
        if (pAssocReq->extendedRatesPresent)
        {
            pMlmReassocInd->nwType = eSIR_11G_NW_TYPE;
            limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_SET);
        }
        else
        {
            if (phyMode == WNI_CFG_PHY_MODE_11A)
                pMlmReassocInd->nwType = eSIR_11A_NW_TYPE;
            else
            {
                pMlmReassocInd->nwType = eSIR_11B_NW_TYPE;
                limSetStaHashErpMode(pMac, pStaDs->assocId, eHAL_CLEAR);
            }
        }

        pMlmReassocInd->reassocType  = (tSirAssocType)pAssocReq->propIEinfo.assocType;
        pMlmReassocInd->load.numStas = pMac->lim.gLimNumOfCurrentSTAs;
        pMlmReassocInd->load.channelUtilization = (pMac->lim.gpLimMeasData) ?
                                                  pMac->lim.gpLimMeasData->avgChannelUtilization : 0;
        pMlmReassocInd->numBss = (tANI_U32) pAssocReq->propIEinfo.numBss;
        if (pAssocReq->propIEinfo.numBss)
        {
            palCopyMemory( pMac->hHdd, 
                           (tANI_U8 *) pMlmReassocInd->neighborList,
                           (tANI_U8 *) pAssocReq->propIEinfo.pBssList,
                           (sizeof(tSirNeighborBssInfo) * pAssocReq->propIEinfo.numBss));
        }
#endif
        if (pAssocReq->propIEinfo.aniIndicator)
            pStaDs->aniPeer = 1;

        pMlmReassocInd->capabilityInfo = pAssocReq->capabilityInfo;
        pMlmReassocInd->rsnIE.length = 0;

        if (pAssocReq->rsnPresent)
        {
            limLog(pMac, LOG2, FL("Assoc Req: RSN IE length = %d\n"), pAssocReq->rsn.length);
            pMlmReassocInd->rsnIE.length = 2 + pAssocReq->rsn.length;
            pMlmReassocInd->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[1] = pAssocReq->rsn.length;
            palCopyMemory( pMac->hHdd, &pMlmReassocInd->rsnIE.rsnIEdata[2], pAssocReq->rsn.info, pAssocReq->rsn.length);
        }

        if(pAssocReq->HTCaps.present)
              limGetHtCbAdminState(pMac, pAssocReq->HTCaps,  &pMlmReassocInd->titanHtCaps );

        // 802.11h support
        if (pAssocReq->powerCapabilityPresent && pAssocReq->supportedChannelsPresent)
        {
            pMlmReassocInd->spectrumMgtIndicator = eSIR_TRUE;
            pMlmReassocInd->powerCap.minTxPower = pAssocReq->powerCapability.minTxPower;
            pMlmReassocInd->powerCap.maxTxPower = pAssocReq->powerCapability.maxTxPower;
            pMlmReassocInd->supportedChannels.numChnl = (tANI_U8)(pAssocReq->supportedChannels.length / 2);

            limLog(pMac, LOG1,
                FL("Sending Reassoc Ind: spectrum ON, minPwr %d, maxPwr %d, numChnl %d\n"),
                pMlmReassocInd->powerCap.minTxPower,
                pMlmReassocInd->powerCap.maxTxPower,
                pMlmReassocInd->supportedChannels.numChnl);

            for(i=0; i < pMlmReassocInd->supportedChannels.numChnl; i++)
            {
                pMlmReassocInd->supportedChannels.channelList[i] = pAssocReq->supportedChannels.supportedChannels[j];
                limLog(pMac, LOG1, FL("Sending ReassocInd: chn[%d] = %d \n"),
                       i, pMlmReassocInd->supportedChannels.channelList[i]);
                j+=2;
            }
        }
        else
            pMlmReassocInd->spectrumMgtIndicator = eSIR_FALSE;


        if (pAssocReq->wpaPresent)
        {
            limLog(pMac, LOG2, FL("Received WPA IE length in Assoc Req is %d\n"), pAssocReq->wpa.length);
            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length] = SIR_MAC_WPA_EID;
            pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 1] = pAssocReq->wpa.length;
            palCopyMemory( pMac->hHdd,
                           &pMlmReassocInd->rsnIE.rsnIEdata[pMlmReassocInd->rsnIE.length + 2],
                           pAssocReq->wpa.info,
                           pAssocReq->wpa.length);
            pMlmReassocInd->rsnIE.length += 2 + pAssocReq->wpa.length;
        }


        /* Initialize wscInfo. */
       vos_mem_set(&pMlmReassocInd->wscInfo, sizeof(tSirMacWscInfo), 0);
       if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
            limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
       
       wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

       if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_VERSION, &wpsVersion) != eSIR_SUCCESS)
            limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_VERSION );

        if (pAssocReq->wscInfo.present && wpsApEnable)
        {
            /* Compare wps version of the received assoc req with AP's wps version */
            /* If not compatible, reject; else, copy into wscinfo into sLimMlmAssocInd */
            if (pAssocReq->wscInfo.wpsVersion != wpsVersion)
            {
                /* Reject Reassociation */
                statusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
                limSendAssocRspMgmtFrame( pMac, statusCode, 1, pStaDs->staAddr, subType, 0 ,psessionEntry);
            }
            else
            {
                /* Copy sSirReassocReq.sSirMacWscInfo to sLimMlmReassocInd.sSirMacWscInfo */
                pMlmReassocInd->wscInfo.present         = 1;
                pMlmReassocInd->wscInfo.wpsVersion      = pAssocReq->wscInfo.wpsVersion;
                pMlmReassocInd->wscInfo.wpsRequestType  = pAssocReq->wscInfo.wpsRequestType;

            }
        }

#ifdef WLAN_SOFTAP_FEATURE
        if(pAssocReq->wmeInfoPresent)
        {

            if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WME_ENABLED, &tmp) != eSIR_SUCCESS)
                 limLog(pMac, LOGP, FL("wlan_cfgGetInt failed for id %d\n"), WNI_CFG_WME_ENABLED );

            /* check whether AP is enabled with WMM */
            if(tmp)
            {
                pMlmReassocInd->WmmStaInfoPresent = 1;
            }
            else
            {
                pMlmReassocInd->WmmStaInfoPresent = 0;
            }
            /* Note: we are not rejecting Re-association here because IOT will fail */

        }
#endif

        limPostSmeMessage(pMac, LIM_MLM_REASSOC_IND, (tANI_U32 *) pMlmReassocInd);
        palFreeMemory( pMac->hHdd, pMlmReassocInd);
    }

    return;

} /*** end limSendMlmAssocInd() ***/
