
/**========================================================================

  \file  wlan_hdd_p2p.c

  \brief WLAN Host Device Driver implementation for P2P commands interface

  Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.

  ========================================================================*/
#ifdef CONFIG_CFG80211

#include <wlan_hdd_includes.h>
#include <net/cfg80211.h>
#include "sme_Api.h"
#include "wlan_hdd_p2p.h"
#include "sapApi.h"

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>

extern struct net_device_ops net_ops_struct;

static int hdd_wlan_add_rx_radiotap_hdr( struct sk_buff *skb,
                                         int rtap_len, int flag );

static void hdd_wlan_tx_complete( hdd_adapter_t* pAdapter,
                                  hdd_cfg80211_state_t* cfgState,
                                  tANI_BOOLEAN actionSendSuccess );

#ifdef WLAN_FEATURE_P2P
eHalStatus wlan_hdd_remain_on_channel_callback( tHalHandle hHal, void* pCtx,
                                                eHalStatus status )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) pCtx;
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = cfgState->remain_on_chan_ctx;

    if( pRemainChanCtx == NULL )
    {
       hddLog( LOGE,
          "%s: No Rem on channel pending for which Rsp is received", __func__);
       return eHAL_STATUS_SUCCESS;
    }

    hddLog( LOGE, "Received remain on channel rsp");

    cfgState->remain_on_chan_ctx = NULL;

    if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
    {
        if( cfgState->buf )
        {
           hddLog( LOGP, 
                   "%s: We need to receive yet an ack from one of tx packet",
                   __func__);
        }
        cfg80211_remain_on_channel_expired( pRemainChanCtx->dev,
                              pRemainChanCtx->cookie,
                              &pRemainChanCtx->chan,
                              pRemainChanCtx->chan_type, GFP_KERNEL );
    }

    vos_mem_free( pRemainChanCtx );

    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode )
       )
    {
        sme_DeregisterMgmtFrame(
                   hHal, pAdapter->sessionId,
                   (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                    NULL, 0 );
    }
    else if ( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
    {
        WLANSAP_DeRegisterMgmtFrame(
                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                NULL, 0 );
    }

    complete(&pAdapter->cancel_rem_on_chan_var);
    return eHAL_STATUS_SUCCESS;
}

static int wlan_hdd_request_remain_on_channel( struct wiphy *wiphy,
                                   struct net_device *dev,
                                   struct ieee80211_channel *chan,
                                   enum nl80211_channel_type channel_type,
                                   unsigned int duration, u64 *cookie,
                                   rem_on_channel_request_type_t request_type )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_remain_on_chan_ctx_t *pRemainChanCtx;
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                                 __func__,pAdapter->device_mode);

    hddLog( LOGE,
        "chan(hw_val)0x%x chan(centerfreq) %d chan type 0x%x, duration %d",
        chan->hw_value, chan->center_freq, channel_type, duration );

    if( cfgState->remain_on_chan_ctx != NULL)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
             "%s: Already one Remain on Channel Pending", __func__);
        return -EBUSY;
    }

    pRemainChanCtx = vos_mem_malloc( sizeof(hdd_remain_on_chan_ctx_t) );
    if( NULL == pRemainChanCtx )
    {
        hddLog(VOS_TRACE_LEVEL_FATAL,
             "%s: Not able to allocate memory for Channel context",
                                         __func__);
        return -ENOMEM;
    }

    vos_mem_copy( &pRemainChanCtx->chan, chan,
                   sizeof(struct ieee80211_channel) );

    pRemainChanCtx->chan_type = channel_type;
    pRemainChanCtx->duration = duration;
    pRemainChanCtx->dev = dev;
    *cookie = (tANI_U32) pRemainChanCtx;
    pRemainChanCtx->cookie = *cookie;
    pRemainChanCtx->rem_on_chan_request = request_type;
    cfgState->remain_on_chan_ctx = pRemainChanCtx;
    cfgState->current_freq = chan->center_freq;

    //call sme API to start remain on channel.
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode )
       )
    {
        //call sme API to start remain on channel.
        sme_RemainOnChannel(
                       WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId,
                       chan->hw_value, duration,
                       wlan_hdd_remain_on_channel_callback, pAdapter );

        sme_RegisterMgmtFrame(WLAN_HDD_GET_HAL_CTX(pAdapter),
                              pAdapter->sessionId, (SIR_MAC_MGMT_FRAME << 2) |
                              (SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 );

    }
    else if ( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
    {
        //call sme API to start remain on channel.
        WLANSAP_RemainOnChannel(
                          (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                          chan->hw_value, duration,
                          wlan_hdd_remain_on_channel_callback, pAdapter );

        WLANSAP_RegisterMgmtFrame(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                    (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                    NULL, 0 );
    }
    return 0;

}

int wlan_hdd_cfg80211_remain_on_channel( struct wiphy *wiphy,
                                struct net_device *dev,
                                struct ieee80211_channel *chan,
                                enum nl80211_channel_type channel_type,
                                unsigned int duration, u64 *cookie )
{
    return wlan_hdd_request_remain_on_channel(wiphy, dev,
                                        chan, channel_type, duration, cookie,
                                        REMAIN_ON_CHANNEL_REQUEST);
}

void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter )
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    hdd_remain_on_chan_ctx_t* pRemainChanCtx = cfgState->remain_on_chan_ctx;

    hddLog( LOG1, "Ready on chan ind");

    if( pRemainChanCtx != NULL )
    {
        if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
        {
            cfg80211_ready_on_channel( pAdapter->dev, (tANI_U32)pRemainChanCtx,
                               &pRemainChanCtx->chan, pRemainChanCtx->chan_type,
                               pRemainChanCtx->duration, GFP_KERNEL );
        }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
        else if( OFF_CHANNEL_ACTION_TX == pRemainChanCtx->rem_on_chan_request )
        {
            complete(&pAdapter->offchannel_tx_event);
        }
#endif
    }
    else
    {
        hddLog( LOGE, "%s: No Pending Remain on channel Request", __func__);
    }
    return;
}

int wlan_hdd_cfg80211_cancel_remain_on_channel( struct wiphy *wiphy,
                                      struct net_device *dev, u64 cookie )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );


    hddLog( LOGE, "Cancel remain on channel req");

    /* FIXME cancel currently running remain on chan.
     * Need to check cookie and cancel accordingly
     */
    if( (cfgState->remain_on_chan_ctx == NULL) ||
        (cfgState->remain_on_chan_ctx->cookie != cookie) )
    {
        hddLog( LOGE,
            "%s: No Remain on channel pending with specified cookie value",
             __func__);
        return -EINVAL;
    }

    INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);
    /* Issue abort remain on chan request to sme.
     * The remain on channel callback will make sure the remain_on_chan
     * expired event is sent.
     */
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode )
       )
    {
        sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter ),
                                            pAdapter->sessionId );
    }
    else if ( (WLAN_HDD_SOFTAP== pAdapter->device_mode) ||
              (WLAN_HDD_P2P_GO == pAdapter->device_mode)
            )
    {
        WLANSAP_CancelRemainOnChannel(
                                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
    }
    else 
    {
       hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid device_mode = %d",
                            __func__, pAdapter->device_mode);
       return -EIO;
    }
    wait_for_completion_interruptible_timeout(&pAdapter->cancel_rem_on_chan_var,
            msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len, u64 *cookie )
#else
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
                     const u8 *buf, size_t len, u64 *cookie )
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                            __func__,pAdapter->device_mode);

    //Call sme API to send out a action frame.
    // OR can we send it directly through data path??
    // After tx completion send tx status back.

    if( NULL != cfgState->buf )
        return -EBUSY;

    hddLog( LOGE, "Action frame tx request");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    if( offchan && wait)
    {
        int status;
        hdd_adapter_t *goAdapter;

        goAdapter = hdd_get_adapter( pAdapter->pHddCtx, WLAN_HDD_P2P_GO );

        //If GO adapter exists and operating on same frequency 
        //then we will not request remain on channel 
        if( goAdapter && ( ieee80211_frequency_to_channel(chan->center_freq)
                             == goAdapter->sessionCtx.ap.operatingChannel ) )
        {
           goto send_frame;
        }

        INIT_COMPLETION(pAdapter->offchannel_tx_event);

        status = wlan_hdd_request_remain_on_channel(wiphy, dev,
                                        chan, channel_type, wait, cookie,
                                        OFF_CHANNEL_ACTION_TX);

        if(0 != status)
        {
            if( (-EBUSY == status) && 
                (cfgState->current_freq == chan->center_freq) )
            {
                goto send_frame;
            }
            goto err_rem_channel;
        }

        /* Wait for driver to be ready on the requested channel */
        status = wait_for_completion_interruptible_timeout(
                     &pAdapter->offchannel_tx_event,
                     msecs_to_jiffies(WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX));
        if(!status)
        {
            hddLog( LOGE, "Not able to complete remain on channel request"
                          " within timeout period");
            goto err_rem_channel;
        }
    }
    send_frame:
#endif

    cfgState->buf = vos_mem_malloc( len ); //buf;
    if( cfgState->buf == NULL )
        return -ENOMEM;

    cfgState->len = len;

    vos_mem_copy( cfgState->buf, buf, len);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    if( cfgState->remain_on_chan_ctx )
    {
        cfgState->action_cookie = cfgState->remain_on_chan_ctx->cookie;
        *cookie = cfgState->action_cookie;
    }
    else
    {
#endif
        *cookie = (tANI_U32) cfgState->buf;
        cfgState->action_cookie = *cookie;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    }
#endif

    if ( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
         (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode)
       )
    {
        if (eHAL_STATUS_SUCCESS !=
               sme_sendAction( WLAN_HDD_GET_HAL_CTX(pAdapter),
                               pAdapter->sessionId, buf, len) )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "%s: sme_sendAction returned fail", __func__);
            goto err;
        }
    }
    else if( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
     {
        if( eHAL_STATUS_SUCCESS !=
             WLANSAP_SendAction( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                                  buf, len ) )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_SendAction returned fail", __func__);
            goto err;
        }
    }

    return 0;
err:
    hdd_sendActionCnf( pAdapter, FALSE );
    return 0;
err_rem_channel:
    *cookie = (tANI_U32)cfgState;
    cfg80211_mgmt_tx_status( pAdapter->dev, *cookie, buf, len, FALSE, GFP_KERNEL );
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy, 
                                          struct net_device *dev,
                                          u64 cookie)
{
    return wlan_hdd_cfg80211_cancel_remain_on_channel( wiphy, dev, cookie );
}
#endif

void hdd_sendActionCnf( hdd_adapter_t *pAdapter, tANI_BOOLEAN actionSendSuccess )
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

    hddLog( LOGE, "Send Action cnf, actionSendSuccess %d", actionSendSuccess);
    if( NULL == cfgState->buf )
    {
        VOS_ASSERT( cfgState->buf );
        return;
    }

    /* If skb is NULL it means this packet was received on CFG80211 interface
     * else it was received on Monitor interface */
    if( cfgState->skb == NULL )
    {
        /*
         * buf is the same pointer it passed us to send. Since we are sending
         * it through control path, we use different buffers.
         * In case of mac80211, they just push it to the skb and pass the same
         * data while sending tx ack status.
         * */
         cfg80211_mgmt_tx_status( pAdapter->dev, cfgState->action_cookie,
                cfgState->buf, cfgState->len, actionSendSuccess, GFP_KERNEL );
         vos_mem_free( cfgState->buf );
         cfgState->buf = NULL;
    }
    else
    {
        hdd_adapter_t* pMonAdapter =
                    hdd_get_adapter( pAdapter->pHddCtx, WLAN_HDD_MONITOR );
        if( pMonAdapter == NULL )
        {
            hddLog( LOGE, "Not able to get Monitor Adapter");
            cfgState->skb = NULL;
            vos_mem_free( cfgState->buf );
            cfgState->buf = NULL;
            complete(&pAdapter->tx_action_cnf_event);
            return;
        }
        /* Send TX completion feedback over monitor interface. */
        hdd_wlan_tx_complete( pMonAdapter, cfgState, actionSendSuccess );
        cfgState->skb = NULL;
        vos_mem_free( cfgState->buf );
        cfgState->buf = NULL;
        /* Look for the next Mgmt packet to TX */
        hdd_mon_tx_mgmt_pkt(pAdapter);
    }
    complete(&pAdapter->tx_action_cnf_event);
}

int hdd_setP2pPs( struct net_device *dev, void *msgData )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tP2pPsConfig NoA;
    p2p_app_setP2pPs_t *pappNoA = (p2p_app_setP2pPs_t *) msgData;

    NoA.opp_ps = pappNoA->opp_ps;
    NoA.ctWindow = pappNoA->ctWindow;
    NoA.duration = pappNoA->duration;
    NoA.interval = pappNoA->interval;
    NoA.count = pappNoA->count;
    NoA.single_noa_duration = pappNoA->single_noa_duration;
    NoA.psSelection = pappNoA->psSelection;
    NoA.sessionid = pAdapter->sessionId;

    sme_p2pSetPs(hHal, &NoA);
    return status;
}
#endif

static tANI_U8 wlan_hdd_get_session_type( enum nl80211_iftype type )
{
    tANI_U8 sessionType;

    switch( type )
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#else
int wlan_hdd_add_virtual_intf( struct wiphy *wiphy, char *name,
                               enum nl80211_iftype type,
                               u32 *flags, struct vif_params *params )
#endif
{
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
    hdd_adapter_t* pAdapter = NULL;

    ENTER();

    pAdapter = hdd_open_adapter( pHddCtx, wlan_hdd_get_session_type(type),
                          name, wlan_hdd_get_intf_addr(pHddCtx), VOS_TRUE );

    if( NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: hdd_open_adapter failed",__func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
        return NULL;
#else
        return -EINVAL;
#endif
    }
    EXIT();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    return pAdapter->dev;
#else
    return 0;
#endif
}

int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct net_device *dev )
{
     hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
     hdd_adapter_t *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
     ENTER();

     hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
            __func__,pVirtAdapter->device_mode);

     wlan_hdd_release_intf_addr( pHddCtx,
                                 pVirtAdapter->macAddressCurrent.bytes );

     hdd_close_adapter( pHddCtx, pVirtAdapter, TRUE );
     EXIT();
     return 0;
}

void hdd_indicateMgmtFrame( hdd_adapter_t *pAdapter,
                            tANI_U32 nProbeReqLength,
                            tANI_U32 nActionLength, tANI_U8* pbFrames )
{
    int rxstat;
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

    if( nActionLength )
    {
        hddLog( LOGE, FL("Indicate Action Frame over NL80211 Intf"));
        //Channel indicated may be wrong. TODO
        //Indicate an action frame.
        cfg80211_rx_mgmt( pAdapter->dev, cfgState->current_freq,
                          pbFrames,
                          nActionLength, GFP_ATOMIC );
    }
    else if( nProbeReqLength )
    {
        //Indicate an probe request frame.
        struct sk_buff *skb = NULL;
        hdd_adapter_t *pMonAdapter =
                   hdd_get_mon_adapter( WLAN_HDD_GET_CTX(pAdapter) );
        int needed_headroom = 0;
        int flag = HDD_RX_FLAG_IV_STRIPPED | HDD_RX_FLAG_DECRYPTED |
                   HDD_RX_FLAG_MMIC_STRIPPED;

        hddLog( LOGE, FL("Indicate Action Frame over Monitor Intf"));

        if( NULL == pMonAdapter ) return;

        VOS_ASSERT( (pbFrames != NULL) );

        /* room for the radiotap header based on driver features
         * 1 Byte for RADIO TAP Flag, 1 Byte padding and 2 Byte for
         * RX flags.
         * */
        needed_headroom = sizeof(struct ieee80211_radiotap_header) + 4;

        //alloc skb  here
        skb = alloc_skb(VPKT_SIZE_BUFFER, GFP_ATOMIC);
        if (unlikely(NULL == skb))
        {
            hddLog( LOGW, FL("Unable to allocate skb"));
            return;
        }
        skb_reserve(skb, VPKT_SIZE_BUFFER);
        if (unlikely(skb_headroom(skb) < nProbeReqLength))
        {
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                     "HDD [%d]: Insufficient headroom, "
                     "head[%p], data[%p], req[%d]",
                     __LINE__, skb->head, skb->data, nProbeReqLength);
           kfree_skb(skb);
           return ;
        }
        // actually push the data
        memcpy(skb_push(skb, nProbeReqLength), pbFrames, nProbeReqLength);
        /* prepend radiotap information */
        if( 0 != hdd_wlan_add_rx_radiotap_hdr( skb,
                                needed_headroom, flag ) )
        {
            hddLog( LOGE, FL("Not Able Add Radio Tap"));
            //free skb
            kfree_skb(skb);
            return;
        }

        skb_reset_mac_header( skb );
        skb->dev = pMonAdapter->dev;
        skb->protocol = eth_type_trans( skb, skb->dev );
        skb->ip_summed = CHECKSUM_UNNECESSARY;
        rxstat = netif_rx_ni(skb);
        if( NET_RX_SUCCESS == rxstat )
        {
            hddLog( LOGE, FL("Success"));
        }
        else
            hddLog( LOGE, FL("Failed %d"), rxstat);                   
    }
    return;
}

/*
 * ieee80211_add_rx_radiotap_header - add radiotap header
 */
static int hdd_wlan_add_rx_radiotap_hdr (
             struct sk_buff *skb, int rtap_len, int flag )
{
    u8 rtap_temp[20] = {0};
    struct ieee80211_radiotap_header *rthdr;
    unsigned char *pos;
    u16 rx_flags = 0;

    rthdr = (struct ieee80211_radiotap_header *)(&rtap_temp[0]);

    /* radiotap header, set always present flags */
    rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_FLAGS)   |
                                    (1 << IEEE80211_RADIOTAP_RX_FLAGS));
    rthdr->it_len = cpu_to_le16(rtap_len);

    pos = (unsigned char *) (rthdr + 1);

    /* the order of the following fields is important */

    /* IEEE80211_RADIOTAP_FLAGS */
    *pos = 0;
    pos++;

    /* IEEE80211_RADIOTAP_RX_FLAGS: Length 2 Bytes */
    /* ensure 2 byte alignment for the 2 byte field as required */
    if ((pos - (u8 *)rthdr) & 1)
        pos++;
    put_unaligned_le16(rx_flags, pos);
    pos += 2;
    
    // actually push the data
    memcpy(skb_push(skb, rtap_len), &rtap_temp[0], rtap_len);

    return 0;
}

static void hdd_wlan_tx_complete( hdd_adapter_t* pAdapter,
                                  hdd_cfg80211_state_t* cfgState,
                                  tANI_BOOLEAN actionSendSuccess )
{
    struct ieee80211_radiotap_header *rthdr;
    unsigned char *pos;
    struct sk_buff *skb = cfgState->skb;

    /* 2 Byte for TX flags and 1 Byte for Retry count */
    u32 rtHdrLen = sizeof(*rthdr) + 3;

    u8 *data;

    /* We have to return skb with Data starting with MAC header. We have
     * copied SKB data starting with MAC header to cfgState->buf. We will pull
     * entire skb->len from skb and then we will push cfgState->buf to skb
     * */
    if( NULL == skb_pull(skb, skb->len) )
    {
        hddLog( LOGE, FL("Not Able to Pull %d byte from skb"), skb->len);
        kfree_skb(cfgState->skb);
        return;
    }

    data = skb_push( skb, cfgState->len );

    if (data == NULL)
    {
        hddLog( LOGE, FL("Not Able to Push %d byte to skb"), cfgState->len);
        kfree_skb( cfgState->skb );
        return;
    }

    memcpy( data, cfgState->buf, cfgState->len );

    /* send frame to monitor interfaces now */
    if( skb_headroom(skb) < rtHdrLen )
    {
        hddLog( LOGE, FL("No headroom for rtap header"));
        kfree_skb(cfgState->skb);
        return;
    }

    rthdr = (struct ieee80211_radiotap_header*) skb_push( skb, rtHdrLen );

    memset( rthdr, 0, rtHdrLen );
    rthdr->it_len = cpu_to_le16( rtHdrLen );
    rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_TX_FLAGS) |
                                    (1 << IEEE80211_RADIOTAP_DATA_RETRIES)
                                   );

    pos = (unsigned char *)( rthdr+1 );

    // Fill TX flags
    *pos = actionSendSuccess;
    pos += 2;

    // Fill retry count
    *pos = 0;
    pos++;

    skb_set_mac_header( skb, 0 );
    skb->ip_summed = CHECKSUM_UNNECESSARY;
    skb->pkt_type  = PACKET_OTHERHOST;
    skb->protocol  = htons(ETH_P_802_2);
    memset( skb->cb, 0, sizeof( skb->cb ) );
    if (in_interrupt())
        netif_rx( skb );
    else
        netif_rx_ni( skb );

    /* Enable Queues which we have disabled earlier */
    netif_tx_start_all_queues( pAdapter->dev ); 

}
#endif // CONFIG_CFG80211
