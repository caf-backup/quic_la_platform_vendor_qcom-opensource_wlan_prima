#if !defined( WLAN_HDD_TYPE_H )
#define WLAN_HDD_TYPE_H

/**===========================================================================
  
  \file  WLAN_HDD_TYPE_H.h
  
  \brief Linux HDD Adapter Type
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <vos_types.h>
#include <wlan_hdd_dp_utils.h>

/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 

/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/

typedef struct hdd_adapter_s
{
   //Global VOS context
   v_CONTEXT_t vosContext;

   //Handle to the network device
   struct net_device *dev;

   //Transmit queues for each AC (VO,VI,BE etc)
   hdd_list_t wmm_tx_queue[NUM_TX_QUEUES];

   //Track whether VOS is in a low resource state
   v_BOOL_t isVosOutOfResource;

   //Station ID
   v_U8_t staId;

   //Device TX/RX statistics
   struct net_device_stats stats;

} hdd_adapter_t;

/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 

#endif    // end #if !defined( WLAN_HDD_TYPE_H )
