/*
 * Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <adf_nbuf.h>         /* adf_nbuf_t, etc. */
#include <adf_os_atomic.h>    /* adf_os_atomic_add, etc. */
#include <htt.h>              /* HTT_TX_EXT_TID_MGMT */
#include <ol_htt_tx_api.h>    /* htt_tx_desc_tid */
#include <ol_txrx_api.h>      /* ol_txrx_vdev_handle */
#include <ol_txrx_ctrl_api.h> /* ol_txrx_sync, ol_tx_addba_conf */
#include <ol_ctrl_txrx_api.h> /* ol_ctrl_addba_req */
#include <ol_txrx_internal.h> /* TXRX_ASSERT1, etc. */
#include <ol_txrx_types.h>    /* pdev stats */
#include <ol_tx_desc.h>       /* ol_tx_desc, ol_tx_desc_frame_list_free */
#include <ol_tx_sched.h>      /* ol_tx_sched_notify, etc. */
#include <ol_tx_queue.h>
#include <ol_txrx_dbg.h>      /* ENABLE_TX_QUEUE_LOG */

#if defined(CONFIG_HL_SUPPORT)

#ifndef offsetof
#define offsetof(type, field)   ((size_t)(&((type *)0)->field))
#endif

/*--- function prototypes for optional queue log feature --------------------*/
#if defined(ENABLE_TX_QUEUE_LOG)

void
ol_tx_queue_log_enqueue(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_msdu_info_t *msdu_info,
    int frms, int bytes);
void
ol_tx_queue_log_dequeue(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    int frms, int bytes);
void
ol_tx_queue_log_free(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    int tid, int frms, int bytes);
#define OL_TX_QUEUE_LOG_ENQUEUE ol_tx_queue_log_enqueue
#define OL_TX_QUEUE_LOG_DEQUEUE ol_tx_queue_log_dequeue
#define OL_TX_QUEUE_LOG_FREE    ol_tx_queue_log_free

#else

#define OL_TX_QUEUE_LOG_ENQUEUE(pdev, msdu_info, frms, bytes) /* no-op */
#define OL_TX_QUEUE_LOG_DEQUEUE(pdev, txq, frms, bytes) /* no-op */
#define OL_TX_QUEUE_LOG_FREE(pdev, txq, tid, frms, bytes) /* no-op */

#endif /* TXRX_DEBUG_LEVEL > 5 */


/*--- function prototypes for optional host ADDBA negotiation ---------------*/
#ifdef FEATURE_WLAN_INTEGRATED_SOC

void
ol_tx_queue_addba_check(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    struct ol_txrx_msdu_info_t *tx_msdu_info);
#define OL_TX_QUEUE_ADDBA_CHECK ol_tx_queue_addba_check

#else

#define OL_TX_QUEUE_ADDBA_CHECK(pdev, txq, tx_msdu_info) /* no-op */

#endif /* QCA_SUPPORT_HOST_ADDBA */

#ifndef container_of
#define container_of(ptr, type, member) ((type *)( \
                (char *)(ptr) - (char *)(&((type *)0)->member) ) )
#endif


/*--- function definitions --------------------------------------------------*/

void
ol_tx_queue_discard(struct ol_txrx_pdev_t *pdev)
{
    ol_tx_desc_list tx_descs;
    int num;

    TAILQ_INIT(&tx_descs);
    num = pdev->tx_queue.rsrc_threshold_hi - pdev->tx_queue.rsrc_threshold_lo;
    while (num > 0) {
        int discarded = ol_tx_sched_discard_select(pdev, num, &tx_descs);
        if (discarded == 0) {
            break; /* couldn't find any to discard - give up */
        }
        num -= discarded;
        adf_os_atomic_add(discarded, &pdev->tx_queue.rsrc_cnt);
    }
    ol_tx_desc_frame_list_free(pdev, &tx_descs, 1 /* error */);
}

void
ol_tx_enqueue(
    struct ol_txrx_pdev_t *pdev, 
    struct ol_tx_frms_queue_t *txq, 
    struct ol_tx_desc_t *tx_desc, 
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    int bytes;
    struct ol_tx_sched_notify_ctx_t notify_ctx;
    
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    /*
     * If too few tx descriptors are available, drop some currently-queued
     * tx frames, to provide enough tx descriptors for new frames, which
     * may be higher priority than the current frames.
     */
    if (adf_os_atomic_read(&pdev->tx_queue.rsrc_cnt) <=
        pdev->tx_queue.rsrc_threshold_lo)
    {
        ol_tx_queue_discard(pdev);
    }
    TAILQ_INSERT_TAIL(&txq->head, tx_desc, tx_desc_list_elem);

    bytes = adf_nbuf_len(tx_desc->netbuf); 
    txq->frms++;
    txq->bytes += bytes;
    OL_TX_QUEUE_LOG_ENQUEUE(pdev, tx_msdu_info, 1, bytes);

    if (txq->flag != ol_tx_queue_paused) {
        notify_ctx.event = OL_TX_ENQUEUE_FRAME;
        notify_ctx.frames = 1;
        notify_ctx.bytes = adf_nbuf_len(tx_desc->netbuf);
        notify_ctx.txq = txq;
        notify_ctx.info.tx_msdu_info = tx_msdu_info;
        ol_tx_sched_notify(pdev, &notify_ctx);
        txq->flag = ol_tx_queue_active;
    }

    OL_TX_QUEUE_ADDBA_CHECK(pdev, txq, tx_msdu_info);

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

int
ol_tx_dequeue(
    struct ol_txrx_pdev_t *pdev, 
    struct ol_tx_frms_queue_t *txq, 
    ol_tx_desc_list *head,
    int max_frames,
    int *credit,
    int *bytes)
{
    int num_frames;
    int bytes_sum;
    unsigned credit_sum;

    TXRX_ASSERT2(txq->flag != ol_tx_queue_paused);
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);

    if (txq->frms < max_frames) {
        max_frames = txq->frms;
    }
    bytes_sum = 0;
    credit_sum = 0;
    for (num_frames = 0; num_frames < max_frames; num_frames++) {
        unsigned frame_credit;
        struct ol_tx_desc_t *tx_desc;
        tx_desc = TAILQ_FIRST(&txq->head);

        frame_credit = htt_tx_msdu_credit(tx_desc->netbuf);
        if (credit_sum + frame_credit > *credit) {
            break;
        }
        credit_sum += frame_credit;
        bytes_sum += adf_nbuf_len(tx_desc->netbuf);
        TAILQ_REMOVE(&txq->head, tx_desc, tx_desc_list_elem);
        TAILQ_INSERT_TAIL(head, tx_desc, tx_desc_list_elem);
    }
    txq->frms -= num_frames;
    txq->bytes -= bytes_sum;
    /* a paused queue remains paused, regardless of whether it has frames */
    if (txq->frms == 0 && txq->flag == ol_tx_queue_active) {
        txq->flag = ol_tx_queue_empty;
    }
    OL_TX_QUEUE_LOG_DEQUEUE(pdev, txq, num_frames, bytes_sum);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);

    *bytes = bytes_sum;
    *credit = credit_sum;
    return num_frames;
}

void
ol_tx_queue_free(
    struct ol_txrx_pdev_t *pdev, 
    struct ol_tx_frms_queue_t *txq,
    int tid)
{
    int frms = 0, bytes = 0;
    struct ol_tx_desc_t *tx_desc;
    struct ol_tx_sched_notify_ctx_t notify_ctx;

    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    frms = txq->frms;
    while (txq->frms) {
        tx_desc = TAILQ_FIRST(&txq->head);
        TAILQ_REMOVE(&txq->head, tx_desc, tx_desc_list_elem);
        bytes += adf_nbuf_len(tx_desc->netbuf);
        ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 0);
        txq->frms--;
    }
    OL_TX_QUEUE_LOG_FREE(pdev, txq, tid, frms, bytes);

    notify_ctx.event = OL_TX_DELETE_QUEUE;
    notify_ctx.txq = txq;
    notify_ctx.info.ext_tid = tid;
    ol_tx_sched_notify(pdev, &notify_ctx);
    txq->flag = ol_tx_queue_empty;

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);    
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}


/*--- queue pause / unpause functions ---------------------------------------*/

static inline void
ol_txrx_peer_tid_pause_base(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer,
    int tid)
{
    struct ol_tx_frms_queue_t *txq = &peer->txqs[tid];

    if (txq->paused_count.total++ == 0) {
        struct ol_tx_sched_notify_ctx_t notify_ctx;

        notify_ctx.event = OL_TX_PAUSE_QUEUE;
        notify_ctx.txq = txq;
        notify_ctx.info.ext_tid = tid;
        ol_tx_sched_notify(pdev, &notify_ctx);
        txq->flag = ol_tx_queue_paused;
    }
}

static inline void
ol_txrx_peer_pause_base(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer)
{
    int i;
    for (i = 0; i < ARRAY_LEN(peer->txqs); i++) {
        ol_txrx_peer_tid_pause_base(pdev, peer, i);
    }
}

static inline void
ol_txrx_peer_tid_unpause_base(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer,
    int tid)
{
    struct ol_tx_frms_queue_t *txq = &peer->txqs[tid];
    /*
     * Don't actually unpause the tx queue until all pause requests
     * have been removed.
     */
    TXRX_ASSERT2(txq->paused_count.total > 0);
    if (--txq->paused_count.total == 0) {
        struct ol_tx_sched_notify_ctx_t notify_ctx;

        notify_ctx.event = OL_TX_UNPAUSE_QUEUE;
        notify_ctx.txq = txq;
        notify_ctx.info.ext_tid = tid;
        ol_tx_sched_notify(pdev, &notify_ctx);

        if (txq->frms == 0) {
            txq->flag = ol_tx_queue_empty;
        } else {
            txq->flag = ol_tx_queue_active;
            /*
             * Now that the are new tx frames available to download,
             * invoke the scheduling function, to see if it wants to
             * download the new frames.
             * Since the queue lock is currently held, and since
             * the scheduler function takes the lock, temporarily
             * release the lock.
             */
            adf_os_spin_unlock(&pdev->tx_queue_spinlock);
            ol_tx_sched(pdev);
            adf_os_spin_lock(&pdev->tx_queue_spinlock);
        }
    }
}

void
ol_txrx_peer_pause(ol_txrx_peer_handle peer)
{
    struct ol_txrx_pdev_t *pdev = peer->vdev->pdev;

    /* TO DO: log the queue pause */

    /* acquire the mutex lock, since we'll be modifying the queues */
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    ol_txrx_peer_pause_base(pdev, peer);

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

void
ol_txrx_peer_tid_unpause(ol_txrx_peer_handle peer, int tid)
{
    struct ol_txrx_pdev_t *pdev = peer->vdev->pdev;

    /* TO DO: log the queue unpause */

    /* acquire the mutex lock, since we'll be modifying the queues */
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    if (tid == -1) {
        int i;
        for (i = 0; i < ARRAY_LEN(peer->txqs); i++) {
            ol_txrx_peer_tid_unpause_base(pdev, peer, i);
        }
    } else {
        ol_txrx_peer_tid_unpause_base(pdev, peer, tid);
    }

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

void
ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_txrx_peer_t *peer;

    /* TO DO: log the queue pause */

    /* acquire the mutex lock, since we'll be modifying the queues */
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
        ol_txrx_peer_pause_base(pdev, peer);
    }

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

void
ol_txrx_vdev_unpause(ol_txrx_vdev_handle vdev)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_txrx_peer_t *peer;

    /* TO DO: log the queue unpause */

    /* acquire the mutex lock, since we'll be modifying the queues */
    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    adf_os_spin_lock(&pdev->tx_queue_spinlock);

    TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
        int i;
        for (i = 0; i < ARRAY_LEN(peer->txqs); i++) {
            ol_txrx_peer_tid_unpause_base(pdev, peer, i);
        }
    }    

    adf_os_spin_unlock(&pdev->tx_queue_spinlock);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}


/*--- ADDBA triggering functions --------------------------------------------*/

#ifdef FEATURE_WLAN_INTEGRATED_SOC

void
ol_tx_queue_addba_check(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    struct ol_txrx_peer_t *peer;
    int tid;
    enum ol_addba_req_status status;

    if (!pdev->cfg.host_addba || /* host doesn't handle ADDBA negotiation */
        txq->aggr_state == ol_tx_aggr_enabled  ||  /* ADDBA already done */
        txq->aggr_state == ol_tx_aggr_disabled ||  /* ADDBA not permitted */
        txq->aggr_state == ol_tx_aggr_in_progress)
    {
        return;
    }

    /* if the tx queue is not marked as aggr_disabled, it belongs to a peer */
    TXRX_ASSERT1(tx_msdu_info->peer);
    peer = tx_msdu_info->peer;
    tid = tx_msdu_info->htt.info.ext_tid;

    /**Temporary Hack to disable Tx AMPDU to work aroung a racing condition where
    a data frame is sent before HW sends BAR after addBa exchange
    if (ETHERTYPE_IS_EAPOL_WAPI(tx_msdu_info->htt.info.ethertype)) {*/
    if (TRUE) {
        if (txq->aggr_state == ol_tx_aggr_untried) {
            /*
             * The queue is currently paused -
             * unpause it so the EAPOL can go through.
             */
            ol_txrx_peer_tid_unpause_base(pdev, peer, tid);
        }
        /*
         * Don't start aggregation based on EAPOL frame,
         * but do start for future real data frames.
         */
        txq->aggr_state = ol_tx_aggr_disabled;//ol_tx_aggr_retry;
        return;
    }
    if (txq->aggr_state == ol_tx_aggr_retry) {
        if (tx_msdu_info->peer) {
            /*
             * The queue is probably currently unpaused.
             * Pause it during the ADDBA negotiation.
             */
            ol_txrx_peer_tid_pause_base(pdev, peer, tid);
        }
    }
    status = ol_ctrl_addba_req(
        pdev->ctrl_pdev, &peer->mac_addr.raw[0], tid);
    if (status == ol_addba_req_reject) {
        /* Aggregation is disabled for this peer-TID. Unpause the tx queue. */
        txq->aggr_state = ol_tx_aggr_disabled;
        ol_txrx_peer_tid_unpause_base(pdev, peer, tid);
    } else if (status == ol_addba_req_busy) {
        /* ADDBA negotiation can't be done now, but try again next time */
        txq->aggr_state = ol_tx_aggr_retry;
        /* unpause the tx queue, so the new frame can be sent */
        ol_txrx_peer_tid_unpause_base(pdev, peer, tid);
    } else {
        /* ADDBA negotiation successfully started */
        txq->aggr_state = ol_tx_aggr_in_progress;
    }
}

void
ol_tx_addba_conf(ol_txrx_peer_handle peer, int tid)
{
    if (!peer->vdev->pdev->cfg.host_addba) {
        /*
         * In theory, this function should never be called if the
         * host_addba configuration flag is not set.
         * In practice, some test framework SW may call this function
         * even if host_addba is not set, so handle this unexpected
         * invocation gracefully.
         */
        adf_os_print(
            "UNEXPECTED CALL TO %s WHEN HOST ADDBA IS DISABLED!\n", __func__);
        return;
    }
    /* mark the aggregation as being complete */
    TXRX_ASSERT1(peer->txqs[tid].aggr_state == ol_tx_aggr_in_progress);
    /*
     * It's possible that the ADDBA negotiation failed, but regardless of
     * whether it succeeded or failed, mark the tx queue to show that
     * ADDBA negotiation has already been done, and need not be attempted
     * again.
     */
    peer->txqs[tid].aggr_state = ol_tx_aggr_enabled;
    /* unpause the tx queue */
    ol_txrx_peer_tid_unpause(peer, tid);
} 

#endif /* QCA_SUPPORT_HOST_ADDBA */

/*=== debug functions =======================================================*/

/*--- queue event log -------------------------------------------------------*/

#if defined(ENABLE_TX_QUEUE_LOG)

static void
ol_tx_queue_log_entry_type_info(
    u_int8_t *type, int *size, int *align, int var_size)
{
    switch (*type) {
    case ol_tx_log_entry_type_enqueue:
    case ol_tx_log_entry_type_dequeue:
    case ol_tx_log_entry_type_queue_free:
        *size = sizeof(struct ol_tx_log_queue_add_t);
        *align = 2;
        break;

    case ol_tx_log_entry_type_queue_state:
        *size = offsetof(struct ol_tx_log_queue_state_var_sz_t, data);
        *align = 4;
        if (var_size) {
            /* read the variable-sized record, to see how large it is */
            int align_pad;
            struct ol_tx_log_queue_state_var_sz_t *record;

            align_pad =
                (*align - ((((u_int32_t) type) + 1))) & (*align - 1);
            record = (struct ol_tx_log_queue_state_var_sz_t *)
                (type + 1 + align_pad);
            *size += record->num_cats_active *
                (sizeof(u_int32_t) /* bytes */ + sizeof(u_int16_t) /* frms */);
        }
        break;

    //case ol_tx_log_entry_type_drop:
    default:
        *size = 0;
        *align = 0;
    };
}

static void
ol_tx_queue_log_oldest_update(struct ol_txrx_pdev_t *pdev, int offset)
{
    int oldest_record_offset;

    /*
     * If the offset of the oldest record is between the current and
     * new values of the offset of the newest record, then the oldest
     * record has to be dropped from the log to provide room for the
     * newest record.
     * Advance the offset of the oldest record until it points to a
     * record that is beyond the new value of the offset of the newest
     * record.
     */
    if (!pdev->txq_log.wrapped) {
        /*
         * The log has not even filled up yet - no need to remove
         * the oldest record to make room for a new record.
         */
        return;
    }

    if (offset > pdev->txq_log.offset) {
        /*
         * not wraparound -
         * The oldest record offset may have already wrapped around,
         * even if the newest record has not.  In this case, then
         * the oldest record offset is fine where it is.
         */
        if (pdev->txq_log.oldest_record_offset == 0) {
            return;
        }
        oldest_record_offset = pdev->txq_log.oldest_record_offset;
    } else {
        /* wraparound */
        oldest_record_offset = 0;
    }

    while (oldest_record_offset < offset) {
        int size, align, align_pad;
        u_int8_t type;

        type = pdev->txq_log.data[oldest_record_offset];
        if (type == ol_tx_log_entry_type_wrap) {
            oldest_record_offset = 0;
            break;
        }
        ol_tx_queue_log_entry_type_info(
            &pdev->txq_log.data[oldest_record_offset], &size, &align, 1);
        align_pad =
            (align - ((oldest_record_offset + 1/*type*/))) & (align - 1);
        /*
        adf_os_print("TXQ LOG old alloc: offset %d, type %d, size %d (%d)\n",
            oldest_record_offset, type, size, size + 1 + align_pad);
         */
        oldest_record_offset += size + 1 + align_pad;
    }
    if (oldest_record_offset >= pdev->txq_log.size) {
        oldest_record_offset = 0;
    }
    pdev->txq_log.oldest_record_offset = oldest_record_offset;
}

void*
ol_tx_queue_log_alloc(
    struct ol_txrx_pdev_t *pdev,
    u_int8_t type /* ol_tx_log_entry_type */,
    int extra_bytes)
{
    int size, align, align_pad;
    int offset;

    ol_tx_queue_log_entry_type_info(&type, &size, &align, 0);
    size += extra_bytes;

    offset = pdev->txq_log.offset;
    align_pad = (align - ((offset + 1/*type*/))) & (align - 1);

    if (pdev->txq_log.size - offset >= size + 1 + align_pad) {
        /* no need to wrap around */
        goto alloc_found;
    }
    if (! pdev->txq_log.allow_wrap) {
        return NULL; /* log is full and can't wrap */
    }
    /* handle wrap-around */
    pdev->txq_log.wrapped = 1;
    offset = 0;
    align_pad = (align - ((offset + 1/*type*/))) & (align - 1);
    /* sanity check that the log is large enough to hold this entry */
    if (pdev->txq_log.size <= size + 1 + align_pad) {
        return NULL;
    }

alloc_found:
    ol_tx_queue_log_oldest_update(pdev, offset + size + 1 + align_pad);
    if (offset == 0)  {
        pdev->txq_log.data[pdev->txq_log.offset] = ol_tx_log_entry_type_wrap;
    }
    /*
    adf_os_print("TXQ LOG new alloc: offset %d, type %d, size %d (%d)\n",
        offset, type, size, size + 1 + align_pad);
     */
    pdev->txq_log.data[offset] = type;
    pdev->txq_log.offset = offset + size + 1 + align_pad;
    if (pdev->txq_log.offset >= pdev->txq_log.size) {
        pdev->txq_log.offset = 0;
        pdev->txq_log.wrapped = 1;
    }
    return &pdev->txq_log.data[offset + 1 + align_pad];
}

static int
ol_tx_queue_log_record_display(struct ol_txrx_pdev_t *pdev, int offset)
{
    int size, align, align_pad;
    u_int8_t type;

    type = pdev->txq_log.data[offset];
    ol_tx_queue_log_entry_type_info(
        &pdev->txq_log.data[offset], &size, &align, 1);
    align_pad = (align - ((offset + 1/*type*/))) & (align - 1);

    switch (type) {
    case ol_tx_log_entry_type_enqueue:
        {
            struct ol_tx_log_queue_add_t *record;
            record = (struct ol_tx_log_queue_add_t *)
                &pdev->txq_log.data[offset + 1 + align_pad];
            if (record->peer_id != 0xffff) {
                adf_os_print(
                    "  added %d frms (%d bytes) for peer %d, tid %d\n",
                    record->num_frms, record->num_bytes,
                    record->peer_id, record->tid);
            } else {
                adf_os_print("  added %d frms (%d bytes) vdev tid %d\n",
                    record->num_frms, record->num_bytes, record->tid);
            }
            break;
        }
    case ol_tx_log_entry_type_dequeue:
        {
            struct ol_tx_log_queue_add_t *record;
            record = (struct ol_tx_log_queue_add_t *)
                &pdev->txq_log.data[offset + 1 + align_pad];
            if (record->peer_id != 0xffff) {
                adf_os_print(
                    "  download %d frms (%d bytes) from peer %d, tid %d\n",
                    record->num_frms, record->num_bytes,
                    record->peer_id, record->tid);
            } else {
                adf_os_print("  download %d frms (%d bytes) from vdev tid %d\n",
                    record->num_frms, record->num_bytes, record->tid);
            }
            break;
        }
    case ol_tx_log_entry_type_queue_free:
        {
            struct ol_tx_log_queue_add_t *record;
            record = (struct ol_tx_log_queue_add_t *)
                &pdev->txq_log.data[offset + 1 + align_pad];
            if (record->peer_id != 0xffff) {
                adf_os_print(
                    "  peer %d, tid %d queue removed (%d frms, %d bytes)\n",
                    record->peer_id, record->tid,
                    record->num_frms, record->num_bytes);
            } else {
                /* shouldn't happen */
                adf_os_print("Unexpected vdev queue removal\n");
            }
            break;
        }

    case ol_tx_log_entry_type_queue_state:
        {
            int i, j;
            u_int32_t active_bitmap;
            struct ol_tx_log_queue_state_var_sz_t *record;
            u_int8_t *data;

            record = (struct ol_tx_log_queue_state_var_sz_t *)
                &pdev->txq_log.data[offset + 1 + align_pad];
            adf_os_print("  credit = %d, active category bitmap = %#x\n",
                record->credit, record->active_bitmap);
            data = &record->data[0];
            j = 0;
            i = 0;
            active_bitmap = record->active_bitmap;
            while (active_bitmap) {
                if (active_bitmap & 0x1) {
                    u_int16_t frms;
                    u_int32_t bytes;

                    frms = data[0] | (data[1] << 8);
                    bytes = (data[2] <<  0) | (data[3] <<  8) |
                            (data[4] << 16) | (data[5] << 24);
                    adf_os_print("    cat %d: %d frms, %d bytes\n",
                        i, frms, bytes);
                    data += 6;
                    j++;
                }
                i++;
                active_bitmap >>= 1;
            }
            break;
        }

    //case ol_tx_log_entry_type_drop:

    case ol_tx_log_entry_type_wrap:
        return -1 * offset; /* go back to the top */

    default:
        adf_os_print("  *** invalid tx log entry type (%d)\n", type);
        return 0; /* error */
    };

    return size + 1 + align_pad;
}

void
ol_tx_queue_log_display(struct ol_txrx_pdev_t *pdev)
{
    int offset;
    offset = pdev->txq_log.oldest_record_offset;
    int unwrap;

    /*
     * In theory, this should use mutex to guard against the offset
     * being changed while in use, but since this is just for debugging,
     * don't bother.
     */
    adf_os_print("tx queue log:\n");
    unwrap = pdev->txq_log.wrapped;
    while (unwrap || offset != pdev->txq_log.offset) {
        int delta = ol_tx_queue_log_record_display(pdev, offset);
        if (delta == 0) {
            return; /* error */
        }
        if (delta < 0) {
            unwrap = 0;
        }
        offset += delta;
    }
}

void
ol_tx_queue_log_enqueue(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_msdu_info_t *msdu_info,
    int frms, int bytes)
{
    int tid;
    tid = msdu_info->htt.info.ext_tid;
    u_int16_t peer_id = msdu_info->htt.info.peer_id;
    struct ol_tx_log_queue_add_t *log_elem;

    log_elem = ol_tx_queue_log_alloc(pdev, ol_tx_log_entry_type_enqueue, 0);
    if (!log_elem) {
        return;
    }

    log_elem->num_frms = frms;
    log_elem->num_bytes = bytes;
    log_elem->peer_id = peer_id;
    log_elem->tid = tid;
}

void
ol_tx_queue_log_dequeue(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    int frms, int bytes)
{
    int ext_tid;
    u_int16_t peer_id;
    struct ol_tx_log_queue_add_t *log_elem;

    ext_tid = txq->ext_tid;
    log_elem = ol_tx_queue_log_alloc(pdev, ol_tx_log_entry_type_dequeue, 0);
    if (!log_elem) {
        return;
    }

    if (ext_tid < OL_TX_NUM_TIDS) {
        struct ol_txrx_peer_t *peer;
        struct ol_tx_frms_queue_t *txq_base;

        txq_base = txq - ext_tid;
        peer = container_of(txq_base, struct ol_txrx_peer_t, txqs[0]);
        peer_id = peer->peer_ids[0];
    } else {
        peer_id = ~0;
    }

    log_elem->num_frms = frms;
    log_elem->num_bytes = bytes;
    log_elem->peer_id = peer_id;
    log_elem->tid = ext_tid;
}

void
ol_tx_queue_log_free(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_frms_queue_t *txq,
    int tid, int frms, int bytes)
{
    u_int16_t peer_id;
    struct ol_tx_log_queue_add_t *log_elem;

    log_elem = ol_tx_queue_log_alloc(pdev, ol_tx_log_entry_type_queue_free, 0);
    if (!log_elem) {
        return;
    }

    if (tid < OL_TX_NUM_TIDS) {
        struct ol_txrx_peer_t *peer;
        struct ol_tx_frms_queue_t *txq_base;

        txq_base = txq - tid;
        peer = container_of(txq_base, struct ol_txrx_peer_t, txqs[0]);
        peer_id = peer->peer_ids[0];
    } else {
        peer_id = ~0;
    }

    log_elem->num_frms = frms;
    log_elem->num_bytes = bytes;
    log_elem->peer_id = peer_id;
    log_elem->tid = tid;
}

void
ol_tx_queue_log_sched(
    struct ol_txrx_pdev_t *pdev,
    int credit,
    int *num_cats,
    u_int32_t **active_bitmap,
    u_int8_t  **data)
{
    int data_size;
    struct ol_tx_log_queue_state_var_sz_t *log_elem;

    data_size = sizeof(u_int32_t) /* bytes */ + sizeof(u_int16_t) /* frms */;
    data_size *= *num_cats;

    log_elem = ol_tx_queue_log_alloc(
        pdev, ol_tx_log_entry_type_queue_state, data_size);
    if (!log_elem) {
        *num_cats = 0;
        return;
    }
    log_elem->num_cats_active = *num_cats;
    log_elem->active_bitmap = 0;
    log_elem->credit = credit;

    *active_bitmap = &log_elem->active_bitmap;
    *data = &log_elem->data[0];
}

#endif /* defined(ENABLE_TX_QUEUE_LOG) */

/*--- queue state printouts -------------------------------------------------*/

#if TXRX_DEBUG_LEVEL > 5

void
ol_tx_queue_display(struct ol_tx_frms_queue_t *txq, int indent)
{
    char *state;

    state = (txq->flag == ol_tx_queue_active) ? "active" : "paused";
    adf_os_print("%*stxq %p (%s): %d frms, %d bytes\n",
        indent, " ", txq, state, txq->frms, txq->bytes);
}

void
ol_tx_queues_display(struct ol_txrx_pdev_t *pdev)
{
    struct ol_txrx_vdev_t *vdev;

    adf_os_print("pdev %p tx queues:\n", pdev);
    TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
        struct ol_txrx_peer_t *peer;
        int i;
        for (i = 0; i < ARRAY_LEN(vdev->txqs); i++) {
            if (vdev->txqs[i].frms == 0) {
                continue;
            }
            adf_os_print("  vdev %d (%p), txq %d\n", vdev->vdev_id, vdev, i);
            ol_tx_queue_display(&vdev->txqs[i], 4);
        }
        TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
            for (i = 0; i < ARRAY_LEN(peer->txqs); i++) {
                if (peer->txqs[i].frms == 0) {
                    continue;
                }
                adf_os_print("    peer %d (%p), txq %d\n",
                    peer->peer_ids[0], vdev, i);
                ol_tx_queue_display(&peer->txqs[i], 6);
            }
        }
    }
}

#endif

#endif /* defined(CONFIG_HL_SUPPORT) */
