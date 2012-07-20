/*
* Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

#ifndef __P2P_IE_H__
#define __P2P_IE_H__

#ifdef WLAN_FEATURE_P2P_INTERNAL

#define MAX_COUNTRY_STRING_SIZE 3
#define P2P_ATTRIB_HEADER_LEN   3
#define MAX_P2P_SPECIFIC_IES 7
#define MAX_P2P_IE_LEN 257
#define MAX_DEVICE_TYPE_LEN 8

#define WFD_DEVICE_CAPABILITY_NONE                                 0x00
#define WFD_DEVICE_CAPABILITY_SERVICE_DISCOVERY                0x01
#define WFD_DEVICE_CAPABILITY_CLIENT_DISCOVERABILITY           0x02
#define WFD_DEVICE_CAPABILITY_CONCURRENT_OPERATION             0x04
#define WFD_DEVICE_CAPABILITY_INFRASTRUCTURE_MANAGEMENT        0x08
#define WFD_DEVICE_CAPABILITY_DEVICE_LIMIT_REACHED             0x10
#define WFD_DEVICE_CAPABILITY_INVITATION_PROCEDURE             0x20

#define P2P_DEVICE_NAME_MAX_LENGTH 32 // 32 bytes

typedef struct _p2p_device_type {
   tANI_U16 CategoryID;
    tANI_U16 SubCategoryID;
    tANI_U16 OUI[4];
} tp2p_device_type;

typedef struct _p2p_device_name {
   tANI_U32 uDeviceNameLength;
   tANI_U8 ucDeviceName[P2P_DEVICE_NAME_MAX_LENGTH];
} tp2p_device_name;

typedef struct _p2p_additional_ie {
   tANI_U8 present;
   tANI_U32 uBeaconIEsLength;
   tANI_U8  *pBeaconIe;
   tANI_U32 uProbeResponseIEsLength;
   tANI_U8  *pProbeResponseIe;
   tANI_U32 uDefaultRequestIEsLength;
   tANI_U8  *pDefaultRequestIe;
} tp2p_additional_ie;

typedef struct _tp2pIeHeader{
   tANI_U8 ElementId;
   tANI_U8 Length;
   tANI_U8 Oui[3];
   tANI_U8 OuiType;
} tp2pIeHeader;

typedef struct _p2p_secondary_device_type_list {
   tANI_U32 uNumOfEntries;
   tANI_U32 uTotalNumOfEntries;
   tp2p_device_type *SecondaryDeviceTypes;
} tp2p_secondary_device_type_list;

typedef struct tpp2pie {
   tDot11fTLVP2PStatus p2pStatus;
   tDot11fTLVMinorReasonCode p2pReasonCode;
   tDot11fTLVP2PCapability p2pCapabilityAttrib;
   tDot11fTLVP2PDeviceId p2pDeviceIdAttrib;
   tDot11fTLVGOIntent p2pGroupOwnerIntent;
   tDot11fTLVConfigurationTimeout p2pConfTimeout;
   tDot11fTLVListenChannel p2pListenChannelAttrib;
   tDot11fTLVP2PGroupBssid p2pGroupBssid;
   tDot11fTLVExtendedListenTiming p2pExtendedListenTimingAttrib;
   tDot11fTLVIntendedP2PInterfaceAddress p2pIntendedInterfaceAddr;
   tDot11fTLVP2PManageability p2pManageability;
   tDot11fTLVChannelList p2pChannelList;
   tDot11fIEP2PNoticeOfAbsence p2pNoticeOfAbsenceAttrib;
   tDot11fTLVP2PDeviceInfo p2pDeviceInfoAttrib;
   tDot11fTLVP2PGroupInfo p2pGroupInfoAttrib;
   tDot11fTLVP2PGroupId p2pGroupID;
   tDot11fTLVP2PInterface p2pInterface;
   tDot11fTLVOperatingChannel p2pOperatingChannelAttrib;
   tDot11fIEP2PInvitationReq p2pInvitationReq;
   tp2p_additional_ie p2pAdditionalIE;
   tp2p_secondary_device_type_list p2pSecondaryDevTypeList;
} tp2pie[MAX_NO_OF_P2P_SESSIONS];

eHalStatus P2P_UpdateIE(tHalHandle hHal, tANI_U8 SessionID, eP2PRequest oid, void *data, tANI_U32 len);
eHalStatus p2pCreateProbeReqIE(tp2pContext *p2pContext, tANI_U8 SessionID, tANI_U8 **pIEField, tANI_U32 *Ielen);
eHalStatus p2pCreateProbeRspIE(tp2pContext *p2pContext, tANI_U8 SessionID, tANI_U8 **pIEField, tANI_U32 *Ielen);
void p2pCreateDefaultIEs(tHalHandle hHal, tANI_U8 SessionID);
eHalStatus P2P_GetActionFrame(tHalHandle hHal, tANI_U8 SessionID, eP2PFrameType ftype, tANI_U8 **pActionFrm, tANI_U32 *pActionFrmLen);
eHalStatus P2P_GetIE(tp2pContext *p2pContext, tANI_U8 SessionId, eP2PFrameType ftype, tANI_U8 **pIEField, tANI_U32 *len);
tANI_U8 *P2P_GetBeaconResponseIe(tANI_U8 SessionID);
tANI_U8 *P2P_GetProbeResponseIe(tANI_U8 SessionID);
eHalStatus  P2P_UpdateGroupCapability(tHalHandle pMac, tANI_U8 SessionID, tANI_U8 GroupCapability, tANI_U8 val);
eHalStatus  P2P_UpdateDeviceCapability(tHalHandle pMac, tANI_U8 SessionID, tANI_U8 DeviceCapability, tANI_U8 val);
eHalStatus P2P_SetOperationMode(tHalHandle hHal, tANI_U8 SessionID, ep2pOperatingMode val);
ep2pOperatingMode P2P_GetOperationMode(tHalHandle hHal, tANI_U8 SessionID);
void packtlvdeviceinfo(tANI_U8 *pIEField, tANI_U8 SessionId, tANI_U32 nBuf, tANI_U32 *pnConsumed);
eHalStatus P2P_GetOperatingChannel(tHalHandle hHal, tANI_U8 SessionID, tP2P_OperatingChannel *p2pOperatingChannel);
eHalStatus p2pUpdateListenChannelAttrib(tHalHandle hHal, tANI_U8 SessionID, tP2P_ListenChannel *p2pListenChannel);
eHalStatus p2pGetListenChannelAttrib(tHalHandle hHal, tANI_U8 SessionID, tP2P_ListenChannel *p2pListenChannel);
#endif // WLAN_WIN8_P2P
#endif //__P2P_IE_H__
