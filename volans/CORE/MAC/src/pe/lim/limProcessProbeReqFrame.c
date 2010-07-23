/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessProbeReqFrame.cc contains the code
 * for processing Probe Request Frame.
 * Author:        Chandra Modumudi
 * Date:          02/28/02
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

#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSerDesUtils.h"
#include "parserApi.h"


/**
 * limProcessProbeReqFrame
 *
 *FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Probe Request frame reception.
 *
 *LOGIC:
 * This function processes received Probe Request frame and responds
 * with Probe Response.
 * Only AP or STA in IBSS mode that sent last Beacon will respond to
 * Probe Request.
 *
 *ASSUMPTIONS:
 * 1. AP or STA in IBSS mode that sent last Beacon will always respond
 *    to Probe Request received with broadcast SSID.
 *
 *NOTE:
 * 1. Dunno what to do with Rates received in Probe Request frame
 * 2. Frames with out-of-order fields/IEs are dropped.
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  *pBd   A pointer to Buffer descriptor + associated PDUs
 *
 * @return None
 */



void
limProcessProbeReqFrame(tpAniSirGlobal pMac, tANI_U32 *pBd)
{
    tANI_U8         *pBody;
    tpSirMacMgmtHdr pHdr;
    tANI_U32        frameLen, len = SIR_MAC_MAX_SSID_LENGTH;
    tSirProbeReq    probeReq;
    tAniSSID        ssId;
    tSirMsgQ           msgQ;
    tSirSmeProbeReq    *pSirSmeProbeReq;
    tANI_U32           wpsApEnable=0, tmp;

    // Don't send probe responses if disabled
    if (pMac->lim.gLimProbeRespDisableFlag)
        return;

    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);

    if ( (pMac->lim.gLimSystemRole == eLIM_AP_ROLE) ||
         ( (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
           (SIR_MAC_BD_TO_IBSS_BCN_SENT(pBd)) ) )
    {
        frameLen = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd);

        PELOG3(limLog(pMac, LOG3, FL("Received Probe Request %d bytes from "), frameLen);
        limPrintMacAddr(pMac, pHdr->sa, LOG3);)

        // Get pointer to Probe Request frame body
        pBody = SIR_MAC_BD_TO_MPDUDATA(pBd);

        // Parse Probe Request frame
        if (sirConvertProbeReqFrame2Struct(pMac, pBody, frameLen, &probeReq)==eSIR_FAILURE)
        {
            PELOGW(limLog(pMac, LOGW, FL("Parse error ProbeRequest, length=%d, SA is:"), frameLen);)
            limPrintMacAddr(pMac, pHdr->sa, LOGW);
            pMac->sys.probeError++;
            return;
        }
        else
        {
            if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
                limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );

            wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;
            if ((wpsApEnable) &&
                (probeReq.wscIePresent ==  1) &&
                (probeReq.probeReqWscIeInfo.DevicePasswordID.id == WSC_PASSWD_ID_PUSH_BUTTON)) {
                    // send the probe req to WSM when it is from a PBC station 
                    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeProbeReq, sizeof(tSirSmeProbeReq)))
                    {
                         // Log error
                         limLog(pMac, LOGP,
                                      FL("call to palAllocateMemory failed for eWNI_SME_PROBE_REQ\n"));
                    }
                    msgQ.type = eWNI_SME_PROBE_REQ;
                    msgQ.bodyval = 0;
                    msgQ.bodyptr = pSirSmeProbeReq;
#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
                    sirStoreU16N((tANI_U8*)&pSirSmeProbeReq->messageType, eWNI_SME_PROBE_REQ);
                    sirStoreU16N((tANI_U8*)&pSirSmeProbeReq->length, sizeof(tSirSmeProbeReq));
#else

                    pSirSmeProbeReq->messageType = eWNI_SME_PROBE_REQ;
                    pSirSmeProbeReq->length = sizeof(tSirSmeProbeReq);
#endif
                    palCopyMemory( pMac->hHdd, pSirSmeProbeReq->peerMacAddr, pHdr->sa, sizeof(tSirMacAddr));
                    pSirSmeProbeReq->devicePasswdId = probeReq.probeReqWscIeInfo.DevicePasswordID.id;
                    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));
                    if (limHalMmhPostMsgApi(pMac, &msgQ,  ePROT) != eSIR_SUCCESS){
                        PELOG3(limLog(pMac, LOG3, FL("couldnt send the probe req to wsm "));)
                    }
             }
            
        }


        if (wlan_cfgGetStr(pMac, WNI_CFG_SSID, (tANI_U8 *) &ssId.ssId, (tANI_U32 *) &len) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve SSID\n"));
        ssId.length = (tANI_U8) len;

        // Compare received SSID with current SSID. If they
        // match, reply with Probe Response.
        if (probeReq.ssId.length)
        {
            if (!ssId.length)
                goto multipleSSIDcheck;

            if (palEqualMemory( pMac->hHdd,(tANI_U8 *) &ssId,
                          (tANI_U8 *) &(probeReq.ssId), (tANI_U8) (len + 1)) )
            {
                limSendProbeRspMgmtFrame(pMac, pHdr->sa, &ssId, DPH_USE_MGMT_STAID,
                                         DPH_NON_KEEPALIVE_FRAME);
                
               
                return;
            }
            else
            {
               PELOG3(limLog(pMac, LOG3,
                   FL("Ignoring ProbeReq frame with unmatched SSID received from "));
                limPrintMacAddr(pMac, pHdr->sa, LOG3);)
                pMac->sys.probeBadSsid++;
            }
        }
        else
        {
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
            tANI_U32    cfg;

            if (wlan_cfgGetInt(pMac, WNI_CFG_SEND_SINGLE_SSID_ALWAYS, &cfg)
                != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrieve SEND_SSID_IN_PR\n"));

            if (!ssId.length &&
                (pMac->lim.gpLimStartBssReq->numSSID == 1) &&
                cfg)
            {
                PELOG2(limLog(pMac, LOG2, FL("Sending ProbeRsp with suppressed SSID to"));
                limPrintMacAddr(pMac, pHdr->sa, LOG2);)

                limSendProbeRspMgmtFrame( pMac, pHdr->sa,
                   (tAniSSID *) pMac->lim.gpLimStartBssReq->ssIdList,
                   DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME);
            }
            else
#endif
            {
                // Broadcast SSID in the Probe Request.
                // Reply with SSID we're configured with.
                limSendProbeRspMgmtFrame(pMac, pHdr->sa, &ssId,
                                         DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME);
            }
            return;
        }

multipleSSIDcheck:
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        if (!pMac->lim.gpLimStartBssReq->ssId.length)
        {
            tANI_U8     i;

            // Multiple SSIDs/Suppressed SSID is enabled.
            for (i = 0; i < pMac->lim.gpLimStartBssReq->numSSID; i++)
            {
                if (palEqualMemory( pMac->hHdd,
                       (tANI_U8 *) &pMac->lim.gpLimStartBssReq->ssIdList[i],
                       (tANI_U8 *) &probeReq.ssId,
                       (tANI_U8) pMac->lim.gpLimStartBssReq->ssIdList[i].length + 1))
                {
                    limSendProbeRspMgmtFrame( pMac, pHdr->sa,
                           (tAniSSID *) &pMac->lim.gpLimStartBssReq->ssIdList[i],
                           DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME);
                    return;
                }
            }

            if (i == pMac->lim.gpLimStartBssReq->numSSID)
            {
                // Local SSID does not match with received one
                // Ignore received Probe Request frame
               PELOG3(limLog(pMac, LOG3,
                   FL("Ignoring ProbeReq frame with unmatched SSID received from "));
                limPrintMacAddr(pMac, pHdr->sa, LOG3);)
                pMac->sys.probeBadSsid++;
            }
        }
        else
#endif
        {
           PELOG3(limLog(pMac, LOG3,
               FL("Ignoring ProbeReq frame with unmatched SSID received from "));
            limPrintMacAddr(pMac, pHdr->sa, LOG3);)
            pMac->sys.probeBadSsid++;
        }


    }
    else
    {
        // Ignore received Probe Request frame
        PELOG3(limLog(pMac, LOG3, FL("Ignoring Probe Request frame received from "));
        limPrintMacAddr(pMac, pHdr->sa, LOG3);)
        pMac->sys.probeIgnore++;
        return;
    }
} /*** end limProcessProbeReqFrame() ***/
