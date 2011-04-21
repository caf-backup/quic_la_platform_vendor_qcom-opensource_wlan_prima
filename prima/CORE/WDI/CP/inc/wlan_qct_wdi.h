#ifndef WLAN_QCT_WDI_H
#define WLAN_QCT_WDI_H

/*===========================================================================

         W L A N   D E V I C E   A B S T R A C T I O N   L A Y E R 
                       E X T E R N A L  A P I
                
                   
DESCRIPTION
  This file contains the external API exposed by the wlan transport layer 
  module.
  
      
  Copyright (c) 2010-2011 QUALCOMM Incorporated.
  All Rights Reserved.
  Qualcomm Confidential and Proprietary
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE


  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  $Header:$ $DateTime: $ $Author: $


when        who    what, where, why
--------    ---    ----------------------------------------------------------
08/04/10    lti     Created module.

===========================================================================*/



/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_pal_api.h" 
#include "wlan_qct_pal_type.h" 
#include "wlan_qct_pack_align.h" 
#include "wlan_qct_wdi_cfg.h" 

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#ifdef __cplusplus
 extern "C" {
#endif 
 
/* MAC ADDRESS LENGTH - per spec*/
#define WDI_MAC_ADDR_LEN 6

/* Max number of 11b rates -> 1,2,5.5,11 */
#define WDI_NUM_11B_RATES                 4  

/* Max number of 11g rates -> 6,9,12,18,24,36,48,54*/
#define WDI_NUM_11A_RATES                 8  

/* Max number of legacy rates -> 72, 96, 108*/
#define WDI_NUM_POLARIS_RATES             3  

/* Max supported MCS set*/
#define WDI_MAC_MAX_SUPPORTED_MCS_SET    16

/*Max number of Access Categories for QoS - per spec */
#define WDI_MAX_NO_AC                     4

/*Max. size for reserving the Beacon Template */
#define WDI_BEACON_TEMPLATE_SIZE  0x180

/*============================================================================
 *     GENERIC STRUCTURES 
  ============================================================================*/

/*---------------------------------------------------------------------------
 WDI Device Capability
---------------------------------------------------------------------------*/
typedef struct 
{
  /*If this flag is true it means that the device can support 802.3/ETH2 to
    802.11 translation*/
  wpt_boolean   bFrameXtlSupported; 

  /*Maximum number of BSSes supported by the Device */
  wpt_uint8     ucMaxBSSSupported;

  /*Maximum number of stations supported by the Device */
  wpt_uint8     ucMaxSTASupported;
}WDI_DeviceCapabilityType; 

/*---------------------------------------------------------------------------
 WDI Channel Offset
---------------------------------------------------------------------------*/
typedef enum
{
  WDI_SECONDARY_CHANNEL_OFFSET_NONE   = 0,
  WDI_SECONDARY_CHANNEL_OFFSET_UP     = 1,
  WDI_SECONDARY_CHANNEL_OFFSET_DOWN   = 3
}WDI_HTSecondaryChannelOffset;

/*---------------------------------------------------------------------------
  WDI_MacFrameCtl
   Frame control field format (2 bytes)
---------------------------------------------------------------------------*/
typedef  struct 
{
    wpt_uint8 protVer :2;
    wpt_uint8 type :2;
    wpt_uint8 subType :4;

    wpt_uint8 toDS :1;
    wpt_uint8 fromDS :1;
    wpt_uint8 moreFrag :1;
    wpt_uint8 retry :1;
    wpt_uint8 powerMgmt :1;
    wpt_uint8 moreData :1;
    wpt_uint8 wep :1;
    wpt_uint8 order :1;

} WDI_MacFrameCtl;

/*---------------------------------------------------------------------------
  WDI Sequence control field
---------------------------------------------------------------------------*/
typedef struct 
{
  wpt_uint8 fragNum  : 4;
  wpt_uint8 seqNumLo : 4;
  wpt_uint8 seqNumHi : 8;
} WDI_MacSeqCtl;

/*---------------------------------------------------------------------------
  Management header format
---------------------------------------------------------------------------*/
typedef struct 
{
    WDI_MacFrameCtl     fc;
    wpt_uint8           durationLo;
    wpt_uint8           durationHi;
    wpt_uint8           da[WDI_MAC_ADDR_LEN];
    wpt_uint8           sa[WDI_MAC_ADDR_LEN];
    wpt_macAddr         bssId;
    WDI_MacSeqCtl       seqControl;
} WDI_MacMgmtHdr;

/*============================================================================
 *     GENERIC STRUCTURES - END
 ============================================================================*/

/*----------------------------------------------------------------------------
 *  Type Declarations
 * -------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
  WDI Status 
---------------------------------------------------------------------------*/
typedef enum
{
   WDI_STATUS_SUCCESS,       /* Operation has completed successfully*/
   WDI_STATUS_PENDING,       /* Operation result is pending and will be
                                provided asynchronously through the Req Status
                                Callback */
   WDI_STATUS_E_FAILURE,     /* Operation has ended in a generic failure*/
   WDI_STATUS_RES_FAILURE,   /* Operation has ended in a resource failure*/
   WDI_STATUS_MEM_FAILURE,   /* Operation has ended in a memory allocation
                               failure*/
   WDI_STATUS_E_NOT_ALLOWED, /* Operation is not allowed in the current state
                               of the driver*/
   WDI_STATUS_E_NOT_IMPLEMENT, /* Operation is not yet implemented*/

   WDI_STATUS_DEV_INTERNAL_FAILURE, /*An internal error has occured in the device*/
   WDI_STATUS_MAX

}WDI_Status;


/*---------------------------------------------------------------------------
   WDI_ReqStatusCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL to deliver to UMAC the result of posting
   a previous request for which the return status was PENDING.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from the Control Transport
    pUserData:  user data  
 
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void (*WDI_ReqStatusCb)(WDI_Status   wdiStatus,
                                void*        pUserData);

/*---------------------------------------------------------------------------
  WDI_LowLevelIndEnumType
    Types of indication that can be posted to UMAC by DAL
---------------------------------------------------------------------------*/
typedef enum
{
  /*When RSSI monitoring is enabled of the Lower MAC and a threshold has been
    passed. */
  WDI_LOW_RSSI_IND,

  /*Link loss in the low MAC */
  WDI_MISSED_BEACON_IND,

  /*when hardware has signaled an unknown addr2 frames. The indication will
  contain info from frames to be passed to the UMAC, this may use this info to
  deauth the STA*/
  WDI_UNKNOWN_ADDR2_FRAME_RX_IND,

  /*MIC Failure detected by HW*/
  WDI_MIC_FAILURE_IND,

  /*Fatal Error Ind*/
  WDI_FATAL_ERROR_IND, 

  /*Delete Station Ind*/
  WDI_DEL_STA_IND, 

  WDI_MAX_IND
}WDI_LowLevelIndEnumType;


/*---------------------------------------------------------------------------
  WDI_LowRSSIThIndType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Positive crossing of Rssi Thresh1*/
  wpt_boolean bRssiThres1PosCross;	
  
  /*Negative crossing of Rssi Thresh1*/
  wpt_boolean bRssiThres1NegCross;	
  
  /*Positive crossing of Rssi Thresh2*/
  wpt_boolean bRssiThres2PosCross;	
  
  /*Negative crossing of Rssi Thresh2*/
  wpt_boolean bRssiThres2NegCross;	
  
  /*Positive crossing of Rssi Thresh3*/
  wpt_boolean bRssiThres3PosCross;	
  
  /*Negative crossing of Rssi Thresh3*/
  wpt_boolean bRssiThres3NegCross;	
}WDI_LowRSSIThIndType;


/*---------------------------------------------------------------------------
  WDI_UnkAddr2FrmRxIndType
---------------------------------------------------------------------------*/
typedef struct
{
	/*Rx Bd data of the unknown received addr2 frame.*/
  void*  bufRxBd;

  /*Buffer Length*/
  wpt_uint16  usBufLen; 
}WDI_UnkAddr2FrmRxIndType;

/*---------------------------------------------------------------------------
  WDI_DeleteSTAIndType
---------------------------------------------------------------------------*/
typedef struct
{
   /*ASSOC ID, as assigned by UMAC*/
	 wpt_uint16    usAssocId;

   /*STA Index returned during DAL_PostAssocReq or DAL_ConfigStaReq*/
   wpt_uint16    usSTAIdx;

   /*BSSID of STA*/
   wpt_macAddr   macBSSID; 

    /*MAC ADDR of STA*/
    wpt_macAddr  macADDR2;          
                                
    /* To unify the keepalive / unknown A2 / tim-based disa*/
    wpt_uint16   wptReasonCode;   

}WDI_DeleteSTAIndType;

/*---------------------------------------------------------------------------
  WDI_MicFailureIndType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Source mac address*/
  wpt_macAddr macSrcAddr;	

 /*Transmitter mac address*/
 wpt_macAddr macTaAddr;

 /*	Destination mac address*/
 wpt_macAddr macDstAddr;

 /*Multicast flag*/
 wpt_uint8   ucMulticast;	

 /*First byte of IV*/
 wpt_uint8   ucIV1;

 /*Key Id*/
 wpt_uint8   keyId;	

 /*	Sequence Number*/
 wpt_uint8   TSC[6];
}WDI_MicFailureIndType; 

/*---------------------------------------------------------------------------
  WDI_LowLevelIndType
    Inidcation type and information about the indication being carried
    over
---------------------------------------------------------------------------*/
typedef struct
{
  /*Inidcation type*/
  WDI_LowLevelIndEnumType  wdiIndicationType; 

  /*Indication data*/
  union
  {
    /*RSSI Threshold Info for WDI_LOW_RSSI_IND*/
    WDI_LowRSSIThIndType        wdiLowRSSIInfo; 

    /*Addr2 Frame Info for WDI_UNKNOWN_ADDR2_FRAME_RX_IND*/
    WDI_UnkAddr2FrmRxIndType    wdiUnkAddr2FrmInfo;

    /*MIC Failure info for WDI_MIC_FAILURE_IND*/
    WDI_MicFailureIndType       wdiMICFailureInfo; 

    /*Error code for WDI_FATAL_ERROR_IND*/
    wpt_uint16                  usErrorCode;

    /*Delete STA Indication*/
    WDI_DeleteSTAIndType        wdiDeleteSTAIndType; 
  }  wdiIndicationData;
}WDI_LowLevelIndType;

/*---------------------------------------------------------------------------
  WDI_LowLevelIndCBType

   DESCRIPTION   
 
   This callback is invoked by DAL to deliver to UMAC certain indications
   that has either received from the lower device or has generated itself.
 
   PARAMETERS 

    IN
    pwdiInd:  information about the indication sent over
    pUserData:  user data provided by UMAC during registration 
 
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void (*WDI_LowLevelIndCBType)(WDI_LowLevelIndType* pwdiInd,
                                      void*                pUserData);

/*---------------------------------------------------------------------------
  WDI_DriverType
---------------------------------------------------------------------------*/
typedef enum
{
    WDI_DRIVER_TYPE_PRODUCTION  = 0,
    WDI_DRIVER_TYPE_MFG         = 1,
    WDI_DRIVER_TYPE_DVT         = 2
} WDI_DriverType;

/*---------------------------------------------------------------------------
  WDI_StartReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*This is a TLV formatted buffer containing all config values that can
   be set through the DAL Interface
 
   The TLV is expected to be formatted like this:
 
   0            7          15              31 .... 
   | CONFIG ID  |  CFG LEN |   RESERVED    |  CFG BODY  |
 
   Or from a C construct point of VU it would look like this:
 
   typedef struct WPT_PACK_POST
   {
       #ifdef  WPT_BIG_ENDIAN
         wpt_uint32   ucCfgId:8;
         wpt_uint32   ucCfgLen:8;
         wpt_uint32   usReserved:16;
       #else
         wpt_uint32   usReserved:16;
         wpt_uint32   ucCfgLen:8;
         wpt_uint32   ucCfgId:8;
       #endif
 
       wpt_uint8   ucCfgBody[ucCfgLen];
   }WDI_ConfigType; 
 
   Multiple such tuplets are to be placed in the config buffer. One for
   each required configuration item:
 
     | TLV 1 |  TLV2 | ....
 
   The buffer is expected to be a flat area of memory that can be manipulated
   with standard memory routines.
 
   For more info please check paragraph 2.3.1 Config Structure from the
   HAL LLD.
 
   For a list of accepted configuration list and IDs please look up
   wlan_qct_dal_cfg.h
 
  */
  void*                   pConfigBuffer; 

  /*Length of the config buffer above*/
  wpt_uint16              usConfigBufferLen;

  /*Production or FTM driver*/
  WDI_DriverType          wdiDriverType; 

  /*Should device enable frame translation */
  wpt_uint8               bFrameTransEnabled;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb         wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                   pUserData;

  /*Indication callback given by UMAC to be called by the WLAN DAL when it
    wishes to send something back independent of a request*/
  WDI_LowLevelIndCBType   wdiLowLevelIndCB; 

  /*The user data passed in by UMAC, it will be sent back when the indication
    function pointer will be called */
  void*                   pIndUserData;
}WDI_StartReqParamsType;


/*---------------------------------------------------------------------------
  WDI_StartRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status    wdiStatus;

  /*Max number of STA supported by the device*/
  wpt_uint8     ucMaxStations;	

  /*Max number of BSS supported by the device*/
  wpt_uint8     ucMaxBssids;

  /*Self STA Index */
  wpt_uint16    usSelfStaIdx;

  /*Self STA Mac*/
  wpt_macAddr   macSelfSta;

  /* Self STA DPU Index */
  wpt_uint16    usSelfStaDpuId;

}WDI_StartRspParamsType;


/*---------------------------------------------------------------------------
  WDI_StopType
---------------------------------------------------------------------------*/
typedef enum
{
  /*Device is being stopped due to a reset*/
  WDI_STOP_TYPE_SYS_RESET,

  /*Device is being stopped due to entering deep sleep*/
  WDI_STOP_TYPE_SYS_DEEP_SLEEP,

  /*Device is being stopped because the RF needs to shut off
    (e.g.:Airplane mode)*/
  WDI_STOP_TYPE_RF_KILL 
}WDI_StopType;

/*---------------------------------------------------------------------------
  WDI_StopReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{

  /*The reason for which the device is being stopped*/
  WDI_StopType   wdiStopReason;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_StopReqParamsType;


/*---------------------------------------------------------------------------
  WDI_ScanMode
---------------------------------------------------------------------------*/
typedef enum
{
  WDI_SCAN_MODE_NORMAL = 0,
  WDI_SCAN_MODE_LEARN,
  WDI_SCAN_MODE_SCAN,
  WDI_SCAN_MODE_PROMISC
} WDI_ScanMode;

/*---------------------------------------------------------------------------
  WDI_InitScanReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*LEARN - AP Role
    SCAN - STA Role*/
  WDI_ScanMode     wdiScanMode;

  /*BSSID of the BSS*/
  wpt_macAddr      macBSSID;

  /*Whether BSS needs to be notified*/
  wpt_boolean      bNotifyBSS;

  /*Kind of frame to be used for notifying the BSS (Data Null, QoS Null, or
  CTS to Self). Must always be a valid frame type.*/
  wpt_uint8        ucFrameType;

  /*UMAC has the option of passing the MAC frame to be used for notifying
   the BSS. If non-zero, HAL will use the MAC frame buffer pointed to by
   macMgmtHdr. If zero, HAL will generate the appropriate MAC frame based on
   frameType.*/
  wpt_uint8        ucFrameLength;

  /*Pointer to the MAC frame buffer. Used only if ucFrameLength is non-zero.*/
  WDI_MacMgmtHdr   wdiMACMgmtHdr;

}WDI_InitScanReqInfoType; 

/*---------------------------------------------------------------------------
  WDI_InitScanReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*The info associated with the request that needs to be sent over to the
    device*/
  WDI_InitScanReqInfoType  wdiReqInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb          wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                    pUserData;
}WDI_InitScanReqParamsType;

/*---------------------------------------------------------------------------
  WDI_StartScanReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the channel to scan*/
  wpt_uint8         ucChannel;	

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_StartScanReqParamsType;

/*---------------------------------------------------------------------------
  WDI_StartScanRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the status of the operation */
  WDI_Status        wdiStatus;	

#if defined WLAN_FEATURE_VOWIFI
  wpt_uint32        aStartTSF[2];
  wpt_int8          ucTxMgmtPower;
#endif
}WDI_StartScanRspParamsType;

/*---------------------------------------------------------------------------
  WDI_EndScanReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the channel to stop scanning.  Not used really. But retained
    for symmetry with "start Scan" message. It can also help in error
    check if needed.*/
  wpt_uint8         ucChannel;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_EndScanReqParamsType;

/*---------------------------------------------------------------------------
  WDI_PhyChanBondState
---------------------------------------------------------------------------*/
typedef enum
{
  WDI_PHY_SINGLE_CHANNEL_CENTERED = 0,            
  WDI_PHY_DOUBLE_CHANNEL_LOW_PRIMARY = 1,     
  WDI_PHY_DOUBLE_CHANNEL_CENTERED = 2,            
  WDI_PHY_DOUBLE_CHANNEL_HIGH_PRIMARY = 3     
} WDI_PhyChanBondState;

/*---------------------------------------------------------------------------
  WDI_FinishScanReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*LEARN - AP Role
    SCAN - STA Role*/
  WDI_ScanMode          wdiScanMode;

  /*Operating channel to tune to.*/
  wpt_uint8             ucCurrentOperatingChannel;

  /*Channel Bonding state If 20/40 MHz is operational, this will indicate the
  40 MHz extension channel in combination with the control channel*/
  WDI_PhyChanBondState  wdiCBState;

  /*BSSID of the BSS*/
  wpt_macAddr           macBSSID;

  /*Whether BSS needs to be notified*/
  wpt_boolean           bNotifyBSS;

  /*Kind of frame to be used for notifying the BSS (Data Null, QoS Null, or
  CTS to Self). Must always be a valid frame type.*/
  wpt_uint8             ucFrameType;

  /*UMAC has the option of passing the MAC frame to be used for notifying
   the BSS. If non-zero, HAL will use the MAC frame buffer pointed to by
   macMgmtHdr. If zero, HAL will generate the appropriate MAC frame based on
   frameType.*/
  wpt_uint8             ucFrameLength;

  /*Pointer to the MAC frame buffer. Used only if ucFrameLength is non-zero.*/
  WDI_MacMgmtHdr        wdiMACMgmtHdr;

}WDI_FinishScanReqInfoType; 

/*---------------------------------------------------------------------------
  WDI_SwitchChReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the channel to switch to.*/
  wpt_uint8         ucChannel;

  /*Local power constraint*/
  wpt_uint8         ucLocalPowerConstraint;

  /*Secondary channel offset */
  WDI_HTSecondaryChannelOffset  wdiSecondaryChannelOffset;

}WDI_SwitchChReqInfoType;

/*---------------------------------------------------------------------------
  WDI_SwitchChReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Channel Info*/
  WDI_SwitchChReqInfoType  wdiChInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_SwitchChReqParamsType;

/*---------------------------------------------------------------------------
  WDI_FinishScanReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Info for the Finish Scan request that will be sent down to the device*/
 	WDI_FinishScanReqInfoType  wdiReqInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb            wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                      pUserData;
}WDI_FinishScanReqParamsType;

/*---------------------------------------------------------------------------
  WDI_JoinReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the BSSID to which STA is going to associate*/
  wpt_macAddr     	macBSSID; 

  /*Indicates the channel to switch to.*/
  WDI_SwitchChReqInfoType  wdiChannelInfo; 
  	
}WDI_JoinReqInfoType;

/*---------------------------------------------------------------------------
  WDI_JoinReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Info for the Join request that will be sent down to the device*/
  WDI_JoinReqInfoType   wdiReqInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb       wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                 pUserData;
}WDI_JoinReqParamsType;

/*---------------------------------------------------------------------------
  WDI_BssType
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_INFRASTRUCTURE_MODE,
  WDI_INFRA_AP_MODE,                    //Added for softAP support
  WDI_IBSS_MODE,
  WDI_BTAMP_STA_MODE, 
  WDI_BTAMP_AP_MODE,
  WDI_BSS_AUTO_MODE,
}WDI_BssType;

/*---------------------------------------------------------------------------
  WDI_NwType
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_11A_NW_TYPE,
  WDI_11B_NW_TYPE,
  WDI_11G_NW_TYPE,
  WDI_11N_NW_TYPE,
} WDI_NwType;	

/*---------------------------------------------------------------------------
  WDI_MacSSid
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint8        ucLength;
    wpt_uint8        sSSID[32];
} WDI_MacSSid;

/*---------------------------------------------------------------------------
  WDI_ConfigAction
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_ADD_BSS,
  WDI_UPDATE_BSS
} WDI_ConfigAction;	

/*---------------------------------------------------------------------------
  WDI_HTOperatingMode
---------------------------------------------------------------------------*/
typedef enum
{
  WDI_HT_OP_MODE_PURE,
  WDI_HT_OP_MODE_OVERLAP_LEGACY,  
  WDI_HT_OP_MODE_NO_LEGACY_20MHZ_HT,  
  WDI_HT_OP_MODE_MIXED

} WDI_HTOperatingMode;	


/*---------------------------------------------------------------------------
  WDI_STAEntryType
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_STA_ENTRY_SELF,
  WDI_STA_ENTRY_PEER,
  WDI_STA_ENTRY_BSSID,
  WDI_STA_ENTRY_BCAST
}WDI_STAEntryType;

/*---------------------------------------------------------------------------
  WDI_ConfigActionType
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_ADD_STA,
  WDI_UPDATE_STA
} WDI_ConfigActionType;

/*---------------------------------------------------------------------------- 
  Each station added has a rate mode which specifies the sta attributes
  ----------------------------------------------------------------------------*/
typedef enum 
{
    WDI_RESERVED_1 = 0,
    WDI_RESERVED_2,
    WDI_RESERVED_3,
    WDI_11b,
    WDI_11bg,
    WDI_11a,
    WDI_11n,
} WDI_RateModeType;

/*---------------------------------------------------------------------------
  WDI_SupportedRatesType
---------------------------------------------------------------------------*/
typedef struct  
{
    /*
    * For Self STA Entry: this represents Self Mode.
    * For Peer Stations, this represents the mode of the peer.
    * On Station:
    * --this mode is updated when PE adds the Self Entry.
    * -- OR when PE sends 'ADD_BSS' message and station context in BSS is used to indicate the mode of the AP.
    * ON AP:
    * -- this mode is updated when PE sends 'ADD_BSS' and Sta entry for that BSS is used
    *     to indicate the self mode of the AP.
    * -- OR when a station is associated, PE sends 'ADD_STA' message with this mode updated.
    */

    WDI_RateModeType   opRateMode;

    /* 11b, 11a and aniLegacyRates are IE rates which gives rate in unit of 500Kbps */
    wpt_uint16         llbRates[WDI_NUM_11B_RATES];
    wpt_uint16         llaRates[WDI_NUM_11A_RATES];
    wpt_uint16         aLegacyRates[WDI_NUM_POLARIS_RATES];

    /*Taurus only supports 26 Titan Rates(no ESF/concat Rates will be supported)
      First 26 bits are reserved for those Titan rates and
     the last 4 bits(bit28-31) for Taurus, 2(bit26-27) bits are reserved.*/
    wpt_uint32         uEnhancedRateBitmap; //Titan and Taurus Rates

    /*
    * 0-76 bits used, remaining reserved
    * bits 0-15 and 32 should be set.
    */
    wpt_uint8           aSupportedMCSSet[WDI_MAC_MAX_SUPPORTED_MCS_SET];

    /*
     * RX Highest Supported Data Rate defines the highest data
     * rate that the STA is able to receive, in unites of 1Mbps.
     * This value is derived from "Supported MCS Set field" inside
     * the HT capability element.
     */
    wpt_uint16         aRxHighestDataRate;

} WDI_SupportedRates;

/*-------------------------------------------------------------------------- 
  WDI_HTMIMOPowerSaveState 
    Spatial Multiplexing(SM) Power Save mode
 --------------------------------------------------------------------------*/
typedef enum 
{
  WDI_HT_MIMO_PS_STATIC   = 0,    // Static SM Power Save mode
  WDI_HT_MIMO_PS_DYNAMIC  = 1,   // Dynamic SM Power Save mode
  WDI_HT_MIMO_PS_NA       = 2,        // reserved
  WDI_HT_MIMO_PS_NO_LIMIT = 3,  // SM Power Save disabled
} WDI_HTMIMOPowerSaveState;

/*---------------------------------------------------------------------------
  WDI_ConfigStaReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BSSID of STA*/
  wpt_macAddr               macBSSID;

  /*ASSOC ID, as assigned by UMAC*/
  wpt_uint16                usAssocId;

  /*Used for configuration of different HW modules.*/
  WDI_STAEntryType          wdiSTAType;

  /*Short Preamble Supported.*/
  wpt_uint8                 ucShortPreambleSupported;

  /*MAC Address of STA*/
  wpt_macAddr               macSTA;

  /*Listen interval of the STA*/
  wpt_uint16                usListenInterval;

  /*Support for 11e/WMM*/
  wpt_uint8                 ucWMMEnabled;

  /*11n HT capable STA*/
  wpt_uint8                 ucHTCapable;

  /*TX Width Set: 0 - 20 MHz only, 1 - 20/40 MHz*/
  wpt_uint8                 ucTXChannelWidthSet;

  /*RIFS mode 0 - NA, 1 - Allowed*/
  wpt_uint8                 ucRIFSMode;

  /*L-SIG TXOP Protection mechanism
  0 - No Support, 1 - Supported
      SG - there is global field*/
  wpt_uint8                 ucLSIGTxopProtection;

  /*Max Ampdu Size supported by STA. Device programming.
    0 : 8k , 1 : 16k, 2 : 32k, 3 : 64k */
  wpt_uint8                 ucMaxAmpduSize;

  /*Max Ampdu density. Used by RA. 3 : 0~7 : 2^(11nAMPDUdensity -4)*/
  wpt_uint8                 ucMaxAmpduDensity;

  /*Max AMSDU size 1 : 3839 bytes, 0 : 7935 bytes*/
  wpt_uint8                 ucMaxAmsduSize;

  /*Short GI support for 40Mhz packets*/
  wpt_uint8                 ucShortGI40Mhz;

  /*Short GI support for 20Mhz packets*/
  wpt_uint8                 ucShortGI20Mhz;

  /*These rates are the intersection of peer and self capabilities.*/
  //! Comented out in the new HAL header 
  //! WDI_SupportedRates        wdiSupportedRates;

  /*Robust Management Frame (RMF) enabled/disabled*/
  wpt_uint8                 ucRMFEnabled;

  /*HAL should update the existing STA entry, if this flag is set. UMAC 
   will set this flag in case of RE-ASSOC, where we want to reuse the old
   STA ID.*/
  WDI_ConfigActionType      wdiAction;	

  /*U-APSD Flags: 1b per AC.  Encoded as follows:
     b7 b6 b5 b4 b3 b2 b1 b0 =
     X  X  X  X  BE BK VI VO
  */
  wpt_uint8                 ucAPSD;

  /*Max SP Length*/
  wpt_uint8                 ucMaxSPLen;

  /*11n Green Field preamble support*/
  wpt_uint8                 ucGreenFieldCapable;

  /*MIMO Power Save mode*/
  WDI_HTMIMOPowerSaveState  wdiMIMOPS;

  /*Delayed BA Support*/
  wpt_uint8                 ucDelayedBASupport;	

  /*Max AMPDU duration in 32us*/
  wpt_uint8                 us32MaxAmpduDuratio;

  /*HT STA should set it to 1 if it is enabled in BSS
   HT STA should set it to 0 if AP does not support it. This indication is
   sent to HAL and HAL uses this flag to pickup up appropriate 40Mhz rates.
  */
  wpt_uint8                 ucDsssCckMode40Mhz;

}WDI_ConfigStaReqInfoType;


/*---------------------------------------------------------------------------
  WDI_RateSet
 
  12 Bytes long because this structure can be used to represent rate
  and extended rate set IEs
  The parser assume this to be at least 12 
---------------------------------------------------------------------------*/
#define WDI_RATESET_EID_MAX            12

typedef struct 
{
    wpt_uint8  ucNumRates;
    wpt_uint8  aRates[WDI_RATESET_EID_MAX];
} WDI_RateSet;

/*---------------------------------------------------------------------------
  WDI_AciAifsnType
   access category record
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint8  rsvd  : 1;
    wpt_uint8  aci   : 2;
    wpt_uint8  acm   : 1;
    wpt_uint8  aifsn : 4;
} WDI_AciAifsnType;

/*---------------------------------------------------------------------------
  WDI_CWType
   contention window size
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint8  max : 4;
    wpt_uint8  min : 4;
} WDI_CWType;

/*---------------------------------------------------------------------------
  WDI_EdcaParamRecord
---------------------------------------------------------------------------*/
typedef struct 
{
    /*Access Category Record*/
    WDI_AciAifsnType  wdiACI;

    /*Contention WIndow Size*/
    WDI_CWType        wdiCW;

    /*TX Oportunity Limit*/
    wpt_uint16        usTXOPLimit;
} WDI_EdcaParamRecord;

/*---------------------------------------------------------------------------
  WDI_EDCAParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BSS Index*/
  wpt_uint16     usBSSIdx;
  
  /*?*/
  wpt_boolean    bHighPerformance;

  /*Best Effort*/
  WDI_EdcaParamRecord wdiACBE; 
           
  /*Background*/
  WDI_EdcaParamRecord wdiACBK; 
                            
  /*Video*/
  WDI_EdcaParamRecord wdiACVI; 
  
  /*Voice*/
  WDI_EdcaParamRecord acvo; // voice
} WDI_EDCAParamsType;

/*---------------------------------------------------------------------------
  WDI_ConfigBSSReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Peer BSSID*/
  wpt_macAddr     	       macBSSID; 	

  /*Self MAC Address*/
  wpt_macAddr     	       macSelfAddr; 

  /*BSS Type*/
  WDI_BssType              wdiBSSType;

  /*Operational Mode: AP =0, STA = 1*/
  wpt_uint8                ucOperMode;

  /*Network Type*/
  WDI_NwType               wdiNWType;

  /*Used to classify PURE_11G/11G_MIXED to program MTU*/
  wpt_uint8                ucShortSlotTimeSupported;

  /*Co-exist with 11a STA*/
  wpt_uint8                ucllaCoexist;		

  /*Co-exist with 11b STA*/
  wpt_uint8                ucllbCoexist;		

  /*Co-exist with 11g STA*/
  wpt_uint8                ucllgCoexist;		

  /*Coexistence with 11n STA*/
  wpt_uint8                ucHT20Coexist;		

  /*Non GF coexist flag*/
  wpt_uint8                ucllnNonGFCoexist;

  /*TXOP protection support*/
  wpt_uint8                ucTXOPProtectionFullSupport;		

  /*RIFS mode*/
  wpt_uint8                ucRIFSMode;	

  /*Beacon Interval in TU*/
  wpt_uint16               usBeaconInterval;	

  /*DTIM period*/
  wpt_uint8                ucDTIMPeriod;

  /*TX Width Set: 0 - 20 MHz only, 1 - 20/40 MHz*/		
  wpt_uint8                ucTXChannelWidthSet;
  	
  /*Operating channel*/	
  wpt_uint8                ucCurrentOperChannel;

  /*Extension channel for channel bonding*/
  wpt_uint8                ucCurrentExtChannel;		

  /*Context of the station being added in HW.*/
  WDI_ConfigStaReqInfoType wdiSTAContext;

  /*SSID of the BSS*/
  WDI_MacSSid              wdiSSID;

  /*HAL should update the existing BSS entry, if this flag is set. UMAC will
    set this flag in case of RE-ASSOC, where we want to reuse the old BSSID*/
  WDI_ConfigAction         wdiAction;

  /*Basic Rate Set*/
  WDI_RateSet	             wdiRateSet;

  /*Enable/Disable HT capabilities of the BSS*/
  wpt_uint8                ucHTCapable;

  /*RMF enabled/disabled*/
  wpt_uint8                ucRMFEnabled;

  /*Determines the current HT Operating Mode operating mode of the
    802.11n STA*/
  WDI_HTOperatingMode      wdiHTOperMod;

  /*Dual CTS Protection: 0 - Unused, 1 - Used*/
  wpt_uint8                ucDualCTSProtection;

    /* Probe Response Max retries */
  wpt_uint8   ucMaxProbeRespRetryLimit;

  /* To Enable Hidden ssid */
  wpt_uint8   bHiddenSSIDEn;

  /* To Enable Disable FW Proxy Probe Resp */
  wpt_uint8   bProxyProbeRespEn;

 /* Boolean to indicate if EDCA params are valid. UMAC might not have valid 
    EDCA params or might not desire to apply EDCA params during config BSS. 
    0 implies Not Valid ; Non-Zero implies valid*/
  wpt_uint8   ucEDCAParamsValid;

   /*EDCA Parameters for BK*/  
  WDI_EdcaParamRecord       wdiBKEDCAParams; 

   /*EDCA Parameters for BE*/  
  WDI_EdcaParamRecord       wdiBEEDCAParams; 

   /*EDCA Parameters for VI*/  
  WDI_EdcaParamRecord       wdiVIEDCAParams; 

   /*EDCA Parameters for VO*/  
  WDI_EdcaParamRecord       wdiVOEDCAParams; 
}WDI_ConfigBSSReqInfoType;


/*---------------------------------------------------------------------------
  WDI_ConfigBSSReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Info for the Join request that will be sent down to the device*/
  WDI_ConfigBSSReqInfoType   wdiReqInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb            wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                      pUserData;
}WDI_ConfigBSSReqParamsType;

/*---------------------------------------------------------------------------
  WDI_ConfigBSSRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 

  /*BSSID of the BSS*/
  wpt_macAddr  macBSSID; 

  /*BSS Index*/
  wpt_uint16   usBSSIdx;

  /*Unicast DPU signature*/
  wpt_uint8    ucUcastSig;

  /*Broadcast DPU Signature*/
  wpt_uint8    ucBcastSig;

  /*MAC Address of STA*/ 
  wpt_macAddr  macSTA;

  /*BSS STA ID*/
  wpt_uint16   usSTAIdx;
  
}WDI_ConfigBSSRspParamsType;

/*---------------------------------------------------------------------------
  WDI_DelBSSReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
   /*BSS Index of the BSS*/
   wpt_uint8      ucBssIdx;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_DelBSSReqParamsType;

/*---------------------------------------------------------------------------
  WDI_DelBSSRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 

  /*BSSID of the BSS*/
  wpt_macAddr  macBSSID; 

}WDI_DelBSSRspParamsType;

/*---------------------------------------------------------------------------
  WDI_PostAssocReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Config STA arguments.*/
  WDI_ConfigStaReqInfoType	wdiSTAParams; 

   /*Config BSS Arguments*/
  WDI_ConfigBSSReqInfoType	wdiBSSParams;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb           wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                     pUserData;
}WDI_PostAssocReqParamsType;

/*---------------------------------------------------------------------------
  WDI_ConfigSTARspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status      wdiStatus;

  /*STA Idx allocated by HAL*/
  wpt_uint16      usSTAId;

  /*MAC Address of STA*/
  wpt_macAddr     macSTA;

  /* DPU Index  - PTK */
  wpt_uint8       ucDpuIndex;

  /* Bcast DPU Index  - GTK */  
  wpt_uint8       ucBcastDpuIndex;

  /* Management DPU Index - IGTK - Why is it called bcastMgmtDpuIdx? */
  wpt_uint8       ucBcastMgmtDpuIdx;

  /*Unicast DPU signature*/
  wpt_uint8       ucUcastSig;

  /*Broadcast DPU Signature*/
  wpt_uint8       ucBcastSig;

}WDI_ConfigSTARspParamsType;

/*---------------------------------------------------------------------------
  WDI_PostAssocRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 

  /*Parameters related to the BSS*/
  WDI_ConfigBSSRspParamsType bssParams;

  /*Parameters related to the self STA*/
  WDI_ConfigSTARspParamsType staParams;

}WDI_PostAssocRspParamsType;

/*---------------------------------------------------------------------------
  WDI_DelSTAReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*STA Index returned during DAL_PostAssocReq or DAL_ConfigStaReq*/
  wpt_uint16        usSTAIdx;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_DelSTAReqParamsType;

/*---------------------------------------------------------------------------
  WDI_DelSTARspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
 /*Status of the response*/
  WDI_Status   wdiStatus; 

  /*STA Index returned during DAL_PostAssocReq or DAL_ConfigStaReq*/
  wpt_uint16   usSTAIdx;
}WDI_DelSTARspParamsType;

/*---------------------------------------------------------------------------
  WDI_EncryptType
---------------------------------------------------------------------------*/
typedef enum 
{
    WDI_ENCR_NONE,
    WDI_ENCR_WEP40,
    WDI_ENCR_WEP104,
    WDI_ENCR_TKIP,
    WDI_ENCR_CCMP,
    WDI_ENCR_AES_128_CMAC,
    WDI_ENCR_WPI
} WDI_EncryptType;

/*---------------------------------------------------------------------------
  WDI_KeyDirectionType
---------------------------------------------------------------------------*/
typedef enum
{
    WDI_TX_ONLY,
    WDI_RX_ONLY,
    WDI_TX_RX,
#ifdef WLAN_SOFTAP_FEATURE
    WDI_TX_DEFAULT,
#endif
    WDI_DONOT_USE_KEY_DIRECTION
} WDI_KeyDirectionType;

#define WDI_MAX_ENCR_KEYS 4
#define WDI_MAX_KEY_LENGTH 32
#if defined(FEATURE_WLAN_WAPI)
#define WDI_MAX_KEY_RSC_LEN         16
#define WDI_WAPI_KEY_RSC_LEN        16
#else
#define WDI_MAX_KEY_RSC_LEN         8
#endif

typedef struct
{
    /* Key ID */
    wpt_uint8                  keyId;
    /* 0 for multicast */
    wpt_uint8                  unicast;     
    /* Key Direction */
    WDI_KeyDirectionType       keyDirection;
    /* Usage is unknown */
    wpt_uint8                  keyRsc[WDI_MAX_KEY_RSC_LEN];   
    /* =1 for authenticator, =0 for supplicant */
    wpt_uint8                  paeRole;     
    wpt_uint16                 keyLength;
    wpt_uint8                  key[WDI_MAX_KEY_LENGTH];

}WDI_KeysType;

/*---------------------------------------------------------------------------
  WDI_SetBSSKeyReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*BSS Index of the BSS*/
  wpt_uint8      ucBssIdx; 

  /*Encryption Type used with peer*/
  WDI_EncryptType  wdiEncType;		

  /*Number of keys*/
  wpt_uint8        ucNumKeys;

  /*Array of keys.*/
  WDI_KeysType	   aKeys[WDI_MAX_ENCR_KEYS]; 

  /*Control for Replay Count, 1= Single TID based replay count on Tx
    0 = Per TID based replay count on TX */
  wpt_uint8        ucSingleTidRc; 
}WDI_SetBSSKeyReqInfoType; 

/*---------------------------------------------------------------------------
  WDI_SetBSSKeyReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Key Info */
  WDI_SetBSSKeyReqInfoType  wdiBSSKeyInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb           wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                      pUserData;
}WDI_SetBSSKeyReqParamsType;

/*---------------------------------------------------------------------------
  WDI_WepType
---------------------------------------------------------------------------*/
typedef enum 
{
  WDI_WEP_STATIC,
  WDI_WEP_DYNAMIC

} WDI_WepType;

/*---------------------------------------------------------------------------
  WDI_RemoveBSSKeyReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*BSS Index of the BSS*/
  wpt_uint8      ucBssIdx; 

  /*Encryption Type used with peer*/
  WDI_EncryptType  wdiEncType;		

  /*Key Id*/
  wpt_uint8    ucKeyId;

  /*STATIC/DYNAMIC. Used in Nullifying in Key Descriptors for Static/Dynamic
    keys*/
  WDI_WepType  wdiWEPType;
}WDI_RemoveBSSKeyReqInfoType;

/*---------------------------------------------------------------------------
  WDI_RemoveBSSKeyReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Key Info */
  WDI_RemoveBSSKeyReqInfoType  wdiKeyInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_RemoveBSSKeyReqParamsType;

/*---------------------------------------------------------------------------
  WDI_SetSTAKeyReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*STA Index*/
  wpt_uint16       usSTAIdx; 

  /*Encryption Type used with peer*/
  WDI_EncryptType  wdiEncType;		

  /*STATIC/DYNAMIC*/
  WDI_WepType      wdiWEPType;

  /*Default WEP key, valid only for static WEP, must between 0 and 3.*/
  wpt_uint8        ucDefWEPIdx;

  /*Number of keys*/
  wpt_uint8        ucNumKeys;

  /*Array of keys.*/
  WDI_KeysType	   wdiKey[WDI_MAX_ENCR_KEYS]; 

  /*Control for Replay Count, 1= Single TID based replay count on Tx
    0 = Per TID based replay count on TX */
  wpt_uint8        ucSingleTidRc; 
}WDI_SetSTAKeyReqInfoType; 

/*---------------------------------------------------------------------------
  WDI_SetSTAKeyReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Key Info*/
  WDI_SetSTAKeyReqInfoType  wdiKeyInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_SetSTAKeyReqParamsType;

/*---------------------------------------------------------------------------
  WDI_RemoveSTAKeyReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*STA Index*/
  wpt_uint16       usSTAIdx; 

  /*Encryption Type used with peer*/
  WDI_EncryptType  wdiEncType;		

  /*Key Id*/
  wpt_uint8        ucKeyId;

  /*Whether to invalidate the Broadcast key or Unicast key. In case of WEP,
  the same key is used for both broadcast and unicast.*/
  wpt_uint8        ucUnicast;
}WDI_RemoveSTAKeyReqInfoType;

/*---------------------------------------------------------------------------
  WDI_RemoveSTAKeyReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Key Info */
  WDI_RemoveSTAKeyReqInfoType  wdiKeyInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_RemoveSTAKeyReqParamsType;

/*---------------------------------------------------------------------------
                            QOS Parameters
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  WDI_TSInfoTfc
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint16       ackPolicy:2;
    wpt_uint16       userPrio:3;
    wpt_uint16       psb:1;
    wpt_uint16       aggregation : 1;
    wpt_uint16       accessPolicy : 2;
    wpt_uint16       direction : 2;
    wpt_uint16       tsid : 4;
    wpt_uint16       trafficType : 1;
} WDI_TSInfoTfc;

/*---------------------------------------------------------------------------
  WDI_TSInfoSch
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint8        rsvd : 7;
    wpt_uint8        schedule : 1;
} WDI_TSInfoSch;

/*---------------------------------------------------------------------------
  WDI_TSInfoType
---------------------------------------------------------------------------*/
typedef struct 
{
    WDI_TSInfoTfc  wdiTraffic;
    WDI_TSInfoSch  wdiSchedule;
} WDI_TSInfoType;

/*---------------------------------------------------------------------------
  WDI_TspecIEType
---------------------------------------------------------------------------*/
typedef struct 
{
    wpt_uint8             ucType;
    wpt_uint8             ucLength;
    WDI_TSInfoType        wdiTSinfo;
    wpt_uint16            usNomMsduSz;
    wpt_uint16            usMaxMsduSz;
    wpt_uint32            uMinSvcInterval;
    wpt_uint32            uMaxSvcInterval;
    wpt_uint32            uInactInterval;
    wpt_uint32            uSuspendInterval;
    wpt_uint32            uSvcStartTime;
    wpt_uint32            uMinDataRate;
    wpt_uint32            uMeanDataRate;
    wpt_uint32            uPeakDataRate;
    wpt_uint32            uMaxBurstSz;
    wpt_uint32            uDelayBound;
    wpt_uint32            uMinPhyRate;
    wpt_uint16            usSurplusBw;
    wpt_uint16            usMediumTime;
}WDI_TspecIEType;

/*---------------------------------------------------------------------------
  WDI_AddTSReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*STA Index*/
  wpt_uint16        usSTAIdx; 

  /*Identifier for TSpec*/
  wpt_uint16        ucTspecIdx;

  /*Tspec IE negotiated OTA*/
	WDI_TspecIEType	  wdiTspecIE;

  /*UAPSD delivery and trigger enabled flags */
  wpt_uint8         ucUapsdFlags;

  /*SI for each AC*/
  wpt_uint8         ucServiceInterval[WDI_MAX_NO_AC];

  /*Suspend Interval for each AC*/
  wpt_uint8         ucSuspendInterval[WDI_MAX_NO_AC];

  /*DI for each AC*/
  wpt_uint8         ucDelayedInterval[WDI_MAX_NO_AC];

}WDI_AddTSReqInfoType;


/*---------------------------------------------------------------------------
  WDI_AddTSReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*TSpec Info */
  WDI_AddTSReqInfoType  wdiTsInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_AddTSReqParamsType;

/*---------------------------------------------------------------------------
  WDI_DelTSReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*STA Index*/
  wpt_uint16        usSTAIdx; 

  /*Identifier for TSpec*/
  wpt_uint16        ucTspecIdx;

  /*BSSID of the BSS*/
  wpt_macAddr      macBSSID;
}WDI_DelTSReqInfoType;

/*---------------------------------------------------------------------------
  WDI_DelTSReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Del TSpec Info*/
  WDI_DelTSReqInfoType  wdiDelTSInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_DelTSReqParamsType;

/*---------------------------------------------------------------------------
  WDI_UpdateEDCAInfoType
---------------------------------------------------------------------------*/
typedef struct
{
   /*BSS Index of the BSS*/
   wpt_uint8      ucBssIdx;

  /* Boolean to indicate if EDCA params are valid. UMAC might not have valid 
    EDCA params or might not desire to apply EDCA params during config BSS. 
    0 implies Not Valid ; Non-Zero implies valid*/
  wpt_uint8   ucEDCAParamsValid;

  /*EDCA params for BE*/
  WDI_EdcaParamRecord wdiEdcaBEInfo;

  /*EDCA params for BK*/
  WDI_EdcaParamRecord wdiEdcaBKInfo;

  /*EDCA params for VI*/
  WDI_EdcaParamRecord wdiEdcaVIInfo;

  /*EDCA params for VO*/
  WDI_EdcaParamRecord wdiEdcaVOInfo;

}WDI_UpdateEDCAInfoType;

/*---------------------------------------------------------------------------
  WDI_UpdateEDCAParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*EDCA Info */
  WDI_UpdateEDCAInfoType  wdiEDCAInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_UpdateEDCAParamsType;

/*---------------------------------------------------------------------------
  WDI_AddBASessionReqinfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the station for which BA is added..*/
  wpt_uint16       usSTAIdx;	

  /*The peer mac address*/
  wpt_macAddr      macPeerAddr;

  /*TID for which BA was negotiated*/
  wpt_uint8        ucBaTID;

  /*Delayed or imediate */
  wpt_uint8        ucBaPolicy;

  /*The number of buffers for this TID (baTID)*/
  wpt_uint16       usBaBufferSize;
  
  /*BA timeout in TU's*/
  wpt_uint16       usBaTimeout;
  
  /*b0..b3 - Fragment Number - Always set to 0
   b4..b15 - Starting Sequence Number of first MSDU for which this BA is setup*/
  wpt_uint16       usBaSSN;
  
  /*Originator/Recipient*/
  wpt_uint8        ucBaDirection;
  
}WDI_AddBASessionReqinfoType;


/*---------------------------------------------------------------------------
  WDI_AddBASessionReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BA Session Info Type*/
  WDI_AddBASessionReqinfoType  wdiBASessionInfoType; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb       wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                 pUserData;
}WDI_AddBASessionReqParamsType;

/*---------------------------------------------------------------------------
  WDI_AddBASessionRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 
  
  /* Dialog token */
  wpt_uint8    ucBaDialogToken;
  
  /* TID for which the BA session has been setup */
  wpt_uint8    ucBaTID;
  
  /* BA Buffer Size allocated for the current BA session */
  wpt_uint8    ucBaBufferSize;

  /* BA session ID */
  wpt_uint16   usBaSessionID;
  
  /* Reordering Window buffer */
  wpt_uint8    ucWinSize;
  
  /*Station Index to id the sta */
  wpt_uint8    usSTAIdx;
  
  /* Starting Sequence Number */
  wpt_uint16   usBaSSN;

}WDI_AddBASessionRspParamsType;

/*---------------------------------------------------------------------------
  WDI_AddBAReqinfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the station for which BA is added..*/
  wpt_uint16       usSTAIdx;

  /* Session Id */
  wpt_uint8        ucBaSessionID;
  
  /* Reorder Window Size */
  wpt_uint8        ucWinSize;
  
#ifdef FEATURE_ON_CHIP_REORDERING
  wpt_boolean      bIsReorderingDoneOnChip;
#endif

}WDI_AddBAReqinfoType;


/*---------------------------------------------------------------------------
  WDI_AddBAReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BA Info Type*/
  WDI_AddBAReqinfoType  wdiBAInfoType; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb       wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                 pUserData;
}WDI_AddBAReqParamsType;


/*---------------------------------------------------------------------------
  WDI_AddBARspinfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 

  /* Dialog token */
  wpt_uint8    ucBaDialogToken;

}WDI_AddBARspinfoType;

/*---------------------------------------------------------------------------
  WDI_TriggerBAReqCandidateType
---------------------------------------------------------------------------*/
typedef struct
{
  /* STA index */
  wpt_uint16  usSTAIdx;

  /* TID bit map for the STA's*/
  wpt_uint8   ucTidBitmap;

}WDI_TriggerBAReqCandidateType;


/*---------------------------------------------------------------------------
  WDI_TriggerBAReqinfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the station for which BA is added..*/
  wpt_uint16       usSTAIdx;

  /* Session Id */
  wpt_uint8        ucBASessionID;

  /* Trigger BA Request candidate count */
  wpt_uint16       usBACandidateCnt;

  /* WDI_TriggerBAReqCandidateType  followed by this*/

}WDI_TriggerBAReqinfoType;


/*---------------------------------------------------------------------------
  WDI_TriggerBAReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BA Trigger Info Type*/
  WDI_TriggerBAReqinfoType  wdiTriggerBAInfoType; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb       wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                 pUserData;
}WDI_TriggerBAReqParamsType;

/*---------------------------------------------------------------------------
  WDI_AddBAInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  wpt_uint16 fBaEnable : 1;
  wpt_uint16 startingSeqNum: 12;
  wpt_uint16 reserved : 3;
}WDI_AddBAInfoType;

/*---------------------------------------------------------------------------
  WDI_TriggerBARspCandidateType
---------------------------------------------------------------------------*/
#define STA_MAX_TC 8

typedef struct
{
  /* STA index */
  wpt_macAddr       macSTA;

  /* BA Info */
  WDI_AddBAInfoType wdiBAInfo[STA_MAX_TC];
}WDI_TriggerBARspCandidateType;

/*---------------------------------------------------------------------------
  WDI_TriggerBARspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Status of the response*/
  WDI_Status   wdiStatus; 

  /*BSSID of the BSS*/
  wpt_macAddr  macBSSID;

  /* Trigger BA response candidate count */
  wpt_uint16   usBaCandidateCnt;

  /* WDI_TriggerBARspCandidateType  followed by this*/

}WDI_TriggerBARspParamsType;

/*---------------------------------------------------------------------------
  WDI_DelBAReqinfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Indicates the station for which BA is added..*/
  wpt_uint16       usSTAIdx;	

  /*TID for which BA was negotiated*/
  wpt_uint8        ucBaTID;

  /*Originator/Recipient*/
  wpt_uint8        ucBaDirection;
  
}WDI_DelBAReqinfoType;

/*---------------------------------------------------------------------------
  WDI_DelBAReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BA Info */
  WDI_DelBAReqinfoType  wdiBAInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_DelBAReqParamsType;


/*---------------------------------------------------------------------------
  WDI_SwitchCHRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
   /*Status of the response*/
  WDI_Status    wdiStatus;

  /*Indicates the channel that WLAN is on*/
  wpt_uint8     ucChannel;	

}WDI_SwitchCHRspParamsType;

/*---------------------------------------------------------------------------
  WDI_ConfigSTAReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Info for the Join request that will be sent down to the device*/
  WDI_ConfigStaReqInfoType   wdiReqInfo; 

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb            wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*                      pUserData;
}WDI_ConfigSTAReqParamsType;


/*---------------------------------------------------------------------------
  WDI_UpdateBeaconParamsInfoType
---------------------------------------------------------------------------*/

typedef struct
{
   /*BSS Index of the BSS*/
   wpt_uint8      ucBssIdx;

    /*shortPreamble mode. HAL should update all the STA rates when it
    receives this message*/
    wpt_uint8 ucfShortPreamble;
    /* short Slot time.*/
    wpt_uint8 ucfShortSlotTime;
    /* Beacon Interval */
    wpt_uint16 usBeaconInterval;
    /*Protection related */
    wpt_uint8 ucllaCoexist;
    wpt_uint8 ucllbCoexist;
    wpt_uint8 ucllgCoexist;
    wpt_uint8 ucHt20MhzCoexist;
    wpt_uint8 ucllnNonGFCoexist;
    wpt_uint8 ucfLsigTXOPProtectionFullSupport;
    wpt_uint8 ucfRIFSMode;

    wpt_uint16 usChangeBitmap;
}WDI_UpdateBeaconParamsInfoType;



/*---------------------------------------------------------------------------
  WDI_UpdateBeaconParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Update Beacon Params  Info*/
  WDI_UpdateBeaconParamsInfoType  wdiUpdateBeaconParamsInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_UpdateBeaconParamsType;

/*---------------------------------------------------------------------------
  WDI_SendBeaconParamsInfoType
---------------------------------------------------------------------------*/

typedef struct {

   /*BSSID of the BSS*/
   wpt_macAddr  macBSSID;

   /* Beacon data */
   wpt_uint8    beacon[WDI_BEACON_TEMPLATE_SIZE];     

   /* length of the template */
   wpt_uint32   beaconLength;

   /* IM IE offset from the beginning of the template.*/
   wpt_uint32   timIeOffset; 
} WDI_SendBeaconParamsInfoType;

/*---------------------------------------------------------------------------
  WDI_SendBeaconParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Send Beacon Params  Info*/
  WDI_SendBeaconParamsInfoType  wdiSendBeaconParamsInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_SendBeaconParamsType;

/*---------------------------------------------------------------------------
  WDI_LinkStateType
---------------------------------------------------------------------------*/
typedef enum 
{
    WDI_LINK_IDLE_STATE              = 0,
    WDI_LINK_PREASSOC_STATE          = 1,
    WDI_LINK_POSTASSOC_STATE         = 2,
    WDI_LINK_AP_STATE                = 3,
    WDI_LINK_IBSS_STATE              = 4,

    // BT-AMP Case
    WDI_LINK_BTAMP_PREASSOC_STATE    = 5,
    WDI_LINK_BTAMP_POSTASSOC_STATE   = 6,
    WDI_LINK_BTAMP_AP_STATE          = 7,
    WDI_LINK_BTAMP_STA_STATE         = 8,
    
    // Reserved for HAL internal use
    WDI_LINK_LEARN_STATE             = 9,
    WDI_LINK_SCAN_STATE              = 10,
    WDI_LINK_FINISH_SCAN_STATE       = 11,
    WDI_LINK_INIT_CAL_STATE          = 12,
    WDI_LINK_FINISH_CAL_STATE        = 13,
    WDI_LINK_MAX                     = 0x7FFFFFFF
} WDI_LinkStateType;

/*---------------------------------------------------------------------------
  WDI_SetLinkReqInfoType
---------------------------------------------------------------------------*/
typedef struct
{
  /*BSSID of the BSS*/
  wpt_macAddr           macBSSID;

  /*Link state*/
  WDI_LinkStateType     wdiLinkState;
}WDI_SetLinkReqInfoType;

/*---------------------------------------------------------------------------
  WDI_SetLinkReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Link Info*/
  WDI_SetLinkReqInfoType  wdiLinkInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_SetLinkReqParamsType;


/*---------------------------------------------------------------------------
  WDI_GetStatsReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
   /*BSSID of the BSS*/
  wpt_macAddr       macBSSID;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_GetStatsReqParamsType;

/*---------------------------------------------------------------------------
  WDI_StatsCountersType - TBD
---------------------------------------------------------------------------*/
typedef struct
{
  wpt_uint8   ucTxPkts; 

}WDI_StatsCountersType;

/*---------------------------------------------------------------------------
  WDI_GetStatsRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Result of the operation*/
  WDI_Status              wdiStatus;
  /*Statistics provided by RIVA (the exact set is TBD)*/
  WDI_StatsCountersType	  statsCounters;
}WDI_GetStatsRspParamsType;

/*---------------------------------------------------------------------------
  WDI_UpdateCfgReqParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*This is a TLV formatted buffer containing all config values that can
   be set through the DAL Interface
 
   The TLV is expected to be formatted like this:
 
   0            7          15              31 .... 
   | CONFIG ID  |  CFG LEN |   RESERVED    |  CFG BODY  |
 
   Or from a C construct point of VU it would look like this:
 
   typedef struct WPT_PACK_POST
   {
       #ifdef  WPT_BIG_ENDIAN
         wpt_uint32   ucCfgId:8;
         wpt_uint32   ucCfgLen:8;
         wpt_uint32   usReserved:16;
       #else
         wpt_uint32   usReserved:16;
         wpt_uint32   ucCfgLen:8;
         wpt_uint32   ucCfgId:8;
       #endif
 
       wpt_uint8   ucCfgBody[ucCfgLen];
   }WDI_ConfigType; 
 
   Multiple such tuplets are to be placed in the config buffer. One for
   each required configuration item:
 
     | TLV 1 |  TLV2 | ....
 
   The buffer is expected to be a flat area of memory that can be manipulated
   with standard memory routines.
 
   For more info please check paragraph 2.3.1 Config Structure from the
   HAL LLD.
 
   For a list of accepted configuration list and IDs please look up
   wlan_qct_dal_cfg.h
  */
  void*                   pConfigBuffer; 

  /*Length of the config buffer above*/
  wpt_uint32              uConfigBufferLen;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_UpdateCfgReqParamsType;

/*---------------------------------------------------------------------------
  WDI_UpdateProbeRspTemplateInfoType
---------------------------------------------------------------------------*/
//Default Beacon template size
#define WDI_PROBE_RSP_TEMPLATE_SIZE 0x180

#define WDI_PROBE_REQ_BITMAP_IE_LEN 8

typedef struct
{
  /*BSSID for which the Probe Template is to be used*/
  wpt_macAddr     macBSSID;

  /*Probe response template*/
  wpt_uint8      *pProbeRespTemplate[WDI_PROBE_RSP_TEMPLATE_SIZE];

  /*Template Len*/
  wpt_uint32      uProbeRespTemplateLen;

  /*Bitmap for the IEs that are to be handled at SLM level*/
  wpt_uint32      uaProxyProbeReqValidIEBmap[WDI_PROBE_REQ_BITMAP_IE_LEN];

}WDI_UpdateProbeRspTemplateInfoType;

/*---------------------------------------------------------------------------
  WDI_UpdateProbeRspParamsType
---------------------------------------------------------------------------*/
typedef struct
{
  /*Link Info*/
  WDI_UpdateProbeRspTemplateInfoType  wdiProbeRspTemplateInfo;

  /*Request status callback offered by UMAC - it is called if the current
    req has returned PENDING as status; it delivers the status of sending
    the message over the BUS */
  WDI_ReqStatusCb   wdiReqStatusCB; 

  /*The user data passed in by UMAC, it will be sent back when the above
    function pointer will be called */
  void*             pUserData;
}WDI_UpdateProbeRspTemplateParamsType;

/*----------------------------------------------------------------------------
 *   WDI callback types
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Start response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void (*WDI_StartRspCb)(WDI_StartRspParamsType*   pwdiRspParams,
                               void*                     pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Stop response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
 
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void (*WDI_StopRspCb)(WDI_Status   wdiStatus,
                              void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received an Init Scan response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_InitScanRspCb)(WDI_Status   wdiStatus,
                                   void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a StartScan response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiParams:  response params received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_StartScanRspCb)(WDI_StartScanRspParamsType*  wdiParams,
                                    void*                        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a End Scan response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_EndScanRspCb)(WDI_Status   wdiStatus,
                                  void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Finish Scan response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_FinishScanRspCb)(WDI_Status   wdiStatus,
                                     void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Join response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
 
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_JoinRspCb)(WDI_Status   wdiStatus,
                               void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Config BSS response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiConfigBSSRsp:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_ConfigBSSRspCb)(
                            WDI_ConfigBSSRspParamsType*   pwdiConfigBSSRsp,
                            void*                        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Del BSS response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiDelBSSRsp:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_DelBSSRspCb)(WDI_DelBSSRspParamsType*  pwdiDelBSSRsp,
                                 void*                     pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Post Assoc response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_PostAssocRspCb)(
                               WDI_PostAssocRspParamsType*  pwdiPostAssocRsp,
                               void*                        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Del STA response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiDelSTARsp:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_DelSTARspCb)(WDI_DelSTARspParamsType*   pwdiDelSTARsp,
                                 void*                      pUserData);



/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Set BSS Key response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_SetBSSKeyRspCb)(WDI_Status   wdiStatus,
                                    void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Remove BSS Key
   response from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_RemoveBSSKeyRspCb)(WDI_Status   wdiStatus,
                                       void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Set STA Key response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_SetSTAKeyRspCb)(WDI_Status   wdiStatus,
                                    void*        pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Remove STA Key
   response from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_RemoveSTAKeyRspCb)(WDI_Status   wdiStatus,
                                       void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Add TS response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_AddTsRspCb)(WDI_Status   wdiStatus,
                                void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Del TS response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_DelTsRspCb)(WDI_Status   wdiStatus,
                                void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received an Update EDCA Params
   response from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
 
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_UpdateEDCAParamsRspCb)(WDI_Status   wdiStatus,
                                           void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Add BA response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_AddBASessionRspCb)(
                            WDI_AddBASessionRspParamsType* wdiAddBASessionRsp,
                            void*                         pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Del BA response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
    
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_DelBARspCb)(WDI_Status   wdiStatus,
                                void*        pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Switch Ch response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_SwitchChRspCb)(WDI_SwitchCHRspParamsType*  pwdiSwitchChRsp,
                                   void*                       pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Config STA response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_ConfigSTARspCb)(
                            WDI_ConfigSTARspParamsType*  pwdiConfigSTARsp,
                            void*                        pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Set Link State
   response from the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_SetLinkStateRspCb)( WDI_Status   wdiStatus,
                                        void*        pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Get Stats response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiRspParams:  response parameters received from HAL
    pUserData:      user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_GetStatsRspCb)(WDI_GetStatsRspParamsType*  pwdiGetStatsRsp,
                                   void*                       pUserData);

 
/*---------------------------------------------------------------------------
   WDI_StartRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Update Cfg response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_UpdateCfgRspCb)(WDI_Status   wdiStatus,
                                    void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_AddBARspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a ADD BA response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_AddBARspCb)(WDI_AddBARspinfoType*   wdiAddBARsp,
                                    void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_TriggerBARspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a ADD BA response
   from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_TriggerBARspCb)(WDI_TriggerBARspParamsType*   wdiTriggerBARsp,
                                    void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_UpdateBeaconParamsRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Update Beacon Params response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_UpdateBeaconParamsRspCb)(WDI_Status   wdiStatus,
                                void*        pUserData);

/*---------------------------------------------------------------------------
   WDI_SendBeaconParamsRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Send Beacon Params response from
   the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  

    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_SendBeaconParamsRspCb)(WDI_Status   wdiStatus,
                                void*        pUserData);


/*---------------------------------------------------------------------------
   WDI_UpdateProbeRspTemplateRspCb
 
   DESCRIPTION   
 
   This callback is invoked by DAL when it has received a Probe RSP Template
   Update  response from the underlying device.
 
   PARAMETERS 

    IN
    wdiStatus:  response status received from HAL
    pUserData:  user data  
    
    
  
  RETURN VALUE 
    The result code associated with performing the operation
---------------------------------------------------------------------------*/
typedef void  (*WDI_UpdateProbeRspTemplateRspCb)(WDI_Status   wdiStatus,
                                               void*        pUserData);


/*========================================================================
 *     Function Declarations and Documentation
 ==========================================================================*/

/*======================================================================== 
 
                             INITIALIZATION APIs
 
==========================================================================*/

/**
 @brief WDI_Init is used to initialize the DAL.
 
 DAL will allocate all the resources it needs. It will open PAL, it will also
 open both the data and the control transport which in their turn will open
 DXE/SMD or any other drivers that they need. 
 
 @param pOSContext: pointer to the OS context provided by the UMAC
                    will be passed on to PAL on Open
        ppWDIGlobalCtx: output pointer of Global Context
        pWdiDevCapability: output pointer of device capability

 @return Result of the function call
*/
WDI_Status 
WDI_Init
( 
  void*                      pOSContext,
  void**                     ppWDIGlobalCtx,
  WDI_DeviceCapabilityType*  pWdiDevCapability
);

/**
 @brief WDI_Start will be called when the upper MAC is ready to
        commence operation with the WLAN Device. Upon the call
        of this API the WLAN DAL will pack and send a HAL Start
        message to the lower RIVA sub-system if the SMD channel
        has been fully opened and the RIVA subsystem is up.

         If the RIVA sub-system is not yet up and running DAL
         will queue the request for Open and will wait for the
         SMD notification before attempting to send down the
         message to HAL. 

 WDI_Init must have been called.

 @param wdiStartParams: the start parameters as specified by 
                      the Device Interface
  
        wdiStartRspCb: callback for passing back the response of
        the start operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_Start
(
  WDI_StartReqParamsType*  pwdiStartParams,
  WDI_StartRspCb           wdiStartRspCb,
  void*                    pUserData
);


/**
 @brief WDI_Stop will be called when the upper MAC is ready to
        stop any operation with the WLAN Device. Upon the call
        of this API the WLAN DAL will pack and send a HAL Stop
        message to the lower RIVA sub-system if the DAL Core is
        in started state.

         In state BUSY this request will be queued.
  
         Request will not be accepted in any other state. 

 WDI_Start must have been called.

 @param wdiStopParams: the stop parameters as specified by 
                      the Device Interface
  
        wdiStopRspCb: callback for passing back the response of
        the stop operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_Stop
(
  WDI_StopReqParamsType*  pwdiStopParams,
  WDI_StopRspCb           wdiStopRspCb,
  void*                   pUserData
);



/**
 @brief WDI_Close will be called when the upper MAC no longer 
        needs to interract with DAL. DAL will free its control
        block.
  
        It is only accepted in state STOPPED.  

 WDI_Stop must have been called.

 @param none
  
 @see WDI_Stop
 @return Result of the function call
*/
WDI_Status 
WDI_Close
(
  void
);

/*======================================================================== 
 
                             SCAN APIs
 
==========================================================================*/

/**
 @brief WDI_InitScanReq will be called when the upper MAC wants 
        the WLAN Device to get ready for a scan procedure. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Init Scan request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiInitScanParams: the init scan parameters as specified
                      by the Device Interface
  
        wdiInitScanRspCb: callback for passing back the response
        of the init scan operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_InitScanReq
(
  WDI_InitScanReqParamsType*  pwdiInitScanParams,
  WDI_InitScanRspCb           wdiInitScanRspCb,
  void*                       pUserData
);

/**
 @brief WDI_StartScanReq will be called when the upper MAC 
        wishes to change the Scan channel on the WLAN Device.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL Start Scan request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_InitScanReq must have been called.

 @param wdiStartScanParams: the start scan parameters as 
                      specified by the Device Interface
  
        wdiStartScanRspCb: callback for passing back the
        response of the start scan operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_InitScanReq
 @return Result of the function call
*/
WDI_Status 
WDI_StartScanReq
(
  WDI_StartScanReqParamsType*  pwdiStartScanParams,
  WDI_StartScanRspCb           wdiStartScanRspCb,
  void*                        pUserData
);


/**
 @brief WDI_EndScanReq will be called when the upper MAC is 
        wants to end scanning for a particular channel that it
        had set before by calling Scan Start on the WLAN Device.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL End Scan request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_StartScanReq must have been called.

 @param wdiEndScanParams: the end scan parameters as specified 
                      by the Device Interface
  
        wdiEndScanRspCb: callback for passing back the response
        of the end scan operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_StartScanReq
 @return Result of the function call
*/
WDI_Status 
WDI_EndScanReq
(
  WDI_EndScanReqParamsType* pwdiEndScanParams,
  WDI_EndScanRspCb          wdiEndScanRspCb,
  void*                     pUserData
);


/**
 @brief WDI_FinishScanReq will be called when the upper MAC has 
        completed the scan process on the WLAN Device. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Finish Scan Request request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_InitScanReq must have been called.

 @param wdiFinishScanParams: the finish scan  parameters as 
                      specified by the Device Interface
  
        wdiFinishScanRspCb: callback for passing back the
        response of the finish scan operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_InitScanReq
 @return Result of the function call
*/
WDI_Status 
WDI_FinishScanReq
(
  WDI_FinishScanReqParamsType* pwdiFinishScanParams,
  WDI_FinishScanRspCb          wdiFinishScanRspCb,
  void*                        pUserData
);

/*======================================================================== 
 
                          ASSOCIATION APIs
 
==========================================================================*/

/**
 @brief WDI_JoinReq will be called when the upper MAC is ready 
        to start an association procedure to a BSS. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Join request message to the lower RIVA sub-system if
        DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiJoinParams: the join parameters as specified by 
                      the Device Interface
  
        wdiJoinRspCb: callback for passing back the response of
        the join operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_JoinReq
(
  WDI_JoinReqParamsType* pwdiJoinParams,
  WDI_JoinRspCb          wdiJoinRspCb,
  void*                  pUserData
);

/**
 @brief WDI_ConfigBSSReq will be called when the upper MAC 
        wishes to configure the newly acquired or in process of
        being acquired BSS to the HW . Upon the call of this API
        the WLAN DAL will pack and send a HAL Config BSS request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_JoinReq must have been called.

 @param wdiConfigBSSParams: the config BSS parameters as 
                      specified by the Device Interface
  
        wdiConfigBSSRspCb: callback for passing back the
        response of the config BSS operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_JoinReq
 @return Result of the function call
*/
WDI_Status 
WDI_ConfigBSSReq
(
  WDI_ConfigBSSReqParamsType* pwdiConfigBSSParams,
  WDI_ConfigBSSRspCb          wdiConfigBSSRspCb,
  void*                       pUserData
);

/**
 @brief WDI_DelBSSReq will be called when the upper MAC is 
        dissasociating from the BSS and wishes to notify HW.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL Del BSS request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_ConfigBSSReq or WDI_PostAssocReq must have been called.

 @param wdiDelBSSParams: the del BSS parameters as specified by 
                      the Device Interface
  
        wdiDelBSSRspCb: callback for passing back the response
        of the del bss operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_ConfigBSSReq, WDI_PostAssocReq 
 @return Result of the function call
*/
WDI_Status 
WDI_DelBSSReq
(
  WDI_DelBSSReqParamsType* pwdiDelBSSParams,
  WDI_DelBSSRspCb          wdiDelBSSRspCb,
  void*                    pUserData
);

/**
 @brief WDI_PostAssocReq will be called when the upper MAC has 
        associated to a BSS and wishes to configure HW for
        associated state. Upon the call of this API the WLAN DAL
        will pack and send a HAL Post Assoc request message to
        the lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_JoinReq must have been called.

 @param wdiPostAssocReqParams: the assoc parameters as specified
                      by the Device Interface
  
        wdiPostAssocRspCb: callback for passing back the
        response of the post assoc operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_JoinReq
 @return Result of the function call
*/
WDI_Status 
WDI_PostAssocReq
(
  WDI_PostAssocReqParamsType* pwdiPostAssocReqParams,
  WDI_PostAssocRspCb          wdiPostAssocRspCb,
  void*                       pUserData
);

/**
 @brief WDI_DelSTAReq will be called when the upper MAC when an 
        association with another STA has ended and the station
        must be deleted from HW. Upon the call of this API the
        WLAN DAL will pack and send a HAL Del STA request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiDelSTAParams: the Del STA parameters as specified by 
                      the Device Interface
  
        wdiDelSTARspCb: callback for passing back the response
        of the del STA operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_DelSTAReq
(
  WDI_DelSTAReqParamsType* pwdiDelSTAParams,
  WDI_DelSTARspCb          wdiDelSTARspCb,
  void*                    pUserData
);

/*======================================================================== 
 
                             SECURITY APIs
 
==========================================================================*/

/**
 @brief WDI_SetBSSKeyReq will be called when the upper MAC ito 
        install a BSS encryption key on the HW. Upon the call of
        this API the WLAN DAL will pack and send a HAL Start
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiSetBSSKeyParams: the BSS Key set parameters as 
                      specified by the Device Interface
  
        wdiSetBSSKeyRspCb: callback for passing back the
        response of the set BSS Key operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetBSSKeyReq
(
  WDI_SetBSSKeyReqParamsType* pwdiSetBSSKeyParams,
  WDI_SetBSSKeyRspCb          wdiSetBSSKeyRspCb,
  void*                       pUserData
);


/**
 @brief WDI_RemoveBSSKeyReq will be called when the upper MAC to
        uninstall a BSS key from HW. Upon the call of this API
        the WLAN DAL will pack and send a HAL Remove BSS Key
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_SetBSSKeyReq must have been called.

 @param wdiRemoveBSSKeyParams: the remove BSS key parameters as 
                      specified by the Device Interface
  
        wdiRemoveBSSKeyRspCb: callback for passing back the
        response of the remove BSS key operation received from
        the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_SetBSSKeyReq
 @return Result of the function call
*/
WDI_Status 
WDI_RemoveBSSKeyReq
(
  WDI_RemoveBSSKeyReqParamsType* pwdiRemoveBSSKeyParams,
  WDI_RemoveBSSKeyRspCb          wdiRemoveBSSKeyRspCb,
  void*                          pUserData
);


/**
 @brief WDI_SetSTAKeyReq will be called when the upper MAC is 
        ready to install a STA(ast) encryption key in HW. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Set STA Key request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiSetSTAKeyParams: the set STA key parameters as 
                      specified by the Device Interface
  
        wdiSetSTAKeyRspCb: callback for passing back the
        response of the set STA key operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetSTAKeyReq
(
  WDI_SetSTAKeyReqParamsType* pwdiSetSTAKeyParams,
  WDI_SetSTAKeyRspCb          wdiSetSTAKeyRspCb,
  void*                       pUserData
);


/**
 @brief WDI_RemoveSTAKeyReq will be called when the upper MAC 
        wants to unistall a previously set STA key in HW. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Remove STA Key request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_SetSTAKeyReq must have been called.

 @param wdiRemoveSTAKeyParams: the remove STA key parameters as 
                      specified by the Device Interface
  
        wdiRemoveSTAKeyRspCb: callback for passing back the
        response of the remove STA key operation received from
        the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_SetSTAKeyReq
 @return Result of the function call
*/
WDI_Status 
WDI_RemoveSTAKeyReq
(
  WDI_RemoveSTAKeyReqParamsType* pwdiRemoveSTAKeyParams,
  WDI_RemoveSTAKeyRspCb          wdiRemoveSTAKeyRspCb,
  void*                          pUserData
);

/**
 @brief WDI_SetSTABcastKeyReq will be called when the upper MAC 
        wants to install a STA Bcast encryption key on the HW.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL Start request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiSetSTABcastKeyParams: the BSS Key set parameters as 
                      specified by the Device Interface
  
        wdiSetSTABcastKeyRspCb: callback for passing back the
        response of the set BSS Key operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetSTABcastKeyReq
(
  WDI_SetSTAKeyReqParamsType* pwdiSetSTABcastKeyParams,
  WDI_SetSTAKeyRspCb          wdiSetSTABcastKeyRspCb,
  void*                       pUserData
);


/**
 @brief WDI_RemoveSTABcastKeyReq will be called when the upper 
        MAC to uninstall a STA Bcast key from HW. Upon the call
        of this API the WLAN DAL will pack and send a HAL Remove
        STA Bcast Key request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_SetSTABcastKeyReq must have been called.

 @param pwdiRemoveSTABcastKeyParams: the remove BSS key 
                      parameters as specified by the Device
                      Interface
  
        wdiRemoveSTABcastKeyRspCb: callback for passing back the
        response of the remove STA Bcast key operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_SetSTABcastKeyReq
 @return Result of the function call
*/
WDI_Status 
WDI_RemoveSTABcastKeyReq
(
  WDI_RemoveSTAKeyReqParamsType* pwdiRemoveSTABcastKeyParams,
  WDI_RemoveSTAKeyRspCb          wdiRemoveSTABcastKeyRspCb,
  void*                          pUserData
);

/*======================================================================== 
 
                            QoS and BA APIs
 
==========================================================================*/

/**
 @brief WDI_AddTSReq will be called when the upper MAC to inform
        the device of a successful add TSpec negotiation. HW
        needs to receive the TSpec Info from the UMAC in order
        to configure properly the QoS data traffic. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Add TS request message to the lower RIVA sub-system if
        DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiAddTsReqParams: the add TS parameters as specified by
                      the Device Interface
  
        wdiAddTsRspCb: callback for passing back the response of
        the add TS operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_AddTSReq
(
  WDI_AddTSReqParamsType* pwdiAddTsReqParams,
  WDI_AddTsRspCb          wdiAddTsRspCb,
  void*                   pUserData
);



/**
 @brief WDI_DelTSReq will be called when the upper MAC has ended
        admission on a specific AC. This is to inform HW that
        QoS traffic parameters must be rest. Upon the call of
        this API the WLAN DAL will pack and send a HAL Del TS
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_AddTSReq must have been called.

 @param wdiDelTsReqParams: the del TS parameters as specified by
                      the Device Interface
  
        wdiDelTsRspCb: callback for passing back the response of
        the del TS operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddTSReq
 @return Result of the function call
*/
WDI_Status 
WDI_DelTSReq
(
  WDI_DelTSReqParamsType* pwdiDelTsReqParams,
  WDI_DelTsRspCb          wdiDelTsRspCb,
  void*                   pUserData
);



/**
 @brief WDI_UpdateEDCAParams will be called when the upper MAC 
        wishes to update the EDCA parameters used by HW for QoS
        data traffic. Upon the call of this API the WLAN DAL
        will pack and send a HAL Update EDCA Params request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiUpdateEDCAParams: the start parameters as specified 
                      by the Device Interface
  
        wdiUpdateEDCAParamsRspCb: callback for passing back the
        response of the start operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_UpdateEDCAParams
(
  WDI_UpdateEDCAParamsType*    pwdiUpdateEDCAParams,
  WDI_UpdateEDCAParamsRspCb    wdiUpdateEDCAParamsRspCb,
  void*                        pUserData
);



/**
 @brief WDI_AddBASessionReq will be called when the upper MAC has setup
        successfully a BA session and needs to notify the HW for
        the appropriate settings to take place. Upon the call of
        this API the WLAN DAL will pack and send a HAL Add BA
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiAddBAReqParams: the add BA parameters as specified by
                      the Device Interface
  
        wdiAddBARspCb: callback for passing back the response of
        the add BA operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_AddBASessionReq
(
  WDI_AddBASessionReqParamsType* pwdiAddBASessionReqParams,
  WDI_AddBASessionRspCb          wdiAddBASessionRspCb,
  void*                          pUserData
);


/**
 @brief WDI_DelBAReq will be called when the upper MAC wants to 
        inform HW that it has deleted a previously created BA
        session. Upon the call of this API the WLAN DAL will
        pack and send a HAL Del BA request message to the lower
        RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_AddBAReq must have been called.

 @param wdiDelBAReqParams: the del BA parameters as specified by
                      the Device Interface
  
        wdiDelBARspCb: callback for passing back the response of
        the del BA operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddBAReq
 @return Result of the function call
*/
WDI_Status 
WDI_DelBAReq
(
  WDI_DelBAReqParamsType* pwdiDelBAReqParams,
  WDI_DelBARspCb          wdiDelBARspCb,
  void*                   pUserData
);

/**
 @brief WDI_UpdateBeaconParamsReq will be called when the upper MAC wants to 
        inform HW that there is a change in the beacon parameters
         Upon the call of this API the WLAN DAL will
        pack and send a UpdateBeacon Params message to the lower
        RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_UpdateBeaconParamsReq must have been called.

 @param WDI_UpdateBeaconParamsType: the Update Beacon parameters as specified by
                      the Device Interface
  
        WDI_UpdateBeaconParamsRspCb: callback for passing back the response of
        the Update Beacon Params operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddBAReq
 @return Result of the function call
*/

WDI_Status 
WDI_UpdateBeaconParamsReq
(
  WDI_UpdateBeaconParamsType *   pwdiUpdateBeaconParams,
  WDI_UpdateBeaconParamsRspCb    wdiUpdateBeaconParamsRspCb,
  void*                          pUserData
);


/**
 @brief WDI_SendBeaconParamsReq will be called when the upper MAC wants to 
        update the beacon template to be transmitted as BT MAP STA/IBSS/Soft AP
         Upon the call of this API the WLAN DAL will
        pack and send the beacon Template  message to the lower
        RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_SendBeaconParamsReq must have been called.

 @param WDI_SendBeaconParamsType: the Update Beacon parameters as specified by
                      the Device Interface
  
        WDI_SendBeaconParamsRspCb: callback for passing back the response of
        the Send Beacon Params operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddBAReq
 @return Result of the function call
*/

WDI_Status 
WDI_SendBeaconParamsReq
(
  WDI_SendBeaconParamsType*    pwdiSendBeaconParams,
  WDI_SendBeaconParamsRspCb    wdiSendBeaconParamsRspCb,
  void*                        pUserData
);


/**
 @brief WDI_UpdateProbeRspTemplateReq will be called when the 
        upper MAC wants to update the probe response template to
        be transmitted as Soft AP
         Upon the call of this API the WLAN DAL will
        pack and send the probe rsp template  message to the
        lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 


 @param pwdiUpdateProbeRspParams: the Update Beacon parameters as 
                      specified by the Device Interface
  
        wdiSendBeaconParamsRspCb: callback for passing back the
        response of the Send Beacon Params operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddBAReq
 @return Result of the function call
*/

WDI_Status 
WDI_UpdateProbeRspTemplateReq
(
  WDI_UpdateProbeRspTemplateParamsType*    pwdiUpdateProbeRspParams,
  WDI_UpdateProbeRspTemplateRspCb          wdiSendBeaconParamsRspCb,
  void*                                  pUserData
);


/*======================================================================== 
 
                             CONTROL APIs
 
==========================================================================*/
/**
 @brief WDI_SwitchChReq will be called when the upper MAC wants 
        the WLAN HW to change the current channel of operation.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL Start request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiSwitchChReqParams: the switch ch parameters as 
                      specified by the Device Interface
  
        wdiSwitchChRspCb: callback for passing back the response
        of the switch ch operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_SwitchChReq
(
  WDI_SwitchChReqParamsType* pwdiSwitchChReqParams,
  WDI_SwitchChRspCb          wdiSwitchChRspCb,
  void*                      pUserData
);



/**
 @brief WDI_ConfigSTAReq will be called when the upper MAC 
        wishes to add or update a STA in HW. Upon the call of
        this API the WLAN DAL will pack and send a HAL Start
        message request message to the lower RIVA sub-system if
        DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiConfigSTAReqParams: the config STA parameters as 
                      specified by the Device Interface
  
        wdiConfigSTARspCb: callback for passing back the
        response of the config STA operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_ConfigSTAReq
(
  WDI_ConfigSTAReqParamsType* pwdiConfigSTAReqParams,
  WDI_ConfigSTARspCb          wdiConfigSTARspCb,
  void*                       pUserData
);

/**
 @brief WDI_SetLinkStateReq will be called when the upper MAC 
        wants to change the state of an ongoing link. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Start message request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_JoinReq must have been called.

 @param wdiSetLinkStateReqParams: the set link state parameters 
                      as specified by the Device Interface
  
        wdiSetLinkStateRspCb: callback for passing back the
        response of the set link state operation received from
        the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_JoinStartReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetLinkStateReq
(
  WDI_SetLinkReqParamsType* pwdiSetLinkStateReqParams,
  WDI_SetLinkStateRspCb     wdiSetLinkStateRspCb,
  void*                     pUserData
);


/**
 @brief WDI_GetStatsReq will be called when the upper MAC wants 
        to get statistics (MIB counters) from the device. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Start request message to the lower RIVA sub-system
        if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiGetStatsReqParams: the stats parameters to get as 
                      specified by the Device Interface
  
        wdiGetStatsRspCb: callback for passing back the response
        of the get stats operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_GetStatsReq
(
  WDI_GetStatsReqParamsType* pwdiGetStatsReqParams,
  WDI_GetStatsRspCb          wdiGetStatsRspCb,
  void*                      pUserData
);


/**
 @brief WDI_UpdateCfgReq will be called when the upper MAC when 
        it wishes to change the configuration of the WLAN
        Device. Upon the call of this API the WLAN DAL will pack
        and send a HAL Update CFG request message to the lower
        RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param wdiUpdateCfgReqParams: the update cfg parameters as 
                      specified by the Device Interface
  
        wdiUpdateCfgsRspCb: callback for passing back the
        response of the update cfg operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_UpdateCfgReq
(
  WDI_UpdateCfgReqParamsType* pwdiUpdateCfgReqParams,
  WDI_UpdateCfgRspCb          wdiUpdateCfgsRspCb,
  void*                       pUserData
);

/**
 @brief WDI_AddBAReq will be called when the upper MAC has setup
        successfully a BA session and needs to notify the HW for
        the appropriate settings to take place. Upon the call of
        this API the WLAN DAL will pack and send a HAL Add BA
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiAddBAReqParams: the add BA parameters as specified by
                      the Device Interface
  
        wdiAddBARspCb: callback for passing back the response of
        the add BA operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_AddBAReq
(
  WDI_AddBAReqParamsType* pwdiAddBAReqParams,
  WDI_AddBARspCb          wdiAddBARspCb,
  void*                   pUserData
);

/**
 @brief WDI_TriggerBAReq will be called when the upper MAC has setup
        successfully a BA session and needs to notify the HW for
        the appropriate settings to take place. Upon the call of
        this API the WLAN DAL will pack and send a HAL Add BA
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiAddBAReqParams: the add BA parameters as specified by
                      the Device Interface
  
        wdiAddBARspCb: callback for passing back the response of
        the add BA operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_TriggerBAReq
(
  WDI_TriggerBAReqParamsType* pwdiTriggerBAReqParams,
  WDI_TriggerBARspCb          wdiTriggerBARspCb,
  void*                       pUserData
);


/**
 @brief WDI_IsHwFrameTxTranslationCapable checks to see if HW 
        frame xtl is enabled for a particular STA.

 WDI_PostAssocReq must have been called.

 @param uSTAIdx: STA index 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
wpt_boolean WDI_IsHwFrameTxTranslationCapable
(
  wpt_uint8 uSTAIdx
);

/**
 @brief WDI_STATableInit - Initializes the STA tables. 
        Allocates the necesary memory.

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
  
 @see
 @return Result of the function call
*/

WDI_Status WDI_StubRunTest
(
   wpt_uint8   ucTestNo
)
;

#ifdef __cplusplus
 }
#endif 


#endif /* #ifndef WLAN_QCT_WDI_H */
