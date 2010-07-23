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
#include "pmc.h"
#include "smeQosInternal.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

static void btcLogEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent);
static void btcRestoreHeartBeatMonitoringHandle(void* hHal);
static void btcUapsdCheck( tpAniSirGlobal pMac, tpSmeBtEvent pBtEvent );
VOS_STATUS btcCheckHeartBeatMonitoring(tHalHandle hHal, tpSmeBtEvent pBtEvent);
static void btcPowerStateCB( v_PVOID_t pContext, tPmcState pmcState );
static VOS_STATUS btcDeferEvent( tpAniSirGlobal pMac, tpSmeBtEvent pEvent );

#ifdef FEATURE_WLAN_DIAG_SUPPORT
static void btcDiagEventLog (tHalHandle hHal, tpSmeBtEvent pBtEvent);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
/* ---------------------------------------------------------------------------
    \fn btcOpen
    \brief  API to init the BTC Events Layer
    \param  hHal - The handle returned by macOpen.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE  success
            VOS_STATUS_SUCCESS  failure
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

   if( !HAL_STATUS_SUCCESS(pmcRegisterDeviceStateUpdateInd( pMac, btcPowerStateCB, pMac )) )
   {
       VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "btcOpen: Fail to register PMC callback\n");
       return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn btcClose
    \brief  API to exit the BTC Events Layer
    \param  hHal - The handle returned by macOpen.
    \return VOS_STATUS
            VOS_STATUS_E_FAILURE  success
            VOS_STATUS_SUCCESS  failure
  ---------------------------------------------------------------------------*/
VOS_STATUS btcClose (tHalHandle hHal)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   VOS_STATUS vosStatus;

   pMac->btc.btcReady = VOS_FALSE;
   pMac->btc.btcUapsdOk = VOS_FALSE;
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
    v_U8_t i;

    pMac->btc.btcReady = VOS_TRUE;
    pMac->btc.btcUapsdOk = VOS_TRUE;

    for(i=0; i < BT_MAX_SCO_SUPPORT; i++)
    {
        pMac->btc.btcScoHandles[i] = BT_INVALID_CONN_HANDLE;
    }


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
            VOS_STATUS_E_FAILURE  BT Event not passed to HAL. This can happen
                                   if driver has not yet been initialized or if BTC
                                   Events Layer has been disabled.
            VOS_STATUS_SUCCESS    BT Event passed to HAL
  ---------------------------------------------------------------------------*/
VOS_STATUS btcSignalBTEvent (tHalHandle hHal, tpSmeBtEvent pBtEvent)
{
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   tpSmeBtEvent ptrSmeBtEvent = NULL;
   vos_msg_t msg;
   VOS_STATUS vosStatus;
   tPmcState pmcState;

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

   //Check PMC state to make sure whether we need to defer
   pmcState = pmcGetPmcState( pMac );
   if( !PMC_IS_CHIP_ACCESSIBLE(pmcState) )
   {
       //We need to defer the event
       vosStatus = btcDeferEvent(pMac, pBtEvent);
       if( VOS_IS_STATUS_SUCCESS(vosStatus) )
       {
           pMac->btc.fReplayBTEvents = VOS_TRUE;
           return VOS_STATUS_SUCCESS;
       }
       else
       {
           return vosStatus;
       }
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

     case BT_EVENT_DEVICE_SWITCHED_OFF:
         //Since fBTSwitchOff is also used to ignore subsequent events in btcDeferEvent, remember.
         pMac->btc.btcEventReplay.fBTSwitchOff = VOS_TRUE;
         pMac->btc.btcEventReplay.fBTSwitchOn = VOS_FALSE;
         pMac->btc.btcEventState = 0;
         break;

     case BT_EVENT_DEVICE_SWITCHED_ON:
         //reset both flags since there is no defer here
         pMac->btc.btcEventReplay.fBTSwitchOn = VOS_FALSE;
         pMac->btc.btcEventReplay.fBTSwitchOff = VOS_FALSE;
         pMac->btc.btcEventState = 0;
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
   (void)btcCheckHeartBeatMonitoring(hHal, pBtEvent);

   //Check whether BTC and UAPSD can co-exist
   btcUapsdCheck( pMac, pBtEvent );

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
            VOS_STATUS_E_FAILURE  Config not passed to HAL.
            VOS_STATUS_SUCCESS  Config passed to HAL
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

    if( !pMac->btc.btcHBActive )
    {
        tPmcState pmcState;

        //Check PMC state to make sure whether we need to defer
        pmcState = pmcGetPmcState( pMac );
        if( PMC_IS_CHIP_ACCESSIBLE(pmcState) )
        {
            // Restore CFG back to the original value
            ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pMac->btc.btcHBCount, NULL, eANI_BOOLEAN_FALSE);
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "BT event timeout, restoring back HeartBeat timer");
        }
        else
        {
            //defer it
            pMac->btc.btcEventReplay.fRestoreHBMonitor = VOS_TRUE;
        }
    }
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
            VOS_STATUS_E_FAILURE  Config not passed to HAL.
            VOS_STATUS_SUCCESS  Config passed to HAL
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
            VOS_STATUS_E_FAILURE  Config not passed to HAL.
            VOS_STATUS_SUCCESS  Config passed to HAL
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
            VOS_STATUS_SUCCESS  success
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

/*
    btcFindAclEventHist find a suited ACL event buffer
    Param: bdAddr - NULL meaning not care.
                    pointer to caller alocated buffer containing the BD address to find a match
           handle - BT_INVALID_CONN_HANDLE == not care
                    otherwise, a handle to match
    NOPTE: Either bdAddr or handle can be valid, if both of them are valid, use bdAddr only. If neither 
           bdAddr nor handle is valid, return the next free slot.
*/
static tpSmeBtAclEventHist btcFindAclEventHist( tpAniSirGlobal pMac, v_U8_t *bdAddr, v_U16_t handle )
{
    int i;
    tpSmeBtAclEventHist pRet = NULL;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;

    for( i = 0; i < BT_MAX_ACL_SUPPORT; i++ )
    {
        if( NULL != bdAddr )
        {
            //try to match addr
            if( pReplay->btcEventHist.btAclConnectionEvent[i].fValid )
            {
                if( vos_mem_compare(pReplay->btcEventHist.btAclConnectionEvent[i].btAclConnection.bdAddr,
                    bdAddr, 6) )
                {
                    //found it
                    pRet = &pReplay->btcEventHist.btAclConnectionEvent[i];
                    break;
                }
            }
        }
        else if( BT_INVALID_CONN_HANDLE != handle )
        {
            //try to match handle
            if( pReplay->btcEventHist.btAclConnectionEvent[i].fValid )
            {
                if( pReplay->btcEventHist.btAclConnectionEvent[i].btAclConnection.connectionHandle ==
                    handle )
                {
                    //found it
                    pRet = &pReplay->btcEventHist.btAclConnectionEvent[i];
                    break;
                }
            }
        }
        else if( !pReplay->btcEventHist.btAclConnectionEvent[i].fValid )
        {
            pRet = &pReplay->btcEventHist.btAclConnectionEvent[i];
            break;
        }
    }

    return (pRet);
}


/*
    btcFindSyncEventHist find a suited SYNC event buffer
    Param: bdAddr - NULL meaning not care.
                    pointer to caller alocated buffer containing the BD address to find a match
           handle - BT_INVALID_CONN_HANDLE == not care
                    otherwise, a handle to match
    NOPTE: Either bdAddr or handle can be valid, if both of them are valid, use bdAddr only. If neither 
           bdAddr nor handle is valid, return the next free slot.
*/
static tpSmeBtSyncEventHist btcFindSyncEventHist( tpAniSirGlobal pMac, v_U8_t *bdAddr, v_U16_t handle )
{
    int i;
    tpSmeBtSyncEventHist pRet = NULL;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;

    for( i = 0; i < BT_MAX_SCO_SUPPORT; i++ )
    {
        if( NULL != bdAddr )
        {
            //try to match addr
            if( pReplay->btcEventHist.btSyncConnectionEvent[i].fValid )
            {
                if( vos_mem_compare(pReplay->btcEventHist.btSyncConnectionEvent[i].btSyncConnection.bdAddr,
                    bdAddr, 6) )
                {
                    //found it
                    pRet = &pReplay->btcEventHist.btSyncConnectionEvent[i];
                    break;
                }
            }
        }
        else if( BT_INVALID_CONN_HANDLE != handle )
        {
            //try to match handle
            if( pReplay->btcEventHist.btSyncConnectionEvent[i].fValid )
            {
                if( pReplay->btcEventHist.btSyncConnectionEvent[i].btSyncConnection.connectionHandle ==
                    handle )
                {
                    //found it
                    pRet = &pReplay->btcEventHist.btSyncConnectionEvent[i];
                    break;
                }
            }
        }
        else if( !pReplay->btcEventHist.btSyncConnectionEvent[i].fValid )
        {
            pRet = &pReplay->btcEventHist.btSyncConnectionEvent[i];
            break;
        }
    }

    return (pRet);
}


/*
    btcFindDisconnEventHist find a slot for the deferred disconnect event
    If handle is invlid, it returns a free slot, if any. 
    If handle is valid, it tries to find a match first in case same disconnect event comes down again.
*/
static tpSmeBtDisconnectEventHist btcFindDisconnEventHist( tpAniSirGlobal pMac, v_U16_t handle )
{
    tpSmeBtDisconnectEventHist pRet = NULL;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;
    int i;

    if( BT_INVALID_CONN_HANDLE != handle )
    {
        for(i = 0; i < BT_MAX_DISCONN_SUPPORT; i++)
        {
            if( pReplay->btcEventHist.btDisconnectEvent[i].fValid &&
                (handle == pReplay->btcEventHist.btDisconnectEvent[i].btDisconnect.connectionHandle) )
            {
                pRet = &pReplay->btcEventHist.btDisconnectEvent[i];
                break;
            }
        }
    }
    if( NULL == pRet )
    {
        //Find a free slot
        for(i = 0; i < BT_MAX_DISCONN_SUPPORT; i++)
        {
            if( !pReplay->btcEventHist.btDisconnectEvent[i].fValid )
            {
                pRet = &pReplay->btcEventHist.btDisconnectEvent[i];
                break;
            }
        }
    }

    return (pRet);
}


/*
    btcFindModeChangeEventHist find a slot for the deferred disconnect event
    If handle is invlid, it returns a free slot, if any. 
    If handle is valid, it tries to find a match first in case same disconnect event comes down again.
*/
tpSmeBtAclModeChangeEventHist btcFindModeChangeEventHist( tpAniSirGlobal pMac, v_U16_t handle )
{
    tpSmeBtAclModeChangeEventHist pRet = NULL;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;
    int i;

    if( BT_INVALID_CONN_HANDLE != handle )
    {
        for(i = 0; i < BT_MAX_ACL_SUPPORT; i++)
        {
            if( pReplay->btcEventHist.btAclModeChangeEvent[i].fValid &&
                (handle == pReplay->btcEventHist.btAclModeChangeEvent[i].btAclModeChange.connectionHandle) )
            {
                pRet = &pReplay->btcEventHist.btAclModeChangeEvent[i];
                break;
            }
        }
    }
    if( NULL == pRet )
    {
        //Find a free slot
        for(i = 0; i < BT_MAX_ACL_SUPPORT; i++)
        {
            if( !pReplay->btcEventHist.btAclModeChangeEvent[i].fValid )
            {
                pRet = &pReplay->btcEventHist.btAclModeChangeEvent[i];
                break;
            }
        }
    }

    return (pRet);
}



/*
    Call must validate pAclEventHist
*/
static void btcReleaseAclEventHist( tpAniSirGlobal pMac, tpSmeBtAclEventHist pAclEventHist )
{
    vos_mem_zero( pAclEventHist, sizeof(tSmeBtAclEventHist) );
}


/*
    Call must validate pSyncEventHist
*/
static void btcReleaseSyncEventHist( tpAniSirGlobal pMac, tpSmeBtSyncEventHist pSyncEventHist )
{
    vos_mem_zero( pSyncEventHist, sizeof(tSmeBtSyncEventHist) );
}


static VOS_STATUS btcDeferAclComplete( tpAniSirGlobal pMac, tpSmeBtEvent pEvent )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpSmeBtAclEventHist pAclEventHist;

    //Find a match
    pAclEventHist = btcFindAclEventHist( pMac, pEvent->uEventParam.btAclConnection.bdAddr, 
                                BT_INVALID_CONN_HANDLE );
    if( NULL == pAclEventHist )
    {
        pAclEventHist = btcFindAclEventHist( pMac, NULL, BT_INVALID_CONN_HANDLE );
    }
    if(pAclEventHist)
    {
        if( (BT_CONN_STATUS_FAIL != pEvent->uEventParam.btAclConnection.status) ||
            //Also take care the case where we don't buffer the create event
            (BT_EVENT_CREATE_ACL_CONNECTION != pAclEventHist->btEventType[0]) )
        {
            pAclEventHist->fValid = VOS_TRUE;
            if( BT_EVENT_CREATE_ACL_CONNECTION == pAclEventHist->btEventType[0] )
            {
                pAclEventHist->btEventType[1] = BT_EVENT_ACL_CONNECTION_COMPLETE;
            }
            else
            {
                pAclEventHist->btEventType[0] = BT_EVENT_ACL_CONNECTION_COMPLETE;
            }
            vos_mem_copy(&pAclEventHist->btAclConnection, &pEvent->uEventParam.btAclConnection, 
                            sizeof(tSmeBtAclConnectionParam));
        }
        else
        {
            smsLog( pMac, LOGE, FL(" ACL connection failed\n") );
            //Connection fail
            btcReleaseAclEventHist( pMac, pAclEventHist );
        }
    }
    else
    {
        smsLog( pMac, LOGE, FL(" cannot find match for failed BT_EVENT_ACL_CONNECTION_COMPLETE of bdAddr (%02X-%02X-%02X-%02X-%02X-%02X)\n"),
            pEvent->uEventParam.btAclConnection.bdAddr[0],
            pEvent->uEventParam.btAclConnection.bdAddr[1],
            pEvent->uEventParam.btAclConnection.bdAddr[2],
            pEvent->uEventParam.btAclConnection.bdAddr[3],
            pEvent->uEventParam.btAclConnection.bdAddr[4],
            pEvent->uEventParam.btAclConnection.bdAddr[5]);
        status = VOS_STATUS_E_EMPTY;
    }

    return (status);
}


static VOS_STATUS btcDeferSyncComplete( tpAniSirGlobal pMac, tpSmeBtEvent pEvent )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpSmeBtSyncEventHist pSyncEventHist;

    //Find a match
    pSyncEventHist = btcFindSyncEventHist( pMac, pEvent->uEventParam.btSyncConnection.bdAddr, 
                                BT_INVALID_CONN_HANDLE );
    if(NULL == pSyncEventHist)
    {
        //In case we don't defer the creation event
        pSyncEventHist = btcFindSyncEventHist( pMac, NULL, BT_INVALID_CONN_HANDLE );
    }
    if(pSyncEventHist)
    {
        if( (BT_CONN_STATUS_FAIL != pEvent->uEventParam.btSyncConnection.status) ||
            (BT_EVENT_CREATE_SYNC_CONNECTION == pSyncEventHist->btEventType[0]) )
        {
            pSyncEventHist->fValid = VOS_TRUE;
            if( BT_EVENT_CREATE_SYNC_CONNECTION == pSyncEventHist->btEventType[0] )
            {
                pSyncEventHist->btEventType[1] = BT_EVENT_SYNC_CONNECTION_COMPLETE;
            }
            else
            {
                pSyncEventHist->btEventType[0] = BT_EVENT_SYNC_CONNECTION_COMPLETE;
            }
            vos_mem_copy(&pSyncEventHist->btSyncConnection, &pEvent->uEventParam.btSyncConnection, 
                            sizeof(tSmeBtSyncConnectionParam));
        }
        else
        {
            smsLog( pMac, LOGE, FL(" SYNC connection failed\n") );
            //Connection fail
            btcReleaseSyncEventHist( pMac, pSyncEventHist );
        }
    }
    else
    {
        smsLog( pMac, LOGE, FL(" cannot find match for BT_EVENT_SYNC_CONNECTION_COMPLETE of bdAddr (%02X-%02X-%02X-%02X-%02X-%02X)\n"),
            pEvent->uEventParam.btSyncConnection.bdAddr[0],
            pEvent->uEventParam.btSyncConnection.bdAddr[1],
            pEvent->uEventParam.btSyncConnection.bdAddr[2],
            pEvent->uEventParam.btSyncConnection.bdAddr[3],
            pEvent->uEventParam.btSyncConnection.bdAddr[4],
            pEvent->uEventParam.btSyncConnection.bdAddr[5]);
        status = VOS_STATUS_E_EMPTY;
    }

    return (status);
}


static VOS_STATUS btcDeferDisconnEvent( tpAniSirGlobal pMac, tpSmeBtEvent pEvent )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpSmeBtDisconnectEventHist pDisconnEventHist;
    tpSmeBtAclEventHist pAclEventHist;
    v_BOOL_t fSaveDisconnEvent = VOS_TRUE;
    tpSmeBtSyncEventHist pSyncEventHist;
    tpSmeBtAclModeChangeEventHist pModeChangeEventHist;

    if( BT_INVALID_CONN_HANDLE == pEvent->uEventParam.btDisconnect.connectionHandle )
    {
        smsLog( pMac, LOGE, FL(" invalid handle\n") );
        return (VOS_STATUS_E_INVAL);
    }

    //Check ACL first
    pAclEventHist = btcFindAclEventHist( pMac, NULL, 
                                pEvent->uEventParam.btDisconnect.connectionHandle );
    if(pAclEventHist)
    {
        //make sure we can cancel the link
        if( BT_EVENT_CREATE_ACL_CONNECTION == pAclEventHist->btEventType[0] )
        {
            //we also holding the create event, we can cancel the link
            btcReleaseAclEventHist( pMac, pAclEventHist );
            //Wipe out the related mode change event if it is there
            pModeChangeEventHist = btcFindModeChangeEventHist( pMac,  
                                    pEvent->uEventParam.btDisconnect.connectionHandle );
            if( pModeChangeEventHist && pModeChangeEventHist->fValid )
            {
                pModeChangeEventHist->fValid = VOS_FALSE;
            }
            fSaveDisconnEvent = VOS_FALSE;
        }
    }
    else
    {
        //look for sync
        pSyncEventHist = btcFindSyncEventHist( pMac, NULL, 
                                pEvent->uEventParam.btDisconnect.connectionHandle );
        if(pSyncEventHist)
        {
            //make sure we can cancel the link
            if( BT_EVENT_CREATE_SYNC_CONNECTION == pSyncEventHist->btEventType[0] )
            {
                //we also holding the create event, we can cancel the link
                btcReleaseSyncEventHist( pMac, pSyncEventHist );
                fSaveDisconnEvent = VOS_FALSE;
            }
        }
    }
    if( fSaveDisconnEvent )
    {
        //Save in the disconnect event
        pDisconnEventHist = btcFindDisconnEventHist( pMac, 
            pEvent->uEventParam.btDisconnect.connectionHandle );
        if( pDisconnEventHist )
        {
            pDisconnEventHist->fValid = VOS_TRUE;
            vos_mem_copy( &pDisconnEventHist->btDisconnect, &pEvent->uEventParam.btDisconnect,
                sizeof(tSmeBtDisconnectParam) );
        }
        else
        {
            smsLog( pMac, LOGE, FL(" cannot find match for BT_EVENT_DISCONNECTION_COMPLETE of handle (%d)\n"),
                pEvent->uEventParam.btDisconnect.connectionHandle);
            status = VOS_STATUS_E_EMPTY;
        }
    }

    return (status);
}


/*
    btcDeferEvent save the event for possible replay when chip can be accessed
    This function is called only when in IMPS/Standby state
*/
static VOS_STATUS btcDeferEvent( tpAniSirGlobal pMac, tpSmeBtEvent pEvent )
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tpSmeBtAclEventHist pAclEventHist;
    tpSmeBtSyncEventHist pSyncEventHist;
    tpSmeBtAclModeChangeEventHist pModeChangeEventHist;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;

    //swicth on/off are very special, handle them first
    if( (BT_EVENT_DEVICE_SWITCHED_OFF == pEvent->btEventType) ||
        (BT_EVENT_DEVICE_SWITCHED_ON == pEvent->btEventType))
    {
        //Clear all events first
        vos_mem_zero( &pReplay->btcEventHist, sizeof(tSmeBtcEventHist) );
        //Only remember swicth on/off
        pReplay->fBTSwitchOff = (BT_EVENT_DEVICE_SWITCHED_OFF == pEvent->btEventType);
        pReplay->fBTSwitchOn = !pReplay->fBTSwitchOff;

        return (VOS_STATUS_SUCCESS);
    }

    if( pReplay->fBTSwitchOff )
    {
        //No need to save any event
        smsLog( pMac, LOGE, FL(" deferring when BT_SWITCH_OFF, ignore event(%d)\n"), pEvent->btEventType );
        //Return a fail case so fReplayBTEvents is not turned on
        return (VOS_STATUS_E_BADMSG);
    }

    switch(pEvent->btEventType)
    {
    case BT_EVENT_DEVICE_SWITCHED_ON:
        pReplay->fBTSwitchOff = VOS_FALSE;
        pReplay->fBTSwitchOn = VOS_TRUE;
        break;

    case BT_EVENT_DEVICE_SWITCHED_OFF:
        //Clear all events first
        vos_mem_zero( &pReplay->btcEventHist, sizeof(tSmeBtcEventHist) );
        //Only remember swicth off
        pReplay->fBTSwitchOff = VOS_TRUE;
        pReplay->fBTSwitchOn = VOS_FALSE;
        break;

    case BT_EVENT_INQUIRY_STARTED:
        pReplay->btcEventHist.fInquiryStarted = VOS_TRUE;
        pReplay->btcEventHist.fInquiryStopped = VOS_FALSE;
        break;

    case BT_EVENT_INQUIRY_STOPPED:
        pReplay->btcEventHist.fInquiryStopped = VOS_TRUE;
        pReplay->btcEventHist.fInquiryStarted = VOS_FALSE;
        break;

    case BT_EVENT_PAGE_STARTED:
        pReplay->btcEventHist.fPageStarted = VOS_TRUE;
        pReplay->btcEventHist.fPageStopped = VOS_FALSE;
        break;

    case BT_EVENT_PAGE_STOPPED:
        pReplay->btcEventHist.fPageStopped = VOS_TRUE;
        pReplay->btcEventHist.fPageStarted = VOS_FALSE;
        break;

    case BT_EVENT_CREATE_ACL_CONNECTION:
        //Find a free slot
        pAclEventHist = btcFindAclEventHist( pMac, NULL, BT_INVALID_CONN_HANDLE );
        if(pAclEventHist)
        {
            pAclEventHist->fValid = VOS_TRUE;
            pAclEventHist->btEventType[0] = BT_EVENT_CREATE_ACL_CONNECTION;
            vos_mem_copy(&pAclEventHist->btAclConnection, &pEvent->uEventParam.btAclConnection, 
                            sizeof(tSmeBtAclConnectionParam));
        }
        else
        {
            smsLog( pMac, LOGE, FL(" cannot free slot for BT_EVENT_CREATE_ACL_CONNECTION of bdAddr (%02X-%02X-%02X-%02X-%02X-%02X)\n"),
                pEvent->uEventParam.btAclConnection.bdAddr[0],
                pEvent->uEventParam.btAclConnection.bdAddr[1],
                pEvent->uEventParam.btAclConnection.bdAddr[2],
                pEvent->uEventParam.btAclConnection.bdAddr[3],
                pEvent->uEventParam.btAclConnection.bdAddr[4],
                pEvent->uEventParam.btAclConnection.bdAddr[5]);
            status = VOS_STATUS_E_RESOURCES;
        }
        break;

    case BT_EVENT_ACL_CONNECTION_COMPLETE:
        status = btcDeferAclComplete( pMac, pEvent );
        break;

    case BT_EVENT_CREATE_SYNC_CONNECTION:
        //Find a free slot
        pSyncEventHist = btcFindSyncEventHist( pMac, NULL, BT_INVALID_CONN_HANDLE );
        if(pSyncEventHist)
        {
            pSyncEventHist->fValid = VOS_TRUE;
            pSyncEventHist->btEventType[0] = BT_EVENT_CREATE_SYNC_CONNECTION;
            vos_mem_copy(&pSyncEventHist->btSyncConnection, &pEvent->uEventParam.btSyncConnection, 
                            sizeof(tSmeBtSyncConnectionParam));
        }
        else
        {
            smsLog( pMac, LOGE, FL(" cannot find match for BT_EVENT_CREATE_SYNC_CONNECTION of bdAddr (%02X-%02X-%02X-%02X-%02X-%02X)\n"),
                pEvent->uEventParam.btSyncConnection.bdAddr[0],
                pEvent->uEventParam.btSyncConnection.bdAddr[1],
                pEvent->uEventParam.btSyncConnection.bdAddr[2],
                pEvent->uEventParam.btSyncConnection.bdAddr[3],
                pEvent->uEventParam.btSyncConnection.bdAddr[4],
                pEvent->uEventParam.btSyncConnection.bdAddr[5]);
            status = VOS_STATUS_E_RESOURCES;
        }
        break;

    case BT_EVENT_SYNC_CONNECTION_COMPLETE:
        status = btcDeferSyncComplete( pMac, pEvent );
        break;

    case BT_EVENT_SYNC_CONNECTION_UPDATED:
        if( BT_INVALID_CONN_HANDLE == pEvent->uEventParam.btDisconnect.connectionHandle )
        {
            smsLog( pMac, LOGE, FL(" invalid handle\n") );
            status = VOS_STATUS_E_INVAL;
            break;
        }
        //Find a match on handle
        pSyncEventHist = btcFindSyncEventHist( pMac, NULL, 
                                    pEvent->uEventParam.btSyncConnection.connectionHandle );
        if(NULL == pSyncEventHist)
        {
            //In case we don't defer the creation event
            pSyncEventHist = btcFindSyncEventHist( pMac, NULL, BT_INVALID_CONN_HANDLE );
        }
        if(pSyncEventHist)
        {
            pSyncEventHist->fValid = VOS_TRUE;
            //No need to change the event
            vos_mem_copy(&pSyncEventHist->btSyncConnection, &pEvent->uEventParam.btSyncConnection, 
                            sizeof(tSmeBtSyncConnectionParam));
        }
        else
        {
            smsLog( pMac, LOGE, FL(" cannot find match for BT_EVENT_SYNC_CONNECTION_UPDATED of handle (%d)\n"),
                pEvent->uEventParam.btSyncConnection.connectionHandle );
            status = VOS_STATUS_E_EMPTY;
        }
        break;

    case BT_EVENT_DISCONNECTION_COMPLETE:
        status = btcDeferDisconnEvent( pMac, pEvent );
        break;

    case BT_EVENT_MODE_CHANGED:
        if( BT_INVALID_CONN_HANDLE == pEvent->uEventParam.btDisconnect.connectionHandle )
        {
            smsLog( pMac, LOGE, FL(" invalid handle\n") );
            status = VOS_STATUS_E_INVAL;
            break;
        }
        //Find a match on handle
        pModeChangeEventHist = btcFindModeChangeEventHist( pMac,  
                                    pEvent->uEventParam.btAclModeChange.connectionHandle );
        if(pModeChangeEventHist)
        {
            pModeChangeEventHist->fValid = VOS_TRUE;
            vos_mem_copy( &pModeChangeEventHist->btAclModeChange,
                            &pEvent->uEventParam.btAclModeChange, sizeof(tSmeBtAclModeChangeParam) );
        }
        else
        {
            smsLog( pMac, LOGE, FL(" cannot find match for BT_EVENT_MODE_CHANGED of handle (%d)\n"),
                pEvent->uEventParam.btAclModeChange.connectionHandle);
            status = VOS_STATUS_E_EMPTY;
        }
        break;

    case BT_EVENT_A2DP_STREAM_START:
        pReplay->btcEventHist.fA2DPStarted = VOS_TRUE;
        pReplay->btcEventHist.fA2DPStopped = VOS_FALSE;
        break;

    case BT_EVENT_A2DP_STREAM_STOP:
        pReplay->btcEventHist.fA2DPStopped = VOS_TRUE;
        pReplay->btcEventHist.fA2DPStarted = VOS_FALSE;
        break;

    default:
        smsLog( pMac, LOGE, FL(" event (%d) is not deferred\n"), pEvent->btEventType );
        status = VOS_STATUS_E_NOSUPPORT;
        break;
    }

    return (status);
}


static void btcReplayEvents( tpAniSirGlobal pMac )
{
    int i;
    tSmeBtEvent btEvent;
    tpSmeBtAclEventHist pAclHist;
    tpSmeBtSyncEventHist pSyncHist;
    tSmeBtcEventReplay *pReplay = &pMac->btc.btcEventReplay;

    //Always turn on HB monitor first. 
    //It is independent of BT events even though BT event causes this
    if( pReplay->fRestoreHBMonitor )
    {
        pReplay->fRestoreHBMonitor = VOS_FALSE;
        //Only do it when needed
        if( !pMac->btc.btcHBActive ) 
        {
            ccmCfgSetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, pMac->btc.btcHBCount, NULL, eANI_BOOLEAN_FALSE);
            pMac->btc.btcHBActive = VOS_TRUE;
        }
    }
    if( pMac->btc.fReplayBTEvents )
    {
        //Is BT still on
        if( pReplay->fBTSwitchOff )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_DEVICE_SWITCHED_OFF;
            btcSignalBTEvent( pMac, &btEvent );
            //We are done
            //Clear all events
            vos_mem_zero( &pReplay->btcEventHist, sizeof(tSmeBtcEventHist) );
            //keep the switch off flag to ignore the rest of the events until on
            pMac->btc.fReplayBTEvents = VOS_FALSE;
            return;
        }

        if( pReplay->fBTSwitchOn )
        {
            pReplay->fBTSwitchOff = VOS_FALSE;
            pReplay->fBTSwitchOn = VOS_FALSE;
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_DEVICE_SWITCHED_ON;
            btcSignalBTEvent( pMac, &btEvent );
        }

        //Do inquire first
        if( pReplay->btcEventHist.fInquiryStarted )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_INQUIRY_STARTED;
            btcSignalBTEvent( pMac, &btEvent );
        }
        else if( pReplay->btcEventHist.fInquiryStopped )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_INQUIRY_STOPPED;
            btcSignalBTEvent( pMac, &btEvent );
        }
        //Page
        if( pReplay->btcEventHist.fPageStarted )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_PAGE_STARTED;
            btcSignalBTEvent( pMac, &btEvent );
        }
        else if( pReplay->btcEventHist.fPageStopped )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_PAGE_STOPPED;
            btcSignalBTEvent( pMac, &btEvent );
        }
        //ACL
        for( i = 0; i < BT_MAX_ACL_SUPPORT; i++ )
        {
            if( pReplay->btcEventHist.btAclConnectionEvent[i].fValid )
            {
                pAclHist = &pReplay->btcEventHist.btAclConnectionEvent[i];
                vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
                if( BT_EVENT_ACL_CONNECTION_COMPLETE == pAclHist->btEventType[1] )
                {
                    btEvent.btEventType = pAclHist->btEventType[1];
                }
                else
                {
                    btEvent.btEventType = pAclHist->btEventType[0];
                }
                vos_mem_copy( &btEvent.uEventParam.btAclConnection, 
                        &pAclHist->btAclConnection, sizeof(tSmeBtAclConnectionParam) );
                btcSignalBTEvent( pMac, &btEvent );
            }
        }
        //Mode change
        for( i = 0; i < BT_MAX_ACL_SUPPORT; i++ )
        {
            if( pReplay->btcEventHist.btAclModeChangeEvent[i].fValid )
            {
                vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
                btEvent.btEventType = BT_EVENT_MODE_CHANGED;
                vos_mem_copy( &btEvent.uEventParam.btAclModeChange, 
                    &pReplay->btcEventHist.btAclModeChangeEvent[i].btAclModeChange, sizeof(tSmeBtAclModeChangeParam) );
                btcSignalBTEvent( pMac, &btEvent );
            }
        }
       //A2DP
        if( pReplay->btcEventHist.fA2DPStarted )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_A2DP_STREAM_START;
            btcSignalBTEvent( pMac, &btEvent );
        }
        else if( pReplay->btcEventHist.fA2DPStopped )
        {
            vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
            btEvent.btEventType = BT_EVENT_A2DP_STREAM_STOP;
            btcSignalBTEvent( pMac, &btEvent );
        }
        //SCO
        for( i = 0; i < BT_MAX_SCO_SUPPORT; i++ )
        {
            if( pReplay->btcEventHist.btSyncConnectionEvent[i].fValid )
            {
                pSyncHist = &pReplay->btcEventHist.btSyncConnectionEvent[i];
                vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
                if( BT_EVENT_SYNC_CONNECTION_COMPLETE == pSyncHist->btEventType[1] )
                {
                    btEvent.btEventType = pSyncHist->btEventType[1];
                }
                else
                {
                    btEvent.btEventType = pSyncHist->btEventType[0];
                }
                vos_mem_copy( &btEvent.uEventParam.btSyncConnection, 
                        &pSyncHist->btSyncConnection, sizeof(tSmeBtSyncConnectionParam) );
                btcSignalBTEvent( pMac, &btEvent );
            }
        }
        //Disconnect
        for( i = 0; i < BT_MAX_DISCONN_SUPPORT; i++ )
        {
            if( pReplay->btcEventHist.btDisconnectEvent[i].fValid )
            {
                vos_mem_zero( &btEvent, sizeof(tSmeBtEvent) );
                btEvent.btEventType = BT_EVENT_DISCONNECTION_COMPLETE;
                vos_mem_copy( &btEvent.uEventParam.btDisconnect, 
                    &pReplay->btcEventHist.btDisconnectEvent[i].btDisconnect, sizeof(tSmeBtDisconnectParam) );
                btcSignalBTEvent( pMac, &btEvent );
            }
        }

        //Clear all events
        vos_mem_zero( &pReplay->btcEventHist, sizeof(tSmeBtcEventHist) );
        pMac->btc.fReplayBTEvents = VOS_FALSE;
    }
}



static void btcPowerStateCB( v_PVOID_t pContext, tPmcState pmcState )
{
    tpAniSirGlobal pMac = PMAC_STRUCT(pContext);

    if( FULL_POWER == pmcState )
    {
        btcReplayEvents( pMac );
    }
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


/*
   Caller can check whether BTC's current event allows UAPSD. This doesn't affect
   BMPS.
   return:  VOS_TRUE -- BTC is ready for UAPSD
            VOS_FALSE -- certain BT event is active, cannot enter UAPSD
*/
v_BOOL_t btcIsReadyForUapsd( tHalHandle hHal )
{
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );

    return( pMac->btc.btcUapsdOk );
}



/*
    Base on the BT event, this function sets the flag on whether to allow UAPSD
    At this time, we are only interested in SCO and A2DP.
    A2DP tracking is through BT_EVENT_A2DP_STREAM_START and BT_EVENT_A2DP_STREAM_STOP
    SCO is through BT_EVENT_SYNC_CONNECTION_COMPLETE and BT_EVENT_DISCONNECTION_COMPLETE
    BT_EVENT_DEVICE_SWITCHED_OFF overwrites them all
*/
void btcUapsdCheck( tpAniSirGlobal pMac, tpSmeBtEvent pBtEvent )
{
   v_U8_t i;
   v_BOOL_t fLastUapsdState = pMac->btc.btcUapsdOk, fMoreSCO = VOS_FALSE;

   switch( pBtEvent->btEventType )
   {
   case BT_EVENT_DISCONNECTION_COMPLETE:
       if( (VOS_FALSE == pMac->btc.btcUapsdOk) && 
           BT_INVALID_CONN_HANDLE != pBtEvent->uEventParam.btDisconnect.connectionHandle )
       {
           //Check whether all SCO connections are gone
           for(i=0; i < BT_MAX_SCO_SUPPORT; i++)
           {
               if( (BT_INVALID_CONN_HANDLE != pMac->btc.btcScoHandles[i]) &&
                   (pMac->btc.btcScoHandles[i] != pBtEvent->uEventParam.btDisconnect.connectionHandle) )
               {
                   //We still have outstanding SCO connection
                   fMoreSCO = VOS_TRUE;
               }
               else if( pMac->btc.btcScoHandles[i] == pBtEvent->uEventParam.btDisconnect.connectionHandle )
               {
                   pMac->btc.btcScoHandles[i] = BT_INVALID_CONN_HANDLE;
               }
           }
           if( !fMoreSCO && !pMac->btc.fA2DPUp )
           {
               //All SCO is disconnected
               pMac->btc.btcUapsdOk = VOS_TRUE;
               smsLog( pMac, LOGE, "BT event (DISCONNECTION) happens, UAPSD-allowed flag (%d) change to TRUE \n", 
                        pBtEvent->btEventType, pMac->btc.btcUapsdOk );
           }
       }
       break;

   case BT_EVENT_DEVICE_SWITCHED_OFF:
       smsLog( pMac, LOGE, "BT event (DEVICE_OFF) happens, UAPSD-allowed flag (%d) change to TRUE \n", 
                        pBtEvent->btEventType, pMac->btc.btcUapsdOk );
	   //Clean up SCO
	   for(i=0; i < BT_MAX_SCO_SUPPORT; i++)
       {
           pMac->btc.btcScoHandles[i] = BT_INVALID_CONN_HANDLE;
       }
	   pMac->btc.fA2DPUp = VOS_FALSE;
       pMac->btc.btcUapsdOk = VOS_TRUE;
	   break;

   case BT_EVENT_A2DP_STREAM_STOP:
       smsLog( pMac, LOGE, "BT event (A2DP_STREAM_STOP) happens, UAPSD-allowed flag (%d) change to TRUE \n", 
            pBtEvent->btEventType, pMac->btc.btcUapsdOk );
	   pMac->btc.fA2DPUp = VOS_FALSE;
	   //Check whether SCO is on
	   for(i=0; i < BT_MAX_SCO_SUPPORT; i++)
       {
           if(pMac->btc.btcScoHandles[i] != BT_INVALID_CONN_HANDLE)
		   {
			   break;
		   }
       }
	   if( BT_MAX_SCO_SUPPORT == i )
	   {
		   pMac->btc.btcUapsdOk = VOS_TRUE;
	   }
       break;

   case BT_EVENT_SYNC_CONNECTION_COMPLETE:
	   smsLog( pMac, LOGE, "BT_EVENT_SYNC_CONNECTION_COMPLETE (%d) happens, UAPSD-allowed flag (%d) change to FALSE \n", 
                pBtEvent->btEventType, pMac->btc.btcUapsdOk );
       //Make sure it is a success
       if( BT_CONN_STATUS_FAIL != pBtEvent->uEventParam.btSyncConnection.status )
       {
           //Save te handle for later use
           for( i = 0; i < BT_MAX_SCO_SUPPORT; i++)
           {
               VOS_ASSERT(BT_INVALID_CONN_HANDLE != pBtEvent->uEventParam.btSyncConnection.connectionHandle);
               if( (BT_INVALID_CONN_HANDLE == pMac->btc.btcScoHandles[i]) &&
                   (BT_INVALID_CONN_HANDLE != pBtEvent->uEventParam.btSyncConnection.connectionHandle))
               {
                   pMac->btc.btcScoHandles[i] = pBtEvent->uEventParam.btSyncConnection.connectionHandle;
			       break;
               }
           }
	       if( i < BT_MAX_SCO_SUPPORT )
	       {
		       pMac->btc.btcUapsdOk = VOS_FALSE;
	       }
	       else
	       {
		       smsLog(pMac, LOGE, FL("Too many SCO, ignore this one\n"));
	       }
       }
       else
       {
           smsLog(pMac, LOGE, FL("TSYNC complete failed\n"));
       }
       break;

   case BT_EVENT_A2DP_STREAM_START:
       smsLog( pMac, LOGE, "BT_EVENT_A2DP_STREAM_START (%d) happens, UAPSD-allowed flag (%d) change to FALSE \n", 
                pBtEvent->btEventType, pMac->btc.btcUapsdOk );
       pMac->btc.btcUapsdOk = VOS_FALSE;
	   pMac->btc.fA2DPUp = VOS_TRUE;
       break;

   default:
       //No change for these events
       smsLog( pMac, LOGE, "BT event (%d) happens, UAPSD-allowed flag (%d) no change \n", 
                    pBtEvent->btEventType, pMac->btc.btcUapsdOk );
       break;
   }

   if(fLastUapsdState != pMac->btc.btcUapsdOk)
   {
      sme_QosTriggerUapsdChange( pMac );
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

   WLAN_VOS_DIAG_EVENT_DEF(btDiagEvent, vos_event_wlan_btc_type);
   {
       btDiagEvent.eventId = pBtEvent->btEventType;
       switch(pBtEvent->btEventType)
       {
            case BT_EVENT_CREATE_SYNC_CONNECTION:
            case BT_EVENT_SYNC_CONNECTION_COMPLETE:
            case BT_EVENT_SYNC_CONNECTION_UPDATED:
                btDiagEvent.connHandle = pBtEvent->uEventParam.btSyncConnection.connectionHandle;
                btDiagEvent.connStatus = pBtEvent->uEventParam.btSyncConnection.status;
                btDiagEvent.linkType   = pBtEvent->uEventParam.btSyncConnection.linkType;
                btDiagEvent.scoInterval = pBtEvent->uEventParam.btSyncConnection.scoInterval;
                btDiagEvent.scoWindow  = pBtEvent->uEventParam.btSyncConnection.scoWindow;
                btDiagEvent.retransWindow = pBtEvent->uEventParam.btSyncConnection.retransmisisonWindow;
                vos_mem_copy(btDiagEvent.btAddr, pBtEvent->uEventParam.btSyncConnection.bdAddr,
                              sizeof(btDiagEvent.btAddr));
                break;

            case BT_EVENT_CREATE_ACL_CONNECTION:
            case BT_EVENT_ACL_CONNECTION_COMPLETE:
                btDiagEvent.connHandle = pBtEvent->uEventParam.btAclConnection.connectionHandle;
                btDiagEvent.connStatus = pBtEvent->uEventParam.btAclConnection.status;
                vos_mem_copy(btDiagEvent.btAddr, pBtEvent->uEventParam.btAclConnection.bdAddr,
                             sizeof(btDiagEvent.btAddr));
                break;

            case BT_EVENT_MODE_CHANGED:
                btDiagEvent.connHandle = pBtEvent->uEventParam.btAclModeChange.connectionHandle;
                btDiagEvent.mode = pBtEvent->uEventParam.btAclModeChange.mode;
                break;

            case BT_EVENT_DISCONNECTION_COMPLETE:
                btDiagEvent.connHandle = pBtEvent->uEventParam.btAclModeChange.connectionHandle;
                break;

            default:
                break;
       }
   }
   WLAN_VOS_DIAG_EVENT_REPORT(&btDiagEvent, EVENT_WLAN_BTC);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
