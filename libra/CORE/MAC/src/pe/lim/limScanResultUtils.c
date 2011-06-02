/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limScanResultUtils.cc contains the utility functions
 * LIM uses for maintaining and accessing scan results on STA.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "limTypes.h"
#include "limUtils.h"
#include "limSerDesUtils.h"
#include "limApi.h"
#include "limSession.h"



/**
 * limDeactiveMinChannelTimerDuringScan()
 *
 *FUNCTION:
 * This function is called during scan upon receiving
 * Beacon/Probe Response frame to deactivate MIN channel
 * timer if running.
 *
 * This function should be called only when pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 *
 * @return eSIR_SUCCESS in case of success
 */

tANI_U32
limDeactivateMinChannelTimerDuringScan(tpAniSirGlobal pMac)
{
    if ((pMac->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE) && (pMac->lim.gLimHalScanState == eLIM_HAL_SCANNING_STATE))
    {
        /**
            * Beacon/Probe Response is received during active scanning.
            * Deactivate MIN channel timer if running.
            */
        
        limDeactivateAndChangeTimer(pMac,eLIM_MIN_CHANNEL_TIMER);
        MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_MAX_CHANNEL_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimMaxChannelTimer)
                                          == TX_TIMER_ERROR)
        {
            /// Could not activate max channel timer.
            // Log error
            limLog(pMac,LOGP, FL("could not activate max channel timer\n"));

            limCompleteMlmScan(pMac, eSIR_SME_RESOURCES_UNAVAILABLE);
            return TX_TIMER_ERROR;
        }
    }
    return eSIR_SUCCESS;
} /*** end limDeactivateMinChannelTimerDuringScan() ***/



/**
 * limCollectBssDescription()
 *
 *FUNCTION:
 * This function is called during scan upon receiving
 * Beacon/Probe Response frame to check if the received
 * frame matches scan criteria, collect BSS description
 * and add it to cached scan results.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pBPR - Pointer to parsed Beacon/Probe Response structure
 * @param  pBd  - Pointer to Received frame's BD
 *
 * @return None
 */

void
limCollectBssDescription(tpAniSirGlobal pMac,
                         tSirBssDescription *pBssDescr,
                         tpSirProbeRespBeacon pBPR,
                         tANI_U32 *pBd)
{
    tANI_U8             *pBody;
    tANI_U32            ieLen = 0;
    tpSirMacMgmtHdr     pHdr;
    tANI_U8             channelNum;
    tANI_U8             rxChannel;

    pHdr = SIR_MAC_BD_TO_MPDUHEADER(pBd);
    ieLen    = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd) - SIR_MAC_B_PR_SSID_OFFSET;
    rxChannel = SIR_MAC_BD_TO_RX_CHANNEL(pBd);
    pBody = SIR_MAC_BD_TO_MPDUDATA(pBd);


    /**
     * Length of BSS desription is without length of
     * length itself and length of pointer
     * that holds the next BSS description
     */
    pBssDescr->length = (tANI_U16)(
                    sizeof(tSirBssDescription) - sizeof(tANI_U16) -
                    sizeof(tANI_U32) + ieLen);

    // Copy BSS Id
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pBssDescr->bssId,
                  (tANI_U8 *) pHdr->bssId,
                  sizeof(tSirMacAddr));

    // Copy Timestamp, Beacon Interval and Capability Info
    pBssDescr->timeStamp[0]   = pBPR->timeStamp[0];
    pBssDescr->timeStamp[1]   = pBPR->timeStamp[1];
    pBssDescr->beaconInterval = pBPR->beaconInterval;
    pBssDescr->capabilityInfo = limGetU16((tANI_U8 *) &pBPR->capabilityInfo);


    /*
    * There is a narrow window after Channel Switch msg is sent to HAL and before the AGC is shut
    * down and beacons/Probe Rsps can trickle in and we may report the incorrect channel in 5Ghz
    * band, so not relying on the 'last Scanned Channel' stored in LIM.
    * Instead use the value returned by RXP in BD. This the the same value which HAL programs into
    * RXP before every channel switch.
    * Right now there is a problem in 5Ghz, where we are receiving beacons from a channel different from
    * the currently scanned channel. so incorrect channel is reported to CSR and association does not happen.
    * So for now we keep on looking for the channel info in the beacon (DSParamSet IE OR HT Info IE), and only if it
    * is not present in the beacon, we go for the channel info present in RXP.
    * This fix will work for 5Ghz 11n devices, but for 11a devices, we have to rely on RXP routing flag to get the correct channel.
    * So The problem of incorrect channel reporting in 5Ghz will still remain for 11a devices.
    */
    pBssDescr->channelId = limGetChannelFromBeacon(pMac, pBPR);

	if (pBssDescr->channelId == 0)
       pBssDescr->channelId = rxChannel;

    pBssDescr->channelIdSelf = rxChannel;
    pBssDescr->titanHtCaps = 0;

    //FIXME_CBMODE : need to seperate out TITAN and HT CB mode.
    //HT neighbor with channel bonding
    if( pBPR->HTCaps.present  )
    {
        tAniTitanHtCapabilityInfo titanHtCaps = 0;
        limGetHtCbAdminState(pMac, pBPR->HTCaps, &titanHtCaps);
        if( pBPR->HTInfo.present &&
          pBPR->HTInfo.secondaryChannelOffset )
        {

            limGetHtCbOpState( pMac,
                pBPR->HTInfo,
                &titanHtCaps );
        }
        pBssDescr->titanHtCaps = (tANI_U32) titanHtCaps;
    }

    // Is this is a TITAN neighbor?
    else if( pBPR->propIEinfo.aniIndicator &&
        pBPR->propIEinfo.titanPresent )
    {
    tAniTitanHtCapabilityInfo titanHtCaps = 0;
      pBssDescr->titanHtCaps = (tANI_U32) titanHtCaps;
    }

    //set the network type in bss description
    channelNum = pBssDescr->channelId;
	pBssDescr->nwType = limGetNwType(pMac, channelNum, SIR_MAC_MGMT_FRAME, pBPR);

    pBssDescr->aniIndicator = pBPR->propIEinfo.aniIndicator;

    // Copy RSSI & SINR from BD



    PELOG4(limLog(pMac, LOG4, "***********BSS Description for BSSID:*********** ");
    sirDumpBuf(pMac, SIR_LIM_MODULE_ID, LOG4, pBssDescr->bssId, 6 );
    sirDumpBuf( pMac, SIR_LIM_MODULE_ID, LOG4, (tANI_U8*)pBd, 36 );)

    pBssDescr->rssi = (tANI_S8)SIR_MAC_BD_TO_RSSI_DB(pBd);
    
    //SINR no longer reported by HW
    pBssDescr->sinr = 0;

    pBssDescr->nReceivedTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

    // Copy IE fields
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pBssDescr->ieFields,
                  pBody + SIR_MAC_B_PR_SSID_OFFSET,
                  ieLen);

    //sirDumpBuf( pMac, SIR_LIM_MODULE_ID, LOGW, (tANI_U8 *) pBssDescr, pBssDescr->length + 2 );
    limLog( pMac, LOG3,
        FL("Collected BSS Description for Channel(%1d), length(%u), aniIndicator(%d), IE Fields(%u)\n"),
        pBssDescr->channelId,
        pBssDescr->length,
        pBssDescr->aniIndicator,
        ieLen );

    return;
} /*** end limCollectBssDescription() ***/

/**
 * limIsScanRequestedSSID()
 *
 *FUNCTION:
 * This function is called during scan upon receiving
 * Beacon/Probe Response frame to check if the received
 * SSID is present in the list of requested SSIDs in scan
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  ssId - SSID Received in beacons/Probe responses that is compared against the 
                            requeusted SSID in scan list
 * ---------------------------------------------
 *
 * @return boolean - TRUE if SSID is present in requested list, FALSE otherwise
 */

tANI_BOOLEAN limIsScanRequestedSSID(tpAniSirGlobal pMac, tSirMacSSid *ssId)
{
    tANI_U8 i = 0;
    tANI_BOOLEAN requestedSsid = eANI_BOOLEAN_FALSE;

    for (i = 0; i < pMac->lim.gpLimMlmScanReq->numSsid; i++)
    {
        if ((requestedSsid = palEqualMemory( pMac->hHdd,(tANI_U8 *) ssId,
                   (tANI_U8 *) &pMac->lim.gpLimMlmScanReq->ssId[i],
                   (tANI_U8) (pMac->lim.gpLimMlmScanReq->ssId[i].length))) == eANI_BOOLEAN_TRUE)
        {
            break;
        }
    }
    return requestedSsid;
}

/**
 * limCheckAndAddBssDescription()
 *
 *FUNCTION:
 * This function is called during scan upon receiving
 * Beacon/Probe Response frame to check if the received
 * frame matches scan criteria, collect BSS description
 * and add it to cached scan results.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pBPR - Pointer to parsed Beacon/Probe Response structure
 * @param  pBd  - Pointer to Received frame's BD
 * @param  fScanning - boolean to indicate whether the BSS is from current scan or just happen to receive a beacon
 *
 * @return None
 */

void
limCheckAndAddBssDescription(tpAniSirGlobal pMac,
                             tpSirProbeRespBeacon pBPR,
                             tANI_U32 *pBd,
                             tANI_BOOLEAN fScanning)
{
    tLimScanResultNode   *pBssDescr;
    tANI_U32                  frameLen, ieLen = 0;

    /**
     * Compare SSID with the one sent in
     * Probe Request frame, if any.
     * If they don't match, ignore the
     * Beacon frame.
     * pMac->lim.gLimMlmScanReq->ssId.length == 0
     * indicates Broadcast SSID.
     */

    if ((fScanning) && ( pMac->lim.gLimReturnAfterFirstMatch & 0x01 ) 
        && (pMac->lim.gpLimMlmScanReq->numSsid) &&
                   !limIsScanRequestedSSID(pMac, &pBPR->ssId))
    {
        /**
         * Received SSID does not match with
         * the one we're scanning for.
         * Ignore received Beacon frame
         */

        return;
    }

    /* There is no point in caching & reporting the scan results for APs
     * which are in the process of switching the channel. So, we are not
     * caching the scan results for APs which are adverzing the channel-switch
     * element in their beacons and probe responses.
     */
    if(pBPR->channelSwitchPresent)
    {
        return;
    }

    /**
     * Allocate buffer to hold BSS description from
     * received Beacon frame.
     * Include size of fixed fields and IEs length
     */

    ieLen    = SIR_MAC_BD_TO_PAYLOAD_LEN(pBd) - SIR_MAC_B_PR_SSID_OFFSET;
    frameLen = sizeof(tLimScanResultNode) + ieLen - sizeof(tANI_U32);   // Sizeof(tANI_U32) is for ieFields[1]

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pBssDescr, frameLen))
    {
        // Log error
        limLog(pMac, LOGP,
           FL("call for palAllocateMemory failed for storing BSS description\n"));

        return;
    }

    // In scan state, store scan result.
    limCollectBssDescription(pMac, &pBssDescr->bssDescription,
                             pBPR, pBd);

    pBssDescr->next = NULL;

    /**
     * Depending on whether to store unique or all
     * scan results, pass hash update/add parameter
     */

    //If it is not scanning, only save unique results
    if (pMac->lim.gLimReturnUniqueResults || (!fScanning))
        limLookupNaddHashEntry(pMac, pBssDescr, LIM_HASH_UPDATE);
    else
         limLookupNaddHashEntry(pMac, pBssDescr, LIM_HASH_ADD);

    if(fScanning)
    {
        if ((pBssDescr->bssDescription.channelId <= 14) &&
            (pMac->lim.gLimReturnAfterFirstMatch & 0x40) &&
            pBPR->countryInfoPresent)
            pMac->lim.gLim24Band11dScanDone = 1;

        if ((pBssDescr->bssDescription.channelId > 14) &&
            (pMac->lim.gLimReturnAfterFirstMatch & 0x80) &&
            pBPR->countryInfoPresent)
            pMac->lim.gLim50Band11dScanDone = 1;

        if ( ( pMac->lim.gLimReturnAfterFirstMatch & 0x01 ) ||
             ( pMac->lim.gLim24Band11dScanDone && ( pMac->lim.gLimReturnAfterFirstMatch & 0x40 ) ) ||
             ( pMac->lim.gLim50Band11dScanDone && ( pMac->lim.gLimReturnAfterFirstMatch & 0x80 ) ) )
/*
        if ((pMac->lim.gLimReturnAfterFirstMatch & 0x01) ||
            (pMac->lim.gLim24Band11dScanDone &&
             !(pMac->lim.gLimReturnAfterFirstMatch & 0xC0)) ||
            (pMac->lim.gLim50Band11dScanDone &&
             !(pMac->lim.gLimReturnAfterFirstMatch & 0xC0)) ||
            (pMac->lim.gLim24Band11dScanDone &&
             pMac->lim.gLim50Band11dScanDone &&
             pMac->lim.gLimReturnAfterFirstMatch & 0xC0))
*/
        {
            /**
             * Stop scanning and return the BSS description(s)
             * collected so far.
             */
            limLog(pMac,
                   LOGW,
                   FL("Completed scan: 24Band11dScan = %d, 50Band11dScan = %d BSS id\n"),
                   pMac->lim.gLim24Band11dScanDone,
                   pMac->lim.gLim50Band11dScanDone);

            limSendHalFinishScanReq( pMac, eLIM_HAL_FINISH_SCAN_WAIT_STATE );
        }
    }//(eANI_BOOLEAN_TRUE == fScanning)
} /****** end limCheckAndAddBssDescription() ******/



/**
 * limScanHashFunction()
 *
 *FUNCTION:
 * This function is called during scan hash entry operations
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  bssId - Received BSSid
 *
 * @return Hash index
 */

tANI_U8
limScanHashFunction(tSirMacAddr bssId)
{
    tANI_U16    i, hash = 0;

    for (i = 0; i < sizeof(tSirMacAddr); i++)
        hash += bssId[i];

    return hash % LIM_MAX_NUM_OF_SCAN_RESULTS;
} /****** end limScanHashFunction() ******/



/**
 * limInitHashTable()
 *
 *FUNCTION:
 * This function is called upon receiving SME_START_REQ
 * to initialize global cached scan hash table
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limInitHashTable(tpAniSirGlobal pMac)
{
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
        pMac->lim.gLimCachedScanHashTable[i] = NULL;
} /****** end limInitHashTable() ******/



/**
 * limLookupNaddHashEntry()
 *
 *FUNCTION:
 * This function is called upon receiving a Beacon or
 * Probe Response frame during scan phase to store
 * received BSS description into scan result hash table.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pBssDescr - Pointer to BSS description to be
 *         added to the scan result hash table.
 * @param  action - Indicates action to be performed
 *         when same BSS description is found. This is
 *         dependent on whether unique scan result to
 *         be stored or not.
 *
 * @return None
 */

void
limLookupNaddHashEntry(tpAniSirGlobal pMac,
                       tLimScanResultNode *pBssDescr, tANI_U8 action)
{
    tANI_U8                  index, ssidLen = 0;
    tANI_U8                found = false;
    tLimScanResultNode *ptemp, *pprev;
    tSirMacCapabilityInfo *pSirCap, *pSirCapTemp;

    index = limScanHashFunction(pBssDescr->bssDescription.bssId);
    ptemp = pMac->lim.gLimCachedScanHashTable[index];

    //ieFields start with TLV of SSID IE
    ssidLen = * ((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1);
    pSirCap = (tSirMacCapabilityInfo *)&pBssDescr->bssDescription.capabilityInfo;

    for (pprev = ptemp; ptemp; pprev = ptemp, ptemp = ptemp->next)
    {
        //For infrastructure, only check BSSID. For IBSS, check more
        pSirCapTemp = (tSirMacCapabilityInfo *)&ptemp->bssDescription.capabilityInfo;
        if((pSirCapTemp->ess == pSirCap->ess) && //matching ESS type first
            (palEqualMemory( pMac->hHdd,(tANI_U8 *) pBssDescr->bssDescription.bssId,
                      (tANI_U8 *) ptemp->bssDescription.bssId,
                      sizeof(tSirMacAddr))) &&   //matching BSSID
            ((pSirCapTemp->ess) ||    //we are done for infrastructure
            //For IBSS, matching SSID, nwType and channelId
            ((palEqualMemory( pMac->hHdd,((tANI_U8 *) &pBssDescr->bssDescription.ieFields + 1),
                           ((tANI_U8 *) &ptemp->bssDescription.ieFields + 1),
                           (tANI_U8) (ssidLen + 1)) &&
            (pBssDescr->bssDescription.nwType ==
                                         ptemp->bssDescription.nwType) &&
            (pBssDescr->bssDescription.channelId ==
                                      ptemp->bssDescription.channelId))))
        )
        {
            // Found the same BSS description
            if (action == LIM_HASH_UPDATE)
            {
                // Delete this entry
                if (ptemp == pMac->lim.gLimCachedScanHashTable[index])
                    pprev = pMac->lim.gLimCachedScanHashTable[index] = ptemp->next;
                else
                    pprev->next = ptemp->next;

                pMac->lim.gLimMlmScanResultLength -=
                    ptemp->bssDescription.length + sizeof(tANI_U16);

                palFreeMemory( pMac->hHdd, (tANI_U8 *) ptemp);
            }
            found = true;
            break;
        }
    }

    // Add this BSS description at same index
    if (pprev == pMac->lim.gLimCachedScanHashTable[index])
    {
        pBssDescr->next = pMac->lim.gLimCachedScanHashTable[index];
        pMac->lim.gLimCachedScanHashTable[index] = pBssDescr;
    }
    else
    {
        pBssDescr->next = pprev->next;
        pprev->next = pBssDescr;
    }
    pMac->lim.gLimMlmScanResultLength +=
        pBssDescr->bssDescription.length + sizeof(tANI_U16);

    PELOG2(limLog(pMac, LOG2, FL("Added new BSS description size %d TOT %d BSS id\n"),
           pBssDescr->bssDescription.length,
           pMac->lim.gLimMlmScanResultLength);
    limPrintMacAddr(pMac, pBssDescr->bssDescription.bssId, LOG2);)

    // Send new BSS found indication to HDD if CFG option is set
    if (!found) limSendSmeNeighborBssInd(pMac, pBssDescr);

    //
    // TODO: IF applicable, do we need to send:
    // Mesg - eWNI_SME_WM_STATUS_CHANGE_NTF
    // Status change code - eSIR_SME_CB_LEGACY_BSS_FOUND_BY_AP
    //
}



/**
 * limDeleteHashEntry()
 *
 *FUNCTION:
 * This function is called upon to delete
 * a BSS description from scan result hash table.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * Yet to find the utility of the function
 *
 * @param  pBssDescr - Pointer to BSS description to be
 *         deleted from the scan result hash table.
 *
 * @return None
 */

void    limDeleteHashEntry(tLimScanResultNode *pBssDescr)
{
} /****** end limDeleteHashEntry() ******/



/**
 * limCopyScanResult()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() while
 * sending SME_SCAN_RSP with scan result to HDD.
 *
 *LOGIC:
 * This function traverses the scan list stored in scan hash table
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pDest - Destination pointer
 *
 * @return None
 */

void
limCopyScanResult(tpAniSirGlobal pMac, tANI_U8 *pDest)
{
    tLimScanResultNode    *ptemp;
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((ptemp = pMac->lim.gLimCachedScanHashTable[i]) != NULL)
        {
            while(ptemp)
            {
                /// Copy entire BSS description including length
                palCopyMemory( pMac->hHdd, pDest,
                              (tANI_U8 *) &ptemp->bssDescription,
                              ptemp->bssDescription.length + 2);
                pDest += ptemp->bssDescription.length + 2;
                ptemp = ptemp->next;
            }
        }
    }
} /****** end limCopyScanResult() ******/



/**
 * limDeleteCachedScanResults()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() upon receiving
 * SME_SCAN_REQ with fresh scan result flag set.
 *
 *LOGIC:
 * This function traverses the scan list stored in scan hash table
 * and deletes the entries if any
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limDeleteCachedScanResults(tpAniSirGlobal pMac)
{
    tLimScanResultNode    *pNode, *pNextNode;
    tANI_U16 i;
    for (i = 0; i < LIM_MAX_NUM_OF_SCAN_RESULTS; i++)
    {
        if ((pNode = pMac->lim.gLimCachedScanHashTable[i]) != NULL)
        {
            while (pNode)
            {
                pNextNode = pNode->next;

                // Delete the current node
                palFreeMemory( pMac->hHdd, (tANI_U8 *) pNode);

                pNode = pNextNode;
            }
        }
    }

    pMac->lim.gLimSmeScanResultLength = 0;
} /****** end limDeleteCachedScanResults() ******/



/**
 * limReInitScanResults()
 *
 *FUNCTION:
 * This function is called delete exisiting scan results
 * and initialize the scan hash table
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void
limReInitScanResults(tpAniSirGlobal pMac)
{
    limDeleteCachedScanResults(pMac);
    limInitHashTable(pMac);

    // !!LAC - need to clear out the global scan result length
    // since the list was just purged from the hash table.
    pMac->lim.gLimMlmScanResultLength = 0;

} /****** end limReInitScanResults() ******/
