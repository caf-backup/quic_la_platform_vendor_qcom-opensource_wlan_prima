#if !defined( HDD_CFG80211_H__ )
#define HDD_CFG80211_H__

/**===========================================================================
  
  \file  wlan_hdd_cfg80211.h
  
  \brief cfg80211 functions declarations
    
               Copyright 2008 (c) Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary.
  
  ==========================================================================*/
  
/* $HEADER$ */

#define WLAN_BSS_MEMBERSHIP_SELECTOR_HT_PHY 127
#define BASIC_RATE_MASK   0x80
#define RATE_MASK         0x7f

void wlan_hdd_cfg80211_update_bss_db( hdd_adapter_t *pAdapter, 
                                      tCsrRoamInfo *pRoamInfo
                                      );
#ifdef FEATURE_WLAN_WAPI
void wlan_hdd_cfg80211_set_key_wapi(hdd_adapter_t* pAdapter,
              u8 key_index, const u8 *mac_addr, u8 *key , int key_Len);
#endif
struct wiphy *wlan_hdd_cfg80211_init( struct device *dev, 
	                                     int priv_size
	                                     );
#endif
