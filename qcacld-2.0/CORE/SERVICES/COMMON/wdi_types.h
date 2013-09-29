/* AUTO-GENERATED FILE - DO NOT EDIT DIRECTLY */
/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 *   @addtogroup WDIAPI
 *@{
 */
/**
 * @file wdi_types.h
 * @brief Data type definitions used within the WDI API
 * @details
 *  The data type definitions shown below are purely for documentation
 *  reference.
 *  The actual data type definitions are obtained by including the individual
 *  API header files that define them.
 */
#ifndef _WDI_TYPES__H_
#define _WDI_TYPES__H_

#ifdef WDI_TYPES_DIRECT_DEFS

struct ol_pdev_t;
typedef struct ol_pdev_t* ol_pdev_handle;

struct ol_vdev_t;
typedef struct ol_vdev_t* ol_vdev_handle;

struct ol_peer_t;
typedef struct ol_peer_t* ol_peer_handle;

/**
 * @typedef ol_osif_vdev_handle
 * @brief opaque handle for OS shim virtual device object
 */
struct ol_osif_vdev_t;
typedef struct ol_osif_vdev_t* ol_osif_vdev_handle;

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

#else

/* obtain data type defs from individual API header files */

#include <ol_ctrl_api.h>
#include <ol_osif_api.h>
#include <ol_txrx_api.h>

#endif /* WDI_TYPES_DIRECT_DEFS */

#endif /* _WDI_TYPES__H_ */

/**@}*/
