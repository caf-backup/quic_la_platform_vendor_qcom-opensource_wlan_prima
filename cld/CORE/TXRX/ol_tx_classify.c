/*
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#include <adf_nbuf.h>         /* adf_nbuf_t, etc. */
#include <htt.h>              /* HTT_TX_EXT_TID_MGMT */
#include <ol_htt_tx_api.h>    /* htt_tx_desc_tid */
#include <ol_txrx_api.h>      /* ol_txrx_vdev_handle */
#include <ol_txrx_ctrl_api.h> /* ol_txrx_sync */
#include <ol_txrx.h>
#include <ol_txrx_internal.h> /* TXRX_ASSERT1 */
#include <ol_txrx_types.h>    /* pdev stats */
#include <ol_tx_desc.h>       /* ol_tx_desc */
#include <ol_tx_send.h>       /* ol_tx_send */
#include <ol_txrx_peer_find.h>
#include <ol_tx_classify.h>
#include <ol_tx_queue.h>
#include <ipv4.h>
#include <ipv6.h>
#include <ip_prot.h>
#include <enet.h>             /* ETHERTYPE_VLAN, etc. */
#include <ieee80211_common.h>        /* ieee80211_frame */


/*
 * In theory, this tx classify code could be used on the host or in the target.
 * Thus, this code uses generic OS primitives, that can be aliased to either
 * the host's OS primitives or the target's OS primitives.
 * For now, the following #defines set up these host-specific or
 * target-specific aliases.
 */

#if defined(CONFIG_HL_SUPPORT)

#ifdef FEATURE_WLAN_INTEGRATED_SOC

A_STATUS
ol_tx_classify_extension(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_nbuf, 
    struct ol_txrx_msdu_info_t *tx_msdu_info);

A_STATUS
ol_tx_classify_mgmt_extension(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_nbuf, 
    struct ol_txrx_msdu_info_t *tx_msdu_info);

#define OL_TX_CLASSIFY_EXTENSION(vdev, tx_desc, netbuf, msdu_info, txq) \
    do { \
        A_STATUS status; \
        status = ol_tx_classify_extension( \
            vdev, tx_desc, netbuf, msdu_info); \
        if (A_OK != status) { \
            txq = NULL; /* error */ \
        } \
    } while (0)

#define OL_TX_CLASSIFY_MGMT_EXTENSION(vdev, tx_desc, netbuf, msdu_info, txq) \
    do { \
        A_STATUS status; \
        status = ol_tx_classify_mgmt_extension( \
            vdev, tx_desc, netbuf, msdu_info); \
        if (A_OK != status) { \
            txq = NULL; /* error */ \
        } \
    } while (0)

#else
#define OL_TX_CLASSIFY_EXTENSION /* no-op */
#define OL_TX_CLASSIFY_MGMT_EXTENSION /* no-op */
#endif /* FEATURE_WLAN_INTEGRATED_SOC */

/* EAPOL go with voice priority: WMM_AC_TO_TID1(WMM_AC_VO);*/
#define TX_EAPOL_TID  6

/* ARP go with voice priority: WMM_AC_TO_TID1(pdev->arp_ac_override)*/
#define TX_ARP_TID  6

/* For non-IP case, use default TID */
#define TX_DEFAULT_TID  0

/* Determine IPTOS priority 
 * IP Tos format :
 *        (Refer Pg 57 WMM-test-plan-v1.2)
 * IP-TOS - 8bits
 *            : DSCP(6-bits) ECN(2-bits)
 *            : DSCP - P2 P1 P0 X X X
 *                where (P2 P1 P0) form 802.1D
 */
static inline A_UINT8 
ol_tx_tid_by_ipv4(
    A_UINT8 *pkt)
{
    A_UINT8 ipPri, tid;
    struct ipv4_hdr_t *ipHdr = (struct ipv4_hdr_t *)pkt;

    ipPri = ipHdr->tos >> 5;
    tid = ipPri & 0x7;

    return tid;
}

static inline A_UINT8 
ol_tx_tid_by_ipv6(
    A_UINT8 *pkt)
{
#if 1
    A_UINT8 tid;
    u_int32_t ver_pri_flowlabel;
    u_int32_t pri;

    ver_pri_flowlabel =
        (pkt[0] << 24) | (pkt[1] << 16) | (pkt[2] << 8) | pkt[3];
    pri = (ver_pri_flowlabel & 0x0FF00000) >> 20;
    tid = (pri >> 5) & 0x7;

    return tid;
#else
    /* use this once the CL with the IPV6_TRAFFIC_CLASS has been merged */
    return (IPV6_TRAFFIC_CLASS((struct ipv6_hdr_t *) pkt) >> 5) & 0x7;
#endif
}

static inline A_UINT8 
ol_tx_tid_by_ether_type(
    A_UINT8 *datap,
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    int tid;
    A_UINT8 *l3_data_ptr;
    A_UINT16 typeorlength;
    A_UINT8 * ptr;
    
    ptr = (datap + ETHERNET_ADDR_LEN * 2);
    typeorlength = (ptr[0] << 8) | ptr[1];
    l3_data_ptr = datap + sizeof(struct ethernet_hdr_t);//ETHERNET_HDR_LEN;

    if (typeorlength == ETHERTYPE_VLAN) {
        ptr = (datap + ETHERNET_ADDR_LEN * 2 + ETHERTYPE_VLAN_LEN);
        typeorlength = (ptr[0] << 8) | ptr[1];
        l3_data_ptr += ETHERTYPE_VLAN_LEN;
    }
    
    if (!IS_ETHERTYPE(typeorlength)) { // 802.3 header
        struct llc_snap_hdr_t *llc_hdr = (struct llc_snap_hdr_t *) l3_data_ptr;
        typeorlength = (llc_hdr->ethertype[0] << 8) | llc_hdr->ethertype[1];
        l3_data_ptr += sizeof(struct llc_snap_hdr_t);
    }
    tx_msdu_info->htt.info.l3_hdr_offset = l3_data_ptr - datap;
    tx_msdu_info->htt.info.ethertype = typeorlength;
    
    /* IP packet, do packet inspection for TID */
    if (typeorlength == ETHERTYPE_IPV4) {
        tid = ol_tx_tid_by_ipv4(l3_data_ptr);
    } else if (typeorlength == ETHERTYPE_IPV6) {
        tid = ol_tx_tid_by_ipv6(l3_data_ptr);
    } else if (ETHERTYPE_IS_EAPOL_WAPI(typeorlength)) {
        /* EAPOL go with voice priority*/
        tid = TX_EAPOL_TID;
    } else if (typeorlength == ETHERTYPE_ARP) {
        tid = TX_ARP_TID;
    } else {
        /* For non-IP case, use default TID */
        tid = TX_DEFAULT_TID; 
    }
    return tid;
}

static inline A_UINT8 
ol_tx_tid_by_raw_type(
    A_UINT8 *datap,
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    int tid = HTT_TX_EXT_TID_NON_QOS_MCAST_BCAST;
    
    /* adjust hdr_ptr to RA */
    struct ieee80211_frame *wh = (struct ieee80211_frame *)datap;
    
    /* FIXME: This code does not handle 4 address formats. The QOS field
     * is not at usual location. 
     */
    if ((wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA) {
        struct llc_snap_hdr_t *llc;
        /* dot11 encapsulated frame */
        struct ieee80211_qosframe *whqos = (struct ieee80211_qosframe *)datap;
        if (whqos->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS) {
            tid = whqos->i_qos[0] & IEEE80211_QOS_TID;
            tx_msdu_info->htt.info.l3_hdr_offset =
                sizeof(struct ieee80211_qosframe);
        } else {
            tid = HTT_NON_QOS_TID;
            tx_msdu_info->htt.info.l3_hdr_offset =
                sizeof(struct ieee80211_frame);
        }
        llc = (struct llc_snap_hdr_t *)
            (datap + tx_msdu_info->htt.info.l3_hdr_offset);
        tx_msdu_info->htt.info.ethertype =
            (llc->ethertype[0] << 8) | llc->ethertype[1];
//TBD: l3_hdr_offset += sizeof(*llc)?
    } else {
        /*
         * This function should only be applied to data frames.
         * For management frames, we already know to use HTT_TX_EXT_TID_MGMT.
         */
        adf_os_assert(FALSE);
    }
    return tid;
}

int
ol_tx_tid(
    struct ol_txrx_pdev_t *pdev, 
    adf_nbuf_t tx_nbuf, 
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    A_UINT8 *datap = adf_nbuf_data(tx_nbuf);
    int tid;

    if (pdev->frame_format == wlan_frm_fmt_raw) {
        tx_msdu_info->htt.info.l2_hdr_type = htt_pkt_type_raw;
        tid = ol_tx_tid_by_raw_type(datap, tx_msdu_info);
    } else if (pdev->frame_format == wlan_frm_fmt_802_3) {
        tx_msdu_info->htt.info.l2_hdr_type = htt_pkt_type_ethernet;
        tid = ol_tx_tid_by_ether_type(datap, tx_msdu_info);
    } else if (pdev->frame_format == wlan_frm_fmt_native_wifi) {
        struct llc_snap_hdr_t *llc;

        tx_msdu_info->htt.info.l2_hdr_type = htt_pkt_type_native_wifi;
        tx_msdu_info->htt.info.l3_hdr_offset = sizeof(struct ieee80211_frame);
        llc = (struct llc_snap_hdr_t *)
            (datap + tx_msdu_info->htt.info.l3_hdr_offset);
        tx_msdu_info->htt.info.ethertype =
            (llc->ethertype[0] << 8) | llc->ethertype[1];
        /*
         * Native WiFi is a special case of "raw" 802.11 header format.
         * However, we expect that for all cases that use native WiFi,
         * the TID will be directly specified out of band.
         */
         //FIXME_RT Hardcode TID info for now until correct method of extracting TID from OS Shim is checked in.
        tx_msdu_info->htt.info.ext_tid = 0;
        tid = tx_msdu_info->htt.info.ext_tid;
    } else {
        adf_os_print("Invalid standard frame type: %d\n", pdev->frame_format);
        adf_os_assert(FALSE);
        tid = HTT_TX_EXT_TID_INVALID;
    }
    return tid;
}

void *
ol_tx_dest_addr_find(
    struct ol_txrx_pdev_t *pdev, 
    adf_nbuf_t tx_nbuf)
{
    A_UINT8 *hdr_ptr;
    void *datap = adf_nbuf_data(tx_nbuf);

    if (pdev->frame_format == wlan_frm_fmt_raw) {
        /* adjust hdr_ptr to RA */
        struct ieee80211_frame *wh = (struct ieee80211_frame *)datap;
        hdr_ptr = wh->i_addr1;
    } else if (pdev->frame_format == wlan_frm_fmt_native_wifi) {
        //pdev->htt_pkt_type == htt_pkt_type_native_wifi;
        /* adjust hdr_ptr to RA */
        struct ieee80211_frame *wh = (struct ieee80211_frame *)datap;
        hdr_ptr = wh->i_addr1; 
    } else if (pdev->frame_format == wlan_frm_fmt_802_3) {
        //pdev->htt_pkt_type == htt_pkt_type_ethernet;
        hdr_ptr = datap;
    } else {
        adf_os_print("Invalid standard frame type: %d\n", pdev->frame_format);
        adf_os_assert(FALSE);
        hdr_ptr = NULL;
    }
    return hdr_ptr;
}

struct ol_tx_frms_queue_t *
ol_tx_classify(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_nbuf, 
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_txrx_peer_t *peer = NULL;
    struct ol_tx_frms_queue_t *txq = NULL;
    u_int8_t *dest_addr;
    int tid;

    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);

    /* temporary debug info - remove this after initial testing */
    //adf_os_print("vdev %p opmode %d\n", vdev, vdev->opmode);
    //txrx_frm_dump(tx_nbuf, 64 /* enough to show L2 and L3 headers */);

    dest_addr = ol_tx_dest_addr_find(pdev, tx_nbuf);
    if (IEEE80211_IS_MULTICAST(dest_addr)) {
        txq = &vdev->txqs[OL_TX_VDEV_MCAST_BCAST];
        tx_msdu_info->htt.info.ext_tid =
            OL_TX_NUM_TIDS + OL_TX_VDEV_MCAST_BCAST;
        if (vdev->opmode == wlan_op_mode_sta) {
            /*
             * The STA sends a frame with a broadcast dest addr (DA) as a
             * unicast frame to the AP's receive addr (RA).
             * Find the peer object that represents the AP that the STA
             * is associated with.
             */
            peer = ol_txrx_assoc_peer_find(vdev);
            if (!peer) {
                adf_os_print(
                    "Error: STA %p (%02x:%02x:%02x:%02x:%02x:%02x) "
                    "trying to send bcast DA tx data frame "
                    "w/o association\n",
                    vdev,
                    vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
                    vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
                    vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);
                return NULL; /* error */
            }
            /*
             * The following line assumes each peer object has a single ID.
             * This is currently true, and is expected to remain true.
             */
            tx_msdu_info->htt.info.peer_id = peer->peer_ids[0];
        } else {
            tx_msdu_info->htt.info.peer_id = HTT_INVALID_PEER_ID;
        }
        tx_msdu_info->htt.info.is_unicast = FALSE;
    } else {
        tid = ol_tx_tid(pdev, tx_nbuf, tx_msdu_info);
        if (HTT_TX_EXT_TID_INVALID == tid) {
             adf_os_print(
                 "%s Error: could not classify packet into valid TID.\n",
                 __func__);
             return NULL;
        }
        /*
         * Find the peer and increment its reference count.
         * If this vdev is an AP, use the dest addr (DA) to determine
         * which peer STA this unicast data frame is for.
         * If this vdev is a STA, the unicast data frame is for the
         * AP the STA is associated with.
         */
        if (vdev->opmode == wlan_op_mode_sta) {
//CHECK THIS - this probably needs to be changed to support TDLS
            peer = ol_txrx_assoc_peer_find(vdev);
        } else {
            peer = ol_txrx_peer_find_hash_find(pdev, dest_addr, 0);
        }
        tx_msdu_info->htt.info.is_unicast = TRUE;
        if (!peer) {
            /*
             * Unicast data xfer can only happen to an associated peer.
             * It is illegitimate to send unicast data if there is no peer
             * to send it to.
             */
            adf_os_print(
                "Error: vdev %p (%02x:%02x:%02x:%02x:%02x:%02x) "
                "trying to send unicast tx data frame to an unknown peer\n",
                vdev,
                vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
                vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
                vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);
            return NULL; /* error */
        }
        TX_SCHED_DEBUG_PRINT("Peer exist\n");
        txq = &peer->txqs[tid];
        tx_msdu_info->htt.info.ext_tid = tid;
        /*
         * The following line assumes each peer object has a single ID.
         * This is currently true, and is expected to remain true.
         */
        tx_msdu_info->htt.info.peer_id = peer->peer_ids[0];
        /*
         * WORKAROUND - check that the peer ID is valid.
         * If tx data is provided before ol_rx_peer_map_handler is called
         * to record the peer ID specified by the target, then we could
         * end up here with an invalid peer ID.
         * TO DO: rather than dropping the tx frame, pause the txq it
         * goes into, then fill in the peer ID for the entries in the
         * txq when the peer_map event provides the peer ID, and then
         * unpause the txq.
         */
        if (tx_msdu_info->htt.info.peer_id == HTT_INVALID_PEER_ID) {
            return NULL;
        }
    }
    tx_msdu_info->peer = peer;
    /*
     * If relevant, do a deeper inspection to determine additional
     * characteristics of the tx frame.
     * If the frame is invalid, then the txq will be set to NULL to
     * indicate an error.
     */
    OL_TX_CLASSIFY_EXTENSION(vdev, tx_desc, tx_nbuf, tx_msdu_info, txq);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
    return txq;
}

struct ol_tx_frms_queue_t *
ol_tx_classify_mgmt(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_nbuf,
    struct ol_txrx_msdu_info_t *tx_msdu_info)
{
    struct ol_txrx_pdev_t *pdev = vdev->pdev;
    struct ol_txrx_peer_t *peer = NULL;
    struct ol_tx_frms_queue_t *txq = NULL;
    u_int8_t *dest_addr;

    TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);
    dest_addr = ol_tx_dest_addr_find(pdev, tx_nbuf);
    if (IEEE80211_IS_MULTICAST(dest_addr)) {
        /*
         * AP:  beacons are broadcast,
         *      public action frames (e.g. extended channel switch announce)
         *      may be broadcast
         * STA: probe requests can be either broadcast or unicast
         */
        txq = &vdev->txqs[OL_TX_VDEV_DEFAULT_MGMT];
        tx_msdu_info->htt.info.ext_tid =
            OL_TX_NUM_TIDS + OL_TX_VDEV_DEFAULT_MGMT;
        tx_msdu_info->htt.info.peer_id = HTT_INVALID_PEER_ID;
        tx_msdu_info->peer = NULL;
        tx_msdu_info->htt.info.is_unicast = 0;
    } else {
        /*
         * Find the peer and increment its reference count.
         * If this vdev is an AP, use the receiver addr (RA) to determine
         * which peer STA this unicast mgmt frame is for.
         * If this vdev is a STA, the unicast mgmt frame is for the
         * AP the STA is associated with.
         * Probe request / response and Assoc request / response are
         * sent before the peer exists - in this case, use the
         * vdev's default tx queue.
         */
        if (vdev->opmode == wlan_op_mode_sta) {
            peer = ol_txrx_assoc_peer_find(vdev);
        } else {
            /* find the peer and increment its reference count */
            peer = ol_txrx_peer_find_hash_find(pdev, dest_addr, 0);
        }
        tx_msdu_info->peer = peer;
        if (!peer) {
            txq = &vdev->txqs[OL_TX_VDEV_DEFAULT_MGMT];
            tx_msdu_info->htt.info.ext_tid =
                OL_TX_NUM_TIDS + OL_TX_VDEV_DEFAULT_MGMT;
            tx_msdu_info->htt.info.peer_id = HTT_INVALID_PEER_ID;
        } else {
            txq = &peer->txqs[HTT_TX_EXT_TID_MGMT];
            tx_msdu_info->htt.info.ext_tid = HTT_TX_EXT_TID_MGMT;
            /*
             * The following line assumes each peer object has a single ID.
             * This is currently true, and is expected to remain true.
             */
            tx_msdu_info->htt.info.peer_id = peer->peer_ids[0];
        }
        tx_msdu_info->htt.info.is_unicast = 1;
    }
    /*
     * If relevant, do a deeper inspection to determine additional
     * characteristics of the tx frame.
     * If the frame is invalid, then the txq will be set to NULL to
     * indicate an error.
     */
    OL_TX_CLASSIFY_MGMT_EXTENSION(vdev, tx_desc, tx_nbuf, tx_msdu_info, txq);
    TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
    return txq;
}

A_STATUS
ol_tx_classify_extension(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_msdu, 
    struct ol_txrx_msdu_info_t *msdu_info)
{
    u_int8_t *datap = adf_nbuf_data(tx_msdu);
    struct ol_txrx_peer_t *peer;
    int which_key;

    /*
     * The following msdu_info fields were already filled in by the
     * ol_tx entry function or the regular ol_tx_classify function:
     *     htt.info.vdev_id            (ol_tx_hl or ol_tx_non_std_hl)
     *     htt.info.ext_tid            (ol_tx_non_std_hl or ol_tx_classify)
     *     htt.info.frame_type         (ol_tx_hl or ol_tx_non_std_hl)
     *     htt.info.l2_hdr_type        (ol_tx_hl or ol_tx_non_std_hl)
     *     htt.info.is_unicast         (ol_tx_classify)
     *     htt.info.peer_id            (ol_tx_classify)
     *     peer                        (ol_tx_classify)
     *     if (is_unicast) {
     *         htt.info.ethertype      (ol_tx_classify)
     *         htt.info.l3_hdr_offset  (ol_tx_classify)
     *     }
     * The following fields need to be filled in by this function:
     *     if (!is_unicast) {
     *         htt.info.ethertype
     *         htt.info.l3_hdr_offset
     *     }
     *     htt.action.band (NOT CURRENTLY USED)
     *     htt.action.do_encrypt
     *     htt.action.do_tx_complete
     * The following fields are not needed for data frames, and can
     * be left uninitialized:
     *     htt.info.frame_subtype
     */

    if (!msdu_info->htt.info.is_unicast) {
        int l2_hdr_size;
        u_int16_t ethertype;

        if (msdu_info->htt.info.l2_hdr_type == htt_pkt_type_ethernet) {
            struct ethernet_hdr_t *eh;

            eh = (struct ethernet_hdr_t *) datap;
            l2_hdr_size = sizeof(*eh);
            ethertype = (eh->ethertype[0] << 8) | eh->ethertype[1];

            if (ethertype == ETHERTYPE_VLAN) {
                struct ethernet_vlan_hdr_t *evh;
 
                evh = (struct ethernet_vlan_hdr_t *) datap;
                l2_hdr_size = sizeof(*evh);
                ethertype = (evh->ethertype[0] << 8) | evh->ethertype[1];
            }

            if (!IS_ETHERTYPE(ethertype)) { // 802.3 header
                struct llc_snap_hdr_t *llc =
                    (struct llc_snap_hdr_t *) (datap + l2_hdr_size);
                ethertype = (llc->ethertype[0] << 8) | llc->ethertype[1];
                l2_hdr_size += sizeof(*llc);
            }
            msdu_info->htt.info.l3_hdr_offset = l2_hdr_size;
            msdu_info->htt.info.ethertype = ethertype;
        } else { /* 802.11 */
            struct llc_snap_hdr_t *llc;
            l2_hdr_size = ol_txrx_ieee80211_hdrsize(datap);
            llc = (struct llc_snap_hdr_t *) (datap + l2_hdr_size);
            ethertype = (llc->ethertype[0] << 8) | llc->ethertype[1];
//TBD: l2_hdr_size += sizeof(*llc)?
        }
        msdu_info->htt.info.l3_hdr_offset = l2_hdr_size;
        msdu_info->htt.info.ethertype = ethertype;
        which_key = txrx_sec_mcast;
    } else {
        which_key = txrx_sec_ucast;
    }
    peer = msdu_info->peer;
    /* msdu_info->htt.action.do_encrypt is partially set in ol_tx_desc_hl.
     * Add more check here.
     */
    msdu_info->htt.action.do_encrypt = (!peer) ? 0 :
        (peer->security[which_key].sec_type == htt_sec_type_none) ? 0 : 
        msdu_info->htt.action.do_encrypt;
    /*
     * For systems that have a frame by frame spec for whether to receive
     * a tx completion notification, use the tx completion notification only
     * for certain management frames, not for data frames.
     * (In the future, this may be changed slightly, e.g. to request a
     * tx completion notification for the final EAPOL message sent by a
     * STA during the key delivery handshake.)
     */
    msdu_info->htt.action.do_tx_complete = 0;

    return A_OK;
}

A_STATUS
ol_tx_classify_mgmt_extension(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t tx_msdu, 
    struct ol_txrx_msdu_info_t *msdu_info)
{
    struct ieee80211_frame *wh;

    /*
     * The following msdu_info fields were already filled in by the
     * ol_tx entry function or the regular ol_tx_classify_mgmt function:
     *     htt.info.vdev_id          (ol_txrx_mgmt_send)
     *     htt.info.frame_type       (ol_txrx_mgmt_send)
     *     htt.info.l2_hdr_type      (ol_txrx_mgmt_send)
     *     htt.action.do_tx_complete (ol_txrx_mgmt_send)
     *     htt.info.peer_id          (ol_tx_classify_mgmt)
     *     htt.info.ext_tid          (ol_tx_classify_mgmt)
     *     htt.info.is_unicast       (ol_tx_classify_mgmt)
     *     peer                      (ol_tx_classify_mgmt)
     * The following fields need to be filled in by this function:
     *     htt.info.frame_subtype
     *     htt.info.l3_hdr_offset
     *     htt.action.band (NOT CURRENTLY USED)
     * The following fields are not needed for mgmt frames, and can
     * be left uninitialized:
     *     htt.info.ethertype
     *     htt.action.do_encrypt
     *         (This will be filled in by other SW, which knows whether
     *         the peer has robust-managment-frames enabled.)
     */
    wh = (struct ieee80211_frame *) adf_nbuf_data(tx_msdu);
    msdu_info->htt.info.frame_subtype =
        (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) >>
        IEEE80211_FC0_SUBTYPE_SHIFT;
    msdu_info->htt.info.l3_hdr_offset = sizeof(struct ieee80211_frame);

    return A_OK;
}

#endif /* defined(CONFIG_HL_SUPPORT) */
