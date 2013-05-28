/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef TXRX_TL_SHIM_H
#define TXRX_TL_SHIM_H

struct tlshim_sta_info {
	bool registered;
	WLANTL_STARxCBType data_rx;
};

struct txrx_tl_shim_ctx {
	void *cfg_ctx;
	ol_txrx_tx_fp tx;
	WLANTL_MgmtFrmRxCBType mgmt_rx;
	struct tlshim_sta_info sta_info[WLAN_MAX_STA_COUNT];
};

#endif
