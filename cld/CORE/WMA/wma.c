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
#include "tl_shim.h"

/* ################### defines ################### */
#define WMA_2_4_GHZ_MAX_FREQ  3000

#define WMA_DEFAULT_SCAN_PRIORITY            1
#define WMA_DEFAULT_SCAN_REQUESTER_ID        1

static void wma_send_msg(tp_wma_handle wma_handle, u_int16_t msg_type,
			 void *body_ptr, u_int32_t body_val);

static void *wma_find_vdev_by_addr(tp_wma_handle wma, u_int8_t *addr,
				   u_int8_t *vdev_id)
{
	u_int8_t i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (vos_mem_compare(wma->interfaces[i].addr, addr,
				sizeof(wma->interfaces[i].addr) == VOS_TRUE)) {
			*vdev_id = i;
			return wma->interfaces[i].handle;
		}
	}
	return NULL;
}

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

static struct wma_target_req *wma_find_vdev_req(tp_wma_handle wma,
						u_int8_t vdev_id)
{
	struct wma_target_req *req_msg, *tmp;

	list_for_each_entry_safe(req_msg, tmp,
				 &wma->vdev_resp_queue, node) {
		if (req_msg->vdev_id != vdev_id)
			continue;

		list_del(&req_msg->node);
		break;
	}
	WMA_LOGD("%s: target request found for vdev id: %d msg_type %d\n",
		 __func__, vdev_id, req_msg->msg_type);
	return req_msg;
}

static int wma_vdev_start_resp_handler(void *handle, u_int8_t *event,
				       u_int16_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct wma_target_req *req_msg;

	wmi_vdev_start_response_event *resp_event =
		(wmi_vdev_start_response_event *)event;

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id);
	if (!req_msg) {
		WMA_LOGP("%s: Failed to lookup request message for vdev %d\n",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}
	vos_timer_stop(&req_msg->event_timeout);
	if (req_msg->msg_type == WDA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams) req_msg->user_data;
		WMA_LOGD("%s: Send channel switch resp vdev %d status %d\n",
			 resp_event->vdev_id, resp_event->status);
		params->status = resp_event->status;
		wma_send_msg(wma, WDA_SWITCH_CHANNEL_RSP, (void *)params, 0);
	}
	vos_timer_destroy(&req_msg->event_timeout);
	vos_mem_free(req_msg);

	return 0;
}

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

static int wma_vdev_stop_resp_handler(void *handle, u8 *event, u16 len)
{
	tp_wma_handle wma = (tp_wma_handle)handle;
	struct wma_target_req *req_msg;
	wmi_vdev_stopped_event *resp_event = (wmi_vdev_stopped_event *)event;
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	u_int8_t peer_id;

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id);
	if (!req_msg) {
		WMA_LOGP("%s: Failed to lookup vdev request for vdev id %d\n",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}
	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);

	vos_timer_stop(&req_msg->event_timeout);
	if (req_msg->msg_type == WDA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams)req_msg->user_data;
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);
		if (!peer)
			WMA_LOGD("%s Failed to find peer %pM\n",
					__func__, params->bssid);
		else
			ol_txrx_peer_detach(peer);
		params->status = VOS_STATUS_SUCCESS;
		wma_send_msg(wma, WDA_DELETE_BSS_RSP, (void *)params, 0);
	} else if (req_msg->msg_type == WDA_SET_LINK_STATE) {
		tpLinkStateParams params =
			(tpLinkStateParams)req_msg->user_data;
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);
		if (!peer)
			WMA_LOGD("%s Failed to find peer %pM\n",
					__func__, params->bssid);
		else
			ol_txrx_peer_detach(peer);
		wma_send_msg(wma, WDA_SET_LINK_STATE_RSP, (void *)params, 0);
	}
	vos_timer_destroy(&req_msg->event_timeout);
	vos_mem_free(req_msg);
	return 0;
}

/*
 * Allocate and init wmi adaptation layer.
 */
VOS_STATUS WDA_open(v_VOID_t *vos_context, v_VOID_t *os_ctx,
		    wda_tgt_cfg_cb tgt_cfg_cb,
		    tMacOpenParameters *mac_params)
{
	tp_wma_handle wma_handle;
	HTC_HANDLE htc_handle;
	adf_os_device_t adf_dev;
	v_VOID_t *wmi_handle;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

	WMA_LOGD("%s: Enter", __func__);

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
	wma_handle->interfaces = vos_mem_malloc(sizeof(struct wma_txrx_node) *
						wma_handle->max_bssid);
	if (!wma_handle->interfaces) {
		WMA_LOGP("failed to allocate interface table");
		goto err_wmi_attach;
	}
	/* Register the debug print event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_DEBUG_PRINT_EVENTID,
					   wma_unified_debug_print_event_handler);

	wma_handle->tgt_cfg_update_cb = tgt_cfg_cb;

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
        vos_status = vos_event_init(&wma_handle->target_suspend);
	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("target suspend event initialization failed");
		goto err_event_init;
	}

	/* Init Tx Frame Complete event */
	vos_status = vos_event_init(&wma_handle->tx_frm_download_comp_event);
	if (!VOS_IS_STATUS_SUCCESS(vos_status)) {
		WMA_LOGP("failed to init tx_frm_download_comp_event");
		goto err_event_init;
	}
	INIT_LIST_HEAD(&wma_handle->vdev_resp_queue);

	/* Register vdev start response event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_VDEV_START_RESP_EVENTID,
					   wma_vdev_start_resp_handler);

	/* Register vdev stop response event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_VDEV_STOPPED_EVENTID,
					   wma_vdev_stop_resp_handler);

	WMA_LOGD("%s: Exit", __func__);

	return VOS_STATUS_SUCCESS;

err_event_init:
	wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
					     WMI_DEBUG_PRINT_EVENTID);
err_wmi_attach:
	vos_mem_free(wma_handle->interfaces);
	vos_free_context(wma_handle->vos_context, VOS_MODULE_ID_WDA,
			 wma_handle);

	WMA_LOGD("%s: Exit", __func__);

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

	WMA_LOGD("%s: Enter", __func__);

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
	WMA_LOGD("%s: Exit", __func__);
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
	void *txrx_hdl;

	/* remove the interface from ath_dev */
	if (wma_unified_vdev_delete_send(wma_handle->wmi_handle, 
			pdel_sta_self_req_param->sessionId)) {
		WMA_LOGP("Unable to remove an interface for ath_dev.\n");
		status = VOS_STATUS_E_FAILURE;
	}

	txrx_hdl = wma_handle->interfaces[pdel_sta_self_req_param->sessionId].handle;
	if(!txrx_hdl)
		status = VOS_STATUS_E_FAILURE;
	else
		ol_txrx_vdev_detach(txrx_hdl, NULL, NULL);
	vos_mem_zero(&wma_handle->interfaces[pdel_sta_self_req_param->sessionId],
		     sizeof(wma_handle->interfaces[pdel_sta_self_req_param->sessionId]));

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
static ol_txrx_vdev_handle wma_vdev_attach(tp_wma_handle wma_handle,
					   tpAddStaSelfParams self_sta_req)
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
	wma_handle->interfaces[self_sta_req->sessionId].handle = txrx_vdev_handle;
	vos_mem_copy(wma_handle->interfaces[self_sta_req->sessionId].addr,
		     self_sta_req->selfMacAddr,
		     sizeof(wma_handle->interfaces[self_sta_req->sessionId].addr));

end:
	self_sta_req->status = status;
	wma_send_msg(wma_handle, WDA_ADD_STA_SELF_RSP, (void *)self_sta_req, 0);
	return txrx_vdev_handle;
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

	WMA_LOGD("%s: Enter", __func__);

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

	WMA_LOGD("%s: Exit", __func__);
	return vos_status;

fail:
	if(cfg_bin != NULL)
		vos_mem_free( file_img );

	WMA_LOGD("%s: Exit", __func__);
	return vos_status;
}

/* function   : wma_set_cur_scan_info
 * Descriptin : function to save current ongoing scan info
 * Args       : wma handle, scan id, scan requestor id, vdev id
 * Retruns    : None
 */
static inline void wma_set_cur_scan_info(tp_wma_handle wma_handle,
					u_int32_t scan_id,
					u_int32_t requestor,
					u_int32_t vdev_id)
{
	wma_handle->cur_scan_info.scan_id = scan_id;
	wma_handle->cur_scan_info.scan_requestor_id = requestor;
	wma_handle->cur_scan_info.vdev_id = vdev_id;
}

/* function   : wma_reset_cur_scan_info
 * Descriptin : function to reset the current ongoing scan info
 * Args       : wma handle
 * Retruns    : None
 */
static inline void wma_reset_cur_scan_info(tp_wma_handle wma_handle)
{
	vos_mem_zero((void *) &wma_handle->cur_scan_info,
			sizeof(struct scan_param));
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
	cmd->scan_id = WMA_HOST_SCAN_REQID_PREFIX | ++wma_handle->scan_id;
	cmd->scan_priority = WMA_DEFAULT_SCAN_PRIORITY;
	cmd->scan_req_id = WMA_HOST_SCAN_REQUESTOR_ID_PREFIX |
			   WMA_DEFAULT_SCAN_REQUESTER_ID;

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

/* function   : wma_get_buf_stop_scan_cmd
 * Descriptin : function to fill the args for wmi_stop_scan_cmd
 * Args       : wma handle, wmi command buffer, buffer length
 * Retruns    : failure or success
 */
VOS_STATUS wma_get_buf_stop_scan_cmd(tp_wma_handle wma_handle,
					wmi_buf_t *buf,
					int *buf_len)
{
	wmi_stop_scan_cmd *cmd;
	VOS_STATUS vos_status;
	int len = sizeof(wmi_stop_scan_cmd);

	/* Allocate the memory */
	*buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!*buf) {
		WMA_LOGP("failed to allocate memory for stop scan cmd");
		vos_status = VOS_STATUS_E_FAILURE;
		goto error;
	}

	cmd = (wmi_stop_scan_cmd *) wmi_buf_data(*buf);

	cmd->requestor = wma_handle->cur_scan_info.scan_requestor_id;
	cmd->scan_id = wma_handle->cur_scan_info.scan_id;
	cmd->vdev_id = wma_handle->cur_scan_info.vdev_id;
	/* stop the scan with the corresponding scan_id */
	cmd->req_type = WMI_SCAN_STOP_ONE;

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
	wmi_start_scan_cmd *cmd;
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

	/* Save current scan info */
	cmd = (wmi_start_scan_cmd *) wmi_buf_data(buf);
	wma_set_cur_scan_info(wma_handle, cmd->scan_id,
			cmd->scan_req_id, cmd->vdev_id);

	status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
			len, WMI_START_SCAN_CMDID);
	/* Call the wmi api to request the scan */
	if (0 != status) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d",
			status);
		vos_status = VOS_STATUS_E_FAILURE;
		goto error;
	}

	WMA_LOGI("WMA --> WMI_START_SCAN_CMDID");
	return VOS_STATUS_SUCCESS;
error:
	wma_reset_cur_scan_info(wma_handle);
	if (buf)
		adf_nbuf_free(buf);
error1:
	return vos_status;
}

/* function   : wma_stop_scan
 * Descriptin : function to send the stop scan command
 * Args       : wma_handle
 * Retruns    : failure or success
 */
VOS_STATUS wma_stop_scan(tp_wma_handle wma_handle)
{
	VOS_STATUS vos_status;
	wmi_buf_t buf;
	int status = 0;
	int len;

	vos_status = wma_get_buf_stop_scan_cmd(wma_handle, &buf, &len);

	if (vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGE("Failed to get buffer for stop scan cmd");
		goto error1;
	}

	status = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
			len, WMI_STOP_SCAN_CMDID);
	/* Call the wmi api to request the scan */
	if (0 != status) {
		WMA_LOGE("wmi_unified_cmd_send returned Error %d",
			status);
		vos_status = VOS_STATUS_E_FAILURE;
		goto error;
	}

	WMA_LOGI("WMA --> WMI_STOP_SCAN_CMDID");

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

static int wmi_unified_peer_create_send(wmi_unified_t wmi,
					const u_int8_t *peer_addr,
					u_int32_t vdev_id)
{
	wmi_peer_create_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_peer_create_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_create_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_CREATE_CMDID)) {
		WMA_LOGP("failed to send peer create command\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d\n", __func__, peer_addr, vdev_id);
	return 0;
}

static VOS_STATUS wma_create_peer(tp_wma_handle wma, ol_txrx_pdev_handle pdev,
				  ol_txrx_vdev_handle vdev, u8 *peer_addr,
				  u_int8_t vdev_id)
{
	ol_txrx_peer_handle peer;
	u_int8_t peer_id;

	WMA_LOGD("%s: peer_addr %pM vdev_id %d\n", __func__, peer_addr, vdev_id);
	if (ol_txrx_find_peer_by_addr(pdev, peer_addr, &peer_id)) {
		WMA_LOGP("%s, Already peer %pM exists\n", __func__, peer_addr);
		return VOS_STATUS_SUCCESS;
	}
	if (++wma->peer_count > wma->wlan_resource_config.num_peers) {
		WMA_LOGP("%s, the peer count exceeds the limit %d\n",
			 __func__, wma->peer_count - 1);
		goto err;
	}
	peer = ol_txrx_peer_attach(pdev, vdev, peer_addr);
	if (!peer)
		goto err;

	if (wmi_unified_peer_create_send(wma->wmi_handle, peer_addr,
					 vdev_id) < 0) {
		WMA_LOGP("%s : Unable to create peer in Target\n", __func__);
		ol_txrx_peer_detach(peer);
		goto err;
	}
	return VOS_STATUS_SUCCESS;
err:
	wma->peer_count--;
	return VOS_STATUS_E_FAILURE;
}

static int32_t wmi_unified_peer_delete_send(wmi_unified_t wmi,
					u_int8_t peer_addr[IEEE80211_ADDR_LEN],
					u_int8_t vdev_id)
{
	wmi_peer_delete_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_peer_delete_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_delete_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		WMA_LOGP("Failed to send peer delete command\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d\n", __func__, peer_addr, vdev_id);
	return 0;
}

static int32_t wmi_unified_peer_flush_tids_send(wmi_unified_t wmi,
					    u_int8_t peer_addr
							[IEEE80211_ADDR_LEN],
					    u_int32_t peer_tid_bitmap,
					    u_int8_t vdev_id)
{
	wmi_peer_flush_tids_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_peer_flush_tids_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_flush_tids_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = peer_tid_bitmap;
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		WMA_LOGP("Failed to send flush tid command\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d\n", __func__, peer_addr, vdev_id);
	return 0;
}

static void wma_remove_peer(tp_wma_handle wma, u_int8_t *bssid,
			    u_int8_t vdev_id)
{
#define PEER_ALL_TID_BITMASK 0xffffffff
	u_int32_t peer_tid_bitmap = PEER_ALL_TID_BITMASK;

	WMA_LOGD("%s: bssid %pM vdevid %d\n", __func__, bssid, vdev_id);
	/* Flush all TIDs except MGMT TID for this peer in Target */
	peer_tid_bitmap &= ~(0x1 << WMI_MGMT_TID);
	wmi_unified_peer_flush_tids_send(wma->wmi_handle, bssid,
					 peer_tid_bitmap, vdev_id);

	wmi_unified_peer_delete_send(wma->wmi_handle, bssid, vdev_id);
#undef PEER_ALL_TID_BITMASK
}

static WLAN_PHY_MODE wma_chan_to_mode(u8 chan, ePhyChanBondState chan_offset)
{
	WLAN_PHY_MODE phymode = MODE_UNKNOWN;

	/* 2.4 GHz band */
	if ((chan >= WMA_11G_CHANNEL_BEGIN) && (chan <= WMA_11G_CHANNEL_END)) {
		switch (chan_offset) {
		case PHY_SINGLE_CHANNEL_CENTERED:
			phymode = MODE_11NG_HT20;
			break;
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			phymode = MODE_11NG_HT40;
			break;
			/* TODO: Handle VHT mode */
		default:
			break;
		}
	}

	/* 5 GHz band */
	if ((chan >= WMA_11A_CHANNEL_BEGIN) && (chan <= WMA_11A_CHANNEL_END)) {
		switch (chan_offset) {
		case PHY_SINGLE_CHANNEL_CENTERED:
			phymode = MODE_11NA_HT20;
			break;
		case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
			phymode = MODE_11NA_HT40;
			break;
			/* TODO: Handle VHT mode */
		default:
			break;
		}
	}
	WMA_LOGD("%s: phymode %d channel %d offset %d\n", __func__,
		 phymode, chan, chan_offset);

	return phymode;
}

static VOS_STATUS wma_vdev_start(tp_wma_handle wma,
				 struct wma_vdev_start_req *req)
{
	wmi_vdev_start_request_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_start_request_cmd);
	WLAN_PHY_MODE chanmode;

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGD("%s : wmi_buf_alloc failed\n", __func__);
		return VOS_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_start_request_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = req->vdev_id;

	/* Fill channel info */
	cmd->chan.mhz = vos_chan_to_freq(req->chan);
	chanmode = wma_chan_to_mode(req->chan, req->chan_offset);
	WMI_SET_CHANNEL_MODE(&cmd->chan, chanmode);
	cmd->chan.band_center_freq1 = cmd->chan.mhz;
	if ((chanmode == MODE_11NA_HT40) || (chanmode == MODE_11NG_HT40)) {
		if (req->chan_offset == PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
			cmd->chan.band_center_freq1 -= 10;
		else
			cmd->chan.band_center_freq1 += 10;
	}
	cmd->chan.band_center_freq2 = 0;
	/*
	 * If the channel has DFS set, flip on radar reporting.
	 *
	 * It may be that this should only be done for IBSS/hostap operation
	 * as this flag may be interpreted (at some point in the future)
	 * by the firmware as "oh, and please do radar DETECTION."
	 *
	 * If that is ever the case we would insert the decision whether to
	 * enable the firmware flag here.
	 */
	if (req->is_dfs) {
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_DFS);
		cmd->disable_hw_ack = (req->oper_mode) ? 0 : 1;
	}

	cmd->beacon_interval = req->beacon_intval;
	cmd->dtim_period = req->dtim_period;
	/* FIXME: Find out min, max and regulatory power levels */
	WMI_SET_CHANNEL_MIN_POWER(&cmd->chan, req->max_txpow);

	/* TODO: Handle regulatory class, max antenna */

	/* Copy the SSID */
	if (req->ssid.length) {
		if (req->ssid.length < sizeof(cmd->ssid.ssid))
			cmd->ssid.ssid_len = req->ssid.length;
		else
			cmd->ssid.ssid_len = sizeof(cmd->ssid.ssid);
		vos_mem_copy(cmd->ssid.ssid, req->ssid.ssId,
			     cmd->ssid.ssid_len);
	}

	if (req->hidden_ssid)
		cmd->flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;

	if (req->pmf_enabled)
		cmd->flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;

	WMA_LOGD("%s: vdev_id %d freq %d channel %d chanmode %d is_dfs %d\
		 beacon interval %d dtim %d\n", __func__, req->vdev_id,
		 cmd->chan.mhz, req->chan, chanmode, req->is_dfs,
		 req->beacon_intval, cmd->dtim_period);

	if (wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				 WMI_VDEV_START_REQUEST_CMDID) < 0) {
		WMA_LOGP("Failed to send vdev start command\n");
		adf_nbuf_free(buf);
		return VOS_STATUS_E_FAILURE;
	}

	return VOS_STATUS_SUCCESS;
}

void wma_vdev_resp_timer(void *data)
{
	tp_wma_handle wma;
	struct wma_target_req *tgt_req = (struct wma_target_req *)data;
	void *vos_context = vos_get_global_context(VOS_MODULE_ID_WDA, NULL);
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	u_int8_t peer_id;

	wma = (tp_wma_handle) vos_get_context(VOS_MODULE_ID_WDA, vos_context);
	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);

	WMA_LOGD("%s: request %d is timed out\n", __func__, tgt_req->msg_type);
	if (tgt_req->msg_type == WDA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams)tgt_req->user_data;
		params->status = VOS_STATUS_E_TIMEOUT;
		wma_send_msg(wma, WDA_SWITCH_CHANNEL_RSP, (void *)params, 0);
	} else if (tgt_req->msg_type == WDA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams)tgt_req->user_data;
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);
		if (!peer)
			WMA_LOGP("%s Failed to find peer %pM\n", __func__,
				 params->bssid);
		else
			ol_txrx_peer_detach(peer);
		params->status = VOS_STATUS_E_TIMEOUT;
		wma_send_msg(wma, WDA_DELETE_BSS_RSP, (void *)params, 0);
	} else if (tgt_req->msg_type == WDA_SET_LINK_STATE) {
		tpLinkStateParams params =
			(tpLinkStateParams)tgt_req->user_data;
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);
		if (!peer)
			WMA_LOGP("%s Failed to find peer %pM\n", __func__,
				 params->bssid);
		else
			ol_txrx_peer_detach(peer);
		wma_send_msg(wma, WDA_SET_LINK_STATE_RSP, (void *)params, 0);
	}
	list_del(&tgt_req->node);
	vos_timer_destroy(&tgt_req->event_timeout);
	vos_mem_free(tgt_req);
}

static VOS_STATUS wma_fill_vdev_req(tp_wma_handle wma, u_int8_t vdev_id,
				    u_int32_t msg_type, void *params)
{
	struct wma_target_req *req;

	req = vos_mem_malloc(sizeof(*req));
	if (!req) {
		WMA_LOGP("Failed to allocate memory for msg %d vdev %d\n",
			 msg_type, vdev_id);
		return VOS_STATUS_E_NOMEM;
	}

	WMA_LOGD("%s: vdev_id %d msg %d\n", __func__, vdev_id, msg_type);
	req->vdev_id = vdev_id;
	req->msg_type = msg_type;
	req->user_data = params;
	list_add_tail(&req->node, &wma->vdev_resp_queue);
	vos_timer_init(&req->event_timeout, VOS_TIMER_TYPE_SW,
		       wma_vdev_resp_timer, req);
	vos_timer_start(&req->event_timeout, 1000);
	return VOS_STATUS_SUCCESS;
}

static void wma_set_channel(tp_wma_handle wma, tpSwitchChannelParams params)
{
	struct wma_vdev_start_req req;
	VOS_STATUS status;

	vos_mem_zero(&req, sizeof(req));
	if (!wma_find_vdev_by_addr(wma, params->selfStaMacAddr, &req.vdev_id)) {
		WMA_LOGP("%s: Failed to find vdev id for %pM\n",
			 __func__, params->selfStaMacAddr);
		status = VOS_STATUS_E_FAILURE;
		goto send_resp;
	}
	req.chan = params->channelNumber;
	req.chan_offset = params->secondaryChannelOffset;
#ifdef WLAN_FEATURE_VOWIFI
	req.max_txpow = params->maxTxPower;
#else
	req.max_txpow = params->localPowerConstraint;
#endif
	req.beacon_intval = 100;
	req.dtim_period = 1;
	status = wma_vdev_start(wma, &req);
	if (status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("vdev start failed status = %d\n", status);
		goto send_resp;
	}

	status = wma_fill_vdev_req(wma, req.vdev_id, WDA_CHNL_SWITCH_REQ,
				   params);
	if (status == VOS_STATUS_SUCCESS)
		return;
	WMA_LOGP("Failed to fill channel switch request for vdev %d status %d\n",
		 req.vdev_id, status);
send_resp:
	WMA_LOGD("%s: channel %d offset %d txpower %d status %d\n", __func__,
		 params->channelNumber, params->secondaryChannelOffset,
#ifdef WLAN_FEATURE_VOWIFI
		 params->maxTxPower,
#else
		 params->localPowerConstraint,
#endif
		 status);
	params->status = status;
	wma_send_msg(wma, WDA_SWITCH_CHANNEL_RSP, (void *)params, 0);
}

static WLAN_PHY_MODE wma_peer_phymode(tSirNwType nw_type, u_int8_t is_ht,
				      u_int8_t is_cw40)
{
	WLAN_PHY_MODE phymode = MODE_UNKNOWN;

	switch (nw_type) {
		case eSIR_11B_NW_TYPE:
		case eSIR_11G_NW_TYPE:
			if (is_ht)
				phymode = (is_cw40) ?
					MODE_11NG_HT40 : MODE_11NG_HT20;
			else
				phymode = MODE_11G;
			break;
		case eSIR_11A_NW_TYPE:
			if (is_ht)
				phymode = (is_cw40) ?
					MODE_11NA_HT40 : MODE_11NA_HT20;
			else
				phymode = MODE_11A;
			break;
		default:
			WMA_LOGP("Invalid nw type %d\n", nw_type);
			break;
	}
	WMA_LOGD("%s: nw_type %d is_ht %d is_cw40 %d phymode %d\n", __func__,
		 nw_type, is_ht, is_cw40, phymode);
	return phymode;
}

static int32_t wmi_unified_send_peer_assoc(tp_wma_handle wma,
					   tSirNwType nw_type,
					   tpAddStaParams params)
{
	wmi_peer_assoc_complete_cmd *cmd;
	int32_t len = sizeof(wmi_peer_assoc_complete_cmd);
	wmi_buf_t buf;
	int32_t ret, max_rates, i;
	u_int8_t rx_stbc;

	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_assoc_complete_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->bssId, &cmd->peer_macaddr);
	cmd->vdev_id = params->smesessionId;
	cmd->peer_new_assoc = 1;
	cmd->peer_associd = params->assocId;

	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;

	if (params->wmmEnabled)
		cmd->peer_flags |= WMI_PEER_QOS;

	if (params->htCapable) {
		cmd->peer_flags |= WMI_PEER_HT;
		cmd->peer_rate_caps |= WMI_RC_HT_FLAG;
	}

	/* TODO: Need to handle uAPSD */
	if (params->txChannelWidthSet) {
		cmd->peer_flags |= WMI_PEER_40MHZ;
		cmd->peer_rate_caps |= WMI_RC_CW40_FLAG;
		if (params->fShortGI40Mhz)
			cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;
	} else if (params->fShortGI20Mhz)
		cmd->peer_rate_caps |= WMI_RC_SGI_FLAG;

#ifdef WLAN_FEATURE_11AC
	if (params->vhtCapable) {
		cmd->peer_flags |= (WMI_PEER_HT | WMI_PEER_VHT);
		cmd->peer_rate_caps |= WMI_RC_HT_FLAG;
	}

	if (params->vhtTxChannelWidthSet)
		cmd->peer_flags |= WMI_PEER_80MHZ;

	cmd->peer_vht_caps = params->vht_caps;
#endif
	rx_stbc = (params->ht_caps & IEEE80211_HTCAP_C_RXSTBC) >>
			IEEE80211_HTCAP_C_RXSTBC_S;
	if (rx_stbc) {
		cmd->peer_flags |= WMI_PEER_STBC;
		cmd->peer_rate_caps |= (rx_stbc << WMI_RC_RX_STBC_FLAG_S);
	}

	if (params->htLdpcCapable || params->vhtLdpcCapable)
		cmd->peer_flags |= WMI_PEER_LDPC;

	switch (params->mimoPS) {
		case eSIR_HT_MIMO_PS_STATIC:
			cmd->peer_flags |= WMI_PEER_STATIC_MIMOPS;
			break;
		case eSIR_HT_MIMO_PS_DYNAMIC:
			cmd->peer_flags |= WMI_PEER_DYN_MIMOPS;
			break;
		case eSIR_HT_MIMO_PS_NO_LIMIT:
			cmd->peer_flags |= WMI_PEER_SPATIAL_MUX;
			break;
		default:
			break;
	}
	cmd->peer_flags |= WMI_PEER_AUTH;
	if (params->wpa_rsn)
		cmd->peer_flags |= WMI_PEER_NEED_PTK_4_WAY;
	if (params->wpa_rsn >> 1)
		cmd->peer_flags |= WMI_PEER_NEED_GTK_2_WAY;

	cmd->peer_caps = params->capab_info;
	cmd->peer_listen_intval = params->listenInterval;
	cmd->peer_ht_caps = params->ht_caps;
	cmd->peer_max_mpdu = params->maxAmpduSize;

	if (params->supportedRates.supportedMCSSet[1] &&
	    params->supportedRates.supportedMCSSet[2])
		cmd->peer_rate_caps |= WMI_RC_TS_FLAG;
	else if (params->supportedRates.supportedMCSSet[1])
		cmd->peer_rate_caps |= WMI_RC_DS_FLAG;

	/* Legacy Rateset */
	for (i = 0; i < SIR_NUM_11B_RATES; i++) {
		if (!params->supportedRates.llbRates[i])
			continue;
		cmd->peer_legacy_rates.rates
			[cmd->peer_legacy_rates.num_rates++] =
			params->supportedRates.llbRates[i];
	}
	for (i = 0; i < SIR_NUM_11A_RATES; i++) {
		if (!params->supportedRates.llaRates[i])
			continue;
		cmd->peer_legacy_rates.rates
			[cmd->peer_legacy_rates.num_rates++] =
			params->supportedRates.llaRates[i];
	}

	/* HT Rateset */
	max_rates = sizeof(cmd->peer_ht_rates.rates) /
		    sizeof(cmd->peer_ht_rates.rates[0]);
	for (i = 0; i < MAX_SUPPORTED_RATES; i++) {
		if (params->supportedRates.supportedMCSSet[i / 8] &
					(1 << (i % 8))) {
			cmd->peer_ht_rates.rates
				[cmd->peer_ht_rates.num_rates++] = i;
		}
		if (cmd->peer_ht_rates.num_rates == max_rates)
		       break;
	}
	/* TODO: VHT Rates */
	cmd->peer_nss = MAX((cmd->peer_ht_rates.num_rates + 7) / 8, 1);
	cmd->peer_phymode = wma_peer_phymode(nw_type, params->htCapable,
					     params->txChannelWidthSet);

	WMA_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x\
		 peer_caps %x listen_intval %d ht_caps %x max_mpdu %d\
		 nss %d phymode %d\n", __func__, cmd->vdev_id, cmd->peer_associd,
		 cmd->peer_flags, cmd->peer_rate_caps, cmd->peer_caps,
		 cmd->peer_listen_intval, cmd->peer_ht_caps, cmd->peer_max_mpdu,
		 cmd->peer_nss, cmd->peer_phymode);
	ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (ret < 0) {
		WMA_LOGP("Failed to send peer assoc command ret = %d\n", ret);
		adf_nbuf_free(buf);
	}
	return ret;
}

static void wma_add_bss(tp_wma_handle wma, tpAddBssParams params)
{
	ol_txrx_pdev_handle pdev;

	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);
	if (params->operMode) {
		if (!params->updateBss)
			goto send_bss_resp;

		/* Update peer state */
		if (params->staContext.encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: Update peer(%pM) state into auth\n",
				 __func__, params->bssId);
			ol_txrx_peer_state_update(pdev, params->bssId,
						  ol_txrx_peer_state_auth);
		} else {
			WMA_LOGD("%s: Update peer(%pM) state into conn\n",
				 __func__, params->bssId);
			ol_txrx_peer_state_update(pdev, params->bssId,
						  ol_txrx_peer_state_conn);
		}

		wmi_unified_send_peer_assoc(wma, params->nwType,
					    &params->staContext);
		if (params->staContext.encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: send peer authorize wmi cmd for %pM\n",
				 __func__, params->bssId);
			wma_set_peer_param(wma, params->bssId,
					   WMI_PEER_AUTHORIZE, 1,
					   params->staContext.bssIdx);
		}
	}
send_bss_resp:
	ol_txrx_find_peer_by_addr(pdev, params->bssId,
				  &params->staContext.staIdx);
	params->status = (params->staContext.staIdx < 0) ?
				VOS_STATUS_E_FAILURE : VOS_STATUS_SUCCESS;
	vos_mem_copy(params->staContext.staMac, params->bssId,
		     sizeof(params->staContext.staMac));
	WMA_LOGD("%s: opermode %d update_bss %d nw_type %d bssid %pM\
		 staIdx %d status %d\n", __func__, params->operMode,
		 params->updateBss, params->nwType, params->bssId,
		 params->staContext.staIdx, params->status);
	wma_send_msg(wma, WDA_ADD_BSS_RSP, (void *)params, 0);
}

static int wmi_unified_vdev_up_send(wmi_unified_t wmi,
				    u_int8_t vdev_id, u_int16_t aid,
				    u_int8_t bssid[IEEE80211_ADDR_LEN])
{
	wmi_vdev_up_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_up_cmd);

	WMA_LOGD("%s: vdev_id %d aid %d bssid %pM\n", __func__,
		 vdev_id, aid, bssid);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_up_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	cmd->vdev_assoc_id = aid;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_UP_CMDID)) {
		WMA_LOGP("Failed to send vdev up command\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	return 0;
}

static void wma_add_sta(tp_wma_handle wma, tpAddStaParams params)
{
	ol_txrx_pdev_handle pdev;
	VOS_STATUS status = VOS_STATUS_SUCCESS;

	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);
	if (params->staType != STA_ENTRY_SELF) {
		WMA_LOGP("%s: unsupported station type %d\n", params->staType);
		goto out;
	}

	if (wmi_unified_vdev_up_send(wma->wmi_handle, params->smesessionId,
				     params->assocId, params->bssId) < 0) {
		WMA_LOGP("Failed to send vdev up cmd: vdev %d bssid %pM\n",
			 params->smesessionId, params->bssId);
		status = VOS_STATUS_E_FAILURE;
	}

out:
	ol_txrx_find_peer_by_addr(pdev, params->bssId, &params->staIdx);
	params->status = status;
	WMA_LOGD("%s: statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d\n",
		 __func__, params->staType, params->smesessionId, params->assocId,
		 params->bssId, params->staIdx, status);
	wma_send_msg(wma, WDA_ADD_STA_RSP, (void *)params, 0);
}

static void wma_set_bsskey(tp_wma_handle wma_handle, tpSetBssKeyParams key_info)
{
	key_info->status = VOS_STATUS_SUCCESS;
	wma_send_msg(wma_handle, WDA_SET_BSSKEY_RSP, (void *)key_info, 0);
}

static void wma_set_stakey(tp_wma_handle wma_handle, tpSetStaKeyParams key_info)
{
	key_info->status = VOS_STATUS_SUCCESS;
	wma_send_msg(wma_handle, WDA_SET_STAKEY_RSP, (void *)key_info, 0);
}

static int wmi_unified_vdev_down_send(wmi_unified_t wmi, u_int8_t vdev_id)
{
	wmi_vdev_down_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_down_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_down_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_DOWN_CMDID)) {
		WMA_LOGP("Failed to send vdev down\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: vdev_id %d\n", __func__, vdev_id);
	return 0;
}

static void wma_delete_sta(tp_wma_handle wma, tpDeleteStaParams params)
{
	VOS_STATUS status = VOS_STATUS_SUCCESS;

	if (wmi_unified_vdev_down_send(wma->wmi_handle, params->smesessionId) < 0) {
		WMA_LOGP("%s: failed to bring down vdev %d\n",
			 __func__, params->smesessionId);
		status = VOS_STATUS_E_FAILURE;
	}
	params->status = status;
	WMA_LOGD("%s: vdev_id %d status %d\n", __func__, params->smesessionId, status);
	wma_send_msg(wma, WDA_DELETE_STA_RSP, (void *)params, 0);
}

static int32_t wmi_unified_vdev_stop_send(wmi_unified_t wmi, u_int8_t vdev_id)
{
	wmi_vdev_stop_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_stop_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed\n", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_stop_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID)) {
		WMA_LOGP("Failed to send vdev stop command\n");
		adf_nbuf_free(buf);
		return -EIO;
	}
	return 0;
}

static void wma_delete_bss(tp_wma_handle wma, tpDeleteBssParams params)
{
	ol_txrx_pdev_handle pdev;
	ol_txrx_peer_handle peer;
	VOS_STATUS status = VOS_STATUS_SUCCESS;
	u_int8_t peer_id;

	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);

	peer = ol_txrx_find_peer_by_addr(pdev, params->bssid,
					 &peer_id);
	if (!peer) {
		WMA_LOGP("%s: Failed to find peer %pM\n", __func__,
			 params->bssid);
		status = VOS_STATUS_E_FAILURE;
		goto out;
	}
	wma_remove_peer(wma, params->bssid, params->smesessionId);
	if (wmi_unified_vdev_stop_send(wma->wmi_handle, params->smesessionId) ||
	    (wma_fill_vdev_req(wma, params->smesessionId, WDA_DELETE_BSS_REQ,
			       params) != VOS_STATUS_SUCCESS)) {
		WMA_LOGP("%s: %d Failed to send vdev stop\n",
			 __func__, __LINE__);
		ol_txrx_peer_detach(peer);
		status = VOS_STATUS_E_FAILURE;
		goto out;
	}
	WMA_LOGD("%s: bssid %pM vdev_id %d\n",
		__func__, params->bssid, params->smesessionId);
	return;
out:
	params->status = status;
	wma_send_msg(wma, WDA_DELETE_BSS_RSP, (void *)params, 0);
}

static void wma_set_linkstate(tp_wma_handle wma, tpLinkStateParams params)
{
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer;
	u_int8_t vdev_id, peer_id;

	WMA_LOGD("%s: state %d selfmac %pM\n", __func__,
		 params->state, params->selfMacAddr);
	if ((params->state != eSIR_LINK_PREASSOC_STATE) &&
	    (params->state != eSIR_LINK_IDLE_STATE)) {
		WMA_LOGD("%s: unsupported link state %d\n",
			 __func__, params->state);
		goto out;
	}

	pdev = vos_get_context(VOS_MODULE_ID_TXRX, wma->vos_context);
	vdev = wma_find_vdev_by_addr(wma, params->selfMacAddr, &vdev_id);
	if (!vdev) {
		WMA_LOGP("%s: vdev not found for addr: %pM\n", params->selfMacAddr);
		goto out;
	}

	if (params->state == eSIR_LINK_PREASSOC_STATE)
		wma_create_peer(wma, pdev, vdev, params->bssid, vdev_id);
	else {
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);
		if (!peer) {
			WMA_LOGP("%s: peer %pM vdev id %d already deleted\n",
				 __func__, params->bssid, vdev_id);
			goto out;
		}
		wma_remove_peer(wma, params->bssid, vdev_id);
		if (wmi_unified_vdev_stop_send(wma->wmi_handle, vdev_id) ||
		    (wma_fill_vdev_req(wma, vdev_id, WDA_SET_LINK_STATE,
				       params) != VOS_STATUS_SUCCESS)) {
			WMA_LOGP("%s: %d Failed to send vdev stop\n",
				 __func__, __LINE__);
			ol_txrx_peer_detach(peer);
			goto out;
		}
		return;
	}
out:
	wma_send_msg(wma, WDA_SET_LINK_STATE_RSP, (void *)params, 0);
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
	ol_txrx_vdev_handle txrx_vdev_handle = NULL;

	WMA_LOGD("%s: Enter", __func__);
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
			txrx_vdev_handle = wma_vdev_attach(wma_handle,
					(tAddStaSelfParams *)msg->bodyptr);
			if (!txrx_vdev_handle)
				WMA_LOGE("Failed to attach vdev");
			else
				WLANTL_RegisterVdev(vos_context,
						    txrx_vdev_handle);
			break;
		case WDA_DEL_STA_SELF_REQ:
			wma_vdev_detach(wma_handle, (tDelStaSelfParams *)msg->bodyptr);
			break;
		case WDA_START_SCAN_OFFLOAD_REQ:
			wma_start_scan(wma_handle, msg->bodyptr);
			break;
		case WDA_STOP_SCAN_OFFLOAD_REQ:
			wma_stop_scan(wma_handle);
			break;
		case WDA_UPDATE_CHAN_LIST_REQ:
			wma_update_channel_list(wma_handle,
					(tSirUpdateChanList *)msg->bodyptr);
			break;
		case WDA_SET_LINK_STATE:
			wma_set_linkstate(wma_handle,
					  (tpLinkStateParams)msg->bodyptr);
			break;
		case WDA_CHNL_SWITCH_REQ:
			wma_set_channel(wma_handle,
					(tpSwitchChannelParams)msg->bodyptr);
			break;
		case WDA_ADD_BSS_REQ:
			wma_add_bss(wma_handle, (tpAddBssParams)msg->bodyptr);
			break;
		case WDA_ADD_STA_REQ:
			wma_add_sta(wma_handle, (tpAddStaParams)msg->bodyptr);
			break;
		case WDA_SET_BSSKEY_REQ:
			wma_set_bsskey(wma_handle,
					(tpSetBssKeyParams)msg->bodyptr);
			break;
		case WDA_SET_STAKEY_REQ:
			wma_set_stakey(wma_handle,
					(tpSetStaKeyParams)msg->bodyptr);
			break;
		case WDA_DELETE_STA_REQ:
			wma_delete_sta(wma_handle,
					(tpDeleteStaParams)msg->bodyptr);
			break;
		case WDA_DELETE_BSS_REQ:
			wma_delete_bss(wma_handle,
					(tpDeleteBssParams)msg->bodyptr);
			break;
		default:
			WMA_LOGD("unknow msg type %x", msg->type);
			/* Do Nothing? MSG Body should be freed at here */
			if(NULL != msg->bodyptr) {
				vos_mem_free(msg->bodyptr);
			}
	}
end:
	WMA_LOGD("%s: Exit", __func__);
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

	WMA_LOGI("WMA <-- wmi_scan_event : event %lu, scan_id %lu, freq %lu",
			wmi_event->event, wmi_event->scan_id,
			wmi_event->channel_freq);

	scan_event->event = wmi_event->event;
	scan_event->scanId = wmi_event->scan_id;
	scan_event->chanFreq = wmi_event->channel_freq;

	if (wmi_event->reason == WMI_SCAN_REASON_COMPLETED)
		scan_event->reasonCode = eSIR_SME_SUCCESS;
	else
		scan_event->reasonCode = eSIR_SME_SCAN_FAILED;

	if (wmi_event->event == WMI_SCAN_EVENT_COMPLETED) {
		if (wmi_event->scan_id == wma_handle->cur_scan_info.scan_id)
			wma_reset_cur_scan_info(wma_handle);
		else
			WMA_LOGE("Scan id not matched for SCAN COMPLETE event");
	}
	wma_send_msg(wma_handle, WDA_RX_SCAN_EVENT, (void *) scan_event, 0) ;
	return 0;
}

static void wma_mgmt_tx_ack_work_handler(struct work_struct *ack_work)
{
	struct wma_tx_ack_work_ctx *work = container_of(ack_work,
		struct wma_tx_ack_work_ctx, ack_cmp_work);
	pWDAAckFnTxComp ack_cb =
		work->wma_handle->umac_ota_ack_cb[work->sub_type];

	WMA_LOGD("Tx Ack Cb SubType %d Status %d",
			work->sub_type, work->status);

	/* Call the Ack Cb registered by UMAC */
	ack_cb((tpAniSirGlobal)(work->wma_handle->mac_context),
                                work->status ? 0 : 1);

	adf_os_mem_free(work);
}

/**
  * wma_mgmt_tx_ack_comp_hdlr - handles tx ack mgmt completion
  * @context: context with which the handler is registered
  * @netbuf: tx mgmt nbuf
  * @err: status of tx completion
  *
  * This is the cb registered with TxRx for
  * Ack Complete
  */
static void
wma_mgmt_tx_ack_comp_hdlr(void *wma_context,
		adf_nbuf_t netbuf, int32_t status)
{
	tpSirMacFrameCtl pFc =
		(tpSirMacFrameCtl)(adf_nbuf_data(netbuf));
	tp_wma_handle wma_handle = (tp_wma_handle)wma_context;

	if(wma_handle && wma_handle->umac_ota_ack_cb[pFc->subType]) {
		struct wma_tx_ack_work_ctx *ack_work;

		ack_work =
		adf_os_mem_alloc(NULL, sizeof(struct wma_tx_ack_work_ctx));

		if(ack_work) {
			INIT_WORK(&ack_work->ack_cmp_work,
					wma_mgmt_tx_ack_work_handler);
			ack_work->wma_handle = wma_handle;
			ack_work->sub_type = pFc->subType;
			ack_work->status = status;

			/* Schedue the Work */
			schedule_work(&ack_work->ack_cmp_work);
		}
	}
}

/**
  * wma_mgmt_tx_dload_comp_hldr - handles tx mgmt completion
  * @context: context with which the handler is registered
  * @netbuf: tx mgmt nbuf
  * @err: status of tx completion
  */
static void
wma_mgmt_tx_dload_comp_hldr(void *wma_context, adf_nbuf_t netbuf,
					int32_t status)
{
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;

	tp_wma_handle wma_handle = (tp_wma_handle)wma_context;
	void *mac_context = wma_handle->mac_context;

	WMA_LOGD("Tx Complete Status %d", status);

	if (!wma_handle->tx_frm_download_comp_cb) {
		WMA_LOGE("Tx Complete Cb not registered by umac");
		return;
	}

	/* Call Tx Mgmt Complete Callback registered by umac */
	wma_handle->tx_frm_download_comp_cb(mac_context,
					netbuf, 0);

	/* Reset Callback */
	wma_handle->tx_frm_download_comp_cb = NULL;

	/* Set the Tx Mgmt Complete Event */
	vos_status  = vos_event_set(
			&wma_handle->tx_frm_download_comp_event);
	if (!VOS_IS_STATUS_SUCCESS(vos_status))
		WMA_LOGP("Event Set failed - tx_frm_comp_event");
}

/**
  * wma_mgmt_attach - attches mgmt fn with underlying layer
  * DXE in case of Integrated, WMI incase of Discrete
  * @pwmaCtx: wma context
  * @pmacCtx: mac Context
  * @mgmt_frm_rxcb: Rx mgmt Callback
  */
VOS_STATUS wma_tx_mgmt_attach(tp_wma_handle wma_handle)
{
	/* Get the Vos Context */
	pVosContextType vos_handle =
		(pVosContextType)(wma_handle->vos_context);

	/* Get the txRx Pdev handle */
	ol_txrx_pdev_handle txrx_pdev =
		(ol_txrx_pdev_handle)(vos_handle->pdev_txrx_ctx);

	/* Register for Tx Management Frames */
	wdi_in_mgmt_tx_cb_set(txrx_pdev, GENERIC_NODOWLOAD_ACK_COMP_INDEX,
				NULL, wma_mgmt_tx_ack_comp_hdlr,wma_handle);

	wdi_in_mgmt_tx_cb_set(txrx_pdev, GENERIC_DOWNLD_COMP_NOACK_COMP_INDEX,
				wma_mgmt_tx_dload_comp_hldr, NULL, wma_handle);

	wdi_in_mgmt_tx_cb_set(txrx_pdev, GENERIC_DOWNLD_COMP_ACK_COMP_INDEX,
				wma_mgmt_tx_dload_comp_hldr,
				wma_mgmt_tx_ack_comp_hdlr,wma_handle);

	/* Store the Mac Context */
	wma_handle->mac_context = vos_handle->pMACContext;

	return VOS_STATUS_SUCCESS;
}

/**
 * wma_tx_mgmt_detach - detaches mgmt fn with underlying layer
 * Deregister with TxRx for Tx Mgmt Download and Ack completion.
 * @tp_wma_handle: wma context
 */
static VOS_STATUS wma_tx_mgmt_detach(tp_wma_handle wma_handle)
{
	u_int32_t frame_index = 0;

	/* Get the Vos Context */
	pVosContextType vos_handle =
		(pVosContextType)(wma_handle->vos_context);

	/* Get the txRx Pdev handle */
	ol_txrx_pdev_handle txrx_pdev =
		(ol_txrx_pdev_handle)(vos_handle->pdev_txrx_ctx);

	/* Deregister with TxRx for Tx Mgmt completion call back */
	for (frame_index = 0; frame_index < FRAME_INDEX_MAX; frame_index++) {
		wdi_in_mgmt_tx_cb_set(txrx_pdev, frame_index, NULL, NULL,
					txrx_pdev);
	}

	/* Destroy Tx Frame Complete event */
	vos_event_destroy(&wma_handle->tx_frm_download_comp_event);

	/* Reset Tx Frm Callbacks */
	wma_handle->tx_frm_download_comp_cb = NULL;

	return VOS_STATUS_SUCCESS;
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
	WMA_LOGD("%s: Enter", __func__);

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

	vos_status = wma_tx_mgmt_attach(wma_handle);
	if(vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("Failed to register tx management");
		goto end;
	}

end:
	WMA_LOGD("%s: Exit", __func__);
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

	WMA_LOGD("%s: Enter", __func__);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("Invalid handle");
		vos_status = VOS_STATUS_E_INVAL;
		goto end;
	}

#ifdef QCA_WIFI_ISOC
	wma_hal_stop_isoc(wma_handle);
#endif

	vos_status = wma_tx_mgmt_detach(wma_handle);
	if(vos_status != VOS_STATUS_SUCCESS) {
		WMA_LOGP("Failed to deregister tx management");
		goto end;
	}
end:
	WMA_LOGD("%s: Exit", __func__);
	return vos_status;
}

static void wma_cleanup_vdev_resp(tp_wma_handle wma)
{
	struct wma_target_req *msg, *tmp;

	list_for_each_entry_safe(msg, tmp,
				 &wma->vdev_resp_queue, node) {
		list_del(&msg->node);
		vos_timer_destroy(&msg->event_timeout);
		vos_mem_free(msg);
	}
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
	WMA_LOGD("%s: Enter", __func__);

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("Invalid handle");
		return VOS_STATUS_E_INVAL;
	}

	/* close the vos events */
	vos_event_destroy(&wma_handle->wma_ready_event);
	vos_event_destroy(&wma_handle->target_suspend);
	wma_cleanup_vdev_resp(wma_handle);
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
	vos_mem_free(wma_handle->interfaces);
	/* free the wma_handle */
	vos_free_context(wma_handle->vos_context, VOS_MODULE_ID_WDA, wma_handle);

	adf_os_mem_free(((pVosContextType) vos_ctx)->cfg_ctx);

	WMA_LOGD("%s: Exit", __func__);
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

#ifndef QCA_WIFI_ISOC
static inline void wma_update_target_services(tp_wma_handle wh,
					      struct hdd_tgt_services *cfg)
{
	/* STA power save */
	cfg->sta_power_save = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						     WMI_SERVICE_STA_PWRSAVE);

	/* Enable UAPSD */
	cfg->uapsd = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					    WMI_SERVICE_AP_UAPSD);

	/* Enable DFS channel scan */
	cfg->dfs_chan_scan = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						    WMI_SERVICE_AP_DFS);

	/* Enable 11AC */
	cfg->en_11ac = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					      WMI_SERVICE_11AC);

	/* ARP offload */
	cfg->arp_offload = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						  WMI_SERVICE_ARPNS_OFFLOAD);
}

static inline void wma_update_target_ht_cap(tp_wma_handle wh,
					    struct hdd_tgt_ht_cap *cfg)
{
	/* RX STBC */
	cfg->ht_rx_stbc = wh->ht_cap_info & WMI_HT_CAP_RX_STBC;

	/* MPDU density */
	cfg->mpdu_density = wh->ht_cap_info & WMI_HT_CAP_MPDU_DENSITY;
}

#ifdef WLAN_FEATURE_11AC
static inline void wma_update_target_vht_cap(tp_wma_handle wh,
					     struct hdd_tgt_vht_cap *cfg)
{
	/* Max MPDU length */
	if (wh->vht_cap_info & IEEE80211_VHTCAP_MAX_MPDU_LEN_3839)
		cfg->vht_max_mpdu = 3839;
	else if (wh->vht_cap_info & IEEE80211_VHTCAP_MAX_MPDU_LEN_7935)
		cfg->vht_max_mpdu = 7935;
	else if (wh->vht_cap_info & IEEE80211_VHTCAP_MAX_MPDU_LEN_11454)
		cfg->vht_max_mpdu = 11454;
	else
		cfg->vht_max_mpdu = 0;

	/* supported channel width */
	if (wh->vht_cap_info & IEEE80211_VHTCAP_SUP_CHAN_WIDTH_80)
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_80MHZ;

	else if (wh->vht_cap_info & IEEE80211_VHTCAP_SUP_CHAN_WIDTH_160)
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_160MHZ;

	else if (wh->vht_cap_info & IEEE80211_VHTCAP_SUP_CHAN_WIDTH_80_160) {
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_80MHZ;
		cfg->supp_chan_width |= 1 << eHT_CHANNEL_WIDTH_160MHZ;
	}

	else
		cfg->supp_chan_width = 0;

	/* LDPC capability */
	cfg->vht_rx_ldpc = wh->vht_cap_info & IEEE80211_VHTCAP_RX_LDPC;

	/* Guard interval */
	cfg->vht_short_gi_80 = wh->vht_cap_info & IEEE80211_VHTCAP_SHORTGI_80;
	cfg->vht_short_gi_160 = wh->vht_cap_info & IEEE80211_VHTCAP_SHORTGI_160;

	/* TX STBC capability */
	cfg->vht_tx_stbc = wh->vht_cap_info & IEEE80211_VHTCAP_TX_STBC;

	/* RX STBC capability */
	cfg->vht_rx_stbc = wh->vht_cap_info & IEEE80211_VHTCAP_RX_STBC;

	/* SU beamformer cap */
	cfg->vht_su_bformer = wh->vht_cap_info & IEEE80211_VHTCAP_SU_BFORMER;

	/* SU beamformee cap */
	cfg->vht_su_bformee = wh->vht_cap_info & IEEE80211_VHTCAP_SU_BFORMEE;

	/* MU beamformer cap */
	cfg->vht_mu_bformer = wh->vht_cap_info & IEEE80211_VHTCAP_MU_BFORMER;

	/* MU beamformee cap */
	cfg->vht_mu_bformee = wh->vht_cap_info & IEEE80211_VHTCAP_MU_BFORMEE;

	/* VHT Max AMPDU Len exp */
	cfg->vht_max_ampdu_len_exp = wh->vht_cap_info &
					IEEE80211_VHTCAP_MAX_AMPDU_LEN_EXP;

	/* VHT TXOP PS cap */
	cfg->vht_txop_ps = wh->vht_cap_info & IEEE80211_VHTCAP_TXOP_PS;
}
#endif	/* #ifdef WLAN_FEATURE_11AC */

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

	wma_update_target_services(wma_handle, &hdd_tgt_cfg.services);
	wma_update_target_ht_cap(wma_handle, &hdd_tgt_cfg.ht_cap);
#ifdef WLAN_FEATURE_11AC
	wma_update_target_vht_cap(wma_handle, &hdd_tgt_cfg.vht_cap);
#endif	/* #ifdef WLAN_FEATURE_11AC */

#ifndef QCA_WIFI_ISOC
	wma_handle->tgt_cfg_update_cb(hdd_ctx, &hdd_tgt_cfg);
#endif
}
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

	WMA_LOGD("%s: Enter", __func__);

	if (!handle || !ev) {
		WMA_LOGP("Invalid arguments");
		return;
	}

	WMA_LOGA("WMA <-- WMI_SERVICE_READY_EVENTID");

	wma_handle->phy_capability = ev->phy_capability;
	wma_handle->max_frag_entry = ev->max_frag_entry;
	vos_mem_copy(&wma_handle->reg_cap, &ev->hal_reg_capabilities,
		     sizeof(HAL_REG_CAPABILITIES));
	wma_handle->ht_cap_info = ev->ht_cap_info;
#ifdef WLAN_FEATURE_11AC
	wma_handle->vht_cap_info = ev->vht_cap_info;
#endif

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

	WMA_LOGD("%s: Enter", __func__);

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

#ifndef QCA_WIFI_ISOC
	wma_update_hdd_cfg(wma_handle);
#endif

	WMA_LOGD("Exit");
}

int wma_set_peer_param(void *wma_ctx, u_int8_t *peer_addr, u_int32_t param_id,
		       u_int32_t param_value, u_int32_t vdev_id)
{
	tp_wma_handle wma_handle = (tp_wma_handle) wma_ctx;
	wmi_peer_set_param_cmd *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wma_handle->wmi_handle,
			    sizeof(wmi_peer_set_param_cmd));
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set_param cmd");
		return -ENOMEM;
	}
	cmd = (wmi_peer_set_param_cmd *) wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param_id;
	cmd->param_value = param_value;
	err = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
				   sizeof(wmi_peer_set_param_cmd),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		WMA_LOGE("Failed to send set_param cmd");
		adf_os_mem_free(buf);
		return -EIO;
	}

	return 0;
}

/**
  * WDA_TxPacket - Sends Tx Frame to TxRx
  * This function sends the frame corresponding to the
  * given vdev id.
  * This is blocking call till the downloading of frame is complete.
  */
VOS_STATUS WDA_TxPacket(void *wma_context, void *tx_frame, u_int16_t frmLen,
			eFrameType frmType, eFrameTxDir txDir, u_int8_t tid,
			pWDATxRxCompFunc tx_frm_download_comp_cb, void *pData,
			pWDAAckFnTxComp tx_frm_ota_comp_cb, u_int8_t tx_flag,
			u_int8_t vdev_id)
{
	tp_wma_handle wma_handle = (tp_wma_handle)(wma_context);
	int32_t status;
	VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
	int32_t is_high_latency;
	ol_txrx_vdev_handle txrx_vdev;
	enum frame_index tx_frm_index =
		GENERIC_NODOWNLD_NOACK_COMP_INDEX;
	tpSirMacFrameCtl pFc = (tpSirMacFrameCtl)(adf_nbuf_data(tx_frame));
	u_int8_t use_6mbps = 0;
	u_int8_t downld_comp_required = 0;

	/* Get the vdev handle from vdev id */
	txrx_vdev = wma_handle->interfaces[vdev_id].handle;

	if(!txrx_vdev) {
		WMA_LOGE("TxRx Vdev Handle is NULL");
		return VOS_STATUS_E_FAILURE;
	}

	if (frmType >= HAL_TXRX_FRM_MAX) {
		WMA_LOGE("Invalid Frame Type Fail to send Frame");
		return VOS_STATUS_E_FAILURE;
	}

	/*
	 * Currently only support to
	 * send Mgmt is added.
	 * TODO: Cntrl and Data frames through
	 * this path
	 */
	if (frmType != HAL_TXRX_FRM_802_11_MGMT) {
		WMA_LOGE("No Support to send other frames except Mgmt");
		return VOS_STATUS_E_FAILURE;
	}

	is_high_latency = wdi_out_cfg_is_high_latency(
				txrx_vdev->pdev->ctrl_pdev);

	downld_comp_required = tx_frm_download_comp_cb && is_high_latency;

	/* Fill the frame index to send */
	if(pFc->type == SIR_MAC_MGMT_FRAME) {
		if(tx_frm_ota_comp_cb) {
			if(downld_comp_required)
				tx_frm_index =
					GENERIC_DOWNLD_COMP_ACK_COMP_INDEX;
			else
				tx_frm_index =
					GENERIC_NODOWLOAD_ACK_COMP_INDEX;

			/* Store the Ack Cb sent by UMAC */
			if(pFc->subType < SIR_MAC_MGMT_RESERVED15) {
				wma_handle->umac_ota_ack_cb[pFc->subType] =
							tx_frm_ota_comp_cb;
			}
		} else {
			if(downld_comp_required)
				tx_frm_index =
					GENERIC_DOWNLD_COMP_NOACK_COMP_INDEX;
			else
				tx_frm_index =
					GENERIC_NODOWNLD_NOACK_COMP_INDEX;
		}
	}

	/*
	 * If Dowload Complete is required
	 * Wait for download complete
	 */
	if(downld_comp_required) {
		/* Store Tx Comp Cb */
		wma_handle->tx_frm_download_comp_cb = tx_frm_download_comp_cb;

		/* Reset the Tx Frame Complete Event */
		vos_status  = vos_event_reset(
				&wma_handle->tx_frm_download_comp_event);

		if (!VOS_IS_STATUS_SUCCESS(vos_status)) {
			WMA_LOGP("Event Reset failed tx comp event %x",vos_status);
			goto error;
		}
	}

	/* If the frame has to be sent at BD Rate2 inform TxRx */
	if(tx_flag & HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME)
		use_6mbps = 1;

	/* Hand over the Tx Mgmt frame to TxRx */
	status = wdi_in_mgmt_send(txrx_vdev, tx_frame, tx_frm_index, use_6mbps);

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
	 * Wait for Download Complete
	 * if required
	 */
	if (downld_comp_required) {
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
		tx_frm_download_comp_cb(wma_handle->mac_context, tx_frame, 0);
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

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	if (NULL == wma_handle) {
		WMA_LOGP("Invalid arguments");
		VOS_ASSERT(0);
		return;
        }

	wma_handle->needShutdown  = TRUE;
	WMA_LOGD("%s: Exit", __func__);
}

/* function   : wma_rx_ready_event
 * Descriptin :
 * Args       :
 * Retruns    :
 */
 v_BOOL_t wma_needshutdown(v_VOID_t *vos_ctx)
 {
	tp_wma_handle wma_handle;

	WMA_LOGD("%s: Enter", __func__);

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

	WMA_LOGD("%s: Exit", __func__);
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

	if (wmi_unified_cmd_send(wma_handle->wmi_handle, wmibuf, len,
				    WMI_PDEV_SUSPEND_CMDID)) {
		adf_nbuf_free(wmibuf);
		return -1;
	}
	vos_event_reset(&wma_handle->target_suspend);
	return vos_wait_single_event(&wma_handle->target_suspend, 200);
}

void wma_target_suspend_complete(void *context)
{
	void *vos_context = vos_get_global_context(VOS_MODULE_ID_WDA, NULL);
	tp_wma_handle wma = vos_get_context(VOS_MODULE_ID_WDA, vos_context);

	vos_event_set(&wma->target_suspend);
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
