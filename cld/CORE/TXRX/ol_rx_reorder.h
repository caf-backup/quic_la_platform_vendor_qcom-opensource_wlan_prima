/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef _OL_RX_REORDER__H_
#define _OL_RX_REORDER__H_

#include <adf_nbuf.h>        /* adf_nbuf_t, etc. */

#include <ol_txrx_api.h>     /* ol_txrx_peer_t, etc. */

#include <ol_txrx_types.h>   /* ol_rx_reorder_t */

void
ol_rx_reorder_store(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    unsigned reorder_array_index,
    adf_nbuf_t head_msdu,
    adf_nbuf_t tail_msdu);

void
ol_rx_reorder_release(
    struct ol_txrx_vdev_t *vdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    unsigned seq_num_start,
    unsigned seq_num_end);

void
ol_rx_reorder_flush(
    struct ol_txrx_vdev_t *vdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    unsigned seq_num_start,
    unsigned seq_num_end,
    enum htt_rx_flush_action action);

/**
 * @brief - find end of first range of present MPDUs after the initial rx hole
 * @param[in] peer - which sender's data is being checked
 * @param[in] tid - which type of data is being checked
 * @param[out] idx_end - the reorder array index holding the last MPDU in the
 *      range of in-order MPDUs that following the initial hole.
 *      Note that this is the index of the last in-order MPDU following the
 *      first hole, rather than the starting index of the second hole.
 */
void
ol_rx_reorder_first_hole(
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    unsigned *idx_end);

void
ol_rx_reorder_peer_cleanup(
    struct ol_txrx_vdev_t *vdev, struct ol_txrx_peer_t *peer);

void
ol_rx_reorder_init(struct ol_rx_reorder_t *rx_reorder, u_int8_t tid);

enum htt_rx_status
ol_rx_reorder_seq_num_check(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer,
    unsigned tid,
    unsigned seq_num);

#ifdef QCA_WIFI_ISOC

/*
 * Riva, Pronto, Northstar: do sequence number checking in the host
 * for peer-TIDs with aggregation is enabled
 */
#define OL_RX_REORDER_SEQ_NUM_CHECK(pdev, peer, tid, rx_mpdu_desc) \
  (peer->tids_rx_reorder[tid].win_sz_mask != 0) ? \
      ol_rx_reorder_seq_num_check( \
          pdev, peer, tid, \
          htt_rx_mpdu_desc_seq_num(pdev->htt_pdev, rx_mpdu_desc)) : \
      htt_rx_status_ok

#else

/*
 * Peregrine and Rome: do sequence number checking in the host
 * for peer-TIDs without aggregation enabled
 */
#define OL_RX_REORDER_SEQ_NUM_CHECK(pdev, peer, tid, rx_mpdu_desc) \
  (pdev->rx.flags.dup_check && peer->tids_rx_reorder[tid].win_sz_mask == 0) ? \
      ol_rx_reorder_seq_num_check( \
          pdev, peer, tid, \
          htt_rx_mpdu_desc_seq_num(pdev->htt_pdev, rx_mpdu_desc)) : \
      htt_rx_status_ok

#endif /* QCA_WIFI_ISOC */


#endif /* _OL_RX_REORDER__H_ */
