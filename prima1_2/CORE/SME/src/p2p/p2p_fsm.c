/*
* Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

/*
 * Copyright (c) 2011 QUALCOMM Incorporated. All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 */

#if defined WLAN_FEATURE_P2P_INTERNAL

//#include "halInternal.h"
#include "smsDebug.h"
#include "sme_Api.h"
#include "csrInsideApi.h"
#include "smeInside.h"
#include "p2pFsm.h"
#include "p2p_ie.h"

 
char * TRIGGER_STR(int x)
{
   return ((x == eP2P_TRIGGER_SCAN_COMPLETE) ? "eP2P_TRIGGER_SCAN_COMPLETE" : \
                  (x == eP2P_TRIGGER_DEVICE_MODE_DEVICE) ? "eP2P_TRIGGER_DEVICE_MODE_DEVICE" : \
                  (x == eP2P_TRIGGER_DEVICE_MODE_GO) ? "eP2P_TRIGGER_DEVICE_MODE_GO" : \
                  (x == eP2P_TRIGGER_DEVICE_MODE_CLIENT) ? "eP2P_TRIGGER_DEVICE_MODE_CLIENT" : \
                  (x == eP2P_TRIGGER_LISTEN_COMPLETE) ? "eP2P_TRIGGER_LISTEN_COMPLETE" : \
                  (x == eP2P_TRIGGER_SEARCH_COMPLETE) ? "eP2P_TRIGGER_SEARCH_COMPLETE" : \
                  (x == eP2P_TRIGGER_GROUP_FORMATION) ? "eP2P_TRIGGER_GROUP_FORMATION" :\
                  (x == eP2P_TRIGGER_DISCONNECTED) ? "eP2P_TRIGGER_DISCONNECTED" : "UNKNOWN_TRIGGER");
}

char * STATE_STR(int y)    
{
   return ((y == eP2P_STATE_INVALID) ? "eP2P_STATE_INVALID" : \
                  (y == eP2P_STATE_DISCONNECTED) ? "eP2P_STATE_DISCONNECTED" :\
                  (y == eP2P_STATE_SCAN_INITIATED) ? "eP2P_STATE_SCAN_INITIATED" :\
                  (y == eP2P_STATE_FIND_LISTEN) ? "eP2P_STATE_FIND_LISTEN" :\
                  (y == eP2P_STATE_FIND_SEARCH) ? "eP2P_STATE_FIND_SEARCH" :\
                  (y == eP2P_STATE_GROUP_FORMATION) ? "eP2P_STATE_GROUP_FORMATION" :\
                  (y == eP2P_STATE_GO_ROLE) ? "eP2P_STATE_GO_ROLE" :\
                  (y == eP2P_STATE_CLIENT_ROLE) ? "eP2P_STATE_CLIENT_ROLE" : "UNKNOWN STATE");
}


#define __ENTER VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "P2P_FSM: %s, %d, state:%s, trigger:%s\n",\
                           __FUNCTION__, __LINE__, STATE_STR(p2pContext->state), TRIGGER_STR(trigger))



static eHalStatus p2pFsm_toGroupFormation(tHalHandle halHandle, tANI_U8 SessionID);
/*
  @breif Function called

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsm_toDisconnected(tp2pContext *p2pContext)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(p2pContext->hHal);
   tANI_U32 expire_time = 0;
   eHalStatus status;

   {
      int trigger = 22;
      __ENTER;
   }
   
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s call", __FUNCTION__);

   if ((p2pContext->pSentActionFrame != NULL) 
         || (p2pContext->pNextActionFrm != NULL))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, "%s Cannot enter disconnected state\n", __FUNCTION__);
      return eHAL_STATUS_SUCCESS;
   }

   p2pContext->state = eP2P_STATE_DISCONNECTED;
   p2pContext->GroupFormationPending = FALSE;
   p2pContext->PeerFound = FALSE;   
   p2pContext->currentSearchIndex = 0;

   if (p2pContext->p2pDiscoverCBFunc != NULL)
   {
      p2pFsm(p2pContext, eP2P_TRIGGER_DEVICE_MODE_DEVICE);
   }
   else if (p2pContext->listenDiscoverableState == eStateEnabled)
   {
      p2pContext->formationReq.targetListenChannel = 0;
      expire_time = p2pContext->expire_time;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s starting listenTimerHandler with expire %d\n", 
            __FUNCTION__, expire_time);

      status = palTimerStart(pMac->hHdd, p2pContext->listenTimerHandler, expire_time, eANI_BOOLEAN_FALSE);
      if (!HAL_STATUS_SUCCESS(status))
      {
         smsLog(pMac, LOGE, " %s fail to start listenTimerHandler\n", __FUNCTION__);
      }

   }

   return eHAL_STATUS_SUCCESS;
}


/*
  @breif Function called from P2P FSM during intiation of the
  Discovery procedure

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsmfromDisconnected(tp2pContext *p2pContext, tP2P_TRIGGER trigger)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   __ENTER;

   switch (trigger)
   {
   case eP2P_TRIGGER_DEVICE_MODE_DEVICE:
      status = p2pFsmScanRequest(p2pContext);
      if (status == eHAL_STATUS_SUCCESS)
         p2pContext->state = eP2P_STATE_SCAN_INITIATED;
      break;

   case eP2P_TRIGGER_GROUP_FORMATION:
      p2pContext->currentSearchIndex = 0;

      status = p2pFsmScanRequest(p2pContext);

      if ( status == eHAL_STATUS_SUCCESS )
         p2pContext->state = eP2P_STATE_SCAN_INITIATED;
      break;

   case eP2P_TRIGGER_DISCONNECTED:
   default :
         status = p2pFsm_toDisconnected( p2pContext );
      break;
   }

   return status;
}



/*
  @breif Function either calls the HDD callback if the Discover type
  is SCAN only and the scan is sucess or calls the listen state if the
  discover type is FIND only.

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsmfromScanRequest(tp2pContext *p2pContext,
             tP2P_TRIGGER trigger)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(p2pContext->hHal);

   eHalStatus status = eHAL_STATUS_SUCCESS;
   {
      __ENTER;
   }

   switch(trigger)
   {
      case eP2P_TRIGGER_SCAN_COMPLETE:
         if(p2pContext->GroupFormationPending)
         {
            status = p2pFsmListenRequest(p2pContext);
            if( status == eHAL_STATUS_SUCCESS )
            {
               p2pContext->state = eP2P_STATE_FIND_LISTEN;
            }
            else
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  " %s fail to enter listen for group formation",
                  __FUNCTION__);
            }
         }
         else if (p2pContext->discoverType == WFD_DISCOVER_TYPE_SCAN_ONLY
             || p2pContext->discoverType == WFD_DISCOVER_SCAN_ONLY_SOCIAL_CHN)
         {
            status = p2pFsmScanRequest(p2pContext);
            if (status == eHAL_STATUS_SUCCESS)
               p2pContext->state = eP2P_STATE_SCAN_INITIATED;
            break;
         }
         else if (p2pContext->discoverType == WFD_DISCOVER_TYPE_FIND_ONLY || 
                  p2pContext->discoverType == WFD_DISCOVER_TYPE_AUTO )
         {
            status = p2pFsmListenRequest(p2pContext);
            if( status == eHAL_STATUS_SUCCESS )
            {
               p2pContext->state = eP2P_STATE_FIND_LISTEN;
            }
            else
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  " %s fail to enter listen for discovery type %d",
                  __FUNCTION__, p2pContext->discoverType);
            }

         }
         break;

      case eP2P_TRIGGER_DISCONNECTED:
         status = p2pFsm_toDisconnected( p2pContext );
         break;

      case eP2P_TRIGGER_GROUP_FORMATION:
         status = palTimerStop(pMac->hHdd, p2pContext->discoverTimer);
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s Timer Stop status %d\n",  __FUNCTION__, status);
         p2pCallDiscoverCallback(p2pContext,  eP2P_DISCOVER_SUCCESS);
         p2pContext->currentSearchIndex = 0;

         status = p2pFsmScanRequest(p2pContext);

         if(status == eHAL_STATUS_SUCCESS)
            p2pContext->state = eP2P_STATE_SCAN_INITIATED;
         break;

      default:
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, 
                     "%s not handled trigger %d\n", __FUNCTION__, trigger);
         break;
   }

   return status;
}


/*
  @breif Callback function called from PE after scan complete

  @param[in] halHandle - Handle to MAC structure.
        [in] pContext - Pointer to the p2pContext structure
        [in] scanId -
    [in] scan_status - status of the scan request.

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
static eHalStatus p2pFsmScanRequestCallback(tHalHandle halHandle, void *pContext, tANI_U32 scanId, eCsrScanStatus scan_status)
{
   tp2pContext *p2pContext = (tp2pContext*) pContext;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT(halHandle);
   tCsrScanResultFilter filter;
   tScanResultHandle hScanResult = NULL;
   tCsrScanResultInfo *pScanResult;

   {
      int trigger = 22;
      __ENTER;
   }

   if(p2pContext->GroupFormationPending && scan_status == eCSR_SCAN_FOUND_PEER )
   {
      vos_mem_zero(&filter, sizeof(filter));
      filter.BSSIDs.numOfBSSIDs = 1;
      filter.BSSIDs.bssid = &p2pContext->peerMacAddress;
      filter.bWPSAssociation = TRUE;
      filter.BSSType = eCSR_BSS_TYPE_ANY;
      status = csrScanGetResult(pMac, &filter, &hScanResult);

      if (hScanResult)
      {
         pScanResult = csrScanResultGetFirst(pMac, hScanResult );
         if(pScanResult)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_USER, "%s found match on channel %d", 
               __FUNCTION__, pScanResult->BssDescriptor.channelId);
            p2pContext->formationReq.targetListenChannel = pScanResult->BssDescriptor.channelId;
            if(p2pContext->P2PListenChannel != pScanResult->BssDescriptor.channelId)
            {

               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_USER, 
                  "%s adapt listen channel to %d", 
                  __FUNCTION__, pScanResult->BssDescriptor.channelId);
               p2pSetListenChannel(pMac, p2pContext->sessionId, pScanResult->BssDescriptor.channelId);
            }
         }
         csrScanResultPurge(pMac, hScanResult);
      }
      else
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_USER, 
            "%s found_peer doesn't find match %02X-%02X-%02X-%02X-%02X-%02X", 
            __FUNCTION__, p2pContext->peerMacAddress[0], p2pContext->peerMacAddress[1],
            p2pContext->peerMacAddress[2], p2pContext->peerMacAddress[3],
            p2pContext->peerMacAddress[4], p2pContext->peerMacAddress[5]);
      }

      pMac->p2pContext[p2pContext->sessionId].PeerFound = TRUE;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s :Peer found\n", __FUNCTION__);
      status = p2pFsm_toGroupFormation(pMac, p2pContext->sessionId);
      return status;
   }
   else if(p2pContext->directedDiscovery && scan_status == eCSR_SCAN_FOUND_PEER )
   {
      status = p2pGetResultFilter(p2pContext, &filter);
      if(HAL_STATUS_SUCCESS(status))
      {
         status = csrScanGetResult(pMac, &filter, &hScanResult);
         if(hScanResult)
         {
            status = palTimerStop(pMac->hHdd, p2pContext->discoverTimer);
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s Timer Stop status %d\n",  __FUNCTION__, status);
            p2pCallDiscoverCallback(p2pContext,  eP2P_DIRECTED_DISCOVER);

            status = p2pFsm( p2pContext, eP2P_TRIGGER_DISCONNECTED );
            csrScanResultPurge(pMac, hScanResult);
         }
         else
         {
            //Not finding it. Make sure it fail
            status = eHAL_STATUS_FAILURE;
         }
      }
      if(!HAL_STATUS_SUCCESS(status))
      {
         //Let it continue
         p2pFsm(p2pContext, eP2P_TRIGGER_SCAN_COMPLETE);
      }
      return status;
   }
   else
   {
      p2pFsm(p2pContext, eP2P_TRIGGER_SCAN_COMPLETE);
   }

      return status;
}

#define MAX_CHANNEL_PER_GROUP 4

eHalStatus getChannelInfo(tp2pContext *p2pContext, 
                           tCsrChannelInfo *scanChannelInfo, ep2pDiscoverType discoverType)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT(p2pContext->hHal);
   tCsrChannelInfo ChannelInfo, SocialChannelInfo, ChannelInfo_2_4Ghz, ChannelInfo_5Ghz;
   tANI_U32 index = 0, numSocialChannel = 0, num_2_4GHz_Channel = 0, num_5GHz_Channel = 0;
   static tANI_U32 channIndex_2_4GHz = 0;
   static tANI_U32 channIndex_5GHz = 0;

   ChannelInfo.numOfChannels = WNI_CFG_VALID_CHANNEL_LIST_LEN;
   ChannelInfo.ChannelList = vos_mem_malloc(WNI_CFG_VALID_CHANNEL_LIST_LEN);
   if(NULL == ChannelInfo.ChannelList)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
         " %s fail to allocate memory", __FUNCTION__);
      ChannelInfo.numOfChannels = 0;
      return eHAL_STATUS_RESOURCES;
   }

   status = csrGetCfgValidChannels(pMac, ChannelInfo.ChannelList, &ChannelInfo.numOfChannels);
   if(status != eHAL_STATUS_SUCCESS)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
         " %s fail to allocate memory", __FUNCTION__);
      status = eHAL_STATUS_RESOURCES;
      goto cleanup;
   }

   for(index = 0; index < ChannelInfo.numOfChannels; index++ )
   {
      if ((ChannelInfo.ChannelList[index] == p2pContext->socialChannel[0])
            || (ChannelInfo.ChannelList[index] == p2pContext->socialChannel[1])
            || (ChannelInfo.ChannelList[index] == p2pContext->socialChannel[2]))
      {
         numSocialChannel++;
      }

      if ((ChannelInfo.ChannelList[index] >= SIR_11B_CHANNEL_BEGIN) &&
            (ChannelInfo.ChannelList[index] <= SIR_11B_CHANNEL_END))
      {
         num_2_4GHz_Channel++;
      }

      if ((ChannelInfo.ChannelList[index] >= SIR_11A_CHANNEL_BEGIN) &&
            (ChannelInfo.ChannelList[index] <= SIR_11A_CHANNEL_END))
      {
         num_5GHz_Channel++;
      }
   }

   SocialChannelInfo.ChannelList = (tANI_U8 *)vos_mem_malloc(numSocialChannel);
   if(NULL == SocialChannelInfo.ChannelList)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
         " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
      status = eHAL_STATUS_RESOURCES;
      goto cleanup;
   }
   SocialChannelInfo.numOfChannels = numSocialChannel;

   ChannelInfo_2_4Ghz.ChannelList = (tANI_U8 *)vos_mem_malloc(num_2_4GHz_Channel);
   if(NULL == ChannelInfo_2_4Ghz.ChannelList)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
         " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
      status = eHAL_STATUS_RESOURCES;
      goto cleanup;
   }
   ChannelInfo_2_4Ghz.numOfChannels = num_2_4GHz_Channel;

   ChannelInfo_5Ghz.ChannelList = (tANI_U8 *)vos_mem_malloc(num_5GHz_Channel);
   if(NULL == ChannelInfo_5Ghz.ChannelList)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
         " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
      status = eHAL_STATUS_RESOURCES;
      goto cleanup;
   }
   ChannelInfo_5Ghz.numOfChannels = num_5GHz_Channel;

   for(index = 0, numSocialChannel = 0, num_2_4GHz_Channel = 0, num_5GHz_Channel = 0; 
         index < ChannelInfo.numOfChannels; index++ )
   {
      if ((ChannelInfo.ChannelList[index] == p2pContext->socialChannel[0])
            || (ChannelInfo.ChannelList[index] == p2pContext->socialChannel[1])
            || (ChannelInfo.ChannelList[index] == p2pContext->socialChannel[2]))
      {
         SocialChannelInfo.ChannelList[numSocialChannel] = ChannelInfo.ChannelList[index];
         numSocialChannel++;
      }

      if ((ChannelInfo.ChannelList[index] >= SIR_11B_CHANNEL_BEGIN) &&
            (ChannelInfo.ChannelList[index] <= SIR_11B_CHANNEL_END))
      {
         ChannelInfo_2_4Ghz.ChannelList[num_2_4GHz_Channel] = ChannelInfo.ChannelList[index];
         num_2_4GHz_Channel++;
      }

      if ((ChannelInfo.ChannelList[index] >= SIR_11A_CHANNEL_BEGIN) &&
            (ChannelInfo.ChannelList[index] <= SIR_11A_CHANNEL_END))
      {
         ChannelInfo_5Ghz.ChannelList[num_5GHz_Channel] = ChannelInfo.ChannelList[index];
         num_5GHz_Channel++;
      }
   }

   switch(discoverType)
   {
   case WFD_DISCOVER_TYPE_SCAN_ONLY:
   case WFD_DISCOVER_TYPE_AUTO:
      if (p2pContext->currentSearchIndex == 0)
      {
         p2pContext->currentSearchIndex++;
         scanChannelInfo->ChannelList = vos_mem_malloc(SocialChannelInfo.numOfChannels);
         if(NULL == scanChannelInfo->ChannelList)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
               " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
            status = eHAL_STATUS_RESOURCES;
            goto cleanup;
         }

         vos_mem_copy(scanChannelInfo->ChannelList, SocialChannelInfo.ChannelList, SocialChannelInfo.numOfChannels);
         scanChannelInfo->numOfChannels = SocialChannelInfo.numOfChannels;
      }
      else if (p2pContext->currentSearchIndex == 1)
      {
         p2pContext->currentSearchIndex++;
         if (ChannelInfo_2_4Ghz.numOfChannels <= MAX_CHANNEL_PER_GROUP)
         {
            scanChannelInfo->ChannelList = vos_mem_malloc(ChannelInfo_2_4Ghz.numOfChannels);
            if(NULL == scanChannelInfo->ChannelList)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
               status = eHAL_STATUS_RESOURCES;
               goto cleanup;
            }
            vos_mem_copy(scanChannelInfo->ChannelList, SocialChannelInfo.ChannelList, SocialChannelInfo.numOfChannels);
            scanChannelInfo->numOfChannels = ChannelInfo_2_4Ghz.numOfChannels;
         }
         else
         {
            if (((channIndex_2_4GHz + MAX_CHANNEL_PER_GROUP) <= ChannelInfo_2_4Ghz.numOfChannels))
            {
               scanChannelInfo->ChannelList = vos_mem_malloc(MAX_CHANNEL_PER_GROUP);
               if(NULL == scanChannelInfo->ChannelList)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
                  status = eHAL_STATUS_RESOURCES;
                  goto cleanup;
               }
               vos_mem_copy(scanChannelInfo->ChannelList, ChannelInfo_2_4Ghz.ChannelList + channIndex_2_4GHz, MAX_CHANNEL_PER_GROUP);
               channIndex_2_4GHz += MAX_CHANNEL_PER_GROUP;
               scanChannelInfo->numOfChannels = MAX_CHANNEL_PER_GROUP;
               if (channIndex_2_4GHz == ChannelInfo_2_4Ghz.numOfChannels)
                  channIndex_2_4GHz = 0;
            }
            else
            {
               scanChannelInfo->ChannelList = vos_mem_malloc(ChannelInfo_2_4Ghz.numOfChannels - channIndex_2_4GHz);
               if(NULL == scanChannelInfo->ChannelList)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
                  status = eHAL_STATUS_RESOURCES;
                  goto cleanup;
               }
               vos_mem_copy(scanChannelInfo->ChannelList, ChannelInfo_2_4Ghz.ChannelList + channIndex_2_4GHz, 
                           ChannelInfo_2_4Ghz.numOfChannels - channIndex_2_4GHz);
               scanChannelInfo->numOfChannels = ChannelInfo_2_4Ghz.numOfChannels - channIndex_2_4GHz;
               channIndex_2_4GHz = 0;
            }
         }
      }
      else if (p2pContext->currentSearchIndex == 2)
      {
         p2pContext->currentSearchIndex++;
         scanChannelInfo->ChannelList = vos_mem_malloc(SocialChannelInfo.numOfChannels);
         if(NULL == scanChannelInfo->ChannelList)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
               " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
            status = eHAL_STATUS_RESOURCES;
            goto cleanup;
         }

         vos_mem_copy(scanChannelInfo->ChannelList, SocialChannelInfo.ChannelList, SocialChannelInfo.numOfChannels);
         scanChannelInfo->numOfChannels = SocialChannelInfo.numOfChannels;
      }
      else if (p2pContext->currentSearchIndex >= 3)
      {
         p2pContext->currentSearchIndex = 0;
         if (ChannelInfo_5Ghz.numOfChannels < MAX_CHANNEL_PER_GROUP)
         {
            scanChannelInfo->ChannelList = vos_mem_malloc(ChannelInfo_5Ghz.numOfChannels);
            if(NULL == scanChannelInfo->ChannelList)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                  " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
               status = eHAL_STATUS_RESOURCES;
               goto cleanup;
            }
            vos_mem_copy(scanChannelInfo->ChannelList, ChannelInfo_5Ghz.ChannelList, ChannelInfo_5Ghz.numOfChannels);
            scanChannelInfo->numOfChannels = ChannelInfo_5Ghz.numOfChannels;
         }
         else
         {
            if (((channIndex_5GHz + MAX_CHANNEL_PER_GROUP) <= ChannelInfo_5Ghz.numOfChannels))
            {
               scanChannelInfo->ChannelList = vos_mem_malloc(MAX_CHANNEL_PER_GROUP);
               if(NULL == scanChannelInfo->ChannelList)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
                  status = eHAL_STATUS_RESOURCES;
                  goto cleanup;
               }
               vos_mem_copy(scanChannelInfo->ChannelList, ChannelInfo_5Ghz.ChannelList + channIndex_5GHz, 4);
               channIndex_5GHz += MAX_CHANNEL_PER_GROUP;
               scanChannelInfo->numOfChannels = MAX_CHANNEL_PER_GROUP;
               if (channIndex_5GHz == ChannelInfo_5Ghz.numOfChannels)
                  channIndex_5GHz = 0;
            }
            else
            {
               scanChannelInfo->ChannelList = vos_mem_malloc(ChannelInfo_5Ghz.numOfChannels - channIndex_5GHz);
               if(NULL == scanChannelInfo->ChannelList)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
                     " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
                  status = eHAL_STATUS_RESOURCES;
                  goto cleanup;
               }
               vos_mem_copy(scanChannelInfo->ChannelList, ChannelInfo_5Ghz.ChannelList + channIndex_5GHz, 
                           ChannelInfo_5Ghz.numOfChannels - channIndex_5GHz);
               scanChannelInfo->numOfChannels = ChannelInfo_5Ghz.numOfChannels - channIndex_5GHz;
               channIndex_5GHz = 0;
               
            }
         }
      }
      break;

   case WFD_DISCOVER_TYPE_FIND_ONLY:
   case WFD_DISCOVER_SCAN_ONLY_SOCIAL_CHN:
      scanChannelInfo->ChannelList = vos_mem_malloc(SocialChannelInfo.numOfChannels);
      if(NULL == scanChannelInfo->ChannelList)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory %d", __FUNCTION__, __LINE__);
         status = eHAL_STATUS_RESOURCES;
         goto cleanup;
      }

      vos_mem_copy(scanChannelInfo->ChannelList, SocialChannelInfo.ChannelList, SocialChannelInfo.numOfChannels);
      scanChannelInfo->numOfChannels = SocialChannelInfo.numOfChannels;
      break;

   default:
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN," %s No match", __FUNCTION__);
      break;
   }

   if (ChannelInfo.ChannelList)
   {
      vos_mem_free(ChannelInfo.ChannelList);
      ChannelInfo.ChannelList = NULL;
   }

cleanup:
   SocialChannelInfo.numOfChannels = 0;
   if (SocialChannelInfo.ChannelList)
   {
      vos_mem_free(SocialChannelInfo.ChannelList);
      SocialChannelInfo.ChannelList = NULL;
   }

   ChannelInfo_2_4Ghz.numOfChannels = 0;
   if (ChannelInfo_2_4Ghz.ChannelList)
   {
      vos_mem_free(ChannelInfo_2_4Ghz.ChannelList);
      ChannelInfo_2_4Ghz.ChannelList = NULL;
   }

   ChannelInfo_5Ghz.numOfChannels = 0;
   if (ChannelInfo_5Ghz.ChannelList)
   {
      vos_mem_free(ChannelInfo_5Ghz.ChannelList);
      ChannelInfo_5Ghz.ChannelList = NULL;
   }

   ChannelInfo.numOfChannels = 0;
   if (ChannelInfo.ChannelList)
   {
      vos_mem_free(ChannelInfo.ChannelList);
      ChannelInfo.ChannelList = NULL;
   }

   return status;
}

/*
  @breif Gets the P2P IEs, forms the scan request and calls sme_ScanRequest.
  This API is called for Device discovery using scan only method.

  @param[in] p2pContext Pointer to the p2pContext structure

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsmScanRequest(tp2pContext *p2pContext)
{
   tCsrScanRequest scanRequest;
   v_U32_t scanId = 0;
   tANI_U32 len = 0;
   tCsrSSIDInfo wcSSID = { {P2P_WILDCARD_SSID_LEN, P2P_WILDCARD_SSID}, 0, 0 };
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT(p2pContext->hHal);
   tANI_U32 i = 0;

   {
      int trigger = 22;
      __ENTER;
   }

   vos_mem_zero( &scanRequest, sizeof(scanRequest));

   /* set the scan type to active */
   scanRequest.scanType = eSIR_ACTIVE_SCAN;

   vos_mem_set( scanRequest.bssid, sizeof( tCsrBssid ), 0xff );

   /* set min and max channel time to zero */
   scanRequest.minChnTime = 0;
   scanRequest.maxChnTime = 0;

   /* set BSSType to default type */
   scanRequest.BSSType = eCSR_BSS_TYPE_ANY;

   /*Scan all the channels */
   scanRequest.ChannelInfo.numOfChannels = 0;
   scanRequest.ChannelInfo.ChannelList = NULL;

   if( p2pContext->GroupFormationPending || (p2pContext->directedDiscovery == TRUE))
   {
      if (p2pContext->formationReq.targetListenChannel)
      {
         scanRequest.ChannelInfo.ChannelList = &p2pContext->formationReq.targetListenChannel;
         scanRequest.ChannelInfo.numOfChannels = 1;
      }
      else
      {
         getChannelInfo(p2pContext, &scanRequest.ChannelInfo, WFD_DISCOVER_TYPE_AUTO);
      }
      vos_mem_copy( scanRequest.bssid, p2pContext->formationReq.deviceAddress, sizeof( tCsrBssid ) );
      scanRequest.requestType = eCSR_SCAN_P2P_FIND_PEER;
   }
   else
   {
      getChannelInfo(p2pContext, &scanRequest.ChannelInfo, p2pContext->discoverType);
      scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
   }

   P2P_GetIE(p2pContext, p2pContext->sessionId, eP2P_PROBE_REQ,  &scanRequest.pIEField, &len);

   scanRequest.uIEFieldLen = len;

   scanRequest.SSIDs.numOfSSIDs = 1;
   scanRequest.SSIDs.SSIDList = &wcSSID;
   scanRequest.p2pSearch = VOS_TRUE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s  mac address %x:%x:%x:%x:%x:%x\n", __FUNCTION__,
               scanRequest.bssid[0], scanRequest.bssid[1], scanRequest.bssid[2], 
               scanRequest.bssid[3], scanRequest.bssid[4], scanRequest.bssid[5]);

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,"%s on channel", __FUNCTION__);
   for (i = 0; i < scanRequest.ChannelInfo.numOfChannels; i++)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO,"%d", scanRequest.ChannelInfo.ChannelList[i]);
   }

   status = csrScanRequest( pMac, p2pContext->SMEsessionId, &scanRequest, &scanId, &p2pFsmScanRequestCallback, p2pContext );

   return status;
}


/*
  @breif Starts the search search state of the find phase in P2p
  Discovery.

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsmfromFindListen(tp2pContext *p2pContext, tP2P_TRIGGER trigger)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   tpAniSirGlobal pMac = PMAC_STRUCT(p2pContext->hHal);
   {
      __ENTER;
   }
   switch(trigger)
   {
      case eP2P_TRIGGER_LISTEN_COMPLETE:
         status = p2pFsmScanRequest(p2pContext);

         if(status == eHAL_STATUS_SUCCESS)
            p2pContext->state = eP2P_STATE_SCAN_INITIATED;
         break;

      case eP2P_TRIGGER_DISCONNECTED:
         status = p2pFsm_toDisconnected( p2pContext );
         break;

      case eP2P_TRIGGER_GROUP_FORMATION:
         status = palTimerStop(pMac->hHdd, p2pContext->discoverTimer);
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s calling p2pDiscoverCompleteCallback\n", __FUNCTION__);
         p2pCallDiscoverCallback(p2pContext,  eP2P_DISCOVER_SUCCESS);

         status = p2pFsmScanRequest(p2pContext);

         if(status == eHAL_STATUS_SUCCESS)
            p2pContext->state = eP2P_STATE_SCAN_INITIATED;
         break;

      default:
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, 
                     "%s not handled trigger %d\n", __FUNCTION__, trigger);
         break;
   }
    
   return status;
}


/*
  @breif Callback function called from PE once the listen (remain on
  channel) is completed.

  This function either calls the HDD callback if during listen
  phase we found any peer P2P device which sent a probe request only
  and the scan is sucess or calls the FSM to indicate listen
  completion and move on the search state.

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
static eHalStatus p2pFsmListenRequestCallback(tHalHandle halHandle, void *pContext, tANI_U32 scanId, eCsrScanStatus scan_status)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *p2pContext = (tp2pContext*) pContext;
   {
      int trigger = 22;
      __ENTER;
   }
   p2pFsm(p2pContext, eP2P_TRIGGER_LISTEN_COMPLETE);

   return eHAL_STATUS_SUCCESS;
}


tANI_U8 p2pFsm_getListenChannel( tp2pContext *p2pContext )
{
   return p2pContext->socialChannel[p2pContext->listenIndex];
}


tANI_U16 p2p_getListenInterval( tp2pContext *p2pContext )
{
   tANI_U32 time_tick;
   tANI_U8 min, max;
   tANI_U8 listenDuration = 1; //TODO:random number between min and max interval.

   time_tick = vos_timer_get_system_ticks();
   min = 1;
   max = 3; //TODO : Define and get from configuration.

   listenDuration = ((tANI_U8)time_tick) % (max - min + 1) + min;

   return (listenDuration * 120);
}


/*
  @breif Performs the Listen state of the Find phase.

  @param[in] p2pContext Pointer to the p2pContext structure

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsmListenRequest(tp2pContext *p2pContext)
{
   tANI_U32 listenDuration;
   tANI_U8 listenChannel;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   v_U32_t scanId = 0;
   tANI_U32 len;
   {
      int trigger = 22;
      __ENTER;
   }
   /*random between min and max*/
   listenDuration = p2p_getListenInterval(p2pContext);

   /*random at the begining of discovery*/
   listenChannel = p2pContext->P2PListenChannel;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s Calling p2pRemainOnChannel with duration %d on channel %d\n",
                           __FUNCTION__, listenDuration, listenChannel);
   status = p2pRemainOnChannel( p2pContext->hHal, p2pContext->SMEsessionId, listenChannel, listenDuration, 
                                &p2pFsmListenRequestCallback, p2pContext, eP2PRemainOnChnReasonDiscovery);
   return status;
}


eHalStatus p2pFsm_toGroupFormation(tHalHandle halHandle, tANI_U8 SessionID)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_U8 *formation_channel;
   tpAniSirGlobal pMac = PMAC_STRUCT(halHandle);
   {
      int trigger = 22;
      tp2pContext *p2pContext = &pMac->p2pContext[SessionID];
      __ENTER;
   }
   if (pMac->p2pContext[SessionID].formationReq.targetListenChannel)
   {
      formation_channel = &pMac->p2pContext[SessionID].formationReq.targetListenChannel;
   }
   else
   {
      formation_channel = p2pFsm_GetNextSearchChannel(&pMac->p2pContext[SessionID]);
   }
    
   pMac->p2pContext[SessionID].state = eP2P_STATE_GROUP_FORMATION;
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO, "%s Calling p2pRemainOnChannel with duration %d on channel %d\n", 
               __FUNCTION__, P2P_REMAIN_ON_CHAN_TIMEOUT, *formation_channel);
   status = p2pRemainOnChannel( pMac, pMac->p2pContext[SessionID].SMEsessionId, *formation_channel, P2P_REMAIN_ON_CHAN_TIMEOUT,
                                 p2pGrpFormationRemainOnChanRspCallback, 
                                 &pMac->p2pContext[SessionID], eP2PRemainOnChnReasonSendFrame);
   if(status != eHAL_STATUS_SUCCESS)
   {
      smsLog( pMac, LOGE, "%s remain on channel failed\n", __FUNCTION__);
   }

   return status; 
}

tANI_U8* p2pFsm_GetNextSearchChannel(tp2pContext *p2pContext)
{
   tANI_U8 searchIndex = p2pContext->currentSearchIndex;

   return &p2pContext->socialChannel[searchIndex];
}

eHalStatus p2pFsm_fromGroupFormation(tp2pContext *p2pContext, tP2P_TRIGGER trigger)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   {
      __ENTER;
   }
   switch(trigger)
   {
      case eP2P_TRIGGER_DISCONNECTED:
         status = p2pFsm_toDisconnected( p2pContext );
         break;

      case eP2P_TRIGGER_GROUP_FORMATION:

         status = p2pFsmScanRequest(p2pContext);

         if(status == eHAL_STATUS_SUCCESS)
            p2pContext->state = eP2P_STATE_SCAN_INITIATED;
         break;

      default:
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, "%s not handling trigger %d\n", __FUNCTION__, trigger);
         break;         
   }
   return status; 
}


/*
  @breif Wifi Direct state machine

  @param[in] p2pContext Pointer to the p2pContext structure
        [in] trigger

  @return eHAL_STATUS_FAILURE - If success.
          eHAL_STATUS_SUCCESS - If failure.
*/
eHalStatus p2pFsm(tp2pContext *p2pContext, tP2P_TRIGGER trigger)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;    
   {
      __ENTER;
   }

   switch(p2pContext->state)
   {
      case eP2P_STATE_DISCONNECTED:
         status = p2pFsmfromDisconnected(p2pContext, trigger);
         break;

      case eP2P_STATE_SCAN_INITIATED:
         status = p2pFsmfromScanRequest(p2pContext, trigger);
         break;

      case eP2P_STATE_FIND_LISTEN:
         status = p2pFsmfromFindListen(p2pContext, trigger);
         break;

      case eP2P_STATE_GROUP_FORMATION:
         status = p2pFsm_fromGroupFormation(p2pContext, trigger);
         break;

      default:
          VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, "%s not handled state %d\n", __FUNCTION__, p2pContext->state);
         break;
   }

   return status;
}
#endif
