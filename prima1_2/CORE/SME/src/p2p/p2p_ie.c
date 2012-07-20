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

/* Pointer to Global P2P IE structut*/
tp2pie gP2PIe;

eHalStatus p2pUpdateDeviceCapAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID, tp2p_device_capability_config *devcap)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL || devcap == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   /* Initialize to zero*/
   gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability = 0;

   if (devcap->bServiceDiscoveryEnabled)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_SERVICE_DISCOVERY;

   if (devcap->bClientDiscoverabilityEnabled)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_CLIENT_DISCOVERABILITY;

   if (devcap->bConcurrentOperationSupported)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_CONCURRENT_OPERATION;

   if (devcap->bInfrastructureManagementEnabled)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_INFRASTRUCTURE_MANAGEMENT;

   if (devcap->bDeviceLimitReached)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_DEVICE_LIMIT_REACHED;

   if (devcap->bInvitationProcedureEnabled)
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= WFD_DEVICE_CAPABILITY_INVITATION_PROCEDURE;

   /*TODO:Dont know what to do with WPSVesionEnabled*/

   return status;
}


eHalStatus p2pUpdateListenChannelAttrib(tHalHandle hHal, tANI_U8 SessionID, tP2P_ListenChannel *p2pListenChannel)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   gP2PIe[SessionID].p2pListenChannelAttrib.channel = p2pListenChannel->channel;   
   gP2PIe[SessionID].p2pListenChannelAttrib.regulatoryClass = 0x51;

   vos_mem_copy(gP2PIe[SessionID].p2pListenChannelAttrib.countryString, 
      &p2pListenChannel->countryString, sizeof(p2pListenChannel->countryString));

   return status;
}


eHalStatus p2pGetListenChannelAttrib(tHalHandle hHal, tANI_U8 SessionID, tP2P_ListenChannel *p2pListenChannel)
{

   vos_mem_copy(&gP2PIe[SessionID].p2pListenChannelAttrib, 
      p2pListenChannel, sizeof(p2pListenChannel));

   return eHAL_STATUS_SUCCESS;
}


eHalStatus p2pUpdateOperatingChannelAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_OperatingChannel *p2pOperatingChannel)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   /*TODO: Fille the country code and channel correctly*/
   vos_mem_copy(gP2PIe[SessionID].p2pOperatingChannelAttrib.countryString, 
      &p2pOperatingChannel->countryString, sizeof(p2pOperatingChannel->countryString));

   gP2PIe[SessionID].p2pOperatingChannelAttrib.regulatoryClass = p2pOperatingChannel->regulatoryClass;
   gP2PIe[SessionID].p2pOperatingChannelAttrib.channel = p2pOperatingChannel->channel;

   return status;
}


eHalStatus p2pUpdateChannelListAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tANI_U8      channelList[] = {0x51, 0x0B, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};

   if (pMac == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   /*TODO: Fill the country code and channel correctly*/
   gP2PIe[SessionID].p2pChannelList.countryString[0] = 0x55;
   gP2PIe[SessionID].p2pChannelList.countryString[1] = 0x53;
   gP2PIe[SessionID].p2pChannelList.countryString[2] = 0x04;

   gP2PIe[SessionID].p2pChannelList.num_channelList = 0x0D;
   vos_mem_copy(gP2PIe[SessionID].p2pChannelList.channelList, channelList, gP2PIe[SessionID].p2pChannelList.num_channelList);

   return status;
}

eHalStatus p2pUpdateExtendedListenTimingAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   gP2PIe[SessionID].p2pExtendedListenTimingAttrib.availibilityInterval = pMac->p2pContext[SessionID].listenDuration;
   gP2PIe[SessionID].p2pExtendedListenTimingAttrib.availibilityPeriod = pMac->p2pContext[SessionID].expire_time;

   return status;
}


eHalStatus p2pUpdateGONegoReq(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_go_request *p2pGONegoReq)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pGONegoReq == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pGONegoReq->IEdata)
   {
      if(pContext->GoNegoReqIeField)
      {
         vos_mem_free(pContext->GoNegoReqIeField);
      }
      pContext->GoNegoReqIeField = (tANI_U8 *)vos_mem_malloc(p2pGONegoReq->uIELength);
      if(NULL == pContext->GoNegoReqIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->GoNegoReqIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->GoNegoReqIeField, p2pGONegoReq->IEdata, p2pGONegoReq->uIELength);
      pContext->GoNegoReqIeLength = p2pGONegoReq->uIELength;
   }

   vos_mem_copy(gP2PIe[SessionID].p2pIntendedInterfaceAddr.P2PInterfaceAddress, 
                p2pGONegoReq->IntendedInterfaceAddress, P2P_MAC_ADDRESS_LEN);
   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pGONegoReq->peerDeviceAddress, P2P_MAC_ADDRESS_LEN);

   pContext->dialogToken = p2pGONegoReq->dialogToken;

   gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability = p2pGONegoReq->GroupCapability;
   gP2PIe[SessionID].p2pGroupOwnerIntent.GOIntent = p2pGONegoReq->GoIntent;
   gP2PIe[SessionID].p2pConfTimeout.GOConfigTimeout = p2pGONegoReq->GoTimeout;
   gP2PIe[SessionID].p2pConfTimeout.CLConfigTimeout = p2pGONegoReq->ClientTimeout;

   return status;
}


eHalStatus p2pUpdateGONegoRes(tpAniSirGlobal pMac,tANI_U8 SessionID, tP2P_go_response *p2pGONegoRes)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pGONegoRes == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pGONegoRes->IEdata)
   {
      if(pContext->GoNegoResIeField)
      {
         vos_mem_free(pContext->GoNegoResIeField);
      }
      pContext->GoNegoResIeField = (tANI_U8 *)vos_mem_malloc(p2pGONegoRes->uIELength);
      if(NULL == pContext->GoNegoResIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->GoNegoResIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->GoNegoResIeField, p2pGONegoRes->IEdata, p2pGONegoRes->uIELength);
      pContext->GoNegoResIeLength = p2pGONegoRes->uIELength;
   }

   gP2PIe[SessionID].p2pStatus.status = p2pGONegoRes->status;
   gP2PIe[SessionID].p2pConfTimeout.GOConfigTimeout = p2pGONegoRes->GoTimeout;
   gP2PIe[SessionID].p2pConfTimeout.CLConfigTimeout = p2pGONegoRes->ClientTimeout;
   pMac->p2pContext[SessionID].receivedDialogToken = p2pGONegoRes->dialog_token;
   gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability = p2pGONegoRes->GroupCapability;

   gP2PIe[SessionID].p2pGroupOwnerIntent.GOIntent = p2pGONegoRes->GoIntent;
   vos_mem_copy(gP2PIe[SessionID].p2pIntendedInterfaceAddr.P2PInterfaceAddress, 
                p2pGONegoRes->IntendedInterfaceAddress, P2P_MAC_ADDRESS_LEN);

   if (p2pGONegoRes->bUsedGroupId)
   {
      gP2PIe[SessionID].p2pGroupID.present = TRUE;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.deviceAddress, p2pGONegoRes->GroupId.deviceAddress, sizeof(p2pGONegoRes->GroupId.deviceAddress));
      gP2PIe[SessionID].p2pGroupID.num_ssid = p2pGONegoRes->GroupId.num_ssid;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.ssid, p2pGONegoRes->GroupId.ssid, sizeof(p2pGONegoRes->GroupId.ssid));
   }
   else
   {
      gP2PIe[SessionID].p2pGroupID.present = FALSE;
   }

   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pGONegoRes->peerDeviceAddress, P2P_MAC_ADDRESS_LEN);

   return status;
}


eHalStatus p2pUpdateGONegoCnf(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_go_confirm *p2pGONegoCnf)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pGONegoCnf == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pGONegoCnf->IEdata)
   {
      if(pContext->GoNegoCnfIeField)
      {
         vos_mem_free(pContext->GoNegoCnfIeField);
      }
      pContext->GoNegoCnfIeField = (tANI_U8 *)vos_mem_malloc(p2pGONegoCnf->uIELength);
      if(NULL == pContext->GoNegoCnfIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->GoNegoCnfIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->GoNegoCnfIeField, p2pGONegoCnf->IEdata, p2pGONegoCnf->uIELength);
      pContext->GoNegoCnfIeLength = p2pGONegoCnf->uIELength;
   }

   gP2PIe[SessionID].p2pStatus.status = p2pGONegoCnf->status;
   pContext->receivedDialogToken = p2pGONegoCnf->dialog_token;
   gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability = p2pGONegoCnf->GroupCapability;

   if (p2pGONegoCnf->bUsedGroupId)
   {
      gP2PIe[SessionID].p2pGroupID.present = TRUE;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.deviceAddress, p2pGONegoCnf->GroupId.deviceAddress, sizeof(p2pGONegoCnf->GroupId.deviceAddress));
      gP2PIe[SessionID].p2pGroupID.num_ssid = p2pGONegoCnf->GroupId.num_ssid;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.ssid, p2pGONegoCnf->GroupId.ssid, sizeof(p2pGONegoCnf->GroupId.ssid));
   }
   else
   {
      gP2PIe[SessionID].p2pGroupID.present = FALSE;
   }

   return status;
}


eHalStatus p2pGetSSID(tANI_U8 *ssId, tANI_U32 *ssIdLen, tANI_U8 SessionID)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   
   vos_mem_copy(ssId, &gP2PIe[SessionID].p2pGroupID.ssid, sizeof(gP2PIe[SessionID].p2pGroupID.ssid));
   *ssIdLen = gP2PIe[SessionID].p2pGroupID.num_ssid;

   return status;
}


eHalStatus p2pUpdateProvisionDiscoveryReq(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_ProvDiscoveryReq *p2pProvisonDiscoverReq)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pProvisonDiscoverReq == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pProvisonDiscoverReq->IEdata)
   {
      if(pContext->ProvDiscReqIeField)
      {
         vos_mem_free(pContext->ProvDiscReqIeField);
         pContext->ProvDiscReqIeField = NULL;
      }
      pContext->ProvDiscReqIeField = (tANI_U8 *)vos_mem_malloc(p2pProvisonDiscoverReq->uIELength);
      if(NULL == pContext->ProvDiscReqIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->ProvDiscReqIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->ProvDiscReqIeField, p2pProvisonDiscoverReq->IEdata, p2pProvisonDiscoverReq->uIELength);
      pContext->ProvDiscReqIeLength = p2pProvisonDiscoverReq->uIELength;
   }

   if (p2pProvisonDiscoverReq->bUseGroupID)
   {
      gP2PIe[SessionID].p2pGroupID.present = TRUE;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.deviceAddress, 
                   p2pProvisonDiscoverReq->GroupId.deviceAddress, 
                   sizeof(p2pProvisonDiscoverReq->GroupId.deviceAddress));
      gP2PIe[SessionID].p2pGroupID.num_ssid = p2pProvisonDiscoverReq->GroupId.num_ssid;
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupID.ssid, 
                   p2pProvisonDiscoverReq->GroupId.ssid, 
                   sizeof(p2pProvisonDiscoverReq->GroupId.ssid));
   }
   else
   {
      gP2PIe[SessionID].p2pGroupID.present = FALSE;
   }

   gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability = p2pProvisonDiscoverReq->GroupCapability;

   pContext->dialogToken = p2pProvisonDiscoverReq->dialogToken;

   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pProvisonDiscoverReq->PeerDeviceAddress, P2P_MAC_ADDRESS_LEN);

   return status;
}


eHalStatus p2pUpdateProvisionDiscoveryRes(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_ProvDiscoveryRes *p2pProvisonDiscoverRes)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pProvisonDiscoverRes == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pProvisonDiscoverRes->IEdata)
   {
      if(pContext->ProvDiscResIeField)
      {
         vos_mem_free(pContext->ProvDiscResIeField);
         pContext->ProvDiscResIeField = NULL;
      }
      pContext->ProvDiscResIeField = (tANI_U8 *)vos_mem_malloc(p2pProvisonDiscoverRes->uIELength);
      if(NULL == pContext->ProvDiscResIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->ProvDiscResIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->ProvDiscResIeField, p2pProvisonDiscoverRes->IEdata, p2pProvisonDiscoverRes->uIELength);
      pContext->ProvDiscResIeLength = p2pProvisonDiscoverRes->uIELength;
   }

   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pProvisonDiscoverRes->ReceiverDeviceAddress, P2P_MAC_ADDRESS_LEN);

   pContext->receivedDialogToken = p2pProvisonDiscoverRes->dialogToken;

   return status;
}


eHalStatus p2pUpdateInvitationReq(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_invitation_request *p2pInvitationReq)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pInvitationReq == NULL) 
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pInvitationReq->IEdata) 
   {
      if(pContext->InvitationReqIeField)
      {
         vos_mem_free(pContext->InvitationReqIeField);
         pContext->InvitationReqIeField = NULL;
      }
      pContext->InvitationReqIeField = (tANI_U8 *)vos_mem_malloc(p2pInvitationReq->uIELength);
      if(NULL == pContext->InvitationReqIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->InvitationReqIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->InvitationReqIeField, p2pInvitationReq->IEdata, p2pInvitationReq->uIELength);
      pContext->InvitationReqIeLength = p2pInvitationReq->uIELength;
   }

   pContext->dialogToken = p2pInvitationReq->DialogToken;
   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pInvitationReq->PeerDeviceAddress, P2P_MAC_ADDRESS_LEN);
   gP2PIe[SessionID].p2pConfTimeout.GOConfigTimeout = p2pInvitationReq->GoTimeout;
   gP2PIe[SessionID].p2pConfTimeout.CLConfigTimeout = p2pInvitationReq->ClientTimeout;
   gP2PIe[SessionID].p2pInvitationReq.InvitationFlags.invitationFlags = p2pInvitationReq->InvitationFlags;
   vos_mem_copy(&gP2PIe[SessionID].p2pGroupBssid, 
                &p2pInvitationReq->GroupBSSID, 
                sizeof(p2pInvitationReq->GroupBSSID));
   vos_mem_copy(&gP2PIe[SessionID].p2pOperatingChannelAttrib, 
                &p2pInvitationReq->OperatingChannel, 
                sizeof(p2pInvitationReq->OperatingChannel));
   vos_mem_copy(&gP2PIe[SessionID].p2pGroupID, 
                &p2pInvitationReq->GroupID, sizeof(p2pInvitationReq->GroupID));

   return status;
}


eHalStatus p2pUpdateInvitationRes(tpAniSirGlobal pMac, tANI_U8 SessionID, tP2P_invitation_response *p2pInvitationRes)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || p2pInvitationRes == NULL) 
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   if (p2pInvitationRes->IEdata) 
   {
      if(pContext->InvitationResIeField)
      {
         vos_mem_free(pContext->InvitationResIeField);
      }
      pContext->InvitationResIeField = (tANI_U8 *)vos_mem_malloc(p2pInvitationRes->uIELength);
      if(NULL == pContext->InvitationResIeField)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
         pContext->InvitationResIeLength = 0;
         return eHAL_STATUS_RESOURCES;
      }
      vos_mem_copy((tANI_U8 *)pContext->InvitationResIeField, 
                        p2pInvitationRes->IEdata, p2pInvitationRes->uIELength);
      pContext->InvitationResIeLength = p2pInvitationRes->uIELength;
   }

   vos_mem_copy((tANI_U8 *)pContext->ReceiverDeviceAddress, p2pInvitationRes->ReceiverDeviceAddress, P2P_MAC_ADDRESS_LEN);
   vos_mem_copy((tANI_U8 *)pContext->peerMacAddress, p2pInvitationRes->ReceiverDeviceAddress, P2P_MAC_ADDRESS_LEN);
   pContext->receivedDialogToken = p2pInvitationRes->DialogToken;

   gP2PIe[SessionID].p2pStatus.status = p2pInvitationRes->status;
   gP2PIe[SessionID].p2pConfTimeout.GOConfigTimeout = p2pInvitationRes->GoTimeout;
   gP2PIe[SessionID].p2pConfTimeout.CLConfigTimeout = p2pInvitationRes->ClientTimeout;

   if (p2pInvitationRes->GroupBSSID.present == TRUE) 
   {
      vos_mem_copy(&gP2PIe[SessionID].p2pGroupBssid, &p2pInvitationRes->GroupBSSID, sizeof(p2pInvitationRes->GroupBSSID));
   } 
   else {
      gP2PIe[SessionID].p2pGroupBssid.present = FALSE;
   }

   if (p2pInvitationRes->OperatingChannel.present == TRUE) 
   {
      gP2PIe[SessionID].p2pOperatingChannelAttrib.present = TRUE;
      vos_mem_copy(gP2PIe[SessionID].p2pOperatingChannelAttrib.countryString, 
                  &p2pInvitationRes->OperatingChannel.countryString, sizeof(p2pInvitationRes->OperatingChannel.countryString));
      gP2PIe[SessionID].p2pOperatingChannelAttrib.regulatoryClass = p2pInvitationRes->OperatingChannel.regulatoryClass;
      gP2PIe[SessionID].p2pOperatingChannelAttrib.channel = p2pInvitationRes->OperatingChannel.channel;
   } 
   else { 
      gP2PIe[SessionID].p2pOperatingChannelAttrib.present = FALSE;
   }

   return status;
}


eHalStatus p2pUpdateAdditonalIE(tpAniSirGlobal pMac, tANI_U8 SessionID, tp2p_additional_ie *additionalIE)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2p_additional_ie *pAddnIe = &gP2PIe[SessionID].p2pAdditionalIE;

   if (pMac == NULL || additionalIE == NULL)
   {
      return eHAL_STATUS_FAILURE;
   }

   pAddnIe->uBeaconIEsLength = additionalIE->uBeaconIEsLength;

   if ( pAddnIe->pBeaconIe )
   {
      vos_mem_free(pAddnIe->pBeaconIe);
   }

   pAddnIe->pBeaconIe = (v_U8_t *)vos_mem_malloc(pAddnIe->uBeaconIEsLength);
   if(NULL == pAddnIe->pBeaconIe)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      pAddnIe->uBeaconIEsLength = 0;
      return eHAL_STATUS_RESOURCES;
   }
   vos_mem_copy(pAddnIe->pBeaconIe, additionalIE->pBeaconIe, pAddnIe->uBeaconIEsLength);

   pAddnIe->uDefaultRequestIEsLength = additionalIE->uDefaultRequestIEsLength;
   if ( pAddnIe->pDefaultRequestIe )
   {
      vos_mem_free(pAddnIe->pDefaultRequestIe);
   }

   pAddnIe->pDefaultRequestIe = (v_U8_t *)vos_mem_malloc(pAddnIe->uDefaultRequestIEsLength);
   if(NULL == pAddnIe->pDefaultRequestIe)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      pAddnIe->uDefaultRequestIEsLength = 0;
      return eHAL_STATUS_RESOURCES;
   }

   vos_mem_copy(pAddnIe->pDefaultRequestIe, additionalIE->pDefaultRequestIe, pAddnIe->uDefaultRequestIEsLength);

   pAddnIe->uProbeResponseIEsLength = additionalIE->uProbeResponseIEsLength;
   if ( pAddnIe->pProbeResponseIe )
      vos_mem_free(pAddnIe->pProbeResponseIe);

   pAddnIe->pProbeResponseIe = (v_U8_t *)vos_mem_malloc(pAddnIe->uProbeResponseIEsLength);
   if(NULL == pAddnIe->pProbeResponseIe)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      pAddnIe->uProbeResponseIEsLength = 0;
      return eHAL_STATUS_RESOURCES;
   }
   vos_mem_copy(pAddnIe->pProbeResponseIe, additionalIE->pProbeResponseIe, pAddnIe->uProbeResponseIEsLength);

   return status;
}


eHalStatus p2pUpdateGroupID(tpAniSirGlobal pMac, tANI_U8 SessionID, tDot11fTLVP2PGroupId *groupId)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL || groupId == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   gP2PIe[SessionID].p2pGroupID = *groupId;
}


eHalStatus p2pUpdateSecondaryDevTypeList(tpAniSirGlobal pMac, tANI_U8 SessionID, tp2p_secondary_device_type_list *SecondaryDeviceType)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL || SecondaryDeviceType == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   gP2PIe[SessionID].p2pSecondaryDevTypeList.uNumOfEntries = SecondaryDeviceType->uNumOfEntries;
   gP2PIe[SessionID].p2pSecondaryDevTypeList.uTotalNumOfEntries = SecondaryDeviceType->uTotalNumOfEntries;

   if ( gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes )
   {
      vos_mem_free(gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes);
      gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes = NULL;
   }

   if(SecondaryDeviceType->uNumOfEntries)
   {
      gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes = 
         (tp2p_device_type *) vos_mem_malloc (SecondaryDeviceType->uNumOfEntries * sizeof (tp2p_device_type));
      if(NULL == gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
               " %s fail to allocate memory", __FUNCTION__);
         gP2PIe[SessionID].p2pSecondaryDevTypeList.uNumOfEntries  = 0;
         gP2PIe[SessionID].p2pSecondaryDevTypeList.uTotalNumOfEntries = 0;
         return eHAL_STATUS_RESOURCES;
      }

      vos_mem_copy (gP2PIe[SessionID].p2pSecondaryDevTypeList.SecondaryDeviceTypes, 
         SecondaryDeviceType->SecondaryDeviceTypes, 
         SecondaryDeviceType->uNumOfEntries * sizeof(tp2p_device_type));
   }

   // TODO: Free the memory allocated while unloading the module or delete_mac for p2p device is called.

   return status;
}


eHalStatus p2pUpdateDeviceAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID, tDot11fTLVP2PDeviceInfo *deviceInfo)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL || deviceInfo == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   gP2PIe[SessionID].p2pDeviceInfoAttrib.configMethod = deviceInfo->configMethod;
   vos_mem_copy(gP2PIe[SessionID].p2pDeviceInfoAttrib.primaryDeviceType,deviceInfo->primaryDeviceType, MAX_DEVICE_TYPE_LEN);

   if (deviceInfo->P2PDeviceAddress)
   {
      vos_mem_copy(&gP2PIe[SessionID].p2pDeviceInfoAttrib.P2PDeviceAddress, deviceInfo->P2PDeviceAddress, P2P_MAC_ADDRESS_LEN);
      vos_mem_copy(&gP2PIe[SessionID].p2pDeviceIdAttrib.P2PDeviceAddress, deviceInfo->P2PDeviceAddress, P2P_MAC_ADDRESS_LEN);
      vos_mem_copy(pMac->p2pContext[SessionID].selfMacAddress, deviceInfo->P2PDeviceAddress, P2P_MAC_ADDRESS_LEN);
   }

   gP2PIe[SessionID].p2pDeviceInfoAttrib.DeviceName.num_text = deviceInfo->DeviceName.num_text;

   if (deviceInfo->DeviceName.text)
   {
      vos_mem_copy(gP2PIe[SessionID].p2pDeviceInfoAttrib.DeviceName.text, deviceInfo->DeviceName.text, P2P_DEVICE_NAME_MAX_LENGTH);
   }

   return status;
}


eHalStatus P2P_SetOperationMode(tHalHandle hHal, tANI_U8 SessionID, ep2pOperatingMode val)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

   pMac->p2pContext[SessionID].operatingmode = val;

   return eHAL_STATUS_SUCCESS;
}


ep2pOperatingMode P2P_GetOperationMode(tHalHandle hHal, tANI_U8 SessionID)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

   return pMac->p2pContext[SessionID].operatingmode;
}


eHalStatus  P2P_UpdateGroupCapability(tHalHandle pMac, tANI_U8 SessionID, tANI_U8 GroupCapability, tANI_U8 val)
{
   if (val)
   {
      gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability |= GroupCapability;
   }
   else
   {
      gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability &= ~(GroupCapability);
   }
}

eHalStatus  P2P_UpdateDeviceCapability(tHalHandle pMac, tANI_U8 SessionID, tANI_U8 DeviceCapability, tANI_U8 val)
{
   if (val)
   {
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability |= DeviceCapability;
   }
   else
   {
      gP2PIe[SessionID].p2pCapabilityAttrib.deviceCapability &= ~(DeviceCapability);
   }
}


eHalStatus p2pUpdateGroupCapAttrib(tpAniSirGlobal pMac, tANI_U8 SessionID, tp2p_group_owner_capability_config *groupcap)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tp2pContext *pP2pContext = &pMac->p2pContext[SessionID];

   if (pMac == NULL || groupcap == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }
   /* Initialize to zero*/
   pP2pContext->OriginalGroupCapability= 0;

   if (pP2pContext->operatingmode == OPERATION_MODE_P2P_GROUP_OWNER)
   {
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_GROUP_OWNER;
   }

   if (groupcap->bPersistentGroupEnabled)
   {
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_PERSISTENT_GROUP;
   }

   pP2pContext->maxGroupLimit = groupcap->uMaximumGroupLimit;
   
   if (pP2pContext->numClients >= groupcap->uMaximumGroupLimit)
   {
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_GROUP_LIMIT_REACHED;
   }

   if (groupcap->bIntraBSSDistributionSupported)
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_INTRABSS_DISTRIBUTION_SUPPORTED;

   if (groupcap->bCrossConnectionSupported)
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_CROSS_CONNECTION_SUPPORTED;

   if (groupcap->bPersistentReconnectSupported)
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_PERSISTENT_RECONNECT_SUPPORTED;

   if (groupcap->bGroupFormationEnabled)
      pP2pContext->OriginalGroupCapability |= DOT11_WFD_GROUP_CAPABILITY_IN_GROUP_FORMATION;

   gP2PIe[SessionID].p2pCapabilityAttrib.groupCapability = pP2pContext->OriginalGroupCapability;

   return status;
}

eHalStatus p2pUpdateDeviceId(tpAniSirGlobal pMac, tANI_U8 SessionID, tANI_U8 *MacAdressList, tANI_U32 numMacAddress)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (pMac == NULL || MacAdressList == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   /*TODO: What to do when more numMacAddress is more than 1*/
   if (numMacAddress)
   {
      vos_mem_copy(gP2PIe[SessionID].p2pDeviceIdAttrib.P2PDeviceAddress, MacAdressList, P2P_MAC_ADDRESS_LEN);
      vos_mem_copy(&gP2PIe[SessionID].p2pDeviceInfoAttrib.P2PDeviceAddress, MacAdressList, P2P_MAC_ADDRESS_LEN);
      gP2PIe[SessionID].p2pDeviceIdAttrib.present = TRUE;
   }
   else
   {
      gP2PIe[SessionID].p2pDeviceIdAttrib.present = FALSE;
   }

   return status;
}

eHalStatus P2P_UpdateIE(tHalHandle hHal, tANI_U8 SessionID, eP2PRequest oid, void *data, tANI_U32 len)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   
   switch (oid)
   {
   case eWFD_DEVICE_ID:
      status = p2pUpdateDeviceId(pMac,SessionID, (tANI_U8 *)data, len);
      break;

   case eWFD_DEVICE_CAPABILITY:
      status = p2pUpdateDeviceCapAttrib(pMac, SessionID, (tp2p_device_capability_config *)data);
      break;

   case eWFD_GROUP_OWNER_CAPABILITY:
      status = p2pUpdateGroupCapAttrib(pMac, SessionID, (tp2p_group_owner_capability_config *)data);
      break;

   case eWFD_DEVICE_INFO:
      status = p2pUpdateDeviceAttrib(pMac, SessionID, (tDot11fTLVP2PDeviceInfo *)data);
      break;

   case eWFD_SECONDARY_DEVICE_TYPE_LIST:
      status = p2pUpdateSecondaryDevTypeList(pMac, SessionID, (tp2p_secondary_device_type_list *)data);
      break;

   case eWFD_ADDITIONAL_IE:
      status = p2pUpdateAdditonalIE(pMac, SessionID, (tp2p_additional_ie *)data);
      break;

   case eWFD_GROUP_ID:
      status = p2pUpdateGroupID(pMac, SessionID, (tDot11fTLVP2PGroupId *)data);
      break;

   case eWFD_SEND_GO_NEGOTIATION_REQUEST:
      status = p2pUpdateGONegoReq(pMac, SessionID, (tP2P_go_request *)data);
      break;

   case eWFD_SEND_GO_NEGOTIATION_RESPONSE:
      status = p2pUpdateGONegoRes(pMac, SessionID, (tP2P_go_response *)data);
      break;

   case eWFD_SEND_GO_NEGOTIATION_CONFIRMATION:
      status = p2pUpdateGONegoCnf(pMac, SessionID, (tP2P_go_confirm *)data);
      break;

   case eWFD_SEND_PROVISION_DISCOVERY_REQUEST:
      status = p2pUpdateProvisionDiscoveryReq(pMac, SessionID, (tP2P_ProvDiscoveryReq *)data);
      break;

   case eWFD_SEND_PROVISION_DISCOVERY_RESPONSE:
      status = p2pUpdateProvisionDiscoveryRes(pMac, SessionID, (tP2P_ProvDiscoveryRes *)data);
      break;

   case eWFD_SEND_INVITATION_REQUEST:
      status = p2pUpdateInvitationReq(pMac, SessionID, (tP2P_invitation_request *)data);
      break;

   case eWFD_SEND_INVITATION_RESPONSE:
      status = p2pUpdateInvitationRes(pMac, SessionID, (tP2P_invitation_response *)data);
      break;

   case eWFD_OPERATING_CHANNEL:
      status = p2pUpdateOperatingChannelAttrib(pMac, SessionID, (tP2P_OperatingChannel *)data);
      break;

   case eWFD_LISTEN_CHANNEL:
      status = p2pUpdateListenChannelAttrib(pMac, SessionID, (tP2P_ListenChannel *)data);
      break;
   }

   return status;
}

eHalStatus p2pCreateBeaconIE(tp2pContext *p2pContext, tANI_U8 SessionID, tANI_U8 **pIEField, tANI_U32 *Ielen)
{
   tHalHandle hHal = p2pContext->hHal;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0;
   tANI_U32 lenRem = 0, len_filled = 0;
   tDot11fIEP2PBeacon p2pBeaconIE;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pIEField)
      vos_mem_free(*pIEField);

   *pIEField = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pIEField)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   vos_mem_zero(*pIEField, lenRem);

   if (gP2PIe[SessionID].p2pAdditionalIE.uBeaconIEsLength)
   {
      vos_mem_copy(*pIEField, gP2PIe[SessionID].p2pAdditionalIE.pBeaconIe, gP2PIe[SessionID].p2pAdditionalIE.uBeaconIEsLength);
      lenRem -= gP2PIe[SessionID].p2pAdditionalIE.uBeaconIEsLength;
      len += gP2PIe[SessionID].p2pAdditionalIE.uBeaconIEsLength;
   }

   p2pBeaconIE.present = TRUE;

   p2pBeaconIE.P2PCapability = gP2PIe[SessionID].p2pCapabilityAttrib;
   p2pBeaconIE.P2PCapability.present = TRUE;

   p2pBeaconIE.P2PDeviceId = gP2PIe[SessionID].p2pDeviceIdAttrib;
   p2pBeaconIE.P2PDeviceId.present = TRUE;

   p2pBeaconIE.NoticeOfAbsence.present = FALSE;//TODO

   dot11fPackIeP2PBeacon(pMac, &p2pBeaconIE, *pIEField + len, lenRem, &len_filled);
    
   *Ielen = len + len_filled;

   return status;
}

eHalStatus p2pCreateAssocReqIE(tp2pContext *p2pContext, tANI_U8 SessionId, tANI_U8 **pIEField, tANI_U32 *Ielen)
{
   tHalHandle hHal = p2pContext->hHal;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U16 probeRspIeLen = 0;
   tANI_U32 len = 0, len_filled = 0;
   tANI_U32 lenRem = 0;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pIEField)
      vos_mem_free(*pIEField);

   *pIEField = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pIEField)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }
   vos_mem_zero(*pIEField, lenRem);

   {
      tANI_U8 *pBuf = *pIEField + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;       
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;

   len_filled = 0;
   dot11fPackTlvP2PCapability(pMac, &gP2PIe[SessionId].p2pCapabilityAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Extended Listen Timing*/
   gP2PIe[SessionId].p2pExtendedListenTimingAttrib.present = FALSE;
   len_filled = 0;
   dot11fPackTlvExtendedListenTiming(pMac, &gP2PIe[SessionId].p2pExtendedListenTimingAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pIEField + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *Ielen = len;

   *pIeLen = len - 2;

   return status;
}


eHalStatus p2pGetGroupID(tp2pContext *p2pContext, tANI_U8 SessionID, tANI_U8 **pIEField, tANI_U32 *Ielen)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;

   if (*pIEField)
      vos_mem_free(*pIEField);

   *pIEField = (tANI_U8 *)vos_mem_malloc(sizeof(tDot11fTLVP2PGroupId));
   if(NULL == *pIEField)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }
   vos_mem_zero(*pIEField, sizeof(tDot11fTLVP2PGroupId));

   vos_mem_copy(*pIEField, &gP2PIe[SessionID].p2pGroupID, sizeof(tDot11fTLVP2PGroupId));

   return status;
}


eHalStatus p2pCreateProbeReqIE(tp2pContext *p2pContext, tANI_U8 SessionID, tANI_U8 **pIEField, tANI_U32 *Ielen)
{
   tHalHandle hHal = p2pContext->hHal;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U16 probeReqIeLen = 0;
   tANI_U32 len = 0;
   tANI_U32 lenRem = 0, len_filled = 0;
   tDot11fIEP2PProbeReq p2pProbeReqIE;
   tDot11fTLVP2PCapability p2pCapabilityAttrib;

   if (pMac == NULL)
   {
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pIEField)
      vos_mem_free(*pIEField);
   *pIEField = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pIEField)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   vos_mem_zero(&p2pProbeReqIE, sizeof(p2pProbeReqIE));

   if (p2pContext->DiscoverReqIeLength)
   {
      vos_mem_copy(*pIEField, p2pContext->DiscoverReqIeField, p2pContext->DiscoverReqIeLength);
      len = p2pContext->DiscoverReqIeLength;
   }
   else if (gP2PIe[SessionID].p2pAdditionalIE.uDefaultRequestIEsLength)
   {
      vos_mem_copy(*pIEField, gP2PIe[SessionID].p2pAdditionalIE.pDefaultRequestIe, 
         gP2PIe[SessionID].p2pAdditionalIE.uDefaultRequestIEsLength);
      len = gP2PIe[SessionID].p2pAdditionalIE.uDefaultRequestIEsLength;
   }

   lenRem -= len;
   p2pProbeReqIE.present = TRUE;

   vos_mem_copy(&p2pCapabilityAttrib, &gP2PIe[SessionID].p2pCapabilityAttrib,
      sizeof(p2pCapabilityAttrib));
   if(OPERATION_MODE_P2P_DEVICE == p2pContext->operatingmode)
   {
      if(!p2pContext->GroupFormationPending)
      {
         p2pCapabilityAttrib.groupCapability = p2pContext->OriginalGroupCapability;
      }
   }
   p2pProbeReqIE.P2PCapability = p2pCapabilityAttrib;
   p2pProbeReqIE.P2PCapability.present = TRUE;

   p2pProbeReqIE.P2PDeviceId = gP2PIe[SessionID].p2pDeviceIdAttrib;
   p2pProbeReqIE.P2PDeviceId.present = FALSE;

   p2pProbeReqIE.ExtendedListenTiming = gP2PIe[SessionID].p2pExtendedListenTimingAttrib;
   p2pProbeReqIE.ExtendedListenTiming.present = FALSE;

   p2pProbeReqIE.ListenChannel = gP2PIe[SessionID].p2pListenChannelAttrib;
   p2pProbeReqIE.ListenChannel.present = TRUE;

   if (pMac->p2pContext[SessionID].operatingmode == OPERATION_MODE_P2P_GROUP_OWNER)
   {
      p2pProbeReqIE.OperatingChannel = gP2PIe[SessionID].p2pOperatingChannelAttrib;
      p2pProbeReqIE.OperatingChannel.present = TRUE;
   }

   if (dot11fPackIeP2PProbeReq(pMac, &p2pProbeReqIE, *pIEField + len, lenRem, &len_filled))
   {
      vos_mem_free(*pIEField);
      status = eHAL_STATUS_FAILURE;
      return status;
   }

   *Ielen = len + len_filled;
   
   return status;
}


void packtlvdeviceinfo(tANI_U8 *pIEField, tANI_U8 SessionId, tANI_U32 nBuf, tANI_U32 *pnConsumed)
{
   /*P2P device Info*/
   tANI_U8 *ptr = pIEField;
   tANI_U16 tempLen = 0;
   tANI_U16 *ptrToLen;
   tANI_U16 *primaryDeviceType;

   /*Device Info Attrib ID*/
   *ptr++ = 13;

   ptrToLen = (tANI_U16 *)ptr;
   ptr += 2;

   vos_mem_copy(ptr, gP2PIe[SessionId].p2pDeviceInfoAttrib.P2PDeviceAddress, sizeof (gP2PIe[SessionId].p2pDeviceInfoAttrib.P2PDeviceAddress));
   ptr += sizeof (gP2PIe[SessionId].p2pDeviceInfoAttrib.P2PDeviceAddress);
   tempLen += sizeof (gP2PIe[SessionId].p2pDeviceInfoAttrib.P2PDeviceAddress);

   *(tANI_U16 *)ptr = gP2PIe[SessionId].p2pDeviceInfoAttrib.configMethod;
       
   *ptr = (gP2PIe[SessionId].p2pDeviceInfoAttrib.configMethod >> 8) & 0xFF ;
   *(ptr + 1) = gP2PIe[SessionId].p2pDeviceInfoAttrib.configMethod & 0xFF;
   ptr += 2;
   tempLen += 2;

   //catogary ID
   primaryDeviceType = (tANI_U16 *)gP2PIe[SessionId].p2pDeviceInfoAttrib.primaryDeviceType;
   *ptr++ = ((*primaryDeviceType) >> 8) & 0xFF ;
   *ptr++ = (*primaryDeviceType) & 0xFF;
   primaryDeviceType += 1;

   // sub catogary id
   *ptr++ = ((*primaryDeviceType) >> 8) & 0xFF ;
   *ptr++ = (*primaryDeviceType) & 0xFF;
   primaryDeviceType += 1;

   //OUI
   vos_mem_copy(ptr, primaryDeviceType, 4);
   ptr += 4;

   tempLen += sizeof (gP2PIe[SessionId].p2pDeviceInfoAttrib.primaryDeviceType);

   *ptr++ = gP2PIe[SessionId].p2pSecondaryDevTypeList.uNumOfEntries;
   tempLen += 1;

   if (gP2PIe[SessionId].p2pSecondaryDevTypeList.uNumOfEntries)
   {
      vos_mem_copy(ptr, gP2PIe[SessionId].p2pSecondaryDevTypeList.SecondaryDeviceTypes, gP2PIe[SessionId].p2pSecondaryDevTypeList.uNumOfEntries * sizeof (gP2PIe[SessionId].p2pSecondaryDevTypeList.SecondaryDeviceTypes));
      ptr += gP2PIe[SessionId].p2pSecondaryDevTypeList.uNumOfEntries * sizeof (gP2PIe[SessionId].p2pSecondaryDevTypeList.SecondaryDeviceTypes);
      tempLen += gP2PIe[SessionId].p2pSecondaryDevTypeList.uNumOfEntries * sizeof (gP2PIe[SessionId].p2pSecondaryDevTypeList.SecondaryDeviceTypes);
   }

   /*WSC Device Name Attribute ID*/
   *ptr++ = 0x10;
   *ptr++ = 0x11;
   tempLen += 2;

   *ptr++ = (gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.num_text & 0xFF) >> 8;
   *ptr++ = gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.num_text & 0xFF;

   tempLen += 2;

   vos_mem_copy (ptr, gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.text, gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.num_text);
   ptr += gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.num_text;
   tempLen += gP2PIe[SessionId].p2pDeviceInfoAttrib.DeviceName.num_text;

   *ptrToLen = tempLen; 
   *pnConsumed = tempLen + 3;//Attirb ID + Length;

   return;
}

eHalStatus p2pCreateProbeRspIE(tp2pContext *p2pContext, tANI_U8 SessionId, tANI_U8 **pIEField, tANI_U32 *Ielen)
{
   tHalHandle hHal = p2pContext->hHal;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U16 probeRspIeLen = 0;
   tANI_U32 len = 0, len_filled = 0;
   tANI_U32 lenRem = 0;
   tANI_U8 *pIeLen;
   tDot11fTLVP2PCapability p2pCapabilityAttrib;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pIEField)
      vos_mem_free(*pIEField);

   *pIEField = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pIEField)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }
   vos_mem_zero(*pIEField, lenRem);

   if (gP2PIe[SessionId].p2pAdditionalIE.uProbeResponseIEsLength)
   {
      vos_mem_copy(*pIEField, gP2PIe[SessionId].p2pAdditionalIE.pProbeResponseIe,
               gP2PIe[SessionId].p2pAdditionalIE.uProbeResponseIEsLength);
      lenRem -= gP2PIe[SessionId].p2pAdditionalIE.uProbeResponseIEsLength;
   }

   len = gP2PIe[SessionId].p2pAdditionalIE.uProbeResponseIEsLength;

   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pIEField + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;       
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;

   len_filled = 0;
   vos_mem_copy(&p2pCapabilityAttrib, &gP2PIe[SessionId].p2pCapabilityAttrib,
      sizeof(p2pCapabilityAttrib));
   if(OPERATION_MODE_P2P_DEVICE == p2pContext->operatingmode)
   {
      if(!p2pContext->GroupFormationPending)
      {
         p2pCapabilityAttrib.groupCapability = p2pContext->OriginalGroupCapability;
      }
   }
   dot11fPackTlvP2PCapability(pMac, &p2pCapabilityAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Extended Listen Timing*/
   gP2PIe[SessionId].p2pExtendedListenTimingAttrib.present = FALSE;
   len_filled = 0;
   dot11fPackTlvExtendedListenTiming(pMac, &gP2PIe[SessionId].p2pExtendedListenTimingAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Notice of absence*/
   gP2PIe[SessionId].p2pNoticeOfAbsenceAttrib.present = FALSE;
   len_filled = 0;
   dot11fPackTlvNoticeOfAbsence(pMac, &gP2PIe[SessionId].p2pNoticeOfAbsenceAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pIEField + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Group Info*/
   gP2PIe[SessionId].p2pGroupInfoAttrib.present = FALSE;
   len_filled = 0;
   dot11fPackTlvP2PGroupInfo(pMac, &gP2PIe[SessionId].p2pGroupInfoAttrib, *pIEField + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *Ielen = len;

   *pIeLen = len - gP2PIe[SessionId].p2pAdditionalIE.uProbeResponseIEsLength - 2;

   return status;
}

void packtlvActionFrameHeaderinfo(tHalHandle hHal, tANI_U8 SessionId, tANI_U8 *pIEField, tANI_U32 nBuf, tANI_U32 *actionFrameHeaderLen, eOUISubType ouiSubType)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 pnConsumed = 0;

   tANI_U8 *pBuf = pIEField + sizeof (tSirMacMgmtHdr);

   *pBuf = 0x04;
   ++pBuf; ++pnConsumed;
   *pBuf = 0x09;
   ++pBuf; ++pnConsumed;
   *pBuf = 0x50;
   ++pBuf; ++pnConsumed;
   *pBuf = 0x6f;
   ++pBuf; ++pnConsumed;
   *pBuf = 0x9a;
   ++pBuf; ++pnConsumed;
   *pBuf = 0x09;
   ++pBuf; ++pnConsumed;
   *pBuf = ouiSubType;
   ++pBuf; ++pnConsumed;
   
   if(ouiSubType == eOUI_P2P_GONEGO_REQ || ouiSubType == eOUI_P2P_PROVISION_DISCOVERY_REQ 
      || ouiSubType == eOUI_P2P_INVITATION_REQ)
   {
      *pBuf = pMac->p2pContext[SessionId].dialogToken;
   }
   else
   {
      *pBuf = pMac->p2pContext[SessionId].receivedDialogToken;
   }

   ++pBuf; ++pnConsumed;
   *actionFrameHeaderLen = pnConsumed;

   pMac->p2pContext[SessionId].actionFrameOUI = ouiSubType;

   return;
}

eHalStatus p2pCreateGONegoReqFrm(tHalHandle hHal, tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0;
   tANI_U32 lenRem = 0;
   tDot11fGONegReq GONegReqIE;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);

   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_GONEGO_REQ);
   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   if (pMac->p2pContext[SessionId].GoNegoReqIeLength)
   {
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].GoNegoReqIeField, pMac->p2pContext[SessionId].GoNegoReqIeLength);
      len += pMac->p2pContext[SessionId].GoNegoReqIeLength;
      lenRem -= len;
   }

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;        
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;
   dot11fPackTlvP2PCapability(pMac, &gP2PIe[SessionId].p2pCapabilityAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Group Owner Intent Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pGroupOwnerIntent.present = TRUE;
   dot11fPackTlvGOIntent(pMac, &gP2PIe[SessionId].p2pGroupOwnerIntent, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Configuration Timeout Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pConfTimeout.present = TRUE;
   dot11fPackTlvConfigurationTimeout(pMac, &gP2PIe[SessionId].p2pConfTimeout, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Listen Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pListenChannelAttrib.present = TRUE;
   dot11fPackTlvListenChannel(pMac, &gP2PIe[SessionId].p2pListenChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Extended Listen Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pExtendedListenTimingAttrib.present = FALSE;
   dot11fPackTlvExtendedListenTiming(pMac, &gP2PIe[SessionId].p2pExtendedListenTimingAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P IntendedP2P Interface address Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pIntendedInterfaceAddr.present = TRUE;
   dot11fPackTlvIntendedP2PInterfaceAddress(pMac, &gP2PIe[SessionId].p2pIntendedInterfaceAddr, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Channel List Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pChannelList.present = TRUE;
   dot11fPackTlvChannelList(pMac, &gP2PIe[SessionId].p2pChannelList, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pActionFrm + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Operating Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pOperatingChannelAttrib.present = TRUE;
   dot11fPackTlvOperatingChannel(pMac, &gP2PIe[SessionId].p2pOperatingChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].GoNegoReqIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}

eHalStatus p2pCreateGONegoRspFrm(tHalHandle hHal, tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0;
   tANI_U32 lenRem = 0;
   tDot11fGONegRes GONegResIE;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);
   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_GONEGO_RES);

   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;        
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P status*/
   len_filled = 0;
   gP2PIe[SessionId].p2pStatus.present = TRUE;
   dot11fPackTlvP2PStatus(pMac, &gP2PIe[SessionId].p2pStatus, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;
   dot11fPackTlvP2PCapability(pMac, &gP2PIe[SessionId].p2pCapabilityAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Group Owner Intent Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pGroupOwnerIntent.present = TRUE;
   dot11fPackTlvGOIntent(pMac, &gP2PIe[SessionId].p2pGroupOwnerIntent, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Configuration Timeout Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pConfTimeout.present = TRUE;
   dot11fPackTlvConfigurationTimeout(pMac, &gP2PIe[SessionId].p2pConfTimeout, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Operating Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pOperatingChannelAttrib.present = TRUE;
   dot11fPackTlvOperatingChannel(pMac, &gP2PIe[SessionId].p2pOperatingChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P IntendedP2P Interface address Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pIntendedInterfaceAddr.present = TRUE;
   dot11fPackTlvIntendedP2PInterfaceAddress(pMac, &gP2PIe[SessionId].p2pIntendedInterfaceAddr, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Channel List Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pChannelList.present = TRUE;
   dot11fPackTlvChannelList(pMac, &gP2PIe[SessionId].p2pChannelList, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pActionFrm + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Group ID Attrib*/
   len_filled = 0;
   //gP2PIe[SessionID].p2pGroupID.present = FALSE; // This field is updated depending on bUseGroupID field in the GO RESPONSE / CONFIRM
   dot11fPackTlvP2PGroupId(pMac, &gP2PIe[SessionId].p2pGroupID, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   if (pMac->p2pContext[SessionId].GoNegoResIeLength)
   {
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].GoNegoResIeField, pMac->p2pContext[SessionId].GoNegoResIeLength);
      len += pMac->p2pContext[SessionId].GoNegoResIeLength;
      lenRem -= len;
   }

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].GoNegoResIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}

eHalStatus p2pCreateGONegoCnfFrm(tHalHandle hHal,  tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0;
   tANI_U32 lenRem = 0;
   tDot11fGONegCnf GONegCnfIE;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);
   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_GONEGO_CNF);

   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   if (pMac->p2pContext[SessionId].GoNegoCnfIeLength)
   {
      vos_mem_copy(*pActionFrm, pMac->p2pContext[SessionId].GoNegoCnfIeField, pMac->p2pContext[SessionId].GoNegoCnfIeLength);
      len = pMac->p2pContext[SessionId].GoNegoCnfIeLength;
      lenRem -= len;
   }

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;        
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P status*/
   len_filled = 0;
   gP2PIe[SessionId].p2pStatus.present = TRUE;
   dot11fPackTlvP2PStatus(pMac, &gP2PIe[SessionId].p2pStatus, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;
   dot11fPackTlvP2PCapability(pMac, &gP2PIe[SessionId].p2pCapabilityAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Operating Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pOperatingChannelAttrib.present = TRUE;
   dot11fPackTlvOperatingChannel(pMac, &gP2PIe[SessionId].p2pOperatingChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Channel List Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pChannelList.present = TRUE;
   dot11fPackTlvChannelList(pMac, &gP2PIe[SessionId].p2pChannelList, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Group ID Attrib*/
   len_filled = 0;
   //gP2PIe[SessionID].p2pGroupID.present = TRUE; // This field is updated depending on bUseGroupID field in the GO RESPONSE / CONFIRM
   dot11fPackTlvP2PGroupId(pMac, &gP2PIe[SessionId].p2pGroupID, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].GoNegoCnfIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}

eHalStatus p2pCreateProvDiscReqFrm(tHalHandle hHal,  tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0 ;
   tANI_U32 lenRem = 0;
   tDot11fProvisionDiscoveryReq ProvDiscReq;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);

   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   /*Action Frame Header*/
   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_PROVISION_DISCOVERY_REQ);

   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   if (pMac->p2pContext[SessionId].ProvDiscReqIeLength)
   {
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].ProvDiscReqIeField, pMac->p2pContext[SessionId].ProvDiscReqIeLength);
      len += pMac->p2pContext[SessionId].ProvDiscReqIeLength;
      lenRem -= len;
   }

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;        
   }

   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   gP2PIe[SessionId].p2pCapabilityAttrib.present = TRUE;
   len_filled = 0;
   dot11fPackTlvP2PCapability(pMac, &gP2PIe[SessionId].p2pCapabilityAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pActionFrm + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Capability Attrib*/
   //gP2PIe[SessionID].p2pCapabilityAttrib.present = TRUE; // This field is updated depending on bUseGroupID field in the PROVISION REQUEST
   len_filled = 0;
   dot11fPackTlvP2PGroupId(pMac, &gP2PIe[SessionId].p2pGroupID, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].ProvDiscReqIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}

eHalStatus p2pCreateProvDiscResFrm(tHalHandle hHal,  tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0 ;
   tANI_U32 lenRem = 0;
   tDot11fProvisionDiscoveryRes ProvDiscRes;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);
   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   /*Action Frame Header*/
   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_PROVISION_DISCOVERY_RES);

   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   if (pMac->p2pContext[SessionId].ProvDiscResIeLength)
   {
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].ProvDiscResIeField, pMac->p2pContext[SessionId].ProvDiscResIeLength);
      len += pMac->p2pContext[SessionId].ProvDiscResIeLength;
      lenRem -= len;
   }

   *pActionFrmlen = pal_cpu_to_be16(len);

   return status;
}

eHalStatus p2pCreateInvitationReqFrm(tHalHandle hHal, tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0;
   tANI_U32 lenRem = 0;
   tDot11fInvitationReq InvitationReqIE;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);

   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_INVITATION_REQ);
   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   if (pMac->p2pContext[SessionId].InvitationReqIeLength) 
   {
      //dump("p2pCreateInvitationReqFrm", pMac->p2pContext[SessionId].InvitationReqIeField, pMac->p2pContext[SessionId].InvitationReqIeLength);
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].InvitationReqIeField, pMac->p2pContext[SessionId].InvitationReqIeLength);
      len += pMac->p2pContext[SessionId].InvitationReqIeLength;
      lenRem -= len;
   }

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;
        
   }
   len += len_filled;
   lenRem -= len_filled;

    /*P2P Configuration Timeout Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pConfTimeout.present = TRUE;
   dot11fPackTlvConfigurationTimeout(pMac, &gP2PIe[SessionId].p2pConfTimeout, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Invitation Flags Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pInvitationReq.InvitationFlags.present = TRUE;
   dot11fPackTlvInvitationFlags(pMac, &gP2PIe[SessionId].p2pInvitationReq.InvitationFlags, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Operating Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pOperatingChannelAttrib.present = TRUE;
   dot11fPackTlvOperatingChannel(pMac, &gP2PIe[SessionId].p2pOperatingChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P GroupBSSID  Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pGroupBssid.present = TRUE;
   dot11fPackTlvP2PGroupBssid(pMac, &gP2PIe[SessionId].p2pGroupBssid, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Channel List Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pChannelList.present = TRUE;
   dot11fPackTlvChannelList(pMac, &gP2PIe[SessionId].p2pChannelList, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Group ID*/
   len_filled = 0;
   gP2PIe[SessionId].p2pGroupID.present = TRUE;
   dot11fPackTlvP2PGroupId(pMac, &gP2PIe[SessionId].p2pGroupID, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*p2P Device Info*/
   len_filled = 0;
   packtlvdeviceinfo(*pActionFrm + len, SessionId, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].InvitationReqIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}


eHalStatus p2pCreateInvitationResFrm(tHalHandle hHal, tANI_U8 SessionId, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmlen)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tANI_U32 len = 0, len_filled = 0, actionFrameHeaderLen = 0;
   tANI_U32 lenRem = 0;
   tDot11fInvitationRes InvitationResIE;
   tANI_U8 *pIeLen;

   lenRem = MAX_P2P_SPECIFIC_IES * MAX_P2P_IE_LEN;

   if (*pActionFrm)
      vos_mem_free(*pActionFrm);
   *pActionFrm = (tANI_U8 *)vos_mem_malloc(lenRem);
   if(NULL == *pActionFrm)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR,
            " %s fail to allocate memory", __FUNCTION__);
      return eHAL_STATUS_RESOURCES;
   }

   packtlvActionFrameHeaderinfo(hHal, SessionId, *pActionFrm, lenRem, &actionFrameHeaderLen, eOUI_P2P_INVITATION_RES);

   len += sizeof (tSirMacMgmtHdr) + actionFrameHeaderLen;
   lenRem -= len;

   len_filled = 0;
   /*P2P IE header*/
   {
      tANI_U8 *pBuf = *pActionFrm + len;

      *pBuf = 221;
      ++pBuf; ++len_filled;
      pIeLen = pBuf;
      ++pBuf; ++len_filled;
      *pBuf = 0x50;
      ++pBuf; ++len_filled;
      *pBuf = 0x6f;
      ++pBuf; ++len_filled;
      *pBuf = 0x9a;
      ++pBuf; ++len_filled;
      *pBuf = 0x9;
      ++pBuf; ++len_filled;
        
   }
   len += len_filled;
   lenRem -= len_filled;

   /*P2P status*/
   len_filled = 0;
   gP2PIe[SessionId].p2pStatus.present = TRUE;
   dot11fPackTlvP2PStatus(pMac, &gP2PIe[SessionId].p2pStatus, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Configuration Timeout Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pConfTimeout.present = TRUE;
   dot11fPackTlvConfigurationTimeout(pMac, &gP2PIe[SessionId].p2pConfTimeout, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Operating Channel Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pOperatingChannelAttrib.present = TRUE;
   dot11fPackTlvOperatingChannel(pMac, &gP2PIe[SessionId].p2pOperatingChannelAttrib, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P GroupBSSID  Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pGroupBssid.present = TRUE;
   dot11fPackTlvP2PGroupBssid(pMac, &gP2PIe[SessionId].p2pGroupBssid, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   /*P2P Channel List Attrib*/
   len_filled = 0;
   gP2PIe[SessionId].p2pChannelList.present = TRUE;
   dot11fPackTlvChannelList(pMac, &gP2PIe[SessionId].p2pChannelList, *pActionFrm + len, lenRem, &len_filled);
   len += len_filled;
   lenRem -= len_filled;

   if (pMac->p2pContext[SessionId].InvitationResIeLength) 
   {
      //dump("p2pCreateInvitationResFrm", pMac->p2pContext[SessionId].InvitationResIeField, pMac->p2pContext[SessionId].InvitationResIeLength);
      vos_mem_copy(*pActionFrm + len, pMac->p2pContext[SessionId].InvitationResIeField, pMac->p2pContext[SessionId].InvitationResIeLength);
      len += pMac->p2pContext[SessionId].InvitationResIeLength;
      lenRem -= len;
   }

   *pActionFrmlen = pal_cpu_to_be16(len);

   *pIeLen = len - pMac->p2pContext[SessionId].InvitationResIeLength - 2 - sizeof (tSirMacMgmtHdr) - actionFrameHeaderLen;

   return status;
}



eHalStatus P2P_GetIE(tp2pContext *p2pContext, tANI_U8 SessionId, eP2PFrameType ftype, tANI_U8 **pIEField, tANI_U32 *len)
{
   eHalStatus status = eHAL_STATUS_FAILURE;

   switch (ftype) {
      case eP2P_PROBE_REQ:
         p2pCreateProbeReqIE(p2pContext, SessionId, pIEField, len);
         break;

      case eP2P_PROBE_RSP:
         p2pCreateProbeRspIE(p2pContext, SessionId, pIEField, len);
         break;

      case eP2P_BEACON:
         p2pCreateBeaconIE(p2pContext, SessionId, pIEField, len);
         break;

      case eP2P_GROUP_ID:
         p2pGetGroupID(p2pContext, SessionId, pIEField, len);
         break;
      case eP2P_ASSOC_REQ:
         p2pCreateAssocReqIE(p2pContext, SessionId, pIEField, len);
         break;

      default:
         break;
   }

   return status;
}


eHalStatus P2P_GetActionFrame(tHalHandle hHal, tANI_U8 SessionID, eP2PFrameType ftype, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmLen)
{
   eHalStatus status = eHAL_STATUS_SUCCESS;
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

   switch (ftype)
   {
   case eP2P_GONEGO_REQ:
      p2pCreateGONegoReqFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_GONEGO_RES:
      p2pCreateGONegoRspFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_GONEGO_CNF:
      p2pCreateGONegoCnfFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_PROVISION_DISCOVERY_REQUEST:
      p2pCreateProvDiscReqFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_PROVISION_DISCOVERY_RESPONSE:
      p2pCreateProvDiscResFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_INVITATION_REQ:
      p2pCreateInvitationReqFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   case eP2P_INVITATION_RSP:
      p2pCreateInvitationResFrm(hHal, SessionID, pActionFrm, pActionFrmLen);
      break;

   }
   return status;
}


void p2pCreateDefaultIEs(tHalHandle hHal, tANI_U8 SessionID)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   tp2p_device_capability_config devcap;
   tp2p_group_owner_capability_config groupcap;
   tANI_U8 MacAddressList[P2P_MAC_ADDRESS_LEN];
   tANI_U32 numMacAddress = 0;
   tP2P_OperatingChannel p2pOperatingChannel;

   vos_mem_set(&gP2PIe[SessionID], 0, sizeof(gP2PIe[SessionID]));

   /*Default device capability IE*/
   devcap.bServiceDiscoveryEnabled = TRUE;
   devcap.bClientDiscoverabilityEnabled = TRUE;
   devcap.bConcurrentOperationSupported = TRUE;
   devcap.bInfrastructureManagementEnabled = TRUE;
   devcap.bDeviceLimitReached = FALSE;
   devcap.bInvitationProcedureEnabled = FALSE;
   p2pUpdateDeviceCapAttrib(pMac, SessionID, &devcap);

   /*Default Group Capability*/
   groupcap.bPersistentGroupEnabled = FALSE;
   groupcap.bIntraBSSDistributionSupported = FALSE;
   groupcap.bCrossConnectionSupported = FALSE;
   groupcap.bPersistentReconnectSupported = FALSE;
   groupcap.bGroupFormationEnabled = TRUE;
   groupcap.uMaximumGroupLimit = 4;
   p2pUpdateGroupCapAttrib(pMac, SessionID, &groupcap);

   
   p2pOperatingChannel.channel = P2P_OPERATING_CHANNEL;
   p2pOperatingChannel.countryString[0] = 0x55;
   p2pOperatingChannel.countryString[1] = 0x53;
   p2pOperatingChannel.countryString[2] = 0x04;

   /*Listen Channel Attribute*/
   p2pUpdateListenChannelAttrib(pMac, SessionID, &p2pOperatingChannel);

   p2pOperatingChannel.regulatoryClass = 0x51;

   /*Operating Channel Attribute*/
   p2pUpdateOperatingChannelAttrib(pMac, SessionID, &p2pOperatingChannel);

   /*Extended Listen Timing Attribute*/
   p2pUpdateExtendedListenTimingAttrib(pMac, SessionID);

   /*Channel List*/
   p2pUpdateChannelListAttrib(pMac, SessionID);

   /*Device ID Attribute*/
   numMacAddress = 0;
   p2pUpdateDeviceId(pMac, SessionID, MacAddressList, numMacAddress);

   return;
}

tANI_U8 *P2P_GetBeaconResponseIe(tANI_U8 SessionID)
{
   return gP2PIe[SessionID].p2pAdditionalIE.pBeaconIe;
}

tANI_U8 *P2P_GetProbeResponseIe(tANI_U8 SessionID)
{
   return gP2PIe[SessionID].p2pAdditionalIE.pProbeResponseIe;
}

eHalStatus P2P_GetOperatingChannel(tHalHandle hHal, tANI_U8 SessionID, tP2P_OperatingChannel *p2pOperatingChannel)
{
   tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
   eHalStatus status = eHAL_STATUS_SUCCESS;

   p2pOperatingChannel->channel = gP2PIe[SessionID].p2pOperatingChannelAttrib.channel;
   vos_mem_copy(p2pOperatingChannel->countryString, gP2PIe[SessionID].p2pOperatingChannelAttrib.countryString, 
         sizeof(p2pOperatingChannel->countryString));
   p2pOperatingChannel->regulatoryClass = gP2PIe[SessionID].p2pOperatingChannelAttrib.regulatoryClass;

   return status;
}
#endif //WLAN_FEATURE_P2P_INTERNAL
