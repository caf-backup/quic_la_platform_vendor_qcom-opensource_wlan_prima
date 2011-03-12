#ifndef WLAN_QCT_WDI_I_H
#define WLAN_QCT_WDI_I_H

/*===========================================================================

         W L A N   D E V I C E   A B S T R A C T I O N   L A Y E R 
              I N T E R N A L     A P I       F O R    T H E
                                 D A T A   P A T H 
                
                   
DESCRIPTION
  This file contains the internal API exposed by the DAL Control Path Core 
  module to be used by the DAL Data Path Core. 
  
      
  Copyright (c) 2010 QUALCOMM Incorporated. All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when        who    what, where, why
--------    ---    ----------------------------------------------------------
08/19/10    lti     Created module.

===========================================================================*/

#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_list.h"
#include "wlan_qct_pal_sync.h"
#include "wlan_qct_pal_timer.h"
#include "wlan_qct_wdi_cts.h" 
#include "wlan_qct_wdi_bd.h" 

#include "wlan_hal_msg.h"
#include "wlan_qct_dev_defs.h"
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*Assert macro - redefined for WDI so it is more flexible in disabling*/
#define WDI_ASSERT(_cond) WPAL_ASSERT(_cond)

/*Error codes that can be returned by WDI - the HAL Error codes are not 
 propagated outside WDI because they are too explicit when refering to RIVA
 HW errors - they are masked under dev internal failure*/
#define WDI_ERR_BASIC_OP_FAILURE           0 
#define WDI_ERR_TRANSPORT_FAILURE          1 
#define WDI_ERR_INVALID_RSP_FMT            2 
#define WDI_ERR_RSP_TIMEOUT                3 
#define WDI_ERR_DEV_INTERNAL_FAILURE       4

/*WDI Response timeout - how long will WDI wait for a response from the device     
    - it should be large enough to allow any other failure mechanism to kick 
      in before we get to a timeout (ms units)*/
#define WDI_RESPONSE_TIMEOUT   120000

/*! TO DO: check this against the HAL header and set the right values*/
#define WDI_MAX_SUPPORTED_STAS    10 
#define WDI_MAX_SUPPORTED_BSS     5 

/* Control transport channel size*/
#define WDI_CT_CHANNEL_SIZE 4096

/*Invalid BSS index ! TO DO: Must come from the HAL header file*/
#define WDI_BSS_INVALID_IDX 0xFF
/*---------------------------------------------------------------------------
  HAL Status
---------------------------------------------------------------------------*/
/*! TO DO: !!! TEMP - Definition must come from a common HAL header file*/
typedef enum
{
    eHAL_STATUS_SUCCESS,

    // general failure.  This status applies to all failure that are not covered
    // by more specific return codes.
    eHAL_STATUS_FAILURE,
    eHAL_STATUS_FAILED_ALLOC,
    eHAL_STATUS_RESOURCES,

    // the HAL has not been opened and a HAL function is being attempted.
    eHAL_STATUS_NOT_OPEN,

    // function failed due to the card being removed...
    eHAL_STATUS_CARD_NOT_PRESENT,

    //halInterrupt status
    eHAL_STATUS_INTERRUPT_ENABLED,
    eHAL_STATUS_INTERRUPT_DISABLED,
    eHAL_STATUS_NO_INTERRUPTS,
    eHAL_STATUS_INTERRUPT_PRESENT,
    eHAL_STATUS_ALL_INTERRUPTS_PROCESSED,
    eHAL_STATUS_INTERRUPT_NOT_PROCESSED,        //interrupt cleared but no Isr to process

    // a parameter on the PAL function call is not valid.
    eHAL_STATUS_INVALID_PARAMETER,

    // the PAL has not been initialized...
    eHAL_STATUS_NOT_INITIALIZED,

    // Error codes for PE-HAL message API
    eHAL_STATUS_INVALID_STAIDX,
    eHAL_STATUS_INVALID_BSSIDX,
    eHAL_STATUS_STA_TABLE_FULL,             // No space to add more STA, sta table full.
    eHAL_STATUS_BSSID_TABLE_FULL,
    eHAL_STATUS_DUPLICATE_BSSID,
    eHAL_STATUS_DUPLICATE_STA,
    eHAL_STATUS_BSSID_INVALID,
    eHAL_STATUS_STA_INVALID,
    eHAL_STATUS_INVALID_KEYID,
    eHAL_STATUS_INVALID_SIGNATURE,

    //DXE
    eHAL_STATUS_DXE_FAILED_NO_DESCS,
    eHAL_STATUS_DXE_CHANNEL_NOT_CONFIG,         // Channel not configured
    eHAL_STATUS_DXE_CHANNEL_MISUSE,             // Specified operation inconsistent w/ configuration
    eHAL_STATUS_DXE_VIRTUAL_MEM_ALLOC_ERROR,    //
    eHAL_STATUS_DXE_SHARED_MEM_ALLOC_ERROR,     //
    eHAL_STATUS_DXE_INVALID_CHANNEL,
    eHAL_STATUS_DXE_INVALID_CALLBACK,
    eHAL_STATUS_DXE_INCONSISTENT_DESC_COUNT,
    eHAL_STATUS_DXE_XFR_QUEUE_ERROR,
    eHAL_STATUS_DXE_INVALID_BUFFER,
    eHAL_STATUS_DXE_INCOMPLETE_PACKET,
    eHAL_STATUS_DXE_INVALID_PARAMETER,
    eHAL_STATUS_DXE_CH_ALREADY_CONFIGURED,
    eHAL_STATUS_DXE_USB_INVALID_EP,
    eHAL_STATUS_DXE_GEN_ERROR,


    // status codes added for the ImageValidate library
    eHAL_STATUS_E_NULL_VALUE,
    eHAL_STATUS_E_FILE_NOT_FOUND,
    eHAL_STATUS_E_FILE_INVALID_CONTENT,
    eHAL_STATUS_E_MALLOC_FAILED,
    eHAL_STATUS_E_FILE_READ_FAILED,
    eHAL_STATUS_E_IMAGE_INVALID,
    eHAL_STATUS_E_IMAGE_UNSUPPORTED,

    // status code returned by device memory calls when memory is
    // not aligned correctly.
    eHAL_STATUS_DEVICE_MEMORY_MISALIGNED,          // memory access is not aligned on a 4 byte boundary
    eHAL_STATUS_DEVICE_MEMORY_LENGTH_ERROR,        // memory access is not a multiple of 4 bytes

    // Generic status code to indicate network congestion.
    eHAL_STATUS_NET_CONGESTION,

    // various status codes for Rx packet dropped conditions...  Note the Min and Max
    // enums that bracked the Rx Packet Dropped status codes.   There is code that
    // looks at the various packet dropped conditions so make sure these min / max
    // enums remain accurate.
    eHAL_STATUS_RX_PACKET_DROPPED,
    eHAL_STATUS_RX_PACKET_DROPPED_MIN = eHAL_STATUS_RX_PACKET_DROPPED,
    eHAL_STATUS_RX_PACKET_DROPPED_NULL_DATA,
    eHAL_STATUS_RX_PACKET_DROPPED_WDS_FRAME,
    eHAL_STATUS_RX_PACKET_DROPPED_FILTERED,
    eHAL_STATUS_RX_PACKET_DROPPED_GROUP_FROM_SELF,
    eHAL_STATUS_RX_PACKET_DROPPED_MAX = eHAL_STATUS_RX_PACKET_DROPPED_GROUP_FROM_SELF,

    // Status indicating that PMU did not power up and hence indicative of the fact that the clocks are not on
    eHAL_STATUS_PMU_NOT_POWERED_UP,

    // Queuing code for BA message API
    eHAL_STATUS_BA_ENQUEUED,        // packets have been buffered in Host
    eHAL_STATUS_BA_INVALID,

    // A-MPDU/BA related Error codes
    eHAL_STATUS_BA_RX_BUFFERS_FULL,
    eHAL_STATUS_BA_RX_MAX_SESSIONS_REACHED,
    eHAL_STATUS_BA_RX_INVALID_SESSION_ID,

    // !!LAC - can we rework the code so these are not needed?
    eHAL_STATUS_BA_RX_DROP_FRAME,
    eHAL_STATUS_BA_RX_INDICATE_FRAME,
    eHAL_STATUS_BA_RX_ENQUEUE_FRAME,

    // PMC return codes.
    eHAL_STATUS_PMC_PENDING,
    eHAL_STATUS_PMC_DISABLED,
    eHAL_STATUS_PMC_NOT_NOW,
    eHAL_STATUS_PMC_AC_POWER,
    eHAL_STATUS_PMC_SYS_ERROR,
    eHAL_STATUS_PMC_CANNOT_ENTER_IMPS,
    eHAL_STATUS_PMC_ALREADY_IN_IMPS,

    eHAL_STATUS_HEARTBEAT_TMOUT,
    eHAL_STATUS_NTH_BEACON_DELIVERY,

    //CSR
    eHAL_STATUS_CSR_WRONG_STATE,

    // DPU
    eHAL_STATUS_DPU_DESCRIPTOR_TABLE_FULL,
    eHAL_STATUS_DPU_MICKEY_TABLE_FULL,

    // HAL-FW messages
    eHAL_STATUS_FW_MSG_FAILURE,                // Error in Hal-FW message interface
    eHAL_STATUS_FW_MSG_TIMEDOUT,
    eHAL_STATUS_FW_MSG_INVALID,
    eHAL_STATUS_FW_SEND_MSG_FAILED,
    eHAL_STATUS_FW_PS_BUSY,

    eHAL_STATUS_TIMER_START_FAILED,
    eHAL_STATUS_TIMER_STOP_FAILED,

    eHAL_STATUS_TL_SUSPEND_TIMEOUT,

    eHAL_STATUS_UMA_DESCRIPTOR_TABLE_FULL,

    eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN,

    // not a real status.  Just a way to mark the maximum in the enum.
    eHAL_STATUS_MAX

} eHalStatus;

/*---------------------------------------------------------------------------
  DAL Control Path Main States
---------------------------------------------------------------------------*/      
typedef enum
{
  /* Transition in this state made upon creation and when a close request is 
     received*/
  WDI_INIT_ST = 0,      

  /* Transition happens after a Start response was received from HAL (as a
  result of a previously sent HAL Request)*/
  WDI_STARTED_ST,     

  /* Transition happens when a Stop request was received */
  WDI_STOPPED_ST, 

  /* Transition happens when a request is being sent down to HAL and we are
    waiting for the response */
  WDI_BUSY_ST,  

  WDI_MAX_ST
}WDI_MainStateType;


/*---------------------------------------------------------------------------
  DAL Control Path Scan States
---------------------------------------------------------------------------*/      
typedef enum
{
  /*The flag will be set to this state when init is called. Once the flag has   
    this value the only two scanning API calls allowed are Scan Start and 
    Scan Finished*/
  WDI_SCAN_INITIALIZED_ST = 0, 

  /*The flag will be set to this value once the Start Scan API is called.   
    When the flag has this value only Scan End API will be allowed. */
  WDI_SCAN_STARTED_ST     = 1, 

  /*The flag will be set to this value when End Scan API is called. When the   
    flag is set to this value the only two Scan APIs allowed are Start and 
    Finish. */
	WDI_SCAN_ENDED_ST       = 2, 

  /*The flag will be set to this value in the beginning before init is called   
    and after the Finish API is called. No other scan APIs will be allowed 
    in this state until Scan Init is called again. */
	WDI_SCAN_FINISHED_ST    = 3,

  WDI_SCAN_MAX_ST
}WDI_ScanStateType;

/*--------------------------------------------------------------------------- 
   WLAN DAL BSS Session Type - used to allow simulatneous association
   and keep track of each associated session 
 ---------------------------------------------------------------------------*/
#define WDI_MAX_BSS_SESSIONS  10

typedef enum
{
  /*Init state*/
  WDI_ASSOC_INIT_ST, 

  /*Joining State*/
  WDI_ASSOC_JOINING_ST, 

  /*Associated state*/
  WDI_ASSOC_POST_ST,

  WDI_ASSOC_MAX_ST
}WDI_AssocStateType;

/*--------------------------------------------------------------------------- 
   WLAN DAL Supported Request Types
 ---------------------------------------------------------------------------*/
typedef enum
{
  /*WLAN DAL START Request*/
  WDI_START_REQ  = 0, 

  /*WLAN DAL STOP Request*/
  WDI_STOP_REQ   = 1, 

  /*WLAN DAL STOP Request*/
  WDI_CLOSE_REQ  = 2, 


  /*SCAN*/
  /*WLAN DAL Init Scan Request*/
  WDI_INIT_SCAN_REQ    = 3, 

  /*WLAN DAL Start Scan Request*/
  WDI_START_SCAN_REQ   = 4, 

  /*WLAN DAL End Scan Request*/
  WDI_END_SCAN_REQ     = 5, 

  /*WLAN DAL Finish Scan Request*/
  WDI_FINISH_SCAN_REQ  = 6, 


  /*ASSOCIATION*/
  /*WLAN DAL Join Request*/
  WDI_JOIN_REQ          = 7, 

  /*WLAN DAL Config BSS Request*/
  WDI_CONFIG_BSS_REQ    = 8, 

  /*WLAN DAL Del BSS Request*/
  WDI_DEL_BSS_REQ       = 9, 

  /*WLAN DAL Post Assoc Request*/
  WDI_POST_ASSOC_REQ    = 10, 

  /*WLAN DAL Del STA Request*/
  WDI_DEL_STA_REQ       = 11, 

  /*Security*/
  /*WLAN DAL Set BSS Key Request*/
  WDI_SET_BSS_KEY_REQ   = 12,
  
  /*WLAN DAL Remove BSS Key Request*/ 
  WDI_RMV_BSS_KEY_REQ   = 13,
  
  /*WLAN DAL Set STA Key Request*/ 
  WDI_SET_STA_KEY_REQ   = 14,
  
  /*WLAN DAL Remove STA Key Request*/ 
  WDI_RMV_STA_KEY_REQ   = 15, 

  /*QOS and BA*/
  /*WLAN DAL Add TSpec Request*/
  WDI_ADD_TS_REQ        = 16,
  
  /*WLAN DAL Delete TSpec Request*/ 
  WDI_DEL_TS_REQ        = 17,
  
  /*WLAN DAL Update EDCA Params Request*/ 
  WDI_UPD_EDCA_PRMS_REQ = 18,

  /*WLAN DAL Add BA Request*/
  WDI_ADD_BA_REQ        = 19,

  /*WLAN DAL Delete BA Request*/ 
  WDI_DEL_BA_REQ        = 20,

   /* Miscellaneous Control	*/
  /*WLAN DAL Channel Switch Request*/ 
  WDI_CH_SWITCH_REQ     = 21,
  
  /*WLAN DAL Config STA Request*/ 
  WDI_CONFIG_STA_REQ    = 22, 

  /*WLAN DAL Set Link State Request*/ 
  WDI_SET_LINK_ST_REQ   = 23,
  
  /*WLAN DAL Get Stats Request*/ 
  WDI_GET_STATS_REQ     = 24, 

 /*WLAN DAL Update Config Request*/ 
  WDI_UPDATE_CFG_REQ    = 25, 

  WDI_MAX_REQ
}WDI_RequestEnumType; 

/*--------------------------------------------------------------------------- 
   WLAN DAL Supported Response Types
 ---------------------------------------------------------------------------*/
typedef enum
{
  /*WLAN DAL START Response*/
  WDI_START_RESP  = 0, 

  /*WLAN DAL STOP Response*/
  WDI_STOP_RESP   = 1, 

  /*WLAN DAL STOP Response*/
  WDI_CLOSE_RESP  = 2, 

  /*SCAN*/
  /*WLAN DAL Init Scan Response*/
  WDI_INIT_SCAN_RESP    = 3, 

  /*WLAN DAL Start Scan Response*/
  WDI_START_SCAN_RESP   = 4, 

  /*WLAN DAL End Scan Response*/
  WDI_END_SCAN_RESP     = 5, 

  /*WLAN DAL Finish Scan Response*/
  WDI_FINISH_SCAN_RESP  = 6, 


  /*ASSOCIATION*/
  /*WLAN DAL Join Response*/
  WDI_JOIN_RESP          = 7, 

  /*WLAN DAL Config BSS Response*/
  WDI_CONFIG_BSS_RESP    = 8, 

  /*WLAN DAL Del BSS Response*/
  WDI_DEL_BSS_RESP       = 9, 

  /*WLAN DAL Post Assoc Response*/
  WDI_POST_ASSOC_RESP    = 10, 

  /*WLAN DAL Del STA Response*/
  WDI_DEL_STA_RESP       = 11, 

  /*WLAN DAL Set BSS Key Response*/
  WDI_SET_BSS_KEY_RESP   = 12,
  
  /*WLAN DAL Remove BSS Key Response*/ 
  WDI_RMV_BSS_KEY_RESP   = 13,
  
  /*WLAN DAL Set STA Key Response*/ 
  WDI_SET_STA_KEY_RESP   = 14,
  
  /*WLAN DAL Remove STA Key Response*/ 
  WDI_RMV_STA_KEY_RESP   = 15, 

  /*WLAN DAL Add TSpec Response*/
  WDI_ADD_TS_RESP        = 16,
  
  /*WLAN DAL Delete TSpec Response*/ 
  WDI_DEL_TS_RESP        = 17,
  
  /*WLAN DAL Update EDCA Params Response*/ 
  WDI_UPD_EDCA_PRMS_RESP = 18,

  /*WLAN DAL Add BA Response*/
  WDI_ADD_BA_RESP        = 19,

  /*WLAN DAL Delete BA Response*/ 
  WDI_DEL_BA_RESP        = 20,

  /*WLAN DAL Channel Switch Response*/ 
  WDI_CH_SWITCH_RESP     = 21,
  
  /*WLAN DAL Config STA Response*/ 
  WDI_CONFIG_STA_RESP    = 22, 

  /*WLAN DAL Set Link State Response*/ 
  WDI_SET_LINK_ST_RESP   = 23,
  
  /*WLAN DAL Get Stats Response*/ 
  WDI_GET_STATS_RESP     = 24, 

 /*WLAN DAL Update Config Response*/ 
  WDI_UPDATE_CFG_RESP    = 25, 

  /*-------------------------------------------------------------------------
    Indications
     !! Keep these last in the enum if possible
    -------------------------------------------------------------------------*/
  /*When RSSI monitoring is enabled of the Lower MAC and a threshold has been
    passed. */
  WDI_HAL_LOW_RSSI_IND                = 26, 

  /*Link loss in the low MAC */
  WDI_HAL_MISSED_BEACON_IND           = 27,

  /*When hardware has signaled an unknown addr2 frames. The indication will
  contain info from frames to be passed to the UMAC, this may use this info to
  deauth the STA*/
  WDI_HAL_UNKNOWN_ADDR2_FRAME_RX_IND  = 28,

  /*MIC Failure detected by HW*/
  WDI_HAL_MIC_FAILURE_IND             = 29,

  /*Fatal Erro Ind*/
  WDI_HAL_FATAL_ERROR_IND             = 30, 

  WDI_MAX_RESP
}WDI_ResponseEnumType; 

typedef struct 
{
  /*Flag that marks a session as being in use*/
  wpt_boolean         bInUse; 

  /*Flag that keeps track if a series of assoc requests for this BSS are
    currently pending in the queue or processed
    - the flag is set to true when the Join request ends up being queued
    - and reset to false when the Pending queue is empty */
  wpt_boolean         bAssocReqQueued;

  /*BSSID of the session*/
  wpt_macAddr     	  macBSSID; 

  /*BSS Index associated with this BSSID*/
  wpt_uint16          usBSSIdx; 

  /*Associated state of the current BSS*/
  WDI_AssocStateType  wdiAssocState;

  /*WDI Pending Request Queue*/
  wpt_list            wptPendingQueue;

  /*DPU Information for this BSS*/
  wpt_uint8           bcastDpuIndex;
  wpt_uint8           bcastDpuSignature;
  wpt_uint8           bcastMgmtDpuIndex;
  wpt_uint8           bcastMgmtDpuSignature;

  /*RMF enabled/disabled*/
  wpt_uint8           ucRmfEnabled;

  /*Bcast STA ID associated with this BSS session */
  wpt_uint8           bcastStaIdx;
}WDI_BSSSessionType;

/*---------------------------------------------------------------------------
  WDI_ConfigBSSRspInfoType
---------------------------------------------------------------------------*/
typedef WPT_PACK_PRE struct 
{
  /*BSS index allocated by HAL*/
  wpt_uint16   usBSSIdx;

  /*BSSID of the BSS*/
  wpt_macAddr  macBSSID; 

  /*Broadcast DPU descriptor index allocated by HAL and used for
  broadcast/multicast packets.*/
  wpt_uint8    ucBcastDpuDescIndx;

  /*DPU signature to be used for broadcast/multicast packets*/
  wpt_uint8    ucBcastDpuSignature;	
  
  /*DPU descriptor index allocated by HAL, used for bcast/mcast management
  packets*/
  wpt_uint8    ucMgmtDpuDescIndx;		

  /*DPU signature to be used for bcast/mcast management packets*/
  wpt_uint8    ucMgmtDpuSignature;		

  /*Status of the request received from HAL */
  eHalStatus   halStatus;
}WPT_PACK_POST WDI_ConfigBSSRspInfoType;


/*---------------------------------------------------------------------------
  WDI_PostAssocRspInfoType
---------------------------------------------------------------------------*/
typedef WPT_PACK_PRE struct
{
  /*STA Index allocated by HAL.*/
  wpt_uint16   usSTAIdx;
  
  /*MAC Address of STA*/ 
  wpt_macAddr  macSTA;
  
  /*Unicast DPU signature*/
  wpt_uint8    ucUcastSig;

  /*Broadcast DPU Signature*/
  wpt_uint8    ucBcastSig;

  /*BSSID of the BSS*/
  wpt_macAddr  macBSSID; 

  /*HAL Status */
  eHalStatus   halStatus;
}WPT_PACK_POST WDI_PostAssocRspInfoType;

/*--------------------------------------------------------------------------- 
   WLAN DAL FSM Event Info Type 
 ---------------------------------------------------------------------------*/
typedef struct
{
  /*Events can be linked in a list - put a node in front for that, it will be
   used by wpt to link them*/
  wpt_list_node          wptListNode; 

  /*Request Received */
  WDI_RequestEnumType    wdiRequest;

  /*Response Received */
  WDI_ResponseEnumType   wdiResponse;

  /*Data associated with the request */
  void*                  pEventData;

  /*Data Size*/
  wpt_uint32             uEventDataSize;

  /*Callback function for receiving the response to the event*/
  void*                  pCBfnc;

  /*User data to be sent along with the CB function call*/
  void*                  pUserData;
}WDI_EventInfoType; 

/*--------------------------------------------------------------------------- 
   WLAN DAL Control Block Type 
 ---------------------------------------------------------------------------*/
typedef struct
{
  /*Ptr to the OS Context received from the UMAC*/
  void*                       pOSContext;

  /*Ptr to the PAL Context received from PAL*/
  void*                       pPALContext; 

  /*Ptr to the Datapath Context received from PAL*/
  void*                       pDPContext; 

  /*Ptr to the Datapath Transport Driver Context received from PAL*/
  void*                       pDTDriverContext; 

  /*Hanlde to the control transport service*/
  WCTS_HandleType             wctsHandle;

  /*Flag that keeps track if CT is Opened or not*/
  wpt_boolean                 bCTOpened;

  /*The global state of the DAL Control Path*/
  WDI_MainStateType           uGlobalState; 

  /*Flag to keep track of the expected state transition after processing
    of a response */
  WDI_MainStateType           ucExpectedStateTransition;

  /*Main Synchronization Object for the WDI CB*/
  wpt_mutex                   wptMutex;

  /*WDI response timer*/
  wpt_timer                   wptResponseTimer;

  /*WDI Pending Request Queue*/
  wpt_list                    wptPendingQueue;

  /*The state of the DAL during a scanning procedure*/
  WDI_ScanStateType           uScanState;  

  /*Flag that keeps track if a Scan is currently in progress*/
  wpt_boolean                 bScanInProgress;

  /*Flag that keeps track if an Association is currently in progress*/
  wpt_boolean                 bAssociationInProgress; 

  /*Array of simultaneous BSS Sessions*/
  WDI_BSSSessionType          aBSSSessions[WDI_MAX_BSS_SESSIONS];

  /*! TO DO : - group these in a union, only one cached req can exist at a
      time  */

  /*Cached post assoc request - there can only be one in the system as
    only one request goes down to hal up until a response is received
    The values cached are used on response to save a station if needed */
  WDI_PostAssocReqParamsType  wdiCachedPostAssocReq; 

  /*Cached config sta request - there can only be one in the system as
    only one request goes down to hal up until a response is received
    The values cached are used on response to save a station if needed */
  WDI_ConfigSTAReqParamsType  wdiCachedConfigStaReq; 

  /*Cached config sta request - there can only be one in the system as
    only one request goes down to hal up until a response is received
    The values cached are used on response to save a BSS if needed */
  WDI_ConfigBSSReqParamsType  wdiCachedConfigBssReq; 

  /*Cached set link state request - there can only be one in the system as
    only one request goes down to hal up until a response is received
    The values cached are used on response to delete a BSS if needed */
  WDI_SetLinkReqParamsType    wdiCacheSetLinkStReq; 


  /*Current session being handled*/
  wpt_uint8                   ucCurrentBSSSesIdx;

  /*Pointer to the response CB of the pending request*/
  void*                       pfncRspCB;

  /*Pointer to the user data to be sent along with the response CB*/
  void*                       pRspCBUserData; 

  /*The expected response from HAL*/
  WDI_ResponseEnumType        wdiExpectedResponse;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb             wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                       pReqStatusUserData;

   /*Indication callback given by UMAC to be called by the WLAN DAL when it
    wishes to send something back independent of a request*/
  WDI_LowLevelIndCBType       wdiLowLevelIndCB; 

  /*The user data passed in by UMAC, it will be sent back when the indication
    function pointer will be called */
  void*                       pIndUserData;

  /*Cached start response parameters*/
  WDI_StartRspParamsType      wdiCachedStartRspParams;

  /*STA Table Information*/
  /*Max number of stations allowed by device */
  wpt_uint8                   ucMaxStations;

  /*Max number of BSSes allowed by device */
  wpt_uint8                   ucMaxBssids;

  /* Global BSS and STA table -  Memory is allocated when needed.*/
  void*                       staTable;

  /*Index of the Self STA */
  wpt_uint8                   ucSelfStaId;
 
  /*Is frame translation enabled */
  wpt_uint8                   bFrameTransEnabled;

  /*AMSDU BD Fix Mask - used by the Fixing routine for Data Path */
  WDI_RxBdType                wdiRxAmsduBdFixMask;

  /*First AMSDU BD - used by the Fixing routine for Data Path */
  WDI_RxBdType                wdiRxAmsduFirstBdCache;

  /*This must be incremented on sta change */
  wpt_uint32                  uBdSigSerialNum;

  /* dpu routing flag
  ! TO DO: - must be set/reset when PS is enabled for UAPSD */  
  wpt_uint8                   ucDpuRF;    
}WDI_ControlBlockType; 




/*---------------------------------------------------------------------------

  DESCRIPTION 
    WLAN DAL Request Processing function definition. 
    
  PARAMETERS 

   IN
   pWDICtx:         pointer to the WLAN DAL context 
   pEventData:      pointer to the event information structure 
 
   
  RETURN VALUE
    The result code associated with performing the operation  

---------------------------------------------------------------------------*/
typedef WDI_Status (*WDI_ReqProcFuncType)( WDI_ControlBlockType*  pWDICtx,
                                           WDI_EventInfoType*     pEventData);


/*---------------------------------------------------------------------------

  DESCRIPTION 
    WLAN DAL Response Processing function definition. 
    
  PARAMETERS 

   IN
   pWDICtx:         pointer to the WLAN DAL context 
   pEventData:      pointer to the event information structure 
 
   
  RETURN VALUE
    The result code associated with performing the operation  

---------------------------------------------------------------------------*/
typedef WDI_Status (*WDI_RspProcFuncType)( WDI_ControlBlockType*  pWDICtx,
                                           WDI_EventInfoType*     pEventData);




/*==========================================================================
              MAIN DAL FSM Definitions and Declarations 
==========================================================================*/

/*--------------------------------------------------------------------------- 
   DAL Control Path Main FSM  
 ---------------------------------------------------------------------------*/
#define WDI_STATE_TRANSITION(_pctx, _st)   (_pctx->uGlobalState = _st)



/*---------------------------------------------------------------------------
  DAL Main Event type
---------------------------------------------------------------------------*/      
typedef enum
{
  /* Start request received from UMAC */
  WDI_START_EVENT          = 0,

  /* Stop request received from UMAC */
  WDI_STOP_EVENT           = 1,

  /* HAL request received from UMAC*/
  WDI_REQUEST_EVENT        = 2,

  /* HAL Response received from device */
  WDI_RESPONSE_EVENT       = 3,

  /* Close request received from UMAC */
  WDI_CLOSE_EVENT          = 4,

  WDI_MAX_EVENT

}WDI_MainEventType;

/*---------------------------------------------------------------------------

  DESCRIPTION 
    Main DAL state machine function definition. 
    
  PARAMETERS 

   IN
   pWDICtx:         pointer to the WLAN DAL context 
   pEventData:      pointer to the event information structure 
 
   
  RETURN VALUE
    The result code associated with performing the operation  

---------------------------------------------------------------------------*/
typedef WDI_Status (*WDI_MainFuncType)( WDI_ControlBlockType*  pWDICtx,
                                        WDI_EventInfoType*     pEventData);

/*---------------------------------------------------------------------------
  MAIN DAL FSM Entry type
---------------------------------------------------------------------------*/      
typedef struct
{
  WDI_MainFuncType  pfnMainTbl[WDI_MAX_EVENT];
} WDI_MainFsmEntryType;

/*Macro to check for valid session id*/
#define WDI_VALID_SESSION_IDX(_idx)  ( _idx < WDI_MAX_BSS_SESSIONS ) 

/*========================================================================== 
 
                      DAL INTERNAL FUNCTION DECLARATION
 
==========================================================================*/ 

/**
 @brief Helper routine for retrieving the PAL Context from WDI - 
        can be used by CTS, DTS, DXE and othe DAL internals 
 
 @param  None
  
 @see
 @return pointer to the context 
*/
WPT_INLINE void* WDI_GET_PAL_CTX( void );

/*---------------------------------------------------------------------------
                    MAIN DAL FSM Function Declarations
---------------------------------------------------------------------------*/      
/**
 @brief WDI_PostMainEvent - Posts an event to the Main FSM

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         wdiEV:           event posted to the main DAL FSM
         pEventData:      pointer to the event information
         structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_PostMainEvent
(
  WDI_ControlBlockType*  pWDICtx, 
  WDI_MainEventType      wdiEV, 
  WDI_EventInfoType*     pEventData
  
);

/*--------------------------------------------------------------------------
  INIT State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStart
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Response function for state INIT

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainRspInit
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Close function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
        
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainClose
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/*--------------------------------------------------------------------------
  STARTED State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for state STARTED

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
      
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStartStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Stop function for state STARTED

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
         uEventDataSize:  size of the data sent in event
         pCBfnc:          cb function for event response
         pUserData:       user data 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStopStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Request function for state started

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
        
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainReqStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Response function for all states except INIT

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/*--------------------------------------------------------------------------
  BUSY State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStartBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Stop function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStopBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Request function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainReqBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Main FSM Close function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainCloseBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);




/*========================================================================
          Main DAL Control Path Request Processing API 
========================================================================*/

/**
 @brief Process Start Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Stop Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStopReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Close Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessCloseReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Init Scan Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessInitScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Start Scan Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process End Scan Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEndScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Finish Scan Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFinishScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Join Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessJoinReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Config BSS Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigBSSReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Del BSS Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBSSReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Post Assoc Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessPostAssocReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Del STA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelSTAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Set BSS Key Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBssKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Remove BSS Key Request function (called when Main    
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveBssKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Set STA KeyRequest function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Remove STA Key Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Add TSpec Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddTSpecReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Del TSpec Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelTSpecReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Update EDCA Params Request function (called when
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Add BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Del BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Channel Switch Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessChannelSwitchReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Config STA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigStaReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Set Link State Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetLinkStateReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Get Stats Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessGetStatsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Update Cfg Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateCfgReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/*========================================================================
          Main DAL Control Path Response Processing API 
========================================================================*/


/**
 @brief Process Start Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Stop Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStopRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Close Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessCloseRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Init Scan Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessInitScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Start Scan Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process End Scan Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEndScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Finish Scan Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFinishScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Join Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessJoinRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Config BSS Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigBSSRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Del BSS Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBSSRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Post Assoc Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessPostAssocRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Del STA Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelSTARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Set BSS Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBssKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Remove BSS Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveBssKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Set STA Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Remove STA Key Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Add TSpec Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddTSpecRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Del TSpec Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelTSpecRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Update EDCA Parameters Rsp function (called when a  
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Add BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Del BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Channel Switch Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessChannelSwitchRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Config STA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigStaRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Set Link State Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetLinkStateRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Get Stats Rsp function (called when a response is   
        being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessGetStatsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

	
/**
 @brief Process Update Cfg Rsp function (called when a response is  
        being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateCfgRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/*==========================================================================
                        Indications from HAL
 ==========================================================================*/
/**
 @brief Process Low RSSI Indication function (called when an 
        indication of this kind is being received over the bus
        from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessLowRSSIInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Missed Beacon Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessMissedBeaconInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process Unk Addr Frame Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUnkAddrFrameInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);


/**
 @brief Process MIC Failure Indication function (called when an 
        indication of this kind is being received over the bus
        from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessMicFailureInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Process Fatal Failure Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFatalErrorInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/*========================================================================
         Internal Helper Routines 
========================================================================*/

/**
 @brief WDI_CleanCB - internal helper routine used to clean the 
        WDI Main Control Block
 
 @param pWDICtx - pointer to the control block

 @return Result of the function call
*/
WPT_INLINE WDI_Status
WDI_CleanCB
(
  WDI_ControlBlockType*  pWDICtx
);

/**
 @brief Main FSM Close function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WPT_INLINE WDI_Status
WDI_ProcessRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Get message helper function - it allocates memory for a 
        message that is to be sent to HAL accross the bus and
        prefixes it with a send message header 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         wdiReqType:      type of the request being sent
         uBufferLen:      message buffer len
         pMsgBuffer:      resulting allocated buffer
         puDataOffset:    offset in the buffer where the caller
         can start copying its message data
         puBufferSize:    the resulting buffer size (offset+buff
         len)
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_GetMessageBuffer
( 
  WDI_ControlBlockType*  pWDICtx, 
  WDI_RequestEnumType    wdiReqType, 
  wpt_uint16             usBufferLen,
  wpt_uint8**            pMsgBuffer, 
  wpt_uint16*            pusDataOffset, 
  wpt_uint16*            pusBufferSize
);

/**
 @brief WDI_DetectedDeviceError - called internally by DAL when 
        it has detected a failure in the device 
 
 @param  pWDICtx:        pointer to the WLAN DAL context 
         usErrorCode:    error code detected by WDI or received
                         from HAL
  
 @see
 @return None 
*/
void
WDI_DetectedDeviceError
(
  WDI_ControlBlockType*  pWDICtx,
  wpt_uint16             usErrorCode
);

/*=========================================================================
                   QUEUE SUPPORT UTILITY FUNCTIONS 
=========================================================================*/

/**
 @brief    Utility function used by the DAL Core to help queue a 
           request that cannot be processed right away. 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: - pointer to the evnt info that needs to be
    queued 
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueuePendingReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief    Utility function used by the DAL Core to clear any 
           pending requests - all req cb will be called with
           failure and the queue will be emptied.
 @param 
    
    pWDICtx: - pointer to the WDI control block
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_ClearPendingRequests
( 
  WDI_ControlBlockType*  pWDICtx
);

/**
 @brief    This callback is invoked by the wpt when a timer that 
           we started on send message has expire - this should
           never happen - it means device is stuck and cannot
           reply - trigger catastrophic failure 
 @param 
    
    pUserData: the callback data of the user (ptr to WDI CB)
    
 @see 
 @return None 
*/
void 
WDI_ResponseTimerCB
(
  void *pUserData
);

/*==========================================================================
                     CONTRL TRANSPORT INTERACTION
 
    Callback function registered with the control transport - for receiving
    notifications and packets 
==========================================================================*/
/**
 @brief    This callback is invoked by the control transport 
   when it wishes to send up a notification like the ones
   mentioned above.
 
 @param
    
    wctsHandle:       handle to the control transport service 
    wctsEvent:        the event being notified
    wctsNotifyCBData: the callback data of the user 
    
 @see  WCTS_OpenTransport
  
 @return None 
*/
void 
WDI_NotifyMsgCTSCB
(
  WCTS_HandleType        wctsHandle, 
  WCTS_NotifyEventType   wctsEvent,
  void*                  wctsNotifyCBData
);

/**
 @brief    This callback is invoked by the control transport 
           when it wishes to send up a packet received over the
           bus.
 
 @param
    
    wctsHandle:  handle to the control transport service 
    pMsg:        the packet
    uLen:        the packet length
    wctsRxMsgCBData: the callback data of the user 
    
 @see  WCTS_OpenTransport
  
 @return None 
*/
void 
WDI_RXMsgCTSCB 
(
  WCTS_HandleType       wctsHandle, 
  void*                 pMsg,
  wpt_uint32            uLen,
  void*                 wctsRxMsgCBData
);

/**
 @brief Process response helper function 

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessResponse
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Send message helper function - sends a message over the 
        bus using the control tranport and saves some info in
        the CB 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pSendBuffer:     buffer to be sent
  
         uSendSize          size of the buffer to be sent
         pRspCb:            response callback - save in the WDI
         CB
         pUserData:         user data associated with the
         callback
         wdiExpectedResponse: the code of the response that is
         expected to be rx-ed for this request
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_SendMsg
( 
  WDI_ControlBlockType*  pWDICtx,  
  wpt_uint8*             pSendBuffer, 
  wpt_uint32             uSendSize, 
  void*                  pRspCb, 
  void*                  pUserData,
  WDI_ResponseEnumType   wdiExpectedResponse
);

/**
 @brief    Utility function used by the DAL Core to help dequeue
           and schedule for execution a pending request 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: - pointer to the evnt info that needs to be
    queued 
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_DequeuePendingReq
(
  WDI_ControlBlockType*  pWDICtx
);

/**
 @brief    Utility function used by the DAL Core to help queue 
           an association request that cannot be processed right
           away.- The assoc requests will be queued by BSSID 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: pointer to the evnt info that needs to be queued
    macBSSID: bssid
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueueNewAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData,
  wpt_macAddr            macBSSID
); 

/**
 @brief    Utility function used by the DAL Core to help queue 
           an association request that cannot be processed right
           away.- The assoc requests will be queued by BSSID 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pSession: - session in which to queue
    pEventData: pointer to the event info that needs to be
    queued
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueueAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_BSSSessionType*    pSession,
  WDI_EventInfoType*     pEventData
);

/**
 @brief Helper routine used to init the BSS Sessions in the WDI control block 
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
  
 @see
*/
void
WDI_ResetAssocSessions
( 
  WDI_ControlBlockType*   pWDICtx
);

/**
 @brief Helper routine used to find an empty session in the WDI 
        CB
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
         pSession:      pointer to the session (if found) 
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindEmptySession
( 
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType**    ppSession
);

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         macBSSID:  BSSID of the session
         ppSession: out pointer to the session (if found)
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSession
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_macAddr             macBSSID,
  WDI_BSSSessionType**    ppSession
);


/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         usBssIdx:  BSS Index of the session
         ppSession: out pointer to the session (if found)
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSessionByBSSIdx
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              usBssIdx,
  WDI_BSSSessionType**    ppSession
);

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         usBssIdx:  BSS Index of the session
         ppSession: out pointer to the session (if found)
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSessionByIdx
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              usBssIdx,
  WDI_BSSSessionType**    ppSession
);

/**
 @brief Helper routine used to find a session based on the BSSID 
 @param  pContext:   pointer to the WLAN DAL context 
 @param  pDPContext:   pointer to the Datapath context 
  
 @see
 @return 
*/
WPT_INLINE void 
WDI_DS_AssignDatapathContext 
(
  void *pContext, 
  void *pDPContext
);

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pContext:   pointer to the WLAN DAL context 
  
 @see
 @return pointer to Datapath context
*/
WPT_INLINE void * 
WDI_DS_GetDatapathContext 
(
  void *pContext
);

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pContext:   pointer to the WLAN DAL context 
 @param  pDTDriverContext:   pointer to the Transport Driver context 
  
 @see
 @return void
*/
WPT_INLINE void  
WDT_AssignTransportDriverContext 
(
  void *pContext, 
  void *pDTDriverContext
);

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
  
 @see
 @return pointer to datapath context 
*/
WPT_INLINE void * 
WDT_GetTransportDriverContext 
(
  void *pContext
);

#endif /*WLAN_QCT_WDI_I_H*/

