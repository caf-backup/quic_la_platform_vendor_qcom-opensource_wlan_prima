/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef _OL_TXRX_INTERNAL__H_
#define _OL_TXRX_INTERNAL__H_

#include <adf_os_util.h>   /* adf_os_assert */
#include <adf_nbuf.h>      /* adf_nbuf_t */
#include <adf_os_mem.h>    /* adf_os_mem_set */
#include <ieee80211_common.h>     /* ieee80211_frame */
#include <ol_htt_rx_api.h> /* htt_rx_msdu_desc_completes_mpdu, etc. */

#include <ol_txrx_types.h>

#include <ol_txrx_dbg.h>


#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x)/sizeof(x[0]))
#endif


#ifndef TXRX_ASSERT_LEVEL
#define TXRX_ASSERT_LEVEL 3
#endif

#ifdef __KLOCWORK__
# define TXRX_ASSERT1(x) do { if (!(x)) abort(); } while (0)
# define TXRX_ASSERT2(x) do { if (!(x)) abort(); } while (0)
#else // #ifdef __KLOCWORK__

#if TXRX_ASSERT_LEVEL > 0
#define TXRX_ASSERT1(condition) adf_os_assert((condition))
#else
#define TXRX_ASSERT1(condition)
#endif

#if TXRX_ASSERT_LEVEL > 1
#define TXRX_ASSERT2(condition) adf_os_assert((condition))
#else
#define TXRX_ASSERT2(condition)
#endif
#endif // #ifdef __KLOCWORK__
enum {
    /* FATAL_ERR - print only irrecoverable error messages */
    TXRX_PRINT_LEVEL_FATAL_ERR,

    /* ERR - include non-fatal err messages */
    TXRX_PRINT_LEVEL_ERR,

    /* WARN - include warnings */
    TXRX_PRINT_LEVEL_WARN,

    /* INFO1 - include fundamental, infrequent events */
    TXRX_PRINT_LEVEL_INFO1,

    /* INFO2 - include non-fundamental but infrequent events */
    TXRX_PRINT_LEVEL_INFO2,

    /* INFO3 - include frequent events */
    /* to avoid performance impact, don't use INFO3 unless explicitly enabled */
    #ifdef TXRX_PRINT_VERBOSE_ENABLE
    TXRX_PRINT_LEVEL_INFO3,
    #endif /* TXRX_PRINT_VERBOSE_ENABLE */
};

extern unsigned g_txrx_print_level;

#ifdef TXRX_PRINT_ENABLE

#include <stdarg.h>       /* va_list */
#include <adf_os_types.h> /* adf_os_vprint */

#define ol_txrx_print(level, fmt, ...) \
    if (level <= g_txrx_print_level) adf_os_print(fmt, ## __VA_ARGS__)
#define TXRX_PRINT(level, fmt, ...) \
    ol_txrx_print(level, "TXRX: " fmt, ## __VA_ARGS__)

#ifdef TXRX_PRINT_VERBOSE_ENABLE

#define ol_txrx_print_verbose(fmt, ...) \
    if (TXRX_PRINT_LEVEL_INFO3 <= g_txrx_print_level) \
        adf_os_print(fmt, ## __VA_ARGS__)
#define TXRX_PRINT_VERBOSE(fmt, ...) \
    ol_txrx_print_verbose("TXRX: " fmt, ## __VA_ARGS__)
#else
#define TXRX_PRINT_VERBOSE(fmt, ...)
#endif /* TXRX_PRINT_VERBOSE_ENABLE */

#else
#define TXRX_PRINT(level, fmt, ...)
#define TXRX_PRINT_VERBOSE(fmt, ...)
#endif /* TXRX_PRINT_ENABLE */

#ifdef HOST_TX_SCHED_DEBUG
#define TX_SCHED_DEBUG_PRINT(fmt, ...) adf_os_print(fmt, ## __VA_ARGS__)
#else
#define TX_SCHED_DEBUG_PRINT(fmt, ...)
#endif

#define OL_TXRX_LIST_APPEND(head, tail, elem) \
do {                                            \
    if (!(head)) {                              \
        (head) = (elem);                        \
    } else {                                    \
        adf_nbuf_set_next((tail), (elem));      \
    }                                           \
    (tail) = (elem);                            \
} while (0)

static inline void
ol_rx_mpdu_list_next(
    struct ol_txrx_pdev_t *pdev,
    void *mpdu_list,
    adf_nbuf_t *mpdu_tail,
    adf_nbuf_t *next_mpdu)
{
    htt_pdev_handle htt_pdev = pdev->htt_pdev;
    adf_nbuf_t msdu;

    /*
     * For now, we use a simply flat list of MSDUs.
     * So, traverse the list until we reach the last MSDU within the MPDU.
     */
    TXRX_ASSERT2(mpdu_list);
    msdu = mpdu_list;
    while (!htt_rx_msdu_desc_completes_mpdu(
                htt_pdev, htt_rx_msdu_desc_retrieve(htt_pdev, msdu)))
    {
        msdu = adf_nbuf_next(msdu);
        TXRX_ASSERT2(msdu);
    }
    /* msdu now points to the last MSDU within the first MPDU */
    *mpdu_tail = msdu;
    *next_mpdu = adf_nbuf_next(msdu);
}


/*--- txrx stats macros ---*/


/* unconditional defs */
#define TXRX_STATS_INCR(pdev, field) TXRX_STATS_ADD(pdev, field, 1)

/* default conditional defs (may be undefed below) */

#define TXRX_STATS_INIT(_pdev) \
    adf_os_mem_set(&((_pdev)->stats), 0x0, sizeof((_pdev)->stats))
#define TXRX_STATS_ADD(_pdev, _field, _delta) \
    _pdev->stats._field += _delta
#define TXRX_STATS_MSDU_INCR(pdev, field, netbuf) \
    do { \
        TXRX_STATS_INCR((pdev), pub.field.pkts); \
        TXRX_STATS_ADD((pdev), pub.field.bytes, adf_nbuf_len(netbuf)); \
    } while (0)

/* conditional defs based on verbosity level */

#if /*---*/ TXRX_STATS_LEVEL == TXRX_STATS_LEVEL_FULL

#define TXRX_STATS_MSDU_LIST_INCR(pdev, field, netbuf_list) \
    do { \
        adf_nbuf_t tmp_list = netbuf_list; \
        while (tmp_list) { \
            TXRX_STATS_MSDU_INCR(pdev, field, tmp_list); \
            tmp_list = adf_nbuf_next(tmp_list); \
        } \
    } while (0)

#define TXRX_STATS_MSDU_INCR_TX_STATUS(status, pdev, netbuf) \
    switch (status) { \
    case htt_tx_status_ok: \
        TXRX_STATS_MSDU_INCR(pdev, tx.delivered, netbuf); \
        break; \
    case htt_tx_status_discard: \
        TXRX_STATS_MSDU_INCR(pdev, tx.dropped.target_discard, netbuf); \
        break; \
    case htt_tx_status_no_ack: \
        TXRX_STATS_MSDU_INCR(pdev, tx.dropped.no_ack, netbuf); \
        break; \
    case htt_tx_status_download_fail: \
        TXRX_STATS_MSDU_INCR(pdev, tx.dropped.download_fail, netbuf); \
        break; \
    default: \
        break; \
    }

#define TXRX_STATS_UPDATE_TX_STATS(_pdev, _status, _p_cntrs, _b_cntrs)          \
do {                                                                            \
    switch (status) {                                                           \
    case htt_tx_status_ok:                                                      \
       TXRX_STATS_ADD(_pdev, pub.tx.delivered.pkts, _p_cntrs);                  \
       TXRX_STATS_ADD(_pdev, pub.tx.delivered.bytes, _b_cntrs);                 \
        break;                                                                  \
    case htt_tx_status_discard:                                                 \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.target_discard.pkts, _p_cntrs);     \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.target_discard.bytes, _b_cntrs);    \
        break;                                                                  \
    case htt_tx_status_no_ack:                                                  \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.no_ack.pkts, _p_cntrs);             \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.no_ack.bytes, _b_cntrs);            \
        break;                                                                  \
    case htt_tx_status_download_fail:                                           \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.download_fail.pkts, _p_cntrs);      \
       TXRX_STATS_ADD(_pdev, pub.tx.dropped.download_fail.bytes, _b_cntrs);     \
        break;                                                                  \
    default:                                                                    \
        break;                                                                  \
    }                                                                           \
} while (0)

#elif /*---*/ TXRX_STATS_LEVEL == TXRX_STATS_LEVEL_BASIC

#define TXRX_STATS_MSDU_LIST_INCR(pdev, field, netbuf_list)

#define TXRX_STATS_MSDU_INCR_TX_STATUS(status, pdev, netbuf) \
    do { \
        if (status == htt_tx_status_ok) { \
            TXRX_STATS_MSDU_INCR(pdev, tx.delivered, netbuf); \
        } \
    } while (0)

#define TXRX_STATS_INIT(_pdev) \
    adf_os_mem_set(&((_pdev)->stats), 0x0, sizeof((_pdev)->stats))

#define TXRX_STATS_UPDATE_TX_STATS(_pdev, _status, _p_cntrs, _b_cntrs)          \
do {                                                                            \
    if (adf_os_likely(_status == htt_tx_status_ok)) {                           \
       TXRX_STATS_ADD(_pdev, pub.tx.delivered.pkts, _p_cntrs);                  \
       TXRX_STATS_ADD(_pdev, pub.tx.delivered.bytes, _b_cntrs);                 \
    }                                                                           \
} while (0)

#else /*---*/ /* stats off */

#undef  TXRX_STATS_INIT
#define TXRX_STATS_INIT(_pdev)

#undef  TXRX_STATS_ADD
#define TXRX_STATS_ADD(_pdev, _field, _delta)

#undef  TXRX_STATS_MSDU_INCR
#define TXRX_STATS_MSDU_INCR(pdev, field, netbuf)

#define TXRX_STATS_MSDU_LIST_INCR(pdev, field, netbuf_list)

#define TXRX_STATS_MSDU_INCR_TX_STATUS(status, pdev, netbuf)

#define TXRX_STATS_UPDATE_TX_STATS(_pdev, _status, _p_cntrs, _b_cntrs)

#endif /*---*/ /* TXRX_STATS_LEVEL */


/*--- txrx sequence number trace macros ---*/


#define TXRX_SEQ_NUM_ERR(_status) (0xffff - _status)

#if defined(ENABLE_RX_REORDER_TRACE)

A_STATUS ol_rx_reorder_trace_attach(ol_txrx_pdev_handle pdev);
void ol_rx_reorder_trace_detach(ol_txrx_pdev_handle pdev);
void ol_rx_reorder_trace_add(
    ol_txrx_pdev_handle pdev,
    u_int8_t tid,
    u_int16_t reorder_idx,
    u_int16_t seq_num,
    int num_mpdus);

#define OL_RX_REORDER_TRACE_ATTACH ol_rx_reorder_trace_attach
#define OL_RX_REORDER_TRACE_DETACH ol_rx_reorder_trace_detach
#define OL_RX_REORDER_TRACE_ADD    ol_rx_reorder_trace_add

#else

#define OL_RX_REORDER_TRACE_ATTACH(_pdev) A_OK
#define OL_RX_REORDER_TRACE_DETACH(_pdev)
#define OL_RX_REORDER_TRACE_ADD(pdev, tid, reorder_idx, seq_num, num_mpdus)

#endif /* ENABLE_RX_REORDER_TRACE */


/*--- txrx packet number trace macros ---*/


#if defined(ENABLE_RX_PN_TRACE)

A_STATUS ol_rx_pn_trace_attach(ol_txrx_pdev_handle pdev);
void ol_rx_pn_trace_detach(ol_txrx_pdev_handle pdev);
void ol_rx_pn_trace_add(
    struct ol_txrx_pdev_t *pdev,
    struct ol_txrx_peer_t *peer,
    u_int16_t tid,
    void *rx_desc);

#define OL_RX_PN_TRACE_ATTACH ol_rx_pn_trace_attach
#define OL_RX_PN_TRACE_DETACH ol_rx_pn_trace_detach
#define OL_RX_PN_TRACE_ADD    ol_rx_pn_trace_add

#else

#define OL_RX_PN_TRACE_ATTACH(_pdev) A_OK
#define OL_RX_PN_TRACE_DETACH(_pdev)
#define OL_RX_PN_TRACE_ADD(pdev, peer, tid, rx_desc)

#endif /* ENABLE_RX_PN_TRACE */

static inline int
ol_txrx_ieee80211_hdrsize(const void *data)
{
    const struct ieee80211_frame *wh = (const struct ieee80211_frame *)data;
    int size = sizeof(struct ieee80211_frame);

    /* NB: we don't handle control frames */
    TXRX_ASSERT1(
        (wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_CTL);
    if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) == IEEE80211_FC1_DIR_DSTODS) {
        size += IEEE80211_ADDR_LEN;
    } 
    if (IEEE80211_QOS_HAS_SEQ(wh)) {
        size += sizeof(u_int16_t);
        /* Qos frame with Order bit set indicates an HTC frame */
        if (wh->i_fc[1] & IEEE80211_FC1_ORDER) {
            size += sizeof(struct ieee80211_htc);
        }
    }
    return size;
}

/*--- frame display utility ---*/

static inline void
txrx_frm_dump(adf_nbuf_t frm, int max_len)
{
    #define TXRX_FRM_DUMP_MAX_LEN 128
    u_int8_t local_buf[TXRX_FRM_DUMP_MAX_LEN];
    u_int8_t *p = adf_nbuf_data(frm);
    int i = 0, frag_num;

    if (max_len > adf_nbuf_len(frm)) {
        max_len = adf_nbuf_len(frm);
    }
    if (max_len > TXRX_FRM_DUMP_MAX_LEN) {
        max_len = TXRX_FRM_DUMP_MAX_LEN;
    }

    /* gather frame contents from netbuf fragments into a contiguous buffer */
    frag_num = 0;
    while (i < max_len) {
        int frag_bytes;
        frag_bytes = adf_nbuf_get_frag_len(frm, frag_num);
        if (frag_bytes > max_len - i) {
            frag_bytes = max_len - i;
        }
        if (frag_bytes > 0) {
            p = adf_nbuf_get_frag_vaddr(frm, frag_num);
            adf_os_mem_copy(&local_buf[i], p, frag_bytes);
        }
        frag_num++;
        i += frag_bytes;
    }

    adf_os_print("frame %p data (%p), hex dump of bytes 0-%d of %d:\n",
        frm, p, max_len-1, (int) adf_nbuf_len(frm));
    p = local_buf;
    while (max_len > 16) {
        adf_os_print("  " /* indent */
            "%02x %02x %02x %02x %02x %02x %02x %02x "
            "%02x %02x %02x %02x %02x %02x %02x %02x\n",
            *(p +  0), *(p +  1), *(p +  2), *(p +  3),
            *(p +  4), *(p +  5), *(p +  6), *(p +  7),
            *(p +  8), *(p +  9), *(p + 10), *(p + 11),
            *(p + 12), *(p + 13), *(p + 14), *(p + 15));
        p += 16;
        max_len -= 16;
    }
    adf_os_print("  " /* indent */);
    while (max_len > 0) {
        adf_os_print("%02x ", *p);
        p++;
        max_len--;
    }
    adf_os_print("\n");
}

#ifdef SUPPORT_HOST_STATISTICS

#define OL_RX_ERR_STATISTICS(pdev, vdev, err_type, sec_type, is_mcast) \
    ol_rx_err_statistics(pdev->ctrl_pdev, vdev->vdev_id, err_type,     \
			             sec_type, is_mcast);

#define OL_RX_ERR_STATISTICS_1(pdev, vdev, peer, rx_desc, err_type)                  \
do {                                                                                    \
    int is_mcast;                                                                    \
    enum htt_sec_type sec_type;                                                      \
    is_mcast = htt_rx_msdu_is_wlan_mcast(pdev->htt_pdev, rx_desc);                   \
    sec_type = peer->security[is_mcast ? txrx_sec_mcast : txrx_sec_ucast].sec_type;  \
	OL_RX_ERR_STATISTICS(pdev, vdev, err_type, pdev->sec_types[sec_type], is_mcast); \
} while (false)

#define OL_RX_ERR_INV_PEER_STATISTICS(pdev, rx_msdu)                             \
do {                                                                                \
	struct ieee80211_frame *wh = NULL;                                           \
	/*FIX THIS :Here htt_rx_mpdu_wifi_hdr_retrieve should be used. But at */     \
    /*present it seems it does not work.*/                                       \
	/*wh = (struct ieee80211_frame *)htt_rx_mpdu_wifi_hdr_retrieve(pdev->htt_pdev, rx_desc);*/ \
	                                                                             \
	/* this only apply to LL device.*/                                           \
	if (!ol_cfg_is_high_latency(pdev->ctrl_pdev) &&                              \
			ol_cfg_frame_type(pdev->ctrl_pdev) == wlan_frm_fmt_native_wifi) {    \
		/* For windows, it is always native wifi header .*/                      \
		wh = (struct ieee80211_frame*)adf_nbuf_data(rx_msdu);                    \
	}                                                                            \
	ol_rx_err_inv_peer_statistics(pdev->ctrl_pdev, wh, OL_RX_ERR_UNKNOWN_PEER);  \
} while (false)

#define OL_RX_ERR_STATISTICS_2(pdev, vdev, peer, rx_desc, rx_msdu, rx_status)                 \
do {                                                                                    \
	enum ol_rx_err_type err_type = OL_RX_ERR_NONE;                                   \
    switch (rx_status) {                                                             \
		case htt_rx_status_decrypt_err:                                              \
			 err_type = OL_RX_ERR_DECRYPT;                                           \
			 break;                                                                  \
		case htt_rx_status_tkip_mic_err:                                             \
			 err_type = OL_RX_ERR_TKIP_MIC;                                          \
			 break;                                                                  \
		case htt_rx_status_mpdu_length_err:                                          \
		     err_type = OL_RX_ERR_MPDU_LENGTH;                                       \
			 break;                                                                  \
		case htt_rx_status_mpdu_encrypt_required_err:                                \
		     err_type = OL_RX_ERR_ENCRYPT_REQUIRED;                                  \
			 break;                                                                  \
		case htt_rx_status_err_dup:                                                  \
		     err_type = OL_RX_ERR_DUP;                                               \
			 break;                                                                  \
		case htt_rx_status_err_fcs:                                                  \
		     err_type = OL_RX_ERR_FCS;                                               \
			 break;                                                                  \
		default:                                                                     \
			 err_type = OL_RX_ERR_UNKNOWN;                                           \
		     break;                                                                  \
	}                                                                                \
	if (vdev != NULL && peer != NULL) {                                              \
	    OL_RX_ERR_STATISTICS_1(pdev, vdev, peer, rx_mpdu_desc, err_type);            \
	} else {                                                                         \
		OL_RX_ERR_INV_PEER_STATISTICS(pdev, rx_msdu);                                \
	}                                                                                \
} while (false)
#else
#define OL_RX_ERR_STATISTICS(pdev, vdev, err_type, sec_type, is_mcast) 
#define OL_RX_ERR_STATISTICS_1(pdev, vdev, peer, rx_desc, err_type) 
#define OL_RX_ERR_STATISTICS_2(pdev, vdev, peer, rx_desc, rx_msdu, rx_status) 
#endif /* SUPPORT_HOST_STATISTICS */

#endif /* _OL_TXRX_INTERNAL__H_ */
