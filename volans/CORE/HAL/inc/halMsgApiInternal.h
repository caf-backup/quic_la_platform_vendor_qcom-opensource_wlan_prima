/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halMsgApiInternal.h:  Provide HAL APIs.
 * Author:    Susan Tsao
 * Date:      02/06/2006
 *
 * --------------------------------------------------------------------------
 */
#ifndef _HALMSGAPI_INTERNAL_H_
#define _HALMSGAPI_INTERNAL_H_

#include "palTypes.h"   // eAniBoolean
#include "halTypes.h"
#include "halMsgApi.h"
#include "halHddApis.h" // tHalMacStartParameters
#include "aniGlobal.h"
#include "halRxp.h"
#include "sirParams.h"

void halMsg_AddSta(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpAddStaParams msg, eAniBoolean isFreeable);
void halMsg_DelSta(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpDeleteStaParams msg);
void halMsg_AddBss(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpAddBssParams msg);
void halMsg_DelBss(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpDeleteBssParams msg);
void halMsg_UpdateUapsd(tpAniSirGlobal pMac, tpUpdateUapsdParams msg);
#ifdef HAL_BCAST_STA_PER_BSS
void halMsg_AddBcastSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tpAddBssParams msg);
void halMsg_DelBcastSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tpDeleteBssParams msg);
#endif
void halMsg_InitScan(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpInitScanParams msg);
void halMsg_StartScan(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpStartScanParams msg);
void halMsg_FinishScan(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpFinishScanParams msg);
void halMsg_EndScan(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpEndScanParams msg);
void halMsg_ChannelSwitch(tpAniSirGlobal pMac, tpSwitchChannelParams param);
void halMsg_SendBeacon(tpAniSirGlobal pMac, tpSendbeaconParams msg);
#ifdef WLAN_SOFTAP_FEATURE
void halMsg_UpdateProbeRspTemplate(tpAniSirGlobal pMac,tpSendProbeRespParams msg);
#endif
void halMsg_GenerateRsp(tpAniSirGlobal pMac, tANI_U16 msgType, tANI_U16 dialog_token, void *param, tANI_U32 value);
void halMsg_UpdateTxCmdTemplate(tpAniSirGlobal  pMac,tANI_U16 dialog_token, tpUpdateTxCmdTemplParams  param);
eHalStatus halMsg_updateRetryLimit(tpAniSirGlobal pMac);
eHalStatus halMsg_updateEdcaParam(tpAniSirGlobal pMac, tEdcaParams *pEdcaParams);
eHalStatus halMsg_updateBeaconParam(tpAniSirGlobal pMac, tpUpdateBeaconParams pBeaconParams);
eHalStatus halSetByteSwap(
    tpAniSirGlobal  pMac,
    eAniBoolean     swap01,
    eAniBoolean     swap2,
    eAniBoolean     swapDma );

eHalStatus halToggleByteSwap(tpAniSirGlobal pMac);
eHalStatus halInitWmParam(tHalHandle hHal, void *arg);
eHalStatus halMsg_AddStaSelf(tpAniSirGlobal  pMac, tANI_U16 dialog_token, tpAddStaSelfParams pAddStaSelfParams );
void halMsg_DelStaSelf(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpDelStaSelfParams pDelStaSelfReq);
void halMsg_setBssLinkState(tpAniSirGlobal pMac, tSirLinkState state,
        tANI_U8 bssIdx);
void halMsg_ProcessSetLinkState(tpAniSirGlobal pMac,
    tpLinkStateParams pLinkStateParams);


void halMsg_GetDpuStats( tpAniSirGlobal  pMac, tANI_U16 dialog_token, tpDpuStatsParams  param);
void halMsg_SetStaKey( tpAniSirGlobal  pMac, tANI_U16 dialog_token, tpSetStaKeyParams  param);
void halMsg_SetStaBcastKey(tpAniSirGlobal pMac,tANI_U16 dialog_token,tpSetStaKeyParams param);
void halMsg_SetBssKey( tpAniSirGlobal  pMac, tANI_U16 dialog_token, tpSetBssKeyParams  param);
void halMsg_GetDpuParams( tpAniSirGlobal  pMac, tANI_U16 dialog_token, tpGetDpuParams  param);
eHalStatus halMsg_updateFragThreshold(tpAniSirGlobal pMac);
tSirRetStatus halMsg_AddTs(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpAddTsParams msg);
tSirRetStatus halMsg_DelTs(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpDelTsParams msg);
void halMsg_RegisterPECallback(tpAniSirGlobal pMac, void *pHalMsgCB);

void halMsg_AddBA(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpAddBAParams msg);
void halMsg_DelBA(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpDelBAParams msg);
void halMsg_BAFail(tpAniSirGlobal pMac, tANI_U16 dialog_token,tpAddBARsp pAddBARsp);
eHalStatus halMsg_PostBADeleteInd(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U8 baTID, tANI_U8 baDirection, tANI_U32 reasonCode);
eHalStatus halMsg_HandleInitScan(tpAniSirGlobal pMac, tpInitScanParams msg, tANI_U32* waitForTxComp);
eHalStatus halMsg_HandleFinishScan(tpAniSirGlobal pMac, tpFinishScanParams msg, tANI_U32* pWaitForTxComp);

void halMsg_InitRxChainsReg( tpAniSirGlobal pMac);
void halMsg_SetMimoPs(tpAniSirGlobal pMac,tpSetMIMOPS msg);
void halMsg_BeaconPre(tpAniSirGlobal pMac);

eHalStatus halMsg_configPowerSave(tpAniSirGlobal pMac, tpSirPowerSaveCfg pPowerSaveConfig);

void halMsg_RemoveBssKey(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpRemoveBssKeyParams param);
void halMsg_RemoveStaKey(tpAniSirGlobal pMac, tANI_U16 dialog_token, tpRemoveStaKeyParams param);
void halMsg_SmacChangeModeResponseTimeout(tpAniSirGlobal pMac);
eHalStatus hal_SysmodeChangeResp(tHalHandle hHal);

void halMsg_HandleTrafficActivity(tpAniSirGlobal pMac);
void halMsg_ChipMonitorTimeout(tpAniSirGlobal pMac);

eHalStatus halMsg_setTxPower(tpAniSirGlobal pMac, tpSirSetTxPowerReq pSetTxPowerReq);
void halMsg_getTxPower(tpAniSirGlobal pMac, tpSirGetTxPowerReq pGetTxPowerReq);
void halMsg_sendSetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 status);
void halMsg_sendGetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 power, eHalStatus status);
void halMsg_sendGetNoiseRsp(tpAniSirGlobal pMac);

#ifdef WLAN_FEATURE_VOWIFI
eHalStatus halMsg_setTxPowerLimit(tpAniSirGlobal pMac, tpMaxTxPowerParams pSetMaxTxPwrReq);
#endif /* WLAN_FEATURE_VOWIFI */

#ifdef WLAN_FEATURE_VOWIFI_11R
eHalStatus halMsg_AggrAddTsReq(tpAniSirGlobal pMac, void* msg);
#endif

eHalStatus halMsg_UpdateTpeProtectionThreshold(tpAniSirGlobal pMac, tANI_U32 val);
void halMsg_SetKeyDone(tpAniSirGlobal pMac, tANI_U32 bssidx);

/** ----- APIs to handle transmission freeze and resume ------- */
eHalStatus
halMsg_FrameTransmitControlInd(tpAniSirGlobal pMac, void *pBuffer);
void halMsg_TXCompleteInd(tpAniSirGlobal pMac, tANI_U32 txCompleteSuccess);
#ifdef FEATURE_WLAN_CCX
tSirRetStatus halMsg_GetTSMStats(tpAniSirGlobal  pMac, tpTSMStats pTsmStats);
#endif
#endif  /* _HALMSGAPI_INTERNAL_H_ */
