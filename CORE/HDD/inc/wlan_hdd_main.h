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
#define HDD_TX_TIMEOUT          (2*HZ)

/**event flags registered net device*/
#define NET_DEVICE_REGISTERED  1<<0

/** Maximum time(ms)to wait for disconnect to complete **/
#define WLAN_WAIT_TIME_DISCONNECT  100

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

/** Mac Address string **/
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"


#define hddLog(level, args...) VOS_TRACE( VOS_MODULE_ID_HDD, level, ## args)
#define ENTER() VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Enter:%s\n", __FUNCTION__)
#define EXIT()  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "Exit:%s\n", __FUNCTION__)


typedef struct hdd_stats_s
{
   tCsrSummaryStatsInfo       summary_stat;
   tCsrGlobalClassAStatsInfo  ClassA_stat;
   tCsrGlobalClassBStatsInfo  ClassB_stat;
   tCsrGlobalClassCStatsInfo  ClassC_stat;
   tCsrGlobalClassDStatsInfo  ClassD_stat;
   tCsrPerStaStatsInfo        perStaStats;

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

/** Adapter stucture definition */

struct hdd_adapter_s
{
   /** Global VOS context  */
   v_CONTEXT_t pvosContext;

   /** HAL handle...*/
   tHalHandle hHal;

   /** Handle to the network device */
   struct net_device *dev;
	
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
   v_BOOL_t isTxSuspended;
   v_U8_t   txSuspendedAc;

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

   /**Track whether driver has been suspended.*/
   hdd_ps_state_t hdd_ps_state;
};

/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 

#endif    // end #if !defined( WLAN_HDD_MAIN_H )
