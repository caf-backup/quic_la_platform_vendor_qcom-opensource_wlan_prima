#ifndef __Q_WLAN_FW_VERIFY_H
#define __Q_WLAN_FW_VERIFY_H

/*===========================================================================

FILE: 
  qwlanfw_verify.h

BRIEF DESCRIPTION:
   Defines message types that are passed to the Quarky  from perl harness
   through a socket

DESCRIPTION:
  qwlanfw_verify.h contains: 
  ** definitions of Ctrl Messages understood by Gen6 FW which is sent through
  ** Perl harness. 
  ** Dot11 definitions understood by Gen6 FW.
  ** May include HW related definitions.
  This file should be used for Host side development.

                Copyright (c) 2008 Qualcomm Technologies, Inc.
                All Right Reserved.
                Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

$Header$
$DateTime$

when       who            what, where, why
--------   ---          -----------------------------------------------------
02/08/08   Ramaswamy BM      Created

===========================================================================*/

/*===========================================================================
   ARCHITECTURE HEADER FILES
===========================================================================*/

#include <qwlanfw.h>

/*===========================================================================
   WLAN FIRMWARE VERIFICATION Message Decleration 
===========================================================================*/


/* Messages used to do the firmware verification */

typedef enum
{
   
    /* Read & Write REGISTER */
    HOST_PE_VERI_WRITE_REG,                         //MsgStruct: tQwlanfw_htop_WriteReg                
    HOST_PE_VERI_READ_REG,                          //MsgStruct: tQwlanfw_htop_ReadReg

    /* Phy Requests */ //FIX THIS
    PE_HOST_VERI_STORE_FLASH_MEMORY_REQ,            //MsgStruct: tQwlanfw_ptoh_StoreFlashMemoryReq
    
 
    /* Scan */
    HOST_PE_VERI_SET_CHAN_LIST_REQ = QWLANFW_HOST_PE_SET_CHAN_LIST_REQ,   //MsgStruct: tQwlanfw_htop_SetChanReq                                        
    HOST_PE_VERI_SCAN_REQ = QWLANFW_HOST_PE_SCAN_REQ,                     //MsgStruct: tQwlanfw_htop_ScanReq                             
    HOST_PE_VERI_FINISH_SCAN_REQ = QWLANFW_HOST_PE_FINISH_SCAN_REQ,       //MsgStruct: tQwlanfw_htop_FinishScanReq
                                         
    /* Join */
    HOST_PE_VERI_JOIN_REQ = QWLANFW_HOST2PE_JOIN_REQ,                     //MsgStruct: tQwlanfw_htop_JoinReq
   
    /* Security */
    HOST_PE_VERI_SET_KEY_REQ = QWLANFW_HOST_PE_SET_KEY_REQ,                //MsgStruct: tQwlanfw_htop_SetKeyReq
    HOST_PE_VERI_DEL_KEY_REQ = QWLANFW_HOST_PE_DEL_KEY_REQ,                //MsgStruct: tQwlanfw_htop_DelKeyReq
    

    /* TSpec */
    HOST_PE_VERI_ADD_TS_REQ = QWLANFW_HOST_PE_ADD_TS_REQ,                  //MsgStruct: tQwlanfw_htop_AddTsReq   
    HOST_PE_VERI_DEL_TS_REQ = QWLANFW_HOST_PE_DEL_TS_REQ,                  //MsgStruct: tQwlanfw_htop_DelTsReq 
    
    /* IBSS */
    HOST_PE_VERI_IBSS_START_REQ = QWLANFW_HOST_PE_IBSS_START_REQ,          //MsgStruct: tQwlanfw_htop_IbssStartReq
    HOST_PE_VERI_IBSS_STOP_REQ = QWLANFW_HOST_PE_IBSS_STOP_REQ,            //MsgStruct: tQwlanfw_htop_IbssStopReq
    HOST_PE_VERI_IBSS_ADD_PEER_REQ = QWLANFW_HOST_PE_IBSS_ADD_PEER_REQ     //MsgStruct: tQwlanfw_htop_IbssAddPeerReq
    HOST_PE_VERI_IBSS_DEL_PEER_REQ = QWLANFW_HOST_PE_IBSS_DEL_PEER_REQ,    //MsgStruct: tQwlanfw_htop_IbssDelPeerReq

    /* Statistics */
    HOST_PE_VERI_STA_STATS_REQ = QWLANFW_HOST_PE_STA_STATS_REQ,            //MsgStruct: tQwlanfw_htop_StaStatReq                           
    HOST_PE_VERI_GLOBAL_STATS_REQ= QWLANFW_HOST_PE_GLOBAL_STATS_REQ,       //MsgStruct: tQwlanfw_htop_GlobalStatReq
 
    /* Power Save */                              
    HOST_PE_VERI_PS_IMPS_START_IND = QWLANFW_HOST_PE_IMPS_START_IND,       //MsgStruct: tQwlanfw_htop_ImpsStartInd                 
    HOST_PE_VERI_PS_BMPS_START_IND = QWLANFW_HOST_PE_BMPS_START_IND,       //MsgStruct: tQwlanfw_htop_BmpsStartInd   
    HOST_PE_VERI_PS_BMPS_STOP_IND  = QWLANFW_HOST_PE_BMPS_STOP_IND ,       //MsgStruct: tQwlanfw_htop_BmpsStopInd
    HOST_PE_VERI_PS_WOW_SET_PTRN_REQ  = QWLANFW_HOST_PE_WOW_SET_PTRN_REQ   //MsgStruct: tQwlanfw_htop_WowSetPtrnReq         

   /* BA functionality */ 
    HOST_PE_VERI_BA_ADDBA_REQ = QWLANFW_HOST_PE_BA_ADDBA_REQ,              //MsgStruct: tQwlanfw_htop_BaAddbaReq                           
    HOST_PE_VERI_BA_DELBA_REQ = QWLANFW_HOST_PE_BA_DELBA_REQ,              //MsgStruct: tQwlanfw_htop_BaDelbaReq

    /* Add Message type as when   they are required. */
 
    /* Scan */
    PE_HOST_VERI_SET_CHAN_LIST_RSP = QWLANFW_PE_HOST_SET_CHAN_LIST_RSP,    //MsgStruct: tQwlanfw_ptoh_SetChanListRsp
    PE_HOST_VERI_SCAN_RSP = QWLANFW_PE_HOST_SCAN_RSP,                      //MsgStruct: tQwlanfw_ptoh_ScanResp 
    PE_HOST_VERI_FINISH_SCAN_RSP = QWLANFW_PE_HOST_FINISH_SCAN_RSP,        //MsgStruct: tQwlanfw_ptoh_FinishScanRsp 
    PE_HOST_VERI_BCN_PR_RCVD_IND = QWLANFW_PE_HOST_BCN_PR_RCVD_IND,        //MsgStruct: tQwlanfw_ptoh_BcnPrRcvdInd

    /* Join */
    PE_HOST_VERI_JOIN_RSP = QWLANFW_PE2HOST_JOIN_RSP,                      //MsgStruct: tQwlanfw_ptoh_JoinRsp
   
    /* Security */
    PE_HOST_VERI_SET_KEY_RSP = QWLANFW_PE_HOST_SET_KEY_RSP,                //MsgStruct: tQwlanfw_ptoh_SetKeyRsp 
    PE_HOST_VERI_DEL_KEY_RSP = QWLANFW_PE_HOST_DEL_KEY_RSP,                //MsgStruct: tQwlanfw_ptoh_DelKeyRsp 

    /* TSpec */
    PE_HOST_VERI_ADD_TS_RSP = QWLANFW_PE_HOST_ADD_TS_RSP,                  //MsgStruct: tQwlanfw_ptoh_AddTsRsp
    PE_HOST_VERI_DEL_TS_RSP = QWLANFW_PE_HOST_DEL_TS_RSP,                  //MsgStruct: tQwlanfw_ptoh_DelTsRsp

    /* IBSS */

    PE_HOST_VERI_IBSS_START_RSP= QWLANFW_PE_HOST_IBSS_START_RSP,           //MsgStruct: tQwlanfw_ptoh_IbssStartRsp 
    PE_HOST_VERI_IBSS_STOP_RSP = QWLANFW_PE_HOST_IBSS_STOP_RSP,            //MsgStruct: tQwlanfw_ptoh_IbssStopRsp
    PE_HOST_VERI_IBSS_PEER_ADDED_IND = QWLANFW_PE_HOST_IBSS_PEER_ADDED_IND,         //MsgStruct: tQwlanfw_ptoh_IbssPeerAddedInd
    PE_HOST_VERI_IBSS_PEER_COALESCED_IND = QWLANFW_PE_HOST_IBSS_PEER_COALESCED_IND,        //MsgStruct: tQwlanfw_ptoh_IbssPeerCoalesedInd
    PE_HOST_VERI_IBSS_PEER_ACTIVE_IND = QWLANFW_PE_HOST_IBSS_PEER_ACTIVE_IND,              //MsgStruct: tQwlanfw_ptoh_IbssPeerActiveInd
    PE_HOST_VERI_IBSS_PEER_INACTIVE_IND = QWLANFW_PE_HOST_IBSS_PEER_INACTIVE_IND,          //MsgStruct: tQwlanfw_ptoh_IbssPeerInactiveInd 
  
    /* Statistics */
    PE_HOST_VERI_STA_STATS_RSP = QWLANFW_PE_HOST_STA_STATS_RSP,            //MsgStruct: tQwlanfw_ptoh_StaStatsRsp
    PE_HOST_VERI_GLOBAL_STATS_RSP = QWLANFW_PE_HOST_GLOBAL_STATS_RSP,             //MsgStruct: tQwlanfw_ptoh_GlobalStatsRsp       

    /* Power Save */
    PE_HOST_VERI_PS_IMPS_STATUS_IND= QWLANFW_PE_HOST_IMPS_STATUS_IND,             //MsgStruct: tQwlanfw_ptoh_ImpsStatusInd
    PE_HOST_VERI_PS_BMPS_STATUS_IND = QWLANFW_PE_HOST_BMPS_STATUS_IND,            //MsgStruct: tQwlanfw_ptoh_BmpsStatusInd
    PE_HOST_VERI_PS_WOW_SET_PTRN_RSP= QWLANFW_PE_HOST_WOW_SET_PTRN_RSP,           //MsgStruct: tQwlanfw_ptoh_WowSetPtrnInd

    /* Block Ack */
    PE_HOST_VERI_BA_ADDBA_RSP = QWLANFW_PE_HOST_BA_ADDBA_RSP,                     //MsgStruct: tQwlanfw_ptoh_BaAddbaRsp
    PE_HOST_VERI_BA_ADDBA_IND = QWLANFW_PE_HOST_BA_ADDBA_IND,                     //MsgStruct: tQwlanfw_ptoh_BaAddbaInd 
    PE_HOST_VERI_BA_DELBA_RSP = QWLANFW_PE_HOST_BA_DELBA_RSP,                     //MsgStruct: tQwlanfw_ptoh_BaDelbaRsp
    PE_HOST_VERI_BA_DELBA_IND = QWLANFW_PE_HOST_BA_DELBA_IND                      //MsgStruct: tQwlanfw_ptoh_BaDelbaInd    
} eQwlanfw_Veri_MsgId;




/* Packet format over interface from test Server and to test server from Quarky */
/* Definition for Message Header used for Firmware verification */
typedef struct
{
    eQwlanfw_Veri_MsgId     msgType;      
    uint16		    usMsgLen;
    eQwlanfw_Veri_STATUS    status;
    /*    
      CAUTION: This structure may not end here.
      This might be followed by the (message body)  data required 
      for that  message
    */
} tQwlanfw_veri_MsgHeader;



/* Definition for Read Register request message from test harness */
typedef struct _Qwlanfw_ReadRegReq
{
   /* HOST_PE_VERI_READ_REG */

    tQwlanfw_veri_MsgHeader      hdr;
    uint32                       addr;
    uint32                       value;
} tQwlanfw_htop_ReadReg;


/* Definition for Write  Register request message from test harness */
typedef struct _Qwlanfw_WriteRegReq
{
   /* HOST_PE_VERI_WRITE_REG */

    tQwlanfw_veri_MsgHeader      hdr;
    uint32                       addr;
    uint32                       value;
} tQwlanfw_htop_WriteReg;



/* Definition for set channel list request message sent to Quarky from test server */

typedef struct _Qwlanfw_SetChanListReqStruct
{ 
     tQwlanfw_veri_MsgHeader         hdr;
     /* HOST_PE_VERI_SET_CHAN_LIST_REQ */
     Qwlanfw_ChanType	            dot11Chan[n];
     /* CAUTION: This structure does not end here.
      This is followed by dot11Chan[n], where n is 
      the number of channels sent by Host. For all 
      implementations it should be assumed with the 
      following declaration here:
     */
} tQwlanfw_htop_SetChanReq;




/* Definition for scan request message to Quarky from test server */
typedef struct _Qwlanfw_ScanReqStruct
{
     /*HOST_PE_VERI_SCAN_REQ*/
     tQwlanfw_veri_MsgHeader         hdr; 
     Qwlanfw_BssType                 bssType;
     Qwlanfw_MacAddrType	     bssid;
     Qwlanfw_MacSsidType             ssid;
    /* CAUTION: This structure does not end here.
       This is followed by ucIeByteStream[n] to be inserted into 
       the probe request, where n is total num of bytes of IEs
       sent by Host. Assume the following for coding:
       uint8                       ucIeByteStream[n];
     */
} tQwlanfw_htop_ScanReq;



/* Definition for finish/abort scan request message to Quarky from test server */
  
typedef struct _Qwlanfw_FinishScanReqStruct
{
     /*HOST_PE_VERI_FINISH_SCAN_REQ*/
     tQwlanfw_veri_MsgHeader         hdr;
   
} tQwlanfw_htop_FinishScanReq;



/* Definition for Join Request ctrl message sent to Quarky from test server */


typedef struct _Qwlanfw_JoinReqStruct
{
    /* HOST_PE_VERI_JOIN_REQ*/

     tQwlanfw_veri_MsgHeader         hdr;
     Qwlanfw_MacAddrType	     bssid;
     Qwlanfw_MacSsidType	     ssid;	
     Qwlanfw_ChanId                  chanId;
     /* BcnIeLen is the length of the beacon IEs inserted 
      in this control message. This variable gives the 
      boundary of beacon IEs and IEs that are to be 
      inserted in Assoc Resp
     */
     uint16                         usBcnIeLen; 
     /* CAUTION: This structure does not end here.
      This is followed by ucBcnIeStream[m] and 
      ucAssocRspIeStream[n], where m,n are the total 
      num of bytes of IEs sent by Host for Beacon and 
      Assoc Rsp respectively. For all implementations 
      it should be assumed with the following declaration 
      here:
     */
      uint8                       ucBcnIeStream[m];
      uint8                       ucAssocRspIeStream[n];
} tQwlanfw_htop_JoinReq;



/* Definition for Set key request message sent over socket to Quarky from test server */

typedef struct _Qwlanfw_SetKeyReqStruct
{
     /*HOST_PE_VERI_SET_KEY_REQ*/
     tQwlanfw_veri_MsgHeader           hdr; 
     Qwlanfw_MacAddrType     	       macAddr;
     Qwlanfw_EncType	               encType;
     uint8                	       ucNumKeys;
     /* CAUTION: This structure does not end here.
      This is followed by Qwlanfw_KeyType[n], where n is 
      the number of keys sent by Host. For all 
      implementations it should be assumed with the 
      following declaration here:
      Qwlanfw_KeyType           dot11Key[uNumKeys];
     */
	
} tQwlanfw_htop_SetKeyReq;   




/* Definition for Delete key request message to Quarky from test server */

typedef struct _Qwlanfw_DelKeyReqStruct
{
     /*HOST_PE_VERI_DEL_KEY_REQ*/
 
     tQwlanfw_veri_MsgHeader          hdr; 
     Qwlanfw_MacAddrType     	      macAddr;
     Qwlanfw_EncType	              encType;
     uint8                	      ucKeyId;
     uint8                	      bIsUnicast : 1;

} tQwlanfw_htop_DelKeyReq;



/* Definition for Add TSpec Request ctrl message */
typedef struct  _Qwlanfw_AddTsReqStruct
{
   /*QWLANFW_HOST_PE_ADD_TS_REQ*/
   uint16                         usMsgType;
   uint16                         usMsgLen;
   TsInfoType                     tsInfo;
   uint16                         usNomMsduSz;
   uint32                         uMeanDataRate;
   uint32                         uMinPhyRate;
   uint16                         usSurplusBw;
} tQwlanfw_htop_AddTsReq;



/* Definition for Delete TSpec Request message sent to Quarky from test server */

typedef struct _Qwlanfw_DelTsReqStruct
{
    /*HOST_PE_DEL_TS_REQ*/

    tQwlanfw_veri_MsgHeader      hdr;
    TsInfoType                   tsInfo;
    uint16                       usReasonCode;

} tQwlanfw_htop_DelTsReq;
			

#endif // __Q_WLAN_FW_VERIFY_H 


