/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limSmeReqUtils.cc contains the utility functions
 * for processing SME request messages.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 
 */

#include "wniApi.h"
#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"



/**
 * limIsRSNieValidInSmeReqMessage()
 *
 *FUNCTION:
 * This function is called to verify if the RSN IE
 * received in various SME_REQ messages is valid or not
 *
 *LOGIC:
 * RSN IE validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  pRSNie Pointer to received RSN IE
 * @return true when RSN IE is valid, false otherwise
 */

static tANI_U8
limIsRSNieValidInSmeReqMessage(tpAniSirGlobal pMac, tpSirRSNie pRSNie)
{
    tANI_U8  wpaIndex = 0;
    tANI_U32 privacy, val;

    if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                  &privacy) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP,
               FL("Unable to retrieve POI from CFG\n"));
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_RSN_ENABLED,
                  &val) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP,
               FL("Unable to retrieve RSN_ENABLED from CFG\n"));
    }

    if (pRSNie->length && (!privacy || !val))
    {
        // Privacy & RSN not enabled in CFG.
        /**
         * In order to allow mixed mode for Guest access
         * allow BSS creation/join with no Privacy capability
         * yet advertising WPA IE
         */
        PELOG1(limLog(pMac, LOG1, FL("RSN ie len %d but PRIVACY %d RSN %d\n"), 
               pRSNie->length, privacy, val);)
    }

    if (pRSNie->length)
    {
        if ((pRSNie->rsnIEdata[0] != SIR_MAC_RSN_EID) &&
            (pRSNie->rsnIEdata[0] != SIR_MAC_WPA_EID))
        {
            limLog(pMac, LOGE, FL("RSN/WPA EID %d not [%d || %d]\n"), 
                   pRSNie->rsnIEdata[0], SIR_MAC_RSN_EID, 
                   SIR_MAC_WPA_EID);
            return false;
        }

        // Check validity of RSN IE
        if ((pRSNie->rsnIEdata[0] == SIR_MAC_RSN_EID) &&
#if 0 // Comparison always false
            (pRSNie->rsnIEdata[1] > SIR_MAC_RSN_IE_MAX_LENGTH) ||
#endif
             (pRSNie->rsnIEdata[1] < SIR_MAC_RSN_IE_MIN_LENGTH))
        {
            limLog(pMac, LOGE, FL("RSN IE len %d not [%d,%d]\n"), 
                   pRSNie->rsnIEdata[1], SIR_MAC_RSN_IE_MIN_LENGTH, 
                   SIR_MAC_RSN_IE_MAX_LENGTH);
            return false;
        }

        if (pRSNie->length > pRSNie->rsnIEdata[1] + 2)
        {
            if (pRSNie->rsnIEdata[0] != SIR_MAC_RSN_EID)
            {
                limLog(pMac,
                       LOGE,
                       FL("First byte[%d] in rsnIEdata is not RSN_EID\n"), 
                       pRSNie->rsnIEdata[1]);
                return false;
            }

            limLog(pMac,
                   LOG1,
                   FL("WPA IE is present along with WPA2 IE\n"));
            wpaIndex = 2 + pRSNie->rsnIEdata[1];
        }
        else if ((pRSNie->length == pRSNie->rsnIEdata[1] + 2) &&
                 (pRSNie->rsnIEdata[0] == SIR_MAC_RSN_EID))
        {
            limLog(pMac,
                   LOG1,
                   FL("Only RSN IE is present\n"));

            return true;
        }
        else if ((pRSNie->length == pRSNie->rsnIEdata[1] + 2) &&
                 (pRSNie->rsnIEdata[0] == SIR_MAC_WPA_EID))
            limLog(pMac,
                   LOG1,
                   FL("Only WPA IE is present\n"));

        // Check validity of WPA IE
        val = sirReadU32((tANI_U8 *) &pRSNie->rsnIEdata[wpaIndex + 2]);

        if ((pRSNie->rsnIEdata[wpaIndex] == SIR_MAC_WPA_EID) &&
#if 0 // Comparison always false
            (pRSNie->rsnIEdata[wpaIndex + 1] > SIR_MAC_WPA_IE_MAX_LENGTH) ||
#endif
             ((pRSNie->rsnIEdata[wpaIndex + 1] < SIR_MAC_WPA_IE_MIN_LENGTH) ||
             (SIR_MAC_WPA_OUI != val)))
        {
            limLog(pMac, LOGE,
               FL("WPA IE len %d not [%d,%d] OR data 0x%x not 0x%x\n"),
               pRSNie->rsnIEdata[1], SIR_MAC_RSN_IE_MIN_LENGTH, 
               SIR_MAC_RSN_IE_MAX_LENGTH, val, SIR_MAC_WPA_OUI);

            return false;
        }
    }

    return true;
} /*** end limIsRSNieValidInSmeReqMessage() ***/



#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
/**
 * limIsBssInfoValidInSmeReqMessage()
 *
 *FUNCTION:
 * This function is called to verify if the BSS info
 * received in various SME_REQ messages is valid or not
 *
 *LOGIC:
 * BSS info validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac     Pointer to Global MAC structure
 * @param  pBssInfo Pointer to received Bss Information
 * @return true when BSS info is valid, false otherwise
 */

static tANI_U8
limIsBssInfoValidInSmeReqMessage(tpAniSirGlobal pMac,
                                 tpSirNeighborBssInfo pBssInfo)
{
    tANI_U8 valid = true;

    if ((pBssInfo->bssType != eSIR_INFRASTRUCTURE_MODE) ||
        limIsGroupAddr(pBssInfo->bssId) ||
        !pBssInfo->channelId ||
        !pBssInfo->ssId.length ||
        (pBssInfo->ssId.length > SIR_MAC_MAX_SSID_LENGTH) ||
        !limIsRSNieValidInSmeReqMessage(pMac, &pBssInfo->rsnIE))
    {
        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsBssInfoValidInSmeReqMessage() ***/
#else



/**
 * limIsBssDescrValidInSmeReqMessage()
 *
 *FUNCTION:
 * This function is called to verify if the BSS Descr
 * received in various SME_REQ messages is valid or not
 *
 *LOGIC:
 * BSS Descritipion validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  pBssDescr Pointer to received Bss Descritipion
 * @return true when BSS description is valid, false otherwise
 */

static tANI_U8
limIsBssDescrValidInSmeReqMessage(tpAniSirGlobal pMac,
                                  tpSirBssDescription pBssDescr)
{
    tANI_U8 valid = true;

    if (limIsAddrBC(pBssDescr->bssId) ||
        !pBssDescr->channelId)
    {
        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsBssDescrValidInSmeReqMessage() ***/
#endif



/**
 * limIsSmeStartReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_START_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMsg - Pointer to received SME_START_BSS_REQ message
 * @return true  when received SME_START_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeStartReqValid(tpAniSirGlobal pMac, tANI_U32 *pMsg)
{
    tANI_U8 valid = true;

    if (((tpSirSmeStartReq) pMsg)->length != sizeof(tSirSmeStartReq))
    {
        /**
         * Invalid length in START_REQ message
         * Log error.
         */
        limLog(pMac, LOGW,
               FL("Invalid length %d in eWNI_SME_START_REQ\n"),
               ((tpSirSmeStartReq) pMsg)->length);

        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsSmeStartReqValid() ***/



/**
 * limIsSmeStartBssReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_START_BSS_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  pStartBssReq Pointer to received SME_START_BSS_REQ message
 * @return true  when received SME_START_BSS_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeStartBssReqValid(tpAniSirGlobal pMac,
                         tpSirSmeStartBssReq pStartBssReq)
{
    tANI_U8   i = 0;
    tANI_U8 valid = true;

    PELOG1(limLog(pMac, LOG1,
           FL("Parsed START_BSS_REQ fields are bssType=%d, channelId=%d, SSID len=%d, rsnIE len=%d, nwType=%d, rateset len=%d\n"),
           pMac->lim.gpLimStartBssReq->bssType,
           pMac->lim.gpLimStartBssReq->channelId,
           pMac->lim.gpLimStartBssReq->ssId.length,
           pMac->lim.gpLimStartBssReq->rsnIE.length,
           pMac->lim.gpLimStartBssReq->nwType,
           pMac->lim.gpLimStartBssReq->operationalRateSet.numRates);)

    switch (pStartBssReq->bssType)
    {
        case eSIR_INFRASTRUCTURE_MODE:
#if (WNI_POLARIS_FW_PRODUCT == AP)
            /* Check for the AP Role/Station role here and act accordingly. 
             * Currently assuming this as AP and breaks TODO */
                break;
#endif
                /**
             * Should not have received start BSS req with bssType
             * Infrastructure on STA.
             * Log error.
             */
            limLog(pMac, LOGE,
               FL("Invalid bssType %d in eWNI_SME_START_BSS_REQ\n"),
               pStartBssReq->bssType);

            valid = false;
            goto end;

        case eSIR_IBSS_MODE:
            break;

        default:
            /**
             * Should not have received start BSS req with bssType
             * other than Infrastructure/IBSS.
             * Log error
             */
            limLog(pMac, LOGW,
               FL("Invalid bssType %d in eWNI_SME_START_BSS_REQ\n"),
               pStartBssReq->bssType);

            valid = false;
            goto end;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)

    /* Assumed as AP again, need to check the role and change accordingly */
    if (pStartBssReq->bssType == eSIR_INFRASTRUCTURE_MODE)
    {
        if ((pStartBssReq->numSSID == 1) && pStartBssReq->ssId.length &&
            ((pStartBssReq->ssId.length != pStartBssReq->ssIdList[0].length) ||
            ( !palEqualMemory( pMac->hHdd,pStartBssReq->ssId.ssId,
                    pStartBssReq->ssIdList[0].ssId,
                    pStartBssReq->ssId.length) )))
        {
        /**
         * Invalid combination of ssID length
         * and number of SSIDs present.
         * Reject START_BSS_REQ.
         */
            limLog(pMac, LOGW,
           FL("Mismatch in SSID length & numSSID in SME_START_BSS_REQ\n"));

            valid = false;
            goto end;
        }

        if (!pStartBssReq->numSSID ||
            (pStartBssReq->ssId.length && (pStartBssReq->numSSID != 1)))
        {
        /**
         * Invalid combination of ssID length
         * and number of SSIDs present.
         * Reject START_BSS_REQ.
         */
            limLog(pMac, LOGW,
                FL("Mismatch in SSID length[%d] & numSSID[%d] in SME_START_BSS_REQ\n"),
                pStartBssReq->ssId.length, pStartBssReq->numSSID);

            valid = false;
            goto end;
        }
    }
#endif
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
    /* This below code is client specific code. TODO */
    if (pStartBssReq->bssType == eSIR_IBSS_MODE)
    {
        if (!pStartBssReq->ssId.length ||
            (pStartBssReq->ssId.length > SIR_MAC_MAX_SSID_LENGTH))
        {
            // Invalid length for SSID.  
            // Reject START_BSS_REQ
            limLog(pMac, LOGW,
                FL("Invalid SSID length in eWNI_SME_START_BSS_REQ\n"));

            valid = false;
            goto end;
        }
    }
#endif

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
    /* Assumed as AP TODO */
    if (pStartBssReq->bssType == eSIR_INFRASTRUCTURE_MODE)
    {
        tpSirAlternateRadioInfo pRadioInfo;

        pRadioInfo = pStartBssReq->alternateRadioList.alternateRadio;
        for (i = 0; i < pStartBssReq->alternateRadioList.numBss; i++)
        {
            if (limIsGroupAddr(pRadioInfo->bssId))
            {
                // Invalid mate BSSID.
                // Reject START_BSS_REQ
                limLog(pMac, LOGW,
                       FL("Invalid mate BSSID in eWNI_SME_START_BSS_REQ\n"));

                valid = false;
                goto end;
            }
            pRadioInfo += sizeof(tSirAlternateRadioInfo);
        }

        /*
            ** check WDS info length
        **/
        if (pStartBssReq->wdsInfo.wdsLength > ANI_WDS_INFO_MAX_LENGTH)
        {
            PELOGW(limLog(pMac, LOGW, FL("Illegal WDS info length\n"));)
            valid = false;
            goto end;
        }
    }
#endif

    if (!limIsRSNieValidInSmeReqMessage(pMac, &pStartBssReq->rsnIE))
    {
        valid = false;
        goto end;
    }

    if (pStartBssReq->nwType != eSIR_11A_NW_TYPE &&
        pStartBssReq->nwType != eSIR_11B_NW_TYPE &&
        pStartBssReq->nwType != eSIR_11G_NW_TYPE)
    {
        valid = false;
        goto end;
    }

    if (pStartBssReq->nwType == eSIR_11A_NW_TYPE)
    {
        for (i = 0; i < pStartBssReq->operationalRateSet.numRates; i++)
            if (!sirIsArate(pStartBssReq->operationalRateSet.rate[i] & 0x7F))
        {
            // Invalid Operational rates
            // Reject START_BSS_REQ
            limLog(pMac, LOGW,
               FL("Invalid operational rates in eWNI_SME_START_BSS_REQ\n"));
            sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOGW,
                       pStartBssReq->operationalRateSet.rate,
                       pStartBssReq->operationalRateSet.numRates);

            valid = false;
            goto end;
        }
    }
    // check if all the rates in the operatioal rate set are legal 11G rates
    else if (pStartBssReq->nwType == eSIR_11G_NW_TYPE)
    {
        for (i = 0; i < pStartBssReq->operationalRateSet.numRates; i++)
            if (!sirIsGrate(pStartBssReq->operationalRateSet.rate[i] & 0x7F))
        {
            // Invalid Operational rates
            // Reject START_BSS_REQ
            limLog(pMac, LOGW,
               FL("Invalid operational rates in eWNI_SME_START_BSS_REQ\n"));
            sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOGW,
                       pStartBssReq->operationalRateSet.rate,
                       pStartBssReq->operationalRateSet.numRates);

            valid = false;
            goto end;
        }
    }
    else 
    {
        for (i = 0; i < pStartBssReq->operationalRateSet.numRates; i++)
            if (!sirIsBrate(pStartBssReq->operationalRateSet.rate[i] & 0x7F))
        {
            // Invalid Operational rates
            // Reject START_BSS_REQ
            limLog(pMac, LOGW,
               FL("Invalid operational rates in eWNI_SME_START_BSS_REQ\n"));
            sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOGW,
                       pStartBssReq->operationalRateSet.rate,
                       pStartBssReq->operationalRateSet.numRates);

            valid = false;
            goto end;
        }
    }

end:
    return valid;
} /*** end limIsSmeStartBssReqValid() ***/



/**
 * limIsSmeJoinReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_JOIN_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac       Pointer to Global MAC structure
 * @param  pJoinReq   Pointer to received SME_JOIN_REQ message
 * @return true  when received SME_JOIN_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeJoinReqValid(tpAniSirGlobal pMac, tpSirSmeJoinReq pJoinReq)
{
    tANI_U8 valid = true;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    if (pJoinReq->assocType > eSIR_TRANSFERRED)
    {
        /// Received eWNI_SME_JOIN_REQ with invalid assocType
        // Log the event
        limLog(pMac, LOGW,
               FL("received SME_JOIN_REQ with invalid assocType\n"));

        valid = false;
        goto end;
    }
#endif

    if (!limIsRSNieValidInSmeReqMessage(pMac, &pJoinReq->rsnIE))
    {
        valid = false;
        goto end;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
    if (!limIsBssInfoValidInSmeReqMessage(
                     pMac,
                     pJoinReq->neighborBssList.bssList))
#else
    if (!limIsBssDescrValidInSmeReqMessage(pMac,
                                           &pJoinReq->bssDescription))
#endif
    {
        /// Received eWNI_SME_JOIN_REQ with invalid BSS Info
        // Log the event
        limLog(pMac, LOGW,
               FL("received SME_JOIN_REQ with invalid bssInfo\n"));

        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsSmeJoinReqValid() ***/



/**
 * limIsSmeDisassocReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_DISASSOC_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  pDisassocReq Pointer to received SME_DISASSOC_REQ message
 * @return true         When received SME_DISASSOC_REQ is formatted
 *                      correctly
 *         false        otherwise
 */

tANI_U8
limIsSmeDisassocReqValid(tpAniSirGlobal pMac,
                         tpSirSmeDisassocReq pDisassocReq)
{
    if (limIsGroupAddr(pDisassocReq->peerMacAddr) &&
         !limIsAddrBC(pDisassocReq->peerMacAddr))
        return false;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) &&
         ((pDisassocReq->aid < 2) || (pDisassocReq->aid > 2007))) ||
        ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pDisassocReq->aid != 1)))
        return false;
#endif

    return true;
} /*** end limIsSmeDisassocReqValid() ***/



/**
 * limIsSmeDisassocCnfValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_DISASSOC_CNF message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  pDisassocCnf Pointer to received SME_DISASSOC_REQ message
 * @return true         When received SME_DISASSOC_CNF is formatted
 *                      correctly
 *         false        otherwise
 */

tANI_U8
limIsSmeDisassocCnfValid(tpAniSirGlobal pMac,
                         tpSirSmeDisassocCnf pDisassocCnf)
{
    if (limIsGroupAddr(pDisassocCnf->peerMacAddr))
        return false;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) &&
         ((pDisassocCnf->aid < 2) || (pDisassocCnf->aid > 2007))) ||
        ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pDisassocCnf->aid != 1)))
        return false;
#endif
    return true;
} /*** end limIsSmeDisassocCnfValid() ***/



/**
 * limIsSmeDeauthReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_DEAUTH_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac       Pointer to Global MAC structure
 * @param  pDeauthReq Pointer to received SME_DEAUTH_REQ message
 * @return true       When received SME_DEAUTH_REQ is formatted correctly
 *         false      otherwise
 */

tANI_U8
limIsSmeDeauthReqValid(tpAniSirGlobal pMac, tpSirSmeDeauthReq pDeauthReq)
{
    if (limIsGroupAddr(pDeauthReq->peerMacAddr) &&
         !limIsAddrBC(pDeauthReq->peerMacAddr))
        return false;

#if (WNI_POLARIS_FW_PRODUCT == AP)
    if (((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) &&
         ((pDeauthReq->aid < 2) || (pDeauthReq->aid > 2007))) ||
        ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pDeauthReq->aid != 1)))
        return false;
#endif
    return true;
} /*** end limIsSmeDeauthReqValid() ***/



/**
 * limIsSmeScanReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_SCAN_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pScanReq Pointer to received SME_SCAN_REQ message
 * @return true  when received SME_SCAN_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeScanReqValid(tpAniSirGlobal pMac, tpSirSmeScanReq pScanReq)
{
    tANI_U8 valid = true;

    if ((pScanReq->bssType > eSIR_AUTO_MODE) ||
        (limIsGroupAddr(pScanReq->bssId) && !limIsAddrBC(pScanReq->bssId)) ||
        (pScanReq->ssId.length > SIR_MAC_MAX_SSID_LENGTH) ||
        (!(pScanReq->scanType == eSIR_PASSIVE_SCAN || pScanReq->scanType == eSIR_ACTIVE_SCAN)) || 
        (pScanReq->channelList.numChannels > SIR_MAX_NUM_CHANNELS))
    {
        valid = false;
        goto end;
    }

    /*
    ** check min/max channelTime range
    **/

    if ((pScanReq->scanType == eSIR_ACTIVE_SCAN) && 
        (pScanReq->maxChannelTime < pScanReq->minChannelTime))
    {
        PELOGW(limLog(pMac, LOGW, FL("Max Channel Time < Min Channel Time\n"));)
        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsSmeScanReqValid() ***/



/**
 * limIsSmeAuthReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_AUTH_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pAuthReq Pointer to received SME_AUTH_REQ message
 * @return true  when received SME_AUTH_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeAuthReqValid(tpSirSmeAuthReq pAuthReq)
{
    tANI_U8 valid = true;

    if (limIsGroupAddr(pAuthReq->peerMacAddr) ||
        (pAuthReq->authType > eSIR_AUTO_SWITCH) ||
        !pAuthReq->channelNumber)
    {
        valid = false;
        goto end;
    }

end:
    return valid;
} /*** end limIsSmeAuthReqValid() ***/



/**
 * limIsSmeSetContextReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_SET_CONTEXT_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMsg - Pointer to received SME_SET_CONTEXT_REQ message
 * @return true  when received SME_SET_CONTEXT_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeSetContextReqValid(tpAniSirGlobal pMac, tpSirSmeSetContextReq  pSetContextReq)
{
    tANI_U8 i = 0;
    tANI_U8 valid = true;
    tpSirKeys pKey = pSetContextReq->keyMaterial.key;

    if ((pSetContextReq->keyMaterial.edType != eSIR_ED_WEP40) &&
        (pSetContextReq->keyMaterial.edType != eSIR_ED_WEP104) &&
        (pSetContextReq->keyMaterial.edType != eSIR_ED_NONE) &&
        !pSetContextReq->keyMaterial.numKeys)
    {
        /**
         * No keys present in case of TKIP or CCMP
         * Log error.
         */
        limLog(pMac, LOGW,
           FL("No keys present in SME_SETCONTEXT_REQ for edType=%d\n"),
           pSetContextReq->keyMaterial.edType);

        valid = false;
        goto end;
    }

    if (pSetContextReq->keyMaterial.numKeys &&
        (pSetContextReq->keyMaterial.edType == eSIR_ED_NONE))
    {
        /**
         * Keys present in case of no ED policy
         * Log error.
         */
        limLog(pMac, LOGW,
           FL("Keys present in SME_SETCONTEXT_REQ for edType=%d\n"),
           pSetContextReq->keyMaterial.edType);

        valid = false;
        goto end;
    }

    if (pSetContextReq->keyMaterial.edType >= eSIR_ED_NOT_IMPLEMENTED)
    {
        /**
         * Invalid edType in the message
         * Log error.
         */
        limLog(pMac, LOGW,
               FL("Invalid edType=%d in SME_SETCONTEXT_REQ\n"),
               pSetContextReq->keyMaterial.edType);

        valid = false;
        goto end;
    }
    else if (pSetContextReq->keyMaterial.edType > eSIR_ED_NONE)
    {
        tANI_U32 poi;

        if (wlan_cfgGetInt(pMac, WNI_CFG_PRIVACY_ENABLED,
                      &poi) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP,
                   FL("Unable to retrieve POI from CFG\n"));
        }

        if (!poi)
        {
            /**
             * Privacy is not enabled
             * In order to allow mixed mode for Guest access
             * allow BSS creation/join with no Privacy capability
             * yet advertising WPA IE
             */
            PELOG1(limLog(pMac, LOG1,
               FL("Privacy is not enabled, yet non-None EDtype=%d in SME_SETCONTEXT_REQ\n"),
               pSetContextReq->keyMaterial.edType);)
        }
    }

    for (i = 0; i < pSetContextReq->keyMaterial.numKeys; i++)
    {
        if (((pSetContextReq->keyMaterial.edType == eSIR_ED_WEP40) &&
             (pKey->keyLength != 5)) ||
            ((pSetContextReq->keyMaterial.edType == eSIR_ED_WEP104) &&
             (pKey->keyLength != 13)) ||
            ((pSetContextReq->keyMaterial.edType == eSIR_ED_TKIP) &&
             (pKey->keyLength != 32)) ||
            ((pSetContextReq->keyMaterial.edType == eSIR_ED_CCMP) &&
             (pKey->keyLength != 16)))
        {
            /**
             * Invalid key length for a given ED type
             * Log error.
             */
            limLog(pMac, LOGW,
               FL("Invalid keyLength =%d for edType=%d in SME_SETCONTEXT_REQ\n"),
               pKey->keyLength, pSetContextReq->keyMaterial.edType);

            valid = false;
            goto end;
        }
        pKey += sizeof(tSirKeys);
    }

end:
    return valid;
} /*** end limIsSmeSetContextReqValid() ***/



/**
 * limIsSmeStopBssReqValid()
 *
 *FUNCTION:
 * This function is called by limProcessSmeReqMessages() upon
 * receiving SME_STOP_BSS_REQ message from application.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMsg - Pointer to received SME_STOP_BSS_REQ message
 * @return true  when received SME_STOP_BSS_REQ is formatted correctly
 *         false otherwise
 */

tANI_U8
limIsSmeStopBssReqValid(tANI_U32 *pMsg)
{
    tANI_U8 valid = true;

    return valid;
} /*** end limIsSmeStopBssReqValid() ***/


/**
 * limGetBssIdFromSmeJoinReqMsg()
 *
 *FUNCTION:
 * This function is called in various places to get BSSID
 * from BSS description/Neighbor BSS Info in the SME_JOIN_REQ/
 * SME_REASSOC_REQ message.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param     pBuf   - Pointer to received SME_JOIN/SME_REASSOC_REQ
 *                     message
 * @return    pBssId - Pointer to BSSID
 */

tANI_U8*
limGetBssIdFromSmeJoinReqMsg(tANI_U8 *pBuf)
{
    if (!pBuf)
        return NULL;

    pBuf += sizeof(tANI_U32); // skip message header

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
    pBuf += sizeof(tSirAssocType); // skip assocType
#endif

    pBuf += limGetU16(pBuf) + sizeof(tANI_U16); // skip RSN IE

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && (WNI_POLARIS_FW_PRODUCT == AP)
    pBuf  += sizeof(tAniBool);            // skip BP indicator
    pBuf  += sizeof(tSirBpIndicatorType); // skip BP indicator type
    pBuf  += sizeof(tANI_U32);                 // skip number of neighbor BSS
#else
    pBuf  += sizeof(tANI_U16);                 // skip length of BSS description
#endif

    return (pBuf);
} /*** end limGetBssIdFromSmeJoinReqMsg() ***/


