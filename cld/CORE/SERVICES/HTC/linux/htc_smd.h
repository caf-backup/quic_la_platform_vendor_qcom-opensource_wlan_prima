/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

/**========================================================================

  \file     htc_smd.h
  \brief    htc smd module implmentation

  ========================================================================*/
/**=========================================================================
  EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header:$   $DateTime: $ $Author: $


  when              who           what, where, why
  --------          ---           -----------------------------------------
  01/03/2013        Ganesh        HTC SMD module implementation
                    Kondabattini
  ==========================================================================*/

#ifndef HTC_SMD_H
#define HTC_SMD_H
#ifdef CONFIG_ANDROID
#include <mach/msm_smd.h>
#include <linux/delay.h>
#else
#include "msm_smd.h"
#endif

#include "adf_nbuf.h"
#include "vos_list.h"
#include "htc_internal.h"
#include "htc_api.h"
#include "vos_api.h"
/* MACRO definitions */
#define HTC_SMD_CTRL_PORT             "WLAN_CTRL"
#define HTC_SMD_OPEN_TIMEOUT          500
#define HTC_CB_MAGIC                  0x53544357
typedef enum
{
	HTC_SMD_STATE_CLOSED,       /* Closed */
	HTC_SMD_STATE_OPEN_PENDING, /* Waiting for the OPEN event from SMD */
	HTC_SMD_STATE_OPEN,         /* Open event received from SMD */
	HTC_SMD_STATE_DEFERRED,     /* Write pending, SMD chennel is full */
	HTC_SMD_STATE_REM_CLOSED,   /* Remote end closed the SMD channel */
	HTC_SMD_STATE_MAX
} t_htc_smd_state;

typedef struct
{
	vos_list_node_t node;
	HTC_PACKET *htc_packet;
}t_htc_buffer;

struct s_htc_handle{
	t_htc_smd_state htc_smd_state;
	smd_channel_t* smd_channel;
	u32 htc_magic;
	t_htc_msg htc_open_msg;
	t_htc_msg htc_data_msg;
	struct completion htc_smd_event;

	/* COMMON for both ROME and PRONTO */
	HTC_ENDPOINT end_point[ENDPOINT_MAX];
	vos_list_t htc_write_pending_q;
	adf_os_device_t osdev;
	HTC_PACKET htc_packet;
};

typedef struct s_htc_handle t_htc_handle, *tp_htc_handle;

#define HTC_ENDPOINT_INIT    ENDPOINT_0
#define HTC_ENDPOINT_WMI     ENDPOINT_1
#define HTC_ENDPOINT_HTT     ENDPOINT_2

#endif
