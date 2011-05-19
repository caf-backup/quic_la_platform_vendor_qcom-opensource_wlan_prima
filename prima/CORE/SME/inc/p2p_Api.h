
/******************************************************************************
*
* Name:  p2p_Api.h
*
* Description: P2P FSM defines.
*
* Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#ifndef __P2P_API_H__
#define __P2P_API_H__

#include "vos_types.h"
#if 0
eHalStatus sme_P2PNoAUpdate (tHalHandle hHal, tpNoAConfig pNoA);
eHalStatus p2pGetNoA(tHalHandle hHal, void *data);
eHalStatus p2pSetPwrSave(tHalHandle hHal, void *data);
eHalStatus p2pSetNoA(tHalHandle hHal, void *data);


typedef struct sP2PConfigs {
   tANI_U8 listenChannel;
}tP2PConfigs,*tpP2PConfigs;  

typedef struct sP2PNoAConfig {
   tANI_U8 count;
   tANI_U32 start_time;
   tANI_U32 duration;
	tANI_U32 interval;
}tNoAConfig,*tpNoAConfig;

typedef struct sP2PPwrSaveConfigs {
   tANI_U8 legacy_PS;
   tANI_U8 oppPS;
   tANI_U32 ctWindow;
}tPwrSaveConfig,*tpPwrSaveConfig;  
#endif
typedef eHalStatus (*remainOnChanCallback)(
		tHalHandle, 
    void* context,
		eHalStatus status);

typedef struct sRemainOnChn{
	tANI_U8 chn;
	tANI_U32 duration;
	remainOnChanCallback callback;
  void *pCBContext;
}tRemainOnChn, tpRemainOnChn;

typedef struct sp2pContext
{
  v_CONTEXT_t vosContext;
  tHalHandle hHal;   

  tANI_U8 sessionId; //Session id corresponding to P2P.
  tANI_U8 probeReqForwarding;
  tANI_U8 *probeRspIe;
  tANI_U32 probeRspIeLength;

  tANI_U8 operatingChn;
} tp2pContext, *tPp2pContext;


eHalStatus sme_RemainOnChannel(tHalHandle hHal, tANI_U8 sessionId,
	     tANI_U8 channel, tANI_U32 duration,
        remainOnChanCallback callback, 
        void *pContext);
eHalStatus sme_ReportProbeReq(tHalHandle hHal,
	     tANI_U8 flag);
eHalStatus sme_updateP2pIe(tHalHandle hHal,
	     void *p2pIe, tANI_U32 p2pIeLength);
eHalStatus sme_sendAction(tHalHandle hHal,
	     const tANI_U8 *pBuf, tANI_U32 len);
eHalStatus sme_CancelRemainOnChannel(tHalHandle hHal, tANI_U8 sessionId );
eHalStatus sme_p2pOpen( tHalHandle hHal );
eHalStatus sme_p2pClose( tHalHandle hHal );

eHalStatus p2pRemainOnChannel(tHalHandle hHal, tANI_U8 sessionId,
	     tANI_U8 channel, tANI_U32 duration,
        remainOnChanCallback callback, 
        void *pContext);
eHalStatus p2pSendAction(tHalHandle hHal,
	     const tANI_U8 *pBuf, tANI_U32 len);
eHalStatus p2pCancelRemainOnChannel(tHalHandle hHal, tANI_U8 sessionId);
#endif //__P2P_API_H__
