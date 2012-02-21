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
#include <linux/spinlock.h>
#ifdef ANI_MANF_DIAG
#include <wlan_hdd_ftm.h>
#endif
/** Number of Tx Queues */
#define NUM_TX_QUEUES 4
#include <wlan_hdd_hostapd.h>
/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/
/** Number of attempts to detect/remove card */
#define LIBRA_CARD_INSERT_DETECT_MAX_COUNT	5
#define LIBRA_CARD_REMOVE_DETECT_MAX_COUNT	5
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
#define WLAN_WAIT_TIME_DISCONNECT  1000
#define WLAN_WAIT_TIME_ABORTSCAN   2000
#define WLAN_WAIT_TIME_STATS       800
#define WLAN_WAIT_TIME_POWER       800
#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
/** Mac Address string **/
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"
#ifdef ANI_CHIPSET_LIBRA
#define WLAN_CHIP_VERSION   "WCN1312"
#endif
#define hddLog(level, args...) VOS_TRACE( VOS_MODULE_ID_HDD, level, ## args)
#define ENTER() VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Enter:%s\n", __FUNCTION__)
#define EXIT()  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Exit:%s\n", __FUNCTION__)
#ifdef CONFIG_CFG80211
#define WLAN_HDD_GET_PRIV_PTR(__dev__) \
    (hdd_adapter_t*) wiphy_priv(((struct wireless_dev*)((__dev__)->ieee80211_ptr))->wiphy)
#else
#define WLAN_HDD_GET_PRIV_PTR(__dev__) (hdd_adapter_t*)(netdev_priv((__dev__)))
#endif
#define MAX_EXIT_ATTEMPTS_DURING_LOGP 6
#define BSS_STOP    0
#define BSS_START   1
#define MAX_CFG_STRING_LEN  255

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

typedef struct hdd_chip_reset_stats_s
{
   __u32    totalLogpResets;
   __u32    totalCMD53Failures;
   __u32    totalMutexReadFailures;
   __u32    totalMIFErrorFailures;
   __u32    totalFWHearbeatFailures;
   __u32    totalUnknownExceptions;
} hdd_chip_reset_stats_t;

typedef struct hdd_stats_s
{
   tCsrSummaryStatsInfo       summary_stat;
   tCsrGlobalClassAStatsInfo  ClassA_stat;
   tCsrGlobalClassBStatsInfo  ClassB_stat;
   tCsrGlobalClassCStatsInfo  ClassC_stat;
   tCsrGlobalClassDStatsInfo  ClassD_stat;
   tCsrPerStaStatsInfo        perStaStats;
   hdd_tx_rx_stats_t          hddTxRxStats;
   hdd_chip_reset_stats_t     hddChipResetStats;
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
   WAPI_AUTH_MODE_OPEN = 0,
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

typedef enum _KEY_DIRECTION WAPI_KEY_DIRECTION;


/** WAPI KEY stucture definition */
struct WLAN_WAPI_KEY
{
   WAPIKeyType     keyType;
   WAPI_KEY_DIRECTION   keyDirection;  /*reserved for future use*/
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
#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WLAN_EID_WAPI 68
#define WAPI_PSK_AKM_SUITE  0x02721400
#define WAPI_CERT_AKM_SUITE 0x01721400

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
   WLAN_HDD_MONITOR,
   WLAN_HDD_FTM,
}device_mode_t;

typedef struct beacon_data_s
{
    u8 *head, *tail;
    int head_len, tail_len;
    int dtim_period;
} beacon_data_t;

/** Adapter stucture definition */

struct hdd_adapter_s
{
   /** Global VOS context  */
   v_CONTEXT_t pvosContext;

   /** HAL handle...*/
   tHalHandle hHal;
#ifdef CONFIG_CFG80211
   struct wiphy* wiphy;
   struct wireless_dev wdev ;
   struct cfg80211_scan_request *request ; 
   device_mode_t  device_mode;
#endif

   /** Handle to the network device */
   struct net_device *dev;
	
#ifdef WLAN_SOFTAP_FEATURE
   /** Handle to the network AP device */
   struct net_device *pHostapd_dev;
   /* pHostapd_dev, pWlanDev and dev would be pointing to only one net_dev
    * so as to minimize changes in the code and ensure compatibility.
    **/
   struct net_device *pWlanDev;
#endif
	
  /** Handle to the Wireless Extension State */
   hdd_wext_state_t *pWextState;	
	
   /** Pointer for firmware image data */
   const struct firmware *fw;
   
   /** Pointer for configuration data */
   const struct firmware *cfg;
   
   /** Pointer for nv data */
   const struct firmware *nv;
   
   /** Handle to the sdio function device */
   struct sdio_func *hsdio_func_dev;
   /**Connection information*/
   connection_info_t conn_info;
   roaming_info_t roam_info;
   /**Mib information*/
   sHddMib_t  hdd_mib;
   
   /** HDD statistics*/
   hdd_stats_t hdd_stats;
           
   /** Current MAC Address for the adapter  */       
   v_MACADDR_t macAddressCurrent;	
      
   /** Transmit queues for each AC (VO,VI,BE etc) */
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];
   /**Track whether VOS is in a low resource state*/
   v_BOOL_t isVosOutOfResource;
  
   /**Track whether OS TX queue has been disabled.*/
   v_BOOL_t isTxSuspended[NUM_TX_QUEUES];
   /**Event Flags*/
   unsigned long event_flags;
   /**Device TX/RX statistics*/
   struct net_device_stats stats;
   /** WMM Status */
   hdd_wmm_status_t hddWmmStatus;
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
   /** completion variable for disconnect callback */
   struct completion disconnect_comp_var;
/* completion variable for Linkup Event */

   struct completion linkup_event_var;
   
   /* Completion  variable to indicate Tx Thread Suspended */
   struct completion tx_sus_event_var;

   /* Completion  variable to indicate Mc Thread Suspended */
   struct completion mc_sus_event_var;

   /* Completion  variable to indicate Mc Thread Suspended */
   struct completion abortscan_event_var;

   /* Track whether the linkup handling is needed  */
   v_BOOL_t isLinkUpSvcNeeded;

   v_BOOL_t isWlanSuspended;

   v_BOOL_t isTxThreadSuspended;

   v_BOOL_t isMcThreadSuspended;

   volatile v_BOOL_t isLogpInProgress;

   v_BOOL_t isLoadUnloadInProgress;
   
   /**Track whether driver has been suspended.*/
   hdd_ps_state_t hdd_ps_state;
   
   /* Track whether Mcast/Bcast Filter is enabled.*/
   v_BOOL_t hdd_mcastbcast_filter_set;
   
   v_BOOL_t hdd_wlan_suspended;
   
   spinlock_t filter_lock;
   
   /** ptt Process ID*/
   v_SINT_t ptt_pid;
   tANI_U8 sessionId;
#ifdef FEATURE_WLAN_WAPI
   hdd_wapi_info_t wapi_info;
#endif
   eCsrEncryptionType ucEncryptType;
   /**The following elements are  the members of hostapd_adapter structure
    * which will now be part of the adapter structure
    **/
   struct hdd_station_info_s aStaInfo[WLAN_MAX_STA_COUNT];

   v_U16_t aTxQueueLimit[NUM_TX_QUEUES];

   hdd_hostapd_state_t HostapdState;

   v_U8_t uBCStaId;

   v_U8_t uPrivacy;

   tSirWPSPBCProbeReq WPSPBCProbeReq;

   v_BOOL_t apDisableIntraBssFwd;

   struct semaphore semWpsPBCOverlapInd;

   // This will have WEP key data, if it is received before start bss
   tCsrRoamSetKey wepKey[CSR_MAX_NUM_KEY];

#ifdef CONFIG_CFG80211
   tCsrRoamSetKey groupKey;

   beacon_data_t *beacon;
#endif

   tsap_Config_t sapConfig;

   vos_timer_t hdd_ap_inactivity_timer;
};
/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 
void wlan_hdd_enable_deepsleep(v_VOID_t * pVosContext);

void hdd_deinit_adapter( hdd_adapter_t *pAdapter );

VOS_STATUS hdd_stop_adapter( hdd_adapter_t *pAdapter);

void hdd_set_conparam ( v_UINT_t newParam );

void hdd_set_station_ops( struct net_device *pWlanDev );

VOS_STATUS hdd_init_station_mode( hdd_adapter_t *pAdapter );

#endif    // end #if !defined( WLAN_HDD_MAIN_H )
