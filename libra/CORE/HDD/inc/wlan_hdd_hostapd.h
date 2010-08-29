#if !defined( WLAN_HDD_HOSTAPD_H )
#define WLAN_HDD_HOSTAPD_H

/**===========================================================================
  
  \file  WLAN_HDD_HOSTAPD_H.h
  
  \brief Linux HDD HOSTAPD include file
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

#include <wlan_qct_tl.h>
#include <wlan_hdd_main.h>

/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 

typedef struct hdd_hostapd_state_s
{
    int HostapdState;
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
   v_BOOL_t txSuspended;
   v_U8_t   txSuspendedAc;   

} hdd_station_info_t;

typedef struct hdd_hostapd_adapter_s
{
   /** Handle to the wlan main network device*/
   struct net_device *pWlanDev;

   hdd_hostapd_state_t *pHostapdState;

    /** Global VOS context  */
    v_CONTEXT_t pvosContext;

    /** HAL handle...*/
    tHalHandle hHal;

   /** Handle to the hostapd (softap) network device */
   struct net_device *pDev;

    /** Multiple station supports */
   /** Per-statioin structure */
   hdd_station_info_t aStaInfo[WLAN_MAX_STA_COUNT];
   //v_U8_t uNumActiveStation;

   v_U16_t aTxQueueLimit[NUM_TX_QUEUES];

   // Memory differentiation mode is enabled
   //v_U16_t uMemoryDiffThreshold;
   //v_U8_t uNumActiveAC;
   //v_U8_t uActiveACMask;
   //v_U8_t aTxQueueLimit[NUM_TX_QUEUES];

   /** Packet Count to update uNumActiveAC and uActiveACMask */
   //v_U16_t uUpdatePktCount;
   /*
    * Other fields from hdd_adapter_s
   */

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

   /** ptt Process ID*/
   v_SINT_t ptt_pid;

   tANI_U8 sessionId;

   /** Station ID assigned after BSS starts */
   v_U8_t uBCStaId;

   v_U8_t uPrivacy;  // The privacy bits of configuration
   
   tSirWPSPBCProbeReq WPSPBCProbeReq;
   
   struct semaphore semWpsPBCOverlapInd;
   
} hdd_hostapd_adapter_t;

int hdd_wlan_create_ap_dev(struct net_device *pWlanDev);

int hdd_register_hostapd(struct net_device *hostap_dev);

int hdd_unregister_hostapd(struct net_device *hostap_dev);

eCsrAuthType 
hdd_TranslateRSNToCsrAuthType( u_int8_t auth_suite[4]);

eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4]);

eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4]);

eCsrAuthType 
hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]);

eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4]);

void hdd_softap_sta_deauth(hdd_hostapd_adapter_t*,v_U8_t*);
void hdd_softap_sta_disassoc(hdd_hostapd_adapter_t*,v_U8_t*);
void hdd_softap_tkip_mic_fail_counter_measure(hdd_hostapd_adapter_t*,v_BOOL_t);

#endif    // end #if !defined( WLAN_HDD_HOSTAPD_H )
