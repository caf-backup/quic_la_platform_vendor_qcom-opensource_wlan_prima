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
 * @brief Send a tx batch download to the target. This function is different from above in that
       It accepts a list of msdu's to be downloaded as a batch
 * @details
 *
 * @param pdev -  the phy dev
 * @param msdu_list - the Head pointer to the Tx Batch
 * @param num_msdus - Total msdus chained in msdu_list
 */

int
ol_tx_send_batch(
    struct ol_txrx_pdev_t *pdev,
    adf_nbuf_t msdu_list, int num_msdus);


#endif /* _OL_TX_SEND__H_ */
