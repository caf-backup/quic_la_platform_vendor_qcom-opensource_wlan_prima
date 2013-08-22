/*
 * Copyright (c) 2013 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * LMAC offload interface functions for WMI TLV Interface
 */

#include "ol_if_athvar.h"
#include <adf_os_mem.h>   /* adf_os_mem_alloc,free, etc. */
#include <osdep.h>
#include "htc_api.h"
#include "wmi.h"
#include "wma.h"

// QCA Main host has dynamic memory allocation and should not define NO_DYNAMIC_MEM_ALLOC
//#define NO_DYNAMIC_MEM_ALLOC

/* Following macro definitions use OS or platform specific functions */
/* Following macro definitions use QCA MAIN windows host driver(applicable for Perigrene and its future platforms,
    Pronto and its future platforms) specific APIs */
  #define wmi_tlv_print_verbose WMA_LOGD
  #define wmi_tlv_print_error   adf_os_print
  #define wmi_tlv_OS_MEMCPY     OS_MEMCPY
  #define wmi_tlv_OS_MEMZERO    OS_MEMZERO
  #define wmi_tlv_OS_MEMMOVE    OS_MEMMOVE

#ifndef NO_DYNAMIC_MEM_ALLOC
  #define wmi_tlv_os_mem_alloc(scn, ptr, numBytes) \
      { \
              (ptr) = OS_MALLOC(NULL, (numBytes), GFP_ATOMIC); \
      }
  #define wmi_tlv_os_mem_free   adf_os_mem_free
#endif

