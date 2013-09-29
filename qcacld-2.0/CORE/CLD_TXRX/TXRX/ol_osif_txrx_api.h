/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_osif_txrx_api.h
 * @brief Define the OS specific API functions called by txrx SW.
 */
#ifndef _OL_OSIF_TXRX_API_H_
#define _OL_OSIF_TXRX_API_H_

#include <adf_nbuf.h>      /* adf_nbuf_t */

/**
 * @brief Call tx completion handler to release the buffers
 * @details
 * 
 * Invoke tx completion handler when the tx credit goes below low water mark.
 * This eliminate the packet drop in the host driver due to send routine not yielding 
 * the cpu when the amount of traffic pumped from the network layer is very high. 
 *
 * @param osdev
 */

void ol_osif_ath_tasklet(adf_os_device_t osdev);

#endif /* _OL_OSIF_TXRX_API_H_ */
