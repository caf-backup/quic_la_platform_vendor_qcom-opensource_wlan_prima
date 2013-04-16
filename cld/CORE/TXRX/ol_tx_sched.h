/*
 * Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_tx_sched.h
 * @brief API definitions for the tx scheduler module within the data SW.
 */
#ifndef _OL_TX_SCHED__H_
#define _OL_TX_SCHED__H_

enum ol_tx_queue_action {
    OL_TX_ENQUEUE_FRAME,
    OL_TX_DELETE_QUEUE,
    OL_TX_PAUSE_QUEUE,
    OL_TX_UNPAUSE_QUEUE,
    OL_TX_DISCARD_FRAMES,
};

struct ol_tx_sched_notify_ctx_t {
    int event;
    struct ol_tx_frms_queue_t *txq;
    union {
        int ext_tid;
        struct ol_txrx_msdu_info_t *tx_msdu_info;
    } info;
    int frames;
    int bytes;
};

#if defined(CONFIG_HL_SUPPORT)

void
ol_tx_sched_notify(
    struct ol_txrx_pdev_t *pdev, struct ol_tx_sched_notify_ctx_t *ctx);

void
ol_tx_sched(struct ol_txrx_pdev_t *pdev);

int
ol_tx_sched_discard_select(
    struct ol_txrx_pdev_t *pdev, int frms, ol_tx_desc_list *tx_descs);

void *
ol_tx_sched_attach(struct ol_txrx_pdev_t *pdev);

void
ol_tx_sched_detach(struct ol_txrx_pdev_t *pdev);

#else

#define ol_tx_notify_sched(pdev, ctx) /* no-op */
#define ol_tx_sched(pdev) /* no-op */
#define ol_tx_sched_discard_select(pdev) 0
#define ol_tx_sched_attach(pdev) NULL
#define ol_tx_sched_detach(pdev) /* no-op */

#endif /* defined(CONFIG_HL_SUPPORT) */

#endif /* _OL_TX_SCHED__H_ */

