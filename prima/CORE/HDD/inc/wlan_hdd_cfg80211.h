#if !defined( HDD_CFG80211_H__ )
#define HDD_CFG80211_H__


#define GAS_INITIAL_REQ "\x04\x0a"
#define GAS_INITIAL_RSP "\x04\x0b"
#define GAS_COMEBACK_REQ "\x04\x0c"
#define GAS_COMEBACK_RSP "\x04\x0d"
#define P2P_PUBLIC_ACTION_FRAME "\x04\x09\x50\x6f\x9a\x09"
#define P2P_ACTION_FRAME "\x7f\x50\x6f\x9a\x09"

/**===========================================================================
  
  \file  wlan_hdd_cfg80211.h
  
  \brief cfg80211 functions declarations
    
               Copyright 2008 (c) Qualcomm, Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/* $HEADER$ */

void wlan_hdd_cfg80211_update_bss_db( hdd_adapter_t *pAdapter, 
                                      tCsrRoamInfo *pRoamInfo
                                      );


struct wiphy *wlan_hdd_cfg80211_init( struct device *dev, 
	                                     int priv_size
	                                     );
void wlan_hdd_cfg80211_post_voss_start(hdd_adapter_t* pAdapter);

#endif
