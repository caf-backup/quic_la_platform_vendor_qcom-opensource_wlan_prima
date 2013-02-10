/*
 * File:        halBtc.c
 * Description: This file contains all the interface functions to
 *              interact with the BTC module in firmware
 *
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 02/25/2008 davidliu    Created the functions for sending BTC message to
 *                        FW, configuring FW BTC sys config.
 *
 *
 */
#include "palApi.h"
#include "aniGlobal.h"
#include "halDebug.h"
#include "halFwApi.h"
#include "halFw.h"
#include "halMailbox.h"
#include "halBtc.h"


void halBtc_SetBtcCfg(tpAniSirGlobal pMac, void *pBuffer)
{
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    eHalStatus status;
    tpSmeBtcConfig pBtcCfg = (tpSmeBtcConfig)pBuffer;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    if(pFwConfig){
        
        pFwConfig->btcBtIntervalMode1     = pBtcCfg->btcBtIntervalMode1;      
        pFwConfig->btcWlanIntervalMode1   = pBtcCfg->btcWlanIntervalMode1;
        pFwConfig->btcActionOnPmFailMode1 = pBtcCfg->btcActionOnPmFail;
        pFwConfig->btcExecutionMode       = pBtcCfg->btcExecutionMode;
        pFwConfig->btcConsBtSlotsToBlockDuringDhcp = pBtcCfg->btcConsBtSlotsToBlockDuringDhcp;

        // Write the configuration parameters in the memory mapped for
        // system configuration parameters
        status = halFW_UpdateSystemConfig(pMac,
                pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
                sizeof(*pFwConfig));
        if(status != eHAL_STATUS_SUCCESS)
            HALLOGE(halLog(pMac,LOGE, FL("Failed to config FW BTC config")));
    }

#endif //WLAN_MDM_CODE_REDUCTION_OPT

    palFreeMemory( pMac->hHdd, pBuffer);
    return;

}


void halBtc_SendBtEventToFW(tpAniSirGlobal pMac, void *pBuffer)
{
#ifndef WLAN_MDM_CODE_REDUCTION_OPT
    eHalStatus status;
    tANI_U16 dialogToken = 0;
    tANI_U16 size = sizeof(Qwlanfw_BtEventMsgType); 
    tSmeBtEvent *pBtEventMsg = (tSmeBtEvent *)pBuffer;
    struct sBtEventFwMesg {
        Qwlanfw_BtEventMsgType msgHdr; 
        union
        {
            Qwlanfw_BtAclConnectionParamType  tAclConnection;
            Qwlanfw_BtSyncConnectionParamType tSyncConnection;
            Qwlanfw_BtAclModeChangeParamType  tAclModeChange;
            Qwlanfw_BtDisconnectParamType     tDisconnect;
        }uBtEvent;
    } msg;
    
    if((tSmeBtEventType)BT_EVENT_TYPE_MAX < pBtEventMsg->btEventType){
        HALLOGE(halLog(pMac,LOGE, FL("BT event type %d out of range"), pBtEventMsg->btEventType ));
        goto out;
    }

    palZeroMemory( pMac->hHdd, &msg, sizeof(struct sBtEventFwMesg));

    msg.msgHdr.btEventType = (tANI_U32)pBtEventMsg->btEventType;

    switch(pBtEventMsg->btEventType){
        case BT_EVENT_CREATE_ACL_CONNECTION:
        case BT_EVENT_ACL_CONNECTION_COMPLETE:
            size += sizeof(Qwlanfw_BtAclConnectionParamType);
            msg.uBtEvent.tAclConnection.connectionHandle = pBtEventMsg->uEventParam.btAclConnection.connectionHandle;
            msg.uBtEvent.tAclConnection.status = pBtEventMsg->uEventParam.btAclConnection.status;
            msg.uBtEvent.tAclConnection.bdAddr0 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[0];
            msg.uBtEvent.tAclConnection.bdAddr1 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[1];
            msg.uBtEvent.tAclConnection.bdAddr2 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[2];
            msg.uBtEvent.tAclConnection.bdAddr3 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[3];
            msg.uBtEvent.tAclConnection.bdAddr4 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[4];
            msg.uBtEvent.tAclConnection.bdAddr5 = pBtEventMsg->uEventParam.btAclConnection.bdAddr[5];
            break;

        case BT_EVENT_CREATE_SYNC_CONNECTION:
        case BT_EVENT_SYNC_CONNECTION_COMPLETE:
        case BT_EVENT_SYNC_CONNECTION_UPDATED:
            size += sizeof(Qwlanfw_BtSyncConnectionParamType);
            msg.uBtEvent.tSyncConnection.connectionHandle = pBtEventMsg->uEventParam.btSyncConnection.connectionHandle;
            msg.uBtEvent.tSyncConnection.status = pBtEventMsg->uEventParam.btSyncConnection.status;
            msg.uBtEvent.tSyncConnection.linkType = pBtEventMsg->uEventParam.btSyncConnection.linkType;
            msg.uBtEvent.tSyncConnection.scoInterval = pBtEventMsg->uEventParam.btSyncConnection.scoInterval;
            msg.uBtEvent.tSyncConnection.scoWindow = pBtEventMsg->uEventParam.btSyncConnection.scoWindow;
            msg.uBtEvent.tSyncConnection.retransmisisonWindow = pBtEventMsg->uEventParam.btSyncConnection.retransmisisonWindow;
            msg.uBtEvent.tSyncConnection.bdAddr0 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[0];
            msg.uBtEvent.tSyncConnection.bdAddr1 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[1];
            msg.uBtEvent.tSyncConnection.bdAddr2 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[2];
            msg.uBtEvent.tSyncConnection.bdAddr3 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[3];
            msg.uBtEvent.tSyncConnection.bdAddr4 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[4];
            msg.uBtEvent.tSyncConnection.bdAddr5 = pBtEventMsg->uEventParam.btSyncConnection.bdAddr[5];
            break;


        case BT_EVENT_MODE_CHANGED:
            size += sizeof(Qwlanfw_BtAclModeChangeParamType);
            msg.uBtEvent.tAclModeChange.connectionHandle = pBtEventMsg->uEventParam.btAclModeChange.connectionHandle;
            msg.uBtEvent.tAclModeChange.mode = pBtEventMsg->uEventParam.btAclModeChange.mode;
            break;

        case BT_EVENT_DISCONNECTION_COMPLETE:
            size+= sizeof(Qwlanfw_BtDisconnectParamType);
            msg.uBtEvent.tDisconnect.connectionHandle = pBtEventMsg->uEventParam.btDisconnect.connectionHandle;
            break;

        default:
            break;
    }

    status = halFW_SendMsg(pMac, HAL_MODULE_ID_BTC,
        QWLANFW_HOST2FW_BT_EVENT, dialogToken, size, &msg, TRUE, NULL);
out:
#endif //WLAN_MDM_CODE_REDUCTION_OPT

    palFreeMemory( pMac->hHdd, pBuffer);
    return;

}

