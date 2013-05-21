/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef TXRX_TL_SHIM_H
#define TXRX_TL_SHIM_H

struct txrx_tl_shim_ctx {
	void *cfg_ctx;
	ol_txrx_tx_fp tx;
	WLANTL_MgmtFrmRxCBType mgmt_rx;
};

#endif
