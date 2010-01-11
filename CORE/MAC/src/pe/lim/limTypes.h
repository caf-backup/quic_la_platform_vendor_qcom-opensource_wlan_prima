/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limTypes.h contains the definitions used by all
 * all LIM modules.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_TYPES_H
#define __LIM_TYPES_H

#include "wniApi.h"
#include "sirApi.h"
#include "sirCommon.h"
#include "sirMacProtDef.h"
#include "utilsApi.h"
#include "halCommonApi.h"

#include "limApi.h"
#include "limDebug.h"
#include "limSendSmeRspMessages.h"
#include "sysGlobal.h"
#include "dphGlobal.h"
#include "parserApi.h"

#define LINK_TEST_DEFER 1

#define TRACE_EVENT_CNF_TIMER_DEACT        0x6600
#define TRACE_EVENT_CNF_TIMER_ACT          0x6601
#define TRACE_EVENT_AUTH_RSP_TIMER_DEACT   0x6602
#define TRACE_EVENT_AUTH_RSP_TIMER_ACT     0x6603

// MLM message types
#define LIM_MLM_MSG_START           1000
#define LIM_MLM_SCAN_REQ            LIM_MLM_MSG_START
#define LIM_MLM_SCAN_CNF            LIM_MLM_MSG_START + 1
#define LIM_MLM_START_REQ           LIM_MLM_MSG_START + 2
#define LIM_MLM_START_CNF           LIM_MLM_MSG_START + 3
#define LIM_MLM_JOIN_REQ            LIM_MLM_MSG_START + 4
#define LIM_MLM_JOIN_CNF            LIM_MLM_MSG_START + 5
#define LIM_MLM_AUTH_REQ            LIM_MLM_MSG_START + 6
#define LIM_MLM_AUTH_CNF            LIM_MLM_MSG_START + 7
#define LIM_MLM_AUTH_IND            LIM_MLM_MSG_START + 8
#define LIM_MLM_ASSOC_REQ           LIM_MLM_MSG_START + 9
#define LIM_MLM_ASSOC_CNF           LIM_MLM_MSG_START + 10
#define LIM_MLM_ASSOC_IND           LIM_MLM_MSG_START + 11
#define LIM_MLM_DISASSOC_REQ        LIM_MLM_MSG_START + 12
#define LIM_MLM_DISASSOC_CNF        LIM_MLM_MSG_START + 13
#define LIM_MLM_DISASSOC_IND        LIM_MLM_MSG_START + 14
#define LIM_MLM_REASSOC_REQ         LIM_MLM_MSG_START + 15
#define LIM_MLM_REASSOC_CNF         LIM_MLM_MSG_START + 16
#define LIM_MLM_REASSOC_IND         LIM_MLM_MSG_START + 17
#define LIM_MLM_DEAUTH_REQ          LIM_MLM_MSG_START + 18
#define LIM_MLM_DEAUTH_CNF          LIM_MLM_MSG_START + 19
#define LIM_MLM_DEAUTH_IND          LIM_MLM_MSG_START + 20
#define LIM_MLM_TSPEC_REQ           LIM_MLM_MSG_START + 21
#define LIM_MLM_TSPEC_CNF           LIM_MLM_MSG_START + 22
#define LIM_MLM_TSPEC_IND           LIM_MLM_MSG_START + 23
#define LIM_MLM_SETKEYS_REQ         LIM_MLM_MSG_START + 24
#define LIM_MLM_SETKEYS_CNF         LIM_MLM_MSG_START + 25
#define LIM_MLM_LINK_TEST_STOP_REQ  LIM_MLM_MSG_START + 30
#define LIM_MLM_PURGE_STA_IND       LIM_MLM_MSG_START + 31
#define LIM_MLM_ADDBA_REQ           LIM_MLM_MSG_START + 32
#define LIM_MLM_ADDBA_CNF           LIM_MLM_MSG_START + 33
#define LIM_MLM_ADDBA_IND           LIM_MLM_MSG_START + 34
#define LIM_MLM_ADDBA_RSP           LIM_MLM_MSG_START + 35
#define LIM_MLM_DELBA_REQ           LIM_MLM_MSG_START + 36
#define LIM_MLM_DELBA_CNF           LIM_MLM_MSG_START + 37
#define LIM_MLM_DELBA_IND           LIM_MLM_MSG_START + 38
#define LIM_MLM_REMOVEKEY_REQ  LIM_MLM_MSG_START + 39
#define LIM_MLM_REMOVEKEY_CNF  LIM_MLM_MSG_START + 40


#define LIM_HASH_ADD            0
#define LIM_HASH_UPDATE         1

#define LIM_WEP_IN_FC           1
#define LIM_NO_WEP_IN_FC        0

#define LIM_DECRYPT_ICV_FAIL    1

/// Definitions to distinquish between Association/Reassociaton
#define LIM_ASSOC    0
#define LIM_REASSOC  1

/// Minimum Memory blocks require for different scenario
#define LIM_MIN_MEM_ASSOC       4

/// Verifies whether given mac addr matches the CURRENT Bssid
#define IS_CURRENT_BSSID(pMac, addr)  (palEqualMemory(pMac->hHdd, addr, \
                                                                                                pMac->lim.gLimCurrentBssId, \
                                                                                                sizeof(pMac->lim.gLimCurrentBssId)))
/// Verifies whether given addr matches the REASSOC Bssid
#define IS_REASSOC_BSSID(pMac, addr)  (palEqualMemory(pMac->hHdd, addr, \
                                                                                                pMac->lim.gLimReassocBssId, \
                                                                                                sizeof(pMac->lim.gLimReassocBssId)))

#define REQ_TYPE_REGISTRAR                   (0x2)
#define REQ_TYPE_WLAN_MANAGER_REGISTRAR      (0x3)

#define RESP_TYPE_REGISTRAR                  (0x2)
#define RESP_TYPE_ENROLLEE_INFO_ONLY         (0x0)
#define RESP_TYPE_ENROLLEE_OPEN_8021X        (0x1)
#define RESP_TYPE_AP                         (0x3)
#define LIM_TX_FRAMES_THRESHOLD_ON_CHIP       300


// enums used by LIM are as follows

enum eLimDisassocTrigger
{
    eLIM_HOST_DISASSOC,
    eLIM_PEER_ENTITY_DISASSOC,
    eLIM_LINK_MONITORING_DISASSOC,
    eLIM_PROMISCUOUS_MODE_DISASSOC,
    eLIM_HOST_DEAUTH,
    eLIM_PEER_ENTITY_DEAUTH,
    eLIM_LINK_MONITORING_DEAUTH,
    eLIM_JOIN_FAILURE,
    eLIM_REASSOC_REJECT
};

/* Reason code to determine the channel change context while sending 
 * SIR_HAL_CHNL_SWITCH_REQ message to HAL
 */
enum eChannelChangeReasonCodes
{
    LIM_SWITCH_CHANNEL_REASSOC,
    LIM_SWITCH_CHANNEL_JOIN
};

typedef struct sLimAuthRspTimeout
{
    tSirMacAddr    peerMacAddr;
} tLimAuthRspTimeout;

typedef struct sLimMlmStartReq
{
    tSirMacSSid           ssId;
    tSirBssType           bssType;
    tSirMacAddr           bssId;
    tSirMacBeaconInterval beaconPeriod;
    tANI_U8               dtimPeriod;
    tSirMacCfParamSet     cfParamSet;
    tSirMacChanNum        channelNumber;
    tAniCBSecondaryMode   cbMode;
    tANI_U16              atimWindow;
    tSirMacRateSet        rateSet;

    // Parameters reqd for new HAL (message) interface
    tSirNwType            nwType;
    tANI_U8               htCapable;
    tSirMacHTOperatingMode     htOperMode;
    tANI_U8                    dualCTSProtection;
    tANI_U8                    txChannelWidthSet;
} tLimMlmStartReq, *tpLimMlmStartReq;

typedef struct sLimMlmStartCnf
{
    tSirResultCodes resultCode;
} tLimMlmStartCnf, *tpLimMlmStartCnf;

typedef struct sLimMlmScanCnf
{
    tSirResultCodes    resultCode;
    tANI_U16                scanResultLength;
    tSirBssDescription bssDescription[1];
} tLimMlmScanCnf, *tpLimMlmScanCnf;

typedef struct sLimScanResult
{
    tANI_U16                numBssDescriptions;
    tSirBssDescription bssDescription[1];
} tLimScanResult;

typedef struct sLimMlmJoinCnf
{
    tSirResultCodes resultCode;
    tANI_U16 protStatusCode;
} tLimMlmJoinCnf, *tpLimMlmJoinCnf;

typedef struct sLimMlmAssocReq
{
    tSirMacAddr           peerMacAddr;
    tANI_U32                   assocFailureTimeout;
    tANI_U16                   capabilityInfo;
    tSirMacListenInterval listenInterval;
} tLimMlmAssocReq, *tpLimMlmAssocReq;

typedef struct sLimMlmAssocCnf
{
    tSirResultCodes resultCode; //Internal status code.
    tANI_U16 protStatusCode; //Protocol Status code.
} tLimMlmAssocCnf, *tpLimMlmAssocCnf;

typedef struct sLimMlmAssocInd
{
    tSirMacAddr          peerMacAddr;
    tANI_U16                  aid;
    tAniAuthType         authType;
    tSirRSNie            rsnIE;
    tSirMacCapabilityInfo capabilityInfo;
        tAniTitanHtCapabilityInfo titanHtCaps;

    tAniBool                spectrumMgtIndicator;
    tSirMacPowerCapInfo     powerCap;
    tSirSupChnl             supportedChannels;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    tANI_U16                  seqNum;
    tAniBool             wniIndicator;
    tAniBool             bpIndicator;
    tSirBpIndicatorType  bpType;
    tSirNwType           nwType;
    tSirAssocType        assocType; // Indicates whether STA is LB'ed or not
    tSirLoad             load; // Current load on the radio for LB
    tAniSSID             ssId;
    tANI_U32                  numBss; // List received from STA
    tSirNeighborBssInfo  neighborList[1]; // List received from STA
#endif
    /**************** QNE updated - BEGIN **********************/
    tSirMacWscInfo       wscInfo;
    /**************** QNE updated - END   **********************/

} tLimMlmAssocInd, *tpLimMlmAssocInd;

typedef struct sLimMlmReassocReq
{
    tSirMacAddr           peerMacAddr;
    tANI_U32                   reassocFailureTimeout;
    tANI_U16                   capabilityInfo;
    tSirMacListenInterval listenInterval;
} tLimMlmReassocReq, *tpLimMlmReassocReq;

typedef struct sLimMlmReassocCnf
{
    tSirResultCodes resultCode;
    tANI_U16 protStatusCode; //Protocol Status code.
} tLimMlmReassocCnf, *tpLimMlmReassocCnf;

typedef struct sLimMlmReassocInd
{
    tSirMacAddr          peerMacAddr;
    tSirMacAddr          currentApAddr;
    tANI_U16                  aid;
    tAniAuthType         authType;
    tSirRSNie            rsnIE;
    tSirMacCapabilityInfo capabilityInfo;
        tAniTitanHtCapabilityInfo titanHtCaps;

    tAniBool                spectrumMgtIndicator;
    tSirMacPowerCapInfo     powerCap;
    tSirSupChnl             supportedChannels;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    tANI_U16                  seqNum;
    tAniBool             wniIndicator;
    tAniBool             bpIndicator;
    tSirBpIndicatorType  bpType;
    tSirNwType           nwType;
    tSirAssocType        reassocType; // Indicates whether STA is LB'ed or not
    tSirLoad             load; // Current load on the radio for LB
    tAniSSID             ssId;
    tANI_U32                  numBss; // List received from STA
    tSirNeighborBssInfo  neighborList[1]; // List received from STA
#endif
    /**************** QNE updated - BEGIN **********************/
    tSirMacWscInfo       wscInfo;
    /**************** QNE updated - END   **********************/
} tLimMlmReassocInd, *tpLimMlmReassocInd;

typedef struct sLimMlmAuthCnf
{
    tSirMacAddr     peerMacAddr;
    tAniAuthType    authType;
    tSirResultCodes resultCode;
    tANI_U16        protStatusCode;
} tLimMlmAuthCnf, *tpLimMlmAuthCnf;

typedef struct sLimMlmAuthInd
{
    tSirMacAddr    peerMacAddr;
    tAniAuthType   authType;
} tLimMlmAuthInd, *tpLimMlmAuthInd;

typedef struct sLimMlmDeauthReq
{
    tSirMacAddr peerMacAddr;
    tANI_U16         reasonCode;
    tANI_U16         deauthTrigger;
    tANI_U16         aid;
} tLimMlmDeauthReq, *tpLimMlmDeauthReq;

typedef struct sLimMlmDeauthCnf
{
    tSirMacAddr     peerMacAddr;
    tSirResultCodes resultCode;
    tANI_U16             deauthTrigger;
    tANI_U16         aid;
} tLimMlmDeauthCnf, *tpLimMLmDeauthCnf;

typedef struct sLimMlmDeauthInd
{
    tSirMacAddr peerMacAddr;
    tANI_U16         reasonCode;
    tANI_U16         deauthTrigger;
    tANI_U16         aid;
} tLimMlmDeauthInd, *tpLimMlmDeauthInd;

typedef struct sLimMlmDisassocReq
{
    tSirMacAddr peerMacAddr;
    tANI_U16         reasonCode;
    tANI_U16         disassocTrigger;
    tANI_U16         aid;
} tLimMlmDisassocReq, *tpLimMlmDisassocReq;

typedef struct sLimMlmDisassocCnf
{
    tSirMacAddr     peerMacAddr;
    tSirResultCodes resultCode;
    tANI_U16             disassocTrigger;
    tANI_U16             aid;
} tLimMlmDisassocCnf, *tpLimMlmDisassocCnf;

typedef struct sLimMlmDisassocInd
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             reasonCode;
    tANI_U16             disassocTrigger;
    tANI_U16             aid;
} tLimMlmDisassocInd, *tpLimMlmDisassocInd;

typedef struct sLimMlmPurgeStaReq
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             aid;
} tLimMlmPurgeStaReq, *tpLimMlmPurgeStaReq;

typedef struct sLimMlmPurgeStaInd
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             reasonCode;
    tANI_U16             purgeTrigger;
    tANI_U16             aid;
} tLimMlmPurgeStaInd, *tpLimMlmPurgeStaInd;

typedef struct sLimMlmSetKeysReq
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             aid;
    tAniEdType      edType;    // Encryption/Decryption type
    tANI_U8              numKeys;
    tSirKeys        key[SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS];
} tLimMlmSetKeysReq, *tpLimMlmSetKeysReq;

typedef struct sLimMlmSetKeysCnf
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             resultCode;
    tANI_U16             aid;
} tLimMlmSetKeysCnf, *tpLimMlmSetKeysCnf;

typedef struct sLimMlmRemoveKeyReq
{
    tSirMacAddr     peerMacAddr;
    tAniEdType      edType;    // Encryption/Decryption type
    tANI_U8          wepType; //STATIC / DYNAMIC specifier
    tANI_U8          keyId; //Key Id To be removed.
    tANI_BOOLEAN unicast;
} tLimMlmRemoveKeyReq, *tpLimMlmRemoveKeyReq;

typedef struct sLimMlmRemoveKeyCnf
{
    tSirMacAddr     peerMacAddr;
    tANI_U16             resultCode;
} tLimMlmRemoveKeyCnf, *tpLimMlmRemoveKeyCnf;


typedef struct sLimMlmResetReq
{
    tSirMacAddr macAddr;
    tANI_U8        performCleanup;
} tLimMlmResetReq, *tpLimMlmResetReq;

typedef struct sLimMlmResetCnf
{
    tSirResultCodes resultCode;
} tLimMlmResetCnf, *tpLimMlmResetCnf;


typedef struct sLimMlmLinkTestStopReq
{
    tSirMacAddr    peerMacAddr;
#ifdef ANI_PRODUCT_TYPE_AP
    tANI_U16             aid;
#endif
} tLimMlmLinkTestStopReq, *tpLimMlmLinkTestStopReq;


//
// Block ACK related MLME data structures
//

typedef struct sLimMlmAddBAReq
{

  // ADDBA recipient
  tSirMacAddr peerMacAddr;

  // ADDBA Action Frame dialog token
  tANI_U8 baDialogToken;

  // ADDBA requested for TID
  tANI_U8 baTID;

  // BA policy
  // 0 - Delayed BA (Not supported)
  // 1 - Immediate BA
  tANI_U8 baPolicy;

  // BA buffer size - (0..127) max size MSDU's
  tANI_U16 baBufferSize;

  // BA timeout in TU's
  // 0 means no timeout will occur
  tANI_U16 baTimeout;

  // ADDBA failure timeout in TU's
  // Greater than or equal to 1
  tANI_U16 addBAFailureTimeout;

  // BA Starting Sequence Number
  tANI_U16 baSSN;

} tLimMlmAddBAReq, *tpLimMlmAddBAReq;

typedef struct sLimMlmAddBACnf
{

  // ADDBA recipient
  tSirMacAddr peerMacAddr;

  // ADDBA Action Frame dialog token
  tANI_U8 baDialogToken;

  // ADDBA requested for TID
  tANI_U8 baTID;

  // BA status code
  tSirMacStatusCodes addBAResultCode;

  // BA policy
  // 0 - Delayed BA (Not supported)
  // 1 - Immediate BA
  tANI_U8 baPolicy;

  // BA buffer size - (0..127) max size MSDU's
  tANI_U16 baBufferSize;

  // BA timeout in TU's
  // 0 means no timeout will occur
  tANI_U16 baTimeout;

  // ADDBA direction
  // 1 - Originator
  // 0 - Recipient
  tANI_U8 baDirection;

} tLimMlmAddBACnf, *tpLimMlmAddBACnf;

typedef struct sLimMlmAddBAInd
{

  // ADDBA recipient
  tSirMacAddr peerMacAddr;

  // ADDBA Action Frame dialog token
  tANI_U8 baDialogToken;

  // ADDBA requested for TID
  tANI_U8 baTID;

  // BA policy
  // 0 - Delayed BA (Not supported)
  // 1 - Immediate BA
  tANI_U8 baPolicy;

  // BA buffer size - (0..127) max size MSDU's
  tANI_U16 baBufferSize;

  // BA timeout in TU's
  // 0 means no timeout will occur
  tANI_U16 baTimeout;

} tLimMlmAddBAInd, *tpLimMlmAddBAInd;

typedef struct sLimMlmAddBARsp
{

  // ADDBA recipient
  tSirMacAddr peerMacAddr;

  // ADDBA Action Frame dialog token
  tANI_U8 baDialogToken;

  // ADDBA requested for TID
  tANI_U8 baTID;

  // BA status code
  tSirMacStatusCodes addBAResultCode;

  // BA policy
  // 0 - Delayed BA (Not supported)
  // 1 - Immediate BA
  tANI_U8 baPolicy;
  
  // BA buffer size - (0..127) max size MSDU's
  tANI_U16 baBufferSize;

  // BA timeout in TU's
  // 0 means no timeout will occur
  tANI_U16 baTimeout;

  //reserved for alignment
  tANI_U8 rsvd[2];
} tLimMlmAddBARsp, *tpLimMlmAddBARsp;

//
// NOTE - Overloading DELBA IND and DELBA CNF
// to use the same data structure as DELBA REQ
// as the parameters do not vary too much.
//
typedef struct sLimMlmDelBAReq
{

  // ADDBA recipient
  tSirMacAddr peerMacAddr;

  // DELBA direction
  // 1 - Originator
  // 0 - Recipient
  tANI_U8 baDirection;

  // DELBA requested for TID
  tANI_U8 baTID;

  // DELBA reason code
  tSirMacReasonCodes delBAReasonCode;

} tLimMlmDelBAReq, *tpLimMlmDelBAReq, tLimMlmDelBAInd, *tpLimMlmDelBAInd, tLimMlmDelBACnf, *tpLimMlmDelBACnf;

// Function templates

tANI_BOOLEAN limProcessSmeReqMessages(tpAniSirGlobal, tpSirMsgQ);
void limProcessMlmReqMessages(tpAniSirGlobal, tpSirMsgQ);
void limProcessMlmRspMessages(tpAniSirGlobal, tANI_U32, tANI_U32 *);
void limProcessLmmMessages(tpAniSirGlobal, tANI_U32, tANI_U32 *);
void limProcessSmeDelBssRsp( tpAniSirGlobal , tANI_U32);

void limGetRandomBssid(tpAniSirGlobal pMac ,tANI_U8 *data);

// Function to handle CB CFG parameter updates
void handleCBCFGChange( tpAniSirGlobal pMac, tANI_U32 cfgId );

// Function to handle HT and HT IE CFG parameter intializations
void handleHTCapabilityandHTInfo(struct sAniSirGlobal *pMac);

// Function to handle CFG parameter updates
void limHandleCFGparamUpdate(tpAniSirGlobal, tANI_U32);

// Function to apply CFG parameters before join/reassoc/start BSS
void limApplyConfiguration(tpAniSirGlobal);

void limSetCfgProtection(tpAniSirGlobal pMac);

// Function to Initialize MLM state machine on STA
void limInitMlm(tpAniSirGlobal);

// Function to cleanup MLM state machine
void limCleanupMlm(tpAniSirGlobal);

// Function to cleanup LMM state machine
void limCleanupLmm(tpAniSirGlobal);


// Management frame handling functions
void limProcessBeaconFrame(tpAniSirGlobal, tANI_U32 *);
void limProcessProbeReqFrame(tpAniSirGlobal, tANI_U32 *);
void limProcessProbeRspFrame(tpAniSirGlobal, tANI_U32 *);
void limProcessAuthFrame(tpAniSirGlobal, tANI_U32 *);

#ifdef ANI_PRODUCT_TYPE_AP
void limProcessAssocReqFrame(tpAniSirGlobal, tANI_U32 *, tANI_U8);
#endif
void limProcessAssocRspFrame(tpAniSirGlobal, tANI_U32 *, tANI_U8);
void limProcessDisassocFrame(tpAniSirGlobal, tANI_U32 *);
void limProcessDeauthFrame(tpAniSirGlobal, tANI_U32 *);
void limProcessActionFrame(tpAniSirGlobal, tANI_U32 *);


tSirRetStatus limPopulateBD(tpAniSirGlobal, tANI_U8*, tANI_U8, tANI_U8, tSirMacAddr);
tSirRetStatus limSendProbeReqMgmtFrame(tpAniSirGlobal, tSirMacSSid *,
                                       tSirMacAddr, tANI_U8);
void limSendProbeRspMgmtFrame(tpAniSirGlobal, tSirMacAddr, tpAniSSID, short, tANI_U8);
void limSendAuthMgmtFrame(tpAniSirGlobal, tSirMacAuthFrameBody *, tSirMacAddr, tANI_U8);
void limSendAssocReqMgmtFrame(tpAniSirGlobal, tLimMlmAssocReq *);
void limSendReassocReqMgmtFrame(tpAniSirGlobal, tLimMlmReassocReq *);
void limSendDeltsReqActionFrame(tpAniSirGlobal pMac, tSirMacAddr peer, tANI_U8 wmmTspecPresent, 
                                tSirMacTSInfo * pTsinfo, tSirMacTspecIE * pTspecIe);
void limSendAddtsReqActionFrame(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
                          tSirAddtsReqInfo *addts);
void limSendAddtsRspActionFrame(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,
                           tANI_U16 statusCode, tSirAddtsReqInfo *addts, tSirMacScheduleIE *pSchedule);

#ifdef ANI_PRODUCT_TYPE_AP
void limSendAssocRspMgmtFrame(tpAniSirGlobal, tANI_U16, tANI_U16, tSirMacAddr, tANI_U8, tpDphHashNode pSta);
#endif
void limSendNullDataFrame(tpAniSirGlobal, tpDphHashNode);
void limSendDisassocMgmtFrame(tpAniSirGlobal, tANI_U16, tSirMacAddr);
void limSendDeauthMgmtFrame(tpAniSirGlobal, tANI_U16, tSirMacAddr);

void limContinueChannelScan(tpAniSirGlobal);
tSirResultCodes limMlmAddBss(tpAniSirGlobal, tLimMlmStartReq *);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
tSirRetStatus limSendChannelSwitchMgmtFrame(tpAniSirGlobal, tSirMacAddr, tANI_U8, tANI_U8, tANI_U8);
#endif

// Algorithms & Link Monitoring related functions
tSirBackgroundScanMode limSelectsBackgroundScanMode(tpAniSirGlobal);
void limTriggerBackgroundScan(tpAniSirGlobal);
void limAbortBackgroundScan(tpAniSirGlobal);

/// Function that handles heartbeat failure
void limHandleHeartBeatFailure(tpAniSirGlobal);

/// Function that triggers link tear down with AP upon HB failure
void limTearDownLinkWithAp(tpAniSirGlobal);

#ifdef ANI_PRODUCT_TYPE_AP
/// Function that performs periodic release of AIDs
void limReleaseAIDHandler(tpAniSirGlobal);

/// Function that performs periodic cleanup of Pre-auth contexts
void limPreAuthClnupHandler(tpAniSirGlobal);

/// Function that processes CF-poll response message from SCH
void limHandleCFpollRsp(tANI_U32);

/// Function that processes PS-poll message from PMM
void limHandlePSpoll(tANI_U32);
#endif

/// Function that sends keep alive message to peer(s)
void limSendKeepAliveToPeer(tpAniSirGlobal);

/// Function that processes Max retries interrupt from TFP
void limHandleMaxRetriesInterrupt(tANI_U32);

/// Function that processes messages deferred during Learn mode
void limProcessDeferredMessageQueue(tpAniSirGlobal);

/// Function that defers the messages received
tANI_U32 limDeferMsg(tpAniSirGlobal, tSirMsgQ *);

/// Function that sets system into scan mode
void limSetScanMode(tpAniSirGlobal pMac);

/// Function that Switches the Channel and sets the CB Mode 
void limSetChannel(tpAniSirGlobal pMac, tANI_U32 titanHtcap, tANI_U8 channel);

/// Function that completes channel scan
void limCompleteMlmScan(tpAniSirGlobal, tSirResultCodes);

#ifdef ANI_SUPPORT_11H
/// Function that sends Measurement Report action frame
tSirRetStatus limSendMeasReportFrame(tpAniSirGlobal, tpSirMacMeasReqActionFrame, tSirMacAddr);

/// Function that sends TPC Report action frame
tSirRetStatus limSendTpcReportFrame(tpAniSirGlobal, tpSirMacTpcReqActionFrame, tSirMacAddr);
#endif

/// Function that sends TPC Request action frame
void limSendTpcRequestFrame(tpAniSirGlobal, tSirMacAddr);

// Function(s) to handle responses received from HAL
void limProcessMlmAddBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
#ifdef ANI_PRODUCT_TYPE_AP
void limProcessApMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessApMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessApMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
#endif
void limProcessStaMlmAddStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessStaMlmDelStaRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessStaMlmDelBssRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );
void limProcessMlmSetKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );


#ifdef GEN4_SCAN
// Function to process SIR_HAL_INIT_SCAN_RSP message
void limProcessInitScanRsp(tpAniSirGlobal,  void * );

// Function to process SIR_HAL_START_SCAN_RSP message
void limProcessStartScanRsp(tpAniSirGlobal,  void * );

// Function to process SIR_HAL_END_SCAN_RSP message
void limProcessEndScanRsp(tpAniSirGlobal, void * );

// Function to process SIR_HAL_FINISH_SCAN_RSP message
void limProcessFinishScanRsp(tpAniSirGlobal,  void * );

// Function to process SIR_HAL_SWITCH_CHANNEL_RSP message
void limProcessSwitchChannelRsp(tpAniSirGlobal pMac,  void * );
  
void limSendHalInitScanReq( tpAniSirGlobal, tLimLimHalScanState);
void limSendHalStartScanReq( tpAniSirGlobal, tANI_U8, tLimLimHalScanState);
void limSendHalEndScanReq( tpAniSirGlobal, tANI_U8, tLimLimHalScanState);
void limSendHalFinishScanReq( tpAniSirGlobal, tLimLimHalScanState);

void limContinuePostChannelScan(tpAniSirGlobal pMac);
void limContinueChannelLearn( tpAniSirGlobal );
#endif // GEN4_SCAN

tSirRetStatus limSendAddBAReq( tpAniSirGlobal pMac,
    tpLimMlmAddBAReq pMlmAddBAReq );

tSirRetStatus limSendAddBARsp( tpAniSirGlobal pMac,
    tpLimMlmAddBARsp pMlmAddBARsp );

tSirRetStatus limSendDelBAInd( tpAniSirGlobal pMac,
    tpLimMlmDelBAReq pMlmDelBAReq );

tSirRetStatus limSendSMPowerStateFrame( tpAniSirGlobal pMac, 
      tSirMacAddr peer, tSirMacHTMIMOPowerSaveState State );

void limProcessMlmHalAddBARsp( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ );

void limProcessMlmHalBADeleteInd( tpAniSirGlobal pMac,
    tpSirMsgQ limMsgQ );

void limProcessMlmRemoveKeyRsp( tpAniSirGlobal pMac, tpSirMsgQ limMsgQ );

void limProcessSetMimoRsp(tpAniSirGlobal pMac, tpSirMsgQ limMsg);

void limProcessLearnIntervalTimeout(tpAniSirGlobal pMac);

// Inline functions

/**
 * limPostSmeMessage()
 *
 *FUNCTION:
 * This function is called by limProcessMlmMessages(). In this
 * function MLM sub-module invokes MLM ind/cnf primitives.
 *
 *LOGIC:
 * Initially MLM makes an SME function call to invoke MLM ind/cnf
 * primitive. In future this can be enhanced to 'post' messages to SME.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac      Pointer to Global MAC structure
 * @param msgType   Indicates the MLM primitive message type
 * @param *pMsgBuf  A pointer to the MLM message buffer
 *
 * @return None
 */
static inline void
limPostSmeMessage(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirMsgQ msg;
    msg.type = (tANI_U16)msgType;
    msg.bodyptr = pMsgBuf;
    msg.bodyval = 0;
    if (msgType > eWNI_SME_MSG_TYPES_BEGIN)
        limProcessSmeReqMessages(pMac, &msg);
    else
        limProcessMlmRspMessages(pMac, msgType, pMsgBuf);
} /*** end limPostSmeMessage() ***/

/**
 * limPostMlmMessage()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages(). In this
 * function SME invokes MLME primitives.
 *
 *PARAMS:
 *
 *LOGIC:
 * Initially SME makes an MLM function call to invoke MLM primitive.
 * In future this can be enhanced to 'post' messages to MLM.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param pMac      Pointer to Global MAC structure
 * @param msgType   Indicates the MLM primitive message type
 * @param *pMsgBuf  A pointer to the MLM message buffer
 *
 * @return None
 */
static inline void
limPostMlmMessage(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirMsgQ msg;
    msg.type = (tANI_U16) msgType;
    msg.bodyptr = pMsgBuf;
    msg.bodyval = 0;
    limProcessMlmReqMessages(pMac, &msg);
} /*** end limPostMlmMessage() ***/



/**
 * limGetCurrentScanChannel()
 *
 *FUNCTION:
 * This function is called in various places to get current channel
 * number being scanned.
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
 * @param  pMac      Pointer to Global MAC structure
 * @return Channel number
 */
static inline tANI_U8
limGetCurrentScanChannel(tpAniSirGlobal pMac)
{
    tANI_U8 *pChanNum = pMac->lim.gpLimMlmScanReq->channelList.channelNumber;

    return (*(pChanNum + pMac->lim.gLimCurrentScanChannelId));
} /*** end limGetCurrentScanChannel() ***/



/**
 * limGetIElenFromBssDescription()
 *
 *FUNCTION:
 * This function is called in various places to get IE length
 * from tSirBssDescription structure
 * number being scanned.
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
 * @param     pBssDescr
 * @return    Total IE length
 */

static inline tANI_U16
limGetIElenFromBssDescription(tpSirBssDescription pBssDescr)
{
    if (!pBssDescr)
        return 0;

    return ((tANI_U16) (pBssDescr->length + sizeof(tANI_U16) +
                   sizeof(tANI_U32) - sizeof(tSirBssDescription)));
} /*** end limGetIElenFromBssDescription() ***/

#endif /* __LIM_TYPES_H */

