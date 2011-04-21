#if !defined( HDD_CFG80211_H__ )
#define HDD_CFG80211_H__

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
#endif
