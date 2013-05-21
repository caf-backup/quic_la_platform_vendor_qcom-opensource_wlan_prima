/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include "vos_sched.h"
#include "wlan_qct_tl.h"
#include "wdi_in.h"
#include "ol_txrx_peer_find.h"
#include "tl_shim.h"
#include "wma.h"
#include "wmi_unified_api.h"
#ifdef QCA_WIFI_ISOC
#include "htt_dxe_types.h"
#endif

#define ENTER() VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO, "Enter:%s", __func__)

#define TLSHIM_LOGD(args...) \
	VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO, ## args)
#define TLSHIM_LOGW(args...) \
	VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_WARNING, ## args)
#define TLSHIM_LOGE(args...) \
	VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR, ## args)
#define TLSHIM_LOGP(args...) \
	VOS_TRACE( VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_FATAL, ## args)

/************************/
/*    Internal Func	*/
/************************/

#ifdef QCA_WIFI_ISOC
static void tlshim_mgmt_rx_dxe_handler(void *context, adf_nbuf_t buflist)
{
	/* TODO: Fill in meta info and send the frame to PE */
}
#else

static int tlshim_mgmt_rx_wmi_handler(void *context, u_int8_t *data,
				       u_int16_t data_len)
{
	/* TODO: Fill in meta info and send the frame to PE */
	return 0;
}
#endif

/*************************/
/*	TL APIs		 */
/*************************/

VOS_STATUS WLANTL_ResumeDataTx(void *vos_ctx, u_int8_t *sta_id)
{
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_SuspendDataTx(void *vos_ctx, u_int8_t *sta_id,
				WLANTL_SuspendCBType suspend_tx_cb)
{
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_TxBAPFrm(void *vos_ctx, vos_pkt_t *buf,
			   WLANTL_MetaInfoType *meta_info,
			   WLANTL_TxCompCBType txcomp_cb)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

void WLANTL_AssocFailed(u_int8_t sta_id)
{
	/* Not needed */
}

VOS_STATUS WLANTL_Finish_ULA(void (*cb) (void *cb_ctx), void *cb_ctx)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

void WLANTLPrintPktsRcvdPerRssi(void *vos_ctx, u_int8_t sta_id, bool flush)
{
	/* TBD */
}

void WLANTLPrintPktsRcvdPerRateIdx(void *vos_ctx, u_int8_t sta_id, bool flush)
{
	/* TBD */
}

VOS_STATUS WLANTL_TxProcessMsg(void *vos_ctx, vos_msg_t *msg)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_McProcessMsg(void *vos_ctx, vos_msg_t *message)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_McFreeMsg(void *vos_ctx, vos_msg_t *message)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_TxFreeMsg(void *vos_ctx, vos_msg_t *message)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_RegisterBAPClient(void *vos_ctx,
				    WLANTL_BAPRxCBType bap_rx,
				    WLANTL_FlushOpCompCBType flush_cb)
{
	/* Not needed */
	return VOS_STATUS_SUCCESS;
}

/*
 * Txrx does weighted RR scheduling, set/get ac weights does not
 * apply here, this is no operation.
 */
VOS_STATUS WLANTL_SetACWeights(void *vos_ctx, u_int8_t *ac_weight)
{
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_GetACWeights(void *vos_ctx, u_int8_t *ac_weight)
{
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_GetSoftAPStatistics(void *vos_ctx,
				      WLANTL_TRANSFER_STA_TYPE *stats_sum,
				      v_BOOL_t reset)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

/*
 * Return txrx stats for a given sta_id
 */
VOS_STATUS WLANTL_GetStatistics(void *vos_ctx,
				WLANTL_TRANSFER_STA_TYPE *stats_buf,
				u_int8_t sta_id)
{
	/*
	 * TODO: Txrx to be modified to maintain per peer stats which
	 * TL shim can return whenever requested.
	 */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_DeregRSSIIndicationCB(void *adapter, v_S7_t rssi,
					u_int8_t trig_evt,
					WLANTL_RSSICrossThresholdCBType func,
					VOS_MODULE_ID mod_id)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_RegRSSIIndicationCB(void *adapter, v_S7_t rssi,
				      u_int8_t trig_evt,
				      WLANTL_RSSICrossThresholdCBType func,
				      VOS_MODULE_ID mod_id, void *usr_ctx)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_EnableUAPSDForAC(void *vos_ctx, u_int8_t sta_id,
				   WLANTL_ACEnumType ac, u_int8_t tid,
				   u_int8_t pri, v_U32_t srvc_int,
				   v_U32_t sus_int, WLANTL_TSDirType dir)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_DisableUAPSDForAC(void *vos_ctx, u_int8_t sta_id,
				    WLANTL_ACEnumType ac)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_DeRegisterMgmtFrmClient(void *vos_ctx)
{
	struct txrx_tl_shim_ctx *tl_shim = vos_get_context(VOS_MODULE_ID_TL,
							   vos_ctx);

#ifdef QCA_WIFI_ISOC
	 ol_txrx_pdev_handle txrx_pdev = vos_get_context(VOS_MODULE_ID_TXRX,
							 vos_ctx);
	 struct htt_dxe_pdev_t *htt_dxe_pdev = txrx_pdev->htt_pdev;

	if (dmux_dxe_register_callback_rx_mgmt(htt_dxe_pdev->dmux_dxe_pdev,
					       NULL, NULL) != 0) {
		TLSHIM_LOGE("Failed to Unregister rx mgmt handler with dxe");
		return VOS_STATUS_E_FAILURE;
	}
#else
	tp_wma_handle wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	if (wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
						 WMI_MGMT_RX_EVENTID) != 0) {
		TLSHIM_LOGE("Failed to Unregister rx mgmt handler with wmi");
		return VOS_STATUS_E_FAILURE;
	}
#endif
	tl_shim->mgmt_rx = NULL;
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_RegisterMgmtFrmClient(void *vos_ctx,
					WLANTL_MgmtFrmRxCBType mgmt_frm_rx)
{
	struct txrx_tl_shim_ctx *tl_shim = vos_get_context(VOS_MODULE_ID_TL,
							   vos_ctx);

#ifdef QCA_WIFI_ISOC
	 ol_txrx_pdev_handle txrx_pdev = vos_get_context(VOS_MODULE_ID_TXRX,
							 vos_ctx);
	 struct htt_dxe_pdev_t *htt_dxe_pdev = txrx_pdev->htt_pdev;

	if (dmux_dxe_register_callback_rx_mgmt(htt_dxe_pdev->dmux_dxe_pdev,
					       tlshim_mgmt_rx_dxe_handler,
					       tl_shim) != 0) {
		TLSHIM_LOGE("Failed to register rx mgmt handler with dxe");
		return VOS_STATUS_E_FAILURE;
	}
#else
	tp_wma_handle wma_handle = vos_get_context(VOS_MODULE_ID_WDA, vos_ctx);

	if (wmi_unified_register_event_handler(wma_handle->wmi_handle,
					       WMI_MGMT_RX_EVENTID,
					       tlshim_mgmt_rx_wmi_handler)
					       != 0) {
		TLSHIM_LOGE("Failed to register rx mgmt handler with wmi");
		return VOS_STATUS_E_FAILURE;
	}
#endif
	tl_shim->mgmt_rx = mgmt_frm_rx;

	return VOS_STATUS_SUCCESS;
}

/*
 * Return the data rssi for the given peer.
 */
VOS_STATUS WLANTL_GetRssi(void *vos_ctx, u_int8_t sta_id, v_S7_t *rssi)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

/*
 * HDD will directly call tx function with the skb for transmission.
 * Txrx is reponsible to enqueue the packet and schedule it for Hight
 * Latency devices, so this API is not used for CLD.
 */
VOS_STATUS WLANTL_STAPktPending(void *vos_ctx, u_int8_t sta_id,
				WLANTL_ACEnumType ac)
{
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_UpdateSTABssIdforIBSS(void *vos_ctx, u_int8_t sta_id,
					u_int8_t *bssid)
{
	/* TBD */
	return VOS_STATUS_SUCCESS;
}

/*
 * In CLD, sec_type along with the peer_state will be used to
 * make sure EAPOL frame after PTK is installed is getting encrypted.
 * So this API is no-op.
 */
VOS_STATUS WLANTL_STAPtkInstalled(void *vos_ctx, u_int8_t sta_id)
{
	return VOS_STATUS_SUCCESS;
}

/*
 * HDD calls this to notify the state change in client.
 * Txrx will do frame filtering.
 */
VOS_STATUS WLANTL_ChangeSTAState(void *vos_ctx, u_int8_t sta_id,
				 WLANTL_STAStateType sta_state)
{
	struct ol_txrx_peer_t *peer;
	enum ol_txrx_peer_state txrx_state = ol_txrx_peer_state_open;

	ENTER();
	peer = ol_txrx_peer_find_by_id(
			((pVosContextType) vos_ctx)->pdev_txrx_ctx,
			sta_id);
	if (!peer)
		return VOS_STATUS_E_FAULT;

	if (sta_state == WLANTL_STA_CONNECTED)
		txrx_state = ol_txrx_peer_state_conn;
	else if (sta_state == WLANTL_STA_AUTHENTICATED)
		txrx_state = ol_txrx_peer_state_auth;

	ol_txrx_peer_state_update(peer->vdev->pdev,
				  (u_int8_t *) peer->mac_addr.raw,
				  txrx_state);

	if (txrx_state == ol_txrx_peer_state_auth) {
		/*
		 * TODO:Send WMI_PEER_SET_PARAM_CMDID to set
		 * the peer sate to authorized.
		 */
	}
	return VOS_STATUS_SUCCESS;
}

/*
 * Nothing to be done for CLD. For CLD, detaching the peer should take
 * care of deregistering the client from data service.
 */
VOS_STATUS WLANTL_ClearSTAClient(void *vos_ctx, u_int8_t sta_id)
{
	return VOS_STATUS_SUCCESS;
}

/*
 * Register a station for data service. This API gives flexibility
 * to register different callbacks for different client though it is
 * needed to register different callbacks for every vdev. Only rxcb
 * is used.
 */
VOS_STATUS WLANTL_RegisterSTAClient(void *vos_ctx,
				    WLANTL_STARxCBType rxcb,
				    WLANTL_TxCompCBType tx_comp,
				    WLANTL_STAFetchPktCBType txpkt_fetch,
				    WLAN_STADescType *sta_desc, v_S7_t rssi)
{
	struct txrx_tl_shim_ctx *tl_shim;
	struct ol_txrx_peer_t *peer;
	struct ol_txrx_osif_ops txrx_ops;
	ol_txrx_peer_update_param_t param;

	ENTER();
	peer = ol_txrx_peer_find_by_id(
			((pVosContextType) vos_ctx)->pdev_txrx_ctx,
			sta_desc->ucSTAId);
	if (!peer)
		return VOS_STATUS_E_FAULT;

	tl_shim = vos_get_context(VOS_MODULE_ID_TL, vos_ctx);
	/* TODO: Change HDD rx callback proto type */
	/*	txrx_ops.rx.std = rxcb; */
	/*
	 * TODO: Change HDD to pass adapter so that we can
	 * pass adapter instead of tl_shim as context to txrx
	 * for rx callback.
	 */
	wdi_in_osif_vdev_register(peer->vdev, tl_shim, &txrx_ops);
	tl_shim->tx = txrx_ops.tx.std;
	param.qos_capable =  sta_desc->ucQosEnabled;
	wdi_in_peer_update(peer->vdev, peer->mac_addr.raw, &param,
			   ol_txrx_peer_update_qos_capable);
	if (sta_desc->ucIsWapiSta) {
		/* param.sec_type = ol_sec_type_wapi; */
		/*
		 * TODO: Peer update also updates the other security types
		 * but HDD will not pass this information.

		wdi_in_peer_update(peer->vdev, peer->mac_addr.raw, &param,
				   ol_txrx_peer_update_peer_security);
		 */
	}
	return VOS_STATUS_SUCCESS;
}

VOS_STATUS WLANTL_Stop(void *vos_ctx)
{
	/* Nothing to do really */
	return VOS_STATUS_SUCCESS;
}

/*
 * Make txrx module ready
 */
VOS_STATUS WLANTL_Start(void *vos_ctx)
{
	ENTER();
	if (wdi_in_pdev_attach_target(((pVosContextType)
				      vos_ctx)->pdev_txrx_ctx))
		return VOS_STATUS_E_FAULT;
	return VOS_STATUS_SUCCESS;
}

/*
 * Deinit txrx module
 */
VOS_STATUS WLANTL_Close(void *vos_ctx)
{
	struct txrx_tl_shim_ctx *tl_shim;

	ENTER();
	tl_shim = vos_get_context(VOS_MODULE_ID_TL, vos_ctx);
	wdi_in_pdev_detach(((pVosContextType) vos_ctx)->pdev_txrx_ctx, 1);
	vos_free_context(vos_ctx, VOS_MODULE_ID_TL, tl_shim);
	return VOS_STATUS_SUCCESS;
}

/*
 * Allocate and Initialize transport layer (txrx)
 */
VOS_STATUS WLANTL_Open(void *vos_ctx, WLANTL_ConfigInfoType *tl_cfg)
{
	struct txrx_tl_shim_ctx *tl_shim;
	VOS_STATUS status;

	ENTER();
	status = vos_alloc_context(vos_ctx, VOS_MODULE_ID_TL,
				   (void *) &tl_shim, sizeof(*tl_shim));
	if (status != VOS_STATUS_SUCCESS)
		return status;

	((pVosContextType) vos_ctx)->pdev_txrx_ctx =
				wdi_in_pdev_attach(
					((pVosContextType) vos_ctx)->cfg_ctx,
					((pVosContextType) vos_ctx)->htc_ctx,
					((pVosContextType) vos_ctx)->adf_ctx);
	if (!((pVosContextType) vos_ctx)->pdev_txrx_ctx) {
		TLSHIM_LOGE("Failed to allocate memory for pdev txrx handle");
		return VOS_STATUS_E_NOMEM;
	}

	/*
	 * TODO: Allocate memory for tx callback for maximum supported
	 * vdevs to maintain tx callbacks per vdev.
	 */

	return VOS_STATUS_SUCCESS;
}
