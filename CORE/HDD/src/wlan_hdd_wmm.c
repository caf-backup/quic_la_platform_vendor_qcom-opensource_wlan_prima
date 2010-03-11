/*============================================================================
  @file wlan_hdd_wmm.c

  This module (wlan_hdd_wmm.h interface + wlan_hdd_wmm.c implementation)
  houses all the logic for WMM in HDD.

  On the control path, it has the logic to setup QoS, modify QoS and delete
  QoS (QoS here refers to a TSPEC). The setup QoS comes in two flavors: an
  explicit application invoked and an internal HDD invoked.  The implicit QoS
  is for applications that do NOT call the custom QCT WLAN OIDs for QoS but
  which DO mark their traffic for priortization. It also has logic to start,
  update and stop the U-APSD trigger frame generation. It also has logic to
  read WMM related config parameters from the registry.

  On the data path, it has the logic to figure out the WMM AC of an egress
  packet and when to signal TL to serve a particular AC queue. It also has the
  logic to retrieve a packet based on WMM priority in response to a fetch from
  TL.

  The remaining functions are utility functions for information hiding.


               Copyright (c) 2008-9 QUALCOMM Incorporated.
               All Rights Reserved.
               Qualcomm Confidential and Proprietary
============================================================================*/

/*--------------------------------------------------------------------------- 
  Include files
  -------------------------------------------------------------------------*/ 
#include <wlan_hdd_tx_rx.h>
#include <wlan_hdd_dp_utils.h>
#include <wlan_hdd_wmm.h>
#include <wlan_hdd_ether.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>

// UAPSD Mask bits
// (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored)
#define HDD_AC_VO 0x1
#define HDD_AC_VI 0x2
#define HDD_AC_BK 0x4
#define HDD_AC_BE 0x8

#define WLAN_HDD_MAX_DSCP 0x3f

static sme_QosWmmUpType hddWmmDscpToUpMap[WLAN_HDD_MAX_DSCP+1];

static const v_U8_t hddWmmUpToAcMap[] = {
   WLANTL_AC_BE,
   WLANTL_AC_BK,
   WLANTL_AC_BK,
   WLANTL_AC_BE,
   WLANTL_AC_VI,
   WLANTL_AC_VI,
   WLANTL_AC_VO,
   WLANTL_AC_VO
};

/**
  @brief hdd_wmm_enable_tl_uapsd() - function which decides whether and
  how to update UAPSD parameters in TL

  @param pQosContext : [in] the pointer the QoS instance control block

  @return
  None
*/
static void hdd_wmm_enable_tl_uapsd (hdd_wmm_qos_context_t* pQosContext)
{
   hdd_adapter_t* pAdapter = pQosContext->pAdapter;
   WLANTL_ACEnumType acType = pQosContext->acType;
   hdd_wmm_ac_status_t *pAc = &pAdapter->hddWmmStatus.wmmAcStatus[acType];
   VOS_STATUS status;
   v_U32_t service_interval;
   v_U32_t suspension_interval;
   sme_QosWmmDirType direction;


   // The TSPEC must be valid
   if (pAc->wmmAcTspecValid == VOS_FALSE)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Invoked with invalid TSPEC",
                __FUNCTION__);
      return;
   }

   // determine the service interval
   if (pAc->wmmAcTspecInfo.min_service_interval)
   {
      service_interval = pAc->wmmAcTspecInfo.min_service_interval;
   }
   else if (pAc->wmmAcTspecInfo.max_service_interval)
   {
      service_interval = pAc->wmmAcTspecInfo.max_service_interval;
   }
   else
   {
      // no service interval is present in the TSPEC
      // this is OK, there just won't be U-APSD
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: No service interval supplied",
                __FUNCTION__);
      return;
   }

   // determine the suspension interval & direction
   suspension_interval = pAc->wmmAcTspecInfo.suspension_interval;
   direction = pAc->wmmAcTspecInfo.ts_info.direction;

   // if we have previously enabled U-APSD, have any params changed?
   if ((pAc->wmmAcUapsdInfoValid) &&
       (pAc->wmmAcUapsdServiceInterval == service_interval) &&
       (pAc->wmmAcUapsdSuspensionInterval == suspension_interval) &&
       (pAc->wmmAcUapsdDirection == direction))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: No change in U-APSD parameters",
                __FUNCTION__);
      return;
   }

   // are we in the appropriate power save modes?
   if (!sme_IsPowerSaveEnabled(pAdapter->hHal, ePMC_BEACON_MODE_POWER_SAVE))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: BMPS is not enabled",
                __FUNCTION__);
      return;
   }

   if (!sme_IsPowerSaveEnabled(pAdapter->hHal, ePMC_UAPSD_MODE_POWER_SAVE))
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: U-APSD is not enabled",
                __FUNCTION__);
      return;
   }

   // everything is in place to notify TL
   status = WLANTL_EnableUAPSDForAC(pAdapter->pvosContext,
                                    pAdapter->conn_info.staId[0],
                                    acType,
                                    pAc->wmmAcTspecInfo.ts_info.tid,
                                    pAc->wmmAcTspecInfo.ts_info.up,
                                    service_interval,
                                    suspension_interval,
                                    direction);

   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: Failed to enable U-APSD for AC=%d",
                 __FUNCTION__, acType );
      return;
   }

   // stash away the parameters that were used
   pAc->wmmAcUapsdInfoValid = VOS_TRUE;
   pAc->wmmAcUapsdServiceInterval = service_interval;
   pAc->wmmAcUapsdSuspensionInterval = suspension_interval;
   pAc->wmmAcUapsdDirection = direction;

   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Enabled UAPSD in TL srv_int=%ld "
             "susp_int=%ld dir=%d AC=%d (PS: NOT AN ERROR)\n",
             __FUNCTION__,
             service_interval,
             suspension_interval,
             direction,
             acType);

}


/**
  @brief hdd_wmm_disable_tl_uapsd() - function which decides whether
  to disable UAPSD parameters in TL

  @param pQosContext : [in] the pointer the QoS instance control block

  @return
  None
*/
static void hdd_wmm_disable_tl_uapsd (hdd_wmm_qos_context_t* pQosContext)
{
   hdd_adapter_t* pAdapter = pQosContext->pAdapter;
   WLANTL_ACEnumType acType = pQosContext->acType;
   hdd_wmm_ac_status_t *pAc = &pAdapter->hddWmmStatus.wmmAcStatus[acType];
   VOS_STATUS status;


   // have we previously enabled UAPSD?
   if (pAc->wmmAcUapsdInfoValid == VOS_TRUE)
   {
      status = WLANTL_DisableUAPSDForAC(pAdapter->pvosContext,
                                        pAdapter->conn_info.staId[0],
                                        acType);

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to disable U-APSD for AC=%d",
                    __FUNCTION__, acType );
      }
      else
      {
         // TL no longer has valid UAPSD info
         pAc->wmmAcUapsdInfoValid = VOS_FALSE;
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: Disabled UAPSD in TL for AC=%d (PS: NOT AN ERROR)\n",
                   __FUNCTION__,
                   acType);
      }
   }
}


/**
  @brief HddWmmSmeCallback() - callback registered by HDD with SME for receiving 
  QoS notifications. Even though this function has a static scope it gets called 
  externally through some function pointer magic (so there is a need for 
  rigorous parameter checking) 

  @param hHal : [in] the HAL handle  
  @param HddCtx : [in] the HDD specified handle 
  @param pCurrentQosInfo : [in] the TSPEC params
  @param SmeStatus : [in] the QoS related SME status

  @return 
  eHAL_STATUS_SUCCESS if all good, eHAL_STATUS_FAILURE otherwise
*/
static eHalStatus hdd_wmm_sme_callback (tHalHandle hHal,
                                        void * hddCtx, 
                                        sme_QosWmmTspecInfo* pCurrentQosInfo, 
                                        sme_QosStatusType smeStatus,
                                        v_U32_t qosFlowId)
{
   hdd_wmm_qos_context_t* pQosContext = hddCtx;
   hdd_adapter_t* pAdapter = pQosContext->pAdapter;
   WLANTL_ACEnumType acType = pQosContext->acType;
   hdd_wmm_ac_status_t *pAc = &pAdapter->hddWmmStatus.wmmAcStatus[acType];
   VOS_STATUS status;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: status %d flowid %d info %p",
             __FUNCTION__, smeStatus, qosFlowId, pCurrentQosInfo);
#endif // HDD_WMM_DEBUG

   switch (smeStatus)
   {

   case SME_QOS_STATUS_SETUP_SUCCESS_IND:
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup is complete, notify TL",
                __FUNCTION__);
#endif // HDD_WMM_DEBUG

      // there will always be a TSPEC returned with this status, even if
      // a TSPEC is not exchanged OTA
      if (pCurrentQosInfo)
      {
         pAc->wmmAcTspecValid = VOS_TRUE;
         memcpy(&pAc->wmmAcTspecInfo,
                pCurrentQosInfo,
                sizeof(pAc->wmmAcTspecInfo));
      }

      // update state
      pAc->wmmAcAccessGranted = VOS_TRUE;
      pAc->wmmAcAccessPending = VOS_FALSE;

      // notify TL that packets are pending
      status = WLANTL_STAPktPending( pAdapter->pvosContext,
                                     pAdapter->conn_info.staId[0],
                                     acType );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to signal TL for AC=%d",
                    __FUNCTION__, acType );
      }

      // notify TL to enable trigger frames if necessary
      hdd_wmm_enable_tl_uapsd(pQosContext);

      break;

   case SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY:
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup is complete (U-APSD previously), notify TL",
                __FUNCTION__);
#endif // HDD_WMM_DEBUG

      // update state
      pAc->wmmAcAccessGranted = VOS_TRUE;
      pAc->wmmAcAccessPending = VOS_FALSE;

      // notify TL that packets are pending
      status = WLANTL_STAPktPending( pAdapter->pvosContext,
                                     pAdapter->conn_info.staId[0],
                                     acType );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to signal TL for AC=%d",
                    __FUNCTION__, acType );
      }

      break;

   case SME_QOS_STATUS_SETUP_FAILURE_RSP:
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup failed, notify TL",
                __FUNCTION__);
      // QoS setup failed
      // we note the failure, but we also mark access as granted so that
      // the packets will flow.  Note that the MAC will "do the right thing"
      pAc->wmmAcAccessFailed = VOS_TRUE;
      pAc->wmmAcAccessGranted = VOS_TRUE;
      pAc->wmmAcAccessPending = VOS_FALSE;
      status = WLANTL_STAPktPending( pAdapter->pvosContext,
                                     pAdapter->conn_info.staId[0],
                                     acType );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to signal TL for AC=%d",
                    __FUNCTION__, acType );
      }
      break;

   case SME_QOS_STATUS_SETUP_MODIFIED_IND:
      if (pCurrentQosInfo)
      {
         // update the TSPEC
         pAc->wmmAcTspecValid = VOS_TRUE;
         memcpy(&pAc->wmmAcTspecInfo,
                pCurrentQosInfo,
                sizeof(pAc->wmmAcTspecInfo));

         // need to tell TL to update its UAPSD handling
         hdd_wmm_enable_tl_uapsd(pQosContext);
      }
      break;

   case SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING:
      // nothing to do for now
      break;

   case SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_SET_FAILED:
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup successful but U-APSD failed, notify TL",
                __FUNCTION__);

      // QoS setup was successful but setting U=APSD failed
      // Since the OTA part of the request was successful, we don't mark
      // this as a failure.
      // the packets will flow.  Note that the MAC will "do the right thing"
      pAc->wmmAcAccessFailed = VOS_FALSE;
      pAc->wmmAcAccessGranted = VOS_TRUE;
      pAc->wmmAcAccessPending = VOS_FALSE;
      status = WLANTL_STAPktPending( pAdapter->pvosContext,
                                     pAdapter->conn_info.staId[0],
                                     acType );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to signal TL for AC=%d",
                    __FUNCTION__, acType );
      }

      // Since U-APSD portion failed disabled trigger frame generation
      hdd_wmm_disable_tl_uapsd(pQosContext);

      break;

   case SME_QOS_STATUS_RELEASE_QOS_LOST_IND:
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: QOS Lost indication received",
                __FUNCTION__);

      // current TSPEC is no longer valid
      pAc->wmmAcTspecValid = VOS_FALSE;

      // need to tell TL to update its UAPSD handling
      hdd_wmm_disable_tl_uapsd(pQosContext);

      // we no longer have access granted
      pAc->wmmAcAccessGranted = VOS_FALSE;
      pAc->wmmAcAccessFailed = VOS_FALSE;

      break;

   case SME_QOS_STATUS_OUT_OF_APSD_POWER_MODE_IND:
      // need to tell TL to stop trigger frame generation
      hdd_wmm_disable_tl_uapsd(pQosContext);
      break;

   case SME_QOS_STATUS_INTO_APSD_POWER_MODE_IND:
      // need to tell TL to start sending trigger frames again 
      hdd_wmm_enable_tl_uapsd(pQosContext);
      break;

   default:
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: unexpected SME Status=%d\n", 
                 __FUNCTION__, smeStatus );
      VOS_ASSERT(0);
   }

   return VOS_STATUS_SUCCESS;
}


/**============================================================================
  @brief hdd_wmm_do_implicit_qos() - Function which will attempt to setup
  QoS for any AC requiring it

  @param work     : [in]  pointer to work structure

  @return         : void
  ===========================================================================*/
static void hdd_wmm_do_implicit_qos(struct work_struct *work)
{
   hdd_wmm_qos_context_t* pQosContext =
      container_of(work, hdd_wmm_qos_context_t, wmmAcSetupImplicitQos);
   hdd_adapter_t* pAdapter = pQosContext->pAdapter;
   WLANTL_ACEnumType acType = pQosContext->acType;
   hdd_wmm_ac_status_t *pAc = &pAdapter->hddWmmStatus.wmmAcStatus[acType];
   VOS_STATUS status;
   sme_QosStatusType smeStatus;
   sme_QosWmmTspecInfo qosInfo;
   v_U32_t flowId;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: pQosContext %p pAdapter %p acType %d",
             __FUNCTION__, pQosContext, pAdapter, acType);
#endif // HDD_WMM_DEBUG

   if (!pAc->wmmAcAccessNeeded)
   {
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: AC %d doesn't need service",
                __FUNCTION__, acType);
      return;
   }

   pAc->wmmAcAccessPending = VOS_TRUE;
   pAc->wmmAcAccessNeeded = VOS_FALSE;

   memset(&qosInfo, 0, sizeof(qosInfo));

   switch (acType)
   {
   case WLANTL_AC_VO:
      qosInfo.ts_info.up = SME_QOS_WMM_UP_VO;
      qosInfo.ts_info.psb = pAdapter->cfg_ini->UapsdMask & 0x01;
      qosInfo.ts_info.direction = pAdapter->cfg_ini->InfraDirAcVo;
      qosInfo.ts_info.tid = 255;
      qosInfo.mean_data_rate = pAdapter->cfg_ini->InfraMeanDataRateAcVo;
      qosInfo.min_phy_rate = pAdapter->cfg_ini->InfraMinPhyRateAcVo;
      qosInfo.min_service_interval = pAdapter->cfg_ini->InfraUapsdVoSrvIntv;
      qosInfo.nominal_msdu_size = pAdapter->cfg_ini->InfraNomMsduSizeAcVo;
      qosInfo.surplus_bw_allowance = pAdapter->cfg_ini->InfraSbaAcVo;
      qosInfo.suspension_interval = pAdapter->cfg_ini->InfraUapsdVoSuspIntv;
      break;
   case WLANTL_AC_VI:
      qosInfo.ts_info.up = SME_QOS_WMM_UP_VI;
      qosInfo.ts_info.psb = pAdapter->cfg_ini->UapsdMask & 0x02;
      qosInfo.ts_info.direction = pAdapter->cfg_ini->InfraDirAcVi;
      qosInfo.ts_info.tid = 255;
      qosInfo.mean_data_rate = pAdapter->cfg_ini->InfraMeanDataRateAcVi;
      qosInfo.min_phy_rate = pAdapter->cfg_ini->InfraMinPhyRateAcVi;
      qosInfo.min_service_interval = pAdapter->cfg_ini->InfraUapsdViSrvIntv;
      qosInfo.nominal_msdu_size = pAdapter->cfg_ini->InfraNomMsduSizeAcVi;
      qosInfo.surplus_bw_allowance = pAdapter->cfg_ini->InfraSbaAcVi;
      qosInfo.suspension_interval = pAdapter->cfg_ini->InfraUapsdViSuspIntv;
      break;
   default:
   case WLANTL_AC_BE:
      qosInfo.ts_info.up = SME_QOS_WMM_UP_BE;
      qosInfo.ts_info.psb = pAdapter->cfg_ini->UapsdMask & 0x08;
      qosInfo.ts_info.direction = pAdapter->cfg_ini->InfraDirAcBe;
      qosInfo.ts_info.tid = 255;
      qosInfo.mean_data_rate = pAdapter->cfg_ini->InfraMeanDataRateAcBe;
      qosInfo.min_phy_rate = pAdapter->cfg_ini->InfraMinPhyRateAcBe;
      qosInfo.min_service_interval = pAdapter->cfg_ini->InfraUapsdBeSrvIntv;
      qosInfo.nominal_msdu_size = pAdapter->cfg_ini->InfraNomMsduSizeAcBe;
      qosInfo.surplus_bw_allowance = pAdapter->cfg_ini->InfraSbaAcBe;
      qosInfo.suspension_interval = pAdapter->cfg_ini->InfraUapsdBeSuspIntv;
      break;
   case WLANTL_AC_BK:
      qosInfo.ts_info.up = SME_QOS_WMM_UP_BK;
      qosInfo.ts_info.psb = pAdapter->cfg_ini->UapsdMask & 0x04;
      qosInfo.ts_info.direction = pAdapter->cfg_ini->InfraDirAcBk;
      qosInfo.ts_info.tid = 255;
      qosInfo.mean_data_rate = pAdapter->cfg_ini->InfraMeanDataRateAcBk;
      qosInfo.min_phy_rate = pAdapter->cfg_ini->InfraMinPhyRateAcBk;
      qosInfo.min_service_interval = pAdapter->cfg_ini->InfraUapsdBkSrvIntv;
      qosInfo.nominal_msdu_size = pAdapter->cfg_ini->InfraNomMsduSizeAcBk;
      qosInfo.surplus_bw_allowance = pAdapter->cfg_ini->InfraSbaAcBk;
      qosInfo.suspension_interval = pAdapter->cfg_ini->InfraUapsdBkSuspIntv;
      break;
   }


   smeStatus = sme_QosSetupReq(pAdapter->hHal,
                               &qosInfo,
                               hdd_wmm_sme_callback,
                               &pAdapter->hddWmmStatus.wmmQosContext[acType],
                               qosInfo.ts_info.up,
                               &flowId);

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: sme_QosSetupReq returned %d flowid %d",
             __FUNCTION__, smeStatus, flowId);
#endif // HDD_WMM_DEBUG

   // need to check the return values and act appropriately
   switch (smeStatus)
   {
   case SME_QOS_STATUS_SETUP_REQ_PENDING_RSP:
   case SME_QOS_STATUS_SETUP_SUCCESS_IND_APSD_PENDING:
      // setup is pending, so no more work to do now.
      // all further work will be done in hdd_wmm_sme_callback()
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup is pending, no further work",
                __FUNCTION__);
#endif // HDD_WMM_DEBUG

      break;


   case SME_QOS_STATUS_SETUP_FAILURE_RSP:
      // we can't tell the difference between when a request fails because
      // AP rejected it versus when SME encounterd an internal error
      // regardless, fall through and start packets flowing
   case SME_QOS_STATUS_SETUP_SUCCESS_NO_ACM_NO_APSD_RSP:
      // no ACM in effect, no need to setup U-APSD
   case SME_QOS_STATUS_SETUP_SUCCESS_APSD_SET_ALREADY:
      // no ACM in effect, U-APSD is desired but was already setup

      // for these cases everything is already setup so we can
      // signal TL that it has work to do
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Setup is complete, notify TL",
                __FUNCTION__);
#endif // HDD_WMM_DEBUG

      pAc->wmmAcAccessGranted = VOS_TRUE;
      pAc->wmmAcAccessPending = VOS_FALSE;

      status = WLANTL_STAPktPending( pAdapter->pvosContext,
                                     pAdapter->conn_info.staId[0],
                                     acType );      

      if ( !VOS_IS_STATUS_SUCCESS( status ) )
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: Failed to signal TL for AC=%d",
                    __FUNCTION__, acType );
      }

      break;


   default:
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: unexpected SME Status=%d\n", 
                 __FUNCTION__, smeStatus );
      VOS_ASSERT(0);
   }

}



/**============================================================================
  @brief hdd_wmm_init() - Function which will initialize the WMM configuation
  and status to an initial state.  The configuration can later be overwritten
  via application APIs

  @param pAdapter : [in]  pointer to adapter context

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure

  ===========================================================================*/
VOS_STATUS hdd_wmm_init ( hdd_adapter_t* pAdapter )
{
   hdd_wmm_qos_context_t *pQosContext;
   hdd_wmm_ac_status_t *pAcStatus;
   WLANTL_ACEnumType acType;
   v_U8_t dscp;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Entered", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   pAdapter->hddWmmStatus.wmmQap = VOS_FALSE;
   for (acType = 0; acType < WLANTL_MAX_AC; acType++)
   {
      pQosContext = &pAdapter->hddWmmStatus.wmmQosContext[acType];
      pQosContext->acType = acType;
      pQosContext->pAdapter = pAdapter;
      pQosContext->qosFlowId = 0;
      INIT_WORK(&pQosContext->wmmAcSetupImplicitQos,
                hdd_wmm_do_implicit_qos);

      pAcStatus = &pAdapter->hddWmmStatus.wmmAcStatus[acType];
      pAcStatus->wmmAcAccessNeeded = VOS_FALSE;
      pAcStatus->wmmAcAccessPending = VOS_FALSE;
      pAcStatus->wmmAcAccessFailed = VOS_FALSE;
      pAcStatus->wmmAcAccessGranted = VOS_FALSE;
      pAcStatus->wmmAcTspecValid = VOS_FALSE;
      pAcStatus->wmmAcUapsdInfoValid = VOS_FALSE;
   }

   // DSCP to User Priority Lookup Table
   for (dscp = 0; dscp <= WLAN_HDD_MAX_DSCP; dscp++)
   {
      hddWmmDscpToUpMap[dscp] = SME_QOS_WMM_UP_BE;
   }
   hddWmmDscpToUpMap[8]  = SME_QOS_WMM_UP_BK; 
   hddWmmDscpToUpMap[16] = SME_QOS_WMM_UP_RESV; 
   hddWmmDscpToUpMap[24] = SME_QOS_WMM_UP_EE; 
   hddWmmDscpToUpMap[32] = SME_QOS_WMM_UP_CL; 
   hddWmmDscpToUpMap[40] = SME_QOS_WMM_UP_VI; 
   hddWmmDscpToUpMap[48] = SME_QOS_WMM_UP_VO; 
   hddWmmDscpToUpMap[56] = SME_QOS_WMM_UP_NC; 

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_wmm_close() - Function which will perform any necessary work to
  to clean up the WMM functionality prior to the kernel module unload

  @param pAdapter : [in]  pointer to adapter context

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure

  ===========================================================================*/
VOS_STATUS hdd_wmm_close ( hdd_adapter_t* pAdapter )
{
#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Entered", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   // need to make sure all of our scheduled work has completed
   flush_scheduled_work();

   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_wmm_classify_pkt() - Function which will classify an OS packet
  into a WMM AC based on either 802.1Q or DSCP

  @param pAdapter : [in]  pointer to adapter context  
  @param skb      : [in]  pointer to OS packet (sk_buff) 
  @param pAcType  : [out] pointer to WMM AC type of OS packet

  @return         : FALSE if any errors encountered
                  : TRUE otherwise
  ===========================================================================*/
v_BOOL_t hdd_wmm_classify_pkt ( hdd_adapter_t* pAdapter,
                                struct sk_buff *skb,
                                WLANTL_ACEnumType* pAcType,
                                sme_QosWmmUpType *pUserPri)
{
   unsigned char * pPkt;
   union generic_ethhdr *pHdr;
   struct iphdr *pIpHdr;
   unsigned char tos;
   unsigned char dscp;
   sme_QosWmmUpType userPri;
   WLANTL_ACEnumType acType;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: Entered", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   pPkt = skb->data;
   pHdr = (union generic_ethhdr *)pPkt;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: proto/length is 0x%04x", 
             __FUNCTION__, pHdr->eth_II.h_proto);
#endif // HDD_WMM_DEBUG

   if (HDD_WMM_CLASSIFICATION_DSCP == pAdapter->cfg_ini->PktClassificationBasis)
   {
      if (pHdr->eth_II.h_proto == htons(ETH_P_IP))
      {
         // case 1: Ethernet II IP packet
         pIpHdr = (struct iphdr *)&pPkt[sizeof(pHdr->eth_II)];
         tos = pIpHdr->tos;
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "%s: Ethernet II IP Packet, tos is %d",
                   __FUNCTION__, tos);
#endif // HDD_WMM_DEBUG

      }
      else if ((ntohs(pHdr->eth_II.h_proto) < WLAN_MIN_PROTO) &&
               (pHdr->eth_8023.h_snap.dsap == WLAN_SNAP_DSAP) &&
               (pHdr->eth_8023.h_snap.ssap == WLAN_SNAP_SSAP) &&
               (pHdr->eth_8023.h_snap.ctrl == WLAN_SNAP_CTRL) &&
               (pHdr->eth_8023.h_proto == htons(ETH_P_IP)))
      {
         // case 2: 802.3 LLC/SNAP IP packet
         pIpHdr = (struct iphdr *)&pPkt[sizeof(pHdr->eth_8023)];
         tos = pIpHdr->tos;
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "%s: 802.3 LLC/SNAP IP Packet, tos is %d",
                   __FUNCTION__, tos);
#endif // HDD_WMM_DEBUG
      }
      else if (pHdr->eth_II.h_proto == htons(ETH_P_8021Q))
      {
         // VLAN tagged

         if (pHdr->eth_IIv.h_vlan_encapsulated_proto == htons(ETH_P_IP))
         {
            // case 3: Ethernet II vlan-tagged IP packet
            pIpHdr = (struct iphdr *)&pPkt[sizeof(pHdr->eth_IIv)];
            tos = pIpHdr->tos;
#ifdef HDD_WMM_DEBUG
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: Ethernet II VLAN tagged IP Packet, tos is %d",
                      __FUNCTION__, tos);
#endif // HDD_WMM_DEBUG
         }
         else if ((ntohs(pHdr->eth_IIv.h_vlan_encapsulated_proto) < WLAN_MIN_PROTO) &&
                  (pHdr->eth_8023v.h_snap.dsap == WLAN_SNAP_DSAP) &&
                  (pHdr->eth_8023v.h_snap.ssap == WLAN_SNAP_SSAP) &&
                  (pHdr->eth_8023v.h_snap.ctrl == WLAN_SNAP_CTRL) &&
                  (pHdr->eth_8023v.h_proto == htons(ETH_P_IP)))
         {
            // case 4: 802.3 LLC/SNAP vlan-tagged IP packet
            pIpHdr = (struct iphdr *)&pPkt[sizeof(pHdr->eth_8023v)];
            tos = pIpHdr->tos;
#ifdef HDD_WMM_DEBUG
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                      "%s: 802.3 LLC/SNAP VLAN tagged IP Packet, tos is %d",
                      __FUNCTION__, tos);
#endif // HDD_WMM_DEBUG
         }
         else
         {
            // default
#ifdef HDD_WMM_DEBUG
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: VLAN tagged Unhandled Protocol, using default tos",
                      __FUNCTION__);
#endif // HDD_WMM_DEBUG
            tos = 0;
         }
      }
      else
      {
         // default
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: Unhandled Protocol, using default tos",
                   __FUNCTION__);
#endif // HDD_WMM_DEBUG
         tos = 0;
      }

      dscp = (tos>>2) & 0x3f;
      userPri = hddWmmDscpToUpMap[dscp];

#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: tos is %d, dscp is %d, up is %d",
                __FUNCTION__, tos, dscp, userPri);
#endif // HDD_WMM_DEBUG

   }
   else if (HDD_WMM_CLASSIFICATION_802_1Q == pAdapter->cfg_ini->PktClassificationBasis)
   {
      if (pHdr->eth_IIv.h_vlan_proto == htons(ETH_P_8021Q))
      {
         // VLAN tagged
         userPri = (ntohs(pHdr->eth_IIv.h_vlan_TCI)>>13) & 0x7;
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   "%s: Tagged frame, UP is %d",
                   __FUNCTION__, userPri);
#endif // HDD_WMM_DEBUG
      }
      else
      {
         // not VLAN tagged, use default
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: Untagged frame, using default UP",
                   __FUNCTION__);
#endif // HDD_WMM_DEBUG
         userPri = SME_QOS_WMM_UP_BE;
      }
   }
   else
   {
      // default
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Unknown classification scheme, using default UP",
                __FUNCTION__);
#endif // HDD_WMM_DEBUG
      userPri = SME_QOS_WMM_UP_BE;
   }

   acType = hddWmmUpToAcMap[userPri];

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
             "%s: UP is %d, AC is %d",
             __FUNCTION__, userPri, acType);
#endif // HDD_WMM_DEBUG

   *pUserPri = userPri;
   *pAcType = acType;

   return VOS_TRUE;
}


/**============================================================================
  @brief hdd_wmm_acquire_access() - Function which will attempt to acquire
  admittance for a WMM AC 

  @param pAdapter : [in]  pointer to adapter context  
  @param acType   : [in]  WMM AC type of OS packet
  @param pGranted : [out] pointer to boolean flag when indicates if access
                          has been granted or not

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure
  ===========================================================================*/
VOS_STATUS hdd_wmm_acquire_access( hdd_adapter_t* pAdapter,
                                   WLANTL_ACEnumType acType,
                                   v_BOOL_t * pGranted )
{
#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Entered for AC %d", __FUNCTION__, acType);
#endif // HDD_WMM_DEBUG

   if (!hdd_wmm_is_active(pAdapter) || !pAdapter->cfg_ini->bImplicitQosEnabled)
   {
      // either we don't want QoS or the AP doesn't support QoS
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: QoS not configured on both ends ", __FUNCTION__);
#endif // HDD_WMM_DEBUG

      pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessGranted = VOS_TRUE;
      *pGranted = VOS_TRUE;
      return VOS_STATUS_SUCCESS;
   }

   // do we already have an implicit QoS request pending for this AC?
   if ((pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessNeeded) ||
       (pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessPending))
   {
      // request already pending so we need to wait for that response
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Implicit QoS for TL AC %d already scheduled",
                __FUNCTION__, acType);
#endif // HDD_WMM_DEBUG

      *pGranted = VOS_FALSE;
      return VOS_STATUS_SUCCESS;
   }

   // did we already fail to establish implicit QoS for this AC?
   if (pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessFailed)
   {
      // request previously failed
      // allow access, but we'll be downgraded
#ifdef HDD_WMM_DEBUG
      VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s: Implicit QoS for TL AC %d previously failed",
                __FUNCTION__, acType);
#endif // HDD_WMM_DEBUG

      pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessGranted = VOS_TRUE;
      *pGranted = VOS_TRUE;
      return VOS_STATUS_SUCCESS;
   }

   // we need to establish implicit QoS
#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: Schedule implicit QoS for TL AC %d", __FUNCTION__, acType);
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: pAdapter is %p", __FUNCTION__, pAdapter);
#endif // HDD_WMM_DEBUG

   pAdapter->hddWmmStatus.wmmAcStatus[acType].wmmAcAccessNeeded = VOS_TRUE;
   schedule_work(&pAdapter->hddWmmStatus.wmmQosContext[acType].wmmAcSetupImplicitQos);

   // caller will need to wait until the work takes place and
   // TSPEC negotiation completes
   *pGranted = VOS_FALSE;
   return VOS_STATUS_SUCCESS;
}

/**============================================================================
  @brief hdd_wmm_assoc() - Function which will handle the housekeeping
  required by WMM when association takes place

  @param pAdapter : [in]  pointer to adapter context
  @param pRoamInfo: [in]  pointer to roam information
  @param eBssType : [in]  type of BSS

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure
  ===========================================================================*/
VOS_STATUS hdd_wmm_assoc( hdd_adapter_t* pAdapter,
                          tCsrRoamInfo *pRoamInfo,
                          eCsrRoamBssType eBssType )
{
   tANI_U8 uapsdMask;
   VOS_STATUS status;

   // when we associate we need to notify TL if it needs to enable
   // UAPSD for any access categories

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
             "%s: Entered", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   // get the negotiated UAPSD Mask
   uapsdMask = pRoamInfo->u.pConnectedProfile->modifyProfileFields.uapsd_mask;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
             "%s: U-APSD mask is 0x%02x", __FUNCTION__, (int) uapsdMask);
#endif // HDD_WMM_DEBUG

   if (uapsdMask & HDD_AC_VO)
   {
      status = WLANTL_EnableUAPSDForAC( pAdapter->pvosContext, 
                                        pAdapter->conn_info.staId[0],
                                        WLANTL_AC_VO,
                                        7,
                                        7,
                                        pAdapter->cfg_ini->InfraUapsdVoSrvIntv,
                                        pAdapter->cfg_ini->InfraUapsdVoSuspIntv,
                                        WLANTL_BI_DIR );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status ));
   }

   if (uapsdMask & HDD_AC_VI)
   {
      status = WLANTL_EnableUAPSDForAC( pAdapter->pvosContext, 
                                        pAdapter->conn_info.staId[0],
                                        WLANTL_AC_VI,
                                        5,
                                        5,
                                        pAdapter->cfg_ini->InfraUapsdViSrvIntv,
                                        pAdapter->cfg_ini->InfraUapsdViSuspIntv,
                                        WLANTL_BI_DIR );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status ));
   }

   if (uapsdMask & HDD_AC_BK)
   {
      status = WLANTL_EnableUAPSDForAC( pAdapter->pvosContext, 
                                        pAdapter->conn_info.staId[0],
                                        WLANTL_AC_BK,
                                        2,
                                        2,
                                        pAdapter->cfg_ini->InfraUapsdBkSrvIntv,
                                        pAdapter->cfg_ini->InfraUapsdBkSuspIntv,
                                        WLANTL_BI_DIR );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status ));
   }

   if (uapsdMask & HDD_AC_BE)
   {
      status = WLANTL_EnableUAPSDForAC( pAdapter->pvosContext, 
                                        pAdapter->conn_info.staId[0],
                                        WLANTL_AC_BE,
                                        3,
                                        3,
                                        pAdapter->cfg_ini->InfraUapsdBeSrvIntv,
                                        pAdapter->cfg_ini->InfraUapsdBeSuspIntv,
                                        WLANTL_BI_DIR );

      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status ));
   }

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
             "%s: Exiting", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   return VOS_STATUS_SUCCESS;
}



static const v_U8_t acmMaskBit[WLANTL_MAX_AC] =
   {
      0x4, /* WLANTL_AC_BK */
      0x8, /* WLANTL_AC_BE */
      0x2, /* WLANTL_AC_VI */
      0x1  /* WLANTL_AC_VO */
   };

/**============================================================================
  @brief hdd_wmm_connect() - Function which will handle the housekeeping
  required by WMM when a connection is established

  @param pAdapter : [in]  pointer to adapter context  
  @param pRoamInfo: [in]  pointer to roam information
  @param eBssType : [in]  type of BSS

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure
  ===========================================================================*/
VOS_STATUS hdd_wmm_connect( hdd_adapter_t* pAdapter,
                            tCsrRoamInfo *pRoamInfo,
                            eCsrRoamBssType eBssType )
{
   int ac;
   v_BOOL_t qap;
   v_U8_t acmMask;

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
             "%s: Entered", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   if ((eCSR_BSS_TYPE_INFRASTRUCTURE == eBssType) &&
       pRoamInfo &&
       pRoamInfo->u.pConnectedProfile)
   {
      qap = pRoamInfo->u.pConnectedProfile->qap;
      acmMask = pRoamInfo->u.pConnectedProfile->acm_mask;
   }
   else
   {
      qap = VOS_TRUE;
      acmMask = 0x0;
   }

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
             "%s: qap is %d, acmMask is 0x%x",
             __FUNCTION__, qap, acmMask);
#endif // HDD_WMM_DEBUG

   pAdapter->hddWmmStatus.wmmQap = qap;

   for (ac = 0; ac < WLANTL_MAX_AC; ac++)
   {
      if (qap &&
          (HDD_WMM_USER_MODE_NO_QOS != pAdapter->cfg_ini->WmmMode) &&
          (acmMask & acmMaskBit[ac]))
      {
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: ac %d on",
                   __FUNCTION__, ac);
#endif // HDD_WMM_DEBUG

         // no access allowed yet so nothing to do for now
      }
      else
      {
#ifdef HDD_WMM_DEBUG
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: ac %d off",
                   __FUNCTION__, ac);
#endif // HDD_WMM_DEBUG
         // no ACM so access is allowed
         pAdapter->hddWmmStatus.wmmAcStatus[ac].wmmAcAccessGranted = VOS_TRUE;
      }

   }

#ifdef HDD_WMM_DEBUG
   VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
             "%s: Exiting", __FUNCTION__);
#endif // HDD_WMM_DEBUG

   return VOS_STATUS_SUCCESS;
}



/**============================================================================
  @brief hdd_wmm_get_uapsd_mask() - Function which will calculate the
  initial value of the UAPSD mask based upon the device configuration

  @param pAdapter  : [in]  pointer to adapter context
  @param pUapsdMask: [in]  pointer to where the UAPSD Mask is to be stored

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure
  ===========================================================================*/
VOS_STATUS hdd_wmm_get_uapsd_mask( hdd_adapter_t* pAdapter,
                                   tANI_U8 *pUapsdMask )
{
   tANI_U8 uapsdMask;

   if (HDD_WMM_USER_MODE_NO_QOS == pAdapter->cfg_ini->WmmMode)
   {
      // no QOS then no UAPSD
      uapsdMask = 0;
   }
   else
   {
      // start with the default mask
      uapsdMask = pAdapter->cfg_ini->UapsdMask;

      // disable UAPSD for any ACs with a 0 Service Interval
      if( pAdapter->cfg_ini->InfraUapsdVoSrvIntv == 0 )
      {
         uapsdMask &= ~HDD_AC_VO;
      }

      if( pAdapter->cfg_ini->InfraUapsdViSrvIntv == 0 )
      {
         uapsdMask &= ~HDD_AC_VI;
      }

      if( pAdapter->cfg_ini->InfraUapsdBkSrvIntv == 0 )
      {
         uapsdMask &= ~HDD_AC_BK;
      }

      if( pAdapter->cfg_ini->InfraUapsdBeSrvIntv == 0 )
      {
         uapsdMask &= ~HDD_AC_BE;
      }
   }

   // return calculated mask
   *pUapsdMask = uapsdMask;
   return VOS_STATUS_SUCCESS;
}


/**============================================================================
  @brief hdd_wmm_is_active() - Function which will determine if WMM is
  active on the current connection

  @param pAdapter  : [in]  pointer to adapter context

  @return         : VOS_TRUE if WMM is enabled
                  : VOS_FALSE if WMM is not enabled
  ===========================================================================*/
v_BOOL_t hdd_wmm_is_active( hdd_adapter_t* pAdapter )
{
   if ((HDD_WMM_USER_MODE_NO_QOS == pAdapter->cfg_ini->WmmMode) ||
       (!pAdapter->hddWmmStatus.wmmQap))
   {
      return VOS_FALSE;
   }
   else
   {
      return VOS_TRUE;
   }
}
