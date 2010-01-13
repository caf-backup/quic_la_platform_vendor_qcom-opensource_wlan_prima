/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrApiRoam.c
  
    Implementation for the Common Roaming interfaces.
  
    Copyright (C) 2008 Qualcomm, Incorporated
  
 
   ========================================================================== */

#include "halInternal.h"
#include "halPhyApi.h"
#include "palApi.h"
#include "csrInsideApi.h"
#include "smsDebug.h"
#include "logDump.h"
#include "smeQosInternal.h"
#include "wlan_qct_tl.h"
#include "smeInside.h"
#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"

#define CSR_NUM_IBSS_START_CHANNELS_50      4
#define CSR_NUM_IBSS_START_CHANNELS_24      3
#define CSR_DEF_IBSS_START_CHANNEL_50       36
#define CSR_DEF_IBSS_START_CHANNEL_24       1
#define CSR_IBSS_JOIN_TIMEOUT_PERIOD        ( 1 *  PAL_TIMER_TO_SEC_UNIT )  // 1 second
#define CSR_WAIT_FOR_KEY_TIMEOUT_PERIOD         ( 5 * PAL_TIMER_TO_SEC_UNIT )  // 5 seconds
/*---------------------------------------------------------------------------
  OBIWAN recommends [8 10]% : pick 9% 
---------------------------------------------------------------------------*/
#define CSR_VCC_UL_MAC_LOSS_THRESHOLD 9

/*---------------------------------------------------------------------------
  OBIWAN recommends -85dBm 
---------------------------------------------------------------------------*/
#define CSR_VCC_RSSI_THRESHOLD 80
#define CSR_MIN_GLOBAL_STAT_QUERY_PERIOD   500 //ms
#define CSR_MIN_GLOBAL_STAT_QUERY_PERIOD_IN_BMPS 2000 //ms
#define CSR_MIN_TL_STAT_QUERY_PERIOD       500 //ms
#define CSR_DIAG_LOG_STAT_PERIOD           3000 //ms


/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/
#ifdef FEATURE_WLAN_DIAG_SUPPORT

int diagAuthTypeFromCSRType(eCsrAuthType authType)
{
    int n = AUTH_OPEN;

    switch(authType)
    {
    case eCSR_AUTH_TYPE_SHARED_KEY:
        n = AUTH_SHARED;
        break;

    case eCSR_AUTH_TYPE_WPA:
        n = AUTH_WPA_EAP;
        break;

    case eCSR_AUTH_TYPE_WPA_PSK:
        n = AUTH_WPA_PSK;
        break;

    case eCSR_AUTH_TYPE_RSN:
        n = AUTH_WPA2_EAP;
        break;

    case eCSR_AUTH_TYPE_RSN_PSK:
        n = AUTH_WPA2_PSK;
        break;

    default:
        break;
    }

    return (n);
}

int diagEncTypeFromCSRType(eCsrEncryptionType encType)
{
    int n = ENC_MODE_OPEN;

    switch(encType)
    {
    case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
    case eCSR_ENCRYPT_TYPE_WEP40:
        n = ENC_MODE_WEP40;
        break;

    case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
    case eCSR_ENCRYPT_TYPE_WEP104:
        n = ENC_MODE_WEP104;
        break;

    case eCSR_ENCRYPT_TYPE_TKIP:
        n = ENC_MODE_TKIP;
        break;

    case eCSR_ENCRYPT_TYPE_AES:
        n = ENC_MODE_AES;
        break;

    default:
        break;
    }

    return (n);
}

#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT

static const tANI_U8 csrStartIbssChannels50[ CSR_NUM_IBSS_START_CHANNELS_50 ] = { 36, 40,  44,  48}; 
static const tANI_U8 csrStartIbssChannels24[ CSR_NUM_IBSS_START_CHANNELS_24 ] = { 1, 6, 11 };

static void initConfigParam(tpAniSirGlobal pMac);
static void csrRoamProcessResults( tpAniSirGlobal pMac, tSmeCmd *pCommand,
                                       eCsrRoamCompleteResult Result, void *Context );
static eHalStatus csrRoamStartIbss( tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tSirBssDescription *pBssDesc, tANI_BOOLEAN *pfSameIbss );
static void csrRoamUpdateConnectedProfileFromNewBss( tpAniSirGlobal pMac, tSirSmeNewBssInfo *pNewBss );
static void csrResetStates(tpAniSirGlobal pMac);
static void csrRoamPrepareIbssParams(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
                                     tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes);
static tAniCBSecondaryMode csrGetCBModeFromIes(tpAniSirGlobal pMac, tANI_U8 primaryChn, tDot11fBeaconIEs *pIes);
eHalStatus csrInitGetChannels(tpAniSirGlobal pMac);
static void csrRoamingStateConfigCnfProcessor( tpAniSirGlobal pMac, tANI_U32 result );
eHalStatus csrRoamOpen(tpAniSirGlobal pMac);
eHalStatus csrRoamClose(tpAniSirGlobal pMac);
void csrRoamMICErrorTimerHandler(void *pv);
void csrRoamTKIPCounterMeasureTimerHandler(void *pv);
tANI_BOOLEAN csrRoamIsSameProfileKeys(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pConnProfile, tCsrRoamProfile *pProfile2);
 
static eHalStatus csrRoamStartRoamingTimer(tpAniSirGlobal pMac, tANI_U32 interval);
static eHalStatus csrRoamStopRoamingTimer(tpAniSirGlobal pMac);
static void csrRoamRoamingTimerHandler(void *pv);
eHalStatus csrRoamStartIbssJoinTimer(tpAniSirGlobal pMac, tANI_U32 interval);
eHalStatus csrRoamStopIbssJoinTimer(tpAniSirGlobal pMac);
static void csrRoamIbssJoinTimerHandler(void *pv);
eHalStatus csrRoamStartWaitForKeyTimer(tpAniSirGlobal pMac, tANI_U32 interval);
eHalStatus csrRoamStopWaitForKeyTimer(tpAniSirGlobal pMac);
static void csrRoamWaitForKeyTimeOutHandler(void *pv);
 
static eHalStatus CsrInit11dInfo(tpAniSirGlobal pMac, tCsr11dinfo *ps11dinfo);
static eHalStatus csrRoamFreeConnectedInfo( tpAniSirGlobal pMac, tCsrRoamConnectedInfo *pConnectedInfo );
eHalStatus csrSendMBSetContextReqMsg( tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, 
                                    tANI_U8 numKeys, tAniEdType edType, tANI_BOOLEAN fUnicast, tAniKeyDirection aniKeyDirection,
                                    tANI_U8 keyId, tANI_U8 keyLength, tANI_U8 *pKey, tANI_U8 paeRole );
void csrRoamIssueReassociate( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile );
void csrRoamStatisticsTimerHandler(void *pv);
void csrRoamStatsGlobalClassDTimerHandler(void *pv);

static void csrRoamLinkUp(tpAniSirGlobal pMac, tCsrBssid bssid);
VOS_STATUS csrRoamVccTriggerRssiIndCallback(tHalHandle hHal, 
                                            v_U8_t  rssiNotification, 
                                            void * context);
static void csrRoamLinkDown(tpAniSirGlobal pMac);
void csrRoamVccTrigger(tpAniSirGlobal pMac);
eHalStatus csrSendMBStatsReqMsg( tpAniSirGlobal pMac, tANI_U32 statsMask, tANI_U8 staId);

#ifdef FEATURE_WLAN_GEN6_ROAMING
static void csrRoamProcessNrtTrafficOnInd(tpAniSirGlobal pMac);
static void csrRoamProcessRtTrafficOnInd(tpAniSirGlobal pMac);
static void csrRoamProcessNrtTrafficOffInd(tpAniSirGlobal pMac);
static void csrRoamProcessRtTrafficOffInd(tpAniSirGlobal pMac);
tANI_BOOLEAN csrRoamShouldExitAp(tpAniSirGlobal pMac);
static void csrRoamUpdatePER(tpAniSirGlobal pMac);
static void csrRoamNtPreTransHandler(tpAniSirGlobal pMac);
VOS_STATUS csrRoamNtRssiIndCallback(tHalHandle hHal, 
                                    v_U8_t  rssiNotification, 
                                    void * context);
static void csrRoamNtPostTransHandler(tpAniSirGlobal pMac);
VOS_STATUS csrRoamNtBgScanRssiIndCallback(tHalHandle hHal, 
                                          v_U8_t  rssiNotification, 
                                          void * context);
static void csrRoamNrtPreTransHandler(tpAniSirGlobal pMac);
VOS_STATUS csrRoamNrtExitCriteriaRssiIndCallback(tHalHandle hHal, 
                                                 v_U8_t  rssiNotification, 
                                                 void * context);
VOS_STATUS csrRoamNrtBgScanRssiIndCallback(tHalHandle hHal, 
                                           v_U8_t  rssiNotification, 
                                           void * context);
VOS_STATUS csrRoamNrtBgScanEmptyCandSetRssiIndCallback(tHalHandle hHal, 
                                                       v_U8_t  rssiNotification, 
                                                       void * context);
static void csrRoamNrtPostTransHandler(tpAniSirGlobal pMac);
static void csrRoamRtPreTransHandler(tpAniSirGlobal pMac);
VOS_STATUS csrRoamRtExitCriteriaRssiIndCallback(tHalHandle hHal, 
                                                v_U8_t  rssiNotification, 
                                                void * context);
VOS_STATUS csrRoamRtBgScanRssiIndCallback(tHalHandle hHal, 
                                          v_U8_t  rssiNotification, 
                                          void * context);
static void csrRoamRtPostTransHandler(tpAniSirGlobal pMac);
void csrRoamCreateHandoffProfile(tpAniSirGlobal pMac, tCsrBssid bssid);
eHalStatus csrHandoffInit(tpAniSirGlobal pMac);
void csrHoConfigDefaultParams(tpAniSirGlobal pMac);
eHalStatus csrRoamStopStatisticsTimer(tpAniSirGlobal pMac);
void csrRoamHandoffStatsProcessor(tpAniSirGlobal pMac);
void  csrRoamHandoffRemoveAllFromList( tpAniSirGlobal pMac,
                                       tDblLinkList *pStaList);
eHalStatus csrRoamLostLinkAfterhandoffFailure( tpAniSirGlobal pMac);
#endif //FEATURE_WLAN_HANDOFF

tCsrStatsClientReqInfo * csrRoamInsertEntryIntoList( tpAniSirGlobal pMac,
                                                     tDblLinkList *pStaList,
                                                     tCsrStatsClientReqInfo *pStaEntry);

void csrRoamStatsClientTimerHandler(void *pv);
tCsrPeStatsReqInfo *  csrRoamCheckPeStatsReqList(tpAniSirGlobal pMac, tANI_U32  statsMask, 
                                                 tANI_U32 periodicity, tANI_BOOLEAN *pFound, tANI_U8 staId);
void csrRoamReportStatistics(tpAniSirGlobal pMac, tANI_U32 statsMask, 
                             tCsrStatsCallback callback, tANI_U8 staId, void *pContext);
void csrRoamSaveStatsFromTl(tpAniSirGlobal pMac, WLANTL_TRANSFER_STA_TYPE tlStats);
void csrRoamTlStatsTimerHandler(void *pv);
void csrRoamPeStatsTimerHandler(void *pv);
tListElem * csrRoamCheckClientReqList(tpAniSirGlobal pMac, tANI_U32 statsMask);
void csrRoamRemoveEntryFromPeStatsReqList(tpAniSirGlobal pMac, tCsrPeStatsReqInfo *pPeStaEntry);
tListElem * csrRoamFindInPeStatsReqList(tpAniSirGlobal pMac, tANI_U32  statsMask);
eHalStatus csrRoamDeregStatisticsReq(tpAniSirGlobal pMac);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
eHalStatus csrRoamStartDiagLogStatsTimer(tpAniSirGlobal pMac, tANI_U32 interval);
eHalStatus csrRoamStopDiagLogStatsTimer(tpAniSirGlobal pMac);
void csrRoamDiagLogStatsTimerHandler(void *pv);
void csrRoamDiagStatsLog(tpAniSirGlobal pMac);
#endif

eHalStatus csrOpen(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
	uNvTables nvTables;
    
    do
    {
        csrRoamStateChange( pMac, eCSR_ROAMING_STATE_STOP );
        pMac->roam.pCurRoamProfile = NULL;
        initConfigParam(pMac);
        if(!HAL_STATUS_SUCCESS((status = csrScanOpen(pMac))))
            break;
        if(!HAL_STATUS_SUCCESS((status = csrRoamOpen(pMac))))
            break;
        pMac->roam.nextRoamId = 1;  //Must not be 0
        if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &pMac->roam.statsClientReqList)))
           break;
        if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &pMac->roam.peStatsReqList)))
           break;
        if(!HAL_STATUS_SUCCESS(csrLLOpen(pMac->hHdd, &pMac->roam.roamCmdPendingList)))
           break;

        pMac->scan.domainIdDefault = halPhyGetRegDomain(pMac);
        pMac->scan.domainIdCurrent = pMac->scan.domainIdDefault;

        status = halReadNvTable( pMac, NV_TABLE_DEFAULT_COUNTRY, &nvTables );
		if (HAL_STATUS_SUCCESS( status ))
		{
            palCopyMemory( pMac->hHdd, pMac->scan.countryCodeDefault, 
					nvTables.defaultCountryTable.countryCode, WNI_CFG_COUNTRY_CODE_LEN );
		}
		else
		{
			smsLog( pMac, LOGE, FL("  fail to get NV_FIELD_IMAGE\n") );
			//hardcoded for now
			pMac->scan.countryCodeDefault[0] = 'U';
			pMac->scan.countryCodeDefault[1] = 'S';
			pMac->scan.countryCodeDefault[2] = 0;
		}
		status = palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, 
                         pMac->scan.countryCodeDefault, WNI_CFG_COUNTRY_CODE_LEN);
		status = csrInitGetChannels( pMac );

    }while(0);
    
    return (status);
}

eHalStatus csrClose(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    csrFreeRoamProfile(pMac);
    csrStop(pMac);
    csrRoamClose(pMac);
    csrScanClose(pMac);
#ifdef FEATURE_WLAN_GEN6_ROAMING
    csrLLClose(&pMac->roam.handoffInfo.candidateList);
    csrLLClose(&pMac->roam.handoffInfo.neighborList);
#endif
    csrLLClose(&pMac->roam.statsClientReqList);
    csrLLClose(&pMac->roam.peStatsReqList);
    csrLLClose(&pMac->roam.roamCmdPendingList);
    return (status);
} 

eHalStatus csrStart(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    do
    {
       //save the global vos context
        pMac->roam.gVosContext = vos_get_global_context(VOS_MODULE_ID_SME, pMac);
        csrRoamStateChange( pMac, eCSR_ROAMING_STATE_IDLE );
        status = csrRoamStart(pMac);
        if(!HAL_STATUS_SUCCESS(status)) break;
        pMac->scan.f11dInfoApplied = eANI_BOOLEAN_FALSE;
        status = pmcRegisterPowerSaveCheck(pMac, csrCheckPSReady, pMac);
        if(!HAL_STATUS_SUCCESS(status)) break;
        pMac->roam.sPendingCommands = 0;
        csrScanEnable(pMac);
        pMac->roam.ibss_join_pending = FALSE;
#ifdef FEATURE_WLAN_GEN6_ROAMING
	    status = csrHandoffInit(pMac);
#endif
        pMac->roam.tlStatsReqInfo.numClient = 0;
        pMac->roam.tlStatsReqInfo.periodicity = 0;
        pMac->roam.tlStatsReqInfo.timerRunning = FALSE;
        //init the link quality indication also
        pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_MIN_IND;
        if(!HAL_STATUS_SUCCESS(status)) 
        {
           smsLog(pMac, LOGW, " csrStart: Couldn't Init HO control blk \n");
           break;
        }
    }while(0);

#if defined(ANI_LOGDUMP)
    csrDumpInit(hHal);
#endif //#if defined(ANI_LOGDUMP)

    return (status);
}


eHalStatus csrStop(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    csrRoamStop(pMac);
    csrScanDisable(pMac);
    csrFreeConnectBssDesc(pMac);
    csrRoamFreeConnectProfile(pMac, &pMac->roam.connectedProfile);
    csrRoamFreeConnectedInfo( pMac, &pMac->roam.connectedInfo );

    //No need to lock here. SME global should be held.
    csrLLPurge( &pMac->roam.roamCmdPendingList, eANI_BOOLEAN_FALSE );
#ifdef FEATURE_WLAN_GEN6_ROAMING
    csrRoamHandoffRemoveAllFromList(pMac, &pMac->roam.handoffInfo.candidateList);
    csrRoamHandoffRemoveAllFromList(pMac, &pMac->roam.handoffInfo.neighborList);
#endif
    csrScanFlushResult(pMac); //Do we want to do this?
    csrResetStates(pMac);
    if(pMac->roam.pWpaRsnReqIE)
    {
        palFreeMemory(pMac->hHdd, pMac->roam.pWpaRsnReqIE);
        pMac->roam.pWpaRsnReqIE = NULL;
    }
    pMac->roam.nWpaRsnReqIeLength = 0;
    if(pMac->roam.pWpaRsnRspIE)
    {
        palFreeMemory(pMac->hHdd, pMac->roam.pWpaRsnRspIE);
        pMac->roam.pWpaRsnRspIE = NULL;
    }
    pMac->roam.nWpaRsnRspIeLength = 0;
    //Reset the domain back to the deault
    pMac->scan.domainIdCurrent = pMac->scan.domainIdDefault;
    csrResetCountryInformation(pMac, eANI_BOOLEAN_TRUE);
    pMac->roam.ibss_join_pending = FALSE;
    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_STOP );

    return (status);
}


eHalStatus csrReady(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    csrScanGetSupportedChannels( pMac );
    //WNI_CFG_VALID_CHANNEL_LIST should be set by this time
    //use it to init the background scan list
    csrInitBGScanChannelList(pMac);
    //Apply the default settings first
    csrResetCountryInformation(pMac, eANI_BOOLEAN_TRUE);
    /* HDD issues the init scan */
    csrScanStartResultAgingTimer(pMac);

    return (status);
}


void csrSetGlobalCfgs( tpAniSirGlobal pMac )
{
    ccmCfgSetInt(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, csrGetFragThresh(pMac), NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_RTS_THRESHOLD, csrGetRTSThresh(pMac), NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_11D_ENABLED,
                        ((pMac->roam.configParam.Is11hSupportEnabled) ? pMac->roam.configParam.Is11dSupportEnabled : pMac->roam.configParam.Is11dSupportEnabled), 
                        NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_11H_ENABLED, pMac->roam.configParam.Is11hSupportEnabled, NULL, eANI_BOOLEAN_FALSE);
    //No channel bonding for Libra
    ccmCfgSetInt(pMac, WNI_CFG_CHANNEL_BONDING_MODE, WNI_CFG_CHANNEL_BONDING_MODE_DISABLE, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pMac->roam.configParam.HeartbeatThresh24, NULL, eANI_BOOLEAN_FALSE);
}


eHalStatus csrRoamOpen(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    do
    {
    status = palTimerAlloc(pMac->hHdd, &pMac->roam.hTimerRoaming, csrRoamRoamingTimerHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
        smsLog(pMac, LOGE, FL("cannot allocate memory for Roaming timer\n"));
        break;
      }

      status = palTimerAlloc(pMac->hHdd, &pMac->roam.hTimerIbssJoining, csrRoamIbssJoinTimerHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
        smsLog(pMac, LOGE, FL("cannot allocate memory for IbssJoining timer\n"));
        break;
      }

      status = palTimerAlloc(pMac->hHdd, &pMac->roam.hTimerWaitForKey, csrRoamWaitForKeyTimeOutHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
        smsLog(pMac, LOGE, FL("cannot allocate memory for WaitForKey time out timer\n"));
        break;
      }

#ifdef FEATURE_WLAN_GEN6_ROAMING
      /* Define the Stats timers                                                */
      status = palTimerAlloc(pMac->hHdd, &pMac->roam.hTimerStatistics, csrRoamStatisticsTimerHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGE, FL("cannot allocate memory for summary Statistics timer\n"));
         return eHAL_STATUS_FAILURE;
      }
#endif
      status = palTimerAlloc(pMac->hHdd, &pMac->roam.tlStatsReqInfo.hTlStatsTimer, csrRoamTlStatsTimerHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGE, FL("cannot allocate memory for summary Statistics timer\n"));
         return eHAL_STATUS_FAILURE;
      }
#ifdef FEATURE_WLAN_DIAG_SUPPORT
      status = palTimerAlloc(pMac->hHdd, &pMac->roam.hTimerDiagLogStats, csrRoamDiagLogStatsTimerHandler, pMac);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGE, FL("cannot allocate memory for Diag log Statistics timer\n"));
         return eHAL_STATUS_FAILURE;
      }
#endif
    }while (0);

    return (status);
}


eHalStatus csrRoamClose(tpAniSirGlobal pMac)
{
    csrRoamStop(pMac);
    palTimerFree(pMac->hHdd, pMac->roam.hTimerRoaming);
    palTimerFree(pMac->hHdd, pMac->roam.hTimerWaitForKey);
    palTimerFree(pMac->hHdd, pMac->roam.hTimerIbssJoining);
#ifdef FEATURE_WLAN_GEN6_ROAMING
    palTimerFree(pMac->hHdd, pMac->roam.hTimerStatistics);
#endif
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    palTimerFree(pMac->hHdd, pMac->roam.hTimerDiagLogStats);
#endif
    palTimerFree(pMac->hHdd, pMac->roam.tlStatsReqInfo.hTlStatsTimer);

    return (eHAL_STATUS_SUCCESS);
}


eHalStatus csrRoamStart(tpAniSirGlobal pMac)
{
    (void)pMac;

    return (eHAL_STATUS_SUCCESS);
}


void csrRoamStop(tpAniSirGlobal pMac)
{
    csrRoamStopRoamingTimer(pMac);
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
//HO
eHalStatus csrHandoffInit(tpAniSirGlobal pMac)
{
   eHalStatus status;
   tSirMacAddr BroadcastMac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
   /*  Reset all the parameters of the state machine                        */

   palZeroMemory(pMac->hHdd, &pMac->roam.summaryStatsInfo, sizeof(tCsrSummaryStatsInfo));

   pMac->roam.handoffInfo.currState = pMac->roam.curState;
   pMac->roam.handoffInfo.currSubState = pMac->roam.curSubState;

   palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                 BroadcastMac, WNI_CFG_BSSID_LEN);

   /* create the bg scan channel list*/
   	
   status = palAllocateMemory(pMac->hHdd, 
							  (void **)&pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList,
                              WNI_CFG_VALID_CHANNEL_LIST_LEN);

   if(!HAL_STATUS_SUCCESS(status))
   {
	   smsLog(pMac, LOGW, " csrHandoffInit: cannot allocate memory for currBgScanChannelList\n");

	   return eHAL_STATUS_FAILURE;

   }
   palZeroMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.ChannelList, 
				 WNI_CFG_VALID_CHANNEL_LIST_LEN);
   pMac->roam.handoffInfo.handoffActivityInfo.currBgScanChannelList.numOfChannels = 0;

   /* Initialize neighbor & candidate lists (only once)                     */
   if (csrLLOpen(pMac->hHdd, &pMac->roam.handoffInfo.candidateList) != eHAL_STATUS_SUCCESS)
   {
      smsLog(pMac, LOGW, " csrHandoffInit: cannot initialize candidateList\n");

      return eHAL_STATUS_FAILURE;
   }

   if (csrLLOpen(pMac->hHdd, &pMac->roam.handoffInfo.neighborList) != eHAL_STATUS_SUCCESS)
   {
      smsLog(pMac, LOGW, " csrHandoffInit: cannot initialize neighborList\n");

      return eHAL_STATUS_FAILURE;
   }

   return eHAL_STATUS_SUCCESS;
}

eHalStatus csrUpdateHandoffParams(tpAniSirGlobal pMac)
{
   smsLog(pMac, LOG1, " csrUpdateHandoffParams: update handoffPermitted flag & some bg scan related info\n");
   pMac->roam.handoffInfo.handoffActivityInfo.maxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
   pMac->roam.handoffInfo.handoffActivityInfo.minChnTime = pMac->roam.configParam.nActiveMinChnTime;
   //since we are associated setting it to active always
   pMac->roam.handoffInfo.handoffActivityInfo.scan_type = eSIR_ACTIVE_SCAN;

   if(!HAL_STATUS_SUCCESS(csrScanCreateOtherChanList(pMac)))
   {
      return eHAL_STATUS_FAILURE;
   }
   
   pMac->roam.handoffInfo.handoffActivityInfo.isHandoffPermitted = 
      pMac->roam.connectedProfile.handoffPermitted;

   //incase we linkup in NRT or RT sub-state, we want to init the index to 0 
   //(to avoid them containing any junk value)
   pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex = 0;
   pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex = 0;

   return eHAL_STATUS_SUCCESS;
}
#endif


static void csrResetStates(tpAniSirGlobal pMac)
{
    pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
    pMac->roam.curState = eCSR_ROAMING_STATE_STOP;
    pMac->roam.curSubState = eCSR_ROAM_SUBSTATE_NONE;
    pMac->scan.fCancelIdleScan = eANI_BOOLEAN_FALSE;
    pMac->scan.fRestartIdleScan = eANI_BOOLEAN_FALSE;
}

eHalStatus csrRoamGetConnectState(tHalHandle hHal, eCsrConnectState *pState)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if(pState)
    {
        status = eHAL_STATUS_SUCCESS;
        *pState = pMac->roam.connectState;
    }    
    return (status);
}



eHalStatus csrRoamCopyConnectProfile(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pProfile)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tANI_U32 size = 0;
    
    if(pProfile)
    {
        if(pMac->roam.pConnectBssDesc)
        {
            do
            {
                size = pMac->roam.pConnectBssDesc->length + sizeof(pMac->roam.pConnectBssDesc->length);
                if(size)
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&pProfile->pBssDesc, size);
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        palCopyMemory(pMac->hHdd, pProfile->pBssDesc, pMac->roam.pConnectBssDesc, size);
                    }
                    else
                        break;
                }
                else
                {
                    pProfile->pBssDesc = NULL;
                }
                pProfile->AuthType = pMac->roam.connectedProfile.AuthType;
                pProfile->EncryptionType = pMac->roam.connectedProfile.EncryptionType;
                pProfile->mcEncryptionType = pMac->roam.connectedProfile.mcEncryptionType;
                pProfile->BSSType = pMac->roam.connectedProfile.BSSType;
                pProfile->operationChannel = pMac->roam.connectedProfile.operationChannel;
                pProfile->CBMode = pMac->roam.connectedProfile.CBMode;
                palCopyMemory(pMac->hHdd, &pProfile->bssid, &pMac->roam.connectedProfile.bssid, sizeof(tCsrBssid));
                palCopyMemory(pMac->hHdd, &pProfile->SSID, &pMac->roam.connectedProfile.SSID, sizeof(tSirMacSSid));
            }while(0);
        }
    }
    
    return (status);
}



eHalStatus csrRoamGetConnectProfile(tHalHandle hHal, tCsrRoamConnectedProfile *pProfile)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    if(csrIsConnStateConnected(pMac))
    {
        if(pProfile)
        {
            status = csrRoamCopyConnectProfile(pMac, pProfile);
        }
    }

    return (status);
}

eHalStatus csrRoamFreeConnectProfile(tHalHandle hHal, tCsrRoamConnectedProfile *pProfile)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    if(pProfile->pBssDesc)
    {
        palFreeMemory(pMac->hHdd, pProfile->pBssDesc);
    }
    palZeroMemory(pMac->hHdd, pProfile, sizeof(tCsrRoamConnectedProfile));
    pProfile->AuthType = eCSR_AUTH_TYPE_UNKNOWN;
    return (status);
}


static eHalStatus csrRoamFreeConnectedInfo( tpAniSirGlobal pMac, tCsrRoamConnectedInfo *pConnectedInfo )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if( pConnectedInfo->pbFrames )
    {
        palFreeMemory( pMac->hHdd, pConnectedInfo->pbFrames );
        pConnectedInfo->pbFrames = NULL;
    }
    pConnectedInfo->nBeaconLength = 0;
    pConnectedInfo->nAssocReqLength = 0;
    pConnectedInfo->nAssocRspLength = 0;
    pConnectedInfo->staId = 0;

#ifdef FEATURE_WLAN_GEN6_ROAMING
    pConnectedInfo->sHOScanChannelList.numChannels = 0;
#endif

    return ( status );
}




void csrReleaseCommandRoam(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    VOS_ASSERT( pMac->roam.sPendingCommands > 0 );
    pMac->roam.sPendingCommands--;
    csrReinitRoamCmd(pMac, pCommand);
    smeReleaseCommand( pMac, pCommand );
}


void csrReleaseCommandScan(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    VOS_ASSERT( pMac->roam.sPendingCommands > 0 );
    pMac->roam.sPendingCommands--;
    csrReinitScanCmd(pMac, pCommand);
    smeReleaseCommand( pMac, pCommand );
}


void csrReleaseCommandWmStatusChange(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    VOS_ASSERT( pMac->roam.sPendingCommands > 0 );
    pMac->roam.sPendingCommands--;
    csrReinitWmStatusChangeCmd(pMac, pCommand);
    smeReleaseCommand( pMac, pCommand );
}



void csrReinitSetKeyCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    palZeroMemory(pMac->hHdd, &pCommand->u.setKeyCmd, sizeof(tSetKeyCmd));
}


void csrReinitRemoveKeyCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    palZeroMemory(pMac->hHdd, &pCommand->u.removeKeyCmd, sizeof(tRemoveKeyCmd));
}


void csrReleaseCommandSetKey(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    VOS_ASSERT( pMac->roam.sPendingCommands > 0 );
    pMac->roam.sPendingCommands--;
    csrReinitSetKeyCmd(pMac, pCommand);
    smeReleaseCommand( pMac, pCommand );
}

void csrReleaseCommandRemoveKey(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    VOS_ASSERT( pMac->roam.sPendingCommands > 0 );
    pMac->roam.sPendingCommands--;
    csrReinitRemoveKeyCmd(pMac, pCommand);
    smeReleaseCommand( pMac, pCommand );
}

void csrAbortCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fStopping )
{
    if( eSmeCsrCommandMask & pCommand->command )
    {
        switch (pCommand->command)
        {
        case eSmeCommandScan:
            csrReleaseCommandScan( pMac, pCommand );
            break;

        case eSmeCommandRoam:
            csrReleaseCommandRoam( pMac, pCommand );
            break;

        case eSmeCommandWmStatusChange:
            csrReleaseCommandWmStatusChange( pMac, pCommand );
            break;

        case eSmeCommandSetKey:
            csrReleaseCommandSetKey( pMac, pCommand );
            break;

        case eSmeCommandRemoveKey:
            csrReleaseCommandRemoveKey( pMac, pCommand );
            break;

        default:
            smsLog( pMac, LOGW, " CSR abort standard command %d\n", pCommand->command );
            smeReleaseCommand( pMac, pCommand );
            break;
        }
    }
}



void csrRoamSubstateChange( tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate )
{
    smsLog( pMac, LOG1, "   CSR RoamSubstate: [ %d <== %d ]\n", NewSubstate, pMac->roam.curSubState);


    if(pMac->roam.curSubState == NewSubstate)
    {
       return;
    }
#ifdef FEATURE_WLAN_GEN6_ROAMING
    switch(pMac->roam.curSubState)
    {
    case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:
       csrRoamNtPreTransHandler(pMac);
       break;
    case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
       csrRoamNrtPreTransHandler(pMac);
       break;
    case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
       csrRoamRtPreTransHandler(pMac);
       break;
    default:
        break;

    }
#endif
    pMac->roam.curSubState = NewSubstate;
#ifdef FEATURE_WLAN_GEN6_ROAMING
    pMac->roam.handoffInfo.currSubState = pMac->roam.curSubState;

    switch(pMac->roam.curSubState)
    {
    case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:
       csrRoamNtPostTransHandler(pMac);
       break;
    case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
       csrRoamNrtPostTransHandler(pMac);
       break;
    case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
       csrRoamRtPostTransHandler(pMac);
       break;
    default:
        break;
       
    }
#endif
}


eCsrRoamState csrRoamStateChange( tpAniSirGlobal pMac, eCsrRoamState NewRoamState )
{
    eCsrRoamState PreviousState;
          
    smsLog( pMac, LOG1, "CSR RoamState: [ %d <== %d ]\n", NewRoamState, pMac->roam.curState);

    PreviousState = pMac->roam.curState;
    
    if ( NewRoamState != pMac->roam.curState ) 
    {
        // Whenever we transition OUT of the Roaming state, clear the Roaming substate...
        if ( CSR_IS_ROAM_JOINING(pMac) ) 
        {
            csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
        }
    
        pMac->roam.curState = NewRoamState;    
    }
#ifdef FEATURE_WLAN_GEN6_ROAMING
    pMac->roam.handoffInfo.currState = pMac->roam.curState;
#endif    
    return( PreviousState );
}


void csrAssignRssiForCategory(tpAniSirGlobal pMac, tANI_U8 catOffset)
{
    int i;

    if(catOffset)
    {
        pMac->roam.configParam.bCatRssiOffset = catOffset;
        for(i = 0; i < CSR_NUM_RSSI_CAT; i++)
        {
            pMac->roam.configParam.RSSICat[CSR_NUM_RSSI_CAT - i - 1] = (int)CSR_BEST_RSSI_VALUE - (int)(i * catOffset);
        }
    }
}


static void initConfigParam(tpAniSirGlobal pMac)
{
    int i;

    pMac->roam.configParam.agingCount = CSR_AGING_COUNT;
    pMac->roam.configParam.ChannelBondingMode = eANI_BOOLEAN_TRUE;
    pMac->roam.configParam.phyMode = eCSR_DOT11_MODE_TAURUS;
    pMac->roam.configParam.eBand = eCSR_BAND_ALL;
    pMac->roam.configParam.uCfgDot11Mode = eCSR_CFG_DOT11_MODE_TAURUS;
    pMac->roam.configParam.FragmentationThreshold = eCSR_DOT11_FRAG_THRESH_DEFAULT;
    pMac->roam.configParam.HeartbeatThresh24 = 40;
    pMac->roam.configParam.HeartbeatThresh50 = 40;
    pMac->roam.configParam.Is11dSupportEnabled = eANI_BOOLEAN_FALSE;
    pMac->roam.configParam.Is11eSupportEnabled = eANI_BOOLEAN_TRUE;
    pMac->roam.configParam.Is11hSupportEnabled = eANI_BOOLEAN_TRUE;
    pMac->roam.configParam.RTSThreshold = 2346;
    pMac->roam.configParam.shortSlotTime = eANI_BOOLEAN_TRUE;
    pMac->roam.configParam.WMMSupportMode = eCsrRoamWmmAuto;
    pMac->roam.configParam.ProprietaryRatesEnabled = eANI_BOOLEAN_TRUE;
    pMac->roam.configParam.TxRate = eCSR_TX_RATE_AUTO;
    pMac->roam.configParam.impsSleepTime = CSR_IDLE_SCAN_NO_PS_INTERVAL;
    pMac->roam.configParam.scanAgeTimeNCNPS = CSR_SCAN_AGING_TIME_NOT_CONNECT_NO_PS;  
    pMac->roam.configParam.scanAgeTimeNCPS = CSR_SCAN_AGING_TIME_NOT_CONNECT_W_PS;   
    pMac->roam.configParam.scanAgeTimeCNPS = CSR_SCAN_AGING_TIME_CONNECT_NO_PS;   
    pMac->roam.configParam.scanAgeTimeCPS = CSR_SCAN_AGING_TIME_CONNECT_W_PS;   
    for(i = 0; i < CSR_NUM_RSSI_CAT; i++)
    {
        pMac->roam.configParam.BssPreferValue[i] = i;
    }
    csrAssignRssiForCategory(pMac, CSR_DEFAULT_RSSI_DB_GAP);
    pMac->roam.configParam.nRoamingTime = CSR_DEFAULT_ROAMING_TIME;
    pMac->roam.configParam.fEnforce11dChannels = eANI_BOOLEAN_FALSE;
    pMac->roam.configParam.fEnforceCountryCodeMatch = eANI_BOOLEAN_FALSE;
    pMac->roam.configParam.fEnforceDefaultDomain = eANI_BOOLEAN_FALSE;
    pMac->roam.configParam.nActiveMaxChnTime = CSR_ACTIVE_MAX_CHANNEL_TIME;
    pMac->roam.configParam.nActiveMinChnTime = CSR_ACTIVE_MIN_CHANNEL_TIME;
    pMac->roam.configParam.nPassiveMaxChnTime = CSR_PASSIVE_MAX_CHANNEL_TIME;
    pMac->roam.configParam.nPassiveMinChnTime = CSR_PASSIVE_MIN_CHANNEL_TIME;

    pMac->roam.configParam.IsIdleScanEnabled = TRUE; //enable the idle scan by default
    pMac->roam.configParam.nTxPowerCap = CSR_MAX_TX_POWER;
    pMac->roam.configParam.statsReqPeriodicity = CSR_MIN_GLOBAL_STAT_QUERY_PERIOD;
    pMac->roam.configParam.statsReqPeriodicityInPS = CSR_MIN_GLOBAL_STAT_QUERY_PERIOD_IN_BMPS;
}


eHalStatus csrChangeDefaultConfigParam(tHalHandle hHal, tCsrConfigParam *pParam)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_SUCCESS;
    if(pParam)
    {
        pMac->roam.configParam.WMMSupportMode = pParam->WMMSupportMode;
        pMac->roam.configParam.Is11eSupportEnabled = pParam->Is11eSupportEnabled;
        pMac->roam.configParam.FragmentationThreshold = pParam->FragmentationThreshold;
        pMac->roam.configParam.Is11dSupportEnabled = pParam->Is11dSupportEnabled;
        pMac->roam.configParam.Is11hSupportEnabled = pParam->Is11hSupportEnabled;
        //if 11h is enabled, so should 11d
        if(pMac->roam.configParam.Is11hSupportEnabled)
        {
            pMac->roam.configParam.Is11dSupportEnabled = eANI_BOOLEAN_TRUE;
        }
        pMac->roam.configParam.ChannelBondingMode = pParam->ChannelBondingMode;
        pMac->roam.configParam.RTSThreshold = pParam->RTSThreshold;
        pMac->roam.configParam.phyMode = pParam->phyMode;
        pMac->roam.configParam.shortSlotTime = pParam->shortSlotTime;
        pMac->roam.configParam.HeartbeatThresh24 = pParam->HeartbeatThresh24;
        pMac->roam.configParam.HeartbeatThresh50 = pParam->HeartbeatThresh50;
        pMac->roam.configParam.ProprietaryRatesEnabled = pParam->ProprietaryRatesEnabled;
        pMac->roam.configParam.TxRate = pMac->roam.configParam.TxRate;
        pMac->roam.configParam.AdHocChannel24 = pParam->AdHocChannel24;
        pMac->roam.configParam.AdHocChannel5G = pParam->AdHocChannel5G;
        pMac->roam.configParam.bandCapability = pParam->bandCapability;
        pMac->roam.configParam.cbChoice = pParam->cbChoice;
        pMac->roam.configParam.bgScanInterval = pParam->bgScanInterval;

        //if HDD passed down non zero values then only update,
        //otherwise keep using the defaults
        if(pParam->nActiveMaxChnTime)
        {
        pMac->roam.configParam.nActiveMaxChnTime = pParam->nActiveMaxChnTime;
        }
        if(pParam->nActiveMinChnTime)
        {
        pMac->roam.configParam.nActiveMinChnTime = pParam->nActiveMinChnTime;
        }
        if(pParam->nPassiveMaxChnTime)
        {
        pMac->roam.configParam.nPassiveMaxChnTime = pParam->nPassiveMaxChnTime;
        }
        if(pParam->nPassiveMinChnTime)
        {
        pMac->roam.configParam.nPassiveMinChnTime = pParam->nPassiveMinChnTime;
        }
        //if upper layer wants to disable idle scan altogether set it to 0
        if(pParam->impsSleepTime)
        {
            //Change the unit from second to microsecond
            tANI_U32 impsSleepTime = pParam->impsSleepTime * PAL_TIMER_TO_SEC_UNIT;

            if(CSR_IDLE_SCAN_NO_PS_INTERVAL_MIN <= impsSleepTime)
            {
                pMac->roam.configParam.impsSleepTime = impsSleepTime;
            }
            else
            {
                pMac->roam.configParam.impsSleepTime = CSR_IDLE_SCAN_NO_PS_INTERVAL;
            }
        }
        else
        {
           pMac->roam.configParam.impsSleepTime = 0;
        }
        pMac->roam.configParam.eBand = pParam->eBand;
        pMac->roam.configParam.uCfgDot11Mode = csrGetCfgDot11ModeFromCsrPhyMode(pMac->roam.configParam.phyMode, 
                                                    pMac->roam.configParam.ProprietaryRatesEnabled);
        //if HDD passed down non zero values for age params, then only update,
        //otherwise keep using the defaults
        if(pParam->nScanResultAgeCount)
        {
            pMac->roam.configParam.agingCount = pParam->nScanResultAgeCount;
        }

        if(pParam->scanAgeTimeNCNPS)
        {
        pMac->roam.configParam.scanAgeTimeNCNPS = pParam->scanAgeTimeNCNPS;  
        }

        if(pParam->scanAgeTimeNCPS)
        {
        pMac->roam.configParam.scanAgeTimeNCPS = pParam->scanAgeTimeNCPS;   
        }

        if(pParam->scanAgeTimeCNPS)
        {
        pMac->roam.configParam.scanAgeTimeCNPS = pParam->scanAgeTimeCNPS;   
        }
        if(pParam->scanAgeTimeCPS)
        {
        pMac->roam.configParam.scanAgeTimeCPS = pParam->scanAgeTimeCPS;   
        }
        
        csrAssignRssiForCategory(pMac, pParam->bCatRssiOffset);
        pMac->roam.configParam.nRoamingTime = pParam->nRoamingTime;
        pMac->roam.configParam.fEnforce11dChannels = pParam->fEnforce11dChannels;
        pMac->roam.configParam.fEnforceCountryCodeMatch = pParam->fEnforceCountryCodeMatch;
        pMac->roam.configParam.fEnforceDefaultDomain = pParam->fEnforceDefaultDomain;

        pMac->roam.configParam.vccRssiThreshold = pParam->vccRssiThreshold;
        pMac->roam.configParam.vccUlMacLossThreshold = pParam->vccUlMacLossThreshold;

        pMac->roam.configParam.IsIdleScanEnabled = pParam->IsIdleScanEnabled;
        pMac->roam.configParam.statsReqPeriodicity = pParam->statsReqPeriodicity;
        pMac->roam.configParam.statsReqPeriodicityInPS = pParam->statsReqPeriodicityInPS;
		//Assign this before calling CsrInit11dInfo
        pMac->roam.configParam.nTxPowerCap = pParam->nTxPowerCap;

        status = CsrInit11dInfo(pMac, &pParam->Csr11dinfo);
        if( !csrIs11dSupported( pMac ) )
        {
            pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
        }

    }
    
    return status;
}


eHalStatus csrGetConfigParam(tHalHandle hHal, tCsrConfigParam *pParam)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;

    if(pParam)
    {
        pParam->WMMSupportMode = pMac->roam.configParam.WMMSupportMode;
        pParam->Is11eSupportEnabled = pMac->roam.configParam.Is11eSupportEnabled;
        pParam->FragmentationThreshold = pMac->roam.configParam.FragmentationThreshold;
        pParam->Is11dSupportEnabled = pMac->roam.configParam.Is11dSupportEnabled;
        pParam->Is11hSupportEnabled = pMac->roam.configParam.Is11hSupportEnabled;
        pParam->ChannelBondingMode = pMac->roam.configParam.ChannelBondingMode;
        pParam->RTSThreshold = pMac->roam.configParam.RTSThreshold;
        pParam->phyMode = pMac->roam.configParam.phyMode;
        pParam->shortSlotTime = pMac->roam.configParam.shortSlotTime;
        pParam->HeartbeatThresh24 = pMac->roam.configParam.HeartbeatThresh24;
        pParam->HeartbeatThresh50 = pMac->roam.configParam.HeartbeatThresh50;
        pParam->ProprietaryRatesEnabled = pMac->roam.configParam.ProprietaryRatesEnabled;
        pMac->roam.configParam.TxRate = pMac->roam.configParam.TxRate;
        pParam->AdHocChannel24 = pMac->roam.configParam.AdHocChannel24;
        pParam->AdHocChannel5G = pMac->roam.configParam.AdHocChannel5G;
        pParam->bandCapability = pMac->roam.configParam.bandCapability;
        pParam->cbChoice = pMac->roam.configParam.cbChoice;
        pParam->bgScanInterval = pMac->roam.configParam.bgScanInterval;

        pParam->nActiveMaxChnTime = pMac->roam.configParam.nActiveMaxChnTime;
        pParam->nActiveMinChnTime = pMac->roam.configParam.nActiveMinChnTime;
        pParam->nPassiveMaxChnTime = pMac->roam.configParam.nPassiveMaxChnTime;
        pParam->nPassiveMinChnTime = pMac->roam.configParam.nPassiveMinChnTime;

        //Change the unit from microsecond to second
        pParam->impsSleepTime = pMac->roam.configParam.impsSleepTime / PAL_TIMER_TO_SEC_UNIT;
        pParam->eBand = pMac->roam.configParam.eBand;
        pParam->nScanResultAgeCount = pMac->roam.configParam.agingCount;
        pParam->scanAgeTimeNCNPS = pMac->roam.configParam.scanAgeTimeNCNPS;  
        pParam->scanAgeTimeNCPS = pMac->roam.configParam.scanAgeTimeNCPS;   
        pParam->scanAgeTimeCNPS = pMac->roam.configParam.scanAgeTimeCNPS;   
        pParam->scanAgeTimeCPS = pMac->roam.configParam.scanAgeTimeCPS;   
        pParam->bCatRssiOffset = pMac->roam.configParam.bCatRssiOffset;
        pParam->nRoamingTime = pMac->roam.configParam.nRoamingTime;
        pParam->fEnforce11dChannels = pMac->roam.configParam.fEnforce11dChannels;
        pParam->fEnforceCountryCodeMatch = pMac->roam.configParam.fEnforceCountryCodeMatch;
        pParam->vccRssiThreshold = pMac->roam.configParam.vccRssiThreshold;
        pParam->vccUlMacLossThreshold = pMac->roam.configParam.vccUlMacLossThreshold;

        pParam->IsIdleScanEnabled = pMac->roam.configParam.IsIdleScanEnabled;
        pParam->nTxPowerCap = pMac->roam.configParam.nTxPowerCap;
        pParam->statsReqPeriodicity = pMac->roam.configParam.statsReqPeriodicity;
        pParam->statsReqPeriodicityInPS = pMac->roam.configParam.statsReqPeriodicityInPS;

        status = eHAL_STATUS_SUCCESS;
    }

    return (status);
}


eHalStatus csrSetPhyMode(tHalHandle hHal, tANI_U32 phyMode, eCsrBand eBand, tANI_BOOLEAN *pfRestartNeeded)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tANI_BOOLEAN fRestartNeeded = eANI_BOOLEAN_FALSE;
    eCsrPhyMode newPhyMode = eCSR_DOT11_MODE_AUTO;

    do
    {
        if(eCSR_BAND_24 == eBand)
        {
            if(CSR_IS_RADIO_A_ONLY(pMac)) break;
            if((eCSR_DOT11_MODE_11a & phyMode) || (eCSR_DOT11_MODE_11a_ONLY & phyMode)) break;
        }
        if(eCSR_BAND_5G == eBand)
        {
            if(CSR_IS_RADIO_BG_ONLY(pMac)) break;
            if((eCSR_DOT11_MODE_11b & phyMode) || (eCSR_DOT11_MODE_11b_ONLY & phyMode) ||
                (eCSR_DOT11_MODE_11g & phyMode) || (eCSR_DOT11_MODE_11g_ONLY & phyMode) ||
                (eCSR_DOT11_MODE_TITAN & phyMode)
                ) 
            {
                break;
            }
        }
        //eCSR_DOT11_MODE_TAURUS is not 0
        if((0 == phyMode) || (eCSR_DOT11_MODE_TAURUS & phyMode))
        {
            newPhyMode = eCSR_DOT11_MODE_TAURUS;
        }
        else if(eCSR_DOT11_MODE_AUTO & phyMode)
        {
            newPhyMode = eCSR_DOT11_MODE_AUTO;
        }
        else
        {
            //Check for dual band and higher capability first
            if(eCSR_DOT11_MODE_TAURUS_ONLY & phyMode)
            {
                if(eCSR_DOT11_MODE_TAURUS_ONLY != phyMode) break;
                newPhyMode = eCSR_DOT11_MODE_TAURUS_ONLY;
            }
            else if(eCSR_DOT11_MODE_11n_ONLY & phyMode)
            {
                if(eCSR_DOT11_MODE_11n_ONLY != phyMode) break;
                newPhyMode = eCSR_DOT11_MODE_11n_ONLY;
            }
            else if(eCSR_DOT11_MODE_11a_ONLY & phyMode)
            {
                if(eCSR_DOT11_MODE_11a_ONLY != phyMode) break;
                if(eCSR_BAND_24 == eBand) break;
                newPhyMode = eCSR_DOT11_MODE_11a_ONLY;
                eBand = eCSR_BAND_5G;
            }
            else if(eCSR_DOT11_MODE_11g_ONLY & phyMode)
            {
                if(eCSR_DOT11_MODE_11g_ONLY != phyMode) break;
                if(eCSR_BAND_5G == eBand) break;
                newPhyMode = eCSR_DOT11_MODE_11g_ONLY;
                eBand = eCSR_BAND_24;
            }
            else if(eCSR_DOT11_MODE_11b_ONLY & phyMode)
            {
                if(eCSR_DOT11_MODE_11b_ONLY != phyMode) break;
                if(eCSR_BAND_5G == eBand) break;
                newPhyMode = eCSR_DOT11_MODE_11b_ONLY;
                eBand = eCSR_BAND_24;
            }
            else if(eCSR_DOT11_MODE_11n & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_11n;
            }
            else if(eCSR_DOT11_MODE_abg & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_abg;
            }
            else if(eCSR_DOT11_MODE_TITAN & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_TITAN;
                eBand = eCSR_BAND_24;
            }
            else if(eCSR_DOT11_MODE_POLARIS & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_POLARIS;
            }
            else if(eCSR_DOT11_MODE_11a & phyMode)
            {
                if((eCSR_DOT11_MODE_11g & phyMode) || (eCSR_DOT11_MODE_11b & phyMode))
                {
                    if(eCSR_BAND_ALL == eBand)
                    {
                        newPhyMode = eCSR_DOT11_MODE_abg;
                    }
                    else
                    {
                        //bad setting
                        break;
                    }
                }
                else
                {
                    newPhyMode = eCSR_DOT11_MODE_11a;
                    eBand = eCSR_BAND_5G;
                }
            }
            else if(eCSR_DOT11_MODE_11g & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_11g;
                eBand = eCSR_BAND_24;
            }
            else if(eCSR_DOT11_MODE_11b & phyMode)
            {
                newPhyMode = eCSR_DOT11_MODE_11b;
                eBand = eCSR_BAND_24;
            }
            else
            {
                //We will never be here
                smsLog( pMac, LOGE, FL(" cannot recognize the phy mode 0x%08X\n"), phyMode );
                newPhyMode = eCSR_DOT11_MODE_AUTO;
            }
        }

        //Done validating
        status = eHAL_STATUS_SUCCESS;

        //Now we need to check whether a restart is needed.
        if(eBand != pMac->roam.configParam.eBand)
        {
            fRestartNeeded = eANI_BOOLEAN_TRUE;
            break;
        }
        if(newPhyMode != pMac->roam.configParam.phyMode)
        {
            fRestartNeeded = eANI_BOOLEAN_TRUE;
            break;
        }

    }while(0);

    if(HAL_STATUS_SUCCESS(status))
    {
        pMac->roam.configParam.eBand = eBand;
        pMac->roam.configParam.phyMode = newPhyMode;
        if(pfRestartNeeded)
        {
            *pfRestartNeeded = fRestartNeeded;
        }
    }

    return (status);
}
    

void csrPruneChannelListForMode( tpAniSirGlobal pMac, tCsrChannel *pChannelList )
{
    tANI_U8 Index;
    tANI_U8 cChannels;

    // for dual band NICs, don't need to trim the channel list....
    if ( !CSR_IS_OPEARTING_DUAL_BAND( pMac ) )
    {
        // 2.4 GHz band operation requires the channel list to be trimmed to
        // the 2.4 GHz channels only...
        if ( CSR_IS_24_BAND_ONLY( pMac ) )
        {
            for( Index = 0, cChannels = 0; Index < pChannelList->numChannels;
                 Index++ )
            {
                if ( CSR_IS_CHANNEL_24GHZ(pChannelList->channelList[ Index ]) )
                {
                    pChannelList->channelList[ cChannels ] = pChannelList->channelList[ Index ];
                    cChannels++;
                }
            }

            // Cleanup the rest of channels.   Note we only need to clean up the channels if we had
            // to trim the list.  Calling palZeroMemory() with a 0 size is going to throw asserts on 
            // the debug builds so let's be a bit smarter about that.  Zero out the reset of the channels
            // only if we need to.
            //
            // The amount of memory to clear is the number of channesl that we trimmed 
            // (pChannelList->numChannels - cChannels) times the size of a channel in the structure.
           
            if ( pChannelList->numChannels > cChannels )
            {
                palZeroMemory( pMac->hHdd, &pChannelList->channelList[ cChannels ],
                               sizeof( pChannelList->channelList[ 0 ] ) * ( pChannelList->numChannels - cChannels ) );
                
            }
            
            pChannelList->numChannels = cChannels;
        }
        else if ( CSR_IS_5G_BAND_ONLY( pMac ) )
        {
            for ( Index = 0, cChannels = 0; Index < pChannelList->numChannels; Index++ )
            {
                if ( CSR_IS_CHANNEL_5GHZ(pChannelList->channelList[ Index ]) )
                {
                    pChannelList->channelList[ cChannels ] = pChannelList->channelList[ Index ];
                    cChannels++;
                }
            }

            // Cleanup the rest of channels.   Note we only need to clean up the channels if we had
            // to trim the list.  Calling palZeroMemory() with a 0 size is going to throw asserts on 
            // the debug builds so let's be a bit smarter about that.  Zero out the reset of the channels
            // only if we need to.
            //
            // The amount of memory to clear is the number of channesl that we trimmed 
            // (pChannelList->numChannels - cChannels) times the size of a channel in the structure.
            if ( pChannelList->numChannels > cChannels )
            {
                palZeroMemory( pMac->hHdd, &pChannelList->channelList[ cChannels ],
                               sizeof( pChannelList->channelList[ 0 ] ) * ( pChannelList->numChannels - cChannels ) );
            }            
                               
            pChannelList->numChannels = cChannels;
        }
    }

}


eHalStatus csrInitGetChannels(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirRetStatus sirStatus;
    tANI_U8 Index = 0;
    tANI_U8 num20MHzChannelsFound = 0;
	tANI_U8 num40MHzChannelsFound = 0;

    //TODO: this interface changed to include the 40MHz channel list
    // this needs to be tied into the adapter structure somehow and referenced appropriately for CB operation
    // Read the scan channel list (including the power limit) from EEPROM
    sirStatus = halPhyGetChannelListWithPower( pMac, pMac->scan.defaultPowerTable, &num20MHzChannelsFound, 
                        pMac->scan.defaultPowerTable40MHz, &num40MHzChannelsFound);
    if ( (eSIR_SUCCESS != sirStatus) || (num20MHzChannelsFound == 0) )
    {
        smsLog( pMac, LOGE, FL("failed to get channels \n"));
        status = eHAL_STATUS_FAILURE;
    }
    else
    {
        if ( num20MHzChannelsFound > WNI_CFG_VALID_CHANNEL_LIST_LEN )
        {
            num20MHzChannelsFound = WNI_CFG_VALID_CHANNEL_LIST_LEN;
        }
        pMac->scan.numChannelsDefault = num20MHzChannelsFound;
        // Move the channel list to the global data
        // structure -- this will be used as the scan list
        for ( Index = 0; Index < num20MHzChannelsFound; Index++)
        {
            pMac->scan.base20MHzChannels.channelList[ Index ] = pMac->scan.defaultPowerTable[ Index ].chanId;
        }
        pMac->scan.base20MHzChannels.numChannels = num20MHzChannelsFound;

        if(num40MHzChannelsFound > WNI_CFG_VALID_CHANNEL_LIST_LEN)
        {
            num40MHzChannelsFound = WNI_CFG_VALID_CHANNEL_LIST_LEN;
        }
        for ( Index = 0; Index < num40MHzChannelsFound; Index++)
        {
            pMac->scan.base40MHzChannels.channelList[ Index ] = pMac->scan.defaultPowerTable40MHz[ Index ].chanId;
        }
        pMac->scan.base40MHzChannels.numChannels = num40MHzChannelsFound;
    }

    return (status);  
}


eHalStatus csrInitChannelList( tHalHandle hHal )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    eHalStatus status = eHAL_STATUS_SUCCESS;

    csrPruneChannelListForMode(pMac, &pMac->scan.baseChannels);
    csrPruneChannelListForMode(pMac, &pMac->scan.base20MHzChannels);
    // Apply the base channel list, power info, and set the Country code...
    csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.base20MHzChannels, pMac->scan.countryCodeCurrent );
 
    return (status);
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
eHalStatus csrHoConfigParams(tpAniSirGlobal pMac, tCsrHandoffConfigParams * pCsrHoConfig)
{
   /* get the handoff configurations                                       */

   if(pCsrHoConfig)
   {
      smsLog(pMac, LOG1, " csrHoConfigParams: setting the passed down HO params \n");
      pMac->roam.handoffInfo.handoffParams = *pCsrHoConfig;
      pMac->roam.handoffInfo.idleScanInterval = pMac->roam.configParam.impsSleepTime;
      pMac->roam.handoffInfo.lostLinkRoamInterval = pMac->roam.configParam.nRoamingTime;      
   }
   else
   {
      smsLog(pMac, LOGW, " csrHoConfigParams: nothing passed down, faling back to default \n");
      csrHoConfigDefaultParams(pMac);
   }
   return eHAL_STATUS_SUCCESS;
}


void csrHoConfigDefaultParams(tpAniSirGlobal pMac)
{
   /* get the default handoff configurations                               */
   /* NO_WIFI                                                              */
   pMac->roam.handoffInfo.handoffParams.noWifiParams.activeScanDuration = 120; // seconds

   pMac->roam.handoffInfo.handoffParams.noWifiParams.activeScanInterval =  30; // seconds

   pMac->roam.handoffInfo.handoffParams.noWifiParams.channelScanTime =    120; // msec

   pMac->roam.handoffInfo.handoffParams.noWifiParams.rssiFilterConst =     90; // %

   pMac->roam.handoffInfo.handoffParams.noWifiParams.rssiThresholdAssociationAdd =  80; //dBm

   pMac->roam.handoffInfo.handoffParams.noWifiParams.rssiThresholdNeighborSet =  95; //dBm

   pMac->roam.handoffInfo.idleScanInterval = pMac->roam.configParam.impsSleepTime;

   pMac->roam.handoffInfo.lostLinkRoamInterval = pMac->roam.configParam.nRoamingTime;

   /* No Traffic                                                            */
   pMac->roam.handoffInfo.handoffParams.ntParams.bestCandidateApRssiDelta = 10; //dBm

   pMac->roam.handoffInfo.handoffParams.ntParams.inactPeriod =    2000; //msec

   pMac->roam.handoffInfo.handoffParams.ntParams.inactThreshold = 25; // 25 packets

   pMac->roam.handoffInfo.handoffParams.ntParams.neighborApBgScanInterval = 2000; // msec

   pMac->roam.handoffInfo.handoffParams.ntParams.neighborApIncrBgScanInterval = 4000; //msec

   pMac->roam.handoffInfo.handoffParams.ntParams.numCandtSetEntry = 5;

   pMac->roam.handoffInfo.handoffParams.ntParams.pmkCacheRssiDelta =  3; //dB

   pMac->roam.handoffInfo.handoffParams.ntParams.rssiFilterConst =   90; // %

   pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCandtSet =   75; //dBm

   pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCurrentApGood =  50; // dBm

   /* Non Real Time                                                           */
   pMac->roam.handoffInfo.handoffParams.nrtParams.bestCandidateApRssiDelta = 10; //dBm

   pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanDelayInterval = 1000; // msec

   pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanIncrInterval = 1000; // msec

   pMac->roam.handoffInfo.handoffParams.nrtParams.bgScanInterval = 600; // msec

   pMac->roam.handoffInfo.handoffParams.nrtParams.numCandtSetEntry =   5;

   pMac->roam.handoffInfo.handoffParams.nrtParams.perMsmtInterval =  2000; //msec

   pMac->roam.handoffInfo.handoffParams.nrtParams.perThresholdHoFromCurrentAp =  5; // %

   pMac->roam.handoffInfo.handoffParams.nrtParams.pmkCacheRssiDelta =   3; // dBm

   pMac->roam.handoffInfo.handoffParams.nrtParams.rssiFilterConst =   90;  // %

   pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCandtSet =  75; // dBm

   pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGood =  50; // dBm

   pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGoodEmptyCandtset =  75; // dBm

   pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdHoFromCurrentAp =  80; // dBm

   /* Real Time                                                               */
   pMac->roam.handoffInfo.handoffParams.rtParams.bestCandidateApRssiDelta = 10; //dBm

   pMac->roam.handoffInfo.handoffParams.rtParams.bgScanInterval = 300; // msec

   pMac->roam.handoffInfo.handoffParams.rtParams.numCandtSetEntry =  5;

   pMac->roam.handoffInfo.handoffParams.rtParams.perMsmtInterval =  2000; //msec

   pMac->roam.handoffInfo.handoffParams.rtParams.perThresholdHoFromCurrentAp =  10; // %

   pMac->roam.handoffInfo.handoffParams.rtParams.pmkCacheRssiDelta =  3; // dBm

   pMac->roam.handoffInfo.handoffParams.rtParams.rssiFilterConst =   90;  // %

   pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCandtSet =  75; // dBm

   pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCurrentApGood = 50; //dBm

   pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdHoFromCurrentAp = 80; // dBm

}
#endif

eHalStatus csrChangeConfigParams(tHalHandle hHal, 
                                 tCsrUpdateConfigParam *pUpdateConfigParam)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tCsr11dinfo *ps11dinfo = NULL;

   ps11dinfo = &pUpdateConfigParam->Csr11dinfo;
   status = CsrInit11dInfo(pMac, ps11dinfo);
   return status;
}


static eHalStatus CsrInit11dInfo(tpAniSirGlobal pMac, tCsr11dinfo *ps11dinfo)
{
  eHalStatus status = eHAL_STATUS_FAILURE;
  tANI_U8  index;
  tANI_U32 count=0;
  tSirMacChanInfo *pChanInfo;
  tSirMacChanInfo *pChanInfoStart;

  if(!ps11dinfo)
  {
     return (status);
  }

  if ( ps11dinfo->Channels.numChannels && ( WNI_CFG_VALID_CHANNEL_LIST_LEN >= ps11dinfo->Channels.numChannels ) ) 
  {
    pMac->scan.base20MHzChannels.numChannels = ps11dinfo->Channels.numChannels;
    status = palCopyMemory(pMac->hHdd, pMac->scan.base20MHzChannels.channelList, 
                           ps11dinfo->Channels.channelList, ps11dinfo->Channels.numChannels);
    if(!HAL_STATUS_SUCCESS(status)) return (status);
  }
  else
  {
	 //No change
     return (eHAL_STATUS_SUCCESS);
  }

  //legacy maintenance
  status = palCopyMemory(pMac->hHdd, pMac->scan.countryCodeDefault, 
                         ps11dinfo->countryCode, WNI_CFG_COUNTRY_CODE_LEN);
  if(!HAL_STATUS_SUCCESS(status)) return (status);

  //Tush: at csropen get this initialized with default, during csr reset if this 
  // already set with some value no need initilaize with default again
  if(0 == pMac->scan.countryCodeCurrent[0])
  {
     status = palCopyMemory(pMac->hHdd, pMac->scan.countryCodeCurrent, 
                         ps11dinfo->countryCode, WNI_CFG_COUNTRY_CODE_LEN);
     if(!HAL_STATUS_SUCCESS(status)) return (status);
  }

  // need to add the max power channel list
  if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN)))
  {
      palZeroMemory(pMac->hHdd, pChanInfo, sizeof(tSirMacChanInfo) * WNI_CFG_VALID_CHANNEL_LIST_LEN);
      pChanInfoStart = pChanInfo;

      for(index = 0; index < ps11dinfo->Channels.numChannels; index++)
      {
        pChanInfo->firstChanNum = ps11dinfo->ChnPower[index].firstChannel;
        pChanInfo->numChannels  = ps11dinfo->ChnPower[index].numChannels;
        pChanInfo->maxTxPower   = CSR_ROAM_MIN( ps11dinfo->ChnPower[index].maxtxPower, pMac->roam.configParam.nTxPowerCap );
        pChanInfo++;
        count++;
      }
      if(count)
      {
          csrSaveToChannelPower2G_5G( pMac, count * sizeof(tSirMacChanInfo), pChanInfoStart );
      }
      palFreeMemory(pMac->hHdd, pChanInfoStart);
  }

  //Only apply them to CFG when not in STOP state. Otherwise they will be applied later
  if( HAL_STATUS_SUCCESS(status) && ( !CSR_IS_ROAM_STOP(pMac) ) )
  {
    // Apply the base channel list, power info, and set the Country code...
    csrApplyChannelPowerCountryInfo( pMac, &pMac->scan.base20MHzChannels, pMac->scan.countryCodeCurrent );
  }

  return (status);
}

//pCommand may be NULL
void csrRoamRemoveDuplicateCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrRoamReason eRoamReason)
{
    tListElem *pEntry, *pNextEntry;
    tSmeCmd *pDupCommand;

    csrLLLock( &pMac->sme.smeCmdPendingList );
    pEntry = csrLLPeekHead( &pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK );
    while( pEntry )
    {
        pNextEntry = csrLLNext( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK );
        pDupCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );

        // Remove the previous command if..
        // - the new roam command is for the same RoamReason...
        // - the new roam command is a NewProfileList.
        // - the new roam command is a Forced Dissoc
        // - the new roam command is from an 802.11 OID (OID_SSID or OID_BSSID).
        if ( 
            (pCommand && 
                ((pCommand->command == pDupCommand->command) &&
                 (pCommand->u.roamCmd.roamReason == pDupCommand->u.roamCmd.roamReason ||
                    eCsrForcedDisassoc == pCommand->u.roamCmd.roamReason ||
                    eCsrHddIssued == pCommand->u.roamCmd.roamReason))) 
                ||
            ((eSmeCommandRoam == pDupCommand->command) &&
                 ((eCsrForcedDisassoc == eRoamReason) ||
                    (eCsrHddIssued == eRoamReason))
               )
           )
        {
            tANI_BOOLEAN fRemoveCmd;

            smsLog(pMac, LOGW, FL("   roamReason = %d\n"), pDupCommand->u.roamCmd.roamReason);
            // Remove the 'stale' roam command from the pending list...
            fRemoveCmd = csrLLRemoveEntry( &pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK );

            //Tell caller that the command is cancelled
            csrRoamCallCallback(pMac, NULL, pDupCommand->u.roamCmd.roamId, eCSR_ROAM_CANCELLED, eCSR_ROAM_RESULT_NONE);
            // ... and release the roam command we just removed.
            if( fRemoveCmd )
            {
                csrReleaseCommandRoam(pMac, pDupCommand);
            }
        }
        pEntry = pNextEntry;
    }
    csrLLUnlock( &pMac->sme.smeCmdPendingList );
}


eHalStatus csrRoamCallCallback(tpAniSirGlobal pMac, tCsrRoamInfo *pRoamInfo, tANI_U32 roamId, eRoamCmdStatus u1, eCsrRoamResult u2)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_EVENT_DEF(connectionStatus, vos_event_wlan_status_payload_type);
#endif

    if(eCSR_ROAM_ASSOCIATION_COMPLETION == u1 && pRoamInfo)
    {
        smsLog(pMac, LOGW, " Assoc complete result = %d statusCode = %d reasonCode = %d\n", u2, pRoamInfo->statusCode, pRoamInfo->reasonCode);
    }

    if(NULL != pMac->roam.callback)
    {
        status = pMac->roam.callback(pMac->roam.pContext, pRoamInfo, roamId, u1, u2);
    }

    pMac->roam.lastRoamCallbackStatus = u1;

    //EVENT_WLAN_STATUS: eCSR_ROAM_ASSOCIATION_COMPLETION, 
    //                   eCSR_ROAM_LOSTLINK, eCSR_ROAM_DISASSOCIATED, 
#ifdef FEATURE_WLAN_DIAG_SUPPORT    
    palZeroMemory(pMac->hHdd, &connectionStatus, sizeof(vos_event_wlan_status_payload_type));
    if((eCSR_ROAM_ASSOCIATION_COMPLETION == u1) && (eCSR_ROAM_RESULT_ASSOCIATED == u2))
    {
        connectionStatus.eventId = eCSR_WLAN_STATUS_CONNECT;
        connectionStatus.bssType = pRoamInfo->u.pConnectedProfile->BSSType;
        connectionStatus.rssi = pRoamInfo->pBssDesc->rssi * (-1);
        connectionStatus.channel = pRoamInfo->pBssDesc->channelId;
        connectionStatus.qosCapability = pRoamInfo->u.pConnectedProfile->qosConnection;
        connectionStatus.authType = (v_U8_t)diagAuthTypeFromCSRType(pRoamInfo->u.pConnectedProfile->AuthType);
        connectionStatus.encryptionType = (v_U8_t)diagEncTypeFromCSRType(pRoamInfo->u.pConnectedProfile->EncryptionType);
        palCopyMemory(pMac->hHdd, connectionStatus.ssid, pRoamInfo->u.pConnectedProfile->SSID.ssId, 6);
        connectionStatus.reason = eCSR_REASON_UNSPECIFIED;
        WLAN_VOS_DIAG_EVENT_REPORT(&connectionStatus, EVENT_WLAN_STATUS);
    }

    if((eCSR_ROAM_MIC_ERROR_IND == u1) || (eCSR_ROAM_RESULT_MIC_FAILURE == u2))
    {
       connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
       connectionStatus.reason = eCSR_REASON_MIC_ERROR;
       WLAN_VOS_DIAG_EVENT_REPORT(&connectionStatus, EVENT_WLAN_STATUS);
    }

    if(eCSR_ROAM_RESULT_FORCED == u2)
    {
       connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
       connectionStatus.reason = eCSR_REASON_USER_REQUESTED;
       WLAN_VOS_DIAG_EVENT_REPORT(&connectionStatus, EVENT_WLAN_STATUS);
    }

    if(eCSR_ROAM_RESULT_DISASSOC_IND == u2)
    {
       connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
       connectionStatus.reason = eCSR_REASON_DISASSOC;
       WLAN_VOS_DIAG_EVENT_REPORT(&connectionStatus, EVENT_WLAN_STATUS);
    }

    if(eCSR_ROAM_RESULT_DEAUTH_IND == u2)
    {
       connectionStatus.eventId = eCSR_WLAN_STATUS_DISCONNECT;
       connectionStatus.reason = eCSR_REASON_DEAUTH;
       WLAN_VOS_DIAG_EVENT_REPORT(&connectionStatus, EVENT_WLAN_STATUS);
    }

#endif //FEATURE_WLAN_DIAG_SUPPORT
    
    return (status);
}



eHalStatus csrRoamIssueDisassociate( tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate, tANI_BOOLEAN fMICFailure )
{   
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrBssid bssId = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    tANI_U16 reasonCode;
    
    if ( fMICFailure )
    {
        reasonCode = eSIR_MAC_MIC_FAILURE_REASON;
    }
    else 
    {
        reasonCode = eSIR_MAC_UNSPEC_FAILURE_REASON;
    }    

    if(pMac->roam.pConnectBssDesc)
    {
        palCopyMemory(pMac->hHdd, &bssId, pMac->roam.pConnectBssDesc->bssId, sizeof(tCsrBssid));
    }

    
    smsLog( pMac, LOGE, "CSR Attempting to Disassociate Bssid= %02x-%02x-%02x-%02x-%02x-%02x subState = %d\n", 
                  bssId[ 0 ], bssId[ 1 ], bssId[ 2 ],
                  bssId[ 3 ], bssId[ 4 ], bssId[ 5 ], NewSubstate );    

    csrRoamSubstateChange( pMac, NewSubstate );
    
    status = csrSendMBDisassocReqMsg( pMac, bssId, reasonCode );    
    
    if(HAL_STATUS_SUCCESS(status)) 
    {
#ifdef FEATURE_WLAN_GEN6_ROAMING
       if(eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF == NewSubstate)
       {
          pMac->roam.handoffInfo.handoffAction = TRUE;
       }
#endif
	csrRoamLinkDown(pMac);
	//no need to tell QoS that we are disassociating, it will be taken care off in assoc req for HO
	if(eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF != NewSubstate)
	{
		//Tush-QoS: notify QoS module that disassoc happening
		sme_QosCsrEventInd(pMac, SME_QOS_CSR_DISCONNECT_REQ, NULL);
	}

     }

    return (status);
}


eHalStatus csrRoamIssueDeauth( tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate )
{   
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrBssid bssId = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    if(pMac->roam.pConnectBssDesc)
    {
        palCopyMemory(pMac->hHdd, &bssId, pMac->roam.pConnectBssDesc->bssId, sizeof(tCsrBssid));
    }

    smsLog( pMac, LOG2, "CSR Attempting to Deauth Bssid= %02x-%02x-%02x-%02x-%02x-%02x\n", 
                  bssId[ 0 ], bssId[ 1 ], bssId[ 2 ],
                  bssId[ 3 ], bssId[ 4 ], bssId[ 5 ] );    

    csrRoamSubstateChange( pMac, NewSubstate );
    
    status = csrSendMBDeauthReqMsg( pMac, bssId, eSIR_MAC_DISASSOC_LEAVING_BSS_REASON );    
    
    return (status);
}



eHalStatus csrRoamSaveConnectedBssDesc( tpAniSirGlobal pMac, tSirBssDescription *pBssDesc )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 size = pBssDesc->length + sizeof( pBssDesc->length );
    
    // If no BSS description was found in this connection (happens with start IBSS), then 
    // nix the BSS description that we keep around for the connected BSS) and get out...
    if(NULL == pBssDesc)
    {
        csrFreeConnectBssDesc(pMac);
    }
    else 
    {
        if(NULL != pMac->roam.pConnectBssDesc)
        {
            if(((pMac->roam.pConnectBssDesc->length) + sizeof(pMac->roam.pConnectBssDesc->length)) < size)
            {
                //not enough room for the new BSS, pMac->roam.pConnectBssDesc is freed inside
                csrFreeConnectBssDesc(pMac);
            }
        }
        if(NULL == pMac->roam.pConnectBssDesc)
        {
            status = palAllocateMemory( pMac->hHdd, (void **)&pMac->roam.pConnectBssDesc, size); 
        }
        if ( HAL_STATUS_SUCCESS(status) && pMac->roam.pConnectBssDesc ) 
        {
            palCopyMemory( pMac->hHdd, pMac->roam.pConnectBssDesc, pBssDesc, size );    
        }
    }
    
    return (status);
}


eHalStatus csrRoamPrepareBssConfig(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
                                    tSirBssDescription *pBssDesc, tBssConfigParam *pBssConfig,
                                    tDot11fBeaconIEs *pIes)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eCsrCfgDot11Mode cfgDot11Mode;

#if defined(VOSS_ENABLED)
    VOS_ASSERT( pIes != NULL );
#endif
    
    do
    {
        palCopyMemory(pMac->hHdd, &pBssConfig->BssCap, &pBssDesc->capabilityInfo, sizeof(tSirMacCapabilityInfo));
        //get qos
        pBssConfig->qosType = csrGetQoSFromBssDesc(pMac, pBssDesc, pIes);
        //get SSID
        if(pIes->SSID.present)
        {
            palCopyMemory(pMac->hHdd, &pBssConfig->SSID.ssId, pIes->SSID.ssid, pIes->SSID.num_ssid);
            pBssConfig->SSID.length = pIes->SSID.num_ssid;
        }
        else
            pBssConfig->SSID.length = 0;
        if(csrIsNULLSSID(pBssConfig->SSID.ssId, pBssConfig->SSID.length))
        {
            smsLog(pMac, LOGW, "  BSS desc SSID is a wildcard\n");
            //Return failed if profile doesn't have an SSID either.
            if(pProfile->SSIDs.numOfSSIDs == 0)
            {
                smsLog(pMac, LOGW, "  Both BSS desc and profile doesn't have SSID\n");
                status = eHAL_STATUS_FAILURE;
                break;
            }
        }
        if(CSR_IS_CHANNEL_5GHZ(pBssDesc->channelId))
        {
            pBssConfig->eBand = eCSR_BAND_5G;
        }
        else
        {
            pBssConfig->eBand = eCSR_BAND_24;
        }
        //phymode
        if(csrIsPhyModeMatch( pMac, pProfile->phyMode, pBssDesc, pProfile, &cfgDot11Mode, pIes ))
        {
            pBssConfig->uCfgDot11Mode = cfgDot11Mode;
        }
        else 
        {
            smsLog(pMac, LOGW, "   Can not find match phy mode\n");
            //force it
            if(eCSR_BAND_24 == pBssConfig->eBand)
            {
                pBssConfig->uCfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
            }
            else
            {
                pBssConfig->uCfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
            }
        }
        //auth type
        switch( pProfile->negotiatedAuthType ) 
        {
            default:
            case eCSR_AUTH_TYPE_WPA:
            case eCSR_AUTH_TYPE_WPA_PSK:
            case eCSR_AUTH_TYPE_WPA_NONE:
            case eCSR_AUTH_TYPE_OPEN_SYSTEM:
                pBssConfig->authType = eSIR_OPEN_SYSTEM;
                break;

            case eCSR_AUTH_TYPE_SHARED_KEY:
                pBssConfig->authType = eSIR_SHARED_KEY;
                break;

            case eCSR_AUTH_TYPE_AUTOSWITCH:
                pBssConfig->authType = eSIR_AUTO_SWITCH;
                break;
        }
        //short slot time
        if( eCSR_CFG_DOT11_MODE_11B != cfgDot11Mode )
        {
            pBssConfig->uShortSlotTime = pMac->roam.configParam.shortSlotTime;
        }
        else
        {
            pBssConfig->uShortSlotTime = 0;
        }
        if(pBssConfig->BssCap.ibss)
        {
            //We don't support 11h on IBSS
            pBssConfig->f11hSupport = eANI_BOOLEAN_FALSE; 
        }
        else
        {
            pBssConfig->f11hSupport = pMac->roam.configParam.Is11hSupportEnabled;
        }
        //power constraint
        pBssConfig->uPowerLimit = csrGet11hPowerConstraint(pMac, &pIes->PowerConstraints);
        //heartbeat
        if ( CSR_IS_11A_BSS( pBssDesc ) )
        {
             pBssConfig->uHeartBeatThresh = pMac->roam.configParam.HeartbeatThresh50;        
        }
        else
        {
             pBssConfig->uHeartBeatThresh = pMac->roam.configParam.HeartbeatThresh24;
        }
        //Join timeout
        // if we find a BeaconInterval in the BssDescription, then set the Join Timeout to 
        // be 3 x the BeaconInterval.                          
        if ( pBssDesc->beaconInterval )
        {
            //Make sure it is bigger than the minimal
            pBssConfig->uJoinTimeOut = CSR_ROAM_MAX(3 * pBssDesc->beaconInterval, CSR_JOIN_FAILURE_TIMEOUT_MIN);
        }
        else 
        {
            pBssConfig->uJoinTimeOut = CSR_JOIN_FAILURE_TIMEOUT_DEFAULT;
        }
        //validate CB
        pBssConfig->cbMode = csrGetCBModeFromIes(pMac, pBssDesc->channelId, pIes);
    }while(0);

    return (status);
}


eHalStatus csrRoamPrepareIBSSConfigFromProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tBssConfigParam *pBssConfig)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 operationChannel = 0; 
    //SSID
    pBssConfig->SSID.length = 0;
    if(pProfile->SSIDs.numOfSSIDs)
    {
        //only use the first one
        palCopyMemory(pMac->hHdd, &pBssConfig->SSID, &pProfile->SSIDs.SSIDList[0].SSID, sizeof(tSirMacSSid));
    }
    pBssConfig->eBand = pMac->roam.configParam.eBand;
    //phymode
    if(pProfile->ChannelInfo.ChannelList)
    {
       operationChannel = pProfile->ChannelInfo.ChannelList[0];
    }
    pBssConfig->uCfgDot11Mode = csrRoamGetPhyModeBandForIBSS(pMac, (eCsrPhyMode)pProfile->phyMode, operationChannel, 
                                        &pBssConfig->eBand);
    //QOS
    //Is this correct to always set to this //***
    if( ( eCsrRoamWmmNoQos != pMac->roam.configParam.WMMSupportMode ) ||
        ( ( eCSR_CFG_DOT11_MODE_11N == pBssConfig->uCfgDot11Mode ) ||
             ( eCSR_CFG_DOT11_MODE_TAURUS == pBssConfig->uCfgDot11Mode ) ) //For 11n, need QoS
      )
    {
        pBssConfig->qosType = eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP;
    }
    else
    {
        pBssConfig->qosType = eCSR_MEDIUM_ACCESS_DCF;
    }
    //auth type
    switch( pProfile->AuthType.authType[0] ) //Take the prefered Auth type.
    {
        default:
        case eCSR_AUTH_TYPE_WPA:
        case eCSR_AUTH_TYPE_WPA_PSK:
        case eCSR_AUTH_TYPE_WPA_NONE:
        case eCSR_AUTH_TYPE_OPEN_SYSTEM:
            pBssConfig->authType = eSIR_OPEN_SYSTEM;
            break;

        case eCSR_AUTH_TYPE_SHARED_KEY:
            pBssConfig->authType = eSIR_SHARED_KEY;
            break;

        case eCSR_AUTH_TYPE_AUTOSWITCH:
            pBssConfig->authType = eSIR_AUTO_SWITCH;
            break;
    }
    //short slot time
    if( WNI_CFG_PHY_MODE_11B != pBssConfig->uCfgDot11Mode )
    {
        pBssConfig->uShortSlotTime = pMac->roam.configParam.shortSlotTime;
    }
    else
    {
        pBssConfig->uShortSlotTime = 0;
    }
    //power constraint. We don't support 11h on IBSS
    pBssConfig->f11hSupport = eANI_BOOLEAN_FALSE;
    pBssConfig->uPowerLimit = 0;
    //heartbeat
    if ( eCSR_BAND_5G == pBssConfig->eBand )
    {
        pBssConfig->uHeartBeatThresh = pMac->roam.configParam.HeartbeatThresh50;        
    }
    else
    {
        pBssConfig->uHeartBeatThresh = pMac->roam.configParam.HeartbeatThresh24;
    }
    //Join timeout
    pBssConfig->uJoinTimeOut = CSR_JOIN_FAILURE_TIMEOUT_DEFAULT;
    
    return (status);
}


void csrSetCfgPrivacy( tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tANI_BOOLEAN fPrivacy )
{

    // !! Note:  the only difference between this function and the csrSetCfgPrivacyFromProfile() is the 
    // setting of the privacy CFG based on the advertised privacy setting from the AP for WPA associations. 
    // See !!Note: below in this function...
    tANI_U32 PrivacyEnabled = 0;
    tANI_U32 RsnEnabled = 0;
    tANI_U32 WepDefaultKeyId = 0;
    tANI_U32 WepKeyLength = 0;
    tANI_U32 Key0Length = 0;
    tANI_U32 Key1Length = 0;
    tANI_U32 Key2Length = 0;
    tANI_U32 Key3Length = 0;
    
    // Reserve for the biggest key 
    tANI_U8 Key0[ WNI_CFG_WEP_DEFAULT_KEY_1_LEN ];
    tANI_U8 Key1[ WNI_CFG_WEP_DEFAULT_KEY_2_LEN ];
    tANI_U8 Key2[ WNI_CFG_WEP_DEFAULT_KEY_3_LEN ];
    tANI_U8 Key3[ WNI_CFG_WEP_DEFAULT_KEY_4_LEN ];
    
    switch ( pProfile->negotiatedUCEncryptionType )
    {
        case eCSR_ENCRYPT_TYPE_NONE:
        
            // for NO encryption, turn off Privacy and Rsn.
            PrivacyEnabled = 0;           
            RsnEnabled = 0;
            
            // WEP key length and Wep Default Key ID don't matter in this case....
            
            // clear out the WEP keys that may be hanging around.
            Key0Length = 0;
            Key1Length = 0;
            Key2Length = 0;
            Key3Length = 0;
            
            break;
            
        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
            
            // Privacy is ON.  NO RSN for Wep40 static key.
            PrivacyEnabled = 1;           
            RsnEnabled = 0;
                        
            // Set the Wep default key ID.
            WepDefaultKeyId = pProfile->Keys.defaultIndex;

            // Wep key size if 5 bytes (40 bits).
            WepKeyLength = WNI_CFG_WEP_KEY_LENGTH_5;            
            
            // set encryption keys in the CFG database or clear those that are not present in this profile.
            if ( pProfile->Keys.KeyLength[0] ) 
            {
                palCopyMemory( pMac->hHdd, Key0, pProfile->Keys.KeyMaterial[0], WNI_CFG_WEP_KEY_LENGTH_5 );
                Key0Length = WNI_CFG_WEP_KEY_LENGTH_5;
            }
            else
            {
                Key0Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[1] ) 
            {
                palCopyMemory( pMac->hHdd, Key1, pProfile->Keys.KeyMaterial[1], WNI_CFG_WEP_KEY_LENGTH_5 );
                Key1Length = WNI_CFG_WEP_KEY_LENGTH_5;
            }
            else
            {
                Key1Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[2] ) 
            {
                palCopyMemory( pMac->hHdd, Key2, pProfile->Keys.KeyMaterial[2], WNI_CFG_WEP_KEY_LENGTH_5 );
                Key2Length = WNI_CFG_WEP_KEY_LENGTH_5;                
            }
            else
            {
                Key2Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[3] ) 
            {
                palCopyMemory( pMac->hHdd, Key3, pProfile->Keys.KeyMaterial[3], WNI_CFG_WEP_KEY_LENGTH_5 );
                Key3Length = WNI_CFG_WEP_KEY_LENGTH_5;                
            }
            else
            {
                Key3Length = 0;
            }      

            break;
        
        case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
            
            // Privacy is ON.  NO RSN for Wep40 static key.
            PrivacyEnabled = 1;           
            RsnEnabled = 0;
            
            // Set the Wep default key ID.
            WepDefaultKeyId = pProfile->Keys.defaultIndex;
           
            // Wep key size if 13 bytes (104 bits).
            WepKeyLength = WNI_CFG_WEP_KEY_LENGTH_13;
            
            // set encryption keys in the CFG database or clear those that are not present in this profile.
            if ( pProfile->Keys.KeyLength[0] ) 
            {
                palCopyMemory( pMac->hHdd, Key0, pProfile->Keys.KeyMaterial[ 0 ], WNI_CFG_WEP_KEY_LENGTH_13 );
                Key0Length = WNI_CFG_WEP_KEY_LENGTH_13;
            }
            else
            {
                Key0Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[1] ) 
            {
                palCopyMemory( pMac->hHdd, Key1, pProfile->Keys.KeyMaterial[ 1 ], WNI_CFG_WEP_KEY_LENGTH_13 );
                Key1Length = WNI_CFG_WEP_KEY_LENGTH_13;
            }
            else
            {
                Key1Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[2] ) 
            {
                palCopyMemory( pMac->hHdd, Key2, pProfile->Keys.KeyMaterial[ 2 ], WNI_CFG_WEP_KEY_LENGTH_13 );
                Key2Length = WNI_CFG_WEP_KEY_LENGTH_13;
            }
            else
            {
                Key2Length = 0;
            }
            
            if ( pProfile->Keys.KeyLength[3] ) 
            {
                palCopyMemory( pMac->hHdd, Key3, pProfile->Keys.KeyMaterial[ 3 ], WNI_CFG_WEP_KEY_LENGTH_13 );
                Key3Length = WNI_CFG_WEP_KEY_LENGTH_13;
            }
            else
            {
                Key3Length = 0;
            }
           
            break;
        
        case eCSR_ENCRYPT_TYPE_WEP40:
        case eCSR_ENCRYPT_TYPE_WEP104:
        case eCSR_ENCRYPT_TYPE_TKIP:
        case eCSR_ENCRYPT_TYPE_AES:
            // !! Note:  this is the only difference between this function and the csrSetCfgPrivacyFromProfile()
            // (setting of the privacy CFG based on the advertised privacy setting from the AP for WPA associations ).        
            PrivacyEnabled = (0 != fPrivacy);
                         
            // turn on RSN enabled for WPA associations   
            RsnEnabled = 1;
            
            // WEP key length and Wep Default Key ID don't matter in this case....
            
            // clear out the static WEP keys that may be hanging around.
            Key0Length = 0;
            Key1Length = 0;
            Key2Length = 0;
            Key3Length = 0;        
          
            break;     

        default:
            PrivacyEnabled = 0;
            RsnEnabled = 0;
            break;            
    }           
    
    ccmCfgSetInt(pMac, WNI_CFG_PRIVACY_ENABLED, PrivacyEnabled, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_RSN_ENABLED, RsnEnabled, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_WEP_DEFAULT_KEY_1, Key0, Key0Length, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_WEP_DEFAULT_KEY_2, Key1, Key1Length, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_WEP_DEFAULT_KEY_3, Key2, Key2Length, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_WEP_DEFAULT_KEY_4, Key3, Key3Length, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_WEP_KEY_LENGTH, WepKeyLength, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_WEP_DEFAULT_KEYID, WepDefaultKeyId, NULL, eANI_BOOLEAN_FALSE);
}


static void csrSetCfgSsid( tpAniSirGlobal pMac, tSirMacSSid *pSSID )
{
    tANI_U32 len = 0;
    if(pSSID->length <= WNI_CFG_SSID_LEN)
    {
        len = pSSID->length;
    }
    ccmCfgSetStr(pMac, WNI_CFG_SSID, (tANI_U8 *)pSSID->ssId, len, NULL, eANI_BOOLEAN_FALSE);
}


eHalStatus csrSetQosToCfg( tpAniSirGlobal pMac, eCsrMediaAccessType qosType )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 QoSEnabled;
    tANI_U32 WmeEnabled;

    // set the CFG enable/disable variables based on the qosType being configured...
    switch( qosType )
    {

        case eCSR_MEDIUM_ACCESS_WMM_eDCF_802dot1p:
            QoSEnabled = FALSE;
            WmeEnabled = TRUE;
            break;

        case eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP:
            QoSEnabled = FALSE;
            WmeEnabled = TRUE;
            break;

        case eCSR_MEDIUM_ACCESS_WMM_eDCF_NoClassify:
            QoSEnabled = FALSE;
            WmeEnabled = TRUE;
            break;

        case eCSR_MEDIUM_ACCESS_11e_eDCF:
            QoSEnabled = TRUE;
            WmeEnabled = FALSE;
            break;

        case eCSR_MEDIUM_ACCESS_11e_HCF:
            QoSEnabled = TRUE;
            WmeEnabled = FALSE;
            break;

        default:
        case eCSR_MEDIUM_ACCESS_DCF:
            QoSEnabled = FALSE;
            WmeEnabled = FALSE;
            break;

    }
    //save the WMM setting for later use
    pMac->roam.fWMMConnection = (tANI_BOOLEAN)WmeEnabled;

    status = ccmCfgSetInt(pMac, WNI_CFG_QOS_ENABLED, QoSEnabled, NULL, eANI_BOOLEAN_FALSE);
    status = ccmCfgSetInt(pMac, WNI_CFG_WME_ENABLED, WmeEnabled, NULL, eANI_BOOLEAN_FALSE);

    return (status);
}


    
static void csrSetCfgRateSet( tpAniSirGlobal pMac, eCsrPhyMode phyMode, tCsrRoamProfile *pProfile,
                              tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes)
{
    int i;
    tANI_U8 *pDstRate;
    eCsrCfgDot11Mode cfgDot11Mode;
    tANI_U8 OperationalRates[ CSR_DOT11_SUPPORTED_RATES_MAX ];    // leave enough room for the max number of rates
    tANI_U32 OperationalRatesLength = 0;
    tANI_U8 ExtendedOperationalRates[ CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX ];    // leave enough room for the max number of rates
    tANI_U32 ExtendedOperationalRatesLength = 0;
    tANI_U8 ProprietaryOperationalRates[ 4 ];    // leave enough room for the max number of proprietary rates
    tANI_U32 ProprietaryOperationalRatesLength = 0;
    tANI_U32 PropRatesEnable = 0;

#if defined(VOSS_ENABLED)
    VOS_ASSERT( pIes != NULL );
#endif

    if( NULL != pIes )
    {
        csrIsPhyModeMatch( pMac, phyMode, pBssDesc, pProfile, &cfgDot11Mode, pIes );

        // Originally, we thought that for 11a networks, the 11a rates are always
        // in the Operational Rate set & for 11b and 11g networks, the 11b rates
        // appear in the Operational Rate set.  Consequently, in either case, we
        // would blindly put the rates we support into our Operational Rate set
        // (including the basic rates, which we have already verified are
        // supported earlier in the roaming decision).

        // However, it turns out that this is not always the case.  Some AP's
        // (e.g. D-Link DI-784) ram 11g rates into the Operational Rate set,
        // too.  Now, we're a little more careful:
        pDstRate = OperationalRates;
        if(pIes->SuppRates.present)
        {
            for ( i = 0; i < pIes->SuppRates.num_rates; i++ ) 
            {
                if ( csrRatesIsDot11RateSupported( pMac, pIes->SuppRates.rates[ i ] ) ) 
                {
                    *pDstRate++ = pIes->SuppRates.rates[ i ];
                    OperationalRatesLength++;
                }
            }
        }

        if ( eCSR_CFG_DOT11_MODE_11G == cfgDot11Mode || 
             eCSR_CFG_DOT11_MODE_11N == cfgDot11Mode ||
             eCSR_CFG_DOT11_MODE_TAURUS == cfgDot11Mode ||
             eCSR_CFG_DOT11_MODE_TITAN == cfgDot11Mode ||
             eCSR_CFG_DOT11_MODE_POLARIS == cfgDot11Mode ||
             eCSR_CFG_DOT11_MODE_ABG == cfgDot11Mode )
        {
            // If there are Extended Rates in the beacon, we will reflect those
            // extended rates that we support in out Extended Operational Rate
            // set:
            pDstRate = ExtendedOperationalRates;
            if(pIes->ExtSuppRates.present)
            {
                for ( i = 0; i < pIes->ExtSuppRates.num_rates; i++ ) 
                {
                    if ( csrRatesIsDot11RateSupported( pMac, pIes->ExtSuppRates.rates[ i ] ) ) 
                    {
                        *pDstRate++ = pIes->ExtSuppRates.rates[ i ];
                        ExtendedOperationalRatesLength++;
                    }
                }
            }
        }

        // Enable proprietary MAC features if peer node is Airgo node and STA
        // user wants to use them
        if( pIes->Airgo.present && pMac->roam.configParam.ProprietaryRatesEnabled )
        {
            PropRatesEnable = 1;
        }
        else
        {
            PropRatesEnable = 0;
        }

        // For ANI network companions, we need to populate the proprietary rate
        // set with any proprietary rates we found in the beacon, only if user
        // allows them...
        if ( pMac->roam.configParam.ProprietaryRatesEnabled &&
                pIes->Airgo.present && pIes->Airgo.PropSuppRates.present && pIes->Airgo.PropSuppRates.num_rates ) 
        {
            palCopyMemory( pMac->hHdd, ProprietaryOperationalRates, pIes->Airgo.PropSuppRates.rates, pIes->Airgo.PropSuppRates.num_rates );
            ProprietaryOperationalRatesLength = pIes->Airgo.PropSuppRates.num_rates;

        }
        else {
            // No proprietary modes...
            ProprietaryOperationalRatesLength = 0;
        }

        // Set the operational rate set CFG variables...
        ccmCfgSetStr(pMac, WNI_CFG_OPERATIONAL_RATE_SET, OperationalRates, 
                        OperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
        ccmCfgSetStr(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET, ExtendedOperationalRates, 
                            ExtendedOperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
        ccmCfgSetStr(pMac, WNI_CFG_PROPRIETARY_OPERATIONAL_RATE_SET, 
                        ProprietaryOperationalRates, 
                        ProprietaryOperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
        ccmCfgSetInt(pMac, WNI_CFG_PROPRIETARY_ANI_FEATURES_ENABLED, PropRatesEnable, NULL, eANI_BOOLEAN_FALSE);
        
    }//Parsing BSSDesc
    else
    {
        smsLog(pMac, LOGE, FL("failed to parse BssDesc\n"));
    }
}


static void csrSetCfgRateSetFromProfile( tpAniSirGlobal pMac,
                                         tCsrRoamProfile *pProfile  )
{
    tSirMacRateSetIE DefaultSupportedRates11a = {  SIR_MAC_RATESET_EID, 
                                                   { 8, 
                                                     { SIR_MAC_RATE_6, 
                                                   SIR_MAC_RATE_9, 
                                                   SIR_MAC_RATE_12, 
                                                   SIR_MAC_RATE_18,
                                                   SIR_MAC_RATE_24,
                                                   SIR_MAC_RATE_36,
                                                   SIR_MAC_RATE_48,
                                                       SIR_MAC_RATE_54  } } };

    tSirMacRateSetIE DefaultSupportedRates11b = {  SIR_MAC_RATESET_EID, 
                                                   { 4, 
                                                     { SIR_MAC_RATE_1, 
                                                   SIR_MAC_RATE_2, 
                                                   SIR_MAC_RATE_5_5, 
                                                       SIR_MAC_RATE_11  } } };
                                                              
                                                              
    tSirMacPropRateSet DefaultSupportedPropRates = { 3, 
                                                     { SIR_MAC_RATE_72,
                                                     SIR_MAC_RATE_96,
                                                       SIR_MAC_RATE_108 } };
    eCsrCfgDot11Mode cfgDot11Mode;
    eCsrBand eBand;
    tANI_U8 OperationalRates[ CSR_DOT11_SUPPORTED_RATES_MAX ];    // leave enough room for the max number of rates
    tANI_U32 OperationalRatesLength = 0;
    tANI_U8 ExtendedOperationalRates[ CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX ];    // leave enough room for the max number of rates
    tANI_U32 ExtendedOperationalRatesLength = 0;
    tANI_U8 ProprietaryOperationalRates[ 4 ];    // leave enough room for the max number of proprietary rates
    tANI_U32 ProprietaryOperationalRatesLength = 0;
    tANI_U32 PropRatesEnable = 0;
    tANI_U8 operationChannel = 0; 

    if(pProfile->ChannelInfo.ChannelList)
    {
       operationChannel = pProfile->ChannelInfo.ChannelList[0];
    }
    cfgDot11Mode = csrRoamGetPhyModeBandForIBSS( pMac, (eCsrPhyMode)pProfile->phyMode, operationChannel, &eBand );
    // For 11a networks, the 11a rates go into the Operational Rate set.  For 11b and 11g 
    // networks, the 11b rates appear in the Operational Rate set.  In either case,
    // we can blindly put the rates we support into our Operational Rate set 
    // (including the basic rates, which we have already verified are supported 
    // earlier in the roaming decision).
    if ( eCSR_BAND_5G == eBand ) 
    {       
        // 11a rates into the Operational Rate Set.                 
        OperationalRatesLength = DefaultSupportedRates11a.supportedRateSet.numRates *
                                            sizeof(*DefaultSupportedRates11a.supportedRateSet.rate);
        palCopyMemory( pMac->hHdd, OperationalRates,         
                        DefaultSupportedRates11a.supportedRateSet.rate, 
                        OperationalRatesLength );
                         
        // Nothing in the Extended rate set.
        ExtendedOperationalRatesLength = 0;

        // populate proprietary rates if user allows them
        if ( pMac->roam.configParam.ProprietaryRatesEnabled ) 
        {
            ProprietaryOperationalRatesLength = DefaultSupportedPropRates.numPropRates * 
                                                            sizeof(*DefaultSupportedPropRates.propRate);         
            palCopyMemory( pMac->hHdd, ProprietaryOperationalRates,
                            DefaultSupportedPropRates.propRate, 
                            ProprietaryOperationalRatesLength );
        }    
    	else 
        {       
        	// No proprietary modes
	        ProprietaryOperationalRatesLength = 0;         
    	}    
    }    
    else if ( eCSR_CFG_DOT11_MODE_11B == cfgDot11Mode ) 
    {       
        // 11b rates into the Operational Rate Set.         
        OperationalRatesLength = DefaultSupportedRates11b.supportedRateSet.numRates *
                                              sizeof(*DefaultSupportedRates11b.supportedRateSet.rate);
        palCopyMemory( pMac->hHdd, OperationalRates, 
                        DefaultSupportedRates11b.supportedRateSet.rate, 
                        OperationalRatesLength );
        // Nothing in the Extended rate set.
        ExtendedOperationalRatesLength = 0;
        // No proprietary modes
        ProprietaryOperationalRatesLength = 0;
    }    
    else 
    {       
        // 11G
        
        // 11b rates into the Operational Rate Set.         
        OperationalRatesLength = DefaultSupportedRates11b.supportedRateSet.numRates * 
                                            sizeof(*DefaultSupportedRates11b.supportedRateSet.rate);
        palCopyMemory( pMac->hHdd, OperationalRates, 
                        DefaultSupportedRates11b.supportedRateSet.rate, 
                        OperationalRatesLength );
        
        // 11a rates go in the Extended rate set.
        ExtendedOperationalRatesLength = DefaultSupportedRates11a.supportedRateSet.numRates * 
                                                    sizeof(*DefaultSupportedRates11a.supportedRateSet.rate);
        palCopyMemory( pMac->hHdd, ExtendedOperationalRates,         
                        DefaultSupportedRates11a.supportedRateSet.rate, 
                        ExtendedOperationalRatesLength );
        
        // populate proprietary rates if user allows them
        if ( pMac->roam.configParam.ProprietaryRatesEnabled ) 
        {
            ProprietaryOperationalRatesLength = DefaultSupportedPropRates.numPropRates *
                                                            sizeof(*DefaultSupportedPropRates.propRate);         
            palCopyMemory( pMac->hHdd, ProprietaryOperationalRates, 
                            DefaultSupportedPropRates.propRate, 
                            ProprietaryOperationalRatesLength );
        }  
    	else 
        {       
        	// No proprietary modes
	        ProprietaryOperationalRatesLength = 0;         
    	}    
    }  

    // set this to 1 if prop. rates need to be advertised in to the IBSS beacon and user wants to use them
    if ( ProprietaryOperationalRatesLength && pMac->roam.configParam.ProprietaryRatesEnabled ) 
    {
        PropRatesEnable = 1;                
    }
    else 
    {
        PropRatesEnable = 0;    
    }
        
    // Set the operational rate set CFG variables...
    ccmCfgSetStr(pMac, WNI_CFG_OPERATIONAL_RATE_SET, OperationalRates, 
                    OperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET, ExtendedOperationalRates, 
                        ExtendedOperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetStr(pMac, WNI_CFG_PROPRIETARY_OPERATIONAL_RATE_SET, 
                    ProprietaryOperationalRates, 
                    ProprietaryOperationalRatesLength, NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_PROPRIETARY_ANI_FEATURES_ENABLED, PropRatesEnable, NULL, eANI_BOOLEAN_FALSE);

}

void csrRoamCcmCfgSetCallback(tHalHandle hHal, tANI_S32 result)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if(CSR_IS_ROAM_JOINING(pMac) && CSR_IS_ROAM_SUBSTATE_CONFIG(pMac))
    {
        csrRoamingStateConfigCnfProcessor(pMac, (tANI_U32)result);
    }
}


//This function is very dump. It is here because PE still need WNI_CFG_PHY_MODE
tANI_U32 csrRoamGetPhyModeFromDot11Mode(eCsrCfgDot11Mode dot11Mode, eCsrBand band)
{
    if(eCSR_CFG_DOT11_MODE_11B == dot11Mode)
    {
        return (WNI_CFG_PHY_MODE_11B);
    }
    else
    {
        if(eCSR_BAND_24 == band)
            return (WNI_CFG_PHY_MODE_11G);
    }

    return (WNI_CFG_PHY_MODE_11A);
}

        
//pIes may be NULL
eHalStatus csrRoamSetBssConfigCfg(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile,
                          tSirBssDescription *pBssDesc, tBssConfigParam *pBssConfig,
                          tDot11fBeaconIEs *pIes)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    //Make sure we have the domain info for the BSS we try to connect to.
    //Do we need to worry about sequence for OSs that are not Windows??
    if(pBssDesc)
    {
        if(csrLearnCountryInformation(pMac, pBssDesc, pIes))
        {
            //Make sure the 11d info from this BSSDesc can be applied
            pMac->scan.fAmbiguous11dInfoFound = eANI_BOOLEAN_FALSE;
            csrApplyCountryInformation( pMac, TRUE );
        }
    }
        
    //Qos
    csrSetQosToCfg( pMac, pBssConfig->qosType );
    //SSID
    csrSetCfgSsid(pMac, &pBssConfig->SSID );
    //fragment threshold
    //ccmCfgSetInt(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, csrGetFragThresh(pMac), NULL, eANI_BOOLEAN_FALSE);
    //RTS threshold
    //ccmCfgSetInt(pMac, WNI_CFG_RTS_THRESHOLD, csrGetRTSThresh(pMac), NULL, eANI_BOOLEAN_FALSE);
    //Phymode
    //*** do we need to set both???
    ccmCfgSetInt(pMac, WNI_CFG_PHY_MODE, csrRoamGetPhyModeFromDot11Mode(pBssConfig->uCfgDot11Mode, pBssConfig->eBand), NULL, eANI_BOOLEAN_FALSE);
    ccmCfgSetInt(pMac, WNI_CFG_DOT11_MODE, csrTranslateToWNICfgDot11Mode(pMac, pBssConfig->uCfgDot11Mode), NULL, eANI_BOOLEAN_FALSE);
        
    //Auth type
    ccmCfgSetInt(pMac, WNI_CFG_AUTHENTICATION_TYPE, pBssConfig->authType, NULL, eANI_BOOLEAN_FALSE);
    //encryption type
    csrSetCfgPrivacy(pMac, pProfile, (tANI_BOOLEAN)pBssConfig->BssCap.privacy );
    //short slot time
    ccmCfgSetInt(pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED, pBssConfig->uShortSlotTime, NULL, eANI_BOOLEAN_FALSE);
    //11d
    /*ccmCfgSetInt(pMac, WNI_CFG_11D_ENABLED,
                        ((pBssConfig->f11hSupport) ? pBssConfig->f11hSupport : pMac->roam.configParam.Is11dSupportEnabled), 
                        NULL, eANI_BOOLEAN_FALSE);
    //11h
    ccmCfgSetInt(pMac, WNI_CFG_11H_ENABLED, pMac->roam.configParam.Is11hSupportEnabled, NULL, eANI_BOOLEAN_FALSE);
    */
    ccmCfgSetInt(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, pBssConfig->uPowerLimit, NULL, eANI_BOOLEAN_FALSE);
    //CB
    //For now, we need to make the CB setting consistent between the CFG and the info in START_BSS_REQ
    /*if(csrIsBssTypeIBSS(pProfile->BSSType))
    {
        if(eANI_CB_SECONDARY_NONE == pMac->roam.IbssParams.cbMode)
        {
            cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
        }
        else
        {
            cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
        }
    }
    else
    {
        if(0 == pMac->roam.configParam.ChannelBondingMode)
        {
            cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
        }
        else
        {
            if(eANI_CB_SECONDARY_NONE != pBssConfig->cbMode)
            {
                cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
            }
            else
            {
                cfgCb = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
            }
        }
    }
    ccmCfgSetInt(pMac, WNI_CFG_CHANNEL_BONDING_MODE, cfgCb, NULL, eANI_BOOLEAN_FALSE);

    //Heartbeat
    ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pBssConfig->uHeartBeatThresh, NULL, eANI_BOOLEAN_FALSE);
    */
    //Rate
    //Fixed Rate
    if(pBssDesc)
    {
        csrSetCfgRateSet(pMac, (eCsrPhyMode)pProfile->phyMode, pProfile, pBssDesc, pIes);
    }
    else
    {
        csrSetCfgRateSetFromProfile(pMac, pProfile);
    }
    //Make this the last CFG to set. The callback will trigger a join_req
    //Join time out
    csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_CONFIG );
    ccmCfgSetInt(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT, pBssConfig->uJoinTimeOut, csrRoamCcmCfgSetCallback, eANI_BOOLEAN_FALSE);

    return (status);
}



eHalStatus csrRoamStopNetwork( tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tSirBssDescription *pBssDesc,
                               tDot11fBeaconIEs *pIes)
{
    eHalStatus status;
    tBssConfigParam *pBssConfig;

    status = palAllocateMemory(pMac->hHdd, (void **)&pBssConfig, sizeof(tBssConfigParam)); 
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pBssConfig, sizeof(tBssConfigParam));
        status = csrRoamPrepareBssConfig(pMac, pProfile, pBssDesc, pBssConfig, pIes);
        if(HAL_STATUS_SUCCESS(status))
        {
            //For IBSS, we need to prepare some more information
            if(csrIsBssTypeIBSS(pProfile->BSSType))
            {
                csrRoamPrepareIbssParams(pMac, pProfile, pBssDesc, pIes);
            }
            // If we are in an IBSS, then stop the IBSS...
            if ( csrIsConnStateIbss( pMac ) ) 
            {
                status = csrRoamIssueStopBss( pMac, eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING );
            }
            else 
            {
                // if we are in an Infrastructure association....
                if ( csrIsConnStateInfra( pMac ) ) 
                {
                    // and the new Bss is an Ibss OR we are roaming from Infra to Infra
                    // across SSIDs (roaming to a new SSID)...            //            
            
                    if ( pBssDesc && ( ( csrIsIbssBssDesc( pBssDesc ) ) ||
                          !csrIsSsidEqual( pMac, pMac->roam.pConnectBssDesc, pBssDesc, pIes ) ) )   
                    {
                        // then we need to disassociate from the Infrastructure network...
                        status = csrRoamIssueDisassociate( pMac, eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING, FALSE );  
                    }
                    else 
                    {  
                        // In an Infrastucture and going to an Infrastructure network with the same SSID.  This
                        // calls for a Reassociation sequence.  So issue the CFG sets for this new AP.
                        if ( pBssDesc ) 
                        {
                            // Set parameters for this Bss.    
                            status = csrRoamSetBssConfigCfg(pMac, pProfile, pBssDesc, pBssConfig, pIes);
                        }
                    }      
                }
                else 
                {
                    // Neiher in IBSS nor in Infra.  We can go ahead and set the CFG for tne new network...
                    // Nothing to stop.
                    if ( pBssDesc ) 
                    {      
                        // Set parameters for this Bss.    
                        status = csrRoamSetBssConfigCfg(pMac, pProfile, pBssDesc, pBssConfig, pIes);
                    }  
                }
            }
        }//Success getting BSS config info
        
        palFreeMemory(pMac->hHdd, pBssConfig);
    }//Allocate memory
    
    return (status);
}


eCsrJoinState csrRoamJoin( tpAniSirGlobal pMac, tCsrScanResultInfo *pScanResult, tCsrRoamProfile *pProfile )
{
    eCsrJoinState eRoamState = eCsrContinueRoaming;
    eHalStatus status;
    tSirBssDescription *pBssDesc = &pScanResult->BssDescriptor;
    tDot11fBeaconIEs *pIes = (tDot11fBeaconIEs *)( pScanResult->pvIes ); //This may be NULL

    if ( csrIsInfraBssDesc( pBssDesc ) ) 
    {
        // If we are connected in infrastructure mode and the Join Bss description is for the same BssID, then we are
        // attempting to join the AP we are already connected with.  In that case, see if the Bss or Sta capabilities
        // have changed and handle the changes (without disturbing the current association).
                
        if ( csrIsConnStateConnectedInfra(pMac) && 
             csrIsBssIdEqual( pMac, pBssDesc, pMac->roam.pConnectBssDesc ) &&
             csrIsSsidEqual( pMac, pMac->roam.pConnectBssDesc, pBssDesc, pIes )
           )               
        {   
            // Check to see if the Auth type has changed in the Profile.  If so, we don't want to Reassociate
            // with Authenticating first.  To force this, stop the current association (Disassociate) and 
            // then re 'Join' the AP, wihch will force an Authentication (with the new Auth type) followed by 
            // a new Association.
            if(csrIsSameProfile(pMac, &pMac->roam.connectedProfile, pProfile))
            {
                smsLog(pMac, LOGW, FL("  detect same porfile authType = %d encryType = %d\n"), pProfile->AuthType, pProfile->EncryptionType);
                if(csrRoamIsSameProfileKeys(pMac, &pMac->roam.connectedProfile, pProfile))
                {
                    eRoamState = eCsrReassocToSelfNoCapChange;
                }
                else
                {
                    tBssConfigParam bssConfig;

                    //The key changes
                    palZeroMemory(pMac->hHdd, &bssConfig, sizeof(bssConfig));
                    status = csrRoamPrepareBssConfig(pMac, pProfile, pBssDesc, &bssConfig, pIes);
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        //Reapply the config including Keys so reassoc is happening.
                        status = csrRoamSetBssConfigCfg(pMac, pProfile, pBssDesc, &bssConfig, pIes);
                        if(!HAL_STATUS_SUCCESS(status))
                        {
                            eRoamState = eCsrStopRoaming;
                        }
                    }
                    else
                    {
                        eRoamState = eCsrStopRoaming;
                    }
                }
            }
            else
            {
                if(!HAL_STATUS_SUCCESS(csrRoamIssueDisassociate( pMac, eCSR_ROAM_SUBSTATE_DISASSOC_REQ, FALSE )))
                {
                    smsLog(pMac, LOGW, FL("  fail to issue disassociate\n"));
                    eRoamState = eCsrStopRoaming;
                }                       
            }
        }            
        else 
        {
            // note:  we used to pre-auth here with open authentication networks but that was not working so well.
            // we had a lot of join timeouts when testing at Samsung.  removing this step helped associations 
            // work much better.
            //
            //
            // stop the existing network before attempting to join the new network...
            if(!HAL_STATUS_SUCCESS(csrRoamStopNetwork(pMac, pProfile, pBssDesc, pIes)))
            {
                eRoamState = eCsrStopRoaming;
            }
        }
    }
    else 
    {
        if(!HAL_STATUS_SUCCESS(csrRoamStopNetwork(pMac, pProfile, pBssDesc, pIes)))
        {
            eRoamState = eCsrStopRoaming;
        }
    }

    return( eRoamState );
}


eHalStatus csrRoamShouldRoam(tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, tANI_U32 roamId)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamInfo roamInfo;

    palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
    roamInfo.pBssDesc = pBssDesc;
    status = csrRoamCallCallback(pMac, &roamInfo, roamId, eCSR_ROAM_SHOULD_ROAM, eCSR_ROAM_RESULT_NONE);
    return (status);
}


static eCsrJoinState csrRoamJoinNextBss( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fUseSameBss )
{
    tCsrScanResult *pScanResult = NULL;
    eCsrJoinState eRoamState = eCsrStopRoaming;
    tScanResultList *pBSSList = (tScanResultList *)pCommand->u.roamCmd.hBSSList;
    tANI_BOOLEAN fDone = eANI_BOOLEAN_FALSE;
    tCsrRoamInfo roamInfo, *pRoamInfo = NULL;
    v_U8_t acm_mask = 0; 
    
    do  
    {
        // Check for Cardbus eject condition, before trying to Roam to any BSS
        //***if( !balIsCardPresent(pAdapter) ) break;
        
        if(NULL != pBSSList)
        {
            if(eANI_BOOLEAN_FALSE == fUseSameBss)
            {
                if(pCommand->u.roamCmd.pRoamBssEntry == NULL)
                {
                    //Try the first BSS
                    pCommand->u.roamCmd.pLastRoamBss = NULL;
                    pCommand->u.roamCmd.pRoamBssEntry = csrLLPeekHead(&pBSSList->List, LL_ACCESS_LOCK);
                }
                else
                {
                    pCommand->u.roamCmd.pRoamBssEntry = csrLLNext(&pBSSList->List, pCommand->u.roamCmd.pRoamBssEntry, LL_ACCESS_LOCK);
                    if(NULL == pCommand->u.roamCmd.pRoamBssEntry)
                    {
                        //Done with all the BSSs
                        //In this case, csrRoamProcessResults will tell HDD the completion
                        break;
                    }
                    else
                    {
                        //We need to indicate to HDD that we are done with this one.
                        palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                        roamInfo.pBssDesc = pCommand->u.roamCmd.pLastRoamBss;     //this shall not be NULL
                        roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
                        roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                        pRoamInfo = &roamInfo;
                    }
                }
                while(pCommand->u.roamCmd.pRoamBssEntry)
                {
                    pScanResult = GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry, tCsrScanResult, Link);
                    if(HAL_STATUS_SUCCESS(csrRoamShouldRoam(pMac, &pScanResult->Result.BssDescriptor, pCommand->u.roamCmd.roamId)))
                    {
                        //Ok to roam this
                        break;
                    }
                    pCommand->u.roamCmd.pRoamBssEntry = csrLLNext(&pBSSList->List, pCommand->u.roamCmd.pRoamBssEntry, LL_ACCESS_LOCK);
                    if(NULL == pCommand->u.roamCmd.pRoamBssEntry)
                    {
                        //Done with all the BSSs
                        fDone = eANI_BOOLEAN_TRUE;
                        break;
                    }
                }
                if(fDone)
                {
                    break;
                }
            }
        }
        //We have something to roam, tell HDD when it is not IBSS.
        //For IBSS, the indication goes back to HDD via eCSR_ROAM_IBSS_IND
        if(!csrIsBssTypeIBSS(pCommand->u.roamCmd.roamProfile.BSSType))
        {
            if(pRoamInfo)
            {
                //Complete the last association attemp because a new one is about to be tried
                csrRoamCallCallback(pMac, pRoamInfo, pCommand->u.roamCmd.roamId, 
                                        eCSR_ROAM_ASSOCIATION_COMPLETION, 
                                        eCSR_ROAM_RESULT_NOT_ASSOCIATED);
            }
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(roamInfo));
            if(pScanResult)
            {
                roamInfo.pBssDesc = &pScanResult->Result.BssDescriptor;
                pCommand->u.roamCmd.pLastRoamBss = roamInfo.pBssDesc;
                acm_mask = sme_QosGetACMMask(pMac, &pScanResult->Result.BssDescriptor, 
                     (tDot11fBeaconIEs *)( pScanResult->Result.pvIes ));
            }
            pCommand->u.roamCmd.roamProfile.uapsd_mask &= ~(acm_mask);
            roamInfo.pProfile = &pCommand->u.roamCmd.roamProfile;
            csrRoamCallCallback( pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                 eCSR_ROAM_ASSOCIATION_START, eCSR_ROAM_RESULT_NONE );
        }

        if ( NULL == pCommand->u.roamCmd.pRoamBssEntry ) 
        {
            // If this is a start IBSS profile, then we need to start the IBSS.
            if ( CSR_IS_START_IBSS(&pCommand->u.roamCmd.roamProfile) ) 
            {
                eHalStatus status;
                tANI_BOOLEAN fSameIbss = eANI_BOOLEAN_FALSE;
                // Attempt to start this IBSS...
                //Need to get all negotiated types in place first
                //auth type
                switch( pCommand->u.roamCmd.roamProfile.AuthType.authType[0] ) //Take the prefered Auth type.
                {
	                default:
	                case eCSR_AUTH_TYPE_WPA:
	                case eCSR_AUTH_TYPE_WPA_PSK:
	                case eCSR_AUTH_TYPE_WPA_NONE:
	                case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		                 pCommand->u.roamCmd.roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		                 break;

	                case eCSR_AUTH_TYPE_SHARED_KEY:
		                 pCommand->u.roamCmd.roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_SHARED_KEY;
		                 break;

	                case eCSR_AUTH_TYPE_AUTOSWITCH:
		                 pCommand->u.roamCmd.roamProfile.negotiatedAuthType = eCSR_AUTH_TYPE_AUTOSWITCH;
		                 break;
                }
                pCommand->u.roamCmd.roamProfile.negotiatedUCEncryptionType = 
                pCommand->u.roamCmd.roamProfile.EncryptionType.encryptionType[0]; 
                //In this case, the multicast encryption needs to follow the uncast ones.
                pCommand->u.roamCmd.roamProfile.negotiatedMCEncryptionType = 
                pCommand->u.roamCmd.roamProfile.EncryptionType.encryptionType[0];
                status = csrRoamStartIbss( pMac, &pCommand->u.roamCmd.roamProfile, NULL, &fSameIbss );
                if(HAL_STATUS_SUCCESS(status))
                {
                    if ( fSameIbss ) 
                    {
                        eRoamState = eCsrStartIbssSameIbss;
                    }
                    else
                    {
                        eRoamState = eCsrContinueRoaming;
                    }
                }
                else
                {
                    //it somehow fail need to stop
                    eRoamState = eCsrStopRoaming;
                }
                break;
            }
            else 
            {
                //Nothing we can do
                smsLog(pMac, LOGW, FL("cannot continue without BSS list\n"));
                eRoamState = eCsrStopRoaming;
                break;
            }
        } 
        else 
        {
            //Need to assign these value because they are used in csrIsSameProfile
            pScanResult = GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry, tCsrScanResult, Link);
            pCommand->u.roamCmd.roamProfile.negotiatedUCEncryptionType = pScanResult->ucEncryptionType; //Negotiated while building scan result.
            pCommand->u.roamCmd.roamProfile.negotiatedMCEncryptionType = pScanResult->mcEncryptionType;
            pCommand->u.roamCmd.roamProfile.negotiatedAuthType = pScanResult->authType;
            if ( CSR_IS_START_IBSS(&pCommand->u.roamCmd.roamProfile) )
            {
                if(csrIsSameProfile(pMac, &pMac->roam.connectedProfile, &pCommand->u.roamCmd.roamProfile))
                {
                    eRoamState = eCsrStartIbssSameIbss;
                    break;
                } 
            }
            if( pCommand->u.roamCmd.fReassocToSelfNoCapChange )
            {
                //trying to connect to the one already connected
                pCommand->u.roamCmd.fReassocToSelfNoCapChange = eANI_BOOLEAN_FALSE;
                eRoamState = eCsrReassocToSelfNoCapChange;
                break;
            }
            // Attempt to Join this Bss...
            eRoamState = csrRoamJoin( pMac, &pScanResult->Result, &pCommand->u.roamCmd.roamProfile );
            break;
        }
        
    } while( 0 );

    return( eRoamState );
}


static eHalStatus csrRoam( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eCsrJoinState RoamState;
    
    smsLog(pMac, LOG2, FL("is called\n"));
    //***if( hddIsRadioStateOn( pAdapter ) )
    {
        // Attept to join a Bss...
        RoamState = csrRoamJoinNextBss( pMac, pCommand, eANI_BOOLEAN_FALSE );
    
		// if nothing to join..
		if ( eCsrStopRoaming == RoamState ) 
		{
            tANI_BOOLEAN fComplete = eANI_BOOLEAN_FALSE;

			// and if connected in Infrastructure mode...
			if ( csrIsConnStateInfra(pMac) ) 
			{
				//... then we need to issue a disassociation
				status = csrRoamIssueDisassociate( pMac, eCSR_ROAM_SUBSTATE_DISASSOC_NOTHING_TO_JOIN, FALSE );
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGW, FL("  failed to issue disassociate, status = %d\n"), status);
                    //roam command is completed by caller in the failed case
                    fComplete = eANI_BOOLEAN_TRUE;
                }
			}
			else if( csrIsConnStateIbss(pMac) )
			{
				status = csrRoamIssueStopBss( pMac, eCSR_ROAM_SUBSTATE_STOP_BSS_REQ );
                if(!HAL_STATUS_SUCCESS(status))
                {
                    smsLog(pMac, LOGW, FL("  failed to issue stop bss, status = %d\n"), status);
                    //roam command is completed by caller in the failed case
                    fComplete = eANI_BOOLEAN_TRUE;
                }
			}
			else
			{        
                fComplete = eANI_BOOLEAN_TRUE;
            }
            if(fComplete)
            {
				// ... otherwise, we can complete the Roam command here.
				csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
			}    
		}
		else if ( eCsrReassocToSelfNoCapChange == RoamState )
		{
			csrRoamComplete( pMac, eCsrSilentlyStopRoamingSaveState, NULL );
		}
		else if ( eCsrStartIbssSameIbss == RoamState )
		{
			csrRoamComplete( pMac, eCsrSilentlyStopRoaming, NULL );        
		}
	}//hddIsRadioStateOn
    
    return status;
}


eHalStatus csrRoamProcessCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamInfo roamInfo;
    
    switch ( pCommand->u.roamCmd.roamReason )
    {
    case eCsrForcedDisassoc:
        csrFreeRoamProfile(pMac);
        status = csrRoamProcessDisassociate( pMac, pCommand, FALSE );
        break;

	case eCsrSmeIssuedDisassocForHandoff:
        //Not to free pMac->roam.pCurRoamProfile (via csrFreeRoamProfile) because it is needed after disconnect
        status = csrRoamProcessDisassociate( pMac, pCommand, FALSE );
        break;

    case eCsrForcedDisassocMICFailure:
        csrFreeRoamProfile(pMac);
        status = csrRoamProcessDisassociate( pMac, pCommand, TRUE );
        break;

    case eCsrForcedDeauth:
        csrFreeRoamProfile(pMac);
        status = csrRoamProcessDeauth( pMac, pCommand );
        break;

    case eCsrHddIssuedReassocToSameAP:
    case eCsrSmeIssuedReassocToSameAP:
    {
        tDot11fBeaconIEs Ies;

        roamInfo.pBssDesc = pMac->roam.pConnectBssDesc;
        if( roamInfo.pBssDesc )
        {
            palZeroMemory(pMac->hHdd, (void *)&Ies, sizeof(tDot11fBeaconIEs));
            status = csrParseBssDescriptionIEs(pMac, roamInfo.pBssDesc, &Ies);
            if( HAL_STATUS_SUCCESS( status ) )
            {
                roamInfo.pProfile = &pCommand->u.roamCmd.roamProfile;
                csrRoamCallCallback( pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                     eCSR_ROAM_ASSOCIATION_START, eCSR_ROAM_RESULT_NONE );
 
                csrRoamIssueReassociate( pMac, pMac->roam.pConnectBssDesc, &Ies,
                                         &pCommand->u.roamCmd.roamProfile );
            }
        }
        break;
    }

    default:
        csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINING );

        if( pCommand->u.roamCmd.fUpdateCurRoamProfile )
        {
            //Remember the roaming profile 
            csrFreeRoamProfile(pMac);
            if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pMac->roam.pCurRoamProfile, sizeof(tCsrRoamProfile))))
            {
                palZeroMemory(pMac->hHdd, pMac->roam.pCurRoamProfile, sizeof(tCsrRoamProfile));
                csrRoamCopyProfile(pMac, pMac->roam.pCurRoamProfile, &pCommand->u.roamCmd.roamProfile);
            }
        }

        //At this point, original uapsd_mask is saved in pCurRoamProfile
        //uapsd_mask in the pCommand may change from this point on.
 
        // Attempt to roam with the new scan results (if we need to..)
        status = csrRoam( pMac, pCommand );
        break;
    }

    return (status);
}


void csrReinitRoamCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand) 
{
    if(pCommand->u.roamCmd.fReleaseBssList)
    {
        csrScanResultPurge(pMac, pCommand->u.roamCmd.hBSSList);
        pCommand->u.roamCmd.fReleaseBssList = eANI_BOOLEAN_FALSE;
        pCommand->u.roamCmd.hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
    }
    if(pCommand->u.roamCmd.fReleaseProfile)
    {
        csrReleaseProfile(pMac, &pCommand->u.roamCmd.roamProfile);
        pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_FALSE;
    }
    pCommand->u.roamCmd.pRoamBssEntry = NULL;
    //Because u.roamCmd is union and share with scanCmd and StatusChange
    palZeroMemory(pMac->hHdd, &pCommand->u.roamCmd, sizeof(tRoamCmd));
}


void csrReinitWmStatusChangeCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
    palZeroMemory(pMac->hHdd, &pCommand->u.wmStatusChangeCmd, sizeof(tWmStatusChangeCmd));
}

void csrRoamComplete( tpAniSirGlobal pMac, eCsrRoamCompleteResult Result, void *Context )
{
    tListElem *pEntry;
    tSmeCmd *pCommand;

    smsLog( pMac, LOG2, "roamQ: Roam Completion ...\n" );

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );

        // If the head of the queue is Active and it is a ROAM command, remove
        // and put this on the Free queue.
        if ( eSmeCommandRoam == pCommand->command )
        {
            //we need to process the result first before removing it from active list because state changes 
            //still happening insides roamQProcessRoamResults so no other roam command should be issued
            csrRoamProcessResults( pMac, pCommand, Result, Context );
            
            if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
            {
                csrReleaseCommandRoam( pMac, pCommand );
            }
            else
            {
                smsLog( pMac, LOGE, " **********csrRoamComplete fail to release command reason %d\n",
                    pCommand->u.roamCmd.roamReason );
            }
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: Roam Completion called but ROAM command is not ACTIVE ...\n" );
        }
    }
    else
    {
        smsLog( pMac, LOGW, "CSR: Roam Completion called but NO commands are ACTIVE ...\n" );
    }

    smeProcessPendingQueue( pMac );
}


void csrResetPMKIDCandidateList( tpAniSirGlobal pMac )
{
    palZeroMemory( pMac->hHdd, &(pMac->scan.PmkidCandidateInfo[0]), sizeof(tPmkidCandidateInfo) * CSR_MAX_PMKID_ALLOWED );
    pMac->scan.NumPmkidCandidate = 0;
}

extern tANI_U8 csrWpaOui[][ CSR_WPA_OUI_SIZE ];

static eHalStatus csrRoamSaveWpaRsnRspIE(tpAniSirGlobal pMac, eCsrAuthType authType, 
                                         tSirBssDescription *pSirBssDesc,
                                         tDot11fBeaconIEs *pIes)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if((eCSR_AUTH_TYPE_WPA == authType) ||
        (eCSR_AUTH_TYPE_WPA_PSK == authType) ||
        (eCSR_AUTH_TYPE_RSN == authType) ||
        (eCSR_AUTH_TYPE_RSN_PSK == authType)
        )
    {

        if( pIes )
        {
            tANI_U32 nIeLen;
            tANI_U8 *pIeBuf;

            if((eCSR_AUTH_TYPE_RSN == authType) ||
                (eCSR_AUTH_TYPE_RSN_PSK == authType))
            {
                if(pIes->RSN.present)
                {
                    //Calculate the actual length
                    nIeLen = 8 //version + gp_cipher_suite + pwise_cipher_suite_count
                        + pIes->RSN.pwise_cipher_suite_count * 4    //pwise_cipher_suites
                        + 2 //akm_suite_count
                        + pIes->RSN.akm_suite_count * 4 //akm_suites
                        + 2; //reserved
					if( pIes->RSN.pmkid_count )
                    {
                        nIeLen += 2 + pIes->RSN.pmkid_count * 4;  //pmkid
					}
                    //nIeLen doesn't count EID and length fields
                    if(HAL_STATUS_SUCCESS((status = palAllocateMemory(pMac->hHdd, (void **)&pMac->roam.pWpaRsnRspIE, nIeLen + 2))))
                    {
                        pMac->roam.pWpaRsnRspIE[0] = DOT11F_EID_RSN;
                        pMac->roam.pWpaRsnRspIE[1] = (tANI_U8)nIeLen;
                        //copy upto akm_suites
                        pIeBuf = pMac->roam.pWpaRsnRspIE + 2;
                        palCopyMemory(pMac->hHdd, pIeBuf, &pIes->RSN.version, 8);
                        pIeBuf += 8;
                        if( pIes->RSN.pwise_cipher_suite_count )
                        {
                            //copy pwise_cipher_suites
                            palCopyMemory(pMac->hHdd, pIeBuf, pIes->RSN.pwise_cipher_suites, pIes->RSN.pwise_cipher_suite_count * 4);
                            pIeBuf += pIes->RSN.pwise_cipher_suite_count * 4;
                        }
						palCopyMemory(pMac->hHdd, pIeBuf, &pIes->RSN.akm_suite_count, 2);
						pIeBuf += 2;
                        if( pIes->RSN.akm_suite_count )
                        {
                            //copy akm_suites
                            palCopyMemory(pMac->hHdd, pIeBuf, pIes->RSN.akm_suites, pIes->RSN.akm_suite_count * 4);
                            pIeBuf += pIes->RSN.akm_suite_count * 4;
                        }
                        //copy the rest
                        palCopyMemory(pMac->hHdd, pIeBuf, pIes->RSN.akm_suites + pIes->RSN.akm_suite_count * 4, 
										2 + pIes->RSN.pmkid_count * 4);
                        pMac->roam.nWpaRsnRspIeLength = nIeLen + 2; 
                    }
                }
            }
            else if((eCSR_AUTH_TYPE_WPA == authType) ||
                (eCSR_AUTH_TYPE_WPA_PSK == authType))
            {
                if(pIes->WPA.present)
                {
                    //Calculate the actual length
                    nIeLen = 12 //OUI + version + multicast_cipher + unicast_cipher_count
                        + pIes->WPA.unicast_cipher_count * 4    //unicast_ciphers
                        + 2 //auth_suite_count
                        + pIes->WPA.auth_suite_count * 4; //auth_suites
                    // The WPA capabilities follows the Auth Suite (two octects)--
                    // this field is optional, and we always "send" zero, so just
                    // remove it.  This is consistent with our assumptions in the
                    // frames compiler; c.f. bug 15234:
                    //nIeLen doesn't count EID and length fields
                    if(HAL_STATUS_SUCCESS((status = palAllocateMemory(pMac->hHdd, (void **)&pMac->roam.pWpaRsnRspIE, nIeLen + 2))))
                    {
                        pMac->roam.pWpaRsnRspIE[0] = DOT11F_EID_WPA;
                        pMac->roam.pWpaRsnRspIE[1] = (tANI_U8)nIeLen;
                        pIeBuf = pMac->roam.pWpaRsnRspIE + 2;
                        //Copy WPA OUI
                        palCopyMemory(pMac->hHdd, pIeBuf, &csrWpaOui[1], 4);
                        pIeBuf += 4;
                        palCopyMemory(pMac->hHdd, pIeBuf, &pIes->WPA.version, 8 + pIes->WPA.unicast_cipher_count * 4);
                        pIeBuf += 8 + pIes->WPA.unicast_cipher_count * 4;
                        palCopyMemory(pMac->hHdd, pIeBuf, &pIes->WPA.auth_suite_count, 2 + pIes->WPA.auth_suite_count * 4);
                        pIeBuf += pIes->WPA.auth_suite_count * 4;
                        pMac->roam.nWpaRsnRspIeLength = nIeLen + 2; 
                    }
                }
            }
        }
    }

    return (status);
}



static void csrRoamProcessResults( tpAniSirGlobal pMac, tSmeCmd *pCommand,
                                       eCsrRoamCompleteResult Result, void *Context )
{
    tSirBssDescription *pSirBssDesc = NULL;   
	tSirBssDescription *pTempBssDesc = NULL;
	tANI_U16 bssLen;
    tSirMacAddr BroadcastMac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    tCsrScanResult *pScanResult = NULL;
    tCsrRoamInfo roamInfo;
    sme_QosAssocInfo assocInfo;
    sme_QosCsrEventIndType ind_qos;//indication for QoS module in SME
    tANI_U8 acm_mask; //HDD needs the ACM mask in the assoc rsp callback
    tDot11fBeaconIEs *pIes = NULL;
    eHalStatus status;
    tCsrRoamProfile *pProfile = NULL;

    smsLog( pMac, LOG1, FL("Processsing ROAM results...\n"));

    switch( Result )
    {
        case eCsrJoinSuccess:
            // reset the IDLE timer
            // !!
            // !! fall through to the next CASE statement here is intentional !!
            // !!
        case eCsrReassocSuccess:
            if(eCsrReassocSuccess == Result)
            {
                ind_qos = SME_QOS_CSR_REASSOC_COMPLETE;
            }
            else
            {
                ind_qos = SME_QOS_CSR_ASSOC_COMPLETE;
            }
            // Success Join Response from LIM.  Tell NDIS we are connected and save the
            // Connected state...
            smsLog(pMac, LOGW, FL("receives association indication\n"));
            pProfile = &pCommand->u.roamCmd.roamProfile;
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(roamInfo));
            //always free the memory here
            if(pMac->roam.pWpaRsnRspIE)
            {
                pMac->roam.nWpaRsnRspIeLength = 0;
                palFreeMemory(pMac->hHdd, pMac->roam.pWpaRsnRspIE);
                pMac->roam.pWpaRsnRspIE = NULL;
            }

            csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINED );
            pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED;

            //Use the last connected bssdesc for reassoc-ing to the same AP.
            //NOTE: What to do when reassoc to a different AP???
            if( eCsrHddIssuedReassocToSameAP == pCommand->u.roamCmd.roamReason )
            {
                pSirBssDesc = pMac->roam.pConnectBssDesc;
            }
            else
            {
     
                if(pCommand->u.roamCmd.pRoamBssEntry)
                {
                    pScanResult = GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry, tCsrScanResult, Link);
                    if(pScanResult != NULL)
                    {
                        pSirBssDesc = &pScanResult->Result.BssDescriptor;
                        pIes = (tDot11fBeaconIEs *)( pScanResult->Result.pvIes );
                        palCopyMemory(pMac->hHdd, &roamInfo.bssid, &pSirBssDesc->bssId, sizeof(tCsrBssid));
                    }
                }
            }
            if( pSirBssDesc )
            {

                roamInfo.staId = HAL_STA_INVALID_IDX;

                // Make sure the Set Context is issued before link indication to NDIS.  After link indication is 
                // made to NDIS, frames could start flowing.  If we have not set context with LIM, the frames
                // will be dropped for the security context may not be set properly. 
                //
                // this was causing issues in the 2c_wlan_wep WHQL test when the SetContext was issued after the link
                // indication.  (Link Indication happens in the profFSMSetConnectedInfra call).
                //
                // this reordering was done on titan_prod_usb branch and is being replicated here.
                //
            
                if( CSR_IS_ENC_TYPE_STATIC( pProfile->negotiatedUCEncryptionType ) &&
                    !pProfile->bWPSAssociation)
                {
                    // Issue the set Context request to LIM to establish the Unicast STA context
                    if( !HAL_STATUS_SUCCESS( csrRoamIssueSetContextReq( pMac, 
                                                pProfile->negotiatedUCEncryptionType, 
                                                pSirBssDesc, &(pSirBssDesc->bssId),
                                                FALSE, TRUE, eSIR_TX_RX, 0, 0, NULL, 0 ) ) ) // NO keys... these key parameters don't matter.
                    {
                        smsLog( pMac, LOGE, FL("  Set contextfor unicast fail\n") );
                        //
                        csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
                    }
                    // Issue the set Context request to LIM to establish the Broadcast STA context
                    csrRoamIssueSetContextReq( pMac, pProfile->negotiatedMCEncryptionType, pSirBssDesc, &BroadcastMac,
                                        FALSE, FALSE, eSIR_TX_RX, 0, 0, NULL, 0 ); // NO keys... these key parameters don't matter.
                }
                else
                {
                    //Set the subestate to WaitForKey in case authentiation is needed
                    csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_WAIT_FOR_KEY );
                    //Need to wait for supplicant authtication
                    roamInfo.fAuthRequired = eANI_BOOLEAN_TRUE;
                    //This time should be long enough for the rest of the process plus setting key
                    if(!HAL_STATUS_SUCCESS( csrRoamStartWaitForKeyTimer( pMac, CSR_WAIT_FOR_KEY_TIMEOUT_PERIOD ) ) )
                    {
                        //Reset our state so nothting is blocked.
                        smsLog( pMac, LOGE, FL("   Failed to start pre-auth timer\n") );
                        csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
                    }
                }

                csrRoamSaveConnectedInfomation(pMac, pProfile, pSirBssDesc, pIes);
                //Save WPA/RSN IE
                csrRoamSaveWpaRsnRspIE(pMac, pProfile->negotiatedAuthType, pSirBssDesc, pIes);
                assocInfo.pBssDesc = pSirBssDesc; //could be NULL
                assocInfo.pProfile = pProfile;
                sme_QosCsrEventInd(pMac, ind_qos, &assocInfo);

                pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED;
                if(Context)
                {
                    tSirSmeJoinRsp *pJoinRsp = (tSirSmeJoinRsp *)Context;
                    tANI_U32 len;

                    csrRoamFreeConnectedInfo( pMac, &pMac->roam.connectedInfo );
                    len = pJoinRsp->assocReqLength + pJoinRsp->assocRspLength + pJoinRsp->beaconLength;
                    if(len)
                    {
                        if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, 
                                                (void **)&pMac->roam.connectedInfo.pbFrames, len)))
                        {
                            if(HAL_STATUS_SUCCESS( palCopyMemory(pMac->hHdd, 
                                            pMac->roam.connectedInfo.pbFrames, pJoinRsp->frames, len) ))
                            {
                                pMac->roam.connectedInfo.nAssocReqLength = pJoinRsp->assocReqLength;
                                pMac->roam.connectedInfo.nAssocRspLength = pJoinRsp->assocRspLength;
                                pMac->roam.connectedInfo.nBeaconLength = pJoinRsp->beaconLength;
                                roamInfo.nAssocReqLength = pJoinRsp->assocReqLength;
                                roamInfo.nAssocRspLength = pJoinRsp->assocRspLength;
                                roamInfo.nBeaconLength = pJoinRsp->beaconLength;
                                roamInfo.pbFrames = pMac->roam.connectedInfo.pbFrames;
                            }
                            else
                            {
                                palFreeMemory( pMac->hHdd, pMac->roam.connectedInfo.pbFrames );
                                pMac->roam.connectedInfo.pbFrames = NULL;
                            }
                        }
                    }
                    if(pCommand->u.roamCmd.fReassoc)
                    {
                        roamInfo.fReassocReq = roamInfo.fReassocRsp = eANI_BOOLEAN_TRUE;
                    }
                    pMac->roam.connectedInfo.staId = ( tANI_U8 )pJoinRsp->staId;
                    roamInfo.staId = ( tANI_U8 )pJoinRsp->staId;
                    roamInfo.ucastSig = ( tANI_U8 )pJoinRsp->ucastSig;
                    roamInfo.bcastSig = ( tANI_U8 )pJoinRsp->bcastSig;
                }

#ifdef FEATURE_WLAN_GEN6_ROAMING
                //Save the channel list for handoff scanning
                if(pProfile->ChannelInfo.numOfChannels && pProfile->ChannelInfo.ChannelList)
                {
                    VOS_ASSERT(WNI_CFG_VALID_CHANNEL_LIST_LEN >= pProfile->ChannelInfo.numOfChannels);
                    pMac->roam.connectedInfo.sHOScanChannelList.numChannels = 
                            CSR_MIN( WNI_CFG_VALID_CHANNEL_LIST_LEN, pProfile->ChannelInfo.numOfChannels );
                    palCopyMemory(pMac->hHdd, pMac->roam.connectedInfo.sHOScanChannelList.channelList, 
                        pProfile->ChannelInfo.ChannelList, pMac->roam.connectedInfo.sHOScanChannelList.numChannels);
                }
                else 
                {
                    //Get all the valid channels
                    tANI_U32 len = WNI_CFG_VALID_CHANNEL_LIST_LEN;
                    if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.connectedInfo.sHOScanChannelList.channelList, &len)))
                    {
                        pMac->roam.connectedInfo.sHOScanChannelList.numChannels = (tANI_U8)len;
                    }
                    else
                    {
                        pMac->roam.connectedInfo.sHOScanChannelList.numChannels = 0;
                    }

                }
#endif

                roamInfo.pBssDesc = pSirBssDesc;
                roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
                roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                acm_mask = sme_QosGetACMMask(pMac, pSirBssDesc, NULL);
                pMac->roam.connectedProfile.acm_mask = acm_mask;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
                //start UAPSD if uapsd_mask is not 0 because HDD will configure for trigger frame
                //It may be better to let QoS do this????
                if( pMac->roam.connectedProfile.modifyProfileFields.uapsd_mask )
                {
                    smsLog(pMac, LOGE, " uapsd_mask (0x%X) set, request UAPSD now\n",
                        pMac->roam.connectedProfile.modifyProfileFields.uapsd_mask);
                    pmcStartUapsd( pMac, NULL, NULL );
                }
#endif

                roamInfo.u.pConnectedProfile = &pMac->roam.connectedProfile;
                csrRoamCallCallback(pMac, &roamInfo, pCommand->u.roamCmd.roamId, eCSR_ROAM_ASSOCIATION_COMPLETION, eCSR_ROAM_RESULT_ASSOCIATED);
                csrRoamCompletion(pMac, NULL, pCommand, eCSR_ROAM_RESULT_NONE, eANI_BOOLEAN_TRUE);

                // reset the PMKID candidate list
                csrResetPMKIDCandidateList( pMac );
            }
            else
            {
                smsLog(pMac, LOGW, "  Roam command doesn't have a BSS desc\n");
            }

            csrScanCancelIdleScan(pMac);
#ifdef FEATURE_WLAN_GEN6_ROAMING
            palZeroMemory(pMac->hHdd, &pMac->roam.handoffInfo.currSta, sizeof(tCsrRoamHandoffStaEntry));
            //Save bssid
            csrGetBssIdBssDesc(pMac, pSirBssDesc, &pMac->roam.handoffInfo.currSta.bssid);
#endif
            //save the bss Descriptor
            bssLen = pSirBssDesc->length + sizeof(pSirBssDesc->length);

            status = palAllocateMemory(pMac->hHdd, (void **)&pTempBssDesc, bssLen);
            if (!HAL_STATUS_SUCCESS(status))
            {
                smsLog(pMac, LOGE, "csrRoamProcessResults: couldn't allocate memory for the \
         		     bss Descriptor\n");
                return;
            }

            palZeroMemory(pMac->hHdd, pTempBssDesc, bssLen);
            palCopyMemory(pMac->hHdd, pTempBssDesc, pSirBssDesc, bssLen);
#ifdef FEATURE_WLAN_GEN6_ROAMING
            pMac->roam.handoffInfo.currSta.pBssDesc = pTempBssDesc;
#endif
            //Not to signal link up because keys are yet to be set.
            //The linkup function will overwrite the sub-state that we need to keep at this point.
            if( !CSR_IS_WAIT_FOR_KEY(pMac) )
            {
                csrRoamLinkUp(pMac, pMac->roam.connectedProfile.bssid);
            }

            break;


        case eCsrStartIbssSuccess:
            // on the StartBss Response, LIM is returning the Bss Description that we
            // are beaconing.  Add this Bss Description to our scan results and
            // chain the Profile to this Bss Description.  On a Start BSS, there was no
            // detected Bss description (no partner) so we issued the Start Bss to
            // start the Ibss without any Bss description.  Lim was kind enough to return
            // the Bss Description that we start beaconing for the newly started Ibss.
            smsLog(pMac, LOG2, FL("receives start IBSS ok indication\n"));
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
            pSirBssDesc = (tSirBssDescription *)Context;
            csrGetParsedBssDescriptionIEs( pMac, pSirBssDesc, &pIes );
            pScanResult = csrScanAppendBssDescription( pMac, pSirBssDesc, pIes );
            pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED;
            csrRoamSaveConnectedBssDesc(pMac, pSirBssDesc);
            csrRoamFreeConnectProfile(pMac, &pMac->roam.connectedProfile);
            csrRoamFreeConnectedInfo( pMac, &pMac->roam.connectedInfo );
            if(pSirBssDesc)
            {
                csrRoamSaveConnectedInfomation(pMac, &pCommand->u.roamCmd.roamProfile, pSirBssDesc, pIes);
                palCopyMemory(pMac->hHdd, &roamInfo.bssid, &pSirBssDesc->bssId, sizeof(tCsrBssid));
            }

#ifdef FEATURE_WLAN_DIAG_SUPPORT
            {
                vos_log_ibss_pkt_type *pIbssLog;
                tANI_U32 bi;

                WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
                if(pIbssLog)
                {
                    if(CSR_INVALID_SCANRESULT_HANDLE == pCommand->u.roamCmd.hBSSList)
                    {
                        //We start the IBSS (didn't find any matched IBSS out there)
                        pIbssLog->eventId = WLAN_IBSS_EVENT_START_IBSS_RSP;
                    }
                    else
                    {
                        pIbssLog->eventId = WLAN_IBSS_EVENT_JOIN_IBSS_RSP;
                    }
                    if(pSirBssDesc)
                    {
                        palCopyMemory(pMac->hHdd, pIbssLog->bssid, pSirBssDesc->bssId, 6);
                        pIbssLog->operatingChannel = pSirBssDesc->channelId;
                    }
                    if(HAL_STATUS_SUCCESS(ccmCfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &bi)))
                    {
                        //***U8 is not enough for beacon interval
                        pIbssLog->beaconInterval = (v_U8_t)bi;
                    }
                    WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
                }
            }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT
    
            csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINED );
            //Only tell upper layer is we start the BSS because Vista doesn't like multiple connection
            //indications. If we don't start the BSS ourself, handler of eSIR_SME_JOINED_NEW_BSS will 
            //trigger the connection start indication in Vista
            if( !CSR_IS_JOIN_TO_IBSS( &pCommand->u.roamCmd.roamProfile ) )
            {
                //Only tell upper layer is we start the BSS because Vista doesn't like multiple connection
                //indications. If we don't start the BSS ourself, handler of eSIR_SME_JOINED_NEW_BSS will 
                //trigger the connection start indication in Vista
                roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
                roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                if(CSR_INVALID_SCANRESULT_HANDLE == pCommand->u.roamCmd.hBSSList)
                {
                    //We start the IBSS (didn't find any matched IBSS out there)
                    roamInfo.pBssDesc = pSirBssDesc;
                }
                csrRoamCallCallback( pMac, &roamInfo, pCommand->u.roamCmd.roamId, eCSR_ROAM_IBSS_IND, eCSR_ROAM_RESULT_IBSS_STARTED );
            }
    
            // Issue the set Context request to LIM to establish the Broadcast STA context for the Ibss.
            csrRoamIssueSetContextReq( pMac, pCommand->u.roamCmd.roamProfile.negotiatedMCEncryptionType, pSirBssDesc, &BroadcastMac,
                                    FALSE, FALSE, eSIR_TX_RX, 0, 0, NULL, 0 ); // NO keys... these key parameters don't matter.
            csrScanCancelIdleScan(pMac);
            csrScanBGScanEnable(pMac);
            if( CSR_IS_JOIN_TO_IBSS( &pCommand->u.roamCmd.roamProfile ) )
            {
              //start the join IBSS timer
              csrRoamStartIbssJoinTimer(pMac, CSR_IBSS_JOIN_TIMEOUT_PERIOD); //interval
              pMac->roam.ibss_join_pending = TRUE;
            }
            break;

        case eCsrStartIbssFailure:

#ifdef FEATURE_WLAN_DIAG_SUPPORT
            {
                vos_log_ibss_pkt_type *pIbssLog;

                WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
                if(pIbssLog)
                {
                    pIbssLog->status = WLAN_IBSS_STATUS_FAILURE;
                    WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
                }
            }
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT
    
            if(Context)
            {
                pSirBssDesc = (tSirBssDescription *)Context;
            }
            else
            {
                pSirBssDesc = NULL;
            }
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
            roamInfo.pBssDesc = pSirBssDesc;
            //We need to associate_complete it first, becasue Associate_start already indicated.
            csrRoamCallCallback( pMac, &roamInfo, pCommand->u.roamCmd.roamId, eCSR_ROAM_IBSS_IND, eCSR_ROAM_RESULT_IBSS_START_FAILED );
            break;

        case eCsrSilentlyStopRoaming:
            // We are here because we try to start the same IBSS
            //No message to PE
            // return the roaming state to Joined.
            smsLog(pMac, LOGW, FL("receives silently roaming indication\n"));
            csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINED );
            csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
            roamInfo.pBssDesc = pMac->roam.pConnectBssDesc;
            if( roamInfo.pBssDesc )
            {
                palCopyMemory(pMac->hHdd, &roamInfo.bssid, &roamInfo.pBssDesc->bssId, sizeof(tCsrBssid));
            }
			//Since there is no change in the current state, simply pass back no result otherwise
			//HDD may be mistakenly mark to disconnected state.
            csrRoamCallCallback( pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                        eCSR_ROAM_IBSS_IND, eCSR_ROAM_RESULT_NONE );

            break;


        case eCsrSilentlyStopRoamingSaveState:
            //We are here because we try to connect to the same AP
            //No message to PE
            smsLog(pMac, LOGW, FL("receives silently stop roaming indication\n"));
            palZeroMemory(pMac->hHdd, &roamInfo, sizeof(roamInfo));
            
            //to aviod resetting the substate to NONE
            pMac->roam.curState = eCSR_ROAMING_STATE_JOINED;
#ifdef FEATURE_WLAN_GEN6_ROAMING
            pMac->roam.handoffInfo.currState = pMac->roam.curState;
#endif
            //No need to change substate to wai_for_key because there is no state change
            roamInfo.pBssDesc = pMac->roam.pConnectBssDesc;
            if( roamInfo.pBssDesc )
            {
                palCopyMemory(pMac->hHdd, &roamInfo.bssid, &roamInfo.pBssDesc->bssId, sizeof(tCsrBssid));
            }

            roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
            roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
            roamInfo.nBeaconLength = pMac->roam.connectedInfo.nBeaconLength;
            roamInfo.nAssocReqLength = pMac->roam.connectedInfo.nAssocReqLength;
            roamInfo.nAssocRspLength = pMac->roam.connectedInfo.nAssocRspLength;
            roamInfo.pbFrames = pMac->roam.connectedInfo.pbFrames;
            roamInfo.staId = pMac->roam.connectedInfo.staId;
			roamInfo.u.pConnectedProfile = &pMac->roam.connectedProfile;
#if defined(VOSS_ENABLED)
            VOS_ASSERT( roamInfo.staId != 0 );
#endif
            csrRoamCallCallback(pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                        eCSR_ROAM_ASSOCIATION_COMPLETION, eCSR_ROAM_RESULT_ASSOCIATED);
            csrRoamCompletion(pMac, NULL, pCommand, eCSR_ROAM_RESULT_ASSOCIATED, eANI_BOOLEAN_TRUE);
            break;

        case eCsrReassocFailure:
            sme_QosCsrEventInd(pMac, SME_QOS_CSR_REASSOC_FAILURE, NULL);

        case eCsrJoinFailure:
        case eCsrNothingToJoin:
        default:
        {
            smsLog(pMac, LOGW, FL("receives no association indication\n"));
            csrFreeConnectBssDesc(pMac);
            csrRoamFreeConnectProfile(pMac, &pMac->roam.connectedProfile);
            csrRoamFreeConnectedInfo( pMac, &pMac->roam.connectedInfo );
            switch( pCommand->u.roamCmd.roamReason )
            {
                // If this transition is because of an 802.11 OID, then we transition
                // back to INIT state so we sit waiting for more OIDs to be issued and
                // we don't start the IDLE timer.
                case eCsrSmeIssuedAssocToSimilarAP:
                case eCsrHddIssuedReassocToSameAP:
                case eCsrSmeIssuedReassocToSameAP:
                case eCsrHddIssued:
                    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_IDLE );
                    palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                    roamInfo.pBssDesc = pCommand->u.roamCmd.pLastRoamBss;
                    roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
                    roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                    csrRoamCallCallback(pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                            eCSR_ROAM_ASSOCIATION_COMPLETION, 
                                            eCSR_ROAM_RESULT_FAILURE);
                    smsLog(pMac, LOG1, FL("  roam(reason %d) failed\n"), pCommand->u.roamCmd.roamReason);
                    csrRoamCompletion(pMac, NULL, pCommand, eCSR_ROAM_RESULT_FAILURE, eANI_BOOLEAN_FALSE);
                    csrScanStartIdleScan(pMac);
                    break;
                case eCsrForcedDisassoc:
                case eCsrForcedDeauth:
                    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_IDLE );
                    csrRoamCallCallback(pMac, NULL, pCommand->u.roamCmd.roamId, eCSR_ROAM_DISASSOCIATED, eCSR_ROAM_RESULT_FORCED);
                    sme_QosCsrEventInd(pMac, SME_QOS_CSR_DISCONNECT_IND, NULL);
                    csrRoamLinkDown(pMac);
                    csrScanStartIdleScan(pMac);
                    break;
                case eCsrForcedDisassocMICFailure:
                    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_IDLE );
                    csrRoamCallCallback(pMac, NULL, pCommand->u.roamCmd.roamId, eCSR_ROAM_DISASSOCIATED, eCSR_ROAM_RESULT_MIC_FAILURE);
                    sme_QosCsrEventInd(pMac, SME_QOS_CSR_DISCONNECT_REQ, NULL);
                    csrScanStartIdleScan(pMac);
                    break;

                case eCsrLostLink1:
                    // if lost link roam1 failed, then issue lost link Scan2 ...
                    csrScanRequestLostLink2(pMac);
                    break;

                case eCsrLostLink2:
                    // if lost link roam2 failed, then issue lost link scan3 ...
                    csrScanRequestLostLink3(pMac);
                    break;

                case eCsrLostLink3:
                default:
                    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_IDLE );
                    //We are done with one round of lostlink roaming here
                    csrScanHandleFailedLostlink3(pMac);
                    break;
            }

            break;
        }
    }

}


eHalStatus csrRoamRegisterCallback(tHalHandle hHal, csrRoamCompleteCallback callback, void *pContext)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    pMac->roam.callback = callback;
    pMac->roam.pContext = pContext;    
    return (status);
}


eHalStatus csrRoamCopyProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pDstProfile, tCsrRoamProfile *pSrcProfile)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 size = 0;
    
    do
    {
        palZeroMemory(pMac->hHdd, pDstProfile, sizeof(tCsrRoamProfile));
        if(pSrcProfile->BSSIDs.numOfBSSIDs)
        {
            size = sizeof(tCsrBssid) * pSrcProfile->BSSIDs.numOfBSSIDs;
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->BSSIDs.bssid, size);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->BSSIDs.numOfBSSIDs = pSrcProfile->BSSIDs.numOfBSSIDs;
            palCopyMemory(pMac->hHdd, pDstProfile->BSSIDs.bssid, pSrcProfile->BSSIDs.bssid, size);
        }
        if(pSrcProfile->SSIDs.numOfSSIDs)
        {
            size = sizeof(tCsrSSIDInfo) * pSrcProfile->SSIDs.numOfSSIDs;
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->SSIDs.SSIDList, size);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->SSIDs.numOfSSIDs = pSrcProfile->SSIDs.numOfSSIDs;
            palCopyMemory(pMac->hHdd, pDstProfile->SSIDs.SSIDList, pSrcProfile->SSIDs.SSIDList, size);
        }
        if(pSrcProfile->nWPAReqIELength)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->pWPAReqIE, pSrcProfile->nWPAReqIELength);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->nWPAReqIELength = pSrcProfile->nWPAReqIELength;
            palCopyMemory(pMac->hHdd, pDstProfile->pWPAReqIE, pSrcProfile->pWPAReqIE, pSrcProfile->nWPAReqIELength);
        }
        if(pSrcProfile->nRSNReqIELength)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->pRSNReqIE, pSrcProfile->nRSNReqIELength);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->nRSNReqIELength = pSrcProfile->nRSNReqIELength;
            palCopyMemory(pMac->hHdd, pDstProfile->pRSNReqIE, pSrcProfile->pRSNReqIE, pSrcProfile->nRSNReqIELength);
        }
        if(pSrcProfile->ChannelInfo.ChannelList)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->ChannelInfo.ChannelList, pSrcProfile->ChannelInfo.numOfChannels);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->ChannelInfo.numOfChannels = pSrcProfile->ChannelInfo.numOfChannels;
            palCopyMemory(pMac->hHdd, pDstProfile->ChannelInfo.ChannelList, pSrcProfile->ChannelInfo.ChannelList, pSrcProfile->ChannelInfo.numOfChannels);
        }

        pDstProfile->AuthType = pSrcProfile->AuthType;
        pDstProfile->EncryptionType = pSrcProfile->EncryptionType;
        pDstProfile->mcEncryptionType = pSrcProfile->mcEncryptionType;
        pDstProfile->negotiatedUCEncryptionType = pSrcProfile->negotiatedUCEncryptionType;
        pDstProfile->negotiatedMCEncryptionType = pSrcProfile->negotiatedMCEncryptionType;
        pDstProfile->negotiatedAuthType = pSrcProfile->negotiatedAuthType;
        pDstProfile->BSSType = pSrcProfile->BSSType;
        pDstProfile->phyMode = pSrcProfile->phyMode;
        pDstProfile->CBMode = pSrcProfile->CBMode;
        /*Save the WPS info*/
        pDstProfile->bWPSAssociation = pSrcProfile->bWPSAssociation;
        pDstProfile->uapsd_mask = pSrcProfile->uapsd_mask;

        palCopyMemory(pMac->hHdd, &pDstProfile->Keys, &pSrcProfile->Keys, sizeof(pDstProfile->Keys));
    
    }while(0);
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrReleaseProfile(pMac, pDstProfile);
        pDstProfile = NULL;
    }
    
    return (status);
}


eHalStatus csrRoamCopyConnectedProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pDstProfile )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCsrRoamConnectedProfile *pSrcProfile = &pMac->roam.connectedProfile; 
    do
    {
        palZeroMemory(pMac->hHdd, pDstProfile, sizeof(tCsrRoamProfile));
        if(pSrcProfile->bssid)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->BSSIDs.bssid, sizeof(tCsrBssid));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->BSSIDs.numOfBSSIDs = 1;
            palCopyMemory(pMac->hHdd, pDstProfile->BSSIDs.bssid, pSrcProfile->bssid, sizeof(tCsrBssid));
        }
        if(pSrcProfile->SSID.ssId)
        {
            status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->SSIDs.SSIDList, sizeof(tCsrSSIDInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pDstProfile->SSIDs.numOfSSIDs = 1;
            pDstProfile->SSIDs.SSIDList[0].handoffPermitted = pSrcProfile->handoffPermitted;
            pDstProfile->SSIDs.SSIDList[0].ssidHidden = pSrcProfile->ssidHidden;
            palCopyMemory(pMac->hHdd, &pDstProfile->SSIDs.SSIDList[0].SSID, &pSrcProfile->SSID, sizeof(tSirMacSSid));
        }

        status = palAllocateMemory(pMac->hHdd, (void **)&pDstProfile->ChannelInfo.ChannelList, 1);
        if(!HAL_STATUS_SUCCESS(status))
        {
           break;
        }
        pDstProfile->ChannelInfo.numOfChannels = 1;
        pDstProfile->ChannelInfo.ChannelList[0] = pSrcProfile->operationChannel;

        pDstProfile->AuthType.numEntries = 1;
        pDstProfile->AuthType.authType[0] = pSrcProfile->AuthType;
        pDstProfile->negotiatedAuthType = pSrcProfile->AuthType;
        pDstProfile->EncryptionType.numEntries = 1;
        pDstProfile->EncryptionType.encryptionType[0] = pSrcProfile->EncryptionType;
        pDstProfile->negotiatedUCEncryptionType = pSrcProfile->EncryptionType;
        pDstProfile->mcEncryptionType.numEntries = 1;
        pDstProfile->mcEncryptionType.encryptionType[0] = pSrcProfile->mcEncryptionType;
        pDstProfile->negotiatedMCEncryptionType = pSrcProfile->mcEncryptionType;
        pDstProfile->BSSType = pSrcProfile->BSSType;
        pDstProfile->CBMode = pSrcProfile->CBMode;
        palCopyMemory(pMac->hHdd, &pDstProfile->Keys, &pSrcProfile->Keys, sizeof(pDstProfile->Keys));
    
    }while(0);
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrReleaseProfile(pMac, pDstProfile);
        pDstProfile = NULL;
    }
    
    return (status);
}


eHalStatus csrRoamIssueConnect(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tScanResultHandle hBSSList, 
                                eCsrRoamReason reason, tANI_U32 roamId, tANI_BOOLEAN fImediate,
                                tANI_BOOLEAN fClearScan)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand;
    
    pCommand = csrGetCommandBuffer(pMac);
    if(NULL == pCommand)
    {
        status = eHAL_STATUS_RESOURCES;
    }
    else
    {
        if( fClearScan )
        {
            csrScanCancelIdleScan(pMac);
            csrScanAbortMacScan(pMac);
        }
        pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_FALSE;
        if(NULL == pProfile)
        {
            //We can roam now
            //Since pProfile is NULL, we need to build our own profile, set everything to default
            //We can only support open and no encryption
            pCommand->u.roamCmd.roamProfile.AuthType.numEntries = 1; 
            pCommand->u.roamCmd.roamProfile.AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;
            pCommand->u.roamCmd.roamProfile.EncryptionType.numEntries = 1;
            pCommand->u.roamCmd.roamProfile.EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        }
        else
        {
            //make a copy of the profile
            status = csrRoamCopyProfile(pMac, &pCommand->u.roamCmd.roamProfile, pProfile);
            if(HAL_STATUS_SUCCESS(status))
            {
                pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_TRUE;
            }
        }
        pCommand->command = eSmeCommandRoam;
        pCommand->u.roamCmd.hBSSList = hBSSList;
        pCommand->u.roamCmd.roamId = roamId;
        pCommand->u.roamCmd.roamReason = reason;
        //We need to free the BssList when the command is done
        pCommand->u.roamCmd.fReleaseBssList = eANI_BOOLEAN_TRUE;
        pCommand->u.roamCmd.fUpdateCurRoamProfile = eANI_BOOLEAN_TRUE;

        status = csrQueueSmeCommand(pMac, pCommand, fImediate);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            csrReleaseCommandRoam( pMac, pCommand );
        }
    }
    
    return (status);
}

eHalStatus csrRoamIssueReassoc(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile,
                               tCsrRoamModifyProfileFields *pMmodProfileFields,
                               eCsrRoamReason reason, tANI_U32 roamId, tANI_BOOLEAN fImediate)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand;
    
    pCommand = csrGetCommandBuffer(pMac);
    if(NULL == pCommand)
    {
        status = eHAL_STATUS_RESOURCES;
    }
    else
    {
        csrScanCancelIdleScan(pMac);
        csrScanAbortMacScan(pMac);
        if(pProfile)
        {

           //This is likely trying to reassoc to different profile
           pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_FALSE;
           //make a copy of the profile
           status = csrRoamCopyProfile(pMac, &pCommand->u.roamCmd.roamProfile, pProfile);
           pCommand->u.roamCmd.fUpdateCurRoamProfile = eANI_BOOLEAN_TRUE;

        }
        else
        {
            status = csrRoamCopyConnectedProfile(pMac, &pCommand->u.roamCmd.roamProfile);
            pCommand->u.roamCmd.roamProfile.uapsd_mask = 
               pMmodProfileFields->uapsd_mask;

        }

        if(HAL_STATUS_SUCCESS(status))
        {
           pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_TRUE;
        }
        pCommand->command = eSmeCommandRoam;
        pCommand->u.roamCmd.roamId = roamId;
        pCommand->u.roamCmd.roamReason = reason;
        //We need to free the BssList when the command is done
        //For reassoc there is no BSS list, so the boolean set to false
        pCommand->u.roamCmd.fReleaseBssList = eANI_BOOLEAN_FALSE;
        pCommand->u.roamCmd.fReassoc = eANI_BOOLEAN_TRUE;

        status = csrQueueSmeCommand(pMac, pCommand, fImediate);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            csrRoamCompletion(pMac, NULL, pCommand, eCSR_ROAM_RESULT_FAILURE, eANI_BOOLEAN_FALSE);
            csrReleaseCommandRoam( pMac, pCommand );
        }
    }

    return (status);
}


eHalStatus csrRoamConnectWithBSSList(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tScanResultHandle hBssListIn, tANI_U32 *pRoamId)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList;
    tANI_U32 roamId = 0;

    status = csrScanCopyResultList(pMac, hBssListIn, &hBSSList);
    if(HAL_STATUS_SUCCESS(status))
    {
        roamId = GET_NEXT_ROAM_ID(&pMac->roam);
        if(pRoamId)
        {
            *pRoamId = roamId;
        }
        status = csrRoamIssueConnect(pMac, pProfile, hBSSList, eCsrHddIssued, 
                                        roamId, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, FL("failed to start a join process\n"));
            csrScanResultPurge(pMac, hBSSList);
        }
    }

    return (status);
}


eHalStatus csrRoamConnect(tHalHandle hHal, tCsrRoamProfile *pProfile, tScanResultHandle hBssListIn, tANI_U32 *pRoamId)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tScanResultHandle hBSSList;
    tCsrScanResultFilter *pScanFilter;
    tANI_U32 roamId = 0;
    tANI_BOOLEAN fCallCallback = eANI_BOOLEAN_FALSE;
    
    smsLog(pMac, LOG1, FL("called  BSSType = %d authtype = %d  encryType = %d\n"), pProfile->BSSType, pProfile->AuthType.authType[0], pProfile->EncryptionType.encryptionType[0]);
    csrRoamCancelRoaming(pMac);
    csrScanRemoveFreshScanCommand(pMac);
    csrScanCancelIdleScan(pMac);
    csrScanAbortMacScan(pMac);
    csrRoamRemoveDuplicateCommand(pMac, NULL, eCsrHddIssued);
    //Check whether ssid changes
    if(csrIsConnStateConnected(pMac))
    {
        if(pProfile->SSIDs.numOfSSIDs && !csrIsSsidInList(pMac, &pMac->roam.connectedProfile.SSID, &pProfile->SSIDs))
        {
            csrRoamIssueDisassociateCmd(pMac, eCSR_DISCONNECT_REASON_UNSPECIFIED);
        }
    }
    if(CSR_INVALID_SCANRESULT_HANDLE != hBssListIn)
    {
        smsLog(pMac, LOGW, FL("is called with BSSList\n"));
        status = csrRoamConnectWithBSSList(pMac, pProfile, hBssListIn, pRoamId);
        if(pRoamId)
        {
            roamId = *pRoamId;
        }
        if(!HAL_STATUS_SUCCESS(status))
        {
            fCallCallback = eANI_BOOLEAN_TRUE;
        }
    }
    else
    {
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            //Try to connect to any BSS
            if(NULL == pProfile)
            {
                //No encryption
                pScanFilter->EncryptionType.numEntries = 1;
                pScanFilter->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
            }//we don't have a profile
            else 
            {
                //Here is the profile we need to connect to
                status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
            }//We have a profile
            roamId = GET_NEXT_ROAM_ID(&pMac->roam);
            if(pRoamId)
            {
                *pRoamId = roamId;
            }
            
            if(HAL_STATUS_SUCCESS(status))
            {
                status = csrScanGetResult(hHal, pScanFilter, &hBSSList);
                if(HAL_STATUS_SUCCESS(status))
                {

                    status = csrRoamIssueConnect(pMac, pProfile, hBSSList, eCsrHddIssued, 
                                                    roamId, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
                    if(!HAL_STATUS_SUCCESS(status))
                    {
                        csrScanResultPurge(pMac, hBSSList);
                        fCallCallback = eANI_BOOLEAN_TRUE;
                    }
                }//Have scan result
                else if(NULL != pProfile)
                {
                    //Check whether it is for start ibss
                    if(CSR_IS_START_IBSS(pProfile))
                    {
                        status = csrRoamIssueConnect(pMac, pProfile, NULL, eCsrHddIssued, 
                                                        roamId, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
                        if(!HAL_STATUS_SUCCESS(status))
                        {
                            smsLog(pMac, LOGE, "   CSR failed to issue startIBSS command with status = 0x%08X\n", status);
                            fCallCallback = eANI_BOOLEAN_TRUE;
                        }
                    }
                    else
                    {
                        //scan for this SSID
                        status = csrScanForSSID(pMac, pProfile, roamId);
                        if(!HAL_STATUS_SUCCESS(status))
                        {
                            fCallCallback = eANI_BOOLEAN_TRUE;
                        }
                    }
                }
                else
                {
                    fCallCallback = eANI_BOOLEAN_TRUE;
                }
            }//Got the scan filter from profile
            
            //we need to free memory for filter if profile exists
            csrFreeScanFilter(pMac, pScanFilter);
            palFreeMemory(pMac->hHdd, pScanFilter);
        }//allocated memory for pScanFilter
    }//No Bsslist coming in
    //tell the caller if we fail to trigger a join request
    if(eANI_BOOLEAN_FALSE != fCallCallback)
    {
        csrRoamCallCallback(pMac, NULL, roamId, eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
    }
   
    return (status);
}                         

eHalStatus csrRoamReassoc(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile,
                          tCsrRoamModifyProfileFields modProfileFields,
                          tANI_U32 *pRoamId)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_BOOLEAN fCallCallback = eANI_BOOLEAN_TRUE;
   tANI_U32 roamId = 0;
   tCsrRoamInfo roamInfo;

   smsLog(pMac, LOG1, FL("called  BSSType = %d authtype = %d  encryType = %d\n"), pProfile->BSSType, pProfile->AuthType.authType[0], pProfile->EncryptionType.encryptionType[0]);
   csrRoamCancelRoaming(pMac);
   csrScanRemoveFreshScanCommand(pMac);
   csrScanCancelIdleScan(pMac);
   csrScanAbortMacScan(pMac);
   csrRoamRemoveDuplicateCommand(pMac, NULL, eCsrHddIssuedReassocToSameAP);

   if(csrIsConnStateConnected(pMac))
   {
      if(pProfile)
      {
         if(pProfile->SSIDs.numOfSSIDs && 
            csrIsSsidInList(pMac, &pMac->roam.connectedProfile.SSID, &pProfile->SSIDs))
         {
            fCallCallback = eANI_BOOLEAN_FALSE;
         }
         else
         {
            smsLog(pMac, LOG1, FL("Not connected to the same SSID asked in the profile\n"));
         }
      }
      else if(!palEqualMemory(pMac->hHdd, &modProfileFields, 
                              &pMac->roam.connectedProfile.modifyProfileFields, 
                              sizeof(tCsrRoamModifyProfileFields)))
      {
         fCallCallback = eANI_BOOLEAN_FALSE;
      }
      else
      {
         smsLog(pMac, LOG1, FL("Either the profile is NULL or none of the fields\
                               in tCsrRoamModifyProfileFields got modified\n"));
      }
   }
   else
   {
      smsLog(pMac, LOG1, FL("Not connected! No need to reassoc\n"));
   }

   if(!fCallCallback)
   {
      roamId = GET_NEXT_ROAM_ID(&pMac->roam);
      if(pRoamId)
      {
         *pRoamId = roamId;
      }
      roamInfo.reasonCode = eCsrRoamReasonStaCapabilityChanged;
      csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);
      pMac->roam.roamingReason = eCsrReassocRoaming;

      status = csrRoamIssueReassoc(pMac, pProfile, &modProfileFields, 
                                   eCsrHddIssuedReassocToSameAP, roamId, eANI_BOOLEAN_FALSE);

   }
   else
   {
      status = csrRoamCallCallback(pMac, NULL, roamId, 
                                   eCSR_ROAM_FAILED, eCSR_ROAM_RESULT_FAILURE);
   }

   return status;
}

eHalStatus csrRoamJoinLastProfile(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tScanResultHandle hBSSList = NULL;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId;
    tCsrRoamProfile *pProfile = NULL;

    do
    {
        if(pMac->roam.pCurRoamProfile)
        {
            csrScanCancelIdleScan(pMac);
            csrScanAbortMacScan(pMac);
            //We have to make a copy of pCurRoamProfile because it will be free inside csrRoamIssueConnect
            status = palAllocateMemory(pMac->hHdd, (void **)&pProfile, sizeof(tCsrRoamProfile));
            if(!HAL_STATUS_SUCCESS(status))
                break;
            palZeroMemory(pMac->hHdd, pProfile, sizeof(tCsrRoamProfile));
            status = csrRoamCopyProfile(pMac, pProfile, pMac->roam.pCurRoamProfile);
            if(!HAL_STATUS_SUCCESS(status))
                break;
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            roamId = GET_NEXT_ROAM_ID(&pMac->roam);
            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                //we want to put the last connected BSS to the very beginning, if possible
                csrMoveBssToHeadFromBSSID(pMac, &pMac->roam.connectedProfile.bssid, hBSSList);
                status = csrRoamIssueConnect(pMac, pProfile, hBSSList, eCsrHddIssued, 
                                                roamId, eANI_BOOLEAN_FALSE, eANI_BOOLEAN_FALSE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    csrScanResultPurge(pMac, hBSSList);
                    break;
                }
            }
            else
            {
                //Do a scan on this profile
                //scan for this SSID only in case the AP suppresses SSID
                status = csrScanForSSID(pMac, pProfile, roamId);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    break;
                }
            }
        }//We have a profile
        else
        {
            smsLog(pMac, LOGW, FL("cannot find a roaming profile\n"));
            break;
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

eHalStatus csrRoamReconnect(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if(csrIsConnStateConnected(pMac))
    {
        status = csrRoamIssueDisassociateCmd(pMac, eCSR_DISCONNECT_REASON_UNSPECIFIED);
        if(HAL_STATUS_SUCCESS(status))
        {
            status = csrRoamJoinLastProfile(pMac);
        }
    }

    return (status);
}


eHalStatus csrRoamConnectToLastProfile(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    smsLog(pMac, LOGW, FL("is called\n"));
    csrRoamCancelRoaming(pMac);
    csrRoamRemoveDuplicateCommand(pMac, NULL, eCsrHddIssued);
    if(csrIsConnStateDisconnected(pMac))
    {
        status = csrRoamJoinLastProfile(pMac);
    }

    return (status);
}


eHalStatus csrRoamProcessDisassociate( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fMICFailure )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fComplete = eANI_BOOLEAN_FALSE;
    eCsrRoamSubState NewSubstate;
    // change state to 'Roaming'...
    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINING );

    if ( csrIsConnStateIbss( pMac ) )
    {
        // If we are in an IBSS, then stop the IBSS...
        status = csrRoamIssueStopBss( pMac, eCSR_ROAM_SUBSTATE_STOP_BSS_REQ );
        fComplete = (!HAL_STATUS_SUCCESS(status));
    }
    else if ( csrIsConnStateInfra( pMac ) )
    {
        // in Infrasturcture, we need to disassociate from the Infrastructure network...
		NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_FORCED;
		if(eCsrSmeIssuedDisassocForHandoff == pCommand->u.roamCmd.roamReason)
		{
			NewSubstate = eCSR_ROAM_SUBSTATE_DISASSOC_HANDOFF;
		}
        status = csrRoamIssueDisassociate( pMac, NewSubstate, fMICFailure );
        fComplete = (!HAL_STATUS_SUCCESS(status));
    }
    else
    {
        // we got a dis-assoc request while not connected to any peer
        // just complete the command
        fComplete = eANI_BOOLEAN_TRUE;
    }
    if(fComplete)
    {
        csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
    }
    if(HAL_STATUS_SUCCESS(status))
    {
        //Set the state to disconnect here 
        pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
    }
    
    return (status);
}


eHalStatus csrRoamProcessDeauth( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fComplete = eANI_BOOLEAN_FALSE;
    
    // change state to 'Roaming'...
    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINING );

    if ( csrIsConnStateIbss( pMac ) )
    {
        // If we are in an IBSS, then stop the IBSS...
        status = csrRoamIssueStopBss( pMac, eCSR_ROAM_SUBSTATE_STOP_BSS_REQ );
        fComplete = (!HAL_STATUS_SUCCESS(status));
    }
    else if ( csrIsConnStateInfra( pMac ) )
    {
        // in Infrasturcture, we need to disassociate from the Infrastructure network...
        status = csrRoamIssueDeauth( pMac, eCSR_ROAM_SUBSTATE_AUTH_REQ );
        fComplete = (!HAL_STATUS_SUCCESS(status));
    }
    else
    {
        // we got a deauth request while not connected to any peer
        // just complete the command
        fComplete = eANI_BOOLEAN_TRUE;
    }
    if(fComplete)
    {
        csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
    }
    if(HAL_STATUS_SUCCESS(status))
    {
        //Set the state to disconnect here 
        pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
    }
    
    return (status);
}

eHalStatus csrRoamIssueDisassociateCmd( tpAniSirGlobal pMac, eCsrRoamDisconnectReason reason )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSmeCmd *pCommand;
	tANI_BOOLEAN fHighPriority = eANI_BOOLEAN_FALSE;

    do
    {
        smsLog( pMac, LOGE, FL("  reason = %d\n"), reason );
        pCommand = csrGetCommandBuffer( pMac );
        if ( !pCommand ) 
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        //Change the substate in case it is wait-for-key
        if( CSR_IS_WAIT_FOR_KEY( pMac ) )
        {
            csrRoamStopWaitForKeyTimer( pMac );
            csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
        }
        pCommand->command = eSmeCommandRoam;
        switch ( reason )
        {
        case eCSR_DISCONNECT_REASON_MIC_ERROR:
            pCommand->u.roamCmd.roamReason = eCsrForcedDisassocMICFailure;
            break;

        case eCSR_DISCONNECT_REASON_DEAUTH:
            pCommand->u.roamCmd.roamReason = eCsrForcedDeauth;
            break;

		case eCSR_DISCONNECT_REASON_HANDOFF:
			fHighPriority = eANI_BOOLEAN_TRUE;
			pCommand->u.roamCmd.roamReason = eCsrSmeIssuedDisassocForHandoff;
			break;

        case eCSR_DISCONNECT_REASON_UNSPECIFIED:
        case eCSR_DISCONNECT_REASON_DISASSOC:
            pCommand->u.roamCmd.roamReason = eCsrForcedDisassoc;
            break;

        default:
            break;
        }
        status = csrQueueSmeCommand(pMac, pCommand, fHighPriority);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            csrReleaseCommandRoam( pMac, pCommand );
        }
    } while( 0 );

    return( status );
}


eHalStatus csrRoamDisconnectInternal(tpAniSirGlobal pMac, eCsrRoamDisconnectReason reason)
{
    eHalStatus status = eHAL_STATUS_CSR_WRONG_STATE;

    //Not to call cancel roaming here
    //Only issue disconnect when necessary
    if(csrIsConnStateConnected(pMac) || csrIsBssTypeIBSS(pMac->roam.connectedProfile.BSSType) 
                || csrIsRoamCommandWaiting(pMac))
    {
        smsLog(pMac, LOG2, FL("called\n"));
        status = csrRoamIssueDisassociateCmd(pMac, reason);
    }

    return (status);
}


eHalStatus csrRoamDisconnect(tHalHandle hHal, eCsrRoamDisconnectReason reason)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    
    csrRoamCancelRoaming(pMac);
    csrRoamRemoveDuplicateCommand(pMac, NULL, eCsrForcedDisassoc);
    return (csrRoamDisconnectInternal(pMac, reason));
}


eHalStatus csrRoamSaveConnectedInfomation(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
                                          tSirBssDescription *pSirBssDesc, tDot11fBeaconIEs *pIes)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tDot11fBeaconIEs *pIesTemp = pIes;
    tANI_U8 index;
    
    palZeroMemory(pMac->hHdd, &pMac->roam.connectedProfile, sizeof(tCsrRoamConnectedProfile));
    pMac->roam.connectedProfile.AuthType = pProfile->negotiatedAuthType;
    pMac->roam.connectedProfile.AuthInfo = pProfile->AuthType;
    pMac->roam.connectedProfile.CBMode = pProfile->CBMode;  //*** this may not be valid
    pMac->roam.connectedProfile.EncryptionType = pProfile->negotiatedUCEncryptionType;
    pMac->roam.connectedProfile.EncryptionInfo = pProfile->EncryptionType;
    pMac->roam.connectedProfile.mcEncryptionType = pProfile->negotiatedMCEncryptionType;
    pMac->roam.connectedProfile.mcEncryptionInfo = pProfile->mcEncryptionType;
    pMac->roam.connectedProfile.BSSType = pProfile->BSSType;
    pMac->roam.connectedProfile.modifyProfileFields.uapsd_mask = pProfile->uapsd_mask;
    pMac->roam.connectedProfile.operationChannel = pSirBssDesc->channelId;
    palCopyMemory(pMac->hHdd, &pMac->roam.connectedProfile.Keys, &pProfile->Keys, sizeof(tCsrKeys));
    //Save bssid
    csrGetBssIdBssDesc(pMac, pSirBssDesc, &pMac->roam.connectedProfile.bssid);
    //save ssid
    if( NULL == pIesTemp )
    {
        status = csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc, &pIesTemp);
    }
    if(HAL_STATUS_SUCCESS(status))
    {
        if(pIesTemp->SSID.present)
        {
            pMac->roam.connectedProfile.SSID.length = pIesTemp->SSID.num_ssid;
            palCopyMemory(pMac->hHdd, pMac->roam.connectedProfile.SSID.ssId, 
                            pIesTemp->SSID.ssid, pIesTemp->SSID.num_ssid);
        }
        
        //Save the bss desc
        status = csrRoamSaveConnectedBssDesc(pMac, pSirBssDesc);

        if( ( NULL == pIes ) && pIesTemp )
        {
            if( CSR_IS_QOS_BSS(pIesTemp) )
            {
                pMac->roam.connectedProfile.qap = TRUE;
            }
            else
            {
                pMac->roam.connectedProfile.qap = FALSE;
            }

            //Free memory if it allocated locally
            palFreeMemory(pMac->hHdd, pIesTemp);
        }
        else
        {
            if( CSR_IS_QOS_BSS(pIes) )
            {
                pMac->roam.connectedProfile.qap = TRUE;
            }
            else
            {
                pMac->roam.connectedProfile.qap = FALSE;
            }
        }
    }
    //Save Qos connection
    pMac->roam.connectedProfile.qosConnection = pMac->roam.fWMMConnection;
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrFreeConnectBssDesc(pMac);
    }
    for(index = 0; index < pProfile->SSIDs.numOfSSIDs; index++)
    {
       if((pProfile->SSIDs.SSIDList[index].SSID.length == pMac->roam.connectedProfile.SSID.length) &&
          palEqualMemory(pMac->hHdd, pProfile->SSIDs.SSIDList[index].SSID.ssId, 
                         pMac->roam.connectedProfile.SSID.ssId, pMac->roam.connectedProfile.SSID.length))
       {
          pMac->roam.connectedProfile.handoffPermitted = pProfile->SSIDs.SSIDList[index].handoffPermitted;
          break;
       }
       pMac->roam.connectedProfile.handoffPermitted = FALSE;
    }
    
    return (status);
}



static void csrRoamJoinRspProcessor( tpAniSirGlobal pMac, tSirSmeJoinRsp *pSmeJoinRsp )
{
#ifdef FEATURE_WLAN_GEN6_ROAMING
   tCsrScanResultFilter *pScanFilter = NULL;
   tScanResultHandle hBSSList;
   tCsrRoamInfo roamInfo;
   eHalStatus status;
#endif
   tListElem *pEntry = NULL;
   tSmeCmd *pCommand = NULL;

   //The head of the active list is the request we sent
   pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
   if(pEntry)
   {
       pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
   }
    if ( eSIR_SME_SUCCESS == pSmeJoinRsp->statusCode ) 
    {
        pMac->roam.cJoinAttemps = 0;
        if(pCommand && eCsrSmeIssuedAssocToSimilarAP == pCommand->u.roamCmd.roamReason)
        {
           sme_QosCsrEventInd(pMac, SME_QOS_CSR_HANDOFF_COMPLETE, NULL);
        }
        csrRoamComplete( pMac, eCsrJoinSuccess, (void *)pSmeJoinRsp );
    }
    else
    {
        tANI_U32 roamId = 0;
        
        //The head of the active list is the request we sent
        //Try to get back the same profile and roam again
        if(pCommand)
        {
            roamId = pCommand->u.roamCmd.roamId;
        }

        pMac->roam.joinFailStatusCode.statusCode = pSmeJoinRsp->statusCode;
        pMac->roam.joinFailStatusCode.reasonCode = pSmeJoinRsp->protStatusCode;
        smsLog( pMac, LOGW, "SmeJoinReq failed with statusCode= 0x%08lX [%d]\n", pSmeJoinRsp->statusCode, pSmeJoinRsp->statusCode );
#ifdef FEATURE_WLAN_GEN6_ROAMING
        //might need to add logic to make sure we get back to the old AP
        //update the scanlist & bssid
        if( eCsrSmeIssuedAssocToSimilarAP == pCommand->u.roamCmd.roamReason)
        {
            //check if this is the retry for original AP case
            if(csrIsBssidMatch(pMac, (tCsrBssid *)&pMac->roam.handoffInfo.currSta.bssid, 
                              (tCsrBssid *) pCommand->u.roamCmd.roamProfile.BSSIDs.bssid))
            {
                pCommand = NULL;
                sme_QosCsrEventInd(pMac, SME_QOS_CSR_HANDOFF_FAILURE, NULL);
            }
            else
            {
                //Complete the last association attemp because a new one is about to be tried
                //We need to indicate to HDD that we are done with this one.
                palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                roamInfo.pBssDesc = pCommand->u.roamCmd.pLastRoamBss;     //this shall not be NULL
                roamInfo.statusCode = pMac->roam.joinFailStatusCode.statusCode;
                roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                csrRoamCallCallback(pMac, &roamInfo, pCommand->u.roamCmd.roamId, 
                                      eCSR_ROAM_ASSOCIATION_COMPLETION, 
                                      eCSR_ROAM_RESULT_NOT_ASSOCIATED);
                //try original AP
                csrRoamCreateHandoffProfile(pMac, pMac->roam.handoffInfo.currSta.bssid);
                status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
                if(HAL_STATUS_SUCCESS(status))
                {
                    palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
                    status = csrRoamPrepareFilterFromProfile(pMac, &pMac->roam.handoffInfo.handoffProfile, pScanFilter);
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        //use the scan filter to get the exact entry using the BSSID 
                        status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
                        pCommand->u.roamCmd.hBSSList = hBSSList;
                        pCommand->u.roamCmd.pRoamBssEntry = NULL;
                        palCopyMemory(pMac->hHdd, pCommand->u.roamCmd.roamProfile.BSSIDs.bssid, 
                                    pMac->roam.handoffInfo.currSta.bssid,
                                    sizeof( tCsrBssid ));
                    }
                    else {
                        smsLog( pMac, LOGE, FL(" csrScanGetResult failed\n"));
                    }
                    csrFreeScanFilter(pMac, pScanFilter);
                    palFreeMemory( pMac->hHdd, pScanFilter );
                }
                else
                {
                    smsLog(pMac, LOGE, FL(" fail to allocate memory for scan filter\n"));
                }
            }
        }
#endif
        if (pCommand)
        {
            csrRoam(pMac, pCommand);
        }    
        else
        {
#ifdef FEATURE_WLAN_GEN6_ROAMING
           status = csrRoamLostLinkAfterhandoffFailure(pMac);
           if(!HAL_STATUS_SUCCESS(status))
           {
              smsLog( pMac, LOGW, "Lost Link roaming failed after handoff failure, indicating upper layer\n");
            csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
        }
#else
           csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
#endif
    }
}
}


eHalStatus csrRoamIssueJoin( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, 
                             tDot11fBeaconIEs *pIes,
                             tCsrRoamProfile *pProfile, tANI_U32 roamId )
{
    eHalStatus status;

    smsLog( pMac, LOG1, "Attempting to Join Bssid= %02x-%02x-%02x-%02x-%02x-%02x\n", 
                  pSirBssDesc->bssId[ 0 ],pSirBssDesc->bssId[ 1 ],pSirBssDesc->bssId[ 2 ],
                  pSirBssDesc->bssId[ 3 ],pSirBssDesc->bssId[ 4 ],pSirBssDesc->bssId[ 5 ] );
    
    // Set the roaming substate to 'join attempt'...
    csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOIN_REQ );

    // attempt to Join this BSS...
    status = csrSendJoinReqMsg( pMac, pSirBssDesc, pProfile, pIes );

    return (status);
}


void csrRoamIssueReassociate( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, 
                              tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile)
{
    csrRoamStateChange( pMac, eCSR_ROAMING_STATE_JOINING );

    // Set the roaming substate to 'join attempt'...
    csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_REASSOC_REQ );

    // attempt to Join this BSS...
    csrSendSmeReassocReqMsg( pMac, pSirBssDesc, pIes, pProfile );
}



void csrRoamReissueRoamCommand(tpAniSirGlobal pMac)
{
    tListElem *pEntry;
    tSmeCmd *pCommand;
            
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
    if(pEntry)
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if ( eSmeCommandRoam == pCommand->command )
        {
            if(eCsrStopRoaming == csrRoamJoinNextBss(pMac, pCommand, eANI_BOOLEAN_TRUE))
            {
                smsLog(pMac, LOGW, " Failed to reissue join command after disassociated\n");
                csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
            }
        }
        else
        {
            smsLog(pMac, LOGW, "  Command is not roaming after disassociated\n");
        }
    }
    else 
    {
        smsLog(pMac, LOGE, "   Disassoc rsp cannot continue because no command is available\n");
    }
}


tANI_BOOLEAN csrIsRoamCommandWaiting(tpAniSirGlobal pMac)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;
    tListElem *pEntry;
    tSmeCmd *pCommand = NULL;

    //alwasy lock active list before locking pending list
    csrLLLock( &pMac->sme.smeCmdActiveList );
    pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK);
    if(pEntry)
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if(eSmeCommandRoam == pCommand->command)
        {
            fRet = eANI_BOOLEAN_TRUE;
        }
    }
    if(eANI_BOOLEAN_FALSE == fRet)
    {
        csrLLLock(&pMac->sme.smeCmdPendingList);
        pEntry = csrLLPeekHead(&pMac->sme.smeCmdPendingList, LL_ACCESS_NOLOCK);
        while(pEntry)
        {
            pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
            if(eSmeCommandRoam == pCommand->command)
            {
                fRet = eANI_BOOLEAN_TRUE;
                break;
            }
            pEntry = csrLLNext(&pMac->sme.smeCmdPendingList, pEntry, LL_ACCESS_NOLOCK);
        }
        csrLLUnlock(&pMac->sme.smeCmdPendingList);
    }
    csrLLUnlock( &pMac->sme.smeCmdActiveList );

    return (fRet);
}


tANI_BOOLEAN csrIsCommandWaiting(tpAniSirGlobal pMac)
{
    tANI_BOOLEAN fRet = eANI_BOOLEAN_FALSE;

    //alwasy lock active list before locking pending list
    csrLLLock( &pMac->sme.smeCmdActiveList );
    fRet = csrLLIsListEmpty(&pMac->sme.smeCmdActiveList, LL_ACCESS_NOLOCK);
    if(eANI_BOOLEAN_FALSE == fRet)
    {
        fRet = csrLLIsListEmpty(&pMac->sme.smeCmdPendingList, LL_ACCESS_LOCK);
    }
    csrLLUnlock( &pMac->sme.smeCmdActiveList );

    return (fRet);
}


static void csrRoamingStateConfigCnfProcessor( tpAniSirGlobal pMac, tANI_U32 result )
{
    tListElem *pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
    tCsrScanResult *pScanResult = NULL;
    tSirBssDescription *pBssDesc = NULL;
    tSmeCmd *pCommand = NULL;

    if(NULL == pEntry)
    {
        smsLog(pMac, LOGW, "   CFG_CNF with active list empty\n");
    }
    else if(CSR_IS_ROAMING(pMac) && pMac->roam.fCancelRoaming)
    {
        //the roaming is cancelled. Simply complete the command
        smsLog(pMac, LOGW, FL("  Roam command cancelled\n"));
        csrRoamComplete(pMac, eCsrNothingToJoin, NULL); 
    }
    else
    {
        pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
        if ( CCM_IS_RESULT_SUCCESS(result) )
        {
            smsLog(pMac, LOG2, "Cfg sequence complete\n");
            // Successfully set the configuration parameters for the new Bss.  Attempt to
            // join the roaming Bss.
            if(pCommand->u.roamCmd.pRoamBssEntry)
            {
                pScanResult = GET_BASE_ADDR(pCommand->u.roamCmd.pRoamBssEntry, tCsrScanResult, Link);
                pBssDesc = &pScanResult->Result.BssDescriptor;
            }
            if ( csrIsBssTypeIBSS( pCommand->u.roamCmd.roamProfile.BSSType ) )
            {
                if(!HAL_STATUS_SUCCESS(csrRoamIssueStartIbss( pMac, &pMac->roam.IbssParams, &pCommand->u.roamCmd.roamProfile, pBssDesc, pCommand->u.roamCmd.roamId )))
                {
                    smsLog(pMac, LOGW, " CSR start IBSS failed\n");
                    //We need to complete the command
                    csrRoamComplete(pMac, eCsrStartIbssFailure, NULL);
                }
            }
            else
            {
#if defined(VOSS_ENABLED)
                VOS_ASSERT(pScanResult->Result.pvIes);
#endif
                // If we are roaming TO an Infrastructure BSS...
                if ( csrIsInfraBssDesc( pBssDesc ) )
                {
                    // ..and currently in an Infrastructure connection....
                    if( csrIsConnStateConnectedInfra( pMac ) )
                    {
                        // ...and the SSIDs are equal, then we Reassoc.
                        if (  csrIsSsidEqual( pMac, pMac->roam.pConnectBssDesc, pBssDesc, 
                                                (tDot11fBeaconIEs *)( pScanResult->Result.pvIes ) ) )
                        // ..and currently in an infrastructure connection
                        {
                            // then issue a Reassoc.
                            pCommand->u.roamCmd.fReassoc = eANI_BOOLEAN_TRUE;
                            csrRoamIssueReassociate( pMac, pBssDesc, (tDot11fBeaconIEs *)( pScanResult->Result.pvIes ),
                                                        &pCommand->u.roamCmd.roamProfile );
                        }
                        else
                        {
                                                     
                            // otherwise, we have to issue a new Join request to LIM because we disassociated from the
                            // previously associated AP.
                            if(!HAL_STATUS_SUCCESS(csrRoamIssueJoin( pMac, pBssDesc, 
													(tDot11fBeaconIEs *)( pScanResult->Result.pvIes ), 
                                                    &pCommand->u.roamCmd.roamProfile, pCommand->u.roamCmd.roamId )))
                            {
                                //try something else
                                csrRoam( pMac, pCommand );
                            }
                        }
                    }
                    else
                    {
                        // else we are not connected and attempting to Join.  Issue the
                        // Join request.
                        if(!HAL_STATUS_SUCCESS(csrRoamIssueJoin( pMac, pBssDesc, 
											(tDot11fBeaconIEs *)( pScanResult->Result.pvIes ),
                                            &pCommand->u.roamCmd.roamProfile, pCommand->u.roamCmd.roamId )))
                        {
                            //try something else
                            csrRoam( pMac, pCommand );
                        }
                    }
                }//if ( csrIsInfraBssDesc( pBssDesc ) )
                else
                {
                    smsLog(pMac, LOGW, FL("  found BSSType mismatching the one in BSS description\n"));
                }
            }//else
        }//if ( WNI_CFG_SUCCESS == result )
        else
        {
            // In the event the configuration failed,  for infra let the roam processor 
            //attempt to join something else...
            if( pCommand->u.roamCmd.pRoamBssEntry && CSR_IS_INFRASTRUCTURE( &pCommand->u.roamCmd.roamProfile ) )
            {
            csrRoam(pMac, pCommand);
            }
            else
            {
                //We need to complete the command
                if ( csrIsBssTypeIBSS( pCommand->u.roamCmd.roamProfile.BSSType ) )
                {
                    csrRoamComplete(pMac, eCsrStartIbssFailure, NULL);
                }
                else
                {
                    csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
                }
            }
        }
    }//we have active entry
}


static void csrRoamRoamingStateAuthRspProcessor( tpAniSirGlobal pMac, tSirSmeAuthRsp *pSmeAuthRsp )
{
    //No one is sending eWNI_SME_AUTH_REQ to PE.
    smsLog(pMac, LOGW, FL("is no-op\n"));
    if ( eSIR_SME_SUCCESS == pSmeAuthRsp->statusCode ) 
    {
        smsLog( pMac, LOGW, "CSR SmeAuthReq Successful\n" );
        // Successfully authenticated with a new Bss.  Attempt to stop the current Bss and
        // join the new one...
        /***pBssDesc = profGetRoamingBssDesc( pAdapter, &pHddProfile );

        roamStopNetwork( pAdapter, &pBssDesc->SirBssDescription );***/
    }
    else {
        smsLog( pMac, LOGW, "CSR SmeAuthReq failed with statusCode= 0x%08lX [%d]\n", pSmeAuthRsp->statusCode, pSmeAuthRsp->statusCode );
        /***profHandleLostLinkAfterReset(pAdapter);
        // In the event the authenticate fails, let the roam processor attempt to join something else...
        roamRoam( pAdapter );***/
    }
}


static void csrRoamRoamingStateReassocRspProcessor( tpAniSirGlobal pMac, tSirSmeReassocRsp *pSmeReassocRsp )
{
    eCsrRoamCompleteResult result;
    
    if ( eSIR_SME_SUCCESS == pSmeReassocRsp->statusCode ) 
    {
        smsLog( pMac, LOGW, "CSR SmeReassocReq Successful\n" );
        result = eCsrReassocSuccess;
        csrRoamComplete( pMac, result, NULL );
    }
    else
    {
        smsLog( pMac, LOGW, "CSR SmeReassocReq failed with statusCode= 0x%08lX [%d]\n", pSmeReassocRsp->statusCode, pSmeReassocRsp->statusCode );
        result = eCsrReassocFailure;
        // In the event that the Reassociation fails, then we need to Disassociate the current association and keep
        // roaming.  Note that we will attempt to Join the AP instead of a Reassoc since we may have attempted a
        // 'Reassoc to self', which AP's that don't support Reassoc will force a Disassoc.
        //The disassoc rsp message will remove the command from active list
        if(!HAL_STATUS_SUCCESS(csrRoamIssueDisassociate( pMac, eCSR_ROAM_SUBSTATE_DISASSOC_REASSOC_FAILURE, FALSE )))
        {
            csrRoamComplete( pMac, eCsrJoinFailure, NULL );
        }
    }
}


static void csrRoamRoamingStateStopBssRspProcessor(tpAniSirGlobal pMac, tSirSmeRsp *pSmeReassocRsp)
{

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        vos_log_ibss_pkt_type *pIbssLog;

        WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
        if(pIbssLog)
        {
            pIbssLog->eventId = WLAN_IBSS_EVENT_STOP_RSP;
            if(eSIR_SME_SUCCESS != pSmeReassocRsp->statusCode)
            {
                pIbssLog->status = WLAN_IBSS_STATUS_FAILURE;
            }
            WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
        }
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
    if(CSR_IS_ROAM_SUBSTATE_STOP_BSS_REQ( pMac ))
    {
        csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
    }
    else if(CSR_IS_ROAM_SUBSTATE_DISCONNECT_CONTINUE( pMac ))
    {
        csrRoamReissueRoamCommand(pMac);
    }
}


static void csrRoamRoamingStateDisassocRspProcessor( tpAniSirGlobal pMac, tSirSmeDisassocRsp *pSmeDisassocRsp )
{
    tSirResultCodes statusCode;
#ifdef FEATURE_WLAN_GEN6_ROAMING
    tScanResultHandle hBSSList;
    tANI_BOOLEAN fCallCallback, fRemoveCmd;
    eHalStatus status;
    tCsrRoamInfo roamInfo;
    tCsrScanResultFilter *pScanFilter = NULL;
    tANI_U32 roamId = 0;
    tCsrRoamProfile *pCurRoamProfile = NULL;
	 tListElem *pEntry = NULL;
	 tSmeCmd *pCommand = NULL;
#endif
    pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
    statusCode = csrGetDisassocRspStatusCode( pSmeDisassocRsp );

    if ( CSR_IS_ROAM_SUBSTATE_DISASSOC_NO_JOIN( pMac ) )
    {
        csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
    }
    else if ( CSR_IS_ROAM_SUBSTATE_DISASSOC_FORCED( pMac ) )
    {
        if ( eSIR_SME_SUCCESS == statusCode )
        {
            smsLog( pMac, LOG2, "CSR SmeDisassocReq force disassociated Successfully\n" );
            //A callback to HDD will be issued from csrRoamComplete so no need to do anything here
        } 
        csrRoamComplete( pMac, eCsrNothingToJoin, NULL );
    }
#ifdef FEATURE_WLAN_GEN6_ROAMING
    else if ( CSR_IS_ROAM_SUBSTATE_DISASSOC_HO( pMac ) )
    {
        pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
        if ( pEntry )
        {
			pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );

			// If the head of the queue is Active and it is a ROAM command, remove
			// and put this on the Free queue.
			if ( eSmeCommandRoam == pCommand->command )
			{
				//we need to process the result first before removing it from active list because state changes 
				//still happening insides roamQProcessRoamResults so no other roam command should be issued
				fRemoveCmd = csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK );
				if(pCommand->u.roamCmd.fReleaseProfile)
				{
					csrReleaseProfile(pMac, &pCommand->u.roamCmd.roamProfile);
					pCommand->u.roamCmd.fReleaseProfile = eANI_BOOLEAN_FALSE;
				}

                if( fRemoveCmd )
                {
				    csrReleaseCommandRoam( pMac, pCommand );
                }
                else
                {
                    smsLog( pMac, LOGE, "  ********csrRoamRoamingStateDisassocRspProcessor fail to remove cmd reason %d\n",
                        pCommand->u.roamCmd.roamReason );
                }
			}
			else
			{
				smsLog( pMac, LOGW, "CSR: Roam Completion called but ROAM command is not ACTIVE ...\n" );
			}
		}
		else
		{
			smsLog( pMac, LOGW, "CSR: Roam Completion called but NO commands are ACTIVE ...\n" );
		}

        //notify HDD for handoff, providing the BSSID too
        roamInfo.reasonCode = eCsrRoamReasonBetterAP;
        palCopyMemory(pMac->hHdd, &roamInfo.bssid, 
                    pMac->roam.handoffInfo.handoffProfile.BSSIDs.bssid, 
                    sizeof( tCsrBssid ));

        csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);

		status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
		if(HAL_STATUS_SUCCESS(status))
		{
			palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
			status = csrRoamPrepareFilterFromProfile(pMac, &pMac->roam.handoffInfo.handoffProfile, pScanFilter);
            if(!HAL_STATUS_SUCCESS(status))
            {
                smsLog(pMac, LOGE, FL(" csrRoamPrepareFilterFromProfile fail to create scan filter\n"));
            }
            //use the scan filter to get the exact entry using the BSSID 

            status = csrScanGetResult(pMac, pScanFilter, &hBSSList);
            if(HAL_STATUS_SUCCESS(status))
            {
                //copy over the connected profile to apply the same for this connection as well
                if(HAL_STATUS_SUCCESS(palAllocateMemory(pMac->hHdd, (void **)&pCurRoamProfile, sizeof(tCsrRoamProfile))))
                {
                    palZeroMemory(pMac->hHdd, pCurRoamProfile, sizeof(tCsrRoamProfile));
                    csrRoamCopyProfile(pMac, pCurRoamProfile, pMac->roam.pCurRoamProfile);
                }
                //make sure to put it at the head of the cmd queue
                status = csrRoamIssueConnect(pMac, pCurRoamProfile, hBSSList, eCsrSmeIssuedAssocToSimilarAP, 
                                           roamId, eANI_BOOLEAN_TRUE, eANI_BOOLEAN_FALSE);
                if(!HAL_STATUS_SUCCESS(status))
                {
                    //msg
                    fCallCallback = eANI_BOOLEAN_TRUE;
                }
                /* Notify sub-modules like QoS etc. that handoff happening         */
                sme_QosCsrEventInd(pMac, SME_QOS_CSR_HANDOFF_ASSOC_REQ, NULL);
                palFreeMemory(pMac->hHdd, pCurRoamProfile);
            }
            else
            {
                //msg
                smsLog( pMac, LOGE,"csrRoamRoamingStateDisassocRspProcessor: csrScanGetResult failed");
                // should have asserted, sending up roam complete instead. Let upper layer
                // decide what to do next
                csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_ROAMING_COMPLETION, eCSR_ROAM_RESULT_FAILURE);
            }
		}
        else
        {
            smsLog(pMac, LOGE, FL(" fail to allocate memory for scan filter\n"));
            csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_ROAMING_COMPLETION, eCSR_ROAM_RESULT_FAILURE);
        }
        if( pScanFilter )
        {
            csrFreeScanFilter(pMac, pScanFilter);
            palFreeMemory( pMac->hHdd, pScanFilter );
        }
    }
#endif
    else
    {
        // Disassoc due to Reassoc failure falls into this codepath....

        if ( eSIR_SME_SUCCESS == statusCode )
        {
            // Successfully disassociated from the 'old' Bss...
            //
            // We get Disassociate response in three conditions.
            // - First is the case where we are disasociating from an Infra Bss to start an IBSS.
            // - Second is the when we are disassociating from an Infra Bss to join an IBSS or a new
            // Infrastructure network.
            // - Third is where we are doing an Infra to Infra roam between networks with different
            // SSIDs.  In all cases, we set the new Bss configuration here and attempt to join
            
            smsLog( pMac, LOG2, "CSR SmeDisassocReq disassociated Successfully\n" );
        }
        else
        {
            smsLog( pMac, LOGW, "SmeDisassocReq failed with statusCode= 0x%08lX\n", statusCode );
        }
        //We are not done yet. Get the data and continue roaming
        csrRoamReissueRoamCommand(pMac);
    }

}


static void csrRoamRoamingStateDeauthRspProcessor( tpAniSirGlobal pMac, tSirSmeDeauthRsp *pSmeRsp )
{
    tSirResultCodes statusCode;

    //No one is sending eWNI_SME_DEAUTH_REQ to PE.
    smsLog(pMac, LOGW, FL("is no-op\n"));
    statusCode = csrGetDeAuthRspStatusCode( pSmeRsp );

    if ( eSIR_SME_SUCCESS == statusCode ) 
    {

        smsLog( pMac, LOGW, "CSR SmeDeauthReq Successful\n" );

        // Successfully disassociated from the 'old' Bss...
        //
        // We get Disassociate response in two conditions.  First is the case where
        // we are disasociating from an Infra Bss to start an IBSS.  Second is the
        // when we are disassociating from an Infra Bss to join an IBSS or a new
        // Infrastructure network.  We have to distinguish between these two
        // cases by looking at the current Profile...
    }
    else {
        smsLog( pMac, LOGW, "SmeDeauthReq failed with statusCode= 0x%08lX\n", statusCode );
    }
}


static void csrRoamRoamingStateStartBssRspProcessor( tpAniSirGlobal pMac, tSirSmeStartBssRsp *pSmeStartBssRsp )
{
    eCsrRoamCompleteResult result;
    
    if ( eSIR_SME_SUCCESS == pSmeStartBssRsp->statusCode ) 
    {
        smsLog( pMac, LOGW, "SmeStartBssReq Successful\n" );
        result = eCsrStartIbssSuccess;
    }
    else {
        smsLog( pMac, LOGW, "SmeStartBssReq failed with statusCode= 0x%08lX\n", pSmeStartBssRsp->statusCode );
        //Let csrRoamComplete decide what to do
        result = eCsrStartIbssFailure;
    }
    csrRoamComplete( pMac, result, &pSmeStartBssRsp->bssDescription );
}

void csrRoamingStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf )
{
    tSirSmeRsp *pSmeRsp;
    tSmeIbssPeerInd *pIbssPeerInd;
    tCsrRoamInfo roamInfo;

    pSmeRsp = (tSirSmeRsp *)pMsgBuf;

    smsLog( pMac, LOG2, "Message %d[0x%04X] received in substate %d\n",
                pSmeRsp->messageType, pSmeRsp->messageType,
                pMac->roam.curSubState );
#if defined ANI_PRODUCT_TYPE_AP
    pSmeRsp->messageType = pal_be16_to_cpu(pSmeRsp->messageType);
    pSmeRsp->length = pal_be16_to_cpu(pSmeRsp->length);
    pSmeRsp->statusCode = pal_be32_to_cpu(pSmeRsp->statusCode);
#else
    pSmeRsp->messageType = (pSmeRsp->messageType);
    pSmeRsp->length = (pSmeRsp->length);
    pSmeRsp->statusCode = (pSmeRsp->statusCode);
#endif
    switch (pSmeRsp->messageType) 
    {
        
        case eWNI_SME_JOIN_RSP:      // in Roaming state, process the Join response message...
            if (CSR_IS_ROAM_SUBSTATE_JOIN_REQ(pMac))
            {
                //We sent a JOIN_REQ
                csrRoamJoinRspProcessor( pMac, (tSirSmeJoinRsp *)pSmeRsp );
            }
            break;
                
        case eWNI_SME_AUTH_RSP:       // or the Authenticate response message...
            if (CSR_IS_ROAM_SUBSTATE_AUTH_REQ( pMac ) ) 
            {
                //We sent a AUTH_REQ
                csrRoamRoamingStateAuthRspProcessor( pMac, (tSirSmeAuthRsp *)pSmeRsp );
            }
            break;
                
        case eWNI_SME_REASSOC_RSP:     // or the Reassociation response message...
            if (CSR_IS_ROAM_SUBSTATE_REASSOC_REQ( pMac ) ) 
            {
                csrRoamRoamingStateReassocRspProcessor( pMac, (tSirSmeReassocRsp *)pSmeRsp );
            }
            break;
                   
        case eWNI_SME_STOP_BSS_RSP:    // or the Stop Bss response message...
            {
                csrRoamRoamingStateStopBssRspProcessor(pMac, pSmeRsp);
            }
            break;
                
        case eWNI_SME_DISASSOC_RSP:    // or the Disassociate response message...
            if ( CSR_IS_ROAM_SUBSTATE_DISASSOC_REQ( pMac )      ||
                 CSR_IS_ROAM_SUBSTATE_DISASSOC_NO_JOIN( pMac )  ||
                 CSR_IS_ROAM_SUBSTATE_REASSOC_FAIL( pMac )      ||
                 CSR_IS_ROAM_SUBSTATE_DISASSOC_FORCED( pMac )   ||
                 CSR_IS_ROAM_SUBSTATE_DISCONNECT_CONTINUE( pMac ) ||
//HO
                 CSR_IS_ROAM_SUBSTATE_DISASSOC_HO( pMac )         )
            {
                csrRoamRoamingStateDisassocRspProcessor( pMac, (tSirSmeDisassocRsp *)pSmeRsp );
            }
            break;
                   
        case eWNI_SME_DEAUTH_RSP:    // or the Deauthentication response message...
            if ( CSR_IS_ROAM_SUBSTATE_DEAUTH_REQ( pMac ) ) 
            {
                csrRoamRoamingStateDeauthRspProcessor( pMac, (tSirSmeDeauthRsp *)pSmeRsp );
            }
            break;
                   
        case eWNI_SME_START_BSS_RSP:      // or the Start BSS response message...
            if (CSR_IS_ROAM_SUBSTATE_START_BSS_REQ( pMac ) ) 
            {
                csrRoamRoamingStateStartBssRspProcessor( pMac, (tSirSmeStartBssRsp *)pSmeRsp );
            } 
            break;
                   
        case WNI_CFG_SET_CNF:    // process the Config Confirm messages when we are in 'Config' substate...
            if ( CSR_IS_ROAM_SUBSTATE_CONFIG( pMac ) ) 
            {
                csrRoamingStateConfigCnfProcessor( pMac, ((tCsrCfgSetRsp *)pSmeRsp)->respStatus );
            }
            break;

        //In case CSR issues STOP_BSS, we need to tell HDD about peer departed becasue PE is removing them
        case eWNI_SME_IBSS_PEER_DEPARTED_IND:
            pIbssPeerInd = (tSmeIbssPeerInd*)pSmeRsp;
            smsLog(pMac, LOGE, "CSR: Peer departed notification from LIM in joining state\n");
            palZeroMemory( pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo) );
			roamInfo.staId = (tANI_U8)pIbssPeerInd->staId;
            roamInfo.ucastSig = (tANI_U8)pIbssPeerInd->ucastSig;
            roamInfo.bcastSig = (tANI_U8)pIbssPeerInd->bcastSig;
			palCopyMemory(pMac->hHdd, &roamInfo.peerMac, pIbssPeerInd->peerAddr, sizeof(tCsrBssid));
            csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_CONNECT_STATUS_UPDATE, eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED);
            break;

        default:
            smsLog( pMac, LOG1, "Unexpected message type = %d[0x%X] received in substate %d\n",
                      pSmeRsp->messageType, pSmeRsp->messageType,
                      pMac->roam.curSubState );
            //If we are connected, check the link status change 
			if(!csrIsConnStateDisconnected(pMac))
			{
				csrRoamCheckForLinkStatusChange( pMac, pSmeRsp );
			}
            break;          
    }
}


void csrRoamJoinedStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf )
{
    tSirSmeRsp *pSirMsg = (tSirSmeRsp *)pMsgBuf;

    switch (pSirMsg->messageType) 
    {
       case eWNI_SME_GET_STATISTICS_RSP:
          smsLog( pMac, LOGW, FL("Stats rsp from PE\n"));
          csrRoamStatsRspProcessor( pMac, pSirMsg );
          break;
       default:
          csrRoamCheckForLinkStatusChange( pMac, pSirMsg );
          break;
    }

}


eHalStatus csrRoamIssueSetContextReq( tpAniSirGlobal pMac, eCsrEncryptionType EncryptType, 
                                     tSirBssDescription *pBssDescription,
                                tSirMacAddr *bssId, tANI_BOOLEAN addKey,
                                 tANI_BOOLEAN fUnicast, tAniKeyDirection aniKeyDirection, 
                                 tANI_U8 keyId, tANI_U16 keyLength, 
                                 tANI_U8 *pKey, tANI_U8 paeRole )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tAniEdType edType;
    
    if(eCSR_ENCRYPT_TYPE_UNKNOWN == EncryptType)
    {
        EncryptType = eCSR_ENCRYPT_TYPE_NONE; //***
    }
    
    edType = csrTranslateEncryptTypeToEdType( EncryptType );
    
    // Allow 0 keys to be set for the non-WPA encrypt types...  For WPA encrypt types, the num keys must be non-zero
    // or LIM will reject the set context (assumes the SET_CONTEXT does not occur until the keys are distrubuted).
    if ( CSR_IS_ENC_TYPE_STATIC( EncryptType ) ||
           addKey )     
    {
        tCsrRoamSetKey setKey;

        setKey.encType = EncryptType;
        setKey.keyDirection = aniKeyDirection;    //Tx, Rx or Tx-and-Rx
        palCopyMemory( pMac->hHdd, &setKey.peerMac, bssId, sizeof(tCsrBssid) );   
        setKey.paeRole = paeRole;      //0 for supplicant
        setKey.keyId = keyId;  // Kye index
        setKey.keyLength = keyLength;  
        if( keyLength )
        {
            palCopyMemory( pMac->hHdd, setKey.Key, pKey, keyLength );
        }
        status = csrRoamIssueSetKeyCommand( pMac, &setKey, 0 );
    }

    return (status);
}


eHalStatus csrRoamIssueSetKeyCommand( tpAniSirGlobal pMac, tCsrRoamSetKey *pSetKey, tANI_U32 roamId )
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pCommand = NULL;

    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(NULL == pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandSetKey;
        // validate the key length,  Adjust if too long...
        // for static WEP the keys are not set thru' SetContextReq
        if ( ( eCSR_ENCRYPT_TYPE_WEP40 == pSetKey->encType ) || 
             ( eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == pSetKey->encType ) ) 
        {
            //KeyLength maybe 0 for static WEP
            if( pSetKey->keyLength )
            {
                if ( pSetKey->keyLength < CSR_WEP40_KEY_LEN ) 
                {
                    smsLog( pMac, LOGW, "Invalid WEP40 keylength [= %d] in SetContext call\n", pSetKey->keyLength );
                    break;        
                }
                
                pCommand->u.setKeyCmd.keyLength = CSR_WEP40_KEY_LEN;
                palCopyMemory( pMac->hHdd, pCommand->u.setKeyCmd.Key, pSetKey->Key, CSR_WEP40_KEY_LEN );
            }
        }
        else if ( ( eCSR_ENCRYPT_TYPE_WEP104 == pSetKey->encType ) || 
             ( eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == pSetKey->encType ) ) 
        {
            //KeyLength maybe 0 for static WEP
            if( pSetKey->keyLength )
            {
                if ( pSetKey->keyLength < CSR_WEP104_KEY_LEN ) 
                {
                    smsLog( pMac, LOGW, "Invalid WEP104 keylength [= %d] in SetContext call\n", pSetKey->keyLength );
                    break;        
                }
                
                pCommand->u.setKeyCmd.keyLength = CSR_WEP104_KEY_LEN;
                palCopyMemory( pMac->hHdd, pCommand->u.setKeyCmd.Key, pSetKey->Key, CSR_WEP104_KEY_LEN );
            }
        }
        else if ( eCSR_ENCRYPT_TYPE_TKIP == pSetKey->encType ) 
        {
            if ( pSetKey->keyLength < CSR_TKIP_KEY_LEN )
            {
                smsLog( pMac, LOGW, "Invalid TKIP keylength [= %d] in SetContext call\n", pSetKey->keyLength );
                break;
            }
            pCommand->u.setKeyCmd.keyLength = CSR_TKIP_KEY_LEN;
            palCopyMemory( pMac->hHdd, pCommand->u.setKeyCmd.Key, pSetKey->Key, CSR_TKIP_KEY_LEN );
        }
        else if ( eCSR_ENCRYPT_TYPE_AES == pSetKey->encType ) 
        {
            if ( pSetKey->keyLength < CSR_AES_KEY_LEN )
            {
                smsLog( pMac, LOGW, "Invalid AES/CCMP keylength [= %d] in SetContext call\n", pSetKey->keyLength );
                break;
            }
            pCommand->u.setKeyCmd.keyLength = CSR_AES_KEY_LEN;
            palCopyMemory( pMac->hHdd, pCommand->u.setKeyCmd.Key, pSetKey->Key, CSR_AES_KEY_LEN );
        }
        status = eHAL_STATUS_SUCCESS;
        pCommand->u.setKeyCmd.roamId = roamId;
        pCommand->u.setKeyCmd.encType = pSetKey->encType;
        pCommand->u.setKeyCmd.keyDirection = pSetKey->keyDirection;    //Tx, Rx or Tx-and-Rx
        palCopyMemory( pMac->hHdd, &pCommand->u.setKeyCmd.peerMac, &pSetKey->peerMac, sizeof(tCsrBssid) );   
        pCommand->u.setKeyCmd.paeRole = pSetKey->paeRole;      //0 for supplicant
        pCommand->u.setKeyCmd.keyId = pSetKey->keyId;
        //Always put set key to the head of the Q because it is the only thing to get executed in case of WT_KEY state
        status = csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
        }
    } while (0);

    if( !HAL_STATUS_SUCCESS( status ) && ( NULL != pCommand ) )
    {
        csrReleaseCommandSetKey( pMac, pCommand );
    }

    return( status );
}


eHalStatus csrRoamIssueRemoveKeyCommand( tpAniSirGlobal pMac, tCsrRoamRemoveKey *pRemoveKey, tANI_U32 roamId )
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tSmeCmd *pCommand = NULL;
    tANI_BOOLEAN fImediate = eANI_BOOLEAN_TRUE;

    do
    {
        pCommand = csrGetCommandBuffer(pMac);
        if(NULL == pCommand)
        {
            status = eHAL_STATUS_RESOURCES;
            break;
        }
        pCommand->command = eSmeCommandRemoveKey;
        pCommand->u.removeKeyCmd.roamId = roamId;
        pCommand->u.removeKeyCmd.encType = pRemoveKey->encType;
        palCopyMemory( pMac->hHdd, &pCommand->u.removeKeyCmd.peerMac, &pRemoveKey->peerMac, sizeof(tSirMacAddr) );
        pCommand->u.removeKeyCmd.keyId = pRemoveKey->keyId;
        if( CSR_IS_WAIT_FOR_KEY( pMac ) )
        {
            //in this case, put it to the end of the Q incase there is a set key pending.
            fImediate = eANI_BOOLEAN_FALSE;
        }

        status = csrQueueSmeCommand(pMac, pCommand, fImediate);
        if( !HAL_STATUS_SUCCESS( status ) )
        {
            smsLog( pMac, LOGE, FL(" fail to send message status = %d\n"), status );
            break;
        }
    } while (0);

    if( !HAL_STATUS_SUCCESS( status ) && ( NULL != pCommand ) )
    {
        csrReleaseCommandRemoveKey( pMac, pCommand );
    }

    return (status );
}


eHalStatus csrRoamProcessSetKeyCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status;
    tANI_U8 numKeys = ( pCommand->u.setKeyCmd.keyLength ) ? 1 : 0;
    tAniEdType edType = csrTranslateEncryptTypeToEdType( pCommand->u.setKeyCmd.encType );
    tANI_BOOLEAN fUnicast = ( pCommand->u.setKeyCmd.peerMac[0] == 0xFF ) ? eANI_BOOLEAN_FALSE : eANI_BOOLEAN_TRUE;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_EVENT_DEF(setKeyEvent, vos_event_wlan_security_payload_type);

    if(eCSR_ENCRYPT_TYPE_NONE != edType)
    {
        palZeroMemory(pMac->hHdd, &setKeyEvent, sizeof(vos_event_wlan_security_payload_type));
        if( *(( tANI_U8 *)&pCommand->u.setKeyCmd.peerMac) & 0x01 )
        {
            setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_GTK_REQ;
            setKeyEvent.encryptionModeMulticast = (v_U8_t)diagEncTypeFromCSRType(pCommand->u.setKeyCmd.encType);
            setKeyEvent.encryptionModeUnicast = (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
        }
        else
        {
            setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_PTK_REQ;
            setKeyEvent.encryptionModeUnicast = (v_U8_t)diagEncTypeFromCSRType(pCommand->u.setKeyCmd.encType);
            setKeyEvent.encryptionModeMulticast = (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
        }
        palCopyMemory( pMac->hHdd, setKeyEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
        if(CSR_IS_ENC_TYPE_STATIC(edType))
        {
            tANI_U32 defKeyId;

            //It has to be static WEP here
            if(HAL_STATUS_SUCCESS(ccmCfgGetInt(pMac, WNI_CFG_WEP_DEFAULT_KEYID, &defKeyId)))
            {
                setKeyEvent.keyId = (v_U8_t)defKeyId;
            }
        }
        else
        {
            setKeyEvent.keyId = pCommand->u.setKeyCmd.keyId;
        }
        setKeyEvent.authMode = (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
        WLAN_VOS_DIAG_EVENT_REPORT(&setKeyEvent, EVENT_WLAN_SECURITY);
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    status = csrSendMBSetContextReqMsg( pMac, ( tANI_U8 *)&pCommand->u.setKeyCmd.peerMac, 
                                        numKeys, edType, fUnicast, pCommand->u.setKeyCmd.keyDirection, 
                                        pCommand->u.setKeyCmd.keyId, pCommand->u.setKeyCmd.keyLength, 
                                        pCommand->u.setKeyCmd.Key, pCommand->u.setKeyCmd.paeRole );
    if( !HAL_STATUS_SUCCESS(status) )
    {

#ifdef FEATURE_WLAN_DIAG_SUPPORT
        if(eCSR_ENCRYPT_TYPE_NONE != edType)
        {
            if( *(( tANI_U8 *)&pCommand->u.setKeyCmd.peerMac) & 0x01 )
            {
                setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_GTK_RSP;
            }
            else
            {
                setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_PTK_RSP;
            }
            setKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;
            WLAN_VOS_DIAG_EVENT_REPORT(&setKeyEvent, EVENT_WLAN_SECURITY);
        }
#endif //FEATURE_WLAN_DIAG_SUPPORT

        csrRoamCallCallback( pMac, NULL, pCommand->u.setKeyCmd.roamId, eCSR_ROAM_SET_KEY_COMPLETE, eCSR_ROAM_RESULT_FAILURE);
        csrReleaseCommandSetKey( pMac, pCommand );
    }

    return ( status );
}


eHalStatus csrRoamProcessRemoveKeyCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status;
    tpSirSmeRemoveKeyReq pMsg;
    tANI_U16 wMsgLen = sizeof(tSirSmeRemoveKeyReq);
    tANI_U8 *p;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    WLAN_VOS_DIAG_EVENT_DEF(removeKeyEvent, vos_event_wlan_security_payload_type);

    palZeroMemory(pMac->hHdd, &removeKeyEvent, sizeof(vos_event_wlan_security_payload_type));
    removeKeyEvent.eventId = WLAN_SECURITY_EVENT_REMOVE_KEY_REQ;
    removeKeyEvent.encryptionModeMulticast = (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
    removeKeyEvent.encryptionModeUnicast = (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
    palCopyMemory( pMac->hHdd, removeKeyEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
    removeKeyEvent.keyId = pCommand->u.removeKeyCmd.keyId;
    removeKeyEvent.authMode = (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
    WLAN_VOS_DIAG_EVENT_REPORT(&removeKeyEvent, EVENT_WLAN_SECURITY);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    status = palAllocateMemory( pMac->hHdd, (void **)&pMsg, wMsgLen );
    if( HAL_STATUS_SUCCESS( status ) )
    {
        palZeroMemory(pMac->hHdd, pMsg, wMsgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_REMOVEKEY_REQ);
		pMsg->length = pal_cpu_to_be16(wMsgLen);
        p = (tANI_U8 *)pMsg + sizeof(pMsg->messageType) + sizeof(pMsg->length);     
        palCopyMemory( pMac->hHdd, p, pCommand->u.removeKeyCmd.peerMac, sizeof(tSirMacAddr) );
        p += sizeof(tSirMacAddr);
        //edType
        *p = (tANI_U8)csrTranslateEncryptTypeToEdType( pCommand->u.removeKeyCmd.encType );
        p++;
        //weptype
        if( ( eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == pCommand->u.removeKeyCmd.encType ) || 
            ( eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == pCommand->u.removeKeyCmd.encType ) )
        {
            *p = (tANI_U8)eSIR_WEP_STATIC;
        }
        else
        {
            *p = (tANI_U8)eSIR_WEP_DYNAMIC;
        }
        p++;
        //keyid
        *p = pCommand->u.removeKeyCmd.keyId;
        p++;
        *p = (pCommand->u.removeKeyCmd.peerMac[0] == 0xFF ) ? 0 : 1;

        status = palSendMBMessage(pMac->hHdd, pMsg);
    }

    if( !HAL_STATUS_SUCCESS( status ) )
    {

#ifdef FEATURE_WLAN_DIAG_SUPPORT
        removeKeyEvent.eventId = WLAN_SECURITY_EVENT_REMOVE_KEY_RSP;
        removeKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;;
        WLAN_VOS_DIAG_EVENT_REPORT(&removeKeyEvent, EVENT_WLAN_SECURITY);
#endif //FEATURE_WLAN_DIAG_SUPPORT

        csrRoamCallCallback( pMac, NULL, pCommand->u.removeKeyCmd.roamId, eCSR_ROAM_REMOVE_KEY_COMPLETE, eCSR_ROAM_RESULT_FAILURE);
        csrReleaseCommandRemoveKey( pMac, pCommand );
    }

    return ( status );
}



eHalStatus csrRoamSetKey( tpAniSirGlobal pMac, tCsrRoamSetKey *pSetKey, tANI_U32 roamId )
{
    eHalStatus status;

    if( csrIsConnStateDisconnected( pMac ) )
    {
        status = eHAL_STATUS_CSR_WRONG_STATE;
    }
    else
    {
        status = csrRoamIssueSetKeyCommand( pMac, pSetKey, roamId );
    }

    return ( status );
}


/*
   Prepare a filter base on a profile for parsing the scan results.
   Upon successful return, caller MUST call csrFreeScanFilter on 
   pScanFilter when it is done with the filter.
*/
eHalStatus csrRoamPrepareFilterFromProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
                                           tCsrScanResultFilter *pScanFilter)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 size = 0;
    tANI_U8  index = 0;
    
    do
    {
        if(pProfile->BSSIDs.numOfBSSIDs)
        {
            size = sizeof(tCsrBssid) * pProfile->BSSIDs.numOfBSSIDs;
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter->BSSIDs.bssid, size);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pScanFilter->BSSIDs.numOfBSSIDs = pProfile->BSSIDs.numOfBSSIDs;
            palCopyMemory(pMac->hHdd, pScanFilter->BSSIDs.bssid, pProfile->BSSIDs.bssid, size);
        }
        if(pProfile->SSIDs.numOfSSIDs)
        {
            size = sizeof(tCsrSSIDInfo) * pProfile->SSIDs.numOfSSIDs;
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter->SSIDs.SSIDList, size);
            if(!HAL_STATUS_SUCCESS(status))
            {
                break;
            }
            pScanFilter->SSIDs.numOfSSIDs = pProfile->SSIDs.numOfSSIDs;
            palCopyMemory(pMac->hHdd, pScanFilter->SSIDs.SSIDList, pProfile->SSIDs.SSIDList, size);
        }
        if(!pProfile->ChannelInfo.ChannelList || (pProfile->ChannelInfo.ChannelList[0] == 0) )
        {
            pScanFilter->ChannelInfo.numOfChannels = 0;
            pScanFilter->ChannelInfo.ChannelList = NULL;
        }
        else if(pProfile->ChannelInfo.numOfChannels)
        {
           status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter->ChannelInfo.ChannelList, sizeof(*pScanFilter->ChannelInfo.ChannelList) * pProfile->ChannelInfo.numOfChannels);
           pScanFilter->ChannelInfo.numOfChannels = 0;
            if(HAL_STATUS_SUCCESS(status))
            {
              for(index = 0; index < pProfile->ChannelInfo.numOfChannels; index++)
              {
                 if(csrRoamIsChannelValid(pMac, pProfile->ChannelInfo.ChannelList[index]))
                 {
                    pScanFilter->ChannelInfo.ChannelList[pScanFilter->ChannelInfo.numOfChannels] 
                       = pProfile->ChannelInfo.ChannelList[index];
                    pScanFilter->ChannelInfo.numOfChannels++;
                 }
                 else 
                 {
                     smsLog(pMac, LOG1, FL("process a channel (%d) that is invalid\n"), pProfile->ChannelInfo.ChannelList[index]);
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
            smsLog(pMac, LOGW, FL("Channel list empty\n"));
            status = eHAL_STATUS_FAILURE;
            break;
        }
        pScanFilter->uapsd_mask = pProfile->uapsd_mask;
        pScanFilter->authType = pProfile->AuthType;
        pScanFilter->EncryptionType = pProfile->EncryptionType;
        pScanFilter->mcEncryptionType = pProfile->mcEncryptionType;
        pScanFilter->BSSType = pProfile->BSSType;
        pScanFilter->phyMode = pProfile->phyMode;
        /*Save the WPS info*/
        pScanFilter->bWPSAssociation = pProfile->bWPSAssociation;

        if( pProfile->countryCode[0] )
        {
            //This causes the matching function to use countryCode as one of the criteria.
            palCopyMemory( pMac->hHdd, pScanFilter->countryCode, pProfile->countryCode, 
                        WNI_CFG_COUNTRY_CODE_LEN );
        }
    
    }while(0);
    
    if(!HAL_STATUS_SUCCESS(status))
    {
        csrFreeScanFilter(pMac, pScanFilter);
    }
    
    return(status);
}


tANI_BOOLEAN csrRoamIssueWmStatusChange( tpAniSirGlobal pMac, eCsrRoamWmStatusChangeTypes Type, tSirSmeRsp *pSmeRsp )
{
    tANI_BOOLEAN fCommandQueued = eANI_BOOLEAN_FALSE;
    tSmeCmd *pCommand;

    do
    {
        // Validate the type is ok...
        if ( ( eCsrDisassociated != Type ) && ( eCsrDeauthenticated != Type ) ) break;
        pCommand = csrGetCommandBuffer( pMac );
        if ( !pCommand ) break;
        //Change the substate in case it is waiting for key
        if( CSR_IS_WAIT_FOR_KEY( pMac ) )
        {
            csrRoamStopWaitForKeyTimer( pMac );
            csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
        }
        pCommand->command = eSmeCommandWmStatusChange;
        pCommand->u.wmStatusChangeCmd.Type = Type;
        if ( eCsrDisassociated ==  Type )
        {
            palCopyMemory( pMac->hHdd, &pCommand->u.wmStatusChangeCmd.u.DisassocIndMsg, pSmeRsp, 
                                sizeof( pCommand->u.wmStatusChangeCmd.u.DisassocIndMsg ) );
        }
        else
        {
            palCopyMemory( pMac->hHdd, &pCommand->u.wmStatusChangeCmd.u.DeauthIndMsg, pSmeRsp, 
                            sizeof( pCommand->u.wmStatusChangeCmd.u.DeauthIndMsg ) );
        }
        if( HAL_STATUS_SUCCESS( csrQueueSmeCommand(pMac, pCommand, eANI_BOOLEAN_TRUE) ) )
        {
            fCommandQueued = eANI_BOOLEAN_TRUE;
        }
        else
        {
            smsLog( pMac, LOGE, FL(" fail to send message \n") );
            csrReleaseCommandWmStatusChange( pMac, pCommand );
        }

    } while( 0 );

    return( fCommandQueued );
}


void csrRoamCheckForLinkStatusChange( tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg )
{
    tSirSmeDisassocInd *pDisassocInd;
    tSirSmeWmStatusChangeNtf *pStatusChangeMsg;
    tSirSmeNewBssInfo *pNewBss;
    tSmeIbssPeerInd *pIbssPeerInd;
    tSirMacAddr Broadcastaddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    tSirSmeApNewCaps *pApNewCaps;
    eCsrRoamResult result;
    eRoamCmdStatus roamStatus;
    tCsrRoamInfo *pRoamInfo = NULL;
    tCsrRoamInfo roamInfo;
    eHalStatus status;

#if defined ANI_PRODUCT_TYPE_AP
    pSirMsg->messageType = pal_be16_to_cpu(pSirMsg->messageType);
    pSirMsg->length = pal_be16_to_cpu(pSirMsg->length);
    pSirMsg->statusCode = pal_be32_to_cpu(pSirMsg->statusCode);
#else
    pSirMsg->messageType = (pSirMsg->messageType);
    pSirMsg->length = (pSirMsg->length);
    pSirMsg->statusCode = (pSirMsg->statusCode);
#endif
    palZeroMemory(pMac->hHdd, &roamInfo, sizeof(roamInfo));

    switch( pSirMsg->messageType ) 
    {
        case eWNI_SME_DISASSOC_IND:
            smsLog( pMac, LOG1, FL("DISASSOCIATION Indication from MAC\n"));

            // Check if AP dis-associated us because of MIC failure. If so,
            // then we need to take action immediately and not wait till the
            // the WmStatusChange requests is pushed and processed
            pDisassocInd = (tSirSmeDisassocInd *)pSirMsg;
            sme_QosCsrEventInd(pMac, SME_QOS_CSR_DISCONNECT_IND, NULL);
            pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
            csrRoamLinkDown(pMac);
            csrRoamIssueWmStatusChange( pMac, eCsrDisassociated, pSirMsg );
            break;

        case eWNI_SME_DEAUTH_IND:
            smsLog( pMac, LOG1, FL("DEAUTHENTICATION Indication from MAC\n"));
            sme_QosCsrEventInd(pMac, SME_QOS_CSR_DISCONNECT_IND, NULL);
            pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED;
            csrRoamLinkDown(pMac);
            csrRoamIssueWmStatusChange( pMac, eCsrDeauthenticated, pSirMsg );
            break;                   
        
        case eWNI_SME_SWITCH_CHL_REQ:        // in case of STA, the SWITCH_CHANNEL originates from its AP
            smsLog( pMac, LOGW, FL("eWNI_SME_SWITCH_CHL_REQ from SME\n"));
            //Update with the new channel id.
            //The channel id is hidden in the statusCode.
            pMac->roam.connectedProfile.operationChannel = (tANI_U8)pSirMsg->statusCode;
            if(pMac->roam.pConnectBssDesc)
            {
                pMac->roam.pConnectBssDesc->channelId = (tANI_U8)pSirMsg->statusCode;
            }
            break;
                
        case eWNI_SME_DISASSOC_RSP:       
            smsLog( pMac, LOG1, FL("eWNI_SME_DISASSOC_RSP from SME\n"));
            break;
     
        case eWNI_SME_MIC_FAILURE_IND:
            {
                tpSirSmeMicFailureInd pMicInd = (tpSirSmeMicFailureInd)pSirMsg;
                tCsrRoamInfo roamInfo, *pRoamInfo = NULL;
                eCsrRoamResult result = eCSR_ROAM_RESULT_MIC_ERROR_UNICAST;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
                {
                    WLAN_VOS_DIAG_EVENT_DEF(secEvent, vos_event_wlan_security_payload_type);
                    palZeroMemory(pMac->hHdd, &secEvent, sizeof(vos_event_wlan_security_payload_type));
                    secEvent.eventId = WLAN_SECURITY_EVENT_MIC_ERROR;
                    secEvent.encryptionModeMulticast = 
                        (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
                    secEvent.encryptionModeUnicast = 
                        (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
                    secEvent.authMode = 
                        (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
                    if(pMicInd)
                    {
                        palCopyMemory( pMac->hHdd, secEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
                    }
                    WLAN_VOS_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
                }
#endif//FEATURE_WLAN_DIAG_SUPPORT

                if(pMicInd)
                {
                    palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                    roamInfo.u.pMICFailureInfo = &pMicInd->info;
                    pRoamInfo = &roamInfo;
                    if(pMicInd->info.multicast)
                    {
                        result = eCSR_ROAM_RESULT_MIC_ERROR_GROUP;
                    }
                    else
                    {
                        result = eCSR_ROAM_RESULT_MIC_ERROR_UNICAST;
                    }
                }
                csrRoamCallCallback(pMac, pRoamInfo, 0, eCSR_ROAM_MIC_ERROR_IND, result);
            }
            break;

        case eWNI_SME_WM_STATUS_CHANGE_NTF:
            pStatusChangeMsg = (tSirSmeWmStatusChangeNtf *)pSirMsg;
            switch( pStatusChangeMsg->statusChangeCode ) 
            {
                case eSIR_SME_IBSS_ACTIVE:
                    pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_IBSS_CONNECTED;
                    if(pMac->roam.pConnectBssDesc)
                    {
                        palCopyMemory(pMac->hHdd, &roamInfo.bssid, pMac->roam.pConnectBssDesc->bssId, sizeof(tCsrBssid));
                        roamInfo.u.pConnectedProfile = &pMac->roam.connectedProfile;
                        pRoamInfo = &roamInfo;
                    }
                    else
                    {
                        smsLog(pMac, LOGE, "  CSR eSIR_SME_IBSS_NEW_PEER connected BSS is empty\n");
                    }
                    result = eCSR_ROAM_RESULT_IBSS_CONNECT;
                    roamStatus = eCSR_ROAM_CONNECT_STATUS_UPDATE;
                    break;

                case eSIR_SME_IBSS_INACTIVE:
                    pMac->roam.connectState = eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED;
                    result = eCSR_ROAM_RESULT_IBSS_INACTIVE;
                    roamStatus = eCSR_ROAM_CONNECT_STATUS_UPDATE;
                    break;

                case eSIR_SME_JOINED_NEW_BSS:    // IBSS coalescing.
                    pNewBss = &pStatusChangeMsg->statusChangeInfo.newBssInfo;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
                    {
                        vos_log_ibss_pkt_type *pIbssLog;
                        tANI_U32 bi;

                        WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
                        if(pIbssLog)
                        {
                            pIbssLog->eventId = WLAN_IBSS_EVENT_COALESCING;
                            if(pNewBss)
                            {
                                palCopyMemory(pMac->hHdd, pIbssLog->bssid, pNewBss->bssId, 6);
                                if(pNewBss->ssId.length)
                                {
                                    palCopyMemory(pMac->hHdd, pIbssLog->ssid, pNewBss->ssId.ssId, pNewBss->ssId.length);
                                }
                                pIbssLog->operatingChannel = pNewBss->channelNumber;
                            }
                            if(HAL_STATUS_SUCCESS(ccmCfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &bi)))
                            {
                                //***U8 is not enough for beacon interval
                                pIbssLog->beaconInterval = (v_U8_t)bi;
                            }
                            WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
                        }
                    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

                    // update the connection state information
                    csrRoamUpdateConnectedProfileFromNewBss( pMac, pNewBss );
                    csrRoamIssueSetContextReq( pMac, pMac->roam.connectedProfile.EncryptionType, 
                                                pMac->roam.pConnectBssDesc,
                                                &Broadcastaddr,
                                                FALSE, FALSE, eSIR_TX_RX, 0, 0, NULL, 0 );
                    result = eCSR_ROAM_RESULT_IBSS_COALESCED;
                    roamStatus = eCSR_ROAM_IBSS_IND;
                    palCopyMemory(pMac->hHdd, &roamInfo.bssid, &pNewBss->bssId, sizeof(tCsrBssid));
                    pRoamInfo = &roamInfo;
                    //This BSSID is th ereal BSSID, let's save it
                    if(pMac->roam.pConnectBssDesc)
                    {
                        palCopyMemory(pMac->hHdd, pMac->roam.pConnectBssDesc->bssId, &pNewBss->bssId, sizeof(tCsrBssid));
                    }
                    // Stop the join IBSS timer in case of join, for 
                    // genuine merge do nothing
                    if(pMac->roam.ibss_join_pending)
                    {
                       pMac->roam.ibss_join_pending = FALSE;
                       csrRoamStopIbssJoinTimer(pMac);
                       result = eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS;
                    }
                    smsLog(pMac, LOGW, "CSR:  eSIR_SME_JOINED_NEW_BSS received from PE\n");
                    break;

                // detection by LIM that the capabilities of the associated AP have changed.
                case eSIR_SME_AP_CAPS_CHANGED:
                    pApNewCaps = &pStatusChangeMsg->statusChangeInfo.apNewCaps;
                    smsLog(pMac, LOGW, "  CSR is handling CAP change\n");
                    csrScanForCapabilityChange( pMac, pApNewCaps );
                    result = eCSR_ROAM_RESULT_CAP_CHANGED;
                    roamStatus = eCSR_ROAM_GEN_INFO;
                    break;
            
                default:
                    roamStatus = eCSR_ROAM_FAILED;
                    result = eCSR_ROAM_RESULT_NONE;
                    break;

            }  // end switch on statusChangeCode
            if(eCSR_ROAM_RESULT_NONE != result)
            {
                csrRoamCallCallback(pMac, pRoamInfo, 0, roamStatus, result);
            }
            break;

        case eWNI_SME_IBSS_NEW_PEER_IND:
            pIbssPeerInd = (tSmeIbssPeerInd *)pSirMsg;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
            {
                vos_log_ibss_pkt_type *pIbssLog;

                WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
                if(pIbssLog)
                {
                    pIbssLog->eventId = WLAN_IBSS_EVENT_PEER_JOIN;
                    if(pIbssPeerInd)
                    {
                        palCopyMemory(pMac->hHdd, pIbssLog->peerMacAddr, &pIbssPeerInd->peerAddr, 6);
                    }
                    WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
                }
            }
#endif //FEATURE_WLAN_DIAG_SUPPORT

            // Issue the set Context request to LIM to establish the Unicast STA context for the new peer...
            if(pMac->roam.pConnectBssDesc)
            {
                palCopyMemory(pMac->hHdd, &roamInfo.peerMac, pIbssPeerInd->peerAddr, sizeof(tCsrBssid));
                palCopyMemory(pMac->hHdd, &roamInfo.bssid, pMac->roam.pConnectBssDesc->bssId, sizeof(tCsrBssid));
                if(pIbssPeerInd->mesgLen > sizeof(tSmeIbssPeerInd))
                {
                    status = palAllocateMemory(pMac->hHdd, (void **)&roamInfo.pbFrames, 
                                                (pIbssPeerInd->mesgLen - sizeof(tSmeIbssPeerInd)));
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        roamInfo.nBeaconLength = (pIbssPeerInd->mesgLen - sizeof(tSmeIbssPeerInd));
                        palCopyMemory(pMac->hHdd, roamInfo.pbFrames, ((tANI_U8 *)pIbssPeerInd) + sizeof(tSmeIbssPeerInd),
                                            roamInfo.nBeaconLength);
                    }
					roamInfo.staId = (tANI_U8)pIbssPeerInd->staId;
                    roamInfo.ucastSig = (tANI_U8)pIbssPeerInd->ucastSig;
                    roamInfo.bcastSig = (tANI_U8)pIbssPeerInd->bcastSig;
                    status = palAllocateMemory(pMac->hHdd, (void **)&roamInfo.pBssDesc, 
                                            pMac->roam.pConnectBssDesc->length);
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        palCopyMemory(pMac->hHdd, roamInfo.pBssDesc, pMac->roam.pConnectBssDesc, 
                                            pMac->roam.pConnectBssDesc->length);
                    }
                    if(HAL_STATUS_SUCCESS(status))
                    {
                        pRoamInfo = (void *)&roamInfo;
                    }
                    else
                    {
                        if(roamInfo.pbFrames)
                        {
                            palFreeMemory(pMac->hHdd, roamInfo.pbFrames);
                        }
                        if(roamInfo.pBssDesc)
                        {
                            palFreeMemory(pMac->hHdd, roamInfo.pBssDesc);
                        }
                    }
                }
                else
                {
                    pRoamInfo = (void *)&roamInfo;
                }
                csrRoamIssueSetContextReq( pMac, pMac->roam.connectedProfile.EncryptionType, 
                                        pMac->roam.pConnectBssDesc,
                                        &(pIbssPeerInd->peerAddr),
                                        FALSE, TRUE, eSIR_TX_RX, 0, 0, NULL, 0 ); // NO keys... these key parameters don't matter.
            }
            else
            {
                smsLog(pMac, LOGW, "  CSR eSIR_SME_IBSS_NEW_PEER connected BSS is empty\n");
            }
            //send up the sec type for the new peer
            pRoamInfo->u.pConnectedProfile = &pMac->roam.connectedProfile;
            csrRoamCallCallback(pMac, pRoamInfo, 0, eCSR_ROAM_CONNECT_STATUS_UPDATE, eCSR_ROAM_RESULT_IBSS_NEW_PEER);
            if(pRoamInfo)
            {
                if(roamInfo.pbFrames)
                {
                    palFreeMemory(pMac->hHdd, roamInfo.pbFrames);
                }
                if(roamInfo.pBssDesc)
                {
                    palFreeMemory(pMac->hHdd, roamInfo.pBssDesc);
                }
            }
            break;

        case eWNI_SME_IBSS_PEER_DEPARTED_IND:
            pIbssPeerInd = (tSmeIbssPeerInd*)pSirMsg;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
            {
                vos_log_ibss_pkt_type *pIbssLog;

                WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
                if(pIbssLog)
                {
                    pIbssLog->eventId = WLAN_IBSS_EVENT_PEER_LEAVE;
                    if(pIbssPeerInd)
                    {
                        palCopyMemory(pMac->hHdd, pIbssLog->peerMacAddr, &pIbssPeerInd->peerAddr, 6);
                    }
                    WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
                }
            }
#endif //FEATURE_WLAN_DIAG_SUPPORT

            smsLog(pMac, LOGW, "CSR: Peer departed notification from LIM\n");
			roamInfo.staId = (tANI_U8)pIbssPeerInd->staId;
            roamInfo.ucastSig = (tANI_U8)pIbssPeerInd->ucastSig;
            roamInfo.bcastSig = (tANI_U8)pIbssPeerInd->bcastSig;
			palCopyMemory(pMac->hHdd, &roamInfo.peerMac, pIbssPeerInd->peerAddr, sizeof(tCsrBssid));
            csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_CONNECT_STATUS_UPDATE, eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED);
            break;

        case eWNI_SME_SETCONTEXT_RSP:
            {
                tSirSmeSetContextRsp *pRsp = (tSirSmeSetContextRsp *)pSirMsg;
                tListElem *pEntry;
                tSmeCmd *pCommand;
                
                pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
                if ( pEntry )
                {
                    pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
                    if ( eSmeCommandSetKey == pCommand->command )
                    {                

#ifdef FEATURE_WLAN_DIAG_SUPPORT
                        if(eCSR_ENCRYPT_TYPE_NONE != pMac->roam.connectedProfile.EncryptionType)
                        {
                            WLAN_VOS_DIAG_EVENT_DEF(setKeyEvent, vos_event_wlan_security_payload_type);
                            palZeroMemory(pMac->hHdd, &setKeyEvent, sizeof(vos_event_wlan_security_payload_type));
                            if( pRsp->peerMacAddr[0] & 0x01 )
                            {
                                setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_GTK_RSP;
                            }
                            else
                            {
                                setKeyEvent.eventId = WLAN_SECURITY_EVENT_SET_PTK_RSP;
                            }
                            setKeyEvent.encryptionModeMulticast = 
                                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
                            setKeyEvent.encryptionModeUnicast = 
                                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
                            palCopyMemory( pMac->hHdd, setKeyEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
                            setKeyEvent.authMode = 
                                (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
                            if( eSIR_SUCCESS != pRsp->statusCode )
                            {
                                setKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;
                            }
                            WLAN_VOS_DIAG_EVENT_REPORT(&setKeyEvent, EVENT_WLAN_SECURITY);
                        }
#endif //FEATURE_WLAN_DIAG_SUPPORT

                        if( CSR_IS_WAIT_FOR_KEY( pMac ) )
                        {
                            //We are done with authentication, whethere succeed or not
                            csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
                            csrRoamStopWaitForKeyTimer( pMac );
                            //We do it here because this linkup function is not called after association 
                            //when a key needs to be set. 
                            if( csrIsConnStateConnectedInfra(pMac) ) 
                            {
                                csrRoamLinkUp(pMac, pMac->roam.connectedProfile.bssid);
                            }
                        }
                        if( eSIR_SUCCESS == pRsp->statusCode )
                        {
                            palCopyMemory( pMac, &roamInfo.peerMac, &pRsp->peerMacAddr, sizeof(tCsrBssid) );
                            //Make sure we install the GTK before indicating to HDD as authenticated
                            //This is to prevent broadcast packets go out after PTK and before GTK.
                            if( palEqualMemory( pMac->hHdd, &Broadcastaddr, pRsp->peerMacAddr, 
                                        sizeof(tSirMacAddr) ) )
                            {
                                result = eCSR_ROAM_RESULT_AUTHENTICATED;
                            }
                            else
                            {
                                result = eCSR_ROAM_RESULT_NONE;
                            }
                            pRoamInfo = &roamInfo;
                        }
                        else
                        {
                            result = eCSR_ROAM_RESULT_FAILURE;
                            smsLog( pMac, LOGE, "CSR: Roam Completion setkey command failed(%d) PeerMac %02X-%02X-%02X-%02X-%02X-%02X...\n", 
                                pRsp->statusCode, pRsp->peerMacAddr[0], pRsp->peerMacAddr[1], pRsp->peerMacAddr[2],
                                pRsp->peerMacAddr[3], pRsp->peerMacAddr[4], pRsp->peerMacAddr[5] );
                        }
                        csrRoamCallCallback(pMac, &roamInfo, pCommand->u.setKeyCmd.roamId, 
                                            eCSR_ROAM_SET_KEY_COMPLETE, result);
                        if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
                        {
                            csrReleaseCommandSetKey( pMac, pCommand );
                        }
                    }
                    else
                    {
                        smsLog( pMac, LOGE, "CSR: Roam Completion called but setkey command is not ACTIVE ...\n" );
                    }
                }
                else
                {
                    smsLog( pMac, LOGE, "CSR: SetKey Completion called but NO commands are ACTIVE ...\n" );
                }

                smeProcessPendingQueue( pMac );
            }
            break;

        case eWNI_SME_REMOVEKEY_RSP:
            {
                tSirSmeRemoveKeyRsp *pRsp = (tSirSmeRemoveKeyRsp *)pSirMsg;
                tListElem *pEntry;
                tSmeCmd *pCommand;
                
                pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
                if ( pEntry )
                {
                    pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
                    if ( eSmeCommandSetKey == pCommand->command )
                    {                

#ifdef FEATURE_WLAN_DIAG_SUPPORT
                        {
                            WLAN_VOS_DIAG_EVENT_DEF(removeKeyEvent, vos_event_wlan_security_payload_type);
                            palZeroMemory(pMac->hHdd, &removeKeyEvent, sizeof(vos_event_wlan_security_payload_type));
                            removeKeyEvent.eventId = WLAN_SECURITY_EVENT_REMOVE_KEY_RSP;
                            removeKeyEvent.encryptionModeMulticast = 
                                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
                            removeKeyEvent.encryptionModeUnicast = 
                                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
                            palCopyMemory( pMac->hHdd, removeKeyEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
                            removeKeyEvent.authMode = 
                                (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
                            if( eSIR_SUCCESS != pRsp->statusCode )
                            {
                                removeKeyEvent.status = WLAN_SECURITY_STATUS_FAILURE;
                            }
                            WLAN_VOS_DIAG_EVENT_REPORT(&removeKeyEvent, EVENT_WLAN_SECURITY);
                        }
#endif //FEATURE_WLAN_DIAG_SUPPORT

                        if( eSIR_SUCCESS == pRsp->statusCode )
                        {
                            palCopyMemory( pMac, &roamInfo.peerMac, &pRsp->peerMacAddr, sizeof(tCsrBssid) );
                            result = eCSR_ROAM_RESULT_NONE;
                            pRoamInfo = &roamInfo;
                        }
                        else
                        {
                            result = eCSR_ROAM_RESULT_FAILURE;
                        }
                        csrRoamCallCallback(pMac, &roamInfo, pCommand->u.setKeyCmd.roamId, 
                                            eCSR_ROAM_REMOVE_KEY_COMPLETE, result);
                        if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
                        {
                            csrReleaseCommandRemoveKey( pMac, pCommand );
                        }
                    }
                    else
                    {
                        smsLog( pMac, LOGW, "CSR: Roam Completion called but setkey command is not ACTIVE ...\n" );
                    }
                }
                else
                {
                    smsLog( pMac, LOGW, "CSR: SetKey Completion called but NO commands are ACTIVE ...\n" );
                }

                smeProcessPendingQueue( pMac );
            }
            break;

        case eWNI_SME_GET_STATISTICS_RSP:
            smsLog( pMac, LOGW, FL("Stats rsp from PE\n"));
            csrRoamStatsRspProcessor( pMac, pSirMsg );
            break;

        default:
            break;

    }  // end switch on message type

}


eHalStatus csrRoamStartRoaming(tpAniSirGlobal pMac, eCsrRoamingReason roamingReason)
{
    eHalStatus status = eHAL_STATUS_FAILURE;

    if(eCsrLostlinkRoaming == roamingReason && (eANI_BOOLEAN_FALSE == pMac->roam.fCancelRoaming))
    {
        status = csrScanRequestLostLink1( pMac );
    }

    return(status);
}


//return a boolean to indicate whether roaming completed or continue.
tANI_BOOLEAN csrRoamCompleteRoaming(tpAniSirGlobal pMac, tANI_BOOLEAN fForce, eCsrRoamResult roamResult)
{
    tANI_BOOLEAN fCompleted = eANI_BOOLEAN_TRUE;
    tANI_TIMESTAMP roamTime = (tANI_TIMESTAMP)(pMac->roam.configParam.nRoamingTime * PAL_TICKS_PER_SECOND);
    tANI_TIMESTAMP curTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);

    //Check whether time is up
    if(pMac->roam.fCancelRoaming || fForce || 
       ((curTime - pMac->roam.roamingStartTime) > roamTime) ||
       eCsrReassocRoaming == pMac->roam.roamingReason ||
       eCsrDynamicRoaming == pMac->roam.roamingReason)
    {
        smsLog(pMac, LOGW, FL("  indicates roaming completion\n"));
        if(pMac->roam.fCancelRoaming && (eCsrLostlinkRoaming == pMac->roam.roamingReason))
        {
            //roaming is cancelled, tell HDD to indicate disconnect
            roamResult = eCSR_ROAM_RESULT_FAILURE;
        }
        csrRoamCallCallback(pMac, NULL, 0, eCSR_ROAM_ROAMING_COMPLETION, roamResult);
        pMac->roam.roamingReason = eCsrNotRoaming;
    }
    else
    {
        pMac->roam.roamResult = roamResult;
        if(!HAL_STATUS_SUCCESS(csrRoamStartRoamingTimer(pMac, PAL_TIMER_TO_SEC_UNIT)))
        {
            csrRoamCallCallback(pMac, NULL, 0, eCSR_ROAM_ROAMING_COMPLETION, roamResult);
            pMac->roam.roamingReason = eCsrNotRoaming;
        }
        else
        {
            fCompleted = eANI_BOOLEAN_FALSE;
        }
    }

    return(fCompleted);
}


void csrRoamCancelRoaming(tpAniSirGlobal pMac)
{
    if(CSR_IS_ROAMING(pMac))
    {
        smsLog(pMac, LOGW, "   Cancelling roaming\n");
        pMac->roam.fCancelRoaming = eANI_BOOLEAN_TRUE;
        if(CSR_IS_ROAM_JOINING(pMac) && CSR_IS_ROAM_SUBSTATE_CONFIG(pMac))
        {
            //No need to do anything in here because the handler takes care of it
        }
        else
        {
            eCsrRoamResult roamResult = (eCsrLostlinkRoaming == (pMac)->roam.roamingReason) ? 
                                                    eCSR_ROAM_RESULT_FAILURE : eCSR_ROAM_RESULT_NONE;
            //Roaming is stopped after here 
            csrRoamCompleteRoaming(pMac, eANI_BOOLEAN_TRUE, roamResult);
            //Since CSR may be in lostlink roaming situation, abort all roaming related activities
            csrScanAbortMacScan(pMac);
            csrRoamStopRoamingTimer(pMac);
        }
    }
}


void csrRoamRoamingTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    
    if(eANI_BOOLEAN_FALSE == pMac->roam.fCancelRoaming) 
    {
        if(!HAL_STATUS_SUCCESS(csrRoamStartRoaming(pMac, eCsrLostlinkRoaming)))
        {
            csrRoamCallCallback(pMac, NULL, 0, eCSR_ROAM_ROAMING_COMPLETION, pMac->roam.roamResult);
            pMac->roam.roamingReason = eCsrNotRoaming;
        }
    }
}


eHalStatus csrRoamStartRoamingTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOG1, " csrScanStartRoamingTimer \n ");
    status = palTimerStart(pMac->hHdd, pMac->roam.hTimerRoaming, interval, eANI_BOOLEAN_FALSE);
    
    return (status);
}


eHalStatus csrRoamStopRoamingTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->roam.hTimerRoaming));
}


void csrRoamWaitForKeyTimeOutHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );

    if( CSR_IS_WAIT_FOR_KEY( pMac ) )
    {
        smsLog(pMac, LOGW, " SME pre-auth state timeout. \n ");
        //Change the substate so command queue is unblocked.
        csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_NONE );
    }
    
}


eHalStatus csrRoamStartWaitForKeyTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOG1, " csrScanStartWaitForKeyTimer \n ");
    status = palTimerStart(pMac->hHdd, pMac->roam.hTimerWaitForKey, interval, eANI_BOOLEAN_FALSE);
    
    return (status);
}


eHalStatus csrRoamStopWaitForKeyTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->roam.hTimerWaitForKey));
}


void csrRoamIbssJoinTimerHandler(void *pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( pv );
    eCsrRoamDisconnectReason reason = eCSR_DISCONNECT_REASON_UNSPECIFIED;
    
    // Notify HDD that IBSS join failed
    csrRoamCallCallback(pMac, NULL, 0, eCSR_ROAM_IBSS_IND, eCSR_ROAM_RESULT_IBSS_JOIN_FAILED);
    // Send an IBSS stop request to PE
    csrRoamDisconnectInternal(pMac, reason);

}

eHalStatus csrRoamStartIbssJoinTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
    eHalStatus status;
    
    smsLog(pMac, LOG1, " csrRoamStartIbssJoinTimer \n ");
    status = palTimerStart(pMac->hHdd, pMac->roam.hTimerIbssJoining, interval, eANI_BOOLEAN_FALSE);
    
    return (status);
}

eHalStatus csrRoamStopIbssJoinTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->roam.hTimerIbssJoining));
}

void csrRoamCompletion(tpAniSirGlobal pMac, tCsrRoamInfo *pRoamInfo, tSmeCmd *pCommand, 
                        eCsrRoamResult roamResult, tANI_BOOLEAN fSuccess)
{
    eRoamCmdStatus roamStatus = csrGetRoamCompleteStatus(pMac);
    tANI_U32 roamId = 0;
    
    if(pCommand)
    {
        roamId = pCommand->u.roamCmd.roamId;
    }

    if(eCSR_ROAM_ROAMING_COMPLETION == roamStatus)
    {
        //if success, force roaming completion
        csrRoamCompleteRoaming(pMac, fSuccess, roamResult);
    }
    else
    {
        smsLog(pMac, LOGW, FL("  indicates association completion. roamResult = %d\n"), roamResult);
        csrRoamCallCallback(pMac, pRoamInfo, roamId, roamStatus, roamResult);
    }
}


eHalStatus csrRoamLostLink( tpAniSirGlobal pMac, tANI_U32 type, tSirSmeRsp *pSirMsg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeDeauthInd *pDeauthIndMsg;
    tSirSmeDisassocInd *pDisassocIndMsg;
    eCsrRoamResult result = eCSR_ROAM_RESULT_LOSTLINK;
    tCsrRoamInfo *pRoamInfo = NULL;
    tCsrRoamInfo roamInfo;

    pMac->roam.fCancelRoaming = eANI_BOOLEAN_FALSE;
    if ( eWNI_SME_DISASSOC_IND == type )
    {
        result = eCSR_ROAM_RESULT_DISASSOC_IND;
    }
    else if ( eWNI_SME_DEAUTH_IND == type )
    {
        result = eCSR_ROAM_RESULT_DEAUTH_IND;
    }
    else
    {
        smsLog(pMac, LOGW, FL("gets an unknown type (%d)\n"), type);
        result = eCSR_ROAM_RESULT_NONE;
    }
    
    // call profile lost link routine here
    csrRoamCallCallback(pMac, NULL, 0, eCSR_ROAM_LOSTLINK_DETECTED, result);
    if ( eWNI_SME_DISASSOC_IND == type )
    {
        pDisassocIndMsg = (tSirSmeDisassocInd *)pSirMsg;
        pMac->roam.roamingStatusCode = pDisassocIndMsg->statusCode;
        status = csrSendMBDisassocCnfMsg(pMac, pDisassocIndMsg);
    }
    else if ( eWNI_SME_DEAUTH_IND == type )
    {
        pDeauthIndMsg = (tSirSmeDeauthInd *)pSirMsg;
        pMac->roam.roamingStatusCode = pDeauthIndMsg->statusCode;
        status = csrSendMBDeauthCnfMsg(pMac, pDeauthIndMsg);
    }

    if(HAL_STATUS_SUCCESS(status))
    {
        //Only remove the connected BSS in infrastructure mode
        csrRoamRemoveConnectedBssFromScanCache(pMac);
        if(pMac->roam.configParam.nRoamingTime)
        {
            if(HAL_STATUS_SUCCESS(status = csrRoamStartRoaming(pMac, eCsrLostlinkRoaming)))
            {
                palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
                //For IBSS, we need to give some more info to HDD
                if(csrIsBssTypeIBSS(pMac->roam.connectedProfile.BSSType))
                {
                    roamInfo.u.pConnectedProfile = &pMac->roam.connectedProfile;
                    roamInfo.statusCode = pMac->roam.roamingStatusCode;
                    roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
                }
                else
                {
                   roamInfo.reasonCode = eCsrRoamReasonSmeIssuedForLostLink;
                }
                pRoamInfo = &roamInfo;
                pMac->roam.roamingReason = eCsrLostlinkRoaming;
                pMac->roam.roamingStartTime = (tANI_TIMESTAMP)palGetTickCount(pMac->hHdd);
                csrRoamCallCallback(pMac, pRoamInfo, 0, eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_LOSTLINK);
            }
        }
        else
        {
            //We are told not to roam, indicate lostlink
            status = eHAL_STATUS_FAILURE;
        }
    }
    else
    {
        smsLog(pMac, LOGW, "  Fail to restart roaming, status = %s", status);
    }
    if(!HAL_STATUS_SUCCESS(status))
    {
        //We cannot roam, just tell HDD to disconnect
        palZeroMemory(pMac->hHdd, &roamInfo, sizeof(tCsrRoamInfo));
        roamInfo.statusCode = pMac->roam.roamingStatusCode;
        roamInfo.reasonCode = pMac->roam.joinFailStatusCode.reasonCode;
        csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_LOSTLINK, result);
        csrScanStartIdleScan(pMac);
    }
    
    return (status);
}

eHalStatus csrRoamLostLinkAfterhandoffFailure( tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tListElem *pEntry = NULL;
    tSmeCmd *pCommand = NULL;
    pMac->roam.fCancelRoaming = eANI_BOOLEAN_FALSE;

    //Only remove the connected BSS in infrastructure mode
    csrRoamRemoveConnectedBssFromScanCache(pMac);
    if(pMac->roam.configParam.nRoamingTime)
    {
       if(HAL_STATUS_SUCCESS(status = csrRoamStartRoaming(pMac, eCsrLostlinkRoaming)))
       {
          //before starting the lost link logic release the roam command for handoff
          pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
          if(pEntry)
          {
              pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
          }
          if(pCommand)
          {
             if (( eSmeCommandRoam == pCommand->command ) &&
                 ( eCsrSmeIssuedAssocToSimilarAP == pCommand->u.roamCmd.roamReason))
             {
                 if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
                 {
                    csrReleaseCommandRoam( pMac, pCommand );
                 }
             }
          }

          smsLog( pMac, LOGW, "Lost link roaming started ...\n");
       }
    }
    else
    {
       //We are told not to roam, indicate lostlink
       status = eHAL_STATUS_FAILURE;
    }
    
    return (status);
}

void csrRoamWmStatusChangeComplete( tpAniSirGlobal pMac )
{
    tListElem *pEntry;
    tSmeCmd *pCommand;

    pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
    if ( pEntry )
    {
        pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
        if ( eSmeCommandWmStatusChange == pCommand->command )
        {
            // Nothing to process in a Lost Link completion....  It just kicks off a
            // roaming sequence.
            if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
            {
                csrReleaseCommandWmStatusChange( pMac, pCommand );            
            }
            else
            {
                smsLog( pMac, LOGE, " ******csrRoamWmStatusChangeComplete fail to release command\n");
            }
            
        }
        else
        {
            smsLog( pMac, LOGW, "CSR: WmStatusChange Completion called but LOST LINK command is not ACTIVE ...\n" );
        }
    }
    else
    {
        smsLog( pMac, LOGW, "CSR: WmStatusChange Completion called but NO commands are ACTIVE ...\n" );
    }

    smeProcessPendingQueue( pMac );
}


void csrRoamProcessWmStatusChangeCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tSirSmeRsp *pSirSmeMsg;

    switch ( pCommand->u.wmStatusChangeCmd.Type )
    {
        case eCsrDisassociated:
            pSirSmeMsg = (tSirSmeRsp *)&pCommand->u.wmStatusChangeCmd.u.DisassocIndMsg;
            status = csrRoamLostLink(pMac, eWNI_SME_DISASSOC_IND, pSirSmeMsg);
            break;

        case eCsrDeauthenticated:
            pSirSmeMsg = (tSirSmeRsp *)&pCommand->u.wmStatusChangeCmd.u.DeauthIndMsg;
            status = csrRoamLostLink(pMac, eWNI_SME_DEAUTH_IND, pSirSmeMsg);
            break;

        default:
            smsLog(pMac, LOGW, FL("gets an unknown command %d\n"), pCommand->u.wmStatusChangeCmd.Type);
            break;
    }

    // Lost Link just triggers a roaming sequence.  We can complte the Lost Link
    // command here since there is nothing else to do.
    csrRoamWmStatusChangeComplete( pMac );
}


//This function returns band and mode information.
//The only tricky part is that if phyMode is set to 11abg, this function may return eCSR_CFG_DOT11_MODE_11B
//instead of eCSR_CFG_DOT11_MODE_11G if everything is set to auto-pick.
eCsrCfgDot11Mode csrRoamGetPhyModeBandForIBSS( tpAniSirGlobal pMac, eCsrPhyMode phyModeIn, tANI_U8 operationChn, 
                               eCsrBand *pBand )
{
    eCsrCfgDot11Mode cfgDot11Mode = csrGetCfgDot11ModeFromCsrPhyMode(phyModeIn, 
                                            pMac->roam.configParam.ProprietaryRatesEnabled);
    eCsrBand eBand;
    
    //If the global setting for dot11Mode is set to auto/abg, we overwrite the setting in the profile.
    if( (eCSR_CFG_DOT11_MODE_AUTO == pMac->roam.configParam.uCfgDot11Mode) ||
        (eCSR_CFG_DOT11_MODE_ABG == pMac->roam.configParam.uCfgDot11Mode) ||
        (eCSR_CFG_DOT11_MODE_AUTO == cfgDot11Mode) || (eCSR_CFG_DOT11_MODE_ABG == cfgDot11Mode) )
    {
        switch( pMac->roam.configParam.uCfgDot11Mode )
        {
            case eCSR_CFG_DOT11_MODE_11A:
                cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
                eBand = eCSR_BAND_5G;
                break;
            case eCSR_CFG_DOT11_MODE_11B:
                cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
                eBand = eCSR_BAND_24;
                break;
            case eCSR_CFG_DOT11_MODE_11G:
                cfgDot11Mode = eCSR_CFG_DOT11_MODE_11G;
                eBand = eCSR_BAND_24;
                break;            
            default:
                // Global dot11 Mode setting is 11a/b/g.
                // use the channel number to determine the Mode setting.
                if ( eCSR_OPERATING_CHANNEL_AUTO == operationChn )
                {
                    eBand = pMac->roam.configParam.eBand;
                    if(eCSR_BAND_24 == eBand)
                    {
                        //See reason in else if ( CSR_IS_CHANNEL_24GHZ(operationChn) ) to pick 11B
                        cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
                    }
                    else
                    {
                        //prefer 5GHz
                        eBand = eCSR_BAND_5G;
                        cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
                    }
                }
                else if ( CSR_IS_CHANNEL_24GHZ(operationChn) )
                {
                    // channel is a 2.4GHz channel.  Set mode to 11g.
                    //
                    // !!LAC - WiFi tests require IBSS networks to start in 11b mode without any change to the
                    // default parameter settings on the adapter.  We use ACU to start an IBSS through creation
                    // of a startIBSS profile.   this startIBSS profile has Auto MACProtocol and the 
                    // adapter property setting for dot11Mode is also AUTO.   So in this case, let's start 
                    // the IBSS network in 11b mode instead of 11g mode.
                    //
                    // so this is for Auto=profile->MacProtocol && Auto=Global.dot11Mode && profile->channel is < 14, 
                    // then start the IBSS in b mode.
                    // 
                    // Note:  we used to have this start as an 11g IBSS for best performance... now to specify that
                    // the user will have to set the do11Mode in the property page to 11g to force it.
                    cfgDot11Mode = eCSR_CFG_DOT11_MODE_11B;
                    eBand = eCSR_BAND_24;
                }
                else 
                {   
                    // else, it's a 5.0GHz channel.  Set mode to 11a.
                    cfgDot11Mode = eCSR_CFG_DOT11_MODE_11A;
                    eBand = eCSR_BAND_5G;
                }
                break;
        }//switch
    }//if( eCSR_CFG_DOT11_MODE_ABG == cfgDot11Mode )
    else
    {
        if(eCSR_CFG_DOT11_MODE_TITAN != cfgDot11Mode)
        {
            //dot11 mode is set, lets pick the band
            if ( eCSR_OPERATING_CHANNEL_AUTO == operationChn )
            {
                // channel is Auto also. 
                eBand = pMac->roam.configParam.eBand;
                if(eCSR_BAND_ALL == eBand)
                {
                    //prefer 5GHz
                    eBand = eCSR_BAND_5G;
                }
            }
            else if ( CSR_IS_CHANNEL_24GHZ(operationChn) )
            {
                eBand = eCSR_BAND_24;
            }
            else 
            {   
                eBand = eCSR_BAND_5G;
            }
        }
        else
        {
            //We assume titan can only do 2.4 Ghz
            eBand = eCSR_BAND_24;
        }
    }
    if(pBand)
    {
        *pBand = eBand;
    }
    
    return( cfgDot11Mode );
}


eHalStatus csrRoamIssueStopBss( tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate )
{
    eHalStatus status;
    
#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        vos_log_ibss_pkt_type *pIbssLog;

        WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
        if(pIbssLog)
        {
            pIbssLog->eventId = WLAN_IBSS_EVENT_STOP_REQ;
            WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
        }
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    // Set the roaming substate to 'stop Bss request'...
    csrRoamSubstateChange( pMac, NewSubstate );
    // attempt to stop the Bss (reason code is ignored...)
    status = csrSendMBStopBssReqMsg( pMac, 0 );
    
    return (status);
}


//pNumChan is a caller allocated space with the sizeof pChannels
eHalStatus csrGetCfgValidChannels(tpAniSirGlobal pMac, tANI_U8 *pChannels, tANI_U32 *pNumChan)
{
   
    return (ccmCfgGetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST,
                  (tANI_U8 *)pChannels,
                  pNumChan));
}


tANI_BOOLEAN csrRoamIsChannelValid( tpAniSirGlobal pMac, tANI_U8 channel )
{
    tANI_BOOLEAN fValid = FALSE;
    tANI_U32 idxValidChannels;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);
    
    if (HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, pMac->roam.validChannelList, &len)))
    {
        for ( idxValidChannels = 0; ( idxValidChannels < len ); idxValidChannels++ )
        {
            if ( channel == pMac->roam.validChannelList[ idxValidChannels ] )
            {
                fValid = TRUE;
                break;
            }
        }
    }    
        
    return fValid;
}


tANI_BOOLEAN csrRoamIsValid40MhzChannel(tpAniSirGlobal pMac, tANI_U8 channel)
{
    tANI_BOOLEAN fValid = eANI_BOOLEAN_FALSE;
    tANI_U8 i;

    for(i = 0; i < pMac->scan.base40MHzChannels.numChannels; i++)
    {
        if(channel == pMac->scan.base40MHzChannels.channelList[i])
        {
            fValid = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    return (fValid);
}


//This function check and validate whether the NIC can do CB (40MHz)
static tAniCBSecondaryMode csrGetCBModeFromIes(tpAniSirGlobal pMac, tANI_U8 primaryChn, tDot11fBeaconIEs *pIes)
{
    tAniCBSecondaryMode eRet = eANI_CB_SECONDARY_NONE;
    tANI_U8 centerChn;

    //Figure what the other side's CB mode
    if(WNI_CFG_CHANNEL_BONDING_MODE_DISABLE != pMac->roam.configParam.ChannelBondingMode)
    {
        if(pIes->HTCaps.present && (eHT_CHANNEL_WIDTH_40MHZ == pIes->HTCaps.supportedChannelWidthSet))
        {
            if(pIes->HTInfo.present)
            {
                if(PHY_DOUBLE_CHANNEL_LOW_PRIMARY == pIes->HTInfo.secondaryChannelOffset)
                {
                    eRet = eANI_CB_SECONDARY_UP;
                    centerChn = primaryChn + CSR_CB_CENTER_CHANNEL_OFFSET;
                }
                else if(PHY_DOUBLE_CHANNEL_HIGH_PRIMARY == pIes->HTInfo.secondaryChannelOffset)
                {
                    eRet = eANI_CB_SECONDARY_DOWN;
                    centerChn = primaryChn - CSR_CB_CENTER_CHANNEL_OFFSET;
                }
                else
                {
                    //PHY_SINGLE_CHANNEL_CENTERED
                    centerChn = primaryChn;
                    eRet = eANI_CB_SECONDARY_NONE;
                }
                if((eANI_CB_SECONDARY_NONE != eRet) && !csrRoamIsValid40MhzChannel(pMac, centerChn))
                {
                    smsLog(pMac, LOGW, "  Invalid center channel (%d), disable 40MHz mode\n", centerChn);
                    eRet = eANI_CB_SECONDARY_NONE;
                }
            }
        }
    }

    return eRet;
}

tANI_BOOLEAN csrIsEncryptionInList( tpAniSirGlobal pMac, tCsrEncryptionList *pCipherList, eCsrEncryptionType encryptionType )
{
    tANI_BOOLEAN fFound = FALSE;
    tANI_U32 idx;

    for( idx = 0; idx < pCipherList->numEntries; idx++ )
    {
        if( pCipherList->encryptionType[idx] == encryptionType )
        {
            fFound = TRUE;
            break;
        }
    }

    return fFound;
}

tANI_BOOLEAN csrIsAuthInList( tpAniSirGlobal pMac, tCsrAuthList *pAuthList, eCsrAuthType authType )
{
    tANI_BOOLEAN fFound = FALSE;
    tANI_U32 idx;

    for( idx = 0; idx < pAuthList->numEntries; idx++ )
    {
        if( pAuthList->authType[idx] == authType )
        {
            fFound = TRUE;
            break;
        }
    }

    return fFound;
}

tANI_BOOLEAN csrIsSameProfile(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pProfile1, tCsrRoamProfile *pProfile2)
{
    tANI_BOOLEAN fCheck = eANI_BOOLEAN_FALSE;
    tCsrScanResultFilter *pScanFilter = NULL;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    if(pProfile1 && pProfile2)
    {
        status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = csrRoamPrepareFilterFromProfile(pMac, pProfile2, pScanFilter);
            if(HAL_STATUS_SUCCESS(status))
            {
                fCheck = eANI_BOOLEAN_FALSE;
                do
                {
                    tANI_U32 i;
                    for(i = 0; i < pScanFilter->SSIDs.numOfSSIDs; i++)
                    {
                        fCheck = csrIsSsidMatch( pMac, pScanFilter->SSIDs.SSIDList[i].SSID.ssId, 
                                                pScanFilter->SSIDs.SSIDList[i].SSID.length,
                                                pProfile1->SSID.ssId, pProfile1->SSID.length, eANI_BOOLEAN_FALSE );
                        if ( fCheck ) break;
                    }
                    if(!fCheck)
                    {
                        break;
                    }
                    if( !csrIsAuthInList( pMac, &pProfile2->AuthType, pProfile1->AuthType)
                        || pProfile2->BSSType != pProfile1->BSSType
                        || !csrIsEncryptionInList( pMac, &pProfile2->EncryptionType, pProfile1->EncryptionType )
                        )
                    {
                        fCheck = eANI_BOOLEAN_FALSE;
                        break;
                    }
                    //Match found
                    fCheck = eANI_BOOLEAN_TRUE;
                }while(0);
                csrFreeScanFilter(pMac, pScanFilter);
            }
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }
    
    return (fCheck);
}


tANI_BOOLEAN csrRoamIsSameProfileKeys(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pConnProfile, tCsrRoamProfile *pProfile2)
{
    tANI_BOOLEAN fCheck = eANI_BOOLEAN_FALSE;
    int i;

    do
    {
        //Only check for static WEP
        if(!csrIsEncryptionInList(pMac, &pProfile2->EncryptionType, eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) &&
            !csrIsEncryptionInList(pMac, &pProfile2->EncryptionType, eCSR_ENCRYPT_TYPE_WEP104_STATICKEY))
        {
            fCheck = eANI_BOOLEAN_TRUE;
            break;
        }
        if(!csrIsEncryptionInList(pMac, &pProfile2->EncryptionType, pConnProfile->EncryptionType)) break;
        if(pConnProfile->Keys.defaultIndex != pProfile2->Keys.defaultIndex) break;
        for(i = 0; i < CSR_MAX_NUM_KEY; i++)
        {
            if(pConnProfile->Keys.KeyLength[i] != pProfile2->Keys.KeyLength[i]) break;
            if(!palEqualMemory(pMac->hHdd, &pConnProfile->Keys.KeyMaterial[i], 
                            &pProfile2->Keys.KeyMaterial[i], pProfile2->Keys.KeyLength[i]))
            {
                break;
            }
        }
        if( i == CSR_MAX_NUM_KEY)
        {
            fCheck = eANI_BOOLEAN_TRUE;
        }
    }while(0);

    return (fCheck);
}


//IBSS


tANI_U8 csrRoamGetIbssStartChannelNumber50( tpAniSirGlobal pMac )
{
    tANI_U8 channel = 0;     
    tANI_U32 idx;
    tANI_U32 idxValidChannels;
    tANI_BOOLEAN fFound = FALSE;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);
    
    if(eCSR_OPERATING_CHANNEL_ANY != pMac->roam.configParam.AdHocChannel5G)
    {
        channel = pMac->roam.configParam.AdHocChannel5G;
        if(!csrRoamIsChannelValid(pMac, channel))
        {
            channel = 0;
        }
    }
    if (0 == channel && HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
    {
        for ( idx = 0; ( idx < CSR_NUM_IBSS_START_CHANNELS_50 ) && !fFound; idx++ ) 
        {
            for ( idxValidChannels = 0; ( idxValidChannels < len ) && !fFound; idxValidChannels++ )
            {
                if ( csrStartIbssChannels50[ idx ] == pMac->roam.validChannelList[ idxValidChannels ] )
                {
                    fFound = TRUE;
                    channel = csrStartIbssChannels50[ idx ];
                }
            }
        }

        // this is rare, but if it does happen, we find anyone in 11a bandwidth and return the first 11a channel found!
        if (!fFound)    
        {
            for ( idxValidChannels = 0; idxValidChannels < len ; idxValidChannels++ )
            {
                if ( CSR_IS_CHANNEL_5GHZ(pMac->roam.validChannelList[ idx ]) )   // the max channel# in 11g is 14
                {
                    channel = csrStartIbssChannels50[ idx ];
                    break;
                }
            }
        }
    }//if
    
    return( channel );    
}


tANI_U8 csrRoamGetIbssStartChannelNumber24( tpAniSirGlobal pMac )
{
    tANI_U8 channel = 1;
    tANI_U32 idx;
    tANI_U32 idxValidChannels;
    tANI_BOOLEAN fFound = FALSE;
    tANI_U32 len = sizeof(pMac->roam.validChannelList);
    
    if(eCSR_OPERATING_CHANNEL_ANY != pMac->roam.configParam.AdHocChannel24)
    {
        channel = pMac->roam.configParam.AdHocChannel24;
        if(!csrRoamIsChannelValid(pMac, channel))
        {
            channel = 0;
        }
    }
    
    if (0 == channel && HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &len)))
    {
        for ( idx = 0; ( idx < CSR_NUM_IBSS_START_CHANNELS_24 ) && !fFound; idx++ ) 
        {
            for ( idxValidChannels = 0; ( idxValidChannels < len ) && !fFound; idxValidChannels++ )
            {
                if ( csrStartIbssChannels24[ idx ] == pMac->roam.validChannelList[ idxValidChannels ] )
                {
                    fFound = TRUE;
                    channel = csrStartIbssChannels24[ idx ];
                }
            }
        }
    }
    
    return( channel );    
}


static void csrRoamGetIbssStartParms( tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tCsrRoamIbssParams *pParam )
{
    eCsrCfgDot11Mode cfgDot11Mode;
    eCsrBand eBand;
    tANI_U8 channel = 0;
    tSirNwType nwType;
    tANI_U8 operationChannel = 0; 
    
    if(pProfile->ChannelInfo.numOfChannels && pProfile->ChannelInfo.ChannelList)
    {
       operationChannel = pProfile->ChannelInfo.ChannelList[0];
    }
    
    cfgDot11Mode = csrRoamGetPhyModeBandForIBSS( pMac, (eCsrPhyMode)pProfile->phyMode, operationChannel, &eBand );
    
    switch( cfgDot11Mode )
    {
        case eCSR_CFG_DOT11_MODE_11G:
        case eCSR_CFG_DOT11_MODE_TITAN:
            nwType = eSIR_11G_NW_TYPE;
            break;

        case eCSR_CFG_DOT11_MODE_11B:
            nwType = eSIR_11B_NW_TYPE;
            break;   

        case eCSR_CFG_DOT11_MODE_11A:
            nwType = eSIR_11A_NW_TYPE;
            break;

        default:
        case eCSR_CFG_DOT11_MODE_POLARIS:
        case eCSR_CFG_DOT11_MODE_11N:
        case eCSR_CFG_DOT11_MODE_TAURUS:
            //Because LIM only verifies it against 11a, 11b or 11g, set only 11g or 11a here
            if(eCSR_BAND_24 == eBand)
            {
                nwType = eSIR_11G_NW_TYPE;
            }
            else
            {
                nwType = eSIR_11A_NW_TYPE;
            }
            break;
    }   
    
    switch ( nwType )
    {
        default:
            smsLog(pMac, LOGE, FL("sees an unknown pSirNwType (%d)\n"), nwType);
        case eSIR_11A_NW_TYPE:
        
            pParam->sirRateSet.numRates = 8;
        
            pParam->sirRateSet.rate[0] = SIR_MAC_RATE_6 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[1] = SIR_MAC_RATE_9;
            pParam->sirRateSet.rate[2] = SIR_MAC_RATE_12 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[3] = SIR_MAC_RATE_18;
            pParam->sirRateSet.rate[4] = SIR_MAC_RATE_24 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[5] = SIR_MAC_RATE_36;
            pParam->sirRateSet.rate[6] = SIR_MAC_RATE_48;
            pParam->sirRateSet.rate[7] = SIR_MAC_RATE_54;
            
            if ( eCSR_OPERATING_CHANNEL_ANY == operationChannel ) 
            {
                channel = csrRoamGetIbssStartChannelNumber50( pMac );
                if( 0 == channel &&
                    CSR_IS_PHY_MODE_DUAL_BAND(pProfile->phyMode) && 
                    CSR_IS_PHY_MODE_DUAL_BAND(pMac->roam.configParam.phyMode) 
                    )
                {
                    //We could not find a 5G channel by auto pick, let's try 2.4G channels
                    //We only do this here because csrRoamGetPhyModeBandForIBSS always picks 11a for AUTO
                    nwType = eSIR_11B_NW_TYPE;
                    channel = csrRoamGetIbssStartChannelNumber24( pMac );
                    pParam->sirRateSet.numRates = 4;
                    pParam->sirRateSet.rate[0] = SIR_MAC_RATE_1 | CSR_DOT11_BASIC_RATE_MASK;
                    pParam->sirRateSet.rate[1] = SIR_MAC_RATE_2 | CSR_DOT11_BASIC_RATE_MASK;
                    pParam->sirRateSet.rate[2] = SIR_MAC_RATE_5_5 | CSR_DOT11_BASIC_RATE_MASK;
                    pParam->sirRateSet.rate[3] = SIR_MAC_RATE_11 | CSR_DOT11_BASIC_RATE_MASK;
                }
            }
            else 
            {
                channel = operationChannel;
            }
            break;
            
        case eSIR_11B_NW_TYPE:
        case eSIR_11G_NW_TYPE:
            
            pParam->sirRateSet.numRates = 4;

            pParam->sirRateSet.rate[0] = SIR_MAC_RATE_1 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[1] = SIR_MAC_RATE_2 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[2] = SIR_MAC_RATE_5_5 | CSR_DOT11_BASIC_RATE_MASK;
            pParam->sirRateSet.rate[3] = SIR_MAC_RATE_11 | CSR_DOT11_BASIC_RATE_MASK;
            
            if ( eCSR_OPERATING_CHANNEL_ANY == operationChannel ) 
            {
                channel = csrRoamGetIbssStartChannelNumber24( pMac );
            }
            else 
            {
                channel = operationChannel;
            }
            
            break;            
    }
    pMac->roam.IbssParams.operationChn = channel;
    pMac->roam.IbssParams.sirNwType = nwType;
}


static void csrRoamGetIbssStartParmsFromBssDesc( tpAniSirGlobal pMac, tSirBssDescription *pBssDesc, 
                                                 tDot11fBeaconIEs *pIes, tCsrRoamIbssParams *pParam )
{
    
    if( pParam )
    {
        pParam->sirNwType = pBssDesc->nwType;
        pParam->cbMode = eANI_CB_SECONDARY_NONE;
        pParam->operationChn = pBssDesc->channelId;
    
        if( pIes )
        {
            if(pIes->SuppRates.present)
            {
                pParam->sirRateSet.numRates = pIes->SuppRates.num_rates;
                palCopyMemory(pMac->hHdd, pParam->sirRateSet.rate, pIes->SuppRates.rates, sizeof(*pIes->SuppRates.rates) * pIes->SuppRates.num_rates);
            }
            if( pIes->SSID.present )
            {
                pParam->ssId.length = pIes->SSID.num_ssid;
                palCopyMemory(pMac->hHdd, pParam->ssId.ssId, pIes->SSID.ssid, pParam->ssId.length);
            }
            pParam->cbMode = csrGetCBModeFromIes(pMac, pParam->operationChn, pIes);

        }
        else
        {
            pParam->ssId.length = 0;
            pParam->sirRateSet.numRates = 0;
        }
    }
}


static void csrRoamDetermineMaxRateForAdHoc( tpAniSirGlobal pMac, tSirMacRateSet *pSirRateSet )
{
    tANI_U8 MaxRate = 0;
    tANI_U32 i;
    tANI_U8 *pRate;    
   
    pRate = pSirRateSet->rate;
    for ( i = 0; i < pSirRateSet->numRates; i++ )
    {
        MaxRate = CSR_MAX( MaxRate, ( pRate[ i ] & (~CSR_DOT11_BASIC_RATE_MASK) ) );
    }
    
    // Save the max rate in the connected state information...
    
    // modify LastRates variable as well
    
    return;
}


//this function finds a valid secondary channel for channel bonding with "channel".
//Param: channel -- primary channel, caller must validate it
//       cbChoice -- CB directory
//Return: if 0, no secondary channel is found. Otherwise a valid secondary channel.
static tANI_U8 csrRoamGetSecondaryChannel(tpAniSirGlobal pMac, tANI_U8 channel, eCsrCBChoice cbChoice)
{
    tANI_U8 chnUp = 0, chnDown = 0, chnRet = 0;

    switch (cbChoice)
    {
    case eCSR_CB_OFF:
        chnUp = 0;
        chnDown = 0;
        break;
    case eCSR_CB_DOWN:
        chnUp = 0;
        chnDown = channel - CSR_CB_CHANNEL_GAP;
        break;
    case eCSR_CB_UP:
        chnUp = channel + CSR_CB_CHANNEL_GAP;
        chnDown = 0;
        break;
    case eCSR_CB_AUTO:
    //consider every other value means auto
    default:
        chnUp = channel + CSR_CB_CHANNEL_GAP;
        chnDown = channel - CSR_CB_CHANNEL_GAP;
        break;
    }

    //if CB_UP or auto, try channel up first
    if(chnUp && CSR_IS_SAME_BAND_CHANNELS(chnUp, channel) && csrRoamIsChannelValid(pMac, chnUp))
    {
        //found a valid up channel for channel bonding
        //check whether the center channel is valid
        if(csrRoamIsValid40MhzChannel(pMac, channel + CSR_CB_CENTER_CHANNEL_OFFSET))
        {
            chnRet = chnUp;
        }
    }
    if(chnRet == 0 && chnDown && CSR_IS_SAME_BAND_CHANNELS(chnDown, channel) && csrRoamIsChannelValid(pMac, chnDown))
    {
        //found a valid down channel for channel bonding
        if(csrRoamIsValid40MhzChannel(pMac, channel - CSR_CB_CENTER_CHANNEL_OFFSET))
        {
            chnRet = chnDown;
        }
    }

    return chnRet;
}


eHalStatus csrRoamIssueStartIbss( tpAniSirGlobal pMac, tCsrRoamIbssParams *pParam, tCsrRoamProfile *pProfile, tSirBssDescription *pBssDesc, tANI_U32 roamId )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    // Set the roaming substate to 'Start BSS attempt'...
    csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_START_BSS_REQ );

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        vos_log_ibss_pkt_type *pIbssLog;

        WLAN_VOS_DIAG_LOG_ALLOC(pIbssLog, vos_log_ibss_pkt_type, LOG_WLAN_IBSS_C);
        if(pIbssLog)
        {
            if(pBssDesc)
            {
                pIbssLog->eventId = WLAN_IBSS_EVENT_JOIN_IBSS_REQ;
                palCopyMemory(pMac->hHdd, pIbssLog->bssid, pBssDesc->bssId, 6);
            }
            else
            {
                pIbssLog->eventId = WLAN_IBSS_EVENT_START_IBSS_REQ;
            }
            palCopyMemory(pMac->hHdd, pIbssLog->ssid, pParam->ssId.ssId,
                pParam->ssId.length);
            if(pProfile->ChannelInfo.numOfChannels == 0)
            {
                pIbssLog->channelSetting = AUTO_PICK;
            }
            else
            {
                pIbssLog->channelSetting = SPECIFIED;
            }
            pIbssLog->operatingChannel = pParam->operationChn;
            WLAN_VOS_DIAG_LOG_REPORT(pIbssLog);
        }
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    // When starting an IBSS, start on the channel from the Profile.
    status = csrSendMBStartBssReqMsg( pMac, eCSR_BSS_TYPE_START_IBSS, pParam );

    return (status);
}


static void csrRoamPrepareIbssParams(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
                                     tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes)
{
    tANI_U8 Channel, SecondChn;
    tAniCBSecondaryMode cbMode = eANI_CB_SECONDARY_NONE;
    eCsrCBChoice cbChoice;

    if( pBssDesc )
    {
        csrRoamGetIbssStartParmsFromBssDesc( pMac, pBssDesc, pIes, &pMac->roam.IbssParams );
    }
    else
    {
        csrRoamGetIbssStartParms(pMac, pProfile, &pMac->roam.IbssParams);

        //Use the first SSID
        if(pProfile->SSIDs.numOfSSIDs)
        {
            palCopyMemory(pMac->hHdd, &pMac->roam.IbssParams.ssId, &pProfile->SSIDs.SSIDList[0].SSID, sizeof(tSirMacSSid));
        }
    }
    Channel = pMac->roam.IbssParams.operationChn;
    if(Channel == 0)
    {
        smsLog(pMac, LOGW, "   CSR cannot find a channel to start IBSS\n");
    }
    else
    {
  
        csrRoamDetermineMaxRateForAdHoc( pMac, &pMac->roam.IbssParams.sirRateSet );

        if( CSR_IS_START_IBSS( pProfile ) )
        {
           //TBH: channel bonding is not supported for Libra
            if( pProfile->ChannelInfo.ChannelList && eCSR_OPERATING_CHANNEL_AUTO != pProfile->ChannelInfo.ChannelList[0] )
            {
                Channel = pProfile->ChannelInfo.ChannelList[0];
                cbChoice = pProfile->CBMode;
            }
            else {
                cbChoice = pMac->roam.configParam.cbChoice;
            }
            pMac->roam.IbssParams.operationChn = Channel;
            //make sure channel is valid
            if(!csrRoamIsChannelValid(pMac, Channel))
            {
                //set Channel to 0 to let lim know this is invalid
                //We still send this request down to lim even though we know the channel is wrong because
                //lim will response with error and hdd's eWNI_SME_START_BSS_RSP handler will roam other profile (if any)
                Channel = 0;
                pMac->roam.IbssParams.operationChn = 0;
            }
            else {
                //now we have a valid channel
                if(WNI_CFG_CHANNEL_BONDING_MODE_DISABLE != pMac->roam.configParam.ChannelBondingMode)
                {
                    //let's pick a secondard channel
                    SecondChn = csrRoamGetSecondaryChannel(pMac, Channel, cbChoice);

                    if(SecondChn > Channel)
                    {
                        cbMode = eANI_CB_SECONDARY_UP;
                    }
                    else if(SecondChn && SecondChn < Channel)
                    {
                        cbMode =eANI_CB_SECONDARY_DOWN;
                    }
                    else
                    {
                        cbMode = eANI_CB_SECONDARY_NONE;
                    }
                    pMac->roam.IbssParams.cbMode = cbMode;
                }
                else
                {
                    pMac->roam.IbssParams.cbMode = eANI_CB_SECONDARY_NONE;
                }
            }
        }
    }
}



static eHalStatus csrRoamStartIbss( tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tSirBssDescription *pBssDesc, tANI_BOOLEAN *pfSameIbss )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fSameIbss = FALSE;
     
    if ( csrIsConnStateIbss( pMac ) ) 
    { 
        // Check if any profile parameter has changed ? If any profile parameter
        // has changed then stop old BSS and start a new one with new parameters
        if ( csrIsSameProfile( pMac, &pMac->roam.connectedProfile, pProfile ) ) 
        {
            fSameIbss = TRUE;
        }
        else
        {
            status = csrRoamIssueStopBss( pMac, eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING );
        }       
    }
    else if ( csrIsConnStateConnectedInfra( pMac ) ) 
    {
        // Disassociate from the connected Infrastructure network...
        status = csrRoamIssueDisassociate( pMac, eCSR_ROAM_SUBSTATE_DISCONNECT_CONTINUE_ROAMING, FALSE );
    }
    else 
    {
        tBssConfigParam *pBssConfig;
        
        status = palAllocateMemory(pMac->hHdd, (void **)&pBssConfig, sizeof(tBssConfigParam)); 
        if(HAL_STATUS_SUCCESS(status))
        {
            palZeroMemory(pMac->hHdd, pBssConfig, sizeof(tBssConfigParam));
            // there is no Bss description before we start an IBSS so we need to adopt
            // all Bss configuration parameters from the Profile.
            status = csrRoamPrepareIBSSConfigFromProfile(pMac, pProfile, pBssConfig);
            if(HAL_STATUS_SUCCESS(status))
            {
                //Prepare some more parameters for this IBSS
                csrRoamPrepareIbssParams(pMac, pProfile, NULL, NULL);
                status = csrRoamSetBssConfigCfg(pMac, pProfile, NULL, pBssConfig, NULL);
            }
            
            palFreeMemory(pMac->hHdd, pBssConfig);
        }//Allocate memory
    }
    
    if(pfSameIbss)
    {
        *pfSameIbss = fSameIbss;
    }
    return( status );
}


static void csrRoamUpdateConnectedProfileFromNewBss( tpAniSirGlobal pMac, tSirSmeNewBssInfo *pNewBss )
{
    if( pNewBss )
    {
        // Set the operating channel.
        pMac->roam.connectedProfile.operationChannel = pNewBss->channelNumber;
        // move the BSSId from the BSS description into the connected state information.
        palCopyMemory( pMac->hHdd, &pMac->roam.connectedProfile.bssid, 
                      &(pNewBss->bssId), sizeof( tCsrBssid ) );    
    }

    return;
}


eHalStatus csrRoamSetPMKIDCache( tHalHandle hHal, tPmkidCacheInfo *pPMKIDCache, tANI_U32 numItems )
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    smsLog(pMac, LOGW, "csrRoamSetPMKIDCache called, numItems = %d\n", numItems);
    if(numItems <= CSR_MAX_PMKID_ALLOWED)
    {

#ifdef FEATURE_WLAN_DIAG_SUPPORT
        {
            WLAN_VOS_DIAG_EVENT_DEF(secEvent, vos_event_wlan_security_payload_type);
            palZeroMemory(pMac->hHdd, &secEvent, sizeof(vos_event_wlan_security_payload_type));
            secEvent.eventId = WLAN_SECURITY_EVENT_PMKID_UPDATE;
            secEvent.encryptionModeMulticast = 
                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.mcEncryptionType);
            secEvent.encryptionModeUnicast = 
                (v_U8_t)diagEncTypeFromCSRType(pMac->roam.connectedProfile.EncryptionType);
            palCopyMemory( pMac->hHdd, secEvent.bssid, pMac->roam.connectedProfile.bssid, 6 );
            secEvent.authMode = 
                (v_U8_t)diagAuthTypeFromCSRType(pMac->roam.connectedProfile.AuthType);
            WLAN_VOS_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
        }
#endif//FEATURE_WLAN_DIAG_SUPPORT

        status = eHAL_STATUS_SUCCESS;
        //numItems may be 0 to clear the cache
        pMac->roam.NumPmkidCache = (tANI_U16)numItems;
        if(numItems && pPMKIDCache)
        {
            status = palCopyMemory( pMac->hHdd, pMac->roam.PmkidCacheInfo, pPMKIDCache,
                            sizeof(tPmkidCacheInfo) * numItems );
        }
    }

    return (status);
}


tANI_U32 csrRoamGetNumPMKIDCache(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return (pMac->roam.NumPmkidCache);
}


eHalStatus csrRoamGetPMKIDCache(tHalHandle hHal, tANI_U32 *pNum, tPmkidCacheInfo *pPmkidCache)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    if(pNum && pPmkidCache)
    {
        if(pMac->roam.NumPmkidCache == 0)
        {
            *pNum = 0;
            status = eHAL_STATUS_SUCCESS;
        }
        else if(*pNum >= pMac->roam.NumPmkidCache)
        {
            palCopyMemory( pMac->hHdd, pPmkidCache, pMac->roam.PmkidCacheInfo,
                            sizeof(tPmkidCacheInfo) * pMac->roam.NumPmkidCache );
            *pNum = pMac->roam.NumPmkidCache;
            status = eHAL_STATUS_SUCCESS;
        }
    }

    return (status);
}


eHalStatus csrRoamGetWpaRsnReqIE(tHalHandle hHal, tANI_U32 *pLen, tANI_U8 *pBuf)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tANI_U32 len;

    if(pLen)
    {
        len = *pLen;
        *pLen = pMac->roam.nWpaRsnReqIeLength;
        if(pBuf)
        {
            if(len >= pMac->roam.nWpaRsnReqIeLength)
            {
                status = palCopyMemory(pMac->hHdd, pBuf, pMac->roam.pWpaRsnReqIE, pMac->roam.nWpaRsnReqIeLength);
            }
        }
    }

    return (status);
}


eHalStatus csrRoamGetWpaRsnRspIE(tHalHandle hHal, tANI_U32 *pLen, tANI_U8 *pBuf)
{
    eHalStatus status = eHAL_STATUS_INVALID_PARAMETER;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tANI_U32 len;

    if(pLen)
    {
        len = *pLen;
        *pLen = pMac->roam.nWpaRsnRspIeLength;
        if(pBuf)
        {
            if(len >= pMac->roam.nWpaRsnRspIeLength)
            {
                status = palCopyMemory(pMac->hHdd, pBuf, pMac->roam.pWpaRsnRspIE, pMac->roam.nWpaRsnRspIeLength);
            }
        }
    }

    return (status);
}



eRoamCmdStatus csrGetRoamCompleteStatus(tpAniSirGlobal pMac)
{
    eRoamCmdStatus retStatus = eCSR_ROAM_CONNECT_COMPLETION;

    if(CSR_IS_ROAMING(pMac))
    {
        retStatus = eCSR_ROAM_ROAMING_COMPLETION;
        pMac->roam.fRoaming = eANI_BOOLEAN_FALSE;
    }

    return (retStatus);
}


//This function remove the connected BSS from te cached scan result
eHalStatus csrRoamRemoveConnectedBssFromScanCache(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tCsrScanResultFilter *pScanFilter = NULL;
    tListElem *pEntry;
    tCsrScanResult *pResult;
	tDot11fBeaconIEs *pIes;

    if(!(csrIsMacAddressZero(pMac, &pMac->roam.connectedProfile.bssid) ||
            csrIsMacAddressBroadcast(pMac, &pMac->roam.connectedProfile.bssid)))
    {
        do
        {
            //Prepare the filter. Only fill in the necessary fields. Not all fields are needed
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter, sizeof(tCsrScanResultFilter));
            if(!HAL_STATUS_SUCCESS(status)) break;
            palZeroMemory(pMac->hHdd, pScanFilter, sizeof(tCsrScanResultFilter));
            status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter->BSSIDs.bssid, sizeof(tCsrBssid));
            if(!HAL_STATUS_SUCCESS(status)) break;
            palCopyMemory(pMac->hHdd, pScanFilter->BSSIDs.bssid, &pMac->roam.connectedProfile.bssid, sizeof(tCsrBssid));
            pScanFilter->BSSIDs.numOfBSSIDs = 1;
            if(!csrIsNULLSSID(pMac->roam.connectedProfile.SSID.ssId, pMac->roam.connectedProfile.SSID.length))
            {
                status = palAllocateMemory(pMac->hHdd, (void **)&pScanFilter->SSIDs.SSIDList, sizeof(tCsrSSIDInfo));
                if(!HAL_STATUS_SUCCESS(status)) break;
                palCopyMemory(pMac->hHdd, &pScanFilter->SSIDs.SSIDList[0].SSID, &pMac->roam.connectedProfile.SSID, sizeof(tSirMacSSid));
            }
            pScanFilter->authType.numEntries = 1;
            pScanFilter->authType.authType[0] = pMac->roam.connectedProfile.AuthType;
            pScanFilter->BSSType = pMac->roam.connectedProfile.BSSType;
            pScanFilter->EncryptionType.numEntries = 1;
            pScanFilter->EncryptionType.encryptionType[0] = pMac->roam.connectedProfile.EncryptionType;
            pScanFilter->mcEncryptionType.numEntries = 1;
            pScanFilter->mcEncryptionType.encryptionType[0] = pMac->roam.connectedProfile.mcEncryptionType;
            //We ignore the channel for now, BSSID should be enough
            pScanFilter->ChannelInfo.numOfChannels = 0;
            //Also ignore the following fields
            pScanFilter->uapsd_mask = 0;
            pScanFilter->bWPSAssociation = eANI_BOOLEAN_FALSE;
            pScanFilter->countryCode[0] = 0;
            pScanFilter->phyMode = eCSR_DOT11_MODE_TAURUS;

            csrLLLock(&pMac->scan.scanResultList);
            pEntry = csrLLPeekHead( &pMac->scan.scanResultList, LL_ACCESS_NOLOCK );
            while( pEntry ) 
            {
                pResult = GET_BASE_ADDR( pEntry, tCsrScanResult, Link );
				pIes = (tDot11fBeaconIEs *)( pResult->Result.pvIes );
                if(csrMatchBSS(pMac, &pResult->Result.BssDescriptor, 
                               pScanFilter, NULL, NULL, NULL, &pIes))
                {
                    //We found the one
                    if( csrLLRemoveEntry(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK) )
                    {
                        //Free the memory
                        csrFreeScanResultEntry( pMac, pResult );
                    }
                    break;
                }
                pEntry = csrLLNext(&pMac->scan.scanResultList, pEntry, LL_ACCESS_NOLOCK);
            }//while
            csrLLUnlock(&pMac->scan.scanResultList);
        }while(0);
        if(pScanFilter)
        {
            csrFreeScanFilter(pMac, pScanFilter);
            palFreeMemory(pMac->hHdd, pScanFilter);
        }
    }
    return (status);
}



////////////////////Mail box


//pBuf is caller allocated memory point to &(tSirSmeJoinReq->rsnIE.rsnIEdata[ 0 ]) + pMsg->rsnIE.length;
//or &(tSirSmeReassocReq->rsnIE.rsnIEdata[ 0 ]) + pMsg->rsnIE.length;
static void csrPropareJoinReassocReqBuffer( tpAniSirGlobal pMac, 
                                            tSirBssDescription *pBssDescription, 
                                            tANI_U8 *pBuf, tANI_U8 uapsdMask)
{
    tCsrChannelSet channelGroup;
    tSirMacCapabilityInfo *pAP_capabilityInfo;
    tAniBool fTmp;
    tANI_BOOLEAN found = FALSE;
    tANI_U32 size = 0;
    tANI_U16 i;

    // plug in neighborhood occupancy info (i.e. BSSes on primary or secondary channels)
    *pBuf++ = (tANI_U8)FALSE;  //tAniTitanCBNeighborInfo->cbBssFoundPri
    *pBuf++ = (tANI_U8)FALSE;  //tAniTitanCBNeighborInfo->cbBssFoundSecDown
    *pBuf++ = (tANI_U8)FALSE;  //tAniTitanCBNeighborInfo->cbBssFoundSecUp

    // 802.11h
    //We can do this because it is in HOST CPU order for now.
    pAP_capabilityInfo = (tSirMacCapabilityInfo *)&pBssDescription->capabilityInfo;

    //tell the target AP my 11H capability only if both AP and STA support 11H and the channel being used is 11a
    if ( csrIs11hSupported( pMac ) && pAP_capabilityInfo->spectrumMgt && eSIR_11A_NW_TYPE == pBssDescription->nwType )   
    {  
        fTmp = (tAniBool)pal_cpu_to_be32(1);
        // corresponds to --- pMsg->spectrumMgtIndicator = ON;
        palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *)&fTmp, sizeof(tAniBool) );
        pBuf += sizeof(tAniBool);
        *pBuf++ = 0;         // it is for pMsg->powerCap.minTxPower = 0;
        found = csrSearchChannelListForTxPower(pMac, pBssDescription, &channelGroup);
        if ( found ) 
        {
            *pBuf++ = channelGroup.txPower;     // it is for pMsg->powerCap.maxTxPower
        }
        else 
        {
            *pBuf++ = SIR_11A_DEFAULT_MAX_TRANSMIT_POWER;   // it is for  pMsg->powerCap.maxTxPower;
        }
        size = sizeof(pMac->roam.validChannelList);
        if(HAL_STATUS_SUCCESS(csrGetCfgValidChannels(pMac, (tANI_U8 *)pMac->roam.validChannelList, &size)))
        { 
            *pBuf++ = (tANI_U8)size;	//tSirSupChnl->numChnl	
            for ( i = 0; i < size; i++) 
            {
                *pBuf++ = pMac->roam.validChannelList[ i ];   //tSirSupChnl->channelList[ i ]
             
            }
        }
        else
        {
            smsLog(pMac, LOGE, FL("can not find any valid channel\n"));
            *pBuf++ = 0;  //tSirSupChnl->numChnl
        }                                                                                                                     
	     
    } 
    else 
    {
        fTmp = eSIR_FALSE;
        // corresponds to --- pMsg->spectrumMgtIndicator = OFF;
        palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *)&fTmp, sizeof(tAniBool) );
        pBuf += sizeof(tAniBool);
    }
    *pBuf++ = uapsdMask;
  

    // move the entire BssDescription into the join request.
    palCopyMemory( pMac->hHdd, pBuf, pBssDescription, 
                    pBssDescription->length + sizeof( pBssDescription->length ) );

    pBuf += pBssDescription->length + sizeof( pBssDescription->length );   // update to new location
}


/* 
  * The communication between HDD and LIM is thru mailbox (MB).
  * Both sides will access the data structure "tSirSmeJoinReq".
  *  The rule is, while the components of "tSirSmeJoinReq" can be accessed in the regular way like tSirSmeJoinReq.assocType, this guideline
  *  stops at component tSirRSNie; any acces to the components after tSirRSNie is forbidden because the space from tSirRSNie is quueezed
  *  with the component "tSirBssDescription". And since the size of actual 'tSirBssDescription' varies, the receiving side (which is the routine
  *  limJoinReqSerDes() of limSerDesUtils.cc) should keep in mind not to access the components DIRECTLY after tSirRSNie.
  */
eHalStatus csrSendJoinReqMsg( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription, 
                              tCsrRoamProfile *pProfile, tDot11fBeaconIEs *pIes )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeJoinReq *pMsg;
    tANI_U8 *pBuf;
    tANI_U16 msgLen;

	do {
        pMac->roam.joinFailStatusCode.statusCode = eSIR_SME_SUCCESS;
        pMac->roam.joinFailStatusCode.reasonCode = 0;
		// There are a number of variable length fields to consider.  First, the tSirSmeJoinReq
		// includes a single bssDescription.   bssDescription includes a single tANI_U32 for the 
		// IE fields, but the length field in the bssDescription needs to be interpreted to 
		// determine length of the IE fields.
		//
		// So, take the size of the JoinReq, subtract the size of the bssDescription and 
		// add in the length from the bssDescription (then add the size of the 'length' field
		// itself because that is NOT included in the length field).
		msgLen = sizeof( tSirSmeJoinReq ) - sizeof( *pBssDescription ) + 
		         pBssDescription->length + sizeof( pBssDescription->length ) +
		         sizeof( tCsrWpaIe ) + sizeof( tCsrWpaAuthIe ) + sizeof( tANI_U16 ); // add in the size of the WPA IE that we may build.

        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_JOIN_REQ);
		pMsg->length = pal_cpu_to_be16(msgLen);
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
		pMsg->assocType = pal_cpu_to_be32(eSIR_NORMAL);
#endif
    

        if ( csrIsProfileWpa( pProfile ) )
        {
            // Insert the Wpa IE into the join request
            pMsg->rsnIE.length = csrRetrieveWpaIe( pMac, pProfile, pBssDescription, pIes,
                                                   (tCsrWpaIe *)( pMsg->rsnIE.rsnIEdata ) );
        }
        else if( csrIsProfileRSN( pProfile ) )
        {
            // Insert the RSN IE into the join request
            pMsg->rsnIE.length = csrRetrieveRsnIe( pMac, pProfile, pBssDescription, pIes,
                                                    (tCsrRSNIe *)( pMsg->rsnIE.rsnIEdata ) );
        }
        else
        {
            pMsg->rsnIE.length = 0;
        }
        //remember the IE for future use
        if(pMsg->rsnIE.length)
        {
            //Check whether we need to allocate more memory
            if(pMsg->rsnIE.length > pMac->roam.nWpaRsnReqIeLength)
            {
                if(pMac->roam.pWpaRsnReqIE && pMac->roam.nWpaRsnReqIeLength)
                {
                    palFreeMemory(pMac->hHdd, pMac->roam.pWpaRsnReqIE);
                }
                status = palAllocateMemory(pMac->hHdd, (void **)&pMac->roam.pWpaRsnReqIE, pMsg->rsnIE.length);
                if(!HAL_STATUS_SUCCESS(status)) break;
            }
            pMac->roam.nWpaRsnReqIeLength = pMsg->rsnIE.length;
            palCopyMemory(pMac->hHdd, pMac->roam.pWpaRsnReqIE, pMsg->rsnIE.rsnIEdata, pMsg->rsnIE.length);
        }
        else
        {
            //free whatever old info
            pMac->roam.nWpaRsnReqIeLength = 0;
            if(pMac->roam.pWpaRsnReqIE)
            {
                palFreeMemory(pMac->hHdd, pMac->roam.pWpaRsnReqIE);
                pMac->roam.pWpaRsnReqIE = NULL;
            }
        }

        pBuf = &(pMsg->rsnIE.rsnIEdata[ 0 ]) + pMsg->rsnIE.length;

        csrPropareJoinReassocReqBuffer( pMac, pBssDescription, pBuf, 
                                        (tANI_U8)pProfile->uapsd_mask);

        pMsg->rsnIE.length = pal_cpu_to_be16(pMsg->rsnIE.length);
        status = palSendMBMessage(pMac->hHdd, pMsg );    
        if(!HAL_STATUS_SUCCESS(status)) break;
        //Tush-QoS: notify QoS module that join happening
        else
        {
           sme_QosCsrEventInd(pMac, SME_QOS_CSR_JOIN_REQ, NULL);
        }

    } while( 0 );

    return( status );

}


eHalStatus csrSendSmeReassocReqMsg( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription, 
                                    tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile )
{
    eHalStatus status;
    tSirSmeReassocReq *pMsg;
    tANI_U8 *pBuf;
    tANI_U16 msgLen;
    v_U8_t acm_mask = 0, uapsd_mask;

    do {
        // There are a number of variable length fields to consider.  First, the tSirSmeJoinReq
        // includes a single bssDescription.   bssDescription includes a single tANI_U32 for the 
        // IE fields, but the length field in the bssDescription needs to be interpreted to 
        // determine length of the IE fields.
        //
        // So, take the size of the JoinReq, subtract the size of the bssDescription and 
        // add in the length from the bssDescription (then add the size of the 'length' field
        // itself because that is NOT included in the length field).
        msgLen = sizeof( tSirSmeReassocReq ) - sizeof( *pBssDescription ) + 
                 pBssDescription->length + sizeof( pBssDescription->length ) +
                 sizeof( tCsrWpaIe ) + sizeof( tCsrWpaAuthIe ) + sizeof( tANI_U16 ); // add in the size of the WPA IE that we may build.
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);         
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_REASSOC_REQ);
        pMsg->length = pal_cpu_to_be16(msgLen);
#if (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        pMsg->assocType = pal_cpu_to_be32(eSIR_NORMAL);
#endif
        if( csrIsProfileWpa( pProfile ) )
        {
            // Insert the Wpa IE into the join request
            pMsg->rsnIE.length = csrRetrieveWpaIe( pMac, pProfile, pBssDescription, pIes,
                                                  (tCsrWpaIe *)( pMsg->rsnIE.rsnIEdata ) );
        }
        else if( csrIsProfileRSN( pProfile ) )
        {
            // Insert the RSN IE into the join request
            pMsg->rsnIE.length = csrRetrieveRsnIe( pMac, pProfile, pBssDescription, pIes,
                                                  (tCsrRSNIe *)( pMsg->rsnIE.rsnIEdata ) );
        }
        else
        {
            pMsg->rsnIE.length = 0;
        }

        pBuf = &(pMsg->rsnIE.rsnIEdata[0]) + pMsg->rsnIE.length;

        //Unmask any AC in reassoc that is ACM-set
        uapsd_mask = (v_U8_t)pProfile->uapsd_mask;
        if( uapsd_mask && ( NULL != pBssDescription ) )
        {
            acm_mask = sme_QosGetACMMask(pMac, pBssDescription, pIes);
            uapsd_mask &= ~(acm_mask);
        }

        csrPropareJoinReassocReqBuffer( pMac, pBssDescription, pBuf, uapsd_mask);
        
        pMsg->rsnIE.length = pal_cpu_to_be16(pMsg->rsnIE.length);

        //Tush-QoS: notify QoS module that reassoc happening
        sme_QosCsrEventInd(pMac, SME_QOS_CSR_REASSOC_REQ, NULL);

        status = palSendMBMessage( pMac->hHdd, pMsg );
    } while( 0 );

    return( status );

}


//
eHalStatus csrSendMBDisassocReqMsg( tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U16 reasonCode )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeDisassocReq *pMsg;

    do {
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof( tSirSmeDisassocReq ));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeDisassocReq ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_DISASSOC_REQ);
		pMsg->reasonCode = pal_cpu_to_be16(reasonCode);
		pMsg->length = pal_cpu_to_be16((tANI_U16)sizeof( tSirSmeDisassocReq ));
		// Set the peer MAC address before sending the message to LIM
		status = palCopyMemory( pMac->hHdd, pMsg->peerMacAddr, bssId, sizeof( pMsg->peerMacAddr ) );
        if(!HAL_STATUS_SUCCESS(status))
        {
            palFreeMemory(pMac->hHdd, pMsg);
            break;
        }     

        status = palSendMBMessage( pMac->hHdd, pMsg );

    } while( 0 );

    return( status );
}


eHalStatus csrSendMBDeauthReqMsg( tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U16 reasonCode )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeDeauthReq *pMsg;

    do {
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof( tSirSmeDeauthReq ));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeDeauthReq ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_DEAUTH_REQ);
		pMsg->reasonCode = pal_cpu_to_be16(reasonCode);
		pMsg->length = pal_cpu_to_be16((tANI_U16)sizeof( tSirSmeDeauthReq ));
		// Set the peer MAC address before sending the message to LIM
		status = palCopyMemory( pMac->hHdd, pMsg->peerMacAddr, bssId, sizeof( pMsg->peerMacAddr ) );
        if(!HAL_STATUS_SUCCESS(status))
        {
            palFreeMemory(pMac->hHdd, pMsg);
            break;
        }     

        status = palSendMBMessage( pMac->hHdd, pMsg );

    } while( 0 );

    return( status );
}


eHalStatus csrSendMBDisassocCnfMsg( tpAniSirGlobal pMac, tpSirSmeDisassocInd pDisassocInd )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeDisassocCnf *pMsg;

    do {
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof( tSirSmeDisassocCnf ));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeDisassocCnf ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_DISASSOC_CNF);
		pMsg->statusCode = pal_cpu_to_be32(eSIR_SME_SUCCESS);
		pMsg->length = pal_cpu_to_be16((tANI_U16)sizeof( tSirSmeDisassocCnf ));
		status = palCopyMemory(pMac->hHdd, pMsg->peerMacAddr, pDisassocInd->peerMacAddr, sizeof(pMsg->peerMacAddr)); 
        if(!HAL_STATUS_SUCCESS(status))
        {
            palFreeMemory(pMac->hHdd, pMsg);
            break;
        }

        status = palSendMBMessage( pMac->hHdd, pMsg );

    } while( 0 );

    return( status );
}


eHalStatus csrSendMBDeauthCnfMsg( tpAniSirGlobal pMac, tpSirSmeDeauthInd pDeauthInd )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirSmeDeauthCnf *pMsg;

    do {
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof( tSirSmeDeauthCnf ));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeDeauthCnf ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_DEAUTH_CNF);
		pMsg->statusCode = pal_cpu_to_be32(eSIR_SME_SUCCESS);
		pMsg->length = pal_cpu_to_be16((tANI_U16)sizeof( tSirSmeDeauthCnf ));
		status = palCopyMemory(pMac->hHdd, pMsg->peerMacAddr, pDeauthInd->peerMacAddr, sizeof(pMsg->peerMacAddr)); 
        if(!HAL_STATUS_SUCCESS(status))
        {
            palFreeMemory(pMac->hHdd, pMsg);
            break;
        }

        status = palSendMBMessage( pMac->hHdd, pMsg );

    } while( 0 );

    return( status );
}




eHalStatus csrSendMBSetContextReqMsg( tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, 
                                    tANI_U8 numKeys, tAniEdType edType, tANI_BOOLEAN fUnicast, tAniKeyDirection aniKeyDirection,
                                    tANI_U8 keyId, tANI_U8 keyLength, tANI_U8 *pKey, tANI_U8 paeRole )
{
    tSirSmeSetContextReq *pMsg;
    tANI_U16 msgLen;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tAniEdType tmpEdType;
    tAniKeyDirection tmpDirection;

    tANI_U8 *p;

    do {

        if( ( 1 != numKeys ) && ( 0 != numKeys ) ) break;

        // all of these fields appear in every SET_CONTEXT message.  Below we'll add in the size for each 
        // key set. Since we only support upto one key, we always allocate memory for 1 key
        msgLen  = sizeof( pMsg->messageType ) + sizeof( pMsg->length ) + sizeof( pMsg->peerMacAddr ) +
                  sizeof( pMsg->keyMaterial.length ) + sizeof( pMsg->keyMaterial.edType ) + sizeof( pMsg->keyMaterial.numKeys ) +
                  ( sizeof( pMsg->keyMaterial.key ) );
                     
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, msgLen);
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_SETCONTEXT_REQ);
		pMsg->length = pal_cpu_to_be16(msgLen);

        //sirCopyMACAddr
        palCopyMemory( pMac->hHdd, (tANI_U8 *)pMsg->peerMacAddr, (tANI_U8 *)peerMacAddr, sizeof(tSirMacAddr) );

        p = pMsg->peerMacAddr + (sizeof( pMsg->peerMacAddr ) );

		// Set the pMsg->keyMaterial.length field (this length is defined as all data that follows the edType field
		// in the tSirKeyMaterial keyMaterial; field).
		//
		// !!NOTE:  This keyMaterial.length contains the length of a MAX size key, though the keyLength can be 
		// shorter than this max size.  Is LIM interpreting this ok ?
		p = pal_set_U16( p, pal_cpu_to_be16((tANI_U16)( sizeof( pMsg->keyMaterial.numKeys ) + ( numKeys * sizeof( pMsg->keyMaterial.key ) ) )) );

		// set pMsg->keyMaterial.edType
        tmpEdType = pal_cpu_to_be32(edType);
        palCopyMemory( pMac->hHdd, p, (tANI_U8 *)&tmpEdType, sizeof(tAniEdType) );
        p += sizeof( pMsg->keyMaterial.edType );

        // set the pMsg->keyMaterial.numKeys field
        *p = numKeys;
        p += sizeof( pMsg->keyMaterial.numKeys );   

        // set pSirKey->keyId = keyId;
        *p = keyId;
        p += sizeof( pMsg->keyMaterial.key[ 0 ].keyId );

        // set pSirKey->unicast = (tANI_U8)fUnicast;
        *p = (tANI_U8)fUnicast;
        p += sizeof( pMsg->keyMaterial.key[ 0 ].unicast );

		// set pSirKey->keyDirection = aniKeyDirection;
        tmpDirection = pal_cpu_to_be32(aniKeyDirection);
        palCopyMemory( pMac->hHdd, p, (tANI_U8 *)&tmpDirection, sizeof(tAniKeyDirection) );
        p += sizeof(tAniKeyDirection);
        //    pSirKey->keyRsc = ;;
        p += sizeof( pMsg->keyMaterial.key[ 0 ].keyRsc );

		// set pSirKey->paeRole
		*p = paeRole;   // 0 is Supplicant
		p++;

		// set pSirKey->keyLength = keyLength;
		p = pal_set_U16( p, pal_cpu_to_be16(keyLength) );

        if ( keyLength && pKey ) 
        {   
            palCopyMemory( pMac->hHdd, p, pKey, keyLength ); 
        }

        status = palSendMBMessage(pMac->hHdd, pMsg);

    } while( 0 );

    return( status );
}



eHalStatus csrSendMBStartBssReqMsg( tpAniSirGlobal pMac, eCsrRoamBssType bssType, tCsrRoamIbssParams *pParam )
{
    eHalStatus status;
	tSirSmeStartBssReq *pMsg;
	unsigned char *pBuf;
	tSirNwType nwType;
    tAniCBSecondaryMode cbMode;

	do {
        pMac->roam.joinFailStatusCode.statusCode = eSIR_SME_SUCCESS;
        pMac->roam.joinFailStatusCode.reasonCode = 0;
        status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tSirSmeStartBssReq));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeStartBssReq ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_START_BSS_REQ);
		pMsg->bssType = pal_cpu_to_be32(csrTranslateBsstypeToMacType(bssType));
	    pMsg->ssId = pParam->ssId;
		pBuf = &(pMsg->ssId.ssId[pMsg->ssId.length]);
		// set the channel Id
		*pBuf = pParam->operationChn;
		pBuf++;
        //What should we really do for the cbmode.
        cbMode = pal_cpu_to_be32(pParam->cbMode);
        palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *)&cbMode, sizeof(tAniCBSecondaryMode) );
        pBuf += sizeof(tAniCBSecondaryMode);
        // set RSN IE Length to zero
        palZeroMemory( pMac->hHdd, pBuf, sizeof(tANI_U16) );    //tSirRSNie->length
        pBuf += sizeof(tANI_U16);
		nwType = (tSirNwType)pal_cpu_to_be32(pParam->sirNwType);
        palCopyMemory( pMac->hHdd, pBuf, (tANI_U8 *)&nwType, sizeof(tSirNwType) );
        pBuf += sizeof(tSirNwType);
        //Prepare rate set as tSirMacRateSet
		*pBuf = pParam->sirRateSet.numRates; //tSirMacRateSet->numRates
        pBuf++;
		palCopyMemory( pMac->hHdd, pBuf, pParam->sirRateSet.rate, pParam->sirRateSet.numRates );
		pMsg->length = sizeof( pMsg->messageType ) + sizeof( pMsg->length ) + sizeof( pMsg->bssType ) +
		               sizeof( pMsg->ssId.length ) + pMsg->ssId.length + sizeof( pMsg->channelId ) + sizeof(tAniCBSecondaryMode) + 
		               sizeof( pMsg->rsnIE.length )  + /*pRSNIE->length +*/  //RSNIe length is 0
		               sizeof( pMsg->nwType ) + sizeof( pMsg->operationalRateSet.numRates ) +
		               ( pParam->sirRateSet.numRates * sizeof(tANI_U8) );
        pMsg->length = pal_cpu_to_be16(pMsg->length);
        status = palSendMBMessage(pMac->hHdd, pMsg);

    } while( 0 );

  return( status );
}


eHalStatus csrSendMBStopBssReqMsg( tpAniSirGlobal pMac, tANI_U16 reasonCode )
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tSirSmeStopBssReq *pMsg;

    do {
        status = palAllocateMemory(pMac, (void **)&pMsg, sizeof(tSirSmeStopBssReq));
        if ( !HAL_STATUS_SUCCESS(status) ) break;
        palZeroMemory(pMac->hHdd, pMsg, sizeof( tSirSmeStopBssReq ));
		pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_STOP_BSS_REQ);
		pMsg->reasonCode = pal_cpu_to_be32(reasonCode);
		pMsg->length = pal_cpu_to_be16((tANI_U16)sizeof( tSirSmeStopBssReq ));
		status = palSendMBMessage( pMac->hHdd, pMsg );
	} while( 0 );

    return( status );
}


eHalStatus csrReassoc(tpAniSirGlobal pMac, 
                      tCsrRoamModifyProfileFields *pModProfileFields,
                      tANI_U32 *pRoamId)
{

   eHalStatus status = eHAL_STATUS_FAILURE;
   tANI_U32 roamId = 0;
   tCsrRoamInfo roamInfo;
   tCsrRoamModifyProfileFields modProfileFields;

   if(pModProfileFields)
   {
       palCopyMemory(pMac->hHdd, &modProfileFields, pModProfileFields,
                     sizeof(tCsrRoamModifyProfileFields));
   }
   if((csrIsConnStateConnected(pMac)) &&
      (!palEqualMemory(pMac->hHdd, &modProfileFields, 
                       &pMac->roam.connectedProfile.modifyProfileFields, 
                       sizeof(tCsrRoamModifyProfileFields))))
   {
      roamId = GET_NEXT_ROAM_ID(&pMac->roam);
      if(pRoamId)
      {
         *pRoamId = roamId;
      }
      roamInfo.reasonCode = eCsrRoamReasonStaCapabilityChanged;
      csrRoamCallCallback(pMac, &roamInfo, 0, eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);
      pMac->roam.roamingReason = eCsrReassocRoaming;

      status = csrRoamIssueReassoc(pMac, NULL, &modProfileFields, 
                                   eCsrHddIssuedReassocToSameAP, roamId, 
                                   eANI_BOOLEAN_FALSE);

   }

   return status;
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
VOS_STATUS csrRoamTrafficIndCallback(tHalHandle hHal, 
                               WLANTL_HO_TRAFFIC_STATUS_TYPE trafficStatus, 
                               void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );

   if(pMac->roam.handoffInfo.isNrtTrafficOn != trafficStatus.nrtTrafficStatus)
   {
      smsLog(pMac, LOGW, "csrRoamTrafficIndCallback: NRT traffic status %d\n", trafficStatus.nrtTrafficStatus);
      if(WLANTL_HO_NRT_TRAFFIC_STATUS_ON == trafficStatus.nrtTrafficStatus)
      {
         csrRoamProcessNrtTrafficOnInd(pMac);
      }
      else
      {
         csrRoamProcessNrtTrafficOffInd(pMac);
      }
   }

   if(pMac->roam.handoffInfo.isRtTrafficOn != trafficStatus.rtTrafficStatus)
   {
      smsLog(pMac, LOGW, "csrRoamTrafficIndCallback: RT traffic status %d\n", trafficStatus.rtTrafficStatus);
      if(WLANTL_HO_RT_TRAFFIC_STATUS_ON == trafficStatus.rtTrafficStatus)
      {
         csrRoamProcessRtTrafficOnInd(pMac);
      }
      else
      {
         csrRoamProcessRtTrafficOffInd(pMac);
      }
   }

   return VOS_STATUS_SUCCESS;
}

static void csrRoamProcessNrtTrafficOnInd(tpAniSirGlobal pMac)
{
   /* cleanup, and move to desired state                                    */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   //clean up command q in case scan is pending
   csrScanRemoveBgScanReq(pMac);


   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
      ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamProcessNrtTrafficOnInd: wrong state %d\n", pMac->roam.handoffInfo.currState);
      //We might come here if control path & data path are out of sync. Typical
      //scenario is, user wanted to disassoc with the cuurent AP while we are in
      //the middle of data traffic. csr gets into disconnected state much before
      //TL gets the notifcation 
      //VOS_ASSERT( 0 );
      return;
   }

   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamProcessNrtTrafficOnInd: ignoring the indication as we are not connected\n");
      return;
   }
   /* Before moving to NRT sub-state, do consider any previous indication from 
      TL for real time traffic                              */

   pMac->roam.handoffInfo.isNrtTrafficOn = TRUE;

   if(pMac->roam.handoffInfo.isRtTrafficOn)
   {
      return;
   }

   csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC );
}


static void csrRoamProcessRtTrafficOnInd(tpAniSirGlobal pMac)
{
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   //clean up command q in case scan is pending
   csrScanRemoveBgScanReq(pMac);

   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamProcessRtTrafficOnInd: wrong state %d\n", pMac->roam.handoffInfo.currState);
      //We might come here if control path & data path are out of sync. Typical
      //scenario is, user wanted to disassoc with the cuurent AP while we are in
      //the middle of data traffic. csr gets into disconnected state much before
      //TL gets the notifcation 
      //VOS_ASSERT( 0 );
      return;
   }

   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamProcessRtTrafficOnInd: ignoring the indication as we are not connected\n");
      return;
   }

   /* cleanup, and move to RT sub-state                                    */
   pMac->roam.handoffInfo.isRtTrafficOn = TRUE;

   csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC );
  
}


static void csrRoamProcessNrtTrafficOffInd(tpAniSirGlobal pMac)
{
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamProcessNrtTrafficOffInd: wrong state %d\n", pMac->roam.handoffInfo.currState);
      //We might come here if control path & data path are out of sync. Typical
      //scenario is, user wanted to disassoc with the cuurent AP while we are in
      //the middle of data traffic. csr gets into disconnected state much before
      //TL gets the notifcation 
      //VOS_ASSERT( 0 );
      return;
   }
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamProcessNrtTrafficOffInd: ignoring the indication as we are not connected\n");
      return;
   }

   pMac->roam.handoffInfo.isNrtTrafficOn = FALSE;

   if(pMac->roam.handoffInfo.isRtTrafficOn)
   {
      return;
   }

   /* cleanup, and move to NT sub-state                                    */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);

   csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC );

}


static void csrRoamProcessRtTrafficOffInd(tpAniSirGlobal pMac)
{
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamProcessRtTrafficOffInd: wrong state %d\n", pMac->roam.handoffInfo.currState);
      //We might come here if control path & data path are out of sync. Typical
      //scenario is, user wanted to disassoc with the cuurent AP while we are in
      //the middle of data traffic. csr gets into disconnected state much before
      //TL gets the notifcation 
      //VOS_ASSERT( 0 );
      return;
   }
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamProcessRtTrafficOffInd: ignoring the indication as we are not connected\n");
      return;
   }

   pMac->roam.handoffInfo.isRtTrafficOn = FALSE;

   /* cleanup, and move to appropriate sub-state                             */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);

   if(pMac->roam.handoffInfo.isNrtTrafficOn)
   {
      csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC );
   }
   else
   {
      csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC );
   }
  
}
#endif

static void csrRoamLinkUp(tpAniSirGlobal pMac, tCsrBssid bssid)
{
#ifdef FEATURE_WLAN_GEN6_ROAMING
   tCsrHandoffStaInfo *pStaEntry = NULL;
   VOS_STATUS status;
   tANI_BOOLEAN found = FALSE;
#endif
   /* Update the current BSS info in ho control block based on connected 
      profile info from pmac global structure                              */
   

   smsLog(pMac, LOGW, " csrRoamLinkUp: WLAN link UP with AP= %02x-%02x-%02x-%02x-%02x-%02x\n", 
          bssid[ 0 ], bssid[ 1 ], bssid[ 2 ],
          bssid[ 3 ], bssid[ 4 ], bssid[ 5 ] );

#ifdef FEATURE_WLAN_GEN6_ROAMING
   if( 0 == csrScanFindBssEntryFromList( pMac,
                                         &pMac->roam.handoffInfo.neighborList,
                                         bssid,
                                         &pStaEntry))
   {
      /* Need to remove new curr_sta from neighbor list                      */
      csrScanRemoveEntryFromList( pMac,
                                 &pMac->roam.handoffInfo.neighborList, 
                                 pStaEntry->sta.bssid);
      found = TRUE;

   }
   if( 0 == csrScanFindBssEntryFromList( pMac,
                                              &pMac->roam.handoffInfo.candidateList,
                                              bssid,
                                              &pStaEntry))
   {
      /* Need to remove new curr_sta from candidate list                      */
      csrScanRemoveEntryFromList( pMac,
                                  &pMac->roam.handoffInfo.candidateList, 
                                  pStaEntry->sta.bssid);
      found = TRUE;
   }
   if(FALSE == found)
   {
      smsLog(pMac, LOGW, " csrRoamLinkUp: Acquired an AP not in neighbor/candidate list!\n");

      //commenting out the assert, in case of reassoc it will be a valid scenario
      //it's work around, but need to fix it properly - TBH 
      //VOS_ASSERT( 0 );

   }

   /* Update the handoff config information if needed                        */
   if(!HAL_STATUS_SUCCESS(csrUpdateHandoffParams(pMac)))
   {
      smsLog(pMac, LOGW, " csrRoamLinkUp: couldn't update params\n");
   }
#endif
   /* Check for user misconfig of RSSI trigger threshold                  */
   pMac->roam.configParam.vccRssiThreshold =
      ( 0 == pMac->roam.configParam.vccRssiThreshold ) ? 
      CSR_VCC_RSSI_THRESHOLD : pMac->roam.configParam.vccRssiThreshold;
   pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;

    /* Check for user misconfig of UL MAC Loss trigger threshold           */
   pMac->roam.configParam.vccUlMacLossThreshold =
      ( 0 == pMac->roam.configParam.vccUlMacLossThreshold ) ? 
      CSR_VCC_UL_MAC_LOSS_THRESHOLD : pMac->roam.configParam.vccUlMacLossThreshold;

#ifdef FEATURE_WLAN_GEN6_ROAMING
   //regirster the vcc rssi trigger with TL
   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.configParam.vccRssiThreshold * (-1),
                                 WLANTL_HO_THRESHOLD_CROSS, 
                                 csrRoamVccTriggerRssiIndCallback, 
               VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      //err msg
      smsLog(pMac, LOGW, " csrRoamLinkUp: couldn't register csrRoamVccTriggerRssiIndCallback with TL\n");
   }

   if(pMac->roam.handoffInfo.handoffAction)
   {
      pMac->roam.handoffInfo.handoffAction = FALSE;
      /* This is an extra transition to reset all timers, values etc for new AP   */

      csrRoamSubstateChange( pMac, pMac->roam.handoffInfo.subStateBeforeHandoff);
   }
   else
   {
      //register traffic ind callback with TL
      status = 
         WLANTL_RegGetTrafficStatus(pMac->roam.gVosContext,  
                                    pMac->roam.handoffInfo.handoffParams.ntParams.inactThreshold,
                                    pMac->roam.handoffInfo.handoffParams.ntParams.inactPeriod, 
                                    csrRoamTrafficIndCallback,
                                    pMac);

      if(!VOS_IS_STATUS_SUCCESS( status))
      {
         //err msg
         smsLog(pMac, LOGW, " csrRoamLinkUp: couldn't register traffic ind callback with TL\n");
         return;
      }

      csrRoamSubstateChange( pMac, eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC );
      
   }
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   csrRoamStartDiagLogStatsTimer(pMac, CSR_DIAG_LOG_STAT_PERIOD);
#endif

}


static void csrRoamLinkDown(tpAniSirGlobal pMac)
{
#ifdef FEATURE_WLAN_GEN6_ROAMING
   VOS_STATUS status;
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   csrRoamStopDiagLogStatsTimer(pMac);
#endif

#ifdef FEATURE_WLAN_GEN6_ROAMING
   /* Clean up, stop timers (bg scan, stats),                              */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   //clear up pending scan req
   csrScanRemoveBgScanReq(pMac);
#endif
   /* deregister the clients requesting stats from PE/TL & also stop the corresponding timers*/
   csrRoamDeregStatisticsReq(pMac);
   pMac->roam.vccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
#ifdef FEATURE_WLAN_GEN6_ROAMING   
   if(!pMac->roam.handoffInfo.handoffAction)
   {

	  pMac->roam.handoffInfo.subStateBeforeHandoff = pMac->roam.handoffInfo.currSubState;
      //cleanup the neighbor & candidate list when connection is lost, to avaid 
      //having stale entries
      csrRoamHandoffRemoveAllFromList(pMac, &pMac->roam.handoffInfo.candidateList);
      csrRoamHandoffRemoveAllFromList(pMac, &pMac->roam.handoffInfo.neighborList);
   }
   
   //deregirster the vcc rssi trigger with TL
   status = 
         WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.configParam.vccRssiThreshold * (-1), 
                                      WLANTL_HO_THRESHOLD_CROSS, csrRoamVccTriggerRssiIndCallback, 
									  VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamLinkUp: couldn't deregister csrRoamVccTriggerRssiIndCallback with TL\n");
      return;
   }
#endif
}


void csrRoamTlStatsTimerHandler(void *pv)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( pv );
   WLANTL_TRANSFER_STA_TYPE tlStats;
   eHalStatus status;

   pMac->roam.tlStatsReqInfo.timerRunning = FALSE;
   
   //req TL for stats
   if(WLANTL_GetStatistics(pMac->roam.gVosContext, &tlStats, pMac->roam.connectedInfo.staId))
   {
      smsLog(pMac, LOGE, FL("csrRoamTlStatsTimerHandler:couldn't get the stats from TL\n"));
   }
   else
   {
      //save in SME
      csrRoamSaveStatsFromTl(pMac, tlStats);
   }

   if(!pMac->roam.tlStatsReqInfo.timerRunning)
   {
      if(pMac->roam.tlStatsReqInfo.periodicity)
      {
         //start timer
         status = palTimerStart(pMac->hHdd, pMac->roam.tlStatsReqInfo.hTlStatsTimer, 
                                pMac->roam.tlStatsReqInfo.periodicity * PAL_TIMER_TO_MS_UNIT, eANI_BOOLEAN_FALSE);
         if(!HAL_STATUS_SUCCESS(status))
         {
            smsLog(pMac, LOGE, FL("csrRoamTlStatsTimerHandler:cannot start TlStatsTimer timer\n"));
            return;
         }
         pMac->roam.tlStatsReqInfo.timerRunning = TRUE;
      }
   }
}

void csrRoamPeStatsTimerHandler(void *pv)
{
   tCsrPeStatsReqInfo *pPeStatsReqListEntry = (tCsrPeStatsReqInfo *)pv;
   eHalStatus status;
   tpAniSirGlobal         pMac = pPeStatsReqListEntry->pMac;
   VOS_STATUS vosStatus;
   tPmcPowerState powerState;

   pPeStatsReqListEntry->timerRunning = FALSE;
   if(!pPeStatsReqListEntry->rspPending)
   {
      status = csrSendMBStatsReqMsg(pMac, pPeStatsReqListEntry->statsMask & ~(1 << eCsrGlobalClassDStats), 
                                    pPeStatsReqListEntry->staId);
      if(!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGE, FL("csrRoamPeStatsTimerHandler:failed to send down stats req to PE\n"));
      }
      else
      {
         pPeStatsReqListEntry->rspPending = TRUE;
      }
   }
   if(!pPeStatsReqListEntry->timerRunning)
   {
      //send down a req
      if(pPeStatsReqListEntry->periodicity && 
         (VOS_TIMER_STATE_STOPPED == vos_timer_getCurrentState(&pPeStatsReqListEntry->hPeStatsTimer)))
      {
         pmcQueryPowerState(pMac, &powerState, NULL, NULL);
         if(ePMC_FULL_POWER == powerState)
         {
            if(pPeStatsReqListEntry->periodicity < pMac->roam.configParam.statsReqPeriodicity)
            {
               pPeStatsReqListEntry->periodicity = pMac->roam.configParam.statsReqPeriodicity;
            }
         }
         else
         {
            if(pPeStatsReqListEntry->periodicity < pMac->roam.configParam.statsReqPeriodicityInPS)
            {
               pPeStatsReqListEntry->periodicity = pMac->roam.configParam.statsReqPeriodicityInPS;
            }
         }
         //start timer
         vosStatus = vos_timer_start( &pPeStatsReqListEntry->hPeStatsTimer, pPeStatsReqListEntry->periodicity );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) 
         {
            smsLog(pMac, LOGE, FL("csrRoamPeStatsTimerHandler:cannot start hPeStatsTimer timer\n"));
            return;
         }

         pPeStatsReqListEntry->timerRunning = TRUE;
      }
   }

}

void csrRoamStatsClientTimerHandler(void *pv)
{
   tCsrStatsClientReqInfo *pStaEntry = (tCsrStatsClientReqInfo *)pv;
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

   if(VOS_TIMER_STATE_STOPPED == vos_timer_getCurrentState(&pStaEntry->timer))
   {
   //start the timer
   vosStatus = vos_timer_start( &pStaEntry->timer, pStaEntry->periodicity );
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) 
   {
      smsLog(pStaEntry->pMac, LOGE, FL("csrGetStatistics:cannot start StatsClient timer\n"));
   }
   }
   //send up the stats report
   csrRoamReportStatistics(pStaEntry->pMac, pStaEntry->statsMask, pStaEntry->callback, 
                           pStaEntry->staId, pStaEntry->pContext);

}
#ifdef FEATURE_WLAN_GEN6_ROAMING
eHalStatus csrRoamStartStatisticsTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
   eHalStatus status;
   tANI_BOOLEAN found = FALSE;
   tCsrPeStatsReqInfo *pPeStaEntry = NULL; 
   smsLog(pMac, LOG1, " csrRoamStartStatisticsTimer \n ");
   do
   {
   
      if(( pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_JOINED ||
		   pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_SCANNING) && interval)
      {
         status = palTimerStart(pMac->hHdd, pMac->roam.hTimerStatistics, interval * PAL_TIMER_TO_MS_UNIT, eANI_BOOLEAN_FALSE);
      }
      else
      {
         smsLog(pMac, LOG1, " csrRoamStartStatisticsTimer: failed to start \n ");
         status = eHAL_STATUS_FAILURE;
         break;
      }
      pPeStaEntry = csrRoamCheckPeStatsReqList(pMac, 1 << eCsrSummaryStats, 
                                               interval, &found, pMac->roam.connectedInfo.staId);
      if(!pPeStaEntry)
      {
         //bail out
         status = eHAL_STATUS_FAILURE;
         break;
      }


   }while(0);

   return (status);
}


eHalStatus csrRoamStopStatisticsTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->roam.hTimerStatistics));
}

void csrRoamStatisticsTimerHandler(void *pv)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( pv );
   /* Extra check on states to see if this is really needed                 */
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamStatisticsTimerHandler: wrong state %d\n", pMac->roam.handoffInfo.currState);

      return;
   }
   //use stats for handoff/Vcc
   csrRoamHandoffStatsProcessor(pMac);

}

static tANI_U32 csrRoamGetStatsTimerVal(tpAniSirGlobal pMac)
{
   tANI_U32 statsTime = 0;
   /* This function returns the timer value to be used for a given state
      (NRT and RT)                                                       */
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamGetStatsTimerVal: wrong state %d\n", pMac->roam.handoffInfo.currState);
      return 0;
   }


   switch( pMac->roam.handoffInfo.currSubState )
   {
     case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:
        statsTime = 0;
        break;
     case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
        statsTime = pMac->roam.handoffInfo.handoffParams.nrtParams.perMsmtInterval;
        break;
     case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
        statsTime = pMac->roam.handoffInfo.handoffParams.rtParams.perMsmtInterval;
        break;

     default:
        smsLog(pMac, LOGW, " csrRoamGetStatsTimerVal : Invalid current sub-state %d\n", pMac->roam.handoffInfo.currSubState);
        statsTime = 0;

   } /* end switch */


   return statsTime;

}
#endif

eHalStatus csrSendMBStatsReqMsg( tpAniSirGlobal pMac, tANI_U32 statsMask, tANI_U8 staId)
{
   tAniGetPEStatsReq *pMsg;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   status = palAllocateMemory(pMac->hHdd, (void **)&pMsg, sizeof(tAniGetPEStatsReq));
   if ( !HAL_STATUS_SUCCESS(status) ) 
   {
      smsLog(pMac, LOG1, " csrSendMBStatsReqMsg: failed to allocate mem for stats req \n");
      return status;
   }
   // need to initiate a stats request to PE
   pMsg->msgType = pal_cpu_to_be16((tANI_U16)eWNI_SME_GET_STATISTICS_REQ);
   pMsg->msgLen = (tANI_U16)sizeof(tAniGetPEStatsReq);
   pMsg->staId = staId;
   pMsg->statsMask = statsMask;

   status = palSendMBMessage(pMac->hHdd, pMsg );    

   if(!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOG1, " csrSendMBStatsReqMsg: failed to send down the stats req \n");
   }

   return status;
}


void csrRoamStatsRspProcessor(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg)
{
   tAniGetPEStatsRsp *pSmeStatsRsp;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tListElem *pEntry = NULL;
   tCsrStatsClientReqInfo *pTempStaEntry = NULL;
   tCsrPeStatsReqInfo *pPeStaEntry = NULL;
   tANI_U32  tempMask = 0;
   tANI_U8 counter = 0;
   tANI_U8 *pStats = NULL;

   pSmeStatsRsp = (tAniGetPEStatsRsp *)pSirMsg;
   if(pSmeStatsRsp->rc)
   {
      smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:stats rsp from PE shows failure\n"));
      return;
   }

   tempMask = pSmeStatsRsp->statsMask;
   pStats = ((tANI_U8 *)&pSmeStatsRsp->statsMask) + sizeof(pSmeStatsRsp->statsMask);

   if(!pStats)
   {
      smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:empty stats buffer from PE\n"));
      return;
   }

   //new stats info from PE, fill up the stats strucutres in PMAC
   while(tempMask)
   {
      if(tempMask & 1)
      {
         switch(counter)
         {
         case eCsrSummaryStats:
            smsLog( pMac, LOG1, FL("csrRoamStatsRspProcessor:summary stats\n"));
            status = palCopyMemory(pMac->hHdd, (tANI_U8 *)&pMac->roam.summaryStatsInfo, 
                                   pStats, sizeof(tCsrSummaryStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:failed to copy summary stats\n"));
            }
            pStats += sizeof(tCsrSummaryStatsInfo);
            break;
         case eCsrGlobalClassAStats:
            smsLog( pMac, LOG1, FL("csrRoamStatsRspProcessor:ClassA stats\n"));
            status = palCopyMemory(pMac->hHdd, (tANI_U8 *)&pMac->roam.classAStatsInfo, 
                                   pStats, sizeof(tCsrGlobalClassAStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:failed to copy ClassA stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassAStatsInfo);

            break;

         case eCsrGlobalClassBStats:
            smsLog( pMac, LOG1, FL("csrRoamStatsRspProcessor:ClassB stats\n"));
            status = palCopyMemory(pMac->hHdd, (tANI_U8 *)&pMac->roam.classBStatsInfo, 
                                   pStats, sizeof(tCsrGlobalClassBStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:failed to copy ClassB stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassBStatsInfo);

            break;

         case eCsrGlobalClassCStats:
            smsLog( pMac, LOG1, FL("csrRoamStatsRspProcessor:ClassC stats\n"));
            status = palCopyMemory(pMac->hHdd, (tANI_U8 *)&pMac->roam.classCStatsInfo, 
                                   pStats, sizeof(tCsrGlobalClassCStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:failed to copy ClassC stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassCStatsInfo);

            break;

         case eCsrPerStaStats:
            smsLog( pMac, LOG1, FL("csrRoamStatsRspProcessor:PerSta stats\n"));
            status = palCopyMemory(pMac->hHdd, (tANI_U8 *)&pMac->roam.perStaStatsInfo[pSmeStatsRsp->staId], 
                                   pStats, sizeof(tCsrPerStaStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:failed to copy PerSta stats\n"));
            }
            pStats += sizeof(tCsrPerStaStatsInfo);

            break;
         default:
            smsLog( pMac, LOGW, FL("csrRoamStatsRspProcessor:unknown stats type\n"));
            break;

         }
      }

      tempMask >>=1;
      counter++;
   }
   //make sure to update the pe stats req list 
   pEntry = csrRoamFindInPeStatsReqList(pMac, pSmeStatsRsp->statsMask);
   if(pEntry)
      {
      pPeStaEntry = GET_BASE_ADDR( pEntry, tCsrPeStatsReqInfo, link );
      pPeStaEntry->rspPending = FALSE;
   
   }
   //check the one timer cases
   pEntry = csrRoamCheckClientReqList(pMac, pSmeStatsRsp->statsMask);
   if(pEntry)
   {

      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrStatsClientReqInfo, link );

      if(pTempStaEntry->timerExpired)
      {
         //send up the stats report
         csrRoamReportStatistics(pMac, pTempStaEntry->statsMask, pTempStaEntry->callback, 
                                 pTempStaEntry->staId, pTempStaEntry->pContext);
         //also remove from the client list
         csrLLRemoveEntry(&pMac->roam.statsClientReqList, pEntry, LL_ACCESS_LOCK);
         pTempStaEntry = NULL;

      }
   }

}

tListElem * csrRoamFindInPeStatsReqList(tpAniSirGlobal pMac, tANI_U32  statsMask)
{
   tListElem *pEntry = NULL;
   tCsrPeStatsReqInfo *pTempStaEntry = NULL;

   pEntry = csrLLPeekHead( &pMac->roam.peStatsReqList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamFindInPeStatsReqList: List empty, no request to PE\n");
      return NULL;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrPeStatsReqInfo, link );

      if(pTempStaEntry->statsMask == statsMask)
      {
         smsLog(pMac, LOGW, "csrRoamFindInPeStatsReqList: match found\n");
         break;
      }

      pEntry = csrLLNext( &pMac->roam.peStatsReqList, pEntry, LL_ACCESS_NOLOCK );
   }

   return pEntry;
}


tListElem * csrRoamChecknUpdateClientReqList(tpAniSirGlobal pMac, tCsrStatsClientReqInfo *pStaEntry,
                                             tANI_BOOLEAN update)
{
   tListElem *pEntry;
   tCsrStatsClientReqInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( &pMac->roam.statsClientReqList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamChecknUpdateClientReqList: List empty, no request from \
             upper layer client(s)\n");
      return NULL;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrStatsClientReqInfo, link );

      if((pTempStaEntry->requesterId == pStaEntry->requesterId) && 
         (pTempStaEntry->statsMask == pStaEntry->statsMask))
      {
         smsLog(pMac, LOGW, "csrRoamChecknUpdateClientReqList: match found\n");
         if(update)
         {
         pTempStaEntry->periodicity = pStaEntry->periodicity;
         pTempStaEntry->callback = pStaEntry->callback;
         pTempStaEntry->pContext = pStaEntry->pContext;
         }
         break;
      }

      pEntry = csrLLNext( &pMac->roam.statsClientReqList, pEntry, LL_ACCESS_NOLOCK );
   }

   return pEntry;
}

tListElem * csrRoamCheckClientReqList(tpAniSirGlobal pMac, tANI_U32 statsMask)
{
   tListElem *pEntry;
   tCsrStatsClientReqInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( &pMac->roam.statsClientReqList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamCheckClientReqList: List empty, no request from \
             upper layer client(s)\n");
      return NULL;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrStatsClientReqInfo, link );

      if((pTempStaEntry->statsMask & ~(1 << eCsrGlobalClassDStats))  == statsMask)
      {
         smsLog(pMac, LOGW, "csrRoamCheckClientReqList: match found\n");

         break;
      }

      pEntry = csrLLNext( &pMac->roam.statsClientReqList, pEntry, LL_ACCESS_NOLOCK );
   }

   return pEntry;
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
void csrRoamHandoffStatsProcessor(tpAniSirGlobal pMac)
{
   tCsrBssid      *pBssid;
   tANI_U32        stats_time;
   /* Extra check on states to see if this is really needed                 */
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamHandoffStatsProcessor: wrong state %d\n", pMac->roam.handoffInfo.currState);
      return;
   }

   csrRoamVccTrigger(pMac);
   /* FDD suggests no filter on PER: overwrite existing values              */
   csrRoamUpdatePER(pMac);
   
   /* Check exit and entry criteria to see if we need a handoff...          */
   pBssid = csrRoamGetStatHoCandidate(pMac);

   if( pBssid)   
   {
      /* set to the probed BSSID                                              */
      palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                    pBssid, WNI_CFG_BSSID_LEN);
      csrScanSendBgProbeReq(pMac, pBssid);
      smsLog(pMac, LOGW, " csrRoamHandoffStatsProcessor : Exit criteria met: probing top candidate\n");
   }
   else
   {
      smsLog(pMac, LOGW, " csrRoamHandoffStatsProcessor : Exit criteria met but no candidates\n");
   }

   /* Find out what new time to use next                                    */
   stats_time = csrRoamGetStatsTimerVal(pMac);

   csrRoamStartStatisticsTimer(pMac, stats_time);
}


tCsrBssid * csrRoamGetStatHoCandidate(tpAniSirGlobal pMac)
{
   tCsrBssid *         pBssid = NULL;

   /* If a probe is already in progress then no need to do anything further */
   if( pMac->roam.handoffInfo.isProbeRspPending )
   {
     return NULL;
   }

   /* Check exit and entry criteria to see if we need a handoff...          */

   if( TRUE == csrRoamShouldExitAp(pMac) )
   {
      smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate : Exit criteria met for handoff\n");
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


     pBssid = csrScanGetHoCandidate(pMac);

     if(pBssid)
     {
        smsLog(pMac, LOGW, " csrRoamGetStatHoCandidate: Entry criteria met for handoff. BSSID= %02x-%02x-%02x-%02x-%02x-%02x\n", 
               pBssid[ 0 ], pBssid[ 1 ], pBssid[ 2 ], pBssid[ 3 ], pBssid[ 4 ], pBssid[ 5 ] );
     }
   }

   return pBssid;


}

tANI_BOOLEAN csrRoamShouldExitAp(tpAniSirGlobal pMac)
{
   tANI_BOOLEAN shouldExitAp = FALSE;

   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamShouldExitAp: wrong state %d\n", pMac->roam.handoffInfo.currState);
      return FALSE;
   }

   //The rssi exit criteria is handled by TL
   switch( pMac->roam.handoffInfo.currSubState )
   {
     case eCSR_ROAM_SUBSTATE_JOINED_NO_TRAFFIC:

        smsLog(pMac, LOGW, "Exit criteria are always met for NT\n");
        shouldExitAp = TRUE;
        break;

     case eCSR_ROAM_SUBSTATE_JOINED_NON_REALTIME_TRAFFIC:
        if(pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet ||
           (pMac->roam.handoffInfo.currSta.txPer >
            pMac->roam.handoffInfo.handoffParams.nrtParams.perThresholdHoFromCurrentAp) )
       {
         shouldExitAp = TRUE;
       }
       break;

     case eCSR_ROAM_SUBSTATE_JOINED_REALTIME_TRAFFIC:
        if( pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet ||
            (pMac->roam.handoffInfo.currSta.txPer >
             pMac->roam.handoffInfo.handoffParams.rtParams.perThresholdHoFromCurrentAp) ||
            (pMac->roam.handoffInfo.currSta.rxPer >
             pMac->roam.handoffInfo.handoffParams.rtParams.perThresholdHoFromCurrentAp) )
       {
         shouldExitAp = TRUE;
       }
       break;

     default:
        smsLog(pMac, LOGW, "Invalid current sub-state: %d", pMac->roam.handoffInfo.currSubState );

   } /* end switch */

   return shouldExitAp;

}


static void csrRoamUpdatePER(tpAniSirGlobal pMac)
{
   //update the TX/RX PER
   tANI_U32                           tx_per, rx_per;
   tANI_U32                           tx_fail_cnt_delta, tx_frm_cnt_delta;
   tANI_U32                           rx_crc_err_delta, rx_crc_ok_delta;
   tANI_U32                           sum_tx_frm_cnt = 0;
   tANI_U32                           sum_tx_fail_cnt = 0;
   tANI_U8                            index;
 /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   //sum up the per AC counter
   for(index = 0; index < 4; index++)
   {
      sum_tx_frm_cnt +=pMac->roam.summaryStatsInfo.tx_frm_cnt[index];
   }
   for(index = 0; index < 4; index++)
   {
      sum_tx_fail_cnt +=pMac->roam.summaryStatsInfo.fail_cnt[index];
   }

   /* preserve previous values to do a delta                                */
   if( pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt > 
       sum_tx_fail_cnt )
   {
      tx_fail_cnt_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt + 
         sum_tx_fail_cnt;
   }
   else
   {
      tx_fail_cnt_delta = sum_tx_fail_cnt - 
         pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt;
   }

   if( pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt > 
       sum_tx_frm_cnt )
   {
      tx_frm_cnt_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt + 
         sum_tx_frm_cnt;
   }
   else
   {
      tx_frm_cnt_delta = sum_tx_frm_cnt - 
         pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt;
   }

   /* check for zero denominator                                            */
   if( tx_frm_cnt_delta + tx_fail_cnt_delta )
   {
      /* UL MAC Loss is represented in %                                     */
      tx_per = 100 * tx_fail_cnt_delta/(tx_frm_cnt_delta+tx_fail_cnt_delta);
   }
   else
   {
      tx_per = 0;
   }    

   smsLog(pMac, LOGW, "Updating Tx PER in sub-state: %d from %d to %d",
          pMac->roam.handoffInfo.currSubState,
          pMac->roam.handoffInfo.currSta.txPer,
          tx_per);
   
   /* preserve previous values to do a delta                                */
   if( pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_err > 
       pMac->roam.summaryStatsInfo.rx_error_cnt )
   {
      rx_crc_err_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_err + 
         pMac->roam.summaryStatsInfo.rx_error_cnt;
   }
   else
   {
      rx_crc_err_delta = pMac->roam.summaryStatsInfo.rx_error_cnt - 
         pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_err;
   }
   
   if( pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_ok > 
       pMac->roam.summaryStatsInfo.rx_frm_cnt )
   {
      rx_crc_ok_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_ok + 
         pMac->roam.summaryStatsInfo.rx_frm_cnt;
   }
   else
   {
      rx_crc_ok_delta = pMac->roam.summaryStatsInfo.rx_frm_cnt - 
         pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_ok;
   }

   /* check for zero denominator                                            */
   if( rx_crc_ok_delta + rx_crc_err_delta )
   {
      /* UL MAC Loss is represented in %                                     */
      rx_per = 100 * rx_crc_err_delta/(rx_crc_ok_delta+rx_crc_err_delta);
   }
   else
   {
      rx_per = 0;
   }    

   smsLog(pMac, LOGW, "Updating Rx PER in sub-state: %d from %d to %d",
          pMac->roam.handoffInfo.currSubState,
          pMac->roam.handoffInfo.currSta.rxPer,
          rx_per);

   /* Update stats for future use                                           */
   //disabling stats based handoff for now
   pMac->roam.handoffInfo.currSta.txPer = tx_per;
   pMac->roam.handoffInfo.currSta.rxPer = 0;
   /* collect the new statistics for the current AP                   */
   pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt = sum_tx_frm_cnt;
   pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt = sum_tx_fail_cnt;
   pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_ok = pMac->roam.summaryStatsInfo.rx_frm_cnt;
   pMac->roam.handoffInfo.currStatsInfo.num_rx_frm_crc_err = pMac->roam.summaryStatsInfo.rx_error_cnt;
}

static void csrRoamNtPreTransHandler(tpAniSirGlobal pMac)
{
   VOS_STATUS status;
   /* This function handles common processing required when moving out of NT */
   /* Clear all timers                                                      */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   //deregister rssi ind callback
   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffActivityInfo.currNtRssiThreshold * (-1), 
                                   WLANTL_HO_THRESHOLD_DOWN,
								   csrRoamNtRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      //err msg
      smsLog(pMac, LOGW, " csrRoamNtPreTransHandler: couldn't deregister csrRoamNtRssiIndCallback with TL\n");
   }

   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCurrentApGood * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
                                   csrRoamNtBgScanRssiIndCallback, 
                                   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNtPreTransHandler: couldn't deregister csrRoamNtBgScanRssiIndCallback with TL\n");
   }

   /* Check if scan rsp is still awaited and set flag accordingly           */
   if( TRUE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /* additional flag to show that further scans cannot be initiated      */
      pMac->roam.handoffInfo.ignoreScanFrmOthrState = TRUE;
   }

     /* Set handoff parameters for this state                                 */
   pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex = 0;
   pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex = 0;
   pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currPermittedNumCandtSetEntry = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currPmkCacheRssiDelta = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta = 0;

}

static void csrRoamNtPostTransHandler(tpAniSirGlobal pMac)
{
   tANI_U32 scan_time;
   VOS_STATUS status;
   //reset the channel mask for HDD
   if(csrScanGetChannelMask(pMac))
   {
       csrScanAbortMacScan(pMac);
       palZeroMemory(pMac->hHdd, &pMac->scan.osScanChannelMask, sizeof(tCsrOsChannelMask));
   }

   /* This function handles common processing required when moving into NT  */
   /* Set handoff parameters for this state                                 */
   pMac->roam.handoffInfo.handoffActivityInfo.candChanListIndex = 0;
   pMac->roam.handoffInfo.handoffActivityInfo.otherChanListIndex = 0;
   pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;

   palZeroMemory(pMac->hHdd, &pMac->roam.handoffInfo.currStatsInfo, sizeof(tCsrCurrStatsInfo));


   pMac->roam.handoffInfo.handoffActivityInfo.currPermittedNumCandtSetEntry = 
      pMac->roam.handoffInfo.handoffParams.ntParams.numCandtSetEntry;

   pMac->roam.handoffInfo.handoffActivityInfo.currPmkCacheRssiDelta = 
      pMac->roam.handoffInfo.handoffParams.ntParams.pmkCacheRssiDelta;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant = 
      pMac->roam.handoffInfo.handoffParams.ntParams.rssiFilterConst;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet = 
      pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCandtSet;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta = 
      pMac->roam.handoffInfo.handoffParams.ntParams.bestCandidateApRssiDelta;

   status = WLANTL_SetAlpha(pMac->roam.gVosContext, 
                            (tANI_U8)pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant/10);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNtPostTransHandler: couldn't register csrRoamNrtExitCriteriaRssiIndCallback with TL\n");
   }
   //register the rssi indications callback with TL
   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.ntParams.rssiThresholdCurrentApGood * (-1),
                                 WLANTL_HO_THRESHOLD_CROSS, 
                                 csrRoamNtBgScanRssiIndCallback, 
                                 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNtPostTransHandler: couldn't register csrRoamNtBgScanRssiIndCallback with TL\n");
      return;
   }


   if( FALSE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
     /*-----------------------------------------------------------------------
       Check if bg scan needs to be started for this state
     -----------------------------------------------------------------------*/
     if( FALSE == csrScanIsBgScanEnabled(pMac) )
     {
       return;
     }

     /*-----------------------------------------------------------------------
       If bg scan required, then start the scan timer
       Call scan timer generator
     -----------------------------------------------------------------------*/
     scan_time = csrScanGetBgScanTimerVal(pMac);

     if(scan_time )
     {
        //start the bg scan timer
        csrScanStartBgScanTimer(pMac, scan_time);

        pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;
     }
   }
   
}

static void csrRoamNrtPreTransHandler(tpAniSirGlobal pMac)
{
   VOS_STATUS status;

   /* This function handles common processing required when moving out of NRT */
   /* Clear all timers                                                      */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   pMac->roam.handoffInfo.handoffActivityInfo.isNrtBgScanEmptyCandSetPermitted = FALSE;
   pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = FALSE;

   /* Check if scan rsp is still awaited and set flag accordingly           */
   if( TRUE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /* additional flag to show that further scans cannot be initiated      */
      pMac->roam.handoffInfo.ignoreScanFrmOthrState = TRUE;
   }
   //deregister the rssi indications callback with TL
   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdHoFromCurrentAp * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
								   csrRoamNrtExitCriteriaRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPreTransHandler: couldn't deregister csrRoamNrtExitCriteriaRssiIndCallback with TL\n");
      return;
   }

   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGood * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
								   csrRoamNrtBgScanRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPreTransHandler: couldn't deregister csrRoamNrtBgScanRssiIndCallback with TL\n");
      return;
   }


   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGoodEmptyCandtset * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
								   csrRoamNrtBgScanEmptyCandSetRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPreTransHandler: couldn't deregister csrRoamNrtBgScanEmptyCandSetRssiIndCallback with TL\n");
      return;
   }

}

static void csrRoamNrtPostTransHandler(tpAniSirGlobal pMac)
{

   tANI_U32 stats_time;
   VOS_STATUS status;

   /* This function handles common processing required when moving into NRT  */
   /* Set handoff parameters for this state                                 */
   pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currPermittedNumCandtSetEntry = 
      pMac->roam.handoffInfo.handoffParams.nrtParams.numCandtSetEntry;

   pMac->roam.handoffInfo.handoffActivityInfo.currPmkCacheRssiDelta = 
      pMac->roam.handoffInfo.handoffParams.nrtParams.pmkCacheRssiDelta;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant = 
      pMac->roam.handoffInfo.handoffParams.nrtParams.rssiFilterConst;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet = 
      pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCandtSet;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta = 
      pMac->roam.handoffInfo.handoffParams.nrtParams.bestCandidateApRssiDelta;

   status = WLANTL_SetAlpha(pMac->roam.gVosContext, 
                            (tANI_U8)pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant/10);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPostTransHandler: couldn't register csrRoamNrtExitCriteriaRssiIndCallback with TL\n");
   }

   //register the rssi indications callback with TL
   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdHoFromCurrentAp * (-1),
                                 WLANTL_HO_THRESHOLD_CROSS, 
                                 csrRoamNrtExitCriteriaRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPostTransHandler: couldn't register csrRoamNrtExitCriteriaRssiIndCallback with TL\n");
      return;
   }

   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGood * (-1),
                                 WLANTL_HO_THRESHOLD_CROSS, 
                                 csrRoamNrtBgScanRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPostTransHandler: couldn't register csrRoamNrtBgScanRssiIndCallback with TL\n");
      return;
   }


   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.nrtParams.rssiThresholdCurrentApGoodEmptyCandtset * (-1),
                                 WLANTL_HO_THRESHOLD_CROSS, 
                                 csrRoamNrtBgScanEmptyCandSetRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamNrtPostTransHandler: couldn't register csrRoamNrtBgScanEmptyCandSetRssiIndCallback with TL\n");
      return;
   }

   /* Find out what new time to use next                                    */
   stats_time = csrRoamGetStatsTimerVal(pMac);

   csrRoamStartStatisticsTimer(pMac, stats_time);
}

static void csrRoamRtPreTransHandler(tpAniSirGlobal pMac)
{
   VOS_STATUS status;

   /* This function handles common processing required when moving out of RT */
   /* Clear all timers                                                      */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = FALSE;

   /* Check if scan rsp is still awaited and set flag accordingly           */
   if( TRUE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /* additional flag to show that further scans cannot be initiated      */
      pMac->roam.handoffInfo.ignoreScanFrmOthrState = TRUE;
   }
   //deregister the rssi indications callback with TL
   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdHoFromCurrentAp * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
								   csrRoamRtExitCriteriaRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamRtPreTransHandler: couldn't deregister csrRoamRtExitCriteriaRssiIndCallback with TL\n");
      return;
   }

   status = 
      WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCurrentApGood * (-1), 
                                   WLANTL_HO_THRESHOLD_CROSS,
								   csrRoamRtBgScanRssiIndCallback, 
								   VOS_MODULE_ID_SME);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamRtPreTransHandler: couldn't deregister csrRoamRtBgScanRssiIndCallback with TL\n");
      return;
   }

}

static void csrRoamRtPostTransHandler(tpAniSirGlobal pMac)
{
   tANI_U32 stats_time;
   VOS_STATUS status;

   /* This function handles common processing required when moving into RT  */
   /* Set handoff parameters for this state                                 */
   pMac->roam.handoffInfo.handoffActivityInfo.channelScanHistory = 0;

   pMac->roam.handoffInfo.handoffActivityInfo.currPermittedNumCandtSetEntry = 
      pMac->roam.handoffInfo.handoffParams.rtParams.numCandtSetEntry;

   pMac->roam.handoffInfo.handoffActivityInfo.currPmkCacheRssiDelta = 
      pMac->roam.handoffInfo.handoffParams.rtParams.pmkCacheRssiDelta;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant = 
      pMac->roam.handoffInfo.handoffParams.rtParams.rssiFilterConst;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiThresholdCandtSet = 
      pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCandtSet;

   pMac->roam.handoffInfo.handoffActivityInfo.currRssiHandoffDelta = 
      pMac->roam.handoffInfo.handoffParams.rtParams.bestCandidateApRssiDelta;


   status = WLANTL_SetAlpha(pMac->roam.gVosContext, 
                            (tANI_U8)pMac->roam.handoffInfo.handoffActivityInfo.currRssiFilterConstant/10);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamRtPostTransHandler: couldn't register csrRoamNrtExitCriteriaRssiIndCallback with TL\n");
   }


   //register the rssi indications callback with TL
   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdHoFromCurrentAp * (-1), 
                                 WLANTL_HO_THRESHOLD_CROSS,
                                 csrRoamRtExitCriteriaRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamRtPostTransHandler: couldn't register csrRoamRtExitCriteriaRssiIndCallback with TL\n");
      return;
   }


   status = 
      WLANTL_RegRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)pMac->roam.handoffInfo.handoffParams.rtParams.rssiThresholdCurrentApGood * (-1), 
                                 WLANTL_HO_THRESHOLD_CROSS,
                                 csrRoamRtBgScanRssiIndCallback, 
								 VOS_MODULE_ID_SME, pMac);

   if(!VOS_IS_STATUS_SUCCESS( status))
   {
      smsLog(pMac, LOGW, " csrRoamRtPostTransHandler: couldn't register csrRoamRtBgScanRssiIndCallback with TL\n");
      return;
   }
   
   /* Find out what new time to use next                                    */
   stats_time = csrRoamGetStatsTimerVal(pMac);

   csrRoamStartStatisticsTimer(pMac, stats_time);
}

void csrRoamCreateHandoffProfile(tpAniSirGlobal pMac, tCsrBssid bssid)
{
   eHalStatus status;
   //get the ho profile
   if(pMac->roam.handoffInfo.handoffProfile.BSSIDs.bssid)
   {
      palFreeMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffProfile.BSSIDs.bssid);
   }
   if(pMac->roam.handoffInfo.handoffProfile.SSIDs.SSIDList)
   {
      palFreeMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffProfile.SSIDs.SSIDList);
   }

   palZeroMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffProfile, sizeof(tCsrRoamProfile));

   //BSSID
   status = palAllocateMemory( pMac->hHdd, 
                               (void **) &pMac->roam.handoffInfo.handoffProfile.BSSIDs.bssid, 
                               sizeof( tCsrBssid ) );
   if(!HAL_STATUS_SUCCESS(status))
   {
      //msg
      smsLog(pMac, LOGW, " csrRoamCreateHandoffProfile: couldn't allocate memory for bssid\n");
      return;
   }

   pMac->roam.handoffInfo.handoffProfile.BSSIDs.numOfBSSIDs = 1;

   palCopyMemory(pMac->hHdd, pMac->roam.handoffInfo.handoffProfile.BSSIDs.bssid, 
                 bssid,
                 sizeof( tCsrBssid ));

   //SSID
   status = palAllocateMemory( pMac->hHdd, (void **) &pMac->roam.handoffInfo.handoffProfile.SSIDs.SSIDList, sizeof( tCsrSSIDInfo ) );
   if(!HAL_STATUS_SUCCESS(status))
   {
      //msg
      smsLog(pMac, LOGW, " csrRoamCreateHandoffProfile: couldn't allocate memory for ssid\n");
      return;
   }

   pMac->roam.handoffInfo.handoffProfile.SSIDs.numOfSSIDs = 1;

   palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffProfile.SSIDs.SSIDList[0].SSID, 
                 &pMac->roam.connectedProfile.SSID,
                 sizeof( tSirMacSSid ));

   //security
   pMac->roam.handoffInfo.handoffProfile.AuthType = pMac->roam.connectedProfile.AuthInfo;
   pMac->roam.handoffInfo.handoffProfile.EncryptionType = pMac->roam.connectedProfile.EncryptionInfo;
   pMac->roam.handoffInfo.handoffProfile.mcEncryptionType = pMac->roam.connectedProfile.mcEncryptionInfo;

}



void csrRoamHandoffRequested(tpAniSirGlobal pMac)
{
   /* cleanup timers, as we are handing off                                 */
   csrRoamStopStatisticsTimer(pMac);
   csrScanStopBgScanTimer(pMac);
   pMac->roam.handoffInfo.isBgScanRspPending = FALSE;
   pMac->roam.handoffInfo.isProbeRspPending = FALSE;
   //save the sub-state
   pMac->roam.handoffInfo.currSubState = pMac->roam.curSubState;
   pMac->roam.handoffInfo.subStateBeforeHandoff = pMac->roam.handoffInfo.currSubState;
   /* disassoc with the current AP, once the disassoc rsp comes back issue
      join req with the new AP                                              */
   smsLog(pMac, LOGW, " csrRoamHandoffRequested: disassociating with current AP\n");
   //?? do I need to free the roam profile
   if(!HAL_STATUS_SUCCESS(csrRoamIssueDisassociateCmd(pMac, eCSR_DISCONNECT_REASON_HANDOFF)))
   {
       smsLog(pMac, LOGW, "csrRoamHandoffRequested:  fail to issue disassociate\n");
       return;
   }                       

   pMac->roam.roamingReason = eCsrDynamicRoaming;

   pMac->roam.handoffInfo.handoffAction = TRUE;
}
#endif

eHalStatus csrRoamRegisterLinkQualityIndCallback(tHalHandle hHal,
                                                 csrRoamLinkQualityIndCallback   callback,  
                                                 void                           *pContext)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   pMac->roam.linkQualityIndInfo.callback = callback;
   pMac->roam.linkQualityIndInfo.context = pContext;
   if( NULL == callback )
   {
     smsLog(pMac, LOGW, "csrRoamRegisterLinkQualityIndCallback: indication callback being deregistered");
   }
   else
   {
     smsLog(pMac, LOGW, "csrRoamRegisterLinkQualityIndCallback: indication callback being registered");

     /* do we need to invoke the callback to notify client of initial value ??  */
   }
   return eHAL_STATUS_SUCCESS;
}

void csrRoamVccTrigger(tpAniSirGlobal pMac)
{
   eCsrRoamLinkQualityInd newVccLinkQuality;
#ifdef FEATURE_WLAN_GEN6_ROAMING
   tANI_U32 fail_cnt_delta, tx_frm_cnt_delta;
   tANI_U8                            index;
   tANI_U32                           sum_tx_frm_cnt = 0;
   tANI_U32                           sum_tx_fail_cnt = 0;
#endif
   tANI_U32 ul_mac_loss = 0;
   tANI_U32 ul_mac_loss_trigger_threshold;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   v_S7_t          currApRssi;
   VOS_STATUS      status;
   WLAN_VOS_DIAG_EVENT_DEF(vcc, vos_event_wlan_vcc_payload_type);
#endif //FEATURE_WLAN_DIAG_SUPPORT
 /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef FEATURE_WLAN_GEN6_ROAMING
   //sum up the per AC counter
   for(index = 0; index < 4; index++)
   {
      sum_tx_frm_cnt +=pMac->roam.summaryStatsInfo.tx_frm_cnt[index];
   }
   for(index = 0; index < 4; index++)
   {
      sum_tx_fail_cnt +=pMac->roam.summaryStatsInfo.fail_cnt[index];
   }

   /*-------------------------------------------------------------------------
     Compute link quality based on new operational info; statistics are
     cumulative; need to compare against previous values
   -------------------------------------------------------------------------*/
   /* preserve previous values to do a delta                                */
   if( pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt > sum_tx_fail_cnt )
   {
     fail_cnt_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt + 
        sum_tx_fail_cnt;
   }
   else
   {
     fail_cnt_delta = sum_tx_fail_cnt - 
        pMac->roam.handoffInfo.currStatsInfo.tx_fail_cnt;
   }

   if(pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt > sum_tx_frm_cnt)
   {
     tx_frm_cnt_delta = (tANI_U32)~0 - pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt + 
       sum_tx_frm_cnt;
   }
   else
   {
     tx_frm_cnt_delta = sum_tx_frm_cnt - 
        pMac->roam.handoffInfo.currStatsInfo.tx_frm_cnt;
   }

   /* check for zero denominator                                            */
   if( tx_frm_cnt_delta + fail_cnt_delta )
   {
     /* UL MAC Loss is represented in %                                     */
      ul_mac_loss = 100 * fail_cnt_delta/(tx_frm_cnt_delta+fail_cnt_delta);
   }
   else
   {
      ul_mac_loss = 0;
   }    
#endif
   /*-------------------------------------------------------------------------
     Link quality is currently binary based on OBIWAN recommended triggers

     Check for a change in link quality and notify client if necessary
   -------------------------------------------------------------------------*/
   ul_mac_loss_trigger_threshold = 
      pMac->roam.configParam.vccUlMacLossThreshold;

   VOS_ASSERT( ul_mac_loss_trigger_threshold != 0 );

   smsLog(pMac, LOGW, "csrRoamVccTrigger: UL_MAC_LOSS_THRESHOLD is %d\n", 
          ul_mac_loss_trigger_threshold );

   if(ul_mac_loss_trigger_threshold < ul_mac_loss)
   {
      smsLog(pMac, LOGW, "csrRoamVccTrigger: link quality is POOR \n");
      newVccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamVccTrigger: link quality is GOOD\n");
      newVccLinkQuality = eCSR_ROAM_LINK_QUAL_GOOD_IND;
   }

   smsLog(pMac, LOGW, "csrRoamVccTrigger: link qual : *** UL_MAC_LOSS %d *** ",
          ul_mac_loss);

   if(newVccLinkQuality != pMac->roam.vccLinkQuality)
   {
      smsLog(pMac, LOGW, "csrRoamVccTrigger: link quality changed: trigger necessary\n");
      if(NULL != pMac->roam.linkQualityIndInfo.callback) 
      {
         smsLog(pMac, LOGW, "csrRoamVccTrigger: link quality indication %d\n",
                newVccLinkQuality );
         
         /* we now invoke the callback once to notify client of initial value   */
         pMac->roam.linkQualityIndInfo.callback( newVccLinkQuality, 
                                                 pMac->roam.linkQualityIndInfo.context );
         //event: EVENT_WLAN_VCC
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
         vcc.eventId = eCSR_WLAN_VCC_EVENT;
         status = WLANTL_GetRssi(pMac->roam.gVosContext, pMac->roam.connectedInfo.staId,
                                 &currApRssi);

         if(status)
         {
            //err msg
            smsLog(pMac, LOGW, " csrRoamVccTrigger: couldn't get the current APs RSSi from TL\n");
            currApRssi = 0;
         }

         vcc.rssi = currApRssi * (-1);
         vcc.txPer = (tANI_U8)pMac->roam.handoffInfo.currSta.txPer;
         vcc.rxPer = (tANI_U8)pMac->roam.handoffInfo.currSta.rxPer;
         vcc.linkQuality = newVccLinkQuality;
         WLAN_VOS_DIAG_EVENT_REPORT(&vcc, EVENT_WLAN_VCC);
#endif //FEATURE_WLAN_DIAG_SUPPORT
      }
   }

   pMac->roam.vccLinkQuality = newVccLinkQuality;


}

VOS_STATUS csrRoamVccTriggerRssiIndCallback(tHalHandle hHal, 
                                            v_U8_t  rssiNotification, 
                                            void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   eCsrRoamLinkQualityInd newVccLinkQuality;
   VOS_STATUS status = VOS_STATUS_SUCCESS;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
   v_S7_t          currApRssi;
   VOS_STATUS      statusTl;
   WLAN_VOS_DIAG_EVENT_DEF(vcc, vos_event_wlan_vcc_payload_type);
#endif //FEATURE_WLAN_DIAG_SUPPORT

   /*-------------------------------------------------------------------------
     Link quality is currently binary based on OBIWAN recommended triggers

     Check for a change in link quality and notify client if necessary
   -------------------------------------------------------------------------*/
   smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: RSSI trigger threshold is %d\n", 
          pMac->roam.configParam.vccRssiThreshold);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification)
   {
      smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: link quality is POOR\n");
      newVccLinkQuality = eCSR_ROAM_LINK_QUAL_POOR_IND;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification)
   {
      smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: link quality is GOOD \n");
      newVccLinkQuality = eCSR_ROAM_LINK_QUAL_GOOD_IND;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: unknown rssi notification %d\n", rssiNotification);
      //Set to this so the code below won't do anything
      newVccLinkQuality = pMac->roam.vccLinkQuality;    

      VOS_ASSERT(0);
   }


   if(newVccLinkQuality != pMac->roam.vccLinkQuality)
   {
      smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: link quality changed: trigger necessary\n");
      if(NULL != pMac->roam.linkQualityIndInfo.callback) 
      {
         smsLog(pMac, LOGW, "csrRoamVccTriggerRssiIndCallback: link quality indication %d\n",
                newVccLinkQuality);

        /* we now invoke the callback once to notify client of initial value   */
        pMac->roam.linkQualityIndInfo.callback( newVccLinkQuality, 
                                                pMac->roam.linkQualityIndInfo.context );
         //event: EVENT_WLAN_VCC
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
         vcc.eventId = eCSR_WLAN_VCC_EVENT;
         statusTl = WLANTL_GetRssi(pMac->roam.gVosContext, pMac->roam.connectedInfo.staId,
                                 &currApRssi);

         if(statusTl)
         {
            //err msg
            smsLog(pMac, LOGW, " csrRoamVccTrigger: couldn't get the current APs RSSi from TL\n");
            currApRssi = 0;
         }

         vcc.rssi = currApRssi * (-1);
         vcc.txPer = (tANI_U8)pMac->roam.handoffInfo.currSta.txPer;
         vcc.rxPer = (tANI_U8)pMac->roam.handoffInfo.currSta.rxPer;
         vcc.linkQuality = newVccLinkQuality;
         WLAN_VOS_DIAG_EVENT_REPORT(&vcc, EVENT_WLAN_VCC);
#endif //FEATURE_WLAN_DIAG_SUPPORT
      }
   }

   pMac->roam.vccLinkQuality = newVccLinkQuality;

   return status;
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
VOS_STATUS csrRoamNtRssiIndCallback(tHalHandle hHal, 
                                    v_U8_t  rssiNotification, 
                                    void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   tCsrBssid *         pBssid = NULL;
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   tANI_U32            oldRssiValue;

   smsLog(pMac, LOGW, "csrRoamNtRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamNtRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   VOS_ASSERT(WLANTL_HO_THRESHOLD_DOWN == rssiNotification );

   //To avoid getting multiple notification from TL
   oldRssiValue = pMac->roam.handoffInfo.handoffActivityInfo.currNtRssiThreshold;
   // do I need to deregister first??
   status = 
    WLANTL_DeregRSSIIndicationCB(pMac->roam.gVosContext, (v_S7_t)oldRssiValue * (-1), WLANTL_HO_THRESHOLD_DOWN,
                   csrRoamNtRssiIndCallback, VOS_MODULE_ID_SME);

   if( 0 == csrScanListSize( pMac, &pMac->roam.handoffInfo.candidateList ))
   {
      smsLog(pMac, LOGW, "csrRoamNtRssiIndCallback: candidateList empty!\n");

      //removing the assert, as it could happen that at some point of time SME
      //registered the RSSI threshold with TL, but the candidate fall off the
      //list some time later 
      //VOS_ASSERT(0);
      return VOS_STATUS_SUCCESS;
   }
   /* Get the top candidate to see if that suits our requirements           */
   pBssid = csrScanGetHoCandidate(pMac);

   if(!pBssid)
   {
      smsLog(pMac, LOGW, "csrRoamNtRssiIndCallback: candidate is NULL!\n");

   }
   else
   {
   /* set to the probed BSSID                                              */
   palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                 pBssid, WNI_CFG_BSSID_LEN);

   smsLog( pMac, LOGW, "csrRoamNtRssiIndCallback: CSR Attempting to probe Bssid= %02x-%02x-%02x-%02x-%02x-%02x\n", 
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 0 ], 
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 1 ], 
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 2 ],
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 3 ], 
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 4 ], 
		   pMac->roam.handoffInfo.handoffActivityInfo.probedBssid[ 5 ] );    

   csrScanSendBgProbeReq(pMac, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid);
   }
   return status;
}

VOS_STATUS csrRoamNtBgScanRssiIndCallback(tHalHandle hHal, 
                                          v_U8_t  rssiNotification, 
                                          void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   tANI_U32 scan_time;
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   smsLog(pMac, LOGW, "csrRoamNtBgScanRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamNtBgScanRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamNrtBgScanRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);

      return VOS_STATUS_E_FAILURE;
   }


   if( FALSE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /*-----------------------------------------------------------------------
       Check if bg scan needs to be started for this state
      -----------------------------------------------------------------------*/
      if( FALSE == csrScanIsBgScanEnabled(pMac) )
      {
         return VOS_STATUS_E_FAILURE;
      }

      /*-----------------------------------------------------------------------
       If bg scan required, then start the scan timer
       Call scan timer generator
      -----------------------------------------------------------------------*/
      scan_time = csrScanGetBgScanTimerVal(pMac);

      if(scan_time )
      {
         //start the bg scan timer
         csrScanStartBgScanTimer(pMac, scan_time);

         pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;
      }
   }
   return status;
}


VOS_STATUS csrRoamNrtBgScanRssiIndCallback(tHalHandle hHal, 
                                     v_U8_t  rssiNotification, 
                                     void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   tANI_U32 scan_time;
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   smsLog(pMac, LOGW, "csrRoamNrtBgScanRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamNrtBgScanRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamNrtBgScanRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);

      return VOS_STATUS_E_FAILURE;
   }


   if( FALSE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /*-----------------------------------------------------------------------
       Check if bg scan needs to be started for this state
      -----------------------------------------------------------------------*/
      if( FALSE == csrScanIsBgScanEnabled(pMac) )
      {
         return VOS_STATUS_E_FAILURE;
      }

      /*-----------------------------------------------------------------------
       If bg scan required, then start the scan timer
       Call scan timer generator
      -----------------------------------------------------------------------*/
      scan_time = csrScanGetBgScanTimerVal(pMac);

      if(scan_time )
      {
         //start the bg scan timer
         csrScanStartBgScanTimer(pMac, scan_time);

         pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;
      }
   }
   return status;
}


VOS_STATUS csrRoamNrtBgScanEmptyCandSetRssiIndCallback(tHalHandle hHal, 
                                                       v_U8_t  rssiNotification, 
                                                       void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   smsLog(pMac, LOGW, "csrRoamNrtBgScanEmptyCandSetRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamNrtBgScanEmptyCandSetRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isNrtBgScanEmptyCandSetPermitted = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isNrtBgScanEmptyCandSetPermitted = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamNrtBgScanEmptyCandSetRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);
      return VOS_STATUS_E_FAILURE;
   }

   return status;
}


VOS_STATUS csrRoamNrtExitCriteriaRssiIndCallback(tHalHandle hHal, 
                                           v_U8_t  rssiNotification, 
                                           void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   tCsrBssid * pBssid;

   smsLog(pMac, LOGW, "csrRoamNrtExitCriteriaRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamNrtExitCriteriaRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification )
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification )
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamNrtExitCriteriaRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);

      return VOS_STATUS_E_FAILURE;
   }

   if( TRUE == csrRoamShouldExitAp(pMac) )
   {
      smsLog(pMac, LOGW, " csrRoamNrtExitCriteriaRssiIndCallback : Exit criteria met for handoff\n");
      smsLog(pMac, LOGW, " csrRoamNrtExitCriteriaRssiIndCallback : Current STA info:\n");
      smsLog(pMac, LOGW, " csrRoamNrtExitCriteriaRssiIndCallback: BSSID= %02x-%02x-%02x-%02x-%02x-%02x\n", 
             pMac->roam.handoffInfo.currSta.bssid[ 0 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 1 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 2 ],
             pMac->roam.handoffInfo.currSta.bssid[ 3 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 4 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 5 ] );

      smsLog(pMac, LOGW, " csrRoamNrtExitCriteriaRssiIndCallback : channel = %d\n", pMac->roam.handoffInfo.currSta.pBssDesc->channelId);
      smsLog(pMac, LOGW, " Tx PER = %d, Rx PER = %d\n", 
             pMac->roam.handoffInfo.currSta.txPer,
             pMac->roam.handoffInfo.currSta.rxPer);
      
      /* Get the top candidate to see if that suits our requirements           */
      pBssid = csrScanGetHoCandidate(pMac);
      
      if(NULL == pBssid)
      {
         //msg
         smsLog(pMac, LOGW, "csrRoamNrtExitCriteriaRssiIndCallback: Candidate is NULL\n");
      }
      else
      {
         /* set to the probed BSSID                                              */
         palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                       pBssid, WNI_CFG_BSSID_LEN);
         
         csrScanSendBgProbeReq(pMac, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid);
      }
   }
   return status;
}


VOS_STATUS csrRoamRtBgScanRssiIndCallback(tHalHandle hHal, 
                                          v_U8_t  rssiNotification, 
                                          void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   tANI_U32 scan_time;
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   smsLog(pMac, LOGW, "csrRoamRtBgScanRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamRtBgScanRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification)
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isBgScanPermitted = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamRtBgScanRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);
   }


   if( FALSE == pMac->roam.handoffInfo.isBgScanRspPending )
   {
      /*-----------------------------------------------------------------------
       Check if bg scan needs to be started for this state
      -----------------------------------------------------------------------*/
      if( FALSE == csrScanIsBgScanEnabled(pMac) )
      {
         return status;
      }

      /*-----------------------------------------------------------------------
       If bg scan required, then start the scan timer
       Call scan timer generator
      -----------------------------------------------------------------------*/
      scan_time = csrScanGetBgScanTimerVal(pMac);

      if(scan_time )
      {
         //start the bg scan timer
         csrScanStartBgScanTimer(pMac, scan_time);

         pMac->roam.handoffInfo.handoffActivityInfo.bgScanTimerExpiredAlready = FALSE;
      }
   }

   return status;
}


VOS_STATUS csrRoamRtExitCriteriaRssiIndCallback(tHalHandle hHal, 
                                                v_U8_t  rssiNotification, 
                                                void * context)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( context );
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   tCsrBssid * pBssid;

   smsLog(pMac, LOGW, "csrRoamRtExitCriteriaRssiIndCallback: rssi notification for %d\n", rssiNotification);
   if(!csrIsConnStateConnectedInfra(pMac))
   {
      smsLog(pMac, LOGW, "csrRoamRtExitCriteriaRssiIndCallback: ignoring the indication as we are not connected\n");
      return VOS_STATUS_SUCCESS;
   }

   if(WLANTL_HO_THRESHOLD_DOWN == rssiNotification )
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = TRUE;
   }
   else if(WLANTL_HO_THRESHOLD_UP == rssiNotification )
   {
      pMac->roam.handoffInfo.handoffActivityInfo.isRtNrtRssiExitCriteriaSet = FALSE;
   }
   else
   {
      smsLog(pMac, LOGW, "csrRoamRtExitCriteriaRssiIndCallback: unknown rssi notification %d\n", rssiNotification);

      VOS_ASSERT(0);
   }

   if( TRUE == csrRoamShouldExitAp(pMac) )
   {
      smsLog(pMac, LOGW, " csrRoamRtExitCriteriaRssiIndCallback : Exit criteria met for handoff\n");
      smsLog(pMac, LOGW, " csrRoamRtExitCriteriaRssiIndCallback : Current STA info:\n");
      smsLog(pMac, LOGW, " csrRoamRtExitCriteriaRssiIndCallback: BSSID= %02x-%02x-%02x-%02x-%02x-%02x\n", 
             pMac->roam.handoffInfo.currSta.bssid[ 0 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 1 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 2 ],
             pMac->roam.handoffInfo.currSta.bssid[ 3 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 4 ], 
             pMac->roam.handoffInfo.currSta.bssid[ 5 ] );

      smsLog(pMac, LOGW, " csrRoamRtExitCriteriaRssiIndCallback : channel = %d\n", pMac->roam.handoffInfo.currSta.pBssDesc->channelId);
      smsLog(pMac, LOGW, " Tx PER = %d, Rx PER = %d\n", 
             pMac->roam.handoffInfo.currSta.txPer,
             pMac->roam.handoffInfo.currSta.rxPer);

      /* Get the top candidate to see if that suits our requirements           */
      pBssid = csrScanGetHoCandidate(pMac);

      if(NULL == pBssid)
      {
         //msg
         smsLog(pMac, LOGW, "csrRoamRtExitCriteriaRssiIndCallback: Candidate is NULL\n");
      }
      else
      {
         /* set to the probed BSSID                                              */
         palCopyMemory(pMac->hHdd, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid, 
                       pBssid, WNI_CFG_BSSID_LEN);

         csrScanSendBgProbeReq(pMac, &pMac->roam.handoffInfo.handoffActivityInfo.probedBssid);
      }
   }
   return status;
}
#endif

tCsrStatsClientReqInfo * csrRoamInsertEntryIntoList( tpAniSirGlobal pMac,
                                                     tDblLinkList *pStaList,
                                                     tCsrStatsClientReqInfo *pStaEntry)
{
   tCsrStatsClientReqInfo *pNewStaEntry = NULL;

   eHalStatus  status;

   //if same entity requested for same set of stats with different periodicity & 
   // callback update it
   if(NULL == csrRoamChecknUpdateClientReqList(pMac, pStaEntry, TRUE))
   {
   
      status = palAllocateMemory(pMac->hHdd, (void **)&pNewStaEntry, sizeof(tCsrStatsClientReqInfo));
      if (!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGW, "csrRoamInsertEntryIntoList: couldn't allocate memory for the \
                entry\n");
         return NULL;
      }
   

      pNewStaEntry->callback = pStaEntry->callback;
      pNewStaEntry->pContext = pStaEntry->pContext;
      pNewStaEntry->periodicity = pStaEntry->periodicity;
      pNewStaEntry->requesterId = pStaEntry->requesterId;
      pNewStaEntry->statsMask = pStaEntry->statsMask;
      pNewStaEntry->pPeStaEntry = pStaEntry->pPeStaEntry;
      pNewStaEntry->pMac = pStaEntry->pMac;
      pNewStaEntry->staId = pStaEntry->staId;
      pNewStaEntry->timerExpired = pStaEntry->timerExpired;
      
      csrLLInsertTail( pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK  );
   }
   return pNewStaEntry;
}


tCsrPeStatsReqInfo * csrRoamInsertEntryIntoPeStatsReqList( tpAniSirGlobal pMac,
                                                           tDblLinkList *pStaList,
                                                           tCsrPeStatsReqInfo *pStaEntry)
{
   tCsrPeStatsReqInfo *pNewStaEntry = NULL;

   eHalStatus  status;

   status = palAllocateMemory(pMac->hHdd, (void **)&pNewStaEntry, sizeof(tCsrPeStatsReqInfo));
   if (!HAL_STATUS_SUCCESS(status))
   {
      smsLog(pMac, LOGW, "csrRoamInsertEntryIntoPeStatsReqList: couldn't allocate memory for the \
                  entry\n");
      return NULL;
   }
   

   pNewStaEntry->hPeStatsTimer = pStaEntry->hPeStatsTimer;
   pNewStaEntry->numClient = pStaEntry->numClient;
   pNewStaEntry->periodicity = pStaEntry->periodicity;
   pNewStaEntry->statsMask = pStaEntry->statsMask;
   pNewStaEntry->pMac = pStaEntry->pMac;
   pNewStaEntry->staId = pStaEntry->staId;
   pNewStaEntry->timerRunning = pStaEntry->timerRunning;
   pNewStaEntry->rspPending = pStaEntry->rspPending;
   
   csrLLInsertTail( pStaList, &pNewStaEntry->link, LL_ACCESS_LOCK  );

   return pNewStaEntry;
}

#ifdef FEATURE_WLAN_GEN6_ROAMING
void  csrRoamHandoffRemoveAllFromList( tpAniSirGlobal pMac,
                                       tDblLinkList *pStaList)

{
   tListElem *pEntry, *pTmpEntry;
   tCsrHandoffStaInfo *pTempStaEntry;

   pEntry = csrLLPeekHead( pStaList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamHandoffRemoveAllFromList: List empty\n");
      return;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrHandoffStaInfo, link );
      smsLog(pMac, LOGW, "csrRoamHandoffRemoveAllFromList: removing entry\n");
      if(pTempStaEntry->sta.pBssDesc)
      {
         palFreeMemory(pMac->hHdd, pTempStaEntry->sta.pBssDesc);
      }
      pTmpEntry = csrLLNext( pStaList, pEntry, LL_ACCESS_NOLOCK );
      csrLLRemoveEntry( pStaList, pEntry, LL_ACCESS_LOCK );
      palFreeMemory(pMac->hHdd, pTempStaEntry);
      pEntry = pTmpEntry;
   }

}
#endif

eHalStatus csrGetStatistics(tHalHandle hHal, eCsrStatsRequesterType requesterId, 
                            tANI_U32 statsMask, 
                            tCsrStatsCallback callback, 
                            tANI_U32 periodicity, tANI_BOOLEAN cache, 
                            tANI_U8 staId, void *pContext)
{  
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tCsrStatsClientReqInfo staEntry;
   tCsrStatsClientReqInfo *pStaEntry = NULL;
   tCsrPeStatsReqInfo *pPeStaEntry = NULL; 
   tListElem *pEntry = NULL;
   tANI_BOOLEAN found = FALSE;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   WLANTL_TRANSFER_STA_TYPE tlStats;
   tANI_BOOLEAN insertInClientList = FALSE;
   VOS_STATUS vosStatus;

   if( pMac->roam.curState != eCSR_ROAMING_STATE_JOINED 
#ifdef FEATURE_WLAN_GEN6_ROAMING
       &&
      !( pMac->roam.handoffInfo.isBgScanRspPending && 
         pMac->roam.curState == eCSR_ROAMING_STATE_SCANNING)
#endif
       )
   {
#ifdef FEATURE_WLAN_GEN6_ROAMING
      smsLog(pMac, LOGW, "csrGetStatistics: wrong state %d\n", pMac->roam.handoffInfo.currState);
#else
      smsLog(pMac, LOGW, "csrGetStatistics: wrong state %d\n", pMac->roam.curState);
#endif
      return eHAL_STATUS_FAILURE;
   }

   if((!statsMask) && (!callback))
   {
      //msg
      smsLog(pMac, LOGW, "csrGetStatistics: statsMask & callback empty in the request\n");
      return eHAL_STATUS_FAILURE;
   }

   //for the search list method for deregister
   staEntry.requesterId = requesterId;
   staEntry.statsMask = statsMask;
   //requester wants to deregister or just an error
   if((statsMask) && (!callback))
   {
      pEntry = csrRoamChecknUpdateClientReqList(pMac, &staEntry, FALSE);
      if(!pEntry)
      {
         //msg
         smsLog(pMac, LOGW, "csrGetStatistics: callback is empty in the request & couldn't \
                find any existing request in statsClientReqList\n");
         return eHAL_STATUS_FAILURE;
      }
      else
      {
         //clean up & return
         pStaEntry = GET_BASE_ADDR( pEntry, tCsrStatsClientReqInfo, link );
         pStaEntry->pPeStaEntry->numClient--;
         //check if we need to delete the entry from peStatsReqList too
         if(!pStaEntry->pPeStaEntry->numClient)
         {
            csrRoamRemoveEntryFromPeStatsReqList(pMac, pStaEntry->pPeStaEntry);
         }
         //check if we need to stop the tl stats timer too 
         pMac->roam.tlStatsReqInfo.numClient--;
         if(!pMac->roam.tlStatsReqInfo.numClient)
         {
            if(pMac->roam.tlStatsReqInfo.timerRunning)
            {
               status = palTimerStop(pMac->hHdd, pMac->roam.tlStatsReqInfo.hTlStatsTimer);
               if(!HAL_STATUS_SUCCESS(status))
               {
                  smsLog(pMac, LOGE, FL("csrGetStatistics:cannot stop TlStatsTimer timer\n"));
                  return eHAL_STATUS_FAILURE;
               }
            }
            pMac->roam.tlStatsReqInfo.periodicity = 0;
            pMac->roam.tlStatsReqInfo.timerRunning = FALSE;
         }
         vos_timer_stop( &pStaEntry->timer );

         // Destroy the vos timer...      
         vosStatus = vos_timer_destroy( &pStaEntry->timer );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
         {
            smsLog(pMac, LOGE, FL("csrGetStatistics:failed to destroy Client req timer\n"));
         }

         csrLLRemoveEntry(&pMac->roam.statsClientReqList, pEntry, LL_ACCESS_LOCK);
         pStaEntry = NULL;
         return eHAL_STATUS_SUCCESS;
      }
   }
   
   if(cache && !periodicity)
   {
      //return the cached stats
      csrRoamReportStatistics(pMac, statsMask, callback, staId, pContext);
   }
   else
   {
      //add the request in the client req list
      staEntry.callback = callback;
      staEntry.pContext = pContext;
      staEntry.periodicity = periodicity;
      staEntry.pPeStaEntry = NULL;
      staEntry.staId = staId;
      staEntry.pMac = pMac;
      staEntry.staId = staId;
      staEntry.timerExpired = FALSE;
   
   

      //if periodic report requested with non cached result from PE/TL
      if(periodicity)
      {
      
         //if looking for stats from PE
         if(statsMask & ~(1 << eCsrGlobalClassDStats))
         {
         
            //check if same request made already & waiting for rsp
            pPeStaEntry = csrRoamCheckPeStatsReqList(pMac, statsMask & ~(1 << eCsrGlobalClassDStats), 
                                               periodicity, &found, staId);
            if(!pPeStaEntry)
            {
               //bail out, maxed out on number of req for PE
               return eHAL_STATUS_FAILURE;
            }
            else
            {
               staEntry.pPeStaEntry = pPeStaEntry;
            }
               
         }
         //request stats from TL rightaway if requested by client, update tlStatsReqInfo if needed
         if(statsMask & (1 << eCsrGlobalClassDStats))
         {
            if(cache && pMac->roam.tlStatsReqInfo.numClient)
            {
               smsLog(pMac, LOGE, FL("csrGetStatistics:Looking for cached stats from TL\n"));
            }
            else
            {
            
               //update periodicity
               if(pMac->roam.tlStatsReqInfo.periodicity)
               {
                  pMac->roam.tlStatsReqInfo.periodicity = 
                     CSR_ROAM_MIN(periodicity, pMac->roam.tlStatsReqInfo.periodicity);
               }
               else
               {
                  pMac->roam.tlStatsReqInfo.periodicity = periodicity;
               }
               if(pMac->roam.tlStatsReqInfo.periodicity < CSR_MIN_TL_STAT_QUERY_PERIOD)
               {
                  pMac->roam.tlStatsReqInfo.periodicity = CSR_MIN_TL_STAT_QUERY_PERIOD;
               }
               
               if(!pMac->roam.tlStatsReqInfo.timerRunning)
               {
                  //req TL for class D stats
                  if(WLANTL_GetStatistics(pMac->roam.gVosContext, &tlStats, pMac->roam.connectedInfo.staId))
                  {
                     smsLog(pMac, LOGE, FL("csrGetStatistics:couldn't get the stats from TL\n"));
                  }
                  else
                  {
                     //save in SME
                     csrRoamSaveStatsFromTl(pMac, tlStats);
                  }
                  if(pMac->roam.tlStatsReqInfo.periodicity)
                  {
                     //start timer
                     status = palTimerStart(pMac->hHdd, pMac->roam.tlStatsReqInfo.hTlStatsTimer, 
                                            pMac->roam.tlStatsReqInfo.periodicity * PAL_TIMER_TO_MS_UNIT, eANI_BOOLEAN_FALSE);
                     if(!HAL_STATUS_SUCCESS(status))
                     {
                        smsLog(pMac, LOGE, FL("csrGetStatistics:cannot start TlStatsTimer timer\n"));
                        return eHAL_STATUS_FAILURE;
                     }
                     pMac->roam.tlStatsReqInfo.timerRunning = TRUE;
                  }
               }
            }
            pMac->roam.tlStatsReqInfo.numClient++;
         }
   
         insertInClientList = TRUE;
      }
      //if one time report requested with non cached result from PE/TL
      else if(!cache && !periodicity)
      {
         if(statsMask & ~(1 << eCsrGlobalClassDStats))
         {
            //send down a req
            status = csrSendMBStatsReqMsg(pMac, statsMask & ~(1 << eCsrGlobalClassDStats), staId);
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog(pMac, LOGE, FL("csrGetStatistics:failed to send down stats req to PE\n"));
            }
            //so that when the stats rsp comes back from PE we respond to upper layer
            //right away
            staEntry.timerExpired = TRUE;
            insertInClientList = TRUE;

         }
         if(statsMask & (1 << eCsrGlobalClassDStats))
         {
            //req TL for class D stats
            if(WLANTL_GetStatistics(pMac->roam.gVosContext, &tlStats, pMac->roam.connectedInfo.staId))
            {
               smsLog(pMac, LOGE, FL("csrGetStatistics:couldn't get the stats from TL\n"));
            }
            else
            {
               //save in SME
               csrRoamSaveStatsFromTl(pMac, tlStats);
            }

         }
         //if looking for stats from TL only 
         if(!insertInClientList)
         {
            //return the stats
            csrRoamReportStatistics(pMac, statsMask, callback, staId, pContext);
         }

      }

      if(insertInClientList)
      {
         pStaEntry = csrRoamInsertEntryIntoList(pMac, &pMac->roam.statsClientReqList, &staEntry); 
         if(!pStaEntry)
         {
            //msg
            smsLog(pMac, LOGW, "csrGetStatistics: Failed to insert req in statsClientReqList\n");
            return eHAL_STATUS_FAILURE;
         }
         //Init & start timer if needed
         if(periodicity)
         {
            vosStatus = vos_timer_init( &pStaEntry->timer, VOS_TIMER_TYPE_SW, 
                                        csrRoamStatsClientTimerHandler, pStaEntry );
            if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
            {
               smsLog(pMac, LOGE, FL("csrGetStatistics:cannot int StatsClient timer\n"));
               return eHAL_STATUS_FAILURE;
            }
            vosStatus = vos_timer_start( &pStaEntry->timer, periodicity );
            if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) 
            {
               smsLog(pMac, LOGE, FL("csrGetStatistics:cannot start StatsClient timer\n"));
               return eHAL_STATUS_FAILURE;
            }

         }

      }

   }
   return eHAL_STATUS_SUCCESS;
}


tCsrPeStatsReqInfo * csrRoamCheckPeStatsReqList(tpAniSirGlobal pMac, tANI_U32  statsMask, 
                                                tANI_U32 periodicity, tANI_BOOLEAN *pFound, tANI_U8 staId)
{
   tANI_BOOLEAN found = FALSE;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tCsrPeStatsReqInfo staEntry;
   tCsrPeStatsReqInfo *pTempStaEntry = NULL;
   tListElem *pStaEntry = NULL;
   VOS_STATUS vosStatus;
   tPmcPowerState powerState;
   *pFound = FALSE;
      
   pStaEntry = csrRoamFindInPeStatsReqList(pMac, statsMask);
   if(pStaEntry)
   {
      pTempStaEntry = GET_BASE_ADDR( pStaEntry, tCsrPeStatsReqInfo, link );
      if(pTempStaEntry->periodicity)
      {
         pTempStaEntry->periodicity = 
            CSR_ROAM_MIN(periodicity, pTempStaEntry->periodicity);
      }
      else
      {
         pTempStaEntry->periodicity = periodicity;
      }

      pTempStaEntry->numClient++;
         found = TRUE;
   }
   else
   {
      palZeroMemory(pMac->hHdd, &staEntry, sizeof(tCsrPeStatsReqInfo));
      staEntry.numClient = 1;
      staEntry.periodicity = periodicity;
      staEntry.pMac = pMac;
      staEntry.rspPending = FALSE;
      staEntry.staId = staId;
      staEntry.statsMask = statsMask;
      staEntry.timerRunning = FALSE;
      pTempStaEntry = csrRoamInsertEntryIntoPeStatsReqList(pMac, &pMac->roam.peStatsReqList, &staEntry); 
      if(!pTempStaEntry)
      {
         //msg
         smsLog(pMac, LOGW, "csrRoamCheckPeStatsReqList: Failed to insert req in peStatsReqList\n");
         return NULL;
      }

   }

   pmcQueryPowerState(pMac, &powerState, NULL, NULL);
   if(ePMC_FULL_POWER == powerState)
   {
      if(pTempStaEntry->periodicity < pMac->roam.configParam.statsReqPeriodicity)
      {
         pTempStaEntry->periodicity = pMac->roam.configParam.statsReqPeriodicity;
      }
   }
   else
   {
      if(pTempStaEntry->periodicity < pMac->roam.configParam.statsReqPeriodicityInPS)
      {
         pTempStaEntry->periodicity = pMac->roam.configParam.statsReqPeriodicityInPS;
      }
   }
   if(!pTempStaEntry->timerRunning)
   {
      //send down a req in case of one time req, for periodic ones wait for timer to expire
      if(!pTempStaEntry->rspPending && 
         !pTempStaEntry->periodicity)
      {
         status = csrSendMBStatsReqMsg(pMac, statsMask & ~(1 << eCsrGlobalClassDStats), staId);
         if(!HAL_STATUS_SUCCESS(status))
         {
            smsLog(pMac, LOGE, FL("csrRoamCheckPeStatsReqList:failed to send down stats req to PE\n"));
         }
         else
         {
            pTempStaEntry->rspPending = TRUE;
         }
      }
      if(pTempStaEntry->periodicity)
      {
         if(!found)
         {
            
            vosStatus = vos_timer_init( &pTempStaEntry->hPeStatsTimer, VOS_TIMER_TYPE_SW, 
                                         csrRoamPeStatsTimerHandler, pTempStaEntry );
            if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
            {
               smsLog(pMac, LOGE, FL("csrRoamCheckPeStatsReqList:cannot init hPeStatsTimer timer\n"));
               return NULL;
            }

         }
         //start timer
         smsLog(pMac, LOG1, "csrRoamCheckPeStatsReqList:peStatsTimer period %d\n", pTempStaEntry->periodicity);

         vosStatus = vos_timer_start( &pTempStaEntry->hPeStatsTimer, pTempStaEntry->periodicity );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) ) 
         {
            smsLog(pMac, LOGE, FL("csrRoamCheckPeStatsReqList:cannot start hPeStatsTimer timer\n"));
            return NULL;
         }
         pTempStaEntry->timerRunning = TRUE;
      }
   }

   *pFound = found;
   return pTempStaEntry;
}


void csrRoamRemoveEntryFromPeStatsReqList(tpAniSirGlobal pMac, tCsrPeStatsReqInfo *pPeStaEntry)
{
   tListElem *pEntry;
   tCsrPeStatsReqInfo *pTempStaEntry;
   VOS_STATUS vosStatus;
   pEntry = csrLLPeekHead( &pMac->roam.peStatsReqList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamRemoveEntryFromPeStatsReqList: List empty, no stats req for PE\n");
      return;
   }

   while( pEntry )
   {
      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrPeStatsReqInfo, link );

      if(pTempStaEntry->statsMask == pPeStaEntry->statsMask)
      {
         smsLog(pMac, LOGW, "csrRoamRemoveEntryFromPeStatsReqList: match found\n");
         if(pTempStaEntry->timerRunning)
         {
            vos_timer_stop( &pTempStaEntry->hPeStatsTimer );
         }
         vosStatus = vos_timer_destroy( &pTempStaEntry->hPeStatsTimer );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
         {
            smsLog(pMac, LOGE, FL("csrRoamRemoveEntryFromPeStatsReqList:failed to destroy hPeStatsTimer timer\n"));
         }
         csrLLRemoveEntry(&pMac->roam.peStatsReqList, pEntry, LL_ACCESS_LOCK);
         pTempStaEntry = NULL;
         //clean up memory ??
         break;
      }

      pEntry = csrLLNext( &pMac->roam.peStatsReqList, pEntry, LL_ACCESS_NOLOCK );
   }

   return;
}


void csrRoamSaveStatsFromTl(tpAniSirGlobal pMac, WLANTL_TRANSFER_STA_TYPE tlStats)
{

   pMac->roam.classDStatsInfo.num_rx_bytes_crc_ok = tlStats.rxBcntCRCok;
   pMac->roam.classDStatsInfo.rx_bc_byte_cnt = tlStats.rxBCBcnt;
   pMac->roam.classDStatsInfo.rx_bc_frm_cnt = tlStats.rxBCFcnt;
   pMac->roam.classDStatsInfo.rx_byte_cnt = tlStats.rxBcnt;
   pMac->roam.classDStatsInfo.rx_mc_byte_cnt = tlStats.rxMCBcnt;
   pMac->roam.classDStatsInfo.rx_mc_frm_cnt = tlStats.rxMCFcnt;
   pMac->roam.classDStatsInfo.rx_rate = tlStats.rxRate;
   //?? need per AC
   pMac->roam.classDStatsInfo.rx_uc_byte_cnt[0] = tlStats.rxUCBcnt;
   pMac->roam.classDStatsInfo.rx_uc_frm_cnt = tlStats.rxUCFcnt;
   pMac->roam.classDStatsInfo.tx_bc_byte_cnt = tlStats.txBCBcnt;
   pMac->roam.classDStatsInfo.tx_bc_frm_cnt = tlStats.txBCFcnt;
   pMac->roam.classDStatsInfo.tx_mc_byte_cnt = tlStats.txMCBcnt;
   pMac->roam.classDStatsInfo.tx_mc_frm_cnt = tlStats.txMCFcnt;
   //?? need per AC
   pMac->roam.classDStatsInfo.tx_uc_byte_cnt[0] = tlStats.txUCBcnt;
   pMac->roam.classDStatsInfo.tx_uc_frm_cnt = tlStats.txUCFcnt;

}


void csrRoamReportStatistics(tpAniSirGlobal pMac, tANI_U32 statsMask, 
                             tCsrStatsCallback callback, tANI_U8 staId, void *pContext)
{
   tANI_U8 stats[500];
   tANI_U8 *pStats = NULL;
   tANI_U32 tempMask = 0;
   tANI_U8 counter = 0;
   eHalStatus status = eHAL_STATUS_FAILURE;

   if(!callback)
   {
      smsLog(pMac, LOGE, FL("csrRoamReportStatistics:cannot report callback NULL\n"));
      return;
   }
   if(!statsMask)
   {
      smsLog(pMac, LOGE, FL("csrRoamReportStatistics:cannot report statsMask is 0\n"));
      return;
   }

   pStats = stats;

   tempMask = statsMask;

   while(tempMask)
   {
      if(tempMask & 1)
      {
         //new stats info from PE, fill up the stats strucutres in PMAC
         switch(counter)
         {
         case eCsrSummaryStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:summary stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.summaryStatsInfo, 
                                   sizeof(tCsrSummaryStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy summary stats\n"));
            }
            pStats += sizeof(tCsrSummaryStatsInfo);
            break;

         case eCsrGlobalClassAStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:ClassA stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.classAStatsInfo, 
                                   sizeof(tCsrGlobalClassAStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy ClassA stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassAStatsInfo);

            break;

         case eCsrGlobalClassBStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:ClassB stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.classBStatsInfo, 
                                   sizeof(tCsrGlobalClassBStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy ClassB stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassBStatsInfo);

            break;

         case eCsrGlobalClassCStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:ClassC stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.classCStatsInfo, 
                                   sizeof(tCsrGlobalClassCStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy ClassC stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassCStatsInfo);

            break;

         case eCsrGlobalClassDStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:ClassD stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.classDStatsInfo, 
                                   sizeof(tCsrGlobalClassDStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy ClassD stats\n"));
            }
            pStats += sizeof(tCsrGlobalClassDStatsInfo);

            break;

         case eCsrPerStaStats:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:PerSta stats\n"));
            status = palCopyMemory(pMac->hHdd, pStats, (tANI_U8 *)&pMac->roam.perStaStatsInfo[staId], 
                                   sizeof(tCsrPerStaStatsInfo));
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog( pMac, LOG1, FL("csrRoamReportStatistics:failed to copy PerSta stats\n"));
            }
            pStats += sizeof(tCsrPerStaStatsInfo);

            break;

         default:
            smsLog( pMac, LOG1, FL("csrRoamReportStatistics:unknown stats type\n"));
            break;

         }
      }

      tempMask >>=1;
      counter++;
   }

   callback(stats, pContext );

}


#ifdef FEATURE_WLAN_DIAG_SUPPORT

eHalStatus csrRoamStartDiagLogStatsTimer(tpAniSirGlobal pMac, tANI_U32 interval)
{
   eHalStatus status;
   tANI_U8   index = CSR_MAX_STATISTICS_REQ;
   tANI_BOOLEAN found = FALSE;
   tCsrPeStatsReqInfo *pPeStaEntry = NULL; 
   smsLog(pMac, LOG1, " csrRoamStartStatisticsTimer \n ");
   
   if(( pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_JOINED ||
    pMac->roam.handoffInfo.currState == eCSR_ROAMING_STATE_SCANNING) && interval)
   {
      status = palTimerStart(pMac->hHdd, pMac->roam.hTimerDiagLogStats, interval * PAL_TIMER_TO_MS_UNIT, eANI_BOOLEAN_FALSE);
   }
   else
   {
      smsLog(pMac, LOG1, " csrRoamStartDiagLogStatsTimer: failed to start \n ");
      status = eHAL_STATUS_FAILURE;
   }

   return (status);
}

eHalStatus csrRoamStopDiagLogStatsTimer(tpAniSirGlobal pMac)
{
    return (palTimerStop(pMac->hHdd, pMac->roam.hTimerDiagLogStats));
}

void csrRoamDiagLogStatsTimerHandler(void *pv)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( pv );
   /* Extra check on states to see if this is really needed                 */
   if( (pMac->roam.handoffInfo.currState != eCSR_ROAMING_STATE_JOINED )&&
       ( pMac->roam.curState != eCSR_ROAMING_STATE_SCANNING))
   {
      smsLog(pMac, LOGW, "csrRoamDiagLogStatsTimerHandler: wrong state %d\n", pMac->roam.handoffInfo.currState);

      return;
   }
   //log stats on Diag
   csrRoamDiagStatsLog(pMac);

   csrRoamStartDiagLogStatsTimer(pMac, CSR_DIAG_LOG_STAT_PERIOD);
}

void csrRoamDiagStatsLog(tpAniSirGlobal pMac)
{
   vos_log_statistics_pkt_type *log_ptr = NULL;

   WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_statistics_pkt_type, LOG_WLAN_LINKLAYER_STAT_C);
   if(log_ptr)
   {
      log_ptr->version = 1;
      log_ptr->stat_mask = 63;//ALL
      palCopyMemory(pMac->hHdd, &log_ptr->summaryStats, 
                    &pMac->roam.summaryStatsInfo, sizeof(log_ptr->summaryStats));
      palCopyMemory(pMac->hHdd, &log_ptr->globalClassAStats, 
                    &pMac->roam.classAStatsInfo, sizeof(log_ptr->globalClassAStats));
      palCopyMemory(pMac->hHdd, &log_ptr->globalClassBStats, 
                    &pMac->roam.classBStatsInfo, sizeof(log_ptr->globalClassBStats));
      palCopyMemory(pMac->hHdd, &log_ptr->globalClassCStats, 
                    &pMac->roam.classCStatsInfo, sizeof(log_ptr->globalClassCStats));
      palCopyMemory(pMac->hHdd, &log_ptr->globalClassDStats, 
                    &pMac->roam.classDStatsInfo, sizeof(log_ptr->globalClassDStats));
      palCopyMemory(pMac->hHdd, &log_ptr->perStaStats, 
                    &pMac->roam.perStaStatsInfo, sizeof(log_ptr->perStaStats));

   }
   WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */


eHalStatus csrRoamDeregStatisticsReq(tpAniSirGlobal pMac)
{
   tListElem *pEntry = NULL;
   tListElem *pPrevEntry = NULL;
   tCsrStatsClientReqInfo *pTempStaEntry;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   VOS_STATUS vosStatus;
   pEntry = csrLLPeekHead( &pMac->roam.statsClientReqList, LL_ACCESS_LOCK );

   if(!pEntry)
   {
      //list empty
      smsLog(pMac, LOGW, "csrRoamDeregStatisticsReq: List empty, no request from \
             upper layer client(s)\n");
      return status;
   }

   while( pEntry )
   {
      if(pPrevEntry)
      {
         csrLLRemoveEntry(&pMac->roam.statsClientReqList, pPrevEntry, LL_ACCESS_LOCK);
      }

      pTempStaEntry = GET_BASE_ADDR( pEntry, tCsrStatsClientReqInfo, link );


      pTempStaEntry->pPeStaEntry->numClient--;
      //check if we need to delete the entry from peStatsReqList too
      if(!pTempStaEntry->pPeStaEntry->numClient)
      {
         csrRoamRemoveEntryFromPeStatsReqList(pMac, pTempStaEntry->pPeStaEntry);
      }
      //check if we need to stop the tl stats timer too 
      pMac->roam.tlStatsReqInfo.numClient--;
      if(!pMac->roam.tlStatsReqInfo.numClient)
      {
         if(pMac->roam.tlStatsReqInfo.timerRunning)
         {
            status = palTimerStop(pMac->hHdd, pMac->roam.tlStatsReqInfo.hTlStatsTimer);
            if(!HAL_STATUS_SUCCESS(status))
            {
               smsLog(pMac, LOGE, FL("csrRoamDeregStatisticsReq:cannot stop TlStatsTimer timer\n"));
               //we will continue
            }
         }
         pMac->roam.tlStatsReqInfo.periodicity = 0;
         pMac->roam.tlStatsReqInfo.timerRunning = FALSE;
      }
      vos_timer_stop( &pTempStaEntry->timer );

      // Destroy the vos timer...      
      vosStatus = vos_timer_destroy( &pTempStaEntry->timer );
      if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
      {
         smsLog(pMac, LOGE, FL("csrRoamDeregStatisticsReq:failed to destroy Client req timer\n"));
      }

      
      pPrevEntry = pEntry;
      pEntry = csrLLNext( &pMac->roam.statsClientReqList, pEntry, LL_ACCESS_NOLOCK );
   }
   //the last one
   if(pPrevEntry)
   {
      csrLLRemoveEntry(&pMac->roam.statsClientReqList, pPrevEntry, LL_ACCESS_LOCK);
   }

   return status;
   
}


eHalStatus csrIsFullPowerNeeded( tpAniSirGlobal pMac, tSmeCmd *pCommand, 
                                   tRequestFullPowerReason *pReason,
                                   tANI_BOOLEAN *pfNeedPower )
{
    tANI_BOOLEAN fNeedFullPower = eANI_BOOLEAN_FALSE;
    tRequestFullPowerReason reason = eSME_REASON_OTHER;
    tPmcState pmcState;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if( pfNeedPower )
    {
        *pfNeedPower = eANI_BOOLEAN_FALSE;
    }
	//We only handle CSR commands
	if( !(eSmeCsrCommandMask & pCommand->command) )
	{
		return eHAL_STATUS_SUCCESS;
	}

    //Check PMC state first
    pmcState = pmcGetPmcState( pMac );
    switch( pmcState )
    {
    case REQUEST_IMPS:
    case IMPS:
        if( eSmeCommandScan == pCommand->command )
        {
            switch( pCommand->u.scanCmd.reason )
            {
            case eCsrScanGetResult:
            case eCsrScanBGScanAbort:
            case eCsrScanBGScanEnable:
            case eCsrScanGetScanChnInfo:
                //Internal process, no need for full power
                fNeedFullPower = eANI_BOOLEAN_FALSE;
                break;

            default:
                //Other scans are real scan, ask for power
                fNeedFullPower = eANI_BOOLEAN_TRUE;
                break;
            } //switch
        }
        else
        {
            //ask for power for roam and status change
            fNeedFullPower = eANI_BOOLEAN_TRUE;
        }
        break;

    case REQUEST_BMPS:
    case BMPS:
    case REQUEST_START_UAPSD:
    case REQUEST_STOP_UAPSD:
    case UAPSD:
    //We treat WOWL same as BMPS
    case REQUEST_ENTER_WOWL:
    case REQUEST_EXIT_WOWL:
    case WOWL:
        if( eSmeCommandRoam == pCommand->command )
        {
            tCsrRoamProfile *pProfile = &pCommand->u.roamCmd.roamProfile;
            tScanResultList *pBSSList = (tScanResultList *)pCommand->u.roamCmd.hBSSList;
            tCsrScanResult *pScanResult;
            tListElem *pEntry;

            switch ( pCommand->u.roamCmd.roamReason )
            {
            case eCsrForcedDisassoc:
            case eCsrForcedDisassocMICFailure:
                reason = eSME_LINK_DISCONNECTED_BY_HDD;
                fNeedFullPower = eANI_BOOLEAN_TRUE;
                break;
	        case eCsrSmeIssuedDisassocForHandoff:
            case eCsrForcedDeauth:
            case eCsrHddIssuedReassocToSameAP:
            case eCsrSmeIssuedReassocToSameAP:
                fNeedFullPower = eANI_BOOLEAN_TRUE;
                break;

            default:
                //Check whether the profile is already connected. If so, no need for full power
                //Note: IBSS is ignored for now because we don't support powersave in IBSS
                if ( csrIsConnStateConnectedInfra(pMac) && pBSSList )
                {
                    //Only need to check the first one
                    pEntry = csrLLPeekHead(&pBSSList->List, LL_ACCESS_LOCK);
                    if( pEntry )
                    {
                        pScanResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
                        if( csrIsBssIdEqual( pMac, &pScanResult->Result.BssDescriptor, pMac->roam.pConnectBssDesc ) &&
                            csrIsSsidEqual( pMac, pMac->roam.pConnectBssDesc, 
                                            &pScanResult->Result.BssDescriptor, (tDot11fBeaconIEs *)( pScanResult->Result.pvIes ) ) )
                        {
                            // Check to see if the Auth type has changed in the Profile.  If so, we don't want to Reassociate
                            // with Authenticating first.  To force this, stop the current association (Disassociate) and 
                            // then re 'Join' the AP, wihch will force an Authentication (with the new Auth type) followed by 
                            // a new Association.
                            if(csrIsSameProfile(pMac, &pMac->roam.connectedProfile, pProfile))
                            {
                                if(csrRoamIsSameProfileKeys(pMac, &pMac->roam.connectedProfile, pProfile))
                                {
                                    //Done, eventually, the command reaches eCsrReassocToSelfNoCapChange;
                                    //No need for full power
                                    //Set the flag so the code later can avoid to do the above
                                    //check again.
                                    pCommand->u.roamCmd.fReassocToSelfNoCapChange = eANI_BOOLEAN_TRUE;
                                    break;
                                }
                            }
                        }
                    }
                }
                //If we are here, full power is needed
                fNeedFullPower = eANI_BOOLEAN_TRUE;
                break;
            }
        }
        else if( eSmeCommandWmStatusChange == pCommand->command )
        {
            //need full power for all
            fNeedFullPower = eANI_BOOLEAN_TRUE;
            reason = eSME_LINK_DISCONNECTED_BY_OTHER;
        }
        break;

    case STOPPED:
    case REQUEST_STANDBY:
    case STANDBY:
    case LOW_POWER:
        //We are not supposed to do anything
        smsLog( pMac, LOGE, FL( "  cannot process because PMC is in stopped/standby state %d\n" ), pmcState );
        status = eHAL_STATUS_FAILURE;
        break;

    case FULL_POWER:
    case REQUEST_FULL_POWER:
    default:
        //No need to ask for full power. This has to be FULL_POWER state
        break;

    } //switch

    if( pReason )
    {
        *pReason = reason;
    }
    if( pfNeedPower )
    {
        *pfNeedPower = fNeedFullPower;
    }

    return ( status );
}


static eHalStatus csrRequestFullPower( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fNeedFullPower = eANI_BOOLEAN_FALSE;
    tRequestFullPowerReason reason = eSME_REASON_OTHER;

    status = csrIsFullPowerNeeded( pMac, pCommand, &reason, &fNeedFullPower );

    if( fNeedFullPower && HAL_STATUS_SUCCESS( status ) )
    {
        status = pmcRequestFullPower(pMac, csrFullPowerCallback, pMac, reason);
    }

    return ( status );
}


tSmeCmd *csrGetCommandBuffer( tpAniSirGlobal pMac )
{
    tSmeCmd *pCmd = smeGetCommandBuffer( pMac );

    if( pCmd )
    {
        pMac->roam.sPendingCommands++;
    }

    return ( pCmd );
}


//Return SUCCESS is the command is queued, failed
eHalStatus csrQueueSmeCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fHighPriority )
{
    eHalStatus status;

    //We can call request full power first before putting the command into pending Q
    //because we are holding SME lock at this point.
    status = csrRequestFullPower( pMac, pCommand );
    if( HAL_STATUS_SUCCESS( status ) )
    {
        tANI_BOOLEAN fNoCmdPending;

        //make sure roamCmdPendingList is not empty first
        fNoCmdPending = csrLLIsListEmpty( &pMac->roam.roamCmdPendingList, eANI_BOOLEAN_FALSE );
        if( fNoCmdPending )
        {
            smePushCommand( pMac, pCommand, fHighPriority );
        }
        else
        {
             //Other commands are waiting for PMC callback, queue the new command to the pending Q
            //no list lock is needed since SME lock is held
            if( !fHighPriority )
            {
                csrLLInsertTail( &pMac->roam.roamCmdPendingList, &pCommand->Link, eANI_BOOLEAN_FALSE );
            }
            else {
                csrLLInsertHead( &pMac->roam.roamCmdPendingList, &pCommand->Link, eANI_BOOLEAN_FALSE );
            }
       }
    }
    else if( eHAL_STATUS_PMC_PENDING == status )
    {
        //no list lock is needed since SME lock is held
        if( !fHighPriority )
        {
            csrLLInsertTail( &pMac->roam.roamCmdPendingList, &pCommand->Link, eANI_BOOLEAN_FALSE );
        }
        else {
            csrLLInsertHead( &pMac->roam.roamCmdPendingList, &pCommand->Link, eANI_BOOLEAN_FALSE );
        }
        //Let caller know the command is queue
        status = eHAL_STATUS_SUCCESS;
    }
    else
    {
        //Not to decrease pMac->roam.sPendingCommands here. Caller will decrease it when it 
        //release the command.
        smsLog( pMac, LOGE, FL( "  cannot queue command %d\n" ), pCommand->command );
    }

    return ( status );
}
