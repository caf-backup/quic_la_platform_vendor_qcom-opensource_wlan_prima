#if !defined( WLAN_HDD_TX_RX_H )
#define WLAN_HDD_TX_RX_H

/**===========================================================================
  
  \file  wlan_hdd_tx_rx.h
  
  \brief Linux HDD Tx/RX APIs
         Copyright 2008 (c) Qualcomm, Incorporated.
         All Rights Reserved.
         Qualcomm Confidential and Proprietary.
  
  ==========================================================================*/
  
/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <wlan_hdd_includes.h>
#include <vos_api.h>
#include <linux/skbuff.h>
#include <wlan_qct_tl.h>

/*--------------------------------------------------------------------------- 
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/ 
#define HDD_ETHERTYPE_802_1_X              ( 0x888E )
#define HDD_ETHERTYPE_802_1_X_FRAME_OFFSET ( 12 )
#define HDD_ETHERTYPE_802_1_X_SIZE         ( 2 )

/*--------------------------------------------------------------------------- 
  Type declarations
  -------------------------------------------------------------------------*/ 
 
/*--------------------------------------------------------------------------- 
  Function declarations and documenation
  -------------------------------------------------------------------------*/ 

/**============================================================================
  @brief hdd_hard_start_xmit() - Function registered with the Linux OS for 
  transmitting packets

  @param skb      : [in]  pointer to OS packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
extern int hdd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);

/**============================================================================
  @brief hdd_tx_timeout() - Function called by OS if there is any
  timeout during transmission. Since HDD simply enqueues packet
  and returns control to OS right away, this would never be invoked

  @param dev : [in] pointer to Libra network device
  @return    : None
  ===========================================================================*/
extern void hdd_tx_timeout(struct net_device *dev);

/**============================================================================
  @brief hdd_stats() - Function registered with the Linux OS for 
  device TX/RX statistics

  @param dev      : [in] pointer to Libra network device
  
  @return         : pointer to net_device_stats structure
  ===========================================================================*/
extern struct net_device_stats* hdd_stats(struct net_device *dev);

/**============================================================================
  @brief hdd_init_tx_rx() - Init function to initialize Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
extern VOS_STATUS hdd_init_tx_rx( hdd_adapter_t *pAdapter );

/**============================================================================
  @brief hdd_deinit_tx_rx() - Deinit function to clean up Tx/RX
  modules in HDD

  @param pAdapter : [in] pointer to adapter context  
  @return         : VOS_STATUS_E_FAILURE if any errors encountered 
                  : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
extern VOS_STATUS hdd_deinit_tx_rx( hdd_adapter_t *pAdapter );

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
extern VOS_STATUS hdd_tx_complete_cbk( v_VOID_t *vosContext, 
                                       vos_pkt_t *pVosPacket, 
                                       VOS_STATUS vosStatusIn );

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
extern VOS_STATUS hdd_tx_fetch_packet_cbk( v_VOID_t *vosContext,
                                           v_U8_t *pStaId,
                                           WLANTL_ACEnumType*    pucAC,
                                           vos_pkt_t **ppVosPacket,
                                           WLANTL_MetaInfoType *pPktMetaInfo );

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
extern VOS_STATUS hdd_tx_low_resource_cbk( vos_pkt_t *pVosPacket, 
                                           v_VOID_t *userData );

/**============================================================================
  @brief hdd_rx_packet_cbk() - Receive callback registered with TL.
  TL will call this to notify the HDD when a packet was received 
  for a registered STA.

  @param vosContext   : [in] pointer to VOS context  
  @param pVosPacket   : [in] pointer to VOS packet (conatining sk_buff) 
  @param staId        : [in] Station Id
  @param pRxMetaInfo  : [in] pointer to meta info for the received pkt(s) 

  @return             : VOS_STATUS_E_FAILURE if any errors encountered, 
                      : VOS_STATUS_SUCCESS otherwise
  ===========================================================================*/
extern VOS_STATUS hdd_rx_packet_cbk( v_VOID_t *vosContext, 
                                     vos_pkt_t *pVosPacket, 
                                     v_U8_t staId,
                                     WLANTL_RxMetaInfoType* pRxMetaInfo );

#endif    // end #if !defined( WLAN_HDD_TX_RX_H )
