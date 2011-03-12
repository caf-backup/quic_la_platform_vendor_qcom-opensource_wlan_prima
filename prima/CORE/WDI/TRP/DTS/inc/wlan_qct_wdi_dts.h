#ifndef __WLAN_QCT_DTS_H
#define __WLAN_QCT_DTS_H
/**=========================================================================
 *     
 *       \file  wlan_qct_wdi_dts.h
 *          
 *       \brief define Datas Trnasport Service API 
 *                               
 * WLAN Device Abstraction layer interface for Transport drivers (SDIO/DXE)
 * DESCRIPTION
 * This file contains the API exposed by the 
 * wlan device abstarction layer module for abstracting DXE/SDIO.
 *
 * Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 * Example usage for DXE.
 * ----------------------
 * On Platform init
 *         DAL  will then invoke WDTS_open
 *
 * On DAL init
 *     DAL will invike WDTS_start
 *
 * On transmit:
 *         DAL will invoke WDTS_TxPacket API
 * 
 * On transmit complete:
 *         DXE will serialize into TX thread
 *         In TX thread it will invoke
 *
 * On receive:
 *        DXE will serialize into RX thread
 *        In TX thread it will invoke WDTS_RXPacket API 
 *
 * On DXE ring full:
 *        DXE will serialize into TX thread
 *        In TX thread it will invoke WDTS_OOResourceNotification API 
 */

typedef enum
{
   WDTS_CHANNEL_TX_LOW_PRI,
   WDTS_CHANNEL_TX_HIGH_PRI,
   WDTS_CHANNEL_RX_LOW_PRI,
   WDTS_CHANNEL_RX_HIGH_PRI,
   WDTS_CHANNEL_MAX
}  WDTS_ChannelType;

typedef wpt_status (*WDTS_TxCompleteCbType)(void *pContext, wpt_packet *pFrame, wpt_status status);
typedef wpt_status (*WDTS_RxFrameReadyCbType) (void *pContext, wpt_packet *pFrame, WDTS_ChannelType channel);
typedef wpt_status (*WDTS_LowResourceCbType)(void *pContext, WDTS_ChannelType channel, wpt_boolean on);

typedef struct {
  void * (*open)(void);
  wpt_status (*start) (void *pContext);
  wpt_status (*register_client)(void *pContext, WDTS_RxFrameReadyCbType, 
      WDTS_TxCompleteCbType, WDTS_LowResourceCbType, void *clientData);
  wpt_status (*xmit) (void *pContext, wpt_packet *packet, WDTS_ChannelType channel);
} WDTS_TransportDriverTrype;


/* DTS open  function. 
 * On open the transport device should initialize itself.
 * Parameters:
 *  pContext:Cookie that should be passed back to the caller along 
 *  with the callback.
 *
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 
 */
wpt_status WDTS_openTransport( void *pContext);




/* DTS start  function. 
 * On start the transport device should start running.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller along 
 * with the callback.
 *
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_startTransport( void *pContext);




/* DTS Tx packet function. 
 * This function should be invoked by the DAL Dataservice to schedule transmit frame through DXE/SDIO.
 * Parameters:
 * pContext:Cookie that should be passed back to the caller along with the callback.
 * pFrame:Refernce to PAL frame.
 * Return Value: SUCCESS  Completed successfully.
 *     FAILURE_XXX  Request was rejected due XXX Reason.
 *
 */
wpt_status WDTS_TxPacket(void *pContext, wpt_packet *pFrame);

#endif
