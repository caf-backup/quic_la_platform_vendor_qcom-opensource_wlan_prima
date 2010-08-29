/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrApiScan.c
  
    Implementation for the Common Scan interfaces.
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
 
   ========================================================================== */
#include "halInternal.h"
#include "palApi.h"
#include "csrInsideApi.h"
#include "smeInside.h"
#include "smsDebug.h"

#include "csrSupport.h"
#include "wlan_qct_tl.h"

#include "vos_diag_core_log.h"
#include "vos_diag_core_event.h"
                                                                     
                                                                     
#define CSR_SCAN_RESULT_RSSI_WEIGHT     80 // must be less than 100, represent the persentage of new RSSI
                                                                     
/*---------------------------------------------------------------------------
  PER filter constant fraction: it is a %
---------------------------------------------------------------------------*/  
#define CSR_SCAN_PER_FILTER_FRAC 100
                                                                     
/*---------------------------------------------------------------------------
  RSSI filter constant fraction: it is a %
---------------------------------------------------------------------------*/  
#define CSR_SCAN_RSSI_FILTER_FRAC 100

/*---------------------------------------------------------------------------
Convert RSSI into overall score: Since RSSI is in -dBm values, and the 
overall needs to be weighted inversely (where greater value means better
system), we convert.
RSSI *cannot* be more than 0xFF or less than 0 for meaningful WLAN operation
---------------------------------------------------------------------------*/
#define CSR_SCAN_MAX_SCORE_VAL 0xFF
#define CSR_SCAN_MIN_SCORE_VAL 0x0
#define CSR_SCAN_HANDOFF_DELTA 10
#define CSR_SCAN_OVERALL_SCORE( rssi ) \
  ( rssi < CSR_SCAN_MAX_SCORE_VAL )\
   ? (CSR_SCAN_MAX_SCORE_VAL-rssi) : CSR_SCAN_MIN_SCORE_VAL
                                                                     

#define CSR_SCAN_IS_OVER_BSS_LIMIT(pMac)  \
	( (pMac)->scan.nBssLimit <= (csrLLCount(&(pMac)->scan.scanResultList)) )

//*** This is temporary work around. It need to call CCM api to get to CFG later
/// Get string parameter value
extern tSirRetStatus wlan_cfgGetStr(tpAniSirGlobal, tANI_U16, tANI_U8*, tANI_U32*);
                                                                     
void csrScanGetResultTimerHandler(void *);
void csrScanResultAgingTimerHandler(void *pv);
void csrScanIdleScanTimerHandler(void *);
eHalStatus csrScanChannels( tpAniSirGlobal pMac, tSmeCmd *pCommand );
void csrSetCfgValidChannelList( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels );
void csrSaveTxPowerToCfg( tpAniSirGlobal pMac, tDblLinkList *pList, tANI_U32 cfgId );
void csrSetCfgCountryCode( tpAniSirGlobal pMac, tANI_U8 *countryCode );
void csrPurgeChannelPower( tpAniSirGlobal pMac, tDblLinkList *pChannelList );
//if bgPeriod is 0, background scan is disabled. It is in millisecond units
eHalStatus csrSetCfgBackgroundScanPeriod(tpAniSirGlobal pMac, tANI_U32 bgPeriod);
eHalStatus csrProcessSetBGScanParam(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReleaseScanCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus);
static tANI_BOOLEAN csrScanValidateScanResult( tpAniSirGlobal pMac, tANI_U8 *pChannels, 
                                               tANI_U8 numChn, tSirBssDescription *pBssDesc, 
                                               tDot11fBeaconIEs **ppIes );
eHalStatus csrSetBGScanChannelList( tpAniSirGlobal pMac, tANI_U8 *pAdjustChannels, tANI_U8 NumAdjustChannels);
void csrReleaseCmdSingle(tpAniSirGlobal pMac, tSmeCmd *pCommand);
tANI_BOOLEAN csrRoamIsValidChannel( tpAniSirGlobal pMac, tANI_U8 channel );

#ifdef FEATURE_WLAN_GEN6_ROAMING
extern VOS_STATUS csrRoamNtRssiIndCallback(tHalHandle hHal, 
                                           v_U8_t  rssiNotification, 
                                           void * context);
//HO
tCsrChannelInfo csrScanGetNextBgScanChannelList(tpAniSirGlobal pMac);
void csrScanGetCandChanList(tpAniSirGlobal pMac);
void csrScanUpdateOtherChanList(tpAniSirGlobal pMac);
void csrScanHoScanSuccess(tpAniSirGlobal pMac);
void csrScanHoScanFailure(tpAniSirGlobal pMac);
void csrScanUpdateHoLists(tpAniSirGlobal pMac);
void csrScanTrimHoListForChannel(tpAniSirGlobal pMac, tDblLinkList *pStaList, tANI_U8 channel);
tANI_BOOLEAN csrScanUpdateHoCandidateList(tpAniSirGlobal pMac,
                                          tCsrHandoffStaInfo *pStaEntry, 
                                          tCsrHandoffStaInfo **ppPoppedEntry);
void csrScanUpdateHoNeighborList( tpAniSirGlobal pMac,
                                  tCsrHandoffStaInfo *pStaEntry);
void csrScanInsertEntryIntoList( tpAniSirGlobal pMac,
                                 tDblLinkList *pStaList,
                                 tCsrHandoffStaInfo *pStaEntry);
void csrScanListRemoveTail( tpAniSirGlobal pMac,
                            tDblLinkList *pStaList, 
                            tCsrHandoffStaInfo **ppStaEntry );
void csrScanListUpdateBssEntry( tpAniSirGlobal pMac,
                            tDblLinkList *pStaList, 
                            tCsrHandoffStaInfo *pStaEntry );
tANI_BOOLEAN csrScanPmkCacheExistsForBssid(tpAniSirGlobal pMac, tCsrBssid bssid ); 
#ifdef FEATURE_WLAN_WAPI
tANI_BOOLEAN csrScanBkCacheExistsForBssid(tpAniSirGlobal pMac, tCsrBssid bssid ); 
#endif /* FEATURE_WLAN_WAPI */
tANI_S8 csrScanUpdateRssi(tpAniSirGlobal pMac, tANI_S8  scanRssi,
                          tANI_S8  oldRssi);

void csrScanBgScanTimerHandler(void *pv);

eHalStatus csrScanSendNoTrafficBgScanReq(tpAniSirGlobal pMac, tCsrBGScanRequest * pBgScanParams);
eHalStatus csrScanSendInTrafficBgScanReq(tpAniSirGlobal pMac, tCsrBGScanRequest *pBgScanParams);
eHalStatus csrScanCreateOtherChanList(tpAniSirGlobal pMac);
eHalStatus csrScanGetScanHoCandidate(tpAniSirGlobal pMac);
tANI_U32 csrScanGetQosScore(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes);
tANI_U32 csrScanGetSecurityScore(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes);
void csrScanUpdateNList(tpAniSirGlobal pMac);
void csrScanDisplayList(tpAniSirGlobal pMac,
                        tDblLinkList *pStaList);
#ifdef FEATURE_WLAN_DIAG_SUPPORT
void csrScanDiagHoLog(tpAniSirGlobal pMac);
#endif
#endif //FEATURE_WLAN_GEN6_ROAMING
//pResult is invalid calling this function.
void csrFreeScanResultEntry( tpAniSirGlobal pMac, tCsrScanResult *pResult )
{
    if( NULL != pResult->Result.pvIes )
    {
        palFreeMemory( pMac->hHdd, pResult->Result.pvIes );
    }
    palFreeMemory(pMac->hHdd, pResult);
}


static eHalStatus csrLLScanPurgeResult(tpAniSirGlobal pMac, tDblLinkList *pList)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry;
    tCsrScanResult *pBssDesc;
    
    csrLLLock(pList);
    
    while((pEntry = csrLLRemoveHead(pList, LL_ACCESS_NOLOCK)) != NULL)
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
        csrFreeScanResultEntry( pMac, pBssDesc );
    }
    
    csrLLUnlock(pList);   
     
    return (status);
}

eHalStatus csrScanOpen( tpAniSirGlobal pMac )
{
    eHalStatus status;
    
    do
    {
        csrLLOpen(pMac->hHdd, &pMac->scan.scanResultList);
        csrLLOpen(pMac->hHdd, &pMac->scan.tempScanResults);
        csrLLOpen(pMac->hHdd, &pMac->scan.channelPowerInfoList24);
        csrLLOpen(pMac->hHdd, &pMac->scan.channelPowerInfoList5G);
        pMac->scan.fFullScanIssued = eANI_BOOLEAN_FALSE;
        pMac->scan.nBssLimit = CSR_MAX_BSS_SUPPORT;
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerGetResult, csrScanGetResultTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for getResult timer\n"));
            break;
        }
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerIdleScan, csrScanIdleScanTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for idleScan timer\n"));
            break;
        }
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerResultAging, csrScanResultAgingTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("cannot allocate memory for ResultAging timer\n"));
            break;
        }
#ifdef FEATURE_WLAN_GEN6_ROAMING
        /* Define the scan timer                                                 */
        status = palTimerAlloc(pMac->hHdd, &pMac->scan.hTimerBgScan, csrScanBgScanTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
           smsLog(pMac, LOGE, FL("cannot allocate memory for BgScan timer\n"));
           return eHAL_STATUS_FAILURE;
        }
#endif
    }while(0);
    
    return (status);
}


eHalStatus csrScanClose( tpAniSirGlobal pMac )
{
    
    csrLLScanPurgeResult(pMac, &pMac->scan.tempScanResults);
    csrLLScanPurgeResult(pMac, &pMac->scan.scanResultList);
    csrLLClose(&pMac->scan.scanResultList);
    csrLLClose(&pMac->scan.tempScanResults);
    csrPurgeChannelPower(pMac, &pMac->scan.channelPowerInfoList24);
    csrPurgeChannelPower(pMac, &pMac->scan.channelPowerInfoList5G);
    csrLLClose(&pMac->scan.channelPowerInfoList24);
    csrLLClose(&pMac->scan.channelPowerInfoList5G);
    csrScanDisable(pMac);
    palTimerFree(pMac->hHdd, pMac->scan.hTimerResultAging);
    palTimerFree(pMac->hHdd, pMac->scan.hTimerGetResult);
    palTimerFree(pMac->hHdd, pMac->scan.hTimerIdleScan);
#ifdef FEATURE_WLAN_GEN6_ROAMING
    palTimerFree(pMac->hHdd, pMac->scan.hTimerBgScan);
#endif
    return eHAL_STATUS_SUCCESS;
}


eHalStatus csrScanEnable( tpAniSirGlobal pMac )
{
    
    pMac->scan.fScanEnable = eANI_BOOLEAN_TRUE;
    pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
    
    return eHAL_STATUS_SUCCESS;
}


eHalStatus csrScanDisable( tpAniSirGlobal pMac )
{
    
    csrScanStopTimers(pMac);
    pMac->scan.fScanEnable = eANI_BOOLEAN_FALSE;
    
    return eHAL_STATUS_SUCCESS;
}


eHalStatus csrScanRequest(tpAniSirGlobal pMac, tCsrScanRequest *pScanRequest, tANI_U32 *pScanRequestID, 
                            csrScanCompleteCallback callback, void *pContext)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tSmeCmd *pScanCmd = NULL;
    
    do
    {
        if(pMac->scan.fScanEnable)
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(pScanCmd)
            {
                pScanCmd->command = eSmeCommandScan; 
                pScanCmd->u.scanCmd.callback = callback;
                pScanCmd->u.scanCmd.pContext = pContext;
                pScanCmd->u.scanCmd.scanID = *pScanRequestID;
                if(eCSR_SCAN_REQUEST_11D_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScan11d1;
                }
                else if(eCSR_SCAN_REQUEST_FULL_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanUserRequest;
                }
                else if(eCSR_SCAN_HO_BG_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanBgScan;
                }
                else if(eCSR_SCAN_HO_PROBE_SCAN == pScanRequest->requestType)
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanProbeBss;
                }
                else
                {
                    pScanCmd->u.scanCmd.reason = eCsrScanIdleScan;
                }
                if(pScanRequest->minChnTime == 0 && pScanRequest->maxChnTime == 0)
                {
                    //The caller doesn't set the time correctly. Set it here
                    if(pScanRequest->scanType == eSIR_ACTIVE_SCAN)
                    {
                        pScanRequest->maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
                        pScanRequest->minChnTime = pMac->roam.configParam.nActiveMinChnTime;
                    }
                    else
                    {
                        pScanRequest->maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
                        pScanRequest->minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
                    }
                }
                //Need to make the following atomic
				    pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //let it wrap around
                
                if(pScanRequestID)
                {
                    *pScanRequestID = pScanCmd->u.scanCmd.scanID; 
                }

                //Tush : If it is the first scan request from HDD, CSR checks if it is for 11d. 
                // If it is not, CSR will save the scan request in the pending cmd queue 
                // & issue an 11d scan request to PE.
                if((0 == pScanCmd->u.scanCmd.scanID)
                   && (eCSR_SCAN_REQUEST_11D_SCAN != pScanRequest->requestType))
                {
                    tSmeCmd *p11dScanCmd;
                    tCsrScanRequest scanReq;
                    tCsrChannelInfo *pChnInfo = &scanReq.ChannelInfo;

                    palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));

                    p11dScanCmd = csrGetCommandBuffer(pMac);
                    if(p11dScanCmd)
                    {
                        tANI_U32 numChn = pMac->scan.baseChannels.numChannels;

                        status = palAllocateMemory( pMac->hHdd, (void **)&pChnInfo->ChannelList, numChn );
                        if( !HAL_STATUS_SUCCESS( status ) )
                        {
                            break;
                        }
                        status = palCopyMemory( pMac->hHdd, pChnInfo->ChannelList, 
                                    pMac->scan.baseChannels.channelList, numChn );
                        if( !HAL_STATUS_SUCCESS( status ) )
                        {
                            palFreeMemory( pMac->hHdd, pChnInfo->ChannelList );
                            pChnInfo->ChannelList = NULL;
                            break;
                        }
                        pChnInfo->numOfChannels = (tANI_U8)numChn;
                        p11dScanCmd->command = eSmeCommandScan;
                        p11dScanCmd->u.scanCmd.callback = NULL;
                        p11dScanCmd->u.scanCmd.pContext = NULL;
                        p11dScanCmd->u.scanCmd.reason = eCsrScan11d1;
                        p11dScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++;
                
                        scanReq.BSSType = eCSR_BSS_TYPE_ANY;
                        scanReq.scanType = eSIR_PASSIVE_SCAN;
                        scanReq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
                        scanReq.maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
                        scanReq.minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
                        status = csrScanCopyRequest(pMac, &p11dScanCmd->u.scanCmd.u.scanRequest, &scanReq);
                        //Free the channel list
                        palFreeMemory( pMac->hHdd, pChnInfo->ChannelList );

                        if(HAL_STATUS_SUCCESS(status))
                        {
                            //Start process the command
                            status = csrQueueSmeCommand(pMac, p11dScanCmd, eANI_BOOLEAN_FALSE);
                            if( !HAL_STATUS_SUCCESS( status ) )
                            {
                                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                                break;
                            }
                        }
                        else 
                        {
                            break;
                        }
                    }
                    else
                    {
                        //error
                        break;
                    }
                }
                status = csrScanCopyRequest(pMac, &pScanCmd->u.scanCmd.u.scanRequest, pScanRequest);
                if(HAL_STATUS_SUCCESS(status))
                {
                    //Start process the command
                    status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
                    if( !HAL_STATUS_SUCCESS( status ) )
                    {
                        smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                        break;
                    }
                }
                else 
                {
                    smsLog( pMac, LOGE, FL(" fail to copy request status = %d\n"), status );
                    break;
                }
            }
            else 
            {
                //log error
                break;
            }
        }
    } while(0);
	if(!HAL_STATUS_SUCCESS(status) && pScanCmd)
	{
		if( eCsrScanIdleScan == pScanCmd->u.scanCmd.reason )
		{
			//Set the flag back for restarting idle scan
			pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
		}
        csrReleaseCommandScan(pMac, pScanCmd);
	}
    
    return (status);
}


eHalStatus csrScanRequestResult(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pScanCmd;
    
    if(pMac->scan.fScanEnable)
    {
        pScanCmd = csrGetCommandBuffer(pMac);
        if(pScanCmd)
        {
            pScanCmd->command = eSmeCommandScan;
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanGetResult;
            //Need to make the following atomic
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //let it wrap around
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }
        else 
        {
            //log error
            smsLog(pMac, LOGE, FL("can not obtain a common buffer\n"));
            status = eHAL_STATUS_RESOURCES;
        }
    }
    
    return (status);
}


eHalStatus csrScanAllChannels(tpAniSirGlobal pMac, eCsrRequestType reqType)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 scanId;
    tCsrScanRequest scanReq;

    palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
    scanReq.BSSType = eCSR_BSS_TYPE_ANY;
    scanReq.scanType = eSIR_ACTIVE_SCAN;
    scanReq.requestType = reqType;
    scanReq.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
    scanReq.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
    status = csrScanRequest(pMac, &scanReq, &scanId, NULL, NULL);

    return (status);
}



#ifdef FEATURE_WLAN_GEN6_ROAMING
//HO
void csrScanSendBgProbeReq(tpAniSirGlobal pMac, tCsrBssid *pBssid)
{
   tCsrHandoffStaInfo *pStaEntry = NULL;
   int query_status = -1;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_U32 scanId;
   tCsrScanRequest scanReq;
   tANI_U32 sessionId = 0;
   tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

   

   // find the entry in candidate list
   query_status = csrScanFindBssEntryFromList( pMac,
                                               &pMac->roam.handoffInfo.candidateList,
                                               *pBssid,
                                               &pStaEntry);

   if( 0 != query_status )
   {
      //msg
      smsLog(pMac, LOGW, "csrScanSendBgProbeReq: entry not in candidateList\n");
      VOS_ASSERT(0);
      return;
   }

   // make sure stop any on going bg scan
   //clear up pending scan req
   csrScanRemoveBgScanReq(pMac);
   csrScanAbortMacScan(pMac);
   status = csrScanBGScanAbort(pMac);
   if(!HAL_STATUS_SUCCESS(status))
   {
	   smsLog(pMac, LOGW, " csrScanSendBgProbeReq: csrScanBGScanAbort failed\n");
   }
   csrScanStopBgScanTimer(pMac);

   //send down the scan req
   palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
   palCopyMemory(pMac->hHdd, &scanReq.bssid, pBssid, sizeof(tCsrBssid));


   status = palAllocateMemory(pMac->hHdd, 
                              (void **)&scanReq.SSIDs.SSIDList,
                              sizeof( tCsrSSIDInfo ));
   if(!HAL_STATUS_SUCCESS(status))
   {
      //err msg
      smsLog(pMac, LOGW, "csrScanSendBgProbeReq: couldn't allocate memory for the ssid\n");

   }
   else
   {
       scanReq.SSIDs.numOfSSIDs = 1;
    
       palCopyMemory(pMac->hHdd, 
                     &scanReq.SSIDs.SSIDList[0].SSID,
                     &pSession->connectedProfile
                     /*&pMac->roam.connectedProfile.SSID*/, 
                     sizeof( tSirMacSSid ));
   }
   

   status = palAllocateMemory(pMac->hHdd, 
                              (void **)&scanReq.ChannelInfo.ChannelList,
                              1);
   if(!HAL_STATUS_SUCCESS(status))
   {
      //err msg
      smsLog(pMac, LOGW, "csrScanSendBgProbeReq: couldn't allocate memory for the ChannelList\n");

   }
   else
   {
       scanReq.ChannelInfo.numOfChannels = 1;
    
       scanReq.ChannelInfo.ChannelList[0] = pStaEntry->sta.pBssDesc->channelId;
   }


   scanReq.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
   scanReq.scanType = eSIR_ACTIVE_SCAN;
   scanReq.requestType = eCSR_SCAN_HO_PROBE_SCAN;
   scanReq.maxChnTime = pMac->roam.handoffInfo.handoffActivityInfo.maxChnTime;
   scanReq.minChnTime = pMac->roam.handoffInfo.handoffActivityInfo.minChnTime;
   pMac->roam.handoffInfo.isProbeRspPending = TRUE;
   status = csrScanRequest(pMac, &scanReq, &scanId, NULL, NULL);
   if(!HAL_STATUS_SUCCESS(status))
   {
      //err msg
      smsLog(pMac, LOGW, "csrScanSendBgProbeReq: couldn't send down the probe\n");

   }

   //free  probe scan params
   if(scanReq.ChannelInfo.ChannelList)
   {
      palFreeMemory(pMac->hHdd, scanReq.ChannelInfo.ChannelList);
   }
   if(scanReq.SSIDs.SSIDList)
   {
       palFreeMemory(pMac->hHdd, scanReq.SSIDs.SSIDList);
   }
   return;
  
}

eHalStatus csrScanSendBgScanReq(tpAniSirGlobal pMac)
{
   tCsrBGScanRequest bgScanParams;
   if(( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC ))
   {
      smsLog(pMac, LOGW, "csrScanSendBgScanReq: wrong substate %d\n", pMac->roam.handoffInfo.currSubState);
      return eHAL_STATUS_FAILURE;
   }
   
   palZeroMemory(pMac->hHdd, &bgScanParams, sizeof(tCsrBGScanRequest));
   bgScanParams.ChannelInfo = csrScanGetNextBgScanChannelList(pMac);

   palZeroMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList, 
				 pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels);
   pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels = 0;

   palCopyMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList,
				 bgScanParams.ChannelInfo.ChannelList, bgScanParams.ChannelInfo.numOfChannels);
   pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels = 
	   bgScanParams.ChannelInfo.numOfChannels;

   bgScanParams.maxChnTime = pMac->roam.handoffInfo.handoffActivityInfo.maxChnTime;
   bgScanParams.minChnTime = pMac->roam.handoffInfo.handoffActivityInfo.minChnTime;

   switch( pMac->roam.handoffInfo.currSubState )
   {
     case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:
        //better logic??
        bgScanParams.scanInterval = csrScanGetBgScanTimerVal(pMac)/bgScanParams.ChannelInfo.numOfChannels;
        csrScanSendNoTrafficBgScanReq(pMac, &bgScanParams);
        pMac->roam.handoffInfo.isBgScanRspPending = TRUE;
        break;

     case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
     case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
        csrScanSendInTrafficBgScanReq(pMac, &bgScanParams);
        pMac->roam.handoffInfo.isBgScanRspPending = TRUE;
        break;

      default:
         smsLog(pMac, LOGW, "csrScanSendBgScanReq: wrong sub-state %d\n", pMac->roam.handoffInfo.currSubState);
         //VOS_ASSERT(0);
   }
   //free bgscan params
   if(bgScanParams.ChannelInfo.ChannelList)
   {
      palFreeMemory(pMac->hHdd, bgScanParams.ChannelInfo.ChannelList);
   }

   return eHAL_STATUS_SUCCESS;
}

eHalStatus csrScanSendNoTrafficBgScanReq(tpAniSirGlobal pMac, tCsrBGScanRequest *pBgScanParams)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_U32 scanId;
   tCsrScanRequest scanReq;
   palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
   smsLog(pMac, LOGW, "csrScanSendNoTrafficBgScanReq:");
   // check if bg scan is on going, no need to send down the new params if true
   if(pMac->roam.handoffInfo.isBgScanRspPending)
   {
      //msg
      smsLog(pMac, LOGW, "csrScanSendNoTrafficBgScanReq: BgScanRsp is Pending\n");
      return status;
   }

   //disabling PE based bg scanning, issuing fg scans instead
   if(0 == pBgScanParams->ChannelInfo.numOfChannels)
   {
       scanReq.ChannelInfo.ChannelList = NULL;
       scanReq.ChannelInfo.numOfChannels = 0;
   }
   else
   {
       status = palAllocateMemory(pMac->hHdd, (void **)&scanReq.ChannelInfo.ChannelList, 
                                  pBgScanParams->ChannelInfo.numOfChannels);
       if(HAL_STATUS_SUCCESS(status))
       {
           palCopyMemory(pMac->hHdd, scanReq.ChannelInfo.ChannelList, pBgScanParams->ChannelInfo.ChannelList, 
                         pBgScanParams->ChannelInfo.numOfChannels );
           scanReq.ChannelInfo.numOfChannels = pBgScanParams->ChannelInfo.numOfChannels;
       }
       else
       {
           smsLog(pMac, LOGW, "csrScanSendNoTrafficBgScanReq: couldn't allocate memory for the ChannelList\n");
           return eHAL_STATUS_FAILURE;
       }
   }//Allocate memory for Channel List

   scanReq.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
   scanReq.scanType = eSIR_ACTIVE_SCAN;
   scanReq.requestType = eCSR_SCAN_HO_BG_SCAN;
   scanReq.maxChnTime = pBgScanParams->maxChnTime;
   scanReq.minChnTime = pBgScanParams->minChnTime;
   status = csrScanRequest(pMac, &scanReq, &scanId, NULL, NULL);
   if(scanReq.ChannelInfo.ChannelList)
   {
       palFreeMemory(pMac->hHdd, scanReq.ChannelInfo.ChannelList);
   }

   return status;
}

eHalStatus csrScanSendInTrafficBgScanReq(tpAniSirGlobal pMac, tCsrBGScanRequest *pBgScanParams)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_U32 scanId;
   tCsrScanRequest scanReq;

   smsLog(pMac, LOGW, "csrScanSendInTrafficBgScanReq:");
   // check if bg scan is on going, no need to send down the new params if true
   if(pMac->roam.handoffInfo.isBgScanRspPending)
   {
      //msg
      smsLog(pMac, LOGW, "csrScanSendInTrafficBgScanReq: BgScanRsp is Pending\n");
      return status;
   }

   //send down the scan req for 1 channel on the associated SSID
   palZeroMemory(pMac->hHdd, &scanReq, sizeof(tCsrScanRequest));
   status = palAllocateMemory(pMac->hHdd, 
                              (void **)&scanReq.ChannelInfo.ChannelList,
                              1);
   if(!HAL_STATUS_SUCCESS(status))
   {
      //err msg
      smsLog(pMac, LOGW, "csrScanSendInTrafficBgScanReq: couldn't allocate memory for the ChannelList\n");

      return eHAL_STATUS_FAILURE;
   }

   scanReq.ChannelInfo.numOfChannels = 1;

   scanReq.ChannelInfo.ChannelList[0] = pBgScanParams->ChannelInfo.ChannelList[0];

   scanReq.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
   scanReq.scanType = eSIR_ACTIVE_SCAN;
   scanReq.requestType = eCSR_SCAN_HO_BG_SCAN;
   scanReq.maxChnTime = pBgScanParams->maxChnTime;
   scanReq.minChnTime = pBgScanParams->minChnTime;
   status = csrScanRequest(pMac, &scanReq, &scanId, NULL, NULL);
   if(scanReq.ChannelInfo.ChannelList)
   {
       palFreeMemory(pMac->hHdd, scanReq.ChannelInfo.ChannelList);
   }

   return status;
}

tCsrChannelInfo csrScanGetNextBgScanChannelList(tpAniSirGlobal pMac)
{
   /* This function returns the next set of channels to perform bg scan on 
      for current state (NT, NRT and RT)                                     */
   eHalStatus status;
   tCsrChannelInfo next_chan_list;
   tANI_U8    next_chan;
   tANI_U8    chanIndex;
   tANI_U8    size;
   palZeroMemory(pMac->hHdd, &next_chan_list, sizeof(tCsrChannelInfo));
   /*-------------------------------------------------------------------------
     This process is the same for NT, NRT and RT (from FDD), the only difference 
     is, for NT we send down the whole channel list to PE, while in traffic we 
     request scan for one channel at a time:

     Choose each of the channels from the candidate list and 1 extra from
     the other channels

     e.g if 1, 6 and 11 are channels for candidate APs, the scan would proceed
     as:
     1-6-11-2-1-6-11-3-1-6-11-4...1-6-11-10
     1-6-11-2-...

     This is translated into cand_chan_mask and other_chan_mask as follows:

     For any given full scan operation (of all candidates and other chans),
     we set cand_chan_mask to all the cand chans to be scanned

     cand_chan_mask_iter is also set to cand_chan_mask but is updated each
     time by removing channels already scanned.

     other_chan_mask_iter is set to all the remaining channels and is updated
     each time by removing channels already scanned.

     Algorithm

     - If both cand_chan_mask_iter and other_chan_mask_iter are 0, then we 
       start a fresh complete scan operation by gathering all the channels
       from candidates (including current), and remaining available channels

     - If cand_chan_mask_iter is 0, then pick one channel from 
       other_chan_mask_iter and replenish cand_chan_mask_iter from 
       cand_chan_mask.

     - If cand_chan_mask_iter is non-zero, pick one channel from this field

     - Issue the scan request for the chosen channel. 

     Channel can never be 0 since we scan current channel at a minimum!!
   -------------------------------------------------------------------------*/
   if(eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC == pMac->roam.handoffInfo.currSubState)
   {
      /* Generate the candidate & other channel list afresh                  */
      csrScanGetCandChanList(pMac);
      csrScanCreateOtherChanList(pMac);
      csrScanUpdateOtherChanList(pMac);

      size = pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels + 1;
      status = palAllocateMemory(pMac->hHdd, 
                                 (void **)&next_chan_list.ChannelList,
                                 size);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrScanGetNextBgScanChannelList: couldn't allocate memory for the ChannelList\n");
         return next_chan_list;
      }

      palCopyMemory(pMac->hHdd, next_chan_list.ChannelList, 
                    pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList, 
                    pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels);
      next_chan_list.numOfChannels = pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels;

      if(!pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels)
      {
         //reset channelScanHistory
         pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;
         csrScanCreateOtherChanList(pMac);
         csrScanUpdateOtherChanList(pMac);
      }

      next_chan_list.ChannelList[next_chan_list.numOfChannels] = pMac->roam.handoffInfo.handoffActivityInfo.
         otherChanList.ChannelList[0];
      next_chan_list.numOfChannels++;

   }
   else 
   {
      if( ( (pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex > 
           pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels) &&
          (pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex > 
           pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels) ) ||
          ( (pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex == 0) &&
          (pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex == 0) ) )
      {
         /* End of full scan: need to generate info for next full scan          */
         
         /* Generate the candidate & other channel list afresh                  */
         csrScanGetCandChanList(pMac);
         csrScanCreateOtherChanList(pMac);
         csrScanUpdateOtherChanList(pMac);

         if(!pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels)
         {
            //msg
            smsLog(pMac, LOGW, "csrScanGetNextBgScanChannelList: empty candChanList\n");
            VOS_ASSERT(0);
            return next_chan_list;
         }
      
         pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex = 1;
      
         pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex = 1;
         //reset channelScanHistory
         pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;
      
         next_chan = pMac->roam.handoffInfo.handoffActivityInfo.
            candChanList.ChannelList[pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex-1];
         pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex++;
      }
      else if( pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex > 
               pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels )
      {
         /* Same iteration: choose next other_chan as curr and update           */
         next_chan = pMac->roam.handoffInfo.handoffActivityInfo.
            otherChanList.ChannelList[pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex-1];
         pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex++;
         if(pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex <= 
            pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels)
         {
            pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex = 1;
         }
      }
      else
      {
         /* Choose the next cand_chan_mask_iter                                 */
         next_chan = pMac->roam.handoffInfo.handoffActivityInfo.
            candChanList.ChannelList[pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex-1];
         pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex++;
      }
      status = palAllocateMemory(pMac->hHdd, 
                                 (void **)&next_chan_list.ChannelList,
                                 1);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrScanGetNextBgScanChannelList: couldn't allocate memory for the ChannelList\n");
         return next_chan_list;
      }

      next_chan_list.numOfChannels = 1;
      next_chan_list.ChannelList[0] = next_chan;
   }

   smsLog(pMac, LOGW, "csrScanGetNextBgScanChannelList: Setting next bg scan channellist :\n");
   for(chanIndex = 0; chanIndex < next_chan_list.numOfChannels; chanIndex++)
   {
      smsLog(pMac, LOGW, "csrScanGetNextBgScanChannelList: channel = %d :\n", next_chan_list.ChannelList[chanIndex]);
      //update channelScanHistory
      pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory |= 
         1 << (next_chan_list.ChannelList[chanIndex] - 1);
   }

   return next_chan_list;

}

void csrScanGetCandChanList(tpAniSirGlobal pMac)
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry;
   tANI_U8  index = 0;
   tANI_U8  size = 0;
   eHalStatus status;
   tANI_U32 chanMask = 0;
   tANI_U32 tempChanMask = 0;
   tANI_U8 bitIndex = 1;

   pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels = 1;

   pEntry = csrLLPeekHead( &pMac->roam.handoffInfo.candidateList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(&pMac->roam.handoffInfo.candidateList))
   {
      smsLog(pMac, LOGW, "csrScanGetCandChanList: candidateList empty\n");
   }

   if(pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList)
   {
      palFreeMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList);
   }


   chanMask |= 1 << pMac->roam.handoffInfo.currSta.pBssDesc->channelId;

   while( pEntry )
   {
	  pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );


	  chanMask |= 1 << pTempStaEntry->sta.pBssDesc->channelId;

	  pEntry = csrLLNext( &pMac->roam.handoffInfo.candidateList, pEntry, LL_ACCESS_NOLOCK );
   }

   //count the bits in chanMask
   tempChanMask = chanMask;
   while(tempChanMask)
   {
	   tempChanMask = tempChanMask & (tempChanMask-1);
	   size++;
   }

   status = palAllocateMemory(pMac->hHdd, 
                              (void **)&pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList,
                              size);
   if(!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOGW, "csrScanGetCandChanList: couldn't allocate memory for the ChannelList\n");
      return;
   }

   while(chanMask)
   {
	   if(chanMask & (1 << bitIndex))
	   {
           pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList[index] = bitIndex;
		   chanMask &= ~(1 << bitIndex);
		   index++;
	   }
	   bitIndex++;
   }

   pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels = size;

}

eHalStatus csrScanCreateOtherChanList(tpAniSirGlobal pMac)
{
   tANI_U8  index = 0;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tCsrChannelInfo ChannelInfo;
   /* NOTE : Scan is not per session */
   tANI_U32 sessionId = 0; // Use default session ID as reference
   tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

   if(pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList)
   {
      palFreeMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList);
      pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList = NULL;
   }
   
   if(pSession->pCurRoamProfile)
   {
   status = palAllocateMemory(pMac->hHdd, 
                                 (void **)&ChannelInfo.ChannelList,
                                 pSession->pCurRoamProfile->ChannelInfo.numOfChannels);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrScanCreateOtherChanList: couldn't allocate memory for the ChannelList\n");
         return status;
      }

      palCopyMemory(pMac->hHdd, ChannelInfo.ChannelList, 
                    pSession->pCurRoamProfile->ChannelInfo.ChannelList,
                              pSession->pCurRoamProfile->ChannelInfo.numOfChannels);
      ChannelInfo.numOfChannels = pSession->pCurRoamProfile->ChannelInfo.numOfChannels;

   }
   else
   {
      //Get all the valid channels
      tANI_U32 len = WNI_CFG_VALID_CHANNEL_LIST_LEN;

      status = csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len);
      if (HAL_STATUS_SUCCESS(status))
      {
         ChannelInfo.ChannelList = pMac->roam.validChannelList;
         ChannelInfo.numOfChannels = (tANI_U8)len;
      }
      else
      {
         smsLog(pMac, LOGE, FL("  fail to get valid channel list status = %d\n"), status);
         return status;
      }

   }
   status = palAllocateMemory(pMac->hHdd, 
                              (void **)&pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList,
                              ChannelInfo.numOfChannels);

   if(!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOGW, "csrScanCreateOtherChanList: couldn't allocate memory for the ChannelList\n");
      return status;
   }

   pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels = 0;

   for(index = 0; index < ChannelInfo.numOfChannels; index++ )
   {
      csrRoamIsChannelValid(pMac, ChannelInfo.ChannelList[0]);
      if(csrRoamIsValidChannel(pMac, ChannelInfo.ChannelList[index]))
      {
         pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList[index]
            = ChannelInfo.ChannelList[index];
         pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels ++;
      }

   }

   return eHAL_STATUS_SUCCESS;

}

void csrScanUpdateOtherChanList(tpAniSirGlobal pMac)
{
   tANI_U8  indexOther = 0;
   tANI_U8  indexCandidate = 0;

   for(indexCandidate = 0; indexCandidate < pMac->roam.handoffInfo.handoffActivityInfo.candChanList.numOfChannels; indexCandidate++ )
   {
      for(indexOther = 0; indexOther < pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels; )
      {
         
         if(pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList[indexOther] == 
            pMac->roam.handoffInfo.handoffActivityInfo.candChanList.ChannelList[indexCandidate])
         {
            if(indexOther != (pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels - 1))
            {
               palCopyMemory(pMac->hHdd, 
                          pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList + indexOther,
                          pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList + indexOther + 1,
                          pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels - indexOther - 1);
            }
            pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels --;
            break;
         }
         indexOther++;
      }
   }

   if(eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC == pMac->roam.handoffInfo.currSubState)
   {
            
      for(indexOther = 0; indexOther < pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels; )
      {
   
         if(pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory & 
            (1 << (pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList[indexOther] - 1)))
         {
            if(indexOther != (pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels - 1))
            {
               palCopyMemory(pMac->hHdd, 
                             pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList + indexOther,
                             pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.ChannelList + indexOther + 1,
                             pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels - indexOther - 1);
            }
            pMac->roam.handoffInfo.handoffActivityInfo.otherChanList.numOfChannels --;
         }
         else
         {
            indexOther++;
         }
         
      }
   }
}
#endif

eHalStatus csrIssueRoamAfterLostlinkScan(tpAniSirGlobal pMac, tANI_U32 sessionId, eCsrRoamReason reason)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId = 0;
    tCsrRoamProfile *pProfile = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    do
    {
        smsLog(pMac, LOG1, " csrIssueRoamAfterLostlinkScan called\n");
        if(pSession->fCancelRoaming)
        {
            smsLog(pMac, LOGW, " lostlink roaming is cancelled\n");
            csrScanStartIdleScan(pMac);
            status = eHAL_STATUS_SUCCESS;
            break;
        }
        //Here is the profile we need to connect to
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        if(NULL == pSession->pCurRoamProfile)
        {
            pScanFilter->EncryptionType.numEntries = 1;
            pScanFilter->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        }
        else
        {
            //We have to make a copy of pCurRoamProfile because it will be free inside csrRoamIssueConnect
            status = palAllocateMemory(pMac->hHdd, (void **)&pProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            palZeroMemory(pMac->hHdd, pProfile, sizeof(tCsrRoamProfile));
            status = csrRoamCopyProfile(pMac, pProfile, pSession->pCurRoamProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        }//We have a profile
        roamId = GET_NEXT_ROAM_ID(&pMac->roam);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                if(eCsrLostLink1 == reason)
                {
                    //we want to put the last connected BSS to the very beginning, if possible
                    csrMoveBssToHeadFromBSSID(pMac, &pSession->connectedProfile.bssid, hBSSList);
                }
                status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, reason, 
                                                roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    csrScanResultPurge(pMac, hBSSList);
                }
            }//Have scan result
        }
    }while(0);
    if(pScanFilter)
    {
        //we need to free memory for filter if profile exists
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(NULL != pProfile)
    {
        csrReleaseProfile(pMac, pProfile);
        palFreeMemory(pMac->hHdd, (void *)pProfile);
    }

    return (status);
}


eHalStatus csrScanGetScanChnInfo(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pScanCmd;
    
    if(pMac->scan.fScanEnable)
    {
        pScanCmd = csrGetCommandBuffer(pMac);
        if(pScanCmd)
        {
            pScanCmd->command = eSmeCommandScan;
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanGetScanChnInfo;
            //Need to make the following atomic
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //let it wrap around
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }
        else 
        {
            //log error
            smsLog(pMac, LOGE, FL("can not obtain a common buffer\n"));
            status = eHAL_STATUS_RESOURCES;
        }
    }
    
    return (status);
}


eHalStatus csrScanHandleFailedLostlink1(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "  Lostlink scan 1 failed\n");
    if(pSession->fCancelRoaming)
    {
        csrScanStartIdleScan(pMac);
    }
    else if(pSession->pCurRoamProfile)
    {
        //We fail lostlink1 but there may be other BSS in the cached result fit the profile. Give it a try first
        if(pSession->pCurRoamProfile->SSIDs.numOfSSIDs == 0 ||
            pSession->pCurRoamProfile->SSIDs.numOfSSIDs > 1)
        {
            //try lostlink scan2
            status = csrScanRequestLostLink2(pMac, sessionId);
        }
        else if(!pSession->pCurRoamProfile->ChannelInfo.ChannelList || 
                pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
        {
            //go straight to lostlink scan3
            status = csrScanRequestLostLink3(pMac, sessionId);
        }
        else
        {
            //we are done with lostlink
            if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
            {
                csrScanStartIdleScan(pMac);
            }
            status = eHAL_STATUS_SUCCESS;
        }
    }
    else
    {
        status = csrScanRequestLostLink3(pMac, sessionId);
    }

    return (status);    
}



eHalStatus csrScanHandleFailedLostlink2(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "  Lostlink scan 2 failed\n");
    if(pSession->fCancelRoaming)
    {
        csrScanStartIdleScan(pMac);
    }
    else if(!pSession->pCurRoamProfile || !pSession->pCurRoamProfile->ChannelInfo.ChannelList || 
                pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
    {
        //try lostlink scan3
        status = csrScanRequestLostLink3(pMac, sessionId);
    }
    else
    {
        //we are done with lostlink
        if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
        {
            csrScanStartIdleScan(pMac);
        }
    }

    return (status);    
}



eHalStatus csrScanHandleFailedLostlink3(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    smsLog(pMac, LOGW, "  Lostlink scan 3 failed\n");
    if(eANI_BOOLEAN_TRUE == csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
    {
        //we are done with lostlink
        csrScanStartIdleScan(pMac);
    }
    
    return (status);    
}




//Lostlink1 scan is to actively scan the last connected profile's SSID on all matched BSS channels.
//If no roam profile (it should not), it is like lostlinkscan3
eHalStatus csrScanRequestLostLink1( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tCsrScanResultFilter *pScanFilter = NULL;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultInfo *pScanResult = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink1;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        if(pSession->connectedProfile.SSID.length)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList, sizeof(tCsrSSIDInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 1;
            palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID, 
                                &pSession->connectedProfile.SSID, sizeof(tSirMacSSid));
        }
        else
        {
            pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 0;
        }
        if(pSession->pCurRoamProfile)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            //Don't change variable status here because whether we can get result or not, the command goes to PE.
            //The status is also used to indicate whether the command is queued. Not success meaning not queue
            if(HAL_STATUS_SUCCESS((csrScanGetResult(pMac, pScanFilter, &hBSSList))) && hBSSList)
            {
                tANI_U8 i, nChn = 0;
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                            WNI_CFG_VALID_CHANNEL_LIST_LEN);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) &&
                    nChn < WNI_CFG_VALID_CHANNEL_LIST_LEN)
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pScanResult->BssDescriptor.channelId)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pScanResult->BssDescriptor.channelId;
                    }
                }
                //Include the last connected BSS' channel
                if(csrRoamIsChannelValid(pMac, pSession->connectedProfile.operationChannel))
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pSession->connectedProfile.operationChannel)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pSession->connectedProfile.operationChannel;
                    }
                }
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = nChn;
            }
            else
            {
                if(csrRoamIsChannelValid(pMac, pSession->connectedProfile.operationChannel))
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                                1);
                    //just try the last connected channel
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[0] = pSession->connectedProfile.operationChannel;
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 1;
                    }
                    else 
                    {
                        break;
                    }
                }
            }
        }
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );

    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink1 failed with status %d\n", status);
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
        status = csrScanHandleFailedLostlink1( pMac, sessionId );
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(hBSSList)
    {
        csrScanResultPurge(pMac, hBSSList);
    }

    return( status );
}


//Lostlink2 scan is to actively scan the all SSIDs of the last roaming profile's on all matched BSS channels.
//Since MAC doesn't support multiple SSID, we scan all SSIDs and filter them afterwards
eHalStatus csrScanRequestLostLink2( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tCsrScanResultFilter *pScanFilter = NULL;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultInfo *pScanResult = NULL;
    tSmeCmd *pCommand = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink2;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        if(pSession->pCurRoamProfile)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            if(hBSSList)
            {
                tANI_U8 i, nChn = 0;
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                            WNI_CFG_VALID_CHANNEL_LIST_LEN);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) &&
                    nChn < WNI_CFG_VALID_CHANNEL_LIST_LEN)
                {
                    for(i = 0; i < nChn; i++)
                    {
                        if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[i] == 
                                        pScanResult->BssDescriptor.channelId)
                        {
                            break;
                        }
                    }
                    if(i == nChn)
                    {
                        pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[nChn++] = pScanResult->BssDescriptor.channelId;
                    }
                }
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = nChn;
            }
        }
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        //Put to the head in pending queue
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );

    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink2 failed with status %d\n", status);
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
        status = csrScanHandleFailedLostlink2( pMac, sessionId );
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(hBSSList)
    {
        csrScanResultPurge(pMac, hBSSList);
    }

    return( status );
}


//To actively scan all valid channels
eHalStatus csrScanRequestLostLink3( tpAniSirGlobal pMac, tANI_U32 sessionId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    smsLog(pMac, LOGW, FL(" called\n"));
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandScan;
        pCommand->sessionId = (tANI_U8)sessionId;
        pCommand->u.scanCmd.reason = eCsrScanLostLink3;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        pCommand->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pCommand->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
        palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.scanRequest.bssid, bAddr, sizeof(tCsrBssid));
        //Put to the head of pending queue
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while( 0 );
    if(!HAL_STATUS_SUCCESS(status))
    {
        smsLog(pMac, LOGW, " csrScanRequestLostLink3 failed with status %d\n", status);
        if(csrRoamCompleteRoaming(pMac, sessionId, eANI_BOOLEAN_FALSE, eCSR_ROAM_RESULT_FAILURE))
        {
            csrScanStartIdleScan(pMac);
        }
        if(pCommand)
        {
            csrReleaseCommandScan(pMac, pCommand);
        }
    }

    return( status );
}


eHalStatus csrScanHandleSearchForSSID(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
    tCsrScanResultFilter *pScanFilter = NULL;
    tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
    tANI_U32 sessionId = pCommand->sessionId;

    do
    {
        //If there is roam command waiting, ignore this roam because the newer roam command is the one to execute
        if(csrIsRoamCommandWaitingForSession(pMac, sessionId))
        {
            smsLog(pMac, LOGW, FL(" aborts because roam commandwaiting\n"));
            break;
        }
        if(pProfile == NULL)
            break;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        if(!HAL_STATUS_SUCCESS(status))
            break;
        status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
        if(!HAL_STATUS_SUCCESS(status))
            break;
        status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, eCsrHddIssued, 
                                    pCommand->u.scanCmd.roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
        if(!HAL_STATUS_SUCCESS(status))
        {
            break;
        }
    }while(0);
    if(!HAL_STATUS_SUCCESS(status))
    {
        if(CSR_INVALID_SCANRESULT_HANDLE != hBSSList)
        {
            csrScanResultPurge(pMac, hBSSList);
        }
        //We haven't done anything to this profile
        csrRoamCallCallback(pMac, sessionId, NULL, pCommand->u.scanCmd.roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
        //In case we have nothing else to do, restart idle scan
        if(csrIsConnStateDisconnected(pMac, sessionId) && !csrIsRoamCommandWaiting(pMac))
        {
            status = csrScanStartIdleScan(pMac);
        }
    }
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }

    return (status);
}


eHalStatus csrScanHandleSearchForSSIDFailure(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 sessionId = pCommand->sessionId;
    tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

#if defined(WLAN_DEBUG)
    if(pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs == 1)
    {
        char str[36];
        palCopyMemory(pMac->hHdd, str, pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.ssId,
            pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.length);
        str[pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID.length] = 0;
        smsLog(pMac, LOGW, FL(" SSID = %s\n"), str);
    }
#endif
    //Check whether it is for start ibss. No need to do anything if it is a JOIN request
    if(pProfile && CSR_IS_START_IBSS(pProfile))
    {
        status = csrRoamIssueConnect(pMac, sessionId, pProfile, NULL, eCsrHddIssued, 
                                        pCommand->u.scanCmd.roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("failed to issue startIBSS command with status = 0x%08X\n"), status);
            csrRoamCallCallback(pMac, sessionId, NULL, pCommand->u.scanCmd.roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
        }
    }
    else 
    {
        eCsrRoamResult roamResult;

        if(csrIsConnStateDisconnected(pMac, sessionId) && !csrIsRoamCommandWaitingForSession(pMac, sessionId))
        {
            status = csrScanStartIdleScan(pMac);
        }
        if((NULL == pProfile) || !csrIsBssTypeIBSS(pProfile->BSSType))
        {
            tCsrRoamInfo *pRoamInfo = NULL, roamInfo;
            tCsrScanResult *pScanResult;

            roamResult = eCSR_ROAM_RESULT_FAILURE;
            if(pCommand->u.roamCmd.pRoamBssEntry)
            {
                palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                pRoamInfo = &roamInfo;
                pScanResult = GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry, tCsrScanResult, Link);
                roamInfo.pBssDesc = &pScanResult->Result.BssDescriptor;
                roamInfo.statusCode = pSession->joinFailStatusCode.statusCode;
                roamInfo.reasonCode = pSession->joinFailStatusCode.reasonCode;
                csrRoamCallCallback(pMac, sessionId, pRoamInfo, pCommand->u.scanCmd.roamId, eCSR_ROAM_ASSOCIATION_COMPLETION, 
                                            eCSR_ROAM_RESULT_FAILURE);
            }
        }
        else
        {
            roamResult = eCSR_ROAM_RESULT_IBSS_START_FAILED;
        }
        csrRoamCompletion(pMac, sessionId, NULL, pCommand, roamResult, eANI_BOOLEAN_FALSE);
    }

    return (status);
}


//After scan for cap changes, issue a roaming command to either reconnect to the AP or pick another one to connect
eHalStatus csrScanHandleCapChangeScanComplete(tpAniSirGlobal pMac, tANI_U32 sessionId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId = 0;
    tCsrRoamProfile *pProfile = NULL;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    do
    {
        //Here is the profile we need to connect to
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(!HAL_STATUS_SUCCESS(status))
            break;
        palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
        if(NULL == pSession->pCurRoamProfile)
        {
            pScanFilter->EncryptionType.numEntries = 1;
            pScanFilter->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        }
        else
        {
            //We have to make a copy of pCurRoamProfile because it will be free inside csrRoamIssueConnect
            status = palAllocateMemory(pMac->hHdd, (void **)&pProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamCopyProfile(pMac, pProfile, pSession->pCurRoamProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
        }//We have a profile
        roamId = GET_NEXT_ROAM_ID(&pMac->roam);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                //we want to put the last connected BSS to the very beginning, if possible
                csrMoveBssToHeadFromBSSID(pMac, &pSession->connectedProfile.bssid, hBSSList);
                status = csrRoamIssueConnect(pMac, sessionId, pProfile, hBSSList, 
                                            eCsrCapsChange, 0, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_TRUE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    csrScanResultPurge(pMac, hBSSList);
                }
            }//Have scan result
            else
            {
                smsLog(pMac, LOGW, FL("cannot find matching BSS of %02X-%02X-%02X-%02X-%02X-%02X\n"), 
                        pSession->connectedProfile.bssid[0],
                        pSession->connectedProfile.bssid[1],
                        pSession->connectedProfile.bssid[2],
                        pSession->connectedProfile.bssid[3],
                        pSession->connectedProfile.bssid[4],
                        pSession->connectedProfile.bssid[5]);
                //Disconnect
                csrRoamDisconnectInternal(pMac, sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);
            }
        }
    }while(0);
    if(pScanFilter)
    {
        csrFreeScanFilter(pMac, pScanFilter);
        palFreeMemory(pMac->hHdd, pScanFilter);
    }
    if(NULL != pProfile)
    {
        csrReleaseProfile(pMac, pProfile);
        palFreeMemory(pMac->hHdd, pProfile);
    }

    return (status);
}



eHalStatus csrScanResultPurge(tpAniSirGlobal pMac, tScanResultHandle hScanList)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tScanResultList *pScanList = (tScanResultList *)hScanList;
     
    if(pScanList)
    {
        status = csrLLScanPurgeResult(pMac, &pScanList->List);
        csrLLClose(&pScanList->List);
        palFreeMemory(pMac->hHdd, pScanList);
    }
    return (status);
}


static tANI_U32 csrGetBssPreferValue(tpAniSirGlobal pMac, int rssi)
{
    tANI_U32 ret = 0;
    int i = CSR_NUM_RSSI_CAT - 1;

    while(i >= 0)
    {
        if(rssi >= pMac->roam.configParam.RSSICat[i])
        {
            ret = pMac->roam.configParam.BssPreferValue[i];
            break;
        }
        i--;
    };

    return (ret);
}


//Return a CapValue base on the capabilities of a BSS
static tANI_U32 csrGetBssCapValue(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes)
{
    tANI_U32 ret = CSR_BSS_CAP_VALUE_NONE;

    if( pIes )
    {
        //We only care about 11N capability
        if(pIes->HTCaps.present)
        {
            ret += CSR_BSS_CAP_VALUE_HT;
        }
        if(CSR_IS_QOS_BSS(pIes))
        {
            ret += CSR_BSS_CAP_VALUE_WMM;
            //Give advantage to UAPSD
            if(CSR_IS_UAPSD_BSS(pIes))
            {
                ret += CSR_BSS_CAP_VALUE_UAPSD;
            }
        }
    }

    return (ret);
}


//To check whther pBss1 is better than pBss2
static tANI_BOOLEAN csrIsBetterBss(tCsrScanResult *pBss1, tCsrScanResult *pBss2)
{
    tANI_BOOLEAN ret;

    if(CSR_IS_BETTER_PREFER_VALUE(pBss1->preferValue, pBss2->preferValue))
    {
        ret = eANI_BOOLEAN_TRUE;
    }
    else if(CSR_IS_EQUAL_PREFER_VALUE(pBss1->preferValue, pBss2->preferValue))
    {
        if(CSR_IS_BETTER_CAP_VALUE(pBss1->capValue, pBss2->capValue))
        {
            ret = eANI_BOOLEAN_TRUE;
        }
        else
        {
            ret = eANI_BOOLEAN_FALSE;
        }
    }
    else
    {
        ret = eANI_BOOLEAN_FALSE;
    }

    return (ret);
}


//Put the BSS into the scan result list
//pIes can not be NULL
static void csrScanAddResult(tpAniSirGlobal pMac, tCsrScanResult *pResult, tDot11fBeaconIEs *pIes)
{
    pResult->preferValue = csrGetBssPreferValue(pMac, (int)pResult->Result.BssDescriptor.rssi);
    pResult->capValue = csrGetBssCapValue(pMac, &pResult->Result.BssDescriptor, pIes);
    csrLLInsertTail( &pMac->scan.scanResultList, &pResult->Link, LL_ACCESS_LOCK );
}


eHalStatus csrScanGetResult(tpAniSirGlobal pMac, tCsrScanResultFilter *pFilter, tScanResultHandle *phResult)
{
    eHalStatus status;
    tScanResultList *pRetList;
    tCsrScanResult *pResult, *pBssDesc;
    tANI_U32 count = 0;
    tListElem *pEntry;
    tANI_U32 bssLen, allocLen;
    eCsrEncryptionType uc = eCSR_ENCRYPT_TYPE_NONE, mc = eCSR_ENCRYPT_TYPE_NONE;
    eCsrAuthType auth = eCSR_AUTH_TYPE_OPEN_SYSTEM;
    tDot11fBeaconIEs *pIes, *pNewIes;
    tANI_BOOLEAN fMatch;
    
    if(phResult)
    {
        *phResult = CSR_INVALID_SCANRESULT_HANDLE;
    }
    status = palAllocateMemory(pMac->hHdd, (void **)&pRetList, sizeof(tScanResultList));
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pRetList, sizeof(tScanResultList));
        csrLLOpen(pMac->hHdd, &pRetList->List);
        pRetList->pCurEntry = NULL;
        csrLLLock(&pMac->scan.scanResultList);
        
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            pIes = (tDot11fBeaconIEs *)( pBssDesc->Result.pvIes );
            //if pBssDesc->Result.pvIes is NULL, we need to free any memory allocated by csrMatchBSS
            //for any error condition, otherwiase, it will be freed later.
            //reset
            fMatch = eANI_BOOLEAN_FALSE;
            pNewIes = NULL;

            if(pFilter)
            {
                fMatch = csrMatchBSS(pMac, &pBssDesc->Result.BssDescriptor, pFilter, &auth, &uc, &mc, &pIes);
                if( NULL != pIes )
                {
                    //Only save it when matching
                    if(fMatch)
                    {
                        if( !pBssDesc->Result.pvIes )
                        {
                            //csrMatchBSS allocates the memory. Simply pass it and it is freed later
                            pNewIes = pIes;
                        }
                        else
                        {
                            //The pIes is allocated by someone else. make a copy
                            //Only to save parsed IEs if caller provides a filter. Most likely the caller
                            //is using to for association, hence save the parsed IEs
				            status = palAllocateMemory(pMac->hHdd, (void **)&pNewIes, sizeof(tDot11fBeaconIEs));
                            if( HAL_STATUS_SUCCESS( status ) )
            {
                                palCopyMemory( pMac->hHdd, pNewIes, pIes, sizeof( tDot11fBeaconIEs ) );
                            }
                            else
                            {
                                smsLog(pMac, LOGE, FL(" fail to allocate memory for IEs\n"));
                                break;
                            }
                        }
                    }//fMatch
                    else if( !pBssDesc->Result.pvIes )
                    {
                        palFreeMemory(pMac->hHdd, pIes);
                    }
                }
            }
            if(NULL == pFilter || fMatch)
            {
                bssLen = pBssDesc->Result.BssDescriptor.length + sizeof(pBssDesc->Result.BssDescriptor.length);
                allocLen = sizeof( tCsrScanResult ) + bssLen;
                status = palAllocateMemory(pMac->hHdd, (void **)&pResult, allocLen);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGE, FL("  fail to allocate memory for scan result, len=%d\n"), allocLen);
                    if(pNewIes)
                    {
                        palFreeMemory(pMac->hHdd, pNewIes);
                    }
                    break;
                }
                palZeroMemory(pMac->hHdd, pResult, allocLen);
                pResult->capValue = pBssDesc->capValue;
                pResult->preferValue = pBssDesc->preferValue;
                pResult->ucEncryptionType = uc;
                pResult->mcEncryptionType = mc;
                pResult->authType = auth;

				//save the pIes for later use
                        pResult->Result.pvIes = pNewIes;
				//save bss description
                status = palCopyMemory(pMac->hHdd, &pResult->Result.BssDescriptor, &pBssDesc->Result.BssDescriptor, bssLen);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGE, FL("  fail to copy memory for scan result\n"));
                    palFreeMemory(pMac->hHdd, pResult);
                    if(pNewIes)
                    {
                        palFreeMemory(pMac->hHdd, pNewIes);
                    }
                    break;
                }
                csrLLLock(&pRetList->List);
                if(csrLLIsListEmpty(&pRetList->List, LL_ACCESS_NOLOCK))
                {
                    csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_NOLOCK);
                }
                else
                {
                    //To sort the list
                    tListElem *pTmpEntry;
                    tCsrScanResult *pTmpResult;
                    
                    pTmpEntry = csrLLPeekHead(&pRetList->List, LL_ACCESS_NOLOCK);
                    while(pTmpEntry)
                    {
                        pTmpResult = GET_BASE_ADDR( pTmpEntry, tCsrScanResult, Link );
                        if(csrIsBetterBss(pResult, pTmpResult))
                        {
                            csrLLInsertEntry(&pRetList->List, pTmpEntry, &pResult->Link, LL_ACCESS_NOLOCK);
                            //To indicate we are done
                            pResult = NULL;
                            break;
                        }
                        pTmpEntry = csrLLNext(&pRetList->List, pTmpEntry, LL_ACCESS_NOLOCK);
                    }
                    if(pResult != NULL)
                    {
                        //This one is not better than any one
                        csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_NOLOCK);
                    }
                }
                csrLLUnlock(&pRetList->List);
                count++;
            }
            pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK );
        }//while
        
        smsLog(pMac, LOG2, FL("return %d BSS\n"), csrLLCount(&pRetList->List));
        csrLLUnlock(&pMac->scan.scanResultList);
        
        if( !HAL_STATUS_SUCCESS(status) || (phResult == NULL) )
        {
            //Fail or No one wants the result.
            csrScanResultPurge(pMac, (tScanResultHandle)pRetList);
        }
        else
        {
            if(0 == count)
            {
                //We are here meaning the there is no match
                csrLLClose(&pRetList->List);
                palFreeMemory(pMac->hHdd, pRetList);
                status = eHAL_STATUS_E_NULL_VALUE;
            }
            else if(phResult && pRetList)
            {
                *phResult = pRetList;
            }
        }
    }//Allocated pRetList
    
    return (status);
}


eHalStatus csrScanFlushResult(tpAniSirGlobal pMac)
{
    return ( csrLLScanPurgeResult(pMac, &pMac->scan.scanResultList) );
} 


eHalStatus csrScanCopyResultList(tpAniSirGlobal pMac, tScanResultHandle hIn, tScanResultHandle *phResult)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tScanResultList *pRetList, *pInList = (tScanResultList *)hIn;
    tCsrScanResult *pResult, *pScanResult;
    tANI_U32 count = 0;
    tListElem *pEntry;
    tANI_U32 bssLen, allocLen;
    
    if(phResult)
    {
        *phResult = CSR_INVALID_SCANRESULT_HANDLE;
    }
    status = palAllocateMemory(pMac->hHdd, (void **)&pRetList, sizeof(tScanResultList));
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pRetList, sizeof(tScanResultList));
        csrLLOpen(pMac->hHdd, &pRetList->List);
        pRetList->pCurEntry = NULL;
        csrLLLock(&pMac->scan.scanResultList);
        csrLLLock(&pInList->List);
        
        pEntry = csrLLPeekHead( &pInList->List, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            pScanResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            bssLen = pScanResult->Result.BssDescriptor.length + sizeof(pScanResult->Result.BssDescriptor.length);
            allocLen = sizeof( tCsrScanResult ) + bssLen;
            status = palAllocateMemory(pMac->hHdd, (void **)&pResult, allocLen);
            if(!HAL_STATUS_SUCCESS(status))
            {
                csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                count = 0;
                break;
            }
            palZeroMemory(pMac->hHdd, pResult, allocLen);
            status = palCopyMemory(pMac->hHdd, &pResult->Result.BssDescriptor, &pScanResult->Result.BssDescriptor, bssLen);
            if(!HAL_STATUS_SUCCESS(status))
            {
                csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                count = 0;
                break;
            }
            if( pScanResult->Result.pvIes )
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pResult->Result.pvIes, sizeof( tDot11fBeaconIEs ));
                if(!HAL_STATUS_SUCCESS(status))
                {
                    //Free the memory we allocate above first
                    palFreeMemory( pMac->hHdd, pResult );
                    csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                    count = 0;
                    break;
                }
                status = palCopyMemory(pMac->hHdd, pResult->Result.pvIes, 
                                pScanResult->Result.pvIes, sizeof( tDot11fBeaconIEs ));
                if(!HAL_STATUS_SUCCESS(status))
                {
                    //Free the memory we allocate above first
                    palFreeMemory( pMac->hHdd, pResult );
                    csrScanResultPurge(pMac, (tScanResultHandle *)pRetList);
                    count = 0;
                    break;
                }
            }
            csrLLInsertTail(&pRetList->List, &pResult->Link, LL_ACCESS_LOCK);
            count++;
            pEntry = csrLLNext( &pInList->List, pEntry, LL_ACCESS_NOLOCK );
        }//while
        csrLLUnlock(&pInList->List);
        csrLLUnlock(&pMac->scan.scanResultList);
        
        if(HAL_STATUS_SUCCESS(status))
        {
            if(0 == count)
            {
                csrLLClose(&pRetList->List);
                palFreeMemory(pMac->hHdd, pRetList);
                status = eHAL_STATUS_E_NULL_VALUE;
            }
            else if(phResult && pRetList)
            {
                *phResult = pRetList;
            }
        }
    }//Allocated pRetList
    
    return (status);
}


 
eHalStatus csrScanningStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg = (tSirMbMsg *)pMsgBuf;

    if((eWNI_SME_SCAN_RSP == pMsg->type) || (eWNI_SME_GET_SCANNED_CHANNEL_RSP == pMsg->type))
    {
        status = csrScanSmeScanResponse( pMac, pMsgBuf );
    }
    else
    {
        if( csrIsAnySessionInConnectState( pMac ) )
        {
            //In case of we are connected, we need to check whether connect status changes
            //because scan may also run while connected.
            csrRoamCheckForLinkStatusChange( pMac, ( tSirSmeRsp * )pMsgBuf );
        }
        else
        {
            smsLog( pMac, LOGW, "Message [0x%04x] received in state, when expecting Scan Response\n", pMsg->type );
        }
    }

    return (status);
}




//pIes may be NULL
tANI_BOOLEAN csrRemoveDupBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDescr,
                                         tDot11fBeaconIEs *pIes ) 
{
    tListElem *pEntry;

    tCsrScanResult *pBssDesc;
    tANI_BOOLEAN fRC = FALSE;

    // Walk through all the chained BssDescriptions.  If we find a chained BssDescription that
    // matches the BssID of the BssDescription passed in, then these must be duplicate scan
    // results for this Bss.  In that case, remove the 'old' Bss description from the linked list.
    pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

    while( pEntry ) 
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        // we have a duplicate scan results only when BSSID, SSID, Channel and NetworkType
        // matches
        if ( csrIsDuplicateBssDescription( pMac, &pBssDesc->Result.BssDescriptor, 
                                                        pSirBssDescr, pIes ) )
        {
#ifdef FEATURE_WLAN_GEN6_ROAMING
            pSirBssDescr->rssi = csrScanUpdateRssi(pMac, pBssDesc->Result.BssDescriptor.rssi, 
                                                   pSirBssDescr->rssi);
#else
            pSirBssDescr->rssi = (tANI_S8)( (((tANI_S32)pSirBssDescr->rssi * CSR_SCAN_RESULT_RSSI_WEIGHT ) +
                                             ((tANI_S32)pBssDesc->Result.BssDescriptor.rssi * (100 - CSR_SCAN_RESULT_RSSI_WEIGHT) )) / 100 );
#endif
            // Remove the 'old' entry from the list....
            if( csrLLRemoveEntry( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK ) )
            {
            // !we need to free the memory associated with this node
                //If failed to remove, assuming someone else got it.
                csrFreeScanResultEntry( pMac, pBssDesc );
            }
            else
            {
                smsLog( pMac, LOGW, FL( "  fail to remove entry\n" ) );
            }
            fRC = TRUE;

            // If we found a match, we can stop looking through the list.
            break;
        }

        pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
    }

    return fRC;
}


eHalStatus csrAddPMKIDCandidateList( tpAniSirGlobal pMac, tANI_U32 sessionId,
                                     tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "csrAddPMKIDCandidateList called pMac->scan.NumPmkidCandidate = %d\n", pSession->NumPmkidCandidate);
    if( pIes )
    {
        // check if this is a RSN BSS
        if( pIes->RSN.present )
        {
            // Check if the BSS is capable of doing pre-authentication
            if( pSession->NumPmkidCandidate < CSR_MAX_PMKID_ALLOWED )
            {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
                {
                    WLAN_VOS_DIAG_EVENT_DEF(secEvent, vos_event_wlan_security_payload_type);
                    palZeroMemory(pMac->hHdd, &secEvent, sizeof(vos_event_wlan_security_payload_type));
                    secEvent.eventId = WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND;
                    secEvent.encryptionModeMulticast = 
                        (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
                    secEvent.encryptionModeUnicast = 
                        (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
                    palCopyMemory( pMac->hHdd, secEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
                    secEvent.authMode = 
                        (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
                    WLAN_VOS_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
                }
#endif//#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

                // if yes, then add to PMKIDCandidateList
                status = palCopyMemory(pMac->hHdd, pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].BSSID, 
                                            pBssDesc->bssId, WNI_CFG_BSSID_LEN);
            
                if( HAL_STATUS_SUCCESS( status ) )
                {
                if ( pIes->RSN.preauth )
                {
                    pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].preAuthSupported = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate].preAuthSupported = eANI_BOOLEAN_FALSE;
                }
                pSession->NumPmkidCandidate++;
            }
            }
            else
            {
                status = eHAL_STATUS_FAILURE;
            }
        }
    }

    return (status);
}

//This function checks whether new AP is found for the current connected profile
//If it is found, it return the sessionId, else it return invalid sessionID
tANI_U32 csrProcessBSSDescForPMKIDList(tpAniSirGlobal pMac, 
                                           tSirBssDescription *pBssDesc,
                                           tDot11fBeaconIEs *pIes)
{
    tANI_U32 i, bRet = CSR_SESSION_ID_INVALID;
    tCsrRoamSession *pSession;
    tDot11fBeaconIEs *pIesLocal = pIes;

    if( pIesLocal || HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal)) )
    {
    for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
    {
        if( CSR_IS_SESSION_VALID( pMac, i ) )
        {
            pSession = CSR_GET_SESSION( pMac, i );
            if( csrIsConnStateConnectedInfra( pMac, i ) && 
                ( eCSR_AUTH_TYPE_RSN == pSession->connectedProfile.AuthType ) )
    {
                    if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile, pBssDesc, pIesLocal))
        {
            //this new BSS fits the current profile connected
                        if(HAL_STATUS_SUCCESS(csrAddPMKIDCandidateList(pMac, i, pBssDesc, pIesLocal)))
            {
                        bRet = i;
                    }
                    break;
                }
            }
        }
    }
        if( !pIes )
        {
            palFreeMemory(pMac->hHdd, pIesLocal);
        }
    }

    return (tANI_U8)bRet;
}

#ifdef FEATURE_WLAN_WAPI
eHalStatus csrAddBKIDCandidateList( tpAniSirGlobal pMac, tANI_U32 sessionId,
                                    tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "csrAddBKIDCandidateList called pMac->scan.NumBkidCandidate = %d\n", pSession->NumBkidCandidate);
    if( pIes )
    {
        // check if this is a WAPI BSS
        if( pIes->WAPI.present )
        {
            // Check if the BSS is capable of doing pre-authentication
            if( pSession->NumBkidCandidate < CSR_MAX_BKID_ALLOWED )
            {

                // if yes, then add to BKIDCandidateList
                status = palCopyMemory(pMac->hHdd, pSession->BkidCandidateInfo[pSession->NumBkidCandidate].BSSID, 
                                            pBssDesc->bssId, WNI_CFG_BSSID_LEN);
            
                if( HAL_STATUS_SUCCESS( status ) )
                {
                    if ( pIes->WAPI.preauth )
                    {
                        pSession->BkidCandidateInfo[pSession->NumBkidCandidate].preAuthSupported = eANI_BOOLEAN_TRUE;
                    }
                    else
                    {
                        pSession->BkidCandidateInfo[pSession->NumBkidCandidate].preAuthSupported = eANI_BOOLEAN_FALSE;
                    }
                    pSession->NumBkidCandidate++;
                }
            }
            else
            {
                status = eHAL_STATUS_FAILURE;
            }
        }
    }

    return (status);
}

//This function checks whether new AP is found for the current connected profile
//if so add to BKIDCandidateList
tANI_BOOLEAN csrProcessBSSDescForBKIDList(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc,
                                          tDot11fBeaconIEs *pIes)
{
    tANI_BOOLEAN fRC = FALSE;
    tDot11fBeaconIEs *pIesLocal = pIes;
    tANI_U32 sessionId, bRet = CSR_SESSION_ID_INVALID;
    tCsrRoamSession *pSession;

    if( pIesLocal || HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal)) )
    {
    for( sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++ )
    {
        if( CSR_IS_SESSION_VALID( pMac, sessionId) )
        {
            pSession = CSR_GET_SESSION( pMac, sessionId );
            if( csrIsConnStateConnectedInfra( pMac, sessionId ) && 
                    eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE == pSession->connectedProfile.AuthType)
    {
                    if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile,pBssDesc, pIesLocal))
        {
            //this new BSS fits the current profile connected
                        if(HAL_STATUS_SUCCESS(csrAddBKIDCandidateList(pMac, sessionId, pBssDesc, pIesLocal)))
            {
                fRC = TRUE;
            }
        }
    }
}
                    }
        if(!pIes)
                    {
            palFreeMemory(pMac->hHdd, pIesLocal);
                    }

                }
    return fRC;
            }

#endif


static void csrMoveTempScanResultsToMainList( tpAniSirGlobal pMac )
{
    tListElem *pEntry;
    tCsrScanResult *pBssDescription;
    tANI_S8              cand_Bss_rssi;
    tANI_BOOLEAN fNewBSSForCurConnection = eANI_BOOLEAN_FALSE, fDupBss;
#ifdef FEATURE_WLAN_WAPI
    tANI_BOOLEAN fNewWapiBSSForCurConnection = eANI_BOOLEAN_FALSE;
#endif /* FEATURE_WLAN_WAPI */
    tDot11fBeaconIEs *pIesLocal = NULL;
    tANI_U32 sessionId = CSR_SESSION_ID_INVALID;

    cand_Bss_rssi = -128; // RSSI coming from PE is -ve

    // remove the BSS descriptions from temporary list
    while( ( pEntry = csrLLRemoveTail( &pMac->scan.tempScanResults, LL_ACCESS_LOCK ) ) != NULL)
    {
        pBssDescription = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        smsLog( pMac, LOGW, "...Bssid= %02x-%02x-%02x-%02x-%02x-%02x chan= %d, rssi = -%d\n",
                      pBssDescription->Result.BssDescriptor.bssId[ 0 ], pBssDescription->Result.BssDescriptor.bssId[ 1 ],
                      pBssDescription->Result.BssDescriptor.bssId[ 2 ], pBssDescription->Result.BssDescriptor.bssId[ 3 ],
                      pBssDescription->Result.BssDescriptor.bssId[ 4 ], pBssDescription->Result.BssDescriptor.bssId[ 5 ],
                      pBssDescription->Result.BssDescriptor.channelId,
                pBssDescription->Result.BssDescriptor.rssi * (-1) );

        //At this time, pBssDescription->Result.pvIes may be NULL
		pIesLocal = (tDot11fBeaconIEs *)( pBssDescription->Result.pvIes );
        if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, &pBssDescription->Result.BssDescriptor, &pIesLocal))) )
        {
            smsLog(pMac, LOGE, FL("  Cannot pared IEs\n"));
            csrFreeScanResultEntry(pMac, pBssDescription);
            continue;
        }
        fDupBss = csrRemoveDupBssDescription( pMac, &pBssDescription->Result.BssDescriptor, pIesLocal );
        //Check whether we have reach out limit
        if( CSR_SCAN_IS_OVER_BSS_LIMIT(pMac) )
        {
            //Limit reach
            smsLog(pMac, LOGW, FL("  BSS limit reached\n"));
            //Free the resources
            csrFreeScanResultEntry(pMac, pBssDescription);
            if( (pBssDescription->Result.pvIes == NULL) && pIesLocal )
            {
                palFreeMemory(pMac->hHdd, pIesLocal);
            }
			//Continue because there may be duplicated BSS
            continue;
        }
        // check for duplicate scan results
        if ( !fDupBss )
        {
            //Found a new BSS
            sessionId = csrProcessBSSDescForPMKIDList(pMac, &pBssDescription->Result.BssDescriptor, pIesLocal);
            if( CSR_SESSION_ID_INVALID != sessionId)
            {
                fNewBSSForCurConnection = eANI_BOOLEAN_TRUE;
            }
        }

        //Tush: find a good AP for 11d info
        if( csrIs11dSupported( pMac ) )
        {
            if(cand_Bss_rssi < pBssDescription->Result.BssDescriptor.rssi)
            {
                // check if country information element is present
                if(pIesLocal->Country.present)
                {
                    cand_Bss_rssi = pBssDescription->Result.BssDescriptor.rssi;
                    // learn country information
                    csrLearnCountryInformation( pMac, &pBssDescription->Result.BssDescriptor, pIesLocal );
                }

            }
        }

        // append to main list
        csrScanAddResult(pMac, pBssDescription, pIesLocal);
        if( (pBssDescription->Result.pvIes == NULL) && pIesLocal )
        {
            palFreeMemory(pMac->hHdd, pIesLocal);
        }
    }

    //Tush: If we can find the current 11d info in any of the scan results, or
    // a good enough AP with the 11d info from the scan results then no need to
    // get into ambiguous state
    if(pMac->scan.fAmbiguous11dInfoFound) 
    {
      if((pMac->scan.fCurrent11dInfoMatch) || (cand_Bss_rssi != -128))
      {
        pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
      }
    }

    if(fNewBSSForCurConnection)
    {
        //remember it first
        csrRoamCallCallback(pMac, sessionId, NULL, 0, eCSR_ROAM_SCAN_FOUND_NEW_BSS, eCSR_ROAM_RESULT_NONE);
    }
#ifdef FEATURE_WLAN_WAPI
    if(fNewWapiBSSForCurConnection)
    {
        //remember it first
        csrRoamCallCallback(pMac, sessionId, NULL, 0, eCSR_ROAM_SCAN_FOUND_NEW_BSS, eCSR_ROAM_RESULT_NEW_WAPI_BSS);
    }
#endif /* FEATURE_WLAN_WAPI */

    return;
}


static tCsrScanResult *csrScanSaveBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pBSSDescription,
                                                  tDot11fBeaconIEs *pIes)
{
    tCsrScanResult *pCsrBssDescription = NULL;
    tANI_U32 cbBSSDesc;
    tANI_U32 cbAllocated;
    eHalStatus halStatus;

    // figure out how big the BSS description is (the BSSDesc->length does NOT
    // include the size of the length field itself).
    cbBSSDesc = pBSSDescription->length + sizeof( pBSSDescription->length );

    cbAllocated = sizeof( tCsrScanResult ) + cbBSSDesc;

    halStatus = palAllocateMemory( pMac->hHdd, (void **)&pCsrBssDescription, cbAllocated );
    if ( HAL_STATUS_SUCCESS(halStatus) )
    {
        palZeroMemory( pMac->hHdd, pCsrBssDescription, cbAllocated );
        pCsrBssDescription->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        palCopyMemory(pMac->hHdd, &pCsrBssDescription->Result.BssDescriptor, pBSSDescription, cbBSSDesc );
#if defined(VOSS_ENSBALED)
        VOS_ASSERT( pCsrBssDescription->Result.pvIes == NULL );
#endif
        csrScanAddResult(pMac, pCsrBssDescription, pIes);
    }

    return( pCsrBssDescription );
}

// Append a Bss Description...
tCsrScanResult *csrScanAppendBssDescription( tpAniSirGlobal pMac, 
                                             tSirBssDescription *pSirBssDescription, 
                                             tDot11fBeaconIEs *pIes )
{
    tCsrScanResult *pCsrBssDescription = NULL;

    csrRemoveDupBssDescription( pMac, pSirBssDescription, pIes );
    pCsrBssDescription = csrScanSaveBssDescription( pMac, pSirBssDescription, pIes );

    return( pCsrBssDescription );
}



void csrPurgeChannelPower( tpAniSirGlobal pMac, tDblLinkList *pChannelList )
{
    tCsrChannelPowerInfo *pChannelSet;
    tListElem *pEntry;

    // Remove the channel sets from the learned list and put them in the free list
    while( ( pEntry = csrLLRemoveHead( pChannelList, LL_ACCESS_LOCK ) ) != NULL)
    {
        pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
        if( pChannelSet )
        {
            palFreeMemory( pMac->hHdd, pChannelSet );
        }
    }

    return;
}


/*
 * Save the channelList into the ultimate storage as the final stage of channel 
 * Input: pCountryInfo -- the country code (e.g. "USI"), channel list, and power limit are all stored inside this data structure
 */
void csrSaveToChannelPower2G_5G( tpAniSirGlobal pMac, tANI_U32 tableSize, tSirMacChanInfo *channelTable )
{
    tANI_U32 i = tableSize / sizeof( tSirMacChanInfo );
    tSirMacChanInfo *pChannelInfo;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_BOOLEAN f2GHzInfoFound = FALSE;
    tANI_BOOLEAN f2GListPurged = FALSE, f5GListPurged = FALSE;
    eHalStatus halStatus;

    pChannelInfo = channelTable;
    // atleast 3 bytes have to be remaining  -- from "countryString"
    while ( i-- )
    {
        halStatus = palAllocateMemory( pMac->hHdd, (void **)&pChannelSet, sizeof(tCsrChannelPowerInfo) );
        if ( eHAL_STATUS_SUCCESS == halStatus )
        {
            palZeroMemory(pMac->hHdd, pChannelSet, sizeof(tCsrChannelPowerInfo));
            pChannelSet->firstChannel = pChannelInfo->firstChanNum;
            pChannelSet->numChannels = pChannelInfo->numChannels;

            // Now set the inter-channel offset based on the frequency band the channel set lies in
            if( CSR_IS_CHANNEL_24GHZ(pChannelSet->firstChannel) )
            {
                pChannelSet->interChannelOffset = 1;
                f2GHzInfoFound = TRUE;
            }
            else
            {
                pChannelSet->interChannelOffset = 4;
                f2GHzInfoFound = FALSE;
            }
            pChannelSet->txPower = CSR_ROAM_MIN( pChannelInfo->maxTxPower, pMac->roam.configParam.nTxPowerCap );

            if( f2GHzInfoFound )
            {
                if( !f2GListPurged )
                {
                    // purge previous results if found new
                    csrPurgeChannelPower( pMac, &pMac->scan.channelPowerInfoList24 );
                    f2GListPurged = TRUE;
                }

                if(CSR_IS_OPERATING_BG_BAND(pMac))
                {
                    // add to the list of 2.4 GHz channel sets
                    csrLLInsertTail( &pMac->scan.channelPowerInfoList24, &pChannelSet->link, LL_ACCESS_LOCK );
                }
                else {
                    smsLog( pMac, LOGW, FL("Adding 11B/G channels in 11A mode -- First Channel is %d"), 
                                pChannelSet->firstChannel);
                    palFreeMemory(pMac->hHdd, pChannelSet);
                }
            }
            else
            {
                // 5GHz info found
                if( !f5GListPurged )
                {
                    // purge previous results if found new
                    csrPurgeChannelPower( pMac, &pMac->scan.channelPowerInfoList5G );
                    f5GListPurged = TRUE;
                }

                if(CSR_IS_OPERATING_A_BAND(pMac))
                {
                    // add to the list of 5GHz channel sets
                    csrLLInsertTail( &pMac->scan.channelPowerInfoList5G, &pChannelSet->link, LL_ACCESS_LOCK );
                }
                else {
                    smsLog( pMac, LOGW, FL("Adding 11A channels in B/G mode -- First Channel is %d"), 
                                pChannelSet->firstChannel);
                    palFreeMemory(pMac->hHdd, pChannelSet);
                }
            }
        }

        pChannelInfo++;                // move to next entry
    }

    return;
}



void csrApplyPower2Current( tpAniSirGlobal pMac )
{
    smsLog( pMac, LOG3, FL(" Updating Cfg with power settings\n"));
    csrSaveTxPowerToCfg( pMac, &pMac->scan.channelPowerInfoList24, WNI_CFG_MAX_TX_POWER_2_4 );
    csrSaveTxPowerToCfg( pMac, &pMac->scan.channelPowerInfoList5G, WNI_CFG_MAX_TX_POWER_5 );
}


void csrApplyChannelPowerCountryInfo( tpAniSirGlobal pMac, tCsrChannel *pChannelList, tANI_U8 *countryCode)
{
	if( pChannelList->numChannels )
	{
		csrSetCfgValidChannelList(pMac, pChannelList->channelList, pChannelList->numChannels);
		// extend scan capability
		csrSetCfgScanControlList(pMac, countryCode, pChannelList);     //  build a scan list based on the channel list : channel# + active/passive scan
	}
	else
	{
		smsLog( pMac, LOGE, FL("  11D channel list is empty\n"));
	}
    csrApplyPower2Current( pMac );     // Store the channel+power info in the global place: Cfg 
    csrSetCfgCountryCode(pMac, countryCode);
}


void csrResetCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce )
{
    if( fForce || (csrIs11dSupported( pMac ) && (!pMac->scan.f11dInfoReset)))
    {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
    {
        vos_log_802_11d_pkt_type *p11dLog;
        int Index;

        WLAN_VOS_DIAG_LOG_ALLOC(p11dLog, vos_log_802_11d_pkt_type, LOG_WLAN_80211D_C);
        if(p11dLog)
        {
            p11dLog->eventId = WLAN_80211D_EVENT_RESET;
            palCopyMemory(pMac->hHdd, p11dLog->countryCode, pMac->scan.countryCodeCurrent, 3);
            p11dLog->numChannel = pMac->scan.base20MHzChannels.numChannels;
            if(p11dLog->numChannel <= VOS_LOG_MAX_NUM_CHANNEL)
            {
                palCopyMemory(pMac->hHdd, p11dLog->Channels, pMac->scan.base20MHzChannels.channelList,
                                p11dLog->numChannel);
                for (Index=0; Index < pMac->scan.base20MHzChannels.numChannels; Index++)
	            {
                    p11dLog->TxPwr[Index] = CSR_ROAM_MIN( pMac->scan.defaultPowerTable[Index].pwr, pMac->roam.configParam.nTxPowerCap );
                }
            }
            if(!pMac->roam.configParam.Is11dSupportEnabled)
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
            }
            else if(pMac->roam.configParam.fEnforceDefaultDomain)
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN;
            }
            else
            {
                p11dLog->supportMultipleDomain = WLAN_80211D_SUPPORT_MULTI_DOMAIN;
            }
            WLAN_VOS_DIAG_LOG_REPORT(p11dLog);
        }
    }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

        // switch to passive scans only when 11d is enabled
        if( csrIs11dSupported( pMac ) )
        {
            pMac->scan.curScanType = eSIR_PASSIVE_SCAN;
        }
        csrSaveChannelPowerForBand(pMac, eANI_BOOLEAN_FALSE);
        csrSaveChannelPowerForBand(pMac, eANI_BOOLEAN_TRUE);
        // ... and apply the channel list, power settings, and the country code.
        csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.base20MHzChannels, pMac->scan.countryCodeCurrent );
        // clear the 11d channel list
        palZeroMemory( pMac->hHdd, &pMac->scan.channels11d, sizeof(pMac->scan.channels11d) );
        pMac->scan.f11dInfoReset = eANI_BOOLEAN_TRUE;
        pMac->scan.f11dInfoApplied = eANI_BOOLEAN_FALSE;
    }

    return;
}


eHalStatus csrResetCountryCodeInformation(tpAniSirGlobal pMac, tANI_BOOLEAN *pfRestartNeeded)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fRestart = eANI_BOOLEAN_FALSE;

    //Use the Country code and domain from EEPROM
    palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, pMac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
    csrSetRegulatoryDomain(pMac, pMac->scan.domainIdCurrent, &fRestart);
    if(eANI_BOOLEAN_FALSE == fRestart || (pfRestartNeeded == NULL))
    {
        //Only reset the country info if we don't need to restart
        csrResetCountryInformation(pMac, eANI_BOOLEAN_TRUE);
    }
    if(pfRestartNeeded)
    {
        *pfRestartNeeded = fRestart;
    }

    return (status);
}


eHalStatus csrSetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pCountry, tANI_BOOLEAN *pfRestartNeeded)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    v_REGDOMAIN_t domainId;

    if(pCountry)
    {
        status = csrGetRegulatoryDomainForCountry(pMac, pCountry, &domainId);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrSetRegulatoryDomain(pMac, domainId, pfRestartNeeded);
            if(HAL_STATUS_SUCCESS(status))
            {
                //We don't need to check the pMac->roam.configParam.fEnforceDefaultDomain flag here,
                //csrSetRegulatoryDomain will fail if the country doesn't fit our domain criteria.
                palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, pCountry, WNI_CFG_COUNTRY_CODE_LEN);
                if((pfRestartNeeded == NULL) || !(*pfRestartNeeded))
                {
                    //Simply set it to cfg. If we need to restart, restart will apply it to the CFG
                    csrSetCfgCountryCode(pMac, pCountry);
                }
            }
        }
    }

    return (status);
}



//caller allocated memory for pNumChn and pChnPowerInfo
//As input, *pNumChn has the size of the array of pChnPowerInfo
//Upon return, *pNumChn has the number of channels assigned.
void csrGetChannelPowerInfo( tpAniSirGlobal pMac, tDblLinkList *pList,
                             tANI_U32 *pNumChn, tChannelListWithPower *pChnPowerInfo)
{
    tListElem *pEntry;
    tANI_U32 chnIdx = 0, idx;
	tCsrChannelPowerInfo *pChannelSet;

    //Get 2.4Ghz first
    pEntry = csrLLPeekHead( pList, LL_ACCESS_LOCK );
    while( pEntry && (chnIdx < *pNumChn) )
    {
        pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
        if ( 1 != pChannelSet->interChannelOffset )
        {
            for( idx = 0; (idx < pChannelSet->numChannels) && (chnIdx < *pNumChn); idx++ )
            {
                pChnPowerInfo[chnIdx].chanId = (tANI_U8)(pChannelSet->firstChannel + ( idx * pChannelSet->interChannelOffset ));
                pChnPowerInfo[chnIdx++].pwr = pChannelSet->txPower;
            }
        }
        else
        {
            for( idx = 0; (idx < pChannelSet->numChannels) && (chnIdx < *pNumChn); idx++ )
            {
                pChnPowerInfo[chnIdx].chanId = (tANI_U8)(pChannelSet->firstChannel + idx);
                pChnPowerInfo[chnIdx++].pwr = pChannelSet->txPower;
            }
        }

        pEntry = csrLLNext( pList, pEntry, LL_ACCESS_LOCK );
    }
    *pNumChn = chnIdx;

    return ;
}



void csrApplyCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce )
{
    v_REGDOMAIN_t domainId;

    do
    {
        if( !csrIs11dSupported( pMac ) || 0 == pMac->scan.channelOf11dInfo) break;
        if( pMac->scan.fAmbiguous11dInfoFound )
        {
            // ambiguous info found
            //Restore te default domain as well
            if(HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pMac->scan.countryCodeCurrent, &domainId )))
            {
                pMac->scan.domainIdCurrent = domainId;
            }
            else
            {
                smsLog(pMac, LOGE, FL(" failed to get domain from currentCountryCode %02X%02X\n"), 
                    pMac->scan.countryCodeCurrent[0], pMac->scan.countryCodeCurrent[1]);
            }
            csrResetCountryInformation( pMac, eANI_BOOLEAN_FALSE );
            break;
        }
        if ( pMac->scan.f11dInfoApplied && !fForce ) break;
        if(HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pMac->scan.countryCode11d, &domainId )))
        {
            //Check whether we need to enforce default domain
            if( ( !pMac->roam.configParam.fEnforceDefaultDomain ) ||
                (pMac->scan.domainIdCurrent == domainId) )
            {

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
                {
                    vos_log_802_11d_pkt_type *p11dLog;
                    tChannelListWithPower chnPwrInfo[WNI_CFG_VALID_CHANNEL_LIST_LEN];
                    tANI_U32 nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN, nTmp;

                    WLAN_VOS_DIAG_LOG_ALLOC(p11dLog, vos_log_802_11d_pkt_type, LOG_WLAN_80211D_C);
                    if(p11dLog)
                    {
                        p11dLog->eventId = WLAN_80211D_EVENT_COUNTRY_SET;
                        palCopyMemory(pMac->hHdd, p11dLog->countryCode, pMac->scan.countryCode11d, 3);
                        p11dLog->numChannel = pMac->scan.channels11d.numChannels;
                        if(p11dLog->numChannel <= VOS_LOG_MAX_NUM_CHANNEL)
                        {
                            palCopyMemory(pMac->hHdd, p11dLog->Channels, pMac->scan.channels11d.channelList,
                                            p11dLog->numChannel);
                            csrGetChannelPowerInfo(pMac, &pMac->scan.channelPowerInfoList24,
                                                    &nChnInfo, chnPwrInfo);
                            nTmp = nChnInfo;
                            nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN - nTmp;
                            csrGetChannelPowerInfo(pMac, &pMac->scan.channelPowerInfoList5G,
                                                    &nChnInfo, &chnPwrInfo[nTmp]);
                            for(nTmp = 0; nTmp < p11dLog->numChannel; nTmp++)
                            {
                                for(nChnInfo = 0; nChnInfo < WNI_CFG_VALID_CHANNEL_LIST_LEN; nChnInfo++)
                                {
                                    if(p11dLog->Channels[nTmp] == chnPwrInfo[nChnInfo].chanId)
                                    {
                                        p11dLog->TxPwr[nTmp] = chnPwrInfo[nChnInfo].pwr;
                                        break;
                                    }
                                }
                            }
                        }
                        if(!pMac->roam.configParam.Is11dSupportEnabled)
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
                        }
                        else if(pMac->roam.configParam.fEnforceDefaultDomain)
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN;
                        }
                        else
                        {
                            p11dLog->supportMultipleDomain = WLAN_80211D_SUPPORT_MULTI_DOMAIN;
                        }
                        WLAN_VOS_DIAG_LOG_REPORT(p11dLog);
                    }
                }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

        pMac->scan.domainIdCurrent = domainId;
        csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.channels11d, pMac->scan.countryCode11d );
        // switch to active scans using this new channel list
        pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
        pMac->scan.f11dInfoApplied = eANI_BOOLEAN_TRUE;
        pMac->scan.f11dInfoReset = eANI_BOOLEAN_FALSE;
            }
        }

    } while( 0 );

    return;
}



tANI_BOOLEAN csrSave11dCountryString( tpAniSirGlobal pMac, tANI_U8 *pCountryCode )
{
    tANI_BOOLEAN fCountryStringChanged = FALSE, fUnknownCountryCode = FALSE;
    tANI_U32 i;

    // convert to UPPER here so we are assured the strings are always in upper case.
    for( i = 0; i < 3; i++ )
    {
        pCountryCode[ i ] = (tANI_U8)csrToUpper( pCountryCode[ i ] );
    }

    // Some of the 'old' Cisco 350 series AP's advertise NA as the country code (for North America ??).
    // NA is not a valid country code or domain so let's allow this by changing it to the proper
    // country code (which is US).  We've also seen some NETGEAR AP's that have "XX " as the country code
    // with valid 2.4 GHz US channel information.  If we cannot find the country code advertised in the
    // 11d information element, let's default to US.
    if ( !HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, pCountryCode, NULL ) ) )
    {
        // Check the enforcement first
        if( pMac->roam.configParam.fEnforceDefaultDomain || pMac->roam.configParam.fEnforceCountryCodeMatch )
        {
            fUnknownCountryCode = TRUE;
        }
        else
        {
        pCountryCode[ 0 ] = 'U';
        pCountryCode[ 1 ] = 'S';
    }
    }

    // We've seen some of the AP's improperly put a 0 for the third character of the country code.
    // spec says valid charcters are 'O' (for outdoor), 'I' for Indoor, or ' ' (space; for either).
    // if we see a 0 in this third character, let's change it to a ' '.
    if ( 0 == pCountryCode[ 2 ] )
    {
        pCountryCode[ 2 ] = ' ';
    }

    if( !fUnknownCountryCode )
    {
        if( 0 == pMac->scan.countryCode11d[ 0 ] && 0 == pMac->scan.countryCode11d[ 1 ] )
        {
        // this is the first .11d information
        palCopyMemory( pMac->hHdd, pMac->scan.countryCode11d, pCountryCode, sizeof( pMac->scan.countryCode11d ) );
    }
    else
    {
        // check that country string has not changed, which it should not
        // compare only the first two bytes as third byte specifies 'I' - Indoor or
        // 'O' - Outdoor or ' ' - for ANY
        if( !palEqualMemory( pMac->hHdd, pMac->scan.countryCode11d, pCountryCode, 2 ) )
        {
            fCountryStringChanged = TRUE;
        }
    }
    }

    return( fCountryStringChanged );
}


void csrSaveChannelPowerForBand( tpAniSirGlobal pMac, tANI_BOOLEAN fPopulate5GBand )
{
    tANI_U32 Index, count=0;
    tSirMacChanInfo *pChanInfo;
    tSirMacChanInfo *pChanInfoStart;

    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN);
        pChanInfoStart = pChanInfo;
	    for (Index=0; Index < pMac->scan.base20MHzChannels.numChannels; Index++)
	    {
	 	    if ((fPopulate5GBand && (CSR_IS_CHANNEL_5GHZ(pMac->scan.defaultPowerTable[Index].chanId))) ||
	 	        (!fPopulate5GBand && (CSR_IS_CHANNEL_24GHZ(pMac->scan.defaultPowerTable[Index].chanId))) )
	 	    {
                pChanInfo->firstChanNum = pMac->scan.defaultPowerTable[Index].chanId;
                pChanInfo->numChannels  = 1;
                pChanInfo->maxTxPower   = CSR_ROAM_MIN( pMac->scan.defaultPowerTable[Index].pwr, pMac->roam.configParam.nTxPowerCap );
                pChanInfo++;
                count++;
             }
        }
        if(count)
        {
            csrSaveToChannelPower2G_5G( pMac, count * sizeof(tSirMacChanInfo), pChanInfoStart );
        }
        palFreeMemory(pMac->hHdd, pChanInfoStart);
    }
}


void csrSetOppositeBandChannelInfo( tpAniSirGlobal pMac )
{
    tANI_BOOLEAN fPopulate5GBand = FALSE;

    do 
    {
        // if this is not a dual band product, then we don't need to set the opposite
        // band info.  We only work in one band so no need to look in the other band.
        if ( !CSR_IS_OPEARTING_DUAL_BAND( pMac ) ) break;
        // if we found channel info on the 5.0 band and...
        if ( CSR_IS_CHANNEL_5GHZ( pMac->scan.channelOf11dInfo ) )
        {
            // and the 2.4 band is empty, then populate the 2.4 channel info
            if ( !csrLLIsListEmpty( &pMac->scan.channelPowerInfoList24, LL_ACCESS_LOCK ) ) break;
            fPopulate5GBand = FALSE;
        }
        else
        {
            // else, we found channel info in the 2.4 GHz band.  If the 5.0 band is empty
            // set the 5.0 band info from the 2.4 country code.
            if ( !csrLLIsListEmpty( &pMac->scan.channelPowerInfoList5G, LL_ACCESS_LOCK ) ) break;
            fPopulate5GBand = TRUE;
        }
        csrSaveChannelPowerForBand( pMac, fPopulate5GBand );

    } while( 0 );
}


tANI_BOOLEAN csrIsSupportedChannel(tpAniSirGlobal pMac, tANI_U8 channelId)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U32 i;

    //Make sure it is a channel that is in our supported list.
    for ( i = 0; i < pMac->scan.baseChannels.numChannels; i++ )
    {
        if ( channelId == pMac->scan.baseChannels.channelList[i] )
        {
            fRet = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    //If it is configured to limit a set of the channels
    if( fRet && pMac->roam.configParam.fEnforce11dChannels )
    {
        fRet = eANI_BOOLEAN_FALSE;
        for ( i = 0; i < pMac->scan.base20MHzChannels.numChannels; i++ )
        {
            if ( channelId == pMac->scan.base20MHzChannels.channelList[i] )
            {
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
        }
    }

    return (fRet);
}



//bSize specify the buffer size of pChannelList
tANI_U8 csrGetChannelListFromChannelSet( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 bSize, tCsrChannelPowerInfo *pChannelSet )
{
    tANI_U8 i, j = 0, chnId;

    bSize = CSR_MIN(bSize, pChannelSet->numChannels);
    for( i = 0; i < bSize; i++ )
    {
        chnId = (tANI_U8)(pChannelSet->firstChannel + ( i * pChannelSet->interChannelOffset ));
        if ( csrIsSupportedChannel( pMac, chnId ) )
        {
            pChannelList[j++] = chnId;
        }
    }

    return (j);
}



//bSize -- specify the buffer size of pChannelList
void csrConstructCurrentValidChannelList( tpAniSirGlobal pMac, tDblLinkList *pChannelSetList, 
                                            tANI_U8 *pChannelList, tANI_U8 bSize, tANI_U8 *pNumChannels )
{
    tListElem *pEntry;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_U8 numChannels;
    tANI_U8 *pChannels;

    if( pChannelSetList && pChannelList && pNumChannels )
    {
        pChannels = pChannelList;
        *pNumChannels = 0;
        pEntry = csrLLPeekHead( pChannelSetList, LL_ACCESS_LOCK );
        while( pEntry )
        {
            pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
            numChannels = csrGetChannelListFromChannelSet( pMac, pChannels, bSize, pChannelSet );
            pChannels += numChannels;
            *pNumChannels += numChannels;
            pEntry = csrLLNext( pChannelSetList, pEntry, LL_ACCESS_LOCK );
        }
    }
}


/*
  * 802.11D only: Gather 11d IE via beacon or Probe response and store them in pAdapter->channels11d
*/
tANI_BOOLEAN csrLearnCountryInformation( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc,
                                         tDot11fBeaconIEs *pIes)
{
    tANI_U8 Num2GChannels, bMaxNumChn;
    eHalStatus status;
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    v_REGDOMAIN_t domainId;
    tDot11fBeaconIEs *pIesLocal = pIes;

#ifdef WLAN_SOFTAP_FEATURE
    if (VOS_STA_SAP_MODE == vos_get_conparam ())
        return eHAL_STATUS_SUCCESS;
#endif

    do
    {
        // check if .11d support is enabled
        if( !csrIs11dSupported( pMac ) ) break;
        if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc, &pIesLocal))) )
        {
            break;
        }
        // check if country information element is present
        if(!pIesLocal->Country.present)
        {
            //No country info
            break;
        }

        if( csrSave11dCountryString( pMac, pIesLocal->Country.country ) )
        {
            // country string changed, this should not happen
            //Need to check whether we care about this BSS' domain info
            //If it doesn't match of the connected profile or roaming profile, let's ignore it
            tANI_U32 i;
            tCsrRoamSession *pSession;

            for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
            {
                if( CSR_IS_SESSION_VALID( pMac, i ) )
                {
                    pSession = CSR_GET_SESSION( pMac, i );
                    if(pSession->pCurRoamProfile)
            {
                tCsrScanResultFilter filter;

                palZeroMemory(pMac->hHdd, &filter, sizeof(tCsrScanResultFilter));
                        status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, &filter);
                if(HAL_STATUS_SUCCESS(status))
                {
                    if(csrMatchBSS(pMac, pSirBssDesc, &filter, NULL, NULL, NULL, NULL))
                    {
                        smsLog(pMac, LOGW, "   Matching roam profile BSSID %02X-%02X-%02X-%02X-%02X-%02X causing ambiguous doamin info\n",
                            pSirBssDesc->bssId[0], pSirBssDesc->bssId[1], pSirBssDesc->bssId[2], 
                            pSirBssDesc->bssId[3], pSirBssDesc->bssId[4], pSirBssDesc->bssId[5]);
                        pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
                                break;
                    }
                    csrFreeScanFilter( pMac, &filter );
                }
            }
                    else if( csrIsConnStateConnected(pMac, i))
            {
                //Reach here only when the currention is base on no profile. 
                //User doesn't give profile and just connect to anything.
                        if(csrMatchBSSToConnectProfile(pMac, &pSession->connectedProfile, pSirBssDesc, pIesLocal))
                {
                    smsLog(pMac, LOGW, "   Matching connect profile BSSID %02X-%02X-%02X-%02X-%02X-%02X causing ambiguous doamin info\n",
                            pSirBssDesc->bssId[0], pSirBssDesc->bssId[1], pSirBssDesc->bssId[2],
                            pSirBssDesc->bssId[3], pSirBssDesc->bssId[4], pSirBssDesc->bssId[5]);
                    //Tush
                    pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
                            if(csrIsBssidMatch(pMac, (tCsrBssid *)&pSirBssDesc->bssId, 
                                                &pSession->connectedProfile.bssid))
                    {
                      //AP changed the 11d info on the fly, modify cfg
                      pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
                      fRet = eANI_BOOLEAN_TRUE;
                    }
                            break;
                }
            }
                } //valid session
            } //for
            if ( i == CSR_ROAM_SESSION_MAX ) 
            {
                //Check whether we can use this country's 11d information
                if( !pMac->roam.configParam.fEnforceDefaultDomain )
                {
                pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_TRUE;
            }
                else 
                {
                    VOS_ASSERT( pMac->scan.domainIdCurrent == pMac->scan.domainIdDefault );
                    if( HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( 
                                pMac, pIesLocal->Country.country, &domainId )) &&
                                ( domainId == pMac->scan.domainIdCurrent ) )
                    {
                        //Two countries in the same domain, do we consider this ambiguious?
                    }
                }
        }
        }
        else //Tush
        {
          pMac->scan.fCurrent11dInfoMatch = eANI_BOOLEAN_TRUE;
        }

        //In case that some channels in 5GHz have the same channel number as 2.4GHz (<= 14)
        if(CSR_IS_CHANNEL_5GHZ(pSirBssDesc->channelId))
        {
            tANI_U8 iC;
            tSirMacChanInfo* pMacChnSet = (tSirMacChanInfo *)(&pIesLocal->Country.triplets[0]);

            for(iC = 0; iC < pIesLocal->Country.num_triplets; iC++)
            {
                if(CSR_IS_CHANNEL_24GHZ(pMacChnSet[iC].firstChanNum))
                {
                    pMacChnSet[iC].firstChanNum += 200; //*** Where is this 200 defined?
                }
            }
        }
        // save the channel/power information from the Channel IE.
        //sizeof(tSirMacChanInfo) has to be 3
        csrSaveToChannelPower2G_5G( pMac, pIesLocal->Country.num_triplets * sizeof(tSirMacChanInfo), 
                                        (tSirMacChanInfo *)(&pIesLocal->Country.triplets[0]) );
        // set the indicator of the channel where the country IE was found...
        pMac->scan.channelOf11dInfo = pSirBssDesc->channelId;
        // Populate both band channel lists based on what we found in the country information...
        csrSetOppositeBandChannelInfo( pMac );
        bMaxNumChn = WNI_CFG_VALID_CHANNEL_LIST_LEN;
        // construct 2GHz channel list first
        csrConstructCurrentValidChannelList( pMac, &pMac->scan.channelPowerInfoList24, pMac->scan.channels11d.channelList, 
                                                bMaxNumChn, &Num2GChannels );
        // construct 5GHz channel list now
        if(bMaxNumChn > Num2GChannels)
        {
            csrConstructCurrentValidChannelList( pMac, &pMac->scan.channelPowerInfoList5G, pMac->scan.channels11d.channelList + Num2GChannels,
                                                 bMaxNumChn - Num2GChannels,
                                                 &pMac->scan.channels11d.numChannels );
        }

        pMac->scan.channels11d.numChannels += Num2GChannels;
        fRet = eANI_BOOLEAN_TRUE;

    } while( 0 );
    
    if( !pIes && pIesLocal )
    {
        //locally allocated
        palFreeMemory(pMac->hHdd, pIesLocal);
    }

    return( fRet );
}


static void csrSaveScanResults( tpAniSirGlobal pMac )
{
    // initialize this to FALSE. profMoveInterimScanResultsToMainList() routine
    // will set this to the channel where an .11d beacon is seen
    pMac->scan.channelOf11dInfo = 0;
    // if we get any ambiguous .11d information then this will be set to TRUE
    pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
    //Tush
    // if we get any ambiguous .11d information, then this will be set to TRUE
    // only if the applied 11d info could be found in one of the scan results
    pMac->scan.fCurrent11dInfoMatch = eANI_BOOLEAN_FALSE;
    // move the scan results from interim list to the main scan list
    csrMoveTempScanResultsToMainList( pMac );

    // Now check if we gathered any domain/country specific information
    // If so, we should update channel list and apply Tx power settings
    csrApplyCountryInformation( pMac, FALSE );
}


void csrReinitScanCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    switch (pCommand->u.scanCmd.reason)
    {
    case eCsrScanSetBGScanParam:
    case eCsrScanAbortBgScan:
        if(pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList)
        {
            palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList);
            pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList = NULL;
        }
        break;
    case eCsrScanBGScanAbort:
    case eCsrScanBGScanEnable:
    case eCsrScanGetScanChnInfo:
        break;
    case eCsrScanAbortNormalScan:
    default:
        csrScanFreeRequest(pMac, &pCommand->u.scanCmd.u.scanRequest);
        break;
    }
    if(pCommand->u.scanCmd.pToRoamProfile)
    {
        csrReleaseProfile(pMac, pCommand->u.scanCmd.pToRoamProfile);
        palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.pToRoamProfile);
    }
    palZeroMemory(pMac->hHdd, &pCommand->u.scanCmd, sizeof(tScanCmd));
}


tANI_BOOLEAN csrGetRemainingChannelsFor11dScan( tpAniSirGlobal pMac, tANI_U8 *pChannels, tANI_U8 *pcChannels )
{
    tANI_U32 index11dChannels, index;
    tANI_U32 indexCurrentChannels;
    tANI_BOOLEAN fChannelAlreadyScanned;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);

    *pcChannels = 0;
    if ( CSR_IS_11D_INFO_FOUND(pMac) && csrRoamIsChannelValid(pMac, pMac->scan.channelOf11dInfo) )
    {
        if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
        {
            //Find the channel index where we found the 11d info
            for(index = 0; index < len; index++)
            {
                if(pMac->scan.channelOf11dInfo == pMac->roam.validChannelList[index])
                    break;
            }
            //check whether we found the channel index
            if(index < len)
            {
                // Now, look through the 11d channel list and create a list of all channels in the 11d list that are
                // NOT in the current channel list.  This gives us a list of the new channels that have not been
                // scanned.  We'll scan this new list so we have a complete set of scan results on all of the domain channels
                // initially.
                for ( index11dChannels = 0; index11dChannels < pMac->scan.channels11d.numChannels; index11dChannels++ )
                {
                    fChannelAlreadyScanned = eANI_BOOLEAN_FALSE;

                    for( indexCurrentChannels = 0; indexCurrentChannels < index; indexCurrentChannels++ )
                    {
                        if ( pMac->roam.validChannelList[ indexCurrentChannels ] == pMac->scan.channels11d.channelList[ index11dChannels ] )
                        {
                            fChannelAlreadyScanned = eANI_BOOLEAN_TRUE;
                            break;
                        }
                    }

                    if ( !fChannelAlreadyScanned )
                    {
                        pChannels[ *pcChannels ] = pMac->scan.channels11d.channelList[ index11dChannels ];
                        ( *pcChannels )++;
                    }
                }
            }
        }//GetCFG
    }
    return( *pcChannels );
}


eCsrScanCompleteNextCommand csrScanGetNextCommandState( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fSuccess )
{
    eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
    
    switch( pCommand->u.scanCmd.reason )
    {
        case eCsrScan11d1:
            NextCommand = (fSuccess) ? eCsrNext11dScan1Success : eCsrNext11dScan1Failure;
            break;
        case eCsrScan11d2:
            NextCommand = (fSuccess) ? eCsrNext11dScan2Success : eCsrNext11dScan2Failure;
            break;    
        case eCsrScan11dDone:
            NextCommand = eCsrNext11dScanComplete;
            break;
        case eCsrScanLostLink1:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan1Success : eCsrNextLostLinkScan1Failed;
            break;
        case eCsrScanLostLink2:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan2Success : eCsrNextLostLinkScan2Failed;
            break;
        case eCsrScanLostLink3:
            NextCommand = (fSuccess) ? eCsrNextLostLinkScan3Success : eCsrNextLostLinkScan3Failed;
            break;
        case eCsrScanForSsid:
            NextCommand = (fSuccess) ? eCsrNexteScanForSsidSuccess : eCsrNexteScanForSsidFailure;
            break;
        case eCsrScanForCapsChange:
            NextCommand = eCsrNextCapChangeScanComplete;    //don't care success or not
            break;
        case eCsrScanIdleScan:
            NextCommand = eCsrNextIdleScanComplete;
            break;
        default:
            NextCommand = eCsrNextScanNothing;
            break;
    }
    return( NextCommand );
}


//Return whether the pCommand is finished.
tANI_BOOLEAN csrHandleScan11d1Failure(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_TRUE;
    
    //Apply back the default setting and passively scan one more time.
    csrResetCountryInformation(pMac, eANI_BOOLEAN_FALSE);
    pCommand->u.scanCmd.reason = eCsrScan11d2;
    if(HAL_STATUS_SUCCESS(csrScanChannels(pMac, pCommand)))
    {
        fRet = eANI_BOOLEAN_FALSE;
    }
    
    return (fRet);
}


tANI_BOOLEAN csrHandleScan11dSuccess(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_TRUE;
    tANI_U8 *pChannels;
    tANI_U8 cChannels;
    
    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, pChannels, WNI_CFG_VALID_CHANNEL_LIST_LEN);
        if ( csrGetRemainingChannelsFor11dScan( pMac, pChannels, &cChannels ) )
        {
            pCommand->u.scanCmd.reason = eCsrScan11dDone;
            if(pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList)
            {
                palFreeMemory(pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList); 
            }
            if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, cChannels)))
            {
                palCopyMemory(pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, pChannels, cChannels);
                pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = cChannels;
                pCommand->u.scanCmd.u.scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
                pCommand->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
                if(HAL_STATUS_SUCCESS(csrScanChannels(pMac, pCommand)))
                {
                    //Reuse the same command buffer
                    fRet = eANI_BOOLEAN_FALSE;
                }
            }
        }
        palFreeMemory(pMac->hHdd, pChannels);
    }
    
    return (fRet);
}

//Return whether the command should be removed
tANI_BOOLEAN csrScanComplete( tpAniSirGlobal pMac, tSirSmeScanRsp *pScanRsp )
{
    eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
    tListElem *pEntry;
    tSmeCmd *pCommand;
    tANI_BOOLEAN fRemoveCommand = eANI_BOOLEAN_TRUE;
    tANI_BOOLEAN fSuccess;

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );

    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );

        // If the head of the queue is Active and it is a SCAN command, remove
        // and put this on the Free queue.
        if ( eSmeCommandScan == pCommand->command )
        {     
            tANI_U32 sessionId = pCommand->sessionId;

            if(eSIR_SME_SUCCESS != pScanRsp->statusCode)
            {
                fSuccess = eANI_BOOLEAN_FALSE;
            }
            else
            {
                //pMac->scan.tempScanResults is not empty meaning the scan found something
                //This check only valid here because csrSaveScanresults is not yet called
                fSuccess = (!csrLLIsListEmpty(&pMac->scan.tempScanResults, LL_ACCESS_LOCK));
            }
            csrSaveScanResults(pMac);

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
            {
                vos_log_scan_pkt_type *pScanLog = NULL;
                tScanResultHandle hScanResult;
                tCsrScanResultInfo *pScanResult;
                tDot11fBeaconIEs *pIes;
                int n = 0, c = 0;

                WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
                if(pScanLog)
                {
                    if(eCsrScanBgScan == pCommand->u.scanCmd.reason || 
                        eCsrScanProbeBss == pCommand->u.scanCmd.reason ||
                        eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason)
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_RSP;
                    }
                    else
                    {
                        if( eSIR_PASSIVE_SCAN != pMac->scan.curScanType )
                        {
                            pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP;
                        }
                        else
                        {
                            pScanLog->eventId = WLAN_SCAN_EVENT_PASSIVE_SCAN_RSP;
                        }
                    }
                    if(eSIR_SME_SUCCESS == pScanRsp->statusCode)
                    {
                        if(HAL_STATUS_SUCCESS(csrScanGetResult(pMac, NULL, &hScanResult)))
                        {
                            while(((pScanResult = csrScanResultGetNext(pMac, hScanResult)) != NULL))
                            {
                                if( n < VOS_LOG_MAX_NUM_BSSID )
                                {
                                    if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, &pScanResult->BssDescriptor, &pIes)))
                                    {
                                        smsLog(pMac, LOGE, FL(" fail to parse IEs\n"));
                                        break;
                                    }
                                    palCopyMemory(pMac->hHdd, pScanLog->bssid[n], pScanResult->BssDescriptor.bssId, 6);
                                    if(pIes && pIes->SSID.present && VOS_LOG_MAX_SSID_SIZE >= pIes->SSID.num_ssid)
                                    {
                                        palCopyMemory(pMac->hHdd, pScanLog->ssid[n], 
                                                pIes->SSID.ssid, pIes->SSID.num_ssid);
                                    }
                                    palFreeMemory(pMac->hHdd, pIes);
                                    n++;
                                }
                                c++;
                            }
                            pScanLog->numSsid = (v_U8_t)n;
                            pScanLog->totalSsid = (v_U8_t)c;
                            csrScanResultPurge(pMac, hScanResult);
                        }
                    }
                    else
                    {
                        pScanLog->status = WLAN_SCAN_STATUS_FAILURE;
                    }
                    WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
                }
            }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

            NextCommand = csrScanGetNextCommandState(pMac, pCommand, fSuccess);
            //We reuse the command here instead reissue a new command
            switch(NextCommand)
            {
            case eCsrNext11dScan1Success:
            case eCsrNext11dScan2Success:
                smsLog( pMac, LOG2, FL("11dScan1/3 produced results.  Reissue Active scan...\n"));
                // if we found country information, no need to continue scanning further, bail out
                fRemoveCommand = eANI_BOOLEAN_TRUE;
                NextCommand = eCsrNext11dScanComplete;
                break;
            case eCsrNext11dScan1Failure:
                //We are not done yet. 11d scan fail once. We will try to reset anything and do it over again
                //The only meaningful thing for this retry is that we cannot find 11d information after a reset so
                //we clear the "old" 11d info and give it once more chance
                fRemoveCommand = csrHandleScan11d1Failure(pMac, pCommand);
                if(fRemoveCommand)
                {
                    NextCommand = eCsrNext11dScanComplete;
                } 
                break;
            case eCsrNextLostLinkScan1Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink1)))
                {
                    csrScanHandleFailedLostlink1(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan2Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink2)))
                {
                    csrScanHandleFailedLostlink2(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan3Success:
                if(!HAL_STATUS_SUCCESS(csrIssueRoamAfterLostlinkScan(pMac, sessionId, eCsrLostLink3)))
                {
                    csrScanHandleFailedLostlink3(pMac, sessionId);
                }
                break;
            case eCsrNextLostLinkScan1Failed:
                csrScanHandleFailedLostlink1(pMac, sessionId);
                break;
            case eCsrNextLostLinkScan2Failed:
                csrScanHandleFailedLostlink2(pMac, sessionId);
                break;
            case eCsrNextLostLinkScan3Failed:
                csrScanHandleFailedLostlink3(pMac, sessionId);
                break;    
            case eCsrNexteScanForSsidSuccess:
                csrScanHandleSearchForSSID(pMac, pCommand);
                break;
            case eCsrNexteScanForSsidFailure:
                csrScanHandleSearchForSSIDFailure(pMac, pCommand);
                break;
            case eCsrNextIdleScanComplete:
                pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
                break;
            case eCsrNextCapChangeScanComplete:
                csrScanHandleCapChangeScanComplete(pMac, sessionId);
                break;
            default:

                break;
            }
#ifdef FEATURE_WLAN_GEN6_ROAMING
            if( (eCsrScanSetBGScanParam != pCommand->u.scanCmd.reason) &&
                (eCsrScanBgScan != pCommand->u.scanCmd.reason) &&
                (eCsrScanProbeBss != pCommand->u.scanCmd.reason) &&
                (eSIR_SME_SUCCESS == pScanRsp->statusCode))
			{
				csrScanUpdateNList(pMac);
            }
#endif
        }
        else
        {
            smsLog( pMac, LOGW, FL("Scan Completion called but SCAN command is not ACTIVE ...\n"));
            fRemoveCommand = eANI_BOOLEAN_FALSE;
        }
    }
    else
    {
        smsLog( pMac, LOGW, FL("Scan Completion called but NO commands are ACTIVE ...\n"));
        fRemoveCommand = eANI_BOOLEAN_FALSE;
    }
   
    return( fRemoveCommand );
}



static void csrScanRemoveDupBssDescriptionFromInterimList( tpAniSirGlobal pMac, 
                                                           tSirBssDescription *pSirBssDescr,
                                                           tDot11fBeaconIEs *pIes)
{
    tListElem *pEntry;
    tCsrScanResult *pCsrBssDescription;

    // Walk through all the chained BssDescriptions.  If we find a chained BssDescription that
    // matches the BssID of the BssDescription passed in, then these must be duplicate scan
    // results for this Bss.  In that case, remove the 'old' Bss description from the linked list.
    pEntry = csrLLPeekHead( &pMac->scan.tempScanResults, LL_ACCESS_LOCK );
    while( pEntry ) 
    {
        pCsrBssDescription = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        // we have a duplicate scan results only when BSSID, SSID, Channel and NetworkType
        // matches

        if ( csrIsDuplicateBssDescription( pMac, &pCsrBssDescription->Result.BssDescriptor, 
                                             pSirBssDescr, pIes ) )
        {
            pSirBssDescr->rssi = (tANI_S8)( (((tANI_S32)pSirBssDescr->rssi * CSR_SCAN_RESULT_RSSI_WEIGHT ) +
                                    ((tANI_S32)pCsrBssDescription->Result.BssDescriptor.rssi * (100 - CSR_SCAN_RESULT_RSSI_WEIGHT) )) / 100 );

            // Remove the 'old' entry from the list....
            if( csrLLRemoveEntry( &pMac->scan.tempScanResults, pEntry, LL_ACCESS_LOCK ) )
            {
            // we need to free the memory associated with this node
                csrFreeScanResultEntry( pMac, pCsrBssDescription );
            }
            
            // If we found a match, we can stop looking through the list.
            break;
        }

        pEntry = csrLLNext( &pMac->scan.tempScanResults, pEntry, LL_ACCESS_LOCK );
    }
}



//Caller allocated memory pfNewBssForConn to return whether new candidate for
//current connection is found. Cannot be NULL
tCsrScanResult *csrScanSaveBssDescriptionToInterimList( tpAniSirGlobal pMac, 
                                                        tSirBssDescription *pBSSDescription)
{
    tCsrScanResult *pCsrBssDescription = NULL;
    tANI_U32 cbBSSDesc;
    tANI_U32 cbAllocated;
    eHalStatus halStatus;
    
    // figure out how big the BSS description is (the BSSDesc->length does NOT
    // include the size of the length field itself).
    cbBSSDesc = pBSSDescription->length + sizeof( pBSSDescription->length );

    cbAllocated = sizeof( tCsrScanResult ) + cbBSSDesc;

    halStatus = palAllocateMemory( pMac->hHdd, (void **)&pCsrBssDescription, cbAllocated );
    if ( HAL_STATUS_SUCCESS(halStatus) )
    {
        palZeroMemory(pMac->hHdd, pCsrBssDescription, cbAllocated);
        pCsrBssDescription->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        palCopyMemory(pMac->hHdd, &pCsrBssDescription->Result.BssDescriptor, pBSSDescription, cbBSSDesc );
        csrLLInsertTail( &pMac->scan.tempScanResults, &pCsrBssDescription->Link, LL_ACCESS_LOCK );
    }

    return( pCsrBssDescription );
}


    

tANI_BOOLEAN csrIsDuplicateBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc1, 
										   tSirBssDescription *pSirBssDesc2, tDot11fBeaconIEs *pIes2 )
{
    tANI_BOOLEAN fMatch = FALSE;
    tSirMacCapabilityInfo *pCap1, *pCap2;
    tDot11fBeaconIEs *pIes1 = NULL;

    pCap1 = (tSirMacCapabilityInfo *)&pSirBssDesc1->capabilityInfo;
    pCap2 = (tSirMacCapabilityInfo *)&pSirBssDesc2->capabilityInfo;
    if(pCap1->ess == pCap2->ess)
    {
        if(( csrIsNetworkTypeEqual( pSirBssDesc1, pSirBssDesc2 ) ))
        {
            if (pCap1->ess && 
                    csrIsMacAddressEqual( pMac, (tCsrBssid *)pSirBssDesc1->bssId, (tCsrBssid *)pSirBssDesc2->bssId))
            {
                fMatch = TRUE;
            }
            else if (pCap1->ibss && (pSirBssDesc1->channelId == pSirBssDesc2->channelId))
            {
                tDot11fBeaconIEs *pIesTemp = pIes2;

                do
                {
                    if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc1, &pIes1)))
                    {
                        break;
                    }
					if( NULL == pIesTemp )
					{
						if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc2, &pIesTemp)))
                        {
                            break;
                        }
					}
                    //Same channel cannot have same SSID for different IBSS
                    if(pIes1->SSID.present && pIesTemp->SSID.present)
                    {
                        fMatch = csrIsSsidMatch(pMac, pIes1->SSID.ssid, pIes1->SSID.num_ssid, 
                                                pIesTemp->SSID.ssid, pIesTemp->SSID.num_ssid, eANI_BOOLEAN_TRUE);
                    }
                }while(0);
                if( (NULL == pIes2) && pIesTemp )
                {
					//locally allocated
                    palFreeMemory(pMac->hHdd, pIesTemp);
                }
            }
        }
    }

    if(pIes1)
    {
        palFreeMemory(pMac->hHdd, pIes1);
    }

    return( fMatch );
}


tANI_BOOLEAN csrIsNetworkTypeEqual( tSirBssDescription *pSirBssDesc1, tSirBssDescription *pSirBssDesc2 )
{
    return( pSirBssDesc1->nwType == pSirBssDesc2->nwType );
}


//to check whether the BSS matches the dot11Mode
static tANI_BOOLEAN csrScanIsBssAllowed(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, 
                                        tDot11fBeaconIEs *pIes)
{
    tANI_BOOLEAN fAllowed = eANI_BOOLEAN_FALSE;
    eCsrPhyMode phyMode;

    if(HAL_STATUS_SUCCESS(csrGetPhyModeFromBss(pMac, pBssDesc, &phyMode, pIes)))
    {
        switch(pMac->roam.configParam.phyMode)
        {
        case eCSR_DOT11_MODE_11b:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a != phyMode);
            break;
        case eCSR_DOT11_MODE_11g:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a != phyMode);
            break;
        case eCSR_DOT11_MODE_11g_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11g == phyMode);
            break;
        case eCSR_DOT11_MODE_11a:
            fAllowed = (tANI_BOOLEAN)((eCSR_DOT11_MODE_11b != phyMode) && (eCSR_DOT11_MODE_11g != phyMode));
            break;
        case eCSR_DOT11_MODE_11n_ONLY:
            fAllowed = (tANI_BOOLEAN)((eCSR_DOT11_MODE_11n == phyMode) || (eCSR_DOT11_MODE_TAURUS == phyMode));
            break;
        case eCSR_DOT11_MODE_11b_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11b == phyMode);
            break;
        case eCSR_DOT11_MODE_11a_ONLY:
            fAllowed = (tANI_BOOLEAN)(eCSR_DOT11_MODE_11a == phyMode);
            break;
        case eCSR_DOT11_MODE_11n:
        case eCSR_DOT11_MODE_TAURUS:
        default:
            fAllowed = eANI_BOOLEAN_TRUE;
            break;
        }
    }
    if( fAllowed && pIes && pIes->SSID.present )
    {
        fAllowed = !( csrIsBogusSsid( pIes->SSID.ssid, pIes->SSID.num_ssid ) );
    }

    return (fAllowed);
}



//Return pIes to caller for future use when returning TRUE.
static tANI_BOOLEAN csrScanValidateScanResult( tpAniSirGlobal pMac, tANI_U8 *pChannels, 
                                               tANI_U8 numChn, tSirBssDescription *pBssDesc, 
                                               tDot11fBeaconIEs **ppIes )
{
    tANI_BOOLEAN fValidChannel = FALSE;
    tDot11fBeaconIEs *pIes = NULL;
	tANI_U8 index;

    for( index = 0; index < numChn; index++ )
    {
        // This check relies on the fact that a single BSS description is returned in each
        // ScanRsp call, which is the way LIM implemented the scan req/rsp funtions.  We changed 
        // to this model when we ran with a large number of APs.  If this were to change, then 
        // this check would have to mess with removing the bssDescription from somewhere in an 
        // arbitrary index in the bssDescription array.
        if ( pChannels[ index ] == pBssDesc->channelId ) 
        {
           fValidChannel = TRUE;
           break;
        }
    }
    *ppIes = NULL;
    if(fValidChannel)
    {
        if( HAL_STATUS_SUCCESS( csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIes) ) )
        {
            fValidChannel = csrScanIsBssAllowed(pMac, pBssDesc, pIes);
            if( fValidChannel )
            {
                *ppIes = pIes;
            }
            else
            {
                palFreeMemory( pMac->hHdd, pIes );
            }
        }
		else
		{
			fValidChannel = FALSE;
		}
    }

    return( fValidChannel );   
}


//Return whether last scan result is received
static tANI_BOOLEAN csrScanProcessScanResults( tpAniSirGlobal pMac, tSmeCmd *pCommand, 
                                                tSirSmeScanRsp *pScanRsp, tANI_BOOLEAN *pfRemoveCommand )
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE, fRemoveCommand = eANI_BOOLEAN_FALSE;
    tDot11fBeaconIEs *pIes = NULL;
    tANI_U32 cbParsed;
    tSirBssDescription *pSirBssDescription;
    tANI_U32 cbBssDesc;
    tANI_U32 cbScanResult = GET_FIELD_OFFSET( tSirSmeScanRsp, bssDescription ) 
                            + sizeof(tSirBssDescription);    //We need at least one CB

    // don't consider the scan rsp to be valid if the status code is Scan Failure.  Scan Failure
    // is returned when the scan could not find anything.  so if we get scan failure return that
    // the scan response is invalid.  Also check the lenght in the scan result for valid scan
    // BssDescriptions....
    do
    {
        if ( ( cbScanResult <= pScanRsp->length ) && 
             (( eSIR_SME_SUCCESS == pScanRsp->statusCode ) ||
              ( eSIR_SME_MORE_SCAN_RESULTS_FOLLOW == pScanRsp->statusCode ) ) )
        {
            tANI_U8 *pChannelList = NULL;
            tANI_U8 cChannels = 0;

            //Different scan type can reach this point, we need to distinguish it
            if( eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason )
            {
                //eCsrScanSetBGScanParam uses different structure
                tCsrBGScanRequest *pBgScanReq = &pCommand->u.scanCmd.u.bgScanRequest;

                cChannels = pBgScanReq->ChannelInfo.numOfChannels;
                pChannelList = pBgScanReq->ChannelInfo.ChannelList;
            }
            else
            {
                //the rest use generic scan request
                cChannels = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
                pChannelList = pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList;
            }

            // if the scan result is not on one of the channels in the Valid channel list, then it
            // must have come from an AP on an overlapping channel (in the 2.4GHz band).  In this case,
            // let's drop the scan result.
            //
            // The other situation is where the scan request is for a scan on a particular channel set
            // and the scan result is from a 
            
            // if the NumChannels is 0, then we are supposed to be scanning all channels.  Use the full channel
            // list as the 'valid' channel list.  Otherwise, use the specific channel list in the scan parms
            // as the valid channels.
            if ( 0 == cChannels ) 
            {
                tANI_U32 len = sizeof(pMac->roam.validChannelList);
                
                if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
                {
                    pChannelList = pMac->roam.validChannelList;
                    cChannels = (tANI_U8)len; 
                }
                else
                {
                    //Cannot continue
                    smsLog( pMac, LOGE, "CSR: Processsing internal SCAN results...csrGetCfgValidChannels failed\n" );
                    break;
                }
            }

            smsLog( pMac, LOG2, "CSR: Processsing internal SCAN results..." );
            cbParsed = GET_FIELD_OFFSET( tSirSmeScanRsp, bssDescription );
            pSirBssDescription = pScanRsp->bssDescription;
            while( cbParsed < pScanRsp->length )
            {
                if ( csrScanValidateScanResult( pMac, pChannelList, cChannels, pSirBssDescription, &pIes ) ) 
                {
                    csrScanRemoveDupBssDescriptionFromInterimList(pMac, pSirBssDescription, pIes);
                    csrScanSaveBssDescriptionToInterimList( pMac, pSirBssDescription );
                    if( eSIR_PASSIVE_SCAN == pMac->scan.curScanType )
                    {
                        if( csrIs11dSupported( pMac) )
                        {
                            //Check whether the BSS is acceptable base on 11d info and our configs.
                            if( csrMatchCountryCode( pMac, NULL, pIes ) )
                            {
                                //Double check whether the channel is acceptable by us.
                                if( csrIsSupportedChannel( pMac, pSirBssDescription->channelId ) )
                                {
                                    pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
                                }
                            }
                        }
                        else
                        {
                            pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
                        }
                    }
                    //Free the resource
                    palFreeMemory( pMac->hHdd, pIes );
                }
                // skip over the BSS description to the next one...
                cbBssDesc = pSirBssDescription->length + sizeof( pSirBssDescription->length );

                cbParsed += cbBssDesc;
                pSirBssDescription = (tSirBssDescription *)((tANI_U8 *)pSirBssDescription + cbBssDesc );

            } //while
        }
        else
        {
            smsLog( pMac, LOGW, " Scanrsp fail (0x%08X), length = %d\n", pScanRsp->statusCode, pScanRsp->length );
            //HO bg scan/probe failed no need to try autonomously
    	    if(eCsrScanBgScan == pCommand->u.scanCmd.reason ||
		       eCsrScanProbeBss == pCommand->u.scanCmd.reason ||
		       eCsrScanSetBGScanParam == pCommand->u.scanCmd.reason)
		    {
			    fRemoveCommand = eANI_BOOLEAN_TRUE;
            }
    }
    }while(0);
    if ( eSIR_SME_MORE_SCAN_RESULTS_FOLLOW != pScanRsp->statusCode )
    {
        smsLog(pMac, LOG1, " Scan received %d unique BSS scan reason is %d\n", csrLLCount(&pMac->scan.tempScanResults), pCommand->u.scanCmd.reason);
        fRemoveCommand = csrScanComplete( pMac, pScanRsp );
        fRet = eANI_BOOLEAN_TRUE;
    }//if ( eSIR_SME_MORE_SCAN_RESULTS_FOLLOW != pScanRsp->statusCode )
    if(pfRemoveCommand)
    {
        *pfRemoveCommand = fRemoveCommand;
    }

    return (fRet);
}


tANI_BOOLEAN csrScanIsWildCardScan( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    tANI_U8 bssid[WNI_CFG_BSSID_LEN] = {0, 0, 0, 0, 0, 0};
    tANI_BOOLEAN f = palEqualMemory( pMac->hHdd, pCommand->u.scanCmd.u.scanRequest.bssid, 
        bssid, sizeof(tCsrBssid) );

    //It is not a wild card scan if the bssid is not broadcast and the number of SSID is 1.
    return ((tANI_BOOLEAN)( (f || (0xff == pCommand->u.scanCmd.u.scanRequest.bssid[0])) &&
        (pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs != 1) ));
}


eHalStatus csrScanSmeScanResponse( tpAniSirGlobal pMac, void *pMsgBuf )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry;
    tSmeCmd *pCommand;
    eCsrScanStatus scanStatus;
    tSirSmeScanRsp *pScanRsp = (tSirSmeScanRsp *)pMsgBuf;
    tSmeGetScanChnRsp *pScanChnInfo;
    tANI_BOOLEAN fRemoveCommand = eANI_BOOLEAN_TRUE;
    eCsrScanReason reason = eCsrScanOther;

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );

    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if ( eSmeCommandScan == pCommand->command )
        {
            scanStatus = (eSIR_SME_SUCCESS == pScanRsp->statusCode) ? eCSR_SCAN_SUCCESS : eCSR_SCAN_FAILURE;
            reason = pCommand->u.scanCmd.reason;
            switch(pCommand->u.scanCmd.reason)
            {
            case eCsrScanAbortBgScan:
            case eCsrScanAbortNormalScan:
            case eCsrScanBGScanAbort:
            case eCsrScanBGScanEnable:
                break;
            case eCsrScanGetScanChnInfo:
                pScanChnInfo = (tSmeGetScanChnRsp *)pMsgBuf;
                csrScanAgeResults(pMac, pScanChnInfo);
                break;
            case eCsrScanForCapsChange:
                csrScanProcessScanResults( pMac, pCommand, pScanRsp, &fRemoveCommand );
                break;
            case eCsrScanSetBGScanParam:
            default:
                if(csrScanProcessScanResults( pMac, pCommand, pScanRsp, &fRemoveCommand ))
                {
                    //Not to get channel info if the scan is not a wildcard scan because
                    //it may cause scan results got aged out incorrectly.
                    if( csrScanIsWildCardScan( pMac, pCommand ) )
                    {
                        //Get the list of channels scanned
                        csrScanGetScanChnInfo(pMac);
                    }
                }
                break;
            }//switch
            if(fRemoveCommand)
            {
#ifdef FEATURE_WLAN_GEN6_ROAMING
                //Save the callback info, we may need to carry on
                csrScanCompleteCallback scanCallback = pCommand->u.scanCmd.callback;
                void *pCallbackContext = pCommand->u.scanCmd.pContext;
#endif

                csrReleaseScanCommand(pMac, pCommand, scanStatus);

#ifdef FEATURE_WLAN_GEN6_ROAMING
                // if we are doing the 1 channel at a time HDD scan, lets continue
                if((eCsrScanUserRequest == reason) && (csrScanGetChannelMask(pMac)))
                {
                    tCsrScanRequest scanReq;
                    tANI_U32 scanId;

                    palZeroMemory(pMac->hHdd, &scanReq, sizeof(scanReq) );

                    scanReq.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

                    palFillMemory( pMac->hHdd, &scanReq.bssid, sizeof( tCsrBssid ), 0xff );

                    scanReq.BSSType     = eCSR_BSS_TYPE_ANY;
                    scanReq.scanType    = eSIR_ACTIVE_SCAN;
                    scanReq.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

                    csrScanRequest( pMac, &scanReq, &scanId, scanCallback, pCallbackContext );
                }
#endif
            }
            smeProcessPendingQueue( pMac );
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: Scan Completion called but SCAN command is not ACTIVE ..." );
            status = eHAL_STATUS_FAILURE;
        }
    }
    else
    {
        smsLog( pMac, LOGW, "CSR: Scan Completion called but NO commands are ACTIVE ..." );
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}




tCsrScanResultInfo *csrScanResultGetFirst(tpAniSirGlobal pMac, tScanResultHandle hScanResult)
{
    tListElem *pEntry;
    tCsrScanResult *pResult;
    tCsrScanResultInfo *pRet = NULL;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    
    if(pResultList)
    {
        csrLLLock(&pResultList->List);
        pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        if(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            pRet = &pResult->Result;
        }
        pResultList->pCurEntry = pEntry;
        csrLLUnlock(&pResultList->List);
    }
    
    return pRet;
}


tCsrScanResultInfo *csrScanResultGetNext(tpAniSirGlobal pMac, tScanResultHandle hScanResult)
{
    tListElem *pEntry = NULL;
    tCsrScanResult *pResult = NULL;
    tCsrScanResultInfo *pRet = NULL;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    
    if(pResultList)
    {
        csrLLLock(&pResultList->List);
        if(NULL == pResultList->pCurEntry)
        {
            pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        }
        else
        {
            pEntry = csrLLNext(&pResultList->List, pResultList->pCurEntry, LL_ACCESS_NOLOCK);
        }
        if(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            pRet = &pResult->Result;
        }
        pResultList->pCurEntry = pEntry;
        csrLLUnlock(&pResultList->List);
    }
    
    return pRet;
}


//This function moves the first BSS that matches the bssid to the head of the result
eHalStatus csrMoveBssToHeadFromBSSID(tpAniSirGlobal pMac, tCsrBssid *bssid, tScanResultHandle hScanResult)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultList *pResultList = (tScanResultList *)hScanResult;
    tCsrScanResult *pResult = NULL;
    tListElem *pEntry = NULL;
   
    if(pResultList && bssid)
    {
        csrLLLock(&pResultList->List);
        pEntry = csrLLPeekHead(&pResultList->List, LL_ACCESS_NOLOCK);
        while(pEntry)
        {
            pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
            if(palEqualMemory(pMac->hHdd, bssid, pResult->Result.BssDescriptor.bssId, sizeof(tCsrBssid)))
            {
                status = eHAL_STATUS_SUCCESS;
                csrLLRemoveEntry(&pResultList->List, pEntry, LL_ACCESS_NOLOCK);
                csrLLInsertHead(&pResultList->List, pEntry, LL_ACCESS_NOLOCK);
                break;
            }
            pEntry = csrLLNext(&pResultList->List, pResultList->pCurEntry, LL_ACCESS_NOLOCK);
        }
        csrLLUnlock(&pResultList->List);
    }
    
    return (status);
}


//Remove the BSS if possible.
//Return -- TRUE == the BSS is remove. False == Fail to remove it
tANI_BOOLEAN csrScanAgeOutBss(tpAniSirGlobal pMac, tCsrScanResult *pResult)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U32 i;
    tCsrRoamSession *pSession;

    for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
    {
        if( CSR_IS_SESSION_VALID( pMac, i ) )
        {
            pSession = CSR_GET_SESSION( pMac, i );
    //Not to remove the BSS we are connected to.
            if(csrIsConnStateDisconnected(pMac, i) || (NULL == pSession->pConnectBssDesc) ||
        (!csrIsDuplicateBssDescription(pMac, &pResult->Result.BssDescriptor, 
                                                pSession->pConnectBssDesc, NULL))
        )
    {
        smsLog(pMac, LOGW, "Aging out BSS %02X-%02X-%02X-%02X-%02X-%02X Channel %d\n",
                                    pResult->Result.BssDescriptor.bssId[0],
                                    pResult->Result.BssDescriptor.bssId[1],
                                    pResult->Result.BssDescriptor.bssId[2],
                                    pResult->Result.BssDescriptor.bssId[3],
                                    pResult->Result.BssDescriptor.bssId[4],
                                    pResult->Result.BssDescriptor.bssId[5],
                                    pResult->Result.BssDescriptor.channelId);
        //No need to hold the spin lock because caller should hold the lock for pMac->scan.scanResultList
        if( csrLLRemoveEntry(&pMac->scan.scanResultList, &pResult->Link, LL_ACCESS_NOLOCK) )
		{
			csrFreeScanResultEntry(pMac, pResult);
		}
        fRet = eANI_BOOLEAN_TRUE;
                break;
    }
        } //valid session
    } //for
    if( CSR_ROAM_SESSION_MAX == i )
    {
        //reset the counter so this won't hapeen too soon
        pResult->AgingCount = (tANI_S32)pMac->roam.configParam.agingCount;
        pResult->Result.BssDescriptor.nReceivedTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);
    }

    return (fRet);
}


eHalStatus csrScanAgeResults(tpAniSirGlobal pMac, tSmeGetScanChnRsp *pScanChnInfo)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry, *tmpEntry;
    tCsrScanResult *pResult;
    tLimScanChn *pChnInfo;
    tANI_U8 i;

    csrLLLock(&pMac->scan.scanResultList);
    for(i = 0; i < pScanChnInfo->numChn; i++)
    {
        pChnInfo = &pScanChnInfo->scanChn[i];
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            tmpEntry = csrLLNext(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK);
            pResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            if(pResult->Result.BssDescriptor.channelId == pChnInfo->channelId)
            {
                pResult->AgingCount--;
                if(pResult->AgingCount <= 0)
                {
                    csrScanAgeOutBss(pMac, pResult);
                }
            }
            pEntry = tmpEntry;
        }
    }
    csrLLUnlock(&pMac->scan.scanResultList);

    return (status);
}


eHalStatus csrSendMBScanReq( tpAniSirGlobal pMac, tCsrScanRequest *pScanReq, tScanReqParam *pScanReqParam )
{
	eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeScanReq *pMsg;
    tANI_U16 msgLen;
    tANI_U8 bssid[WNI_CFG_BSSID_LEN] = {0, 0, 0, 0, 0, 0};
    tSirScanType scanType = pScanReq->scanType;
    tANI_U32 minChnTime;    //in units of milliseconds
    tANI_U32 maxChnTime;    //in units of milliseconds
    tANI_U32 i;
    tANI_U8 selfMacAddr[WNI_CFG_BSSID_LEN];
    tANI_U8 *pSelfMac;

    msgLen = (tANI_U16)(sizeof( tSirSmeScanReq ) - sizeof( pMsg->channelList.channelNumber ) + 
		               ( sizeof( pMsg->channelList.channelNumber ) * pScanReq->ChannelInfo.numOfChannels ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_REQ);
        pMsg->length = pal_cpu_to_be16(msgLen);
        //ToDO: Fill in session info when we need to do scan base on session.
        pMsg->sessionId = 0;
        pMsg->transactionId = 0;
        pMsg->dot11mode = (tANI_U8) csrTranslateToWNICfgDot11Mode(pMac, csrFindBestPhyMode( pMac, pMac->roam.configParam.phyMode ));
        pMsg->bssType = pal_cpu_to_be32(csrTranslateBsstypeToMacType(pScanReq->BSSType));

        // Since we don't have session for the scanning, we find a valid session. In case we fail to
        // do so, get the WNI_CFG_STA_ID
        for( i = 0; i < CSR_ROAM_SESSION_MAX; i++ )
        {
            if( CSR_IS_SESSION_VALID( pMac, i ) )
            {
                pSelfMac = (tANI_U8 *)&pMac->roam.roamSession[i].selfMacAddr;
            }
        }
        if( CSR_ROAM_SESSION_MAX == i )
        {
            tANI_U32 len = WNI_CFG_BSSID_LEN;
            pSelfMac = selfMacAddr;
            if( !HAL_STATUS_SUCCESS( ccmCfgGetStr( pMac, WNI_CFG_STA_ID, pSelfMac, &len ) ) || 
                ( len < WNI_CFG_BSSID_LEN ) )
            {
#if defined( VOSS_ENABLED )
                VOS_ASSERT( 0 );
#endif
            }
        }
        palCopyMemory( pMac->hHdd, (tANI_U8 *)pMsg->selfMacAddr, pSelfMac, sizeof(tSirMacAddr) );

        //sirCopyMacAddr
        palCopyMemory( pMac->hHdd, (tANI_U8 *)pMsg->bssId, (tANI_U8 *)&pScanReq->bssid, sizeof(tSirMacAddr) );
        if( palEqualMemory( pMac->hHdd, pScanReq->bssid, bssid, sizeof(tCsrBssid) ) )
        {
            palFillMemory( pMac->hHdd, pMsg->bssId, sizeof(tSirMacAddr), 0xff );
        }
        else
        {
            palCopyMemory(pMac->hHdd, pMsg->bssId, pScanReq->bssid, WNI_CFG_BSSID_LEN); 
        }
        minChnTime = pScanReq->minChnTime;
        maxChnTime = pScanReq->maxChnTime;

        //Verify the scan type first, if the scan is active scan, we need to make sure we 
        //are allowed to do so.
        if( eSIR_PASSIVE_SCAN != scanType )
        {
            scanType = pMac->scan.curScanType;
            if(eSIR_PASSIVE_SCAN == pMac->scan.curScanType)
        {
           if(minChnTime < pMac->roam.configParam.nPassiveMinChnTime) 
           {
              minChnTime = pMac->roam.configParam.nPassiveMinChnTime;
           }
           if(maxChnTime < pMac->roam.configParam.nPassiveMaxChnTime)
           {
              maxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
           }
        }
        }
        pMsg->scanType = pal_cpu_to_be32(scanType);


        if((pScanReq->SSIDs.numOfSSIDs != 0) && ( eSIR_PASSIVE_SCAN != scanType ))
        {
            palCopyMemory(pMac->hHdd, &pMsg->ssId, &pScanReq->SSIDs.SSIDList[0].SSID, sizeof(tSirMacSSid));
        }
        else
        {
            //Otherwise we scan all SSID and let the result filter later
            pMsg->ssId.length = 0;
        }

//TODO: This preprocessor macro should be removed from CSR for production driver
//This is a temperarory fix for scanning on FPGA.
#if defined (ANI_CHIPSET_VIRGO) || defined (LIBRA_FPGA)
        pMsg->minChannelTime = pal_cpu_to_be32(minChnTime * 8);
        pMsg->maxChannelTime = pal_cpu_to_be32(maxChnTime * 8);
#elif defined (ANI_CHIPSET_TAURUS) || defined(ANI_CHIPSET_LIBRA)
        pMsg->minChannelTime = pal_cpu_to_be32(minChnTime);
        pMsg->maxChannelTime = pal_cpu_to_be32(maxChnTime);
#else
#error unknown chipset
#endif
        //hidden SSID option
        pMsg->hiddenSsid = pScanReqParam->hiddenSsid;
        //rest time
        //pMsg->restTime = pScanReq->restTime;
        pMsg->returnAfterFirstMatch = pScanReqParam->bReturnAfter1stMatch;
	    // All the scan results caching will be done by Roaming
	    // We do not want LIM to do any caching of scan results,
	    // so delete the LIM cache on all scan requests
        pMsg->returnFreshResults = pScanReqParam->freshScan;
        //Always ask for unique result
        pMsg->returnUniqueResults = pScanReqParam->fUniqueResult;
        pMsg->channelList.numChannels = (tANI_U8)pScanReq->ChannelInfo.numOfChannels;
        if(pScanReq->ChannelInfo.numOfChannels)
        {
            //Assuming the channelNumber is tANI_U8 (1 byte)
            status = palCopyMemory(pMac->hHdd, pMsg->channelList.channelNumber, pScanReq->ChannelInfo.ChannelList, 
                                    pScanReq->ChannelInfo.numOfChannels);
        }

	    if(HAL_STATUS_SUCCESS(status))
        {
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        else {
            palFreeMemory(pMac->hHdd, pMsg);
        }
    }                             

	return( status );
}

eHalStatus csrSendMBScanResultReq( tpAniSirGlobal pMac, tScanReqParam *pScanReqParam )
{
	eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeScanReq *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirSmeScanReq ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_REQ);
        pMsg->length = pal_cpu_to_be16(msgLen);
        pMsg->returnFreshResults = pScanReqParam->freshScan;
        //Always ask for unique result
        pMsg->returnUniqueResults = pScanReqParam->fUniqueResult;
        pMsg->returnAfterFirstMatch = pScanReqParam->bReturnAfter1stMatch;
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

	return( status );
}



eHalStatus csrScanChannels( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
  	eHalStatus status = eHAL_STATUS_FAILURE;
    tScanReqParam scanReq;
    
    do
    {    
        scanReq.freshScan = CSR_SME_SCAN_FLAGS_DELETE_CACHE | TRUE;
        scanReq.fUniqueResult = TRUE;
        scanReq.hiddenSsid = SIR_SCAN_NO_HIDDEN_SSID;
        if(eCsrScanForSsid == pCommand->u.scanCmd.reason)
        {
            scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_FIRST_MATCH;
        }
        else
        {
            if(eCSR_SCAN_REQUEST_11D_SCAN == pCommand->u.scanCmd.u.scanRequest.requestType)
            {
                if(CSR_IS_OPEARTING_DUAL_BAND(pMac))
                {
                    //To ask for both band 
                    scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_EITHER_BAND_11d_FOUND;
                }
                else if(CSR_IS_OPERATING_BG_BAND(pMac))
                {
                    //To ask for 2.4GHz band 
                    scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_24_BAND_11d_FOUND;
                }
                else
                {
                    //To ask for 5GHz band 
                    scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_5_BAND_11d_FOUND;
                }
            }
            else
            {
                scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_ALL_CHANNELS;
            }
        }
        if((eCsrScanBgScan == pCommand->u.scanCmd.reason)||
           (eCsrScanProbeBss == pCommand->u.scanCmd.reason))
        {
            scanReq.hiddenSsid = SIR_SCAN_HIDDEN_SSID_PE_DECISION;
        }
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
        {
            vos_log_scan_pkt_type *pScanLog = NULL;

            WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
            if(pScanLog)
            {
                if(eCsrScanBgScan == pCommand->u.scanCmd.reason || 
                    eCsrScanProbeBss == pCommand->u.scanCmd.reason)
                {
                    pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_REQ;
                }
                else
                {
                    if( (eSIR_PASSIVE_SCAN != pCommand->u.scanCmd.u.scanRequest.scanType) && 
                        (eSIR_PASSIVE_SCAN != pMac->scan.curScanType) )
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ;
                    }
                    else
                    {
                        pScanLog->eventId = WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ;
                    }
                }
                pScanLog->minChnTime = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.minChnTime;
                pScanLog->maxChnTime = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.maxChnTime;
                pScanLog->numChannel = (v_U8_t)pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
                if(pScanLog->numChannel && (pScanLog->numChannel < VOS_LOG_MAX_NUM_CHANNEL))
                {
                    palCopyMemory(pMac->hHdd, pScanLog->channels, pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
                        pScanLog->numChannel);
                }
                WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
            }
        }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR


	    status = csrSendMBScanReq(pMac, &pCommand->u.scanCmd.u.scanRequest, &scanReq);
    }while(0);
    
  	return( status );
}


eHalStatus csrScanRetrieveResult(tpAniSirGlobal pMac)
{
  	eHalStatus status = eHAL_STATUS_FAILURE;
    tScanReqParam scanReq;
    
    do
    {    
        //not a fresh scan
        scanReq.freshScan = CSR_SME_SCAN_FLAGS_DELETE_CACHE;
        scanReq.fUniqueResult = TRUE;
        scanReq.bReturnAfter1stMatch = CSR_SCAN_RETURN_AFTER_ALL_CHANNELS;
	    status = csrSendMBScanResultReq(pMac, &scanReq);
    }while(0);
    
    return (status);
}



eHalStatus csrProcessScanCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    // Transition to Scanning state...
    pCommand->u.scanCmd.lastRoamState = csrRoamStateChange( pMac, eCSR_ROAMING_STATE_SCANNING );

    smsLog( pMac, LOG3, "starting SCAN command from %d state.... reason is %d\n", pCommand->u.scanCmd.lastRoamState, pCommand->u.scanCmd.reason );
    
    switch(pCommand->u.scanCmd.reason)
    {
    case eCsrScanGetResult:
    case eCsrScanForCapsChange:     //For cap change, LIM already save BSS description
        status = csrScanRetrieveResult(pMac);
        break;
    case eCsrScanSetBGScanParam:
        status = csrProcessSetBGScanParam(pMac, pCommand);
        break;
    case eCsrScanBGScanAbort:
        status = csrSetCfgBackgroundScanPeriod(pMac, 0);
        break;
    case eCsrScanBGScanEnable:
        status = csrSetCfgBackgroundScanPeriod(pMac, pMac->roam.configParam.bgScanInterval);
        break;
    case eCsrScanGetScanChnInfo:
        status = csrScanGetScanChannelInfo(pMac);
        break;
    default:
        status = csrScanChannels(pMac, pCommand);
        break;
    }    
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrReleaseScanCommand(pMac, pCommand, eCSR_SCAN_FAILURE);
    }
    
    return (status);
}


eHalStatus csrScanSetBGScanparams(tpAniSirGlobal pMac, tCsrBGScanRequest *pScanReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    if(pScanReq)
    {
        do
        {
            pCommand = csrGetCommandBuffer(pMac);
            if(!pCommand)
            {
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            pCommand->command = eSmeCommandScan;
            pCommand->u.scanCmd.reason = eCsrScanSetBGScanParam;
            pCommand->u.scanCmd.callback = NULL;
            pCommand->u.scanCmd.pContext = NULL;
            palCopyMemory(pMac->hHdd, &pCommand->u.scanCmd.u.bgScanRequest, pScanReq, sizeof(tCsrBGScanRequest));
            //we have to do the follow
            if(pScanReq->ChannelInfo.numOfChannels == 0)
            {
                pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList,
                                             pScanReq->ChannelInfo.numOfChannels);
                if(HAL_STATUS_SUCCESS(status))
                {
                    palCopyMemory(pMac->hHdd, pCommand->u.scanCmd.u.bgScanRequest.ChannelInfo.ChannelList,
                                    pScanReq->ChannelInfo.ChannelList, pScanReq->ChannelInfo.numOfChannels); 
                }
                else
                {
                    smsLog(pMac, LOGE, FL("ran out of memory\n"));
                    csrReleaseCommandScan(pMac, pCommand);
                    break;
                }
            }

            //scan req for SSID
            if(pScanReq->SSID.length)
            {
               palCopyMemory(pMac->hHdd, 
                             pCommand->u.scanCmd.u.bgScanRequest.SSID.ssId,
                             pScanReq->SSID.ssId, 
                             pScanReq->SSID.length);
               pCommand->u.scanCmd.u.bgScanRequest.SSID.length = pScanReq->SSID.length;

            }
            pCommand->u.scanCmd.u.bgScanRequest.maxChnTime= pScanReq->maxChnTime;
            pCommand->u.scanCmd.u.bgScanRequest.minChnTime = pScanReq->minChnTime;
            pCommand->u.scanCmd.u.bgScanRequest.scanInterval = pScanReq->scanInterval;


            status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan( pMac, pCommand );
                break;
            }
        }while(0);
    }
    
    return (status);
}

eHalStatus csrScanBGScanAbort( tpAniSirGlobal pMac )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(!pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandScan; 
        pCommand->u.scanCmd.reason = eCsrScanBGScanAbort;
        pCommand->u.scanCmd.callback = NULL;
        pCommand->u.scanCmd.pContext = NULL;
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            csrReleaseCommandScan( pMac, pCommand );
            break;
        }
    }while(0);
    
    return (status);
}


//This will enable the background scan with the non-zero interval 
eHalStatus csrScanBGScanEnable(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand = NULL;
    
    if(pMac->roam.configParam.bgScanInterval)
    {
        do
        {
            pCommand = csrGetCommandBuffer(pMac);
            if(!pCommand)
            {
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            pCommand->command = eSmeCommandScan; 
            pCommand->u.scanCmd.reason = eCsrScanBGScanEnable;
            pCommand->u.scanCmd.callback = NULL;
            pCommand->u.scanCmd.pContext = NULL;
            status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                csrReleaseCommandScan( pMac, pCommand );
                break;
            }
        }while(0);
        //BG scan results are reported automatically by PE to SME once the scan is done.
        //No need to fetch the results explicitly.
        //csrScanStartGetResultTimer(pMac);
        csrScanStartResultAgingTimer(pMac);
    }
    else
    {
        //We don't have BG scan so stop the aging timer
        csrScanStopResultAgingTimer(pMac);
        smsLog(pMac, LOGE, FL("cannot continue because the bgscan interval is 0\n"));
        status = eHAL_STATUS_INVALID_PARAMETER;
    }
    
    return (status);
}


eHalStatus csrScanCopyRequest(tpAniSirGlobal pMac, tCsrScanRequest *pDstReq, tCsrScanRequest *pSrcReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
	 
#ifdef FEATURE_WLAN_GEN6_ROAMING	 
    tANI_U8  index =0;
#endif

    do
    {
        status = csrScanFreeRequest(pMac, pDstReq);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = palCopyMemory(pMac->hHdd, pDstReq, pSrcReq, sizeof(tCsrScanRequest));
            if(pSrcReq->uIEFieldLen == 0)
            {
                pDstReq->pIEField = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->pIEField, pSrcReq->uIEFieldLen);
                if(HAL_STATUS_SUCCESS(status))
                {
                    palCopyMemory(pMac->hHdd, pDstReq->pIEField, pSrcReq->pIEField, pSrcReq->uIEFieldLen);
                }
                else
                {
                    smsLog(pMac, LOGE, "No memory for scanning IE fields\n");
                    break;
                }
            }//Allocate memory for IE field

#ifdef FEATURE_WLAN_GEN6_ROAMING
            if(csrScanGetChannelMask(pMac))
            {
                //clear one channel & scan on that channel
                status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->ChannelInfo.ChannelList, 
                                           1);
                if(HAL_STATUS_SUCCESS(status))
                {
                    for(index = 0; index < pMac->scan.osScanChannelMask.numChannels; index++)
                    {
                        if(TRUE == pMac->scan.osScanChannelMask.scanEnabled[index])
                        {
                            pDstReq->ChannelInfo.ChannelList[0] = 
                                pMac->scan.osScanChannelMask.channelList[index];
                            pDstReq->ChannelInfo.numOfChannels = 1;
                            pMac->scan.osScanChannelMask.scanEnabled[index] = FALSE;
                            break;
                        }
                    }
                    //didn't find any channel to scan
                    if(index == pMac->scan.osScanChannelMask.numChannels)
                    {
                        palFreeMemory(pMac->hHdd, (void **)&pDstReq->ChannelInfo.ChannelList);
                        pDstReq->ChannelInfo.ChannelList = NULL;
                        pDstReq->ChannelInfo.numOfChannels = 0;
                    }
                }
                else
                {
                    smsLog(pMac, LOGE, "No memory for scanning Channel List\n");
                    break;
                }
            }
            else
#endif //#def FEATURE_WLAN_GEN6_ROAMING
            {
                if(pSrcReq->ChannelInfo.numOfChannels == 0)
                {
                    pDstReq->ChannelInfo.ChannelList = NULL;
                        pDstReq->ChannelInfo.numOfChannels = 0;
                }
                else
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->ChannelInfo.ChannelList, 
                                        pSrcReq->ChannelInfo.numOfChannels * sizeof(*pDstReq->ChannelInfo.ChannelList));
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        palCopyMemory(pMac->hHdd, pDstReq->ChannelInfo.ChannelList, pSrcReq->ChannelInfo.ChannelList, 
                                        pSrcReq->ChannelInfo.numOfChannels * sizeof(*pDstReq->ChannelInfo.ChannelList));
                            pDstReq->ChannelInfo.numOfChannels = pSrcReq->ChannelInfo.numOfChannels;
                    }
                    else
                    {
                            pDstReq->ChannelInfo.numOfChannels = 0;
                        smsLog(pMac, LOGE, "No memory for scanning Channel List\n");
                        break;
                    }
                }//Allocate memory for Channel List
            }
            if(pSrcReq->SSIDs.numOfSSIDs == 0)
            {
                pDstReq->SSIDs.numOfSSIDs = 0;
                pDstReq->SSIDs.SSIDList = NULL;
            }
            else
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pDstReq->SSIDs.SSIDList, 
                                    pSrcReq->SSIDs.numOfSSIDs * sizeof(*pDstReq->SSIDs.SSIDList));
                if(HAL_STATUS_SUCCESS(status))
                {
                    pDstReq->SSIDs.numOfSSIDs = pSrcReq->SSIDs.numOfSSIDs;
                    palCopyMemory(pMac->hHdd, pDstReq->SSIDs.SSIDList, pSrcReq->SSIDs.SSIDList, 
                                    pSrcReq->SSIDs.numOfSSIDs * sizeof(*pDstReq->SSIDs.SSIDList));
                }
                else
                {
                    pDstReq->SSIDs.numOfSSIDs = 0;
                    smsLog(pMac, LOGE, "No memory for scanning SSID List\n");
                    break;
                }
            }//Allocate memory for SSID List
        }
    }while(0);
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrScanFreeRequest(pMac, pDstReq);
    }
    
    return (status);
}


eHalStatus csrScanFreeRequest(tpAniSirGlobal pMac, tCsrScanRequest *pReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    if(pReq->ChannelInfo.ChannelList)
    {
        status = palFreeMemory(pMac->hHdd, pReq->ChannelInfo.ChannelList);
        pReq->ChannelInfo.ChannelList = NULL;
    }
    pReq->ChannelInfo.numOfChannels = 0;
    if(pReq->pIEField)
    {
        status = palFreeMemory(pMac->hHdd, pReq->pIEField);
        pReq->pIEField = NULL;
    }
    pReq->uIEFieldLen = 0;
    if(pReq->SSIDs.SSIDList)
    {
        palFreeMemory(pMac->hHdd, pReq->SSIDs.SSIDList);
        pReq->SSIDs.SSIDList = NULL;
    }
    pReq->SSIDs.numOfSSIDs = 0;
    
    return (status);
}


void csrScanCallCallback(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus)
{
    if(pCommand->u.scanCmd.callback)
    {
        pCommand->u.scanCmd.callback(pMac, pCommand->u.scanCmd.pContext, pCommand->u.scanCmd.scanID, scanStatus); 
    }
}


void csrScanStopTimers(tpAniSirGlobal pMac)
{
    csrScanStopResultAgingTimer(pMac);
    csrScanStopIdleScanTimer(pMac);
    csrScanStopGetResultTimer(pMac);
}


eHalStatus csrScanStartGetResultTimer(tpAniSirGlobal pMac)
{
    eHalStatus status;
    
    if(pMac->scan.fScanEnable)
    {
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerGetResult, CSR_SCAN_GET_RESULT_INTERVAL, eANI_BOOLEAN_TRUE);
    }
    else
    {
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopGetResultTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerGetResult));
}


void csrScanGetResultTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    
    csrScanRequestResult(pMac);
}


eHalStatus csrScanStartResultAgingTimer(tpAniSirGlobal pMac)
{
    eHalStatus status;
    
    if(pMac->scan.fScanEnable)
    {
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerResultAging, CSR_SCAN_RESULT_AGING_INTERVAL, eANI_BOOLEAN_TRUE);
    }
    else
    {
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopResultAgingTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerResultAging));
}


//This function returns the maximum time a BSS is allowed in the scan result.
//The time varies base on connection and power saving factors.
//Not connected, No PS
//Not connected, with PS
//Connected w/o traffic, No PS
//Connected w/o traffic, with PS
//Connected w/ traffic, no PS -- Not supported
//Connected w/ traffic, with PS -- Not supported
//the return unit is in seconds. 
tANI_U32 csrScanGetAgeOutTime(tpAniSirGlobal pMac)
{
    tANI_U32 nRet;

    if(pMac->scan.nAgingCountDown)
    {
        //Calculate what should be the timeout value for this
        nRet = pMac->scan.nLastAgeTimeOut * pMac->scan.nAgingCountDown;
        pMac->scan.nAgingCountDown--;
    }
    else
    {
        if( csrIsAllSessionDisconnected( pMac ) )
        {
            if(pmcIsPowerSaveEnabled(pMac, ePMC_IDLE_MODE_POWER_SAVE))
            {
                nRet = pMac->roam.configParam.scanAgeTimeNCPS;
            }
            else
            {
                nRet = pMac->roam.configParam.scanAgeTimeNCNPS;
            }
        }
        else
        {
            if(pmcIsPowerSaveEnabled(pMac, ePMC_BEACON_MODE_POWER_SAVE))
            {
                nRet = pMac->roam.configParam.scanAgeTimeCPS;
            }
            else
            {
                nRet = pMac->roam.configParam.scanAgeTimeCNPS;
            }
        }
        //If state-change causing aging time out change, we want to delay it somewhat to avoid
        //unnecessary removal of BSS. This is mostly due to transition from connect to disconnect.
        if(pMac->scan.nLastAgeTimeOut > nRet)
        {
           if(nRet)
           {
            pMac->scan.nAgingCountDown = (pMac->scan.nLastAgeTimeOut / nRet);
           }
            pMac->scan.nLastAgeTimeOut = nRet;
            nRet *= pMac->scan.nAgingCountDown;
        }
        else
        {
            pMac->scan.nLastAgeTimeOut = nRet;
        }
    }

    return (nRet);
}


void csrScanResultAgingTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    tANI_BOOLEAN fDisconnected = csrIsAllSessionDisconnected(pMac);
    
    //no scan, no aging
    if(pMac->scan.fScanEnable && 
        (((eANI_BOOLEAN_FALSE == fDisconnected) && pMac->roam.configParam.bgScanInterval)    
        || (fDisconnected && (pMac->scan.fCancelIdleScan == eANI_BOOLEAN_FALSE)))
        )
    {
        tListElem *pEntry, *tmpEntry;
        tCsrScanResult *pResult;
        tANI_TIMESTAMP ageOutTime = (tANI_TIMESTAMP)(csrScanGetAgeOutTime(pMac) * PAL_TICKS_PER_SECOND); //turn it into 10ms units
        tANI_TIMESTAMP curTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

        csrLLLock(&pMac->scan.scanResultList);
        pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
        while( pEntry ) 
        {
            tmpEntry = csrLLNext(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK);
            pResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
            if((curTime - pResult->Result.BssDescriptor.nReceivedTime) > ageOutTime)
            {
                csrScanAgeOutBss(pMac, pResult);
            }
            pEntry = tmpEntry;
        }
        csrLLUnlock(&pMac->scan.scanResultList);
    }
}


eHalStatus csrScanStartIdleScanTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOG1, " csrScanStartIdleScanTimer \n ");
    if((pMac->scan.fScanEnable) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) && interval)
    {
        pMac->scan.nIdleScanTimeGap += interval;
        palTimerStop(pMac->hHdd, pMac->scan.hTimerIdleScan);
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerIdleScan, interval, eANI_BOOLEAN_FALSE);
        if( !HAL_STATUS_SUCCESS(status) )
        {
            smsLog(pMac, LOGE, "  Fail to start Idle scan timer. status = %d interval = %d\n", status, interval);
            //This should not happen but set the flag to restart when ready
            pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
    }
    }
    else
    {
		if( pMac->scan.fScanEnable && interval )
		{
			pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
		}
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopIdleScanTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerIdleScan));
}


//Stop CSR from asking for IMPS, This function doesn't disable IMPS from CSR
void csrScanSuspendIMPS( tpAniSirGlobal pMac )
{
    csrScanCancelIdleScan(pMac);
}


//Start CSR from asking for IMPS. This function doesn't trigger CSR to request entering IMPS
//because IMPS maybe disabled.
void csrScanResumeIMPS( tpAniSirGlobal pMac )
{
    csrScanStartIdleScan( pMac );
}


void csrScanIMPSCallback(void *callbackContext, eHalStatus status)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( callbackContext );

    if(pMac->roam.configParam.IsIdleScanEnabled)
    {
    
    if(eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) 
    {
        if(HAL_STATUS_SUCCESS(status))
        {
            if(csrIsAllSessionDisconnected(pMac) && !csrIsRoamCommandWaiting(pMac))
            {
                smsLog(pMac, LOGW, FL("starts idle mdoe full scan\n"));
                csrScanAllChannels(pMac, eCSR_SCAN_IDLE_MODE_SCAN);
            }
            else
            {
                smsLog(pMac, LOGW, FL("cannot start idle mdoe full scan\n"));
                //even though we are in timer handle, calling stop timer will make sure the timer
                //doesn't get to restart.
                csrScanStopIdleScanTimer(pMac);
            }
        }
        else
        {
            smsLog(pMac, LOGE, FL("sees not success status (%d)\n"), status);
        }
    }
    }
    else
    {//we might need another flag to check if CSR needs to request imps at all
       
       tANI_U32 nTime = 0;

       pMac->scan.fRestartIdleScan = eANI_BOOLEAN_FALSE;
       if(!HAL_STATUS_SUCCESS(csrScanTriggerIdleScan(pMac, &nTime)))
       {
          csrScanStartIdleScanTimer(pMac, nTime);
       }
    }
}


//Param: pTimeInterval -- Caller allocated memory in return, if failed, to specify the nxt time interval for 
//idle scan timer interval
//Return: Not success -- meaning it cannot start IMPS, caller needs to start a timer for idle scan
eHalStatus csrScanTriggerIdleScan(tpAniSirGlobal pMac, tANI_U32 *pTimeInterval)
{
    eHalStatus status = eHAL_STATUS_CSR_WRONG_STATE;
    *pTimeInterval = 0;

    smsLog(pMac, LOGW, FL("called\n"));
    if( smeCommandPending( pMac ) )
    {
        smsLog( pMac, LOGW, FL("  Cannot request IMPS because command pending\n") );
        //Not to enter IMPS because more work to do
        if(pTimeInterval)
        {
            *pTimeInterval = 0;
        }
        //restart when ready
        pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;

        return (status);
    }

    if((pMac->scan.fScanEnable) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan) && 
        pMac->roam.configParam.impsSleepTime)
    {
        //Stop get result timer because idle scan gets scan result out of PE
        csrScanStopGetResultTimer(pMac);
        if(pTimeInterval)
        {
            *pTimeInterval = pMac->roam.configParam.impsSleepTime;
        }
        //pmcRequestImps take a period in millisecond unit.
        status = pmcRequestImps(pMac, pMac->roam.configParam.impsSleepTime / PAL_TIMER_TO_MS_UNIT, csrScanIMPSCallback, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(eHAL_STATUS_PMC_ALREADY_IN_IMPS != status)
            {
                //Do restart the timer if CSR thinks it cannot do IMPS
                if( !csrCheckPSReady( pMac ) )
                {
                    if(pTimeInterval)
                    {
                    *pTimeInterval = 0;
                }
                    //Set the restart flag to true because that idle scan 
                    //can be restarted even though the timer will not be running
                    pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
                }
                else
                {
                    //For not now, we do a quicker retry
                    if(pTimeInterval)
                    {
                    *pTimeInterval = CSR_IDLE_SCAN_WAIT_TIME;
                }
            }
                smsLog(pMac, LOGW, FL("call pmcRequestImps and it returns status code (%d)\n"), status);
            }
            else
            {
                smsLog(pMac, LOGW, FL("already in IMPS\n"));
                //Since CSR is the only module to request for IMPS. If it is already in IMPS, CSR assumes
                //the callback will be called in the future. Should not happen though.
                status = eHAL_STATUS_SUCCESS;
                pMac->scan.nIdleScanTimeGap = 0;
            }
        }
        else
        {
            //requested so let's reset the value
            pMac->scan.nIdleScanTimeGap = 0;
        }
    }

    return (status);
}


eHalStatus csrScanStartIdleScan(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_CSR_WRONG_STATE;
    tANI_U32 nTime = 0;

    smsLog(pMac, LOGW, FL("called\n"));
    if(pMac->roam.configParam.IsIdleScanEnabled)
    {
    //stop bg scan first
    csrScanBGScanAbort(pMac);
    //Stop get result timer because idle scan gets scan result out of PE
    csrScanStopGetResultTimer(pMac);
    //Enable aging timer since idle scan is going on
    csrScanStartResultAgingTimer(pMac);
    }
    pMac->scan.fCancelIdleScan = eANI_BOOLEAN_FALSE;
    status = csrScanTriggerIdleScan(pMac, &nTime);
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrScanStartIdleScanTimer(pMac, nTime);
    }

    return (status);
}


void csrScanCancelIdleScan(tpAniSirGlobal pMac)
{
    if(eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan)
    {
        smsLog(pMac, LOG1, "  csrScanCancelIdleScan\n");
        pMac->scan.fCancelIdleScan = eANI_BOOLEAN_TRUE;
        //Set the restart flag in case later on it is uncancelled
		pMac->scan.fRestartIdleScan = eANI_BOOLEAN_TRUE;
        csrScanStopIdleScanTimer(pMac);
        csrScanRemoveNotRoamingScanCommand(pMac);
    }
}


void csrScanIdleScanTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    eHalStatus status;
    tANI_U32 nTime = 0;

    smsLog(pMac, LOGW, "  csrScanIdleScanTimerHandler called  ");
    status = csrScanTriggerIdleScan(pMac, &nTime);
    if(!HAL_STATUS_SUCCESS(status) && (eANI_BOOLEAN_FALSE == pMac->scan.fCancelIdleScan))
    {
        //Check whether it is time to actually do an idle scan
        if(pMac->scan.nIdleScanTimeGap >= pMac->roam.configParam.impsSleepTime)
        {
            pMac->scan.nIdleScanTimeGap = 0;
            csrScanIMPSCallback(pMac, eHAL_STATUS_SUCCESS);
        }
        else
        {
            csrScanStartIdleScanTimer(pMac, nTime);
        }
    }
}
#ifdef FEATURE_WLAN_GEN6_ROAMING
void csrScanRemoveBgScanReq(tpAniSirGlobal pMac)
{
    tListElem *pEntry = NULL, *pNextEntry = NULL;
	tSmeCmd *pCommand = NULL;
    tSirMacAddr BroadcastMac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    eCsrScanReason reason;
    eCsrRoamState lastRoamState;
	//remove HO related scan cmd from active list
	pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
	if ( pEntry )
	{
		pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        reason = pCommand->u.scanCmd.reason;
        lastRoamState = pCommand->u.scanCmd.lastRoamState;
		// If the head of the queue is Active and it is a scan command, change
        // the reason so when the command is finished, we don't process it.
		if ( eSmeCommandScan == pCommand->command &&
			 (eCsrScanSetBGScanParam == reason ||
			  eCsrScanBgScan == reason ||
			  eCsrScanProbeBss == reason))
		{
			//we need to process the result first before removing it from active list because state changes 
			//still happening insides roamQProcessRoamResults so no other roam command should be issued
			smsLog(pMac, LOGW, "csrScanRemoveBgScanReq: Changing the original reason code (%d) so that we can drop the rsp silently\n", reason);
            //Assign the right reason because different command needs different way to free resource
            switch (reason)
            {
            case eCsrScanSetBGScanParam:
                pCommand->u.scanCmd.reason = eCsrScanAbortBgScan;
                break;
            case eCsrScanBgScan:
            case eCsrScanProbeBss:
                pCommand->u.scanCmd.reason = eCsrScanAbortNormalScan;
                break;
            default:
                //Should not be here because new reason should be handle above by adding new case
                //treat it as normal scan
                VOS_ASSERT(0);
                pCommand->u.scanCmd.reason = eCsrScanAbortNormalScan;
                break;
            }
			pMac->roam.handoffInfo.isBgScanRspPending = FALSE;
			pMac->roam.handoffInfo.isProbeRspPending = FALSE;
		    if(eCsrScanProbeBss == reason)
			{//reset the probe bssid too
			   palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
						   BroadcastMac, WNI_CFG_BSSID_LEN);
			}
		}
	}
	//remove HO related scan cmd from pending list
	pEntry = csrLLPeekHead( &pMac->sme.smeCmdPendingList, LL_ACCESS_LOCK );
	while ( pEntry )
	{
		pNextEntry = csrLLNext( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK );
		pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        reason = pCommand->u.scanCmd.reason;

		// If the head of the queue is Active and it is a ROAM command, remove
		// and put this on the Free queue.
		if ( eSmeCommandScan == pCommand->command &&
			 (eCsrScanSetBGScanParam == reason ||
			  eCsrScanBgScan == reason ||
			  eCsrScanProbeBss == reason))
		{
			//we need to process the result first before removing it from active list because state changes 
			//still happening insides roamQProcessRoamResults so no other roam command should be issued
			smsLog(pMac, LOGW, "csrScanRemoveBgScanReq: Removing the entry from Pending list\n");
			if( csrLLRemoveEntry( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_LOCK ) )
			{
				csrReleaseCommandScan( pMac, pCommand );
			}
			else
			{
				smsLog(pMac, LOGE, "   *********csrScanRemoveBgScanReq fail to remove entry2\n");
			}
			pMac->roam.handoffInfo.isBgScanRspPending = FALSE;
			pMac->roam.handoffInfo.isProbeRspPending = FALSE;
		}
		pEntry = pNextEntry;
	}


}

eHalStatus csrScanStartBgScanTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOGW, " csrScanStartBgScanTimer: interval %d \n ", interval);

    if(( pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_JOINED ||
		 pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_SCANNING ) && interval)
    {
        status = palTimerStart(pMac->hHdd, pMac->scan.hTimerBgScan, interval * PAL_TIMER_TO_MS_UNIT, eANI_BOOLEAN_FALSE);
    }
    else
    {
        status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}


eHalStatus csrScanStopBgScanTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->scan.hTimerBgScan));
}

void csrScanBgScanTimerHandler(void *pv)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( pv );
   // need to start a new bg scan
   /* Extra check on state to see if this is really needed                   */
   if( ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC ) )
   {
      smsLog(pMac, LOGW, "csrScanBgScanTimerHandler: wrong substate %d\n", pMac->roam.handoffInfo.currSubState);
      return;
   }
   /*-------------------------------------------------------------------------
     Check if bg scan is enabled for the connected profile
   -------------------------------------------------------------------------*/
   if(FALSE == csrScanIsBgScanEnabled(pMac))
   {
      return;
   }
   /*-------------------------------------------------------------------------
     Check if scan (probe, previous bg scan) is pending: no need to issue bg 
     scan in this case! 
   -------------------------------------------------------------------------*/
   if(pMac->roam.handoffInfo.isBgScanRspPending || pMac->roam.handoffInfo.isProbeRspPending)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = TRUE;
      smsLog(pMac, LOGW, "csrScanBgScanTimerHandler: defer bg scan as bg scan/probe rsp pending\n");
      return;
   }

   smsLog(pMac, LOGW, "csrScanBgScanTimerHandler: requesting bg scan");
   /*-------------------------------------------------------------------------
     issue scan
   -------------------------------------------------------------------------*/
   csrScanSendBgScanReq(pMac);
}


tCsrScanResult * csrScanFindEntryInScanList( tpAniSirGlobal pMac, tSirMacAddr Bssid) 
{
    tListElem *pEntry;

    tCsrScanResult *pBssDesc;
    tCsrScanResult *pReturnBssDesc = NULL;

    // Walk through all the scan results.  If we find an entry that
    // matches the Bssid passed in, we will be good.
    pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

    while( pEntry ) 
    {
        pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );

        if(csrIsMacAddressEqual(pMac, 
                             (tCsrBssid *)pBssDesc->Result.BssDescriptor.bssId,
                             (tCsrBssid *) Bssid))
        {
           pReturnBssDesc = pBssDesc;
           //msg
           smsLog(pMac, LOGW, "csrScanFindEntryInScanList: Found entry\n");
           break;
        }

        pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
    }

    return pReturnBssDesc;
}


void csrScanHoScanSuccess(tpAniSirGlobal pMac)
{

   tSirMacAddr BroadcastMac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
   tCsrScanResult *pBssDesc;
   tCsrBssid       bssid;
   tANI_U16        rssi;
   tANI_U32        scan_time = 0;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
   v_S7_t          currApRssi;
   VOS_STATUS      status;
   WLAN_VOS_DIAG_EVENT_DEF(handoff, vos_event_wlan_handoff_payload_type);
#endif //FEATURE_WLAN_DIAG_SUPPORT_CSR
/* Clear the bg scan rsp pending flag                                  */
   pMac->roam.handoffInfo.isBgScanRspPending = eANI_BOOLEAN_FALSE;

   smsLog(pMac, LOGW, "csrScanHoScanSuccess: invoked\n");
   /*-------------------------------------------------------------------------
     If this scan response is to be ignored, should be due to pending scan 
     from previous HO sub-state
   -------------------------------------------------------------------------*/
   if(eANI_BOOLEAN_FALSE == pMac->roam.handoffInfo.ignoreScanFrmOthrState)
   {
      /* Update the neighbor & candidate lists                               */
      csrScanUpdateHoLists(pMac);
         //log: LOG_WLAN_HANDOFF_C
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
      csrScanDiagHoLog(pMac);
#endif //FEATURE_WLAN_DIAG_SUPPORT_CSR
   }
   else
   {
      pMac->roam.handoffInfo.ignoreScanFrmOthrState = eANI_BOOLEAN_FALSE;
   }

   /*-------------------------------------------------------------------------
     If this is a probe_bssid response, then check if probed AP is still good.
     If so then trigger handoff
     Otherwise, probe the next entry, make sure to check PER, TL indications
     are only based of RSSI...
   -------------------------------------------------------------------------*/
   if( !csrIsMacAddressBroadcast(pMac, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid ))
   {
      pMac->roam.handoffInfo.isProbeRspPending = FALSE;
      pBssDesc = 
         csrScanFindEntryInScanList(pMac, pMac->roam.handoffInfo.handoffActivityInfo.probedBssid);

      if(!pBssDesc)
      {
         //msg
         smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate: probe failed for Bssid= %02x-%02x-%02x-%02x-%02x-%02x\n", 
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 0 ], 
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 1 ], 
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 2 ],
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 3 ], 
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 4 ], 
                pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 5 ] );

         /* Reset probed BSSID field                                            */
         palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                       BroadcastMac, WNI_CFG_BSSID_LEN);

      }
      else
      {
      
         palCopyMemory(pMac->hHdd, &bssid, 
                       &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                       WNI_CFG_BSSID_LEN);

         /* Reset probed BSSID field                                            */
         palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                       BroadcastMac, WNI_CFG_BSSID_LEN);

         /* RSSI still good? */
         if(pBssDesc->Result.BssDescriptor.rssi < 0)
         {
            rssi = pBssDesc->Result.BssDescriptor.rssi * (-1);
         }
         else
         {
            rssi = pBssDesc->Result.BssDescriptor.rssi;
         }
         if( rssi < pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet )
         {

            smsLog(pMac, LOGW, "csrScanHoScanSuccess: Starting Handoff process");
            smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate : Current STA info:\n");
            smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate: BSSID= %02x-%02x-%02x-%02x-%02x-%02x\n", 
                   pMac->roam.handoffInfo.currSta.bssid[ 0 ], 
                   pMac->roam.handoffInfo.currSta.bssid[ 1 ], 
                   pMac->roam.handoffInfo.currSta.bssid[ 2 ],
                   pMac->roam.handoffInfo.currSta.bssid[ 3 ], 
                   pMac->roam.handoffInfo.currSta.bssid[ 4 ], 
                   pMac->roam.handoffInfo.currSta.bssid[ 5 ] );

            smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate : channel = %d\n", pMac->roam.handoffInfo.currSta.pBssDesc->channelId);
            smsLog(pMac, LOGW, " Tx PER = %d, Rx PER = %d\n", 
                   pMac->roam.handoffInfo.currSta.txPer,
                   pMac->roam.handoffInfo.currSta.rxPer);
            
            smsLog(pMac, LOGW, "csrScanHoScanSuccess:HO STA's RSSI = %d", rssi );

            csrRoamCreateHandoffProfile(pMac, bssid);
            csrScanAbortMacScan(pMac);
            csrRoamHandoffRequested(pMac);
            //event: EVENT_WLAN_HANDOFF
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR          
            handoff.eventId = eCSR_WLAN_HANDOFF_EVENT;
            palCopyMemory(pMac->hHdd, handoff.currentApBssid, 
                          pMac->roam.handoffInfo.currSta.bssid , 6);
            status = WLANTL_GetRssi(pMac->roam.gVosContext, pMac->roam.connectedInfo.staId,
                                    &currApRssi);

            if(status)
            {
               //err msg
               smsLog(pMac, LOGW, " csrScanHoScanSuccess: couldn't get the current APs RSSi from TL\n");
               currApRssi = 0;
            }

            handoff.currentApRssi = currApRssi * (-1);
            palCopyMemory(pMac->hHdd, handoff.candidateApBssid, 
                          pBssDesc->Result.BssDescriptor.bssId , 6);
            handoff.candidateApRssi = (tANI_U8)rssi;
            WLAN_VOS_DIAG_EVENT_REPORT(&handoff, EVENT_WLAN_HANDOFF);
#endif //FEATURE_WLAN_DIAG_SUPPORT_CSR
            return;
         }
         else
         {
            /* Fall through to check further for handoff possibilities           */
         }
      }
   }

   /*------------------------------------------------------------------------
    Set the rssi threashold of the best candidate in TL & wait for trigger 
   from TL.
    Based on TL trigger will initiate a probe_bssid that time
   -----------------------------------------------------------------------*/
   if( HAL_STATUS_SUCCESS(csrScanGetScanHoCandidate(pMac))   )
   {
      //msg  
      smsLog(pMac, LOG1, "csrScanHoScanSuccess: set the RSSI threshold in TL but not probing now\n");
   }
   else
   {
   
	   /*--------------------------------------------------------------------------
		 The following stats based check is necessary in case we previously deferred
		 a handoff decision due to STATS timer when scan response was pending
		   
		 Check if handoff is necessary.
		 If handoff is required, initiate a probe_bssid and return
     TODO
		-------------------------------------------------------------------------*/
   }
   
   /*-------------------------------------------------------------------------
       Check if bg scan needs to be started for this state
   -------------------------------------------------------------------------*/
   if( FALSE == csrScanIsBgScanEnabled(pMac) )
   {
	  smsLog(pMac, LOGW, "csrScanHoScanSuccess: bg scan disabled\n");
      return;
   }
   
   /*-------------------------------------------------------------------------
       If bg scan required, then start the scan timer (stats timer should
       already be running)
       Call scan timer generator, a 0 indicates timer already running.
       if the bg scan timer is already running no need to start again.
       If we defered a previous bg scan req, send the request right away
   -------------------------------------------------------------------------*/
   scan_time = csrScanGetBgScanTimerVal(pMac);

   smsLog(pMac, LOGW, "csrScanHoScanSuccess: scan_time %d\n", scan_time);
   if(!scan_time )
   {
      return;
   }

   if (pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready &&
       !pMac->roam.handoffInfo.isProbeRspPending) 
   {
      //bg scan req
      csrScanSendBgScanReq(pMac);
   }

   //start the bg scan timer
   csrScanStartBgScanTimer(pMac, scan_time);
   
   pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;
   
   return;
}     

void csrScanHoScanFailure(tpAniSirGlobal pMac)
{
   tSirMacAddr BroadcastMac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
   tANI_U8 chanCount;
   tANI_U8 channel;
   tANI_U32        scan_time = 0;

   smsLog(pMac, LOGW, "csrScanHoScanFailure: invoked\n");
   /* Clear the bg scan rsp pending flag                                  */
   pMac->roam.handoffInfo.isBgScanRspPending = eANI_BOOLEAN_FALSE;
   /*-------------------------------------------------------------------------
     If this scan response is to be ignored, should be due to pending scan 
     from previous HO sub-state
   -------------------------------------------------------------------------*/
   if(eANI_BOOLEAN_FALSE == pMac->roam.handoffInfo.ignoreScanFrmOthrState)
   {
      /* Update the neighbor & candidate lists by removing entries on the channel
         just scanned                                                          */
      chanCount = 
         pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels;

      while(chanCount)
      {
         channel = pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList
            [pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels - chanCount]; 
         csrScanTrimHoListForChannel( pMac,
                                      &pMac->roam.handoffInfo.neighborList, 
                                      channel );
         csrScanTrimHoListForChannel( pMac,
                                      &pMac->roam.handoffInfo.candidateList, 
                                      channel );
         chanCount--;
      }
   }
   else
   {
      pMac->roam.handoffInfo.ignoreScanFrmOthrState = eANI_BOOLEAN_FALSE;
   }


   /*-------------------------------------------------------------------------
     If this is a probe_bssid response, remove the entry from candidate list
   -------------------------------------------------------------------------*/
   if( !csrIsMacAddressBroadcast(pMac, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid ))
   {
      pMac->roam.handoffInfo.isProbeRspPending = FALSE;
      //remove the entry from candidate list??
      csrScanRemoveEntryFromList( pMac,
                                  &pMac->roam.handoffInfo.candidateList, 
                                  pMac->roam.handoffInfo.handoffActivityInfo.probedBssid);
      
      /* Reset probed BSSID field                                            */
      palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                    BroadcastMac, WNI_CFG_BSSID_LEN);

   }
   /*-------------------------------------------------------------------------
     If bg scan required, then start the scan timer (stats timer should
     already be running)
     Call scan timer generator, a 0 indicates timer already running.
     if the bg scan timer is already running no need to start again.
     If we defered a previous bg scan req, send the request right away
   -------------------------------------------------------------------------*/
   scan_time = csrScanGetBgScanTimerVal(pMac);

   smsLog(pMac, LOGW, "csrScanHoScanFailure: scan_time %d\n", scan_time);
   if(!scan_time )
   {
      return;
   }
   if (pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready) 
   {
      //bg scan req
      csrScanSendBgScanReq(pMac);
   }

   //start the bg scan timer
   csrScanStartBgScanTimer(pMac, scan_time);

   pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;

   return;


}

void csrScanUpdateNList(tpAniSirGlobal pMac)
{
   tListElem *pEntry = NULL;

   tCsrScanResult *pBssDesc = NULL;

   tCsrHandoffStaInfo staEntry;

   tSirBssDescription *pTempBssDesc = NULL;
   tANI_U8 chanCount;
   tANI_U8 channel;
   tANI_U8 updated_rssi;
   tANI_U16 bssLen;
   eHalStatus status;
   tDot11fBeaconIEs *pIes = NULL;
   smsLog(pMac, LOGW, "csrScanUpdateNList: \n");

   palZeroMemory(pMac->hHdd, &staEntry, sizeof(tCsrHandoffStaInfo));
   /*-------------------------------------------------------------------------
     Before updating the lists, we first remove all entries from lists
     for which we have not received a response
   -------------------------------------------------------------------------*/
   chanCount = 
      pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels;

   while(chanCount)
   {
      channel = pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList
         [pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels - chanCount]; 
      csrScanTrimHoListForChannel( pMac,
                                   &pMac->roam.handoffInfo.neighborList, 
                                   channel );
      chanCount--;
   }

   /*-------------------------------------------------------------------------
     To Update STA Lists:

     - check which list, entry needs to be added to: candidate needs threshold
     - if entry should be in candidate_list
         > if entry exists update entry
         else add entry
     - if entry should be in neighbor_list
         > If entry exists update entry
         else add entry to neighbor_list

     - If when adding an entry to candidate_list an entry was returned, this
     needs to go into neighbor list since candidate list size is limited
   -------------------------------------------------------------------------*/
   // Walk through all the scan results.  
   pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

   while( pEntry ) 
   {
      pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
      pIes = (tDot11fBeaconIEs *)( pBssDesc->Result.pvIes );    //this can be NULL
      /* Compute score first and put it in the neighbor list            */

	  /* new entry               */
	  if(pBssDesc->Result.BssDescriptor.rssi < 0)
	   {
		   updated_rssi = pBssDesc->Result.BssDescriptor.rssi * (-1);
	   }
	   else
	   {
		   updated_rssi = pBssDesc->Result.BssDescriptor.rssi;
	   }

	  smsLog(pMac, LOGW, "csrScanUpdateNList: add an entry in neighborList\n");

      /* get the new BSS descriptor in the entry                     */
      bssLen = pBssDesc->Result.BssDescriptor.length + 
         sizeof(pBssDesc->Result.BssDescriptor.length);

      //save the bss Descriptor
      status = palAllocateMemory(pMac->hHdd, (void **)&pTempBssDesc, bssLen);
      if (!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrScanUpdateNList: couldn't allocate memory for the \
                bss Descriptor\n");
         return;
      }

      palZeroMemory(pMac->hHdd, pTempBssDesc, bssLen);
      palCopyMemory(pMac->hHdd, pTempBssDesc, &pBssDesc->Result.BssDescriptor, bssLen);
      staEntry.sta.pBssDesc = pTempBssDesc;

	  palCopyMemory(pMac->hHdd, &staEntry.sta.bssid, 
					&pBssDesc->Result.BssDescriptor.bssId, sizeof(tCsrBssid));

      // calcualate the rssi, qos & sec scores
      
     /* Dump the new scores into the existing entry and update record     */
      staEntry.sta.rssiScore = updated_rssi;
      
      staEntry.sta.qosScore = csrScanGetQosScore(pMac, staEntry.sta.pBssDesc, pIes);
      
     
      staEntry.sta.secScore = csrScanGetSecurityScore(pMac, staEntry.sta.pBssDesc, pIes);

      smsLog(pMac, LOGW, "csrScanUpdateNList: update the neighborList\n");	
      staEntry.sta.overallScore = CSR_SCAN_OVERALL_SCORE(staEntry.sta.rssiScore);
	  /* Check and either update an existing record or push the new one in */
	  csrScanUpdateHoNeighborList(pMac,&staEntry);
      if(pTempBssDesc)
      {
         palFreeMemory(pMac->hHdd, pTempBssDesc);
      }

      pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
   }/* while loop for scan result list */
   

   smsLog(pMac, LOGW, "csrScanUpdateNList: done updating Neighbor STA list after scan success\n");

}

void csrScanUpdateHoLists(tpAniSirGlobal pMac)
{
   tListElem *pEntry = NULL;

   tCsrScanResult *pBssDesc = NULL;

   tCsrHandoffStaInfo *pStaEntry = NULL;

   tSirBssDescription *pTempBssDesc = NULL;
   tANI_U8 chanCount;
   tANI_U8 channel;
   int query_status;
   tANI_U8 updated_rssi;
   tANI_BOOLEAN neighbor_or_candidate;
   tANI_BOOLEAN popped = FALSE;
   tANI_U16 bssLen;
   eHalStatus status;
   tANI_U8 scan_rssi;	
   tDot11fBeaconIEs *pIes = NULL;
   tANI_BOOLEAN newEntry = FALSE;

   smsLog(pMac, LOGW, "csrScanUpdateHoLists: \n");
   /*-------------------------------------------------------------------------
     Before updating the lists, we first remove all entries from lists
     for which we have not received a response
   -------------------------------------------------------------------------*/
   chanCount = 
      pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels;

   while(chanCount)
   {
      channel = pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList
         [pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels - chanCount]; 
      csrScanTrimHoListForChannel( pMac,
                                   &pMac->roam.handoffInfo.neighborList, 
                                   channel );
      csrScanTrimHoListForChannel( pMac,
                                   &pMac->roam.handoffInfo.candidateList, 
                                   channel );
      chanCount--;
   }

   /*-------------------------------------------------------------------------
     To Update STA Lists:

     - check which list, entry needs to be added to: candidate needs threshold
     - if entry should be in candidate_list
         > if entry exists update entry
         else add entry
     - if entry should be in neighbor_list
         > If entry exists update entry
         else add entry to neighbor_list

     - If when adding an entry to candidate_list an entry was returned, this
     needs to go into neighbor list since candidate list size is limited
   -------------------------------------------------------------------------*/
   // Walk through all the scan results.  
   pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

   while( pEntry ) 
   {
      pBssDesc = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
      pIes = (tDot11fBeaconIEs *)( pBssDesc->Result.pvIes );
      if(csrIsMacAddressEqual(pMac, 
                              (tCsrBssid *)pBssDesc->Result.BssDescriptor.bssId,
                              (tCsrBssid *) pMac->roam.handoffInfo.currSta.bssid))
      {
         /* Do not add current AP to the lists                 */

         //msg
		 smsLog(pMac, LOGW, "csrScanUpdateHoLists: no need to add current AP to the lists\n");
		 pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
         continue;
      }

      if( !pIes && !HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, &pBssDesc->Result.BssDescriptor, &pIes)) )
      {
         smsLog(pMac, LOGE, FL("  fail to parse IEs\n"));
         pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
         continue;
      }
#if 0
      /* csr scan result can have multiple ssid, only look for our one*/
      if(pIes->SSID.present)
      {
		 /* TODO : Scan do not have session info.
		    Connected profile is a per session information
			We can not perform this during scan.
		 */
         if(!csrIsSsidMatch( pMac, 
                             pMac->roam.connectedProfile.SSID.ssId, 
                             pMac->roam.connectedProfile.SSID.length,
                             pIes->SSID.ssid, pIes->SSID.num_ssid,
                             eANI_BOOLEAN_TRUE ))
         {
            smsLog(pMac, LOGW, "csrScanUpdateHoLists: no need to add, SSID doesn't match \n");
            if( !pBssDesc->Result.pvIes )
            {
               palFreeMemory(pMac->hHdd, pIes);
            }
            pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
            continue;
   
         }
      }
      else
#endif
      {
         smsLog(pMac, LOGW, "csrScanUpdateHoLists: no need to add, SSID doesn't match \n");
         if( !pBssDesc->Result.pvIes )
         {
            palFreeMemory(pMac->hHdd, pIes);
         }
         pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
         continue;
      }

      /* Compute score first and determine which list this is going           */

      query_status = csrScanFindBssEntryFromList( pMac,
                                                  &pMac->roam.handoffInfo.neighborList,
                                                  pBssDesc->Result.BssDescriptor.bssId,
                                                  &pStaEntry);

      if( 0 == query_status )
      {
		  if(pBssDesc->Result.BssDescriptor.rssi < 0)
		  {
			  scan_rssi = pBssDesc->Result.BssDescriptor.rssi * (-1);
		  }
		  else
		  {
			  scan_rssi = pBssDesc->Result.BssDescriptor.rssi;
		  }
		 

         /* BSSID exists - but no need to update the RSSI               */
         updated_rssi = scan_rssi;

         smsLog(pMac, LOGW, "csrScanUpdateHoLists: found an entry in neighborList\n");
         neighbor_or_candidate = FALSE;
      }
      else
      {
         /* Check if it is in candidate_list and compute RSSI                 */
         query_status = csrScanFindBssEntryFromList( pMac,
                                                     &pMac->roam.handoffInfo.candidateList,
                                                     pBssDesc->Result.BssDescriptor.bssId,
                                                     &pStaEntry);

         if( 0 == query_status )
         {
            if(pBssDesc->Result.BssDescriptor.rssi < 0)
            {
               scan_rssi = pBssDesc->Result.BssDescriptor.rssi * (-1);
            }
            else
            {
               scan_rssi = pBssDesc->Result.BssDescriptor.rssi;
            }

            /* BSSID exists - but no need to update the RSSI            */
            updated_rssi = scan_rssi;         
            smsLog(pMac, LOGW, "csrScanUpdateHoLists: found an entry in candidateList\n");
            neighbor_or_candidate = TRUE;
         }
         else
         {
            /* New entry */
		    smsLog(pMac, LOGW, "csrScanUpdateHoLists: new entry\n");
			if(pBssDesc->Result.BssDescriptor.rssi < 0)
		    {
			   updated_rssi = pBssDesc->Result.BssDescriptor.rssi * (-1);
			}
			else
			{
			   updated_rssi = pBssDesc->Result.BssDescriptor.rssi;
			}


            status = palAllocateMemory(pMac->hHdd, (void **)&pStaEntry, sizeof(tCsrHandoffStaInfo));
            if (!HAL_STATUS_SUCCESS(status))
            {
               smsLog(pMac, LOGW, "csrScanUpdateHoLists: couldn't allocate memory for the \
	                  bss Descriptor\n");
               if( !pBssDesc->Result.pvIes )
               {
                  palFreeMemory(pMac->hHdd, pIes);
               }
               return;
            }
            else
            {
               newEntry = TRUE;
            }

            palZeroMemory(pMac->hHdd, pStaEntry, sizeof(tCsrHandoffStaInfo));

            neighbor_or_candidate = FALSE;
         }
      }

      /* get the new BSS descriptor in the entry                     */
      bssLen = pBssDesc->Result.BssDescriptor.length + 
         sizeof(pBssDesc->Result.BssDescriptor.length);

      //save the bss Descriptor
      status = palAllocateMemory(pMac->hHdd, (void **)&pTempBssDesc, bssLen);
      if (!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrScanUpdateHoLists: couldn't allocate memory for the \
                bss Descriptor\n");
         if( !pBssDesc->Result.pvIes )
         {
            palFreeMemory(pMac->hHdd, pIes);
         }
         return;
      }

      palZeroMemory(pMac->hHdd, pTempBssDesc, bssLen);
      palCopyMemory(pMac->hHdd, pTempBssDesc, &pBssDesc->Result.BssDescriptor, bssLen);
      if(pStaEntry->sta.pBssDesc)
      {
         palFreeMemory(pMac->hHdd, pStaEntry->sta.pBssDesc);
      }
      pStaEntry->sta.pBssDesc = pTempBssDesc;

	  palCopyMemory(pMac->hHdd, &pStaEntry->sta.bssid, 
					&pBssDesc->Result.BssDescriptor.bssId, sizeof(tCsrBssid));

      // calcualate the rssi, qos & sec scores
      
     /* Dump the new scores into the existing entry and update record     */
      pStaEntry->sta.rssiScore = updated_rssi;
      
      pStaEntry->sta.qosScore = csrScanGetQosScore(pMac, pStaEntry->sta.pBssDesc, pIes);
      
     
      pStaEntry->sta.secScore = csrScanGetSecurityScore(pMac, pStaEntry->sta.pBssDesc, pIes);

      /* Overall score is RSSI score except where QoS/SEC forbidden          */
      /* Also note: we only use the first iteration's RSSI threshold         */
      if( (0 == pStaEntry->sta.qosScore) ||
          (0 == pStaEntry->sta.secScore) )
      {
         pStaEntry->sta.overallScore = 0;
      }
      else
      {
         pStaEntry->sta.overallScore = CSR_SCAN_OVERALL_SCORE(pStaEntry->sta.rssiScore);
      }



      /* Check if entry goes into neighbor list or candidate list            */
      if((pStaEntry->sta.overallScore != 0) &&
         (updated_rssi < pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet))
      {
         tCsrHandoffStaInfo  *pPoppedEntry;

         popped = csrScanUpdateHoCandidateList( pMac,
												pStaEntry,
                                                &pPoppedEntry );
		 if( TRUE == popped)
		 {
            /* An entry has popped out: need to put it in neighbor list       */
            smsLog(pMac, LOGW, "csrScanUpdateHoLists: An entry has popped out: need to put it in neighbor list\n");
            csrScanUpdateHoNeighborList( pMac, pPoppedEntry);
         }
         
         
         if( ( 0 == query_status ) &&
             (FALSE == neighbor_or_candidate))
         {
            /* It could me my new entry */
			if(popped && csrIsMacAddressEqual(pMac, 
											  &pStaEntry->sta.bssid,
											  &pPoppedEntry->sta.bssid) )
			{
                smsLog(pMac, LOGW, "csrScanUpdateHoLists: Not deleting the entry from neighbor list, just got popped\n");
			}
			else
			{
			
				/* Entry was found in neighbor list but belongs to candidate list */
				smsLog(pMac, LOGW, "csrScanUpdateHoLists: Entry was found in neighbor list but belongs to candidate list\n");
				csrScanRemoveEntryFromList( pMac,
											&pMac->roam.handoffInfo.neighborList, 
											pStaEntry->sta.bssid);
			}
         }
      }
      else
      {
         /* Entry goes into neighbor list and possibly out of candidate list  */
         smsLog(pMac, LOGW, "csrScanUpdateHoLists: Entry goes into neighbor list and possibly out of candidate list\n");
         /* Check and either update an existing record or push the new one in */
         csrScanUpdateHoNeighborList(pMac,pStaEntry);

         if( (TRUE == neighbor_or_candidate) &&
             (0 == query_status) )
         {
            /* Need to remove from the candidate_list                        */
            csrScanRemoveEntryFromList( pMac,
                                        &pMac->roam.handoffInfo.candidateList, 
                                        pStaEntry->sta.bssid);
         }
      }
      pEntry = csrLLNext( &pMac->scan.scanResultList, pEntry, LL_ACCESS_LOCK );
	  popped = FALSE;
    if(pStaEntry && newEntry)
    {
       palFreeMemory( pMac->hHdd, pStaEntry);
       newEntry = FALSE;
    }
      if( !pBssDesc->Result.pvIes )
      {
         palFreeMemory(pMac->hHdd, pIes);
      }

   }/* while loop for scan result list */
   
   smsLog(pMac, LOGW, "csrScanUpdateHoLists: Neighbor List:\n");
   csrScanDisplayList(pMac, &pMac->roam.handoffInfo.neighborList);
   smsLog(pMac, LOGW, "csrScanUpdateHoLists: Candidate List:\n");
   csrScanDisplayList(pMac, &pMac->roam.handoffInfo.candidateList);
   smsLog(pMac, LOGW, "csrScanUpdateHoLists: done updating candidate & Neighbor STA list after scan success\n");


}

void csrScanTrimHoListForChannel(tpAniSirGlobal pMac, tDblLinkList *pStaList, tANI_U8 channel)
{
   tListElem *pEntry;
   tListElem *pTempEntry;

   tCsrScanResult *pScanEntry;

   tCsrHandoffStaInfo *pstaEntry;

   tANI_BOOLEAN  remove_entry = eANI_BOOLEAN_FALSE;

   if( 0 == channel )
   {
      smsLog(pMac, LOGW, "csrScanTrimHoListForChannel: No current channel: ignoring trim request\n");
      return;
   }

   /* Take out the non-matching entries from STA list on the scanned channel  */

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanTrimHoListForChannel: list empty\n");
      return;
   }

   while( pEntry ) 
   {
      pstaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      if(channel == pstaEntry->sta.pBssDesc->channelId)
      {
         pTempEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_LOCK );

         remove_entry = eANI_BOOLEAN_TRUE;

         while( pTempEntry )
         {
            pScanEntry = GET_BASE_ADDR( pTempEntry, tCsrScanResult, Link );

            if(csrIsMacAddressEqual(pMac, 
                                    (tCsrBssid *)pScanEntry->Result.BssDescriptor.bssId,
                                    (tCsrBssid *) pstaEntry->sta.bssid))
            {
               remove_entry = eANI_BOOLEAN_FALSE;
               //msg
               smsLog(pMac, LOGW, "csrScanTrimHoListForChannel: match found\n");
               break;
            }

            pTempEntry = csrLLNext( &pMac->scan.scanResultList, pTempEntry, LL_ACCESS_LOCK );
         }
      }

      /* If BSSID was not found in scan results: remove from list            */
      if( TRUE == remove_entry )
      {
         smsLog(pMac, LOGW, "csrScanTrimHoListForChannel: Removing entry from list on channel: %d", channel);
         csrScanRemoveEntryFromList( pMac,
                                     pStaList, 
                                     pstaEntry->sta.bssid);
         remove_entry = eANI_BOOLEAN_FALSE;
      }
      pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
   }

}


tANI_BOOLEAN csrScanUpdateHoCandidateList(tpAniSirGlobal pMac,
                                          tCsrHandoffStaInfo *pStaEntry, 
                                          tCsrHandoffStaInfo **ppPoppedEntry)
{
   tCsrHandoffStaInfo *pTempStaEntry = NULL;
   /* This function adds an entry to the candidate list and returns an entry if
   one has to be popped out because the list is full. List size is limited  */

   /* Check if the BSSID exists; otherwise push the entry                   */
   if( 0 != csrScanFindBssEntryFromList(pMac, &pMac->roam.handoffInfo.candidateList,
                                        pStaEntry->sta.bssid,
                                        &pTempStaEntry))
   {
      csrScanInsertEntryIntoList( pMac, &pMac->roam.handoffInfo.candidateList,
                                  pStaEntry);

      /* If number of entries in the list is more than allowed, pop one!     */
      if( pMac->roam.handoffInfo.handoffActivityInfo.currPermittedNumCandtSetEntry <
          csrScanListSize( pMac, &pMac->roam.handoffInfo.candidateList ) )
      {
         
         csrScanListRemoveTail( pMac, &pMac->roam.handoffInfo.candidateList, 
                                ppPoppedEntry );
         return TRUE;
      }
   }
   else
   {
      csrScanListUpdateBssEntry( pMac, &pMac->roam.handoffInfo.candidateList,
                                 pStaEntry);
   }

   return FALSE;

}

void csrScanUpdateHoNeighborList( tpAniSirGlobal pMac,
                                  tCsrHandoffStaInfo *pStaEntry)
{
   tCsrHandoffStaInfo *pTempStaEntry = NULL;
   /* Check if the BSSID exists; otherwise push the entry                   */
   if( 0 != csrScanFindBssEntryFromList(pMac, &pMac->roam.handoffInfo.neighborList,
                                        pStaEntry->sta.bssid,
                                        &pTempStaEntry))
   {
      csrScanInsertEntryIntoList( pMac, &pMac->roam.handoffInfo.neighborList,
                                  pStaEntry);

   }
   else
   {
      csrScanListUpdateBssEntry( pMac, &pMac->roam.handoffInfo.neighborList,
                                 pStaEntry);
   }

}

void csrScanDisplayList(tpAniSirGlobal pMac,
                        tDblLinkList *pStaList)
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanDisplayList: List empty\n");
      return;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      smsLog( pMac, LOGW, "Bssid= %02x-%02x-%02x-%02x-%02x-%02x chan= %d, rssi = %d\n",
                    pTempStaEntry->sta.pBssDesc->bssId[ 0 ], pTempStaEntry->sta.pBssDesc->bssId[ 1 ],
                    pTempStaEntry->sta.pBssDesc->bssId[ 2 ], pTempStaEntry->sta.pBssDesc->bssId[ 3 ],
                    pTempStaEntry->sta.pBssDesc->bssId[ 4 ], pTempStaEntry->sta.pBssDesc->bssId[ 5 ],
                    pTempStaEntry->sta.pBssDesc->channelId, pTempStaEntry->sta.rssiScore );


      pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
   }

}


void csrScanInsertEntryIntoList( tpAniSirGlobal pMac,
                                 tDblLinkList *pStaList,
                                 tCsrHandoffStaInfo *pStaEntry)
{
   tListElem *pEntry = NULL;
   tCsrHandoffStaInfo *pTempStaEntry;

   tCsrHandoffStaInfo *pNewStaEntry = NULL;
   eHalStatus  status;
   tANI_U16 bssLen;
   tSirBssDescription *pTempBssDesc = NULL;
   status = palAllocateMemory(pMac->hHdd, (void **)&pNewStaEntry, sizeof(tCsrHandoffStaInfo));
   if (!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOGW, "csrScanInsertEntryIntoList: couldn't allocate memory for the \
             entry\n");
      return;
   }

   palCopyMemory(pMac->hHdd, &pNewStaEntry->sta, &pStaEntry->sta, sizeof(tCsrRoamHandoffStaEntry));
   
   bssLen = pStaEntry->sta.pBssDesc->length + 
      sizeof(pStaEntry->sta.pBssDesc->length);

   //save the bss Descriptor
   status = palAllocateMemory(pMac->hHdd, (void **)&pTempBssDesc, bssLen);
   if (!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOGW, "csrScanInsertEntryIntoList: couldn't allocate memory for the \
             bss Descriptor\n");
      palFreeMemory(pMac->hHdd, pNewStaEntry);
      return;
   }

   palZeroMemory(pMac->hHdd, pTempBssDesc, bssLen);
   palCopyMemory(pMac->hHdd, pTempBssDesc, pStaEntry->sta.pBssDesc, bssLen);
   pNewStaEntry->sta.pBssDesc = pTempBssDesc;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanInsertEntryIntoList: List empty, adding first\n");
      csrLLInsertTail( pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK  );
      return;
   }


   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      if(pStaEntry->sta.overallScore > pTempStaEntry->sta.overallScore)
      {
         //msg
         smsLog(pMac, LOGW, "csrScanInsertEntryIntoList: inserting\n");
         break;
      }

      pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
   }

   if(pEntry)
   {
      csrLLInsertEntry( pStaList, pEntry, &pNewStaEntry->link, LL_ACCESS_LOCK );
   }
   else
   {
      csrLLInsertTail( pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK  );
   }
   
}

void  csrScanRemoveEntryFromList( tpAniSirGlobal pMac,
                                  tDblLinkList *pStaList,
                                  tCsrBssid    bssid)
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanRemoveEntryFromList: List empty\n");
      return;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      if(csrIsMacAddressEqual(pMac, 
                              (tCsrBssid *)bssid,
                              (tCsrBssid *) pTempStaEntry->sta.bssid))
      {
         //msg
         smsLog(pMac, LOGW, "csrScanRemoveEntryFromList: match found, removing entry\n");
         if(csrLLRemoveEntry( pStaList, pEntry, LL_ACCESS_LOCK ))
         {
            //make sure to clean up memory
            if(pTempStaEntry->sta.pBssDesc)
            {
                palFreeMemory( pMac->hHdd, pTempStaEntry->sta.pBssDesc );
            }

            palFreeMemory(pMac->hHdd, pTempStaEntry);
         }
         else
         {
            smsLog( pMac, LOGW, FL( "csrScanRemoveEntryFromList: fail to remove entry\n" ) );
         }
         break;
      }

      pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
   }

}

tANI_U8 csrScanListSize( tpAniSirGlobal pMac,
                         tDblLinkList *pStaList )
{
   tListElem *pEntry;
   tANI_U8 size = 0;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanListSize: List empty\n");
      return 0;
   }

   size = (tANI_U8)csrLLCount(pStaList);

   smsLog(pMac, LOGW, "csrScanListSize: List size %d\n", size);
   return size;
}

void csrScanListRemoveTail( tpAniSirGlobal pMac,
                            tDblLinkList *pStaList, 
                            tCsrHandoffStaInfo **ppStaEntry )
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry;

   pEntry = csrLLRemoveTail( pStaList, LL_ACCESS_NOLOCK );
   if(!pEntry || !csrLLCount(pStaList))
   {
      //msg
      smsLog(pMac, LOGW, "csrScanListRemoveTail: List empty\n");
      return;
   }

   pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

   *ppStaEntry = pTempStaEntry;
   //palCopyMemory(pMac->hHdd, &pStaEntry->sta, &pTempStaEntry->sta, sizeof(tCsrRoamHandoffStaEntry));

   return;
}

void csrScanListUpdateBssEntry( tpAniSirGlobal pMac,
                            tDblLinkList *pStaList, 
                            tCsrHandoffStaInfo *pStaEntry )
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanListUpdateBssEntry: List empty\n");
      return;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      if(csrIsMacAddressEqual(pMac, 
                              (tCsrBssid *)pStaEntry->sta.bssid,
                              (tCsrBssid *) pTempStaEntry->sta.bssid))
      {
         //msg
         smsLog(pMac, LOGW, "csrScanListUpdateBssEntry: match found, updating entry\n");
         if(csrLLRemoveEntry(pStaList, pEntry, LL_ACCESS_LOCK))
         {
            //need to clean up memory
            csrScanInsertEntryIntoList( pMac, pStaList, pStaEntry);
            if( pTempStaEntry->sta.pBssDesc )
            {
               palFreeMemory( pMac->hHdd, pTempStaEntry->sta.pBssDesc );
            }
            palFreeMemory( pMac->hHdd, pTempStaEntry );
         }
         else
         {
            smsLog(pMac, LOGW, "csrScanListUpdateBssEntry: failed to remove entry\n");
         }
         break;
      }

      pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
   }

}


int csrScanFindBssEntryFromList( tpAniSirGlobal pMac,
                                 tDblLinkList *pStaList,
                                 tCsrBssid    bssid,
                                 tCsrHandoffStaInfo **ppStaEntry)
{
   tListElem *pEntry = NULL, *pNextEntry = NULL;
   tCsrHandoffStaInfo *pTempStaEntry;
   int rc = -1;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );


   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanFindBssEntryFromList: List empty\n");
      return -1;
   }

   while( pEntry )
   {
	   pNextEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

      if(csrIsMacAddressEqual(pMac, 
                              (tCsrBssid *)bssid,
                              (tCsrBssid *) pTempStaEntry->sta.bssid))
      {
         rc = 0;
         *ppStaEntry = pTempStaEntry;
         //msg
         smsLog(pMac, LOGW, "csrScanFindBssEntryFromList: match found\n");
         break;
      }

      pEntry = pNextEntry;
   }

   return rc;
}

tCsrHandoffStaInfo * csrScanGetFirstBssEntryFromList( tpAniSirGlobal pMac, 
                                                      tDblLinkList *pStaList)
{
   tListElem *pEntry;
   tCsrHandoffStaInfo *pTempStaEntry = NULL;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry || !csrLLCount(pStaList))
   {
      //list empty
      smsLog(pMac, LOGW, "csrScanGetFirstBssEntryFromList: List empty\n");
      return NULL;
   }

   pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

   return pTempStaEntry;
}

eHalStatus csrScanGetScanHoCandidate(tpAniSirGlobal pMac)
{
   tCsrHandoffStaInfo *pTempStaEntry = NULL;
   tANI_U32            rssiValue, oldRssiValue;
   VOS_STATUS status;

   /* This is only required for NT: all others depend upon stats based one  */
    if( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC )
   {
      smsLog(pMac, LOG1, "csrScanGetScanHoCandidate: wrong sub-state %d\n", pMac->roam.handoffInfo.currSubState);
      return eHAL_STATUS_FAILURE;
   }

   /* If a probe is already in progress then no need to do anything further */
   if( pMac->roam.handoffInfo.isProbeRspPending )
   {
      smsLog(pMac, LOGW, "csrScanGetScanHoCandidate: probe in progress\n");
      return eHAL_STATUS_FAILURE;
   }

   //set the new rssi threshold in TL
   
   if( 0 == csrScanListSize( pMac, &pMac->roam.handoffInfo.candidateList ) )
   {
      smsLog(pMac, LOGW, "csrScanGetScanHoCandidate: empty candidateList\n");
      return eHAL_STATUS_FAILURE;
   }

   /* Get the top candidate to see if that suits our requirements           */
   pTempStaEntry = 
      csrScanGetFirstBssEntryFromList( pMac, &pMac->roam.handoffInfo.candidateList);

   if(!pTempStaEntry)
   {
      smsLog(pMac, LOGW, "csrScanGetScanHoCandidate: top candidate is NULL\n");
      return eHAL_STATUS_FAILURE;
   }

   /* An AP must be a minimum of 10dB more than current AP to switch        */
   rssiValue = pTempStaEntry->sta.rssiScore +  pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta;
   oldRssiValue = pMac->roam.handoffInfo.handoffActivityInfo.currNtRssiThreshold;
   // do I need to deregister first??
   status = 
	  WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)oldRssiValue * (-1), WLANTL_HO_THRESHOLD_DOWN,
								   csrRoamNtRssiIndCallback, VOS_MODULE_ID_SME);

   //Temp fix till TL adds the support for handling multiple callback for same threshold
   if ((rssiValue == pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCurrentApGood)|| 
       (rssiValue == pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdHoFromCurrentAp)|| 
       (rssiValue == pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGood)|| 
       (rssiValue == pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGoodEmptyCandtset)||
       (rssiValue == pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdHoFromCurrentAp)||
       (rssiValue == pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCurrentApGood))
   {
      rssiValue += 1;
   }

   pMac->roam.handoffInfo.handoffActivityInfo.currNtRssiThreshold = rssiValue;
   
   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)rssiValue * (-1), 
								 WLANTL_HO_THRESHOLD_DOWN, csrRoamNtRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(status)
   {
      smsLog(pMac, LOGW, " csrScanGetScanHoCandidate: couldn't register csrRoamNtRssiIndCallback with TL\n");
      return eHAL_STATUS_FAILURE;
   }

   return eHAL_STATUS_SUCCESS;
}


tCsrBssid * csrScanGetHoCandidate(tpAniSirGlobal pMac)
{

   tCsrHandoffStaInfo *pTempStaEntry = NULL;
   tCsrBssid *         pBestBssid = NULL;
   int                 bestRssi;
   tListElem *         pEntry;
#if 0
   v_S7_t              currApRssi;
   VOS_STATUS          status;
#endif
   /* This function checks if a handoff candidate is available based on the
      entry criteria for a given state                                      */
   if( 0 == csrScanListSize( pMac, &pMac->roam.handoffInfo.candidateList ) )
   {
      smsLog(pMac, LOGW, "csrScanGetHoCandidate: empty candidateList\n");
      return NULL;
   }

   /* Get the top candidate to see if that suits our requirements           */
   pTempStaEntry = 
      csrScanGetFirstBssEntryFromList( pMac, &pMac->roam.handoffInfo.candidateList);

   if(!pTempStaEntry)
   {
      smsLog(pMac, LOGW, "csrScanGetScanHoCandidate: top candidate is NULL\n");
      return NULL;
   }
#if 0
   /* TODO : ConnectedInfo is a per session Info
   */
   pBestBssid = &pTempStaEntry->sta.bssid;
   /* An AP must be a minimum of 10dB more than current AP to switch        */
   status = WLANTL_GetRssi(pMac->roam.gVosContext, pMac->roam.connectedInfo.staId,
                           &currApRssi);

   if(status)
   {
      //err msg
      smsLog(pMac, LOGW, " csrScanGetHoCandidate: couldn't get the current APs RSSi from TL\n");
      return pBestBssid;
   }
   if( pTempStaEntry->sta.rssiScore + (int)pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta > currApRssi * (-1) )
   {
      smsLog(pMac, LOGW, " csrScanGetHoCandidate: current AP became better %d\n", currApRssi * (-1));
      return NULL;
   }
#endif


   

   bestRssi = pTempStaEntry->sta.rssiScore;
#if 0
   /* TODO : pCurRoamProfile is a per session Info
   */
   /* If no PMK/BK caching, just pick the best AP                             */
   if( (eCSR_AUTH_TYPE_RSN != (eCsrAuthType)pMac->roam.pCurRoamProfile->AuthType.authType ) &&
       (eCSR_AUTH_TYPE_RSN_PSK != (eCsrAuthType)pMac->roam.pCurRoamProfile->AuthType.authType ) 
#ifdef FEATURE_WLAN_WAPI
       &&
       (eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE != (eCsrAuthType)pMac->roam.pCurRoamProfile->AuthType.authType ) &&
       (eCSR_AUTH_TYPE_WAPI_WAI_PSK != (eCsrAuthType)pMac->roam.pCurRoamProfile->AuthType.authType )
#endif /* FEATURE_WLAN_WAPI */
      )
   {
     return pBestBssid;
   }
#endif
   /* Give preference to an AP for which we have PMK/BK cached if needed       */
   pEntry = csrLLPeekHead( &pMac->roam.handoffInfo.candidateList, LL_ACCESS_LOCK );


   while( pEntry )
   {

      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );
      
      if(csrScanPmkCacheExistsForBssid(pMac, pTempStaEntry->sta.bssid)
#ifdef FEATURE_WLAN_WAPI
       ||
         csrScanBkCacheExistsForBssid(pMac, pTempStaEntry->sta.bssid)
#endif /* FEATURE_WLAN_WAPI */
       )
      {
         //msg
         /* Check for PMK/BK cahce delta difference and if so, update best BSSID   */
         if( pTempStaEntry->sta.rssiScore < 
             bestRssi + (int)pMac->roam.handoffInfo.handoffActivityInfo.currPmkCacheRssiDelta )
         {
            bestRssi = pTempStaEntry->sta.rssiScore;
            pBestBssid = &pTempStaEntry->sta.bssid;
            //msg
            break;
         }
         
      }

      pEntry = csrLLNext( &pMac->roam.handoffInfo.candidateList, pEntry, 
                          LL_ACCESS_NOLOCK );
      
   }

   return pBestBssid;

}

#ifdef FEATURE_WLAN_WAPI
tANI_BOOLEAN csrScanBkCacheExistsForBssid(tpAniSirGlobal pMac, tCsrBssid bssid )
{
   tANI_BOOLEAN found = FALSE;
#ifdef CANNOT_SUPPORTTHIS_FUNCTION_BECAUSE_BKID_IS_SESSION
   tANI_U16  index;
   for(index = 0; index < pMac->roam.NumBkidCache; index++)
   {
      if(csrIsBssidMatch(pMac, (tCsrBssid *)&pMac->roam.BkidCacheInfo[index].BSSID, 
                         (tCsrBssid *)&bssid))
      {
         found = TRUE;
         break;
      }
   }
#endif
   return found;
}
#endif /* FEATURE_WLAN_WAPI */

tANI_BOOLEAN csrScanPmkCacheExistsForBssid(tpAniSirGlobal pMac, tCsrBssid bssid )
{
   tANI_BOOLEAN found = FALSE;
#if 0
   tANI_U16  index;
   /* TODO : NumPmkidCache, PmkidCacheInfo is a per session Info
   */
   
   for(index = 0; index < pMac->roam.NumPmkidCache; index++)
   {
      if(csrIsBssidMatch(pMac, (tCsrBssid *)&pMac->roam.PmkidCacheInfo[index].BSSID, 
                         (tCsrBssid *)&bssid))
      {
         found = TRUE;
         break;
      }
   }
#endif
   return found;
}


tANI_U32 csrScanGetQosScore(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc,
                            tDot11fBeaconIEs *pIes)
{
    tANI_U32 nRet = 0;
    tDot11fBeaconIEs *pIesLocal = pIes;

    do
   {
        if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal))) )
        {
      //err msg
      smsLog(pMac, LOGW, "csrScanGetQosScore: pIes NULL\n");
      
            break;
   }
   // if user mode is set for QoS & AP doesn't support
    if ((eCsrRoamWmmQbssOnly == pMac->roam.configParam.WMMSupportMode) &&
            !CSR_IS_QOS_BSS(pIesLocal))
    {
            break;
    }
  
    /*-------------------------------------------------------------------------
      If ACM is enabled for best effort, avoid the AP!//??
      -------------------------------------------------------------------------*/

        nRet = 100;
    }while(0);

    if( !pIes && pIesLocal )
    {
        //locally allocated
        palFreeMemory(pMac->hHdd, pIesLocal);
    }

    return nRet;
}


tANI_U32 csrScanGetSecurityScore(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc,
                                 tDot11fBeaconIEs *pIes)
{
   eCsrEncryptionType uc, mc;
   eCsrAuthType auth;
   tCsrScanResultFilter *pScanFilter = NULL;
   tANI_U32 nRet = 0;
   tDot11fBeaconIEs *pIesLocal = pIes;

   do
   {
      if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter))))
      {
         palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
#if 0
      /* TODO : pCurRoamProfile is a per session Info
       */

         if(pMac->roam.pCurRoamProfile)
         {
	        if(!HAL_STATUS_SUCCESS(csrRoamPrepareFilterFromProfile(pMac, pMac->roam.pCurRoamProfile, pScanFilter)))
	        {
		       //err msg
		       smsLog(pMac, LOGW, "csrScanGetSecurityScore: csrRoamPrepareFilterFromProfile() failed\n");
		       break;
	        }
         }
         else
#endif
         {
            smsLog(pMac, LOGW, "csrScanGetSecurityScore: pCurRoamProfile NULL\n");
            break;
         }
      }
      else
      {
	     //err msg
	     smsLog(pMac, LOGW, "csrScanGetSecurityScore: couldn't allocate memory for pScanFilter\n");
	     break;
      }

      if( !pIesLocal && (!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIesLocal))) )
      {
         //err msg
         smsLog(pMac, LOGW, "csrScanGetSecurityScore: pIes NULL\n");

         break;
      }

      if ( !csrIsSecurityMatch( pMac, &pScanFilter->authType, &pScanFilter->EncryptionType, 
                             &pScanFilter->mcEncryptionType, 
                             pBssDesc, pIesLocal, &auth, &uc, &mc ) )
      {
         break;
      }
      nRet = 100;
   }while(0);

   if( pScanFilter )
   {
      csrFreeScanFilter( pMac, pScanFilter );
      palFreeMemory( pMac->hHdd, pScanFilter );
   }

   if( !pIes && pIesLocal )
   {
      //locally allocated
      palFreeMemory(pMac->hHdd, pIesLocal);
   }

   return nRet;
}


tANI_S8 csrScanUpdateRssi(tpAniSirGlobal pMac, tANI_S8  scanRssi,
                          tANI_S8  oldRssi)
{
   tANI_U32  filteredRssi, newRssi, olderRssi;
   tANI_U32  alpha;
   tANI_U32  tempRssi;
   tANI_S8   newerRssi;
 
   newRssi = scanRssi * (-1);
   olderRssi = oldRssi * (-1);
   if(pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant)
   {
      alpha = pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant;
   }
   else
   {
      alpha = CSR_SCAN_RSSI_FILTER_FRAC - CSR_SCAN_RESULT_RSSI_WEIGHT;
   }
   /* Use the filter constant for the current state to compute new RSSI     */
   tempRssi = ((newRssi * (CSR_SCAN_RSSI_FILTER_FRAC - alpha))
               + (olderRssi * alpha));


   filteredRssi = tempRssi/CSR_SCAN_RSSI_FILTER_FRAC;

   if( 50 <= tempRssi % CSR_SCAN_RSSI_FILTER_FRAC)
   {
      filteredRssi += 1;
   }

   smsLog(pMac, LOGW, "csrScanUpdateRssi: Updating RSSI in sub-state: %d  from %d to %d\n",
          pMac->roam.handoffInfo.currSubState,
          olderRssi,
          filteredRssi);

   newerRssi = (tANI_S8)(filteredRssi * (-1));
   return newerRssi;  
}


tANI_BOOLEAN csrScanIsBgScanEnabled(tpAniSirGlobal pMac)
{
   /* This function checks if bg scan needs to be enabled for a given state
      (NT, NRT and RT)                                                       */
   if( FALSE == pMac->roam.handoffInfo.handoffActivityInfo.isHandoffPermitted )
   {
      smsLog(pMac, LOGW, "csrScanIsBgScanEnabled: bg scan disabled as handoff_permitted flag is false\n");
      return FALSE;
   }
   
   if( eSIR_ACTIVE_SCAN != pMac->roam.handoffInfo.handoffActivityInfo.scan_type )   
   {
      smsLog(pMac, LOGW, "csrScanIsBgScanEnabled: bg scan disabled as scan_type recommended by 802.11d is PASSIVE\n");
      return FALSE;
   }
   
   if( !pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted )
   {
      smsLog(pMac, LOGW, "csrScanIsBgScanEnabled: bg scan disabled as we are in %d sub-state & curr_sta's RSSI is good", pMac->roam.handoffInfo.currSubState);
      return FALSE;
   } 
   

   return TRUE;

}


tANI_U32 csrScanGetBgScanTimerVal(tpAniSirGlobal pMac)
{
   tANI_U8 size = 0;
   tANI_U32 scanTime = 0;

   /* This function returns the timer value to be used for a given state
      (NT, NRT and RT)                                                       */
   if( ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC )&&
      ( pMac->roam.handoffInfo.currSubState != eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC ) )
   {
      smsLog(pMac, LOGW, "csrScanGetBgScanTimerVal: wrong substate %d\n", pMac->roam.handoffInfo.currSubState);
      return 0;
   }

   //if timer already running return 0??



   size = csrScanListSize( pMac, &pMac->roam.handoffInfo.candidateList );

   switch( pMac->roam.handoffInfo.currSubState )
   {
   case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:

      if( 0 < size )
      {
         scanTime = pMac->roam.handoffInfo.handoffParams.ntParams.neighborApBgScanInterval;
      }
      else
      {
         scanTime = pMac->roam.handoffInfo.handoffParams.ntParams.neighborApIncrBgScanInterval;
      }
      break;
        
   case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
      if( 0 < size )
      {
         scanTime = pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanInterval;
      }
      else
      {
         if( pMac->roam.handoffInfo.handoffActivityInfo.isNrtBgScanEmptyCandSetPermitted  )
         {
            scanTime = pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanDelayInterval;             
         }
         else
         {
            scanTime = pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanIncrInterval;
         }
      }
      break;

   case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
      scanTime = pMac->roam.handoffInfo.handoffParams.rtParams.bgScanInterval;
      break;
        
   default:
      smsLog(pMac, LOGW, "csrScanGetBgScanTimerVal: Invalid current sub-state: %d", 
               pMac->roam.handoffInfo.currSubState);
        
   } /* end switch */

   smsLog(pMac, LOGW, "csrScanGetBgScanTimerVal: Setting bg scan timer : %d", 
          scanTime);

   return scanTime;

}

#endif




tANI_BOOLEAN csrScanRemoveNotRoamingScanCommand(tpAniSirGlobal pMac)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tListElem *pEntry, *pEntryTmp;
    tSmeCmd *pCommand;

    csrLLLock(&pMac->sme.smeCmdPendingList);
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
    while(pEntry)
    {
        pEntryTmp = csrLLNext(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK);
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if( eSmeCommandScan == pCommand->command )
        {
            switch( pCommand->u.scanCmd.reason )
            {
            case eCsrScanIdleScan:
            case eCsrScanGetResult:
                if( csrLLRemoveEntry(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK) )
                {
                    csrReleaseCommandScan( pMac, pCommand );
                }
                fRet = eANI_BOOLEAN_TRUE;
                break;

            default:
                break;
            } //switch
        }
        pEntry = pEntryTmp;
    }
    csrLLUnlock(&pMac->sme.smeCmdPendingList);

    return (fRet);
}


tANI_BOOLEAN csrScanRemoveFreshScanCommand(tpAniSirGlobal pMac)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tListElem *pEntry, *pEntryTmp;
    tSmeCmd *pCommand;

    csrLLLock(&pMac->sme.smeCmdPendingList);
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
    while(pEntry)
    {
        pEntryTmp = csrLLNext(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK);
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if(eSmeCommandScan == pCommand->command)
        {
            switch(pCommand->u.scanCmd.reason)
            {
            case eCsrScanGetResult:
            case eCsrScanSetBGScanParam:
            case eCsrScanBGScanAbort:
            case eCsrScanBGScanEnable:
            case eCsrScanGetScanChnInfo:
                break;
            default:
                //The rest are fresh scan requests
                if( csrLLRemoveEntry(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK) )
				{
					csrReleaseCommandScan( pMac, pCommand );
				}
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
        }
        pEntry = pEntryTmp;
    }
    csrLLUnlock(&pMac->sme.smeCmdPendingList);

    return (fRet);
}


void csrReleaseScanCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus)
{
    eCsrScanReason reason = pCommand->u.scanCmd.reason;

    csrRoamStateChange( pMac, pCommand->u.scanCmd.lastRoamState );            

#ifdef FEATURE_WLAN_GEN6_ROAMING
    if(!((eCsrScanUserRequest == reason) && (csrScanGetChannelMask(pMac))))
    {
#endif
        csrScanCallCallback(pMac, pCommand, scanStatus);
#ifdef FEATURE_WLAN_GEN6_ROAMING
    }
#endif

    smsLog(pMac, LOG3, "   Remove Scan command reason = %d\n", reason);
    if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, &pCommand->Link, LL_ACCESS_LOCK ) )
	{
		csrReleaseCommandScan( pMac, pCommand );
	}
	else
	{
		smsLog(pMac, LOGE, " ********csrReleaseScanCommand cannot release command reason %d\n", pCommand->u.scanCmd.reason );
	}
#ifdef FEATURE_WLAN_GEN6_ROAMING
	if(eCsrScanBgScan == reason ||
	   eCsrScanProbeBss == reason ||
	   eCsrScanSetBGScanParam == reason)
	{
		if(eCSR_SCAN_SUCCESS == scanStatus)
		{
			csrScanHoScanSuccess(pMac);
		}
		else
		{
			csrScanHoScanFailure(pMac);
		}
    }
#endif          
}


eHalStatus csrScanGetPMKIDCandidateList(tpAniSirGlobal pMac, tANI_U32 sessionId,
                                        tPmkidCandidateInfo *pPmkidList, tANI_U32 *pNumItems )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "  pMac->scan.NumPmkidCandidate = %d\n ", pSession->NumPmkidCandidate);
    csrResetPMKIDCandidateList(pMac, sessionId);
    if(csrIsConnStateConnected(pMac, sessionId) && pSession->pCurRoamProfile)
    {
        tCsrScanResultFilter *pScanFilter;
        tCsrScanResultInfo *pScanResult;
        tScanResultHandle hBSSList;
        tANI_U32 nItems = *pNumItems;

        *pNumItems = 0;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            //Here is the profile we need to connect to
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(HAL_STATUS_SUCCESS(status))
            {
                status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
                if(HAL_STATUS_SUCCESS(status))
                {
                    while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) && ( pSession->NumPmkidCandidate < nItems))
                    {
                        //NumPmkidCandidate adds up here
                        csrProcessBSSDescForPMKIDList(pMac, &pScanResult->BssDescriptor, 
														(tDot11fBeaconIEs *)( pScanResult->pvIes ));
                    }
                    if(pSession->NumPmkidCandidate)
                    {
                        *pNumItems = pSession->NumPmkidCandidate;
                        palCopyMemory(pMac->hHdd, pPmkidList, pSession->PmkidCandidateInfo, 
                                      pSession->NumPmkidCandidate * sizeof(tPmkidCandidateInfo));
                    }
                    csrScanResultPurge(pMac, hBSSList);
                }//Have scan result
                csrFreeScanFilter(pMac, pScanFilter);
            }
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }

    return (status);
}



#ifdef FEATURE_WLAN_WAPI
eHalStatus csrScanGetBKIDCandidateList(tpAniSirGlobal pMac, tANI_U32 sessionId,
                                       tBkidCandidateInfo *pBkidList, tANI_U32 *pNumItems )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, sessionId );

    smsLog(pMac, LOGW, "  pMac->scan.NumBkidCandidate = %d\n ", pSession->NumBkidCandidate);
    csrResetBKIDCandidateList(pMac, sessionId);
    if(csrIsConnStateConnected(pMac, sessionId) && pSession->pCurRoamProfile)
    {
        tCsrScanResultFilter *pScanFilter;
        tCsrScanResultInfo *pScanResult;
        tScanResultHandle hBSSList;
        tANI_U32 nItems = *pNumItems;
        *pNumItems = 0;
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            //Here is the profile we need to connect to
            status = csrRoamPrepareFilterFromProfile(pMac, pSession->pCurRoamProfile, pScanFilter);
            if(HAL_STATUS_SUCCESS(status))
            {
                status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
                if(HAL_STATUS_SUCCESS(status))
                {
                    while(((pScanResult = csrScanResultGetNext(pMac, hBSSList)) != NULL) && ( pSession->NumBkidCandidate < nItems))
                    {
                        //pMac->scan.NumBkidCandidate adds up here
                        csrProcessBSSDescForBKIDList(pMac, &pScanResult->BssDescriptor,
                              (tDot11fBeaconIEs *)( pScanResult->pvIes ));
				
                    }
                    if(pSession->NumBkidCandidate)
                    {
                        *pNumItems = pSession->NumBkidCandidate;
                        palCopyMemory(pMac->hHdd, pBkidList, pSession->BkidCandidateInfo, pSession->NumBkidCandidate * sizeof(tBkidCandidateInfo));
                    }
                    csrScanResultPurge(pMac, hBSSList);
                }//Have scan result
            }
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }

    return (status);
}
#endif /* FEATURE_WLAN_WAPI */



//This function is usually used for BSSs that suppresses SSID so the profile 
//shall have one and only one SSID
eHalStatus csrScanForSSID(tpAniSirGlobal pMac, tANI_U32 sessionId, tCsrRoamProfile *pProfile, tANI_U32 roamId)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pScanCmd = NULL;
    tANI_U8 bAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
    tANI_U8  index = 0;
	tANI_U32 numSsid = pProfile->SSIDs.numOfSSIDs;

    smsLog(pMac, LOG2, FL("called\n"));
	//For WDS, we use the index 0. There must be at least one in there
	if( CSR_IS_WDS_STA( pProfile ) && numSsid )
	{
		numSsid = 1;
	}
    if(pMac->scan.fScanEnable && ( numSsid == 1 ) )
    {
        do
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(!pScanCmd)
            {
                smsLog(pMac, LOGE, FL("failed to allocate command buffer\n"));
                break;
            }
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.pToRoamProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = csrRoamCopyProfile(pMac, pScanCmd->u.scanCmd.pToRoamProfile, pProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            pScanCmd->u.scanCmd.roamId = roamId;
            pScanCmd->command = eSmeCommandScan;
            pScanCmd->sessionId = (tANI_U8)sessionId; 
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanForSsid;
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //let it wrap around
            palZeroMemory(pMac->hHdd, &pScanCmd->u.scanCmd.u.scanRequest, sizeof(tCsrScanRequest));
            pScanCmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
            pScanCmd->u.scanCmd.u.scanRequest.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
            pScanCmd->u.scanCmd.u.scanRequest.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
            pScanCmd->u.scanCmd.u.scanRequest.BSSType = pProfile->BSSType;
            pScanCmd->u.scanCmd.u.scanRequest.uIEFieldLen = 0;
            if(pProfile->BSSIDs.numOfBSSIDs == 1)
            {
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.bssid, pProfile->BSSIDs.bssid, sizeof(tCsrBssid));
            }
            else
            {
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.bssid, bAddr, 6);
            }
            if(pProfile->ChannelInfo.numOfChannels)
            {
               status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList, sizeof(*pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList) * pProfile->ChannelInfo.numOfChannels);
               pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 0;
               if(HAL_STATUS_SUCCESS(status))
                {
                  csrRoamIsChannelValid(pMac, pProfile->ChannelInfo.ChannelList[0]);
                  for(index = 0; index < pProfile->ChannelInfo.numOfChannels; index++)
                  {
                     if(csrRoamIsValidChannel(pMac, pProfile->ChannelInfo.ChannelList[index]))
                     {
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList[pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels] 
                           = pProfile->ChannelInfo.ChannelList[index];
                        pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels++;
                     }
                     else 
                     {
                         smsLog(pMac, LOGW, FL("process a channel (%d) that is invalid\n"), pProfile->ChannelInfo.ChannelList[index]);
                     }

                  }
               }
               else
                {
                    break;
                }

            }
            else
            {
                pScanCmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = 0;
            }
            if(pProfile->SSIDs.numOfSSIDs)
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pScanCmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList, 
                                            pProfile->SSIDs.numOfSSIDs * sizeof(tCsrSSIDInfo)); 
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
                pScanCmd->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs = 1;
                palCopyMemory(pMac->hHdd, pScanCmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList, pProfile->SSIDs.SSIDList,
                                sizeof(tCsrSSIDInfo));
            }
            //Start process the command
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                break;
            }
        }while(0);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(pScanCmd)
            {
                csrReleaseCommandScan(pMac, pScanCmd);
                //TODO:free the memory that is allocated in this function
            }
            csrRoamCallCallback(pMac, sessionId, NULL, roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
        }
    }//valid
    else
    {
        smsLog(pMac, LOGE, FL("cannot scan because scanEnable (%d) or numSSID (%d) is invalid\n"),
                pMac->scan.fScanEnable, pProfile->SSIDs.numOfSSIDs);
    }
    
    return (status);
}


//Issue a scan base on the new capability infomation
//This should only happen when the associated AP changes its capability.
//After this scan is done, CSR reroams base on the new scan results
eHalStatus csrScanForCapabilityChange(tpAniSirGlobal pMac, tSirSmeApNewCaps *pNewCaps)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pScanCmd = NULL;

    if(pNewCaps)
    {
        do
        {
            pScanCmd = csrGetCommandBuffer(pMac);
            if(!pScanCmd)
            {
                smsLog(pMac, LOGE, FL("failed to allocate command buffer\n"));
                status = eHAL_STATUS_RESOURCES;
                break;
            }
            status = eHAL_STATUS_SUCCESS;
            pScanCmd->u.scanCmd.roamId = 0;
            pScanCmd->command = eSmeCommandScan; 
            pScanCmd->u.scanCmd.callback = NULL;
            pScanCmd->u.scanCmd.pContext = NULL;
            pScanCmd->u.scanCmd.reason = eCsrScanForCapsChange;
            pScanCmd->u.scanCmd.scanID = pMac->scan.nextScanID++; //let it wrap around
            status = csrQueueSmeCommand(pMac, pScanCmd, eANI_BOOLEAN_FALSE);
            if( !HAL_STATUS_SUCCESS( status ) )
            {
                smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
                break;
            }
        }while(0);
        if(!HAL_STATUS_SUCCESS(status))
        {
            if(pScanCmd)
            {
                csrReleaseCommandScan(pMac, pScanCmd);
            }
        }    
    }

    return (status);
}



void csrInitBGScanChannelList(tpAniSirGlobal pMac)
{
    tANI_U32 len = CSR_MIN(sizeof(pMac->roam.validChannelList), sizeof(pMac->scan.bgScanChannelList));

    palZeroMemory(pMac->hHdd, pMac->scan.bgScanChannelList, len);
    pMac->scan.numBGScanChannel = 0;

    if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len)))
    {
        pMac->scan.numBGScanChannel = (tANI_U8)CSR_MIN(len, WNI_CFG_BG_SCAN_CHANNEL_LIST_LEN);
        palCopyMemory(pMac->hHdd, pMac->scan.bgScanChannelList, pMac->roam.validChannelList, pMac->scan.numBGScanChannel);
        csrSetBGScanChannelList(pMac, pMac->scan.bgScanChannelList, pMac->scan.numBGScanChannel);
    }
}


//This function return TRUE if background scan channel list is adjusted. 
//this function will only shrink the background scan channel list
tANI_BOOLEAN csrAdjustBGScanChannelList(tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels,
                                        tANI_U8 *pAdjustChannels, tANI_U8 *pNumAdjustChannels)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tANI_U8 i, j, count = *pNumAdjustChannels;

    i = 0;
    while(i < count)
    {
        for(j = 0; j < NumChannels; j++)
        {
            if(pChannelList[j] == pAdjustChannels[i])
                break;
        }
        if(j == NumChannels)
        {
            //This channel is not in the list, remove it
            fRet = eANI_BOOLEAN_TRUE;
            count--;
            if(count - i)
            {
                palCopyMemory(pMac->hHdd, &pAdjustChannels[i], &pAdjustChannels[i+1], count - i);
            }
            else
            {
                //already remove the last one. Done.
                break;
            }
        }
        else
        {
            i++;
        }
    }//while(i<count)
    *pNumAdjustChannels = count;

    return (fRet);
}


//Get the list of the base channels to scan for passively 11d info
eHalStatus csrScanGetSupportedChannels( tpAniSirGlobal pMac )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    int n = WNI_CFG_VALID_CHANNEL_LIST_LEN;

    status = halPhyGetSupportedChannels( pMac, pMac->scan.baseChannels.channelList, &n, NULL, NULL );
    if( HAL_STATUS_SUCCESS(status) )
    {
        pMac->scan.baseChannels.numChannels = (tANI_U8)n;
    }
    else
    {
        smsLog( pMac, LOGE, FL(" failed\n") );
        pMac->scan.baseChannels.numChannels = 0;
    }

    return ( status );
}

//This function use the input pChannelList to validate the current saved channel list
eHalStatus csrSetBGScanChannelList( tpAniSirGlobal pMac, tANI_U8 *pAdjustChannels, tANI_U8 NumAdjustChannels)
{
    tANI_U32 dataLen = sizeof( tANI_U8 ) * NumAdjustChannels;

    return (ccmCfgSetStr(pMac, WNI_CFG_BG_SCAN_CHANNEL_LIST, pAdjustChannels, dataLen, NULL, eANI_BOOLEAN_FALSE));
}


void csrSetCfgValidChannelList( tpAniSirGlobal pMac, tANI_U8 *pChannelList, tANI_U8 NumChannels )
{
    tANI_U32 dataLen = sizeof( tANI_U8 ) * NumChannels;


    ccmCfgSetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST, pChannelList, dataLen, NULL, eANI_BOOLEAN_FALSE);

    return;
}



/*
 * The Tx power limits are saved in the cfg for future usage.
 */
void csrSaveTxPowerToCfg( tpAniSirGlobal pMac, tDblLinkList *pList, tANI_U32 cfgId )
{
    tListElem *pEntry;
    tANI_U32 cbLen = 0, dataLen;
    tCsrChannelPowerInfo *pChannelSet;
    tANI_U32 idx;
    tSirMacChanInfo *pChannelPowerSet;
    tANI_U8 *pBuf = NULL;

    //allocate maximum space for all channels
    dataLen = WNI_CFG_VALID_CHANNEL_LIST_LEN * sizeof(tSirMacChanInfo);
    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pBuf, dataLen)))
    {
        palZeroMemory(pMac->hHdd, pBuf, dataLen);
        pChannelPowerSet = (tSirMacChanInfo *)(pBuf);

        pEntry = csrLLPeekHead( pList, LL_ACCESS_LOCK );
        // write the tuples (startChan, numChan, txPower) for each channel found in the channel power list.
        while( pEntry )
        {
            pChannelSet = GET_BASE_ADDR( pEntry, tCsrChannelPowerInfo, link );
            if ( 1 != pChannelSet->interChannelOffset )
            {
                // we keep the 5G channel sets internally with an interchannel offset of 4.  Expand these
                // to the right format... (inter channel offset of 1 is the only option for the triplets
                // that 11d advertises.
                for( idx = 0; idx < pChannelSet->numChannels; idx++ )
                {
                    pChannelPowerSet->firstChanNum = (tSirMacChanNum)(pChannelSet->firstChannel + ( idx * pChannelSet->interChannelOffset ));
					smsLog(pMac, LOG3, " Setting Channel Number %d\n", pChannelPowerSet->firstChanNum);
                    pChannelPowerSet->numChannels  = 1;
#ifdef WLAN_SOFTAP_FEATURE
                    pChannelPowerSet->maxTxPower = CSR_ROAM_MIN( pChannelSet->txPower, pMac->roam.configParam.nTxPowerCap );
#else
                    pChannelPowerSet->maxTxPower = pChannelSet->txPower;
#endif
                    smsLog(pMac, LOG3, " Setting Max Transmit Power %d\n", pChannelPowerSet->maxTxPower);					
                    cbLen += sizeof( tSirMacChanInfo );
                    pChannelPowerSet++;
                }
            }
            else
            {
                pChannelPowerSet->firstChanNum = pChannelSet->firstChannel;
				smsLog(pMac, LOG3, " Setting Channel Number %d\n", pChannelPowerSet->firstChanNum);
                pChannelPowerSet->numChannels = pChannelSet->numChannels;
#ifdef WLAN_SOFTAP_FEATURE
                pChannelPowerSet->maxTxPower = CSR_ROAM_MIN( pChannelSet->txPower, pMac->roam.configParam.nTxPowerCap );
#else
                pChannelPowerSet->maxTxPower = pChannelSet->txPower;
#endif
                smsLog(pMac, LOG3, " Setting Max Transmit Power %d, nTxPower %d\n", pChannelPowerSet->maxTxPower,pMac->roam.configParam.nTxPowerCap );


                cbLen += sizeof( tSirMacChanInfo );
                pChannelPowerSet++;
            }

            pEntry = csrLLNext( pList, pEntry, LL_ACCESS_LOCK );
        }

        if(cbLen)
        {
            ccmCfgSetStr(pMac, cfgId, (tANI_U8 *)pBuf, cbLen, NULL, eANI_BOOLEAN_FALSE); 
        }
        palFreeMemory( pMac->hHdd, pBuf );
    }//Allocate memory
}


void csrSetCfgCountryCode( tpAniSirGlobal pMac, tANI_U8 *countryCode )
{
    tANI_U8 cc[WNI_CFG_COUNTRY_CODE_LEN];
    
	smsLog( pMac, LOG3, "Setting Country Code in Cfg from csrSetCfgCountryCode %s\n",countryCode );   
	palCopyMemory( pMac->hHdd, cc, countryCode, WNI_CFG_COUNTRY_CODE_LEN );

    // don't program the bogus country codes that we created for Korea in the MAC.  if we see
    // the bogus country codes, program the MAC with the right country code.
    if ( ( 'K'  == countryCode[ 0 ] && '1' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '2' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '3' == countryCode[ 1 ]  ) ||
         ( 'K'  == countryCode[ 0 ] && '4' == countryCode[ 1 ]  )    )
    {
        // replace the alternate Korea country codes, 'K1', 'K2', .. with 'KR' for Korea
        cc[ 1 ] = 'R';
    }
    ccmCfgSetStr(pMac, WNI_CFG_COUNTRY_CODE, cc, WNI_CFG_COUNTRY_CODE_LEN, NULL, eANI_BOOLEAN_FALSE);
}



eHalStatus csrGetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pBuf, tANI_U8 *pbLen)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tANI_U32 len;

    if(pBuf && pbLen && (*pbLen >= WNI_CFG_COUNTRY_CODE_LEN))
    {
        len = *pbLen;
        status = ccmCfgGetStr(pMac, WNI_CFG_COUNTRY_CODE, pBuf, &len);
        if(HAL_STATUS_SUCCESS(status))
        {
            *pbLen = (tANI_U8)len;
        }
    }
    
    return (status);
}


void csrSetCfgScanControlList( tpAniSirGlobal pMac, tANI_U8 *countryCode, tCsrChannel *pChannelList  )
{   
    tANI_U8 i, j;
    tANI_BOOLEAN found=FALSE;  
    tANI_U8 *pControlList = NULL;
    tANI_U32 len = WNI_CFG_SCAN_CONTROL_LIST_LEN;
    v_REGDOMAIN_t eDomain;


    if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pControlList, WNI_CFG_SCAN_CONTROL_LIST_LEN)))
    {
        palZeroMemory(pMac->hHdd, (void *)pControlList, WNI_CFG_SCAN_CONTROL_LIST_LEN);
        if(HAL_STATUS_SUCCESS(csrGetRegulatoryDomainForCountry( pMac, countryCode, &eDomain )))
        {
            if(HAL_STATUS_SUCCESS(ccmCfgGetStr(pMac, WNI_CFG_SCAN_CONTROL_LIST, pControlList, &len)))
            {
                for (i = 0; i < pChannelList->numChannels; i++)
                {
                    for (j = 0; j < len; j += 2) 
                    {
                        if (pControlList[j] == pChannelList->channelList[i]) 
                        {
                            found = TRUE;
                            break;
                        }
                    }
                   
                    if (found)    // insert a pair(channel#, flag)
                    {
                        if (CSR_IS_CHANNEL_5GHZ(pControlList[j]))
                        {
                            pControlList[j+1] = csrGetScanType(pControlList[j], eDomain);     
                        }
                        else  
                        {
                            pControlList[j+1]  = eSIR_ACTIVE_SCAN;  
                        }

                        found = FALSE;  // reset the flag
                    }
                       
                }            

                ccmCfgSetStr(pMac, WNI_CFG_SCAN_CONTROL_LIST, pControlList, len, NULL, eANI_BOOLEAN_FALSE);
            }//Successfully getting scan control list
        }//getting domain
        else
        {
            smsLog(pMac, LOGE, FL(" failed to get country-to-domain\n"));
        }
        palFreeMemory(pMac->hHdd, pControlList);
    }//AllocateMemory
}


//if bgPeriod is 0, background scan is disabled. It is in millisecond units
eHalStatus csrSetCfgBackgroundScanPeriod(tpAniSirGlobal pMac, tANI_U32 bgPeriod)
{
    return (ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, bgPeriod, (tCcmCfgSetCallback) csrScanCcmCfgSetCallback, eANI_BOOLEAN_FALSE));
}
    

void csrScanCcmCfgSetCallback(tHalHandle hHal, tANI_S32 result)
{
    tListElem *pEntry = NULL;
    tSmeCmd *pCommand = NULL;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if ( eSmeCommandScan == pCommand->command )
        {
            eCsrScanStatus scanStatus = (CCM_IS_RESULT_SUCCESS(result)) ? eCSR_SCAN_SUCCESS : eCSR_SCAN_FAILURE;
            csrReleaseScanCommand(pMac, pCommand, scanStatus);
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: Scan Completion called but SCAN command is not ACTIVE ...\n" );
        }
    }   
    smeProcessPendingQueue( pMac );
}

eHalStatus csrProcessSetBGScanParam(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    eHalStatus status;
    tCsrBGScanRequest *pScanReq = &pCommand->u.scanCmd.u.bgScanRequest;
    tANI_U32 dataLen = sizeof( tANI_U8 ) * pScanReq->ChannelInfo.numOfChannels;
        
    //***setcfg for background scan channel list
    status = ccmCfgSetInt(pMac, WNI_CFG_ACTIVE_MINIMUM_CHANNEL_TIME, pScanReq->minChnTime, NULL, eANI_BOOLEAN_FALSE);
    status = ccmCfgSetInt(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME, pScanReq->maxChnTime, NULL, eANI_BOOLEAN_FALSE);
    //Not set the background scan interval if not connected because bd scan should not be run if not connected
    if(!csrIsAllSessionDisconnected(pMac))
    {
        //If disbaling BG scan here, we need to stop aging as well
        if(pScanReq->scanInterval == 0)
        {
            //Stop aging because no new result is coming in
            csrScanStopResultAgingTimer(pMac);
        }

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
        {
            vos_log_scan_pkt_type *pScanLog = NULL;

            WLAN_VOS_DIAG_LOG_ALLOC(pScanLog, vos_log_scan_pkt_type, LOG_WLAN_SCAN_C);
            if(pScanLog)
            {
                pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_REQ;
                pScanLog->minChnTime = (v_U8_t)pScanReq->minChnTime;
                pScanLog->maxChnTime = (v_U8_t)pScanReq->maxChnTime;
                pScanLog->timeBetweenBgScan = (v_U8_t)pScanReq->scanInterval;
                pScanLog->numChannel = pScanReq->ChannelInfo.numOfChannels;
                if(pScanLog->numChannel && (pScanLog->numChannel < VOS_LOG_MAX_NUM_CHANNEL))
                {
                    palCopyMemory(pMac->hHdd, pScanLog->channels, pScanReq->ChannelInfo.ChannelList,
                        pScanLog->numChannel);
                }
                WLAN_VOS_DIAG_LOG_REPORT(pScanLog);
            }
        }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

        status = ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, pScanReq->scanInterval, NULL, eANI_BOOLEAN_FALSE);
    }
    else
    {
        //No need to stop aging because IDLE scan is still running
        status = ccmCfgSetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD, 0, NULL, eANI_BOOLEAN_FALSE);
    }
    
    if(pScanReq->SSID.length > WNI_CFG_SSID_LEN)
    {
        pScanReq->SSID.length = WNI_CFG_SSID_LEN;
    }
    
    status = ccmCfgSetStr(pMac, WNI_CFG_BG_SCAN_CHANNEL_LIST, pScanReq->ChannelInfo.ChannelList, dataLen, NULL, eANI_BOOLEAN_FALSE);
    status = ccmCfgSetStr(pMac, WNI_CFG_SSID, (tANI_U8 *)pScanReq->SSID.ssId, pScanReq->SSID.length, NULL, eANI_BOOLEAN_FALSE);



    return (status);
}


eHalStatus csrScanAbortMacScan(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirMbMsg ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, (void *)pMsg, msgLen);
        pMsg->type = pal_cpu_to_be16((tANI_U16)eWNI_SME_SCAN_ABORT_IND);
        pMsg->msgLen = pal_cpu_to_be16(msgLen);
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

	return( status );
}


eHalStatus csrScanAbortMacScanNotForConnect(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if( !csrIsScanForRoamCommandActive( pMac ) )
    {
        //Only abort the scan if it is not used for other roam/connect purpose
        status = csrScanAbortMacScan(pMac);
    }

    return (status);
}


eHalStatus csrScanGetScanChannelInfo(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMbMsg *pMsg;
    tANI_U16 msgLen;

    msgLen = (tANI_U16)(sizeof( tSirMbMsg ));
    status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->type = eWNI_SME_GET_SCANNED_CHANNEL_REQ;
        pMsg->msgLen = msgLen;
        status = palSendMBMessage(pMac->hHdd, pMsg);
    }                             

	return( status );
}

tANI_BOOLEAN csrRoamIsValidChannel( tpAniSirGlobal pMac, tANI_U8 channel )
{
    tANI_BOOLEAN fValid = FALSE;
    tANI_U32 idxValidChannels;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);
    
    for ( idxValidChannels = 0; ( idxValidChannels < len ); idxValidChannels++ )
    {
       if ( channel == pMac->roam.validChannelList[ idxValidChannels ] )
       {
          fValid = TRUE;
          break;
       }
    }
        
    return fValid;
}




#ifdef FEATURE_WLAN_GEN6_ROAMING

void csrScanSetChannelMask(tpAniSirGlobal pMac, tCsrChannelInfo *pChannelInfo)
{
    tANI_U8 index;

    tANI_U32 len = 0;

    //looking for all channels
    if(0 == pChannelInfo->numOfChannels)
    {
        if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len)))
        {
            pMac->scan.osScanChannelMask.numChannels = (tANI_U8)CSR_MIN(len, WNI_CFG_BG_SCAN_CHANNEL_LIST_LEN);
            for(index = 0; index < pMac->scan.osScanChannelMask.numChannels; index++)
            {
                pMac->scan.osScanChannelMask.channelList[index] = pMac->roam.validChannelList[index];
                pMac->scan.osScanChannelMask.scanEnabled[index] = TRUE;
            }
        }
        else
        {
            pMac->scan.osScanChannelMask.numChannels = 11;
            for(index = 0; index < pMac->scan.osScanChannelMask.numChannels; index++)
            {
                pMac->scan.osScanChannelMask.channelList[index] = index + 1;
                pMac->scan.osScanChannelMask.scanEnabled[index] = TRUE;
            }
        }
    }
    else
    {
        pMac->scan.osScanChannelMask.numChannels = pChannelInfo->numOfChannels;
        for(index = 0; index < pMac->scan.osScanChannelMask.numChannels; index++)
        {
            pMac->scan.osScanChannelMask.channelList[index] = pChannelInfo->ChannelList[index];
            pMac->scan.osScanChannelMask.scanEnabled[index] = TRUE;
        }
    }
}

tANI_BOOLEAN csrScanGetChannelMask(tpAniSirGlobal pMac)
{
    tANI_U8 index;
    tANI_BOOLEAN channelMask = FALSE;
    for(index = 0; index < pMac->scan.osScanChannelMask.numChannels; index++)
    {
        if(TRUE == pMac->scan.osScanChannelMask.scanEnabled[index])
        {
            channelMask = TRUE;
            break;
        }
    }
    return channelMask;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
void csrScanDiagHoLog(tpAniSirGlobal pMac)
{
   vos_log_ho_pkt_type *log_ptr = NULL;
   tListElem *pEntry = NULL;
   tCsrHandoffStaInfo *pTempStaEntry = NULL;
   tDblLinkList *pStaList = &pMac->roam.handoffInfo.candidateList;
   tANI_U8 index = 1;

   WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_ho_pkt_type, LOG_WLAN_HANDOFF_C);
   if(log_ptr)
   {
      log_ptr->num_aps = 1;
      log_ptr->current_ap_info.channel_id = pMac->roam.handoffInfo.currSta.pBssDesc->channelId;
      log_ptr->current_ap_info.overall_score = pMac->roam.handoffInfo.currSta.overallScore;
      log_ptr->current_ap_info.qos_score = pMac->roam.handoffInfo.currSta.qosScore;
      log_ptr->current_ap_info.rssi_score = pMac->roam.handoffInfo.currSta.rssiScore;
      log_ptr->current_ap_info.rx_per = pMac->roam.handoffInfo.currSta.rxPer;
      log_ptr->current_ap_info.sec_score = pMac->roam.handoffInfo.currSta.secScore;
      log_ptr->current_ap_info.tx_per = pMac->roam.handoffInfo.currSta.txPer;

      palCopyMemory(pMac->hHdd, log_ptr->current_ap_info.bssid, 
                    pMac->roam.handoffInfo.currSta.bssid, 6);
      palCopyMemory(pMac->hHdd, log_ptr->current_ap_info.ssid,
                    pMac->roam.connectedProfile.SSID.ssId, 
                    CSR_ROAM_MIN(9, pMac->roam.connectedProfile.SSID.length));

      pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

      if(!pEntry || !csrLLCount(pStaList))
      {
         //list empty
         smsLog(pMac, LOGW, "csrScanDiagHoLog: Candidate List empty\n");
      }
      else
      {
         while( pEntry  && index <= VOS_LOG_MAX_NUM_HO_CANDIDATE_APS)
         {
            pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );

            log_ptr->candidate_ap_info[index].channel_id = pTempStaEntry->sta.pBssDesc->channelId;
            log_ptr->candidate_ap_info[index].overall_score = pTempStaEntry->sta.overallScore;
            log_ptr->candidate_ap_info[index].qos_score = pTempStaEntry->sta.qosScore;
            log_ptr->candidate_ap_info[index].rssi_score = pTempStaEntry->sta.rssiScore;
            log_ptr->candidate_ap_info[index].rx_per = pTempStaEntry->sta.rxPer;
            log_ptr->candidate_ap_info[index].sec_score = pTempStaEntry->sta.secScore;
            log_ptr->candidate_ap_info[index].tx_per = pTempStaEntry->sta.txPer;

            palCopyMemory(pMac->hHdd, log_ptr->candidate_ap_info[index].bssid, 
                          pTempStaEntry->sta.bssid, 6);
            palCopyMemory(pMac->hHdd, log_ptr->candidate_ap_info[index].ssid,
                          pMac->roam.connectedProfile.SSID.ssId, 
                          CSR_ROAM_MIN(9, pMac->roam.connectedProfile.SSID.length));

            pEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
            log_ptr->num_aps++;
            index++;
         }
      }
   }
   WLAN_VOS_DIAG_LOG_REPORT(log_ptr);

}
#endif //FEATURE_WLAN_DIAG_SUPPORT_CSR
#endif

