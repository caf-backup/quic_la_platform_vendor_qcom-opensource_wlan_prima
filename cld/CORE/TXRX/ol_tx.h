/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_tx.h
 * @brief Internal definitions for the high-level tx module.
 */
#ifndef _OL_TX__H_
#define _OL_TX__H_

#include <adf_nbuf.h>    /* adf_nbuf_t */
#include <adf_os_lock.h>
#include <ol_txrx_api.h> /* ol_txrx_vdev_handle */

#include <ol_txrx_types.h>  /* ol_tx_desc_t, ol_txrx_msdu_info_t */

adf_nbuf_t
ol_tx_ll(ol_txrx_vdev_handle vdev, adf_nbuf_t msdu_list);

#ifdef QCA_HOST_SIDE_TX_TID_CLASSIFICATION
adf_nbuf_t
ol_tx_single_non_std_ll(
    ol_txrx_vdev_handle vdev,
    u_int8_t ext_tid,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu);
#endif /* QCA_HOST_SIDE_TX_TID_CLASSIFICATION */
adf_nbuf_t
ol_tx_non_std_ll(
    ol_txrx_vdev_handle data_vdev,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu_list);

adf_nbuf_t
ol_tx_hl(ol_txrx_vdev_handle vdev, adf_nbuf_t msdu_list);

adf_nbuf_t
ol_tx_non_std_hl(
    ol_txrx_vdev_handle data_vdev,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu_list);

adf_nbuf_t 
ol_tx_reinject(struct ol_txrx_vdev_t *vdev, adf_nbuf_t msdu, uint32_t peer_id);

void
ol_txrx_mgmt_tx_complete(void *ctxt, adf_nbuf_t netbuf, int err);

#endif /* _OL_TX__H_ */
