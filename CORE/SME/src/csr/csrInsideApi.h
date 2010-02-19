/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrInsideApi.h
  
    Define interface only used by CSR.
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
 
   ========================================================================== */
#ifndef CSR_INSIDE_API_H__
#define CSR_INSIDE_API_H__


#include "csrSupport.h"
#include "smeInside.h"
#include "vos_nvitem.h"

#define CSR_PASSIVE_MAX_CHANNEL_TIME   110
#define CSR_PASSIVE_MIN_CHANNEL_TIME   60

#define CSR_ACTIVE_MAX_CHANNEL_TIME    40
#define CSR_ACTIVE_MIN_CHANNEL_TIME    20

#define CSR_MAX_NUM_SUPPORTED_CHANNELS 55

//This number minus 1 means the number of times a channel is scanned before a BSS is remvoed from
//cache scan result
#define CSR_AGING_COUNT     3   
//The following defines are used by palTimer
//This is used for palTimer when request to imps fails
#define CSR_IDLE_SCAN_WAIT_TIME     (1 * PAL_TIMER_TO_SEC_UNIT)     //1 second 
//This is used for palTimer when imps ps is disabled
//This number shall not be smaller than 5-6 seconds in general because a full scan may take 3-4 seconds
#define CSR_IDLE_SCAN_NO_PS_INTERVAL     (10 * PAL_TIMER_TO_SEC_UNIT)     //10 second 
#define CSR_IDLE_SCAN_NO_PS_INTERVAL_MIN (5 * PAL_TIMER_TO_SEC_UNIT)
#define CSR_SCAN_GET_RESULT_INTERVAL    (5 * PAL_TIMER_TO_SEC_UNIT)     //5 seconds
#define CSR_MIC_ERROR_TIMEOUT  (60 * PAL_TIMER_TO_SEC_UNIT)     //60 seconds
#define CSR_TKIP_COUNTER_MEASURE_TIMEOUT  (60 * PAL_TIMER_TO_SEC_UNIT)     //60 seconds
#define CSR_SCAN_RESULT_AGING_INTERVAL    (5 * PAL_TIMER_TO_SEC_UNIT)     //5 seconds
//the following defines are NOT used by palTimer
#define CSR_SCAN_AGING_TIME_NOT_CONNECT_NO_PS 50     //50 seconds
#define CSR_SCAN_AGING_TIME_NOT_CONNECT_W_PS 300     //300 seconds
#define CSR_SCAN_AGING_TIME_CONNECT_NO_PS 150        //150 seconds
#define CSR_SCAN_AGING_TIME_CONNECT_W_PS 600         //600 seconds
#define CSR_JOIN_FAILURE_TIMEOUT_DEFAULT ( 3000 )
#define CSR_JOIN_FAILURE_TIMEOUT_MIN   (300)  //minimal value
//These are going against the signed RSSI (tANI_S8) so it is between -+127
#define CSR_BEST_RSSI_VALUE         (-30)   //RSSI >= this is in CAT4
#define CSR_DEFAULT_RSSI_DB_GAP     30 //every 30 dbm for one category
#define CSR_BSS_CAP_VALUE_NONE  0    //not much value
#define CSR_BSS_CAP_VALUE_1     1    //something better
#define CSR_DEFAULT_ROAMING_TIME 10   //10 seconds
#define CSR_ROAM_MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define CSR_ROAM_MAX(X, Y)  ((X) > (Y) ? (X) : (Y))


typedef enum 
{
	eCsrNextScanNothing,
	eCsrNextLostLinkScan1Success,
	eCsrNextLostLinkScan1Failed,
	eCsrNextLostLinkScan2Success,
	eCsrNextLostLinkScan2Failed,
	eCsrNextLostLinkScan3Success,
    eCsrNexteScanForSsidSuccess,
	eCsrNextLostLinkScan3Failed,
    eCsrNext11dScan1Failure,
	eCsrNext11dScan1Success,
	eCsrNext11dScan2Failure, 
	eCsrNext11dScan2Success,
	eCsrNext11dScanComplete,
    eCsrNexteScanForSsidFailure,
    eCsrNextIdleScanComplete,
    eCsrNextCapChangeScanComplete,

}eCsrScanCompleteNextCommand;

typedef enum  
{
    eCsrJoinSuccess, 
    eCsrJoinFailure,
    eCsrReassocSuccess,
    eCsrReassocFailure, 
    eCsrNothingToJoin, 
    eCsrStartIbssSuccess,
    eCsrStartIbssFailure,
    eCsrSilentlyStopRoaming,
    eCsrSilentlyStopRoamingSaveState,
    
}eCsrRoamCompleteResult;

typedef struct tagScanReqParam
{
    tANI_U8 bReturnAfter1stMatch;
    tANI_U8 fUniqueResult;
    tANI_U8 freshScan;
    tANI_U8 hiddenSsid;
    tANI_U8 reserved;
}tScanReqParam;

typedef struct tagCsrScanResult
{
    tListElem Link;
    tANI_S32 AgingCount;    //This BSS is removed when it reaches 0 or less
    tANI_U32 preferValue;   //The bigger the number, the better the BSS. This value override capValue
    tANI_U32 capValue;  //The biggger the better. This value is in use only if we have equal preferValue
    //This member must be the last in the structure because the end of tSirBssDescription (inside) is an
    //    array with nonknown size at this time
    
    eCsrEncryptionType ucEncryptionType; //Preferred Encryption type that matched with profile.
    eCsrEncryptionType mcEncryptionType; 
    eCsrAuthType authType; //Preferred auth type that matched with the profile.

    tCsrScanResultInfo Result;
}tCsrScanResult;

typedef struct
{
    tDblLinkList List;
    tListElem *pCurEntry;
}tScanResultList;




#define CSR_IS_ROAM_REASON( pCmd, reason ) ( (reason) == (pCmd)->roamCmd.roamReason )
#define CSR_IS_BETTER_PREFER_VALUE(v1, v2)   ((v1) > (v2))
#define CSR_IS_EQUAL_PREFER_VALUE(v1, v2)   ((v1) == (v2))
#define CSR_IS_BETTER_CAP_VALUE(v1, v2)     ((v1) > (v2))
#define CSR_IS_ENC_TYPE_STATIC( encType ) ( ( eCSR_ENCRYPT_TYPE_NONE == (encType) ) || \
                                            ( eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == (encType) ) || \
                                            ( eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == (encType) ) )
#define CSR_IS_WAIT_FOR_KEY( pMac ) ( CSR_IS_ROAM_JOINED( (pMac) ) && CSR_IS_ROAM_SUBSTATE_WAITFORKEY( (pMac) ) )
//WIFI has a test case for not using HT rates with TKIP as encryption
//We may need to add WEP but for now, TKIP only.
#define CSR_IS_11n_ALLOWED( encType ) ( eCSR_ENCRYPT_TYPE_TKIP != (encType) )


eCsrRoamState csrRoamStateChange( tpAniSirGlobal pMac, eCsrRoamState NewRoamState );
eHalStatus csrScanningStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf );
void csrRoamingStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf );
void csrRoamJoinedStateMsgProcessor( tpAniSirGlobal pMac, void *pMsgBuf );
tANI_BOOLEAN csrScanComplete( tpAniSirGlobal pMac, tSirSmeScanRsp *pScanRsp );
void csrReleaseCommandRoam(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReleaseCommandScan(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrReleaseCommandWmStatusChange(tpAniSirGlobal pMac, tSmeCmd *pCommand);
//pIes2 can be NULL
tANI_BOOLEAN csrIsDuplicateBssDescription( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc1, 
                                           tDot11fBeaconIEs *pIes1,
                                           tSirBssDescription *pSirBssDesc2, tDot11fBeaconIEs *pIes2 );
eHalStatus csrRoamSaveConnectedBssDesc( tpAniSirGlobal pMac, tSirBssDescription *pBssDesc );
tANI_BOOLEAN csrIsNetworkTypeEqual( tSirBssDescription *pSirBssDesc1, tSirBssDescription *pSirBssDesc2 );
eHalStatus csrScanSmeScanResponse( tpAniSirGlobal pMac, void *pMsgBuf );
/*
   Prepare a filter base on a profile for parsing the scan results.
   Upon successful return, caller MUST call csrFreeScanFilter on 
   pScanFilter when it is done with the filter.
*/
eHalStatus csrRoamPrepareFilterFromProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tCsrScanResultFilter *pScanFilter);
eHalStatus csrRoamCopyProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pDstProfile, tCsrRoamProfile *pSrcProfile);
eHalStatus csrRoamStart(tpAniSirGlobal pMac);
void csrRoamStop(tpAniSirGlobal pMac);
void csrRoamStartMICFailureTimer(tpAniSirGlobal pMac);
void csrRoamStopMICFailureTimer(tpAniSirGlobal pMac);
void csrRoamStartTKIPCounterMeasureTimer(tpAniSirGlobal pMac);
void csrRoamStopTKIPCounterMeasureTimer(tpAniSirGlobal pMac);

eHalStatus csrScanOpen(tHalHandle hHal);
eHalStatus csrScanClose(tHalHandle hHal);
eHalStatus csrScanRequestLostLink1( tpAniSirGlobal pMac );
eHalStatus csrScanRequestLostLink2( tpAniSirGlobal pMac );
eHalStatus csrScanRequestLostLink3( tpAniSirGlobal pMac );
eHalStatus csrScanHandleFailedLostlink1(tpAniSirGlobal pMac);
eHalStatus csrScanHandleFailedLostlink2(tpAniSirGlobal pMac);
eHalStatus csrScanHandleFailedLostlink3(tpAniSirGlobal pMac);
tCsrScanResult *csrScanAppendBssDescription( tpAniSirGlobal pMac, 
                                             tSirBssDescription *pSirBssDescription,
                                             tDot11fBeaconIEs *pIes);
void csrScanCallCallback(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrScanStatus scanStatus);
eHalStatus csrScanCopyRequest(tpAniSirGlobal pMac, tCsrScanRequest *pDstReq, tCsrScanRequest *pSrcReq);
eHalStatus csrScanFreeRequest(tpAniSirGlobal pMac, tCsrScanRequest *pReq);
eHalStatus csrScanCopyResultList(tpAniSirGlobal pMac, tScanResultHandle hIn, tScanResultHandle *phResult);
void csrInitBGScanChannelList(tpAniSirGlobal pMac);
eHalStatus csrScanForSSID(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tANI_U32 roamId);
eHalStatus csrScanForCapabilityChange(tpAniSirGlobal pMac, tSirSmeApNewCaps *pNewCaps);
eHalStatus csrScanStartGetResultTimer(tpAniSirGlobal pMac);
eHalStatus csrScanStopGetResultTimer(tpAniSirGlobal pMac);
eHalStatus csrScanStartResultAgingTimer(tpAniSirGlobal pMac);
eHalStatus csrScanStopResultAgingTimer(tpAniSirGlobal pMac);
eHalStatus csrScanBGScanEnable(tpAniSirGlobal pMac);
eHalStatus csrScanStartIdleScanTimer(tpAniSirGlobal pMac, tANI_U32 interval);
eHalStatus csrScanStopIdleScanTimer(tpAniSirGlobal pMac);
eHalStatus csrScanStartIdleScan(tpAniSirGlobal pMac);
//Param: pTimeInterval -- Caller allocated memory in return, if failed, to specify the nxt time interval for 
//idle scan timer interval
//Return: Not success -- meaning it cannot start IMPS, caller needs to start a timer for idle scan
eHalStatus csrScanTriggerIdleScan(tpAniSirGlobal pMac, tANI_U32 *pTimeInterval);
void csrScanCancelIdleScan(tpAniSirGlobal pMac);
void csrScanStopTimers(tpAniSirGlobal pMac);
//This function will remove scan commands that are not related to association or IBSS
tANI_BOOLEAN csrScanRemoveNotRoamingScanCommand(tpAniSirGlobal pMac);
//To remove fresh scan commands from the pending queue
tANI_BOOLEAN csrScanRemoveFreshScanCommand(tpAniSirGlobal pMac);
eHalStatus csrScanAbortMacScan(tpAniSirGlobal pMac);
eHalStatus csrScanGetScanChannelInfo(tpAniSirGlobal pMac);
//To age out scan results base. tSmeGetScanChnRsp is a pointer returned by LIM that
//has the information regarding scanned channels.
//The logic is that whenever CSR add a BSS to scan result, it set the age count to
//a value. This function deduct the age count if channelId matches the BSS' channelId
//The BSS is remove if the count reaches 0.
eHalStatus csrScanAgeResults(tpAniSirGlobal pMac, tSmeGetScanChnRsp *pScanChnInfo);
tANI_BOOLEAN csrLearnCountryInformation( tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc,
                                         tDot11fBeaconIEs *pIes);
void csrApplyCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce );
void csrSetCfgScanControlList( tpAniSirGlobal pMac, tANI_U8 *countryCode, tCsrChannel *pChannelList  );
void csrReinitScanCmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csrFreeScanResultEntry( tpAniSirGlobal pMac, tCsrScanResult *pResult );

eHalStatus csrRoamCallCallback(tpAniSirGlobal pMac, tCsrRoamInfo *pRoamInfo, tANI_U32 roamId, eRoamCmdStatus u1, eCsrRoamResult u2);
eHalStatus csrRoamIssueConnect(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, tScanResultHandle hBSSList, 
                               eCsrRoamReason reason, tANI_U32 roamId, 
                               tANI_BOOLEAN fImediate, tANI_BOOLEAN fClearScan);
eHalStatus csrRoamIssueReassoc(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile,
                               tCsrRoamModifyProfileFields *pModProfileFields,
                               eCsrRoamReason reason, tANI_U32 roamId, tANI_BOOLEAN fImediate);
void csrRoamComplete( tpAniSirGlobal pMac, eCsrRoamCompleteResult Result, void *Context );
eHalStatus csrRoamIssueSetContextReq( tpAniSirGlobal pMac, eCsrEncryptionType EncryptType, tSirBssDescription *pBssDescription,
                                tSirMacAddr *bssId, tANI_BOOLEAN addKey,
                                 tANI_BOOLEAN fUnicast, tAniKeyDirection aniKeyDirection, 
                                 tANI_U8 keyId, tANI_U16 keyLength, 
                                 tANI_U8 *pKey, tANI_U8 paeRole );
eHalStatus csrRoamProcessDisassociate( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fMICFailure );
eHalStatus csrRoamProcessDeauth( tpAniSirGlobal pMac, tSmeCmd *pCommand );
eHalStatus csrRoamSaveConnectedInfomation(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile, 
										  tSirBssDescription *pSirBssDesc, tDot11fBeaconIEs *pIes);
void csrRoamCheckForLinkStatusChange( tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg );
void csrRoamStatsRspProcessor(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg);
eHalStatus csrRoamIssueStartIbss( tpAniSirGlobal pMac, tCsrRoamIbssParams *pParam, tCsrRoamProfile *pProfile, tSirBssDescription *pBssDesc, tANI_U32 roamId );
eHalStatus csrRoamIssueStopBss( tpAniSirGlobal pMac, eCsrRoamSubState NewSubstate );
tANI_BOOLEAN csrIsSameProfile(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pProfile1, tCsrRoamProfile *pProfile2);
tANI_BOOLEAN csrIsRoamCommandWaiting(tpAniSirGlobal pMac);
eRoamCmdStatus csrGetRoamCompleteStatus(tpAniSirGlobal pMac);
//pBand can be NULL if caller doesn't need to get it
eCsrCfgDot11Mode csrRoamGetPhyModeBandForIBSS( tpAniSirGlobal pMac, eCsrPhyMode phyModeIn, tANI_U8 operationChn, eCsrBand *pBand );
eHalStatus csrRoamIssueDisassociateCmd( tpAniSirGlobal pMac, eCsrRoamDisconnectReason reason );
eHalStatus csrRoamDisconnectInternal(tpAniSirGlobal pMac, eCsrRoamDisconnectReason reason);
//pCommand may be NULL
void csrRoamRemoveDuplicateCommand(tpAniSirGlobal pMac, tSmeCmd *pCommand, eCsrRoamReason eRoamReason);
                                 
eHalStatus csrSendJoinReqMsg( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription, 
                              tCsrRoamProfile *pProfile, tDot11fBeaconIEs *pIes );
eHalStatus csrSendMBDisassocReqMsg( tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U16 reasonCode );
eHalStatus csrSendMBDeauthReqMsg( tpAniSirGlobal pMac, tSirMacAddr bssId, tANI_U16 reasonCode );
eHalStatus csrSendMBDisassocCnfMsg( tpAniSirGlobal pMac, tpSirSmeDisassocInd pDisassocInd );
eHalStatus csrSendMBDeauthCnfMsg( tpAniSirGlobal pMac, tpSirSmeDeauthInd pDeauthInd );
eHalStatus csrSendMBStartBssReqMsg( tpAniSirGlobal pMac, eCsrRoamBssType bssType, tCsrRoamIbssParams *pParam );
eHalStatus csrSendMBStopBssReqMsg( tpAniSirGlobal pMac, tANI_U16 reasonCode );
eHalStatus csrSendSmeReassocReqMsg( tpAniSirGlobal pMac, tSirBssDescription *pBssDescription, 
                                    tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile );

tANI_BOOLEAN csrIsMacAddressEqual( tpAniSirGlobal pMac, tCsrBssid *pMacAddr1, tCsrBssid *pMacAddr2 );
//Caller should put the BSS' ssid to fiedl bssSsid when comparing SSID for a BSS.
tANI_BOOLEAN csrIsSsidMatch( tHalHandle hHal, tANI_U8 *ssid1, tANI_U8 ssid1Len, tANI_U8 *bssSsid, 
                            tANI_U8 bssSsidLen, tANI_BOOLEAN fSsidRequired );
tANI_BOOLEAN csrIsPhyModeMatch( tpAniSirGlobal pMac, tANI_U32 phyMode,
                                    tSirBssDescription *pSirBssDesc, tCsrRoamProfile *pProfile,
                                    eCsrCfgDot11Mode *pReturnCfgDot11Mode,
                                    tDot11fBeaconIEs *pIes);
tANI_BOOLEAN csrRoamIsChannelValid( tpAniSirGlobal pMac, tANI_U8 channel );
tANI_BOOLEAN csrIs40MhzChannel(tpAniSirGlobal pMac, tANI_U8 chnId);

//pNumChan is a caller allocated space with the sizeof pChannels
eHalStatus csrGetCfgValidChannels(tpAniSirGlobal pMac, tANI_U8 *pChannels, tANI_U32 *pNumChan);
void csrRoamCcmCfgSetCallback(tHalHandle hHal, tANI_S32 result);
void csrScanCcmCfgSetCallback(tHalHandle hHal, tANI_S32 result);

//To free the last roaming profile
void csrFreeRoamProfile(tpAniSirGlobal pMac);
void csrFreeConnectBssDesc(tpAniSirGlobal pMac);
eHalStatus csrMoveBssToHeadFromBSSID(tpAniSirGlobal pMac, tCsrBssid *bssid, tScanResultHandle hScanResult);
tANI_BOOLEAN csrCheckPSReady(void *pv);
void csrFullPowerCallback(void *pv, eHalStatus status);
//to free memory allocated inside the profile structure
void csrReleaseProfile(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile);
//To free memory allocated inside scanFilter
void csrFreeScanFilter(tpAniSirGlobal pMac, tCsrScanResultFilter *pScanFilter);
eCsrCfgDot11Mode csrGetCfgDot11ModeFromCsrPhyMode(eCsrPhyMode phyMode, tANI_BOOLEAN fProprietary);
tANI_U32 csrTranslateToWNICfgDot11Mode(tpAniSirGlobal pMac, eCsrCfgDot11Mode csrDot11Mode);
void csrSaveChannelPowerForBand( tpAniSirGlobal pMac, tANI_BOOLEAN fPopulate5GBand );
void csrApplyChannelPowerCountryInfo( tpAniSirGlobal pMac, tCsrChannel *pChannelList, tANI_U8 *countryCode);
void csrAssignRssiForCategory(tpAniSirGlobal pMac, tANI_U8 catOffset);
tANI_BOOLEAN csrIsMacAddressZero( tpAniSirGlobal pMac, tCsrBssid *pMacAddr );
tANI_BOOLEAN csrIsMacAddressBroadcast( tpAniSirGlobal pMac, tCsrBssid *pMacAddr );
eHalStatus csrRoamRemoveConnectedBssFromScanCache(tpAniSirGlobal pMac);
eHalStatus csrRoamStartRoaming(tpAniSirGlobal pMac, eCsrRoamingReason roamingReason);
//return a boolean to indicate whether roaming completed or continue.
tANI_BOOLEAN csrRoamCompleteRoaming(tpAniSirGlobal pMac, tANI_BOOLEAN fForce, eCsrRoamResult roamResult);
void csrRoamCompletion(tpAniSirGlobal pMac, tCsrRoamInfo *pRoamInfo, tSmeCmd *pCommand, eCsrRoamResult roamResult, tANI_BOOLEAN fSuccess);
void csrRoamCancelRoaming(tpAniSirGlobal pMac);
void csrResetCountryInformation( tpAniSirGlobal pMac, tANI_BOOLEAN fForce );
void csrResetPMKIDCandidateList( tpAniSirGlobal pMac );

void csrSaveToChannelPower2G_5G( tpAniSirGlobal pMac, tANI_U32 tableSize, tSirMacChanInfo *channelTable );
eHalStatus csrRoamSetKey( tpAniSirGlobal pMac, tCsrRoamSetKey *pSetKey, tANI_U32 roamId );
eHalStatus csrRoamIssueRemoveKeyCommand( tpAniSirGlobal pMac, tCsrRoamRemoveKey *pRemoveKey, tANI_U32 roamId );
//Get the list of the base channels to scan for passively 11d info
eHalStatus csrScanGetSupportedChannels( tpAniSirGlobal pMac );
//To check whether a country code matches the one in the IE
//Only check the first two characters, ignoring in/outdoor
//pCountry -- caller allocated buffer contain the country code that is checking against
//the one in pIes. It can be NULL.
//This function always return TRUE if 11d support is not turned on.
tANI_BOOLEAN csrMatchCountryCode( tpAniSirGlobal pMac, tANI_U8 *pCountry, tDot11fBeaconIEs *pIes );

/* ---------------------------------------------------------------------------
    \fn csrGetCountryCode
    \brief this function is to get the country code current being used
    \param pBuf - Caller allocated buffer with at least 3 bytes, upon success return, this has the country code
    \param pbLen - Caller allocated, as input, it indicates the length of pBuf. Upon success return,
    this contains the length of the data in pBuf
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrGetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pBuf, tANI_U8 *pbLen);

/* ---------------------------------------------------------------------------
    \fn csrSetCountryCode
    \brief this function is to set the country code so channel/power setting matches the countrycode and
    the domain it belongs to.
    \param pCountry - Caller allocated buffer with at least 3 bytes specifying the country code
    \param pfRestartNeeded - pointer to a caller allocated space. Upon successful return, it indicates whether 
    a restart is needed to apply the change
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrSetCountryCode(tpAniSirGlobal pMac, tANI_U8 *pCountry, tANI_BOOLEAN *pfRestartNeeded);

/* ---------------------------------------------------------------------------
    \fn csrResetCountryCodeInformation
    \brief this function is to reset the country code current being used back to EEPROM default
    this includes channel list and power setting.
    \param pfRestartNeeded - pointer to a caller allocated space. Upon successful return, it indicates whether 
    a restart is needed to apply the change
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrResetCountryCodeInformation(tpAniSirGlobal pMac, tANI_BOOLEAN *pfRestartNeeded);

/* ---------------------------------------------------------------------------
    \fn csrGetSupportedCountryCode
    \brief this function is to get a list of the country code current being supported
    \param pBuf - Caller allocated buffer with at least 3 bytes, upon success return, 
    this has the country code list. 3 bytes for each country code. This may be NULL if
    caller wants to know the needed bytes.
    \param pbLen - Caller allocated, as input, it indicates the length of pBuf. Upon success return,
    this contains the length of the data in pBuf
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrGetSupportedCountryCode(tpAniSirGlobal pMac, tANI_U8 *pBuf, tANI_U32 *pbLen);

/* ---------------------------------------------------------------------------
    \fn csrSetRegulatoryDomain
    \brief this function is to set the current regulatory domain.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \param domainId - indicate the domain (defined in the driver) needs to set to.  
    See eRegDomainId for definition
    \param pfRestartNeeded - pointer to a caller allocated space. Upon successful return, it indicates whether 
    a restart is needed to apply the change
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrSetRegulatoryDomain(tpAniSirGlobal pMac, v_REGDOMAIN_t domainId, tANI_BOOLEAN *pfRestartNeeded);

/* ---------------------------------------------------------------------------
    \fn csrGetCurrentRegulatoryDomain
    \brief this function is to get the current regulatory domain.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \return eRegDomainId     
  -------------------------------------------------------------------------------*/
v_REGDOMAIN_t csrGetCurrentRegulatoryDomain(tpAniSirGlobal pMac);

/* ---------------------------------------------------------------------------
    \fn csrGetRegulatoryDomainForCountry
    \brief this function is to get the regulatory domain for a country.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \param pCountry - Caller allocated buffer with at least 3 bytes specifying the country code
    \param pDomainId - Caller allocated buffer to get the return domain ID upon success return. Can be NULL.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrGetRegulatoryDomainForCountry(tpAniSirGlobal pMac, tANI_U8 *pCountry, v_REGDOMAIN_t *pDomainId);




//some support functions
tANI_BOOLEAN csrIs11dSupported(tpAniSirGlobal pMac);
tANI_BOOLEAN csrIs11hSupported(tpAniSirGlobal pMac);
tANI_BOOLEAN csrIs11eSupported(tpAniSirGlobal pMac);
tANI_BOOLEAN csrIsWmmSupported(tpAniSirGlobal pMac); 
//Upper layer to get the list of the base channels to scan for passively 11d info from csr
eHalStatus csrScanGetBaseChannels( tpAniSirGlobal pMac, tCsrChannelInfo * pChannelInfo );
//Return SUCCESS is the command is queued, failed
eHalStatus csrQueueSmeCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand, tANI_BOOLEAN fHighPriority );
tSmeCmd *csrGetCommandBuffer( tpAniSirGlobal pMac );

#ifdef FEATURE_WLAN_DIAG_SUPPORT

//Security
#define WLAN_SECURITY_EVENT_SET_PTK_REQ     1
#define WLAN_SECURITY_EVENT_SET_PTK_RSP     2
#define WLAN_SECURITY_EVENT_SET_GTK_REQ     3
#define WLAN_SECURITY_EVENT_SET_GTK_RSP     4
#define WLAN_SECURITY_EVENT_REMOVE_KEY_REQ  5
#define WLAN_SECURITY_EVENT_REMOVE_KEY_RSP  6
#define WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND  7
#define WLAN_SECURITY_EVENT_PMKID_UPDATE    8
#define WLAN_SECURITY_EVENT_MIC_ERROR       9   

#define AUTH_OPEN       0
#define AUTH_SHARED     1
#define AUTH_WPA_EAP    2
#define AUTH_WPA_PSK    3
#define AUTH_WPA2_EAP   4
#define AUTH_WPA2_PSK   5

#define ENC_MODE_OPEN   0
#define ENC_MODE_WEP40  1
#define ENC_MODE_WEP104 2
#define ENC_MODE_TKIP   3
#define ENC_MODE_AES    4

#define NO_MATCH    0
#define MATCH       1

#define WLAN_SECURITY_STATUS_SUCCESS        0
#define WLAN_SECURITY_STATUS_FAILURE        1

//Scan
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ     1
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP     2
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ    3
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_RSP    4
#define WLAN_SCAN_EVENT_HO_SCAN_REQ         5
#define WLAN_SCAN_EVENT_HO_SCAN_RSP         6

#define WLAN_SCAN_STATUS_SUCCESS        0
#define WLAN_SCAN_STATUS_FAILURE        1
#define WLAN_SCAN_STATUS_ABORT          2

//Ibss
#define WLAN_IBSS_EVENT_START_IBSS_REQ      0
#define WLAN_IBSS_EVENT_START_IBSS_RSP      1
#define WLAN_IBSS_EVENT_JOIN_IBSS_REQ       2
#define WLAN_IBSS_EVENT_JOIN_IBSS_RSP       3
#define WLAN_IBSS_EVENT_COALESCING          4
#define WLAN_IBSS_EVENT_PEER_JOIN           5
#define WLAN_IBSS_EVENT_PEER_LEAVE          6
#define WLAN_IBSS_EVENT_STOP_REQ            7
#define WLAN_IBSS_EVENT_STOP_RSP            8

#define AUTO_PICK       0
#define SPECIFIED       1

#define WLAN_IBSS_STATUS_SUCCESS        0
#define WLAN_IBSS_STATUS_FAILURE        1

//11d
#define WLAN_80211D_EVENT_COUNTRY_SET   0
#define WLAN_80211D_EVENT_RESET         1

#define WLAN_80211D_DISABLED         0
#define WLAN_80211D_SUPPORT_MULTI_DOMAIN     1
#define WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN     2

int diagAuthTypeFromCSRType(eCsrAuthType authType);
int diagEncTypeFromCSRType(eCsrEncryptionType encType);
#endif //#ifdef FEATURE_WLAN_DIAG_SUPPORT

#endif

