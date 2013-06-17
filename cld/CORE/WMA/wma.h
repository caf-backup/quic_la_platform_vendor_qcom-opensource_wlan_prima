/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/**========================================================================

  \file     wma.h
  \brief    Implementation of WMA

  Copyright 2013 (c) Qualcomm Technologies, Inc.  All Rights Reserved.

  Qualcomm Technologies Confidential and Proprietary.

  ========================================================================*/
/**=========================================================================
  EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header:$   $DateTime: $ $Author: $


  when              who           what, where, why
  --------          ---           -----------------------------------------
  12/03/2013        Ganesh        Created module for WMA
                    Kondabattini
  27/03/2013        Ganesh        Rx Mgmt Related added 
                    Babu
  ==========================================================================*/
#ifndef WMA_H
#define WMA_H

#include "a_types.h"
#include "vos_types.h"
#include "osapi_linux.h"
#include "htc_packet.h"
#include "i_vos_event.h"
#include "wmi_services.h"
#include "wmi_unified.h"
#include "halTypes.h"
#include "cfgApi.h"
#include "vos_status.h"
#include "vos_sched.h"
#include "wlan_hdd_tgt_cfg.h"
#include "ol_txrx_api.h"
#include "sirMacProtDef.h"
#include "wlan_qct_wda.h"
#include "ol_txrx_types.h"
#include <linux/workqueue.h>

/* Platform specific configuration for max. no. of fragments */
#define QCA_OL_11AC_TX_MAX_FRAGS            2

/** Private **/
#define WMA_CFG_NV_DNLD_TIMEOUT            500
#define WMA_READY_EVENTID_TIMEOUT          2000
#define MAX_MEM_CHUNKS 32
/*
   In prima 12 HW stations are supported including BCAST STA(staId 0)
   and SELF STA(staId 1) so total ASSOC stations which can connect to Prima
   SoftAP = 12 - 1(Self STa) - 1(Bcast Sta) = 10 Stations. */
   
#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define WMA_MAX_SUPPORTED_STAS    38 
#else
#define WMA_MAX_SUPPORTED_STAS    12 
#endif
#define WMA_MAX_SUPPORTED_BSS     5 

#define FRAGMENT_SIZE 3072

#define WMA_INVALID_VDEV_ID				0xFF
#define MAX_MEM_CHUNKS					32
#define WMA_MAX_VDEV_SIZE				20
#define WMA_VDEV_TBL_ENTRY_ADD				1
#define WMA_VDEV_TBL_ENTRY_DEL				0


#define WMA_LOGD(args...) \
	VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_DEBUG, ## args)
#define WMA_LOGI(args...) \
	VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO, ## args)
#define WMA_LOGW(args...) \
	VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_WARNING, ## args)
#define WMA_LOGE(args...) \
	VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, ## args)
#define WMA_LOGP(args...) \
	VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_FATAL, ## args)

#define WMA_DEBUG_ALWAYS

#ifdef WMA_DEBUG_ALWAYS
#define WMA_LOGA(fmt, args...) \
	printk(KERN_INFO "\n%s-%d: " fmt, __func__, __LINE__, ## args)
#else
#define WMA_LOGA(fmt, args...) 
#endif

#define     ALIGNED_WORD_SIZE       4
#define WLAN_HAL_MSG_TYPE_MAX_ENUM_SIZE    0x7FFF

/* Prefix used by scan req ids generated on the host */
#define WMA_HOST_SCAN_REQID_PREFIX	 0xA000
/* Prefix used by scan requestor id on host */
#define WMA_HOST_SCAN_REQUESTOR_ID_PREFIX 0xA000
#define WMA_HW_DEF_SCAN_MAX_DURATION	  5000 /* 5 secs */

typedef struct {
	HTC_ENDPOINT_ID endpoint_id;
}t_cfg_nv_param;

struct wma_version {
	u_int32_t    host_ver;
	u_int32_t    target_ver;
	u_int32_t    wlan_ver;
	u_int32_t    abi_ver;
};

typedef enum
{
	WMA_DRIVER_TYPE_PRODUCTION  = 0,
	WMA_DRIVER_TYPE_MFG         = 1,
	WMA_DRIVER_TYPE_DVT         = 2,
	WMA_DRIVER_TYPE_INVALID     = 0x7FFFFFFF
}t_wma_drv_type;

typedef enum {
	WMA_STATE_OPEN,
	WMA_STATE_START,
	WMA_STATE_STOP,
	WMA_STATE_CLOSE
}t_wma_state;

/*
 * memory chunck allocated by Host to be managed by FW
 * used only for low latency interfaces like pcie
 */
struct wma_mem_chunk {
    u_int32_t *vaddr;
    u_int32_t paddr;
    adf_os_dma_mem_context(memctx);
    u_int32_t len;
    u_int32_t req_id;
};

typedef struct s_vdev_tbl {
	u_int8_t vdev_id;
	u_int8_t sta_mac[ETH_ALEN];
	ol_txrx_vdev_handle tx_rx_vdev_handle;
	u_int32_t vdev_type;
	bool used;
}t_vdev_tbl;

struct scan_param{
	u_int32_t scan_id;
	u_int32_t scan_requestor_id;
	u_int32_t vdev_id;
};

struct wma_txrx_node {
	u_int8_t addr[ETH_ALEN];
	void *handle;
};

typedef struct {
	void *wmi_handle;
	void *htc_handle;
	void *vos_context;
	void *mac_context;

#ifdef QCA_WIFI_ISOC
	vos_event_t cfg_nv_tx_complete;
	vos_event_t cfg_nv_rx_complete;
#endif
	vos_event_t wma_ready_event;
	t_cfg_nv_param cfg_nv;

	v_U16_t max_station;
	v_U16_t max_bssid;
	v_U32_t frame_xln_reqd;
	t_wma_drv_type driver_type;

	/* TODO: Check below 2 parameters are required for ROME/PRONTO ? */
	u_int8_t myaddr[ETH_ALEN]; /* current mac address */
	u_int8_t hwaddr[ETH_ALEN]; /* mac address from EEPROM */

	struct wma_version version;
	bool wmi_ready;
	u_int32_t wlan_init_status;
	adf_os_device_t adf_dev;
	u_int32_t phy_capability; /* PHY Capability from Target*/
	u_int32_t max_frag_entry; /* Max number of Fragment entry */
	u_int32_t wmi_service_bitmap[WMI_SERVICE_BM_SIZE]; /* wmi services bitmap received from Target */
	wmi_resource_config   wlan_resource_config;
	u_int32_t frameTransRequired;
	tBssSystemRole       wmaGlobalSystemRole;

	/* Tx Frame Compl Cb registered by umac */
	pWDATxRxCompFunc tx_frm_download_comp_cb;

	/* Event to wait for tx download completion */
	vos_event_t tx_frm_download_comp_event;

	/* Ack Complete Callback registered by umac */
	pWDAAckFnTxComp umac_ota_ack_cb[SIR_MAC_MGMT_RESERVED15];

	v_BOOL_t needShutdown;
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	u_int32_t num_mem_chunks;
	struct wma_mem_chunk mem_chunks[MAX_MEM_CHUNKS];
#endif
	hdd_tgt_cfg_cb tgt_cfg_update_cb;
	HAL_REG_CAPABILITIES reg_cap;
	u_int32_t scan_id;
	struct scan_param cur_scan_info;
	struct wma_txrx_node *interfaces;
}t_wma_handle, *tp_wma_handle;

struct wma_target_cap {
	u_int32_t wmi_service_bitmap[WMI_SERVICE_BM_SIZE]; /* wmi services bitmap received from Target */
	wmi_resource_config wlan_resource_config; /* default resource config,the os shim can overwrite it */
};

/********** The following structures are referenced from legacy prima code *********/
typedef enum {
	QWLAN_ISOC_START_CMDID = 0x4000,
	QWLAN_ISOC_END_CMDID   = 0x4FFF,

	FW_CFG_DOWNLOAD_REQ = QWLAN_ISOC_START_CMDID,
	FW_NV_DOWNLOAD_REQ,
	FW_WLAN_HAL_STOP_REQ,
	/* Add additional commands here */

	QWLAN_ISOC_MAX_CMDID = QWLAN_ISOC_END_CMDID - 1
}QWLAN_CMD_ID;
/* The shared memory between WDI and HAL is 4K so maximum data can be transferred
from WDI to HAL is 4K.This 4K should also include the Message header so sending 4K
of NV fragment is nt possbile.The next multiple of 1Kb is 3K */

typedef struct
{
  v_VOID_t *pConfigBuffer; 
  
  /*Length of the config buffer above*/
  v_U16_t usConfigBufferLen;
  
  /*Production or FTM driver*/
  t_wma_drv_type driver_type; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  v_VOID_t *pUserData;

  /*The user data passed in by UMAC, it will be sent back when the indication
    function pointer will be called */
  v_VOID_t *pIndUserData;
}t_wma_start_req;

/* Config format required by HAL for each CFG item*/
typedef PACKED_PRE struct PACKED_POST
{
	/* Cfg Id. The Id required by HAL is exported by HAL
	 * in shared header file between UMAC and HAL.*/
	tANI_U16   uCfgId;

	/* Length of the Cfg. This parameter is used to go to next cfg
	 * in the TLV format.*/
	tANI_U16   uCfgLen;

	/* Padding bytes for unaligned address's */
	tANI_U16   uCfgPadBytes;

	/* Reserve bytes for making cfgVal to align address */
	tANI_U16   uCfgReserve;

	/* Following the uCfgLen field there should be a 'uCfgLen' bytes
	 * containing the uCfgValue ; tANI_U8 uCfgValue[uCfgLen] */
} tHalCfg, *tpHalCfg;

/* Message types for messages exchanged between WDI and HAL */
typedef enum 
{
   //Init/De-Init
   WLAN_HAL_START_REQ = 0,
   WLAN_HAL_START_RSP = 1,
   WLAN_HAL_STOP_REQ  = 2,
   WLAN_HAL_STOP_RSP  = 3,

   //Scan
   WLAN_HAL_INIT_SCAN_REQ    = 4,
   WLAN_HAL_INIT_SCAN_RSP    = 5,
   WLAN_HAL_START_SCAN_REQ   = 6,
   WLAN_HAL_START_SCAN_RSP   = 7 ,
   WLAN_HAL_END_SCAN_REQ     = 8,
   WLAN_HAL_END_SCAN_RSP     = 9,
   WLAN_HAL_FINISH_SCAN_REQ  = 10,
   WLAN_HAL_FINISH_SCAN_RSP  = 11,

   // HW STA configuration/deconfiguration
   WLAN_HAL_CONFIG_STA_REQ   = 12,
   WLAN_HAL_CONFIG_STA_RSP   = 13,
   WLAN_HAL_DELETE_STA_REQ   = 14,
   WLAN_HAL_DELETE_STA_RSP   = 15,
   WLAN_HAL_CONFIG_BSS_REQ   = 16,
   WLAN_HAL_CONFIG_BSS_RSP   = 17,
   WLAN_HAL_DELETE_BSS_REQ   = 18,
   WLAN_HAL_DELETE_BSS_RSP   = 19,

   //Infra STA asscoiation
   WLAN_HAL_JOIN_REQ         = 20,
   WLAN_HAL_JOIN_RSP         = 21,
   WLAN_HAL_POST_ASSOC_REQ   = 22,
   WLAN_HAL_POST_ASSOC_RSP   = 23,

   //Security
   WLAN_HAL_SET_BSSKEY_REQ   = 24,
   WLAN_HAL_SET_BSSKEY_RSP   = 25,
   WLAN_HAL_SET_STAKEY_REQ   = 26,
   WLAN_HAL_SET_STAKEY_RSP   = 27,
   WLAN_HAL_RMV_BSSKEY_REQ   = 28,
   WLAN_HAL_RMV_BSSKEY_RSP   = 29,
   WLAN_HAL_RMV_STAKEY_REQ   = 30,
   WLAN_HAL_RMV_STAKEY_RSP   = 31,

   //Qos Related
   WLAN_HAL_ADD_TS_REQ          = 32,
   WLAN_HAL_ADD_TS_RSP          = 33,
   WLAN_HAL_DEL_TS_REQ          = 34,
   WLAN_HAL_DEL_TS_RSP          = 35,
   WLAN_HAL_UPD_EDCA_PARAMS_REQ = 36,
   WLAN_HAL_UPD_EDCA_PARAMS_RSP = 37,
   WLAN_HAL_ADD_BA_REQ          = 38,
   WLAN_HAL_ADD_BA_RSP          = 39,
   WLAN_HAL_DEL_BA_REQ          = 40,
   WLAN_HAL_DEL_BA_RSP          = 41,

   WLAN_HAL_CH_SWITCH_REQ       = 42,
   WLAN_HAL_CH_SWITCH_RSP       = 43,
   WLAN_HAL_SET_LINK_ST_REQ     = 44,
   WLAN_HAL_SET_LINK_ST_RSP     = 45,
   WLAN_HAL_GET_STATS_REQ       = 46,
   WLAN_HAL_GET_STATS_RSP       = 47,
   WLAN_HAL_UPDATE_CFG_REQ      = 48,
   WLAN_HAL_UPDATE_CFG_RSP      = 49,

   WLAN_HAL_MISSED_BEACON_IND           = 50,
   WLAN_HAL_UNKNOWN_ADDR2_FRAME_RX_IND  = 51,
   WLAN_HAL_MIC_FAILURE_IND             = 52,
   WLAN_HAL_FATAL_ERROR_IND             = 53,
   WLAN_HAL_SET_KEYDONE_MSG             = 54,
   
   //NV Interface
   WLAN_HAL_DOWNLOAD_NV_REQ             = 55,
   WLAN_HAL_DOWNLOAD_NV_RSP             = 56,

   WLAN_HAL_ADD_BA_SESSION_REQ          = 57,
   WLAN_HAL_ADD_BA_SESSION_RSP          = 58,
   WLAN_HAL_TRIGGER_BA_REQ              = 59,
   WLAN_HAL_TRIGGER_BA_RSP              = 60,
   WLAN_HAL_UPDATE_BEACON_REQ           = 61,
   WLAN_HAL_UPDATE_BEACON_RSP           = 62,
   WLAN_HAL_SEND_BEACON_REQ             = 63,
   WLAN_HAL_SEND_BEACON_RSP             = 64,

   WLAN_HAL_SET_BCASTKEY_REQ               = 65,
   WLAN_HAL_SET_BCASTKEY_RSP               = 66,
   WLAN_HAL_DELETE_STA_CONTEXT_IND         = 67,
   WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_REQ  = 68,
   WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_RSP  = 69,
   
  // PTT interface support
   WLAN_HAL_PROCESS_PTT_REQ   = 70,
   WLAN_HAL_PROCESS_PTT_RSP   = 71,
   
   // BTAMP related events
   WLAN_HAL_SIGNAL_BTAMP_EVENT_REQ  = 72,
   WLAN_HAL_SIGNAL_BTAMP_EVENT_RSP  = 73,
   WLAN_HAL_TL_HAL_FLUSH_AC_REQ     = 74,
   WLAN_HAL_TL_HAL_FLUSH_AC_RSP     = 75,

   WLAN_HAL_ENTER_IMPS_REQ           = 76,
   WLAN_HAL_EXIT_IMPS_REQ            = 77,
   WLAN_HAL_ENTER_BMPS_REQ           = 78,
   WLAN_HAL_EXIT_BMPS_REQ            = 79,
   WLAN_HAL_ENTER_UAPSD_REQ          = 80,
   WLAN_HAL_EXIT_UAPSD_REQ           = 81,
   WLAN_HAL_UPDATE_UAPSD_PARAM_REQ   = 82,
   WLAN_HAL_CONFIGURE_RXP_FILTER_REQ = 83,
   WLAN_HAL_ADD_BCN_FILTER_REQ       = 84,
   WLAN_HAL_REM_BCN_FILTER_REQ       = 85,
   WLAN_HAL_ADD_WOWL_BCAST_PTRN      = 86,
   WLAN_HAL_DEL_WOWL_BCAST_PTRN      = 87,
   WLAN_HAL_ENTER_WOWL_REQ           = 88,
   WLAN_HAL_EXIT_WOWL_REQ            = 89,
   WLAN_HAL_HOST_OFFLOAD_REQ         = 90,
   WLAN_HAL_SET_RSSI_THRESH_REQ      = 91,
   WLAN_HAL_GET_RSSI_REQ             = 92,
   WLAN_HAL_SET_UAPSD_AC_PARAMS_REQ  = 93,
   WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ = 94,

   WLAN_HAL_ENTER_IMPS_RSP           = 95,
   WLAN_HAL_EXIT_IMPS_RSP            = 96,
   WLAN_HAL_ENTER_BMPS_RSP           = 97,
   WLAN_HAL_EXIT_BMPS_RSP            = 98,
   WLAN_HAL_ENTER_UAPSD_RSP          = 99,
   WLAN_HAL_EXIT_UAPSD_RSP           = 100,
   WLAN_HAL_SET_UAPSD_AC_PARAMS_RSP  = 101,
   WLAN_HAL_UPDATE_UAPSD_PARAM_RSP   = 102,
   WLAN_HAL_CONFIGURE_RXP_FILTER_RSP = 103,
   WLAN_HAL_ADD_BCN_FILTER_RSP       = 104,
   WLAN_HAL_REM_BCN_FILTER_RSP       = 105,
   WLAN_HAL_SET_RSSI_THRESH_RSP      = 106,
   WLAN_HAL_HOST_OFFLOAD_RSP         = 107,
   WLAN_HAL_ADD_WOWL_BCAST_PTRN_RSP  = 108,
   WLAN_HAL_DEL_WOWL_BCAST_PTRN_RSP  = 109,
   WLAN_HAL_ENTER_WOWL_RSP           = 110,
   WLAN_HAL_EXIT_WOWL_RSP            = 111,
   WLAN_HAL_RSSI_NOTIFICATION_IND    = 112,
   WLAN_HAL_GET_RSSI_RSP             = 113,
   WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_RSP = 114,

   //11k related events
   WLAN_HAL_SET_MAX_TX_POWER_REQ   = 115,
   WLAN_HAL_SET_MAX_TX_POWER_RSP   = 116,

   //11R related msgs
   WLAN_HAL_AGGR_ADD_TS_REQ        = 117,
   WLAN_HAL_AGGR_ADD_TS_RSP        = 118,

   //P2P  WLAN_FEATURE_P2P
   WLAN_HAL_SET_P2P_GONOA_REQ      = 119,
   WLAN_HAL_SET_P2P_GONOA_RSP      = 120,
   
   //WLAN Dump commands
   WLAN_HAL_DUMP_COMMAND_REQ       = 121,
   WLAN_HAL_DUMP_COMMAND_RSP       = 122,

   //OEM_DATA FEATURE SUPPORT
   WLAN_HAL_START_OEM_DATA_REQ   = 123,
   WLAN_HAL_START_OEM_DATA_RSP   = 124,

   //ADD SELF STA REQ and RSP
   WLAN_HAL_ADD_STA_SELF_REQ       = 125,
   WLAN_HAL_ADD_STA_SELF_RSP       = 126,

   //DEL SELF STA SUPPORT
   WLAN_HAL_DEL_STA_SELF_REQ       = 127,
   WLAN_HAL_DEL_STA_SELF_RSP       = 128,

   // Coex Indication
   WLAN_HAL_COEX_IND               = 129,

   // Tx Complete Indication 
   WLAN_HAL_OTA_TX_COMPL_IND       = 130,

   //Host Suspend/resume messages
   WLAN_HAL_HOST_SUSPEND_IND       = 131,
   WLAN_HAL_HOST_RESUME_REQ        = 132,
   WLAN_HAL_HOST_RESUME_RSP        = 133,

   WLAN_HAL_SET_TX_POWER_REQ       = 134,
   WLAN_HAL_SET_TX_POWER_RSP       = 135,
   WLAN_HAL_GET_TX_POWER_REQ       = 136,
   WLAN_HAL_GET_TX_POWER_RSP       = 137,

   WLAN_HAL_P2P_NOA_ATTR_IND       = 138,
   
   WLAN_HAL_ENABLE_RADAR_DETECT_REQ  = 139,
   WLAN_HAL_ENABLE_RADAR_DETECT_RSP  = 140,
   WLAN_HAL_GET_TPC_REPORT_REQ       = 141,
   WLAN_HAL_GET_TPC_REPORT_RSP       = 142,
   WLAN_HAL_RADAR_DETECT_IND         = 143,
   WLAN_HAL_RADAR_DETECT_INTR_IND    = 144,
   WLAN_HAL_KEEP_ALIVE_REQ           = 145,
   WLAN_HAL_KEEP_ALIVE_RSP           = 146,      

   /*PNO messages*/
   WLAN_HAL_SET_PREF_NETWORK_REQ     = 147,
   WLAN_HAL_SET_PREF_NETWORK_RSP     = 148,
   WLAN_HAL_SET_RSSI_FILTER_REQ      = 149,
   WLAN_HAL_SET_RSSI_FILTER_RSP      = 150,
   WLAN_HAL_UPDATE_SCAN_PARAM_REQ    = 151,
   WLAN_HAL_UPDATE_SCAN_PARAM_RSP    = 152,
   WLAN_HAL_PREF_NETW_FOUND_IND      = 153, 

   WLAN_HAL_SET_TX_PER_TRACKING_REQ  = 154,
   WLAN_HAL_SET_TX_PER_TRACKING_RSP  = 155,
   WLAN_HAL_TX_PER_HIT_IND           = 156,
   
   WLAN_HAL_8023_MULTICAST_LIST_REQ   = 157,
   WLAN_HAL_8023_MULTICAST_LIST_RSP   = 158,   

   WLAN_HAL_SET_PACKET_FILTER_REQ     = 159,
   WLAN_HAL_SET_PACKET_FILTER_RSP     = 160,   
   WLAN_HAL_PACKET_FILTER_MATCH_COUNT_REQ   = 161,
   WLAN_HAL_PACKET_FILTER_MATCH_COUNT_RSP   = 162,   
   WLAN_HAL_CLEAR_PACKET_FILTER_REQ         = 163,
   WLAN_HAL_CLEAR_PACKET_FILTER_RSP         = 164,  
   /*This is temp fix. Should be removed once 
    * Host and Riva code is in sync*/
   WLAN_HAL_INIT_SCAN_CON_REQ               = 165,
    
   WLAN_HAL_SET_POWER_PARAMS_REQ            = 166,
   WLAN_HAL_SET_POWER_PARAMS_RSP            = 167,

   WLAN_HAL_TSM_STATS_REQ                   = 168,
   WLAN_HAL_TSM_STATS_RSP                   = 169,

   // wake reason indication (WOW)
   WLAN_HAL_WAKE_REASON_IND                 = 170,
   // GTK offload support 
   WLAN_HAL_GTK_OFFLOAD_REQ                 = 171,
   WLAN_HAL_GTK_OFFLOAD_RSP                 = 172,
   WLAN_HAL_GTK_OFFLOAD_GETINFO_REQ         = 173,
   WLAN_HAL_GTK_OFFLOAD_GETINFO_RSP         = 174,

   WLAN_HAL_FEATURE_CAPS_EXCHANGE_REQ       = 175,
   WLAN_HAL_FEATURE_CAPS_EXCHANGE_RSP       = 176,
   WLAN_HAL_EXCLUDE_UNENCRYPTED_IND         = 177,

   WLAN_HAL_SET_THERMAL_MITIGATION_REQ      = 178,
   WLAN_HAL_SET_THERMAL_MITIGATION_RSP      = 179,

  WLAN_HAL_UPDATE_VHT_OP_MODE_REQ          = 182,
  WLAN_HAL_UPDATE_VHT_OP_MODE_RSP          = 183,
 
   WLAN_HAL_P2P_NOA_START_IND               = 184,

   WLAN_HAL_GET_ROAM_RSSI_REQ               = 185,
   WLAN_HAL_GET_ROAM_RSSI_RSP               = 186,
   
   WLAN_HAL_CLASS_B_STATS_IND               = 187,
   WLAN_HAL_DEL_BA_IND                      = 188,
   WLAN_HAL_DHCP_START_IND                  = 189,
   WLAN_HAL_DHCP_STOP_IND                   = 190,

  WLAN_HAL_MSG_MAX = WLAN_HAL_MSG_TYPE_MAX_ENUM_SIZE
}tHalHostMsgType;

/* Enumeration for Version */
typedef enum
{
   WLAN_HAL_MSG_VERSION0 = 0,
   WLAN_HAL_MSG_VERSION1 = 1,
   WLAN_HAL_MSG_WCNSS_CTRL_VERSION = 0x7FFF, /*define as 2 bytes data*/
   WLAN_HAL_MSG_VERSION_MAX_FIELD  = WLAN_HAL_MSG_WCNSS_CTRL_VERSION
} tHalHostMsgVersion;

/* 4-byte control message header used by HAL*/
typedef PACKED_PRE struct PACKED_POST
{
   tHalHostMsgType  msgType:16;
   tHalHostMsgVersion msgVersion:16;
   tANI_U32         msgLen;
} tHalMsgHeader, *tpHalMsgHeader;
#ifdef QCA_WIFI_ISOC
/*Version string max length (including NUL) */
#define WLAN_HAL_VERSION_LENGTH  64
/* Definition for HAL API Version.*/
typedef PACKED_PRE struct PACKED_POST
{
	u_int8_t                  revision;
	u_int8_t                  version;
	u_int8_t                  minor;
	u_int8_t                  major;
} tWcnssWlanVersion, *tpWcnssWlanVersion;

/*---------------------------------------------------------------------------
 * WLAN_HAL_DOWNLOAD_NV_REQ
 *--------------------------------------------------------------------------*/
typedef PACKED_PRE struct PACKED_POST
{
    /* Fragment sequence number of the NV Image. Note that NV Image might not
     * fit into one message due to size limitation of the SMD channel FIFO. UMAC
     * can hence choose to chop the NV blob into multiple fragments starting with 
     * seqeunce number 0, 1, 2 etc. The last fragment MUST be indicated by 
     * marking the isLastFragment field to 1. Note that all the NV blobs would be
     * concatenated together by HAL without any padding bytes in between.*/
    tANI_U16 fragNumber;

    /* Is this the last fragment? When set to 1 it indicates that no more fragments
     * will be sent by UMAC and HAL can concatenate all the NV blobs rcvd & proceed 
     * with the parsing. HAL would generate a WLAN_HAL_DOWNLOAD_NV_RSP to the
     * WLAN_HAL_DOWNLOAD_NV_REQ after it receives each fragment */
    tANI_U16 isLastFragment;

    /* NV Image size (number of bytes) */
    tANI_U32 nvImgBufferSize;

    /* Following the 'nvImageBufferSize', there should be nvImageBufferSize
     * bytes of NV Image i.e. uint8[nvImageBufferSize] */
} tHalNvImgDownloadReqParams, *tpHalNvImgDownloadReqParams;


typedef PACKED_PRE struct PACKED_POST
{
    /* Note: The length specified in tHalNvImgDownloadReqMsg messages should be
     * header.msgLen = sizeof(tHalNvImgDownloadReqMsg) + nvImgBufferSize */
    tHalMsgHeader header;
    tHalNvImgDownloadReqParams nvImageReqParams;
} tHalNvImgDownloadReqMsg, *tpHalNvImgDownloadReqMsg;

/*---------------------------------------------------------------------------
  WLAN_HAL_STOP_REQ
---------------------------------------------------------------------------*/

typedef PACKED_PRE struct PACKED_POST
{
	/*The reason for which the device is being stopped*/
	tHalStopType   reason;
} tHalMacStopReqParams, *tpHalMacStopReqParams;

typedef PACKED_PRE struct PACKED_POST
{
	tHalMsgHeader header;
	tHalMacStopReqParams stopReqParams;
} tHalMacStopReqMsg, *tpHalMacStopReqMsg;

/*---------------------------------------------------------------------------
  WLAN_HAL_STOP_RSP
---------------------------------------------------------------------------*/

typedef PACKED_PRE struct PACKED_POST
{
	/*success or failure */
	u_int32_t   status;

} tHalMacStopRspParams, *tpHalMacStopRspParams;

typedef PACKED_PRE struct PACKED_POST
{
	tHalMsgHeader header;
	tHalMacStopRspParams stopRspParams;
}  tHalMacStopRspMsg, *tpHalMacStopRspMsg;

/*---------------------------------------------------------------------------
 * WLAN_HAL_START_RSP
 *----------------------------------------------------------------------------*/

typedef PACKED_PRE struct PACKED_POST sHalMacStartRspParameters
{
	/*success or failure */
	u_int16_t  status;

	/*Max number of STA supported by the device*/
	u_int8_t     ucMaxStations;

	/*Max number of BSS supported by the device*/
	u_int8_t     ucMaxBssids;

	/*API Version */
	tWcnssWlanVersion wcnssWlanVersion;

	/*CRM build information */
	u_int8_t
		wcnssCrmVersionString[WLAN_HAL_VERSION_LENGTH];

	/*hardware/chipset/misc version information
	 * */
	u_int8_t
		wcnssWlanVersionString[WLAN_HAL_VERSION_LENGTH];

} tHalMacStartRspParams, *tpHalMacStartRspParams;

typedef PACKED_PRE struct PACKED_POST
{
	tHalMsgHeader header;
	tHalMacStartRspParams startRspParams;
}  tHalMacStartRspMsg, *tpHalMacStartRspMsg;

/*---------------------------------------------------------------------------
 * WLAN_HAL_DOWNLOAD_NV_RSP
 *--------------------------------------------------------------------------*/
typedef PACKED_PRE struct PACKED_POST
{
	/* Success or Failure. HAL would generate a WLAN_HAL_DOWNLOAD_NV_RSP
	 * after each fragment */
	u_int32_t   status;
} tHalNvImgDownloadRspParams, *tpHalNvImgDownloadRspParams;

typedef PACKED_PRE struct PACKED_POST
{
	tHalMsgHeader header;
	tHalNvImgDownloadRspParams nvImageRspParams;
}  tHalNvImgDownloadRspMsg, *tpHalNvImgDownloadRspMsg;

#endif

/*---------------------------------------------------------------------------
  WLAN_HAL_START_REQ
  ---------------------------------------------------------------------------*/

typedef PACKED_PRE struct PACKED_POST sHalMacStartParameter
{
	/* Drive Type - Production or FTM etc */
	tDriverType  driverType;

	/*Length of the config buffer*/
	tANI_U32  uConfigBufferLen;

	/* Following this there is a TLV formatted buffer of length
	 * "uConfigBufferLen" bytes containing all config values.
	 * The TLV is expected to be formatted like this:
	 * 0           15            31           31+CFG_LEN-1        length-1
	 * |   CFG_ID   |   CFG_LEN   |   CFG_BODY    |  CFG_ID  |......|
	 */
} tHalMacStartParameter, *tpHalMacStartParameter;

typedef PACKED_PRE struct PACKED_POST
{
   /* Note: The length specified in tHalMacStartReqMsg messages should be
    * header.msgLen = sizeof(tHalMacStartReqMsg) + uConfigBufferLen */
   tHalMsgHeader header;
   tHalMacStartParameter startReqParams;
}  tHalMacStartReqMsg, *tpHalMacStartReqMsg;

extern v_BOOL_t sys_validateStaConfig(void *pImage, unsigned long cbFile,
                               void **ppStaConfig, v_SIZE_t *pcbStaConfig);
extern void vos_WDAComplete_cback(v_PVOID_t pVosContext);

#ifdef QCA_WIFI_ISOC
VOS_STATUS wma_cfg_download_isoc(v_VOID_t *vos_context, 
		tp_wma_handle wma_handle);

VOS_STATUS wma_nv_download_start(v_VOID_t *handle);

VOS_STATUS wma_cfg_nv_get_hal_message_buffer(tp_wma_handle wma_handle,
		u_int16_t uReqType, u_int16_t usBufferLen,
		u_int8_t **pMsgBuffer, u_int16_t *pusDataOffset,
		u_int16_t *pusBufferSize);

VOS_STATUS wma_prepare_config_tlv(v_VOID_t *vos_context, 
		t_wma_start_req *wdi_start_params );

VOS_STATUS wma_htc_cfg_nv_connect_service(tp_wma_handle wma_handle);
VOS_STATUS wma_hal_stop_isoc(tp_wma_handle wma_handle);
#endif

/**
  * Frame index
  */
enum frame_index {
	GENERIC_NODOWNLD_NOACK_COMP_INDEX,
	GENERIC_DOWNLD_COMP_NOACK_COMP_INDEX,
	GENERIC_DOWNLD_COMP_ACK_COMP_INDEX,
	GENERIC_NODOWLOAD_ACK_COMP_INDEX = (OL_TXRX_MGMT_NUM_TYPES - 1) ,
	FRAME_INDEX_MAX
};

VOS_STATUS wma_update_vdev_tbl(tp_wma_handle wma_handle, u_int8_t vdev_id, 
		ol_txrx_vdev_handle tx_rx_vdev_handle, u_int8_t *mac, 
		u_int32_t vdev_type, bool add_del);
#ifndef QCA_WIFI_ISOC
int regdmn_get_country_alpha2(u_int16_t rd, u_int8_t *alpha2);
#endif

/*
 * Setting the Tx Comp Timeout to 1 secs.
 * TODO: Need to Revist the Timing
 */
#define WMA_TX_FRAME_COMPLETE_TIMEOUT  1000

struct wma_tx_ack_work_ctx {
	tp_wma_handle wma_handle;
	u_int16_t sub_type;
	int32_t status;
	struct work_struct ack_cmp_work;
};
#endif
