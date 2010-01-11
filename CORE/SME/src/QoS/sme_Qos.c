


/**=========================================================================
  
  \file  sme_Qos.c
  
  \brief implementation for SME QoS APIs
  
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/

#include "halInternal.h"
#include "smeInside.h"

#include "vos_diag_core_event.h"
#include "vos_diag_core_log.h"

#define SME_QOS_MAX_FLOW_ID 0xFFFFFFFF
/* TODO : 6Mbps as Cisco APs seem to like only this value; analysis req.   */
#define SME_QOS_MIN_PHY_RATE         0x5B8D80    

#define SME_QOS_SURPLUS_BW_ALLOWANCE  0x2000     /* Ratio of 1.0           */

/*---------------------------------------------------------------------------
  Max values to bound tspec params against and avoid rollover
---------------------------------------------------------------------------*/
#define SME_QOS_32BIT_MAX  0xFFFFFFFF
#define SME_QOS_16BIT_MAX  0xFFFF

#define SME_QOS_16BIT_MSB  0x8000

/*---------------------------------------------------------------------------
  Adds y to x, but saturates at 32-bit max to avoid rollover
---------------------------------------------------------------------------*/
#define SME_QOS_BOUNDED_U32_ADD_Y_TO_X( _x, _y )                            \
  do                                                                        \
  {                                                                         \
    (_x) = ( (SME_QOS_32BIT_MAX-(_x))<(_y) ) ?                              \
    (SME_QOS_32BIT_MAX) : (_x)+(_y);                                        \
  } while(0)

/*---------------------------------------------------------------------------
  As per WMM, the dialog token is a non-zero value chosen by sender. Since
  we use our flow index, which starts with a 0, we simply choose an
  arbitrary offset to make it non-zero
---------------------------------------------------------------------------*/
#define SME_QOS_TSPEC_DLG_TKN_OFFSET    10

/*---------------------------------------------------------------------------
  As per WMM spec there could be max 2 TSPEC running on the same AC with 
  different direction. We will refer each TSPEC with an index
---------------------------------------------------------------------------*/
#define SME_QOS_TSPEC_INDEX_0            0
#define SME_QOS_TSPEC_INDEX_1            1
#define SME_QOS_TSPEC_INDEX_MAX          2
#define SME_QOS_TSPEC_MASK_BIT_1_SET     1
#define SME_QOS_TSPEC_MASK_BIT_2_SET     2
#define SME_QOS_TSPEC_MASK_BIT_1_2_SET   3
#define SME_QOS_TSPEC_MASK_CLEAR         0


//allowing at max 10 flows per ac per tspec index to run at a time
#define SME_QOS_NUM_FLOW_ID_PER_AC       10

//which key to search on, in the flowlist (1 = flowID, 2 = AC, 4 = reason)
#define SME_QOS_SEARCH_KEY_INDEX_1       1
#define SME_QOS_SEARCH_KEY_INDEX_2       2
#define SME_QOS_SEARCH_KEY_INDEX_3       4

#define SME_QOS_AP_SUPPORTS_APSD         0x80

#define SME_QOS_ACCESS_POLICY_EDCA       1

//allowing at max 10 APSD request pending from PMC
#define SME_QOS_MAX_APSD_REQ_PENDING     10

#define SME_QOS_MAX_TID                  255

#define SME_QOS_TSPEC_IE_LENGTH          61

#define SME_QOS_TSPEC_IE_TYPE            2
/*-------------------------------------------------------------------------- 
  Type declarations
  ------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
   Enumeration of the various states in the QoS state m/c
---------------------------------------------------------------------------*/
typedef enum
{
   SME_QOS_CLOSED = 0,
   SME_QOS_INIT,
   SME_QOS_LINK_UP,
   SME_QOS_REQUESTED,
   SME_QOS_QOS_ON,
   SME_QOS_HANDOFF,
   
}sme_QosStates;

/*---------------------------------------------------------------------------
   Enumeration of the various QoS cmds 
---------------------------------------------------------------------------*/
typedef enum
{
   SME_QOS_SETUP_REQ = 0,
   SME_QOS_RELEASE_REQ,
   SME_QOS_MODIFY_REQ,

   SME_QOS_CMD_MAX
}sme_QosCmdType;

/*---------------------------------------------------------------------------
   Enumeration of the various QoS reason codes to be used in the Flow list 
---------------------------------------------------------------------------*/
typedef enum
{
   SME_QOS_REASON_SETUP = 0,
   SME_QOS_REASON_RELEASE,
   SME_QOS_REASON_MODIFY,
   SME_QOS_REASON_MODIFY_PENDING,
   SME_QOS_REASON_REQ_SUCCESS,
   SME_QOS_REASON_SETUP_REQ_APSD_PENDING,
   SME_QOS_REASON_MODIFY_REQ_APSD_PENDING,

   SME_QOS_REASON_MAX
}sme_QosReasonType;


/*---------------------------------------------------------------------------
  Table to map user priority passed in as an argument to appropriate Access 
  Category as specified in 802.11e/WMM
---------------------------------------------------------------------------*/
sme_QosEdcaAcType sme_QosUPtoACMap[SME_QOS_WMM_UP_MAX] = 
{
   SME_QOS_EDCA_AC_BE, /* User Priority 0 */
   SME_QOS_EDCA_AC_BK, /* User Priority 1 */
   SME_QOS_EDCA_AC_BK, /* User Priority 2 */
   SME_QOS_EDCA_AC_BE, /* User Priority 3 */
   SME_QOS_EDCA_AC_VI, /* User Priority 4 */
   SME_QOS_EDCA_AC_VI, /* User Priority 5 */
   SME_QOS_EDCA_AC_VO, /* User Priority 6 */
   SME_QOS_EDCA_AC_VO  /* User Priority 7 */
};


/*---------------------------------------------------------------------------
  Table to map access category (AC) to appropriate user priority as specified
  in 802.11e/WMM

  Note: there is a quantization loss here because 4 ACs are mapped to 8 UPs
  Mapping is done for consistency
---------------------------------------------------------------------------*/
sme_QosWmmUpType sme_QosACtoUPMap[SME_QOS_EDCA_AC_MAX] = 
{
   SME_QOS_WMM_UP_BE,   /* AC BE */
   SME_QOS_WMM_UP_BK,   /* AC BK */
   SME_QOS_WMM_UP_VI,   /* AC VI */
   SME_QOS_WMM_UP_VO    /* AC VO */
};

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's FLOW Link List strucutre. This list can hold information per 
  flow/request, like TSPEC params requested, which AC it is running on 
---------------------------------------------------------------------------*/
typedef struct sme_QosFlowInfoEntry_s
{
    tListElem             link;  /* list links */
    v_U8_t                tspec_mask;
    sme_QosReasonType     reason;
    v_U32_t               QosFlowID;
    sme_QosEdcaAcType     ac_type;
    sme_QosWmmTspecInfo   QoSInfo;
    void                * HDDcontext;
    sme_QosCallback       QoSCallback;
    v_BOOL_t              hoRenewal;//set to TRUE while re-negotiating flows after
                                     //handoff, will set to FALSE once done with
                                     //the process. Helps SME to decide if at all 
                                     //to notify HDD/LIS for flow renewal after HO
} sme_QosFlowInfoEntry;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's setup request cmd related information strucutre. 
---------------------------------------------------------------------------*/
typedef struct sme_QosSetupCmdInfo_s
{
    v_U32_t               QosFlowID;
    sme_QosWmmTspecInfo  *pQoSInfo;
    void                 *HDDcontext;
    sme_QosCallback       QoSCallback;
    sme_QosWmmUpType      UPType;
    v_BOOL_t              hoRenewal;//set to TRUE while re-negotiating flows after
                                     //handoff, will set to FALSE once done with
                                     //the process. Helps SME to decide if at all 
                                     //to notify HDD/LIS for flow renewal after HO
} sme_QosSetupCmdInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's modify cmd related information strucutre. 
---------------------------------------------------------------------------*/
typedef struct sme_QosModifyCmdInfo_s
{
    v_U32_t               QosFlowID;
    sme_QosWmmTspecInfo  *pQoSInfo;
} sme_QosModifyCmdInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's release cmd related information strucutre. 
---------------------------------------------------------------------------*/
typedef struct sme_QosReleaseCmdInfo_s
{
    v_U32_t               QosFlowID;
} sme_QosReleaseCmdInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's buffered cmd related information strucutre. 
---------------------------------------------------------------------------*/
typedef struct sme_QosCmdInfo_s
{
    sme_QosCmdType        command;
    tpAniSirGlobal        pMac;
    union
    {
       sme_QosSetupCmdInfo    setupCmdInfo;
       sme_QosModifyCmdInfo   modifyCmdInfo;
       sme_QosReleaseCmdInfo  releaseCmdInfo;
    }u;
} sme_QosCmdInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's buffered cmd List strucutre. This list can hold information 
  related to any pending cmd from HDD
---------------------------------------------------------------------------*/
typedef struct sme_QosCmdInfoEntry_s
{
    tListElem             link;  /* list links */
    sme_QosCmdInfo        cmdInfo;
} sme_QosCmdInfoEntry;

/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's Per AC information strucutre. This can hold information on
  how many flows running on the AC, the current, previous states the AC is in 
---------------------------------------------------------------------------*/
typedef struct sme_QosACInfo_s
{
   v_U8_t                 num_flows[SME_QOS_TSPEC_INDEX_MAX];
   v_U32_t                curr_flowID;
   sme_QosStates          curr_state;
   sme_QosStates          prev_state;
   sme_QosWmmTspecInfo    curr_QoSInfo[SME_QOS_TSPEC_INDEX_MAX];
   sme_QosWmmTspecInfo    requested_QoSInfo[SME_QOS_TSPEC_INDEX_MAX];
   v_BOOL_t               reassoc_pending;//reassoc requested for APSD
   //As per WMM spec there could be max 2 TSPEC running on the same AC with 
   //different direction. We will refer each TSPEC with an index
   v_U8_t                 tspec_mask_status; //status showing if both the indices are in use
   v_U8_t                 tspec_pending;//tspec negotiation going on for which index
   v_BOOL_t               hoRenewal;//set to TRUE while re-negotiating flows after
                                    //handoff, will set to FALSE once done with
                                    //the process. Helps SME to decide if at all 
                                    //to notify HDD/LIS for flow renewal after HO

} sme_QosACInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  Search key union. We can either use the flowID or the ac type to find an entry 
  in the flow list
---------------------------------------------------------------------------*/
typedef union sme_QosSearchKey_s
{
   v_U32_t               QosFlowID;
   sme_QosEdcaAcType     ac_type;
   sme_QosReasonType     reason;
}sme_QosSearchKey;

/*---------------------------------------------------------------------------
DESCRIPTION
  We can either use the flowID or the ac type to find an entry in the flow list.
  The index is a bitmap telling us which key to use. Starting from LSB,
  bit 0 - Flow ID
  bit 1 - AC type
---------------------------------------------------------------------------*/
typedef struct sme_QosSearchInfo_s
{
   v_U8_t           index;
   sme_QosSearchKey key;
}sme_QosSearchInfo;

/*---------------------------------------------------------------------------
DESCRIPTION
  Start UAPSD request structure, containing the global Mac pointer & the pointer 
  to the entry in Flow link list which is looking for UAPSD.
---------------------------------------------------------------------------*/
typedef struct sme_QosPmcStartUAPSDRequest_s
{
   tpAniSirGlobal   pMac;
   tListElem       *pEntry;
   v_BOOL_t         hoRenewal;//set to TRUE while re-negotiating flows after
                              //handoff, will set to FALSE once done with
                              //the process. Helps SME to decide if at all 
                              //to notify HDD/LIS for flow renewal after HO

}sme_QosPmcStartUAPSDRequest;


/*---------------------------------------------------------------------------
DESCRIPTION
  SME QoS module's internal control block.
---------------------------------------------------------------------------*/
struct sme_QosCb_s
{
   //global Mac pointer
   tpAniSirGlobal   pMac;
   //All AC info
   sme_QosACInfo          ac_info[SME_QOS_EDCA_AC_MAX];
   //All FLOW info
   tDblLinkList           flow_list;
   //default TSPEC params
   sme_QosWmmTspecInfo    def_QoSInfo[SME_QOS_EDCA_AC_MAX];
   //association info
   sme_QosAssocInfo       assoc_Info;
   //All buffered cmd info
   tDblLinkList           buffered_cmd_list;
   //currently the ACs with APSD on 
   // Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored
   v_U8_t                 apsd_mask;
   //to know if the reassoc was indeed initiated by QoS
   v_U32_t                roamID;
   //maintaining a powersave status in QoS module, to be fed back to PMC at 
   // times through the sme_QosPmcCheckRoutine
   v_BOOL_t               readyForPowerSave;
   //start UAPSD request list
   sme_QosPmcStartUAPSDRequest uapsdRequest[SME_QOS_MAX_APSD_REQ_PENDING];
   //UAPSD request counter
   v_U8_t                 apsd_req_counter;
   //UAPSD is already requested from PMC
   v_BOOL_t               uapsdAlreadyRequested;
   //if we are in the process of handing off to a different AP
   v_BOOL_t               handoffRequested;
}sme_QosCb;

typedef eHalStatus (*sme_QosProcessSearchEntry)(tpAniSirGlobal pMac, tListElem *pEntry);
/*-------------------------------------------------------------------------- 
                         Internal function declarations
  ------------------------------------------------------------------------*/
sme_QosStatusType sme_QosInternalSetupReq(tpAniSirGlobal pMac, 
                                          sme_QosWmmTspecInfo * pQoSInfo,
                                          sme_QosCallback QoSCallback, 
                                          void * HDDcontext,
                                          sme_QosWmmUpType UPType, 
                                          v_U32_t * pQosFlowID,
                                          v_BOOL_t buffered_cmd,
                                          v_BOOL_t hoRenewal);
sme_QosStatusType sme_QosInternalModifyReq(tpAniSirGlobal pMac, 
                                           sme_QosWmmTspecInfo * pQoSInfo,
                                           v_U32_t QosFlowID,
                                           v_BOOL_t buffered_cmd);
sme_QosStatusType sme_QosInternalReleaseReq(tpAniSirGlobal pMac, 
                                            v_U32_t QosFlowID,
                                            v_BOOL_t buffered_cmd);

sme_QosStatusType sme_QosSetup(tpAniSirGlobal pMac,
                               sme_QosWmmTspecInfo *pTspec_Info, 
                               sme_QosEdcaAcType ac);
eHalStatus sme_QosAddTsReq(tpAniSirGlobal pMac, 
                           sme_QosWmmTspecInfo * pTspec_Info,
                           sme_QosEdcaAcType ac);

eHalStatus sme_QosDelTsReq(tpAniSirGlobal pMac, sme_QosEdcaAcType ac, v_U8_t tspec_mask);

eHalStatus sme_QosProcessAddTsRsp(tpAniSirGlobal pMac, void *pMsgBuf);
eHalStatus sme_QosProcessDelTsInd(tpAniSirGlobal pMac, void *pMsgBuf);
eHalStatus sme_QosProcessDelTsRsp(tpAniSirGlobal pMac, void *pMsgBuf);

eHalStatus sme_QosProcessAssocCompleteEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessReassocReqEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessReassocSuccessEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessReassocFailureEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessDisconnectEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessJoinReqEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessHandoffAssocReqEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessHandoffSuccessEv(tpAniSirGlobal pMac, void * pEvent_info);
eHalStatus sme_QosProcessHandoffFailureEv(tpAniSirGlobal pMac, void * pEvent_info);

eHalStatus sme_QosProcessAddTsSuccessRsp(tpAniSirGlobal pMac, 
                                         tSirAddtsRspInfo * pRsp);
eHalStatus sme_QosProcessAddTsFailureRsp(tpAniSirGlobal pMac, 
                                         tSirAddtsRspInfo * pRsp);

eHalStatus sme_QosAggregateParams(
   sme_QosWmmTspecInfo * pInput_Tspec_Info,
   sme_QosWmmTspecInfo * pCurrent_Tspec_Info,
   sme_QosWmmTspecInfo * pUpdated_Tspec_Info);

eHalStatus sme_QosUpdateParams(sme_QosEdcaAcType ac, v_U8_t tspec_mask, 
                               sme_QosWmmTspecInfo * pTspec_Info);
sme_QosWmmUpType sme_QosAcToUp(sme_QosEdcaAcType ac);
sme_QosEdcaAcType sme_QosUpToAc(sme_QosWmmUpType up);
v_BOOL_t sme_QosIsACM(tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, 
                      sme_QosEdcaAcType ac, tDot11fBeaconIEs *pIes);
tListElem *sme_QosFindInFlowList(sme_QosSearchInfo search_key);
eHalStatus sme_QosFindAllInFlowList(tpAniSirGlobal pMac,
                                    sme_QosSearchInfo search_key, 
                                    sme_QosProcessSearchEntry fnp);

void sme_QosStateTransition(sme_QosStates new_state, sme_QosEdcaAcType ac);
eHalStatus sme_QosBufferCmd(sme_QosCmdInfo *pcmd, v_BOOL_t insert_head);
eHalStatus sme_QosProcessBufferedCmd(void);
eHalStatus sme_QosSaveAssocInfo(sme_QosAssocInfo *pAssoc_info);

eHalStatus sme_QosSetupFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosModificationNotifyFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosModifyFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosDelTsIndFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosReassocSuccessEvFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosAddTsFailureFnp(tpAniSirGlobal pMac, tListElem *pEntry);
eHalStatus sme_QosAddTsSuccessFnp(tpAniSirGlobal pMac, tListElem *pEntry);

v_BOOL_t sme_QosIsRspPending(sme_QosEdcaAcType ac);


void sme_QosPmcFullPowerCallback(void *callbackContext, eHalStatus status);
void sme_QosPmcStartUapsdCallback(void *callbackContext, eHalStatus status);
v_BOOL_t sme_QosPmcCheckRoutine(void *callbackContext);
void sme_QosPmcDeviceStateUpdateInd(void *callbackContext, tPmcState pmcState);
eHalStatus sme_QosProcessOutOfUapsdMode(tpAniSirGlobal pMac);
eHalStatus sme_QosProcessIntoUapsdMode(tpAniSirGlobal pMac);
eHalStatus sme_QosBufferExistingFlows(tpAniSirGlobal pMac);
eHalStatus sme_QosDeleteExistingFlows(tpAniSirGlobal pMac);
void sme_QosCleanupCtrlBlkForHandoff(tpAniSirGlobal pMac);
eHalStatus sme_QosDeleteBufferedRequests(tpAniSirGlobal pMac);
v_BOOL_t sme_QosValidateRequestedParams(sme_QosWmmTspecInfo * pQoSInfo);

extern eHalStatus sme_AcquireGlobalLock( tSmeStruct *psSme);
extern eHalStatus sme_ReleaseGlobalLock( tSmeStruct *psSme);

static eHalStatus qosIssueCommand( tpAniSirGlobal pMac, eSmeCommandType cmdType, void *pvParam, tANI_U32 size,
                            sme_QosEdcaAcType ac, v_U8_t tspec_mask );
static void sme_QosHandleCallback(eHalStatus status);

#if defined(SME_QOS_NOT_SUPPORTED)
eHalStatus sme_QosSetBSSID(tpAniSirGlobal pMac);
eHalStatus sme_QosSetSSID(tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc);
#endif

/*-------------------------------------------------------------------------- 
                         External APIs definitions
  ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
    \brief sme_QosOpen() - This function must be called before any API call to 
    SME QoS module.

    \param pMac - Pointer to the global MAC parameter structure.
    
    \return eHalStatus     
----------------------------------------------------------------------------*/
eHalStatus sme_QosOpen(tpAniSirGlobal pMac)
{
   sme_QosEdcaAcType ac;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosOpen:Test: initializing SME-QoS module\n" );

   //init the control block
   vos_mem_zero(&sme_QosCb, sizeof(sme_QosCb));

   sme_QosCb.pMac = pMac;
   //init flow list
   if (csrLLOpen(pMac->hHdd, &sme_QosCb.flow_list) != eHAL_STATUS_SUCCESS)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosOpen: cannot initialize Flow List\n" );

      return eHAL_STATUS_FAILURE;
   }
   
   //init cmd list
   if (csrLLOpen(pMac->hHdd, &sme_QosCb.buffered_cmd_list) != 
       eHAL_STATUS_SUCCESS)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosOpen: cannot initialize cmd List\n" );

      return eHAL_STATUS_FAILURE;
   }

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      sme_QosStateTransition(SME_QOS_INIT,
                             ac);       
      sme_QosCb.ac_info[ac].curr_flowID = SME_QOS_NUM_FLOW_ID_PER_AC * (ac + 1); 
   }

   //the routine registered here gets called by PMC whenever the device is about 
   //to enter one of the power save modes. PMC runs a poll with all the 
   //registered modules if device can enter powersave mode or remain full power
   if(!HAL_STATUS_SUCCESS(
      pmcRegisterPowerSaveCheck(pMac, sme_QosPmcCheckRoutine, pMac)))
   {
      return eHAL_STATUS_FAILURE;
   }

   sme_QosCb.readyForPowerSave = VOS_TRUE;

   //the routine registered here gets called by PMC whenever there is a device 
   // state change. PMC might go to full power because of many reasons and this 
   // is the way for PMC to inform all the other registered modules so that 
   // everyone is in sync.
   if(!HAL_STATUS_SUCCESS(
      pmcRegisterDeviceStateUpdateInd(pMac, sme_QosPmcDeviceStateUpdateInd, pMac)))
   {
      return eHAL_STATUS_FAILURE;
   }

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosOpen:Test: done initializing SME-QoS module\n" );

   return eHAL_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
    \brief sme_QosClose() - To close down SME QoS module. There should not be 
    any API call into this module after calling this function until another
    call of sme_QosOpen.

    \param pMac - Pointer to the global MAC parameter structure.
    
    \return eHalStatus     
----------------------------------------------------------------------------*/
eHalStatus sme_QosClose(tpAniSirGlobal pMac)
{
   sme_QosEdcaAcType ac;
   //cleanup control block

   //close the flow list
   csrLLClose(&sme_QosCb.flow_list);

   //close the cmd list
   csrLLClose(&sme_QosCb.buffered_cmd_list);

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      sme_QosStateTransition(SME_QOS_CLOSED,
                             ac);       
   }

   sme_QosCb.readyForPowerSave = VOS_TRUE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosClose:Test: closed down QoS\n" );

   return eHAL_STATUS_SUCCESS;
}



/*--------------------------------------------------------------------------
  \brief sme_QosSetupReq() - The SME QoS API exposed to HDD to request for QoS 
  on a particular AC. This function should be called after a link has been 
  established, i.e. STA is associated with an AP etc. If the request involves 
  admission control on the requested AC, HDD needs to provide the necessary 
  Traffic Specification (TSPEC) parameters otherwise SME is going to use the
  default params.
  
  \param hHal - The handle returned by macOpen.
  \param pQoSInfo - Pointer to sme_QosWmmTspecInfo which contains the WMM TSPEC
                    related info as defined above, provided by HDD
  \param QoSCallback - The callback which is registered per flow while 
                       requesting for QoS. Used for any notification for the 
                       flow (i.e. setup success/failure/release) which needs to 
                       be sent to HDD
  \param HDDcontext - A cookie passed by HDD to be used by SME during any QoS 
                      notification (through the callabck) to HDD 
  \param UPType - Useful only if HDD or any other upper layer module (BAP etc.)
                  looking for implicit QoS setup, in that 
                  case, the pQoSInfo will be NULL & SME will know about the AC
                  (from the UP provided in this param) QoS is requested on
  \param pQosFlowID - Identification per flow running on each AC generated by 
                      SME. 
                     It is only meaningful if the QoS setup for the flow is 
                     successful
                  
  \return eHAL_STATUS_SUCCESS - Setup is successful.
  
          Other status means Setup request failed     
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosSetupReq(tHalHandle hHal, 
                                  sme_QosWmmTspecInfo * pQoSInfo,
                                  sme_QosCallback QoSCallback, void * HDDcontext,
                                  sme_QosWmmUpType UPType, v_U32_t * pQosFlowID)
{
   eHalStatus lock_status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   sme_QosWmmTspecInfo * pInQoSInfo = NULL;
   sme_QosStatusType status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   //copying over the QoS info structure
   pInQoSInfo = (sme_QosWmmTspecInfo*)vos_mem_malloc(sizeof(sme_QosWmmTspecInfo));
   if(!pInQoSInfo)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupReq:failed to allocate memory for Qos info\n" );
      return eHAL_STATUS_FAILURE;
   }

   vos_mem_zero(pInQoSInfo, sizeof(sme_QosWmmTspecInfo));
   vos_mem_copy(pInQoSInfo, pQoSInfo, sizeof(sme_QosWmmTspecInfo));

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosSetupReq:Test: QoS setup requested by client\n" );

   lock_status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( lock_status ) )
   {
   //Call the internal function for QoS setup, adding a layer of abstraction
   status = sme_QosInternalSetupReq(pMac, pInQoSInfo, QoSCallback, HDDcontext,
                                    UPType, pQosFlowID, VOS_FALSE, VOS_FALSE);

      if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != status)
      {
         sme_QosCb.readyForPowerSave = VOS_TRUE;
      }

      sme_ReleaseGlobalLock( &pMac->sme );
   }
   else
   {
      vos_mem_free(pInQoSInfo);
   }
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosSetupReq:Test: QoS setup status = %d\n", status );

   return status;

}


/*--------------------------------------------------------------------------
  \brief sme_QosModifyReq() - The SME QoS API exposed to HDD to request for 
  modification of certain QoS params on a flow running on a particular AC. 
  This function should be called after a link has been established, i.e. STA is 
  associated with an AP etc. & a QoS setup has been succesful for that flow. 
  If the request involves admission control on the requested AC, HDD needs to 
  provide the necessary Traffic Specification (TSPEC) parameters & SME might
  start the renegotiation process through ADDTS.
  
  \param hHal - The handle returned by macOpen.
  \param pQoSInfo - Pointer to sme_QosWmmTspecInfo which contains the WMM TSPEC
                    related info as defined above, provided by HDD
  \param QosFlowID - Identification per flow running on each AC generated by 
                      SME. 
                     It is only meaningful if the QoS setup for the flow has 
                     been successful already
                  
  \return SME_QOS_STATUS_SETUP_SUCCESS_RSP - Modification is successful.
  
          Other status means request failed     
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosModifyReq(tHalHandle hHal, 
                                   sme_QosWmmTspecInfo * pQoSInfo,
                                   v_U32_t QosFlowID)
{
   eHalStatus lock_status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   sme_QosStatusType status = SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosModifyReq:Test: QoS Modify requested by client\n" );

   lock_status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( lock_status ) )
   {
   //Call the internal function for QoS modify, adding a layer of abstraction
   status = sme_QosInternalModifyReq(pMac, pQoSInfo, QosFlowID, VOS_FALSE);
      if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != status)&&
         (SME_QOS_STATUS_MODIFY_SETUP_PENDING_RSP != status))
      {
         sme_QosCb.readyForPowerSave = VOS_TRUE;
      }

      sme_ReleaseGlobalLock( &pMac->sme );
   }
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosModifyReq:Test: QoS Modify status = %d\n", status );

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosReleaseReq() - The SME QoS API exposed to HDD to request for 
  releasing a QoS flow running on a particular AC. This function should be 
  called only if a QoS is set up with a valid FlowID. HDD sould invoke this 
  API only if an explicit request for QoS release has come from Application 
  
  \param hHal - The handle returned by macOpen.
  \param QosFlowID - Identification per flow running on each AC generated by SME 
                     It is only meaningful if the QoS setup for the flow is 
                     successful
  
  \return eHAL_STATUS_SUCCESS - Release is successful.
  
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosReleaseReq(tHalHandle hHal, v_U32_t QosFlowID)
{
   eHalStatus lock_status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
   sme_QosStatusType status = SME_QOS_STATUS_RELEASE_FAILURE_RSP;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosReleaseReq:Test: QoS release requested by client\n" );

   lock_status = sme_AcquireGlobalLock( &pMac->sme );
   if ( HAL_STATUS_SUCCESS( lock_status ) )
   {
   //Call the internal function for QoS release, adding a layer of abstraction
   status = sme_QosInternalReleaseReq(pMac, QosFlowID, VOS_FALSE);
      if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != status)&&
         (SME_QOS_STATUS_RELEASE_REQ_PENDING_RSP != status))
      {
         sme_QosCb.readyForPowerSave = VOS_TRUE;
      }

      sme_ReleaseGlobalLock( &pMac->sme );
   }

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosReleaseReq:Test: QoS release status = %d\n", status );

   return status;

}
/*--------------------------------------------------------------------------
  \brief sme_QosSetParams() - This function is used by HDD to provide the 
   default TSPEC params to SME.
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param pQoSInfo - Pointer to sme_QosWmmTspecInfo which contains the WMM TSPEC
                    related info per AC as defined above, provided by HDD
  
  \return eHAL_STATUS_SUCCESS - Setparam is successful.
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosSetParams(tpAniSirGlobal pMac, sme_QosWmmTspecInfo * pQoSInfo)
{
   sme_QosEdcaAcType ac;
   // find the AC
   ac = sme_QosUpToAc(pQoSInfo->ts_info.up);
   if(SME_QOS_EDCA_AC_MAX == ac)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetParams: invalid AC = %d\n", ac );

      return eHAL_STATUS_FAILURE;
   }

   //copy over the default params 
   vos_mem_copy( &sme_QosCb.def_QoSInfo, pQoSInfo,
                 sizeof(sme_QosWmmTspecInfo));

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosSetParams:Test: QoS default params set \n" );

   return eHAL_STATUS_SUCCESS;
}


void qosReleaseCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
   vos_mem_zero( &pCommand->u.qosCmd, sizeof( tGenericQosCmd ) );
   smeReleaseCommand( pMac, pCommand );
}


/*--------------------------------------------------------------------------
  \brief sme_QosMsgProcessor() - sme_ProcessMsg() calls this function for the 
  messages that are handled by SME QoS module.
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param msg_type - the type of msg passed by PE as defined in wniApi.h
  \param pMsgBuf - a pointer to a buffer that maps to various structures base 
                   on the message type.
                   The beginning of the buffer can always map to tSirSmeRsp.
  
  \return eHAL_STATUS_SUCCESS - Validation is successful.
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosMsgProcessor( tpAniSirGlobal pMac,  v_U16_t msg_type, 
                                void *pMsgBuf)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tListElem *pEntry;
   tSmeCmd *pCommand;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosMsgProcessor:Test: msg = %d for QoS\n", msg_type );

   //switch on the msg type & make the state transition accordingly
   switch(msg_type)
   {
      case eWNI_SME_ADDTS_RSP:
         pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
         if( pEntry )
         {
             pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
             if( eSmeCommandAddTs == pCommand->command )
             {
                status = sme_QosProcessAddTsRsp(pMac, pMsgBuf);
                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
                {
                   qosReleaseCommand( pMac, pCommand );
                }
                smeProcessPendingQueue( pMac );
             }
         }
         sme_QosCb.readyForPowerSave = VOS_TRUE;
         break;

      case eWNI_SME_DELTS_RSP:
         pEntry = csrLLPeekHead(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
         if( pEntry )
         {
             pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
             if( eSmeCommandDelTs == pCommand->command )
             {
                status = sme_QosProcessDelTsRsp(pMac, pMsgBuf);
                if( csrLLRemoveEntry( &pMac->sme.smeCmdActiveList, pEntry, LL_ACCESS_LOCK ) )
                {
                   qosReleaseCommand( pMac, pCommand );
                }
                smeProcessPendingQueue( pMac );
             }
         }
         sme_QosCb.readyForPowerSave = VOS_TRUE;
         break;

      case eWNI_SME_DELTS_IND:
         status = sme_QosProcessDelTsInd(pMac, pMsgBuf);
         break;

      default:
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosMsgProcessor:unknown msg type = %d\n", msg_type);

         break;
   }

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosValidateParams() - The SME QoS API exposed to CSR to validate AP 
  capabilities regarding QoS support & any other QoS parameter validation.
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param pBssDesc - Pointer to the BSS Descriptor information passed down by 
                    CSR to PE while issuing the Join request
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosValidateParams(tpAniSirGlobal pMac, 
                                 tSirBssDescription *pBssDesc)
{
   tDot11fBeaconIEs *pIes = NULL;
   eHalStatus status = eHAL_STATUS_FAILURE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosValidateParams:Test: validation for QAP & APSD\n");

   do
   {
      if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pBssDesc, &pIes)))
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosValidateParams:csrGetParsedBssDescriptionIEs() failed\n");

         break;
      }

      //check if the AP is QAP & it supports APSD
      if( !CSR_IS_QOS_BSS(pIes) )
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosValidateParams:AP doesn't support QoS\n");
         
         break;
      }

      if(!(pIes->WMMParams.qosInfo & SME_QOS_AP_SUPPORTS_APSD) &&
         !(pIes->WMMInfoAp.uapsd))
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosValidateParams:AP doesn't support APSD\n");

         break;
      }
      status = eHAL_STATUS_SUCCESS;

   }while(0);

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosValidateParams:Test: validated with status = %d\n", status );

   if(pIes)
   {
      vos_mem_free(pIes);
   }
   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosCsrEventInd() - The QoS sub-module in SME expects notifications 
  from CSR when certain events occur as mentioned in sme_QosCsrEventIndType.

  \param pMac - Pointer to the global MAC parameter structure.
  \param ind - The event occured of type sme_QosCsrEventIndType.
  \param pEvent_info - Information related to the event
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosCsrEventInd(tpAniSirGlobal pMac,
                              sme_QosCsrEventIndType ind, 
                              void *pEvent_info)
{
   eHalStatus status = eHAL_STATUS_FAILURE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosCsrEventInd:Test: Event ind = %d from CSR\n", ind );

   switch(ind)
   {
      case SME_QOS_CSR_ASSOC_COMPLETE:
         //expecting assoc info in pEvent_info
         status = sme_QosProcessAssocCompleteEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_REASSOC_REQ:
         //nothing expected in pEvent_info
         status = sme_QosProcessReassocReqEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_REASSOC_COMPLETE:
         //expecting assoc info in pEvent_info
         status = sme_QosProcessReassocSuccessEv(pMac, pEvent_info);
         sme_QosCb.readyForPowerSave = VOS_TRUE;
         break;
      case SME_QOS_CSR_REASSOC_FAILURE:
         //nothing expected in pEvent_info
         status = sme_QosProcessReassocFailureEv(pMac, pEvent_info);
         sme_QosCb.readyForPowerSave = VOS_TRUE;
         break;
      case SME_QOS_CSR_DISCONNECT_REQ:
      case SME_QOS_CSR_DISCONNECT_IND:
         //nothing expected in pEvent_info
         status = sme_QosProcessDisconnectEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_JOIN_REQ:
         //nothing expected in pEvent_info
         status = sme_QosProcessJoinReqEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_HANDOFF_ASSOC_REQ:
         //nothing expected in pEvent_info
         status = sme_QosProcessHandoffAssocReqEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_HANDOFF_COMPLETE:
         //nothing expected in pEvent_info
         status = sme_QosProcessHandoffSuccessEv(pMac, pEvent_info);
         break;
      case SME_QOS_CSR_HANDOFF_FAILURE:
         //nothing expected in pEvent_info
         status = sme_QosProcessHandoffFailureEv(pMac, pEvent_info);
         break;
      default:
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosCsrEventInd:unknown event indication = %d from CSR\n", 
                   ind);

         break;
   }

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosCsrEventInd:Test: processed the ind with status = %d\n", status );

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosGetACMMask() - The QoS sub-module API to find out on which ACs
  AP mandates Admission Control (ACM = 1)
  (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored)

  \param pMac - Pointer to the global MAC parameter structure.
  \param pSirBssDesc - The event occured of type sme_QosCsrEventIndType.

  
  \return a bit mask indicating for which ACs AP has ACM set to 1
  
  \sa
  
  --------------------------------------------------------------------------*/
v_U8_t sme_QosGetACMMask(tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, tDot11fBeaconIEs *pIes)
{
   sme_QosEdcaAcType ac;
   v_U8_t acm_mask = 0;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosGetACMMask:Test: invoked\n");

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++)
   {
      if(sme_QosIsACM(pMac, pSirBssDesc, ac, pIes))
      {
         acm_mask = acm_mask | (1 << (SME_QOS_EDCA_AC_VO - ac));
      }
      
   }

   return acm_mask;
}

/*-------------------------------------------------------------------------- 
                         Internal function definitions
  ------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  \brief sme_QosInternalSetupReq() - The SME QoS internal setup request handling
  function.
  If the request involves admission control on the requested AC, HDD needs to 
  provide the necessary Traffic Specification (TSPEC) parameters otherwise SME 
  is going to use the default params.
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param pQoSInfo - Pointer to sme_QosWmmTspecInfo which contains the WMM TSPEC
                    related info as defined above, provided by HDD
  \param QoSCallback - The callback which is registered per flow while 
                       requesting for QoS. Used for any notification for the 
                       flow (i.e. setup success/failure/release) which needs to 
                       be sent to HDD
  \param HDDcontext - A cookie passed by HDD to be used by SME during any QoS 
                      notification (through the callabck) to HDD 
  \param UPType - Useful only if HDD or any other upper layer module (BAP etc.)
                  looking for implicit QoS setup, in that 
                  case, the pQoSInfo will be NULL & SME will know about the AC
                  (from the UP provided in this param) QoS is requested on
  \param pQosFlowID - Identification per flow running on each AC generated by 
                      SME. 
                     It is only meaningful if the QoS setup for the flow is 
                     successful
  \param buffered_cmd - tells us if the cmd was a buffered one or fresh from 
                        cleint
                  
  \return eHAL_STATUS_SUCCESS - Setup is successful.
  
          Other status means Setup request failed     
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosInternalSetupReq(tpAniSirGlobal pMac, 
                                          sme_QosWmmTspecInfo * pQoSInfo,
                                          sme_QosCallback QoSCallback, 
                                          void * HDDcontext,
                                          sme_QosWmmUpType UPType, 
                                          v_U32_t * pQosFlowID,
                                          v_BOOL_t buffered_cmd,
                                          v_BOOL_t hoRenewal)
{
   sme_QosEdcaAcType ac;
   sme_QosWmmTspecInfo Tspec_Info;
   sme_QosStates new_state = SME_QOS_CLOSED;
   sme_QosFlowInfoEntry *pentry = NULL;
   tListElem *pEntry1= NULL;
   sme_QosCmdInfo  cmd;
   sme_QosStatusType status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   v_U8_t tmask = 0;
   v_U8_t new_tmask = 0;
   sme_QosSearchInfo search_key;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
   sme_QosFlowInfoEntry *flow_info2 = NULL;
#endif

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosInternalSetupReq:Test: invoked\n");

   vos_mem_zero(&Tspec_Info, sizeof(sme_QosWmmTspecInfo));
   // if caller sent an emtry TSPEC, fill up with the default one
   if(!pQoSInfo)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_WARN, 
                "sme_QosInternalSetupReq: caller sent an empty QoS param list, using defaults\n" );

      // find the AC with UPType passed in
      ac = sme_QosUpToAc(UPType);
      if(SME_QOS_EDCA_AC_MAX == ac)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: invalid AC = %d\n", ac );
         
         return SME_QOS_STATUS_SETUP_INVALID_PARAMS_RSP;
      }

      vos_mem_copy(&Tspec_Info, &sme_QosCb.def_QoSInfo[ac], 
                   sizeof(sme_QosWmmTspecInfo));
   }
   else
   {
      // find the AC
      ac = sme_QosUpToAc(pQoSInfo->ts_info.up);
      if(SME_QOS_EDCA_AC_MAX == ac)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: invalid AC = %d\n", ac );
         
         return SME_QOS_STATUS_SETUP_INVALID_PARAMS_RSP;
      }

      //validate QoS params
      if(!sme_QosValidateRequestedParams(pQoSInfo))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: invalid params\n");

         return SME_QOS_STATUS_SETUP_INVALID_PARAMS_RSP;
      }
      vos_mem_copy(&Tspec_Info, pQoSInfo, sizeof(sme_QosWmmTspecInfo));
   }

   sme_QosCb.readyForPowerSave = VOS_FALSE;
   //call PMC's request for power function
   // AND
   //another check is added cosidering the flowing scenario
   //Addts reqest is pending on one AC, while APSD requested on another which 
   //needs a reassoc. Will buffer a request if Addts is pending on any AC, 
   //which will safegaurd the above scenario, & also won't confuse PE with back 
   //to back Addts or Addts followed by Reassoc

   if(sme_QosIsRspPending(ac) || 
      ( eHAL_STATUS_PMC_PENDING == pmcRequestFullPower(pMac, sme_QosPmcFullPowerCallback, pMac, eSME_REASON_OTHER)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                "sme_QosInternalSetupReq: buffering the setup request in state \
                = %d as Addts is pending on other AC/waiting for full power\n", 
                sme_QosCb.ac_info[ac].curr_state );

      //buffer cmd
      cmd.command = SME_QOS_SETUP_REQ;
      cmd.u.setupCmdInfo.HDDcontext = HDDcontext;
      cmd.u.setupCmdInfo.pQoSInfo = pQoSInfo;
      cmd.u.setupCmdInfo.QoSCallback = QoSCallback;
      cmd.pMac = pMac;
      cmd.u.setupCmdInfo.UPType = UPType;
      cmd.u.setupCmdInfo.hoRenewal = hoRenewal;
      //assign a flowID while buffering
      if(!buffered_cmd)
      {
      *pQosFlowID = sme_QosCb.ac_info[ac].curr_flowID++;
      }
      
      cmd.u.setupCmdInfo.QosFlowID = *pQosFlowID;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosInternalSetupReq: *pQosFlowID = %d, \
                cmd.u.setupCmdInfo.QosFlowID = %d, sme_QosCb.ac_info[ac].curr_flowID = %d \n",
                *pQosFlowID, cmd.u.setupCmdInfo.QosFlowID, sme_QosCb.ac_info[ac].curr_flowID);

      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: couldn't buffer the setup request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );

         return SME_QOS_STATUS_SETUP_FAILURE_RSP;
      }

      return SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
   }


   //get into the state m/c to see if the request can be granted
   switch(sme_QosCb.ac_info[ac].curr_state)
   {
   case SME_QOS_CLOSED:
   case SME_QOS_INIT:
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalSetupReq: wrong state = %d for setup request\n", 
                sme_QosCb.ac_info[ac].curr_state );
      //ASSERT?
      VOS_ASSERT(0);
      new_state = sme_QosCb.ac_info[ac].curr_state;
      break;
   case SME_QOS_LINK_UP:

      //call the internal qos setup logic to decide on if the
      // request is NOP, or need reassoc for APSD and/or need to send out ADDTS
      status = sme_QosSetup(pMac, &Tspec_Info, ac);
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosInternalSetupReq:Test: sme_QosSetup returned in \
                SME_QOS_LINK_UP state on AC = %d, with status =%d\n", ac, status);

      if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status)||
         (SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
         (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status))
      {
         new_state = SME_QOS_REQUESTED;
         sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_0]++;
         sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0] = Tspec_Info;
         //create an entry in the flow list
         pentry= (sme_QosFlowInfoEntry *)
         vos_mem_malloc(sizeof(sme_QosFlowInfoEntry));
         if (!pentry)
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosInternalSetupReq: couldn't allocate memory for the new\
                      entry in the Flow List\n");
            return SME_QOS_STATUS_SETUP_FAILURE_RSP;
         }
         pentry->ac_type = ac;
         pentry->HDDcontext = HDDcontext;
         pentry->QoSCallback = QoSCallback;
         pentry->hoRenewal = hoRenewal;
         //we already assigned a flowID while buffering the cmd & passed it back 
         // to HDD
         if(!buffered_cmd)
         {
            pentry->QosFlowID = sme_QosCb.ac_info[ac].curr_flowID++;
            *pQosFlowID = pentry->QosFlowID;
         }
         else
         {
            pentry->QosFlowID = *pQosFlowID;
         }
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                   "sme_QosInternalSetupReq: *pQosFlowID = %d, \
                   pentry->QosFlowID = %d, sme_QosCb.ac_info[ac].curr_flowID = %d \n",
                   *pQosFlowID, pentry->QosFlowID, sme_QosCb.ac_info[ac].curr_flowID);


         if(SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status)
         {
            pentry->reason = SME_QOS_REASON_REQ_SUCCESS;
            new_state = SME_QOS_QOS_ON;
            sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_BIT_1_SET;
            sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0] = 
               sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0];
            //notify HDD through the synchronous status msg
            //but if the cmd was buffered before notify with async msg
            if(buffered_cmd && !pentry->hoRenewal)
            {
               QoSCallback(pMac, HDDcontext, 
                           &sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0],
                           status,
                           pentry->QosFlowID);

            }
            pentry->hoRenewal = VOS_FALSE;
         }
         else if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status)
         {
            if(sme_QosCb.ac_info[ac].tspec_mask_status &&
               !sme_QosCb.ac_info[ac].reassoc_pending)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalSetupReq: tspec_mask_status shouldn't be set already\n");

               //ASSERT
               VOS_ASSERT(0);
               return SME_QOS_STATUS_SETUP_FAILURE_RSP;
            }
            sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_BIT_1_SET;
            //incase we didn't request for reassoc, it must be a tspec negotiation
            if(!sme_QosCb.ac_info[ac].reassoc_pending)
            {
               sme_QosCb.ac_info[ac].tspec_pending = 1;
            }
             
            pentry->reason = SME_QOS_REASON_SETUP;
         }
         //SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY - could happen if APSD 
         //is setup at assoc on the same AC
         else
         {
            pentry->reason = SME_QOS_REASON_REQ_SUCCESS;
            new_state = SME_QOS_QOS_ON;
            sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0] = 
               sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0];
            if(buffered_cmd && !pentry->hoRenewal)
            {
               QoSCallback(pMac, HDDcontext, 
                           &sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0],
                           status,
                           pentry->QosFlowID);

            }
            pentry->hoRenewal = VOS_FALSE;
         }
         //indicate on which index the flow entry belongs to & add it to the 
         //Flow List at the end
         pentry->tspec_mask = sme_QosCb.ac_info[ac].tspec_mask_status;
         vos_mem_copy(&pentry->QoSInfo, &Tspec_Info, 
                      sizeof(sme_QosWmmTspecInfo));
         csrLLInsertTail(&sme_QosCb.flow_list, &pentry->link, VOS_TRUE);

      }
      else
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: unexpected status = %d returned by \
                   sme_QosSetup\n", status);

         new_state = sme_QosCb.ac_info[ac].curr_state;
         if(buffered_cmd && hoRenewal)
         {
            QoSCallback(pMac, HDDcontext, 
                        &sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0],
                        SME_QOS_STATUS_RELEASE_QOS_LOST_IND,
                        *pQosFlowID);

         }

      }

      break;
   case SME_QOS_HANDOFF:
   case SME_QOS_REQUESTED:
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                "sme_QosInternalSetupReq: buffering the setup request in state = %d\n", 
                sme_QosCb.ac_info[ac].curr_state );

      //buffer cmd
      cmd.command = SME_QOS_SETUP_REQ;
      cmd.u.setupCmdInfo.HDDcontext = HDDcontext;
      cmd.u.setupCmdInfo.pQoSInfo = pQoSInfo;
      cmd.u.setupCmdInfo.QoSCallback = QoSCallback;
      cmd.pMac = pMac;
      cmd.u.setupCmdInfo.UPType = UPType;
      cmd.u.setupCmdInfo.hoRenewal = hoRenewal;
      //assign a flowID while buffering
      *pQosFlowID = sme_QosCb.ac_info[ac].curr_flowID++;
      cmd.u.setupCmdInfo.QosFlowID = *pQosFlowID;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosInternalSetupReq: *pQosFlowID = %d, \
                cmd.u.setupCmdInfo.QosFlowID = %d, sme_QosCb.ac_info[ac].curr_flowID = %d \n",
                *pQosFlowID, cmd.u.setupCmdInfo.QosFlowID, sme_QosCb.ac_info[ac].curr_flowID);

      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: couldn't buffer the setup request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );

         return SME_QOS_STATUS_SETUP_FAILURE_RSP;
      }
      status = SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
      new_state = sme_QosCb.ac_info[ac].curr_state;
      break;
   case SME_QOS_QOS_ON:
      
      //check if multiple flows running on the ac
      if((sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_0] > 0)||
         (sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_1] > 0))
      {
         //do we need to care about the case where APSD needed on ACM = 0 below?
         if(sme_QosIsACM(pMac, sme_QosCb.assoc_Info.pBssDesc, ac, NULL))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                      "sme_QosInternalSetupReq: tspec_mask_status = %d for AC = %d \n", 
                      sme_QosCb.ac_info[ac].tspec_mask_status, ac);

            if(!sme_QosCb.ac_info[ac].tspec_mask_status)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalSetupReq: tspec_mask_status can't be 0 for ac = %d in \
                         state = %d\n", ac, sme_QosCb.ac_info[ac].curr_state );
               //ASSERT
               VOS_ASSERT(0);
               return status;
            }

            // need to aggregate?
            //if both the indices are not in use try to find out if the new 
            //request is looking for a different direction
            if(SME_QOS_TSPEC_MASK_BIT_1_2_SET != sme_QosCb.ac_info[ac].tspec_mask_status)
            {
               //if the direction is different, no aggregation
               if(sme_QosCb.ac_info[ac].
                  curr_QoSInfo[sme_QosCb.ac_info[ac].tspec_mask_status - 1].
                  ts_info.direction != Tspec_Info.ts_info.direction)
               {
                  new_tmask = SME_QOS_TSPEC_MASK_BIT_1_2_SET & ~sme_QosCb.ac_info[ac].tspec_mask_status;
                  sme_QosCb.ac_info[ac].requested_QoSInfo[new_tmask - 1] = 
                     Tspec_Info;
                  sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_BIT_1_2_SET;
                  //indicates NO aggregation
                  tmask = SME_QOS_TSPEC_MASK_CLEAR;
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                            "sme_QosInternalSetupReq: tspec_mask_status = %d for AC = %d with new_tmask =%d\n", 
                            sme_QosCb.ac_info[ac].tspec_mask_status, ac, new_tmask);

               }
               else
               {
                  //aggregate on the index mentioned below
                  tmask = sme_QosCb.ac_info[ac].tspec_mask_status;
               }
            }
            else
            {
               //Since we already have 2 tspec running, pick your best match
               if(sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0].ts_info.direction != 
                  Tspec_Info.ts_info.direction)
               {
                  tmask = SME_QOS_TSPEC_MASK_BIT_2_SET;
               }
               else
               {
                  tmask = SME_QOS_TSPEC_MASK_BIT_1_SET;
               }

            }

         }
         else
         {//ACM = 0
            tmask = SME_QOS_TSPEC_MASK_BIT_1_SET;
         }

         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                   "sme_QosInternalSetupReq:Test: tmask = %d, new_tmask = %d \
                   in state = %d\n", tmask, new_tmask, sme_QosCb.ac_info[ac].curr_state );

         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                   "sme_QosInternalSetupReq: tspec_mask_status = %d for AC = %d\n", 
                   sme_QosCb.ac_info[ac].tspec_mask_status, ac);

         if(tmask)
         {
            //if ACM, update TSPEC & send out a new ADDTS
            if(!HAL_STATUS_SUCCESS(sme_QosAggregateParams(&Tspec_Info, 
                                                          &sme_QosCb.ac_info[ac].
                                                          curr_QoSInfo[tmask - 1],
                                                          &sme_QosCb.ac_info[ac].
                                                          requested_QoSInfo[tmask - 1])))
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalSetupReq: failed to aggregate params\n");
               return SME_QOS_STATUS_SETUP_FAILURE_RSP;
            }

         }
         else
         {
            tmask = new_tmask;
         }
      }
      else
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: no flows running for ac = %d while in \
                   state = %d\n", ac, sme_QosCb.ac_info[ac].curr_state );
         //ASSERT
         VOS_ASSERT(0);
         return status;
      }
      //although aggregating, make sure to request on the correct UP
      sme_QosCb.ac_info[ac].requested_QoSInfo[tmask - 1].ts_info.up =
         Tspec_Info.ts_info.up;
      status = sme_QosSetup(pMac, &sme_QosCb.ac_info[ac].
                            requested_QoSInfo[tmask - 1], ac);

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosInternalSetupReq:Test: sme_QosSetup returned in \
                SME_QOS_QOS_ON state on AC = %d, with status =%d\n", ac, status);

      if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status)||
         (SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
         (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status) ||
         (SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status))
      {
         new_state = SME_QOS_REQUESTED;
         sme_QosCb.ac_info[ac].num_flows[tmask - 1]++;
         //create an entry in the flow list
         pentry= (sme_QosFlowInfoEntry *)
         vos_mem_malloc(sizeof(sme_QosFlowInfoEntry));
         if (!pentry)
         {
            //err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosInternalSetupReq: couldn't allocate memory for the new\
                      entry in the Flow List\n");
            return SME_QOS_STATUS_SETUP_FAILURE_RSP;
         }
         pentry->ac_type = ac;
         pentry->HDDcontext = HDDcontext;
         pentry->QoSCallback = QoSCallback;
         pentry->hoRenewal = hoRenewal;
         if(!buffered_cmd)
         {
            pentry->QosFlowID = sme_QosCb.ac_info[ac].curr_flowID++;
            *pQosFlowID = pentry->QosFlowID;
         }
         else
         {
            pentry->QosFlowID = *pQosFlowID;
         }
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                   "sme_QosInternalSetupReq: *pQosFlowID = %d, \
                   pentry->QosFlowID = %d, sme_QosCb.ac_info[ac].curr_flowID = %d \n",
                   *pQosFlowID, pentry->QosFlowID, sme_QosCb.ac_info[ac].curr_flowID);

         pentry->reason = SME_QOS_REASON_SETUP;

         if((SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status)||
            (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status) ||
            (SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status))
         {
            new_state = sme_QosCb.ac_info[ac].curr_state;
            pentry->reason = SME_QOS_REASON_REQ_SUCCESS;
            sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0] = 
               sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0];
            if(buffered_cmd && !pentry->hoRenewal)
            {
               QoSCallback(pMac, HDDcontext, 
                           &sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0],
                           status,
                           pentry->QosFlowID);

            }

            if(SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status)
            {
               pentry->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
            }
            if(SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status)
            {
               search_key.key.ac_type = ac;
               search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

               if(!pentry->hoRenewal)
               {
                  if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, 
                                                                  sme_QosSetupFnp)))
                  {
                     VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                               "sme_QosInternalSetupReq: couldn't notify other \
                               entries on this AC =%d\n", ac);

                  }
               }
            }
            pentry->hoRenewal = VOS_FALSE;
         }
         else if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status)
         {
            //Need this info when addts comes back from PE to know on which index
            //of the AC the request was from
            sme_QosCb.ac_info[ac].tspec_pending = tmask;
         }

         //indicate on which index the flow entry belongs to & add it to the 
         //Flow List at the end
         pentry->tspec_mask = tmask;

         vos_mem_copy(&pentry->QoSInfo, &Tspec_Info, 
                      sizeof(sme_QosWmmTspecInfo));
         csrLLInsertTail(&sme_QosCb.flow_list, &pentry->link, VOS_TRUE);

         //add the entry to the apsd request list
         if(SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status)
         {
            pEntry1 = csrLLPeekTail(&sme_QosCb.flow_list, VOS_FALSE);
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
            sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry1;
            sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
            sme_QosCb.apsd_req_counter++;
#else
            //In this case, tell HDD about the new parameter
            flow_info2 = GET_BASE_ADDR(pEntry1, sme_QosFlowInfoEntry, link);
            flow_info2->reason = SME_QOS_REASON_REQ_SUCCESS;
            //Is it always SETUP here, can it be MODIFY her????
            flow_info2->QoSCallback(pMac, flow_info2->HDDcontext, 
                       &sme_QosCb.ac_info[flow_info2->ac_type].curr_QoSInfo[flow_info2->tspec_mask - 1],
                       SME_QOS_STATUS_SETUP_SUCCESS_IND,
                       flow_info2->QosFlowID);
#endif
         }

      }
      else
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalSetupReq: unexpected status = %d returned by \
                   sme_QosSetup\n", status);
         
         new_state = sme_QosCb.ac_info[ac].curr_state;
      }
      break;
   default:
    
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalSetupReq: setup requested on unexpected state \
                = %d \n", sme_QosCb.ac_info[ac].curr_state);
      //ASSERT
      VOS_ASSERT(0);
      new_state = sme_QosCb.ac_info[ac].curr_state;
   }

   /* if current state is same as previous no need for transistion,
      if we are doing reassoc & we are already in handoff state, no need to move
      to requested state. But make sure to set the previous state as requested
      state
   */
   if((new_state != sme_QosCb.ac_info[ac].curr_state)&&
      (!(sme_QosCb.ac_info[ac].reassoc_pending && 
         (SME_QOS_HANDOFF == sme_QosCb.ac_info[ac].curr_state))))
   {
      sme_QosStateTransition(new_state, ac);
   }
   
   if(sme_QosCb.ac_info[ac].reassoc_pending && 
      (SME_QOS_HANDOFF == sme_QosCb.ac_info[ac].curr_state))
   {
      sme_QosCb.ac_info[ac].prev_state = SME_QOS_REQUESTED;
   }

   if((SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
      (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status)) 
   {
      (void)sme_QosProcessBufferedCmd();
   }


   return status;
}


/*--------------------------------------------------------------------------
  \brief sme_QosInternalModifyReq() - The SME QoS internal function to request 
  for modification of certain QoS params on a flow running on a particular AC. 
  If the request involves admission control on the requested AC, HDD needs to 
  provide the necessary Traffic Specification (TSPEC) parameters & SME might
  start the renegotiation process through ADDTS.
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param pQoSInfo - Pointer to sme_QosWmmTspecInfo which contains the WMM TSPEC
                    related info as defined above, provided by HDD
  \param QosFlowID - Identification per flow running on each AC generated by 
                      SME. 
                     It is only meaningful if the QoS setup for the flow has 
                     been successful already
                  
  \return SME_QOS_STATUS_SETUP_SUCCESS_RSP - Modification is successful.
  
          Other status means request failed     
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosInternalModifyReq(tpAniSirGlobal pMac, 
                                           sme_QosWmmTspecInfo * pQoSInfo,
                                           v_U32_t QosFlowID,
                                           v_BOOL_t buffered_cmd)
{
   tListElem *pEntry= NULL;
   sme_QosFlowInfoEntry *pNewEntry= NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosEdcaAcType ac;
   sme_QosStates new_state = SME_QOS_CLOSED;
   sme_QosStatusType status = SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
   sme_QosWmmTspecInfo Aggr_Tspec_Info;
   sme_QosSearchInfo search_key;
   sme_QosCmdInfo  cmd;
   tListElem *pEntry1= NULL;
#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
   sme_QosFlowInfoEntry *flow_info2 = NULL;
#endif

   //set the key type & the key to be searched in the Flow List
   search_key.key.QosFlowID = QosFlowID;
   search_key.index = SME_QOS_SEARCH_KEY_INDEX_1;


   //go through the link list to find out the details on the flow
   pEntry = sme_QosFindInFlowList(search_key);

   if(!pEntry)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalModifyReq: no match found for flowID = %d\n", 
                search_key.key.QosFlowID);

      return SME_QOS_STATUS_MODIFY_SETUP_INVALID_PARAMS_RSP;
   }

   //validate QoS params
   if(!sme_QosValidateRequestedParams(pQoSInfo))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalModifyReq: invalid params\n");

      return SME_QOS_STATUS_MODIFY_SETUP_INVALID_PARAMS_RSP;
   }


   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
             "sme_QosInternalModifyReq: Modify requested for flowID = %d\n", 
             QosFlowID);

   // find the AC
   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   ac = flow_info->ac_type;

   sme_QosCb.readyForPowerSave = VOS_FALSE;
   //call PMC's request for power function
   // AND
   //another check is added cosidering the flowing scenario
   //Addts reqest is pending on one AC, while APSD requested on another which 
   //needs a reassoc. Will buffer a request if Addts is pending on any AC, 
   //which will safegaurd the above scenario, & also won't confuse PE with back 
   //to back Addts or Addts followed by Reassoc

   if(sme_QosIsRspPending(ac) || 
      ( eHAL_STATUS_PMC_PENDING == pmcRequestFullPower(pMac, sme_QosPmcFullPowerCallback, pMac, eSME_REASON_OTHER)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                "sme_QosInternalModifyReq: buffering the modify request in state \
                = %d as Addts is pending on other AC/waiting for full power\n", 
                sme_QosCb.ac_info[ac].curr_state );

      //buffer cmd
      cmd.command = SME_QOS_MODIFY_REQ;
      cmd.pMac = pMac;
      cmd.u.modifyCmdInfo.QosFlowID = QosFlowID;
      cmd.u.modifyCmdInfo.pQoSInfo = pQoSInfo;
      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalModifyReq: couldn't buffer the modify request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );
         return SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
      }
      return SME_QOS_STATUS_MODIFY_SETUP_PENDING_RSP;
   }


   //get into the stat m/c to see if the request can be granted
   switch(sme_QosCb.ac_info[ac].curr_state)
   {
   case SME_QOS_QOS_ON:
      //update the entry from Flow List which needed to be modified
      flow_info->reason = SME_QOS_REASON_MODIFY;
      //save the new params adding a new (duplicate) entry in the Flow List
      //Once we have decided on OTA exchange needed or not we can delete the
      //original one from the List
      pNewEntry= (sme_QosFlowInfoEntry *)
      vos_mem_malloc(sizeof(sme_QosFlowInfoEntry));
      if (!pNewEntry)
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalModifyReq: couldn't allocate memory for the new\
                   entry in the Flow List\n");

         return SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
      }
      pNewEntry->ac_type = ac;
      pNewEntry->HDDcontext = flow_info->HDDcontext;
      pNewEntry->QoSCallback = flow_info->QoSCallback;
      pNewEntry->QosFlowID = flow_info->QosFlowID;
      pNewEntry->reason = SME_QOS_REASON_MODIFY_PENDING;
      //since it is a modify request, use the same index on which the flow entry 
      //originally was running & add it to the Flow List at the end
      pNewEntry->tspec_mask = flow_info->tspec_mask;
      vos_mem_copy(&pNewEntry->QoSInfo, pQoSInfo, 
                   sizeof(sme_QosWmmTspecInfo));
      csrLLInsertTail(&sme_QosCb.flow_list, &pNewEntry->link, VOS_TRUE);

      //update TSPEC with the new param set
      if(HAL_STATUS_SUCCESS(sme_QosUpdateParams(ac, pNewEntry->tspec_mask, 
                                                &Aggr_Tspec_Info)))
      {
         vos_mem_copy(&sme_QosCb.ac_info[ac].
                      requested_QoSInfo[pNewEntry->tspec_mask -1], &Aggr_Tspec_Info, 
                      sizeof(sme_QosWmmTspecInfo));
         //if ACM, send out a new ADDTS
         status = sme_QosSetup(pMac, &sme_QosCb.ac_info[ac].requested_QoSInfo[pNewEntry->tspec_mask -1], ac);

         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                   "sme_QosInternalModifyReq:Test: sme_QosSetup returned in \
                   SME_QOS_QOS_ON state on AC = %d, with status =%d\n", ac, status);

         if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status) 
         {
            new_state = SME_QOS_REQUESTED;
            status = SME_QOS_STATUS_MODIFY_SETUP_PENDING_RSP;
            sme_QosCb.ac_info[ac].tspec_pending = pNewEntry->tspec_mask;
         }
         else if((SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
                 (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status) ||
                 (SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status))
         {
            new_state = SME_QOS_QOS_ON;
            if(SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status)
            {
               pNewEntry->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
            }

            //delete the origianl entry in FLOW list which got modified
            search_key.key.ac_type = ac;
            search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;
            if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, sme_QosModifyFnp)))
            {
               status = SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
            }
            if(SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP != status)
            {
               sme_QosCb.ac_info[ac].curr_QoSInfo[pNewEntry->tspec_mask -1] = 
                  sme_QosCb.ac_info[ac].requested_QoSInfo[pNewEntry->tspec_mask -1];

               if(SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status)
               {
                  status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_APSD_SET_ALREADY;
                  search_key.key.ac_type = ac;
                  search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

                  if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, 
                                                                  sme_QosModificationNotifyFnp)))
                  {
                     VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                               "sme_QosInternalModifyReq: couldn't notify all \
                               entries on this AC =%d\n", ac);

                  }

               }
               else if(SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status)
               {
                  status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP;
               }
               else if(SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING == status)
               {
                  status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_PENDING;
               }
            }

            if(buffered_cmd)
            {
               flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                      &sme_QosCb.ac_info[ac].curr_QoSInfo[pNewEntry->tspec_mask -1],
                                      status,
                                      flow_info->QosFlowID);

            }
            //add the entry to the apsd request list
            if(SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_PENDING == status)
            {
               pEntry1 = csrLLPeekTail(&sme_QosCb.flow_list, VOS_FALSE);
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry1;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
               sme_QosCb.apsd_req_counter++;
#else

               //Is pEntry1 == pNewEntry????
               //In this case, tell HDD about the new parameter
               flow_info2 = GET_BASE_ADDR(pEntry1, sme_QosFlowInfoEntry, link);
               flow_info2->reason = SME_QOS_REASON_REQ_SUCCESS;
               flow_info2->QoSCallback(pMac, flow_info2->HDDcontext, 
                          &sme_QosCb.ac_info[flow_info2->ac_type].curr_QoSInfo[flow_info2->tspec_mask - 1],
                          SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND,
                          flow_info2->QosFlowID);
#endif
            }
            
         }
         else
         {
            //err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosInternalModifyReq: unexpected status = %d returned by \
                      sme_QosSetup\n", status);

            new_state = SME_QOS_QOS_ON;
         }

      }
      else
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalModifyReq: sme_QosUpdateParams() failed\n");

         new_state = SME_QOS_LINK_UP;
      }

      /* if we are doing reassoc & we are already in handoff state, no need to move
         to requested state. But make sure to set the previous state as requested
         state
      */
      if(!(sme_QosCb.ac_info[ac].reassoc_pending && 
           (SME_QOS_HANDOFF == sme_QosCb.ac_info[ac].curr_state)))
      {
         sme_QosStateTransition(new_state, ac);      
      }
      else
      {
         sme_QosCb.ac_info[ac].prev_state = SME_QOS_REQUESTED;
      }

      break;
   case SME_QOS_HANDOFF:
   case SME_QOS_REQUESTED:
      //print error msg, let HDD know with a new status code
      //buffer cmd
      cmd.command = SME_QOS_MODIFY_REQ;
      cmd.pMac = pMac;
      cmd.u.modifyCmdInfo.QosFlowID = QosFlowID;
      cmd.u.modifyCmdInfo.pQoSInfo = pQoSInfo;
      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalModifyReq: couldn't buffer the modify request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );
         return SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
      }
      status = SME_QOS_STATUS_MODIFY_SETUP_PENDING_RSP;
      break;
   case SME_QOS_CLOSED:
   case SME_QOS_INIT:
      //print error msg 
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalModifyReq: modify request in unexpected state \
                = %d\n", sme_QosCb.ac_info[ac].curr_state );

      // ASSERT?
      break;
   case SME_QOS_LINK_UP:
   default:
      //print error msg, 
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalModifyReq: modify request in unexpected state \
                = %d\n", sme_QosCb.ac_info[ac].curr_state );

      // ASSERT
      break;
   }

  if((SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
      (SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_APSD_SET_ALREADY == status)) 
   {
      (void)sme_QosProcessBufferedCmd();
   }
   return status;

}

/*--------------------------------------------------------------------------
  \brief sme_QosInternalReleaseReq() - The SME QoS internal function to request 
  for releasing a QoS flow running on a particular AC. 
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param QosFlowID - Identification per flow running on each AC generated by SME 
                     It is only meaningful if the QoS setup for the flow is 
                     successful
  
  \return eHAL_STATUS_SUCCESS - Release is successful.
  
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosInternalReleaseReq(tpAniSirGlobal pMac, 
                                            v_U32_t QosFlowID,
                                            v_BOOL_t buffered_cmd)
{
   tListElem *pEntry= NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosEdcaAcType ac;
   sme_QosStates new_state = SME_QOS_CLOSED;
   sme_QosStatusType status = SME_QOS_STATUS_RELEASE_FAILURE_RSP;
   sme_QosWmmTspecInfo Aggr_Tspec_Info;
   sme_QosSearchInfo search_key;
   sme_QosCmdInfo  cmd;
   tCsrRoamModifyProfileFields modifyProfileFields;
   v_BOOL_t  deltsIssued = VOS_FALSE;
   //set the key type & the key to be searched in the Flow List
   search_key.key.QosFlowID = QosFlowID;
   search_key.index = SME_QOS_SEARCH_KEY_INDEX_1;


   //go through the link list to find out the details on the flow
   pEntry = sme_QosFindInFlowList(search_key);
   
   if(!pEntry)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalReleaseReq: no match found for flowID = %d\n", 
                search_key.key.QosFlowID);

      return SME_QOS_STATUS_RELEASE_INVALID_PARAMS_RSP;
   }
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
             "sme_QosInternalReleaseReq: Release requested for flowID = %d\n", 
             QosFlowID);

   // find the AC
   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   ac = flow_info->ac_type;

   sme_QosCb.readyForPowerSave = VOS_FALSE;
   //call PMC's request for power function
   // AND
   //another check is added cosidering the flowing scenario
   //Addts reqest is pending on one AC, while APSD requested on another which 
   //needs a reassoc. Will buffer a request if Addts is pending on any AC, 
   //which will safegaurd the above scenario, & also won't confuse PE with back 
   //to back Addts or Addts followed by Reassoc

   if(sme_QosIsRspPending(ac) || 
      ( eHAL_STATUS_PMC_PENDING == pmcRequestFullPower(pMac, sme_QosPmcFullPowerCallback, pMac, eSME_REASON_OTHER)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                "sme_QosInternalReleaseReq: buffering the release request in state \
                = %d as Addts is pending on other AC/waiting for full power\n", 
                sme_QosCb.ac_info[ac].curr_state );

      //buffer cmd
      cmd.command = SME_QOS_RELEASE_REQ;
      cmd.pMac = pMac;
      cmd.u.releaseCmdInfo.QosFlowID = QosFlowID;
      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalReleaseReq: couldn't buffer the release request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );
         return SME_QOS_STATUS_RELEASE_FAILURE_RSP;
      }


      return SME_QOS_STATUS_RELEASE_REQ_PENDING_RSP;
   }

   //get into the stat m/c to see if the request can be granted
   switch(sme_QosCb.ac_info[ac].curr_state)
   {
   case SME_QOS_QOS_ON:

      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
                "sme_QosInternalReleaseReq: tspec_mask_status = %d for AC = %d with entry tspec_mask =%d\n", 
                sme_QosCb.ac_info[ac].tspec_mask_status, ac, flow_info->tspec_mask);

      //check if multiple flows running on the ac
      if(sme_QosCb.ac_info[ac].num_flows[flow_info->tspec_mask - 1] > 1)
      {
         //don't want to include the flow in the new TSPEC on which release 
         //is requested
         flow_info->reason = SME_QOS_REASON_RELEASE;
         //update TSPEC
         if(HAL_STATUS_SUCCESS(sme_QosUpdateParams(ac, flow_info->tspec_mask,
                                                   &Aggr_Tspec_Info)))
         {
            vos_mem_copy(&sme_QosCb.ac_info[ac].
                         requested_QoSInfo[flow_info->tspec_mask - 1], 
                         &Aggr_Tspec_Info, sizeof(sme_QosWmmTspecInfo));
            //if ACM, send out a new ADDTS
            status = sme_QosSetup(pMac, &sme_QosCb.ac_info[ac].requested_QoSInfo[flow_info->tspec_mask - 1], ac);

            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosInternalReleaseReq:Test: sme_QosSetup returned in \
                      SME_QOS_QOS_ON state on AC = %d, with status =%d\n", ac, status);

            if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP == status) 
            {
               new_state = SME_QOS_REQUESTED;
               status = SME_QOS_STATUS_RELEASE_REQ_PENDING_RSP;
               sme_QosCb.ac_info[ac].tspec_pending = flow_info->tspec_mask;
            }
            else if((SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP == status) ||
                    (SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status))
            {
               new_state = SME_QOS_QOS_ON;
               sme_QosCb.ac_info[ac].num_flows[flow_info->tspec_mask - 1]--;
               sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1] =
                  sme_QosCb.ac_info[ac].requested_QoSInfo[flow_info->tspec_mask - 1];
               //delete the entry from Flow List
               csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );
               if(SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY == status)
               {
                  search_key.key.ac_type = ac;
                  search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

                  if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, 
                                                                  sme_QosSetupFnp)))
                  {
                     VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                               "sme_QosInternalReleaseReq: couldn't notify other \
                               entries on this AC =%d\n", ac);

                  }
               }
               status = SME_QOS_STATUS_RELEASE_SUCCESS_RSP;
               if(buffered_cmd)
               {
                  flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                         &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                                         status,
                                         flow_info->QosFlowID);

               }
            }
            else
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalReleaseReq: unexpected status = %d returned by \
                         sme_QosSetup\n", status);

               new_state = SME_QOS_LINK_UP;
               sme_QosCb.ac_info[ac].num_flows[flow_info->tspec_mask - 1]--;
               sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1] =
                  sme_QosCb.ac_info[ac].requested_QoSInfo[flow_info->tspec_mask - 1];

               //delete the entry from Flow List
               csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );
               if(buffered_cmd)
               {
                  flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                         &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                                         status,
                                         flow_info->QosFlowID);

               }
            }
         }
         else
         {
            //err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosInternalReleaseReq: sme_QosUpdateParams() failed\n");

            new_state = SME_QOS_LINK_UP;
            if(buffered_cmd)
            {
               flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                      &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                                      status,
                                      flow_info->QosFlowID);

            }

         }
      }
      else
      {
         status = SME_QOS_STATUS_RELEASE_SUCCESS_RSP;
         //check if delts needs to be sent
         if(sme_QosIsACM(pMac, sme_QosCb.assoc_Info.pBssDesc, ac, NULL))
         {
            //check if other UP for this AC is also in use
            if(SME_QOS_TSPEC_MASK_BIT_1_2_SET != sme_QosCb.ac_info[ac].tspec_mask_status)
            {
               sme_QosCb.apsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
               //Also update modifyProfileFields.uapsd_mask in CSR for consistency
               csrGetModifyProfileFields(pMac, &modifyProfileFields);
               modifyProfileFields.uapsd_mask = sme_QosCb.apsd_mask; 
               csrSetModifyProfileFields(pMac, &modifyProfileFields);
               if(!sme_QosCb.apsd_mask)
               {
                  //really don't care when PMC stops it
                  (void)pmcStopUapsd(pMac);
               }
            }
            //send delts
            if(!HAL_STATUS_SUCCESS(qosIssueCommand(pMac, eSmeCommandDelTs, NULL, 0, ac, flow_info->tspec_mask)))
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalReleaseReq: sme_QosDelTsReq() failed\n");
               status = SME_QOS_STATUS_RELEASE_FAILURE_RSP;
            }
            else
            {
               sme_QosCb.ac_info[ac].tspec_mask_status &= SME_QOS_TSPEC_MASK_BIT_1_2_SET & 
                  (~flow_info->tspec_mask);
               deltsIssued = VOS_TRUE;
            }
         }
         else if(sme_QosCb.apsd_mask & (1 << (SME_QOS_EDCA_AC_VO - ac)))
         {
            //reassoc logic
            csrGetModifyProfileFields(pMac, &modifyProfileFields);
            modifyProfileFields.uapsd_mask |= sme_QosCb.apsd_mask;
            modifyProfileFields.uapsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
            sme_QosCb.apsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
            if(!sme_QosCb.apsd_mask)
            {
               //really don't care when PMC stops it
               (void)pmcStopUapsd(pMac);
            }

            if(!HAL_STATUS_SUCCESS(csrReassoc(pMac, &modifyProfileFields, &sme_QosCb.roamID)))
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosInternalReleaseReq:csrReassoc failed\n");
               status = SME_QOS_STATUS_RELEASE_FAILURE_RSP;
            }
            else
            {
               sme_QosCb.ac_info[ac].reassoc_pending = VOS_FALSE;//no need to wait
               sme_QosCb.ac_info[ac].prev_state = SME_QOS_LINK_UP;
               sme_QosCb.ac_info[ac].tspec_pending = 0;
            }

         }
         else
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosInternalReleaseReq: nothing to do for \
                      AC = %d\n", ac);

         }

         if(buffered_cmd)
         {
            flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                   NULL,
                                   status,
                                   flow_info->QosFlowID);

         }

         if(SME_QOS_STATUS_RELEASE_FAILURE_RSP == status)
         {
            break;
         }

         if(sme_QosCb.ac_info[ac].num_flows[(SME_QOS_TSPEC_MASK_BIT_1_2_SET & ~flow_info->tspec_mask) - 1] > 0)
         {
            new_state = SME_QOS_QOS_ON;
         }
         else
         {
            new_state = SME_QOS_LINK_UP;
         }         

         if(VOS_FALSE == deltsIssued)
         {
         vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1], 
                      sizeof(sme_QosWmmTspecInfo));
         }
         vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[flow_info->tspec_mask - 1], 
                      sizeof(sme_QosWmmTspecInfo));
         sme_QosCb.ac_info[ac].num_flows[flow_info->tspec_mask - 1]--;
         //delete the entry from Flow List
         csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );

      }

      /* if we are doing reassoc & we are already in handoff state, no need to move
         to requested state. But make sure to set the previous state as requested
         state
      */
      if(SME_QOS_HANDOFF != sme_QosCb.ac_info[ac].curr_state)
      {
      sme_QosStateTransition(new_state, ac);      
      }
      if(sme_QosCb.ac_info[ac].reassoc_pending)
      {
         sme_QosCb.ac_info[ac].prev_state = SME_QOS_REQUESTED;
      }

      break;
   case SME_QOS_HANDOFF:
   case SME_QOS_REQUESTED:
      //print error msg, let HDD know with a new status code
      //buffer cmd
      cmd.command = SME_QOS_RELEASE_REQ;
      cmd.pMac = pMac;
      cmd.u.releaseCmdInfo.QosFlowID = QosFlowID;
      if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, buffered_cmd)))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosInternalReleaseReq: couldn't buffer the release request \
                   in state = %d\n", sme_QosCb.ac_info[ac].curr_state );
         return SME_QOS_STATUS_RELEASE_FAILURE_RSP;
      }


      status = SME_QOS_STATUS_RELEASE_REQ_PENDING_RSP;
      break;
   case SME_QOS_CLOSED:
   case SME_QOS_INIT:
      //print error msg, ASSERT?
   case SME_QOS_LINK_UP:
   default:
      //print error msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosInternalReleaseReq: modify request in unexpected state \
                = %d\n", sme_QosCb.ac_info[ac].curr_state );

      //ASSERT
      VOS_ASSERT(0);
      break;
   }

  if((SME_QOS_STATUS_RELEASE_SUCCESS_RSP == status)) 
   {
      (void)sme_QosProcessBufferedCmd();
   }
   return status;
}


/*--------------------------------------------------------------------------
  \brief sme_QosSetup() - The internal qos setup function which has the 
  intelligence if the request is NOP, or for APSD and/or need to send out ADDTS.
  It also does the sanity check for QAP, AP supports APSD etc.

  \param pMac - Pointer to the global MAC parameter structure.   
  \param pTspec_Info - Pointer to sme_QosWmmTspecInfo which contains the WMM 
                       TSPEC related info as defined above
  \param ac - Enumeration of the various EDCA Access Categories.
  
  \return SME_QOS_STATUS_SETUP_SUCCESS_RSP if the setup is successful

  The logic used in the code might be confusing. Tryng to cover all the cases 
  here.
  AP supports  App wants   ACM = 1  Already set for APSD   Result
  |    0     |    0     |     0   |          0          |  NO ACM NO APSD
  |    0     |    0     |     0   |          1          |  NO ACM NO APSD/INVALID
  |    0     |    0     |     1   |          0          |  ADDTS
  |    0     |    0     |     1   |          1          |  ADDTS
  |    0     |    1     |     0   |          0          |  FAILURE
  |    0     |    1     |     0   |          1          |  INVALID
  |    0     |    1     |     1   |          0          |  ADDTS
  |    0     |    1     |     1   |          1          |  ADDTS
  |    1     |    0     |     0   |          0          |  NO ACM NO APSD
  |    1     |    0     |     0   |          1          |  NO ACM NO APSD
  |    1     |    0     |     1   |          0          |  ADDTS
  |    1     |    0     |     1   |          1          |  ADDTS
  |    1     |    1     |     0   |          0          |  REASSOC
  |    1     |    1     |     0   |          1          |  NOP: APSD SET ALREADY
  |    1     |    1     |     1   |          0          |  ADDTS
  |    1     |    1     |     1   |          1          |  ADDTS
  
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosStatusType sme_QosSetup(tpAniSirGlobal pMac,
                               sme_QosWmmTspecInfo *pTspec_Info, 
                               sme_QosEdcaAcType ac)
{
   sme_QosStatusType status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   tDot11fBeaconIEs *pIes = NULL;
   //sme_QosWmmTspecInfo *pTspec_Info = &Tspec_Info;
   tCsrRoamModifyProfileFields modifyProfileFields;
   tListElem *pEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   v_U8_t index = 0;

   if(!sme_QosCb.assoc_Info.pBssDesc)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetup:sme_QosCb.assoc_Info.pBssDesc is NULL\n");
      return status;
   }

   if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, sme_QosCb.assoc_Info.pBssDesc, &pIes)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetup:csrGetParsedBssDescriptionIEs() failed\n");
      return status;
   }

   if( !CSR_IS_QOS_BSS(pIes) )
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetup: Ap doesn't support QoS \n");

      if(pIes)
      {
         vos_mem_free(pIes);
      }

      //notify HDD through the synchronous status msg
      return SME_QOS_STATUS_SETUP_NOT_QOS_AP_RSP;
   }

   if(pTspec_Info->max_service_interval || pTspec_Info->min_service_interval)
   {
      pTspec_Info->ts_info.psb = 1;
   }
   else
   {
      pTspec_Info->ts_info.psb = 0;
   }


   do
   {
      //this needs addts
      if(sme_QosIsACM(pMac, sme_QosCb.assoc_Info.pBssDesc, ac, NULL))
      {
         if(pTspec_Info->ts_info.psb && 
            (!pMac->pmc.uapsdEnabled ))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosSetup:Request is looking for APSD but PMC doesn't \
                       have support for APSD\n");
            break;
         }

         if(SME_QOS_MAX_TID == pTspec_Info->ts_info.tid)
         {
            //App didn't set TID, generate one
            pTspec_Info->ts_info.tid = SME_QOS_WMM_UP_NC - pTspec_Info->ts_info.up;
         }
         //addts logic
         if(!HAL_STATUS_SUCCESS(qosIssueCommand(pMac, eSmeCommandAddTs, pTspec_Info, 
                            sizeof( sme_QosWmmTspecInfo ), ac, 0)))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosSetup:sme_QosAddTsReq() failed\n");

            break;
         }
         status = SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
         break;
      }

      //APP is not looking for APSD either
      if(0 == pTspec_Info->ts_info.psb)
      {
         //but check the case, if the setup is called as a result of a release 
         // or modify which boils down to the fact that APSD was set on this AC
         // but no longer needed - so we need a reassoc for the above case to 
         // let the AP know
         if(sme_QosCb.apsd_mask & (1 << (SME_QOS_EDCA_AC_VO - ac)))
         {
            //reassoc logic
            csrGetModifyProfileFields(pMac, &modifyProfileFields);
            modifyProfileFields.uapsd_mask |= sme_QosCb.apsd_mask;
            modifyProfileFields.uapsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
            if(!HAL_STATUS_SUCCESS(csrReassoc(pMac, &modifyProfileFields, &sme_QosCb.roamID)))
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosSetup:csrReassoc failed\n");
               break;
            }
            else
            {
               status = SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
               sme_QosCb.ac_info[ac].reassoc_pending = VOS_TRUE;
            }

         }
         else
         {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                   "sme_QosSetup:Request is not looking for APSD & Admission \
                   control isn't mandetory for the ac, so return right away\n");

         //return success right away
         status = SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP;
         }
         break;
      }
      else if(!(pIes->WMMParams.qosInfo & SME_QOS_AP_SUPPORTS_APSD) &&
              !(pIes->WMMInfoAp.uapsd)) //AP doesn't support APSD
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosSetup:AP doesn't support APSD\n");

         break;
      }
      else if(sme_QosCb.apsd_mask & (1 << (SME_QOS_EDCA_AC_VO - ac))) //already set
      {
         status = SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY;
         if(sme_QosCb.uapsdAlreadyRequested)
         {
            //findout if any requst on the same AC is pending on pmcStartUapsd
            for(index = 0; index < sme_QosCb.apsd_req_counter; index++)
            {
               pEntry = sme_QosCb.uapsdRequest[index].pEntry;
               if(!pEntry)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                            "sme_QosSetup: pEntry is NULL\n");
                  //ASSERT
                  VOS_ASSERT(0);
                  status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
                  break;
               }

               flow_info = GET_BASE_ADDR(pEntry, sme_QosFlowInfoEntry, link);
               if(!flow_info)
               {
                  //Err msg
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                            "sme_QosSetup: couldn't access the QoS \
                            params from the Flow List entry\n");
                  //ASSERT
                  VOS_ASSERT(0);
                  status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
                  break;
               }
               if(flow_info->ac_type == ac)
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                            "sme_QosSetup:Request is looking for APSD & we \
                            sucessfully negotiated with the AP for the ac, \
                            waiting for PMC to put us in UAPSD mode\n");

                  status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
                  break;
               }
            }

         }
         else
         {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                   "sme_QosSetup:Request is looking for APSD but it is already \
                    set for the ac, so return right away\n");
         }
         break;
      }
      else //reassoc
      {
         if(pMac->pmc.uapsdEnabled)
         {

            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosSetup: reassoc needed\n");

            //reassoc logic
            //send down the current mask & the new one on which APSD is requested
            csrGetModifyProfileFields(pMac, &modifyProfileFields);
            modifyProfileFields.uapsd_mask |= sme_QosCb.apsd_mask;
            modifyProfileFields.uapsd_mask |= 1 << (SME_QOS_EDCA_AC_VO - ac);
            if(!HAL_STATUS_SUCCESS(csrReassoc(pMac, &modifyProfileFields, &sme_QosCb.roamID)))
            {
               //err msg
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosSetup:csrReassoc failed\n");
               break;
            }
            else
            {
               status = SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
               sme_QosCb.ac_info[ac].reassoc_pending = VOS_TRUE;
            }
         }
         else
         {
            //err msg: no support for APSD from PMC
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosSetup:no support for APSD or BMPS from PMC\n");
            
         }

      }

   }while(0);

   if(pIes)
   {
      vos_mem_free(pIes);
   }

   return status;

}

/*--------------------------------------------------------------------------
  \brief sme_QosAddTsReq() - To send down the ADDTS request with TSPEC params
  to PE 
  
 
  \param pMac - Pointer to the global MAC parameter structure.  
  \param pTspec_Info - Pointer to sme_QosWmmTspecInfo which contains the WMM 
                       TSPEC related info as defined above
  \param ac - Enumeration of the various EDCA Access Categories.

  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosAddTsReq(tpAniSirGlobal pMac, 
                           sme_QosWmmTspecInfo * pTspec_Info,
                           sme_QosEdcaAcType ac)
{
   tSirAddtsReq *pMsg = NULL;
   eHalStatus status = eHAL_STATUS_FAILURE;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   WLAN_VOS_DIAG_EVENT_DEF(qos, vos_event_wlan_qos_payload_type);
#endif

   pMsg = (tSirAddtsReq *)vos_mem_malloc(sizeof(tSirAddtsReq));
   if (!pMsg)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosAddTsReq: couldn't allocate memory for the msg buffer\n");

      return eHAL_STATUS_FAILURE;
   }

   vos_mem_zero(pMsg, sizeof(tSirAddtsReq));

   pMsg->messageType = pal_cpu_to_be16((v_U16_t)eWNI_SME_ADDTS_REQ);
   pMsg->length = sizeof(tSirAddtsReq);
   pMsg->timeout = 0;
   pMsg->rspReqd = VOS_TRUE;
   pMsg->req.dialogToken = (v_U8_t)(sme_QosCb.ac_info[ac].curr_flowID + 
      SME_QOS_TSPEC_DLG_TKN_OFFSET);

   pMsg->req.tspec.delayBound = pTspec_Info->delay_bound;
   pMsg->req.tspec.inactInterval = pTspec_Info->inactivity_interval;
   pMsg->req.tspec.length = SME_QOS_TSPEC_IE_LENGTH;
   pMsg->req.tspec.maxBurstSz = pTspec_Info->max_burst_size;
   pMsg->req.tspec.maxMsduSz = pTspec_Info->maximum_msdu_size;
   pMsg->req.tspec.maxSvcInterval = pTspec_Info->max_service_interval;
   pMsg->req.tspec.meanDataRate = pTspec_Info->mean_data_rate;
   pMsg->req.tspec.mediumTime = pTspec_Info->medium_time;
   pMsg->req.tspec.minDataRate = pTspec_Info->min_data_rate;
   pMsg->req.tspec.minPhyRate = pTspec_Info->min_phy_rate;
   pMsg->req.tspec.minSvcInterval = pTspec_Info->min_service_interval;
   pMsg->req.tspec.nomMsduSz = pTspec_Info->nominal_msdu_size;
   pMsg->req.tspec.peakDataRate = pTspec_Info->peak_data_rate;
   pMsg->req.tspec.surplusBw = pTspec_Info->surplus_bw_allowance;
   pMsg->req.tspec.suspendInterval = pTspec_Info->suspension_interval;
   pMsg->req.tspec.svcStartTime = pTspec_Info->svc_start_time;
   pMsg->req.tspec.tsinfo.traffic.direction = pTspec_Info->ts_info.direction;
   pMsg->req.tspec.tsinfo.traffic.psb = pTspec_Info->ts_info.psb;
   pMsg->req.tspec.tsinfo.traffic.tsid = pTspec_Info->ts_info.tid;
   pMsg->req.tspec.tsinfo.traffic.userPrio = pTspec_Info->ts_info.up;
   pMsg->req.tspec.tsinfo.traffic.accessPolicy = SME_QOS_ACCESS_POLICY_EDCA;
   pMsg->req.tspec.type = SME_QOS_TSPEC_IE_TYPE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosAddTsReq:Test: up passed down = %d %d\n", 
             pTspec_Info->ts_info.up, pMsg->req.tspec.tsinfo.traffic.userPrio);

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosAddTsReq:Test: tid passed down = %d %d\n", 
             pTspec_Info->ts_info.tid, pMsg->req.tspec.tsinfo.traffic.tsid);

   if(HAL_STATUS_SUCCESS(palSendMBMessage(pMac->hHdd, pMsg)))
   {
      status = eHAL_STATUS_SUCCESS;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAddTsReq:Test: sent down a ADDTS req to PE\n");
         //event: EVENT_WLAN_QOS
#ifdef FEATURE_WLAN_DIAG_SUPPORT          
      qos.eventId = SME_QOS_DIAG_ADDTS_REQ;
      qos.reasonCode = SME_QOS_DIAG_USER_REQUESTED;
      WLAN_VOS_DIAG_EVENT_REPORT(&qos, EVENT_WLAN_QOS);
#endif //FEATURE_WLAN_DIAG_SUPPORT

   }

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosDelTsReq() - To send down the DELTS request with TSPEC params
  to PE 
  
 
  \param pMac - Pointer to the global MAC parameter structure.  
  \param ac - Enumeration of the various EDCA Access Categories.
  \param tspec_mask - on which tspec per AC, the delts is requested
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosDelTsReq(tpAniSirGlobal pMac, sme_QosEdcaAcType ac, v_U8_t tspec_mask)
{
   tSirDeltsReq *pMsg = NULL;
   eHalStatus status = eHAL_STATUS_FAILURE;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   WLAN_VOS_DIAG_EVENT_DEF(qos, vos_event_wlan_qos_payload_type);
#endif

   pMsg = (tSirDeltsReq *)vos_mem_malloc(sizeof(tSirDeltsReq));

   if (!pMsg)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDelTsReq: couldn't allocate memory for the msg buffer\n");

      return eHAL_STATUS_FAILURE;
   }

   vos_mem_zero(pMsg, sizeof(tSirDeltsReq));

   pMsg->messageType = pal_cpu_to_be16((v_U16_t)eWNI_SME_DELTS_REQ);
   pMsg->length = sizeof(tSirDeltsReq);
   pMsg->rspReqd = VOS_TRUE;

   pMsg->req.tspec.delayBound = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].delay_bound;
   pMsg->req.tspec.inactInterval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].inactivity_interval;

   pMsg->req.tspec.length = SME_QOS_TSPEC_IE_LENGTH;
   pMsg->req.tspec.maxBurstSz = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].max_burst_size;
   pMsg->req.tspec.maxMsduSz = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].maximum_msdu_size;
   pMsg->req.tspec.maxSvcInterval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].max_service_interval;
   pMsg->req.tspec.meanDataRate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].mean_data_rate;
   pMsg->req.tspec.mediumTime = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].medium_time;
   pMsg->req.tspec.minDataRate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].min_data_rate;
   pMsg->req.tspec.minPhyRate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].min_phy_rate;
   pMsg->req.tspec.minSvcInterval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].min_service_interval;
   pMsg->req.tspec.nomMsduSz = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].nominal_msdu_size;
   pMsg->req.tspec.peakDataRate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].peak_data_rate;
   pMsg->req.tspec.surplusBw = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].surplus_bw_allowance;
   pMsg->req.tspec.suspendInterval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].suspension_interval;
   pMsg->req.tspec.svcStartTime = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].svc_start_time;
   pMsg->req.tspec.tsinfo.traffic.direction = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.direction;
   pMsg->req.tspec.tsinfo.traffic.psb = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.psb;
   pMsg->req.tspec.tsinfo.traffic.tsid = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.tid;
   pMsg->req.tspec.tsinfo.traffic.userPrio = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.up;
   pMsg->req.tspec.tsinfo.traffic.accessPolicy = SME_QOS_ACCESS_POLICY_EDCA;
   pMsg->req.tspec.type = SME_QOS_TSPEC_IE_TYPE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosdelTsReq:Test: up passed down = %d %d\n", 
             sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.up, pMsg->req.tspec.tsinfo.traffic.userPrio);

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosdelTsReq:Test: tid passed down = %d %d\n", 
             sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.tid, pMsg->req.tspec.tsinfo.traffic.tsid);

   vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1], 
                sizeof(sme_QosWmmTspecInfo));

   if(HAL_STATUS_SUCCESS(palSendMBMessage(pMac->hHdd, pMsg)))
   {
      status = eHAL_STATUS_SUCCESS;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosDelTsReq:Test: sent down a DELTS req to PE\n");
         //event: EVENT_WLAN_QOS
#ifdef FEATURE_WLAN_DIAG_SUPPORT          
      qos.eventId = SME_QOS_DIAG_DELTS;
      qos.reasonCode = SME_QOS_DIAG_USER_REQUESTED;
      WLAN_VOS_DIAG_EVENT_REPORT(&qos, EVENT_WLAN_QOS);
#endif //FEATURE_WLAN_DIAG_SUPPORT
   }

   return status;
}


/*--------------------------------------------------------------------------
  \brief sme_QosProcessAddTsRsp() - Function to process the
  eWNI_SME_ADDTS_RSP came from PE 
  

  \param pMac - Pointer to the global MAC parameter structure.  
  \param pMsgBuf - Pointer to the msg buffer came from PE.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessAddTsRsp(tpAniSirGlobal pMac, void *pMsgBuf)
{
   tpSirAddtsRsp paddts_rsp = (tpSirAddtsRsp)pMsgBuf;
   eHalStatus status = eHAL_STATUS_FAILURE;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   WLAN_VOS_DIAG_EVENT_DEF(qos, vos_event_wlan_qos_payload_type);
#endif

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessAddTsRsp:Test: invoked\n");

   if(paddts_rsp->rc)
   {
         //event: EVENT_WLAN_QOS
#ifdef FEATURE_WLAN_DIAG_SUPPORT          
      qos.eventId = SME_QOS_DIAG_ADDTS_RSP;
      qos.reasonCode = SME_QOS_DIAG_ADDTS_REFUSED;
      WLAN_VOS_DIAG_EVENT_REPORT(&qos, EVENT_WLAN_QOS);
#endif //FEATURE_WLAN_DIAG_SUPPORT

      status = sme_QosProcessAddTsFailureRsp(pMac, &paddts_rsp->rsp);
   }
   else
   {
      status = sme_QosProcessAddTsSuccessRsp(pMac, &paddts_rsp->rsp);
   }
   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessDelTsRsp() - Function to process the
  eWNI_SME_DELTS_RSP came from PE 
  

  \param pMac - Pointer to the global MAC parameter structure.  
  \param pMsgBuf - Pointer to the msg buffer came from PE.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessDelTsRsp(tpAniSirGlobal pMac, void *pMsgBuf)
{
   //currently a dummy fn.
   // msg
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessDelTsRsp:Test: invoked\n");
   (void)sme_QosProcessBufferedCmd();
   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessDelTsInd() - Function to process the
  eWNI_SME_DELTS_IND came from PE 
  
  Since it's a DELTS indication from AP, will notify all the flows running on 
  this AC about QoS release

  \param pMac - Pointer to the global MAC parameter structure.  
  \param pMsgBuf - Pointer to the msg buffer came from PE.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessDelTsInd(tpAniSirGlobal pMac, void *pMsgBuf)
{
   tpSirDeltsRsp pdeltsind = (tpSirDeltsRsp)pMsgBuf ;
   sme_QosEdcaAcType ac;
   sme_QosSearchInfo search_key;
   sme_QosWmmUpType up = (sme_QosWmmUpType)pdeltsind->rsp.tspec.tsinfo.traffic.userPrio;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
   WLAN_VOS_DIAG_EVENT_DEF(qos, vos_event_wlan_qos_payload_type);
#endif

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessDelTsInd:Test: invoked fro UP = %d\n", up);

   ac = sme_QosUpToAc(up);
   if(SME_QOS_EDCA_AC_MAX == ac)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessDelTsInd: invalid AC = %d\n", ac );

      return eHAL_STATUS_FAILURE;
   }
   //set the key type & the key to be searched in the Flow List
   search_key.key.ac_type = ac;
   search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

   //find all Flows on the perticular AC & delete them, also send HDD indication
   // through the callback it registered per request
   if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, sme_QosDelTsIndFnp)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessDelTsInd: no match found for ac = %d\n", 
                search_key.key.ac_type);
      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }
   //clean up the CB
   vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                sizeof(sme_QosWmmTspecInfo));
   vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                sizeof(sme_QosWmmTspecInfo));
   vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                sizeof(sme_QosWmmTspecInfo));
   vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                sizeof(sme_QosWmmTspecInfo));
   sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_CLEAR;
   sme_QosCb.ac_info[ac].tspec_pending = 0;
         //event: EVENT_WLAN_QOS
#ifdef FEATURE_WLAN_DIAG_SUPPORT          
      qos.eventId = SME_QOS_DIAG_DELTS;
      qos.reasonCode = SME_QOS_DIAG_DELTS_IND_FROM_AP;
      WLAN_VOS_DIAG_EVENT_REPORT(&qos, EVENT_WLAN_QOS);
#endif //FEATURE_WLAN_DIAG_SUPPORT


   sme_QosStateTransition(SME_QOS_LINK_UP, ac);       

   (void)sme_QosProcessBufferedCmd();

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessAssocCompleteEv() - Function to process the
  SME_QOS_CSR_ASSOC_COMPLETE event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessAssocCompleteEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   sme_QosEdcaAcType ac = SME_QOS_EDCA_AC_BE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessAssocCompleteEv:Test: invoked\n");

   if(((SME_QOS_INIT == sme_QosCb.ac_info[SME_QOS_EDCA_AC_BE].curr_state)&&
      (SME_QOS_INIT == sme_QosCb.ac_info[SME_QOS_EDCA_AC_BK].curr_state)&&
      (SME_QOS_INIT == sme_QosCb.ac_info[SME_QOS_EDCA_AC_VI].curr_state)&&
       (SME_QOS_INIT == sme_QosCb.ac_info[SME_QOS_EDCA_AC_VO].curr_state)) ||
       (sme_QosCb.handoffRequested))
   {
      //get the association info
      if(!pEvent_info)
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessAssocCompleteEv: pEvent_info is NULL\n");
         return status;
      }
      if(!((sme_QosAssocInfo *)pEvent_info)->pBssDesc)
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessAssocCompleteEv: pBssDesc is NULL\n");
         
         return status;
      }
      if((sme_QosCb.assoc_Info.pBssDesc) &&
         (csrIsBssidMatch(pMac, (tCsrBssid *)&sme_QosCb.assoc_Info.pBssDesc->bssId, 
                          (tCsrBssid *) &(((sme_QosAssocInfo *)pEvent_info)->pBssDesc->bssId))))
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessAssocCompleteEv: assoc with the same BSS, no update needed\n");
      }
      else
      {
         status = sme_QosSaveAssocInfo(pEvent_info);
      }
      
   }
   else
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAssocCompleteEv: wrong state: %d, %d, %d, %d\n",
                sme_QosCb.ac_info[SME_QOS_EDCA_AC_BE].curr_state, sme_QosCb.ac_info[SME_QOS_EDCA_AC_BK].curr_state,
                sme_QosCb.ac_info[SME_QOS_EDCA_AC_VI].curr_state, sme_QosCb.ac_info[SME_QOS_EDCA_AC_VO].curr_state);

      //ASSERT
      VOS_ASSERT(0);
      return status;
   }

   if(sme_QosCb.handoffRequested)
   {
      sme_QosCb.handoffRequested = VOS_FALSE;
      //renew all flows
      (void)sme_QosProcessBufferedCmd();
      status = eHAL_STATUS_SUCCESS;
   }
   else
   {
      for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
      {
         switch(sme_QosCb.ac_info[ac].curr_state)
         {
            case SME_QOS_INIT:
               sme_QosStateTransition(SME_QOS_LINK_UP,
                                   ac);   
               break;
            case SME_QOS_LINK_UP:
            case SME_QOS_REQUESTED:
            case SME_QOS_QOS_ON:
            case SME_QOS_HANDOFF:
            case SME_QOS_CLOSED:
            default:
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosProcessAssocCompleteEv: wrong state = %d\n",
                         sme_QosCb.ac_info[ac].curr_state);
               //ASSERT
               VOS_ASSERT(0);
               break;
         }

      }
   }
   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessReassocReqEv() - Function to process the
  SME_QOS_CSR_REASSOC_REQ event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessReassocReqEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   sme_QosEdcaAcType ac;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessReassocReqEv:Test: invoked\n");

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
            sme_QosStateTransition(SME_QOS_HANDOFF,
                                   ac);
            break;
         case SME_QOS_HANDOFF:
            //print error msg
         case SME_QOS_CLOSED:
         case SME_QOS_INIT:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessReassocReqEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);
            //ASSERT
            VOS_ASSERT(0);
            break;
      }
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessReassocSuccessEv() - Function to process the
  SME_QOS_CSR_REASSOC_COMPLETE event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessReassocSuccessEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   sme_QosEdcaAcType ac, ac_index;
   sme_QosSearchInfo search_key;
   sme_QosSearchInfo search_key1;
   eHalStatus status = eHAL_STATUS_FAILURE;
   tListElem *pEntry= NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;


   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessReassocSuccessEv:Test: invoked\n");


   //get the association info
   if(!pEvent_info)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessReassocSuccessEv: pEvent_info is NULL\n");

   }

   if(!((sme_QosAssocInfo *)pEvent_info)->pBssDesc)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessReassocSuccessEv: pBssDesc is NULL\n");

      return status;
   }
   status = sme_QosSaveAssocInfo(pEvent_info);

   if(status)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessReassocSuccessEv: sme_QosSaveAssocInfo() failed\n");

   }

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_HANDOFF:
            sme_QosStateTransition(sme_QosCb.ac_info[ac].prev_state,
                                   ac);
            //for which ac APSD (hence the reassoc) is requested
            if(sme_QosCb.ac_info[ac].reassoc_pending)
            {
               //update the apsd mask in CB - make sure to take care of the case
               //where we are reseting the bit in apsd_mask
               if(sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0].ts_info.psb)
               {
                  sme_QosCb.apsd_mask |= 1 << (SME_QOS_EDCA_AC_VO - ac);
                  //vote for power save since we might be looking for APSD
                  sme_QosCb.readyForPowerSave = VOS_TRUE;
               }
               else
               {
                  sme_QosCb.apsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
               }
               sme_QosCb.ac_info[ac].reassoc_pending = VOS_FALSE;
               //during setup it gets set as addts & reassoc both gets a pending flag
               //sme_QosCb.ac_info[ac].tspec_pending = 0;
               sme_QosStateTransition(SME_QOS_QOS_ON,
                                      ac);
               // notify HDD with new Service Interval
               sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0] = 
                  sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0];
               //set the key type & the key to be searched in the Flow List
               search_key.key.ac_type = ac;
               search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

               //notify PMC that reassoc is done for APSD on certain AC??

               //set the horenewal field in control block if needed
               search_key1.index = SME_QOS_SEARCH_KEY_INDEX_3;
               search_key1.key.reason = SME_QOS_REASON_SETUP;
               for(ac_index = SME_QOS_EDCA_AC_BE; ac_index < SME_QOS_EDCA_AC_MAX; ac_index++)
               {
                  pEntry = sme_QosFindInFlowList(search_key1);
                  if(pEntry)
                  {
                     flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
                     if(flow_info->ac_type == ac)
                     {
                        sme_QosCb.ac_info[ac].hoRenewal = flow_info->hoRenewal;
                        break;
                     }
                  }
               }

               //notify HDD the success for the requested flow 
               //notify all the other flows running on the AC that QoS got modified
               if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, sme_QosReassocSuccessEvFnp)))
               {
                  VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                            "sme_QosProcessReassocSuccessEv: no match found for ac = %d\n", 
                            search_key.key.ac_type);
                  //ASSERT
                  VOS_ASSERT(0);
                  return eHAL_STATUS_FAILURE;
               }
               sme_QosCb.ac_info[ac].hoRenewal = VOS_FALSE;
               vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                            sizeof(sme_QosWmmTspecInfo));

            }
            status = eHAL_STATUS_SUCCESS;
            break;
         case SME_QOS_INIT:
         case SME_QOS_CLOSED:
            //NOP
            status = eHAL_STATUS_SUCCESS;
            break;
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessReassocSuccessEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);

            //ASSERT
            VOS_ASSERT(0);
            break;
      }

   }

   (void)sme_QosProcessBufferedCmd();

   return status;
}


/*--------------------------------------------------------------------------
  \brief sme_QosProcessReassocFailureEv() - Function to process the
  SME_QOS_CSR_REASSOC_FAILURE event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessReassocFailureEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   sme_QosEdcaAcType ac;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessReassocFailureEv:Test: invoked\n");

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_HANDOFF:
            sme_QosStateTransition(SME_QOS_INIT,
                                   ac);       
            if(sme_QosCb.ac_info[ac].reassoc_pending)
            {
               sme_QosCb.ac_info[ac].reassoc_pending = VOS_FALSE;
            }
            vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                         sizeof(sme_QosWmmTspecInfo));
            sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_CLEAR;
            sme_QosCb.ac_info[ac].tspec_pending = 0;
            sme_QosCb.ac_info[ac].curr_flowID = 0;
            sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_0] = 0;
            sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_1] = 0;
            break;
         case SME_QOS_INIT:
         case SME_QOS_CLOSED:
            //NOP
            break;
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessReassocFailureEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);

            //ASSERT
            VOS_ASSERT(0);
            break;
      }

   }
   //need to clean up flows
   sme_QosDeleteExistingFlows(pMac);

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessHandoffAssocReqEv() - Function to process the
  SME_QOS_CSR_HANDOFF_ASSOC_REQ event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessHandoffAssocReqEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   v_U8_t ac;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessHandoffAssocReqEv:Test: invoked\n");

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
            sme_QosStateTransition(SME_QOS_HANDOFF,
                                   ac);
            break;
         case SME_QOS_HANDOFF:
            //print error msg
         case SME_QOS_CLOSED:
         case SME_QOS_INIT:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessHandoffAssocReqEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);
            //ASSERT
            VOS_ASSERT(0);
            break;
      }
   }

   sme_QosCb.handoffRequested = VOS_TRUE;

   sme_QosCb.apsd_mask = 0;
   sme_QosCb.apsd_req_counter = 0;
   sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
   vos_mem_zero(sme_QosCb.uapsdRequest, sizeof(sme_QosPmcStartUAPSDRequest) *
                SME_QOS_MAX_APSD_REQ_PENDING);
   //ask PMC to stop UAPSD, incase we have already requsted one.
   (void)pmcStopUapsd(pMac);

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessHandoffSuccessEv() - Function to process the
  SME_QOS_CSR_HANDOFF_COMPLETE event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessHandoffSuccessEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   v_U8_t ac;
   eHalStatus status = eHAL_STATUS_FAILURE;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessHandoffSuccessEv:Test: invoked\n");


   //go back to original state before handoff
   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_HANDOFF:
            sme_QosStateTransition(sme_QosCb.ac_info[ac].prev_state,
                                   ac);
            //we will retry for the requested flow(s) with the new AP
            if(SME_QOS_REQUESTED == sme_QosCb.ac_info[ac].curr_state)
            {
               sme_QosCb.ac_info[ac].curr_state = SME_QOS_LINK_UP;
            }
            status = eHAL_STATUS_SUCCESS;
            break;
         case SME_QOS_INIT:
         case SME_QOS_CLOSED:
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessHandoffSuccessEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);

            //ASSERT
            VOS_ASSERT(0);
            break;
      }

   }

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessHandoffFailureEv() - Function to process the
  SME_QOS_CSR_HANDOFF_FAILURE event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessHandoffFailureEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   v_U8_t ac;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessHandoffFailureEv:Test: invoked\n");

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      switch(sme_QosCb.ac_info[ac].curr_state)
      {
         case SME_QOS_HANDOFF:
            sme_QosStateTransition(SME_QOS_INIT,
                                   ac);       
            sme_QosCb.handoffRequested = VOS_FALSE;
            //need to clean up flows: TODO
            vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                         sizeof(sme_QosWmmTspecInfo));
            vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_1], 
                         sizeof(sme_QosWmmTspecInfo));
            sme_QosCb.ac_info[ac].tspec_mask_status = SME_QOS_TSPEC_MASK_CLEAR;
            sme_QosCb.ac_info[ac].tspec_pending = 0;
            sme_QosCb.ac_info[ac].curr_flowID = 0;
            sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_0] = 0;
            sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_1] = 0;
            break;
         case SME_QOS_INIT:
         case SME_QOS_CLOSED:
         case SME_QOS_LINK_UP:
         case SME_QOS_REQUESTED:
         case SME_QOS_QOS_ON:
         default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessHandoffFailureEv: wrong state = %d\n",
                      sme_QosCb.ac_info[ac].curr_state);

            //ASSERT
            VOS_ASSERT(0);
            break;
      }

   }
   //clean up the assoc info
   if(sme_QosCb.assoc_Info.pBssDesc)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pBssDesc);
      sme_QosCb.assoc_Info.pBssDesc = NULL;
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessDisconnectEv() - Function to process the
  SME_QOS_CSR_DISCONNECT_REQ or  SME_QOS_CSR_DISCONNECT_IND event indication 
  from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessDisconnectEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   sme_QosEdcaAcType ac;
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessDisconnectEv:Test: invoked\n");

   if(sme_QosCb.handoffRequested)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosProcessDisconnectEv:no need for state transition, should \
                already be in handoff state\n");
      VOS_ASSERT(sme_QosCb.ac_info[0].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[1].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[2].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[3].curr_state == SME_QOS_HANDOFF);

      return eHAL_STATUS_SUCCESS;
   }

   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {//cleanup control blk
      vos_mem_zero(&sme_QosCb.ac_info[ac], sizeof(sme_QosACInfo));
      sme_QosStateTransition(SME_QOS_INIT,
                             ac);       
   }

   sme_QosCb.apsd_mask = 0;
   sme_QosCb.apsd_req_counter = 0;
   sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
   vos_mem_zero(sme_QosCb.uapsdRequest, sizeof(sme_QosPmcStartUAPSDRequest) *
                SME_QOS_MAX_APSD_REQ_PENDING);
   sme_QosCb.handoffRequested = VOS_FALSE;
   sme_QosCb.readyForPowerSave = VOS_TRUE;
   sme_QosCb.roamID = 0;
   //need to clean up buffered req
   sme_QosDeleteBufferedRequests(pMac);
   //need to clean up flows
   sme_QosDeleteExistingFlows(pMac);
   //clean up the assoc info
   if(sme_QosCb.assoc_Info.pBssDesc)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pBssDesc);
      sme_QosCb.assoc_Info.pBssDesc = NULL;
   }
   //ask PMC to stop UAPSD, incase we have already requsted one.
   (void)pmcStopUapsd(pMac);

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessJoinReqEv() - Function to process the
  SME_QOS_CSR_JOIN_REQ event indication from CSR

  \param pEvent_info - Pointer to relevant info from CSR.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessJoinReqEv(tpAniSirGlobal pMac, void * pEvent_info)
{
   sme_QosEdcaAcType ac;
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessJoinReqEv:Test: invoked\n");

   if(sme_QosCb.handoffRequested)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosProcessJoinReqEv:no need for state transition, should \
                already be in handoff state\n");
      VOS_ASSERT(sme_QosCb.ac_info[0].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[1].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[2].curr_state == SME_QOS_HANDOFF);
      VOS_ASSERT(sme_QosCb.ac_info[3].curr_state == SME_QOS_HANDOFF);
      //buffer the existing flows to be renewed after ahndoff is done
      sme_QosBufferExistingFlows(pMac);
      //clean up the control block partially for handoff
      sme_QosCleanupCtrlBlkForHandoff(pMac);
      return eHAL_STATUS_SUCCESS;
   }

   
   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      sme_QosStateTransition(SME_QOS_INIT,
                             ac);       
   }
   //clean up the assoc info if already set
   if(sme_QosCb.assoc_Info.pBssDesc)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pBssDesc);
      sme_QosCb.assoc_Info.pBssDesc = NULL;
   }
   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessAddTsFailureRsp() - Function to process the
  Addts request failure response came from PE 
  
  We will notify HDD only for the requested Flow, other Flows running on the AC 
  stay intact
  
  \param pMac - Pointer to the global MAC parameter structure.  
  \param pRsp - Pointer to the addts response structure came from PE.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessAddTsFailureRsp(tpAniSirGlobal pMac, 
                                         tSirAddtsRspInfo * pRsp)
{
   sme_QosEdcaAcType ac;
   sme_QosSearchInfo search_key;
   v_U8_t tspec_pending;
   sme_QosWmmUpType up = (sme_QosWmmUpType)pRsp->tspec.tsinfo.traffic.userPrio;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessAddTsFailureRsp:Test: invoked for UP = %d\n", up);

   ac = sme_QosUpToAc(up);
   if(SME_QOS_EDCA_AC_MAX == ac)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsFailureRsp: invalid AC = %d\n", ac );

      return eHAL_STATUS_FAILURE;
   }
   //set the key type & the key to be searched in the Flow List
   search_key.key.ac_type = ac;
   search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;
   tspec_pending = sme_QosCb.ac_info[ac].tspec_pending;

   if(!tspec_pending)
   {
      //ASSERT
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsFailureRsp: no ADDTS is requested on AC = %d\n",
                ac);
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }
   if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, sme_QosAddTsFailureFnp)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsFailureRsp: no match found for ac = %d\n", 
                search_key.key.ac_type);
      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[tspec_pending - 1], 
                sizeof(sme_QosWmmTspecInfo));


   if((!sme_QosCb.ac_info[ac].num_flows[0])&&
      (!sme_QosCb.ac_info[ac].num_flows[1]))
   {
      sme_QosCb.ac_info[ac].tspec_mask_status &= SME_QOS_TSPEC_MASK_BIT_1_2_SET & 
         (~sme_QosCb.ac_info[ac].tspec_pending);

   sme_QosStateTransition(SME_QOS_LINK_UP, ac);       
   }
   else
   {
      sme_QosStateTransition(SME_QOS_QOS_ON, ac);       
   }

   sme_QosCb.ac_info[ac].tspec_pending = 0;

   (void)sme_QosProcessBufferedCmd();

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessAddTsSuccessRsp() - Function to process the
  Addts request success response came from PE 
  
  We will notify HDD with addts success for the requested Flow, & for other 
  Flows running on the AC we will send an addts modify status 
  
  
  \param pMac - Pointer to the global MAC parameter structure.  
  \param pRsp - Pointer to the addts response structure came from PE.   
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessAddTsSuccessRsp(tpAniSirGlobal pMac, 
                                         tSirAddtsRspInfo * pRsp)
{
   sme_QosEdcaAcType ac, ac_index;
   sme_QosSearchInfo search_key;
   sme_QosSearchInfo search_key1;
   v_U8_t tspec_pending;
   tListElem *pEntry= NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosWmmUpType up = (sme_QosWmmUpType)pRsp->tspec.tsinfo.traffic.userPrio;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
   WLAN_VOS_DIAG_EVENT_DEF(qos, vos_event_wlan_qos_payload_type);
   vos_log_qos_tspec_pkt_type *log_ptr = NULL;
#endif //FEATURE_WLAN_DIAG_SUPPORT
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessAddTsSuccessRsp:Test: invoked for up =%d\n", up);

   ac = sme_QosUpToAc(up);
   if(SME_QOS_EDCA_AC_MAX == ac)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsSuccessRsp: invalid AC = %d\n", ac );

      return eHAL_STATUS_FAILURE;
   }

   //set the key type & the key to be searched in the Flow List
   search_key.key.ac_type = ac;
   search_key.index = SME_QOS_SEARCH_KEY_INDEX_2;

   tspec_pending = sme_QosCb.ac_info[ac].tspec_pending;

   if(!tspec_pending)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsSuccessRsp: no ADDTS is requested on AC = %d\n",
                ac);

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   //App is looking for APSD or the App which was looking for APSD has been 
   //released, so STA re-negotiated with AP
   if(sme_QosCb.ac_info[ac].requested_QoSInfo[tspec_pending - 1].ts_info.psb)
   {
      //update the apsd mask in CB
      sme_QosCb.apsd_mask |= 1 << (SME_QOS_EDCA_AC_VO - ac);
      //vote for power save since we might be looking for APSD
      sme_QosCb.readyForPowerSave = VOS_TRUE;
   }
   else
   {
      if(!sme_QosCb.ac_info[ac].requested_QoSInfo
         [(SME_QOS_TSPEC_MASK_BIT_1_2_SET & ~tspec_pending) - 1].ts_info.psb)
      {
         //update the apsd mask in CB
         sme_QosCb.apsd_mask &= ~(1 << (SME_QOS_EDCA_AC_VO - ac));
      }
   }

   sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1] = 
      sme_QosCb.ac_info[ac].requested_QoSInfo[tspec_pending - 1];

   //set the horenewal field in control block if needed
   search_key1.index = SME_QOS_SEARCH_KEY_INDEX_3;
   search_key1.key.reason = SME_QOS_REASON_SETUP;
   for(ac_index = SME_QOS_EDCA_AC_BE; ac_index < SME_QOS_EDCA_AC_MAX; ac_index++)
   {
      pEntry = sme_QosFindInFlowList(search_key1);
      if(pEntry)
      {
         flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
         if(flow_info->ac_type == ac)
         {
            sme_QosCb.ac_info[ac].hoRenewal = flow_info->hoRenewal;
            break;
         }
      }
   }
   
   //notify HDD the success for the requested flow 
   //notify all the other flows running on the AC that QoS got modified
   if(!HAL_STATUS_SUCCESS(sme_QosFindAllInFlowList(pMac, search_key, sme_QosAddTsSuccessFnp)))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessAddTsSuccessRsp: no match found for ac = %d\n", 
                search_key.key.ac_type);
      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   sme_QosCb.ac_info[ac].hoRenewal = VOS_FALSE;

   vos_mem_zero(&sme_QosCb.ac_info[ac].requested_QoSInfo[tspec_pending - 1], 
                sizeof(sme_QosWmmTspecInfo));

         //event: EVENT_WLAN_QOS
#ifdef FEATURE_WLAN_DIAG_SUPPORT          
      qos.eventId = SME_QOS_DIAG_ADDTS_RSP;
      qos.reasonCode = SME_QOS_DIAG_ADDTS_ADMISSION_ACCEPTED;
      WLAN_VOS_DIAG_EVENT_REPORT(&qos, EVENT_WLAN_QOS);
      WLAN_VOS_DIAG_LOG_ALLOC(log_ptr, vos_log_qos_tspec_pkt_type, LOG_WLAN_QOS_TSPEC_C);
      if(log_ptr)
      {
         log_ptr->delay_bound = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].delay_bound;
         log_ptr->inactivity_interval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].inactivity_interval;
         log_ptr->max_burst_size = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].max_burst_size;
         log_ptr->max_service_interval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].max_service_interval;
         log_ptr->maximum_msdu_size = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].maximum_msdu_size;
         log_ptr->mean_data_rate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].mean_data_rate;
         log_ptr->medium_time = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].medium_time;
         log_ptr->min_data_rate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].min_data_rate;
         log_ptr->min_phy_rate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].min_phy_rate;
         log_ptr->min_service_interval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].min_service_interval;
         log_ptr->nominal_msdu_size = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].nominal_msdu_size;
         log_ptr->peak_data_rate = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].peak_data_rate;
         log_ptr->surplus_bw_allowance = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].surplus_bw_allowance;
         log_ptr->suspension_interval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].surplus_bw_allowance;
         log_ptr->suspension_interval = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].suspension_interval;
         log_ptr->svc_start_time = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].svc_start_time;
         log_ptr->tsinfo[0] = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].ts_info.direction << 5 |
            sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].ts_info.tid << 1;
         log_ptr->tsinfo[1] = sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].ts_info.up << 11 |
            sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_pending - 1].ts_info.psb << 10;
         log_ptr->tsinfo[2] = 0;
      }
      WLAN_VOS_DIAG_LOG_REPORT(log_ptr);
#endif //FEATURE_WLAN_DIAG_SUPPORT

   sme_QosCb.ac_info[ac].tspec_pending = 0;


   sme_QosStateTransition(SME_QOS_QOS_ON, ac);       

   (void)sme_QosProcessBufferedCmd();

   return eHAL_STATUS_SUCCESS;
   
}

/*--------------------------------------------------------------------------
  \brief sme_QosAggregateParams() - Utiltity function to increament the TSPEC 
  params per AC. Typical usage while using flow aggregation or deletion of flows
  
  \param pInput_Tspec_Info - Pointer to sme_QosWmmTspecInfo which contains the 
  WMM TSPEC related info with which pCurrent_Tspec_Info will be updated

  \param pCurrent_Tspec_Info - Pointer to sme_QosWmmTspecInfo which contains 
  current the WMM TSPEC related info


  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosAggregateParams(
   sme_QosWmmTspecInfo * pInput_Tspec_Info,
   sme_QosWmmTspecInfo * pCurrent_Tspec_Info,
   sme_QosWmmTspecInfo * pUpdated_Tspec_Info)
{
   sme_QosWmmTspecInfo TspecInfo;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosAggregateParams:Test: invoked\n");

   if(!pInput_Tspec_Info)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAggregateParams:input is NULL, nothing to aggregate\n");
      return eHAL_STATUS_FAILURE;
   }
   if(!pCurrent_Tspec_Info)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAggregateParams:Current is NULL, can't aggregate\n");
      return eHAL_STATUS_FAILURE;
   }
   vos_mem_zero(&TspecInfo, sizeof(sme_QosWmmTspecInfo));
   vos_mem_copy(&TspecInfo, pCurrent_Tspec_Info, 
                sizeof(sme_QosWmmTspecInfo));

   /*-------------------------------------------------------------------------
     APSD preference is only meaningful if service interval was set by app
   -------------------------------------------------------------------------*/
   if(pCurrent_Tspec_Info->min_service_interval && pInput_Tspec_Info->min_service_interval)
   {
      TspecInfo.min_service_interval = VOS_MIN(
         pCurrent_Tspec_Info->min_service_interval,
         pInput_Tspec_Info->min_service_interval);
   }
   else if(pInput_Tspec_Info->min_service_interval)
   {
      TspecInfo.min_service_interval = pInput_Tspec_Info->min_service_interval;
   }

   if(pCurrent_Tspec_Info->max_service_interval)
   {
      TspecInfo.max_service_interval = VOS_MIN(
         pCurrent_Tspec_Info->max_service_interval,
         pInput_Tspec_Info->max_service_interval);
   }
   else
   {
      TspecInfo.max_service_interval = pInput_Tspec_Info->max_service_interval;
   }

   /*-------------------------------------------------------------------------
     If directions don't match, it must necessarily be both uplink and
     downlink
   -------------------------------------------------------------------------*/
   if(pCurrent_Tspec_Info->ts_info.direction != 
      pInput_Tspec_Info->ts_info.direction)
   {
      TspecInfo.ts_info.direction = SME_QOS_WMM_TS_DIR_BOTH;
   }

   /*-------------------------------------------------------------------------
     Max MSDU size : these sizes are `maxed'
   -------------------------------------------------------------------------*/
   TspecInfo.maximum_msdu_size = VOS_MAX(pCurrent_Tspec_Info->maximum_msdu_size,
                                         pInput_Tspec_Info->maximum_msdu_size);


   /*-------------------------------------------------------------------------
     Inactivity interval : these sizes are `maxed'
   -------------------------------------------------------------------------*/
   TspecInfo.inactivity_interval = VOS_MAX(pCurrent_Tspec_Info->inactivity_interval,
                                         pInput_Tspec_Info->inactivity_interval);


   /*-------------------------------------------------------------------------
     Delay bounds: min of all values
     Check on 0: if 0, it means initial value since delay can never be 0!!
   -------------------------------------------------------------------------*/
   if(pCurrent_Tspec_Info->delay_bound)
   {
      TspecInfo.delay_bound = VOS_MIN(pCurrent_Tspec_Info->delay_bound,
                                      pInput_Tspec_Info->delay_bound);
   }
   else
   {
      TspecInfo.delay_bound = pInput_Tspec_Info->delay_bound;
   }

   TspecInfo.max_burst_size = VOS_MAX(pCurrent_Tspec_Info->max_burst_size,
                                      pInput_Tspec_Info->max_burst_size);


   /*-------------------------------------------------------------------------
     Nominal MSDU size also has a fixed bit that needs to be `handled' before
     aggregation

     This can be handled only if previous size is the same as new or both have
     the fixed bit set

     These sizes are not added: but `maxed'
   -------------------------------------------------------------------------*/
   TspecInfo.nominal_msdu_size = VOS_MAX(
      pCurrent_Tspec_Info->nominal_msdu_size & ~SME_QOS_16BIT_MSB,
      pInput_Tspec_Info->nominal_msdu_size & ~SME_QOS_16BIT_MSB);


   if( ((pCurrent_Tspec_Info->nominal_msdu_size == 0) ||
        (pCurrent_Tspec_Info->nominal_msdu_size & SME_QOS_16BIT_MSB)) &&
       ((pInput_Tspec_Info->nominal_msdu_size == 0) ||
        (pInput_Tspec_Info->nominal_msdu_size & SME_QOS_16BIT_MSB)))
   {
     TspecInfo.nominal_msdu_size |= SME_QOS_16BIT_MSB;
   }


   /*-------------------------------------------------------------------------
     Data rates: 
     Add up the rates for aggregation
   -------------------------------------------------------------------------*/

   SME_QOS_BOUNDED_U32_ADD_Y_TO_X( TspecInfo.peak_data_rate,
                                   pInput_Tspec_Info->peak_data_rate );

   SME_QOS_BOUNDED_U32_ADD_Y_TO_X( TspecInfo.min_data_rate,
                                   pInput_Tspec_Info->min_data_rate );

   /* mean data rate = peak data rate: aggregate to be flexible on apps  */
   SME_QOS_BOUNDED_U32_ADD_Y_TO_X( TspecInfo.mean_data_rate,
                                   pInput_Tspec_Info->mean_data_rate );


   /*-------------------------------------------------------------------------
     Suspension interval : this is set to the inactivity interval since per
     spec it is less than or equal to inactivity interval

     This is not provided by app since we currently don't support the HCCA
     mode of operation

     Currently set it to 0 to avoid confusion: Cisco CCX needs ~0; spec 
     requires inactivity interval to be > suspension interval: this could
     be tricky!
   -------------------------------------------------------------------------*/
   TspecInfo.suspension_interval = 0;

   /*-------------------------------------------------------------------------
     Remaining parameters do not come from app as they are very WLAN
     air interface specific

     Set meaningful values here
   -------------------------------------------------------------------------*/
   TspecInfo.medium_time = 0;               /* per WMM spec                 */

   TspecInfo.min_phy_rate = SME_QOS_MIN_PHY_RATE;

   TspecInfo.svc_start_time = 0;           /* arbitrary                  */

   TspecInfo.surplus_bw_allowance += pInput_Tspec_Info->surplus_bw_allowance;
   if(TspecInfo.surplus_bw_allowance > SME_QOS_SURPLUS_BW_ALLOWANCE)
   {
   TspecInfo.surplus_bw_allowance = SME_QOS_SURPLUS_BW_ALLOWANCE;
   }
   

   if(pUpdated_Tspec_Info)
   {
      vos_mem_copy(pUpdated_Tspec_Info, &TspecInfo, 
                   sizeof(sme_QosWmmTspecInfo));

   }
   else
   {
      vos_mem_copy(pCurrent_Tspec_Info, &TspecInfo, 
                   sizeof(sme_QosWmmTspecInfo));

   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosUpdateParams() - Utiltity function to update the TSPEC 
  params per AC. Typical usage while deleting flows on AC which is running
  multiple flows
  
  \param ac - Enumeration of the various EDCA Access Categories.
  \param tspec_mask - on which tspec per AC, the update is requested
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosUpdateParams(sme_QosEdcaAcType ac, v_U8_t tspec_mask,
                               sme_QosWmmTspecInfo * pTspec_Info)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosWmmTspecInfo Tspec_Info;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosUpdateParams:Test: invoked\n");

   if(!pTspec_Info)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosUpdateParams:output is NULL, can't aggregate\n");
      return eHAL_STATUS_FAILURE;
   }

   vos_mem_zero(&Tspec_Info, sizeof(sme_QosWmmTspecInfo));

   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosUpdateParams: Flow List empty, nothing to update\n");
      return eHAL_STATUS_FAILURE;
   }
   else
   {
      //init the TS info field
      Tspec_Info.ts_info.up = 
         sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.up;
      Tspec_Info.ts_info.psb = 
         sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.psb;
      Tspec_Info.ts_info.tid = 
         sme_QosCb.ac_info[ac].curr_QoSInfo[tspec_mask - 1].ts_info.tid;

   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if((ac == flow_info->ac_type) && (tspec_mask == flow_info->tspec_mask))
      {
         if((SME_QOS_REASON_RELEASE == flow_info->reason ) ||
            (SME_QOS_REASON_MODIFY == flow_info->reason))
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosUpdateParams:no need to consider this entry for \
                      updatation/aggregation as it is marked for release/modify\n");
         }
         else if(!HAL_STATUS_SUCCESS(sme_QosAggregateParams(&flow_info->QoSInfo, 
                                                            &Tspec_Info,
                                                            NULL)))
         {
            //err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosUpdateParams:sme_QosAggregateParams() failed\n");
         }
      }
      pEntry = pNextEntry;
   }

   vos_mem_copy(pTspec_Info, &Tspec_Info, 
                sizeof(sme_QosWmmTspecInfo));

   return eHAL_STATUS_SUCCESS;

}

/*--------------------------------------------------------------------------
  \brief sme_QosAcToUp() - Utiltity function to map an AC to UP

  Note: there is a quantization loss here because 4 ACs are mapped to 8 UPs
  Mapping is done for consistency

  \param ac - Enumeration of the various EDCA Access Categories.

  \return an User Priority
  
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosWmmUpType sme_QosAcToUp(sme_QosEdcaAcType ac)
{
   sme_QosWmmUpType up = SME_QOS_WMM_UP_MAX;
   if(ac >= 0 && ac < SME_QOS_EDCA_AC_MAX)
   {
      up = sme_QosACtoUPMap[ac];
   }
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
             "sme_QosAcToUp:Test: ac = %d up = %d returned\n", ac, up);

   return up;
}

/*--------------------------------------------------------------------------
  \brief sme_QosUpToAc() - Utiltity function to map an UP to AC

  \param up - Enumeration of the various User priorities (UP).

  \return an Access Category
  
  \sa
  
  --------------------------------------------------------------------------*/
sme_QosEdcaAcType sme_QosUpToAc(sme_QosWmmUpType up)
{
   sme_QosEdcaAcType ac = SME_QOS_EDCA_AC_MAX;

   if(up >= 0 && up < SME_QOS_WMM_UP_MAX)
   {
      ac = sme_QosUPtoACMap[up];
   }
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_MED, 
             "sme_QosUpToAc:Test: up = %d ac = %d returned\n", up, ac);

   return ac;
}

/*--------------------------------------------------------------------------
  \brief sme_QosStateTransition() - The state transition function per AC. We
  save the previous state also.

  \param new_state - The state FSM is moving to.
  \param ac - Enumeration of the various EDCA Access Categories.
  
  \return None
  
  \sa
  
  --------------------------------------------------------------------------*/
void sme_QosStateTransition(sme_QosStates new_state, sme_QosEdcaAcType ac)
{
   sme_QosCb.ac_info[ac].prev_state = sme_QosCb.ac_info[ac].curr_state;
   sme_QosCb.ac_info[ac].curr_state = new_state;
   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "QoS: new state=%d, old state=%d, for AC=%d\n", 
             sme_QosCb.ac_info[ac].curr_state, sme_QosCb.ac_info[ac].prev_state,
             ac );

}

/*--------------------------------------------------------------------------
  \brief sme_QosFindInFlowList() - Utility function to find an flow entry from
  the flow_list.

  \param search_key -  We can either use the flowID or the ac type to find the 
  entry in the flow list.
  A bitmap in sme_QosSearchInfo tells which key to use. Starting from LSB,
  bit 0 - Flow ID
  bit 1 - AC type

  \return the pointer to the entry in the link list
  
  \sa
  
  --------------------------------------------------------------------------*/
tListElem *sme_QosFindInFlowList(sme_QosSearchInfo search_key)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosFindInFlowList: Flow List empty, can't search\n");
      return NULL;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if(search_key.index & SME_QOS_SEARCH_KEY_INDEX_1)
      {
         if(search_key.key.QosFlowID == flow_info->QosFlowID)
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosFindInFlowList: match found on flowID, ending search\n");

            break;
         }
      }
      else if(search_key.index & SME_QOS_SEARCH_KEY_INDEX_2)
      {
         if(search_key.key.ac_type == flow_info->ac_type)
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosFindInFlowList: match found on ac, ending search\n");

            break;
         }
      }
      else if(search_key.index & SME_QOS_SEARCH_KEY_INDEX_3)
      {
         if(search_key.key.reason == flow_info->reason)
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosFindInFlowList: match found on ac, ending search\n");

            break;
         }
      }

      pEntry = pNextEntry;
   }
   return pEntry;
}

/*--------------------------------------------------------------------------
  \brief sme_QosFindAllInFlowList() - Utility function to find an flow entry 
  from the flow_list & act on it.

  \param search_key -  We can either use the flowID or the ac type to find the 
  entry in the flow list.
  A bitmap in sme_QosSearchInfo tells which key to use. Starting from LSB,
  bit 0 - Flow ID
  bit 1 - AC type
  \param fnp - function pointer specifying the action type for the entry found

  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosFindAllInFlowList(tpAniSirGlobal pMac,
                                    sme_QosSearchInfo search_key, 
                                    sme_QosProcessSearchEntry fnp)

{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   eHalStatus status = eHAL_STATUS_FAILURE;
   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosFindAllInFlowList: Flow List empty, can't search\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if(search_key.index & SME_QOS_SEARCH_KEY_INDEX_1)
      {
         if(search_key.key.QosFlowID == flow_info->QosFlowID)
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosFindAllInFlowList: match found on flowID, ending search\n");

            status = fnp(pMac, pEntry);
            if(eHAL_STATUS_FAILURE == status)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosFindAllInFlowList: Failed to process entry\n");
               break;
            }
         }
      }
      else if(search_key.index & SME_QOS_SEARCH_KEY_INDEX_2)
      {
         if(search_key.key.ac_type == flow_info->ac_type)
         {
            //msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosFindAllInFlowList: match found on ac, ending search\n");

            flow_info->hoRenewal = sme_QosCb.ac_info[flow_info->ac_type].hoRenewal;
            status = fnp(pMac, pEntry);
            if(eHAL_STATUS_FAILURE == status)
            {
               VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                         "sme_QosFindAllInFlowList: Failed to process entry\n");
               break;
            }
         }
      }
      pEntry = pNextEntry;
   }
   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosIsACM() - Utility function to check if a perticular AC
  mandates Admission Control.

  \param ac - Enumeration of the various EDCA Access Categories.
  
  \return VOS_TRUE if the AC mandates Admission Control
  
  \sa
  
  --------------------------------------------------------------------------*/
v_BOOL_t sme_QosIsACM(tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc, 
                      sme_QosEdcaAcType ac, tDot11fBeaconIEs *pIes)
{
   v_BOOL_t ret_val = VOS_FALSE;
   tDot11fBeaconIEs *pIesLocal = pIes;
   if(!pSirBssDesc)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosIsACM:pSirBssDesc is NULL\n");
      return VOS_FALSE;
   }

   if((NULL == pIesLocal) && !HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc, &pIesLocal)))
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosIsACM:csrGetParsedBssDescriptionIEs() failed\n");
      if(pIesLocal)
      {
         vos_mem_free(pIesLocal);
      }

      return VOS_FALSE;
   }

   switch(ac)
   {
      case SME_QOS_EDCA_AC_BE:
         if(pIesLocal->WMMParams.acbe_acm) ret_val = VOS_TRUE;
         break;
      case SME_QOS_EDCA_AC_BK:
         if(pIesLocal->WMMParams.acbk_acm) ret_val = VOS_TRUE;
         break;
      case SME_QOS_EDCA_AC_VI:
         if(pIesLocal->WMMParams.acvi_acm) ret_val = VOS_TRUE;
         break;
      case SME_QOS_EDCA_AC_VO:
         if(pIesLocal->WMMParams.acvo_acm) ret_val = VOS_TRUE;
         break;
      default:
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosIsACM:unknown AC = %d\n", ac);
         //Assert
         VOS_ASSERT(0);
         break;
   }

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosIsACM:Test: ACM = %d for AC = %d\n", ret_val, ac );
   if((NULL == pIes) && pIesLocal)
   {
      vos_mem_free(pIesLocal);
   }

   return ret_val;
}

/*--------------------------------------------------------------------------
  \brief sme_QosBufferExistingFlows() - Utility function to buffer the existing
  flows in flow_list, so that we can renew them after handoff is done.

                
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosBufferExistingFlows(tpAniSirGlobal pMac)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosCmdInfo  cmd;
   sme_QosWmmTspecInfo  *pInQoSInfo;

   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosBufferExistingFlows: Flow List empty, nothing to buffer\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if((SME_QOS_REASON_REQ_SUCCESS == flow_info->reason )||
         (SME_QOS_REASON_SETUP_REQ_APSD_PENDING == flow_info->reason )||
         (SME_QOS_REASON_SETUP == flow_info->reason ))
      {
         cmd.command = SME_QOS_SETUP_REQ;
         cmd.u.setupCmdInfo.HDDcontext = flow_info->HDDcontext;
         //copying over the QoS info structure
         pInQoSInfo = (sme_QosWmmTspecInfo*)vos_mem_malloc(sizeof(sme_QosWmmTspecInfo));
         vos_mem_zero(pInQoSInfo, sizeof(sme_QosWmmTspecInfo));
         vos_mem_copy(pInQoSInfo, &flow_info->QoSInfo, sizeof(sme_QosWmmTspecInfo));

         cmd.u.setupCmdInfo.pQoSInfo = pInQoSInfo;
         cmd.u.setupCmdInfo.QoSCallback = flow_info->QoSCallback;
         cmd.pMac = pMac;
         cmd.u.setupCmdInfo.UPType = SME_QOS_WMM_UP_MAX;//shouldn't be needed
         cmd.u.setupCmdInfo.QosFlowID = flow_info->QosFlowID;
         if(SME_QOS_REASON_SETUP == flow_info->reason )
         {
            cmd.u.setupCmdInfo.hoRenewal = VOS_FALSE;
         }
         else
         {
            cmd.u.setupCmdInfo.hoRenewal = VOS_TRUE;//TODO: might need this for modify
         }
         if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, VOS_TRUE)))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosBufferExistingFlows: couldn't buffer the setup request \
                      in handoff state\n" );
         }
         else
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosBufferExistingFlows:request buffered for setup\n");
         }

         
      }
      else if(SME_QOS_REASON_RELEASE == flow_info->reason ) 
      {
         cmd.command = SME_QOS_RELEASE_REQ;
         cmd.pMac = pMac;
         cmd.u.releaseCmdInfo.QosFlowID = flow_info->QosFlowID;

         if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, VOS_TRUE)))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosBufferExistingFlows: couldn't buffer the release request \
                      in handoff state\n" );
         }
         else
         {

            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosBufferExistingFlows:request buffered for release\n");
         }

      }
      else if(SME_QOS_REASON_MODIFY_PENDING == flow_info->reason)
      {
         cmd.command = SME_QOS_MODIFY_REQ;
         cmd.pMac = pMac;
         cmd.u.modifyCmdInfo.QosFlowID = flow_info->QosFlowID;
         //copying over the QoS info structure
         pInQoSInfo = (sme_QosWmmTspecInfo*)vos_mem_malloc(sizeof(sme_QosWmmTspecInfo));
         vos_mem_zero(pInQoSInfo, sizeof(sme_QosWmmTspecInfo));
         vos_mem_copy(pInQoSInfo, &flow_info->QoSInfo, sizeof(sme_QosWmmTspecInfo));

         cmd.u.modifyCmdInfo.pQoSInfo = pInQoSInfo;

         if(!HAL_STATUS_SUCCESS(sme_QosBufferCmd(&cmd, VOS_TRUE)))
         {
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosBufferExistingFlows: couldn't buffer the modify request \
                      in handoff state\n" );
         }
         else
         {

            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                      "sme_QosBufferExistingFlows:request buffered for modify\n");
         }

      }
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosBufferExistingFlows:deleting entry from Flow List\n");
      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );

      pEntry = pNextEntry;
   }
   //clear the UAPSD request list & no need to send a stopUapsd to PMC
   if(sme_QosCb.apsd_req_counter)
   {
      vos_mem_zero(sme_QosCb.uapsdRequest, 
                   sizeof(sme_QosPmcStartUAPSDRequest)* SME_QOS_MAX_APSD_REQ_PENDING);
      sme_QosCb.apsd_req_counter= 0;
      sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosDeleteExistingFlows() - Utility function to Delete the existing
  flows in flow_list, if we lost connectivity.

                
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosDeleteExistingFlows(tpAniSirGlobal pMac)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;

   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_TRUE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDeleteExistingFlows: Flow List empty, nothing to buffer\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_TRUE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if((SME_QOS_REASON_REQ_SUCCESS == flow_info->reason )||
         (SME_QOS_REASON_SETUP == flow_info->reason )||
         (SME_QOS_REASON_RELEASE == flow_info->reason )||
         (SME_QOS_REASON_MODIFY == flow_info->reason )||
         (SME_QOS_REASON_SETUP_REQ_APSD_PENDING == flow_info->reason )||
         (SME_QOS_REASON_MODIFY_REQ_APSD_PENDING == flow_info->reason ))

      {
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                NULL,
                                SME_QOS_STATUS_RELEASE_QOS_LOST_IND,
                                flow_info->QosFlowID);

        
      }
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDeleteExistingFlows:deleting entry from Flow List\n");
      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );

      pEntry = pNextEntry;
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosBufferCmd() - Utility function to buffer a request (setup/modify/
  release) from client while processing another one on the same AC.

  \param pcmd - a pointer to the cmd structure to be saved inside the buffered
                cmd link list
                
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosBufferCmd(sme_QosCmdInfo *pcmd, v_BOOL_t insert_head)
{
   sme_QosCmdInfoEntry * pentry = NULL;
   sme_QosWmmTspecInfo  *pTempQoSInfo = NULL;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosBufferCmd: invoked\n");

   pentry= (sme_QosCmdInfoEntry *)
   vos_mem_malloc(sizeof(sme_QosCmdInfoEntry));
   if (!pentry)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosBufferCmd: couldn't allocate memory for the new\
                entry in the buffered CMD list\n");
      return eHAL_STATUS_FAILURE;
   }

   pentry->cmdInfo.command = pcmd->command;
   pentry->cmdInfo.pMac = pcmd->pMac;

   switch(pcmd->command)
   {
   case SME_QOS_SETUP_REQ:
      pentry->cmdInfo.u.setupCmdInfo.HDDcontext = pcmd->u.setupCmdInfo.HDDcontext;
      pentry->cmdInfo.u.setupCmdInfo.QosFlowID = pcmd->u.setupCmdInfo.QosFlowID;
      pentry->cmdInfo.u.setupCmdInfo.QoSCallback = pcmd->u.setupCmdInfo.QoSCallback;
      pentry->cmdInfo.u.setupCmdInfo.UPType = pcmd->u.setupCmdInfo.UPType;
      pentry->cmdInfo.u.setupCmdInfo.HDDcontext = pcmd->u.setupCmdInfo.HDDcontext;
      pentry->cmdInfo.u.setupCmdInfo.hoRenewal = pcmd->u.setupCmdInfo.hoRenewal;
      //copy over the QoS params structure in case the caller destroys it
      pTempQoSInfo = (sme_QosWmmTspecInfo *)
      vos_mem_malloc(sizeof(sme_QosWmmTspecInfo));
      if (!pTempQoSInfo)
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosBufferCmd: couldn't allocate memory for the QoS params\n");

         return eHAL_STATUS_FAILURE;
      }
      pentry->cmdInfo.u.setupCmdInfo.pQoSInfo = pTempQoSInfo;
      vos_mem_copy(pentry->cmdInfo.u.setupCmdInfo.pQoSInfo, 
                   pcmd->u.setupCmdInfo.pQoSInfo, 
                   sizeof(sme_QosWmmTspecInfo));

      break;
   case SME_QOS_RELEASE_REQ:
      pentry->cmdInfo.u.releaseCmdInfo.QosFlowID = pcmd->u.releaseCmdInfo.QosFlowID;
      break;
   case SME_QOS_MODIFY_REQ:
      pentry->cmdInfo.u.modifyCmdInfo.QosFlowID = pcmd->u.modifyCmdInfo.QosFlowID;
      //copy over the QoS params structure in case the caller destroys it
      pTempQoSInfo = (sme_QosWmmTspecInfo *)
      vos_mem_malloc(sizeof(sme_QosWmmTspecInfo));
      if (!pTempQoSInfo)
      {
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosBufferCmd: couldn't allocate memory for the QoS params\n");

         return eHAL_STATUS_FAILURE;
      }
      pentry->cmdInfo.u.modifyCmdInfo.pQoSInfo = pTempQoSInfo;
      vos_mem_copy(pentry->cmdInfo.u.modifyCmdInfo.pQoSInfo, 
                   pcmd->u.modifyCmdInfo.pQoSInfo, 
                   sizeof(sme_QosWmmTspecInfo));

      break;
   default:
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosIsACM:unknown cmd = %d\n", pcmd->command);
      //ASSERT
      VOS_ASSERT(0);
      break;
   }

   if(insert_head) 
   {
      csrLLInsertHead(&sme_QosCb.buffered_cmd_list, &pentry->link, VOS_TRUE);
   }
   else
   {
   csrLLInsertTail(&sme_QosCb.buffered_cmd_list, &pentry->link, VOS_TRUE);
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessBufferedCmd() - Utility function to process a buffered 
  request (setup/modify/release) initially came from the client.

  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessBufferedCmd(void)
{
   sme_QosCmdInfoEntry *pcmd = NULL;
   tListElem *pEntry= NULL;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;

   VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
             "sme_QosProcessBufferedCmd: invoked\n");

   if(!csrLLIsListEmpty( &sme_QosCb.buffered_cmd_list, VOS_FALSE ))
   {
      pEntry = csrLLRemoveHead( &sme_QosCb.buffered_cmd_list, VOS_TRUE );
      if(!pEntry)
      {
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessBufferedCmd: no more buffered cmd\n");
         sme_QosCb.readyForPowerSave = VOS_TRUE;

         return eHAL_STATUS_FAILURE;
      }
      pcmd = GET_BASE_ADDR( pEntry, sme_QosCmdInfoEntry, link );
      switch(pcmd->cmdInfo.command)
      {
      case SME_QOS_SETUP_REQ:
         hdd_status = sme_QosInternalSetupReq(pcmd->cmdInfo.pMac, 
                                              pcmd->cmdInfo.u.setupCmdInfo.pQoSInfo,
                                              pcmd->cmdInfo.u.setupCmdInfo.QoSCallback, 
                                              pcmd->cmdInfo.u.setupCmdInfo.HDDcontext, 
                                              pcmd->cmdInfo.u.setupCmdInfo.UPType, 
                                              &pcmd->cmdInfo.u.setupCmdInfo.QosFlowID, 
                                              VOS_TRUE,
                                              pcmd->cmdInfo.u.setupCmdInfo.hoRenewal);
         if(SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != hdd_status)
         {
            sme_QosCb.readyForPowerSave = VOS_TRUE;
         }
         if(SME_QOS_STATUS_SETUP_FAILURE_RSP == hdd_status)
         {
            //Err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessBufferedCmd: sme_QosInternalSetupReq failed\n");

            return eHAL_STATUS_FAILURE;
         }
         break;
      case SME_QOS_RELEASE_REQ:
         hdd_status = sme_QosInternalReleaseReq(pcmd->cmdInfo.pMac, 
                                                pcmd->cmdInfo.u.releaseCmdInfo.QosFlowID,
                                                VOS_TRUE);

         if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != hdd_status)&&
            (SME_QOS_STATUS_RELEASE_REQ_PENDING_RSP != hdd_status))
         {
            sme_QosCb.readyForPowerSave = VOS_TRUE;
         }
         if(SME_QOS_STATUS_RELEASE_FAILURE_RSP == hdd_status)
         {
            //Err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessBufferedCmd: sme_QosInternalReleaseReq failed\n");

            return eHAL_STATUS_FAILURE;
         }
         break;
      case SME_QOS_MODIFY_REQ:
         hdd_status = sme_QosInternalModifyReq(pcmd->cmdInfo.pMac, 
                                               pcmd->cmdInfo.u.modifyCmdInfo.pQoSInfo,
                                               pcmd->cmdInfo.u.modifyCmdInfo.QosFlowID,
                                               VOS_TRUE);

         if((SME_QOS_STATUS_SETUP_REQ_PENDING_RSP != hdd_status)&&
            (SME_QOS_STATUS_MODIFY_SETUP_PENDING_RSP != hdd_status))
         {
            sme_QosCb.readyForPowerSave = VOS_TRUE;
         }
         if(SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP == hdd_status)
         {
            //Err msg
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                      "sme_QosProcessBufferedCmd: sme_QosInternalModifyReq failed\n");

            return eHAL_STATUS_FAILURE;
         }
         break;
      default:
         //err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessBufferedCmd: unknown cmd = %d\n", pcmd->cmdInfo.command);
         //ASSERT
         VOS_ASSERT(0);
         break;
      }
   }
   else
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosProcessBufferedCmd: cmd buffer empty\n");

      sme_QosCb.readyForPowerSave = VOS_TRUE;
   }
   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosDeleteBufferedRequests() - Utility function to Delete the buffered
  requests in the buffered_cmd_list, if we lost connectivity.

                
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosDeleteBufferedRequests(tpAniSirGlobal pMac)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;

   pEntry = csrLLPeekHead( &sme_QosCb.buffered_cmd_list, VOS_TRUE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDeleteBufferedRequests: Buffered List empty, nothing to delete\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.buffered_cmd_list, pEntry, VOS_TRUE );
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDeleteBufferedRequests:deleting entry from buffered List\n");
      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.buffered_cmd_list, pEntry, VOS_TRUE );

      pEntry = pNextEntry;
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosSaveAssocInfo() - Utility function to save the assoc info in the
  CB like BSS descritor of the AP, the profile that HDD sent down with the 
  connect request, while CSR notifies for assoc/reassoc success.

  \param pAssoc_info - pointer to the assoc structure to store the BSS descritor 
                       of the AP, the profile that HDD sent down with the 
                       connect request
                       
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosSaveAssocInfo(sme_QosAssocInfo *pAssoc_info)
{
   tSirBssDescription    *pBssDesc = NULL;
   v_U32_t                bssLen = 0;

   //clean up the assoc info if already set
   if(sme_QosCb.assoc_Info.pBssDesc)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pBssDesc);
      sme_QosCb.assoc_Info.pBssDesc = NULL;
   }

   bssLen = pAssoc_info->pBssDesc->length + 
      sizeof(pAssoc_info->pBssDesc->length);

   //save the bss Descriptor
   pBssDesc = (tSirBssDescription *)vos_mem_malloc(bssLen);
   vos_mem_zero(pBssDesc, bssLen);
   if (!pBssDesc)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupReq: couldn't allocate memory for the \
                bss Descriptor\n");
      return eHAL_STATUS_FAILURE;
   }
   vos_mem_copy(pBssDesc, pAssoc_info->pBssDesc, bssLen);
   sme_QosCb.assoc_Info.pBssDesc = pBssDesc;

   //save the apsd info from assoc
   if(pAssoc_info)
   {
      if(pAssoc_info->pProfile)
   {
         sme_QosCb.apsd_mask |= pAssoc_info->pProfile->uapsd_mask;
      }
   }

   return eHAL_STATUS_SUCCESS;
}

#if defined(SME_QOS_NOT_SUPPORTED)
/*--------------------------------------------------------------------------
  \brief sme_QosSetBSSID() - Utility function to set the BSSID to be passed as 
  part of the profile while requesting for the reassoc

  \param pMac - Pointer to the global MAC parameter structure.
                       
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosSetBSSID(tpAniSirGlobal pMac)
{
   //clean up the BSSID requested with the profile as part of prev. reassoc
   // req for APSD
   if(sme_QosCb.assoc_Info.pProfile->BSSIDs.bssid)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pProfile->BSSIDs.bssid);
   }
   
   sme_QosCb.assoc_Info.pProfile->BSSIDs.bssid = 
      vos_mem_malloc( sizeof( tCsrBssid ));
   if(sme_QosCb.assoc_Info.pProfile->BSSIDs.bssid)
   {
      sme_QosCb.assoc_Info.pProfile->BSSIDs.numOfBSSIDs = 1;
   }
   else
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetBSSID: couldn't allocate memory for the bssid\n");

      return eHAL_STATUS_FAILURE;
   }
   if(!sme_QosCb.assoc_Info.pBssDesc->bssId)
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetBSSID: the BSS descritor doesn't have a BSSID\n");

      return eHAL_STATUS_FAILURE;

   }

   vos_mem_copy( sme_QosCb.assoc_Info.pProfile->BSSIDs.bssid, 
                 &sme_QosCb.assoc_Info.pBssDesc->bssId[ 0 ], 
                 sizeof(tCsrBssid) );

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosSetSSID() - Utility function to set the SSID from the BSS 
  descritor  saved in CB to be passed as part of the profile while requesting 
  for the reassoc

  \param pMac - Pointer to the global MAC parameter structure.
                       
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosSetSSID(tpAniSirGlobal pMac, tSirBssDescription *pSirBssDesc)
{
   tDot11fBeaconIEs *pIes = NULL;

   if(!HAL_STATUS_SUCCESS(csrGetParsedBssDescriptionIEs(pMac, pSirBssDesc, &pIes)))
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetSSID:csrGetParsedBssDescriptionIEs() failed\n");
      return eHAL_STATUS_FAILURE;
   }

   //clean up the SSID requested with the profile as part of prev. reassoc
   // req for APSD
   if(sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList)
   {
      vos_mem_free(sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList);
   }

   sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList = 
      vos_mem_malloc( sizeof( tCsrSSIDInfo ));
   if(sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList)
   {
      sme_QosCb.assoc_Info.pProfile->SSIDs.numOfSSIDs = 1;
   }
   else
   {
      //err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetSSID: couldn't allocate memory for the ssid\n");

      return eHAL_STATUS_FAILURE;
   }

   if(!pIes->SSID.present)
   {
      //err 
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetSSID: the BSS descritor doesn't have the SSID\n");
      return eHAL_STATUS_FAILURE;
   }
   sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList[0].length = 
      pIes->SSID.num_ssid;
   vos_mem_copy( sme_QosCb.assoc_Info.pProfile->SSIDs.SSIDList[0].ssId, 
                 pIes->SSID.ssid, 
                 pIes->SSID.num_ssid);


   return eHAL_STATUS_SUCCESS;
}

#endif
/*--------------------------------------------------------------------------
  \brief sme_QosSetupFnp() - Utility function (pointer) to notify other entries 
  in FLOW list on the same AC that qos pamas got modified

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosSetupFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_MODIFIED_IND;
   sme_QosEdcaAcType ac;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupFnp: Entry is NULL\n");

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }


   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }

   ac = flow_info->ac_type;

   if(SME_QOS_REASON_REQ_SUCCESS == flow_info->reason)
   {
      //notify HDD, only the other Flows running on the AC 
      flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                             &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                             hdd_status,
                             flow_info->QosFlowID);
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosSetupFnp: Entry with flowID = %d getting notified\n", 
                flow_info->QosFlowID);

   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosModificationNotifyFnp() - Utility function (pointer) to notify 
  other entries in FLOW list on the same AC that qos params got modified

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosModificationNotifyFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_MODIFIED_IND;
   sme_QosEdcaAcType ac;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupFnp: Entry is NULL\n");

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }


   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosSetupFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }

   ac = flow_info->ac_type;

   if(SME_QOS_REASON_REQ_SUCCESS == flow_info->reason)
   {
      //notify HDD, only the other Flows running on the AC 
      flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                             &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                             hdd_status,
                             flow_info->QosFlowID);
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosSetupFnp: Entry with flowID = %d getting notified\n", 
                flow_info->QosFlowID);

   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosModifyFnp() - Utility function (pointer) to delete the origianl 
  entry in FLOW list & add the modified one

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosModifyFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosModifyFnp: entry is NULL\n");
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosModifyFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }

   switch(flow_info->reason)
   {
   case SME_QOS_REASON_MODIFY_PENDING:
      //set the proper reason code for the new (with modified params) entry
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      break;
   case SME_QOS_REASON_MODIFY:
      //delete the original entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );
      break;
   default:
      break;
   }
   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosDelTsIndFnp() - Utility function (pointer) to find all Flows on 
  the perticular AC & delete them, also send HDD indication through the callback 
  it registered per request

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosDelTsIndFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosEdcaAcType ac;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDelTsIndFnp: Entry is NULL\n");

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }
   //delete the entry from Flow List
   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosDelTsIndFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }
   ac = flow_info->ac_type;

   flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                          &sme_QosCb.ac_info[ac].curr_QoSInfo[flow_info->tspec_mask - 1],
                          SME_QOS_STATUS_RELEASE_QOS_LOST_IND,
                          flow_info->QosFlowID);
   sme_QosCb.ac_info[ac].num_flows[flow_info->tspec_mask - 1]--;
   csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosReassocSuccessEvFnp() - Utility function (pointer) to notify HDD 
  the success for the requested flow & notify all the other flows running on the 
  same AC that QoS params got modified

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosReassocSuccessEvFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   v_BOOL_t delete_entry = VOS_FALSE;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   sme_QosEdcaAcType ac;
   eHalStatus pmc_status = eHAL_STATUS_FAILURE;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosReassocSuccessEvFnp: Entry is NULL\n");
      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   flow_info = GET_BASE_ADDR(pEntry, sme_QosFlowInfoEntry, link);
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosReassocSuccessEvFnp: couldn't access the QoS \
                params from the Flow List entry\n");
   
      return eHAL_STATUS_FAILURE;
   }

   ac = flow_info->ac_type;

   switch(flow_info->reason)
   {
   case SME_QOS_REASON_SETUP:
      hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
      delete_entry = VOS_FALSE;
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      //check for the case where we had to do reassoc to reset the apsd bit for
      //the ac - release or modify scenario
      if(sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0].ts_info.psb)
      {
         //notify PMC as App is looking for APSD. If we already requested just 
         // buffer the request & wait for PMC notification through callback.
         //if PMC doesn't return sucess right away means it is yet to put the
         //module in BMPS state & later to UAPSD state
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = 
            flow_info->hoRenewal;
         sme_QosCb.apsd_req_counter++;
#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
         //When trigger frmae is sent by FW, notify HDD independent off UAPSD
         flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
   
         if(!sme_QosCb.uapsdAlreadyRequested)
         {
            pmc_status = pmcStartUapsd(pMac, sme_QosPmcStartUapsdCallback, pMac);
         
            if(eHAL_STATUS_FAILURE == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_SET_FAILED;
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
               flow_info->hoRenewal = VOS_FALSE;//we need to notify this case
            }
            else if(eHAL_STATUS_PMC_PENDING == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
               sme_QosCb.uapsdAlreadyRequested = VOS_TRUE;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
            }
            else if(eHAL_STATUS_SUCCESS == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since it right away got success from PMC, remove the entry from 
               //APSD request list
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_DISABLED == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since power save is disabled sytem wide, remove the entry from 
               //APSD request list & don't expect any further response from PMC
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
               //overloading the existing indication for HDD. APSD isn't
               //pending from PMC, as powersave is disabled system wide
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
            }
   
         }
         else
         {
            hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
            flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
         }
      }
      break;
   case SME_QOS_REASON_RELEASE:
      sme_QosCb.ac_info[ac].num_flows[SME_QOS_TSPEC_INDEX_0]--;
   case SME_QOS_REASON_MODIFY:
      delete_entry = VOS_TRUE;
      break;

   case SME_QOS_REASON_MODIFY_PENDING:
      hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
      delete_entry = VOS_FALSE;
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      if(sme_QosCb.ac_info[ac].requested_QoSInfo[SME_QOS_TSPEC_INDEX_0].ts_info.psb)
      {
         //notify PMC as App is looking for APSD. If we already requested just 
         // buffer the request & wait for PMC notification through callback.
         //if PMC doesn't return sucess right away means it is yet to put the
         //module in BMPS state & later to UAPSD state
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
         sme_QosCb.apsd_req_counter++;
#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
         //When trigger frmae is sent by FW, notify HDD independent off UAPSD
         flow_info->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
#endif
   
         if(!sme_QosCb.uapsdAlreadyRequested)
         {
            pmc_status = pmcStartUapsd(pMac, sme_QosPmcStartUapsdCallback, pMac);
         
            if(eHAL_STATUS_FAILURE == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_SET_FAILED;
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_PENDING == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
#endif
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_PENDING;
               sme_QosCb.uapsdAlreadyRequested = VOS_TRUE;
            }
            else if(eHAL_STATUS_SUCCESS == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since it right away got success from PMC, remove the entry from 
               //APSD request list
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_DISABLED == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since power save is disabled sytem wide, remove the entry from 
               //APSD request list & don't expect any further response from PMC
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
               //overloading the existing indication for HDD. APSD isn't
               //pending from PMC, as powersave is disabled system wide
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
            }

   
         }
         else
         {
            hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
            flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
         }
      }
      break;
   case SME_QOS_REASON_REQ_SUCCESS:
      hdd_status = SME_QOS_STATUS_SETUP_MODIFIED_IND;
   default:
      delete_entry = VOS_FALSE;
      break;
   }

   if(!delete_entry)
   {
      if(!flow_info->hoRenewal)
      {
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                &sme_QosCb.ac_info[ac].curr_QoSInfo[SME_QOS_TSPEC_INDEX_0],
                                hdd_status,
                                flow_info->QosFlowID);
         sme_QosHandleCallback(eHAL_STATUS_SUCCESS);
      }
      else
      {
         flow_info->hoRenewal = VOS_FALSE;
      }
   }
   else
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosReassocSuccessEvFnp: Entry with flowID = %d getting deleted\n", 
                flow_info->QosFlowID);
      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );
   }
   
   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosAddTsFailureFnp() - Utility function (pointer), 
  if the Addts request was for for an flow setup request, delete the entry from 
  Flow list & notify HDD 
  if the Addts request was for downgrading of QoS params because of an flow 
  release requested on the AC, delete the entry from Flow list & notify HDD 
  if the Addts request was for change of QoS params because of an flow 
  modification requested on the AC, delete the new entry from Flow list & notify 
  HDD 


  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosAddTsFailureFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   v_BOOL_t inform_hdd = VOS_FALSE;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   sme_QosEdcaAcType ac;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosAddTsFailureFnp: Entry is NULL\n");

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }


   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosAddTsFailureFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }

   ac = flow_info->ac_type;

   switch(flow_info->reason)
   {
   case SME_QOS_REASON_SETUP:
      hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
      sme_QosCb.ac_info[ac].num_flows[sme_QosCb.ac_info[ac].tspec_pending - 1]--;
      inform_hdd = VOS_TRUE;
      break;
   case SME_QOS_REASON_RELEASE:
      hdd_status = SME_QOS_STATUS_RELEASE_FAILURE_RSP;
      sme_QosCb.ac_info[ac].num_flows[sme_QosCb.ac_info[ac].tspec_pending - 1]--;
      inform_hdd = VOS_TRUE;
      break;
   case SME_QOS_REASON_MODIFY_PENDING:
      hdd_status = SME_QOS_STATUS_MODIFY_SETUP_FAILURE_RSP;
      inform_hdd = VOS_TRUE;
      break;
   case SME_QOS_REASON_MODIFY:
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
   case SME_QOS_REASON_REQ_SUCCESS:
   default:
      inform_hdd = VOS_FALSE;
      break;
   }
   if(inform_hdd)
   {
      //notify HDD, only the requested Flow, other Flows running on the AC stay 
      // intact
      if(!flow_info->hoRenewal)
      {
      flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                             &sme_QosCb.ac_info[ac].curr_QoSInfo[sme_QosCb.ac_info[ac].tspec_pending - 1],
                             hdd_status,
                             flow_info->QosFlowID);
      }
      else
      {
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                &sme_QosCb.ac_info[ac].curr_QoSInfo[sme_QosCb.ac_info[ac].tspec_pending - 1],
                                SME_QOS_STATUS_RELEASE_QOS_LOST_IND,
                                flow_info->QosFlowID);
      }
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAddTsFailureFnp: Entry with flowID = %d getting deleted\n", 
                flow_info->QosFlowID);

      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );

   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosAddTsSuccessFnp() - Utility function (pointer), 
  if the Addts request was for for an flow setup request, notify HDD for success
  for the flow & notify all the other flows running on the same AC that QoS 
  params got modified
  if the Addts request was for downgrading of QoS params because of an flow 
  release requested on the AC, delete the entry from Flow list & notify HDD 
  if the Addts request was for change of QoS params because of an flow 
  modification requested on the AC, delete the old entry from Flow list & notify 
  HDD for success for the flow & notify all the other flows running on the same 
  AC that QoS params got modified

  \param pMac - Pointer to the global MAC parameter structure.
  \param pEntry - Pointer to an entry in the flow_list(i.e. tListElem structure)
  
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosAddTsSuccessFnp(tpAniSirGlobal pMac, tListElem *pEntry)
{
   sme_QosFlowInfoEntry *flow_info = NULL;
   v_BOOL_t inform_hdd = VOS_FALSE;
   v_BOOL_t delete_entry = VOS_FALSE;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   sme_QosEdcaAcType ac;
   eHalStatus pmc_status = eHAL_STATUS_FAILURE;

   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosAddTsSuccessFnp: Entry is NULL\n");

      //ASSERT
      VOS_ASSERT(0);
      return eHAL_STATUS_FAILURE;
   }

   flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
   if(!flow_info)
   {
      //Err msg
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosAddTsSuccessFnp: couldn't access the QoS \
                params from the Flow List entry\n");

      return eHAL_STATUS_FAILURE;
   }

   ac = flow_info->ac_type;

   if(flow_info->tspec_mask != sme_QosCb.ac_info[ac].tspec_pending)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAddTsSuccessFnp: No need to notify the HDD, the ADDTS \
                success is not for index = %d of the AC = %d\n",
                flow_info->tspec_mask, ac);

      return eHAL_STATUS_SUCCESS;
   }

   switch(flow_info->reason)
   {
   case SME_QOS_REASON_SETUP:
      hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      delete_entry = VOS_FALSE;
      inform_hdd = VOS_TRUE;
      //notify PMC if App is looking for APSD
      if(sme_QosCb.ac_info[ac].requested_QoSInfo[sme_QosCb.ac_info[ac].
         tspec_pending - 1].ts_info.psb)
      {
         //notify PMC as App is looking for APSD. If we already requested, just 
         // buffer the request & wait for PMC notification through callback.
         //if PMC doesn't return sucess right away means it is yet to put the
         //module in BMPS state & later to UAPSD state
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = 
            flow_info->hoRenewal;
         sme_QosCb.apsd_req_counter++;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
         //When trigger frmae is sent by FW, notify HDD independent off UAPSD
         flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif

         if(!sme_QosCb.uapsdAlreadyRequested)
         {
            pmc_status = pmcStartUapsd(pMac, sme_QosPmcStartUapsdCallback, pMac);

            if(eHAL_STATUS_FAILURE == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
               flow_info->hoRenewal = VOS_FALSE;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_PENDING == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
               sme_QosCb.uapsdAlreadyRequested = VOS_TRUE;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
            }
            else if(eHAL_STATUS_SUCCESS == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since it right away got success from PMC, remove the entry from 
               //APSD request list
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_DISABLED == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since power save is disabled sytem wide, remove the entry from 
               //APSD request list & don't expect any further response from PMC
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
               //overloading the existing indication for HDD. APSD isn't
               //pending from PMC, as powersave is disabled system wide
               hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
            }


         }
         else
         {
            hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
            flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
         }
      }
      break;
   case SME_QOS_REASON_RELEASE:
      sme_QosCb.ac_info[ac].num_flows[sme_QosCb.ac_info[ac].tspec_pending - 1]--;
      hdd_status = SME_QOS_STATUS_RELEASE_SUCCESS_RSP;
      inform_hdd = VOS_TRUE;
      delete_entry = VOS_TRUE;
      break;
   case SME_QOS_REASON_MODIFY:
      delete_entry = VOS_TRUE;
      inform_hdd = VOS_FALSE;
      break;

   case SME_QOS_REASON_MODIFY_PENDING:
      hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
      delete_entry = VOS_FALSE;
      flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      inform_hdd = VOS_TRUE;
      //notify PMC if App is looking for APSD
      if(sme_QosCb.ac_info[ac].requested_QoSInfo[sme_QosCb.ac_info[ac].
         tspec_pending - 1].ts_info.psb)
      {
         //notify PMC as App is looking for APSD. If we already requested just 
         // buffer the request & wait for PMC notification through callback.
         //if PMC doesn't return sucess right away means it is yet to put the
         //module in BMPS state & later to UAPSD state
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = pEntry;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = pMac;
         sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].hoRenewal = VOS_FALSE;
         sme_QosCb.apsd_req_counter++;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
         //When trigger frmae is sent by FW, notify HDD independent off UAPSD
         flow_info->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
#endif

         if(!sme_QosCb.uapsdAlreadyRequested)
         {
            pmc_status = pmcStartUapsd(pMac, sme_QosPmcStartUapsdCallback, pMac);

            if(eHAL_STATUS_FAILURE == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_PENDING;
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
#endif
            }
            else if(eHAL_STATUS_PMC_PENDING == pmc_status)
            {
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND_APSD_PENDING;
               sme_QosCb.uapsdAlreadyRequested = VOS_TRUE;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               flow_info->reason = SME_QOS_REASON_MODIFY_REQ_APSD_PENDING;
#endif
            }
            else if(eHAL_STATUS_SUCCESS == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since it right away got success from PMC, remove the entry from 
               //APSD request list
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
            }
            else if(eHAL_STATUS_PMC_DISABLED == pmc_status)
            {
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
               //since power save is disabled sytem wide, remove the entry from 
               //APSD request list & don't expect any further response from PMC
               sme_QosCb.apsd_req_counter--;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pEntry = NULL;
               sme_QosCb.uapsdRequest[sme_QosCb.apsd_req_counter].pMac = NULL;
#endif
               sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
               //overloading the existing indication for HDD. APSD isn't
               //pending from PMC, as powersave is disabled system wide
               hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
            }


         }
         else
         {
            hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING;
#ifndef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
            flow_info->reason = SME_QOS_REASON_SETUP_REQ_APSD_PENDING;
#endif
         }
      }
      break;
   case SME_QOS_REASON_REQ_SUCCESS:
      hdd_status = SME_QOS_STATUS_SETUP_MODIFIED_IND;
      inform_hdd = VOS_TRUE;
   default:
      delete_entry = VOS_FALSE;
      break;
   }

   if(inform_hdd)
   {
      if(!flow_info->hoRenewal)
      {
      
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                &sme_QosCb.ac_info[ac].curr_QoSInfo[sme_QosCb.ac_info[ac].tspec_pending - 1],
                                hdd_status,
                                flow_info->QosFlowID);
         //Call the callback at here
         sme_QosHandleCallback(eHAL_STATUS_SUCCESS);
      }
      else
      {
         flow_info->hoRenewal = VOS_FALSE;
      }
   }
   if(delete_entry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_INFO_HIGH, 
                "sme_QosAddTsSuccessFnp: Entry with flowID = %d getting deleted\n", 
                flow_info->QosFlowID);

      //delete the entry from Flow List
      csrLLRemoveEntry(&sme_QosCb.flow_list, pEntry, VOS_TRUE );
   }


   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosIsAddtsPending() - Utility function to check if we are waiting 
  for addts response on some AC
  
  \param pMac - Pointer to the global MAC parameter structure.
  \param ac - Enumeration of the various EDCA Access Categories.
  
  \return boolean
  TRUE - Addts response is pending on an AC
  
  \sa
  
  --------------------------------------------------------------------------*/
v_BOOL_t sme_QosIsRspPending(sme_QosEdcaAcType ac)
{
   sme_QosEdcaAcType acIndex;
   v_BOOL_t status = VOS_FALSE;

   for(acIndex = SME_QOS_EDCA_AC_BE; acIndex < SME_QOS_EDCA_AC_MAX; acIndex++) 
   {
      if(acIndex == ac)
      {
         continue;
      }
      if(sme_QosCb.ac_info[acIndex].tspec_pending)
      {
         status = VOS_TRUE;
         break;
      }
      if(sme_QosCb.ac_info[acIndex].reassoc_pending)
      {
         status = VOS_TRUE;
         break;
      }

   }

   return status;
}

/*--------------------------------------------------------------------------
  \brief sme_QosPmcFullPowerCallback() - Callback function registered with PMC 
  to notify SME-QoS when it puts the chip into full power
  
  \param callbackContext - The context passed to PMC during pmcRequestFullPower
  call.
  \param status - eHalStatus returned by PMC.
  
  \return None
  
  \sa
  
  --------------------------------------------------------------------------*/
void sme_QosPmcFullPowerCallback(void *callbackContext, eHalStatus status)
{
   if(HAL_STATUS_SUCCESS(status))
   {
      (void)sme_QosProcessBufferedCmd();
   }
   else
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcFullPowercallback: PMC failed to put the chip in Full power\n");

      //ASSERT
      VOS_ASSERT(0);
   }

}


static void sme_QosHandleCallback(eHalStatus status)
{
#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

   tListElem *pEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   v_U8_t index = 0;

   for(index = 0; index < sme_QosCb.apsd_req_counter; index++)
   {
      pEntry = sme_QosCb.uapsdRequest[index].pEntry;
      if(!pEntry)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                    "sme_QosPmcStartUAPSDcallback: callbackContext is NULL\n");
         //ASSERT
         VOS_ASSERT(0);
         return;
      }

      flow_info = GET_BASE_ADDR(pEntry, sme_QosFlowInfoEntry, link);
      if(!flow_info)
      {
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcStartUAPSDcallback: couldn't access the QoS \
                params from the Flow List entry\n");
         //ASSERT
         VOS_ASSERT(0);
         return;
      }

      if(SME_QOS_REASON_MODIFY_REQ_APSD_PENDING == flow_info->reason)
      {
         hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
         flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      }
      else if(SME_QOS_REASON_SETUP_REQ_APSD_PENDING == flow_info->reason)
      {
         hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
         flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      }

      flow_info->QoSCallback(sme_QosCb.uapsdRequest[index].pMac, flow_info->HDDcontext, 
                             &sme_QosCb.ac_info[flow_info->ac_type].curr_QoSInfo[flow_info->tspec_mask - 1],
                             hdd_status,
                             flow_info->QosFlowID);
      sme_QosCb.uapsdRequest[index].pEntry = NULL;
      sme_QosCb.uapsdRequest[index].pMac   = NULL;
   }

   sme_QosCb.apsd_req_counter = 0;

#endif //#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
}


/*--------------------------------------------------------------------------
  \brief sme_QosPmcStartUAPSDCallback() - Callback function registered with PMC 
  to notify SME-QoS when it puts the chip into UAPSD mode
  
  \param callbackContext - The context passed to PMC during pmcStartUapsd call.
  \param status - eHalStatus returned by PMC.
  
  \return None
  
  \sa
  
  --------------------------------------------------------------------------*/
void sme_QosPmcStartUapsdCallback(void *callbackContext, eHalStatus status)
{

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
   //We don't need to do anything here when FW is controlling sending trigger frame
#else

   tListElem *pEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   sme_QosStatusType hdd_status = SME_QOS_STATUS_SETUP_FAILURE_RSP;
   v_U8_t index = 0;

   if(!sme_QosCb.apsd_req_counter)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcStartUapsdCallback: PMC invoked the callback but no \
                request is pending\n");
      return;
   }

   //As per aggrement, PMC shouldn't return a failure status
   if(eHAL_STATUS_FAILURE == status)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcStartUapsdCallback: PMC returned failure status\n");
      //Removing the assert, as PMC can recover from the error conditions or in
      //case of fatal errors PMC itself has asserts to take care of it 
      //VOS_ASSERT(0);
      return;
   }

   for(index = 0; index < sme_QosCb.apsd_req_counter; index++)
   {
      pEntry = sme_QosCb.uapsdRequest[index].pEntry;
      if(!pEntry)
      {
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                    "sme_QosPmcStartUAPSDcallback: callbackContext is NULL\n");
         //ASSERT
         VOS_ASSERT(0);
         return;
      }

      flow_info = GET_BASE_ADDR(pEntry, sme_QosFlowInfoEntry, link);
      if(!flow_info)
      {
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcStartUAPSDcallback: couldn't access the QoS \
                params from the Flow List entry\n");
         //ASSERT
         VOS_ASSERT(0);
         return;
      }

      if(SME_QOS_REASON_MODIFY_REQ_APSD_PENDING == flow_info->reason)
      {
         hdd_status = SME_QOS_STATUS_MODIFY_SETUP_SUCCESS_IND;
         flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      }
      else if(SME_QOS_REASON_SETUP_REQ_APSD_PENDING == flow_info->reason)
      {
         hdd_status = SME_QOS_STATUS_SETUP_SUCCESS_IND;
         flow_info->reason = SME_QOS_REASON_REQ_SUCCESS;
      }

      flow_info->QoSCallback(sme_QosCb.uapsdRequest[index].pMac, flow_info->HDDcontext, 
                             &sme_QosCb.ac_info[flow_info->ac_type].curr_QoSInfo[flow_info->tspec_mask - 1],
                             hdd_status,
                             flow_info->QosFlowID);
      sme_QosCb.uapsdRequest[index].pEntry = NULL;
      sme_QosCb.uapsdRequest[index].pMac   = NULL;
   }

   sme_QosCb.apsd_req_counter = 0;

#endif  //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

   sme_QosCb.uapsdAlreadyRequested = VOS_FALSE;
}

/*--------------------------------------------------------------------------
  \brief sme_QosPmcCheckRoutine() - Function registered with PMC to check with 
  SME-QoS whenever the device is about to enter one of the power 
  save modes. PMC runs a poll with all the registered modules if device can 
  enter powersave mode or remain in full power  
  
  \param callbackContext - The context passed to PMC during registration through
  pmcRegisterPowerSaveCheck.

  \return boolean
  
  SME-QOS returns PMC true or false respectively if it wants to vote for 
  entering power save or not
  
  \sa
  
  --------------------------------------------------------------------------*/
v_BOOL_t sme_QosPmcCheckRoutine(void *callbackContext)
{
   return (sme_QosCb.readyForPowerSave);
}

/*--------------------------------------------------------------------------
  \brief sme_QosPmcDeviceStateUpdateInd() - Callback function registered with 
  PMC to notify SME-QoS when it changes the power state
  
  \param callbackContext - The context passed to PMC during registration 
  through pmcRegisterDeviceStateUpdateInd.
  \param pmcState - Current power state that PMC moved into.
  
  \return None
  
  \sa
  
  --------------------------------------------------------------------------*/
void sme_QosPmcDeviceStateUpdateInd(void *callbackContext, tPmcState pmcState)
{
   eHalStatus status = eHAL_STATUS_FAILURE;
   tpAniSirGlobal pMac = PMAC_STRUCT( callbackContext );
   //check all the entries in Flow list for non-zero service interval, which will
   //tell us if we need to notify HDD when PMC is out of UAPSD mode or going 
   // back to UAPSD mode

   switch(pmcState)
   {
   case FULL_POWER:
      status = sme_QosProcessOutOfUapsdMode(pMac);
      break;
   case UAPSD:
      status = sme_QosProcessIntoUapsdMode(pMac);
      break;
   default:
      status = eHAL_STATUS_SUCCESS;
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcDeviceStateUpdateInd:nothing to process in PMC state %d\n", 
                pmcState);

   }
   if(!HAL_STATUS_SUCCESS(status))
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcDeviceStateUpdateInd:ignoring Device(PMC)\
                 state change to %d\n", pmcState);

   }


}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessOutOfUapsdMode() - Function to notify HDD when PMC 
  notifies SME-QoS that it moved out of UAPSD mode to FULL power
  
  \param pMac - Pointer to the global MAC parameter structure.
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessOutOfUapsdMode(tpAniSirGlobal pMac)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;
   

   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosPmcChangePowerModeCallback: Flow List empty, can't search\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if(!flow_info)
      {
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosPmcChangePowerModeCallback: couldn't access the QoS \
                   params from the Flow List entry\n");

         return eHAL_STATUS_FAILURE;
      }

      //only notify the flows which already successfully setup UAPSD
      if((flow_info->QoSInfo.max_service_interval || flow_info->QoSInfo.min_service_interval) &&
         (SME_QOS_REASON_REQ_SUCCESS == flow_info->reason))
      {
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                &sme_QosCb.ac_info[flow_info->ac_type].curr_QoSInfo[flow_info->tspec_mask - 1],
                                SME_QOS_STATUS_OUT_OF_APSD_POWER_MODE_IND,
                                flow_info->QosFlowID);

      }
      pEntry = pNextEntry;
   }

   return eHAL_STATUS_SUCCESS;
}

/*--------------------------------------------------------------------------
  \brief sme_QosProcessIntoUapsdMode() - Function to notify HDD when PMC 
  notifies SME-QoS that it is moving into UAPSD mode 
  
  \param pMac - Pointer to the global MAC parameter structure.
  \return eHalStatus
  
  \sa
  
  --------------------------------------------------------------------------*/
eHalStatus sme_QosProcessIntoUapsdMode(tpAniSirGlobal pMac)
{
   tListElem *pEntry= NULL, *pNextEntry = NULL;
   sme_QosFlowInfoEntry *flow_info = NULL;

   
   pEntry = csrLLPeekHead( &sme_QosCb.flow_list, VOS_FALSE );
   if(!pEntry)
   {
      VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                "sme_QosProcessIntoUapsdMode: Flow List empty, can't search\n");
      return eHAL_STATUS_FAILURE;
   }

   while( pEntry )
   {
      pNextEntry = csrLLNext( &sme_QosCb.flow_list, pEntry, VOS_FALSE );
      flow_info = GET_BASE_ADDR( pEntry, sme_QosFlowInfoEntry, link );
      if(!flow_info)
      {
         //Err msg
         VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, 
                   "sme_QosProcessIntoUapsdMode: couldn't access the QoS \
                   params from the Flow List entry\n");

         return eHAL_STATUS_FAILURE;
      }
      //only notify the flows which already successfully setup UAPSD
      if((flow_info->QoSInfo.max_service_interval || flow_info->QoSInfo.min_service_interval) &&
         (SME_QOS_REASON_REQ_SUCCESS == flow_info->reason))
      {
         flow_info->QoSCallback(pMac, flow_info->HDDcontext, 
                                &sme_QosCb.ac_info[flow_info->ac_type].curr_QoSInfo[flow_info->tspec_mask - 1],
                                SME_QOS_STATUS_INTO_APSD_POWER_MODE_IND,
                                flow_info->QosFlowID);

      }
      pEntry = pNextEntry;
   }

   return eHAL_STATUS_SUCCESS;
}


void sme_QosCleanupCtrlBlkForHandoff(tpAniSirGlobal pMac)
{
   sme_QosEdcaAcType ac;
   for(ac = SME_QOS_EDCA_AC_BE; ac < SME_QOS_EDCA_AC_MAX; ac++) 
   {
      vos_mem_zero(sme_QosCb.ac_info[ac].curr_QoSInfo, 
                   sizeof(sme_QosWmmTspecInfo) * SME_QOS_TSPEC_INDEX_MAX);
      vos_mem_zero(sme_QosCb.ac_info[ac].requested_QoSInfo, 
                   sizeof(sme_QosWmmTspecInfo) * SME_QOS_TSPEC_INDEX_MAX);
      sme_QosCb.ac_info[ac].num_flows[0] = 0;
      sme_QosCb.ac_info[ac].num_flows[1] = 0;
      sme_QosCb.ac_info[ac].reassoc_pending = VOS_FALSE;
      sme_QosCb.ac_info[ac].tspec_mask_status = 0;
      sme_QosCb.ac_info[ac].tspec_pending = VOS_FALSE;
      sme_QosCb.ac_info[ac].hoRenewal = VOS_FALSE;
      sme_QosCb.ac_info[ac].prev_state = SME_QOS_LINK_UP;
   }
}

v_BOOL_t sme_QosValidateRequestedParams(sme_QosWmmTspecInfo * pQoSInfo)
{
   v_BOOL_t rc = VOS_FALSE;

   do
   {
      if(SME_QOS_WMM_TS_DIR_RESV == pQoSInfo->ts_info.direction) break;
      
      rc = VOS_TRUE;
   }while(0);

   return rc;
}


eHalStatus qosIssueCommand( tpAniSirGlobal pMac, eSmeCommandType cmdType, void *pvParam, tANI_U32 size,
                            sme_QosEdcaAcType ac, v_U8_t tspec_mask )
{
    eHalStatus status = eHAL_STATUS_RESOURCES;
    tSmeCmd *pCommand = NULL;

    do
    {
        pCommand = smeGetCommandBuffer( pMac );
        if ( !pCommand ) break;
        pCommand->command = cmdType;
        switch ( cmdType )
        {
        case eSmeCommandAddTs:
            status = eHAL_STATUS_SUCCESS;
            if( pvParam )
            {
                pCommand->u.qosCmd.tspecInfo = *( (sme_QosWmmTspecInfo *)pvParam );
            }
            pCommand->u.qosCmd.ac = ac;
            break;

        case eSmeCommandDelTs:
            status = eHAL_STATUS_SUCCESS;
                pCommand->u.qosCmd.ac = ac;
                pCommand->u.qosCmd.tspec_mask = tspec_mask;
            break;

        default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, " qosIssueCommand invalid command type %d\n", cmdType );
            status = eHAL_STATUS_INVALID_PARAMETER;
            break;
        }

    } while( 0 );

    if( HAL_STATUS_SUCCESS( status ) && pCommand )
    {
        smePushCommand( pMac, pCommand, eANI_BOOLEAN_FALSE );
    }
    else if( pCommand )
    {
        qosReleaseCommand( pMac, pCommand );
    }

    return( status );
}


tANI_BOOLEAN qosProcessCommand( tpAniSirGlobal pMac, tSmeCmd *pCommand )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN fRemoveCmd = eANI_BOOLEAN_TRUE;

    do
    {
        switch ( pCommand->command )
        {
        case eSmeCommandAddTs:
            status = sme_QosAddTsReq( pMac, &pCommand->u.qosCmd.tspecInfo, pCommand->u.qosCmd.ac);
            if( HAL_STATUS_SUCCESS( status ) )
            {
                fRemoveCmd = eANI_BOOLEAN_FALSE;
                status = SME_QOS_STATUS_SETUP_REQ_PENDING_RSP;
            }
            break;

        case eSmeCommandDelTs:
            status = sme_QosDelTsReq( pMac, pCommand->u.qosCmd.ac, pCommand->u.qosCmd.tspec_mask );
            if( HAL_STATUS_SUCCESS( status ) )
            {
                fRemoveCmd = eANI_BOOLEAN_FALSE;
            }
            break;

        default:
            VOS_TRACE(VOS_MODULE_ID_SME, VOS_TRACE_LEVEL_ERROR, " qosProcessCommand invalid command type %d\n", pCommand->command );
            break;
        }//switch
    } while(0);

    return( fRemoveCmd );
}
