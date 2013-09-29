/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
/**
 * @file ol_tx_send.h
 * @brief API definitions for the tx sendriptor module within the data SW.
 */
#ifndef _OL_TX_SEND__H_
#define _OL_TX_SEND__H_

#include <adf_nbuf.h>      /* adf_nbuf_t */
#include <ol_txrx_types.h> /* ol_tx_send_t */

#if defined(CONFIG_HL_SUPPORT)
#define ol_tx_discard_target_frms(pdev) /* no-op */
#else

/**
 * @flush the ol tx when surprise remove.
 *
 */
void
ol_tx_discard_target_frms(ol_txrx_pdev_handle pdev);
#endif

/**
 * @brief Send a tx frame to the target.
 * @details
 *
 * @param pdev -  the phy dev
 * @param vdev - the virtual device sending the data
 *      (for specifying the transmitter address for multicast / broadcast data)
 * @param netbuf - the tx frame
 */
void
ol_tx_send(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_desc_t *tx_desc,
    adf_nbuf_t msdu);

/**
 * @brief Send a tx batch download to the target.
 * @details
 *     This function is different from above in that
 *     it accepts a list of msdu's to be downloaded as a batch
 *
 * @param pdev -  the phy dev
 * @param msdu_list - the Head pointer to the Tx Batch
 * @param num_msdus - Total msdus chained in msdu_list
 */

int
ol_tx_send_batch(
    struct ol_txrx_pdev_t *pdev,
    adf_nbuf_t msdu_list, int num_msdus);

/**
 * @brief Send a tx frame with a non-std header or payload type to the target.
 * @details
 *
 * @param pdev -  the phy dev
 * @param vdev - the virtual device sending the data
 *      (for specifying the transmitter address for multicast / broadcast data)
 * @param netbuf - the tx frame
 * @param pkt_type - what kind of non-std frame is being sent
 */
void
ol_tx_send_nonstd(
    struct ol_txrx_pdev_t *pdev,
    struct ol_tx_desc_t *tx_desc,
    adf_nbuf_t msdu,
    enum htt_pkt_type pkt_type);
#endif /* _OL_TX_SEND__H_ */
