/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <adf_nbuf.h>         /* adf_nbuf_t, etc. */
#include <adf_os_atomic.h>    /* adf_os_atomic_read, etc. */
#include <adf_os_util.h>      /* adf_os_unlikely */

#include <htt.h>              /* HTT_TX_EXT_TID_MGMT */
#include <ol_htt_tx_api.h>    /* htt_tx_desc_tid */
#include <ol_txrx_api.h>      /* ol_txrx_vdev_handle */
#include <ol_txrx_ctrl_api.h> /* ol_txrx_sync */

#include <ol_txrx_internal.h> /* TXRX_ASSERT1 */
#include <ol_txrx_types.h>    /* pdev stats */
#include <ol_tx_desc.h>       /* ol_tx_desc */
#include <ol_tx_send.h>       /* ol_tx_send */

#include <ol_tx_classify.h>   /* ol_tx_classify, ol_tx_classify_mgmt */
#include <ol_tx_queue.h>      /* ol_tx_enqueue */
#include <ol_tx_sched.h>      /* ol_tx_sched */
#include <ol_txrx.h>

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#include <ol_txrx_encap.h>    /* OL_TX_ENCAP, etc*/
#endif 

#define ol_tx_prepare_ll(tx_desc, vdev, msdu, msdu_info) \
    do {                                                                      \
        /* 
         * The TXRX module doesn't accept tx frames unless the target has 
         * enough descriptors for them.
         * For LL, the TXRX descriptor pool is sized to match the target's
         * descriptor pool.  Hence, if the descriptor allocation in TXRX
         * succeeds, that guarantees that the target has room to accept
         * the new tx frame.
         */                                                                   \
        tx_desc = ol_tx_desc_ll(vdev->pdev, vdev, msdu, msdu_info);           \
        if (adf_os_unlikely(! tx_desc)) {                                     \
            TXRX_STATS_MSDU_LIST_INCR(                                        \
                vdev->pdev, tx.dropped.host_reject, msdu);                    \
            return msdu; /* the list of unaccepted MSDUs */                   \
        }                                                                     \
    } while (0)

adf_nbuf_t
ol_tx_ll(ol_txrx_vdev_handle vdev, adf_nbuf_t msdu_list)
{
    adf_nbuf_t msdu = msdu_list;
    /*
     * The msdu_list variable could be used instead of the msdu var, 
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */
    while (msdu) {
        adf_nbuf_t next;
        struct ol_tx_desc_t *tx_desc;
        struct ol_txrx_msdu_info_t msdu_info;
        msdu_info.peer = NULL;

        msdu_info.htt.info.ext_tid = adf_nbuf_get_tid(msdu);
        ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);
        /*
         * If debug display is enabled, show the meta-data being
         * downloaded to the target via the HTT tx descriptor.
         */
        htt_tx_desc_display(tx_desc->htt_tx_desc);
        /*
         * The netbuf may get linked into a different list inside the
         * ol_tx_send function, so store the next pointer before the
         * tx_send call.
         */
        next = adf_nbuf_next(msdu);
        ol_tx_send(vdev->pdev, tx_desc, msdu);
        msdu = next;
    }
    return NULL; /* all MSDUs were accepted */
}

static inline int
OL_TXRX_TX_IS_RAW(enum ol_txrx_osif_tx_spec tx_spec)
{
    return
        tx_spec &
        (ol_txrx_osif_tx_spec_raw |
         ol_txrx_osif_tx_spec_no_aggr |
         ol_txrx_osif_tx_spec_no_encrypt);
}

static inline u_int8_t
OL_TXRX_TX_RAW_SUBTYPE(enum ol_txrx_osif_tx_spec tx_spec)
{
    u_int8_t sub_type = 0x1; /* 802.11 MAC header present */

    if (tx_spec & ol_txrx_osif_tx_spec_no_aggr) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_AGGR_S;
    }
    if (tx_spec & ol_txrx_osif_tx_spec_no_encrypt) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
    }
    if (tx_spec & ol_txrx_osif_tx_spect_nwifi_no_encrypt) {
        sub_type |= 0x1 << HTT_TX_MSDU_DESC_RAW_SUBTYPE_NO_ENCRYPT_S;
    }
    return sub_type;
}

#ifdef QCA_HOST_SIDE_TX_TID_CLASSIFICATION
adf_nbuf_t
ol_tx_single_non_std_ll(
    ol_txrx_vdev_handle vdev,
    u_int8_t ext_tid,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu)
{
    htt_pdev_handle htt_pdev = vdev->pdev->htt_pdev;
    struct ol_txrx_msdu_info_t msdu_info;
    msdu_info.peer = NULL;
    struct ol_tx_desc_t *tx_desc;
    ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);

    if (tx_spec != ol_txrx_osif_tx_spec_std) {
        if (tx_spec & ol_txrx_osif_tx_spec_tso) {
            tx_desc->pkt_type = ol_tx_frm_tso;
        } else if (tx_spec & ol_txrx_osif_tx_spect_nwifi_no_encrypt) {
            u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
            htt_tx_desc_type(
                htt_pdev, tx_desc->htt_tx_desc,
                htt_pkt_type_native_wifi, sub_type);
        } else if (OL_TXRX_TX_IS_RAW(tx_spec)) {
            /* different types of raw frames */
            u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
            htt_tx_desc_type(
                htt_pdev, tx_desc->htt_tx_desc,
                htt_pkt_type_raw, sub_type);
        }
    }
    /* explicitly specify the TID and the limit 
     * it to the 0-15 value of the QoS TID.
     */

    if (ext_tid >= HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST) {
        ext_tid = HTT_TX_EXT_TID_DEFAULT;           
    }
    htt_tx_desc_tid(htt_pdev, tx_desc->htt_tx_desc, ext_tid);

    /*
    * If debug display is enabled, show the meta-data being
    * downloaded to the target via the HTT tx descriptor.
    */
    htt_tx_desc_display(tx_desc->htt_tx_desc);
    ol_tx_send(vdev->pdev, tx_desc, msdu);
    return NULL; /* all MSDUs were accepted */
}
#else /* QCA_HOST_SIDE_TX_TID_CLASSIFICATION */

adf_nbuf_t
ol_tx_non_std_ll(
    ol_txrx_vdev_handle vdev,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu_list)
{
    adf_nbuf_t msdu = msdu_list;
    htt_pdev_handle htt_pdev = vdev->pdev->htt_pdev;
    struct ol_txrx_msdu_info_t msdu_info;
    msdu_info.peer = NULL;

    /*
     * The msdu_list variable could be used instead of the msdu var, 
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */
    while (msdu) {
        adf_nbuf_t next;
        struct ol_tx_desc_t *tx_desc;

        msdu_info.htt.info.ext_tid = adf_nbuf_get_tid(msdu);
        ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);

        /*
         * The netbuf may get linked into a different list inside the
         * ol_tx_send function, so store the next pointer before the
         * tx_send call.
         */
        next = adf_nbuf_next(msdu);

        if (tx_spec != ol_txrx_osif_tx_spec_std) {
            if (tx_spec & ol_txrx_osif_tx_spec_tso) {
                tx_desc->pkt_type = ol_tx_frm_tso;
            } else if (tx_spec & ol_txrx_osif_tx_spect_nwifi_no_encrypt) {
                u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
                htt_tx_desc_type(
                    htt_pdev, tx_desc->htt_tx_desc,
                    htt_pkt_type_native_wifi, sub_type);
            } else if (OL_TXRX_TX_IS_RAW(tx_spec)) {
                /* different types of raw frames */
                u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
                htt_tx_desc_type(
                    htt_pdev, tx_desc->htt_tx_desc,
                    htt_pkt_type_raw, sub_type);
            }
        }
        /*
         * If debug display is enabled, show the meta-data being
         * downloaded to the target via the HTT tx descriptor.
         */
        htt_tx_desc_display(tx_desc->htt_tx_desc);
        ol_tx_send(vdev->pdev, tx_desc, msdu);
        msdu = next;
    }
    return NULL; /* all MSDUs were accepted */
}
#endif

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
#define OL_TX_ENCAP_WRAPPER(pdev, vdev, tx_desc, msdu, tx_msdu_info) \
    do { \
        if (OL_TX_ENCAP(vdev, tx_desc, msdu, &tx_msdu_info) != A_OK) { \
            ol_tx_desc_frame_free_nonstd(pdev, tx_desc, 1); \
            if (tx_msdu_info.peer) { \
                /* remove the peer reference added above */ \
                ol_txrx_peer_unref_delete(tx_msdu_info.peer); \
            } \
            goto MSDU_LOOP_BOTTOM; \
        } \
    } while (0)
#else
#define OL_TX_ENCAP_WRAPPER(pdev, vdev, tx_desc, msdu, tx_msdu_info) /* no-op */
#endif

static inline adf_nbuf_t
ol_tx_hl_base(
    ol_txrx_vdev_handle vdev,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu_list)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    adf_nbuf_t msdu = msdu_list;
    struct ol_txrx_msdu_info_t tx_msdu_info;
    htt_pdev_handle htt_pdev = pdev->htt_pdev;
    tx_msdu_info.peer = NULL;

    /*
     * The msdu_list variable could be used instead of the msdu var,
     * but just to clarify which operations are done on a single MSDU
     * vs. a list of MSDUs, use a distinct variable for single MSDUs
     * within the list.
     */
    while (msdu) {
        adf_nbuf_t next;
        struct ol_tx_frms_queue_t *txq;
        struct ol_tx_desc_t *tx_desc;

        /*
         * The netbuf will get stored into a (peer-TID) tx queue list
         * inside the ol_tx_classify_store function or else dropped,
         * so store the next pointer immediately.
         */
        next = adf_nbuf_next(msdu);

        tx_desc = ol_tx_desc_hl(pdev, vdev, msdu, &tx_msdu_info);
        if (! tx_desc) {
            /*
             * If we're out of tx descs, there's no need to try to allocate
             * tx descs for the remaining MSDUs.
             */
            TXRX_STATS_MSDU_LIST_INCR(pdev, tx.dropped.host_reject, msdu);
            return msdu; /* the list of unaccepted MSDUs */
        }
        adf_os_atomic_dec(&pdev->tx_queue.rsrc_cnt);
//        OL_TXRX_PROT_AN_LOG(pdev->prot_an_tx_sent, msdu);

        if (tx_spec != ol_txrx_osif_tx_spec_std) {
            if (tx_spec & ol_txrx_osif_tx_spec_tso) {
                tx_desc->pkt_type = ol_tx_frm_tso;
            }
            if (OL_TXRX_TX_IS_RAW(tx_spec)) {
                // CHECK THIS: does this need to happen after htt_tx_desc_init?
                /* different types of raw frames */
                u_int8_t sub_type = OL_TXRX_TX_RAW_SUBTYPE(tx_spec);
                htt_tx_desc_type(
                    htt_pdev, tx_desc->htt_tx_desc,
                    htt_pkt_type_raw, sub_type);
            }
        }

        tx_msdu_info.htt.info.ext_tid = adf_nbuf_get_tid(msdu);
        tx_msdu_info.htt.info.vdev_id = vdev->vdev_id;
        tx_msdu_info.htt.info.frame_type = htt_frm_type_data;
        tx_msdu_info.htt.info.l2_hdr_type = pdev->htt_pkt_type;

        txq = ol_tx_classify(vdev, tx_desc, msdu, &tx_msdu_info);
        if (!txq) {
            adf_os_atomic_inc(&pdev->tx_queue.rsrc_cnt);
            //TXRX_STATS_MSDU_LIST_INCR(pdev, tx.dropped.no_txq, msdu);
            ol_tx_desc_free(pdev, tx_desc);
            if (tx_msdu_info.peer) {
                /* remove the peer reference added above */
                ol_txrx_peer_unref_delete(tx_msdu_info.peer);
            }
            return msdu; /* the list of unaccepted MSDUs */
        }

        //INSERT TX FILTER HERE:
        if (0 /*pdev->tx_filter*/) {
            int keep = 1; //ol_tx_filter(tx_desc, netbuf, &txrx_msdu_info);
            if (!keep) {
                adf_nbuf_unmap(
                    pdev->osdev, tx_desc->netbuf, ADF_OS_DMA_TO_DEVICE);
                adf_nbuf_set_next(tx_desc->netbuf, NULL);
                adf_nbuf_tx_free(tx_desc->netbuf, 1);
                goto MSDU_LOOP_BOTTOM;
            }
        }

        /*
         * Initialize the HTT tx desc l2 header offset field.
         * htt_tx_desc_mpdu_header  needs to be called to make sure,
         * the l2 header size is initialized correctly to handle cases
         * where TX ENCAP is disabled or Tx Encap fails to perform Encap
         */
        htt_tx_desc_mpdu_header(tx_desc->htt_tx_desc, 0);

        /*
         * Note: when the driver is built without support for SW tx encap,
         * the following macro is a no-op.   When the driver is built with
         * support for SW tx encap, it performs encap, and if an error is
         * encountered, jumps to the MSDU_LOOP_BOTTOM label.
         */
        OL_TX_ENCAP_WRAPPER(pdev, vdev, tx_desc, msdu, tx_msdu_info);

        /* initialize the HW tx descriptor */
        htt_tx_desc_init(
            pdev->htt_pdev, tx_desc->htt_tx_desc,
            ol_tx_desc_id(pdev, tx_desc),
            msdu,
            &tx_msdu_info.htt);
        /*
         * If debug display is enabled, show the meta-data being
         * downloaded to the target via the HTT tx descriptor.
         */
        htt_tx_desc_display(tx_desc->htt_tx_desc);

        ol_tx_enqueue(pdev, txq, tx_desc, &tx_msdu_info);
        if (tx_msdu_info.peer) {
            /* remove the peer reference added above */
            ol_txrx_peer_unref_delete(tx_msdu_info.peer);
        }

MSDU_LOOP_BOTTOM:
        msdu = next;
    }
    ol_tx_sched(pdev);

    return NULL; /* all MSDUs were accepted */
}

adf_nbuf_t
ol_tx_hl(ol_txrx_vdev_handle vdev, adf_nbuf_t msdu_list)
{
    return ol_tx_hl_base(vdev, ol_txrx_osif_tx_spec_std, msdu_list);
}

adf_nbuf_t
ol_tx_non_std_hl(
    ol_txrx_vdev_handle vdev,
    enum ol_txrx_osif_tx_spec tx_spec,
    adf_nbuf_t msdu_list)
{
    return ol_tx_hl_base(vdev, tx_spec, msdu_list);
}

void
ol_txrx_mgmt_tx_cb_set(
    ol_txrx_pdev_handle pdev,
    u_int8_t type,
    ol_txrx_mgmt_tx_cb cb,
    void *ctxt)
{
    TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
    pdev->tx_mgmt.callbacks[type].cb = cb;
    pdev->tx_mgmt.callbacks[type].ctxt = ctxt;
}

#ifdef FEATURE_WLAN_INTEGRATED_SOC
int
ol_txrx_mgmt_send(
    ol_txrx_vdev_handle vdev,
    adf_nbuf_t tx_mgmt_frm,
    u_int8_t type,
    u_int8_t use_6mbps)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_tx_desc_t *tx_desc;
    struct ol_txrx_msdu_info_t tx_msdu_info;

    tx_msdu_info.htt.action.use_6mbps = use_6mbps;
    tx_msdu_info.htt.info.ext_tid = HTT_TX_EXT_TID_MGMT;
    tx_msdu_info.peer = NULL;

    if (pdev->cfg.is_high_latency) {
        tx_desc = ol_tx_desc_hl(pdev, vdev, tx_mgmt_frm, &tx_msdu_info);
    } else {
        tx_desc = ol_tx_desc_ll(pdev, vdev, tx_mgmt_frm, &tx_msdu_info);
    }
    if (! tx_desc) {
        return 1; /* can't accept the tx mgmt frame */
    }
    TXRX_STATS_MSDU_INCR(vdev->pdev, tx.mgmt, tx_mgmt_frm);
    adf_nbuf_map_single(pdev->osdev, tx_mgmt_frm, ADF_OS_DMA_TO_DEVICE);

    TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
    tx_desc->pkt_type = type + OL_TXRX_MGMT_TYPE_BASE;
    
    if (pdev->cfg.is_high_latency) {
		struct ol_tx_frms_queue_t *txq;
        /*
         * 1.  Look up the peer and queue the frame in the peer's mgmt queue.
         * 2.  Invoke the download scheduler.
         */
		tx_msdu_info.htt.info.vdev_id = vdev->vdev_id;
		tx_msdu_info.htt.info.frame_type = htt_frm_type_mgmt;
		tx_msdu_info.htt.info.l2_hdr_type = htt_pkt_type_native_wifi;
		tx_msdu_info.htt.action.do_tx_complete =
            pdev->tx_mgmt.callbacks[type].cb ? 1 : 0;

        txq = ol_tx_classify_mgmt(vdev, tx_desc, tx_mgmt_frm, &tx_msdu_info);
        if (!txq) {
            //TXRX_STATS_MSDU_LIST_INCR(vdev->pdev, tx.dropped.no_txq, msdu);
            ol_tx_desc_free(vdev->pdev, tx_desc);
            if (tx_msdu_info.peer) {
                /* remove the peer reference added above */
                ol_txrx_peer_unref_delete(tx_msdu_info.peer);
            }
            return 1; /* can't accept the tx mgmt frame */
        }
        /*
        * Initialize the HTT tx desc l2 header offset field.
        * Even though tx encap does not apply to mgmt frames,
        * htt_tx_desc_mpdu_header still needs to be called,
        * to specifiy that there was no L2 header added by tx encap,
        * so the frame's length does not need to be adjusted to account for
        * an added L2 header.
        */
        htt_tx_desc_mpdu_header(tx_desc->htt_tx_desc, 0);
        htt_tx_desc_init(
            pdev->htt_pdev, tx_desc->htt_tx_desc,
            ol_tx_desc_id(pdev, tx_desc),
            tx_mgmt_frm,
            &tx_msdu_info.htt);
        htt_tx_desc_display(tx_desc->htt_tx_desc);

	    ol_tx_enqueue(vdev->pdev, txq, tx_desc, &tx_msdu_info);
	    if (tx_msdu_info.peer) {
	        /* remove the peer reference added above */
	        ol_txrx_peer_unref_delete(tx_msdu_info.peer);
	    }
        ol_tx_sched(vdev->pdev);
    } else {
        ol_tx_send(pdev, tx_desc, tx_mgmt_frm);
    }

    return 0; /* accepted the tx mgmt frame */
}
#else
int
ol_txrx_mgmt_send(
    ol_txrx_vdev_handle vdev,
    adf_nbuf_t tx_mgmt_frm,
    u_int8_t type,
    u_int8_t use_6mbps)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    A_UINT8  frm_hdr[HTT_MGMT_FRM_HDR_DOWNLOAD_LEN];
    A_UINT8 *frm_buf;

    frm_buf = (A_UINT8 *)adf_nbuf_data(tx_mgmt_frm);
    adf_os_mem_copy(frm_hdr, frm_buf, HTT_MGMT_FRM_HDR_DOWNLOAD_LEN);

#if 0
    struct ol_tx_desc_t *tx_desc;

    if (pdev->cfg.is_high_latency) {
        tx_desc = ol_tx_desc_hl(pdev, vdev, tx_mgmt_frm);
    } else {
        tx_desc = ol_tx_desc_ll(pdev, vdev, tx_mgmt_frm);
    }
    if (! tx_desc) {
        return 1; /* can't accept the tx mgmt frame */
    }
    TXRX_STATS_MSDU_INCR(vdev->pdev, tx.mgmt, tx_mgmt_frm);
    adf_nbuf_map_single(pdev->osdev, tx_mgmt_frm, ADF_OS_DMA_TO_DEVICE);

    TXRX_ASSERT1(type < OL_TXRX_MGMT_NUM_TYPES);
    tx_desc->pkt_type = type + OL_TXRX_MGMT_TYPE_BASE;
    htt_tx_desc_tid(pdev->htt_pdev, tx_desc->htt_tx_desc, HTT_TX_EXT_TID_MGMT);
    
    if (pdev->cfg.is_high_latency) {
#if ENABLE_HOST_TX_SCHED
		struct ol_tx_frms_queue_t *txq;
        struct ol_txrx_msdu_info_t tx_msdu_info = {{{0}, {0}}, 0};
/*
 * 1.  Look up the peer and queue the frame in the peer's mgmt queue.
 * 2.  Invoke the download scheduler.
 */
		tx_msdu_info.htt.info.vdev_id = vdev->vdev_id;
		tx_msdu_info.htt.info.frame_type = htt_frm_type_mgmt;
		tx_msdu_info.htt.info.l2_hdr_type = htt_pkt_type_native_wifi;

        txq = ol_tx_classify_mgmt(vdev, tx_desc, tx_mgmt_frm, &tx_msdu_info);
        
	    ol_tx_enqueue(vdev->pdev, txq, tx_desc, &tx_msdu_info);
	    if (tx_msdu_info.peer) {
	        /* remove the peer reference added above */
	        ol_txrx_peer_unref_delete(tx_msdu_info.peer);
	    }
        ol_tx_dl_sched(vdev->pdev);
#endif
    } else {
        ol_tx_send(pdev, tx_desc, tx_mgmt_frm);
    }
#endif

    adf_nbuf_map_single(pdev->osdev, tx_mgmt_frm, ADF_OS_DMA_TO_DEVICE);

    htt_h2t_mgmt_tx(pdev->htt_pdev,
                    adf_nbuf_get_frag_paddr_lo(tx_mgmt_frm, 0),
                    tx_mgmt_frm,
                    adf_nbuf_len(tx_mgmt_frm),
                    vdev->vdev_id,
                    frm_hdr);

    return 0; /* accepted the tx mgmt frame */
}
#endif

void
ol_txrx_sync(ol_txrx_pdev_handle pdev, u_int8_t sync_cnt)
{
    htt_h2t_sync_msg(pdev->htt_pdev, sync_cnt);
}

adf_nbuf_t ol_tx_reinject(
    struct ol_txrx_vdev_t *vdev,
    adf_nbuf_t msdu, uint32_t peer_id)
{
    struct ol_tx_desc_t *tx_desc;
    struct ol_txrx_msdu_info_t msdu_info;
    msdu_info.peer = NULL;

    msdu_info.htt.info.ext_tid = HTT_TX_EXT_TID_INVALID;
    ol_tx_prepare_ll(tx_desc, vdev, msdu, &msdu_info);
    HTT_TX_DESC_POSTPONED_SET(*((u_int32_t *)(tx_desc->htt_tx_desc)), TRUE);

    htt_tx_desc_set_peer_id((u_int32_t *)(tx_desc->htt_tx_desc), peer_id);

    ol_tx_send(vdev->pdev, tx_desc, msdu);

    return NULL;
}
