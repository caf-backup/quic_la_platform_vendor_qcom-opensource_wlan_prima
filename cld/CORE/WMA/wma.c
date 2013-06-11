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

  \file     wma.c
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
  12/03/2013        Ganesh        Implementation of WMA APIs.
                    Kondabattini
  27/03/2013        Ganesh        Rx Management Support added   
                    Babu
  ==========================================================================*/

/* ################ Header files ################ */
#include "wma.h"
#include "wma_api.h"
#include "vos_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wniApi.h"
#include "aniGlobal.h"
#include "wmi_unified.h"
#include "wniCfgAp.h"
#include "wlan_hal_cfg.h"
#include "cfgApi.h"
#include "ol_txrx_ctrl_api.h"
#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif
#include "wma_api.h"


#include "adf_nbuf.h"
#include "adf_os_types.h"
#include "ol_txrx_api.h"
#include "vos_memory.h"
#include "ol_txrx_types.h"

#include "wlan_qct_wda.h"
#include "limApi.h"

#include "wdi_out.h"
#include "wdi_in.h"

#include "vos_utils.h"

/* ################### defines ################### */
#define WMA_2_4_GHZ_MAX_FREQ  3000

#define WMA_DEFAULT_SCAN_PRIORITY            1
#define WMA_DEFAULT_SCAN_REQUESTER_ID        1

#ifdef BIG_ENDIAN_HOST

/* ############# function definitions ############ */

/* function   : wma_swap_bytes
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
v_VOID_t wma_swap_bytes(v_VOID_t *pv, v_SIZE_t n)
{
	v_SINT_t no_words;
	v_SINT_t i;
	v_U32_t *word_ptr;

	no_words =   n/sizeof(v_U32_t);
	word_ptr = (v_U32_t *)pv;
	for (i=0; i<no_words; i++) {
		*(word_ptr + i) = __cpu_to_le32(*(word_ptr + i));
	}
}
#define SWAPME(x, len) wma_swap_bytes(&x, len);
#endif

/* function   : wma_unified_debug_print_event_handler
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
static v_SINT_t wma_unified_debug_print_event_handler(v_VOID_t *handle,
						      v_U8_t *data,
						      v_U16_t datalen)
{
#ifdef BIG_ENDIAN_HOST
	v_U8_t dbgbuf[500] = {0};
	memcpy(dbgbuf, data, datalen);
	SWAPME(dbgbuf, datalen);
	WMA_LOGD("FIRMWARE:%s ", dbgbuf);
	return VOS_STATUS_SUCCESS;
#else
	WMA_LOGD("FIRMWARE:%s ", data);
	return VOS_STATUS_SUCCESS;
#endif
}

static v_VOID_t wma_set_default_tgt_config(tp_wma_handle wma_handle)
{
	wmi_resource_config tgt_cfg = {
		CFG_TGT_NUM_VDEV,
		CFG_TGT_NUM_PEERS + CFG_TGT_NUM_VDEV, /* reserve an additional peer for each VDEV */
		CFG_TGT_NUM_OFFLOAD_PEERS,
		CFG_TGT_NUM_PEER_KEYS,
		CFG_TGT_NUM_TIDS,
		CFG_TGT_AST_SKID_LIMIT,
		CFG_TGT_DEFAULT_TX_CHAIN_MASK,
		CFG_TGT_DEFAULT_RX_CHAIN_MASK,
		{ CFG_TGT_RX_TIMEOUT_LO_PRI, CFG_TGT_RX_TIMEOUT_LO_PRI, CFG_TGT_RX_TIMEOUT_LO_PRI, CFG_TGT_RX_TIMEOUT_HI_PRI },
		CFG_TGT_RX_DECAP_MODE,
		CFG_TGT_DEFAULT_SCAN_MAX_REQS,
		CFG_TGT_DEFAULT_BMISS_OFFLOAD_MAX_VDEV,
		CFG_TGT_DEFAULT_ROAM_OFFLOAD_MAX_VDEV,
		CFG_TGT_DEFAULT_ROAM_OFFLOAD_MAX_PROFILES,
		CFG_TGT_DEFAULT_NUM_MCAST_GROUPS,
		CFG_TGT_DEFAULT_NUM_MCAST_TABLE_ELEMS,
		CFG_TGT_DEFAULT_MCAST2UCAST_MODE,
		CFG_TGT_DEFAULT_TX_DBG_LOG_SIZE,
		CFG_TGT_WDS_ENTRIES,
		CFG_TGT_DEFAULT_DMA_BURST_SIZE,
		CFG_TGT_DEFAULT_MAC_AGGR_DELIM,
		CFG_TGT_DEFAULT_RX_SKIP_DEFRAG_TIMEOUT_DUP_DETECTION_CHECK,
		CFG_TGT_DEFAULT_VOW_CONFIG,
		CFG_TGT_DEFAULT_GTK_OFFLOAD_MAX_VDEV,
		CFG_TGT_NUM_MSDU_DESC
	};

	/* reduce the peer/vdev if CFG_TGT_NUM_MSDU_DESC exceeds 1000 */
#ifdef PERE_IP_HDR_ALIGNMENT_WAR
	if (scn->host_80211_enable) {
		/*
		 * To make the IP header begins at dword aligned address,
		 * we make the decapsulation mode as Native Wifi.
		 */
		tgt_cfg.rx_decap_mode = CFG_TGT_RX_DECAP_MODE_NWIFI;
	}
#endif
	wma_handle->wlan_resource_config = tgt_cfg;
}

/*
 * Allocate and init wmi adaptation layer.
 */
VOS_STATUS wma_open(v_VOID_t *vos_context, v_VOID_t *os_ctx,
		    tMacOpenParameters *mac_params)
{
	tp_wma_handle wma_handle;
	HTC_HANDLE htc_handle;
	adf_os_device_t adf_dev;
	v_VOID_t *wmi_handle;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

	WMA_LOGD("Enter");

	adf_dev = vos_get_context(VOS_MODULE_ID_ADF, vos_context);
	htc_handle = vos_get_context(VOS_MODULE_ID_HTC, vos_context);

	if (!htc_handle) {
		WMA_LOGP("\n Invalid HTC handle");
		return VOS_STATUS_E_INVAL;
	}

	/* Alloc memory for WMA Context */
	vos_status = vos_alloc_context(vos_context, VOS_MODULE_ID_WDA,
				       (v_VOID_t **) &wma_handle,
				       sizeof (t_wma_handle));

	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("Memory allocation failed for wma_handle");
		return VOS_STATUS_E_NOMEM;
	}

	vos_mem_zero(wma_handle, sizeof (t_wma_handle));

	/* attach the wmi */
	wmi_handle = wmi_unified_attach(wma_handle);
	if (!wmi_handle) {
		WMA_LOGP("failed to attach WMI");
		vos_status = VOS_STATUS_E_NOMEM;
		goto err_wmi_attach;
	}

	WMA_LOGA("WMA --> wmi_unified_attach - success");

	/* initialize default target config */
	wma_set_default_tgt_config(wma_handle);

	/* Allocate cfg handle */
	((pVosContextType) vos_context)->cfg_ctx =
		ol_pdev_cfg_attach(((pVosContextType) vos_context)->adf_ctx);
	if (!(((pVosContextType) vos_context)->cfg_ctx)) {
		WMA_LOGP("failed to init cfg handle");
		goto err_wmi_attach;
	}

	/* Save the WMI & HTC handle */
	wma_handle->wmi_handle = wmi_handle;
	wma_handle->htc_handle = htc_handle;
	wma_handle->vos_context = vos_context;
        wma_handle->adf_dev = adf_dev;

        /*TODO: Recheck below parameters */
	mac_params->maxStation = WMA_MAX_SUPPORTED_STAS;
        mac_params->maxBssId = WMA_MAX_SUPPORTED_BSS;
	mac_params->frameTransRequired = 0;

	wma_handle->max_station = mac_params->maxStation;
	wma_handle->max_bssid = mac_params->maxBssId;
	wma_handle->frame_xln_reqd = mac_params->frameTransRequired;
	wma_handle->driver_type = mac_params->driverType;

	/* Register the debug print event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_DEBUG_PRINT_EVENTID,
					   wma_unified_debug_print_event_handler);

#ifdef NOT_YET
	wma_handle->tgt_cfg_update_cb = tgt_cfg_cb;
#endif

#ifdef QCA_WIFI_ISOC
	vos_status = vos_event_init(&wma_handle->cfg_nv_tx_complete);
	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("cfg_nv_tx_complete initialization failed");
		goto err_event_init;
	}

	vos_status = vos_event_init(&(wma_handle->cfg_nv_rx_complete));
	if (VOS_STATUS_SUCCESS != vos_status) {
		WMA_LOGP("cfg_nv_tx_complete initialization failed");
		return VOS_STATUS_E_FAILURE;
	}
#endif
        vos_status = vos_event_init(&wma_handle->wma_ready_event);
	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("wma_ready_event initialization failed");
		goto err_event_init;
	}

	WMA_LOGD("Exit");

	return VOS_STATUS_SUCCESS;

err_event_init:
	wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
					     WMI_DEBUG_PRINT_EVENTID);
err_wmi_attach:
	vos_free_context(wma_handle->vos_context, VOS_MODULE_ID_WDA,
			 wma_handle);

	return vos_status;
}

/* function   : wma_pre_start    
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
VOS_STATUS wma_pre_start(v_VOID_t *vos_ctx)
{
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	A_STATUS status = A_OK;
	tp_wma_handle wma_handle;
	vos_msg_t wma_msg = {0} ;

	WMA_LOGD("Enter");

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	/* Validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("invalid argument");
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}
	/* Open endpoint for ctrl path - WMI <--> HTC */
	status = wmi_unified_connect_htc_service(
			wma_handle->wmi_handle, 
			wma_handle->htc_handle);	
	if (A_OK != status) {
		WMA_LOGP("wmi_unified_connect_htc_service");
		vos_status = VOS_STATUS_E_FAULT;
		goto end;
	}

	WMA_LOGA("WMA --> wmi_unified_connect_htc_service - success");

#ifdef QCA_WIFI_ISOC
	/* Open endpoint for cfg and nv download path - WMA <--> HTC */
	status = wma_htc_cfg_nv_connect_service(wma_handle);
	if (A_OK != status) {
		WMA_LOGP("\n htc_connect_service failed");
		vos_status = VOS_STATUS_E_FAULT;
		goto end;
	}
#endif
	/* Trigger the CFG DOWNLOAD */
	wma_msg.type = WNI_CFG_DNLD_REQ ;
	wma_msg.bodyptr = NULL;
	wma_msg.bodyval = 0;
	
	vos_status = vos_mq_post_message( VOS_MQ_ID_WDA, &wma_msg );
	if (VOS_STATUS_SUCCESS !=vos_status) {
		WMA_LOGP("Failed to post WNI_CFG_DNLD_REQ msg");
		VOS_ASSERT(0);
		vos_status = VOS_STATUS_E_FAILURE;
	}
end:
	WMA_LOGD("Exit");
	return vos_status;
}

/* function   : wma_send_msg
 * Descriptin :
 * Args       :
 * Retruns    :
 */
static void wma_send_msg(tp_wma_handle wma_handle, u_int16_t msg_type,
		void *body_ptr, u_int32_t body_val)
{
	tSirMsgQ msg = {0} ;
	tANI_U32 status = VOS_STATUS_SUCCESS ;
	tpAniSirGlobal pMac = (tpAniSirGlobal )vos_get_context(VOS_MODULE_ID_PE,
			wma_handle->vos_context);
	msg.type        = msg_type;
	msg.bodyval     = body_val;
	msg.bodyptr     = body_ptr;
	status = limPostMsgApi(pMac, &msg);
	if (VOS_STATUS_SUCCESS != status) {
		if(NULL != body_ptr) 
			vos_mem_free(body_ptr);
		VOS_ASSERT(0) ;
	}
	return ;
}

/* function   : wma_get_txrx_vdev_type
 * Descriptin :
 * Args       :
 * Retruns    :
 */
enum wlan_op_mode wma_get_txrx_vdev_type(u_int32_t type)
{
	enum wlan_op_mode vdev_type = wlan_op_mode_unknown;
	switch (type) {
		case WMI_VDEV_TYPE_AP:
			vdev_type = wlan_op_mode_ap;
			break;
		case WMI_VDEV_TYPE_STA:
			vdev_type = wlan_op_mode_sta;
			break;
		case WMI_VDEV_TYPE_IBSS:
		case WMI_VDEV_TYPE_MONITOR:
		default:
			WMA_LOGE("Invalid vdev type %u", type);
			vdev_type = wlan_op_mode_unknown;
	}

	return vdev_type;
}

/* function   : wma_unified_vdev_create_send
 * Descriptin :
 * Args       :
 * Retruns    :
 */
int wma_unified_vdev_create_send(wmi_unified_t wmi_handle, u_int8_t if_id,
				 u_int32_t type, u_int32_t subtype,
				 u_int8_t macaddr[IEEE80211_ADDR_LEN])
{
	wmi_vdev_create_cmd* cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_create_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed\n", __FUNCTION__);
		return ENOMEM;
	}
	cmd = (wmi_vdev_create_cmd *) wmi_buf_data(buf);
	cmd->vdev_id = if_id;
	cmd->vdev_type = type;
	cmd->vdev_subtype = subtype;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	WMA_LOGA("%s: ID = %d VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x:\n",
		 __func__, if_id,
		 macaddr[0], macaddr[1], macaddr[2],
		 macaddr[3], macaddr[4], macaddr[5]);
	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
}

/* function   : wma_unified_vdev_delete_send
 * Descriptin :
 * Args       :
 * Retruns    :
 */
static int wma_unified_vdev_delete_send(wmi_unified_t wmi_handle, u_int8_t if_id)
{
	wmi_vdev_delete_cmd* cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_vdev_delete_cmd));
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed\n", __FUNCTION__);
		return ENOMEM;
	}

	cmd = (wmi_vdev_delete_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = if_id;
	return wmi_unified_cmd_send(wmi_handle, buf, sizeof(wmi_vdev_delete_cmd),
			WMI_VDEV_DELETE_CMDID);
}

/* function   : wma_vdev_detach
 * Descriptin :
 * Args       :
 * Retruns    :
 */
static VOS_STATUS wma_vdev_detach(tp_wma_handle wma_handle,
				tpDelStaSelfParams pdel_sta_self_req_param)
{
	VOS_STATUS status = VOS_STATUS_SUCCESS;
	ol_txrx_pdev_handle txrx_pdev = vos_get_context(VOS_MODULE_ID_TXRX,
						wma_handle->vos_context);
	void *txrx_hdl;

	/* remove the interface from ath_dev */
	if (wma_unified_vdev_delete_send(wma_handle->wmi_handle, 
			pdel_sta_self_req_param->sessionId)) {
		WMA_LOGP("Unable to remove an interface for ath_dev.\n");
		status = VOS_STATUS_E_FAILURE;
	}

	txrx_hdl = wdi_in_get_vdev(txrx_pdev,
					pdel_sta_self_req_param->sessionId);
	if(!txrx_hdl)
		status = VOS_STATUS_E_FAILURE;
	else
		ol_txrx_vdev_detach(txrx_hdl, NULL, NULL);

	WMA_LOGA("vdev_id:%hu vdev_hdl:%p\n", pdel_sta_self_req_param->sessionId,
			txrx_hdl);

	wma_send_msg(wma_handle, WDA_DEL_STA_SELF_RSP, (void *)pdel_sta_self_req_param, 0);
	return status;
}

/* function   : wma_vdev_attach
 * Descriptin :
 * Args       :
 * Retruns    :
 */
static VOS_STATUS wma_vdev_attach(tp_wma_handle wma_handle, tpAddStaSelfParams self_sta_req)
{
	ol_txrx_vdev_handle txrx_vdev_handle = NULL;
	ol_txrx_pdev_handle txrx_pdev = vos_get_context(VOS_MODULE_ID_TXRX,
			wma_handle->vos_context);
	enum wlan_op_mode txrx_vdev_type;
	VOS_STATUS status = VOS_STATUS_SUCCESS;

	/* Create a vdev in target */
	if (wma_unified_vdev_create_send(wma_handle->wmi_handle,
						self_sta_req->sessionId,
						self_sta_req->type,
						self_sta_req->subType,
						self_sta_req->selfMacAddr))
	{
		WMA_LOGP("Unable to add an interface for ath_dev.\n");
		status = VOS_STATUS_E_RESOURCES;
		goto end;
	}

	txrx_vdev_type = wma_get_txrx_vdev_type(self_sta_req->type);

	if (wlan_op_mode_unknown == txrx_vdev_type) {
		WMA_LOGE("Failed to get txrx vdev type");
		wma_unified_vdev_delete_send(wma_handle->wmi_handle,
						self_sta_req->sessionId);
		goto end;
	}

	txrx_vdev_handle = ol_txrx_vdev_attach(txrx_pdev,
						self_sta_req->selfMacAddr,
						self_sta_req->sessionId,
						txrx_vdev_type);

	WMA_LOGA("vdev_id %hu, txrx_vdev_handle = %p", self_sta_req->sessionId,
			txrx_vdev_handle);

	if (NULL == txrx_vdev_handle) {
		WMA_LOGP("ol_txrx_vdev_attach failed");
		status = VOS_STATUS_E_FAILURE;
		wma_unified_vdev_delete_send(wma_handle->wmi_handle,
						self_sta_req->sessionId);
		goto end;
	}

end:
	self_sta_req->status = status;
	wma_send_msg(wma_handle, WDA_ADD_STA_SELF_RSP, (void *)self_sta_req, 0);
	return status;
}

static VOS_STATUS wma_wni_cfg_dnld(tp_wma_handle wma_handle)
{
	VOS_STATUS vos_status = VOS_STATUS_E_FAILURE;
	v_VOID_t *file_img = NULL;
	v_SIZE_t file_img_sz = 0;
	v_VOID_t *cfg_bin = NULL;
	v_SIZE_t cfg_bin_sz = 0;
	v_BOOL_t status = VOS_FALSE;
	v_VOID_t *mac = vos_get_context(VOS_MODULE_ID_PE,
			wma_handle->vos_context);

	WMA_LOGD("Enter");

	if (NULL == mac) {
		WMA_LOGP("Invalid context");
		VOS_ASSERT(0);
		return VOS_STATUS_E_FAILURE;
	}

	/* get the number of bytes in the CFG Binary... */
	vos_status = vos_get_binary_blob(VOS_BINARY_ID_CONFIG, NULL,
			&file_img_sz);
	if (VOS_STATUS_E_NOMEM != vos_status) {
		WMA_LOGP("Error in obtaining the binary size");
		goto fail;
	}

	/* malloc a buffer to read in the Configuration binary file. */
	file_img = vos_mem_malloc(file_img_sz);
	if (NULL == file_img) {
		WMA_LOGP("Unable to allocate memory for the CFG binary"
				"[size= %d bytes]", file_img_sz);
		vos_status = VOS_STATUS_E_NOMEM;
		goto fail;
	}

	/* Get the entire CFG file image. */
	vos_status = vos_get_binary_blob(VOS_BINARY_ID_CONFIG, file_img,
			&file_img_sz);
	if (VOS_STATUS_SUCCESS != vos_status) {
		WMA_LOGP("Error: Cannot retrieve CFG file image from vOSS."
				"[size= %d bytes]", file_img_sz);
		goto fail;
	}

	/*
	 * Validate the binary image.  This function will return a pointer
	 * and length where the CFG binary is located within the binary image file.
	 */
	status = sys_validateStaConfig( file_img, file_img_sz,
			&cfg_bin, &cfg_bin_sz );
	if ( VOS_FALSE == status )
	{
		WMA_LOGP("Error: Cannot find STA CFG in binary image file.");
		vos_status = VOS_STATUS_E_FAILURE;
		goto fail;
	}
	/*
	 * TODO: call the config download function
	 * for now calling the existing cfg download API
	 */
	processCfgDownloadReq(mac, cfg_bin_sz, cfg_bin);
	if (file_img != NULL) {
		vos_mem_free(file_img);
	}

	WMA_LOGD("Exit");
	return vos_status;

fail:
	if(cfg_bin != NULL)
		vos_mem_free( file_img );

	WMA_LOGD("Exit");
	return vos_status;
}

/* function   : wma_get_buf_start_scan_cmd
 * Descriptin :
 * Args       :
 * Retruns    :
 */
VOS_STATUS wma_get_buf_start_scan_cmd(tp_wma_handle wma_handle,
					tSirScanOffloadReq *scan_req,
					wmi_buf_t *buf,
					int *buf_len)
{
	wmi_start_scan_cmd *cmd;
	wmi_chan_list *chan_list = NULL;
	wmi_bssid_list *bssid_list = NULL;
	wmi_ssid_list *ssid_list = NULL;
	wmi_ie_data *ie_data;
	u_int32_t *tmp_ptr;
	VOS_STATUS vos_status;
	int align, i;
	int len = sizeof(wmi_start_scan_cmd);

	/* calculate the length of buffer required */
	if (scan_req->channelList.numChannels) {
		len += sizeof(wmi_chan_list) +
			(scan_req->channelList.numChannels - 1) *
			(sizeof(u_int32_t));
	}

	if (scan_req->numSsid) {
		len += sizeof(wmi_ssid_list) +
			(scan_req->numSsid - 1) * sizeof(wmi_ssid);
	}

	len += sizeof(wmi_bssid_list);

	if (scan_req->uIEFieldLen) {
		align = scan_req->uIEFieldLen % sizeof(u_int32_t);
		len += sizeof(u_int32_t) - align;
		len += 2 * sizeof(u_int32_t) + scan_req->uIEFieldLen;
	}

	/* Allocate the memory */
	*buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!*buf) {
		WMA_LOGP("failed to allocate memory for start scan cmd");
		vos_status = VOS_STATUS_E_FAILURE;
		goto error;
	}

	cmd = (wmi_start_scan_cmd *) wmi_buf_data(*buf);

	cmd->vdev_id = scan_req->sessionId;
	/*TODO: Populate actual values */
	cmd->scan_id = ++wma_handle->scan_id;
	cmd->scan_priority = WMA_DEFAULT_SCAN_PRIORITY;
	cmd->scan_req_id = WMA_DEFAULT_SCAN_REQUESTER_ID;

	/* Set the scan events which the driver is intereseted to receive */
	/* TODO: handle all the other flags also */
	cmd->notify_scan_events = WMI_SCAN_EVENT_STARTED |
				WMI_SCAN_EVENT_START_FAILED |
				WMI_SCAN_EVENT_COMPLETED;

	cmd->dwell_time_active = scan_req->maxChannelTime;
	cmd->dwell_time_passive = scan_req->maxChannelTime;

	if (scan_req->scanType == eSIR_PASSIVE_SCAN)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_PASSIVE;

	cmd->max_scan_time = WMA_HW_DEF_SCAN_MAX_DURATION;
	cmd->scan_ctrl_flags |= WMI_SCAN_ADD_OFDM_RATES;

	/* if p2pSearch then disable the 11b rates */
	if (!scan_req->p2pSearch) {
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_CCK_RATES;
		cmd->scan_ctrl_flags |= WMI_SCAN_FILTER_PROBE_REQ;
	}

	cmd->scan_ctrl_flags |= WMI_SCAN_ADD_BCAST_PROBE_REQ;

	tmp_ptr = (u_int32_t *) (cmd + 1);

	if (scan_req->channelList.numChannels) {
		chan_list  = (wmi_chan_list *) tmp_ptr;
		chan_list->tag = WMI_CHAN_LIST_TAG;
		chan_list->num_chan = scan_req->channelList.numChannels;
		for (i = 0; i < scan_req->channelList.numChannels; ++i) {
			chan_list->channel_list[i] =
				vos_chan_to_freq(
					scan_req->channelList.channelNumber[i]);
		}
		tmp_ptr += (2 + scan_req->channelList.numChannels);
	}

	if (scan_req->numSsid) {
		ssid_list = (wmi_ssid_list *) tmp_ptr;
		ssid_list->tag = WMI_SSID_LIST_TAG;
		ssid_list->num_ssids = scan_req->numSsid;
		for (i = 0; i < scan_req->numSsid; ++i) {
			ssid_list->ssids[i].ssid_len = scan_req->ssId[i].length;
			vos_mem_copy(ssid_list->ssids[i].ssid,
					scan_req->ssId[i].ssId,
					scan_req->ssId[i].length);
		}
		tmp_ptr += (2 + (sizeof(wmi_ssid) *
					scan_req->numSsid) / sizeof(u_int32_t));
	}

	bssid_list  = (wmi_bssid_list *) tmp_ptr;
	bssid_list->tag = WMI_BSSID_LIST_TAG;
	bssid_list->num_bssid = 1;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(scan_req->bssId, &bssid_list->bssid_list[0]);

	tmp_ptr += (2 + (sizeof(wmi_mac_addr) *
				bssid_list->num_bssid) / sizeof(u_int32_t));

	if (scan_req->uIEFieldLen) {
		ie_data  = (wmi_ie_data *) tmp_ptr;
		ie_data->tag = WMI_IE_TAG;
		ie_data->ie_len = scan_req->uIEFieldLen;
		vos_mem_copy(ie_data->ie_data,
				(u_int8_t *)scan_req +
				(scan_req->uIEFieldOffset),
				scan_req->uIEFieldLen);
	}

	*buf_len = len;
	vos_status = VOS_STATUS_SUCCESS;
error:
	return vos_status;
}

/* function   : wma_start_scan
 * Descriptin :
 * Args       :
 * Retruns    :
 */
VOS_STATUS wma_start_scan(tp_wma_handle wma_handle,
			tSirScanOffloadReq *scan_req)
{
	VOS_STATUS vos_status;
	wmi_buf_t buf;
	int status = 0;
	int len;

	/* Fill individual elements of wmi_start_scan_req and
	 * TLV for channel list, bssid, ssid etc ... */
	vos_status = wma_get_buf_start_scan_cmd(wma_handle, scan_req,
			&buf, &len);

	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGE("Failed to get buffer for start scan cmd");
		goto error1;
	}
	status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
			len, WMI_START_SCAN_CMDID);
	/* Call the wmi api to request the scan */
	if (0 != status) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d",
			status);
		vos_status = VOS_STATUS_E_FAILURE;
		goto error;
	}
	return VOS_STATUS_SUCCESS;
error:
	if (buf)
		adf_nbuf_free(buf);
error1:
	return vos_status;
}

/* function   : wma_update_channel_list
 * Descriptin : Function is used to update the support channel list
 * Args       : wma_handle, list of supported channels and power
 * Retruns    : SUCCESS or FAILURE
 */
VOS_STATUS wma_update_channel_list(WMA_HANDLE handle,
				tSirUpdateChanList *chan_list)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_buf_t buf;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	wmi_scan_chan_list_cmd *cmd;
	int status, len, i;

	len = sizeof(wmi_scan_chan_list_cmd)+
		(sizeof(wmi_channel) * (chan_list->numChan - 1));

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("Failed to allocate memory");
		vos_status = VOS_STATUS_E_NOMEM;
		goto end;
	}

	cmd = (wmi_scan_chan_list_cmd *) wmi_buf_data(buf);

	WMA_LOGD("no of channels = %d, len = %d", chan_list->numChan, len);

	cmd->num_scan_chans = chan_list->numChan;
	vos_mem_zero(cmd->chan_info,
			sizeof(wmi_channel) * cmd->num_scan_chans);

	for (i = 0; i < chan_list->numChan; ++i) {
		cmd->chan_info[i].mhz =
			vos_chan_to_freq(chan_list->chanParam[i].chanId);
		cmd->chan_info[i].band_center_freq1 = cmd->chan_info[i].mhz;
		cmd->chan_info[i].band_center_freq2 = 0;

		WMA_LOGD("chan[%d] = %u", i, cmd->chan_info[i].mhz);

		if (cmd->chan_info[i].mhz < WMA_2_4_GHZ_MAX_FREQ) {
			WMI_SET_CHANNEL_MODE(&cmd->chan_info[i],
					MODE_11G);
		} else {
			WMI_SET_CHANNEL_MODE(&cmd->chan_info[i],
					MODE_11A);
		}

		WMI_SET_CHANNEL_MAX_POWER(&cmd->chan_info[i],
				chan_list->chanParam[i].pwr);

		WMI_SET_CHANNEL_REG_POWER(&cmd->chan_info[i],
				chan_list->chanParam[i].pwr);
		/*TODO: Set WMI_SET_CHANNEL_MIN_POWER */
		/*TODO: Set WMI_SET_CHANNEL_ANTENNA_MAX */
		/*TODO: WMI_SET_CHANNEL_REG_CLASSID*/
	}

	status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
			WMI_SCAN_CHAN_LIST_CMDID);

	if (status != 0) {
		vos_status = VOS_STATUS_E_FAILURE;
		WMA_LOGE("Failed to send the WMI_SCAN_CHAN_LIST_CMDID");
	}
end:
	return vos_status;
}

/* function   : wma_mc_process_msg
 * Descriptin :
 * Args       :
 * Retruns    :
 */
VOS_STATUS wma_mc_process_msg(v_VOID_t *vos_context, vos_msg_t *msg)
{
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	tp_wma_handle wma_handle;

	WMA_LOGD("Enter");
	if(NULL == msg)	{
		WMA_LOGE("msg is NULL");
		VOS_ASSERT(0);
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}

	WMA_LOGD("msg->type = %x", msg->type);

	wma_handle = (tp_wma_handle) vos_get_context(VOS_MODULE_ID_WDA,
			vos_context);
	
	if (NULL == wma_handle) {
		WMA_LOGP("wma_handle is NULL");
		VOS_ASSERT(0);
		vos_mem_free(msg->bodyptr);
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}

	switch (msg->type) {
		case WNI_CFG_DNLD_REQ:
			WMA_LOGA("McThread: WNI_CFG_DNLD_REQ");
			vos_status = wma_wni_cfg_dnld(wma_handle);
			if (VOS_IS_STATUS_SUCCESS(vos_status)) {
				vos_WDAComplete_cback(vos_context);
			}
			else {
				WMA_LOGD("config download failure");
			}
			break ;
		case WDA_ADD_STA_SELF_REQ:
			wma_vdev_attach(wma_handle, (tAddStaSelfParams *)msg->bodyptr);
			break;
		case WDA_DEL_STA_SELF_REQ:
			wma_vdev_detach(wma_handle, (tDelStaSelfParams *)msg->bodyptr);
			break;
		case WDA_START_SCAN_OFFLOAD_REQ:
			wma_start_scan(wma_handle, msg->bodyptr);
			break;
		case WDA_UPDATE_CHAN_LIST_REQ:
			wma_update_channel_list(wma_handle,
					(tSirUpdateChanList *)msg->bodyptr);
			break;
		default:
			WMA_LOGD("unknow msg type %x", msg->type);
			/* Do Nothing? MSG Body should be freed at here */
			if(NULL != msg->bodyptr) {
				vos_mem_free(msg->bodyptr);
			}
	}
end:
	WMA_LOGD("Exit");
	return vos_status ;
}

static int wma_scan_event_callback(WMA_HANDLE handle, u_int8_t *event_buf,
                                    u_int16_t len)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_scan_event *wmi_event = (wmi_scan_event *) event_buf;
	tSirScanOffloadEvent *scan_event;
	scan_event = (tSirScanOffloadEvent *) vos_mem_malloc
                                (sizeof(tSirScanOffloadEvent));
	if (!scan_event) {
		WMA_LOGE("Memory allocation failed for tSirScanOffloadEvent");
		return -ENOMEM;
	}

	scan_event->event = wmi_event->event;
	scan_event->scanId = wmi_event->scan_id;
	scan_event->chanFreq = wmi_event->channel_freq;

	if (wmi_event->reason == WMI_SCAN_REASON_COMPLETED)
		scan_event->reasonCode = eSIR_SME_SUCCESS;
	else
		scan_event->reasonCode = eSIR_SME_SCAN_FAILED;

	WMA_LOGD("Received SCAN Event, Posting msg WMA_RX_SCAN_EVENT");
	wma_send_msg(wma_handle, WDA_RX_SCAN_EVENT, (void *) scan_event, 0) ;
	return 0;
}

/* function   : wma_start    
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
VOS_STATUS wma_start(v_VOID_t *vos_ctx)
{
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	tp_wma_handle wma_handle;
	int status;
	WMA_LOGD("Enter");

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("Invalid handle");
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}

#ifdef QCA_WIFI_ISOC
	vos_event_reset(&wma_handle->wma_ready_event);

	/* start cfg download to soc */
	vos_status = wma_cfg_download_isoc(wma_handle->vos_context, wma_handle);
	if (vos_status != 0) {
		WMA_LOGP("failed to download the cfg to FW");
		vos_status = VOS_STATUS_E_FAILURE;
		goto end;
	}

	/* wait until WMI_READY_EVENTID received from FW */
	vos_status = wma_wait_for_ready_event(wma_handle);
	if (vos_status == VOS_STATUS_E_FAILURE)
		goto end;
#endif

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_SCAN_EVENTID,
						wma_scan_event_callback);
	if (0 != status) {
		WMA_LOGP("Failed to register scan callback");
		vos_status = VOS_STATUS_E_FAILURE;
		goto end;
	}
	vos_status = VOS_STATUS_SUCCESS;

end:
	WMA_LOGD("Exit");
	return vos_status;
}

/* function   : wma_stop
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
VOS_STATUS wma_stop(v_VOID_t *vos_ctx, tANI_U8 reason)
{
	tp_wma_handle wma_handle;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	WMA_LOGD("Enter");

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("Invalid handle");
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}

#ifdef QCA_WIFI_ISOC
	wma_hal_stop_isoc(wma_handle);
#endif
end:
	WMA_LOGD("Exit");
	return vos_status;
}

/* function   : wma_close
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
VOS_STATUS wma_close(v_VOID_t *vos_ctx)
{
	tp_wma_handle wma_handle;
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	u_int32_t idx;
#endif
	WMA_LOGD("Enter");

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("Invalid handle");
		return VOS_STATUS_E_INVAL;
	}

	/* close the vos events */
	vos_event_destroy(&wma_handle->wma_ready_event);
#ifdef QCA_WIFI_ISOC
	vos_event_destroy(&wma_handle->cfg_nv_tx_complete);
#endif
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	for(idx = 0; idx < wma_handle->num_mem_chunks; ++idx) {
		adf_os_mem_free_consistent(
				wma_handle->adf_dev,
				wma_handle->mem_chunks[idx].len,
				wma_handle->mem_chunks[idx].vaddr,
				wma_handle->mem_chunks[idx].paddr,
				adf_os_get_dma_mem_context(
					(&(wma_handle->mem_chunks[idx])),
					memctx));
	}
#endif

	/* dettach the wmi serice */
	if (wma_handle->wmi_handle) {
		WMA_LOGD("calling wmi_unified_detach");
		wmi_unified_detach(wma_handle->wmi_handle);
		wma_handle->wmi_handle = NULL;
	}
	/* free the wma_handle */
	vos_free_context(wma_handle->vos_context, VOS_MODULE_ID_WDA, wma_handle);

	adf_os_mem_free(((pVosContextType) vos_ctx)->cfg_ctx);

	WMA_LOGD("Exit");
	return VOS_STATUS_SUCCESS;
}

static v_VOID_t wma_update_fw_config(tp_wma_handle wma_handle,
				     struct wma_target_cap *tgt_cap)
{
	/*
	 * tgt_cap contains default target resource configuration
	 * which can be modified here, if required
	 */
	/* Override the no. of max fragments as per platform configuration */
	tgt_cap->wlan_resource_config.max_frag_entries =
		MIN(QCA_OL_11AC_TX_MAX_FRAGS, wma_handle->max_frag_entry);
	wma_handle->max_frag_entry = tgt_cap->wlan_resource_config.max_frag_entries;
}

#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
/**
 * allocate a chunk of memory at the index indicated and 
 * if allocation fail allocate smallest size possiblr and
 * return number of units allocated.
 */
static u_int32_t wma_alloc_host_mem_chunk(tp_wma_handle wma_handle,
					  u_int32_t req_id, u_int32_t idx,
					  u_int32_t num_units,
					  u_int32_t unit_len)
{
	adf_os_dma_addr_t paddr;
	if (!num_units  || !unit_len)  {
		return 0;
	}
	wma_handle->mem_chunks[idx].vaddr = NULL ;
	/** reduce the requested allocation by half until allocation succeeds */
	while(wma_handle->mem_chunks[idx].vaddr == NULL && num_units ) {
		wma_handle->mem_chunks[idx].vaddr = adf_os_mem_alloc_consistent(
				wma_handle->adf_dev, num_units*unit_len, &paddr,
				adf_os_get_dma_mem_context(
					(&(wma_handle->mem_chunks[idx])),
					memctx));
		if(wma_handle->mem_chunks[idx].vaddr == NULL) {
			num_units = (num_units >> 1) ; /* reduce length by half */
		} else {
			wma_handle->mem_chunks[idx].paddr = paddr;
			wma_handle->mem_chunks[idx].len = num_units*unit_len;
			wma_handle->mem_chunks[idx].req_id =  req_id;
		}
	}
	return num_units;
}

#define HOST_MEM_SIZE_UNIT 4
/*
 * allocate amount of memory requested by FW.
 */
static void wma_alloc_host_mem(tp_wma_handle wma_handle, u_int32_t req_id,
				u_int32_t num_units, u_int32_t unit_len)
{
	u_int32_t remaining_units,allocated_units, idx;

	/* adjust the length to nearest multiple of unit size */
	unit_len = (unit_len + (HOST_MEM_SIZE_UNIT - 1)) &
			(~(HOST_MEM_SIZE_UNIT - 1));
	idx = wma_handle->num_mem_chunks ;
	remaining_units = num_units;
	while(remaining_units) {
		allocated_units = wma_alloc_host_mem_chunk(wma_handle, req_id,
							   idx, remaining_units,
							   unit_len);
		if (allocated_units == 0) {
			printk("FAILED TO ALLOCATED memory unit len %d\
				units requested %d units allocated %d \n",
				unit_len, num_units,
				(num_units - remaining_units));
			wma_handle->num_mem_chunks = idx;
			break;
		}
		remaining_units -= allocated_units;
		++idx;
		if (idx == MAX_MEM_CHUNKS ) {
			printk("RWACHED MAX CHUNK LIMIT for memory units %d\
				unit len %d requested by FW, only allocated %d \n",
				num_units,unit_len,
				(num_units - remaining_units));
			wma_handle->num_mem_chunks = idx;
			break;
		}
	}
	wma_handle->num_mem_chunks = idx;
}
#endif

#ifdef NOT_YET
#ifndef QCA_WIFI_ISOC
static void wma_update_hdd_cfg(tp_wma_handle wma_handle)
{
	struct hdd_tgt_cfg hdd_tgt_cfg;
	int err;
	void *hdd_ctx = vos_get_context(VOS_MODULE_ID_HDD,
					wma_handle->vos_context);

	err = regdmn_get_country_alpha2(wma_handle->reg_cap.eeprom_rd,
					hdd_tgt_cfg.alpha2);
	if (err) {
		WMA_LOGE("Invalid regulatory settings");
		return;
	}

	switch (wma_handle->phy_capability) {
	case WMI_11G_CAPABILITY:
	case WMI_11NG_CAPABILITY:
		hdd_tgt_cfg.band_cap = eCSR_BAND_24;
		break;
	case WMI_11A_CAPABILITY:
	case WMI_11NA_CAPABILITY:
	case WMI_11AC_CAPABILITY:
		hdd_tgt_cfg.band_cap = eCSR_BAND_5G;
		break;
	case WMI_11AG_CAPABILITY:
	case WMI_11NAG_CAPABILITY:
	default:
		hdd_tgt_cfg.band_cap = eCSR_BAND_ALL;
	}

	adf_os_mem_copy(hdd_tgt_cfg.hw_macaddr.bytes, wma_handle->hwaddr,
			ATH_MAC_LEN);
	wma_handle->tgt_cfg_update_cb(hdd_ctx, &hdd_tgt_cfg);
}
#endif
#endif

static wmi_buf_t wma_setup_wmi_init_msg(tp_wma_handle wma_handle,
					wmi_service_ready_event *ev,
					v_SIZE_t *len)
{
	wmi_buf_t buf;
	wmi_init_cmd *cmd;
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	u_int16_t idx;
	u_int32_t num_units;
#endif

	*len = sizeof(wmi_init_cmd);
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	*len += (sizeof(wlan_host_memory_chunk) * MAX_MEM_CHUNKS);
#endif
	buf = wmi_buf_alloc(wma_handle->wmi_handle, *len);
	if (!buf) {
		WMA_LOGP("wmi_buf_alloc failed");
		return NULL;
	}

	cmd = (wmi_init_cmd *) wmi_buf_data(buf);
	cmd->resource_config = wma_handle->wlan_resource_config;

	/* allocate memory requested by FW */
	if (ev->num_mem_reqs > WMI_MAX_MEM_REQS) {
		VOS_ASSERT(0);
		adf_nbuf_free(buf);
		return NULL;
	}

	cmd->num_host_mem_chunks = 0;
#if !defined(QCA_WIFI_ISOC) && !defined(CONFIG_HL_SUPPORT)
	for(idx = 0; idx < ev->num_mem_reqs; ++idx) {
		num_units = ev->mem_reqs[idx].num_units;
		if  (ev->mem_reqs[idx].num_unit_info & NUM_UNITS_IS_NUM_PEERS) {
			/*
			 * number of units to allocate is number
			 * of peers, 1 extra for self peer on
			 * target. this needs to be fied, host
			 * and target can get out of sync
			 */
			num_units = cmd->resource_config.num_peers + 1;
		}
		WMA_LOGD("idx %d req %d  num_units %d num_unit_info %d unit size %d actual units %d \n",
			 idx, ev->mem_reqs[idx].req_id,
			 ev->mem_reqs[idx].num_units,
			 ev->mem_reqs[idx].num_unit_info,
			 ev->mem_reqs[idx].unit_size,
			 num_units);
		wma_alloc_host_mem(wma_handle, ev->mem_reqs[idx].req_id,
				   num_units, ev->mem_reqs[idx].unit_size);
	}
	for(idx = 0; idx < wma_handle->num_mem_chunks; ++idx) {
		cmd->host_mem_chunks[idx].ptr = wma_handle->mem_chunks[idx].paddr;
		cmd->host_mem_chunks[idx].size = wma_handle->mem_chunks[idx].len;
		cmd->host_mem_chunks[idx].req_id = wma_handle->mem_chunks[idx].req_id;
		WMA_LOGD("chunk %d len %d requested ,ptr  0x%x \n",
			 idx, cmd->host_mem_chunks[idx].size,
			 cmd->host_mem_chunks[idx].ptr) ;
	}
	cmd->num_host_mem_chunks = wma_handle->num_mem_chunks;
	if (wma_handle->num_mem_chunks > 1) {
		*len += ((wma_handle->num_mem_chunks - 1) *
		        sizeof(wlan_host_memory_chunk));
	}
#endif
	return buf;
}

/* Process service ready event and send wmi_init command */
v_VOID_t wma_rx_service_ready_event(WMA_HANDLE handle,
				    wmi_service_ready_event *ev)
{
	wmi_buf_t buf;
	v_SIZE_t len;
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct wma_target_cap target_cap;

	WMA_LOGD("Enter");

	if (!handle || !ev) {
		WMA_LOGP("Invalid arguments");
		return;
	}

	WMA_LOGA("WMA <-- WMI_SERVICE_READY_EVENTID");

	wma_handle->phy_capability = ev->phy_capability;
	wma_handle->max_frag_entry = ev->max_frag_entry;
	vos_mem_copy(&wma_handle->reg_cap, &ev->hal_reg_capabilities,
		     sizeof(HAL_REG_CAPABILITIES));

	 /* TODO: Recheck below line to dump service ready event */
	 /* dbg_print_wmi_service_11ac(ev); */

	/* wmi service is ready */
	vos_mem_copy(wma_handle->wmi_service_bitmap, ev->wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));
	vos_mem_copy(target_cap.wmi_service_bitmap, ev->wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));
	target_cap.wlan_resource_config = wma_handle->wlan_resource_config;
	wma_update_fw_config(wma_handle, &target_cap);
	vos_mem_copy(wma_handle->wmi_service_bitmap, target_cap.wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));
	wma_handle->wlan_resource_config = target_cap.wlan_resource_config;

	buf = wma_setup_wmi_init_msg(wma_handle, ev, &len);
	if (!buf) {
		WMA_LOGE("Failed to setup buffer for wma init command");
		return;
	}

	WMA_LOGA("WMA --> WMI_INIT_CMDID");
	wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len, WMI_INIT_CMDID);
}

/* function   : wma_rx_ready_event
 * Descriptin :  
 * Args       :        
 * Retruns    :     
 */
v_VOID_t wma_rx_ready_event(WMA_HANDLE handle, wmi_ready_event *ev)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;

	WMA_LOGD("Enter");

	if ((NULL == wma_handle) || (NULL == ev)) {
		WMA_LOGP("Invalid arguments");
		VOS_ASSERT(0);
		return;
	}
	
	WMA_LOGA("WMA <-- WMI_READY_EVENTID");

	wma_handle->version.wlan_ver = ev->sw_version;
	wma_handle->version.abi_ver = ev->abi_version;

	/* Indicate to the waiting thread that the ready
	 * event was received */
	wma_handle->wmi_ready = TRUE;
	wma_handle->wlan_init_status = ev->status;

	/* copy the mac addr */
	WMI_MAC_ADDR_TO_CHAR_ARRAY (&ev->mac_addr, wma_handle->myaddr);
	WMI_MAC_ADDR_TO_CHAR_ARRAY (&ev->mac_addr, wma_handle->hwaddr);

	vos_event_set(&wma_handle->wma_ready_event);

#ifdef NOT_YET
#ifndef QCA_WIFI_ISOC
	wma_update_hdd_cfg(wma_handle);
#endif
#endif

	WMA_LOGD("Exit");
}

/**
  * wma_mgmt_tx_dload_comp_hldr - handles tx mgmt completion
  * @context: context with which the handler is registered
  * @netbuf: tx mgmt nbuf
  * @err: status of tx completion
  */
static void
wma_mgmt_tx_dload_comp_hldr(void *mac_context, adf_nbuf_t netbuf,
					int32_t status)
{
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

	void *vos_context = vos_get_global_context(VOS_MODULE_ID_PE,
							mac_context);
	tp_wma_handle wma_handle = (tp_wma_handle)vos_get_context(
						VOS_MODULE_ID_WDA, vos_context);

	WMA_LOGD("Tx Complete Status %d", status);

	if (!wma_handle->tx_frm_download_comp_cb) {
		WMA_LOGE("Tx Complete Cb not registered by umac");
		return;
	}

	/* Call Tx Mgmt Complete Callback registered by umac */
	wma_handle->tx_frm_download_comp_cb(mac_context,
					netbuf, status);

	/* Reset Callback */
	wma_handle->tx_frm_download_comp_cb = NULL;

	/* Set the Tx Mgmt Complete Event */
	vos_status  = vos_event_set(
			&wma_handle->tx_frm_download_comp_event);
	if (!VOS_IS_STATUS_SUCCESS(wma_handle))
		WMA_LOGP("Event Set failed - tx_frm_comp_event");
}

/**
  * wma_send_tx_frame - Sends Tx Frame to TxRx
  * @wma_context: wma context
  * @vdev_handle: vdev for which frame has to be send
  * @tx_frm: Tx frame
  * @use_6mbps: whether to send the frame in 6 mbps rate or not
  * @tx_frm_type: data or management
  * @tx_frm_index: index at frame has to be send to TxRx
  * @mgmt_tx_frm_download_comp_cb: cb registered by umac to
  * be called upon download completion
  * This function is a blocking call
  * till it gets download complete indication from TxRx Module
  */
VOS_STATUS wma_send_tx_frame(void *wma_context, void *vdev_handle,
				adf_nbuf_t tx_frm, u_int8_t use_6mbps,
				enum frame_type tx_frm_type,
				enum frame_index tx_frm_index,
				wma_tx_frm_comp_cb tx_frm_download_comp_cb)
{
	tp_wma_handle wma_handle = (tp_wma_handle)(wma_context);
	int32_t status;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	int32_t is_high_latency;

	ol_txrx_vdev_handle txrx_vdev =
			(ol_txrx_vdev_handle)(vdev_handle);

	if (tx_frm_type >= FRAME_80211_MAX) {
		WMA_LOGE("Invalid Frame Type Fail to send Frame");
		return VOS_STATUS_E_FAILURE;
	}

	/*
	 * TODO: Support to send TDLS Frames
	 * TxRx doesn't support Tx Comp/Ack for
	 * Data Frames.
	 * Need to see How to send TDLS Frames ?
	 * Either through Data/Mgmt Path since Ack
	 * Complete Required for TDLS Frames
	 */
	if (tx_frm_type == DATA_FRAME_80211) {
		WMA_LOGE("No Support to send Data Frames (TDLS)");
		return VOS_STATUS_E_FAILURE;
	}

	is_high_latency = wdi_out_cfg_is_high_latency(
				txrx_vdev->pdev->ctrl_pdev);

	/* Send the Tx Mgmt Frame */
	if (tx_frm_download_comp_cb && is_high_latency) {

		/* Store Tx Comp Cb */
		wma_handle->tx_frm_download_comp_cb = tx_frm_download_comp_cb;

		/* Reset the Tx Frame Complete Event */
		vos_status  = vos_event_reset(
				&wma_handle->tx_frm_download_comp_event);

		if (!VOS_IS_STATUS_SUCCESS(wma_handle)) {
			WMA_LOGP("Event Reset failed tx comp event");
			goto error;
		}
	}

	/* Hand over the Tx Mgmt frame to TxRx */
	status = wdi_in_mgmt_send(txrx_vdev, tx_frm, tx_frm_index, use_6mbps);

	/*
	 * Failed to send Tx Mgmt Frame
	 * Return Failure so that umac can freeup the buf
	 */
	if (status) {
		WMA_LOGP("Failed to send Mgmt Frame");
		goto error;
	}

	if (!tx_frm_download_comp_cb)
		return VOS_STATUS_SUCCESS;

	/*
	 * Wait for Download Complete only for
	 * High latency devices
	 */
	if (is_high_latency) {
		/*
		 * Wait for Download Complete
		 * @ Integrated : Dxe Complete
		 * @ Discrete : Target Download Complete
		 */
		vos_status = vos_wait_single_event(
				&wma_handle->tx_frm_download_comp_event,
				WMA_TX_FRAME_COMPLETE_TIMEOUT);

		if (!VOS_IS_STATUS_SUCCESS(vos_status)) {
			WMA_LOGP("Wait Event failed txfrm_comp_event");
			/*
			 * @Integrated: Something Wrong with Dxe
			 *   TODO: Some Debug Code
			 * Here We need to trigger SSR since
			 * since system went into a bad state where
			 * we didn't get Download Complete for almost
			 * WMA_TX_FRAME_COMPLETE_TIMEOUT (1 sec)
			 */
		}
	} else {
		/*
		 * For Low Latency Devices
		 * Call the download complete
		 * callback once the frame is successfully
		 * given to txrx module
		 */
		tx_frm_download_comp_cb(wma_handle->mac_context, tx_frm, A_OK);
	}

	return VOS_STATUS_SUCCESS;

error:
	wma_handle->tx_frm_download_comp_cb = NULL;
	return VOS_STATUS_E_FAILURE;
}

/* function   :wma_setneedshutdown 
 * Descriptin :
 * Args       :
 * Retruns    :
 */
v_VOID_t wma_setneedshutdown(v_VOID_t *vos_ctx)
{
	tp_wma_handle wma_handle;

	WMA_LOGD("Enter");

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	if (NULL == wma_handle) {
		WMA_LOGP("Invalid arguments");
		VOS_ASSERT(0);
		return;
        }

	wma_handle->needShutdown  = TRUE;
	WMA_LOGD("Exit");
}

/* function   : wma_rx_ready_event
 * Descriptin :
 * Args       :
 * Retruns    :
 */
 v_BOOL_t wma_needshutdown(v_VOID_t *vos_ctx)
 {
	tp_wma_handle wma_handle;

	WMA_LOGD("Enter");

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	if (NULL == wma_handle) {
		WMA_LOGP("Invalid arguments");
		VOS_ASSERT(0);
		return 0;
        }

#ifndef QCA_WIFI_ISOC
       /* Suspend the target and disable interrupt */
       if (wma_suspend_target(wma_handle, 1)) {
               WMA_LOGE("Failed to suspend target\n");
       }
#endif

	WMA_LOGD("Exit");
	return wma_handle->needShutdown;
}

VOS_STATUS wma_wait_for_ready_event(WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	VOS_STATUS vos_status;

	/* wait until WMI_READY_EVENTID received from FW */
	vos_status = vos_wait_single_event( &(wma_handle->wma_ready_event),
			WMA_READY_EVENTID_TIMEOUT );

	if (VOS_STATUS_SUCCESS != vos_status) {
		WMA_LOGP("Timeout waiting for ready event from FW");
		vos_status = VOS_STATUS_E_FAILURE;
	}
	return vos_status;
}

#ifndef QCA_WIFI_ISOC
int wma_suspend_target(WMA_HANDLE handle, int disable_target_intr)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_pdev_suspend_cmd* cmd;
	wmi_buf_t wmibuf;
	u_int32_t len = sizeof(wmi_pdev_suspend_cmd);

	if (!wma_handle || !wma_handle->wmi_handle) {
		printk("WMA is closed. can not issue suspend cmd\n");
		return -EINVAL;
	}
	/*
	 * send the comand to Target to ignore the
	 * PCIE reset so as to ensure that Host and target
	 * states are in sync
	 */
	wmibuf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (wmibuf == NULL) {
		return -1;
	}

	cmd = (wmi_pdev_suspend_cmd *)wmi_buf_data(wmibuf);
	if (disable_target_intr) {
		cmd->suspend_opt = WMI_PDEV_SUSPEND_AND_DISABLE_INTR;
	}
	else {
		cmd->suspend_opt = WMI_PDEV_SUSPEND;
	}

	return wmi_unified_cmd_send(wma_handle->wmi_handle, wmibuf, len,
				    WMI_PDEV_SUSPEND_CMDID);
}

int wma_resume_target(WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	wmi_buf_t wmibuf;

	wmibuf = wmi_buf_alloc(wma_handle->wmi_handle, 0);
	if (wmibuf == NULL) {
		return  -1;
	}
	return wmi_unified_cmd_send(wma_handle->wmi_handle, wmibuf, 0,
			WMI_PDEV_RESUME_CMDID);
}
#endif

void WDA_TimerTrafficStatsInd(tWDA_CbContext *pWDA)
{
}
/* TODO: Below is stub should be removed later */
void WDI_DS_ActivateTrafficStats(void)
{
}
/*
 * Function fills the rx packet meta info from the the vos packet
 */
VOS_STATUS WDA_DS_PeekRxPacketInfo(vos_pkt_t *pkt, v_PVOID_t *pkt_meta,
					v_BOOL_t  bSwap)
{
	/* Sanity Check */
	if(pkt == NULL) {
		WMA_LOGE("wma:Invalid parameter sent on wma_peek_rx_pkt_info");
		return VOS_STATUS_E_FAULT;
	}

	*pkt_meta = &(pkt->pkt_meta);

	return VOS_STATUS_SUCCESS;
}
