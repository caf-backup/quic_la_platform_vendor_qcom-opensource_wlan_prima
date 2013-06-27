/*
 * Copyright (c) 2012-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

/*
 *  The file is used to define structures that are shared between
 *  kernel space and user space pktlog application.
 */

#ifndef _PKTLOG_AC_API_
#define _PKTLOG_AC_API_
#ifndef REMOVE_PKT_LOG

/**
 * @typedef ol_pktlog_dev_handle
 * @brief opaque handle for pktlog device object
 */
struct ol_pktlog_dev_t;
typedef struct ol_pktlog_dev_t* ol_pktlog_dev_handle;

/**
 * @typedef ol_softc_handle
 * @brief opaque handle for ol_softc
 */
struct ol_softc;
typedef struct ol_softc* ol_softc_handle;

/**
 * @typedef net_device_handle
 * @brief opaque handle linux phy device object
 */
struct net_device;
typedef struct net_device* net_device_handle;

void ol_pl_set_name(ol_softc_handle scn, net_device_handle dev);

void ol_pl_sethandle(ol_pktlog_dev_handle *pl_handle,
		     ol_softc_handle scn);

/* Packet log state information */
#ifndef _PKTLOG_INFO
#define _PKTLOG_INFO
struct ath_pktlog_info {
	struct ath_pktlog_buf *buf;
	u_int32_t log_state;
	u_int32_t saved_state;
	u_int32_t options;

	/* Size of buffer in bytes */
	int32_t buf_size;
	spinlock_t log_lock;

	/* Threshold of TCP SACK packets for triggered stop */
	int sack_thr;

	/* # of tail packets to log after triggered stop */
	int tail_length;

	/* throuput threshold in bytes for triggered stop */
	u_int32_t thruput_thresh;

	/* (aggregated or single) packet size in bytes */
	u_int32_t pktlen;

	/* a temporary variable for counting TX throughput only */
	/* PER threshold for triggered stop, 10 for 10%, range [1, 99] */
	u_int32_t per_thresh;

	/* Phyerr threshold for triggered stop */
	u_int32_t phyerr_thresh;

	/* time period for counting trigger parameters, in milisecond */
	u_int32_t trigger_interval;
	u_int32_t start_time_thruput;
	u_int32_t start_time_per;
};
#endif /* _PKTLOG_INFO */
#else  /* REMOVE_PKT_LOG */
typedef void* ol_pktlog_dev_handle;
#define ol_pl_sethandle(pl_handle, scn)	\
	do {				\
		(void)pl_handle;	\
		(void)scn;		\
	} while (0)

#define ol_pl_set_name(scn, dev)	\
	do {				\
		(void)scn;		\
		(void)dev;		\
	} while (0)

#endif /* REMOVE_PKT_LOG */
#endif  /* _PKTLOG_AC_API_ */
