/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

/**========================================================================

  \file     hif_dxe_ospvt.h
  \brief    OS Private Layer used by Linux DXE OS Specific Module

  ========================================================================*/
/**=========================================================================
  EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header:$   $DateTime: $ $Author: $


  when              who           what, where, why
  --------          ---           -----------------------------------------
  05/03/2013        Ganesh        Created module for HIF Dxe Linux Specific
                    Babu
  ==========================================================================*/

#ifndef _HIF_DXE_PVTOS_H_
#define _HIF_DXE_PVTOS_H_
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include "athdefs.h"
#include "adf_os_types.h"
#include "adf_os_atomic.h"
#include <linux/wcnss_wlan.h>


#define DXE_READ_REG(ctx,addr)   hif_dxe_os_readreg(ctx,addr)
#define DXE_WRITE_REG(ctx,addr,val) hif_dxe_os_writereg(ctx,addr,val)


typedef struct _S_HIF_DXE_OSCONTEXT
{
	struct resource       *wcnss_memory;
	void __iomem          *mmio;

	/***************************************/
	/*        Tx Irq related Params                         */
	/***************************************/

	int   tx_irq;

	/* Whether Tx Irq is registered with the OS or not */
	unsigned char   bTxRegistered;

	/* Tasklet to do the Tx Processing */
	struct tasklet_struct hif_dxe_tx_tasklet;  


	/***************************************/
	/*        Rx Irq related Params                         */
	/***************************************/

	int   rx_irq;

	/* Whether Rx Irq is registered with the OS or not */
	unsigned char   bRxRegistered;

	/* Tasklet to do the Rx Processing */
	struct tasklet_struct hif_dxe_rx_tasklet;

	unsigned char      bTxIntEnabled , bRxIntEnabled; 
	S_HIF_DXE_OS_PARAMS dxe_hif_params;
	adf_os_atomic_t ref_count;

	/* Currently not used. Is it required ??? */
	unsigned long        dxe_unloaded;
}S_HIF_DXE_OSCONTEXT;
#endif

