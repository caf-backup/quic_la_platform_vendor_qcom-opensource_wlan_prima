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

#define g_mode_rates_size (12)
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

#define HDD_G_MODE_RATETAB(rate, rate_id, flag)\
{\
    .bitrate = rate, \
    .hw_value = rate_id, \
    .flags = flag, \
}

static const u32 hdd_cipher_suites[] = 
{
    WLAN_CIPHER_SUITE_WEP40,
    WLAN_CIPHER_SUITE_WEP104,
    WLAN_CIPHER_SUITE_TKIP,
    WLAN_CIPHER_SUITE_CCMP
};

static inline int is_broadcast_ether_addr(const u8 *addr)
{
    return ((addr[0] == 0xff) && (addr[1] == 0xff) && (addr[2] == 0xff) &&   \
            (addr[3] == 0xff) && (addr[4] == 0xff) && (addr[5] == 0xff));
}

static struct ieee80211_channel hdd_2GHZ_channels[] =
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

static struct ieee80211_rate g_mode_rates[] =
{
    HDD_G_MODE_RATETAB(10, 0x1, 0),    
    HDD_G_MODE_RATETAB(20, 0x2, 0),    
    HDD_G_MODE_RATETAB(55, 0x4, 0),    
    HDD_G_MODE_RATETAB(110, 0x8, 0),    
    HDD_G_MODE_RATETAB(60, 0x10, 0),    
    HDD_G_MODE_RATETAB(90, 0x20, 0),    
    HDD_G_MODE_RATETAB(110, 0x40, 0),    
    HDD_G_MODE_RATETAB(180, 0x80, 0),    
    HDD_G_MODE_RATETAB(240, 0x100, 0),    
    HDD_G_MODE_RATETAB(360, 0x200, 0),    
    HDD_G_MODE_RATETAB(480, 0x400, 0),    
    HDD_G_MODE_RATETAB(540, 0x800, 0),
};   

static struct ieee80211_supported_band wlan_hdd_band_2GHZ = 
{
    .channels = hdd_2GHZ_channels,
    .n_channels = ARRAY_SIZE(hdd_2GHZ_channels),
    .bitrates = g_mode_rates,
    .n_bitrates = g_mode_rates_size,
};

static struct cfg80211_ops wlan_hdd_cfg80211_ops;

/*
 * FUNCTION: wlan_hdd_cfg80211_init
 * This function is called by hdd_wlan_sdio_probe() 
 * during initialization. 
 * This function is used to initialize and register wiphy structure.
 */
struct wireless_dev *wlan_hdd_cfg80211_init( struct device *dev, 
                                             int priv_size
                                             )
{
    struct wireless_dev *wdev = kzalloc(sizeof(struct wireless_dev), \
            GFP_KERNEL);

    ENTER();

    if (!wdev)
    {
        /* print error and return */
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wireless_dev alloc failed", 
                __func__);
        return NULL;
    }

    /* 
     *  Now create wiphy device 
     * Currently, not keeping any private data under wireless_dev
     */
    wdev->wiphy = wiphy_new(&wlan_hdd_cfg80211_ops, priv_size);

    if (!wdev->wiphy)
    {
        /* Print error and jump into err label and free the memory */
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wiphy init failed", __func__);
        goto wiphy_init_err;
    }

    /* Now bind sdio device with wiphy */
    set_wiphy_dev(wdev->wiphy, dev);

    wdev->wiphy->max_scan_ssids = MAX_SCAN_SSID; 

    /* Supports STATION & AD-HOC modes right now */
    wdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
        BIT(NL80211_IFTYPE_ADHOC);

    /*Initialise the band details*/
    wdev->wiphy->bands[IEEE80211_BAND_2GHZ] = &wlan_hdd_band_2GHZ;

    /*Initialise the supported cipher suite details*/
    wdev->wiphy->cipher_suites = hdd_cipher_suites;
    wdev->wiphy->n_cipher_suites = ARRAY_SIZE(hdd_cipher_suites);

    /* Register our wiphy dev with cfg80211 */
    if (0 > wiphy_register(wdev->wiphy))
    {
        /* print eror */
        printk("%s: wiphy register failed\n", __func__);
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: wiphy register failed", __func__);
        goto register_err;
    }

    EXIT();
    return wdev;

register_err:
    wiphy_free(wdev->wiphy);

wiphy_init_err:
    kfree(wdev);
    EXIT();
    return NULL;
}     

/*
 * FUNCTION: wlan_hdd_cfg80211_change_iface
 * This function is used to set the interface type (INFRASTRUCTURE/ADHOC)
 */
static int wlan_hdd_cfg80211_change_iface( struct wiphy *wiphy,
                                           struct net_device *ndev,
                                           enum nl80211_iftype type, 
                                           u32 *flags,
                                           struct vif_params *params
                                           )
{
    struct wireless_dev *wdev;
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile;
    eCsrRoamBssType LastBSSType;
    hdd_config_t *pConfig = pAdapter->cfg_ini;
    eMib_dot11DesiredBssType connectedBssType;

    ENTER();
 
    wdev = ndev->ieee80211_ptr;
    pRoamProfile = &pWextState->roamProfile;
    LastBSSType = pRoamProfile->BSSType;

    switch (type) 
    {
        case NL80211_IFTYPE_STATION:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: setting interface Type to INFRASTRUCTURE", __func__);
            pRoamProfile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
	        pRoamProfile->phyMode = 
                hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);

            break;
        case NL80211_IFTYPE_ADHOC:
            hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting interface Type to ADHOC",
                    __func__);
            pRoamProfile->BSSType = eCSR_BSS_TYPE_START_IBSS;
            pRoamProfile->phyMode = 
                hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Unsupported interface Type",
                    __func__);
            return -EOPNOTSUPP;
    }

    if ( LastBSSType != pRoamProfile->BSSType )
    {
        /*interface type changed update in wiphy structure*/
        wdev->iftype = type;

        /*the BSS mode changed, We need to issue disconnect 
          if connected or in IBSS disconnect state*/
        if ( hdd_connGetConnectedBssType( pAdapter, &connectedBssType ) ||
                ( eCSR_BSS_TYPE_START_IBSS == LastBSSType ) )
        {
            /*need to issue a disconnect to CSR.*/
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            sme_RoamDisconnect( pAdapter->hHal, pAdapter->sessionId, 
                                     eCSR_DISCONNECT_REASON_UNSPECIFIED );
        }
    }
    return 0;
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
        if ( (0 != pAdapter->conn_info.staId[ idx ]) 
                && memcmp((u8*)&pAdapter->conn_info.peerMacAddress[idx], \
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

static int wlan_hdd_cfg80211_add_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, const u8 *mac_addr,
                                      struct key_params *params
                                      )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState = pAdapter->pWextState; 
    tCsrRoamSetKey  setKey;
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int status = 0;
    v_U32_t roamId= 0xFF;
    VOS_STATUS vos_status;

    ENTER();

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
    pWextState->roamProfile.Keys.KeyLength[key_index] = (u8)params->key_len;
    vos_mem_copy(&pWextState->roamProfile.Keys.KeyMaterial[key_index][0], 
            params->key, params->key_len);

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

    pAdapter->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;

    if (!(  (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) 
                && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType)
         )
            &&
            (  (WLAN_CIPHER_SUITE_WEP40 == params->cipher)
               || (WLAN_CIPHER_SUITE_WEP104 == params->cipher)
            )
      )
    {
        /* in case of static WEP, macaddr/bssid is not coming from nl80211 interface,
         * copy bssid for pairwise key and group macaddr for group key initialization*/

        tCsrRoamProfile          *pRoamProfile = &pWextState->roamProfile;

        if (eCSR_BSS_TYPE_START_IBSS == pRoamProfile->BSSType)
        {
            /* If IBSS, update the encryption type as we are using default encryption type as
             * eCSR_ENCRYPT_TYPE_WEP40_STATICKEY during ibss join*/
            pWextState->roamProfile.negotiatedUCEncryptionType = 
                pAdapter->conn_info.ucEncryptionType = 
                ((WLAN_CIPHER_SUITE_WEP40 == params->cipher) ? \
                eCSR_ENCRYPT_TYPE_WEP40_STATICKEY : \
                eCSR_ENCRYPT_TYPE_WEP104_STATICKEY);

            hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                    "%s: IBSS - negotiated encryption type %d", __func__, 
                    pWextState->roamProfile.negotiatedUCEncryptionType);
        }
        else 
        {
            pWextState->roamProfile.negotiatedUCEncryptionType = 
                pAdapter->conn_info.ucEncryptionType;
            
            hddLog(VOS_TRACE_LEVEL_INFO_MED, 
                    "%s: BSS - negotiated encryption type %d", __func__, 
                    pWextState->roamProfile.negotiatedUCEncryptionType);
        }

        sme_SetCfgPrivacy((tpAniSirGlobal)pAdapter->hHal, \
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
                    vos_mem_copy(setKey.peerMac, \
                            &pAdapter->conn_info.peerMacAddress[staidx], \
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
                vos_mem_copy(setKey.peerMac, \
                        &pAdapter->conn_info.bssId[0], \
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
    status = sme_RoamSetKey( pAdapter->hHal, pAdapter->sessionId, &setKey, 
                             &roamId );

    if ( 0 != status )
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: sme_RoamSetKey failed, returned %d", __func__, status);
        pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
        return -EINVAL;
    }


    /* in case of IBSS as there was no information available about WEP keys during 
     * IBSS join, group key intialized with NULL key, so re-initialize group key 
     * with correct value*/
    if ( (eCSR_BSS_TYPE_START_IBSS == pWextState->roamProfile.BSSType) && 
            !(  (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) 
                && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType)
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
                "%s: set key for peerMac %2x:%2x:%2x:%2x:%2x:%2x, direction %d",
                __func__, setKey.peerMac[0], setKey.peerMac[1], 
                setKey.peerMac[2], setKey.peerMac[3], 
                setKey.peerMac[4], setKey.peerMac[5], 
                setKey.keyDirection);
        status = sme_RoamSetKey( pAdapter->hHal, pAdapter->sessionId, 
                                                    &setKey, &roamId );
        if ( 0 != status )
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: sme_RoamSetKey failed for group key (IBSS), returned %d", 
                    __func__, status);
            pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
            return -EINVAL;
        }
    }

    return 0;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_get_key
 * This function is used to get the key information
 */
static int wlan_hdd_cfg80211_get_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, const u8 *mac_addr, void *cookie,
                                      void (*callback)(void *cookie, struct key_params*)
                                      )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState= pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
    struct key_params params;

    ENTER();
    
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
static int wlan_hdd_cfg80211_del_key( struct wiphy *wiphy, 
                                      struct net_device *ndev,
                                      u8 key_index, 
                                      const u8 *mac_addr
                                    )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    u8 groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    tCsrRoamSetKey  setKey;
    v_U32_t roamId= 0xFF;
    int status = 0;
    
    ENTER();

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
    pAdapter->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;   

    hddLog(VOS_TRACE_LEVEL_INFO_MED, 
            "%s: delete key for peerMac %2x:%2x:%2x:%2x:%2x:%2x",
            __func__, setKey.peerMac[0], setKey.peerMac[1], 
            setKey.peerMac[2], setKey.peerMac[3], 
            setKey.peerMac[4], setKey.peerMac[5]);

    status = sme_RoamSetKey( pAdapter->hHal, pAdapter->sessionId, 
                                                &setKey, &roamId );

    if ( 0 != status )
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, 
                "%s: sme_RoamSetKey failure, returned %d", __func__, status);
        pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
        return -EINVAL;
    }
    return status;
}

/*
 * FUNCTION: wlan_hdd_cfg80211_set_default_key
 * This function is used to set the default tx key index
 */
static int wlan_hdd_cfg80211_set_default_key( struct wiphy *wiphy,
                                              struct net_device *ndev,
                                              u8 key_index
                                              )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    int status = 0;

    ENTER();
   
    if (CSR_MAX_NUM_KEY <= key_index)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid key index %d", __func__, 
                key_index);

        return -EINVAL;
    }

    if ((key_index != pWextState->roamProfile.Keys.defaultIndex) && 
            (eCSR_ENCRYPT_TYPE_TKIP != pWextState->roamProfile.EncryptionType.encryptionType[0]) &&
             (eCSR_ENCRYPT_TYPE_AES != pWextState->roamProfile.EncryptionType.encryptionType[0]))
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
                &pAdapter->conn_info.bssId[0],
                WNI_CFG_BSSID_LEN);

        setKey.encType = 
            pWextState->roamProfile.EncryptionType.encryptionType[0];
         
        /* issue set key request */
        status = sme_RoamSetKey( pAdapter->hHal, pAdapter->sessionId, 
                                                      &setKey, &roamId );

        if ( 0 != status )
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: sme_RoamSetKey failed, returned %d", __func__, 
                    status);
            return -EINVAL;
        }
    }
    return status;
}

/**
 * FUNCTION: wlan_hdd_cfg80211_set_channel
 * This function is used to set the channel number 
 */
static int wlan_hdd_cfg80211_set_channel( struct wiphy *wiphy,
                                          struct ieee80211_channel *chan,
                                          enum nl80211_channel_type channel_type
                                          )
{
    v_U32_t num_ch = 0;
    u8 valid_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN];    
    u32 indx = 0;
    u32 channel = 0;
    hdd_adapter_t   *pAdapter = ((hdd_adapter_t*) wiphy_priv(wiphy));
    tHalHandle hHal = pAdapter->hHal;
    hdd_wext_state_t *pWextState = pAdapter->pWextState; 
    tCsrRoamProfile * pRoamProfile = &pWextState->roamProfile;
    int freq = chan->center_freq; /* freq is in MHZ */ 
 
    ENTER();

    if (eConnectionState_IbssConnected == pAdapter->conn_info.connState)
    {
        /* Link is up then return cant set channel*/
        hddLog( VOS_TRACE_LEVEL_ERROR, 
                "%s: IBSS Associated, can't set the channel\n", __func__);
        return -EINVAL;
    }

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
    
    hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: set channel to [%d]", __func__, 
            channel);

    num_ch = pRoamProfile->ChannelInfo.numOfChannels = 1;
    pAdapter->conn_info.operationChannel = channel; 
    pRoamProfile->ChannelInfo.ChannelList = 
        &pAdapter->conn_info.operationChannel; 

    EXIT();
    return 0;
}



/*
 * FUNCTION: wlan_hdd_cfg80211_inform_bss
 * This function is used to inform the BSS details to nl80211 interface.
 */
static struct cfg80211_bss* wlan_hdd_cfg80211_inform_bss( hdd_adapter_t *pAdapter, 
                                                          tSirBssDescription *bss_desc
                                                         )
{
    struct net_device *dev = pAdapter->dev;
    struct wireless_dev *wdev = dev->ieee80211_ptr;
    struct wiphy *wiphy = wdev->wiphy;
    int chan_no = bss_desc->channelId;
    int ie_length = GET_IE_LEN_IN_BSS_DESC( bss_desc->length );
    const char *ie = 
        ((ie_length != 0) ? (const char *)&bss_desc->ieFields: NULL);
    unsigned int freq = ieee80211_channel_to_frequency(chan_no);
    struct ieee80211_channel *chan = __ieee80211_get_channel(wiphy, freq);

    ENTER();

    return (cfg80211_inform_bss(wiphy, chan, bss_desc->bssId, 
                le64_to_cpu(*(__le64 *)bss_desc->timeStamp), 
                bss_desc->capabilityInfo,
                bss_desc->beaconInterval, ie, ie_length,
                bss_desc->rssi, GFP_KERNEL ));
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
    unsigned int freq = ieee80211_channel_to_frequency(chan_no);
    struct ieee80211_channel *chan = __ieee80211_get_channel(wiphy, freq);
    struct ieee80211_mgmt *mgmt =
        kzalloc((sizeof (struct ieee80211_mgmt) + ie_length), GFP_KERNEL);
    struct cfg80211_bss *bss_status = NULL;
    size_t frame_len = sizeof (struct ieee80211_mgmt) + ie_length;

    ENTER();

    memcpy(mgmt->bssid, bss_desc->bssId, ETH_ALEN);
    memcpy(&mgmt->u.probe_resp.timestamp, bss_desc->timeStamp,
            sizeof (bss_desc->timeStamp));
    mgmt->u.probe_resp.beacon_int = bss_desc->beaconInterval;
    mgmt->u.probe_resp.capab_info = bss_desc->capabilityInfo;
    memcpy(mgmt->u.probe_resp.variable, ie, ie_length);

    mgmt->frame_control |=
        (u16)(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);

    bss_status = cfg80211_inform_bss_frame(wiphy, chan, mgmt,
            frame_len, 0, GFP_KERNEL);
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
    tHalHandle hHal = pAdapter->hHal;

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
    tHalHandle hHal = pAdapter->hHal;
    tCsrScanResultInfo *pScanResult;
    eHalStatus status = 0;
    tScanResultHandle pResult;
    struct cfg80211_bss *bss_status = NULL;
    ENTER();

    if (pAdapter->isLogpInProgress) {
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
           cfg80211_inform_bss() is not updating ie field of bss entry, if entry
           already exists in bss data base of cfg80211 for that particular BSS ID.
           Using cfg80211_inform_bss_frame to update the bss entry instead of
           cfg80211_inform_bss, But this call expects mgmt packet as input. As of
           now there is no possibility to get the mgmt(probe response) frame from PE,
           converting bss_desc to ieee80211_mgmt(probe response) and passing to
           cfg80211_inform_bss_frame.
         */
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
    struct wireless_dev *wdev = dev->ieee80211_ptr;    
    hdd_adapter_t *pAdapter = ((hdd_adapter_t*) wiphy_priv(wdev->wiphy));
    hdd_wext_state_t *pwextBuf = pAdapter->pWextState;
    struct cfg80211_scan_request *req = pAdapter->request;
    int ret = 0;
    VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
 
    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO,
            "%s called with halHandle = %p, pContext = %p,"
            "scanID = %d, returned status = %d\n", 
            __func__, halHandle, pContext, (int) scanId, (int) status);

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

    ret = wlan_hdd_cfg80211_update_bss(wdev->wiphy, pAdapter);

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
    pAdapter->request = NULL;

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
    hdd_adapter_t *pAdapter = ((hdd_adapter_t*) wiphy_priv(wiphy));
    hdd_wext_state_t *pwextBuf = pAdapter->pWextState;
    hdd_config_t *cfg_param = pAdapter->cfg_ini;
    tCsrScanRequest scanRequest;
    v_U32_t scanId = 0;
    int status = 0;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "scan request for ssid = %d\n", 
            (int)request->n_ssids);  

    if (TRUE == pwextBuf->mScanPending)
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: mScanPending is TRUE\n", __func__);
        return -EBUSY;                  
    }

    if (pAdapter->isLogpInProgress) {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:LOGP in Progress. Ignore!!!",__func__);
      return -EAGAIN;
    }

    vos_mem_zero( &scanRequest, sizeof(scanRequest));

    if (request)
    {
        if (0 < request->n_ssids)
        {
            /* TODO:
             * Populate: SSID for all ssids
	     * Right now we are supporting only one SSID.
	     */            

            scanRequest.SSIDs.numOfSSIDs = request->n_ssids;
            scanRequest.SSIDs.SSIDList =(tCsrSSIDInfo *)vos_mem_malloc(sizeof(tCsrSSIDInfo));
            scanRequest.SSIDs.SSIDList->SSID.length = request->ssids->ssid_len;
            vos_mem_copy(scanRequest.SSIDs.SSIDList->SSID.ssId,
                    request->ssids->ssid,request->ssids->ssid_len);
	}
        
	/*Set the scan type to default type, in this case it is ACTIVE*/
	scanRequest.scanType = pAdapter->pWextState->scan_mode;
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
    scanRequest.ChannelInfo.numOfChannels = 0;

    scanRequest.ChannelInfo.ChannelList = NULL;

    /* set requestType to full scan */
    scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;

    if(pAdapter->pWextState->roamProfile.nWSCReqIELength != 0)
    {
       scanRequest.uIEFieldLen = pAdapter->pWextState->roamProfile.nWSCReqIELength;
       scanRequest.pIEField = pAdapter->pWextState->roamProfile.pWSCReqIE;
    }

    pwextBuf->mScanPending = TRUE;
    pAdapter->request = request;
    status = sme_ScanRequest( pAdapter->hHal, pAdapter->sessionId, &scanRequest, 
                       &scanId, &hdd_cfg80211_scan_done_callback, dev ); 

    if( !HAL_STATUS_SUCCESS(status) )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: SME scan fail status %d !!!",__func__, status);
        pwextBuf->mScanPending = FALSE;
        return -EINVAL;
    }

    pwextBuf->scanId = scanId;

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

    ENTER();

    pWextState = pAdapter->pWextState;
    
    if (SIR_MAC_MAX_SSID_LENGTH < ssid_len)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: wrong SSID len", __func__);
        return -EINVAL;
    }

    pRoamProfile = &pWextState->roamProfile;

    if (pRoamProfile) 
    {
        if (hdd_connGetConnectedBssType( pAdapter, &connectedBssType ) ||
                ( eMib_dot11DesiredBssType_independent == connectedBssType))
        {
            /* Issue disconnect to CSR */
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            sme_RoamDisconnect( pAdapter->hHal, pAdapter->sessionId, 
                                       eCSR_DISCONNECT_REASON_UNSPECIFIED );
        }

        if (HDD_WMM_USER_MODE_NO_QOS == pAdapter->cfg_ini->WmmMode)
        {
            /*QoS not enabled in cfg file*/
            pRoamProfile->uapsd_mask = 0;
        }
        else
        {
            /*QoS enabled, update uapsd mask from cfg file*/
            pRoamProfile->uapsd_mask = pAdapter->cfg_ini->UapsdMask;
        }

        pRoamProfile->SSIDs.numOfSSIDs = 1;
        pRoamProfile->SSIDs.SSIDList->SSID.length = ssid_len;
        vos_mem_zero(pRoamProfile->SSIDs.SSIDList->SSID.ssId, \
                sizeof(pRoamProfile->SSIDs.SSIDList->SSID.ssId)); 
        vos_mem_copy((void *)(pRoamProfile->SSIDs.SSIDList->SSID.ssId), \
                ssid, ssid_len);

        if (bssid)
        {
            pRoamProfile->BSSIDs.numOfBSSIDs = 1;
            vos_mem_copy((void *)(pRoamProfile->BSSIDs.bssid), bssid,
                    WNI_CFG_BSSID_LEN);
        }

        if ((IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion) ||
                (IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion))
        { 
            /*set gen ie*/
            hdd_SetGENIEToCsr(pAdapter);
            /*set auth*/
            hdd_set_csr_auth_type(pAdapter);
        }

        status = sme_RoamConnect( pAdapter->hHal, pAdapter->sessionId,
                                         pRoamProfile, &roamId);

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
    hdd_wext_state_t *pWextState = pAdapter->pWextState;   

    ENTER();

    /*set authentication type*/
    switch (auth_type) 
    {
        case NL80211_AUTHTYPE_OPEN_SYSTEM:
        case NL80211_AUTHTYPE_AUTOMATIC:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to OPEN", __func__);
            pAdapter->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
            break;

        case NL80211_AUTHTYPE_SHARED_KEY:
            hddLog(VOS_TRACE_LEVEL_INFO, 
                    "%s: set authentication type to SHARED", __func__);
            pAdapter->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
            break;

        default:
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Unsupported authentication type %d", __func__, 
                    auth_type);
            pAdapter->conn_info.authType = eCSR_AUTH_TYPE_UNKNOWN;
            return -EINVAL;
    }

    pWextState->roamProfile.AuthType.authType[0] = pAdapter->conn_info.authType;
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
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
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
    hdd_wext_state_t *pWextState = pAdapter->pWextState;   

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
                    (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
                encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
            else
                encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
            break;

        case WLAN_CIPHER_SUITE_WEP104:
            if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                    (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
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
        pAdapter->conn_info.ucEncryptionType                     = encryptionType;
        pWextState->roamProfile.EncryptionType.numEntries        = 1;
        pWextState->roamProfile.EncryptionType.encryptionType[0] = encryptionType;
    }
    else
    {
        hddLog(VOS_TRACE_LEVEL_INFO, "%s: setting mcast cipher type to %d", 
                __func__, encryptionType);
        pAdapter->conn_info.mcEncryptionType                       = encryptionType;
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
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    u8 *genie = ie;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: iw_set_genie ioctl IE[0x%X], LEN[%d]\n", 
            __func__, genie[0], genie[1]);

    switch ( genie[0] ) 
    {
        case DOT11F_EID_WPA: 
            if ((2+4) > genie[1]) /* should have at least OUI */
            {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid WPA IE", __func__);
                /* should I clear WPA/WPSIE here ? or should I keep old value ? */
                /* for now, it just ignore this invalid ie, and keep old IE */
                return -EINVAL;
            }
            else if (0 == memcmp(&genie[2], "\x00\x50\xf2\x04", 4)) 
            {
                hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPS IE(len %d)", 
                        __func__, genie[1]+2);

                if(ie_len > sizeof(pWextState->WPSWSCIE.wscIEdata)) {
                   hddLog(LOGE, "%s Invalid WPS IE len %d\n", __FUNCTION__, ie_len);
                   return -EINVAL;
                }
                if(pWextState->roamProfile.bWPSAssociation == FALSE) {
                    memset( &pWextState->WPSWSCIE, 0, sizeof(pWextState->WPSWSCIE) );
                    memcpy( pWextState->WPSWSCIE.wscIEdata, ie, ie_len);
                    pWextState->WPSWSCIE.length = ie_len;

                    pWextState->roamProfile.pWSCReqIE = pWextState->WPSWSCIE.wscIEdata;
                    pWextState->roamProfile.nWSCReqIELength = ie_len;
                    pWextState->roamProfile.bWPSAssociation = VOS_TRUE;
                }
                else {
                    hddLog (VOS_TRACE_LEVEL_INFO, "WPSAssociation Pending: Previous len=%d, Combining WSC IE (new ie_len=%d)\n", 
                            __func__, pWextState->WPSWSCIE.length, ie_len);
                    sme_combineWSCIE(pAdapter->hHal, &pWextState->WPSWSCIE, ie, ie_len);
                }
            }
            else 
            {  
                hddLog (VOS_TRACE_LEVEL_INFO, "%s Set WPA IE",__func__);            
                memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
                memcpy( pWextState->WPARSNIE, ie, ie_len);
                pWextState->roamProfile.pWPAReqIE = pWextState->WPARSNIE;
                pWextState->roamProfile.nWPAReqIELength = ie_len;
            }     
            break;
        case DOT11F_EID_RSN:
            hddLog (VOS_TRACE_LEVEL_INFO, "%s Set RSN IE",__func__);            
            memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
            memcpy( pWextState->WPARSNIE, ie, ie_len);
            pWextState->roamProfile.pRSNReqIE = pWextState->WPARSNIE;
            pWextState->roamProfile.nRSNReqIELength = ie_len;
            break;

        default:
            hddLog (VOS_TRACE_LEVEL_ERROR, "%s Set UNKNOWN IE %X", __func__, 
                    genie[0]);
            return 0;
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
    hdd_wext_state_t *pWextState = pAdapter->pWextState;   
    ENTER();

    /*set wpa version*/
    pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    if (req->crypto.wpa_versions) 
    {
        if (NL80211_WPA_VERSION_1 == req->crypto.wpa_versions)
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
        status = wlan_hdd_cfg80211_set_cipher(pAdapter, req->crypto.ciphers_pairwise[0], true);
        if (0 > status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: failed to set unicast cipher type", __func__);
            return status;
        }
    }

    /*set group cipher type*/
    status = wlan_hdd_cfg80211_set_cipher(pAdapter, req->crypto.cipher_group, false);

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
                    vos_mem_copy(&pWextState->roamProfile.Keys.KeyMaterial[key_idx][0], \
                            req->key, key_len);
                    pWextState->roamProfile.Keys.KeyLength[key_idx] = (u8)key_len;
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
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    
    ENTER();

    /*initialise security parameters*/
    status = wlan_hdd_cfg80211_set_privacy(pAdapter, req); 

    if ( 0 > status)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR, "%s: failed to set security params", 
                __func__);
        return status;
    }

    status = wlan_hdd_cfg80211_connect_start(pAdapter, req->ssid, req->ssid_len, 
            req->bssid);

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
    hdd_adapter_t *pAdapter = wiphy_priv(wiphy);
    tCsrRoamProfile  *pRoamProfile =  &pAdapter->pWextState->roamProfile;
    int status = 0;
    
    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: Disconnect called with reason code %d", 
            __func__, reason);    

    if (NULL != pRoamProfile)
    {
        /*issue disconnect request to SME, if station is in connected state*/
        if (pAdapter->conn_info.connState == eConnectionState_Associated)
        {
            eCsrRoamDisconnectReason reasonCode =  eCSR_DISCONNECT_REASON_UNSPECIFIED;
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
            pAdapter->conn_info.connState = eConnectionState_NotConnected;
            INIT_COMPLETION(pAdapter->disconnect_comp_var);

            /*issue disconnect*/
            status = sme_RoamDisconnect( pAdapter->hHal, pAdapter->sessionId, 
                                                       reasonCode);

            if ( 0 != status)
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,
                        "%s csrRoamDisconnect failure, returned %d \n", 
                        __func__, (int)status );
                return -EINVAL;
            }

            /*stop tx queues*/
            netif_tx_stop_all_queues(dev);
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
static int wlan_hdd_cfg80211_set_privacy_ibss( hdd_adapter_t *pAdapter, 
                                               struct cfg80211_ibss_params *params
                                               )
{
    int status = 0;
    hdd_wext_state_t *pWextState = pAdapter->pWextState;   
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    
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
                                pAdapter->conn_info.authType = 
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

    pAdapter->conn_info.ucEncryptionType                     = encryptionType;
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
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile          *pRoamProfile;
    int status;

    ENTER();

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
        channelNum = ieee80211_frequency_to_channel(params->channel->center_freq);

        /*TODO: use macro*/
        if (14 >= channelNum)
        {
            v_U32_t numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
            v_U8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
            tHalHandle hHal = pAdapter->hHal;
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
            pAdapter->conn_info.operationChannel = channelNum;
            pRoamProfile->ChannelInfo.ChannelList = 
                &pAdapter->conn_info.operationChannel;
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
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile;
    
    ENTER();

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
    sme_RoamDisconnect( pAdapter->hHal, pAdapter->sessionId, \
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
    hdd_adapter_t *pAdapter = wiphy_priv(wiphy);
    tHalHandle hHal = pAdapter->hHal;

    ENTER();

    if (changed & WIPHY_PARAM_RTS_THRESHOLD)
    {
        u16 rts_threshold = (wiphy->rts_threshold == -1) ? \
                               WNI_CFG_RTS_THRESHOLD_STAMAX : \
                               wiphy->rts_threshold;

        if ((WNI_CFG_RTS_THRESHOLD_STAMIN > rts_threshold) || \
                (WNI_CFG_RTS_THRESHOLD_STAMAX < rts_threshold)) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid RTS Threshold value %hu", 
                    __func__, rts_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_RTS_THRESHOLD, \
                    rts_threshold, ccmCfgSetCallback, \
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
        u16 frag_threshold = (wiphy->frag_threshold == -1) ? \
                                WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX : \
                                wiphy->frag_threshold;

        if ((WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN > frag_threshold)|| \
                (WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX < frag_threshold) ) 
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: Invalid frag_threshold value %hu", __func__, 
                    frag_threshold);
            return -EINVAL;
        }

        if (0 != ccmCfgSetInt(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD, \
                    frag_threshold, ccmCfgSetCallback, \
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
        u8 retry_value = (changed & WIPHY_PARAM_RETRY_SHORT) ? \
                         wiphy->retry_short : \
                         wiphy->retry_long;

        if ((WNI_CFG_LONG_RETRY_LIMIT_STAMIN > retry_value) || \
                (WNI_CFG_LONG_RETRY_LIMIT_STAMAX < retry_value))
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid Retry count %hu", 
                    __func__, retry_value);
            return -EINVAL;
        }

        if (changed & WIPHY_PARAM_RETRY_SHORT)
        {
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_LONG_RETRY_LIMIT, \
                        retry_value, ccmCfgSetCallback, \
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
            if (0 != ccmCfgSetInt(hHal, WNI_CFG_SHORT_RETRY_LIMIT, \
                        retry_value, ccmCfgSetCallback, \
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
        enum tx_power_setting type, int dbm)
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);
    tHalHandle hHal = pAdapter->hHal;

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

extern void hdd_StatisticsCB( void *pStats, void *pContext );

/*
 * FUNCTION: wlan_hdd_cfg80211_get_txpower
 * This function is used to read the txpower
 */
static int wlan_hdd_cfg80211_get_txpower(struct wiphy *wiphy, int *dbm)
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) wiphy_priv(wiphy);

    ENTER();

    if (eConnectionState_Associated != pAdapter->conn_info.connState) 
    {
        *dbm = 0;
    }
    else 
    {
        int status;
        hdd_wext_state_t *pWextState = pAdapter->pWextState;
        status = sme_GetStatistics( pAdapter->hHal, eCSR_HDD, 
                SME_SUMMARY_STATS      | 
                SME_GLOBAL_CLASSA_STATS | 
                SME_GLOBAL_CLASSB_STATS | 
                SME_GLOBAL_CLASSC_STATS | 
                SME_GLOBAL_CLASSD_STATS | 
                SME_PER_STA_STATS, 
                hdd_StatisticsCB, 0, FALSE, 
                pAdapter->conn_info.staId[0], 
                pAdapter );

        if (0 != status)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR, 
                    "%s: sme_GetStatistics failed, returned %d", __func__, 
                    status);
            return -EIO;
        }

        status = vos_wait_single_event(&pWextState->vosevent, 1000);

        if (0 != status)
        { 
            return -EIO;
        }

        *dbm = pAdapter->hdd_stats.ClassA_stat.max_pwr;
    }

    return 0;
}


/* cfg80211_ops */
static struct cfg80211_ops wlan_hdd_cfg80211_ops = 
{
    .change_virtual_intf = wlan_hdd_cfg80211_change_iface,
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
};

