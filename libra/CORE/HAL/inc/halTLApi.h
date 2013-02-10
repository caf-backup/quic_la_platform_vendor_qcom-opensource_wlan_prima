/**
 *
 *  @file:      halTLApi.h
 *
 *  @brief:     Provides all the APIs to interact with Transport Layer.
 *
 *  @author:    Lawrie Kurian
 *
 *  Copyright (C) 2008, Qualcomm Technologies, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 05/21/2008  File created.
 */

#ifndef _HALTLAPI_H_
#define _HALTLAPI_H_

#include "halTypes.h"
#include "aniGlobal.h"
#include "sirApi.h"
#include "vos_status.h"
#include "vos_types.h"


#define HAL_TL_TX_SUSPEND_SUCCESS   0
#define HAL_TL_TX_SUSPEND_FAILURE   1
#define HAL_TL_TX_SUCCESS   0
#define HAL_TL_TX_FAILURE   1
#define HAL_TL_SUSPEND_TIMEOUT  2000 //ms unit
#define HAL_TL_TX_FRAME_TIMEOUT 2000 //ms unit
#define HAL_DPU_FEEDBACK_OFFSET 1

#ifdef WLAN_PERF
/* TxBD signature fields
 * serial(8): a monotonically increasing serial # whenever there is a Add/Del STA or Add/Del Key event
 * macHash(16): Hash value of DA
 * tid(4):    TID
 * ucast(1):  Unicast or Broadcast data frame
 */
#define HAL_TXBD_SIG_SERIAL_OFFSET      0   
#define HAL_TXBD_SIG_TID_OFFSET         8
#define HAL_TXBD_SIG_UCAST_DATA_OFFSET  9
#define HAL_TXBD_SIG_MACADDR_HASH_OFFSET 16
#define HAL_TXBD_SIG_MGMT_MAGIC         0xbdbdbdbd

#endif
/* 
 * DESCRIPTION:
 *      Intializes the parameters required to interact with TL
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      arg  :    dummy to match with the functable
 *
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLApiInit(tpAniSirGlobal pMac, void* arg);

/* 
 * DESCRIPTION:
 *      Destroy the parameters required to interact with TL
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      arg   :  dummy to match with the func table
 *
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLApiExit(tpAniSirGlobal pMac, void* arg);


/* 
 * DESCRIPTION:
 *      Callback function provided for TL(transport layer), called after
 *      suspending transmission
 *
 * PARAMETERS:
 *      pVosGCtx:   Pointer to the global vos context
 *      pStaId:  pointer to Station ID for which the TX will be suspended 
 *      void*:  Status Code, specifying success or failure when suspending 
 *              transmission for the particular STA
 *
 * RETURN VALUE:
 *      VOS_STATUS_SUCCESS
 *      VOS_STATUS_FAILURE
 */
VOS_STATUS halTLSuspendTxCallBack(v_PVOID_t pVosGCtx, v_U8_t* pStaId, VOS_STATUS statusCode);

/* 
 * DESCRIPTION:
 *      Function to suspend transmission in the transport layer
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      *staId: Pointer to the Station ID for which the TX will be suspended.
 *              Value of NULL signifies all STA's TX queue needs to be 
 *              suspended.
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLSuspendTx(tpAniSirGlobal pMac, tANI_U8 *pStaId);

/* 
 * DESCRIPTION:
 *      Function to resume transmission in the transport layer for a particular 
 *      station.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      *pStaId:Pointer to the Station ID for which the TX will be resumed.
 *              Value of NULL signifies all STA's TX queue needs to be 
 *              resumed.
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLResumeTx(tpAniSirGlobal pMac, tANI_U8 *pStaId);

/* 
 * DESCRIPTION:
 *      Function to send frame through the TL module
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFrame: Pointer to the frame to be txmitted
 *      frameType:  Type of frame to be transmitted
 *      frameLen:   Length of the frame to be transmitted
 *      tid:    TID on which frame needs to be transmitted
 *      waitForRsp:Flag which specifies to wait for the Rsp
 *      intrAckRsp: Flag which specifies to wait till ACK is recieved
 *      
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halTLSend80211Frame(tpAniSirGlobal pMac, 
        void* pFrame, tANI_U8 frameType, tANI_U16 frameLen, tANI_U8 tid, 
        tANI_U8 intrAckRsp);


/**
 * DESCRIPTION:
 *      Notify TL whenever the RSSI threshold hits low/high.
 *    It is based on the system configuration programmed in the firmware
 *
 * PARAMETERS:
 *      pRSSINotification:  Pointer to the detected RSSI thresholds.
 *
 * RETURN VALUE:
 *      void
 */
void halTLRSSINotification(tpAniSirGlobal pMac, tpSirRSSINotification pRSSINotification);

void halTLGetTxPktCount(tpAniSirGlobal pMac, tANI_U8 curSta, tANI_U8 tId, tANI_U32 *txPcktCount );

#endif /*_HALTLAPI_H_*/
