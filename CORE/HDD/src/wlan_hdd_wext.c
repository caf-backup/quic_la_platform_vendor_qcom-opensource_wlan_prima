/** ------------------------------------------------------------------------ *
    ------------------------------------------------------------------------ *

  
    \file wlan_hdd_wext.c
  
    \brief Airgo Linux Wireless Extensions Common Control Plane Types and
    interfaces.
  
    $Id: wlan_hdd_wext.c,v 1.34 2007/04/14 01:49:23 jimz Exp jimz $ 
  
    Copyright (C) 2007 Airgo Networks, Incorporated
    
    This file defines all of the types that are utilized by the CCP module
    of the "Portable" HDD.   This file also includes the underlying Linux 
    Wireless Extensions Data types referred to by CCP. 
  
	======================================================================== */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <wlan_btc_svc.h>
#include <wlan_nlink_common.h>
#include <net/arp.h>
#include "ccmApi.h"
#include "sirParams.h"
#include "csrApi.h"
#include <aniGlobal.h>
#include "dot11f.h"
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_wmm.h>
#include <linux/earlysuspend.h>
#include "wlan_hdd_power.h"
#include "qwlan_version.h"
#include <vos_power.h>

#ifdef CONFIG_CFG80211
#include <linux/wireless.h>
#include <net/cfg80211.h>
#endif

#define WE_MAX_STR_LEN 1024

extern void hdd_suspend_wlan(struct early_suspend *wlan_suspend);
extern void hdd_resume_wlan(struct early_suspend *wlan_suspend);
extern VOS_STATUS hdd_enter_standby(hdd_adapter_t* pAdapter) ;


/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_NONE    (SIOCIWFIRSTPRIV + 0)
#define WE_SET_11D_STATE     1
#define WE_WOWL              2
#define WE_SET_POWER         3

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_INT    (SIOCIWFIRSTPRIV + 1)
#define WE_GET_11D_STATE     1
#define WE_IBSS_STATUS       2
#define WE_PMC_STATE         3
#define WE_GET_WLAN_DBG      4
#define WE_MODULE_DOWN_IND   5

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_INT     (SIOCIWFIRSTPRIV + 2)

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_CHAR_GET_NONE   (SIOCIWFIRSTPRIV + 3)
#define WE_WOWL_ADD_PTRN     1
#define WE_WOWL_DEL_PTRN     2

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_THREE_INT_GET_NONE   (SIOCIWFIRSTPRIV + 4)
#define WE_SET_WLAN_DBG      1

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_GET_CHAR_SET_NONE   (SIOCIWFIRSTPRIV + 5)
#define WE_WLAN_VERSION      1
#define WE_GET_STATS         2
#define WE_GET_CFG           3

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_NONE   (SIOCIWFIRSTPRIV + 6)
#define WE_CLEAR_STATS       1

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_VAR_INT_GET_NONE   (SIOCIWFIRSTPRIV + 7)
#define WE_LOG_DUMP_CMD      1
#define MAX_VAR_ARGS         5			

/* Private ioctls (with no sub-ioctls) */
/* note that they must be odd so that they have "get" semantics */
#define WLAN_PRIV_ADD_TSPEC (SIOCIWFIRSTPRIV +  9)
#define WLAN_PRIV_DEL_TSPEC (SIOCIWFIRSTPRIV + 11)
#define WLAN_PRIV_GET_TSPEC (SIOCIWFIRSTPRIV + 13)

#ifdef FEATURE_WLAN_WAPI
/* Private ioctls EVEN NO: SET, ODD NO:GET */
#define WLAN_PRIV_SET_WAPI_MODE         (SIOCIWFIRSTPRIV + 8)
#define WLAN_PRIV_GET_WAPI_MODE         (SIOCIWFIRSTPRIV + 16)
#define WLAN_PRIV_SET_WAPI_ASSOC_INFO   (SIOCIWFIRSTPRIV + 10)
#define WLAN_PRIV_SET_WAPI_KEY          (SIOCIWFIRSTPRIV + 12)
#define WLAN_PRIV_SET_WAPI_BKID         (SIOCIWFIRSTPRIV + 14)
#define WLAN_PRIV_GET_WAPI_BKID         (SIOCIWFIRSTPRIV + 15)
#define WAPI_PSK_AKM_SUITE  0x02721400
#define WAPI_CERT_AKM_SUITE 0x01721400
#endif

/* To Validate Channel against the Frequency and Vice-Versa */
static const hdd_freq_chan_map_t freq_chan_map[] = { {2412, 1}, {2417, 2}, 
        {2422, 3}, {2427, 4}, {2432, 5}, {2437, 6}, {2442, 7}, {2447, 8}, 
        {2452, 9}, {2457, 10}, {2462, 11}, {2467 ,12}, {2472, 13}, 
        {2484, 14}, {4920, 240}, {4940, 244}, {4960, 248}, {4980, 252}, 
        {5040, 208}, {5060, 212}, {5080, 216}, {5180, 36}, {5200, 40}, {5220, 44}, 
        {5240, 48}, {5260, 52}, {5280, 56}, {5300, 60}, {5320, 64}, {5500, 100}, 
        {5520, 104}, {5540, 108}, {5560, 112}, {5580, 116}, {5600, 120}, 
        {5620, 124}, {5640, 128}, {5660, 132}, {5680, 136}, {5700, 140}, 
        {5745, 149}, {5765, 153}, {5785, 157}, {5805, 161}, {5825, 165} };

#define FREQ_CHAN_MAP_TABLE_SIZE sizeof(freq_chan_map)/sizeof(freq_chan_map[0])

static v_BOOL_t 
hdd_IsAuthTypeRSN( tHalHandle halHandle, eCsrAuthType authType)
{
    v_BOOL_t rsnType = VOS_FALSE; 
    // is the authType supported?
    switch (authType)
    {
        case eCSR_AUTH_TYPE_NONE:    //never used
            rsnType = eANI_BOOLEAN_FALSE; 
            break;
        // MAC layer authentication types
        case eCSR_AUTH_TYPE_OPEN_SYSTEM:
            rsnType = eANI_BOOLEAN_FALSE; 
            break;
        case eCSR_AUTH_TYPE_SHARED_KEY:
            rsnType = eANI_BOOLEAN_FALSE; 
            break;
        case eCSR_AUTH_TYPE_AUTOSWITCH:
            rsnType = eANI_BOOLEAN_FALSE; 
            break;
    
        // Upper layer authentication types
        case eCSR_AUTH_TYPE_WPA:
            rsnType = eANI_BOOLEAN_TRUE; 
            break;
        case eCSR_AUTH_TYPE_WPA_PSK:
            rsnType = eANI_BOOLEAN_TRUE; 
            break;
        case eCSR_AUTH_TYPE_WPA_NONE:
            rsnType = eANI_BOOLEAN_TRUE; 
            break;
        case eCSR_AUTH_TYPE_RSN:
            rsnType = eANI_BOOLEAN_TRUE; 
            break;
        case eCSR_AUTH_TYPE_RSN_PSK:
            rsnType = eANI_BOOLEAN_TRUE; 
            break;
        //case eCSR_AUTH_TYPE_FAILED:
        case eCSR_AUTH_TYPE_UNKNOWN:
            rsnType = eANI_BOOLEAN_FALSE;
            break;
        default:
            hddLog(LOGE, FL("%s called with unknown authType - default to Open, None\n"),
                                                                                      __FUNCTION__); 
            rsnType = eANI_BOOLEAN_FALSE;
            break;
    }
    hddLog(LOGE, FL("%s called with authType: %d, returned: %d\n"),
                                             __FUNCTION__, authType, rsnType); 
    return rsnType;
}

void hdd_StatisticsCB( void *pStats, void *pContext )
{
   hdd_adapter_t             *pAdapter      = (hdd_adapter_t *)pContext;
   hdd_stats_t               *pStatsCache   = NULL;   
   hdd_wext_state_t *pWextState;   
   VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
   
   tCsrSummaryStatsInfo      *pSummaryStats = NULL;
   tCsrGlobalClassAStatsInfo *pClassAStats  = NULL;
   tCsrGlobalClassBStatsInfo *pClassBStats  = NULL;
   tCsrGlobalClassCStatsInfo *pClassCStats  = NULL;
   tCsrGlobalClassDStatsInfo *pClassDStats  = NULL;
   tCsrPerStaStatsInfo       *pPerStaStats  = NULL;

   if (pAdapter!= NULL)
     pStatsCache = &pAdapter->hdd_stats;
      
   pSummaryStats = (tCsrSummaryStatsInfo *)pStats;
   pClassAStats  = (tCsrGlobalClassAStatsInfo *)( pSummaryStats + 1 );
   pClassBStats  = (tCsrGlobalClassBStatsInfo *)( pClassAStats + 1 );
   pClassCStats  = (tCsrGlobalClassCStatsInfo *)( pClassBStats + 1 );
   pClassDStats  = (tCsrGlobalClassDStatsInfo *)( pClassCStats + 1 );
   pPerStaStats  = (tCsrPerStaStatsInfo *)( pClassDStats + 1 );
      
   if (pStatsCache!=NULL) 
   {
      // and copy the stats into the cache we keep in the adapter instance structure   
      vos_mem_copy( &pStatsCache->summary_stat, pSummaryStats, sizeof( pStatsCache->summary_stat ) );
      vos_mem_copy( &pStatsCache->ClassA_stat, pClassAStats, sizeof( pStatsCache->ClassA_stat ) );
      vos_mem_copy( &pStatsCache->ClassB_stat, pClassBStats, sizeof( pStatsCache->ClassB_stat ) );
      vos_mem_copy( &pStatsCache->ClassC_stat, pClassCStats, sizeof( pStatsCache->ClassC_stat ) );
      vos_mem_copy( &pStatsCache->ClassD_stat, pClassDStats, sizeof( pStatsCache->ClassD_stat ) );
      vos_mem_copy( &pStatsCache->perStaStats, pPerStaStats, sizeof( pStatsCache->perStaStats ) );
   }
   pWextState = pAdapter->pWextState;
   
   vos_status = vos_event_set(&pWextState->vosevent);
   
   if (!VOS_IS_STATUS_SUCCESS(vos_status))
   {   
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos_event_set failed!!\n"));
      return;
   }
   
}

void ccmCfgSetCallback(tHalHandle halHandle, tANI_S32 result)
{
   v_CONTEXT_t pVosContext;
   hdd_adapter_t *pAdapter;
   hdd_wext_state_t *pWextState;
   v_U32_t roamId;

   ENTER();

   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS,NULL);

   pAdapter = (hdd_adapter_t*) vos_get_context(VOS_MODULE_ID_HDD,pVosContext);

   pWextState = pAdapter->pWextState;
   
   if (WNI_CFG_NEED_RESTART == result || WNI_CFG_NEED_RELOAD == result)
   {
      VOS_STATUS vosStatus;
      pAdapter->conn_info.connState = eConnectionState_NotConnected;
      init_completion(&pAdapter->disconnect_comp_var);
      vosStatus = sme_RoamDisconnect(halHandle, eCSR_DISCONNECT_REASON_UNSPECIFIED);

      if(VOS_STATUS_SUCCESS == vosStatus)
          wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                     msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));

      sme_RoamConnect(halHandle,
                    &(pWextState->roamProfile),
                     NULL,
                     &roamId);
   }
   
   EXIT();
   
}

static int iw_set_commit(struct net_device *dev, struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
    hddLog( LOG1, "In %s\n", __FUNCTION__);
    /* Do nothing for now */
    return 0;
}

static int iw_get_name(struct net_device *dev,
                       struct iw_request_info *info,
                       char *wrqu, char *extra)
{
    
    ENTER();
    strcpy(wrqu, "Qcom:802.11n");
    EXIT();
    return 0;
}

static int iw_set_mode(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
    hdd_wext_state_t         *pWextState;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tCsrRoamProfile          *pRoamProfile;
    eCsrRoamBssType          LastBSSType;
    eMib_dot11DesiredBssType connectedBssType;
    hdd_config_t *pConfig  = pAdapter->cfg_ini;
       
    ENTER();

    pWextState = pAdapter->pWextState; 
    if (pWextState == NULL)
    {
        hddLog (LOGE, "%s ERROR: Data Storage Corruption\n", __FUNCTION__);
        return -EINVAL;
    }
   
    pRoamProfile = &pWextState->roamProfile;
    LastBSSType = pRoamProfile->BSSType;

    hddLog( LOGE,"%s Old Bss type = %d\n", __FUNCTION__, LastBSSType); 

    switch (wrqu->mode)
    {
    case IW_MODE_ADHOC:
        hddLog( LOG1,"%s Setting AP Mode as IW_MODE_ADHOC\n", __FUNCTION__); 
        pRoamProfile->BSSType = eCSR_BSS_TYPE_START_IBSS;
        // Set the phymode correctly for IBSS.
        pWextState->roamProfile.phyMode = hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
        break;
    case IW_MODE_INFRA:
        hddLog( LOG1, "%s Setting AP Mode as IW_MODE_INFRA\n", __FUNCTION__);
        pRoamProfile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
        break;
    case IW_MODE_AUTO:
        hddLog(LOG1,"%s Setting AP Mode as IW_MODE_AUTO\n", __FUNCTION__); 
        pRoamProfile->BSSType = eCSR_BSS_TYPE_ANY;
        break;
    default:
        hddLog(LOG1,"%s Unknown AP Mode value\n", __FUNCTION__); 
        return -EOPNOTSUPP;
    }

    if ( LastBSSType != pRoamProfile->BSSType )
    {
        //the BSS mode changed
        // We need to issue disconnect if connected or in IBSS disconnect state
        if ( hdd_connGetConnectedBssType( pAdapter, &connectedBssType ) ||
             ( eCSR_BSS_TYPE_START_IBSS == LastBSSType ) )
        {
            VOS_STATUS vosStatus;
            // need to issue a disconnect to CSR.
            init_completion(&pAdapter->disconnect_comp_var);
            vosStatus = sme_RoamDisconnect( pAdapter->hHal, eCSR_DISCONNECT_REASON_UNSPECIFIED );
            if(VOS_STATUS_SUCCESS == vosStatus)
                 wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                     msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
        }
    }



    EXIT();
    return 0;
}


static int iw_get_mode(struct net_device *dev,
                             struct iw_request_info *info,
                             v_U32_t *uwrq, char *extra)
{

    hdd_wext_state_t *pWextState;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

    hddLog (LOG1, "In %s",__FUNCTION__);
      
    pWextState = pAdapter->pWextState;
    if (pWextState == NULL)
    {
        hddLog (LOGE, "%s ERROR: Data Storage Corruption\n", __FUNCTION__);
        return -EINVAL;
    }
 
    switch (pWextState->roamProfile.BSSType)
    {
    case eCSR_BSS_TYPE_INFRASTRUCTURE:
        hddLog(LOG1, "%s returns IW_MODE_INFRA\n", __FUNCTION__);
        *uwrq = IW_MODE_INFRA ;
        break;
    case eCSR_BSS_TYPE_IBSS:
    case eCSR_BSS_TYPE_START_IBSS:
        hddLog( LOG1,"%s returns IW_MODE_ADHOC\n", __FUNCTION__); 
        *uwrq= IW_MODE_ADHOC;
        break;
    case eCSR_BSS_TYPE_ANY:
        hddLog( LOG1,"%s returns IW_MODE_AUTO\n", __FUNCTION__); 
        *uwrq= IW_MODE_AUTO;
        break;
    default:
        hddLog( LOG1,"%s returns APMODE_UNKNOWN\n", __FUNCTION__); 
        break;
    }
    return 0;
}

static int iw_set_freq(struct net_device *dev, struct iw_request_info *info,
             union iwreq_data *wrqu, char *extra)
{
    v_U32_t numChans = 0;
    v_U8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];    
    v_U32_t indx = 0;
    v_U32_t status = 0;

    hdd_wext_state_t *pWextState; 
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;
    tCsrRoamProfile * pRoamProfile;
    ENTER();
   
    pWextState = pAdapter->pWextState;
   
    pRoamProfile = &pWextState->roamProfile;

    hddLog(LOG1,"setCHANNEL ioctl\n");
    
    /* Link is up then return cant set channel*/
    if(eConnectionState_IbssConnected == pAdapter->conn_info.connState || 
       eConnectionState_Associated == pAdapter->conn_info.connState)
    {
        hddLog( LOGE, "IBSS Associated\n");
        return -EOPNOTSUPP;
    }

    /* Settings by Frequency as input */
    if((wrqu->freq.e == 1) && (wrqu->freq.m >= (tANI_U32)2.412e8) &&
                            (wrqu->freq.m <= (tANI_U32)5.825e8))
    {
        tANI_U32 freq = wrqu->freq.m / 100000;
        
        while ((indx <  FREQ_CHAN_MAP_TABLE_SIZE) && (freq != freq_chan_map[indx].freq))
            indx++; 
        if (indx >= FREQ_CHAN_MAP_TABLE_SIZE)
        {
            return -EINVAL;
        }
        wrqu->freq.e = 0;
        wrqu->freq.m = freq_chan_map[indx].chan;
       
    }
   
    if (wrqu->freq.e == 0) 
    {
        if((wrqu->freq.m < WNI_CFG_CURRENT_CHANNEL_STAMIN) || 
                        (wrqu->freq.m > WNI_CFG_CURRENT_CHANNEL_STAMAX)) 
        {
            hddLog(LOG1,"%s: Channel [%d] is outside valid range from %d to %d\n",
                __FUNCTION__, wrqu->freq.m, WNI_CFG_CURRENT_CHANNEL_STAMIN,
                    WNI_CFG_CURRENT_CHANNEL_STAMAX);
             return -EINVAL;
        }

        numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
      
        if (ccmCfgGetStr(hHal, WNI_CFG_VALID_CHANNEL_LIST,
                validChan, &numChans) != eHAL_STATUS_SUCCESS){
            return -EIO;
        }

        for (indx = 0; indx < numChans; indx++) {
            if (wrqu->freq.m == validChan[indx]){
                break;
            }
        }
    }
    else{
    
        return -EINVAL;
    }

    if(indx >= numChans)
    {
        return -EINVAL;
    }
   
    /* Set the Operational Channel */
    numChans = pRoamProfile->ChannelInfo.numOfChannels = 1;
    pAdapter->conn_info.operationChannel = wrqu->freq.m; 
    pRoamProfile->ChannelInfo.ChannelList = &pAdapter->conn_info.operationChannel; 
    
    hddLog(LOG1,"pRoamProfile->operationChannel  = %d\n", wrqu->freq.m);

    EXIT();
   
    return status;
}

static int iw_get_freq(struct net_device *dev, struct iw_request_info *info,
             struct iw_freq *fwrq, char *extra)
{    
   v_U32_t status = 0,channel;
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal;
   hdd_wext_state_t *pWextState; 
   tCsrRoamProfile * pRoamProfile;

   ENTER();
   
   pWextState = pAdapter->pWextState;
   hHal = pAdapter->hHal;
    
   pRoamProfile = &pWextState->roamProfile;

   if( pAdapter->conn_info.connState== eConnectionState_Associated )
   {
       if (ccmCfgGetInt(hHal, WNI_CFG_CURRENT_CHANNEL, &channel) != eHAL_STATUS_SUCCESS)
       {
           return -EIO;
       }
       else
       {
          fwrq->m = channel;
          fwrq->e = 0;   
       }
    }
    else
    {
       channel = pAdapter->conn_info.operationChannel; 
       if(channel <= (FREQ_CHAN_MAP_TABLE_SIZE - 1)) {
           fwrq->m = freq_chan_map[channel-1].freq * 100000;
           fwrq->e = 0;
       }
    }

   return status;
}

static int iw_get_tx_power(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
  
  VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
  eHalStatus status = eHAL_STATUS_SUCCESS;
  hdd_wext_state_t *pWextState;
  hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
  
  ENTER();
  
  if(eConnectionState_Associated != pAdapter->conn_info.connState) {
   
     wrqu->txpower.value = 0;
  }
  else {
    status = sme_GetStatistics( pAdapter->hHal, eCSR_HDD, 
                       SME_SUMMARY_STATS      |
                       SME_GLOBAL_CLASSA_STATS |
                       SME_GLOBAL_CLASSB_STATS |
                       SME_GLOBAL_CLASSC_STATS |
                       SME_GLOBAL_CLASSD_STATS |
                       SME_PER_STA_STATS,
                       hdd_StatisticsCB, 0, FALSE, 
                       pAdapter->conn_info.staId[0], pAdapter );
    
    if(eHAL_STATUS_SUCCESS != status)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD sme_GetStatistics failed!!\n"));
       return status;
    }
  
    pWextState = pAdapter->pWextState;
    
    vos_status = vos_wait_single_event(&pWextState->vosevent, 1000);
  
    if (!VOS_IS_STATUS_SUCCESS(vos_status))
    { 
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos wait for single_event failed!!\n"));
       return VOS_STATUS_E_FAILURE;
    }
  
    wrqu->txpower.value = pAdapter->hdd_stats.ClassA_stat.max_pwr;
  }
  
  EXIT();
  
  return vos_status;
  
    
}


static int iw_set_tx_power(struct net_device *dev,
                           struct iw_request_info *info,
                           union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;

    ENTER();
      
    if ( ccmCfgSetInt(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL, wrqu->txpower.value, ccmCfgSetCallback, eANI_BOOLEAN_TRUE) != eHAL_STATUS_SUCCESS ) 
    {
        return -EIO;
    }

    EXIT();

    return 0;
}

static int iw_get_bitrate(struct net_device *dev, 
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
   VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   hdd_wext_state_t *pWextState;
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   
   ENTER();
   
   if(eConnectionState_Associated != pAdapter->conn_info.connState) {
      
        wrqu->bitrate.value = 0;
   }
   else {
      status = sme_GetStatistics( pAdapter->hHal, eCSR_HDD, 
                               SME_SUMMARY_STATS       |
                               SME_GLOBAL_CLASSA_STATS |
                               SME_GLOBAL_CLASSB_STATS |
                               SME_GLOBAL_CLASSC_STATS |
                               SME_GLOBAL_CLASSD_STATS |
                               SME_PER_STA_STATS,
                               hdd_StatisticsCB, 0, FALSE, 
                               pAdapter->conn_info.staId[0], pAdapter );
      
      if(eHAL_STATUS_SUCCESS != status)
      {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD sme_GetStatistics failed!!\n"));
         return status;
      }
   
      pWextState = pAdapter->pWextState;
      
      vos_status = vos_wait_single_event(&pWextState->vosevent, 1000);
   
      if (!VOS_IS_STATUS_SUCCESS(vos_status))
      {   
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos wait for single_event failed!!\n"));
         return VOS_STATUS_E_FAILURE;
      }
   
      wrqu->bitrate.value = pAdapter->hdd_stats.ClassA_stat.tx_rate*500*1000;
   }

   EXIT();
   
   return vos_status;
}
/* ccm call back function */

static int iw_set_bitrate(struct net_device *dev, 
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, 
                          char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState;
    v_U8_t supp_rates[WNI_CFG_SUPPORTED_RATES_11A_LEN];
    v_U32_t a_len = WNI_CFG_SUPPORTED_RATES_11A_LEN;
    v_U32_t b_len = WNI_CFG_SUPPORTED_RATES_11B_LEN;
    v_U32_t i, rate;
    v_U32_t valid_rate = FALSE, active_phy_mode = 0;

    ENTER();
   
    pWextState =  pAdapter->pWextState;

    if (eConnectionState_Associated != pAdapter->conn_info.connState)
    {
        return -ENXIO ;
    }
   
    rate = wrqu->bitrate.value;

    if (rate == -1)
    {
        rate = WNI_CFG_FIXED_RATE_AUTO;
        valid_rate = TRUE;
    }
   else if (ccmCfgGetInt(pAdapter->hHal, 
                        WNI_CFG_DOT11_MODE, &active_phy_mode) == eHAL_STATUS_SUCCESS) 
    {
        if (active_phy_mode == WNI_CFG_DOT11_MODE_11A || active_phy_mode == WNI_CFG_DOT11_MODE_11G
            || active_phy_mode == WNI_CFG_DOT11_MODE_11B)
        {
            if ((ccmCfgGetStr(pAdapter->hHal, 
                        WNI_CFG_SUPPORTED_RATES_11A,
                        supp_rates, &a_len) == eHAL_STATUS_SUCCESS) &&
                (ccmCfgGetStr(pAdapter->hHal, 
                        WNI_CFG_SUPPORTED_RATES_11B,
                        supp_rates, &b_len) == eHAL_STATUS_SUCCESS))
            {
                for (i = 0; i < (b_len + a_len); ++i)
                {
                    /* supported rates returned is double the actual rate so we divide it by 2 */
                    if ((supp_rates[i]&0x7F)/2 == rate)
                    {
                        valid_rate = TRUE;
                        rate = i + WNI_CFG_FIXED_RATE_1MBPS;
                        break;
                    }
                }
            }
        }
    }
    if (valid_rate != TRUE)
    {
        return -EINVAL;
    }
    if (ccmCfgSetInt(pAdapter->hHal, 
                     WNI_CFG_FIXED_RATE, rate, 
                     ccmCfgSetCallback,eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS)
    {
        return -EIO;
    }
    return 0;
}


static int iw_set_genie(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, 
        char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t *pWextState = pAdapter->pWextState;
   u_int8_t *genie = wrqu->data.pointer;
   u_int8_t *pos;

   ENTER();
   if(!wrqu->data.length)
      return 0;

   hddLog(LOG1,"iw_set_genie ioctl IE[0x%X], LEN[%d]\n", genie[0], genie[1]);

   switch ( genie[0] ) 
   {
      case DOT11F_EID_WPA: 
         if (genie[1] < 2 + 4)
            return -EINVAL;
         else if (memcmp(&genie[2], "\x00\x50\xf2\x04", 4) == 0) 
         {
            hddLog (LOG1, "%s Set WPS IE(len %d)",__FUNCTION__, genie[1]+2);
           
             pWextState->wpsMode = eWEXT_WPS_ON;
             pWextState->roamProfile.bWPSAssociation = VOS_TRUE;
             if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_ASSOC_METHOD, 2,
                                ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
             {
                hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                return -EIO;
             }
             pos = &genie[6];
             while (((size_t)pos - (size_t)&genie[6])  < (genie[1] - 4) )
             {
                switch(*pos<<8 | *(pos+1))
                {
                   case HDD_WPS_ELEM_VERSION:
                      pos += 4;
                      hddLog(LOG1, "ver: %d\n", *pos);
                      if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_VERSION, *pos++,
                                       ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      break;
                   case HDD_WPS_ELEM_REQUEST_TYPE:
                      pos += 4;
                      hddLog(LOG1, "type: %d\n", *pos);
                      if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_REQUEST_TYPE, *pos++,
                        ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      break;
                   case HDD_WPS_ELEM_CONFIG_METHODS:
                      pos += 4;
                      hddLog(LOG1, "type: %d\n", (*pos<<8 | *(pos+1)));
                      if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_CFG_METHOD,
                        (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      break;
                   case HDD_WPS_ELEM_UUID_E:
                      pos += 4;
                      if (ccmCfgSetStr( pAdapter->hHal, WNI_CFG_WPS_UUID, pos,
                         HDD_WPS_UUID_LEN, ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += HDD_WPS_UUID_LEN;
                      break;
                   case HDD_WPS_ELEM_PRIMARY_DEVICE_TYPE:
                      pos += 4;
                      hddLog(LOG1, "primary dev category: %d\n", (*pos<<8 | *(pos+1)));  
                      if (ccmCfgSetInt( pAdapter->hHal, WNI_CFG_WPS_PRIMARY_DEVICE_CATEGORY, 
                        (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      hddLog(LOG1, "primary dev oui: %d\n", (*pos<<8 | *(pos+1)));
                      if (ccmCfgSetStr(pAdapter->hHal, WNI_CFG_WPS_PIMARY_DEVICE_OUI, pos, 4,
                          ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 4;
                      hddLog(LOG1, "primary dev sub category: %d\n", (*pos<<8 | *(pos+1)));  
                      if (ccmCfgSetInt( pAdapter->hHal, WNI_CFG_WPS_DEVICE_SUB_CATEGORY, 
                        (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      break;
                   case HDD_WPS_ELEM_RF_BANDS:
                      pos += 4;
                     //skip RF Band
                      pos++; 
                      break;
                   case HDD_WPS_ELEM_ASSOCIATION_STATE:
                      pos += 4;
                      hddLog(LOG1, "association state: %d\n", (*pos<<8 | *(pos+1)));
                      if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_ASSOCIATION_STATE, 
                         (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      break;
                   case HDD_WPS_ELEM_CONFIGURATION_ERROR:
                      pos += 4;
                      hddLog(LOG1, "config error: %d\n", (*pos<<8 | *(pos+1)));
                      if (ccmCfgSetInt(pAdapter->hHal, WNI_CFG_WPS_CONFIGURATION_ERROR, 
                        (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog( LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      break;
                   case HDD_WPS_ELEM_DEVICE_PASSWORD_ID:
                      pos += 4;
                      hddLog(LOG1, "password id: %d\n", (*pos<<8 | *(pos+1)));
                      if (ccmCfgSetInt( pAdapter->hHal, WNI_CFG_WPS_DEVICE_PASSWORD_ID, 
                       (*pos<<8 | *(pos+1)), ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      pos += 2;
                      if (ccmCfgSetInt( pAdapter->hHal, WNI_CFG_WPS_PROBE_REQ_FLAG, 1,
                         ccmCfgSetCallback, eANI_BOOLEAN_FALSE) != eHAL_STATUS_SUCCESS) 
                      {
                         hddLog(LOGE, FL("\n ccmCfgSetInt failed "));
                         return -EIO;
                      }
                      break;
                   default:
                      hddLog (LOGE, "UNKNOWN TLV in WPS IE(%x)\n", (*pos<<8 | *(pos+1)));
                      return -EINVAL; 
                 }
               }  
            }
            else {  
               hddLog (LOG1, "%s Set RSN IE",__FUNCTION__);            
               memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
               memcpy( pWextState->WPARSNIE, wrqu->data.pointer, wrqu->data.length);
               pWextState->roamProfile.pWPAReqIE = pWextState->WPARSNIE;
               pWextState->roamProfile.nWPAReqIELength = wrqu->data.length;
            }     
            break;
         case DOT11F_EID_RSN:
            hddLog (LOG1, "%s Set RSN IE",__FUNCTION__);            
            memset( pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN );
            memcpy( pWextState->WPARSNIE, wrqu->data.pointer, wrqu->data.length);
            pWextState->roamProfile.pRSNReqIE = pWextState->WPARSNIE;
            pWextState->roamProfile.nRSNReqIELength = wrqu->data.length;
            break;
                 
         default:
            hddLog (LOGE, "%s Set UNKNOWN IE %X",__FUNCTION__, genie[0]);
            return 0;
    }
    EXIT();
    return 0;
}

static int iw_get_genie(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, 
                        char *extra)
{
    hdd_wext_state_t *pWextState;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    eHalStatus status;
    v_U32_t length = DOT11F_IE_RSN_MAX_LEN;
    v_U8_t genIeBytes[DOT11F_IE_RSN_MAX_LEN];

    ENTER();

    hddLog(LOG1,"getGEN_IE ioctl\n");
   
    pWextState =  pAdapter->pWextState;

    if( pAdapter->conn_info.connState == eConnectionState_NotConnected)
    {
        return -ENXIO;
    }
   
    // Return something ONLY if we are associated with an RSN or WPA network
    if ( VOS_TRUE != hdd_IsAuthTypeRSN(pAdapter->hHal, 
                                                pWextState->roamProfile.negotiatedAuthType))  
    {
        return -ENXIO; 
    }
   
    // Actually retrieve the RSN IE from CSR.  (We previously sent it down in the CSR Roam Profile.)
    status = sme_RoamGetSecurityReqIE(pAdapter->hHal, &length, genIeBytes, eCSR_SECURITY_TYPE_WPA);
    
    wrqu->data.length = VOS_MIN((u_int16_t) length, DOT11F_IE_RSN_MAX_LEN);

    vos_mem_copy( wrqu->data.pointer, (v_VOID_t*)genIeBytes, wrqu->data.length);
      
    hddLog(LOG1,"%s: RSN IE of %d bytes returned\n", __FUNCTION__, wrqu->data.length ); 

    EXIT();

    return 0;
}

static int iw_get_encode(struct net_device *dev, 
                       struct iw_request_info *info,
                       struct iw_point *dwrq, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
    int keyId;
    eCsrAuthType authType = eCSR_AUTH_TYPE_NONE;
    int i;

    ENTER();

    keyId = pRoamProfile->Keys.defaultIndex;

    if(keyId < 0 || keyId > MAX_WEP_KEYS)
    {
        hddLog(LOG1,"%s: Invalid keyId : %d\n",__FUNCTION__,keyId);
        return -EINVAL;
    }

    if(pRoamProfile->Keys.KeyLength[keyId] > 0)
    {
        dwrq->flags |= IW_ENCODE_ENABLED;
        dwrq->length = pRoamProfile->Keys.KeyLength[keyId];
        vos_mem_copy(extra,&(pRoamProfile->Keys.KeyMaterial[keyId][0]),pRoamProfile->Keys.KeyLength[keyId]);
        
        dwrq->flags |= (keyId + 1);

    }
    else
    {
        dwrq->flags |= IW_ENCODE_DISABLED;
    }

    for(i=0; i < MAX_WEP_KEYS; i++)
    {
        if(pRoamProfile->Keys.KeyMaterial[i] == NULL)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if(MAX_WEP_KEYS == i)
    {
        dwrq->flags |= IW_ENCODE_NOKEY;
    }

    authType = pAdapter->conn_info.authType;

    if(eCSR_AUTH_TYPE_OPEN_SYSTEM == authType)
    {
        dwrq->flags |= IW_ENCODE_OPEN;
    }
    else
    {
        dwrq->flags |= IW_ENCODE_RESTRICTED;
    }
    EXIT();
    return 0;
}

#define PAE_ROLE_AUTHENTICATOR 1 // =1 for authenticator,
#define PAE_ROLE_SUPPLICANT 0 // =0 for supplicant

/*
 * This function sends a single 'key' to LIM at all time.
 */

static int iw_get_rts_threshold(struct net_device *dev,
            struct iw_request_info *info,
            union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal = pAdapter->hHal;
   v_U32_t threshold = 0;
   
   ENTER();   
   
   if ( ccmCfgGetInt(hHal, WNI_CFG_RTS_THRESHOLD, &threshold) != eHAL_STATUS_SUCCESS ) 
   {
      return -EIO;
   }
   wrqu->rts.value = threshold;

   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Rts-Threshold=%ld!!\n"),wrqu->rts.value);

   EXIT();
   
   return 0;
}

static int iw_set_rts_threshold(struct net_device *dev,
                                struct iw_request_info *info,
                                union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;

    ENTER();
   
    if ( wrqu->rts.value < WNI_CFG_RTS_THRESHOLD_STAMIN || wrqu->rts.value > WNI_CFG_RTS_THRESHOLD_STAMAX ) 
    {
        return -EINVAL;
    }

    if ( ccmCfgSetInt(hHal, WNI_CFG_RTS_THRESHOLD, wrqu->rts.value, ccmCfgSetCallback, eANI_BOOLEAN_TRUE) != eHAL_STATUS_SUCCESS ) 
    {
        return -EIO;
    }

    EXIT();

    return 0;
}

static int iw_get_frag_threshold(struct net_device *dev,
                                 struct iw_request_info *info,
                                 union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;
    v_U32_t threshold = 0;

    ENTER();

    if ( ccmCfgGetInt(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD, &threshold) != eHAL_STATUS_SUCCESS ) 
    {
        return -EIO;
    }
    wrqu->frag.value = threshold;
   
    EXIT();
    return 0;
}

static int iw_set_frag_threshold(struct net_device *dev,
             struct iw_request_info *info,
                 union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal = pAdapter->hHal;
   
   ENTER();   
   
    if ( wrqu->frag.value < WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN || wrqu->frag.value > WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX ) 
    {
        return -EINVAL;
    }

    if ( ccmCfgSetInt(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD, wrqu->frag.value, ccmCfgSetCallback, eANI_BOOLEAN_TRUE) != eHAL_STATUS_SUCCESS ) 
    {
        return -EIO;
    }
   
   EXIT();
   
   return 0;
}

static int iw_get_power_mode(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
   ENTER();
   return -EOPNOTSUPP;
}

static int iw_set_power_mode(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
    ENTER();
    return -EOPNOTSUPP;
}

static int iw_get_range(struct net_device *dev, struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal = pAdapter->hHal;
   struct iw_range *range = (struct iw_range *) extra;
   
   v_U8_t channels[WNI_CFG_VALID_CHANNEL_LIST_LEN];
   
   v_U32_t num_channels = sizeof(channels);
   v_U8_t supp_rates[WNI_CFG_SUPPORTED_RATES_11A_LEN];
   v_U32_t a_len = WNI_CFG_SUPPORTED_RATES_11A_LEN;
   
   v_U32_t b_len = WNI_CFG_SUPPORTED_RATES_11B_LEN;
   v_U32_t active_phy_mode = 0;
   v_U8_t index = 0, i;
   
   ENTER();
   
   wrqu->data.length = sizeof(struct iw_range);
   memset(range, 0, sizeof(struct iw_range));
   
 
    /*Get the phy mode*/
   if (ccmCfgGetInt(pAdapter->hHal, 
                  WNI_CFG_DOT11_MODE, &active_phy_mode) == eHAL_STATUS_SUCCESS) 
   {  
     VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("active_phy_mode = %ld\n"),active_phy_mode);
     
      if (active_phy_mode == WNI_CFG_DOT11_MODE_11A || active_phy_mode == WNI_CFG_DOT11_MODE_11G)
      { 
         /*Get the supported rates for 11G band*/
           if (ccmCfgGetStr(pAdapter->hHal, 
                  WNI_CFG_SUPPORTED_RATES_11A,
                  supp_rates, &a_len) == eHAL_STATUS_SUCCESS)
           {
               if(a_len <= IW_MAX_BITRATES) {
                 for (i = 0; i < a_len; ++i)
                 {
                   range->bitrate[i] = ((supp_rates[i]& 0x7F)/2)*1000000;
                 }
               }   
           else 
           {
                 return -EIO;
               }
           }
           range->num_bitrates = a_len;
      }
     else if (active_phy_mode == WNI_CFG_DOT11_MODE_11B) 
     {
         /*Get the supported rates for 11B band*/
           if (ccmCfgGetStr(pAdapter->hHal, 
                  WNI_CFG_SUPPORTED_RATES_11B,
                  supp_rates, &b_len) == eHAL_STATUS_SUCCESS)
           {
              if(b_len <= IW_MAX_BITRATES)
              {
                  for (i = 0; i < b_len; ++i)
                  {
                     range->bitrate[i] = ((supp_rates[i]& 0x7F)/2)*1000000;
                  }
              }
              else {
               return -EIO;
              }
           }
           range->num_bitrates = b_len;
      }
     
   }
 

   range->max_rts = WNI_CFG_RTS_THRESHOLD_STAMAX;
   range->min_frag = WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN;
   range->max_frag = WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX;
   
   range->encoding_size[0] = 5;
   range->encoding_size[1] = 13;
   range->num_encoding_sizes = 2;
   range->max_encoding_tokens = MAX_WEP_KEYS;
   
   // we support through Wireless Extensions 22
   range->we_version_compiled = WIRELESS_EXT;
   range->we_version_source = 22;
   
   /*Supported Channels and Frequencies*/
   if (ccmCfgGetStr((hHal), WNI_CFG_VALID_CHANNEL_LIST, channels, &num_channels) != eHAL_STATUS_SUCCESS){
      return -EIO;
   }
   if (num_channels > IW_MAX_FREQUENCIES){
      num_channels = IW_MAX_FREQUENCIES;
   }
     
   range->num_channels = num_channels;
   range->num_frequency = num_channels;
  
   for(index=0; index < num_channels; index++)
   {
      v_U32_t frq_indx = 0;
   
      range->freq[index].i = channels[index];
      while (frq_indx <  FREQ_CHAN_MAP_TABLE_SIZE)
      {
           if(channels[index] == freq_chan_map[frq_indx].chan)
           {
             range->freq[index].m = freq_chan_map[frq_indx].freq * 100000;
             range->freq[index].e = 1;
             break;
           }
           frq_indx++;
      }
   } 
 
   /* Event capability (kernel + driver) */
   range->event_capa[0] = (IW_EVENT_CAPA_K_0 |
                    IW_EVENT_CAPA_MASK(SIOCGIWAP) |
                    IW_EVENT_CAPA_MASK(SIOCGIWSCAN));
   range->event_capa[1] = IW_EVENT_CAPA_K_1;
   
   /*Encryption capability*/
   range->enc_capa = IW_ENC_CAPA_WPA | IW_ENC_CAPA_WPA2 |
                IW_ENC_CAPA_CIPHER_TKIP | IW_ENC_CAPA_CIPHER_CCMP;
   
   /* Txpower capability */
   range->txpower_capa = IW_TXPOW_MWATT;
   
   /*Scanning capability*/
   #if WIRELESS_EXT >= 22
   range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE | IW_SCAN_CAPA_CHANNEL;
   #endif
   
   EXIT(); 
   return 0;
}

/* Callback function registered with PMC to know status of PMC request */
void iw_priv_callback_fn (void *callbackContext, eHalStatus status)
{
  struct completion *completion_var = (struct completion*) callbackContext;

  hddLog(LOGE, "Received callback from PMC with status = %d\n", status);
  if(completion_var != NULL)
    complete(completion_var);
}

static int iw_set_priv(struct net_device *dev,
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    char *cmd = (char*)wrqu->data.pointer;
    int ret=0;
    int status= VOS_STATUS_SUCCESS;
    hdd_wext_state_t *pWextState = pAdapter->pWextState;

    ENTER();

    hddLog(VOS_TRACE_LEVEL_INFO_MED, "***Received %s cmd from Wi-Fi GUI***", cmd);

    if( strcasecmp(cmd, "start") == 0 ) {

        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Start command\n");
        /*Exit from Deep sleep or standby if we get the driver START cmd from android GUI*/
        if(pAdapter->hdd_ps_state == eHDD_SUSPEND_STANDBY) 
        {
           
           hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: WLAN being exit from Stand by\n",__func__);
           status = hdd_exit_standby(pAdapter);
        } 
        else if(pAdapter->hdd_ps_state == eHDD_SUSPEND_DEEP_SLEEP) 
        {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s: WLAN being exit from deep sleep\n",__func__);
            status = hdd_exit_deep_sleep(pAdapter);
        }
        else {
            hddLog(VOS_TRACE_LEVEL_INFO_LOW, "%s: Not in standby or deep sleep. "
               "Ignore start cmd %d", __func__, pAdapter->hdd_ps_state);
            status = VOS_STATUS_E_FAILURE;
        }
        
        if(status == VOS_STATUS_SUCCESS) {
            union iwreq_data wrqu;
            char buf[10];

            strcpy(buf,"START");
            memset(&wrqu, 0, sizeof(wrqu));
            wrqu.data.length = strlen(buf);
            wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
        }
        goto done;
    }
    else if( strcasecmp(cmd, "stop") == 0 ) {

        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Stop command\n");

        if(pAdapter->cfg_ini->nEnableDriverStop == WLAN_MAP_DRIVER_STOP_TO_STANDBY) 
        {
            //Execute standby procedure. Executing standby procedure will cause the STA to
            //disassociate first and then the chip will be put into standby.
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Wlan driver entering Stand by mode\n");
            status  = hdd_enter_standby(pAdapter);
        }
        else if(pAdapter->cfg_ini->nEnableDriverStop == WLAN_MAP_DRIVER_STOP_TO_DEEP_SLEEP) {
            //Execute deep sleep procedure
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Wlan driver entering deep sleep mode\n");
            status = hdd_enter_deep_sleep(pAdapter);
        }
        else {
            hddLog(VOS_TRACE_LEVEL_INFO_LOW, "%s: Driver stop is not enabled %d",
             __func__, pAdapter->cfg_ini->nEnableDriverStop);
            status = VOS_STATUS_E_FAILURE;
        }

        if(status == VOS_STATUS_SUCCESS) {
            union iwreq_data wrqu;
            char buf[10];

            strcpy(buf,"STOP");
            memset(&wrqu, 0, sizeof(wrqu));
            wrqu.data.length = strlen(buf);
            wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);
        }
        
        goto done;
    }
    else if( strcasecmp(cmd, "macaddr") == 0 ) {

        ret = sprintf(cmd, "Macaddr = " MAC_ADDRESS_STR "\n", MAC_ADDR_ARRAY(pAdapter->macAddressCurrent.bytes));
        hddLog( VOS_TRACE_LEVEL_INFO_HIGH, "cmd %s", cmd);
        goto done;
    }
    else if ((strcasecmp(cmd, "scan-passive") == 0) || (strcasecmp(cmd, "scan-active") == 0)) 
    {        
        pAdapter->pWextState->scan_mode = (!strcasecmp(cmd, "scan-passive")) ? eSIR_PASSIVE_SCAN : eSIR_ACTIVE_SCAN; 

        goto done;
    }
    else if( strcasecmp(cmd, "scan-mode") == 0 ) 
    {
        v_U16_t cmd_len = wrqu->data.length;
        hddLog(VOS_TRACE_LEVEL_INFO, "Scan Mode command\n"); 
        ret = snprintf(cmd, cmd_len, "ScanMode = %u\n", pAdapter->pWextState->scan_mode);
        if (ret < (int)cmd_len) {
              return( ret );
        }
    }
    else if( strcasecmp(cmd, "linkspeed") == 0 ) 
    {
        v_U16_t link_speed=0;

        hddLog(VOS_TRACE_LEVEL_INFO, "Link Speed command\n"); 
        if(eConnectionState_Associated != pAdapter->conn_info.connState) {
            wrqu->bitrate.value = 0;
        }
        else {
           status = sme_GetStatistics( pAdapter->hHal, eCSR_HDD, 
                                    SME_SUMMARY_STATS       |
                                    SME_GLOBAL_CLASSA_STATS |
                                    SME_GLOBAL_CLASSB_STATS |
                                    SME_GLOBAL_CLASSC_STATS |
                                    SME_GLOBAL_CLASSD_STATS |
                                    SME_PER_STA_STATS,
                                    hdd_StatisticsCB, 0, FALSE, 
                                    pAdapter->conn_info.staId[0], pAdapter );
           
           if(eHAL_STATUS_SUCCESS != status)
           {
              hddLog( VOS_TRACE_LEVEL_ERROR, "HDD sme_GetStatistics failed");
              return status;
           }
                   
           status = vos_wait_single_event(&pWextState->vosevent, 1000);
        
           if (!VOS_IS_STATUS_SUCCESS(status))
           {   
              hddLog( VOS_TRACE_LEVEL_ERROR, "HDD vos wait for single_event failed");
              return VOS_STATUS_E_FAILURE;
           }
        
           link_speed = (pAdapter->hdd_stats.ClassA_stat.tx_rate/2);
        }

        ret = sprintf(cmd,"LinkSpeed %u\n", link_speed);
        
        hddLog( VOS_TRACE_LEVEL_INFO_MED, "cmd %s\n", cmd); 
    }
    else if( strncasecmp(cmd, "COUNTRY", 7) == 0 ) {
        char *country_code;

        country_code =  cmd + 8;
        /*TODO:Set the country code to sme*/
    }
    else if( strncasecmp(cmd, "rssi", 4) == 0 ) {

        v_S7_t s7Rssi = 0;
        int  len;
        
        hddLog( VOS_TRACE_LEVEL_INFO_MED, "rssi command"); 

        if(pAdapter->conn_info.connState == eConnectionState_Associated) {

            len = pAdapter->conn_info.SSID.SSID.length;
            if( (len > 0) && (len <= 32) ) {
                memcpy( (void *)cmd, (void *)pAdapter->conn_info.SSID.SSID.ssId, len );
                ret = len;
                
                status = WLANTL_GetRssi( pAdapter->pvosContext, pAdapter->conn_info.staId[ 0 ], &s7Rssi );
                if ( !VOS_IS_STATUS_SUCCESS( status ) )
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR, "%s Failed\n", __func__); 
                    goto done; 
                }
                ret += sprintf(&cmd[ret], " rssi %d\n", s7Rssi);
                
                hddLog(VOS_TRACE_LEVEL_INFO_MED, "cmd %s\n", cmd); 
            }
             
        }
        else
        {
            hddLog( VOS_TRACE_LEVEL_INFO, "cmd %s\n", cmd); 
            ret = sprintf(cmd, " rssi %d\n", s7Rssi);
        }
        
    }
    else if( strncasecmp(cmd, "powermode", 9) == 0 ) {
        int mode;
        char *ptr = (char*)(cmd + 9); 
        
        sscanf(ptr,"%d",&mode);

        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "mode=%d\n",mode);
        
        init_completion(&pWextState->completion_var);

        if(mode == DRIVER_POWER_MODE_ACTIVE) 
        {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Wlan driver Entering Full Power\n");
            status = sme_RequestFullPower(pAdapter->hHal, iw_priv_callback_fn,
                          &pWextState->completion_var, eSME_FULL_PWR_NEEDED_BY_HDD);
       
            if(status == eHAL_STATUS_PMC_PENDING)
                wait_for_completion_interruptible(&pWextState->completion_var);
        }
        else if (mode == DRIVER_POWER_MODE_AUTO)
        {
            
            if (pAdapter->cfg_ini->fIsBmpsEnabled) {
                
                hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "Wlan driver Entering Bmps\n");
                status = sme_RequestBmps(pAdapter->hHal, iw_priv_callback_fn, &pAdapter->pWextState->completion_var);
    
                if (status == eHAL_STATUS_PMC_PENDING)
                    wait_for_completion_interruptible(&pWextState->completion_var);
            }
            else 
            {
               hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"BMPS is not enabled in the cfg\n");
            }
        }
        hddLog(VOS_TRACE_LEVEL_INFO, "Power Mode command"); 

        
        /*TODO:Set the power mode*/
    }
    else if (strncasecmp(cmd, "getpower", 8) == 0 ) {
        v_U32_t pmc_state; 
        v_U16_t value = DRIVER_POWER_MODE_ACTIVE;

        hddLog( VOS_TRACE_LEVEL_INFO, "Get Power\n"); 
       
        pmc_state = pmcGetPmcState(pAdapter->hHal);

        if(pmc_state == BMPS) {
           value = DRIVER_POWER_MODE_AUTO;
           ret = sprintf(cmd, "powermode = %u\n", value);
        }
        else {
           value = DRIVER_POWER_MODE_ACTIVE;  
           ret = sprintf(cmd, "powermode = %u\n", value);
        }
    }
    else if( strncasecmp(cmd, "btcoexmode", 10) == 0 ) {
        hddLog( VOS_TRACE_LEVEL_INFO, "btcoexmode\n"); 
        /*TODO: set the btcoexmode*/
    }
    else if( strcasecmp(cmd, "btcoexstat") == 0 ) {
        
        hddLog(VOS_TRACE_LEVEL_INFO, "BtCoex Status\n"); 
        /*TODO: Return the btcoex status*/
    }
    else if( strcasecmp(cmd, "rxfilter-start") == 0 ) {
        
        hddLog(VOS_TRACE_LEVEL_INFO, "Rx Data Filter Start command\n"); 
        
        /*TODO: Enable Rx data Filter*/        
    }
    else if( strcasecmp(cmd, "rxfilter-stop") == 0 ) {
        
        hddLog(VOS_TRACE_LEVEL_INFO, "Rx Data Filter Stop command\n"); 
        
        /*TODO: Disable Rx data Filter*/        
    }
    else if( strcasecmp(cmd, "rxfilter-statistics") == 0 ) {
       
        hddLog( VOS_TRACE_LEVEL_INFO, "Rx Data Filter Statistics command\n"); 
        /*TODO: rxfilter-statistics*/ 
    }
    else if( strncasecmp(cmd, "rxfilter-add", 12) == 0 ) {
        
        hddLog( VOS_TRACE_LEVEL_INFO, "rxfilter-add\n"); 
        /*TODO: rxfilter-add*/ 
    }
    else if( strncasecmp(cmd, "rxfilter-remove",15) == 0 ) {
        
        hddLog( VOS_TRACE_LEVEL_INFO, "rxfilter-remove\n"); 
        /*TODO: rxfilter-remove*/
    }
    else {
        hddLog( VOS_TRACE_LEVEL_ERROR,"Unsupported GUI command %s", cmd);
    }
done:
    return status;
   
}
static int iw_set_nick(struct net_device *dev, 
                       struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
   ENTER();
   return 0;
}

static int iw_get_nick(struct net_device *dev, 
                       struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
   ENTER();
   return 0;
}

static struct iw_statistics *get_wireless_stats(struct net_device *dev)
{
   ENTER();
   return NULL;
}

static int iw_set_encode(struct net_device *dev,struct iw_request_info *info,
                        union iwreq_data *wrqu,char *extra)

{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t  *pWextState =  pAdapter->pWextState;   
   struct iw_point *encoderq = &(wrqu->encoding);
   v_U32_t keyId;
   v_U8_t key_length;
   eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
   v_BOOL_t fKeyPresent = 0;
   int i;
   eHalStatus status = eHAL_STATUS_SUCCESS;   
   
   
   ENTER();    
   
   
   keyId = encoderq->flags & IW_ENCODE_INDEX;
      
   if(keyId)
   {
       if(keyId > MAX_WEP_KEYS)
       {
           return -EINVAL;
       }
      
       fKeyPresent = 1;
       keyId--;
   }
   else
   {
       fKeyPresent = 0;
   }   

   
   if(wrqu->data.flags & IW_ENCODE_DISABLED)
   {    
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "****iwconfig wlan0 key off*****\n");
       if(!fKeyPresent) {
        
          for(i=0;i < CSR_MAX_NUM_KEY; i++) {
         
             if(pWextState->roamProfile.Keys.KeyMaterial[i])   
                pWextState->roamProfile.Keys.KeyLength[i] = 0;
          }
       }
       pAdapter->conn_info.authType =  eCSR_AUTH_TYPE_OPEN_SYSTEM; 
       pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED; 
       pWextState->roamProfile.EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE; 
       pWextState->roamProfile.mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
       
       pAdapter->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
       pAdapter->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE; 
      
       if(eConnectionState_Associated == pAdapter->conn_info.connState)
       {
           init_completion(&pAdapter->disconnect_comp_var);
           status = sme_RoamDisconnect( pAdapter->hHal,  eCSR_DISCONNECT_REASON_UNSPECIFIED );
           if(VOS_STATUS_SUCCESS == status)
                 wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                     msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
       }
   
       return status;
   
   }
   
   if (wrqu->data.flags & (IW_ENCODE_OPEN | IW_ENCODE_RESTRICTED)) 
   {
      hddLog(VOS_TRACE_LEVEL_INFO, "iwconfig wlan0 key on");
   
      pAdapter->conn_info.authType = (encoderq->flags & IW_ENCODE_RESTRICTED) ? eCSR_AUTH_TYPE_SHARED_KEY : eCSR_AUTH_TYPE_OPEN_SYSTEM;
   
   }   
   
      
   if(wrqu->data.length > 0)
   {
       hddLog(VOS_TRACE_LEVEL_INFO, "%s : wrqu->data.length : %d",__FUNCTION__,wrqu->data.length);
   
       key_length = wrqu->data.length;        
   
       /* IW_ENCODING_TOKEN_MAX is the value that is set for wrqu->data.length by iwconfig.c when 'iwconfig wlan0 key on' is issued.*/
      
       if(5 == key_length)
       {   
           hddLog(VOS_TRACE_LEVEL_INFO, "%s: Call with WEP40,key_len=%d",__FUNCTION__,key_length);
         
           if((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
           {
               encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
           }
           else
           {
               encryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
           }
       }
       else if(13 == key_length)
       {
           hddLog(VOS_TRACE_LEVEL_INFO, "%s:Call with WEP104,key_len:%d",__FUNCTION__,key_length);
      
           if((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
           {
               encryptionType = eCSR_ENCRYPT_TYPE_WEP104; 
           }
           else
           {
               encryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
           }
       }
       else 
       {
           hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid WEP key length :%d",__FUNCTION__,key_length);
           return -EINVAL;
       }  
      
       pAdapter->conn_info.ucEncryptionType = encryptionType;
       pAdapter->conn_info.mcEncryptionType = encryptionType;
       pWextState->roamProfile.EncryptionType.numEntries = 1;
       pWextState->roamProfile.EncryptionType.encryptionType[0] = encryptionType;
       pWextState->roamProfile.mcEncryptionType.numEntries = 1;
       pWextState->roamProfile.mcEncryptionType.encryptionType[0] = encryptionType;
          
       if((eConnectionState_NotConnected == pAdapter->conn_info.connState) && 
            ((eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType) || 
              (eCSR_AUTH_TYPE_SHARED_KEY == pAdapter->conn_info.authType)))                                                                                    
       {
         
          vos_mem_copy(&pWextState->roamProfile.Keys.KeyMaterial[keyId][0],extra,key_length);
      
          pWextState->roamProfile.Keys.KeyLength[keyId] = (v_U8_t)key_length;
          pWextState->roamProfile.Keys.defaultIndex = (v_U8_t)keyId;
    
          return status;    
       }
   }

   return 0; 
}

static int iw_get_encodeext(struct net_device *dev,
               struct iw_request_info *info,
               struct iw_point *dwrq,
               char *extra)
{ 
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState= pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
    int keyId;
    eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
    eCsrAuthType authType = eCSR_AUTH_TYPE_NONE;
    int i;

    ENTER();

    keyId = pRoamProfile->Keys.defaultIndex;

    if(keyId < 0 || keyId > MAX_WEP_KEYS)
    {
        hddLog(LOG1,"%s: Invalid keyId : %d\n",__FUNCTION__,keyId);
        return -EINVAL;
    }

    if(pRoamProfile->Keys.KeyLength[keyId] > 0)
    {
        dwrq->flags |= IW_ENCODE_ENABLED;
        dwrq->length = pRoamProfile->Keys.KeyLength[keyId];
        palCopyMemory(dev,extra,&(pRoamProfile->Keys.KeyMaterial[keyId][0]),pRoamProfile->Keys.KeyLength[keyId]);
    }
    else
    {
        dwrq->flags |= IW_ENCODE_DISABLED;
    }

    for(i=0; i < MAX_WEP_KEYS; i++)
    {
        if(pRoamProfile->Keys.KeyMaterial[i] == NULL)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if(MAX_WEP_KEYS == i)
    {
        dwrq->flags |= IW_ENCODE_NOKEY;
    }
    else
    {
        dwrq->flags |= IW_ENCODE_ENABLED;
    }

    encryptionType = pRoamProfile->EncryptionType.encryptionType[0];

    if(eCSR_ENCRYPT_TYPE_NONE == encryptionType)
    {
        dwrq->flags |= IW_ENCODE_DISABLED;
    }

    authType = pAdapter->conn_info.authType;

    if(IW_AUTH_ALG_OPEN_SYSTEM == authType)
    {
        dwrq->flags |= IW_ENCODE_OPEN;
    }
    else
    {
        dwrq->flags |= IW_ENCODE_RESTRICTED;
    }
    EXIT();
    return 0;

}

static int iw_set_encodeext(struct net_device *dev, 
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t  *pWextState =  pAdapter->pWextState;   
    eHalStatus halStatus= eHAL_STATUS_SUCCESS;

    tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
    v_U32_t status = 0; 

    struct iw_encode_ext *ext = (struct iw_encode_ext*)extra;

    v_U8_t groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    int key_index;
    struct iw_point *encoding = &wrqu->encoding;
    tCsrRoamSetKey  setKey;   
    v_U32_t  roamId= 0xFF;
   
    ENTER();    
   
    key_index = encoding->flags & IW_ENCODE_INDEX;
   
    if(key_index > 0) {
      
         /*Convert from 1-based to 0-based keying*/
        key_index--;
    }
    if(!ext->key_len) {
      
      /*Set the encrytion type to NONE*/
       pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
       return status;
    }   

    if(eConnectionState_NotConnected == pAdapter->conn_info.connState && 
                                                  (IW_ENCODE_ALG_WEP == ext->alg))
    {    
       if(IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) {
         
          VOS_TRACE (VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,("Invalid Configuration:%s \n"),__FUNCTION__);      
          return -EINVAL;
       }
       else {
         /*Static wep, update the roam profile with the keys */ 
          if(ext->key && (ext->key_len <= eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES) &&
                                                               key_index <=CSR_MAX_NUM_KEY) {
             vos_mem_copy(&pRoamProfile->Keys.KeyMaterial[key_index][0],ext->key,ext->key_len);
             pRoamProfile->Keys.KeyLength[key_index] = (v_U8_t)ext->key_len;
          
             if(ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
                pRoamProfile->Keys.defaultIndex = (v_U8_t)key_index;
          
          }
       }
       return status;
    }
   
    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
   
    setKey.keyId = key_index;
    setKey.keyLength = ext->key_len;
   
    if(ext->key_len <= CSR_MAX_KEY_LEN) {
       vos_mem_copy(&setKey.Key[0],ext->key,ext->key_len);
    }   
   
    if(ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
      /*Key direction for group is RX only*/
       setKey.keyDirection = eSIR_RX_ONLY;
       vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
    }   
    else {
      
       setKey.keyDirection =  eSIR_TX_RX;
       vos_mem_copy(setKey.peerMac,ext->addr.sa_data,WNI_CFG_BSSID_LEN);
    }
   
    /*For supplicant pae role is zero*/
    setKey.paeRole = 0;
      
    switch(ext->alg)
    {   
       case IW_ENCODE_ALG_NONE:   
         setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
         break;
         
       case IW_ENCODE_ALG_WEP:
         setKey.encType = (ext->key_len== 5) ? eCSR_ENCRYPT_TYPE_WEP40:eCSR_ENCRYPT_TYPE_WEP104;
         break;
      
       case IW_ENCODE_ALG_TKIP:
       {
          v_U8_t *pKey = &setKey.Key[0];
  
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
          vos_mem_copy(pKey,ext->key,16);
           
         /*Copy the rx mic first*/
          vos_mem_copy(&pKey[16],&ext->key[24],8); 
          
         /*Copy the tx mic */
          vos_mem_copy(&pKey[24],&ext->key[16],8); 
  
       }     
       break;
      
       case IW_ENCODE_ALG_CCMP:
          setKey.encType = eCSR_ENCRYPT_TYPE_AES;
          break;
          
       default:
          setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
          break;
    }
         
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
          ("%s:cipher_alg:%d key_len[%d] *pEncryptionType :%d \n"),__FUNCTION__,(int)ext->alg,(int)ext->key_len,setKey.encType);
   
    pAdapter->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;   
   
    
    halStatus = sme_RoamSetKey( pAdapter->hHal, &setKey, &roamId );
    
    if ( halStatus != eHAL_STATUS_SUCCESS )
    {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "[%4d] sme_RoamSetKey returned ERROR status= %d", __LINE__, halStatus );

       pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
    }   
   
   return halStatus;
}
 
static int iw_set_retry(struct net_device *dev, struct iw_request_info *info,
           union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal = pAdapter->hHal;
   
   ENTER();

   if(wrqu->retry.value < WNI_CFG_LONG_RETRY_LIMIT_STAMIN ||
       wrqu->retry.value > WNI_CFG_LONG_RETRY_LIMIT_STAMAX) {
       
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Invalid Retry-Limit=%ld!!\n"),wrqu->retry.value);
      
      return -EINVAL;
   }
   
   if(wrqu->retry.flags & IW_RETRY_LIMIT) {
      
       if((wrqu->retry.flags & IW_RETRY_LONG)) 
       {
          if ( ccmCfgSetInt(hHal, WNI_CFG_LONG_RETRY_LIMIT, wrqu->retry.value, ccmCfgSetCallback, eANI_BOOLEAN_TRUE) != eHAL_STATUS_SUCCESS ) 
          {
             return -EIO;
          }
       }
       else if((wrqu->retry.flags & IW_RETRY_SHORT)) 
       {      
          if ( ccmCfgSetInt(hHal, WNI_CFG_SHORT_RETRY_LIMIT, wrqu->retry.value, ccmCfgSetCallback, eANI_BOOLEAN_TRUE) != eHAL_STATUS_SUCCESS ) 
          {
             return -EIO;
          }   
       }
   }
   else 
   {
       return -EOPNOTSUPP;
   }
      
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Set Retry-Limit=%ld!!\n"),wrqu->retry.value);
   
   EXIT();
   
   return 0;

}

static int iw_get_retry(struct net_device *dev, struct iw_request_info *info,
           union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   tHalHandle hHal = pAdapter->hHal;
   v_U32_t retry = 0;
   
   ENTER();  

   if((wrqu->retry.flags & IW_RETRY_LONG)) 
   {
      wrqu->retry.flags = IW_RETRY_LIMIT | IW_RETRY_LONG;
      
      if ( ccmCfgGetInt(hHal, WNI_CFG_LONG_RETRY_LIMIT, &retry) != eHAL_STATUS_SUCCESS ) 
      {
         return -EIO;
      }
      
      wrqu->retry.value = retry;
   }
   else if ((wrqu->retry.flags & IW_RETRY_SHORT))
   {
      wrqu->retry.flags = IW_RETRY_LIMIT | IW_RETRY_SHORT;
      
      if ( ccmCfgGetInt(hHal, WNI_CFG_SHORT_RETRY_LIMIT, &retry) != eHAL_STATUS_SUCCESS ) 
      {
         return -EIO;
      }
      
      wrqu->retry.value = retry;
   }
   else {
      return -EOPNOTSUPP;
   }
      
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("Retry-Limit=%ld!!\n"),retry);
   
   EXIT();
   
   return 0;
}

static int iw_set_mlme(struct net_device *dev,
                       struct iw_request_info *info,
                       union iwreq_data *wrqu,
                       char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    struct iw_mlme *mlme = (struct iw_mlme *)extra;
    eHalStatus status = eHAL_STATUS_SUCCESS;
 
    ENTER();    
   
    //reason_code is unused. By default it is set to eCSR_DISCONNECT_REASON_UNSPECIFIED
    switch (mlme->cmd) {
        case IW_MLME_DISASSOC:
        case IW_MLME_DEAUTH:

            if( pAdapter->conn_info.connState == eConnectionState_Associated ) 
            {
                eCsrRoamDisconnectReason reason = eCSR_DISCONNECT_REASON_UNSPECIFIED;
                
                if( mlme->reason_code == HDD_REASON_MICHAEL_MIC_FAILURE )
                    reason = eCSR_DISCONNECT_REASON_MIC_ERROR;
                
                init_completion(&pAdapter->disconnect_comp_var);
                status = sme_RoamDisconnect( pAdapter->hHal, reason);
                
                if(VOS_STATUS_SUCCESS == status)
                    wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                        msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
                else
                    hddLog(LOGE,"%s %d Command Disassociate/Deauthenticate : csrRoamDisconnect failure returned %d \n",
                       __FUNCTION__, (int)mlme->cmd, (int)status );

               netif_tx_disable(dev);
               netif_carrier_off(dev);

            }
            else
            {
                hddLog(LOGE,"%s %d Command Disassociate/Deauthenticate called but station is not in associated state \n", __FUNCTION__, (int)mlme->cmd );
            }
            break;
        default:
            hddLog(LOGE,"%s %d Command should be Disassociate/Deauthenticate \n", __FUNCTION__, (int)mlme->cmd );
            return -EINVAL;
    }//end of switch

    EXIT();

    return status;

}

/* set param sub-ioctls */
static int iw_setint_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;
    int *value = (int *)extra;
    int sub_cmd = value[0];
    int set_value = value[1];
    int ret = 0; /* success */
    int enable_pbm, enable_mp;
     v_U8_t nEnableSuspendOld;
    
    init_completion(&pAdapter->pWextState->completion_var);
    
    switch(sub_cmd)
    {
        case WE_SET_11D_STATE:
        {
            tSmeConfigParams smeConfig;;
            if((ENABLE_11D == set_value) || (DISABLE_11D == set_value)) {
               
                sme_GetConfigParam(pAdapter->hHal,&smeConfig.csrConfig);
                smeConfig.csrConfig.Is11dSupportEnabled = (v_BOOL_t)set_value;

                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("11D state=%ld!!\n"),smeConfig.csrConfig.Is11dSupportEnabled);
                
                sme_UpdateConfig(pAdapter->hHal,&smeConfig);
            } 
            else {
               return -EINVAL;
            }
            break;
        }

        case WE_WOWL:
        {
           switch (set_value) 
           {
              case 0x00: 
                 hdd_exit_wowl();
                 break;
              case 0x01: 
              case 0x02: 
              case 0x03: 
                 enable_mp =  (set_value & 0x01) ? 1 : 0;
                 enable_pbm = (set_value & 0x02) ? 1 : 0;
                 hddLog(LOGE, "magic packet ? = %s pattern byte matching ? = %s\n", 
                     (enable_mp ? "YES":"NO"), (enable_pbm ? "YES":"NO"));
                 hdd_enter_wowl(enable_mp, enable_pbm);
                 break;
              default:
                 hddLog(LOGE, "Invalid arg  %d in WE_WOWL IOCTL\n", set_value);
                 ret = -EINVAL;
                 break;
           }

           break;
        }
        case WE_SET_POWER:
        {
           switch (set_value) 
           {
              case  0: //Full Power
                 if(sme_RequestFullPower(hHal, iw_priv_callback_fn,
                       &pAdapter->pWextState->completion_var, eSME_FULL_PWR_NEEDED_BY_HDD) ==
                       eHAL_STATUS_PMC_PENDING)
                    wait_for_completion_interruptible(&pAdapter->pWextState->completion_var);
                 hddLog(LOGE, "iwpriv Full Power completed\n");
                 break;
              case  1: //Enable BMPS
                 sme_EnablePowerSave(hHal, ePMC_BEACON_MODE_POWER_SAVE);
                 break;
              case  2: //Disable BMPS
                 sme_DisablePowerSave(hHal, ePMC_BEACON_MODE_POWER_SAVE);
                 break;
              case  3: //Request Bmps
                 if(sme_RequestBmps(hHal, iw_priv_callback_fn, &pAdapter->pWextState->completion_var) == 
                    eHAL_STATUS_PMC_PENDING)
                    wait_for_completion_interruptible(&pAdapter->pWextState->completion_var);
                 hddLog(LOGE, "iwpriv Request BMPS completed\n");
                 break;
              case  4: //Enable IMPS
                 sme_EnablePowerSave(hHal, ePMC_IDLE_MODE_POWER_SAVE);
                 break;
              case  5: //Disable IMPS
                 sme_DisablePowerSave(hHal, ePMC_IDLE_MODE_POWER_SAVE);
                 break;
              case  6: //Enable Standby
                 sme_EnablePowerSave(hHal, ePMC_STANDBY_MODE_POWER_SAVE);
                 break;
              case  7: //Disable Standby
                 sme_DisablePowerSave(hHal, ePMC_STANDBY_MODE_POWER_SAVE);
                 break;
              case  8: //Request Standby
                 (void)hdd_enter_standby(pAdapter);
                 break;
              case  9: //Start Auto Bmps Timer
                 sme_StartAutoBmpsTimer(hHal);
                 break;
              case  10://Stop Auto BMPS Timer
                 sme_StopAutoBmpsTimer(hHal);
                 break;
              case  11://suspend to standby
                 nEnableSuspendOld = pAdapter->cfg_ini->nEnableSuspend;
                 pAdapter->cfg_ini->nEnableSuspend = 1;
                 hdd_suspend_wlan(NULL);
                 pAdapter->cfg_ini->nEnableSuspend = nEnableSuspendOld;
                 break;
              case  12://suspend to deep sleep
                 nEnableSuspendOld = pAdapter->cfg_ini->nEnableSuspend;
                 pAdapter->cfg_ini->nEnableSuspend = 2;
                 hdd_suspend_wlan(NULL);
                 pAdapter->cfg_ini->nEnableSuspend = nEnableSuspendOld;
                 break;
              case  13://resume from suspend
                 hdd_resume_wlan(NULL);
                 break;
              case  14://reset wlan (power down/power up)
                 vos_chipReset(NULL, VOS_FALSE, NULL, NULL);
                 break;					  
              default:
                 hddLog(LOGE, "Invalid arg  %d in WE_SET_POWER IOCTL\n", set_value);
                 ret = -EINVAL;
                 break;
           }
           break;
        }
           
        default:  
        {
            hddLog(LOGE, "Invalid IOCTL setvalue command %d value %d \n",
                sub_cmd, set_value);
            break;
        }
    }

    return ret;
}

/* set param sub-ioctls */
static int iw_setchar_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{
    int sub_cmd = wrqu->data.flags;
    int ret = 0; /* sucess */

    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    
    switch(sub_cmd) 
    {
       case WE_WOWL_ADD_PTRN:
          VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "ADD_PTRN\n");
          hdd_add_wowl_ptrn((char*)wrqu->data.pointer);
          break;
       case WE_WOWL_DEL_PTRN:
          VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "DEL_PTRN\n");
          hdd_del_wowl_ptrn((char*)wrqu->data.pointer);
          break;
       default:  
       {
           hddLog(LOGE, "%s: Invalid sub command %d\n",__FUNCTION__, sub_cmd);
           ret = -EINVAL;
           break;
       }
    }
    return ret;
}

/* get param sub-ioctls */
static int iw_setnone_getint(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{  
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    int *value = (int *)extra;
    int ret = 0; /* sucess */
    

    switch (value[0])
    {
        case WE_GET_11D_STATE:
        {
           tCsrConfigParam configInfo;
           
           sme_GetConfigParam(pAdapter->hHal,&configInfo);
           
           *value = configInfo.Is11dSupportEnabled;

            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("11D state=%ld!!\n"),*value);
           
           break;
        }

        case WE_IBSS_STATUS:
           VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "****Return IBSS Status*****\n");
           break;

        case WE_PMC_STATE:
        {
             *value = pmcGetPmcState(pAdapter->hHal);
             VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, ("PMC state=%ld!!\n"),*value);
             break;            
        }
        case WE_GET_WLAN_DBG:
        {
           vos_trace_display();
           *value = 0;
           break;            
        }         
        case WE_MODULE_DOWN_IND:
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,"%s: sending WLAN_MODULE_DOWN_IND", __FUNCTION__);
            send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);
            *value = 0;
            break;
        }
        default:
        {
            hddLog(LOGE, "Invalid IOCTL get_value command %d ",value[0]);
            break;
        }
    }

    return ret;
}

/* set param sub-ioctls */
static int iw_set_three_ints_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{    
    int *value = (int *)extra;
    int sub_cmd = value[0];
    
    
    switch(sub_cmd)
    {
        case WE_SET_WLAN_DBG:
        {
            vos_trace_setValue( value[1], value[2], value[3]);
            break;
        }
           
        default:  
        {
            hddLog(LOGE, "Invalid IOCTL command %d  \n",  sub_cmd );
            break;
        }
    }

    return 0;
}

static int iw_get_char_setnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{    
    int sub_cmd = wrqu->data.flags;
    VOS_STATUS status;
    FwVersionInfo fwversion;    
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    v_U32_t reg_val;

    switch(sub_cmd)
    {
        case WE_WLAN_VERSION:
        {
            char *buf = extra;
               
            buf += sprintf(buf,"%s_",WLAN_CHIP_VERSION);
            /*Read the RevID*/
            status = sme_DbgReadRegister(pAdapter->hHal,QWLAN_RFAPB_REV_ID_REG,&reg_val);

            if ( !VOS_IS_STATUS_SUCCESS( status ) ) {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s Failed!!!\n",__func__);
                return -EINVAL;
            }

            buf += sprintf(buf,"%x.%x-",(v_U8_t)(reg_val >> 8), (v_U8_t)(reg_val & 0x000000FF));
            
            status = sme_GetFwVersion( pAdapter->hHal,&fwversion); 

            if ( !VOS_IS_STATUS_SUCCESS( status ) ) {
                hddLog(VOS_TRACE_LEVEL_ERROR, "%s Failed!!!\n",__func__);
                return -EINVAL;
            }
            buf += sprintf(buf,"%s-", QWLAN_VERSIONSTR);
            buf += sprintf(buf,"%ld.%ld.%ld.%ld",fwversion.uMj,fwversion.uMn,fwversion.uPatch,fwversion.uBuild);            
            wrqu->data.length = strlen(extra);
            break;
        }
           
        case WE_GET_STATS:
        {
            hdd_tx_rx_stats_t *pStats = &pAdapter->hdd_stats.hddTxRxStats;

            snprintf(extra, WE_MAX_STR_LEN,
                     "\nTransmit"
                     "\ncalled %u, dropped %u, backpressured %u, queued %u"
                     "\n      dropped BK %u, BE %u, VI %u, VO %u"
                     "\n   classified BK %u, BE %u, VI %u, VO %u"
                     "\nbackpressured BK %u, BE %u, VI %u, VO %u"
                     "\n       queued BK %u, BE %u, VI %u, VO %u"
                     "\nfetched %u, empty %u, lowres %u, deqerr %u"
                     "\ndequeued %u, depressured %u, completed %u, flushed %u"
                     "\n      fetched BK %u, BE %u, VI %u, VO %u"
                     "\n     dequeued BK %u, BE %u, VI %u, VO %u"
                     "\n  depressured BK %u, BE %u, VI %u, VO %u"
                     "\n      flushed BK %u, BE %u, VI %u, VO %u"
                     "\n\nReceive"
                     "\nchains %u, packets %u, dropped %u, delivered %u, refused %u"
                     "\n",
                     pStats->txXmitCalled,
                     pStats->txXmitDropped,
                     pStats->txXmitBackPressured,
                     pStats->txXmitQueued,

                     pStats->txXmitDroppedAC[WLANTL_AC_BK],
                     pStats->txXmitDroppedAC[WLANTL_AC_BE],
                     pStats->txXmitDroppedAC[WLANTL_AC_VI],
                     pStats->txXmitDroppedAC[WLANTL_AC_VO],

                     pStats->txXmitClassifiedAC[WLANTL_AC_BK],
                     pStats->txXmitClassifiedAC[WLANTL_AC_BE],
                     pStats->txXmitClassifiedAC[WLANTL_AC_VI],
                     pStats->txXmitClassifiedAC[WLANTL_AC_VO],

                     pStats->txXmitBackPressuredAC[WLANTL_AC_BK],
                     pStats->txXmitBackPressuredAC[WLANTL_AC_BE],
                     pStats->txXmitBackPressuredAC[WLANTL_AC_VI],
                     pStats->txXmitBackPressuredAC[WLANTL_AC_VO],

                     pStats->txXmitQueuedAC[WLANTL_AC_BK],
                     pStats->txXmitQueuedAC[WLANTL_AC_BE],
                     pStats->txXmitQueuedAC[WLANTL_AC_VI],
                     pStats->txXmitQueuedAC[WLANTL_AC_VO],

                     pStats->txFetched,
                     pStats->txFetchEmpty,
                     pStats->txFetchLowResources,
                     pStats->txFetchDequeueError,

                     pStats->txFetchDequeued,
                     pStats->txFetchDePressured,
                     pStats->txCompleted,
                     pStats->txFlushed,

                     pStats->txFetchedAC[WLANTL_AC_BK],
                     pStats->txFetchedAC[WLANTL_AC_BE],
                     pStats->txFetchedAC[WLANTL_AC_VI],
                     pStats->txFetchedAC[WLANTL_AC_VO],

                     pStats->txFetchDequeuedAC[WLANTL_AC_BK],
                     pStats->txFetchDequeuedAC[WLANTL_AC_BE],
                     pStats->txFetchDequeuedAC[WLANTL_AC_VI],
                     pStats->txFetchDequeuedAC[WLANTL_AC_VO],

                     pStats->txFetchDePressuredAC[WLANTL_AC_BK],
                     pStats->txFetchDePressuredAC[WLANTL_AC_BE],
                     pStats->txFetchDePressuredAC[WLANTL_AC_VI],
                     pStats->txFetchDePressuredAC[WLANTL_AC_VO],

                     pStats->txFlushedAC[WLANTL_AC_BK],
                     pStats->txFlushedAC[WLANTL_AC_BE],
                     pStats->txFlushedAC[WLANTL_AC_VI],
                     pStats->txFlushedAC[WLANTL_AC_VO],

                     pStats->rxChains,
                     pStats->rxPackets,
                     pStats->rxDropped,
                     pStats->rxDelivered,
                     pStats->rxRefused

                     );
            wrqu->data.length = strlen(extra)+1;
            break;
        }

        case WE_GET_CFG:
        {
            hdd_cfg_get_config(pAdapter, extra, WE_MAX_STR_LEN);
            wrqu->data.length = strlen(extra)+1;
            break;
        }

        default:  
        {
            hddLog(LOGE, "Invalid IOCTL command %d  \n",  sub_cmd );
            break;
        }
    }

    return 0;
}


/*  action sub-ioctls */
static int iw_setnone_getnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra)
{  
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    int sub_cmd = wrqu->data.flags;
    int ret = 0; /* sucess */
    
    switch (sub_cmd)
    {
        case WE_CLEAR_STATS:
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,"%s: clearing", __FUNCTION__);
            memset(&pAdapter->stats, 0, sizeof(pAdapter->stats));
            memset(&pAdapter->hdd_stats, 0, sizeof(pAdapter->hdd_stats));
            break;
        }

        default:
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: unknown ioctl %d", __FUNCTION__, sub_cmd);
            hddLog(LOGE, "Invalid IOCTL action command %d ", sub_cmd);
            break;
        }
    }

    return ret;
}

static int iw_set_var_ints_getnone(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{   
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = pAdapter->hHal;
    int sub_cmd = wrqu->data.flags;
    int *value = (int*)wrqu->data.pointer;
    int log_dump_args[MAX_VAR_ARGS] = {0};

    hddLog(LOGW, "The function iw_set_var_ints_getnone called \n");
    hddLog(LOGW, "%s: Received length %d\n", __FUNCTION__, wrqu->data.length);
    hddLog(LOGW, "%s: Received data %s\n", __FUNCTION__, (char*)wrqu->data.pointer);

    switch (sub_cmd)
    {
        case WE_LOG_DUMP_CMD:
            {
                vos_mem_copy(log_dump_args, value, (sizeof(int))*wrqu->data.length);


                hddLog(LOGE, "%s: PTT_MSG_LOG_DUMP %d arg1 %d arg2 %d arg3 %d arg4 %d\n",
                        __FUNCTION__, log_dump_args[0], log_dump_args[1], log_dump_args[2], 
                        log_dump_args[3], log_dump_args[4]);

                logPrintf(hHal, log_dump_args[0], log_dump_args[1], log_dump_args[2], 
                        log_dump_args[3], log_dump_args[4]);

            }
            break;

        default:  
            {
                hddLog(LOGE, "Invalid IOCTL command %d  \n",  sub_cmd );
                break;
            }
    }

    return 0;
}


static int iw_add_tspec(struct net_device *dev, struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = (netdev_priv(dev));
   hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *)extra;
   int params[HDD_WLAN_WMM_PARAM_COUNT];
   sme_QosWmmTspecInfo tSpec;
   v_U32_t handle;

   // make sure the application is sufficiently priviledged
   // note that the kernel will do this for "set" ioctls, but since
   // this ioctl wants to return status to user space it must be
   // defined as a "get" ioctl
   if (!capable(CAP_NET_ADMIN))
   {
      return -EPERM;
   }

   // we must be associated in order to add a tspec
   if (eConnectionState_Associated != pAdapter->conn_info.connState)
   {
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }

   // since we are defined to be a "get" ioctl, and since the number
   // of params exceeds the number of params that wireless extensions
   // will pass down in the iwreq_data, we must copy the "set" params
   // from user space ourselves
   if (copy_from_user(&params, wrqu->data.pointer, sizeof(params)))
   {
      // hmmm, can't get them
      return -EIO;
   }

   // clear the tspec
   memset(&tSpec, 0, sizeof(tSpec));

   // validate the handle
   handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
   if (HDD_WMM_HANDLE_IMPLICIT == handle)
   {
      // that one is reserved
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }

   // validate the TID
   if (params[HDD_WLAN_WMM_PARAM_TID] > 7)
   {
      // out of range
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }
   tSpec.ts_info.tid = params[HDD_WLAN_WMM_PARAM_TID];

   // validate the direction
   switch (params[HDD_WLAN_WMM_PARAM_DIRECTION])
   {
   case HDD_WLAN_WMM_DIRECTION_UPSTREAM:
      tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_UPLINK;
      break;

   case HDD_WLAN_WMM_DIRECTION_DOWNSTREAM:
      tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_DOWNLINK;
      break;

   case HDD_WLAN_WMM_DIRECTION_BIDIRECTIONAL:
      tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_BOTH;
      break;

   default:
      // unknown
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }

   // validate the user priority
   if (params[HDD_WLAN_WMM_PARAM_USER_PRIORITY] >= SME_QOS_WMM_UP_MAX)
   {
      // out of range
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }
   tSpec.ts_info.up = params[HDD_WLAN_WMM_PARAM_USER_PRIORITY];

   tSpec.nominal_msdu_size = params[HDD_WLAN_WMM_PARAM_NOMINAL_MSDU_SIZE];
   tSpec.maximum_msdu_size = params[HDD_WLAN_WMM_PARAM_MAXIMUM_MSDU_SIZE];
   tSpec.min_data_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_DATA_RATE];
   tSpec.mean_data_rate = params[HDD_WLAN_WMM_PARAM_MEAN_DATA_RATE];
   tSpec.peak_data_rate = params[HDD_WLAN_WMM_PARAM_PEAK_DATA_RATE];
   tSpec.max_burst_size = params[HDD_WLAN_WMM_PARAM_MAX_BURST_SIZE];
   tSpec.min_phy_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_PHY_RATE];
   tSpec.surplus_bw_allowance = params[HDD_WLAN_WMM_PARAM_SURPLUS_BANDWIDTH_ALLOWANCE];
   tSpec.min_service_interval = params[HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL];
   tSpec.max_service_interval = params[HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL];
   tSpec.suspension_interval = params[HDD_WLAN_WMM_PARAM_SUSPENSION_INTERVAL];

   *pStatus = hdd_wmm_addts(pAdapter, handle, &tSpec);
   return 0;
}


static int iw_del_tspec(struct net_device *dev, struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = (netdev_priv(dev));
   int *params = (int *)extra;
   hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *)extra;
   v_U32_t handle;

   // make sure the application is sufficiently priviledged
   // note that the kernel will do this for "set" ioctls, but since
   // this ioctl wants to return status to user space it must be
   // defined as a "get" ioctl
   if (!capable(CAP_NET_ADMIN))
   {
      return -EPERM;
   }

   // although we are defined to be a "get" ioctl, the params we require
   // will fit in the iwreq_data, therefore unlike iw_add_tspec() there
   // is no need to copy the params from user space

   // validate the handle
   handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
   if (HDD_WMM_HANDLE_IMPLICIT == handle)
   {
      // that one is reserved
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }

   *pStatus = hdd_wmm_delts(pAdapter, handle);
   return 0;
}


static int iw_get_tspec(struct net_device *dev, struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = (netdev_priv(dev));
   int *params = (int *)extra;
   hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *)extra;
   v_U32_t handle;

   // although we are defined to be a "get" ioctl, the params we require
   // will fit in the iwreq_data, therefore unlike iw_add_tspec() there
   // is no need to copy the params from user space

   // validate the handle
   handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
   if (HDD_WMM_HANDLE_IMPLICIT == handle)
   {
      // that one is reserved
      *pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
      return 0;
   }

   *pStatus = hdd_wmm_checkts(pAdapter, handle);
   return 0;
}


#ifdef FEATURE_WLAN_WAPI
static int iw_qcom_set_wapi_mode(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;

    WAPI_FUNCTION_MODE *pWapiMode = (WAPI_FUNCTION_MODE *)wrqu->data.pointer;

    hddLog(LOG1, "The function iw_qcom_set_wapi_mode called");
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    hddLog(LOG1, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    hddLog(LOG1, "%s: Input Data (wreq) WAPI Mode:%02d", __FUNCTION__, pWapiMode->wapiMode);
   

    if(WZC_ORIGINAL == pWapiMode->wapiMode) {
        hddLog(LOG1, "%s: WAPI Mode Set to OFF", __FUNCTION__);
         /* Set Encryption mode to defualt , this allows next successfull non-WAPI Association */
        pRoamProfile->EncryptionType.numEntries = 1;
        pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
        pRoamProfile->mcEncryptionType.numEntries = 1;
        pRoamProfile->mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;

        pRoamProfile->AuthType.numEntries = 1;
        pAdapter->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
        pRoamProfile->AuthType.authType[0] = pAdapter->conn_info.authType;
    }
    else if(WAPI_EXTENTION == pWapiMode->wapiMode) {
        hddLog(LOG1, "%s: WAPI Mode Set to ON", __FUNCTION__);
    }
    else
         return -EINVAL;

    pAdapter->wapi_info.nWapiMode = pWapiMode->wapiMode;

    return 0;
}

static int iw_qcom_get_wapi_mode(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    WAPI_FUNCTION_MODE *pWapiMode = (WAPI_FUNCTION_MODE *)(extra);

    hddLog(LOG1, "The function iw_qcom_get_wapi_mode called");

    pWapiMode->wapiMode = pAdapter->wapi_info.nWapiMode;
    hddLog(LOG1, "%s: GET WAPI Mode Value:%02d", __FUNCTION__, pWapiMode->wapiMode);
    printk("\nGET WAPI MODE:%d",pWapiMode->wapiMode);
    return 0;
}

static int iw_qcom_set_wapi_assoc_info(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
//    WAPI_AssocInfo *pWapiAssocInfo = (WAPI_AssocInfo *)(wrqu->data.pointer);
    WAPI_AssocInfo *pWapiAssocInfo = (WAPI_AssocInfo *)(extra);
    int i = 0, j = 0;
    hddLog(LOG1, "The function iw_qcom_set_wapi_assoc_info called");
    hddLog(LOG1, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)extra);

    VOS_ASSERT(pWapiAssocInfo);

    hddLog(LOG1, "%s: INPUT DATA:\nElement ID:0x%02x Length:0x%02x Version:0x%04x\n",__FUNCTION__,pWapiAssocInfo->elementID,pWapiAssocInfo->length,pWapiAssocInfo->version);
    hddLog(LOG1,"%s: akm Suite Cnt:0x%04x",__FUNCTION__,pWapiAssocInfo->akmSuiteCount);
    for(i =0 ; i < 16 ; i++)
        hddLog(LOG1,"akm suite[%02d]:0x%08lx",i,pWapiAssocInfo->akmSuite[i]);

    hddLog(LOG1,"%s: Unicast Suite Cnt:0x%04x",__FUNCTION__,pWapiAssocInfo->unicastSuiteCount);
    for(i =0 ; i < 16 ; i++)
        hddLog(LOG1, "Unicast suite[%02d]:0x%08lx",i,pWapiAssocInfo->unicastSuite[i]);

    hddLog(LOG1,"%s: Multicast suite:0x%08lx Wapi capa:0x%04x",__FUNCTION__,pWapiAssocInfo->multicastSuite,pWapiAssocInfo->wapiCability);
    hddLog(LOG1, "%s: BKID Cnt:0x%04x\n",__FUNCTION__,pWapiAssocInfo->bkidCount);
    for(i = 0 ; i < 16 ; i++) {
        hddLog(LOG1, "BKID List[%02d].bkid:0x",i);
        for(j = 0 ; j < 16 ; j++)
            hddLog(LOG1,"%02x",pWapiAssocInfo->bkidList[i].bkid[j]);
    }

    /* We are not using the entire IE as provided by the supplicant.
     * This is being calculated by SME. This is the same as in the
     * case of WPA. Only the auth mode information needs to be
     * extracted here*/
    if ( pWapiAssocInfo->akmSuite[0] == WAPI_PSK_AKM_SUITE ) {
       hddLog(LOG1, "%s: WAPI AUTH MODE SET TO PSK",__FUNCTION__);
       pAdapter->wapi_info.wapiAuthMode = WAPI_AUTH_MODE_PSK;
    }

    if ( pWapiAssocInfo->akmSuite[0] == WAPI_CERT_AKM_SUITE) {
       hddLog(LOG1, "%s: WAPI AUTH MODE SET TO CERTIFICATE",__FUNCTION__);
       pAdapter->wapi_info.wapiAuthMode = WAPI_AUTH_MODE_CERT;
    }
    return 0;
}

static int iw_qcom_set_wapi_key(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t   *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    eHalStatus       halStatus   = eHAL_STATUS_SUCCESS;
    tANI_U32         roamId      = 0xFF;
    tANI_U8         *pKeyPtr     = NULL;
    v_BOOL_t         isConnected = TRUE;
    tCsrRoamSetKey   setKey;
    int i = 0;

//    WLAN_WAPI_KEY *pWapiKey = (WLAN_WAPI_KEY *)(wrqu->data.pointer);
    WLAN_WAPI_KEY *pWapiKey = (WLAN_WAPI_KEY *)(extra);

    hddLog(LOG1, "The function iw_qcom_set_wapi_key called ");
    hddLog(LOG1, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)extra);

    hddLog(LOG1,":s: INPUT DATA:\nKey Type:0x%02x Key Direction:0x%02x KEY ID:0x%02x\n", __FUNCTION__,pWapiKey->keyType,pWapiKey->keyDirection,pWapiKey->keyId);
    hddLog(LOG1,"Add Index:0x");
    for(i =0 ; i < 12 ; i++)
        hddLog(LOG1,"%02x",pWapiKey->addrIndex[i]);

    hddLog(LOG1,"\n%s: WAPI ENCRYPTION KEY LENGTH:0x%04x", __FUNCTION__,pWapiKey->wpiekLen);
    hddLog(LOG1, "WAPI ENCRYPTION KEY:0x");
    for(i =0 ; i < 16 ; i++)
        hddLog(LOG1,"%02x",pWapiKey->wpiek[i]);

    hddLog(LOG1,"\n%s: WAPI INTEGRITY CHECK KEY LENGTH:0x%04x", __FUNCTION__,pWapiKey->wpickLen);
    hddLog(LOG1,"WAPI INTEGRITY CHECK KEY:0x");
    for(i =0 ; i < 16 ; i++)
        hddLog(LOG1,"%02x",pWapiKey->wpick[i]);

    hddLog(LOG1,"\nWAPI PN NUMBER:0x");
    for(i = 0 ; i < 16 ; i++)
        hddLog(LOG1,"%02x",pWapiKey->pn[i]);

    // Clear the setkey memory
    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
    // Store Key ID
    setKey.keyId = (unsigned char)( pWapiKey->keyId );
    // SET WAPI Encryption
    setKey.encType  = eCSR_ENCRYPT_TYPE_WPI;
    // Key Directionn both TX and RX
    setKey.keyDirection = eSIR_TX_RX; // Do WE NEED to update this based on Key Type as GRP/UNICAST??
    // the PAE role
    setKey.paeRole = 0 ;

    switch ( pWapiKey->keyType )
    {
        case HDD_PAIRWISE_WAPI_KEY:
        {
            isConnected = hdd_connIsConnected(pAdapter);
            vos_mem_copy(setKey.peerMac,&pAdapter->conn_info.bssId,WNI_CFG_BSSID_LEN);
            break;
        }
        case HDD_GROUP_WAPI_KEY:
        {
            vos_set_macaddr_broadcast( (v_MACADDR_t *)setKey.peerMac );
            break;
        }
        default:
        {
            //Any other option is invalid.
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "[%4d] %s() failed to Set Key. Invalid key type %d", __LINE__,__FUNCTION__ , -1 );

            hddLog(LOGE," %s: Error WAPI Key Add Type",__FUNCTION__);
            halStatus = !eHAL_STATUS_SUCCESS; // NEED TO UPDATE THIS WITH CORRECT VALUE
            break; // NEED RETURN FROM HERE ????
        }
    }

    // Concatenating the Encryption Key (EK) and the MIC key (CK): EK followed by CK
    setKey.keyLength = (v_U16_t)((pWapiKey->wpiekLen)+(pWapiKey->wpickLen));
    pKeyPtr = setKey.Key;
    memcpy( pKeyPtr, pWapiKey->wpiek, pWapiKey->wpiekLen );
    pKeyPtr += pWapiKey->wpiekLen;
    memcpy( pKeyPtr, pWapiKey->wpick, pWapiKey->wpickLen );

    // Set the new key with SME.
    pAdapter->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;

    if ( isConnected ) {
        halStatus = sme_RoamSetKey( pAdapter->hHal, &setKey, &roamId );
        if ( halStatus != eHAL_STATUS_SUCCESS )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "[%4d] sme_RoamSetKey returned ERROR status= %d", __LINE__, halStatus );

            pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
        }
    }
#if 0 /// NEED TO CHECK ON THIS
    else
    {
        // Store the keys in the adapter to be moved to the profile & passed to
        // SME in the ConnectRequest if we are not yet in connected state.
         memcpy( &pAdapter->setKey[ setKey.keyId ], &setKey, sizeof( setKey ) );
         pAdapter->fKeySet[ setKey.keyId ] = TRUE;

         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                    "  Saving key [idx= %d] to apply when moving to connected state ",
                    setKey.keyId );

    }
#endif
    return halStatus;
}

static int iw_qcom_set_wapi_bkid(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    int i = 0;
    WLAN_BKID_LIST  *pBkid       = ( WLAN_BKID_LIST *) (wrqu->data.pointer);
//    WLAN_BKID_LIST  *pBkid       = ( WLAN_BKID_LIST *) (extra);

    hddLog(LOG1, "The function iw_qcom_set_wapi_bkid called");
    hddLog(LOG1, "%s: Received length %d", __FUNCTION__, wrqu->data.length);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)wrqu->data.pointer);
    hddLog(LOG1, "%s: Received data %s", __FUNCTION__, (char*)extra);

    hddLog(LOG1,"%s: INPUT DATA:\n BKID Length:0x%08lx\n", __FUNCTION__,pBkid->length);
    hddLog(LOG1,"%s: BKID Cnt:0x%04lx",pBkid->BKIDCount);

    hddLog(LOG1,"BKID KEY LIST[0]:0x");
    for(i =0 ; i < 16 ; i++)
        hddLog(LOG1,"%02x",pBkid->BKID[0].bkid[i]);

    return 0;
}

static int iw_qcom_get_wapi_bkid(struct net_device *dev, struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    /* Yet to implement this function, 19th April 2010 */
    hddLog(LOG1, "The function iw_qcom_get_wapi_bkid called ");

    return 0;
}
#endif /* FEATURE_WLAN_WAPI */

// Define the Wireless Extensions to the Linux Network Device structure
// A number of these routines are NULL (meaning they are not implemented.) 

static const iw_handler      we_handler[] =
{
   (iw_handler) iw_set_commit,      /* SIOCSIWCOMMIT */
   (iw_handler) iw_get_name,      /* SIOCGIWNAME */
   (iw_handler) NULL,            /* SIOCSIWNWID */
   (iw_handler) NULL,            /* SIOCGIWNWID */
   (iw_handler) iw_set_freq,      /* SIOCSIWFREQ */
   (iw_handler) iw_get_freq,      /* SIOCGIWFREQ */
   (iw_handler) iw_set_mode,      /* SIOCSIWMODE */
   (iw_handler) iw_get_mode,      /* SIOCGIWMODE */
   (iw_handler) NULL,              /* SIOCSIWSENS */
   (iw_handler) NULL,              /* SIOCGIWSENS */
   (iw_handler) NULL,             /* SIOCSIWRANGE */
   (iw_handler) iw_get_range,      /* SIOCGIWRANGE */
   (iw_handler) iw_set_priv,       /* SIOCSIWPRIV */
   (iw_handler) NULL,             /* SIOCGIWPRIV */
   (iw_handler) NULL,             /* SIOCSIWSTATS */
   (iw_handler) NULL,             /* SIOCGIWSTATS */
   iw_handler_set_spy,             /* SIOCSIWSPY */
   iw_handler_get_spy,             /* SIOCGIWSPY */
   iw_handler_set_thrspy,         /* SIOCSIWTHRSPY */
   iw_handler_get_thrspy,         /* SIOCGIWTHRSPY */
   (iw_handler) iw_set_ap_address,   /* SIOCSIWAP */
   (iw_handler) iw_get_ap_address,   /* SIOCGIWAP */
   (iw_handler) iw_set_mlme,              /* SIOCSIWMLME */
   (iw_handler) NULL,              /* SIOCGIWAPLIST */
   (iw_handler) iw_set_scan,      /* SIOCSIWSCAN */
   (iw_handler) iw_get_scan,      /* SIOCGIWSCAN */
   (iw_handler) iw_set_essid,      /* SIOCSIWESSID */
   (iw_handler) iw_get_essid,      /* SIOCGIWESSID */
   (iw_handler) iw_set_nick,      /* SIOCSIWNICKN */
   (iw_handler) iw_get_nick,      /* SIOCGIWNICKN */
   (iw_handler) NULL,             /* -- hole -- */
   (iw_handler) NULL,             /* -- hole -- */
   (iw_handler) iw_set_bitrate,   /* SIOCSIWRATE */
   (iw_handler) iw_get_bitrate,   /* SIOCGIWRATE */
   (iw_handler) iw_set_rts_threshold,/* SIOCSIWRTS */
   (iw_handler) iw_get_rts_threshold,/* SIOCGIWRTS */
   (iw_handler) iw_set_frag_threshold,   /* SIOCSIWFRAG */
   (iw_handler) iw_get_frag_threshold,   /* SIOCGIWFRAG */
   (iw_handler) iw_set_tx_power,      /* SIOCSIWTXPOW */
   (iw_handler) iw_get_tx_power,      /* SIOCGIWTXPOW */
   (iw_handler) iw_set_retry,          /* SIOCSIWRETRY */
   (iw_handler) iw_get_retry,          /* SIOCGIWRETRY */
   (iw_handler) iw_set_encode,          /* SIOCSIWENCODE */
   (iw_handler) iw_get_encode,          /* SIOCGIWENCODE */
   (iw_handler) iw_set_power_mode,      /* SIOCSIWPOWER */
   (iw_handler) iw_get_power_mode,      /* SIOCGIWPOWER */
   (iw_handler) NULL,                 /* -- hole -- */
   (iw_handler) NULL,                /* -- hole -- */
   (iw_handler) iw_set_genie,      /* SIOCSIWGENIE */
   (iw_handler) iw_get_genie,      /* SIOCGIWGENIE */
   (iw_handler) iw_set_auth,      /* SIOCSIWAUTH */
   (iw_handler) iw_get_auth,      /* SIOCGIWAUTH */
   (iw_handler) iw_set_encodeext,   /* SIOCSIWENCODEEXT */
   (iw_handler) iw_get_encodeext,   /* SIOCGIWENCODEEXT */
   (iw_handler) NULL,         /* SIOCSIWPMKSA */
};

static const iw_handler we_private[] = {
   
   [WLAN_PRIV_SET_INT_GET_NONE      - SIOCIWFIRSTPRIV]   = iw_setint_getnone,  //set priv ioctl
   [WLAN_PRIV_SET_NONE_GET_INT      - SIOCIWFIRSTPRIV]   = iw_setnone_getint,  //get priv ioctl   
   [WLAN_PRIV_SET_CHAR_GET_NONE     - SIOCIWFIRSTPRIV]   = iw_setchar_getnone, //get priv ioctl   
   [WLAN_PRIV_SET_THREE_INT_GET_NONE - SIOCIWFIRSTPRIV]  = iw_set_three_ints_getnone,   
   [WLAN_PRIV_GET_CHAR_SET_NONE      - SIOCIWFIRSTPRIV]  = iw_get_char_setnone,
   [WLAN_PRIV_SET_NONE_GET_NONE     - SIOCIWFIRSTPRIV]   = iw_setnone_getnone, //action priv ioctl   
   [WLAN_PRIV_SET_VAR_INT_GET_NONE	- SIOCIWFIRSTPRIV]	 = iw_set_var_ints_getnone,
   [WLAN_PRIV_ADD_TSPEC	            - SIOCIWFIRSTPRIV]   = iw_add_tspec,
   [WLAN_PRIV_DEL_TSPEC	            - SIOCIWFIRSTPRIV]   = iw_del_tspec,
   [WLAN_PRIV_GET_TSPEC	            - SIOCIWFIRSTPRIV]   = iw_get_tspec,
#ifdef FEATURE_WLAN_WAPI
   [WLAN_PRIV_SET_WAPI_MODE             - SIOCIWFIRSTPRIV]  = iw_qcom_set_wapi_mode,
   [WLAN_PRIV_GET_WAPI_MODE             - SIOCIWFIRSTPRIV]  = iw_qcom_get_wapi_mode,
   [WLAN_PRIV_SET_WAPI_ASSOC_INFO       - SIOCIWFIRSTPRIV]  = iw_qcom_set_wapi_assoc_info,
   [WLAN_PRIV_SET_WAPI_KEY              - SIOCIWFIRSTPRIV]  = iw_qcom_set_wapi_key,
   [WLAN_PRIV_SET_WAPI_BKID             - SIOCIWFIRSTPRIV]  = iw_qcom_set_wapi_bkid,
   [WLAN_PRIV_GET_WAPI_BKID             - SIOCIWFIRSTPRIV]  = iw_qcom_get_wapi_bkid,
#endif /* FEATURE_WLAN_WAPI */
};

/*Maximum command length can be only 15 */
static const struct iw_priv_args we_private_args[] = {

    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_INT_GET_NONE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_SET_11D_STATE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "set11Dstate" },    

    {   WE_WOWL,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "wowl" },    

    {   WE_SET_POWER,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0, 
        "setPower" },    

    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_NONE_GET_INT,
        0, 
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "" },
        
    /* handlers for sub-ioctl */
    {   WE_GET_11D_STATE,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 
        "get11Dstate" }, 

    {   WE_IBSS_STATUS,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 
        "getAdhocStatus" },    

    {   WE_PMC_STATE,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "pmcState" },
        
    {   WE_GET_WLAN_DBG,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "getwlandbg" },    

    {   WE_MODULE_DOWN_IND,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "moduleDownInd" },

    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_CHAR_GET_NONE,
        IW_PRIV_TYPE_CHAR| 512,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_WOWL_ADD_PTRN,
        IW_PRIV_TYPE_CHAR| 512,
        0, 
        "wowlAddPtrn" },

    {   WE_WOWL_DEL_PTRN,
        IW_PRIV_TYPE_CHAR| 512,
        0, 
        "wowlDelPtrn" },
        
    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_THREE_INT_GET_NONE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_SET_WLAN_DBG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
        0, 
        "setwlandbg" },

            /* handlers for main ioctl */
    {   WLAN_PRIV_GET_CHAR_SET_NONE,
        0,
        IW_PRIV_TYPE_CHAR| WE_MAX_STR_LEN,
        "" },

            /* handlers for sub-ioctl */
    {   WE_WLAN_VERSION,
        0,
        IW_PRIV_TYPE_CHAR| WE_MAX_STR_LEN,
        "version" },
    {   WE_GET_STATS,
        0,
        IW_PRIV_TYPE_CHAR| WE_MAX_STR_LEN,
        "getStats" },
    {   WE_GET_CFG,
        0,
        IW_PRIV_TYPE_CHAR| WE_MAX_STR_LEN,
        "getConfig" },

    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_NONE_GET_NONE,
        0,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_CLEAR_STATS,
        0,
        0, 
        "clearStats" },
		
    /* handlers for main ioctl */
    {   WLAN_PRIV_SET_VAR_INT_GET_NONE,
        IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
        0, 
        "" },

    /* handlers for sub-ioctl */
    {   WE_LOG_DUMP_CMD,
        IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
        0, 
        "dump" },

    /* handlers for main ioctl */
    {   WLAN_PRIV_ADD_TSPEC,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | HDD_WLAN_WMM_PARAM_COUNT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "addTspec" },

    /* handlers for main ioctl */
    {   WLAN_PRIV_DEL_TSPEC,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "delTspec" },

    /* handlers for main ioctl */
    {   WLAN_PRIV_GET_TSPEC,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "getTspec" },

#ifdef FEATURE_WLAN_WAPI
   /* handlers for main ioctl SET_WAPI_MODE */
    {   WLAN_PRIV_SET_WAPI_MODE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        0,
        "SET_WAPI_MODE" },

   /* handlers for main ioctl GET_WAPI_MODE */
    {   WLAN_PRIV_GET_WAPI_MODE,
        0,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
        "GET_WAPI_MODE" },

   /* handlers for main ioctl SET_ASSOC_INFO */
    {   WLAN_PRIV_SET_WAPI_ASSOC_INFO,
        IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 400,
        0,
        "SET_WAPI_ASSOC" },

   /* handlers for main ioctl SET_WAPI_KEY */
    {   WLAN_PRIV_SET_WAPI_KEY,
        IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 71,
        0,
        "SET_WAPI_KEY" },

   /* handlers for main ioctl SET_WAPI_BKID */
    {   WLAN_PRIV_SET_WAPI_BKID,
        IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 24,
        0,
        "SET_WAPI_BKID" },

   /* handlers for main ioctl GET_WAPI_BKID */
    {   WLAN_PRIV_GET_WAPI_BKID,
        0,
        IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 24,
        "GET_WAPI_BKID" },
#endif /* FEATURE_WLAN_WAPI */
};



const struct iw_handler_def we_handler_def = {
   .num_standard     = sizeof(we_handler) / sizeof(we_handler[0]),
   .num_private      = sizeof(we_private) / sizeof(we_private[0]),
   .num_private_args = sizeof(we_private_args) / sizeof(we_private_args[0]),

   .standard         = (iw_handler *)we_handler,
   .private          = (iw_handler *)we_private,
   .private_args     = we_private_args,
   .get_wireless_stats = get_wireless_stats,
};

int hdd_set_wext(hdd_adapter_t *pAdapter) 
{
    hdd_wext_state_t *pwextBuf;

    pwextBuf = pAdapter->pWextState;
   
    // Now configure the roaming profile links. To SSID and bssid.
    pwextBuf->roamProfile.SSIDs.numOfSSIDs = 0;  
    pwextBuf->roamProfile.SSIDs.SSIDList = &pAdapter->conn_info.SSID;  
  
    pwextBuf->roamProfile.BSSIDs.numOfBSSIDs = 0;  
    pwextBuf->roamProfile.BSSIDs.bssid = &pAdapter->conn_info.bssId; 

    /*Set the numOfChannels to zero to scan all the channels*/
    pwextBuf->roamProfile.ChannelInfo.numOfChannels = 0;
    pwextBuf->roamProfile.ChannelInfo.ChannelList = NULL;
   
    /* Default is no encryption */
    pwextBuf->roamProfile.EncryptionType.numEntries = 1; 
    pwextBuf->roamProfile.EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;

    pwextBuf->roamProfile.mcEncryptionType.numEntries = 1;
    pwextBuf->roamProfile.mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;

    pwextBuf->roamProfile.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;

    /* Default is no authentication */
    pwextBuf->roamProfile.AuthType.numEntries = 1;
    pwextBuf->roamProfile.AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;

    pwextBuf->roamProfile.phyMode = eCSR_DOT11_MODE_TAURUS;
    pwextBuf->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

    /*Set the default scan mode*/
    pwextBuf->scan_mode = eSIR_ACTIVE_SCAN;

    return VOS_STATUS_SUCCESS;
 
    }

int hdd_register_wext(struct net_device *dev)
    {
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pwextBuf;
    VOS_STATUS status;

   ENTER();
    
    // Allocate the Wireless Extensions state structure   
   pwextBuf = kmalloc(sizeof(hdd_wext_state_t), GFP_KERNEL);

   if( !pwextBuf ) {
      hddLog( LOG1,"VOS unable to allocate memory\n");
      return VOS_STATUS_E_FAILURE;
    }
    // Zero the memory.  This zeros the profile structure.
   memset(pwextBuf, 0,sizeof(hdd_wext_state_t));
   
    // Set up the pointer to the Wireless Extensions state structure
    pAdapter->pWextState = pwextBuf;

    status = hdd_set_wext(pAdapter);

    if(!VOS_IS_STATUS_SUCCESS(status)) {

        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: hdd_set_wext failed!!\n"));
        return eHAL_STATUS_FAILURE;
    }

    if (!VOS_IS_STATUS_SUCCESS(vos_event_init(&pwextBuf->vosevent)))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos event init failed!!\n"));
        return eHAL_STATUS_FAILURE;
    }
    // Register as a wireless device
    dev->wireless_handlers = (struct iw_handler_def *)&we_handler_def;
   
    EXIT();
    return 0;
}
    
int hdd_UnregisterWext(struct net_device *dev)
{
   hdd_wext_state_t *wextBuf;
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   ENTER();
   // Set up the pointer to the Wireless Extensions state structure
   wextBuf = pAdapter->pWextState;
   
   // De-allocate the Wireless Extensions state structure
   kfree(wextBuf);
      
   // Clear out the pointer to the Wireless Extensions state structure
   pAdapter->pWextState = NULL;

   EXIT();
   return 0;
}
    

