/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limIbssPeerMgmt.cc contains the utility functions
 * LIM uses to maintain peers in IBSS.
 * Author:        Chandra Modumudi
 * Date:          03/12/04
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#include "palTypes.h"
#include "aniGlobal.h"
#include "sirCommon.h"
#if (WNI_POLARIS_FW_PRODUCT == AP)
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limStaHashApi.h"
#include "schApi.h"          // schSetFixedBeaconFields for IBSS coalesce
#include "limSecurityUtils.h"
#include "limSendMessages.h"


/**
 * ibss_peer_find
 *
 *FUNCTION:
 * This function is called while adding a context at
 * DPH & Polaris for a peer in IBSS.
 * If peer is found in the list, capabilities from the
 * returned BSS description are used at DPH node & Polaris.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  macAddr - MAC address of the peer
 *
 * @return Pointer to peer node if found, else NULL
 */

static tLimIbssPeerNode *
ibss_peer_find(
    tpAniSirGlobal  pMac,
    tSirMacAddr     macAddr)
{
    tLimIbssPeerNode *pTempNode = pMac->lim.gLimIbssPeerList;

    while (pTempNode != NULL)
    {
        if (palEqualMemory( pMac->hHdd,(tANI_U8 *) macAddr,
                      (tANI_U8 *) &pTempNode->peerMacAddr,
                      sizeof(tSirMacAddr)) )
            break;
        pTempNode = pTempNode->next;
    }
    return pTempNode;
} /*** end ibss_peer_find() ***/

/**
 * ibss_peer_add
 *
 *FUNCTION:
 * This is called on a STA in IBSS upon receiving Beacon/
 * Probe Response from a peer.
 *
 *LOGIC:
 * Node is always added to the front of the list
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      - Pointer to Global MAC structure
 * @param  pPeerNode - Pointer to peer node to be added to the list.
 *
 * @return None
 */

static tSirRetStatus
ibss_peer_add(tpAniSirGlobal pMac, tLimIbssPeerNode *pPeerNode)
{
#ifdef ANI_SIR_IBSS_PEER_CACHING
    tANI_U32 numIbssPeers = (2 * pMac->lim.maxStation);

    if (pMac->lim.gLimNumIbssPeers >= numIbssPeers)
    {
        /**
         * Reached max number of peers to be maintained.
         * Delete last entry & add new entry at the beginning.
         */
        tLimIbssPeerNode *pTemp, *pPrev;
        pTemp = pPrev = pMac->lim.gLimIbssPeerList;
        while (pTemp->next != NULL)
        {
            pPrev = pTemp;
            pTemp = pTemp->next;
        }
        if(pTemp->beacon)
        {
            palFreeMemory(pMac->hHdd, pTemp->beacon);
        }

        palFreeMemory( pMac->hHdd, (tANI_U8 *) pTemp);
        pPrev->next = NULL;
    }
    else
#endif
        pMac->lim.gLimNumIbssPeers++;

    pPeerNode->next = pMac->lim.gLimIbssPeerList;
    pMac->lim.gLimIbssPeerList = pPeerNode;

    return eSIR_SUCCESS;

} /*** end limAddIbssPeerToList() ***/

/**
 * ibss_peer_collect
 *
 *FUNCTION:
 * This is called to collect IBSS peer information
 * from received Beacon/Probe Response frame from it.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 * @param  pPeer   - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_peer_collect(
    tpAniSirGlobal      pMac,
    tpSchBeaconStruct   pBeacon,
    tpSirMacMgmtHdr     pHdr,
    tLimIbssPeerNode    *pPeer)
{
    palCopyMemory( pMac->hHdd, pPeer->peerMacAddr, pHdr->sa, sizeof(tSirMacAddr));

    pPeer->capabilityInfo       = pBeacon->capabilityInfo;
    pPeer->extendedRatesPresent = pBeacon->extendedRatesPresent;
    pPeer->edcaPresent          = pBeacon->edcaPresent;
    pPeer->wmeEdcaPresent       = pBeacon->wmeEdcaPresent;
    pPeer->wmeInfoPresent       = pBeacon->wmeInfoPresent;

    if(IS_DOT11_MODE_HT(pMac->lim.gLimDot11Mode) &&
        (pBeacon->HTCaps.present))
    {
        pPeer->htCapable =  pBeacon->HTCaps.present;   
        palCopyMemory(pMac->hHdd, (tANI_U8 *)pPeer->supportedMCSSet,
                        (tANI_U8 *)pBeacon->HTCaps.supportedMCSSet,
                        sizeof(pPeer->supportedMCSSet));
        pPeer->htGreenfield = (tANI_U8)pBeacon->HTCaps.greenField;
        pPeer->htSupportedChannelWidthSet = ( tANI_U8 ) pBeacon->HTCaps.supportedChannelWidthSet;
        pPeer->htMIMOPSState =  (tSirMacHTMIMOPowerSaveState)pBeacon->HTCaps.mimoPowerSave;
        pPeer->htMaxAmsduLength = ( tANI_U8 ) pBeacon->HTCaps.maximalAMSDUsize;
        pPeer->htAMpduDensity =             pBeacon->HTCaps.mpduDensity;
        pPeer->htDsssCckRate40MHzSupport = (tANI_U8)pBeacon->HTCaps.dsssCckMode40MHz;
        pPeer->htShortGI20Mhz = (tANI_U8)pBeacon->HTCaps.shortGI20MHz;
        pPeer->htShortGI40Mhz = (tANI_U8)pBeacon->HTCaps.shortGI40MHz;
        pPeer->htMaxRxAMpduFactor = pBeacon->HTCaps.maxRxAMPDUFactor;
    }

    pPeer->erpIePresent = pBeacon->erpPresent;

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pPeer->supportedRates,
                  (tANI_U8 *) &pBeacon->supportedRates,
                  pBeacon->supportedRates.numRates + 1);
    if (pPeer->extendedRatesPresent)
        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pPeer->extendedRates,
                          (tANI_U8 *) &pBeacon->extendedRates,
                          pBeacon->extendedRates.numRates + 1);
    else
        pPeer->extendedRates.numRates = 0;

    // TBD copy EDCA parameters
    // pPeer->edcaParams;

    pPeer->next = NULL;
} /*** end ibss_peer_collect() ***/

// handle change in peer qos/wme capabilities
static void
ibss_sta_caps_update(
    tpAniSirGlobal    pMac,
    tLimIbssPeerNode *pPeerNode)
{
    tANI_U16      aid;
    tpDphHashNode pStaDs;

    pPeerNode->beaconHBCount++; //Update beacon count.
    
    // if the peer node exists, update its qos capabilities
    if ((pStaDs = dphLookupHashEntry(pMac, pPeerNode->peerMacAddr, &aid)) == NULL)
        return;


    //Update HT Capabilities
    if(IS_DOT11_MODE_HT(pMac->lim.gLimDot11Mode))
    {
        pStaDs->mlmStaContext.htCapability = pPeerNode->htCapable;
        if (pPeerNode->htCapable)
        {
            pStaDs->htGreenfield = pPeerNode->htGreenfield;
            pStaDs->htSupportedChannelWidthSet =  pPeerNode->htSupportedChannelWidthSet;
            pStaDs->htMIMOPSState =             pPeerNode->htMIMOPSState;
            pStaDs->htMaxAmsduLength =  pPeerNode->htMaxAmsduLength;
            pStaDs->htAMpduDensity =             pPeerNode->htAMpduDensity;
            pStaDs->htDsssCckRate40MHzSupport = pPeerNode->htDsssCckRate40MHzSupport;
            pStaDs->htShortGI20Mhz = pPeerNode->htShortGI20Mhz;
            pStaDs->htShortGI40Mhz = pPeerNode->htShortGI40Mhz;
            pStaDs->htMaxRxAMpduFactor = pPeerNode->htMaxRxAMpduFactor;
            // In the future, may need to check for "delayedBA"
            // For now, it is IMMEDIATE BA only on ALL TID's
            pStaDs->baPolicyFlag = 0xFF;
        }
    }

    if(IS_DOT11_MODE_PROPRIETARY(pMac->lim.gLimDot11Mode) &&
      pPeerNode->aniIndicator)
    {
        pStaDs->aniPeer = pPeerNode->aniIndicator;
        pStaDs->propCapability = pPeerNode->propCapability;
    }


    // peer is 11e capable but is not 11e enabled yet
    // some STA's when joining Airgo IBSS, assert qos capability even when
    // they don't suport qos. however, they do not include the edca parameter
    // set. so let's check for edcaParam in addition to the qos capability
    if (pPeerNode->capabilityInfo.qos && (pMac->lim.gLimQosEnabled) && pPeerNode->edcaPresent)
    {
        pStaDs->qosMode    = 1;
        pStaDs->wmeEnabled = 0;
        if (! pStaDs->lleEnabled)
        {
            pStaDs->lleEnabled = 1;
            //dphSetACM(pMac, pStaDs);
        }
        return;
    }
    // peer is not 11e capable now but was 11e enabled earlier
    else if (pStaDs->lleEnabled)
    {
        pStaDs->qosMode      = 0;
        pStaDs->lleEnabled   = 0;
    }

    // peer is wme capable but is not wme enabled yet
    if (pPeerNode->wmeInfoPresent &&  pMac->lim.gLimWmeEnabled)
    {
        pStaDs->qosMode    = 1;
        pStaDs->lleEnabled = 0;
        if (! pStaDs->wmeEnabled)
        {
            pStaDs->wmeEnabled = 1;
            //dphSetACM(pMac, pStaDs);
        }
        return;
    }
    /* When the peer device supports EDCA parameters, then we were not 
      considering. Added this code when we saw that one of the Peer Device
      was advertising WMM param where we were not honouring that. CR# 210756
    */
    if (pPeerNode->wmeEdcaPresent && pMac->lim.gLimWmeEnabled) {
        pStaDs->qosMode    = 1;
        pStaDs->lleEnabled = 0;
        if (! pStaDs->wmeEnabled) {
            pStaDs->wmeEnabled = 1;
        }
        return;
    }

    // peer is not wme capable now but was wme enabled earlier
    else if (pStaDs->wmeEnabled)
    {
        pStaDs->qosMode      = 0;
        pStaDs->wmeEnabled   = 0;
    }

}

static void
ibss_sta_rates_update(
    tpAniSirGlobal   pMac,
    tpDphHashNode    pStaDs,
    tLimIbssPeerNode *pPeer)
{

    // Populate supported rateset
    limPopulateMatchingRateSet(pMac, pStaDs, &pPeer->supportedRates,
                               &pPeer->extendedRates, pPeer->supportedMCSSet,
                               &pStaDs->mlmStaContext.propRateSet);

    pStaDs->mlmStaContext.capabilityInfo = pPeer->capabilityInfo;
} /*** end ibss_sta_info_update() ***/

/**
 * ibss_sta_info_update
 *
 *FUNCTION:
 * This is called to program both SW & Polaris context
 * for peer in IBSS.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac   - Pointer to Global MAC structure
 * @param  pStaDs - Pointer to DPH node
 * @param  pPeer  - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_sta_info_update(
    tpAniSirGlobal   pMac,
    tpDphHashNode    pStaDs,
    tLimIbssPeerNode *pPeer)
{
    pStaDs->staType = STA_ENTRY_PEER;
    ibss_sta_caps_update(pMac, pPeer);
    ibss_sta_rates_update(pMac, pStaDs, pPeer);
} /*** end ibss_sta_info_update() ***/

static void
ibss_coalesce_free(
    tpAniSirGlobal pMac)
{
    if (pMac->lim.ibssInfo.pHdr != NULL)
        palFreeMemory(pMac->hHdd, pMac->lim.ibssInfo.pHdr);
    if (pMac->lim.ibssInfo.pBeacon != NULL)
        palFreeMemory(pMac->hHdd, pMac->lim.ibssInfo.pBeacon);

    pMac->lim.ibssInfo.pHdr    = NULL;
    pMac->lim.ibssInfo.pBeacon = NULL;
}

/*
 * save the beacon params for use when adding the bss
 */
static void
ibss_coalesce_save(
    tpAniSirGlobal      pMac,
    tpSirMacMgmtHdr     pHdr,
    tpSchBeaconStruct   pBeacon)
{
    eHalStatus status;

    // get rid of any saved info
    ibss_coalesce_free(pMac);

    status = palAllocateMemory(pMac->hHdd, (void **) &pMac->lim.ibssInfo.pHdr,
                               sizeof(*pHdr));
    if (status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("ibbs-save: Failed malloc pHdr\n"));)
        return;
    }
    status = palAllocateMemory(pMac->hHdd, (void **) &pMac->lim.ibssInfo.pBeacon,
                               sizeof(*pBeacon));
    if (status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("ibbs-save: Failed malloc pBeacon\n"));)
        ibss_coalesce_free(pMac);
        return;
    }

    palCopyMemory(pMac->hHdd, pMac->lim.ibssInfo.pHdr, pHdr, sizeof(*pHdr));
    palCopyMemory(pMac->hHdd, pMac->lim.ibssInfo.pBeacon, pBeacon, sizeof(*pBeacon));
}

/*
 * tries to add a new entry to dph hash node
 * if necessary, an existing entry is eliminated
 */
static tSirRetStatus
ibss_dph_entry_add(
    tpAniSirGlobal  pMac,
    tSirMacAddr     peerAddr,
    tpDphHashNode   *ppSta)
{
    tANI_U16      aid;
    tpDphHashNode pStaDs;

    *ppSta = NULL;

    pStaDs = dphLookupHashEntry(pMac, peerAddr, &aid);
    if (pStaDs != NULL)
    {
        /* Trying to add context for already existing STA in IBSS */
        PELOGE(limLog(pMac, LOGE, FL("STA exists already "));)
        limPrintMacAddr(pMac, peerAddr, LOGE);
        return eSIR_FAILURE;
    }

    /**
     * Assign an AID, delete context existing with that
     * AID and then add an entry to hash table maintained
     * by DPH module.
     */
    aid = limAssignAID(pMac);

    pStaDs = dphGetHashEntry(pMac, aid);
    if (pStaDs)
    {
        (void) limDelSta(pMac, pStaDs, false /*asynchronous*/);
        limDeleteDphHashEntry(pMac, pStaDs->staAddr, aid);
    }

    pStaDs = dphAddHashEntry(pMac, peerAddr, aid);
    if (pStaDs == NULL)
    {
        // Could not add hash table entry
        PELOGE(limLog(pMac, LOGE, FL("could not add hash entry at DPH for aid=%d MACaddr:\n"), aid);)
        limPrintMacAddr(pMac, peerAddr, LOGE);
        return eSIR_FAILURE;
    }

    *ppSta = pStaDs;
    return eSIR_SUCCESS;
}

// send a status change notification
static void
ibss_status_chg_notify(
    tpAniSirGlobal          pMac,
    tSirMacAddr             peerAddr,
    tANI_U16                staIndex,
    tANI_U16                status)
{

    tLimIbssPeerNode *peerNode;
    tANI_U8 *beacon = NULL;
    tANI_U16 bcnLen = 0;


    peerNode = ibss_peer_find(pMac,peerAddr);
    if(peerNode != NULL)
    {
        if(peerNode->beacon == NULL) peerNode->beaconLen = 0;
        beacon = peerNode->beacon;
        bcnLen = peerNode->beaconLen;
        peerNode->beacon = NULL;
        peerNode->beaconLen = 0; 
    }

    limSendSmeIBSSPeerInd(pMac,peerAddr, staIndex, beacon, bcnLen, status);

    if(beacon != NULL)
    {
        palFreeMemory(pMac->hHdd, beacon);
    }
}


static void
ibss_bss_add(
    tpAniSirGlobal    pMac)
{
    tLimMlmStartReq   mlmStartReq;
    tANI_U32          cfg;
    tpSirMacMgmtHdr   pHdr    = (tpSirMacMgmtHdr)   pMac->lim.ibssInfo.pHdr;
    tpSchBeaconStruct pBeacon = (tpSchBeaconStruct) pMac->lim.ibssInfo.pBeacon;
    tANI_U8 numExtRates = 0;

    if ((pHdr == NULL) || (pBeacon == NULL))
    {
        PELOGE(limLog(pMac, LOGE, FL("Unable to add BSS (no cached BSS info)\n"));)
        return;
    }

    palCopyMemory( pMac->hHdd, pMac->lim.gLimCurrentBssId, pHdr->bssId,
                   sizeof(tSirMacAddr));

    if (cfgSetStr(pMac, WNI_CFG_BSSID, (tANI_U8 *) pHdr->bssId, sizeof(tSirMacAddr))
        != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not update BSSID at CFG\n"));
    limSetBssid(pMac, pHdr->bssId);

    if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &cfg) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("Can't read beacon interval\n"));
    if (cfg != pBeacon->beaconInterval)
        if (cfgSetInt(pMac, WNI_CFG_BEACON_INTERVAL, pBeacon->beaconInterval)
            != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("Can't update beacon interval\n"));

    palCopyMemory( pMac->hHdd,
       (tANI_U8 *) &pMac->lim.gpLimStartBssReq->operationalRateSet,
       (tANI_U8 *) &pBeacon->supportedRates,
       pBeacon->supportedRates.numRates);
    if (cfgSetStr(pMac, WNI_CFG_OPERATIONAL_RATE_SET,
           (tANI_U8 *) &pMac->lim.gpLimStartBssReq->operationalRateSet.rate,
           pMac->lim.gpLimStartBssReq->operationalRateSet.numRates)
        != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not update OperRateset at CFG\n"));



    /**
    * WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET CFG needs to be reset, when
    * there is no extended rate IE present in beacon. This is especially important when
    * supportedRateSet IE contains all the extended rates as well and STA decides to coalesce. 
    * In this IBSS coalescing scenario LIM will tear down the BSS and Add a new one. So LIM needs to 
    * reset this CFG, just in case CSR originally had set this CFG when IBSS was started from the local profile.
    * If IBSS was started by CSR from the BssDescription, then it would reset this CFG before StartBss is issued.
    * The idea is that the count of OpRateSet and ExtendedOpRateSet rates should not be more than 12.
    */
    
    if(pBeacon->extendedRatesPresent)
        numExtRates = pBeacon->extendedRates.numRates;
    if (cfgSetStr(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET, 
           (tANI_U8 *) &pBeacon->extendedRates.rate, numExtRates) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not update ExtendedOperRateset at CFG\n"));


    /*
    * Each IBSS node will advertise its own HT Capabilities instead of adapting to the Peer's capabilities
    * If we don't do this then IBSS may not go back to full capabilities when the STA with lower capabilities
    * leaves the IBSS.  e.g. when non-CB STA joins an IBSS and then leaves, the IBSS will be stuck at non-CB mode
    * even though all the nodes are capable of doing CB.
    * so it is decided to leave the self HT capabilties intact. This may change if some issues are found in interop.
    */
    palZeroMemory(pMac->hHdd, (void *) &mlmStartReq, sizeof(mlmStartReq));

    palCopyMemory(pMac->hHdd, mlmStartReq.bssId, pHdr->bssId, sizeof(tSirMacAddr));
    mlmStartReq.rateSet.numRates = pMac->lim.gpLimStartBssReq->operationalRateSet.numRates;
    palCopyMemory(pMac->hHdd, &mlmStartReq.rateSet.rate[0],
                  &pMac->lim.gpLimStartBssReq->operationalRateSet.rate[0],
                  mlmStartReq.rateSet.numRates);
    mlmStartReq.bssType             = eSIR_IBSS_MODE;
    mlmStartReq.beaconPeriod        = pBeacon->beaconInterval;
    mlmStartReq.nwType              = pMac->lim.gpLimStartBssReq->nwType;
    mlmStartReq.htCapable           = pMac->lim.htCapability;
    mlmStartReq.htOperMode          = pMac->lim.gHTOperMode;
    mlmStartReq.dualCTSProtection   = pMac->lim.gHTDualCTSProtection;
    mlmStartReq.txChannelWidthSet   = pMac->lim.gHTRecommendedTxWidthSet;


    if (wlan_cfgGetInt(pMac, WNI_CFG_CURRENT_CHANNEL, &cfg) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("CurrentChannel CFG get fialed!\n"));
    mlmStartReq.channelNumber       = (tSirMacChanNum) cfg;
    mlmStartReq.cbMode              = pMac->lim.gpLimStartBssReq->cbMode;


    // Copy the SSID for RxP filtering based on SSID.
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &mlmStartReq.ssId,
        (tANI_U8 *) &pMac->lim.gpLimStartBssReq->ssId,
        pMac->lim.gpLimStartBssReq->ssId.length + 1);

    PELOG1(limLog(pMac, LOG1, FL("invoking ADD_BSS as part of coalescing!\n"));)
    if (limMlmAddBss(pMac, &mlmStartReq) != eSIR_SME_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("AddBss failure\n"));)
        return;
    }

    // Update fields in Beacon
    if (schSetFixedBeaconFields(pMac) != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("*** Unable to set fixed Beacon fields ***\n"));)
        return;
    }

}



/* delete the current BSS */
static void
ibss_bss_delete(
    tpAniSirGlobal pMac)
{
    tSirRetStatus status;
    PELOGW(limLog(pMac, LOGW, FL("Initiating IBSS Delete BSS\n"));) 
    if (pMac->lim.gLimMlmState != eLIM_MLM_BSS_STARTED_STATE)
    {
        limLog(pMac, LOGW, FL("Incorrect LIM MLM state for delBss (%d)\n"),
               pMac->lim.gLimMlmState);
        return;
    }
    status = limDelBss(pMac, NULL, pMac->lim.gLimBssIdx);
    if (status != eSIR_SUCCESS)
        PELOGE(limLog(pMac, LOGE, FL("delBss failed for bss %d\n"), pMac->lim.gLimBssIdx);)
}

/**
 * limIbssInit
 *
 *FUNCTION:
 * This function is called while starting an IBSS
 * to initialize list used to maintain IBSS peers.
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
limIbssInit(
    tpAniSirGlobal pMac)
{
    pMac->lim.gLimIbssActive = 0;
    pMac->lim.gLimIbssCoalescingHappened = 0;
    pMac->lim.gLimIbssPeerList = NULL;
    pMac->lim.gLimNumIbssPeers = 0;

    // ibss info - params for which ibss to join while coalescing
    palZeroMemory(pMac->hHdd, &pMac->lim.ibssInfo, sizeof(tAniSirLimIbss));
} /*** end limIbssInit() ***/

/**
 * limIbssDeleteAllPeers
 *
 *FUNCTION:
 * This function is called to delete all peers.
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

void limIbssDeleteAllPeers( tpAniSirGlobal pMac )
{
    tLimIbssPeerNode    *pCurrNode, *pTempNode;
    tpDphHashNode pStaDs;
    tANI_U16 aid;

    pCurrNode = pTempNode = pMac->lim.gLimIbssPeerList;

    while (pCurrNode != NULL)
    {
        if (!pMac->lim.gLimNumIbssPeers)
        {
            limLog(pMac, LOGP,
               FL("Number of peers in the list is zero and node present"));
            return;
        }
		/* Delete the dph entry for the station
         	  * Since it is called to remove all peers, just delete from dph,
		  * no need to do any beacon related params i.e., dont call limDeleteDphHashEntry
		  */ 
        pStaDs = dphLookupHashEntry(pMac, pCurrNode->peerMacAddr, &aid);
        if( pStaDs )
        {
			ibss_status_chg_notify( pMac, pCurrNode->peerMacAddr, pStaDs->staIndex, eWNI_SME_IBSS_PEER_DEPARTED_IND );
            dphDeleteHashEntry(pMac, pStaDs->staAddr, aid);
		}

        pTempNode = pCurrNode->next;
        if(pCurrNode->beacon)
        {
            palFreeMemory(pMac->hHdd, pCurrNode->beacon);
        }
        palFreeMemory( pMac->hHdd, (tANI_U8 *) pCurrNode);
        if (pMac->lim.gLimNumIbssPeers > 0) // be paranoid
            pMac->lim.gLimNumIbssPeers--;
        pCurrNode = pTempNode;
    }

    if (pMac->lim.gLimNumIbssPeers)
        limLog(pMac, LOGP, FL("Number of peers[%d] in the list is non-zero"),
                pMac->lim.gLimNumIbssPeers);

    pMac->lim.gLimNumIbssPeers = 0;
    pMac->lim.gLimIbssPeerList = NULL;

}
/**
 * limIbssDelete
 *
 *FUNCTION:
 * This function is called while tearing down an IBSS.
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
limIbssDelete(
    tpAniSirGlobal pMac)
{
    limIbssDeleteAllPeers(pMac);

    ibss_coalesce_free(pMac);
} /*** end limIbssDelete() ***/

/** Commenting this Code as from no where it is being invoked */
#if 0
/**
 * limIbssPeerDelete
 *
 *FUNCTION:
 * This may be called on a STA in IBSS to delete a peer
 * from the list.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  peerMacAddr - MAC address of the peer STA that
 *                       need to be deleted from peer list.
 *
 * @return None
 */

void
limIbssPeerDelete(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    tLimIbssPeerNode    *pPrevNode, *pTempNode;

    pTempNode = pPrevNode = pMac->lim.gLimIbssPeerList;

    if (pTempNode == NULL)
        return;

    while (pTempNode != NULL)
    {
        if (palEqualMemory( pMac->hHdd,(tANI_U8 *) macAddr,
                      (tANI_U8 *) &pTempNode->peerMacAddr,
                      sizeof(tSirMacAddr)) )
        {
            // Found node to be deleted
            if (pMac->lim.gLimIbssPeerList == pTempNode) /** First Node to be deleted*/
		        pMac->lim.gLimIbssPeerList = pTempNode->next;
			else
	            pPrevNode->next = pTempNode->next;
			
            if(pTempNode->beacon)
            {
                palFreeMemory(pMac->hHdd, pTempNode->beacon);
                pTempNode->beacon = NULL;
            }
            palFreeMemory( pMac->hHdd, (tANI_U8 *) pTempNode);
            pMac->lim.gLimNumIbssPeers--;
            return;
        }

        pPrevNode = pTempNode;
        pTempNode = pTempNode->next;
    }

    // Should not be here
    PELOGE(limLog(pMac, LOGE, FL("peer not found in the list, addr= "));)
    limPrintMacAddr(pMac, macAddr, LOGE);
} /*** end limIbssPeerDelete() ***/

#endif


/** -------------------------------------------------------------
\fn limIbssSetProtection
\brief Decides all the protection related information.
\
\param  tpAniSirGlobal    pMac
\param  tSirMacAddr peerMacAddr
\param  tpUpdateBeaconParams pBeaconParams
\return None
  -------------------------------------------------------------*/
static void
limIbssSetProtection(tpAniSirGlobal pMac, tANI_U8 enable, tpUpdateBeaconParams pBeaconParams)
{

    if(!pMac->lim.cfgProtection.fromllb)
    {
        PELOG1(limLog(pMac, LOG1, FL("protection from 11b is disabled\n"));)
        return;
    }

    if (enable)
    {
        pMac->lim.gLim11bParams.protectionEnabled = true;
        if(false == pMac->lim.llbCoexist)
        {
			PELOGE(limLog(pMac, LOGE, FL("=> IBSS: Enable Protection \n"));)
            pBeaconParams->llbCoexist = pMac->lim.llbCoexist = true;
            pBeaconParams->paramChangeBitmap |= PARAM_llBCOEXIST_CHANGED;
        }
    }
    else if (true == pMac->lim.llbCoexist)
    {
        pMac->lim.gLim11bParams.protectionEnabled = false;
		PELOGE(limLog(pMac, LOGE, FL("===> IBSS: Disable protection \n"));)
        pBeaconParams->llbCoexist = pMac->lim.llbCoexist = false;
        pBeaconParams->paramChangeBitmap |= PARAM_llBCOEXIST_CHANGED;
    }
    return;
}


/** -------------------------------------------------------------
\fn limIbssUpdateProtectionParams
\brief Decides all the protection related information.
\
\param  tpAniSirGlobal    pMac
\param  tSirMacAddr peerMacAddr
\param  tpUpdateBeaconParams pBeaconParams
\return None
  -------------------------------------------------------------*/
static void
limIbssUpdateProtectionParams(tpAniSirGlobal pMac,
tSirMacAddr peerMacAddr, tLimProtStaCacheType protStaCacheType)
{
  tANI_U32 i;

  PELOG1(limLog(pMac,LOG1, FL("A STA is associated:"));
  limLog(pMac,LOG1, FL("Addr : "));
  limPrintMacAddr(pMac, peerMacAddr, LOG1);)

  for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
  {
      if (pMac->lim.protStaCache[i].active)
      {
          PELOG1(limLog(pMac, LOG1, FL("Addr: "));)
          PELOG1(limPrintMacAddr(pMac, pMac->lim.protStaCache[i].addr, LOG1);)

          if (palEqualMemory( pMac->hHdd,
              pMac->lim.protStaCache[i].addr,
              peerMacAddr, sizeof(tSirMacAddr)))
          {
              PELOG1(limLog(pMac, LOG1, FL("matching cache entry at %d already active.\n"), i);)
              return;
          }
      }
  }

  for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
  {
      if (!pMac->lim.protStaCache[i].active)
          break;
  }

  if (i >= LIM_PROT_STA_CACHE_SIZE)
  {
      PELOGE(limLog(pMac, LOGE, FL("No space in ProtStaCache\n"));)
      return;
  }

  palCopyMemory( pMac->hHdd, pMac->lim.protStaCache[i].addr,
                peerMacAddr,
                sizeof(tSirMacAddr));

  pMac->lim.protStaCache[i].protStaCacheType = protStaCacheType;
  pMac->lim.protStaCache[i].active = true;
  if(eLIM_PROT_STA_CACHE_TYPE_llB == protStaCacheType)
  {
      pMac->lim.gLim11bParams.numSta++;
  }
  else if(eLIM_PROT_STA_CACHE_TYPE_llG == protStaCacheType)
  {
      pMac->lim.gLim11gParams.numSta++;
  }
}


/** -------------------------------------------------------------
\fn limIbssDecideProtection
\brief Decides all the protection related information.
\
\param  tpAniSirGlobal    pMac
\param  tSirMacAddr peerMacAddr
\param  tpUpdateBeaconParams pBeaconParams
\return None
  -------------------------------------------------------------*/
static void
limIbssDecideProtection(tpAniSirGlobal pMac, tpDphHashNode pStaDs, tpUpdateBeaconParams pBeaconParams)
{
    tSirRFBand rfBand = SIR_BAND_UNKNOWN;
    tANI_U32 phyMode;
    tLimProtStaCacheType protStaCacheType = eLIM_PROT_STA_CACHE_TYPE_INVALID;

    pBeaconParams->paramChangeBitmap = 0;

    if(NULL == pStaDs)
    {
      PELOGE(limLog(pMac, LOGE, FL("pStaDs is NULL\n"));)
      return;
    }

    limGetRfBand(pMac, &rfBand);
    if(SIR_BAND_2_4_GHZ== rfBand)
    {
        limGetPhyMode(pMac, &phyMode);
        //We are 11G or 11n. Check if we need protection from 11b Stations.
        if ((phyMode == WNI_CFG_PHY_MODE_11G) || (pMac->lim.htCapability))
        {
            /* As we found in the past, it is possible that a 11n STA sends
             * Beacon with HT IE but not ERP IE.  So the absense of ERP IE
             * in the Beacon is not enough to conclude that STA is 11b. 
             */
            if ((pStaDs->erpEnabled == eHAL_CLEAR) &&
                (!pStaDs->mlmStaContext.htCapability))
            {
                protStaCacheType = eLIM_PROT_STA_CACHE_TYPE_llB;
                PELOGE(limLog(pMac, LOGE, FL("Enable protection from 11B\n"));)
                limIbssSetProtection(pMac, true, pBeaconParams);
            }
        }
    }
    limIbssUpdateProtectionParams(pMac, pStaDs->staAddr, protStaCacheType);
    return;
}


/**
 * limIbssStaAdd()
 *
 *FUNCTION:
 * This function is called to add an STA context in IBSS role
 * whenever a data frame is received from/for a STA that failed
 * hash lookup at DPH.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 * @return retCode    Indicates success or failure return code
 * @return
 */

tSirRetStatus
limIbssStaAdd(
    tpAniSirGlobal  pMac,
    void *pBody)
{
    tSirRetStatus       retCode = eSIR_SUCCESS;
    tpDphHashNode       pStaDs;
    tLimIbssPeerNode    *pPeerNode;
    tLimMlmStates       prevState;
    tSirMacAddr         *pPeerAddr = (tSirMacAddr *) pBody;
    tUpdateBeaconParams beaconParams; 

    palZeroMemory( pMac->hHdd, (tANI_U8 *) &beaconParams, sizeof(tUpdateBeaconParams));

    if (pBody == 0)
    {
        PELOGE(limLog(pMac, LOGE, FL("Invalid IBSS AddSta\n"));)
        return eSIR_FAILURE;
    }

    PELOGE(limLog(pMac, LOGE, FL("Rx Add-Ibss-Sta for MAC:\n"));)
    limPrintMacAddr(pMac, *pPeerAddr, LOGE);

    pPeerNode = ibss_peer_find(pMac, *pPeerAddr);
    if(NULL != pPeerNode)
    {
        retCode = ibss_dph_entry_add(pMac, *pPeerAddr, &pStaDs);
        if (eSIR_SUCCESS == retCode)
        {
            prevState = pStaDs->mlmStaContext.mlmState;
            pStaDs->erpEnabled = pPeerNode->erpIePresent;

            ibss_sta_info_update(pMac, pStaDs, pPeerNode);
            PELOGW(limLog(pMac, LOGW, FL("initiating ADD STA for the IBSS peer.\n"));)
            retCode = limAddSta(pMac, pStaDs);
            if(retCode != eSIR_SUCCESS)
            {
                PELOGE(limLog(pMac, LOGE, FL("ibss-sta-add failed (reason %x)\n"), retCode);)
                limPrintMacAddr(pMac, *pPeerAddr, LOGE);
                if(NULL != pStaDs)
                {
                    pStaDs->mlmStaContext.mlmState = prevState;
                    dphDeleteHashEntry(pMac, pStaDs->staAddr, pStaDs->assocId);
                }
            }
            else
            {
                if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
                    limIbssDecideProtection(pMac, pStaDs, &beaconParams);

                if(beaconParams.paramChangeBitmap)
                {
                    PELOGE(limLog(pMac, LOGE, FL("---> Update Beacon Params \n"));)
                    schSetFixedBeaconFields(pMac);    
                    limSendBeaconParams(pMac, &beaconParams );
                }
            }
        }
        else
        {
            PELOGE(limLog(pMac, LOGE, FL("hashTblAdd failed (reason %x)\n"), retCode);)
            limPrintMacAddr(pMac, *pPeerAddr, LOGE);
        }
    }
    else
    {
        retCode = eSIR_FAILURE;
    }

    return retCode;
}

/* handle the response from HAL for an ADD STA request */
tSirRetStatus
limIbssAddStaRsp(
    tpAniSirGlobal  pMac,
    void *msg)
{
    tpDphHashNode   pStaDs;
    tANI_U16        aid;
    tpAddStaParams  pAddStaParams = (tpAddStaParams) msg;

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    if (pAddStaParams == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: ADD_STA_RSP with no body!\n"));)
        return eSIR_FAILURE;
    }

    pStaDs = dphLookupHashEntry(pMac, pAddStaParams->staMac, &aid);
    if (pStaDs == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: ADD_STA_RSP for unknown MAC addr "));)
        limPrintMacAddr(pMac, pAddStaParams->staMac, LOGE);
        palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
        return eSIR_FAILURE;
    }

    if (pAddStaParams->status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: ADD_STA_RSP error (%x) "), pAddStaParams->status);)
        limPrintMacAddr(pMac, pAddStaParams->staMac, LOGE);
        palFreeMemory( pMac->hHdd, (void *) pAddStaParams );
        return eSIR_FAILURE;
    }

    pStaDs->bssId                  = pAddStaParams->bssIdx;
    pStaDs->staIndex               = pAddStaParams->staIdx;
    pStaDs->valid                  = 1;
    pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;

    PELOGW(limLog(pMac, LOGW, FL("IBSS: sending IBSS_NEW_PEER msg to SME!\n"));)

    ibss_status_chg_notify(pMac, pAddStaParams->staMac, pStaDs->staIndex, eWNI_SME_IBSS_NEW_PEER_IND);
    palFreeMemory( pMac->hHdd, (void *) pAddStaParams );

    return eSIR_SUCCESS;
}



void limIbssDelBssRspWhenCoalescing(tpAniSirGlobal  pMac,  void *msg)
{
   tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;

    PELOGW(limLog(pMac, LOGW, FL("IBSS: DEL_BSS_RSP Rcvd during coalescing!\n"));)

    if (pDelBss == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: DEL_BSS_RSP(coalesce) with no body!\n"));)
        goto end;
    }

    if (pDelBss->status != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGE, FL("IBSS: DEL_BSS_RSP(coalesce) error (%x) Bss %d "),
               pDelBss->status, pDelBss->bssIdx);
        goto end;
    }
    //Delete peer entries.
    limIbssDeleteAllPeers(pMac);

    /* add the new bss */
    ibss_bss_add(pMac);

    end:
    if(pDelBss != NULL)
        palFreeMemory( pMac->hHdd, (void *) pDelBss );
}



void limIbssAddBssRspWhenCoalescing(tpAniSirGlobal  pMac, void *msg)
{
    tANI_U8           infoLen;
    tSirSmeNewBssInfo newBssInfo;

   tpAddBssParams pAddBss = (tpAddBssParams) msg;

    tpSirMacMgmtHdr   pHdr    = (tpSirMacMgmtHdr)   pMac->lim.ibssInfo.pHdr;
    tpSchBeaconStruct pBeacon = (tpSchBeaconStruct) pMac->lim.ibssInfo.pBeacon;

    if ((pHdr == NULL) || (pBeacon == NULL))
    {
        PELOGE(limLog(pMac, LOGE, FL("Unable to handle AddBssRspWhenCoalescing (no cached BSS info)\n"));)
        goto end;
    }

    // Inform Host of IBSS coalescing
    infoLen = sizeof(tSirMacAddr) + sizeof(tSirMacChanNum) +
              sizeof(tANI_U8) + pBeacon->ssId.length + 1;

    palZeroMemory(pMac->hHdd, (void *) &newBssInfo, sizeof(newBssInfo));
    palCopyMemory( pMac->hHdd, newBssInfo.bssId, pHdr->bssId, sizeof(tSirMacAddr));
    newBssInfo.channelNumber = (tSirMacChanNum) pAddBss->currentOperChannel;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &newBssInfo.ssId,
                  (tANI_U8 *) &pBeacon->ssId, pBeacon->ssId.length + 1);

    PELOGW(limLog(pMac, LOGW, FL("Sending JOINED_NEW_BSS notification to SME.\n"));)

    limSendSmeWmStatusChangeNtf(pMac, eSIR_SME_JOINED_NEW_BSS,
                                (tANI_U32 *) &newBssInfo,
                                infoLen);

    end:
    ibss_coalesce_free(pMac);



}




void
limIbssDelBssRsp(
    tpAniSirGlobal  pMac,
    void *msg)
{
    tSirResultCodes rc = eSIR_SME_SUCCESS;
    tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;

    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);

    /*
    * If delBss was issued as part of IBSS Coalescing, gLimIbssCoalescingHappened flag will be true.
    * BSS has to be added again in this scenario, so this case needs to be handled separately.
    * If delBss was issued as a result of trigger from SME_STOP_BSS Request, then limSme state changes to
    * 'IDLE' and gLimIbssCoalescingHappened flag will be false. In this case STOP BSS RSP has to be sent to SME.
    */
    if(true == pMac->lim.gLimIbssCoalescingHappened)
    {
        limIbssDelBssRspWhenCoalescing(pMac,msg);
        return;
    }

    
    if (pDelBss == NULL)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: DEL_BSS_RSP with no body!\n"));)
        rc = eSIR_SME_REFUSED;
        goto end;
    }

    if (pDelBss->status != eHAL_STATUS_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: DEL_BSS_RSP error (%x) Bss %d "),
               pDelBss->status, pDelBss->bssIdx);)
        rc = eSIR_SME_STOP_BSS_FAILURE;
        goto end;
    }



    if(limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, pMac->lim.gLimBssid) != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("IBSS: DEL_BSS_RSP setLinkState failed\n"));)
        rc = eSIR_SME_REFUSED;
        goto end;
    }

    limDeletePreAuthList(pMac);
	/* First we will indicate SME about peer departed then we will reinitialize the dphHashTable using dphHashTableClassInit() */
    limIbssDelete(pMac);
    dphHashTableClassInit(pMac);
    pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));

    pMac->lim.gLimSystemRole = eLIM_STA_ROLE;

    end:
    limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, rc);
    if(pDelBss != NULL)
        palFreeMemory( pMac->hHdd, (void *) pDelBss );
    
}

/**
 * limIbssCoalesce()
 *
 *FUNCTION:
 * This function is called upon receiving Beacon/Probe Response
 * while operating in IBSS mode.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 *
 * @return Status whether to process or ignore received Beacon Frame
 */

tSirRetStatus
limIbssCoalesce(
    tpAniSirGlobal      pMac,
    tpSirMacMgmtHdr     pHdr,
    tpSchBeaconStruct   pBeacon,
    tANI_U8              *pIEs,
    tANI_U32            ieLen,
    tANI_U16            fTsfLater)
{
    tANI_U16            aid;
    tANI_U32            cfg = sizeof(tSirMacAddr);
    tSirMacAddr         currentBssId;
    tLimIbssPeerNode    *pPeerNode;
    tpDphHashNode       pStaDs;
    tUpdateBeaconParams beaconParams; 

    palZeroMemory( pMac->hHdd, (tANI_U8 *) &beaconParams, sizeof(tUpdateBeaconParams));

    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) != eSIR_SUCCESS)
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));

    /* Check for IBSS Coalescing only if Beacon is from different BSS */
    if ( !palEqualMemory( pMac->hHdd, currentBssId, pHdr->bssId, sizeof( tSirMacAddr ) ) )
    {
        if (! fTsfLater) // No Coalescing happened.
            return eSIR_LIM_IGNORE_BEACON;
        /*
         * IBSS Coalescing happened.
         * save the received beacon, and delete the current BSS. The rest of the
         * processing will be done in the delBss response processing
         */
        pMac->lim.gLimIbssCoalescingHappened = true;
        PELOGW(limLog(pMac, LOGW, FL("IBSS Coalescing happened\n"));)
        ibss_coalesce_save(pMac, pHdr, pBeacon);
        ibss_bss_delete(pMac);
        return eSIR_SUCCESS;
    }

    // STA in IBSS mode and SSID matches with ours
    pPeerNode = ibss_peer_find(pMac, pHdr->sa);
    if (pPeerNode == NULL)
    {
        /* Peer not in the list - Collect BSS description & add to the list */
        tANI_U32      frameLen;
        tSirRetStatus retCode;
        PELOGW(limLog(pMac, LOGW, FL("IBSS Peer node does not exist, adding it***\n"));)

#ifndef ANI_SIR_IBSS_PEER_CACHING
		/** Limit the Max number of IBSS Peers allowed as the max number of STA's allowed
		  */
		if (pMac->lim.gLimNumIbssPeers >= pMac->lim.maxStation)
			return eSIR_LIM_MAX_STA_REACHED_ERROR;
#endif		
        frameLen = sizeof(tLimIbssPeerNode) + ieLen - sizeof(tANI_U32);

        if (eHAL_STATUS_SUCCESS !=
            palAllocateMemory(pMac->hHdd, (void **) &pPeerNode, (tANI_U16)frameLen))
            limLog(pMac, LOGP, FL("alloc fail (%d bytes) storing IBSS peer info\n"),
                   frameLen);

        pPeerNode->beacon = NULL;
        pPeerNode->beaconLen = 0;

        ibss_peer_collect(pMac, pBeacon, pHdr, pPeerNode);
        if(eHAL_STATUS_SUCCESS != 
                palAllocateMemory(pMac->hHdd, (void**)&pPeerNode->beacon, ieLen))
       {
                PELOGE(limLog(pMac, LOGE, FL("Unable to allocate memory to store beacon"));)
        }
        else
        {
            palCopyMemory(pMac->hHdd, pPeerNode->beacon, pIEs, ieLen);
            pPeerNode->beaconLen = (tANI_U16)ieLen;
        }
        ibss_peer_add(pMac, pPeerNode);

        pStaDs = dphLookupHashEntry(pMac, pPeerNode->peerMacAddr, &aid);
        if (pStaDs != NULL)
        {
            /// DPH node already exists for the peer
            PELOGW(limLog(pMac, LOGW, FL("DPH Node present for just learned peer\n"));)
            PELOG1(limPrintMacAddr(pMac, pPeerNode->peerMacAddr, LOG1);)
            ibss_sta_info_update(pMac, pStaDs, pPeerNode);
        }
        retCode = limIbssStaAdd(pMac, pPeerNode->peerMacAddr);
        if (retCode != eSIR_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("lim-ibss-sta-add failed (reason %x)\n"), retCode);)
            limPrintMacAddr(pMac, pPeerNode->peerMacAddr, LOGE);
            return retCode;
        }

        // Decide protection mode
        pStaDs = dphLookupHashEntry(pMac, pPeerNode->peerMacAddr, &aid);
        if(pMac->lim.gLimProtectionControl != WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
            limIbssDecideProtection(pMac, pStaDs, &beaconParams);

        if(beaconParams.paramChangeBitmap)
        {
            PELOGE(limLog(pMac, LOGE, FL("beaconParams.paramChangeBitmap=1 ---> Update Beacon Params \n"));)
            schSetFixedBeaconFields(pMac);    
            limSendBeaconParams(pMac, &beaconParams );
        }
    }
    else
	{
        ibss_sta_caps_update(pMac, pPeerNode);
	}

    if (pMac->lim.gLimSmeState != eLIM_SME_NORMAL_STATE)
        return eSIR_SUCCESS;

    // Received Beacon from same IBSS we're
    // currently part of. Inform Roaming algorithm
    // if not already that IBSS is active.
    if (pMac->lim.gLimIbssActive == false)
    {
        limResetHBPktCount(pMac);
        PELOGW(limLog(pMac, LOGW, FL("Partner joined our IBSS, Sending IBSS_ACTIVE Notification to SME\n"));)
        pMac->lim.gLimIbssActive = true;
        limSendSmeWmStatusChangeNtf(pMac, eSIR_SME_IBSS_ACTIVE, NULL, 0);
        //in case heart beat timer is not activate
        limDeactivateAndChangeTimer(pMac, eLIM_HEART_BEAT_TIMER);
        MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_HEART_BEAT_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimHeartBeatTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("could not activate Heartbeat timer\n"));
    }

    return eSIR_SUCCESS;
} /*** end limHandleIBSScoalescing() ***/


void limIbssHeartBeatHandle(tpAniSirGlobal pMac)
{
    tLimIbssPeerNode *pTempNode, *pPrevNode;
    tLimIbssPeerNode *pTempNextNode = NULL;
    tANI_U16      aid;
    tpDphHashNode pStaDs;
    tANI_U32 threshold;
	
	/** MLM BSS is started and if PE in scanmode then MLM state will be waiting for probe resp.
	 *  If Heart beat timeout triggers during this corner case then we need to reactivate HeartBeat timer 
	 */
	if(pMac->lim.gLimMlmState != eLIM_MLM_BSS_STARTED_STATE) {
			limReactivateTimer(pMac, eLIM_HEART_BEAT_TIMER);
			return;
	}
	/** If LinkMonitor is Disabled */
	if(!pMac->sys.gSysEnableLinkMonitorMode)
			return;

    pPrevNode = pTempNode  = pMac->lim.gLimIbssPeerList;
    threshold = (pMac->lim.gLimNumIbssPeers / 4 ) + 1;

	/** Monitor the HeartBeat with the Individual PEERS in the IBSS */
    while (pTempNode != NULL) 
    {
        pTempNextNode = pTempNode->next;
        if(pTempNode->beaconHBCount) //There was a beacon for this peer during heart beat.
        {
            pTempNode->beaconHBCount = 0;
            pTempNode->heartbeatFailure = 0;
        }
        else //There wasnt any beacon recieved during heartbeat timer.
         {
            pTempNode->heartbeatFailure++;
            PELOGE(limLog(pMac, LOGE, FL("Heartbeat fail = %d  thres = %d"), pTempNode->heartbeatFailure, pMac->lim.gLimNumIbssPeers);)
            if(pTempNode->heartbeatFailure >= threshold )
            {
                pTempNextNode = pTempNode->next;
                //Remove this entry from the list.
                pStaDs = dphLookupHashEntry(pMac, pTempNode->peerMacAddr, &aid);
                if (pStaDs)
                {
                    (void) limDelSta(pMac, pStaDs, false /*asynchronous*/);
                    limDeleteDphHashEntry(pMac, pStaDs->staAddr, aid);
                }
                if(pTempNode == pMac->lim.gLimIbssPeerList)
                {
                    pMac->lim.gLimIbssPeerList = pTempNode->next;
                    pPrevNode = pMac->lim.gLimIbssPeerList; 
                }
                else
                    pPrevNode->next = pTempNode->next;

                palFreeMemory(pMac->hHdd,pTempNode);
                pMac->lim.gLimNumIbssPeers--;
 
                //Send indication.
                ibss_status_chg_notify( pMac, pTempNode->peerMacAddr, pStaDs->staIndex, eWNI_SME_IBSS_PEER_DEPARTED_IND );
                pTempNode = pTempNextNode; //Since we deleted current node, prevNode remains same.
                continue;
             }
         }
        
        pPrevNode = pTempNode;
        pTempNode = pTempNextNode;
    }

	/** General IBSS Activity Monitor, check if in IBSS Mode we are received any Beacons */
    if(pMac->lim.gLimNumIbssPeers)
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
	else 
	{

    	PELOGW(limLog(pMac, LOGW, FL("Heartbeat Failure\n"));)
	    pMac->lim.gLimHBfailureCntInLinkEstState++;

		if (pMac->lim.gLimIbssActive == true)
	    {
    		// We don't receive Beacon frames from any
        	// other STA in IBSS. Announce IBSS inactive
	        // to Roaming algorithm
    	    PELOGW(limLog(pMac, LOGW, FL("Alone in IBSS\n"));)
        	pMac->lim.gLimIbssActive = false;

	        limSendSmeWmStatusChangeNtf(pMac, eSIR_SME_IBSS_INACTIVE,
                                          NULL, 0);
		}
	}
}


/** -------------------------------------------------------------
\fn limIbssDecideProtectionOnDelete
\brief Decides all the protection related information.
\
\param  tpAniSirGlobal    pMac
\param  tSirMacAddr peerMacAddr
\param  tpUpdateBeaconParams pBeaconParams
\return None
  -------------------------------------------------------------*/
void
limIbssDecideProtectionOnDelete(tpAniSirGlobal pMac, 
      tpDphHashNode pStaDs, tpUpdateBeaconParams pBeaconParams)
{
    tANI_U32 phyMode;
    tHalBitVal erpEnabled = eHAL_CLEAR;
    tSirRFBand rfBand = SIR_BAND_UNKNOWN;
    tANI_U32 i;    
    
    if(NULL == pStaDs)
      return;

    limGetRfBand(pMac, &rfBand);
    if(SIR_BAND_2_4_GHZ == rfBand)
    {
        limGetPhyMode(pMac, &phyMode);
        erpEnabled = pStaDs->erpEnabled;
        //we are HT or 11G and 11B station is getting deleted.
        if ( ((phyMode == WNI_CFG_PHY_MODE_11G) || pMac->lim.htCapability) 
              && (erpEnabled == eHAL_CLEAR))
        {
            PELOGE(limLog(pMac, LOGE, FL("(%d) A legacy STA is disassociated. Addr is "),
                   pMac->lim.gLim11bParams.numSta);
            limPrintMacAddr(pMac, pStaDs->staAddr, LOGE);)
            if (pMac->lim.gLim11bParams.numSta > 0)
            {
                for (i=0; i<LIM_PROT_STA_CACHE_SIZE; i++)
                {
                    if (pMac->lim.protStaCache[i].active)
                    {
                        if (palEqualMemory( pMac->hHdd,pMac->lim.protStaCache[i].addr,
                                pStaDs->staAddr, sizeof(tSirMacAddr)))
                        {
                            pMac->lim.gLim11bParams.numSta--;
                            pMac->lim.protStaCache[i].active = false;
                            break;
                        }
                    }
                }
            }

            if (pMac->lim.gLim11bParams.numSta == 0)
            {
                PELOGE(limLog(pMac, LOGE, FL("No more 11B STA exists. Disable protection. \n"));)
				limIbssSetProtection(pMac, false, pBeaconParams);
            }
        }
    }
}
