/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

/**========================================================================

  \file     packet.c
  \brief    Implementation of packet module

  ========================================================================*/
/**=========================================================================
  EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header:$   $DateTime: $ $Author: $


  when              who           what, where, why
  --------          ---           -----------------------------------------
  26/03/2013        Ganesh        Created module for Packet Handling
                    Babu
  ==========================================================================*/
#include "packet.h"
#include "adf_nbuf.h"
#include "vos_memory.h"

/**
 * voss_rx_packet_free  Free the Rx Packet
 * @ Rx Packet
 */
void voss_rx_packet_free(void* rxpacket)
{
	tp_rxpacket rx_packet = (tp_rxpacket)rxpacket;

	/* Free up the Adf nbuf */
	adf_nbuf_free(rx_packet->rx_nbuf);

	/* Free up the Rx packet */
	vos_mem_free(rx_packet);
}


