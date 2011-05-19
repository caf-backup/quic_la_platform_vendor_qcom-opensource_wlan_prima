
#ifndef __P2P_H
#define __P2P_H/**===========================================================================    \file  wlan_hdd_p2p.h    \brief Linux HDD P2P include file         Copyright 2008 (c) Qualcomm, Incorporated.         All Rights Reserved.         Qualcomm Confidential and Proprietary.    ==========================================================================*/

#ifdef WLAN_FEATURE_P2P
int wlan_hdd_remain_on_channel( struct wiphy *wiphy, struct net_device *dev, 
                              struct ieee80211_channel *chan, enum nl80211_channel_type channel_type,
                              unsigned int duration, u64 *cookie );
int wlan_hdd_cancel_remain_on_channel(struct wiphy *wiphy, struct net_device *dev,
                                      u64 cookie);
#endif

int wlan_hdd_action(struct wiphy *wiphy, struct net_device *dev,
			  struct ieee80211_channel *chan,
			  enum nl80211_channel_type channel_type,
			  bool channel_type_valid,
			  const u8 *buf, size_t len, u64 *cookie);
int wlan_hdd_add_virtual_intf(struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params);
int wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct net_device *dev);

void hdd_indicateMgmtFrame( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo );
void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo );
void hdd_sendActionCnf( hdd_adapter_t *pAdapter,  tCsrRoamInfo *pRoamInfo );
#endif
