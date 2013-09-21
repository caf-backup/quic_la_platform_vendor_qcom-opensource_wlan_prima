/*
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */
#ifndef _DMUX_DXE_INTERNAL__H_
#define _DMUX_DXE_INTERNAL__H_

#include <athdefs.h>     /* A_STATUS */
#include <adf_nbuf.h>    /* adf_nbuf_t */
#include <adf_os_types.h>    /* adf_os_device_t, adf_os_print */
#include <adf_os_atomic.h>

#include <hif_dxe.h>     /* E_HIFDXE_CHANNELTYPE */

struct dmux_dxe_pdev_t {
    adf_os_atomic_t  ref_count;  /* module reference count */
    hif_dxe_handle   h_hif_dxe;  
    dmux_dxe_data_cb rx_data_cb;
    dmux_dxe_mgmt_cb rx_mgmt_cb;
    dmux_dxe_ctrl_cb rx_ctrl_cb;
    dmux_dxe_msg_cb  msg_cb;     /* callback for HTT T2H message */
    void             *rx_data_context;
    void             *rx_mgmt_context;
    void             *rx_ctrl_context;
    void             *msg_context;
};

#endif /* _DMUX_DXE_INTERNAL__H_ */
