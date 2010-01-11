#ifndef WLAN_QCT_TLI_H
#define WLAN_QCT_TLI_H

/*===========================================================================

               W L A N   T R A N S P O R T   L A Y E R 
                     I N T E R N A L   A P I
                
                   
DESCRIPTION
  This file contains the internal declarations used within wlan transport 
  layer module.
        
  Copyright (c) 2008 QUALCOMM Incorporated. All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when        who    what, where, why
--------    ---    ----------------------------------------------------------
02/02/09    sch     Add Handoff support
12/09/08    lti     Fixes for AMSS compilation 
12/02/08    lti     Fix fo trigger frame generation 
10/31/08    lti     Fix fo TL tx suspend
10/01/08    lti     Merged in fixes from reordering
09/05/08    lti     Fixes following QOS unit testing 
08/06/08    lti     Added QOS support 
07/18/08    lti     Fixes following integration
                    Added frame translation
06/26/08    lti     Fixes following unit testing 
05/05/08    lti     Created module.

===========================================================================*/



/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "vos_packet.h" 
#include "vos_api.h" 
#include "vos_timer.h" 
#include "vos_mq.h" 
#include "vos_list.h"
#include "wlan_qct_bal.h" 

#define STATIC  static

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*Maximum number of TIDs */
#define WLAN_MAX_TID                          8

/*Maximum number of supported stations */
#define WLAN_MAX_STA_COUNT                    5


/*Offset of the OUI field inside the LLC/SNAP header*/
#define WLANTL_LLC_OUI_OFFSET                 3

/*Size of the OUI type field inside the LLC/SNAP header*/
#define WLANTL_LLC_OUI_SIZE                   3

/*Offset of the protocol type field inside the LLC/SNAP header*/
#define WLANTL_LLC_PROTO_TYPE_OFFSET  WLANTL_LLC_OUI_OFFSET +  WLANTL_LLC_OUI_SIZE

/*Size of the protocol type field inside the LLC/SNAP header*/
#define WLANTL_LLC_PROTO_TYPE_SIZE            2

/*802.1x protocol type */
#define WLANTL_LLC_8021X_TYPE            0x888E

/*Length offset inside the AMSDU sub-frame header*/
#define WLANTL_AMSDU_SUBFRAME_LEN_OFFSET     12

/*802.3 header definitions*/
#define  WLANTL_802_3_HEADER_LEN             14

/* Offset of DA field in a 802.3 header*/
#define  WLANTL_802_3_HEADER_DA_OFFSET        0

/*802.11 header definitions - header len without QOS ctrl field*/
#define  WLANTL_802_11_HEADER_LEN            24

/*802.11 header length + QOS ctrl field*/
#define  WLANTL_MPDU_HEADER_LEN              26

/*802.11 header definitions*/
#define  WLANTL_802_11_MAX_HEADER_LEN        40

/*802.11 header definitions - qos ctrl field len*/
#define  WLANTL_802_11_HEADER_QOS_CTL         2

/*802.11 header definitions - ht ctrl field len*/
#define  WLANTL_802_11_HEADER_HT_CTL          4

/* Offset of Addr1 field in a 802.11 header*/
#define  WLANTL_802_11_HEADER_ADDR1_OFFSET    4 

/* Length of an AMSDU sub-frame */
#define TL_AMSDU_SUBFRM_HEADER_LEN           14

/* Length of the LLC header*/
#define WLANTL_LLC_HEADER_LEN   8 

/*As per 802.11 spec */
#define WLANTL_MGMT_FRAME_TYPE       0x00 
#define WLANTL_CTRL_FRAME_TYPE       0x10
#define WLANTL_DATA_FRAME_TYPE       0x20

/*Value of the data type field in the 802.11 frame */
#define WLANTL_80211_DATA_TYPE         0x02
#define WLANTL_80211_DATA_QOS_SUBTYPE  0x08
#define WLANTL_80211_NULL_QOS_SUBTYPE  0x0C


/*-------------------------------------------------------------------------
  Libra specific defines
-------------------------------------------------------------------------*/

/*Libra PDU size*/
#define WLANTL_PDU_RES_SIZE                 124

/*Minimum resources needed - arbitrary*/

/*DXE + SD*/
#define WLAN_LIBRA_HEADER_LEN              20+8

/*The lenght of the tx BD header*/
#define WLAN_LIBRA_BD_HEADER_LEN            128 

#define WLANTL_MAX_MSDU                    1538

#define WLANTL_MIN_RES_MF                          13 /*Keeping for MF*/
#define WLANTL_MIN_RES_BAP    WLANTL_MIN_RES_MF  + 13 /*Another for BAP*/
#define WLANTL_MIN_RES_DATA   WLANTL_MIN_RES_BAP + 13 /*Min 12 for data*/
#define WLANTL_TH_RES_DATA                        100
/*-------------------------------------------------------------------------
  BT-AMP related definition - !!! should probably be moved to BT-AMP header
---------------------------------------------------------------------------*/

/*BT-AMP packet of type data*/
#define WLANTL_BT_AMP_TYPE_DATA       0x0001

/*BT-AMP packet of type activity report*/
#define WLANTL_BT_AMP_TYPE_AR         0x0002

/*BT-AMP packet of type security frame*/
#define WLANTL_BT_AMP_TYPE_SEC        0x0003


/*-------------------------------------------------------------------------
  Helper macros
---------------------------------------------------------------------------*/
 /*Checks STA index validity*/
#define WLANTL_STA_ID_INVALID( _staid )( _staid >= WLAN_MAX_STA_COUNT ) 

/*As per Libra behavior */
#define WLANTL_STA_ID_BCAST     0xFF 

/*Checks TID validity*/
#define WLANTL_TID_INVALID( _tid )     ( _tid >= WLAN_MAX_TID ) 

/*Checks AC validity*/
#define WLANTL_AC_INVALID( _tid )     ( _tid >= WLANTL_MAX_AC ) 

/*Determines the addr field offset based on the frame xtl bit*/
#define WLANTL_MAC_ADDR_ALIGN( _dxtl )                                    \
      ( ( 0 == _dxtl ) ?                              \
        WLANTL_802_3_HEADER_DA_OFFSET: WLANTL_802_11_HEADER_ADDR1_OFFSET )

/*Determines the header len based on the disable xtl field*/
#define WLANTL_MAC_HEADER_LEN( _dxtl)                                     \
      ( ( 0 == _dxtl )?                               \
         WLANTL_802_3_HEADER_LEN:WLANTL_802_11_HEADER_LEN )

/*Determines the necesary length of the BD header - in case 
  UMA translation is enabled enough room needs to be left in front of the
  packet for the 802.11 header to be inserted*/
#define WLANTL_BD_HEADER_LEN( _dxtl )                                    \
      ( ( 0 == _dxtl )?                               \
         (WLANHAL_TX_BD_HEADER_SIZE+WLANTL_802_11_MAX_HEADER_LEN): WLANHAL_TX_BD_HEADER_SIZE ) 


#define WLAN_TL_CEIL( _a, _b)  (( 0 != (_a)%(_b))? (_a)/(_b) + 1: (_a)/(_b)) 

/*get TL control block from vos global context */
#define VOS_GET_TL_CB(_pvosGCtx) \
        (WLANTL_CbType*)vos_get_context( VOS_MODULE_ID_TL, _pvosGCtx)

/*---------------------------------------------------------------------------
  TL signals for TX thread 
---------------------------------------------------------------------------*/      
typedef enum
{
  /*Suspend signal - following serialization of a HAL suspend request*/
  WLANTL_TX_SIG_SUSPEND = 0,

  /*Res need signal - triggered when all pending TxComp have been received 
   and TL is low on resources*/
  WLANTL_TX_RES_NEEDED  = 1,

  WLANTL_TX_MAX
}WLANTL_TxSignalsType;

/*---------------------------------------------------------------------------
  STA Event type
---------------------------------------------------------------------------*/      
typedef enum
{
  /* Transmit frame event */
  WLANTL_TX_EVENT = 0,

  /* Transmit frame event when U-APSD is enabled for AC*/
  WLANTL_TX_ON_UAPSD_EVENT = 1,

  /* Receive frame event */
  WLANTL_RX_EVENT = 2,

  /* Receive frame event when U-APSD is enabled for AC*/
  WLANTL_RX_ON_UAPSD_EVENT = 3,

  WLANTL_MAX_EVENT 
}WLANTL_STAEventType;

/*---------------------------------------------------------------------------

  DESCRIPTION 
    State machine used by transport layer for receiving or transmitting 
    packets. 
    
  PARAMETERS 

   IN
   pAdapter:        pointer to the global adapter context; a handle to TL's 
                    control block can be extracted from its context 
   ucSTAId:         identifier of the station being processed 
   vosDataBuff:    pointer to the tx/rx vos buffer
   
  RETURN VALUE
    The result code associated with performing the operation  

---------------------------------------------------------------------------*/
typedef VOS_STATUS (*WLANTL_STAFuncType)( v_PVOID_t     pAdapter,
                                          v_U8_t        ucSTAId,
                                          vos_pkt_t**   pvosDataBuff);

/*---------------------------------------------------------------------------
  STA FSM Entry type
---------------------------------------------------------------------------*/      
typedef struct
{
  WLANTL_STAFuncType  pfnSTATbl[WLANTL_MAX_EVENT];
} WLANTL_STAFsmEntryType;

/* Receive in connected state - only EAPOL*/
VOS_STATUS WLANTL_STARxConn( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* Transmit in connected state - only EAPOL*/
VOS_STATUS WLANTL_STATxConn( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* Receive in authenticated state - all data allowed*/
VOS_STATUS WLANTL_STARxAuth( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* Receive in authenticated state on UAPSD - all data allowed*/
VOS_STATUS WLANTL_STARxAuthUAPSD( v_PVOID_t     pAdapter,
                                  v_U8_t        ucSTAId,
                                  vos_pkt_t**   pvosDataBuff );

/* Transmit in authenticated state - all data allowed*/
VOS_STATUS WLANTL_STATxAuth( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* Transmit in authenticated state - all data allowed*/
VOS_STATUS WLANTL_STATxAuthUAPSD( v_PVOID_t     pAdapter,
                                  v_U8_t        ucSTAId,
                                  vos_pkt_t**   pvosDataBuff );

/* Receive in disconnected state - no data allowed*/
VOS_STATUS WLANTL_STARxDisc( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* Transmit in disconnected state - no data allowed*/
VOS_STATUS WLANTL_STATxDisc( v_PVOID_t     pAdapter,
                             v_U8_t        ucSTAId,
                             vos_pkt_t**   pvosDataBuff );

/* TL State Machine */
STATIC const WLANTL_STAFsmEntryType tlSTAFsm[WLANTL_STA_MAX_STATE] = 
{
  /* WLANTL_STA_INIT */
  { {
    NULL,      /* WLANTL_TX_EVENT - no packets should get transmitted*/
    NULL,      /* WLANTL_TX_ON_UAPSD_EVENT - same as above*/
    NULL,      /* WLANTL_RX_EVENT - no packets should be received - drop*/
    NULL,      /* WLANTL_RX_ON_UAPSD_EVENT - same as above*/
  } },

  /* WLANTL_STA_CONNECTED */
  { {
    WLANTL_STATxConn,      /* WLANTL_TX_EVENT - only EAPoL frames are allowed*/
    WLANTL_STATxConn,      /* WLANTL_TX_ON_UAPSD_EVENT - same as above; 
                              no distinction will be made for UAPSD*/
    WLANTL_STARxConn,      /* WLANTL_RX_EVENT - only EAPoL frames can be rx*/
    WLANTL_STARxConn,      /* WLANTL_RX_ON_UAPSD_EVENT - same as above; 
                              no distinction will be made for UAPSD*/
  } },

  /* WLANTL_STA_AUTHENTICATED */
  { {
    WLANTL_STATxAuth,      /* WLANTL_TX_EVENT - all data frames allowed*/
    WLANTL_STATxAuthUAPSD, /* WLANTL_TX_ON_UAPSD_EVENT - all data frames can 
                        be tx in addition trigger frames can be tx-ed*/
    WLANTL_STARxAuth,      /* WLANTL_RX_EVENT - all data frames can be rx */
    WLANTL_STARxAuthUAPSD, /* WLANTL_RX_ON_UAPSD_EVENT - all data frames can 
                        be rx in addition Serv Int timer will be restarted*/
  } },

  /* WLANTL_STA_DISCONNECTED */
  { {
    WLANTL_STATxDisc,      /* WLANTL_TX_EVENT - do nothing */
    WLANTL_STATxDisc,      /* WLANTL_TX_ON_UAPSD_EVENT - do nothing */
    WLANTL_STARxDisc,      /* WLANTL_RX_EVENT - frames will still be fwd-ed*/
    WLANTL_STARxDisc,      /* WLANTL_RX_ON_UAPSD_EVENT - frames will still be 
                              fwd-ed*/
  } }
};

/*---------------------------------------------------------------------------
  Reordering information
---------------------------------------------------------------------------*/      

#define WLANTL_MAX_WINSIZE      64
#define WLANTL_MAX_BA_SESSION   8

typedef struct
{
   v_BOOL_t     isAvailable;
   v_PVOID_t    arrayBuffer[WLANTL_MAX_WINSIZE];
} WLANTL_REORDER_BUFFER_T;


/* To handle Frame Q aging, timer is needed
 * After timer expired, Qed frames have to be routed to upper layer
 * WLANTL_TIMER_EXPIER_UDATA_T is user data type for timer callback
 */
typedef struct
{
   /* Global contect, HAL, HDD need this */
   v_PVOID_t          pAdapter;

   /* TL context handle */
   v_PVOID_t          pTLHandle;

   /* Current STAID, to know STA context */
   v_U32_t            STAID;

   v_U8_t             TID;
} WLANTL_TIMER_EXPIER_UDATA_T;

typedef struct
{
  /*specifies if re-order session exists*/
  v_U8_t             ucExists;

  /* Current Index */
  v_U32_t             ucCIndex; 

  /* Count of the total packets in list*/
  v_U16_t            usCount;

  /* vos ttimer to handle Qed frames aging */
  vos_timer_t        agingTimer;

  /* Q windoe size */
  v_U32_t            winSize;

  /* Available RX frame buffer size */
  v_U32_t            bufferSize;

  /* Start Sequence number */
  v_U32_t            SSN;

  /* BA session ID, generate by HAL */
  v_U32_t            sessionID;

  v_U32_t            currentESN;

  v_U32_t            pendingFramesCount;

  vos_lock_t         reorderLock;

  /* Aging timer callback user data */
  WLANTL_TIMER_EXPIER_UDATA_T timerUdata;

  WLANTL_REORDER_BUFFER_T     *reorderBuffer;
}WLANTL_BAReorderType;


/*---------------------------------------------------------------------------
  UAPSD information
---------------------------------------------------------------------------*/      
typedef struct
{
  /*specifies if re-order session exists*/
  v_U8_t             ucExists;

  /*Service interval timer*/
  vos_timer_t        vosServiceTimer; 

  /*Suspend interval timer*/
  vos_timer_t        vosSuspendTimer; 

  /*Delayed interval timer*/
  vos_timer_t        vosDelayedTimer; 

  /*Delayed mode flag*/
  v_U8_t             ucDelayedMode;

  /*Service interval*/
  v_U32_t            uServiceInterval;

  /*Suspend interval*/
  v_U32_t            uSuspendInterval;

  /*Delayed interval*/
  v_U32_t            uDelayedInterval;

  /* TSpec direction*/
  WLANTL_TSDirType   wTSpecDir;

  /* Station Id for which the UAPSD has been setup*/
  v_U8_t             ucStaId; 

  /* Access Category for which the UAPSD has been setup*/
  WLANTL_ACEnumType   ucAC;

  /* TSpec Id for which the UAPSD has been setup*/
  v_U8_t              ucTid;

  /* UP to be used in trigger frame*/
  v_U8_t              ucUP;

  /* Pointer to the TL main control block - used in timers*/
  v_PVOID_t           pTLCb; /*type is WLANTL_CBType*/

  /* vos buffer used for trigger frames */
  vos_pkt_t*          vosTriggBuf;

  /* flag to signal if a trigger frames is pending */
  v_U8_t              ucPendingTrigFrm;

  /* flag to signal if a data frame was sent out instead of a trigger frame */
  v_U8_t              ucDataSent;

  /* flag set when a UAPSD session with triggers generated in fw is being set*/
  v_U8_t              ucSet; 
}WLANTL_UAPSDInfoType;

/*---------------------------------------------------------------------------
  STA Client type
---------------------------------------------------------------------------*/      
typedef struct
{
  /* Flag that keeps track of registration; only one STA with unique 
     ID allowed */
  v_U8_t                        ucExists;

  /* Function pointer to the receive packet handler from HDD */
  WLANTL_STARxCBType            pfnSTARx;  

  /* Function pointer to the transmit complete confirmation handler 
    from HDD */
  WLANTL_TxCompCBType           pfnSTATxComp;

  /* Function pointer to the packet retrieval routine in HDD */
  WLANTL_STAFetchPktCBType      pfnSTAFetchPkt;

  /* Reordering information for the STA */
  WLANTL_BAReorderType          atlBAReorderInfo[WLAN_MAX_TID];

  /* STA Descriptor, contains information related to the new added STA */
  WLAN_STADescType              wSTADesc;

  /* Current connectivity state of the STA */
  WLANTL_STAStateType           tlState;

  /* Station priority */
  WLANTL_STAPriorityType        tlPri;

  /* Value of the averaged RSSI for this station */
  v_S7_t                        uRssiAvg;

  /* Tx packet count per station per TID */
  v_U32_t                       auTxCount[WLAN_MAX_TID];

  /* Rx packet count per station per TID */
  v_U32_t                       auRxCount[WLAN_MAX_TID];

  /*	Suspend flag */
  v_U8_t                        ucTxSuspended;

  /*	Pointer to the AMSDU chain maintained by the AMSDU de-aggregation 
      completion sub-module */
  vos_pkt_t*                    vosAMSDUChainRoot;

  /*	Pointer to the root of the chain */
  vos_pkt_t*                    vosAMSDUChain;

  /*	Used for saving/restoring frame header for 802.3/11 AMSDU sub-frames */
  v_U8_t                        aucMPDUHeader[WLANTL_MPDU_HEADER_LEN];

  /* length of the header */
  v_U8_t                        ucMPDUHeaderLen;

  /*Enabled ACs currently serviced by TL (automatic setup in TL)*/
  v_U8_t                        ucACMask; 

  /*	Current AC to be retrieved */
  WLANTL_ACEnumType             ucCurrentAC;

  /*	Last serviced AC to be retrieved */
  WLANTL_ACEnumType             ucServicedAC;

   /*	Current weight for the AC */
  v_U8_t                        ucCurrentWeight; 

  /* Info used for UAPSD trigger frame generation  */
  WLANTL_UAPSDInfoType          wUAPSDInfo[WLANTL_MAX_AC];

  /* flag to signal if a trigger frames is pending */
  v_U8_t                        ucPendingTrigFrm;

  WLANTL_TRANSFER_STA_TYPE      trafficStatistics;
}WLANTL_STAClientType;

/*---------------------------------------------------------------------------
  BAP Client type
---------------------------------------------------------------------------*/ 
typedef struct
{
  /* flag that keeps track of registration; only one non-data BT-AMP client 
     allowed */
  v_U8_t                    ucExists;

  /* pointer to the receive processing routine for non-data BT-AMP frames */
  WLANTL_BAPRxCBType        pfnTlBAPRx;  

  /* pointer to the non-data BT-AMP frame pending transmission */
  vos_pkt_t*                vosPendingDataBuff;

  /* BAP station ID */
  v_U8_t                    ucBAPSTAId; 
}WLANTL_BAPClientType;


/*---------------------------------------------------------------------------
  Management Frame Client type
---------------------------------------------------------------------------*/  
typedef struct
{
  /* flag that keeps track of registration; only one management frame 
     client allowed */
  v_U8_t                       ucExists;

  /* pointer to the receive processing routine for management frames */
  WLANTL_MgmtFrmRxCBType       pfnTlMgmtFrmRx;  

  /* pointer to the management frame pending transmission */
  vos_pkt_t*                   vosPendingDataBuff;
}WLANTL_MgmtFrmClientType;

typedef struct
{
   WLANTL_TrafficStatusChangedCBType  trafficCB;
   WLANTL_HO_TRAFFIC_STATUS_TYPE      trafficStatus;
   v_U32_t                            idleThreshold;
   v_U32_t                            measurePeriod;
   v_U32_t                            rtRXFrameCount;
   v_U32_t                            rtTXFrameCount;
   v_U32_t                            nrtRXFrameCount;
   v_U32_t                            nrtTXFrameCount;
   vos_timer_t                        trafficTimer;
   v_PVOID_t                          usrCtxt;
} WLANTL_HO_TRAFFIC_STATUS_HANDLE_TYPE;

typedef struct
{
   v_U8_t                          triggerEvent[WLANTL_HS_NUM_CLIENT];
   v_S7_t                          rssiValue;
   WLANTL_RSSICrossThresholdCBType crossCBFunction[WLANTL_HS_NUM_CLIENT];
   v_PVOID_t                       usrCtxt[WLANTL_HS_NUM_CLIENT];
   v_BOOL_t                        isEmpty;
   v_BOOL_t                        isMultipleClient;
} WLANTL_HO_RSSI_INDICATION_TYPE;

typedef struct
{
   v_U8_t                             numThreshold;
   v_U8_t                             regionNumber;
   v_S7_t                             historyRSSI;
   v_U8_t                             alpha;
   v_U32_t                            sampleTime;
} WLANTL_CURRENT_HO_STATE_TYPE;

typedef struct
{
   WLANTL_HO_RSSI_INDICATION_TYPE       registeredInd[WLANTL_MAX_AVAIL_THRESHOLD];
   WLANTL_CURRENT_HO_STATE_TYPE         currentHOState;
   WLANTL_HO_TRAFFIC_STATUS_HANDLE_TYPE currentTraffic;
   v_BOOL_t                             isBMPS;
   v_PVOID_t                            macCtxt;
} WLANTL_HO_SUPPORT_TYPE;

/*---------------------------------------------------------------------------
  TL control block type
---------------------------------------------------------------------------*/ 
typedef struct 
{
  /* TL configuration information */
  WLANTL_ConfigInfoType     tlConfigInfo;

  /* list of the active stations */
  WLANTL_STAClientType      atlSTAClients[WLAN_MAX_STA_COUNT];

  /* information on the management frame client */
  WLANTL_MgmtFrmClientType  tlMgmtFrmClient;

  /* information on the BT AMP client */
  WLANTL_BAPClientType      tlBAPClient;

  /* number of packets sent to BAL waiting for tx complete confirmation */
  v_U16_t                   usPendingTxCompleteCount;

  /* global suspend flag */
  v_U8_t                    ucTxSuspended;

  /* resource flag */
  v_U32_t                   uResCount;

  /* dummy vos buffer - used for chains */
  vos_pkt_t*                vosDummyBuf;

  /* temporary buffer for storing the packet that no longer fits */
  vos_pkt_t*                vosTempBuf;

  /* The value of the station id for the cached buffer */
  v_U8_t                    ucCachedSTAId;

  /* Last registered STA - until multiple sta support is added this will 
     be used always for tx */
  v_U8_t                    ucRegisteredStaId;

  /*used on tx packet to signal when there is no more data to tx for the 
   moment=> packets can be passed to BAL */
  v_U8_t                    ucNoMoreData;

  WLANTL_REORDER_BUFFER_T   reorderBufferPool[WLANTL_MAX_BA_SESSION];

  WLANTL_HO_SUPPORT_TYPE    hoSupport;

  v_BOOL_t                  bUrgent;
}WLANTL_CbType;

/*==========================================================================

  FUNCTION    WLANTL_GetFrames

  DESCRIPTION 

    BAL calls this function at the request of the lower bus interface. 
    When this request is being received TL will retrieve packets from HDD 
    in accordance with the priority rules and the count supplied by BAL. 
    
  DEPENDENCIES 

    HDD must have registered with TL at least one STA before this function 
    can be called.

  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    or BAL's control block can be extracted from its context 
    uSize:          maximum size accepted by the lower layer
                    
    OUT
    vosDataBuff:   it will contain a pointer to the first buffer supplied 
                    by TL, if there is more than one packet supplied, TL 
                    will chain them through vOSS buffers
   
  RETURN VALUE

    The result code associated with performing the operation  

    TRUE: if there are still frames to fetch 
    FALSE: error or HDD queues are drained 
    
  SIDE EFFECTS 
  
============================================================================*/
v_BOOL_t 
WLANTL_GetFrames
( 
  v_PVOID_t       pAdapter,
  vos_pkt_t**     vosDataBuff,
  v_U32_t         uSize,
  v_BOOL_t*       pbUrgent
);

/*==========================================================================

  FUNCTION    WLANTL_TxComp

  DESCRIPTION 
    It is being called by BAL upon asynchronous notification of the packet 
    or packets  being sent over the bus.
    
  DEPENDENCIES 
    Tx complete cannot be called without a previous transmit. 

  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    or BAL's control block can be extracted from its context 
    vosDataBuff:   it will contain a pointer to the first buffer for which 
                    the BAL report is being made, if there is more then one 
                    packet they will be chained using vOSS buffers. 
    wTxSTAtus:      the status of the transmitted packet, see above chapter 
                    on HDD interaction for a list of possible values 
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANTL_TxComp
( 
  v_PVOID_t      pAdapter,
  vos_pkt_t*     vosDataBuff,
  VOS_STATUS     wTxStatus
);

/*==========================================================================

  FUNCTION    WLANTL_RxFrames

  DESCRIPTION 
    Callback registered by TL and called by BAL when a packet is received 
    over the bus. Upon the call of this function TL will make the necessary 
    decision with regards to the forwarding or queuing of this packet and 
    the layer it needs to be delivered to. 
    
  DEPENDENCIES 
    TL must be initiailized before this function gets called. 
    If the frame carried is a data frame then the station for which it is
    destined to must have been previously registered with TL. 

  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    or BAL's control block can be extracted from its context 

    vosDataBuff:   it will contain a pointer to the first buffer received, 
                    if there is more then one packet they will be chained 
                    using vOSS buffers. 
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANTL_RxFrames
( 
  v_PVOID_t      pAdapter,
  vos_pkt_t*     vosDataBuff
);


/*==========================================================================
  FUNCTION    WLANTL_ResourceCB

  DESCRIPTION 
    Called by the TL when it has packets available for transmission. 

  DEPENDENCIES 
    The TL must be registered with BAL before this function can be called. 
    
  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    or BAL's control block can be extracted from its context 
    uCount:         avail resource count obtained from hw 
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_ResourceCB
( 
  v_PVOID_t       pAdapter,
  v_U32_t         uCount
);


/*==========================================================================
  FUNCTION    WLANTL_ProcessMainMessage

  DESCRIPTION 
    Called by VOSS when a message was serialized for TL through the
    main thread/task. 

  DEPENDENCIES 
    The TL must be initialized before this function can be called. 
    
  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    control block can be extracted from its context 
    message:        type and content of the message 
                    
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_ProcessMainMessage
(
  v_PVOID_t        pAdapter,
  vos_msg_t*       message
);

/*==========================================================================
  FUNCTION    WLANTL_ProcessTxMessage

  DESCRIPTION 
    Called by VOSS when a message was serialized for TL through the
    tx thread/task. 

  DEPENDENCIES 
    The TL must be initialized before this function can be called. 
    
  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    control block can be extracted from its context 
    message:        type and content of the message 
                    
   
  RETURN VALUE

    The result code associated with performing the operation  
    VOS_STATUS_SUCCESS:  Everything is good :) 


  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_ProcessTxMessage
(
  v_PVOID_t        pAdapter,
  vos_msg_t*       message
);

/*==========================================================================
  FUNCTION    WLAN_TLGetNextTxIds

  DESCRIPTION 
    Gets the next station and next AC in the list

  DEPENDENCIES 
         
  PARAMETERS 

   OUT
   pucSTAId:    STAtion ID 
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLAN_TLGetNextTxIds
(
  v_PVOID_t    pAdapter,
  v_U8_t*      pucSTAId
);

/*==========================================================================

  FUNCTION    WLANTL_CleanCb

  DESCRIPTION 
    Cleans TL control block
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pTLCb:       pointer to TL's control block 
    ucEmpty:     set if TL has to clean up the queues and release pedning pkts
        
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANTL_CleanCB
( 
  WLANTL_CbType*  pTLCb,
  v_U8_t          ucEmpty
);

/*==========================================================================

  FUNCTION    WLANTL_CleanSTA

  DESCRIPTION 
    Cleans a station control block. 
    
  DEPENDENCIES 
    
  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to TL's 
                    control block can be extracted from its context 
    ucEmpty:        if set the queues and pending pkts will be emptyed
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS 
WLANTL_CleanSTA
( 
  WLANTL_STAClientType*  ptlSTAClient,
  v_U8_t                 ucEmpty
);

/*==========================================================================
  FUNCTION    WLANTL_GetTxResourcesCB

  DESCRIPTION 
    Processing function for Resource needed signal. A request will be issued
    to BAL to get mor tx resources. 

  DEPENDENCIES 
    The TL must be initialized before this function can be called. 
    
  PARAMETERS 

    IN
    pvosGCtx:       pointer to the global vos context; a handle to TL's 
                    control block can be extracted from its context 
                    
   
  RETURN VALUE
    The result code associated with performing the operation  

    VOS_STATUS_E_FAULT:   pointer to TL cb is NULL ; access would cause a 
                          page fault  
    VOS_STATUS_SUCCESS:   Everything is good :)

  Other values can be returned as a result of a function call, please check 
  corresponding API for more info. 
  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_GetTxResourcesCB
(
  v_PVOID_t        pvosGCtx
);

/*==========================================================================
  FUNCTION    WLANTL_PrepareBDHeader

  DESCRIPTION 
    Callback function for serializing Suspend signal through Tx thread

  DEPENDENCIES 
    Just notify HAL that suspend in TL is complete.
     
  PARAMETERS 

   IN
   pAdapter:       pointer to the global adapter context; a handle to TL's 
                   control block can be extracted from its context 
   pUserData:      user data sent with the callback
   
  RETURN VALUE
    The result code associated with performing the operation  

  SIDE EFFECTS 
  
============================================================================*/
void
WLANTL_PrepareBDHeader
( 
  vos_pkt_t*      vosDataBuff,
  v_PVOID_t*      ppvBDHeader,
  v_MACADDR_t*    pvDestMacAdddr,
  v_U8_t          ucDisableFrmXtl,
  VOS_STATUS*     pvosSTAtus,
  v_U16_t*        usPktLen,
  v_U8_t          ucQosEnabled, 
  v_U8_t          extraHeadSpace
);

/*==========================================================================
  FUNCTION    WLANTL_Translate8023To80211Header

  DESCRIPTION 
    Inline function for translating and 802.3 header into an 802.11 header.

  DEPENDENCIES 
    
     
  PARAMETERS 

   IN
    pTLCb:            TL control block 
    ucStaId:          station ID 
    
   IN/OUT
    vosDataBuff:      vos data buffer, will contain the new header on output

   OUT
    pvosStatus:       status of the operation
   
  RETURN VALUE
    No return.   

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_Translate8023To80211Header
(
  vos_pkt_t*      vosDataBuff,
  VOS_STATUS*     pvosStatus,
  WLANTL_CbType*  pTLCb,
  v_U8_t          ucStaId,
  v_U8_t          ucUP,
  v_U8_t          *extraHeadSpace
);

/*==========================================================================
  FUNCTION    WLANTL_Translate80211To8023Header

  DESCRIPTION 
    Inline function for translating and 802.11 header into an 802.3 header.

  DEPENDENCIES 
    
     
  PARAMETERS 

   IN
    pTLCb:            TL control block 
    ucStaId:          station ID 
    ucHeaderLen:      Length of the header from BD 
    
   IN/OUT
    vosDataBuff:      vos data buffer, will contain the new header on output

   OUT
    pvosStatus:       status of the operation
   
  RETURN VALUE
    Status of the operation

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_Translate80211To8023Header
(
  vos_pkt_t*      vosDataBuff,
  VOS_STATUS*     pvosStatus,
  v_U8_t          ucHeaderLen,
  WLANTL_CbType*  pTLCb,
  v_U8_t          ucSTAId
);

/*==========================================================================

  FUNCTION    WLANTL_MgmtFrmRxDefaultCb

  DESCRIPTION 
    Default Mgmt Frm rx callback: asserts all the time. If this function gets 
    called  it means there is no registered rx cb pointer for Mgmt Frm.
    
  DEPENDENCIES 
    
  PARAMETERS 

    Not used.
  
  RETURN VALUE 
   Always FAILURE.

============================================================================*/
VOS_STATUS 
WLANTL_MgmtFrmRxDefaultCb
(
  v_PVOID_t  pAdapter, 
  v_PVOID_t  vosBuff
);

/*==========================================================================

  FUNCTION    WLANTL_STARxDefaultCb

  DESCRIPTION 
    Default BAP rx callback: asserts all the time. If this function gets 
    called  it means there is no registered rx cb pointer for BAP.
    
  DEPENDENCIES 
    
  PARAMETERS 

    Not used.
  
  RETURN VALUE 
   Always FAILURE.

============================================================================*/
VOS_STATUS 
WLANTL_BAPRxDefaultCb
(
  v_PVOID_t    pAdapter,
  vos_pkt_t*   vosDataBuff
);

/*==========================================================================

  FUNCTION    WLANTL_STARxDefaultCb

  DESCRIPTION 
    Default STA rx callback: asserts all the time. If this function gets 
    called  it means there is no registered rx cb pointer for station.
    (Mem corruption most likely, it should never happen) 
    
  DEPENDENCIES 
    
  PARAMETERS 

    Not used.
  
  RETURN VALUE 
   Always FAILURE.

============================================================================*/
VOS_STATUS 
WLANTL_STARxDefaultCb
(
  v_PVOID_t               pAdapter,
  vos_pkt_t*              vosDataBuff,
  v_U8_t                  ucSTAId,
  WLANTL_RxMetaInfoType*  pRxMetaInfo
);

/*==========================================================================

  FUNCTION    WLANTL_STAFetchPktDefaultCb

  DESCRIPTION 
    Default fetch callback: asserts all the time. If this function gets 
    called  it means there is no registered fetch cb pointer for station.
    (Mem corruption most likely, it should never happen) 
    
  DEPENDENCIES 
    
  PARAMETERS 

    Not used.
  
  RETURN VALUE 
   Always FAILURE.

============================================================================*/
VOS_STATUS 
WLANTL_STAFetchPktDefaultCb
(
  v_PVOID_t              pAdapter,
  v_U8_t*                pucSTAId,
  WLANTL_ACEnumType*     pucAC,
  vos_pkt_t**            vosDataBuff,
  WLANTL_MetaInfoType*   tlMetaInfo
);

/*==========================================================================

  FUNCTION    WLANTL_TxCompDefaultCb

  DESCRIPTION   
    Default tx complete handler. It will release the completed pkt to 
    prevent memory leaks.  

  PARAMETERS 

    IN
    pAdapter:       pointer to the global adapter context; a handle to 
                    TL/HAL/PE/BAP/HDD control block can be extracted from 
                    its context 
    vosDataBuff:   pointer to the VOSS data buffer that was transmitted 
    wTxSTAtus:      status of the transmission 

  
  RETURN VALUE 
    The result code associated with performing the operation; please 
    check vos_pkt_return_pkt for possible error codes. 

============================================================================*/
VOS_STATUS 
WLANTL_TxCompDefaultCb
( 
 v_PVOID_t      pAdapter,
 vos_pkt_t*     vosDataBuff,
 VOS_STATUS     wTxSTAtus 
);

/*==========================================================================
  
  FUNCTION    WLANTL_PackUpTriggerFrame
    
  DESCRIPTION 
    Packs up a trigger frame and places it in TL's cache for tx and notifies 
    BAL 

  DEPENDENCIES 
         
  PARAMETERS 

  IN
    pTLCb:         pointer to the TL control block
    pfnSTATxComp:  Tx Complete Cb to be used when frame is received 
    ucSTAId:       station id 
    ucAC:          access category
      
  RETURN VALUE
    None

  SIDE EFFECTS 
  
============================================================================*/
VOS_STATUS
WLANTL_PackUpTriggerFrame
( 
  WLANTL_CbType*            pTLCb,
  WLANTL_TxCompCBType       pfnSTATxComp,
  v_U8_t                    ucSTAId,
  WLANTL_ACEnumType         ucAC 
);

/*==========================================================================
  
  FUNCTION    WLANTL_TxCompTriggFrameSI

  DESCRIPTION   
    Tx complete handler for the service interval trigger frame. 
    It will restart the SI timer. 
         
  PARAMETERS 

   IN
    pvosGCtx:       pointer to the global vos context; a handle to 
                    TL/HAL/PE/BAP/HDD control block can be extracted from 
                    its context 
    vosDataBuff:   pointer to the VOSS data buffer that was transmitted 
    wTxSTAtus:      status of the transmission 

      
  RETURN VALUE
    The result code associated with performing the operation  

 ============================================================================*/
VOS_STATUS
WLANTL_TxCompTriggFrameSI
( 
  v_PVOID_t      pvosGCtx,
  vos_pkt_t*     vosDataBuff,
  VOS_STATUS     wTxSTAtus 
);

/*==========================================================================
  
  FUNCTION    WLANTL_TxCompTriggFrameSI

  DESCRIPTION   
    Tx complete handler for the service interval trigger frame. 
    It will restart the SI timer. 
         
  PARAMETERS 

   IN
    pvosGCtx:       pointer to the global vos context; a handle to 
                    TL/HAL/PE/BAP/HDD control block can be extracted from 
                    its context 
    vosDataBuff:   pointer to the VOSS data buffer that was transmitted 
    wTxSTAtus:      status of the transmission 
         
  
  RETURN VALUE
    The result code associated with performing the operation  

============================================================================*/
VOS_STATUS
WLANTL_TxCompTriggFrameDI
( 
 v_PVOID_t      pvosGCtx,
 vos_pkt_t*     vosDataBuff, 
 VOS_STATUS     wTxSTAtus 
);


#endif /* #ifndef WLAN_QCT_TLI_H */
