/*===========================================================================

                       W L A N _ Q C T _ T L _ HOSUPPORT. C
                                               
  OVERVIEW:
  
  DEPENDENCIES: 

  Are listed for each API below. 
  
  
  Copyright (c) 2008 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


   $Header$$DateTime$$Author$


  when        who     what, where, why
----------    ---    --------------------------------------------------------
02/19/09      lti     Vos trace fix
02/06/09      sch     Dereg Bug fix
12/11/08      sch     Initial creation

===========================================================================*/
#include "wlan_qct_tl.h"
#ifdef FEATURE_WLAN_GEN6_ROAMING
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_tl_hosupport.h"
#include "wlan_qct_tli.h"
#include "halCommonApi.h"
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
//#define WLANTL_HO_DEBUG_MSG
//#define WLANTL_HO_UTEST

#define WLANTL_HO_DEFAULT_RSSI      0xFF
#define WLANTL_HO_DEFAULT_ALPHA     5
#define WLANTL_HO_INVALID_RSSI      -100
/* RSSI sampling period, usec based
 * To reduce performance overhead
 * Current default 500msec */
#define WLANTL_HO_SAMPLING_PERIOD   500000

#define TH_MSG_ERROR(a, b, c, d) VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_ERROR, a, b, c, d)
#define TH_MSG_WARN(a, b, c, d)  VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_WARN, a, b, c, d)
#define TH_MSG_INFO(a, b, c, d)  VOS_TRACE(VOS_MODULE_ID_TL, VOS_TRACE_LEVEL_INFO, a, b, c, d)

#define WLANHAL_TX_BD_GET_RMF(_pvBDHeader)          (((tpHalRxBd)_pvBDHeader)->rmf)
#define WLANHAL_TX_BD_GET_UB(_pvBDHeader)           (((tpHalRxBd)_pvBDHeader)->ub)
#define WLANHAL_RX_BD_GET_RMF(_pvBDHeader)          (((tpHalRxBd)_pvBDHeader)->rmf)
#define WLANHAL_RX_BD_GET_UB(_pvBDHeader)           (((tpHalRxBd)_pvBDHeader)->ub)
#define WLANHAL_RX_BD_GET_RATEINDEX(_pvBDHeader)    (((tpHalRxBd)_pvBDHeader)->rateIndex)
#define WLANHAL_RX_BD_GET_TIMESTAMP(_pvBDHeader)    (((tpHalRxBd)_pvBDHeader)->mclkRxTimestamp)


/* Get and release lock */
#define THSGETLOCK(a, b)                                      \
        do                                                    \
        {                                                     \
           if(!VOS_IS_STATUS_SUCCESS(vos_lock_acquire(b)))    \
           {                                                  \
              TH_MSG_ERROR("%s Get Lock Fail", a, 0, 0);      \
              return VOS_STATUS_E_FAILURE;                    \
           }                                                  \
        }while(0)

#define THSRELEASELOCK(a, b)                                  \
        do                                                    \
        {                                                     \
           if(!VOS_IS_STATUS_SUCCESS(vos_lock_release(b)))    \
           {                                                  \
              TH_MSG_ERROR("%s Release Lock Fail", a, 0, 0);  \
              return VOS_STATUS_E_FAILURE;                    \
           }                                                  \
        }while(0)

const v_U8_t  WLANTL_HO_TID_2_AC[WLAN_MAX_TID] = {WLANTL_AC_BE, 
                                                  WLANTL_AC_BK,
                                                  WLANTL_AC_BK, 
                                                  WLANTL_AC_BE,
                                                  WLANTL_AC_VI, 
                                                  WLANTL_AC_VI,
                                                  WLANTL_AC_VO,
                                                  WLANTL_AC_VO};
/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
/* Temporary threshold store place for BMPS */
typedef struct
{
   v_S7_t  rssi;
   v_U8_t  event;
} WLANTL_HSTempPSIndType;

#ifdef WLANTL_HO_UTEST
/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
static v_S7_t   rssi;
static v_S7_t   direction;
void TLHS_UtestHandleNewRSSI(v_S7_t *newRSSI, v_PVOID_t pAdapter)
{
   if(0 == rssi)
   {
      direction = -1;
   }
   else if(-90 == rssi)
   {
      direction = 1;
   }

   *newRSSI = rssi;
   rssi += direction;

   return;
}
#endif /* WLANTL_HO_UTEST */

#ifdef WLANTL_HO_DEBUG_MSG
/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
void WLANTL_StatDebugDisplay
(
   v_U8_t                    STAid,
   WLANTL_TRANSFER_STA_TYPE *statistics
)
{
   TH_MSG_INFO("=================================================", 0, 0, 0);
   TH_MSG_INFO("Statistics for STA %d", STAid, 0, 0);
   TH_MSG_INFO("RX UC Fcnt %5d, MC Fcnt %5d, BC Fcnt %5d",
                statistics->rxUCFcnt, statistics->rxMCFcnt, statistics->rxBCFcnt);
   TH_MSG_INFO("RX UC Bcnt %5d, MC Bcnt %5d, BC Bcnt %5d",
                statistics->rxUCBcnt, statistics->rxMCBcnt, statistics->rxBCBcnt);
   TH_MSG_INFO("TX UC Fcnt %5d, MC Fcnt %5d, BC Fcnt %5d",
                statistics->txUCFcnt, statistics->txMCFcnt, statistics->txBCFcnt);
   TH_MSG_INFO("TX UC Bcnt %5d, MC Bcnt %5d, BC Bcnt %5d",
                statistics->txUCBcnt, statistics->txMCBcnt, statistics->txBCBcnt);
   TH_MSG_INFO("TRX Bcnt %5d, CRCOK Bcnt %5d, RXRate %5d",
                statistics->rxBcnt, statistics->rxBcntCRCok, statistics->rxRate);
   TH_MSG_INFO("=================================================", 0, 0, 0);
   return;
}
#endif /* WLANTL_HO_DEBUG_MSG */

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
void WLANTL_HSDebugDisplay
(
   v_PVOID_t pAdapter
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   v_U8_t                          idx, sIdx;
   v_BOOL_t                        regionFound = VOS_FALSE;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;

   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);

   TH_MSG_ERROR("=================================================", 0, 0, 0);

   for(idx = 0; idx < currentHO->numThreshold; idx++)
   {
      if(idx == currentHO->regionNumber)
      {
         regionFound = VOS_TRUE;
         if(VOS_TRUE == tlCtxt->isBMPS)
         {
            TH_MSG_ERROR(" ----> CRegion %d, hRSSI:NA, BMPS, Alpha %d",
                         currentHO->regionNumber, currentHO->alpha, 0);
         }
         else
         {
            TH_MSG_ERROR(" ----> CRegion %d, hRSSI %d, Alpha %d",
                         currentHO->regionNumber,
                         currentHO->historyRSSI,
                         currentHO->alpha);
         }
      }
      for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
      {
         if(NULL != hoSupport->registeredInd[idx].crossCBFunction[sIdx])
         {
            if(VOS_MODULE_ID_HDD == hoSupport->registeredInd[idx].whoIsClient[sIdx])
            {
               TH_MSG_ERROR("Client HDD pCB %p, triggerEvt %d, RSSI %d",
                   hoSupport->registeredInd[idx].crossCBFunction[sIdx],
                             hoSupport->registeredInd[idx].triggerEvent[sIdx],
                             hoSupport->registeredInd[idx].rssiValue);
   }
            else
            {
               TH_MSG_ERROR("Client SME pCB %p, triggerEvt %d, RSSI %d",
                             hoSupport->registeredInd[idx].crossCBFunction[sIdx],
                             hoSupport->registeredInd[idx].triggerEvent[sIdx],
                             hoSupport->registeredInd[idx].rssiValue);
            }
         }
      }
   }

   if(VOS_FALSE == regionFound)
   {
      if(VOS_TRUE == tlCtxt->isBMPS)
      {
         TH_MSG_ERROR(" ----> CRegion %d, hRSSI:NA, BMPS, Alpha %d",
                      currentHO->regionNumber, currentHO->alpha, 0);
      }
      else
      {
         TH_MSG_ERROR(" ----> CRegion %d, hRSSI %d, Alpha %d",
                      currentHO->regionNumber,
                      currentHO->historyRSSI,
                      currentHO->alpha);
      }
   }

   TH_MSG_ERROR("=================================================", 0, 0, 0);
   return;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_SetFWRSSIThresholds
(
   v_PVOID_t                       pAdapter
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   tSirRSSIThresholds              bmpsThresholds;
   WLANTL_HSTempPSIndType          tempIndSet[WLANTL_SINGLE_CLNT_THRESHOLD];
   v_U8_t                          bmpsLoop;
   v_U8_t                          bmpsInd;
   v_U8_t                          clientLoop;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   WLANTL_HSDebugDisplay(pAdapter);
   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);

   memset((v_U8_t *)&tempIndSet[0], 0, WLANTL_SINGLE_CLNT_THRESHOLD * sizeof(WLANTL_HSTempPSIndType));
   memset(&bmpsThresholds, 0, sizeof(tSirRSSIThresholds));

   bmpsInd = 0;
   for(bmpsLoop = 0; bmpsLoop < WLANTL_MAX_AVAIL_THRESHOLD; bmpsLoop++)
   {
      for(clientLoop = 0; clientLoop < WLANTL_HS_NUM_CLIENT; clientLoop++)
      {
         if(0 != hoSupport->registeredInd[bmpsLoop].triggerEvent[clientLoop])
         {
            if(bmpsInd == WLANTL_SINGLE_CLNT_THRESHOLD)
            {
               TH_MSG_ERROR("Single Client Threashold should be less than %d", WLANTL_SINGLE_CLNT_THRESHOLD, 0, 0);
               TH_MSG_ERROR("Something wrong Out from here", 0, 0, 0);
               break;
            }
            tempIndSet[bmpsInd].rssi  = hoSupport->registeredInd[bmpsLoop].rssiValue;
            tempIndSet[bmpsInd].event = hoSupport->registeredInd[bmpsLoop].triggerEvent[clientLoop];
            bmpsInd++;
            break;
         }
      }
   }

   bmpsThresholds.ucRssiThreshold1 = tempIndSet[0].rssi;
   if((WLANTL_HO_THRESHOLD_DOWN == tempIndSet[0].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[0].event))
   {
      bmpsThresholds.bRssiThres1NegNotify = 1;
   }
   if((WLANTL_HO_THRESHOLD_UP == tempIndSet[0].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[0].event))
   {
      bmpsThresholds.bRssiThres1PosNotify = 1;
   }

   bmpsThresholds.ucRssiThreshold2 = tempIndSet[1].rssi;
   if((WLANTL_HO_THRESHOLD_DOWN == tempIndSet[1].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[1].event))
   {
      bmpsThresholds.bRssiThres2NegNotify = 1;
   }
   if((WLANTL_HO_THRESHOLD_UP == tempIndSet[1].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[1].event))
   {
      bmpsThresholds.bRssiThres2PosNotify = 1;
   }

   bmpsThresholds.ucRssiThreshold3 = tempIndSet[2].rssi;
   if((WLANTL_HO_THRESHOLD_DOWN == tempIndSet[2].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[2].event))
   {
      bmpsThresholds.bRssiThres3NegNotify = 1;
   }
   if((WLANTL_HO_THRESHOLD_UP == tempIndSet[2].event) ||
      (WLANTL_HO_THRESHOLD_CROSS == tempIndSet[2].event))
   {
      bmpsThresholds.bRssiThres3PosNotify = 1;
   }

   halPS_SetRSSIThresholds(hoSupport->macCtxt, &bmpsThresholds);
   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_StatHandleRXFrame
(
   v_PVOID_t        pAdapter,
   v_PVOID_t        pBDHeader,
   v_U8_t           STAid,
   v_BOOL_t         isBroadcast,
   vos_pkt_t       *dataBuffer   
)
{
   WLANTL_CbType            *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                status = VOS_STATUS_SUCCESS;
   WLANTL_TRANSFER_STA_TYPE *statistics;
   v_U16_t                   packetSize;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if(NULL == dataBuffer)
   {
      TH_MSG_INFO("Management Frame, not need to handle with Stat", 0, 0, 0);
      return status;
   }

   if(0 == tlCtxt->atlSTAClients[STAid].ucExists)
   {
      TH_MSG_ERROR("WLAN TL: %d STA ID is not exist", STAid, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   /* TODO : BC/MC/UC have to be determined by MAC address */
   statistics = &tlCtxt->atlSTAClients[STAid].trafficStatistics;
   vos_pkt_get_packet_length(dataBuffer, &packetSize);
   if(WLANHAL_RX_BD_GET_UB(pBDHeader))
   {
      TH_MSG_INFO("This is RX BC/MC frame", 0, 0, 0);
      if(VOS_FALSE == isBroadcast)
      {
         TH_MSG_INFO("This is RX BC frame", 0, 0, 0);
         statistics->rxBCFcnt++;
         statistics->rxBCBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);
      }
      else
      {
         TH_MSG_INFO("This is RX MC frame", 0, 0, 0);
         statistics->rxMCFcnt++;
         statistics->rxMCBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);
      }
   }
   else
   {
      TH_MSG_INFO("This is RX UC frame", 0, 0, 0);
      statistics->rxUCFcnt++;
      statistics->rxUCBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);
   }

   /* TODO caculation is needed, dimension of 500kbps */
   statistics->rxRate = WLANHAL_RX_BD_GET_RATEINDEX(pBDHeader);
   statistics->rxBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);

#ifdef WLANTL_HO_DEBUG_MSG
   WLANTL_StatDebugDisplay(STAid, statistics);
#endif /* WLANTL_HO_DEBUG_MSG */

   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_StatHandleTXFrame
(
   v_PVOID_t        pAdapter,
   v_U8_t           STAid,
   vos_pkt_t       *dataBuffer,
   v_PVOID_t        pBDHeader
)
{
   WLANTL_CbType            *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                status = VOS_STATUS_SUCCESS;
   WLANTL_TRANSFER_STA_TYPE *statistics;
   v_U16_t                   packetSize;

   if((NULL == tlCtxt) || (NULL == dataBuffer))
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if(0 == tlCtxt->atlSTAClients[STAid].ucExists)
   {
      TH_MSG_ERROR("WLAN TL: %d STA ID is not exist", STAid, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   /* TODO : BC/MC/UC have to be determined by MAC address */
   statistics = &tlCtxt->atlSTAClients[STAid].trafficStatistics;
   vos_pkt_get_packet_length(dataBuffer, &packetSize);
   if(WLANTL_STA_ID_BCAST == STAid)
   {
      TH_MSG_INFO("This TX is BC frame", 0, 0, 0);
      statistics->txBCFcnt++;
      statistics->txBCBcnt += (packetSize - WLANHAL_TX_BD_HEADER_SIZE);
   }
/*
   if(WLANHAL_TX_BD_GET_UB(pBDHeader))
   {
      TH_MSG_INFO("This TX is BC/MC frame", 0, 0, 0);
      if(WLANTL_STA_ID_BCAST == STAid)
      {
         TH_MSG_INFO("This TX is BC frame", 0, 0, 0);
         statistics->txBCFcnt++;
         statistics->txBCBcnt += (packetSize - WLANHAL_TX_BD_HEADER_SIZE);
      }
      else
      {
         TH_MSG_INFO("This TX is MC frame", 0, 0, 0);
         statistics->txMCFcnt++;
         statistics->txMCBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);
      }
   }
*/
   else
   {
      TH_MSG_INFO("This is TX UC frame", 0, 0, 0);
      statistics->txUCFcnt++;
      statistics->txUCBcnt += (packetSize - WLANHAL_RX_BD_HEADER_SIZE);
   }

#ifdef WLANTL_HO_DEBUG_MSG
   WLANTL_StatDebugDisplay(STAid, statistics);
#endif /* WLANTL_HO_DEBUG_MSG */

   return status;
}

/*==========================================================================

   FUNCTION  WLANTL_HSTrafficStatusTimerExpired

   DESCRIPTION  If traffic status monitoring timer is expiered,
                Count how may frames have sent and received during 
                measure period and if traffic status is changed
                send notification to Client(SME)
    
   PARAMETERS pAdapter
              Global handle

   RETURN VALUE

============================================================================*/
v_VOID_t WLANTL_HSTrafficStatusTimerExpired
(
   v_PVOID_t pAdapter
)
{
   WLANTL_CbType                        *tlCtxt = VOS_GET_TL_CB(pAdapter);
   WLANTL_HO_TRAFFIC_STATUS_HANDLE_TYPE *trafficHandle = NULL;
   WLANTL_HO_TRAFFIC_STATUS_TYPE         newTraffic;
   v_U32_t                               rtFrameCount;
   v_U32_t                               nrtFrameCount;
   v_BOOL_t                              trafficStatusChanged = VOS_FALSE;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return;
   }

   /* Get rt and nrt frame count sum */
   trafficHandle = &tlCtxt->hoSupport.currentTraffic;
   rtFrameCount  = trafficHandle->rtRXFrameCount + trafficHandle->rtTXFrameCount;
   nrtFrameCount = trafficHandle->nrtRXFrameCount + trafficHandle->nrtTXFrameCount;

   TH_MSG_INFO("Traffic status timer expired RT FC %d, NRT FC %d", rtFrameCount, nrtFrameCount, 0);

   /* Get current traffic status */
   if(rtFrameCount > trafficHandle->idleThreshold)
   {
      newTraffic.rtTrafficStatus = WLANTL_HO_RT_TRAFFIC_STATUS_ON;
   }
   else
   {
      newTraffic.rtTrafficStatus = WLANTL_HO_RT_TRAFFIC_STATUS_OFF;
   }

   if(nrtFrameCount > trafficHandle->idleThreshold)
   {
      newTraffic.nrtTrafficStatus = WLANTL_HO_NRT_TRAFFIC_STATUS_ON;
   }
   else
   {
      newTraffic.nrtTrafficStatus = WLANTL_HO_NRT_TRAFFIC_STATUS_OFF;
   }

   /* Differentiate with old traffic status */
   if(trafficHandle->trafficStatus.rtTrafficStatus != newTraffic.rtTrafficStatus)
   {
      TH_MSG_WARN("RT Traffic status changed from %d to %d",
                   trafficHandle->trafficStatus.rtTrafficStatus,
                   newTraffic.rtTrafficStatus, 0);
      trafficStatusChanged = VOS_TRUE;
   }
   if(trafficHandle->trafficStatus.nrtTrafficStatus != newTraffic.nrtTrafficStatus)
   {
      TH_MSG_WARN("NRT Traffic status changed from %d to %d",
                   trafficHandle->trafficStatus.nrtTrafficStatus,
                   newTraffic.nrtTrafficStatus, 0);
      trafficStatusChanged = VOS_TRUE;
   }

   /* If traffic status is changed send notification to client */
   if((VOS_TRUE == trafficStatusChanged) && (NULL != trafficHandle->trafficCB))
   {
      trafficHandle->trafficCB(pAdapter, newTraffic, trafficHandle->usrCtxt);
      trafficHandle->trafficStatus.rtTrafficStatus = newTraffic.rtTrafficStatus;
      trafficHandle->trafficStatus.nrtTrafficStatus = newTraffic.nrtTrafficStatus;
   }
   else if((VOS_TRUE == trafficStatusChanged) && (NULL == trafficHandle->trafficCB))
   {
      TH_MSG_WARN("Traffic status is changed but not need to report", 0, 0, 0);
   }

   /* Reset frame counters */
   trafficHandle->rtRXFrameCount = 0;
   trafficHandle->rtTXFrameCount = 0;
   trafficHandle->nrtRXFrameCount = 0;
   trafficHandle->nrtTXFrameCount = 0;

   /* restart timer */
   vos_timer_start(&trafficHandle->trafficTimer, trafficHandle->measurePeriod);

   return;
}


/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSGetRSSI
(
   v_PVOID_t        pAdapter,
   v_PVOID_t        pBDHeader,
   v_U8_t           STAid,
   v_S7_t          *currentAvgRSSI
)
{
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;
   v_S7_t           currentRSSI, currentRSSI0, currentRSSI1;
   WLANTL_CURRENT_HO_STATE_TYPE *currentHO = NULL;


   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   /* 
	 Compute RSSI only for the last MPDU of an AMPDU.
	 Only last MPDU carries the Phy Stats Values 
	 */
    if (WLAN_HAL_IS_AN_AMPDU (pBDHeader)) {
       if (!WLAN_HAL_IS_LAST_MPDU(pBDHeader)) {
           return VOS_STATUS_E_FAILURE;
          }
    }

   currentHO = &tlCtxt->hoSupport.currentHOState;

   currentRSSI0 = WLANTL_GETRSSI0(pBDHeader);
   currentRSSI1 = WLANTL_GETRSSI1(pBDHeader);
   currentRSSI  = (currentRSSI0 > currentRSSI1) ? currentRSSI0 : currentRSSI1;


#ifdef WLANTL_HO_UTEST
   TLHS_UtestHandleNewRSSI(&currentRSSI, pAdapter);
#endif /* WLANTL_HO_UTEST */

/* Commenting this part of the code as this may not be necessarity true in all cases */
#if 0
   if(WLANTL_HO_INVALID_RSSI == currentRSSI)
   {
      return status;
   }
#endif

   if(0 == currentHO->historyRSSI)
   {
      *currentAvgRSSI = currentRSSI;
   }
   else
   {
      *currentAvgRSSI = ((currentHO->historyRSSI * currentHO->alpha) +
                         (currentRSSI * (10 - currentHO->alpha))) / 10;
   }

   tlCtxt->atlSTAClients[STAid].uRssiAvg = *currentAvgRSSI;

   TH_MSG_INFO("Current new RSSI is %d, averaged RSSI is %d", currentRSSI, *currentAvgRSSI, 0);
   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSBMPSRSSIRegionChangedNotification
(
   v_PVOID_t             pAdapter,
   tpSirRSSINotification pRSSINotification
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;
   WLANTL_RSSICrossThresholdCBType cbFunction = NULL;
   v_PVOID_t                       usrCtxt = NULL;
   v_U8_t                          evtType = WLANTL_HO_THRESHOLD_NA;
   static v_U32_t                  preFWNotification = 0;
   v_U32_t                         curFWNotification = 0;
   v_U8_t                          newRegionNumber = 0;
   v_U8_t                          pRegionNumber = 0, nRegionNumber = 0;
   v_U32_t                         isSet;
   v_U8_t                          idx, sIdx;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if(NULL == pRSSINotification)
   {
      TH_MSG_ERROR("Invalid FW RSSI Notification", 0, 0, 0);
      VOS_ASSERT(0);
      return VOS_STATUS_E_INVAL;
   }

   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);

   isSet = pRSSINotification->bRssiThres1PosCross;
   curFWNotification |= isSet << 5;
   isSet = pRSSINotification->bRssiThres2PosCross;
   curFWNotification |= isSet << 4;
   isSet = pRSSINotification->bRssiThres3PosCross;
   curFWNotification |= isSet << 3;
   isSet = pRSSINotification->bRssiThres1NegCross;
   curFWNotification |= isSet << 2;
   isSet = pRSSINotification->bRssiThres2NegCross;
   curFWNotification |= isSet << 1;
   isSet = pRSSINotification->bRssiThres3NegCross;
   curFWNotification |= isSet;
   TH_MSG_INFO("Current FW Notification is 0x%x", (v_U32_t)curFWNotification, 0, 0);

   if(0 == preFWNotification)
   {
      TH_MSG_INFO("This is the first time notification from FW Value is 0x%x", curFWNotification, 0, 0);
      preFWNotification = curFWNotification;
   }
   else if(preFWNotification == curFWNotification)
   {
      return status;
   }

   if(1 == pRSSINotification->bRssiThres1PosCross)
   {
      TH_MSG_INFO("POS Cross to Region 0", 0, 0, 0);
      pRegionNumber = 0;
   }
   else if(1 == pRSSINotification->bRssiThres2PosCross)
   {
      TH_MSG_INFO("POS Cross to Region 1", 0, 0, 0);
      pRegionNumber = 1;
   }
   else if(1 == pRSSINotification->bRssiThres3PosCross)
   {
      TH_MSG_INFO("POS Cross to Region 2", 0, 0, 0);
      pRegionNumber = 2;
   }

   if(1 == pRSSINotification->bRssiThres3NegCross)
   {
      TH_MSG_INFO("NEG Cross to Region 3", 0, 0, 0);
      nRegionNumber = 3;
   }
   else if(1 == pRSSINotification->bRssiThres2NegCross)
   {
      TH_MSG_INFO("NEG Cross to Region 2", 0, 0, 0);
      nRegionNumber = 2;
   }
   else if(1 == pRSSINotification->bRssiThres1NegCross)
   {
      TH_MSG_INFO("NEG Cross to Region 1", 0, 0, 0);
      nRegionNumber = 1;
   }

   newRegionNumber = (nRegionNumber > pRegionNumber) ? nRegionNumber : pRegionNumber;
   if(newRegionNumber == currentHO->regionNumber)
   {
      TH_MSG_INFO("No Region Change with BMPS mode", 0, 0, 0);
      preFWNotification = curFWNotification;
      return status;
   }
   else if(newRegionNumber > currentHO->regionNumber)
   {
      TH_MSG_ERROR("Region Increase Worse RSSI", 0, 0, 0);
      for(idx = currentHO->regionNumber; idx < newRegionNumber; idx++)
      {
         for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
         {
            if((WLANTL_HO_THRESHOLD_DOWN == hoSupport->registeredInd[idx].triggerEvent[sIdx]) ||
               (WLANTL_HO_THRESHOLD_CROSS == hoSupport->registeredInd[idx].triggerEvent[sIdx]))
            {
               if(NULL != hoSupport->registeredInd[idx].crossCBFunction[sIdx])
               {
                  cbFunction = hoSupport->registeredInd[idx].crossCBFunction[sIdx];
                  usrCtxt = hoSupport->registeredInd[idx].usrCtxt[sIdx];
                  evtType = WLANTL_HO_THRESHOLD_DOWN;
                  TH_MSG_ERROR("Trigger Event %d, region index %d", hoSupport->registeredInd[idx].triggerEvent[sIdx], idx, 0);
                  currentHO->regionNumber = newRegionNumber;
                  status = cbFunction(pAdapter, evtType, usrCtxt);
               }
            }
         }
      }
   }
   else
   {
      TH_MSG_ERROR("Region Decrease Better RSSI", 0, 0, 0);
      for(idx = currentHO->regionNumber; idx > newRegionNumber; idx--)
      {
         for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
         {
            if((WLANTL_HO_THRESHOLD_UP & hoSupport->registeredInd[idx - 1].triggerEvent[sIdx]) ||
               (WLANTL_HO_THRESHOLD_CROSS & hoSupport->registeredInd[idx - 1].triggerEvent[sIdx]))
            {
               if(NULL != hoSupport->registeredInd[idx - 1].crossCBFunction[sIdx])
               {
                  cbFunction = hoSupport->registeredInd[idx - 1].crossCBFunction[sIdx];
                  usrCtxt = hoSupport->registeredInd[idx - 1].usrCtxt[sIdx];
                  evtType = WLANTL_HO_THRESHOLD_UP;
                  TH_MSG_ERROR("Trigger Event %d, region index %d", hoSupport->registeredInd[idx - 1].triggerEvent[sIdx], idx - 1, 0);
                  currentHO->regionNumber = newRegionNumber;
                  status = cbFunction(pAdapter, evtType, usrCtxt);
               }
            }
         }
      }
   }

   TH_MSG_INFO("BMPS State, MSG from FW, Trigger Event %d, region index %d",
                 evtType, currentHO->regionNumber, 0);
   preFWNotification = curFWNotification;

   return VOS_STATUS_SUCCESS;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSHandleRSSIChange
(
   v_PVOID_t   pAdapter,
   v_S7_t      currentRSSI
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;
   v_U8_t                          currentRegion = 0;
   v_U8_t                          idx, sIdx;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;
   WLANTL_RSSICrossThresholdCBType cbFunction = NULL;
   v_PVOID_t                       usrCtxt = NULL;
   v_U8_t                          evtType = WLANTL_HO_THRESHOLD_NA;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);
   TH_MSG_INFO("CRegion %d, NThreshold %d, HRSSI %d",
                currentHO->regionNumber,
                currentHO->numThreshold,
                currentHO->historyRSSI);

   /* Find where is current region */
   for(idx = 0; idx < currentHO->numThreshold; idx++)
   {
      if(hoSupport->registeredInd[idx].rssiValue < currentRSSI)
      {
         currentRegion = idx;
         TH_MSG_INFO("Found region %d, not bottom", currentRegion, 0, 0);
         break;
      }
   }

   /* If could not find then new RSSI is belong to bottom region */
   if(idx == currentHO->numThreshold)
   {
      currentRegion = idx;
      TH_MSG_INFO("Current region is bottom %d", idx, 0, 0);
   }

   if(currentRegion == currentHO->regionNumber)
   {
      currentHO->historyRSSI = currentRSSI;
      return status;
   }
   else if(currentRegion > currentHO->regionNumber)
   {
      TH_MSG_ERROR("Region Increase Worse RSSI", 0, 0, 0);
      for(idx = currentHO->regionNumber; idx < currentRegion; idx++)
      {
         for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
         {
            if((WLANTL_HO_THRESHOLD_DOWN == hoSupport->registeredInd[idx].triggerEvent[sIdx]) ||
               (WLANTL_HO_THRESHOLD_CROSS == hoSupport->registeredInd[idx].triggerEvent[sIdx]))
            {
               if(NULL != hoSupport->registeredInd[idx].crossCBFunction[sIdx])
               {
                  cbFunction = hoSupport->registeredInd[idx].crossCBFunction[sIdx];
                  usrCtxt = hoSupport->registeredInd[idx].usrCtxt[sIdx];
                  evtType = WLANTL_HO_THRESHOLD_DOWN;
                  TH_MSG_ERROR("Trigger Event %d, region index %d", hoSupport->registeredInd[idx].triggerEvent[sIdx], idx, 0);
                  status = cbFunction(pAdapter, evtType, usrCtxt);
               }
            }
         }
      }
   }
   else
   {
      TH_MSG_ERROR("Region Decrease Better RSSI", 0, 0, 0);
      for(idx = currentHO->regionNumber; idx > currentRegion; idx--)
      {
         for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
         {
            if((WLANTL_HO_THRESHOLD_UP & hoSupport->registeredInd[idx - 1].triggerEvent[sIdx]) ||
               (WLANTL_HO_THRESHOLD_CROSS & hoSupport->registeredInd[idx - 1].triggerEvent[sIdx]))
            {
               if(NULL != hoSupport->registeredInd[idx - 1].crossCBFunction[sIdx])
               {
                  cbFunction = hoSupport->registeredInd[idx - 1].crossCBFunction[sIdx];
                  usrCtxt = hoSupport->registeredInd[idx - 1].usrCtxt[sIdx];
                  evtType = WLANTL_HO_THRESHOLD_UP;
                  TH_MSG_ERROR("Trigger Event %d, region index %d", hoSupport->registeredInd[idx - 1].triggerEvent[sIdx], idx - 1, 0);
                  status = cbFunction(pAdapter, evtType, usrCtxt);
               }
            }
         }
      }
   }

   currentHO->historyRSSI = currentRSSI;
   currentHO->regionNumber = currentRegion;
   WLANTL_HSDebugDisplay(pAdapter);

   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      TH_MSG_ERROR("Client fail to handle region change in normal mode %d", status, 0, 0);
   }
   return VOS_STATUS_SUCCESS;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSHandleRXFrame
(
   v_PVOID_t        pAdapter,
   v_U8_t           frameType,
   v_PVOID_t        pBDHeader,
   v_U8_t           STAid,
   v_BOOL_t         isBroadcast,
   vos_pkt_t       *dataBuffer
)
{
   WLANTL_CURRENT_HO_STATE_TYPE *currentHO = NULL;
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;
   v_S7_t           currentAvgRSSI = 0;
   v_U8_t           ac;
   v_U32_t          currentTimestamp;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   WLANTL_StatHandleRXFrame(pAdapter, pBDHeader, STAid, isBroadcast, dataBuffer);

   /* If this frame is not management frame increase frame count */
   if((0 != tlCtxt->hoSupport.currentTraffic.idleThreshold) &&
      (WLANTL_MGMT_FRAME_TYPE != frameType))
   {
      ac = WLANTL_HO_TID_2_AC[(v_U8_t)WLANHAL_RX_BD_GET_TID(pBDHeader)];

      /* Only Voice traffic is handled as real time traffic */
      if(WLANTL_AC_VO == ac)
      {
         tlCtxt->hoSupport.currentTraffic.rtRXFrameCount++;
      }
      else
      {
         tlCtxt->hoSupport.currentTraffic.nrtRXFrameCount++;
      }
      TH_MSG_INFO("RX frame AC %d, RT Frame Count %d, NRT Frame Count %d",
                   ac,
                   tlCtxt->hoSupport.currentTraffic.rtRXFrameCount,
                   tlCtxt->hoSupport.currentTraffic.nrtRXFrameCount);
   }

   currentHO = &tlCtxt->hoSupport.currentHOState;
   if(VOS_TRUE == tlCtxt->isBMPS)
   {
      WLANTL_HSGetRSSI(pAdapter, pBDHeader, STAid, &currentAvgRSSI);
      return status;
   }

   currentTimestamp = WLANHAL_RX_BD_GET_TIMESTAMP(pBDHeader);
   if((currentTimestamp - currentHO->sampleTime) < WLANTL_HO_SAMPLING_PERIOD)
   {
      return status;
   }
   currentHO->sampleTime = currentTimestamp;

   /* If any threshold is not registerd, DO NOTHING! */
   if(0 == tlCtxt->hoSupport.currentHOState.numThreshold)
   {
      TH_MSG_INFO("There is no thresholds pass", 0, 0 ,0);
   }
   else
   {
      /* Get Current RSSI from BD Heaser */
      status = WLANTL_HSGetRSSI(pAdapter, pBDHeader, STAid, &currentAvgRSSI);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         TH_MSG_INFO("Get RSSI Fail", 0, 0, 0);
         return status;
      }
      /* Handle current RSSI value, region, notification, etc */
      status = WLANTL_HSHandleRSSIChange(pAdapter, currentAvgRSSI);
      if(!VOS_IS_STATUS_SUCCESS(status))
      {
         TH_MSG_ERROR("Handle new RSSI fail", 0, 0, 0);
         return status;
      }
   }

   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSHandleTXFrame
(
   v_PVOID_t        pAdapter,
   v_U8_t           ac,
   v_U8_t           STAid,
   vos_pkt_t       *dataBuffer,
   v_PVOID_t        bdHeader
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   /* Traffic status report is not registered, JUST DO NOTHING */
   if(0 == tlCtxt->hoSupport.currentTraffic.idleThreshold)
   {
      return VOS_STATUS_SUCCESS;
   }

   WLANTL_StatHandleTXFrame(pAdapter, STAid, dataBuffer, bdHeader);

   /* Only Voice traffic is handled as real time traffic */
   if(WLANTL_AC_VO == ac)
   {
      tlCtxt->hoSupport.currentTraffic.rtTXFrameCount++;
   }
   else
   {
      tlCtxt->hoSupport.currentTraffic.nrtTXFrameCount++;
   }
   TH_MSG_INFO("TX frame AC %d, RT Frame Count %d, NRT Frame Count %d",
                ac,
                tlCtxt->hoSupport.currentTraffic.rtTXFrameCount,
                tlCtxt->hoSupport.currentTraffic.nrtTXFrameCount);

   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSRegRSSIIndicationCB
(
   v_PVOID_t                       pAdapter,
   v_S7_t                          rssiValue,
   v_U8_t                          triggerEvent,
   WLANTL_RSSICrossThresholdCBType crossCBFunction,
   VOS_MODULE_ID                   moduleID,
   v_PVOID_t                       usrCtxt
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;
   v_U8_t                          idx, sIdx;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   v_U8_t                          clientOrder;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if((-1 < rssiValue) || (NULL == crossCBFunction))
   {
      TH_MSG_ERROR("Reg Invalid Argument", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   THSGETLOCK("WLANTL_HSRegRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);

   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);
   TH_MSG_ERROR("Make Registration Module %d, Event %d, RSSI %d", moduleID, triggerEvent, rssiValue);

   if((WLANTL_MAX_AVAIL_THRESHOLD < currentHO->numThreshold) ||
      (WLANTL_MAX_AVAIL_THRESHOLD == currentHO->numThreshold))
   {
      TH_MSG_ERROR("No more available slot, please DEL first %d",
                    currentHO->numThreshold, 0, 0);
      THSRELEASELOCK("WLANTL_HSRegRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
      return VOS_STATUS_E_RESOURCES;
   }

   if(0 == currentHO->numThreshold)
   {
      TH_MSG_INFO("First Registration", 0, 0, 0);
      hoSupport->registeredInd[0].rssiValue    = rssiValue;
      hoSupport->registeredInd[0].triggerEvent[0]    = triggerEvent;
      hoSupport->registeredInd[0].crossCBFunction[0] = crossCBFunction;
      hoSupport->registeredInd[0].usrCtxt[0]         = usrCtxt;
      hoSupport->registeredInd[0].whoIsClient[0]     = moduleID;
      hoSupport->registeredInd[0].numClient++;
   }
   else
   {
      for(idx = 0; idx < currentHO->numThreshold; idx++)
      {
         if(rssiValue == hoSupport->registeredInd[idx].rssiValue)
         {
            for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
            {
               TH_MSG_ERROR("Reg CB P 0x%x, registered CB P 0x%x",
                             crossCBFunction,
                             hoSupport->registeredInd[idx].crossCBFunction[sIdx], 0);
               if(crossCBFunction == hoSupport->registeredInd[idx].crossCBFunction[sIdx])
               {
                  TH_MSG_ERROR("Same RSSI %d, Same CB 0x%x already registered",
                               rssiValue, crossCBFunction, 0);
                  WLANTL_HSDebugDisplay(pAdapter);
                  THSRELEASELOCK("WLANTL_HSRegRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
                  return status;
               }
            }

            for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
            {
               if(NULL == hoSupport->registeredInd[idx].crossCBFunction[sIdx])
               {
                  clientOrder = sIdx;
                  break;
               }
            }
            hoSupport->registeredInd[idx].triggerEvent[clientOrder]    = triggerEvent;
            hoSupport->registeredInd[idx].crossCBFunction[clientOrder] = crossCBFunction;
            hoSupport->registeredInd[idx].usrCtxt[clientOrder]         = usrCtxt;
            hoSupport->registeredInd[idx].whoIsClient[clientOrder]     = moduleID;
            hoSupport->registeredInd[idx].numClient++;
            WLANTL_HSDebugDisplay(pAdapter);
            THSRELEASELOCK("WLANTL_HSRegRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
            return status;
         }
      }
      for(idx = 0; idx < currentHO->numThreshold; idx++)
      {
         if(rssiValue > hoSupport->registeredInd[idx].rssiValue)
         {
            for(sIdx = (currentHO->numThreshold - 1); (sIdx > idx) || (sIdx == idx); sIdx--)
            {
               TH_MSG_INFO("Shift %d array to %d", sIdx, sIdx + 1, 0);
               memcpy(&hoSupport->registeredInd[sIdx + 1], &hoSupport->registeredInd[sIdx], sizeof(WLANTL_HO_RSSI_INDICATION_TYPE));
               memset(&hoSupport->registeredInd[sIdx], 0, sizeof(WLANTL_HO_RSSI_INDICATION_TYPE));
			   if(0 == sIdx)
               {
                  break;
               }
            }
            TH_MSG_INFO("Put in Here %d", idx , 0, 0);
            hoSupport->registeredInd[idx].rssiValue    = rssiValue;
            hoSupport->registeredInd[idx].triggerEvent[0]    = triggerEvent;
            hoSupport->registeredInd[idx].crossCBFunction[0] = crossCBFunction;
            hoSupport->registeredInd[idx].usrCtxt[0]         = usrCtxt;
            hoSupport->registeredInd[idx].whoIsClient[0]     = moduleID;
            hoSupport->registeredInd[idx].numClient++;
            break;
         }
      }
      if(currentHO->numThreshold == idx)
      {
         TH_MSG_INFO("New threshold put in bottom", 0, 0, 0);

         hoSupport->registeredInd[currentHO->numThreshold].rssiValue    = rssiValue;
         hoSupport->registeredInd[currentHO->numThreshold].triggerEvent[0] = triggerEvent;
         hoSupport->registeredInd[currentHO->numThreshold].crossCBFunction[0] = crossCBFunction;
         hoSupport->registeredInd[currentHO->numThreshold].usrCtxt[0] = usrCtxt;
         hoSupport->registeredInd[currentHO->numThreshold].whoIsClient[0]     = moduleID;
         hoSupport->registeredInd[currentHO->numThreshold].numClient++;
      }
   }

   currentHO->numThreshold++;
   if((VOS_FALSE == tlCtxt->isBMPS) && (rssiValue > currentHO->historyRSSI))
   {
      TH_MSG_ERROR("Added Threashold above current RSSI levle, old RN %d", currentHO->regionNumber, 0, 0);
      if(4 > currentHO->regionNumber)
      {
         currentHO->regionNumber++;
      }
      else
      {
         TH_MSG_ERROR("Current region number is max %d, cannot increase anymore", currentHO->regionNumber, 0, 0);
      }
      TH_MSG_ERROR("increase region number without notification %d", currentHO->regionNumber, 0, 0);
   }
   else if(VOS_TRUE == tlCtxt->isBMPS)
   {
      if(0 != currentHO->regionNumber)
      {
         if(hoSupport->registeredInd[currentHO->regionNumber].rssiValue < rssiValue)
         {
            currentHO->regionNumber++;
            if((WLANTL_HO_THRESHOLD_DOWN == triggerEvent) || (WLANTL_HO_THRESHOLD_CROSS == triggerEvent))
            {
               TH_MSG_ERROR("Registered RSSI value larger than Current RSSI, and DOWN event, Send Notification", 0, 0, 0);
               crossCBFunction(pAdapter, WLANTL_HO_THRESHOLD_DOWN, usrCtxt);
            }
         }
         else if((currentHO->regionNumber < (currentHO->numThreshold - 1)) &&
                 (hoSupport->registeredInd[currentHO->regionNumber + 1].rssiValue > rssiValue))
         {
            if((WLANTL_HO_THRESHOLD_UP == triggerEvent) || (WLANTL_HO_THRESHOLD_CROSS == triggerEvent))
            {
               TH_MSG_ERROR("Registered RSSI value smaller than Current RSSI, and UP event, Send Notification", 0, 0, 0);
               crossCBFunction(pAdapter, WLANTL_HO_THRESHOLD_UP, usrCtxt);
            }
         }
      }
      else
      {
         if(hoSupport->registeredInd[currentHO->regionNumber].rssiValue > rssiValue)
         {
            if((WLANTL_HO_THRESHOLD_UP == triggerEvent) || (WLANTL_HO_THRESHOLD_CROSS == triggerEvent))
            {
               TH_MSG_ERROR("Registered RSSI value smaller than Current RSSI, and UP event, Send Notification", 0, 0, 0);
               crossCBFunction(pAdapter, WLANTL_HO_THRESHOLD_UP, usrCtxt);
            }
         }
      }
   }

   if((VOS_FALSE == tlCtxt->isBMPS) &&
      (rssiValue >= currentHO->historyRSSI) && (0 != currentHO->historyRSSI) &&
      ((WLANTL_HO_THRESHOLD_DOWN == triggerEvent) || (WLANTL_HO_THRESHOLD_CROSS == triggerEvent)))
   {
      TH_MSG_ERROR("Registered RSSI value larger than Current RSSI, and DOWN event, Send Notification", 0, 0, 0);
      crossCBFunction(pAdapter, WLANTL_HO_THRESHOLD_DOWN, usrCtxt);
   }
   else if((VOS_FALSE == tlCtxt->isBMPS) &&
           (rssiValue < currentHO->historyRSSI) && (0 != currentHO->historyRSSI) &&
           ((WLANTL_HO_THRESHOLD_UP == triggerEvent) || (WLANTL_HO_THRESHOLD_CROSS == triggerEvent)))
   {
      TH_MSG_ERROR("Registered RSSI value smaller than Current RSSI, and UP event, Send Notification", 0, 0, 0);
      crossCBFunction(pAdapter, WLANTL_HO_THRESHOLD_UP, usrCtxt);
   }

   if(VOS_TRUE == tlCtxt->isBMPS)
   {
      TH_MSG_ERROR("Register into FW, now BMPS", 0, 0, 0);
      WLANTL_SetFWRSSIThresholds(pAdapter);
   }

   WLANTL_HSDebugDisplay(pAdapter);
   THSRELEASELOCK("WLANTL_HSRegRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSDeregRSSIIndicationCB
(
   v_PVOID_t                       pAdapter,
   v_S7_t                          rssiValue,
   v_U8_t                          triggerEvent,
   WLANTL_RSSICrossThresholdCBType crossCBFunction,
   VOS_MODULE_ID                   moduleID
)
{
   WLANTL_CbType                  *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS                      status = VOS_STATUS_SUCCESS;
   v_U8_t                          idx, sIdx;
   WLANTL_HO_SUPPORT_TYPE         *hoSupport;
   WLANTL_CURRENT_HO_STATE_TYPE   *currentHO;
   v_BOOL_t                        bmpsAbove = VOS_FALSE;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if(0 == tlCtxt->hoSupport.currentHOState.numThreshold)
   {
      TH_MSG_ERROR("Empty list, can not remove", 0, 0, 0);
      return VOS_STATUS_E_EMPTY;
   }

   THSGETLOCK("WLANTL_HSDeregRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
   currentHO = &(tlCtxt->hoSupport.currentHOState);
   hoSupport = &(tlCtxt->hoSupport);

   TH_MSG_INFO("Delete Registration", 0, 0, 0);
   TH_MSG_ERROR("DEL target RSSI %d, event %d", rssiValue, triggerEvent, 0);

   if((VOS_TRUE == tlCtxt->isBMPS) && (0 < currentHO->regionNumber))
   {
      if(rssiValue >= hoSupport->registeredInd[currentHO->regionNumber - 1].rssiValue)
      {
         bmpsAbove = VOS_TRUE;
         TH_MSG_ERROR("Remove Threshold larger than current region", 0, 0, 0);
      }
   }

   for(idx = 0; idx < currentHO->numThreshold; idx++)
   {
      if(rssiValue == hoSupport->registeredInd[idx].rssiValue)
      {
         for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
         {
            if(crossCBFunction == tlCtxt->hoSupport.registeredInd[idx].crossCBFunction[sIdx])
            {
               tlCtxt->hoSupport.registeredInd[idx].triggerEvent[sIdx]    = 0;
               tlCtxt->hoSupport.registeredInd[idx].crossCBFunction[sIdx] = NULL;
               tlCtxt->hoSupport.registeredInd[idx].usrCtxt[sIdx]         = NULL;
               tlCtxt->hoSupport.registeredInd[idx].whoIsClient[sIdx]     = 0;
               tlCtxt->hoSupport.registeredInd[idx].numClient--;
            }
         }
         if(0 != tlCtxt->hoSupport.registeredInd[idx].numClient)
         {
            TH_MSG_ERROR("Found Multiple idx is %d", idx, 0, 0);
            WLANTL_HSDebugDisplay(pAdapter);
            THSRELEASELOCK("WLANTL_HSDeregRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
            return status;
         }
         else
         {
            TH_MSG_ERROR("Found Single idx is %d", idx, 0, 0);
            break;
         }
      }
   }
   if(idx == currentHO->numThreshold)
   {
      TH_MSG_ERROR("Cound not find entry, maybe invalid arg", 0, 0, 0);
      THSRELEASELOCK("WLANTL_HSDeregRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
      return VOS_STATUS_E_INVAL;
   }

   for(idx = 0; idx < currentHO->numThreshold; idx++)
   {
      if(rssiValue == hoSupport->registeredInd[idx].rssiValue)
      {
         if((currentHO->numThreshold - 1) == idx)
         {
            TH_MSG_INFO("Remove target is last one", 0, 0, 0);
            /* Does not need move any element, just remove last array entry */
         }
         else
         {
            for(sIdx = idx; sIdx < (currentHO->numThreshold - 1); sIdx++)
            {
               TH_MSG_INFO("Shift up from %d to %d", sIdx + 1, sIdx, 0);
               memcpy(&hoSupport->registeredInd[sIdx], &hoSupport->registeredInd[sIdx + 1], sizeof(WLANTL_HO_RSSI_INDICATION_TYPE));
            }
         }
         break;
      }
   }
   /* Common remove last array entry */
   tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].rssiValue    = WLANTL_HO_DEFAULT_RSSI;
   for(idx = 0; idx < WLANTL_HS_NUM_CLIENT; idx++)
   {
      tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].triggerEvent[idx]    = WLANTL_HO_THRESHOLD_NA;
      tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].crossCBFunction[idx] = NULL;
      tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].usrCtxt[idx]         = NULL;
      tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].whoIsClient[idx]     = 0;
      tlCtxt->hoSupport.registeredInd[currentHO->numThreshold - 1].numClient            = 0;
   }

   if((VOS_FALSE == tlCtxt->isBMPS) && (rssiValue >= currentHO->historyRSSI))
   {
      TH_MSG_ERROR("Removed Threashold above current RSSI levle, old RN %d", currentHO->regionNumber, 0, 0);
      if(0 < currentHO->regionNumber)
      {
         currentHO->regionNumber--;
      }
      else
      {
         TH_MSG_ERROR("Current Region number is 0, cannot decrease anymore", 0, 0, 0);
      }
      TH_MSG_ERROR("Decrese region number without notification %d", currentHO->regionNumber, 0, 0);
   }
   else if((VOS_TRUE == tlCtxt->isBMPS) && (VOS_TRUE == bmpsAbove))
   {
      currentHO->regionNumber--;
   }
   /* Decrease number of thresholds */
   tlCtxt->hoSupport.currentHOState.numThreshold--;

   if(VOS_TRUE == tlCtxt->isBMPS)
   {
      TH_MSG_ERROR("Register into FW, now BMPS", 0, 0, 0);
      WLANTL_SetFWRSSIThresholds(pAdapter);
   }

   /* Based on new threshold set recalculated current RSSI status */
   if(0 < tlCtxt->hoSupport.currentHOState.numThreshold)
   {
   }
   else if(0 == tlCtxt->hoSupport.currentHOState.numThreshold)
   {
      currentHO->regionNumber = 0;
      TH_MSG_WARN("No registered Threshold", 0, 0, 0);
      /* What should do? */
   }

   WLANTL_HSDebugDisplay(pAdapter);
   THSRELEASELOCK("WLANTL_HSDeregRSSIIndicationCB", &tlCtxt->hoSupport.hosLock);
   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSSetAlpha
(
   v_PVOID_t pAdapter,
   int       valueAlpha
)
{
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   THSGETLOCK("WLANTL_HSSetAlpha", &tlCtxt->hoSupport.hosLock);
   tlCtxt->hoSupport.currentHOState.alpha = (v_U8_t)valueAlpha;
   THSRELEASELOCK("WLANTL_HSSetAlpha", &tlCtxt->hoSupport.hosLock);
   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSRegGetTrafficStatus
(
   v_PVOID_t                          pAdapter,
   v_U32_t                            idleThreshold,
   v_U32_t                            period,
   WLANTL_TrafficStatusChangedCBType  trfficStatusCB,
   v_PVOID_t                          usrCtxt
)
{
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   if((0 == idleThreshold) || (0 == period) || (NULL == trfficStatusCB))
   {
      TH_MSG_ERROR("Invalid Argument Passed from SME", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   tlCtxt->hoSupport.currentTraffic.idleThreshold = idleThreshold;
   tlCtxt->hoSupport.currentTraffic.measurePeriod = period;
   tlCtxt->hoSupport.currentTraffic.trafficCB     = trfficStatusCB;
   tlCtxt->hoSupport.currentTraffic.usrCtxt       = usrCtxt;

   vos_timer_start(&tlCtxt->hoSupport.currentTraffic.trafficTimer,
                   tlCtxt->hoSupport.currentTraffic.measurePeriod);

   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSInit
(
   v_PVOID_t   pAdapter
)
{
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;
   v_U8_t           idx, sIdx;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }
#ifdef WLANTL_HO_UTEST
   rssi = 0;
   direction = -1;
#endif /* WLANTL_HO_UTEST */

   /* set default current HO status */
   tlCtxt->hoSupport.currentHOState.alpha        = WLANTL_HO_DEFAULT_ALPHA;
   tlCtxt->hoSupport.currentHOState.historyRSSI  = 0;
   tlCtxt->hoSupport.currentHOState.numThreshold = 0;
   tlCtxt->hoSupport.currentHOState.regionNumber = 0;
   tlCtxt->hoSupport.currentHOState.sampleTime   = 0;

   /* set default current traffic status */
   tlCtxt->hoSupport.currentTraffic.trafficStatus.rtTrafficStatus
                                                    = WLANTL_HO_RT_TRAFFIC_STATUS_OFF;
   tlCtxt->hoSupport.currentTraffic.trafficStatus.nrtTrafficStatus
                                                    = WLANTL_HO_NRT_TRAFFIC_STATUS_OFF;
   tlCtxt->hoSupport.currentTraffic.idleThreshold   = 0;
   tlCtxt->hoSupport.currentTraffic.measurePeriod   = 0;
   tlCtxt->hoSupport.currentTraffic.rtRXFrameCount  = 0;
   tlCtxt->hoSupport.currentTraffic.rtTXFrameCount  = 0;
   tlCtxt->hoSupport.currentTraffic.nrtRXFrameCount = 0;
   tlCtxt->hoSupport.currentTraffic.nrtTXFrameCount = 0;
   tlCtxt->hoSupport.currentTraffic.trafficCB       = NULL;

   /* Initialize indication array */
   for(idx = 0; idx < WLANTL_MAX_AVAIL_THRESHOLD; idx++)
   {
      for(sIdx = 0; sIdx < WLANTL_HS_NUM_CLIENT; sIdx++)
      {
         tlCtxt->hoSupport.registeredInd[idx].triggerEvent[sIdx]    = WLANTL_HO_THRESHOLD_NA;
         tlCtxt->hoSupport.registeredInd[idx].crossCBFunction[sIdx] = NULL;
         tlCtxt->hoSupport.registeredInd[idx].usrCtxt[sIdx]         = NULL;
         tlCtxt->hoSupport.registeredInd[idx].whoIsClient[sIdx]     = 0;
      }
      tlCtxt->hoSupport.registeredInd[idx].rssiValue          = WLANTL_HO_DEFAULT_RSSI;
      tlCtxt->hoSupport.registeredInd[idx].numClient          = 0;
   }

   vos_timer_init(&tlCtxt->hoSupport.currentTraffic.trafficTimer,
                  VOS_TIMER_TYPE_SW,
                  WLANTL_HSTrafficStatusTimerExpired,
                  pAdapter);


   vos_lock_init(&tlCtxt->hoSupport.hosLock);
   tlCtxt->hoSupport.macCtxt = vos_get_context(VOS_MODULE_ID_SME, pAdapter);

   return status;
}

/*==========================================================================

   FUNCTION

   DESCRIPTION 
    
   PARAMETERS 

   RETURN VALUE

============================================================================*/
VOS_STATUS WLANTL_HSStop
(
   v_PVOID_t   pAdapter
)
{
   WLANTL_CbType   *tlCtxt = VOS_GET_TL_CB(pAdapter);
   VOS_STATUS       status = VOS_STATUS_SUCCESS;
   VOS_TIMER_STATE  timerState;

   if(NULL == tlCtxt)
   {
      TH_MSG_ERROR("Invalid TL handle", 0, 0, 0);
      return VOS_STATUS_E_INVAL;
   }

   timerState = vos_timer_getCurrentState(&tlCtxt->hoSupport.currentTraffic.trafficTimer);
   if(VOS_TIMER_STATE_RUNNING == timerState)
   {
      TH_MSG_INFO("Stop Traffic status monitoring timer", 0, 0, 0);
      status = vos_timer_stop(&tlCtxt->hoSupport.currentTraffic.trafficTimer);
   }
   if(VOS_STATUS_SUCCESS != status)
   {
      TH_MSG_ERROR("Timer Staop Fail Status %d", status, 0, 0);
   }
   
   return status;   
}
#endif //FEATURE_WLAN_GEN6_ROAMING
