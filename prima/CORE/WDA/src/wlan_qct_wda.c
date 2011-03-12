
/*===========================================================================

                       W L A N _ Q C T _ WDA . C

  OVERVIEW:

  This software unit holds the implementation of the WLAN Transport Layer.

  The functions externalized by this module are to be called ONLY by other
  WLAN modules that properly register with the Transport Layer initially.

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


  when        who        what, where, why
----------    ---       -------------------------------------------------
2010-12-30    smiryala     UMAC convergence changes
2010-08-19    adwivedi    WLAN DAL AL(WDA) layer for Prima
===========================================================================*/

#if defined( FEATURE_WLAN_INTEGRATED_SOC )

#include "vos_mq.h" 
#include "vos_api.h" 
#include "vos_packet.h" 
#include "sirApi.h"
#include "wlan_qct_pal_packet.h"
#include "wlan_qct_wda.h"
#include "wlan_qct_wda_msg.h"
#include "wlan_qct_wdi_cfg.h"
#include "wlan_qct_wdi.h"
#include "wlan_qct_wdi_ds.h"
#include "wlan_hal_cfg.h"
/**********************/
#include "wniApi.h"
#include "cfgApi.h"
#include "limApi.h"


/* Used MACRO's */
/* Get WDA context from vOSS module */
#define VOS_GET_WDA_CTXT(a)            vos_get_context(VOS_MODULE_ID_WDA, a)
#define VOS_GET_MAC_CTXT(a)            vos_get_context(VOS_MODULE_ID_PE, a)
#define OFFSET_OF(structType,fldName)   (&((structType*)0)->fldName)

 
#define WDA_VOS_ALLOC_FAIL(x)   \
   do \
   {   \
      if(NULL == x) \
         { VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR\
           ,"%s: VOS Alloc Failure", __FUNCTION__); \
           VOS_ASSERT(0) ; \
           return VOS_STATUS_E_NOMEM; } \
   } while(0) ;

#define WDA_VOS_ASSERT(x)   \
   do \
   {   \
      if(!(x)) \
      { VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR\
      ,"%s: Failure", __FUNCTION__); \
      VOS_ASSERT(0) ; } \
   } while(0) ;

#define  CONVERT_WDI2SIR_STATUS(x) \
   ((WDI_STATUS_SUCCESS != x) ? eSIR_FAILURE : eSIR_SUCCESS)

#define  CONVERT_WDI2VOS_STATUS(x) \
   ((WDI_STATUS_SUCCESS != x) ? VOS_STATUS_E_FAILURE  : VOS_STATUS_SUCCESS)

#define  IS_WDI_STATUS_FAILURE(status) \
   ((WDI_STATUS_SUCCESS != status) && (WDI_STATUS_PENDING != status))

/* extern declarations */
extern void vos_WDAComplete_cback(v_PVOID_t pVosContext);

/* forward declarations */
void WDA_SendMsg(tWDA_CbContext *pWDA, tANI_U16 msgType, 
                                        void *pBodyptr, tANI_U32 bodyVal) ;
VOS_STATUS WDA_prepareConfigTLV(v_PVOID_t pVosContext, 
                                WDI_StartReqParamsType  *wdiStartParams ) ;

VOS_STATUS WDA_wdiCompleteCB(v_PVOID_t pVosContext) ;

extern v_BOOL_t sys_validateStaConfig( void *pImage, unsigned long cbFile,
                               void **ppStaConfig, v_SIZE_t *pcbStaConfig ) ;
void processCfgDownloadReq(tpAniSirGlobal pMac, tANI_U16 length, 
                                                         tANI_U32 *pConfig) ;
void WDA_UpdateBSSParams(tWDA_CbContext *pWDA, 
        WDI_ConfigBSSReqInfoType *wdiBssParams, tAddBssParams *wdaBssParams) ;
void WDA_UpdateSTAParams(tWDA_CbContext *pWDA, 
        WDI_ConfigStaReqInfoType *wdiStaParams, tAddStaParams *wdaStaParams) ;
void WDA_lowLevelIndCallback(WDI_LowLevelIndType *wdiLowLevelInd, 
                                                          void* pUserData ) ;

 
/*
 * FUNCTION: WDA_open
 * Allocate the WDA context 
 */ 

VOS_STATUS WDA_open(v_PVOID_t pVosContext, v_PVOID_t pOSContext,
                                                tMacOpenParameters *pMacParams )
{
   tWDA_CbContext *wdaContext;
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   WDI_DeviceCapabilityType wdiDevCapability = {0} ;

   /* Allocate WDA context */
   status = vos_alloc_context(pVosContext, VOS_MODULE_ID_WDA, 
                           (v_VOID_t **)&wdaContext, sizeof(tWDA_CbContext)) ;
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      return VOS_STATUS_E_NOMEM;
   }

   /*__asm int 3;*/
   vos_mem_zero(wdaContext,sizeof(tWDA_CbContext));
   
   /* Initialize data structures */
   wdaContext->pVosContext = pVosContext;
   wdaContext->wdaState = WDA_INIT_STATE;
   wdaContext->uTxFlowMask = WDA_TXFLOWMASK;
   
   /* Init Frame transfer event */
   status = vos_event_init(&wdaContext->txFrameEvent);
   if(!VOS_IS_STATUS_SUCCESS(status)) 
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "VOS Mgmt Frame Event init failed - tatus = %d\n", status);
      status = VOS_STATUS_E_FAILURE;
   }

   vos_trace_setLevel(VOS_MODULE_ID_WDA,VOS_TRACE_LEVEL_ERROR);

   if(WDI_STATUS_SUCCESS != WDI_Init(pOSContext, &wdaContext->pWdiContext, 
                                                            &wdiDevCapability))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "WDI Init failed" );
      status = VOS_STATUS_E_FAILURE;
   }
   else 
   {
      pMacParams->maxStation = wdiDevCapability.ucMaxSTASupported ;
      pMacParams->maxBssId =  wdiDevCapability.ucMaxBSSSupported;
      pMacParams->frameTransRequired = wdiDevCapability.bFrameXtlSupported;
      /* store the frameTransRequired flag in wdaContext, to send this to HAL 
      * in WDA_Start*/
      wdaContext->frameTransRequired = wdiDevCapability.bFrameXtlSupported;
   }

   return status;
}


/*
 * FUNCTION: WDA_preStart
 * Trigger DAL-AL to start CFG download 
 */ 
VOS_STATUS WDA_preStart(v_PVOID_t pVosContext)
{   
   VOS_STATUS vosStatus = VOS_STATUS_SUCCESS;
   vos_msg_t wdaMsg = {0} ;

   /*
    * trigger CFG download in WDA by sending WDA_CFG_DNLD message
    */ 
   wdaMsg.type = WNI_CFG_DNLD_REQ ; 
   wdaMsg.bodyptr = NULL;
   wdaMsg.bodyval = 0;

   /* post the message.. */
   vosStatus = vos_mq_post_message( VOS_MQ_ID_WDA, &wdaMsg );
   if ( !VOS_IS_STATUS_SUCCESS(vosStatus) )
   {
      vosStatus = VOS_STATUS_E_BADMSG;
   }

   return( vosStatus );
}

/*
 * FUNCTION: WDA_startCalback
 * Once WDI_start is finished, WDI start callback will be called by WDI
 * to indicate completion of WDI_Start. 
 */
void WDA_wdiStartCallback(WDI_StartRspParamsType *wdiRspParams, 
                                                            void *pVosContext)
{
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);

   /* Free'ing allocated memory for WDI msg params as well as config params */
   if(NULL != wdaContext->wdaWdiApiMsgParam)
   {
      WDI_StartReqParamsType *wdiStartParam = (WDI_StartReqParamsType *)
                                               wdaContext->wdaWdiApiMsgParam ;
      if(NULL != wdiStartParam->pConfigBuffer)
      {
         vos_mem_free(wdiStartParam->pConfigBuffer);
      }
      else
      {
         WDA_VOS_ASSERT(0) ;
      }
      vos_mem_free(wdaContext->wdaWdiApiMsgParam) ;
      wdaContext->wdaWdiApiMsgParam = NULL;
   }

   if(WDI_STATUS_SUCCESS != wdiRspParams->wdiStatus)
   {
      WDA_VOS_ASSERT(0) ;
   }
   else
   {
      wdaContext->wdaState = WDA_START_STATE;
   }

   /* Indicate VOSS about the start complete */
   vos_WDAComplete_cback(pVosContext);
   return ;
}

/*
 * FUNCTION: WDA_start
 * Prepare TLV configuration and call EDI_start. 
 */
 
VOS_STATUS WDA_start(v_PVOID_t pVosContext)
{
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   WDI_Status wdiStatus;
   WDI_StartReqParamsType *wdiStartParam = 
                    (WDI_StartReqParamsType *)vos_mem_malloc(
                                             sizeof(WDI_StartReqParamsType)) ;

   vos_mem_set(wdiStartParam, sizeof(*wdiStartParam), 0);

   WDA_VOS_ASSERT(WDA_INIT_STATE == wdaContext->wdaState);

   WDA_VOS_ALLOC_FAIL(wdiStartParam);

   if( NULL == wdiStartParam )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                               "%s fail to allocate memory", __FUNCTION__ );
      return VOS_STATUS_E_NOMEM;
   }

#ifndef WLAN_FTM_PLAT
   wdiStartParam->wdiDriverType = eDRIVER_TYPE_PRODUCTION;
#else
   wdiStartParam->wdiDriverType = eDRIVER_TYPE_MFG;
#endif

   /* prepare the config TLV for the WDI */
   status = WDA_prepareConfigTLV(pVosContext, wdiStartParam) ;

   if ( !VOS_IS_STATUS_SUCCESS(status) )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                     "Error prepare TLV for WDA" );
      status = VOS_STATUS_E_FAILURE;
   }
   else
   {
      wdiStartParam->wdiLowLevelIndCB = WDA_lowLevelIndCallback ;
      wdiStartParam->pIndUserData = (v_PVOID_t *)wdaContext ;

      wdiStartParam->wdiReqStatusCB = NULL;

      /* check if already there is a WDI request */
      WDA_VOS_ASSERT(NULL == wdaContext->wdaWdiApiMsgParam);

      /* store Params pass it to WDI */
      wdaContext->wdaWdiApiMsgParam = (v_PVOID_t *)wdiStartParam ;

      /* call WDI start */
      wdiStatus = WDI_Start(wdiStartParam, 
                (WDI_StartRspCb)WDA_wdiStartCallback,(v_VOID_t *)pVosContext);
      if ( WDI_STATUS_SUCCESS != wdiStatus )
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                     "WDI Start failed" );
         vos_mem_free(wdaContext->wdaWdiApiMsgParam);
         wdaContext->wdaWdiApiMsgParam = NULL;
         status = VOS_STATUS_E_FAILURE;
      }
   }

   return status;
}


/*
 * FUNCTION: WDA_prepareConfigTLV
 * Function to prepare CFG for DAL(WDA)
 */
VOS_STATUS WDA_prepareConfigTLV(v_PVOID_t pVosContext, 
                            WDI_StartReqParamsType  *wdiStartParams )
{
   /* get pMac to acess CFG data base */
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pVosContext);
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);
   tHalCfg        *tlvStruct = NULL ;
   tANI_U8        *tlvStructStart = NULL ;
   tANI_U32       strLength = WNI_CFG_STA_ID_LEN;
   v_PVOID_t      *configParam;
   tANI_U32       configParamSize;
   tANI_U32       *configDataValue;

   configParamSize = (sizeof(tHalCfg) * QWLAN_HAL_CFG_MAX_PARAMS) + 
                           WNI_CFG_STA_ID_LEN +
                           WNI_CFG_EDCA_WME_ACBK_LEN +
                           WNI_CFG_EDCA_WME_ACBE_LEN +
                           WNI_CFG_EDCA_WME_ACVI_LEN +
                           WNI_CFG_EDCA_WME_ACVO_LEN +
                           + (QWLAN_HAL_CFG_INTEGER_PARAM * sizeof(tANI_U32));

   /* malloc memory for all configs in one shot */ 
   configParam = vos_mem_malloc(configParamSize);
   
   if(NULL == configParam )
   {
      WDA_VOS_ASSERT(0) ;
      return VOS_STATUS_E_NOMEM;
   }
   vos_mem_set(configParam, configParamSize, 0);

   wdiStartParams->pConfigBuffer = configParam;

   tlvStruct = (tHalCfg *)configParam;
   tlvStructStart = (tANI_U8 *)configParam;

   /* TODO: Remove Later */
   /* This CFG ID is updating the MAC address to HAL from the CFG 
    * so commented for now */
#if 0
   /* QWLAN_HAL_CFG_STA_ID */
   tlvStruct->type = QWLAN_HAL_CFG_STA_ID;
   configDataValue = ((tANI_U8 *) tlvStruct + sizeof(tHalCfg));
   if(wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, configDataValue, &strLength) != 
                                                                eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                   "Failed to get value for WNI_CFG_STA_ID");
      goto handle_failure;
   }
   tlvStruct->length = strLength ;
   /* calculate the pad bytes to have the CFG in aligned format */
   tlvStruct->padBytes = ALIGNED_WORD_SIZE - 
                              (tlvStruct->length & (ALIGNED_WORD_SIZE - 1));

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
               + sizeof(tHalCfg) + tlvStruct->length + tlvStruct->padBytes)) ;
#endif
   /* QWLAN_HAL_CFG_CURRENT_TX_ANTENNA */
   tlvStruct->type = QWLAN_HAL_CFG_CURRENT_TX_ANTENNA;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_CURRENT_TX_ANTENNA, configDataValue ) 
                                                    != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                          "Failed to get value for WNI_CFG_CURRENT_TX_ANTENNA");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_CURRENT_RX_ANTENNA */
   tlvStruct->type = QWLAN_HAL_CFG_CURRENT_RX_ANTENNA;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_CURRENT_RX_ANTENNA, configDataValue) != 
                                                                  eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_CURRENT_RX_ANTENNA");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_LOW_GAIN_OVERRIDE */
   tlvStruct->type = QWLAN_HAL_CFG_LOW_GAIN_OVERRIDE;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_LOW_GAIN_OVERRIDE, configDataValue ) 
                                                 != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_LOW_GAIN_OVERRIDE");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ;
 
   /* QWLAN_HAL_CFG_POWER_STATE_PER_CHAIN */
   tlvStruct->type = QWLAN_HAL_CFG_POWER_STATE_PER_CHAIN;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_POWER_STATE_PER_CHAIN, 
                                            configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_POWER_STATE_PER_CHAIN");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)); 

   /* QWLAN_HAL_CFG_CAL_PERIOD */
   tlvStruct->type = QWLAN_HAL_CFG_CAL_PERIOD;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_CAL_PERIOD, configDataValue ) 
                                                  != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_CAL_PERIOD");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));

   /* QWLAN_HAL_CFG_CAL_CONTROL  */
   tlvStruct->type = QWLAN_HAL_CFG_CAL_CONTROL ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_CAL_CONTROL, configDataValue ) 
                                                   != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_CAL_CONTROL");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));

   /* QWLAN_HAL_CFG_PROXIMITY  */
   tlvStruct->type = QWLAN_HAL_CFG_PROXIMITY ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_PROXIMITY, configDataValue ) 
                                                   != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_PROXIMITY");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                             + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_NETWORK_DENSITY  */
   tlvStruct->type = QWLAN_HAL_CFG_NETWORK_DENSITY ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_NETWORK_DENSITY, configDataValue ) 
                                                   != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_NETWORK_DENSITY");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)); 

   /* QWLAN_HAL_CFG_MAX_MEDIUM_TIME  */
   tlvStruct->type = QWLAN_HAL_CFG_MAX_MEDIUM_TIME ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_MEDIUM_TIME, configDataValue ) != 
                                                                 eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_MAX_MEDIUM_TIME");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)); 

   /* QWLAN_HAL_CFG_MAX_MPDUS_IN_AMPDU   */
   tlvStruct->type = QWLAN_HAL_CFG_MAX_MPDUS_IN_AMPDU  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_MPDUS_IN_AMPDU,
                                            configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_MAX_MPDUS_IN_AMPDU");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ;

   /* QWLAN_HAL_CFG_RTS_THRESHOLD   */
   tlvStruct->type = QWLAN_HAL_CFG_RTS_THRESHOLD  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_RTS_THRESHOLD, configDataValue ) != 
                                                                  eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_RTS_THRESHOLD");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));

   /* QWLAN_HAL_CFG_SHORT_RETRY_LIMIT   */
   tlvStruct->type = QWLAN_HAL_CFG_SHORT_RETRY_LIMIT  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_SHORT_RETRY_LIMIT, configDataValue ) != 
                                                                 eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_SHORT_RETRY_LIMIT");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_LONG_RETRY_LIMIT   */
   tlvStruct->type = QWLAN_HAL_CFG_LONG_RETRY_LIMIT  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_LONG_RETRY_LIMIT, configDataValue ) != 
                                                                 eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_LONG_RETRY_LIMIT");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ;

   /* QWLAN_HAL_CFG_FRAGMENTATION_THRESHOLD   */
   tlvStruct->type = QWLAN_HAL_CFG_FRAGMENTATION_THRESHOLD  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, 
                                             configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_FRAGMENTATION_THRESHOLD");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ;

   /* QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ZERO   */
   tlvStruct->type = QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ZERO  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_ZERO,
                                             configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DYNAMIC_THRESHOLD_ZERO");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));
 
   /* QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ONE   */
   tlvStruct->type = QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ONE  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_ONE, 
                                             configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DYNAMIC_THRESHOLD_ONE");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)); 

   /* QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_TWO   */
   tlvStruct->type = QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_TWO  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_TWO, 
                                             configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DYNAMIC_THRESHOLD_TWO");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));
 
   /* QWLAN_HAL_CFG_FIXED_RATE   */
   tlvStruct->type = QWLAN_HAL_CFG_FIXED_RATE  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_FIXED_RATE, configDataValue) 
                                                      != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_FIXED_RATE");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));
 
   /* QWLAN_HAL_CFG_RETRYRATE_POLICY   */
   tlvStruct->type = QWLAN_HAL_CFG_RETRYRATE_POLICY  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_RETRYRATE_POLICY, configDataValue ) 
                                                         != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_RETRYRATE_POLICY");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length));
 
   /* QWLAN_HAL_CFG_RETRYRATE_SECONDARY   */
   tlvStruct->type = QWLAN_HAL_CFG_RETRYRATE_SECONDARY  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_RETRYRATE_SECONDARY, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_RETRYRATE_SECONDARY");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_RETRYRATE_TERTIARY   */
   tlvStruct->type = QWLAN_HAL_CFG_RETRYRATE_TERTIARY  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_RETRYRATE_TERTIARY, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_RETRYRATE_TERTIARY");
      goto handle_failure;
   }
   tlvStruct = (tHalCfg *)(( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length)) ; 

   /* QWLAN_HAL_CFG_FORCE_POLICY_PROTECTION   */
   tlvStruct->type = QWLAN_HAL_CFG_FORCE_POLICY_PROTECTION  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_FORCE_POLICY_PROTECTION");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_24GHZ   */
   tlvStruct->type = QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_24GHZ  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_FIXED_RATE_MULTICAST_24GHZ, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "Failed to get value for WNI_CFG_FIXED_RATE_MULTICAST_24GHZ");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_5GHZ   */
   tlvStruct->type = QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_5GHZ  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_FIXED_RATE_MULTICAST_5GHZ, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "Failed to get value for WNI_CFG_FIXED_RATE_MULTICAST_5GHZ");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_24GHZ   */
   tlvStruct->type = QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_24GHZ  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DEFAULT_RATE_INDEX_24GHZ, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DEFAULT_RATE_INDEX_24GHZ");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_5GHZ   */
   tlvStruct->type = QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_5GHZ  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DEFAULT_RATE_INDEX_5GHZ, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DEFAULT_RATE_INDEX_5GHZ");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                             + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_MAX_BA_SESSIONS   */
   tlvStruct->type = QWLAN_HAL_CFG_MAX_BA_SESSIONS  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_BA_SESSIONS, configDataValue ) != 
                                                                eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_MAX_BA_SESSIONS");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                             + sizeof(tHalCfg) + tlvStruct->length);
 
   /* QWLAN_HAL_CFG_PS_DATA_INACTIVITY_TIMEOUT   */
   tlvStruct->type = QWLAN_HAL_CFG_PS_DATA_INACTIVITY_TIMEOUT  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT, 
                                            configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "Failed to get value for WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                             + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_PS_ENABLE_BCN_FILTER   */
   tlvStruct->type = QWLAN_HAL_CFG_PS_ENABLE_BCN_FILTER  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_PS_ENABLE_BCN_FILTER, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_PS_ENABLE_BCN_FILTER");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                             + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_PS_ENABLE_RSSI_MONITOR   */
   tlvStruct->type = QWLAN_HAL_CFG_PS_ENABLE_RSSI_MONITOR  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_PS_ENABLE_RSSI_MONITOR, 
                                              configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_PS_ENABLE_RSSI_MONITOR");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                              + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_NUM_BEACON_PER_RSSI_AVERAGE   */
   tlvStruct->type = QWLAN_HAL_CFG_NUM_BEACON_PER_RSSI_AVERAGE  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE, 
                                            configDataValue ) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                "Failed to get value for WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                              + sizeof(tHalCfg) + tlvStruct->length);
 
   /* QWLAN_HAL_CFG_STATS_PERIOD   */
   tlvStruct->type = QWLAN_HAL_CFG_STATS_PERIOD  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_STATS_PERIOD, configDataValue ) != 
                                                                eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_STATS_PERIOD");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                               + sizeof(tHalCfg) + tlvStruct->length); 

   /* QWLAN_HAL_CFG_CFP_MAX_DURATION   */
   tlvStruct->type = QWLAN_HAL_CFG_CFP_MAX_DURATION  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_CFP_MAX_DURATION, configDataValue ) != 
                                                                eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_CFP_MAX_DURATION");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length) ; 

   /* QWLAN_HAL_CFG_FRAME_TRANS_ENABLED */
   tlvStruct->type = QWLAN_HAL_CFG_FRAME_TRANS_ENABLED  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   vos_mem_copy(configDataValue, &wdaContext->frameTransRequired, 
                                               sizeof(tANI_U32));

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length) ; 

   /* QWLAN_HAL_CFG_DTIM_PERIOD */
   tlvStruct->type = QWLAN_HAL_CFG_DTIM_PERIOD  ;
   tlvStruct->length = sizeof(tANI_U32);
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetInt(pMac, WNI_CFG_DTIM_PERIOD, configDataValue) 
                                                   != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_DTIM_PERIOD");
      goto handle_failure;
   }

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                            + sizeof(tHalCfg) + tlvStruct->length) ; 

   /* QWLAN_HAL_CFG_EDCA_WMM_ACBK */
   tlvStruct->type = QWLAN_HAL_CFG_EDCA_WMM_ACBK  ;
   strLength = WNI_CFG_EDCA_WME_ACBK_LEN;
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetStr(pMac, WNI_CFG_EDCA_WME_ACBK, (tANI_U8 *)configDataValue,
                                             &strLength) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_EDCA_WME_ACBK");
      goto handle_failure;
   }
   tlvStruct->length = strLength;
   /* calculate the pad bytes to have the CFG in aligned format */
   tlvStruct->padBytes = ALIGNED_WORD_SIZE - 
                              (tlvStruct->length & (ALIGNED_WORD_SIZE - 1));

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                 + sizeof(tHalCfg) + tlvStruct->length + tlvStruct->padBytes) ; 

   /* QWLAN_HAL_CFG_EDCA_WMM_ACBE */
   tlvStruct->type = QWLAN_HAL_CFG_EDCA_WMM_ACBE  ;
   strLength = WNI_CFG_EDCA_WME_ACBE_LEN;
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetStr(pMac, WNI_CFG_EDCA_WME_ACBE, (tANI_U8 *)configDataValue,
                                             &strLength) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_EDCA_WME_ACBE");
      goto handle_failure;
   }
   tlvStruct->length = strLength;
   /* calculate the pad bytes to have the CFG in aligned format */
   tlvStruct->padBytes = ALIGNED_WORD_SIZE - 
                              (tlvStruct->length & (ALIGNED_WORD_SIZE - 1));

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                 + sizeof(tHalCfg) + tlvStruct->length + tlvStruct->padBytes) ; 

   /* QWLAN_HAL_CFG_EDCA_WMM_ACVI */
   tlvStruct->type = QWLAN_HAL_CFG_EDCA_WMM_ACVO  ;
   strLength = WNI_CFG_EDCA_WME_ACVI_LEN;
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetStr(pMac, WNI_CFG_EDCA_WME_ACVO, (tANI_U8 *)configDataValue,
                                             &strLength) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_EDCA_WME_ACVI");
      goto handle_failure;
   }
   tlvStruct->length = strLength;
   /* calculate the pad bytes to have the CFG in aligned format */
   tlvStruct->padBytes = ALIGNED_WORD_SIZE - 
                              (tlvStruct->length & (ALIGNED_WORD_SIZE - 1));

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                 + sizeof(tHalCfg) + tlvStruct->length + tlvStruct->padBytes) ; 

   /* QWLAN_HAL_CFG_EDCA_WMM_ACVO */
   tlvStruct->type = QWLAN_HAL_CFG_EDCA_WMM_ACVI  ;
   strLength = WNI_CFG_EDCA_WME_ACVO_LEN;
   configDataValue = (tANI_U32 *)(tlvStruct + 1);
   if(wlan_cfgGetStr(pMac, WNI_CFG_EDCA_WME_ACVI, (tANI_U8 *)configDataValue,
                                             &strLength) != eSIR_SUCCESS)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                    "Failed to get value for WNI_CFG_EDCA_WME_ACVO");
      goto handle_failure;
   }
   tlvStruct->length = strLength;
   /* calculate the pad bytes to have the CFG in aligned format */
   tlvStruct->padBytes = ALIGNED_WORD_SIZE - 
                              (tlvStruct->length & (ALIGNED_WORD_SIZE - 1));

   tlvStruct = (tHalCfg *)( (tANI_U8 *) tlvStruct 
                 + sizeof(tHalCfg) + tlvStruct->length + tlvStruct->padBytes) ; 

   wdiStartParams->usConfigBufferLen = (tANI_U8 *)tlvStruct - tlvStructStart ;

#ifdef WLAN_DEBUG
   {
      int i;

       VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                    "****** Dumping CFG TLV ***** ");
      for (i=0; (i+7) < wdiStartParams->usConfigBufferLen; i+=8)
      {
         VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                    "%02x %02x %02x %02x %02x %02x %02x %02x", 
                    tlvStructStart[i],
                    tlvStructStart[i+1],
                    tlvStructStart[i+2],
                    tlvStructStart[i+3],
                    tlvStructStart[i+4],
                    tlvStructStart[i+5],
                    tlvStructStart[i+6],
                    tlvStructStart[i+7]);
      }
      /* Dump the bytes in the last line*/
      for (; i < wdiStartParams->usConfigBufferLen; i++)
      {
         VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                    "%02x ",tlvStructStart[i]);
      }
      VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                    "**************************** ");
   }
#endif

   return VOS_STATUS_SUCCESS ;

handle_failure:
   vos_mem_free(configParam);
   return VOS_STATUS_E_FAILURE;
}

/*
 * FUNCTION: WDA_wdiCompleteCB
 * call the voss call back function
 */ 
void WDA_stopCallback(WDI_Status status, v_PVOID_t *pVosContext)
{
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);

   /* free the config structure */
   if(wdaContext->wdaWdiApiMsgParam != NULL)
   {
      vos_mem_free(wdaContext->wdaWdiApiMsgParam);
      wdaContext->wdaWdiApiMsgParam = NULL;
      wdaContext->wdaConfigParam = NULL;
   }

   if(WDI_STATUS_SUCCESS != status)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                               "WDI stop callback returned failure" );
      WDA_VOS_ASSERT(0) ;
   }
   else
   {
      wdaContext->wdaState = WDA_STOP_STATE;
   }

   /* Indicate VOSS about the start complete */
   vos_WDAComplete_cback(pVosContext);

   return ;
}

/*
 * FUNCTION: WDA_stop
 * call WDI_stop
 */ 

VOS_STATUS WDA_stop(v_PVOID_t pVosContext, tANI_U8 reason)
{
   WDI_Status status = WDI_STATUS_SUCCESS;

   WDI_StopReqParamsType *wdiStopReq = (WDI_StopReqParamsType *)
                            vos_mem_malloc(sizeof(WDI_StopReqParamsType)) ;
   tWDA_CbContext *pWDA = (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);

   if((WDA_READY_STATE != pWDA->wdaState) && 
                                 (WDA_INIT_STATE != pWDA->wdaState))
   {
      WDA_VOS_ASSERT(0);
   }
   
   WDA_VOS_ALLOC_FAIL(wdiStopReq) ;

   wdiStopReq->wdiStopReason = reason ;

   WDA_VOS_ASSERT(NULL == pWDA->wdaWdiApiMsgParam);

   pWDA->wdaWdiApiMsgParam = (v_PVOID_t *)wdiStopReq ;

   /* call WDI stop */
   status = WDI_Stop(wdiStopReq, 
                           (WDI_StopRspCb)WDA_stopCallback, pVosContext);

   if (WDI_STATUS_SUCCESS != status )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "error in WDA Stop" );
      vos_mem_free(pWDA->wdaWdiApiMsgParam);
      pWDA->wdaWdiApiMsgParam = NULL;
      status = VOS_STATUS_E_FAILURE;
   }

   return status;
}

/*
 * FUNCTION: WDA_close
 * call WDI_close and free the WDA context
 */ 
VOS_STATUS WDA_close(v_PVOID_t pVosContext)
{
   WDI_Status status = WDI_STATUS_SUCCESS;
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);

   if((WDA_INIT_STATE != wdaContext->wdaState) && 
                              (WDA_STOP_STATE != wdaContext->wdaState))
   {
      WDA_VOS_ASSERT(0);
   }

   /*call WDI close*/
   status = WDI_Close();
   if ( status != WDI_STATUS_SUCCESS )
   {
      status = VOS_STATUS_E_FAILURE;
   }

   wdaContext->wdaState = WDA_CLOSE_STATE;

   /* Destroy the event */
   status = vos_event_destroy(&wdaContext->txFrameEvent);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                  "VOS Event init failed - status = %d\n", status);
      status = VOS_STATUS_E_FAILURE;
   }

   /* free WDA context */
   status = vos_free_context(pVosContext,VOS_MODULE_ID_WDA,wdaContext);
   if ( !VOS_IS_STATUS_SUCCESS(status) )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "error in WDA close " );
      status = VOS_STATUS_E_FAILURE;
   }
   return status;
}

/*
 * FUNCTION: WDA_WniCfgDnld
 * Trigger CFG Donwload
 */ 
VOS_STATUS WDA_WniCfgDnld(tWDA_CbContext *pWDA) 
{
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pWDA->pVosContext);

   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;

   v_VOID_t *pFileImage = NULL;
   v_SIZE_t cbFileImageSize = 0;

   v_VOID_t *pCfgBinary = NULL;
   v_SIZE_t cbCfgBinarySize = 0;
   
   v_BOOL_t bStatus = VOS_FALSE;

   /* get the number of bytes in the CFG Binary... */
   vosStatus = vos_get_binary_blob( VOS_BINARY_ID_CONFIG, NULL, 
                                                &cbFileImageSize );
   if ( VOS_STATUS_E_NOMEM != vosStatus )
   {
      VOS_TRACE( VOS_MODULE_ID_SYS, VOS_TRACE_LEVEL_ERROR,
                 "Error obtaining binary size" );
      goto fail;
   }

   // malloc a buffer to read in the Configuration binary file.
   pFileImage = vos_mem_malloc( cbFileImageSize );

   if ( NULL == pFileImage )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Unable to allocate memory for the CFG binary [size= %d bytes]",
                 cbFileImageSize );

      vosStatus = VOS_STATUS_E_NOMEM;
      goto fail;
   }
   
   /* Get the entire CFG file image... */
   vosStatus = vos_get_binary_blob( VOS_BINARY_ID_CONFIG, pFileImage, 
                                                         &cbFileImageSize );
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
         "Error: Cannot retrieve CFG fine image from vOSS. [size= %d bytes]",
                                                             cbFileImageSize );
      goto fail;
   }
   
   /* 
    * Validate the binary image.  This function will return a pointer 
    * and lengthwhere the CFG binary is located within the binary image file.
    */
   bStatus = sys_validateStaConfig( pFileImage, cbFileImageSize,
                                   &pCfgBinary, &cbCfgBinarySize );
   if ( VOS_FALSE == bStatus )
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 "Error: Cannot find STA CFG in binary image fileze" );
      vosStatus = VOS_STATUS_E_FAILURE;
      goto fail;
   }

   /*
    * TODO: call the config download function 
    * for now calling the existing cfg download API 
    */
   processCfgDownloadReq(pMac,cbCfgBinarySize,pCfgBinary);

   return vosStatus;
   
fail:
   if(pCfgBinary != NULL)
      vos_mem_free( pFileImage );
   
   return vosStatus;
}

/* -----------------------------------------------------------------
 * WDI interface 
 * -----------------------------------------------------------------
 */

/*
 * FUNCTION: WDA_suspendDataTxCallback
 * call back function called from TL after suspend Transmission
 */ 
VOS_STATUS WDA_SuspendDataTxCallback( v_PVOID_t      pvosGCtx,
                                            v_U8_t*        ucSTAId,
                                            VOS_STATUS     vosStatus)
{
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                      "%s: Entered " ,__FUNCTION__);

   /* TODO: Do we really need this */
   return VOS_STATUS_SUCCESS;
}

/*
 * FUNCTION: WDA_suspendDataTx
 * Update TL to suspend the data Transmission
 */ 
VOS_STATUS WDA_SuspendDataTx(tWDA_CbContext *pWDA)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                      "%s: Entered " ,__FUNCTION__);

   /* TODO: Do we really need the serialization mechanism in PRIMA*/
   status = WLANTL_SuspendDataTx(pWDA->pVosContext, NULL, WDA_SuspendDataTxCallback);
   return status;
}

/*
 * FUNCTION: WDA_resumeDataTx
 * Update TL to resume the data Transmission
 */ 
VOS_STATUS WDA_ResumeDataTx(tWDA_CbContext *pWDA)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;

   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                      "%s: Entered " ,__FUNCTION__);

   status = WLANTL_ResumeDataTx(pWDA->pVosContext, NULL);
   return status;
}

/*
 * FUNCTION: WDA_InitScanReqCallback
 * Trigger Init SCAN callback
 */ 
void WDA_InitScanReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tInitScanParams *pWDA_ScanParam = (tInitScanParams *)pWDA->wdaScanMsgParam ;

   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                          "<------ %s " ,__FUNCTION__);

   WDA_VOS_ASSERT(NULL != pWDA_ScanParam);

   if(WDI_STATUS_SUCCESS != status)
   {
      status = WDA_ResumeDataTx(pWDA) ;

      if(WDI_STATUS_SUCCESS != status)
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "%s error in Resume Tx ", __FUNCTION__ );
         WDA_VOS_ASSERT(0) ;
      }

   }
   /* free WDI command buffer */
   vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;

   /* set stored memory pointers to NULL */
   pWDA->wdaWdiScanApiMsgParam = NULL;
   pWDA->wdaScanMsgParam = NULL;

   
   /* assign status to scan params */
   pWDA_ScanParam->status = CONVERT_WDI2SIR_STATUS(status) ;

   /* send SCAN RSP message back to PE */
   WDA_SendMsg(pWDA, WDA_INIT_SCAN_RSP, (void *)pWDA_ScanParam, 0) ;

   return ;
}
  
/*
 * FUNCTION: WDA_ProcessInitScanReq
 * Trigger Init SCAN in DAL
 */ 
VOS_STATUS  WDA_ProcessInitScanReq(tWDA_CbContext *pWDA, 
                                           tInitScanParams *initScanParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_InitScanReqParamsType *wdiInitScanParam = 
                  (WDI_InitScanReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_InitScanReqParamsType)) ;

   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                          "------> %s " ,__FUNCTION__);

   WDA_VOS_ALLOC_FAIL(wdiInitScanParam) ; 

   /* Copy init Scan params to WDI structure */
   wdiInitScanParam->wdiReqInfo.wdiScanMode = initScanParams->scanMode ;
   vos_mem_copy(wdiInitScanParam->wdiReqInfo.macBSSID, initScanParams->bssid,
                                             sizeof(tSirMacAddr)) ;
   wdiInitScanParam->wdiReqInfo.bNotifyBSS = initScanParams->notifyBss ;
   wdiInitScanParam->wdiReqInfo.ucFrameType = initScanParams->frameType ;
   wdiInitScanParam->wdiReqInfo.ucFrameLength = initScanParams->frameLength ;
 
   /* if Frame length, copy macMgmtHdr ro WDI structure */ 
   if(0 != wdiInitScanParam->wdiReqInfo.ucFrameLength)
   {
      vos_mem_copy(&wdiInitScanParam->wdiReqInfo.wdiMACMgmtHdr, 
                       &initScanParams->macMgmtHdr, sizeof(tSirMacMgmtHdr)) ;
   } 

   WDA_VOS_ASSERT((NULL == pWDA->wdaWdiScanApiMsgParam) && 
                                            (NULL == pWDA->wdaScanMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaWdiScanApiMsgParam = 
                           (WDI_InitScanReqParamsType *)wdiInitScanParam ;
   pWDA->wdaScanMsgParam = initScanParams;

   /* first try to suspend TX */
   status = WDA_SuspendDataTx(pWDA) ;

   if(WDI_STATUS_SUCCESS != status)
   {
      goto handleWdiFailure;
   }

   /* call DAL API to pass init scan request to DAL */
   status = WDI_InitScanReq(wdiInitScanParam, 
                        WDA_InitScanReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                               "error in WDA Init Scan, Resume Tx " );
      status = WDA_ResumeDataTx(pWDA) ;

      WDA_VOS_ASSERT(0) ;
  
      goto handleWdiFailure;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;

handleWdiFailure:
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                      "Failure in WDI Api, free all the memory " );
   /* free WDI command buffer */
   vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;
   pWDA->wdaWdiScanApiMsgParam = NULL;
   pWDA->wdaScanMsgParam = NULL;

   /* send Failure to PE */
   initScanParams->status = eSIR_FAILURE ;
   WDA_SendMsg(pWDA, WDA_INIT_SCAN_RSP, (void *)initScanParams, 0) ;

   return CONVERT_WDI2VOS_STATUS(status) ;
}


/*
 * FUNCTION: WDA_StartScanReqCallback
 * send Start SCAN RSP back to PE
 */ 
void WDA_StartScanReqCallback(WDI_StartScanRspParamsType *pScanRsp, 
                                                    void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tStartScanParams *pWDA_ScanParam = 
                           (tStartScanParams *)pWDA->wdaScanMsgParam ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                          "<------ %s " ,__FUNCTION__);

   WDA_VOS_ASSERT(NULL != pWDA_ScanParam);

   WDA_VOS_ASSERT(NULL != pWDA->wdaWdiScanApiMsgParam) ;

   vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;

   pWDA->wdaWdiScanApiMsgParam = NULL;
   pWDA->wdaScanMsgParam = NULL;
   
   /* assign status to scan params */
   pWDA_ScanParam->status = CONVERT_WDI2SIR_STATUS(pScanRsp->wdiStatus) ;

   /* send SCAN RSP message back to PE */
   WDA_SendMsg(pWDA, WDA_START_SCAN_RSP, (void *)pWDA_ScanParam, 0) ;

   return ;
}



/*
 * FUNCTION: WDA_ProcessStartScanReq
 * Trigger start SCAN in DAL
 */ 
VOS_STATUS  WDA_ProcessStartScanReq(tWDA_CbContext *pWDA, 
                                           tStartScanParams *startScanParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS;
   WDI_StartScanReqParamsType *wdiStartScanParams = 
                            (WDI_StartScanReqParamsType *)vos_mem_malloc(
                                          sizeof(WDI_StartScanReqParamsType)) ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                          "------> %s " ,__FUNCTION__);

   WDA_VOS_ALLOC_FAIL(wdiStartScanParams) ;

   /* Copy init Scan params to WDI structure */
   wdiStartScanParams->ucChannel = startScanParams->scanChannel ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaWdiScanApiMsgParam) && 
                                           (NULL == pWDA->wdaScanMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaScanMsgParam = (void *)startScanParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiScanApiMsgParam = (void *)wdiStartScanParams ;

   /* call DAL API to pass init scan request to DAL */
   status = WDI_StartScanReq(wdiStartScanParams, 
                              WDA_StartScanReqCallback, pWDA) ;

   /* failure returned by WDI API */
   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Failure in Start Scan WDI API, free all the memory "
                     "It should be due to previous abort scan." );
      vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;
      pWDA->wdaWdiScanApiMsgParam = NULL;
      pWDA->wdaScanMsgParam = NULL;
      startScanParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_START_SCAN_RSP, (void *)startScanParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_InitScanReqCallback
 * Trigger Init SCAN callback
 */ 
void WDA_EndScanReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tEndScanParams *endScanParam = (tEndScanParams *)pWDA->wdaScanMsgParam ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                             "<------ %s " ,__FUNCTION__);

   WDA_VOS_ASSERT(NULL != endScanParam);

   /* Free WDI command buffer */
   vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;
   pWDA->wdaWdiScanApiMsgParam = NULL;
   pWDA->wdaScanMsgParam = NULL;

   /* assign status to scan params */
   endScanParam->status = CONVERT_WDI2SIR_STATUS(status) ;

   /* send response back to PE */
   WDA_SendMsg(pWDA, WDA_END_SCAN_RSP, (void *)endScanParam, 0) ;
   return ;
}


/*
 * FUNCTION: WDA_ProcessStartScanReq
 * Trigger Init SCAN in DAL
 */ 
VOS_STATUS  WDA_ProcessEndScanReq(tWDA_CbContext *pWDA, 
                                           tEndScanParams *endScanParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS;
   WDI_EndScanReqParamsType *wdiEndScanParams =
                            (WDI_EndScanReqParamsType *)vos_mem_malloc(
                                          sizeof(WDI_EndScanReqParamsType)) ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                             "------> %s " ,__FUNCTION__);

   WDA_VOS_ALLOC_FAIL(wdiEndScanParams) ;

   /* Copy init Scan params to WDI structure */
   wdiEndScanParams->ucChannel = endScanParams->scanChannel ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaWdiScanApiMsgParam) && 
                                             (NULL == pWDA->wdaScanMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaScanMsgParam = (void *)endScanParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiScanApiMsgParam = (void *)wdiEndScanParams ;

   /* call DAL API to pass init scan request to DAL */
   status = WDI_EndScanReq(wdiEndScanParams, 
                              WDA_EndScanReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Failure in End Scan WDI API, free all the memory "
                     "It should be due to previous abort scan." );
      vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ;
      pWDA->wdaWdiScanApiMsgParam = NULL;
      pWDA->wdaScanMsgParam = NULL;
      endScanParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_END_SCAN_RSP, (void *)endScanParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_InitScanReqCallback
 * Trigger Init SCAN callback
 */ 
void WDA_FinishScanReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tFinishScanParams *finishScanParam = 
                                   (tFinishScanParams *)pWDA->wdaScanMsgParam ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                          "<------ %s " ,__FUNCTION__);

   WDA_VOS_ASSERT(NULL != finishScanParam);

   vos_mem_free(pWDA->wdaWdiScanApiMsgParam) ; 
   pWDA->wdaWdiScanApiMsgParam = NULL;
   pWDA->wdaScanMsgParam = NULL;

   /* 
    * Now Resume TX, if we reached here means, TX is already suspended, we 
    * have to resume it unconditionaly
    */
   status = WDA_ResumeDataTx(pWDA) ;
       
   if(WDI_STATUS_SUCCESS != status)
   {
      WDA_VOS_ASSERT(0) ;
   }

   finishScanParam->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_FINISH_SCAN_RSP, (void *)finishScanParam, 0) ;
   return ;
}

/*
 * FUNCTION: WDA_ProcessFinshScanReq
 * Trigger Finish SCAN in WDI
 */ 
VOS_STATUS  WDA_ProcessFinishScanReq(tWDA_CbContext *pWDA, 
                                           tFinishScanParams *finishScanParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS;
   WDI_FinishScanReqParamsType *wdiFinishScanParams =
                            (WDI_FinishScanReqParamsType *)vos_mem_malloc(
                                        sizeof(WDI_FinishScanReqParamsType)) ;
   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_INFO,
                                             "------> %s " ,__FUNCTION__);

   WDA_VOS_ALLOC_FAIL(wdiFinishScanParams) ;

   /* Copy init Scan params to WDI structure */
   wdiFinishScanParams->wdiReqInfo.wdiScanMode = finishScanParams->scanMode ;
   vos_mem_copy(wdiFinishScanParams->wdiReqInfo.macBSSID, 
                              finishScanParams->bssid, sizeof(tSirMacAddr)) ;

   wdiFinishScanParams->wdiReqInfo.bNotifyBSS = finishScanParams->notifyBss ;
   wdiFinishScanParams->wdiReqInfo.ucFrameType = finishScanParams->frameType ;
   wdiFinishScanParams->wdiReqInfo.ucFrameLength = 
                                                finishScanParams->frameLength ;
   wdiFinishScanParams->wdiReqInfo.ucCurrentOperatingChannel = 
                                         finishScanParams->currentOperChannel ;
   wdiFinishScanParams->wdiReqInfo.wdiCBState = finishScanParams->cbState ;

   /* if Frame length, copy macMgmtHdr ro WDI structure */ 
   if(0 != wdiFinishScanParams->wdiReqInfo.ucFrameLength)
   {
      vos_mem_copy(&wdiFinishScanParams->wdiReqInfo.wdiMACMgmtHdr, 
                                          &finishScanParams->macMgmtHdr, 
                                                     sizeof(WDI_MacMgmtHdr)) ;
   } 

   WDA_VOS_ASSERT((NULL == pWDA->wdaWdiScanApiMsgParam) && 
                                             (NULL == pWDA->wdaScanMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaScanMsgParam = (void *)finishScanParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiScanApiMsgParam = (void *)wdiFinishScanParams ;

   /* call DAL API to pass init scan request to DAL */
   status = WDI_FinishScanReq(wdiFinishScanParams, 
                              WDA_FinishScanReqCallback, pWDA) ;

   
   /* 
    * WDI API returns failure..
    */
   if(IS_WDI_STATUS_FAILURE( status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Failure in Finish Scan WDI API, free all the memory " );
      vos_mem_free(((WDI_FinishScanReqParamsType *)
                                      pWDA->wdaWdiScanApiMsgParam)) ;
      pWDA->wdaWdiScanApiMsgParam = NULL;
      pWDA->wdaScanMsgParam = NULL;

      finishScanParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_FINISH_SCAN_RSP, (void *)finishScanParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*---------------------------------------------------------------------
 * ASSOC API's
 *---------------------------------------------------------------------
 */

/*
 * FUNCTION: WDA_JoinReqCallback
 * Trigger Init SCAN callback
 */ 
void WDA_JoinReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tSwitchChannelParams *joinReqParam = 
                           (tSwitchChannelParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   /* reset macBSSID */
   vos_mem_set(pWDA->macBSSID, sizeof(pWDA->macBSSID),0 );

   joinReqParam->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_SWITCH_CHANNEL_RSP, (void *)joinReqParam , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessJoinReq
 * Trigger Join REQ in WDI
 */ 

VOS_STATUS WDA_ProcessJoinReq(tWDA_CbContext *pWDA, 
                                            tSwitchChannelParams* joinReqParam)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_JoinReqParamsType *wdiJoinReqParam = 
                             (WDI_JoinReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_JoinReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiJoinReqParam) ;

   /* copy the BSSID for pWDA */
   vos_mem_copy(wdiJoinReqParam->wdiReqInfo.macBSSID, pWDA->macBSSID, 
                                             sizeof(tSirMacAddr)) ;
   wdiJoinReqParam->wdiReqInfo.wdiChannelInfo.ucChannel = 
                                                 joinReqParam->channelNumber ;
#ifndef WLAN_FEATURE_VOWIFI 
   wdiJoinReqParam->wdiReqInfo.wdiChannelInfo.ucLocalPowerConstraint = 
                                          joinReqParam->localPowerConstraint ;
#endif
   wdiJoinReqParam->wdiReqInfo.wdiChannelInfo.wdiSecondaryChannelOffset = 
                                        joinReqParam->secondaryChannelOffset ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));
   
   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)joinReqParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiJoinReqParam ;

   status = WDI_JoinReq(wdiJoinReqParam, 
                               (WDI_JoinRspCb )WDA_JoinReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Join WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      joinReqParam->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_SWITCH_CHANNEL_RSP, (void *)joinReqParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_SwitchChannelReqCallback
 * send Switch channel RSP back to PE
 */ 
void WDA_SwitchChannelReqCallback(
               WDI_SwitchCHRspParamsType   *wdiSwitchChanRsp, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tSwitchChannelParams *pSwitchChanParams = 
                                    (tSwitchChannelParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   pSwitchChanParams->status = 
                          CONVERT_WDI2SIR_STATUS(wdiSwitchChanRsp->wdiStatus) ;

   WDA_SendMsg(pWDA, WDA_SWITCH_CHANNEL_RSP, (void *)pSwitchChanParams , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessChannelSwitchReq
 * Request to WDI to switch channel REQ params.
 */ 
VOS_STATUS WDA_ProcessChannelSwitchReq(tWDA_CbContext *pWDA, 
                                       tSwitchChannelParams *pSwitchChanParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_SwitchChReqParamsType *wdiSwitchChanParam = 
                         (WDI_SwitchChReqParamsType *)vos_mem_malloc(
                                         sizeof(WDI_SwitchChReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiSwitchChanParam);

   wdiSwitchChanParam->wdiChInfo.ucChannel = pSwitchChanParams->channelNumber;
#ifndef WLAN_FEATURE_VOWIFI 
   wdiSwitchChanParam->wdiChInfo.ucLocalPowerConstraint = 
                                       pSwitchChanParams->localPowerConstraint;
#endif
   wdiSwitchChanParam->wdiChInfo.wdiSecondaryChannelOffset = 
                                     pSwitchChanParams->secondaryChannelOffset;

   /* Store req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)pSwitchChanParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiSwitchChanParam ;

   status = WDI_SwitchChReq(wdiSwitchChanParam, 
                     (WDI_SwitchChRspCb)WDA_SwitchChannelReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
       "Failure in process channel switch Req WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      pSwitchChanParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_SWITCH_CHANNEL_RSP, (void *)pSwitchChanParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_ConfigBssReqCallback
 * config BSS Req Callback, called by WDI
 */ 
void WDA_ConfigBssReqCallback(WDI_ConfigBSSRspParamsType *wdiConfigBssRsp  
                                                          ,void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tAddBssParams *configBssReqParam = 
                             (tAddBssParams *)pWDA->wdaMsgParam ;
   tAddStaParams *staConfigBssParam = &configBssReqParam->staContext ;

   configBssReqParam->status = 
                     CONVERT_WDI2SIR_STATUS(wdiConfigBssRsp->wdiStatus);

   if(WDI_STATUS_SUCCESS == wdiConfigBssRsp->wdiStatus)
   {
      vos_mem_copy(configBssReqParam->bssId, wdiConfigBssRsp->macBSSID, 
                                                    sizeof(tSirMacAddr));
      if (configBssReqParam->operMode == BSS_OPERATIONAL_MODE_STA) 
      {
         if(configBssReqParam->bssType == eSIR_IBSS_MODE) 
         {
             staConfigBssParam->staType = STA_ENTRY_BSSID;
         }
         else if ((configBssReqParam->bssType == eSIR_BTAMP_STA_MODE) &&
                 (staConfigBssParam->staType == STA_ENTRY_SELF))
         {
             /* This is the 1st add BSS Req for the BTAMP STA */
             staConfigBssParam->staType = STA_ENTRY_BSSID;
         }
         else if ((configBssReqParam->bssType == eSIR_BTAMP_AP_MODE) &&
                      (staConfigBssParam->staType == STA_ENTRY_SELF))
         {
            // statype is already set by PE.
            // Only 1 ADD BSS Req is sent on the BTAMP AP side.
            staConfigBssParam->staType = STA_ENTRY_BSSID;
         } 
         else 
         {
            staConfigBssParam->staType = STA_ENTRY_PEER;
         }
      }
      else if (configBssReqParam->operMode == BSS_OPERATIONAL_MODE_AP) 
      {
          staConfigBssParam->staType = STA_ENTRY_SELF;
      }

      staConfigBssParam->staIdx = wdiConfigBssRsp->usSTAIdx;

      staConfigBssParam->bssIdx = wdiConfigBssRsp->usBSSIdx;

      staConfigBssParam->ucUcastSig = wdiConfigBssRsp->ucUcastSig;

      staConfigBssParam->ucBcastSig = wdiConfigBssRsp->ucBcastSig;

      vos_mem_copy(staConfigBssParam->staMac, wdiConfigBssRsp->macBSSID, 
                                                    sizeof(tSirMacAddr));
      staConfigBssParam->txChannelWidthSet = 
                               configBssReqParam->txChannelWidthSet;
   }

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   WDA_SendMsg(pWDA, WDA_ADD_BSS_RSP, (void *)configBssReqParam , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_UpdateEdcaParamsForAC
 * Update WDI EDCA params with PE edca params
 */ 
void WDA_UpdateEdcaParamsForAC(tWDA_CbContext *pWDA, 
                         WDI_EdcaParamRecord *wdiEdcaParam, 
                             tSirMacEdcaParamRecord *macEdcaParam)
{
   wdiEdcaParam->wdiACI.aifsn = macEdcaParam->aci.aifsn;
   wdiEdcaParam->wdiACI.acm= macEdcaParam->aci.acm;
   wdiEdcaParam->wdiACI.aci = macEdcaParam->aci.aci;
   wdiEdcaParam->wdiCW.min = macEdcaParam->cw.min;
   wdiEdcaParam->wdiCW.max = macEdcaParam->cw.max;
   wdiEdcaParam->usTXOPLimit = macEdcaParam->txoplimit;
}

/*
 * FUNCTION: WDA_ProcessConfigBssReq
 * Configure BSS before starting Assoc with AP
 */ 
VOS_STATUS WDA_ProcessConfigBssReq(tWDA_CbContext *pWDA, 
                                         tAddBssParams* configBssReqParam)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_ConfigBSSReqParamsType *wdiConfigBssReqParam =
                             (WDI_ConfigBSSReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_ConfigBSSReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiConfigBssReqParam) ;

   WDA_UpdateBSSParams(pWDA, &wdiConfigBssReqParam->wdiReqInfo, 
                       configBssReqParam) ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)configBssReqParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiConfigBssReqParam ;

   status = WDI_ConfigBSSReq(wdiConfigBssReqParam, 
                        (WDI_ConfigBSSRspCb )WDA_ConfigBssReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Config BSS WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;

      configBssReqParam->status = eSIR_FAILURE ;

      WDA_SendMsg(pWDA, WDA_ADD_BSS_RSP, (void *)configBssReqParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

#ifdef ENABLE_HAL_COMBINED_MESSAGES
/*
 * FUNCTION: WDA_PostAssocReqCallback
 * Post ASSOC req callback, send RSP back to PE
 */ 
void WDA_PostAssocReqCallback(WDI_PostAssocRspParamsType *wdiPostAssocRsp,  
                                                          void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tPostAssocParams *postAssocReqParam = 
                             (tPostAssocParams *)pWDA->wdaMsgParam ;
   /*STA context within the BSS Params*/
   tAddStaParams *staPostAssocParam = 
      &postAssocReqParam->addBssParams.staContext ;
   /*STA Params for self STA*/
   tAddStaParams *selfStaPostAssocParam = 
      &postAssocReqParam->addStaParams ;

   postAssocReqParam->status = 
                   CONVERT_WDI2SIR_STATUS(wdiPostAssocRsp->wdiStatus) ;

   if(WDI_STATUS_SUCCESS == wdiPostAssocRsp->wdiStatus)
   {
      staPostAssocParam->staIdx = wdiPostAssocRsp->bssParams.usSTAIdx ;
      vos_mem_copy(staPostAssocParam->staMac, wdiPostAssocRsp->bssParams.macSTA, 
                   sizeof(tSirMacAddr)) ;
      staPostAssocParam->ucUcastSig = wdiPostAssocRsp->bssParams.ucUcastSig ;
      staPostAssocParam->ucBcastSig = wdiPostAssocRsp->bssParams.ucBcastSig ;
      staPostAssocParam->bssIdx = wdiPostAssocRsp->bssParams.usBSSIdx;

      selfStaPostAssocParam->staIdx = wdiPostAssocRsp->staParams.usSTAId;
   }

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   WDA_SendMsg(pWDA, WDA_POST_ASSOC_RSP, (void *)postAssocReqParam, 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessPostAssocReq
 * Trigger POST ASSOC processing in WDI
 */ 
VOS_STATUS WDA_ProcessPostAssocReq(tWDA_CbContext *pWDA, 
                                    tPostAssocParams *postAssocReqParam)
{

   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_PostAssocReqParamsType *wdiPostAssocReqParam =
                           (WDI_PostAssocReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_PostAssocReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiPostAssocReqParam) ;

   /* update BSS params into WDI structure */
   WDA_UpdateBSSParams(pWDA, &wdiPostAssocReqParam->wdiBSSParams, 
                       &postAssocReqParam->addBssParams) ;
   /* update STA params into WDI structure */
   WDA_UpdateSTAParams(pWDA, &wdiPostAssocReqParam->wdiSTAParams, 
                       &postAssocReqParam->addStaParams) ;
   
   wdiPostAssocReqParam->wdiBSSParams.ucEDCAParamsValid = 
                           postAssocReqParam->addBssParams.highPerformance;
   WDA_UpdateEdcaParamsForAC(pWDA, 
                        &wdiPostAssocReqParam->wdiBSSParams.wdiBEEDCAParams,
                        &postAssocReqParam->addBssParams.acbe);
   WDA_UpdateEdcaParamsForAC(pWDA, 
                        &wdiPostAssocReqParam->wdiBSSParams.wdiBKEDCAParams,
                        &postAssocReqParam->addBssParams.acbk);
   WDA_UpdateEdcaParamsForAC(pWDA, 
                        &wdiPostAssocReqParam->wdiBSSParams.wdiVIEDCAParams,
                        &postAssocReqParam->addBssParams.acvi);
   WDA_UpdateEdcaParamsForAC(pWDA, 
                        &wdiPostAssocReqParam->wdiBSSParams.wdiVOEDCAParams,
                        &postAssocReqParam->addBssParams.acvo);

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)postAssocReqParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiPostAssocReqParam ;

   status = WDI_PostAssocReq(wdiPostAssocReqParam, 
                        (WDI_PostAssocRspCb )WDA_PostAssocReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Post Assoc WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;

      postAssocReqParam->status = eSIR_FAILURE ;

      WDA_SendMsg(pWDA, WDA_POST_ASSOC_RSP, (void *)postAssocReqParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}
#endif

/*
 * FUNCTION: WDA_AddStaReqCallback
 * ADD STA req callback, send RSP back to PE
 */ 
void WDA_AddStaReqCallback(WDI_ConfigSTARspParamsType *wdiConfigStaRsp,  
                                                          void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tAddStaParams *addStaReqParam = 
                             (tAddStaParams *)pWDA->wdaMsgParam ;

   addStaReqParam->status = 
                   CONVERT_WDI2SIR_STATUS(wdiConfigStaRsp->wdiStatus) ;

   if(WDI_STATUS_SUCCESS == wdiConfigStaRsp->wdiStatus)
   {
      addStaReqParam->staIdx = wdiConfigStaRsp->usSTAId;
      /*TODO: UMAC structure doesn't have these fields*/
      /*addStaReqParam-> = wdiConfigStaRsp->ucDpuIndex;
      addStaReqParam-> = wdiConfigStaRsp->ucBcastDpuIndex;
      addStaReqParam-> = wdiConfigStaRsp->ucBcastMgmtDpuIdx; */
      addStaReqParam->ucUcastSig = wdiConfigStaRsp->ucUcastSig;
      addStaReqParam->ucBcastSig = wdiConfigStaRsp->ucBcastSig;
   }

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   WDA_SendMsg(pWDA, WDA_ADD_STA_RSP, (void *)addStaReqParam, 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ConfigStaReq
 * Trigger Config STA processing in WDI
 */ 
VOS_STATUS WDA_ProcessAddStaReq(tWDA_CbContext *pWDA, 
                                    tAddStaParams *addStaReqParam)
{

   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_ConfigSTAReqParamsType *wdiConfigStaReqParam =
                           (WDI_ConfigSTAReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_ConfigSTAReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiConfigStaReqParam) ;

   /* update STA params into WDI structure */
   WDA_UpdateSTAParams(pWDA, &wdiConfigStaReqParam->wdiReqInfo, 
                       addStaReqParam) ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)addStaReqParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiConfigStaReqParam ;

   status = WDI_ConfigSTAReq(wdiConfigStaReqParam, 
                        (WDI_ConfigSTARspCb )WDA_AddStaReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Config STA WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;

      addStaReqParam->status = eSIR_FAILURE ;

      WDA_SendMsg(pWDA, WDA_ADD_STA_RSP, (void *)addStaReqParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_DelBSSReqCallback
 * Dens DEL BSS RSP back to PE
 */ 
void WDA_DelBSSReqCallback(WDI_DelBSSRspParamsType *wdiDelBssRsp, 
                                                         void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tDeleteBssParams *delBssReqParam = (tDeleteBssParams *)pWDA->wdaMsgParam ;

   delBssReqParam->status = CONVERT_WDI2SIR_STATUS(wdiDelBssRsp->wdiStatus) ;

   if(WDI_STATUS_SUCCESS == wdiDelBssRsp->wdiStatus)
   {
      vos_mem_copy(delBssReqParam->bssid, wdiDelBssRsp->macBSSID, 
                                             sizeof(tSirMacAddr)) ;
   }
   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   WDA_SendMsg(pWDA, WDA_DELETE_BSS_RSP, (void *)delBssReqParam , 0) ;

   return ;
}


/*
 * FUNCTION: WDA_ProcessDelBssReq
 * Init DEL BSS req with WDI
 */ 
VOS_STATUS WDA_ProcessDelBssReq(tWDA_CbContext *pWDA, 
                                        tDeleteBssParams *delBssParam)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_DelBSSReqParamsType *wdiDelBssReqParam = 
                             (WDI_DelBSSReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_DelBSSReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiDelBssReqParam) ;
   vos_mem_copy(wdiDelBssReqParam->macBSSID, delBssParam->bssid, 
                                             sizeof(tSirMacAddr)) ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)delBssParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiDelBssReqParam ;

   status = WDI_DelBSSReq(wdiDelBssReqParam, 
                        (WDI_DelBSSRspCb )WDA_DelBSSReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Del BSS WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      delBssParam->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_DELETE_BSS_RSP, (void *)delBssParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_DelSTAReqCallback
 * Dens DEL STA RSP back to PE
 */ 
void WDA_DelSTAReqCallback(WDI_DelSTARspParamsType *wdiDelStaRsp, 
                                                         void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tDeleteStaParams *delStaReqParam = (tDeleteStaParams *)pWDA->wdaMsgParam ;

   delStaReqParam->status = CONVERT_WDI2SIR_STATUS(wdiDelStaRsp->wdiStatus) ;

   if(WDI_STATUS_SUCCESS == wdiDelStaRsp->wdiStatus)
   {
      delStaReqParam->staIdx = wdiDelStaRsp->usSTAIdx ;
   }
   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   WDA_SendMsg(pWDA, WDA_DELETE_STA_RSP, (void *)delStaReqParam , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessDelStaReq
 * Init DEL STA req with WDI
 */ 
VOS_STATUS WDA_ProcessDelStaReq(tWDA_CbContext *pWDA, 
                                      tDeleteStaParams *delStaParam)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;

   WDI_DelSTAReqParamsType *wdiDelStaReqParam = 
                             (WDI_DelSTAReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_DelSTAReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiDelStaReqParam) ;

   wdiDelStaReqParam->usSTAIdx = delStaParam->staIdx ;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store Init Req pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)delStaParam ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiDelStaReqParam ;

   status = WDI_DelSTAReq(wdiDelStaReqParam, 
                        (WDI_DelSTARspCb )WDA_DelSTAReqCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "Failure in Del STA WDI API, free all the memory status = %d", 
                                                                status );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      delStaParam->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_DELETE_STA_RSP, (void *)delStaParam, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_SendMsg
 * Send Message back to PE
 */ 
void WDA_SendMsg(tWDA_CbContext *pWDA, tANI_U16 msgType, 
                                        void *pBodyptr, tANI_U32 bodyVal)
{
   tSirMsgQ msg = {0} ;
   tANI_U32 status = VOS_STATUS_SUCCESS ;
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pWDA->pVosContext);

   msg.type        = msgType;
   msg.bodyval     = bodyVal;
   msg.bodyptr     = pBodyptr;

   status = limPostMsgApi(pMac, &msg);

   if (VOS_STATUS_SUCCESS != status)
   {
      if(NULL != pBodyptr)
      {
         vos_mem_free(pBodyptr);
      }
      WDA_VOS_ASSERT(0) ;
   }

   return ;
}

/*
 * FUNCTION: WDA_UpdateBSSParams
 * Translated WDA/PE BSS info into WDI BSS info..
 */
void WDA_UpdateBSSParams(tWDA_CbContext *pWDA, 
                         WDI_ConfigBSSReqInfoType *wdiBssParams, 
                         tAddBssParams *wdaBssParams)
{
   /* copy bssReq Params to WDI structure */
   vos_mem_copy(wdiBssParams->macBSSID,
                           wdaBssParams->bssId, sizeof(tSirMacAddr)) ;
   vos_mem_copy(wdiBssParams->macSelfAddr, wdaBssParams->selfMacAddr,
                                                   sizeof(tSirMacAddr)) ;
   wdiBssParams->wdiBSSType = wdaBssParams->bssType ;
   wdiBssParams->ucOperMode = wdaBssParams->operMode ;
   wdiBssParams->wdiNWType   = wdaBssParams->nwType ;

   wdiBssParams->ucShortSlotTimeSupported = 
                                  wdaBssParams->shortSlotTimeSupported ;

   wdiBssParams->ucllaCoexist  = wdaBssParams->llaCoexist ;
   wdiBssParams->ucllbCoexist  = wdaBssParams->llbCoexist ;
   wdiBssParams->ucllgCoexist  = wdaBssParams->llgCoexist ;
   wdiBssParams->ucHT20Coexist = wdaBssParams->ht20Coexist ;

   wdiBssParams->ucllnNonGFCoexist = wdaBssParams->llnNonGFCoexist ;
   wdiBssParams->ucTXOPProtectionFullSupport =
                           wdaBssParams->fLsigTXOPProtectionFullSupport ;

   wdiBssParams->ucRIFSMode = wdaBssParams->fRIFSMode ;
   wdiBssParams->usBeaconInterval = wdaBssParams->beaconInterval ;

   wdiBssParams->ucDTIMPeriod = wdaBssParams->dtimPeriod ;

   wdiBssParams->ucTXChannelWidthSet = wdaBssParams->txChannelWidthSet ;
   wdiBssParams->ucCurrentOperChannel = wdaBssParams->currentOperChannel ;
   wdiBssParams->ucCurrentExtChannel = wdaBssParams->currentExtChannel ;

   /* copy SSID into WDI structure */
   wdiBssParams->wdiSSID.ucLength = wdaBssParams->ssId.length ;
   vos_mem_copy(wdiBssParams->wdiSSID.sSSID,
                 wdaBssParams->ssId.ssId, wdaBssParams->ssId.length) ;

   WDA_UpdateSTAParams(pWDA, &wdiBssParams->wdiSTAContext, 
                       &wdaBssParams->staContext) ;

   wdiBssParams->wdiAction = wdaBssParams->updateBss;

   return ;
}

/*
 * FUNCTION: WDA_UpdateSTAParams
 * Translated WDA/PE BSS info into WDI BSS info..
 */
void WDA_UpdateSTAParams(tWDA_CbContext *pWDA, 
                               WDI_ConfigStaReqInfoType *wdiStaParams, 
                                                tAddStaParams *wdaStaParams)
{
   /* Update STA params */
   vos_mem_copy(wdiStaParams->macBSSID, wdaStaParams->bssId, 
                                            sizeof(tSirMacAddr)) ;
   wdiStaParams->usAssocId = wdaStaParams->assocId;
   wdiStaParams->wdiSTAType = wdaStaParams->staType;
   
   wdiStaParams->ucShortPreambleSupported = 
                                        wdaStaParams->shortPreambleSupported;
   vos_mem_copy(wdiStaParams->macSTA, wdaStaParams->staMac, 
                                               sizeof(tSirMacAddr)) ;
   wdiStaParams->usListenInterval = wdaStaParams->listenInterval;
   
   wdiStaParams->ucWMMEnabled = wdaStaParams->wmmEnabled;
   
   wdiStaParams->ucHTCapable = wdaStaParams->htCapable;
   wdiStaParams->ucTXChannelWidthSet = wdaStaParams->txChannelWidthSet;
   wdiStaParams->ucRIFSMode = wdaStaParams->rifsMode;
   wdiStaParams->ucLSIGTxopProtection = wdaStaParams->lsigTxopProtection;
   wdiStaParams->ucMaxAmpduSize = wdaStaParams->maxAmpduSize;
   wdiStaParams->ucMaxAmpduDensity = wdaStaParams->maxAmpduDensity;
   wdiStaParams->ucMaxAmsduSize = wdaStaParams->maxAmsduSize;
   
   wdiStaParams->ucShortGI40Mhz = wdaStaParams->fShortGI40Mhz;
   wdiStaParams->ucShortGI20Mhz = wdaStaParams->fShortGI20Mhz;

   /* WDI_SupportedRates is not defined in WDI */
   //vos_mem_copy(wdiStaParams->wdiSupportedRates,
  //                   wdaStaParams->supportedRates,sizeof(WDI_SupportedRates)); //revisit
   
   wdiStaParams->ucRMFEnabled = wdaStaParams->rmfEnabled;
   
   wdiStaParams->wdiAction = wdaStaParams->updateSta; 
   
   wdiStaParams->ucAPSD = wdaStaParams->uAPSD;
   wdiStaParams->ucMaxSPLen = wdaStaParams->maxSPLen;
   wdiStaParams->ucGreenFieldCapable = wdaStaParams->greenFieldCapable;
   
   wdiStaParams->ucDelayedBASupport = wdaStaParams->delBASupport;
   wdiStaParams->us32MaxAmpduDuratio = wdaStaParams->us32MaxAmpduDuration;
   wdiStaParams->ucDsssCckMode40Mhz = wdaStaParams->fDsssCckMode40Mhz;
   return ;
}

/*
 * -------------------------------------------------------------------------
 * CFG update to WDI
 * ------------------------------------------------------------------------- 
 */
 
 /*
 * FUNCTION: WDA_ConvertWniCfgIdToHALCfgId
 * Convert the WNI CFG ID to HAL CFG ID
 */ 
static inline v_U8_t WDA_ConvertWniCfgIdToHALCfgId(v_U8_t wniCfgId)
{
   switch(wniCfgId)
   {
      case WNI_CFG_STA_ID:
         return QWLAN_HAL_CFG_STA_ID;
      case WNI_CFG_CURRENT_TX_ANTENNA:
         return QWLAN_HAL_CFG_CURRENT_TX_ANTENNA;
      case WNI_CFG_CURRENT_RX_ANTENNA:
         return QWLAN_HAL_CFG_CURRENT_RX_ANTENNA;
      case WNI_CFG_LOW_GAIN_OVERRIDE:
         return QWLAN_HAL_CFG_LOW_GAIN_OVERRIDE;
      case WNI_CFG_POWER_STATE_PER_CHAIN:
         return QWLAN_HAL_CFG_POWER_STATE_PER_CHAIN;
      case WNI_CFG_CAL_PERIOD:
         return QWLAN_HAL_CFG_CAL_PERIOD;
      case WNI_CFG_CAL_CONTROL:
         return QWLAN_HAL_CFG_CAL_CONTROL;
      case WNI_CFG_PROXIMITY:
         return QWLAN_HAL_CFG_PROXIMITY;
      case WNI_CFG_NETWORK_DENSITY:
         return QWLAN_HAL_CFG_NETWORK_DENSITY;
      case WNI_CFG_MAX_MEDIUM_TIME:
         return QWLAN_HAL_CFG_MAX_MEDIUM_TIME;
      case WNI_CFG_MAX_MPDUS_IN_AMPDU:
         return QWLAN_HAL_CFG_MAX_MPDUS_IN_AMPDU;
      case WNI_CFG_RTS_THRESHOLD:
         return QWLAN_HAL_CFG_RTS_THRESHOLD;
      case WNI_CFG_SHORT_RETRY_LIMIT:
         return QWLAN_HAL_CFG_SHORT_RETRY_LIMIT;
      case WNI_CFG_LONG_RETRY_LIMIT:
         return QWLAN_HAL_CFG_LONG_RETRY_LIMIT;
      case WNI_CFG_FRAGMENTATION_THRESHOLD:
         return QWLAN_HAL_CFG_FRAGMENTATION_THRESHOLD;
      case WNI_CFG_DYNAMIC_THRESHOLD_ZERO:
         return QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ZERO;
      case WNI_CFG_DYNAMIC_THRESHOLD_ONE:
         return QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_ONE;
      case WNI_CFG_DYNAMIC_THRESHOLD_TWO:
         return QWLAN_HAL_CFG_DYNAMIC_THRESHOLD_TWO;
      case WNI_CFG_FIXED_RATE:
         return QWLAN_HAL_CFG_FIXED_RATE;
      case WNI_CFG_RETRYRATE_POLICY:
         return QWLAN_HAL_CFG_RETRYRATE_POLICY;
      case WNI_CFG_RETRYRATE_SECONDARY:
         return QWLAN_HAL_CFG_RETRYRATE_SECONDARY;
      case WNI_CFG_RETRYRATE_TERTIARY:
         return QWLAN_HAL_CFG_RETRYRATE_TERTIARY;
      case WNI_CFG_FORCE_POLICY_PROTECTION:
         return QWLAN_HAL_CFG_FORCE_POLICY_PROTECTION;
      case WNI_CFG_FIXED_RATE_MULTICAST_24GHZ:
         return QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_24GHZ;
      case WNI_CFG_FIXED_RATE_MULTICAST_5GHZ:
         return QWLAN_HAL_CFG_FIXED_RATE_MULTICAST_5GHZ;
      case WNI_CFG_DEFAULT_RATE_INDEX_24GHZ:
         return QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_24GHZ;
      case WNI_CFG_DEFAULT_RATE_INDEX_5GHZ:
         return QWLAN_HAL_CFG_DEFAULT_RATE_INDEX_5GHZ;
      case WNI_CFG_MAX_BA_SESSIONS:
         return QWLAN_HAL_CFG_MAX_BA_SESSIONS;
      case WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT:
         return QWLAN_HAL_CFG_PS_DATA_INACTIVITY_TIMEOUT;
      case WNI_CFG_PS_ENABLE_BCN_FILTER:
         return QWLAN_HAL_CFG_PS_ENABLE_BCN_FILTER;
      case WNI_CFG_PS_ENABLE_RSSI_MONITOR:
         return QWLAN_HAL_CFG_PS_ENABLE_RSSI_MONITOR;
      case WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE:
         return QWLAN_HAL_CFG_NUM_BEACON_PER_RSSI_AVERAGE;
      case WNI_CFG_STATS_PERIOD:
         return QWLAN_HAL_CFG_STATS_PERIOD;
      case WNI_CFG_CFP_MAX_DURATION:
         return QWLAN_HAL_CFG_CFP_MAX_DURATION;
#if 0 /*This is not part of CFG*/
      case WNI_CFG_FRAME_TRANS_ENABLED:
         return QWLAN_HAL_CFG_FRAME_TRANS_ENABLED;
#endif
      case WNI_CFG_DTIM_PERIOD:
         return QWLAN_HAL_CFG_DTIM_PERIOD;
      case WNI_CFG_EDCA_WME_ACBK:
         return QWLAN_HAL_CFG_EDCA_WMM_ACBK;
      case WNI_CFG_EDCA_WME_ACBE:
         return QWLAN_HAL_CFG_EDCA_WMM_ACBE;
      case WNI_CFG_EDCA_WME_ACVI:
         return QWLAN_HAL_CFG_EDCA_WMM_ACVI;
      case WNI_CFG_EDCA_WME_ACVO:
         return QWLAN_HAL_CFG_EDCA_WMM_ACVO;
      default:
      {
         VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "There is no HAL CFG Id corresponding to WNI CFG Id: %d\n",
                       wniCfgId);
         //WDA_VOS_ASSERT(0);
         return VOS_STATUS_E_INVAL;
      }
   }
}

/*
 * FUNCTION: WDA_UpdateCfgCallback
 * 
 */ 
void WDA_UpdateCfgCallback(WDI_Status   wdiStatus, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   WDI_UpdateCfgReqParamsType *wdiCfgParam = 
                  (WDI_UpdateCfgReqParamsType *)pWDA->wdaWdiCfgApiMsgParam ;
   /*
    * currently there is no response message is expected between PE and
    * WDA, Failure return from WDI is a ASSERT condition
    */
   if(WDI_STATUS_SUCCESS != wdiStatus)
   {
      VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                  "%s: CFG (%d) config failure \n", __FUNCTION__, 
              ((tHalCfg *)(wdiCfgParam->pConfigBuffer))->type);
   }
   
   vos_mem_free(wdiCfgParam->pConfigBuffer) ;
   vos_mem_free(pWDA->wdaWdiCfgApiMsgParam) ;
   pWDA->wdaWdiCfgApiMsgParam = NULL;

   return ;
}

/*
 * FUNCTION: WDA_UpdateCfg
 * 
 */ 
VOS_STATUS WDA_UpdateCfg(tWDA_CbContext *pWDA, tSirMsgQ *cfgParam)
{
   
   WDI_Status status = WDI_STATUS_SUCCESS ;
   tANI_U32 val =0;
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pWDA->pVosContext) ;
   tHalCfg *configData;
   WDI_UpdateCfgReqParamsType *wdiCfgReqParam = NULL ;
   tANI_U8        *configDataValue;

   if(WDA_START_STATE != pWDA->wdaState)
   {
      return VOS_STATUS_E_FAILURE;
   }

   wdiCfgReqParam = (WDI_UpdateCfgReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_UpdateCfgReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiCfgReqParam);
   
   wdiCfgReqParam->pConfigBuffer =  vos_mem_malloc(sizeof(tHalCfg) + 
                                                            sizeof(tANI_U32)) ;

   if(NULL == wdiCfgReqParam->pConfigBuffer)
   {
      VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                             "%s: VOS Alloc Failure \n", __FUNCTION__);
      vos_mem_free(wdiCfgReqParam);
      WDA_VOS_ASSERT(0);
      return VOS_STATUS_E_NOMEM;
   }
   
   /*convert the WNI CFG Id to HAL CFG Id*/
   ((tHalCfg *)wdiCfgReqParam->pConfigBuffer)->type =
                             WDA_ConvertWniCfgIdToHALCfgId(cfgParam->bodyval);
   
   /*TODO: revisit this for handling string parameters */
   if (wlan_cfgGetInt(pMac, (tANI_U16) cfgParam->bodyval, 
                                                      &val) != eSIR_SUCCESS)
   {
       VOS_TRACE(VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                              "Failed to cfg get id %d\n", cfgParam->bodyval);
       return eSIR_FAILURE;
   }

   ((tHalCfg *)wdiCfgReqParam->pConfigBuffer)->length = sizeof(tANI_U32);
   configData =((tHalCfg *)wdiCfgReqParam->pConfigBuffer) ;
   configDataValue = ((tANI_U8 *)configData + sizeof(tHalCfg));
   vos_mem_copy( configDataValue, &val, sizeof(tANI_U32));

   WDA_VOS_ASSERT(NULL == pWDA->wdaWdiCfgApiMsgParam);
   
   /* store Params pass it to WDI */
   pWDA->wdaWdiCfgApiMsgParam = (void *)wdiCfgReqParam ;

#ifdef FEATURE_HAL_SUPPORT_DYNAMIC_UPDATE_CFG
   status = WDI_UpdateCfgReq(wdiCfgReqParam, 
                   (WDI_UpdateCfgRspCb )WDA_UpdateCfgCallback, pWDA) ;

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Update CFG WDI API, free all the memory " );
      vos_mem_free(wdiCfgReqParam->pConfigBuffer) ;
      vos_mem_free(pWDA->wdaWdiCfgApiMsgParam) ;
      pWDA->wdaWdiCfgApiMsgParam = NULL;
      /* Failure is not expected */
      WDA_VOS_ASSERT(0) ;
   }
#else
   vos_mem_free(wdiCfgReqParam->pConfigBuffer) ;
   vos_mem_free(pWDA->wdaWdiCfgApiMsgParam) ;
   pWDA->wdaWdiCfgApiMsgParam = NULL;
#endif
   return CONVERT_WDI2VOS_STATUS(status) ;
}


VOS_STATUS WDA_GetWepKeysFromCfg( tWDA_CbContext *pWDA, 
                                                      v_U8_t *pDefaultKeyId,
                                                      v_U8_t *pNumKeys,
                                                      WDI_KeysType *pWdiKeys )
{
   v_U32_t i, j, defKeyId = 0;
   v_U32_t val = SIR_MAC_KEY_LENGTH;
   VOS_STATUS status = WDI_STATUS_SUCCESS;
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pWDA->pVosContext) ;

   if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, WNI_CFG_WEP_DEFAULT_KEYID,
                                                                    &defKeyId ))
   {
        VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
            "Unable to retrieve defaultKeyId from CFG. Defaulting to 0...");
   }
   
  *pDefaultKeyId = (v_U8_t)defKeyId;

   /* Need to extract ALL of the configured WEP Keys */
   for( i = 0, j = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++ )
   {
      val = SIR_MAC_KEY_LENGTH;
      if( eSIR_SUCCESS != wlan_cfgGetStr( pMac, 
                                     (v_U16_t) (WNI_CFG_WEP_DEFAULT_KEY_1 + i),
                                     pWdiKeys[j].key,
                                     &val ))
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                            "WEP Key index [%d] may not configured in CFG\n",i);
      }
      else
      {
         pWdiKeys[j].keyId = (tANI_U8) i;
         /* 
         * Actually, a DC (Don't Care) because
         * this is determined (and set) by PE/MLME
         */
         pWdiKeys[j].unicast = 0;
         /*
         *  Another DC (Don't Care)
         */
         pWdiKeys[j].keyDirection = eSIR_TX_RX;
         /* Another DC (Don't Care). Unused for WEP */
         pWdiKeys[j].paeRole = 0;
         /* Determined from wlan_cfgGetStr() above.*/
         pWdiKeys[j].keyLength = (tANI_U16) val;

         j++;
         *pNumKeys = (tANI_U8) j;
      }
   }

   return status;
}

/*
 * FUNCTION: WDA_SetBssKeyReqCallback
 * send SET BSS key RSP back to PE
 */ 
void WDA_SetBssKeyReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tSetBssKeyParams *setBssKeyParams = (tSetBssKeyParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;
   setBssKeyParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_SET_BSSKEY_RSP, (void *)setBssKeyParams , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessSetBssKeyReq
 * Request to WDI for programming the BSS key( key for 
 * broadcast/multicast frames Encryption)
 */ 
VOS_STATUS WDA_ProcessSetBssKeyReq(tWDA_CbContext *pWDA, 
                                          tSetBssKeyParams *setBssKeyParams )
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_SetBSSKeyReqParamsType *wdiSetBssKeyParam = 
                  (WDI_SetBSSKeyReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_SetBSSKeyReqParamsType)) ;
   v_U8_t keyIndex;

   WDA_VOS_ALLOC_FAIL(wdiSetBssKeyParam) ; 

   if(setBssKeyParams->encType == eSIR_ED_NONE)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                 " set BSS key request called with NONE encryption type " );
      vos_mem_free(wdiSetBssKeyParam) ;
      setBssKeyParams->status = eSIR_SUCCESS ;
      WDA_SendMsg(pWDA, WDA_SET_BSSKEY_RSP, (void *)setBssKeyParams, 0) ;
      return CONVERT_WDI2VOS_STATUS(status) ;
   }

   vos_mem_zero(wdiSetBssKeyParam, sizeof(WDI_SetBSSKeyReqParamsType));

   /* copy set BSS params to WDI structure */
   wdiSetBssKeyParam->wdiBSSKeyInfo.ucBssIdx = setBssKeyParams->bssIdx;
   wdiSetBssKeyParam->wdiBSSKeyInfo.wdiEncType = setBssKeyParams->encType;
   wdiSetBssKeyParam->wdiBSSKeyInfo.ucNumKeys = setBssKeyParams->numKeys;
   if( setBssKeyParams->numKeys == 0 && 
      (( setBssKeyParams->encType == eSIR_ED_WEP40)|| 
                                    setBssKeyParams->encType == eSIR_ED_WEP104))
   {
      tANI_U8 defaultKeyId = 0;

      WDA_GetWepKeysFromCfg( pWDA, &defaultKeyId, 
         &wdiSetBssKeyParam->wdiBSSKeyInfo.ucNumKeys,
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys );
   }
   else
   {
      for( keyIndex=0; keyIndex < setBssKeyParams->numKeys; keyIndex++)
      {
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].keyId =
                                       setBssKeyParams->key[keyIndex].keyId;
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].unicast =
                                       setBssKeyParams->key[keyIndex].unicast;
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].keyDirection =
                                   setBssKeyParams->key[keyIndex].keyDirection;
         vos_mem_copy(wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].keyRsc, 
                  setBssKeyParams->key[keyIndex].keyRsc, WLAN_MAX_KEY_RSC_LEN);
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].paeRole =
                                        setBssKeyParams->key[keyIndex].paeRole;
         wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].keyLength =
                                      setBssKeyParams->key[keyIndex].keyLength;
         vos_mem_copy(wdiSetBssKeyParam->wdiBSSKeyInfo.aKeys[keyIndex].key, 
                                       setBssKeyParams->key[keyIndex].key, 
                                       SIR_MAC_MAX_KEY_LENGTH);
      }
   }
   wdiSetBssKeyParam->wdiBSSKeyInfo.ucSingleTidRc = 
                                      setBssKeyParams->singleTidRc;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store set key pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)setBssKeyParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiSetBssKeyParam ;

   status = WDI_SetBSSKeyReq(wdiSetBssKeyParam, 
                           (WDI_SetBSSKeyRspCb)WDA_SetBssKeyReqCallback ,pWDA);
   
   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in Set BSS Key Req WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      setBssKeyParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_SET_BSSKEY_RSP, (void *)setBssKeyParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_SetBssKeyReqCallback
 * send SET BSS key RSP back to PE
 */ 
void WDA_RemoveBssKeyReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tRemoveBssKeyParams *removeBssKeyParams = 
                              (tRemoveBssKeyParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;
   
   removeBssKeyParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_REMOVE_BSSKEY_RSP, (void *)removeBssKeyParams , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessRemoveBssKeyReq
 * Request to WDI to remove the BSS key( key for broadcast/multicast 
 * frames Encryption)
 */ 
VOS_STATUS WDA_ProcessRemoveBssKeyReq(tWDA_CbContext *pWDA, 
                                       tRemoveBssKeyParams *removeBssKeyParams )
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_RemoveBSSKeyReqParamsType *wdiRemoveBssKeyParam = 
                  (WDI_RemoveBSSKeyReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_RemoveBSSKeyReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiRemoveBssKeyParam) ; 

   /* copy Remove BSS key params to WDI structure*/
   wdiRemoveBssKeyParam->wdiKeyInfo.ucBssIdx = removeBssKeyParams->bssIdx;
   wdiRemoveBssKeyParam->wdiKeyInfo.wdiEncType = removeBssKeyParams->encType;
   wdiRemoveBssKeyParam->wdiKeyInfo.ucKeyId = removeBssKeyParams->keyId;
   wdiRemoveBssKeyParam->wdiKeyInfo.wdiWEPType = removeBssKeyParams->wepType;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store remove key pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)removeBssKeyParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiRemoveBssKeyParam ;

   status = WDI_RemoveBSSKeyReq(wdiRemoveBssKeyParam, 
                     (WDI_RemoveBSSKeyRspCb)WDA_RemoveBssKeyReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in Remove BSS Key Req WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      removeBssKeyParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_REMOVE_BSSKEY_RSP, (void *)removeBssKeyParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_SetBssKeyReqCallback
 * send SET BSS key RSP back to PE
 */ 
void WDA_SetStaKeyReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tSetStaKeyParams *setStaKeyParams = (tSetStaKeyParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   setStaKeyParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_SET_STAKEY_RSP, (void *)setStaKeyParams , 0) ;

   return ;
}

/*
 * FUNCTION: WDA_ProcessSetStaKeyReq
 * Request to WDI for programming the STA key( key for Unicast frames 
 * Encryption)
 */
VOS_STATUS WDA_ProcessSetStaKeyReq(tWDA_CbContext *pWDA, 
                                          tSetStaKeyParams *setStaKeyParams )
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_SetSTAKeyReqParamsType *wdiSetStaKeyParam = 
                  (WDI_SetSTAKeyReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_SetSTAKeyReqParamsType)) ;
   v_U8_t keyIndex;

   WDA_VOS_ALLOC_FAIL(wdiSetStaKeyParam) ; 

   if(setStaKeyParams->encType == eSIR_ED_NONE)
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   " set STA key request called with NONE encryption type ");
      vos_mem_free(wdiSetStaKeyParam) ;
      setStaKeyParams->status = eSIR_SUCCESS;
      WDA_SendMsg(pWDA, WDA_SET_STAKEY_RSP, (void *)setStaKeyParams, 0) ;
      return CONVERT_WDI2VOS_STATUS(status) ;
   }
   vos_mem_zero(wdiSetStaKeyParam, sizeof(WDI_SetSTAKeyReqParamsType));

   /* copy set STA key params to WDI structure */
   wdiSetStaKeyParam->wdiKeyInfo.usSTAIdx = setStaKeyParams->staIdx;
   wdiSetStaKeyParam->wdiKeyInfo.wdiEncType = setStaKeyParams->encType;
   wdiSetStaKeyParam->wdiKeyInfo.wdiWEPType = setStaKeyParams->wepType;
   wdiSetStaKeyParam->wdiKeyInfo.ucDefWEPIdx = setStaKeyParams->defWEPIdx;

   if(setStaKeyParams->encType == eSIR_WEP_STATIC && 
                                          setStaKeyParams->defWEPIdx ==  0xFF)
   {
      WDA_GetWepKeysFromCfg( pWDA, &wdiSetStaKeyParam->wdiKeyInfo.ucDefWEPIdx, 
         &wdiSetStaKeyParam->wdiKeyInfo.ucNumKeys,
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey );
   }
   else
   {
#ifdef FEATURE_SAP
      for( keyIndex=0; keyIndex < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; keyIndex++)
      {
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].keyId =
                                        setStaKeyParams->key[keyIndex].keyId;
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].unicast =
                                        setStaKeyParams->key[keyIndex].unicast;
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].keyDirection =
                                   setStaKeyParams->key[keyIndex].keyDirection;

         vos_mem_copy(wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].keyRsc, 
                   setStaKeyParams->key[keyIndex].keyRsc, WLAN_MAX_KEY_RSC_LEN);
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].paeRole =
                                        setStaKeyParams->key[keyIndex].paeRole;
         wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].keyLength =
                                      setStaKeyParams->key[keyIndex].keyLength;
         vos_mem_copy(wdiSetStaKeyParam->wdiKeyInfo.wdiKey[keyIndex].key, 
                   setStaKeyParams->key[keyIndex].key, SIR_MAC_MAX_KEY_LENGTH);
      }

      wdiSetStaKeyParam->wdiKeyInfo.ucNumKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
#else
      wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].keyId =setStaKeyParams->key.keyId;
      wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].unicast = 
                                                setStaKeyParams->key.unicast;
      wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].keyDirection =
                                          setStaKeyParams->key.keyDirection;
      vos_mem_copy(wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].keyRsc, 
                           setStaKeyParams->key.keyRsc, WLAN_MAX_KEY_RSC_LEN);
      wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].paeRole =
                                                setStaKeyParams->key.paeRole;
      wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].keyLength =
                                               setStaKeyParams->key.keyLength;
      vos_mem_copy(wdiSetStaKeyParam->wdiKeyInfo.wdiKey[0].key, 
                       setStaKeyParams->key[keyIndex].key, 
                                           SIR_MAC_MAX_KEY_LENGTH);
      wdiSetStaKeyParam->wdiKeyInfo.ucNumKeys = 1;
#endif
   }
   wdiSetStaKeyParam->wdiKeyInfo.ucSingleTidRc = setStaKeyParams->singleTidRc;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store set key pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)setStaKeyParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiSetStaKeyParam ;

   status = WDI_SetSTAKeyReq(wdiSetStaKeyParam, 
                          (WDI_SetSTAKeyRspCb)WDA_SetStaKeyReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                   "Failure in set STA Key Req WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      setStaKeyParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_SET_STAKEY_RSP, (void *)setStaKeyParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_SetBssKeyReqCallback
 * send SET BSS key RSP back to PE
 */ 
void WDA_RemoveStaKeyReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tRemoveStaKeyParams *removeStaKeyParams = 
                           (tRemoveStaKeyParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   removeStaKeyParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_REMOVE_STAKEY_RSP, (void *)removeStaKeyParams , 0) ;

   return ;
}


/*
 * FUNCTION: WDA_ProcessRemoveStaKeyReq
 * Request to WDI to remove the STA key( key for Unicast frames Encryption)
 */ 
VOS_STATUS WDA_ProcessRemoveStaKeyReq(tWDA_CbContext *pWDA, 
                                    tRemoveStaKeyParams *removeStaKeyParams )
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_RemoveSTAKeyReqParamsType *wdiRemoveStaKeyParam = 
                  (WDI_RemoveSTAKeyReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_RemoveSTAKeyReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiRemoveStaKeyParam) ; 

   /* copy remove STA key params to WDI structure*/
   wdiRemoveStaKeyParam->wdiKeyInfo.usSTAIdx = removeStaKeyParams->staIdx;
   wdiRemoveStaKeyParam->wdiKeyInfo.wdiEncType = removeStaKeyParams->encType;
   wdiRemoveStaKeyParam->wdiKeyInfo.ucKeyId = removeStaKeyParams->keyId;
   wdiRemoveStaKeyParam->wdiKeyInfo.ucUnicast = removeStaKeyParams->unicast;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store remove key pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)removeStaKeyParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiRemoveStaKeyParam ;

   status = WDI_RemoveSTAKeyReq(wdiRemoveStaKeyParam, 
                     (WDI_RemoveSTAKeyRspCb)WDA_RemoveStaKeyReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
               "Failure in remove STA Key Req WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
      removeStaKeyParams->status = eSIR_FAILURE ;
      WDA_SendMsg(pWDA, WDA_REMOVE_STAKEY_RSP, (void *)removeStaKeyParams, 0) ;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_IsHandleSetLinkStateReq
 * Update the WDA state and return the status to handle this message or not
 */ 

WDA_processSetLinkStateStatus WDA_IsHandleSetLinkStateReq(
                                          tWDA_CbContext *pWDA,
                                          tLinkStateParams *linkStateParams)
{
   WDA_processSetLinkStateStatus status = WDA_PROCESS_SET_LINK_STATE;

   switch(linkStateParams->state)
   {
      case eSIR_LINK_PREASSOC_STATE:
        /* 
         * set the WDA state to PRE ASSOC 
         * copy the BSSID into pWDA to use it in join request and return, 
         * No need to handle these messages.
         */
         vos_mem_copy(pWDA->macBSSID,linkStateParams->bssid, 
                                                   sizeof(tSirMacAddr));
        /* UMAC is issuing the setlink state with PREASSOC twice (before set 
         *channel and after ) so reset the WDA state to ready when the second 
         * time UMAC issue the link state with PREASSOC 
         */
         if(WDA_PRE_ASSOC_STATE == pWDA->wdaState)
         {
            /* RESET WDA state back to WDA_READY_STATE */
            pWDA->wdaState = WDA_READY_STATE;
         }
         else
         {
            pWDA->wdaState = WDA_PRE_ASSOC_STATE;
         }
         status = WDA_IGNORE_SET_LINK_STATE;
         break;

      default:
         if(pWDA->wdaState != WDA_READY_STATE)
         {
             VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Set link state called when WDA is not in READY STATE " );
             status = WDA_IGNORE_SET_LINK_STATE;
         }
         break;
   }
   
   return status;
}

/*
 * FUNCTION: WDA_SetLinkStateCallback
 * call back function for set link state from WDI
 */ 
void WDA_SetLinkStateCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tLinkStateParams *linkStateParams = 
                        (tLinkStateParams *)pWDA->wdaLinkMsgParam ;

   vos_mem_free(pWDA->wdaWdiLinkApiMsgParam) ;
   pWDA->wdaWdiLinkApiMsgParam = NULL;
   pWDA->wdaLinkMsgParam = NULL;

   /* 
    * No respone required for WDA_SET_LINK_STATE so free the request 
    * param here
    */

   vos_mem_free(linkStateParams);

   return ;
}

/*
 * FUNCTION: WDA_ProcessSetLinkState
 * Request to WDI to set the link status.
 */ 
VOS_STATUS WDA_ProcessSetLinkState(tWDA_CbContext *pWDA, 
                                           tLinkStateParams *linkStateParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_SetLinkReqParamsType *wdiSetLinkStateParam = 
                  (WDI_SetLinkReqParamsType *)vos_mem_malloc(
                                   sizeof(WDI_SetLinkReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiSetLinkStateParam) ; 

   if(WDA_IGNORE_SET_LINK_STATE == 
                  WDA_IsHandleSetLinkStateReq(pWDA,linkStateParams))
   {
      status = WDI_STATUS_E_FAILURE;
   }
   else
   {
      vos_mem_copy(wdiSetLinkStateParam->wdiLinkInfo.macBSSID, 
                                      linkStateParams->bssid, sizeof(tSirMacAddr));
      wdiSetLinkStateParam->wdiLinkInfo.wdiLinkState = linkStateParams->state;

      WDA_VOS_ASSERT((NULL == pWDA->wdaLinkMsgParam) && 
                                          (NULL == pWDA->wdaWdiLinkApiMsgParam));

      /* Store remove key pointer, as this will be used for response */
      pWDA->wdaLinkMsgParam = (void *)linkStateParams ;

      /* store Params pass it to WDI */
      pWDA->wdaWdiLinkApiMsgParam = (void *)wdiSetLinkStateParam ;

      status = WDI_SetLinkStateReq(wdiSetLinkStateParam, 
                        (WDI_SetLinkStateRspCb)WDA_SetLinkStateCallback, pWDA);

      if(IS_WDI_STATUS_FAILURE(status))
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
             "Failure in set link state Req WDI API, free all the memory " );
      }
   }

   if(IS_WDI_STATUS_FAILURE(status))
   {
      vos_mem_free(wdiSetLinkStateParam) ;
      vos_mem_free(linkStateParams);
      pWDA->wdaWdiLinkApiMsgParam = NULL;
      pWDA->wdaLinkMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_UpdateEDCAParamCallback
 * call back function for Update EDCA params from WDI
 */ 
void WDA_UpdateEDCAParamCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tEdcaParams *pEdcaParams = (tEdcaParams *)pWDA->wdaEdcaMsgParam ;

   vos_mem_free(pWDA->wdaWdiEdcaApiMsgParam) ;
   pWDA->wdaWdiEdcaApiMsgParam = NULL;
   pWDA->wdaEdcaMsgParam = NULL;

   vos_mem_free(pEdcaParams);

   return ;
}

/*
 * FUNCTION: WDA_ProcessUpdateEDCAParamReq
 * Request to WDI to Update the EDCA params.
 */ 
VOS_STATUS WDA_ProcessUpdateEDCAParamReq(tWDA_CbContext *pWDA, 
                                                   tEdcaParams *pEdcaParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_UpdateEDCAParamsType *wdiEdcaParam = 
                     (WDI_UpdateEDCAParamsType *)vos_mem_malloc(
                                             sizeof(WDI_UpdateEDCAParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiEdcaParam);

   wdiEdcaParam->wdiEDCAInfo.ucBssIdx = pEdcaParams->bssIdx;
   wdiEdcaParam->wdiEDCAInfo.ucEDCAParamsValid = pEdcaParams->highPerformance;
   WDA_UpdateEdcaParamsForAC(pWDA, &wdiEdcaParam->wdiEDCAInfo.wdiEdcaBEInfo,
                                                           &pEdcaParams->acbe);
   WDA_UpdateEdcaParamsForAC(pWDA, &wdiEdcaParam->wdiEDCAInfo.wdiEdcaBKInfo,
                                                           &pEdcaParams->acbk);
   WDA_UpdateEdcaParamsForAC(pWDA, &wdiEdcaParam->wdiEDCAInfo.wdiEdcaVIInfo,
                                                           &pEdcaParams->acvi);
   WDA_UpdateEdcaParamsForAC(pWDA, &wdiEdcaParam->wdiEDCAInfo.wdiEdcaVOInfo,
                                                           &pEdcaParams->acvo);

   WDA_VOS_ASSERT((NULL == pWDA->wdaEdcaMsgParam) && 
                                       (NULL == pWDA->wdaWdiEdcaApiMsgParam));

   /* Store remove key pointer, as this will be used for response */
   pWDA->wdaEdcaMsgParam = (void *)pEdcaParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiEdcaApiMsgParam = (void *)wdiEdcaParam ;

   status = WDI_UpdateEDCAParams(wdiEdcaParam, 
               (WDI_UpdateEDCAParamsRspCb)WDA_UpdateEDCAParamCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in Update EDCA Params WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiEdcaApiMsgParam) ;
      vos_mem_free(pEdcaParams);
      pWDA->wdaWdiEdcaApiMsgParam = NULL;
      pWDA->wdaEdcaMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;
}

/*
 * FUNCTION: WDA_AddBAReqCallback
 * send ADD BA RSP back to PE
 */ 
void WDA_AddBAReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tAddBAParams *pAddBAReqParams = 
                           (tAddBAParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   pAddBAReqParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_ADDBA_RSP, (void *)pAddBAReqParams , 0) ;

   return ;
}


/*
 * FUNCTION: WDA_ProcessAddBAReq
 * Request to WDI to Update the ADDBA REQ params.
 */ 
VOS_STATUS WDA_ProcessAddBAReq(tWDA_CbContext *pWDA, 
                                                tAddBAParams *pAddBAReqParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_AddBAReqParamsType *wdiAddBAReqParam = 
                     (WDI_AddBAReqParamsType *)vos_mem_malloc(
                                             sizeof(WDI_AddBAReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiAddBAReqParam);

   wdiAddBAReqParam->wdiBAInfoType.usSTAIdx = pAddBAReqParams->staIdx;
   vos_mem_copy(wdiAddBAReqParam->wdiBAInfoType.macPeerAddr,
                           pAddBAReqParams->peerMacAddr, sizeof(tSirMacAddr));
   wdiAddBAReqParam->wdiBAInfoType.ucBaTID = pAddBAReqParams->baTID;
   wdiAddBAReqParam->wdiBAInfoType.ucBaPolicy = pAddBAReqParams->baPolicy;
   wdiAddBAReqParam->wdiBAInfoType.usBaBufferSize = 
                                                pAddBAReqParams->baBufferSize;
   wdiAddBAReqParam->wdiBAInfoType.usBaTimeout = pAddBAReqParams->baTimeout;
   wdiAddBAReqParam->wdiBAInfoType.usBaSSN = pAddBAReqParams->baSSN;
   wdiAddBAReqParam->wdiBAInfoType.ucBaDirection = 
                                                pAddBAReqParams->baDirection;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store ADD BA pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)pAddBAReqParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiAddBAReqParam ;

   status = WDI_AddBAReq(wdiAddBAReqParam, 
                                (WDI_AddBARspCb)WDA_AddBAReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in ADD BA REQ Params WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      vos_mem_free(pWDA->wdaMsgParam);
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;

}

/*
 * FUNCTION: WDA_DelBAReqCallback
 * send DEL BA RSP back to PE
 */ 
void WDA_DelBAReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   //tDelBAParams *pDelBAReqParams = 
   //                        (tDelBAParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   vos_mem_free(pWDA->wdaMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   /* 
    * No respone required for WDA_DELBA_IND so just free the request 
    * param here
    */

   return ;
}


/*
 * FUNCTION: WDA_ProcessDelBAReq
 * Request to WDI to Update the DELBA REQ params.
 */ 
VOS_STATUS WDA_ProcessDelBAReq(tWDA_CbContext *pWDA, 
                                                tDelBAParams *pDelBAReqParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_DelBAReqParamsType *wdiDelBAReqParam = 
                     (WDI_DelBAReqParamsType *)vos_mem_malloc(
                                             sizeof(WDI_DelBAReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiDelBAReqParam);

   wdiDelBAReqParam->wdiBAInfo.usSTAIdx = pDelBAReqParams->staIdx;
   wdiDelBAReqParam->wdiBAInfo.ucBaTID = pDelBAReqParams->baTID;
   wdiDelBAReqParam->wdiBAInfo.ucBaDirection = pDelBAReqParams->baDirection;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store DEL BA pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)pDelBAReqParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiDelBAReqParam ;

   status = WDI_DelBAReq(wdiDelBAReqParam, 
                                  (WDI_DelBARspCb)WDA_DelBAReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in DEL BA REQ Params WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      vos_mem_free(pWDA->wdaMsgParam);
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;

}

/*
 * FUNCTION: WDA_AddTSReqCallback
 * send ADD TS RSP back to PE
 */ 
void WDA_AddTSReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   tAddTsParams *pAddTsReqParams = 
                           (tAddTsParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;
   
   pAddTsReqParams->status = CONVERT_WDI2SIR_STATUS(status) ;

   WDA_SendMsg(pWDA, WDA_ADD_TS_RSP, (void *)pAddTsReqParams , 0) ;

   return ;
}



/*
 * FUNCTION: WDA_ProcessAddTSReq
 * Request to WDI to Update the ADD TS  REQ params.
 */ 
VOS_STATUS WDA_ProcessAddTSReq(tWDA_CbContext *pWDA, 
                                                tAddTsParams *pAddTsReqParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_AddTSReqParamsType *wdiAddTSReqParam = 
                     (WDI_AddTSReqParamsType *)vos_mem_malloc(
                                             sizeof(WDI_AddTSReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiAddTSReqParam);

   wdiAddTSReqParam->wdiTsInfo.usSTAIdx = pAddTsReqParams->staIdx;
   wdiAddTSReqParam->wdiTsInfo.ucTspecIdx = pAddTsReqParams->tspecIdx;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.ucType = pAddTsReqParams->tspec.type;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.ucLength = 
                                                pAddTsReqParams->tspec.length;
   
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.ackPolicy =
                           pAddTsReqParams->tspec.tsinfo.traffic.ackPolicy;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.userPrio =
                           pAddTsReqParams->tspec.tsinfo.traffic.userPrio;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.psb =
                           pAddTsReqParams->tspec.tsinfo.traffic.psb;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.aggregation =
                           pAddTsReqParams->tspec.tsinfo.traffic.aggregation;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.accessPolicy =
                           pAddTsReqParams->tspec.tsinfo.traffic.accessPolicy;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.direction =
                           pAddTsReqParams->tspec.tsinfo.traffic.direction;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.tsid =
                           pAddTsReqParams->tspec.tsinfo.traffic.tsid;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiTraffic.trafficType =
                           pAddTsReqParams->tspec.tsinfo.traffic.trafficType;
   
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.wdiTSinfo.wdiSchedule.schedule = 
                           pAddTsReqParams->tspec.tsinfo.schedule.schedule;
   
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.usNomMsduSz = 
                           pAddTsReqParams->tspec.nomMsduSz;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.usMaxMsduSz = 
                           pAddTsReqParams->tspec.maxMsduSz;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMinSvcInterval = 
                           pAddTsReqParams->tspec.minSvcInterval;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMaxSvcInterval = 
                           pAddTsReqParams->tspec.maxSvcInterval;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uInactInterval = 
                           pAddTsReqParams->tspec.inactInterval;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uSuspendInterval = 
                           pAddTsReqParams->tspec.suspendInterval;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uSvcStartTime = 
                           pAddTsReqParams->tspec.svcStartTime;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMinDataRate = 
                           pAddTsReqParams->tspec.minDataRate;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMeanDataRate = 
                           pAddTsReqParams->tspec.meanDataRate;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uPeakDataRate = 
                           pAddTsReqParams->tspec.peakDataRate;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMaxBurstSz = 
                           pAddTsReqParams->tspec.maxBurstSz;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uDelayBound = 
                           pAddTsReqParams->tspec.delayBound;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.uMinPhyRate = 
                           pAddTsReqParams->tspec.minPhyRate;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.usSurplusBw = 
                           pAddTsReqParams->tspec.surplusBw;
   wdiAddTSReqParam->wdiTsInfo.wdiTspecIE.usMediumTime = 
                           pAddTsReqParams->tspec.mediumTime;

   /* TODO: tAddTsParams doesn't have the following fields */
#if 0 
   wdiAddTSReqParam->wdiTsInfo.ucUapsdFlags = 
   wdiAddTSReqParam->wdiTsInfo.ucServiceInterval = 
   wdiAddTSReqParam->wdiTsInfo.ucSuspendInterval = 
   wdiAddTSReqParam->wdiTsInfo.ucDelayedInterval = 
#endif
   
   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store ADD TS pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)pAddTsReqParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiAddTSReqParam ;

   status = WDI_AddTSReq(wdiAddTSReqParam, 
                                  (WDI_AddTsRspCb)WDA_AddTSReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in ADD TS REQ Params WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      vos_mem_free(pWDA->wdaMsgParam);
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;

}


/*
 * FUNCTION: WDA_DelTSReqCallback
 * send DEL TS RSP back to PE
 */ 
void WDA_DelTSReqCallback(WDI_Status status, void* pUserData)
{
   tWDA_CbContext *pWDA = (tWDA_CbContext *)pUserData ; 
   //tDelTsParams *pDelTSReqParams = 
   //                        (tDelTsParams *)pWDA->wdaMsgParam ;

   vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
   vos_mem_free(pWDA->wdaMsgParam) ;
   pWDA->wdaWdiApiMsgParam = NULL;
   pWDA->wdaMsgParam = NULL;

   /* 
    * No respone required for WDA_DEL_TS_REQ so just free the request 
    * param here
    */

   return ;
}


/*
 * FUNCTION: WDA_ProcessDelTSReq
 * Request to WDI to Update the DELTS REQ params.
 */ 
VOS_STATUS WDA_ProcessDelTSReq(tWDA_CbContext *pWDA, 
                                                 tDelTsParams *pDelTSReqParams)
{
   WDI_Status status = WDI_STATUS_SUCCESS ;
   WDI_DelTSReqParamsType *wdiDelTSReqParam = 
                     (WDI_DelTSReqParamsType *)vos_mem_malloc(
                                             sizeof(WDI_DelTSReqParamsType)) ;

   WDA_VOS_ALLOC_FAIL(wdiDelTSReqParam);

   vos_mem_copy(wdiDelTSReqParam->wdiDelTSInfo.macBSSID, 
                                  pDelTSReqParams->bssId, sizeof(tSirMacAddr));
   wdiDelTSReqParam->wdiDelTSInfo.usSTAIdx = pDelTSReqParams->staIdx;
   wdiDelTSReqParam->wdiDelTSInfo.ucTspecIdx = pDelTSReqParams->tspecIdx;

   WDA_VOS_ASSERT((NULL == pWDA->wdaMsgParam) && 
                                       (NULL == pWDA->wdaWdiApiMsgParam));

   /* Store DEL TS pointer, as this will be used for response */
   pWDA->wdaMsgParam = (void *)pDelTSReqParams ;

   /* store Params pass it to WDI */
   pWDA->wdaWdiApiMsgParam = (void *)wdiDelTSReqParam ;

   status = WDI_DelTSReq(wdiDelTSReqParam, 
                                  (WDI_DelTsRspCb)WDA_DelTSReqCallback, pWDA);

   if(IS_WDI_STATUS_FAILURE(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
              "Failure in DEL TS REQ Params WDI API, free all the memory " );
      vos_mem_free(pWDA->wdaWdiApiMsgParam) ;
      vos_mem_free(pWDA->wdaMsgParam);
      pWDA->wdaWdiApiMsgParam = NULL;
      pWDA->wdaMsgParam = NULL;
   }

   return CONVERT_WDI2VOS_STATUS(status) ;

}

/*
 * -------------------------------------------------------------------------
 * DATA interface with WDI for Mgmt Frames
 * ------------------------------------------------------------------------- 
 */

/*
 * FUNCTION: WDA_TxComplete
 * Callback function for the WDA_TxPacket
 */ 
VOS_STATUS WDA_TxComplete( v_PVOID_t pVosContext, vos_pkt_t *pData, 
                                                VOS_STATUS status )
{
   tANI_U32 vosStatus = VOS_STATUS_SUCCESS ;
   tWDA_CbContext *wdaContext= (tWDA_CbContext *)VOS_GET_WDA_CTXT(pVosContext);

   /* 
    * Trigger the event to bring the HAL TL Tx complete function to come 
    * out of wait 
    */
   status  = vos_event_set(&wdaContext->txFrameEvent);
   if(!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
                      "NEW VOS Event Set failed - status = %d \n", vosStatus);
   }

   wdaContext->pTxCbFunc(pVosContext, pData);

   return status;
}

/*
 * FUNCTION: WDA_TxPacket
 * Forward TX management frame to WDI
 */ 
VOS_STATUS WDA_TxPacket(tWDA_CbContext *pWDA, 
                           void *pFrmBuf,
                           tANI_U16 frmLen,
                           eFrameType frmType,
                           eFrameTxDir txDir,
                           tANI_U8 tid,
                           pWDATxRxCompFunc pCompFunc,
                           void *pData)
{
   tANI_U32 status = VOS_STATUS_SUCCESS ;
   tpSirMacFrameCtl pFc = (tpSirMacFrameCtl ) pData;
   tANI_U8 ucTypeSubType = pFc->type <<4 | pFc->subType;
   tANI_U8 eventIdx = 0, txFlag = 0;
   tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
   tpAniSirGlobal pMac = (tpAniSirGlobal )VOS_GET_MAC_CTXT(pWDA->pVosContext) ;

   WDA_VOS_ASSERT(pWDA) ;
   WDA_VOS_ASSERT(pFrmBuf) ;

   VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
               "Tx Mgmt Frame Subtype: %d alloc(%x)\n", pFc->subType, pFrmBuf);

   /* store the call back function in WDA context */
   pWDA->pTxCbFunc = pCompFunc;

   /* Reset the event to be not signalled */
   status = vos_event_reset(&pWDA->txFrameEvent);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
                            "VOS Event reset failed - status = %d\n",status);
      pCompFunc(pWDA->pVosContext, (vos_pkt_t *)pFrmBuf);
      return VOS_STATUS_E_FAILURE;
   }

   /* Get system role, use the self station if in unknown role or STA role */
   systemRole = wdaGetGlobalSystemRole(pMac);

   if (( eSYSTEM_UNKNOWN_ROLE == systemRole ) || 
       ( eSYSTEM_STA_ROLE == systemRole )) 
   {
       txFlag = HAL_USE_SELF_STA_REQUESTED_MASK;
   }

   if((status = WLANTL_TxMgmtFrm(pWDA->pVosContext, (vos_pkt_t *)pFrmBuf, 
                     frmLen, ucTypeSubType, tid, 
                     WDA_TxComplete, NULL, txFlag)) != VOS_STATUS_SUCCESS) 
   {
       VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
                        "Sending Mgmt Frame failed - status = %d\n", status);
       pCompFunc(pWDA->pVosContext, (vos_pkt_t *)pFrmBuf);
       return VOS_STATUS_E_FAILURE;
   }

   /* 
    * Wait for the event to be set by the TL, to get the response of TX 
    * complete, this event should be set by the Callback function called by TL 
    */
   status = vos_wait_events(&pWDA->txFrameEvent, 1, WDA_TL_TX_FRAME_TIMEOUT,
                                                                     &eventIdx);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR, 
                            "VOS Event wait failed - status = %d\n",status);
      pCompFunc(pWDA->pVosContext, (vos_pkt_t *)pFrmBuf);
      status = VOS_STATUS_E_FAILURE;
   }

   return status;
}      



/*
 * FUNCTION: WDA_McProcessMsg
 * Trigger DAL-AL to start CFG download 
 */ 
VOS_STATUS WDA_McProcessMsg( v_CONTEXT_t pVosContext, vos_msg_t *pMsg )
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   tWDA_CbContext *pWDA = NULL ; 
   WDA_VOS_ASSERT( pMsg );

   
   pWDA = vos_get_context( VOS_MODULE_ID_WDA, pVosContext );
   /* Process all the WDA messages.. */
   switch( pMsg->type )
   {
      case WNI_CFG_DNLD_REQ:
      {
         status = WDA_WniCfgDnld(pWDA);

         /* call WDA complete event if config download success */
         if( VOS_IS_STATUS_SUCCESS(status) )
         {
            vos_WDAComplete_cback(pVosContext);
         }
         else
         {
            VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                     "WDA Config Download failure" );
         }
         break ;
      }

      /* 
       * Init SCAN request from PE, convert it into DAL format 
       * and send it to DAL 
       */ 
      case WDA_INIT_SCAN_REQ:
      {
         WDA_ProcessInitScanReq(pWDA, (tInitScanParams *)pMsg->bodyptr) ;
         break ;    
      }
      /* start SCAN request from PE */
      case WDA_START_SCAN_REQ:
      {
         WDA_ProcessStartScanReq(pWDA, (tStartScanParams *)pMsg->bodyptr) ;
         break ;    
      }
      /* end SCAN request from PE */
      case WDA_END_SCAN_REQ:
      {
         WDA_ProcessEndScanReq(pWDA, (tEndScanParams *)pMsg->bodyptr) ;
         break ;
      }
      /* end SCAN request from PE */
      case WDA_FINISH_SCAN_REQ:
      {
         WDA_ProcessFinishScanReq(pWDA, (tFinishScanParams *)pMsg->bodyptr) ;
         break ;    
      }
      /* join request from PE */
      case WDA_CHNL_SWITCH_REQ:
      {
         if(WDA_PRE_ASSOC_STATE == pWDA->wdaState)
         {
            WDA_ProcessJoinReq(pWDA, (tSwitchChannelParams *)pMsg->bodyptr) ;
         }
         else
         {
            WDA_ProcessChannelSwitchReq(pWDA, 
                                 (tSwitchChannelParams*)pMsg->bodyptr) ;
         }
         break ;
      }
      /* ADD BSS request from PE */
      case WDA_ADD_BSS_REQ:
      {
         WDA_ProcessConfigBssReq(pWDA, (tAddBssParams*)pMsg->bodyptr) ;
         break ;
      }
      case WDA_ADD_STA_REQ:
      {
         WDA_ProcessAddStaReq(pWDA, (tAddStaParams *)pMsg->bodyptr) ;
         break ;
      }
      case WDA_DELETE_BSS_REQ:
      {
         WDA_ProcessDelBssReq(pWDA, (tDeleteBssParams *)pMsg->bodyptr) ;
         break ;
      }
      case WDA_DELETE_STA_REQ:
      {
         WDA_ProcessDelStaReq(pWDA, (tDeleteStaParams *)pMsg->bodyptr) ;
         break ;
      }
      case WDA_CONFIG_PARAM_UPDATE_REQ:
      {
         WDA_UpdateCfg(pWDA, (tSirMsgQ *)pMsg) ;
         break ;
      }
      case WDA_SET_BSSKEY_REQ:
      {
         WDA_ProcessSetBssKeyReq(pWDA, (tSetBssKeyParams *)pMsg->bodyptr);
         break ;
      }
      case WDA_SET_STAKEY_REQ:
      {
         WDA_ProcessSetStaKeyReq(pWDA, (tSetStaKeyParams *)pMsg->bodyptr);
         break ;
      }
      case WDA_REMOVE_BSSKEY_REQ:
      {
         WDA_ProcessRemoveBssKeyReq(pWDA, 
                                    (tRemoveBssKeyParams *)pMsg->bodyptr);
         break ;
      }
      case WDA_REMOVE_STAKEY_REQ:
      {
         WDA_ProcessRemoveStaKeyReq(pWDA, 
                                    (tRemoveStaKeyParams *)pMsg->bodyptr);
         break ;
      }
      case WDA_UPDATE_EDCA_PROFILE_IND:
      {
         WDA_ProcessUpdateEDCAParamReq(pWDA, (tEdcaParams *)pMsg->bodyptr);
         break;
      }
      case WDA_ADD_TS_REQ:
      {
         WDA_ProcessAddTSReq(pWDA, (tAddTsParams *)pMsg->bodyptr);
         break;
      }
      case WDA_DEL_TS_REQ:
      {
         WDA_ProcessDelTSReq(pWDA, (tDelTsParams *)pMsg->bodyptr);
         break;
      }
      case WDA_ADDBA_REQ:
      {
         WDA_ProcessAddBAReq(pWDA, (tAddBAParams *)pMsg->bodyptr);
         break;
      }
      case WDA_DELBA_IND:
      {
         WDA_ProcessDelBAReq(pWDA, (tDelBAParams *)pMsg->bodyptr);
         break;
      }
      case WDA_SET_LINK_STATE:
      {
         WDA_ProcessSetLinkState(pWDA, (tLinkStateParams *)pMsg->bodyptr);
         break;
      }
      case WDA_PWR_SAVE_CFG:
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Handling msg type WDA_PWR_SAVE_CFG" );
         if(pWDA->wdaState == WDA_READY_STATE)
         {
            /*TODO: handle this while dealing with power save*/
         }
         break;
      }
      case WDA_REGISTER_PE_CALLBACK :
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                           "Handling msg type WDA_REGISTER_PE_CALLBACK " );
         /*TODO: store the PE callback */
         break;
      }
      case WDA_SYS_READY_IND :
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Handling msg type WDA_SYS_READY_IND " );
         pWDA->wdaState = WDA_READY_STATE;
         break;
      }
      case WDA_BEACON_FILTER_IND  :
      {
         /*TODO: handle this while dealing with power save */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                              "Handling msg type WDA_BEACON_FILTER_IND  " );
         break;
      }
      case WDA_BTC_SET_CFG:
      {
         /*TODO: handle this while dealing with BTC */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Handling msg type WDA_BTC_SET_CFG  " );
         break;
      }
#ifdef WDA_UT
      case WDA_WDI_EVENT_MSG:
      {
         WDI_processEvent(pMsg->bodyptr,(void *)pMsg->bodyval);
         break ;
      }
#endif
      default:
      {
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "No Handling for msg type %x in WDA " 
                                  ,pMsg->type);
         //WDA_VOS_ASSERT(0) ;
      }
   }

   return status ;
}


/*
 * FUNCTION: WDA_LowLevelIndCallback
 * IND API callback from WDI, send Ind to PE
 */ 
void WDA_lowLevelIndCallback(WDI_LowLevelIndType *wdiLowLevelInd, 
                                                         void* pUserData )
{

   switch(wdiLowLevelInd->wdiIndicationType)
   {
      case WDI_LOW_RSSI_IND:
      {
         /*TODO: decode Ind and send Meassage to PE */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                      "Recieved WDI_LOW_RSSI_IND from WDI ");
         break ;
      }
      case WDI_MISSED_BEACON_IND:
      {
         /* TODO: Decode Ind and send Ind to PE */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Recieved WDI_MISSED_BEACON_IND from WDI ");
         break ;
      }
      case WDI_UNKNOWN_ADDR2_FRAME_RX_IND:
      {
         /* TODO: Decode Ind and send Ind to PE */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                     "Recieved WDI_UNKNOWN_ADDR2_FRAME_RX_IND from WDI ");
         break ;
      }
       
      case WDI_MIC_FAILURE_IND:
      {
         /* TODO: Decode Ind and send Ind to PE */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Recieved WDI_MIC_FAILURE_IND from WDI ");
         break ;
      }
      case WDI_FATAL_ERROR_IND:
      {
         /* TODO: Decode Ind and send Ind to PE */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Recieved WDI_FATAL_ERROR_IND from WDI ");
         break ;
      }
      default:
      {
         /* TODO error */
         VOS_TRACE( VOS_MODULE_ID_WDA, VOS_TRACE_LEVEL_ERROR,
                                  "Recieved UNKNOWN Indication from WDI ");
      } 
   }
   return ;
}

#endif  /* FEATURE_WLAN_INTEGRATED_SOC */

