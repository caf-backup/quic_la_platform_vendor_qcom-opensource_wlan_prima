/******************************************************************************
*
* Name:  btcApi.c
*
* Description: Routines that make up the BTC API.
*
* Copyright 2008 (c) Qualcomm, Incorporated. All Rights Reserved.
* Qualcomm Confidential and Proprietary.
*
******************************************************************************/

#include "aniGlobal.h"
#include "smsDebug.h"
#include "btcApi.h"
#include "cfgApi.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

static void btcLogEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent);
static void btcRestoreHeartBeatMonitoringHandle(void* hHal);
VOS_STATUS btcCheckHeartBeatMonitoring(tHalHandle hHal, tpSmeBtEvent pBtEvent);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
static void btcDiagEventLog (tHalHandle hHal, tpSmeBtEvent pBtEvent);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
/* ---------------------------------------------------------------------------
    \fn btcOpen
    \brief  API to init the BTC Events Layer
    \param  hHal - The handle returned by macOpen.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – success
            VOS_STATUS_SUCCESS – failure
  ---------------------------------------------------------------------------*/
VOS_STATUS btcOpen (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   VOS_STATUS vosStatus;

   /* Initialize BTC configuartion. */
   pMac->btc.btcConfig.btcExecutionMode = BTC_SMART_COEXISTENCE;
   pMac->btc.btcConfig.btcBtIntervalMode1 = 120;
   pMac->btc.btcConfig.btcWlanIntervalMode1 = 30;
   pMac->btc.btcConfig.btcActionOnPmFail = BTC_START_NEXT;
   pMac->btc.btcReady = VOS_FALSE;
   pMac->btc.btcEventState = 0;
   pMac->btc.btcHBActive = VOS_TRUE;


   vosStatus = vos_timer_init( &pMac->btc.restoreHBTimer,
                      VOS_TIMER_TYPE_SW,
                      btcRestoreHeartBeatMonitoringHandle,
                      (void*) hHal);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcOpen: Fail to init timer");
       return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcClose
    \brief  API to exit the BTC Events Layer
    \param  hHal - The handle returned by macOpen.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – success
            VOS_STATUS_SUCCESS – failure
  ---------------------------------------------------------------------------*/
VOS_STATUS btcClose (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   VOS_STATUS vosStatus;

   vosStatus = vos_timer_destroy(&pMac->btc.restoreHBTimer);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcClose: Fail to destroy timer");
       return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}


/* ---------------------------------------------------------------------------
    \fn btcReady
    \brief  fn to inform BTC that eWNI_SME_SYS_READY_IND has been sent to PE.
            This acts as a trigger to send a message to HAL to update the BTC
            related conig to FW. Note that if HDD configures any power BTC
            related stuff before this API is invoked, BTC will buffer all the
            configutaion.
    \param  hHal - The handle returned by macOpen.
    \return VOS_STATUS
  ---------------------------------------------------------------------------*/
VOS_STATUS btcReady (tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
    v_U32_t cfgVal = 0;

    pMac->btc.btcReady = VOS_TRUE;

    // Read heartbeat threshold CFG and save it.
    ccmCfgGetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &cfgVal);
    pMac->btc.btcHBCount = (v_U8_t)cfgVal;

    if (btcSendCfgMsg(hHal, &(pMac->btc.btcConfig)) != VOS_STATUS_SUCCESS)
    {
        return VOS_STATUS_E_FAILURE;
    }

    return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcSignalBTEvent
    \brief  API to signal Bluetooth (BT) event to the WLAN driver. Based on the
            BT event type and the current operating mode of Libra (full power,
            BMPS, UAPSD etc), appropriate Bluetooth Coexistence (BTC) strategy
            would be employed.
    \param  hHal - The handle returned by macOpen.
    \param  pBtEvent -  Pointer to a caller allocated object of type tSmeBtEvent.
                        Caller owns the memory and is responsible for freeing it.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – BT Event not passed to HAL. This can happen
                                   if driver has not yet been initialized or if BTC
                                   Events Layer has been disabled.
            VOS_STATUS_SUCCESS   – BT Event passed to HAL
  ---------------------------------------------------------------------------*/
VOS_STATUS btcSignalBTEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tpSmeBtEvent ptrSmeBtEvent = NULL;
   vos_msg_t msg;

   if( NULL == pBtEvent )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
         "Null pointer for SME BT Event");
      return VOS_STATUS_E_FAILURE;
   }

   if( BTC_SMART_COEXISTENCE != pMac->btc.btcConfig.btcExecutionMode )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
         "BTC execution mode not set to BTC_SMART_COEXISTENCE. BT event will be dropped");
      return VOS_STATUS_E_FAILURE;
   }

   if( pBtEvent->btEventType < 0 || pBtEvent->btEventType >= BT_EVENT_TYPE_MAX )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
         "Invalid BT event %d being passed. BT event will be dropped",
          pBtEvent->btEventType);
      return VOS_STATUS_E_FAILURE;
   }

   switch(pBtEvent->btEventType)
   {
      case BT_EVENT_CREATE_SYNC_CONNECTION:
      case BT_EVENT_SYNC_CONNECTION_UPDATED:
         if(pBtEvent->uEventParam.btSyncConnection.linkType != BT_SCO &&
            pBtEvent->uEventParam.btSyncConnection.linkType != BT_eSCO)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
               "Invalid link type %d for Sync Connection. BT event will be dropped ",
               pBtEvent->uEventParam.btSyncConnection.linkType);
            return VOS_STATUS_E_FAILURE;
         }
         break;

      case BT_EVENT_SYNC_CONNECTION_COMPLETE:
         if((pBtEvent->uEventParam.btSyncConnection.status == BT_CONN_STATUS_SUCCESS) &&
            ((pBtEvent->uEventParam.btSyncConnection.linkType != BT_SCO && pBtEvent->uEventParam.btSyncConnection.linkType != BT_eSCO) ||
             (pBtEvent->uEventParam.btSyncConnection.connectionHandle == BT_INVALID_CONN_HANDLE)))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
               "Invalid connection handle %d or link type %d for Sync Connection. BT event will be dropped ",
               pBtEvent->uEventParam.btSyncConnection.connectionHandle,
               pBtEvent->uEventParam.btSyncConnection.linkType);
            return VOS_STATUS_E_FAILURE;
         }
         break;

      case BT_EVENT_MODE_CHANGED:
         if(pBtEvent->uEventParam.btAclModeChange.mode >= BT_ACL_MODE_MAX)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
               "Invalid mode %d for ACL Connection. BT event will be dropped ",
               pBtEvent->uEventParam.btAclModeChange.mode);
            return VOS_STATUS_E_FAILURE;
         }
         break;
      default:
         break;
   }

   ptrSmeBtEvent = vos_mem_malloc(sizeof(tSmeBtEvent));

   if (NULL == ptrSmeBtEvent)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
         "Not able to allocate memory for BT event");
      return VOS_STATUS_E_FAILURE;
   }

   btcLogEvent(hHal, pBtEvent);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   btcDiagEventLog(hHal, pBtEvent);
#endif

   vos_mem_copy(ptrSmeBtEvent, pBtEvent, sizeof(tSmeBtEvent));

   msg.type = SIR_HAL_SIGNAL_BT_EVENT;
   msg.reserved = 0;
   msg.bodyptr = ptrSmeBtEvent;

   if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_HAL, &msg))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
         "Not able to post SIR_HAL_SIGNAL_BT_EVENT message to HAL");
      vos_mem_free( ptrSmeBtEvent );
      return VOS_STATUS_E_FAILURE;
   }

   // After successfully posting the message, check if heart beat
   // monitoring needs to be turned off
   // done only when station is in associated state (Neglected during IMPS)

   if(csrIsConnStateConnected(pMac))
   {
      (void)btcCheckHeartBeatMonitoring(hHal, pBtEvent);
   }

   return VOS_STATUS_SUCCESS;
}


/* ---------------------------------------------------------------------------
    \fn btcCheckHeartBeatMonitoring
    \brief  API to check whether heartbeat monitoring is required to be disabled
            for specific BT start events which takes significant time to complete
            during which WLAN misses beacons. To avoid WLAN-MAC from disconnecting
            for the not enough beacons received we stop the heartbeat timer during
            this start BT event till the stop of that BT event.
    \param  hHal - The handle returned by macOpen.
    \param  pBtEvent -  Pointer to a caller allocated object of type tSmeBtEvent.
                        Caller owns the memory and is responsible for freeing it.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – Config not passed to HAL.
            VOS_STATUS_SUCCESS – Config passed to HAL
  ---------------------------------------------------------------------------*/
VOS_STATUS btcCheckHeartBeatMonitoring(tHalHandle hHal, tpSmeBtEvent pBtEvent)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   VOS_STATUS vosStatus;

   switch(pBtEvent->btEventType)
   {
      // Start events which requires heartbeat monitoring be disabled.
      case BT_EVENT_INQUIRY_STARTED:
          pMac->btc.btcEventState |= BT_INQUIRY_STARTED;
          break;
      case BT_EVENT_PAGE_STARTED:
          pMac->btc.btcEventState |= BT_PAGE_STARTED;
          break;
      case BT_EVENT_CREATE_ACL_CONNECTION:
          pMac->btc.btcEventState |= BT_CREATE_ACL_CONNECTION_STARTED;
          break;
      case BT_EVENT_CREATE_SYNC_CONNECTION:
          pMac->btc.btcEventState |= BT_CREATE_SYNC_CONNECTION_STARTED;
          break;

      // Stop/done events which indicates heartbeat monitoring can be enabled
      case BT_EVENT_INQUIRY_STOPPED:
          pMac->btc.btcEventState &= ~(BT_INQUIRY_STARTED);
          break;
      case BT_EVENT_PAGE_STOPPED:
          pMac->btc.btcEventState &= ~(BT_PAGE_STARTED);
          break;
      case BT_EVENT_ACL_CONNECTION_COMPLETE:
          pMac->btc.btcEventState &= ~(BT_CREATE_ACL_CONNECTION_STARTED);
          break;
      case BT_EVENT_SYNC_CONNECTION_COMPLETE:
          pMac->btc.btcEventState &= ~(BT_CREATE_SYNC_CONNECTION_STARTED);
          break;

      default:
          // Ignore other events
          return VOS_STATUS_SUCCESS;
   }

   // Check if any of the BT start events are active
   if (pMac->btc.btcEventState) {
       if (pMac->btc.btcHBActive) {
           // set heartbeat threshold CFG to zero
           ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, 0, NULL, eANI_BOOLEAN_FALSE);
           pMac->btc.btcHBActive = VOS_FALSE;
       }

       // Deactivate and active the restore HB timer
       vos_timer_stop( &pMac->btc.restoreHBTimer);
       vosStatus= vos_timer_start( &pMac->btc.restoreHBTimer, BT_MAX_EVENT_DONE_TIMEOUT );
       if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
           VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcCheckHeartBeatMonitoring: Fail to start timer");
           return VOS_STATUS_E_FAILURE;
       }
   } else {
       // Restore CFG back to the original value only if it was disabled
       if (!pMac->btc.btcHBActive) {
           ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pMac->btc.btcHBCount, NULL, eANI_BOOLEAN_FALSE);
           pMac->btc.btcHBActive = VOS_TRUE;
       }
       // Deactivate the timer
       vosStatus = vos_timer_stop( &pMac->btc.restoreHBTimer);
       if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
           VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcCheckHeartBeatMonitoring: Fail to stop timer");
           return VOS_STATUS_E_FAILURE;
       }
   }

   return VOS_STATUS_SUCCESS;

}

/* ---------------------------------------------------------------------------
    \fn btcRestoreHeartBeatMonitoringHandle
    \brief  Timer handler to handlet the timeout condition when a specific BT
            stop event does not come back, in which case to restore back the
            heartbeat timer.
    \param  hHal - The handle returned by macOpen.
    \return VOID
  ---------------------------------------------------------------------------*/
void btcRestoreHeartBeatMonitoringHandle(tHalHandle hHal)
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    // Restore CFG back to the original value
    ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pMac->btc.btcHBCount, NULL, eANI_BOOLEAN_FALSE);
    VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "BT event timeout, restoring back HeartBeat timer");
}


/* ---------------------------------------------------------------------------
    \fn btcSetConfig
    \brief  API to change the current Bluetooth Coexistence (BTC) configuration
            This function should be invoked only after CFG download has completed.
    \param  hHal - The handle returned by macOpen.
    \param  pSmeBtcConfig - Pointer to a caller allocated object of type
                            tSmeBtcConfig. Caller owns the memory and is responsible
                            for freeing it.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – Config not passed to HAL.
            VOS_STATUS_SUCCESS – Config passed to HAL
  ---------------------------------------------------------------------------*/
VOS_STATUS btcSetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if(!pMac->btc.btcReady)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSetConfig: "
         "Trying to set cfg before BTC is ready. Set cfg not accepted ");
      return VOS_STATUS_E_FAILURE;
   }

   //Send message to HAL
   if(VOS_STATUS_SUCCESS != btcSendCfgMsg(hHal, pSmeBtcConfig))
   {
      return VOS_STATUS_E_FAILURE;
   }

   //Save a copy in the global BTC config
   vos_mem_copy(&(pMac->btc.btcConfig), pSmeBtcConfig, sizeof(tSmeBtcConfig));

   return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcPostBtcCfgMsg
    \brief  Private API to post BTC config message to HAL
    \param  hHal - The handle returned by macOpen.
    \param  pSmeBtcConfig - Pointer to a caller allocated object of type
                            tSmeBtcConfig. Caller owns the memory and is responsible
                            for freeing it.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE – Config not passed to HAL.
            VOS_STATUS_SUCCESS – Config passed to HAL
  ---------------------------------------------------------------------------*/
VOS_STATUS btcSendCfgMsg(tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig)
{
   tpSmeBtcConfig ptrSmeBtcConfig = NULL;
   vos_msg_t msg;

   if( NULL == pSmeBtcConfig )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSendCfgMsg: "
         "Null pointer for BTC Config");
      return VOS_STATUS_E_FAILURE;
   }

   if( pSmeBtcConfig->btcExecutionMode >= BT_EXEC_MODE_MAX )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSendCfgMsg: "
         "Invalid BT execution mode %d being set",
          pSmeBtcConfig->btcExecutionMode);
      return VOS_STATUS_E_FAILURE;
   }

   ptrSmeBtcConfig = vos_mem_malloc(sizeof(tSmeBtcConfig));

   if (NULL == ptrSmeBtcConfig)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSendCfgMsg: "
         "Not able to allocate memory for SME BTC Config");
      return VOS_STATUS_E_FAILURE;
   }

   vos_mem_copy(ptrSmeBtcConfig, pSmeBtcConfig, sizeof(tSmeBtcConfig));

   msg.type = SIR_HAL_BTC_SET_CFG;
   msg.reserved = 0;
   msg.bodyptr = ptrSmeBtcConfig;

   if(VOS_STATUS_SUCCESS != vos_mq_post_message(VOS_MODULE_ID_HAL, &msg))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSendCfgMsg: "
         "Not able to post SIR_HAL_BTC_SET_CFG message to HAL");
      vos_mem_free( ptrSmeBtcConfig );
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcGetConfig
    \brief  API to retrieve the current Bluetooth Coexistence (BTC) configuration
    \param  hHal - The handle returned by macOpen.
    \param  pSmeBtcConfig - Pointer to a caller allocated object of type
                            tSmeBtcConfig. Caller owns the memory and is responsible
                            for freeing it.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE - failure
            VOS_STATUS_SUCCESS – success
  ---------------------------------------------------------------------------*/
VOS_STATUS btcGetConfig (tHalHandle hHal, tpSmeBtcConfig pSmeBtcConfig)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

   if( NULL == pSmeBtcConfig )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcGetConfig: "
         "Null pointer for BTC Config");
      return VOS_STATUS_E_FAILURE;
   }

   vos_mem_copy(pSmeBtcConfig, &(pMac->btc.btcConfig), sizeof(tSmeBtcConfig));

   return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcLogEvent
    \brief  API to log the the current Bluetooth event
    \param  hHal - The handle returned by macOpen.
    \param  pSmeBtcConfig - Pointer to a caller allocated object of type
                            tSmeBtEvent. Caller owns the memory and is responsible
                            for freeing it.
    \return None
  ---------------------------------------------------------------------------*/
static void btcLogEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent)
{

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcSignalBTEvent: "
               "Bluetooth Event %d received", pBtEvent->btEventType);

   switch(pBtEvent->btEventType)
   {
      case BT_EVENT_CREATE_SYNC_CONNECTION:
      case BT_EVENT_SYNC_CONNECTION_COMPLETE:
      case BT_EVENT_SYNC_CONNECTION_UPDATED:
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "SCO Connection: "
               "connectionHandle = %d status = %d linkType %d "
               "scoInterval %d scoWindow %d retransmisisonWindow = %d ",
               pBtEvent->uEventParam.btSyncConnection.connectionHandle,
               pBtEvent->uEventParam.btSyncConnection.status,
               pBtEvent->uEventParam.btSyncConnection.linkType,
               pBtEvent->uEventParam.btSyncConnection.scoInterval,
               pBtEvent->uEventParam.btSyncConnection.scoWindow,
               pBtEvent->uEventParam.btSyncConnection.retransmisisonWindow);
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "BD ADDR = "
               "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
               pBtEvent->uEventParam.btSyncConnection.bdAddr[5],
               pBtEvent->uEventParam.btSyncConnection.bdAddr[4],
               pBtEvent->uEventParam.btSyncConnection.bdAddr[3],
               pBtEvent->uEventParam.btSyncConnection.bdAddr[2],
               pBtEvent->uEventParam.btSyncConnection.bdAddr[1],
               pBtEvent->uEventParam.btSyncConnection.bdAddr[0]);

          break;

      case BT_EVENT_CREATE_ACL_CONNECTION:
      case BT_EVENT_ACL_CONNECTION_COMPLETE:
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "ACL Connection: "
               "connectionHandle = %d status = %d ",
               pBtEvent->uEventParam.btAclConnection.connectionHandle,
               pBtEvent->uEventParam.btAclConnection.status);
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "BD ADDR = "
               "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
               pBtEvent->uEventParam.btAclConnection.bdAddr[5],
               pBtEvent->uEventParam.btAclConnection.bdAddr[4],
               pBtEvent->uEventParam.btAclConnection.bdAddr[3],
               pBtEvent->uEventParam.btAclConnection.bdAddr[2],
               pBtEvent->uEventParam.btAclConnection.bdAddr[1],
               pBtEvent->uEventParam.btAclConnection.bdAddr[0]);

          break;

      case BT_EVENT_MODE_CHANGED:
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "ACL Mode change : "
               "connectionHandle %d mode %d ",
               pBtEvent->uEventParam.btAclModeChange.connectionHandle,
               pBtEvent->uEventParam.btAclModeChange.mode);
          break;

      case BT_EVENT_DISCONNECTION_COMPLETE:
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "Disconnect Event : "
               "connectionHandle %d ", pBtEvent->uEventParam.btAclModeChange.connectionHandle);
          break;

      default:
         break;
   }
 }

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/* ---------------------------------------------------------------------------
    \fn btcDiagEventLog
    \brief  API to log the the current Bluetooth event
    \param  hHal - The handle returned by macOpen.
    \param  pSmeBtcConfig - Pointer to a caller allocated object of type
                            tSmeBtEvent. Caller owns the memory and is responsible
                            for freeing it.
    \return None
  ---------------------------------------------------------------------------*/
static void btcDiagEventLog (tHalHandle hHal, tpSmeBtEvent pBtEvent)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   vos_event_wlan_btc_type *log_ptr = NULL;

   WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_event_wlan_btc_type, LOG_WLAN_LL_STAT_C);
   if(log_ptr)
   {
       log_ptr->eventId = pBtEvent->btEventType;
       switch(pBtEvent->btEventType)
       {
            case BT_EVENT_CREATE_SYNC_CONNECTION:
            case BT_EVENT_SYNC_CONNECTION_COMPLETE:
            case BT_EVENT_SYNC_CONNECTION_UPDATED:
                log_ptr->connHandle = pBtEvent->uEventParam.btSyncConnection.connectionHandle;
                log_ptr->connStatus = pBtEvent->uEventParam.btSyncConnection.status;
                log_ptr->linkType   = pBtEvent->uEventParam.btSyncConnection.linkType;
                log_ptr->scoInterval = pBtEvent->uEventParam.btSyncConnection.scoInterval;
                log_ptr->scoWindow  = pBtEvent->uEventParam.btSyncConnection.scoWindow;
                log_ptr->retransWindow = pBtEvent->uEventParam.btSyncConnection.retransmisisonWindow;
                vos_mem_copy(log_ptr->btAddr, pBtEvent->uEventParam.btSyncConnection.bdAddr,
                              sizeof(log_ptr->btAddr));
                break;

            case BT_EVENT_CREATE_ACL_CONNECTION:
            case BT_EVENT_ACL_CONNECTION_COMPLETE:
                log_ptr->connHandle = pBtEvent->uEventParam.btAclConnection.connectionHandle;
                log_ptr->connStatus = pBtEvent->uEventParam.btAclConnection.status;
                vos_mem_copy(log_ptr->btAddr, pBtEvent->uEventParam.btAclConnection.bdAddr,
                             sizeof(log_ptr->btAddr));
                break;

            case BT_EVENT_MODE_CHANGED:
                log_ptr->connHandle = pBtEvent->uEventParam.btAclModeChange.connectionHandle;
                log_ptr->mode = pBtEvent->uEventParam.btAclModeChange.mode;
                break;

            case BT_EVENT_DISCONNECTION_COMPLETE:
                log_ptr->connHandle = pBtEvent->uEventParam.btAclModeChange.connectionHandle;
                break;

            default:
                break;
       }
   }
   WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
