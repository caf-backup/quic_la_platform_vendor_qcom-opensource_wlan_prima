/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/**========================================================================

  \file     wma.c
  \brief    Implementation of WMA

  Copyright 2013 (c) Qualcomm Technologies, Inc.  All Rights Reserved.

  Qualcomm Technologies Confidential and Proprietary.

  ========================================================================*/
/**=========================================================================
  EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header:$   $DateTime: $ $Author: $


  when              who           what, where, why
  --------          ---           -----------------------------------------
  12/03/2013        Ganesh        Implementation of wma function for initialization
                    Kondabattini
  ==========================================================================*/
#ifndef WMA_API_H
#define WMA_API_H

#include "osdep.h"
#include "vos_mq.h"
#include "aniGlobal.h"
#include "a_types.h"
#include "wmi_unified.h"
#ifdef NOT_YET
#include "wlan_hdd_tgt_cfg.h"
#include "htc_api.h"
#endif
#include "limGlobal.h"

typedef v_VOID_t* WMA_HANDLE;

VOS_STATUS wma_open(v_VOID_t *vos_context, v_VOID_t *os_context,
		    tMacOpenParameters *mac_params);

#ifdef QCA_WIFI_ISOC
VOS_STATUS wma_nv_download_start(v_VOID_t *vos_context);
#endif

VOS_STATUS wma_pre_start(v_VOID_t *vos_context);

VOS_STATUS wma_mc_process_msg( v_VOID_t *vos_context, vos_msg_t *msg );

VOS_STATUS wma_start(v_VOID_t *vos_context);

VOS_STATUS wma_stop(v_VOID_t *vos_context, tANI_U8 reason);

VOS_STATUS wma_close(v_VOID_t *vos_context);

v_VOID_t wma_rx_ready_event(WMA_HANDLE handle, wmi_ready_event *ev);

v_VOID_t wma_rx_service_ready_event(WMA_HANDLE handle, 
        wmi_service_ready_event *ev);

v_VOID_t wma_setneedshutdown(v_VOID_t *vos_context);

v_BOOL_t wma_needshutdown(v_VOID_t *vos_context);

VOS_STATUS wma_wait_for_ready_event(WMA_HANDLE handle);

#ifndef QCA_WIFI_ISOC
int wma_suspend_target(WMA_HANDLE handle, int disable_target_intr);
int wma_resume_target(WMA_HANDLE handle);
#endif
int wma_set_peer_param(void *wma_ctx, u_int8_t *peer_addr, u_int32_t param_id,
		       u_int32_t param_value, u_int32_t vdev_id);
#ifdef NOT_YET
VOS_STATUS wma_update_channel_list(WMA_HANDLE handle, void *scan_chan_info);
#endif
#endif
