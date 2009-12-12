/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limUtils.h contains the utility definitions
 * LIM uses.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef __LIM_UTILS_H
#define __LIM_UTILS_H

#include "sirApi.h"
#include "sirDebug.h"
#include "cfgApi.h"

#include "limTypes.h"
#include "limScanResultUtils.h"
#include "limTimerUtils.h"
#include "limTrace.h"


#define LIM_STA_ID_MASK                        0x00FF
#define LIM_AID_MASK                              0xC000

// classifier ID is coded as 0-3: tsid, 4-5:direction
#define LIM_MAKE_CLSID(tsid, dir) (((tsid) & 0x0F) | (((dir) & 0x03) << 4))

#define LIM_SET_STA_BA_STATE(pSta, tid, newVal) \
{\
    pSta->baState = ((pSta->baState | (0x3 << tid*2)) & ((newVal << tid*2) | ~(0x3 << tid*2)));\
}

#define LIM_GET_STA_BA_STATE(pSta, tid, pCurVal)\
{\
    *pCurVal = ((pSta->baState >> tid*2) & 0x3);\
}

// LIM utilility functions

void limGetBssidFromBD(tpAniSirGlobal, tpHalBufDesc , tANI_U8 *, tANI_U32 *);
char * limMlmStateStr(tLimMlmStates state);
char * limSmeStateStr(tLimSmeStates state);
char * limMsgStr(tANI_U32 msgType);
char * limResultCodeStr(tSirResultCodes resultCode);
char* limDot11ModeStr(tpAniSirGlobal pMac, tANI_U8 dot11Mode);
char* limStaOpRateModeStr(tStaRateMode opRateMode);
void limPrintMlmState(tpAniSirGlobal pMac, tANI_U16 logLevel, tLimMlmStates state);
void limPrintSmeState(tpAniSirGlobal pMac, tANI_U16 logLevel, tLimSmeStates state);
void limPrintMsgName(tpAniSirGlobal pMac, tANI_U16 logLevel, tANI_U32 msgType);
void limPrintMsgInfo(tpAniSirGlobal pMac, tANI_U16 logLevel, tSirMsgQ *msg);


tANI_U32            limPostMsgApiNoWait(tpAniSirGlobal, tSirMsgQ *);
tANI_U8           limIsAddrBC(tSirMacAddr);
tANI_U8           limIsGroupAddr(tSirMacAddr);

// check for type of scan allowed
tANI_U8 limActiveScanAllowed(tpAniSirGlobal, tANI_U8);

// AID pool management functions
void    limInitAIDpool(tpAniSirGlobal);
tANI_U16     limAssignAID(tpAniSirGlobal);
#if (WNI_POLARIS_FW_PRODUCT == AP)
void limDecideApProtection(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr,  tpUpdateBeaconParams pBeaconParams);
void limUpdateShortPreamble(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, tpUpdateBeaconParams pBeaconParams);
void limUpdateShortSlotTime(tpAniSirGlobal pMac, tSirMacAddr peerMacAddr, tpUpdateBeaconParams pBeaconParams);
void limEnableOverlap11gProtection(tpAniSirGlobal pMac, tpUpdateBeaconParams pBeaconParams, tpSirMacMgmtHdr pMh);
void limUpdateOverlapStaParam(tpAniSirGlobal pMac, tSirMacAddr bssId, tpLimProtStaParams pStaParams);
/*
 * The below 'product' check tobe removed if 'Association' is
 * allowed in IBSS.
 */
void    limReleaseAID(tpAniSirGlobal, tANI_U16);
void    limAddAIDtoTBRList(tpAniSirGlobal, tANI_U16);

// LIM informs WSM that radar is detected
void limDetectRadar(tpAniSirGlobal, tANI_U32 *);
#endif
extern tSirRetStatus limEnable11aProtection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnable11gProtection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHtProtectionFrom11g(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHT20Protection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHTNonGfProtection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHtRifsProtection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHTLsigTxopProtection(tpAniSirGlobal pMac, tANI_U8 enable, tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableShortPreamble(tpAniSirGlobal pMac, tANI_U8 enable, tpUpdateBeaconParams pBeaconParams);
extern tSirRetStatus limEnableHtOBSSProtection (tpAniSirGlobal pMac, tANI_U8 enable,  tANI_U8 overlap, tpUpdateBeaconParams pBeaconParams);
void limDecideStaProtection(tpAniSirGlobal pMac, tSchBeaconStruct beaconStruct, tpUpdateBeaconParams pBeaconParams);
void limDecideStaProtectionOnAssoc(tpAniSirGlobal pMac, tSchBeaconStruct beaconStruct);
void limUpdateStaRunTimeHTSwitchChnlParams(tpAniSirGlobal pMac, tDot11fIEHTInfo * pHTInfo, tANI_U8 bssIdx);

// Print MAC address utility function
void    limPrintMacAddr(tpAniSirGlobal, tSirMacAddr, tANI_U8);



// Deferred Message Queue read/write
tANI_U8 limWriteDeferredMsgQ(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
tSirMsgQ* limReadDeferredMsgQ(tpAniSirGlobal pMac);
void limHandleDeferMsgError(tpAniSirGlobal pMac, tpSirMsgQ pLimMsg);

// Deferred Message Queue Reset
void limResetDeferredMsgQ(tpAniSirGlobal pMac);

tSirRetStatus limHalMmhPostMsgApi(tpAniSirGlobal, tSirMsgQ*, tANI_U8);

#if defined(ANI_PRODUCT_TYPE_AP)
void limHandleUpdateOlbcCache(tpAniSirGlobal pMac);
#endif

tANI_U8 limIsNullSsid( tSirMacSSid *pSsid );

void limProcessAddtsRspTimeout(tpAniSirGlobal pMac, tANI_U32 param);

// 11h Support
#ifdef ANI_PRODUCT_TYPE_AP
tANI_U32 computeChannelSwitchCount(tpAniSirGlobal, tANI_U32);
#endif
void limStopTxAndSwitchChannel(tpAniSirGlobal pMac);
void limProcessChannelSwitchTimeout(tpAniSirGlobal);
tSirRetStatus limStartChannelSwitch(tpAniSirGlobal pMac);
void limUpdateChannelSwitch(tpAniSirGlobal, tpSirProbeRespBeacon);
void limProcessQuietTimeout(tpAniSirGlobal);
void limProcessQuietBssTimeout(tpAniSirGlobal);
void limStartQuietTimer(tpAniSirGlobal pMac);
void limUpdateQuietIEFromBeacon(tpAniSirGlobal, tDot11fIEQuiet *);
void limGetHtCbAdminState(tpAniSirGlobal pMac, tDot11fIEHTCaps htCaps, tANI_U8 * titanHtCaps);
void limGetHtCbOpState(tpAniSirGlobal pMac, tDot11fIEHTInfo htInfo, tANI_U8 * titanHtCaps);
void limSwitchPrimaryChannel(tpAniSirGlobal, tANI_U8);
void limSwitchPrimarySecondaryChannel(tpAniSirGlobal, tANI_U8, tAniCBSecondaryMode);
tAniBool limTriggerBackgroundScanDuringQuietBss(tpAniSirGlobal);
void limUpdateStaRunTimeHTSwtichChnlParams(tpAniSirGlobal pMac, tDot11fIEHTInfo *pRcvdHTInfo, tANI_U8 bssIdx);
void limUpdateStaRunTimeHTCapability(tpAniSirGlobal pMac, tDot11fIEHTCaps *pHTCaps);
void limUpdateStaRunTimeHTInfo(struct sAniSirGlobal *pMac, tDot11fIEHTInfo *pRcvdHTInfo);
void limCancelDot11hChannelSwitch(tpAniSirGlobal pMac);
void limCancelDot11hQuiet(tpAniSirGlobal pMac);
tAniBool limIsChannelValidForChannelSwitch(tpAniSirGlobal pMac, tANI_U8 channel);
void limFrameTransmissionControl(tpAniSirGlobal pMac, tLimQuietTxMode type, tLimControlTx mode);
tSirRetStatus limRestorePreChannelSwitchState(tpAniSirGlobal pMac);
tSirRetStatus limRestorePreQuietState(tpAniSirGlobal pMac);

void limPrepareFor11hChannelSwitch(tpAniSirGlobal pMac);


static inline tSirRFBand limGetRFBand(tANI_U8 channel)
{
    if ((channel >= SIR_11A_CHANNEL_BEGIN) &&
        (channel <= SIR_11A_CHANNEL_END))
        return SIR_BAND_5_GHZ;

    if ((channel >= SIR_11B_CHANNEL_BEGIN) &&
        (channel <= SIR_11B_CHANNEL_END))
        return SIR_BAND_2_4_GHZ;

    return SIR_BAND_UNKNOWN;
}


static inline tSirRetStatus
limGetMgmtStaid(tpAniSirGlobal pMac, tANI_U16 *staid)
{
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
        *staid = 1;
    else if (pMac->lim.gLimSystemRole == eLIM_STA_ROLE)
        *staid = 0;
    else
        return eSIR_FAILURE;

    return eSIR_SUCCESS;
}

static inline tANI_U8
limIsSystemInSetMimopsState(tpAniSirGlobal pMac)
{
    if (pMac->lim.gLimMlmState == eLIM_MLM_WT_SET_MIMOPS_STATE)
        return true;
    return false;
}
        
static inline tANI_U8
 isEnteringMimoPS(tSirMacHTMIMOPowerSaveState curState, tSirMacHTMIMOPowerSaveState newState)
 {
    if (curState == eSIR_HT_MIMO_PS_NO_LIMIT &&
        (newState == eSIR_HT_MIMO_PS_DYNAMIC ||newState == eSIR_HT_MIMO_PS_STATIC))
        return TRUE;
    return FALSE;
}

/// ANI peer station count management and associated actions
void limUtilCountStaAdd(tpAniSirGlobal pMac, tpDphHashNode pSta);
void limUtilCountStaDel(tpAniSirGlobal pMac, tpDphHashNode pSta);
tANI_U8 limGetHTCapability( tpAniSirGlobal, tANI_U32 );
void limTxComplete( tHalHandle hHal, void *pData );

/**********Admit Control***************************************/

//callback function for HAL to issue DelTS request to PE.
//This function will be registered with HAL for callback when TSPEC inactivity timer fires.

void limProcessDelTsInd(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
tSirRetStatus limProcessHalIndMessages(tpAniSirGlobal pMac, tANI_U32 mesgId, void *mesgParam );
tSirRetStatus limValidateDeltsReq(tpAniSirGlobal pMac, tpSirDeltsReq pDeltsReq, tSirMacAddr peerMacAddr);
/**********************************************************/

//callback function registration to HAL for any indication.
void limRegisterHalIndCallBack(tpAniSirGlobal pMac);
void limPktFree (
    tpAniSirGlobal  pMac,
    eFrameType      frmType,
    tANI_U32        *pBD,
    void            *body);



void limGetBDfromRxPacket(tpAniSirGlobal pMac, void *body, tANI_U32 **pBD);

/**
 * \brief Given a base(X) and power(Y), this API will return
 * the result of base raised to power - (X ^ Y)
 *
 * \sa utilsPowerXY
 *
 * \param base Base value
 *
 * \param power Base raised to this Power value
 *
 * \return Result of X^Y
 *
 */
static inline tANI_U32 utilsPowerXY( tANI_U16 base, tANI_U16 power )
{
tANI_U32 result = 1, i;

  for( i = 0; i < power; i++ )
    result *= base;

  return result;
}



tSirRetStatus limPostMlmAddBAReq( tpAniSirGlobal pMac,
    tpDphHashNode pStaDs,
    tANI_U8 tid, tANI_U16 startingSeqNum);
tSirRetStatus limPostMlmAddBARsp( tpAniSirGlobal pMac,
    tSirMacAddr peerMacAddr,
    tSirMacStatusCodes baStatusCode,
    tANI_U8 baDialogToken,
    tANI_U8 baTID,
    tANI_U8 baPolicy,
    tANI_U16 baBufferSize,
    tANI_U16 baTimeout);
tSirRetStatus limPostMlmDelBAReq( tpAniSirGlobal pMac,
    tpDphHashNode pSta,
    tANI_U8 baDirection,
    tANI_U8 baTID,
    tSirMacReasonCodes baReasonCode );
tSirRetStatus limPostMsgAddBAReq( tpAniSirGlobal pMac,
    tpDphHashNode pSta,
    tANI_U8 baDialogToken,
    tANI_U8 baTID,
    tANI_U8 baPolicy,
    tANI_U16 baBufferSize,
    tANI_U16 baTimeout,
    tANI_U16 baSSN,
    tANI_U8 baDirection );
tSirRetStatus limPostMsgDelBAInd( tpAniSirGlobal pMac,
    tpDphHashNode pSta,
    tANI_U8 baTID,
    tANI_U8 baDirection );

tSirRetStatus limPostSMStateUpdate(tpAniSirGlobal pMac,
    tANI_U16 StaIdx, 
    tSirMacHTMIMOPowerSaveState MIMOPSState);

void limDeleteStaContext(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
void limProcessAddBaInd(tpAniSirGlobal pMac, tpSirMsgQ limMsg);
void limDelAllBASessions(tpAniSirGlobal pMac);
void limDeleteDialogueTokenList(tpAniSirGlobal pMac);
tSirRetStatus limSearchAndDeleteDialogueToken(tpAniSirGlobal pMac, tANI_U8 token, tANI_U16 assocId, tANI_U16 tid);
void limRessetScanChannelInfo(tpAniSirGlobal pMac);
void limAddScanChannelInfo(tpAniSirGlobal pMac, tANI_U8 channelId);

tANI_U8 limGetChannelFromBeacon(tpAniSirGlobal pMac, tpSchBeaconStruct pBeacon);
tSirNwType limGetNwType(tpAniSirGlobal pMac, tANI_U8 channelNum, tANI_U32 type, tpSchBeaconStruct pBeacon);
void limSetTspecUapsdMask(tpAniSirGlobal pMac, tSirMacTSInfo *pTsInfo, tANI_U32 action);


#ifdef FEATURE_WLAN_DIAG_SUPPORT

typedef enum
{
    WLAN_PE_DIAG_SCAN_REQ_EVENT = 0,
    WLAN_PE_DIAG_SCAN_ABORT_IND_EVENT,
    WLAN_PE_DIAG_SCAN_RSP_EVENT,
    WLAN_PE_DIAG_JOIN_REQ_EVENT,
    WLAN_PE_DIAG_JOIN_RSP_EVENT,
    WLAN_PE_DIAG_SETCONTEXT_REQ_EVENT,  
    WLAN_PE_DIAG_SETCONTEXT_RSP_EVENT, 
    WLAN_PE_DIAG_REASSOC_REQ_EVENT,
    WLAN_PE_DIAG_REASSOC_RSP_EVENT,
    WLAN_PE_DIAG_AUTH_REQ_EVENT,
    WLAN_PE_DIAG_AUTH_RSP_EVENT,
    WLAN_PE_DIAG_DISASSOC_REQ_EVENT,
    WLAN_PE_DIAG_DISASSOC_RSP_EVENT,
    WLAN_PE_DIAG_DISASSOC_IND_EVENT,
    WLAN_PE_DIAG_DISASSOC_CNF_EVENT,
    WLAN_PE_DIAG_DEAUTH_REQ_EVENT,
    WLAN_PE_DIAG_DEAUTH_RSP_EVENT,
    WLAN_PE_DIAG_DEAUTH_IND_EVENT,
    WLAN_PE_DIAG_START_BSS_REQ_EVENT,
    WLAN_PE_DIAG_START_BSS_RSP_EVENT,
    WLAN_PE_DIAG_AUTH_IND_EVENT,
    WLAN_PE_DIAG_ASSOC_IND_EVENT,
    WLAN_PE_DIAG_ASSOC_CNF_EVENT,
    WLAN_PE_DIAG_REASSOC_IND_EVENT,
    WLAN_PE_DIAG_SWITCH_CHL_REQ_EVENT,
    WLAN_PE_DIAG_SWITCH_CHL_RSP_EVENT,
    WLAN_PE_DIAG_STOP_BSS_REQ_EVENT,
    WLAN_PE_DIAG_STOP_BSS_RSP_EVENT,
    WLAN_PE_DIAG_DEAUTH_CNF_EVENT,
    WLAN_PE_DIAG_ADDTS_REQ_EVENT,
    WLAN_PE_DIAG_ADDTS_RSP_EVENT,
    WLAN_PE_DIAG_DELTS_REQ_EVENT,
    WLAN_PE_DIAG_DELTS_RSP_EVENT,
    WLAN_PE_DIAG_DELTS_IND_EVENT,
    WLAN_PE_DIAG_ENTER_BMPS_REQ_EVENT,
    WLAN_PE_DIAG_ENTER_BMPS_RSP_EVENT,
    WLAN_PE_DIAG_EXIT_BMPS_REQ_EVENT,
    WLAN_PE_DIAG_EXIT_BMPS_RSP_EVENT,
    WLAN_PE_DIAG_EXIT_BMPS_IND_EVENT,
    WLAN_PE_DIAG_ENTER_IMPS_REQ_EVENT,
    WLAN_PE_DIAG_ENTER_IMPS_RSP_EVENT,
    WLAN_PE_DIAG_EXIT_IMPS_REQ_EVENT,
    WLAN_PE_DIAG_EXIT_IMPS_RSP_EVENT,
    WLAN_PE_DIAG_ENTER_UAPSD_REQ_EVENT,
    WLAN_PE_DIAG_ENTER_UAPSD_RSP_EVENT,
    WLAN_PE_DIAG_EXIT_UAPSD_REQ_EVENT,
    WLAN_PE_DIAG_EXIT_UAPSD_RSP_EVENT,
    WLAN_PE_DIAG_WOWL_ADD_BCAST_PTRN_EVENT,
    WLAN_PE_DIAG_WOWL_DEL_BCAST_PTRN_EVENT,
    WLAN_PE_DIAG_ENTER_WOWL_REQ_EVENT,
    WLAN_PE_DIAG_ENTER_WOWL_RSP_EVENT,
    WLAN_PE_DIAG_EXIT_WOWL_REQ_EVENT,
    WLAN_PE_DIAG_EXIT_WOWL_RSP_EVENT,
    WLAN_PE_DIAG_HAL_ADDBA_REQ_EVENT,
    WLAN_PE_DIAG_HAL_ADDBA_RSP_EVENT,
    WLAN_PE_DIAG_HAL_DELBA_IND_EVENT,
}WLAN_PE_DIAG_EVENT_TYPE;

void limDiagEventReport(tpAniSirGlobal pMac, tANI_U16 eventType, tSirMacAddr bssid, tANI_U16 status, tANI_U16 reasonCode);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

#endif /* __LIM_UTILS_H */
