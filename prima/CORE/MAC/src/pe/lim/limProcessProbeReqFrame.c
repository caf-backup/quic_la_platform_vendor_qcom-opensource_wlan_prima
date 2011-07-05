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
#include "limSession.h"

#if defined WLAN_FEATURE_P2P
void
limProcessP2PProbeReq(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo, tpPESession psessionEntry);
#endif

#ifdef WLAN_SOFTAP_FEATURE
void

limSendSmeProbeReqInd(tpAniSirGlobal pMac,
                      tSirMacAddr peerMacAddr,
                      tANI_U8 *pProbeReqIE,
                      tANI_U32 ProbeReqIELen,
                      tpPESession psessionEntry);
                      
/**
 * limGetWPSPBCSessions
 *
 *FUNCTION:
 * This function is called to query the WPS PBC overlap
 *
 *LOGIC:
 * This function check WPS PBC probe request link list for PBC overlap 
 *
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  addr   A pointer to probe request source MAC addresss
 * @param  uuid_e A pointer to UUIDE element of WPS IE in WPS PBC probe request  
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

void limGetWPSPBCSessions(tpAniSirGlobal pMac,
				     tANI_U8 *addr, tANI_U8 *uuid_e, eWPSPBCOverlap *overlap, tpPESession psessionEntry)
{
	int count = 0;
	tSirWPSPBCSession *pbc;
    tANI_TIMESTAMP curTime;

    curTime = (tANI_TIMESTAMP)(palGetTickCount(pMac->hHdd) / PAL_TICKS_PER_SECOND);

    palFillMemory( pMac->hHdd, (tANI_U8 *)addr, sizeof(tSirMacAddr), 0);
    palFillMemory( pMac->hHdd, (tANI_U8 *)uuid_e, SIR_WPS_UUID_LEN, 0);

    for (pbc = psessionEntry->pAPWPSPBCSession; pbc; pbc = pbc->next) {

        if (curTime > pbc->timestamp + SIR_WPS_PBC_WALK_TIME)
            break;

        count++;
        if(count > 1)
            break;
            
        palCopyMemory(pMac->hHdd, (tANI_U8 *)addr, (tANI_U8 *)pbc->addr, sizeof(tSirMacAddr));
        palCopyMemory(pMac->hHdd, (tANI_U8 *)uuid_e, (tANI_U8 *)pbc->uuid_e, SIR_WPS_UUID_LEN);                
	}

	if (count > 1)
    {
        *overlap = eSAP_WPSPBC_OVERLAP_IN120S;    // Overlap  
    }
    else if(count == 0)
    {
        *overlap = eSAP_WPSPBC_NO_WPSPBC_PROBE_REQ_IN120S;    // no WPS probe request in 120 second    
    } else
    {
         *overlap = eSAP_WPSPBC_ONE_WPSPBC_PROBE_REQ_IN120S;   // One WPS probe request in 120 second
    }

    PELOGE(limLog(pMac, LOGE, FL("overlap = %d\n"), *overlap);)
    PELOGE(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOGE, addr, sizeof(tSirMacAddr));)
    PELOGE(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOGE, uuid_e, SIR_WPS_UUID_LEN);)

	return;
}

/**
 * limRemoveTimeoutPBCsessions
 *
 *FUNCTION:
 * This function is called to remove the WPS PBC probe request entires from specific entry to end.
 *
 *LOGIC:
 *
 * 
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  pbc    The beginning entry in WPS PBC probe request link list
 *
 * @return None
 */
static void limRemoveTimeoutPBCsessions(tpAniSirGlobal pMac, tSirWPSPBCSession *pbc)
{
	tSirWPSPBCSession *prev;

	while (pbc) {
		prev = pbc;
		pbc = pbc->next;

        PELOG4(limLog(pMac, LOG4, FL("WPS PBC sessions remove\n"));)
        PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, prev->addr, sizeof(tSirMacAddr));)
        PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, prev->uuid_e, SIR_WPS_UUID_LEN);)
        
        palFreeMemory(pMac->hHdd, prev);
	}
}

void limRemovePBCSessions(tpAniSirGlobal pMac, tSirMacAddr pRemoveMac,tpPESession psessionEntry)
{
    tSirWPSPBCSession *pbc, *prev = NULL;
    prev = pbc = psessionEntry->pAPWPSPBCSession;

    while (pbc) {
        if (palEqualMemory(pMac->hHdd, (tANI_U8 *)pbc->addr, 
	     (tANI_U8 *)pRemoveMac, sizeof(tSirMacAddr))) {
          prev->next = pbc->next;
          if (pbc == psessionEntry->pAPWPSPBCSession)
            psessionEntry->pAPWPSPBCSession = pbc->next;
            palFreeMemory(pMac->hHdd, pbc);
            return;
        }
        prev = pbc;
        pbc = pbc->next;
    }

}

/**
 * limUpdatePBCSessionEntry
 *
 *FUNCTION:
 * This function is called when probe request with WPS PBC IE is received
 *
 *LOGIC:
 * This function add the WPS PBC probe request in the WPS PBC probe request link list 
 * The link list is in decreased time order of probe request that is received.
 * The entry that is more than 120 second is removed.
 * 
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  addr   A pointer to probe request source MAC addresss
 * @param  uuid_e A pointer to UUIDE element of WPS IE 
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

static void limUpdatePBCSessionEntry(tpAniSirGlobal pMac,
					  tANI_U8 *addr, tANI_U8 *uuid_e, tpPESession psessionEntry)
{
    tSirWPSPBCSession *pbc, *prev = NULL;

    tANI_TIMESTAMP curTime;

    curTime = (tANI_TIMESTAMP)(palGetTickCount(pMac->hHdd) / PAL_TICKS_PER_SECOND);
            
    PELOG4(limLog(pMac, LOG4, FL("Receive WPS probe reques curTime=%d\n"), curTime);)
    PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, addr, sizeof(tSirMacAddr));)
    PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, uuid_e, SIR_WPS_UUID_LEN);)

    pbc = psessionEntry->pAPWPSPBCSession;

    while (pbc) {
        if (palEqualMemory(pMac->hHdd, (tANI_U8 *)pbc->addr, (tANI_U8 *)addr, sizeof(tSirMacAddr)) &&
            palEqualMemory(pMac->hHdd, (tANI_U8 *)pbc->uuid_e, (tANI_U8 *)uuid_e, SIR_WPS_UUID_LEN)) {
            if (prev)
                prev->next = pbc->next;
            else
                psessionEntry->pAPWPSPBCSession = pbc->next;
            break;
        }
        prev = pbc;
        pbc = pbc->next;
    }

    if (!pbc) {
        if (eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd,
            (void **) &pbc, sizeof(tSirWPSPBCSession)))
        {
            PELOGE(limLog(pMac, LOGE, FL("memory allocate failed!\n"));)
        }
        palCopyMemory(pMac->hHdd, (tANI_U8 *)pbc->addr, (tANI_U8 *)addr, sizeof(tSirMacAddr));
    		
        if (uuid_e)
            palCopyMemory(pMac->hHdd, (tANI_U8 *)pbc->uuid_e, (tANI_U8 *)uuid_e, SIR_WPS_UUID_LEN);
    }
    
    pbc->next = psessionEntry->pAPWPSPBCSession;
    psessionEntry->pAPWPSPBCSession = pbc;
    pbc->timestamp = curTime;
    
	/* remove entries that have timed out */
    prev = pbc;
    pbc = pbc->next;

    while (pbc) {
        if (curTime > pbc->timestamp + SIR_WPS_PBC_WALK_TIME) {
            prev->next = NULL;
            limRemoveTimeoutPBCsessions(pMac, pbc);
           break;
        }
        prev = pbc;
        pbc = pbc->next;
    }
}
#if 0
/**
 * limWPSPBCTimeout
 *
 *FUNCTION:
 * This function is called when WPS PBC enrtries clean up timer is expired
 *
 *LOGIC:
 * This function remove all the entryies that more than 120 second old
 *
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

void limWPSPBCTimeout(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
    tANI_TIMESTAMP curTime;
    tSirWPSPBCSession *pbc, *prev = NULL;

    curTime = (tANI_TIMESTAMP)(palGetTickCount(pMac->hHdd) / PAL_TICKS_PER_SECOND);
    
    PELOG3(limLog(pMac, LOG3, FL("WPS PBC cleanup timeout curTime=%d\n"), curTime);)

    prev = psessionEntry->pAPWPSPBCSession; 
    if(prev)
        pbc = prev->next;
    else
        return;
            
    while (pbc) {
        if (curTime > pbc->timestamp + SIR_WPS_PBC_WALK_TIME) {
            prev->next = NULL;
            limRemoveTimeoutPBCsessions(pMac, pbc);
            break;
        }
        prev = pbc;
        pbc = pbc->next;
    }

    if(prev)
    {
         if (curTime > prev->timestamp + SIR_WPS_PBC_WALK_TIME) {
            psessionEntry->pAPWPSPBCSession = NULL;  
            limRemoveTimeoutPBCsessions(pMac, prev);
         }
    }

}
#endif
/**
 * limWPSPBCClose
 *
 *FUNCTION:
 * This function is called when BSS is closed
 *
 *LOGIC:
 * This function remove all the WPS PBC entries
 *
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

void limWPSPBCClose(tpAniSirGlobal pMac, tpPESession psessionEntry)
{

    limRemoveTimeoutPBCsessions(pMac, psessionEntry->pAPWPSPBCSession);

}
#endif

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
 * @param  *pRxPacketInfo   A pointer to Buffer descriptor + associated PDUs
 *
 * @return None
 */
    
void
limProcessProbeReqFrame(tpAniSirGlobal pMac, tANI_U8 *pRxPacketInfo,tpPESession psessionEntry)
{
    tANI_U8             *pBody;
    tpSirMacMgmtHdr     pHdr;
    tANI_U32            frameLen;
    tSirProbeReq        probeReq;
    tAniSSID            ssId;
    tSirMsgQ            msgQ;
    tSirSmeProbeReq     *pSirSmeProbeReq;
    tANI_U32            wpsApEnable=0, tmp;

	 do{
	    // Don't send probe responses if disabled
	    if (pMac->lim.gLimProbeRespDisableFlag)
	        break;

       pHdr = WDA_GET_RX_MAC_HEADER(pRxPacketInfo);

       if ( (psessionEntry->limSystemRole == eLIM_AP_ROLE) ||
            (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE)||
            (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE)||          
            ( (psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
             (WDA_GET_RX_BEACON_SENT(pRxPacketInfo)) ) )
       {
           frameLen = WDA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

           PELOG3(limLog(pMac, LOG3, FL("Received Probe Request %d bytes from "), frameLen);
           limPrintMacAddr(pMac, pHdr->sa, LOG3);)

           // Get pointer to Probe Request frame body
           pBody = WDA_GET_RX_MPDU_DATA(pRxPacketInfo);

           // Parse Probe Request frame
           if (sirConvertProbeReqFrame2Struct(pMac, pBody, frameLen, &probeReq)==eSIR_FAILURE)
           {
               PELOGW(limLog(pMac, LOGW, FL("Parse error ProbeRequest, length=%d, SA is:"), frameLen);)
               limPrintMacAddr(pMac, pHdr->sa, LOGW);
               pMac->sys.probeError++;
               break;
           }
           else
           {
#ifdef WLAN_SOFTAP_FEATURE            
	            if ((psessionEntry->limSystemRole == eLIM_AP_ROLE))
	            {
	              
	                if ( (psessionEntry->APWPSIEs.SirWPSProbeRspIE.FieldPresent & SIR_WPS_PROBRSP_VER_PRESENT) &&
	                        (probeReq.wscIePresent ==  1) &&
	                        (probeReq.probeReqWscIeInfo.DevicePasswordID.id == WSC_PASSWD_ID_PUSH_BUTTON) &&
	                        (probeReq.probeReqWscIeInfo.UUID_E.present == 1))
	                {
	                    if(psessionEntry->fwdWPSPBCProbeReq)
	                    {                                          
	                        PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, pHdr->sa, sizeof(tSirMacAddr));)                        
	                        PELOG4(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, pBody, frameLen);)
	                                                                                                                                       
	                        limSendSmeProbeReqInd(pMac, pHdr->sa, pBody, frameLen, psessionEntry);                        
	                    } 
	                    else
	                    {                            
	                        limUpdatePBCSessionEntry(pMac,
	                            pHdr->sa, probeReq.probeReqWscIeInfo.UUID_E.uuid, psessionEntry);
	                    }
	                }
	            }
	            else
	            {
#endif                
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
	                        if (limSysProcessMmhMsgApi(pMac, &msgQ,  ePROT) != eSIR_SUCCESS){
	                            PELOG3(limLog(pMac, LOG3, FL("couldnt send the probe req to wsm "));)
	                        }
	                }            
#ifdef WLAN_SOFTAP_FEATURE                   
	            }
#endif
	        }

	        ssId.length = psessionEntry->ssId.length;

	         /* Copy the SSID from sessio entry to local variable */   
	         palCopyMemory( pMac->hHdd, ssId.ssId,
	                   psessionEntry->ssId.ssId,
	                   psessionEntry->ssId.length);

	        // Compare received SSID with current SSID. If they
	        // match, reply with Probe Response.
	        if (probeReq.ssId.length)
	        {
	            if (!ssId.length)
	                goto multipleSSIDcheck;

	            if (palEqualMemory( pMac->hHdd,(tANI_U8 *) &ssId,
	                          (tANI_U8 *) &(probeReq.ssId), (tANI_U8) (ssId.length + 1)) )
	            {
	                limSendProbeRspMgmtFrame(pMac, pHdr->sa, &ssId, DPH_USE_MGMT_STAID,
	                                         DPH_NON_KEEPALIVE_FRAME,psessionEntry);
	                
	               
	                break;
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
	                (psessionEntry->pLimStartBssReq->numSSID == 1) &&
	                cfg)
	            {
	                PELOG2(limLog(pMac, LOG2, FL("Sending ProbeRsp with suppressed SSID to"));
	                limPrintMacAddr(pMac, pHdr->sa, LOG2);)

	                limSendProbeRspMgmtFrame( pMac, pHdr->sa,
	                   (tAniSSID *) psessionEntry->pLimStartBssReq->ssIdList,
	                   DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME,psessionEntry);
	            }
	            else
#endif
	            {
	                // Broadcast SSID in the Probe Request.
	                // Reply with SSID we're configured with.
#ifdef WLAN_SOFTAP_FEATURE
	                //Turn off the SSID length to 0 if hidden SSID feature is present
	                if(psessionEntry->ssidHidden)
	                    ssId.length = 0;
#endif
	                limSendProbeRspMgmtFrame(pMac, pHdr->sa, &ssId,
	                                         DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME,psessionEntry);
	            }
	            break;
	        }

	multipleSSIDcheck:
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
	        if (!psessionEntry->pLimStartBssReq->ssId.length)
	        {
	            tANI_U8     i;

	            // Multiple SSIDs/Suppressed SSID is enabled.
	            for (i = 0; i < psessionEntry->pLimStartBssReq->numSSID; i++)
	            {
	                if (palEqualMemory( pMac->hHdd,
	                       (tANI_U8 *) &psessionEntry->pLimStartBssReq->ssIdList[i],
	                       (tANI_U8 *) &probeReq.ssId,
	                       (tANI_U8) psessionEntry->pLimStartBssReq->ssIdList[i].length + 1))
	                {
	                    limSendProbeRspMgmtFrame( pMac, pHdr->sa,
	                           (tAniSSID *) &psessionEntry->pLimStartBssReq->ssIdList[i],
	                           DPH_USE_MGMT_STAID, DPH_NON_KEEPALIVE_FRAME,psessionEntry);
	                    break;
	                }
	            }

	            if (i == psessionEntry->pLimStartBssReq->numSSID)
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
	        break;
	    }
	}while(0);

	return;
} /*** end limProcessProbeReqFrame() ***/



void
limProcessProbeReqFrame_multiple_BSS(tpAniSirGlobal pMac, tANI_U8 *pBd,  tpPESession psessionEntry)
{
    tANI_U8 i;

    if(psessionEntry != NULL)
    {
#ifdef WLAN_FEATURE_P2P
        if( (psessionEntry->limSystemRole == eLIM_AP_ROLE) 
         && (psessionEntry->pePersona == VOS_P2P_GO_MODE)
          )
        {
            limProcessP2PProbeReq(pMac, pBd, psessionEntry);
        }
        else 
#endif        
            limProcessProbeReqFrame(pMac,pBd,psessionEntry);
         return;
    }

    for(i =0; i < pMac->lim.maxBssId;i++)
    {
        psessionEntry = peFindSessionBySessionId(pMac,i);
        if( (psessionEntry != NULL) )
        {
#ifdef WLAN_FEATURE_P2P
            if( (psessionEntry->limSystemRole == eLIM_AP_ROLE) 
             && (psessionEntry->pePersona == VOS_P2P_GO_MODE)
              )
            {
                limProcessP2PProbeReq(pMac, pBd, psessionEntry);
            }
            else 
#endif        
            if( (psessionEntry->limSystemRole == eLIM_AP_ROLE) || 
                (psessionEntry->limSystemRole == eLIM_STA_IN_IBSS_ROLE) || 
                (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE) ||
                (psessionEntry->limSystemRole == eLIM_BT_AMP_STA_ROLE) 
              )
            {
                limProcessProbeReqFrame(pMac,pBd,psessionEntry);
            }
        }
    }

}

#ifdef WLAN_SOFTAP_FEATURE
/**
 * limSendSmeProbeReqInd()
 *
 *FUNCTION:
 * This function is to send
 *  eWNI_SME_WPS_PBC_PROBE_REQ_IND message to host
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * This function is used for sending  eWNI_SME_WPS_PBC_PROBE_REQ_IND
 * to host.
 *
 * @param peerMacAddr       Indicates the peer MAC addr that the probe request
 *                          is generated.
 * @param pProbeReqIE       pointer to RAW probe request IE
 * @param ProbeReqIELen     The length of probe request IE.
 * @param psessionEntry     A pointer to PE session
 *
 * @return None
 */
void
limSendSmeProbeReqInd(tpAniSirGlobal pMac,
                      tSirMacAddr peerMacAddr,
                      tANI_U8 *pProbeReqIE,
                      tANI_U32 ProbeReqIELen, 
                      tpPESession psessionEntry)
{
    tSirSmeProbeReqInd     *pSirSmeProbeReqInd;
    tSirMsgQ                msgQ;   
        
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeProbeReqInd, sizeof(tSirSmeProbeReqInd)))
    {
        // Log error
        limLog(pMac, LOGP,
            FL("call to palAllocateMemory failed for eWNI_SME_PROBE_REQ\n"));
    }
    
    msgQ.type =  eWNI_SME_WPS_PBC_PROBE_REQ_IND;
    msgQ.bodyval = 0;
    msgQ.bodyptr = pSirSmeProbeReqInd;
    
    pSirSmeProbeReqInd->messageType =  eWNI_SME_WPS_PBC_PROBE_REQ_IND;
    pSirSmeProbeReqInd->length = sizeof(tSirSmeProbeReq);

    palCopyMemory( pMac->hHdd, pSirSmeProbeReqInd->bssId, psessionEntry->bssId, sizeof(tSirMacAddr));
    palCopyMemory( pMac->hHdd, pSirSmeProbeReqInd->WPSPBCProbeReq.peerMacAddr, peerMacAddr, sizeof(tSirMacAddr));

    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));
    pSirSmeProbeReqInd->WPSPBCProbeReq.probeReqIELen = (tANI_U16)ProbeReqIELen;
    palCopyMemory( pMac->hHdd, pSirSmeProbeReqInd->WPSPBCProbeReq.probeReqIE, pProbeReqIE, ProbeReqIELen);
    
    if (limSysProcessMmhMsgApi(pMac, &msgQ,  ePROT) != eSIR_SUCCESS){
                            PELOGE(limLog(pMac, LOGE, FL("couldnt send the probe req to hdd"));)
    } 
        
} /*** end limSendSmeProbeReqInd() ***/
#endif

#if defined WLAN_FEATURE_P2P
void
limProcessP2PProbeReq(tpAniSirGlobal pMac, tANI_U8 *pBd, 
                      tpPESession psessionEntry)
{
    tpSirMacMgmtHdr     pHdr;
    tANI_U32            frameLen;
  
    limLog( pMac, LOG1, "Recieved a probe request frame\n");
  
    pHdr = WDA_GET_RX_MAC_HEADER(pBd);
    frameLen = WDA_GET_RX_PAYLOAD_LEN(pBd);
  
    //send the probe req to SME. 
    limSendSmeMgmtFrameInd( pMac, eSIR_MGMT_FRM_PROBE_REQ,
               (tANI_U8*)pHdr, (frameLen + sizeof(tSirMacMgmtHdr)), 
               psessionEntry->smeSessionId );
}
#endif
