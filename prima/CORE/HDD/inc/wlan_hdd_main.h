#if !defined( WLAN_HDD_MAIN_H )
#define WLAN_HDD_MAIN_H
/**===========================================================================
  
  \file  WLAN_HDD_MAIN_H.h
  
  \brief Linux HDD Adapter Type
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
  
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#ifdef CONFIG_CFG80211
#include <net/cfg80211.h>
#endif
#include <vos_list.h>
#include <vos_types.h>
#include "sirMacProtDef.h"
#include "csrApi.h"
#include <wlan_hdd_assoc.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_hdd_wmm.h>
#include <wlan_hdd_cfg.h>
#ifdef ANI_MANF_DIAG
#include <wlan_hdd_ftm.h>
#endif
/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 
/** Number of Tx Queues */  
#define NUM_TX_QUEUES 4
/** Queue length specified to OS in the net_device */
#define NET_DEV_TX_QUEUE_LEN 100
/** HDD's internal Tx Queue Length. Needs to be a power of 2 */
#define HDD_TX_QUEUE_MAX_LEN 128
/** HDD internal Tx Queue Low Watermark. Net Device TX queue is disabled
 *  when HDD queue becomes full. This Low watermark is used to enable
 *  the Net Device queue again */
#define HDD_TX_QUEUE_LOW_WATER_MARK (HDD_TX_QUEUE_MAX_LEN*3/4)
/** Bytes to reserve in the headroom */
#define LIBRA_HW_NEEDED_HEADROOM   128
/** Hdd Tx Time out value */
#ifdef LIBRA_LINUX_PC
#define HDD_TX_TIMEOUT          (8000)       
#else
#define HDD_TX_TIMEOUT          (2*HZ)    
#endif
/** Hdd Default MTU */
#define HDD_DEFAULT_MTU         (1500)
/**event flags registered net device*/
#define NET_DEVICE_REGISTERED  1<<0
#define SME_SESSION_OPENED     1<<1
#define INIT_TX_RX_SUCCESS     1<<2
#define WMM_INIT_DONE          1<<3
#define SOFTAP_BSS_STARTED     1<<4

/** Maximum time(ms)to wait for disconnect to complete **/
#define WLAN_WAIT_TIME_DISCONNECT  100
#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
/** Mac Address string **/
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAX_GENIE_LEN 255

#if defined(QC_WLAN_CHIPSET_PRIMA)
#define WLAN_CHIP_VERSION   "WCNSS"
#elif defined(ANI_CHIPSET_LIBRA)
#define WLAN_CHIP_VERSION   "WCN1312"
#elif defined(ANI_CHIPSET_VOLANS)
#define WLAN_CHIP_VERSION   "WCN1314"
#else
#define WLAN_CHIP_VERSION   "UNKNOWN"
#endif

#define hddLog(level, args...) VOS_TRACE( VOS_MODULE_ID_HDD, level, ## args)
#define ENTER() VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Enter:%s\n", __FUNCTION__)
#define EXIT()  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Exit:%s\n", __FUNCTION__)

#define WLAN_HDD_GET_PRIV_PTR(__dev__) (hdd_adapter_t*)(netdev_priv((__dev__)))
typedef struct hdd_tx_rx_stats_s
{
   // start_xmit stats
   __u32    txXmitCalled;
   __u32    txXmitDropped;
   __u32    txXmitBackPressured;
   __u32    txXmitQueued;
   __u32    txXmitClassifiedAC[NUM_TX_QUEUES];
   __u32    txXmitDroppedAC[NUM_TX_QUEUES];
   __u32    txXmitBackPressuredAC[NUM_TX_QUEUES];
   __u32    txXmitQueuedAC[NUM_TX_QUEUES];
   // fetch_cbk stats
   __u32    txFetched;
   __u32    txFetchedAC[NUM_TX_QUEUES];
   __u32    txFetchEmpty;
   __u32    txFetchLowResources;
   __u32    txFetchDequeueError;
   __u32    txFetchDequeued;
   __u32    txFetchDequeuedAC[NUM_TX_QUEUES];
   __u32    txFetchDePressured;
   __u32    txFetchDePressuredAC[NUM_TX_QUEUES];
   // complete_cbk_stats
   __u32    txCompleted;
   // flush stats
   __u32    txFlushed;
   __u32    txFlushedAC[NUM_TX_QUEUES];
   // rx stats
   __u32    rxChains;
   __u32    rxPackets;
   __u32    rxDropped;
   __u32    rxDelivered;
   __u32    rxRefused;
} hdd_tx_rx_stats_t;
typedef struct hdd_stats_s
{
   tCsrSummaryStatsInfo       summary_stat;
   tCsrGlobalClassAStatsInfo  ClassA_stat;
   tCsrGlobalClassBStatsInfo  ClassB_stat;
   tCsrGlobalClassCStatsInfo  ClassC_stat;
   tCsrGlobalClassDStatsInfo  ClassD_stat;
   tCsrPerStaStatsInfo        perStaStats;
   hdd_tx_rx_stats_t          hddTxRxStats;
} hdd_stats_t;

typedef enum
{
   HDD_ROAM_STATE_NONE,
   
   // Issuing a disconnect due to transition into low power states.  
   HDD_ROAM_STATE_DISCONNECTING_POWER,
   
   // move to this state when HDD sets a key with SME/CSR.  Note this is
   // an important state to get right because we will get calls into our SME
   // callback routine for SetKey activity that we did not initiate!
   HDD_ROAM_STATE_SETTING_KEY,
} HDD_ROAM_STATE;

typedef enum
{
   eHDD_SUSPEND_NONE = 0,
   eHDD_SUSPEND_DEEP_SLEEP,
   eHDD_SUSPEND_STANDBY,
} hdd_ps_state_t;

typedef struct roaming_info_s
{
   HDD_ROAM_STATE roamingState;
   vos_event_t roamingEvent;
   
} roaming_info_t;

#ifdef FEATURE_WLAN_WAPI
/* Define WAPI macros for Length, BKID count etc*/
#define MAX_WPI_KEY_LENGTH    16
#define MAX_NUM_PN            16
#define MAC_ADDR_LEN           6
#define MAX_ADDR_INDEX        12
#define MAX_NUM_AKM_SUITES    16
#define MAX_NUM_UNI_SUITES    16
#define MAX_NUM_BKIDS         16
#define HDD_PAIRWISE_WAPI_KEY 0
#define HDD_GROUP_WAPI_KEY    1

/** WAPI AUTH mode definition */
enum _WAPIAuthMode
{
   WAPI_AUTH_MODE_PSK = 1,
   WAPI_AUTH_MODE_CERT
} __attribute__((packed));
typedef enum _WAPIAuthMode WAPIAuthMode;

/** WAPI Work mode structure definition */
#define   WZC_ORIGINAL      0
#define   WAPI_EXTENTION    1

struct _WAPI_FUNCTION_MODE
{
   unsigned char wapiMode;
}__attribute__((packed));

typedef struct _WAPI_FUNCTION_MODE WAPI_FUNCTION_MODE;

typedef struct _WAPI_BKID
{
   v_U8_t   bkid[16];
}WAPI_BKID, *pWAPI_BKID;

/** WAPI Association information structure definition */
struct _WAPI_AssocInfo
{
   v_U8_t      elementID;
   v_U8_t      length;
   v_U16_t     version;
   v_U16_t     akmSuiteCount;
   v_U32_t     akmSuite[MAX_NUM_AKM_SUITES];
   v_U16_t     unicastSuiteCount;
   v_U32_t     unicastSuite[MAX_NUM_UNI_SUITES];
   v_U32_t     multicastSuite;
   v_U16_t     wapiCability;
   v_U16_t     bkidCount;
   WAPI_BKID   bkidList[MAX_NUM_BKIDS];
} __attribute__((packed));

typedef struct _WAPI_AssocInfo WAPI_AssocInfo;
typedef struct _WAPI_AssocInfo *pWAPI_IEAssocInfo;

/** WAPI KEY Type definition */
enum _WAPIKeyType
{
   PAIRWISE_KEY, //0
   GROUP_KEY     //1
}__attribute__((packed));
typedef enum _WAPIKeyType WAPIKeyType;

/** WAPI KEY Direction definition */
enum _KEY_DIRECTION
{
   None,
   Rx,
   Tx,
   Rx_Tx
}__attribute__((packed));

typedef enum _KEY_DIRECTION KEY_DIRECTION;

/** WAPI KEY stucture definition */
struct WLAN_WAPI_KEY
{
   WAPIKeyType     keyType;
   KEY_DIRECTION   keyDirection;  /*reserved for future use*/
   v_U8_t          keyId;
   v_U8_t          addrIndex[MAX_ADDR_INDEX]; /*reserved for future use*/
   int             wpiekLen;
   v_U8_t          wpiek[MAX_WPI_KEY_LENGTH];
   int             wpickLen;
   v_U8_t          wpick[MAX_WPI_KEY_LENGTH];
   v_U8_t          pn[MAX_NUM_PN];        /*reserved for future use*/
}__attribute__((packed));

typedef struct WLAN_WAPI_KEY WLAN_WAPI_KEY;
typedef struct WLAN_WAPI_KEY *pWLAN_WAPI_KEY;

#ifdef CONFIG_CFG80211

typedef struct beacon_data_s {
    u8 *head, *tail;
    int head_len, tail_len;
    int dtim_period;
} beacon_data_t;
#endif


/** WAPI BKID List stucture definition */
struct _WLAN_BKID_LIST
{
   v_U32_t          length;
   v_U32_t          BKIDCount;
   WAPI_BKID        BKID[1];
}__attribute__((packed));

typedef struct _WLAN_BKID_LIST WLAN_BKID_LIST;
typedef struct _WLAN_BKID_LIST *pWLAN_BKID_LIST;


/** WAPI Information stucture definition */
struct hdd_wapi_info_s
{
   v_U32_t     nWapiMode;
   v_BOOL_t    fIsWapiSta;
   v_MACADDR_t cachedMacAddr;
   v_UCHAR_t   wapiAuthMode;
}__attribute__((packed));
typedef struct hdd_wapi_info_s hdd_wapi_info_t;
#endif /* FEATURE_WLAN_WAPI */


typedef enum device_mode
{  /* MAINTAIN 1 - 1 CORRESPONDENCE WITH tVOS_CON_MODE*/
   WLAN_HDD_INFRA_STATION,
   WLAN_HDD_SOFTAP,
   WLAN_HDD_P2P_CLIENT,
   WLAN_HDD_P2P_GO,
   WLAN_HDD_MONITOR
#ifdef ANI_MANF_DIAG
   ,WLAN_HDD_FTM,
#endif
}device_mode_t;

#if defined CONFIG_CFG80211
typedef struct hdd_remain_on_chan_ctx
{
  struct net_device *dev;
  struct ieee80211_channel chan;
  enum nl80211_channel_type chan_type;
  unsigned int duration;
}hdd_remain_on_chan_ctx_t;

typedef struct hdd_cfg80211_state_s 
{
  tANI_U16 current_freq;
  u64 action_cookie;
  tANI_U8 *buf;
  size_t len;
}hdd_cfg80211_state_t;

#endif
struct hdd_station_ctx
{
  /** Handle to the Wireless Extension State */
   hdd_wext_state_t WextState;

   /**Connection information*/
   connection_info_t conn_info;

   roaming_info_t roam_info;

#ifdef CONFIG_CFG80211
   hdd_cfg80211_state_t cfg80211State;
#endif
};

#define BSS_STOP    0 
#define BSS_START   1
typedef struct hdd_hostapd_state_s
{
    int bssState;
    vos_event_t vosEvent;
    VOS_STATUS vosStatus;
    v_BOOL_t bCommit; 

} hdd_hostapd_state_t;


/*
 * Per station structure kept in HDD for multiple station support for SoftAP
*/
typedef struct {
    /** The station entry is used or not  */
    v_BOOL_t isUsed;

    /** Station ID reported back from HAL (through SAP). Broadcast
     *  uses station ID zero by default in both libra and volans. */
    v_U8_t ucSTAId;

    /** MAC address of the station */
    v_MACADDR_t macAddrSTA;

    /** Current Station state so HDD knows how to deal with packet
     *  queue. Most recent states used to change TL STA state. */
    WLANTL_STAStateType tlSTAState;

   /** Transmit queues for each AC (VO,VI,BE etc). */
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];

   /** Might need to differentiate queue depth in contention case */
   v_U16_t aTxQueueDepth[NUM_TX_QUEUES];
   
   /**Track whether OS TX queue has been disabled.*/
   v_BOOL_t txSuspended[NUM_TX_QUEUES];

   /** Track QoS status of station */
   v_BOOL_t isQosEnabled;

} hdd_station_info_t;

struct hdd_ap_ctx_s
{
   hdd_hostapd_state_t HostapdState;

   // Memory differentiation mode is enabled
   //v_U16_t uMemoryDiffThreshold;
   //v_U8_t uNumActiveAC;
   //v_U8_t uActiveACMask;
   //v_U8_t aTxQueueLimit[NUM_TX_QUEUES];

   /** Packet Count to update uNumActiveAC and uActiveACMask */
   //v_U16_t uUpdatePktCount;

   /** Station ID assigned after BSS starts */
   v_U8_t uBCStaId;

   v_U8_t uPrivacy;  // The privacy bits of configuration

#ifdef WLAN_SOFTAP_FEATURE   
   tSirWPSPBCProbeReq WPSPBCProbeReq;

   tsap_Config_t sapConfig;
#endif
   
   struct semaphore semWpsPBCOverlapInd;
   
   v_BOOL_t apDisableIntraBssFwd;
      
   vos_timer_t hdd_ap_inactivity_timer;

   
   
   v_BOOL_t uIsAuthenticated;
   
#ifdef CONFIG_CFG80211   
   beacon_data_t *beacon;
#endif
};

struct hdd_adapter_s
{
   void *pHddCtx;

   device_mode_t device_mode; 

   /** Handle to the network device */
   struct net_device *dev;
    
   //TODO Move this to sta Ctx
#ifdef CONFIG_CFG80211
   struct wireless_dev wdev ;
   struct cfg80211_scan_request *request ; 
#endif

   /** Current MAC Address for the adapter  */       
   v_MACADDR_t macAddressCurrent;    
      
   /**Event Flags*/
   unsigned long event_flags;

   /**Device TX/RX statistics*/
   struct net_device_stats stats;
   /** HDD statistics*/
   hdd_stats_t hdd_stats;
   /**Mib information*/
   sHddMib_t  hdd_mib;
   
           
   tANI_U8 sessionId;

   //TODO: move these to sta ctx. These may not be used in AP 
   /** completion variable for disconnect callback */
   struct completion disconnect_comp_var;

   /* completion variable for Linkup Event */
   struct completion linkup_event_var;

   /* Track whether the linkup handling is needed  */
   v_BOOL_t isLinkUpSvcNeeded;

/*************************************************************
 *  Tx Queues
 */
   /** Transmit queues for each AC (VO,VI,BE etc) */
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];
   /**Track whether VOS is in a low resource state*/
   v_BOOL_t isVosOutOfResource;
  
   /**Track whether OS TX queue has been disabled.*/
   v_BOOL_t isTxSuspended[NUM_TX_QUEUES];

   /** WMM Status */
   hdd_wmm_status_t hddWmmStatus;
/*************************************************************
 */
/*************************************************************
 * TODO - Remove it later
 */
    /** Multiple station supports */
   /** Per-statioin structure */
   hdd_station_info_t aStaInfo[WLAN_MAX_STA_COUNT];
   //v_U8_t uNumActiveStation;

   v_U16_t aTxQueueLimit[NUM_TX_QUEUES];
/*************************************************************
 */

#ifdef FEATURE_WLAN_WAPI
   hdd_wapi_info_t wapi_info;
#endif

   union {
      hdd_station_ctx_t station;
      hdd_ap_ctx_t  ap;
   }sessionCtx;

};

#define WLAN_HDD_GET_STATION_CTX_PTR(pAdapter) &(pAdapter)->sessionCtx.station
#define WLAN_HDD_GET_AP_CTX_PTR(pAdapter) &(pAdapter)->sessionCtx.ap
#define WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter)  &(pAdapter)->sessionCtx.station.WextState
#define WLAN_HDD_GET_CTX(pAdapter) (hdd_context_t*)pAdapter->pHddCtx
#define WLAN_HDD_GET_HAL_CTX(pAdapter)  ((hdd_context_t*)(pAdapter->pHddCtx))->hHal
#define WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter) &(pAdapter)->sessionCtx.ap.HostapdState
#define WLAN_HDD_GET_CFG_STATE_PTR(pAdapter)  &(pAdapter)->sessionCtx.station.cfg80211State

typedef struct hdd_adapter_list_node
{
   vos_list_node_t node;     // MUST be first element
   hdd_adapter_t *pAdapter;
}hdd_adapter_list_node_t;

/** Adapter stucture definition */

struct hdd_context_s
{
   /** Global VOS context  */
   v_CONTEXT_t pvosContext;

   /** HAL handle...*/
   tHalHandle hHal;

#ifdef CONFIG_CFG80211
   struct wiphy *wiphy ;
#endif
   //TODO Remove this from here.

   vos_list_t hddAdapters; //List of adapters
   /* One per STA: 1 for RX_BCMC_STA_ID and 1 for SAP_SELF_STA_ID*/
   hdd_adapter_t *sta_to_adapter[WLAN_MAX_STA_COUNT + 3]; //One per sta. For quick reference.

   /** Pointer for firmware image data */
   const struct firmware *fw;
   
   /** Pointer for configuration data */
   const struct firmware *cfg;
   
   /** Pointer for nv data */
   const struct firmware *nv;
   
   /** Pointer to the parent device */
   struct device *parent_dev;

   pid_t  pid_sdio_claimed;
   atomic_t sdio_claim_count;

   /** Config values read from qcom_cfg.ini file */ 
   hdd_config_t *cfg_ini;
  #ifdef ANI_MANF_DIAG
   wlan_hdd_ftm_status_t ftm; 
  #endif
   /** completion variable for full power callback */
   struct completion full_pwr_comp_var;
   /** completion variable for standby callback */
   struct completion standby_comp_var;
   
#ifdef FEATURE_WLAN_INTEGRATED_SOC
   /* Completion  variable to indicate Rx Thread Suspended */
   struct completion rx_sus_event_var;
#endif // FEATURE_WLAN_INTEGRATED_SOC

   /* Completion  variable to indicate Tx Thread Suspended */
   struct completion tx_sus_event_var;

   /* Completion  variable to indicate Mc Thread Suspended */
   struct completion mc_sus_event_var;


   v_BOOL_t isWlanSuspended;

   v_BOOL_t isTxThreadSuspended;

   v_BOOL_t isMcThreadSuspended;

   volatile v_BOOL_t isLogpInProgress;

   v_BOOL_t isLoadUnloadInProgress;
   
   /**Track whether driver has been suspended.*/
   hdd_ps_state_t hdd_ps_state;
   
   /* Track whether Mcast/Bcast Filter is enabled.*/
   v_BOOL_t hdd_mcastbcast_filter_set;
   
   /** ptt Process ID*/
   v_SINT_t ptt_pid;

   v_U8_t change_iface;
};
/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 
hdd_adapter_t* hdd_open_adapter( hdd_context_t *pHddCtx, tANI_U8 session_type, char* name, tSirMacAddr macAddr, tANI_U8 rtnl_held );
VOS_STATUS hdd_close_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter, tANI_U8 rtnl_held );
VOS_STATUS hdd_close_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_stop_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_start_all_adapters( hdd_context_t *pHddCtx );
VOS_STATUS hdd_reconnect_all_adapters( hdd_context_t *pHddCtx );
hdd_adapter_t * hdd_get_adapter_by_name( hdd_context_t *pHddCtx, tANI_U8 *name );
hdd_adapter_t * hdd_get_adapter_by_macaddr( hdd_context_t *pHddCtx, tSirMacAddr macAddr );
hdd_adapter_t * hdd_get_mon_adapter( hdd_context_t *pHddCtx );
VOS_STATUS hdd_init_station_mode( hdd_adapter_t *pAdapter );
hdd_adapter_t * hdd_get_adapter( hdd_context_t *pHddCtx, device_mode_t mode );
void hdd_deinit_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
VOS_STATUS hdd_stop_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter );
void hdd_set_station_ops( struct net_device *pWlanDev );
tANI_U8* wlan_hdd_get_intf_addr(hdd_context_t* pHddCtx);
void wlan_hdd_release_intf_addr(hdd_context_t* pHddCtx, tANI_U8* releaseAddr);


#if defined(WLAN_SOFTAP_FEATURE) || defined(ANI_MANF_DIAG)
void hdd_set_conparam ( v_UINT_t newParam );
tVOS_CON_MODE hdd_get_conparam( void );
#endif

void wlan_hdd_enable_deepsleep(v_VOID_t * pVosContext);
#endif    // end #if !defined( WLAN_HDD_MAIN_H )
