/*
* Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

#ifndef HDD_TGT_CFG_H
#define HDD_TGT_CFG_H

/* TODO: Find it from the max number of supported vdev */
#define INTF_MACADDR_MASK	0x7

struct hdd_tgt_services {
	u_int32_t sta_power_save;
	u_int32_t uapsd;
	u_int32_t dfs_chan_scan;
	u_int32_t en_11ac;
	u_int32_t arp_offload;
};

struct hdd_tgt_ht_cap {
	u_int32_t mpdu_density;
	bool ht_rx_stbc;
	bool ht_tx_stbc;
	bool ht_rx_ldpc;
	bool ht_sgi_20;
	bool ht_sgi_40;
	u_int32_t num_rf_chains;
};

#ifdef WLAN_FEATURE_11AC
struct hdd_tgt_vht_cap {
	u_int32_t vht_max_mpdu;
	u_int32_t supp_chan_width;
	u_int32_t vht_rx_ldpc;
	u_int32_t vht_short_gi_80;
	u_int32_t vht_short_gi_160;
	u_int32_t vht_tx_stbc;
	u_int32_t vht_rx_stbc;
	u_int32_t vht_su_bformer;
	u_int32_t vht_su_bformee;
	u_int32_t vht_mu_bformer;
	u_int32_t vht_mu_bformee;
	u_int32_t vht_max_ampdu_len_exp;
	u_int32_t vht_txop_ps;
};
#endif

struct hdd_tgt_cfg {
	u_int8_t band_cap;
	u_int8_t alpha2[3];
	v_MACADDR_t hw_macaddr;
	struct hdd_tgt_services services;
	struct hdd_tgt_ht_cap ht_cap;
#ifdef WLAN_FEATURE_11AC
	struct hdd_tgt_vht_cap vht_cap;
#endif
};

#endif /* HDD_TGT_CFG_H */
