#if !defined( __WLAN_QCT_PAL_PACKET_H )
#define __WLAN_QCT_PAL_PACKET_H

/**=========================================================================
  
  \file  wlan_qct_pal_packet.h
  
  \brief define PAL packet. wpt = (Wlan Pal Type)
               
   Definitions for platform independent.
  
   Copyright 2010 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"

typedef enum
{
   ///Packet is used to transmit 802.11 Management frames.
   eWLAN_PAL_PKT_TYPE_TX_802_11_MGMT,
   ///Packet is used to transmit 802.11 Data frames.
   eWLAN_PAL_PKT_TYPE_TX_802_11_DATA,
   ///Packet is used to transmit 802.3 Data frames.
   eWLAN_PAL_PKT_TYPE_TX_802_3_DATA,
   ///Packet contains Received data of an unknown frame type
   eWLAN_PAL_PKT_TYPE_RX_RAW
} wpt_packet_type;


typedef struct swpt_packet
{
   /*
   Pointer to a buffer for BD for TX packets
   For RX packets. The pBD MUST set to NULL.
   PAL packet shall set the pointer point to the start of the flat buffer
   where the BD starts.
   */
   void *pBD;
   //Physical address for pBD for DMA-able devices
   void *pBDPhys; 
   //OS dependent strucutre used only by OS specific code.
   void *pOSStruct;  
   void *pktMetaInfo;
   wpt_packet_type pktType;
   //The number of bytes pBD uses. It MUST be set to 0 for RX packets
   wpt_uint16 BDLength;

   //Internal data for PAL packet implementation usage only
   void *pInternalData; 
} wpt_packet;

typedef struct swpt_iterator
{
   void *pNext;  
   void *pCur;
   void *pContext;
} wpt_iterator;


//pPkt is a pointer to wpt_packet
#define WPAL_PACKET_SET_BD_POINTER(pPkt, pBd)   ( (pPkt)->pBD = (pBd) )
#define WPAL_PACKET_GET_BD_POINTER(pPkt)  ( (pPkt)->pBD )
//Access the physical address of BD
#define WPAL_PACKET_SET_BD_PHYS(pPkt, pBdPhys)   ( (pPkt)->pBDPhys = (pBdPhys) )
#define WPAL_PACKET_GET_BD_PHYS(pPkt)  ( (pPkt)->pBDPhys )
#define WPAL_PACKET_SET_BD_LENGTH(pPkt, len)   ( (pPkt)->BDLength = (len) )
#define WPAL_PACKET_GET_BD_LENGTH(pPkt)  ( (pPkt)->BDLength )
#define WPAL_PACKET_SET_METAINFO_POINTER(pPkt, p) ( (pPkt)->pktMetaInfo = (p) )
#define WPAL_PACKET_GET_METAINFO_POINTER(pPkt) ( (pPkt)->pktMetaInfo )
#define WPAL_PACKET_SET_TYPE(pPkt, type)  ( (pPkt)->pktType = (type) )
#define WPAL_PACKET_GET_TYPE(pPkt)  ( (pPkt)->pktType )
#define WPAL_PACKET_SET_OS_STRUCT_POINTER(pPkt, pStruct)   ( (pPkt)->pOSStruct = (pStruct) )
#define WPAL_PACKET_GET_OS_STRUCT_POINTER(pPkt)  ( (pPkt)->pOSStruct )
#define WPAL_PACKET_IS_FLAT_BUF(pktType) ( (eWLAN_PAL_PKT_TYPE_RX_RAW == (pktType)) || \
                                           (eWLAN_PAL_PKT_TYPE_TX_802_11_MGMT == (pktType)) )

/* RX RAW packet alloc fail due to out of resource CB function type */
typedef void ( *wpalPacketLowPacketCB )( wpt_packet *pPacket, void *usrData );


/*---------------------------------------------------------------------------
    wpalPacketInit – Initialize all wpt_packet related objects. Allocate memory for wpt_packet. 
    Allocate memory for TX management frames and RX frames.
    For our legacy UMAC, it is not needed because vos_packet contains wpt_packet.
    Param: 
        pPalContext – A context PAL uses??
    Return:
        eWLAN_PAL_STATUS_SUCCESS -- success
---------------------------------------------------------------------------*/
wpt_status wpalPacketInit(void *pPalContext);

/*---------------------------------------------------------------------------
    wpalPacketClose – Free all allocated resource by wpalPacketInit.
    For our legacy UMAC, it is not needed because vos_packet contains pal_packet.
    Param: 
        pPalContext – A context PAL uses??
    Return:
        eWLAN_PAL_STATUS_SUCCESS -- success
---------------------------------------------------------------------------*/
wpt_status wpalPacketClose(void *pPalContext);


/*---------------------------------------------------------------------------
    wpalPacketAlloc – Allocate a wpt_packet from PAL.
    Param: 
        pPalContext – A context PAL uses??
        pktType – specify the type of wpt_packet to allocate
        nPktSize - specify the maximum size of the packet buffer.
    Return:
        A pointer to the wpt_packet. NULL means fail.
---------------------------------------------------------------------------*/
wpt_packet * wpalPacketAlloc(wpt_packet_type pktType, wpt_uint32 nPktSize,
                             wpalPacketLowPacketCB rxLowCB, void *usrdata);

/*---------------------------------------------------------------------------
    wpalPacketFree – Free a wpt_packet chain for one particular type.
    Packet type is carried in wpt_packet structure.
    Param: 
        pPalContext – A context PAL uses??
        pPkt - pointer to a packet to be freed.
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success
---------------------------------------------------------------------------*/
wpt_status wpalPacketFree(wpt_packet *pPkt);

/*---------------------------------------------------------------------------
    wpalPacketGetLength – Get number of bytes in a wpt_packet.
    Param: 
        pPalContext – PAL context returned from PAL open
        pPkt - pointer to a packet to be freed.
    Return:
        Length of the data include layer-2 headers. For example, if the frame is 802.3, 
        the length includes the ethernet header.
---------------------------------------------------------------------------*/
wpt_uint32 wpalPacketGetLength(wpt_packet *pPkt);

/*---------------------------------------------------------------------------
    wpalPacketRawTrimHead – Move the starting offset and reduce packet length.
          The function can only be used with raw packets,
          whose buffer is one piece and allocated by WLAN driver. 
    Param: 
        pPkt - pointer to a wpt_packet.
        size – number of bytes to take off the head.
    Return:
        eWPAL_STATUS_SUCCESS - success. Fail otherwise.
---------------------------------------------------------------------------*/
wpt_status wpalPacketRawTrimHead(wpt_packet *pPkt, wpt_uint32 size);

/*---------------------------------------------------------------------------
    wpalPacketRawTrimTail – reduce the length of the packet. The function can 
          only be used with raw packets, whose buffer is one piece and 
          allocated by WLAN driver. This also reduce the length of the packet.
    Param: 
        pPkt - pointer to a wpt_packet.
        size – number of bytes to take of the packet length
    Return:
        eWLAN_PAL_STATUS_SUCCESS – success. Otherwise fail.
---------------------------------------------------------------------------*/
wpt_status wpalPacketRawTrimTail(wpt_packet *pPkt, wpt_uint32 size);


/*---------------------------------------------------------------------------
    wpalPacketGetRawBuf – Return the starting buffer's virtual address for the RAW flat buffer
    It is inline in hope of faster implementation for certain platform.
    Param: 
        pPkt - pointer to a wpt_packet.
    Return:
        NULL - fail.
        Otherwise the address of the starting of the buffer
---------------------------------------------------------------------------*/
extern WPT_INLINE wpt_uint8 *wpalPacketGetRawBuf(wpt_packet *pPkt);


/*---------------------------------------------------------------------------
    wpalPacketSetRxLength – Set the valid data length on a RX packet. This function must 
    be called once per RX packet per receiving. It indicates the available data length from
    the start of the buffer.
    Param: 
        pPkt - pointer to a wpt_packet.
    Return:
        NULL - fail.
        Otherwise the address of the starting of the buffer
---------------------------------------------------------------------------*/
extern WPT_INLINE wpt_status wpalPacketSetRxLength(wpt_packet *pPkt, wpt_uint32 len);


/*---------------------------------------------------------------------------
    wpalIteratorInit – Initialize an interator by updating pCur to first item.
    Param: 
        pIter – pointer to a caller allocated wpt_iterator
        pPacket – pointer to a wpt_packet
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success
---------------------------------------------------------------------------*/
wpt_status wpalIteratorInit(wpt_iterator *pIter, wpt_packet *pPacket);

/*---------------------------------------------------------------------------
    wpalIteratorNext – Get the address for the next item
    Param: 
        pIter – pointer to a caller allocated wpt_iterator
        pPacket – pointer to a wpt_packet
        ppAddr – Caller allocated pointer to return the address of the item. For DMA-able devices, this is the physical address of the item.
        pLen – To return the number of bytes in the item.
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success
---------------------------------------------------------------------------*/
wpt_status wpalIteratorNext(wpt_iterator *pIter, wpt_packet *pPacket, void **ppAddr, wpt_uint32 *pLen);


/*---------------------------------------------------------------------------
    wpalLockPacketForTransfer – Packet must be locked before transfer can begin,
    the lock will ensure that the DMA engine has access to the data packet
    in a cache coherent manner
 
    Param: 
        pPacket – pointer to a wpt_packet
 
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success
---------------------------------------------------------------------------*/
wpt_status wpalLockPacketForTransfer( wpt_packet *pPacket);

/*---------------------------------------------------------------------------
    wpalUnlockPacket – Once the transfer has been completed the packet should
                       be unlocked so that normal operation may resume
    Param: 
        pPacket – pointer to a wpt_packet
 
    Return:
        eWLAN_PAL_STATUS_SUCCESS - success
---------------------------------------------------------------------------*/
wpt_status wpalUnlockPacket( wpt_packet *pPacket);

#endif // __WLAN_QCT_PAL_PACKET_H
