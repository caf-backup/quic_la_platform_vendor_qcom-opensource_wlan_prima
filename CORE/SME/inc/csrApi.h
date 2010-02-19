/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file csrApi.h
  
    Exports and types for the Common Scan and Roaming Module interfaces.
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
 
   ========================================================================== */
#ifndef CSRAPI_H__
#define CSRAPI_H__

#include "sirApi.h"
#include "sirMacProtDef.h"
#include "halRfTypes.h"
#include "csrLinkList.h"

typedef enum 
{
    eCSR_AUTH_TYPE_NONE,    //never used
    // MAC layer authentication types
    eCSR_AUTH_TYPE_OPEN_SYSTEM,
    eCSR_AUTH_TYPE_SHARED_KEY,
    eCSR_AUTH_TYPE_AUTOSWITCH,

    // Upper layer authentication types
    eCSR_AUTH_TYPE_WPA,
    eCSR_AUTH_TYPE_WPA_PSK,
    eCSR_AUTH_TYPE_WPA_NONE,

    eCSR_AUTH_TYPE_RSN,
    eCSR_AUTH_TYPE_RSN_PSK,

    eCSR_NUM_OF_SUPPORT_AUTH_TYPE = eCSR_AUTH_TYPE_RSN_PSK + 1,

    eCSR_AUTH_TYPE_FAILED = 0xff,
    eCSR_AUTH_TYPE_UNKNOWN = eCSR_AUTH_TYPE_FAILED,

}eCsrAuthType;


typedef enum 
{
    eCSR_ENCRYPT_TYPE_NONE,
    eCSR_ENCRYPT_TYPE_WEP40_STATICKEY,
    eCSR_ENCRYPT_TYPE_WEP104_STATICKEY,

    eCSR_ENCRYPT_TYPE_WEP40,
    eCSR_ENCRYPT_TYPE_WEP104,
    eCSR_ENCRYPT_TYPE_TKIP,
    eCSR_ENCRYPT_TYPE_AES,
    eCSR_ENCRYPT_TYPE_ANY,
    eCSR_NUM_OF_ENCRYPT_TYPE = eCSR_ENCRYPT_TYPE_ANY,

    eCSR_ENCRYPT_TYPE_FAILED = 0xff,
    eCSR_ENCRYPT_TYPE_UNKNOWN = eCSR_ENCRYPT_TYPE_FAILED,

}eCsrEncryptionType;

typedef enum
{
    eCSR_DOT11_MODE_TAURUS = 0, //Taurus mean everything because it covers all thing we support
    eCSR_DOT11_MODE_abg = 0x0001,    //11a/b/g only, no HT, no proprietary
    eCSR_DOT11_MODE_11a = 0x0002,
    eCSR_DOT11_MODE_11b = 0x0004,
    eCSR_DOT11_MODE_11g = 0x0008,
    eCSR_DOT11_MODE_11n = 0x0010,
    eCSR_DOT11_MODE_POLARIS = 0x0020,
    eCSR_DOT11_MODE_TITAN = 0x0040,
    eCSR_DOT11_MODE_11g_ONLY = 0x0080,
    eCSR_DOT11_MODE_11n_ONLY = 0x0100,
    eCSR_DOT11_MODE_TAURUS_ONLY = 0x0200,
    eCSR_DOT11_MODE_11b_ONLY = 0x0400,
    eCSR_DOT11_MODE_11a_ONLY = 0x0800,
    //This is for WIFI test. It is same as eWNIAPI_MAC_PROTOCOL_ALL except when it starts IBSS in 11B of 2.4GHz
    //It is for CSR internal use
    eCSR_DOT11_MODE_AUTO = 0x1000,

    eCSR_NUM_PHY_MODE = 16,     //specify the number of maximum bits for phyMode
}eCsrPhyMode;


typedef tANI_U8 tCsrBssid[WNI_CFG_BSSID_LEN];

typedef enum
{
    eCSR_BSS_TYPE_INFRASTRUCTURE,
    eCSR_BSS_TYPE_IBSS,           // an IBSS network we will NOT start
    eCSR_BSS_TYPE_START_IBSS,     // an IBSS network we will start if no partners detected.
    eCSR_BSS_TYPE_ANY,            // any BSS type (IBSS or Infrastructure).
}eCsrRoamBssType;



typedef enum {
    eCSR_SCAN_REQUEST_11D_SCAN = 1,
    eCSR_SCAN_REQUEST_FULL_SCAN,
    eCSR_SCAN_IDLE_MODE_SCAN,
    eCSR_SCAN_HO_BG_SCAN, // bg scan request in NRT & RT Handoff sub-states
    eCSR_SCAN_HO_PROBE_SCAN, // directed probe on an entry from the candidate list
    eCSR_SCAN_HO_NT_BG_SCAN, // bg scan request in NT  sub-state

}eCsrRequestType;

typedef enum {
    eCSR_SCAN_RESULT_GET = 0,
    eCSR_SCAN_RESULT_FLUSH = 1,     //to delete all cached scan results
}eCsrScanResultCmd;

typedef enum
{
    eCSR_SCAN_SUCCESS,
    eCSR_SCAN_FAILURE,
    eCSR_SCAN_ABORT,
}eCsrScanStatus;

#define CSR_SCAN_TIME_DEFAULT       0
#define CSR_VALUE_IGNORED           0xFFFFFFFF
#define CSR_RSN_PMKID_SIZE          16
#define CSR_MAX_PMKID_ALLOWED       16
#define CSR_WEP40_KEY_LEN       5
#define CSR_WEP104_KEY_LEN      13
#define CSR_TKIP_KEY_LEN        32
#define CSR_AES_KEY_LEN         16
#define CSR_MAX_KEY_LEN         ( CSR_TKIP_KEY_LEN )  //longest one is for TKIP
#define CSR_MAX_TX_POWER        ( WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX )

typedef struct tagCsrChannelInfo
{
    tANI_U8 numOfChannels;
    tANI_U8 *ChannelList;   //it will be an array of channels
}tCsrChannelInfo;

typedef struct tagCsrSSIDInfo
{
   tSirMacSSid     SSID;   
   tANI_BOOLEAN    handoffPermitted;
   tANI_BOOLEAN    ssidHidden;
}tCsrSSIDInfo;

typedef struct tagCsrSSIDs
{
    tANI_U32 numOfSSIDs;
    tCsrSSIDInfo *SSIDList;   //To be allocated for array of SSIDs
}tCsrSSIDs;

typedef struct tagCsrBSSIDs
{
    tANI_U32 numOfBSSIDs;
    tCsrBssid *bssid;
}tCsrBSSIDs;


typedef struct tagCsrScanRequest 
{
    tSirScanType scanType;
    tCsrBssid bssid;
    eCsrRoamBssType BSSType;
    tCsrSSIDs SSIDs;   
    tCsrChannelInfo ChannelInfo;
    tANI_U32 minChnTime;    //in units of milliseconds
    tANI_U32 maxChnTime;    //in units of milliseconds
    tANI_U32 restTime;      //in units of milliseconds  //ignored when not connected
    tANI_U32 uIEFieldLen;
    tANI_U8 *pIEField;
    eCsrRequestType requestType;    //11d scan or full scan
}tCsrScanRequest;

typedef struct tagCsrBGScanRequest
{
    tSirScanType scanType;
    tSirMacSSid SSID;
    tCsrChannelInfo ChannelInfo;
    tANI_U32 scanInterval;  //in units of milliseconds
    tANI_U32 minChnTime;    //in units of milliseconds
    tANI_U32 maxChnTime;    //in units of milliseconds
    tANI_U32 restTime;      //in units of milliseconds  //ignored when not connected
    tANI_U32 throughputImpact;      //specify whether BG scan cares about impacting throughput  //ignored when not connected
    tCsrBssid bssid;    //how to use it?? Apple
}tCsrBGScanRequest;


typedef struct tagCsrScanResultInfo
{
    //Carry the IEs for the current BSSDescription. A pointer to tDot11fBeaconIEs. Maybe NULL for start BSS.
    void *pvIes;
    //This member must be the last in the structure because the end of tSirBssDescription is an
    //    array with nonknown size at this time
    tSirBssDescription BssDescriptor;
}tCsrScanResultInfo;

typedef struct tagCsrEncryptionList
{

    tANI_U32 numEntries;
    eCsrEncryptionType encryptionType[eCSR_NUM_OF_ENCRYPT_TYPE];

}tCsrEncryptionList, *tpCsrEncryptionList;

typedef struct tagCsrAuthList
{
    tANI_U32 numEntries;
    eCsrAuthType authType[eCSR_NUM_OF_SUPPORT_AUTH_TYPE];
}tCsrAuthList, *tpCsrAuthList;

typedef struct tagCsrScanResultFilter
{
    tCsrBSSIDs BSSIDs;    //each bssid has a length of WNI_CFG_BSSID_LEN (6)
    tCsrSSIDs SSIDs;   
    tCsrChannelInfo ChannelInfo;
    tCsrAuthList authType;
    tCsrEncryptionList EncryptionType;
    //eCSR_ENCRYPT_TYPE_ANY cannot be set in multicast encryption type. If caller doesn't case, 
    //put all supported encryption types in here
    tCsrEncryptionList mcEncryptionType;
    eCsrRoamBssType BSSType;   
    //this is a bit mask of all the needed phy mode defined in eCsrPhyMode
    tANI_U32 phyMode;   
    //If countryCode[0] is not 0, countryCode is checked independent of fCheckUnknwonCountryCode
    tANI_U8 countryCode[WNI_CFG_COUNTRY_CODE_LEN]; 
    tANI_U8 uapsd_mask; 
    /*For WPS filtering if true => auth and ecryption should be ignored*/
    tANI_BOOLEAN bWPSAssociation;
}tCsrScanResultFilter;


typedef struct sCsrChnPower_
{
  tANI_U8 firstChannel;
  tANI_U8 numChannels;
  tANI_U8 maxtxPower;
}sCsrChnPower;


typedef struct sCsrChannel_
{
    tANI_U8 numChannels;
    tANI_U8 channelList[WNI_CFG_VALID_CHANNEL_LIST_LEN];
}sCsrChannel;


typedef struct tagCsr11dinfo
{
  sCsrChannel     Channels;
  tANI_U8         countryCode[WNI_CFG_COUNTRY_CODE_LEN+1];
  //max power channel list
  sCsrChnPower    ChnPower[WNI_CFG_VALID_CHANNEL_LIST_LEN];
}tCsr11dinfo;


typedef enum
{
    eCSR_ROAM_CANCELLED = 1,
    //this mean error happens before association_start or roaming_start is called.
    eCSR_ROAM_FAILED,   
    //a CSR trigger roaming operation starts, callback may get a pointer to tCsrConnectedPorfile
    eCSR_ROAM_ROAMING_START,    
    //a CSR trigger roaming operation is completed
    eCSR_ROAM_ROAMING_COMPLETION,   
    //Connection completed status.
    eCSR_ROAM_CONNECT_COMPLETION, 
    //an association or start_IBSS operation starts, 
    //callback may get a pointer to tCsrRoamProfile and a pointer to tSirBssDescription 
    eCSR_ROAM_ASSOCIATION_START,    
    //a roaming operation is finish, see eCsrRoamResult for 
    //possible data passed back
    eCSR_ROAM_ASSOCIATION_COMPLETION,   
    eCSR_ROAM_DISASSOCIATED,
    //when callback with this flag. callback gets a pointer to the BSS desc.
    eCSR_ROAM_SHOULD_ROAM,  
    //A new candidate for PMKID is found
    eCSR_ROAM_SCAN_FOUND_NEW_BSS,
    //CSR is done lostlink roaming and still cannot reconnect
    eCSR_ROAM_LOSTLINK,
    //a link lost is detected. CSR starts roaming.
    eCSR_ROAM_LOSTLINK_DETECTED,   
    //TKIP MIC error detected, callback gets a pointer to tpSirSmeMicFailureInd
    eCSR_ROAM_MIC_ERROR_IND,
    eCSR_ROAM_IBSS_IND, //IBSS indications.
    //Update the connection status, useful for IBSS: new peer added, network is active etc. 
    eCSR_ROAM_CONNECT_STATUS_UPDATE,  
    eCSR_ROAM_GEN_INFO,
    eCSR_ROAM_SET_KEY_COMPLETE,
    eCSR_ROAM_REMOVE_KEY_COMPLETE,
}eRoamCmdStatus;


//comment inside indicates what roaming callback gets
typedef enum
{
    eCSR_ROAM_RESULT_NONE,
    //this means no more action in CSR
    //If roamStatus is eCSR_ROAM_ASSOCIATION_COMPLETION, tCsrRoamInfo's pBssDesc may pass back
    eCSR_ROAM_RESULT_FAILURE,   
    //Pass back pointer to tCsrRoamInfo
    eCSR_ROAM_RESULT_ASSOCIATED,    
    eCSR_ROAM_RESULT_NOT_ASSOCIATED,
    eCSR_ROAM_RESULT_MIC_FAILURE,
    eCSR_ROAM_RESULT_FORCED,
    eCSR_ROAM_RESULT_DISASSOC_IND,
    eCSR_ROAM_RESULT_DEAUTH_IND,
    eCSR_ROAM_RESULT_CAP_CHANGED,
    //This means we starts an IBSS
    //tCsrRoamInfo's pBssDesc may pass back
    eCSR_ROAM_RESULT_IBSS_STARTED,  
    //START_BSS failed
    //tCsrRoamInfo's pBssDesc may pass back
    eCSR_ROAM_RESULT_IBSS_START_FAILED, 
    eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS,
    eCSR_ROAM_RESULT_IBSS_JOIN_FAILED, 
    eCSR_ROAM_RESULT_IBSS_CONNECT,
    eCSR_ROAM_RESULT_IBSS_INACTIVE,
    //If roamStatus is eCSR_ROAM_ASSOCIATION_COMPLETION
    //tCsrRoamInfo's pBssDesc may pass back. and the peer's MAC address in peerMacOrBssid 
    //If roamStatus is eCSR_ROAM_IBSS_IND,  
    //the peer's MAC address in peerMacOrBssid and a beacon frame of the IBSS in pbFrames
    eCSR_ROAM_RESULT_IBSS_NEW_PEER, 
    //Peer departed from IBSS, Callback may get a pointer tSmeIbssPeerInd in pIbssPeerInd
	eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED, 
    //Coalescing in the IBSS network (joined an IBSS network)
    //Callback pass a BSSID in peerMacOrBssid
    eCSR_ROAM_RESULT_IBSS_COALESCED,    
    //If roamStatus is eCSR_ROAM_ROAMING_START, callback may get a pointer to tCsrConnectedProfile used to connect.
    eCSR_ROAM_RESULT_LOSTLINK, 
    eCSR_ROAM_RESULT_MIC_ERROR_UNICAST,
    eCSR_ROAM_RESULT_MIC_ERROR_GROUP,
    eCSR_ROAM_RESULT_AUTHENTICATED,
}eCsrRoamResult;



/*----------------------------------------------------------------------------
  List of link quality indications HDD can receive from SME
-----------------------------------------------------------------------------*/
typedef enum
{
 eCSR_ROAM_LINK_QUAL_MIN_IND     = -1,

 eCSR_ROAM_LINK_QUAL_POOR_IND            =  0,   /* bad link                */
 eCSR_ROAM_LINK_QUAL_GOOD_IND            =  1,   /* acceptable for voice    */
 eCSR_ROAM_LINK_QUAL_VERY_GOOD_IND       =  2,   /* suitable for voice      */
 eCSR_ROAM_LINK_QUAL_EXCELLENT_IND       =  3,   /* suitable for voice      */

 eCSR_ROAM_LINK_QUAL_MAX_IND  /* invalid value */

} eCsrRoamLinkQualityInd;

typedef enum
{
    eCSR_DISCONNECT_REASON_UNSPECIFIED = 0,
    eCSR_DISCONNECT_REASON_MIC_ERROR,
    eCSR_DISCONNECT_REASON_DISASSOC,
    eCSR_DISCONNECT_REASON_DEAUTH,
	eCSR_DISCONNECT_REASON_HANDOFF,
    eCSR_DISCONNECT_REASON_IBSS_JOIN_FAILURE,
}eCsrRoamDisconnectReason;

typedef enum 
{
    // Not associated in Infra or participating in an IBSS / Ad-hoc network.
    eCSR_ASSOC_STATE_TYPE_NOT_CONNECTED,
    // Associated in an Infrastructure network.
    eCSR_ASSOC_STATE_TYPE_INFRA_ASSOCIATED,
    // Participating in an IBSS network though disconnected (no partner stations
    // in the IBSS).
    eCSR_ASSOC_STATE_TYPE_IBSS_DISCONNECTED,
    // Participating in an IBSS network with partner stations also present
    eCSR_ASSOC_STATE_TYPE_IBSS_CONNECTED,

}eCsrConnectState;


// This parameter is no longer supported in the Profile.  Need to set this in the global properties
// for the adapter.
typedef enum eCSR_MEDIUM_ACCESS 
{
    eCSR_MEDIUM_ACCESS_AUTO = 0,
    eCSR_MEDIUM_ACCESS_DCF,
    eCSR_MEDIUM_ACCESS_eDCF,
    eCSR_MEDIUM_ACCESS_HCF,

    eCSR_MEDIUM_ACCESS_WMM_eDCF_802dot1p,
    eCSR_MEDIUM_ACCESS_WMM_eDCF_DSCP,
    eCSR_MEDIUM_ACCESS_WMM_eDCF_NoClassify,
    eCSR_MEDIUM_ACCESS_11e_eDCF = eCSR_MEDIUM_ACCESS_eDCF,
    eCSR_MEDIUM_ACCESS_11e_HCF  = eCSR_MEDIUM_ACCESS_HCF,
}eCsrMediaAccessType;

typedef enum 
{
    eCSR_TX_RATE_AUTO = 0,   // use rate adaption to determine Tx rate.

    eCSR_TX_RATE_1Mbps   = 0x00000001,
    eCSR_TX_RATE_2Mbps   = 0x00000002,
    eCSR_TX_RATE_5_5Mbps = 0x00000004,
    eCSR_TX_RATE_6Mbps   = 0x00000008,
    eCSR_TX_RATE_9Mbps   = 0x00000010,
    eCSR_TX_RATE_11Mbps  = 0x00000020,
    eCSR_TX_RATE_12Mbps  = 0x00000040,
    eCSR_TX_RATE_18Mbps  = 0x00000080,
    eCSR_TX_RATE_24Mbps  = 0x00000100,
    eCSR_TX_RATE_36Mbps  = 0x00000200,
    eCSR_TX_RATE_42Mbps  = 0x00000400,
    eCSR_TX_RATE_48Mbps  = 0x00000800,
    eCSR_TX_RATE_54Mbps  = 0x00001000,
    eCSR_TX_RATE_72Mbps  = 0x00002000,
    eCSR_TX_RATE_84Mbps  = 0x00004000,
    eCSR_TX_RATE_96Mbps  = 0x00008000,
    eCSR_TX_RATE_108Mbps = 0x00010000,
    eCSR_TX_RATE_126Mbps = 0x00020000,
    eCSR_TX_RATE_144Mbps = 0x00040000,
    eCSR_TX_RATE_168Mbps = 0x00080000,
    eCSR_TX_RATE_192Mbps = 0x00100000,
    eCSR_TX_RATE_216Mbps = 0x00200000,
    eCSR_TX_RATE_240Mbps = 0x00400000,

}eCsrExposedTxRate;

typedef enum 
{
    eCSR_OPERATING_CHANNEL_ALL  = 0,
    eCSR_OPERATING_CHANNEL_AUTO = eCSR_OPERATING_CHANNEL_ALL,
    eCSR_OPERATING_CHANNEL_ANY  = eCSR_OPERATING_CHANNEL_ALL,
}eOperationChannel;

typedef enum 
{
    eCSR_DOT11_FRAG_THRESH_AUTO            = -1,
    eCSR_DOT11_FRAG_THRESH_MIN             = 256,
    eCSR_DOT11_FRAG_THRESH_MAX             = 2346,
    eCSR_DOT11_FRAG_THRESH_DEFAULT         = 2000
}eCsrDot11FragThresh;


//for channel bonding for ibss
typedef enum 
{
    eCSR_CB_OFF = 0,
    eCSR_CB_AUTO = 1,
    eCSR_CB_DOWN = 2,
    eCSR_CB_UP = 3,
}eCsrCBChoice;

//For channel bonding, the channel number gap is 4, either up or down. For both 11a and 11g mode.
#define CSR_CB_CHANNEL_GAP 4
#define CSR_CB_CENTER_CHANNEL_OFFSET    2
#define CSR_MAX_24GHz_CHANNEL_NUMBER ( SIR_11B_CHANNEL_END )

// WEP keysize (in bits)...
typedef enum  
{
    eCSR_SECURITY_WEP_KEYSIZE_40  =  40,   // 40 bit key + 24bit IV = 64bit WEP
    eCSR_SECURITY_WEP_KEYSIZE_104 = 104,   // 104bit key + 24bit IV = 128bit WEP

    eCSR_SECURITY_WEP_KEYSIZE_MIN = eCSR_SECURITY_WEP_KEYSIZE_40,
    eCSR_SECURITY_WEP_KEYSIZE_MAX = eCSR_SECURITY_WEP_KEYSIZE_104,
    eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES = ( eCSR_SECURITY_WEP_KEYSIZE_MAX / 8 ),
}eCsrWEPKeySize;


// Possible values for the WEP static key ID...
typedef enum
{

    eCSR_SECURITY_WEP_STATIC_KEY_ID_MIN       =  0,
    eCSR_SECURITY_WEP_STATIC_KEY_ID_MAX       =  3,
    eCSR_SECURITY_WEP_STATIC_KEY_ID_DEFAULT   =  0,

    eCSR_SECURITY_WEP_STATIC_KEY_ID_INVALID   = -1,

}eCsrWEPStaticKeyID;

#define CSR_MAX_NUM_KEY     (eCSR_SECURITY_WEP_STATIC_KEY_ID_MAX + 1)

typedef enum 
{
    eCSR_SECURITY_SET_KEY_ACTION_NO_CHANGE,
    eCSR_SECURITY_SET_KEY_ACTION_SET_KEY,
    eCSR_SECURITY_SET_KEY_ACTION_DELETE_KEY,
}eCsrSetKeyAction;

typedef enum
{
    eCSR_BAND_ALL,
    eCSR_BAND_24,
    eCSR_BAND_5G,
}eCsrBand;


typedef enum 
{
   // Roaming because HDD requested for reassoc by changing one of the fields in 
   // tCsrRoamModifyProfileFields. OR
   // Roaming because SME requested for reassoc by changing one of the fields in 
   // tCsrRoamModifyProfileFields.
   eCsrRoamReasonStaCapabilityChanged,
   // Roaming because SME requested for reassoc to a different AP, as part of 
   // inter AP handoff.
   eCsrRoamReasonBetterAP,
   // Roaming because SME requested it as the link is lost - placeholder, will 
   // clean it up once handoff code gets in
   eCsrRoamReasonSmeIssuedForLostLink,

}eCsrRoamReasonCodes;

typedef enum
{
   eCsrRoamWmmAuto = 0,
   eCsrRoamWmmQbssOnly = 1,
   eCsrRoamWmmNoQos = 2,

} eCsrRoamWmmUserModeType;

typedef enum
{
   eCSR_REQUESTER_MIN = 0,
   eCSR_DIAG,
   eCSR_UMA_GAN,
   eCSR_HDD
} eCsrStatsRequesterType;

typedef struct tagPmkidCandidateInfo
{
    tCsrBssid BSSID;
    tANI_BOOLEAN preAuthSupported;
}tPmkidCandidateInfo;

typedef struct tagPmkidCacheInfo
{
    tCsrBssid BSSID;
    tANI_U8 PMKID[CSR_RSN_PMKID_SIZE];
}tPmkidCacheInfo;


typedef struct tagCsrKeys
{
    tANI_U8 KeyLength[ CSR_MAX_NUM_KEY ];   //Also use to indicate whether the key index is set
    tANI_U8 KeyMaterial[ CSR_MAX_NUM_KEY ][ eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES ];
    tANI_U8 defaultIndex;
}tCsrKeys;

/* Following are fields which are part of tCsrRoamConnectedProfile might need 
   modification dynamically once STA is up & running and this could trigger
   reassoc */
typedef struct tagCsrRoamModifyProfileFields
{
   // during connect this specifies ACs U-APSD is to be setup 
   //   for (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored).
   //  During assoc response this COULD carry confirmation of what ACs U-APSD 
   // got setup for. Later if an APP looking for APSD, SME-QoS might need to
   // modify this field
   tANI_U8     uapsd_mask;
   // HDD might ask to modify this field
   tANI_U16    listen_interval;
}tCsrRoamModifyProfileFields;


typedef struct tagCsrRoamProfile
{
    tCsrSSIDs SSIDs;
    tCsrBSSIDs BSSIDs;
    tANI_U32 phyMode;   //this is a bit mask of all the needed phy mode defined in eCsrPhyMode
    eCsrRoamBssType BSSType;

    tCsrAuthList AuthType;
    eCsrAuthType negotiatedAuthType;

    tCsrEncryptionList EncryptionType;
    //This field is for output only, not for input
    eCsrEncryptionType negotiatedUCEncryptionType;

    //eCSR_ENCRYPT_TYPE_ANY cannot be set in multicast encryption type. If caller doesn't case, 
    //put all supported encryption types in here
    tCsrEncryptionList mcEncryptionType;
    //This field is for output only, not for input
    eCsrEncryptionType negotiatedMCEncryptionType;  

    tCsrKeys Keys;
    eCsrCBChoice CBMode; //up, down or auto
    tCsrChannelInfo ChannelInfo;
    // during connect this specifies ACs U-APSD is to be setup 
    //   for (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored).
    //  During assoc response this COULD carry confirmation of what ACs U-APSD got setup for
    tANI_U8 uapsd_mask; 
    tANI_U32 nWPAReqIELength;   //The byte count in the pWPAReqIE
    tANI_U8 *pWPAReqIE;   //If not null, it has the IE byte stream for WPA
    tANI_U32 nRSNReqIELength;  //The byte count in the pRSNReqIE
    tANI_U8 *pRSNReqIE;     //If not null, it has the IE byte stream for RSN
    tANI_U8 countryCode[WNI_CFG_COUNTRY_CODE_LEN];  //it is ignored if [0] is 0.
    /*WPS Association if true => auth and ecryption should be ignored*/
    tANI_BOOLEAN bWPSAssociation;
}tCsrRoamProfile;


typedef struct tagCsrRoamConnectedProfile
{
    tSirMacSSid SSID;
    tANI_BOOLEAN    handoffPermitted;
    tANI_BOOLEAN    ssidHidden;
    tCsrBssid bssid;
    eCsrRoamBssType BSSType;
    eCsrAuthType AuthType;
	tCsrAuthList AuthInfo;
    eCsrEncryptionType EncryptionType;
	tCsrEncryptionList EncryptionInfo;
    eCsrEncryptionType mcEncryptionType;
	tCsrEncryptionList mcEncryptionInfo;
    eCsrCBChoice CBMode; //up, down or auto
    tANI_U8 operationChannel;
    tCsrKeys Keys;
    // meaningless on connect. It's an OUT param from CSR's point of view
    // During assoc response carries the ACM bit-mask i.e. what
    // ACs have ACM=1 (if any), 
    // (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored)
    tANI_U8  acm_mask;
    tCsrRoamModifyProfileFields modifyProfileFields;
    tSirBssDescription *pBssDesc;   
    tANI_BOOLEAN   qap; //AP supports QoS
    tANI_BOOLEAN   qosConnection; //A connection is QoS enabled
}tCsrRoamConnectedProfile;

#ifdef FEATURE_WLAN_GEN6_ROAMING
/*---------------------------------------------------------------------------
 All tunable handoff parmeters which will be part of handoff config file
---------------------------------------------------------------------------*/
typedef struct tagCsrConfigNoWifiParams
{
   tANI_U32          rssiFilterConst;
   tANI_U32          channelScanTime;
   tANI_U32          rssiThresholdNeighborSet;
   tANI_U32          rssiThresholdAssociationAdd;
   tANI_U32          activeScanInterval;
   tANI_U32          activeScanDuration;
} tCsrConfigNoWifiParams;

typedef struct tagCsrConfigIdleParams
{
  tANI_U32          rssiFilterConst;
  tANI_U32          numCandtSetEntry;
  tANI_U32          inactThreshold;
  tANI_U32          inactPeriod;
  tANI_U32          bestCandidateApRssiDelta;
  tANI_U32          neighborApBgScanInterval;
  tANI_U32          neighborApIncrBgScanInterval;
  tANI_U32          rssiThresholdCandtSet;
  tANI_U32          pmkCacheRssiDelta;
  tANI_U32          rssiThresholdCurrentApGood;
} tCsrConfigNtParams;

typedef struct tagCsrConfigNrtParams
{
  tANI_U32          rssiFilterConst;
  tANI_U32          numCandtSetEntry;
  tANI_U32          rssiThresholdCurrentApGood;
  tANI_U32          rssiThresholdCurrentApGoodEmptyCandtset;
  tANI_U32          rssiThresholdHoFromCurrentAp;
  tANI_U32          rssiThresholdCandtSet;
  tANI_U32          bgScanInterval;
  tANI_U32          bgScanIncrInterval;
  tANI_U32          bgScanDelayInterval;
  tANI_U32          perMsmtInterval;
  tANI_U32          perThresholdHoFromCurrentAp;
  tANI_U32          pmkCacheRssiDelta;
  tANI_U32          bestCandidateApRssiDelta;
} tCsrConfigNrtParams;

typedef struct tagCsrConfigRtParams
{
  tANI_U32          rssiFilterConst;
  tANI_U32          numCandtSetEntry;
  tANI_U32          rssiThresholdCurrentApGood;
  tANI_U32          rssiThresholdHoFromCurrentAp;
  tANI_U32          rssiThresholdCandtSet;
  tANI_U32          bgScanInterval;
  tANI_U32          perMsmtInterval;
  tANI_U32          perThresholdHoFromCurrentAp;
  tANI_U32          pmkCacheRssiDelta;
  tANI_U32          bestCandidateApRssiDelta;
} tCsrConfigRtParams;

/*---------------------------------------------------------------------------
  Structure with all the handoff(WLAN) related configuration parameters
---------------------------------------------------------------------------*/
typedef struct tagCsrHandoffConfigParams
{
  tCsrConfigNoWifiParams    noWifiParams;
  tCsrConfigNtParams        ntParams;
  tCsrConfigNrtParams       nrtParams;
  tCsrConfigRtParams        rtParams;
} tCsrHandoffConfigParams;
#endif

typedef struct tagCsrConfigParam
{
    tANI_U32 FragmentationThreshold;
    tANI_U32 ChannelBondingMode;
    eCsrPhyMode phyMode;
    eCsrBand eBand;
    tANI_U32 RTSThreshold;
    tANI_U32 HeartbeatThresh50;
    tANI_U32 HeartbeatThresh24;
    eCsrCBChoice cbChoice;
    eCsrBand bandCapability;     //indicate hw capability
    tANI_U32 bgScanInterval;
    tANI_U16 TxRate;
    eCsrRoamWmmUserModeType WMMSupportMode;
    tANI_BOOLEAN Is11eSupportEnabled;
    tANI_BOOLEAN Is11dSupportEnabled;
    tANI_BOOLEAN Is11hSupportEnabled;
    tANI_BOOLEAN shortSlotTime;
    tANI_BOOLEAN ProprietaryRatesEnabled;
    tANI_U8 AdHocChannel24;
    tANI_U8 AdHocChannel5G;
    tANI_U32 impsSleepTime;     //in units of seconds
    tANI_U32 nScanResultAgeCount;   //this number minus one is the number of times a scan doesn't find it before it is removed
    tANI_U32 scanAgeTimeNCNPS;  //scan result aging time threshold when Not-Connect-No-Power-Save, in seconds
    tANI_U32 scanAgeTimeNCPS;   //scan result aging time threshold when Not-Connect-Power-Save, in seconds
    tANI_U32 scanAgeTimeCNPS;   //scan result aging time threshold when Connect-No-Power-Save, in seconds,
    tANI_U32 scanAgeTimeCPS;   //scan result aging time threshold when Connect-Power-Savein seconds
    tANI_U32 nRoamingTime;  //In seconds, CSR will try this long before gives up. 0 means no roaming
    tANI_U8 bCatRssiOffset;     //to set the RSSI difference for each category

    tCsr11dinfo  Csr11dinfo;
    //Whether to limit the channels to the ones set in Csr11dInfo. If true, the opertaional
    //channels are limited to the default channel list. It is an "AND" operation between the 
    //default channels and the channels in the 802.11d IE.
    tANI_BOOLEAN fEnforce11dChannels;   
    //When true, AP with unknown country code won't be see. 
    //"Unknown country code" means either Ap doesn't have 11d IE or we cannot 
    //find a domain for the country code in its 11d IE. 
    tANI_BOOLEAN fEnforceCountryCodeMatch;  
    //When true, only APs in the default domain can be seen. If the Ap has "unknown country
    //code", or the doamin of the country code doesn't match the default domain, the Ap is
    //not acceptable.
    tANI_BOOLEAN fEnforceDefaultDomain;     

    tANI_U16 vccRssiThreshold;
    tANI_U32 vccUlMacLossThreshold;

    tANI_U32  nPassiveMinChnTime;    //in units of milliseconds
    tANI_U32  nPassiveMaxChnTime;    //in units of milliseconds
    tANI_U32  nActiveMinChnTime;     //in units of milliseconds
    tANI_U32  nActiveMaxChnTime;     //in units of milliseconds

    tANI_BOOLEAN IsIdleScanEnabled;
    //in dBm, the maximum TX power
    //The actual TX power is the lesser of this value and 11d. 
    //If 11d is disable, the lesser of this and default setting.
    tANI_U8 nTxPowerCap;     
    tANI_U32  statsReqPeriodicity;  //stats request frequency from PE while in full power
    tANI_U32  statsReqPeriodicityInPS;//stats request frequency from PE while in power save
}tCsrConfigParam;   

//Tush
typedef struct tagCsrUpdateConfigParam
{
   tCsr11dinfo  Csr11dinfo;
}tCsrUpdateConfigParam;

typedef struct tagCsrRoamInfo
{
    tCsrRoamProfile *pProfile;  //may be NULL
    tSirBssDescription *pBssDesc;  //May be NULL
    tANI_U32 nBeaconLength; //the length, in bytes, of the beacon frame, can be 0
    tANI_U32 nAssocReqLength;   //the length, in bytes, of the assoc req frame, can be 0
    tANI_U32 nAssocRspLength;   //The length, in bytes, of the assoc rsp frame, can be 0
    tANI_U8 *pbFrames;  //Point to a buffer contain the beacon, assoc req, assoc rsp frame, in that order
                        //user needs to use nBeaconLength, nAssocReqLength, nAssocRspLength to desice where
                        //each frame starts and ends.
    tANI_BOOLEAN fReassocReq;   //set to true if for re-association
    tANI_BOOLEAN fReassocRsp;   //set to true if for re-association
    tCsrBssid bssid;
    //Only valid in IBSS 
    //this is the peers MAC address for eCSR_ROAM_RESULT_IBSS_NEW_PEER or PEER_DEPARTED
    tCsrBssid peerMac;  
    tSirResultCodes statusCode;
    tANI_U32 reasonCode;    //this could be our own defined or sent from the other BSS(per 802.11 spec)
    tANI_U8  staId;         // Peer stationId when connected
    /*The DPU signatures will be sent eventually to TL to help it determine the 
      association to which a packet belongs to*/
    /*Unicast DPU signature*/
    tANI_U8            ucastSig;

    /*Broadcast DPU signature*/
    tANI_U8            bcastSig;

    tANI_BOOLEAN fAuthRequired;   //FALSE means auth needed from supplicant. TRUE means authenticated(static WEP, open)

    union
    {
        tSirMicFailureInfo *pMICFailureInfo;
        tCsrRoamConnectedProfile *pConnectedProfile;
    } u;
}tCsrRoamInfo;





typedef struct tagCsrFreqScanInfo
{
    tANI_U32 nStartFreq;    //in unit of MHz
    tANI_U32 nEndFreq;      //in unit of MHz
    tSirScanType scanType;
}tCsrFreqScanInfo;



typedef struct tagCsrSummaryStatsInfo
{
   tANI_U32 retry_cnt[4];
   tANI_U32 multiple_retry_cnt[4];
   tANI_U32 tx_frm_cnt[4];
   //tANI_U32 num_rx_frm_crc_err; same as rx_error_cnt
   //tANI_U32 num_rx_frm_crc_ok; same as rx_frm_cnt
   tANI_U32 rx_frm_cnt;
   tANI_U32 frm_dup_cnt;
   tANI_U32 fail_cnt[4];
   tANI_U32 rts_fail_cnt;
   tANI_U32 ack_fail_cnt;
   tANI_U32 rts_succ_cnt;
   tANI_U32 rx_discard_cnt;
   tANI_U32 rx_error_cnt;
   tANI_U32 tx_byte_cnt;

}tCsrSummaryStatsInfo;

typedef struct tagCsrGlobalClassAStatsInfo
{
   tANI_U32 rx_frag_cnt;
   tANI_U32 promiscuous_rx_frag_cnt;
   //tANI_U32 rx_fcs_err;
   tANI_U32 rx_input_sensitivity;
   tANI_U32 max_pwr;
   //tANI_U32 default_pwr;
   tANI_U32 sync_fail_cnt;
   tANI_U32 tx_rate;

}tCsrGlobalClassAStatsInfo;

typedef struct tagCsrGlobalClassBStatsInfo
{
   tANI_U32 uc_rx_wep_unencrypted_frm_cnt;
   tANI_U32 uc_rx_mic_fail_cnt;
   tANI_U32 uc_tkip_icv_err;
   tANI_U32 uc_aes_ccmp_format_err;
   tANI_U32 uc_aes_ccmp_replay_cnt;
   tANI_U32 uc_aes_ccmp_decrpt_err;
   tANI_U32 uc_wep_undecryptable_cnt;
   tANI_U32 uc_wep_icv_err;
   tANI_U32 uc_rx_decrypt_succ_cnt;
   tANI_U32 uc_rx_decrypt_fail_cnt;
   tANI_U32 mcbc_rx_wep_unencrypted_frm_cnt;
   tANI_U32 mcbc_rx_mic_fail_cnt;
   tANI_U32 mcbc_tkip_icv_err;
   tANI_U32 mcbc_aes_ccmp_format_err;
   tANI_U32 mcbc_aes_ccmp_replay_cnt;
   tANI_U32 mcbc_aes_ccmp_decrpt_err;
   tANI_U32 mcbc_wep_undecryptable_cnt;
   tANI_U32 mcbc_wep_icv_err;
   tANI_U32 mcbc_rx_decrypt_succ_cnt;
   tANI_U32 mcbc_rx_decrypt_fail_cnt;

}tCsrGlobalClassBStatsInfo;

typedef struct tagCsrGlobalClassCStatsInfo
{
   tANI_U32 rx_amsdu_cnt;
   tANI_U32 rx_ampdu_cnt;
   tANI_U32 tx_20_frm_cnt;
   tANI_U32 rx_20_frm_cnt;
   tANI_U32 rx_mpdu_in_ampdu_cnt;
   tANI_U32 ampdu_delimiter_crc_err;

}tCsrGlobalClassCStatsInfo;

typedef struct tagCsrGlobalClassDStatsInfo
{
   tANI_U32 tx_uc_frm_cnt;
   tANI_U32 tx_mc_frm_cnt;
   tANI_U32 tx_bc_frm_cnt;
   tANI_U32 rx_uc_frm_cnt;
   tANI_U32 rx_mc_frm_cnt;
   tANI_U32 rx_bc_frm_cnt;
   tANI_U32 tx_uc_byte_cnt[4];
   tANI_U32 tx_mc_byte_cnt;
   tANI_U32 tx_bc_byte_cnt;
   tANI_U32 rx_uc_byte_cnt[4];
   tANI_U32 rx_mc_byte_cnt;
   tANI_U32 rx_bc_byte_cnt;
   tANI_U32 rx_byte_cnt;
   tANI_U32 num_rx_bytes_crc_ok;
   tANI_U32 rx_rate;

}tCsrGlobalClassDStatsInfo;

typedef struct tagCsrPerStaStatsInfo
{
   tANI_U32 tx_frag_cnt[4];
   tANI_U32 tx_ampdu_cnt;
   tANI_U32 tx_mpdu_in_ampdu_cnt;
} tCsrPerStaStatsInfo;

typedef struct tagCsrRoamSetKey
{
    eCsrEncryptionType encType;
    tAniKeyDirection keyDirection;    //Tx, Rx or Tx-and-Rx
    tCsrBssid peerMac;   //Peer’s MAC address. ALL 1's for group key
    tANI_U8 paeRole;      //0 for supplicant
    tANI_U8 keyId;  // Kye index
    tANI_U16 keyLength;  //Number of bytes containing the key in pKey
    tANI_U8 Key[CSR_MAX_KEY_LEN];
} tCsrRoamSetKey;

typedef struct tagCsrRoamRemoveKey
{
    eCsrEncryptionType encType;
    tCsrBssid peerMac;   //Peer’s MAC address. ALL 1's for group key
    tANI_U8 keyId;  //key index
} tCsrRoamRemoveKey;


typedef void * tScanResultHandle;

#define CSR_INVALID_SCANRESULT_HANDLE       (NULL)



////////////////////////////////////////////Common SCAN starts

//void *p2 -- the second context pass in for the caller
//***what if callback is called before requester gets the scanId??
typedef eHalStatus (*csrScanCompleteCallback)(tHalHandle, void *p2, tANI_U32 scanID, eCsrScanStatus status);   



/* ---------------------------------------------------------------------------
    \fn csrScanEnable
    \brief Enable the scanning feature of CSR. It must be called before any scan request can be performed.
    \param tHalHandle - HAL context handle
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanEnable(tHalHandle);

/* ---------------------------------------------------------------------------
    \fn csrScanDisable
    \brief Disableing the scanning feature of CSR. After this function return success, no scan is performed until 
a successfull to csrScanEnable
    \param tHalHandle - HAL context handle
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanDisable(tHalHandle);

/* ---------------------------------------------------------------------------
    \fn csrScanRequest
    \brief Request a 11d or full scan.
    \param pScanRequestID - pointer to an object to get back the request ID
    \param callback - a callback function that scan calls upon finish, will not be called if csrScanRequest returns error
    \param pContext - a pointer passed in for the callback
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanRequest(tHalHandle, tCsrScanRequest *, tANI_U32 *pScanRequestID, 
                            csrScanCompleteCallback callback, void *pContext);

/* ---------------------------------------------------------------------------
    \fn csrScanAbort
    \brief If a scan request is abort, the scan complete callback will be called first before csrScanAbort returns.
    \param pScanRequestID - The request ID returned from csrScanRequest
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanAbort(tHalHandle, tANI_U32 scanRequestID);

eHalStatus csrScanSetBGScanparams(tHalHandle, tCsrBGScanRequest *);
eHalStatus csrScanBGScanAbort(tHalHandle);

/* ---------------------------------------------------------------------------
    \fn csrScanGetResult
    \brief Return scan results.
    \param pFilter - If pFilter is NULL, all cached results are returned
    \param phResult - an object for the result.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanGetResult(tHalHandle, tCsrScanResultFilter *pFilter, tScanResultHandle *phResult);

/* ---------------------------------------------------------------------------
    \fn csrScanFlushResult
    \brief Clear scan results.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanFlushResult(tHalHandle);

/* ---------------------------------------------------------------------------
    \fn csrScanBGScanGetParam
    \brief Returns the current background scan settings.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanBGScanGetParam(tHalHandle, tCsrBGScanRequest *);

/* ---------------------------------------------------------------------------
    \fn csrScanResultGetFirst
    \brief Returns the first element of scan result.
    \param hScanResult - returned from csrScanGetResult
    \return tCsrScanResultInfo * - NULL if no result     
  -------------------------------------------------------------------------------*/
tCsrScanResultInfo *csrScanResultGetFirst(tHalHandle, tScanResultHandle hScanResult);
/* ---------------------------------------------------------------------------
    \fn csrScanResultGetNext
    \brief Returns the next element of scan result. It can be called without calling csrScanResultGetFirst first
    \param hScanResult - returned from csrScanGetResult
    \return Null if no result or reach the end     
  -------------------------------------------------------------------------------*/
tCsrScanResultInfo *csrScanResultGetNext(tHalHandle, tScanResultHandle hScanResult);

/* ---------------------------------------------------------------------------
    \fn csrScanResultPurge
    \brief remove all items(tCsrScanResult) in the list and free memory for each item
    \param hScanResult - returned from csrScanGetResult. hScanResult is considered gone by 
    calling this function and even before this function reutrns.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrScanResultPurge(tHalHandle hHal, tScanResultHandle hScanResult);

/* ---------------------------------------------------------------------------
    \fn csrScanGetPMKIDCandidateList
    \brief return the PMKID candidate list
    \param pPmkidList - caller allocated buffer point to an array of tPmkidCandidateInfo
    \param pNumItems - pointer to a variable that has the number of tPmkidCandidateInfo allocated
    when retruning, this is either the number needed or number of items put into pPmkidList
    \return eHalStatus - when fail, it usually means the buffer allocated is not big enough and pNumItems
    has the number of tPmkidCandidateInfo.
    \Note: pNumItems is a number of tPmkidCandidateInfo, not sizeof(tPmkidCandidateInfo) * something
  -------------------------------------------------------------------------------*/
eHalStatus csrScanGetPMKIDCandidateList(tHalHandle hHal, tPmkidCandidateInfo *pPmkidList, tANI_U32 *pNumItems );

///////////////////////////////////////////Common Scan ends

///////////////////////////////////////////Common Roam starts

//pContext is the pContext passed in with the roam request
//pParam is a pointer to a tCsrRoamInfo, see definition of eRoamCmdStatus and
//   eRoamCmdResult for detail valid members. It may be NULL
//roamId is to identify the callback related roam request. 0 means unsolicit
//roamStatus is a flag indicating the status of the callback
//roamResult is the result
typedef eHalStatus (*csrRoamCompleteCallback)(void *pContext, tCsrRoamInfo *pParam, tANI_U32 roamId, 
                                              eRoamCmdStatus roamStatus, eCsrRoamResult roamResult);

/* ---------------------------------------------------------------------------
    \fn csrRoamRegisterCallback
    \brief HDD can register a callback. Unlike scan, roam has one callback for all the roam requests
    \param callback - a callback function that roam calls upon when state changes
    \param pContext - a pointer passed in for the callback
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamRegisterCallback(tHalHandle hHal, csrRoamCompleteCallback callback, void *pContext);
/* ---------------------------------------------------------------------------
    \fn csrRoamConnect
    \brief To inititiate an association
    \param pProfile - can be NULL to join to any open ones
    \param hBssListIn - a list of BSS descriptor to roam to. It is returned from csrScanGetResult
    \param pRoamId - to get back the request ID
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamConnect(tHalHandle hHal, tCsrRoamProfile *pProfile, tScanResultHandle hBssListIn, tANI_U32 *pRoamId);

/* ---------------------------------------------------------------------------
    \fn csrRoamReconnect
    \brief To disconnect and reconnect with the same profile
    \return eHalStatus. It returns fail if currently not connected     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamReconnect(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrRoamConnectToLastProfile
    \brief To disconnect and reconnect with the same profile
    \return eHalStatus. It returns fail if currently connected     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamConnectToLastProfile(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrRoamDisconnect
    \brief To disconnect from a network
    \param reason -- To indicate the reason for disconnecting. Currently, only eCSR_DISCONNECT_REASON_MIC_ERROR is meanful.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamDisconnect(tHalHandle hHal, eCsrRoamDisconnectReason reason);

/* ---------------------------------------------------------------------------
    \fn csrRoamGetConnectState
    \brief To return the current connect state of Roaming
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamGetConnectState(tHalHandle hHal, eCsrConnectState *pState);

/* ---------------------------------------------------------------------------
    \fn csrRoamGetConnectProfile
    \brief To return the current connect profile. Caller must call csrRoamFreeConnectProfile
           after it is done and before reuse for another csrRoamGetConnectProfile call.
    \param pProfile - pointer to a caller allocated structure tCsrRoamConnectedProfile
    \return eHalStatus. Failure if not connected     
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamGetConnectProfile(tHalHandle hHal, tCsrRoamConnectedProfile *pProfile);

/* ---------------------------------------------------------------------------
    \fn csrRoamFreeConnectProfile
    \brief To free and reinitialize the profile return previous by csrRoamGetConnectProfile.
    \param pProfile - pointer to a caller allocated structure tCsrRoamConnectedProfile
    \return eHalStatus.      
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamFreeConnectProfile(tHalHandle hHal, tCsrRoamConnectedProfile *pProfile);

/* ---------------------------------------------------------------------------
    \fn csrRoamSetPMKIDCache
    \brief return the PMKID candidate list
    \param pPMKIDCache - caller allocated buffer point to an array of tPmkidCacheInfo
    \param numItems - a variable that has the number of tPmkidCacheInfo allocated
    when retruning, this is either the number needed or number of items put into pPMKIDCache
    \return eHalStatus - when fail, it usually means the buffer allocated is not big enough and pNumItems
    has the number of tPmkidCacheInfo.
    \Note: pNumItems is a number of tPmkidCacheInfo, not sizeof(tPmkidCacheInfo) * something
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamSetPMKIDCache( tHalHandle hHal, tPmkidCacheInfo *pPMKIDCache, tANI_U32 numItems );

/* ---------------------------------------------------------------------------
    \fn csrRoamGetWpaRsnReqIE
    \brief return the WPA or RSN IE CSR passes to PE to JOIN request or START_BSS request
    \param pLen - caller allocated memory that has the length of pBuf as input. Upon returned, *pLen has the 
    needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any, upon return
    \return eHalStatus - when fail, it usually means the buffer allocated is not big enough
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamGetWpaRsnReqIE(tHalHandle hHal, tANI_U32 *pLen, tANI_U8 *pBuf);

/* ---------------------------------------------------------------------------
    \fn csrRoamGetWpaRsnRspIE
    \brief return the WPA or RSN IE from the beacon or probe rsp if connected
    \param pLen - caller allocated memory that has the length of pBuf as input. Upon returned, *pLen has the 
    needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any, upon return
    \return eHalStatus - when fail, it usually means the buffer allocated is not big enough
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamGetWpaRsnRspIE(tHalHandle hHal, tANI_U32 *pLen, tANI_U8 *pBuf);


/* ---------------------------------------------------------------------------
    \fn csrRoamGetNumPMKIDCache
    \brief return number of PMKID cache entries
    \return tANI_U32 - the number of PMKID cache entries
  -------------------------------------------------------------------------------*/
tANI_U32 csrRoamGetNumPMKIDCache(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrRoamGetPMKIDCache
    \brief return PMKID cache from CSR
    \param pNum - caller allocated memory that has the space of the number of pBuf tPmkidCacheInfo as input. Upon returned, *pNum has the 
    needed or actually number in tPmkidCacheInfo.
    \param pPmkidCache - Caller allocated memory that contains PMKID cache, if any, upon return
    \return eHalStatus - when fail, it usually means the buffer allocated is not big enough
  -------------------------------------------------------------------------------*/
eHalStatus csrRoamGetPMKIDCache(tHalHandle hHal, tANI_U32 *pNum, tPmkidCacheInfo *pPmkidCache);

//pProfile - pointer to tCsrRoamProfile
#define CSR_IS_START_IBSS(pProfile) (eCSR_BSS_TYPE_START_IBSS == (pProfile)->BSSType)
#define CSR_IS_JOIN_TO_IBSS(pProfile) (eCSR_BSS_TYPE_IBSS == (pProfile)->BSSType)
#define CSR_IS_INFRASTRUCTURE(pProfile) (eCSR_BSS_TYPE_INFRASTRUCTURE == (pProfile)->BSSType)
#define CSR_IS_ANY_BSS_TYPE(pProfile) (eCSR_BSS_TYPE_ANY == (pProfile)->BSSType)



///////////////////////////////////////////Common Roam ends


/* ---------------------------------------------------------------------------
    \fn csrOpen
    \brief This function must be called before any API call to CSR. 
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrOpen(tHalHandle hHal);
/* ---------------------------------------------------------------------------
    \fn csrClose
    \brief To close down CSR module. There should not be any API call into CSR after calling this function.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrClose(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrStart
    \brief To start CSR.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrStart(tHalHandle hHal);
/* ---------------------------------------------------------------------------
    \fn csrStop
    \brief To stop CSR. CSR still keeps its current setting.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrStop(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrReady
    \brief To let CSR is ready to operate
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrReady(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn csrMsgProcessor
    \brief HDD calls this function for the messages that are handled by CSR.
    \param pMsgBuf - a pointer to a buffer that maps to various structures base on the message type.
    The beginning of the buffer can always map to tSirSmeRsp.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrMsgProcessor( tHalHandle hHal,  void *pMsgBuf );

/* ---------------------------------------------------------------------------
    \fn csrMsgProcessor
    \brief HDD calls this function to change some global settings. 
    caller must set the all fields or call csrGetConfigParam to prefill the fields.
    \param pParam - caller allocated memory
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrChangeDefaultConfigParam(tHalHandle hHal, tCsrConfigParam *pParam);

/* ---------------------------------------------------------------------------
    \fn csrGetConfigParam
    \brief HDD calls this function to get the global settings currently maintained by CSR. 
    \param pParam - caller allocated memory
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrGetConfigParam(tHalHandle hHal, tCsrConfigParam *pParam);



/* ---------------------------------------------------------------------------
    \fn csrInitChannelList
    \brief HDD calls this function to set the WNI_CFG_VALID_CHANNEL_LIST base on the band/mode settings.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrInitChannelList( tHalHandle hHal );

//Tush
/* ---------------------------------------------------------------------------
    \fn csrChangeConfigParams
    \brief The CSR API exposed for HDD to provide config params to CSR during 
    SME’s stop -> start sequence.
    If HDD changed the domain that will cause a reset. This function will 
    provide the new set of 11d information for the new domain. Currrently this
    API provides info regarding 11d only at reset but we can extend this for
    other params (PMC, QoS) which needs to be initialized again at reset.
    \param 
    hHal - Handle to the HAL. The HAL handle is returned by the HAL after it is 
           opened (by calling halOpen).
    pUpdateConfigParam - a pointer to a structure (tCsrUpdateConfigParam) that 
                currently provides 11d related information like Country code, 
                Regulatory domain, valid channel list, Tx power per channel, a 
                list with active/passive scan allowed per valid channel. 

    \return eHalStatus     
  ---------------------------------------------------------------------------*/
eHalStatus csrChangeConfigParams(tHalHandle hHal, 
                                 tCsrUpdateConfigParam *pUpdateConfigParam);


//enum to string conversion for debug output
const char * get_eRoamCmdStatus_str(eRoamCmdStatus val);
const char * get_eCsrRoamResult_str(eCsrRoamResult val);
/* ---------------------------------------------------------------------------
    \fn csrSetPhyMode
    \brief HDD calls this function to set the phyMode.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \param phyMode - indicate the phyMode needs to set to. The value has to be either 0, or some bits set. 
    See eCsrPhyMode for definition
    \param eBand - specify the operational band (2.4, 5 or both)
    \param pfRestartNeeded - pointer to a caller allocated space. Upon successful return, it indicates whether 
    a restart is needed to apply the change
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus csrSetPhyMode(tHalHandle hHal, tANI_U32 phyMode, eCsrBand eBand, tANI_BOOLEAN *pfRestartNeeded);

void csrDumpInit(tHalHandle hHal);


/*---------------------------------------------------------------------------
  This is the type for a link quality callback to be registered with SME
  for indications
  Once the link quality has been indicated, subsequently, link indications are 
  posted each time there is a CHANGE in link quality.
  *** If there is no change in link, there will be no indication ***

  The indications may be based on one or more criteria internal to SME
  such as RSSI and PER.

  \param ind - Indication being posted
  \param pContext - any user data given at callback registration.  
  \return None
  
---------------------------------------------------------------------------*/
typedef void (* csrRoamLinkQualityIndCallback)
             (eCsrRoamLinkQualityInd  ind, void *pContext);

/*----------------------------------------------------------------------------
  \fn csrRoamRegisterLinkQualityIndCallback

  \brief
  a CSR function to allow HDD to register a callback handler with CSR for 
  link quality indications. 

  Only one callback may be registered at any time.
  In order to deregister the callback, a NULL cback may be provided.

  Registration happens in the task context of the caller.

  \param callback - Call back being registered
  \param pContext - user data
  
  DEPENDENCIES: After CSR open

  \return eHalStatus  
-----------------------------------------------------------------------------*/
eHalStatus csrRoamRegisterLinkQualityIndCallback(tHalHandle hHal,
                                                 csrRoamLinkQualityIndCallback   callback,  
                                                 void                           *pContext);

/*---------------------------------------------------------------------------
  This is the type for a statistics callback to be registered with SME
  for stats reporting

  Since the client requesting for the stats already know which class/type of 
  stats it asked for, the callback will carry them in the rsp buffer 
  (void * stats) whose size will be same as the size of requested stats & 
  will be exactly in the same order requested in the stats mask from LSB to MSB

  \param stats - stats rsp buffer sent back with the report
  \param pContext - any user data given at callback registration.  
  \return None
  
---------------------------------------------------------------------------*/
typedef void ( *tCsrStatsCallback) (void * stats, void *pContext);

/* ---------------------------------------------------------------------------
    \fn csrGetStatistics
    \brief csr function that client calls to register a callback to get 
    different PHY level statistics from CSR. 
    
    \param requesterId - different client requesting for statistics, HDD, UMA/GAN etc
    \param statsMask - The different category/categories of stats requester is looking for
    \param callback - SME sends back the requested stats using the callback
    \param periodicity - If requester needs periodic update, 0 means it's an one 
                         time request
    \param cache - If requester is happy with cached stats
    \param staId - The station ID for which the stats is requested for
    \param pContext - user context to be passed back along with the callback

    \return eHalStatus     
  ---------------------------------------------------------------------------*/
eHalStatus csrGetStatistics(tHalHandle hHal, eCsrStatsRequesterType requesterId, 
                            tANI_U32 statsMask, 
                            tCsrStatsCallback callback, 
                            tANI_U32 periodicity, tANI_BOOLEAN cache, 
                            tANI_U8 staId, void *pContext);


#endif

