/**
 * \file limSendManagementFrames.c
 *
 * \brief Code for preparing and sending 802.11 Management frames
 *
 *
 * Copyright (C) 2005-2007 Airgo Networks, Incorporated
 *
 *
 */

#include "sirApi.h"
#include "aniGlobal.h"
#include "sirMacProtDef.h"
#include "halDataStruct.h"
#include "cfgApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limSecurityUtils.h"
#include "dot11f.h"
#include "limStaHashApi.h"
#include "schApi.h"
#include "limSendMessages.h"


////////////////////////////////////////////////////////////////////////

/// Get an integral configuration item & check return status; if it
/// fails, return.
#define CFG_LIM_GET_INT_NO_STATUS(nStatus, pMac, nItem, cfg )      \
    (nStatus) = wlan_cfgGetInt( (pMac), (nItem), & (cfg) );             \
    if ( eSIR_SUCCESS != (nStatus) )                               \
    {                                                              \
        limLog( (pMac), LOGP, FL("Failed to retrieve "             \
                                 #nItem " from CFG (%d).\n"),      \
                (nStatus) );                                       \
        return;                                                    \
    }

/// Get an text configuration item & check return status; if it fails,
/// return.
#define CFG_LIM_GET_STR_NO_STATUS(nStatus, pMac, nItem, cfg, nCfg, \
                              nMaxCfg)                             \
    (nCfg) = (nMaxCfg);                                            \
    (nStatus) = wlan_cfgGetStr( (pMac), (nItem), (cfg), & (nCfg) );     \
    if ( eSIR_SUCCESS != (nStatus) )                               \
    {                                                              \
        limLog( (pMac), LOGP, FL("Failed to retrieve "             \
                                 #nItem " from CFG (%d).\n"),      \
                (nStatus) );                                       \
        return;                                                    \
    }

/**
 *
 * \brief This function is called by various LIM modules to prepare the
 * 802.11 frame MAC header
 *
 *
 * \param  pMac Pointer to Global MAC structure
 *
 * \param pBD Pointer to the frame buffer that needs to be populate
 *
 * \param type Type of the frame
 *
 * \param subType Subtype of the frame
 *
 * \return eHalStatus
 *
 *
 * The pFrameBuf argument points to the beginning of the frame buffer to
 * which - a) The 802.11 MAC header is set b) Following this MAC header
 * will be the MGMT frame payload The payload itself is populated by the
 * caller API
 *
 *
 */

tSirRetStatus limPopulateBD( tpAniSirGlobal pMac,
                             tANI_U8* pBD,
                             tANI_U8 type,
                             tANI_U8 subType,
                             tSirMacAddr peerAddr ,tSirMacAddr selfMacAddr)
{
    tSirRetStatus   statusCode = eSIR_SUCCESS;
    tpSirMacMgmtHdr pMacHdr;
    
    /// Prepare MAC management header
    pMacHdr = (tpSirMacMgmtHdr) (pBD);

    // Prepare FC
    pMacHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
    pMacHdr->fc.type    = type;
    pMacHdr->fc.subType = subType;

    // Prepare Address 1
    palCopyMemory( pMac->hHdd,
                   (tANI_U8 *) pMacHdr->da,
                   (tANI_U8 *) peerAddr,
                   sizeof( tSirMacAddr ));

    // Prepare Address 2
    #if 0
    if ((statusCode = wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *) pMacHdr->sa,
                                &cfgLen)) != eSIR_SUCCESS)
    {
        // Could not get STA_ID from CFG. Log error.
        limLog( pMac, LOGP,
                FL("Failed to retrive STA_ID\n"));
        return statusCode;
    }
    #endif// TO SUPPORT BT-AMP
    sirCopyMacAddr(pMacHdr->sa,selfMacAddr);

    // Prepare Address 3
    palCopyMemory( pMac->hHdd,
                   (tANI_U8 *) pMacHdr->bssId,
                   (tANI_U8 *) peerAddr,
                   sizeof( tSirMacAddr ));
    return statusCode;
} /*** end limPopulateBD() ***/

/**
 * \brief limSendProbeReqMgmtFrame
 *
 *
 * \param  pMac Pointer to Global MAC structure
 *
 * \param  pSsid SSID to be sent in Probe Request frame
 *
 * \param  bssid BSSID to be sent in Probe Request frame
 *
 * \param  nProbeDelay probe delay to be used before sending Probe Request
 * frame
 *
 * \param nChannelNum Channel # on which the Probe Request is going out
 *
 * This function is called by various LIM modules to send Probe Request frame
 * during active scan/learn phase.
 * Probe request is sent out in the following scenarios:
 * --heartbeat failure:  session needed
 * --join req:           session needed
 * --foreground scan:    no session
 * --background scan:    no session
 * --schBeaconProcessing:  to get EDCA parameters:  session needed
 *
 *
 */

tSirRetStatus
limSendProbeReqMgmtFrame(tpAniSirGlobal pMac,
                         tSirMacSSid   *pSsid,
                         tSirMacAddr    bssid,
                         tANI_U8        nChannelNum,
                         tSirMacAddr    SelfMacAddr,
                         tANI_U32 dot11mode)
{
    tDot11fProbeRequest pr;
    tANI_U32            nStatus, nBytes, nPayload;
    tSirRetStatus       nSirStatus;
    tANI_U8            *pFrame;
    void               *pPacket;
    eHalStatus          halstatus;
    tANI_U32            wps, addnIEPresent;
    tANI_U32            addnIELen = 0;
    tpPESession         psessionEntry;
    tANI_U8             sessionId;

   #ifndef GEN4_SCAN
    return eSIR_FAILURE;
#endif

#if defined ( ANI_DVT_DEBUG )
  return eSIR_FAILURE;
#endif

	/*
	* session context may or may not be present, when probe request needs to be sent out.
	* following cases exist:
       * --heartbeat failure:  session needed
	* --join req:           session needed
	* --foreground scan:    no session
	* --background scan:    no session
	* --schBeaconProcessing:  to get EDCA parameters:  session needed
	* If session context does not exist, some IEs will be populated from CFGs, 
	* e.g. Supported and Extended rate set IEs
	*/
     psessionEntry = peFindSessionByBssid(pMac,bssid,&sessionId);

     // The scheme here is to fill out a 'tDot11fProbeRequest' structure
    // and then hand it off to 'dot11fPackProbeRequest' (for
    // serialization).  We start by zero-initializing the structure:
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&pr, sizeof( pr ) );

    // & delegating to assorted helpers:
    PopulateDot11fSSID( pMac, pSsid, &pr.SSID );

    PopulateDot11fSuppRates( pMac, nChannelNum, &pr.SuppRates,psessionEntry);
      
    if ( WNI_CFG_DOT11_MODE_11B != dot11mode )
    {
        PopulateDot11fExtSuppRates1( pMac, nChannelNum, &pr.ExtSuppRates );
    }

    pMac->lim.htCapability = IS_DOT11_MODE_HT(dot11mode);

    if (psessionEntry != NULL ) {
    psessionEntry->htCapabality = IS_DOT11_MODE_HT(dot11mode);
       //Include HT Capability IE
       if (psessionEntry->htCapabality)
       {
           PopulateDot11fHTCaps( pMac, &pr.HTCaps );
       }
    } else {
           if (pMac->lim.htCapability)
           {
               PopulateDot11fHTCaps( pMac, &pr.HTCaps );
           }
    }

    // Are we including the WPS Information Element?
    CFG_GET_INT( nSirStatus, pMac, WNI_CFG_WPS_PROBE_REQ_FLAG, wps );

    if ( wps )
    {
        PopulateDot11fWscProbeReq( pMac, &pr.WscProbeReq );
    }

    // That's it-- now we pack it.  First, how much space are we going to
    // need?
    nStatus = dot11fGetPackedProbeRequestSize( pMac, &pr, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Probe Request (0x%08x).\n"), nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fProbeRequest );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Probe Request ("
                               "0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr ); // For the buffer descriptor

    //TODO: If additional IE needs to be added. Add then alloc required buffer.
    if(wlan_cfgGetInt(pMac, WNI_CFG_PROBE_REQ_ADDNIE_FLAG, &addnIEPresent) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_REQ_ADDNIE_FLAG\n"));
        return eSIR_FAILURE;
    }
    
    if(addnIEPresent)
    {
        if(wlan_cfgGetStrLen(pMac, WNI_CFG_PROBE_REQ_ADDNIE_DATA, &addnIELen) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_REQ_ADDNIE_DATA length"));
            return eSIR_FAILURE;
        }

        if((nBytes + addnIELen) <= SIR_MAX_PACKET_SIZE ) 
            nBytes += addnIELen;
       else 
            addnIEPresent = false; //Dont include the IE.     
    }
       

    // Ok-- try to allocate some memory:
    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a Pro"
                               "be Request.\n"), nBytes );
        return eSIR_MEM_ALLOC_FAILED;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_PROBE_REQ, bssid ,SelfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a Probe Request (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return nSirStatus;      // allocated!
    }

    // That done, pack the Probe Request:
    nStatus = dot11fPackProbeRequest( pMac, &pr, pFrame +
                                      sizeof( tSirMacMgmtHdr ),
                                      nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Probe Request (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a P"
                               "robe Request (0x%08x).\n") );
    }


    //TODO: Append any AddnIE if present.
    if( addnIEPresent && addnIELen && addnIELen <= WNI_CFG_PROBE_REQ_ADDNIE_DATA_LEN )
    {
            if(wlan_cfgGetStr(pMac, WNI_CFG_PROBE_REQ_ADDNIE_DATA, pFrame+sizeof(tSirMacMgmtHdr)+nPayload, &addnIELen) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Additional IE request failed while Appending\n"));
                palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                            ( void* ) pFrame, ( void* ) pPacket );
                return eSIR_FAILURE;
            }
    }
 
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("could not send Probe Request frame!\n" ));
        //Pkt will be freed up by the callback
        return eSIR_FAILURE;

    }

    return eSIR_SUCCESS;

} // End limSendProbeReqMgmtFrame.

void
limSendProbeRspMgmtFrame(tpAniSirGlobal pMac,
                         tSirMacAddr    peerMacAddr,
                         tpAniSSID      pSsid,
                         short          nStaId,
                         tANI_U8        nKeepAlive,
                         tpPESession psessionEntry)
{
    tDot11fProbeResponse frm;
    tSirRetStatus        nSirStatus;
    tANI_U32             cfg, nPayload, nBytes, nStatus;
    tpSirMacMgmtHdr      pMacHdr;
    tANI_U8             *pFrame;
    void                *pPacket;
    eHalStatus           halstatus;
    tANI_U32             addnIEPresent;
    tANI_U32             addnIELen=0;
    tANI_U32             wpsApEnable=0, tmp;
  
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)         // We don't answer requests
    {
    return;                     // in this case.
    }


    if(NULL == psessionEntry)
    {
        return;
    }
    
    // I dunno what the story is here, but I'm afraid to change it...
    if ( NULL == psessionEntry->pLimStartBssReq )
    {
        limLog( pMac, LOGE, FL( "No Start BSS information while sendi"
                                "ng probeRsp\n" ) );
        return;
    }

    // Fill out 'frm', after which we'll just hand the struct off to
    // 'dot11fPackProbeResponse'.
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    // Timestamp to be updated by TFP, below.

    // Beacon Interval:
    CFG_LIM_GET_INT_NO_STATUS( nSirStatus, pMac,
                               WNI_CFG_BEACON_INTERVAL, cfg );
    frm.BeaconInterval.interval = ( tANI_U16 ) cfg;

    PopulateDot11fCapabilities( pMac, &frm.Capabilities, psessionEntry );
    PopulateDot11fSSID( pMac, ( tSirMacSSid* )pSsid, &frm.SSID );
    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL,
                             &frm.SuppRates,psessionEntry);

    PopulateDot11fDSParams( pMac, &frm.DSParams, psessionEntry);
    PopulateDot11fIBSSParams( pMac, &frm.IBSSParams );

#ifdef ANI_PRODUCT_TYPE_AP
    PopulateDot11fCFParams( pMac, &frm.Capabilities, &frm.CFParams );
#endif // AP Image

#ifdef WLAN_SOFTAP_FEATURE
    if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
    {
        if(psessionEntry->wps_state != SAP_WPS_DISABLED)
        {
            PopulateDot11fProbeResWPSIEs(pMac, &frm.WscProbeRes, psessionEntry);
        }
    }
    else
    {
#endif
    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
        limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
    
    wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;
    
    if (wpsApEnable)
    {
        PopulateDot11fWscInProbeRes(pMac, &frm.WscProbeRes);
    }

    if (pMac->lim.wscIeInfo.probeRespWscEnrollmentState == eLIM_WSC_ENROLL_BEGIN)
    {
        PopulateDot11fWscRegistrarInfoInProbeRes(pMac, &frm.WscProbeRes);
        pMac->lim.wscIeInfo.probeRespWscEnrollmentState = eLIM_WSC_ENROLL_IN_PROGRESS;
    }

    if (pMac->lim.wscIeInfo.wscEnrollmentState == eLIM_WSC_ENROLL_END)
    {
        DePopulateDot11fWscRegistrarInfoInProbeRes(pMac, &frm.WscProbeRes);
        pMac->lim.wscIeInfo.probeRespWscEnrollmentState = eLIM_WSC_ENROLL_NOOP;
    }
#ifdef WLAN_SOFTAP_FEATURE
    }
#endif

    PopulateDot11fCountry( pMac, &frm.Country );
    PopulateDot11fEDCAParamSet( pMac, &frm.EDCAParamSet );

#ifdef ANI_PRODUCT_TYPE_AP
    if( pMac->lim.gLim11hEnable )
    {
        PopulateDot11fPowerConstraints( pMac, &frm.PowerConstraints );
        PopulateDot11fTPCReport( pMac, &frm.TPCReport );

        // If .11h isenabled & channel switching is not already started and
        // we're in either PRIMARY_ONLY or PRIMARY_AND_SECONDARY state, then
        // populate 802.11h channel switch IE
        if (( pMac->lim.gLimChannelSwitch.switchCount != 0 ) &&
             ( pMac->lim.gLimChannelSwitch.state ==
               eLIM_CHANNEL_SWITCH_PRIMARY_ONLY ||
               pMac->lim.gLimChannelSwitch.state ==
               eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY ) )
        {
            PopulateDot11fChanSwitchAnn( pMac, &frm.ChanSwitchAnn );
            PopulateDot11fExtChanSwitchAnn(pMac, &frm.ExtChanSwitchAnn);
        }
    }
#endif

#ifdef WLAN_SOFTAP_FEATURE
    PopulateDot11fERPInfo( pMac, &frm.ERPInfo, psessionEntry);
#else
    PopulateDot11fERPInfo( pMac, &frm.ERPInfo );
#endif


    // N.B. In earlier implementations, the RSN IE would be placed in
    // the frame here, before the WPA IE, if 'RSN_BEFORE_WPA' was defined.
    PopulateDot11fExtSuppRates( pMac, &frm.ExtSuppRates );

    //Populate HT IEs, when operating in 11n or Taurus modes.
    if ( psessionEntry->htCapabality )
    {
        PopulateDot11fHTCaps( pMac, &frm.HTCaps );
#ifdef WLAN_SOFTAP_FEATURE
        PopulateDot11fHTInfo( pMac, &frm.HTInfo, psessionEntry );
#else
        PopulateDot11fHTInfo( pMac, &frm.HTInfo );
#endif
    }

    PopulateDot11fWPA( pMac, &( psessionEntry->pLimStartBssReq->rsnIE ),
                       &frm.WPA );
    PopulateDot11fRSN( pMac, &( psessionEntry->pLimStartBssReq->rsnIE ),
                       &frm.RSN );

#ifdef WLAN_SOFTAP_FEATURE
    PopulateDot11fWMM( pMac, &frm.WMMInfoAp, &frm.WMMParams, &frm.WMMCaps, psessionEntry );
#else
    PopulateDot11fWMM( pMac, &frm.WMMInfoAp, &frm.WMMParams, &frm.WMMCaps );
#endif

#if defined(FEATURE_WLAN_WAPI)
    PopulateDot11fWAPI( pMac, &( psessionEntry->pLimStartBssReq->rsnIE ),
                       &frm.WAPI );
#endif // defined(FEATURE_WLAN_WAPI)


    nStatus = dot11fGetPackedProbeResponseSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Probe Response (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fProbeResponse );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Probe Response "
                               "(0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );
    
    //TODO: If additional IE needs to be added. Add then alloc required buffer.
    if(wlan_cfgGetInt(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, &addnIEPresent) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG\n"));
        return;
    }
    
    if(addnIEPresent)
    {
        if(wlan_cfgGetStrLen(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, &addnIELen) != eSIR_SUCCESS)
        {
            limLog(pMac, LOGP, FL("Unable to get WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA length"));
            return ;
        }

        if((nBytes + addnIELen) <= SIR_MAX_PACKET_SIZE ) 
            nBytes += addnIELen;
       else 
            addnIEPresent = false; //Dont include the IE.     
    }
       
    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a Pro"
                               "be Response.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_PROBE_RSP, peerMacAddr,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a Probe Response (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;
  
    #if 0
    if ( eSIR_SUCCESS != wlan_cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending a Probe Response.\n") );
        // It seems like this should be fatal...
    }
    #endif //To SUPPORT BT-AMP
    

    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);
    

    // That done, pack the Probe Response:
    nStatus = dot11fPackProbeResponse( pMac, &frm, pFrame + sizeof(tSirMacMgmtHdr),
                                       nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Probe Response (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a P"
                               "robe Response (0x%08x).\n") );
    }

    PELOG3(limLog( pMac, LOG3, FL("Sending Probe Response frame to ") );
    limPrintMacAddr( pMac, peerMacAddr, LOG3 );)

    pMac->sys.probeRespond++;

    //TODO: Append any AddnIE if present.
    if( addnIEPresent && addnIELen && addnIELen <= WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA_LEN )
    {
            if(wlan_cfgGetStr(pMac, WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, pFrame+sizeof(tSirMacMgmtHdr)+nPayload, &addnIELen) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Additional IE request failed while Appending: %x\n"),halstatus);
                palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                            ( void* ) pFrame, ( void* ) pPacket );
                return;
            }
    }
    
    // Queue Probe Response frame in high priority WQ
    halstatus = halTxFrame( ( tHalHandle ) pMac, pPacket,
                            ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_LOW,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Could not send Probe Response.\n") );
        //Pkt will be freed up by the callback
    }

} // End limSendProbeRspMgmtFrame.

void
limSendAddtsReqActionFrame(tpAniSirGlobal    pMac,
                           tSirMacAddr       peerMacAddr,
                           tSirAddtsReqInfo *pAddTS,
                           tpPESession       psessionEntry)
{
    tANI_U16               i;
    tANI_U8               *pFrame;
    tSirRetStatus          nSirStatus;
    tDot11fAddTSRequest    AddTSReq;
    tDot11fWMMAddTSRequest WMMAddTSReq;
    tANI_U32               nPayload, nBytes, nStatus;
    tpSirMacMgmtHdr        pMacHdr;
    void                  *pPacket;
    eHalStatus             halstatus;

    
    if(NULL == psessionEntry)
    {
           return;
    }

    if ( ! pAddTS->wmeTspecPresent )
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&AddTSReq, sizeof( AddTSReq ) );

        AddTSReq.Action.action     = SIR_MAC_QOS_ADD_TS_REQ;
        AddTSReq.DialogToken.token = pAddTS->dialogToken;
        AddTSReq.Category.category = SIR_MAC_ACTION_QOS_MGMT;
        if ( pAddTS->lleTspecPresent )
        {
            PopulateDot11fTSPEC( &pAddTS->tspec, &AddTSReq.TSPEC );
        }
        else
        {
            PopulateDot11fWMMTSPEC( &pAddTS->tspec, &AddTSReq.WMMTSPEC );
        }

        if ( pAddTS->lleTspecPresent )
        {
            AddTSReq.num_WMMTCLAS = 0;
            AddTSReq.num_TCLAS = pAddTS->numTclas;
            for ( i = 0; i < pAddTS->numTclas; ++i)
            {
                PopulateDot11fTCLAS( pMac, &pAddTS->tclasInfo[i],
                                     &AddTSReq.TCLAS[i] );
            }
        }
        else
        {
            AddTSReq.num_TCLAS = 0;
            AddTSReq.num_WMMTCLAS = pAddTS->numTclas;
            for ( i = 0; i < pAddTS->numTclas; ++i)
            {
                PopulateDot11fWMMTCLAS( pMac, &pAddTS->tclasInfo[i],
                                        &AddTSReq.WMMTCLAS[i] );
            }
        }

        if ( pAddTS->tclasProcPresent )
        {
            if ( pAddTS->lleTspecPresent )
            {
                AddTSReq.TCLASSPROC.processing = pAddTS->tclasProc;
                AddTSReq.TCLASSPROC.present    = 1;
            }
            else
            {
                AddTSReq.WMMTCLASPROC.version    = 1;
                AddTSReq.WMMTCLASPROC.processing = pAddTS->tclasProc;
                AddTSReq.WMMTCLASPROC.present    = 1;
            }
        }

        nStatus = dot11fGetPackedAddTSRequestSize( pMac, &AddTSReq, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                                   "or an Add TS Request (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fAddTSRequest );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calculating"
                                   "the packed size for an Add TS Request"
                                   " (0x%08x).\n"), nStatus );
        }
    }
    else
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&WMMAddTSReq, sizeof( WMMAddTSReq ) );

        WMMAddTSReq.Action.action     = SIR_MAC_QOS_ADD_TS_REQ;
        WMMAddTSReq.DialogToken.token = pAddTS->dialogToken;
        WMMAddTSReq.Category.category = SIR_MAC_ACTION_WME;

        // WMM spec 2.2.10 - status code is only filled in for ADDTS response
        WMMAddTSReq.StatusCode.statusCode = 0;

	    PopulateDot11fWMMTSPEC( &pAddTS->tspec, &WMMAddTSReq.WMMTSPEC );

        // fillWmeTspecIE

        nStatus = dot11fGetPackedWMMAddTSRequestSize( pMac, &WMMAddTSReq, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                                   "or a WMM Add TS Request (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fAddTSRequest );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calculating"
                                   "the packed size for a WMM Add TS Requ"
                                   "est (0x%08x).\n"), nStatus );
        }
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for an Ad"
                               "d TS Request.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peerMacAddr,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Add TS Request (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    #if 0
    cfgLen = SIR_MAC_ADDR_LENGTH;
    if ( eSIR_SUCCESS != wlan_cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending an Add TS Request.\n") );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }
    #endif //TO SUPPORT BT-AMP
    
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

    // That done, pack the struct:
    if ( ! pAddTS->wmeTspecPresent )
    {
        nStatus = dot11fPackAddTSRequest( pMac, &AddTSReq,
                                          pFrame + sizeof(tSirMacMgmtHdr),
                                          nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack an Add TS Request "
                                   "(0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;             // allocated!
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "an Add TS Request (0x%08x).\n") );
        }
    }
    else
    {
        nStatus = dot11fPackWMMAddTSRequest( pMac, &WMMAddTSReq,
                                             pFrame + sizeof(tSirMacMgmtHdr),
                                             nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack a WMM Add TS Reque"
                                   "st (0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;            // allocated!
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "a WMM Add TS Request (0x%08x).\n") );
        }
    }

    PELOG3(limLog( pMac, LOG3, FL("Sending an Add TS Request frame to ") );
    limPrintMacAddr( pMac, peerMacAddr, LOG3 );)

    // Queue Addts Response frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL( "*** Could not send an Add TS Request"
                                " (%X) ***\n" ), halstatus );
        //Pkt will be freed up by the callback
    }

} // End limSendAddtsReqActionFrame.

/* Added ANI_PRODUCT_TYPE_CLIENT for BT-AMP Support */
#ifdef ANI_PRODUCT_TYPE_AP 

void
limSendAssocRspMgmtFrame(tpAniSirGlobal pMac,
                         tANI_U16       statusCode,
                         tANI_U16       aid,
                         tSirMacAddr    peerMacAddr,
                         tANI_U8        subType,
                         tpDphHashNode  pSta)
{
    tDot11fAssocResponse frm;
    tANI_U8             *pFrame, *macAddr;
    tpSirMacMgmtHdr      pMacHdr;
    tSirRetStatus        nSirStatus;
    tANI_U8              lleMode = 0, fAddTS, edcaInclude = 0;
    tHalBitVal           qosMode, wmeMode;
    tANI_U32             nPayload, nBytes, nStatus, cfgLen;
    void                *pPacket;
    eHalStatus           halstatus;
    tUpdateBeaconParams beaconParams;
    tANI_U32             wpsApEnable=0, tmp;

     
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    limGetQosMode(pMac, &qosMode);
    limGetWmeMode(pMac, &wmeMode);

    // An Add TS IE is added only if the AP supports it and the requesting
    // STA sent a traffic spec.
    fAddTS = ( qosMode && pSta && pSta->qos.addtsPresent ) ? 1 : 0;

    PopulateDot11fCapabilities( pMac, &frm.Capabilities );

    frm.Status.status = statusCode;

    frm.AID.associd = aid | LIM_AID_MASK;

    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL, &frm.SuppRates );

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
        limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
    
    wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

    if (wpsApEnable)
    {
        PopulateDot11fWscInAssocRes(pMac, &frm.WscAssocRes);
    }
    
    PopulateDot11fExtSuppRates( pMac, &frm.ExtSuppRates );

    if ( NULL != pSta )
    {
        if ( eHAL_SET == qosMode )
        {
            if ( pSta->lleEnabled )
            {
                lleMode = 1;
                if ( ( ! pSta->aniPeer ) || ( ! PROP_CAPABILITY_GET( 11EQOS, pSta->propCapability ) ) )
                {
                    PopulateDot11fEDCAParamSet( pMac, &frm.EDCAParamSet );

//                     FramesToDo:...
//                     if ( fAddTS )
//                     {
//                         tANI_U8 *pAf = pBody;
//                         *pAf++ = SIR_MAC_QOS_ACTION_EID;
//                         tANI_U32 tlen;
//                         status = sirAddtsRspFill(pMac, pAf, statusCode, &pSta->qos.addts, NULL,
//                                                  &tlen, bufLen - frameLen);
//                     } // End if on Add TS.
                }
            } // End if on .11e enabled in 'pSta'.
        } // End if on QOS Mode on.

        if ( ( ! lleMode ) && ( eHAL_SET == wmeMode ) && pSta->wmeEnabled )
        {
            if ( ( ! pSta->aniPeer ) || ( ! PROP_CAPABILITY_GET( WME, pSta->propCapability ) ) )
            {
                PopulateDot11fWMMParams( pMac, &frm.WMMParams );

                if ( pSta->wsmEnabled )
                {
                    PopulateDot11fWMMCaps(&frm.WMMCaps );
                }
            }
        }

        if ( pSta->aniPeer )
        {
            if ( ( lleMode && PROP_CAPABILITY_GET( 11EQOS, pSta->propCapability ) ) ||
                 ( pSta->wmeEnabled && PROP_CAPABILITY_GET( WME, pSta->propCapability ) ) )
            {
                edcaInclude = 1;
            }

        } // End if on Airgo peer.

        if ( pSta->mlmStaContext.htCapability  && 
             pMac->lim.htCapability )
        {
            PopulateDot11fHTCaps( pMac, &frm.HTCaps );
            PopulateDot11fHTInfo( pMac, &frm.HTInfo );
        }
    } // End if on non-NULL 'pSta'.


    if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
        limDecideApProtection(pMac, peerMacAddr, &beaconParams);
    limUpdateShortPreamble(pMac, peerMacAddr, &beaconParams);
    limUpdateShortSlotTime(pMac, peerMacAddr, &beaconParams);

    //Send message to HAL about beacon parameter change.
    if(beaconParams.paramChangeBitmap)
    {
        schSetFixedBeaconFields(pMac,psessionEntry);
        limSendBeaconParams(pMac, &beaconParams, psessionEntry );
    }

    // Allocate a buffer for this frame:
    nStatus = dot11fGetPackedAssocResponseSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to calculate the packed size f"
                               "or an Association Response (0x%08x).\n"),
                nStatus );
        return;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for an Association Re"
                               "sponse (0x%08x).\n"), nStatus );
    }

    nBytes = sizeof( tSirMacMgmtHdr ) + nPayload;

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog(pMac, LOGP, FL("Call to bufAlloc failed for RE/ASSOC RSP.\n"));
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac,
                                pFrame,
                                SIR_MAC_MGMT_FRAME,
                                ( LIM_ASSOC == subType ) ?
                                    SIR_MAC_MGMT_ASSOC_RSP :
                                    SIR_MAC_MGMT_REASSOC_RSP,
                                    peerMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Association Response (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

   
    cfgLen = SIR_MAC_ADDR_LENGTH;
    if ( eSIR_SUCCESS != wlan_cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending an Association Response.\n") );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
  
    nStatus = dot11fPackAssocResponse( pMac, &frm,
                                       pFrame + sizeof( tSirMacMgmtHdr ),
                                       nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack an Association Response (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing an "
                               "Association Response (0x%08x).\n") );
    }

    macAddr = pMacHdr->da;

    if (subType == LIM_ASSOC)
        limLog(pMac, LOG1,
               FL("*** Sending Assoc Resp status %d aid %d to "),
               statusCode, aid);
    else
        limLog(pMac, LOG1,
               FL("*** Sending ReAssoc Resp status %d aid %d to "),
               statusCode, aid);
    limPrintMacAddr(pMac, pMacHdr->da, LOG1);

    /// Queue Association Response frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog(pMac, LOGE,
               FL("*** Could not Send Re/AssocRsp, retCode=%X ***\n"),
               nSirStatus);

        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                  (void *) pFrame, (void *) pPacket );
    }

    // update the ANI peer station count
    //FIXME_PROTECTION : take care of different type of station
    // counter inside this function.
    limUtilCountStaAdd(pMac, pSta, pMac->lim.gLimSystemRole);

} // End limSendAssocRspMgmtFrame.


#endif  // ANI_PRODUCT_TYPE_AP


void
limSendAssocRspMgmtFrame(tpAniSirGlobal pMac,
                         tANI_U16       statusCode,
                         tANI_U16       aid,
                         tSirMacAddr    peerMacAddr,
                         tANI_U8        subType,
                         tpDphHashNode  pSta,tpPESession psessionEntry)
{
    tDot11fAssocResponse frm;
    tANI_U8             *pFrame, *macAddr;
    tpSirMacMgmtHdr      pMacHdr;
    tSirRetStatus        nSirStatus;
    tANI_U8              lleMode = 0, fAddTS, edcaInclude = 0;
    tHalBitVal           qosMode, wmeMode;
    tANI_U32             nPayload, nBytes, nStatus;
    void                *pPacket;
    eHalStatus           halstatus;
    tUpdateBeaconParams beaconParams;
    tANI_U32             wpsApEnable=0, tmp;

    if(NULL == psessionEntry)
    {
              return;
    }

    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    limGetQosMode(pMac, &qosMode);
    limGetWmeMode(pMac, &wmeMode);

    // An Add TS IE is added only if the AP supports it and the requesting
    // STA sent a traffic spec.
    fAddTS = ( qosMode && pSta && pSta->qos.addtsPresent ) ? 1 : 0;

    PopulateDot11fCapabilities( pMac, &frm.Capabilities, psessionEntry );

    frm.Status.status = statusCode;

    frm.AID.associd = aid | LIM_AID_MASK;

    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL, &frm.SuppRates,psessionEntry);

#ifdef WLAN_SOFTAP_FEATURE
    if(psessionEntry->limSystemRole == eLIM_AP_ROLE)
    {
        PopulateDot11fAssocResWPSIEs(pMac, &frm.WscAssocRes, psessionEntry);
    }
    else
    {
#endif
    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_ENABLE, &tmp) != eSIR_SUCCESS)
        limLog(pMac, LOGP,"Failed to cfg get id %d\n", WNI_CFG_WPS_ENABLE );
    
    wpsApEnable = tmp & WNI_CFG_WPS_ENABLE_AP;

    if (wpsApEnable)
    {
        PopulateDot11fWscInAssocRes(pMac, &frm.WscAssocRes);
    }
#ifdef WLAN_SOFTAP_FEATURE    
    }
#endif
    
    PopulateDot11fExtSuppRates( pMac, &frm.ExtSuppRates );

    if ( NULL != pSta )
    {
        if ( eHAL_SET == qosMode )
        {
            if ( pSta->lleEnabled )
            {
                lleMode = 1;
                if ( ( ! pSta->aniPeer ) || ( ! PROP_CAPABILITY_GET( 11EQOS, pSta->propCapability ) ) )
                {
                    PopulateDot11fEDCAParamSet( pMac, &frm.EDCAParamSet );

//                     FramesToDo:...
//                     if ( fAddTS )
//                     {
//                         tANI_U8 *pAf = pBody;
//                         *pAf++ = SIR_MAC_QOS_ACTION_EID;
//                         tANI_U32 tlen;
//                         status = sirAddtsRspFill(pMac, pAf, statusCode, &pSta->qos.addts, NULL,
//                                                  &tlen, bufLen - frameLen);
//                     } // End if on Add TS.
                }
            } // End if on .11e enabled in 'pSta'.
        } // End if on QOS Mode on.

        if ( ( ! lleMode ) && ( eHAL_SET == wmeMode ) && pSta->wmeEnabled )
        {
            if ( ( ! pSta->aniPeer ) || ( ! PROP_CAPABILITY_GET( WME, pSta->propCapability ) ) )
            {

#ifdef WLAN_SOFTAP_FEATURE
                PopulateDot11fWMMParams( pMac, &frm.WMMParams, psessionEntry);
#else
                PopulateDot11fWMMParams( pMac, &frm.WMMParams );
#endif

                if ( pSta->wsmEnabled )
                {
                    PopulateDot11fWMMCaps(&frm.WMMCaps );
                }
            }
        }

        if ( pSta->aniPeer )
        {
            if ( ( lleMode && PROP_CAPABILITY_GET( 11EQOS, pSta->propCapability ) ) ||
                 ( pSta->wmeEnabled && PROP_CAPABILITY_GET( WME, pSta->propCapability ) ) )
            {
                edcaInclude = 1;
            }

        } // End if on Airgo peer.

        if ( pSta->mlmStaContext.htCapability  && 
             psessionEntry->htCapabality )
        {
            PopulateDot11fHTCaps( pMac, &frm.HTCaps );
#ifdef WLAN_SOFTAP_FEATURE
            PopulateDot11fHTInfo( pMac, &frm.HTInfo, psessionEntry );
#else
            PopulateDot11fHTInfo( pMac, &frm.HTInfo );
#endif
        }
    } // End if on non-NULL 'pSta'.


   palZeroMemory( pMac->hHdd, ( tANI_U8* )&beaconParams, sizeof( tUpdateBeaconParams) );

#ifdef WLAN_SOFTAP_FEATURE
    if( psessionEntry->limSystemRole == eLIM_AP_ROLE ){
        if(psessionEntry->gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
        limDecideApProtection(pMac, peerMacAddr, &beaconParams,psessionEntry);
    }	
#endif

    limUpdateShortPreamble(pMac, peerMacAddr, &beaconParams, psessionEntry);
    limUpdateShortSlotTime(pMac, peerMacAddr, &beaconParams, psessionEntry);

	beaconParams.bssIdx = psessionEntry->bssIdx;

    //Send message to HAL about beacon parameter change.
    if(beaconParams.paramChangeBitmap)
    {
        schSetFixedBeaconFields(pMac,psessionEntry);
        limSendBeaconParams(pMac, &beaconParams, psessionEntry );
    }

    // Allocate a buffer for this frame:
    nStatus = dot11fGetPackedAssocResponseSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to calculate the packed size f"
                               "or an Association Response (0x%08x).\n"),
                nStatus );
        return;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for an Association Re"
                               "sponse (0x%08x).\n"), nStatus );
    }

    nBytes = sizeof( tSirMacMgmtHdr ) + nPayload;

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog(pMac, LOGP, FL("Call to bufAlloc failed for RE/ASSOC RSP.\n"));
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac,
                                pFrame,
                                SIR_MAC_MGMT_FRAME,
                                ( LIM_ASSOC == subType ) ?
                                    SIR_MAC_MGMT_ASSOC_RSP :
                                    SIR_MAC_MGMT_REASSOC_RSP,
                                peerMacAddr,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Association Response (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    #if 0
    cfgLen = SIR_MAC_ADDR_LENGTH;
    if ( eSIR_SUCCESS != cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending an Association Response.\n") );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    #endif //TO SUPPORT BT-AMP
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

    nStatus = dot11fPackAssocResponse( pMac, &frm,
                                       pFrame + sizeof( tSirMacMgmtHdr ),
                                       nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack an Association Response (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing an "
                               "Association Response (0x%08x).\n") );
    }

    macAddr = pMacHdr->da;

    if (subType == LIM_ASSOC)
    {
        PELOGE(limLog(pMac, LOGE,
              FL("*** Sending Assoc Resp status %d aid %d to "),statusCode, aid);)
    }    
    else{
        PELOGE(limLog(pMac, LOGE,
               FL("*** Sending ReAssoc Resp status %d aid %d to "),
               statusCode, aid);)
        PELOGE(limPrintMacAddr(pMac, pMacHdr->da, LOGE);)
    }

    /// Queue Association Response frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog(pMac, LOGE,
               FL("*** Could not Send Re/AssocRsp, retCode=%X ***\n"),
               nSirStatus);

        //Pkt will be freed up by the callback
    }

    // update the ANI peer station count
    //FIXME_PROTECTION : take care of different type of station
    // counter inside this function.
    limUtilCountStaAdd(pMac, pSta, psessionEntry->limSystemRole);

} // End limSendAssocRspMgmtFrame.


 
void
limSendAddtsRspActionFrame(tpAniSirGlobal     pMac,
                           tSirMacAddr        peer,
                           tANI_U16           nStatusCode,
                           tSirAddtsReqInfo  *pAddTS,
                           tSirMacScheduleIE *pSchedule,
                           tpPESession        psessionEntry)
{
    tANI_U8                *pFrame;
    tpSirMacMgmtHdr         pMacHdr;
    tDot11fAddTSResponse    AddTSRsp;
    tDot11fWMMAddTSResponse WMMAddTSRsp;
    tSirRetStatus           nSirStatus;
    tANI_U32                i, nBytes, nPayload, nStatus;
    void                   *pPacket;
    eHalStatus              halstatus;

    if(NULL == psessionEntry)
    {
              return;
    }

    if ( ! pAddTS->wmeTspecPresent )
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&AddTSRsp, sizeof( AddTSRsp ) );

        AddTSRsp.Category.category = SIR_MAC_ACTION_QOS_MGMT;
        AddTSRsp.Action.action     = SIR_MAC_QOS_ADD_TS_RSP;
        AddTSRsp.DialogToken.token = pAddTS->dialogToken;
        AddTSRsp.Status.status     = nStatusCode;

        // The TsDelay information element is only filled in for a specific
        // status code:
        if ( eSIR_MAC_TS_NOT_CREATED_STATUS == nStatusCode )
        {
            if ( pAddTS->wsmTspecPresent )
            {
                AddTSRsp.WMMTSDelay.version = 1;
                AddTSRsp.WMMTSDelay.delay   = 10;
                AddTSRsp.WMMTSDelay.present = 1;
            }
            else
            {
                AddTSRsp.TSDelay.delay   = 10;
                AddTSRsp.TSDelay.present = 1;
            }
        }

        if ( pAddTS->wsmTspecPresent )
        {
            PopulateDot11fWMMTSPEC( &pAddTS->tspec, &AddTSRsp.WMMTSPEC );
        }
        else
        {
            PopulateDot11fTSPEC( &pAddTS->tspec, &AddTSRsp.TSPEC );
        }

        if ( pAddTS->wsmTspecPresent )
        {
            AddTSRsp.num_WMMTCLAS = 0;
            AddTSRsp.num_TCLAS = pAddTS->numTclas;
            for ( i = 0; i < AddTSRsp.num_TCLAS; ++i)
            {
                PopulateDot11fTCLAS( pMac, &pAddTS->tclasInfo[i],
                                     &AddTSRsp.TCLAS[i] );
            }
        }
        else
        {
            AddTSRsp.num_TCLAS = 0;
            AddTSRsp.num_WMMTCLAS = pAddTS->numTclas;
            for ( i = 0; i < AddTSRsp.num_WMMTCLAS; ++i)
            {
                PopulateDot11fWMMTCLAS( pMac, &pAddTS->tclasInfo[i],
                                        &AddTSRsp.WMMTCLAS[i] );
            }
        }

        if ( pAddTS->tclasProcPresent )
        {
            if ( pAddTS->wsmTspecPresent )
            {
                AddTSRsp.WMMTCLASPROC.version    = 1;
                AddTSRsp.WMMTCLASPROC.processing = pAddTS->tclasProc;
                AddTSRsp.WMMTCLASPROC.present    = 1;
            }
            else
            {
                AddTSRsp.TCLASSPROC.processing = pAddTS->tclasProc;
                AddTSRsp.TCLASSPROC.present    = 1;
            }
        }

        // schedule element is included only if requested in the tspec and we are
        // using hcca (or both edca and hcca)
        // 11e-D8.0 is inconsistent on whether the schedule element is included
        // based on tspec schedule bit or not. Sec 7.4.2.2. says one thing but
        // pg 46, line 17-18 says something else. So just include it and let the
        // sta figure it out
        if ((pSchedule != NULL) &&
            ((pAddTS->tspec.tsinfo.traffic.accessPolicy == SIR_MAC_ACCESSPOLICY_HCCA) ||
             (pAddTS->tspec.tsinfo.traffic.accessPolicy == SIR_MAC_ACCESSPOLICY_BOTH)))
        {
            if ( pAddTS->wsmTspecPresent )
            {
                PopulateDot11fWMMSchedule( pSchedule, &AddTSRsp.WMMSchedule );
            }
            else
            {
                PopulateDot11fSchedule( pSchedule, &AddTSRsp.Schedule );
            }
        }

        nStatus = dot11fGetPackedAddTSResponseSize( pMac, &AddTSRsp, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed si"
                                   "ze for an Add TS Response (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fAddTSResponse );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calcula"
                                   "tingthe packed size for an Add TS"
                                   " Response (0x%08x).\n"), nStatus );
        }
    }
    else
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&WMMAddTSRsp, sizeof( WMMAddTSRsp ) );

        WMMAddTSRsp.Category.category = SIR_MAC_ACTION_WME;
        WMMAddTSRsp.Action.action     = SIR_MAC_QOS_ADD_TS_RSP;
        WMMAddTSRsp.DialogToken.token = pAddTS->dialogToken;
        WMMAddTSRsp.StatusCode.statusCode = (tANI_U8)nStatusCode;

        PopulateDot11fWMMTSPEC( &pAddTS->tspec, &WMMAddTSRsp.WMMTSPEC );

        nStatus = dot11fGetPackedWMMAddTSResponseSize( pMac, &WMMAddTSRsp, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed si"
                                   "ze for a WMM Add TS Response (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fWMMAddTSResponse );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calcula"
                                   "tingthe packed size for a WMM Add"
                                   "TS Response (0x%08x).\n"), nStatus );
        }
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for an Ad"
                               "d TS Response.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Add TS Response (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

     
    #if 0
    if ( eSIR_SUCCESS != wlan_cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending an Add TS Response.\n") );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    #endif //TO SUPPORT BT-AMP
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

    // That done, pack the struct:
    if ( ! pAddTS->wmeTspecPresent )
    {
        nStatus = dot11fPackAddTSResponse( pMac, &AddTSRsp,
                                           pFrame + sizeof( tSirMacMgmtHdr ),
                                           nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack an Add TS Response "
                                   "(0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "an Add TS Response (0x%08x).\n") );
        }
    }
    else
    {
        nStatus = dot11fPackWMMAddTSResponse( pMac, &WMMAddTSRsp,
                                              pFrame + sizeof( tSirMacMgmtHdr ),
                                              nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack a WMM Add TS Response "
                                   "(0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "a WMM Add TS Response (0x%08x).\n") );
        }
    }

    PELOG1(limLog( pMac, LOG1, FL("Sending an Add TS Response (status %d) to "),
            nStatusCode );
    limPrintMacAddr( pMac, pMacHdr->da, LOG1 );)

    // Queue the frame in high priority WQ:
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send Add TS Response (%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
    }

} // End limSendAddtsRspActionFrame.

void
limSendDeltsReqActionFrame(tpAniSirGlobal  pMac,
                           tSirMacAddr  peer,
                           tANI_U8  wmmTspecPresent,
                           tSirMacTSInfo  *pTsinfo,
                           tSirMacTspecIE  *pTspecIe,
                           tpPESession psessionEntry)
{
    tANI_U8         *pFrame;
    tpSirMacMgmtHdr  pMacHdr;
    tDot11fDelTS     DelTS;
    tDot11fWMMDelTS  WMMDelTS;
    tSirRetStatus    nSirStatus;
    tANI_U32         nBytes, nPayload, nStatus;
    void            *pPacket;
    eHalStatus       halstatus;

    if(NULL == psessionEntry)
    {
              return;
    }

    if ( ! wmmTspecPresent )
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&DelTS, sizeof( DelTS ) );

        DelTS.Category.category = SIR_MAC_ACTION_QOS_MGMT;
        DelTS.Action.action     = SIR_MAC_QOS_DEL_TS_REQ;
        PopulateDot11fTSInfo( pTsinfo, &DelTS.TSInfo );

        nStatus = dot11fGetPackedDelTSSize( pMac, &DelTS, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed si"
                                   "ze for a Del TS (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fDelTS );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calcula"
                                   "ting the packed size for a Del TS"
                                   " (0x%08x).\n"), nStatus );
        }
    }
    else
    {
        palZeroMemory( pMac->hHdd, ( tANI_U8* )&WMMDelTS, sizeof( WMMDelTS ) );

        WMMDelTS.Category.category = SIR_MAC_ACTION_WME;
        WMMDelTS.Action.action     = SIR_MAC_QOS_DEL_TS_REQ;
        WMMDelTS.DialogToken.token = 0;
        WMMDelTS.StatusCode.statusCode = 0;
        PopulateDot11fWMMTSPEC( pTspecIe, &WMMDelTS.WMMTSPEC );
        nStatus = dot11fGetPackedWMMDelTSSize( pMac, &WMMDelTS, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGP, FL("Failed to calculate the packed si"
                                   "ze for a WMM Del TS (0x%08x).\n"),
                    nStatus );
            // We'll fall back on the worst case scenario:
            nPayload = sizeof( tDot11fDelTS );
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while calcula"
                                   "ting the packed size for a WMM De"
                                   "l TS (0x%08x).\n"), nStatus );
        }
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for an Ad"
                               "d TS Response.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer,
                                psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Add TS Response (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    #if 0

    cfgLen = SIR_MAC_ADDR_LENGTH;
    if ( eSIR_SUCCESS != wlan_cfgGetStr( pMac, WNI_CFG_BSSID,
                                    ( tANI_U8* )pMacHdr->bssId, &cfgLen ) )
    {
        limLog( pMac, LOGP, FL("Failed to retrieve WNI_CFG_BSSID whil"
                               "e sending an Add TS Response.\n") );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    #endif //TO SUPPORT BT-AMP
    sirCopyMacAddr(pMacHdr->bssId, psessionEntry->bssId);
    
    // That done, pack the struct:
    if ( !wmmTspecPresent )
    {
        nStatus = dot11fPackDelTS( pMac, &DelTS,
                                   pFrame + sizeof( tSirMacMgmtHdr ),
                                   nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack a Del TS frame (0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;             // allocated!
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "a Del TS frame (0x%08x).\n") );
        }
    }
    else
    {
        nStatus = dot11fPackWMMDelTS( pMac, &WMMDelTS,
                                      pFrame + sizeof( tSirMacMgmtHdr ),
                                      nPayload, &nPayload );
        if ( DOT11F_FAILED( nStatus ) )
        {
            limLog( pMac, LOGE, FL("Failed to pack a WMM Del TS frame (0x%08x).\n"),
                    nStatus );
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
            return;             // allocated!
        }
        else if ( DOT11F_WARNED( nStatus ) )
        {
            limLog( pMac, LOGW, FL("There were warnings while packing"
                                   "a WMM Del TS frame (0x%08x).\n") );
        }
    }

    PELOG1(limLog(pMac, LOG1, FL("Sending DELTS REQ (size %d) to "), nBytes);
    limPrintMacAddr(pMac, pMacHdr->da, LOG1);)

    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send Del TS (%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
    }

} // End limSendDeltsReqActionFrame.

void
limSendAssocReqMgmtFrame(tpAniSirGlobal   pMac,
                         tLimMlmAssocReq *pMlmAssocReq,
                         tpPESession psessionEntry)
{
    tDot11fAssocRequest frm;
    tANI_U16            caps;
    tANI_U8            *pFrame;
    tSirRetStatus       nSirStatus;
    tLimMlmAssocCnf     mlmAssocCnf;
    tANI_U32            nBytes, nPayload, nStatus, nCfg, nWps;
    tANI_U8             fQosEnabled, fWmeEnabled, fWsmEnabled;
    void               *pPacket;
    eHalStatus          halstatus;

     if(NULL == psessionEntry)
    {
        return;
    }
    
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    caps = pMlmAssocReq->capabilityInfo;
    if ( PROP_CAPABILITY_GET( 11EQOS, psessionEntry->limCurrentBssPropCap ) )
        ((tSirMacCapabilityInfo *) &caps)->qos = 0;
    swapBitField16(caps, ( tANI_U16* )&frm.Capabilities );

    CFG_LIM_GET_INT_NO_STATUS( nSirStatus, pMac, WNI_CFG_WPS_ASSOC_METHOD,
                               nWps );
    if ( nWps )
    {
        // Ok-- here's what's going on.  *If* 'nWps' is non-zero, then
        // we're going to associate for purposes of WPS enrollment.
        // However, we don't want to leave that flag on forever--
        // otherwise we'll end up continuing as if we want to enroll
        // when we just want to associate!

        cfgSetInt( pMac, WNI_CFG_WPS_ASSOC_METHOD, 0 ); // Return value
                                                        // intentionally
                                                        // ignored...
    }

    frm.ListenInterval.interval = pMlmAssocReq->listenInterval;
    PopulateDot11fSSID2( pMac, &frm.SSID );
    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL,
                             &frm.SuppRates,psessionEntry);

    fQosEnabled = ( pMac->lim.gLimQosEnabled) &&
        SIR_MAC_GET_QOS( psessionEntry->limCurrentBssCaps );

    fWmeEnabled = ( pMac->lim.gLimWmeEnabled ) &&
        LIM_BSS_CAPS_GET( WME, psessionEntry->limCurrentBssQosCaps );

    // We prefer .11e asociations:
    if ( fQosEnabled ) fWmeEnabled = false;

    fWsmEnabled = ( pMac->lim.gLimWsmEnabled ) && fWmeEnabled &&
        LIM_BSS_CAPS_GET( WSM, psessionEntry->limCurrentBssQosCaps );


    if ( pMac->lim.gLim11hEnable  &&
         psessionEntry->pLimJoinReq->spectrumMgtIndicator == eSIR_TRUE )
    {
        PopulateDot11fPowerCaps( pMac, &frm.PowerCaps, LIM_ASSOC,psessionEntry);
        PopulateDot11fSuppChannels( pMac, &frm.SuppChannels, LIM_ASSOC,psessionEntry);
    }

    if ( fQosEnabled &&
         ( ! PROP_CAPABILITY_GET(11EQOS, psessionEntry->limCurrentBssPropCap)))
                PopulateDot11fQOSCapsStation( pMac, &frm.QOSCapsStation );

    PopulateDot11fExtSuppRates( pMac, &frm.ExtSuppRates );

    // The join request *should* contain zero or one of the WPA and RSN
    // IEs.  The payload send along with the request is a
    // 'tSirSmeJoinReq'; the IE portion is held inside a 'tSirRSNie':

    //     typedef struct sSirRSNie
    //     {
    //         tANI_U16       length;
    //         tANI_U8        rsnIEdata[SIR_MAC_MAX_IE_LENGTH+2];
    //     } tSirRSNie, *tpSirRSNie;

    // So, we should be able to make the following two calls harmlessly,
    // since they do nothing if they don't find the given IE in the
    // bytestream with which they're provided.

    // The net effect of this will be to faithfully transmit whatever
    // security IE is in the join request.

    // *However*, if we're associating for the purpose of WPS
    // enrollment, and we've been configured to indicate that by
    // eliding the WPA or RSN IE, we just skip this:
    if ( 0 == ( 0x0002 & nWps ) )
    {
        PopulateDot11fRSNOpaque( pMac, &( psessionEntry->pLimJoinReq->rsnIE ),
                                 &frm.RSNOpaque );
        PopulateDot11fWPAOpaque( pMac, &( psessionEntry->pLimJoinReq->rsnIE ),
                                 &frm.WPAOpaque );
#if defined(FEATURE_WLAN_WAPI)
        PopulateDot11fWAPIOpaque( pMac, &( psessionEntry->pLimJoinReq->rsnIE ),
                                 &frm.WAPIOpaque );
#endif // defined(FEATURE_WLAN_WAPI)
    }

    // include WME EDCA IE as well
    if ( fWmeEnabled )
    {
        if ( ! PROP_CAPABILITY_GET( WME, psessionEntry->limCurrentBssPropCap ) )
        {
            PopulateDot11fWMMInfoStation( pMac, &frm.WMMInfoStation );
        }

        if ( fWsmEnabled &&
             ( ! PROP_CAPABILITY_GET(WSM, psessionEntry->limCurrentBssPropCap )))
        {
            PopulateDot11fWMMCaps( &frm.WMMCaps );
        }
    }

    //Populate HT IEs, when operating in 11n or Taurus modes AND
    //when AP is also operating in 11n mode.
    if ( psessionEntry->htCapabality &&
         pMac->lim.htCapabilityPresentInBeacon)
    {
        PopulateDot11fHTCaps( pMac, &frm.HTCaps );
#ifdef DISABLE_GF_FOR_INTEROP

         /*
         * To resolve the interop problem with Broadcom AP, 
         * where TQ STA could not pass traffic with GF enabled,
         * TQ STA will do Greenfield only with TQ AP, for 
         * everybody else it will be turned off.
        */

        if( (psessionEntry->pLimJoinReq != NULL) && (!psessionEntry->pLimJoinReq->bssDescription.aniIndicator))
            {
                limLog( pMac, LOGE, FL("Sending Assoc Req to Non-TQ AP, Turning off Greenfield"));
                frm.HTCaps.greenField = WNI_CFG_GREENFIELD_CAPABILITY_DISABLE;
            }
#endif

    }

    // If we're associating for the purpose of WPS
    // enrollment, and we've been configured to indicate that by
    // including the WPS IE, populate it:
    if ( 0 != ( 0x0001 & nWps ) )
    {
        PopulateDot11fWscAssocReq( pMac, &frm.WscAssocReq );
    }

    nStatus = dot11fGetPackedAssocRequestSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or an Association Request (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fAssocRequest );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for an Association Re "
                               "quest(0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for an As"
                               "sociation Request.\n"), nBytes );

        psessionEntry->limMlmState = psessionEntry->limPrevMlmState;
	 MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

        
        /* Update PE session id*/
        mlmAssocCnf.sessionId = psessionEntry->peSessionId;

        mlmAssocCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;

        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );

        limPostSmeMessage( pMac, LIM_MLM_ASSOC_CNF,
                          ( tANI_U32* ) &mlmAssocCnf);

        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    nCfg = sizeof( tSirMacAddr );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ASSOC_REQ, psessionEntry->bssId,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Association Request (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;
    }


    // That done, pack the Probe Request:
    nStatus = dot11fPackAssocRequest( pMac, &frm, pFrame +
                                      sizeof(tSirMacMgmtHdr),
                                      nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Probe Response (0x%0"
                               "8x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a P"
                               "robe Response (0x%08x).\n") );
    }

    PELOG1(limLog( pMac, LOG1, FL("*** Sending Association Request length %d"
                           "to \n"),
            nBytes );)
 //   limPrintMacAddr( pMac, bssid, LOG1 );

    if( psessionEntry->assocReq != NULL )
    {
        palFreeMemory(pMac->hHdd, psessionEntry->assocReq);
        psessionEntry->assocReq = NULL;
    }
    if( (palAllocateMemory(pMac->hHdd, (void**)&psessionEntry->assocReq, nPayload)) != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Unable to allocate memory to store assoc request"));)
     }
     else
     {
        //Store the Assoc request. This is sent to csr/hdd in join cnf response. 
        palCopyMemory(pMac->hHdd, psessionEntry->assocReq, pFrame + sizeof(tSirMacMgmtHdr), nPayload);
        psessionEntry->assocReqLen = nPayload;
     }


    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send Association Request (%X)!\n"),
                halstatus );
        //Pkt will be freed up by the callback
        return;
    }

    // Free up buffer allocated for mlmAssocReq
    palFreeMemory( pMac->hHdd, ( tANI_U8* ) pMlmAssocReq );

} // End limSendAssocReqMgmtFrame

void
limSendReassocReqMgmtFrame(tpAniSirGlobal     pMac,
                           tLimMlmReassocReq *pMlmReassocReq,tpPESession psessionEntry)
{
    tDot11fReAssocRequest frm;
    tANI_U16              caps;
    tANI_U8              *pFrame;
    tSirRetStatus         nSirStatus;
    tANI_U32              nBytes, nPayload, nStatus, nWps;
    tANI_U8               fQosEnabled, fWmeEnabled, fWsmEnabled;
    void                 *pPacket;
    eHalStatus            halstatus;

    if(NULL == psessionEntry)
    {
        return;
    }
    
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    caps = pMlmReassocReq->capabilityInfo;
    if (PROP_CAPABILITY_GET(11EQOS, psessionEntry->limReassocBssPropCap))
        ((tSirMacCapabilityInfo *) &caps)->qos = 0;
    swapBitField16(caps, ( tANI_U16* )&frm.Capabilities );

    CFG_LIM_GET_INT_NO_STATUS( nSirStatus, pMac, WNI_CFG_WPS_ASSOC_METHOD,
                               nWps );

    frm.ListenInterval.interval = pMlmReassocReq->listenInterval;

    palCopyMemory( pMac->hHdd, ( tANI_U8* )frm.CurrentAPAddress.mac,
                   ( tANI_U8* )psessionEntry->bssId, 6 );

    PopulateDot11fSSID2( pMac, &frm.SSID );
    PopulateDot11fSuppRates( pMac, POPULATE_DOT11F_RATES_OPERATIONAL,
                             &frm.SuppRates,psessionEntry);

    fQosEnabled = ( pMac->lim.gLimQosEnabled ) &&
        SIR_MAC_GET_QOS( psessionEntry->limReassocBssCaps );

    fWmeEnabled = ( pMac->lim.gLimWmeEnabled ) &&
        LIM_BSS_CAPS_GET( WME, psessionEntry->limReassocBssQosCaps );

    fWsmEnabled = ( pMac->lim.gLimWsmEnabled ) && fWmeEnabled &&
        LIM_BSS_CAPS_GET( WSM, psessionEntry->limReassocBssQosCaps );


    if ( pMac->lim.gLim11hEnable  &&
         psessionEntry->pLimReAssocReq->spectrumMgtIndicator == eSIR_TRUE )
    {
        PopulateDot11fPowerCaps( pMac, &frm.PowerCaps, LIM_REASSOC,psessionEntry);
        PopulateDot11fSuppChannels( pMac, &frm.SuppChannels, LIM_REASSOC,psessionEntry);
    }

    if ( fQosEnabled &&
         ( ! PROP_CAPABILITY_GET(11EQOS, psessionEntry->limReassocBssPropCap ) ))
    {
        PopulateDot11fQOSCapsStation( pMac, &frm.QOSCapsStation );
    }

    PopulateDot11fExtSuppRates( pMac, &frm.ExtSuppRates );

    // The join request *should* contain zero or one of the WPA and RSN
    // IEs.  The payload send along with the request is a
    // 'tSirSmeJoinReq'; the IE portion is held inside a 'tSirRSNie':

    //     typedef struct sSirRSNie
    //     {
    //         tANI_U16       length;
    //         tANI_U8        rsnIEdata[SIR_MAC_MAX_IE_LENGTH+2];
    //     } tSirRSNie, *tpSirRSNie;

    // So, we should be able to make the following two calls harmlessly,
    // since they do nothing if they don't find the given IE in the
    // bytestream with which they're provided.

    // The net effect of this will be to faithfully transmit whatever
    // security IE is in the join request.

    // *However*, if we're associating for the purpose of WPS
    // enrollment, and we've been configured to indicate that by
    // eliding the WPA or RSN IE, we just skip this:
    if ( 0 == ( 0x0002 & nWps ) )
    {
        PopulateDot11fRSNOpaque( pMac, &( psessionEntry->pLimReAssocReq->rsnIE ),
                                 &frm.RSNOpaque );
        PopulateDot11fWPAOpaque( pMac, &( psessionEntry->pLimReAssocReq->rsnIE ),
                                 &frm.WPAOpaque );
#if defined(FEATURE_WLAN_WAPI)
        PopulateDot11fWAPIOpaque( pMac, &( psessionEntry->pLimReAssocReq->rsnIE ),
                                 &frm.WAPIOpaque );
#endif // defined(FEATURE_WLAN_WAPI)
    }

    // include WME EDCA IE as well
    if ( fWmeEnabled )
    {
        if ( ! PROP_CAPABILITY_GET( WME, psessionEntry->limReassocBssPropCap ) )
        {
            PopulateDot11fWMMInfoStation( pMac, &frm.WMMInfoStation );
        }

        if ( fWsmEnabled &&
             ( ! PROP_CAPABILITY_GET(WSM, psessionEntry->limReassocBssPropCap )))
        {
            PopulateDot11fWMMCaps( &frm.WMMCaps );
        }
    }

    if ( psessionEntry->htCapabality &&
          pMac->lim.htCapabilityPresentInBeacon)
    {
        PopulateDot11fHTCaps( pMac, &frm.HTCaps );
    }

    // If we're associating for the purpose of WPS
    // enrollment, and we've been configured to indicate that by
    // including the WPS IE, populate it:
    if ( 0 != ( 0x0001 & nWps ) )
    {
        PopulateDot11fWscAssocReq( pMac, &frm.WscAssocReq );
    }


    nStatus = dot11fGetPackedReAssocRequestSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Re-Association Request (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fReAssocRequest );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Re-Association Re "
                               "quest(0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        psessionEntry->limMlmState = psessionEntry->limPrevMlmState;
	 MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a Re-As"
                               "sociation Request.\n"), nBytes );
        goto end;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_REASSOC_REQ,
                                psessionEntry->limReAssocbssId,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for an Association Request (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        goto end;
    }


    // That done, pack the Probe Request:
    nStatus = dot11fPackReAssocRequest( pMac, &frm, pFrame +
                                        sizeof(tSirMacMgmtHdr),
                                        nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Re-Association Reque"
                               "st (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        goto end;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a R"
                               "e-Association Request (0x%08x).\n") );
    }

    PELOG1(limLog( pMac, LOG1, FL("*** Sending Re-Association Request length %d"
                           "to \n"),
            nBytes );)

    if( psessionEntry->assocReq != NULL )
    {
        palFreeMemory(pMac->hHdd, psessionEntry->assocReq);
        psessionEntry->assocReq = NULL;
    }
    if( (palAllocateMemory(pMac->hHdd, (void**)&psessionEntry->assocReq, nPayload)) != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("Unable to allocate memory to store assoc request"));)
     }
     else
     {
        //Store the Assoc request. This is sent to csr/hdd in join cnf response. 
        palCopyMemory(pMac->hHdd, psessionEntry->assocReq, pFrame + sizeof(tSirMacMgmtHdr), nPayload);
        psessionEntry->assocReqLen = nPayload;
     }


    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send Re-Association Request"
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        goto end;
    }

end:
    // Free up buffer allocated for mlmAssocReq
    palFreeMemory( pMac->hHdd, ( tANI_U8* ) pMlmReassocReq );
    psessionEntry->pLimMlmReassocReq = NULL;

} // limSendReassocReqMgmtFrame

/**
 * \brief Send an Authentication frame
 *
 *
 * \param pMac Pointer to Global MAC structure
 *
 * \param pAuthFrameBody Pointer to Authentication frame structure that need
 * to be sent
 *
 * \param peerMacAddr MAC address of the peer entity to which Authentication
 * frame is destined
 *
 * \param wepBit Indicates whether wep bit to be set in FC while sending
 * Authentication frame3
 *
 *
 * This function is called by limProcessMlmMessages().  Authentication frame
 * is formatted and sent when this function is called.
 *
 *
 */

void
limSendAuthMgmtFrame(tpAniSirGlobal pMac,
                     tpSirMacAuthFrameBody pAuthFrameBody,
                     tSirMacAddr           peerMacAddr,
                     tANI_U8               wepBit,
                     tpPESession           psessionEntry 
                                                       )
{
    tANI_U8            *pFrame, *pBody;
    tANI_U32            frameLen = 0, bodyLen = 0;
    tpSirMacMgmtHdr     pMacHdr;
    tANI_U16            i;
    void               *pPacket;
    eHalStatus          halstatus;
  
    if (wepBit == LIM_WEP_IN_FC)
    {
        /// Auth frame3 to be sent with encrypted framebody
        /**
         * Allocate buffer for Authenticaton frame of size equal
         * to management frame header length plus 2 bytes each for
         * auth algorithm number, transaction number, status code,
         * 128 bytes for challenge text and 4 bytes each for
         * IV & ICV.
         */

        frameLen = sizeof(tSirMacMgmtHdr) + LIM_ENCR_AUTH_BODY_LEN;

        bodyLen = LIM_ENCR_AUTH_BODY_LEN;
    } // if (wepBit == LIM_WEP_IN_FC)
    else
    {
        switch (pAuthFrameBody->authTransactionSeqNumber)
        {
            case SIR_MAC_AUTH_FRAME_1:
                /**
                 * Allocate buffer for Authenticaton frame of size
                 * equal to management frame header length plus 2 bytes
                 * each for auth algorithm number, transaction number
                 * and status code.
                 */

                frameLen = sizeof(tSirMacMgmtHdr) +
                           SIR_MAC_AUTH_CHALLENGE_OFFSET;
                bodyLen  = SIR_MAC_AUTH_CHALLENGE_OFFSET;

                break;

            case SIR_MAC_AUTH_FRAME_2:
                if ((pAuthFrameBody->authAlgoNumber == eSIR_OPEN_SYSTEM) ||
                    ((pAuthFrameBody->authAlgoNumber == eSIR_SHARED_KEY) &&
                     (pAuthFrameBody->authStatusCode != eSIR_MAC_SUCCESS_STATUS)))
                {
                    /**
                     * Allocate buffer for Authenticaton frame of size
                     * equal to management frame header length plus
                     * 2 bytes each for auth algorithm number,
                     * transaction number and status code.
                     */

                    frameLen = sizeof(tSirMacMgmtHdr) +
                               SIR_MAC_AUTH_CHALLENGE_OFFSET;
                    bodyLen = SIR_MAC_AUTH_CHALLENGE_OFFSET;
                }
                else
                {
                    // Shared Key algorithm with challenge text
                    // to be sent
                    /**
                     * Allocate buffer for Authenticaton frame of size
                     * equal to management frame header length plus
                     * 2 bytes each for auth algorithm number,
                     * transaction number, status code and 128 bytes
                     * for challenge text.
                     */

                    frameLen = sizeof(tSirMacMgmtHdr) +
                               sizeof(tSirMacAuthFrame);
                    bodyLen  = sizeof(tSirMacAuthFrameBody);
                }

                break;

            case SIR_MAC_AUTH_FRAME_3:
                /// Auth frame3 to be sent without encrypted framebody
                /**
                 * Allocate buffer for Authenticaton frame of size equal
                 * to management frame header length plus 2 bytes each
                 * for auth algorithm number, transaction number and
                 * status code.
                 */

                frameLen = sizeof(tSirMacMgmtHdr) +
                           SIR_MAC_AUTH_CHALLENGE_OFFSET;
                bodyLen  = SIR_MAC_AUTH_CHALLENGE_OFFSET;

                break;

            case SIR_MAC_AUTH_FRAME_4:
                /**
                 * Allocate buffer for Authenticaton frame of size equal
                 * to management frame header length plus 2 bytes each
                 * for auth algorithm number, transaction number and
                 * status code.
                 */

                frameLen = sizeof(tSirMacMgmtHdr) +
                           SIR_MAC_AUTH_CHALLENGE_OFFSET;
                bodyLen  = SIR_MAC_AUTH_CHALLENGE_OFFSET;

                break;
        } // switch (pAuthFrameBody->authTransactionSeqNumber)
    } // end if (wepBit == LIM_WEP_IN_FC)


    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )frameLen, ( void** ) &pFrame, ( void** ) &pPacket );

    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        // Log error
        limLog(pMac, LOGP, FL("call to bufAlloc failed for AUTH frame\n"));

        return;
    }

    for (i = 0; i < frameLen; i++)
        pFrame[i] = 0;

    // Prepare BD
    if (limPopulateBD(pMac, pFrame, SIR_MAC_MGMT_FRAME,
                      SIR_MAC_MGMT_AUTH, peerMacAddr,psessionEntry->selfMacAddr) != eSIR_SUCCESS)
    {
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;
    pMacHdr->fc.wep = wepBit;

    // Prepare BSSId
    if(  (psessionEntry->limSystemRole == eLIM_AP_ROLE)|| (psessionEntry->limSystemRole == eLIM_BT_AMP_AP_ROLE) )
    {
    
	    palCopyMemory( pMac->hHdd,(tANI_U8 *) pMacHdr->bssId,
        	          (tANI_U8 *) psessionEntry->bssId,
            	       sizeof( tSirMacAddr ));
    }

    /// Prepare Authentication frame body
    pBody    = pFrame + sizeof(tSirMacMgmtHdr);

    if (wepBit == LIM_WEP_IN_FC)
    {
        palCopyMemory( pMac->hHdd, pBody, (tANI_U8 *) pAuthFrameBody, bodyLen);

        PELOG1(limLog(pMac, LOG1,
           FL("*** Sending Auth seq# 3 status %d (%d) to\n"),
           pAuthFrameBody->authStatusCode,
           (pAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS));

        limPrintMacAddr(pMac, pMacHdr->da, LOG1);)
    }
    else
    {
        *((tANI_U16 *)(pBody)) = sirSwapU16ifNeeded(pAuthFrameBody->authAlgoNumber);
        pBody   += sizeof(tANI_U16);
        bodyLen -= sizeof(tANI_U16);

        *((tANI_U16 *)(pBody)) = sirSwapU16ifNeeded(pAuthFrameBody->authTransactionSeqNumber);
        pBody   += sizeof(tANI_U16);
        bodyLen -= sizeof(tANI_U16);

        *((tANI_U16 *)(pBody)) = sirSwapU16ifNeeded(pAuthFrameBody->authStatusCode);
        pBody   += sizeof(tANI_U16);
        bodyLen -= sizeof(tANI_U16);

        palCopyMemory( pMac->hHdd, pBody, (tANI_U8 *) &pAuthFrameBody->type, bodyLen);

        PELOG1(limLog(pMac, LOG1,
           FL("*** Sending Auth seq# %d status %d (%d) to "),
           pAuthFrameBody->authTransactionSeqNumber,
           pAuthFrameBody->authStatusCode,
           (pAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS));

        limPrintMacAddr(pMac, pMacHdr->da, LOG1);)
    }
    PELOG2(sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG2, pFrame, frameLen);)

    /// Queue Authentication frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) frameLen,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog(pMac, LOGE,
               FL("*** Could not send Auth frame, retCode=%X ***\n"),
               halstatus);

        //Pkt will be freed up by the callback
    }

    return;
} /*** end limSendAuthMgmtFrame() ***/

/**
 * \brief This function is called to send Disassociate frame.
 *
 *
 * \param pMac Pointer to Global MAC structure
 *
 * \param nReason Indicates the reason that need to be sent in
 * Disassociation frame
 *
 * \param peerMacAddr MAC address of the STA to which Disassociation frame is
 * sent
 *
 *
 */

void
limSendDisassocMgmtFrame(tpAniSirGlobal pMac,
                         tANI_U16       nReason,
                         tSirMacAddr    peer,tpPESession psessionEntry)
{
    tDot11fDisassociation frm;
    tANI_U8              *pFrame;
    tSirRetStatus         nSirStatus;
    tpSirMacMgmtHdr       pMacHdr;
    tANI_U32              nBytes, nPayload, nStatus;
    void                 *pPacket;
    eHalStatus            halstatus;

    if(NULL == psessionEntry)
    {
        return;
    }
    
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    frm.Reason.code = nReason;

    nStatus = dot11fGetPackedDisassociationSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Disassociation (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fDisassociation );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Disassociation "
                               "(0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a Dis"
                               "association.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_DISASSOC, peer,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a Disassociation (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;                 // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    // Prepare the BSSID
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);
    
    nStatus = dot11fPackDisassociation( pMac, &frm, pFrame +
                                        sizeof(tSirMacMgmtHdr),
                                        nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Disassociation (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a D"
                               "isassociation (0x%08x).\n") );
    }

    PELOG1(limLog( pMac, LOG1, FL("*** Sending Disassociation frame with rea"
                           "son %d to\n"), nReason );
    limPrintMacAddr( pMac, pMacHdr->da, LOG1 );)

    // Queue Disassociation frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send Disassociation "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return;
    }

} // End limSendDisassocMgmtFrame.

/**
 * \brief This function is called to send a Deauthenticate frame
 *
 *
 * \param pMac Pointer to global MAC structure
 *
 * \param nReason Indicates the reason that need to be sent in the
 * Deauthenticate frame
 *
 * \param peeer address of the STA to which the frame is to be sent
 *
 *
 */

void
limSendDeauthMgmtFrame(tpAniSirGlobal pMac,
                       tANI_U16       nReason,
                       tSirMacAddr    peer,tpPESession psessionEntry)
{
    tDot11fDeAuth    frm;
    tANI_U8         *pFrame;
    tSirRetStatus    nSirStatus;
    tpSirMacMgmtHdr  pMacHdr;
    tANI_U32         nBytes, nPayload, nStatus;
    void            *pPacket;
    eHalStatus       halstatus;

    if(NULL == psessionEntry)
    {
        return;
    }
    
    palZeroMemory( pMac->hHdd, ( tANI_U8* ) &frm, sizeof( frm ) );

    frm.Reason.code = nReason;

    nStatus = dot11fGetPackedDeAuthSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a De-Authentication (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fDeAuth );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a De-Authentication "
                               "(0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                             ( tANI_U16 )nBytes, ( void** ) &pFrame,
                             ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a De-"
                               "Authentication.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_DEAUTH, peer,psessionEntry->selfMacAddr);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a De-Authentication (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;                 // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    // Prepare the BSSID
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

    nStatus = dot11fPackDeAuth( pMac, &frm, pFrame +
                                sizeof(tSirMacMgmtHdr),
                                nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a DeAuthentication (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
                    ( void* ) pFrame, ( void* ) pPacket );
        return;
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a D"
                               "e-Authentication (0x%08x).\n") );
    }

    PELOG1(limLog( pMac, LOG1, FL("*** Sending De-Authentication frame with rea"
                           "son %d to\n"), nReason );
    limPrintMacAddr( pMac, pMacHdr->da, LOG1 );)

    // Queue Disassociation frame in high priority WQ
    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send De-Authentication "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return;
    }

} // End limSendDeauthMgmtFrame.


#ifdef ANI_SUPPORT_11H
/**
 * \brief Send a Measurement Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC structure
 *
 * \param pMeasReqFrame Address of a tSirMacMeasReqActionFrame
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
limSendMeasReportFrame(tpAniSirGlobal             pMac,
                       tpSirMacMeasReqActionFrame pMeasReqFrame,
                       tSirMacAddr                peer)
{
    tDot11fMeasurementReport frm;
    tANI_U8                      *pFrame;
    tSirRetStatus            nSirStatus;
    tpSirMacMgmtHdr          pMacHdr;
    tANI_U32                      nBytes, nPayload, nStatus, nCfg;
    void               *pPacket;
    eHalStatus          halstatus;
   
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    frm.Category.category = SIR_MAC_ACTION_SPECTRUM_MGMT;
    frm.Action.action     = SIR_MAC_ACTION_MEASURE_REPORT_ID;
    frm.DialogToken.token = pMeasReqFrame->actionHeader.dialogToken;

    switch ( pMeasReqFrame->measReqIE.measType )
    {
    case SIR_MAC_BASIC_MEASUREMENT_TYPE:
        nSirStatus =
            PopulateDot11fMeasurementReport0( pMac, pMeasReqFrame,
                                               &frm.MeasurementReport );
        break;
    case SIR_MAC_CCA_MEASUREMENT_TYPE:
        nSirStatus =
            PopulateDot11fMeasurementReport1( pMac, pMeasReqFrame,
                                               &frm.MeasurementReport );
        break;
    case SIR_MAC_RPI_MEASUREMENT_TYPE:
        nSirStatus =
            PopulateDot11fMeasurementReport2( pMac, pMeasReqFrame,
                                               &frm.MeasurementReport );
        break;
    default:
        limLog( pMac, LOGE, FL("Unknown measurement type %d in limSen"
                               "dMeasReportFrame.\n"),
                pMeasReqFrame->measReqIE.measType );
        return eSIR_FAILURE;
    }

    if ( eSIR_SUCCESS != nSirStatus ) return eSIR_FAILURE;

    nStatus = dot11fGetPackedMeasurementReportSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Measurement Report (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fMeasurementReport );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Measurement Rep"
                               "ort (0x%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a De-"
                               "Authentication.\n"), nBytes );
        return eSIR_FAILURE;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a Measurement Report (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    nCfg = 6;
    nSirStatus = wlan_cfgGetStr( pMac, WNI_CFG_BSSID, pMacHdr->bssId, &nCfg );
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to retrieve WNI_CFG_BSSID from"
                               " CFG (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    nStatus = dot11fPackMeasurementReport( pMac, &frm, pFrame +
                                           sizeof(tSirMacMgmtHdr),
                                           nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Measurement Report (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a M"
                               "easurement Report (0x%08x).\n") );
    }

    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send a Measurement Report  "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return eSIR_FAILURE;    // just allocated...
    }

    return eSIR_SUCCESS;

} // End limSendMeasReportFrame.


/**
 * \brief Send a TPC Request Action frame
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param peer MAC address to which the frame should be sent
 *
 *
 */

void
limSendTpcRequestFrame(tpAniSirGlobal pMac,
                       tSirMacAddr    peer)
{
    tDot11fTPCRequest  frm;
    tANI_U8                *pFrame;
    tSirRetStatus      nSirStatus;
    tpSirMacMgmtHdr    pMacHdr;
    tANI_U32                nBytes, nPayload, nStatus, nCfg;
    void               *pPacket;
    eHalStatus          halstatus;
   
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    frm.Category.category  = SIR_MAC_ACTION_SPECTRUM_MGMT;
    frm.Action.action      = SIR_MAC_ACTION_TPC_REQUEST_ID;
    frm.DialogToken.token  = 1;
    frm.TPCRequest.present = 1;

    nStatus = dot11fGetPackedTPCRequestSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a TPC Request (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fTPCRequest );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a TPC Request (0x"
                               "%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
                               " Request.\n"), nBytes );
        return;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a TPC Request (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    nCfg = 6;
    nSirStatus = wlan_cfgGetStr( pMac, WNI_CFG_BSSID, pMacHdr->bssId, &nCfg );
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to retrieve WNI_CFG_BSSID from"
                               " CFG (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // just allocated...
    }

    nStatus = dot11fPackTPCRequest( pMac, &frm, pFrame +
                                    sizeof(tSirMacMgmtHdr),
                                    nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a TPC Request (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return;                 // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a T"
                               "PC Request (0x%08x).\n") );
    }

    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send a TPC Request "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return;
    }

} // End limSendTpcRequestFrame.


/**
 * \brief Send a TPC Report Action frame
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param pTpcReqFrame Pointer to the received TPC Request
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
limSendTpcReportFrame(tpAniSirGlobal            pMac,
                      tpSirMacTpcReqActionFrame pTpcReqFrame,
                      tSirMacAddr               peer)
{
    tDot11fTPCReport frm;
    tANI_U8              *pFrame;
    tSirRetStatus    nSirStatus;
    tpSirMacMgmtHdr  pMacHdr;
    tANI_U32              nBytes, nPayload, nStatus, nCfg;
    void               *pPacket;
    eHalStatus          halstatus;
   
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    frm.Category.category  = SIR_MAC_ACTION_SPECTRUM_MGMT;
    frm.Action.action      = SIR_MAC_ACTION_TPC_REPORT_ID;
    frm.DialogToken.token  = pTpcReqFrame->actionHeader.dialogToken;

    // FramesToDo: On the Gen4_TVM branch, there was a comment:
    // "misplaced this function, need to replace:
    // txPower = halGetRateToPwrValue(pMac, staid,
    //     pMac->lim.gLimCurrentChannelId, 0);
    frm.TPCReport.tx_power    = 0;
    frm.TPCReport.link_margin = 0;
    frm.TPCReport.present     = 1;

    nStatus = dot11fGetPackedTPCReportSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a TPC Report (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fTPCReport );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a TPC Report (0x"
                               "%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
                               " Report.\n"), nBytes );
        return eSIR_FAILURE;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a TPC Report (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    nCfg = 6;
    nSirStatus = wlan_cfgGetStr( pMac, WNI_CFG_BSSID, pMacHdr->bssId, &nCfg );
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to retrieve WNI_CFG_BSSID from"
                               " CFG (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    nStatus = dot11fPackTPCReport( pMac, &frm, pFrame +
                                   sizeof(tSirMacMgmtHdr),
                                   nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a TPC Report (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a T"
                               "PC Report (0x%08x).\n") );
    }

    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send a TPC Report "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return eSIR_FAILURE;    // just allocated...
    }

    return eSIR_SUCCESS;

} // End limSendTpcReportFrame.
#endif  //ANI_SUPPORT_11H


#ifdef ANI_PRODUCT_TYPE_AP
/**
 * \brief Send a Channel Switch Announcement
 *
 *
 * \param pMac Pointer to the global MAC datastructure
 *
 * \param peer MAC address to which this frame will be sent
 *
 * \param nMode
 *
 * \param nNewChannel
 *
 * \param nCount
 *
 * \return eSIR_SUCCESS on success, eSIR_FAILURE else
 *
 *
 */

tSirRetStatus
limSendChannelSwitchMgmtFrame(tpAniSirGlobal pMac,
                              tSirMacAddr    peer,
                              tANI_U8             nMode,
                              tANI_U8             nNewChannel,
                              tANI_U8             nCount)
{
    tDot11fChannelSwitch frm;
    tANI_U8                  *pFrame;
    tSirRetStatus        nSirStatus;
    tpSirMacMgmtHdr      pMacHdr;
    tANI_U32                  nBytes, nPayload, nStatus, nCfg;
    void               *pPacket;
    eHalStatus          halstatus;
    
    palZeroMemory( pMac->hHdd, ( tANI_U8* )&frm, sizeof( frm ) );

    frm.Category.category     = SIR_MAC_ACTION_SPECTRUM_MGMT;
    frm.Action.action         = SIR_MAC_ACTION_CHANNEL_SWITCH_ID;
    frm.ChanSwitchAnn.switchMode    = nMode;
    frm.ChanSwitchAnn.newChannel    = nNewChannel;
    frm.ChanSwitchAnn.switchCount   = nCount;
    frm.ChanSwitchAnn.present = 1;

    nStatus = dot11fGetPackedChannelSwitchSize( pMac, &frm, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to calculate the packed size f"
                               "or a Channel Switch (0x%08x).\n"),
                nStatus );
        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fChannelSwitch );
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while calculating"
                               "the packed size for a Channel Switch (0x"
                               "%08x).\n"), nStatus );
    }

    nBytes = nPayload + sizeof( tSirMacMgmtHdr );

    halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( tANI_U16 )nBytes, ( void** ) &pFrame, ( void** ) &pPacket );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a TPC"
                               " Report.\n"), nBytes );
        return eSIR_FAILURE;
    }

    // Paranoia:
    palZeroMemory( pMac->hHdd, pFrame, nBytes );

    // Next, we fill out the buffer descriptor:
    nSirStatus = limPopulateBD( pMac, pFrame, SIR_MAC_MGMT_FRAME,
                                SIR_MAC_MGMT_ACTION, peer);
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to populate the buffer descrip"
                               "tor for a Channel Switch (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    pMacHdr = ( tpSirMacMgmtHdr ) pFrame;

    nCfg = 6;
    nSirStatus = wlan_cfgGetStr( pMac, WNI_CFG_BSSID, pMacHdr->bssId, &nCfg );
    if ( eSIR_SUCCESS != nSirStatus )
    {
        limLog( pMac, LOGE, FL("Failed to retrieve WNI_CFG_BSSID from"
                               " CFG (%d).\n"),
                nSirStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // just allocated...
    }

    nStatus = dot11fPackChannelSwitch( pMac, &frm, pFrame +
                                       sizeof(tSirMacMgmtHdr),
                                       nPayload, &nPayload );
    if ( DOT11F_FAILED( nStatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to pack a Channel Switch (0x%08x).\n"),
                nStatus );
        palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, ( void* ) pFrame, ( void* ) pPacket );
        return eSIR_FAILURE;    // allocated!
    }
    else if ( DOT11F_WARNED( nStatus ) )
    {
        limLog( pMac, LOGW, FL("There were warnings while packing a C"
                               "hannel Switch (0x%08x).\n") );
    }

    halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
                            HAL_TXRX_FRM_802_11_MGMT,
                            ANI_TXDIR_TODS,
                            7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                            limTxComplete, pFrame );
    if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
    {
        limLog( pMac, LOGE, FL("Failed to send a Channel Switch "
                               "(%X)!\n"),
                nSirStatus );
        //Pkt will be freed up by the callback
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;

} // End limSendChannelSwitchMgmtFrame.

#endif // (ANI_PRODUCT_TYPE_AP)


/**
 * \brief Send an ADDBA Req Action Frame to peer
 *
 * \sa limSendAddBAReq
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param pMlmAddBAReq A pointer to tLimMlmAddBAReq. This contains
 * the necessary parameters reqd by PE send the ADDBA Req Action
 * Frame to the peer
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */
tSirRetStatus limSendAddBAReq( tpAniSirGlobal pMac,
    tpLimMlmAddBAReq pMlmAddBAReq ,tpPESession psessionEntry)
{
    tDot11fAddBAReq frmAddBAReq;
    tANI_U8 *pAddBAReqBuffer = NULL;
    tpSirMacMgmtHdr pMacHdr;
    tANI_U32 frameLen = 0, nStatus, nPayload;
    tSirRetStatus statusCode;
    eHalStatus halStatus;
    void *pPacket;
     if(NULL == psessionEntry)
    {
        return eSIR_FAILURE;
    }

    palZeroMemory( pMac->hHdd, (void *) &frmAddBAReq, sizeof( frmAddBAReq ));

    // Category - 3 (BA)
    frmAddBAReq.Category.category = SIR_MAC_ACTION_BLKACK;
    
    // Action - 0 (ADDBA Req)
    frmAddBAReq.Action.action = SIR_MAC_BLKACK_ADD_REQ;

    // FIXME - Dialog Token, generalize this...
    frmAddBAReq.DialogToken.token = pMlmAddBAReq->baDialogToken;

    // Fill the ADDBA Parameter Set
    frmAddBAReq.AddBAParameterSet.tid = pMlmAddBAReq->baTID;
    frmAddBAReq.AddBAParameterSet.policy = pMlmAddBAReq->baPolicy;
    frmAddBAReq.AddBAParameterSet.bufferSize = pMlmAddBAReq->baBufferSize;

    // BA timeout
    // 0 - indicates no BA timeout
    frmAddBAReq.BATimeout.timeout = pMlmAddBAReq->baTimeout;

  // BA Starting Sequence Number
  // Fragment number will always be zero
  if (pMlmAddBAReq->baSSN < LIM_TX_FRAMES_THRESHOLD_ON_CHIP) {
      pMlmAddBAReq->baSSN = LIM_TX_FRAMES_THRESHOLD_ON_CHIP;
  }
  
  frmAddBAReq.BAStartingSequenceControl.ssn = 
	  pMlmAddBAReq->baSSN - LIM_TX_FRAMES_THRESHOLD_ON_CHIP;

    nStatus = dot11fGetPackedAddBAReqSize( pMac, &frmAddBAReq, &nPayload );

    if( DOT11F_FAILED( nStatus ))
    {
        limLog( pMac, LOGW,
        FL( "Failed to calculate the packed size for "
          "an ADDBA Request (0x%08x).\n"),
        nStatus );

        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fAddBAReq );
    }
    else if( DOT11F_WARNED( nStatus ))
    {
        limLog( pMac, LOGW,
        FL( "There were warnings while calculating"
          "the packed size for an ADDBA Req (0x%08x).\n"),
        nStatus );
    }

    // Add the MGMT header to frame length
    frameLen = nPayload + sizeof( tSirMacMgmtHdr );

    // Need to allocate a buffer for ADDBA AF
    if( eHAL_STATUS_SUCCESS !=
      (halStatus = palPktAlloc( pMac->hHdd,
                                HAL_TXRX_FRM_802_11_MGMT,
                                (tANI_U16) frameLen,
                                (void **) &pAddBAReqBuffer,
                                (void **) &pPacket )))
    {
        // Log error
        limLog( pMac, LOGP,
        FL("palPktAlloc FAILED! Length [%d], Status [%d]\n"),
        frameLen,
        halStatus );

        statusCode = eSIR_MEM_ALLOC_FAILED;
        goto returnAfterError;
    }

    palZeroMemory( pMac->hHdd, (void *) pAddBAReqBuffer, frameLen );

    // Copy necessary info to BD
    if( eSIR_SUCCESS !=
      (statusCode = limPopulateBD( pMac,
                                   pAddBAReqBuffer,
                                   SIR_MAC_MGMT_FRAME,
                                   SIR_MAC_MGMT_ACTION,
                                   pMlmAddBAReq->peerMacAddr,psessionEntry->selfMacAddr)))
    goto returnAfterError;

    // Update A3 with the BSSID
    pMacHdr = ( tpSirMacMgmtHdr ) pAddBAReqBuffer;
    
    #if 0
    cfgLen = SIR_MAC_ADDR_LENGTH;
    if( eSIR_SUCCESS != cfgGetStr( pMac,
        WNI_CFG_BSSID,
        (tANI_U8 *) pMacHdr->bssId,
        &cfgLen ))
    {
        limLog( pMac, LOGP,
        FL( "Failed to retrieve WNI_CFG_BSSID while"
          "sending an ACTION Frame\n" ));

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
    }
    #endif//TO SUPPORT BT-AMP
    sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

    // Now, we're ready to "pack" the frames
    nStatus = dot11fPackAddBAReq( pMac,
      &frmAddBAReq,
      pAddBAReqBuffer + sizeof( tSirMacMgmtHdr ),
      nPayload,
      &nPayload );

    if( DOT11F_FAILED( nStatus ))
    {
        limLog( pMac, LOGE,
        FL( "Failed to pack an ADDBA Req (0x%08x).\n" ),
        nStatus );

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
    }
    else if( DOT11F_WARNED( nStatus ))
    {
        limLog( pMac, LOGW,
        FL( "There were warnings while packing an ADDBA Req (0x%08x).\n" ));
    }

    limLog( pMac, LOGW,
      FL( "Sending an ADDBA REQ to \n" ));
    limPrintMacAddr( pMac, pMlmAddBAReq->peerMacAddr, LOGW );

    if( eHAL_STATUS_SUCCESS !=
      (halStatus = halTxFrame( pMac,
                               pPacket,
                               (tANI_U16) frameLen,
                               HAL_TXRX_FRM_802_11_MGMT,
                               ANI_TXDIR_TODS,
                               7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                               limTxComplete,
                               pAddBAReqBuffer )))
    {
        limLog( pMac, LOGE,
        FL( "halTxFrame FAILED! Status [%d]\n"),
        halStatus );

    // FIXME - Need to convert eHalStatus to tSirRetStatus
    statusCode = eSIR_FAILURE;
    //Pkt will be freed up by the callback
    return statusCode;
  }
  else
    return eSIR_SUCCESS;

returnAfterError:

  // Release buffer, if allocated
  if( NULL != pAddBAReqBuffer )
    palPktFree( pMac->hHdd,
        HAL_TXRX_FRM_802_11_MGMT,
        (void *) pAddBAReqBuffer,
        (void *) pPacket );

  return statusCode;
}

/**
 * \brief Send an ADDBA Rsp Action Frame to peer
 *
 * \sa limSendAddBARsp
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param pMlmAddBARsp A pointer to tLimMlmAddBARsp. This contains
 * the necessary parameters reqd by PE send the ADDBA Rsp Action
 * Frame to the peer
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */
tSirRetStatus limSendAddBARsp( tpAniSirGlobal pMac,
    tpLimMlmAddBARsp pMlmAddBARsp,
    tpPESession      psessionEntry)
{
    tDot11fAddBARsp frmAddBARsp;
    tANI_U8 *pAddBARspBuffer = NULL;
    tpSirMacMgmtHdr pMacHdr;
    tANI_U32 frameLen = 0, nStatus, nPayload;
    tSirRetStatus statusCode;
    eHalStatus halStatus;
    void *pPacket;

     if(NULL == psessionEntry)
    {
        return eSIR_FAILURE;
    }

      palZeroMemory( pMac->hHdd, (void *) &frmAddBARsp, sizeof( frmAddBARsp ));

      // Category - 3 (BA)
      frmAddBARsp.Category.category = SIR_MAC_ACTION_BLKACK;
      // Action - 1 (ADDBA Rsp)
      frmAddBARsp.Action.action = SIR_MAC_BLKACK_ADD_RSP;

      // Should be same as the one we received in the ADDBA Req
      frmAddBARsp.DialogToken.token = pMlmAddBARsp->baDialogToken;

      // ADDBA Req status
      frmAddBARsp.Status.status = pMlmAddBARsp->addBAResultCode;

      // Fill the ADDBA Parameter Set as provided by caller
      frmAddBARsp.AddBAParameterSet.tid = pMlmAddBARsp->baTID;
      frmAddBARsp.AddBAParameterSet.policy = pMlmAddBARsp->baPolicy;
      frmAddBARsp.AddBAParameterSet.bufferSize = pMlmAddBARsp->baBufferSize;

      // BA timeout
      // 0 - indicates no BA timeout
      frmAddBARsp.BATimeout.timeout = pMlmAddBARsp->baTimeout;

      nStatus = dot11fGetPackedAddBARspSize( pMac, &frmAddBARsp, &nPayload );

      if( DOT11F_FAILED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "Failed to calculate the packed size for "
              "an ADDBA Response (0x%08x).\n"),
            nStatus );

        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fAddBARsp );
      }
      else if( DOT11F_WARNED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "There were warnings while calculating"
              "the packed size for an ADDBA Rsp (0x%08x).\n"),
            nStatus );
      }

      // Need to allocate a buffer for ADDBA AF
      frameLen = nPayload + sizeof( tSirMacMgmtHdr );

      // Allocate shared memory
      if( eHAL_STATUS_SUCCESS !=
          (halStatus = palPktAlloc( pMac->hHdd,
                                    HAL_TXRX_FRM_802_11_MGMT,
                                    (tANI_U16) frameLen,
                                    (void **) &pAddBARspBuffer,
                                    (void **) &pPacket )))
      {
        // Log error
        limLog( pMac, LOGP,
            FL("palPktAlloc FAILED! Length [%d], Status [%d]\n"),
            frameLen,
            halStatus );

        statusCode = eSIR_MEM_ALLOC_FAILED;
        goto returnAfterError;
      }

      palZeroMemory( pMac->hHdd, (void *) pAddBARspBuffer, frameLen );

      // Copy necessary info to BD
      if( eSIR_SUCCESS !=
          (statusCode = limPopulateBD( pMac,
                                       pAddBARspBuffer,
                                       SIR_MAC_MGMT_FRAME,
                                       SIR_MAC_MGMT_ACTION,
                                       pMlmAddBARsp->peerMacAddr,psessionEntry->selfMacAddr)))
        goto returnAfterError;

      // Update A3 with the BSSID
      
      pMacHdr = ( tpSirMacMgmtHdr ) pAddBARspBuffer;
      
      #if 0
      cfgLen = SIR_MAC_ADDR_LENGTH;
      if( eSIR_SUCCESS != wlan_cfgGetStr( pMac,
            WNI_CFG_BSSID,
            (tANI_U8 *) pMacHdr->bssId,
            &cfgLen ))
      {
        limLog( pMac, LOGP,
            FL( "Failed to retrieve WNI_CFG_BSSID while"
              "sending an ACTION Frame\n" ));

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
      }
      #endif // TO SUPPORT BT-AMP
      sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

      // Now, we're ready to "pack" the frames
      nStatus = dot11fPackAddBARsp( pMac,
          &frmAddBARsp,
          pAddBARspBuffer + sizeof( tSirMacMgmtHdr ),
          nPayload,
          &nPayload );

      if( DOT11F_FAILED( nStatus ))
      {
        limLog( pMac, LOGE,
            FL( "Failed to pack an ADDBA Rsp (0x%08x).\n" ),
            nStatus );

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
      }
      else if( DOT11F_WARNED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "There were warnings while packing an ADDBA Rsp (0x%08x).\n" ));
      }

      limLog( pMac, LOGW,
          FL( "Sending an ADDBA RSP to \n" ));
      limPrintMacAddr( pMac, pMlmAddBARsp->peerMacAddr, LOGW );

  if( eHAL_STATUS_SUCCESS !=
      (halStatus = halTxFrame( pMac,
                               pPacket,
                               (tANI_U16) frameLen,
                               HAL_TXRX_FRM_802_11_MGMT,
                               ANI_TXDIR_TODS,
                               7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                               limTxComplete,
                               pAddBARspBuffer )))
  {
    limLog( pMac, LOGE,
        FL( "halTxFrame FAILED! Status [%d]\n" ),
        halStatus );

    // FIXME - HAL error codes are different from PE error
    // codes!! And, this routine is returning tSirRetStatus
    statusCode = eSIR_FAILURE;
    //Pkt will be freed up by the callback
    return statusCode;
  }
  else
    return eSIR_SUCCESS;

    returnAfterError:

      // Release buffer, if allocated
      if( NULL != pAddBARspBuffer )
        palPktFree( pMac->hHdd,
            HAL_TXRX_FRM_802_11_MGMT,
            (void *) pAddBARspBuffer,
            (void *) pPacket );

      return statusCode;
}

/**
 * \brief Send a DELBA Indication Action Frame to peer
 *
 * \sa limSendDelBAInd
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param peerMacAddr MAC Address of peer
 *
 * \param reasonCode Reason for the DELBA notification
 *
 * \param pBAParameterSet The DELBA Parameter Set.
 * This identifies the TID for which the BA session is
 * being deleted.
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */
tSirRetStatus limSendDelBAInd( tpAniSirGlobal pMac,
    tpLimMlmDelBAReq pMlmDelBAReq,tpPESession psessionEntry)
{
    tDot11fDelBAInd frmDelBAInd;
    tANI_U8 *pDelBAIndBuffer = NULL;
    //tANI_U32 val;
    tpSirMacMgmtHdr pMacHdr;
    tANI_U32 frameLen = 0, nStatus, nPayload;
    tSirRetStatus statusCode;
    eHalStatus halStatus;
    void *pPacket;

     if(NULL == psessionEntry)
    {
        return eSIR_FAILURE;
    }

    palZeroMemory( pMac->hHdd, (void *) &frmDelBAInd, sizeof( frmDelBAInd ));

      // Category - 3 (BA)
      frmDelBAInd.Category.category = SIR_MAC_ACTION_BLKACK;
      // Action - 2 (DELBA)
      frmDelBAInd.Action.action = SIR_MAC_BLKACK_DEL;

      // Fill the DELBA Parameter Set as provided by caller
      frmDelBAInd.DelBAParameterSet.tid = pMlmDelBAReq->baTID;
      frmDelBAInd.DelBAParameterSet.initiator = pMlmDelBAReq->baDirection;

      // BA Starting Sequence Number
      // Fragment number will always be zero
      frmDelBAInd.Reason.code = pMlmDelBAReq->delBAReasonCode;

      nStatus = dot11fGetPackedDelBAIndSize( pMac, &frmDelBAInd, &nPayload );

      if( DOT11F_FAILED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "Failed to calculate the packed size for "
              "an DELBA Indication (0x%08x).\n"),
            nStatus );

        // We'll fall back on the worst case scenario:
        nPayload = sizeof( tDot11fDelBAInd );
      }
      else if( DOT11F_WARNED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "There were warnings while calculating"
              "the packed size for an DELBA Ind (0x%08x).\n"),
            nStatus );
      }

      // Add the MGMT header to frame length
      frameLen = nPayload + sizeof( tSirMacMgmtHdr );

      // Allocate shared memory
      if( eHAL_STATUS_SUCCESS !=
          (halStatus = palPktAlloc( pMac->hHdd,
                                    HAL_TXRX_FRM_802_11_MGMT,
                                    (tANI_U16) frameLen,
                                    (void **) &pDelBAIndBuffer,
                                    (void **) &pPacket )))
      {
        // Log error
        limLog( pMac, LOGP,
            FL("palPktAlloc FAILED! Length [%d], Status [%d]\n"),
            frameLen,
            halStatus );

        statusCode = eSIR_MEM_ALLOC_FAILED;
        goto returnAfterError;
      }

      palZeroMemory( pMac->hHdd, (void *) pDelBAIndBuffer, frameLen );

      // Copy necessary info to BD
      if( eSIR_SUCCESS !=
          (statusCode = limPopulateBD( pMac,
                                       pDelBAIndBuffer,
                                       SIR_MAC_MGMT_FRAME,
                                       SIR_MAC_MGMT_ACTION,
                                       pMlmDelBAReq->peerMacAddr,psessionEntry->selfMacAddr)))
        goto returnAfterError;

      // Update A3 with the BSSID
      pMacHdr = ( tpSirMacMgmtHdr ) pDelBAIndBuffer;
      
      #if 0
      cfgLen = SIR_MAC_ADDR_LENGTH;
      if( eSIR_SUCCESS != cfgGetStr( pMac,
            WNI_CFG_BSSID,
            (tANI_U8 *) pMacHdr->bssId,
            &cfgLen ))
      {
        limLog( pMac, LOGP,
            FL( "Failed to retrieve WNI_CFG_BSSID while"
              "sending an ACTION Frame\n" ));

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
      }
      #endif //TO SUPPORT BT-AMP
      sirCopyMacAddr(pMacHdr->bssId,psessionEntry->bssId);

      // Now, we're ready to "pack" the frames
      nStatus = dot11fPackDelBAInd( pMac,
          &frmDelBAInd,
          pDelBAIndBuffer + sizeof( tSirMacMgmtHdr ),
          nPayload,
          &nPayload );

      if( DOT11F_FAILED( nStatus ))
      {
        limLog( pMac, LOGE,
            FL( "Failed to pack an DELBA Ind (0x%08x).\n" ),
            nStatus );

        // FIXME - Need to convert to tSirRetStatus
        statusCode = eSIR_FAILURE;
        goto returnAfterError;
      }
      else if( DOT11F_WARNED( nStatus ))
      {
        limLog( pMac, LOGW,
            FL( "There were warnings while packing an DELBA Ind (0x%08x).\n" ));
      }

      limLog( pMac, LOGW,
          FL( "Sending a DELBA IND to \n" ));
      limPrintMacAddr( pMac, pMlmDelBAReq->peerMacAddr, LOGW );

  if( eHAL_STATUS_SUCCESS !=
      (halStatus = halTxFrame( pMac,
                               pPacket,
                               (tANI_U16) frameLen,
                               HAL_TXRX_FRM_802_11_MGMT,
                               ANI_TXDIR_TODS,
                               7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
                               limTxComplete,
                               pDelBAIndBuffer )))
  {
    PELOGE(limLog( pMac, LOGE, FL( "halTxFrame FAILED! Status [%d]\n" ), halStatus );)
    statusCode = eSIR_FAILURE;
    //Pkt will be freed up by the callback
    return statusCode;
  }
  else
    return eSIR_SUCCESS;

    returnAfterError:

      // Release buffer, if allocated
      if( NULL != pDelBAIndBuffer )
        palPktFree( pMac->hHdd,
            HAL_TXRX_FRM_802_11_MGMT,
            (void *) pDelBAIndBuffer,
            (void *) pPacket );

      return statusCode;
}
/**
 * \brief Send a SM Power Save state update Action Frame to AP
 *
 * \sa limSendSMPowerSaveStateFrame
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param peer  The Mac address of the AP to which this action frame is addressed
*
 * \param State
 *
 * \return eSIR_SUCCESS if setup completes successfully
 *         eSIR_FAILURE is some problem is encountered
 */
 
tSirRetStatus limSendSMPowerStateFrame( tpAniSirGlobal pMac, tSirMacAddr peer,
                                                tSirMacHTMIMOPowerSaveState State )
{

        tSirRetStatus statusCode = eSIR_SUCCESS;
             
        #if 0
        tDot11fSMPowerSave frmSMpowerstate;
        tANI_U8 *pSMPowerBuffer = NULL;
        tANI_U32 frameLen = 0, nStatus, nPayload, cfgLen;
        tpSirMacMgmtHdr pMacHdr;
        eHalStatus halStatus;
        void *pPacket;
        tpDphHashNode pSta = NULL;

        limLog( pMac, LOG1,FL( "State is %d \n" ), State);

        palZeroMemory(pMac->hHdd, (void *) &frmSMpowerstate, sizeof( frmSMpowerstate ));


         pSta = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);

         if (pSta->mlmStaContext.htCapability == false) {
             limLog( pMac, LOGE,FL( "Peer is not HT Capable \n" ));
             return eSIR_FAILURE;
        }
         
        /**Fill  Category - 7 (HT) */
        frmSMpowerstate.Category.category = SIR_MAC_ACTION_HT;
        /** Fill Action - 1 (SM Power) */
        frmSMpowerstate.Action.action = SIR_MAC_SM_POWER_SAVE;

        /**  Fill the SMPowerModeSet as provided by caller */
        switch(State) {
        case eSIR_HT_MIMO_PS_NO_LIMIT:
                frmSMpowerstate.SMPowerModeSet.PowerSave_En = 0;
                frmSMpowerstate.SMPowerModeSet.Mode = 0;
                break;

        case eSIR_HT_MIMO_PS_STATIC:
                frmSMpowerstate.SMPowerModeSet.PowerSave_En = 1;
                frmSMpowerstate.SMPowerModeSet.Mode = 0;
                break;

        case eSIR_HT_MIMO_PS_DYNAMIC:
                frmSMpowerstate.SMPowerModeSet.PowerSave_En = 1;
                frmSMpowerstate.SMPowerModeSet.Mode = 1;
                break;

        default:
                limLog( pMac, LOGW,FL( "invalid state\n" ));
                return eSIR_FAILURE;
        }

        limLog( pMac, LOGE,FL( "frmSMPowerstate is updated \n" ));

        nStatus = dot11fGetPackedSMPowerSaveSize( pMac, &frmSMpowerstate, &nPayload );

        if (DOT11F_FAILED( nStatus )) {

                limLog( pMac, LOGW, FL( "Failed to calculate the packed size for "
                            "an SMPowerState Indication (0x%08x).\n"),nStatus );
                /** We'll fall back on the worst case scenario: */
                nPayload = sizeof( tDot11fSMPowerSave);
        } else if (DOT11F_WARNED( nStatus )) {
                limLog( pMac, LOGW, FL( "There were warnings while calculating"
                        "the packed size for an SMPowerState Ind (0x%08x).\n"), nStatus);
        }

        limLog( pMac, LOGE,FL( "dot11fGetPacked smpwr save is done \n" ));

        /** Add the MGMT header to frame length */
        frameLen = nPayload + sizeof( tSirMacMgmtHdr );

        /** Allocate shared memory */
        halStatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, (tANI_U16) frameLen,
                                (void **) &pSMPowerBuffer, (void **) &pPacket );
	
        if (eHAL_STATUS_SUCCESS != halStatus) {
                limLog( pMac, LOGP, FL("palPktAlloc FAILED! Length [%d], Status [%d]\n"),
                                             frameLen, halStatus );
                statusCode = eSIR_MEM_ALLOC_FAILED;
                goto returnAfterError;
        }

        palZeroMemory( pMac->hHdd, (void *) pSMPowerBuffer, frameLen);

        /** Copy necessary info to BD */
        statusCode = limPopulateBD( pMac, pSMPowerBuffer, SIR_MAC_MGMT_FRAME, 
                                    SIR_MAC_MGMT_ACTION, peer,);

        if( eSIR_SUCCESS != statusCode)
                goto returnAfterError;

        pMacHdr = ( tpSirMacMgmtHdr ) pSMPowerBuffer;
        cfgLen = SIR_MAC_ADDR_LENGTH;

        statusCode = wlan_cfgGetStr( pMac, WNI_CFG_BSSID, (tANI_U8 *) pMacHdr->bssId, &cfgLen );
	
        if( eSIR_SUCCESS != statusCode) {
                limLog( pMac, LOGP, FL( "Failed to retrieve WNI_CFG_BSSID while"
                                 "sending an ACTION Frame\n" ));
                goto returnAfterError;
        }

        /** Now, we're ready to "pack" the frames */
        nStatus = dot11fPackSMPowerSave(pMac, &frmSMpowerstate, 
                                    pSMPowerBuffer + sizeof( tSirMacMgmtHdr ), nPayload, &nPayload );

        if( DOT11F_FAILED( nStatus )) {
                limLog( pMac, LOGE, FL( "Failed to pack an SM Power Save (0x%08x).\n" ),
                                        nStatus );
                statusCode = eSIR_FAILURE;
                goto returnAfterError;
        } else if( DOT11F_WARNED( nStatus )) {
                limLog( pMac, LOGW, FL( "There were warnings while packing an SMPowerState Ind (0x%08x).\n" ));
        }

        limLog( pMac, LOGW, FL( "Sending a SM Power Save mode update to \n" ));
        limPrintMacAddr( pMac, peer, LOGW );

        halStatus = halTxFrame( pMac, pPacket, (tANI_U16) frameLen, HAL_TXRX_FRM_802_11_MGMT,
                               ANI_TXDIR_TODS, 7,//SMAC_SWBD_TX_TID_MGMT_HIGH
                               limTxComplete, pSMPowerBuffer );

        if( eHAL_STATUS_SUCCESS != halStatus) {
                limLog( pMac, LOGE, FL( "halTxFrame FAILED! Status [%d]\n" ),
                        halStatus );
                statusCode = eSIR_FAILURE;
                //Pkt will be freed up by the callback
                return statusCode;
        } else {
                return eSIR_SUCCESS;
        }

        returnAfterError:

        // Release buffer, if allocated
        if( NULL != pSMPowerBuffer) 
            palPktFree( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT, (void *) pSMPowerBuffer, 

        
        #endif 
   return statusCode;
        
}




