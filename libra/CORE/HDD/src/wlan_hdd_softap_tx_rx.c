
/**===========================================================================
  
  \file  wlan_hdd_softap_tx_rx.c
  
  \brief Linux HDD Tx/RX APIs
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
#ifdef WLAN_SOFTAP_FEATURE

/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_qct_tl.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
//#include <vos_list.h>
#include <vos_types.h>
#include <aniGlobal.h>
#include <halTypes.h>
#include <halHddApis.h>


/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 

/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/ 
 extern v_U8_t hddWmmUpToAcMap[]; 
/*--------------------------------------------------------------------------- 
  Function definitions and documenation
  -------------------------------------------------------------------------*/ 
#if 0
static void hdd_softap_dump_sk_buff(struct sk_buff * skb)
{
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: head = %p ", __FUNCTION__, skb->head);
  //VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: data = %p ", __FUNCTION__, skb->data);
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: tail = %p ", __FUNCTION__, skb->tail);
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: end = %p ", __FUNCTION__, skb->end);
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: len = %d ", __FUNCTION__, skb->len);
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: data_len = %d ", __FUNCTION__, skb->data_len);
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: mac_len = %d\n", __FUNCTION__, skb->mac_len);

  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ", 
     skb->data[0], skb->data[1], skb->data[2], skb->data[3], skb->data[4], 
     skb->data[5], skb->data[6], skb->data[7]); 
  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", 
     skb->data[8], skb->data[9], skb->data[10], skb->data[11], skb->data[12],
     skb->data[13], skb->data[14], skb->data[15]); 
}
#endif
/**============================================================================
  @brief hdd_softap_flush_tx_queues() - Utility function to flush the TX queues

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
static VOS_STATUS hdd_softap_flush_tx_queues( hdd_hostapd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;
   v_U8_t STAId = 0;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;

   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      if (FALSE == pAdapter->aStaInfo[STAId].isUsed)
      {
         continue;
      }

      for (i = 0; i < NUM_TX_QUEUES; i ++)
      {
         spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i].lock);
         while (true) 
         {
            status = hdd_list_remove_front ( &pAdapter->aStaInfo[STAId].wmm_tx_queue[i], &anchor);

            if (VOS_STATUS_E_EMPTY != status)
            {
               //If success then we got a valid packet from some AC
               pktNode = list_entry(anchor, skb_list_node_t, anchor);
               skb = pktNode->skb;
               ++pAdapter->stats.tx_dropped;
               ++pAdapter->hdd_stats.hddTxRxStats.txFlushed;
               ++pAdapter->hdd_stats.hddTxRxStats.txFlushedAC[i];
               kfree_skb(skb);
               continue;
            }

            //current list is empty
            break;
         }
         spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i].lock);
      }
   }

   // backpressure is no longer in effect
   pAdapter->isTxSuspended = VOS_FALSE;

   return status;
}


/**============================================================================
  @brief hdd_softap_hard_start_xmit() - Function registered with the Linux OS for 
  transmitting packets. There are 2 versions of this function. One that uses
  locked queue and other that uses lockless queues. Both have been retained to
  do some performance testing

  @param skb      : [in]  pointer to OS packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
int hdd_softap_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
   VOS_STATUS status;
   WLANTL_ACEnumType ac = WLANTL_AC_BE;
   sme_QosWmmUpType up = SME_QOS_WMM_UP_BE;
   skb_list_node_t *pktNode = NULL;
   v_SIZE_t pktListSize = 0;
   hdd_hostapd_adapter_t *pAdapter = (hdd_hostapd_adapter_t *)netdev_priv(dev);
   hdd_adapter_t *pStaAdapter = (hdd_adapter_t *)netdev_priv(pAdapter->pWlanDev);

   vos_list_node_t *anchor = NULL;
   /* For MAC address hashing purpose */
   tpAniSirGlobal  pMac = (tpAniSirGlobal) vos_get_context(VOS_MODULE_ID_HAL, pAdapter->pvosContext);
   v_U8_t STAId = WLAN_MAX_STA_COUNT;
   //Extract the destination address from ethernet frame
   v_MACADDR_t *pDestMacAddress = (v_MACADDR_t*)skb->data;

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
              "%s: enter\n", __FUNCTION__);

   if (vos_is_macaddr_broadcast( pDestMacAddress ) || vos_is_macaddr_group(pDestMacAddress))
   {
      //The BC/MC station ID is assigned during BSS starting phase. SAP will return the station 
      //ID used for BC/MC traffic. The station id is registered to TL as well.
      STAId = pAdapter->uBCStaId;
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO_LOW,
              "%s: BC/MC packet\n", __FUNCTION__);
   }
   else
   {
      if (eHAL_STATUS_SUCCESS != halTable_FindStaidByAddr(pMac, (tANI_U8 *)pDestMacAddress, &STAId))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to find right station", __FUNCTION__);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         return NETDEV_TX_OK;
      }

      if (FALSE == vos_is_macaddr_equal(&pAdapter->aStaInfo[STAId].macAddrSTA, pDestMacAddress))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                    "%s: Station MAC address does not matching", __FUNCTION__);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         return NETDEV_TX_OK;
      }

      if ( (WLANTL_STA_CONNECTED != pAdapter->aStaInfo[STAId].tlSTAState) && 
           (WLANTL_STA_AUTHENTICATED != pAdapter->aStaInfo[STAId].tlSTAState) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                    "%s: Station not connected yet", __FUNCTION__);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         kfree_skb(skb);
         return NETDEV_TX_OK;
      }
   }
   //Classify the packet
   if ( (HDD_WMM_USER_MODE_NO_QOS != pStaAdapter->cfg_ini->WmmMode) &&
         !hdd_wmm_classify_pkt (pStaAdapter, skb, &ac, &up) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to classify packet..pkt dropped", __FUNCTION__);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
      kfree_skb(skb);
      return NETDEV_TX_OK;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO,
              "%s: Classified as ac %d up %d", __FUNCTION__, ac, up);

#ifdef WLAN_SOFTAP_FEATURE
   // If the memory differentiation mode is enabled, the memory limit of each queue will be 
   // checked. Over-limit packets will be dropped.
    hdd_list_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &pktListSize);
    if(pktListSize >= pAdapter->aTxQueueLimit[ac])
    {
       VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
            "%s: station %d ac %d queue over limit %d \n", __FUNCTION__, STAId, ac, pktListSize); 
       pAdapter->aStaInfo[STAId].txSuspended = VOS_TRUE;
       pAdapter->aStaInfo[STAId].txSuspendedAc = ac; 
       netif_stop_queue(dev);
       //netif_carrier_off(dev);  // don't turn off carrier in softAP
               
       return NETDEV_TX_BUSY;
    }
#else
   //If we have already reached the max queue size, disable the TX queue
   if ( pAdapter->wmm_tx_queue[ac].count == pAdapter->wmm_tx_queue[ac].max_size)
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitBackPressured;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitBackPressuredAC[ac];

#define BACKPRESSURE_FULL_QUEUE
#ifdef BACKPRESSURE_FULL_QUEUE
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, 
                 "%s: TX queue full for AC=%d Disable OS TX queue", 
                 __FUNCTION__, ac );

      netif_stop_queue(dev);
      netif_carrier_off(dev);
      pAdapter->isTxSuspended = VOS_TRUE;
      pAdapter->txSuspendedAc = ac;
      return NETDEV_TX_BUSY;
#else //DROP_FULL_QUEUE
      kfree_skb(skb);
      return NETDEV_TX_OK;
#endif

   }
#endif //WLAN_SOFTAP_FEATURE

   //Use the skb->cb field to hold the list node information
   pktNode = (skb_list_node_t *)&skb->cb;

   //Stick the OS packet inside this node.
   pktNode->skb = skb;

   //Stick the User Priority inside this node 
   pktNode->userPriority = up;

   INIT_LIST_HEAD(&pktNode->anchor);

   spin_lock(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   status = hdd_list_insert_back_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &pktNode->anchor, &pktListSize );
   spin_unlock(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);

   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s:Insert Tx queue failed. Pkt dropped", __FUNCTION__);
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
      ++pAdapter->stats.tx_dropped;
      kfree_skb(skb);
      return NETDEV_TX_OK;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueued;
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueuedAC[ac];

#ifdef WLAN_SOFTAP_FEATURE
   if (1 == pktListSize)
   {
      //Let TL know we have a packet to send for this AC
      //VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s:Indicating Packet to TL", __FUNCTION__);
      status = WLANTL_STAPktPending( pAdapter->pvosContext, STAId, ac );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Failed to signal TL for AC=%d STAId =%d", 
                    __FUNCTION__, ac, STAId );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &anchor);
         spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
         kfree_skb(skb);
         return NETDEV_TX_OK;
      }
   }
#else
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
      //VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s:Indicating Packet to TL", __FUNCTION__);
      status = WLANTL_STAPktPending( pAdapter->pvosContext, pAdapter->conn_info.staId[0], ac );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Failed to signal TL for AC=%d", __FUNCTION__, ac );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock(&pAdapter->wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pAdapter->wmm_tx_queue[ac], &anchor );
         spin_unlock(&pAdapter->wmm_tx_queue[ac].lock);
         netif_stop_queue(dev);
         netif_carrier_off(dev);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
         kfree_skb(skb);
         return NETDEV_TX_OK;
      }
   }
#endif //WLAN_SOFTAP_FEATURE
   dev->trans_start = jiffies;

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO_LOW, "%s: exit \n", __FUNCTION__);

   return NETDEV_TX_OK;
}

/**============================================================================
  @brief hdd_softap_sta_2_sta_xmit This function for Transmitting the frames when the traffic is between two stations.

  @param skb      : [in] pointer to packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  @param STAId    : [in] Station Id of Destination Station
  @param up       : [in] User Priority 
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
VOS_STATUS hdd_softap_sta_2_sta_xmit(struct sk_buff *skb, 
                                      struct net_device *dev,
                                      v_U8_t STAId, 
                                      v_U8_t up)
{
   VOS_STATUS status;
   skb_list_node_t *pktNode = NULL;
   v_SIZE_t pktListSize = 0;
   hdd_hostapd_adapter_t *pAdapter = (hdd_hostapd_adapter_t *)netdev_priv(dev);
   v_U8_t ac;
   vos_list_node_t *anchor = NULL;

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitCalled;

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
              "%s: enter\n", __FUNCTION__);
   ac = hddWmmUpToAcMap[up];
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitClassifiedAC[ac];
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO,
              "%s: Classified as ac %d up %d", __FUNCTION__, ac, up);

   // If the memory differentiation mode is enabled, the memory limit of each queue will be 
   // checked. Over-limit packets will be dropped.
    hdd_list_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &pktListSize);
    if(pktListSize >= pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].max_size)
    {
       VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
            "%s: station %d ac %d queue over limit %d \n", __FUNCTION__, STAId, ac, pktListSize); 
       pAdapter->aStaInfo[STAId].txSuspended = VOS_TRUE;
       pAdapter->aStaInfo[STAId].txSuspendedAc = ac; 
       /* TODO:Rx Flowchart should be trigerred here to SUPEND SSC on RX side.
        * SUSPEND should be done based on Threshold. RESUME would be 
        * triggered in fetch cbk after recovery.
        */
       kfree_skb(skb);
       
       return VOS_STATUS_E_FAILURE;
    }


   //Use the skb->cb field to hold the list node information
   pktNode = (skb_list_node_t *)&skb->cb;

   //Stick the OS packet inside this node.
   pktNode->skb = skb;

   //Stick the User Priority inside this node 
   pktNode->userPriority = up;

   INIT_LIST_HEAD(&pktNode->anchor);

   spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   status = hdd_list_insert_back_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &pktNode->anchor, &pktListSize );
   spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);

   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s:Insert Tx queue failed. Pkt dropped", __FUNCTION__);
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
      ++pAdapter->stats.tx_dropped;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueued;
   ++pAdapter->hdd_stats.hddTxRxStats.txXmitQueuedAC[ac];

   if (1 == pktListSize)
   {
      //Let TL know we have a packet to send for this AC
      //VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s:Indicating Packet to TL", __FUNCTION__);
      status = WLANTL_STAPktPending( pAdapter->pvosContext, STAId, ac );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Failed to signal TL for AC=%d STAId =%d", 
                    __FUNCTION__, ac, STAId );

         //Remove the packet from queue. It must be at the back of the queue, as TX thread cannot preempt us in the middle
         //as we are in a soft irq context. Also it must be the same packet that we just allocated.
         spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         status = hdd_list_remove_back( &pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &anchor);
         spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
         ++pAdapter->stats.tx_dropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDropped;
         ++pAdapter->hdd_stats.hddTxRxStats.txXmitDroppedAC[ac];
         kfree_skb(skb);
         return VOS_STATUS_E_FAILURE;
      }
   }

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO_LOW, "%s: exit \n", __FUNCTION__);

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_softap_tx_timeout() - Function called by OS if there is any
  timeout during transmission. Since HDD simply enqueues packet
  and returns control to OS right away, this would never be invoked

  @param dev : [in] pointer to Libra network device
  @return    : None
  ===========================================================================*/
void hdd_softap_tx_timeout(struct net_device *dev)
{
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
      "%s: Transmission timeout occurred", __FUNCTION__);
   //Getting here implies we disabled the TX queues for too long. Queues are 
   //disabled either because of disassociation or low resource scenarios. In
   //case of disassociation it is ok to ignore this. But if associated, we have
   //do possible recovery here
} 


/**============================================================================
  @brief hdd_softap_stats() - Function registered with the Linux OS for 
  device TX/RX statistic

  @param dev      : [in] pointer to Libra network device
  
  @return         : pointer to net_device_stats structure
  ===========================================================================*/
struct net_device_stats* hdd_softap_stats(struct net_device *dev)
{
   hdd_hostapd_adapter_t* priv = netdev_priv(dev);
   return &priv->stats;
}


/**============================================================================
  @brief hdd_softap_init_tx_rx() - Init function to initialize Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_init_tx_rx( hdd_hostapd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_SINT_t i = -1;
   v_SIZE_t size = 0;

   v_U8_t STAId = 0;

   pAdapter->isVosOutOfResource = VOS_FALSE;
   pAdapter->isTxSuspended = VOS_FALSE;

   vos_mem_zero(&pAdapter->stats, sizeof(struct net_device_stats));

   while (++i != NUM_TX_QUEUES) 
      hdd_list_init( &pAdapter->wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);

   /* Initial HDD buffer control / flow control fields*/
   vos_pkt_get_available_buffer_pool (VOS_PKT_TYPE_TX_802_3_DATA, &size);

   pAdapter->aTxQueueLimit[WLANTL_AC_BK] = HDD_SOFTAP_TX_BK_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_BE] = HDD_SOFTAP_TX_BE_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_VI] = HDD_SOFTAP_TX_VI_QUEUE_MAX_LEN;
   pAdapter->aTxQueueLimit[WLANTL_AC_VO] = HDD_SOFTAP_TX_VO_QUEUE_MAX_LEN;

   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      vos_mem_zero(&pAdapter->aStaInfo[STAId], sizeof(hdd_station_info_t));
      for (i = 0; i < NUM_TX_QUEUES; i ++)
      {
         hdd_list_init(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);
      }
   }

   return status;
}

/**============================================================================
  @brief hdd_softap_deinit_tx_rx() - Deinit function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_deinit_tx_rx( hdd_hostapd_adapter_t *pAdapter )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   status = hdd_softap_flush_tx_queues(pAdapter);

   return status;
}

/**============================================================================
  @brief hdd_softap_flush_tx_queues_sta() - Utility function to flush the TX queues of a station

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit 
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
static VOS_STATUS hdd_softap_flush_tx_queues_sta( hdd_hostapd_adapter_t *pAdapter, v_U8_t STAId )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   v_U8_t i = -1;

   hdd_list_node_t *anchor = NULL;

   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;

   if (FALSE == pAdapter->aStaInfo[STAId].isUsed)
   {
      return status;
   }

   for (i = 0; i < NUM_TX_QUEUES; i ++)
   {
      spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i].lock);
      while (true) 
      {
         status = hdd_list_remove_front ( &pAdapter->aStaInfo[STAId].wmm_tx_queue[i], &anchor);
         if (VOS_STATUS_E_EMPTY != status)
         {
            //If success then we got a valid packet from some AC
            pktNode = list_entry(anchor, skb_list_node_t, anchor);
            skb = pktNode->skb;
            ++pAdapter->stats.tx_dropped;
            ++pAdapter->hdd_stats.hddTxRxStats.txFlushed;
            ++pAdapter->hdd_stats.hddTxRxStats.txFlushedAC[i];
            kfree_skb(skb);
            continue;
         }

         //current list is empty
         break;
      }
      spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i].lock);
   }

   return status;
}

/**============================================================================
  @brief hdd_softap_init_tx_rx_sta() - Init function to initialize a station in Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit
  @param pmacAddrSTA  : [in] pointer to the MAC address of the station  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_init_tx_rx_sta( hdd_hostapd_adapter_t *pAdapter, v_U8_t STAId, v_MACADDR_t *pmacAddrSTA)
{
   v_U8_t i = 0;
   if (pAdapter->aStaInfo[STAId].isUsed)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Reinit station %d", __FUNCTION__, STAId );
      return VOS_STATUS_E_FAILURE;
   }

   vos_mem_zero(&pAdapter->aStaInfo[STAId], sizeof(hdd_station_info_t));
   for (i = 0; i < NUM_TX_QUEUES; i ++)
   {
      hdd_list_init(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i], HDD_TX_QUEUE_MAX_LEN);
   }

   pAdapter->aStaInfo[STAId].isUsed = TRUE;
   vos_copy_macaddr( &pAdapter->aStaInfo[STAId].macAddrSTA, pmacAddrSTA);

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_softap_deinit_tx_rx_sta() - Deinit function to clean up a statioin in Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context
  @param STAId    : [in] Station ID to deinit 
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_deinit_tx_rx_sta ( hdd_hostapd_adapter_t *pAdapter, v_U8_t STAId )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   if (FALSE == pAdapter->aStaInfo[STAId].isUsed)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Deinit station not inited %d", __FUNCTION__, STAId );
      return VOS_STATUS_E_FAILURE;
   }

   status = hdd_softap_flush_tx_queues_sta(pAdapter, STAId);
   vos_mem_zero(&pAdapter->aStaInfo[STAId], sizeof(hdd_station_info_t));

   return status;
}

/**============================================================================
  @brief hdd_softap_disconnect_tx_rx() - Disconnect function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_disconnect_tx_rx( hdd_hostapd_adapter_t *pAdapter )
{
   return hdd_softap_flush_tx_queues(pAdapter);
}

/**============================================================================
  @brief hdd_softap_tx_complete_cbk() - Callback function invoked by TL
  to indicate that a packet has been transmitted across the SDIO bus
  succesfully. OS packet resources can be released after this cbk.

  @param vosContext   : [in] pointer to VOS context   
  @param pVosPacket   : [in] pointer to VOS packet (containing skb) 
  @param vosStatusIn  : [in] status of the transmission 

  @return             : VOS_STATUS_E_FAILURE if any errors encountered 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_tx_complete_cbk( v_VOID_t *vosContext, 
                                vos_pkt_t *pVosPacket, 
                                VOS_STATUS vosStatusIn )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_hostapd_adapter_t *pAdapter = NULL;
   void* pOsPkt = NULL;
   
   if( ( NULL == vosContext ) || ( NULL == pVosPacket )  )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Null params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE; 
   }

   //Return the skb to the OS
   status = vos_pkt_get_os_packet( pVosPacket, &pOsPkt, VOS_TRUE );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      //This is bad but still try to free the VOSS resources if we can
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Failure extracting skb from vos pkt", __FUNCTION__);
      vos_pkt_return_packet( pVosPacket );
      return VOS_STATUS_E_FAILURE;
   }

   //Get the HDD context.
   pAdapter = (hdd_hostapd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD_SOFTAP, vosContext );
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
   }
   else
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txCompleted;
   }

   kfree_skb((struct sk_buff *)pOsPkt); 

   //Return the VOS packet resources.
   status = vos_pkt_return_packet( pVosPacket );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Could not return VOS packet to the pool", __FUNCTION__);
   }

   return status;
}


/**============================================================================
  @brief hdd_softap_tx_fetch_packet_cbk() - Callback function invoked by TL to 
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
VOS_STATUS hdd_softap_tx_fetch_packet_cbk( v_VOID_t *vosContext,
                                    v_U8_t *pStaId,
                                    WLANTL_ACEnumType*  pAc,
                                    vos_pkt_t **ppVosPacket,
                                    WLANTL_MetaInfoType *pPktMetaInfo )
{
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   hdd_hostapd_adapter_t *pAdapter = NULL;
   hdd_list_node_t *anchor = NULL;
   skb_list_node_t *pktNode = NULL;
   struct sk_buff *skb = NULL;
   vos_pkt_t *pVosPacket = NULL;
   v_MACADDR_t* pDestMacAddress = NULL;
   v_TIME_t timestamp;
   WLANTL_ACEnumType ac;
   v_SIZE_t size = 0;
   v_U8_t STAId = WLAN_MAX_STA_COUNT;

   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pStaId ) || 
        ( NULL == pAc ) ||
        ( NULL == ppVosPacket ) ||
        ( NULL == pPktMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Null Params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   //Get the HDD context.
   pAdapter = (hdd_hostapd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD_SOFTAP, vosContext );
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }
 
   ++pAdapter->hdd_stats.hddTxRxStats.txFetched;

   STAId = *pStaId;
   if (STAId >= WLAN_MAX_STA_COUNT)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Invalid STAId %d passed by TL", __FUNCTION__, STAId);
      return VOS_STATUS_E_FAILURE;
   }

   if (FALSE == pAdapter->aStaInfo[STAId].isUsed )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Access unregistered STA by TL", __FUNCTION__, STAId);
      return VOS_STATUS_E_FAILURE;
   }

   ac = *pAc;
   *ppVosPacket = NULL;

   //Make sure the AC being asked for is sane
   if( ac > WLANTL_MAX_AC || ac < 0)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Invalid AC %d passed by TL", __FUNCTION__, ac);
      return VOS_STATUS_E_FAILURE;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.txFetchedAC[ac];

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: AC %d passed by TL", __FUNCTION__, ac);

   /* Only fetch this station and this AC. Return VOS_STATUS_E_EMPTY if nothing there. Do not get next AC
      as the other branch does.
   */
   hdd_list_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &size);
   if (0 == size)
   {
      return VOS_STATUS_E_EMPTY;
   }

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                       "%s: AC %d has packets pending", __FUNCTION__, ac);
   
   //Get the vos packet. I don't want to dequeue and enqueue again if we are out of VOS resources 
   //This simplifies the locking and unlocking of Tx queue
   status = vos_pkt_wrap_data_packet( &pVosPacket, 
                                      VOS_PKT_TYPE_TX_802_3_DATA, 
                                      NULL, //OS Pkt is not being passed
                                      hdd_softap_tx_low_resource_cbk, 
                                      pAdapter );

   if (status == VOS_STATUS_E_ALREADY || status == VOS_STATUS_E_RESOURCES)
   {
      //Remember VOS is in a low resource situation
      pAdapter->isVosOutOfResource = VOS_TRUE;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchLowResources;
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: VOSS in Low Resource scenario", __FUNCTION__);
      //TL needs to handle this case. VOS_STATUS_E_EMPTY is returned when the queue is empty.
      return VOS_STATUS_E_FAILURE;
   }

   spin_lock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);
   status = hdd_list_remove_front( &pAdapter->aStaInfo[STAId].wmm_tx_queue[ac], &anchor );
   spin_unlock_bh(&pAdapter->aStaInfo[STAId].wmm_tx_queue[ac].lock);

   if(VOS_STATUS_SUCCESS == status)
   {
      //If success then we got a valid packet from some AC
      pktNode = list_entry(anchor, skb_list_node_t, anchor);
      skb = pktNode->skb;
      *pAc = ac;
   }
   else
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Error in de-queuing "
         "skb from Tx queue status = %d", __FUNCTION__, status );
      vos_pkt_return_packet(pVosPacket);
      return VOS_STATUS_E_FAILURE;
   }

   //Attach skb to VOS packet.
   status = vos_pkt_set_os_packet( pVosPacket, skb );
   if (status != VOS_STATUS_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Error atatching skb", __FUNCTION__);
      vos_pkt_return_packet(pVosPacket);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Just being paranoid. To be removed later
   if(pVosPacket == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: VOS packet returned by VOSS is NULL", __FUNCTION__);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }

   //Return VOS packet to TL;
   *ppVosPacket = pVosPacket;

   //Fill out the meta information needed by TL
   //FIXME This timestamp is really the time stamp of wrap_data_packet
   vos_pkt_get_timestamp( pVosPacket, &timestamp );
   pPktMetaInfo->usTimeStamp = (v_U16_t)timestamp;

   pPktMetaInfo->ucIsEapol = 0;

   if(pAdapter->aStaInfo[STAId].tlSTAState != WLANTL_STA_AUTHENTICATED) 
   {
      if (TRUE == hdd_IsEAPOLPacket( pVosPacket ))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO_HIGH,
                    "%s: VOS packet is EAPOL packet", __FUNCTION__);
         pPktMetaInfo->ucIsEapol = 1;
      }
   }
 
//xg: @@@@: temporarily disble these. will revisit later
#ifndef WLAN_SOFTAP_FEATURE
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
#endif
   {
      pPktMetaInfo->ucUP = pktNode->userPriority;
      pPktMetaInfo->ucTID = pPktMetaInfo->ucUP;
   }

   pPktMetaInfo->ucType = 0;          //FIXME Don't know what this is
   //Extract the destination address from ethernet frame
   pDestMacAddress = (v_MACADDR_t*)skb->data;
   /* For BC/MC packets, disable hardware frame translation */
   pPktMetaInfo->ucDisableFrmXtl = vos_is_macaddr_group( pDestMacAddress ) ? 1 : 0; 
   pPktMetaInfo->ucBcast = vos_is_macaddr_broadcast( pDestMacAddress ) ? 1 : 0;
   pPktMetaInfo->ucMcast = vos_is_macaddr_group( pDestMacAddress ) ? 1 : 0;

   if ( (pAdapter->aStaInfo[STAId].txSuspended) &&
        (ac == pAdapter->aStaInfo[STAId].txSuspendedAc) &&
        (size <= ((pAdapter->aTxQueueLimit[ac]*3)/4) ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                 "%s: TX queue re-enabled", __FUNCTION__);
      pAdapter->aStaInfo[STAId].txSuspended = VOS_FALSE;
      netif_carrier_on(pAdapter->pDev);
      netif_start_queue(pAdapter->pDev);
   }    

#if 0
   //BC/MC packets go to WLANTL_BC_STA_ID now. Error Checking.
   if ( ((pPktMetaInfo->ucBcast || pPktMetaInfo->ucMcast) && (WLANTL_BC_STA_ID != STAId)) ||
        ((0 == pPktMetaInfo->ucBcast) && (0 == pPktMetaInfo->ucMcast) && (WLANTL_BC_STA_ID == STAId)) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "%s: Incorrect BC/MC handling", __FUNCTION__);
      vos_pkt_return_packet(pVosPacket);
      ++pAdapter->stats.tx_dropped;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeueError;
      kfree_skb(skb);
      return VOS_STATUS_E_FAILURE;
   }
#endif


#ifndef WLAN_SOFTAP_FEATURE
   // if we are in a backpressure situation see if we can turn the hose back on
   if ( (pAdapter->isTxSuspended) &&
        (ac == pAdapter->txSuspendedAc) &&
        (size <= HDD_TX_QUEUE_LOW_WATER_MARK) )
   {
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDePressured;
      ++pAdapter->hdd_stats.hddTxRxStats.txFetchDePressuredAC[ac];
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                 "%s: TX queue re-enabled", __FUNCTION__);
      pAdapter->isTxSuspended = VOS_FALSE;
      netif_carrier_on(pAdapter->pDev);
      netif_start_queue(pAdapter->pDev);
   }    
#endif
   // We're giving the packet to TL so consider it transmitted from
   // a statistics perspective.  We account for it here instead of
   // when the packet is returned for two reasons.  First, TL will
   // manipulate the skb to the point where the len field is not
   // accurate, leading to inaccurate byte counts if we account for
   // it later.  Second, TL does not provide any feedback as to
   // whether or not the packet was successfully sent over the air,
   // so the packet counts will be the same regardless of where we
   // account for them
   pAdapter->stats.tx_bytes += skb->len;
   ++pAdapter->stats.tx_packets;
   ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeued;
   ++pAdapter->hdd_stats.hddTxRxStats.txFetchDequeuedAC[ac];

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Valid VOS PKT returned to TL", __FUNCTION__);

   return status;
}


/**============================================================================
  @brief hdd_softap_tx_low_resource_cbk() - Callback function invoked in the 
  case where VOS packets are not available at the time of the call to get 
  packets. This callback function is invoked by VOS when packets are 
  available.

  @param pVosPacket : [in]  pointer to VOS packet 
  @param userData   : [in]  opaque user data that was passed initially 
  
  @return           : VOS_STATUS_E_FAILURE if any errors encountered, 
                    : VOS_STATUS_SUCCESS otherwise
  =============================================================================*/
VOS_STATUS hdd_softap_tx_low_resource_cbk( vos_pkt_t *pVosPacket, 
                                    v_VOID_t *userData )
{
   VOS_STATUS status;
   v_SINT_t i = 0;
   v_SIZE_t size = 0;
   hdd_hostapd_adapter_t* pAdapter = (hdd_hostapd_adapter_t *)userData;
   v_U8_t STAId = WLAN_MAX_STA_COUNT;
 
   if(pAdapter == NULL)
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   //Return the packet to VOS. We just needed to know that VOS is out of low resource
   //situation. Here we will only signal TL that there is a pending data for a STA. 
   //VOS packet will be requested (if needed) when TL comes back to fetch data.
   vos_pkt_return_packet( pVosPacket );

   pAdapter->isVosOutOfResource = VOS_FALSE;
   
   // Indicate to TL that there is pending data if a queue is non empty.
   // This Code wasnt included in earlier version which resulted in
   // Traffic stalling
   for (STAId = 0; STAId < WLAN_MAX_STA_COUNT; STAId++)
   {
      if ((pAdapter->aStaInfo[STAId].tlSTAState == WLANTL_STA_AUTHENTICATED) ||
           (pAdapter->aStaInfo[STAId].tlSTAState == WLANTL_STA_CONNECTED))
      {
         for( i=NUM_TX_QUEUES-1; i>=0; --i )
         {
            size = 0;
            hdd_list_size(&pAdapter->aStaInfo[STAId].wmm_tx_queue[i], &size);
            if ( size > 0 )
            {
               status = WLANTL_STAPktPending( pAdapter->pvosContext, 
                                        STAId, 
                                        (WLANTL_ACEnumType)i );
               if( !VOS_IS_STATUS_SUCCESS( status ) )
               {
                  VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Failure in indicating pkt to TL for ac=%d", __FUNCTION__,i); 
               }
            }
         }
      }
   } 
   return VOS_STATUS_SUCCESS;
}


/**============================================================================
  @brief hdd_softap_rx_packet_cbk() - Receive callback registered with TL.
  TL will call this to notify the HDD when one or more packets were
  received for a registered STA.

  @param vosContext      : [in] pointer to VOS context  
  @param pVosPacketChain : [in] pointer to VOS packet chain
  @param staId           : [in] Destinatioin Station Id. 
  @param pRxMetaInfo     : [in] pointer to meta info for the received pkt(s).

  @return                : VOS_STATUS_E_FAILURE if any errors encountered, 
                         : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
VOS_STATUS hdd_softap_rx_packet_cbk( v_VOID_t *vosContext, 
                              vos_pkt_t *pVosPacketChain,
                              v_U8_t staId,
                              WLANTL_RxMetaInfoType* pRxMetaInfo )
{
   hdd_hostapd_adapter_t *pAdapter = NULL;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   int rxstat;
   struct sk_buff *skb = NULL;
   vos_pkt_t* pVosPacket;
   vos_pkt_t* pNextVosPacket;

   //Sanity check on inputs
   if ( ( NULL == vosContext ) || 
        ( NULL == pVosPacketChain ) ||
        ( NULL == pRxMetaInfo ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Null params being passed", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   pAdapter = (hdd_hostapd_adapter_t *)vos_get_context( VOS_MODULE_ID_HDD_SOFTAP, vosContext );
   if ( NULL == pAdapter )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: HDD adapter context is Null", __FUNCTION__);
      return VOS_STATUS_E_FAILURE;
   }

   ++pAdapter->hdd_stats.hddTxRxStats.rxChains;

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
         ++pAdapter->hdd_stats.hddTxRxStats.rxDropped;
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Failure walking packet chain", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }

      // Extract the OS packet (skb).
      // Tell VOS to detach the OS packet from the VOS packet
      status = vos_pkt_get_os_packet( pVosPacket, (v_VOID_t **)&skb, VOS_TRUE );
      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         ++pAdapter->hdd_stats.hddTxRxStats.rxDropped;
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Failure extracting skb from vos pkt", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
      }
   
      //hdd_softap_dump_sk_buff(skb);

      skb->dev = pAdapter->pDev;
      ++pAdapter->hdd_stats.hddTxRxStats.rxPackets;
      ++pAdapter->stats.rx_packets;
      pAdapter->stats.rx_bytes += skb->len;

      if (WLAN_RX_BCMC_STA_ID == staId)
      {
        //MC/BC packets. Duplicate a copy of packet
        struct sk_buff *pSkbCopy;
        pSkbCopy = skb_copy(skb, GFP_ATOMIC);

        if (pSkbCopy)
        {
           hdd_softap_hard_start_xmit(pSkbCopy, skb->dev);
        }
        else
        {
            VOS_TRACE(VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                      "%s: skb allocation fails", __FUNCTION__);
        }

      } //(WLAN_RX_BCMC_STA_ID == staId)

      if ((WLAN_RX_BCMC_STA_ID == staId) || (WLAN_RX_SAP_SELF_STA_ID == staId))
      {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO_LOW,
                          "%s: send one packet to kernel \n", __FUNCTION__);

         skb->protocol = eth_type_trans(skb, skb->dev);
         skb->ip_summed = CHECKSUM_UNNECESSARY;
         rxstat = netif_rx_ni(skb);
         if (NET_RX_SUCCESS == rxstat)
         {
            ++pAdapter->hdd_stats.hddTxRxStats.rxDelivered;
         }
         else
         {
            ++pAdapter->hdd_stats.hddTxRxStats.rxRefused;
         }
      }
      else if(pAdapter->apDisableIntraBssFwd)
      {
        kfree_skb(skb);   
      } 
      else
      {
         //loopback traffic
        status = hdd_softap_sta_2_sta_xmit(skb, skb->dev,staId,(pRxMetaInfo->ucUP)); 
      }

      // now process the next packet in the chain
      pVosPacket = pNextVosPacket;

   } while (pVosPacket);

   //Return the entire VOS packet chain to the resource pool
   status = vos_pkt_return_packet( pVosPacketChain );
   if(!VOS_IS_STATUS_SUCCESS( status ))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,"%s: Failure returning vos pkt", __FUNCTION__);
   }
   
   pAdapter->pDev->last_rx = jiffies;

   return status;   
}

VOS_STATUS hdd_softap_DeregisterSTA( hdd_hostapd_adapter_t *pAdapter, tANI_U8 staId )
{
    VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;

    vosStatus = hdd_softap_deinit_tx_rx_sta ( pAdapter, staId);
    if( VOS_STATUS_E_FAILURE == vosStatus ){
        VOS_TRACE ( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                    "hdd_softap_deinit_tx_rx_sta() failed for staID %d. Status = %d [0x%08lX]",
                    staId, vosStatus, vosStatus );
        return (vosStatus );
    }

    vosStatus = WLANTL_ClearSTAClient( pAdapter->pvosContext, staId );
    if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, 
                    "WLANTL_ClearSTAClient() failed to for staID %d.  Status= %d [0x%08lX]",
                    staId, vosStatus, vosStatus );
    }

    return( vosStatus );
}

VOS_STATUS hdd_softap_RegisterSTA( hdd_hostapd_adapter_t *pAdapter,
                                       v_BOOL_t fAuthRequired,
                                       v_BOOL_t fPrivacyBit,
                                       v_U8_t staId,
                                       v_U8_t ucastSig,
                                       v_U8_t bcastSig,
                                       v_MACADDR_t *pPeerMacAddress,
                                       v_BOOL_t fWmmEnabled )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
   WLAN_STADescType staDesc;
   //eCsrEncryptionType connectedCipherAlgo;
   //v_BOOL_t  fConnected;
   
   /*
    * Clean up old entry if it is not cleaned up properly
   */
   if ( pAdapter->aStaInfo[staId].isUsed )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "clean up old entry\n");
      hdd_softap_DeregisterSTA( pAdapter, staId );
   }

   // Get the Station ID from the one saved during the assocation.
   staDesc.ucSTAId = staId;

   staDesc.wSTAType = WLAN_STA_SOFTAP;

   vos_mem_copy( staDesc.vSTAMACAddress.bytes, pPeerMacAddress->bytes,sizeof(pPeerMacAddress->bytes) );
   vos_mem_copy( staDesc.vBSSIDforIBSS.bytes, &pAdapter->macAddressCurrent,6 );
   vos_copy_macaddr( &staDesc.vSelfMACAddress, &pAdapter->macAddressCurrent );

   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "register station \n");
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "station mac %02x:%02x:%02x:%02x:%02x:%02x\n",
                      staDesc.vSTAMACAddress.bytes[0],
                      staDesc.vSTAMACAddress.bytes[1],
                      staDesc.vSTAMACAddress.bytes[2],
                      staDesc.vSTAMACAddress.bytes[3],
                      staDesc.vSTAMACAddress.bytes[4],
                      staDesc.vSTAMACAddress.bytes[5]);
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "BSSIDforIBSS %02x:%02x:%02x:%02x:%02x:%02x\n",
                      staDesc.vBSSIDforIBSS.bytes[0],
                      staDesc.vBSSIDforIBSS.bytes[1],
                      staDesc.vBSSIDforIBSS.bytes[2],
                      staDesc.vBSSIDforIBSS.bytes[3],
                      staDesc.vBSSIDforIBSS.bytes[4],
                      staDesc.vBSSIDforIBSS.bytes[5]);
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "SOFTAP SELFMAC %02x:%02x:%02x:%02x:%02x:%02x\n",
                      staDesc.vSelfMACAddress.bytes[0],
                      staDesc.vSelfMACAddress.bytes[1],
                      staDesc.vSelfMACAddress.bytes[2],
                      staDesc.vSelfMACAddress.bytes[3],
                      staDesc.vSelfMACAddress.bytes[4],
                      staDesc.vSelfMACAddress.bytes[5]);

   vosStatus = hdd_softap_init_tx_rx_sta(pAdapter, staId, &staDesc.vSTAMACAddress);

#ifdef WLAN_SOFTAP_FEATURE
   staDesc.ucQosEnabled = fWmmEnabled;
#else
   // set the QoS field appropriately
   if (hdd_wmm_is_active(pAdapter))
   {
      staDesc.ucQosEnabled = 1;
   }
   else
   {
      staDesc.ucQosEnabled = 0;
   }
#endif
   VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, "HDD SOFTAP register TL QoS_enabled=%d\n", 
              staDesc.ucQosEnabled );

   staDesc.ucProtectedFrame = (v_U8_t)fPrivacyBit ;

   // UMA is ready we inform TL not to do frame 
   // translation for WinMob 6.1
   // @@@@@ xg: what value
   staDesc.ucSwFrameTXXlation = 0;
   staDesc.ucSwFrameRXXlation = 1;
   staDesc.ucAddRmvLLC = 1;

   // Initialize signatures and state
   staDesc.ucUcastSig  = ucastSig;
   staDesc.ucBcastSig  = bcastSig;
   staDesc.ucInitState = fAuthRequired ?
      WLANTL_STA_CONNECTED : WLANTL_STA_AUTHENTICATED;

   // Register the Station with TL...      
   vosStatus = WLANTL_RegisterSTAClient( pAdapter->pvosContext, 
                                         hdd_softap_rx_packet_cbk, 
                                         hdd_softap_tx_complete_cbk, 
                                         hdd_softap_tx_fetch_packet_cbk, &staDesc );
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR, 
                 "SOFTAP WLANTL_RegisterSTAClient() failed to register.  Status= %d [0x%08lX]",
                 vosStatus, vosStatus );
      return vosStatus;      
   }                                            
    
   // if ( WPA ), tell TL to go to 'connected' and after keys come to the driver, 
   // then go to 'authenticated'.  For all other authentication types (those that do 
   // not require upper layer authentication) we can put TL directly into 'authenticated'
   // state.
   
   //VOS_ASSERT( fConnected );
   pAdapter->aStaInfo[staId].ucSTAId = staId;
   if ( !fAuthRequired )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                 "open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time", pAdapter->conn_info.staId[ 0 ] );
   
      // Connections that do not need Upper layer auth, transition TL directly
      // to 'Authenticated' state.      
      vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_AUTHENTICATED );
  
      pAdapter->aStaInfo[staId].tlSTAState = WLANTL_STA_AUTHENTICATED;
      pAdapter->conn_info.uIsAuthenticated = VOS_TRUE;
   }                                            
   else
   {

      VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                 "ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time", pAdapter->conn_info.staId[ 0 ] );
   
      vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_CONNECTED );
      pAdapter->aStaInfo[staId].tlSTAState = WLANTL_STA_CONNECTED;

      pAdapter->conn_info.uIsAuthenticated = VOS_FALSE;
   }      

   return( vosStatus );
}

VOS_STATUS hdd_softap_Register_BC_STA( hdd_hostapd_adapter_t *pAdapter, v_BOOL_t fPrivacyBit)
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

   v_MACADDR_t broadcastMacAddr = VOS_MAC_ADDR_BROADCAST_INITIALIZER;

   vosStatus = hdd_softap_RegisterSTA( pAdapter, VOS_FALSE, fPrivacyBit, pAdapter->uBCStaId, 0, 1, &broadcastMacAddr,0);

   return vosStatus;
}

VOS_STATUS hdd_softap_Deregister_BC_STA( hdd_hostapd_adapter_t *pAdapter)
{
   return hdd_softap_DeregisterSTA( pAdapter, pAdapter->uBCStaId);
}

VOS_STATUS hdd_softap_stop_bss( hdd_hostapd_adapter_t *pAdapter)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    v_U8_t staId = 0;

    vosStatus = hdd_softap_Deregister_BC_STA( pAdapter);

    if (!HAL_STATUS_SUCCESS(vosStatus))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                   "%s: Failed to deregister BC sta Id %d", __FUNCTION__, pAdapter->uBCStaId);
        return vosStatus;
    }

    for (staId = 0; staId < WLAN_MAX_STA_COUNT; staId++)
    {
        if (pAdapter->aStaInfo[staId].isUsed)// This excludes BC sta as it is already deregistered
            vosStatus = hdd_softap_DeregisterSTA( pAdapter, staId);

        if (!HAL_STATUS_SUCCESS(vosStatus))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                       "%s: Failed to deregister sta Id %d", __FUNCTION__, staId);
            return vosStatus;
        }
    }

    return vosStatus;
}

VOS_STATUS hdd_softap_change_STA_state( hdd_hostapd_adapter_t *pAdapter, v_MACADDR_t *pDestMacAddress, WLANTL_STAStateType state)
{

    tHalHandle hHalHandle;
    v_U8_t ucSTAId = WLAN_MAX_STA_COUNT;
    VOS_STATUS vosStatus = eHAL_STATUS_SUCCESS;
	v_CONTEXT_t pVosContext = pAdapter->pvosContext;
    hHalHandle = (tHalHandle ) vos_get_context(VOS_MODULE_ID_HAL, pVosContext);
    hddLog(LOG1, FL("%s enter \n"));

    if (eHAL_STATUS_SUCCESS != halTable_FindStaidByAddr(hHalHandle, (tANI_U8 *)pDestMacAddress, &ucSTAId))
    {
        VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                   "%s: Failed to find right station", __FUNCTION__);
        return VOS_STATUS_E_FAILURE;
    }

    if (FALSE == vos_is_macaddr_equal(&pAdapter->aStaInfo[ucSTAId].macAddrSTA, pDestMacAddress))
    {
         VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                    "%s: Station MAC address does not matching", __FUNCTION__);
         return VOS_STATUS_E_FAILURE;
    }

    vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, ucSTAId, state );
    VOS_TRACE( VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_ERROR,
                   "%s: change station to state %d succeed", __FUNCTION__, state);

    if (eHAL_STATUS_SUCCESS == vosStatus)
    {
       pAdapter->aStaInfo[ucSTAId].tlSTAState = WLANTL_STA_AUTHENTICATED;
    }

    VOS_TRACE(VOS_MODULE_ID_HDD_SOFTAP, VOS_TRACE_LEVEL_INFO,
	          "%s exit\n",__FUNCTION__);

    return vosStatus;
}


VOS_STATUS hdd_softap_GetStaId(hdd_hostapd_adapter_t *pAdapter, v_MACADDR_t *pMacAddress, v_U16_t *staId)
{
    v_U16_t i;

    for (i = 0; i < WLAN_MAX_STA_COUNT; i++)
    {
        if (vos_mem_compare(&pAdapter->aStaInfo[i].macAddrSTA, pMacAddress, sizeof(v_MACADDR_t)) &&
            pAdapter->aStaInfo[i].isUsed)
        {
            *staId = i;
            return VOS_STATUS_SUCCESS;
        }
    }

    return VOS_STATUS_E_FAILURE;
}

#endif //WLAN_SOFTAP_FEATURE
