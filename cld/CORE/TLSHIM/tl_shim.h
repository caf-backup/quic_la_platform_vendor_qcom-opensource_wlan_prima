/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef TXRX_TL_SHIM_H
#define TXRX_TL_SHIM_H

struct tlshim_buf {
	struct list_head list;
	adf_nbuf_t buf;
};

struct tlshim_sta_info {
	bool registered;
	bool suspend_flush;
	WLANTL_STARxCBType data_rx;
	struct list_head cached_bufq;
};

struct txrx_tl_shim_ctx {
	void *cfg_ctx;
	ol_txrx_tx_fp tx;
	WLANTL_MgmtFrmRxCBType mgmt_rx;
	struct tlshim_sta_info sta_info[WLAN_MAX_STA_COUNT];
	adf_os_spinlock_t bufq_lock;
	struct work_struct cache_flush_work;
};

/*
 * APIs used by CLD specific components, as of now these are used only
 * in WMA.
 */
void WLANTL_RegisterVdev(void *vos_ctx, void *vdev);
#endif
