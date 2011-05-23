
/**========================================================================

  \file  wlan_hdd_p2p.c

  \brief WLAN Host Device Driver implementation for P2P commands interface

  Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.

  ========================================================================*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>
#include <net/cfg80211.h>
#include <linux/wireless.h>
#include <wlan_hdd_wowl.h>
#include <aniGlobal.h>
#include "ccmApi.h"
#include "sirParams.h"
#include "dot11f.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_wext.h"
#include "sme_Api.h"
#include "wlan_hdd_p2p.h"
#include "p2p_Api.h"

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_hostapd.h>

extern struct net_device_ops net_ops_struct;

#ifdef WLAN_FEATURE_P2P
eHalStatus wlan_hdd_remain_on_channel_callback( tHalHandle hHal, void* pCtx, eHalStatus status )
{
  hdd_remain_on_chan_ctx_t *pRemainChanCtx = (hdd_remain_on_chan_ctx_t*) pCtx;
  hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(pRemainChanCtx->dev);

  hddLog( LOGE, "Recieved remain on channel rsp");
  cfg80211_remain_on_channel_expired( pRemainChanCtx->dev, 
         (tANI_U32)pRemainChanCtx, &pRemainChanCtx->chan, 
         pRemainChanCtx->chan_type, GFP_KERNEL ); 

  vos_mem_free( pRemainChanCtx );

  sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, 
       (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 );

  return eHAL_STATUS_SUCCESS;
}

int wlan_hdd_remain_on_channel( struct wiphy *wiphy, struct net_device *dev, 
    struct ieee80211_channel *chan, enum nl80211_channel_type channel_type,
    unsigned int duration, u64 *cookie )
{
  hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
  eHalStatus status;
  hdd_remain_on_chan_ctx_t *pRemainChanCtx;
  hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
  
  hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",__func__,pAdapter->device_mode);

  hddLog( LOGE, "chan(hw_val) = 0x%x chan(centerfreq) = 0x%x chan type = 0x%x, duration = %d", 
      chan->hw_value, chan->center_freq, channel_type, duration );

  pRemainChanCtx = vos_mem_malloc( sizeof(hdd_remain_on_chan_ctx_t) );
  if( NULL == pRemainChanCtx )
    return -ENOMEM;

  vos_mem_copy( &pRemainChanCtx->chan, chan, sizeof(struct ieee80211_channel) );
  pRemainChanCtx->chan_type = channel_type;
  pRemainChanCtx->duration = duration;
  pRemainChanCtx->dev = dev;
  *cookie = (tANI_U32) pRemainChanCtx;
  cfgState->current_freq = pRemainChanCtx->chan.center_freq;

  //call sme API to start remain on channel.

  status = sme_RemainOnChannel(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId, \
           chan->hw_value, duration, wlan_hdd_remain_on_channel_callback, 
           pRemainChanCtx );

  sme_RegisterMgmtFrame(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId, 
       (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 );
  return 0;
}

void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo )
{
  hdd_remain_on_chan_ctx_t *pRemainChanCtx = (hdd_remain_on_chan_ctx_t*) pRoamInfo->pRemainCtx;

   hddLog( LOGE, "Ready on chan ind");
  cfg80211_ready_on_channel( pAdapter->dev, (tANI_U32)pRemainChanCtx, &pRemainChanCtx->chan, pRemainChanCtx->chan_type, pRemainChanCtx->duration, GFP_KERNEL ); 
}

int wlan_hdd_cancel_remain_on_channel(struct wiphy *wiphy, struct net_device *dev,
                                  u64 cookie)
{
  hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
  eHalStatus status;
  
  hddLog( LOGE, "Cancel remain on channel req");
#if 0
  if( !cfgState->remain_on_chan_active || cookie != cfgState->cookie )
    return -EINVAL;
#endif
  //FIXME cancel currently running remain on chan. Need to check cookie and cancel accordingly

  //Issue abort remain on chan request to sme. 
  //The remain on channel callback will make sure the remain_on_chan expired event is sent.
  status = sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId );

  sme_DeregisterMgmtFrame(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId, 
       (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 );
  return 0;
}

int wlan_hdd_action(struct wiphy *wiphy, struct net_device *dev,
			  struct ieee80211_channel *chan,
			  enum nl80211_channel_type channel_type,
			  bool channel_type_valid,
			  const u8 *buf, size_t len, u64 *cookie)
{
  hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
  hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

  hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",__func__,pAdapter->device_mode);
  //Call sme API to send out a action frame.
  // OR can we send it directly through data path??
  // After tx completion send tx status back.

#if 0
  if( !cfgState->remain_on_chan_active ) //Currently we support sending action frame only if remain on chan is active
    return -EINVAL;
#endif

  if( NULL != cfgState->buf )
    return -EBUSY;

  hddLog( LOGE, "Action frame tx request\n");

  cfgState->buf = vos_mem_malloc( len ); //buf;
  if( cfgState->buf == NULL )
    return -ENOMEM;

  cfgState->len = len;

  vos_mem_copy( cfgState->buf, buf, len);

  *cookie = (tANI_U32) cfgState->buf;
  cfgState->action_cookie = *cookie;

  sme_sendAction( WLAN_HDD_GET_HAL_CTX(pAdapter), buf, len);

  return 0;
}

void hdd_sendActionCnf( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo )
{
  hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

  hddLog( LOGE, "Send Action cnf");
  if( NULL == cfgState->buf )
  {
    VOS_ASSERT( cfgState->buf );
    return;
  }
  /*
   * Assuming tx is success. TODO. should it take data path?
   * buf is the same pointer it passed us to send. Since we are sending it through control path, we use different buffers.
   * In case of mac80211, they just push it to the skb and pass the same data while sending tx ack status.
   * */
  cfg80211_mgmt_tx_status( pAdapter->dev, cfgState->action_cookie, cfgState->buf, cfgState->len, true, GFP_KERNEL ); 
  vos_mem_free( cfgState->buf );
  cfgState->buf = NULL;
}
#endif

static tANI_U8 wlan_hdd_get_session_type(enum nl80211_iftype type)
{
   tANI_U8 sessionType; 
   switch (type)
   {
      case NL80211_IFTYPE_AP:
         sessionType = WLAN_HDD_SOFTAP;
         break;
      case NL80211_IFTYPE_P2P_GO:
         sessionType = WLAN_HDD_P2P_GO;
         break;
      case NL80211_IFTYPE_P2P_CLIENT: 
         sessionType = WLAN_HDD_P2P_CLIENT;
         break;
      case NL80211_IFTYPE_STATION:
         sessionType = WLAN_HDD_INFRA_STATION;
         break;
      case NL80211_IFTYPE_MONITOR: 
         sessionType = WLAN_HDD_MONITOR;
         break;
      default:
         break;
   }
   return sessionType;
}

int wlan_hdd_add_virtual_intf(struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params)
{
  hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);

  ENTER();

  if( NULL == hdd_open_adapter( pHddCtx, wlan_hdd_get_session_type(type),
                        name, wlan_hdd_get_intf_addr(pHddCtx),VOS_TRUE)
  )
  {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: hdd_open_adapter failed",__func__);
      return -EINVAL;
  }
  EXIT();
  return 0;
}

int wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct net_device *dev)
{
  hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
  hdd_adapter_t *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
  ENTER();  
  hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",__func__,pVirtAdapter->device_mode);
  wlan_hdd_release_intf_addr(pHddCtx, pVirtAdapter->macAddressCurrent.bytes);
  hdd_close_adapter( pHddCtx, pVirtAdapter, TRUE );
  EXIT();
  return 0;
}

void hdd_indicateMgmtFrame( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo )
{
  int rxstat;
  hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

  if( pRoamInfo->nActionLength )
  {
     hddLog( LOGE, FL("Indicate Action Frame"));
     //Channel indicated may be wrong. TODO
    //Indicate an action frame.
    cfg80211_rx_mgmt( pAdapter->dev, cfgState->current_freq, 
           pRoamInfo->pbFrames, 
           pRoamInfo->nActionLength, GFP_KERNEL );
  }
  else if( pRoamInfo->nProbeReqLength )
  {
    //Indicate an probe request frame.
   struct sk_buff *skb = NULL;
   VOS_STATUS status;
   vos_pkt_t *pVosPkt;
   struct ethhdr *eth;
   hdd_adapter_t *pMonAdapter = hdd_get_mon_adapter( WLAN_HDD_GET_CTX(pAdapter) );

   if( NULL == pMonAdapter ) return;

   VOS_ASSERT( (pRoamInfo->pbFrames != NULL) );
   status = vos_pkt_get_packet( &pVosPkt, VOS_PKT_TYPE_RX_RAW, 
                                   pRoamInfo->nProbeReqLength, 1, 0, NULL, NULL);

   if( VOS_STATUS_SUCCESS == status )
   {
     hddLog( LOGE, FL("vos_pkt_get success"));
     if( VOS_STATUS_SUCCESS == vos_pkt_push_head( pVosPkt, 
           pRoamInfo->pbFrames,
           pRoamInfo->nProbeReqLength ) )
     {
       hddLog( LOGE, FL("vos_pkt_get_os_pkt success"));

       status = vos_pkt_get_os_packet( pVosPkt, (v_VOID_t **)&skb, VOS_TRUE );
       if(!VOS_IS_STATUS_SUCCESS( status ))
       {
         //This is bad but still try to free the VOSS resources if we can
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure extracting skb from vos pkt", __FUNCTION__);
         vos_pkt_return_packet( pVosPkt );
         return; // VOS_STATUS_E_FAILURE;
       }
       skb->dev = pMonAdapter->dev;
       skb->protocol = eth_type_trans(skb, skb->dev); 
       skb->ip_summed = CHECKSUM_UNNECESSARY;
       eth = eth_hdr(skb);
       
       rxstat = netif_rx_ni(skb);
       if( NET_RX_SUCCESS == rxstat )
       {
         hddLog( LOGE, FL("Success"));
       }
       else
         hddLog( LOGE, FL("Failed %d"), rxstat);
     }

     status = vos_pkt_return_packet( pVosPkt );
   }
  }

  return;
}
