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

  Copyright 2013 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.

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
#include "htc_api.h"
#include "aniGlobal.h"
#include "wmi_unified.h"

typedef v_VOID_t* WMA_HANDLE;

VOS_STATUS wma_open(adf_os_device_t adf_dev, HTC_HANDLE htc_handle, 
		v_VOID_t *vos_context, tMacOpenParameters *mac_params);

VOS_STATUS wma_nv_download_start(WMA_HANDLE wma_handle);

VOS_STATUS wma_pre_start(WMA_HANDLE wma_handle);

VOS_STATUS wma_mc_process_msg( v_VOID_t *vos_context, vos_msg_t *msg );

VOS_STATUS wma_start(WMA_HANDLE wma_handle);

VOS_STATUS wma_stop(WMA_HANDLE wma_handle);

VOS_STATUS wma_close(WMA_HANDLE wma_handle);

v_VOID_t wma_rx_ready_event(WMA_HANDLE handle, wmi_ready_event *ev);

v_VOID_t wma_rx_service_ready_event(WMA_HANDLE handle, 
        wmi_service_ready_event *ev);

v_VOID_t wma_setneedshutdown(WMA_HANDLE handle);

v_BOOL_t wma_needshutdown(WMA_HANDLE handle);

VOS_STATUS wma_wait_for_ready_event(WMA_HANDLE handle);
#ifndef FEATURE_WLAN_INTEGRATED_SOC
int wma_suspend_target(WMA_HANDLE handle, int disable_target_intr);
int wma_resume_target(WMA_HANDLE handle);
#endif
#endif
