/**===========================================================================
  
  \file  wlan_hdd_tx_rx.c
  
  \brief Linux HDD Tx/RX APIs
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_qct_tl.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>


/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 

/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/ 
  
/*--------------------------------------------------------------------------- 
  Function definitions and documenation
  -------------------------------------------------------------------------*/ 

#ifdef DATA_PATH_UNIT_TEST
//Utility function to dump an sk_buff
static void dump_sk_buff(struct sk_buff * skb)
{
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: head = %p ", __FUNCTION__, skb->head);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: data = %p ", __FUNCTION__, skb->data);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: tail = %p ", __FUNCTION__, skb->tail);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: end = %p ", __FUNCTION__, skb->end);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: len = %d ", __FUNCTION__, skb->len);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: data_len = %d ", __FUNCTION__, skb->data_len);
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: mac_len = %d\n", __FUNCTION__, skb->mac_len);

  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", 
     skb->data[0], skb->data[1], skb->data[2], skb->data[3], skb->data[4], 
     skb->data[5], skb->data[6], skb->data[7]); 
  VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", 
     skb->data[8], skb->data[9], skb->data[10], skb->data[11], skb->data[12],
     skb->data[13], skb->data[14], skb->data[15]); 
}

//Function for Unit Test only
static void transport_thread(hdd_adapter_t *pAdapter)
{
   v_U8_t staId;
   WLANTL_ACEnumType ac = WLANTL_AC_BE;
   vos_pkt_t *pVosPacket = NULL ;
   vos_pkt_t dummyPacket;
   WLANTL_MetaInfoType pktMetaInfo;
   WLANTL_RxMetaInfoType pktRxMetaInfo;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;

   status = hdd_tx_fetch_packet_cbk( pAdapter->pvosContext,
                                     &staId,
                                     &ac,
                                     &pVosPacket,
                                     &pktMetaInfo );
  if (status != VOS_STATUS_SUCCESS && status != VOS_STATUS_E_EMPTY)
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test FAIL hdd_tx_fetch_packet_cbk", __FUNCTION__);
  else
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test PASS hdd_tx_fetch_packet_cbk", __FUNCTION__);

  status = hdd_tx_complete_cbk(pAdapter->pvosContext, &dummyPacket, VOS_STATUS_SUCCESS);; 
  if (status != VOS_STATUS_SUCCESS)
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test FAIL hdd_tx_complete_cbk", __FUNCTION__);
  else
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test PASS hdd_tx_complete_cbk", __FUNCTION__);

  status = hdd_tx_low_resource_cbk(pVosPacket, pAdapter);
  if (status != VOS_STATUS_SUCCESS)
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test FAIL hdd_tx_low_resource_cbk", __FUNCTION__);
  else
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test PASS hdd_tx_low_resource_cbk", __FUNCTION__);
  
  status = hdd_rx_packet_cbk( pAdapter->pvosContext,
                              &dummyPacket,
                              staId,
                              &pktRxMetaInfo);
  if (status != VOS_STATUS_SUCCESS)
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test FAIL hdd_rx_packet_cbk", __FUNCTION__);
  else
     VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Test PASS hdd_rx_packet_cbk", __FUNCTION__);

}
#endif


/**============================================================================
  @brief hdd_hard_start_xmit() - Function registered with the Linux OS for 
  transmitting packets. There are 2 versions of this function. One that uses
  locked queue and other that uses lockless queues. Both have been retained to
  do some performance testing

  @param skb      : [in]  pointer to OS packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
int hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
   VOS_STATUS status;
   WLANTL_ACEnumType ac;
   sme_QosWmmUpType up;
   skb_list_node_t *pktNode = NULL;
   hdd_list_node_t *anchor = NULL;
   v_SIZE_t pktListSize = 0;
   hdd_adapter_t* pAdapter = netdev_priv(dev);
   v_BOOL_t granted;

   if(pAdapter == NULL) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return -1;
   }
   
   if(skb->len < ETH_ZLEN)
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN, "%s: Ethernet packet size < 60 bytes" , __FUNCTION__);

   //Classify the packet
   if ( !hdd_wmm_classify_pkt (pAdapter, skb, &ac, &up) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "%s: Failed to classify packet..pkt dropped", __FUNCTION__);
      kfree_skb(skb);
      return 0;
   }

#ifdef HDD_WMM_DEBUG
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
              "%s: Classified as ac %d up %d", __FUNCTION__, ac, up);
#endif // HDD_WMM_DEBUG

   //If we have already reached the max queue size, disable the TX queue
   if ( pAdapter->wmm_tx_queue[ac].count == pAdapter->wmm_tx_queue[ac].max_size)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "%s: TX queue full for AC=%d Disable OS TX queue", __FUNCTION__, ac );
      netif_stop_queue(dev);
      netif_carrier_off(dev);
      return -1;
   }

   //Use the skb->cb field to hold the list node information
   pktNode = (skb_list_node_t *)&skb->cb;

   //Stick the OS packet inside this node.
   pktNode->skb = skb;

   //Stick the User Priority inside this node 
   pktNode->userPriority = up;


   INIT_LIST_HEAD(&pktNode->anchor);

   //Insert the OS packet into the appropriate AC queue
   spin_lock(&pAdapter->wmm_tx_queue[ac].lock);
   status = hdd_list_insert_back_size( &pAdapter->wmm_tx_queue[ac], &pktNode->anchor, &pktListSize );
   spin_unlock(&pAdapter->wmm_tx_queue[ac].lock);

   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s:Insert Tx queue failed. Pkt dropped", __FUNCTION__);
      kfree_skb(skb);
      return 0;
   }

   //Make sure we have been admitted to this access category
   if (likely(pAdapter->hddWmmStatus.wmmAcStatus[ac].wmmAcAccessGranted))
   {
      granted = VOS_TRUE;
   }
   else
   {
      status = hdd_wmm_acquire_access( pAdapter, ac, &granted );
   }

   if ( granted && ( pktListSize == 1 ))
   {
      //Let TL know we have a packet to send for this AC
      //VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s:Indicating Packet to TL", __FUNCTION__);
      status = WLANTL_STAPktPending( pAdapter->pvosContext, pAdapter->conn_info.staId[0], ac );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "%s: Failed to signal TL for AC=%d", __FUNCTION__, ac );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock(&pAdapter->wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pAdapter->wmm_tx_queue[ac], &anchor );
         spin_unlock(&pAdapter->wmm_tx_queue[ac].lock);
         netif_stop_queue(dev);
         netif_carrier_off(dev);
         kfree_skb(skb);
         return 0;
      }
   }

   dev->trans_start = jiffies;

   //FIXME Update statistics 

   return 0; 
}

/**============================================================================
  @brief hdd_tx_timeout() - Function called by OS if there is any
  timeout during transmission. Since HDD simply enqueues packet
  and returns control to OS right away, this would never be invoked

  @param dev : [in] pointer to Libra network device
  @return    : None
  ===========================================================================*/
void hdd_tx_timeout(struct net_device *dev)
{
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
      "%s: Transmission timeout occurred", __FUNCTION__);
   //Getting here implies we disabled the TX queues for too long. Queues are 
   //disabled either because of disassociation or low resource scenarios. In
   //case of disassociation it is ok to ignore this. But if associated, we have
   //do possible recovery here
} 


/**============================================================================
  @brief hdd_stats() - Function registered with the Linux OS for 
  device TX/RX statistic

  @param dev      : [in] pointer to Libra network device
  
  @return         : pointer to net_device_stats structure
  ===========================================================================*/
struct net_device_stats* hdd_stats(struct net_device *dev)
{
   hdd_adapter_t* priv = netdev_priv(dev);
   return &priv->stats;
}


/**============================================================================
  @brief hdd_init_tx_rx() - Init function to initialize Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_init_tx_rx( hdd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;

   pAdapter->isVosOutOfResource = VOS_FALSE;
   pAdapter->isTxSuspended = VOS_FALSE;

   vos_mem_zero(&pAdapter->stats, sizeof(struct net_device_stats));

   while (++i != NUM_TX_QUEUES) 
      hdd_list_init( &pAdapter->wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);

   return status;
}


/**============================================================================
  @brief hdd_deinit_tx_rx() - Deinit function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_deinit_tx_rx( hdd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;

   while (++i != NUM_TX_QUEUES) 
   {
      //Free up any packets in the Tx queue
      spin_lock_bh(&pAdapter->wmm_tx_queue[i].lock);
      while (true) 
      {
         status = hdd_list_remove_front( &pAdapter->wmm_tx_queue[i], &anchor );
         if(VOS_STATUS_E_EMPTY != status)
         {
            pktNode = list_entry(anchor, skb_list_node_t, anchor);
            skb = pktNode->skb;
            kfree(pktNode);
            kfree_skb(skb);
            continue;
         }
         break;
      }
      spin_unlock_bh(&pAdapter->wmm_tx_queue[i].lock);
      hdd_list_destroy( &pAdapter->wmm_tx_queue[i] );
   }

   return status;
}
/**============================================================================
  @brief hdd_IsEAPOLPacket() - Checks the packet is EAPOL or not.

  @param pVosPacket : [in] pointer to vos packet  
  @return         : VOS_TRUE if the packet is EAPOL 
                  : VOS_FALSE otherwise
  ===========================================================================*/

v_BOOL_t hdd_IsEAPOLPacket( vos_pkt_t *pVosPacket )
{
    VOS_STATUS vosStatus  = VOS_STATUS_SUCCESS;
    v_BOOL_t   fEAPOL     = VOS_FALSE; 
    void       *pBuffer   = NULL;

    
    vosStatus = vos_pkt_peek_data( pVosPacket, (v_SIZE_t)HDD_ETHERTYPE_802_1_X_FRAME_OFFSET,
                          &pBuffer, HDD_ETHERTYPE_802_1_X_SIZE );
	
    if (VOS_IS_STATUS_SUCCESS( vosStatus ) )
    {
       if ( vos_be16_to_cpu( *(unsigned short*)pBuffer ) == HDD_ETHERTYPE_802_1_X )
       {
          fEAPOL = VOS_TRUE;
       }
    }  
    
   return fEAPOL;
}



/**============================================================================
  @brief hdd_tx_complete_cbk() - Callback function invoked by TL
  to indicate that a packet has been transmitted across the SDIO bus
  succesfully. OS packet resources can be released after this cbk.

  @param vosContext   : [in] pointer to VOS context   
  @param pVosPacket   : [in] pointer to VOS packet (containing skb) 
  @param vosStatusIn  : [in] status of the transmission 

  @return             : VOS_STATUS_E_FAILURE if any errors encountered 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_tx_complete_cbk( v_VOID_t *vosContext, 
                                vos_pkt_t *pVosPacket, 
                                VOS_STATUS vosStatusIn )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_adapter_t *pAdapter = NULL;
   void* pOsPkt = NULL;
   
   if( ( NULL == vosContext ) || ( NULL == pVosPacket )  )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Null params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE; 
   }

   //Return the skb to the OS
   status = vos_pkt_get_os_packet( pVosPacket, &pOsPkt, VOS_TRUE );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      //This is bad but still try to free the VOSS resources if we can
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure extracting skb from vos pkt", __FUNCTION__);
      vos_pkt_return_packet( pVosPacket );
      return VOS_STATUS_E_FAILURE;
   }

   kfree_skb((struct sk_buff *)pOsPkt); 

   //Return the VOS packet resources.
   status = vos_pkt_return_packet( pVosPacket );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Could not return VOS packet to the pool", __FUNCTION__);
   }

   //Get the HDD context.
   pAdapter = (hdd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD, vosContext );
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }


   //FIXME Update packet statistics
   return status;
}


/**============================================================================
  @brief hdd_tx_fetch_packet_cbk() - Callback function invoked by TL to 
  fetch a packet for transmission.

  @param vosContext   : [in] pointer to VOS context  
  @param staId        : [in] Station for which TL is requesting a pkt
  @param pAc          : [in] pointer to access category requested by TL
  @param pVosPacket   : [out] pointer to VOS packet packet pointer
  @param pPktMetaInfo : [out] pointer to meta info for the pkt 
  
  @return             : VOS_STATUS_E_EMPTY if no packets to transmit
                      : VOS_STATUS_E_FAILURE if any errors encountered 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_tx_fetch_packet_cbk( v_VOID_t *vosContext,
                                    v_U8_t *pStaId,
                                    WLANTL_ACEnumType*  pAc,
                                    vos_pkt_t **ppVosPacket,
                                    WLANTL_MetaInfoType *pPktMetaInfo )
{
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   hdd_adapter_t *pAdapter = NULL;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;
   vos_pkt_t *pVosPacket = NULL;
   v_MACADDR_t* pDestMacAddress = NULL;
   v_TIME_t timestamp;
   WLANTL_ACEnumType ac;
   v_SIZE_t size = 0;

   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pStaId ) || 
        ( NULL == pAc ) ||
        ( NULL == ppVosPacket ) ||
        ( NULL == pPktMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Null Params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   ac = *pAc;
   *ppVosPacket = NULL;

   //Make sure the AC being asked for is sane
   if( ac > WLANTL_MAX_AC || ac < 0)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Invalid AC %d passed by TL", __FUNCTION__, ac);
      return VOS_STATUS_E_FAILURE;
   }

   //Get the HDD context.
   pAdapter = (hdd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD, vosContext );
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }
 
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: AC %d passed by TL", __FUNCTION__, ac);

   // loop until we find an AC with packets
   // or we determine we have no more packets to send
   while (1)
   {
      // has this AC been admitted?
      if (likely(pAdapter->hddWmmStatus.wmmAcStatus[ac].wmmAcAccessGranted))
      {
         // do we have any packets pending in this AC?
         hdd_list_size( &pAdapter->wmm_tx_queue[ac], &size ); 
         if( size >  0 )
         {
            // yes, so process it
#ifdef HDD_WMM_DEBUG
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "%s: AC %d has packets pending", __FUNCTION__, ac);
#endif // HDD_WMM_DEBUG
            break;
         }
      }

      // no so advance to next AC, wrapping around
      ac++;
      if (ac >= WLANTL_MAX_AC)
      {
         ac = 0;
      }

      // have we examined all ACs (wrapped back to the initial input AC)?
      if (ac == *pAc)
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: no packets pending", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }
   }

   //Get the vos packet. I don't want to dequeue and enqueue again if we are out of VOS resources 
   //This simplifies the locking and unlocking of Tx queue
   status = vos_pkt_wrap_data_packet( &pVosPacket, 
                                      VOS_PKT_TYPE_TX_802_3_DATA, 
                                      NULL, //OS Pkt is not being passed
                                      hdd_tx_low_resource_cbk, 
                                      pAdapter );

   if (status == VOS_STATUS_E_ALREADY || status == VOS_STATUS_E_RESOURCES)
   {
      //Remember VOS is in a low resource situation
      pAdapter->isVosOutOfResource = VOS_TRUE;
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: VOSS in Low Resource scenario", __FUNCTION__);
      //TL will now think we have no more packets in this AC
      return VOS_STATUS_E_FAILURE;
   }

   //Remove the packet from the queue
   spin_lock_bh(&pAdapter->wmm_tx_queue[ac].lock);
   status = hdd_list_remove_front( &pAdapter->wmm_tx_queue[ac], &anchor );
   spin_unlock_bh(&pAdapter->wmm_tx_queue[ac].lock);

   if(VOS_STATUS_SUCCESS == status)
   {
      //If success then we got a valid packet from some AC
      pktNode = list_entry(anchor, skb_list_node_t, anchor);
      skb = pktNode->skb;
      *pAc = ac;
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "%s: Error in de-queuing "
         "skb from Tx queue status = %d", __FUNCTION__, status );
      vos_pkt_return_packet(pVosPacket);
      return VOS_STATUS_E_FAILURE;
   }
      
   //Attach skb to VOS packet.
   status = vos_pkt_set_os_packet( pVosPacket, skb );
   if (status != VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Error atatching skb", __FUNCTION__);
      vos_pkt_return_packet(pVosPacket);
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Just being paranoid. To be removed later
   if(pVosPacket == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: VOS packet returned by VOSS is NULL", __FUNCTION__);
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Return VOS packet to TL;
   *ppVosPacket = pVosPacket;

   //Make sure stores are not reordered.
   wmb();
   
   //Fill out the meta information neede by TL
   //FIXME This timestamp is really the time stamp of wrap_data_packet
   vos_pkt_get_timestamp( pVosPacket, &timestamp );
   pPktMetaInfo->usTimeStamp = (v_U16_t)timestamp;
   
   if(pAdapter->conn_info.uIsAuthenticated == VOS_TRUE)
      pPktMetaInfo->ucIsEapol = 0;       
   else 
      pPktMetaInfo->ucIsEapol = hdd_IsEAPOLPacket( pVosPacket ) ? 1 : 0;
		
      	
   if ((HDD_WMM_USER_MODE_NO_QOS == pAdapter->cfg_ini->WmmMode) ||
       (!pAdapter->hddWmmStatus.wmmQap))
   {
      // either we don't want QoS or the AP doesn't support QoS
      pPktMetaInfo->ucUP = 0;
      pPktMetaInfo->ucTID = 0;
   }
   else
   {
      pPktMetaInfo->ucUP = pktNode->userPriority;
      pPktMetaInfo->ucTID = pPktMetaInfo->ucUP;
   }

   pPktMetaInfo->ucType = 0;          //FIXME Don't know what this is
   pPktMetaInfo->ucDisableFrmXtl = 0; //802.3 frame so we need to xlate

   //Extract the destination address from ethernet frame
   pDestMacAddress = (v_MACADDR_t*)skb->data;
   pPktMetaInfo->ucBcast = vos_is_macaddr_broadcast( pDestMacAddress ) ? 1 : 0;
   pPktMetaInfo->ucMcast = vos_is_macaddr_group( pDestMacAddress ) ? 1 : 0;

   //VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Valid VOS PKT returned to TL", __FUNCTION__);

   if ( netif_queue_stopped(pAdapter->dev) && size <= HDD_TX_QUEUE_LOW_WATER_MARK )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: TX queue re-enabled", __FUNCTION__);
      pAdapter->isTxSuspended = VOS_FALSE;
      netif_carrier_on(pAdapter->dev);
      netif_start_queue(pAdapter->dev);
   }    

   return status;
}


/**============================================================================
  @brief hdd_tx_low_resource_cbk() - Callback function invoked in the 
  case where VOS packets are not available at the time of the call to get 
  packets. This callback function is invoked by VOS when packets are 
  available.

  @param pVosPacket : [in]  pointer to VOS packet 
  @param userData   : [in]  opaque user data that was passed initially 
  
  @return           : VOS_STATUS_E_FAILURE if any errors encountered, 
                    : VOS_STATUS_SUCCESS otherwise
  =============================================================================*/
VOS_STATUS hdd_tx_low_resource_cbk( vos_pkt_t *pVosPacket, 
                                    v_VOID_t *userData )
{
   VOS_STATUS status;
   v_SINT_t i = 0;
   v_SIZE_t size = 0;
   hdd_adapter_t* pAdapter = (hdd_adapter_t *)userData;
   
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   //Return the packet to VOS. We just needed to know that VOS is out of low resource
   //situation. Here we will only signal TL that there is a pending data for a STA. 
   //VOS packet will be requested (if needed) when TL comes back to fetch data.
   vos_pkt_return_packet( pVosPacket );

   pAdapter->isVosOutOfResource = VOS_FALSE;

   //Indicate to TL that there is pending data if a queue is non empty
   for( i=NUM_TX_QUEUES-1; i>=0; --i )
   {
      size = 0;
      hdd_list_size( &pAdapter->wmm_tx_queue[i], &size );
      if ( size > 0 )
      {
         status = WLANTL_STAPktPending( pAdapter->pvosContext, 
                                        pAdapter->conn_info.staId [0], 
                                        (WLANTL_ACEnumType)i );
         if( !VOS_IS_STATUS_SUCCESS( status ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure in indicating pkt to TL for ac=%d", __FUNCTION__,i); 
         }
      }
   }

   return VOS_STATUS_SUCCESS;
}


/**============================================================================
  @brief hdd_rx_packet_cbk() - Receive callback registered with TL.
  TL will call this to notify the HDD when one or more packets were
  received for a registered STA.

  @param vosContext      : [in] pointer to VOS context  
  @param pVosPacketChain : [in] pointer to VOS packet chain
  @param staId           : [in] Station Id
  @param pRxMetaInfo     : [in] pointer to meta info for the received pkt(s) 

  @return                : VOS_STATUS_E_FAILURE if any errors encountered, 
                         : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_rx_packet_cbk( v_VOID_t *vosContext, 
                              vos_pkt_t *pVosPacketChain,
                              v_U8_t staId,
                              WLANTL_RxMetaInfoType* pRxMetaInfo )
{
   hdd_adapter_t *pAdapter = NULL;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   struct sk_buff *skb = NULL;
   vos_pkt_t* pVosPacket;
   vos_pkt_t* pNextVosPacket;

   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pVosPacketChain ) ||
        ( NULL == pRxMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Null params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   pAdapter = (hdd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD, vosContext );
   if ( NULL == pAdapter )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   // walk the chain until all are processed
   pVosPacket = pVosPacketChain;
   do
   {
      // get the pointer to the next packet in the chain
      // (but don't unlink the packet since we free the entire chain later)
      status = vos_pkt_walk_packet_chain( pVosPacket, &pNextVosPacket, VOS_FALSE);

      // both "success" and "empty" are acceptable results
      if (!((status == VOS_STATUS_SUCCESS) || (status == VOS_STATUS_E_EMPTY)))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure walking packet chain", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }

      // Extract the OS packet (skb).
      // Tell VOS to detach the OS packet from the VOS packet
      status = vos_pkt_get_os_packet( pVosPacket, (v_VOID_t **)&skb, VOS_TRUE );
      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure extracting skb from vos pkt", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }
   
      skb->dev = pAdapter->dev;
      skb->protocol = eth_type_trans(skb, skb->dev);
      skb->ip_summed = CHECKSUM_UNNECESSARY;
      netif_receive_skb(skb);

      // now process the next packet in the chain
      pVosPacket = pNextVosPacket;

   } while (pVosPacket);

   //Return the entire VOS packet chain to the resource pool
   status = vos_pkt_return_packet( pVosPacketChain );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"%s: Failure returning vos pkt", __FUNCTION__);
   }
   
   pAdapter->dev->last_rx = jiffies;

   //FIXME Update any packet statistics

   return status;   
}

