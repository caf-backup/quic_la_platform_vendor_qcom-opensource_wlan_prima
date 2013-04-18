/*
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_tx_classify.h
 * @brief API definitions for the tx classify module within the data SW.
 */
#ifndef _OL_TX_CLASSIFY__H_
#define _OL_TX_CLASSIFY__H_

#include <adf_nbuf.h>      /* adf_nbuf_t */
#include <ol_txrx_types.h> /* ol_txrx_vdev_t, etc. */

#if defined(CONFIG_HL_SUPPORT)

/**
 * @brief Classify a tx frame to which tid queue.
 *
 * @param vdev - the virtual device sending the data
 *      (for specifying the transmitter address for multicast / broadcast data)
 * @param tx_desc - descriptor object with meta-data about the tx frame
 * @param netbuf - the tx frame
 * @param tx_msdu_info - characteristics of the tx frame
 */
struct ol_tx_frms_queue_t *
ol_tx_classify(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t netbuf,
    struct ol_txrx_msdu_info_t *tx_msdu_info);

struct ol_tx_frms_queue_t *
ol_tx_classify_mgmt(
    struct ol_txrx_vdev_t *vdev, 
    struct ol_tx_desc_t *tx_desc, 
    adf_nbuf_t netbuf,
    struct ol_txrx_msdu_info_t *tx_msdu_info);

#else

#define ol_tx_classify(vdev, tx_desc, netbuf, tx_msdu_info) NULL
#define ol_tx_classify_mgmt(vdev, tx_desc, netbuf, tx_msdu_info) NULL

#endif /* defined(CONFIG_HL_SUPPORT) */


#endif /* _OL_TX_CLASSIFY__H_ */

