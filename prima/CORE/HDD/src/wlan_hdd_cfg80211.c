/**========================================================================

  \file  wlan_hdd_cfg80211.c

  \brief WLAN Host Device Driver implementation

  Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.

  ========================================================================*/

/**========================================================================= 

  EDIT HISTORY FOR FILE 


  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 


  $Header:$   $DateTime: $ $Author: $ 


  when        who            what, where, why 
  --------    ---            --------------------------------------------------------
 21/12/09     Ashwani        Created module.  

 07/06/10     Kumar Deepak   Implemented cfg80211 callbacks for ANDROID
              Ganesh K       
  ==========================================================================*/

#ifdef CONFIG_CFG80211

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
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_hostapd.h"
#include "sapInternal.h"
#include "wlan_hdd_softap_tx_rx.h"
#include "wlan_hdd_main.h"

#define g_mode_rates_size (12)
#define a_mode_rates_size (8)
#define FREQ_BASE_80211G          (2407)
#define FREQ_BAND_DIFF_80211G     (5)
#define MAX_SCAN_SSID 2
#define GET_IE_LEN_IN_BSS_DESC(lenInBss) ( lenInBss + sizeof(lenInBss) - \
        ((int) OFFSET_OF( tSirBssDescription, ieFields)))

#define HDD2GHZCHAN(freq, chan, flag)   {     \
    .band =  IEEE80211_BAND_2GHZ, \
    .center_freq = (freq), \
    .hw_value = (chan),\
    .flags = (flag), \
    .max_antenna_gain = 0 ,\
    .max_power = 30, \
}

#define HDD5GHZCHAN(freq, chan, flag)   {     \
    .band =  IEEE80211_BAND_5GHZ, \
    .center_freq = (freq), \
    .hw_value = (chan),\
    .flags = (flag), \
    .max_antenna_gain = 0 ,\
    .max_power = 30, \
}

#define HDD_G_MODE_RATETAB(rate, rate_id, flag)\
{\
    .bitrate = rate, \
    .hw_value = rate_id, \
    .flags = flag, \
}

#define g_ht_capability \
             IEEE80211_HT_CAP_SGI_20 |        \
             IEEE80211_HT_CAP_GRN_FLD |       \
             IEEE80211_HT_CAP_DSSSCCK40 |     \
             IEEE80211_HT_CAP_LSIG_TXOP_PROT

static const u32 hdd_cipher_suites[] = 
{
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP
};

static inline int is_broadcast_ether_addr(const u8 *addr)
{
    return ((addr[0] == 0xff) && (addr[1] == 0xff) && (addr[2] == 0xff) &&
            (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff));
}

static struct ieee80211_channel hdd_channels_2_4_GHZ[] =
{  
    HDD2GHZCHAN(2412, 1, 0) ,
    HDD2GHZCHAN(2417, 2, 0) ,
    HDD2GHZCHAN(2422, 3, 0) ,
    HDD2GHZCHAN(2427, 4, 0) ,
    HDD2GHZCHAN(2432, 5, 0) ,
    HDD2GHZCHAN(2437, 6, 0) ,
    HDD2GHZCHAN(2442, 7, 0) ,
    HDD2GHZCHAN(2447, 8, 0) ,
    HDD2GHZCHAN(2452, 9, 0) ,
    HDD2GHZCHAN(2457, 10, 0) ,
    HDD2GHZCHAN(2462, 11, 0) ,
    HDD2GHZCHAN(2467, 12, 0) ,
    HDD2GHZCHAN(2472, 13, 0) ,
    HDD2GHZCHAN(2484, 14, 0) ,
};

#ifdef WLAN_FEATURE_P2P
static struct ieee80211_channel hdd_social_channels_2_4_GHZ[] =
{
    HDD2GHZCHAN(2412, 1, 0) ,
    HDD2GHZCHAN(2437, 6, 0) ,
    HDD2GHZCHAN(2462, 11, 0) ,
};
#endif

static struct ieee80211_channel hdd_channels_5_GHZ[] =
{
    HDD5GHZCHAN(5180, 36, 0) ,
    HDD5GHZCHAN(5200, 40, 0) ,
    HDD5GHZCHAN(5220, 44, 0) ,
    HDD5GHZCHAN(5240, 48, 0) ,
    HDD5GHZCHAN(5260, 52, 0) ,
    HDD5GHZCHAN(5280, 56, 0) ,
    HDD5GHZCHAN(5300, 60, 0) ,
    HDD5GHZCHAN(5320, 64, 0) ,
    HDD5GHZCHAN(5500,100, 0) ,
    HDD5GHZCHAN(5520,104, 0) ,
    HDD5GHZCHAN(5540,108, 0) ,
    HDD5GHZCHAN(5560,112, 0) ,
    HDD5GHZCHAN(5580,116, 0) ,
    HDD5GHZCHAN(5600,120, 0) ,
    HDD5GHZCHAN(5620,124, 0) ,
    HDD5GHZCHAN(5640,128, 0) ,
    HDD5GHZCHAN(5660,132, 0) ,
    HDD5GHZCHAN(5680,136, 0) ,
    HDD5GHZCHAN(5700,140, 0) ,
    HDD5GHZCHAN(5745,149, 0) ,
    HDD5GHZCHAN(5765,153, 0) ,
    HDD5GHZCHAN(5785,157, 0) ,
    HDD5GHZCHAN(5805,161, 0) ,
    HDD5GHZCHAN(5825,165, 0) ,
};

static struct ieee80211_rate g_mode_rates[] =
{
    HDD_G_MODE_RATETAB(10, 0x1, 0),    
    HDD_G_MODE_RATETAB(20, 0x2, 0),    
    HDD_G_MODE_RATETAB(55, 0x4, 0),    
    HDD_G_MODE_RATETAB(110, 0x8, 0),    
    HDD_G_MODE_RATETAB(60, 0x10, 0),    
    HDD_G_MODE_RATETAB(90, 0x20, 0),    
    HDD_G_MODE_RATETAB(120, 0x40, 0),    
    HDD_G_MODE_RATETAB(180, 0x80, 0),    
    HDD_G_MODE_RATETAB(240, 0x100, 0),    
    HDD_G_MODE_RATETAB(360, 0x200, 0),    
    HDD_G_MODE_RATETAB(480, 0x400, 0),    
    HDD_G_MODE_RATETAB(540, 0x800, 0),
};   

static struct ieee80211_rate a_mode_rates[] =
{
    HDD_G_MODE_RATETAB(60, 0x10, 0),    
    HDD_G_MODE_RATETAB(90, 0x20, 0),    
    HDD_G_MODE_RATETAB(120, 0x40, 0),    
    HDD_G_MODE_RATETAB(180, 0x80, 0),    
    HDD_G_MODE_RATETAB(240, 0x100, 0),    
    HDD_G_MODE_RATETAB(360, 0x200, 0),    
    HDD_G_MODE_RATETAB(480, 0x400, 0),    
    HDD_G_MODE_RATETAB(540, 0x800, 0),
};

static struct ieee80211_supported_band wlan_hdd_band_2_4_GHZ = 
{
    .channels = hdd_channels_2_4_GHZ,
    .n_channels = ARRAY_SIZE(hdd_channels_2_4_GHZ),
    .band       = IEEE80211_BAND_2GHZ,
    .bitrates = g_mode_rates,
    .n_bitrates = g_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            = g_ht_capability,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};

#ifdef WLAN_FEATURE_P2P
static struct ieee80211_supported_band wlan_hdd_band_p2p_2_4_GHZ = 
{
    .channels = hdd_social_channels_2_4_GHZ,
    .n_channels = ARRAY_SIZE(hdd_social_channels_2_4_GHZ),
    .band       = IEEE80211_BAND_2GHZ,
    .bitrates = g_mode_rates,
    .n_bitrates = g_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            = g_ht_capability,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};
#endif

static struct ieee80211_supported_band wlan_hdd_band_5_GHZ =
{
    .channels = hdd_channels_5_GHZ,
    .n_channels = ARRAY_SIZE(hdd_channels_5_GHZ),
    .band     = IEEE80211_BAND_5GHZ,
    .bitrates = a_mode_rates,
    .n_bitrates = a_mode_rates_size,
    .ht_cap.ht_supported   = 1,
    .ht_cap.cap            = g_ht_capability,
    .ht_cap.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,
    .ht_cap.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_16,
    .ht_cap.mcs.rx_mask    = { 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    .ht_cap.mcs.rx_highest = cpu_to_le16( 72 ),
    .ht_cap.mcs.tx_params  = IEEE80211_HT_MCS_TX_DEFINED,
};

/* This structure contain information what kind of frame are expected in 
     TX/RX direction for each kind of interface */
static const struct ieee80211_txrx_stypes
wlan_hdd_txrx_stypes[NUM_NL80211_IFTYPES] = {
    [NL80211_IFTYPE_STATION] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ACTION) |
            BIT(SIR_MAC_MGMT_PROBE_REQ),
    },
    [NL80211_IFTYPE_AP] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
            BIT(SIR_MAC_MGMT_REASSOC_REQ) |
            BIT(SIR_MAC_MGMT_PROBE_REQ) |
            BIT(SIR_MAC_MGMT_DISASSOC) |
            BIT(SIR_MAC_MGMT_AUTH) |
            BIT(SIR_MAC_MGMT_DEAUTH) |
            BIT(SIR_MAC_MGMT_ACTION),
    },
#ifdef WLAN_FEATURE_P2P    
    [NL80211_IFTYPE_P2P_CLIENT] = {
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ACTION) |
            BIT(SIR_MAC_MGMT_PROBE_REQ),
    },
    [NL80211_IFTYPE_P2P_GO] = {
        /* This is also same as for SoftAP */
        .tx = 0xffff,
        .rx = BIT(SIR_MAC_MGMT_ASSOC_REQ) |
            BIT(SIR_MAC_MGMT_REASSOC_REQ) |
            BIT(SIR_MAC_MGMT_PROBE_REQ) |
            BIT(SIR_MAC_MGMT_DISASSOC) |
            BIT(SIR_MAC_MGMT_AUTH) |
            BIT(SIR_MAC_MGMT_DEAUTH) |
            BIT(SIR_MAC_MGMT_ACTION),
    },
#endif    
};

static struct cfg80211_ops wlan_hdd_cfg80211_ops;

extern struct net_device_ops net_ops_struct;

/*
 * FUNCTION: wlan_hdd_cfg80211_init
 * This function is called by hdd_wlan_startup() 
 * during initialization. 
 * This function is used to initialize and register wiphy structure.
 */
struct wiphy *wlan_hdd_cfg80211_init(int priv_size)
{
    struct wiphy *wiphy;
    ENTER();

    /* 
         *   Create wiphy device 
        */
    wiphy = wiphy_new(&wlan_hdd_cfg80211_ops, priv_size);

    if (!wiphy)
    {
        /* Print error and jump into err label and free the memory */
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wiphy init failed", __func__);
        return NULL;
    }

    return wiphy;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_init
 * This function is called by hdd_wlan_startup() 
 * during initialization. 
 * This function is used to initialize and register wiphy structure.
 */
int wlan_hdd_cfg80211_register(struct device *dev, 
                               struct wiphy *wiphy,
                               hdd_config_t *pCfg
                               )
{
    /* Now bind the underlying wlan device with wiphy */
    set_wiphy_dev(wiphy, dev);

    wiphy->mgmt_stypes = wlan_hdd_txrx_stypes;

    wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;

    wiphy->max_scan_ssids = MAX_SCAN_SSID; 
    
    wiphy->max_scan_ie_len = 200 ; //TODO: define a macro

    /* Supports STATION & AD-HOC modes right now */
    wiphy->interface_modes =   BIT(NL80211_IFTYPE_STATION) 
                             | BIT(NL80211_IFTYPE_ADHOC)
#ifdef WLAN_FEATURE_P2P
                             | BIT(NL80211_IFTYPE_P2P_CLIENT)
                             | BIT(NL80211_IFTYPE_P2P_GO)
#endif                       
                             | BIT(NL80211_IFTYPE_MONITOR)
                             | BIT(NL80211_IFTYPE_AP);


    /*Initialize band capability*/
    switch(pCfg->nBandCapability)
    {
        case eCSR_BAND_24:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            break;
        case eCSR_BAND_5G:
#ifdef WLAN_FEATURE_P2P
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_p2p_2_4_GHZ;
#endif
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
            break;
        case eCSR_BAND_ALL:
        default:
            wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2_4_GHZ;
            wiphy->bands[IEEE80211_BAND_5GHZ] = &wlan_hdd_band_5_GHZ;
    }
    /*Initialise the supported cipher suite details*/
    wiphy->cipher_suites = hdd_cipher_suites;
    wiphy->n_cipher_suites = ARRAY_SIZE(hdd_cipher_suites);

    /*signal strength in mBm (100*dBm) */
    wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
#ifdef WLAN_FEATURE_P2P
    wiphy->max_remain_on_channel_duration = 1000;
#endif
#endif

    /* Register our wiphy dev with cfg80211 */
    if (0 > wiphy_register(wiphy))
    {
        /* print eror */
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: wiphy register failed", __func__);
        return -EIO;
    }

    EXIT();
    return 0;
}     

/* In this function we will do all post VOS start initialization.
   In this function we will register for all frame in which supplicant
   is interested. 
*/
void wlan_hdd_cfg80211_post_voss_start(hdd_adapter_t* pAdapter)
{
#ifdef WLAN_FEATURE_P2P
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    /* Register for all P2P action, public action etc frames */
    v_U16_t type = (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_ACTION << 4);

   /* Right now we are registering these frame when driver is getting
      initialized. Once we will move to 2.6.37 kernel, in which we have
      frame register ops, we will move this code as a part of that */
    /* GAS Initial Request */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type, 
                         (v_U8_t*)GAS_INITIAL_REQ, GAS_INITIAL_REQ_SIZE );

    /* GAS Initial Response */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_INITIAL_RSP, GAS_INITIAL_RSP_SIZE );
    
    /* GAS Comeback Request */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_REQ, GAS_COMEBACK_REQ_SIZE );

    /* GAS Comeback Response */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_RSP, GAS_COMEBACK_RSP_SIZE );

    /* P2P Public Action */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_PUBLIC_ACTION_FRAME, 
                                  P2P_PUBLIC_ACTION_FRAME_SIZE );

    /* P2P Action */
    sme_RegisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_ACTION_FRAME,
                                  P2P_ACTION_FRAME_SIZE );
#endif /* WLAN_FEATURE_P2P */
}

void wlan_hdd_cfg80211_pre_voss_stop(hdd_adapter_t* pAdapter)
{
#ifdef WLAN_FEATURE_P2P
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    /* Register for all P2P action, public action etc frames */
    v_U16_t type = (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_ACTION << 4);

   /* Right now we are registering these frame when driver is getting
      initialized. Once we will move to 2.6.37 kernel, in which we have
      frame register ops, we will move this code as a part of that */
    /* GAS Initial Request */

    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                          (v_U8_t*)GAS_INITIAL_REQ, GAS_INITIAL_REQ_SIZE );

    /* GAS Initial Response */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_INITIAL_RSP, GAS_INITIAL_RSP_SIZE );
    
    /* GAS Comeback Request */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_REQ, GAS_COMEBACK_REQ_SIZE );

    /* GAS Comeback Response */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)GAS_COMEBACK_RSP, GAS_COMEBACK_RSP_SIZE );

    /* P2P Public Action */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_PUBLIC_ACTION_FRAME, 
                                  P2P_PUBLIC_ACTION_FRAME_SIZE );

    /* P2P Action */
    sme_DeregisterMgmtFrame(hHal, pAdapter->sessionId, type,
                         (v_U8_t*)P2P_ACTION_FRAME,
                                  P2P_ACTION_FRAME_SIZE );
#endif /* WLAN_FEATURE_P2P */
}

int wlan_hdd_cfg80211_alloc_new_beacon(hdd_adapter_t *pAdapter, 
                                       beacon_data_t **ppBeacon,
                                       struct beacon_parameters *params)
{    
    int size;
    beacon_data_t *beacon = NULL;
    beacon_data_t *old = NULL;
    int head_len,tail_len;

    if (params->head && !params->head_len)
        return -EINVAL;

    old = pAdapter->sessionCtx.ap.beacon;

    if (!params->head && !old)
        return -EINVAL;

    if (params->tail && !params->tail_len)
        return -EINVAL;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,38))
    /* Kernel 3.0 is not updating dtim_period for set beacon */
    if (!params->dtim_period)
        return -EINVAL;
#endif

    if(params->head)
        head_len = params->head_len;
    else
        head_len = old->head_len;

    if(params->tail || !old)
        tail_len = params->tail_len;
    else
        tail_len = old->tail_len;

    size = sizeof(beacon_data_t) + head_len + tail_len;

    beacon = kzalloc(size, GFP_KERNEL);

    if( beacon == NULL )
        return -ENOMEM;

    if(params->dtim_period)
        beacon->dtim_period = params->dtim_period;
    else
        beacon->dtim_period = old->dtim_period;
 
    beacon->head = ((u8 *) beacon) + sizeof(beacon_data_t);
    beacon->tail = beacon->head + head_len;
    beacon->head_len = head_len;
    beacon->tail_len = tail_len;

    if(params->head) {
        memcpy (beacon->head,params->head,beacon->head_len);
    }
    else { 
        if(old)
            memcpy (beacon->head,old->head,beacon->head_len);
    }
    
    if(params->tail) {
        memcpy (beacon->tail,params->tail,beacon->tail_len);
    }
    else {
       if(old)   
           memcpy (beacon->tail,old->tail,beacon->tail_len);
    }

    *ppBeacon = beacon;

    kfree(old);

    return 0;

}

v_U8_t* wlan_hdd_cfg80211_get_ie_ptr(v_U8_t *pIes, int length, v_U8_t eid)
{
    int left = length;
    v_U8_t *ptr = pIes;
    v_U8_t elem_id,elem_len;
   
    while(left >= 2)
    {   
        elem_id  =  ptr[0];
        elem_len =  ptr[1];
        left -= 2;
        if(elem_len > left)
        {
            hddLog(VOS_TRACE_LEVEL_FATAL,
                    "****Invalid IEs eid = %d elem_len=%d left=%d*****\n",
                                                    eid,elem_len,left);
            return NULL;
        }
        if (elem_id == eid) 
        {
            return ptr;
        }
   
        left -= elem_len;
        ptr += (elem_len + 2);
    }
    return NULL;
}

/* Check if rate is 11g rate or not */
static int wlan_hdd_rate_is_11g(u8 rate)
{
    u8 gRateArray[8] = {12, 18, 24, 36, 48, 72, 96, 104}; /* actual rate * 2 */
    u8 i;
    for (i = 0; i < 8; i++)
    {
        if(rate == gRateArray[i])
            return TRUE;
    }
    return FALSE;
}

/* Check for 11g rate and set proper 11g only mode */
static void wlan_hdd_check_11gmode(u8 *pIe, u8* require_ht,
                     u8* pCheckRatesfor11g, eSapPhyMode* pSapHw_mode)
{
    u8 i, num_rates = pIe[0];

    pIe += 1;
    for ( i = 0; i < num_rates; i++)
    {
        if( *pCheckRatesfor11g && (TRUE == wlan_hdd_rate_is_11g(pIe[i] & RATE_MASK)))
        {
            /* If rate set have 11g rate than change the mode to 11G */
            *pSapHw_mode = eSAP_DOT11_MODE_11g;
            if (pIe[i] & BASIC_RATE_MASK)
            {
                /* If we have 11g rate as  basic rate, it means mode
                   is 11g only mode.
                 */
               *pSapHw_mode = eSAP_DOT11_MODE_11g_ONLY;
               *pCheckRatesfor11g = FALSE;
            }
        }
        else if((BASIC_RATE_MASK | WLAN_BSS_MEMBERSHIP_SELECTOR_HT_PHY) == pIe[i])
        {
            *require_ht = TRUE;
        }
    }
    return;
}

static void wlan_hdd_set_sapHwmode(hdd_adapter_t *pHostapdAdapter)
{
    tsap_Config_t *pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;
    beacon_data_t *pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;
    struct ieee80211_mgmt *pMgmt_frame = (struct ieee80211_mgmt*)pBeacon->head;
    u8 checkRatesfor11g = TRUE;
    u8 require_ht = FALSE;
    u8 *pIe=NULL;

    pConfig->SapHw_mode= eSAP_DOT11_MODE_11b;

    pIe = wlan_hdd_cfg80211_get_ie_ptr(&pMgmt_frame->u.beacon.variable[0],
                                       pBeacon->head_len, WLAN_EID_SUPP_RATES);
    if (pIe != NULL)
    {
        pIe += 1;
        wlan_hdd_check_11gmode(pIe, &require_ht, &checkRatesfor11g,
                               &pConfig->SapHw_mode);
    }

    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
                                WLAN_EID_EXT_SUPP_RATES);
    if (pIe != NULL)
    {

        pIe += 1;
        wlan_hdd_check_11gmode(pIe, &require_ht, &checkRatesfor11g,
                               &pConfig->SapHw_mode);
    }

    if( pConfig->channel > 14 )
    {
        pConfig->SapHw_mode= eSAP_DOT11_MODE_11a;
    }

    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len,
                                       WLAN_EID_HT_CAPABILITY);

    if(pIe) 
    {
        pConfig->SapHw_mode= eSAP_DOT11_MODE_11n;
        if(require_ht)
            pConfig->SapHw_mode= eSAP_DOT11_MODE_11n_ONLY;
    }

}

static int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter)
{
    tsap_Config_t *pConfig;
    beacon_data_t *pBeacon = NULL;
    struct ieee80211_mgmt *pMgmt_frame;
    v_U8_t *pIe=NULL;
    v_U16_t capab_info;
    eCsrAuthType RSNAuthType;
    eCsrEncryptionType RSNEncryptType;
    eCsrEncryptionType mcRSNEncryptType;
    int status = VOS_STATUS_SUCCESS;
    tpWLAN_SAPEventCB pSapEventCallback;
    v_U8_t *genie;
    v_U8_t total_ielen=0,ielen=0;
    hdd_hostapd_state_t *pHostapdState;
    v_U8_t wpaRsnIEdata[(SIR_MAC_MAX_IE_LENGTH * 2)+4];  //Max ie length 255 * 2(WPA+RSN) + 2 bytes (vendor specific ID) * 2
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;

    ENTER();

    pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);

    pConfig = &pHostapdAdapter->sessionCtx.ap.sapConfig;

    pBeacon = pHostapdAdapter->sessionCtx.ap.beacon;

    pMgmt_frame = (struct ieee80211_mgmt*)pBeacon->head;

    pConfig->beacon_int =  pMgmt_frame->u.beacon.beacon_int;

    //channel is already set in the set_channel Call back
    //pConfig->channel = pCommitConfig->channel;

    /*Protection parameter to enable or disable*/
    pConfig->protEnabled = 
           (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apProtEnabled;

    pConfig->dtim_period = pBeacon->dtim_period;

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"****pConfig->dtim_period=%d***\n",
                                      pConfig->dtim_period);


    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len, 
                                       WLAN_EID_COUNTRY);
    if(pIe)
    { 
        pConfig->ieee80211d = 1;
        vos_mem_copy(pConfig->countryCode, &pIe[2], 3);
    }
    else {
        pConfig->ieee80211d = 0;
    }
    pConfig->authType = eSAP_AUTO_SWITCH;

    capab_info = pMgmt_frame->u.beacon.capab_info;
    
    pConfig->privacy = (pMgmt_frame->u.beacon.capab_info & 
                        WLAN_CAPABILITY_PRIVACY) ? VOS_TRUE : VOS_FALSE;

    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = pConfig->privacy;

    /*Set wps station to configured*/
    pConfig->wps_state = 0;
    pConfig->fwdWPSPBCProbeReq  = 1; // Forward WPS PBC probe request frame up 

    pConfig->RSNWPAReqIELength = 0;
    pConfig->pRSNWPAReqIE = NULL;
    pIe = wlan_hdd_cfg80211_get_ie_ptr(pBeacon->tail, pBeacon->tail_len, 
                                       WLAN_EID_RSN);
    if(pIe && pIe[1])
        {
            pConfig->RSNWPAReqIELength = pIe[1] + 2;
        memcpy(&wpaRsnIEdata[0], pIe, pConfig->RSNWPAReqIELength);
        pConfig->pRSNWPAReqIE = &wpaRsnIEdata[0];
                /* The actual processing may eventually be more extensive than 
                 * this. Right now, just consume any PMKIDs that are  sent in 
                 * by the app.
                 * */
                status = hdd_softap_unpackIE( 
                        vos_get_context( VOS_MODULE_ID_SME, pVosContext),
                           &RSNEncryptType,
                           &mcRSNEncryptType,
                           &RSNAuthType,
                           pConfig->pRSNWPAReqIE[1]+2,
                           pConfig->pRSNWPAReqIE );
                  
                if( VOS_STATUS_SUCCESS == status )
                {
                    /* Now copy over all the security attributes you have 
                     * parsed out 
                     * */
                    pConfig->RSNEncryptType = RSNEncryptType; // Use the cipher type in the RSN IE
                    pConfig->mcRSNEncryptType = mcRSNEncryptType;
                    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->ucEncryptType
                                                              = RSNEncryptType;
                    hddLog( LOG1, FL("%s: CSR AuthType = %d, "
                             "EncryptionType = %d mcEncryptionType = %d\n"),
                             RSNAuthType, RSNEncryptType, mcRSNEncryptType);
                }
            }
    
    pIe = wlan_hdd_get_vendor_oui_ie_ptr(WPA_OUI_TYPE, WPA_OUI_TYPE_SIZE,
                                         pBeacon->tail, pBeacon->tail_len);

    if(pIe && pIe[1] && (pIe[0] == DOT11F_EID_WPA))
    {
        if (pConfig->pRSNWPAReqIE)
        {
            /*Mixed mode WPA/WPA2*/
            memcpy((&wpaRsnIEdata[0] + pConfig->RSNWPAReqIELength), pIe, pIe[1] + 2);
            pConfig->RSNWPAReqIELength += pIe[1] + 2;
        }
        else
        {
            pConfig->RSNWPAReqIELength = pIe[1] + 2;
            memcpy(&wpaRsnIEdata[0], pIe, pConfig->RSNWPAReqIELength);
            pConfig->pRSNWPAReqIE = &wpaRsnIEdata[0];
            status = hdd_softap_unpackIE( 
                          vos_get_context( VOS_MODULE_ID_SME, pVosContext),
                          &RSNEncryptType,
                          &mcRSNEncryptType,
                          &RSNAuthType,
                          pConfig->pRSNWPAReqIE[1]+2,
                          pConfig->pRSNWPAReqIE );

            if( VOS_STATUS_SUCCESS == status )
            {
                /* Now copy over all the security attributes you have 
                 * parsed out 
                 * */
                pConfig->RSNEncryptType = RSNEncryptType; // Use the cipher type in the RSN IE
                pConfig->mcRSNEncryptType = mcRSNEncryptType;
                hddLog( LOG1, FL("%s: CSR AuthType = %d, "
                                "EncryptionType = %d mcEncryptionType = %d\n"),
                                RSNAuthType, RSNEncryptType, mcRSNEncryptType);
            }
        }
    }

    pConfig->SSIDinfo.ssidHidden = VOS_FALSE; 

    pIe = wlan_hdd_cfg80211_get_ie_ptr(&pMgmt_frame->u.beacon.variable[0],
                                       pBeacon->head_len, WLAN_EID_SSID);

    pConfig->SSIDinfo.ssid.length = pIe[1];

    if(pConfig->SSIDinfo.ssid.length) 
    {
        vos_mem_copy(pConfig->SSIDinfo.ssid.ssId, &pIe[2], 
                     pConfig->SSIDinfo.ssid.length);
    }
    else 
    {
      /*Empty SSID*/ 
       pConfig->SSIDinfo.ssidHidden = VOS_TRUE;
    }
    vos_mem_copy(pConfig->self_macaddr.bytes, 
               pHostapdAdapter->macAddressCurrent.bytes, sizeof(v_MACADDR_t));
    
    pConfig->SapMacaddr_acl = eSAP_ACCEPT_UNLESS_DENIED;

    wlan_hdd_set_sapHwmode(pHostapdAdapter);

    // ht_capab is not what the name conveys,this is used for protection bitmap
    pConfig->ht_capab =
                 (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apProtection;
    genie = vos_mem_malloc(MAX_GENIE_LEN);

    if(genie == NULL) {
       
        return -ENOMEM;
    }
    
    pIe = wlan_hdd_get_wps_ie_ptr(pBeacon->tail,pBeacon->tail_len);

    if(pIe) 
    {
        /*Copy the wps IE*/
        ielen = pIe[1] + 2;  
        if( ielen <=MAX_GENIE_LEN) {
            vos_mem_copy(genie,pIe,ielen);
        }
        else {
           hddLog( VOS_TRACE_LEVEL_ERROR, "**Wps Ie Length is too big***\n");
           return -EINVAL;
        }
        total_ielen = ielen;
    }

#ifdef WLAN_FEATURE_P2P    
    pIe = wlan_hdd_get_p2p_ie_ptr(pBeacon->tail,pBeacon->tail_len);

    if(pIe) 
    {
        ielen = pIe[1] + 2;
        if(total_ielen + ielen <= MAX_GENIE_LEN) {
            vos_mem_copy(&genie[total_ielen],pIe,(pIe[1] + 2));
        }
        else {
           hddLog( VOS_TRACE_LEVEL_ERROR, 
                    "**Wps Ie+ P2pIE Length is too big***\n");
           return -EINVAL;
        }
        total_ielen += ielen; 
    }
#endif
    
#ifdef WLAN_FEATURE_WFD
    pIe = wlan_hdd_get_wfd_ie_ptr(pBeacon->tail,pBeacon->tail_len);

    if(pIe) 
    { 
        ielen = pIe[1] + 2;
        if(total_ielen + ielen <= MAX_GENIE_LEN) {
            vos_mem_copy(&genie[total_ielen],pIe,(pIe[1] + 2));
        }
        else {
           hddLog( VOS_TRACE_LEVEL_ERROR, "**Wps Ie + P2p Ie + Wfd Ie Length is too big***\n");
           return -EINVAL;
        }
        total_ielen += ielen; 
    }
#endif

    if(ccmCfgSetStr((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal, 
       WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA, genie, total_ielen, NULL, 
               eANI_BOOLEAN_FALSE)==eHAL_STATUS_FAILURE)
    {            
        hddLog(LOGE, 
               "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_DATA to CCM\n");
        return -EINVAL;
    }

    if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pHostapdAdapter))->hHal, 
          WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, 1,NULL,
          test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags) ?
                   eANI_BOOLEAN_TRUE : eANI_BOOLEAN_FALSE)
          ==eHAL_STATUS_FAILURE)
    {
        hddLog(LOGE, 
            "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG to CCM\n");
        return -EINVAL;
    }

    vos_mem_free(genie);
    
    pConfig->num_accept_mac =0;
    pConfig->num_deny_mac = 0;
    //Uapsd Enabled Bit
    pConfig->UapsdEnable =  
          (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apUapsdEnabled;
    //Enable OBSS protection
    pConfig->obssProtEnabled = 
           (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apOBSSProtEnabled; 
    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->apDisableIntraBssFwd = 
           (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apDisableIntraBssFwd;
    
    hddLog(LOGW, FL("SOftAP macaddress : "MAC_ADDRESS_STR"\n"), 
                 MAC_ADDR_ARRAY(pHostapdAdapter->macAddressCurrent.bytes));
    hddLog(LOGW,FL("ssid =%s\n"), pConfig->SSIDinfo.ssid.ssId);  
    hddLog(LOGW,FL("beaconint=%d, channel=%d\n"), (int)pConfig->beacon_int,
                                                     (int)pConfig->channel);
    hddLog(LOGW,FL("hw_mode=%x\n"),  pConfig->SapHw_mode);
    hddLog(LOGW,FL("privacy=%d, authType=%d\n"), pConfig->privacy, 
                                                 pConfig->authType); 
    hddLog(LOGW,FL("RSN/WPALen=%d, \n"),(int)pConfig->RSNWPAReqIELength);
    hddLog(LOGW,FL("Uapsd = %d\n"),pConfig->UapsdEnable); 
    hddLog(LOGW,FL("ProtEnabled = %d, OBSSProtEnabled = %d\n"),
                          pConfig->protEnabled, pConfig->obssProtEnabled);
    hddLog(LOGW,FL("DisableIntraBssFwd = %d\n"),
          (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->apDisableIntraBssFwd); 

    if(test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) 
    {
        //Bss already started. just return.
        //TODO Probably it should update some beacon params.
        hddLog( LOGE, "Bss Already started...Ignore the request");
        EXIT();
        return 0;
    }
    
    pConfig->persona = pHostapdAdapter->device_mode;

    pSapEventCallback = hdd_hostapd_SAPEventCB;
    if(WLANSAP_StartBss(pVosContext, pSapEventCallback, pConfig,
                 (v_PVOID_t)pHostapdAdapter->dev) != VOS_STATUS_SUCCESS)
    {
        hddLog(LOGE,FL("SAP Start Bss fail\n"));
        return -EINVAL;
    }

    hddLog(LOGE, 
           FL("Waiting for Scan to complete(auto mode) and BSS to start"));

    status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);
   
    if (!VOS_IS_STATUS_SUCCESS(status))
    {  
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                 ("ERROR: HDD vos wait for single_event failed!!\n"));
        VOS_ASSERT(0);
    }
 
    //Succesfully started Bss update the state bit.
    set_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);

    pHostapdState->bCommit = TRUE;
    EXIT();

   return 0;
}

static int wlan_hdd_cfg80211_add_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev, 
                                        struct beacon_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);
    int status=VOS_STATUS_SUCCESS; 

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "device mode=%d\n",pAdapter->device_mode);

    if ( (pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {
        beacon_data_t *old,*new;

        old = pAdapter->sessionCtx.ap.beacon;
   
        if (old)
            return -EALREADY;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,&new,params);

        if(status != VOS_STATUS_SUCCESS) 
        {
             hddLog(VOS_TRACE_LEVEL_FATAL,
                   "%s:Error!!! Allocating the new beacon\n",__func__);
             return -EINVAL;
        }

        pAdapter->sessionCtx.ap.beacon = new;

        status = wlan_hdd_cfg80211_start_bss(pAdapter);
    }

    EXIT();
    return status;
}
 
static int wlan_hdd_cfg80211_set_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev,
                                        struct beacon_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev); 
    int status=VOS_STATUS_SUCCESS;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                __func__,pAdapter->device_mode);

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    {
        beacon_data_t *old,*new;
                
        old = pAdapter->sessionCtx.ap.beacon;
        
        if (!old)
            return -ENOENT;

        status = wlan_hdd_cfg80211_alloc_new_beacon(pAdapter,&new,params);

        if(status != VOS_STATUS_SUCCESS) {
            hddLog(VOS_TRACE_LEVEL_FATAL, 
                   "%s: Error!!! Allocating the new beacon\n",__func__);
            return -EINVAL;
       }

       pAdapter->sessionCtx.ap.beacon = new;

       status = wlan_hdd_cfg80211_start_bss(pAdapter);
    }

    EXIT();
    return status;
}

static int wlan_hdd_cfg80211_del_beacon(struct wiphy *wiphy, 
                                        struct net_device *dev)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);
    VOS_STATUS status = 0;

    ENTER();
 
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                              __func__,pAdapter->device_mode);

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    {
        beacon_data_t *old;
             
        old = pAdapter->sessionCtx.ap.beacon;
        
        if (!old)
            return -ENOENT;

         
        if(test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) 
        {
            if ( VOS_STATUS_SUCCESS == (status = WLANSAP_StopBss((WLAN_HDD_GET_CTX(pAdapter))->pvosContext) ) )
            {
                hdd_hostapd_state_t *pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

                status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);
   
                if (!VOS_IS_STATUS_SUCCESS(status))
                {  
                    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                             ("ERROR: HDD vos wait for single_event failed!!\n"));
                    VOS_ASSERT(0);
                }
            }
            clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);
        }
 
        if(status != VOS_STATUS_SUCCESS) 
        {
            hddLog(VOS_TRACE_LEVEL_FATAL,
                    "%s:Error!!! Stopping the BSS\n",__func__);
            return -EINVAL;
        }
        
        if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pAdapter))->hHal, 
            WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, 0,NULL, eANI_BOOLEAN_FALSE) 
                                                    ==eHAL_STATUS_FAILURE)
        {
            hddLog(LOGE, 
               "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG to CCM\n");
        } 
        pAdapter->sessionCtx.ap.beacon = NULL;
        kfree(old);
    }
    EXIT();
    return status;
}

static int wlan_hdd_cfg80211_change_bss (struct wiphy *wiphy,
                                      struct net_device *dev,
                                      struct bss_parameters *params)
{
    hdd_adapter_t *pAdapter =  WLAN_HDD_GET_PRIV_PTR(dev);

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                               __func__,pAdapter->device_mode);

    if((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
     ||  (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
      ) 
    {
        if (params->ap_isolate) 
        {
            pAdapter->sessionCtx.ap.apDisableIntraBssFwd = VOS_TRUE; 
        } 
        else 
        { 
            pAdapter->sessionCtx.ap.apDisableIntraBssFwd = VOS_FALSE; 
        }
    }

    EXIT();
    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_change_iface
 * This function is used to set the interface type (INFRASTRUCTURE/ADHOC)
 */
int wlan_hdd_cfg80211_change_iface( struct wiphy *wiphy,
                                    struct net_device *ndev,
                                    enum nl80211_iftype type, 
                                    u32 *flags,
                                    struct vif_params *params
                                  )
{
    struct wireless_dev *wdev;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); //(hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_context_t *pHddCtx = (hdd_context_t*)pAdapter->pHddCtx;
    tCsrRoamProfile *pRoamProfile = NULL;
    eCsrRoamBssType LastBSSType;
    hdd_config_t *pConfig = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;
    eMib_dot11DesiredBssType connectedBssType;
    VOS_STATUS status;

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                             __func__,pAdapter->device_mode);
 
    wdev = ndev->ieee80211_ptr;

    /* Reset the current device mode bit mask*/
    wlan_hdd_clear_concurrency_mode(pHddCtx, pAdapter->device_mode);
    
    if( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
      )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
        pRoamProfile = &pWextState->roamProfile;
        LastBSSType = pRoamProfile->BSSType;

        switch (type)
        {
            case NL80211_IFTYPE_STATION:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_CLIENT:
#endif
                hddLog(VOS_TRACE_LEVEL_INFO, 
                   "%s: setting interface Type to INFRASTRUCTURE", __func__);
                pRoamProfile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
                pRoamProfile->phyMode = 
                hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_STATION) ?
                                WLAN_HDD_INFRA_STATION: WLAN_HDD_P2P_CLIENT;
#endif
                break;
            case NL80211_IFTYPE_ADHOC:
                hddLog(VOS_TRACE_LEVEL_INFO, 
                  "%s: setting interface Type to ADHOC", __func__);
                pRoamProfile->BSSType = eCSR_BSS_TYPE_START_IBSS;
                pRoamProfile->phyMode = 
                    hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
                wdev->iftype = type;
                break;

            case NL80211_IFTYPE_AP:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_GO:
#endif
            {
                hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
                      "%s: setting interface Type to %s", __func__, 
                      (type == NL80211_IFTYPE_AP) ? "SoftAP" : "P2pGo");

                //De-init the adapter.
                hdd_stop_adapter( WLAN_HDD_GET_CTX(pAdapter), pAdapter );
                hdd_deinit_adapter( WLAN_HDD_GET_CTX(pAdapter), pAdapter );
#ifdef WLAN_SOFTAP_FEATURE
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_AP) ? 
                                   WLAN_HDD_SOFTAP : WLAN_HDD_P2P_GO;
#else
                pAdapter->device_mode = WLAN_HDD_SOFTAP;
#endif
                hdd_set_ap_ops( pAdapter->dev );
               
                status = hdd_init_ap_mode(pAdapter);
                if(status != VOS_STATUS_SUCCESS)
                {
                    hddLog(VOS_TRACE_LEVEL_FATAL,"Error initializing the ap mode\n");
                    return -EINVAL;
                }
                hdd_set_conparam(1);

#endif
                /*interface type changed update in wiphy structure*/
                if(wdev) 
                {
                    wdev->iftype = type;
                    (WLAN_HDD_GET_CTX(pAdapter))->change_iface = type;
                }
                else 
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR, 
                            "%s: ERROR !!!! Wireless dev is NULL", __func__);
                    return -EINVAL;
                }
                goto done;
            }

            default:
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported interface Type",
                        __func__);
                return -EOPNOTSUPP;
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
           || (pAdapter->device_mode == WLAN_HDD_P2P_GO) 
#endif
            )
    {
       switch(type)
       {
           case NL80211_IFTYPE_STATION:
#ifdef WLAN_FEATURE_P2P
           case NL80211_IFTYPE_P2P_CLIENT:               
#endif
           case NL80211_IFTYPE_ADHOC:
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_STATION) ?
                                  WLAN_HDD_INFRA_STATION: WLAN_HDD_P2P_CLIENT;
#endif
                hdd_set_conparam(0);
               (WLAN_HDD_GET_CTX(pAdapter))->change_iface = type;
                hdd_deinit_adapter( WLAN_HDD_GET_CTX(pAdapter), pAdapter );

                hdd_set_station_ops( pAdapter->dev );
                status = hdd_init_station_mode( pAdapter );
                if( VOS_STATUS_SUCCESS != status )
                    return -EOPNOTSUPP;
                goto done;
            case NL80211_IFTYPE_AP:
#ifdef WLAN_FEATURE_P2P
            case NL80211_IFTYPE_P2P_GO:
#endif
                wdev->iftype = type;
#ifdef WLAN_FEATURE_P2P
                pAdapter->device_mode = (type == NL80211_IFTYPE_AP) ?
                                        WLAN_HDD_SOFTAP : WLAN_HDD_P2P_GO;
#endif
               goto done;
           default:
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported interface Type",
                        __func__);
                return -EOPNOTSUPP;
               
       }

    }
    else 
    {
      return -EOPNOTSUPP;
    }


    if(pRoamProfile)
    {
        if ( LastBSSType != pRoamProfile->BSSType )
        {
            /*interface type changed update in wiphy structure*/
            wdev->iftype = type;

            /*the BSS mode changed, We need to issue disconnect 
              if connected or in IBSS disconnect state*/
            if ( hdd_connGetConnectedBssType(
                 WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), &connectedBssType ) || 
                ( eCSR_BSS_TYPE_START_IBSS == LastBSSType ) )
            {
                /*need to issue a disconnect to CSR.*/
                INIT_COMPLETION(pAdapter->disconnect_comp_var);
                if( eHAL_STATUS_SUCCESS == 
                        sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter),
                                pAdapter->sessionId,
                                eCSR_DISCONNECT_REASON_UNSPECIFIED ) )
                {
                    wait_for_completion_interruptible_timeout(
                              &pAdapter->disconnect_comp_var,
                              msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
                }
            }
        }
    }

done:
    /*set bitmask based on updated value*/
    wlan_hdd_set_concurrency_mode(pHddCtx, pAdapter->device_mode);
    EXIT();
    return 0;
}

static int wlan_hdd_change_station(struct wiphy *wiphy,
                                         struct net_device *dev,
                                         u8 *mac,
                                         struct station_parameters *params)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    v_MACADDR_t STAMacAddress;

    vos_mem_copy(STAMacAddress.bytes, mac, sizeof(v_MACADDR_t));

    if ( ( pAdapter->device_mode == WLAN_HDD_SOFTAP )
#ifdef WLAN_FEATURE_P2P
      || ( pAdapter->device_mode == WLAN_HDD_P2P_GO )
#endif
       )
    {
        if(params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) 
        {
            status = hdd_softap_change_STA_state( pAdapter, &STAMacAddress, 
                                                  WLANTL_STA_AUTHENTICATED);

            VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                "%s: Station MAC address does not matching", __FUNCTION__);
            return -EINVAL;
        }
    }
    
    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_get_ibss_peer_staidx
 * This function is used to get peer station index in IBSS mode
 */
static u8 wlan_hdd_cfg80211_get_ibss_peer_staidx(hdd_adapter_t* pAdapter)
{
    u8 idx = 0;
    u8 temp[VOS_MAC_ADDR_SIZE] = {0}; 
    ENTER();
    memset(temp, 0, VOS_MAC_ADDR_SIZE);
    for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
    {
        if ( (0 != 
            (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.staId[idx]) 
          && memcmp((u8*)&(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.peerMacAddress[idx],
                                                     temp, VOS_MAC_ADDR_SIZE) 
           )
        {
            return idx;
        }
    }
    return idx;
}


/*
 * FUNCTION: wlan_hdd_cfg80211_add_key
 * This function is used to initialize the key information
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_add_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, bool pairwise,
                                      const u8 *mac_addr,
                                      struct key_params *params
                                      )
#else
static int wlan_hdd_cfg80211_add_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, const u8 *mac_addr,
                                      struct key_params *params
                                      )
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    tCsrRoamSetKey  setKey;
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int status = 0;
    v_U32_t roamId= 0xFF; 
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;  
    hdd_hostapd_state_t *pHostapdState;
    VOS_STATUS vos_status;

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                        __func__,pAdapter->device_mode);

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }
    
    hddLog(VOS_TRACE_LEVEL_INFO, 
            "%s: called with key index = %d & key length %d",
            __func__, key_index, params->key_len);

    /*extract key idx, key len and key*/
    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
    setKey.keyId = key_index;
    setKey.keyLength = params->key_len;
    vos_mem_copy(&setKey.Key[0],params->key, params->key_len);

    switch (params->cipher) 
    {
        case WLAN_CIPHER_SUITE_WEP40:
            setKey.encType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_WEP104:
            setKey.encType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_TKIP:
            {
                u8 *pKey = &setKey.Key[0];
                setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;

                vos_mem_zero(pKey, CSR_MAX_KEY_LEN);

                /*Supplicant sends the 32bytes key in this order 

                  |--------------|----------|----------|
                  |   Tk1        |TX-MIC    |  RX Mic  | 
                  |--------------|----------|----------|
                  <---16bytes---><--8bytes--><--8bytes-->

                 */
                /*Sme expects the 32 bytes key to be in the below order

                  |--------------|----------|----------|
                  |   Tk1        |RX-MIC    |  TX Mic  | 
                  |--------------|----------|----------|
                  <---16bytes---><--8bytes--><--8bytes-->
                 */
                /* Copy the Temporal Key 1 (TK1) */
                vos_mem_copy(pKey, params->key,16);

                /*Copy the rx mic first*/
                vos_mem_copy(&pKey[16],&params->key[24],8); 

                /*Copy the tx mic */
                vos_mem_copy(&pKey[24],&params->key[16],8); 


                break;
            }

        case WLAN_CIPHER_SUITE_CCMP:
            setKey.encType = eCSR_ENCRYPT_TYPE_AES;
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: unsupported cipher type %lu",
                    __func__, params->cipher);
            return -EOPNOTSUPP;
    }

    hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: encryption type %d",
            __func__, setKey.encType);

   
    
    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
        || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       )
    {
        
        if (!mac_addr || is_broadcast_ether_addr(mac_addr)) 
        {
            /* set group key*/
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
        }
        else
        {
            /* set pairwise key*/
            setKey.keyDirection = eSIR_TX_RX;
            vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
        }

        pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
        if( pHostapdState->bssState == BSS_START ) 
        { 
            status = WLANSAP_SetKeySta( pVosContext, &setKey);
        
            if ( status != eHAL_STATUS_SUCCESS )
            {
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "[%4d] WLANSAP_SetKeySta returned ERROR status= %d",
                       __LINE__, status );
            }
        }
        else
        {
             //Save the key in ap context. Issue setkey after the BSS is started.
             hdd_ap_ctx_t *pAPCtx = WLAN_HDD_GET_AP_CTX_PTR(pAdapter); 
             vos_mem_copy(&pAPCtx->groupKey, &setKey, sizeof(tCsrRoamSetKey));
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION) 
#ifdef WLAN_FEATURE_P2P
           || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
            )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        pWextState->roamProfile.Keys.KeyLength[key_index] = (u8)params->key_len;

        vos_mem_copy(&pWextState->roamProfile.Keys.KeyMaterial[key_index][0], 
                params->key, params->key_len);

        pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;

        if (!( (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) 
                    && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType)
             )
                &&
                (  (WLAN_CIPHER_SUITE_WEP40 == params->cipher)
                   || (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
                )
           )
        {
            /* in case of static WEP, macaddr/bssid is not coming from nl80211
             * interface, copy bssid for pairwise key and group macaddr for 
             * group key initialization*/

            tCsrRoamProfile          *pRoamProfile = &pWextState->roamProfile;

            if (eCSR_BSS_TYPE_START_IBSS == pRoamProfile->BSSType)
            {
                /* If IBSS, update the encryption type as we are using default
                 * encryption type as  eCSR_ENCRYPT_TYPE_WEP40_STATICKEY 
                 * during ibss join*/
                pWextState->roamProfile.negotiatedUCEncryptionType = 
                    pHddStaCtx->conn_info.ucEncryptionType = 
                    ((WLAN_CIPHER_SUITE_WEP40 == params->cipher) ?
                    eCSR_ENCRYPT_TYPE_WEP40_STATICKEY :
                     eCSR_ENCRYPT_TYPE_WEP104_STATICKEY);

                hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                        "%s: IBSS - negotiated encryption type %d", __func__, 
                        pWextState->roamProfile.negotiatedUCEncryptionType);
            }
            else 
            {
                pWextState->roamProfile.negotiatedUCEncryptionType = 
                    pHddStaCtx->conn_info.ucEncryptionType;

                hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                        "%s: BSS - negotiated encryption type %d", __func__, 
                        pWextState->roamProfile.negotiatedUCEncryptionType);
            }

            sme_SetCfgPrivacy((tpAniSirGlobal)WLAN_HDD_GET_HAL_CTX(pAdapter),
                    &pWextState->roamProfile, true);
            setKey.keyLength = 0;
            setKey.keyDirection =  eSIR_TX_RX;

            if (mac_addr)
            {
                vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
            }
            else
            {
                /* macaddr is NULL, set the peerMac to bssId in case of BSS, 
                 * and peerMacAddress in case of IBSS*/
                if (eCSR_BSS_TYPE_START_IBSS == pRoamProfile->BSSType)
                {
                    u8 staidx = wlan_hdd_cfg80211_get_ibss_peer_staidx(pAdapter);
                    if (HDD_MAX_NUM_IBSS_STA != staidx)
                    {
                        vos_mem_copy(setKey.peerMac,
                                &pHddStaCtx->conn_info.peerMacAddress[staidx],
                                WNI_CFG_BSSID_LEN);

                    } 
                    else
                    {
                        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No peerMac found", 
                                __func__);
                        return -EOPNOTSUPP;
                    } 
                }
                else
                {
                    vos_mem_copy(setKey.peerMac,
                            &pHddStaCtx->conn_info.bssId[0],
                            WNI_CFG_BSSID_LEN);
                }
            }
        }
        else if (!mac_addr || is_broadcast_ether_addr(mac_addr)) 
        {
            /* set group key*/
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
        }
        else
        {
            /* set pairwise key*/
            setKey.keyDirection = eSIR_TX_RX;
            vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                "%s: set key for peerMac %2x:%2x:%2x:%2x:%2x:%2x, direction %d",
                __func__, setKey.peerMac[0], setKey.peerMac[1], 
                setKey.peerMac[2], setKey.peerMac[3], 
                setKey.peerMac[4], setKey.peerMac[5], 
                setKey.keyDirection);

        vos_status = wlan_hdd_check_ula_done(pAdapter);

        if ( vos_status != VOS_STATUS_SUCCESS )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "[%4d] wlan_hdd_check_ula_done returned ERROR status= %d",
                    __LINE__, vos_status );

            pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;

            return -EINVAL;

        }


        /* issue set key request to SME*/
        status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter),
                pAdapter->sessionId, &setKey, &roamId );

        if ( 0 != status )
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: sme_RoamSetKey failed, returned %d", __func__, status);
            pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
            return -EINVAL;
        }


        /* in case of IBSS as there was no information available about WEP keys during 
         * IBSS join, group key intialized with NULL key, so re-initialize group key 
         * with correct value*/
        if ( (eCSR_BSS_TYPE_START_IBSS == pWextState->roamProfile.BSSType) && 
                !(  (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) 
                    && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType)
                 )
                &&
                (  (WLAN_CIPHER_SUITE_WEP40 == params->cipher)
                   || (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
                )
           )
        {
            setKey.keyDirection = eSIR_RX_ONLY;
            vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);

            hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                    "%s: set key peerMac %2x:%2x:%2x:%2x:%2x:%2x, direction %d",
                    __func__, setKey.peerMac[0], setKey.peerMac[1], 
                    setKey.peerMac[2], setKey.peerMac[3], 
                    setKey.peerMac[4], setKey.peerMac[5], 
                    setKey.keyDirection);

            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                    pAdapter->sessionId, &setKey, &roamId );

            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failed for group key (IBSS), returned %d", 
                        __func__, status);
                pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
                return -EINVAL;
            }
        }
    }

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_get_key
 * This function is used to get the key information
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_get_key( 
                        struct wiphy *wiphy, 
                        struct net_device *ndev,
                        u8 key_index, bool pairwise, 
                        const u8 *mac_addr, void *cookie,
                        void (*callback)(void *cookie, struct key_params*)
                        )
#else
static int wlan_hdd_cfg80211_get_key( 
                        struct wiphy *wiphy, 
                        struct net_device *ndev,
                        u8 key_index, const u8 *mac_addr, void *cookie,
                        void (*callback)(void *cookie, struct key_params*)
                        )
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    hdd_wext_state_t *pWextState= WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
    struct key_params params;

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                 __func__,pAdapter->device_mode);
    
    memset(&params, 0, sizeof(params));

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        return -EINVAL;
    }   

    switch(pRoamProfile->EncryptionType.encryptionType[0])
    {
        case eCSR_ENCRYPT_TYPE_NONE:
            params.cipher = IW_AUTH_CIPHER_NONE;
            break;

        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
        case eCSR_ENCRYPT_TYPE_WEP40:
            params.cipher = WLAN_CIPHER_SUITE_WEP40;
            break;

        case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
        case eCSR_ENCRYPT_TYPE_WEP104:
            params.cipher = WLAN_CIPHER_SUITE_WEP104;
            break;

        case eCSR_ENCRYPT_TYPE_TKIP:
            params.cipher = WLAN_CIPHER_SUITE_TKIP;
            break;

        case eCSR_ENCRYPT_TYPE_AES:
            params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
            break;

        default:
            params.cipher = IW_AUTH_CIPHER_NONE;
            break;
    }

    params.key_len = pRoamProfile->Keys.KeyLength[key_index];
    params.seq_len = 0;
    params.seq = NULL;
    params.key = &pRoamProfile->Keys.KeyMaterial[key_index][0];
    callback(cookie, &params);
    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_del_key
 * This function is used to delete the key information
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_del_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, 
                                      bool pairwise, 
                                      const u8 *mac_addr
                                    )
#else
static int wlan_hdd_cfg80211_del_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index,
                                      const u8 *mac_addr
                                    )
#endif
{
    int status = 0;

    //This code needs to be revisited. There is sme_removeKey API, we should
    //plan to use that. After the change to use correct index in setkey, 
    //it is observed that this is invalidating peer
    //key index whenever re-key is done. This is affecting data link.
    //It should be ok to ignore del_key.
#if 0
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;  
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    tCsrRoamSetKey  setKey;
    v_U32_t roamId= 0xFF;
    
    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: device_mode = %d\n",
                                     __func__,pAdapter->device_mode);

    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
    setKey.keyId = key_index;

    if (mac_addr)
        vos_mem_copy(setKey.peerMac, mac_addr,WNI_CFG_BSSID_LEN);
    else
        vos_mem_copy(setKey.peerMac, groupmacaddr, WNI_CFG_BSSID_LEN);

    setKey.encType = eCSR_ENCRYPT_TYPE_NONE;

    if ((pAdapter->device_mode == WLAN_HDD_SOFTAP)
#ifdef WLAN_FEATURE_P2P
      || (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
       ) 
    { 
       
        hdd_hostapd_state_t *pHostapdState =  
                                  WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);
        if( pHostapdState->bssState == BSS_START)
        {
            status = WLANSAP_SetKeySta( pVosContext, &setKey);
    
            if ( status != eHAL_STATUS_SUCCESS )
            {
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "[%4d] WLANSAP_SetKeySta returned ERROR status= %d",
                     __LINE__, status );
            }
        }
    }
    else if ( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
           || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT) 
#endif
            )
    {
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;   
    
        hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                "%s: delete key for peerMac %2x:%2x:%2x:%2x:%2x:%2x",
                __func__, setKey.peerMac[0], setKey.peerMac[1], 
                setKey.peerMac[2], setKey.peerMac[3], 
                setKey.peerMac[4], setKey.peerMac[5]);
        if(pAdapter->sessionCtx.station.conn_info.connState == 
                                       eConnectionState_Associated) 
        {
            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                   pAdapter->sessionId, &setKey, &roamId );
        
            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failure, returned %d",
                                                     __func__, status);
                pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
                return -EINVAL;
            }
        }
    }
#endif
    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_default_key
 * This function is used to set the default tx key index
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_cfg80211_set_default_key( struct wiphy *wiphy,
                                              struct net_device *ndev,
                                              u8 key_index,
                                              bool unicast, bool multicast)
#else
static int wlan_hdd_cfg80211_set_default_key( struct wiphy *wiphy,
                                              struct net_device *ndev,
                                              u8 key_index)
#endif
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    int status = 0;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                         __func__,pAdapter->device_mode);
   
    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    
    if ((pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
       ) 
    {
        if ( (key_index != pWextState->roamProfile.Keys.defaultIndex) && 
             (eCSR_ENCRYPT_TYPE_TKIP != 
                pWextState->roamProfile.EncryptionType.encryptionType[0]) &&
             (eCSR_ENCRYPT_TYPE_AES != 
                pWextState->roamProfile.EncryptionType.encryptionType[0])
           )
        {  
            /* if default key index is not same as previous one, 
             * then update the default key index */
    
            tCsrRoamSetKey  setKey;
            v_U32_t roamId= 0xFF;
            tCsrKeys *Keys = &pWextState->roamProfile.Keys;
            
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: default tx key index %d", 
                    __func__, key_index);
            
            Keys->defaultIndex = (u8)key_index;
            vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
            setKey.keyId = key_index;
            setKey.keyLength = Keys->KeyLength[key_index];
            
            vos_mem_copy(&setKey.Key[0], 
                    &Keys->KeyMaterial[key_index][0], 
                    Keys->KeyLength[key_index]);
    
            setKey.keyDirection = eSIR_TX_ONLY;
    
            vos_mem_copy(setKey.peerMac, 
                    &pHddStaCtx->conn_info.bssId[0],
                    WNI_CFG_BSSID_LEN);
    
            setKey.encType = 
                pWextState->roamProfile.EncryptionType.encryptionType[0];
             
            /* issue set key request */
            status = sme_RoamSetKey( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                   pAdapter->sessionId, &setKey, &roamId );
    
            if ( 0 != status )
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: sme_RoamSetKey failed, returned %d", __func__, 
                        status);
                return -EINVAL;
            }
        }
    }
    return status;
}

/**
 * FUNCTION: wlan_hdd_cfg80211_set_channel
 * This function is used to set the channel number 
 */
int wlan_hdd_cfg80211_set_channel( struct wiphy *wiphy, struct net_device *dev,
                                   struct ieee80211_channel *chan,
                                   enum nl80211_channel_type channel_type
                                 )
{
    v_U32_t num_ch = 0;
    u8 valid_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN];    
    u32 indx = 0;
    u32 channel = 0;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    int freq = chan->center_freq; /* freq is in MHZ */ 
 
    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, 
                "%s: device_mode = %d  freq = %d \n",__func__, 
                            pAdapter->device_mode, chan->center_freq);

    /* 
     * Do freq to chan conversion
     * TODO: for 11a
     */

    channel = ieee80211_frequency_to_channel(freq);

    /* Check freq range */
    if ((WNI_CFG_CURRENT_CHANNEL_STAMIN > channel) || 
            (WNI_CFG_CURRENT_CHANNEL_STAMAX < channel)) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: Channel [%d] is outside valid range from %d to %d\n",
                __func__, channel, WNI_CFG_CURRENT_CHANNEL_STAMIN,
                WNI_CFG_CURRENT_CHANNEL_STAMAX);
        return -EINVAL;
    }

    num_ch = WNI_CFG_VALID_CHANNEL_LIST_LEN;

    if (0 != ccmCfgGetStr(hHal, WNI_CFG_VALID_CHANNEL_LIST, 
                valid_ch, &num_ch))
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: failed to get valid channel list\n", __func__);
        return -EIO;
    }

    for (indx = 0; indx < num_ch; indx++) 
    {
        if (channel == valid_ch[indx])
        {
            break;
        }
    }

    if (indx >= num_ch)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: Invalid Channel [%d] \n", __func__, channel);
        return -EINVAL;
    }
    
    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
             "%s: set channel to [%d] for device mode =%d", 
                          __func__, channel,pAdapter->device_mode);

    if( (pAdapter->device_mode == WLAN_HDD_INFRA_STATION)
#ifdef WLAN_FEATURE_P2P
     || (pAdapter->device_mode == WLAN_HDD_P2P_CLIENT)
#endif
      )
    {
        hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter); 
        tCsrRoamProfile * pRoamProfile = &pWextState->roamProfile;
        hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

        if (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState)
        {
           /* Link is up then return cant set channel*/
            hddLog( VOS_TRACE_LEVEL_ERROR, 
                   "%s: IBSS Associated, can't set the channel\n", __func__);
            return -EINVAL;
        }

        num_ch = pRoamProfile->ChannelInfo.numOfChannels = 1;
        pHddStaCtx->conn_info.operationChannel = channel; 
        pRoamProfile->ChannelInfo.ChannelList = 
            &pHddStaCtx->conn_info.operationChannel; 
    }
    else if ((pAdapter->device_mode == WLAN_HDD_SOFTAP) 
#ifdef WLAN_FEATURE_P2P
        ||   (pAdapter->device_mode == WLAN_HDD_P2P_GO)
#endif
            ) 
    {
        (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->sapConfig.channel = channel;
    }
    else 
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, 
           "%s: Invalid device mode failed to set valid channel\n", __func__);
        return -EINVAL;
    }
    EXIT();
    return 0;
}



/*
 * FUNCTION: wlan_hdd_cfg80211_inform_bss
 * This function is used to inform the BSS details to nl80211 interface.
 */
static struct cfg80211_bss* wlan_hdd_cfg80211_inform_bss(
                    hdd_adapter_t *pAdapter, tSirBssDescription *bss_desc)
{
    struct net_device *dev = pAdapter->dev;
    struct wireless_dev *wdev = dev->ieee80211_ptr;
    struct wiphy *wiphy = wdev->wiphy;
    int chan_no = bss_desc->channelId;
    int ie_length = GET_IE_LEN_IN_BSS_DESC( bss_desc->length );
    const char *ie = 
        ((ie_length != 0) ? (const char *)&bss_desc->ieFields: NULL);
    unsigned int freq;
    struct ieee80211_channel *chan;
    int rssi = 0;

    ENTER();

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
    if (chan_no <= ARRAY_SIZE(hdd_channels_2_4_GHZ))
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_2GHZ);
    }
    else
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_5GHZ);
    }
#else
    freq = ieee80211_channel_to_frequency(chan_no);
#endif

    chan = __ieee80211_get_channel(wiphy, freq);

    rssi = (VOS_MIN ((bss_desc->rssi + bss_desc->sinr), 0))*100;

    return (cfg80211_inform_bss(wiphy, chan, bss_desc->bssId, 
                le64_to_cpu(*(__le64 *)bss_desc->timeStamp), 
                bss_desc->capabilityInfo,
                bss_desc->beaconInterval, ie, ie_length,
                rssi, GFP_KERNEL ));
}



/*
 * FUNCTION: wlan_hdd_cfg80211_inform_bss_frame
 * This function is used to inform the BSS details to nl80211 interface.
 */
struct cfg80211_bss*
wlan_hdd_cfg80211_inform_bss_frame( hdd_adapter_t *pAdapter,
                                    tSirBssDescription *bss_desc
                                    )
{
    /*
      cfg80211_inform_bss() is not updating ie field of bss entry, if entry
      already exists in bss data base of cfg80211 for that particular BSS ID.
      Using cfg80211_inform_bss_frame to update the bss entry instead of
      cfg80211_inform_bss, But this call expects mgmt packet as input. As of
      now there is no possibility to get the mgmt(probe response) frame from PE,
      converting bss_desc to ieee80211_mgmt(probe response) and passing to
      cfg80211_inform_bss_frame.
     */
    struct net_device *dev = pAdapter->dev;
    struct wireless_dev *wdev = dev->ieee80211_ptr;
    struct wiphy *wiphy = wdev->wiphy;
    int chan_no = bss_desc->channelId;
    int ie_length = GET_IE_LEN_IN_BSS_DESC( bss_desc->length );
    const char *ie =
        ((ie_length != 0) ? (const char *)&bss_desc->ieFields: NULL);
    unsigned int freq;
    struct ieee80211_channel *chan;
    struct ieee80211_mgmt *mgmt =
        kzalloc((sizeof (struct ieee80211_mgmt) + ie_length), GFP_KERNEL);
    struct cfg80211_bss *bss_status = NULL;
    size_t frame_len = sizeof (struct ieee80211_mgmt) + ie_length;
    int rssi = 0;

    ENTER();

    memcpy(mgmt->bssid, bss_desc->bssId, ETH_ALEN);
    memcpy(&mgmt->u.probe_resp.timestamp, bss_desc->timeStamp,
            sizeof (bss_desc->timeStamp));
    mgmt->u.probe_resp.beacon_int = bss_desc->beaconInterval;
    mgmt->u.probe_resp.capab_info = bss_desc->capabilityInfo;
    memcpy(mgmt->u.probe_resp.variable, ie, ie_length);

    mgmt->frame_control |=
        (u16)(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
    if (chan_no <= ARRAY_SIZE(hdd_channels_2_4_GHZ))
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_2GHZ);
    }
    else
    {
        freq = ieee80211_channel_to_frequency(chan_no, IEEE80211_BAND_5GHZ);
    }
#else
    freq = ieee80211_channel_to_frequency(chan_no);
#endif

    chan = __ieee80211_get_channel(wiphy, freq);

    /*To keep the rssi icon of the connected AP in the scan window
    *and the rssi icon of the wireless networks in sync 
    * */
    if (( eConnectionState_Associated ==
             pAdapter->sessionCtx.station.conn_info.connState ) &&
             ( VOS_TRUE == vos_mem_compare(bss_desc->bssId,
                             pAdapter->sessionCtx.station.conn_info.bssId,
                             WNI_CFG_BSSID_LEN)))
    {
       /* supplicant takes the signal strength in terms of mBm(100*dBm) */
       rssi = (pAdapter->rssi * 100);
    }
    else
    {
       rssi = (VOS_MIN ((bss_desc->rssi + bss_desc->sinr), 0))*100;
    }

    bss_status = cfg80211_inform_bss_frame(wiphy, chan, mgmt,
            frame_len, rssi, GFP_KERNEL);
    kfree(mgmt);
    return bss_status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_update_bss_db
 * This function is used to update the BSS data base of CFG8011
 */
void wlan_hdd_cfg80211_update_bss_db( hdd_adapter_t *pAdapter, 
                                      tCsrRoamInfo *pRoamInfo
                                      )
{
    tCsrRoamConnectedProfile roamProfile;
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);

    ENTER();

    memset(&roamProfile, 0, sizeof(tCsrRoamConnectedProfile));
    sme_RoamGetConnectProfile(hHal, pAdapter->sessionId, &roamProfile);

    if (NULL != roamProfile.pBssDesc)
    {
        struct cfg80211_bss *bss_status = NULL;
        bss_status = wlan_hdd_cfg80211_inform_bss(pAdapter, 
                roamProfile.pBssDesc);

        if (NULL == bss_status)
        {
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: cfg80211_inform_bss return NULL",
                    __func__);
        }

        sme_RoamFreeConnectProfile(hHal, &roamProfile);
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s:  roamProfile.pBssDesc is NULL",
                __func__);
    }
}

/*
 * FUNCTION: wlan_hdd_cfg80211_update_bss
 */
static int wlan_hdd_cfg80211_update_bss( struct wiphy *wiphy, 
                                         hdd_adapter_t *pAdapter 
                                        )
{   
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    tCsrScanResultInfo *pScanResult;
    eHalStatus status = 0;
    tScanResultHandle pResult;
    struct cfg80211_bss *bss_status = NULL;

    ENTER();

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
      return -EAGAIN;
    }

    /*
     * start getting scan results and populate cgf80211 BSS database
     */
    status = sme_ScanGetResult(hHal, pAdapter->sessionId, NULL, &pResult);

    /* no scan results */
    if (NULL == pResult)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: No scan result\n", __func__);
        return status;
    }

    pScanResult = sme_ScanResultGetFirst(hHal, pResult);

    while (pScanResult)
    {
#if 0
        bss_status = wlan_hdd_cfg80211_inform_bss(pAdapter,
                &pScanResult->BssDescriptor);
#else
        /* 
         * cfg80211_inform_bss() is not updating ie field of bss entry, if 
         * entry already exists in bss data base of cfg80211 for that 
         * particular BSS ID.  Using cfg80211_inform_bss_frame to update the 
         * bss entry instead of cfg80211_inform_bss, But this call expects 
         * mgmt packet as input. As of now there is no possibility to get 
         * the mgmt(probe response) frame from PE, converting bss_desc to 
         * ieee80211_mgmt(probe response) and passing to c
         * fg80211_inform_bss_frame.
         * */

        bss_status = wlan_hdd_cfg80211_inform_bss_frame(pAdapter,
                &pScanResult->BssDescriptor);
#endif
    

        if (NULL == bss_status)
        {
            hddLog(VOS_TRACE_LEVEL_INFO,
                    "%s: NULL returned by cfg80211_inform_bss\n", __func__);
            break;
        }
        pScanResult = sme_ScanResultGetNext(hHal, pResult);
    }

    sme_ScanResultPurge(hHal, pResult); 

    return 0; 
}

/*
 * FUNCTION: hdd_cfg80211_scan_done_callback
 * scanning callback function, called after finishing scan
 *
 */
static eHalStatus hdd_cfg80211_scan_done_callback(tHalHandle halHandle, 
        void *pContext, tANI_U32 scanId, eCsrScanStatus status)
{
    struct net_device *dev = (struct net_device *) pContext;
    //struct wireless_dev *wdev = dev->ieee80211_ptr;    
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    struct cfg80211_scan_request *req = pAdapter->request;
    int ret = 0;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO,
            "%s called with halHandle = %p, pContext = %p,"
            "scanID = %d, returned status = %d\n", 
            __func__, halHandle, pContext, (int) scanId, (int) status);

    if(pwextBuf->mScanPending != VOS_TRUE)
    {
        VOS_ASSERT(pwextBuf->mScanPending);
        return 0;
    }

    /* Check the scanId */
    if (pwextBuf->scanId != scanId) 
    {
        hddLog(VOS_TRACE_LEVEL_INFO,
                "%s called with mismatched scanId pWextState->scanId = %d "
                "scanId = %d \n", __func__, (int) pwextBuf->scanId, 
                (int) scanId);
    }

    /* Scan is no longer pending */
    pwextBuf->mScanPending = VOS_FALSE;

    ret = wlan_hdd_cfg80211_update_bss((WLAN_HDD_GET_CTX(pAdapter))->wiphy, 
                                        pAdapter);

    if (0 > ret)
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: NO SCAN result", __func__);    

    if (!pAdapter->request)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "request is became NULL\n");
        return 0;
    }

    /*
     * setting up 0, just in case.
     */  
    req->n_ssids = 0;
    req->n_channels = 0;
    req->ie = 0;

    /*
     * cfg80211_scan_done informing NL80211 about completion
     * of scanning
     */
    cfg80211_scan_done(pAdapter->request, false);
    complete(&pAdapter->abortscan_event_var);
    pAdapter->request = NULL;

#ifdef WLAN_FEATURE_P2P
    /* Flush out scan result after p2p_serach is done */
    if( pwextBuf->p2pSearch )
    {
        sme_ScanFlushResult(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId);
        pwextBuf->p2pSearch = 0;
    }
#endif

    EXIT();
    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_scan
 * this scan respond to scan trigger and update cfg80211 scan database
 * later, scan dump command can be used to recieve scan results
 */
int wlan_hdd_cfg80211_scan( struct wiphy *wiphy, struct net_device *dev,
        struct cfg80211_scan_request *request)
{  
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    hdd_config_t *cfg_param = (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini;
    tCsrScanRequest scanRequest;
    tANI_U8 *channelList = NULL, i;
    v_U32_t scanId = 0;
    int status = 0;
#ifdef WLAN_FEATURE_P2P
    v_U8_t* pP2pIe = NULL;
#endif

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",
                                   __func__,pAdapter->device_mode);
    hddLog(VOS_TRACE_LEVEL_INFO, "scan request for ssid = %d\n", 
            (int)request->n_ssids);  

    //Scan on any other interface is not supported.
    if((pAdapter->device_mode != WLAN_HDD_INFRA_STATION) 
#ifdef WLAN_FEATURE_P2P
       && (pAdapter->device_mode != WLAN_HDD_P2P_CLIENT)
#endif
      )
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: Not scanning on device_mode = %d\n",
                                    __func__,pAdapter->device_mode);
        return -EOPNOTSUPP;
    }

    if (TRUE == pwextBuf->mScanPending)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: mScanPending is TRUE\n", __func__);
        return -EBUSY;                  
    }

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                        "%s:LOGP in Progress. Ignore!!!",__func__);
      return -EAGAIN;
    }

    vos_mem_zero( &scanRequest, sizeof(scanRequest));

    if (request)
    {
        /* Even though supplicant doesn't provide any SSIDs, n_ssids is set to 1.
         * Becasue of this, driver is assuming that this is not wildcard scan and so
         * is not aging out the scan results.
         */
        if ('\0' == request->ssids->ssid[0])
        {
            request->n_ssids = 0;
        }

        if (0 < request->n_ssids)
        {
            tCsrSSIDInfo *SsidInfo;
            int j;
            scanRequest.SSIDs.numOfSSIDs = request->n_ssids;
            /* Allocate num_ssid tCsrSSIDInfo structure */
            SsidInfo = scanRequest.SSIDs.SSIDList =
                      ( tCsrSSIDInfo *)vos_mem_malloc(
                              request->n_ssids*sizeof(tCsrSSIDInfo));

            if(NULL == scanRequest.SSIDs.SSIDList)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,
                               "memory alloc failed SSIDInfo buffer");
                return -ENOMEM;
            }

            /* copy all the ssid's and their length */
            for(j = 0; j < request->n_ssids; j++, SsidInfo++)
            {
                /* get the ssid length */
                SsidInfo->SSID.length = request->ssids[j].ssid_len;
                vos_mem_copy(SsidInfo->SSID.ssId, &request->ssids[j].ssid[0],
                             SsidInfo->SSID.length);
                hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "SSID number %d:  %s\n",
                                                   j, SsidInfo->SSID.ssId);
            }
        }

        /*Set the scan type to default type, in this case it is ACTIVE*/
        scanRequest.scanType = 
                    (WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter))->scan_mode;
        scanRequest.minChnTime = cfg_param->nActiveMinChnTime; 
        scanRequest.maxChnTime = cfg_param->nActiveMaxChnTime;
    }
    else
    {
        /* set the scan type to active */
        scanRequest.scanType = eSIR_ACTIVE_SCAN;
        vos_mem_set( scanRequest.bssid, sizeof( tCsrBssid ), 0xff );

        /* set min and max channel time to zero */
        scanRequest.minChnTime = 0;
        scanRequest.maxChnTime = 0;
    }

    /* set BSSType to default type */
    scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

    /*TODO: scan the requested channels only*/

    /*Right now scanning all the channels */
    if( request->n_channels )
    {
        channelList = vos_mem_malloc( request->n_channels );
        if( NULL == channelList )
        {
            status = -ENOMEM;
            goto free_mem;
        }
  
        for( i = 0 ; i < request->n_channels ; i++ )
            channelList[i] = request->channels[i]->hw_value;
    }

    scanRequest.ChannelInfo.numOfChannels = request->n_channels;
    scanRequest.ChannelInfo.ChannelList = channelList;

    /* set requestType to full scan */
    scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

    if( request->ie_len )
    {
        /* save this for future association (join requires this) */
        memset( &pwextBuf->scanAddIE, 0, sizeof(pwextBuf->scanAddIE) );
        memcpy( pwextBuf->scanAddIE.addIEdata, request->ie, request->ie_len);
        pwextBuf->scanAddIE.length = request->ie_len;

        pwextBuf->roamProfile.pAddIEScan = pwextBuf->scanAddIE.addIEdata;
        pwextBuf->roamProfile.nAddIEScanLength = pwextBuf->scanAddIE.length;
        
        scanRequest.uIEFieldLen = pwextBuf->roamProfile.nAddIEScanLength;
        scanRequest.pIEField = pwextBuf->roamProfile.pAddIEScan;

#ifdef WLAN_FEATURE_P2P
        pP2pIe = wlan_hdd_get_p2p_ie_ptr((v_U8_t*)request->ie,
                                                   request->ie_len);
        if (pP2pIe != NULL)
        {
            /*
             * If Number of channel to scan are 3 and channels 
             * to be scan are 1, 6, 11, with p2p IE is provided in 
             * scan, then it is P2P search.
             */ 
            if ( (request->n_channels == 3) &&
                 ( (channelList[0]== 1) && (channelList[1] == 6 )
                && (channelList[2] == 11))
                )
            {
                hddLog(VOS_TRACE_LEVEL_INFO,
                       "%s: This is a P2P Search", __func__);
                scanRequest.p2pSearch = 1;
                pwextBuf->p2pSearch = 1;

                /* set requestType to P2P Discovery */
                scanRequest.requestType = eCSR_SCAN_P2P_DISCOVERY;

                sme_ScanFlushResult( WLAN_HDD_GET_HAL_CTX(pAdapter),
                                     pAdapter->sessionId );
            }
        }
#endif
    }

    status = sme_ScanRequest( WLAN_HDD_GET_HAL_CTX(pAdapter),
                              pAdapter->sessionId, &scanRequest, &scanId,
                              &hdd_cfg80211_scan_done_callback, dev );

    if (eHAL_STATUS_SUCCESS != status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
                "%s: sme_ScanRequest returned error %d", __func__, status);
        status = -EIO;
        goto free_mem;
    }

    pwextBuf->mScanPending = TRUE;
    pAdapter->request = request;
    pwextBuf->scanId = scanId;

free_mem:
    if( scanRequest.SSIDs.SSIDList )
    {
        vos_mem_free(scanRequest.SSIDs.SSIDList);
    }

    if( channelList )
      vos_mem_free( channelList );

    EXIT();

    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_connect_start
 * This function is used to start the association process 
 */
int wlan_hdd_cfg80211_connect_start( hdd_adapter_t  *pAdapter, 
        const u8 *ssid, size_t ssid_len, const u8 *bssid)
{
    int status = 0;
    hdd_wext_state_t *pWextState;
    v_U32_t roamId;
    tCsrRoamProfile *pRoamProfile;
    eMib_dot11DesiredBssType connectedBssType;
    eCsrAuthType RSNAuthType;

    ENTER();

    pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    
    if (SIR_MAC_MAX_SSID_LENGTH < ssid_len)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wrong SSID len", __func__);
        return -EINVAL;
    }

    pRoamProfile = &pWextState->roamProfile;

    if (pRoamProfile) 
    {
        if (hdd_connGetConnectedBssType(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter),
            &connectedBssType ) || ( eMib_dot11DesiredBssType_independent == 
              connectedBssType))
        {
            /* Issue disconnect to CSR */
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            if( eHAL_STATUS_SUCCESS == 
                  sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter),
                            pAdapter->sessionId,
                            eCSR_DISCONNECT_REASON_UNSPECIFIED ) )
            {
                wait_for_completion_interruptible_timeout(
                        &pAdapter->disconnect_comp_var,
                        msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
            }
        }

        if (HDD_WMM_USER_MODE_NO_QOS == 
                        (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini->WmmMode)
        {
            /*QoS not enabled in cfg file*/
            pRoamProfile->uapsd_mask = 0;
        }
        else
        {
            /*QoS enabled, update uapsd mask from cfg file*/
            pRoamProfile->uapsd_mask = 
                     (WLAN_HDD_GET_CTX(pAdapter))->cfg_ini->UapsdMask;
        }

        pRoamProfile->SSIDs.numOfSSIDs = 1;
        pRoamProfile->SSIDs.SSIDList->SSID.length = ssid_len;
        vos_mem_zero(pRoamProfile->SSIDs.SSIDList->SSID.ssId,
                sizeof(pRoamProfile->SSIDs.SSIDList->SSID.ssId)); 
        vos_mem_copy((void *)(pRoamProfile->SSIDs.SSIDList->SSID.ssId),
                ssid, ssid_len);

        if (bssid)
        {
            pRoamProfile->BSSIDs.numOfBSSIDs = 1;
            vos_mem_copy((void *)(pRoamProfile->BSSIDs.bssid), bssid,
                    WNI_CFG_BSSID_LEN);
            /* Save BSSID in seperate variable as well, as RoamProfile 
               BSSID is getting zeroed out in the association process. And in 
               case of join failure we should send valid BSSID to supplicant
             */
            vos_mem_copy((void *)(pWextState->req_bssId), bssid,
                    WNI_CFG_BSSID_LEN);
        }

        if ((IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion) ||
                (IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion))
        { 
            /*set gen ie*/
            hdd_SetGENIEToCsr(pAdapter, &RSNAuthType);
            /*set auth*/
            hdd_set_csr_auth_type(pAdapter, RSNAuthType);
        }
        
        pRoamProfile->csrPersona = pAdapter->device_mode;

        status = sme_RoamConnect( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                            pAdapter->sessionId, pRoamProfile, &roamId);

        pRoamProfile->ChannelInfo.ChannelList = NULL; 
        pRoamProfile->ChannelInfo.numOfChannels = 0;
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid Roam profile", __func__);
        return -EINVAL;
    }
    EXIT(); 
    return status;
}

/*
 * FUNCTION: wlan_hdd_set_cfg80211_auth_type
 * This function is used to set the authentication type (OPEN/SHARED).
 *
 */
static int wlan_hdd_cfg80211_set_auth_type(hdd_adapter_t *pAdapter,
        enum nl80211_auth_type auth_type)
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    /*set authentication type*/
    switch (auth_type) 
    {
        case NL80211_AUTHTYPE_OPEN_SYSTEM:
        case NL80211_AUTHTYPE_AUTOMATIC:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to OPEN", __func__);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
            break;

        case NL80211_AUTHTYPE_SHARED_KEY:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to SHARED", __func__);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Unsupported authentication type %d", __func__, 
                    auth_type);
            pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_UNKNOWN;
            return -EINVAL;
    }

    pWextState->roamProfile.AuthType.authType[0] = 
                                        pHddStaCtx->conn_info.authType;
    return 0;
}

/*
 * FUNCTION: wlan_hdd_set_akm_suite
 * This function is used to set the key mgmt type(PSK/8021x).
 *
 */
static int wlan_hdd_set_akm_suite( hdd_adapter_t *pAdapter, 
                                   u32 key_mgmt
                                   )
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    ENTER();
    
    /*set key mgmt type*/
    switch(key_mgmt)
    {
        case WLAN_AKM_SUITE_PSK:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting key mgmt type to PSK", 
                    __func__);
            pWextState->authKeyMgmt = IW_AUTH_KEY_MGMT_PSK;
            break;

        case WLAN_AKM_SUITE_8021X:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting key mgmt type to 8021x", 
                    __func__);
            pWextState->authKeyMgmt = IW_AUTH_KEY_MGMT_802_1X;
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported key mgmt type %d", 
                    __func__, key_mgmt);
            return -EINVAL;

    }
    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_cipher
 * This function is used to set the encryption type 
 * (NONE/WEP40/WEP104/TKIP/CCMP).
 */
static int wlan_hdd_cfg80211_set_cipher( hdd_adapter_t *pAdapter, 
                                u32 cipher, 
                                bool ucast
                                )
{
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();

    if (!cipher) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid value for cipher type %d", 
                __func__, cipher);
        encryptionType = eCSR_ENCRYPT_TYPE_FAILED;
        return 0;
    }

    /*set encryption method*/
    switch (cipher) 
    {
        case IW_AUTH_CIPHER_NONE:
            encryptionType = eCSR_ENCRYPT_TYPE_NONE;
            break;

        case WLAN_CIPHER_SUITE_WEP40:
            if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
                encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
            else
                encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_WEP104:
            if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                (eCSR_AUTH_TYPE_OPEN_SYSTEM == pHddStaCtx->conn_info.authType))
                encryptionType = eCSR_ENCRYPT_TYPE_WEP104;
            else
                encryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_TKIP:
            encryptionType = eCSR_ENCRYPT_TYPE_TKIP;
            break;

        case WLAN_CIPHER_SUITE_CCMP:
            encryptionType = eCSR_ENCRYPT_TYPE_AES;
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported cipher type %d", 
                    __func__, cipher);
            return -EOPNOTSUPP;
    }

    if (ucast)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting unicast cipher type to %d", 
                __func__, encryptionType);
        pHddStaCtx->conn_info.ucEncryptionType            = encryptionType;
        pWextState->roamProfile.EncryptionType.numEntries = 1;
        pWextState->roamProfile.EncryptionType.encryptionType[0] = 
                                          encryptionType;
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting mcast cipher type to %d", 
                __func__, encryptionType);
        pHddStaCtx->conn_info.mcEncryptionType                       = encryptionType;
        pWextState->roamProfile.mcEncryptionType.numEntries        = 1;
        pWextState->roamProfile.mcEncryptionType.encryptionType[0] = encryptionType;
    }

    return 0;
}


/*
 * FUNCTION: wlan_hdd_cfg80211_set_ie
 * This function is used to parse WPA/RSN IE's.
 */
int wlan_hdd_cfg80211_set_ie( hdd_adapter_t *pAdapter, 
                              u8 *ie, 
                              size_t ie_len
                              )
{
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    u8 *genie = ie;
    v_U16_t remLen = ie_len;
    ENTER();

    /* clear previous assocAddIE */
    pWextState->assocAddIE.length = 0;
    pWextState->roamProfile.bWPSAssociation = VOS_FALSE;

    while (remLen >= 2)
    {
        v_U16_t eLen = 0;
        v_U8_t elementId;
        elementId = *genie++;
        eLen  = *genie++;
        remLen -= 2;
    
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: IE[0x%X], LEN[%d]\n", 
            __func__, elementId, eLen);
         
        switch ( elementId ) 
        {
            case DOT11F_EID_WPA: 
                if ((2+4) > eLen) /* should have at least OUI */
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR, 
                              "%s: Invalid WPA IE", __func__);
                    return -EINVAL;
                }
                else if (0 == memcmp(&genie[0], "\x00\x50\xf2\x04", 4)) 
                {
                    v_U16_t curAddIELen = pWextState->assocAddIE.length;
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPS IE(len %d)", 
                            __func__, eLen + 2);
                    
                    if( SIR_MAC_MAX_IE_LENGTH < (pWextState->assocAddIE.length + eLen) )
                    {
                       hddLog(VOS_TRACE_LEVEL_FATAL, "Cannot accomadate assocAddIE. "
                                                      "Need bigger buffer space\n");
                       VOS_ASSERT(0);
                       return -ENOMEM;
                    }
                    // WSC IE is saved to Additional IE ; it should be accumulated to handle WPS IE + P2P IE
                    memcpy( pWextState->assocAddIE.addIEdata + curAddIELen, genie - 2, eLen + 2);
                    pWextState->assocAddIE.length += eLen + 2;
                    
                    pWextState->roamProfile.bWPSAssociation = VOS_TRUE;
                    pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
                    pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;
                }
                else if (0 == memcmp(&genie[0], "\x00\x50\xf2", 3)) 
                {  
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPA IE (len %d)",__func__, eLen + 2);
                    memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
                    memcpy( pWextState->WPARSNIE, genie - 2, (eLen + 2) /*ie_len*/);
                    pWextState->roamProfile.pWPAReqIE = pWextState->WPARSNIE;
                    pWextState->roamProfile.nWPAReqIELength = eLen + 2;//ie_len;
                }
#ifdef WLAN_FEATURE_P2P
                else if ( (0 == memcmp(&genie[0], P2P_OUI_TYPE, 
                                                         P2P_OUI_TYPE_SIZE)) 
                        /*Consider P2P IE, only for P2P Client */
                         && (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
                {
                    v_U16_t curAddIELen = pWextState->assocAddIE.length;
                    hddLog (VOS_TRACE_LEVEL_INFO, "%s Set P2P IE(len %d)", 
                            __func__, eLen + 2);
                    
                    if( SIR_MAC_MAX_IE_LENGTH < (pWextState->assocAddIE.length + eLen) )
                    {
                       hddLog(VOS_TRACE_LEVEL_FATAL, "Cannot accomadate assocAddIE "
                                                      "Need bigger buffer space\n");
                       VOS_ASSERT(0);
                       return -ENOMEM;
                    }
                    // P2P IE is saved to Additional IE ; it should be accumulated to handle WPS IE + P2P IE
                    memcpy( pWextState->assocAddIE.addIEdata + curAddIELen, genie - 2, eLen + 2);
                    pWextState->assocAddIE.length += eLen + 2;
                    
                    pWextState->roamProfile.pAddIEAssoc = pWextState->assocAddIE.addIEdata;
                    pWextState->roamProfile.nAddIEAssocLength = pWextState->assocAddIE.length;
                }
#endif
                break;
            case DOT11F_EID_RSN:
                hddLog (VOS_TRACE_LEVEL_INFO, "%s Set RSN IE(len %d)",__func__, eLen + 2);
                memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
                memcpy( pWextState->WPARSNIE, genie - 2, (eLen + 2)/*ie_len*/);
                pWextState->roamProfile.pRSNReqIE = pWextState->WPARSNIE;
                pWextState->roamProfile.nRSNReqIELength = eLen + 2; //ie_len;
                break;

            default:
                hddLog (VOS_TRACE_LEVEL_ERROR, 
                        "%s Set UNKNOWN IE %X", __func__, elementId);
                return 0;
        }
        genie += eLen;
        remLen -= eLen;
    }
    EXIT();
    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_privacy
 * This function is used to initialize the security 
 * parameters during connect operation.
 */
int wlan_hdd_cfg80211_set_privacy( hdd_adapter_t *pAdapter, 
                                   struct cfg80211_connect_params *req
                                   )
{
    int status = 0;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    ENTER();

    /*set wpa version*/
    pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    if (req->crypto.wpa_versions) 
    {
        if ( (NL80211_WPA_VERSION_1 == req->crypto.wpa_versions)
            && ( (req->ie_len) 
           && (0 == memcmp( &req->ie[2], "\x00\x50\xf2",3) ) ) ) 
           // Make sure that it is including a WPA IE.
           /* Currently NL is putting WPA version 1 even for open, 
            * since p2p ie is also put in same buffer.
            * */
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA;
        }
        else if (NL80211_WPA_VERSION_2 == req->crypto.wpa_versions)
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA2;
        }
    }
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: set wpa version to %d", __func__, 
            pWextState->wpaVersion);

    /*set authentication type*/
    status = wlan_hdd_cfg80211_set_auth_type(pAdapter, req->auth_type);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: failed to set authentication type ", __func__);
        return status;
    }

    /*set key mgmt type*/
    if (req->crypto.n_akm_suites)
    {
        status = wlan_hdd_set_akm_suite(pAdapter, req->crypto.akm_suites[0]);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set akm suite", 
                    __func__);
            return status;
        }
    }

    /*set pairwise cipher type*/
    if (req->crypto.n_ciphers_pairwise)
    {
        status = wlan_hdd_cfg80211_set_cipher(pAdapter,
                                      req->crypto.ciphers_pairwise[0], true);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: failed to set unicast cipher type", __func__);
            return status;
        }
    }

    /*set group cipher type*/
    status = wlan_hdd_cfg80211_set_cipher(pAdapter, req->crypto.cipher_group,
                                                                       false);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set mcast cipher type", 
                __func__);
        return status;
    }

    /*parse WPA/RSN IE, and set the correspoing fileds in Roam profile*/
    if (req->ie_len)
    {
        status = wlan_hdd_cfg80211_set_ie(pAdapter, req->ie, req->ie_len);
        if ( 0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to parse the WPA/RSN IE", 
                    __func__);
            return status;
        }
    }

    /*incase of WEP set default key information*/
    if (req->key && req->key_len) 
    {
        if ( (WLAN_CIPHER_SUITE_WEP40 == req->crypto.ciphers_pairwise[0])
                || (WLAN_CIPHER_SUITE_WEP104 == req->crypto.ciphers_pairwise[0])
          )
        {
            if (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Dynamic WEP not supported", 
                        __func__);
                return -EOPNOTSUPP;
            }
            else
            {
                u8 key_len = req->key_len;
                u8 key_idx = req->key_idx;

                if ((eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES >= key_len) 
                        && (CSR_MAX_NUM_KEY >= key_idx)
                  )
                {
                    hddLog(VOS_TRACE_LEVEL_INFO, 
                     "%s: setting default wep key, key_idx = %hu key_len %hu", 
                            __func__, key_idx, key_len);
                    vos_mem_copy(
                       &pWextState->roamProfile.Keys.KeyMaterial[key_idx][0], 
                                  req->key, key_len);
                    pWextState->roamProfile.Keys.KeyLength[key_idx] = 
                                                               (u8)key_len;
                    pWextState->roamProfile.Keys.defaultIndex = (u8)key_idx;
                }
            }
        }
    }

    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_privacy
 * This function is used to initialize the security 
 * parameters during connect operation.
 */
static int wlan_hdd_cfg80211_connect( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      struct cfg80211_connect_params *req
                                      )
{
    int status = 0;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( ndev ); 
    
    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, 
             "%s: device_mode = %d\n",__func__,pAdapter->device_mode);

    /*initialise security parameters*/
    status = wlan_hdd_cfg80211_set_privacy(pAdapter, req); 

    if ( 0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set security params", 
                __func__);
        return status;
    }

    status = wlan_hdd_cfg80211_connect_start(pAdapter, req->ssid, 
                                                req->ssid_len, req->bssid);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: connect failed", __func__);
        return status;
    }

    EXIT();
    return status;
}


/*
 * FUNCTION: wlan_hdd_cfg80211_disconnect
 * This function is used to issue a disconnect request to SME
 */
static int wlan_hdd_cfg80211_disconnect( struct wiphy *wiphy, 
                                         struct net_device *dev,
                                         u16 reason
                                         )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    tCsrRoamProfile  *pRoamProfile = 
                    &(WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter))->roamProfile;
    int status = 0;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    
    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n", 
                                    __func__,pAdapter->device_mode);

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: Disconnect called with reason code %d", 
            __func__, reason);    

    if (NULL != pRoamProfile)
    {
        /*issue disconnect request to SME, if station is in connected state*/
        if (pHddStaCtx->conn_info.connState == eConnectionState_Associated)
        {
            eCsrRoamDisconnectReason reasonCode = 
                                       eCSR_DISCONNECT_REASON_UNSPECIFIED;
            switch(reason)
            {
                case WLAN_REASON_MIC_FAILURE:
                    reasonCode = eCSR_DISCONNECT_REASON_MIC_ERROR;
                    break;

                case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
                case WLAN_REASON_DISASSOC_AP_BUSY:
                case WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:
                    reasonCode = eCSR_DISCONNECT_REASON_DISASSOC;
                    break;

                case WLAN_REASON_PREV_AUTH_NOT_VALID:
                case WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:
                    reasonCode = eCSR_DISCONNECT_REASON_DEAUTH;
                    break;

                case WLAN_REASON_DEAUTH_LEAVING:
                default:
                    reasonCode = eCSR_DISCONNECT_REASON_UNSPECIFIED;
                    break;
            }
            pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;
            INIT_COMPLETION(pAdapter->disconnect_comp_var);

            /*issue disconnect*/
            status = sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                         pAdapter->sessionId, reasonCode);

            if ( 0 != status)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,
                        "%s csrRoamDisconnect failure, returned %d \n", 
                        __func__, (int)status );
                return -EINVAL;
            }

            wait_for_completion_interruptible_timeout(
                   &pAdapter->disconnect_comp_var,
                   msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));


            /*stop tx queues*/
            netif_tx_disable(dev);
            netif_carrier_off(dev);
        }
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid roam profile", __func__);
    }

    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_privacy_ibss
 * This function is used to initialize the security 
 * settings in IBSS mode.
 */
static int wlan_hdd_cfg80211_set_privacy_ibss(
                                         hdd_adapter_t *pAdapter, 
                                         struct cfg80211_ibss_params *params
                                         )
{
    int status = 0;
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);   
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    
    ENTER();

    pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    if (params->ie_len && ( NULL != params->ie) )
    {
        if (WLAN_EID_RSN == params->ie[0]) 
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA2;
            encryptionType = eCSR_ENCRYPT_TYPE_AES;
        }
        else
        {
            pWextState->wpaVersion = IW_AUTH_WPA_VERSION_WPA;
            encryptionType = eCSR_ENCRYPT_TYPE_TKIP;
        }
        status = wlan_hdd_cfg80211_set_ie(pAdapter, params->ie, params->ie_len);

        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to parse WPA/RSN IE", 
                    __func__);
            return status;
        }
    }

    pWextState->roamProfile.AuthType.authType[0] = 
                                pHddStaCtx->conn_info.authType = 
                                eCSR_AUTH_TYPE_OPEN_SYSTEM;

    if (params->privacy)
    {
        /* Security enabled IBSS, At this time there is no information available 
         * about the security paramters, so initialise the encryption type to 
         * eCSR_ENCRYPT_TYPE_WEP40_STATICKEY.
         * The correct security parameters will be updated later in 
         * wlan_hdd_cfg80211_add_key */
        /* Hal expects encryption type to be set inorder 
         *enable privacy bit in beacons */

        encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    }

    pHddStaCtx->conn_info.ucEncryptionType                   = encryptionType;
    pWextState->roamProfile.EncryptionType.numEntries        = 1;
    pWextState->roamProfile.EncryptionType.encryptionType[0] = encryptionType;

    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_join_ibss
 * This function is used to create/join an IBSS 
 */
static int wlan_hdd_cfg80211_join_ibss( struct wiphy *wiphy, 
                                        struct net_device *dev,
                                        struct cfg80211_ibss_params *params
                                       )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile          *pRoamProfile;
    int status;
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

    ENTER();
    
    hddLog(VOS_TRACE_LEVEL_INFO, 
                  "%s: device_mode = %d\n",__func__,pAdapter->device_mode);

    if (NULL == pWextState)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s ERROR: Data Storage Corruption\n", 
                __func__);
        return -EIO;
    }

    pRoamProfile = &pWextState->roamProfile;

    if ( eCSR_BSS_TYPE_START_IBSS != pRoamProfile->BSSType )
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, 
                "%s Interface type is not set to IBSS \n", __func__);
        return -EINVAL;
    }

    /* Set Channel */
    if (NULL != params->channel)
    {
        u8 channelNum;
        if (IEEE80211_BAND_5GHZ == params->channel->band)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: IBSS join is called with unsupported band %d", 
                    __func__, params->channel->band);
            return -EOPNOTSUPP;
        }

        /* Get channel number */
        channelNum = 
               ieee80211_frequency_to_channel(params->channel->center_freq);

        /*TODO: use macro*/
        if (14 >= channelNum)
        {
            v_U32_t numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
            v_U8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
            tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
            int indx;

            if (0 != ccmCfgGetStr(hHal, WNI_CFG_VALID_CHANNEL_LIST,
                        validChan, &numChans))
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: No valid channel list", 
                        __func__);
                return -EOPNOTSUPP;
            }

            for (indx = 0; indx < numChans; indx++)
            {
                if (channelNum == validChan[indx])
                {
                    break;
                }
            }
            if (indx >= numChans)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Not valid Channel %d", 
                        __func__, channelNum);
                return -EINVAL;
            }
            /* Set the Operational Channel */
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: set channel %d", __func__, 
                    channelNum);
            pRoamProfile->ChannelInfo.numOfChannels = 1;
            pHddStaCtx->conn_info.operationChannel = channelNum;
            pRoamProfile->ChannelInfo.ChannelList = 
                &pHddStaCtx->conn_info.operationChannel;
        }
        else
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Not valid Channel %hu", 
                    __func__, channelNum);
            return -EINVAL;
        }
    }

    /* Initialize security parameters */
    status = wlan_hdd_cfg80211_set_privacy_ibss(pAdapter, params); 
    if (status < 0)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set security parameters", 
                __func__);
        return status;
    }

    /* Issue connect start */
    status = wlan_hdd_cfg80211_connect_start(pAdapter, params->ssid, 
            params->ssid_len, params->bssid);

    if (0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: connect failed", __func__);
        return status;
    }

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_leave_ibss
 * This function is used to leave an IBSS 
 */
static int wlan_hdd_cfg80211_leave_ibss( struct wiphy *wiphy, 
                                         struct net_device *dev
                                         )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev ); 
    hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    tCsrRoamProfile *pRoamProfile;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d\n",__func__,pAdapter->device_mode);
    if (NULL == pWextState)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s ERROR: Data Storage Corruption\n", 
                __func__);
        return -EIO;
    }

    pRoamProfile = &pWextState->roamProfile;

    /* Issue disconnect only if interface type is set to IBSS */
    if (eCSR_BSS_TYPE_START_IBSS != pRoamProfile->BSSType)
    {
        hddLog (VOS_TRACE_LEVEL_ERROR, "%s: BSS Type is not set to IBSS", 
                __func__);
        return -EINVAL;
    }

    /* Issue Disconnect request */
    INIT_COMPLETION(pAdapter->disconnect_comp_var);
    sme_RoamDisconnect( WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId,
                                  eCSR_DISCONNECT_REASON_IBSS_LEAVE);

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_wiphy_params
 * This function is used to set the phy parameters
 * (RTS Threshold/FRAG Threshold/Retry Count etc ...)
 */
static int wlan_hdd_cfg80211_set_wiphy_params(struct wiphy *wiphy, 
        u32 changed)
{
    hdd_context_t *pHddCtx = wiphy_priv(wiphy);
    tHalHandle hHal = pHddCtx->hHal;

    ENTER();

    if (changed & WIPHY_PARAM_RTS_THRESHOLD)
    {
        u16 rts_threshold = (wiphy->rts_threshold == -1) ?
                               WNI_CFG_RTS_THRESHOLD_STAMAX :
                               wiphy->rts_threshold;

        if ((WNI_CFG_RTS_THRESHOLD_STAMIN > rts_threshold) ||
                (WNI_CFG_RTS_THRESHOLD_STAMAX < rts_threshold)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid RTS Threshold value %hu", 
                    __func__, rts_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_RTS_THRESHOLD,
                    rts_threshold, ccmCfgSetCallback,
                    eANI_BOOLEAN_TRUE)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: ccmCfgSetInt failed for rts_threshold value %hu", 
                    __func__, rts_threshold);
            return -EIO;
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set rts threshold %hu", __func__, 
                rts_threshold);
    }

    if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
    {
        u16 frag_threshold = (wiphy->frag_threshold == -1) ?
                                WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX :
                                wiphy->frag_threshold;

        if ((WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN > frag_threshold)||
                (WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX < frag_threshold) ) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid frag_threshold value %hu", __func__, 
                    frag_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD,
                    frag_threshold, ccmCfgSetCallback,
                    eANI_BOOLEAN_TRUE)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: ccmCfgSetInt failed for frag_threshold value %hu", 
                    __func__, frag_threshold);
            return -EIO;
        }

        hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set frag threshold %hu", __func__,
                frag_threshold);
    }

    if ((changed & WIPHY_PARAM_RETRY_SHORT)
            || (changed & WIPHY_PARAM_RETRY_LONG))
    {
        u8 retry_value = (changed & WIPHY_PARAM_RETRY_SHORT) ?
                         wiphy->retry_short :
                         wiphy->retry_long;

        if ((WNI_CFG_LONG_RETRY_LIMIT_STAMIN > retry_value) ||
                (WNI_CFG_LONG_RETRY_LIMIT_STAMAX < retry_value))
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid Retry count %hu", 
                    __func__, retry_value);
            return -EINVAL;
        }

        if (changed & WIPHY_PARAM_RETRY_SHORT)
        {
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_LONG_RETRY_LIMIT,
                        retry_value, ccmCfgSetCallback,
                        eANI_BOOLEAN_TRUE)) 
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: ccmCfgSetInt failed for long retry count %hu", 
                        __func__, retry_value);
                return -EIO;
            }
            hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set long retry count %hu", 
                    __func__, retry_value);
        }
        else if (changed & WIPHY_PARAM_RETRY_SHORT)
        {
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_SHORT_RETRY_LIMIT,
                        retry_value, ccmCfgSetCallback,
                        eANI_BOOLEAN_TRUE)) 
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, 
                        "%s: ccmCfgSetInt failed for short retry count %hu", 
                        __func__, retry_value);
                return -EIO;
            }
            hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set short retry count %hu", 
                    __func__, retry_value);
        }
    }

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_txpower
 * This function is used to set the txpower
 */
static int wlan_hdd_cfg80211_set_txpower(struct wiphy *wiphy,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
        enum tx_power_setting type, 
#else
        enum nl80211_tx_power_setting type, 
#endif
        int dbm)
{
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
    tHalHandle hHal = pHddCtx->hHal;

    ENTER();

    if (0 != ccmCfgSetInt(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL, 
                dbm, ccmCfgSetCallback, 
                eANI_BOOLEAN_TRUE)) 
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: ccmCfgSetInt failed for tx power %hu", __func__, dbm);
        return -EIO;
    }

    hddLog(VOS_TRACE_LEVEL_INFO_MED, "%s: set tx power level %d dbm", __func__,
            dbm);

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_get_txpower
 * This function is used to read the txpower
 */
static int wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy, int *dbm)
{
  
    hdd_adapter_t *pAdapter;
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);

    if (NULL == pHddCtx)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD context is Null",__func__);
        *dbm = 0;
        return -ENOENT;
    }

    pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
    if (NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Not in station context " ,__func__);
        return -ENOENT;
    }

    wlan_hdd_get_classAstats(pAdapter);
    *dbm = pAdapter->hdd_stats.ClassA_stat.max_pwr;

    return 0;
}

static int wlan_hdd_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
                                   u8* mac, struct station_info *sinfo)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
    int ssidlen = pHddStaCtx->conn_info.SSID.SSID.length;
    tANI_U8 rate_flags;

    if ((eConnectionState_Associated != pHddStaCtx->conn_info.connState) ||
            (0 == ssidlen))
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: Not associated or"
                    " Invalid ssidlen, %d", __func__, ssidlen);
        /*To keep GUI happy*/
        return 0;
    }

    wlan_hdd_get_rssi(pAdapter, &sinfo->signal);
    sinfo->filled |= STATION_INFO_SIGNAL;

    wlan_hdd_get_classAstats(pAdapter);
    rate_flags = pAdapter->hdd_stats.ClassA_stat.tx_rate_flags;

    if (rate_flags & eHAL_TX_RATE_LEGACY)
    {
        //provide to the UI in units of 100kbps
        sinfo->txrate.legacy = pAdapter->hdd_stats.ClassA_stat.tx_rate * 5;
    }
    else if (rate_flags & eHAL_TX_RATE_HT20)
    {
        sinfo->txrate.mcs = pAdapter->hdd_stats.ClassA_stat.mcs_index;
        sinfo->txrate.flags |= RATE_INFO_FLAGS_MCS;
        if (rate_flags & eHAL_TX_RATE_SGI)
        { 
            sinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;
        }
    }
    sinfo->filled |= STATION_INFO_TX_BITRATE;

    return 0;
}

static int wlan_hdd_cfg80211_set_power_mgmt(struct wiphy *wiphy,
                     struct net_device *dev, bool mode, v_SINT_t timeout)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    VOS_STATUS vos_status;

    if (NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Adapter is NULL\n", __func__);
        return -ENODEV;
    }

    /**The get power cmd from the supplicant gets updated by the nl only
     *on successful execution of the function call
     *we are oppositely mapped w.r.t mode in the driver
     **/
    vos_status =  wlan_hdd_enter_bmps(pAdapter, !mode);

    if (VOS_STATUS_E_FAILURE == vos_status)
    {
        return -EINVAL;
    }
    return 0;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
static int wlan_hdd_set_default_mgmt_key(struct wiphy *wiphy,
                         struct net_device *netdev,
                         u8 key_index)
{
    return 0;
}

static int wlan_hdd_set_txq_params(struct wiphy *wiphy,
                   struct ieee80211_txq_params *params)
{
    return 0;
}
#endif

static int wlan_hdd_cfg80211_del_station(struct wiphy *wiphy,
                                         struct net_device *dev, u8 *mac)
{
    // TODO: Implement this later.
    return 0;
}

static int wlan_hdd_cfg80211_add_station(struct wiphy *wiphy,
          struct net_device *dev, u8 *mac, struct station_parameters *params)
{
    // TODO: Implement this later.
    return 0;
}

/* cfg80211_ops */
static struct cfg80211_ops wlan_hdd_cfg80211_ops = 
{
    .add_virtual_intf = wlan_hdd_add_virtual_intf,
    .del_virtual_intf = wlan_hdd_del_virtual_intf,
    .change_virtual_intf = wlan_hdd_cfg80211_change_iface,
    .change_station = wlan_hdd_change_station,
    .add_beacon = wlan_hdd_cfg80211_add_beacon,
    .del_beacon = wlan_hdd_cfg80211_del_beacon,
    .set_beacon = wlan_hdd_cfg80211_set_beacon,
    .change_bss = wlan_hdd_cfg80211_change_bss,
    .add_key = wlan_hdd_cfg80211_add_key,
    .get_key = wlan_hdd_cfg80211_get_key,
    .del_key = wlan_hdd_cfg80211_del_key,
    .set_default_key = wlan_hdd_cfg80211_set_default_key,
    .set_channel = wlan_hdd_cfg80211_set_channel,
    .scan = wlan_hdd_cfg80211_scan,
    .connect = wlan_hdd_cfg80211_connect,
    .disconnect = wlan_hdd_cfg80211_disconnect,
    .join_ibss  = wlan_hdd_cfg80211_join_ibss,
    .leave_ibss = wlan_hdd_cfg80211_leave_ibss,
    .set_wiphy_params = wlan_hdd_cfg80211_set_wiphy_params,
    .set_tx_power = wlan_hdd_cfg80211_set_txpower,
    .get_tx_power = wlan_hdd_cfg80211_get_txpower,
#ifdef WLAN_FEATURE_P2P
    .remain_on_channel = wlan_hdd_cfg80211_remain_on_channel,
    .cancel_remain_on_channel =  wlan_hdd_cfg80211_cancel_remain_on_channel,
    .mgmt_tx =  wlan_hdd_action,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
     .mgmt_tx_cancel_wait = wlan_hdd_cfg80211_mgmt_tx_cancel_wait,
     .set_default_mgmt_key = wlan_hdd_set_default_mgmt_key,
     .set_txq_params = wlan_hdd_set_txq_params,
#endif
#endif
     .get_station = wlan_hdd_cfg80211_get_station,
     .set_power_mgmt = wlan_hdd_cfg80211_set_power_mgmt,
     .del_station  = wlan_hdd_cfg80211_del_station,
     .add_station  = wlan_hdd_cfg80211_add_station
};

#endif // CONFIG_CFG80211
