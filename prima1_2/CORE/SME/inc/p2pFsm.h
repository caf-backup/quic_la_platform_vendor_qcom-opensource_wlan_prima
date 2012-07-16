/*
* Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

/******************************************************************************
*
* Name:  p2pFsm.h
*
* Description: P2P FSM defines.
*
* Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#ifndef __P2P_FSM_H__
#define __P2P_FSM_H__

#include "vos_types.h"

typedef enum eP2P_TRIGGER {
   eP2P_TRIGGER_SCAN_COMPLETE,
   eP2P_TRIGGER_DEVICE_MODE_DEVICE,
   eP2P_TRIGGER_DEVICE_MODE_GO,
   eP2P_TRIGGER_DEVICE_MODE_CLIENT,
   eP2P_TRIGGER_LISTEN_COMPLETE,
   eP2P_TRIGGER_SEARCH_COMPLETE,
   eP2P_TRIGGER_GROUP_FORMATION,
   eP2P_TRIGGER_DISCONNECTED
} tP2P_TRIGGER;

typedef enum eP2PFsmState {
   eP2P_STATE_INVALID,
   eP2P_STATE_DISCONNECTED,
   eP2P_STATE_SCAN_INITIATED,
   eP2P_STATE_FIND_LISTEN,
   eP2P_STATE_FIND_SEARCH,
   eP2P_STATE_GROUP_FORMATION,
   eP2P_STATE_GO_ROLE,
   eP2P_STATE_CLIENT_ROLE
} tP2PFsmState;

eHalStatus p2pFsm(tp2pContext *p2pContext, tP2P_TRIGGER trigger);
eHalStatus p2pFsmScanRequest(tp2pContext *p2pContext);
eHalStatus p2pFsmListenRequest(tp2pContext *p2pContext);
tANI_U16 p2p_getListenInterval( tp2pContext *p2pContext );
eHalStatus p2pFsm_toFindSearch(tp2pContext *p2pContext);
tANI_U8 p2pFsm_getListenChannel( tp2pContext *p2pContext );
tANI_U8* p2pFsm_GetNextSearchChannel(tp2pContext *p2pContext);
eHalStatus getChannelInfo(tp2pContext *p2pContext, tCsrChannelInfo *scanChannelInfo, 
                           ep2pDiscoverType discoverType);
#endif //__P2P_FSM_H__
