/**=========================================================================
 *     
 *       \file  wlan_qct_dti_bd.c
 *          
 *       \brief Datapath utilities file.
 *                               
 * WLAN Device Abstraction layer External API for Dataservice
 * DESCRIPTION
 *  This file contains the external API implemntation exposed by the 
 *   wlan device abstarction layer module.
 *
 *   Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
 *   Qualcomm Confidential and Proprietary
 */

#include "wlan_qct_wdi.h"
#include "wlan_qct_wdi_ds.h"
#include "wlan_qct_wdi_ds_i.h"
#include "wlan_qct_wdi_dts.h"
#include "wlan_qct_wdi_dp.h"
#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_packet.h"



/*==========================================================================
 *
 FUNCTION    WDI_DS_PrepareBDHeader

 DESCRIPTION
 function for preparing BD header before HAL processing.

 PARAMETERS

 IN
palPacket:     PAL packet pointer 


RETURN VALUE
No return.

SIDE EFFECTS

============================================================================*/
void
WDI_DS_PrepareBDHeader (wpt_packet* palPacket, 
	wpt_uint8 ucDisableHWFrmXtl, wpt_uint8 alignment)
{
  void*          pvBDHeader;
  wpt_uint8      ucHeaderOffset;
  wpt_uint8      ucHeaderLen;
  wpt_uint8      ucQosEnabled;
  wpt_uint8      ucWDSEnabled;
  wpt_uint32     ucMpduLen;
  wpt_uint32     ucPktLen;
  WDI_DS_TxMetaInfoType     *pTxMetadata;


  /* Extract reuqired information from Metadata */
  pvBDHeader = WPAL_PACKET_GET_BD_POINTER(palPacket);
  pTxMetadata = WDI_DS_ExtractTxMetaData(palPacket);
  ucQosEnabled = pTxMetadata->qosEnabled;
  ucWDSEnabled = pTxMetadata->fenableWDS;

  WPAL_PACKET_SET_BD_LENGTH(palPacket, WDI_TX_BD_HEADER_SIZE);

  /*---------------------------------------------------------------------
    Fill MPDU info fields:
    - MPDU data start offset
    - MPDU header start offset
    - MPDU header length
    - MPDU length - this is a 16b field - needs swapping
    --------------------------------------------------------------------*/

  if ( ucDisableHWFrmXtl ) {
    ucHeaderOffset = WDI_TX_BD_HEADER_SIZE;
    ucHeaderLen = WDI_802_11_HEADER_LEN;
    if ( 0 != ucQosEnabled ) {
      ucHeaderLen += WDI_802_11_HEADER_QOS_CTL;
    }
    if ( 0 != ucWDSEnabled) {
      ucHeaderLen    += WDI_802_11_HEADER_ADDR4_LEN;
    }
  } else {
    ucHeaderOffset = WDI_TX_BD_HEADER_SIZE+WDI_802_11_MAX_HEADER_LEN;
    ucHeaderLen = WDI_802_3_HEADER_LEN;
  }

  WDI_TX_BD_SET_MPDU_HEADER_LEN( pvBDHeader, ucHeaderLen);
  WDI_TX_BD_SET_MPDU_HEADER_OFFSET( pvBDHeader, ucHeaderOffset);
  WDI_TX_BD_SET_MPDU_DATA_OFFSET( pvBDHeader,
      ucHeaderOffset + ucHeaderLen + alignment);

  // pkt length from PAL API. Need to change in case of HW FT used
  ucPktLen  = wpalPacketGetLength( palPacket ); // This includes BD length
  /** This is the length (in number of bytes) of the entire MPDU 
      (header and data). Note that the length INCLUDES FCS field. */
  ucMpduLen = ucPktLen - WPAL_PACKET_GET_BD_LENGTH( palPacket );
  WDI_TX_BD_SET_MPDU_LEN( pvBDHeader, ucMpduLen );

  DTI_TRACE(  DTI_TRACE_LEVEL_INFO,
      "WLAN DTI: VALUES ARE HLen=%x Hoff=%x doff=%x len=%x ex=%d",
      ucHeaderLen, ucHeaderOffset, 
      (ucHeaderOffset + ucHeaderLen + alignment), 
      pTxMetadata->fPktlen, alignment);

}/* WDI_DS_PrepareBDHeader */

/*==========================================================================
 *
 FUNCTIONS    WDI_DS_MemPoolXXX

 DESCRIPTION
  APIs for managing the BD header memory pool
 PARAMETERS

 IN
WDI_DS_BdMemPoolType:     Memory pool pointer 



============================================================================*/

/*
 * Create a memory pool which is DMA capabale
 */
WDI_Status WDI_DS_MemPoolCreate(WDI_DS_BdMemPoolType *memPool, wpt_uint8 chunkSize,
                                                                  wpt_uint8 numChunks)
{
  //Allocate all the max size and align them to a double word boundary. The first 8 bytes are control bytes.
  memPool->numChunks = 0;
  memPool->chunkSize = chunkSize + 16 - (chunkSize%8);
  memPool->pVirtBaseAddress = wpalDmaMemoryAllocate((numChunks * memPool->chunkSize),
          &(memPool->pPhysBaseAddress));

  if( memPool->pVirtBaseAddress == 0)
    return WDI_STATUS_E_FAILURE;

  memPool->AllocationBitmap = (wpt_uint32*)wpalMemoryAllocate( (numChunks/32 + 1) * sizeof(wpt_uint32));
  if( NULL == memPool->AllocationBitmap) 
     return WDI_STATUS_E_FAILURE;
  wpalMemoryZero(memPool->AllocationBitmap, (numChunks/32+1)*sizeof(wpt_uint32));
  
  return WDI_STATUS_SUCCESS;
}

/*
 * Destroy the memory pool
 */
void WDI_DS_MemPoolDestroy(WDI_DS_BdMemPoolType *memPool)
{
  //Allocate all the max size.
  wpalDmaMemoryFree(memPool->pVirtBaseAddress);
  wpalMemoryZero(memPool, sizeof(*memPool));
}
/*
 * Allocate chunk memory
 */
WPT_STATIC WPT_INLINE int find_leading_zero_and_setbit(wpt_uint32 *bitmap, wpt_uint32 maxNumPool)
{
  wpt_uint32 i,j, word;

  for(i=0; i < (maxNumPool/32 + 1); i++){
    j = 0;
    word = bitmap[i]; 
    for(j=0; j< 32; j++){
      if((word & 1) == 0) {
        bitmap[i] |= (1 << j);
        return((i<<5) + j);
      }
      word >>= 1;
    }
  }
  return -1;
}
  
void *WDI_DS_MemPoolAlloc(WDI_DS_BdMemPoolType *memPool, void **pPhysAddress, 
                               WDI_ResPoolType wdiResPool)
{
  wpt_uint32 index;
  void *pVirtAddress;
  wpt_uint32 maxNumPool;
  switch(wdiResPool) 
  {
    case WDI_MGMT_POOL_ID:
      maxNumPool = WDI_DS_HI_PRI_RES_NUM;
      break;
    case WDI_DATA_POOL_ID:
       maxNumPool = WDI_DS_LO_PRI_RES_NUM;
      break;
    default:
      return NULL; 
  }

  if(maxNumPool == memPool->numChunks) 
  { 
     return NULL; 
  }
  //Find the leading 0 in the allocation bitmap

  if((index = find_leading_zero_and_setbit(memPool->AllocationBitmap, maxNumPool)) == -1) 
  {
     //DbgBreakPoint();
     DTI_TRACE(  DTI_TRACE_LEVEL_INFO, "WDI_DS_MemPoolAlloc: index:%d(NULL), numChunks:%d",
                  index, memPool->numChunks );
     return NULL; 
  }
  memPool->numChunks++;
  // The first 8 bytes are reserved for internal use for control bits and hash.
  pVirtAddress  = (wpt_uint8 *)memPool->pVirtBaseAddress + (memPool->chunkSize * index) + 8;
  *pPhysAddress = (wpt_uint8 *)memPool->pPhysBaseAddress + (memPool->chunkSize * index) + 8;

  DTI_TRACE(  DTI_TRACE_LEVEL_INFO, "WDI_DS_MemPoolAlloc: index:%d, numChunks:%d", index, memPool->numChunks );

  return pVirtAddress;
  
}

/*
 * Free chunk memory
 */
void  WDI_DS_MemPoolFree(WDI_DS_BdMemPoolType *memPool, void *pVirtAddress, void *pPhysAddress)
{
  wpt_uint32 index = 
    ((wpt_uint8 *)pVirtAddress - (wpt_uint8 *)memPool->pVirtBaseAddress - 8)/memPool->chunkSize;
  wpt_uint32 word = memPool->AllocationBitmap[index/32];
  word &= ~(1<<(index%32));
  memPool->AllocationBitmap[index/32] = word;
  memPool->numChunks--;

  //DbgPrint( "WDI_DS_MemPoolFree: index:%d, numChunks:%d", index, memPool->numChunks );
}


/**
 @brief Returns the available number of resources (BD headers) 
        available for TX
 
 @param  pMemPool:         pointer to the BD memory pool 
  
 @see
 @return Result of the function call
*/
wpt_uint32 WDI_DS_GetAvailableResCount(WDI_DS_BdMemPoolType *pMemPool)
{
  return pMemPool->numChunks;
}
