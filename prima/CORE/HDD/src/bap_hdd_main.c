/**========================================================================
  
  \file  bap_hdd_main.c

  \brief 802.11 BT-AMP PAL Host Device Driver implementation
               
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/**========================================================================= 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$   $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  12/1/09     JZmuda    Created module. 

  ==========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
  
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/spinlock.h>
//#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/io.h>
//#include <wlan_qct_driver.h>
#include <wlan_hdd_includes.h>
/* -------------------------------------------------------------------------*/
#include <bap_hdd_main.h>
#include <vos_api.h>
#include <bapApi.h>
#include <btampHCI.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

// the difference between the next two is that the first is the max 
// number we support in our current implementation while the second is
// the max allowed by the spec
#define BSL_MAX_PHY_LINKS           ( BSL_MAX_CLIENTS * BSL_MAX_PHY_LINK_PER_CLIENT )
#define BSL_MAX_ALLOWED_PHY_LINKS   255

// these likely will need tuning based on experiments
#define BSL_MAX_RX_PKT_DESCRIPTOR   100
#define BSL_MAX_TX_PKT_DESCRIPTOR   100

// these caps are in place to not have run-away queues, again needs empirical tuning
#define BSL_MAX_SIZE_TX_ACL_QUEUE   50
#define BSL_MAX_SIZE_RX_ACL_QUEUE   50
#define BSL_MAX_SIZE_RX_EVT_QUEUE   50

#if 0
What are the maximum sizes of a command packet, an event packet and an ACL 
   data packet?

   [JimZ]: Sizes:
   1. Cmd Maximum size is slightly greater than 672 btyes.  But I am pretty sure 
   right now that I will never have more than 240 bytes to send down at a time.  And 
   that is good. Because some rather unpleasant things happen at the HCI interface 
   ifI exceed that.  ( Think 8-bit CPUs.  And the limitations of an 8-bit length 
                       field. )

2. Event -  Ditto.

   3. Data 1492 bytes
#endif

// jimz
// TLV related defines

#define USE_FINAL_FRAMESC
//#undef USE_FINAL_FRAMESC
// jimz
// TLV related defines

#ifndef USE_FINAL_FRAMESC        //USE_FINAL_FRAMESC
// AMP ASSOC TLV related defines
#define AMP_ASSOC_TLV_TYPE_SIZE 2
#define AMP_ASSOC_TLV_LEN_SIZE 2
#define AMP_ASSOC_TLV_TYPE_AND_LEN_SIZE  (AMP_ASSOC_TLV_TYPE_SIZE + AMP_ASSOC_TLV_LEN_SIZE)

// FLOW SPEC TLV related defines
#define FLOWSPEC_TYPE_SIZE 2
#define FLOWSPEC_LEN_SIZE 2
#define FLOWSPEC_TYPE_AND_LEN_SIZE  (FLOWSPEC_TYPE_SIZE + FLOWSPEC_LEN_SIZE)

// CMD TLV related defines
#define CMD_TLV_TYPE_SIZE 2
#define CMD_TLV_LEN_SIZE 2
#define CMD_TLV_TYPE_AND_LEN_SIZE  (CMD_TLV_TYPE_SIZE + CMD_TLV_LEN_SIZE)

// Event TLV related defines
#define EVENT_TLV_TYPE_SIZE 2
#define EVENT_TLV_LEN_SIZE 2
#define EVENT_TLV_TYPE_AND_LEN_SIZE  (EVENT_TLV_TYPE_SIZE + EVENT_TLV_LEN_SIZE)

// Data header size related defines
#define DATA_HEADER_SIZE 4

#else                            //USE_FINAL_FRAMESC

// AMP ASSOC TLV related defines
#define AMP_ASSOC_TLV_TYPE_SIZE 1
#define AMP_ASSOC_TLV_LEN_SIZE 2
#define AMP_ASSOC_TLV_TYPE_AND_LEN_SIZE  (AMP_ASSOC_TLV_TYPE_SIZE + AMP_ASSOC_TLV_LEN_SIZE)

// FLOW SPEC TLV related defines
#define FLOWSPEC_TYPE_SIZE 1
#define FLOWSPEC_LEN_SIZE 1
#define FLOWSPEC_TYPE_AND_LEN_SIZE  (FLOWSPEC_TYPE_SIZE + FLOWSPEC_LEN_SIZE)

// CMD TLV related defines
#define CMD_TLV_TYPE_SIZE 2
#define CMD_TLV_LEN_SIZE 1
#define CMD_TLV_TYPE_AND_LEN_SIZE  (CMD_TLV_TYPE_SIZE + CMD_TLV_LEN_SIZE)

// Event TLV related defines
#define EVENT_TLV_TYPE_SIZE 1
#define EVENT_TLV_LEN_SIZE 1
#define EVENT_TLV_TYPE_AND_LEN_SIZE  (EVENT_TLV_TYPE_SIZE + EVENT_TLV_LEN_SIZE)

// Data header size related defines
#define DATA_HEADER_SIZE 4

#endif                           // USE_FINAL_FRAMESC
// jimz

#define BSL_MAX_EVENT_SIZE 700

#define BSL_DEV_HANDLE 0x1234

// Debug related defines
#define DBGLOG printf
//#define DUMPLOG
#if defined DUMPLOG
#define DUMPLOG(n, name1, name2, aStr, size) \
    if (1) \
{\
    int i;\
    DBGLOG("%d. %s: %s = \n", n, name1, name2); \
    for (i = 0; i < size; i++) \
        DBGLOG("%2.2x%s", ((unsigned char *)aStr)[i], i % 16 == 15 ? "\n" : " "); \
    DBGLOG("\n"); \
}
#else
#define DUMPLOG(n, name1, name2, aStr, size)
#endif

// These are required to replace some Microsoft specific specifiers
//#define UNALIGNED __align 
#define UNALIGNED  
#define INFINITE 0

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

// Temporary Windows types
typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef void * HANDLE;
typedef char  TCHAR;
typedef void *LPVOID;
typedef const void *LPCVOID;

typedef struct
{
   BOOL          used;          // is this a valid context?
//   HANDLE        ReadableEvt;   // the event a ReadFile can block on
   vos_event_t   ReadableEvt;   // the event a ReadFile can block on
   vos_list_t    HCIEvtQueue;   // the RX queue of HCI events
   vos_list_t    ACLRxQueue;    // the RX queue of ACL data
   ptBtampHandle bapHdl;        // our handle in BAP
   vos_list_t    PhyLinks;      // a list of all associations setup by this client

} BslClientCtxType;

typedef struct
{
   TCHAR* ValueName;     // name of the value
   DWORD  Type;          // type of value
   DWORD  DwordValue;    // DWORD value
   TCHAR* StringValue;   // string value

} BslRegEntry;

typedef struct
{
   BOOL              used;                // is this a valid context?
   vos_list_t        ACLTxQueue[WLANTL_MAX_AC];  // the TX ACL queues
   BslClientCtxType* pClientCtx;          // ptr to application context that spawned
                                          // this association 
   v_U8_t            PhyLinkHdl;          // BAP handle for this association
   void*             pPhyLinkDescNode;    // ptr to node in list of assoc in client ctx 
                                          // real type BslPhyLinksNodeType*

} BslPhyLinkCtxType;

typedef struct
{
   vos_list_node_t    node;  // MUST be first element
   BslPhyLinkCtxType* pPhy;  // ptr to an association context

} BslPhyLinksNodeType;

typedef struct
{
   vos_list_node_t node;     // MUST be first element
   vos_pkt_t*      pVosPkt;  // ptr to a RX VoS pkt which can hold an HCI event or ACL data

} BslRxListNodeType;

typedef struct
{
   vos_list_node_t     node;         // MUST be first element
   vos_pkt_t*          pVosPkt;      // ptr to TX VoS pkt for ACL data
   WLANTL_MetaInfoType TlMetaInfo;   // meta-data needed by TL for above ACL data pkt

} BslTxListNodeType;

typedef struct
{
   BslPhyLinkCtxType* ptr;   // ptr to the association context for this phy_link_handle

} BslPhyLinkMapEntryType;

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
// Temporary (until multi-phy link) pointer to BT-AMP context
static void *gpCtx;

// an efficient lookup from phy_link_handle to phy link context
static BslPhyLinkMapEntryType BslPhyLinkMap[BSL_MAX_ALLOWED_PHY_LINKS];

//static HANDLE hBsl = NULL; //INVALID_HANDLE_VALUE;
static BOOL bBslInited = FALSE;

static BslClientCtxType BslClientCtx[BSL_MAX_CLIENTS];
static vos_lock_t BslClientLock;

static BslPhyLinkCtxType BslPhyLinkCtx[BSL_MAX_PHY_LINKS];
static vos_lock_t BslPhyLock;

// the pool for RX pkts used for RX ACL data and HCI events
static vos_list_t BslRxPktDescPool;
static BslRxListNodeType BslRxPktDesc[ BSL_MAX_RX_PKT_DESCRIPTOR ];

// the pool for TX pkts used for TX ACL data
static vos_list_t BslTxPktDescPool;
static BslTxListNodeType BslTxPktDesc[ BSL_MAX_TX_PKT_DESCRIPTOR ];

// the pool for association contexts
static vos_list_t BslPhyLinksDescPool;
static BslPhyLinksNodeType BslPhyLinksDesc[BSL_MAX_PHY_LINKS];

//static v_U32_t Eventlen = 0;

/*--------------------------------------------------------------------------- 
 *   Driver Structure definitions
 *-------------------------------------------------------------------------*/ 
static ssize_t BSL_Read(struct file *pFile, char __user *pBuffer, size_t Count,
			  loff_t *ppos);
static ssize_t BSL_Write(struct file *pFile, const char __user *pBuffer,
			 size_t Count, loff_t *pOff);
static int BSL_Open (struct inode *pInode, struct file *pFile);
static int BSL_Close (struct inode *pInode, struct file *pFile);


static struct file_operations bsl_fops = {
//	.owner = THIS_MODULE,
	.read = BSL_Read,
    .write = BSL_Write, 
    .open = BSL_Open, 
    .release= BSL_Close,
};

static struct miscdevice bsl_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bsl",
	.fops = &bsl_fops,
};

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/


/**
  @brief WLANBAP_STAFetchPktCB() - The fetch packet callback registered 
  with BAP by HDD. 
    
  It is called by the BAP immediately upon the underlying 
  WLANTL_STAFetchPktCBType routine being called.  Which is called by
  TL when the scheduling algorithms allows for transmission of another 
  packet to the module. 
    
  This function is here to "wrap" or abstract WLANTL_STAFetchPktCBType.
  Because the BAP-specific HDD "shim" layer (BSL) doesn't know anything 
  about STAIds, or other parameters required by TL.  

  @param pHddHdl: [in] The HDD(BSL) specific context for this association.  
  Use the STAId passed to me by TL in WLANTL_STAFetchCBType to retreive 
  this value.
  @param  pucAC: [inout] access category requested by TL, if HDD does not 
  have packets on this AC it can choose to service another AC queue in 
  the order of priority
  @param  vosDataBuff: [out] pointer to the VOSS data buffer that was 
  transmitted 
  @param tlMetaInfo: [out] meta info related to the data frame
  
  @return 
  The result code associated with performing the operation
*/
static VOS_STATUS WLANBAP_STAFetchPktCB
   (
   v_PVOID_t             pHddHdl,
   WLANTL_ACEnumType*    pucAC,
   vos_pkt_t**           vosDataBuff,
   WLANTL_MetaInfoType*  tlMetaInfo
   )
{
   BslPhyLinkCtxType* pctx;
   VOS_STATUS VosStatus;
   v_U8_t AcIdxStart;
   v_U8_t AcIdx;
   vos_list_node_t *pLink;
   BslTxListNodeType *pNode;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "WLANBAP_STAFetchPktCB\n" );

   // sanity checking
   if( pHddHdl == NULL || pucAC == NULL || vosDataBuff == NULL ||
       tlMetaInfo == NULL || *pucAC >= WLANTL_MAX_AC )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STAFetchPktCB bad input\n" );
      return VOS_STATUS_E_FAILURE;
   }

   pctx = (BslPhyLinkCtxType *)pHddHdl; 
   AcIdx = AcIdxStart = *pucAC;

   VosStatus = vos_list_remove_front( &pctx->ACLTxQueue[AcIdx], &pLink );

   if ( VOS_STATUS_E_EMPTY == VosStatus )
   {
      do
      {
         AcIdx = (AcIdx + 1) % WLANTL_MAX_AC;

         VosStatus = vos_list_remove_front( &pctx->ACLTxQueue[AcIdx], &pLink );

      } while ( VosStatus == VOS_STATUS_E_EMPTY && AcIdx != AcIdxStart );

      if ( VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         *pucAC = AcIdx;
      }
      else if ( VosStatus == VOS_STATUS_E_EMPTY )
      {
         // Queue is empty.  This can happen.  Just return NULL back to TL...
         return(VOS_STATUS_E_EMPTY);
      }
      else
      {
         VOS_ASSERT( 0 );
      }
   }

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_ASSERT( 0 );
   }

   pNode = (BslTxListNodeType *)pLink;

   // give TL the VoS pkt
   *vosDataBuff = pNode->pVosPkt;

   // provide the meta-info BAP provided previously
   *tlMetaInfo = pNode->TlMetaInfo;

   // need to stick a ptr to the TX node inside the VoS pkt to free the descriptor later
   // during TxCompleteCB.
   vos_pkt_set_user_data_ptr( pNode->pVosPkt, VOS_PKT_USER_DATA_ID_BAP, 
                              (v_PVOID_t) pNode );

   return(VOS_STATUS_SUCCESS);      
}

/**
  @brief WLANBAP_STARxCB() - The receive callback registered with BAP by HDD. 
    
  It is called by the BAP immediately upon the underlying 
  WLANTL_STARxCBType routine being called.  Which is called by
  TL to notify when a packet was received for a registered STA.

  @param  pHddHdl: [in] The HDD(BSL) specific context for this association.  
  Use the STAId passed to me by TL in WLANTL_STARxCBType to retrieve this value.
  @param  vosDataBuff: [in] pointer to the VOSS data buffer that was received 
  (it may be a linked list) 
  @param  pRxMetaInfo: [in] Rx meta info related to the data frame
  
  @return 
  The result code associated with performing the operation
*/
static VOS_STATUS WLANBAP_STARxCB
   (
   v_PVOID_t              pHddHdl,
   vos_pkt_t*             vosDataBuff,
   WLANTL_RxMetaInfoType* pRxMetaInfo
   )
{
   BslPhyLinkCtxType* pctx;
   BslClientCtxType* ppctx;
   vos_list_node_t* pLink;
   BslRxListNodeType *pNode;
   VOS_STATUS VosStatus;
   WLANTL_ACEnumType Ac; // this is not needed really
   v_SIZE_t ListSize;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "WLANBAP_STARxCB\n" );

   // sanity checking
   if ( pHddHdl == NULL || vosDataBuff == NULL || pRxMetaInfo == NULL )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STARxCB bad input\n" );
      return VOS_STATUS_E_FAILURE;
   }

   pctx = (BslPhyLinkCtxType *)pHddHdl; 
   ppctx = pctx->pClientCtx;

   VOS_ASSERT( ppctx != NULL );

   // check if queue is already at capped size
   VosStatus = vos_list_size( &ppctx->ACLRxQueue, &ListSize );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_ASSERT(0);
   }

   if ( ListSize == BSL_MAX_SIZE_RX_ACL_QUEUE )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STARxCB \
          vos_list_size ==%d\n", ListSize );

      VosStatus = vos_pkt_return_packet( vosDataBuff );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      return VOS_STATUS_E_FAILURE;
   }

   // process the packet
   VosStatus = WLANBAP_XlateRxDataPkt( ppctx->bapHdl, pctx->PhyLinkHdl, 
                                       &Ac, vosDataBuff );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STARxCB WLANBAP_XlateRxDataPkt \
          failed status = %d\n", VosStatus );

      VosStatus = vos_pkt_return_packet( vosDataBuff );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      return VOS_STATUS_E_FAILURE;
   }

   // get a pkt desc
   VosStatus = vos_list_remove_front( &BslRxPktDescPool, &pLink );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      // this can happen if the pool is not big enough, free the VoS pkt
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STARxCB vos_list_remove_front \
          failed status=%d\n", VosStatus );

      VosStatus = vos_pkt_return_packet( vosDataBuff );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      return VOS_STATUS_E_FAILURE;
   }

   // stick the VOS pkt into the node
   pNode = (BslRxListNodeType *) pLink;
   pNode->pVosPkt = vosDataBuff;

   // stick in a ptr to the RX descriptor inside the user data area of the VoS pkt
   // so the descriptor can be freed later when the Read completes
   vos_pkt_set_user_data_ptr( vosDataBuff, VOS_PKT_USER_DATA_ID_BAP, 
                              (v_PVOID_t) pNode );

   // insert the pkt in the ACL Rx queue
   VosStatus = vos_list_insert_back( &ppctx->ACLRxQueue, pLink );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      // this should never happen
      VOS_ASSERT(0);
   }

   // signal event
   //if ( !SetEvent( ppctx->ReadableEvt ) )
   VosStatus = vos_event_set( &(ppctx->ReadableEvt) );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_STARxCB vos_event_set \
          failed status=%d\n", VosStatus );

      // only way this can happen is if BAP is giving us a Rx Pkt after the app
      // has done a BSL_Close, this is a violation of the BAP contract
      VOS_ASSERT(0);
   }

   return(VOS_STATUS_SUCCESS);
}

/**
  @brief WLANBAP_TxCompCB() - The Tx complete callback registered with BAP by HDD. 
    
  It is called by the BAP immediately upon the underlying 
  WLANTL_TxCompCBType routine being called.  Which is called by
  TL to notify when a transmission for a packet has ended.

  @param pHddHdl: [in] The HDD(BSL) specific context for this association
  @param vosDataBuff: [in] pointer to the VOSS data buffer that was transmitted 
  @param wTxSTAtus: [in] status of the transmission 
  
  @return 
  The result code associated with performing the operation
*/
static VOS_STATUS WLANBAP_TxCompCB
   (
   v_PVOID_t      pHddHdl,
   vos_pkt_t*     vosDataBuff,
   VOS_STATUS     wTxSTAtus 
   )
{
   VOS_STATUS VosStatus;
   BslTxListNodeType* pTxNode;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "WLANBAP_TxCompCB\n" );

   // be aware that pHddHdl can be NULL or can point to the per association
   // BSL context from the register data plane. In either case it does not 
   // matter since we will simply free the VoS pkt and reclaim the TX
   // descriptor
   
   // sanity checking
   if ( vosDataBuff == NULL )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_TxCompCB bad input\n" );
      return VOS_STATUS_E_FAILURE;
   }

   // extract from the user data area the ptr to the TX descriptor which 
   // was in limbo following its Fetch
   vos_pkt_get_user_data_ptr( vosDataBuff, VOS_PKT_USER_DATA_ID_BAP, 
                              (v_VOID_t**)&pTxNode );

   VOS_ASSERT( pTxNode != NULL );

   VosStatus = vos_pkt_return_packet( vosDataBuff );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_ASSERT(0);
   }

   // now release the TX descriptor
   VosStatus = vos_list_insert_front( &BslTxPktDescPool, &pTxNode->node );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   return(VOS_STATUS_SUCCESS);
}

/**
  @brief BslReleasePhyCtx() - this function will free up an association context 
 
  @param pPhyCtx : [in] ptr to the phy context to release

  @return 
  None

*/
static void BslReleasePhyCtx
   (
   BslPhyLinkCtxType* pPhyCtx
   )
{
   v_U32_t OldMapVal;
   VOS_STATUS VosStatus;
   v_U8_t j;
   vos_list_node_t* pLink;
   BslTxListNodeType* pTxNode;
   vos_pkt_t* pVosPkt;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslReleasePhyCtx\n" );

   if (BslPhyLinkMap[pPhyCtx->PhyLinkHdl].ptr == NULL) return;

   
   // update the phy link handle based map so TX data is stopped from flowing through
   OldMapVal = vos_atomic_set_U32( (v_U32_t *) (BslPhyLinkMap[pPhyCtx->PhyLinkHdl].ptr), 
                                   (v_U32_t) 0 );

   //Remove the VOS_ASSERT on Ravi's advice
   //VOS_ASSERT( OldMapVal != 0 );
// cleanup the queues
#if 0
//Work around for data transfer to work with disconnect->reconnect feature
   for ( j=0; j<WLANTL_MAX_AC; j++ )
   {
      while ( VOS_IS_STATUS_SUCCESS( VosStatus = vos_list_remove_front( &pPhyCtx->ACLTxQueue[j], &pLink ) ) );
   
       VosStatus = vos_list_destroy( &pPhyCtx->ACLTxQueue[j] );
       //VosStatus = vos_list_init( &(BslPhyLinkCtx+i)->ACLTxQueue );
       VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }
//End of work around
#endif
//Commented out the implementation to check DISCONNECTING---->DISCONNECTED feature   
// cleanup the queues
   for ( j=0; j<WLANTL_MAX_AC; j++ )
   {
      while ( VOS_IS_STATUS_SUCCESS( VosStatus = vos_list_remove_front( &pPhyCtx->ACLTxQueue[j], &pLink ) ) )
      {
         pTxNode = (BslTxListNodeType *)pLink;

         // extract the VoS pkt from the list node element
         pVosPkt = pTxNode->pVosPkt;

         // now free the VoS pkt
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         // now free the TX desc
         VosStatus = vos_list_insert_front( &BslTxPktDescPool, &pTxNode->node );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
      }

      VOS_ASSERT( VosStatus == VOS_STATUS_E_EMPTY );

      VosStatus = vos_list_destroy( &pPhyCtx->ACLTxQueue[j] );
      //VosStatus = vos_list_init( &(BslPhyLinkCtx+i)->ACLTxQueue );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   // clear out the parent ptr
 //  pPhyCtx->pClientCtx = NULL;//commented to debug exception

   // we also need to remove this assocation from the list of active
   // associations maintained in the application context
   if( pPhyCtx->pPhyLinkDescNode )
   {
      VosStatus = vos_list_remove_node( &pPhyCtx->pClientCtx->PhyLinks, 
          &((BslPhyLinksNodeType*)pPhyCtx->pPhyLinkDescNode)->node);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
      //Return the PhyLink handle to the free pool
      VosStatus = vos_list_insert_front(&BslPhyLinksDescPool,&((BslPhyLinksNodeType*)pPhyCtx->pPhyLinkDescNode)->node);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      pPhyCtx->pPhyLinkDescNode = NULL;
   }
   pPhyCtx->pClientCtx = NULL;//Moved here to bebug the exception

   // release the context we were using back for re-use
   VosStatus = vos_lock_acquire(&BslPhyLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   pPhyCtx->used = FALSE;

   VosStatus = vos_lock_release(&BslPhyLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
}

/**
  @brief WLAN_BAPEventCB() - Implements the callback for ALL asynchronous events. 

  Including Events resulting from:
     * HCI Create Physical Link, 
     * Disconnect Physical Link, 
     * Create Logical Link,
     * Flow Spec Modify, 
     * HCI Reset,
     * HCI Flush,...

  Also used to return sync events locally by BSL  

  @param pHddHdl: [in] The HDD(BSL) specific context for this association.  
  BSL gets this from the downgoing packets Physical handle value. 
  @param pBapHCIEvent: [in] pointer to the union of "HCI Event" structures.  
  Contains all info needed for HCI event.
  @param AssocSpecificEvent: [in] flag indicates assoc-specific (1) or 
  global (0) event
   
  @return
  The result code associated with performing the operation  

  VOS_STATUS_E_FAULT:  pointer to pBapHCIEvent is NULL 
  VOS_STATUS_SUCCESS:  Success
*/
static VOS_STATUS WLANBAP_EventCB
   (
   v_PVOID_t      pHddHdl,   /* this could refer to either the BSL per 
                                association context which got passed in during 
                                register data plane OR the BSL per application 
                                context passed in during register BAP callbacks 
                                based on setting of the Boolean flag below */ 
   tpBtampHCI_Event pBapHCIEvent, /* This now encodes ALL event types including 
                                     Command Complete and Command Status*/
   v_BOOL_t AssocSpecificEvent /* Flag to indicate global or assoc-specific event */
   )   
{
   BslClientCtxType* pctx;
   VOS_STATUS VosStatus = VOS_STATUS_SUCCESS;
   vos_pkt_t* pVosPkt;
   v_U32_t PackStatus;
   v_U8_t Buff[BSL_MAX_EVENT_SIZE]; // stack overflow?
   v_U32_t Written = 0; // FramesC REQUIRES this
   vos_list_node_t* pLink;
   BslRxListNodeType *pNode;
   v_U32_t OldMapVal;
   v_SIZE_t ListSize;
   int plen = 0;

   // sanity checking
   if ( pHddHdl == NULL || pBapHCIEvent == NULL )
   {
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB bad input\n" );
     return VOS_STATUS_E_FAILURE;
   }

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB event=%d \
       assoc_specific=%d\n", pBapHCIEvent->bapHCIEventCode, AssocSpecificEvent );

   if ( AssocSpecificEvent )
   {
      // get the app context from the assoc context
      pctx = ((BslPhyLinkCtxType *)pHddHdl)->pClientCtx;
   }
   else
   {
      pctx = (BslClientCtxType *)pHddHdl;
   }

   // check if queue is already at capped size
   if(pctx && &pctx->HCIEvtQueue)
   VosStatus = vos_list_size( &pctx->HCIEvtQueue, &ListSize );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_ASSERT(0);
   }

   if ( ListSize == BSL_MAX_SIZE_RX_EVT_QUEUE )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB \
          vos_list_size ==%d\n", ListSize );

      return VOS_STATUS_E_FAILURE;
   }

   VosStatus = vos_pkt_get_packet( &pVosPkt, VOS_PKT_TYPE_RX_RAW, 
                                   BSL_MAX_EVENT_SIZE, 1, 0, NULL, NULL);

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB vos_pkt_get_packet \
          failed status=%d\n", VosStatus );
      return(VosStatus);
   }
#if 0
   Buff = vos_mem_malloc(BSL_MAX_EVENT_SIZE);
   if(Buff == NULL)
       return;
   else
       tmp = Buff;
#endif   
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "%s:Static Buff address=%u\n",__FUNCTION__,Buff);
   switch ( pBapHCIEvent->bapHCIEventCode )
   {
      /** BT events */
      case BTAMP_TLV_HCI_COMMAND_COMPLETE_EVENT:
         {
           /*
               BTAMP_TLV_HCI_RESET_CMD:
               BTAMP_TLV_HCI_FLUSH_CMD:
               BTAMP_TLV_HCI_LOGICAL_LINK_CANCEL_CMD:
               BTAMP_TLV_HCI_SET_EVENT_MASK_CMD:
               BTAMP_TLV_HCI_READ_CONNECTION_ACCEPT_TIMEOUT_CMD:
               BTAMP_TLV_HCI_WRITE_CONNECTION_ACCEPT_TIMEOUT_CMD:
               BTAMP_TLV_HCI_READ_LINK_SUPERVISION_TIMEOUT_CMD:
               BTAMP_TLV_HCI_WRITE_LINK_SUPERVISION_TIMEOUT_CMD:
               BTAMP_TLV_HCI_READ_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD:
               BTAMP_TLV_HCI_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD:
               BTAMP_TLV_HCI_SET_EVENT_MASK_PAGE_2_CMD:
               BTAMP_TLV_HCI_READ_LOCATION_DATA_CMD:
               BTAMP_TLV_HCI_WRITE_LOCATION_DATA_CMD:
               BTAMP_TLV_HCI_READ_FLOW_CONTROL_MODE_CMD:
               BTAMP_TLV_HCI_WRITE_FLOW_CONTROL_MODE_CMD:
               BTAMP_TLV_HCI_READ_BEST_EFFORT_FLUSH_TO_CMD:
               BTAMP_TLV_HCI_WRITE_BEST_EFFORT_FLUSH_TO_CMD:
               BTAMP_TLV_HCI_SET_SHORT_RANGE_MODE_CMD:
               BTAMP_TLV_HCI_READ_LOCAL_VERSION_INFORMATION_CMD:
               BTAMP_TLV_HCI_READ_LOCAL_SUPPORTED_COMMANDS_CMD:
               BTAMP_TLV_HCI_READ_BUFFER_SIZE_CMD:
               BTAMP_TLV_HCI_READ_DATA_BLOCK_SIZE_CMD:
               BTAMP_TLV_HCI_READ_FAILED_CONTACT_COUNTER_CMD:
               BTAMP_TLV_HCI_RESET_FAILED_CONTACT_COUNTER_CMD:
               BTAMP_TLV_HCI_READ_LINK_QUALITY_CMD:
               BTAMP_TLV_HCI_READ_RSSI_CMD:
               BTAMP_TLV_HCI_READ_LOCAL_AMP_INFORMATION_CMD:
               BTAMP_TLV_HCI_READ_LOCAL_AMP_ASSOC_CMD:
               BTAMP_TLV_HCI_WRITE_REMOTE_AMP_ASSOC_CMD:
               BTAMP_TLV_HCI_READ_LOOPBACK_MODE_CMD:
               BTAMP_TLV_HCI_WRITE_LOOPBACK_MODE_CMD:

            */

            // pack
            PackStatus = btampPackTlvHCI_Command_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampCommandCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_COMMAND_STATUS_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Command_Status_Event( NULL, 
                &pBapHCIEvent->u.btampCommandStatusEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_HARDWARE_ERROR_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Hardware_Error_Event( NULL, 
                &pBapHCIEvent->u.btampHardwareErrorEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_FLUSH_OCCURRED_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Flush_Occurred_Event( NULL, 
                &pBapHCIEvent->u.btampFlushOccurredEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_LOOPBACK_COMMAND_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Loopback_Command_Event( NULL, 
                &pBapHCIEvent->u.btampLoopbackCommandEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_DATA_BUFFER_OVERFLOW_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Data_Buffer_Overflow_Event( NULL, 
                &pBapHCIEvent->u.btampDataBufferOverflowEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_QOS_VIOLATION_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Qos_Violation_Event( NULL, 
                &pBapHCIEvent->u.btampQosViolationEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
         /** BT v3.0 events */
      case BTAMP_TLV_HCI_GENERIC_AMP_LINK_KEY_NOTIFICATION_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Generic_AMP_Link_Key_Notification_Event( NULL, 
                &pBapHCIEvent->u.btampGenericAMPLinkKeyNotificationEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_PHYSICAL_LINK_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Physical_Link_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampPhysicalLinkCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            // look at this event to determine whether to cleanup the PHY context
            if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                 WLANBAP_STATUS_SUCCESS )
            {
               // register the data plane now
               VosStatus = WLANBAP_RegisterDataPlane( pctx->bapHdl, 
                                                      WLANBAP_STAFetchPktCB, 
                                                      WLANBAP_STARxCB, 
                                                      WLANBAP_TxCompCB,
                                                      (BslPhyLinkCtxType *)pHddHdl );

               if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
               {
                  VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB WLANBAP_RegisterDataPlane \
                      failed status = %d\n", VosStatus );
                  // we still want to send the event upto app so do not bail
               }
               else
               {
                  // update the phy link handle based map so TX data can start flowing through
                  OldMapVal = vos_atomic_set_U32( (v_U32_t *)BslPhyLinkMap+pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.phy_link_handle, 
                                         (v_U32_t) pHddHdl );

//                  VOS_ASSERT( OldMapVal == 0 );//Commented to test reconnect
               }
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      WLANBAP_ERROR_HOST_REJ_RESOURCES )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      WLANBAP_ERROR_CNCT_TIMEOUT )
            {
		//We need to update the phy link handle here to be able to reissue physical link accept    
	       // update the phy link handle based map so TX data can start flowing through
                  OldMapVal = vos_atomic_set_U32( (v_U32_t *)BslPhyLinkMap+pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.phy_link_handle, 
                                         (v_U32_t) pHddHdl );

//                  VOS_ASSERT( OldMapVal == 0 );//Commented to test reconnect
               	    
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      WLANBAP_ERROR_MAX_NUM_CNCTS )
            {
	       	//We need to update the phy link handle here to be able to reissue physical link /create/accept    
	       // update the phy link handle based map so TX data can start flowing through
               OldMapVal = vos_atomic_set_U32( (v_U32_t *)BslPhyLinkMap+pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.phy_link_handle, 
                                         (v_U32_t) pHddHdl );
//                  VOS_ASSERT( OldMapVal == 0 );//Commented to test reconnect
 	    
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      WLANBAP_ERROR_CNCT_TIMEOUT )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      0x16 /* WLANBAP_ERROR_FAILED_CONNECTION? */ )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      0x8 /* WLANBAP_ERROR_AUTH_FAILED? */ )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else if ( pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status == 
                      WLANBAP_ERROR_NO_CNCT )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB unexpected HCI Phy Link Comp Evt \
                   status =%d\n", pBapHCIEvent->u.btampPhysicalLinkCompleteEvent.status );
            }

            break;
         }
      case BTAMP_TLV_HCI_CHANNEL_SELECTED_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Channel_Selected_Event( NULL, 
                &pBapHCIEvent->u.btampChannelSelectedEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_DISCONNECT_PHYSICAL_LINK_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Disconnect_Physical_Link_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampDisconnectPhysicalLinkCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            // we need to cleanup the PHY context always but have these checks to make
            // sure we catch unexpected behavior, strangely enough even when peer triggers
            // the disconnect the reason code is still 0x16, weird
            if ( pBapHCIEvent->u.btampDisconnectPhysicalLinkCompleteEvent.status == WLANBAP_STATUS_SUCCESS && 
                 pBapHCIEvent->u.btampDisconnectPhysicalLinkCompleteEvent.reason == WLANBAP_ERROR_TERM_BY_LOCAL_HOST )
            {
               BslReleasePhyCtx( (BslPhyLinkCtxType *)pHddHdl );
            }
            else
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB unexpected HCI Dis Phy Link Comp Evt \
                   status =%d reason =%d\n", pBapHCIEvent->u.btampDisconnectPhysicalLinkCompleteEvent.status,
                   pBapHCIEvent->u.btampDisconnectPhysicalLinkCompleteEvent.reason );
            }

            break;
         }
      case BTAMP_TLV_HCI_PHYSICAL_LINK_LOSS_WARNING_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Physical_Link_Loss_Warning_Event( NULL, 
                &pBapHCIEvent->u.btampPhysicalLinkLossWarningEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_PHYSICAL_LINK_RECOVERY_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Physical_Link_Recovery_Event( NULL, 
                &pBapHCIEvent->u.btampPhysicalLinkRecoveryEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_LOGICAL_LINK_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Logical_Link_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampLogicalLinkCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_DISCONNECT_LOGICAL_LINK_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Disconnect_Logical_Link_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampDisconnectLogicalLinkCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_FLOW_SPEC_MODIFY_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Flow_Spec_Modify_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampFlowSpecModifyCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      case BTAMP_TLV_HCI_SHORT_RANGE_MODE_CHANGE_COMPLETE_EVENT:
         {
            // pack
            PackStatus = btampPackTlvHCI_Short_Range_Mode_Change_Complete_Event( NULL, 
                &pBapHCIEvent->u.btampShortRangeModeChangeCompleteEvent, Buff, BSL_MAX_EVENT_SIZE, &Written );

            if ( !BTAMP_SUCCEEDED( PackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "WLANBAP_EventCB: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", PackStatus);
               // handle the error
               VosStatus = vos_pkt_return_packet( pVosPkt );

               VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

               return(VOS_STATUS_E_FAILURE);
            }

            break;
         }
      default:
         {
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB unexpected event\n" );

            VosStatus = vos_pkt_return_packet( pVosPkt );

            VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

            return(VOS_STATUS_E_FAILURE);
            break;
         }
   }

   VOS_ASSERT(Written <= BSL_MAX_EVENT_SIZE);

   // stick the event into a VoS pkt
   VosStatus = vos_pkt_push_head( pVosPkt, Buff, Written );
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "%s:Pushed Buff address=%u len=%d\n",__FUNCTION__,Buff,Written);
   for(plen = 0; plen < 5; plen++)
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "%s:Pushed Buff[%d]=%x\n",__FUNCTION__,plen,Buff[plen] );
   //Eventlen = Written;

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB vos_pkt_push_head \
          status =%d\n", VosStatus );

      // return the packet 
      VosStatus = vos_pkt_return_packet( pVosPkt );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

      return(VOS_STATUS_E_FAILURE);
   }

   // get a pkt desc
   VosStatus = vos_list_remove_front( &BslRxPktDescPool, &pLink );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      // this can happen if pool is too small or there is a leak, etc
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB vos_list_remove_front \
          status =%d\n", VosStatus );
     
      // return the packet 
      VosStatus = vos_pkt_return_packet( pVosPkt );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

      return(VOS_STATUS_E_FAILURE);
   }

   // stick the VOS pkt into the node
   pNode = (BslRxListNodeType *) pLink;
   pNode->pVosPkt = pVosPkt;
   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: Vospkt sticked is:%u\n", __FUNCTION__, pNode->pVosPkt);

   // stick in a ptr to the RX descriptor inside the user data area of the VoS pkt
   // so the descriptor can be freed later when the Read completes
   vos_pkt_set_user_data_ptr( pVosPkt, VOS_PKT_USER_DATA_ID_BAP, 
                              (v_PVOID_t) pNode );

   // insert the pkt in the HCI event queue
   VosStatus = vos_list_insert_back( &pctx->HCIEvtQueue, pLink );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_ASSERT(0);
   }

   // signal event
   //if ( ! SetEvent( pctx->ReadableEvt ) )
   VosStatus = vos_event_set( &(pctx->ReadableEvt) );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_EventCB vos_event_set \
          failed status=%d\n", VosStatus );

      // only way this can happen is if BAP is giving us an event after the app
      // has done a BSL_Close, this is a violation of the BAP contract
      VOS_ASSERT(0);
   }

   return(VOS_STATUS_SUCCESS);
}

/**
  @brief BslFindAndInitClientCtx() - This function will find and initialize a client 
  a.k.a app context
 
  @param pctx : [inout] ptr to the client context

  @return 
  TRUE if all OK, FALSE otherwise

*/
static BOOL BslFindAndInitClientCtx
   (
   BslClientCtxType** pctx_
   )
{
   VOS_STATUS VosStatus;
   BslClientCtxType* pctx;
   v_U8_t i;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslFindAndInitClientCtx\n" );

   VosStatus = vos_lock_init(&BslClientLock);

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {  
      VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"%s:BslClientLock already inited",__FUNCTION__); 
     // return(0);
   }

   // find a client context
   VosStatus = vos_lock_acquire(&BslClientLock);

   for ( i=0; i < BSL_MAX_CLIENTS; i++ )
   {
      if ( !BslClientCtx[i].used )
      {
         BslClientCtx[i].used = TRUE;
         break;
      }
   }

   VosStatus = vos_lock_release(&BslClientLock);

   if ( i == BSL_MAX_CLIENTS )
   {
      // no more clients can be supported
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslFindAndInitClientCtx no more \
          clients can be supported MAX=%d\n", BSL_MAX_CLIENTS );
      return FALSE;
   }

   pctx = BslClientCtx + i;

   // get a handle from BAP
   VosStatus = WLANBAP_GetNewHndl(&pctx->bapHdl);

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      // need to release back the client context
      VosStatus = vos_lock_acquire(&BslClientLock);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      pctx->used = FALSE;

      VosStatus = vos_lock_release(&BslClientLock);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s:WLAN_GetNewHndl Failed",__FUNCTION__);

      return(FALSE);
   }

   // register the event cb with BAP, this cb is used for BOTH association 
   // specific and non-association specific event notifications by BAP. 
   // However association specific events will be called with a different
   // cookie that is passed in during the physical link create/accept
   VosStatus = WLAN_BAPRegisterBAPCallbacks( pctx->bapHdl, WLANBAP_EventCB, pctx );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      // need to release back the client context
      VosStatus = vos_lock_acquire(&BslClientLock);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      pctx->used = FALSE;

      VosStatus = vos_lock_release(&BslClientLock);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s:WLAN_BAPRegsiterBAPCallaback Failed",__FUNCTION__);

      return(FALSE);
   }

   // TODO: none of the following APIs should fail hence no error
   // handling but asserting

   // init the "readable" event
//   pctx->ReadableEvt = CreateEvent( NULL, FALSE, FALSE, NULL );
//   VOS_ASSERT( pctx->ReadableEvt != NULL );
   VosStatus = vos_event_init (&(pctx->ReadableEvt));
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // init the HCI event queue
   VosStatus = vos_list_init( &pctx->HCIEvtQueue );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // init the RX ACL data queue
   VosStatus = vos_list_init( &pctx->ACLRxQueue );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // init the PhyLinks queue to keep track of the assoc's of this client
   VosStatus = vos_list_init( &pctx->PhyLinks );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   *pctx_ = pctx;

   return(TRUE);
}

/**
  @brief BslReleaseClientCtx() - This function will release a client a.k.a. app
  context 
 
  @param pctx : [in] ptr to the client context

  @return 
  None

*/
static void BslReleaseClientCtx
   (
   BslClientCtxType* pctx
   )
{
   VOS_STATUS VosStatus;
   vos_list_node_t* pLink;
   BslPhyLinksNodeType *pPhyNode;
   BslRxListNodeType* pRxNode;
   vos_pkt_t* pVosPkt;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslReleaseClientCtx\n" );

   // an app can do this without cleaning up after itself i.e. it can have active associations and
   // data pending, we need to cleanup its mess

   // first tell BAP we dont want the handle anymore, BAP will cleanup all the associations and 
   // consume resulting HCI events, so after this we will not get any HCI events. we will also
   // not see any FetchPktCB and RxPktCB. We can still expect TxCompletePktCB
   VosStatus = WLANBAP_ReleaseHndl( pctx->bapHdl );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // destroy the "readable" event
//   CloseHandle( pctx->ReadableEvt );
   VosStatus = vos_event_destroy( &(pctx->ReadableEvt) );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // find and free all of the association contexts belonging to this app
   while ( VOS_IS_STATUS_SUCCESS( VosStatus = vos_list_remove_front( &pctx->PhyLinks, &pLink ) ) )
   {
      pPhyNode = (BslPhyLinksNodeType *)pLink;

      // since the phy link has already been removed from the list of active
      // associations, make sure we dont attempt to do this again
      pPhyNode->pPhy->pPhyLinkDescNode = NULL;

      BslReleasePhyCtx( pPhyNode->pPhy );
   }

   VOS_ASSERT( VosStatus == VOS_STATUS_E_EMPTY );

   while ( VOS_IS_STATUS_SUCCESS( VosStatus = vos_list_remove_front( &pctx->HCIEvtQueue, &pLink ) ) )
   {
      pRxNode = (BslRxListNodeType *)pLink;

      // extract the VoS pkt from the list node element
      pVosPkt = pRxNode->pVosPkt;

      // now free the VoS pkt
      VosStatus = vos_pkt_return_packet( pVosPkt );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

      // now free the RX desc
      VosStatus = vos_list_insert_front( &BslRxPktDescPool, &pRxNode->node );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   VOS_ASSERT( VosStatus == VOS_STATUS_E_EMPTY );

   while ( VOS_IS_STATUS_SUCCESS( VosStatus = vos_list_remove_front( &pctx->ACLRxQueue, &pLink ) ) )
   {
      pRxNode = (BslRxListNodeType *)pLink;

      // extract the VoS pkt from the list node element
      pVosPkt = pRxNode->pVosPkt;

      // now free the VoS pkt
      VosStatus = vos_pkt_return_packet( pVosPkt );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

      // now free the RX desc
      VosStatus = vos_list_insert_front( &BslRxPktDescPool, &pRxNode->node );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   VOS_ASSERT( VosStatus == VOS_STATUS_E_EMPTY );

   // destroy the RX ACL data queue
   VosStatus = vos_list_destroy( &pctx->ACLRxQueue );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // destroy the HCI event queue
   VosStatus = vos_list_destroy( &pctx->HCIEvtQueue );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // destroy the PhyLinks queue 
   VosStatus = vos_list_destroy( &pctx->PhyLinks );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   // mark the client context as being available for re-use
   VosStatus = vos_lock_acquire(&BslClientLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   pctx->used = FALSE;

   VosStatus = vos_lock_release(&BslClientLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
}

/**
  @brief BslFindAndInitPhyCtx() - This function will try to find a free physical
  link a.k.a assocation context and if successful, then init that context 
 
  @param pctx : [in] the client context
  @param PhyLinkHdl : [in] the physical link handle chosen by application
  @param ppPhyCtx : [inout] ptr to the physical link context

  @return 
  TRUE if all OK, FALSE otherwise

*/
static BOOL BslFindAndInitPhyCtx
   (
   BslClientCtxType*   pctx,
   v_U8_t              PhyLinkHdl,
   BslPhyLinkCtxType** ppPhyCtx
   )
{
   VOS_STATUS VosStatus;
   v_U16_t i = 0;
   v_U16_t j;
   vos_list_node_t* pLink;
   BslPhyLinksNodeType *pNode;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslFindAndInitPhyCtx\n" );
#if 0
   //Work around for BSL Initphyctx failure for DISCONNECTING---->DISCONNECTED feature
 // now init this context
 *ppPhyCtx = &BslPhyLinkCtx[0];

 // setup a ptr to the app context that this assocation specific context lives in
      BslPhyLinkCtx[0].pClientCtx = pctx;

      // store the PHY link handle
      BslPhyLinkCtx[0].PhyLinkHdl = PhyLinkHdl;

//End of Workaround for  DISCONNECTING---->DISCONNECTED feature
//Work around for data transfer problem after disconnect->reconnect works
// init the TX queues
      for ( j=0; j<WLANTL_MAX_AC; j++ )
     {
         VosStatus = vos_list_init( &BslPhyLinkCtx[0].ACLTxQueue[j] );
         //VosStatus = vos_list_init( &(BslPhyLinkCtx+i)->ACLTxQueue );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
     }
      //end of work around
#endif     

   // find a free PHY context
   VosStatus = vos_lock_acquire(&BslPhyLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

#if 0
   for ( i=0; i<BSL_MAX_PHY_LINKS; i++ )
   {
      if ( !BslPhyLinkCtx[i].used )
      {
         BslPhyLinkCtx[i].used = TRUE;
         break;
      }
   }
#endif

   VosStatus = vos_lock_release(&BslPhyLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   if ( i==BSL_MAX_PHY_LINKS )
   {
      return(FALSE);
   }
   else
   {
      // now init this context

      *ppPhyCtx = BslPhyLinkCtx + i;

      // setup a ptr to the app context that this assocation specific context lives in
      BslPhyLinkCtx[i].pClientCtx = pctx;

      // store the PHY link handle
      BslPhyLinkCtx[i].PhyLinkHdl = PhyLinkHdl;

      // init the TX queues
      for ( j=0; j<WLANTL_MAX_AC; j++ )
      {
         VosStatus = vos_list_init( &BslPhyLinkCtx[i].ACLTxQueue[j] );
         //VosStatus = vos_list_init( &(BslPhyLinkCtx+i)->ACLTxQueue );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
      }

      // need to add this Phy context to the client list of associations,
      // useful during Close operation

      // get a pkt desc
      VosStatus = vos_list_remove_front( &BslPhyLinksDescPool, &pLink );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         // this could happen due to pool not being big enough, etc
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslFindAndInitPhyCtx failed to \
             get node from BslPhyLinksDescPool vstatus=%d\n", VosStatus );
         BslReleasePhyCtx( *ppPhyCtx );
         return FALSE;
      }

      // stick the VOS pkt into the node
      pNode = (BslPhyLinksNodeType *) pLink;
      pNode->node = *pLink;
      pNode->pPhy = *ppPhyCtx;


      // now queue the pkt into the correct queue
      VosStatus = vos_list_insert_back( &pctx->PhyLinks, pLink );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_ASSERT(0);
      }

      // need to record the desc for this assocation in the list of
      // active assocations in client context to allow cleanup later
      (*ppPhyCtx)->pPhyLinkDescNode = pNode;

      return(TRUE);
   }
}

/**
  @brief BslProcessHCICommand() - This function will process an HCI command i.e
  take an HCI command buffer, unpack it and then call the appropriate BAP API
 
  @param pctx : [in] ptr to the client context
  @param pBuffer_ : [in] the input buffer containing the HCI command 
  @param Count_ : [in] size of the HCI command buffer

  @return 
  TRUE if all OK, FALSE otherwise

*/
static BOOL BslProcessHCICommand
   (
   BslClientCtxType* pctx,
   LPCVOID pBuffer_,
   DWORD Count_ 
   )
{
   LPVOID pBuffer = (LPVOID) pBuffer_; // castaway the const-ness of the ptr
   v_U16_t Count = (v_U16_t) Count_;  // this should be OK max size < 1500
   v_U32_t UnpackStatus;
   VOS_STATUS VosStatus;
   BOOL Status;
   BslPhyLinkCtxType* pPhyCtx;
   tBtampHCI_Event HCIEvt;
   v_U16_t x = 1;
   int i = 0;

   // the opcode is in LE, if we are LE too then this is fine else we need some
   // byte swapping
   v_U16_t cmdOpcode = *(UNALIGNED v_U16_t *)pBuffer;
   v_U8_t *pBuf = (v_U8_t *)pBuffer;
   v_U8_t *pTmp = (v_U8_t *)pBuf;
 
   // TODO: do we really need to do this per call even though the op is quite cheap
   if(*(v_U8_t *)&x == 0)
   {
     // BE
     cmdOpcode = ( cmdOpcode & 0xFF ) << 8 | ( cmdOpcode & 0xFF00 ) >> 8;
   }

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslProcessHCICommand: cmdOpcode = %hx\n", cmdOpcode );

   // JEZ081118: Advance past two opcode bytes. Per Arun and Ravi's suggestion.
   // JEZ081118: FIXME.  I think the real answer is to use a FramesC routine.
   // JEZ081129: Advance past two length bytes. Discovered in Unit Test
   //pBuf+=2;
   // JEZ081204: Advance past BOTH the two type bytes AND the two length bytes.
   //pBuf+=4;
   // JEZ081215: Advance past BOTH the two type bytes AND the one length byte.
   //pBuf+=3;
   for(i=0;i<4;i++)
   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: *pBuf before advancepTmp[%x] = %x\n", i,pTmp[i] );

   pBuf+=CMD_TLV_TYPE_AND_LEN_SIZE;

   Count -= CMD_TLV_TYPE_AND_LEN_SIZE;

   switch ( cmdOpcode )
   {
      /** BT v3.0 Link Control commands */
      case BTAMP_TLV_HCI_CREATE_PHYSICAL_LINK_CMD:
         {
            tBtampTLVHCI_Create_Physical_Link_Cmd CreatePhysicalLinkCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Create_Physical_Link_Cmd( NULL,
                pBuf, Count, &CreatePhysicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Create_Physical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            // setup the per PHY link BAP context
            Status = BslFindAndInitPhyCtx( pctx, CreatePhysicalLinkCmd.phy_link_handle, 
                                             &pPhyCtx );

            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "CreatePhysicalLinkCmd.phy_link_handle=%d\n",CreatePhysicalLinkCmd.phy_link_handle);

            if ( !Status )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: BslFindAndInitPhyCtx failed");
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPPhysicalLinkCreate( pctx->bapHdl, 
                                                    &CreatePhysicalLinkCmd, pPhyCtx, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPPhysicalLinkCreate failed status %d", VosStatus);
               // handle the error
               BslReleasePhyCtx( pPhyCtx );
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pPhyCtx, &HCIEvt, TRUE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               BslReleasePhyCtx( pPhyCtx );
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_ACCEPT_PHYSICAL_LINK_CMD:
         {
            tBtampTLVHCI_Accept_Physical_Link_Cmd AcceptPhysicalLinkCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Accept_Physical_Link_Cmd( NULL,
                pBuf, Count, &AcceptPhysicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Accept_Physical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            // setup the per PHY link BAP context
            Status = BslFindAndInitPhyCtx( pctx, AcceptPhysicalLinkCmd.phy_link_handle,
                                             &pPhyCtx );

            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "AcceptPhysicalLinkCmd.phy_link_handle=%d\n",AcceptPhysicalLinkCmd.phy_link_handle);

            if ( !Status )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: BslFindAndInitPhyCtx failed");
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPPhysicalLinkAccept( pctx->bapHdl, 
                                                    &AcceptPhysicalLinkCmd, pPhyCtx, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPPhysicalLinkAccept failed status %d", VosStatus);
               // handle the error
               BslReleasePhyCtx( pPhyCtx );
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pPhyCtx, &HCIEvt, TRUE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               BslReleasePhyCtx( pPhyCtx );
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_DISCONNECT_PHYSICAL_LINK_CMD:
         {
            tBtampTLVHCI_Disconnect_Physical_Link_Cmd DisconnectPhysicalLinkCmd;
            //Count = Count - 3;//Type and length field lengths are not needed
            pTmp = pBuf;
   for(i=0;i<4;i++)
   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: *pBuf in Disconnect phy link pTmp[%x] = %x\n", i,pTmp[i] );
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Disconnect_Physical_Link_Cmd( NULL,
                pBuf, Count, &DisconnectPhysicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Disconnect_Physical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPPhysicalLinkDisconnect( pctx->bapHdl, 
                                                        &DisconnectPhysicalLinkCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPPhysicalLinkDisconnect failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_CREATE_LOGICAL_LINK_CMD:
         {
            tBtampTLVHCI_Create_Logical_Link_Cmd CreateLogicalLinkCmd;
            Count -= 3; //To send the correct length to unpack event
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Create_Logical_Link_Cmd( NULL,
                pBuf, Count, &CreateLogicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Create_Logical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPLogicalLinkCreate( pctx->bapHdl, 
                                                   &CreateLogicalLinkCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPLogicalLinkCreate failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_ACCEPT_LOGICAL_LINK_CMD:
         {
            tBtampTLVHCI_Accept_Logical_Link_Cmd AcceptLogicalLinkCmd;
            Count = Count - 3;//Subtract Type and Length fields
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Accept_Logical_Link_Cmd( NULL,
                pBuf, Count, &AcceptLogicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Accept_Logical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPLogicalLinkAccept( pctx->bapHdl, 
                                                   &AcceptLogicalLinkCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPLogicalLinkAccept failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_DISCONNECT_LOGICAL_LINK_CMD:
         {
            tBtampTLVHCI_Disconnect_Logical_Link_Cmd DisconnectLogicalLinkCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Disconnect_Logical_Link_Cmd( NULL,
                pBuf, Count, &DisconnectLogicalLinkCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Disconnect_Logical_Link_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPLogicalLinkDisconnect( pctx->bapHdl, 
                                                       &DisconnectLogicalLinkCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPLogicalLinkDisconnect failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_LOGICAL_LINK_CANCEL_CMD:
         {
            tBtampTLVHCI_Logical_Link_Cancel_Cmd LogicalLinkCancelCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Logical_Link_Cancel_Cmd( NULL,
                pBuf, Count, &LogicalLinkCancelCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Logical_Link_Cancel_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPLogicalLinkCancel( pctx->bapHdl, 
                                                   &LogicalLinkCancelCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPLogicalLinkCancel failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_FLOW_SPEC_MODIFY_CMD:
         {
            tBtampTLVHCI_Flow_Spec_Modify_Cmd FlowSpecModifyCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Flow_Spec_Modify_Cmd( NULL,
                pBuf, Count, &FlowSpecModifyCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Flow_Spec_Modify_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPFlowSpecModify( pctx->bapHdl, 
                                                &FlowSpecModifyCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPFlowSpecModify failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /*
           Host Controller and Baseband Commands
         */
      case BTAMP_TLV_HCI_RESET_CMD:
         {
            VosStatus = WLAN_BAPReset( pctx->bapHdl );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReset failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_SET_EVENT_MASK_CMD:
         {
            tBtampTLVHCI_Set_Event_Mask_Cmd SetEventMaskCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Set_Event_Mask_Cmd( NULL,
                pBuf, Count, &SetEventMaskCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Set_Event_Mask_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPSetEventMask( pctx->bapHdl, 
                                              &SetEventMaskCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPSetEventMask failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_FLUSH_CMD:
         {
            tBtampTLVHCI_Flush_Cmd FlushCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Flush_Cmd( NULL,
                pBuf, Count, &FlushCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Flush_Cmd failed status %d", UnpackStatus); 
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPFlush( pctx->bapHdl, &FlushCmd );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPFlush failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_CONNECTION_ACCEPT_TIMEOUT_CMD:
         {
            VosStatus = WLAN_BAPReadConnectionAcceptTimeout( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadConnectionAcceptTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_CONNECTION_ACCEPT_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Write_Connection_Accept_Timeout_Cmd WriteConnectionAcceptTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Connection_Accept_Timeout_Cmd( NULL,
                pBuf, Count, &WriteConnectionAcceptTimeoutCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Connection_Accept_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteConnectionAcceptTimeout( pctx->bapHdl, 
                                                              &WriteConnectionAcceptTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteConnectionAcceptTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LINK_SUPERVISION_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Read_Link_Supervision_Timeout_Cmd ReadLinkSupervisionTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Link_Supervision_Timeout_Cmd( NULL,
                pBuf, Count, &ReadLinkSupervisionTimeoutCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Link_Supervision_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadLinkSupervisionTimeout( pctx->bapHdl, 
                                                            &ReadLinkSupervisionTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLinkSupervisionTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_LINK_SUPERVISION_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Write_Link_Supervision_Timeout_Cmd WriteLinkSupervisionTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Link_Supervision_Timeout_Cmd( NULL,
                pBuf, Count, &WriteLinkSupervisionTimeoutCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Link_Supervision_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteLinkSupervisionTimeout( pctx->bapHdl, 
                                                             &WriteLinkSupervisionTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteLinkSupervisionTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /* v3.0 Host Controller and Baseband Commands */
      case BTAMP_TLV_HCI_READ_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD:
         {
            VosStatus = WLAN_BAPReadLogicalLinkAcceptTimeout( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLogicalLinkAcceptTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_LOGICAL_LINK_ACCEPT_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Write_Logical_Link_Accept_Timeout_Cmd WriteLogicalLinkAcceptTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Logical_Link_Accept_Timeout_Cmd( NULL,
                pBuf, Count, &WriteLogicalLinkAcceptTimeoutCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Logical_Link_Accept_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteLogicalLinkAcceptTimeout( pctx->bapHdl, 
                                                               &WriteLogicalLinkAcceptTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteLogicalLinkAcceptTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_SET_EVENT_MASK_PAGE_2_CMD:
         {
            tBtampTLVHCI_Set_Event_Mask_Page_2_Cmd SetEventMaskPage2Cmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Set_Event_Mask_Page_2_Cmd( NULL,
                pBuf, Count, &SetEventMaskPage2Cmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Set_Event_Mask_Page_2_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPSetEventMaskPage2( pctx->bapHdl, 
                                                   &SetEventMaskPage2Cmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPSetEventMaskPage2 failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LOCATION_DATA_CMD:
         {
            VosStatus = WLAN_BAPReadLocationData( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLocationData failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_LOCATION_DATA_CMD:
         {
            tBtampTLVHCI_Write_Location_Data_Cmd WriteLocationDataCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Location_Data_Cmd( NULL,
                pBuf, Count, &WriteLocationDataCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Location_Data_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteLocationData( pctx->bapHdl, 
                                                   &WriteLocationDataCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteLocationData failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_FLOW_CONTROL_MODE_CMD:
         {
            VosStatus = WLAN_BAPReadFlowControlMode( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadFlowControlMode failed status %d", VosStatus); 
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_FLOW_CONTROL_MODE_CMD:
         {
            tBtampTLVHCI_Write_Flow_Control_Mode_Cmd WriteFlowControlModeCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Flow_Control_Mode_Cmd( NULL,
                pBuf, Count, &WriteFlowControlModeCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Flow_Control_Mode_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteFlowControlMode( pctx->bapHdl, 
                                                      &WriteFlowControlModeCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteFlowControlMode failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_BEST_EFFORT_FLUSH_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Read_Best_Effort_Flush_Timeout_Cmd ReadBestEffortFlushTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Best_Effort_Flush_Timeout_Cmd( NULL,
                pBuf, Count, &ReadBestEffortFlushTimeoutCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Best_Effort_Flush_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadBestEffortFlushTimeout( pctx->bapHdl, 
                                                            &ReadBestEffortFlushTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadBestEffortFlushTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_BEST_EFFORT_FLUSH_TIMEOUT_CMD:
         {
            tBtampTLVHCI_Write_Best_Effort_Flush_Timeout_Cmd WriteBestEffortFlushTimeoutCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Best_Effort_Flush_Timeout_Cmd( NULL,
                pBuf, Count, &WriteBestEffortFlushTimeoutCmd);

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Best_Effort_Flush_Timeout_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteBestEffortFlushTimeout( pctx->bapHdl, 
                                                             &WriteBestEffortFlushTimeoutCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteBestEffortFlushTimeout failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /** opcode definition for this command from AMP HCI CR D9r4 markup */
      case BTAMP_TLV_HCI_SET_SHORT_RANGE_MODE_CMD:
         {
            tBtampTLVHCI_Set_Short_Range_Mode_Cmd SetShortRangeModeCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Set_Short_Range_Mode_Cmd( NULL,
                pBuf, Count, &SetShortRangeModeCmd);

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Set_Short_Range_Mode_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPSetShortRangeMode( pctx->bapHdl, 
                                                   &SetShortRangeModeCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPSetShortRangeMode failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /* End of v3.0 Host Controller and Baseband Commands */
         /*
            Informational Parameters
         */
      case BTAMP_TLV_HCI_READ_LOCAL_VERSION_INFO_CMD:
         {
            VosStatus = WLAN_BAPReadLocalVersionInfo( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLocalVersionInfo failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LOCAL_SUPPORTED_CMDS_CMD:
         {
            VosStatus = WLAN_BAPReadLocalSupportedCmds( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLocalSupportedCmds failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_BUFFER_SIZE_CMD:
         {
            VosStatus = WLAN_BAPReadBufferSize( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadBufferSize failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /* v3.0 Informational commands */
      case BTAMP_TLV_HCI_READ_DATA_BLOCK_SIZE_CMD:
         {
            VosStatus = WLAN_BAPReadDataBlockSize( pctx->bapHdl, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadDataBlockSize failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /*
           Status Parameters
         */
      case BTAMP_TLV_HCI_READ_FAILED_CONTACT_COUNTER_CMD:
         {
            tBtampTLVHCI_Read_Failed_Contact_Counter_Cmd ReadFailedContactCounterCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Failed_Contact_Counter_Cmd( NULL,
                pBuf, Count, &ReadFailedContactCounterCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Failed_Contact_Counter_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadFailedContactCounter( pctx->bapHdl, 
                                                          &ReadFailedContactCounterCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadFailedContactCounter failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_RESET_FAILED_CONTACT_COUNTER_CMD:
         {
            tBtampTLVHCI_Reset_Failed_Contact_Counter_Cmd ResetFailedContactCounterCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Reset_Failed_Contact_Counter_Cmd( NULL,
                pBuf, Count, &ResetFailedContactCounterCmd);

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Reset_Failed_Contact_Counter_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPResetFailedContactCounter( pctx->bapHdl, 
                                                           &ResetFailedContactCounterCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPResetFailedContactCounter failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LINK_QUALITY_CMD:
         {
            tBtampTLVHCI_Read_Link_Quality_Cmd ReadLinkQualityCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Link_Quality_Cmd( NULL,
                pBuf, Count, &ReadLinkQualityCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Link_Quality_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadLinkQuality( pctx->bapHdl, 
                                                 &ReadLinkQualityCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLinkQuality failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_RSSI_CMD:
         {
            tBtampTLVHCI_Read_RSSI_Cmd ReadRssiCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_RSSI_Cmd( NULL,
                pBuf, Count, &ReadRssiCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_RSSI_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadRSSI( pctx->bapHdl, 
                                          &ReadRssiCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadRSSI failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LOCAL_AMP_INFORMATION_CMD:
         {
            tBtampTLVHCI_Read_Local_AMP_Information_Cmd ReadLocalAmpInformationCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Local_AMP_Information_Cmd( NULL,
                pBuf, Count, &ReadLocalAmpInformationCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Local_AMP_Information_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadLocalAMPInfo( pctx->bapHdl, 
                                                  &ReadLocalAmpInformationCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLocalAMPInfo failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_READ_LOCAL_AMP_ASSOC_CMD:
         {
            tBtampTLVHCI_Read_Local_AMP_Assoc_Cmd ReadLocalAmpAssocCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Local_AMP_Assoc_Cmd( NULL,
                pBuf, Count, &ReadLocalAmpAssocCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Local_AMP_Assoc_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadLocalAMPAssoc( pctx->bapHdl, 
                                                   &ReadLocalAmpAssocCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLocalAMPAssoc failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_REMOTE_AMP_ASSOC_CMD:
         {
            tBtampTLVHCI_Write_Remote_AMP_ASSOC_Cmd WriteRemoteAmpAssocCmd;
            // unpack

            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: HCI_Write_Remote_AMP_ASSOC_Cmd Count = %d", Count);
			DUMPLOG(1, __FUNCTION__, "HCI_Write_Remote_AMP_ASSOC cmd",  
                    pBuf, 
                    Count);

            UnpackStatus = btampUnpackTlvHCI_Write_Remote_AMP_ASSOC_Cmd( NULL,
                pBuf, Count, &WriteRemoteAmpAssocCmd );

            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WriteRemoteAmpAssocCmd.amp_assoc_remaining_length = %d", 
                        WriteRemoteAmpAssocCmd.amp_assoc_remaining_length
                        );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Remote_AMP_ASSOC_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

//#define BAP_UNIT_TEST
#ifdef BAP_UNIT_TEST
            {
            unsigned char test_amp_assoc_fragment[] = 
            { 0x01, 0x00, 0x06, 0x00, 0x00, 0xde, 0xad, 0xbe, 
              0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
              0x0c, 0x00, 0x55, 0x53, 0x20, 0xc9, 0x0c, 0x00, 
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
              0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x55, 0x53, 
              0x20, 0xc9, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
              0x04, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 
              0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 
              0x00, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
              0x00, 0x00 };
            WriteRemoteAmpAssocCmd.present = 1;
            WriteRemoteAmpAssocCmd.phy_link_handle = 1;
            WriteRemoteAmpAssocCmd.length_so_far = 0;
            WriteRemoteAmpAssocCmd.amp_assoc_remaining_length = 74;
            /* Set the amp_assoc_fragment to the right values of MAC addr and
             * channels
             */ 
            vos_mem_copy( 
                    WriteRemoteAmpAssocCmd.amp_assoc_fragment,
                    test_amp_assoc_fragment, 
                    sizeof( test_amp_assoc_fragment)); 

            }
#endif

            VosStatus = WLAN_BAPWriteRemoteAMPAssoc( pctx->bapHdl, 
                                                     &WriteRemoteAmpAssocCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteRemoteAMPAssoc failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
         /*
           Debug Commands
         */
      case BTAMP_TLV_HCI_READ_LOOPBACK_MODE_CMD:
         {
            tBtampTLVHCI_Read_Loopback_Mode_Cmd ReadLoopbackModeCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Read_Loopback_Mode_Cmd( NULL,
                pBuf, Count, &ReadLoopbackModeCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Read_Loopback_Mode_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPReadLoopbackMode( pctx->bapHdl, 
                                                  &ReadLoopbackModeCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPReadLoopbackMode failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      case BTAMP_TLV_HCI_WRITE_LOOPBACK_MODE_CMD:
         {
            tBtampTLVHCI_Write_Loopback_Mode_Cmd WriteLoopbackModeCmd;
            // unpack
            UnpackStatus = btampUnpackTlvHCI_Write_Loopback_Mode_Cmd( NULL, 
                pBuf, Count, &WriteLoopbackModeCmd );

            if ( !BTAMP_SUCCEEDED( UnpackStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: btampUnpackTlvHCI_Write_Loopback_Mode_Cmd failed status %d", UnpackStatus);
               // handle the error
               return(FALSE);
            }

            VosStatus = WLAN_BAPWriteLoopbackMode( pctx->bapHdl, 
                                                   &WriteLoopbackModeCmd, &HCIEvt );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLAN_BAPWriteLoopbackMode failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            // this may look strange as this is the function registered
            // with BAP for the EventCB but we are also going to use it
            // as a helper function. The difference is that this invocation
            // runs in HCI command sending caller context while the callback
            // will happen in BAP's context whatever that may be
            VosStatus = WLANBAP_EventCB( pctx, &HCIEvt, FALSE );

            if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
            {
               VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BslProcessHCICommand: WLANBAP_EventCB failed status %d", VosStatus);
               // handle the error
               return(FALSE);
            }

            break;
         }
      default:
         {
            return(FALSE);
            break;
         }
   }

   return(TRUE);
}

/**
  @brief BslFindPhyCtx() - This function will find the physical link a.k.a assocation
  context using a phy_link_handle from an egress ACL data packet
 
  @param pctx : [in] ptr to the client context
  @param PhyLinkHdl : [in] the phy_link_handle from the ACL data packet
  @packet ppPhyCtx ; [inout] ptr to the physical link a.k.a assocation context

  @return 
  TRUE if all OK, FALSE otherwise

*/
static BOOL BslFindPhyCtx
   (
   BslClientCtxType* pctx, 
   v_U8_t PhyLinkHdl,
   BslPhyLinkCtxType** ppPhyCtx
   )
{
   // no need for an entry trace here

   // TODO: if a packet sneaks by i.e. as the phy link context is being invalidated
   // then we will rely on BAP to flag an error. Alternatively, access the map in a race 
   // free manner (but going with former for now)
   *ppPhyCtx = BslPhyLinkMap[PhyLinkHdl].ptr;

   if ( *ppPhyCtx && 
       ( (*ppPhyCtx)->pClientCtx == pctx ) )
   {
      return(TRUE);
   }
   else
   {
      return(FALSE);
   }
}

/**
  @brief BslProcessACLDataTx() - This function will process an egress ACL data packet 
 
  @param pctx : [in] ptr to the client context
  @param pBuffer_ : [in] ptr to the buffer containing the ACL data packet
  @param pCount : [in] size of the ACL data packet buffer

  @return 
  TRUE if all OK, FALSE otherwise

*/
static BOOL BslProcessACLDataTx
   (
   BslClientCtxType* pctx,
   LPCVOID pBuffer_,
   v_SIZE_t* pCount
   )
{
   LPVOID pBuffer = (LPVOID) pBuffer_; // castaway const-ness of ptr
   BOOL findPhyStatus;
   BslPhyLinkCtxType* pPhyCtx;
   VOS_STATUS VosStatus;
   WLANTL_ACEnumType Ac;
   vos_pkt_t* pVosPkt;
   vos_list_node_t* pLink;
   BslTxListNodeType *pNode;
   v_SIZE_t ListSize;
   WLANTL_MetaInfoType TlMetaInfo;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_LOW, "BslProcessACLDataTx\n" );

   // need to find the PHY link for this ACL data pkt based on phy_link_handle
   // TODO need some endian-ness check?
   findPhyStatus = BslFindPhyCtx( pctx, *(v_U8_t *)pBuffer, &pPhyCtx );

   if ( findPhyStatus )
   {
      // create a VOS pkt out of the data, Ok this is rather UGLY having to use
      // something called a RX raw packet for TX data but lets get it to work first,
      // not using the low resource callback for now
      VosStatus = vos_pkt_get_packet( &pVosPkt, VOS_PKT_TYPE_RX_RAW, *pCount, 1, 
                                      0, NULL, NULL);

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx vos_pkt_get_packet \
             failed status =%d\n", VosStatus );

         return(FALSE);
      }

      VosStatus = vos_pkt_push_head( pVosPkt, pBuffer, *pCount );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx vos_pkt_push_head \
             failed status =%d\n", VosStatus );

         // return the packet 
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         return(FALSE);
      }

      VosStatus = WLANBAP_XlateTxDataPkt( pctx->bapHdl, pPhyCtx->PhyLinkHdl, 
                                          &Ac, &TlMetaInfo, pVosPkt);

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx WLANBAP_XlateTxDataPkt \
             failed status =%d\n", VosStatus );

         // return the packet 
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         return(FALSE);
      }

      // sanitize Ac
      if ( Ac >= WLANTL_MAX_AC )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx bad AC =%d\n", Ac );

         // return the packet 
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         return(FALSE);
      }

      // check if queue is already at capped size
      VosStatus = vos_list_size( &pPhyCtx->ACLTxQueue[Ac], &ListSize );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_ASSERT(0);
      }

      if ( ListSize == BSL_MAX_SIZE_TX_ACL_QUEUE )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx \
             vos_list_size ==%d\n", ListSize );

         // return the packet 
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         return(FALSE);
      }

      // get a pkt desc
      VosStatus = vos_list_remove_front( &BslTxPktDescPool, &pLink );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx vos_list_remove_front \
             failed status =%d\n", VosStatus );

         // return the packet 
         VosStatus = vos_pkt_return_packet( pVosPkt );
         VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

         return(FALSE);
      }

      // stick the VOS pkt into the node
      pNode = (BslTxListNodeType *) pLink;
      pNode->pVosPkt = pVosPkt;
      // cache the meta-info BAP returns for use during BAP fetch i.e. TL fetch
      pNode->TlMetaInfo = TlMetaInfo;

      // now queue the pkt into the correct queue
      VosStatus = vos_list_insert_back( &pPhyCtx->ACLTxQueue[Ac], pLink );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_ASSERT(0);
      }

      // determine if there is a need to signal TL through BAP
      VosStatus = vos_list_size( &pPhyCtx->ACLTxQueue[Ac], &ListSize );

      if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         VOS_ASSERT(0);
      }

      if ( ListSize == 1 )
      {
         // Let TL know we have a packet to send for this AC
         VosStatus = WLANBAP_STAPktPending( pctx->bapHdl, pPhyCtx->PhyLinkHdl, Ac );      

         if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
         {
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx WLANBAP_STAPktPending \
                failed status =%d\n", VosStatus );
            VOS_ASSERT(0);
         }
      }

      return(TRUE);
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BslProcessACLDataTx attempting to send \
          data for a non-existant assocation\n" );

      return(FALSE);
   }
}

/*--------------------------------------------------------------------------- 
 *   Function definitions
 *-------------------------------------------------------------------------*/ 

/**---------------------------------------------------------------------------
  
  \brief BSL_Init() - Initialize the BSL Misc char driver
  
  This is called in by WLANBAP_Open() as part of bringing up the BT-AMP PAL (BAP)
  WLANBAP_Open() will pass in the device context created.
  
  \param  - NA
  
  \return - 0 for success non-zero for failure
              
  --------------------------------------------------------------------------*/
int BSL_Init (void *pCtx)
{
   int err = 0;
	
   //Register as a Misc char driver o
   err = misc_register(&bsl_miscdevice);
	if (err) {
		VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, 
                "Unable to register bsl driver, err=%d\n", err);
		return -1;
	}

   // Save away the btamp context
   gpCtx = pCtx;

   return 0;
}

/**---------------------------------------------------------------------------
  
  \brief BSL_Deinit() - De-initialize the BSL Misc char driver
  
  This is called in by WLANBAP_Close() as part of bringing down the BT-AMP PAL (BAP)
  
  \param  - NA
  
  \return - 0 for success non-zero for failure
              
  --------------------------------------------------------------------------*/

int BSL_Deinit(void)
{
   int err = 0;
	
   //Deregister as a Misc char driver o
   err = misc_deregister(&bsl_miscdevice);
	if (err) {
		VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, 
                "Unable to de-register bsl driver, err=%d\n", err);
		return -1;
	}

   return 0;
}


/**
  @brief BSL_Open() - This function opens a device for reading, and writing. 
  An application indirectly invokes this function when it calls the fopen() 
  system call to open a special device file names.  

  @param *pInode : [in] pointer to the inode structure for this device file.
  @param *pFile : [in] pointer to the file structure for this device file.
  We are going to set file->private_data to hold the BslClientCtxType. 

  @return
  This function returns a status code.  Negative codes are failures.
*/
static int BSL_Open (struct inode *pInode, struct file *pFile)
{
   VOS_STATUS VosStatus;
   BslClientCtxType* pctx = 0;
   v_U16_t i;
   BOOL rval;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Open");

   /* We are going to set file->private_data to hold the BSL context */
   /*  you can only open a btamp device one time */
   if (pFile->private_data != NULL)
      return -EPERM; /* Operation not permitted */

   VosStatus = vos_lock_init(&BslClientLock);			

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSLClientLock already inited");
     // return -EIO;	/* I/O error */
        return 0; 
   }
   VosStatus = vos_lock_init(&BslPhyLock);

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VosStatus = vos_lock_destroy(&BslClientLock);
      // if this fails, nothing we can do
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

     // return -EIO;	/* I/O error */
        return 0;
   }


   VosStatus = vos_list_init( &BslRxPktDescPool );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSLRxPktDescPool list already inited");
      VosStatus = vos_lock_destroy(&BslPhyLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_lock_destroy(&BslClientLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      //return -EIO;	/* I/O error */
        return 0;   
   }

   VosStatus = vos_list_init( &BslTxPktDescPool );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSLTxPktDescPool list already inited");
      VosStatus = vos_list_destroy( &BslRxPktDescPool );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_lock_destroy(&BslPhyLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_lock_destroy(&BslClientLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      //return -EIO;	/* I/O error */
      return 0;
   }

   VosStatus = vos_list_init( &BslPhyLinksDescPool );

   if ( !VOS_IS_STATUS_SUCCESS( VosStatus ) )
   {
      VosStatus = vos_list_destroy( &BslTxPktDescPool );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_list_destroy( &BslRxPktDescPool );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_lock_destroy(&BslPhyLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      VosStatus = vos_lock_destroy(&BslClientLock);
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

      //return -EIO;	/* I/O error */
        return 0;
   }

   // now we need to populate this pool with the free pkt desc from the array
   for ( i=0; i<BSL_MAX_RX_PKT_DESCRIPTOR; i++ )
   {
      VosStatus = vos_list_insert_front( &BslRxPktDescPool, &BslRxPktDesc[i].node );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   // now we need to populate this pool with the free pkt desc from the array
   for ( i=0; i<BSL_MAX_TX_PKT_DESCRIPTOR; i++ )
   {
      VosStatus = vos_list_insert_front( &BslTxPktDescPool, &BslTxPktDesc[i].node );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   // now we need to populate this pool with the free pkt desc from the array
   for ( i=0; i<BSL_MAX_PHY_LINKS; i++ )
   {
      VosStatus = vos_list_insert_front( &BslPhyLinksDescPool, &BslPhyLinksDesc[i].node );
      VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
   }

   // This is redundent.  See the check above on (fp->private_data != NULL)
   bBslInited = TRUE;


   rval = BslFindAndInitClientCtx( &pctx );

   if(rval != TRUE)
   {
     // Where is the clean-up in case the above BslFindAndInitClientCtx() call 
     // fails?
     //return -EIO;	/* I/O error */
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSLFindAndInitClientContext failed");
       return 0;
   }

   /* We are going to set pFile->private_data to hold the BSL context */
   /* Return the BslClientCtxType pctx */
   pFile->private_data = pctx;


    VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "private_data initialized");
   /* Let Linux fopen() know everything is all right */
   return 0;
}

/**
  @brief BSL_Close() - This function closes a device context created by the 
  hOpenContext parameter

  @param *pInode : [in] pointer to the inode structure for this device file.
  @param *pFile : [in] pointer to the file structure for this device file.
  Which is used to identify the open context of the device.

  @return
  TRUE indicates success. FALSE indicates failure. 
*/
static int BSL_Close (struct inode *pInode, struct file *pFile)
{
   VOS_STATUS VosStatus;
   BslClientCtxType* pctx = (BslClientCtxType *)pFile->private_data;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Close");

   // it may seem there is some risk here because we are using a value
   // passed into us as a pointer. what if this pointer is 0 or points 
   // someplace bad? as it turns out the caller is device manager and not
   // the application. kernel should trap such invalid access but we will check
   // for NULL pointer
   if ( pctx == NULL )
   {
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Close: NULL Open ctx specified"); 
     return FALSE;
   }

   // need to cleanup any per PHY state and the common RX state
   BslReleaseClientCtx( pctx );


   VosStatus = vos_list_destroy( &BslPhyLinksDescPool );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   VosStatus = vos_list_destroy( &BslTxPktDescPool );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   VosStatus = vos_list_destroy( &BslRxPktDescPool );
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   VosStatus = vos_lock_destroy(&BslPhyLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   VosStatus = vos_lock_destroy(&BslClientLock);
   VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );

   bBslInited = FALSE;

   // NULL this out so that the device can be opened a second time.
   pFile->private_data = NULL;

   return(TRUE);
}

/**
  @brief BSL_Read() - This function reads data from the device identified by 
  the context indicated in the file structure.

  @param *pFile : [in] pointer to the file structure for this device file.
  BSL_Open function creates and returns this identifier.
  @param pBuffer : [out] Pointer to the buffer that stores the data read from the 
  device. This buffer should be at least count bytes long.
  @param Count : [in] Number of bytes to read from the device into pBuffer.
  @param *ppos : [in] Pointer to a "long offset type" indicating the file position.

  @return
  Returns zero to indicate end-of-file. Returns -1 to indicate an error. Returns the 
  number of bytes read to indicate success. 
*/
static ssize_t BSL_Read(struct file *pFile, char __user *pBuffer, size_t Count,
			  loff_t *ppos)
{
   BslClientCtxType* pctx = (BslClientCtxType *)pFile->private_data;
   vos_list_node_t* pNode;
   vos_pkt_t* pVosPkt;
   v_SIZE_t ListSize;
   v_SIZE_t BuffSize;
   VOS_STATUS waitRslt;
   VOS_STATUS VosStatus;
   BslRxListNodeType* pRxNode;
   char *bslBuff = NULL;
   char *temp=NULL;
   int cnt;
   v_U16_t packetSize = 0;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Read Count=%d \n",Count);

   // it may seem there is some risk here because we are using a value
   // passed into us as a pointer. what if this pointer is 0 or points 
   // someplace bad? as it turns out the caller is device manager and not
   // the application. kernel should trap such invalid access but we will check
   // for NULL pointer
   if ( pctx == NULL || pBuffer == NULL || Count == 0 )
   {
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Read: bad i/p");
     return -1;
   }

   // yeah, yeah GOTO considered harmful and all that
   start:
     VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "start: label");

     VosStatus = vos_list_size( &pctx->HCIEvtQueue, &ListSize );

   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

   if ( ListSize )
   {
      VosStatus = vos_list_remove_front( &pctx->HCIEvtQueue, &pNode );

      if ( VosStatus == VOS_STATUS_E_EMPTY )
      {
         // this can happen due to re-entrancy, no biggie
           VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"empty HCIEvnt list:\n");   

      }
      else if ( VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         // stick in the special flag to let app know this is an HCI event
         *(v_U8_t *)pBuffer = WLANBAP_HCI_EVENT_PACKET;

         // We have an event
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: HCI_Event: pBuffer = %lx, Count = %d\n", __FUNCTION__, pBuffer, Count);
           
         //pBuffer++;
         BuffSize = Count - 1;

         // extract the VoS pkt from the list node element
         pVosPkt = ((BslRxListNodeType *)pNode)->pVosPkt;
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: pVosPkt extracted from list:%u\n", __FUNCTION__, pVosPkt);

         // extract the RX node descriptor from user data area of VoS pkt 
         vos_pkt_get_user_data_ptr( pVosPkt, VOS_PKT_USER_DATA_ID_BAP, 
                                    (v_VOID_t**)&pRxNode );

         // TODO: this API is weird, if the buffer size is more than the amount of
         // data in the vos pkt it fails!!! a workaround is to pass 0 for the size
         // telling vos we want to copy all the data in the packet into the buffer
         // the catch here is the caller has to ensure there is no overflow, etc. in
         // general that is difficult to guarantee but here we know that BT is
         // guaranteeing to specify a big enough buffer always so we may get away with
         // this
//         BuffSize = 0;

         bslBuff = vos_mem_malloc(Count -1);
         //VosStatus = vos_pkt_extract_data(pVosPkt, 0, (v_U8_t *)(pBuffer + 1), &BuffSize );
         VosStatus = vos_pkt_get_packet_length(pVosPkt , &packetSize);
         VosStatus = vos_pkt_pop_head(pVosPkt,(v_U8_t *)(bslBuff), packetSize );
//         VosStatus = vos_pkt_extract_data(pVosPkt, 0, (v_U8_t *)bslBuff, &BuffSize );
         //if(Count > (BuffSize + 1)){
           //  Count = BuffSize + 1;
         //}              
         if( !VOS_IS_STATUS_SUCCESS(VosStatus) ){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_ERROR,"Extraction from Vos Pkt failed \n");
         }

//#if 0
         if(copy_to_user(pBuffer +1 , bslBuff, Count - 1)){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_ERROR,"copy_to_user failed");
             BuffSize = -EFAULT;
             vos_mem_free(bslBuff);
             bslBuff = NULL;
          }
//#endif         
//#if 0         
         temp = pBuffer + 1;
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_INFO_HIGH,"extracted buffer sz packetSize:%d\n",packetSize);
         for(cnt=0;cnt < 10; cnt++,temp++){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_INFO_HIGH,"user buffer is:temp[%d]=%x\n",cnt,temp[cnt]);
         }
         temp = bslBuff;
         for(cnt=0;cnt < 10; cnt++,temp++){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_INFO_HIGH,"user buffer is:temp[%d]=%x\n",cnt,temp[cnt]);
         }    
//#endif         
          if( bslBuff ){
              vos_mem_free(bslBuff);
              bslBuff = NULL;
          }                         

         // We have read out the data from the VOS PACKET
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: HCI_Event: BuffSize = %d Count-1 = %d\n", __FUNCTION__, BuffSize,Count-1);
           
         // we may not get here if we abort, hehe
         VOS_ASSERT( BuffSize <= Count - 1 );

         // some checks here
         if( !VOS_IS_STATUS_SUCCESS( VosStatus ))
         {
           // stick the VoS pkt back into the queue, expecting BuffSize will be set
           // appropriately.
           VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BSL_Read buffer too small for event\n" );
           
           // ordering may not preserve
           VosStatus = vos_list_insert_front( &pctx->HCIEvtQueue, pNode );
           VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
         }
         else
         {
            // now free the VoS pkt
            VosStatus = vos_pkt_return_packet( pVosPkt );
            VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

            // now free the RX desc
            VosStatus = vos_list_insert_front( &BslRxPktDescPool, &pRxNode->node );
            VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
         }

         // Don't forget.  You tacked on one byte at the beginning.
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "pVosPktning %d bytes\n",(Count) );
         *ppos +=Count;
//         *ppos = 1;
         return(Count);
         //return 0;

      }
      else
      {
         VOS_ASSERT(0);
      }
   }

   VosStatus = vos_list_size( &pctx->ACLRxQueue, &ListSize );

   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( VosStatus ) );

   if ( ListSize )
   {
      VosStatus = vos_list_remove_front( &pctx->ACLRxQueue, &pNode );

      if ( VosStatus == VOS_STATUS_E_EMPTY )
      {
         // this can happen due to re-entrancy, no biggie
           VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"Empty RxQueue:\n");   

      }
      else if ( VOS_IS_STATUS_SUCCESS( VosStatus ) )
      {
         // stick in the special flag to let app know this is an HCI event
         *(v_U8_t *)pBuffer = WLANBAP_HCI_ACL_DATA_PACKET;

         // We have some data
         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "%s: ACL Data Rx Event: pBuffer = %lx, Count = %d\n", __FUNCTION__, pBuffer, Count);
           
         //pBuffer++;
         //BuffSize = Count - 1;

         // extract the VoS pkt from the list node element
         pVosPkt = ((BslRxListNodeType *)pNode)->pVosPkt;

         // extract the RX node descriptor from user data area of VoS pkt 
         vos_pkt_get_user_data_ptr( pVosPkt, VOS_PKT_USER_DATA_ID_BAP, 
                                    (v_VOID_t**)&pRxNode );

         // TODO: this API is weird, if the buffer size is more than the amount of
         // data in the vos pkt it fails!!! a workaround is to pass 0 for the size
         // telling vos we want to copy all the data in the packet into the buffer
         // the catch here is the caller has to ensure there is no overflow, etc. in
         // general that is difficult to guarantee but here we know that BT is
         // guaranteeing to specify a big enough buffer always so we may get away with
         // this
         BuffSize = 0;

         bslBuff = vos_mem_malloc(Count -1);
         VosStatus = vos_pkt_get_packet_length(pVosPkt , &packetSize);
         VosStatus = vos_pkt_pop_head(pVosPkt,(v_U8_t *)(bslBuff), packetSize );
         //VosStatus = vos_pkt_extract_data(pVosPkt, 0, (v_U8_t *)pBuffer + 1, &BuffSize );
         if( !VOS_IS_STATUS_SUCCESS(VosStatus) ){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_ERROR,"Extraction from Vos Pkt failed \n");
         }
         
         if(copy_to_user(pBuffer +1 , bslBuff, Count - 1)){
             VOS_TRACE(VOS_MODULE_ID_BAP,VOS_TRACE_LEVEL_ERROR,"copy_to_user failed");
             BuffSize = -EFAULT;
             vos_mem_free(bslBuff);
             bslBuff = NULL;
          }
          
         if( bslBuff ){
              vos_mem_free(bslBuff);
              bslBuff = NULL;
          }                         

         // we may not get here if we abort, hehe
         //JEZ090116: DM calls BSL_Read with invalid Count. Temporarily comment out.
         //VOS_ASSERT( BuffSize <= Count - 1 );

         // some checks here
         if( !VOS_IS_STATUS_SUCCESS( VosStatus ))
         {
           // stick the VoS pkt back into the queue, expecting BuffSize will be set
           // appropriately
           VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BSL_Read buffer too small for ACL data\n" );
           
           // ordering may not preserve
           VosStatus = vos_list_insert_front( &pctx->ACLRxQueue, pNode );
           VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
         }
         else
         {
            // now free the VoS pkt
            VosStatus = vos_pkt_return_packet( pVosPkt );
            VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ));

            // now free the RX desc
            VosStatus = vos_list_insert_front( &BslRxPktDescPool, &pRxNode->node );
            VOS_ASSERT(VOS_IS_STATUS_SUCCESS( VosStatus ) );
         }

         VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Read returning %d bytes\n",(BuffSize + 1) );
         // Don't forget.  You tacked on one byte at the beginning.
//         return(BuffSize+1);
         return Count;
      }
      else
      {
         VOS_ASSERT(0);
      }
   }

   // JEZ091204: I need to replace the event signalling stuff with VOSS routines.
   // JEZ091204: Where are ALL the places that pctx->ReadableEvt is ref'd

   // if we are here it means no HCI events AND no ACL data, so we block on the
   // "Readable" event
//   waitRslt = WaitForSingleObject(pctx->ReadableEvt, INFINITE);
   VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"Calling vos_wait_single_event");   
   waitRslt = vos_wait_single_event(&(pctx->ReadableEvt), INFINITE);
   VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"After vos_wait_single_event waitRslt:%d\n",waitRslt);   

   switch ( waitRslt )
   {
      //case WAIT_OBJECT_0:
      case VOS_STATUS_SUCCESS:
         {
            // event got signalled, either event or data or both available
           VOS_TRACE(VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH,"goto start:\n");   
            goto start;
            break;
         }
      //case WAIT_FAILED:
      case VOS_STATUS_E_ABORTED:
         {
            // the idea here is that a client is going away by calling BAP_Close
            // and there is a client thread potentially blocked on a Read. Inside
            // BAP_Close, we will invalidate the ReadableEvt which will cause the
            // blocked thread to end up here and we bail out, nice...
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BSL_Read buffer vos_wait_single_event() aborted\n" );
            return(0);
            break;
         }
      // Handle a number of other causes of failure
      case VOS_STATUS_E_TIMEOUT:
         {
            // This shouldn't happen.  Since we asked for an infinite timeout. 
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BSL_Read buffer vos_wait_single_event() timeout\n" );
            VOS_ASSERT(0);
            break;
         }
      case VOS_STATUS_E_FAILURE:
         {
            // This shouldn't happen.  
            VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "BSL_Read buffer vos_wait_single_event() failure\n" );
            // Should I assert?
            //VOS_ASSERT(0);
            return(0);
            break;
         }
      default:
         {
            VOS_ASSERT(0);
            break;
         }
   }

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Read returning -1\n");
   return(-1);
}

/**
  @brief BSL_Write() - This function writes data to the device.
  An application indirectly invokes this function when it calls the fwrite() 
  system call to write to a special device file.  

  @param *pFile : [in] pointer to the file structure for this device file.
  We retrieve the BslClientCtxType struct from fp->private_data. 
  @param *pBuffer : [in] Pointer to the buffer that contains the data to write.
  @param Count : [in] Number of bytes to write from the "buf" buffer into the 
  device.
  @param *pOff : [in] Pointer to a "long offset type" indicating the file position.

  @return
  The number of bytes written indicates success. 
  Negative values indicate various failures. 
*/
static ssize_t BSL_Write(struct file *pFile, const char __user *pBuffer,
			 size_t Count, loff_t *pOff)
{
   BslClientCtxType* pctx = (BslClientCtxType *)pFile->private_data;
   v_SIZE_t written = 0;
   BOOL status;

   VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write Count=%d",Count);

   // Sanity check inputs
   if ( pctx == NULL || pBuffer == NULL || Count == 0 )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: bad i/p");
      return -EFAULT; /* Bad address */
   }

   if ( *(v_U8_t *)pBuffer == WLANBAP_HCI_COMMAND_PACKET )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: HCI command");
      written = 1;
#if 0
      bslBuff = vos_mem_malloc(Count -1);
      if( copy_from_user(bslBuff,(v_U8_t *)pBuffer + 1,Count -1) ){
               written = -EFAULT;
               vos_mem_free(bslBuff);
               bslBuff = NULL;
      }              
#endif      
      // HCI command
      status = BslProcessHCICommand( pctx, (v_U8_t *)pBuffer + 1, Count - 1 );
#if 0 
      status = BslProcessHCICommand( pctx, (v_U8_t *)bslBuff, Count - 1 );
      if ( bslBuff ){
          vos_mem_free(bslBuff);
          bslBuff = NULL;
      }           
#endif      
      if ( status )
      {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: BSlProcessHCICommand returned status TRUE :%d",status);
          //*pOff += Count;
          written += (Count - 1);
      }    
      else {

      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: BSlProcessHCICommand returned status FALSE:%d",status);
      }
   }
   else if ( *(v_U8_t *)pBuffer == WLANBAP_HCI_ACL_DATA_PACKET )
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: HCI ACL data tx");

          written = Count - 1;//Add the type of packet(DATA packet
      // ACL data
      status = BslProcessACLDataTx( pctx, (v_U8_t *)pBuffer + 1, &written );
      if ( status )
      {
          written += Count - 1;//, else status will be FALSE
      }           
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: unknown type");
      // anything else including HCI events and SCO data
      status = FALSE;
   }

   if ( status )
   {
      //return(Count);
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: returning %d bytes",written);
      return(written);
   }
   else
   {
      VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_INFO_HIGH, "BSL_Write: write failed!!!");
      return(-1);
   }
}

VOS_STATUS WLANBAP_SetConfig
(
    WLANBAP_ConfigType *pConfig
)
{
    VOS_STATUS status;
    // sanity checking
    if ( pConfig == NULL )
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_SetConfig bad input\n" );
        return VOS_STATUS_E_FAILURE;
    }
    status = WLAN_BAPSetConfig(gpCtx, pConfig);
    if ( !VOS_IS_STATUS_SUCCESS( status ) )
    {
        VOS_TRACE( VOS_MODULE_ID_BAP, VOS_TRACE_LEVEL_ERROR, "WLANBAP_SetConfig can't set BAP config\n" );
        return VOS_STATUS_E_FAILURE;
    }

    return(VOS_STATUS_SUCCESS);
}