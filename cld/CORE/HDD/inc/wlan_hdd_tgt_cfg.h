/*
* Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

#ifndef HDD_TGT_CFG_H
#define HDD_TGT_CFG_H

struct hdd_tgt_cfg {
	u_int8_t band_cap;
	u_int8_t alpha2[3];
};

typedef void (*hdd_tgt_cfg_cb) (void *, struct hdd_tgt_cfg *);
#endif /* HDD_TGT_CFG_H */
