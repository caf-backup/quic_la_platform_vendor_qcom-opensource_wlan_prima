#ifndef __P2P_H
#define __P2P_H
/**===========================================================================

\file         wlan_hdd_p2p.h

\brief       Linux HDD P2P include file
               Copyright 2008 (c) Qualcomm, Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary.

==========================================================================*/
#ifdef CONFIG_CFG80211
#define ACTION_FRAME_TX_TIMEOUT 500
#define WAIT_CANCEL_REM_CHAN    100
#define WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX 3000

enum hdd_rx_flags {
    HDD_RX_FLAG_DECRYPTED        = 1 << 0,
    HDD_RX_FLAG_MMIC_STRIPPED    = 1 << 1,
    HDD_RX_FLAG_IV_STRIPPED      = 1 << 2,
};


#ifdef WLAN_FEATURE_P2P
typedef struct p2p_app_setP2pPs{
   tANI_U8     opp_ps;
   tANI_U32     ctWindow;
   tANI_U8     count;
   tANI_U32     duration;
   tANI_U32    interval;
   tANI_U32    single_noa_duration;
   tANI_U8      psSelection;
}p2p_app_setP2pPs_t;

int wlan_hdd_cfg80211_remain_on_channel( struct wiphy *wiphy,
                                struct net_device *dev,
                                struct ieee80211_channel *chan,
                                enum nl80211_channel_type channel_type,
                                unsigned int duration, u64 *cookie );

int wlan_hdd_cfg80211_cancel_remain_on_channel( struct wiphy *wiphy,
                                       struct net_device *dev,
                                       u64 cookie );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy, 
                                          struct net_device *dev,
                                          u64 cookie);
#endif

int hdd_setP2pPs( struct net_device *dev, void *msgData );

void hdd_indicateMgmtFrame( hdd_adapter_t *pAdapter,
                            tANI_U32 nFrameLength, tANI_U8* pbFrames,
                            tANI_U8 frameType );

void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter );
void hdd_sendActionCnf( hdd_adapter_t *pAdapter, tANI_BOOLEAN actionSendSuccess );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len, u64 *cookie );
#else
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
                     const u8 *buf, size_t len, u64 *cookie );
#endif

#endif // WLAN_FEATURE_P2P

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params );
#else
int wlan_hdd_add_virtual_intf( struct wiphy *wiphy, char *name,
                               enum nl80211_iftype type,
                               u32 *flags, struct vif_params *params );
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params );
#else
int wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params );
#endif

int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct net_device *dev );

#endif // CONFIG_CFG80211

#endif // __P2P_H
