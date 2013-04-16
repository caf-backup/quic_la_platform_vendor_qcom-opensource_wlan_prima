/*
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
#ifndef _OL_RX_REORDER_TIMEOUT__H_
#define _OL_RX_REORDER_TIMEOUT__H_

#include <ol_txrx_types.h> /* ol_txrx_pdev_t, etc. */

#ifdef QCA_SUPPORT_OL_RX_REORDER_TIMEOUT

void
ol_rx_reorder_timeout_init(struct ol_txrx_pdev_t *pdev);
void
ol_rx_reorder_timeout_cleanup(struct ol_txrx_pdev_t *pdev);
void
ol_rx_reorder_timeout_remove(struct ol_txrx_peer_t *peer, unsigned tid);
void
ol_rx_reorder_timeout_update(struct ol_txrx_peer_t *peer, unsigned tid);
void
ol_rx_reorder_timeout_peer_cleanup(struct ol_txrx_peer_t *peer);

#define OL_RX_REORDER_TIMEOUT_INIT    ol_rx_reorder_timeout_init
#define OL_RX_REORDER_TIMEOUT_PEER_CLEANUP ol_rx_reorder_timeout_peer_cleanup
#define OL_RX_REORDER_TIMEOUT_CLEANUP ol_rx_reorder_timeout_cleanup
#define OL_RX_REORDER_TIMEOUT_REMOVE  ol_rx_reorder_timeout_remove
#define OL_RX_REORDER_TIMEOUT_UPDATE  ol_rx_reorder_timeout_update
#define OL_RX_REORDER_TIMEOUT_PEER_TID_INIT(peer, tid) \
    (peer)->tids_rx_reorder[(tid)].timeout.active = 0
#define OL_RX_REORDER_TIMEOUT_MUTEX_LOCK(pdev) \
    adf_os_spin_lock(&(pdev)->rx.mutex)
#define OL_RX_REORDER_TIMEOUT_MUTEX_UNLOCK(pdev) \
    adf_os_spin_unlock(&(pdev)->rx.mutex)

#else

#define OL_RX_REORDER_TIMEOUT_INIT(pdev)               /* no-op */
#define OL_RX_REORDER_TIMEOUT_PEER_CLEANUP(peer)       /* no-op */
#define OL_RX_REORDER_TIMEOUT_CLEANUP(pdev)            /* no-op */
#define OL_RX_REORDER_TIMEOUT_REMOVE(peer, tid)        /* no-op */
#define OL_RX_REORDER_TIMEOUT_UPDATE(peer, tid)        /* no-op */
#define OL_RX_REORDER_TIMEOUT_PEER_TID_INIT(peer, tid) /* no-op */
#define OL_RX_REORDER_TIMEOUT_MUTEX_LOCK(pdev)         /* no-op */
#define OL_RX_REORDER_TIMEOUT_MUTEX_UNLOCK(pdev)       /* no-op */

#endif /* QCA_SUPPORT_OL_RX_REORDER_TIMEOUT */

#endif /* _OL_RX_REORDER_TIMEOUT__H_ */
