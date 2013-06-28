/*
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_txrx_api.h
 * @brief Definitions used in multiple external interfaces to the txrx SW.
 */
#ifndef _OL_TXRX_API__H_
#define _OL_TXRX_API__H_

/**
 * @typedef ol_txrx_pdev_handle
 * @brief opaque handle for txrx physical device object
 */
struct ol_txrx_pdev_t;
typedef struct ol_txrx_pdev_t* ol_txrx_pdev_handle;

/**
 * @typedef ol_txrx_vdev_handle
 * @brief opaque handle for txrx virtual device object
 */
struct ol_txrx_vdev_t;
typedef struct ol_txrx_vdev_t* ol_txrx_vdev_handle;

/**
 * @typedef ol_txrx_peer_handle
 * @brief opaque handle for txrx peer object
 */
struct ol_txrx_peer_t;
typedef struct ol_txrx_peer_t* ol_txrx_peer_handle;

/**
 * @brief ADDBA negotiation status, used both during requests and confirmations
 */
enum ol_addba_status {
    /* status: negotiation started or completed successfully */
    ol_addba_success,

    /* reject: aggregation is not applicable - don't try again */
    ol_addba_reject,

    /* busy: ADDBA negotiation couldn't be performed - try again later */
    ol_addba_busy,
};

enum ol_sec_type {
    ol_sec_type_none,
    ol_sec_type_wep128,
    ol_sec_type_wep104,
    ol_sec_type_wep40,
    ol_sec_type_tkip,
    ol_sec_type_tkip_nomic,
    ol_sec_type_aes_ccmp,
    ol_sec_type_wapi,

    /* keep this last! */
    ol_sec_type_types
};

#endif /* _OL_TXRX_API__H_ */
