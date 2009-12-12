#ifndef _WLAN_HDD_WMM_H
#define _WLAN_HDD_WMM_H
/*============================================================================
  @file wlan_hdd_wmm.h

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
/* $Header$ */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <linux/workqueue.h>
#include <wlan_hdd_main.h>
#include <wlan_qct_tl.h>
#include <sme_QosApi.h>

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define HDD_WMM_DEBUG 1

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/*! @brief types of classification supported
*/
typedef enum
{
   HDD_WMM_CLASSIFICATION_DSCP = 0,
   HDD_WMM_CLASSIFICATION_802_1Q = 1

} hdd_wmm_classification_t;

/*! @brief UAPSD state
*/
typedef enum
{
   HDD_WMM_NON_UAPSD = 0,
   HDD_WMM_UAPSD = 1

} hdd_wmm_uapsd_state_t;


typedef enum
{
   //STA can associate with any AP, & HDD looks at the SME notification after
   // association to find out if associated with QAP and acts accordingly
   HDD_WMM_USER_MODE_AUTO = 0,
   //SME will add the extra logic to make sure STA associates with a QAP only
   HDD_WMM_USER_MODE_QBSS_ONLY = 1,
   //SME will not join a QoS AP, unless the phy mode setting says "Auto". In
   // that case, STA is free to join 11n AP. Although from HDD point of view,
   // it will not be doing any packet classifications
   HDD_WMM_USER_MODE_NO_QOS = 2,

} hdd_wmm_user_mode_t;


/*! @brief WMM related per-AC TSpec configuration info
*/
typedef struct
{
   sme_QosWmmUpType    wmmAcUserPriority;
   v_U8_t              wmmAcPowerSaveEnabled;
   sme_QosWmmDirType   wmmAcDirection;
   v_U32_t             wmmAcMeanDataRate;
   v_U32_t             wmmAcMinPhyRate;
   v_U32_t             wmmAcMinServiceInterval;
   v_U16_t             wmmAcNominalMsduSize;
   v_U16_t             wmmAcSurplusBwAllowance;
   v_U32_t             wmmAcSuspensionInterval;
} hdd_wmm_tspec_cfg_t;


/*! @brief WMM Qos instance control block
*/
typedef struct
{
   v_U32_t                      qosFlowId;
   hdd_adapter_t*               pAdapter;
   WLANTL_ACEnumType            acType;
   struct work_struct           wmmAcSetupImplicitQos;
} hdd_wmm_qos_context_t;

/*! @brief WMM related per-AC state & status info
*/
typedef struct
{
   v_BOOL_t                     wmmAcAccessNeeded;
   v_BOOL_t                     wmmAcAccessPending;
   v_BOOL_t                     wmmAcAccessFailed;
   v_BOOL_t                     wmmAcAccessGranted;
   v_BOOL_t                     wmmAcTspecValid;
   v_BOOL_t                     wmmAcUapsdInfoValid;
   sme_QosWmmTspecInfo          wmmAcTspecInfo;
   v_U32_t                      wmmAcUapsdServiceInterval;
   v_U32_t                      wmmAcUapsdSuspensionInterval;
   sme_QosWmmDirType            wmmAcUapsdDirection;
} hdd_wmm_ac_status_t;

/*! @brief WMM state & status info
*/
typedef struct
{
   hdd_wmm_ac_status_t          wmmAcStatus[WLANTL_MAX_AC];
   hdd_wmm_qos_context_t        wmmQosContext[WLANTL_MAX_AC];
   v_BOOL_t                     wmmQap;
} hdd_wmm_status_t;

/**============================================================================
  @brief hdd_wmm_init() - Function which will initialize the WMM configuation
  and status to an initial state.  The configuration can later be overwritten
  via application APIs

  @param pAdapter : [in]  pointer to adapter context

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure

  ===========================================================================*/
VOS_STATUS hdd_wmm_init ( hdd_adapter_t* pAdapter );

/**============================================================================
  @brief hdd_wmm_close() - Function which will perform any necessary work to
  to clean up the WMM functionality prior to the kernel module unload

  @param pAdapter : [in]  pointer to adapter context

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure

  ===========================================================================*/
VOS_STATUS hdd_wmm_close ( hdd_adapter_t* pAdapter );

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
                                sme_QosWmmUpType* pUserPri);


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
                                   v_BOOL_t * pGranted );

/**============================================================================
  @brief hdd_wmm_connect() - Function which will handle the housekeeping
  required by WMM when a connection is established

  @param pAdapter : [in]  pointer to adapter context
  @param pRoamInfo: [in]  pointer to raom information
  @param eBssType : [in]  type of BSS

  @return         : VOS_STATUS_SUCCESS if succssful
                  : other values if failure
  ===========================================================================*/
VOS_STATUS hdd_wmm_connect( hdd_adapter_t* pAdapter,
                            tCsrRoamInfo *pRoamInfo,
                            eCsrRoamBssType eBssType );

#endif /* #ifndef _WLAN_HDD_WMM_H */
