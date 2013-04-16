/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file ol_txrx_types.h
 * @brief Define the major data types used internally by the host datapath SW.
 */
#ifndef _OL_TXRX_TYPES__H_
#define _OL_TXRX_TYPES__H_

#include <adf_nbuf.h>         /* adf_nbuf_t */
#include <queue.h>            /* TAILQ */
#include <a_types.h>	      /* A_UINT8 */
#include <htt.h>	      /* htt_sec_type, htt_pkt_type, etc. */
#include <adf_os_atomic.h>    /* adf_os_atomic_t */
#include <wdi_event.h>	      /* wdi_event_subscribe */
#include <adf_os_timer.h>     /* adf_os_timer_t */
#include <adf_os_lock.h>      /* adf_os_spinlock */
#include <pktlog.h>	      /* ol_pktlog_dev_handle */
#include <ol_txrx_stats.h>
#include <txrx.h>
#include "ol_txrx_htt_api.h"
#include "ol_htt_tx_api.h"
#include "ol_htt_rx_api.h"
#include <ol_ctrl_txrx_api.h>
#include <ol_txrx_ctrl_api.h>
/*
 * The target may allocate multiple IDs for a peer.
 * In particular, the target may allocate one ID to represent the
 * multicast key the peer uses, and another ID to represent the
 * unicast key the peer uses.
 */
#define MAX_NUM_PEER_ID_PER_PEER 8

#define OL_TXRX_MAC_ADDR_LEN 6

/* OL_TXRX_NUM_EXT_TIDS -
 * 16 "real" TIDs + 3 pseudo-TIDs for mgmt, mcast/bcast & non-QoS data
 */
#define OL_TXRX_NUM_EXT_TIDS 19

#define OL_TX_NUM_QOS_TIDS 16 /* 16 regular TIDs */
#define OL_TX_NON_QOS_TID 16
#define OL_TX_MGMT_TID    17
#define OL_TX_NUM_TIDS    18

#define OL_TX_VDEV_MCAST_BCAST    0 // HTT_TX_EXT_TID_MCAST_BCAST
#define OL_TX_VDEV_DEFAULT_MGMT   1 // HTT_TX_EXT_TID_DEFALT_MGMT
#define OL_TX_VDEV_NUM_QUEUES     2

#define OL_TXRX_MGMT_TYPE_BASE htt_pkt_num_types
#define OL_TXRX_MGMT_NUM_TYPES 8

#define OL_TX_MUTEX_TYPE adf_os_spinlock_t
#define OL_RX_MUTEX_TYPE adf_os_spinlock_t

struct ol_txrx_pdev_t;
struct ol_txrx_vdev_t;
struct ol_txrx_peer_t;

struct ol_pdev_t;
typedef struct ol_pdev_t* ol_pdev_handle;

struct ol_vdev_t;
typedef struct ol_vdev_t* ol_vdev_handle;

struct ol_peer_t;
typedef struct ol_peer_t* ol_peer_handle;

/* rx filter related */
#define MAX_PRIVACY_FILTERS           4 /* max privacy filters */

typedef enum _privacy_filter {
	PRIVACY_FILTER_ALWAYS,
	PRIVACY_FILTER_KEY_UNAVAILABLE,
} privacy_filter ;

typedef enum _privacy_filter_packet_type {
	PRIVACY_FILTER_PACKET_UNICAST,
	PRIVACY_FILTER_PACKET_MULTICAST,
	PRIVACY_FILTER_PACKET_BOTH
} privacy_filter_packet_type ;

typedef struct _privacy_excemption_filter {
	/* type of ethernet to apply this filter, in host byte order*/
	u_int16_t ether_type;
	privacy_filter filter_type;
	privacy_filter_packet_type packet_type;
} privacy_exemption;

enum ol_tx_frm_type {
	ol_tx_frm_std = 0, /* regular frame - no added header fragments */
	ol_tx_frm_tso,     /* TSO segment, with a modified IP header added */
	ol_tx_frm_audio,   /* audio frames, with a custom LLC/SNAP header added */
};

struct ol_tx_desc_t {
	adf_nbuf_t netbuf;
	void *htt_tx_desc;
	adf_os_atomic_t ref_cnt;
	enum htt_tx_status status;

	/*
	 * Allow tx descriptors to be stored in (doubly-linked) lists.
	 * This is mainly used for HL tx queuing and scheduling, but is
	 * also used by LL+HL for batch processing of tx frames.
	 */
	TAILQ_ENTRY(ol_tx_desc_t) tx_desc_list_elem;

	/*
	 * Remember whether the tx frame is a regular packet, or whether
	 * the driver added extra header fragments (e.g. a modified IP header
	 * for TSO fragments, or an added LLC/SNAP header for audio interworking
	 * data) that need to be handled in a special manner.
	 * This field is filled in with the ol_tx_frm_type enum.
	 */
	u_int8_t pkt_type;
#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
	/* used by tx encap, to restore the os buf start offset after tx complete*/
	u_int8_t orig_l2_hdr_bytes;
#endif
};

typedef TAILQ_HEAD(, ol_tx_desc_t) ol_tx_desc_list;

union ol_tx_desc_list_elem_t {
	union ol_tx_desc_list_elem_t *next;
	struct ol_tx_desc_t tx_desc;
};

union ol_txrx_align_mac_addr_t {
	u_int8_t raw[OL_TXRX_MAC_ADDR_LEN];
	struct {
		u_int16_t bytes_ab;
		u_int16_t bytes_cd;
		u_int16_t bytes_ef;
	} align2;
	struct {
		u_int32_t bytes_abcd;
		u_int16_t bytes_ef;
	} align4;
};

struct ol_rx_reorder_timeout_list_elem_t
{
	TAILQ_ENTRY(ol_rx_reorder_timeout_list_elem_t) reorder_timeout_list_elem;
	u_int32_t timestamp_ms;
	struct ol_txrx_peer_t *peer;
	u_int8_t tid;
	u_int8_t active;
};

#define TXRX_TID_TO_WMM_AC(_tid) (\
		(((_tid) >> 1) == 3) ? TXRX_WMM_AC_VO : \
		(((_tid) >> 1) == 2) ? TXRX_WMM_AC_VI : \
		(((_tid) ^ ((_tid) >> 1)) & 0x1) ? TXRX_WMM_AC_BK : \
		TXRX_WMM_AC_BE)

struct ol_tx_reorder_cat_timeout_t {
	TAILQ_HEAD(, ol_rx_reorder_timeout_list_elem_t) virtual_timer_list;
	adf_os_timer_t timer;
	u_int32_t duration_ms;
	struct ol_txrx_pdev_t *pdev;
};

enum ol_tx_scheduler_status {
	ol_tx_scheduler_idle = 0,
	ol_tx_scheduler_running,
};

enum ol_tx_queue_status {
	ol_tx_queue_empty = 0,
	ol_tx_queue_active,
	ol_tx_queue_paused,
};

struct ol_txrx_msdu_info_t {
	struct htt_msdu_info_t htt;
	struct ol_txrx_peer_t *peer;
};

enum {
	ol_tx_aggr_untried = 0,
	ol_tx_aggr_enabled,
	ol_tx_aggr_disabled,
	ol_tx_aggr_retry,
	ol_tx_aggr_in_progress,
};

struct ol_tx_frms_queue_t {
	/* Which is used to queue up into scheduler queues*/
	TAILQ_ENTRY(ol_tx_frms_queue_t) list_elem;

	u_int8_t aggr_state;
	struct {
		u_int8_t total;
		/* pause requested by ctrl SW rather than txrx SW */
		u_int8_t by_ctrl;
	} paused_count;
	u_int8_t ext_tid;
	u_int16_t frms;
	u_int32_t bytes;
	ol_tx_desc_list head;
	enum ol_tx_queue_status flag;
};

enum {
	ol_tx_log_entry_type_invalid,
	ol_tx_log_entry_type_queue_state,
	ol_tx_log_entry_type_enqueue,
	ol_tx_log_entry_type_dequeue,
	ol_tx_log_entry_type_drop,
	ol_tx_log_entry_type_queue_free,

	ol_tx_log_entry_type_wrap,
};

struct ol_tx_log_queue_state_var_sz_t {
	u_int32_t active_bitmap;
	u_int16_t credit;
	u_int8_t  num_cats_active;
	u_int8_t  data[1];
};

struct ol_tx_log_queue_add_t {
	u_int8_t  num_frms;
	u_int8_t  tid;
	u_int16_t peer_id;
	u_int16_t num_bytes;
};

struct ol_mac_addr {
	u_int8_t mac_addr[OL_TXRX_MAC_ADDR_LEN];
};

struct ol_tx_sched_t;
typedef struct ol_tx_sched_t *ol_tx_sched_handle;

/*
 * As depicted in the diagram below, the pdev contains an array of
 * NUM_EXT_TID ol_tx_active_queues_in_tid_t elements.
 * Each element identifies all the tx queues that are active for
 * the TID, from the different peers.
 *
 * Each peer contains an array of NUM_EXT_TID ol_tx_frms_queue_t elements.
 * Each element identifies the tx frames for the TID that need to be sent
 * to the peer.
 *
 *
 *  pdev: ol_tx_active_queues_in_tid_t active_in_tids[NUM_EXT_TIDS]
 *                                TID
 *       0            1            2                     17
 *  +============+============+============+==    ==+============+
 *  | active (y) | active (n) | active (n) |        | active (y) |
 *  |------------+------------+------------+--    --+------------|
 *  | queues     | queues     | queues     |        | queues     |
 *  +============+============+============+==    ==+============+
 *       |                                               |
 *    .--+-----------------------------------------------'
 *    |  |
 *    |  |     peer X:                            peer Y:
 *    |  |     ol_tx_frms_queue_t                 ol_tx_frms_queue_t
 *    |  |     tx_queues[NUM_EXT_TIDS]            tx_queues[NUM_EXT_TIDS]
 *    |  | TID +======+                       TID +======+
 *    |  `---->| next |-------------------------->| next |--X
 *    |     0  | prev |   .------.   .------.  0  | prev |   .------.
 *    |        | txq  |-->|txdesc|-->|txdesc|     | txq  |-->|txdesc|
 *    |        +======+   `------'   `------'     +======+   `------'
 *    |        | next |      |          |      1  | next |      |
 *    |     1  | prev |      v          v         | prev |      v
 *    |        | txq  |   .------.   .------.     | txq  |   .------.
 *    |        +======+   |netbuf|   |netbuf|     +======+   |netbuf|
 *    |        | next |   `------'   `------'     | next |   `------'
 *    |     2  | prev |                        2  | prev |
 *    |        | txq  |                           | txq  |
 *    |        +======+                           +======+
 *    |        |      |                           |      |
 *    |
 *    |
 *    |        |      |                           |      |
 *    |        +======+                           +======+
 *    `------->| next |--X                        | next |
 *          17 | prev |   .------.             17 | prev |
 *             | txq  |-->|txdesc|                | txq  |
 *             +======+   `------'                +======+
 *                           |
 *                           v
 *                        .------.
 *                        |netbuf|
 *                        `------'
 */
struct ol_txrx_pdev_t {
	/* ctrl_pdev - handle for querying config info */
	ol_pdev_handle ctrl_pdev;

	/* osdev - handle for mem alloc / free, map / unmap */
	adf_os_device_t osdev;

	htt_pdev_handle htt_pdev;

	struct {
		int is_high_latency;
		int host_addba;
	} cfg;

	/* WDI subscriber's event list */
	wdi_event_subscribe **wdi_event_list;

#ifndef REMOVE_PKT_LOG
	/* Pktlog pdev */
	 struct ol_pktlog_dev_t* pl_dev;
#endif	/* #ifndef REMOVE_PKT_LOG */

	enum ol_sec_type sec_types[htt_num_sec_types];
	/* standard frame type */
	enum wlan_frm_fmt frame_format;
	enum htt_pkt_type htt_pkt_type;

#ifdef QCA_SUPPORT_SW_TXRX_ENCAP
	/* txrx encap/decap   */
	u_int8_t sw_tx_encap;
	u_int8_t sw_rx_decap;
	u_int8_t target_tx_tran_caps;
	u_int8_t target_rx_tran_caps;
	/* llc process */
	u_int8_t sw_tx_llc_proc_enable;
	u_int8_t sw_rx_llc_proc_enable;
	/* A-MSDU */
	u_int8_t sw_subfrm_hdr_recovery_enable;
	/* Protected Frame bit handling */
	u_int8_t sw_pf_proc_enable;
#endif
	/*
	 * target tx credit -
	 * not needed for LL, but used for HL download scheduler to keep
	 * track of roughly how much space is available in the target for
	 * tx frames
	 */
	adf_os_atomic_t target_tx_credit;

	/* Peer mac address to staid mapping */
	struct ol_mac_addr mac_to_staid[WLAN_MAX_STA_COUNT + 3];

	/* ol_txrx_vdev list */
	TAILQ_HEAD(, ol_txrx_vdev_t) vdev_list;

	/* peer ID to peer object map (array of pointers to peer objects) */
	struct ol_txrx_peer_t **peer_id_to_obj_map;

	struct {
		unsigned mask;
		unsigned idx_bits;
		TAILQ_HEAD(, ol_txrx_peer_t) *bins;
	} peer_hash;

	/* rx specific processing */
	struct {
		struct {
			TAILQ_HEAD(, ol_rx_reorder_t) waitlist;
			u_int32_t timeout_ms;
		} defrag;
		struct {
			int defrag_timeout_check;
			int dup_check;
		} flags;

		struct {
			struct ol_tx_reorder_cat_timeout_t access_cats[TXRX_NUM_WMM_AC];
		} reorder_timeout;
		adf_os_spinlock_t mutex;
	} rx;

	/* rx proc function */
	void (*rx_opt_proc)(
			struct ol_txrx_vdev_t *vdev,
			struct ol_txrx_peer_t *peer,
			unsigned tid,
			adf_nbuf_t msdu_list);

	/* rx mgmt function */
	VOS_STATUS (*rx_mgmt) (void *g_vosctx, void *buf);

	/* tx management delivery notification callback functions */
	struct {
		struct {
			ol_txrx_mgmt_tx_cb cb;
			void *ctxt;
		} callbacks[OL_TXRX_MGMT_NUM_TYPES];
	} tx_mgmt;

	/* tx descriptor pool */
	struct {
		int pool_size;
		union ol_tx_desc_list_elem_t *array;
		union ol_tx_desc_list_elem_t *freelist;
	} tx_desc;

	struct {
		int (*cmp)(
				union htt_rx_pn_t *new,
				union htt_rx_pn_t *old,
				int is_unicast,
				int opmode);
		int len;
	} rx_pn[htt_num_sec_types];

	/* tx mutex */
	OL_TX_MUTEX_TYPE tx_mutex;

	/*
	 * peer ref mutex:
	 * 1. Protect peer object lookups until the returned peer object's
	 *    reference count is incremented.
	 * 2. Provide mutex when accessing peer object lookup structures.
	 */
	OL_RX_MUTEX_TYPE peer_ref_mutex;

	/*
	 * last_real_peer_mutex:
	 * Protect lookups of any vdev's last_real_peer pointer until the
	 * reference count for the pointed-to peer object is incremented.
	 * This mutex could be in the vdev struct, but it's slightly simpler
	 * to have a single lock in the pdev struct.  Since the lock is only
	 * held for an extremely short time, and since it's very unlikely for
	 * two vdev's to concurrently access the lock, there's no real
	 * benefit to having a per-vdev lock.
	 */
	OL_RX_MUTEX_TYPE last_real_peer_mutex;


#if TXRX_STATS_LEVEL != TXRX_STATS_LEVEL_OFF
	struct {
		struct {
			struct {
				struct {
					u_int64_t ppdus;
					u_int64_t mpdus;
				} normal;
				struct {
					/*
					 * mpdu_bad is general -
					 * replace it with the specific counters below
					 */
					u_int64_t mpdu_bad;
					//u_int64_t mpdu_fcs;
					//u_int64_t mpdu_duplicate;
					//u_int64_t mpdu_pn_replay;
					//u_int64_t mpdu_bad_sender; /* peer not found */
					//u_int64_t mpdu_flushed;
					//u_int64_t msdu_defrag_mic_err;
				} err;
			} rx;
		} priv;
		struct ol_txrx_stats pub;
	} stats;
#endif /* TXRX_STATS_LEVEL */

#if defined(ENABLE_RX_REORDER_TRACE)
	struct {
		u_int32_t mask;
		u_int32_t idx;
		u_int64_t cnt;
#define TXRX_RX_REORDER_TRACE_SIZE_LOG2 8 /* 256 entries */
		struct {
			u_int16_t reorder_idx;
			u_int16_t seq_num;
			u_int8_t  num_mpdus;
			u_int8_t  tid;
		} *data;
	} rx_reorder_trace;
#endif /* ENABLE_RX_REORDER_TRACE */

#if defined(ENABLE_RX_PN_TRACE)
	struct {
		u_int32_t mask;
		u_int32_t idx;
		u_int64_t cnt;
#define TXRX_RX_PN_TRACE_SIZE_LOG2 5 /* 32 entries */
		struct {
			struct ol_txrx_peer_t *peer;
			u_int32_t pn32;
			u_int16_t seq_num;
			u_int8_t  unicast;
			u_int8_t  tid;
		} *data;
	} rx_pn_trace;
#endif /* ENABLE_RX_PN_TRACE */

#if defined(PERE_IP_HDR_ALIGNMENT_WAR)
	bool        host_80211_enable;
#endif

	/*
	 * tx_sched only applies for HL, but is defined unconditionally rather than
	 * only if defined(CONFIG_HL_SUPPORT).  This is because the struct only
	 * occupies a few bytes, and to avoid the complexity of wrapping references
	 * to the struct members in "defined(CONFIG_HL_SUPPORT)" conditional
	 * compilation.
	 * If this struct gets expanded to a non-trivial size, then it should be
	 * conditionally compiled to only apply if defined(CONFIG_HL_SUPPORT).
	 */
	adf_os_spinlock_t tx_queue_spinlock;
	struct {
		enum ol_tx_scheduler_status tx_sched_status;
		ol_tx_sched_handle scheduler;
	} tx_sched;
	/*
	 * tx_queue only applies for HL, but is defined unconditionally to avoid
	 * wrapping references to tx_queue in "defined(CONFIG_HL_SUPPORT)"
	 * conditional compilation.
	 */
	struct {
		adf_os_atomic_t rsrc_cnt;
		/* threshold_lo - when to start tx desc margin replenishment */
		int rsrc_threshold_lo;
		/* threshold_hi - where to stop during tx desc margin replenishment */
		int rsrc_threshold_hi;
	} tx_queue;

#if defined(ENABLE_TX_QUEUE_LOG) && defined(CONFIG_HL_SUPPORT)
#define OL_TXQ_LOG_SIZE 1024
	struct {
		int size;
		int oldest_record_offset;
		int offset;
		int allow_wrap;
		u_int32_t wrapped;
		u_int8_t data[OL_TXQ_LOG_SIZE]; /* aligned to u_int32_t boundary */
	} txq_log;
#endif
};

struct ol_txrx_vdev_t {
	/* pdev - the physical device that is the parent of this virtual device */
	struct ol_txrx_pdev_t *pdev;

	/* vdev_id - ID used to specify a particular vdev to the target */
	u_int8_t vdev_id;

	void *osif_dev;

	/* MAC address */
	union ol_txrx_align_mac_addr_t mac_addr;

	/* tx paused - NO LONGER NEEDED? */

	/* node in the pdev's list of vdevs */
	TAILQ_ENTRY(ol_txrx_vdev_t) vdev_list_elem;

	/* ol_txrx_peer list */
	TAILQ_HEAD(, ol_txrx_peer_t) peer_list;
	/* last real peer created for this vdev (not "self" pseudo-peer) */
	struct ol_txrx_peer_t *last_real_peer;

	/* transmit function used by this vdev */
	ol_txrx_tx_fp tx;

	/* receive function used by this vdev to hand rx frames to the OS shim */
	ol_txrx_rx_fp osif_rx;

	struct {
		/*
		 * If the vdev object couldn't be deleted immediately because it still
		 * had some peer objects left, remember that a delete was requested,
		 * so it can be deleted once all its peers have been deleted.
		 */
		int pending;
		/*
		 * Store a function pointer and a context argument to provide a
		 * notification for when the vdev is deleted.
		 */
		ol_txrx_vdev_delete_cb callback;
		void *context;
	} delete;

	/* safe mode control to bypass the encrypt and decipher process*/
	u_int32_t safemode;

	/* rx filter related */
	u_int32_t drop_unenc;
	privacy_exemption privacy_filters[MAX_PRIVACY_FILTERS];
	u_int32_t filters_num;

	enum wlan_op_mode opmode;

#if defined(CONFIG_HL_SUPPORT)
	struct ol_tx_frms_queue_t txqs[OL_TX_VDEV_NUM_QUEUES];
#endif
};

struct ol_rx_reorder_array_elem_t {
	adf_nbuf_t head;
	adf_nbuf_t tail;
};

struct ol_rx_reorder_t {
	u_int8_t win_sz;
	u_int8_t win_sz_mask;
	u_int8_t num_mpdus;
	struct ol_rx_reorder_array_elem_t *array;
	/* base - single rx reorder element used for non-aggr cases */
	struct ol_rx_reorder_array_elem_t base;
#if defined(QCA_SUPPORT_OL_RX_REORDER_TIMEOUT)
	struct ol_rx_reorder_timeout_list_elem_t timeout;
#endif
	/* only used for defrag right now */
	TAILQ_ENTRY(ol_rx_reorder_t) defrag_waitlist_elem;
	u_int32_t defrag_timeout_ms;
	/* get back to parent ol_txrx_peer_t when ol_rx_reorder_t is in a
	 * waitlist */
	u_int16_t tid;
};

enum {
    txrx_sec_mcast = 0,
    txrx_sec_ucast
};

struct ol_txrx_peer_t {
	struct ol_txrx_vdev_t *vdev;

	adf_os_atomic_t ref_cnt;

	/* peer ID(s) for this peer */
	u_int16_t peer_ids[MAX_NUM_PEER_ID_PER_PEER];

	union ol_txrx_align_mac_addr_t mac_addr;

	/* node in the vdev's list of peers */
	TAILQ_ENTRY(ol_txrx_peer_t) peer_list_elem;
	/* node in the hash table bin's list of peers */
	TAILQ_ENTRY(ol_txrx_peer_t) hash_list_elem;

	/*
	 * per TID info -
	 * stored in separate arrays to avoid alignment padding mem overhead
	 */
	struct ol_rx_reorder_t tids_rx_reorder[OL_TXRX_NUM_EXT_TIDS];
	union htt_rx_pn_t      tids_last_pn[OL_TXRX_NUM_EXT_TIDS];
	u_int8_t               tids_last_pn_valid[OL_TXRX_NUM_EXT_TIDS];
	u_int16_t              tids_next_rel_idx[OL_TXRX_NUM_EXT_TIDS];
	u_int16_t              tids_last_seq[OL_TXRX_NUM_EXT_TIDS];

	struct {
		enum htt_sec_type sec_type;
		u_int32_t michael_key[2]; /* relevant for TKIP */
	} security[2]; /* 0 -> multicast, 1 -> unicast */

	/*
	 * rx proc function: this either is a copy of pdev's rx_opt_proc for
	 * regular rx processing, or has been redirected to a /dev/null discard
	 * function when peer deletion is in progress.
	 */
	void (*rx_opt_proc)(struct ol_txrx_vdev_t *vdev,
			    struct ol_txrx_peer_t *peer,
			    unsigned tid,
			    adf_nbuf_t msdu_list);

#if defined(CONFIG_HL_SUPPORT)
	struct ol_tx_frms_queue_t txqs[OL_TX_NUM_TIDS];
#endif

	/* NAWDS Flag and Bss Peer bit */
	u_int8_t nawds_enabled:1,
		 bss_peer:1;

	/*qos info*/
	u_int8_t qos_capable;
};

#endif /* _OL_TXRX_TYPES__H_ */
