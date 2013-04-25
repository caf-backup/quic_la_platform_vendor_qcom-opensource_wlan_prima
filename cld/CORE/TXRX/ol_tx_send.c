/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <adf_os_atomic.h>   /* adf_os_atomic_inc, etc. */
#include <adf_os_lock.h>     /* adf_os_spinlock */
#include <adf_nbuf.h>        /* adf_nbuf_t */

#include <queue.h>           /* TAILQ */

#include <ol_txrx_api.h>     /* ol_txrx_vdev_handle, etc. */
#include <ol_htt_tx_api.h>   /* htt_tx_compl_desc_id */
#include <ol_txrx_htt_api.h> /* htt_tx_status */

#include <ol_ctrl_txrx_api.h>
#include <ol_txrx_types.h>   /* ol_txrx_vdev_t, etc */
#include <ol_tx_desc.h>      /* ol_tx_desc_find, ol_tx_desc_frame_free */
#include <ol_txrx_internal.h>
#include <ol_osif_txrx_api.h>
#include <ol_tx.h>           /* ol_tx_reinject */

#include <ol_cfg.h>          /* ol_cfg_is_high_latency */
#include <ol_tx_sched.h>
#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#include <ol_txrx_encap.h>  /* OL_TX_RESTORE_HDR, etc*/
#endif 

#ifndef DEBUG_CREDIT
#define DEBUG_CREDIT 0
#endif
#ifdef TX_CREDIT_RECLAIM_SUPPORT

#define OL_TX_CREDIT_RECLAIM(pdev)                                  \
    do {                                                            \
        if (adf_os_atomic_read(&pdev->target_tx_credit)  <          \
                        ol_cfg_tx_credit_lwm(pdev->ctrl_pdev)) {    \
            ol_osif_ath_tasklet(pdev->osdev);                       \
        }                                                           \
    } while (0)

#else

#define OL_TX_CREDIT_RECLAIM(pdev)

#endif /* TX_CREDIT_RECLAIM_SUPPORT */

#if defined(CONFIG_HL_SUPPORT)
/*
 * HL needs to keep track of the amount of credit available to download
 * tx frames to the target - the download scheduler decides when to
 * download frames, and which frames to download, based on the credit
 * availability.
 */
#define OL_TX_TARGET_CREDIT_ADJUST(factor, pdev, msdu) \
    adf_os_atomic_add( \
        factor * htt_tx_msdu_credit(msdu), &pdev->target_tx_credit)
#define OL_TX_TARGET_CREDIT_DECR(pdev, msdu) \
    OL_TX_TARGET_CREDIT_ADJUST(-1, pdev, msdu)
#define OL_TX_TARGET_CREDIT_INCR(pdev, msdu) \
    OL_TX_TARGET_CREDIT_ADJUST(1, pdev, msdu)
#else
/*
 * LL does not need to keep track of target credit.
 * Since the host tx descriptor pool size matches the target's,
 * we know the target has space for the new tx frame if the host's
 * tx descriptor allocation succeeded.
 */
#define OL_TX_TARGET_CREDIT_DECR(pdev, msdu)  /* no-op */
#define OL_TX_TARGET_CREDIT_INCR(pdev, msdu)  /* no-op */
#endif

void
ol_tx_send(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_desc_t *tx_desc,
    adf_nbuf_t msdu)
{
    u_int16_t id;
    int failed;
#if DEBUG_CREDIT
    adf_os_print("TX %d bytes\n", adf_nbuf_len(msdu));
    adf_os_print(" <HTT> Decrease credit %d - 1 = %d, len:%d.\n",
            adf_os_atomic_read(&pdev->target_tx_credit),
            adf_os_atomic_read(&pdev->target_tx_credit) -1,
            adf_nbuf_len(msdu));
#endif
    OL_TX_TARGET_CREDIT_DECR(pdev, msdu);

    OL_TX_CREDIT_RECLAIM(pdev);

    /*
     * When the tx frame is downloaded to the target, there are two
     * outstanding references:
     * 1.  The host download SW (HTT, HTC, HIF)
     *     This reference is cleared by the ol_tx_send_done callback
     *     functions.
     * 2.  The target FW
     *     This reference is cleared by the ol_tx_completion_handler
     *     function.
     * It is extremely probable that the download completion is processed
     * before the tx completion message.  However, under exceptional
     * conditions the tx completion may be processed first.  Thus, rather
     * that assuming that reference (1) is done before reference (2),
     * explicit reference tracking is needed.
     * Double-increment the ref count to account for both references
     * described above.
     */

#ifndef ATH_11AC_TXCOMPACT
    adf_os_atomic_init(&tx_desc->ref_cnt);
    adf_os_atomic_inc(&tx_desc->ref_cnt);
    adf_os_atomic_inc(&tx_desc->ref_cnt); 
#endif

    id = ol_tx_desc_id(pdev, tx_desc);
    failed = htt_tx_send_std(pdev->htt_pdev, tx_desc->htt_tx_desc, msdu, id);
    if (adf_os_unlikely(failed)) {
        /*
         * It's inefficient to call htt_tx_msdu_credit a 2nd time here,
         * but that's okay, since this error case should never happen.
         */
        OL_TX_TARGET_CREDIT_INCR(pdev, msdu);
        ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1 /* had error */);
    }
}

static inline void
ol_tx_download_done_base(
    struct ol_txrx_pdev_t *pdev,
    A_STATUS status,
    adf_nbuf_t msdu,
    u_int16_t msdu_id)
{
    struct ol_tx_desc_t *tx_desc;

    tx_desc = ol_tx_desc_find(pdev, msdu_id);
    adf_os_assert(tx_desc);

    /*
     * If the download is done for
     * the Management frame then
     * call the download callback if registered
     */
    if (tx_desc->pkt_type >= OL_TXRX_MGMT_TYPE_BASE) {
        int tx_mgmt_index = tx_desc->pkt_type - OL_TXRX_MGMT_TYPE_BASE;
        ol_txrx_mgmt_tx_cb download_cb =
             pdev->tx_mgmt.callbacks[tx_mgmt_index].download_cb;

        if (download_cb) {
            download_cb(pdev->tx_mgmt.callbacks[tx_mgmt_index].ctxt,
                tx_desc->netbuf, status != A_OK);
        }
    }

    if (status != A_OK) {
        OL_TX_TARGET_CREDIT_INCR(pdev, msdu);
        ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1 /* download err */);
    } else {
#ifndef ATH_11AC_TXCOMPACT        
        if (adf_os_atomic_dec_and_test(&tx_desc->ref_cnt)) 
#endif            
        {
            /*
             * The decremented value was zero - free the frame.
             * Use the tx status recorded previously during
             * tx completion handling.
             */
            ol_tx_desc_frame_free_nonstd(
                pdev, tx_desc, tx_desc->status != htt_tx_status_ok);
        }
    }
}

void
ol_tx_download_done_ll(
    void *pdev,
    A_STATUS status,
    adf_nbuf_t msdu,
    u_int16_t msdu_id)
{
    ol_tx_download_done_base(
        (struct ol_txrx_pdev_t *) pdev, status, msdu, msdu_id);
}

void
ol_tx_download_done_hl_retain(
    void *txrx_pdev,
    A_STATUS status,
    adf_nbuf_t msdu,
    u_int16_t msdu_id)
{
    struct ol_txrx_pdev_t *pdev = txrx_pdev;
    ol_tx_download_done_base(pdev, status, msdu, msdu_id);
#if 0 /* TODO: Advanced feature */
    //ol_tx_dwl_sched(pdev, OL_TX_HL_SCHED_DOWNLOAD_DONE);
adf_os_assert(0);
#endif
}

void
ol_tx_download_done_hl_free(
    void *txrx_pdev,
    A_STATUS status,
    adf_nbuf_t msdu,
    u_int16_t msdu_id)
{
    struct ol_txrx_pdev_t *pdev = txrx_pdev;
    struct ol_tx_desc_t *tx_desc;

    tx_desc = ol_tx_desc_find(pdev, msdu_id);
    adf_os_assert(tx_desc);
    ol_tx_desc_frame_free_nonstd(pdev, tx_desc, status != A_OK);
#if 0 /* TODO: Advanced feature */
    //ol_tx_dwl_sched(pdev, OL_TX_HL_SCHED_DOWNLOAD_DONE);
adf_os_assert(0);
#endif
}

void
ol_tx_target_credit_update(struct ol_txrx_pdev_t *pdev, int credit_delta)
{
    adf_os_atomic_add(credit_delta, &pdev->target_tx_credit);
}

#ifndef OL_TX_RESTORE_HDR
#define OL_TX_RESTORE_HDR(__tx_desc, __msdu)
#endif
/*
 * The following macros could have been inline functions too.
 * The only rationale for choosing macros, is to force the compiler to inline
 * the implementation, which cannot be controlled for actual "inline" functions,
 * since "inline" is only a hint to the compiler.
 * In the performance path, we choose to force the inlining, in preference to
 * type-checking offered by the actual inlined functions.
 */
#define ol_tx_msdu_complete_batch(_pdev, _tx_desc, _tx_descs, _status)                          \
        do {                                                                                    \
                TAILQ_INSERT_TAIL(&(_tx_descs), (_tx_desc), tx_desc_list_elem);                 \
        } while (0)
#ifndef ATH_11AC_TXCOMPACT
#define ol_tx_msdu_complete_single(_pdev, _tx_desc, _netbuf, _lcl_freelist, _tx_desc_last)      \
        do {                                                                                    \
                adf_os_atomic_init(&(_tx_desc)->ref_cnt); /* clear the ref cnt */               \
                OL_TX_RESTORE_HDR((_tx_desc), (_netbuf)); /* restore orginal hdr offset */      \
                adf_nbuf_unmap((_pdev)->osdev, (_netbuf), ADF_OS_DMA_TO_DEVICE);                \
                adf_nbuf_free((_netbuf));                                                       \
                ((union ol_tx_desc_list_elem_t *)(_tx_desc))->next = (_lcl_freelist);           \
                if (adf_os_unlikely(!lcl_freelist)) {                                           \
                    (_tx_desc_last) = (union ol_tx_desc_list_elem_t *)(_tx_desc);               \
                }                                                                               \
                (_lcl_freelist) = (union ol_tx_desc_list_elem_t *)(_tx_desc);                   \
        } while (0)
#else  /*!ATH_11AC_TXCOMPACT*/

#define ol_tx_msdu_complete_single(_pdev, _tx_desc, _netbuf, _lcl_freelist, _tx_desc_last)      \
        do {                                                                                    \
                OL_TX_RESTORE_HDR((_tx_desc), (_netbuf)); /* restore orginal hdr offset */      \
                adf_nbuf_unmap((_pdev)->osdev, (_netbuf), ADF_OS_DMA_TO_DEVICE);                \
                adf_nbuf_free((_netbuf));                                                       \
                ((union ol_tx_desc_list_elem_t *)(_tx_desc))->next = (_lcl_freelist);           \
                if (adf_os_unlikely(!lcl_freelist)) {                                           \
                    (_tx_desc_last) = (union ol_tx_desc_list_elem_t *)(_tx_desc);               \
                }                                                                               \
                (_lcl_freelist) = (union ol_tx_desc_list_elem_t *)(_tx_desc);                   \
        } while (0)


#endif /*!ATH_11AC_TXCOMPACT*/

#ifdef QCA_TX_SINGLE_COMPLETIONS 
    #ifdef QCA_TX_STD_PATH_ONLY
        #define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs, _netbuf, _lcl_freelist,         \
                                        _tx_desc_last, _status)                                 \
            ol_tx_msdu_complete_single((_pdev), (_tx_desc), (_netbuf), (_lcl_freelist),         \
                                             _tx_desc_last)
    #else   /* !QCA_TX_STD_PATH_ONLY */
        #define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs, _netbuf, _lcl_freelist,         \
                                        _tx_desc_last, _status)                                 \
        do {                                                                                    \
            if (adf_os_likely((_tx_desc)->pkt_type == ol_tx_frm_std)) {                         \
                ol_tx_msdu_complete_single((_pdev), (_tx_desc), (_netbuf), (_lcl_freelist),     \
                                             (_tx_desc_last));                                  \
            } else {                                                                            \
                ol_tx_desc_frame_free_nonstd(                                                   \
                    (_pdev), (_tx_desc), (_status) != htt_tx_status_ok);                        \
            }                                                                                   \
        } while (0)
    #endif  /* !QCA_TX_STD_PATH_ONLY */
#else  /* !QCA_TX_SINGLE_COMPLETIONS */
    #ifdef QCA_TX_STD_PATH_ONLY
        #define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs, _netbuf, _lcl_freelist,         \
                                        _tx_desc_last, _status)                                 \
            ol_tx_msdus_complete_batch((_pdev), (_tx_desc), (_tx_descs), (_status))
    #else   /* !QCA_TX_STD_PATH_ONLY */
        #define ol_tx_msdu_complete(_pdev, _tx_desc, _tx_descs, _netbuf, _lcl_freelist,         \
                                        _tx_desc_last, _status)                                 \
        do {                                                                                    \
            if (adf_os_likely((_tx_desc)->pkt_type == ol_tx_frm_std)) {                         \
                ol_tx_msdu_complete_batch((_pdev), (_tx_desc), (_tx_descs), (_status));         \
            } else {                                                                            \
                ol_tx_desc_frame_free_nonstd(                                                   \
                    (_pdev), (_tx_desc), (_status) != htt_tx_status_ok);                        \
            }                                                                                   \
        } while (0)
    #endif  /* !QCA_TX_STD_PATH_ONLY */
#endif /* QCA_TX_SINGLE_COMPLETIONS */

/* WARNING: ol_tx_inspect_handler()'s bahavior is similar to that of ol_tx_completion_handler().
 * any change in ol_tx_completion_handler() must be mirrored in ol_tx_inspect_handler().
 */
void
ol_tx_completion_handler(
    ol_txrx_pdev_handle pdev,
    int num_msdus,
    enum htt_tx_status status,
    void *tx_desc_id_iterator)
{
    int i;
    u_int16_t *desc_ids = (u_int16_t *)tx_desc_id_iterator;
    u_int16_t tx_desc_id;
    struct ol_tx_desc_t *tx_desc;

    uint32_t   byte_cnt = 0;
    union ol_tx_desc_list_elem_t *td_array = pdev->tx_desc.array;
    adf_nbuf_t  netbuf;

    union ol_tx_desc_list_elem_t *lcl_freelist = NULL;
    union ol_tx_desc_list_elem_t *tx_desc_last = NULL;
    ol_tx_desc_list tx_descs;
    TAILQ_INIT(&tx_descs);

    for (i = 0; i < num_msdus; i++) {
        tx_desc_id = desc_ids[i];
        tx_desc = &td_array[tx_desc_id].tx_desc;
        tx_desc->status = status;
        netbuf = tx_desc->netbuf;

        /* Per SDU update of byte count */
        byte_cnt += adf_nbuf_len(netbuf);
#ifndef ATH_11AC_TXCOMPACT
        if (adf_os_atomic_dec_and_test(&tx_desc->ref_cnt)) 
#endif        
        {
			ol_tx_statistics(pdev->ctrl_pdev, 
				HTT_TX_DESC_VDEV_ID_GET(*((u_int32_t *)(tx_desc->htt_tx_desc))),
				status != htt_tx_status_ok);
            ol_tx_msdu_complete(pdev, tx_desc, tx_descs, netbuf, lcl_freelist,
                                    tx_desc_last, status);
        }
    }

    /* One shot protected access to pdev freelist, when setup */
    if (lcl_freelist) {
        adf_os_spin_lock(&pdev->tx_mutex);
        tx_desc_last->next = pdev->tx_desc.freelist;
        pdev->tx_desc.freelist = lcl_freelist; 
        adf_os_spin_unlock(&pdev->tx_mutex);
    } else {
        ol_tx_desc_frame_list_free(pdev, &tx_descs, status != htt_tx_status_ok);
    }
#if DEBUG_CREDIT
    adf_os_print(" <HTT> Increase credit %d + %d = %d\n",
            adf_os_atomic_read(&pdev->target_tx_credit),
            num_msdus,
            adf_os_atomic_read(&pdev->target_tx_credit) + num_msdus);
#endif
    if (pdev->cfg.is_high_latency) {
        /*
         * Credit was already explicitly updated by HTT,
         * but update the number of available tx descriptors,
         * then invoke the scheduler, since new credit is probably
         * available now.
         */
        adf_os_atomic_add(num_msdus, &pdev->tx_queue.rsrc_cnt);
    	ol_tx_sched(pdev);
    }
    /* Do one shot statistics */
    TXRX_STATS_UPDATE_TX_STATS(pdev, status, num_msdus, byte_cnt);
}

/* WARNING: ol_tx_inspect_handler()'s bahavior is similar to that of ol_tx_completion_handler().
 * any change in ol_tx_completion_handler() must be mirrored here.
 */
void
ol_tx_inspect_handler(
    ol_txrx_pdev_handle pdev,
    int num_msdus,
    void *tx_desc_id_iterator)
{
    u_int16_t vdev_id, i;
    struct ol_txrx_vdev_t *vdev;
    u_int16_t *desc_ids = (u_int16_t *)tx_desc_id_iterator;
    u_int16_t tx_desc_id;
    struct ol_tx_desc_t *tx_desc;
    union ol_tx_desc_list_elem_t *td_array = pdev->tx_desc.array;
    union ol_tx_desc_list_elem_t *lcl_freelist = NULL;
    union ol_tx_desc_list_elem_t *tx_desc_last = NULL;
    adf_nbuf_t  netbuf;
    ol_tx_desc_list tx_descs;
    TAILQ_INIT(&tx_descs);

    for (i = 0; i < num_msdus; i++) {
        tx_desc_id = desc_ids[i];
        tx_desc = &td_array[tx_desc_id].tx_desc;
        netbuf = tx_desc->netbuf;

        /* find the "vdev" this tx_desc belongs to */
        vdev_id = HTT_TX_DESC_VDEV_ID_GET(*((u_int32_t *)(tx_desc->htt_tx_desc)));
        TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
            if (vdev->vdev_id == vdev_id)
                break;
        }

        /* vdev now points to the vdev for this descriptor. */

#ifndef ATH_11AC_TXCOMPACT        
        /* save this multicast packet to local free list */
        if (adf_os_atomic_dec_and_test(&tx_desc->ref_cnt)) 
#endif            
        {
            /* for this function only, force htt status to be "htt_tx_status_ok" 
             * for graceful freeing of this multicast frame
             */
            ol_tx_msdu_complete(pdev, tx_desc, tx_descs, netbuf, lcl_freelist,
                                    tx_desc_last, htt_tx_status_ok);
        }
    }

    if (lcl_freelist) {
        adf_os_spin_lock(&pdev->tx_mutex);
        tx_desc_last->next = pdev->tx_desc.freelist;
        pdev->tx_desc.freelist = lcl_freelist; 
        adf_os_spin_unlock(&pdev->tx_mutex);
    } else {
        ol_tx_desc_frame_list_free(pdev, &tx_descs, htt_tx_status_discard);
    }
#if DEBUG_CREDIT
    adf_os_print(" <HTT> Increase HTT credit %d + %d = %d..\n",
            adf_os_atomic_read(&pdev->target_tx_credit),
            num_msdus,
            adf_os_atomic_read(&pdev->target_tx_credit) + num_msdus);
#endif

    if (pdev->cfg.is_high_latency) {
        /* credit was already explicitly updated by HTT */
    	ol_tx_sched(pdev);
    }
}

