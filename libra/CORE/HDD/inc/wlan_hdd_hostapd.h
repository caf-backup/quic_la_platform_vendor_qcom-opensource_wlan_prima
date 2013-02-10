#if !defined( WLAN_HDD_HOSTAPD_H )
#define WLAN_HDD_HOSTAPD_H

/**===========================================================================
  
  \file  WLAN_HDD_HOSTAPD_H.h
  
  \brief Linux HDD HOSTAPD include file
         Copyright 2008 (c) Qualcomm Technologies, Inc.
         All Rights Reserved.
         Qualcomm Technologies Confidential and Proprietary.
  
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
typedef struct hdd_station_info_s 
{
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

void hdd_softap_sta_deauth(hdd_adapter_t*, v_U8_t*);

void hdd_softap_sta_disassoc(hdd_adapter_t*, v_U8_t*);

void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t*, v_BOOL_t);

void hdd_set_ap_ops(struct net_device *pWlanHostapdDev );

int  hdd_softap_unpackIE(tHalHandle halHandle, eCsrEncryptionType *pEncryptType,
                eCsrEncryptionType *mcEncryptType, eCsrAuthType *pAuthType,
                u_int16_t gen_ie_len, u_int8_t *gen_ie );

VOS_STATUS hdd_hostapd_SAPEventCB( tpSap_Event pSapEvent,
                                  v_PVOID_t usrDataForCallback);

#endif    // end #if !defined( WLAN_HDD_HOSTAPD_H )
