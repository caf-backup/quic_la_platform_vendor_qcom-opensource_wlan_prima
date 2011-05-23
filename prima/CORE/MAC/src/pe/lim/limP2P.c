#ifdef WLAN_FEATURE_P2P
#include "palTypes.h"
#include "wniApi.h"
#include "sirApi.h"
#include "aniGlobal.h"
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halDataStruct.h"
#endif
#ifdef ANI_PRODUCT_TYPE_AP
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#include "wniApi.h"
#endif
#include "cfgApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limApi.h"
#include "limGlobal.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limSerDesUtils.h"
#include "limAdmitControl.h"
#include "logDump.h"
#include "limSendSmeRspMessages.h"
#include "limTrace.h"
#include "limSession.h"

#ifdef VOSS_ENABLED
#include "vos_types.h"
#include "vos_packet.h"
#include "wlan_qct_tl.h"
#include "sysStartup.h"
#endif

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
#include "halInternal.h"
#endif
#include "sme_Api.h"
#include "smsDebug.h"
#include "csrInsideApi.h"
#include "smeInside.h"
#include "p2p_Api.h"
#include "limApi.h"


void RemainOnChannelSuspendLinkHandler(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data);
void remainOnChannelSetLinkStat(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data, 
    tpPESession psessionEntry);
void ExitRemainOnChannel(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data, 
    tpPESession psessionEntry);
void remainOnChnRsp(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data);
extern tSirRetStatus limSetLinkState(tpAniSirGlobal pMac, tSirLinkState state,tSirMacAddr bssId, tSirMacAddr selfMacAddr);


/*------------------------------------------------------------------
 *
 * Remain on channel req handler. Initiate the INIT_SCAN, CHN_CHANGE and SET_LINK
 *
 * Request from SME, chnNum and duration to remain on channel.
 *
 *------------------------------------------------------------------*/
 

int limProcessReaminOnChnlReq(tpAniSirGlobal pMac, tANI_U32 *pMsg)
{
  tSirRemainOnChnReq *MsgBuff = (tSirRemainOnChnReq *)pMsg;
//  tLimRemainOnChnReq *param;

  pMac->lim.gLimPrevMlmState = pMac->lim.gLimMlmState;

  pMac->lim.gLimMlmState = eLIM_MLM_P2P_LISTEN_STATE;
  pMac->lim.gpLimRemainOnChanReq = MsgBuff;
#if 0
  if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&param, sizeof(tLimRemainOnChnReq) ))
  {
    limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_LISTEN_RSP\n"));
    goto error;
  }
  palZeroMemory(pMac->hHdd, (void*)param, sizeof(tLimRemainOnChnReq));

  param->chnNum = MsgBuff->chnNum;
  param->duration = MsgBuff->duration;
#endif
  /* Get the Remain on channel context */

#if 0
  /* Update the p2pIE and WPS IE in probe res and allow probe Req flag */
  if (MsgRemainonChannel->p2pIe == NULL) 
  {
    PELOGE(limLog( pMac, LOGE, "No P2P IE set\n");)
      return;
  }
#endif
  /* Update the IEs : TBD*/

  /* 1st we need to suspend link with callback to initiate change channel */
  limSuspendLink(pMac, eSIR_CHECK_LINK_TRAFFIC_BEFORE_SCAN,  
      RemainOnChannelSuspendLinkHandler, NULL); 
  /* pMsg is freed by the caller */
  return FALSE;
#if 0
error:
  limSendSmeRsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP, eHAL_STATUS_FAILURE, 0, 0);
  return TRUE;
#endif

}

/*------------------------------------------------------------------
 *
 * limSuspenLink callback, on success link suspend, trigger change chn 
 *
 * 
 *------------------------------------------------------------------*/



tSirRetStatus remainOnChannelChangeChnReq(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data)
{
    tpPESession psessionEntry;
    tANI_U8 sessionId=0;
    tSirRetStatus nSirStatus = eSIR_FAILURE;
    tANI_U32 val;
   
     /* The link is not suspended */
    if (status != eHAL_STATUS_SUCCESS) 
    {
        PELOGE(limLog( pMac, LOGE, "%s: Suspend link Failure \n", __func__);)
		  goto error;
    }

    VOS_ASSERT( pMac->lim.gpLimRemainOnChanReq );
	 
    if((psessionEntry = peFindSessionByBssid(pMac,pMac->lim.gpLimRemainOnChanReq->selfMacAddr,&sessionId)) != NULL)
    {
      limLog(pMac, LOGP, FL("Session Already exists for given BSSID\n"));
      goto error;
    }    
    else       /* Session Entry does not exist for given BSSId */
    {       
      /* Try to Create a new session */
      if((psessionEntry = peCreateSession(pMac,pMac->lim.gpLimRemainOnChanReq->selfMacAddr, &sessionId, 1)) == NULL)
      {
        limLog(pMac, LOGE, FL("Session Can not be created \n"));
		  /* send remain on chn failure */
        //limSendSmeListenRsp(pMac, eSIR_SME_INVALID_STATE, pListenReq->sessionId, pListenReq->transactionId);
        goto error;
      }
      /* Store PE sessionId in session Table  */
      psessionEntry->peSessionId = sessionId;

      psessionEntry->limSystemRole = eLIM_P2P_DEVICE_ROLE;
      CFG_GET_STR( nSirStatus, pMac,  WNI_CFG_SUPPORTED_RATES_11A,
          psessionEntry->rateSet.rate, val , SIR_MAC_MAX_NUMBER_OF_RATES );
      psessionEntry->rateSet.numRates = val;

      CFG_GET_STR( nSirStatus, pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
          psessionEntry->extRateSet.rate, val , WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET_LEN );
      psessionEntry->extRateSet.numRates = val;

      //psessionEntry->currentOperChannel = ((tSirRemainOnChnReq *)data)->chnNum;
      sirCopyMacAddr(psessionEntry->selfMacAddr, pMac->lim.gpLimRemainOnChanReq->selfMacAddr);
    }
    
    /* change channel to the requested by RemainOn Chn*/
	 limChangeChannelWithCallback(pMac, 
           pMac->lim.gpLimRemainOnChanReq->chnNum,
           remainOnChannelSetLinkStat, NULL, psessionEntry);
	 return eSIR_SUCCESS;

error:
	 limSendSmeRsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP, eHAL_STATUS_FAILURE, 0, 0);
	 return eSIR_FAILURE;
}

void RemainOnChannelSuspendLinkHandler(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data)
{
    remainOnChannelChangeChnReq(pMac, status, data);
    return;
}

/*------------------------------------------------------------------
 *
 * Set the LINK state to LISTEN to allow only PROBE_REQ and Action frames
 *
 *------------------------------------------------------------------*/

void remainOnChannelSetLinkStat(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data, 
    tpPESession psessionEntry)
{
    tANI_U32 val;
    tSirRemainOnChnReq *MsgRemainonChannel = pMac->lim.gpLimRemainOnChanReq;
    tSirMacAddr             nullBssid = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
    if (status != eHAL_STATUS_SUCCESS) 
    {
        limLog( pMac, LOGE, "%s: Change channel not successful\n");
        goto error1;
    }

    //Send Ready on channel indication to SME
    limSendSmeRsp(pMac, eWNI_SME_REMAIN_ON_CHN_RDY_IND, status, 0, 0); 

    if ((limSetLinkState(pMac, eSIR_LINK_LISTEN_STATE,nullBssid, pMac->lim.gSelfMacAddr)) != eSIR_SUCCESS)
    {
      limLog( pMac, LOGE, "Unable to change link state");
      goto error1;
    }

    // Start timer here to come back to operating channel.
    pMac->lim.limTimers.gLimRemainOnChannelTimer.sessionId = psessionEntry->peSessionId;
	  /* get the duration from the request */
    val = SYS_MS_TO_TICKS(MsgRemainonChannel->duration);

    limLog( pMac, LOGE, "Start listen duration = %d", val);
    if (tx_timer_change(&pMac->lim.limTimers.gLimRemainOnChannelTimer,
                                                val, 0) != TX_SUCCESS)
    {
       /**
             * Could not change Join Failure
             * timer. Log error.
             */
      limLog(pMac, LOGP, FL("Unable to change remain on channel Timer val\n"));
		goto error;
    }
				
    if(TX_SUCCESS !=  tx_timer_activate(&pMac->lim.limTimers.gLimRemainOnChannelTimer))
    {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
        limLog( pMac, LOGE, "%s: remain on channel Timer Start Failed\n", __func__);
#endif
      goto error;
    }
    return;
error:
	 limDeactivateAndChangeTimer(pMac, eLIM_REMAIN_CHN_TIMER);
error1:
	 remainOnChnRsp(pMac,eHAL_STATUS_FAILURE, NULL);
	 return;
}

/*------------------------------------------------------------------
 *
 * limchannelchange callback, on success channel change, set the link_state to LISTEN
 *
 *------------------------------------------------------------------*/

void limProcessRemainOnChnTimeout(tpAniSirGlobal pMac)
{
    tpPESession psessionEntry;
    tANI_U8 prevChannel = 6;
    tSirMacAddr             nullBssid = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    /* get the session */

    limDeactivateAndChangeTimer(pMac, eLIM_REMAIN_CHN_TIMER);
    if((psessionEntry = peFindSessionBySessionId(pMac, pMac->lim.limTimers.gLimRemainOnChannelTimer.sessionId))== NULL) 
    {
        limLog(pMac, LOGE, FL("Session Does not exist for given sessionID\n"));
        goto error;
    }

    /* get the previous valid LINK state */
	 if (limSetLinkState(pMac, eSIR_LINK_IDLE_STATE, nullBssid, pMac->lim.gSelfMacAddr) != eSIR_SUCCESS)
    {
      limLog( pMac, LOGE, "Unable to change link state");
      return;
    }

    /* Switch back to the operating channel */
	 // prevChannel = <get the previous channel number 
    limChangeChannelWithCallback(pMac, 
          prevChannel,
            ExitRemainOnChannel, NULL, psessionEntry);
	 return;
error:
	remainOnChnRsp(pMac,eHAL_STATUS_FAILURE, NULL);
	return;
}


/*------------------------------------------------------------------
 *
 * limchannelchange callback, on success channel change, set the link_state to LISTEN
 *
 *------------------------------------------------------------------*/

void ExitRemainOnChannel(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data, 
    tpPESession psessionEntry)
{
    
    if (status != eHAL_STATUS_SUCCESS) 
    {
        PELOGE(limLog( pMac, LOGE, "%s: Change channel not successful for FT pre-auth\n");)
       goto error;
    }
    limResumeLink(pMac, remainOnChnRsp, NULL); 	 
    return;
error:
	remainOnChnRsp(pMac,eHAL_STATUS_FAILURE, NULL);
	return;
}

/*------------------------------------------------------------------
 *
 * Send remain on channel respone: Success/ Failure
 *
 *------------------------------------------------------------------*/

void remainOnChnRsp(tpAniSirGlobal pMac, eHalStatus status, tANI_U32 *data)
{
   tpPESession psessionEntry;
   tANI_U8             sessionId;
	
    /* delete the session */
	 if((psessionEntry = peFindSessionByBssid(pMac,pMac->lim.gSelfMacAddr,&sessionId)) != NULL)
	 {
	   peDeleteSession( pMac, psessionEntry);
	 }
	 
    if( pMac->lim.gpLimRemainOnChanReq )
    {
      palFreeMemory( pMac->hHdd, pMac->lim.gpLimRemainOnChanReq );
      pMac->lim.gpLimRemainOnChanReq = NULL;
    }
    /* Post the meessage to Sme */
    limSendSmeRsp(pMac, eWNI_SME_REMAIN_ON_CHN_RSP, status, 0, 0);
    pMac->lim.gLimMlmState = pMac->lim.gLimPrevMlmState;
    return;
}

/*------------------------------------------------------------------
 *
 * Indicate the Mgmt Frame received to SME to HDD callback
 * handle Probe_req/Action frame currently
 *
 *------------------------------------------------------------------*/
void
limSendSmeMgmtFrameInd(tpAniSirGlobal pMac, tSirSmeMgmtFrameType frameType,
                    tANI_U8  *frame, tANI_U32 frameLen)
{
     tSirMsgQ              mmhMsg;
     tpSirSmeMgmtFrameInd pSirSmeMgmtFrame = NULL;
     tANI_U16              length;

     length = sizeof(tSirSmeMgmtFrameInd) + frameLen;
	  
     if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeMgmtFrame, length ))
     {
          limLog(pMac, LOGP, FL("palAllocateMemory failed for eWNI_SME_LISTEN_RSP\n"));
          return;
     }
     palZeroMemory(pMac->hHdd, (void*)pSirSmeMgmtFrame, length);

    pSirSmeMgmtFrame->mesgType = eWNI_SME_MGMT_FRM_IND;
    pSirSmeMgmtFrame->mesgLen = length;
    pSirSmeMgmtFrame->frameType = frameType;
    
	  vos_mem_zero(pSirSmeMgmtFrame->frameBuf,frameLen);
	  vos_mem_copy(pSirSmeMgmtFrame->frameBuf,frame,frameLen);

     mmhMsg.type = eWNI_SME_MGMT_FRM_IND;
     mmhMsg.bodyptr = pSirSmeMgmtFrame;
     mmhMsg.bodyval = 0;
     limSysProcessMmhMsgApi(pMac, &mmhMsg, ePROT);

     return;
} /*** end limSendSmeListenRsp() ***/

void limSendP2PActionFrame(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
  tSirMbMsg *pMbMsg = (tSirMbMsg *)pMsg->bodyptr;
  tANI_U32            nBytes;
  tANI_U8            *pFrame;
  void               *pPacket;
  eHalStatus          halstatus;

  nBytes = pMbMsg->msgLen - sizeof(tSirMbMsg);

  // Ok-- try to allocate some memory:
  halstatus = palPktAlloc( pMac->hHdd, HAL_TXRX_FRM_802_11_MGMT,
      ( tANI_U16 )nBytes, ( void** ) &pFrame,
      ( void** ) &pPacket );
  if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
  {
    limLog( pMac, LOGP, FL("Failed to allocate %d bytes for a Pro"
          "be Request.\n"), nBytes );
    return;
  }

  // Paranoia:
  palZeroMemory( pMac->hHdd, pFrame, nBytes );

  palCopyMemory( pMac->hHdd, pFrame, pMbMsg->data, nBytes ); 

  halstatus = halTxFrame( pMac, pPacket, ( tANI_U16 ) nBytes,
      HAL_TXRX_FRM_802_11_MGMT,
      ANI_TXDIR_TODS,
      7,//SMAC_SWBD_TX_TID_MGMT_HIGH,
      limTxComplete, pFrame );
  if ( ! HAL_STATUS_SUCCESS ( halstatus ) )
  {
    limLog( pMac, LOGE, FL("could not send Probe Request frame!\n" ));
    //Pkt will be freed up by the callback
  }

  limSendSmeRsp(pMac, eWNI_SME_ACTION_FRAME_SEND_CNF, halstatus, 0, 0);

  return ;
}

void
limAbortRemainOnChan(tpAniSirGlobal pMac)
{
  //TODO check for state and take appropriate actions
  limDeactivateAndChangeTimer(pMac, eLIM_REMAIN_CHN_TIMER);
  limProcessRemainOnChnTimeout(pMac);
  return;
}

#endif
