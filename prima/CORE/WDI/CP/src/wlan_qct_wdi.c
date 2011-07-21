/*===========================================================================

                       W L A N _ Q C T _ W D I. C

  OVERVIEW:

  This software unit holds the implementation of the WLAN Device Abstraction     
  Layer Interface.

  The functions externalized by this module are to be called by any upper     
  MAC implementation that wishes to use the WLAN Device.

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
2010-08-09    lti     Created module

===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "wlan_qct_wdi.h" 
#include "wlan_qct_wdi_i.h" 
#include "wlan_qct_wdi_sta.h" 
#include "wlan_qct_wdi_dp.h" 

#include "wlan_qct_wdi_cts.h" 

#include "wlan_qct_pal_api.h"
#include "wlan_qct_pal_type.h"
#include "wlan_qct_pal_status.h"
#include "wlan_qct_pal_sync.h"
#include "wlan_qct_pal_msg.h"
#include "wlan_qct_pal_trace.h"
#include "wlan_qct_pal_packet.h"

#include "wlan_qct_wdi_dts.h" 

#include "wlan_hal_msg.h"

#ifdef ANI_MANF_DIAG
#include "pttMsgApi.h"
#endif /* ANI_MANF_DIAG */

/*===========================================================================
   WLAN DAL Control Path Internal Data Definitions and Declarations 
 ===========================================================================*/
#define WDI_SET_POWER_STATE_TIMEOUT  10000 /* in msec a very high upper limit */

/*-------------------------------------------------------------------------- 
   WLAN DAL  State Machine
 --------------------------------------------------------------------------*/
WPT_STATIC const WDI_MainFsmEntryType wdiMainFSM[WDI_MAX_ST] = 
{
  /*WDI_INIT_ST*/
  {{
    WDI_MainStart,              /*WDI_START_EVENT*/
    NULL,                       /*WDI_STOP_EVENT*/
    WDI_MainReqBusy,            /*WDI_REQUEST_EVENT*/
    WDI_MainRspInit,            /*WDI_RESPONSE_EVENT*/
    WDI_MainClose               /*WDI_CLOSE_EVENT*/
  }},

  /*WDI_STARTED_ST*/
  {{
    WDI_MainStartStarted,       /*WDI_START_EVENT*/
    WDI_MainStopStarted,        /*WDI_STOP_EVENT*/
    WDI_MainReqStarted,         /*WDI_REQUEST_EVENT*/
    WDI_MainRsp,                /*WDI_RESPONSE_EVENT*/
    NULL                        /*WDI_CLOSE_EVENT*/
  }},

  /*WDI_STOPPED_ST*/
  {{
    WDI_MainStart,              /*WDI_START_EVENT*/
    NULL,                       /*WDI_STOP_EVENT*/
    NULL,                       /*WDI_REQUEST_EVENT*/
    WDI_MainRsp,                /*WDI_RESPONSE_EVENT*/
    WDI_MainClose               /*WDI_CLOSE_EVENT*/
  }},

  /*WDI_BUSY_ST*/
  {{
    WDI_MainStartBusy,          /*WDI_START_EVENT*/
    WDI_MainStopBusy,           /*WDI_STOP_EVENT*/
    WDI_MainReqBusy,            /*WDI_REQUEST_EVENT*/
    WDI_MainRsp,                /*WDI_RESPONSE_EVENT*/
    WDI_MainCloseBusy           /*WDI_CLOSE_EVENT*/
  }}
};

/*--------------------------------------------------------------------------- 
  DAL Request Processing Array  - the functions in this table will only be
  called when the processing of the specific request is allowed by the
  Main FSM 
 ---------------------------------------------------------------------------*/
WDI_ReqProcFuncType  pfnReqProcTbl[WDI_MAX_REQ] = 
{
  /*INIT*/
  WDI_ProcessStartReq,      /* WDI_START_REQ  */
  WDI_ProcessStopReq,       /* WDI_STOP_REQ  */
  WDI_ProcessCloseReq,      /* WDI_CLOSE_REQ  */

  /*SCAN*/
  WDI_ProcessInitScanReq,   /* WDI_INIT_SCAN_REQ  */
  WDI_ProcessStartScanReq,  /* WDI_START_SCAN_REQ  */
  WDI_ProcessEndScanReq,    /* WDI_END_SCAN_REQ  */
  WDI_ProcessFinishScanReq, /* WDI_FINISH_SCAN_REQ  */

  /*ASSOCIATION*/
  WDI_ProcessJoinReq,       /* WDI_JOIN_REQ  */
  WDI_ProcessConfigBSSReq,  /* WDI_CONFIG_BSS_REQ  */
  WDI_ProcessDelBSSReq,     /* WDI_DEL_BSS_REQ  */
  WDI_ProcessPostAssocReq,  /* WDI_POST_ASSOC_REQ  */
  WDI_ProcessDelSTAReq,     /* WDI_DEL_STA_REQ  */

  /* Security */
  WDI_ProcessSetBssKeyReq,        /* WDI_SET_BSS_KEY_REQ  */
  WDI_ProcessRemoveBssKeyReq,     /* WDI_RMV_BSS_KEY_REQ  */
  WDI_ProcessSetStaKeyReq,        /* WDI_SET_STA_KEY_REQ  */
  WDI_ProcessRemoveStaKeyReq,     /* WDI_RMV_BSS_KEY_REQ  */

  /* QoS and BA APIs */
  WDI_ProcessAddTSpecReq,          /* WDI_ADD_TS_REQ  */
  WDI_ProcessDelTSpecReq,          /* WDI_DEL_TS_REQ  */
  WDI_ProcessUpdateEDCAParamsReq,  /* WDI_UPD_EDCA_PRMS_REQ  */
  WDI_ProcessAddBASessionReq,      /* WDI_ADD_BA_SESSION_REQ  */
  WDI_ProcessDelBAReq,             /* WDI_DEL_BA_REQ  */

  /* Miscellaneous Control APIs	*/
  WDI_ProcessChannelSwitchReq,     /* WDI_CH_SWITCH_REQ  */
  WDI_ProcessConfigStaReq,         /* WDI_CONFIG_STA_REQ  */
  WDI_ProcessSetLinkStateReq,      /* WDI_SET_LINK_ST_REQ  */
  WDI_ProcessGetStatsReq,          /* WDI_GET_STATS_REQ  */
  WDI_ProcessUpdateCfgReq,         /* WDI_UPDATE_CFG_REQ  */

  /*BA APIs*/
  WDI_ProcessAddBAReq,             /* WDI_ADD_BA_REQ  */
  WDI_ProcessTriggerBAReq,         /* WDI_TRIGGER_BA_REQ  */

  /*Beacon processing APIs*/
  WDI_ProcessUpdateBeaconParamsReq, /* WDI_UPD_BCON_PRMS_REQ */
  WDI_ProcessSendBeaconParamsReq, /* WDI_SND_BCON_REQ */

  WDI_ProcessUpdateProbeRspTemplateReq, /* WDI_UPD_PROBE_RSP_TEMPLATE_REQ */
  WDI_ProcessSetStaBcastKeyReq,        /* WDI_SET_STA_BCAST_KEY_REQ  */
  WDI_ProcessRemoveStaBcastKeyReq,     /* WDI_RMV_STA_BCAST_KEY_REQ  */
#ifdef WLAN_FEATURE_VOWIFI
  WDI_ProcessSetMaxTxPowerReq,         /*WDI_SET_MAX_TX_POWER_REQ*/
#else
  NULL,
#endif
#ifdef WLAN_FEATURE_P2P
  WDI_ProcessP2PGONOAReq,              /* WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ */
#else
  NULL,
#endif
  /* PowerSave APIs */
  WDI_ProcessEnterImpsReq,         /* WDI_ENTER_IMPS_REQ  */
  WDI_ProcessExitImpsReq,          /* WDI_EXIT_IMPS_REQ  */
  WDI_ProcessEnterBmpsReq,         /* WDI_ENTER_BMPS_REQ  */
  WDI_ProcessExitBmpsReq,          /* WDI_EXIT_BMPS_REQ  */
  WDI_ProcessEnterUapsdReq,        /* WDI_ENTER_UAPSD_REQ  */
  WDI_ProcessExitUapsdReq,         /* WDI_EXIT_UAPSD_REQ  */
  WDI_ProcessSetUapsdAcParamsReq,  /* WDI_SET_UAPSD_PARAM_REQ  */
  WDI_ProcessUpdateUapsdParamsReq, /* WDI_UPDATE_UAPSD_PARAM_REQ  */
  WDI_ProcessConfigureRxpFilterReq, /* WDI_CONFIGURE_RXP_FILTER_REQ  */
  WDI_ProcessSetBeaconFilterReq,   /* WDI_SET_BEACON_FILTER_REQ  */
  WDI_ProcessRemBeaconFilterReq,   /* WDI_REM_BEACON_FILTER_REQ  */
  WDI_ProcessSetRSSIThresholdsReq, /* WDI_SET_RSSI_THRESHOLDS_REQ  */
  WDI_ProcessHostOffloadReq,       /* WDI_HOST_OFFLOAD_REQ  */
  WDI_ProcessWowlAddBcPtrnReq,     /* WDI_WOWL_ADD_BC_PTRN_REQ  */
  WDI_ProcessWowlDelBcPtrnReq,     /* WDI_WOWL_DEL_BC_PTRN_REQ  */
  WDI_ProcessWowlEnterReq,         /* WDI_WOWL_ENTER_REQ  */
  WDI_ProcessWowlExitReq,          /* WDI_WOWL_EXIT_REQ  */
  WDI_ProcessConfigureAppsCpuWakeupStateReq, /* WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ  */
  /*NV Download APIs*/
  WDI_ProcessNvDownloadReq,      /* WDI_NV_DOWNLOAD_REQ*/
  WDI_ProcessFlushAcReq,           /* WDI_FLUSH_AC_REQ  */
  WDI_ProcessBtAmpEventReq,        /* WDI_BTAMP_EVENT_REQ  */
#ifdef WLAN_FEATURE_VOWIFI_11R
  WDI_ProcessAggrAddTSpecReq,      /* WDI_AGGR_ADD_TS_REQ */
#else
  NULL,
#endif /* WLAN_FEATURE_VOWIFI_11R */

#ifdef ANI_MANF_DIAG
  WDI_ProcessFTMCommandReq,            /* WDI_FTM_CMD_REQ */
#else
  NULL,
#endif /* ANI_MANF_DIAG */
};


/*--------------------------------------------------------------------------- 
  DAL Request Processing Array  - the functions in this table will only be
  called when the processing of the specific request is allowed by the
  Main FSM 
 ---------------------------------------------------------------------------*/
WDI_RspProcFuncType  pfnRspProcTbl[WDI_MAX_RESP] = 
{
  /*INIT*/
  WDI_ProcessStartRsp,            /* WDI_START_RESP  */
  WDI_ProcessStopRsp,             /* WDI_STOP_RESP  */
  WDI_ProcessCloseRsp,            /* WDI_CLOSE_RESP  */

  /*SCAN*/
  WDI_ProcessInitScanRsp,         /* WDI_INIT_SCAN_RESP  */
  WDI_ProcessStartScanRsp,        /* WDI_START_SCAN_RESP  */
  WDI_ProcessEndScanRsp,          /* WDI_END_SCAN_RESP  */
  WDI_ProcessFinishScanRsp,       /* WDI_FINISH_SCAN_RESP  */

  /* ASSOCIATION*/
  WDI_ProcessJoinRsp,             /* WDI_JOIN_RESP  */
  WDI_ProcessConfigBSSRsp,        /* WDI_CONFIG_BSS_RESP  */
  WDI_ProcessDelBSSRsp,           /* WDI_DEL_BSS_RESP  */
  WDI_ProcessPostAssocRsp,        /* WDI_POST_ASSOC_RESP  */
  WDI_ProcessDelSTARsp,           /* WDI_DEL_STA_RESP  */

  /* Security */
  WDI_ProcessSetBssKeyRsp,        /* WDI_SET_BSS_KEY_RESP  */
  WDI_ProcessRemoveBssKeyRsp,     /* WDI_RMV_BSS_KEY_RESP  */
  WDI_ProcessSetStaKeyRsp,        /* WDI_SET_STA_KEY_RESP  */
  WDI_ProcessRemoveStaKeyRsp,     /* WDI_RMV_BSS_KEY_RESP  */

  /* QoS and BA APIs */
  WDI_ProcessAddTSpecRsp,          /* WDI_ADD_TS_RESP  */
  WDI_ProcessDelTSpecRsp,          /* WDI_DEL_TS_RESP  */
  WDI_ProcessUpdateEDCAParamsRsp,  /* WDI_UPD_EDCA_PRMS_RESP  */
  WDI_ProcessAddBASessionRsp,      /* WDI_ADD_BA_SESSION_RESP  */
  WDI_ProcessDelBARsp,             /* WDI_DEL_BA_RESP  */

  /* Miscellaneous Control APIs	*/
  WDI_ProcessChannelSwitchRsp,     /* WDI_CH_SWITCH_RESP  */
  WDI_ProcessConfigStaRsp,         /* WDI_CONFIG_STA_RESP  */
  WDI_ProcessSetLinkStateRsp,      /* WDI_SET_LINK_ST_RESP  */
  WDI_ProcessGetStatsRsp,          /* WDI_GET_STATS_RESP  */
  WDI_ProcessUpdateCfgRsp,         /* WDI_UPDATE_CFG_RESP  */

  /* BA APIs*/
  WDI_ProcessAddBARsp,             /* WDI_ADD_BA_RESP  */
  WDI_ProcessTriggerBARsp,         /* WDI_TRIGGER_BA_RESP  */
  
  /* IBSS APIs*/
  WDI_ProcessUpdateBeaconParamsRsp, /* WDI_UPD_BCON_PRMS_RSP */
  WDI_ProcessSendBeaconParamsRsp,   /* WDI_SND_BCON_RSP */

  /*Soft AP APIs*/
  WDI_ProcessUpdateProbeRspTemplateRsp,/*WDI_UPD_PROBE_RSP_TEMPLATE_RESP */
  WDI_ProcessSetStaBcastKeyRsp,        /*WDI_SET_STA_BCAST_KEY_RESP */
  WDI_ProcessRemoveStaBcastKeyRsp,     /*WDI_RMV_STA_BCAST_KEY_RESP */
#ifdef WLAN_FEATURE_VOWIFI
  WDI_ProcessSetMaxTxPowerRsp,         /*WDI_SET_MAX_TX_POWER_RESP */
#else
  NULL,
#endif

  /* PowerSave APIs */
  WDI_ProcessEnterImpsRsp,         /* WDI_ENTER_IMPS_RESP  */
  WDI_ProcessExitImpsRsp,          /* WDI_EXIT_IMPS_RESP  */
  WDI_ProcessEnterBmpsRsp,         /* WDI_ENTER_BMPS_RESP  */
  WDI_ProcessExitBmpsRsp,          /* WDI_EXIT_BMPS_RESP  */
  WDI_ProcessEnterUapsdRsp,        /* WDI_ENTER_UAPSD_RESP  */
  WDI_ProcessExitUapsdRsp,         /* WDI_EXIT_UAPSD_RESP  */
  WDI_ProcessSetUapsdAcParamsRsp,  /* WDI_SET_UAPSD_PARAM_RESP  */
  WDI_ProcessUpdateUapsdParamsRsp, /* WDI_UPDATE_UAPSD_PARAM_RESP  */
  WDI_ProcessConfigureRxpFilterRsp,/* WDI_CONFIGURE_RXP_FILTER_RESP  */
  WDI_ProcessSetBeaconFilterRsp,   /* WDI_SET_BEACON_FILTER_RESP  */
  WDI_ProcessRemBeaconFilterRsp,   /* WDI_REM_BEACON_FILTER_RESP  */
  WDI_ProcessSetRSSIThresoldsRsp,  /* WDI_SET_RSSI_THRESHOLDS_RESP  */
  WDI_ProcessHostOffloadRsp,       /* WDI_HOST_OFFLOAD_RESP  */
  WDI_ProcessWowlAddBcPtrnRsp,     /* WDI_WOWL_ADD_BC_PTRN_RESP  */
  WDI_ProcessWowlDelBcPtrnRsp,     /* WDI_WOWL_DEL_BC_PTRN_RESP  */
  WDI_ProcessWowlEnterRsp,         /* WDI_WOWL_ENTER_RESP  */
  WDI_ProcessWowlExitRsp,          /* WDI_WOWL_EXIT_RESP  */
  WDI_ProcessConfigureAppsCpuWakeupStateRsp, /* WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP  */
  

  WDI_ProcessNvDownloadRsp, /* WDI_NV_DOWNLOAD_RESP*/

  WDI_ProcessFlushAcRsp,           /* WDI_FLUSH_AC_RESP  */
  WDI_ProcessBtAmpEventRsp,        /* WDI_BTAMP_EVENT_RESP  */
#ifdef WLAN_FEATURE_VOWIFI_11R
  WDI_ProcessAggrAddTSpecRsp,       /* WDI_ADD_TS_RESP  */
#else
  NULL,
#endif /* WLAN_FEATURE_VOWIFI_11R */
  /*Indications*/
  WDI_ProcessLowRSSIInd,            /* WDI_HAL_RSSI_NOTIFICATION_IND  */
  WDI_ProcessMissedBeaconInd,       /* WDI_HAL_MISSED_BEACON_IND  */
  WDI_ProcessUnkAddrFrameInd,       /* WDI_HAL_UNKNOWN_ADDR2_FRAME_RX_IND  */
  WDI_ProcessMicFailureInd,         /* WDI_HAL_MIC_FAILURE_IND  */
  WDI_ProcessFatalErrorInd,         /* WDI_HAL_FATAL_ERROR_IND  */
  WDI_ProcessDelSTAInd,             /* WDI_HAL_DEL_STA_IND  */

#ifdef WLAN_FEATURE_P2P
  WDI_ProcessP2PGONOARsp,           /*WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP */
#else
  NULL,
#endif

#ifdef ANI_MANF_DIAG
  WDI_ProcessFTMCommadRsp,          /* WDI_FTM_CMD_RESP */
#else
  NULL,
#endif /* ANI_MANF_DIAG */
};


/*--------------------------------------------------------------------------- 
  WLAN DAL Global Control Block
 ---------------------------------------------------------------------------*/
WDI_ControlBlockType  gWDICb; 
static wpt_uint8      gWDIInitialized = eWLAN_PAL_FALSE;

const wpt_uint8 szTransportChName[] = "WLAN_CTRL"; 

/*Helper routine for retrieving the PAL Context from WDI*/
WPT_INLINE 
void* WDI_GET_PAL_CTX( void )
{
  return gWDICb.pPALContext; 
}/*WDI_GET_PAL_CTX*/

/*============================================================================ 
  Helper inline convertors
 ============================================================================*/
/*Convert WDI driver type into HAL driver type*/
WPT_STATIC WPT_INLINE WDI_Status
WDI_HAL_2_WDI_STATUS
(
  eHalStatus halStatus
);

/*Convert WDI request type into HAL request type*/
WPT_STATIC WPT_INLINE tHalHostMsgType
WDI_2_HAL_REQ_TYPE
(
  WDI_RequestEnumType    wdiReqType
);

/*Convert WDI response type into HAL response type*/
WPT_STATIC WPT_INLINE WDI_ResponseEnumType
HAL_2_WDI_RSP_TYPE
(
  tHalHostMsgType halMsg
);

/*Convert WDI driver type into HAL driver type*/
WPT_STATIC WPT_INLINE tDriverType
WDI_2_HAL_DRV_TYPE
(
  WDI_DriverType wdiDriverType
);

/*Convert WDI stop reason into HAL stop reason*/
WPT_STATIC WPT_INLINE tHalStopType
WDI_2_HAL_STOP_REASON
(
  WDI_StopType wdiStopType
);

/*Convert WDI scan mode type into HAL scan mode type*/
WPT_STATIC WPT_INLINE eHalSysMode
WDI_2_HAL_SCAN_MODE
(
  WDI_ScanMode wdiScanMode
);

/*Convert WDI sec ch offset into HAL sec ch offset type*/
WPT_STATIC WPT_INLINE tSirMacHTSecondaryChannelOffset
WDI_2_HAL_SEC_CH_OFFSET
(
  WDI_HTSecondaryChannelOffset wdiSecChOffset
);

/*Convert WDI BSS type into HAL BSS type*/
WPT_STATIC WPT_INLINE tSirBssType
WDI_2_HAL_BSS_TYPE
(
  WDI_BssType wdiBSSType
);

/*Convert WDI NW type into HAL NW type*/
WPT_STATIC WPT_INLINE tSirNwType
WDI_2_HAL_NW_TYPE
(
  WDI_NwType wdiNWType
);

/*Convert WDI chanel bonding type into HAL cb type*/
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_CB_STATE
(
  WDI_PhyChanBondState wdiCbState
);

/*Convert WDI chanel bonding type into HAL cb type*/
WPT_STATIC WPT_INLINE tSirMacHTOperatingMode
WDI_2_HAL_HT_OPER_MODE
(
  WDI_HTOperatingMode wdiHTOperMode
);

/*Convert WDI mimo PS type into HAL mimo PS type*/
WPT_STATIC WPT_INLINE tSirMacHTMIMOPowerSaveState
WDI_2_HAL_MIMO_PS
(
  WDI_HTMIMOPowerSaveState wdiHTOperMode
);

/*Convert WDI ENC type into HAL ENC type*/
WPT_STATIC WPT_INLINE tAniEdType
WDI_2_HAL_ENC_TYPE
(
  WDI_EncryptType wdiEncType
);

/*Convert WDI WEP type into HAL WEP type*/
WPT_STATIC WPT_INLINE tAniWepType
WDI_2_HAL_WEP_TYPE
(
  WDI_WepType  wdiWEPType
);

/*Convert WDI Link State into HAL Link State*/
WPT_STATIC WPT_INLINE tSirLinkState
WDI_2_HAL_LINK_STATE
(
  WDI_LinkStateType  wdiLinkState
);

/*Translate a STA Context from WDI into HAL*/ 
WPT_STATIC WPT_INLINE 
void
WDI_CopyWDIStaCtxToHALStaCtx
( 
  tConfigStaParams*          phalConfigSta,
  WDI_ConfigStaReqInfoType*  pwdiConfigSta
);
 
/*Translate a Rate set info from WDI into HAL*/ 
WPT_STATIC WPT_INLINE void 
WDI_CopyWDIRateSetToHALRateSet
( 
  tSirMacRateSet* pHalRateSet,
  WDI_RateSet*	  pwdiRateSet
);

/*Translate an EDCA Parameter Record from WDI into HAL*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIEDCAParamsToHALEDCAParams
( 
  tSirMacEdcaParamRecord* phalEdcaParam,
  WDI_EdcaParamRecord*    pWDIEdcaParam
);

/*Copy a management frame header from WDI fmt into HAL fmt*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr
(
  tSirMacMgmtHdr* pmacMgmtHdr,
  WDI_MacMgmtHdr* pwdiMacMgmtHdr
);

/*Copy config bss parameters from WDI fmt into HAL fmt*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIConfigBSSToHALConfigBSS
(
  tConfigBssParams*         phalConfigBSS,
  WDI_ConfigBSSReqInfoType* pwdiConfigBSS
);

/*Extract the request CB function and user data from a request structure 
  pointed to by user data */
WPT_STATIC WPT_INLINE void
WDI_ExtractRequestCBFromEvent
(
  WDI_EventInfoType* pEvent,
  WDI_ReqStatusCb*   ppfnReqCB, 
  void**             ppUserData
);

wpt_uint8
WDI_FindEmptySession
( 
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType**    ppSession
);

void
WDI_AddBcastSTAtoSTATable
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_AddStaParams *     staParams,
  wpt_uint16             usBcastStaIdx
);

WDI_Status WDI_SendNvBlobReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
);

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
  WDI_DeviceCapabilityType*  pWdiDevCapability,
  unsigned int               driverType
)
{
  wpt_uint8               i;
  wpt_status              wptStatus; 
  WCTS_TransportCBsType   wctsCBs; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---------------------------------------------------------------------
    Sanity check
  ---------------------------------------------------------------------*/
  if (( NULL == ppWDIGlobalCtx ) || ( NULL == pWdiDevCapability ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid input parameters in WDI_Init");

    return WDI_STATUS_E_FAILURE; 
  }

  /*---------------------------------------------------------------------
    Check to see if the module has already been initialized or not 
  ---------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE != gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI module already initialized - return");

    return WDI_STATUS_SUCCESS; 
  }

  /*Module is now initialized - this flag is to ensure the fact that multiple
   init will not happen on WDI
   !! - potential race does exist because read and set are not atomic,
   however an atomic operation would be closely here - reanalize if necessary*/
  gWDIInitialized = eWLAN_PAL_TRUE; 

    /*Setup the control block */
  WDI_CleanCB(&gWDICb);
  gWDICb.pOSContext = pOSContext; 

  /*Setup the STA Table*/
  WDI_STATableInit(&gWDICb);

  /*------------------------------------------------------------------------
    Open the PAL
   ------------------------------------------------------------------------*/
  wptStatus =  wpalOpen(&gWDICb.pPALContext, pOSContext);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI Init failed to open PAL");

    WDI_ASSERT(0); 
    return WDI_STATUS_E_FAILURE; 
  }

  /*Initialize main synchro mutex - it will be used to ensure integrity of
   the main WDI Control Block*/
  wptStatus =  wpalMutexInit(&gWDICb.wptMutex);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Failed to init mutext %d", wptStatus);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE; 
  }

  /*Initialize the response timer - it will be used to time all messages
    expected as response from device*/
  wptStatus = wpalTimerInit( &gWDICb.wptResponseTimer, 
                             WDI_ResponseTimerCB, 
                             &gWDICb);

  /* Initialize the  WDI Pending Request Queue*/
  wpal_list_init(&(gWDICb.wptPendingQueue), pNode);
  /*Initialize the BSS seesions pending Queue */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wpal_list_init(&(gWDICb.aBSSSessions[i].wptPendingQueue), pNode);
  }
  /*------------------------------------------------------------------------
    Initialize the Data Path Utility Module
   ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_DP_UtilsInit(&gWDICb))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI Init failed to initialize the DP Util Module");

    WDI_ASSERT(0); 
    return WDI_STATUS_E_FAILURE; 
  }

  /* Init Set power state event */
  wptStatus = wpalEventInit(&gWDICb.setPowerStateEvent);
  if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus ) 
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "WDI Init failed to initialize an event");

     WDI_ASSERT(0); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*------------------------------------------------------------------------
    Open the Transport Services for Control and Data 
   ------------------------------------------------------------------------*/
  wctsCBs.wctsNotifyCB      = WDI_NotifyMsgCTSCB;
  wctsCBs.wctsNotifyCBData  = &gWDICb;
  wctsCBs.wctsRxMsgCB       = WDI_RXMsgCTSCB; 
  wctsCBs.wctsRxMsgCBData   = &gWDICb;

  gWDICb.bCTOpened          = eWLAN_PAL_FALSE; 
  gWDICb.wctsHandle = WCTS_OpenTransport( szTransportChName ,
                                          WDI_CT_CHANNEL_SIZE, 
                                          &wctsCBs ); 

  gWDICb.driverMode = (tDriverType)driverType;
  /* FTM mode not need to open Transport Driver */
  if(eDRIVER_TYPE_MFG != (tDriverType)driverType)
  {  
    /*------------------------------------------------------------------------
  	Open the Data Transport
     ------------------------------------------------------------------------*/
    if(eWLAN_PAL_STATUS_SUCCESS != WDTS_openTransport(&gWDICb))
    {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to initialize the DT Transport");

      WDI_ASSERT(0); 
      return WDI_STATUS_E_FAILURE; 
    }
  }

  /*The WDI is initialized - set state to init */
  gWDICb.uGlobalState = WDI_INIT_ST; 

  /*Send the context as a ptr to the global WDI Control Block*/
  *ppWDIGlobalCtx = &gWDICb;

  /*Fill in the device capabilities*/
  pWdiDevCapability->bFrameXtlSupported = eWLAN_PAL_FALSE; 
  pWdiDevCapability->ucMaxSTASupported  = gWDICb.ucMaxStations;
  pWdiDevCapability->ucMaxBSSSupported  = gWDICb.ucMaxBssids;
  return WDI_STATUS_SUCCESS; 
}/*WDI_Init*/;

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_START_REQ;
  wdiEventData.pEventData      = pwdiStartParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiStartParams); 
  wdiEventData.pCBfnc          = wdiStartRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_Start*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_STOP_REQ;
  wdiEventData.pEventData      = pwdiStopParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiStopParams); 
  wdiEventData.pCBfnc          = wdiStopRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_STOP_EVENT, &wdiEventData);

}/*WDI_Stop*/



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
)
{
  wpt_uint8              i;
  WDI_EventInfoType      wdiEventData;
  wpt_status              wptStatus; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }


  /*destroy the response timer */
  wptStatus = wpalTimerDelete( &gWDICb.wptResponseTimer);

  /* destroy the  WDI Pending Request Queue*/
  wpal_list_destroy(&(gWDICb.wptPendingQueue), pNode);

  /*destroy the BSS seesions pending Queue */
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wpal_list_destroy(&(gWDICb.aBSSSessions[i].wptPendingQueue), pNode);
  }
  /*------------------------------------------------------------------------
    Closes the Data Path Utility Module
   ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_DP_UtilsExit(&gWDICb))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "WDI Init failed to close the DP Util Module");

    WDI_ASSERT(0); 
  }

   /* Destroy the event */
   wptStatus = wpalEventDelete(&gWDICb.setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Close failed to destroy an event");

      WDI_ASSERT(0); 
   }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_CLOSE_REQ;
  wdiEventData.pEventData      = NULL; 
  wdiEventData.uEventDataSize  = 0; 
  wdiEventData.pCBfnc          = NULL; 
  wdiEventData.pUserData       = NULL;

  gWDIInitialized = eWLAN_PAL_FALSE;

  return WDI_PostMainEvent(&gWDICb, WDI_CLOSE_EVENT, &wdiEventData);

}/*WDI_Close*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_INIT_SCAN_REQ;
  wdiEventData.pEventData      = pwdiInitScanParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiInitScanParams); 
  wdiEventData.pCBfnc          = wdiInitScanRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_InitScanReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_START_SCAN_REQ;
  wdiEventData.pEventData      = pwdiStartScanParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiStartScanParams); 
  wdiEventData.pCBfnc          = wdiStartScanRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_StartScanReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_END_SCAN_REQ;
  wdiEventData.pEventData      = pwdiEndScanParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiEndScanParams); 
  wdiEventData.pCBfnc          = wdiEndScanRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_EndScanReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_FINISH_SCAN_REQ;
  wdiEventData.pEventData      = pwdiFinishScanParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiFinishScanParams); 
  wdiEventData.pCBfnc          = wdiFinishScanRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_FinishScanReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_JOIN_REQ;
  wdiEventData.pEventData      = pwdiJoinParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiJoinParams); 
  wdiEventData.pCBfnc          = wdiJoinRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_JoinReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_CONFIG_BSS_REQ;
  wdiEventData.pEventData      = pwdiConfigBSSParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiConfigBSSParams); 
  wdiEventData.pCBfnc          = wdiConfigBSSRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_ConfigBSSReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_DEL_BSS_REQ;
  wdiEventData.pEventData      = pwdiDelBSSParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelBSSParams); 
  wdiEventData.pCBfnc          = wdiDelBSSRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_DelBSSReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_POST_ASSOC_REQ;
  wdiEventData.pEventData      = pwdiPostAssocReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiPostAssocReqParams); 
  wdiEventData.pCBfnc          = wdiPostAssocRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_PostAssocReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_DEL_STA_REQ;
  wdiEventData.pEventData      = pwdiDelSTAParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelSTAParams); 
  wdiEventData.pCBfnc          = wdiDelSTARspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_DelSTAReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_BSS_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetBSSKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetBSSKeyParams); 
  wdiEventData.pCBfnc          = wdiSetBSSKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetBSSKeyReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_RMV_BSS_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveBSSKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveBSSKeyParams); 
  wdiEventData.pCBfnc          = wdiRemoveBSSKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_RemoveBSSKeyReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_STA_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetSTAKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetSTAKeyParams); 
  wdiEventData.pCBfnc          = wdiSetSTAKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetSTAKeyReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_RMV_STA_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveSTAKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveSTAKeyParams); 
  wdiEventData.pCBfnc          = wdiRemoveSTAKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_RemoveSTAKeyReq*/


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
)

{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_STA_BCAST_KEY_REQ;
  wdiEventData.pEventData      = pwdiSetSTABcastKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetSTABcastKeyParams); 
  wdiEventData.pCBfnc          = wdiSetSTABcastKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetSTABcastKeyReq*/

/**
 @brief WDI_RemoveSTABcastKeyReq will be called when the upper 
        MAC wants to uninstall a STA Bcast key from HW. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Remove STA Bcast Key request message to the lower RIVA
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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_RMV_STA_BCAST_KEY_REQ;
  wdiEventData.pEventData      = pwdiRemoveSTABcastKeyParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiRemoveSTABcastKeyParams); 
  wdiEventData.pCBfnc          = wdiRemoveSTABcastKeyRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_RemoveSTABcastKeyReq*/

#ifdef WLAN_FEATURE_VOWIFI
/**
 @brief WDI_SetMaxTxPowerReq will be called when the upper 
        MAC wants to set Max Tx Power to HW. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Remove STA Bcast Key request message to the lower RIVA
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
  
 @see WDI_SetMaxTxPowerReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetMaxTxPowerReq
(
  WDI_SetMaxTxPowerParamsType*   pwdiSetMaxTxPowerParams,
  WDA_SetMaxTxPowerRspCb         wdiReqStatusCb,
  void*                          pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_MAX_TX_POWER_REQ;
  wdiEventData.pEventData      = pwdiSetMaxTxPowerParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetMaxTxPowerParams); 
  wdiEventData.pCBfnc          = wdiReqStatusCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}
#endif

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ADD_TS_REQ;
  wdiEventData.pEventData      = pwdiAddTsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddTsReqParams); 
  wdiEventData.pCBfnc          = wdiAddTsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_AddTSReq*/



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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_DEL_TS_REQ;
  wdiEventData.pEventData      = pwdiDelTsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelTsReqParams); 
  wdiEventData.pCBfnc          = wdiDelTsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_DelTSReq*/



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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPD_EDCA_PRMS_REQ;
  wdiEventData.pEventData      = pwdiUpdateEDCAParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateEDCAParams); 
  wdiEventData.pCBfnc          = wdiUpdateEDCAParamsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_UpdateEDCAParams*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ADD_BA_SESSION_REQ;
  wdiEventData.pEventData      = pwdiAddBASessionReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddBASessionReqParams); 
  wdiEventData.pCBfnc          = wdiAddBASessionRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_AddBASessionReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_DEL_BA_REQ;
  wdiEventData.pEventData      = pwdiDelBAReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiDelBAReqParams); 
  wdiEventData.pCBfnc          = wdiDelBARspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_DelBAReq*/

/*======================================================================== 
 
                            Power Save APIs
 
==========================================================================*/

/**
 @brief WDI_SetPwrSaveCfgReq will be called when the upper MAC 
        wants to set the power save related configurations of
        the WLAN Device. Upon the call of this API the WLAN DAL
        will pack and send a HAL Update CFG request message to
        the lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_Start must have been called.

 @param pwdiPowerSaveCfg: the power save cfg parameters as 
                      specified by the Device Interface
  
        wdiSetPwrSaveCfgCb: callback for passing back the
        response of the set power save cfg operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call  
*/ 
WDI_Status 
WDI_SetPwrSaveCfgReq
(
  WDI_UpdateCfgReqParamsType*   pwdiPowerSaveCfg,
  WDI_SetPwrSaveCfgCb     wdiSetPwrSaveCfgCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPDATE_CFG_REQ;
  wdiEventData.pEventData      = pwdiPowerSaveCfg; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiPowerSaveCfg); 
  wdiEventData.pCBfnc          = wdiSetPwrSaveCfgCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetPwrSaveCfgReq*/

/**
 @brief WDI_EnterImpsReq will be called when the upper MAC to 
        request the device to get into IMPS power state. Upon
        the call of this API the WLAN DAL will send a HAL Enter
        IMPS request message to the lower RIVA sub-system if DAL
        is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param wdiEnterImpsRspCb: callback for passing back the 
        response of the Enter IMPS operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_EnterImpsReq
(
   WDI_EnterImpsRspCb  wdiEnterImpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ENTER_IMPS_REQ;
  wdiEventData.pEventData      = NULL; 
  wdiEventData.uEventDataSize  = 0; 
  wdiEventData.pCBfnc          = wdiEnterImpsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_EnterImpsReq*/

/**
 @brief WDI_ExitImpsReq will be called when the upper MAC to 
        request the device to get out of IMPS power state. Upon
        the call of this API the WLAN DAL will send a HAL Exit
        IMPS request message to the lower RIVA sub-system if DAL
        is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 

 @param wdiExitImpsRspCb: callback for passing back the response 
        of the Exit IMPS operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_Start
 @return Result of the function call
*/
WDI_Status 
WDI_ExitImpsReq
(
   WDI_ExitImpsRspCb  wdiExitImpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_EXIT_IMPS_REQ;
  wdiEventData.pEventData      = NULL; 
  wdiEventData.uEventDataSize  = 0; 
  wdiEventData.pCBfnc          = wdiExitImpsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_ExitImpsReq*/

/**
 @brief WDI_EnterBmpsReq will be called when the upper MAC to 
        request the device to get into BMPS power state. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Enter BMPS request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiEnterBmpsReqParams: the Enter BMPS parameters as 
                      specified by the Device Interface
  
        wdiEnterBmpsRspCb: callback for passing back the
        response of the Enter BMPS operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_EnterBmpsReq
(
   WDI_EnterBmpsReqParamsType *pwdiEnterBmpsReqParams,
   WDI_EnterBmpsRspCb  wdiEnterBmpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ENTER_BMPS_REQ;
  wdiEventData.pEventData      = pwdiEnterBmpsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiEnterBmpsReqParams); 
  wdiEventData.pCBfnc          = wdiEnterBmpsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_EnterBmpsReq*/

/**
 @brief WDI_ExitBmpsReq will be called when the upper MAC to 
        request the device to get out of BMPS power state. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Exit BMPS request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiExitBmpsReqParams: the Exit BMPS parameters as 
                      specified by the Device Interface
  
        wdiExitBmpsRspCb: callback for passing back the response
        of the Exit BMPS operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_ExitBmpsReq
(
   WDI_ExitBmpsReqParamsType *pwdiExitBmpsReqParams,
   WDI_ExitBmpsRspCb  wdiExitBmpsRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_EXIT_BMPS_REQ;
  wdiEventData.pEventData      = pwdiExitBmpsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiExitBmpsReqParams); 
  wdiEventData.pCBfnc          = wdiExitBmpsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_ExitBmpsReq*/

/**
 @brief WDI_EnterUapsdReq will be called when the upper MAC to 
        request the device to get into UAPSD power state. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL Enter UAPSD request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.
 WDI_SetUapsdAcParamsReq must have been called.
  
 @param pwdiEnterUapsdReqParams: the Enter UAPSD parameters as 
                      specified by the Device Interface
  
        wdiEnterUapsdRspCb: callback for passing back the
        response of the Enter UAPSD operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq, WDI_SetUapsdAcParamsReq
 @return Result of the function call
*/
WDI_Status 
WDI_EnterUapsdReq
(
   WDI_EnterUapsdReqParamsType *pwdiEnterUapsdReqParams,
   WDI_EnterUapsdRspCb  wdiEnterUapsdRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ENTER_UAPSD_REQ;
  wdiEventData.pEventData      = pwdiEnterUapsdReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiEnterUapsdReqParams); 
  wdiEventData.pCBfnc          = wdiEnterUapsdRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_EnterUapsdReq*/

/**
 @brief WDI_ExitUapsdReq will be called when the upper MAC to 
        request the device to get out of UAPSD power state. Upon
        the call of this API the WLAN DAL will send a HAL Exit
        UAPSD request message to the lower RIVA sub-system if
        DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiExitUapsdRspCb: callback for passing back the 
        response of the Exit UAPSD operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_ExitUapsdReq
(
   WDI_ExitUapsdRspCb  wdiExitUapsdRspCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_EXIT_UAPSD_REQ;
  wdiEventData.pEventData      = NULL; 
  wdiEventData.uEventDataSize  = 0; 
  wdiEventData.pCBfnc          = wdiExitUapsdRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_ExitUapsdReq*/

/**
 @brief WDI_UpdateUapsdParamsReq will be called when the upper 
        MAC wants to set the UAPSD related configurations
        of an associated STA (while acting as an AP) to the WLAN
        Device. Upon the call of this API the WLAN DAL will pack
        and send a HAL Update UAPSD params request message to
        the lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_ConfigBSSReq must have been called.

 @param pwdiUpdateUapsdReqParams: the UAPSD parameters 
                      as specified by the Device Interface
  
        wdiUpdateUapsdParamsCb: callback for passing back the
        response of the update UAPSD params operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_ConfigBSSReq
 @return Result of the function call
*/
WDI_Status 
WDI_UpdateUapsdParamsReq
(
   WDI_UpdateUapsdReqParamsType *pwdiUpdateUapsdReqParams,
   WDI_UpdateUapsdParamsCb  wdiUpdateUapsdParamsCb,
   void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPDATE_UAPSD_PARAM_REQ;
  wdiEventData.pEventData      = pwdiUpdateUapsdReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateUapsdReqParams);; 
  wdiEventData.pCBfnc          = wdiUpdateUapsdParamsCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_UpdateUapsdParamsReq*/

/**
 @brief WDI_SetUapsdAcParamsReq will be called when the upper 
        MAC wants to set the UAPSD related configurations before
        requesting for enter UAPSD power state to the WLAN
        Device. Upon the call of this API the WLAN DAL will pack
        and send a HAL Set UAPSD params request message to
        the lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiUapsdInfo: the UAPSD parameters as specified by
                      the Device Interface
  
        wdiSetUapsdAcParamsCb: callback for passing back the
        response of the set UAPSD params operation received from
        the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetUapsdAcParamsReq
(
  WDI_SetUapsdAcParamsReqParamsType*      pwdiUapsdInfo,
  WDI_SetUapsdAcParamsCb  wdiSetUapsdAcParamsCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_UAPSD_PARAM_REQ;
  wdiEventData.pEventData      = pwdiUapsdInfo; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUapsdInfo); 
  wdiEventData.pCBfnc          = wdiSetUapsdAcParamsCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetUapsdAcParamsReq*/

/**
 @brief WDI_ConfigureRxpFilterReq will be called when the upper 
        MAC wants to set/reset the RXP filters for recieved pkts
        (MC, BC etc.). Upon the call of this API the WLAN DAL will pack
        and send a HAL configure RXP filter request message to
        the lower RIVA sub-system.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param pwdiConfigureRxpFilterReqParams: the RXP 
                      filter as specified by the Device
                      Interface
  
        wdiConfigureRxpFilterCb: callback for passing back the
        response of the configure RXP filter operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @return Result of the function call
*/
WDI_Status 
WDI_ConfigureRxpFilterReq
(
   WDI_ConfigureRxpFilterReqParamsType *pwdiConfigureRxpFilterReqParams,
   WDI_ConfigureRxpFilterCb             wdiConfigureRxpFilterCb,
   void*                                pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_CONFIGURE_RXP_FILTER_REQ;
   wdiEventData.pEventData      = pwdiConfigureRxpFilterReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiConfigureRxpFilterReqParams); 
   wdiEventData.pCBfnc          = wdiConfigureRxpFilterCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_ConfigureRxpFilterReq*/

/**
 @brief WDI_SetBeaconFilterReq will be called when the upper MAC
        wants to set the beacon filters while in power save.
        Upon the call of this API the WLAN DAL will pack and
        send a Beacon filter request message to the
        lower RIVA sub-system.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param pwdiBeaconFilterReqParams: the beacon 
                      filter as specified by the Device
                      Interface
  
        wdiBeaconFilterCb: callback for passing back the
        response of the set beacon filter operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @return Result of the function call
*/
WDI_Status 
WDI_SetBeaconFilterReq
(
   WDI_BeaconFilterReqParamsType   *pwdiBeaconFilterReqParams,
   WDI_SetBeaconFilterCb            wdiBeaconFilterCb,
   void*                            pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_SET_BEACON_FILTER_REQ;
   wdiEventData.pEventData      = pwdiBeaconFilterReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiBeaconFilterReqParams);; 
   wdiEventData.pCBfnc          = wdiBeaconFilterCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_SetBeaconFilterReq*/

/**
 @brief WDI_RemBeaconFilterReq will be called when the upper MAC
        wants to remove the beacon filter for perticular IE
        while in power save. Upon the call of this API the WLAN
        DAL will pack and send a remove Beacon filter request
        message to the lower RIVA sub-system.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param pwdiBeaconFilterReqParams: the beacon 
                      filter as specified by the Device
                      Interface
  
        wdiBeaconFilterCb: callback for passing back the
        response of the remove beacon filter operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @return Result of the function call
*/
WDI_Status 
WDI_RemBeaconFilterReq
(
   WDI_RemBeaconFilterReqParamsType *pwdiBeaconFilterReqParams,
   WDI_RemBeaconFilterCb             wdiBeaconFilterCb,
   void*                             pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_REM_BEACON_FILTER_REQ;
   wdiEventData.pEventData      = pwdiBeaconFilterReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiBeaconFilterReqParams);; 
   wdiEventData.pCBfnc          = wdiBeaconFilterCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_RemBeaconFilterReq*/

/**
 @brief WDI_SetRSSIThresholdsReq will be called when the upper 
        MAC wants to set the RSSI thresholds related
        configurations while in power save. Upon the call of
        this API the WLAN DAL will pack and send a HAL Set RSSI
        thresholds request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiUapsdInfo: the UAPSD parameters as specified by
                      the Device Interface
  
        wdiSetUapsdAcParamsCb: callback for passing back the
        response of the set UAPSD params operation received from
        the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SetRSSIThresholdsReq
(
  WDI_SetRSSIThresholdsReqParamsType*      pwdiRSSIThresholdsParams,
  WDI_SetRSSIThresholdsCb                  wdiSetRSSIThresholdsCb,
  void*                                    pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_SET_RSSI_THRESHOLDS_REQ;
   wdiEventData.pEventData      = pwdiRSSIThresholdsParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiRSSIThresholdsParams);; 
   wdiEventData.pCBfnc          = wdiSetRSSIThresholdsCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/* WDI_SetRSSIThresholdsReq*/

/**
 @brief WDI_HostOffloadReq will be called when the upper MAC 
        wants to set the filter to minimize unnecessary host
        wakeup due to broadcast traffic while in power save.
        Upon the call of this API the WLAN DAL will pack and
        send a HAL host offload request message to the
        lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiHostOffloadParams: the host offload as specified 
                      by the Device Interface
  
        wdiHostOffloadCb: callback for passing back the response
        of the host offload operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_HostOffloadReq
(
  WDI_HostOffloadReqParamsType*      pwdiHostOffloadParams,
  WDI_HostOffloadCb                  wdiHostOffloadCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_HOST_OFFLOAD_REQ;
   wdiEventData.pEventData      = pwdiHostOffloadParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiHostOffloadParams);; 
   wdiEventData.pCBfnc          = wdiHostOffloadCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_HostOffloadReq*/

/**
 @brief WDI_WowlAddBcPtrnReq will be called when the upper MAC 
        wants to set the Wowl Bcast ptrn to minimize unnecessary
        host wakeup due to broadcast traffic while in power
        save. Upon the call of this API the WLAN DAL will pack
        and send a HAL Wowl Bcast ptrn request message to the
        lower RIVA sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiWowlAddBcPtrnParams: the Wowl bcast ptrn as 
                      specified by the Device Interface
  
        wdiWowlAddBcPtrnCb: callback for passing back the
        response of the add Wowl bcast ptrn operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_WowlAddBcPtrnReq
(
  WDI_WowlAddBcPtrnReqParamsType*    pwdiWowlAddBcPtrnParams,
  WDI_WowlAddBcPtrnCb                wdiWowlAddBcPtrnCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_WOWL_ADD_BC_PTRN_REQ;
   wdiEventData.pEventData      = pwdiWowlAddBcPtrnParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlAddBcPtrnParams);; 
   wdiEventData.pCBfnc          = wdiWowlAddBcPtrnCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_WowlAddBcPtrnReq*/

/**
 @brief WDI_WowlDelBcPtrnReq will be called when the upper MAC 
        wants to clear the Wowl Bcast ptrn. Upon the call of
        this API the WLAN DAL will pack and send a HAL delete
        Wowl Bcast ptrn request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_WowlAddBcPtrnReq must have been called.

 @param pwdiWowlDelBcPtrnParams: the Wowl bcast ptrn as 
                      specified by the Device Interface
  
        wdiWowlDelBcPtrnCb: callback for passing back the
        response of the del Wowl bcast ptrn operation received
        from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_WowlAddBcPtrnReq
 @return Result of the function call
*/
WDI_Status 
WDI_WowlDelBcPtrnReq
(
  WDI_WowlDelBcPtrnReqParamsType*    pwdiWowlDelBcPtrnParams,
  WDI_WowlDelBcPtrnCb                wdiWowlDelBcPtrnCb,
  void*                              pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_WOWL_DEL_BC_PTRN_REQ;
   wdiEventData.pEventData      = pwdiWowlDelBcPtrnParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlDelBcPtrnParams);; 
   wdiEventData.pCBfnc          = wdiWowlDelBcPtrnCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_WowlDelBcPtrnReq*/

/**
 @brief WDI_WowlEnterReq will be called when the upper MAC 
        wants to enter the Wowl state to minimize unnecessary
        host wakeup while in power save. Upon the call of this
        API the WLAN DAL will pack and send a HAL Wowl enter
        request message to the lower RIVA sub-system if DAL is
        in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param pwdiWowlEnterReqParams: the Wowl enter info as 
                      specified by the Device Interface
  
        wdiWowlEnterReqCb: callback for passing back the
        response of the enter Wowl operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_WowlEnterReq
(
  WDI_WowlEnterReqParamsType*    pwdiWowlEnterParams,
  WDI_WowlEnterReqCb             wdiWowlEnterCb,
  void*                          pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_WOWL_ENTER_REQ;
   wdiEventData.pEventData      = pwdiWowlEnterParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiWowlEnterParams);; 
   wdiEventData.pCBfnc          = wdiWowlEnterCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_WowlEnterReq*/

/**
 @brief WDI_WowlExitReq will be called when the upper MAC 
        wants to exit the Wowl state. Upon the call of this API
        the WLAN DAL will pack and send a HAL Wowl exit request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_WowlEnterReq must have been called.

 @param pwdiWowlExitReqParams: the Wowl exit info as 
                      specified by the Device Interface
  
        wdiWowlExitReqCb: callback for passing back the response
        of the exit Wowl operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_WowlEnterReq
 @return Result of the function call
*/
WDI_Status 
WDI_WowlExitReq
(
  WDI_WowlExitReqCb              wdiWowlExitCb,
  void*                          pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_WOWL_EXIT_REQ;
   wdiEventData.pEventData      = NULL; 
   wdiEventData.uEventDataSize  = 0; 
   wdiEventData.pCBfnc          = wdiWowlExitCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_WowlExitReq*/

/**
 @brief WDI_ConfigureAppsCpuWakeupStateReq will be called when 
        the upper MAC wants to dynamically adjusts the listen
        interval based on the WLAN/MSM activity. Upon the call
        of this API the WLAN DAL will pack and send a HAL
        configure Apps Cpu Wakeup State request message to the
        lower RIVA sub-system.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param pwdiConfigureAppsCpuWakeupStateReqParams: the 
                      Apps Cpu Wakeup State as specified by the
                      Device Interface
  
        wdiConfigureAppsCpuWakeupStateCb: callback for passing
        back the response of the configure Apps Cpu Wakeup State
        operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @return Result of the function call
*/
WDI_Status 
WDI_ConfigureAppsCpuWakeupStateReq
(
   WDI_ConfigureAppsCpuWakeupStateReqParamsType *pwdiConfigureAppsCpuWakeupStateReqParams,
   WDI_ConfigureAppsCpuWakeupStateCb             wdiConfigureAppsCpuWakeupStateCb,
   void*                                         pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ;
   wdiEventData.pEventData      = pwdiConfigureAppsCpuWakeupStateReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiConfigureAppsCpuWakeupStateReqParams); 
   wdiEventData.pCBfnc          = wdiConfigureAppsCpuWakeupStateCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}/*WDI_ConfigureAppsCpuWakeupStateReq*/
/**
 @brief WDI_FlushAcReq will be called when the upper MAC wants 
        to to perform a flush operation on a given AC. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Flush AC request message to the lower RIVA sub-system if
        DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_AddBAReq must have been called.

 @param pwdiFlushAcReqParams: the Flush AC parameters as 
                      specified by the Device Interface
  
        wdiFlushAcRspCb: callback for passing back the response
        of the Flush AC operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_AddBAReq
 @return Result of the function call
*/
WDI_Status 
WDI_FlushAcReq
(
  WDI_FlushAcReqParamsType* pwdiFlushAcReqParams,
  WDI_FlushAcRspCb          wdiFlushAcRspCb,
  void*                     pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_FLUSH_AC_REQ;
   wdiEventData.pEventData      = pwdiFlushAcReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiFlushAcReqParams); 
   wdiEventData.pCBfnc          = wdiFlushAcRspCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_FlushAcReq*/

/**
 @brief WDI_BtAmpEventReq will be called when the upper MAC 
        wants to notify the lower mac on a BT AMP event. This is
        to inform BTC-SLM that some BT AMP event occured. Upon
        the call of this API the WLAN DAL will pack and send a
        HAL BT AMP event request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

  
 @param wdiBtAmpEventReqParams: the BT AMP event parameters as 
                      specified by the Device Interface
  
        wdiBtAmpEventRspCb: callback for passing back the
        response of the BT AMP event operation received from the
        device
  
        pUserData: user data will be passed back with the
        callback 
 
 @return Result of the function call
*/
WDI_Status 
WDI_BtAmpEventReq
(
  WDI_BtAmpEventParamsType* pwdiBtAmpEventReqParams,
  WDI_BtAmpEventRspCb       wdiBtAmpEventRspCb,
  void*                     pUserData
)
{
   WDI_EventInfoType      wdiEventData;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*------------------------------------------------------------------------
     Sanity Check 
   ------------------------------------------------------------------------*/
   if ( eWLAN_PAL_FALSE == gWDIInitialized )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI API call before module is initialized - Fail request");

     return WDI_STATUS_E_NOT_ALLOWED; 
   }

   /*------------------------------------------------------------------------
     Fill in Event data and post to the Main FSM
   ------------------------------------------------------------------------*/
   wdiEventData.wdiRequest      = WDI_BTAMP_EVENT_REQ;
   wdiEventData.pEventData      = pwdiBtAmpEventReqParams; 
   wdiEventData.uEventDataSize  = sizeof(*pwdiBtAmpEventReqParams); 
   wdiEventData.pCBfnc          = wdiBtAmpEventRspCb; 
   wdiEventData.pUserData       = pUserData;

   return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_BtAmpEventReq*/

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_CH_SWITCH_REQ;
  wdiEventData.pEventData      = pwdiSwitchChReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSwitchChReqParams); 
  wdiEventData.pCBfnc          = wdiSwitchChRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SwitchChReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_CONFIG_STA_REQ;
  wdiEventData.pEventData      = pwdiConfigSTAReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiConfigSTAReqParams); 
  wdiEventData.pCBfnc          = wdiConfigSTARspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_ConfigSTAReq*/

/**
 @brief WDI_SetLinkStateReq will be called when the upper MAC 
        wants to change the state of an ongoing link. Upon the
        call of this API the WLAN DAL will pack and send a HAL
        Start message request message to the lower RIVA
        sub-system if DAL is in state STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_JoinStartReq must have been called.

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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SET_LINK_ST_REQ;
  wdiEventData.pEventData      = pwdiSetLinkStateReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSetLinkStateReqParams); 
  wdiEventData.pCBfnc          = wdiSetLinkStateRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetLinkStateReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_GET_STATS_REQ;
  wdiEventData.pEventData      = pwdiGetStatsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiGetStatsReqParams); 
  wdiEventData.pCBfnc          = wdiGetStatsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_GetStatsReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPDATE_CFG_REQ;
  wdiEventData.pEventData      = pwdiUpdateCfgReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateCfgReqParams); 
  wdiEventData.pCBfnc          = wdiUpdateCfgsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_UpdateCfgReq*/



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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_ADD_BA_REQ;
  wdiEventData.pEventData      = pwdiAddBAReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiAddBAReqParams); 
  wdiEventData.pCBfnc          = wdiAddBARspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_AddBAReq*/


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
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_TRIGGER_BA_REQ;
  wdiEventData.pEventData      = pwdiTriggerBAReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiTriggerBAReqParams); 
  wdiEventData.pCBfnc          = wdiTriggerBARspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_AddBAReq*/

/**
 @brief WDI_UpdateBeaconParamsReq will be called when the upper MAC 
        wishes to update any of the BEacon parameters used by HW.
        Upon the call of this API the WLAN DAL will pack and send a HAL Update Beacon Params request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiUpdateBeaconParams: the Beacon parameters as specified 
                      by the Device Interface
  
        wdiUpdateBeaconParamsRspCb: callback for passing back the
        response of the start operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_UpdateBeaconParamsReq
(
  WDI_UpdateBeaconParamsType*    pwdiUpdateBeaconParams,
  WDI_UpdateBeaconParamsRspCb    wdiUpdateBeaconParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPD_BCON_PRMS_REQ;
  wdiEventData.pEventData      = pwdiUpdateBeaconParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateBeaconParams); 
  wdiEventData.pCBfnc          = wdiUpdateBeaconParamsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_UpdateBeaconParamsReq*/

/**
 @brief WDI_SendBeaconParamsReq will be called when the upper MAC 
        wishes to update  the Beacon template used by HW.
        Upon the call of this API the WLAN DAL will pack and send a HAL Update Beacon template request
        message to the lower RIVA sub-system if DAL is in state
        STARTED.

        In state BUSY this request will be queued. Request won't
        be allowed in any other state. 

 WDI_PostAssocReq must have been called.

 @param wdiSendBeaconParams: the Beacon parameters as specified 
                      by the Device Interface
  
        wdiSendBeaconParamsRspCb: callback for passing back the
        response of the start operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_SendBeaconParamsReq
(
  WDI_SendBeaconParamsType*    pwdiSendBeaconParams,
  WDI_SendBeaconParamsRspCb    wdiSendBeaconParamsRspCb,
  void*                        pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_SND_BCON_REQ;
  wdiEventData.pEventData      = pwdiSendBeaconParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiSendBeaconParams); 
  wdiEventData.pCBfnc          = wdiSendBeaconParamsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SendBeaconParamsReq*/

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
  WDI_UpdateProbeRspTemplateRspCb          wdiUpdateProbeRspParamsRspCb,
  void*                                    pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_UPD_PROBE_RSP_TEMPLATE_REQ;
  wdiEventData.pEventData      = pwdiUpdateProbeRspParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiUpdateProbeRspParams); 
  wdiEventData.pCBfnc          = wdiUpdateProbeRspParamsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_UpdateProbeRspTemplateReq*/

/**
 @brief WDI_NvDownloadReq will be called by the UMAC to dowload the NV blob
        to the NV memory.


 @param wdiNvDownloadReqParams: the NV Download parameters as specified by
                      the Device Interface
  
        wdiNvDownloadRspCb: callback for passing back the response of
        the NV Download operation received from the device
  
        pUserData: user data will be passed back with the
        callback 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
WDI_Status 
WDI_NvDownloadReq
(
  WDI_NvDownloadReqParamsType* pwdiNvDownloadReqParams,
  WDI_NvDownloadRspCb        wdiNvDownloadRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  
  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_NV_DOWNLOAD_REQ;            
  wdiEventData.pEventData      = (void *)pwdiNvDownloadReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiNvDownloadReqParams); 
  wdiEventData.pCBfnc          = wdiNvDownloadRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_START_EVENT, &wdiEventData);

}/*WDI_NVDownloadReq*/

#ifdef WLAN_FEATURE_P2P
/**
 @brief WDI_SetP2PGONOAReq will be called when the 
        upper MAC wants to send Notice of Absence
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
WDI_SetP2PGONOAReq
(
  WDI_SetP2PGONOAReqParamsType*    pwdiP2PGONOAReqParams,
  WDI_SetP2PGONOAReqParamsRspCb    wdiP2PGONOAReqParamsRspCb,
  void*                            pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ;
  wdiEventData.pEventData      = pwdiP2PGONOAReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiP2PGONOAReqParams); 
  wdiEventData.pCBfnc          = wdiP2PGONOAReqParamsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_SetP2PGONOAReq*/
#endif

#ifdef WLAN_FEATURE_VOWIFI_11R 
/**
 @brief WDI_AggrAddTSReq will be called when the upper MAC to inform
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
WDI_AggrAddTSReq
(
  WDI_AggrAddTSReqParamsType* pwdiAggrAddTsReqParams,
  WDI_AggrAddTsRspCb          wdiAggrAddTsRspCb,
  void*                   pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest      = WDI_AGGR_ADD_TS_REQ;
  wdiEventData.pEventData      = pwdiAggrAddTsReqParams; 
  wdiEventData.uEventDataSize  = sizeof(*pwdiAggrAddTsReqParams); 
  wdiEventData.pCBfnc          = wdiAggrAddTsRspCb; 
  wdiEventData.pUserData       = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);

}/*WDI_AggrAddTSReq*/

#endif /* WLAN_FEATURE_VOWIFI_11R */

#ifdef ANI_MANF_DIAG
/**
 @brief WDI_FTMCommandReq
        Post FTM Command Event
 
 @param  ftmCommandReq:   FTM Command Body 
 @param  ftmCommandRspCb: FTM Response from HAL CB 
 @param  pUserData:       Client Data
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_FTMCommandReq
(
  WDI_FTMCommandReqType *ftmCommandReq,
  WDI_FTMCommandRspCb    ftmCommandRspCb,
  void                  *pUserData
)
{
  WDI_EventInfoType      wdiEventData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Fill in Event data and post to the Main FSM
  ------------------------------------------------------------------------*/
  wdiEventData.wdiRequest     = WDI_FTM_CMD_REQ;
  wdiEventData.pEventData     = (void *)ftmCommandReq;
  wdiEventData.uEventDataSize = ftmCommandReq->bodyLength + sizeof(wpt_uint32);
  wdiEventData.pCBfnc         = ftmCommandRspCb;
  wdiEventData.pUserData      = pUserData;

  return WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, &wdiEventData);
}
#endif /* ANI_MANF_DIAG */ 

/*============================================================================ 
 
            DAL Control Path Main FSM Function Implementation
 
 ============================================================================*/

/**
 @brief Main FSM Start function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         wdiEV:           event posted to the main DAL FSM
         pEventData:      pointer to the event information
         structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_PostMainEvent
(
  WDI_ControlBlockType*  pWDICtx, 
  WDI_MainEventType      wdiEV, 
  WDI_EventInfoType*     pEventData
  
)
{
  WDI_Status         wdiStatus; 
  WDI_MainFuncType   pfnWDIMainEvHdlr; 
  WDI_MainStateType  wdiOldState; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
     Sanity check
   -------------------------------------------------------------------------*/
  if (( pWDICtx->uGlobalState >= WDI_MAX_ST ) ||
      ( wdiEV >= WDI_MAX_EVENT ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid state or event in Post Main Ev function ST: %d EV: %d",
               pWDICtx->uGlobalState, wdiEV);
     return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*Access to the global state must be locked */
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*Fetch event handler for state*/
  pfnWDIMainEvHdlr = wdiMainFSM[pWDICtx->uGlobalState].pfnMainTbl[wdiEV]; 

  wdiOldState = pWDICtx->uGlobalState;
  /*Transition to BUSY State - the request is now being processed by the FSM,
   if the request fails we shall transition back to the old state, if not
   the request will manage its own state transition*/
  WDI_STATE_TRANSITION( pWDICtx, WDI_BUSY_ST);

  wpalMutexRelease(&pWDICtx->wptMutex);

  /* If the state function associated with the EV is NULL it means that this
     event is not allowed in this state*/
  if ( NULL != pfnWDIMainEvHdlr ) 
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "Posting event %d in state: %d to the Main FSM", 
              wdiEV, pWDICtx->uGlobalState);
    wdiStatus = pfnWDIMainEvHdlr( pWDICtx, pEventData); 
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Unexpected event %d in state: %d", 
              wdiEV, pWDICtx->uGlobalState, 0);
    wdiStatus = WDI_STATUS_E_NOT_ALLOWED; 
  }

  /* !UT TO DO L: possible race condition here in the gap between synchro -
  reanalize this */

  /* If a request handles itself well it will end up in a success or in a
     pending
     Success - means that the request was processed and the proper state
     transition already occured or will occur when the resp is received
     - NO other state transition or dequeueing is required
 
     Pending - means the request could not be processed at this moment in time
     because the FSM was already busy so no state transition or dequeueing
     is necessary anymore*/
  if (( WDI_STATUS_SUCCESS != wdiStatus )&&
      ( WDI_STATUS_PENDING != wdiStatus ))
  {
    /*Access to the global state must be locked */
    wpalMutexAcquire(&pWDICtx->wptMutex);
    
    /*The request has failed or could not be processed - transition back to
      the old state - check to see if anything was queued and try to execute
      The dequeue logic should post a message to a thread and return - no
      actual processing can occur */
    WDI_STATE_TRANSITION( pWDICtx, wdiOldState);
    WDI_DequeuePendingReq(pWDICtx);
        
    wpalMutexRelease(&pWDICtx->wptMutex);
  }
  return wdiStatus; 

}/*WDI_PostMainEvent*/


/*--------------------------------------------------------------------------
  INIT State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStart
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*--------------------------------------------------------------------
     Check if the Control Transport has been opened 
  ----------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == pWDICtx->bCTOpened )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Control Transport not yet Open - queueing the request");

     WDI_STATE_TRANSITION( pWDICtx, WDI_INIT_ST);
     WDI_QueuePendingReq( pWDICtx, pEventData); 

     wpalMutexRelease(&pWDICtx->wptMutex);
     return WDI_STATUS_PENDING;
  }
 
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Return Success*/
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*WDI_MainStart*/

/**
 @brief Main FSM Response function for state INIT

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainRspInit
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*------------------------------------------------------------------------
    Not expecting a response from the device before it is started 
  ------------------------------------------------------------------------*/
  WDI_ASSERT(0); 

  /*Return Success*/
  return WDI_STATUS_E_NOT_ALLOWED;
}/* WDI_MainRspInit */

/**
 @brief Main FSM Close function for all states except BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainClose
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Close %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*Return Success*/
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*WDI_MainClose*/
/*--------------------------------------------------------------------------
  STARTED State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for state STARTED

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStartStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartRspCb           wdiStartRspCb = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*--------------------------------------------------------------------
     Nothing to do transport was already started 
  ----------------------------------------------------------------------*/
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
    "Received start while transport was already started - nothing to do"); 

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*Transition back to started because the post function transitioned us to
    busy*/
  WDI_STATE_TRANSITION( pWDICtx, WDI_STARTED_ST);

  /*Check to see if any request is pending*/
  WDI_DequeuePendingReq(pWDICtx);
  
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Tell UMAC Success*/
  wdiStartRspCb = (WDI_StartRspCb)pEventData->pCBfnc; 
  
   /*Notify UMAC*/
  wdiStartRspCb( &pWDICtx->wdiCachedStartRspParams, pWDICtx->pRspCBUserData);

  /*Return Success*/
  return WDI_STATUS_SUCCESS;

}/*WDI_MainStartStarted*/

/**
 @brief Main FSM Stop function for state STARTED

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStopStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on Main Start %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*State at this point is BUSY - because we enter this state before posting
    an event to the FSM in order to prevent potential race conditions*/

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
            "Processing stop request in FSM");

  /*Return Success*/
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*WDI_MainStopStarted*/
/**
 @brief Main FSM Request function for state started

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainReqStarted
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Req Started %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*State at this point is BUSY - because we enter this state before posting
    an event to the FSM in order to prevent potential race conditions*/

  /*Return Success*/
  return WDI_ProcessRequest( pWDICtx, pEventData );

}/*WDI_MainReqStarted*/

/**
 @brief Main FSM Response function for all states except INIT

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status  wdiStatus; 
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Response %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*We expect that we will transition to started after this processing*/
  pWDICtx->ucExpectedStateTransition = WDI_STARTED_ST; 

  /*Process the response */
  wdiStatus = WDI_ProcessResponse( pWDICtx, pEventData );

  /*Lock the CB as we are about to do a state transition*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*Transition to the expected state after the response processing
  - this should always be started state with the following exceptions:
  1. processing of a failed start respone
  2. device failure detected while processing response
  3. stop response received*/
  WDI_STATE_TRANSITION( pWDICtx, pWDICtx->ucExpectedStateTransition);

  /*Dequeue request that may have been queued while we were waiting for the
    response */
  WDI_DequeuePendingReq(pWDICtx); 

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Return Success - always */
  return WDI_STATUS_SUCCESS; 

}/*WDI_MainRsp*/

/*--------------------------------------------------------------------------
  BUSY State Functions 
--------------------------------------------------------------------------*/
/**
 @brief Main FSM Start function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStartBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Start in BUSY %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*--------------------------------------------------------------------
     Check if the Control Transport has been opened 
  ----------------------------------------------------------------------*/
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue start request");

  /*Queue the start request*/
  WDI_QueuePendingReq( pWDICtx, pEventData); 

  /*Return Success*/
  return WDI_STATUS_PENDING;
}/*WDI_MainStartBusy*/

/**
 @brief Main FSM Stop function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainStopBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Stop in BUSY %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*--------------------------------------------------------------------
     Check if the Control Transport has been opened 
  ----------------------------------------------------------------------*/
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue stop request");

  WDI_QueuePendingReq( pWDICtx, pEventData); 
  return WDI_STATUS_PENDING;
  
}/*WDI_MainStopBusy*/

/**
 @brief Main FSM Request function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainReqBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Request in BUSY %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*--------------------------------------------------------------------
     Check if the Control Transport has been opened 
  ----------------------------------------------------------------------*/
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue request");

  WDI_QueuePendingReq( pWDICtx, pEventData); 
  return WDI_STATUS_PENDING;
  
}/*WDI_MainReqBusy*/
/**
 @brief Main FSM Close function for state BUSY

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_MainCloseBusy
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*--------------------------------------------------------------------
     Sanity Check 
  ----------------------------------------------------------------------*/
  if (( NULL ==  pWDICtx ) || ( NULL == pEventData ))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters on Main Close in BUSY %x %x", 
               pWDICtx, pEventData);
     return WDI_STATUS_E_FAILURE;
  }

  /*--------------------------------------------------------------------
     Check if the Control Transport has been opened 
  ----------------------------------------------------------------------*/
  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
           "WDI Busy state - queue close request");

  WDI_QueuePendingReq( pWDICtx, pEventData); 
  return WDI_STATUS_PENDING;
  
}/*WDI_MainCloseBusy*/


/*======================================================================= 
 
           WLAN DAL Control Path Main Processing Functions
 
*=======================================================================*/

/*========================================================================
          Main DAL Control Path Request Processing API 
========================================================================*/
/**
 @brief Process Start Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartReqParamsType* pwdiStartParams    = NULL;
  WDI_StartRspCb          wdiStartRspCb      = NULL;
  wpt_uint8*              pSendBuffer        = NULL; 
  wpt_uint16              usDataOffset       = 0;
  wpt_uint16              usSendSize         = 0;

  tHalMacStartReqMsg      halStartReq; 
  wpt_uint16              usLen              = 0; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStartParams = (WDI_StartReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStartRspCb   = (WDI_StartRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_FATAL,
              "Invalid parameters in start req %x %x %x",
                pEventData, pwdiStartParams, wdiStartRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  usLen = sizeof(halStartReq.startReqParams) + 
          pwdiStartParams->usConfigBufferLen;

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_START_REQ, 
                        usLen,
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + usLen )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_FATAL,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiStartParams, wdiStartRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Fill in the message
  -----------------------------------------------------------------------*/
  halStartReq.startReqParams.driverType = 
     WDI_2_HAL_DRV_TYPE(pwdiStartParams->wdiDriverType); 

  halStartReq.startReqParams.uConfigBufferLen = 
                  pwdiStartParams->usConfigBufferLen; 
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halStartReq.startReqParams, 
                  sizeof(halStartReq.startReqParams)); 

  usDataOffset  += sizeof(halStartReq.startReqParams); 
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  pwdiStartParams->pConfigBuffer, 
                  pwdiStartParams->usConfigBufferLen); 

  pWDICtx->wdiReqStatusCB     = pwdiStartParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStartParams->pUserData; 

  /*Save Low Level Ind CB and associated user data - it will be used further
    on when an indication is comming from the lower MAC*/
  pWDICtx->wdiLowLevelIndCB   = pwdiStartParams->wdiLowLevelIndCB;
  pWDICtx->pIndUserData       = pwdiStartParams->pIndUserData; 

  pWDICtx->bFrameTransEnabled = pwdiStartParams->bFrameTransEnabled; 
  /*-------------------------------------------------------------------------
    Send Start Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiStartRspCb, pEventData->pUserData, WDI_START_RESP);

  
}/*WDI_ProcessStartReq*/

/**
 @brief Process Stop Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStopReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StopReqParamsType* pwdiStopParams      = NULL;
  WDI_StopRspCb          wdiStopRspCb        = NULL;
  wpt_uint8*             pSendBuffer         = NULL; 
  wpt_uint16             usDataOffset        = 0;
  wpt_uint16             usSendSize          = 0;

  tHalMacStopReqMsg      halStopReq; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

 /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStopParams = (WDI_StopReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStopRspCb   = (WDI_StopRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in stop req %x %x %x",
                pEventData, pwdiStopParams, wdiStopRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_STOP_REQ, 
                        sizeof(halStopReq.stopReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStopReq.stopReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in stop req %x %x %x",
                pEventData, pwdiStopParams, wdiStopRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Fill in the message
  -----------------------------------------------------------------------*/
  halStopReq.stopReqParams.reason = WDI_2_HAL_STOP_REASON(
                                          pwdiStopParams->wdiStopReason);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halStopReq.stopReqParams, 
                  sizeof(halStopReq.stopReqParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiStopParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStopParams->pUserData; 

  /*! TO DO: stop the data services */

  /*Stop the STA Table !UT- check this logic again
   It is safer to do it here than on the response - because a stop is iminent*/
  WDI_STATableStop(pWDICtx);

  /* Stop Transport Driver, DXE */
  WDTS_Stop(pWDICtx);

  /*-------------------------------------------------------------------------
    Send Stop Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiStopRspCb, pEventData->pUserData, WDI_STOP_RESP);

}/*WDI_ProcessStopReq*/

/**
 @brief Process Close Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessCloseReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   wpt_status              wptStatus; 
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*Lock control block for cleanup*/
   wpalMutexAcquire(&pWDICtx->wptMutex);
       
   /*Clear all pending request*/
   WDI_ClearPendingRequests(pWDICtx);

   /* Close Control transport*/
   WCTS_CloseTransport(pWDICtx->wctsHandle); 

   /* Close Data transport*/
   WDTS_Close(pWDICtx);

   /*Close the STA Table !UT- check this logic again*/
   WDI_STATableClose(pWDICtx);

   /*close the PAL */
   wptStatus = wpalClose(pWDICtx->pPALContext);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to wpal Close %d", wptStatus);
     WDI_ASSERT(0);
   }

   /*Transition back to init state*/
   WDI_STATE_TRANSITION( pWDICtx, WDI_INIT_ST);

   wpalMutexRelease(&pWDICtx->wptMutex);

   /*invalidate the main synchro mutex */
   wptStatus = wpalMutexDelete(&pWDICtx->wptMutex);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to delete mutex %d", wptStatus);
     WDI_ASSERT(0);
   }

   /*Clear control block */
   WDI_CleanCB(pWDICtx);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessCloseReq*/


/*===========================================================================
                  SCANING REQUEST PROCESSING API 
===========================================================================*/

/**
 @brief Process Init Scan Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessInitScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_InitScanReqParamsType*  pwdiInitScanParams    = NULL;
  WDI_InitScanRspCb           wdiInitScanRspCb      = NULL;
  wpt_uint8*                  pSendBuffer           = NULL; 
  wpt_uint16                  usDataOffset          = 0;
  wpt_uint16                  usSendSize            = 0;

  tHalInitScanReqMsg          halInitScanReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiInitScanParams = (WDI_InitScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiInitScanRspCb   = (WDI_InitScanRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in init scan req %x %x %x",
                pEventData, pwdiInitScanParams, wdiInitScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Check to see if SCAN is already in progress - if so reject the req
    We only allow one scan at a time
    ! TO DO: - revisit this constraint 
  -----------------------------------------------------------------------*/
  if ( pWDICtx->bScanInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Scan is already in progress - subsequent scan is not allowed"
              " until the first scan completes");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  pWDICtx->bScanInProgress = eWLAN_PAL_TRUE; 
  pWDICtx->uScanState      = WDI_SCAN_INITIALIZED_ST; 

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_INIT_SCAN_REQ, 
                        sizeof(halInitScanReqMsg.initScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halInitScanReqMsg.initScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in init scan req %x %x %x",
                pEventData, pwdiInitScanParams, wdiInitScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Fill in the message
  -----------------------------------------------------------------------*/
  halInitScanReqMsg.initScanParams.scanMode = 
             WDI_2_HAL_SCAN_MODE(pwdiInitScanParams->wdiReqInfo.wdiScanMode);

  wpalMemoryCopy(halInitScanReqMsg.initScanParams.bssid,
                 pwdiInitScanParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

  halInitScanReqMsg.initScanParams.notifyBss = 
                                  pwdiInitScanParams->wdiReqInfo.bNotifyBSS;
  halInitScanReqMsg.initScanParams.frameType = 
                                  pwdiInitScanParams->wdiReqInfo.ucFrameType;
  halInitScanReqMsg.initScanParams.frameLength = 
                                  pwdiInitScanParams->wdiReqInfo.ucFrameLength;

  WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr( &halInitScanReqMsg.initScanParams.macMgmtHdr,
                              &pwdiInitScanParams->wdiReqInfo.wdiMACMgmtHdr);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halInitScanReqMsg.initScanParams, 
                  sizeof(halInitScanReqMsg.initScanParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiInitScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiInitScanParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Init Scan Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiInitScanRspCb, pEventData->pUserData, WDI_INIT_SCAN_RESP);

}/*WDI_ProcessInitScanReq*/

/**
 @brief Process Start Scan Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartScanReqParamsType*  pwdiStartScanParams    = NULL;
  WDI_StartScanRspCb           wdiStartScanRspCb      = NULL;
  wpt_uint8*                   pSendBuffer            = NULL; 
  wpt_uint16                   usDataOffset           = 0;
  wpt_uint16                   usSendSize             = 0;

  tHalStartScanReqMsg          halStartScanReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiStartScanParams = (WDI_StartScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiStartScanRspCb   = (WDI_StartScanRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in start scan req %x %x %x",
                pEventData, pwdiStartScanParams, wdiStartScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Check to see if SCAN is already in progress - start scan is only
    allowed when a scan is ongoing and the state of the scan procedure
    is either init or end 
  -----------------------------------------------------------------------*/
  if (( !pWDICtx->bScanInProgress ) || 
      (( WDI_SCAN_INITIALIZED_ST != pWDICtx->uScanState ) &&
       ( WDI_SCAN_ENDED_ST != pWDICtx->uScanState )))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Scan start not allowed in this state %d %d",
               pWDICtx->bScanInProgress, pWDICtx->uScanState);
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  pWDICtx->uScanState      = WDI_SCAN_STARTED_ST; 

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_START_SCAN_REQ, 
                        sizeof(halStartScanReqMsg.startScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStartScanReqMsg.startScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiStartScanParams, wdiStartScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halStartScanReqMsg.startScanParams.scanChannel = 
                              pwdiStartScanParams->ucChannel;
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halStartScanReqMsg.startScanParams, 
                  sizeof(halStartScanReqMsg.startScanParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiStartScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiStartScanParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Start Scan Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiStartScanRspCb, pEventData->pUserData, WDI_START_SCAN_RESP);
}/*WDI_ProcessStartScanReq*/


/**
 @brief Process End Scan Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEndScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_EndScanReqParamsType*  pwdiEndScanParams    = NULL;
  WDI_EndScanRspCb           wdiEndScanRspCb      = NULL;
  wpt_uint8*                 pSendBuffer          = NULL; 
  wpt_uint16                 usDataOffset         = 0;
  wpt_uint16                 usSendSize           = 0;

  tHalEndScanReqMsg          halEndScanReqMsg;           
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiEndScanParams = (WDI_EndScanReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiEndScanRspCb   = (WDI_EndScanRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in end scan req %x %x %x",
                pEventData, pwdiEndScanParams, wdiEndScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Check to see if SCAN is already in progress - end scan is only
    allowed when a scan is ongoing and the state of the scan procedure
    is started
  -----------------------------------------------------------------------*/
  if (( !pWDICtx->bScanInProgress ) || 
      ( WDI_SCAN_STARTED_ST != pWDICtx->uScanState ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "End start not allowed in this state %d %d",
               pWDICtx->bScanInProgress, pWDICtx->uScanState);
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  pWDICtx->uScanState      = WDI_SCAN_ENDED_ST; 

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_END_SCAN_REQ, 
                        sizeof(halEndScanReqMsg.endScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halEndScanReqMsg.endScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiEndScanParams, wdiEndScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halEndScanReqMsg.endScanParams.scanChannel = pwdiEndScanParams->ucChannel;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halEndScanReqMsg.endScanParams, 
                  sizeof(halEndScanReqMsg.endScanParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiEndScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiEndScanParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send End Scan Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiEndScanRspCb, pEventData->pUserData, WDI_END_SCAN_RESP);
}/*WDI_ProcessEndScanReq*/


/**
 @brief Process Finish Scan Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFinishScanReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_FinishScanReqParamsType*  pwdiFinishScanParams;
  WDI_FinishScanRspCb           wdiFinishScanRspCb;
  wpt_uint8*                    pSendBuffer          = NULL; 
  wpt_uint16                    usDataOffset         = 0;
  wpt_uint16                    usSendSize           = 0;

  tHalFinishScanReqMsg          halFinishScanReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in finish scan req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiFinishScanParams = (WDI_FinishScanReqParamsType*)pEventData->pEventData;
  wdiFinishScanRspCb   = (WDI_FinishScanRspCb)pEventData->pCBfnc;
  wpalMutexAcquire(&pWDICtx->wptMutex);
   /*-----------------------------------------------------------------------
    Check to see if SCAN is already in progress
    Finish scan gets invoked any scan states. ie. abort scan
    It should be allowed in any states.
  -----------------------------------------------------------------------*/
  if ( !pWDICtx->bScanInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Finish start not allowed in this state %d",
               pWDICtx->bScanInProgress );

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*-----------------------------------------------------------------------
    It is safe to reset the scan flags here because until the response comes
    back all subsequent requests will be blocked at BUSY state 
  -----------------------------------------------------------------------*/
  pWDICtx->uScanState      = WDI_SCAN_FINISHED_ST; 
  pWDICtx->bScanInProgress = eWLAN_PAL_FALSE; 
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_FINISH_SCAN_REQ, 
                        sizeof(halFinishScanReqMsg.finishScanParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halFinishScanReqMsg.finishScanParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start scan req %x %x %x",
                pEventData, pwdiFinishScanParams, wdiFinishScanRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halFinishScanReqMsg.finishScanParams.scanMode = 
    WDI_2_HAL_SCAN_MODE(pwdiFinishScanParams->wdiReqInfo.wdiScanMode);

  halFinishScanReqMsg.finishScanParams.currentOperChannel = 
    pwdiFinishScanParams->wdiReqInfo.ucCurrentOperatingChannel;

  halFinishScanReqMsg.finishScanParams.cbState = 
    WDI_2_HAL_CB_STATE(pwdiFinishScanParams->wdiReqInfo.wdiCBState);

  wpalMemoryCopy(halFinishScanReqMsg.finishScanParams.bssid,
                 pwdiFinishScanParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN);

  halFinishScanReqMsg.finishScanParams.notifyBss   = 
    pwdiFinishScanParams->wdiReqInfo.bNotifyBSS;
  halFinishScanReqMsg.finishScanParams.frameType   = 
    pwdiFinishScanParams->wdiReqInfo.ucFrameType;
  halFinishScanReqMsg.finishScanParams.frameLength = 
    pwdiFinishScanParams->wdiReqInfo.ucFrameLength;

  WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr( &halFinishScanReqMsg.finishScanParams.macMgmtHdr,
                              &pwdiFinishScanParams->wdiReqInfo.wdiMACMgmtHdr);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halFinishScanReqMsg.finishScanParams, 
                  sizeof(halFinishScanReqMsg.finishScanParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiFinishScanParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiFinishScanParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Finish Scan Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiFinishScanRspCb, pEventData->pUserData, WDI_FINISH_SCAN_RESP);
}/*WDI_ProcessFinishScanReq*/


/*==========================================================================
                    ASSOCIATION REQUEST API 
==========================================================================*/
/**
 @brief Process BSS Join for a given Session 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessBSSSessionJoinReq
( 
  WDI_ControlBlockType*   pWDICtx,
  WDI_JoinReqParamsType*  pwdiJoinParams,
  WDI_JoinRspCb           wdiJoinRspCb,
  void*                   pUserData
)
{
  WDI_BSSSessionType*     pBSSSes             = NULL;
  wpt_uint8*              pSendBuffer         = NULL; 
  wpt_uint16              usDataOffset        = 0;
  wpt_uint16              usSendSize          = 0;
  wpt_uint8               ucCurrentBSSSesIdx  = 0; 

  tHalJoinReqMsg          halJoinReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*------------------------------------------------------------------------
    Check to see if we have any session with this BSSID already stored, we
    should not
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                                   pwdiJoinParams->wdiReqInfo.macBSSID, 
                                  &pBSSSes);  

  if ( NULL != pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association for this BSSID is already in place");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*------------------------------------------------------------------------
    Fetch an empty session block 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindEmptySession( pWDICtx, &pBSSSes); 
  if ( NULL == pBSSSes )
  {

    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "DAL has no free sessions - cannot run another join");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_RES_FAILURE; 
  }

  /*Save BSS Session Info*/
  pBSSSes->bInUse = eWLAN_PAL_TRUE; 
  wpalMemoryCopy( pBSSSes->macBSSID, pwdiJoinParams->wdiReqInfo.macBSSID, 
                  WDI_MAC_ADDR_LEN);

  /*Transition to state Joining*/
  pBSSSes->wdiAssocState      = WDI_ASSOC_JOINING_ST; 
  pWDICtx->ucCurrentBSSSesIdx = ucCurrentBSSSesIdx;
  
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_JOIN_REQ, 
                        sizeof(halJoinReqMsg.joinReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halJoinReqMsg.joinReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in join req %x %x %x",
                pUserData, pwdiJoinParams, wdiJoinRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy(halJoinReqMsg.joinReqParams.bssId,
                 pwdiJoinParams->wdiReqInfo.macBSSID, WDI_MAC_ADDR_LEN); 

  halJoinReqMsg.joinReqParams.ucChannel = 
    pwdiJoinParams->wdiReqInfo.wdiChannelInfo.ucChannel;

#ifndef WLAN_FEATURE_VOWIFI
  halJoinReqMsg.joinReqParams.ucLocalPowerConstraint = 
    pwdiJoinParams->wdiReqInfo.wdiChannelInfo.ucLocalPowerConstraint;
#endif

  halJoinReqMsg.joinReqParams.secondaryChannelOffset =     
     WDI_2_HAL_SEC_CH_OFFSET(pwdiJoinParams->wdiReqInfo.wdiChannelInfo.
                             wdiSecondaryChannelOffset);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halJoinReqMsg.joinReqParams, 
                  sizeof(halJoinReqMsg.joinReqParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiJoinParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiJoinParams->pUserData;  

  /*-------------------------------------------------------------------------
    Send Join Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiJoinRspCb, pUserData, WDI_JOIN_RESP); 

}/*WDI_ProcessBSSSessionJoinReq*/

/**
 @brief Process Join Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessJoinReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus          = WDI_STATUS_SUCCESS;
  WDI_JoinReqParamsType*  pwdiJoinParams     = NULL;
  WDI_JoinRspCb           wdiJoinRspCb       = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiJoinParams = (WDI_JoinReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiJoinRspCb   = (WDI_JoinRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in join req %x %x %x",
                pEventData, pwdiJoinParams, wdiJoinRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }
  
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  if ( eWLAN_PAL_FALSE != pWDICtx->bAssociationInProgress )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "Association is currently in progress, queueing new join req");

    /*Association is in progress - queue current one*/
    wdiStatus = WDI_QueueNewAssocRequest(pWDICtx, pEventData, 
                             pwdiJoinParams->wdiReqInfo.macBSSID);

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus; 
  }

  /*Starting a new association */
  pWDICtx->bAssociationInProgress = eWLAN_PAL_TRUE;
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Process the Join Request*/
  return WDI_ProcessBSSSessionJoinReq( pWDICtx, pwdiJoinParams,
                                       wdiJoinRspCb,pEventData->pUserData);

}/*WDI_ProcessJoinReq*/


/**
 @brief Process Config BSS Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigBSSReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigBSSReqParamsType*  pwdiConfigBSSParams;
  WDI_ConfigBSSRspCb           wdiConfigBSSRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint16                   uMsgSize            = 0; 
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 

  tConfigBssReqMsg             halConfigBssReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in config BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiConfigBSSParams = (WDI_ConfigBSSReqParamsType*)pEventData->pEventData;
  wdiConfigBSSRspCb   = (WDI_ConfigBSSRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                                 pwdiConfigBSSParams->wdiReqInfo.macBSSID, 
                                 &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    /* If the BSS type is IBSS create the session here as there is no Join 
     * Request in case of IBSS*/
    if((pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_IBSS_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_INFRA_AP_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_BTAMP_AP_MODE) ||
       (pwdiConfigBSSParams->wdiReqInfo.wdiBSSType == WDI_BTAMP_STA_MODE))
    {
      /*------------------------------------------------------------------------
        Fetch an empty session block 
      ------------------------------------------------------------------------*/
      ucCurrentBSSSesIdx = WDI_FindEmptySession( pWDICtx, &pBSSSes); 
      if ( NULL == pBSSSes )
      {
    
        WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "DAL has no free sessions - cannot run another join");
    
        wpalMutexRelease(&pWDICtx->wptMutex);
        return WDI_STATUS_RES_FAILURE; 
      }
    
      /*Save BSS Session Info*/
      pBSSSes->bInUse = eWLAN_PAL_TRUE; 
      wpalMemoryCopy( pBSSSes->macBSSID, pwdiConfigBSSParams->wdiReqInfo.macBSSID, 
                      WDI_MAC_ADDR_LEN);
    
      /*Transition to state Joining*/
      pBSSSes->wdiAssocState      = WDI_ASSOC_JOINING_ST; 
      pWDICtx->ucCurrentBSSSesIdx = ucCurrentBSSSesIdx;
    }
    else
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Association sequence for this BSS does not yet exist");
      /* for IBSS testing */
      wpalMutexRelease(&pWDICtx->wptMutex);
      return WDI_STATUS_E_NOT_ALLOWED; 
    }
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus; 
  }

  /* Cache the request for response processing */
  wpalMemoryCopy(&pWDICtx->wdiCachedConfigBssReq, 
                 pwdiConfigBSSParams, 
                 sizeof(pWDICtx->wdiCachedConfigBssReq));

  wpalMutexRelease(&pWDICtx->wptMutex);

  uMsgSize = sizeof(halConfigBssReqMsg.configBssParams); 

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIG_BSS_REQ, 
                    uMsgSize, &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + uMsgSize )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in config bss req %x %x %x",
                pEventData, pwdiConfigBSSParams, wdiConfigBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*Copy the BSS request */
  WDI_CopyWDIConfigBSSToHALConfigBSS( &halConfigBssReqMsg.configBssParams,
                                      &pwdiConfigBSSParams->wdiReqInfo);

  /* Need to fill in the STA Index to invalid, since at this point we have not
     yet received it from HAL */
  halConfigBssReqMsg.configBssParams.staContext.staIdx = WDI_STA_INVALID_IDX;

  /* Need to fill in the BSS index */
  halConfigBssReqMsg.configBssParams.staContext.bssIdx = pBSSSes->usBSSIdx;
  
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halConfigBssReqMsg.configBssParams, 
                  sizeof(halConfigBssReqMsg.configBssParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiConfigBSSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiConfigBSSParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Config BSS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiConfigBSSRspCb, pEventData->pUserData, 
                       WDI_CONFIG_BSS_RESP);

}/*WDI_ProcessConfigBSSReq*/


/**
 @brief Process Del BSS Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBSSReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBSSReqParamsType*  pwdiDelBSSParams    = NULL;
  WDI_DelBSSRspCb           wdiDelBSSRspCb      = NULL;
  wpt_uint8                 ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*       pBSSSes             = NULL;
  wpt_uint8*                pSendBuffer         = NULL; 
  wpt_uint16                usDataOffset        = 0;
  wpt_uint16                usSendSize          = 0;
  WDI_Status                wdiStatus           = WDI_STATUS_SUCCESS; 

  tDeleteBssReqMsg          halBssReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiDelBSSParams = (WDI_DelBSSReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiDelBSSRspCb   = (WDI_DelBSSRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pwdiDelBSSParams, wdiDelBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx, 
                                             pwdiDelBSSParams->ucBssIdx, 
                                            &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus; 
  }

  /*-----------------------------------------------------------------------
    If we receive a Del BSS request for an association that is already in
    progress, it indicates that the assoc has failed => we no longer have
    an association in progress => we must check for pending associations
    that were queued and start as soon as the Del BSS response is received 
  -----------------------------------------------------------------------*/
  if ( ucCurrentBSSSesIdx == pWDICtx->ucCurrentBSSSesIdx )
  {
    /*We can switch to false here because even if a subsequent Join comes in
      it will only be processed when DAL transitions out of BUSY state which
      happens when the Del BSS request comes */
     pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_BSS_REQ, 
                        sizeof(halBssReqMsg.deleteBssParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halBssReqMsg.deleteBssParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiDelBSSParams, wdiDelBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*Fill in the message request structure*/

  /*BSS Index is saved on config BSS response and Post Assoc Response */
  halBssReqMsg.deleteBssParams.bssIdx = pBSSSes->usBSSIdx; 

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halBssReqMsg.deleteBssParams, 
                  sizeof(halBssReqMsg.deleteBssParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiDelBSSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelBSSParams->pUserData; 

 
  /*-------------------------------------------------------------------------
    Send Del BSS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiDelBSSRspCb, pEventData->pUserData, WDI_DEL_BSS_RESP);

  
}/*WDI_ProcessDelBSSReq*/

/**
 @brief Process Post Assoc Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessPostAssocReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_PostAssocReqParamsType* pwdiPostAssocParams   = NULL;
  WDI_PostAssocRspCb          wdiPostAssocRspCb     = NULL;
  wpt_uint8                   ucCurrentBSSSesIdx    = 0; 
  WDI_BSSSessionType*         pBSSSes               = NULL;
  wpt_uint8*                  pSendBuffer           = NULL; 
  wpt_uint16                  usDataOffset          = 0;
  wpt_uint16                  usSendSize            = 0;
  wpt_uint16                  uMsgSize              = 0;
  wpt_uint16                  uOffset               = 0;
  WDI_Status                  wdiStatus             = WDI_STATUS_SUCCESS; 

  tPostAssocReqMsg            halPostAssocReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiPostAssocParams = (WDI_PostAssocReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiPostAssocRspCb   = (WDI_PostAssocRspCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Post Assoc req %x %x %x",
                pEventData, pwdiPostAssocParams, wdiPostAssocRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                              pwdiPostAssocParams->wdiBSSParams.macBSSID, 
                              &pBSSSes); 

  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist - "
              "operation not allowed");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 

    wpalMutexRelease(&pWDICtx->wptMutex);

    return wdiStatus; 
  }

  /*-----------------------------------------------------------------------
    If Post Assoc was not yet received - the current association must
    be in progress
    -----------------------------------------------------------------------*/
  if (( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx ) || 
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS association no longer in "
              "progress - not allowed");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*-----------------------------------------------------------------------
    Post Assoc Request is only allowed in Joining state 
  -----------------------------------------------------------------------*/
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Post Assoc not allowed before JOIN - failing request");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  uMsgSize = sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams) +
             sizeof(halPostAssocReqMsg.postAssocReqParams.configBssParams) ;
  /*-----------------------------------------------------------------------
    Fill message for tx over the bus 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_POST_ASSOC_REQ, 
                        uMsgSize,&pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + uMsgSize )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiPostAssocParams, wdiPostAssocRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*Copy the STA parameters */
  WDI_CopyWDIStaCtxToHALStaCtx(&halPostAssocReqMsg.postAssocReqParams.configStaParams,
                               &pwdiPostAssocParams->wdiSTAParams );

  /* Need to fill in the self STA Index */
  if ( WDI_STATUS_SUCCESS != 
       WDI_STATableFindStaidByAddr(pWDICtx,
                                   pwdiPostAssocParams->wdiSTAParams.macSTA,
                                   (wpt_uint8*)&halPostAssocReqMsg.postAssocReqParams.configStaParams.staIdx ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  /* Need to fill in the BSS index */
  halPostAssocReqMsg.postAssocReqParams.configStaParams.bssIdx = 
     pBSSSes->usBSSIdx;

  /*Copy the BSS parameters */
  WDI_CopyWDIConfigBSSToHALConfigBSS( &halPostAssocReqMsg.postAssocReqParams.configBssParams,
                                      &pwdiPostAssocParams->wdiBSSParams);

  /* Need to fill in the STA index of the peer */
  if ( WDI_STATUS_SUCCESS != 
       WDI_STATableFindStaidByAddr(pWDICtx,
                                   pwdiPostAssocParams->wdiBSSParams.wdiSTAContext.macSTA,
                                   (wpt_uint8*)&halPostAssocReqMsg.postAssocReqParams.configBssParams.staContext.staIdx)) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  /* Need to fill in the BSS index */
  halPostAssocReqMsg.postAssocReqParams.configStaParams.bssIdx = 
     pBSSSes->usBSSIdx;

  
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halPostAssocReqMsg.postAssocReqParams.configStaParams, 
                  sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams)); 

  uOffset = sizeof(halPostAssocReqMsg.postAssocReqParams.configStaParams);

  wpalMemoryCopy( pSendBuffer+usDataOffset + uOffset, 
                  &halPostAssocReqMsg.postAssocReqParams.configBssParams, 
                  sizeof(halPostAssocReqMsg.postAssocReqParams.configBssParams)); 

 
  pWDICtx->wdiReqStatusCB     = pwdiPostAssocParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiPostAssocParams->pUserData; 

 
  wpalMemoryCopy( &pWDICtx->wdiCachedPostAssocReq, 
                  pwdiPostAssocParams,
                  sizeof(pWDICtx->wdiCachedPostAssocReq));  

  /*-------------------------------------------------------------------------
    Send Post Assoc Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiPostAssocRspCb, pEventData->pUserData, WDI_POST_ASSOC_RESP);

  
}/*WDI_ProcessPostAssocReq*/

/**
 @brief Process Del STA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelSTAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTAReqParamsType*  pwdiDelSTAParams;
  WDI_DelSTARspCb           wdiDelSTARspCb;
  wpt_uint8                 ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*       pBSSSes             = NULL;
  wpt_uint8*                pSendBuffer         = NULL; 
  wpt_uint16                usDataOffset        = 0;
  wpt_uint16                usSendSize          = 0;
  wpt_macAddr               macBSSID; 
  WDI_Status                wdiStatus           = WDI_STATUS_SUCCESS;

  tDeleteStaReqMsg          halDelStaReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiDelSTAParams = (WDI_DelSTAReqParamsType*)pEventData->pEventData;
  wdiDelSTARspCb   = (WDI_DelSTARspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                                         pwdiDelSTAParams->usSTAIdx, 
                                                         &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_STA_REQ, 
                        sizeof(halDelStaReqMsg.delStaParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halDelStaReqMsg.delStaParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in start req %x %x %x",
                pEventData, pwdiDelSTAParams, wdiDelSTARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halDelStaReqMsg.delStaParams.staIdx = pwdiDelSTAParams->usSTAIdx; 
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halDelStaReqMsg.delStaParams, 
                  sizeof(halDelStaReqMsg.delStaParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiDelSTAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelSTAParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Del STA Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiDelSTARspCb, pEventData->pUserData, WDI_DEL_STA_RESP);

}/*WDI_ProcessDelSTAReq*/


/*==========================================================================
                 SECURITY REQUEST PROCESSING API 
==========================================================================*/
/**
 @brief Process Set BSS Key Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBssKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetBSSKeyReqParamsType*  pwdiSetBSSKeyParams;
  WDI_SetBSSKeyRspCb           wdiSetBSSKeyRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  tSetBssKeyReqMsg             halSetBssKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiSetBSSKeyParams = (WDI_SetBSSKeyReqParamsType*)pEventData->pEventData;
  wdiSetBSSKeyRspCb   = (WDI_SetBSSKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx, 
                           pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucBssIdx, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_BSS_KEY_REQ, 
                        sizeof(halSetBssKeyReqMsg.setBssKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetBssKeyReqMsg.setBssKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetBSSKeyParams, wdiSetBSSKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Copy the Key parameters into the HAL message
  -----------------------------------------------------------------------*/

  halSetBssKeyReqMsg.setBssKeyParams.bssIdx = ucCurrentBSSSesIdx; 

  halSetBssKeyReqMsg.setBssKeyParams.encType = 
             WDI_2_HAL_ENC_TYPE (pwdiSetBSSKeyParams->wdiBSSKeyInfo.wdiEncType);

  halSetBssKeyReqMsg.setBssKeyParams.numKeys = 
                                  pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucNumKeys;

  for(keyIndex = 0; keyIndex < pwdiSetBSSKeyParams->wdiBSSKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyId = 
                      pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyId;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].unicast =
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].unicast;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyDirection =
                pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyDirection;
    wpalMemoryCopy(halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyRsc,
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyRsc, 
                     WDI_MAX_KEY_RSC_LEN);
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].paeRole = 
                     pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].paeRole;
    halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].keyLength = 
                   pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].keyLength;
    wpalMemoryCopy(halSetBssKeyReqMsg.setBssKeyParams.key[keyIndex].key,
                         pwdiSetBSSKeyParams->wdiBSSKeyInfo.aKeys[keyIndex].key, 
                        WDI_MAX_KEY_LENGTH);
   }
                                                                  
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halSetBssKeyReqMsg.setBssKeyParams, 
                    sizeof(halSetBssKeyReqMsg.setBssKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetBSSKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetBSSKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Set BSS Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetBSSKeyRspCb, pEventData->pUserData, 
                       WDI_SET_BSS_KEY_RESP); 

}/*WDI_ProcessSetBssKeyReq*/

/**
 @brief Process Remove BSS Key Request function (called when Main    
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveBssKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveBSSKeyReqParamsType*  pwdiRemoveBSSKeyParams;
  WDI_RemoveBSSKeyRspCb           wdiRemoveBSSKeyRspCb;
  wpt_uint8                       ucCurrentBSSSesIdx     = 0; 
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL; 
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS; 
  tRemoveBssKeyReqMsg             halRemoveBssKeyReqMsg  = {{0}};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiRemoveBSSKeyParams = (WDI_RemoveBSSKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveBSSKeyRspCb   = (WDI_RemoveBSSKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx, 
                           pwdiRemoveBSSKeyParams->wdiKeyInfo.ucBssIdx, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_BSS_KEY_REQ, 
                        sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveBSSKeyParams, wdiRemoveBSSKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }
  /*-----------------------------------------------------------------------
    Copy the Key parameters into the HAL message
  -----------------------------------------------------------------------*/
  halRemoveBssKeyReqMsg.removeBssKeyParams.bssIdx = ucCurrentBSSSesIdx;

  halRemoveBssKeyReqMsg.removeBssKeyParams.encType = 
      WDI_2_HAL_ENC_TYPE (pwdiRemoveBSSKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveBssKeyReqMsg.removeBssKeyParams.keyId = pwdiRemoveBSSKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveBssKeyReqMsg.removeBssKeyParams.wepType = 
      WDI_2_HAL_WEP_TYPE(pwdiRemoveBSSKeyParams->wdiKeyInfo.wdiWEPType);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halRemoveBssKeyReqMsg.removeBssKeyParams, 
                    sizeof(halRemoveBssKeyReqMsg.removeBssKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiRemoveBSSKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveBSSKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Remove BSS Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiRemoveBSSKeyRspCb, pEventData->pUserData,
                       WDI_RMV_BSS_KEY_RESP); 
}/*WDI_ProcessRemoveBssKeyReq*/

/**
 @brief Process Set STA KeyRequest function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetSTAKeyReqParamsType*  pwdiSetSTAKeyParams;
  WDI_SetSTAKeyRspCb           wdiSetSTAKeyRspCb;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr                  macBSSID;
  wpt_uint8                    ucCurrentBSSSesIdx; 
  tSetStaKeyReqMsg             halSetStaKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del STA req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

   pwdiSetSTAKeyParams = (WDI_SetSTAKeyReqParamsType*)pEventData->pEventData;
   wdiSetSTAKeyRspCb   = (WDI_SetSTAKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                  pwdiSetSTAKeyParams->wdiKeyInfo.usSTAIdx, 
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_STA_KEY_REQ, 
                        sizeof(halSetStaKeyReqMsg.setStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetStaKeyReqMsg.setStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetSTAKeyParams, wdiSetSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }
  /*-----------------------------------------------------------------------
    Copy the STA Key parameters into the HAL message
  -----------------------------------------------------------------------*/
  halSetStaKeyReqMsg.setStaKeyParams.encType = 
      WDI_2_HAL_ENC_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiEncType);

  halSetStaKeyReqMsg.setStaKeyParams.wepType = 
      WDI_2_HAL_WEP_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiWEPType );

  halSetStaKeyReqMsg.setStaKeyParams.staIdx = pwdiSetSTAKeyParams->wdiKeyInfo.usSTAIdx;

  halSetStaKeyReqMsg.setStaKeyParams.defWEPIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucDefWEPIdx;

  halSetStaKeyReqMsg.setStaKeyParams.singleTidRc = pwdiSetSTAKeyParams->wdiKeyInfo.ucSingleTidRc;

#ifdef WLAN_SOFTAP_FEATURE
  for(keyIndex = 0; keyIndex < pwdiSetSTAKeyParams->wdiKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyId = 
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyId;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].unicast;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyDirection;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyRsc, 
                     WDI_MAX_KEY_RSC_LEN);
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].paeRole = 
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].paeRole;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyLength = 
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyLength;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].key, 
                        WDI_MAX_KEY_LENGTH);
   }
#else
  halSetStaKeyReqMsg.setStaKeyParams.key.keyId = 
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyId;
  halSetStaKeyReqMsg.setStaKeyParams.key.unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].unicast;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyDirection;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyRsc, 
                     WDI_MAX_KEY_RSC_LEN);
  halSetStaKeyReqMsg.setStaKeyParams.key.paeRole = 
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].paeRole;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyLength = 
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyLength;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].key, 
                        WDI_MAX_KEY_LENGTH);
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halSetStaKeyReqMsg.setStaKeyParams, 
                    sizeof(halSetStaKeyReqMsg.setStaKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetSTAKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Set STA Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetSTAKeyRspCb, pEventData->pUserData, 
                       WDI_SET_STA_KEY_RESP); 

}/*WDI_ProcessSetSTAKeyReq*/

/**
 @brief Process Remove STA Key Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveSTAKeyReqParamsType*  pwdiRemoveSTAKeyParams;
  WDI_RemoveSTAKeyRspCb           wdiRemoveSTAKeyRspCb;
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL; 
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS; 
  wpt_macAddr                     macBSSID;
  wpt_uint8                       ucCurrentBSSSesIdx;
  tRemoveStaKeyReqMsg             halRemoveStaKeyReqMsg  = {{0}};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
 if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del STA req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiRemoveSTAKeyParams = (WDI_RemoveSTAKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveSTAKeyRspCb   = (WDI_RemoveSTAKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                             pwdiRemoveSTAKeyParams->wdiKeyInfo.usSTAIdx, 
                             &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }



  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_STA_KEY_REQ, 
                        sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveSTAKeyParams, wdiRemoveSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Copy the Key parameters into the HAL message
  -----------------------------------------------------------------------*/

  halRemoveStaKeyReqMsg.removeStaKeyParams.staIdx = 
      pwdiRemoveSTAKeyParams->wdiKeyInfo.usSTAIdx;

  halRemoveStaKeyReqMsg.removeStaKeyParams.encType = 
      WDI_2_HAL_ENC_TYPE (pwdiRemoveSTAKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveStaKeyReqMsg.removeStaKeyParams.keyId = 
      pwdiRemoveSTAKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveStaKeyReqMsg.removeStaKeyParams.unicast = 
      pwdiRemoveSTAKeyParams->wdiKeyInfo.ucUnicast;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halRemoveStaKeyReqMsg.removeStaKeyParams, 
                    sizeof(halRemoveStaKeyReqMsg.removeStaKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiRemoveSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveSTAKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Remove STA Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiRemoveSTAKeyRspCb, pEventData->pUserData,
                       WDI_RMV_STA_KEY_RESP); 

}/*WDI_ProcessRemoveSTAKeyReq*/

/**
 @brief Process Set STA KeyRequest function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaBcastKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetSTAKeyReqParamsType*  pwdiSetSTAKeyParams;
  WDI_SetSTAKeyRspCb           wdiSetSTAKeyRspCb;
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr                  macBSSID;
  wpt_uint8                    ucCurrentBSSSesIdx; 
  tSetStaKeyReqMsg             halSetStaKeyReqMsg  = {{0}};
  wpt_uint8                    keyIndex            = 0;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del STA req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

   pwdiSetSTAKeyParams = (WDI_SetSTAKeyReqParamsType*)pEventData->pEventData;
   wdiSetSTAKeyRspCb   = (WDI_SetSTAKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                  pwdiSetSTAKeyParams->wdiKeyInfo.usSTAIdx, 
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_STA_KEY_REQ, 
                        sizeof(halSetStaKeyReqMsg.setStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetStaKeyReqMsg.setStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetSTAKeyParams, wdiSetSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }
  /*-----------------------------------------------------------------------
    Copy the STA Key parameters into the HAL message
  -----------------------------------------------------------------------*/
  halSetStaKeyReqMsg.setStaKeyParams.encType = 
      WDI_2_HAL_ENC_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiEncType);

  halSetStaKeyReqMsg.setStaKeyParams.wepType = 
      WDI_2_HAL_WEP_TYPE (pwdiSetSTAKeyParams->wdiKeyInfo.wdiWEPType );

  halSetStaKeyReqMsg.setStaKeyParams.staIdx = pwdiSetSTAKeyParams->wdiKeyInfo.usSTAIdx;

  halSetStaKeyReqMsg.setStaKeyParams.defWEPIdx = pwdiSetSTAKeyParams->wdiKeyInfo.ucDefWEPIdx;

  halSetStaKeyReqMsg.setStaKeyParams.singleTidRc = pwdiSetSTAKeyParams->wdiKeyInfo.ucSingleTidRc;

#ifdef WLAN_SOFTAP_FEATURE
  for(keyIndex = 0; keyIndex < pwdiSetSTAKeyParams->wdiKeyInfo.ucNumKeys ;
                                                                 keyIndex++)
  {
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyId = 
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyId;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].unicast;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyDirection;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyRsc, 
                     WDI_MAX_KEY_RSC_LEN);
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].paeRole = 
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].paeRole;
    halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].keyLength = 
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].keyLength;
    wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key[keyIndex].key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[keyIndex].key, 
                        WDI_MAX_KEY_LENGTH);
   }
#else
  halSetStaKeyReqMsg.setStaKeyParams.key.keyId = 
                      pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyId;
  halSetStaKeyReqMsg.setStaKeyParams.key.unicast =
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].unicast;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyDirection =
                pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyDirection;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.keyRsc,
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyRsc, 
                     WDI_MAX_KEY_RSC_LEN);
  halSetStaKeyReqMsg.setStaKeyParams.key.paeRole = 
                     pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].paeRole;
  halSetStaKeyReqMsg.setStaKeyParams.key.keyLength = 
                   pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].keyLength;
  wpalMemoryCopy(halSetStaKeyReqMsg.setStaKeyParams.key.key,
                         pwdiSetSTAKeyParams->wdiKeyInfo.wdiKey[0].key, 
                        WDI_MAX_KEY_LENGTH);
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halSetStaKeyReqMsg.setStaKeyParams, 
                    sizeof(halSetStaKeyReqMsg.setStaKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetSTAKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetSTAKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Set STA Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetSTAKeyRspCb, pEventData->pUserData, 
                       WDI_SET_STA_KEY_RESP); 

}/*WDI_ProcessSetSTABcastKeyReq*/

/**
 @brief Process Remove STA Key Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaBcastKeyReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_RemoveSTAKeyReqParamsType*  pwdiRemoveSTABcastKeyParams;
  WDI_RemoveSTAKeyRspCb           wdiRemoveSTAKeyRspCb;
  WDI_BSSSessionType*             pBSSSes                = NULL;
  wpt_uint8*                      pSendBuffer            = NULL; 
  wpt_uint16                      usDataOffset           = 0;
  wpt_uint16                      usSendSize             = 0;
  WDI_Status                      wdiStatus              = WDI_STATUS_SUCCESS; 
  wpt_macAddr                     macBSSID;
  wpt_uint8                       ucCurrentBSSSesIdx;
  tRemoveStaKeyReqMsg             halRemoveStaBcastKeyReqMsg = {{0}};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
 if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del STA req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiRemoveSTABcastKeyParams = (WDI_RemoveSTAKeyReqParamsType*)pEventData->pEventData;
  wdiRemoveSTAKeyRspCb   = (WDI_RemoveSTAKeyRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                             pwdiRemoveSTABcastKeyParams->wdiKeyInfo.usSTAIdx, 
                             &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }



  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_RMV_STA_BCAST_KEY_REQ, 
                        sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiRemoveSTABcastKeyParams, wdiRemoveSTAKeyRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Copy the Key parameters into the HAL message
  -----------------------------------------------------------------------*/

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.staIdx = 
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.usSTAIdx;

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.encType = 
      WDI_2_HAL_ENC_TYPE (pwdiRemoveSTABcastKeyParams->wdiKeyInfo.wdiEncType);

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.keyId = 
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucKeyId;

  halRemoveStaBcastKeyReqMsg.removeStaKeyParams.unicast = 
      pwdiRemoveSTABcastKeyParams->wdiKeyInfo.ucUnicast;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                    &halRemoveStaBcastKeyReqMsg.removeStaKeyParams, 
                    sizeof(halRemoveStaBcastKeyReqMsg.removeStaKeyParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiRemoveSTABcastKeyParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRemoveSTABcastKeyParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Remove STA Key Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiRemoveSTAKeyRspCb, pEventData->pUserData,
                       WDI_RMV_STA_KEY_RESP); 

}/*WDI_ProcessRemoveSTABcastKeyReq*/

/*==========================================================================
                   QOS and BA PROCESSING REQUEST API 
==========================================================================*/
/**
 @brief Process Add TSpec Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddTSpecReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddTSReqParamsType*  pwdiAddTSParams;
  WDI_AddTsRspCb           wdiAddTSRspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL; 
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr              macBSSID;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiAddTSParams = (WDI_AddTSReqParamsType*)pEventData->pEventData;
  wdiAddTSRspCb   = (WDI_AddTsRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                        pwdiAddTSParams->wdiTsInfo.usSTAIdx, 
                                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ADD_TS_REQ, 
                        sizeof(pwdiAddTSParams->wdiTsInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiAddTSParams->wdiTsInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiAddTSParams, wdiAddTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiAddTSParams->wdiTsInfo, 
                  sizeof(pwdiAddTSParams->wdiTsInfo)); 

  pWDICtx->wdiReqStatusCB     = pwdiAddTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddTSParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Add TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiAddTSRspCb, pEventData->pUserData,
                       WDI_ADD_TS_RESP); 
}/*WDI_ProcessAddTSpecReq*/


/**
 @brief Process Del TSpec Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelTSpecReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelTSReqParamsType*      pwdiDelTSParams;
  WDI_DelTsRspCb               wdiDelTSRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiDelTSParams = (WDI_DelTSReqParamsType*)pEventData->pEventData;
  wdiDelTSRspCb   = (WDI_DelTsRspCb)pEventData->pCBfnc;

  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                           pwdiDelTSParams->wdiDelTSInfo.macBSSID, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_TS_REQ, 
                        sizeof(pwdiDelTSParams->wdiDelTSInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiDelTSParams->wdiDelTSInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiDelTSParams, wdiDelTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiDelTSParams->wdiDelTSInfo, 
                  sizeof(pwdiDelTSParams->wdiDelTSInfo)); 

  pWDICtx->wdiReqStatusCB     = pwdiDelTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelTSParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Del TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiDelTSRspCb, pEventData->pUserData, WDI_DEL_TS_RESP); 
}/*WDI_ProcessDelTSpecReq*/

/**
 @brief Process Update EDCA Params Request function (called when
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateEDCAParamsType*    pwdiUpdateEDCAParams;
  WDI_UpdateEDCAParamsRspCb    wdiUpdateEDCARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset         = 0;
  wpt_uint16                   usSendSize           = 0;
  WDI_Status                   wdiStatus; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiUpdateEDCAParams = (WDI_UpdateEDCAParamsType*)pEventData->pEventData;
  wdiUpdateEDCARspCb   = (WDI_UpdateEDCAParamsRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx, 
                           pwdiUpdateEDCAParams->wdiEDCAInfo.ucBssIdx, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_EDCA_PRMS_REQ, 
                        sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateEDCAParams, wdiUpdateEDCARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiUpdateEDCAParams->wdiEDCAInfo, 
                  sizeof(pwdiUpdateEDCAParams->wdiEDCAInfo)); 

  pWDICtx->wdiReqStatusCB     = pwdiUpdateEDCAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateEDCAParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Update EDCA Params Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiUpdateEDCARspCb, pEventData->pUserData, 
                       WDI_UPD_EDCA_PRMS_RESP); 
}/*WDI_ProcessUpdateEDCAParamsReq*/

/**
 @brief Process Add BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBASessionReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBASessionReqParamsType*  pwdiAddBASessionParams;
  WDI_AddBASessionRspCb           wdiAddBASessionRspCb;
  wpt_uint8                       ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*             pBSSSes             = NULL;
  wpt_uint8*                      pSendBuffer         = NULL; 
  wpt_uint16                      usDataOffset        = 0;
  wpt_uint16                      usSendSize          = 0;
  WDI_Status                      wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr                     macBSSID;

  tAddBASessionReqMsg             halAddBASessionReq;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiAddBASessionParams = 
                  (WDI_AddBASessionReqParamsType*)pEventData->pEventData;
  wdiAddBASessionRspCb = 
                  (WDI_AddBASessionRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                   pwdiAddBASessionParams->wdiBASessionInfoType.usSTAIdx, 
                   &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }


  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, 
                        WDI_ADD_BA_SESSION_REQ, 
                        sizeof(halAddBASessionReq.addBASessionParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < 
            (usDataOffset + sizeof(halAddBASessionReq.addBASessionParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Add BA session req %x %x %x",
                pEventData, pwdiAddBASessionParams, wdiAddBASessionRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halAddBASessionReq.addBASessionParams.staIdx =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usSTAIdx;
  wpalMemoryCopy(halAddBASessionReq.addBASessionParams.peerMacAddr,
                  pwdiAddBASessionParams->wdiBASessionInfoType.macPeerAddr,
                  WDI_MAC_ADDR_LEN);
  halAddBASessionReq.addBASessionParams.baTID =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaTID;
  halAddBASessionReq.addBASessionParams.baPolicy =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaPolicy;
  halAddBASessionReq.addBASessionParams.baBufferSize =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaBufferSize;
  halAddBASessionReq.addBASessionParams.baTimeout =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaTimeout;
  halAddBASessionReq.addBASessionParams.baSSN =
                  pwdiAddBASessionParams->wdiBASessionInfoType.usBaSSN;
  halAddBASessionReq.addBASessionParams.baDirection =
                  pwdiAddBASessionParams->wdiBASessionInfoType.ucBaDirection;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halAddBASessionReq.addBASessionParams, 
                  sizeof(halAddBASessionReq.addBASessionParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiAddBASessionParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddBASessionParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Start Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiAddBASessionRspCb, pEventData->pUserData, 
                        WDI_ADD_BA_SESSION_RESP); 
}/*WDI_ProcessAddBASessionReq*/

/**
 @brief Process Del BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBAReqParamsType*      pwdiDelBAParams;
  WDI_DelBARspCb               wdiDelBARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr                  macBSSID;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiDelBAParams = (WDI_DelBAReqParamsType*)pEventData->pEventData;
  wdiDelBARspCb   = (WDI_DelBARspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                     pwdiDelBAParams->wdiBAInfo.usSTAIdx, 
                                     &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_DEL_BA_REQ, 
                        sizeof(pwdiDelBAParams->wdiBAInfo),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiDelBAParams->wdiBAInfo) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiDelBAParams, wdiDelBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiDelBAParams->wdiBAInfo, 
                  sizeof(pwdiDelBAParams->wdiBAInfo)); 

  pWDICtx->wdiReqStatusCB     = pwdiDelBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiDelBAParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Start Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiDelBARspCb, pEventData->pUserData, WDI_DEL_BA_RESP); 
}/*WDI_ProcessDelBAReq*/

/**
 @brief Process Flush AC Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFlushAcReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_FlushAcReqParamsType*    pwdiFlushAcParams = NULL;
   WDI_FlushAcRspCb             wdiFlushAcRspCb;
   wpt_uint8*                   pSendBuffer         = NULL; 
   wpt_uint16                   usDataOffset        = 0;
   wpt_uint16                   usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
       ( NULL == pEventData->pCBfnc ))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Flush AC req %x %x %x",
                 pEventData, pEventData->pEventData, pEventData->pCBfnc);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   pwdiFlushAcParams = (WDI_FlushAcReqParamsType*)pEventData->pEventData;
   wdiFlushAcRspCb   = (WDI_FlushAcRspCb)pEventData->pCBfnc;
   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_FLUSH_AC_REQ, 
                         sizeof(pwdiFlushAcParams->wdiFlushAcInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiFlushAcParams->wdiFlushAcInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in set bss key req %x %x %x",
                 pEventData, pwdiFlushAcParams, wdiFlushAcRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiFlushAcParams->wdiFlushAcInfo, 
                   sizeof(pwdiFlushAcParams->wdiFlushAcInfo)); 

   pWDICtx->wdiReqStatusCB     = pwdiFlushAcParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiFlushAcParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Start Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiFlushAcRspCb, pEventData->pUserData, WDI_FLUSH_AC_RESP); 
}/*WDI_ProcessFlushAcReq*/

/**
 @brief Process BT AMP event Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessBtAmpEventReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_BtAmpEventParamsType*    pwdiBtAmpEventParams = NULL;
   WDI_BtAmpEventRspCb          wdiBtAmpEventRspCb;
   wpt_uint8*                   pSendBuffer         = NULL; 
   wpt_uint16                   usDataOffset        = 0;
   wpt_uint16                   usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
       ( NULL == pEventData->pCBfnc ))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in BT AMP event req %x %x %x",
                 pEventData, pEventData->pEventData, pEventData->pCBfnc);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   pwdiBtAmpEventParams = (WDI_BtAmpEventParamsType*)pEventData->pEventData;
   wdiBtAmpEventRspCb   = (WDI_BtAmpEventRspCb)pEventData->pCBfnc;
   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_BTAMP_EVENT_REQ, 
                         sizeof(pwdiBtAmpEventParams->wdiBtAmpEventInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiBtAmpEventParams->wdiBtAmpEventInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in BT AMP event req %x %x %x",
                 pEventData, pwdiBtAmpEventParams, wdiBtAmpEventRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiBtAmpEventParams->wdiBtAmpEventInfo, 
                   sizeof(pwdiBtAmpEventParams->wdiBtAmpEventInfo)); 

   pWDICtx->wdiReqStatusCB     = pwdiBtAmpEventParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBtAmpEventParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Start Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiBtAmpEventRspCb, pEventData->pUserData, WDI_BTAMP_EVENT_RESP); 
}/*WDI_ProcessBtAmpEventReq*/

/*==========================================================================
                  MISC CONTROL PROCESSING REQUEST API 
==========================================================================*/
/**
 @brief Process Channel Switch Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessChannelSwitchReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SwitchChReqParamsType*   pwdiSwitchChParams;
  WDI_SwitchChRspCb            wdiSwitchChRspCb;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  tSwitchChannelReqMsg         halSwitchChannelReq = {{0}};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in switch req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiSwitchChParams = (WDI_SwitchChReqParamsType*)pEventData->pEventData;
  wdiSwitchChRspCb   = (WDI_SwitchChRspCb)pEventData->pCBfnc;
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CH_SWITCH_REQ, 
                        sizeof(halSwitchChannelReq.switchChannelParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSwitchChannelReq.switchChannelParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in channel switch req %x %x %x",
                pEventData, pwdiSwitchChParams, wdiSwitchChRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halSwitchChannelReq.switchChannelParams.channelNumber = 
                       pwdiSwitchChParams->wdiChInfo.ucChannel;
#ifndef WLAN_FEATURE_VOWIFI    
  halSwitchChannelReq.switchChannelParams.localPowerConstraint = 
                       pwdiSwitchChParams->wdiChInfo.ucLocalPowerConstraint;
#endif
  halSwitchChannelReq.switchChannelParams.secondaryChannelOffset = 
                       pwdiSwitchChParams->wdiChInfo.wdiSecondaryChannelOffset;

#ifdef WLAN_FEATURE_VOWIFI
  halSwitchChannelReq.switchChannelParams.maxTxPower
                            = pwdiSwitchChParams->wdiChInfo.cMaxTxPower; 
  wpalMemoryCopy(halSwitchChannelReq.switchChannelParams.selfStaMacAddr,
                  pwdiSwitchChParams->wdiChInfo.macSelfStaMacAddr,
                  WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(halSwitchChannelReq.switchChannelParams.bssId,
                  pwdiSwitchChParams->wdiChInfo.macBSSId,
                  WDI_MAC_ADDR_LEN);
#endif
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halSwitchChannelReq.switchChannelParams, 
                  sizeof(halSwitchChannelReq.switchChannelParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiSwitchChParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSwitchChParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Switch Channel Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSwitchChRspCb, pEventData->pUserData, WDI_CH_SWITCH_RESP); 
}/*WDI_ProcessChannelSwitchReq*/

/**
 @brief Process Config STA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigStaReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigSTAReqParamsType*  pwdiConfigSTAParams;
  WDI_ConfigSTARspCb           wdiConfigSTARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 

  tConfigStaReqMsg             halConfigStaReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiConfigSTAParams = (WDI_ConfigSTAReqParamsType*)pEventData->pEventData;
  wdiConfigSTARspCb   = (WDI_ConfigSTARspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                           pwdiConfigSTAParams->wdiReqInfo.macBSSID, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIG_STA_REQ, 
                        sizeof(halConfigStaReqMsg.configStaParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halConfigStaReqMsg.configStaParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in config sta req %x %x %x",
                pEventData, pwdiConfigSTAParams, wdiConfigSTARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*Copy the station context*/
  WDI_CopyWDIStaCtxToHALStaCtx( &halConfigStaReqMsg.configStaParams,
                                &pwdiConfigSTAParams->wdiReqInfo);

  if(pwdiConfigSTAParams->wdiReqInfo.wdiSTAType == WDI_STA_ENTRY_SELF)
  {
    /* Need to fill in the self STA Index */
    if ( WDI_STATUS_SUCCESS != 
         WDI_STATableFindStaidByAddr(pWDICtx,
                                     pwdiConfigSTAParams->wdiReqInfo.macSTA,
                                     (wpt_uint8*)&halConfigStaReqMsg.configStaParams.staIdx ))
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "This station does not exist in the WDI Station Table %d");
      wpalMutexRelease(&pWDICtx->wptMutex);
      return WDI_STATUS_E_FAILURE; 
    }
  }
  else
  {
  /* Need to fill in the STA Index to invalid, since at this point we have not
     yet received it from HAL */
    halConfigStaReqMsg.configStaParams.staIdx = WDI_STA_INVALID_IDX;
  }

  /* Need to fill in the BSS index */
  halConfigStaReqMsg.configStaParams.bssIdx = pBSSSes->usBSSIdx;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halConfigStaReqMsg.configStaParams, 
                  sizeof(halConfigStaReqMsg.configStaParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiConfigSTAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiConfigSTAParams->pUserData; 

  wpalMemoryCopy( &pWDICtx->wdiCachedConfigStaReq, 
                  pwdiConfigSTAParams, 
                  sizeof(pWDICtx->wdiCachedConfigStaReq));

  /*-------------------------------------------------------------------------
    Send Config STA Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiConfigSTARspCb, pEventData->pUserData, WDI_CONFIG_STA_RESP); 
}/*WDI_ProcessConfigStaReq*/


/**
 @brief Process Set Link State Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetLinkStateReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetLinkReqParamsType*    pwdiSetLinkParams;
  WDI_SetLinkStateRspCb        wdiSetLinkRspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS;
  tLinkStateParams             halLinkStateReqMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiSetLinkParams = (WDI_SetLinkReqParamsType*)pEventData->pEventData;
  wdiSetLinkRspCb   = (WDI_SetLinkStateRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                           pwdiSetLinkParams->wdiLinkInfo.macBSSID, 
                          &pBSSSes); 

  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Set link request received outside association session");
  }
  else
  {
    /*------------------------------------------------------------------------
      Check if this BSS is being currently processed or queued,
      if queued - queue the new request as well 
    ------------------------------------------------------------------------*/
    if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                "Association sequence for this BSS exists but currently queued");
  
      wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
      wpalMutexRelease(&pWDICtx->wptMutex);
      return wdiStatus; 
    }
  }
  /* If the link is set to enter IDLE - the Session allocated for this BSS
     will be deleted on the Set Link State response comming from HAL
   - cache the request for response processing */
  wpalMemoryCopy(&pWDICtx->wdiCacheSetLinkStReq, pwdiSetLinkParams, 
                 sizeof(pWDICtx->wdiCacheSetLinkStReq));

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_LINK_ST_REQ, 
                        sizeof(halLinkStateReqMsg),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halLinkStateReqMsg) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiSetLinkParams, wdiSetLinkRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy(halLinkStateReqMsg.bssid,
                 pwdiSetLinkParams->wdiLinkInfo.macBSSID, WDI_MAC_ADDR_LEN);
  halLinkStateReqMsg.state = 
     WDI_2_HAL_LINK_STATE(pwdiSetLinkParams->wdiLinkInfo.wdiLinkState);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halLinkStateReqMsg, 
                  sizeof(halLinkStateReqMsg)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetLinkParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetLinkParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Set Link State Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetLinkRspCb, pEventData->pUserData, WDI_SET_LINK_ST_RESP); 
}/*WDI_ProcessSetLinkStateReq*/


/**
 @brief Process Get Stats Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessGetStatsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_GetStatsReqParamsType*   pwdiGetStatsParams;
  WDI_GetStatsRspCb            wdiGetStatsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_macAddr                  macBSSID;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  tHalStatsReqMsg              halStatsReqMsg;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc ) )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Get Stats req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiGetStatsParams = (WDI_GetStatsReqParamsType*)pEventData->pEventData;
  wdiGetStatsRspCb   = (WDI_GetStatsRspCb)pEventData->pCBfnc;

  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                        pwdiGetStatsParams->wdiGetStatsParamsInfo.usSTAIdx, 
                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_GET_STATS_REQ, 
                        sizeof(halStatsReqMsg.statsReqParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halStatsReqMsg.statsReqParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiGetStatsParams, wdiGetStatsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halStatsReqMsg.statsReqParams.staId = 
                  pwdiGetStatsParams->wdiGetStatsParamsInfo.usSTAIdx;
  halStatsReqMsg.statsReqParams.statsMask = 
                  pwdiGetStatsParams->wdiGetStatsParamsInfo.uStatsMask;
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halStatsReqMsg.statsReqParams, 
                  sizeof(halStatsReqMsg.statsReqParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiGetStatsParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiGetStatsParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Get STA Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiGetStatsRspCb, pEventData->pUserData, WDI_GET_STATS_RESP); 
}/*WDI_ProcessGetStatsReq*/

/**
 @brief Process Update Cfg Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateCfgReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateCfgReqParamsType*  pwdiUpdateCfgParams = NULL;
  WDI_UpdateCfgRspCb           wdiUpdateCfgRspCb = NULL;

  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset         = 0;
  wpt_uint16                   usSendSize          = 0;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in switch req");
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiUpdateCfgParams = (WDI_UpdateCfgReqParamsType*)pEventData->pEventData;
  wdiUpdateCfgRspCb   = (WDI_UpdateCfgRspCb)pEventData->pCBfnc;

  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/

  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPDATE_CFG_REQ, 
                        pwdiUpdateCfgParams->uConfigBufferLen + sizeof(wpt_uint32),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset +  pwdiUpdateCfgParams->uConfigBufferLen)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateCfgParams, wdiUpdateCfgRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiUpdateCfgParams->uConfigBufferLen, 
                  sizeof(wpt_uint32)); 
  wpalMemoryCopy( pSendBuffer+usDataOffset+sizeof(wpt_uint32), 
                  pwdiUpdateCfgParams->pConfigBuffer, 
                  pwdiUpdateCfgParams->uConfigBufferLen); 

  pWDICtx->wdiReqStatusCB     = pwdiUpdateCfgParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateCfgParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Update Cfg Request to HAL 
  -------------------------------------------------------------------------*/

  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiUpdateCfgRspCb, pEventData->pUserData, WDI_UPDATE_CFG_RESP); 

}/*WDI_ProcessUpdateCfgReq*/


/**
 @brief Process Add BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBAReqParamsType*  pwdiAddBAParams;
  WDI_AddBARspCb           wdiAddBARspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL; 
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr              macBSSID;

  tAddBAReqMsg             halAddBAReq;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
            "Invalid parameters in Add BA req %x %x %x",
            pEventData, pEventData->pEventData, pEventData->pCBfnc );
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiAddBAParams = (WDI_AddBAReqParamsType*)pEventData->pEventData;
  wdiAddBARspCb   = (WDI_AddBARspCb)pEventData->pCBfnc;

  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                  pwdiAddBAParams->wdiBAInfoType.usSTAIdx, 
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ADD_BA_REQ, 
                        sizeof(halAddBAReq.addBAParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < 
            (usDataOffset + sizeof(halAddBAReq.addBAParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Add BA req %x %x %x",
                pEventData, pwdiAddBAParams, wdiAddBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halAddBAReq.addBAParams.baSessionID = 
                             pwdiAddBAParams->wdiBAInfoType.ucBaSessionID;
  halAddBAReq.addBAParams.winSize = pwdiAddBAParams->wdiBAInfoType.ucWinSize;
#ifdef FEATURE_ON_CHIP_REORDERING
  halAddBAReq.addBAParams.isReorderingDoneOnChip = 
                       pwdiAddBAParams->wdiBAInfoType.bIsReorderingDoneOnChip;
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halAddBAReq.addBAParams, 
                  sizeof(halAddBAReq.addBAParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiAddBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAddBAParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Start Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiAddBARspCb, pEventData->pUserData, 
                        WDI_ADD_BA_RESP); 
}/*WDI_ProcessAddBAReq*/



/**
 @brief Process Trigger BA Request function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessTriggerBAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TriggerBAReqParamsType*  pwdiTriggerBAParams;
  WDI_TriggerBARspCb           wdiTriggerBARspCb;
  wpt_uint8                    ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*          pBSSSes             = NULL;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  WDI_Status                   wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_uint16                   index;
  wpt_macAddr                  macBSSID;
  
  tTriggerBAReqMsg               halTriggerBAReq;
  tTriggerBaReqCandidate*        halTriggerBACandidate;
  WDI_TriggerBAReqCandidateType* wdiTriggerBACandidate;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
            "Invalid parameters in Trigger BA req %x %x %x",
            pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiTriggerBAParams = (WDI_TriggerBAReqParamsType*)pEventData->pEventData;
  wdiTriggerBARspCb = (WDI_TriggerBARspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                  pwdiTriggerBAParams->wdiTriggerBAInfoType.usSTAIdx, 
                                  &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }


  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, 
                  WDI_TRIGGER_BA_REQ, 
                  sizeof(halTriggerBAReq.triggerBAParams) +
                  (sizeof(tTriggerBaReqCandidate) * 
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt),
                  &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < 
            (usDataOffset + sizeof(halTriggerBAReq.triggerBAParams)+
               (sizeof(tTriggerBaReqCandidate) * 
               pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Trigger BA req %x %x %x",
                pEventData, pwdiTriggerBAParams, wdiTriggerBARspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halTriggerBAReq.triggerBAParams.baSessionID = 
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.ucBASessionID;
  halTriggerBAReq.triggerBAParams.baCandidateCnt = 
                  pwdiTriggerBAParams->wdiTriggerBAInfoType.usBACandidateCnt;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halTriggerBAReq.triggerBAParams, 
                  sizeof(halTriggerBAReq.triggerBAParams)); 

  wdiTriggerBACandidate = 
    (WDI_TriggerBAReqCandidateType*)(pwdiTriggerBAParams + 1);
  halTriggerBACandidate = (tTriggerBaReqCandidate*)(pSendBuffer+usDataOffset+
                                 sizeof(halTriggerBAReq.triggerBAParams));
  
  for(index = 0 ; index < halTriggerBAReq.triggerBAParams.baCandidateCnt ; 
                                                                     index++)
  {
    halTriggerBACandidate->staIdx = wdiTriggerBACandidate->usSTAIdx;
    halTriggerBACandidate->tidBitmap = wdiTriggerBACandidate->ucTidBitmap;
    halTriggerBACandidate++;
    wdiTriggerBACandidate++;
  }

  pWDICtx->wdiReqStatusCB     = pwdiTriggerBAParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiTriggerBAParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Start Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiTriggerBARspCb, pEventData->pUserData, 
                        WDI_TRIGGER_BA_RESP); 
}/*WDI_ProcessTriggerBAReq*/



/**
 @brief Process Update Beacon Params  Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateBeaconParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateBeaconParamsType*  pwdiUpdateBeaconParams;
  WDI_UpdateBeaconParamsRspCb  wdiUpdateBeaconParamsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  tUpdateBeaconParams          halUpdateBeaconParams; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiUpdateBeaconParams = (WDI_UpdateBeaconParamsType*)pEventData->pEventData;
  wdiUpdateBeaconParamsRspCb = (WDI_UpdateBeaconParamsRspCb)pEventData->pCBfnc;
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_BCON_PRMS_REQ, 
                        sizeof(halUpdateBeaconParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halUpdateBeaconParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiUpdateBeaconParams, wdiUpdateBeaconParamsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*BSS Index of the BSS*/
  halUpdateBeaconParams.bssIdx =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucBssIdx;
  /*shortPreamble mode. HAL should update all the STA rates when it
    receives this message*/
  halUpdateBeaconParams.fShortPreamble = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfShortPreamble;
  /* short Slot time.*/
  halUpdateBeaconParams.fShortSlotTime = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfShortSlotTime;
  /* Beacon Interval */
  halUpdateBeaconParams.beaconInterval = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.usBeaconInterval;

  /*Protection related */
  halUpdateBeaconParams.llaCoexist = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllaCoexist;
  halUpdateBeaconParams.llbCoexist = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllbCoexist;
  halUpdateBeaconParams.llgCoexist = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllgCoexist;
  halUpdateBeaconParams.ht20MhzCoexist  = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucHt20MhzCoexist;
  halUpdateBeaconParams.llnNonGFCoexist = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucllnNonGFCoexist;
  halUpdateBeaconParams.fLsigTXOPProtectionFullSupport = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfLsigTXOPProtectionFullSupport;
  halUpdateBeaconParams.fRIFSMode =
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.ucfRIFSMode;
  halUpdateBeaconParams.paramChangeBitmap = 
    pwdiUpdateBeaconParams->wdiUpdateBeaconParamsInfo.usChangeBitmap;

  wpalMemoryCopy( pSendBuffer+usDataOffset, &halUpdateBeaconParams, 
                  sizeof(halUpdateBeaconParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiUpdateBeaconParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateBeaconParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Del TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiUpdateBeaconParamsRspCb, pEventData->pUserData, WDI_UPD_BCON_PRMS_RESP); 
}/*WDI_ProcessUpdateBeaconParamsReq*/



/**
 @brief Process Send Beacon template  Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSendBeaconParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SendBeaconParamsType*    pwdiSendBeaconParams;
  WDI_SendBeaconParamsRspCb    wdiSendBeaconParamsRspCb;
  wpt_uint8*                   pSendBuffer         = NULL; 
  wpt_uint16                   usDataOffset        = 0;
  wpt_uint16                   usSendSize          = 0;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  tSendBeaconReqMsg            halSendBeaconReq;
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Send BEacon Params req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiSendBeaconParams = (WDI_SendBeaconParamsType*)pEventData->pEventData;
  wdiSendBeaconParamsRspCb   = (WDI_SendBeaconParamsRspCb)pEventData->pCBfnc;
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SND_BCON_REQ, 
                        sizeof(halSendBeaconReq.sendBeaconParam),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSendBeaconReq.sendBeaconParam) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in send beacon req %x %x %x",
                pEventData, pwdiSendBeaconParams, wdiSendBeaconParamsRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy(halSendBeaconReq.sendBeaconParam.bssId,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.macBSSID,
                  WDI_MAC_ADDR_LEN);
  halSendBeaconReq.sendBeaconParam.beaconLength = 
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beaconLength;
  wpalMemoryCopy(halSendBeaconReq.sendBeaconParam.beacon,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beacon,
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.beaconLength);
#ifdef WLAN_SOFTAP_FEATURE
  halSendBeaconReq.sendBeaconParam.timIeOffset = 
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.timIeOffset;
#endif
#ifdef WLAN_FEATURE_P2P
  halSendBeaconReq.sendBeaconParam.p2pIeOffset = 
                  pwdiSendBeaconParams->wdiSendBeaconParamsInfo.usP2PIeOffset;
#endif

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halSendBeaconReq.sendBeaconParam, 
                  sizeof(halSendBeaconReq.sendBeaconParam)); 

  pWDICtx->wdiReqStatusCB     = pwdiSendBeaconParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSendBeaconParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Del TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSendBeaconParamsRspCb, pEventData->pUserData, WDI_SND_BCON_RESP); 
}/*WDI_ProcessSendBeaconParamsReq*/

/**
 @brief Process Update Beacon Params  Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateProbeRspTemplateReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_UpdateProbeRspTemplateParamsType*  pwdiUpdateProbeRespTmplParams;
  WDI_UpdateProbeRspTemplateRspCb        wdiUpdateProbeRespTmplRspCb;
  wpt_uint8*                             pSendBuffer         = NULL; 
  wpt_uint16                             usDataOffset        = 0;
  wpt_uint16                             usSendSize          = 0;
  tSendProbeRespReqParams                halUpdateProbeRspTmplParams; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiUpdateProbeRespTmplParams = 
    (WDI_UpdateProbeRspTemplateParamsType*)pEventData->pEventData;
  wdiUpdateProbeRespTmplRspCb = 
    (WDI_UpdateProbeRspTemplateRspCb)pEventData->pCBfnc;
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPD_PROBE_RSP_TEMPLATE_REQ, 
                        sizeof(halUpdateProbeRspTmplParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halUpdateProbeRspTmplParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
     pEventData, pwdiUpdateProbeRespTmplParams, wdiUpdateProbeRespTmplRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy(halUpdateProbeRspTmplParams.bssId,
                 pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.macBSSID, 
                 WDI_MAC_ADDR_LEN);

  halUpdateProbeRspTmplParams.probeRespTemplateLen = 
    pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.uProbeRespTemplateLen;

  wpalMemoryCopy(halUpdateProbeRspTmplParams.pProbeRespTemplate,
    pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.pProbeRespTemplate,
                 BEACON_TEMPLATE_SIZE);     


  wpalMemoryCopy(halUpdateProbeRspTmplParams.ucProxyProbeReqValidIEBmap,
           pwdiUpdateProbeRespTmplParams->wdiProbeRspTemplateInfo.uaProxyProbeReqValidIEBmap,
                 WDI_PROBE_REQ_BITMAP_IE_LEN);

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halUpdateProbeRspTmplParams, 
                  sizeof(halUpdateProbeRspTmplParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiUpdateProbeRespTmplParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiUpdateProbeRespTmplParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Update Probe Resp Template Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiUpdateProbeRespTmplRspCb, pEventData->pUserData, 
                       WDI_UPD_PROBE_RSP_TEMPLATE_RESP); 
}/*WDI_ProcessUpdateProbeRspTemplateReq*/

/**
 @brief Process NV blob download function (called when Main FSM 
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessNvDownloadReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  WDI_NvDownloadReqParamsType*  pwdiNvDownloadReqParams = NULL;
  WDI_NvDownloadRspCb      wdiNvDownloadRspCb = NULL;
  
  /*-------------------------------------------------------------------------
     Sanity check       
   -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiNvDownloadReqParams = 
                 (WDI_NvDownloadReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiNvDownloadRspCb = 
                (WDI_NvDownloadRspCb)pEventData->pCBfnc)))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,	eWLAN_PAL_TRACE_LEVEL_WARN,
            "Invalid parameters in NV Download req %x %x %x",
            pEventData, pwdiNvDownloadReqParams, wdiNvDownloadRspCb);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE; 
  }

  /*Intialize the Nv Blob Info */
  pWDICtx->wdiNvBlobInfo.usTotalFragment = 
                TOTALFRAGMENTS(pwdiNvDownloadReqParams->wdiBlobInfo.uBlobSize);
  /*cache the wdi nv request message here if the the first fragment
   * To issue the request to HAL for the next fragment */
  if( 0 == pWDICtx->wdiNvBlobInfo.usCurrentFragment)
  {
    wpalMemoryCopy(&pWDICtx->wdiCachedNvDownloadReq, 
                 pwdiNvDownloadReqParams, 
                 sizeof(pWDICtx->wdiCachedNvDownloadReq));
  }

  return WDI_SendNvBlobReq(pWDICtx,pEventData);
}

#ifdef WLAN_FEATURE_VOWIFI
/**
 @brief Process Set Max Tx Power Request function (called when Main    
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status WDI_ProcessSetMaxTxPowerReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetMaxTxPowerParamsType*      pwdiSetMaxTxPowerParams;
  WDA_SetMaxTxPowerRspCb            wdiSetMaxTxPowerRspCb;
  wpt_uint8*                        pSendBuffer         = NULL; 
  wpt_uint16                        usDataOffset        = 0;
  wpt_uint16                        usSendSize          = 0;
  tSetMaxTxPwrReq                   halSetMaxTxPower;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set Max Tx Power Params req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiSetMaxTxPowerParams = 
    (WDI_SetMaxTxPowerParamsType*)pEventData->pEventData;
  wdiSetMaxTxPowerRspCb = 
    (WDA_SetMaxTxPowerRspCb)pEventData->pCBfnc;

  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_MAX_TX_POWER_REQ, 
                        sizeof(halSetMaxTxPower.setMaxTxPwrParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetMaxTxPower.setMaxTxPwrParams) 
)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get Set Max Tx Power req %x %x %x",
                pEventData, pwdiSetMaxTxPowerParams, wdiSetMaxTxPowerRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }


  wpalMemoryCopy(halSetMaxTxPower.setMaxTxPwrParams.bssId,
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.macBSSId,
                  WDI_MAC_ADDR_LEN);

  wpalMemoryCopy(halSetMaxTxPower.setMaxTxPwrParams.selfStaMacAddr,
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.macSelfStaMacAddr,
                  WDI_MAC_ADDR_LEN);
  halSetMaxTxPower.setMaxTxPwrParams.power = 
                  pwdiSetMaxTxPowerParams->wdiMaxTxPowerInfo.ucPower;
  
  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halSetMaxTxPower.setMaxTxPwrParams, 
                  sizeof(halSetMaxTxPower.setMaxTxPwrParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetMaxTxPowerParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetMaxTxPowerParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Del TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetMaxTxPowerRspCb, pEventData->pUserData, 
                                                      WDI_SET_MAX_TX_POWER_RESP); 
  
}
#endif

#ifdef WLAN_FEATURE_P2P

/**
 @brief Process P2P Notice Of Absence Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessP2PGONOAReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetP2PGONOAReqParamsType*          pwdiP2PGONOAReqParams;
  WDI_SetP2PGONOAReqParamsRspCb          wdiP2PGONOAReqRspCb;
  wpt_uint8*                             pSendBuffer         = NULL; 
  wpt_uint16                             usDataOffset        = 0;
  wpt_uint16                             usSendSize          = 0;
  tSetP2PGONOAParams                     halSetP2PGONOAParams; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == pEventData->pCBfnc))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in P2P GO NOA REQ %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  pwdiP2PGONOAReqParams = 
    (WDI_SetP2PGONOAReqParamsType*)pEventData->pEventData;
  wdiP2PGONOAReqRspCb = 
    (WDI_SetP2PGONOAReqParamsRspCb)pEventData->pCBfnc;
  /*-----------------------------------------------------------------------
    Get message buffer
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, 
                        WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ, 
                        sizeof(halSetP2PGONOAParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(halSetP2PGONOAParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set P2P GO NOA REQ %x %x %x",
     pEventData, pwdiP2PGONOAReqParams, wdiP2PGONOAReqRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halSetP2PGONOAParams.opp_ps = 
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucOpp_ps;
  halSetP2PGONOAParams.ctWindow = 
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uCtWindow;
  halSetP2PGONOAParams.count = pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucCount;
  halSetP2PGONOAParams.duration = 
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uDuration;
  halSetP2PGONOAParams.interval = 
                           pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uInterval;
  halSetP2PGONOAParams.single_noa_duration = 
                 pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.uSingle_noa_duration;
  halSetP2PGONOAParams.psSelection = 
                   pwdiP2PGONOAReqParams->wdiP2PGONOAInfo.ucPsSelection;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halSetP2PGONOAParams, 
                  sizeof(halSetP2PGONOAParams)); 

  pWDICtx->wdiReqStatusCB     = pwdiP2PGONOAReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiP2PGONOAReqParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Update Probe Resp Template Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiP2PGONOAReqRspCb, pEventData->pUserData, 
                       WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP); 
}/*WDI_ProcessP2PGONOAReq*/

#endif

/**
 @brief Process Enter IMPS Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterImpsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_EnterImpsRspCb       wdiEnterImpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (wdiEnterImpsRspCb   = (WDI_EnterImpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Enter IMPS req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_IMPS_REQ, 
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter IMPS req %x %x",
                 pEventData, wdiEnterImpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiEnterImpsRspCb, pEventData->pUserData, WDI_ENTER_IMPS_RESP); 
}/*WDI_ProcessEnterImpsReq*/

/**
 @brief Process Exit IMPS Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitImpsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitImpsRspCb        wdiExitImpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (wdiExitImpsRspCb   = (WDI_ExitImpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Exit IMPS req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_IMPS_REQ, 
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit IMPS req %x %x",
                 pEventData, wdiExitImpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiExitImpsRspCb, pEventData->pUserData, WDI_EXIT_IMPS_RESP); 
}/*WDI_ProcessExitImpsReq*/

/**
 @brief    Function to handle the ack from DXE once the power 
           state is set.
 @param    None 
    
 @see 
 @return void 
*/
void
WDI_SetPowerStateCb
(
   wpt_status status,
   unsigned int dxePhyAddr,
   void      *pContext
)
{
   wpt_status              wptStatus;
   WDI_ControlBlockType *pCB = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
   if(eWLAN_PAL_STATUS_E_FAILURE == status )
   {
      //it shouldn't happen, put an error msg
   }
   /* 
    * Trigger the event to bring the Enter BMPS req function to come 
    * out of wait 
*/
   if( NULL != pContext )
   {
      pCB = (WDI_ControlBlockType *)pContext; 
   }
   else
   {
      //put an error msg 
      pCB = &gWDICb;
   }
   pCB->dxePhyAddr = dxePhyAddr;
   wptStatus  = wpalEventSet(&pCB->setPowerStateEvent);
   if ( eWLAN_PAL_STATUS_SUCCESS !=  wptStatus )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "Failed to set an event");

      WDI_ASSERT(0); 
   }
   return;
}

/**
 @brief Process Enter BMPS Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterBmpsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_EnterBmpsReqParamsType*  pwdiEnterBmpsReqParams = NULL;
   WDI_EnterBmpsRspCb           wdiEnterBmpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalEnterBmpsReqParams   enterBmpsReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
     Sanity check 
  -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiEnterBmpsReqParams = (WDI_EnterBmpsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiEnterBmpsRspCb   = (WDI_EnterBmpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Enter BMPS req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_BMPS_REQ, 
                         sizeof(enterBmpsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(enterBmpsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter BMPS req %x %x %x",
                 pEventData, pwdiEnterBmpsReqParams, wdiEnterBmpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /* Reset the event to be not signalled */
   if(WDI_STATUS_SUCCESS != wpalEventReset(&pWDICtx->setPowerStateEvent) )
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to reset an event");

      WDI_ASSERT(0); 
      return VOS_STATUS_E_FAILURE;
   }

   // notify DTS that we are entering BMPS
   WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_BMPS, WDI_SetPowerStateCb);

/*
    * Wait for the event to be set once the ACK comes back from DXE 
    */
   if(WDI_STATUS_SUCCESS != wpalEventWait(&pWDICtx->setPowerStateEvent, 
                                          WDI_SET_POWER_STATE_TIMEOUT))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "WDI Init failed to wait on an event");

      WDI_ASSERT(0); 
      return VOS_STATUS_E_FAILURE;
   }

   enterBmpsReq.bssIdx = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucBssIdx;
   enterBmpsReq.tbtt = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.uTbtt;
   enterBmpsReq.dtimCount = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucDtimCount;
   enterBmpsReq.dtimPeriod = pwdiEnterBmpsReqParams->wdiEnterBmpsInfo.ucDtimPeriod;

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &enterBmpsReq, 
                   sizeof(enterBmpsReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiEnterBmpsReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiEnterBmpsReqParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiEnterBmpsRspCb, pEventData->pUserData, WDI_ENTER_BMPS_RESP); 
}/*WDI_ProcessEnterBmpsReq*/

/**
 @brief Process Exit BMPS Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitBmpsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitBmpsReqParamsType*  pwdiExitBmpsReqParams = NULL;
   WDI_ExitBmpsRspCb           wdiExitBmpsRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalExitBmpsReqParams    exitBmpsReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiExitBmpsReqParams = (WDI_ExitBmpsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiExitBmpsRspCb   = (WDI_ExitBmpsRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Exit BMPS req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_BMPS_REQ, 
                         sizeof(exitBmpsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(exitBmpsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit BMPS req %x %x %x",
                 pEventData, pwdiExitBmpsReqParams, wdiExitBmpsRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }
   exitBmpsReq.sendDataNull = pwdiExitBmpsReqParams->wdiExitBmpsInfo.ucSendDataNull;

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &exitBmpsReq, 
                   sizeof(exitBmpsReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiExitBmpsReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiExitBmpsReqParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiExitBmpsRspCb, pEventData->pUserData, WDI_EXIT_BMPS_RESP); 
}/*WDI_ProcessExitBmpsReq*/

/**
 @brief Process Enter UAPSD Request function (called when Main 
        FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterUapsdReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_EnterUapsdReqParamsType*  pwdiEnterUapsdReqParams = NULL;
   WDI_EnterUapsdRspCb           wdiEnterUapsdRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tUapsdReqParams          enterUapsdReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiEnterUapsdReqParams = (WDI_EnterUapsdReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiEnterUapsdRspCb   = (WDI_EnterUapsdRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Enter UAPSD req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_ENTER_UAPSD_REQ, 
                         sizeof(enterUapsdReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(enterUapsdReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Enter UAPSD req %x %x %x",
                 pEventData, pwdiEnterUapsdReqParams, wdiEnterUapsdRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   enterUapsdReq.beDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBeDeliveryEnabled;
   enterUapsdReq.beTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBeTriggerEnabled;
   enterUapsdReq.bkDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBkDeliveryEnabled;
   enterUapsdReq.bkTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucBkTriggerEnabled;
   enterUapsdReq.viDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucViDeliveryEnabled;
   enterUapsdReq.viTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucViTriggerEnabled;
   enterUapsdReq.voDeliveryEnabled  = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucVoDeliveryEnabled;
   enterUapsdReq.voTriggerEnabled   = pwdiEnterUapsdReqParams->wdiEnterUapsdInfo.ucVoTriggerEnabled;

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &enterUapsdReq, 
                   sizeof(enterUapsdReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiEnterUapsdReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiEnterUapsdReqParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiEnterUapsdRspCb, pEventData->pUserData, WDI_ENTER_UAPSD_RESP); 
}/*WDI_ProcessEnterUapsdReq*/

/**
 @brief Process Exit UAPSD Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitUapsdReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ExitUapsdRspCb       wdiExitUapsdRspCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (wdiExitUapsdRspCb   = (WDI_ExitUapsdRspCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Exit UAPSD req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_EXIT_UAPSD_REQ, 
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Exit UAPSD req %x %x",
                 pEventData, wdiExitUapsdRspCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiExitUapsdRspCb, pEventData->pUserData, WDI_EXIT_UAPSD_RESP); 
}/*WDI_ProcessExitUapsdReq*/

/**
 @brief Process Set UAPSD params Request function (called when 
        Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetUapsdAcParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SetUapsdAcParamsReqParamsType*  pwdiSetUapsdAcParams = NULL;
  WDI_SetUapsdAcParamsCb              wdiSetUapsdAcParamsCb = NULL;
  wpt_uint8*               pSendBuffer         = NULL; 
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  tUapsdInfo               uapsdAcParamsReq;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiSetUapsdAcParams = (WDI_SetUapsdAcParamsReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiSetUapsdAcParamsCb   = (WDI_SetUapsdAcParamsCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set UAPSD params req");
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_UAPSD_PARAM_REQ, 
                        sizeof(uapsdAcParamsReq),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(uapsdAcParamsReq) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Set UAPSD params req %x %x %x",
                pEventData, pwdiSetUapsdAcParams, wdiSetUapsdAcParamsCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  uapsdAcParamsReq.ac = pwdiSetUapsdAcParams->wdiUapsdInfo.ucAc;
  uapsdAcParamsReq.staidx = pwdiSetUapsdAcParams->wdiUapsdInfo.ucStaIdx;
  uapsdAcParamsReq.up = pwdiSetUapsdAcParams->wdiUapsdInfo.ucUp;
  uapsdAcParamsReq.delayInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uDelayInterval;
  uapsdAcParamsReq.srvInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uSrvInterval;
  uapsdAcParamsReq.susInterval = pwdiSetUapsdAcParams->wdiUapsdInfo.uSusInterval;

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &uapsdAcParamsReq, 
                  sizeof(uapsdAcParamsReq)); 

  pWDICtx->wdiReqStatusCB     = pwdiSetUapsdAcParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiSetUapsdAcParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Get STA Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiSetUapsdAcParamsCb, pEventData->pUserData, WDI_SET_UAPSD_PARAM_RESP); 
}/*WDI_ProcessSetUapsdAcParamsReq*/

/**
 @brief Process update UAPSD params Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateUapsdParamsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_UpdateUapsdReqParamsType*  pwdiUpdateUapsdReqParams = NULL;
   WDI_UpdateUapsdParamsCb        wdiUpdateUapsdParamsCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiUpdateUapsdReqParams = (WDI_UpdateUapsdReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiUpdateUapsdParamsCb   = (WDI_UpdateUapsdParamsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Update UAPSD params req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_UPDATE_UAPSD_PARAM_REQ, 
                         sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Update UAPSD params req %x %x %x",
                 pEventData, pwdiUpdateUapsdReqParams, wdiUpdateUapsdParamsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo, 
                   sizeof(pwdiUpdateUapsdReqParams->wdiUpdateUapsdInfo)); 

   pWDICtx->wdiReqStatusCB     = pwdiUpdateUapsdReqParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiUpdateUapsdReqParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiUpdateUapsdParamsCb, pEventData->pUserData, WDI_UPDATE_UAPSD_PARAM_RESP); 
}/*WDI_ProcessUpdateUapsdParamsReq*/

/**
 @brief Process Configure RXP filter Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigureRxpFilterReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigureRxpFilterReqParamsType*  pwdiRxpFilterParams = NULL;
  WDI_ConfigureRxpFilterCb              wdiConfigureRxpFilterCb = NULL;
  wpt_uint8*               pSendBuffer         = NULL; 
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == (pwdiRxpFilterParams = (WDI_ConfigureRxpFilterReqParamsType*)pEventData->pEventData)) ||
      ( NULL == (wdiConfigureRxpFilterCb   = (WDI_ConfigureRxpFilterCb)pEventData->pCBfnc)))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Configure RXP filter req");
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIGURE_RXP_FILTER_REQ, 
                        sizeof(pwdiRxpFilterParams->uFiltermask),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(pwdiRxpFilterParams->uFiltermask) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in Set UAPSD params req %x %x %x",
                pEventData, pwdiRxpFilterParams, wdiConfigureRxpFilterCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &pwdiRxpFilterParams->uFiltermask, 
                  sizeof(pwdiRxpFilterParams->uFiltermask)); 

  pWDICtx->wdiReqStatusCB     = pwdiRxpFilterParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiRxpFilterParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Get STA Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiConfigureRxpFilterCb, pEventData->pUserData, WDI_CONFIGURE_RXP_FILTER_RESP); 
}/*WDI_ProcessConfigureRxpFilterReq*/

/**
 @brief Process set beacon filter Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBeaconFilterReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_BeaconFilterReqParamsType*  pwdiBeaconFilterParams = NULL;
   WDI_SetBeaconFilterCb           wdiBeaconFilterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiBeaconFilterParams = (WDI_BeaconFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiBeaconFilterCb   = (WDI_SetBeaconFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Set beacon filter req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_BEACON_FILTER_REQ, 
                         sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) + pwdiBeaconFilterParams->wdiBeaconFilterInfo.usIeNum * sizeof(tBeaconFilterIe),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Set beacon filter req %x %x %x",
                 pEventData, pwdiBeaconFilterParams, wdiBeaconFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiBeaconFilterParams->wdiBeaconFilterInfo, 
                   sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo)); 
   wpalMemoryCopy( pSendBuffer+usDataOffset+sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo), 
                   &pwdiBeaconFilterParams->aFilters[0], 
                   pwdiBeaconFilterParams->wdiBeaconFilterInfo.usIeNum * sizeof(tBeaconFilterIe)); 

   pWDICtx->wdiReqStatusCB     = pwdiBeaconFilterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBeaconFilterParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiBeaconFilterCb, pEventData->pUserData, WDI_SET_BEACON_FILTER_RESP); 
}/*WDI_ProcessSetBeaconFilterReq*/

/**
 @brief Process remove beacon filter Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemBeaconFilterReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_RemBeaconFilterReqParamsType*  pwdiBeaconFilterParams = NULL;
   WDI_RemBeaconFilterCb              wdiBeaconFilterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiBeaconFilterParams = (WDI_RemBeaconFilterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiBeaconFilterCb   = (WDI_RemBeaconFilterCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in remove beacon filter req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_REM_BEACON_FILTER_REQ, 
                         sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in remove beacon filter req %x %x %x",
                  pEventData, pwdiBeaconFilterParams, wdiBeaconFilterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiBeaconFilterParams->wdiBeaconFilterInfo, 
                   sizeof(pwdiBeaconFilterParams->wdiBeaconFilterInfo)); 

   pWDICtx->wdiReqStatusCB     = pwdiBeaconFilterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiBeaconFilterParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiBeaconFilterCb, pEventData->pUserData, WDI_REM_BEACON_FILTER_RESP); 
}

/**
 @brief Process set RSSI thresholds Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetRSSIThresholdsReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_SetRSSIThresholdsReqParamsType*  pwdiRSSIThresholdsParams = NULL;
   WDI_SetRSSIThresholdsCb              wdiRSSIThresholdsCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalRSSIThresholds       rssiThresholdsReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiRSSIThresholdsParams = (WDI_SetRSSIThresholdsReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiRSSIThresholdsCb   = (WDI_SetRSSIThresholdsCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in set RSSI thresholds req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_SET_RSSI_THRESHOLDS_REQ, 
                         sizeof(rssiThresholdsReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(rssiThresholdsReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in remove beacon filter req %x %x %x",
                  pEventData, pwdiRSSIThresholdsParams, wdiRSSIThresholdsCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   rssiThresholdsReq.bReserved10 = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bReserved10;
   rssiThresholdsReq.bRssiThres1NegNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres1NegNotify;
   rssiThresholdsReq.bRssiThres1PosNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres1PosNotify;
   rssiThresholdsReq.bRssiThres2NegNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres2NegNotify;
   rssiThresholdsReq.bRssiThres2PosNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres2PosNotify;
   rssiThresholdsReq.bRssiThres3NegNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres3NegNotify;
   rssiThresholdsReq.bRssiThres3PosNotify = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.bRssiThres3PosNotify;
   rssiThresholdsReq.ucRssiThreshold1 = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold1;
   rssiThresholdsReq.ucRssiThreshold2 = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold2;
   rssiThresholdsReq.ucRssiThreshold3 = 
      pwdiRSSIThresholdsParams->wdiRSSIThresholdsInfo.ucRssiThreshold3;

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &rssiThresholdsReq, 
                   sizeof(rssiThresholdsReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiRSSIThresholdsParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiRSSIThresholdsParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiRSSIThresholdsCb, pEventData->pUserData, WDI_SET_RSSI_THRESHOLDS_RESP); 
}

/**
 @brief Process set RSSI thresholds Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessHostOffloadReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_HostOffloadReqParamsType*  pwdiHostOffloadParams = NULL;
   WDI_HostOffloadCb              wdiHostOffloadCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalHostOffloadReq       hostOffloadReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiHostOffloadParams = (WDI_HostOffloadReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiHostOffloadCb   = (WDI_HostOffloadCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in host offload req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_HOST_OFFLOAD_REQ, 
                         sizeof(hostOffloadReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(hostOffloadReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in host offload req %x %x %x",
                  pEventData, pwdiHostOffloadParams, wdiHostOffloadCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   hostOffloadReq.offloadType = pwdiHostOffloadParams->wdiHostOffloadInfo.ucOffloadType;
   hostOffloadReq.enableOrDisable = pwdiHostOffloadParams->wdiHostOffloadInfo.ucEnableOrDisable;
   if( 0 == hostOffloadReq.offloadType )
   {
      wpalMemoryCopy(hostOffloadReq.params.hostIpv4Addr,
                     pwdiHostOffloadParams->wdiHostOffloadInfo.params.aHostIpv4Addr,
                     4);
   }
   else
   {
      wpalMemoryCopy(hostOffloadReq.params.hostIpv6Addr,
                     pwdiHostOffloadParams->wdiHostOffloadInfo.params.aHostIpv6Addr,
                     16);
   }
   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &hostOffloadReq, 
                   sizeof(hostOffloadReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiHostOffloadParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiHostOffloadParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiHostOffloadCb, pEventData->pUserData, WDI_HOST_OFFLOAD_RESP); 
}/*WDI_ProcessHostOffloadReq*/

/**
 @brief Process Wowl add bc ptrn Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlAddBcPtrnReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlAddBcPtrnReqParamsType*  pwdiWowlAddBcPtrnParams = NULL;
   WDI_WowlAddBcPtrnCb              wdiWowlAddBcPtrnCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlAddBcastPtrn     wowlAddBcPtrnReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlAddBcPtrnParams = (WDI_WowlAddBcPtrnReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlAddBcPtrnCb   = (WDI_WowlAddBcPtrnCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Wowl add bc ptrn req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_ADD_BC_PTRN_REQ, 
                         sizeof(wowlAddBcPtrnReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlAddBcPtrnReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl add bc ptrn req %x %x %x",
                  pEventData, pwdiWowlAddBcPtrnParams, wdiWowlAddBcPtrnCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wowlAddBcPtrnReq.ucPatternId = 
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternId;
   wowlAddBcPtrnReq.ucPatternByteOffset = 
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternByteOffset;
   wowlAddBcPtrnReq.ucPatternMaskSize = 
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize;
   wowlAddBcPtrnReq.ucPatternSize = 
      pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternSize;
   wpalMemoryCopy(wowlAddBcPtrnReq.ucPattern,
                  pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPattern,
                  pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternSize);
   wpalMemoryCopy(wowlAddBcPtrnReq.ucPatternMask,
                  pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMask,
                  pwdiWowlAddBcPtrnParams->wdiWowlAddBcPtrnInfo.ucPatternMaskSize);

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &wowlAddBcPtrnReq, 
                   sizeof(wowlAddBcPtrnReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiWowlAddBcPtrnParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlAddBcPtrnParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiWowlAddBcPtrnCb, pEventData->pUserData, WDI_WOWL_ADD_BC_PTRN_RESP); 
}/*WDI_ProcessWowlAddBcPtrnReq*/

/**
 @brief Process Wowl delete bc ptrn Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlDelBcPtrnReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlDelBcPtrnReqParamsType*  pwdiWowlDelBcPtrnParams = NULL;
   WDI_WowlDelBcPtrnCb              wdiWowlDelBcPtrnCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlDelBcastPtrn     wowlDelBcPtrnReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlDelBcPtrnParams = (WDI_WowlDelBcPtrnReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlDelBcPtrnCb   = (WDI_WowlDelBcPtrnCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Wowl del bc ptrn req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_DEL_BC_PTRN_REQ, 
                         sizeof(wowlDelBcPtrnReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlDelBcPtrnReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl del bc ptrn req %x %x %x",
                  pEventData, pwdiWowlDelBcPtrnParams, wdiWowlDelBcPtrnCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wowlDelBcPtrnReq.ucPatternId = 
      pwdiWowlDelBcPtrnParams->wdiWowlDelBcPtrnInfo.ucPatternId;
   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &wowlDelBcPtrnReq, 
                   sizeof(wowlDelBcPtrnReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiWowlDelBcPtrnParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlDelBcPtrnParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiWowlDelBcPtrnCb, pEventData->pUserData, WDI_WOWL_DEL_BC_PTRN_RESP); 
}/*WDI_ProcessWowlDelBcPtrnReq*/

/**
 @brief Process Wowl enter Request function (called 
        when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlEnterReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlEnterReqParamsType*  pwdiWowlEnterParams = NULL;
   WDI_WowlEnterReqCb           wdiWowlEnterCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   tHalWowlEnterParams      wowlEnterReq;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiWowlEnterParams = (WDI_WowlEnterReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiWowlEnterCb   = (WDI_WowlEnterReqCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Wowl enter req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_ENTER_REQ, 
                         sizeof(wowlEnterReq),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(wowlEnterReq) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                  "Unable to get send buffer in Wowl enter req %x %x %x",
                  pEventData, pwdiWowlEnterParams, wdiWowlEnterCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wowlEnterReq.ucMagicPktEnable = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucMagicPktEnable;
   wowlEnterReq.ucPatternFilteringEnable = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucPatternFilteringEnable;
   wowlEnterReq.ucUcastPatternFilteringEnable = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucUcastPatternFilteringEnable;
   wowlEnterReq.ucWowChnlSwitchRcv = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowChnlSwitchRcv;
   wowlEnterReq.ucWowDeauthRcv = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowDeauthRcv;
   wowlEnterReq.ucWowDisassocRcv = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowDisassocRcv;
   wowlEnterReq.ucWowMaxMissedBeacons = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowMaxMissedBeacons;
   wowlEnterReq.ucWowMaxSleepUsec = 
      pwdiWowlEnterParams->wdiWowlEnterInfo.ucWowMaxSleepUsec;
   wpalMemoryCopy(wowlEnterReq.magicPtrn,
                  pwdiWowlEnterParams->wdiWowlEnterInfo.magicPtrn,
                  sizeof(tSirMacAddr));

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &wowlEnterReq, 
                   sizeof(wowlEnterReq)); 

   pWDICtx->wdiReqStatusCB     = pwdiWowlEnterParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiWowlEnterParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiWowlEnterCb, pEventData->pUserData, WDI_WOWL_ENTER_RESP); 
}/*WDI_ProcessWowlEnterReq*/

/**
 @brief Process Wowl exit Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlExitReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_WowlExitReqCb           wdiWowlExitCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (wdiWowlExitCb   = (WDI_WowlExitReqCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Wowl exit req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_WOWL_EXIT_REQ, 
                                                     0,
                                                     &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Wowl Exit req %x %x",
                 pEventData, wdiWowlExitCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiWowlExitCb, pEventData->pUserData, WDI_WOWL_EXIT_RESP); 
}/*WDI_ProcessWowlExitReq*/

/**
 @brief Process Configure Apps Cpu Wakeup State Request function
        (called when Main FSM allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigureAppsCpuWakeupStateReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_ConfigureAppsCpuWakeupStateReqParamsType*  pwdiAppsCpuWakeupStateParams = NULL;
   WDI_ConfigureAppsCpuWakeupStateCb              wdiConfigureAppsCpuWakeupStateCb = NULL;
   wpt_uint8*               pSendBuffer         = NULL; 
   wpt_uint16               usDataOffset        = 0;
   wpt_uint16               usSendSize          = 0;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pEventData ) ||
       ( NULL == (pwdiAppsCpuWakeupStateParams = (WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEventData->pEventData)) ||
       ( NULL == (wdiConfigureAppsCpuWakeupStateCb   = (WDI_ConfigureAppsCpuWakeupStateCb)pEventData->pCBfnc)))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in Apps Cpu Wakeup State req");
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-----------------------------------------------------------------------
     Get message buffer
     ! TO DO : proper conversion into the HAL Message Request Format 
   -----------------------------------------------------------------------*/
   if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ, 
                         sizeof(pwdiAppsCpuWakeupStateParams->bIsAppsAwake),
                         &pSendBuffer, &usDataOffset, &usSendSize))||
       ( usSendSize < (usDataOffset + sizeof(pwdiAppsCpuWakeupStateParams->bIsAppsAwake) )))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Unable to get send buffer in Apps CPU Wakeup State req %x %x %x",
                 pEventData, pwdiAppsCpuWakeupStateParams, wdiConfigureAppsCpuWakeupStateCb);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   wpalMemoryCopy( pSendBuffer+usDataOffset, 
                   &pwdiAppsCpuWakeupStateParams->bIsAppsAwake, 
                   sizeof(pwdiAppsCpuWakeupStateParams->bIsAppsAwake)); 

   pWDICtx->wdiReqStatusCB     = pwdiAppsCpuWakeupStateParams->wdiReqStatusCB;
   pWDICtx->pReqStatusUserData = pwdiAppsCpuWakeupStateParams->pUserData; 

   /*-------------------------------------------------------------------------
     Send Get STA Request to HAL 
   -------------------------------------------------------------------------*/
   return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                        wdiConfigureAppsCpuWakeupStateCb, pEventData->pUserData, 
                        WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP); 
}/*WDI_ProcessConfigureAppsCpuWakeupStateReq*/

#ifdef WLAN_FEATURE_VOWIFI_11R
/**
 @brief Process Aggregated Add TSpec Request function (called when Main FSM
        allows it)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAggrAddTSpecReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AggrAddTSReqParamsType*  pwdiAggrAddTSParams;
  WDI_AggrAddTsRspCb           wdiAggrAddTSRspCb;
  wpt_uint8                ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*      pBSSSes             = NULL;
  wpt_uint8*               pSendBuffer         = NULL; 
  wpt_uint16               usDataOffset        = 0;
  wpt_uint16               usSendSize          = 0;
  WDI_Status               wdiStatus           = WDI_STATUS_SUCCESS; 
  wpt_macAddr              macBSSID;
  tAggrAddTsReq            halAggrAddTsReq;
  int i;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) || ( NULL == pEventData->pEventData ) ||
      ( NULL == pEventData->pCBfnc ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in del BSS req %x %x %x",
                pEventData, pEventData->pEventData, pEventData->pCBfnc);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }
  wpalMemoryFill( &halAggrAddTsReq, sizeof(tAggrAddTsReq), 0 );
  pwdiAggrAddTSParams = (WDI_AggrAddTSReqParamsType*)pEventData->pEventData;
  wdiAggrAddTSRspCb   = (WDI_AggrAddTsRspCb)pEventData->pCBfnc;
  /*-------------------------------------------------------------------------
    Check to see if we are in the middle of an association, if so queue, if
    not it means it is free to process request 
  -------------------------------------------------------------------------*/
  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made and identify WDI session
  ------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != WDI_STATableGetStaBSSIDAddr(pWDICtx, 
                                        pwdiAggrAddTSParams->wdiAggrTsInfo.usSTAIdx, 
                                        &macBSSID))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
             "This station does not exist in the WDI Station Table %d");
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }

  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, macBSSID, &pBSSSes); 
  if ( NULL == pBSSSes ) 
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist");

    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
 
  /*------------------------------------------------------------------------
    Check if this BSS is being currently processed or queued,
    if queued - queue the new request as well 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_TRUE == pBSSSes->bAssocReqQueued )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS exists but currently queued");

    wdiStatus = WDI_QueueAssocRequest( pWDICtx, pBSSSes, pEventData); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return wdiStatus; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);
  /*-----------------------------------------------------------------------
    Get message buffer
    ! TO DO : proper conversion into the HAL Message Request Format 
  -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx, WDI_AGGR_ADD_TS_REQ, 
                        sizeof(tAggrAddTsParams),
                        &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < (usDataOffset + sizeof(tAggrAddTsParams) )))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Unable to get send buffer in set bss key req %x %x %x",
                pEventData, pwdiAggrAddTSParams, wdiAggrAddTSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  halAggrAddTsReq.aggrAddTsParam.staIdx = 
     pwdiAggrAddTSParams->wdiAggrTsInfo.usSTAIdx;
  halAggrAddTsReq.aggrAddTsParam.tspecIdx = 
     pwdiAggrAddTSParams->wdiAggrTsInfo.ucTspecIdx;

  for( i = 0; i < WLAN_HAL_MAX_AC; i++ )
  {
     halAggrAddTsReq.aggrAddTsParam.tspec[i].type = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].ucType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].length = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].ucLength;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.ackPolicy = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        ackPolicy;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.accessPolicy = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        accessPolicy;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.userPrio = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        userPrio;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.psb = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        psb;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.aggregation = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        aggregation;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.direction = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        direction;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.tsid = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        trafficType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.traffic.tsid = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiTraffic.
        trafficType;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.schedule.rsvd = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiSchedule.rsvd;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].tsinfo.schedule.schedule = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].wdiTSinfo.wdiSchedule.schedule;
        
                  
     halAggrAddTsReq.aggrAddTsParam.tspec[i].nomMsduSz = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usNomMsduSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxMsduSz = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usMaxMsduSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minSvcInterval = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinSvcInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxSvcInterval = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMaxSvcInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].inactInterval = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uInactInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].suspendInterval = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uSuspendInterval;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].svcStartTime = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uSvcStartTime;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minDataRate = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].meanDataRate = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMeanDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].peakDataRate = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uPeakDataRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].maxBurstSz = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMaxBurstSz;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].delayBound = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uDelayBound;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].minPhyRate = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].uMinPhyRate;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].surplusBw = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usSurplusBw;
     halAggrAddTsReq.aggrAddTsParam.tspec[i].mediumTime = 
        pwdiAggrAddTSParams->wdiAggrTsInfo.wdiTspecIE[i].usMediumTime;
  }

  wpalMemoryCopy( pSendBuffer+usDataOffset, 
                  &halAggrAddTsReq, 
                  sizeof(halAggrAddTsReq)); 

  pWDICtx->wdiReqStatusCB     = pwdiAggrAddTSParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiAggrAddTSParams->pUserData; 

  /*-------------------------------------------------------------------------
    Send Add TS Request to HAL 
  -------------------------------------------------------------------------*/
  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                       wdiAggrAddTSRspCb, pEventData->pUserData,
                       WDI_AGGR_ADD_TS_RESP); 
}/*WDI_ProcessAggrAddTSpecReq*/
#endif /* WLAN_FEATURE_VOWIFI_11R */

/*========================================================================
          Main DAL Control Path Response Processing API 
========================================================================*/

/**
 @brief Process Start Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartRspParamsType   wdiRspParams;
  WDI_StartRspCb           wdiStartRspCb = NULL;

  tHalMacStartRspMsg       halStartRspMsg; 
  WDI_AddStaParams         wdiAddSTAParam = {0};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiStartRspCb = (WDI_StartRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == wdiStartRspCb ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Start Resp %x %x",
                pEventData,  wdiStartRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  if ( sizeof(halStartRspMsg) < pEventData->uEventDataSize )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid response length in Start Resp %x %x",
                pEventData->uEventDataSize);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }


  /*-------------------------------------------------------------------------
    Unpack HAL Response Message - the header was already extracted by the
    main Response Handling procedure 
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStartRspMsg.startRspParams , 
                   pEventData->pEventData, 
                   sizeof(halStartRspMsg.startRspParams));

  wdiRspParams.ucMaxBssids    = halStartRspMsg.startRspParams.ucMaxBssids; 
  wdiRspParams.ucMaxStations  = halStartRspMsg.startRspParams.ucMaxStations;
  wdiRspParams.wdiStatus      = WDI_HAL_2_WDI_STATUS(halStartRspMsg.startRspParams.status); 
  wpalMemoryCopy(wdiRspParams.macSelfSta,
                 halStartRspMsg.startRspParams.selfStaMac, WDI_MAC_ADDR_LEN);
  wdiRspParams.usSelfStaDpuId  = halStartRspMsg.startRspParams.selfStaDpuId;
  wdiRspParams.usSelfStaIdx  = halStartRspMsg.startRspParams.selfStaIdx;

  wpalMutexAcquire(&pWDICtx->wptMutex);
  if ( WDI_STATUS_SUCCESS == wdiRspParams.wdiStatus  )
  {
    pWDICtx->ucExpectedStateTransition =  WDI_STARTED_ST;

    /*Cache the start response for further use*/
    wpalMemoryCopy( &pWDICtx->wdiCachedStartRspParams ,
                  &wdiRspParams, 
                  sizeof(pWDICtx->wdiCachedStartRspParams));

  }
  else
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to start device");

    /*Set the expected state transition to stopped - because the start has
      failed*/
    pWDICtx->ucExpectedStateTransition =  WDI_STOPPED_ST;

    wpalMutexRelease(&pWDICtx->wptMutex);

     /*Notify UMAC*/
    wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);
    
    WDI_DetectedDeviceError(pWDICtx, wdiRspParams.wdiStatus);

    /*Although the response is an error - it was processed by our function
    so as far as the caller is concerned this is a succesful reponse processing*/
    return WDI_STATUS_SUCCESS;
  }
  
  wpalMutexRelease(&pWDICtx->wptMutex);

  if(eDRIVER_TYPE_MFG == pWDICtx->driverMode)
  {
    /* FTM mode does not need to execute below */
    /* Notify UMAC */
    wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);
    return WDI_STATUS_SUCCESS;
  }

  /* START the Data transport */
  WDTS_startTransport(pWDICtx);

  /*Start the STA Table !- check this logic again*/
  WDI_STATableStart(pWDICtx);

  /* Store the Self STA Index */
  pWDICtx->ucSelfStaId = halStartRspMsg.startRspParams.selfStaIdx;

  pWDICtx->usSelfStaDpuId = wdiRspParams.usSelfStaDpuId;
  wpalMemoryCopy(pWDICtx->macSelfSta, wdiRspParams.macSelfSta,
                 WDI_MAC_ADDR_LEN);

  /* At this point add the self-STA */

  /*! TO DO: wdiAddSTAParam.bcastMgmtDpuSignature */
  /* !TO DO: wdiAddSTAParam.bcastDpuSignature */
  /*! TO DO: wdiAddSTAParam.dpuSig */
  /*! TO DO: wdiAddSTAParam.ucWmmEnabled */
  /*! TO DO: wdiAddSTAParam.ucHTCapable */
  /*! TO DO: wdiAddSTAParam.ucRmfEnabled */

  //all DPU indices are the same for self STA
  wdiAddSTAParam.bcastDpuIndex = wdiRspParams.usSelfStaDpuId;
  wdiAddSTAParam.bcastMgmtDpuIndex = wdiRspParams.usSelfStaDpuId;
  wdiAddSTAParam.dpuIndex = wdiRspParams.usSelfStaDpuId;;
  wpalMemoryCopy(wdiAddSTAParam.staMacAddr, wdiRspParams.macSelfSta,
                 WDI_MAC_ADDR_LEN);
  wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_SELF; /* 0 - self */
  wdiAddSTAParam.usStaIdx = halStartRspMsg.startRspParams.selfStaIdx;

  /* Note: Since we don't get an explicit config STA request for self STA, we
     add the self STA upon receiving the Start response message. But the
     self STA entry in the table is deleted when WDI gets an explicit delete STA
     request */
  (void)WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);

  /*Notify UMAC*/
  wdiStartRspCb( &wdiRspParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessStartRsp*/


/**
 @brief Process Stop Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStopRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status          wdiStatus;
  WDI_StopRspCb       wdiStopRspCb = NULL;

  tHalMacStopRspMsg   halMacStopRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiStopRspCb = (WDI_StopRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == wdiStopRspCb ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Stop Resp %x %x",
                pEventData,  wdiStopRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  if ( sizeof(halMacStopRspMsg) < pEventData->uEventDataSize )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid response length in Stop Resp %x %x",
                pEventData->uEventDataSize);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Unpack HAL Response Message - the header was already extracted by the
    main Response Handling procedure 
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halMacStopRspMsg.stopRspParams,  
                  pEventData->pEventData, 
                  sizeof(halMacStopRspMsg.stopRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halMacStopRspMsg.stopRspParams.status); 

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*--------------------------------------------------------------------------
    Check to see if the stop went OK 
  --------------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != wdiStatus  )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Failed to stop the device");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_FAILURE; 
  }
  
  pWDICtx->ucExpectedStateTransition = WDI_STOPPED_ST;
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*! TO DO: - STOP the Data transport */

  /*Notify UMAC*/
  wdiStopRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessStopRsp*/

/**
 @brief Process Close Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessCloseRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*There is no close response comming from HAL - function just kept for
  simmetry */
  WDI_ASSERT(0);
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessCloseRsp*/


/*============================================================================
                      SCAN RESPONSE PROCESSING API 
============================================================================*/

/**
 @brief Process Init Scan Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessInitScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  WDI_InitScanRspCb     wdiInitScanRspCb    = NULL;
  tHalInitScanRspMsg    halInitScanRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiInitScanRspCb = (WDI_InitScanRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Init Scan Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Unpack HAL Response Message - the header was already extracted by the
    main Response Handling procedure 
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halInitScanRspMsg.initScanRspParams, 
                  pEventData->pEventData, 
                  sizeof(halInitScanRspMsg.initScanRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halInitScanRspMsg.initScanRspParams.status); 

  /*Notify UMAC*/
  wdiInitScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessInitScanRsp*/


/**
 @brief Process Start Scan Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessStartScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_StartScanRspParamsType   wdiStartScanParams;
  WDI_StartScanRspCb           wdiStartScanRspCb   = NULL;
  
  tHalStartScanRspMsg          halStartScanRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiStartScanRspCb = (WDI_StartScanRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid parameters in Start Scan Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStartScanRspMsg.startScanRspParams, 
                  pEventData->pEventData, 
                  sizeof(halStartScanRspMsg.startScanRspParams));

  wdiStartScanParams.wdiStatus   =   WDI_HAL_2_WDI_STATUS(
                             halStartScanRspMsg.startScanRspParams.status);
#ifdef WLAN_FEATURE_VOWIFI
  wdiStartScanParams.ucTxMgmtPower = 
                             halStartScanRspMsg.startScanRspParams.txMgmtPower;
  wpalMemoryCopy( wdiStartScanParams.aStartTSF, 
                  halStartScanRspMsg.startScanRspParams.startTSF,
                  2);
#endif 

  if ( eHAL_STATUS_SUCCESS != halStartScanRspMsg.startScanRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Start Scan - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiStartScanRspCb( &wdiStartScanParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 

}/*WDI_ProcessStartScanRsp*/


/**
 @brief Process End Scan Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEndScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  tHalEndScanRspMsg     halEndScanRspMsg;
  WDI_EndScanRspCb      wdiEndScanRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiEndScanRspCb = (WDI_EndScanRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in End Scan Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halEndScanRspMsg.endScanRspParams, 
                  pEventData->pEventData, 
                  sizeof(halEndScanRspMsg.endScanRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halEndScanRspMsg.endScanRspParams.status); 

  if ( eHAL_STATUS_SUCCESS != halEndScanRspMsg.endScanRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in End Scan - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiEndScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessEndScanRsp*/


/**
 @brief Process Finish Scan Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFinishScanRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)  
{
  WDI_Status            wdiStatus;
  WDI_FinishScanRspCb   wdiFinishScanRspCb   = NULL;
  
  tHalFinishScanRspMsg  halFinishScanRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiFinishScanRspCb = (WDI_FinishScanRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Finish Scan Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( (void *)&halFinishScanRspMsg.finishScanRspParams.status, 
                  pEventData->pEventData, 
                  sizeof(halFinishScanRspMsg.finishScanRspParams.status));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halFinishScanRspMsg.finishScanRspParams.status); 

  if ( eHAL_STATUS_SUCCESS != halFinishScanRspMsg.finishScanRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Finish Scan - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiFinishScanRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessFinishScanRsp*/

/**
 @brief Process Join Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessJoinRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status                    wdiStatus;
  WDI_JoinRspCb                 wdiJoinRspCb        = NULL;
  WDI_BSSSessionType*           pBSSSes             = NULL;
  
  tHalJoinRspMsg                halJoinRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiJoinRspCb = (WDI_JoinRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData) ||
      ( NULL == wdiJoinRspCb ))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Post Assoc req %x %x",
                pEventData,  wdiJoinRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halJoinRspMsg.joinRspParams, 
                  pEventData->pEventData, 
                  sizeof(halJoinRspMsg.joinRspParams));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halJoinRspMsg.joinRspParams.status); 

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*-----------------------------------------------------------------------
    Join response can only be received for an existing assoc that
    is current and in progress 
    -----------------------------------------------------------------------*/
  if (( !WDI_VALID_SESSION_IDX(pWDICtx->ucCurrentBSSSesIdx )) || 
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist or "
              "association no longer in progress - misterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  pBSSSes = &pWDICtx->aBSSSessions[pWDICtx->ucCurrentBSSSesIdx];

  /*-----------------------------------------------------------------------
    Join Response is only allowed in init state 
  -----------------------------------------------------------------------*/
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Join only allowed in Joining state - failure state is %d "
              "strange HAL response", pBSSSes->wdiAssocState);

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }


  /*-----------------------------------------------------------------------
    If assoc has failed the current session will be deleted 
  -----------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != wdiStatus )
  {
    /*Association was failed by HAL - remove session*/
    WDI_DeleteSession(pWDICtx, pBSSSes);

    /*Association no longer in progress  */
    pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
  
  }
  else
  {
    /*Transition to state Joining - this may be redundant as we are supposed
      to be in this state already - but just to be safe*/
    pBSSSes->wdiAssocState = WDI_ASSOC_JOINING_ST; 
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Notify UMAC*/
  wdiJoinRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessJoinRsp*/


/**
 @brief Process Config BSS Response function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigBSSRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigBSSRspParamsType    wdiConfigBSSParams;
  WDI_ConfigBSSRspCb            wdiConfigBSSRspCb   = NULL;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*           pBSSSes             = NULL;

  tConfigBssRspMsg              halConfigBssRspMsg; 
  WDI_AddStaParams              wdiBcastAddSTAParam = {0};
  WDI_AddStaParams              wdiAddSTAParam = {0};
  
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiConfigBSSRspCb = (WDI_ConfigBSSRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Post Assoc req %x %x ",
                pEventData, wdiConfigBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC 
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halConfigBssRspMsg.configBssRspParams, 
                   pEventData->pEventData, 
                   sizeof(halConfigBssRspMsg.configBssRspParams));

  wdiConfigBSSParams.wdiStatus = WDI_HAL_2_WDI_STATUS(
                            halConfigBssRspMsg.configBssRspParams.status);
  wpalMemoryCopy( wdiConfigBSSParams.macBSSID, 
                  pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.macBSSID,
                  WDI_MAC_ADDR_LEN);

  wdiConfigBSSParams.usBSSIdx = halConfigBssRspMsg.configBssRspParams.bssIdx;

  wdiConfigBSSParams.ucBcastSig = 
     halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;

  wdiConfigBSSParams.ucUcastSig = 
     halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;

  wdiConfigBSSParams.usSTAIdx = halConfigBssRspMsg.configBssRspParams.bssStaIdx;

#ifdef WLAN_FEATURE_VOWIFI
  wdiConfigBSSParams.ucTxMgmtPower = 
                             halConfigBssRspMsg.configBssRspParams.txMgmtPower;
#endif
   wpalMemoryCopy( wdiConfigBSSParams.macSTA,
                   halConfigBssRspMsg.configBssRspParams.staMac,
                   WDI_MAC_ADDR_LEN );

  wpalMutexAcquire(&pWDICtx->wptMutex);
  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                                             wdiConfigBSSParams.macBSSID, 
                                            &pBSSSes); 

  /*-----------------------------------------------------------------------
    Config BSS response can only be received for an existing assoc that
    is current and in progress 
    -----------------------------------------------------------------------*/
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist "
              "- misterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*Save data for this BSS*/
  pBSSSes->usBSSIdx = halConfigBssRspMsg.configBssRspParams.bssIdx;
  pBSSSes->bcastDpuIndex     = 
    halConfigBssRspMsg.configBssRspParams.bcastDpuDescIndx;
  pBSSSes->bcastDpuSignature = 
    halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;
  pBSSSes->bcastMgmtDpuIndex = 
    halConfigBssRspMsg.configBssRspParams.mgmtDpuDescIndx;
  pBSSSes->bcastMgmtDpuSignature = 
    halConfigBssRspMsg.configBssRspParams.mgmtDpuSignature;
  pBSSSes->ucRmfEnabled      = 
    pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.ucRMFEnabled;
  pBSSSes->bcastStaIdx =
     halConfigBssRspMsg.configBssRspParams.bssBcastStaIdx;

/* Commenting the following code as explicit ADD STA will come with convergence 
 * code */
#if 0 
  /* Store index into the pBSSSes array, this is useful for look-up later
     wdiAddSTAParam.ucbssIndex = ucCurrentBSSSesIdx */

  /* Update the BSSID of self STA */
  if(WDI_STATableSetBSSID( pWDICtx, 
                           halConfigBssRspMsg.configBssRspParams.bssSelfStaIdx,
                           wdiConfigBSSParams.macBSSID ) != WDI_STATUS_SUCCESS)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Could not update the Self STA BSSID!");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /* Update the BSS Index of self STA */
  if(WDI_STATableSetBSSIdx( pWDICtx, 
                            halConfigBssRspMsg.configBssRspParams.bssSelfStaIdx,
                            ucCurrentBSSSesIdx ) != WDI_STATUS_SUCCESS)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Could not update the Self STA BSS Index!");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }
#endif
  /* !TO DO: Shuould we be updating the RMF Capability of self STA here? */

  /* Add the Broadcast STA to the STA table */
  /* !TO DO: Need Bcast STA params from HAL to populate wdiBcastAddSTAParam */
  WDI_AddBcastSTAtoSTATable( pWDICtx, &wdiBcastAddSTAParam,
                            halConfigBssRspMsg.configBssRspParams
                            .bssBcastStaIdx );

  /* Add Peer STA */
      
  wdiAddSTAParam.usStaIdx     = halConfigBssRspMsg.configBssRspParams.bssStaIdx; 

  wdiAddSTAParam.dpuIndex = halConfigBssRspMsg.configBssRspParams.dpuDescIndx;
  wdiAddSTAParam.dpuSig = halConfigBssRspMsg.configBssRspParams.ucastDpuSignature;
   
   /*This info can be retrieved from the cached initial request*/
   wdiAddSTAParam.ucWmmEnabled = 
      pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.ucWMMEnabled;
   wdiAddSTAParam.ucHTCapable  = 
      pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.ucHTCapable; 
   wdiAddSTAParam.ucStaType    = 
      pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.wdiSTAType;  

   /* MAC Address of STA */
   wpalMemoryCopy(wdiAddSTAParam.staMacAddr, 
                halConfigBssRspMsg.configBssRspParams.staMac, 
                WDI_MAC_ADDR_LEN);

   wpalMemoryCopy(wdiAddSTAParam.macBSSID, 
                pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.wdiSTAContext.macBSSID , 
                WDI_MAC_ADDR_LEN); 
   
   /*Add BSS specific parameters*/
   wdiAddSTAParam.bcastMgmtDpuIndex     = 
      halConfigBssRspMsg.configBssRspParams.mgmtDpuDescIndx;
   wdiAddSTAParam.bcastMgmtDpuSignature = 
      halConfigBssRspMsg.configBssRspParams.mgmtDpuSignature;
   wdiAddSTAParam.bcastDpuIndex         = 
      halConfigBssRspMsg.configBssRspParams.bcastDpuDescIndx;
   wdiAddSTAParam.bcastDpuSignature     = 
      halConfigBssRspMsg.configBssRspParams.bcastDpuSignature;
   wdiAddSTAParam.ucRmfEnabled          =  
      pWDICtx->wdiCachedConfigBssReq.wdiReqInfo.ucRMFEnabled;
   wdiAddSTAParam.ucbssIndex = ucCurrentBSSSesIdx;

  WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);


  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Notify UMAC*/
  wdiConfigBSSRspCb( &wdiConfigBSSParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessConfigBSSRsp*/


/**
 @brief Process Del BSS Response function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBSSRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelBSSRspParamsType       wdiDelBSSParams;
  WDI_DelBSSRspCb               wdiDelBSSRspCb      = NULL;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0; 
  WDI_BSSSessionType*           pBSSSes             = NULL;

  tDeleteBssRspMsg              halDelBssRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiDelBSSRspCb = (WDI_DelBSSRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Delete BSS response %x %x ",
                pEventData, wdiDelBSSRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC 
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halDelBssRspMsg.deleteBssRspParams, 
                  pEventData->pEventData, 
                  sizeof(halDelBssRspMsg.deleteBssRspParams));


  wdiDelBSSParams.wdiStatus   =   WDI_HAL_2_WDI_STATUS(
                                 halDelBssRspMsg.deleteBssRspParams.status); 

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSessionByBSSIdx( pWDICtx, 
                             halDelBssRspMsg.deleteBssRspParams.bssIdx, 
                             &pBSSSes); 

  /*-----------------------------------------------------------------------
    Del BSS response can only be received for an existing assoc that
    is current and in progress 
    -----------------------------------------------------------------------*/
  if ( NULL == pBSSSes )
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist or "
              "association no longer in progress - misterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*Extract BSSID for the response to UMAC*/
  wpalMemoryCopy(wdiDelBSSParams.macBSSID, 
                 pBSSSes->macBSSID, WDI_MAC_ADDR_LEN);

  /*-----------------------------------------------------------------------
    The current session will be deleted 
  -----------------------------------------------------------------------*/
  WDI_DeleteSession(pWDICtx, pBSSSes);


  /*-----------------------------------------------------------------------
    Check to see if this association is in progress - if so disable the
    flag as this has ended
  -----------------------------------------------------------------------*/
  if ( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx )
  {  
   /*Association no longer in progress  */
    pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
  }

  /* Delete the BCAST STA entry from the STA table */
  (void)WDI_STATableDelSta( pWDICtx, pBSSSes->bcastStaIdx );

  /* Delete the STA's in this BSS */
  WDI_STATableBSSDelSta(pWDICtx, halDelBssRspMsg.deleteBssRspParams.bssIdx);
  
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Notify UMAC*/
  wdiDelBSSRspCb( &wdiDelBSSParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessDelBSSRsp*/

/**
 @brief Process Post Assoc Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessPostAssocRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_PostAssocRspParamsType    wdiPostAssocParams;
  WDI_PostAssocRspCb            wdiPostAssocRspCb      = NULL;
  wpt_uint8                     ucCurrentBSSSesIdx     = 0; 
  WDI_BSSSessionType*           pBSSSes                = NULL;
  tPostAssocRspMsg              halPostAssocRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiPostAssocRspCb = (WDI_PostAssocRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Post Assoc req %x %x ",
                pEventData, wdiPostAssocRspCb);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halPostAssocRspMsg.postAssocRspParams, 
                   pEventData->pEventData, 
                   sizeof(halPostAssocRspMsg.postAssocRspParams));

  /*Extract the Post Assoc STA Params */

  wdiPostAssocParams.staParams.usSTAId   = 
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.staIdx;
  wdiPostAssocParams.staParams.ucUcastSig = 
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucUcastSig;
  wdiPostAssocParams.staParams.ucBcastSig = 
    halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucBcastSig;

 wdiPostAssocParams.wdiStatus = 
    WDI_HAL_2_WDI_STATUS(halPostAssocRspMsg.postAssocRspParams.configStaRspParams.status); 

 /*Copy the MAC addresses from the cached storage in the WDI CB as they are not
   included in the response */
  wpalMemoryCopy( wdiPostAssocParams.staParams.macSTA, 
                  pWDICtx->wdiCachedPostAssocReq.wdiSTAParams.macSTA, 
                  WDI_MAC_ADDR_LEN);

  /* Extract Post Assoc BSS Params */

  wpalMemoryCopy( wdiPostAssocParams.bssParams.macBSSID, 
                  pWDICtx->wdiCachedPostAssocReq.wdiBSSParams.macBSSID, 
                  WDI_MAC_ADDR_LEN); 

  /*Copy the MAC addresses from the cached storage in the WDI CB as they are not
   included in the response */
  wpalMemoryCopy( wdiPostAssocParams.bssParams.macSTA, 
                  pWDICtx->wdiCachedPostAssocReq.wdiBSSParams.wdiSTAContext
                  .macSTA, WDI_MAC_ADDR_LEN);

  wdiPostAssocParams.bssParams.ucBcastSig = 
     halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucBcastSig;

  wdiPostAssocParams.bssParams.ucUcastSig = 
     halPostAssocRspMsg.postAssocRspParams.configStaRspParams.ucUcastSig;

  wdiPostAssocParams.bssParams.usBSSIdx =
     halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssIdx;

  wdiPostAssocParams.bssParams.usSTAIdx = 
     halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssStaIdx;

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Find the BSS for which the request is made 
  ------------------------------------------------------------------------*/
  ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                                             wdiPostAssocParams.bssParams.
                                             macBSSID, &pBSSSes); 

  /*-----------------------------------------------------------------------
    Post assoc response can only be received for an existing assoc that
    is current and in progress 
    -----------------------------------------------------------------------*/
  if (( NULL == pBSSSes ) ||
      ( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx ) || 
      ( eWLAN_PAL_FALSE == pWDICtx->bAssociationInProgress ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Association sequence for this BSS does not yet exist or "
              "association no longer in progress - misterious HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*-----------------------------------------------------------------------
    Post Assoc Request is only allowed in Joining state 
  -----------------------------------------------------------------------*/
  if ( WDI_ASSOC_JOINING_ST != pBSSSes->wdiAssocState)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Post Assoc not allowed before JOIN - failing request "
              "strange HAL response");

    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
    
    wpalMutexRelease(&pWDICtx->wptMutex);
    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  /*-----------------------------------------------------------------------
    If assoc has failed the current session will be deleted 
  -----------------------------------------------------------------------*/
  if ( WDI_STATUS_SUCCESS != wdiPostAssocParams.wdiStatus )
  {
    /*Association was failed by HAL - remove session*/
    WDI_DeleteSession(pWDICtx, pBSSSes);
  }
  else
  {
    /*Transition to state POST Assoc*/
    pBSSSes->wdiAssocState = WDI_ASSOC_POST_ST; 

    /*Save DPU Info*/
    pBSSSes->bcastMgmtDpuIndex     = 
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.mgmtDpuDescIndx;
    pBSSSes->bcastMgmtDpuSignature = 
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.mgmtDpuSignature;
    pBSSSes->bcastDpuIndex         = 
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bcastDpuDescIndx;
    pBSSSes->bcastDpuSignature     = 
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bcastDpuSignature;

    pBSSSes->usBSSIdx              = 
      halPostAssocRspMsg.postAssocRspParams.configBssRspParams.bssIdx;
  }

  /*Association no longer in progress  */
  pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
  wpalMutexRelease(&pWDICtx->wptMutex);

  /*Notify UMAC*/
  wdiPostAssocRspCb( &wdiPostAssocParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessPostAssocRsp*/

/**
 @brief Process Del STA Rsp function (called when a response is 
        being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelSTARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_DelSTARspParamsType   wdiDelSTARsp;
  WDI_DelSTARspCb           wdiDelSTARspCb   = NULL;

  tDeleteStaRspMsg          halDelStaRspMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiDelSTARspCb = (WDI_DelSTARspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Del STA Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halDelStaRspMsg.delStaRspParams,
                  pEventData->pEventData, 
                  sizeof(halDelStaRspMsg.delStaRspParams));

  wdiDelSTARsp.usSTAIdx    = halDelStaRspMsg.delStaRspParams.staId;
  wdiDelSTARsp.wdiStatus   =   
    WDI_HAL_2_WDI_STATUS(halDelStaRspMsg.delStaRspParams.status); 

  /*Delete the station in the table*/
  WDI_STATableDelSta( pWDICtx, wdiDelSTARsp.usSTAIdx);

  /* If the DEL STA request is for self STA add the entry back */
  if(pWDICtx->ucSelfStaId == wdiDelSTARsp.usSTAIdx)
  {
    WDI_AddStaParams         wdiAddSTAParam = {0};

    /* At this point add the self-STA */

    /*! TO DO: wdiAddSTAParam.bcastMgmtDpuSignature */
    /* !TO DO: wdiAddSTAParam.bcastDpuSignature */
    /*! TO DO: wdiAddSTAParam.dpuSig */
    /*! TO DO: wdiAddSTAParam.ucWmmEnabled */
    /*! TO DO: wdiAddSTAParam.ucHTCapable */
    /*! TO DO: wdiAddSTAParam.ucRmfEnabled */
  
    //all DPU indices are the same for self STA
    wdiAddSTAParam.bcastDpuIndex = pWDICtx->usSelfStaDpuId;
    wdiAddSTAParam.bcastMgmtDpuIndex = pWDICtx->usSelfStaDpuId;
    wdiAddSTAParam.dpuIndex = pWDICtx->usSelfStaDpuId;;
    wpalMemoryCopy(wdiAddSTAParam.staMacAddr, pWDICtx->macSelfSta,
                   WDI_MAC_ADDR_LEN);
    wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_SELF; /* 0 - self */
    wdiAddSTAParam.usStaIdx = pWDICtx->ucSelfStaId;
  
    WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);

  }

  /*Notify UMAC*/
  wdiDelSTARspCb( &wdiDelSTARsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessDelSTARsp*/


/*==========================================================================
                   Security Response Processing Functions 
==========================================================================*/

/**
 @brief Process Set BSS Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBssKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetBSSKeyRspCb    wdiSetBSSKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSetBSSKeyRspCb = (WDI_SetBSSKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set BSS Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Set BSS Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiSetBSSKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetBssKeyRsp*/

/**
 @brief Process Remove BSS Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveBssKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveBSSKeyRspCb   wdiRemoveBSSKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiRemoveBSSKeyRspCb = (WDI_RemoveBSSKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Remove BSS Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Remove BSS Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiRemoveBSSKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetBssKeyRsp*/


/**
 @brief Process Set STA Key Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetSTAKeyRspCb    wdiSetSTAKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSetSTAKeyRspCb = (WDI_SetSTAKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set STA Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Set STA Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiSetSTAKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetSTAKeyRsp*/

/**
 @brief Process Remove STA Key Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveSTAKeyRspCb   wdiRemoveSTAKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiRemoveSTAKeyRspCb = (WDI_RemoveSTAKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Remove STA Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Remove STA Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiRemoveSTAKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessRemoveStaKeyRsp*/

/**
 @brief Process Set STA Bcast Key Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetStaBcastKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_SetSTAKeyRspCb    wdiSetSTABcastKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSetSTABcastKeyRspCb = (WDI_SetSTAKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set STA Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Set STA Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiSetSTABcastKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetSTABcastKeyRsp*/

/**
 @brief Process Remove STA Bcast Key Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemoveStaBcastKeyRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_RemoveSTAKeyRspCb   wdiRemoveSTABcastKeyRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiRemoveSTABcastKeyRspCb = (WDI_RemoveSTAKeyRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Remove STA Key Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS != halStatus )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Remove STA Key - considered catastrophic");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  /*Notify UMAC*/
  wdiRemoveSTABcastKeyRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessRemoveStaBcastKeyRsp*/


/*==========================================================================
                   QoS and BA Response Processing Functions 
==========================================================================*/

/**
 @brief Process Add TSpec Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddTSpecRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_AddTsRspCb   wdiAddTsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiAddTsRspCb = (WDI_AddTsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Add TS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiAddTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessAddTSpecRsp*/


/**
 @brief Process Del TSpec Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelTSpecRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_DelTsRspCb   wdiDelTsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiDelTsRspCb = (WDI_DelTsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Del TS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiDelTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessDelTSpecRsp*/

/**
 @brief Process Update EDCA Parameters Rsp function (called when a  
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateEDCAParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateEDCAParamsRspCb   wdiUpdateEDCAParamsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiUpdateEDCAParamsRspCb = (WDI_UpdateEDCAParamsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Update EDCA Params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiUpdateEDCAParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUpdateEDCAParamsRsp*/


/**
 @brief Process Add BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBASessionRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBASessionRspCb   wdiAddBASessionRspCb   = NULL;

  tAddBASessionRspParams        halBASessionRsp;
  WDI_AddBASessionRspParamsType wdiBASessionRsp;

  
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiAddBASessionRspCb = (WDI_AddBASessionRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Add BA Session Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halBASessionRsp, 
                  pEventData->pEventData, 
                  sizeof(halBASessionRsp));

  wdiBASessionRsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halBASessionRsp.status);

  if ( eHAL_STATUS_SUCCESS == wdiBASessionRsp.wdiStatus )
  {
    wdiBASessionRsp.ucBaDialogToken = halBASessionRsp.baDialogToken;
    wdiBASessionRsp.ucBaTID = halBASessionRsp.baTID;
    wdiBASessionRsp.ucBaBufferSize = halBASessionRsp.baBufferSize;
    wdiBASessionRsp.usBaSessionID = halBASessionRsp.baSessionID;
    wdiBASessionRsp.ucWinSize = halBASessionRsp.winSize;
    wdiBASessionRsp.usSTAIdx = halBASessionRsp.STAID;
    wdiBASessionRsp.usBaSSN = halBASessionRsp.SSN;
  }

  /*Notify UMAC*/
  wdiAddBASessionRspCb( &wdiBASessionRsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessAddSessionBARsp*/


/**
 @brief Process Del BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelBARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_DelBARspCb   wdiDelBARspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiDelBARspCb = (WDI_DelBARspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Del BA Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  if ( eHAL_STATUS_SUCCESS == halStatus )
  {
    /*! TO DO: I should notify the DAL Data Path that the BA was deleted*/
  }

  /*Notify UMAC*/
  wdiDelBARspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessDelBARsp*/

/**
 @brief Process Flush AC Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFlushAcRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_FlushAcRspCb wdiFlushAcRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiFlushAcRspCb = (WDI_FlushAcRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Flush AC Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiFlushAcRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessFlushAcRsp*/

/**
 @brief Process BT AMP event Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessBtAmpEventRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status       wdiStatus;
   eHalStatus       halStatus;
   WDI_BtAmpEventRspCb wdiBtAmpEventRspCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiBtAmpEventRspCb = (WDI_BtAmpEventRspCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in BT AMP event Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   wpalMemoryCopy( &halStatus, 
                   pEventData->pEventData, 
                   sizeof(halStatus));

   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiBtAmpEventRspCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessBtAmpEventRsp*/

/*===========================================================================
           Miscellaneous Control Response Processing API 
===========================================================================*/

/**
 @brief Process Channel Switch Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessChannelSwitchRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_SwitchCHRspParamsType   wdiSwitchChRsp;
  WDI_SwitchChRspCb           wdiChSwitchRspCb   = NULL;
  tSwitchChannelRspParams     halSwitchChannelRsp;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiChSwitchRspCb = (WDI_SwitchChRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Del BA Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halSwitchChannelRsp, 
                  (wpt_uint8*)pEventData->pEventData,
                  sizeof(halSwitchChannelRsp));

  wdiSwitchChRsp.wdiStatus   =  
               WDI_HAL_2_WDI_STATUS(halSwitchChannelRsp.status); 
  wdiSwitchChRsp.ucChannel = halSwitchChannelRsp.channelNumber;

#ifdef WLAN_FEATURE_VOWIFI
  wdiSwitchChRsp.ucTxMgmtPower =  halSwitchChannelRsp.txMgmtPower; 
#endif

  /*Notify UMAC*/
  wdiChSwitchRspCb( &wdiSwitchChRsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessChannelSwitchRsp*/


/**
 @brief Process Config STA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigStaRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_ConfigSTARspParamsType    wdiCfgSTAParams;
  WDI_ConfigSTARspCb            wdiConfigSTARspCb   = NULL;
  WDI_AddStaParams              wdiAddSTAParam;

  WDI_BSSSessionType*           pBSSSes             = NULL;
  wpt_uint8                     ucCurrentBSSSesIdx  = 0; 

  tConfigStaRspMsg              halConfigStaRsp; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiConfigSTARspCb = (WDI_ConfigSTARspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Config STA Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halConfigStaRsp.configStaRspParams, 
                  pEventData->pEventData, 
                  sizeof(halConfigStaRsp.configStaRspParams));


  wdiCfgSTAParams.usSTAId     = halConfigStaRsp.configStaRspParams.staIdx;
  wdiCfgSTAParams.ucUcastSig  = halConfigStaRsp.configStaRspParams.ucUcastSig;
  wdiCfgSTAParams.ucBcastSig  = halConfigStaRsp.configStaRspParams.ucBcastSig;

   /* MAC Address of STA - take from cache as it does not come back in the
   response*/
   wpalMemoryCopy( wdiCfgSTAParams.macSTA,
          pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macSTA, 
          WDI_MAC_ADDR_LEN);
  
  wdiCfgSTAParams.wdiStatus   =   
    WDI_HAL_2_WDI_STATUS(halConfigStaRsp.configStaRspParams.status); 

  wdiCfgSTAParams.ucDpuIndex = halConfigStaRsp.configStaRspParams.dpuIndex;
  wdiCfgSTAParams.ucBcastDpuIndex = halConfigStaRsp.configStaRspParams.bcastDpuIndex;
  wdiCfgSTAParams.ucBcastMgmtDpuIdx = halConfigStaRsp.configStaRspParams.bcastMgmtDpuIdx;

  if ( WDI_STATUS_SUCCESS == wdiCfgSTAParams.wdiStatus )
  {
    /*Only add sta to the table if this was an add operation
    ! - check this logic again - anything special that needs to be done for
    update*/
    if ( WDI_ADD_STA == pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.wdiAction )
    {
      /* ADD STA to table */
      wdiAddSTAParam.usStaIdx     = wdiCfgSTAParams.usSTAId; 
      wdiAddSTAParam.dpuSig       = wdiCfgSTAParams.ucUcastSig;
      wdiAddSTAParam.dpuIndex     = wdiCfgSTAParams.ucDpuIndex;
        
      /*This info can be retrieved from the cached initial request*/
      wdiAddSTAParam.ucWmmEnabled = 
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.ucWMMEnabled;
      wdiAddSTAParam.ucHTCapable  = 
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.ucHTCapable; 
      wdiAddSTAParam.ucStaType    = 
        pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.wdiSTAType;  
   
      /* MAC Address of STA */
      wpalMemoryCopy(wdiAddSTAParam.staMacAddr, 
                     pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macSTA, 
                     WDI_MAC_ADDR_LEN);
  
      wpalMemoryCopy(wdiAddSTAParam.macBSSID, 
                     pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macBSSID , 
                     WDI_MAC_ADDR_LEN); 
  
      ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                    pWDICtx->wdiCachedConfigStaReq.wdiReqInfo.macBSSID, 
                    &pBSSSes);  

      if ( NULL == pBSSSes )
      {
        WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
                  "Association for this BSSID is not in place");
    
        WDI_ASSERT(0);
        return WDI_STATUS_E_NOT_ALLOWED; 
      }
      /*Add BSS specific parameters*/
      wdiAddSTAParam.bcastMgmtDpuIndex     = pBSSSes->bcastMgmtDpuIndex;
      wdiAddSTAParam.bcastMgmtDpuSignature = pBSSSes->bcastMgmtDpuSignature;
      wdiAddSTAParam.bcastDpuIndex         = pBSSSes->bcastDpuIndex;
      wdiAddSTAParam.bcastDpuSignature     = pBSSSes->bcastDpuSignature;
      wdiAddSTAParam.ucRmfEnabled          = pBSSSes->ucRmfEnabled;
      wdiAddSTAParam.ucbssIndex            = ucCurrentBSSSesIdx;
      
      WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
    }
  }

  /*Notify UMAC*/
  wdiConfigSTARspCb( &wdiCfgSTAParams, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessConfigStaRsp*/


/**
 @brief Process Set Link State Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetLinkStateRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status              wdiStatus;
  eHalStatus              halStatus;
  WDI_SetLinkStateRspCb   wdiSetLinkStateRspCb = NULL;

  WDI_BSSSessionType*     pBSSSes              = NULL;
  wpt_uint8               ucCurrentBSSSesIdx   = 0; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSetLinkStateRspCb = (WDI_SetLinkStateRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set Link State Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  wpalMutexAcquire(&pWDICtx->wptMutex);

  /*If the link is being transitioned to idle - the BSS is to be deleted
  - this type of ending a session is possible when UMAC has failed an
  - association session during Join*/
  if ( WDI_LINK_IDLE_STATE == 
       pWDICtx->wdiCacheSetLinkStReq.wdiLinkInfo.wdiLinkState )
  {
    /*------------------------------------------------------------------------
      Find the BSS for which the request is made 
    ------------------------------------------------------------------------*/
    ucCurrentBSSSesIdx = WDI_FindAssocSession( pWDICtx, 
                        pWDICtx->wdiCacheSetLinkStReq.wdiLinkInfo.macBSSID, 
                        &pBSSSes); 
  
    /*-----------------------------------------------------------------------
      Del BSS response can only be received for an existing assoc that
      is current and in progress 
      -----------------------------------------------------------------------*/
    if ( NULL == pBSSSes )
    {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
                "Set link response received outside association session");
    }
    else
    {
      /*-----------------------------------------------------------------------
        The current session will be deleted 
      -----------------------------------------------------------------------*/
      WDI_DeleteSession(pWDICtx, pBSSSes);

      /*-----------------------------------------------------------------------
        Check to see if this association is in progress - if so disable the
        flag as this has ended
      -----------------------------------------------------------------------*/
      if ( ucCurrentBSSSesIdx != pWDICtx->ucCurrentBSSSesIdx )
      {  
       /*Association no longer in progress  */
        pWDICtx->bAssociationInProgress = eWLAN_PAL_FALSE;
      }
    }
  }

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiSetLinkStateRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetLinkStateRsp*/

/**
 @brief Process Get Stats Rsp function (called when a response is   
        being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessGetStatsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_GetStatsRspParamsType   *wdiGetStatsRsp;
  WDI_GetStatsRspCb           wdiGetStatsRspCb   = NULL;
  tHalStatsRspParams          halStatsRspParams;
  
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiGetStatsRspCb = (WDI_GetStatsRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Get Stats Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatsRspParams, 
                  (wpt_uint8*)pEventData->pEventData,
                  sizeof(halStatsRspParams));

  /*allocate the stats response buffer */
  wdiGetStatsRsp = (WDI_GetStatsRspParamsType *)wpalMemoryAllocate(
                                                   halStatsRspParams.msgLen);

  wdiGetStatsRsp->usMsgType  = halStatsRspParams.msgType;
  wdiGetStatsRsp->usMsgLen   = halStatsRspParams.msgLen;
  wdiGetStatsRsp->wdiStatus  = WDI_HAL_2_WDI_STATUS(halStatsRspParams.status);
  wdiGetStatsRsp->usSTAIdx   = halStatsRspParams.staId;
  wdiGetStatsRsp->uStatsMask = halStatsRspParams.statsMask;

  /* copy the stats buffer at the end of the tHalStatsRspParams message */
  wpalMemoryCopy(
         ((wpt_uint8 *)wdiGetStatsRsp) + sizeof(WDI_GetStatsRspParamsType),
         ((wpt_uint8 *)&halStatsRspParams) + sizeof(tHalStatsRspParams),
         halStatsRspParams.msgLen - sizeof(tHalStatsRspParams));

  /*Notify UMAC*/
  wdiGetStatsRspCb( wdiGetStatsRsp, pWDICtx->pRspCBUserData);

  wpalMemoryFree(wdiGetStatsRsp);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessGetStatsRsp*/

	
/**
 @brief Process Update Cfg Rsp function (called when a response is  
        being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateCfgRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_UpdateCfgRspCb   wdiUpdateCfgRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiUpdateCfgRspCb = (WDI_UpdateCfgRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Update Cfg Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiUpdateCfgRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUpdateCfgRsp*/



/**
 @brief Process Add BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAddBARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_AddBARspCb          wdiAddBARspCb   = NULL;

  tAddBARspParams         halAddBARsp;
  WDI_AddBARspinfoType    wdiAddBARsp;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiAddBARspCb = (WDI_AddBARspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Add BA Session Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halAddBARsp, 
                  pEventData->pEventData, 
                  sizeof(halAddBARsp));

  wdiAddBARsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halAddBARsp.status);

  if ( eHAL_STATUS_SUCCESS == wdiAddBARsp.wdiStatus )
  {
    wdiAddBARsp.ucBaDialogToken = halAddBARsp.baDialogToken;
  }

  /*Notify UMAC*/
  wdiAddBARspCb( &wdiAddBARsp, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessAddSessionBARsp*/

/**
 @brief Process Add BA Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessTriggerBARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_TriggerBARspCb      wdiTriggerBARspCb   = NULL;

  tTriggerBARspParams*           halTriggerBARsp;
  tTriggerBaRspCandidate*        halBaCandidate;
  WDI_TriggerBARspParamsType*    wdiTriggerBARsp;
  WDI_TriggerBARspCandidateType* wdiTriggerBARspCandidate;
  wpt_uint16                     index;
  wpt_uint16                     TidIndex;
  
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiTriggerBARspCb = (WDI_TriggerBARspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Trigger BA  Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halTriggerBARsp = (tTriggerBARspParams *)pEventData->pEventData;

  wdiTriggerBARsp = wpalMemoryAllocate(sizeof(WDI_TriggerBARspParamsType) + 
                      halTriggerBARsp->baCandidateCnt * 
                      sizeof(WDI_TriggerBARspCandidateType));
  if(NULL == wdiTriggerBARsp)
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
                "Failed to allocate memory in TRigger BA Response %x %x %x ",
                 pWDICtx, pEventData, pEventData->pEventData);
    wpalMemoryFree(halTriggerBARsp);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE; 
  }

  wdiTriggerBARsp->wdiStatus = WDI_HAL_2_WDI_STATUS(halTriggerBARsp->status);

  if ( WDI_STATUS_SUCCESS == wdiTriggerBARsp->wdiStatus)
  {
    wdiTriggerBARsp->usBaCandidateCnt = halTriggerBARsp->baCandidateCnt;
    wpalMemoryCopy(wdiTriggerBARsp->macBSSID, 
                                 halTriggerBARsp->bssId , WDI_MAC_ADDR_LEN);

    wdiTriggerBARspCandidate = (WDI_TriggerBARspCandidateType*)(wdiTriggerBARsp + 1);
    halBaCandidate = (tTriggerBaRspCandidate*)(halTriggerBARsp + 1);

    for(index = 0; index < wdiTriggerBARsp->usBaCandidateCnt; index++)
    {
      wpalMemoryCopy(wdiTriggerBARspCandidate->macSTA, 
                                  halBaCandidate->staAddr, WDI_MAC_ADDR_LEN);
      for(TidIndex = 0; TidIndex < STA_MAX_TC; TidIndex++)
      {
        wdiTriggerBARspCandidate->wdiBAInfo[TidIndex].fBaEnable = 
                            halBaCandidate->baInfo[TidIndex].fBaEnable;
        wdiTriggerBARspCandidate->wdiBAInfo[TidIndex].startingSeqNum = 
                            halBaCandidate->baInfo[TidIndex].startingSeqNum;
      }
      wdiTriggerBARspCandidate++;
      halBaCandidate++;
    }
  }

  /*Notify UMAC*/
  wdiTriggerBARspCb( wdiTriggerBARsp, pWDICtx->pRspCBUserData);

  wpalMemoryFree(wdiTriggerBARsp);
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessAddSessionBARsp*/

/**
 @brief Process Update Beacon Params Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateBeaconParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateBeaconParamsRspCb   wdiUpdateBeaconParamsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiUpdateBeaconParamsRspCb = (WDI_UpdateBeaconParamsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Update Beacon Params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiUpdateBeaconParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUpdateBeaconParamsRsp*/

/**
 @brief Process Send Beacon template Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSendBeaconParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_SendBeaconParamsRspCb   wdiSendBeaconParamsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSendBeaconParamsRspCb = (WDI_SendBeaconParamsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Send Beacon Params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiSendBeaconParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSendBeaconParamsRsp*/

  
/**
 @brief Process Update Probe Resp Template Rsp function (called 
        when a response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateProbeRspTemplateRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_UpdateProbeRspTemplateRspCb   wdiUpdProbeRspTemplRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiUpdProbeRspTemplRspCb = (WDI_UpdateProbeRspTemplateRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Send Beacon Params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiUpdProbeRspTemplRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUpdateProbeRspTemplateRsp*/

#ifdef WLAN_FEATURE_VOWIFI
  /**
 @brief Process Set Max Tx Power Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetMaxTxPowerRsp
( 
  WDI_ControlBlockType*          pWDICtx,
  WDI_EventInfoType*             pEventData
)
{
  tSetMaxTxPwrRspMsg             halTxpowerrsp;
  
  WDI_SetMaxTxPowerRspMsg        wdiSetMaxTxPowerRspMsg;
  
  WDA_SetMaxTxPowerRspCb         wdiReqStatusCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiReqStatusCb = (WDA_SetMaxTxPowerRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set Max Tx Power Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halTxpowerrsp.setMaxTxPwrRspParams, 
                           pEventData->pEventData, 
                           sizeof(halTxpowerrsp.setMaxTxPwrRspParams)); 

  if ( eHAL_STATUS_SUCCESS != halTxpowerrsp.setMaxTxPwrRspParams.status )
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Error status returned in Set Max Tx Power Response ");
     WDI_DetectedDeviceError( pWDICtx, WDI_ERR_BASIC_OP_FAILURE); 
     return WDI_STATUS_E_FAILURE; 
  }

  wdiSetMaxTxPowerRspMsg.wdiStatus = 
         WDI_HAL_2_WDI_STATUS(halTxpowerrsp.setMaxTxPwrRspParams.status);
  wdiSetMaxTxPowerRspMsg.ucPower  = halTxpowerrsp.setMaxTxPwrRspParams.power; 

  /*Notify UMAC*/
  wdiReqStatusCb( &wdiSetMaxTxPowerRspMsg, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}
#endif

#ifdef WLAN_FEATURE_P2P
/**
 @brief Process P2P Group Owner Notice Of Absense Rsp function (called 
        when a response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessP2PGONOARsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status       wdiStatus;
  eHalStatus       halStatus;
  WDI_SetP2PGONOAReqParamsRspCb   wdiP2PGONOAReqParamsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiP2PGONOAReqParamsRspCb = (WDI_SetP2PGONOAReqParamsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in P2P GO NOA Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halStatus, 
                  pEventData->pEventData, 
                  sizeof(halStatus));

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiP2PGONOAReqParamsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessP2PGONOARsp*/
#endif
/**
 @brief Process Enter IMPS Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterImpsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterImpsRspCb   wdiEnterImpsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiEnterImpsRspCb = (WDI_EnterImpsRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Enter IMPS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);

  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiEnterImpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessEnterImpsRsp*/

/**
 @brief Process Exit IMPS Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitImpsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitImpsRspCb    wdiExitImpsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiExitImpsRspCb = (WDI_ExitImpsRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Exit IMPS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiExitImpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessExitImpsRsp*/

/**
 @brief Process Enter BMPS Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterBmpsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterBmpsRspCb   wdiEnterBmpsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiEnterBmpsRspCb = (WDI_EnterBmpsRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Enter BMPS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiEnterBmpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessEnterBmpsRsp*/

/**
 @brief Process Exit BMPS Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitBmpsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitBmpsRspCb   wdiExitBmpsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiExitBmpsRspCb = (WDI_ExitBmpsRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Exit BMPS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  // notify DTS that we are entering Full power
  WDTS_SetPowerState(pWDICtx, WDTS_POWER_STATE_FULL, NULL);

  /*Notify UMAC*/
  wdiExitBmpsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessExitBmpsRsp*/

/**
 @brief Process Enter UAPSD Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessEnterUapsdRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_EnterUapsdRspCb   wdiEnterUapsdRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiEnterUapsdRspCb = (WDI_EnterUapsdRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Enter UAPSD Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiEnterUapsdRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessEnterUapsdRsp*/

/**
 @brief Process Exit UAPSD Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessExitUapsdRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ExitUapsdRspCb   wdiExitUapsdRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiExitUapsdRspCb = (WDI_ExitUapsdRspCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Exit UAPSD Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiExitUapsdRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessExitUapsdRsp*/

/**
 @brief Process set UAPSD params Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetUapsdAcParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_SetUapsdAcParamsCb   wdiSetUapsdAcParamsCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiSetUapsdAcParamsCb = (WDI_SetUapsdAcParamsCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Set UAPSD params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiSetUapsdAcParamsCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetUapsdAcParamsRsp*/

/**
 @brief Process update UAPSD params Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUpdateUapsdParamsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_UpdateUapsdParamsCb   wdiUpdateUapsdParamsCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiUpdateUapsdParamsCb = (WDI_UpdateUapsdParamsCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Update UAPSD params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiUpdateUapsdParamsCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUpdateUapsdParamsRsp*/

/**
 @brief Process Configure RXP filter Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigureRxpFilterRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_ConfigureRxpFilterCb   wdiConfigureRxpFilterCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiConfigureRxpFilterCb = (WDI_ConfigureRxpFilterCb)pWDICtx->pfncRspCB; 

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in configure RXP filter Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  wdiConfigureRxpFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessConfigureRxpFilterRsp*/

/**
 @brief Process Set beacon filter Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetBeaconFilterRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetBeaconFilterCb   wdiBeaconFilterCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiBeaconFilterCb = (WDI_SetBeaconFilterCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in set beacon filter Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiBeaconFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetBeaconFilterRsp*/

/**
 @brief Process remove beacon filter Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessRemBeaconFilterRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_RemBeaconFilterCb   wdiBeaconFilterCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiBeaconFilterCb = (WDI_RemBeaconFilterCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in remove beacon filter Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiBeaconFilterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessRemBeaconFilterRsp*/

/**
 @brief Process set RSSI thresholds Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessSetRSSIThresoldsRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_SetRSSIThresholdsCb   wdiRSSIThresholdsCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiRSSIThresholdsCb = (WDI_SetRSSIThresholdsCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in set RSSI thresholds Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiRSSIThresholdsCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessSetRSSIThresoldsRsp*/

/**
 @brief Process host offload Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessHostOffloadRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_HostOffloadCb    wdiHostOffloadCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiHostOffloadCb = (WDI_HostOffloadCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in host offload Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiHostOffloadCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessHostOffloadRsp*/


/**
 @brief Process wowl add ptrn Rsp function (called when a
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlAddBcPtrnRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlAddBcPtrnCb    wdiWowlAddBcPtrnCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiWowlAddBcPtrnCb = (WDI_WowlAddBcPtrnCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in wowl add ptrn Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiWowlAddBcPtrnCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessWowlAddBcPtrnRsp*/

/**
 @brief Process wowl delete ptrn Rsp function (called when a 
        response is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlDelBcPtrnRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlDelBcPtrnCb    wdiWowlDelBcPtrnCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiWowlDelBcPtrnCb = (WDI_WowlDelBcPtrnCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in wowl delete ptrn Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiWowlDelBcPtrnCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessWowlDelBcPtrnRsp*/

/**
 @brief Process wowl enter Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlEnterRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlEnterReqCb   wdiWowlEnterCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiWowlEnterCb = (WDI_WowlEnterReqCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in wowl enter Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiWowlEnterCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessWowlEnterRsp*/

/**
 @brief Process wowl exit Rsp function (called when a response 
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessWowlExitRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_WowlExitReqCb   wdiWowlExitCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiWowlExitCb = (WDI_WowlExitReqCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in wowl exit Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiWowlExitCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessWowlExitRsp*/

/**
 @brief Process Configure Apps CPU wakeup State Rsp function 
        (called when a response is being received over the bus
        from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessConfigureAppsCpuWakeupStateRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
   WDI_Status           wdiStatus;
   eHalStatus           halStatus;
   WDI_ConfigureAppsCpuWakeupStateCb   wdiConfigureAppsCpuWakeupStateCb   = NULL;
   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   wdiConfigureAppsCpuWakeupStateCb = (WDI_ConfigureAppsCpuWakeupStateCb)pWDICtx->pfncRspCB; 

   /*-------------------------------------------------------------------------
     Sanity check 
   -------------------------------------------------------------------------*/
   if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
       ( NULL == pEventData->pEventData))
   {
      WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
               "Invalid parameters in configure Apps CPU wakeup state Response %x %x %x ",
                pWDICtx, pEventData, pEventData->pEventData);
      WDI_ASSERT(0);
      return WDI_STATUS_E_FAILURE; 
   }

   /*-------------------------------------------------------------------------
     Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
   halStatus = *((eHalStatus*)pEventData->pEventData);
   wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

   /*Notify UMAC*/
   wdiConfigureAppsCpuWakeupStateCb( wdiStatus, pWDICtx->pRspCBUserData);

   return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessConfigureAppsCpuWakeupStateRsp*/


/**
 @brief Process Nv download(called when a response
        is being received over the bus from HAL,will check if the responce is )
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessNvDownloadRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  WDI_NvDownloadRspCb    wdiNvDownloadRspCb   = NULL;
  tHalNvImgDownloadRspParams halNvDownloadRsp;
  WDI_NvDownloadRspInfoType wdiNvDownloadRsp;


  /*-------------------------------------------------------------------------
   Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
    ( NULL == pEventData->pEventData))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,	eWLAN_PAL_TRACE_LEVEL_WARN,
         "Invalid parameters in NV Download Response %x %x %x ",
          pWDICtx, pEventData, pEventData->pEventData);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
   -------------------------------------------------------------------------*/
  wpalMemoryCopy( &halNvDownloadRsp, 
                  pEventData->pEventData, 
                  sizeof(halNvDownloadRsp));

  wdiNvDownloadRsp.wdiStatus = WDI_HAL_2_WDI_STATUS(halNvDownloadRsp.status);

  if((wdiNvDownloadRsp.wdiStatus == WDI_STATUS_SUCCESS) &&
    (pWDICtx->wdiNvBlobInfo.usCurrentFragment != 
         pWDICtx->wdiNvBlobInfo.usTotalFragment )) 
  {
    WDI_NvDownloadReq(&pWDICtx->wdiCachedNvDownloadReq,
       (WDI_NvDownloadRspCb)pWDICtx->pfncRspCB, pWDICtx->pRspCBUserData); 
  }
  else
  {
    /*Reset the Nv related global information in WDI context information */
    pWDICtx->wdiNvBlobInfo.usTotalFragment = 0;
    pWDICtx->wdiNvBlobInfo.usFragmentSize = 0;
    pWDICtx->wdiNvBlobInfo.usCurrentFragment = 0;
    /*call WDA callback function for last fragment */
    wdiNvDownloadRspCb = (WDI_NvDownloadRspCb)pWDICtx->pfncRspCB;
    wdiNvDownloadRspCb( &wdiNvDownloadRsp, pWDICtx->pRspCBUserData);
  }

  return WDI_STATUS_SUCCESS; 
}
#ifdef WLAN_FEATURE_VOWIFI_11R
/**
 @brief Process Add TSpec Rsp function (called when a response
        is being received over the bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessAggrAddTSpecRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  tAggrAddTsRspParams   aggrAddTsRsp;
  WDI_AggrAddTsRspCb    wdiAggrAddTsRspCb   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wdiAggrAddTsRspCb = (WDI_AddTsRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Add TS Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract response and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( &aggrAddTsRsp, 
                  pEventData->pEventData, 
                  sizeof(aggrAddTsRsp));

  /* What is the difference between status0 and status1? */
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(aggrAddTsRsp.status0); 

  /*Notify UMAC*/
  wdiAggrAddTsRspCb( wdiStatus, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessAddTSpecRsp*/
#endif /* WLAN_FEATURE_VOWIFI_11R */

/*==========================================================================
                        Indications from HAL
 ==========================================================================*/
/**
 @brief Process Low RSSI Indication function (called when an 
        indication of this kind is being received over the bus
        from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessLowRSSIInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_LowLevelIndType  wdiInd;
  tHalRSSINotificationIndMsg   halRSSINotificationIndMsg;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/
  wpalMemoryCopy( (void *)&halRSSINotificationIndMsg.rssiNotificationParams, 
                  pEventData->pEventData, 
                  sizeof(tHalRSSINotification));

  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType = WDI_HAL_RSSI_NOTIFICATION_IND; 
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres1PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres1PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres1NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres1NegCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres2PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres2PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres2NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres2NegCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres3PosCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres3PosCross;
  wdiInd.wdiIndicationData.wdiLowRSSIInfo.bRssiThres3NegCross =
     halRSSINotificationIndMsg.rssiNotificationParams.bRssiThres3NegCross;

  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessLowRSSIInd*/


/**
 @brief Process Missed Beacon Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessMissedBeaconInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/
  /*! TO DO: Parameters need to be unpacked according to HAL struct*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType = WDI_MISSED_BEACON_IND; 
  
  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessMissedBeaconInd*/


/**
 @brief Process Unk Addr Frame Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessUnkAddrFrameInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/
  /*! TO DO: Parameters need to be unpacked according to HAL struct*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType = WDI_UNKNOWN_ADDR2_FRAME_RX_IND; 
  /* ! TO DO - fill in from HAL struct:
    wdiInd.wdiIndicationData.wdiUnkAddr2FrmInfo*/

  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessUnkAddrFrameInd*/


/**
 @brief Process MIC Failure Indication function (called when an 
        indication of this kind is being received over the bus
        from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessMicFailureInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/
  /*! TO DO: Parameters need to be unpacked according to HAL struct*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType = WDI_MIC_FAILURE_IND; 
  /* ! TO DO - fill in from HAL struct:
    wdiInd.wdiIndicationData.wdiMICFailureInfo*/

  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessMicFailureInd*/


/**
 @brief Process Fatal Failure Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFatalErrorInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status           wdiStatus;
  eHalStatus           halStatus;
  WDI_LowLevelIndType  wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/

  /*! TO DO: Parameters need to be unpacked according to HAL struct*/
  halStatus = *((eHalStatus*)pEventData->pEventData);
  wdiStatus   =   WDI_HAL_2_WDI_STATUS(halStatus); 

  WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Fatal failure received from device %d ", halStatus );
  
  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType             = WDI_FATAL_ERROR_IND; 
  wdiInd.wdiIndicationData.usErrorCode = WDI_ERR_DEV_INTERNAL_FAILURE; 

  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessFatalErrorInd*/

/**
 @brief Process Delete STA Indication function (called when 
        an indication of this kind is being received over the
        bus from HAL)
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessDelSTAInd
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  tDeleteStaContextParams    halDelSTACtx;
  WDI_LowLevelIndType        wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Low RSSI Ind %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  /*-------------------------------------------------------------------------
    Extract indication and send it to UMAC
  -------------------------------------------------------------------------*/

  /* Parameters need to be unpacked according to HAL struct*/
  wpalMemoryCopy( &halDelSTACtx, 
                  pEventData->pEventData, 
                  sizeof(halDelSTACtx));

  /*Fill in the indication parameters*/
  wdiInd.wdiIndicationType             = WDI_DEL_STA_IND; 

  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiDeleteSTAIndType.macADDR2,
                 halDelSTACtx.addr2, WDI_MAC_ADDR_LEN);
  wpalMemoryCopy(wdiInd.wdiIndicationData.wdiDeleteSTAIndType.macBSSID,
                 halDelSTACtx.bssId, WDI_MAC_ADDR_LEN);

  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.usAssocId = 
    halDelSTACtx.assocId;
  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.usSTAIdx  = 
    halDelSTACtx.staId;
  wdiInd.wdiIndicationData.wdiDeleteSTAIndType.wptReasonCode = 
    halDelSTACtx.reasonCode; 

  /*Notify UMAC*/
  pWDICtx->wdiLowLevelIndCB( &wdiInd, pWDICtx->pIndUserData );
  
  return WDI_STATUS_SUCCESS; 
}/*WDI_ProcessDelSTAInd*/

#ifdef ANI_MANF_DIAG
/**
 @brief WDI_ProcessFTMCommandReq
        Process FTM Command, simply route to HAL
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFTMCommandReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_FTMCommandReqType  *ftmCommandReq = NULL;
  wpt_uint8              *ftmCommandBuffer = NULL;
  wpt_uint16              dataOffset;
  wpt_uint16              bufferSize;
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))

  {
     WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in FTM CMD Req %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  ftmCommandReq = (WDI_FTMCommandReqType *)pEventData->pEventData;

  /* Get MSG Buffer */
  WDI_GetMessageBuffer(pWDICtx,
                       WDI_FTM_CMD_REQ,
                       ftmCommandReq->bodyLength,
                       &ftmCommandBuffer,
                       &dataOffset,
                       &bufferSize);

  wpalMemoryCopy(ftmCommandBuffer + dataOffset,
                 ftmCommandReq->FTMCommandBody,
                 ftmCommandReq->bodyLength);

  /* Send MSG */
  return WDI_SendMsg(pWDICtx,
                     ftmCommandBuffer,
                     bufferSize,
                     pEventData->pCBfnc,
                     pEventData->pUserData,
                     WDI_FTM_CMD_RESP);
}

/**
 @brief WDI_ProcessFTMCommadRsp
        Process FTM Command Response from HAL, simply route to HDD FTM
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_ProcessFTMCommadRsp
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  WDI_Status            wdiStatus;
  eHalStatus            halStatus;
  WDI_FTMCommandRspCb   ftmCMDRspCb = NULL;
  tProcessPttRspParams *ftmCMDRspData = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  ftmCMDRspCb = (WDI_FTMCommandRspCb)pWDICtx->pfncRspCB; 
  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if (( NULL == pWDICtx ) || ( NULL == pEventData ) ||
      ( NULL == pEventData->pEventData))
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
              "Invalid parameters in Update Beacon Params Response %x %x %x ",
               pWDICtx, pEventData, pEventData->pEventData);
     WDI_ASSERT(0);
     return WDI_STATUS_E_FAILURE; 
  }

  ftmCMDRspData = (tProcessPttRspParams *)pEventData->pEventData;

  wpalMemoryCopy((void *)pWDICtx->ucFTMCommandRspBuffer, 
                 (void *)&ftmCMDRspData->pttMsgBuffer, 
                 ftmCMDRspData->pttMsgBuffer.msgBodyLength);

  wdiStatus = WDI_HAL_2_WDI_STATUS(halStatus); 

  /*Notify UMAC*/
  ftmCMDRspCb((void *)pWDICtx->ucFTMCommandRspBuffer, pWDICtx->pRspCBUserData);

  return WDI_STATUS_SUCCESS; 
}
#endif /* ANI_MANF_DIAG */

/*==========================================================================
                     CONTRL TRANSPORT INTERACTION
 
    Callback function registered with the control transport - for receiving
    notifications and packets 
==========================================================================*/
/**
 @brief    This callback is invoked by the control transport 
   when it wishes to send up a notification like the ones
   mentioned above.
 
 @param
    
    wctsHandle:       handle to the control transport service 
    wctsEvent:        the event being notified
    wctsNotifyCBData: the callback data of the user 
    
 @see  WCTS_OpenTransport
  
 @return None 
*/
void 
WDI_NotifyMsgCTSCB
(
  WCTS_HandleType        wctsHandle, 
  WCTS_NotifyEventType   wctsEvent,
  void*                  wctsNotifyCBData
)
{
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)wctsNotifyCBData; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (NULL == pWDICtx )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Inavlid parameters received from CTS on the Notify CB");
    WDI_ASSERT(0);
    return; 
  }

  if ( WCTS_EVENT_OPEN == wctsEvent )
  {
    /*Flag must be set attomically as it is checked from incoming request
      functions*/
    wpalMutexAcquire(&pWDICtx->wptMutex);
    pWDICtx->bCTOpened   = eWLAN_PAL_TRUE; 

    /*Nothing to do - so try to dequeue any pending request that may have
     occured while we were trying to establish this*/
    WDI_DequeuePendingReq(pWDICtx);
    wpalMutexRelease(&pWDICtx->wptMutex);   
  }
  else if  ( WCTS_EVENT_CLOSE == wctsEvent ) 
  {
    /*Flag must be set attomically as it is checked from incoming request
      functions*/
    wpalMutexAcquire(&pWDICtx->wptMutex);
    pWDICtx->bCTOpened   = eWLAN_PAL_FALSE; 

    /*No other request will be processed from now on - fail all*/
    WDI_ClearPendingRequests(pWDICtx); 
    wpalMutexRelease(&pWDICtx->wptMutex);
  }

}/*WDI_NotifyMsgCTSCB*/


/**
 @brief    This callback is invoked by the control transport 
           when it wishes to send up a packet received over the
           bus.
 
 @param
    
    wctsHandle:  handle to the control transport service 
    pMsg:        the packet
    uLen:        the packet length
    wctsRxMsgCBData: the callback data of the user 
    
 @see  WCTS_OpenTransport
  
 @return None 
*/
void 
WDI_RXMsgCTSCB 
(
  WCTS_HandleType       wctsHandle, 
  void*                 pMsg,
  wpt_uint32            uLen,
  void*                 wctsRxMsgCBData
)
{
  tHalMsgHeader          halMsgHeader; 
  WDI_EventInfoType      wdiEventData; 
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)wctsRxMsgCBData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*------------------------------------------------------------------------
    Sanity check 
  ------------------------------------------------------------------------*/
  if ((NULL == pWDICtx ) || ( NULL == pMsg ) || 
      ( uLen < sizeof(halMsgHeader)))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Inavlid parameters received from CTS on the RX CB %x %x %d",
              pWDICtx, pMsg, uLen);
    WDI_ASSERT(0);
    return; 
  }

  /*The RX CAllback is expected to be serialized in the proper control thread 
    context - so no serialization is necessary here
    ! - revisit this assumption */

  wpalMemoryCopy(&halMsgHeader, pMsg, sizeof(halMsgHeader)); 

  if ( uLen != halMsgHeader.msgLen )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid packet received from HAL - catastrophic failure");
    WDI_DetectedDeviceError( pWDICtx, WDI_ERR_INVALID_RSP_FMT); 
    return; 
  }

  wdiEventData.wdiResponse = HAL_2_WDI_RSP_TYPE( halMsgHeader.msgType );
  
  /*The message itself starts after the header*/
  wdiEventData.pEventData     = (wpt_uint8*)pMsg + sizeof(halMsgHeader);
  wdiEventData.uEventDataSize = halMsgHeader.msgLen - sizeof(halMsgHeader);
  wdiEventData.pCBfnc         = gWDICb.pfncRspCB;
  wdiEventData.pUserData      = gWDICb.pRspCBUserData;


  if ( wdiEventData.wdiResponse ==  pWDICtx->wdiExpectedResponse )
  {
    /*Stop the timer as the response was received */
    /*!UT - check for potential race conditions between stop and response */
     wpalTimerStop(&pWDICtx->wptResponseTimer);
  }

  /*Post response event to the state machine*/
  WDI_PostMainEvent(pWDICtx, WDI_RESPONSE_EVENT, &wdiEventData);
  
}/*WDI_RXMsgCTSCB*/


/*========================================================================
         Internal Helper Routines 
========================================================================*/

/**
 @brief WDI_CleanCB - internal helper routine used to clean the 
        WDI Main Control Block
 
 @param pWDICtx - pointer to the control block

 @return Result of the function call
*/
WPT_INLINE WDI_Status
WDI_CleanCB
(
  WDI_ControlBlockType*  pWDICtx
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*Clean the WDI Control Block*/
  wpalMemoryZero( pWDICtx, sizeof(*pWDICtx)); 

  pWDICtx->uGlobalState  = WDI_MAX_ST; 
  pWDICtx->ucMaxBssids   = WDI_MAX_SUPPORTED_BSS;
  pWDICtx->ucMaxStations = WDI_MAX_SUPPORTED_STAS;

  WDI_ResetAssocSessions( pWDICtx );

  return WDI_STATUS_SUCCESS;
}/*WDI_CleanCB*/


/**
 @brief Process request helper function 

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WPT_INLINE WDI_Status
WDI_ProcessRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*!! Skip sanity check as this is called from the FSM functionss which 
    already checked these pointers*/

  if ( pEventData->wdiRequest < WDI_MAX_REQ )
  {  
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Calling request processing function for req %d %x", 
               pEventData->wdiRequest, pfnReqProcTbl[pEventData->wdiRequest]);
    return pfnReqProcTbl[pEventData->wdiRequest](pWDICtx, pEventData);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Operation % is not yet implemented ", 
               pEventData->wdiRequest);
    return WDI_STATUS_E_NOT_IMPLEMENT;
  }
}/*WDI_ProcessRequest*/


/**
 @brief Get message helper function - it allocates memory for a 
        message that is to be sent to HAL accross the bus and
        prefixes it with a send message header 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         wdiReqType:      type of the request being sent
         uBufferLen:      message buffer len
         pMsgBuffer:      resulting allocated buffer
         pusDataOffset:    offset in the buffer where the caller
         can start copying its message data
         puBufferSize:    the resulting buffer size (offset+buff
         len)
  
 @see
 @return Result of the function call
*/
WDI_Status
WDI_GetMessageBuffer
( 
  WDI_ControlBlockType*  pWDICtx, 
  WDI_RequestEnumType    wdiReqType, 
  wpt_uint16             usBufferLen,
  wpt_uint8**            pMsgBuffer, 
  wpt_uint16*            pusDataOffset, 
  wpt_uint16*            pusBufferSize
)
{
  tHalMsgHeader  halMsgHeader;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*!! No sanity check here as we trust the called - ! check this assumption 
    again*/

  /*-------------------------------------------------------------------------
     Try to allocate message buffer from PAL 
  -------------------------------------------------------------------------*/
  *pusBufferSize = sizeof(halMsgHeader) + usBufferLen; 
  *pMsgBuffer   = (wpt_uint8*)wpalMemoryAllocate(*pusBufferSize);
  if ( NULL ==  *pMsgBuffer )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Unable to allocate message buffer for req %d", wdiReqType); 
     WDI_ASSERT(0);
     return WDI_STATUS_MEM_FAILURE; 
  }

  /*-------------------------------------------------------------------------
     Fill in the message header
  -------------------------------------------------------------------------*/
  halMsgHeader.msgType = WDI_2_HAL_REQ_TYPE(wdiReqType); 
  halMsgHeader.msgLen  = sizeof(halMsgHeader) + usBufferLen; 
  *pusDataOffset       = sizeof(halMsgHeader); 
  wpalMemoryCopy(*pMsgBuffer, &halMsgHeader, sizeof(halMsgHeader)); 

  return WDI_STATUS_SUCCESS; 
}/*WDI_GetMessageBuffer*/


/**
 @brief Send message helper function - sends a message over the 
        bus using the control tranport and saves some info in
        the CB 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pSendBuffer:     buffer to be sent
  
         usSendSize          size of the buffer to be sent
         pRspCb:            response callback - save in the WDI
         CB
         pUserData:         user data associated with the
         callback
         wdiExpectedResponse: the code of the response that is
         expected to be rx-ed for this request
  
 @see
 @return Result of the function call
*/
WDI_Status 
WDI_SendMsg
( 
  WDI_ControlBlockType*  pWDICtx,  
  wpt_uint8*             pSendBuffer, 
  wpt_uint32             usSendSize, 
  void*                  pRspCb, 
  void*                  pUserData,
  WDI_ResponseEnumType   wdiExpectedResponse
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*------------------------------------------------------------------------
    Save needed info in the CB 
  ------------------------------------------------------------------------*/
  pWDICtx->pRspCBUserData      = pUserData;
  pWDICtx->pfncRspCB           = pRspCb; 
  pWDICtx->wdiExpectedResponse = wdiExpectedResponse; 

   /*-----------------------------------------------------------------------
     Call the CTS to send this message over - free message afterwards
     - notify transport failure
     Note: CTS is reponsible for freeing the message buffer.
   -----------------------------------------------------------------------*/
   if ( 0 != WCTS_SendMessage( pWDICtx->wctsHandle, (void*)pSendBuffer, usSendSize ))
   {
      WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
                "Failed to send message over the bus - catastrophic failure");

      /*Inform Upper MAC that the request could not go through*/
      if ( NULL != pWDICtx->wdiReqStatusCB )
      {
        pWDICtx->wdiReqStatusCB( WDI_STATUS_E_FAILURE, 
                                 pWDICtx->pReqStatusUserData); 
      }

      /*Free the buffer to prevent memory leak*/
      /*wpalMemoryFree( pSendBuffer);
        WCTS API takes ownership of this pointer and it is therefore in charge
        of freeing it
      **/
      WDI_DetectedDeviceError( pWDICtx, WDI_ERR_TRANSPORT_FAILURE);
      return WDI_STATUS_E_FAILURE;
   }

   /*Inform Upper MAC that the request went through*/
   if ( NULL != pWDICtx->wdiReqStatusCB )
   {
     pWDICtx->wdiReqStatusCB( WDI_STATUS_SUCCESS, pWDICtx->pReqStatusUserData); 
   }

   /*Start timer for the expected response */
   wpalTimerStart(&pWDICtx->wptResponseTimer, WDI_RESPONSE_TIMEOUT);

   return WDI_STATUS_SUCCESS; 
}/*WDI_SendMsg*/


/**
 @brief WDI_DetectedDeviceError - called internally by DAL when 
        it has detected a failure in the device 
 
 @param  pWDICtx:        pointer to the WLAN DAL context 
         usErrorCode:    error code detected by WDI or received
                         from HAL
  
 @see
 @return None 
*/
void
WDI_DetectedDeviceError
(
  WDI_ControlBlockType*  pWDICtx,
  wpt_uint16             usErrorCode
)
{
  WDI_LowLevelIndType  wdiInd;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Device Error detected code: %d - transitioning to stopped state",
            usErrorCode);

  wpalMutexAcquire(&pWDICtx->wptMutex);

  WDI_STATableStop(pWDICtx);

  WDI_ResetAssocSessions(pWDICtx);

  /*Set the expected state transition to stopped - because the device
  experienced a failure*/
  pWDICtx->ucExpectedStateTransition =  WDI_STOPPED_ST;

  /*Transition to stopped to fail all incomming requests from this point on*/
  WDI_STATE_TRANSITION( pWDICtx, WDI_STOPPED_ST); 

  WDI_ClearPendingRequests(pWDICtx); 

  /*TO DO: -  there should be an attempt to reset the device here*/

  wpalMutexRelease(&pWDICtx->wptMutex);

  /*------------------------------------------------------------------------
    Notify UMAC
  ------------------------------------------------------------------------*/
  wdiInd.wdiIndicationType             = WDI_FATAL_ERROR_IND; 
  wdiInd.wdiIndicationData.usErrorCode = usErrorCode; 

  pWDICtx->wdiLowLevelIndCB( &wdiInd,  pWDICtx->pIndUserData);
}/*WDI_DetectedDeviceError*/

/**
 @brief    This callback is invoked by the wpt when a timer that 
           we started on send message has expire - this should
           never happen - it means device is stuck and cannot
           reply - trigger catastrophic failure 
 @param 
    
    pUserData: the callback data of the user (ptr to WDI CB)
    
 @see 
 @return None 
*/
void 
WDI_ResponseTimerCB
(
  void *pUserData
)
{
  WDI_ControlBlockType*  pWDICtx = (WDI_ControlBlockType*)pUserData; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (NULL == pWDICtx )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid parameters received in the timer callback");
    WDI_ASSERT(0);
    return; 
  }


  WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
            "Timeout occured while waiting for response from device "
            " - catastrophic failure");
  WDI_DetectedDeviceError( pWDICtx, WDI_ERR_RSP_TIMEOUT); 
  return; 

}/*WDI_ResponseTimerCB*/


/**
 @brief Process response helper function 

 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
*/
WPT_INLINE WDI_Status
WDI_ProcessResponse
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /* Skip sanity check as this is called from the FSM functions which 
    already checked these pointers
    ! - revisit this assumption */

  if ( pEventData->wdiResponse < WDI_MAX_RESP )
  {  
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_INFO,
              "Calling response processing function for resp %d %x", 
               pEventData->wdiResponse, pfnRspProcTbl[pEventData->wdiResponse]);
    return pfnRspProcTbl[pEventData->wdiResponse](pWDICtx, pEventData);
  }
  else
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Operation % is not yet implemented ", 
               pEventData->wdiResponse);
    return WDI_STATUS_E_NOT_IMPLEMENT;
  }
}/*WDI_ProcessResponse*/


/*=========================================================================
                   QUEUE SUPPORT UTILITY FUNCTIONS 
=========================================================================*/

/**
 @brief    Utility function used by the DAL Core to help queue a 
           request that cannot be processed right away. 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: - pointer to the evnt info that needs to be
    queued 
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueuePendingReq
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{
  wpt_list_node*      pNode; 
  WDI_EventInfoType*  pEventDataQueue = wpalMemoryAllocate(sizeof(*pEventData));
  void*               pEventInfo; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if ( NULL ==  pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse; 

  pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);

  if ( NULL ==  pEventInfo )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing event data info"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);
  pEventDataQueue->pEventData = pEventInfo;

  /*Send wpt a pointer to the node (this is the 1st element in the event data)*/
  pNode = (wpt_list_node*)pEventDataQueue; 

  wpal_list_insert_back(&(pWDICtx->wptPendingQueue), pNode); 

  return WDI_STATUS_SUCCESS;
}/*WDI_QueuePendingReq*/

/**
 @brief    Callback function for serializing queued message 
           processing in the control context
 @param 
    
    pMsg - pointer to the message 
    
 @see 
 @return Result of the operation  
*/
void 
WDI_PALCtrlMsgCB
(
 wpt_msg *pMsg
)
{
  WDI_EventInfoType*     pEventData = NULL;
  WDI_ControlBlockType*  pWDICtx    = NULL; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  if (( NULL == pMsg )||
      ( NULL == (pEventData = (WDI_EventInfoType*)pMsg->ptr)) ||
      ( NULL == (pWDICtx  = (WDI_ControlBlockType*)pMsg->pContext )))
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid message received on serialize ctrl context API"); 
    WDI_ASSERT(0);
    return; 
  }

  /*Transition back to the state that we had before serialization
  - serialization transitions us to BUSy to stop any incomming requests
  ! TO DO L: possible race condition here if a request comes in between the
   state transition and the post function*/

  WDI_STATE_TRANSITION( pWDICtx, pMsg->val); 

  /*-----------------------------------------------------------------------
     Check to see what type of event we are serializing
     - responses are never expected to come through here 
  -----------------------------------------------------------------------*/
  switch ( pEventData->wdiRequest )
  {
     
  case WDI_STOP_REQ:
    WDI_PostMainEvent(&gWDICb, WDI_STOP_EVENT, pEventData);
    break;
  
  default: 
    WDI_PostMainEvent(&gWDICb, WDI_REQUEST_EVENT, pEventData);
    break;
  }/*switch ( pEventData->wdiRequest )*/

  /* Free data - that was allocated when queueing*/
  wpalMemoryFree(pEventData->pEventData);
  wpalMemoryFree(pEventData);
  wpalMemoryFree(pMsg);
}/*WDI_PALCtrlMsgCB*/

/**
 @brief    Utility function used by the DAL Core to help dequeue
           and schedule for execution a pending request 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: - pointer to the evnt info that needs to be
    queued 
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_DequeuePendingReq
(
  WDI_ControlBlockType*  pWDICtx
)
{
  wpt_list_node*      pNode           = NULL; 
  WDI_EventInfoType*  pEventDataQueue = NULL;
  wpt_msg*            palMsg; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode); 

  if ( NULL ==  pNode )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_WARN,
              "List is empty - return"); 
    return WDI_STATUS_SUCCESS;
  }

  /*The node actually points to the 1st element inside the Event Data struct -
    just cast it back to the struct*/
  pEventDataQueue = (WDI_EventInfoType*)pNode; 

  /*Serialize processing in the control thread
     !TO DO: - check to see if these are all the messages params that need
     to be filled in*/
  palMsg = wpalMemoryAllocate(sizeof(wpt_msg));

  if ( NULL ==  palMsg )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "WDI_DequeuePendingReq: Cannot allocate memory for palMsg."); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE; 
  }
  palMsg->pContext = pWDICtx; 
  palMsg->callback = WDI_PALCtrlMsgCB;
  palMsg->ptr      = pEventDataQueue;

  /*Save the global state as we need it on the other side*/
  palMsg->val      = pWDICtx->uGlobalState; 
    
  /*Transition back to BUSY as we need to handle a queued request*/
  WDI_STATE_TRANSITION( pWDICtx, WDI_BUSY_ST);
  
  wpalPostCtrlMsg(pWDICtx->pPALContext, palMsg);

  /*!TO DO L: - need to also try to dequeue assoc messages - but I duno in what
    order  !!! - design problem */
  return WDI_STATUS_PENDING;
}/*WDI_DequeuePendingReq*/


/**
 @brief    Utility function used by the DAL Core to help queue 
           an association request that cannot be processed right
           away.- The assoc requests will be queued by BSSID 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pEventData: pointer to the evnt info that needs to be queued
    macBSSID: bssid
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueueNewAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData,
  wpt_macAddr            macBSSID
)
{
  wpt_uint8 i; 
  WDI_BSSSessionType* pSession = NULL; 
  wpt_list_node*      pNode; 
  WDI_EventInfoType*  pEventDataQueue;
  void*               pEventInfo; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  

  /*------------------------------------------------------------------------ 
      Search for a session that matches the BSSID 
    ------------------------------------------------------------------------*/
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( eWLAN_PAL_FALSE == pWDICtx->aBSSSessions[i].bInUse )
     {
       /*Found an empty session*/
       pSession = &pWDICtx->aBSSSessions[i]; 
       break; 
     }
  }

  if ( i >=  WDI_MAX_BSS_SESSIONS )
  {
    /*Cannot find any empty sessions*/
    return WDI_STATUS_E_FAILURE; 
  }
  
  /*------------------------------------------------------------------------
    Fill in the BSSID for this session and set the usage flag
  ------------------------------------------------------------------------*/
  wpalMemoryCopy(pWDICtx->aBSSSessions[i].macBSSID, macBSSID, WDI_MAC_ADDR_LEN);
  pWDICtx->aBSSSessions[i].bInUse = eWLAN_PAL_TRUE; 

  /*------------------------------------------------------------------------
    Allocate memory for this and place it in the queue 
  ------------------------------------------------------------------------*/
  pEventDataQueue = (WDI_EventInfoType*)wpalMemoryAllocate(sizeof(WDI_EventInfoType));
  if ( NULL ==  pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse; 

  pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);

  if ( NULL ==  pEventInfo )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing event data info"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);
  pEventDataQueue->pEventData = pEventInfo;

  /*Send wpt a pointer to the node (this is the 1st element in the event data)*/
  pNode = (wpt_list_node*)pEventDataQueue; 

  /*This association is currently being queued*/
  pSession->bAssocReqQueued = eWLAN_PAL_TRUE; 

  wpal_list_insert_back(&(pSession->wptPendingQueue), pNode); 

  /*Return pending as this is what the status of the request is since it has
    been queued*/
  return WDI_STATUS_PENDING;
}/*WDI_QueueNewAssocRequest*/

/**
 @brief    Utility function used by the DAL Core to help queue 
           an association request that cannot be processed right
           away.- The assoc requests will be queued by BSSID 
 @param 
    
    pWDICtx: - pointer to the WDI control block
    pSession: - session in which to queue
    pEventData: pointer to the event info that needs to be
    queued
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_QueueAssocRequest
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_BSSSessionType*    pSession,
  WDI_EventInfoType*     pEventData
)
{
  wpt_list_node*      pNode; 
  WDI_EventInfoType*  pEventDataQueue;
  void*               pEventInfo; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  
  /*------------------------------------------------------------------------ 
      Sanity check
    ------------------------------------------------------------------------*/
  if (( NULL == pSession ) || ( NULL == pWDICtx ))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid parameters on WDI_QueueAssocRequest");

    return WDI_STATUS_E_FAILURE; 
  }

  /*------------------------------------------------------------------------
    Allocate memory for this and place it in the queue 
  ------------------------------------------------------------------------*/
  pEventDataQueue = (WDI_EventInfoType*)wpalMemoryAllocate(sizeof(WDI_EventInfoType));
  if ( NULL ==  pEventDataQueue )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  pEventDataQueue->pCBfnc          = pEventData->pCBfnc;
  pEventDataQueue->pUserData       = pEventData->pUserData;
  pEventDataQueue->uEventDataSize  = pEventData->uEventDataSize;
  pEventDataQueue->wdiRequest      = pEventData->wdiRequest;
  pEventDataQueue->wdiResponse     = pEventData->wdiResponse; 

  pEventInfo = wpalMemoryAllocate(pEventData->uEventDataSize);

  if ( NULL ==  pEventInfo )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Cannot allocate memory for queueing event data info"); 
    WDI_ASSERT(0);
    return WDI_STATUS_MEM_FAILURE;
  }

  wpalMemoryCopy(pEventInfo, pEventData->pEventData, pEventData->uEventDataSize);
  pEventDataQueue->pEventData = pEventInfo;

  /*Send wpt a pointer to the node (this is the 1st element in the event data)*/
  pNode = (wpt_list_node*)pEventDataQueue; 

  /*This association is currently being queued*/
  pSession->bAssocReqQueued = eWLAN_PAL_TRUE; 

  wpal_list_insert_back(&(pSession->wptPendingQueue), pNode); 

  /*The result of this operation is pending because the request has been
    queued and it will be processed at a later moment in time */
  return WDI_STATUS_PENDING;
}/*WDI_QueueAssocRequest*/


/**
 @brief    Utility function used by the DAL Core to clear any 
           pending requests - all req cb will be called with
           failure and the queue will be emptied.
 @param 
    
    pWDICtx: - pointer to the WDI control block
    
 @see 
 @return Result of the operation  
*/
WDI_Status
WDI_ClearPendingRequests
( 
  WDI_ControlBlockType*  pWDICtx
)
{
  wpt_list_node*      pNode = NULL; 
  WDI_EventInfoType*  pEventDataQueue = NULL;
  WDI_ReqStatusCb     pfnReqStatusCB; 
  void*               pUserData;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode); 

  /*------------------------------------------------------------------------
    Go through all the requests and fail them - this will only be called
    when device is being stopped or an error was detected - either case the
    pending requests can no longer be sent down to HAL 
  ------------------------------------------------------------------------*/
  while( pNode )
  {
      /*The node actually points to the 1st element inside the Event Data struct -
    just cast it back to the struct*/
    pEventDataQueue = (WDI_EventInfoType*)pNode; 
  
    WDI_ExtractRequestCBFromEvent(pEventDataQueue, &pfnReqStatusCB, &pUserData);
    if ( NULL != pfnReqStatusCB )
    {
      /*Fail the request*/
      pfnReqStatusCB( WDI_STATUS_E_FAILURE, pUserData);
    }
    
    wpal_list_remove_front(&(pWDICtx->wptPendingQueue), &pNode); 
  } 
 
  return WDI_STATUS_SUCCESS;
}/*WDI_ClearPendingRequests*/

/**
 @brief Helper routine used to init the BSS Sessions in the WDI control block 
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
  
 @see
*/
void
WDI_ResetAssocSessions
( 
  WDI_ControlBlockType*   pWDICtx
)
{
  wpt_uint8 i; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    No Sanity check
  -------------------------------------------------------------------------*/
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
    wpalMemoryZero( &pWDICtx->aBSSSessions[i], sizeof(WDI_BSSSessionType) ); 
    pWDICtx->aBSSSessions[i].wdiAssocState = WDI_ASSOC_INIT_ST;
    pWDICtx->aBSSSessions[i].bcastStaIdx = WDI_STA_INVALID_IDX;
    pWDICtx->aBSSSessions[i].usBSSIdx = WDI_BSS_INVALID_IDX;
  }
}/*WDI_ResetAssocSessions*/

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
         macBSSID:      BSSID of the session
         pSession:      pointer to the session (if found) 
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSession
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_macAddr             macBSSID,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if ( NULL == ppSession )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on WDI_FindAssocSession"); 
     return WDI_MAX_BSS_SESSIONS; 
  }

  *ppSession = NULL; 

  /*------------------------------------------------------------------------ 
      Search for a session that matches the BSSID 
    ------------------------------------------------------------------------*/
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( eWLAN_PAL_TRUE == 
          wpalMemoryCompare(pWDICtx->aBSSSessions[i].macBSSID, macBSSID, WDI_MAC_ADDR_LEN) )
     {
       /*Found the session*/
       *ppSession = &pWDICtx->aBSSSessions[i]; 
       return i;
     }
  }

  return i; 
}/*WDI_FindAssocSession*/

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         usBssIdx:  BSS Index of the session
         ppSession: out pointer to the session (if found)
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSessionByBSSIdx
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              usBssIdx,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if ( NULL == ppSession )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on WDI_FindAssocSessionByBSSIdx"); 
     return WDI_MAX_BSS_SESSIONS; 
  }

  *ppSession = NULL; 

  /*------------------------------------------------------------------------ 
      Search for a session that matches the BSSID 
    ------------------------------------------------------------------------*/
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( usBssIdx == pWDICtx->aBSSSessions[i].usBSSIdx )
     {
       /*Found the session*/
       *ppSession = &pWDICtx->aBSSSessions[i]; 
       return i;
     }
  }

  return i; 
}/*WDI_FindAssocSessionByBSSIdx*/

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
         usBssIdx:  BSS Index of the session
         ppSession: out pointer to the session (if found)
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindAssocSessionByIdx
( 
  WDI_ControlBlockType*   pWDICtx,
  wpt_uint16              usIdx,
  WDI_BSSSessionType**    ppSession
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if ( NULL == ppSession && usIdx >= WDI_MAX_BSS_SESSIONS )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on WDI_FindAssocSessionByIdx"); 
     return WDI_MAX_BSS_SESSIONS; 
  }

  /*Found the session*/
  *ppSession = &pWDICtx->aBSSSessions[usIdx]; 

  return usIdx;
  
}/*WDI_FindAssocSessionByBSSIdx*/

/**
 @brief Helper routine used to find an empty session in the WDI 
        CB
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
         pSession:      pointer to the session (if found) 
  
 @see
 @return Index of the session in the array 
*/
wpt_uint8
WDI_FindEmptySession
( 
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType**    ppSession
)
{
  wpt_uint8 i; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
   /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if ( NULL == ppSession )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on WDI_FindAssocSession"); 
     return WDI_MAX_BSS_SESSIONS; 
  }

  *ppSession = NULL; 

  /*------------------------------------------------------------------------ 
      Search for a session that it is not in use 
    ------------------------------------------------------------------------*/
  for ( i = 0; i < WDI_MAX_BSS_SESSIONS; i++ )
  {
     if ( ! pWDICtx->aBSSSessions[i].bInUse )
     {
       /*Found a session*/
       *ppSession = &pWDICtx->aBSSSessions[i]; 
       return i;
     }
  }

  return i; 
}/*WDI_FindEmptySession*/


/**
 @brief Helper routine used to delete session in the WDI 
        CB
  
 
 @param  pWDICtx:       pointer to the WLAN DAL context 
         pSession:      pointer to the session (if found) 
  
 @see
 @return Index of the session in the array 
*/
void 
WDI_DeleteSession
( 
  WDI_ControlBlockType*   pWDICtx,
  WDI_BSSSessionType*     ppSession
)
{
   /*-------------------------------------------------------------------------
    Sanity check 
  -------------------------------------------------------------------------*/
  if ( NULL == ppSession )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
               "Invalid parameters on WDI_FindAssocSession"); 
     return ; 
  }

  /*------------------------------------------------------------------------ 
      Reset the entries int session 
    ------------------------------------------------------------------------*/
  wpal_list_destroy(&ppSession->wptPendingQueue, pNode);
  wpalMemoryZero(ppSession,  sizeof(*ppSession));
  ppSession->wdiAssocState = WDI_ASSOC_INIT_ST; 
  ppSession->bInUse        = eWLAN_PAL_FALSE; 
  wpal_list_init(&ppSession->wptPendingQueue, pNode);

}/*WDI_FindEmptySession*/

/**
 @brief    Utility function to add the broadcast STA to the the STA table. 
 The bcast STA ID is assigned by HAL and must be valid.
 @param 
    
    WDI_AddStaParams: - pointer to the WDI Add STA params
    usBcastStaIdx: - Broadcast STA index passed by HAL
    
 @see 
 @return void 
*/
void
WDI_AddBcastSTAtoSTATable
(
  WDI_ControlBlockType*  pWDICtx,
  WDI_AddStaParams *     staParams,
  wpt_uint16             usBcastStaIdx
)
{
  WDI_AddStaParams              wdiAddSTAParam = {0};
  wpt_macAddr  bcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---------------------------------------------------------------------
    Sanity check
  ---------------------------------------------------------------------*/
  if ( NULL == staParams )
  {
     WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "Invalid input parameters in WDI_AddBcastSTAtoSTATable");

    return; 
  }

  wdiAddSTAParam.bcastDpuIndex = staParams->bcastDpuIndex;
  wdiAddSTAParam.bcastDpuSignature = staParams->bcastDpuSignature;
  wdiAddSTAParam.bcastMgmtDpuIndex = staParams->bcastMgmtDpuIndex;
  wdiAddSTAParam.bcastMgmtDpuSignature = staParams->bcastMgmtDpuSignature;
  wdiAddSTAParam.dpuIndex = staParams->dpuIndex;
  wdiAddSTAParam.dpuSig = staParams->dpuSig;
  wpalMemoryCopy( wdiAddSTAParam.macBSSID, staParams->macBSSID,
                  WDI_MAC_ADDR_LEN );
  wpalMemoryCopy( wdiAddSTAParam.staMacAddr, bcastMacAddr, WDI_MAC_ADDR_LEN );
  wdiAddSTAParam.ucbssIndex = staParams->ucbssIndex;
  wdiAddSTAParam.ucHTCapable = staParams->ucHTCapable;
  wdiAddSTAParam.ucRmfEnabled = staParams->ucRmfEnabled;
  wdiAddSTAParam.ucStaType = WDI_STA_ENTRY_BCAST;
  wdiAddSTAParam.ucWmmEnabled = staParams->ucWmmEnabled;
  wdiAddSTAParam.usStaIdx = usBcastStaIdx;
    
  (void)WDI_STATableAddSta(pWDICtx,&wdiAddSTAParam);
}

/**
 @brief NV blob will be divided into fragments of size 4kb and 
 Sent to HAL 
 
 @param  pWDICtx:         pointer to the WLAN DAL context 
         pEventData:      pointer to the event information structure 
  
 @see
 @return Result of the function call
 */

WDI_Status WDI_SendNvBlobReq
( 
  WDI_ControlBlockType*  pWDICtx,
  WDI_EventInfoType*     pEventData
)
{

  tHalNvImgDownloadReqMsg    halNvImgDownloadParam;
  wpt_uint8*                 pSendBuffer   = NULL;
  wpt_uint16                 usDataOffset  = 0;
  wpt_uint16                 usSendSize    = 0;
  wpt_uint16                 usCurrentFragmentSize =0;
  wpt_uint8*                 pSrcBuffer = NULL;
  WDI_NvDownloadReqParamsType*  pwdiNvDownloadReqParams =NULL ;
  WDI_NvDownloadRspCb      wdiNvDownloadRspCb;


  wdiNvDownloadRspCb = (WDI_NvDownloadRspCb)pEventData->pCBfnc;
  WDI_ASSERT(NULL != wdiNvDownloadRspCb);
  pwdiNvDownloadReqParams = (WDI_NvDownloadReqParamsType*)pEventData->pEventData;

  /* Sanity Check is done by the caller */ 
  pSrcBuffer =(wpt_uint8 *) pwdiNvDownloadReqParams->wdiBlobInfo.pBlobAddress;

  /* Update the current  Fragment Number */
  pWDICtx->wdiNvBlobInfo.usCurrentFragment += 1;

  /*Update the HAL REQ structure */
  /*HAL maintaining the fragment count as 0,1,2...n where at WDI it is represented as 1,2,3.. n*/
  halNvImgDownloadParam.nvImageReqParams.fragNumber =
                                     pWDICtx->wdiNvBlobInfo.usCurrentFragment-1;

  /*    Divide the NV Image to size of 'FRAGMENT_SIZE' fragments and send it to HAL.
       If the size of the Image is less than 'FRAGMENT_SIZE' then in one iteration total 
       image will be sent to HAL*/

  if(pWDICtx->wdiNvBlobInfo.usTotalFragment 
                         == pWDICtx->wdiNvBlobInfo.usCurrentFragment)
  { 
    /*     Taking care of boundry condition */
    if( !(usCurrentFragmentSize = 
                 pwdiNvDownloadReqParams->wdiBlobInfo.uBlobSize%FRAGMENT_SIZE ))
      usCurrentFragmentSize = FRAGMENT_SIZE;

    /*Update the HAL REQ structure */
    halNvImgDownloadParam.nvImageReqParams.isLastFragment = 1;
    halNvImgDownloadParam.nvImageReqParams.nvImgBufferSize= usCurrentFragmentSize;

  }
  else
  { 
    usCurrentFragmentSize = FRAGMENT_SIZE;

    /*Update the HAL REQ structure */
    halNvImgDownloadParam.nvImageReqParams.isLastFragment =0;
    halNvImgDownloadParam.nvImageReqParams.nvImgBufferSize = usCurrentFragmentSize;
  }


  /*-----------------------------------------------------------------------
   Get message buffer
   -----------------------------------------------------------------------*/
  if (( WDI_STATUS_SUCCESS != WDI_GetMessageBuffer( pWDICtx,WDI_NV_DOWNLOAD_REQ,
         sizeof(halNvImgDownloadParam.nvImageReqParams)+ usCurrentFragmentSize,
                    &pSendBuffer, &usDataOffset, &usSendSize))||
      ( usSendSize < 
           (usDataOffset + sizeof(halNvImgDownloadParam.nvImageReqParams) + usCurrentFragmentSize )))
  {
    WPAL_TRACE( eWLAN_MODULE_DAL_CTRL,  eWLAN_PAL_TRACE_LEVEL_WARN,
         "Unable to get send buffer in NV Download req %x %x ",
         pEventData, pwdiNvDownloadReqParams);
    WDI_ASSERT(0);
    return WDI_STATUS_E_FAILURE; 
  }

  /* Copying the Hal NV download REQ structure */
  wpalMemoryCopy(pSendBuffer + usDataOffset , 
    &halNvImgDownloadParam.nvImageReqParams ,sizeof(tHalNvImgDownloadReqParams));

  /* Appending the NV image fragment */
  wpalMemoryCopy(pSendBuffer + usDataOffset + sizeof(tHalNvImgDownloadReqParams),
        (void *)(pSrcBuffer + halNvImgDownloadParam.nvImageReqParams.fragNumber * FRAGMENT_SIZE),
                  usCurrentFragmentSize);

  pWDICtx->wdiReqStatusCB     = pwdiNvDownloadReqParams->wdiReqStatusCB;
  pWDICtx->pReqStatusUserData = pwdiNvDownloadReqParams->pUserData; 

  return  WDI_SendMsg( pWDICtx, pSendBuffer, usSendSize, 
                           wdiNvDownloadRspCb, pEventData->pUserData, 
                           WDI_NV_DOWNLOAD_RESP);

}
/*============================================================================ 
  Helper inline functions for 
 ============================================================================*/
/**
 @brief Helper routine used to find a session based on the BSSID 
 @param  pContext:   pointer to the WLAN DAL context 
 @param  pDPContext:   pointer to the Datapath context 
  
 @see
 @return 
*/
WPT_INLINE void 
WDI_DS_AssignDatapathContext (void *pContext, void *pDPContext)
{
	WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
	pCB->pDPContext = pDPContext;
	return;
}

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pContext:   pointer to the WLAN DAL context 
  
 @see
 @return pointer to Datapath context
*/
WPT_INLINE void * 
WDI_DS_GetDatapathContext (void *pContext)
{
	WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
	return pCB->pDPContext;
}
/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pContext:   pointer to the WLAN DAL context 
 @param  pDTDriverContext:   pointer to the Transport Driver context 
  
 @see
 @return void
*/
WPT_INLINE void  
WDT_AssignTransportDriverContext (void *pContext, void *pDTDriverContext)
{
	WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
	pCB->pDTDriverContext = pDTDriverContext;
	return; 
}

/**
 @brief Helper routine used to find a session based on the BSSID 
  
 
 @param  pWDICtx:   pointer to the WLAN DAL context 
  
 @see
 @return pointer to datapath context 
*/
WPT_INLINE void * 
WDT_GetTransportDriverContext (void *pContext)
{
	WDI_ControlBlockType *pCB = (WDI_ControlBlockType *)pContext;
	return(pCB->pDTDriverContext); 
}

/*============================================================================ 
  Helper inline convertors
 ============================================================================*/
/*Convert WDI driver type into HAL driver type*/
WPT_STATIC WPT_INLINE WDI_Status
WDI_HAL_2_WDI_STATUS
(
  eHalStatus halStatus
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  halStatus )
  {
  case eHAL_STATUS_SUCCESS:
    return WDI_STATUS_SUCCESS;
  case eHAL_STATUS_FAILURE:
    return WDI_STATUS_E_FAILURE;
  case eHAL_STATUS_FAILED_ALLOC:
    return WDI_STATUS_MEM_FAILURE; 
  case eHAL_STATUS_RESOURCES:
    return WDI_STATUS_RES_FAILURE;
  case eHAL_STATUS_NOT_OPEN:
   return WDI_STATUS_E_NOT_ALLOWED;       
   /*The rest of the HAL error codes must be kept hidden from the UMAC as 
     they refer to specific internal modules of our device*/
  default: 
    return WDI_STATUS_DEV_INTERNAL_FAILURE; 
  } 

  return WDI_STATUS_E_FAILURE; 
}/*WDI_HAL_2_WDI_STATUS*/

/*Convert WDI request type into HAL request type*/
WPT_STATIC WPT_INLINE tHalHostMsgType
WDI_2_HAL_REQ_TYPE
(
  WDI_RequestEnumType    wdiReqType
)
{
   /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiReqType )
  {    
  case WDI_START_REQ:
    return WLAN_HAL_START_REQ; 
  case WDI_STOP_REQ:
    return WLAN_HAL_STOP_REQ; 
  case WDI_INIT_SCAN_REQ:
    return WLAN_HAL_INIT_SCAN_REQ; 
  case WDI_START_SCAN_REQ:
    return WLAN_HAL_START_SCAN_REQ; 
  case WDI_END_SCAN_REQ:
    return WLAN_HAL_END_SCAN_REQ; 
  case WDI_FINISH_SCAN_REQ:
    return WLAN_HAL_FINISH_SCAN_REQ; 
  case WDI_JOIN_REQ:
    return WLAN_HAL_JOIN_REQ; 
  case WDI_CONFIG_BSS_REQ:
    return WLAN_HAL_CONFIG_BSS_REQ; 
  case WDI_DEL_BSS_REQ:
    return WLAN_HAL_DELETE_BSS_REQ; 
  case WDI_POST_ASSOC_REQ:
    return WLAN_HAL_POST_ASSOC_REQ; 
  case WDI_DEL_STA_REQ:
    return WLAN_HAL_DELETE_STA_REQ; 
  case WDI_SET_BSS_KEY_REQ:
    return WLAN_HAL_SET_BSSKEY_REQ; 
  case WDI_RMV_BSS_KEY_REQ:
    return WLAN_HAL_RMV_BSSKEY_REQ; 
  case WDI_SET_STA_KEY_REQ:
    return WLAN_HAL_SET_STAKEY_REQ; 
  case WDI_RMV_STA_KEY_REQ:
    return WLAN_HAL_RMV_STAKEY_REQ; 
  case WDI_SET_STA_BCAST_KEY_REQ:
    return WLAN_HAL_SET_BCASTKEY_REQ; 
  case WDI_RMV_STA_BCAST_KEY_REQ:
    //Some conflict in the old code - check this: return WLAN_HAL_RMV_BCASTKEY_REQ; 
    return WLAN_HAL_RMV_STAKEY_REQ;
  case WDI_ADD_TS_REQ:
    return WLAN_HAL_ADD_TS_REQ; 
  case WDI_DEL_TS_REQ:
    return WLAN_HAL_DEL_TS_REQ; 
  case WDI_UPD_EDCA_PRMS_REQ:
    return WLAN_HAL_UPD_EDCA_PARAMS_REQ; 
  case WDI_ADD_BA_REQ:
    return WLAN_HAL_ADD_BA_REQ; 
  case WDI_DEL_BA_REQ:
    return WLAN_HAL_DEL_BA_REQ; 
  case WDI_CH_SWITCH_REQ:
    return WLAN_HAL_CH_SWITCH_REQ; 
  case WDI_CONFIG_STA_REQ:
    return WLAN_HAL_CONFIG_STA_REQ; 
  case WDI_SET_LINK_ST_REQ:
    return WLAN_HAL_SET_LINK_ST_REQ; 
  case WDI_GET_STATS_REQ:
    return WLAN_HAL_GET_STATS_REQ; 
  case WDI_UPDATE_CFG_REQ:
    return WLAN_HAL_UPDATE_CFG_REQ; 
  case WDI_ADD_BA_SESSION_REQ:
    return WLAN_HAL_ADD_BA_SESSION_REQ;
  case WDI_TRIGGER_BA_REQ:
    return WLAN_HAL_TRIGGER_BA_REQ;
  case WDI_UPD_BCON_PRMS_REQ:
    return WLAN_HAL_UPDATE_BEACON_REQ; 
  case WDI_SND_BCON_REQ:
    return WLAN_HAL_SEND_BEACON_REQ; 
  case WDI_UPD_PROBE_RSP_TEMPLATE_REQ:
    return WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_REQ;
#ifdef WLAN_FEATURE_VOWIFI
   case WDI_SET_MAX_TX_POWER_REQ:
    return WLAN_HAL_SET_MAX_TX_POWER_REQ;
#endif
#ifdef WLAN_FEATURE_P2P
  case WDI_P2P_GO_NOTICE_OF_ABSENCE_REQ:
    return WLAN_HAL_SET_P2P_GONOA_REQ;
#endif
  case WDI_ENTER_IMPS_REQ:
    return WLAN_HAL_ENTER_IMPS_REQ; 
  case WDI_EXIT_IMPS_REQ:
    return WLAN_HAL_EXIT_IMPS_REQ; 
  case WDI_ENTER_BMPS_REQ:
    return WLAN_HAL_ENTER_BMPS_REQ; 
  case WDI_EXIT_BMPS_REQ:
    return WLAN_HAL_EXIT_BMPS_REQ; 
  case WDI_ENTER_UAPSD_REQ:
    return WLAN_HAL_ENTER_UAPSD_REQ; 
  case WDI_EXIT_UAPSD_REQ:
    return WLAN_HAL_EXIT_UAPSD_REQ; 
  case WDI_SET_UAPSD_PARAM_REQ:
    return WLAN_HAL_SET_UAPSD_AC_PARAMS_REQ; 
  case WDI_UPDATE_UAPSD_PARAM_REQ:
    return WLAN_HAL_UPDATE_UAPSD_PARAM_REQ; 
  case WDI_CONFIGURE_RXP_FILTER_REQ:
    return WLAN_HAL_CONFIGURE_RXP_FILTER_REQ; 
  case WDI_SET_BEACON_FILTER_REQ:
    return WLAN_HAL_ADD_BCN_FILTER_REQ; 
  case WDI_REM_BEACON_FILTER_REQ:
    return WLAN_HAL_REM_BCN_FILTER_REQ;
  case WDI_SET_RSSI_THRESHOLDS_REQ:
    return WLAN_HAL_SET_RSSI_THRESH_REQ;
  case WDI_HOST_OFFLOAD_REQ:
    return WLAN_HAL_HOST_OFFLOAD_REQ;
  case WDI_WOWL_ADD_BC_PTRN_REQ:
    return WLAN_HAL_ADD_WOWL_BCAST_PTRN;
  case WDI_WOWL_DEL_BC_PTRN_REQ:
    return WLAN_HAL_DEL_WOWL_BCAST_PTRN;
  case WDI_WOWL_ENTER_REQ:
    return WLAN_HAL_ENTER_WOWL_REQ;
  case WDI_WOWL_EXIT_REQ:
    return WLAN_HAL_EXIT_WOWL_REQ;
  case WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ:
    return WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ;
   case WDI_NV_DOWNLOAD_REQ:
    return WLAN_HAL_DOWNLOAD_NV_REQ;
  case WDI_FLUSH_AC_REQ:
    return WLAN_HAL_TL_HAL_FLUSH_AC_REQ;
  case WDI_BTAMP_EVENT_REQ:
    return WLAN_HAL_SIGNAL_BTAMP_EVENT_REQ;
#ifdef WLAN_FEATURE_VOWIFI_11R
  case WDI_AGGR_ADD_TS_REQ:
     return WLAN_HAL_AGGR_ADD_TS_REQ;
#endif /* WLAN_FEATURE_VOWIFI_11R */
#ifdef ANI_MANF_DIAG
  case WDI_FTM_CMD_REQ:
    return WLAN_HAL_PROCESS_PTT_REQ;
#endif /* ANI_MANF_DIAG */
  default:
    return WLAN_HAL_MSG_MAX; 
  }
  
}/*WDI_2_HAL_REQ_TYPE*/

/*Convert WDI response type into HAL response type*/
WPT_STATIC WPT_INLINE WDI_ResponseEnumType
HAL_2_WDI_RSP_TYPE
(
  tHalHostMsgType halMsg
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  halMsg )
  {
  case WLAN_HAL_START_RSP:
    return WDI_START_RESP;
  case WLAN_HAL_STOP_RSP:
    return WDI_STOP_RESP;
  case WLAN_HAL_INIT_SCAN_RSP:
    return WDI_INIT_SCAN_RESP;
  case WLAN_HAL_START_SCAN_RSP:
    return WDI_START_SCAN_RESP;
  case WLAN_HAL_END_SCAN_RSP:
    return WDI_END_SCAN_RESP;
  case WLAN_HAL_FINISH_SCAN_RSP:
    return WDI_FINISH_SCAN_RESP;
  case WLAN_HAL_CONFIG_STA_RSP:
    return WDI_CONFIG_STA_RESP;
  case WLAN_HAL_DELETE_STA_RSP:
    return WDI_DEL_STA_RESP;
  case WLAN_HAL_CONFIG_BSS_RSP:
    return WDI_CONFIG_BSS_RESP;
  case WLAN_HAL_DELETE_BSS_RSP:
    return WDI_DEL_BSS_RESP;
  case WLAN_HAL_JOIN_RSP:
    return WDI_JOIN_RESP;
  case WLAN_HAL_POST_ASSOC_RSP:
    return WDI_POST_ASSOC_RESP;
  case WLAN_HAL_SET_BSSKEY_RSP:
    return WDI_SET_BSS_KEY_RESP;
  case WLAN_HAL_SET_STAKEY_RSP:
    return WDI_SET_STA_KEY_RESP;
  case WLAN_HAL_RMV_BSSKEY_RSP:
    return WDI_RMV_BSS_KEY_RESP;
  case WLAN_HAL_RMV_STAKEY_RSP:
    return WDI_RMV_STA_KEY_RESP;
  case WLAN_HAL_SET_BCASTKEY_RSP:
    return WDI_SET_STA_BCAST_KEY_RESP;
  //Some conflict in the old code - check this: case WLAN_HAL_RMV_BCASTKEY_RSP:
  //  return WDI_RMV_STA_BCAST_KEY_RESP;
  case WLAN_HAL_ADD_TS_RSP:
    return WDI_ADD_TS_RESP;
  case WLAN_HAL_DEL_TS_RSP:
    return WDI_DEL_TS_RESP;
  case WLAN_HAL_UPD_EDCA_PARAMS_RSP:
    return WDI_UPD_EDCA_PRMS_RESP;
  case WLAN_HAL_ADD_BA_RSP:
    return WDI_ADD_BA_RESP;
  case WLAN_HAL_DEL_BA_RSP:
    return WDI_DEL_BA_RESP;
  case WLAN_HAL_CH_SWITCH_RSP:
    return WDI_CH_SWITCH_RESP;
  case WLAN_HAL_SET_LINK_ST_RSP:
    return WDI_SET_LINK_ST_RESP;
  case WLAN_HAL_GET_STATS_RSP:
    return WDI_GET_STATS_RESP;
  case WLAN_HAL_UPDATE_CFG_RSP:
    return WDI_UPDATE_CFG_RESP;
  case WLAN_HAL_ADD_BA_SESSION_RSP:
    return WDI_ADD_BA_SESSION_RESP;
  case WLAN_HAL_TRIGGER_BA_RSP:
    return WDI_TRIGGER_BA_RESP;
  case WLAN_HAL_UPDATE_BEACON_RSP:
    return WDI_UPD_BCON_PRMS_RESP;
  case WLAN_HAL_SEND_BEACON_RSP:
    return WDI_SND_BCON_RESP;
  case WLAN_HAL_UPDATE_PROBE_RSP_TEMPLATE_RSP:
    return WDI_UPD_PROBE_RSP_TEMPLATE_RESP;
  /*Indications*/
  case WLAN_HAL_RSSI_NOTIFICATION_IND:
    return WDI_HAL_RSSI_NOTIFICATION_IND;
  case WLAN_HAL_MISSED_BEACON_IND:
    return WDI_HAL_MISSED_BEACON_IND;
  case WLAN_HAL_UNKNOWN_ADDR2_FRAME_RX_IND:
    return WDI_HAL_UNKNOWN_ADDR2_FRAME_RX_IND;
  case WLAN_HAL_MIC_FAILURE_IND:
    return WDI_HAL_MIC_FAILURE_IND;
  case WLAN_HAL_FATAL_ERROR_IND:
    return WDI_HAL_FATAL_ERROR_IND;
  case WLAN_HAL_DELETE_STA_CONTEXT_IND:
    return WDI_HAL_DEL_STA_IND;
#ifdef WLAN_FEATURE_VOWIFI
  case WLAN_HAL_SET_MAX_TX_POWER_RSP:
    return WDI_SET_MAX_TX_POWER_RESP;
#endif
#ifdef WLAN_FEATURE_P2P
  case WLAN_HAL_SET_P2P_GONOA_RSP:
    return WDI_P2P_GO_NOTICE_OF_ABSENCE_RESP;
#endif
  case WLAN_HAL_ENTER_IMPS_RSP:
    return WDI_ENTER_IMPS_RESP; 
  case WLAN_HAL_EXIT_IMPS_RSP:
    return WDI_EXIT_IMPS_RESP; 
  case WLAN_HAL_ENTER_BMPS_RSP:
    return WDI_ENTER_BMPS_RESP; 
  case WLAN_HAL_EXIT_BMPS_RSP:
    return WDI_EXIT_BMPS_RESP; 
  case WLAN_HAL_ENTER_UAPSD_RSP:
    return WDI_ENTER_UAPSD_RESP; 
  case WLAN_HAL_EXIT_UAPSD_RSP:
    return WDI_EXIT_UAPSD_RESP; 
  case WLAN_HAL_SET_UAPSD_AC_PARAMS_RSP:
    return WDI_SET_UAPSD_PARAM_RESP; 
  case WLAN_HAL_UPDATE_UAPSD_PARAM_RSP:
    return WDI_UPDATE_UAPSD_PARAM_RESP; 
  case WLAN_HAL_CONFIGURE_RXP_FILTER_RSP:
    return WDI_CONFIGURE_RXP_FILTER_RESP; 
  case WLAN_HAL_ADD_BCN_FILTER_RSP:
    return WDI_SET_BEACON_FILTER_RESP;
  case WLAN_HAL_REM_BCN_FILTER_RSP:
    return WDI_REM_BEACON_FILTER_RESP;
  case WLAN_HAL_SET_RSSI_THRESH_RSP:
    return WDI_SET_RSSI_THRESHOLDS_RESP;
  case WLAN_HAL_HOST_OFFLOAD_RSP:
    return WDI_HOST_OFFLOAD_RESP;
  case WLAN_HAL_ADD_WOWL_BCAST_PTRN_RSP:
    return WDI_WOWL_ADD_BC_PTRN_RESP;
  case WLAN_HAL_DEL_WOWL_BCAST_PTRN_RSP:
    return WDI_WOWL_DEL_BC_PTRN_RESP;
  case WLAN_HAL_ENTER_WOWL_RSP:
    return WDI_WOWL_ENTER_RESP;
  case WLAN_HAL_EXIT_WOWL_RSP:
    return WDI_WOWL_EXIT_RESP;
  case WLAN_HAL_CONFIGURE_APPS_CPU_WAKEUP_STATE_RSP:
    return WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_RESP;
  case WLAN_HAL_DOWNLOAD_NV_RSP:
    return WDI_NV_DOWNLOAD_RESP;
  case WLAN_HAL_TL_HAL_FLUSH_AC_RSP:
    return WDI_FLUSH_AC_RESP;
  case WLAN_HAL_SIGNAL_BTAMP_EVENT_RSP:
    return WDI_BTAMP_EVENT_RESP;
#ifdef ANI_MANF_DIAG
  case WLAN_HAL_PROCESS_PTT_RSP:
    return  WDI_FTM_CMD_RESP;
#endif /* ANI_MANF_DIAG */
  default:
    return eDRIVER_TYPE_MAX; 
  }

}/*HAL_2_WDI_RSP_TYPE*/


/*Convert WDI driver type into HAL driver type*/
WPT_STATIC WPT_INLINE tDriverType
WDI_2_HAL_DRV_TYPE
(
  WDI_DriverType wdiDriverType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiDriverType )
  {
  case WDI_DRIVER_TYPE_PRODUCTION:
    return eDRIVER_TYPE_PRODUCTION;
  case WDI_DRIVER_TYPE_MFG:
    return eDRIVER_TYPE_MFG;
  case WDI_DRIVER_TYPE_DVT:
    return eDRIVER_TYPE_DVT;
  }

  return eDRIVER_TYPE_MAX; 
}/*WDI_2_HAL_DRV_TYPE*/


/*Convert WDI stop reason into HAL stop reason*/
WPT_STATIC WPT_INLINE tHalStopType
WDI_2_HAL_STOP_REASON
(
  WDI_StopType wdiDriverType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiDriverType )
  {
  case WDI_STOP_TYPE_SYS_RESET:
    return HAL_STOP_TYPE_SYS_RESET;
  case WDI_DRIVER_TYPE_MFG:
    return WDI_STOP_TYPE_SYS_DEEP_SLEEP;
  case WDI_STOP_TYPE_RF_KILL:
    return HAL_STOP_TYPE_RF_KILL;
  }

  return HAL_STOP_TYPE_MAX; 
}/*WDI_2_HAL_STOP_REASON*/


/*Convert WDI scan mode type into HAL scan mode type*/
WPT_STATIC WPT_INLINE eHalSysMode
WDI_2_HAL_SCAN_MODE
(
  WDI_ScanMode wdiScanMode
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiScanMode )
  {
  case WDI_SCAN_MODE_NORMAL:
    return eHAL_SYS_MODE_NORMAL;
  case WDI_SCAN_MODE_LEARN:
    return eHAL_SYS_MODE_LEARN;
  case WDI_SCAN_MODE_SCAN:
    return eHAL_SYS_MODE_SCAN;
  case WDI_SCAN_MODE_PROMISC:
    return eHAL_SYS_MODE_PROMISC; 
  }

  return eHAL_SYS_MODE_MAX; 
}/*WDI_2_HAL_SCAN_MODE*/

/*Convert WDI sec ch offset into HAL sec ch offset type*/
WPT_STATIC WPT_INLINE tSirMacHTSecondaryChannelOffset
WDI_2_HAL_SEC_CH_OFFSET
(
  WDI_HTSecondaryChannelOffset wdiSecChOffset
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiSecChOffset )
  {
  case WDI_SECONDARY_CHANNEL_OFFSET_NONE:
    return eHT_SECONDARY_CHANNEL_OFFSET_NONE;
  case WDI_SECONDARY_CHANNEL_OFFSET_UP:
    return eHT_SECONDARY_CHANNEL_OFFSET_UP;
  case WDI_SECONDARY_CHANNEL_OFFSET_DOWN:
    return eHT_SECONDARY_CHANNEL_OFFSET_DOWN;
  }

  return eHT_SECONDARY_CHANNEL_OFFSET_MAX; 
}/*WDI_2_HAL_SEC_CH_OFFSET*/

/*Convert WDI BSS type into HAL BSS type*/
WPT_STATIC WPT_INLINE tSirBssType
WDI_2_HAL_BSS_TYPE
(
  WDI_BssType wdiBSSType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiBSSType )
  {
  case WDI_INFRASTRUCTURE_MODE:
    return eSIR_INFRASTRUCTURE_MODE;
  case WDI_INFRA_AP_MODE:
    return eSIR_INFRA_AP_MODE;
  case WDI_IBSS_MODE:
    return eSIR_IBSS_MODE;
  case WDI_BTAMP_STA_MODE:
    return eSIR_BTAMP_STA_MODE;
  case WDI_BTAMP_AP_MODE:
    return eSIR_BTAMP_AP_MODE; 
  case WDI_BSS_AUTO_MODE:
    return eSIR_AUTO_MODE;
  }

  return eSIR_DONOT_USE_BSS_TYPE; 
}/*WDI_2_HAL_BSS_TYPE*/

/*Convert WDI NW type into HAL NW type*/
WPT_STATIC WPT_INLINE tSirNwType
WDI_2_HAL_NW_TYPE
(
  WDI_NwType wdiNWType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch(  wdiNWType )
  {
  case WDI_11A_NW_TYPE:
    return eSIR_11A_NW_TYPE;
  case WDI_11B_NW_TYPE:
    return eSIR_11B_NW_TYPE;
  case WDI_11G_NW_TYPE:
    return eSIR_11G_NW_TYPE;
  case WDI_11N_NW_TYPE:
    return eSIR_11N_NW_TYPE;
  }

  return eSIR_DONOT_USE_NW_TYPE; 
}/*WDI_2_HAL_NW_TYPE*/

/*Convert WDI chanel bonding type into HAL cb type*/
WPT_STATIC WPT_INLINE ePhyChanBondState
WDI_2_HAL_CB_STATE
(
  WDI_PhyChanBondState wdiCbState
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiCbState )
  {
  case WDI_PHY_SINGLE_CHANNEL_CENTERED:
    return PHY_SINGLE_CHANNEL_CENTERED;
  case WDI_PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
    return PHY_DOUBLE_CHANNEL_LOW_PRIMARY;
  case WDI_PHY_DOUBLE_CHANNEL_CENTERED:
    return PHY_DOUBLE_CHANNEL_CENTERED;
  case WDI_PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
    return PHY_DOUBLE_CHANNEL_HIGH_PRIMARY;
  }
  
  return PHY_CHANNEL_BONDING_STATE_MAX;
}/*WDI_2_HAL_CB_STATE*/

/*Convert WDI chanel bonding type into HAL cb type*/
WPT_STATIC WPT_INLINE tSirMacHTOperatingMode
WDI_2_HAL_HT_OPER_MODE
(
  WDI_HTOperatingMode wdiHTOperMode
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiHTOperMode )
  {
  case WDI_HT_OP_MODE_PURE:
    return eSIR_HT_OP_MODE_PURE;
  case WDI_HT_OP_MODE_OVERLAP_LEGACY:
    return eSIR_HT_OP_MODE_OVERLAP_LEGACY;
  case WDI_HT_OP_MODE_NO_LEGACY_20MHZ_HT:
    return eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT;
  case WDI_HT_OP_MODE_MIXED:
    return eSIR_HT_OP_MODE_MIXED;
  }
  
  return eSIR_HT_OP_MODE_MAX;
}/*WDI_2_HAL_HT_OPER_MODE*/

/*Convert WDI mimo PS type into HAL mimo PS type*/
WPT_STATIC WPT_INLINE tSirMacHTMIMOPowerSaveState
WDI_2_HAL_MIMO_PS
(
  WDI_HTMIMOPowerSaveState wdiHTOperMode
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiHTOperMode )
  {
  case WDI_HT_MIMO_PS_STATIC:
    return eSIR_HT_MIMO_PS_STATIC;
  case WDI_HT_MIMO_PS_DYNAMIC:
    return eSIR_HT_MIMO_PS_DYNAMIC;
  case WDI_HT_MIMO_PS_NA:
    return eSIR_HT_MIMO_PS_NA;
  case WDI_HT_MIMO_PS_NO_LIMIT:
    return eSIR_HT_MIMO_PS_NO_LIMIT;
  }
  
  return eSIR_HT_MIMO_PS_MAX;
}/*WDI_2_HAL_MIMO_PS*/

/*Convert WDI ENC type into HAL ENC type*/
WPT_STATIC WPT_INLINE tAniEdType
WDI_2_HAL_ENC_TYPE
(
  WDI_EncryptType wdiEncType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiEncType )
  {
  case WDI_ENCR_NONE:
    return eSIR_ED_NONE;

  case WDI_ENCR_WEP40:
    return eSIR_ED_WEP40;

  case WDI_ENCR_WEP104:
    return eSIR_ED_WEP104;

  case WDI_ENCR_TKIP:
    return eSIR_ED_TKIP;

  case WDI_ENCR_CCMP:
    return eSIR_ED_CCMP;

  case WDI_ENCR_AES_128_CMAC:
    return eSIR_ED_AES_128_CMAC;
#if defined(FEATURE_WLAN_WAPI)
  case WDI_ENCR_WPI:
    return eSIR_ED_WPI;
#endif
  default:
    return eSIR_ED_NOT_IMPLEMENTED;
  }

}/*WDI_2_HAL_ENC_TYPE*/

/*Convert WDI WEP type into HAL WEP type*/
WPT_STATIC WPT_INLINE tAniWepType
WDI_2_HAL_WEP_TYPE
(
  WDI_WepType  wdiWEPType
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiWEPType )
  {
  case WDI_WEP_STATIC:
    return eSIR_WEP_STATIC;

  case WDI_WEP_DYNAMIC:
    return eSIR_WEP_DYNAMIC;
  }
  
  return eSIR_WEP_MAX;
}/*WDI_2_HAL_WEP_TYPE*/

WPT_STATIC WPT_INLINE tSirLinkState
WDI_2_HAL_LINK_STATE
(
  WDI_LinkStateType  wdiLinkState
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/
  switch ( wdiLinkState )
  {
  case WDI_LINK_IDLE_STATE:
    return eSIR_LINK_IDLE_STATE;

  case WDI_LINK_PREASSOC_STATE:
    return eSIR_LINK_PREASSOC_STATE;

  case WDI_LINK_POSTASSOC_STATE:
    return eSIR_LINK_POSTASSOC_STATE;

  case WDI_LINK_AP_STATE:
    return eSIR_LINK_AP_STATE;

  case WDI_LINK_IBSS_STATE:
    return eSIR_LINK_IBSS_STATE;

  case WDI_LINK_BTAMP_PREASSOC_STATE:
    return eSIR_LINK_BTAMP_PREASSOC_STATE;

  case WDI_LINK_BTAMP_POSTASSOC_STATE:
    return eSIR_LINK_BTAMP_POSTASSOC_STATE;

  case WDI_LINK_BTAMP_AP_STATE:
    return eSIR_LINK_BTAMP_AP_STATE;

  case WDI_LINK_BTAMP_STA_STATE:
    return eSIR_LINK_BTAMP_STA_STATE;

  case WDI_LINK_LEARN_STATE:
    return eSIR_LINK_LEARN_STATE;

  case WDI_LINK_SCAN_STATE:
    return eSIR_LINK_SCAN_STATE;

  case WDI_LINK_FINISH_SCAN_STATE:
    return eSIR_LINK_FINISH_SCAN_STATE;

  case WDI_LINK_INIT_CAL_STATE:
    return eSIR_LINK_INIT_CAL_STATE;

  case WDI_LINK_FINISH_CAL_STATE:
    return eSIR_LINK_FINISH_CAL_STATE;

#ifdef WLAN_FEATURE_P2P
  case WDI_LINK_LISTEN_STATE:
    return eSIR_LINK_LISTEN_STATE;
#endif

  default:
    return eSIR_LINK_MAX;
  }  
}

/*Translate a STA Context from WDI into HAL*/ 
WPT_STATIC WPT_INLINE 
void
WDI_CopyWDIStaCtxToHALStaCtx
( 
  tConfigStaParams*          phalConfigSta,
  WDI_ConfigStaReqInfoType*  pwdiConfigSta
)
{
   wpt_uint8 i;
   /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/

  wpalMemoryCopy(phalConfigSta->bssId, 
                  pwdiConfigSta->macBSSID, WDI_MAC_ADDR_LEN); 
  
  wpalMemoryCopy(phalConfigSta->staMac, 
                  pwdiConfigSta->macSTA, WDI_MAC_ADDR_LEN); 

  phalConfigSta->assocId                 = pwdiConfigSta->usAssocId;
  phalConfigSta->staType                 = pwdiConfigSta->wdiSTAType;
  phalConfigSta->shortPreambleSupported  = pwdiConfigSta->ucShortPreambleSupported;
  phalConfigSta->listenInterval          = pwdiConfigSta->usListenInterval;
  phalConfigSta->wmmEnabled              = pwdiConfigSta->ucWMMEnabled;
  phalConfigSta->htCapable               = pwdiConfigSta->ucHTCapable;
  phalConfigSta->txChannelWidthSet       = pwdiConfigSta->ucTXChannelWidthSet;
  phalConfigSta->rifsMode                = pwdiConfigSta->ucRIFSMode;
  phalConfigSta->lsigTxopProtection      = pwdiConfigSta->ucLSIGTxopProtection;
  phalConfigSta->maxAmpduSize            = pwdiConfigSta->ucMaxAmpduSize;
  phalConfigSta->maxAmpduDensity         = pwdiConfigSta->ucMaxAmpduDensity;
  phalConfigSta->maxAmsduSize            = pwdiConfigSta->ucMaxAmsduSize;
  phalConfigSta->fShortGI40Mhz           = pwdiConfigSta->ucShortGI40Mhz;
  phalConfigSta->fShortGI20Mhz           = pwdiConfigSta->ucShortGI20Mhz;
  phalConfigSta->rmfEnabled              = pwdiConfigSta->ucRMFEnabled;
  phalConfigSta->action                  = pwdiConfigSta->wdiAction;
  phalConfigSta->uAPSD                   = pwdiConfigSta->ucAPSD;
  phalConfigSta->maxSPLen                = pwdiConfigSta->ucMaxSPLen;
  phalConfigSta->greenFieldCapable       = pwdiConfigSta->ucGreenFieldCapable;
  phalConfigSta->delayedBASupport        = pwdiConfigSta->ucDelayedBASupport;
  phalConfigSta->us32MaxAmpduDuration    = pwdiConfigSta->us32MaxAmpduDuratio;
  phalConfigSta->fDsssCckMode40Mhz       = pwdiConfigSta->ucDsssCckMode40Mhz;
  
  phalConfigSta->mimoPS = WDI_2_HAL_MIMO_PS(pwdiConfigSta->wdiMIMOPS);

  phalConfigSta->supportedRates.opRateMode = 
                          pwdiConfigSta->wdiSupportedRates.opRateMode;
  for(i = 0; i < SIR_NUM_11B_RATES; i ++)
  {
     phalConfigSta->supportedRates.llbRates[i] = 
                          pwdiConfigSta->wdiSupportedRates.llbRates[i];
  }
  for(i = 0; i < SIR_NUM_11A_RATES; i ++)
  {
     phalConfigSta->supportedRates.llaRates[i] = 
                          pwdiConfigSta->wdiSupportedRates.llaRates[i];
  }
  for(i = 0; i < SIR_NUM_POLARIS_RATES; i ++)
  {
     phalConfigSta->supportedRates.aniLegacyRates[i] =
                          pwdiConfigSta->wdiSupportedRates.aLegacyRates[i];
  }
  phalConfigSta->supportedRates.aniEnhancedRateBitmap = 
                          pwdiConfigSta->wdiSupportedRates.uEnhancedRateBitmap;
  for(i = 0; i < SIR_MAC_MAX_SUPPORTED_MCS_SET; i ++)
  {
     phalConfigSta->supportedRates.supportedMCSSet[i] = 
                          pwdiConfigSta->wdiSupportedRates.aSupportedMCSSet[i];
  }
  phalConfigSta->supportedRates.rxHighestDataRate =
                          pwdiConfigSta->wdiSupportedRates.aRxHighestDataRate;

}/*WDI_CopyWDIStaCtxToHALStaCtx*/;
 
/*Translate a Rate set info from WDI into HAL*/ 
WPT_STATIC WPT_INLINE void 
WDI_CopyWDIRateSetToHALRateSet
( 
  tSirMacRateSet* pHalRateSet,
  WDI_RateSet*	  pwdiRateSet
)
{
  wpt_uint8 i; 
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  pHalRateSet->numRates = ( pwdiRateSet->ucNumRates <= SIR_MAC_RATESET_EID_MAX )?
                            pwdiRateSet->ucNumRates:SIR_MAC_RATESET_EID_MAX;

  for ( i = 0; i < pHalRateSet->numRates; i++ )
  {
    pHalRateSet->rate[i] = pwdiRateSet->aRates[i];
  }
  
}/*WDI_CopyWDIRateSetToHALRateSet*/


/*Translate an EDCA Parameter Record from WDI into HAL*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIEDCAParamsToHALEDCAParams
( 
  tSirMacEdcaParamRecord* phalEdcaParam,
  WDI_EdcaParamRecord*    pWDIEdcaParam
)
{
  /*Lightweight function - no sanity checks and no unecessary code to increase 
    the chances of getting inlined*/

  phalEdcaParam->aci.rsvd   = pWDIEdcaParam->wdiACI.rsvd;
  phalEdcaParam->aci.aci    = pWDIEdcaParam->wdiACI.aci;
  phalEdcaParam->aci.acm    = pWDIEdcaParam->wdiACI.acm;
  phalEdcaParam->aci.aifsn  = pWDIEdcaParam->wdiACI.aifsn;

  phalEdcaParam->cw.max     = pWDIEdcaParam->wdiCW.max;
  phalEdcaParam->cw.min     = pWDIEdcaParam->wdiCW.min;
  phalEdcaParam->txoplimit  = pWDIEdcaParam->usTXOPLimit;
}/*WDI_CopyWDIEDCAParamsToHALEDCAParams*/


/*Copy a management frame header from WDI fmt into HAL fmt*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr
(
  tSirMacMgmtHdr* pmacMgmtHdr,
  WDI_MacMgmtHdr* pwdiMacMgmtHdr
)
{
  pmacMgmtHdr->fc.protVer    =  pwdiMacMgmtHdr->fc.protVer;
  pmacMgmtHdr->fc.type       =  pwdiMacMgmtHdr->fc.type;
  pmacMgmtHdr->fc.subType    =  pwdiMacMgmtHdr->fc.subType;
  pmacMgmtHdr->fc.toDS       =  pwdiMacMgmtHdr->fc.toDS;
  pmacMgmtHdr->fc.fromDS     =  pwdiMacMgmtHdr->fc.fromDS;
  pmacMgmtHdr->fc.moreFrag   =  pwdiMacMgmtHdr->fc.moreFrag;
  pmacMgmtHdr->fc.retry      =  pwdiMacMgmtHdr->fc.retry;
  pmacMgmtHdr->fc.powerMgmt  =  pwdiMacMgmtHdr->fc.powerMgmt;
  pmacMgmtHdr->fc.moreData   =  pwdiMacMgmtHdr->fc.moreData;
  pmacMgmtHdr->fc.wep        =  pwdiMacMgmtHdr->fc.wep;
  pmacMgmtHdr->fc.order      =  pwdiMacMgmtHdr->fc.order;

  pmacMgmtHdr->durationLo =  pwdiMacMgmtHdr->durationLo;
  pmacMgmtHdr->durationHi =  pwdiMacMgmtHdr->durationHi;

  wpalMemoryCopy(pmacMgmtHdr->da, 
                 pwdiMacMgmtHdr->da, 6);
  wpalMemoryCopy(pmacMgmtHdr->sa, 
                 pwdiMacMgmtHdr->sa, 6);
  wpalMemoryCopy(pmacMgmtHdr->bssId, 
                 pwdiMacMgmtHdr->bssId, 6);

  pmacMgmtHdr->seqControl.fragNum  =  pwdiMacMgmtHdr->seqControl.fragNum;
  pmacMgmtHdr->seqControl.seqNumLo =  pwdiMacMgmtHdr->seqControl.seqNumLo;
  pmacMgmtHdr->seqControl.seqNumHi =  pwdiMacMgmtHdr->seqControl.seqNumHi;

}/*WDI_CopyWDIMgmFrameHdrToHALMgmFrameHdr*/


/*Copy config bss parameters from WDI fmt into HAL fmt*/
WPT_STATIC WPT_INLINE void
WDI_CopyWDIConfigBSSToHALConfigBSS
(
  tConfigBssParams*         phalConfigBSS,
  WDI_ConfigBSSReqInfoType* pwdiConfigBSS
)
{
  wpalMemoryCopy( phalConfigBSS->bssId,
                  pwdiConfigBSS->macBSSID,
                  WDI_MAC_ADDR_LEN);

#ifdef HAL_SELF_STA_PER_BSS
  wpalMemoryCopy( phalConfigBSS->selfMacAddr,
                  pwdiConfigBSS->macSelfAddr,
                  WDI_MAC_ADDR_LEN);
#endif

  phalConfigBSS->bssType  = WDI_2_HAL_BSS_TYPE(pwdiConfigBSS->wdiBSSType);

  phalConfigBSS->operMode = pwdiConfigBSS->ucOperMode;
  phalConfigBSS->nwType   = WDI_2_HAL_NW_TYPE(pwdiConfigBSS->wdiNWType);

  phalConfigBSS->shortSlotTimeSupported = 
     pwdiConfigBSS->ucShortSlotTimeSupported;
  phalConfigBSS->llaCoexist         = pwdiConfigBSS->ucllaCoexist;
  phalConfigBSS->llbCoexist         = pwdiConfigBSS->ucllbCoexist;
  phalConfigBSS->llgCoexist         = pwdiConfigBSS->ucllgCoexist;
  phalConfigBSS->ht20Coexist        = pwdiConfigBSS->ucHT20Coexist;
  phalConfigBSS->llnNonGFCoexist    = pwdiConfigBSS->ucllnNonGFCoexist;
  phalConfigBSS->fLsigTXOPProtectionFullSupport = 
    pwdiConfigBSS->ucTXOPProtectionFullSupport;
  phalConfigBSS->fRIFSMode          = pwdiConfigBSS->ucRIFSMode;
  phalConfigBSS->beaconInterval     = pwdiConfigBSS->usBeaconInterval;
  phalConfigBSS->dtimPeriod         = pwdiConfigBSS->ucDTIMPeriod;
  phalConfigBSS->txChannelWidthSet  = pwdiConfigBSS->ucTXChannelWidthSet;
  phalConfigBSS->currentOperChannel = pwdiConfigBSS->ucCurrentOperChannel;
  phalConfigBSS->currentExtChannel  = pwdiConfigBSS->ucCurrentExtChannel;
  phalConfigBSS->action             = pwdiConfigBSS->wdiAction;
  phalConfigBSS->htCapable          = pwdiConfigBSS->ucHTCapable;
  phalConfigBSS->rmfEnabled         = pwdiConfigBSS->ucRMFEnabled;

  phalConfigBSS->htOperMode = 
    WDI_2_HAL_HT_OPER_MODE(pwdiConfigBSS->wdiHTOperMod); 

  phalConfigBSS->dualCTSProtection        = pwdiConfigBSS->ucDualCTSProtection;
  phalConfigBSS->ucMaxProbeRespRetryLimit = pwdiConfigBSS->ucMaxProbeRespRetryLimit;
  phalConfigBSS->bHiddenSSIDEn            = pwdiConfigBSS->bHiddenSSIDEn;
  phalConfigBSS->bProxyProbeRespEn        = pwdiConfigBSS->bProxyProbeRespEn;

#ifdef WLAN_FEATURE_VOWIFI
  phalConfigBSS->maxTxPower               = pwdiConfigBSS->cMaxTxPower;
#endif

  /*! Used 32 as magic number because that is how the ssid is declared inside the
   hal header - hal needs a macro for it */
  phalConfigBSS->ssId.length = 
    (pwdiConfigBSS->wdiSSID.ucLength <= 32)?
    pwdiConfigBSS->wdiSSID.ucLength : 32;
  wpalMemoryCopy(phalConfigBSS->ssId.ssId,
                 pwdiConfigBSS->wdiSSID.sSSID, 
                 phalConfigBSS->ssId.length); 

  WDI_CopyWDIStaCtxToHALStaCtx( &phalConfigBSS->staContext,
                                &pwdiConfigBSS->wdiSTAContext);
  
  WDI_CopyWDIRateSetToHALRateSet( &phalConfigBSS->rateSet,
                                  &pwdiConfigBSS->wdiRateSet);

  phalConfigBSS->edcaParamsValid = pwdiConfigBSS->ucEDCAParamsValid;

  if(phalConfigBSS->edcaParamsValid)
  {
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acbe,
                                        &pwdiConfigBSS->wdiBEEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acbk,
                                           &pwdiConfigBSS->wdiBKEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acvi,
                                           &pwdiConfigBSS->wdiVIEDCAParams);
     WDI_CopyWDIEDCAParamsToHALEDCAParams( &phalConfigBSS->acvo,
                                           &pwdiConfigBSS->wdiVOEDCAParams);
  }

}/*WDI_CopyWDIConfigBSSToHALConfigBSS*/


/*Extract the request CB function and user data from a request structure 
  pointed to by user data */
WPT_STATIC WPT_INLINE void
WDI_ExtractRequestCBFromEvent
(
  WDI_EventInfoType* pEvent,
  WDI_ReqStatusCb*   ppfnReqCB, 
  void**             ppUserData
)
{
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  switch ( pEvent->wdiRequest )
  {
  case WDI_START_REQ:
    *ppfnReqCB   =  ((WDI_StartReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StartReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_STOP_REQ:
    *ppfnReqCB   =  ((WDI_StopReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StopReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_INIT_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_InitScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_InitScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_START_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_StartScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_StartScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_END_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_EndScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EndScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_FINISH_SCAN_REQ:
    *ppfnReqCB   =  ((WDI_FinishScanReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_FinishScanReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_JOIN_REQ:
    *ppfnReqCB   =  ((WDI_JoinReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_JoinReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_CONFIG_BSS_REQ:
    *ppfnReqCB   =  ((WDI_ConfigBSSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigBSSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_BSS_REQ:
    *ppfnReqCB   =  ((WDI_DelBSSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelBSSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_POST_ASSOC_REQ:
    *ppfnReqCB   =  ((WDI_PostAssocReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_PostAssocReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_STA_REQ:
    *ppfnReqCB   =  ((WDI_DelSTAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelSTAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_BSS_KEY_REQ:
    *ppfnReqCB   =  ((WDI_SetBSSKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetBSSKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_RMV_BSS_KEY_REQ:
    *ppfnReqCB   =  ((WDI_RemoveBSSKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemoveBSSKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_STA_KEY_REQ:
    *ppfnReqCB   =  ((WDI_SetSTAKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetSTAKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_RMV_STA_KEY_REQ:
    *ppfnReqCB   =  ((WDI_RemoveSTAKeyReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemoveSTAKeyReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_TS_REQ:
    *ppfnReqCB   =  ((WDI_AddTSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddTSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_TS_REQ:
    *ppfnReqCB   =  ((WDI_DelTSReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelTSReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_UPD_EDCA_PRMS_REQ:
    *ppfnReqCB   =  ((WDI_UpdateEDCAParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateEDCAParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_BA_SESSION_REQ:
    *ppfnReqCB   =  ((WDI_AddBASessionReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddBASessionReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_DEL_BA_REQ:
    *ppfnReqCB   =  ((WDI_DelBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_DelBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_CH_SWITCH_REQ:
    *ppfnReqCB   =  ((WDI_SwitchChReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SwitchChReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_CONFIG_STA_REQ:
    *ppfnReqCB   =  ((WDI_ConfigSTAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigSTAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_SET_LINK_ST_REQ:
    *ppfnReqCB   =  ((WDI_SetLinkReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetLinkReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_GET_STATS_REQ:
    *ppfnReqCB   =  ((WDI_GetStatsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_GetStatsReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_UPDATE_CFG_REQ:
    *ppfnReqCB   =  ((WDI_UpdateCfgReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateCfgReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_ADD_BA_REQ:
    *ppfnReqCB   =  ((WDI_AddBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_AddBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_TRIGGER_BA_REQ:
    *ppfnReqCB   =  ((WDI_TriggerBAReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_TriggerBAReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case  WDI_UPD_BCON_PRMS_REQ:
    *ppfnReqCB   =  ((WDI_UpdateBeaconParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateBeaconParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_SND_BCON_REQ:
    *ppfnReqCB   =  ((WDI_SendBeaconParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SendBeaconParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_ENTER_BMPS_REQ:
    *ppfnReqCB   =  ((WDI_EnterBmpsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EnterBmpsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_EXIT_BMPS_REQ:
    *ppfnReqCB   =  ((WDI_ExitBmpsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ExitBmpsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_ENTER_UAPSD_REQ:
    *ppfnReqCB   =  ((WDI_EnterUapsdReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_EnterUapsdReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_UPDATE_UAPSD_PARAM_REQ:
    *ppfnReqCB   =  ((WDI_UpdateUapsdReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_UpdateUapsdReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_CONFIGURE_RXP_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_ConfigureRxpFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigureRxpFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_SET_BEACON_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_BeaconFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_BeaconFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_REM_BEACON_FILTER_REQ:
    *ppfnReqCB   =  ((WDI_RemBeaconFilterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_RemBeaconFilterReqParamsType*)pEvent->pEventData)->pUserData;
     break; 
  case  WDI_SET_RSSI_THRESHOLDS_REQ:
    *ppfnReqCB   =  ((WDI_SetRSSIThresholdsReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_SetRSSIThresholdsReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_HOST_OFFLOAD_REQ:
    *ppfnReqCB   =  ((WDI_HostOffloadReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_HostOffloadReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_ADD_BC_PTRN_REQ:
    *ppfnReqCB   =  ((WDI_WowlAddBcPtrnReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlAddBcPtrnReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_DEL_BC_PTRN_REQ:
    *ppfnReqCB   =  ((WDI_WowlDelBcPtrnReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlDelBcPtrnReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_WOWL_ENTER_REQ:
    *ppfnReqCB   =  ((WDI_WowlEnterReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_WowlEnterReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case  WDI_CONFIGURE_APPS_CPU_WAKEUP_STATE_REQ:
    *ppfnReqCB   =  ((WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_ConfigureAppsCpuWakeupStateReqParamsType*)pEvent->pEventData)->pUserData;
     break;
  case WDI_FLUSH_AC_REQ:
    *ppfnReqCB   =  ((WDI_FlushAcReqParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_FlushAcReqParamsType*)pEvent->pEventData)->pUserData;
    break;
  case WDI_BTAMP_EVENT_REQ:
    *ppfnReqCB   =  ((WDI_BtAmpEventParamsType*)pEvent->pEventData)->wdiReqStatusCB;
    *ppUserData  =  ((WDI_BtAmpEventParamsType*)pEvent->pEventData)->pUserData;
    break;
  default:
    *ppfnReqCB   =  NULL;
    *ppUserData  =  NULL;
      break; 
  }
}/*WDI_ExtractRequestCBFromEvent*/


/**
 @brief WDI_IsHwFrameTxTranslationCapable checks to see if HW 
        frame xtl is enabled for a particular STA.

 WDI_PostAssocReq must have been called.

 @param uSTAIdx: STA index 
  
 @see WDI_PostAssocReq
 @return Result of the function call
*/
wpt_boolean 
WDI_IsHwFrameTxTranslationCapable
(
  wpt_uint8 uSTAIdx
)
{
  /*!! FIX ME - this must eventually be per station - for now just feedback 
    uma value*/
  /*------------------------------------------------------------------------
    Sanity Check 
  ------------------------------------------------------------------------*/
  if ( eWLAN_PAL_FALSE == gWDIInitialized )
  {
    WPAL_TRACE(eWLAN_MODULE_DAL_CTRL, eWLAN_PAL_TRACE_LEVEL_ERROR,
              "WDI API call before module is initialized - Fail request");

    return WDI_STATUS_E_NOT_ALLOWED; 
  }

  
  return gWDICb.bFrameTransEnabled;
}/*WDI_IsHwFrameTxTranslationCapable*/
