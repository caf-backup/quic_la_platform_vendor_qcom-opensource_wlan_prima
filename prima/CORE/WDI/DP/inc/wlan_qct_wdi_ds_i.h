#if !defined( __WLAN_QCT_WDI_DS_I_H ) 
#define __WLAN_QCT_WDI_DS_I_H

/**=========================================================================
 *     
 *       \file  wlan_qct_wdi_ds_i.h
 *          
 *       \brief define Dataservice API 
 *                               
 * WLAN Device Abstraction layer External API for Dataservice
 * DESCRIPTION
 *  This file contains the external API exposed by the 
 *   wlan device abstarction layer module.
 *
 *   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
 *   Qualcomm Confidential and Proprietary
 */

#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_packet.h"
#include "wlan_qct_wdi_ds.h"


#define WDI_DS_MAX_CHUNK_SIZE 128
#define WDI_802_11_MAX_HEADER_LEN 40
#define MAX_NUM_CHUNKS 32


#define WDI_MAC_ADDR_SIZE ( 6 )
/*802.3 header definitions*/
#define  WDI_802_3_HEADER_LEN             14
/* Offset of DA field in a 802.3 header*/
#define  WDI_802_3_HEADER_DA_OFFSET        0
/*802.11 header definitions - header len without QOS ctrl field*/
#define  WDI_802_11_HEADER_LEN            24
/*802.11 header length + QOS ctrl field*/
#define  WDI_MPDU_HEADER_LEN              26
/*802.11 header definitions*/
#define  WDI_802_11_MAX_HEADER_LEN        40
/*802.11 header definitions - qos ctrl field len*/
#define  WDI_802_11_HEADER_QOS_CTL         2
/*802.11 ADDR4 MAC addr field len */
#define  WDI_802_11_HEADER_ADDR4_LEN       WDI_MAC_ADDR_SIZE





typedef enum 
{
   DTI_TRACE_LEVEL_FATAL,
   DTI_TRACE_LEVEL_ERROR,
   DTI_TRACE_LEVEL_WARN,
   DTI_TRACE_LEVEL_INFO

} DTI_TRACE_LEVEL;

WPT_STATIC WPT_INLINE void DTI_TRACE ( DTI_TRACE_LEVEL level, ...) { };

typedef struct {  
  void *pVirtBaseAddress; 
  void *pPhysBaseAddress; 
  wpt_uint32 poolSize;
  wpt_uint32 numChunks;
  wpt_uint32 chunkSize;
  wpt_uint32 AllocationBitmap[MAX_NUM_CHUNKS/32];
  
} WDI_DS_BdMemPoolType;

WDI_Status WDI_DS_MemPoolCreate(WDI_DS_BdMemPoolType *memPool, wpt_uint8 chunk_size);
void *WDI_DS_MemPoolAlloc(WDI_DS_BdMemPoolType *memPool, void **pPhysAddress);
void  WDI_DS_MemPoolFree(WDI_DS_BdMemPoolType *memPool, void *pVirtAddress, void *pPhysAddress);


typedef struct
{  
  void                            *pcontext;
  void                            *pCallbackContext;
  wpt_uint8			   suspend;	
  WDI_DS_BdMemPoolType		   memPool;       
  WDI_DS_RxPacketCallback          receiveFrameCB;
  WDI_DS_TxCompleteCallback        txCompleteCB;
  WDI_DS_TxFlowControlCallback     txResourceCB;
} WDI_DS_ClientDataType;

WPT_STATIC WPT_INLINE void WDI_GetBDPointers(wpt_packet *pFrame, void **pVirt, void **pPhys)
{
        *pVirt = pFrame->pBD;
        *pPhys = pFrame->pBDPhys;
}


WPT_STATIC WPT_INLINE void WDI_SetBDPointers(wpt_packet *pFrame, void *pVirt, void *pPhys)
{
  pFrame->pBD = pVirt;
  pFrame->pBDPhys = pPhys;
}


void
WDI_DS_PrepareBDHeader (
  wpt_packet*     palPacket,
  wpt_uint8      ucDisableHWFrmXtl,
  wpt_uint8       alignment
);


#endif
