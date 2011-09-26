/*
 * Qualcomm Inc Inc proprietary. All rights reserved.
 * halMsg.c:  Provides STA entry specific API to PEs
 * Author:    Arul Vasantharaj
 * Date:      05/14/2008
 *
 * --------------------------------------------------------------------------
 */
#include "halInternal.h"
#include "halMsgApi.h"
#include "halMsgApiInternal.h"
#include "halStaTableApi.h"
#include "halMacSecurityApi.h"
#include "halRateAdaptApi.h"
#include "halDebug.h"
#include "halBDApi.h"
#include "cfgApi.h"
#include "halMTU.h"
#include "halMacBA.h"
#include "halUtils.h"
#include "halAdaptThrsh.h"
#include "halTimer.h"
#include "ani_assert.h"
#include "halLED.h"
#include "halTLApi.h"
#include "halFw.h"

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "vos_diag_core_event.h"
#endif

#define WLAN_SOFTAP_DTIM_INTERVAL_THRESHOLD 0x2800
#define MAX_PS_MSG_RETRY_COUNT  3
#define HAL_PHY_MONITOR_TH        3
#define HAL_PHY_MONITOR_WINDOW    1

#define GET_DIFS(sifs, slot)        ( sifs + (slot*2) )
#define GET_PIFS(sifs, slot)        ( sifs + slot )
#define GET_ACKTO(sifs, slot)        ( sifs + slot )
//FIXME : This value will be provided by HW team.
#define TX_TIMESTAMP_COMPENSATION 10

//FIXME : need to be redefined.
#define HAL_BSS_BEACON_MAX_SIZE 512
#define HAL_BSS_BEACON_TX_OFFSET 1

#define HAL_BTAMP_MODE_OFF 0
#define HAL_BTAMP_MODE_ON  1
/** List of Defines for setting the nAciveChain */


#define     GET_PIF_REGINDEX(regAddr)     ((regAddr - PIF_BASE) / 4)

#define     BTCF_CONFIG_REG_VAL_MIN     ((1 << BTCF_BTCF_CONFIG_PKT_DET_DSHIFT_OFFSET) | \
                                                                              (5 << BTCF_BTCF_CONFIG_PKT_DET_RATIO_OFFSET) | \
                                                                              BTCF_BTCF_CONFIG_APB_MODE_DEFAULT | \
                                                                              BTCF_BTCF_CONFIG_FFT_TOFFSET128_DEFAULT | \
                                                                              BTCF_BTCF_CONFIG_FFT_TOFFSET64_DEFAULT)

#define     BTCF_CONFIG_REG_VAL_RX_2   ((1 << BTCF_BTCF_CONFIG_PKT_DET_DSHIFT_OFFSET) | \
                                                                               (0 << BTCF_BTCF_CONFIG_PKT_DET_RATIO_OFFSET) | \
                                                                               BTCF_BTCF_CONFIG_APB_MODE_DEFAULT | \
                                                                               BTCF_BTCF_CONFIG_FFT_TOFFSET128_DEFAULT | \
                                                                               BTCF_BTCF_CONFIG_FFT_TOFFSET64_DEFAULT)

#define     BTCF_CONFIG_REG_VAL_RX_3   ((2 << BTCF_BTCF_CONFIG_PKT_DET_DSHIFT_OFFSET) | \
                                                                              (2 << BTCF_BTCF_CONFIG_PKT_DET_RATIO_OFFSET) | \
                                                                              BTCF_BTCF_CONFIG_APB_MODE_DEFAULT | \
                                                                              BTCF_BTCF_CONFIG_FFT_TOFFSET128_DEFAULT | \
                                                                              BTCF_BTCF_CONFIG_FFT_TOFFSET64_DEFAULT)

#define   CHE_SUB_MODULE_OFF_REG_VAL_MIN (CHE_SUB_MODULE_OFF_AMIMO_DETECT_OFF_MASK | \
                                                                                          CHE_SUB_MODULE_OFF_WINDOW_OFF_DEFAULT | \
                                                                                          CHE_SUB_MODULE_OFF_TRUNCATION_OFF_DEFAULT | \
                                                                                          CHE_SUB_MODULE_OFF_BAND_EDGE_REPLACE_OFF_DEFAULT)


#define  CHE_SUB_MODULE_OFF_REG_VAL_RX_2 (CHE_SUB_MODULE_OFF_WINDOW_OFF_DEFAULT | \
                                                                                           CHE_SUB_MODULE_OFF_TRUNCATION_OFF_DEFAULT | \
                                                                                           CHE_SUB_MODULE_OFF_BAND_EDGE_REPLACE_OFF_DEFAULT)

#define  CHE_SUB_MODULE_OFF_REG_VAL_RX_3  (CHE_SUB_MODULE_OFF_WINDOW_OFF_DEFAULT | \
                                                                                            CHE_SUB_MODULE_OFF_TRUNCATION_OFF_DEFAULT | \
                                                                                            CHE_SUB_MODULE_OFF_BAND_EDGE_REPLACE_OFF_DEFAULT)


#define  AGC_ANTCONFIG_REG_VAL_MIN   ((AGC_ANTCONFIG_ANT_EN_EENABLE0 << AGC_ANTCONFIG_ANT_EN_OFFSET) | \
                                                                               (AGC_ANTCONFIG_COMMON_ANT_DEFAULT << AGC_ANTCONFIG_COMMON_ANT_OFFSET))

#define AGC_ANTCONFIG_REG_VAL_RX_2  (((AGC_ANTCONFIG_ANT_EN_EENABLE0 | AGC_ANTCONFIG_ANT_EN_EENABLE1) << AGC_ANTCONFIG_ANT_EN_OFFSET) | \
                                                                                (AGC_ANTCONFIG_COMMON_ANT_DEFAULT << AGC_ANTCONFIG_COMMON_ANT_OFFSET))

#define  AGC_ANTCONFIG_REG_VAL_RX_3 (((AGC_ANTCONFIG_ANT_EN_EENABLE0 | AGC_ANTCONFIG_ANT_EN_EENABLE1 |AGC_ANTCONFIG_ANT_EN_EENABLE2) << AGC_ANTCONFIG_ANT_EN_OFFSET) | \
                                                                               (AGC_ANTCONFIG_COMMON_ANT_DEFAULT << AGC_ANTCONFIG_COMMON_ANT_OFFSET))


#define AGC_RX_OVERRIDE_REG_VAL_MIN  (( ( (~(tANI_U32 )AGC_RX_0) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_EN_OFFSET) |  \
                                                                                    ( (((tANI_U32 )AGC_RX_0) &AGC_RX_OVERRIDE_MASK ) << AGC_RX_OVERRIDE_ENRX_VAL_OFFSET) | \
                                                                                    ( ( (~(tANI_U32 )AGC_RX_0) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_STBY_VAL_OFFSET) )



#define AGC_RX_OVERRIDE_REG_VAL_RX_2  (( ( (~(tANI_U32 )AGC_RX_0_AND_1) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_EN_OFFSET) |  \
                                                                                    ( (((tANI_U32 )AGC_RX_0_AND_1) &AGC_RX_OVERRIDE_MASK ) << AGC_RX_OVERRIDE_ENRX_VAL_OFFSET) | \
                                                                                    ( ( (~(tANI_U32 )AGC_RX_0_AND_1) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_STBY_VAL_OFFSET) )


#define AGC_RX_OVERRIDE_REG_VAL_RX_3  (( ( (~(tANI_U32 )AGC_ALL_RX) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_EN_OFFSET) |  \
                                                                                    ( (((tANI_U32 )AGC_ALL_RX) &AGC_RX_OVERRIDE_MASK ) << AGC_RX_OVERRIDE_ENRX_VAL_OFFSET) | \
                                                                                    ( ( (~(tANI_U32 )AGC_ALL_RX) &AGC_RX_OVERRIDE_MASK) << AGC_RX_OVERRIDE_STBY_VAL_OFFSET) )
#ifdef ANI_SUPPORT_SMPS

/** @brief :    This Structure stores the Register Indexes and the Values that are
  *                     needed to set the nActive chains change in softmac
  */
typedef struct sRxChainRegVal {
    tANI_U32        regIndex; /** Store the RegisterIndex*/
    tANI_U16        valMin;      /**  RegValues for 1Rx Chain */
    tANI_U16        valMax[2]; /**  RegValues of 2 and 3 RxChains */
}tRxChainRegVal;



/** Staticaly store the initial Values that are neede to set the nActive Chains
  *    when you add a new entry do update the SMAC_RXCHAINS_REGISTERS_TOTAL macro too
  */
static tRxChainRegVal  sInitRegValues[ ] = {
                                            {GET_PIF_REGINDEX(AGC_N_LISTEN_REG), 1, {2, 3}},
                                            {GET_PIF_REGINDEX(AGC_N_CAPTURE_REG), 1, {2, 3}},
                                            {GET_PIF_REGINDEX(AGC_N_MEASURE_REG), 1, {2, 3}},
                                            {GET_PIF_REGINDEX(AGC_N_ACTIVE_REG), 1, {2, 3}},

                                            {GET_PIF_REGINDEX(AGC_TH_MAXCORA_REG), 10, {10, 10}},
#ifdef FIXME_GEN5
                                            {GET_PIF_REGINDEX(AGC_TH_MAXCORDBL_REG), 250, {250, 250}},
#endif
                                            {GET_PIF_REGINDEX(AGC_C11BP_REG), 254, {254, 254}},
                                            {GET_PIF_REGINDEX(AGC_TH_POWER_CLIP_REG), 63, {63, 63}},
                                            /** If AnalogLink the value needs to be set is 63 */
#ifdef FIXME_GEN5
                                            {GET_PIF_REGINDEX(AGC_TH_RXB_PWR_LOW_REG), 28, {28, 28}},
#endif
                                            {GET_PIF_REGINDEX(AGC_TH_CW_RSSI_REG), 14, {14, 14}},

                                            {GET_PIF_REGINDEX(AGC_TH_D0_A_REG), 130, {110, 100}},
                                            {GET_PIF_REGINDEX(AGC_TH_D0_11N_REG), 100, {90, 70}},
                                            {GET_PIF_REGINDEX(AGC_TH_MAXCORAB_REG), 50, {30, 16}},
                                            {GET_PIF_REGINDEX(AGC_TH_MAXCORN_REG), 140, {120, 120}},
                                            {GET_PIF_REGINDEX(AGC_TH_D0_B_REG), 60, {55, 40}},
                                            {GET_PIF_REGINDEX(AGC_TH_D0_B_TF_EST_REG), 40, {40, 30}},
                                            {GET_PIF_REGINDEX(AGC_TH_SIGNAL_LOW_REG), 25, {50, 70}},
                                            {GET_PIF_REGINDEX(AGC_TH_SIGNAL_HIGH_REG), 180, {180, 110}},

                                            {GET_PIF_REGINDEX(BTCF_BTCF_CONFIG_REG), BTCF_CONFIG_REG_VAL_MIN,  \
                                                                                        {BTCF_CONFIG_REG_VAL_RX_2, BTCF_CONFIG_REG_VAL_RX_3}},

                                            {GET_PIF_REGINDEX(RACTL_DAGC_TGTBO_REG), 30, {32, 32}},
#ifdef FIXME_GEN5
                                            {GET_PIF_REGINDEX(CHE_SUB_MODULE_OFF_REG), CHE_SUB_MODULE_OFF_REG_VAL_MIN,  \
                                                                                        {CHE_SUB_MODULE_OFF_REG_VAL_RX_2, CHE_SUB_MODULE_OFF_REG_VAL_RX_3}},
                                            {GET_PIF_REGINDEX(AGC_ANTCONFIG_REG), AGC_ANTCONFIG_REG_VAL_MIN,  \
                                                                                        {AGC_ANTCONFIG_REG_VAL_RX_2, AGC_ANTCONFIG_REG_VAL_RX_3}},
#endif

                                            {GET_PIF_REGINDEX(AGC_RX_OVERRIDE_REG), AGC_RX_OVERRIDE_REG_VAL_MIN, \
                                                                                        {AGC_RX_OVERRIDE_REG_VAL_RX_2, AGC_RX_OVERRIDE_REG_VAL_RX_3}},

                                            {GET_PIF_REGINDEX(AGC_CONFIG_XBAR_REG), 0x10, {0x10, 0x10}}
                                   };
#endif
static void halMsg_storeBssLinkState(tpAniSirGlobal pMac, tSirLinkState state, 
        tANI_U8 bssIdx);
/* ------------------------------------------
 * FUNCTION:  __halMsg_update11bCoexist
 *
 * NOTE: Could only be called in ADD BSS and update beacon param functions
 * ------------------------------------------
 */

static void
__halMsg_update11bCoexist(tpAniSirGlobal pMac, tANI_U32 llbCoexist)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    if(pMac->hal.currentRfBand == eRF_BAND_2_4_GHZ){
        if(llbCoexist == 1){
            // Set the TPE reg to use the 11b rates for control frames
            halTpe_Set11gProtectionCntrlIndex(pMac, TRUE);
            /* Add for b/g interop Wifi test */
            pFwConfig->ucUcastDataRecepTimeoutMs = 80;
            pFwConfig->nullDataApRespTimeoutMsec = 40;
        }else{
            tANI_U32 dataInActivityTimeout;
            // Clear the TPE reg to use the 11b rates for control frames
            halTpe_Set11gProtectionCntrlIndex(pMac, FALSE);
            
            wlan_cfgGetInt( pMac, WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT, &dataInActivityTimeout );
            /* Add for b/g interop Wifi test */
            pFwConfig->ucUcastDataRecepTimeoutMs = (tANI_U8)dataInActivityTimeout;
            pFwConfig->nullDataApRespTimeoutMsec = 0;
        }
        /* Update FW SysConfig with NULL Data AP response delay timeout Value */
        halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
            (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    }

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        tANI_U32 cfgVal = 0;

        WLAN_VOS_DIAG_EVENT_DEF(protectionStatus, vos_event_wlan_bss_prot_payload_type);

        palZeroMemory(pMac->hHdd, &protectionStatus, sizeof(vos_event_wlan_bss_prot_payload_type));

        wlan_cfgGetInt(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, &cfgVal);
        protectionStatus.prot_type = cfgVal;
        if (llbCoexist) {
            protectionStatus.event_type = WLAN_BSS_PROTECTION_ON;
        } else {
            protectionStatus.event_type = WLAN_BSS_PROTECTION_OFF;
        }

        WLAN_VOS_DIAG_EVENT_REPORT(&protectionStatus, EVENT_WLAN_BSS_PROTECTION);
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    return;
}

#ifdef ANI_SUPPORT_SMPS

/**
 * @function : halInitMimoPs_AddSta
 *
 * @breif :  This function Initializes the Mimo PowerSave state before adding the Station
 *
 *      LOGIC:
 *
 *      NOTE :
 *
 *
 */
static inline void
halInitMimoPs_AddSta(tpAniSirGlobal pMac, tANI_U16 staIdx,
                                                        tSirMacHTMIMOPowerSaveState mimoPS)
{
    tSetMIMOPS sMIMO_PSparams;

#ifdef FIXME_GEN5
    if ((param->staType == STA_ENTRY_SELF) && (systemRole == eSYSTEM_STA_ROLE))
    {
        pMac->hal.halMac.psInfo.mimoPSenabled =
            (param->mimoPS == eSIR_HT_MIMO_PS_STATIC) ? 0:1;

        if (pMac->hal.halMac.psInfo.mimoPSenabled)
        {
            if (halRxp_setMimoPwrSaveCtrlReg(pMac,
                   QWLAN_RXP_MIMO_PWR_SAVE_CTRL_MULTIPLE_RX_ADDR1_2_HIT_BASED_MASK |
                   QWLAN_RXP_MIMO_PWR_SAVE_CTRL_MULTIPLE_RX_INIT_ENABLE_MASK) != eHAL_STATUS_SUCCESS)
                          goto generate_response;

            if (halAdu_SetMimoPScfg(pMac, eSIR_HT_MIMO_PS_2SS_1SS) != eHAL_STATUS_SUCCESS)
                          goto generate_response;

            // Enable Mimo power save for the station
            if (halAdu_UpdateControl(pMac, ADU_CONTROL_MIMO_POWER_SAVING_ENABLE_MASK)
                                                                                              != eHAL_STATUS_SUCCESS)
                          goto generate_response;
        }
    }
#endif
    /** SM Power Save : Initialize the Number of Rxp Chains depending on the SM PS State
         * Set the state in AP if the staId is for PEER, in STA if staID is for Self */

    if (( pMac->hal.halSystemRole == eSYSTEM_AP_ROLE && pMac->hal.halMac.selfStaId != staIdx)  ||
          (pMac->hal.halSystemRole == eSYSTEM_STA_ROLE && pMac->hal.halMac.selfStaId == staIdx)) {

        sMIMO_PSparams.staIdx = staIdx;
        sMIMO_PSparams.htMIMOPSState = mimoPS;
        sMIMO_PSparams.fsendRsp = false;
        /** To Set the Hal Globals, hal Phy and Update S/W Mac about the Mimo PS */
        halMsg_SetMimoPs(pMac, &sMIMO_PSparams);
     }

}

#endif

/*
*   halMsg_allBssShortSlotEnabled
*   Brief: The function returns false if there is at least one BSS which has shortSlot disabled.
*            otherwise returns true.
*/
tANI_U32 halMsg_allBssShortSlotEnabled(tpAniSirGlobal pMac)
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U32 idx = 0;
    for(idx = 0; idx < pMac->hal.halMac.maxBssId; idx++)
    {
        if(bssTable[idx].valid && !bssTable[idx].bssRaInfo.u.bit.fShortSlot)
            return 0;
    }
    return 1;
}
////////////////////////////////////////////////////////////////////////////
//                 STA ADD/DEL API
////////////////////////////////////////////////////////////////////////////

eHalStatus halMsg_addStaUpdateRPE( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param )
{
    tRpeStaDesc rpeStaDesc;
        tANI_U8 i;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
        tANI_U8 qId;

    /* For UAPSD Re-Assoc Rsp we need not update RPE since it
     * creates problem in re-ordering.
     *
     * One of the reason for ping timeout in UAPSD is due to RPE reset. In UAPSD
     * when we get ReAssoc Rsp, we add station, in that process we update all
     * H/W modules including RPE. We basically reset RPE bitmap, SSN and all
     * other attributes of RPE. Let's say if BA is already established
     * before Libra gets Reassoc RSP, then as part of response handling we
     * reset RPE. AP still thinks that Libra has BA and tries to send AMPDU
     * and then BAR frames which get acknowledged with normal Acks.
     *
     * */

    if (param->updateSta) {
        return status;
    }

    for (qId = 0; qId < HW_MAX_QUEUES; qId++)
    {
        status = halRpe_BlockAndFlushFrames(pMac, staIdx, qId, eRPE_SW_DISABLE_DROP);
        if (eHAL_STATUS_SUCCESS != status) {
                    return status;
        }
    }

    /** Zero out the RPE STA descriptor structure */
    if ((status = palZeroMemory(pMac->hHdd, &rpeStaDesc,
                    sizeof(rpeStaDesc))) != eHAL_STATUS_SUCCESS)
        return status;

    /** Configure the RPE STA Desc */
    for (i = 0; i < HW_MAX_QUEUES; i++) {
        rpeStaDesc.rpeStaQueueInfo[i].val = 1;
        rpeStaDesc.rpeStaQueueInfo[i].reserved1 = 0;
        rpeStaDesc.rpeStaQueueInfo[i].bar = 0;
        rpeStaDesc.rpeStaQueueInfo[i].psr = 0;
        rpeStaDesc.rpeStaQueueInfo[i].rty = 1;
        rpeStaDesc.rpeStaQueueInfo[i].fsh = 1;
        rpeStaDesc.rpeStaQueueInfo[i].ord = 1;
        rpeStaDesc.rpeStaQueueInfo[i].frg = 1;
        rpeStaDesc.rpeStaQueueInfo[i].check_2k = 0;
        rpeStaDesc.rpeStaQueueInfo[i].ba_window_size = 0;

        rpeStaDesc.rpeStaQueueInfo[i].ssn_sval = 0;
        rpeStaDesc.rpeStaQueueInfo[i].ba_ssn = 0;
#ifdef WLAN_HAL_VOLANS
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_BAbitmap = 0;
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_Reorderbitmap = 0;
        rpeStaDesc.rpeStaQueueInfo[i].rod = 1; //To disable HW reordering for now
        rpeStaDesc.rpeStaQueueInfo[i].current_index = 0;        
        rpeStaDesc.rpeStaQueueInfo[i].mem_location_staId_qId = 0;
#else
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_BAbitmapLo = 0;
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_BAbitmapHi = 0;
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_ReorderbitmapLo = 0;
        rpeStaDesc.rpeStaQueueInfo[i].staId_queueId_ReorderbitmapHi = 0;
#endif
        rpeStaDesc.rpeStaQueueInfo[i].reorder_ssn = 0;
        rpeStaDesc.rpeStaQueueInfo[i].reorder_sval = 0;
        rpeStaDesc.rpeStaQueueInfo[i].reorder_window_size = 0;
        rpeStaDesc.rpeStaQueueInfo[i].reserved2 = 0;

    }

    if ( (status = halRpe_cfgStaDesc(pMac, staIdx, &rpeStaDesc)) != eHAL_STATUS_SUCCESS) {
        HALLOGW( halLog(pMac, LOGW, FL("halRpe_cfgStaDesc() fail\n")));
    }

        for (qId = 0; qId < HW_MAX_QUEUES; qId++)
        {
                status = halRpe_BlockAndFlushFrames(pMac, staIdx, qId, eRPE_SW_ENABLE_DROP);
        if (eHAL_STATUS_SUCCESS != status) {
                        return status;
        }
    }

    return status;

}


eHalStatus halMsg_UpdateTpeProtectionThreshold(tpAniSirGlobal pMac, tANI_U32 val)
{
    /** Set the Protection threshold */
    if (halTpe_SetProtectionThreshold(pMac, val) != eHAL_STATUS_SUCCESS) {
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}


static eHalStatus __halMsg_FillTpeRateInfo(tpAniSirGlobal pMac,
        tpTpeStaDescRateInfo pRateInfo, tpAddStaParams param, tTpeRateType rateType)
{
    tANI_U8  i;
    tANI_U32 ampduDensity = 0; //, txPower = 0;
    tTpeRateIdx defRateIdx;
    tANI_U32 protPolicy = 0;
    eHalStatus status=eHAL_STATUS_SUCCESS;
    tPwrTemplateIndex txPower = 0;
#ifdef WLAN_FEATURE_VOWIFI
    tANI_U8 bssIdx;
    tPwrTemplateIndex   maxPwrIndex = 0;
    eRfBandMode rfBand;
#endif

    halGetNonBcnRateIdx(pMac, &defRateIdx);
    if ((status = halTpe_CalculateAmpduDensity(pMac,
                    defRateIdx, &ampduDensity,
                    param->maxAmpduDensity)) != eHAL_STATUS_SUCCESS) {
        return status;
    }

    // Get the protection policy as set in the CFG
#ifndef HAL_SELF_STA_PER_BSS
    if(param->staIdx != pMac->hal.halMac.selfStaId)
#endif
    {
        halRate_getProtectionInfo(pMac, param->staIdx, 0, 0,
            halRate_tpeRate2HalRate(defRateIdx), &protPolicy);
    }

    halRate_getPowerIndex(pMac, defRateIdx, &txPower);

#ifdef WLAN_FEATURE_VOWIFI
    /* The power for the BD rates in TPE descriptor should  be less than the max power limit */
    if ((rateType == TPE_STA_BD_RATE) && eHAL_STATUS_SUCCESS == halTable_GetBssIndexForSta(pMac, &bssIdx, param->staIdx))
    {
        rfBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);
        if (eHAL_STATUS_SUCCESS == halTable_GetTxPowerLimitIndex(pMac, rfBand, &maxPwrIndex))
        {
            txPower = ((txPower < maxPwrIndex) ? txPower: maxPwrIndex);
        }
    }
#endif

    /* Fill the basic rate initially for all the rate primary, secondary and tertiary */
    for (i=0; i<TPE_STA_MAX_RETRY_RATE; i++,pRateInfo++) {
        if(rateType != TPE_STA_BD_RATE) {
            pRateInfo->protection_mode = protPolicy;
        }
        pRateInfo->ampdu_density = ampduDensity;
        pRateInfo->STBC_Valid = 0;
        pRateInfo->rate_index = defRateIdx;
        pRateInfo->tx_power = txPower;
        pRateInfo->tx_antenna_enable = 0;
    }

    return status;
}

static eHalStatus halMsg_addStaUpdateTPE( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param )
{
    tTpeStaDesc tpeStaDescCfg;
    tANI_U32 val;
    tANI_U32    cfgLen;
    tMtuMode mtuMode;
    tANI_U8 rifs;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tSirMacAddr selfMac;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);

    /** Zero out the TPE STA descriptor structure */
    if ((status = palZeroMemory(pMac->hHdd, &tpeStaDescCfg,
                    sizeof(tpeStaDescCfg))) != eHAL_STATUS_SUCCESS)
        return status;


    /** Fill the TPE STA Desc */
    if (param->staType == STA_ENTRY_PEER)
    {
        tpeStaDescCfg.macAddr1Lo = (((tANI_U32)param->staMac[3]) << 24) |
                                   (((tANI_U32)param->staMac[2]) << 16) |
                                   (((tANI_U32)param->staMac[1]) << 8)  |
                                   ((tANI_U32)param->staMac[0]);
        tpeStaDescCfg.macAddr1Hi = (((tANI_U32)param->staMac[5]) << 8)  |
                                   ((tANI_U32)param->staMac[4]);


        if(systemRole == eSYSTEM_AP_ROLE)
        {
            tpeStaDescCfg.macAddr2Lo = (((tANI_U32)param->bssId[1]) << 8)  |
                                       ((tANI_U32)param->bssId[0]);
            tpeStaDescCfg.macAddr2Hi = (((tANI_U32)param->bssId[5]) << 24) |
                                       (((tANI_U32)param->bssId[4]) << 16) |
                                       (((tANI_U32)param->bssId[3]) << 8)  |
                                       ((tANI_U32)param->bssId[2]);
        }
        else
        {
            if (systemRole == eSYSTEM_MULTI_BSS_ROLE)
            {
                HALLOGW( halLog(pMac, LOGW, FL("Multi system role for sta %d\n"),
                            staIdx));
                // FIXME : What to do for this case.
            }
            assert (systemRole != eSYSTEM_MULTI_BSS_ROLE)

            cfgLen = SIR_MAC_ADDR_LENGTH;
            if ( (wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *)selfMac, &cfgLen)) != (tSirRetStatus) eSIR_SUCCESS)
            {
                HALLOGE( halLog(pMac, LOGE, FL("cfgGetStr(WNI_CFG_STA_ID) failed \n")));
                return status;
            }

            tpeStaDescCfg.macAddr2Lo = (((tANI_U32)selfMac[1]) << 8) |
                ((tANI_U32)selfMac[0]);
            tpeStaDescCfg.macAddr2Hi = (((tANI_U32)selfMac[5]) << 24) |
                (((tANI_U32)selfMac[4]) << 16)  |
                (((tANI_U32)selfMac[3]) << 8) |
                ((tANI_U32)selfMac[2]);
        }
    }

    /** Get the retry threshold0 value */
    if ( wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_ZERO, &val) != (tSirRetStatus) eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_DYNAMIC_THRESHOLD_ZERO Failed\n")));
        return status;
    }
    tpeStaDescCfg.retry_threshold0 = val;
    HALLOG1( halLog( pMac, LOG1, FL("retry zero =  %d\n"),  val ));

    /** Get the retry threshold1 value */
    if ( wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_ONE, &val) != (tSirRetStatus) eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_DYNAMIC_THRESHOLD_ZERO Failed\n")));
        return status;
    }

    tpeStaDescCfg.retry_threshold1 = val;
    HALLOG1( halLog( pMac, LOG1, FL("retry one =  %d\n"),  val ));

    /** Get the retry threshold1 value */
    if ( wlan_cfgGetInt(pMac, WNI_CFG_DYNAMIC_THRESHOLD_TWO, &val) != (tSirRetStatus) eSIR_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("cfgGet WNI_CFG_DYNAMIC_THRESHOLD_TWO Failed\n")));
        return status;
    }

    HALLOG1( halLog( pMac, LOG1, FL("retry two =  %d\n"),  val ));
    tpeStaDescCfg.retry_threshold2 = val;

#ifndef WLAN_HAL_VOLANS
    // Libra 1.0 HW bug:  CR-0000141920
    // SW Workaround for MPDU Ack counters are not incremented after the first retry
    if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_1_0) {
        tpeStaDescCfg.retry_threshold0 = LIBRA1_0_RA_STATS_HW_BUG_PRIM_RETRY_THRESHOLD;
        tpeStaDescCfg.retry_threshold1 = LIBRA1_0_RA_STATS_HW_BUG_SECD_RETRY_THRESHOLD;
        tpeStaDescCfg.retry_threshold2 = LIBRA1_0_RA_STATS_HW_BUG_TERT_RETRY_THRESHOLD;
    }
#endif
    /** Fill the TPE STA Desc */
    tpeStaDescCfg.ampdu_valid = 0;

    if (param->staType == STA_ENTRY_SELF)
    {
        // For bc/mc traffic set ack policy to no-ack
        tpeStaDescCfg.ack_policy_vectorLo = SELF_STA_PER_QUEUE_ACK_POLICY_Q0_15; /** Ack needed for Queue 8-10 */
        tpeStaDescCfg.ack_policy_vectorHi = SELF_STA_PER_QUEUE_ACK_POLICY_Q16_19;

        tpeStaDescCfg.mcbcStatsQidMap = (1 << BC_STATS_QID_0_MASK) | (1 << MC_STATS_QID_1_MASK);
#if 0
        // Fill in the self sta mac address for the self sta
        cfgLen = SIR_MAC_ADDR_LENGTH;
        if ( (wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *)selfMac, &cfgLen)) != (tSirRetStatus) eSIR_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("wlan_cfgGetStr(WNI_CFG_STA_ID) failed \n")));
            return status;
    }

        tpeStaDescCfg.macAddr2Lo = (((tANI_U32)selfMac[1]) << 8) |
            ((tANI_U32)selfMac[0]);
        tpeStaDescCfg.macAddr2Hi = (((tANI_U32)selfMac[5]) << 24) |
            (((tANI_U32)selfMac[4]) << 16)  |
            (((tANI_U32)selfMac[3]) << 8) |
            ((tANI_U32)selfMac[2]);
#endif
        tpeStaDescCfg.macAddr2Lo = (((tANI_U32)param->staMac[1]) << 8) |
            ((tANI_U32)param->staMac[0]);
        tpeStaDescCfg.macAddr2Hi = (((tANI_U32)param->staMac[5]) << 24) |
            (((tANI_U32)param->staMac[4]) << 16)  |
            (((tANI_U32)param->staMac[3]) << 8) |
            ((tANI_U32)param->staMac[2]);

    }
    else
    {
        // For uc traffic set ack policy to imm-ack
        tpeStaDescCfg.ack_policy_vectorLo = PEER_STA_PER_QUEUE_ACK_POLICY_Q0_15;
        tpeStaDescCfg.ack_policy_vectorHi = PEER_STA_PER_QUEUE_ACK_POLICY_Q16_19;
    }

    halTable_SetStaMaxAmpduDensity(pMac, staIdx, param->maxAmpduDensity);

    __halMsg_FillTpeRateInfo(pMac, tpeStaDescCfg.rate_params_20Mhz, param, TPE_STA_20MHZ_RATE);
    __halMsg_FillTpeRateInfo(pMac, tpeStaDescCfg.rate_params_40Mhz, param, TPE_STA_40MHZ_RATE);
    __halMsg_FillTpeRateInfo(pMac, tpeStaDescCfg.bd_rate_params, param, TPE_STA_BD_RATE);

    /** Set Max bytes in AMPDU
        00: 8k; 01: 16k; 10: 32k; 11:64k */
    tpeStaDescCfg.max_bytes_in_ampdu = param->maxAmpduSize;
    // If RIFS mode is enabled configure the "data_wt_cycles" field to a non-zero value
    if(param->rifsMode)
    {
        mtuMode = halMTU_getMode(pMac);
        //get RIFS, minus the time before which TPE needs to program the TXP, change it to clock cycles, devided by 4
        if( (status = halMTU_getRIFS( pMac, mtuMode, &rifs)) != eHAL_STATUS_SUCCESS)
            return status;
        tpeStaDescCfg.data_wt_cycles = (rifs * (SYS_CLOCKS_PER_MS/1000) - (TIME_TPE_PROGRAM_TXP * SYS_CLOCKS_PER_MS)/1000/1000) / 4;
    }

    /** Configure the TPE STA Desc */
    status = halTpe_SetStaDesc(pMac, staIdx, &tpeStaDescCfg);

        //txop correspoding to each back off engine not programmed assuming the default value
        //in there is good enough for now.

    return status;
}

//
//
// Function that will fill up the UMA descriptor table.
//
static eHalStatus FillUmaDescriptor(tpAniSirGlobal pMac,
    tANI_U8 UmaIdx, tANI_U32 macAddrLo,
    tANI_U32 macAddrHi, tANI_U8 dpuIdx, tANI_U8 dpuSig,
    tANI_U8 staIdx, tANI_U8 wmmEnabled)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tBssSystemRole BssSystemRole = eSYSTEM_UNKNOWN_ROLE;
    tAduUmaStaDesc aduUmaDescCfg;

    BssSystemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);

    /* Program STA descriptors for UMA */
    /** Zero out the UMA STA descriptor structure */
    if ((status = palZeroMemory(pMac->hHdd, &aduUmaDescCfg,
        sizeof(tAduUmaStaDesc))) != eHAL_STATUS_SUCCESS)
        return status;

    aduUmaDescCfg.valid = 1;

    aduUmaDescCfg.ackPolicy= UMA_ACK_POLICY_UC_FRAME;

    /** Fill the UMA STA Desc */
    aduUmaDescCfg.bssidHi = macAddrHi;
    aduUmaDescCfg.bssidLo = macAddrLo;

    aduUmaDescCfg.dpuDescIdx = dpuIdx;
    aduUmaDescCfg.dpuSig = dpuSig;
    aduUmaDescCfg.staIdx = staIdx;

    aduUmaDescCfg.protVer = 0;
    aduUmaDescCfg.type = SIR_MAC_DATA_FRAME;

    if(wmmEnabled)
        aduUmaDescCfg.subType =  SIR_MAC_DATA_QOS_DATA;
    else
        aduUmaDescCfg.subType =  SIR_MAC_DATA_DATA;
    if(BssSystemRole == eSYSTEM_AP_ROLE)
    {
        aduUmaDescCfg.toDS = 0;
        aduUmaDescCfg.fromDS = 1;
    }
    else if(BssSystemRole == eSYSTEM_STA_ROLE)
    {
        aduUmaDescCfg.toDS = 1;
        aduUmaDescCfg.fromDS = 0;
    }
    else if(BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE)
    {
        aduUmaDescCfg.toDS = 0;
        aduUmaDescCfg.fromDS = 0;
    }
    else if ((BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) ||
             (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE))
    {
        aduUmaDescCfg.toDS = 1;
        aduUmaDescCfg.fromDS = 1;
    }
    aduUmaDescCfg.moreFrag = 0;
    aduUmaDescCfg.retry = 0;
    aduUmaDescCfg.powerMgmt = 0;
    aduUmaDescCfg.moreData = 0;
    aduUmaDescCfg.wep = 0;
    aduUmaDescCfg.order = 0;

    status = halAdu_SetUmaStaDesc(pMac, UmaIdx, &aduUmaDescCfg);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
        return status;
    }
    return eHAL_STATUS_SUCCESS;
}

/**
 * ADU/UMA changes when adding a station
 */
static eHalStatus halMsg_addStaUpdateADU(tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 macAddrHi, macAddrLo;
    tANI_U32 staMacAddrHi, staMacAddrLo;
    tANI_U8 dpuPTKIdx, dpuPTKSig;
    tBssSystemRole BssSystemRole = eSYSTEM_UNKNOWN_ROLE;
    tANI_U8 umaIdx=0;

    /** If adding a peer station (association), add the entry into UMA */
    if (param->staType != STA_ENTRY_PEER)
        return eHAL_STATUS_SUCCESS;

    macAddrHi = (((tANI_U32)param->bssId[0]) << 8) |
        ((tANI_U32)param->bssId[1]);
    macAddrLo =
        (((tANI_U32)param->bssId[2]) << 24) |
        (((tANI_U32)param->bssId[3]) << 16) |
        (((tANI_U32)param->bssId[4]) << 8) |
        ((tANI_U32)param->bssId[5]);

    // Get the PTK DPU index and signature
    status = halTable_GetStaDpuIdx( pMac, staIdx, &dpuPTKIdx );
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Invalid station index\n")));
        return status;
    }

    status = halDpu_GetSignature( pMac, dpuPTKIdx, &dpuPTKSig );
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Sta signature not found\n")));
        return status;
    }

    // Should get Bss specific role here.
    BssSystemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);
    assert (BssSystemRole != eSYSTEM_UNKNOWN_ROLE);

    HALLOG1( halLog( pMac, LOG1, FL("WMM=%d\n"), param->wmmEnabled));
    HALLOG1( halLog( pMac, LOG1, FL("STA MAC=%x%x%x%x%x%x\n"),
        (tANI_U32)param->staMac[0],
        (tANI_U32)param->staMac[1],
        (tANI_U32)param->staMac[2],
        (tANI_U32)param->staMac[3],
        (tANI_U32)param->staMac[4],
        (tANI_U32)param->staMac[5]) );

    if ((status=halTable_GetStaUMAIdx(pMac, staIdx, &umaIdx))
            != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(  pMac, LOGE,
                FL("Unable to allocate umaIdx for sta%d\n"), staIdx));
        return status;

    }
    // Allocate an index into UMA descriptor table.
    if (umaIdx == HAL_INVALID_KEYID_INDEX) {
        if ((status = halUMA_AllocId(pMac, &umaIdx)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed, uma table full\n")));
            return status;
        }
    }
    HALLOG1( halLog( pMac, LOG1, FL("UMA programming allocated %d\n"), umaIdx));
    halTable_SetStaUMAIdx(pMac, staIdx, umaIdx);
    FillUmaDescriptor( pMac, umaIdx, macAddrLo, macAddrHi,
        dpuPTKIdx, dpuPTKSig, staIdx, (tANI_U8)(param->wmmEnabled | param->htCapable) );

    /* staMac would contain the peer's MAC address */
    staMacAddrHi = (((tANI_U32)param->staMac[0]) << 8) |
        ((tANI_U32)param->staMac[1]);

    staMacAddrLo = (((tANI_U32)param->staMac[2]) << 24) |
        (((tANI_U32)param->staMac[3]) << 16) |
        (((tANI_U32)param->staMac[4]) << 8) |
        ((tANI_U32)param->staMac[5]);

    if(BssSystemRole == eSYSTEM_STA_ROLE) {
        // In the case of infra program the default MAC address.
        // to be the APs/peers MAC address.
        status = halAdu_WriteUmaDefaultMacAddr(pMac, staMacAddrLo,
            staMacAddrHi, umaIdx);
        if(status != eHAL_STATUS_SUCCESS) {
            HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
            return status;
        }
    } else {
        // Put the remote ends address.
        status = halAdu_AddToUmaSearchTable(pMac, staMacAddrLo,
                staMacAddrHi, umaIdx);
        if(status != eHAL_STATUS_SUCCESS) {
            HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
            return status;
        }
    }

    return status;
}

/**
 * ADU/UMA changes when deleting a station.
 */
static eHalStatus halMsg_delStaUpdateADU( tpAniSirGlobal pMac, tANI_U16 umaIdx, tANI_U8 staType)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAduUmaStaDesc pAduUmaDescCfg = &(pMac->hal.halMac.aduUmaDesc[umaIdx]);

    if (staType != STA_ENTRY_PEER)
        return eHAL_STATUS_SUCCESS;

    pAduUmaDescCfg->valid = 0;

    status = halAdu_SetUmaStaDesc(pMac, umaIdx, pAduUmaDescCfg);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
        return status;
    }

    return status;
}


static eHalStatus halMsg_addStaUpdateBMU( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param )
{
#ifdef WLAN_SOFTAP_FEATURE
    tBssSystemRole systemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);

    if(systemRole == eSYSTEM_AP_ROLE)
        halBmu_UpdateStaBMUApMode(pMac, staIdx, param->uAPSD, param->maxSPLen, 0 /*full cleanup before adding*/); 
    else
        halBmu_sta_enable_disable_control(pMac, staIdx, eBMU_ENB_TX_QUE_ENB_TRANS);

    return eHAL_STATUS_SUCCESS;
#else
    return halBmu_sta_enable_disable_control(pMac, staIdx, eBMU_ENB_TX_QUE_ENB_TRANS);
#endif

}

/*
 * Initialize security configurations for this STA
 *
 * Information regarding encMode & dpuPli will be
 * available only during eWNI_SME_SET_CONTEXT_REQ.
 * A separate SMAC_HOSTMESG_ADD_STA will be used
 * at that time to update the STA-related security
 * configurations
 *
 * For SELF / BSSID entry, The DPU index doesn't matter on RX. It is never used.
 * In case of AP and STA role - AP/STA shouldn't recieve any packet with its MAC Address in TA.
 *
 * In case of IBSS, For BSSID entry - STA shouldn't recieve any packet with BSSID as TA.
 *
 * For TX of bc/mc frames. prefilled BD is set such that it takes DPU associated with BSS.
 */

static eHalStatus halMsg_addStaDpuRelatedProcessing( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams param )
{
    tANI_U8     dpuIdx = HAL_INVALID_KEYID_INDEX;
    tANI_U8     bcastDpuIdx = HAL_INVALID_KEYID_INDEX;
    tANI_U8     bcastMgmtDpuIdx = HAL_INVALID_KEYID_INDEX;
    tANI_U32    val;

    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);

    // By the time the code flow reaches here, we should
    // get the Bss specific system role.
    assert (systemRole != eSYSTEM_MULTI_BSS_ROLE);

    if((param->staType == STA_ENTRY_SELF) ||
            ((systemRole == eSYSTEM_STA_IN_IBSS_ROLE) && (param->staType == STA_ENTRY_BSSID)) ||
            ((systemRole == eSYSTEM_BTAMP_AP_ROLE) && (param->staType == STA_ENTRY_BSSID)) ||
            ((systemRole == eSYSTEM_BTAMP_STA_ROLE) && (param->staType == STA_ENTRY_BSSID)))
    {
        // Get the DPU Descriptor Index for the given BSS index
        status = halTable_GetBssDpuIdx( pMac, (tANI_U8)param->bssIdx, &dpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
            // broadcast DPU descriptor not found for this BSS
            HALLOGE( halLog(pMac, LOGE, FL("halTable_GetBssDpuIdx() failed\n")));
            return status;
        }
        // Set Bcast DPU Index
        halTable_SetStaBcastDpuIdx(pMac, staIdx, dpuIdx);
        bcastDpuIdx = dpuIdx;
    } else {
        status = halTable_GetStaDpuIdx(pMac, staIdx, &dpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
            // DPU descriptor not found for this STA
            HALLOGE( halLog(pMac, LOGE, FL("halTable_GetStaDpuIdx() failed\n")));
            return status;
        }
        if(dpuIdx == HAL_INVALID_KEYID_INDEX) {
            // Allocate the DPU entries
            status = halDpu_AllocId(pMac, & dpuIdx);
            if (status != eHAL_STATUS_SUCCESS)
            {
                // DPU descriptor table full...
                HALLOGE( halLog(pMac, LOGE, FL("halDpu_AllocId() fail\n")));
                return status;
            }
        }

        // Get the DPU Descriptor Index for the given BSS index
        status = halTable_GetBssDpuIdx( pMac, (tANI_U8)param->bssIdx, &bcastDpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
            // broadcast DPU descriptor not found for this BSS
            HALLOGE( halLog(pMac, LOGE, FL("halTable_GetBssDpuIdx() failed\n")));
            return status;
        }

        //BcastDpu Index points to the DPU index of BSS.
        //For AP - This Index shouldn't be used. Stations doesnt transmit with bc/mc in RA.
        //For STA - This Index will be used to get DPU index of broadcast keys. when RA is bc/mc.
        //For STA in IBSS - This index points to DPU index of BSS for STATIC WEP
        //                         encryption mode.
        //        For WPA/WPA2 mode - new DPU index will be allocated
        //                        during eWNI_SME_SET_CONTEXT_REQ. This DPU index will hold
        //                        broadcast keys for that station.

        halTable_SetStaBcastDpuIdx(pMac, staIdx, bcastDpuIdx);

        // Get the Mgmt DPU Descriptor Index for the given BSS index
        status = halTable_GetBssBcastMgmtDpuIdx( pMac, (tANI_U8)param->bssIdx, &bcastMgmtDpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
            // broadcast Mgmt DPU descriptor not found for this BSS
            HALLOGE( halLog(pMac, LOGE, FL("halTable_GetBssDpuIdx() failed\n")));
            return status;
        }
        halTable_SetStaBcastMgmtDpuIdx(pMac, staIdx, bcastMgmtDpuIdx);
    }

    if ( wlan_cfgGetInt(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, &val) != eSIR_SUCCESS)
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_FRAGMENTATION_THRESHOLD Failed\n")));


    // Set the CFG fragthreshold only for unicast STAs
    // Broadcast / Multicast entries are Self STA entries
    if(STA_ENTRY_SELF != param->staType)
    {
        if(param->htCapable)
        {
            HALLOGE(halLog(pMac, LOGE, FL("11N Mode --> Disable Fragmentation \n"));)
                halDpu_SetFragThreshold(pMac, dpuIdx, (tANI_U16) WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX);
        }
        else
        {
            halDpu_SetFragThreshold(pMac, dpuIdx, (tANI_U16) val);
        }
    }

    // Store dpu index into sta structure
    status = halTable_SetStaDpuIdx(pMac, staIdx, dpuIdx);

    /*Extract and save the DPU Sig*/
    halDpu_GetSignature(pMac,dpuIdx,&param->ucUcastSig);
    halDpu_GetSignature(pMac,bcastDpuIdx,&param->ucBcastSig);

    /* This would reset the DPU descriptor encyrpt mode if it is set
       and is applicable only during re-assoc
       */
    if (param->updateSta)
    {
        if (halDpu_ResetEncryMode(pMac, dpuIdx) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halDpu_SetDescriptorAttributes() failed for UnicastdpuIdx %d\n"), dpuIdx));
            return eHAL_STATUS_FAILURE;
        }
    }
     return status;
}

static eHalStatus halMsg_addStaUpdateRXP( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param )
{
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    tRxpRole    rxpRole = eRXP_SELF;
    tANI_U8     dpuPTKIdx, dpuGTKIdx, dpuIGTKIdx;
    tANI_U8     dpuPTKSig, dpuGTKSig, dpuIGTKSig;
    eAniBoolean wep_keyId_extract = eANI_BOOLEAN_FALSE; //Station is added as ENC_MODE_NONE. It is changed in SET_CONTEXT
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 ftBit;

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);
    assert (systemRole != eSYSTEM_MULTI_BSS_ROLE);

    // Get the PTK DPU index and signature
    status = halTable_GetStaDpuIdx( pMac, staIdx, &dpuPTKIdx );
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Invalid station index\n")));
        return status;
    }
    halDpu_GetSignature( pMac, dpuPTKIdx, &dpuPTKSig );

    // Get the GTK DPU index and signature
    status = halTable_GetStaBcastDpuIdx(pMac, staIdx, &dpuGTKIdx);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Invalid station index\n")));
        return status;
    }
    halDpu_GetSignature( pMac, dpuGTKIdx, &dpuGTKSig );

    // Get the IGTK DPU index and signature
    status = halTable_GetStaBcastMgmtDpuIdx(pMac, staIdx, &dpuIGTKIdx);
    if(status != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog( pMac, LOGE, FL("Invalid station index\n")));
        return status;
    }
    halDpu_GetSignature( pMac, dpuIGTKIdx, &dpuIGTKSig );


    // STA signature is no more available in GEN6
#ifdef FIXME_GEN6
    halTable_GetStaSignature(pMac, staIdx, &sign);
#endif

    switch(param->staType)
    {
        case STA_ENTRY_SELF:
            rxpRole = eRXP_SELF;
            break;
        case STA_ENTRY_PEER:
            {
                //station mac address and bssid is same ==> insfrastructure AP
                if(systemRole == eSYSTEM_AP_ROLE)
                {
                    rxpRole = eRXP_PEER_STA;
                }
                else if((systemRole == eSYSTEM_STA_ROLE) ||
                        (systemRole == eSYSTEM_BTAMP_STA_ROLE))
                {
                    rxpRole = eRXP_PEER_AP;
                }
                else if((systemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
                        (systemRole == eSYSTEM_BTAMP_AP_ROLE))
                {
                    rxpRole = eRXP_PEER_STA;
                }

#ifdef FIXME_GEN6 //keeping it until QOS integration. needs to be removed
                // Clean up the TID to TSPEC map on this STA
                tANI_U8 i;
                for (i = 0; i < STACFG_MAX_TC; i++)
                {
                    status = halTable_UpdateStaTidToTsMap(pMac, staIdx, i, LIM_NUM_TSPEC_MAX);
                    if (status != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGE( halLog(pMac, LOGE, FL("halTable_UpdateStaTidToTsMap() fail\n")));
                        return status;
                    }
                }
#endif //#ifdef FIXME_GEN6 //keeping it until QOS integration. needs to be removed
            }
            break;
        case STA_ENTRY_BSSID:
            rxpRole = eRXP_BSSID;
            break;

        default:
            break;
    }

    // Get the frame translation setting
    ftBit = halGetFrameTranslation(pMac);

    if (param->encryptType != eSIR_ED_NONE) {
        wep_keyId_extract = eANI_BOOLEAN_TRUE;
    }
    
    status = halRxp_AddEntry(
            pMac, (tANI_U8) staIdx, param->staMac, rxpRole, param->rmfEnabled,
            dpuPTKIdx, dpuGTKIdx, dpuIGTKIdx, dpuPTKSig, dpuGTKSig, dpuIGTKSig,
            0, ftBit, wep_keyId_extract);

    if (status != eHAL_STATUS_SUCCESS )
        HALLOGE( halLog(pMac, LOGE, FL("halRxp_addEntry failed, status = %x"), status));

    return status;
}

/* Function to update FW parameters when STA is added */

eHalStatus halMsg_addStaUpdateFW( tpAniSirGlobal pMac, tANI_U8 staIdx, tpAddStaParams  param )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tBssSystemRole  BssSystemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);
    tANI_U32 cfgLen = SIR_MAC_ADDR_LENGTH;
    tANI_U8 selfMac[6];
    tANI_U16 beaconInterval = 0;
    tTpeRateIdx rateIndex = TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET;
    tPwrTemplateIndex txPower = 0;

    // BTAMP Fixme
    if ( wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *)selfMac, &cfgLen) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halMsg_addStaUpdateFW: cfgGetStr(WNI_CFG_STA_ID) failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // As part of Add self sta during association get the AID and the BSSID
    if ( (param->staType == STA_ENTRY_SELF) && (BssSystemRole == eSYSTEM_STA_ROLE)) {
        // Prepare the PS poll template in the HW for FW to send during BMPS
        halGetNonBcnRateIdx(pMac, &rateIndex);

            // Get the Tx power index for this rate, ignore the error and continue with tx power 0
            status = halRate_getPowerIndex(pMac, rateIndex, &txPower);
            if (status != eHAL_STATUS_SUCCESS) {
                HALLOGE( halLog( pMac, LOGE, FL("Get power idx failed - %d"), status));
            }

            status = halPS_SetPsPollParam(pMac, staIdx, param->assocId, rateIndex, txPower);
            if ( status != eHAL_STATUS_SUCCESS ) {
                HALLOGE( halLog( pMac, LOGE, FL("halPS_PreparePsPoll failed - 0x%x"), status));
            }

            status = halPS_SetListenIntervalParam(pMac, param->listenInterval);
            if ( status != eHAL_STATUS_SUCCESS ) {
                HALLOGE( halLog( pMac, LOGE, FL("halPS_SetListenIntervalParam failed - 0x%x"), status));
            }         
    }

    //Update FW sys config for PowerSave
    else if ( (param->staType == STA_ENTRY_PEER) && (BssSystemRole == eSYSTEM_STA_ROLE))
        {
            // Get the beacon interval for this bss
            halTable_GetBeaconIntervalForBss(pMac, (tANI_U8)param->bssIdx, &beaconInterval);

            // Update the beacon interval into the FW system config
            halPS_SetBeaconInterval(pMac, beaconInterval);

            status = halPS_SetPeerParams( pMac, staIdx, selfMac, param->bssId);
            if ( status != eHAL_STATUS_SUCCESS )
            {
                HALLOGE( halLog( pMac, LOGE, FL("halMsg_addStaUpdateFW failed - 0x%x"), status));
            }

    }else{
        HALLOGE( halLog( pMac, LOGE, FL("halMsg_addStaUpdateFW: Not in STA role in Infra BSS. Bypass FW cfg update\n")));
    }
    return status;
}


// Function to set the CW Min value for IBSS. Based on the precense of 11b stas.
// If 11b stas are present we set the CW min to 0x3f. If not we switch to 0x1f.
// We rely on the oprateMode passed to HAL from PE on a per peer STA basis.
void halMsg_addSetIbssCWMinValue(
    tpAniSirGlobal  pMac,
    tpSirSupportedRates pRates,
    tANI_U8 staIdx,
    tANI_U8 bssIdx
        )
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    bssTable = &bssTable[bssIdx];

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, (tANI_U8)staIdx);

    // OK we care about all this only for IBSS.
    if ((systemRole != eSYSTEM_STA_IN_IBSS_ROLE) &&
        (systemRole != eSYSTEM_BTAMP_STA_ROLE) &&
        (systemRole != eSYSTEM_BTAMP_AP_ROLE))return;

#ifdef VOLANS_FPGA
    // FPGA works only with 0x7, so dont bother
    // about beacon balance.
    return;
#endif

    HALLOG1(halLog( pMac, LOG1, FL("opRateMode=%d\n"), pRates->opRateMode));

    if (pRates->opRateMode == eSTA_11b)
    {
        // Ok set CW to 0x3f
        halMTU_updateIbssCW(pMac, HAL_IBSS_CW_LIMIT_11b);

        // Increment the no. of 11b Ibss Peers in this bss
        bssTable->numIbssllbPeerCnt++;
        HALLOG1(halLog( pMac, LOG1, FL("num 11b Ibss peers=%d\n"),
            bssTable->numIbssllbPeerCnt));
    }
    else
    {
        if ( bssTable->numIbssllbPeerCnt == 0 )
        {
            halMTU_updateIbssCW(pMac, HAL_IBSS_CW_LIMIT_NON11b);
        }
    }
    halTable_SetStaopRateMode(pMac, staIdx, pRates->opRateMode);
}

/* Function to Add Station */
void
halMsg_AddSta(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpAddStaParams  param, eAniBoolean isFreeable)
{
    tANI_U8     staIdx = 0;
    tANI_U8     bssIdx = 0;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
#ifdef HAL_SELF_STA_PER_BSS
    tANI_U8     selfStaIdx;
#endif

#ifdef FIXME_GEN6
    tSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    systemRole = halGetSystemRole(pMac);
#endif

    HALLOGE( halLog(pMac, LOGE, FL("StaMac: %x %x %x %x %x %x, BSSID: %x %x %x %x %x %x \n"),
                param->staMac[0], param->staMac[1], param->staMac[2], param->staMac[3], param->staMac[4], param->staMac[5],
                param->bssId[0], param->bssId[1], param->bssId[2], param->bssId[3], param->bssId[4], param->bssId[5]));

    // Make sure the BSSID exists.
    if ((status = halTable_FindBssidByAddr(pMac, param->bssId, &bssIdx)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halTable_FindBssidByAddr() fail\n")));
        goto generate_response;
    }

    //use the station provided with param if staIdx is valid
    //Assumption : the station id passed in has been allocated calling function halTable_GetStaId
    //if station id is not provided then get it now.
    staIdx =  (tANI_U8)(param->staIdx);
    if(staIdx  == HAL_STA_INVALID_IDX)
    {
        // Get a free station index.
        status = halTable_GetStaId(pMac, param->staType, param->bssId, param->staMac, &staIdx);
        if (status != eHAL_STATUS_SUCCESS)  {
            if ((eHAL_STATUS_DUPLICATE_STA == status) && (param->updateSta)) {
                HALLOGE( halLog(pMac, LOGE, FL("Update existing sta %d\n"), staIdx));                
            } else  {
                // Station table full...
                HALLOGE( halLog(pMac, LOGE, FL("halTable_GetStaId() fail with return code = %d\n"), status));
                goto generate_response;
            }
        }
    }

#if defined(ANI_OS_TYPE_LINUX)
    if (halTable_AddToStaCache(pMac,
                param->staMac,
                staIdx) != eHAL_STATUS_SUCCESS)
    {
        // Station table full...
        HALLOGE( halLog(pMac, LOGE, FL("%s: halTable_GetStaId() fail with return code = %d\n"),
                __FUNCTION__, status));
        goto generate_response;
    }
#endif

    //Update station index and bssid index.
    param->staIdx = staIdx;
    param->bssIdx = bssIdx;

#ifdef HAL_SELF_STA_PER_BSS
    halTable_GetBssSelfStaIdxForBss(pMac, bssIdx, &selfStaIdx);
    halTable_SetBssSelfStaIdxForSta(pMac, staIdx, selfStaIdx);
#endif

    halTable_SetBssIndexForSta(pMac, staIdx, bssIdx);

    // Save the STA type - this is used for lookup
    (void) halTable_SetStaType(pMac, staIdx, param->staType);
    (void)halTable_SetStaAssocId(pMac, staIdx, param->assocId);
    halTable_SetStaHtEnabled(pMac, staIdx, (tANI_U8)param->htCapable,param->greenFieldCapable);
    halTable_SetStaQosEnabled(pMac, staIdx, (tANI_U8)(param->wmmEnabled | param->htCapable) );
#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif

    // Get the system role for the given staIdx
    systemRole = halGetBssSystemRoleFromStaIdx(pMac, staIdx);

    if( STA_ENTRY_PEER == param->staType )
    {
        // Add this STA in the BSS context if this is a PEER STA entry
        status = halTable_BssAddSta(pMac, bssIdx, staIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halTable_BssAddSta() fail\n")));
            goto generate_response;
        }

        // Take care of beacon balance for IBSS.
        halMsg_addSetIbssCWMinValue( pMac, &param->supportedRates, staIdx, bssIdx );
    }

    halMsg_addStaDpuRelatedProcessing( pMac, staIdx, param );

    //Update TPE.
    status = halMsg_addStaUpdateTPE( pMac, staIdx, param );
    if ( status != eHAL_STATUS_SUCCESS )
    {
        HALLOGE( halLog( pMac, LOGE, FL("halMsg_addStaUpdateTPE failed - %x"), status));
        goto generate_response;
    }

    //Update ADU / UMA
    if(halGetFrameTranslation(pMac))
    {
        status = halMsg_addStaUpdateADU( pMac, staIdx, param);
        if (status != eHAL_STATUS_SUCCESS) {
            HALLOGE( halLog(pMac, LOGE, FL(" halMsg_addStaUpdateADU failed - %x"), status ));
            goto generate_response;
        }
    }

    //Update RPE.
    status = halMsg_addStaUpdateRPE( pMac, staIdx, param );
    if( status != eHAL_STATUS_SUCCESS )
    {
        HALLOGE( halLog( pMac, LOGE, FL(" halMsg_addSraUpdateRPE failed - %x"), status ));
        goto generate_response;
    }

    //Update FW
    status = halMsg_addStaUpdateFW(pMac, staIdx, param);
    if( status != eHAL_STATUS_SUCCESS )
    {
        HALLOGE( halLog( pMac, LOGE, FL(" halMsg_addStaUpdateFW failed - %x"), status ));
        goto generate_response;
    }

    //TODO : Gen 6 : Enable BTQM sta transmit queues and
    // transmission for newly added sta at BTQM

    //Update BMU.
    status = halMsg_addStaUpdateBMU( pMac, staIdx, param );
    if( status != eHAL_STATUS_SUCCESS )
    {
        HALLOGE( halLog( pMac, LOGE, FL(" halMsg_addSraUpdateRPE failed - %x"), status ));
        goto generate_response;
    }

    //Update RXP.
    status = halMsg_addStaUpdateRXP( pMac, staIdx, param );
    if( status != eHAL_STATUS_SUCCESS )
    {
        HALLOGE( halLog( pMac, LOGE, FL(" halMsg_addSraUpdateRPE failed - %x"), status ));
        goto generate_response;
    }

    {
        tHalRaModeCfg raModeCfg;
        tANI_U8  selfStaIdx;

        raModeCfg.channelBondingEnable = param->txChannelWidthSet;
        if(param->htCapable){
            /* SGI and MIMO are enabled by STA's capability */
            raModeCfg.sgiEnable  = param->fShortGI20Mhz;
            raModeCfg.sgiEnable40 = param->fShortGI40Mhz;
            raModeCfg.mimoEnable  = ( param->htCapable &&
                    (param->mimoPS != eSIR_HT_MIMO_PS_STATIC) );
        } else {
            /* SGI rates not applicable to non 11n STAs */
            raModeCfg.sgiEnable = 0;
            raModeCfg.sgiEnable40 = 0;
            raModeCfg.mimoEnable = 0;
        }
        raModeCfg.shortPreambleEnable = param->shortPreambleSupported;

        // update STA rate info and send rate info mesg to TPE
        if (halMacRaStaInit(pMac, staIdx, &param->supportedRates,
                    &raModeCfg)== eSIR_FAILURE) {
            HALLOGW( halLog(pMac, LOGW, FL("halMacRaStaInit() fail\n")));
            goto generate_response;
        }

        if(param->staType == STA_ENTRY_SELF && bssIdx < 
                pMac->hal.memMap.maxBssids){
            tANI_U8  thisStaIdx;
#ifdef WLAN_SOFTAP_FEATURE            
            tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;            
#endif

            /* note: in STA and IBSS mode, the sequences are different from the upper layer
               Mode  StaType  RA action
               STA   SELF     Update AutoSamplingTable (raGlobalUpdate flag set)
               STA   PEER     Start RA (raStaUpdate flag set)
               IBSS  BSSID    Update AutoSamplingTable (raGlobalUpdate flag set)
               IBSS  PEER     Start RA (raStaUpdate flag set) 
               AP    SELF     Update AutoSamplingTable (raGlobalUpdate flag set)
               AP    PEER     Start RA (raStaUPdate flag set)
            In order to build AutoSamplingTable, SelfSta information is needed,
            but SelfSta information becomes valid only upon the combination of Mode & StaType
            */

            if((systemRole == eSYSTEM_STA_ROLE) ||(systemRole == eSYSTEM_AP_ROLE)){
               /* this will send selfSta information in shared memory without messaging */
                halMacRaStaAdd(pMac, staIdx, STA_ENTRY_SELF);
#ifdef WLAN_SOFTAP_FEATURE               
               halFW_AddStaReq(pMac, staIdx, 1, 0);

#endif
#ifndef FEATURE_RA_CHANGE
               /* send RA_ADD_BSS message to let firmware to build AutoSampleTable */
                halMacRaAddBssReq(pMac, bssIdx, staIdx);
#endif
            }
            /* If this STA is a self entry, walk through all STAs in the
             * STA table and change the valid rate bitmap for all STAs in this BSS */
            for (thisStaIdx = 0; thisStaIdx < pMac->hal.halMac.maxSta; thisStaIdx++){
#ifdef WLAN_SOFTAP_FEATURE
                if(thisStaIdx != staIdx) 
                {
                    if(pSta && pSta->valid && (pSta->bssIdx == bssIdx)) //This sta is a valid entry in the current BSS.
                        halRateUpdateStaRateInfo(pMac, thisStaIdx);
                }
                pSta++;
#else
                if(thisStaIdx != staIdx)
                    halRateUpdateStaRateInfo(pMac, thisStaIdx);
#endif
            }
        }
        /* Note for Firmware RA:
        This part is little bit tricky. firmware RA requires valid 'supported rates'.
        But, supported rates is only valid at last ADD_STA message,
        I had to make sure that RA_ADD_BSS message should be sent after valid supported
        rates are updated both in IBSS and BSS mode.
           In IBSS mode, this ENTRY_BSSID is the point where all supported rates are 
           updated correctly */
        // Get the selfstation index for the BSS
#ifdef HAL_SELF_STA_PER_BSS
        halTable_GetBssSelfStaIdxForBss(pMac, bssIdx, &selfStaIdx);
#else        
        selfStaIdx = (tANI_U8) pMac->hal.halMac.selfStaId;
#endif
        if(systemRole == eSYSTEM_STA_IN_IBSS_ROLE) {
            if(param->staType == STA_ENTRY_BSSID) {
                /* this will send selfSta information in shared memory only without messaging */
                halMacRaStaAdd(pMac, selfStaIdx, STA_ENTRY_BSSID);
#ifdef WLAN_SOFTAP_FEATURE
                halFW_AddStaReq(pMac, selfStaIdx, 1, 0);
#endif
#ifndef FEATURE_RA_CHANGE
                /* send RA_ADD_BSS message to let firmware to build AutoSampleTable */
                halMacRaAddBssReq(pMac, bssIdx, selfStaIdx);
#endif
            }
        }
    }
    /* Note for Firmware RA:
    tricky part, for BSS mode, this is the point where all supported rates are updated correctly
     */
    if(param->staType == STA_ENTRY_PEER)
    {
#ifdef WLAN_SOFTAP_FEATURE
        halMacRaStaAdd(pMac, staIdx, STA_ENTRY_PEER);
        halFW_AddStaReq(pMac, staIdx, 0, 1);
#endif
#ifndef FEATURE_RA_CHANGE
        halMacRaAddStaReq(pMac, staIdx, STA_ENTRY_PEER);
#endif
    }

#ifdef ANI_LED_ENABLE
    // Set the link LED
    if (staIdx > 0 && pMac->lim.gLimSystemRole == eLIM_STA_ROLE) {
        // Turn off the power LED
        if (pMac->hal.ledParam.config.bEnable == eANI_BOOLEAN_TRUE)
        {
        if (pMac->hal.ledParam.config.powerInd == eHAL_LED_ON)
            halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_OFF);

        halSetLed(pMac, eHAL_LINK_LED, eHAL_LED_ON);
        pMac->hal.ledParam.linkLedOn = eANI_BOOLEAN_TRUE;
    }
    }
#endif

    if(param->staType == STA_ENTRY_SELF) {
        tANI_U32 cfgVal = 0;
        // Activate the BA inactivity check timer if we are ht enabled and current sta context is for self.
        // and block ack is not disabled by config.

        if(wlan_cfgGetInt(pMac, WNI_CFG_BA_AUTO_SETUP, &cfgVal)
             != eSIR_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("could not get BLOCK_ACK cfg value\n")));
            return;
        }

        //enable the BA activity check timer only when we are HT capable
        if((true == cfgVal) && (true == param->htCapable)) {
            pMac->hal.halMac.baAutoSetupEnabled = true;
        }else{
           HALLOGW( halLog(pMac, LOGW, FL("AUTO BA setup default NOT enabled from CFG!\n")));
        }

        //Set AMPDU relate TPE Global settings
        if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_MPDUS_IN_AMPDU, &cfgVal)
             != eSIR_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("could not get MAX_MPDUS_IN_AMPDU\n")));
            return;
        }
        halTpe_UpdateMaxMpduInAmpdu(pMac, cfgVal);
        halTpe_TerminateAmpduAtRateChange(pMac, TRUE);

        if(wlan_cfgGetInt(pMac, WNI_CFG_MAX_MEDIUM_TIME, &cfgVal)
             != eSIR_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("could not get MAX_MEDIUM_TIME\n")));
            return;
        }
        halTpe_SetAmpduTxTime(pMac, cfgVal);

    }

    /* ravij: RxP configurations must be updated also for Station Entry. Currently, it is
     * happening only for BSS
     */
    halRxp_EnableBssBeaconParamFilter( pMac, bssIdx);

#ifdef ANI_SUPPORT_SMPS
    /** Initialize the Station's MimoPS State for the new Station */
    halInitMimoPs_AddSta(pMac, staIdx, param->mimoPS);
#endif
    // send a response to caller.
generate_response:
    if ((param->status = status) == eHAL_STATUS_SUCCESS)
    {
        param->staIdx = staIdx;
        /*
         * FIXME:
         * HAL will pass BSSID back to LIM. Today LIM does not store BSS Table.
         * LIM will use this BSSID to store it in the Station Entry in hashtable.
         * That will eventually be used by LIM to delete BSS when station
         * gets Dissociated/Deauthenticated.
         * Need to fix this when LIM maintains the BSS Table of its own
         */
        param->bssIdx = bssIdx;

        /** Increment the Current Num of Sta, counter to track the connected STA's */
        pMac->hal.halMac.numOfValidSta++;
    }

    if (isFreeable == eANI_BOOLEAN_TRUE)
    {
    if(param->respReqd)
          halMsg_GenerateRsp(pMac, SIR_HAL_ADD_STA_RSP, dialog_token, (void *) param, 0);
        else
            palFreeMemory(pMac->hHdd, param);
    }

    return;
}

void
halMsg_UpdateTxCmdTemplate(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpUpdateTxCmdTemplParams  param)
{
/*
    eHalStatus status = eHAL_STATUS_SUCCESS;

   status = halTable_ValidateStaIndex(pMac, (tANI_U8) param->staIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog( pMac, LOGW, FL("halMsg_UpdateTxCmdTemplate: Invalid sta index %d\n"),  param->staIdx ));
        goto generate_response;
    }

    // Get the MPI and CE data.

    halRate_getMPICmd(pMac, (tANI_U8 *) &addPtr->txCmdTempl, param->rateIdx);
    halRate_getCE(pMac, (tANI_U8 *) &addPtr->txCEparam, param->rateIdx);

    halRateDbg_printCeDescriptor(pMac, &addPtr->txCEparam);
    halRateDbg_printTxCmdTemplate(pMac, &addPtr->txCmdTempl);

    // send a response to caller.
generate_response:
    param->status = status;
    if(param->respReqd)
      halMsg_GenerateRsp(pMac, SIR_HAL_UPDATE_STARATEINFO_RSP, dialog_token, (void *) param);
      */
    return;
}

// Function to set the CW Min value for IBSS. Based on the precense of 11b stas.
// If 11b stas are present we set the CW min to 0x3f. If not we switch to 0x1f.
void halMsg_delSetIbssCWMinValue(
    tpAniSirGlobal  pMac,
    tANI_U8 staIdx,
    tANI_U8 bssIdx
        )
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;
    tStaRateMode opRateMode;
    bssTable = &bssTable[bssIdx];

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, (tANI_U8)staIdx);


    // OK we care about all this only for IBSS.
     if ((systemRole != eSYSTEM_STA_IN_IBSS_ROLE) &&
        (systemRole != eSYSTEM_BTAMP_STA_ROLE) &&
        (systemRole != eSYSTEM_BTAMP_AP_ROLE)) return;

#ifdef VOLANS_FPGA
    // FPGA works only with 0x7, so dont bother
    // about beacon balance.
    return;
#endif

    halTable_GetStaopRateMode(pMac, staIdx, &opRateMode);

    if ((opRateMode == eSTA_11b) &&
            (bssTable->numIbssllbPeerCnt > 0))
    {
        bssTable->numIbssllbPeerCnt--;

        // Increment the no. of 11b Ibss Peers in this bss
        HALLOG1(halLog( pMac, LOG1, FL("num 11b Ibss peers=%d\n"),
            bssTable->numIbssllbPeerCnt));
    }
    if ( bssTable->numIbssllbPeerCnt == 0 )
    {
        halMTU_updateIbssCW(pMac, HAL_IBSS_CW_LIMIT_NON11b);
    }

    halTable_SetStaopRateMode(pMac, staIdx, eSTA_INVALID_RATE_MODE);
}

/*
 * Delete STA
 */
void
halMsg_DelSta(
    tpAniSirGlobal      pMac,
    tANI_U16            dialog_token,
    tpDeleteStaParams   pDelStaReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    tANI_U8  dpuIdx;
    tANI_U8 staType, qId;
    tANI_U8 bssDpuIdx, bssIdx = 0;
    tANI_U8 umaIdx;
    tSirMacAddr staMac;
    tBssSystemRole systemRole = eSYSTEM_UNKNOWN_ROLE;

    HALLOGW( halLog(pMac, LOGW, FL("STA #%d\n"), pDelStaReq->staIdx));

    systemRole = halGetBssSystemRoleFromStaIdx(pMac, (tANI_U8)pDelStaReq->staIdx);

    // Validate the station index
    status = halTable_ValidateStaIndex(pMac, (tANI_U8) pDelStaReq->staIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid sta index %d\n"), pDelStaReq->staIdx));
        goto generate_response;
    }

    status = halTable_GetStaType(pMac, (tANI_U8)pDelStaReq->staIdx, &staType);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid sta index %d"), pDelStaReq->staIdx));
        goto generate_response;
    }

#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif

    if( STA_ENTRY_SELF != staType )
    {
        status = halTable_GetStaDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx, &dpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
        HALLOGW( halLog(pMac, LOGW, FL("No DPU associated with sta %d!\n"),  pDelStaReq->staIdx));
        }
        else
        {
            status = halDpu_ReleaseDescriptor(pMac, dpuIdx);
            if (status != eHAL_STATUS_SUCCESS)
            HALLOGW( halLog(pMac, LOGW, FL("Cannot releae DPU descriptor for sta %d!\n"),
                   pDelStaReq->staIdx));
            status = halTable_SetStaDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx,
                                           HAL_INVALID_KEYID_INDEX);
        }

        status = halTable_GetStaBcastDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx, &dpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
        HALLOGW( halLog(pMac, LOGW, FL("No DPU associated with sta %d!\n"),  pDelStaReq->staIdx));
        }
        else
        {
            if((status = halTable_GetBssIndexForSta(pMac, &bssIdx, (tANI_U8)pDelStaReq->staIdx)) != eHAL_STATUS_SUCCESS)
            {
            HALLOGW( halLog(pMac, LOGW, FL("Unable to get BSSIDX")));
                goto generate_response;
            }

            }
            if((status = halTable_GetBssDpuIdx(pMac, bssIdx, &bssDpuIdx))  != eHAL_STATUS_SUCCESS )
            {
            HALLOGW( halLog(pMac, LOGW, FL("Unable to get BSS DPU Index")));
                goto generate_response;
            }
            if(bssDpuIdx != dpuIdx) //Not same as bss DPU index, Allocated during set context. Free it.
            {
                status = halDpu_ReleaseDescriptor(pMac, dpuIdx);
                if (status != eHAL_STATUS_SUCCESS)
                HALLOGW( halLog(pMac, LOGW, FL("Cannot releae DPU descriptor for sta %d!\n"),
                       pDelStaReq->staIdx));
                status = halTable_SetStaBcastDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx,
                                              HAL_INVALID_KEYID_INDEX);
             }

             // For Ibss beacon balance set the IBSS CWmin correctly.
             halMsg_delSetIbssCWMinValue( pMac, (tANI_U8)pDelStaReq->staIdx, bssIdx);

        // Release the Bcast Mgmt DPU descriptor for this station
        status = halTable_GetStaBcastMgmtDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx, &dpuIdx);
        if (status != eHAL_STATUS_SUCCESS)
        {
        HALLOGW( halLog(pMac, LOGW, FL("No Mgmt DPU associated with sta %d!\n"),  pDelStaReq->staIdx));
        }
        else
        {
            if((status = halTable_GetBssIndexForSta(pMac, &bssIdx, (tANI_U8)pDelStaReq->staIdx)) != eHAL_STATUS_SUCCESS)
            {
            HALLOGW( halLog(pMac, LOGW, FL("Unable to get BSSIDX")));
                goto generate_response;
            }
            if((status = halTable_GetBssBcastMgmtDpuIdx(pMac, bssIdx, &bssDpuIdx))  != eHAL_STATUS_SUCCESS )
            {
            HALLOGW( halLog(pMac, LOGW, FL("Unable to get BSS DPU Index")));
                goto generate_response;
            }
            if(bssDpuIdx != dpuIdx) //Not same as bss DPU index, Allocated during set context. Free it.
            {
                status = halDpu_ReleaseDescriptor(pMac, dpuIdx);
                if (status != eHAL_STATUS_SUCCESS)
                HALLOGW( halLog(pMac, LOGW, FL("Cannot releae Mgmt DPU descriptor for sta %d!\n"),
                       pDelStaReq->staIdx));
                status = halTable_SetStaBcastMgmtDpuIdx(pMac, (tANI_U8) pDelStaReq->staIdx,
                                              HAL_INVALID_KEYID_INDEX);
             }
        }
        if( STA_ENTRY_PEER == staType )
        {
        tDelBAParams    delBAParams;
        tANI_U8         tid;
        tpStaStruct pSta = &((tpStaStruct) pMac->hal.halMac.staTable)[pDelStaReq->staIdx];
        tANI_U8  tidBitSet;


        delBAParams.staIdx = pDelStaReq->staIdx;

            for( tid = 1; tid <= STACFG_MAX_TC; tid++ )
        {
           tidBitSet = (pSta->baInitiatorTidBitMap & (1 << tid)) ;
           delBAParams.baTID  = tid;

           if(tidBitSet)
           {
              /* Delete BA sessions established as Initiator */
              delBAParams.baDirection = eBA_INITIATOR;
              baDelBASession(pMac, &delBAParams);
           }

           /* Delete BA sessions established as Receipient */
           tidBitSet = (pSta->baReceipientTidBitMap & (1 << tid));
           if(tidBitSet)
           {
              delBAParams.baDirection = eBA_RECIPIENT;
              baDelBASession(pMac, &delBAParams);
           }

        }

            // Free all of the associated BA buffers and
            // reclaim all the Session ID's, if any
            baReleaseSTA( pMac,  pDelStaReq->staIdx );

            if (systemRole == eSYSTEM_STA_ROLE) {

                /* This turns off the traffic regulation and also
                 * nullifes the Peer mac address in system config.
                 */
                status = halPS_SetPeerParams( pMac, HAL_STA_INVALID_IDX, NULL, NULL);
                if (status != eHAL_STATUS_SUCCESS) {
                    HALLOGE(halLog( pMac, LOGE, FL("Unable to update FW System Config")));
                    goto generate_response;
                }
            }
        }
    }
    else
    {
        halTable_SetStaDpuIdx(pMac, (tANI_U8)pDelStaReq->staIdx, HAL_INVALID_KEYID_INDEX);
        halTable_SetStaBcastDpuIdx(pMac, (tANI_U8)pDelStaReq->staIdx, HAL_INVALID_KEYID_INDEX);
        halTable_SetStaBcastMgmtDpuIdx(pMac, (tANI_U8)pDelStaReq->staIdx, HAL_INVALID_KEYID_INDEX);
    }


    //Disable TX for this staId
    status = halBmu_sta_enable_disable_control(pMac, pDelStaReq->staIdx, eBMU_DIS_TX_QUE_DIS_TRANS_CLEANUP_QUE); //TODO: verify
    if( eHAL_STATUS_SUCCESS != status )
    {
        HALLOGE( halLog( pMac, LOGE, FL("Unable to update BMU")));
        goto generate_response;
    }

    if (halTable_GetStaUMAIdx(pMac, (tANI_U8)pDelStaReq->staIdx,
                &umaIdx) == eHAL_STATUS_SUCCESS)
    {
        if (HAL_INVALID_KEYID_INDEX != umaIdx) {
            status = halMsg_delStaUpdateADU( pMac, umaIdx, staType);
            if( eHAL_STATUS_SUCCESS != status )
            {
                HALLOGE( halLog( pMac, LOGE, FL("Unable to update ADU/UMA")));
                goto generate_response;
            }
            halTable_SetStaUMAIdx(pMac, (tANI_U8)pDelStaReq->staIdx,
                                  HAL_INVALID_KEYID_INDEX);

            // if not infra STA then delete search table entry
            systemRole = halGetBssSystemRoleFromStaIdx(pMac, (tANI_U8)pDelStaReq->staIdx);
            if(systemRole != eSYSTEM_STA_ROLE)
            {
                // Clear the UMA search table.
                status = halAdu_AddToUmaSearchTable(pMac, 0, 0, umaIdx);
            }
        }
    }

    // Delete RXP entry
    if((status = halTable_FindAddrByStaid(pMac, (tANI_U8) (pDelStaReq->staIdx), staMac)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Failed at halTable_GetStaMac()  %X \n"), status));
          goto generate_response;
    }
    if((status = halRxp_DelEntry(pMac, staMac)) != eHAL_STATUS_SUCCESS)
    {
         HALLOGW( halLog(pMac, LOGW, FL("Failed at halRxp_DelEntry()  %X \n"), status));
         // NOTE: Ignore the failure here as this entry would have been
         // already deleted as part of change link state when Auth/Join fails
    }

#if defined(ANI_OS_TYPE_LINUX)
    halTable_RemoveFromStaCache(pMac, staMac);
#endif
    // Remove the entry.
    halTable_ClearSta(pMac, (tANI_U8) pDelStaReq->staIdx);

    //If Self Entry for Station Context, add it back
    if(staType == STA_ENTRY_SELF)
    {
        HALLOG1( halLog(pMac, LOG1, FL("Adding Self Entry back")));
        if ((status = halMsg_AddStaSelf(pMac)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog(pMac, LOGW, FL("Failed at halMsg_AddStaSelf()  %X \n"), status));
            goto generate_response;
        }
    }

#ifdef ANI_LED_ENABLE
    // Turn OFF Link LED
    if ((pDelStaReq->staIdx > 0) && (pMac->lim.gLimSystemRole == eLIM_STA_ROLE))
    {
        if (pMac->hal.ledParam.config.bEnable == eANI_BOOLEAN_TRUE)
        {
        halSetLed(pMac, eHAL_LINK_LED, eHAL_LED_OFF); // Link LED off
        pMac->hal.ledParam.linkLedOn = eANI_BOOLEAN_FALSE;
        // Turn on the Power LED
        if (pMac->hal.ledParam.config.powerInd == eHAL_LED_ON)
            halSetLed(pMac, eHAL_POWER_LED, eHAL_LED_ON);
    }
    }
#endif

        for (qId = 0; qId < HW_MAX_QUEUES; qId++)
        {
                status = halRpe_BlockAndFlushFrames(pMac, (tANI_U8)pDelStaReq->staIdx, qId, eRPE_SW_DISABLE_DROP);
                if (eHAL_STATUS_SUCCESS != status)
                        goto generate_response;
        }

    //Collect and reset tx/rx stats.
    halMacCollectAndClearStaStats( pMac, (tANI_U8) pDelStaReq->staIdx );

    /** Decrement the counter of Current num of Sta */
    pMac->hal.halMac.numOfValidSta--;

#ifdef WLAN_SOFTAP_FEATURE
    halMacRaStaDel(pMac, (tANI_U8) pDelStaReq->staIdx);
    status = halFW_DelStaReq(pMac, (tANI_U8)pDelStaReq->staIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("halFW_DelStaReq failed for station id %d"), pDelStaReq->staIdx));
        goto generate_response;
    }
    
//#else
//    halMacRaDelStaReq(pMac, (tANI_U8) pDelStaReq->staIdx);
#endif
generate_response:
    // Error case; send message to Caller.
    pDelStaReq->status = status;
    if( pDelStaReq->respReqd )
      halMsg_GenerateRsp(pMac, SIR_HAL_DELETE_STA_RSP, dialog_token, (void *) pDelStaReq, 0);
    return;
}

////////////////////////////////////////////////////////////////////////////
//                 BSS ADD/DEL API
////////////////////////////////////////////////////////////////////////////

static void halMsg_SendAddBssRsp(
        tpAniSirGlobal  pMac,
        tANI_U16 dialog_token,
        tpAddBssParams param,
        tANI_U8 bssIdx,
        eHalStatus status)
{
    param->status = status;
    if (param->status == eHAL_STATUS_SUCCESS) {
        param->bssIdx = bssIdx;
    }
    if (param->respReqd) halMsg_GenerateRsp(pMac, SIR_HAL_ADD_BSS_RSP,
            dialog_token, (void *) param, 0);
    return;
}

/**
 * \fn     :   halMsg_SetupBTAMPAndIBSS
 *
 * \brief  :  called from addBss context for IBSS and BTAMP and does everything that is special for BTAMP and IBSS.
 *               calls routine to enable IBSS mode or btAmp mode based on the btamp_flag.
 *               adds station for self and BSS
 *
 * \param  :   tpAniSirGlobal pMac : Handle to Mac Structure.
 *                  tpAddBssParams param : addbss parameter
 *                  tANI_U8 bssStaIdx : station index for BSS.
 *                  tBssSystemRole BssSystemRole : system role for this BSS.
 *                  IsBtamp_flag : flag will be set for btamp BSS.
 *
 * \return :   eHalStatus
 */

static eHalStatus halMsg_SetupBTAMPAndIBSS(
    tpAniSirGlobal  pMac,
    tANI_U16 dialog_token,
    tpAddBssParams  param,
    tANI_U8         bssStaIdx,
    tBssSystemRole  BssSystemRole,
    tANI_U8         IsBtamp_flag)
{
    eHalStatus   status = eHAL_STATUS_SUCCESS;
    tANI_U8      selfIdx = 0;

    /** Configure the STA to send beacons */
    halMTU_SetIbssValid_And_BTAMPMode(pMac, IsBtamp_flag);

    param->staContext.staType = STA_ENTRY_SELF;
    status = halTable_FindStaidByAddr(pMac, param->staContext.staMac, &selfIdx);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("Unable to locate self STA entry\n")));
        return status;
    }
    param->staContext.staIdx  = selfIdx;
    halMsg_AddSta(pMac,  dialog_token, &(param->staContext), eANI_BOOLEAN_FALSE);
    //using same parameter for adding self station.
    //just need to change the staMac = bssid
    //add sta for BSS

    param->staContext.staType = STA_ENTRY_BSSID;
    param->staContext.staIdx  = bssStaIdx;
    palCopyMemory(pMac->hHdd, &(param->staContext.staMac), param->bssId, sizeof(tSirMacAddr));
    halMsg_AddSta(pMac,  dialog_token, &(param->staContext), eANI_BOOLEAN_FALSE);

    //restore staIdx to selfIdx before generating add bss response to PE
     param->staContext.staIdx  = selfIdx;


    /** Set the Number of Rxp Chains as SM Enabled for IBSS mode */
    status = halSetPowerSaveMode(pMac, eSIR_HT_MIMO_PS_NO_LIMIT);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGP( halLog(pMac, LOGP, FL(" SetPowerSave has failed \n")));
        return status;
    }

    return eHAL_STATUS_SUCCESS;
}

void halMsg_AddBssPostSetChan(tpAniSirGlobal pMac, void* pData,
        tANI_U32 status, tANI_U16 dialog_token)
{
    tANI_U8      bssIdx = 0;
    tpAddBssParams  param = (tpAddBssParams)pData;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
#ifdef HAL_SELF_STA_PER_BSS
    tANI_U8      idx;
#endif
#ifdef WLAN_SOFTAP_FEATURE
    tANI_U8 bcastStaIdx = HAL_STA_INVALID_IDX;
#endif

    tANI_U8      bssStaIdx = 0;
    tANI_U8      bcastDpuIdx = 0;
    tANI_U8      bcastMgmtDpuIdx = 0;
    tBssSystemRole  BssSystemRole = eSYSTEM_UNKNOWN_ROLE;
    tANI_U8      val = 0;
    eRfBandMode  rfBand;
    tTpeRateIdx  rateIndex, mcastRateIndex;
    tMtuMode curMtuMode;
    tMtuMode newMtuMode;
#ifdef WLAN_FEATURE_VOWIFI
    tPwrTemplateIndex   pwrIndex;
    eHalPhyRates        phyRateIndex;
    t2Decimal           pwrDbm;
#endif /* WLAN_FEATURE_VOWIFI */

    tANI_U8      savedBTStatype=0;
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        return;
    }

    if(status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() to Ch %d failed \n"), param->currentOperChannel));
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
        return;
    }

    // Get the BSS index by bssid - find a free slot in the bss table
    halTable_GetBssIndex(pMac, param->bssId, &bssIdx);
    if (status != eHAL_STATUS_SUCCESS) {
        if(param->updateBss && (status == eHAL_STATUS_DUPLICATE_BSSID)) {
            HALLOGW( halLog (pMac, LOGW, FL("Updating the existing BSS %d"), bssIdx));
        } else {
            HALLOGW( halLog(pMac, LOGW,
                FL("halTable_GetBssIndex() fail (error 0x%x, bss %d)\n"),
                status, bssIdx));
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        }
    }

    HALLOGW( halLog(pMac, LOGW, FL("BSS #%d (OpMode %d, bssType %d)\n"),
            bssIdx, param->operMode, param->bssType));

    //
    // Also, update BSS parameters wrt security configuration
    // This is with respect to Group Transient Keys
    //
    {
        // Check if the STA has a valid DPU index
        halTable_GetBssDpuIdx(pMac, bssIdx, &bcastDpuIdx);
        if (bcastDpuIdx == HAL_INVALID_KEYID_INDEX) {
            // Allocate the DPU entries
            status = halDpu_AllocId( pMac, &bcastDpuIdx );
            if ( eHAL_STATUS_SUCCESS != status ) {
                // DPU descriptor table full...
                halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
                return;
            } else {
                HALLOGW( halLog( pMac, LOGW, FL("BSS %d using DpuDesc %d for bcast/mcast\n"),
                        bssIdx, bcastDpuIdx) );
            }
        }
        // setup the signature field, take the signature from dpu
        status = halDpu_GetSignature( pMac, bcastDpuIdx, &val );
        if ( eHAL_STATUS_SUCCESS != status ) {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        }

        param->bcastDpuSignature = (tANI_U8) val & 0xf;
        param->bcastDpuDescIndx = bcastDpuIdx;

        //set bcast DPU index.
        halTable_SetBssDpuIdx( pMac, bssIdx, bcastDpuIdx );
    }

    halTable_GetBssBcastMgmtDpuIdx(pMac, bssIdx, &bcastMgmtDpuIdx);
    if (bcastMgmtDpuIdx == HAL_INVALID_KEYID_INDEX) {
        // Allocate the DPU entry
        status = halDpu_AllocId( pMac, &bcastMgmtDpuIdx );
        if( eHAL_STATUS_SUCCESS != status ) {
            // DPU descriptor table full...
                HALLOGW( halLog( pMac, LOGW, FL("halDpu_AllocId() fail\n")));
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        } else {
                HALLOGW( halLog( pMac, LOGW, FL("BSS %d using DpuDesc %d for bcast/mcast MGMT\n"),
                        bssIdx, bcastMgmtDpuIdx) );
        }
    }
    // setup the signature field, take the signature from dpu
    status = halDpu_GetSignature( pMac, bcastMgmtDpuIdx, &val );
    if (eHAL_STATUS_SUCCESS != status) {
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
        return;
    }

    param->mgmtDpuSignature = (tANI_U8) val & 0xf;
    param->mgmtDpuDescIndx = bcastMgmtDpuIdx;

    //set bcast DPU index.
    halTable_SetBssBcastMgmtDpuIdx( pMac, bssIdx, bcastMgmtDpuIdx );

    // AP Mode, BSS type
    if (param->operMode == BSS_OPERATIONAL_MODE_STA) {
        if(param->bssType == eSIR_IBSS_MODE) {
            BssSystemRole = eSYSTEM_STA_IN_IBSS_ROLE;
            param->staContext.staType = STA_ENTRY_BSSID;
        }
        else if ((param->bssType == eSIR_BTAMP_STA_MODE) &&
                (param->staContext.staType == STA_ENTRY_SELF))
        {
            // This is the 1st add BSS Req for the BTAMP STA
            BssSystemRole = eSYSTEM_BTAMP_STA_ROLE;
            param->staContext.staType = STA_ENTRY_BSSID;
            savedBTStatype = STA_ENTRY_SELF;
        }
        else if ((param->bssType == eSIR_BTAMP_AP_MODE) &&
                (param->staContext.staType == STA_ENTRY_PEER))
        {
            // This is the 2nd ADD BSS Request that is sent
            // on the BTAMP STA side. The Sta type is
            // set to STA_ENTRY_PEER here.
            BssSystemRole = eSYSTEM_BTAMP_STA_ROLE;
            savedBTStatype = STA_ENTRY_PEER;
        }
        else
            if ((param->bssType == eSIR_BTAMP_AP_MODE) &&
                    (param->staContext.staType == STA_ENTRY_SELF))
            {
                // statype is already set by PE.
                // Only 1 ADD BSS Req is sent on the BTAMP AP side.
                BssSystemRole = eSYSTEM_BTAMP_AP_ROLE;
            param->staContext.staType = STA_ENTRY_BSSID;
        } else {
                BssSystemRole = eSYSTEM_STA_ROLE ;
            param->staContext.staType = STA_ENTRY_PEER;
        }
    } else if (param->operMode == BSS_OPERATIONAL_MODE_AP) {
        BssSystemRole = eSYSTEM_AP_ROLE;
        param->staContext.staType = STA_ENTRY_SELF;
    } else {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid operation mode specified\n")));
        status = eHAL_STATUS_FAILURE;
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
        return;
    }

    //set system role
    halSetBssSystemRole(pMac, BssSystemRole, bssIdx);

    // Get a free station index
    status = halTable_GetStaId(pMac, param->staContext.staType, param->bssId, param->bssId, &bssStaIdx);

    //save staIdx for BSS entry
    halTable_SetStaIndexForBss(pMac, bssIdx, bssStaIdx);

#ifdef HAL_SELF_STA_PER_BSS
    // Get the station index allocated with the self MAC address
    HALLOGE( halLog(pMac, LOGE, FL("Adding Bss: selfMacAddr %x:%x:%x:%x:%x:%x\n"),
                param->selfMacAddr[0], param->selfMacAddr[1], param->selfMacAddr[1], 
                param->selfMacAddr[3], param->selfMacAddr[4], param->selfMacAddr[5]));
    status = halTable_FindStaidByAddr((tHalHandle)pMac, param->selfMacAddr, &idx);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("Cannot find selfMacAddr %x:%x:%x:%x:%x:%x\n"),
                    param->selfMacAddr[0], param->selfMacAddr[1], param->selfMacAddr[1], 
                    param->selfMacAddr[3], param->selfMacAddr[4], param->selfMacAddr[5]));
    } else {

        // This would become the self station index for this BSS
        halTable_SetBssSelfStaIdxForBss(pMac, bssIdx, idx);
    }
#endif

    // Update beacon Interval for the bss
    halTable_SetBeaconIntervalForBss(pMac, bssIdx, param->beaconInterval);

    halTable_SetSsidForBss(pMac, bssIdx, param->ssId);
    //set new RF band mode
    rfBand = halUtil_GetRfBand(pMac, param->currentOperChannel);
    pMac->hal.currentRfBand = rfBand;

    //If in 2.4G, update based on 11b coexist flag
    __halMsg_update11bCoexist(pMac,  param->llbCoexist );

    status = halGetDefaultAndMulticastRates(pMac, rfBand, &rateIndex, &mcastRateIndex);
    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW(halLog(pMac, LOGW, FL("halMsg_AddBss: halGetDefaultAndMulticastRates() fail\n")));
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
        return;
    }

    //update hal global configuration
    halSetBcnRateIdx(pMac, rateIndex);
    halSetNonBcnRateIdx(pMac, rateIndex);
    halSetMulticastRateIdx(pMac, mcastRateIndex);

    HALLOGW( halLog(pMac, LOGW, FL("AddBSS (Channel = %d,  extChannel = %d, 11bCoexist: %d) \n"),
            param->currentOperChannel, param->currentExtChannel, param->llbCoexist));

    {
      bssRaParam config;
      config.bit.reserved1     = 0;
#ifndef HAL_SELF_STA_PER_BSS
      config.bit.selfStaIdx    = pMac->hal.halMac.selfStaId;
#else
      config.bit.selfStaIdx    = idx;
#endif
      config.bit.bssIdx         = bssIdx;
      config.bit.llbCoexist    = param->llbCoexist;
      config.bit.ht20Coexist   = param->ht20Coexist;
      config.bit.llgCoexist    = param->llgCoexist;
      config.bit.nonGfPresent  = param->llnNonGFCoexist;
      config.bit.rifsMode      = param->fRIFSMode;
      config.bit.fShortSlot    = param->shortSlotTimeSupported;
      config.bit.fShortPreamble = param->staContext.shortPreambleSupported;
#ifdef WLAN_FEATURE_VOWIFI        
        //Need to pass maxPowerIndex instead of power in dBm. 
        /* Now we should get the power index by taking maxTxPower(power constraint) into consideration
         * and set this power index as the maxPwrIndex in RA BSS Info. No frames should be transmitted 
         * with the power value greater than this */
        halPhyGetMaxTxPowerIndex(pMac, param->maxTxPower, &pwrIndex);
        config.bit.maxPwrIndex = pwrIndex;
        HALLOGE(halLog(pMac, LOGE, FL("Add BSS: BSSIDX = %d, Power(dBm) = %d, Pwr Index = %d"),
                                    bssIdx, param->maxTxPower, config.bit.maxPwrIndex));
#else        
        config.bit.maxPwrIndex = 0;
#endif /* WLAN_FEATURE_VOWIFI */

      halTable_SaveBssConfig(pMac, rfBand, config, bssIdx);

#ifdef WLAN_FEATURE_VOWIFI
        /* Need  to return the power for management frames in the response */
        phyRateIndex = halRate_MacRateIdxtoPhyRateIdx(pMac, rateIndex);
        if (HAL_PHY_RATE_INVALID == phyRateIndex)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Invalid Phy rate index for management frames, setting default")));
            phyRateIndex = HAL_PHY_RATE_11B_LONG_1_MBPS;
        }
        /* The power returned to the PE is used to fill the TX power in WFA TPC report in 802.11k Link measurement frames 
           and probe request frames */
        status = halPhyGetPwrFromRate2PwrTable(pMac, phyRateIndex, &pwrDbm);
        if (eHAL_STATUS_SUCCESS != status)
        {
            HALLOGP(halLog(pMac, LOGP, FL("Get power from rate2pwr table for rateindex %d failed with status %d\n"), phyRateIndex, status));
        }
        pwrDbm /= (t2Decimal)100;      //The halPhyGetPwrFromRate2PwrTable provides pwrDbm as 1900 for 19dBm. So, divide by 100
        param->txMgmtPower = (tPowerdBm)VOS_MIN(param->maxTxPower, pwrDbm);
        HALLOGE(halLog(pMac, LOGE, FL("Add Bss: Mgmt Rate Index = %d, Power = %d"),
                                            rateIndex, param->txMgmtPower));
#endif /* WLAN_FEATURE_VOWIFI */
    }
    curMtuMode = halMTU_getMode(pMac);
	if(eSIR_11B_NW_TYPE  == param->nwType)	  
	{
	   newMtuMode = MODE_11B;
	}
	else if(eSIR_11A_NW_TYPE  == param->nwType)    
	{
	   newMtuMode = MODE_11A;
	}
	else
	{
    newMtuMode = curMtuMode;
    switch(curMtuMode)
    {
        case MODE_11G_PURE:
            if(!param->shortSlotTimeSupported) {
                newMtuMode = MODE_11G_MIXED;
            }
            break;
        case MODE_11G_MIXED:
            if(param->shortSlotTimeSupported &&
                halMsg_allBssShortSlotEnabled(pMac)) {
                    newMtuMode = MODE_11G_PURE;
            }
            break;
        default:
            break;
    }
	}

    if(curMtuMode != newMtuMode)
    {
        pMac->hal.halMac.fShortSlot  = param->shortSlotTimeSupported;
        halMTU_updateTimingParams(pMac, newMtuMode);
    }

    palCopyMemory(pMac->hHdd, &(param->staContext.bssId), param->bssId, sizeof(tSirMacAddr));

    // Set the BSS index before adding the corresponding STA entry
    param->staContext.bssIdx = bssIdx;
    pMac->hal.nwType = param->nwType;

    //not doing SCAN, now update the rate-to-power table and
    //send the updated rate table to TPE.
    if ((BssSystemRole == eSYSTEM_AP_ROLE) ||
            (BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
            (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE))
    {
#ifdef FEATURE_TX_PWR_CONTROL
        status = halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)HALRATE_MODE_START, (tTpeRateIdx)HAL_MAC_MAX_TX_RATES, TRUE);
#else	
        status = halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)MIN_LIBRA_RATE_NUM, (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM, TRUE);
#endif
        if (status != eHAL_STATUS_SUCCESS ) {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
    }
    }

    if((BssSystemRole == eSYSTEM_STA_ROLE) ||
            (((BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) &&
              (param->staContext.staType == STA_ENTRY_PEER))))
    {
        //add sta entry for BSSID
        param->staContext.staType = STA_ENTRY_PEER;
        param->staContext.staIdx = bssStaIdx;
        palCopyMemory(pMac->hHdd, &(param->staContext.staMac), param->bssId, 
                sizeof(tSirMacAddr));
        param->staContext.txChannelWidthSet = param->txChannelWidthSet;
        halMsg_AddSta(pMac,  dialog_token, &(param->staContext), eANI_BOOLEAN_FALSE);
    }
    else if (BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) {
        if ((status = halMsg_SetupBTAMPAndIBSS(pMac, dialog_token, param,
                        bssStaIdx, BssSystemRole,
                        HAL_BTAMP_MODE_OFF)) != eHAL_STATUS_SUCCESS) {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        }
        //CR-0000142146 for Libra 1.0
        //brief : In simultaneous BSS/ IBSS case, the rtsf setting in BD is not correct when address3 mismatch happen
        // solutiong : Enable ssid based filtering in the rxp for IBSS and don't enable addr2/add3 filtering

        //ravij: addr2 and addr3 update is not happening in case of BSS. because of which TSF timers in RXP are
        //not getting updated with the last received beacon TSF which is necessary for Beacon Mode Power Save
        //else BMPS will not work. Hence moving this function outside IBSS case
        //halRxp_EnableBssBeaconParamFilter( pMac, bssIdx);
#ifndef WLAN_HAL_VOLANS
        if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_1_0) { 
            //CR-0000142146 for Libra 1.0
            //brief : In simultaneous BSS/ IBSS case, the rtsf setting in BD is not correct when address3 mismatch happen
            // solution : Enable ssid based filtering in the rxp for IBSS and don't enable addr2/add3 filtering
        halRxp_EnableSSIDBasedFilter( pMac, &(param->ssId));
        }
#endif
        }
    else if ((BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) ||
                (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE))
    {
        if ((status = halMsg_SetupBTAMPAndIBSS(pMac, dialog_token, param,
                        bssStaIdx, BssSystemRole,
                        HAL_BTAMP_MODE_ON)) != eHAL_STATUS_SUCCESS) {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        }
       halRxp_EnableBssBeaconParamFilter( pMac, bssIdx);
       
#ifndef WLAN_HAL_VOLANS
        if (halGetChipRevNum(pMac) == LIBRA_CHIP_REV_ID_1_0) { 
            //CR-0000142146 for Libra 1.0
            //brief : In simultaneous BSS/ IBSS case, the rtsf setting in BD is not correct when address3 mismatch happen
            // solution : Enable ssid based filtering in the rxp for IBSS and don't enable addr2/add3 filtering
            halRxp_EnableSSIDBasedFilter( pMac, &(param->ssId));
        }
#endif        
    }
#ifdef WLAN_SOFTAP_FEATURE
    else if(BssSystemRole == eSYSTEM_AP_ROLE)
    {
        v_U32_t dtimPeriod, dtimThreshLimit;
        /** Set the DTIM period and DTIM treshold limit */

        dtimThreshLimit = WLAN_SOFTAP_DTIM_INTERVAL_THRESHOLD;   /** This is temperory until cfg is added */
        dtimPeriod = param->dtimPeriod;
        halMTU_SetDtim(pMac, dtimPeriod, dtimThreshLimit);

        //This station id will be used for unicast TX for unknown peer as well as for RX
        param->staContext.staType = STA_ENTRY_SELF;
        param->staContext.staIdx  = bssStaIdx;
        palCopyMemory(pMac->hHdd, &(param->staContext.staMac), param->bssId, sizeof(tSirMacAddr));
        halMsg_AddSta(pMac,  dialog_token, &(param->staContext), eANI_BOOLEAN_FALSE);
        }
#endif        
    else
    {
        HALLOGW( halLog(pMac, LOGW, FL("BSS System Role is wrong!!! %d\n"), BssSystemRole));
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
    }

    //if(param->updateBss) {
    //    halRate_updateResponseRateTableByBssBasicRate(pMac, &param->staContext.supportedRates);
    //}

    if (param->fLsigTXOPProtectionFullSupport) {
        if (halTpe_SetLsigTxopProtection(pMac, param->fLsigTXOPProtectionFullSupport, 0)
                != eHAL_STATUS_SUCCESS)
        {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
    }
    }

    /** Update the Valid bssid bitmap */
    if ((status = halMTU_UpdateValidBssid(pMac, bssIdx, eHAL_SET)) != eHAL_STATUS_SUCCESS)
    {
        halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
        return;
    }

    // Set the TSF compensation values in RXP for beacon reception
    halRxp_SetTsfCompensationValues(pMac);
    // This will update the Global HAL System Role
    // to be Multi or Bss specific. It will be Multi
    // if the no. of active BSS are > 1.
    halSetGlobalSystemRole(pMac, BssSystemRole);


    // set beacon bssid filter
    if ((BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
            ((BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) &&
             (savedBTStatype == STA_ENTRY_SELF)) ||
            (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE) ||
            (BssSystemRole == eSYSTEM_AP_ROLE))
    {
        // Initialize the beacon template to first 4 bytes (used as beacon length) as zero.
        // FIXME - Should this be removed once the interrupts
        // are hooked in??
        // set beacon template

        /* Always write beacon template on BSS INDEX = 0. This is a limitation
           in Libra. Libra H/W always expects beacon to be written on beacon Index = 0.
           In BT-AMP Multiseesion, this is been tested with infra link on BSS Index 0
           and BT-AMP link on BSS INDEX 1. ALthough BT-AMP link is BSS INDEX 1, we
           write on to TPE with beacon index 0

           In Volans this has to be reverted to use proper BSS INDEX 
           */ 
        halTpe_SetBeaconTemplate(pMac, bssIdx, param->bssId);

        /** Enable the beacon transmission */
        if ( (status = halTpe_EnableBeacon(pMac, param->beaconInterval)) != eHAL_STATUS_SUCCESS)
        {
            halMsg_SendAddBssRsp(pMac, dialog_token, param, bssIdx, status);
            return;
        }
        }

    // Store it in the BSS table, will be used when RXP type/subtype filters are set
    // during SET_LINK_STATE (AP_MODE), to enable all beacon reception 
    halTable_SetObssProtForBss(pMac, bssIdx, param->obssProtEnabled);

#ifdef HAL_BCAST_STA_PER_BSS
        // System role is AP, allocate a broadcast station for buffering broadcast 
        // frames when one of the associated peer stations enter power save
        if (BssSystemRole == eSYSTEM_AP_ROLE) {
            // Allocate a broadcast station for the BSS
            halMsg_AddBcastSta(pMac, bssIdx, param);
            // Get the allocated bcast station index
            halTable_GetBssBcastStaIdx(pMac, bssIdx, &bcastStaIdx);
            /* Enable Listen Mode according to cfg */                     
            halEnableListenMode(pMac, pMac->hal.ghalPhyAgcListenMode);                     
        } else {
            // If not AP role point the bcastStaIdx to the BSS staIdx
            halTable_SetBssBcastStaIdx(pMac, bssIdx, bssStaIdx);
            bcastStaIdx = bssStaIdx;
        }
#endif
#ifndef WLAN_SOFTAP_FEATURE
    if ((BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
        ((BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) &&
            (savedBTStatype == STA_ENTRY_SELF)) ||
        (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE) ||
        (BssSystemRole == eSYSTEM_AP_ROLE))
    {
        /// Enables TIMER5 interrupt for pre-beacon indication
        status = halInitPreBeaconTmr(pMac);
    }
#else
    t[bssIdx].hiddenSsid = param->bHiddenSSIDEn;
    status = halFW_AddBssReq(pMac, bssIdx);
#endif
    if (status != eHAL_STATUS_SUCCESS)
    {
            goto generate_response;
    }

    halRxp_EnableBssBeaconParamFilter( pMac, bssIdx);

generate_response:

    // Error case; send message to Caller.
    param->status = status;

    if (param->status == eHAL_STATUS_SUCCESS) {
        param->bssIdx = bssIdx;
        //param->staContext.staIdx = bssStaIdx;
    }
    if(param->respReqd) {
        halMsg_GenerateRsp(pMac, SIR_HAL_ADD_BSS_RSP, dialog_token, (void *) param, 0);
    }
    return;
}



/*
 * An AddBss may also add a STA as part of the bss addition
 * This STA should always be the correponding AP
 * Thus in the case of an infra AP
 *      AddBss - adds local BSS
 *             - adds local sta (the AP itself)
 * In the case of infra STA
 *      AddBss - adds the local BSS (i.e the BSS we are associating with)
 *             - adds remote sta (i.e teh AP itself)
 * In the case of IBSS
 *      AddBss - either STA is starting IBSS or associating with an IBSS.
 */
void
halMsg_AddBss(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpAddBssParams  param)
{
    eHalStatus   status = eHAL_STATUS_SUCCESS;

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        return;
    }
    //system role, nwType, pure G mode all updated, now change channel
    status = halPhy_ChangeChannel(pMac, param->currentOperChannel,
            (ePhyChanBondState)param->currentExtChannel, TRUE, halMsg_AddBssPostSetChan, (void*)param, dialog_token);
    // If channel is already on the request channel, proceed further with
    // post set channel configuration
    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        halMsg_AddBssPostSetChan(pMac, (void*)param, eHAL_STATUS_SUCCESS, dialog_token);
    } else {
        if (status != eHAL_STATUS_SUCCESS ) {
            goto generate_response;
        }
    }

    return;

generate_response:
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_ADD_BSS_RSP, dialog_token, (void *) param, 0);

    return;
}

#ifdef HAL_BCAST_STA_PER_BSS
void halMsg_AddBcastSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tpAddBssParams param)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpStaStruct pBssSta, pBcastSta;
    tANI_U8 bssStaIdx, bcastStaIdx = HAL_STA_INVALID_IDX;
    tSirMacAddr  bcastMacAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;

    // Get the station index for the BSS
    halTable_GetStaIndexForBss(pMac, bssIdx, &bssStaIdx);

    // Get a free broadcast station
    status = halTable_GetStaId(pMac, STA_ENTRY_BCAST, 
            param->bssId, bcastMacAddr, &bcastStaIdx);
    if (status != eHAL_STATUS_SUCCESS) {
        if (status != eHAL_STATUS_DUPLICATE_STA) {
            bcastStaIdx = bssStaIdx;
            halTable_SetBssBcastStaIdx(pMac, bssIdx, bcastStaIdx);
            HALLOGE(halLog (pMac, LOGE, FL("Could not find free Bcast STA for bssIndex %d, \
                            assigning BSS STA index to BSS Bcast STA index"), bssIdx));
        }
        return;
    }

    halTable_SetBssBcastStaIdx(pMac, bssIdx, bcastStaIdx);
    HALLOGE(halLog (pMac, LOGE, FL("Adding bcast station %d for bss %d"), bcastStaIdx, bssIdx));

    // Get the BSS station context
    pBssSta = &t[bssStaIdx];
    // Get the bcast station context
    pBcastSta = &t[bcastStaIdx];

    // Copy the BSS STA properties into the bcast sta
    vos_mem_copy(pBcastSta, pBssSta, sizeof(tStaStruct));

    pBcastSta->staId = bcastStaIdx;
    // Copy the broadcast station address in the staAddr
    vos_mem_copy((void*)pBcastSta->staAddr, (void*)&bcastMacAddr, sizeof(tSirMacAddr));

    // Initialize the TPE descriptor to be same as the BSS's TPE descriptor
    status = halTpe_SetStaDesc(pMac, bcastStaIdx, &pBssSta->tpeStaDesc);

    // Enable the BTQM for this station
    halBmu_sta_enable_disable_control(pMac, bcastStaIdx, eBMU_ENB_TX_QUE_ENB_TRANS);

    // Set the station type
    (void) halTable_SetStaType(pMac, bcastStaIdx, STA_ENTRY_BCAST);

    return;
}

void halMsg_DelBcastSta(tpAniSirGlobal pMac, tANI_U8 bssIdx, tpDeleteBssParams param)
{
    tANI_U8 bcastStaIdx, bssStaIdx;

    // Get the station idx assigned for the BSS
    halTable_GetStaIndexForBss(pMac, bssIdx, &bssStaIdx);
    // Get the bcast station index assigned to the BSS
    halTable_GetBssBcastStaIdx(pMac, bssIdx, &bcastStaIdx);

    HALLOGE(halLog(pMac, LOGE, FL("Deleting BcastStaIdx %d for bssIdx=%d, bssStaIdx=%d\n"), 
                bcastStaIdx, bssIdx, bssStaIdx));
    // If the bcast station idx is same as the BSS station index, which means
    // there is no special bcast station allocated for the BSS. In that case
    // no need of clearing that station as the station would be cleared as part
    // of delBSS
    if (bssStaIdx != bcastStaIdx) {
        halTable_ClearSta(pMac, bcastStaIdx);
    }

    return;
}
#endif //HAL_BCAST_STA_PER_BSS


/*
 * delete all sta's in the BSS
 */
static void
halmsg_bss_sta_del(
    tpAniSirGlobal  pMac,
    tANI_U8         bssIdx)
{
    tDeleteStaParams delSta;
    tANI_U32         numSta;

    delSta.respReqd = 0;
    delSta.status   = eHAL_STATUS_SUCCESS;
    delSta.assocId  = 0; /* unused */

    for (numSta = 0; (numSta < pMac->hal.halMac.maxSta); numSta++)
    {
        tANI_U8     staIdx;
        eHalStatus  status;
        status = halTable_FindStaInBss(pMac, bssIdx, &staIdx);
        if (status != eHAL_STATUS_SUCCESS) // no sta's found in bss
            return;
        delSta.staIdx = staIdx;
        halMsg_DelSta(pMac, 0 /*dlgToken*/, &delSta);
    }
}


void
halMsg_DelBss(
    tpAniSirGlobal      pMac,
    tANI_U16            dialog_token,
    tpDeleteBssParams   pDelBssReq)
{
    eHalStatus              status = eHAL_STATUS_SUCCESS;
    tANI_U8                 staIdx, bcastDpuIdx, bcastMgmtDpuIdx;
    tSirMacAddr             bssId;
    tSirMacAddr selfAddr;
    tANI_U32 cfgLen = SIR_MAC_ADDR_LENGTH;
    tANI_U8 selfIdx = 0;
    tBssSystemRole  BssSystemRole = eSYSTEM_UNKNOWN_ROLE;

    // Validate the bssIdx
    status = halTable_ValidateBssIndex(pMac, (tANI_U8) pDelBssReq->bssIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid BSSindex %d\n"), pDelBssReq->bssIdx));
        goto invalid_request;
    }

    // Get the Bss System Role
    BssSystemRole = halGetBssSystemRole(pMac, (tANI_U8)pDelBssReq->bssIdx);

    HALLOGW( halLog(pMac, LOGW, FL("bss #%d deleted\n"), pDelBssReq->bssIdx));

    // find the corresponding sta index
    status = halTable_GetStaIndexForBss(pMac, (tANI_U8) pDelBssReq->bssIdx, &staIdx);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("halTable_GetStaIndexForBss() fail (bssidx %d)\n"),
               pDelBssReq->bssIdx));
        status = eHAL_STATUS_STA_INVALID;
        goto invalid_request;
    }

    if ((status = halTable_ValidateStaIndex(pMac, staIdx)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("staId in STA mode is invalid %d\n"), staIdx));
        goto invalid_request;
    }

#ifdef WLAN_SOFTAP_FEATURE
    status = halFW_DelBssReq(pMac, (tANI_U8) pDelBssReq->bssIdx);
    if(status != eHAL_STATUS_SUCCESS)    
    {
        HALLOGW( halLog(pMac, LOGW, FL("halFW_DelBssReq() fail (bssidx %d)\n"),
               pDelBssReq->bssIdx));
        status = eHAL_STATUS_FAILURE;
        goto generate_response;
    }
#endif    

    /* delete all STA's in this BSS first */
    halmsg_bss_sta_del(pMac, (tANI_U8) pDelBssReq->bssIdx);

    // Update RXP - Delete this BSS entry from RXP
    if ( BssSystemRole != eSYSTEM_BTAMP_STA_ROLE  &&  BssSystemRole != eSYSTEM_BTAMP_AP_ROLE )
    {
    status = halTable_FindAddrByBssid( pMac, (tANI_U8) pDelBssReq->bssIdx, bssId );
    if( eHAL_STATUS_SUCCESS == status)
    {
        status = halRxp_DelEntry( pMac, bssId );
        if( eHAL_STATUS_SUCCESS != status)
      {
            HALLOGW( halLog( pMac, LOGW, FL("Unable to DELETE the RXP Entry for BSS Index %d!\n"),
            pDelBssReq->bssIdx ));
      }

      // clear beacon bssid3 filter
      // halRxp_SetupBcnBssidFilters( pMac, bssId, eHAL_CLEAR,
      //                             (pMac->hal.halSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ? eRXP_BCN_BSSID3 : eRXP_BCN_BSSID2);

    }
    }
    // Release the Broadcast/Multicast DPU Descriptor associated
    // with this BSS
    status = halTable_GetBssDpuIdx( pMac, (tANI_U8) pDelBssReq->bssIdx, &bcastDpuIdx );
    if( eHAL_STATUS_SUCCESS != status)
    {
        HALLOGW( halLog( pMac, LOGW, FL("No DPU associated with BSS %d!\n"), pDelBssReq->bssIdx ));
    }
    else
    {
        status = halDpu_ReleaseDescriptor( pMac, bcastDpuIdx );
        if( eHAL_STATUS_SUCCESS != status)
            HALLOGW( halLog( pMac, LOGW, FL("Unable to release the DPU descriptor for BSS %d!\n"),
            pDelBssReq->bssIdx) );

        halTable_SetBssDpuIdx(pMac, (tANI_U8)pDelBssReq->bssIdx, HAL_INVALID_KEYID_INDEX);
    }

    // Release the Broadcast/Multicast Mgmt DPU Descriptor associated with this BSS

    status = halTable_GetBssBcastMgmtDpuIdx( pMac, (tANI_U8) pDelBssReq->bssIdx, &bcastMgmtDpuIdx );
    if( eHAL_STATUS_SUCCESS != status)
    {
            HALLOGW( halLog( pMac, LOGW, FL("No Mgmt DPU associated with BSS %d!\n"), pDelBssReq->bssIdx ));
    }
    else
    {
        status = halDpu_ReleaseDescriptor( pMac, bcastMgmtDpuIdx );
        if( eHAL_STATUS_SUCCESS != status)
                HALLOGW( halLog( pMac, LOGW, FL("Unable to release the MGMT DPU descriptor for BSS %d!\n"),
                pDelBssReq->bssIdx ));

        halTable_SetBssBcastMgmtDpuIdx(pMac, (tANI_U8)pDelBssReq->bssIdx, HAL_INVALID_KEYID_INDEX);
    }

    // In IBSS mode restore the DPU settings for the self sta to original DPU Index of 0.
    if ( BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE  || BssSystemRole == eSYSTEM_BTAMP_AP_ROLE 
            || BssSystemRole == eSYSTEM_BTAMP_STA_ROLE)
    {
        if (wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *) selfAddr, &cfgLen) != eSIR_SUCCESS)
        {
            HALLOGE(halLog(pMac, LOGE, FL("WNI_CFG_STA_ID get failed \n")));
            return;
        }
        status = halTable_FindStaidByAddr(pMac, selfAddr, &selfIdx);
        if (status != eHAL_STATUS_SUCCESS){
            HALLOGE(halLog(pMac, LOGE, FL("Unable to locate self STA entry\n")));
        }
        // restore it back to the original settings.
        halTable_SetStaDpuIdx(pMac, (tANI_U8)selfIdx, (tANI_U8)pMac->hal.halMac.selfStaDpuId);
        halTable_SetStaBcastDpuIdx(pMac, (tANI_U8)selfIdx, (tANI_U8)pMac->hal.halMac.selfStaDpuId);
        halTable_SetStaBcastMgmtDpuIdx(pMac, (tANI_U8)selfIdx, (tANI_U8)pMac->hal.halMac.selfStaDpuId);
    }

    __halMsg_update11bCoexist(pMac, 0 );

    if(halMsg_allBssShortSlotEnabled(pMac))
    {
        pMac->hal.halMac.fShortSlot = 1;
        halMTU_updateTimingParams(pMac, MODE_11G_PURE);
    }

#ifdef HAL_BCAST_STA_PER_BSS
    if (BssSystemRole == eSYSTEM_AP_ROLE) {
        halMsg_DelBcastSta(pMac, pDelBssReq->bssIdx, pDelBssReq);
    }
#endif

    // free the sta
    halTable_ClearSta(pMac, staIdx);

#ifndef WLAN_SOFTAP_FEATURE    
    /// Deactivate PRE_BEACON TIMER_5 interrupt
    if ((BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE) ||
            (BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) ||
        (BssSystemRole == eSYSTEM_BTAMP_AP_ROLE))
    {
        if ( halMTU_DeactivateTimer(pMac, MTUTIMER_BEACON_PRE) != eHAL_STATUS_SUCCESS)
            return;
        // Turns off TBTT enable flag and disable beacon transmission.
        halMTU_DisableBeaconTransmission(pMac);
    }
#endif

    /** Reset the valid bssid index for the bss */
    if (halMTU_UpdateValidBssid(pMac, (tANI_U8) pDelBssReq->bssIdx, eHAL_CLEAR) != eHAL_STATUS_SUCCESS) {

        // This will update the Global HAL System Role
        // Since we dont know all the Bss that are
        // active, we pass the unknown state
        halSetGlobalSystemRole(pMac, eSYSTEM_UNKNOWN_ROLE);
    }

        /** Disable the beacon transmission for that BSS */
    halTpe_UpdateMtuMaxBssid(pMac);

    // Deactivate autoBaSetupTimer
    if (true == pMac->hal.halMac.baAutoSetupEnabled) {
         pMac->hal.halMac.baAutoSetupEnabled = false;
         halDeactivateAndChangeTimer(pMac, eHAL_BA_ACT_CHK_TIMER);
    }

    halRxp_DisableBssBeaconParamFilter(pMac, pDelBssReq->bssIdx);

      // This will update the Global HAL System Role. 
      // Since we dont know all the Bss that are active, we pass the unknown state 
    halSetGlobalSystemRole(pMac, eSYSTEM_UNKNOWN_ROLE);
      
   // free the bss locally
   status = halTable_ClearBss(pMac, (tANI_U8) pDelBssReq->bssIdx);

generate_response:
#ifndef WLAN_SOFTAP_FEATURE
//    halMacRaDelBssReq(pMac, (tANI_U8) pDelBssReq->bssIdx);
#endif
    if(BssSystemRole == eSYSTEM_STA_ROLE)
    {
        //Infra Session being torn down. So inform BTC module in FW.
        halFW_SendConnectionStatusMesg(pMac, eSIR_LINK_IDLE_STATE);
    }
    sirCopyMacAddr(pDelBssReq->bssid, bssId);
invalid_request:
    pDelBssReq->status = status;
    if(pDelBssReq->respReqd)
        halMsg_GenerateRsp(pMac, SIR_HAL_DELETE_BSS_RSP, dialog_token, (void *) pDelBssReq, 0);
    return;
}

#ifdef WLAN_SOFTAP_FEATURE
/*
    message handler to update Uapsd parameters.
*/
void halMsg_UpdateUapsd(tpAniSirGlobal pMac, tpUpdateUapsdParams msg)
{
    halBmu_UpdateStaBMUApMode(pMac, msg->staIdx, msg->uapsdACMask, msg->maxSpLen, 1 /* update Only Uapsd settings */);
}
#endif

/*
*  fn halEnableTLTx
 * DESCRIPTION:
 *      Function to enable TL TX..
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN VALUE:
 *      eHalStatus
 */
eHalStatus halEnableTLTx(tpAniSirGlobal pMac)
{
#ifdef BMU_FATAL_ERROR
    tANI_U8 staId = 0;
#endif
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eHalStatus tmpStatus = eHAL_STATUS_SUCCESS;

    // This piece of code, is to be investigated for 
    // BMU fatal errors. Currently this causes BMU fatal error on Android.
#ifdef BMU_FATAL_ERROR
    // Enable TX queues.
    for( staId = 0 ; staId < pMac->hal.memMap.maxStations ; staId++ ) {
        // Exclude the selfSTA TX queues as this was not disabled during 
        // INIT SCAN
        // Check the validity of the station first
        if ((tmpStatus = halTable_ValidateStaIndex( pMac, staId ) ) == eHAL_STATUS_SUCCESS) {

            halTable_GetStaType(pMac, staId, &staType);
            if ( staType == STA_ENTRY_SELF) {
            continue;
            }

            // Enable the TX queues in the BMU
            if( (tmpStatus = halBmu_sta_enable_disable_control( pMac, staId,
                            eBMU_ENB_TX_QUE_ENB_TRANS )) != eHAL_STATUS_SUCCESS ) {
                HALLOGE(halLog( pMac, LOGE, 
                            FL("Disabling Tx queue failed for staId %d"), staId ));
                status = (status == eHAL_STATUS_SUCCESS) ? tmpStatus : status;
            }
        }
    }
#else

    // Enable data backoffs
    halMTU_startBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);

#endif

    tmpStatus = halTLResumeTx(pMac, NULL);
    // Resume the transmission in TL, NULL specifies resume all STA
    if( tmpStatus  != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE, FL(" TL failed resuming Tx queue")));
        status = (status == eHAL_STATUS_SUCCESS) ? tmpStatus : status;
    }
    return status;
}


void halMsg_ScanComplete(tpAniSirGlobal pMac)
{
    tANI_U8 needToBeaconFlag = FALSE;
    tANI_U8 staId = 0;
    tBssSystemRole  BssSystemRole = eSYSTEM_UNKNOWN_ROLE;

    pMac->hal.scanParam.linkState = eSIR_LINK_FINISH_SCAN_STATE;
    pMac->hal.scanParam.isScanInProgress = eANI_BOOLEAN_FALSE;
    if(eHAL_STATUS_SUCCESS != halEnableTLTx(pMac))
    {
        HALLOGE(halLog(pMac, LOGE, FL("TL TX failed\n")));
    }
    halRxp_setScanLearn( pMac, FALSE );

    // Set RXP filter appropriately.
    //halRxp_setRxpFilterMode( pMac, halRxp_getRxpMode(pMac), NULL );
    // Clear the global RXP filter appropriately. Only the
    // Bss filter mode should be applicable now.
    halRxp_setSystemRxpFilterMode(pMac,
            eRXP_IDLE_MODE, eHAL_USE_BSS_RXP);

    for( staId = 0 ; staId < pMac->hal.memMap.maxStations ; staId++ ) {
        BssSystemRole = halGetBssSystemRoleFromStaIdx(pMac, staId);
        if ((BssSystemRole == eSYSTEM_BTAMP_AP_ROLE) ||
                (BssSystemRole == eSYSTEM_BTAMP_STA_ROLE) ||
                (BssSystemRole == eSYSTEM_STA_IN_IBSS_ROLE))
        {
            needToBeaconFlag = TRUE;
            break;
        }
    }

    // Enable beacon transmission in case of IBSS and BTAMP STA and AP mode.
    // Hence set enable flag = TRUE.
    if (needToBeaconFlag == TRUE)
    {
        halMTU_EnableDisableBssidTBTTBeaconTransmission(pMac,
            pMac->hal.halMac.beaconInterval, TRUE);
    }
}

/*
*  fn halMsg_SendInitScanResp
 * DESCRIPTION:
 *      Function to send Init scan response asynchronously.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      txComplete: 1 = successfully completed, 0 = otherwise.
 *
 * RETURN VALUE:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halMsg_SendInitScanResp(tpAniSirGlobal pMac, tANI_U32 txCompleteSuccess)
{
    tpInitScanParams    param = pMac->hal.scanParam.pReqParam;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    pMac->hal.scanParam.pReqParam = NULL;
    if(param)
    {
        if(!txCompleteSuccess)
        {
            halFW_SendScanStopMesg(pMac);

            status = eHAL_STATUS_FAILURE;
            halMsg_ScanComplete(pMac);

            //failure case. make sure we are out of dot11 pwr save
            HALLOGE(halLog(pMac, LOGE, FL("Ack timed out, Sending NULL frame once again\n")));
            param->macMgmtHdr.fc.powerMgmt = 0;
            halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                    param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                    param->frameLength, 0, HAL_USE_SELF_STA_REQUESTED_MASK);

        } else {
            // After receiving Null Data ACK, send SCAN start message to FW.
            halFW_SendScanStartMesg(pMac);
        }

        param->status = status;
        HALLOG1(halLog(pMac, LOG1, FL("Sending INIT_SCAN_RSP to LIM (status %d)\n"), param->status));
        halMsg_GenerateRsp(pMac, SIR_HAL_INIT_SCAN_RSP, pMac->hal.scanParam.dialog_token, (void *) param, 0);
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("Something terribly wrong here! param should not be NULL.\n")));
    }


    return status;
}

///////////////////////////////////////////////////////////////////////
//                 SCAN API
//////////////////////////////////////////////////////////////////////
eHalStatus halMsg_HandleInitScan( tpAniSirGlobal pMac, tpInitScanParams param, tANI_U32* pWaitForTxComp )
{
#ifdef BMU_FATAL_ERROR
    tANI_U8 staId, staType;
#endif
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 psState;

    // Suspend the transmission from TL, for ALL STA by passing NULL in the 
    // staID and then resume only the self sta
    if((status = halTLSuspendTx(pMac, NULL)) != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("TL failed in suspending TX queue for all STA")));
        // Check if the system is in Power save state
        psState = halPS_GetState(pMac);
        if (psState & HAL_PWR_SAVE_SUSPEND_BMPS_STATE) {
            halPS_ResumeBmps(pMac, 0, NULL, (void*)param, FALSE);
        }
        return eHAL_STATUS_FAILURE;
    }

#ifdef BMU_FATAL_ERROR
    {
        tANI_U8 staId, staType;    
    // Self STA is used for sending the management frames, so disable all the
    // TX queues of all the STAs except for the Self STA entry
    for( staId = 0 ; staId < pMac->hal.memMap.maxStations ; staId++ ) {

        // Check the validity of the station first
        if ((status = halTable_ValidateStaIndex(pMac, staId)) == eHAL_STATUS_SUCCESS) {
        // Self sta is used to send management frames
            halTable_GetStaType(pMac, staId, &staType);
            if ( staType == STA_ENTRY_SELF) {
            continue;
            }

            // Disable the TX queue in BMU
            if( (status = halBmu_sta_enable_disable_control( pMac, staId,
                            eBMU_ENB_TX_QUE_DONOT_ENB_TRANS )) != eHAL_STATUS_SUCCESS ) {
                HALLOGE( halLog(  pMac, LOGE,
                        FL("Disabling BTQM Tx queue failed for staId %d"), staId ));
            }
        }
    }
    }
#else

    // Disable data backoffs
    halMTU_stallBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);

#endif

    // Set RXP routing flag with scanbit, to distingiush packets recvd in scan mode.
    halRxp_setScanLearn( pMac, TRUE );
    status = eHAL_STATUS_SUCCESS;

    // Check if notification to BSS/STAs is required by sending Data Null or self CTS
    if(param->notifyBss && param->frameLength) {
        // Send Data Null only if in full power
        if(halPS_GetState(pMac) == HAL_PWR_SAVE_ACTIVE_STATE) {

            if(pMac->hal.pCBackFnTxComp == NULL) {
                pMac->hal.pCBackFnTxComp = halMsg_SendInitScanResp;
                // wait timer start
                if(TX_SUCCESS != tx_timer_activate(&pMac->hal.txCompTimer)) {
                    status = eHAL_STATUS_FAILURE;
                    HALLOGP(halLog(pMac, LOGP, FL("txCompTimer Could not get activated\n")));
                goto out;
                }

                // Send the Data Null frame
                status = halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                        param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                        param->frameLength, 0, (HAL_TXCOMP_REQUESTED_MASK | HAL_USE_SELF_STA_REQUESTED_MASK));
                if(eHAL_STATUS_SUCCESS == status) {
                    *pWaitForTxComp = 1;
                } else {
                    pMac->hal.pCBackFnTxComp = NULL;
                    *pWaitForTxComp = 0;

                    //wait timer stop
                    if(TX_SUCCESS != tx_timer_deactivate(&pMac->hal.txCompTimer)) {
                        status = eHAL_STATUS_FAILURE;
                        HALLOGP(halLog(pMac, LOGP, FL("txCompTimer Could not get deactivated\n")));
                        goto out;
                    }
                    HALLOGE(halLog(pMac, LOGE, FL("Failed to send NULL frame\n")));
                }
            } else {
                *pWaitForTxComp = 0;
                HALLOGE(halLog(pMac, LOGE, FL("There is already one request for TxComplete pending\n")));
                status = eHAL_STATUS_FAILURE;
            }
        }
    } else {
        // Send Start scan message to FW. This would be the case when STA is not
        // associated.
        halFW_SendScanStartMesg(pMac);
    }
out:
    if(status != eHAL_STATUS_SUCCESS)
        halFW_SendScanStopMesg(pMac);

    // Make sure to Disable beacon transmission. Hence enable flag is set to FALSE.
    halMTU_EnableDisableBssidTBTTBeaconTransmission(pMac,
            pMac->hal.halMac.beaconInterval, FALSE);

    return status;
}

/* Function to be called when in Power save, after BMPS is suspended in FW */
void halMsg_HandlePSInitScan(tpAniSirGlobal pMac, void* param,
                tANI_U16 dialog_token, tANI_U32 psStatus)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpInitScanParams pInitScanParam = (tpInitScanParams)param;
    tANI_U32 waitForTxComp = 0;

    // Check the status from the suspend bmps procedure
    if(psStatus != eHAL_STATUS_SUCCESS) {
        status = eHAL_STATUS_FAILURE;
        goto generate_response;
    }

    // Now handle the Init Scan
    status = halMsg_HandleInitScan( pMac, pInitScanParam, &waitForTxComp );
    if (status != eHAL_STATUS_SUCCESS)
        goto generate_response;

    pMac->hal.scanParam.pReqParam = param;
    pMac->hal.scanParam.linkState = eSIR_LINK_SCAN_STATE;
    pMac->hal.scanParam.dialog_token = dialog_token;
    pMac->hal.scanParam.isScanInProgress = eANI_BOOLEAN_TRUE;

	// Send start scan message to FW once FW is suspended from BMPS
	halFW_SendScanStartMesg(pMac);

generate_response:
    //if TxComplete is required then sending the response will be deferred until
    //either wait timer is expired or the response is received.
    if(!waitForTxComp)
    {
        HALLOG1(halLog(pMac, LOG1, FL("Sending INIT_SCAN_RSP to LIM (status %d)\n"), status));
        if(eHAL_STATUS_SUCCESS != status)
            halMsg_ScanComplete(pMac);

        pMac->hal.scanParam.pReqParam = NULL;
        pInitScanParam->status = status;
        halMsg_GenerateRsp(pMac, SIR_HAL_INIT_SCAN_RSP, dialog_token,
                (void *) pInitScanParam, 0);
    }
    return;
}



/* -------------------------------------------------------
 * FUNCTION: halMsg_InitScan()
 *
 * NOTE:
 *   HAL changes channel and then sends a "SIR_HAL_INIT_SCAN_RSP" to LIM
 *   with the status code.
 *
 *   If notifyBss is 0, that means PE does not have any
 *   frames to send. Hence, set frameLen to 0 to notify
 *   to softMac that it does not need to transmit a frame.
 *   Otherwise, softmac will need to send either DATA NULL
 *   or CTS-TO-SELF.
 *
 *                     AP            STATION
 *    -----------------------------------------
 *    LEARN MODE:   CTS-TO-SELF      Nothing
 *    SCAN  MODE:   NOTHING          DATA NULL
 * -------------------------------------------------------
 */
void
halMsg_InitScan(
    tpAniSirGlobal      pMac,
    tANI_U16            dialog_token,
    tpInitScanParams    param)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    // This is a global link state.
    tSirLinkState  linkState = eSIR_LINK_SCAN_STATE;
    tANI_U32 waitForTxComp = 0;
    tANI_U8 psState;

    /* Check if Periodic Calibration is in progress, if true
     * notify failure to LIM.
     */
    if (pMac->hal.scanParam.linkState == eSIR_LINK_INIT_CAL_STATE)
    {
        status = eHAL_STATUS_FAILURE;
        goto generate_response;
    }

#ifdef ANI_PRODUCT_TYPE_AP
    if (param->scanMode == eHAL_SYS_MODE_LEARN)
        linkState = eSIR_LINK_LEARN_STATE;
#endif
    /* If HAL is suppose to check for link traffic, and if
     * link is busy, If link is busy, then skip scan
     */
    if (param->checkLinkTraffic == 1)
    {
        if (halIsLinkBusy(pMac) == TRUE )
        {
            status = eHAL_STATUS_FAILURE;
            goto generate_response;
        }
    }

    // Check if the system is in Power save state
    psState = halPS_GetState(pMac);
    if ((psState & HAL_PWR_SAVE_BMPS_STATE) ||
        (psState & HAL_PWR_SAVE_UAPSD_STATE)) {
        // Suspend FW from BMPS
        status = halPS_SuspendBmps(pMac, dialog_token,
                halMsg_HandlePSInitScan, (void*)param);
        if (status != eHAL_STATUS_SUCCESS) {
            goto generate_response;
        }

        // Sending of the response message back to PE is done in the
        // callback function, so we return here.
        return;
    } else {

        status = halMsg_HandleInitScan( pMac, param, &waitForTxComp );
        if (status != eHAL_STATUS_SUCCESS)
            goto generate_response;

        pMac->hal.scanParam.pReqParam = param;
        pMac->hal.scanParam.linkState = linkState;
        pMac->hal.scanParam.dialog_token = dialog_token;
        pMac->hal.scanParam.isScanInProgress = eANI_BOOLEAN_TRUE;
    }

    // Send response to PE
generate_response:
    //if TxComplete is required then sending the response will be deferred until
    //either wait timer is expired or the response is received.
    if(!waitForTxComp)
    {
        HALLOG1( halLog(pMac, LOG1, FL("Sending INIT_SCAN_RSP to LIM (status %d)\n"), status));
        if(eHAL_STATUS_SUCCESS != status)
            halMsg_ScanComplete(pMac);

        pMac->hal.scanParam.pReqParam = NULL;
        param->status = status;
        halMsg_GenerateRsp(pMac, SIR_HAL_INIT_SCAN_RSP, dialog_token, (void *) param, 0);
    }
    return;
}

void halMsg_StartScanPostSetChan(tpAniSirGlobal pMac, void* pData,
        tANI_U32 status, tANI_U16 dialog_token)
{
    tpStartScanParams param = (tpStartScanParams)pData;
    tTpeRateIdx        defaultRateIdx, bcastRateIdx;
    eRfBandMode        rfBand;
    tANI_U8            currChannel = pMac->hal.currentChannel;
#ifdef WLAN_FEATURE_VOWIFI
    tPowerdBm             regPowerLimit = 0;
    tTpeRateIdx           rateIndex;
    eHalPhyRates          phyRateIndex;
    t2Decimal             pwrDbm;
#endif

    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() failed \n")));
        goto generate_response;
    }
    if ((currChannel >= SIR_11A_CHANNEL_BEGIN) &&
        (currChannel <= SIR_11A_CHANNEL_END)) {
        rfBand = eRF_BAND_5_GHZ;
    } else {
        rfBand = eRF_BAND_2_4_GHZ;
    }

    status = halGetDefaultAndMulticastRates(pMac, rfBand, &defaultRateIdx, &bcastRateIdx);

#if 0
    // TODO: Updating the Tx Power should be done on TPE descriptor as the Mgmt Probe would go
    // out at the power index specified in TPE descriptor power for staIdx 0. Also need to
    // update the TxPower for all the Ctrl/Rsp rates in the MPI cmd table as Ack can go out at
    // any rate depending on the rate at which probe response is received.
    if(eHAL_STATUS_SUCCESS == status) {
        //now update the rate-to-power table for bCast rate only.
        halRate_UpdateTpeTxPowerRateEntry(pMac, bcastRateIdx) ;
    }
#endif

    // Set RXP filter appropriately.
    //halRxp_setRxpFilterMode( pMac, eRXP_SCAN_MODE, NULL );
    halRxp_setSystemRxpFilterMode(pMac,
        eRXP_SCAN_MODE, eHAL_USE_GLOBAL_AND_BSS_RXP);

#if defined WLAN_FEATURE_VOWIFI
    //Collect start TSF of the scan.
    halMTU_GetTsfTimer( pMac, &param->startTSF[0], &param->startTSF[1] );
    
    /* Need  to return the power for management frames in the response */
    halGetNonBcnRateIdx(pMac,(tTpeRateIdx *) &rateIndex);
    
    /* Get the regulatory power limit for the current channel and calculate the 
        power in dBm for management frames */
    halUtil_GetRegPowerLimit(pMac, pMac->hal.currentChannel, 0, &regPowerLimit);
    phyRateIndex = halRate_MacRateIdxtoPhyRateIdx(pMac, rateIndex);
    if (HAL_PHY_RATE_INVALID == phyRateIndex)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid Phy rate index for management frames, setting default")));
        phyRateIndex = HAL_PHY_RATE_11B_LONG_1_MBPS;
    }
    status = halPhyGetPwrFromRate2PwrTable(pMac, phyRateIndex, &pwrDbm);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGP(halLog(pMac, LOGP, FL("Get power from rate2pwr table for rateindex %d failed with status %d\n"), phyRateIndex, status));
    }
    pwrDbm /= (t2Decimal)100;      //The halPhyGetPwrFromRate2PwrTable provides pwrDbm as 1900 for 19dBm. So, divide by 100
    param->txMgmtPower = (tPowerdBm)VOS_MIN(regPowerLimit, pwrDbm);
    
    /* The power returned to the PE is used to fill the TX power in WFA TPC report in 802.11k Link measurement frames 
            and probe request frames */
    
    HALLOGE(halLog(pMac, LOGE, FL("Start scan Rsp: Mgmt Rate Index = %d, Power = %d"), rateIndex, param->txMgmtPower);)
#endif
    // response to PE
generate_response:
    HALLOG1( halLog(pMac, LOG1, FL("Send SIR_HAL_START_SCAN_RSP to LIM (status %d)\n"), status));
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_START_SCAN_RSP, dialog_token, (void *) param, 0);
    return;
}

/* ----------------------------------------------------
 * FUNCTION: halMsg_StartScan()
 *
 * NOTE:
 *   HAL shall switch to the new channel given in the
 *   tpStartScanParams structure, and then send a
 *   "SIR_HAL_START_SCAN_RSP" to LIM.
 * ----------------------------------------------------
 */
void
halMsg_StartScan(
    tpAniSirGlobal      pMac,
    tANI_U16            dialog_token,
    tpStartScanParams   param)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    // Save the current channel and switch to new scan channel
    HALLOG1( halLog(pMac, LOG1, FL("StartScan (Channel = %d) \n"), param->scanChannel));

    status = halPhy_ChangeChannel(pMac, param->scanChannel,
            PHY_SINGLE_CHANNEL_CENTERED, FALSE, halMsg_StartScanPostSetChan, param, dialog_token);
    // If channel is already on the request channel, proceed further with
    // post set channel configuration
    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        halMsg_StartScanPostSetChan(pMac, (void*)param, eHAL_STATUS_SUCCESS, dialog_token);
    } else {
        if ( status != eHAL_STATUS_SUCCESS ) {
            HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() failed \n")));
            param->status = status;
            halMsg_GenerateRsp(pMac, SIR_HAL_START_SCAN_RSP, dialog_token, (void *) param, 0);
        }
    }

    return;
}


/* ---------------------------------------------------
 * FUNCTION: halMsg_ChannelSwitch()
 *
 * NOTE:
 *   HAL shall switch to the new channel and cbState
 *   given in the tpSwitchChannelParams structure.
 * ---------------------------------------------------
 */
void halMsg_ChannelSwitchPostSetChan(tpAniSirGlobal pMac, void *pData,
        tANI_U32 status, tANI_U16 dialog_token)
{
    tpSwitchChannelParams param = (tpSwitchChannelParams)pData;
#ifdef WLAN_FEATURE_VOWIFI
    tANI_U8               bssIdx;
    tTpeRateIdx           rateIndex;
    eHalPhyRates          phyRateIndex;
    tPwrTemplateIndex     pwrIndex;
    bssRaParam            bssRaInfo;
    tANI_U8               staIdx;
    t2Decimal             pwrDbm;
#endif /* WLAN_FEATURE_VOWIFI */

    if ( status != eHAL_STATUS_SUCCESS ){
        HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() to Ch %d failed \n"), param->channelNumber));
        goto generate_response;
    }

        
    //update whole TPE rate power table.
#ifdef FEATURE_TX_PWR_CONTROL
    status = halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)HALRATE_MODE_START, (tTpeRateIdx)HAL_MAC_MAX_TX_RATES, TRUE);
#else	
    status = halRate_UpdateRateTablePower(pMac, (tTpeRateIdx)MIN_LIBRA_RATE_NUM, (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM, TRUE);
#endif	
    if ( status != eHAL_STATUS_SUCCESS ){
        HALLOGE( halLog(pMac, LOGE, FL("Failed updating Tx power\n")));
        goto generate_response;
    }

#ifdef WLAN_FEATURE_VOWIFI
            
    if (eHAL_STATUS_SUCCESS != halTable_FindBssidByAddr(pMac, param->bssId, &bssIdx))
    {
        HALLOGE(halLog(pMac, LOGE, FL("Not a valid BSS IDX")));
        goto generate_response;
    }
    else
    {

        tANI_U8      rfBand;

        rfBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);
        halTable_GetBssRaConfig(pMac, rfBand, &bssRaInfo, bssIdx);
        //Should convert the max Tx power to power index and save it in RA BSS info
        /* Now we should get the power index by taking maxTxPower(power constraint) into consideration
         * and set this power index as the maxPwrIndex in RA BSS Info. No frames should be transmitted 
         * with the power value greater than this */
        halPhyGetMaxTxPowerIndex(pMac, param->maxTxPower, &pwrIndex);
        bssRaInfo.bit.maxPwrIndex = pwrIndex;
        halTable_SaveBssConfig(pMac, rfBand, bssRaInfo, bssIdx);
        HALLOGE(halLog(pMac, LOGE, FL("Channel Switch: BSSIDX = %d, Power(dBm) = %d, Pwr Index = %d"),
                                    bssIdx, param->maxTxPower, bssRaInfo.bit.maxPwrIndex));
    }

    //Need to update the power for the BD rates in the TPE station descriptor
    halTable_FindStaidByAddr(pMac , param->selfStaMacAddr, &staIdx);
    halTpe_UpdateDescPowerParams(pMac, staIdx, TPE_STA_BD_RATE, pwrIndex);
#endif /* WLAN_FEATURE_VOWIFI */    
    
generate_response:
#ifdef WLAN_FEATURE_VOWIFI
    /* Need  to return the power for management frames in the response */
    halGetNonBcnRateIdx(pMac,(tTpeRateIdx *) &rateIndex);

    phyRateIndex = halRate_MacRateIdxtoPhyRateIdx(pMac, rateIndex);
    if (HAL_PHY_RATE_INVALID == phyRateIndex)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Invalid Phy rate index for management frames, setting default")));
        phyRateIndex = HAL_PHY_RATE_11B_LONG_1_MBPS;
    }
    status = halPhyGetPwrFromRate2PwrTable(pMac, phyRateIndex, &pwrDbm);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGP(halLog(pMac, LOGP, FL("Get power from rate2pwr table for rateindex %d failed with status %d\n"), phyRateIndex, status));
    }
    pwrDbm /= 100;      //The halPhyGetPwrFromRate2PwrTable provides pwrDbm as 1900 for 19dBm. So, divide by 100
    /* The power returned to the PE is used to fill the TX power in WFA TPC report in 802.11k Link measurement frames 
       and probe request frames */
    param->txMgmtPower = VOS_MIN(param->maxTxPower, pwrDbm);
    HALLOGE(halLog(pMac, LOGE, FL("Channel Switch Rsp: Mgmt Rate Index = %d, Power = %d"),
                                        rateIndex, param->txMgmtPower));
#endif /* WLAN_FEATURE_VOWIFI */    

    HALLOG1(halLog(pMac, LOG1, FL("Send SIR_HAL_SWITCH_CHANNEL_RSP to LIM (status %d)\n"), status));
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_SWITCH_CHANNEL_RSP, dialog_token, (void *) param, 0);


    return;
}


void
halMsg_ChannelSwitch(
    tpAniSirGlobal         pMac,
    tpSwitchChannelParams  param)
{
    eHalStatus         status = eHAL_STATUS_SUCCESS;
    ePhyChanBondState  cbState = PHY_SINGLE_CHANNEL_CENTERED;

    cbState = (ePhyChanBondState)param->secondaryChannelOffset;

    // Save the current channel and switch to new scan channel
    HALLOGW( halLog(pMac, LOGW, FL("ChannelSwitch to chId %d, cbState %d) \n"), param->channelNumber, cbState));
#ifndef WLAN_FEATURE_VOWIFI
    pMac->hal.gHalLocalPwrConstraint = param->localPowerConstraint;
#endif /* WLAN_FEATURE_VOWIFI */

    status = halPhy_ChangeChannel(pMac, param->channelNumber, cbState, TRUE, halMsg_ChannelSwitchPostSetChan, (void*)param, 0);

    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        halMsg_ChannelSwitchPostSetChan(pMac, (void *)param, eHAL_STATUS_SUCCESS, 0);
    } else {
        if ( status != eHAL_STATUS_SUCCESS ){
            HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() to Ch %d failed \n"), param->channelNumber));
            param->status = status;
            halMsg_GenerateRsp(pMac, SIR_HAL_SWITCH_CHANNEL_RSP, 0, (void *) param, 0);
        }
    }

    return;
}

#if 0
void
halMsg_ChannelSwitch(
    tpAniSirGlobal         pMac,
    tpSwitchChannelParams  param)
{
    eHalStatus         status = eHAL_STATUS_SUCCESS;
    ePhyChanBondState  cbState = PHY_SINGLE_CHANNEL_CENTERED;

    cbState = (ePhyChanBondState)param->secondaryChannelOffset;

    // Save the current channel and switch to new scan channel
    HALLOGW( halLog(pMac, LOGW, FL("ChannelSwitch to chId %d, cbState %d) \n"), param->channelNumber, cbState));
    pMac->hal.gHalLocalPwrConstraint = param->localPowerConstraint;

    //printf("Channel Number in halMsg_ChannelSwitch %d\n", param->channelNumber);
    status = halPhy_ChangeChannel(pMac, param->channelNumber, cbState, TRUE, halMsg_ChannelSwitchPostSetChan, (void*)param, 0);

    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        halMsg_ChannelSwitchPostSetChan(pMac, (void *)param, eHAL_STATUS_SUCCESS, 0);
    } else {
        if ( status != eHAL_STATUS_SUCCESS ){
            HALLOGE( halLog(pMac, LOGE, FL("halChangeChannel() to Ch %d failed \n"), param->channelNumber));
            param->status = status;
            halMsg_GenerateRsp(pMac, SIR_HAL_SWITCH_CHANNEL_RSP, 0, (void *) param, 0);
        }
    }
   return;
}
#endif
/* ---------------------------------------------------
 * FUNCTION: halMsg_EndScan()
 *
 * NOTE:
 *   HAL shall switch back to the operational channel
 *   given in the tpEndScanParams structure, and then
 *   send a "SIR_HAL_END_SCAN_RSP" to LIM.
 * ---------------------------------------------------
 */
void
halMsg_EndScan(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpEndScanParams param)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    // TODO - Switch to saved (operational) channel
    //              should have functionality similar to dphChannelChange()

    // Set RXP filter appropriately.
    //halRxp_setRxpFilterMode( pMac, halRxp_getRxpMode(pMac), NULL );
    // We are done with scan so only the bss is valid
    halRxp_setSystemRxpFilterMode(pMac,
        eRXP_IDLE_MODE, eHAL_USE_BSS_RXP);

    // response back to PE
//generate_response:
    HALLOG1( halLog(pMac, LOG1, FL("Sending SIR_HAL_END_SCAN_RSP to LIM (status %d)\n"), status));
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_END_SCAN_RSP, dialog_token, (void *) param, 0);
    return;
}


/*
*  fn halMsg_SendFinishScanResp
 * DESCRIPTION:
 *      Function to send Finish scan response asynchronously.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      txCompleteSuccess: 1 = Tx Completed successfully, 0 = otherwise.
 *
 * RETURN VALUE:
 *      eHalStatus
 */
eHalStatus halMsg_SendFinishScanResp(tpAniSirGlobal pMac, tANI_U32 txCompleteSuccess)
{
    tpFinishScanParams param = pMac->hal.scanParam.pReqParam;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    pMac->hal.scanParam.pReqParam = NULL;

    if(param)
    {
        if(!txCompleteSuccess)
        {
            //failure case. try one last time without interrupt Acked response
            HALLOGE(halLog(pMac, LOGE, FL("Ack Timed out, sending NULL frame once again\n")));
            halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                    param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                    param->frameLength, 0, HAL_USE_SELF_STA_REQUESTED_MASK);
            status = eHAL_STATUS_FAILURE;
        }

        param->status = status;
        HALLOG1(halLog(pMac, LOG1, FL("Sending FINISH_SCAN_RSP to LIM (status %d)\n"), param->status));
        halMsg_GenerateRsp(pMac, SIR_HAL_FINISH_SCAN_RSP, pMac->hal.scanParam.dialog_token, (void *) param, 0);

    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("Something terribly wrong here! param should not be NULL.\n")));
    }

    return status;
}

eHalStatus halMsg_HandleFinishScan( tpAniSirGlobal pMac, tpFinishScanParams param, tANI_U32* pWaitForTxComp )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eHalStatus tmpStatus = eHAL_STATUS_SUCCESS;

    // Clear the scan bit
    halRxp_setScanLearn( pMac, FALSE );

    // Set RXP filter appropriately.
    halRxp_setSystemRxpFilterMode(pMac, eRXP_IDLE_MODE, eHAL_USE_GLOBAL_AND_BSS_RXP);

    // Send the raw frame given by PE, to TL for transmission
    if (param->notifyBss && param->frameLength ) {
        if(halPS_GetState(pMac) == HAL_PWR_SAVE_ACTIVE_STATE) {
            if(pMac->hal.pCBackFnTxComp == NULL) {
                pMac->hal.pCBackFnTxComp = halMsg_SendFinishScanResp;
                // wait timer started.
                if(TX_SUCCESS != tx_timer_activate(&pMac->hal.txCompTimer)) {
                    status = eHAL_STATUS_FAILURE;
                    HALLOGP(halLog(pMac, LOGP, FL("could not activate txCompTimer\n")));
                    return status;
                }
                tmpStatus = halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                        param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                        param->frameLength, 0, (HAL_TXCOMP_REQUESTED_MASK | HAL_USE_SELF_STA_REQUESTED_MASK));
                if(tmpStatus == eHAL_STATUS_SUCCESS) {
                    *pWaitForTxComp = 1;
                } else {
                    pMac->hal.pCBackFnTxComp = NULL;
                    *pWaitForTxComp = 0;

                    //failure case. try one last time without interrupt Acked response
                    HALLOGE( halLog(pMac, LOGE, FL("halTLSend80211Frame failed. Sending NULL frame once again\n")));
                    halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                            param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                            param->frameLength, 0, HAL_USE_SELF_STA_REQUESTED_MASK);

                    //wait timer stopped.
                    if(TX_SUCCESS != tx_timer_deactivate(&pMac->hal.txCompTimer)) {
                        status = eHAL_STATUS_FAILURE;
                        HALLOGP(halLog(pMac, LOGP, FL("could not deactivate txCompTimer\n")));
                        return status;
                    }
                    status = (status == eHAL_STATUS_SUCCESS) ? tmpStatus : status;
                }
            } else {
                //failure case. try one last time without interrupt Acked response
                HALLOGE( halLog(pMac, LOGE, FL("not Null CB, Sending NULL frame once again\n")));
                halTLSend80211Frame(pMac, (void*) &param->macMgmtHdr,
                        param->macMgmtHdr.fc.type <<4 | param->macMgmtHdr.fc.subType,
                        param->frameLength, 0, HAL_USE_SELF_STA_REQUESTED_MASK);
                *pWaitForTxComp = 0;
                status = eHAL_STATUS_FAILURE;
            }
        }

    }

    // Resume back TL and the BTQM queues
    halMsg_ScanComplete(pMac);

    return status;
}

/* Function to be called when in Power save, after BMPS is Resumed  in FW */
void halMsg_HandlePSFinishScan(tpAniSirGlobal pMac, void* param,
                tANI_U16 dialog_token, tANI_U32 psStatus)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tpFinishScanParams pFinishScanParam = (tpFinishScanParams)param;
    tANI_U32 waitForTxComp = 0;

    // Check the status from the suspend bmps procedure
    if(psStatus != eHAL_STATUS_SUCCESS) {
        status = eHAL_STATUS_FAILURE;
        goto generate_response;
    }

    // Now handle the Finish Scan
    status = halMsg_HandleFinishScan(pMac, pFinishScanParam, &waitForTxComp );
    goto generate_response1;

generate_response:
    halMsg_ScanComplete(pMac);
generate_response1:
    if(!waitForTxComp)
    {
        HALLOG1(halLog(pMac, LOG1, FL("Sending FINISH_SCAN_RSP to LIM (status %d)\n"), status));
        pMac->hal.scanParam.pReqParam = NULL;
        pFinishScanParam->status = status;
        halMsg_GenerateRsp(pMac, SIR_HAL_FINISH_SCAN_RSP, dialog_token,
                (void *) pFinishScanParam, 0);
    }
    return;
}


/* ---------------------------------------------------------
 * FUNCTION: halMsg_FinishScan()
 * ---------------------------------------------------------
 */
/* ---------------------------------------------------------
 * FUNCTION: halMsg_FinishScan()
 * ---------------------------------------------------------
 */
void halMsg_FinishScanPostSetChan(tpAniSirGlobal pMac, void* pData,
        tANI_U32 status, tANI_U16 dialog_token)
{
    tANI_U32       waitForTxComp = 0;
    tTpeRateIdx    defaultRateIdx, bcastRateIdx;
    eRfBandMode    rfBand;
    tANI_U8        currChannel = pMac->hal.currentChannel;
    tANI_U8        psState;
    tpFinishScanParams param = (tpFinishScanParams)pData;

    if (status != eHAL_STATUS_SUCCESS) {
            goto generate_response;
    }

    if ((currChannel >= SIR_11A_CHANNEL_BEGIN) &&
        (currChannel <= SIR_11A_CHANNEL_END)) {
        rfBand = eRF_BAND_5_GHZ;
    } else {
        rfBand = eRF_BAND_2_4_GHZ;
    }

    // Get the broadcast rate
    status = halGetDefaultAndMulticastRates(pMac, rfBand, &defaultRateIdx, &bcastRateIdx);

#if 0
    // TODO: Updating the Tx Power should be done on TPE descriptor as the Mgmt Probe would go
    // out at the power index specified in TPE descriptor power for staIdx 0. Also need to
    // update the TxPower for all the Ctrl/Rsp rates in the MPI cmd table as Ack can go out at
    // any rate depending on the rate at which probe response is received.
    if(eHAL_STATUS_SUCCESS == status) {
        //now update the rate-to-power table for bCast rate only.
        halRate_UpdateTpeTxPowerRateEntry(pMac, bcastRateIdx);
    }
#endif

    pMac->hal.scanParam.pReqParam = param;

    //send SCAN stop message to FW.
    halFW_SendScanStopMesg(pMac);

    // Check if the system is in Power save state
    psState = halPS_GetState(pMac);
    if (psState & HAL_PWR_SAVE_SUSPEND_BMPS_STATE) {
        // Resume FW from BMPS
        status = halPS_ResumeBmps(pMac, dialog_token,
                halMsg_HandlePSFinishScan, (void*)param, TRUE);
        if (status != eHAL_STATUS_SUCCESS) {
            goto generate_response;
        }

        // Sending of the response message back to PE is done in the
        // callback function, so we return here.
        return;

    } else {
        status = halMsg_HandleFinishScan(pMac, param, &waitForTxComp);
        goto generate_response1;
    }

generate_response:
    halMsg_ScanComplete(pMac);

generate_response1:
    if(!waitForTxComp)
    {
        pMac->hal.scanParam.pReqParam = NULL;
        param->status = status;
        HALLOG1(halLog(pMac, LOG1, FL("sending FINISH SCAN rsp with status %d\n"), status));
        halMsg_GenerateRsp(pMac, SIR_HAL_FINISH_SCAN_RSP, dialog_token, (void *) param, 0);
    }

    return;
}

void
halMsg_FinishScan(
    tpAniSirGlobal      pMac,
    tANI_U16            dialog_token,
    tpFinishScanParams  param)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    HALLOG1(halLog(pMac, LOG1, FL("FinishScan (Channel = %d, extChannel = %d) \n"),
                param->currentOperChannel, param->cbState));

    //we should not change the channel if channel id is invalid.
    if(HAL_INVALID_CHANNEL_ID != param->currentOperChannel) {
        status = halPhy_ChangeChannel(pMac, param->currentOperChannel,
                (ePhyChanBondState)param->cbState, FALSE, halMsg_FinishScanPostSetChan, param, dialog_token);
        // If channel is already on the request channel, proceed further with
        // post set channel configuration
        if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
            halMsg_FinishScanPostSetChan(pMac, (void*)param, eHAL_STATUS_SUCCESS, dialog_token);
        } else {
            if (status != eHAL_STATUS_SUCCESS) {
                halMsg_ScanComplete(pMac);
                param->status = status;
                HALLOG1(halLog(pMac, LOG1, FL("sending FINISH SCAN rsp with status %d\n"), status));
                halMsg_GenerateRsp(pMac, SIR_HAL_FINISH_SCAN_RSP, dialog_token, (void *) param, 0);
            }
        }
    } else {
        // If the channel passed is invalid stay on the current channel and proceed further
        halMsg_FinishScanPostSetChan(pMac, (void*)param, eHAL_STATUS_SUCCESS, dialog_token);
    }

    return;
}


#ifdef ANI_SNIFFER

/** ----------------------------------------------------
 * FUNCTION:  halMsg_setPromiscMode()
 *
 * NOTE:
 *   Compose a ChangeSysMode message with mode set to
 *   "eHAL_SYS_MODE_PROMISC", and send it to MCPU.
 * ----------------------------------------------------
 */
eHalStatus halMsg_setPromiscMode(tpAniSirGlobal pMac)
{
    tSmacHostMesg_ChangeSysMode msgBuf, *pSysMode;

    pSysMode = (tSmacHostMesg_ChangeSysMode *) &msgBuf;
    if (palFillMemory(pMac->hHdd, (void *)pSysMode, sizeof (tSmacHostMesg_ChangeSysMode), 0) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("palFillMemory() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGW( halLog(pMac, LOGW, FL("Current RXP Mode = %d"), halRxp_getRxpMode(pMac)));


    {

        tSmacHostMesg_ChangeSysMode  msgBuf, *finishScan;
        eHalStatus     status = eHAL_STATUS_SUCCESS;

        // Compose message to send to softmac
        finishScan = (tSmacHostMesg_ChangeSysMode *) &msgBuf;
        palZeroMemory(pMac->hHdd, (void *)finishScan, sizeof (tSmacHostMesg_ChangeSysMode));
        finishScan->hdr.ver     = SMAC_HOSTMESG_VER;
        finishScan->hdr.msgType = SMAC_HOSTMESG_CHANGE_SYSMODE;
        finishScan->hdr.msgLen  = sizeof(tSmacHostMesg_ChangeSysMode);

        finishScan->mode = SMAC_SYSMODE_NORMAL;

        // Send message to softMAC
        if (halMbox_SendMsg(pMac, (void *) &msgBuf) != eHAL_STATUS_SUCCESS)
        {
            status = eHAL_STATUS_SOFTMAC_MSG_FAILURE;
            return status;
        }

        /* Restore the rxp mode to what it was prior to
         * INIT_SCAN. It should be either IDLE, PREASSOC,
         * POSTASSOC, or AP mode
         */
        halMsg_setLinkState(pMac, eSIR_LINK_FINISH_SCAN_STATE);
    }

    sirBusyWait(10000000);      //10ms delay for softmac to settle in normal mode

    // Fill message header
    pSysMode->hdr.ver     = SMAC_HOSTMESG_VER;
    pSysMode->hdr.msgType = SMAC_HOSTMESG_CHANGE_SYSMODE;
    pSysMode->hdr.msgLen  = sizeof(tSmacHostMesg_ChangeSysMode);

     // Fill message body
    pSysMode->frameLen = 0;               // No frame to send
    pSysMode->dialogToken = 0;
    pSysMode->noResponse = SMAC_ACK_RESPONSE_NOT_REQUIRED;
    pSysMode->mode = SMAC_SYSMODE_PROMISC;
    pSysMode->flushMgmtQueue = 0;

    // Send message to mCPU
    if (halMbox_SendMsg(pMac, (void *) &msgBuf) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halMbox_SendMsg() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // Set RXP Filter to promisc mode & Enable RXP to receive packets
    //halRxp_setRxpFilterMode(pMac, eRXP_PROMISCUOUS_MODE, NULL);

    return eHAL_STATUS_SUCCESS;
}

#endif //ANI_SNIFFER

#ifdef WLAN_SOFTAP_FEATURE

////////////////////////////////////////////////////////////////////////
//                     updating probeRsp template for FW.
///////////////////////////////////////////////////////////////////////
void
halMsg_UpdateProbeRspTemplate(
    tpAniSirGlobal      pMac,
    tpSendProbeRespParams msg)
{
    tANI_U8 bssIndex;

    HALLOG2( halLog( pMac, LOG2,
        FL("Looking up BSSID - %x:%x:%x:%x:%x:%x\n"),
        msg->bssId[0],
        msg->bssId[1],
        msg->bssId[2],
        msg->bssId[3],
        msg->bssId[4],
        msg->bssId[5] ));
    if (halTable_GetBssIndex(pMac, msg->bssId, &bssIndex) != eHAL_STATUS_DUPLICATE_BSSID){
      HALLOGE( halLog(  pMac, LOGE,
          FL("Unable to find a matching BSS Index for BSSID %x:%x:%x:%x:%x:%x\n"),
          msg->bssId[0],
          msg->bssId[1],
          msg->bssId[2],
          msg->bssId[3],
          msg->bssId[4],
          msg->bssId[5] ));
    }else
    {
        tpBssStruct pBss = &(((tpBssStruct) pMac->hal.halMac.bssTable)[bssIndex]);            
        if (halFW_WriteProbeRspToMemory(pMac, (tANI_U8 *) &(((tAniProbeRspStruct*) msg->pProbeRespTemplate)->macHdr), 
            pBss->bssSelfStaIdx, bssIndex, msg->probeRespTemplateLen)!= eHAL_STATUS_SUCCESS){

          HALLOGE( halLog(  pMac, LOGE,
              FL("Failed to write probeRsp template for BSSID %x:%x:%x:%x:%x:%x/BSS Index %d\n"),
              msg->bssId[0],
              msg->bssId[1],
              msg->bssId[2],
              msg->bssId[3],
              msg->bssId[4],
              msg->bssId[5] ));
        }
    }
    //update probeRsp IE bitmap
    halFW_UpdateProbeRspIeBitmap(pMac, msg->ucProxyProbeReqValidIEBmap);
    
    // Free memory that was allocated by SCH
    // No need to send any response
    palFreeMemory( pMac->hHdd, (tANI_U8 *) msg );

}

#endif

////////////////////////////////////////////////////////////////////////
//                     Beacon API
///////////////////////////////////////////////////////////////////////
void
halMsg_SendBeacon(
    tpAniSirGlobal      pMac,
    tpSendbeaconParams  msg)
{
    tANI_U8 bssIndex;
    //The offset of the TIM IE from the beginning of the template.
    //take out the size of the beaconLength that PE has added and add the template header length.
#ifdef WLAN_SOFTAP_FEATURE    
    tANI_U32 timIeOffset = msg->timIeOffset - sizeof(((tAniBeaconStruct*) msg->beacon )->beaconLength) + BEACON_TEMPLATE_HEADER;
#endif

    HALLOG2( halLog( pMac, LOG2,
        FL("Looking up BSSID - %x:%x:%x:%x:%x:%x\n"),
        msg->bssId[0],
        msg->bssId[1],
        msg->bssId[2],
        msg->bssId[3],
        msg->bssId[4],
        msg->bssId[5] ));
    if (halTable_GetBssIndex(pMac, msg->bssId, &bssIndex) != eHAL_STATUS_DUPLICATE_BSSID){
      HALLOGE( halLog(  pMac, LOGE,
          FL("Unable to find a matching BSS Index for BSSID %x:%x:%x:%x:%x:%x\n"),
          msg->bssId[0],
          msg->bssId[1],
          msg->bssId[2],
          msg->bssId[3],
          msg->bssId[4],
          msg->bssId[5] ));
#ifdef WLAN_SOFTAP_FEATURE      
    }else if (halTpe_UpdateBeacon(pMac, (tANI_U8*)&(((tAniBeaconStruct*) msg->beacon )->macHdr),
        bssIndex, (((tAniBeaconStruct*) msg->beacon )->beaconLength), (tANI_U16)timIeOffset)!= eHAL_STATUS_SUCCESS){
#else
         /* Always write beacon template on BSS INDEX = 0. This is a limitation
           in Libra. Libra H/W always expects beacon to be written on beacon Index = 0.
           In BT-AMP Multiseesion, this is been tested with infra link on BSS Index 0
           and BT-AMP link on BSS INDEX 1. ALthough BT-AMP link is BSS INDEX 1, we
           write on to TPE with beacon index 0
           In Volans this has to be reverted to use proper BSS INDEX 
        */ 
    }else if (halTpe_UpdateBeacon(pMac, (tANI_U8*)&(((tAniBeaconStruct*) msg->beacon )->macHdr), 
              0/*bssIndex*/, (((tAniBeaconStruct*) msg->beacon )->beaconLength))!= eHAL_STATUS_SUCCESS){
#endif
      HALLOGE( halLog(  pMac, LOGE,
          FL("Failed to write beacon template for BSSID %x:%x:%x:%x:%x:%x/BSS Index %d\n"),
          msg->bssId[0],
          msg->bssId[1],
          msg->bssId[2],
          msg->bssId[3],
          msg->bssId[4],
          msg->bssId[5] ));
    }
    // Free memory that was allocated by SCH
    // No need to send any response
    palFreeMemory( pMac->hHdd, (tANI_U8 *) msg );

#ifndef WLAN_SOFTAP_FEATURE
    // Enable beacon transmission, hence send the enable flag = TRUE.
    halMTU_EnableDisableBssidTBTTBeaconTransmission(pMac,
            pMac->hal.halMac.beaconInterval, TRUE);
#endif
}


/* --------------------------------------------------
 * FUNCTION:  halMsg_updateRetryLimit()
 *
 */
eHalStatus halMsg_updateRetryLimit(tpAniSirGlobal pMac)
{
    tANI_U32   shortRetry, longRetry;
    // Update Short retry and Long Retry limit
    if (wlan_cfgGetInt(pMac, WNI_CFG_SHORT_RETRY_LIMIT, &shortRetry) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_SHORT_RETRY_LIMIT failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    if (wlan_cfgGetInt(pMac, WNI_CFG_LONG_RETRY_LIMIT, &longRetry) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_LONG_RETRY_LIMIT failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOG1( halLog(pMac, LOG1, FL("shortRetry=%d, longRetry=%d \n"), shortRetry, longRetry));

    halMTU_updateRetryLimit(pMac, (tANI_U8)shortRetry, (tANI_U8)longRetry);

    return eHAL_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////
//                 Global PARAM change API
///////////////////////////////////////////////////////////////////////////
// set the broadcast rate/
void
halSetBroadcastRate(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tANI_U32        rate)
{
    return;
}



/* ----------------------------------------------------
 * FUNCTION:  halMsg_AddStaSelf()
 *
 * NOTE:
 *   Station add its default entry upon initilaization.
 * ----------------------------------------------------
 */
eHalStatus halMsg_AddStaSelf(tpAniSirGlobal  pMac)
{
    tANI_U32     cfgLen;
    tANI_U8      staIdx;
    tSirMacAddr  staMac;
    tSirMacAddr  bssid = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    tANI_BOOLEAN wep_keyId_extract = 0;  //No encryption.
    tANI_U8 dpuIdx = HAL_DPU_SELF_STA_DEFAULT_IDX;
    tANI_U8 dpuSignature = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 ftBit;
    tANI_U8 rmfBit = 0;

    cfgLen = SIR_MAC_ADDR_LENGTH;
    if ( wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *)staMac, &cfgLen) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("cfgGetStr(WNI_CFG_STA_ID) failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGE( halLog(pMac, LOGE, FL("set selfstaMac: %x %x %x %x %x %x \n"),
            staMac[0], staMac[1], staMac[2], staMac[3], staMac[4], staMac[5]));

    HALLOGE( halLog( pMac, LOGE, FL("halMac.selfStaId index = %d\n"), pMac->hal.halMac.selfStaId));


    halSetBcnRateIdx(pMac, 8);
    halSetNonBcnRateIdx(pMac, 8);
    halSetMulticastRateIdx(pMac, 8);

    if(HAL_STA_INVALID_IDX == pMac->hal.halMac.selfStaId)
    {
        eHalStatus status = halTable_GetStaId(pMac, STA_ENTRY_SELF, bssid, staMac, &staIdx);
        if (eHAL_STATUS_SUCCESS != status)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halTable_GetStaId() failed - return code = %d\n"),
                    status));
            return eHAL_STATUS_FAILURE;
        }

        // Save the STA type - this is used for lookup
        halTable_SetStaType(pMac, staIdx, STA_ENTRY_SELF);

        // Finally, save the Self-STA ID
        pMac->hal.halMac.selfStaId = staIdx;

        HALLOGE( halLog( pMac, LOGE, FL("Added self station index = %d\n"), staIdx));
    }
    else
    {
        tANI_U8 *pCurrStaAddr;

        HALLOG1( halLog(pMac, LOG1, FL("Called for Self STAID update with MAC addr: %x %x %x %x %x %x \n"),
                staMac[0], staMac[1], staMac[2], staMac[3], staMac[4], staMac[5]));

        staIdx = (tANI_U8)pMac->hal.halMac.selfStaId;

        // Retrieve the current STA addr and delete it from the RXP search table
        if(eHAL_STATUS_SUCCESS != halTable_GetStaAddr(pMac, staIdx, &pCurrStaAddr))
        {
            HALLOGE( halLog( pMac, LOGE, FL("Unable to retrieve Current STA Addr for STAID %d\n"),
                    staIdx));
            return eHAL_STATUS_INVALID_STAIDX;
        }
        // If the new address being added is same as the old address just bail out
        if(sirCompareMacAddr(pCurrStaAddr, staMac))
        {
            HALLOGE( halLog( pMac, LOGE, FL("Called to set the same MAC address again. Just bail out!!!\n")));
            return eHAL_STATUS_SUCCESS;
        }

        if ( halRxp_DelEntry(pMac, pCurrStaAddr) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halRxp_DelEntry() fail\n")));
            return eHAL_STATUS_FAILURE;
        }

        // Now update the STA entry with the new MAC address
        if(eHAL_STATUS_SUCCESS != halTable_SetStaAddr(pMac, staIdx, staMac))
        {
            HALLOGE( halLog( pMac, LOGE, FL("Unable to update STA Addr for STAID %d\n"),
                    staIdx));
            return eHAL_STATUS_INVALID_STAIDX;
        }
    }

#if defined(ANI_OS_TYPE_LINUX)
    halTable_AddToStaCache(pMac, staMac, staIdx);
#endif

    {
        tAddStaParams param;
        tTpeRateIdx  rateIndex, mcastRateIndex;

        /** Zero out AddStaParam */
        if ((status = palZeroMemory(pMac->hHdd, &param,
            sizeof(param))) != eHAL_STATUS_SUCCESS) {
            return status;
        }

        // Read the default rates for 2.4Ghz (Default RF band)
        status = halGetDefaultAndMulticastRates(pMac, eRF_BAND_2_4_GHZ, &rateIndex, &mcastRateIndex);
        if(eHAL_STATUS_SUCCESS != status) {
            HALLOGW(halLog(pMac, LOGW, FL("halMsg_AddBss: halGetDefaultAndMulticastRates() fail\n")));
            return status;
        }

        //update hal global configuration
        halSetBcnRateIdx(pMac, rateIndex);
        halSetNonBcnRateIdx(pMac, rateIndex);
        halSetMulticastRateIdx(pMac, mcastRateIndex);

        // Copy the Mac address into the param
        palCopyMemory(pMac, (void*)&param.staMac, (void*)&staMac, SIR_MAC_ADDR_LENGTH);

        param.staType = STA_ENTRY_SELF;
        halMsg_addStaUpdateTPE(pMac, staIdx, &param);
    }

    //Update BMU.
    status = halBmu_sta_enable_disable_control(pMac, staIdx, eBMU_ENB_TX_QUE_ENB_TRANS);
    if( eHAL_STATUS_SUCCESS != status )
    {
        HALLOGE( halLog( pMac, LOGE, FL("Unable to update BMU")));
        return status;
    }

    //setting up same dpu ID for all thre indices. GTK and IGTK will be overwritten in addBss.
    halTable_SetStaDpuIdx(pMac, staIdx, dpuIdx);
    halTable_SetStaBcastDpuIdx(pMac, staIdx, dpuIdx);
    halTable_SetStaBcastMgmtDpuIdx(pMac, staIdx, dpuIdx);
    pMac->hal.halMac.selfStaDpuId = dpuIdx;

    // Get the frame translation setting
    ftBit = halGetFrameTranslation(pMac);
    halDpu_GetSignature(pMac, dpuIdx, &dpuSignature);

    {
        // Initialize the RPE descriptor as well, as during PRE-ASSOC state ADDR2 filter
        // for the peer sta is temporarily added in the RXP with sta index 0. Without a valid
        // RPE entry, RXP drops the ADDR2 hit frames with invaild RPE sta entry.
        tAddStaParams param;
        param.updateSta = FALSE;
        halMsg_addStaUpdateRPE( pMac, staIdx, &param);

        //Add RXP entry.
        if (halRxp_AddEntry(pMac, (tANI_U8) staIdx, staMac, eRXP_SELF, rmfBit,
                            dpuIdx, dpuIdx, dpuIdx,
                            dpuSignature, dpuSignature, dpuSignature,
                            0, ftBit, wep_keyId_extract) != eHAL_STATUS_SUCCESS) {
            return eHAL_STATUS_FAILURE;
        }
    }

    return eHAL_STATUS_SUCCESS;
}

// Function to post the SIR_HAL_SET_KEY_DONE message back to HAL
// after the keys are set properly.
void halMsg_sendSetKeyDoneMsg(tHalHandle hHal, tANI_U8 bssidx)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

    tSirMsgQ msg;

    msg.type =  SIR_HAL_SET_KEY_DONE;
    msg.reserved = 0;
    msg.bodyptr = NULL;
    msg.bodyval = bssidx;

    if(halPostMsgApi(pMac,&msg) != eSIR_SUCCESS) {
        HALLOGE(halLog(pMac, LOGE, FL("Posting SIR_HAL_SET_KEY_DONE msg failed")));
    }

    HALLOGE(halLog(pMac, LOGE, FL("Posting SIR_HAL_SET_KEY_DONE msg\n")));

    return;
}

// Function to handle the set key done message, here it handles
// the activities that are done after the set key is finished, like
// the loading of the TX power det values, starting the BA timer.
// This cannot be handled within the halMsg_AddBss, beacuse of time
// expensive writes over the SDIO bus. And the BA timer cannot
// be started till the keys are set properly.
void halMsg_SetKeyDone(tpAniSirGlobal  pMac, tANI_U32 bssidx)
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
	 
    //(void)halPhyLoadTxPowerDetValues(pMac); //ravij

    // Send message to FW indicating the connection is successfully established
	// with security keys (if any) all set. Needs to be done only for Infra STA

    if (bssidx < pMac->hal.halMac.maxBssId &&
            bssTable[bssidx].bssSystemRole == eSYSTEM_STA_ROLE)
    {
    (void)halFW_SendConnectionEndMesg(pMac);
    }

    // Start the BA activity check timer.
    if (halStartBATimer(pMac) != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE, FL("Failed to start BA Timer\n")));
    }

    return;
}

////////////////////////////////////////////////////////////////////////////
//                 KEY SET API
////////////////////////////////////////////////////////////////////////////
/*
 * Set a Key entry to a BSS. This is used for Static-WEP case.
 */
void
halMsg_SetBssKey( tpAniSirGlobal  pMac,
    tANI_U16 dialog_token,
    tpSetBssKeyParams param )
{
    int i;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bcastDpuIdx, bcastMgmtDpuIdx;
    tBssSystemRole  BssSystemRole = eSYSTEM_UNKNOWN_ROLE;
#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif

    HALLOGE(halLog(pMac, LOGE, FL("bssIdx=%d, numerKeys=%d, encType=%d\n"), param->bssIdx, param->numKeys, param->encType));
#ifdef WLAN_SOFTAP_FEATURE
    BssSystemRole = halGetBssSystemRole(pMac, param->bssIdx);
#endif


    switch( param->encType )
    {
        case eSIR_ED_WEP40:
        case eSIR_ED_WEP104:
            {
                tANI_U8 defaultKeyId = 0;
                tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;

                if( 0 == param->numKeys )
                    halGetWepKeysFromCfg( pMac,
                            &defaultKeyId,
                            &param->numKeys,
                            param->key );

                for(i = 0; i < param->numKeys; i++)
                {
#ifdef WLAN_SOFTAP_FEATURE
                    if(eSYSTEM_AP_ROLE == BssSystemRole)
                    {
                        int j;
                        HALLOGE(halLog(pMac, LOGE, FL("keyId=%d, KeyLength=%d\n"), param->key[i].keyId, param->key[i].keyLength));

                        for( j = 0; j < param->key[i].keyLength; j++)
                            HALLOGE(halLog(pMac, LOGE, FL("%2x"), param->key[i].key[j]));

                        HALLOGE(halLog(pMac, LOGE, FL("\n")));                      

                        if(param->key[i].keyDirection == eSIR_TX_DEFAULT)
                        {   
                            bssTable[param->bssIdx].defaultKeyId = param->key[i].keyId;
                            HALLOGE(halLog(pMac, LOGE, FL("defaultKeyId=%d"), bssTable[param->bssIdx].defaultKeyId));
                        }
                        defaultKeyId = bssTable[param->bssIdx].defaultKeyId;
                    }	
#endif
                    status = halSetBssWepKey( pMac,
                            param->bssIdx,
                            param->encType,
                            param->key[i].keyId,
                            param->key[i].key );

                    if(status != eHAL_STATUS_SUCCESS)
                        goto generate_response;
                }
                // Get the DPU Descriptor Index for the given BSS index
                status = halTable_GetBssDpuIdx( pMac, param->bssIdx, &bcastDpuIdx );

                // Set WEP Keys in the "context" of the BSS DPU Descriptor
                status = halDpu_SetWepKeys( pMac,
                        bcastDpuIdx,
                        param->encType,
                        defaultKeyId,
                        bssTable[param->bssIdx].wepKeyIds[0],
                        bssTable[param->bssIdx].wepKeyIds[1],
                        bssTable[param->bssIdx].wepKeyIds[2],
                        bssTable[param->bssIdx].wepKeyIds[3] );

            }
            break;

        case eSIR_ED_TKIP:
		case eSIR_ED_CCMP:
#ifdef FEATURE_WLAN_WAPI
        case eSIR_ED_WPI:
#endif
            //
            // Find the DPU Descriptor index that corresponds
            // to this BSS (AP). This will be the DPU index
            // that will be used to decrypt and encrypt
            // Broadcast/Multicast frames
            //
            if( eHAL_STATUS_SUCCESS ==
                    (status = halTable_GetBssDpuIdx( pMac,
                                                     param->bssIdx,
                                                     &bcastDpuIdx )))
            {
                tANI_U8 winChkSize[MAX_NUM_OF_TIDS];

                palZeroMemory( pMac->hHdd, winChkSize, sizeof( winChkSize ));

                // Now set GTK wrt this DPU descriptor
                status = halSetPerStaKey( pMac,
                        (tANI_U8) bcastDpuIdx,
                        0, // Callee to ignore STAID param
                        param->encType,
                        (tANI_U16) HAL_DPU_DEFAULT_RCE_ON,  // RCE for Group Keys
                        (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                        winChkSize,
                        param->singleTidRc,
                        param->key[0].key, // FIXME - Need an index. Assuming 0 for now!!
                        param->key[0].paeRole,
                        param->key[0].keyId,  // FIXME - Need an index. Assuming 0 for now!!
                        param->key[0].keyRsc );
            }
            break;

        case eSIR_ED_AES_128_CMAC:
            // Get the DPU index allocated for MGMT frames
            if(eHAL_STATUS_SUCCESS ==
                    (status = halTable_GetBssBcastMgmtDpuIdx( pMac,
                                                         param->bssIdx,
                                                         &bcastMgmtDpuIdx ))) {
                tANI_U8 winChkSize[MAX_NUM_OF_TIDS];
                palZeroMemory( pMac->hHdd, winChkSize, sizeof( winChkSize ));

                // Set the IGTK wrt to this DPU descriptor
                status = halSetPerStaKey( pMac,
                        bcastMgmtDpuIdx,
                        0,                                  // Callee to ignore STAID param
                        param->encType,
                        (tANI_U16) HAL_DPU_DEFAULT_RCE_ON,  // RCE for Group Keys
                        (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                        winChkSize,
                        param->singleTidRc,
                        param->key[0].key,
                        param->key[0].paeRole,
                        param->key[0].keyId,
                        param->key[0].keyRsc );
            }
            break;

        default:
            break;
    }


    //FIXME_GEN6 Is this portion of the code required in Gen6 ???

    if( eHAL_STATUS_SUCCESS == status &&
            eSIR_ED_NONE != param->encType )
    {
        tANI_U8 staIdx;

        // Get the Bss System Role
        BssSystemRole = halGetBssSystemRole(pMac, param->bssIdx);

        if( ((eSYSTEM_AP_ROLE == BssSystemRole) ||
                    (eSYSTEM_BTAMP_AP_ROLE == BssSystemRole) ||
                    (eSYSTEM_BTAMP_STA_ROLE == BssSystemRole) ||
                    (eSYSTEM_STA_IN_IBSS_ROLE == BssSystemRole)) &&
                ( eHAL_STATUS_SUCCESS == halTable_GetStaIndexForBss( pMac, param->bssIdx, &staIdx )) )
        {
            status = halTable_SaveEncMode(pMac, (tANI_U8)staIdx, param->encType);
            if(eHAL_STATUS_SUCCESS != status)
            {
                HALLOGW( halLog(pMac, LOGW, FL("halTable_SaveEncMode failed for staIdx = %d, encMode = %d with status = %d\n"),
                        staIdx, param->encType, status));
                goto generate_response;
            }
        }
        /* Start the BA timer only after GTK is set */
        if(param->encType != eSIR_ED_NONE) {
            // Post a message back to HAL to indicate set key done. This is done to
            // handle the activities after set key like the loading of the TX power
            // detection values, updating the rate to power table. This cannot be
            // handled within the halMsg_AddBss, beacuse of time expensive writes
            // over the SDIO bus.
            halMsg_sendSetKeyDoneMsg(pMac, param->bssIdx);
        }
    }

    /* TODO : Send the response back */
generate_response:
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_SET_BSSKEY_RSP, dialog_token, (void *) param, 0);
    return;
}

/*
 * Config a Key context to a station, i.e.:
 * The old key information will be replaced by the new one.
 */
void
halMsg_SetStaKey(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpSetStaKeyParams  param )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bssidx;
    tANI_U8 winChkSize[MAX_NUM_OF_TIDS];
    tANI_U32 regValue;
#ifdef FIXME_GEN5
    tANI_U16 rce = 0, wce = 0;
#endif

    /* Diable DPU RX WQ3 to avoid any encrypted packet rx before setKey done*/
    /* avoid race condition in TKIP where GTK recievied before setKey done */
    halReadRegister(pMac, QWLAN_BMU_DISABLE_WQ_DA_REG, &regValue);
    regValue |=(1<<BMUWQ_DPU_RX);
    halWriteRegister(pMac, QWLAN_BMU_DISABLE_WQ_DA_REG, regValue);


    // Now since the STA context is being added, the STA must be already ULAP authenticated
    // So, set the STA signature bit that indicates that the STA is ULAP authenticated
    status = halTable_SetStaAuthState(pMac, (tANI_U8 )param->staIdx, eANI_BOOLEAN_TRUE);

    HALLOGE(halLog(pMac, LOGE, FL("staIdx=%d, encType=%d\n"), param->staIdx, param->encType));

    if(eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog(  pMac, LOGE,
                    FL("Unable to set STAID %d to Authenticated state.\n"),
                    param->staIdx));
        goto generate_response;
    }

#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif
    if( param->encType == eSIR_ED_WEP40 || param->encType == eSIR_ED_WEP104 )
    {
		tANI_U8 defWEPIdx; 
                defWEPIdx = param->defWEPIdx;            	
                
        if( eHAL_STATUS_SUCCESS ==
                (status = halTable_GetBssIndexForSta( pMac,
                                                      &bssidx,
                                                      (tANI_U8) param->staIdx)))
        {
            //
            // DVT configures WEP Keys in the following way:
            // 1) Configure BSS WEP Keys
            // 2) Configure Unicast WEP Key (by only specifying
            // the "defaultWEP" Key index in the message
            //
            // SME configures WEP Keys in the following way:
            // 1) Configure Unicast WEP Key (by specifying
            // "nothing" in the message parameters!!
            // 2) Configure BSS WEP Keys (again, by specifying
            // "nothing" in the message parameters!!
            // The reason for this "nothingness":
            // MAC SW is supposed pull out the WEP Keys from the
            // CFG and plumb it into the hardware!
            //
            // To accomodate both these northbound interfaces, the
            // paramater defWEPIdx is used in the following way:
            // if( 0xff == param->defWEPIdx )
            //   extractWEPKeysFromCFG
            // else
            //   assumeWEPKeysAlreadyConfiguredForBSS
            //

                        HALLOGE(halLog(pMac, LOGE, FL("bssIdx=%d, wepType=%d, defWEPIdx=%d\n"), bssidx, param->wepType, param->defWEPIdx)); 
            if( eSIR_WEP_STATIC == param->wepType &&
                    0xff == param->defWEPIdx )
            {
                tANI_U8 numKeys, i;
                tSirKeys key[SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS];
#ifdef WLAN_SOFTAP_FEATURE
                tBssSystemRole BssSystemRole = eSYSTEM_UNKNOWN_ROLE;

                //
                // WEP Keys are common BSS-wide
                // Extract the WEP Keys & the Default WEP Key from CFG
                // And then, configure the WEP Keys at the BSS level
                //
                BssSystemRole =  halGetBssSystemRoleFromStaIdx(pMac, param->staIdx);
                //                BssSystemRole = halGetBssSystemRole(pMac, param->staIdx);


                if(eSYSTEM_AP_ROLE == BssSystemRole)
                {
                    numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;

                    palCopyMemory(pMac->hHdd, key, param->key, sizeof(tSirKeys)*SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS);                    
                    for( i = 0; i < numKeys; i++)
                    {
                        tANI_U8 j;
                        HALLOGE(halLog(pMac, LOGE, FL("keyId=%d, KeyLength=%d\n"), param->key[i].keyId, param->key[i].keyLength));
                        if(key[i].keyDirection == eSIR_TX_DEFAULT)
                            defWEPIdx = i; 
                        for( j =0; j < key[i].keyLength; j++)
                        {
                            HALLOGE(halLog(pMac, LOGE, FL("%2x"), key[i].key[j]));
                        }
                        HALLOGE(halLog(pMac, LOGE, FL("\n")));
                    } 
                } 
                else
                {
#endif                
                    if( eHAL_STATUS_SUCCESS !=
                            (status = halGetWepKeysFromCfg( pMac,
                                                            &param->defWEPIdx,
                                                            &numKeys,
                                                            key )))
                    {
                        HALLOGE( halLog(  pMac, LOGE,
                                    FL("Unable to extract WEP info from CFG!\n" )));
                        goto generate_response;
                    }
                    defWEPIdx = param->defWEPIdx;
#ifdef WLAN_SOFTAP_FEATURE
                }
#endif
                
                HALLOGE(halLog(pMac, LOGE, FL("Default WEP keyId=%d\n"), defWEPIdx));//param->defWEPIdx defWEPIdx can be set to zero

                for( i = 0; i < numKeys; i++ )
                {
                    status = halSetBssWepKey( pMac,
                            bssidx,
                            param->encType,
                            key[i].keyId,
                            key[i].key );

                    if(status != eHAL_STATUS_SUCCESS) {
                        goto generate_response;
                    }
                }
            }
        }

        halRxp_UpdateEntry(pMac, (tANI_U8)param->staIdx, eRXP_PEER_STA, TRUE);

        if(param->wepType == eSIR_WEP_STATIC)
        {
            status = halSetStaWepKey(pMac, (tANI_U8 ) param->staIdx,
                    param->encType, defWEPIdx);//param->defWEPIdx defWEPIdx can be set to zero

            goto key_set_done;
        }
    }

    if( eSIR_ED_NONE == param->encType )
    {
        status = halSetPerStaKey( pMac,
                HAL_INVALID_KEYID_INDEX,
                (tANI_U8 ) param->staIdx,
                eSIR_ED_NONE,
                0, // RCE
                0, // WCE,
                0,
                param->singleTidRc,
                0,
                0,
                0,
                NULL );
        halTable_GetBssIndexForSta( pMac, &bssidx,(tANI_U8) param->staIdx );
        goto key_set_done;
    }
#ifdef FIXME_GEN5
    if( eHAL_STATUS_SUCCESS == (status =
                halSta_getDefaultRCDescriptorFields( pMac,
                    addPtr->newSta,
                    &rce,
                    &wce,
                    winChkSize )))
#endif
    {
#ifdef WLAN_SOFTAP_FEATURE
        status = halSetPerStaKey( pMac,
                HAL_INVALID_KEYID_INDEX,
                (tANI_U8 ) param->staIdx,
                param->encType,
                (tANI_U16) HAL_DPU_DEFAULT_RCE_OFF, // RCE, WCE are always disabled due to DPU RC check HW bug. See bug 15423 for more details.
                HAL_DPU_DEFAULT_WCE_OFF,
                winChkSize,
                param->singleTidRc,
                param->key[0].key,
                param->key[0].paeRole,
                param->key[0].keyId,
                param->key[0].keyRsc );
#else
        status = halSetPerStaKey( pMac,
                HAL_INVALID_KEYID_INDEX,
                (tANI_U8 ) param->staIdx,
                param->encType,
                (tANI_U16) HAL_DPU_DEFAULT_RCE_OFF, // RCE, WCE are always disabled due to DPU RC check HW bug. See bug 15423 for more details.
                HAL_DPU_DEFAULT_WCE_OFF,
                winChkSize,
                param->singleTidRc,
                param->key.key,
                param->key.paeRole,
                param->key.keyId,
                param->key.keyRsc );
#endif
    }
    // send a response to caller.
key_set_done:
    if(status == eHAL_STATUS_SUCCESS)
    {

        status = halTable_SaveEncMode(pMac, (tANI_U8)param->staIdx, param->encType);
        if(eHAL_STATUS_SUCCESS != status)
        {
            HALLOGW( halLog(pMac, LOGW, FL("halTable_SaveEncMode failed for staIdx = %d, encMode = %d with status = %d\n"),
                        param->staIdx, param->encType, status));
            goto generate_response;
        }
        /** Update UMA Sta Desc */
        if (param->encType != eSIR_ED_NONE)
        {
            status = halAdu_UpdateUmaDescForPrivacy(pMac, param->staIdx);
            if(status != eHAL_STATUS_SUCCESS)
            {
                HALLOGE( halLog( pMac, LOGE, FL("UMA programming failed\n")));
            }
        }

        if (param->encType == eSIR_ED_NONE ||
                param->encType == eSIR_ED_WEP40 ||
                param->encType == eSIR_ED_WEP104) {

            // Post a message back to HAL to indicate set key done. This is done to
            // handle the activities after set key like the loading of the TX power
            // detection values, updating the rate to power table. This cannot be
            // handled within the halMsg_AddBss, beacuse of time expensive writes
            // over the SDIO bus.
            halMsg_sendSetKeyDoneMsg(pMac, bssidx);
        }
    }

generate_response:
    /* Enable the DPU RX WQ3 */
    halReadRegister(pMac, QWLAN_BMU_DISABLE_WQ_DA_REG, &regValue);
    regValue &=~(1<<BMUWQ_DPU_RX);
    halWriteRegister(pMac, QWLAN_BMU_DISABLE_WQ_DA_REG, regValue);

    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_SET_STAKEY_RSP, dialog_token, (void *) param, 0);
    return;

}

void
halMsg_SetStaBcastKey(tpAniSirGlobal pMac,
     tANI_U16 dialog_token,
     tpSetStaKeyParams param )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 winChkSize[MAX_NUM_OF_TIDS];
    tANI_U8 staDpuIdx = HAL_INVALID_KEYID_INDEX;
    tANI_U8 staBcastDpuIdx  = HAL_INVALID_KEYID_INDEX;
    tANI_U8 staBcastMgmtDpuIdx = HAL_INVALID_KEYID_INDEX;
    tANI_U8 staDpuSig, staBcastDpuSig, staMgmtDpuSig;
    tANI_U8 bcastDpuIdx, bcastMgmtDpuIdx;
    tANI_U8 *pStaMacAddr;
    tANI_BOOLEAN rmfEnabled = eANI_BOOLEAN_FALSE;
    tANI_U8 bssIdx;
        tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif
    if((param->encType == eSIR_ED_WEP104 || param->encType == eSIR_ED_WEP40) &&
            (param->wepType == eSIR_WEP_STATIC))
    {
        //Do nothing. Static WEP keys remain same throughtout BSS.
        //It shouldnt be set in a per station call.
        status = eHAL_STATUS_INVALID_PARAMETER;
        goto generate_response;
    }

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_GetBssIndexForSta( pMac,
                                                  &bssIdx,
                                                  (tANI_U8) param->staIdx)))
    {
        goto generate_response;
    }

    if((status = halTable_GetBssDpuIdx( pMac, bssIdx, &bcastDpuIdx))  !=
            eHAL_STATUS_SUCCESS )
    {
        goto generate_response;
    }

    if((status = halTable_GetStaBcastDpuIdx(pMac, (tANI_U8)param->staIdx, &staBcastDpuIdx))  !=
            eHAL_STATUS_SUCCESS )
    {
        goto generate_response;
    }

    if((status = halTable_GetBssBcastMgmtDpuIdx( pMac, bssIdx, &bcastMgmtDpuIdx))  !=
            eHAL_STATUS_SUCCESS )
    {
        goto generate_response;
    }

    if((status = halTable_GetStaBcastMgmtDpuIdx(pMac, (tANI_U8)param->staIdx, &staBcastMgmtDpuIdx))  !=
            eHAL_STATUS_SUCCESS )
    {
        goto generate_response;
    }

    switch( param->encType ) {
        case eSIR_ED_TKIP:
        case eSIR_ED_CCMP:
#ifdef FEATURE_WLAN_WAPI
        case eSIR_ED_WPI:
#endif
            if( bcastDpuIdx == staBcastDpuIdx )
            {
                // Check if the STA has a valid DPU index
                halTable_GetStaBcastDpuIdx(pMac, (tANI_U8)param->staIdx, &staBcastDpuIdx);
                if(staBcastDpuIdx == HAL_INVALID_KEYID_INDEX)
                {
                    //Allocate a new DPU. DPU for broadcast was never allocated.
                    if( (status = halDpu_AllocId ( pMac, &staBcastDpuIdx )) != eHAL_STATUS_SUCCESS )
                    {
                        goto generate_response;
                    }
                    status = halTable_SetStaBcastDpuIdx(pMac, (tANI_U8)param->staIdx,staBcastDpuIdx);
                    if(status != eHAL_STATUS_SUCCESS) goto generate_response;
                }
            }

            palZeroMemory( pMac->hHdd, winChkSize, sizeof( winChkSize ));

            // Now set GTK wrt this DPU descriptor
#ifdef WLAN_SOFTAP_FEATURE
            status = halSetPerStaKey( pMac,
                    (tANI_U8) staBcastDpuIdx,
                    0, // Callee to ignore STAID param
                    param->encType,
                    (tANI_U16) HAL_DPU_DEFAULT_RCE_ON, // RCE for Group Keys
                    (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                    winChkSize,
                    param->singleTidRc,
                    param->key[0].key, // FIXME - Need an index. Assuming 0 for now!!
                    param->key[0].paeRole,
                    param->key[0].keyId,
                    param->key[0].keyRsc ); // FIXME - Need an index. Assuming 0 for now!!
#else            
            status = halSetPerStaKey( pMac,
                    (tANI_U8) staBcastDpuIdx,
                    0, // Callee to ignore STAID param
                    param->encType,
                    (tANI_U16) HAL_DPU_DEFAULT_RCE_ON, // RCE for Group Keys
                    (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                    winChkSize,
                    param->singleTidRc,
                    param->key.key, // FIXME - Need an index. Assuming 0 for now!!
                    param->key.paeRole,
                    param->key.keyId,  // FIXME - Need an index. Assuming 0 for now!!
                    param->key.keyRsc );
#endif
            break;

        case eSIR_ED_AES_128_CMAC:
            // If 11w is enabled allocate DPU descriptor for management bc/mc packets
                        pSta += param->staIdx;

            if( bcastMgmtDpuIdx == staBcastMgmtDpuIdx )
            {
                // Check if the STA has a valid DPU index
                halTable_GetStaBcastMgmtDpuIdx(pMac, (tANI_U8)param->staIdx, &staBcastMgmtDpuIdx);
                if(staBcastMgmtDpuIdx == HAL_INVALID_KEYID_INDEX)
                {
                    //Allocate a new DPU. DPU for Mgmt was never allocated.
                    if( (status = halDpu_AllocId ( pMac, &staBcastMgmtDpuIdx )) != eHAL_STATUS_SUCCESS )
                    {
                        goto generate_response;
                    }
                    status = halTable_SetStaBcastMgmtDpuIdx(pMac, (tANI_U8)param->staIdx, staBcastMgmtDpuIdx);
                    if(status != eHAL_STATUS_SUCCESS) goto generate_response;
                }
            }
            palZeroMemory( pMac->hHdd, winChkSize, sizeof( winChkSize ));

            // Now set iGTK wrt this DPU descriptor
#ifdef WLAN_SOFTAP_FEATURE
            status = halSetPerStaKey( pMac,
                    (tANI_U8) staBcastMgmtDpuIdx,
                    0, // Callee to ignore STAID param
                    param->encType,
                    (tANI_U16) HAL_DPU_DEFAULT_RCE_ON, // RCE for Group Keys
                    (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                    winChkSize,
                    param->singleTidRc,
                    param->key[0].key, // FIXME - Need an index. Assuming 0 for now!!
                    param->key[0].paeRole,
                    param->key[0].keyId,
                    param->key[0].keyRsc ); // FIXME - Need an index. Assuming 0 for now!!
#else            
            status = halSetPerStaKey( pMac,
                    (tANI_U8) staBcastMgmtDpuIdx,
                    0, // Callee to ignore STAID param
                    param->encType,
                    (tANI_U16) HAL_DPU_DEFAULT_RCE_ON, // RCE for Group Keys
                    (tANI_U16) HAL_DPU_DEFAULT_WCE_OFF, // WCE for Group Keys
                    winChkSize,
                    param->singleTidRc,
                    param->key.key, // FIXME - Need an index. Assuming 0 for now!!
                    param->key.paeRole,
                    param->key.keyId,  // FIXME - Need an index. Assuming 0 for now!!
                    param->key.keyRsc );
#endif
            rmfEnabled = eANI_BOOLEAN_TRUE;
                        pSta->rmfEnabled = eANI_BOOLEAN_TRUE;

            break;

        default:
            status = eHAL_STATUS_INVALID_PARAMETER;
            goto generate_response;
    }

    // Since new dpu descriptor has been allocated, RXP table should be updated
    if( status == eHAL_STATUS_SUCCESS) {
                tANI_U8 ftBit;

        status = halTable_GetStaAddr(pMac, (tANI_U8)(param->staIdx), &pStaMacAddr);

        // Get the PTK DPU index and signature
        status = halTable_GetStaDpuIdx( pMac, (tANI_U8)(param->staIdx), &staDpuIdx );
        if(status != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog( pMac, LOGE, FL("Invalid station index\n")));
            goto generate_response;
        }
        halDpu_GetSignature( pMac, staDpuIdx, &staDpuSig );

        // Get the GTK DPU signature
        halDpu_GetSignature( pMac, staBcastDpuIdx, &staBcastDpuSig );

        // Get the IGTK DPU signature
        halDpu_GetSignature( pMac, staBcastMgmtDpuIdx, &staMgmtDpuSig );

        // Get the frame translation setting
        ftBit = halGetFrameTranslation(pMac);

        status = halRxp_AddEntry(
                pMac, (tANI_U8)(param->staIdx), pStaMacAddr, eRXP_PEER_STA, rmfEnabled,
                staDpuIdx, staBcastDpuIdx, staBcastMgmtDpuIdx, staDpuSig, staBcastDpuSig, staMgmtDpuSig,
                0, ftBit, 0);
    }

generate_response:

    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_SET_STA_BCASTKEY_RSP, dialog_token, (void*) param, 0);
    return;
}

void
halMsg_RemoveBssKey( tpAniSirGlobal  pMac,
    tANI_U16 dialog_token,
    tpRemoveBssKeyParams param )
{

    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8 bcastDpuIdx;
    tANI_U8 keyId;
#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif
    HALLOGE(halLog(pMac, LOGE, FL("bssIdx=%d\n"), param->bssIdx));

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_GetBssDpuIdx( pMac,
                                             param->bssIdx,
                                             &bcastDpuIdx )))
    {
        HALLOGE( halLog(pMac, LOGE, FL("Unable to get Bssidx\n")));
        goto end;
    }


    if( (param->encType == eSIR_ED_WEP104 || param->encType == eSIR_ED_WEP40) &&
            param->wepType == eSIR_WEP_STATIC )
    {
        //Invalidate key entry.
        status = halInvalidateBssWepKey( pMac,
                bcastDpuIdx,
                param->encType,
                param->keyId);

        goto end;
    }

    //TKIP, CCMP and Dynamic WEP

    if( eHAL_STATUS_SUCCESS == (status = halDpu_GetKeyId(pMac, bcastDpuIdx, &keyId)))
    {
        status = halInvalidateStaKey(pMac, keyId, param->encType);
    }

end:
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_REMOVE_BSSKEY_RSP, dialog_token, (void *) param, 0);
    return;
}

/*
 * Remove station key.
 *
 */
void
halMsg_RemoveStaKey(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpRemoveStaKeyParams  param )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U8  dpuIdx, keyIdx;
#ifdef WLAN_PERF
    pMac->hal.halMac.uBdSigSerialNum ++;
#endif
    HALLOGE(halLog(pMac, LOGE, FL("staIdx%d\n"), param->staIdx));

    if(param->unicast)
    {
        if((status = halTable_GetStaDpuIdx(pMac, (tANI_U8) param->staIdx, &dpuIdx)) != eHAL_STATUS_SUCCESS )
        {
            HALLOGE( halLog(pMac, LOGE, FL("Unable to get station DPU index")));
            goto generate_response;
        }
    }
    else
    {
        if((status = halTable_GetStaBcastDpuIdx(pMac, (tANI_U8) param->staIdx, &dpuIdx)) != eHAL_STATUS_SUCCESS )
        {
            HALLOGE( halLog(pMac, LOGE, FL("Unable to get station's Broadcast  DPU index")));
            goto generate_response;
        }
    }
    if( eHAL_STATUS_SUCCESS == (status = halDpu_GetKeyId(pMac, dpuIdx, &keyIdx)))
    {
        status = halInvalidateStaKey (pMac, keyIdx, param->encType);
    }

#if defined(LIBRA_WAPI_SUPPORT)
    if(HAL_STATUS_SUCCESS(status))
    {
        tHalDpuDescEntry *pDpuDesc;
        tpDpuInfo pDpu = (tpDpuInfo) pMac->hal.halMac.dpuInfo;

        pDpuDesc = &pDpu->descTable[dpuIdx];
        if(pDpuDesc->halDpuDescriptor.wapi)
        {
            Qwlanfw_AddRemoveKeyReqType removeKey;
            tANI_U8 uFwMesgType = QWLANFW_HOST2FW_WPI_KEY_REMOVED;

            if(!param->unicast)
            {
                removeKey.keyType = QWLANFW_KEY_TYPE_GTK;
            }
            else
            {
                removeKey.keyType = QWLANFW_KEY_TYPE_PTK;
            }
            removeKey.keyIndex = keyIdx;   //For Libra, micKeyIdx = keyIdx
            removeKey.dpuIndex = dpuIdx;
            removeKey.reserved0 = 0;
            status = halFW_SendMsg(pMac, HAL_MODULE_ID_WAPI, uFwMesgType, 0, 
                    sizeof(Qwlanfw_AddRemoveKeyReqType), (void *)&removeKey, FALSE, NULL);
            if(!HAL_STATUS_SUCCESS(status))
            {
                if(pMac->hal.halMac.isFwInitialized)
                {
                    //if FW already initialized, should never fail. Assert here.
                    VOS_ASSERT(0);
                }
            }
        }
    }
#endif

generate_response:
    param->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_REMOVE_STAKEY_RSP, dialog_token, (void *) param, 0);
    return;

}

/*
 * Get the DPU signature information
 */
void
halMsg_GetDpuParams(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpGetDpuParams  param)
{
    tANI_U8 dpuIdx = 0;
    tANI_U8 dpuSig = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    status = halTable_GetStaDpuIdx(pMac, (tANI_U8 ) param->staIdx, &dpuIdx);

    if(status != eHAL_STATUS_SUCCESS)
        goto generate_response;

    status = halDpu_GetSignature(pMac, dpuIdx, &dpuSig);

generate_response:
    // send a response to caller.
    param->status = status;
    param->dpuDescIndx = (tANI_U8)dpuIdx;
    param->dpuSignature =  (tANI_U8) (dpuSig & 0xf);


    halMsg_GenerateRsp(pMac, SIR_HAL_GET_DPUINFO_RSP, dialog_token, (void *) param, 0);
    return;

}


////////////////////////////////////////////////////////////////////////////
//                 DPU Stats API
////////////////////////////////////////////////////////////////////////////
/*
 * Get the DPU statistics for a given sta
 */
void
halMsg_GetDpuStats(
    tpAniSirGlobal  pMac,
    tANI_U16        dialog_token,
    tpDpuStatsParams  param)
{
    tANI_U8 dpuIdx;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    status = halTable_GetStaDpuIdx(pMac, (tANI_U8 ) param->staIdx, & dpuIdx);

    if(status != eHAL_STATUS_SUCCESS)
        goto generate_response;

    status = halDpu_GetStatus(pMac, dpuIdx, param);

generate_response:
    // send a response to caller.
    param->status = status;

    halMsg_GenerateRsp(pMac, SIR_HAL_DPU_STATS_RSP, dialog_token, (void *) param, 0);
    return;

}

/* -----------------------------------------
 * Function: halMsg_updateBeaconParam()
 *
 * NOTE: Srini said the protection mechanism would need to be revised as the draft changes
 *       for now, this code is based on draft 1.07.
 * -----------------------------------------
 */



eHalStatus halMsg_updateBeaconParam(tpAniSirGlobal pMac, tpUpdateBeaconParams pBeaconParams)
{
    eHalStatus status = (eHalStatus)eHAL_STATUS_FAILURE;
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U32 bssIdx = pBeaconParams->bssIdx;
    tANI_U32 updateBssCfg = 0;
    tANI_U8  staIdx, staType;

    //nothing changed. Bogus message?
    if(0 == (PARAM_BEACON_UPDATE_MASK & pBeaconParams->paramChangeBitmap ))
        goto out;

    bssTable = &bssTable[bssIdx];
    if(pBeaconParams->paramChangeBitmap & PARAM_BCN_INTERVAL_CHANGED){
        bssTable->tuBeaconInterval = pBeaconParams->beaconInterval;
        //should send a add bss message to softmac again to update its internal beacon
        //schedling.
        // Update the beacon interval into the FW system config
        halPS_SetBeaconInterval(pMac, pBeaconParams->beaconInterval);
        updateBssCfg = 1;
    }

    if(pBeaconParams->paramChangeBitmap & PARAM_llBCOEXIST_CHANGED){
        HALLOGW( halLog(pMac, LOGW, FL("BcnParmUpdated: 11B Coexist changed to %d\n"),
                    pBeaconParams->llbCoexist));
        bssTable->bssRaInfo.u.bit.llbCoexist = pBeaconParams->llbCoexist;
        __halMsg_update11bCoexist(pMac, pBeaconParams->llbCoexist );
    }

    //9.20.4 An HT STA transmitting a 40 MHz PPDU shall ensure this transmission is protected if its AP indicates there
    //are 20 MHz capable HT STAs and/or non-HT STAs associated in the same BSS. Operating mode = 11 or 10
    //in the HT Information element indicates this case.
    //If such protection is needed, it shall be implemented in the following way: non-HT control frames RTS/CTS,
    //MPDU/Ack or A-MPDU/BlockAck, or CTS-to-self may be used for protection of a PPDU, a frame exchange
    //sequence or an entire TXOP (Ed: CID 7679). MPDU/Ack shall only be used for protection of the transmission
    //in a 20 MHz wide channel. (Ed: CID 7679)
    //An HT STA operating in 20/40 MHz Mode transmitting a 40 MHz PPDU under the protection of a RTS-CTS
    //exchange or a CTS-to-self frame shall transmit a RTS or a CTS-to-self using a non-HT duplicate frame. (Ed:
    //CID 1455)(ED: CID 1457)

    //if HT20 coexist while BSS is 20/40 capable, send protection frame with duplicate 11b rates if 11b coexist, otherwise
    //send with duplicate 11g rates.
    if(pBeaconParams->paramChangeBitmap & PARAM_HT20MHZCOEXIST_CHANGED)
        bssTable->bssRaInfo.u.bit.ht20Coexist = pBeaconParams->ht20MhzCoexist;

    //use 11bCoexist as the only signal to enable protection, ignore the shortTimeChanged change.
    if(pBeaconParams->paramChangeBitmap & PARAM_llGCOEXIST_CHANGED)
        bssTable->bssRaInfo.u.bit.llgCoexist = (pBeaconParams->llgCoexist);

    if(pBeaconParams->paramChangeBitmap & PARAM_NON_GF_DEVICES_PRESENT_CHANGED){
        bssTable->bssRaInfo.u.bit.nonGfPresent = pBeaconParams->llnNonGFCoexist;
        //if nonGFpresent=1: 9.13.3.2
        //A STA that is associated with a BSS shall protect Green Field PPDUs using any of the protection mechanisms
        //described in 9.13.6 (Protection mechanisms for A-MPDU (Ed: CID 1286) (Ed: CID 414) exchange sequences)
        //(Ed: CID 2553) when its AP transmits an HT Information element with the Non-Greenfield STAs Present
        //field set to 1. (Ed: CID 1298)

        //if selected rate is using GF preamble and nonGFpresent=1 (11abg, non GF HT) do protection
        //TODO: Add a nonGF present flag to BSS and use it in deciding protection policy

    }
    if(pBeaconParams->paramChangeBitmap & PARAM_RIFS_MODE_CHANGED){
        //9.13.3.1 RIFS protection
        //The AP shall set the RIFS Mode field of the HT Information element to 1 if there are no APSD non-HT STAs
        //associated, otherwise it shall be set to 0. (Ed: CID 1107 - moved from table n21 in D1.02)
        //A STA that is associated with a BSS shall protect RIFS sequences when the Operating Mode field of the HT
        //Information element transmitted by its AP is set to 1 (there may be non-HT STAs in either the primary or
        //secondary channel or both) or 3 (mixed) (Ed: CID 7609). (Ed: CID 1297)
        //RIFS shall only be used when the RIFS Mode field of the HT Information element is set to 1. (Ed: CID 6765)

        //Whether using RIFS or not is decided by Softmac. Protection decision is partly done in softmac level.
        //Add a allowRifs cfg to BSS cfg. In SOftmac, if the rate info indicates no protection is needed, and softmac
        //wants to use RIFS to transmit the frame when allowRifs=1, if 11gCoexist=1 and 11bCoexist=0, then send CTS2S.
        if( pBeaconParams->fRIFSMode ){ //chg from 0->1
            pMac->hal.halMac.nonRifsBssCount--;
            pMac->hal.halMac.rifsBssCount++;
        }else{ //chg from 1->0
            pMac->hal.halMac.nonRifsBssCount++;
            pMac->hal.halMac.rifsBssCount--;
        }
        updateBssCfg = 1;
    }

    if(pBeaconParams->paramChangeBitmap & PARAM_LSIG_TXOP_FULL_SUPPORT_CHANGED){
        //9.13.5.1 A STA shall not transmit a frame using L-SIG TXOP Protection directed to a recipient that does not support
        //L-SIG TXOP Protection. When the STA is associated with an AP, support at a recipient that is associated
        //with the same AP is indicated if the L-SIG TXOP Protection Full Support field is set to 1 in the HT Information
        //element broadcast in Beacons transmitted by the AP with which the STAs are associated. L-SIG TXOP
        //support at the recipient may additionally be determined through examination of HT Capability elements exchanged
        //during association or exchanged during DLS setup. (Ed: CID 6780)
        //Non-HT STAs are not able to receive any PPDU that starts during the L-SIG duration. Therefore, no frame
        //shall be transmitted to a non-HT STA (Ed: CID 1317) during an L-SIG protected TXOP.
        //L-SIG TXOP Protection should not be used and the implementers of L-SIG TXOP Protection are advised to
        //include a NAV based fallback mechanism, if it is determined that the mechanism fails to effectively supp
        //non-HT transmissions. How this is determined is outside the scope of this standard.

        //If (1) peer is HT STA and (2) AP indicates Full L-SIG TXOP protection Support =1, 3) rate is using mixed mode preamble,
        //frames send to HT STA can use L-SIG TXOP to protect PPDUs.
    }

    if(updateBssCfg){

        // Call appropriate MTU/TPE function to update  RIFS mode.
        HALLOGE( halLog(pMac, LOGE, FL("updating BSS config at MTU/TPE not implemented yet\n")));
    }

    if(pBeaconParams->paramChangeBitmap & PARAM_SHORT_PREAMBLE_CHANGED){
        bssTable->bssRaInfo.u.bit.fShortPreamble  = 
            pBeaconParams->fShortPreamble;
        //Now for all STAs, update their rates
        for(staIdx = 0; staIdx <pMac->hal.halMac.maxSta ; staIdx++) {
            if ((halTable_GetStaType(pMac, staIdx, &staType) == eHAL_STATUS_SUCCESS) && 
                    (staType == STA_ENTRY_SELF)) {
                continue;
            }
                halRateUpdateStaRateInfo(pMac, staIdx);
            }
        }

    if(pBeaconParams->paramChangeBitmap & PARAM_SHORT_SLOT_TIME_CHANGED){
        tMtuMode newMode = MODE_11G_PURE;
        tMtuMode curMode = halMTU_getMode(pMac);
        bssTable->bssRaInfo.u.bit.fShortSlot  = pBeaconParams->fShortSlotTime;
        if(!pBeaconParams->fShortSlotTime && (curMode == MODE_11G_PURE))
        {
            newMode = MODE_11G_MIXED;
        }
        else if(pBeaconParams->fShortSlotTime && (curMode == MODE_11G_MIXED))
        {
            if(halMsg_allBssShortSlotEnabled(pMac))
            {
                newMode = MODE_11G_PURE;
            }
        }
        if(curMode != newMode)
        {
            pMac->hal.halMac.fShortSlot  = pBeaconParams->fShortSlotTime;
            halMTU_update11gSlotTimingParams(pMac, newMode);
        }
    }

    /* update BssInfo change to FW */
    if(pBeaconParams->paramChangeBitmap & \
            ( PARAM_SHORT_PREAMBLE_CHANGED | PARAM_SHORT_SLOT_TIME_CHANGED | PARAM_llBCOEXIST_CHANGED | \
              PARAM_llGCOEXIST_CHANGED | PARAM_HT20MHZCOEXIST_CHANGED | PARAM_NON_GF_DEVICES_PRESENT_CHANGED |\
              PARAM_RIFS_MODE_CHANGED )) {
        halMacRaBssInfoToFW(pMac, &bssTable->bssRaInfo, pBeaconParams->bssIdx);
    }

    status = eHAL_STATUS_SUCCESS;

out:
    palFreeMemory( pMac->hHdd, pBeaconParams);
    return status;

}


/* -----------------------------------------
 * Function: halMsg_updateEdcaParam()
 *
 * NOTE:
 *   Update the following EDCA Parameter:
 *       1) ACM
 *       2) AIFSN
 *       3) CWMin
 *       4) CWMax
 *       5) TXOP
 * -----------------------------------------
 */
eHalStatus halMsg_updateEdcaParam(tpAniSirGlobal pMac, tEdcaParams *pEdcaParams)
{
    tANI_U8  i;
    tANI_U16 aEdcaTxop[MTU_BKID_HI_NUM];

    // Overwrite the existing edca profile
    palCopyMemory( pMac->hHdd, (void *)&pMac->hal.edcaParam[SIR_MAC_EDCAACI_BESTEFFORT], (void *)&pEdcaParams->acbe, sizeof(tSirMacEdcaParamRecord) );
    palCopyMemory( pMac->hHdd, (void *)&pMac->hal.edcaParam[SIR_MAC_EDCAACI_BACKGROUND], (void *)&pEdcaParams->acbk, sizeof(tSirMacEdcaParamRecord) );
    palCopyMemory( pMac->hHdd, (void *)&pMac->hal.edcaParam[SIR_MAC_EDCAACI_VIDEO], (void *)&pEdcaParams->acvi, sizeof(tSirMacEdcaParamRecord) );
    palCopyMemory( pMac->hHdd, (void *)&pMac->hal.edcaParam[SIR_MAC_EDCAACI_VOICE], (void *)&pEdcaParams->acvo, sizeof(tSirMacEdcaParamRecord) );

    for (i=0; i<MAX_NUM_AC; i++)
    {
            HALLOG1( halLog(pMac, LOG1, FL("HAL received the following EDCA params from PE\n")));
            HALLOG1( halLog(pMac, LOG1, FL("AC[%d]:  AIFSN %d, ACM %d, CWmin %d, CWmax %d, TxOp %d\n"),
              i,
              pMac->hal.edcaParam[i].aci.aifsn,
              pMac->hal.edcaParam[i].aci.acm,
              pMac->hal.edcaParam[i].cw.min,
              pMac->hal.edcaParam[i].cw.max,
              pMac->hal.edcaParam[i].txoplimit));
    }

    for (i=0;  i < MAX_NUM_AC; i++)
    {
        // update CWMin, CWMax
        halMTU_updateCW(pMac, i, pMac->hal.edcaParam[i].cw.min, pMac->hal.edcaParam[i].cw.max);
    }

    halMTU_updateAIFS(pMac, pMac->hal.edcaParam);

    palZeroMemory(pMac->hHdd, aEdcaTxop, sizeof(aEdcaTxop));

    aEdcaTxop[MTU_BKID_AC_BE] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_BESTEFFORT].txoplimit);
    aEdcaTxop[MTU_BKID_AC_BK] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_BACKGROUND].txoplimit);
    aEdcaTxop[MTU_BKID_AC_VI] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_VIDEO].txoplimit);
    aEdcaTxop[MTU_BKID_AC_VO] = CONVERT_TXOP_TO_US(pMac->hal.edcaParam[SIR_MAC_EDCAACI_VOICE].txoplimit);

    HALLOGW( halLog(pMac, LOGW, FL("dump EDCA reg before update EDCA \n")));
    halTpe_DumpEdcaTxOp(pMac);
    // update TXOP
    if (halTpe_UpdateEdcaTxOp(pMac, aEdcaTxop) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halTpe_UpdateEdcaTxOp() failed \n")));
        return eHAL_STATUS_FAILURE;
    }
    HALLOGW( halLog(pMac, LOGW, FL("dump EDCA reg after update EDCA \n")));
    halTpe_DumpEdcaTxOp(pMac);

    // update Short retry and Long Retry limit
    if (halMsg_updateRetryLimit(pMac) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("halMsg_updateRetryLimit() failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    palFreeMemory( pMac->hHdd, pEdcaParams);

    return eHAL_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
//FIXME : Add the function details here.
\fn halMsgAddTs
\brief
\param pMac -
\param dialog_token -
\param param - pointer to
\return None
-------------------------------------------------------------*/
tSirRetStatus
halMsg_AddTs(
  tpAniSirGlobal  pMac,
  tANI_U16        dialog_token,
  tpAddTsParams    param)
{
    return halWmmAddTspec(pMac, param);
}

/** -------------------------------------------------------------
//FIXME : Add the function details here.
\fn halMsgDelTs
\brief
\param pMac -
\param dialog_token -
\param param - pointer to
\return None
-------------------------------------------------------------*/
tSirRetStatus
halMsg_DelTs(
  tpAniSirGlobal  pMac,
  tANI_U16        dialog_token,
  tpDelTsParams    param)
{
    return halWmmDelTspec(pMac, param);
}

/** -------------------------------------------------------------
//FIXME : Add the function details here.
\fn halMsg_RegisterPECallback
\brief
\param pMac -
\param pHalMsgCB -
\return None
-------------------------------------------------------------*/
void halMsg_RegisterPECallback(tpAniSirGlobal pMac, void *pHalMsgCBptr)
{
    tpHalIndCB pHalCB = (tpHalIndCB)pHalMsgCBptr;

    if (pHalCB->pHalIndCB == NULL)
    {
        HALLOGE( halLog(pMac, LOGE, FL("PE Callback function pointer NULL")));
    }
    else
    {
        pMac->hal.pPECallBack = pHalCB->pHalIndCB;
    }

    palFreeMemory(pMac->hHdd, pHalMsgCBptr);
}

#ifndef WLAN_SOFTAP_FEATURE
#ifdef FIXME_VOLANS
/**
  *     @function : halFillBeaconGenParams
  *     @brief      : Constructs the PreBeacon Message from the Message given by
  *                         S/W mac.
  *
  *     @param    :  pMac - tpAniSirGlobal
  *                       pBeaconPreMesg - Message params sent by S/W Mac
  *                       pBeaconGenParam - Message params to be sent to PE
  *
  *     @return    :  void
  */

static eHalStatus
halFillBeaconGenParams(tpAniSirGlobal pMac, tBmuStaQueueData *pStaQueueData,
                                                                           tpBeaconGenParams pBeaconGenParams, tANI_U32 totalSta)
{
    eHalStatus    status = eHAL_STATUS_SUCCESS;

    /** Fill the Beacon Prams from the preBeconMsg got from S/W Mac*/
    pBeaconGenParams->bssIdx = (tANI_U8)pStaQueueData->bssIdx;

    status = halTable_FindAddrByBssid( pMac, pBeaconGenParams->bssIdx , pBeaconGenParams->bssId );
    if (eHAL_STATUS_SUCCESS != status)
        HALLOGE( halLog( pMac, LOGE, FL("BSSID lookup fails for bssindex: %d!\n"), pBeaconGenParams->bssIdx));

    /* No need to query BTQM queues for getting BSS context. This code doesn't
       serve the purpose during pre-beacon indication. 

       During pre-beacon indication (halMsg_BeaconPre()), we are querying BTQM queues 
       for all STA indices to see if there is any traffic for that STA index. 
       During pre-beacon,the purpose is to get some parameters like StaIdx, BssIndex 
       and to provide them to PE. Looks like this code suits more for SoftAP (may be Gen4)
       implementation where HAL also queries DTIM count for that STA index and takes some 
       action.   

       As a fix, getting relevant information of pre-beacon from HAL Table.
       */
#ifdef FIXME_VOLANS
    pBeaconGenParams->dtimCount = (tANI_U8)pStaQueueData->dtimCount;
    pBeaconGenParams->fBroadcastTrafficPending = (tANI_U8)pStaQueueData->fBroadcastTrafficPending;
    pBeaconGenParams->numOfSta = (tANI_U8)pStaQueueData->numOfSta;
    pBeaconGenParams->numOfStaWithoutData =(tANI_U8)pStaQueueData->numOfStaWithoutData;

    HALLOG2( halLog(pMac, LOG2, FL("numStaWithData: %d numStaWithoutData: %d BrcastTrffic: %d\n"),
                pStaQueueData->numOfSta, pStaQueueData->numOfStaWithoutData, pStaQueueData->fBroadcastTrafficPending));


    if (totalSta > 0)
    {
        tANI_U32      cnt;
        tANI_U8        staId;
        tpBeaconGenStaInfo  pStaInfo;

        /** When there are STA in PowerSave Piggy Back the StaInfo's after BeaconGenParams*/
        pStaInfo = (tpBeaconGenStaInfo)((tANI_U8 *)pBeaconGenParams + sizeof(*pBeaconGenParams));
        /** For all the STA in PS, obtain the current MpduTxAck Count*/
        for(cnt=0; cnt < totalSta; cnt++) {
            pStaInfo[cnt].assocId = pStaQueueData->assocId[cnt];
            status = halTable_FindStaIdByAssocId(pMac, pStaQueueData->assocId[cnt], &staId);
            if (status != eHAL_STATUS_SUCCESS) {
                HALLOGE( halLog(pMac, LOGE, FL("Not Able to find the StaID for the assocId %d\n"),
                            pStaInfo[cnt].assocId));
                return status;
            }
            HALLOG2( halLog(pMac, LOG2, FL("Assoc ID: %d\t StaId : %d \n"), pStaQueueData->assocId[cnt], staId));
            status = halMacRaGetStaTxCount(pMac, staId, &pStaInfo[cnt].staTxAckCnt);
            if (status != eHAL_STATUS_SUCCESS) {
                HALLOGE( halLog(pMac, LOGE, FL("Not Able to get the staTxAckCnt for StaId %d\n"), staId));
                return status;
            }
        }
    }

#endif /* FIXME_VOLANS */
    return status;
}
#endif /* FIXME_VOLANS */
#endif

/* --------------------------------------------------
 * FUNCTION:  halDPU_updateFragThreshold()
 *
 * NOTE:
 *   For all valid STA, update its fragment threshold.
 * --------------------------------------------------
 */
eHalStatus halMsg_updateFragThreshold(tpAniSirGlobal pMac)
{
    tANI_U8      staid;
    tANI_U8      dpuIdx;
    tANI_U32     val;
    tpStaStruct  staTable = (tpStaStruct) pMac->hal.halMac.staTable;

    if ( wlan_cfgGetInt(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, &val) != eSIR_SUCCESS)
        HALLOGE( halLog(pMac, LOGE, FL("cfgGet WNI_CFG_FRAGMENTATION_THRESHOLD Failed\n")));

    for (staid=0; staid < pMac->hal.halMac.maxSta; staid++, staTable++)
    {
        // Apply fragmentation to only unicast STAs
        // Self-STA entries are used for broadcast/multicast addresses
        if ((staTable->valid == 1) && (staTable->staType != STA_ENTRY_SELF))
        {
            dpuIdx = staTable->dpuIndex;
            halDpu_SetFragThreshold(pMac, dpuIdx, (tANI_U16) val);
        }
    }

    return eHAL_STATUS_SUCCESS;
}

/**
 * \brief Process SIR_HAL_ADDBA_REQ coming from PE
 *
 * \sa halMsg_AddBA
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param dialogToken A unique dialogToken for each REQ from sender
 *
 * \param pAddBAParams A pointer to the tAddBAParams structure
 *
 * \return none
 */
void halMsg_AddBA( tpAniSirGlobal  pMac,
    tANI_U16 dialog_token,
    tpAddBAParams pAddBAParams )
{
    tANI_U16 baBufferSize, baSessionID;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSavedAddBAReqParamsStruct addBAReqParamsStruct;
    tHalCfgSta staEntry;
    tCfgTrafficClass tcCfg;

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_ValidateStaIndex( pMac,
                                                 (tANI_U8) pAddBAParams->staIdx )))
    {
        HALLOGW( halLog(  pMac, LOGW,
                    FL("Invalid STA Index %d\n"),
                    pAddBAParams->staIdx ));

        goto generate_response;
    }
    else
    {
        // Restore the "saved" STA context in HAL for this STA
        halTable_RestoreStaConfig( pMac, (tHalCfgSta *) &staEntry, (tANI_U8 ) pAddBAParams->staIdx );
    }

    // Restore the current TC settings from the saved STA config
    // This ensures that the existing TC configuration
    // for this TID does not get over-written
    palCopyMemory( pMac->hHdd,
            (void *) &tcCfg,
            (void *) &(staEntry.tcCfg[pAddBAParams->baTID]),
            sizeof( tCfgTrafficClass ));

  if( eBA_RECIPIENT == pAddBAParams->baDirection )
  {
    //Force baAllocateBuffer to use default window size
    baBufferSize = 0;

#ifdef FEATURE_ON_CHIP_REORDERING
    if(pMac->hal.halMac.numOfOnChipReorderSessions < pMac->hal.halMac.maxNumOfOnChipReorderSessions)
    {
      baBufferSize = (pMac->hal.halMac.maxNumOfOnChipReorderSessions == 1)? 0 : BA_DEFAULT_RX_BUFFER_SIZE/2;
    }
#endif

    if( eHAL_STATUS_SUCCESS ==
        (status = baAllocateBuffer( pMac,
                                    &baBufferSize )))
    {
            if( eHAL_STATUS_SUCCESS ==
                    (status = baAllocateSessionID( pMac,
                                                   pAddBAParams->staIdx,
                                                   pAddBAParams->baTID,
                                                   &baSessionID )))
            {
                pAddBAParams->baBufferSize = baBufferSize;

                // Update the STA context with the BA Session ID
                halTable_SetStaBASessionID( pMac,
                        (tANI_U8) pAddBAParams->staIdx,
                        pAddBAParams->baTID,
                        baSessionID );

                pAddBAParams->status = status;
                addBAReqParamsStruct.pAddBAReqParams = pAddBAParams;
                halTable_SetStaAddBAReqParams(pMac, pAddBAParams->staIdx,
                        pAddBAParams->baTID, addBAReqParamsStruct);

                tcCfg.fUseBARx = eBA_ENABLE;
                tcCfg.fRxCompBA = eBA_ENABLE;
                tcCfg.fRxBApolicy = pAddBAParams->baPolicy;

                // Update BA buffer size with what we just allocated
                tcCfg.rxBufSize = pAddBAParams->baBufferSize;

                tcCfg.tuRxBAWaitTimeout = pAddBAParams->baTimeout;
                tcCfg.role = eBA_RECIPIENT;

                // Update "local STA table entry" with new A-MPDU/BA
                // related settings that just got configured
                palCopyMemory( pMac->hHdd,
                        (void *) &(staEntry.tcCfg[pAddBAParams->baTID]),
                        (void *) &tcCfg,
                        sizeof( tCfgTrafficClass ));

                // Save/Update copy of sta entry.
                halTable_SaveStaConfig( pMac, &staEntry, (tANI_U8) pAddBAParams->staIdx );

                // Configure the device
                if( eHAL_STATUS_SUCCESS !=
                        (status = baAddBASession( pMac, pAddBAParams )))
                {
                    HALLOGW( halLog(  pMac, LOGW,
                                FL("Unable to add BA session for STA index %d, TID %d. "
                                    "Error code - [%d]\n"),
                                pAddBAParams->staIdx,
                                pAddBAParams->baTID,
                                status ));
                    baReleaseSessionID( pMac,
                            pAddBAParams->staIdx,
                            pAddBAParams->baTID );
                    halTable_SetStaBASessionID( pMac,
                            (tANI_U8) pAddBAParams->staIdx,
                            pAddBAParams->baTID,
                            BA_SESSION_ID_INVALID );
                    tcCfg.fUseBARx = eBA_DISABLE;
                    tcCfg.fRxCompBA = eBA_DISABLE;
                    tcCfg.tuRxBAWaitTimeout = 0;
                }
            }
            if(eHAL_STATUS_SUCCESS != status)
                baReleaseBuffer( pMac, baBufferSize );
        }

        if( eHAL_STATUS_SUCCESS != status )
            goto generate_response;
    }
    else
    {
        pAddBAParams->status = status;
        addBAReqParamsStruct.pAddBAReqParams = pAddBAParams;
        halTable_SetStaAddBAReqParams(pMac, pAddBAParams->staIdx,
                pAddBAParams->baTID, addBAReqParamsStruct);

        tcCfg.fUseBATx = eBA_ENABLE;
        tcCfg.fTxCompBA = eBA_ENABLE;
        tcCfg.fTxBApolicy = pAddBAParams->baPolicy;
        tcCfg.txBufSize = pAddBAParams->baBufferSize;
        tcCfg.tuTxBAWaitTimeout = pAddBAParams->baTimeout;
        tcCfg.role = eBA_INITIATOR;

        // Update "local STA table entry" with new A-MPDU/BA
        // related settings that just got configured
        palCopyMemory( pMac->hHdd,
                (void *) &(staEntry.tcCfg[pAddBAParams->baTID]),
                (void *) &tcCfg,
                sizeof( tCfgTrafficClass ));

        // Save/Update copy of sta entry.
        halTable_SaveStaConfig( pMac, &staEntry, (tANI_U8) pAddBAParams->staIdx );

        // Configure the device
        if( eHAL_STATUS_SUCCESS !=
                (status = baAddBASession( pMac, pAddBAParams )))
        {
            HALLOGW( halLog(  pMac, LOGW,
                        FL("Unable to add BA session for STA index %d, TID %d. "
                            "Error code - [%d]\n"),
                        pAddBAParams->staIdx,
                        pAddBAParams->baTID,
                        status ));
            tcCfg.fUseBATx = eBA_DISABLE;
            tcCfg.fTxCompBA = eBA_DISABLE;
            tcCfg.tuTxBAWaitTimeout = 0;
        }    
    }

generate_response:

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        if(status == eHAL_STATUS_SUCCESS){
            WLAN_VOS_DIAG_EVENT_DEF(AddBaSuccessStatus, vos_event_wlan_add_block_ack_success_payload_type);
            palZeroMemory(pMac->hHdd, &AddBaSuccessStatus, sizeof(vos_event_wlan_add_block_ack_success_payload_type));
            palCopyMemory(pMac->hHdd, &AddBaSuccessStatus.ucBaPeerMac,pAddBAParams->peerMacAddr, sizeof(AddBaSuccessStatus.ucBaPeerMac));
            AddBaSuccessStatus.ucBaTid   = pAddBAParams->baTID;
            AddBaSuccessStatus.ucBaBufferSize = (v_U8_t) pAddBAParams->baBufferSize;
            AddBaSuccessStatus.usBaSSN = pAddBAParams->baSSN;
            AddBaSuccessStatus.fInitiator = (eBA_RECIPIENT == pAddBAParams->baDirection)?0:1;
            WLAN_VOS_DIAG_EVENT_REPORT(&AddBaSuccessStatus, EVENT_WLAN_ADD_BLOCK_ACK_SUCCESS);
        }else{
            WLAN_VOS_DIAG_EVENT_DEF(AddBaFailStatus, vos_event_wlan_add_block_ack_failed_payload_type);
            palZeroMemory(pMac->hHdd, &AddBaFailStatus, sizeof(vos_event_wlan_add_block_ack_failed_payload_type));
            palCopyMemory(pMac->hHdd, &AddBaFailStatus.ucBaPeerMac,pAddBAParams->peerMacAddr, sizeof(AddBaFailStatus.ucBaPeerMac));
            AddBaFailStatus.ucBaTid   = pAddBAParams->baTID;
            AddBaFailStatus.ucReasonCode = status;
            AddBaFailStatus.fInitiator = (eBA_RECIPIENT == pAddBAParams->baDirection)?0:1;
            WLAN_VOS_DIAG_EVENT_REPORT(&AddBaFailStatus, EVENT_WLAN_ADD_BLOCK_ACK_FAILED);

        }
    }
#endif //FEATURE_WLAN_DIAG_SUPPORT

    pAddBAParams->status = status;
    if(eHAL_STATUS_SUCCESS != status)
        halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, dialog_token, (void *) pAddBAParams, 0);
    return;
}

/**
 * \brief Process SIR_HAL_DELBA_REQ coming from PE
 *
 * \sa halMsg_DelBA
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param dialogToken A unique dialogToken for each REQ from sender
 *
 * \param pDelBAParams A pointer to the tDelBAParams structure
 *
 * \return none
 */
void halMsg_DelBA( tpAniSirGlobal  pMac,
    tANI_U16 dialog_token,
    tpDelBAParams pDelBAParams )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalCfgSta staEntry;
    tCfgTrafficClass tcCfg;

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_ValidateStaIndex( pMac,
                                                 (tANI_U8) pDelBAParams->staIdx )))
    {
        HALLOGW( halLog(  pMac, LOGW,
                    FL("Invalid STA Index %d\n"),
                    pDelBAParams->staIdx ));

        goto free_mem;
    }
    else
    {
        // Restore the "saved" STA context in HAL for this STA
        halTable_RestoreStaConfig( pMac, (tHalCfgSta *) &staEntry, (tANI_U8 ) pDelBAParams->staIdx );

        // Determine if a BA session is currently active for
        // this TID or not
        if( !staEntry.tcCfg[pDelBAParams->baTID].fUseBARx &&
                !staEntry.tcCfg[pDelBAParams->baTID].fUseBATx )
        {
            HALLOGW( halLog(  pMac, LOGW,
                        FL("No BA session is active for TID %d\n"),
                        pDelBAParams->baTID ));

            // FIXME - Do we update the HAL Status Codes with
            // newer status info?
            status = eHAL_STATUS_INVALID_PARAMETER;

            goto free_mem;
        }
    }

    // Restore the current TC settings from the saved STA config
    // This ensures that the existing TC configuration
    // for this TID does not get over-written
    palCopyMemory( pMac->hHdd,
            (void *) &tcCfg,
            (void *) &(staEntry.tcCfg[pDelBAParams->baTID]),
            sizeof( tCfgTrafficClass ));

    if( eBA_RECIPIENT == pDelBAParams->baDirection )
    {
        tcCfg.fUseBARx = eBA_DISABLE;
        tcCfg.fRxCompBA = eBA_DISABLE;
        tcCfg.tuRxBAWaitTimeout = 0;

        // Reclaim BA buffers
        baReleaseBuffer( pMac, (tANI_U16) tcCfg.rxBufSize );

        // Release BA Session ID back to the pool
        baReleaseSessionID( pMac,
                pDelBAParams->staIdx,
                pDelBAParams->baTID );

        // Update the STA context with the BA Session ID
        halTable_SetStaBASessionID( pMac,
                (tANI_U8) pDelBAParams->staIdx,
                pDelBAParams->baTID,
                BA_SESSION_ID_INVALID );
    }
    else
    {
        tcCfg.fUseBATx = eBA_DISABLE;
        tcCfg.fTxCompBA = eBA_DISABLE;
        tcCfg.tuTxBAWaitTimeout = 0;
    }

    // Update local STA table entry with new
    // A-MPDU/BA related settings
    palCopyMemory( pMac->hHdd,
            (void *) &(staEntry.tcCfg[pDelBAParams->baTID]),
            (void *) &tcCfg,
            sizeof( tCfgTrafficClass ));

    // Save/Update copy of sta entry.
    halTable_SaveStaConfig( pMac, &staEntry, (tANI_U8) pDelBAParams->staIdx );

    if( eHAL_STATUS_SUCCESS !=
            (status = baDelBASession( pMac, pDelBAParams )))
    {
        HALLOGW( halLog(  pMac, LOGW,
                    FL("Unable to delete BA session for STA index %d, TID %d. "
                        "Error code - [%d]\n"),
                    pDelBAParams->staIdx,
                    pDelBAParams->baTID,
                    status ));
    }  

free_mem:
#ifdef FEATURE_WLAN_DIAG_SUPPORT
      {
          tANI_U8 *pStrAddr;
          if(status == eHAL_STATUS_SUCCESS){
              WLAN_VOS_DIAG_EVENT_DEF(DelBaSuccessStatus, vos_event_wlan_add_block_ack_deleted_payload_type);
              palZeroMemory(pMac->hHdd, &DelBaSuccessStatus, sizeof(vos_event_wlan_add_block_ack_deleted_payload_type));
              pStrAddr = (tANI_U8 *)&DelBaSuccessStatus.ucBaPeerMac;
              halTable_GetStaAddr(pMac, (tANI_U8) pDelBAParams->staIdx, &pStrAddr);
              DelBaSuccessStatus.ucBaTid   = pDelBAParams->baTID;
              DelBaSuccessStatus.ucDeleteReasonCode = 0;
              WLAN_VOS_DIAG_EVENT_REPORT(&DelBaSuccessStatus, EVENT_WLAN_DELETE_BLOCK_ACK_SUCCESS);
          }else{
              WLAN_VOS_DIAG_EVENT_DEF(DelBaFailStatus, vos_event_wlan_add_block_ack_delete_failed_payload_type);
              palZeroMemory(pMac->hHdd, &DelBaFailStatus, sizeof(vos_event_wlan_add_block_ack_failed_payload_type));
              pStrAddr = (tANI_U8 *)&DelBaFailStatus.ucBaPeerMac;
              halTable_GetStaAddr(pMac, (tANI_U8)pDelBAParams->staIdx, &pStrAddr);
              DelBaFailStatus.ucBaTid   = pDelBAParams->baTID;
              DelBaFailStatus.ucDeleteReasonCode = 0;
              DelBaFailStatus.ucFailReasonCode = status;
              WLAN_VOS_DIAG_EVENT_REPORT(&DelBaFailStatus, EVENT_WLAN_DELETE_BLOCK_ACK_FAILED);
          }
      }
#endif //FEATURE_WLAN_DIAG_SUPPORT


    palFreeMemory( pMac->hHdd, (tANI_U8 *) pDelBAParams );
    return;
}


void halMsg_BAFail(tpAniSirGlobal pMac, tANI_U16 dialog_token,
                        tpAddBARsp pAddBARsp)
{
    tpAddBAParams pAddBAParams = NULL;
    tDelBAParams delBAParams;
    tSavedAddBAReqParamsStruct addBAReqParamsStruct;
    tpStaStruct t = (tpStaStruct) pMac->hal.halMac.staTable;
    tANI_U16 staIdx;
    tANI_U8 tid;

    HALLOG1( halLog( pMac, LOG1, FL("In halMsg_BAFail.......... \n")));
  
    /* Deactivate the addBARspTimer, as we are sending ADD BA RSP to AP with failure information. */
    if (tx_timer_deactivate(&pMac->hal.addBARspTimer) != TX_SUCCESS)
    {
	/** Could not deactivate Log error*/
	HALLOGP( halLog(pMac, LOGP,
		  FL("Unable to deactivate addBARsp timer\n")));
	//return eHAL_STATUS_FAILURE;
	return ;
    }


    for(staIdx = 0; staIdx < pMac->hal.halMac.maxSta; staIdx++)
    {
        if(t[staIdx].valid)
        {
            for(tid = 0; tid < STACFG_MAX_TC; tid++)
            {
                addBAReqParamsStruct = t[staIdx].addBAReqParams[tid];
                if(addBAReqParamsStruct.pAddBAReqParams)
                {
                    pAddBAParams = addBAReqParamsStruct.pAddBAReqParams;
                    if(pAddBARsp->baSessionID == t[pAddBAParams->staIdx].baSessionID[pAddBAParams->baTID])
                    {
                        delBAParams.staIdx = pAddBAParams->staIdx;
                        delBAParams.baTID = pAddBAParams->baTID;

			/* The baDirection is to be copied from AddBAParams, it cannot be assumed to be eBA_INITIATOR always*/
			delBAParams.baDirection = pAddBAParams->baDirection;

                        /* Reset the H/W configuration which we did while establishing BA */
                        baDelBASession(pMac, &delBAParams);
                        // Update the STA context with the BA Session ID
                        halTable_SetStaBASessionID( pMac, (tANI_U8) pAddBAParams->staIdx,
                                pAddBAParams->baTID,BA_SESSION_ID_INVALID );

                        pAddBAParams->status = eHAL_STATUS_FAILURE;
                        halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, pAddBAParams->baDialogToken, (void *) pAddBAParams, 0);
                        break;
                    }
                }
            }
        }
    }

    return;
}


#ifdef ANI_SUPPORT_SMPS
/**
 * @function : halMsg_InitRxChainsReg
 *
 * @brief :  Initializes List of RXP Registers and there MIN and MAX values to set the RXP Chains
 *    for runtime changing of the Rx Chains in MIMO PS Dynamic Mode.
 *
 *      LOGIC:
 *
 *      ASSUMPTIONS:
 *
 *      NOTE:
*
 * @param pMac A pointer to the tAniSirGlobal structure
 * @return void
*/

void halMsg_InitRxChainsReg( tpAniSirGlobal pMac)
{
    tSmacHostMesg_RxChainRegUpdate msg;
    uEepromFields       nEeprom_Rx;
    tANI_BOOLEAN      fIsAnalog= eANI_BOOLEAN_FALSE;
    tANI_U32                 count,nRx;


   /** Check for if it is analog links and set the nRx to 2 as we support only 2 Rx chains*/
   if ((pMac->hphy.phy.test.testDisableRfAccess == eANI_BOOLEAN_TRUE) &&
       (pMac->hphy.phy.test.testDisablePhyRegAccess == eANI_BOOLEAN_FALSE)) {
            fIsAnalog= eANI_BOOLEAN_TRUE;
            nRx = 2;
    } else {
        halReadEepromField(pMac, EEPROM_COMMON_NUM_OF_RX_CHAINS, &nEeprom_Rx);
        nRx = nEeprom_Rx.numOfRxChains;
    }

    for (count =0; count < (sizeof(sInitRegValues)/sizeof(tRxChainRegVal)); count ++)
    {

        msg.regSet.regList[count].regIndex  = sInitRegValues[count].regIndex;
        msg.regSet.regList[count].valMin = sInitRegValues[count].valMin;

        switch(nRx) {/** Update the RegSet Structure with the static values defined as per the RxChains*/
            case 2:
                msg.regSet.regList[count].valMax = sInitRegValues[count].valMax[0];
                break;
            case 3:
                msg.regSet.regList[count].valMax = sInitRegValues[count].valMax[1];
                break;
            default:
                msg.regSet.regList[count].valMax = sInitRegValues[count].valMin;
                break;
        }

#ifdef FIXME_GEN5
        if ((sInitRegValues[count].regIndex == GET_PIF_REGINDEX(AGC_TH_RXB_PWR_LOW_REG))
                && fIsAnalog)
            msg.regSet.regList[count].valMin = msg.regSet.regList[8].valMax = 63;
#endif
    }
}

/**
  *     @function : halStaSetMimoPs
  *
  *     @brief       :  This function Does the Setting up of the STA side
  *                           MIMO PS settings when the Request is sent
  */
static eHalStatus halStaSetMimoPs(tpAniSirGlobal pMac, tpSetMIMOPS pMIMO_PSparams)
{
    tSmacHostMesg_SetGlobalCfg  cfg, *pCfg;
    eHalStatus     status;

    status = halSetPowerSaveMode(pMac, pMIMO_PSparams->htMIMOPSState);
    if (status != eHAL_STATUS_SUCCESS) {
        /**  LOGP as Setting the PHY has Failed */
        HALLOGP( halLog(pMac, LOGP, FL(" halSetPowerSaveMode Failed!\n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOG1( halLog(pMac, LOG1, FL(" Updated the Rxp Chains and HAL globals\n")));

    if (pMIMO_PSparams->htMIMOPSState ==  eSIR_HT_MIMO_PS_DYNAMIC) {
        if (pMac->hal.CfgGlobalParams.fRxMimoPsDynamic == 1)
            return eHAL_STATUS_SUCCESS;
        else
            pMac->hal.CfgGlobalParams.fRxMimoPsDynamic = 1;
    } else {
        if (pMac->hal.CfgGlobalParams.fRxMimoPsDynamic == 0)
            return eHAL_STATUS_SUCCESS;
        else
            pMac->hal.CfgGlobalParams.fRxMimoPsDynamic = 0;
    }
    return  eHAL_STATUS_SUCCESS;
}



/**
* \brief Set the MIMO PS State/Mode for that particular STA
*
* \sa       halMsg_SetMimoPs
* \param    pMac              The global tpAniSirGlobal object
* \param    pMIMO_PSparams    The  tpSetMIMOPS parameter
*/

void
halMsg_SetMimoPs(tpAniSirGlobal pMac, tpSetMIMOPS pMIMO_PSparams)
{
    tpStaStruct pStaTable = (tpStaStruct) pMac->hal.halMac.staTable;
    tHalCfgSta              *pStaParam;
    eHalStatus                  status = eHAL_STATUS_SUCCESS;

    if (pMIMO_PSparams == NULL)
    {
        HALLOGE( halLog(pMac, LOGE, FL(" MimoPS Param is NULL \n")));
        return;
    }

    do
    {
        /** Obtain the Station context from the HAL Station Table*/
        pStaTable= &pStaTable[pMIMO_PSparams->staIdx];
        pStaParam = &pStaTable->staParam;

        if (!pStaTable->valid)
        {
            HALLOGE( halLog(pMac, LOGE, FL(" Station Idx is invalid for Setting MIMO PS  \n")));
            status = eHAL_STATUS_INVALID_STAIDX; break;
        }

        /** Set the new MIMO PS State in the STATION context */
        pStaParam->mimoPwrSaveMode = pMIMO_PSparams->htMIMOPSState;
        HALLOG1( halLog( pMac, LOG1, FL("Settin MIMO PS  state for the station idx : %d is successfull \n"), pMIMO_PSparams->staIdx));

        /** Update the STA Configurations in the case of updation of the self idx */
        switch ( halGetBssSystemRole(pMac, pStaParam->bssIdx ))
        { case eSYSTEM_STA_ROLE:
        {
                status = halStaSetMimoPs(pMac, pMIMO_PSparams);
                if (status != eHAL_STATUS_SUCCESS)
                    return;
            }
            break;

            case eSYSTEM_AP_ROLE:
            { /** In AP side update the RA Module for not using MIMO for the PEER*/
                tHalRaModeCfg         RaMode;
                tRateAdaptMode       pRateAdaptMode;

                if((status = halMacRaStaModeGet(pMac, pMIMO_PSparams->staIdx, &RaMode, &pRateAdaptMode)) != eSIR_SUCCESS)
                {
                    HALLOGE( halLog(pMac, LOGE, FL(" Getting Rate Station Mode failed \n")));
                    break;
                }

            if (pMIMO_PSparams->htMIMOPSState == eSIR_HT_MIMO_PS_STATIC)
                RaMode.mimoEnable = 0;
            else /** If the STA is not in MIMO PS Static Mode and HT Enabled it should be able to accept MIMO too */
                    RaMode.mimoEnable = (tANI_U8)pStaTable->htEnabled;

                if ((status = halMacRaStaCapModeUpdate(pMac,  pMIMO_PSparams->staIdx, &RaMode)) != eSIR_SUCCESS)
                {
                    HALLOGE( halLog(pMac, LOGE, FL(" Setting Rate for Station %d failed \n"), pMIMO_PSparams->staIdx));
                    break;
                }
            }
            }
            break;

            default:
            break;
        }
    }while(0);

    /** Send to LIM the Response with the status*/
    if (pMIMO_PSparams->fsendRsp == true)
    {
        pMIMO_PSparams->status = status;
        halMsg_GenerateRsp( pMac, SIR_HAL_SET_MIMOPS_RSP, 0, (void *) pMIMO_PSparams, 0);
    }
    return;
}


#endif

/**
 * \brief   Delete a BA session for a specific TID in a specific direction. Deletion
 *          can occur due to BA timeout (traffic inactivity), Error interrupt from
 *          the HW
 *
 * \sa                  halMsg_PostBADeleteInd
 *
 * \param pMac          The global tpAniSirGlobal object
 * \param staIdx        Station Index
 * \param baTID         TID for which the BA has to be delete
 * \param baDirection   RX or TX direction (eBA_RECEPIENT or EBA_INITIATOR)
 * \param reasonCode    Reason for deletion
 *
 * \return eHAL_STATUS_SUCCESS, eHAL_STATUS_FAILURE
 */
eHalStatus halMsg_PostBADeleteInd( tpAniSirGlobal  pMac,
    tANI_U16 staIdx,
    tANI_U8 baTID,
    tANI_U8 baDirection,
    tANI_U32 reasonCode)
{
    tpStaStruct pSta = NULL;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpBADeleteParams pBADeleteParams;

    // Sanity check - Validate STA
    status = halTable_ValidateStaIndex( pMac, (tANI_U8) staIdx );
    if( eHAL_STATUS_SUCCESS != status ) {
        return status;
    }

    pSta = &((tpStaStruct) pMac->hal.halMac.staTable)[staIdx];
    // For multi BSS case copy the sta bssid in the response.

    // Allocate for SIR_LIM_DEL_BA_IND
    status = palAllocateMemory( pMac->hHdd, (void **) &pBADeleteParams,
            sizeof( tBADeleteParams ));

    if( eHAL_STATUS_SUCCESS != status ) {
        HALLOGE(halLog( pMac, LOGE,
                    FL("palAllocateMemory failed with error code %d\n"),
                    status ));
        return status;
    } else {
        sirCopyMacAddr(pBADeleteParams->bssId, pSta->bssId);
        palZeroMemory( pMac->hHdd, (void *) pBADeleteParams,
                sizeof( tBADeleteParams ));
        // Copy SIR_LIM_DEL_BA_IND parameters
        pBADeleteParams->staIdx = staIdx;
        palCopyMemory( pMac->hHdd,
                (void *) pBADeleteParams->peerMacAddr,
                (void *) pSta->staAddr,
                sizeof( tSirMacAddr ));
        pBADeleteParams->baTID = baTID;
        pBADeleteParams->baDirection = baDirection;
        pBADeleteParams->reasonCode  = reasonCode;
    }
    // Send message to PE
    halMsg_GenerateRsp( pMac, SIR_LIM_DEL_BA_IND, 0, (void *) pBADeleteParams, 0);

    return status;
}

/** -------------------------------------------------------------
\fn halSetBcnRateIdx
\brief updates HAL cache for smca cfg global
\param   tpAniSirGlobal pMac
\param       tTpeRateIdx rateIndex
\return none.
  -------------------------------------------------------------*/
void halSetBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex)
{
    if(rateIndex != TPE_RT_IDX_INVALID)
        pMac->hal.halMac.BeaconRateIndex = rateIndex;
    else
        HALLOGE( halLog( pMac, LOGE, FL("Illegal TPE rate index %d\n"), rateIndex ));
}

/** -------------------------------------------------------------
\fn halSetNonBcnRateIdx
\brief updates HAL cache for smca cfg global
\param   tpAniSirGlobal pMac
\param       tTpeRateIdx rateIndex
\return none.
  -------------------------------------------------------------*/

void halSetNonBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex)
{
    if(rateIndex != TPE_RT_IDX_INVALID)
        pMac->hal.halMac.NonBeaconRateIndex = rateIndex;
    else
        HALLOGE( halLog(  pMac, LOGE,
            FL("Illegal TPE rate index %d\n"),
            rateIndex ));
}

/** -------------------------------------------------------------
\fn halSetMulticastRateIdx
\brief updates HAL cache for smca cfg global
\param   tpAniSirGlobal pMac
\param       tTpeRateIdx rateIndex
\return none.
  -------------------------------------------------------------*/

void halSetMulticastRateIdx(tpAniSirGlobal pMac, tTpeRateIdx rateIndex)
{
    if(rateIndex != TPE_RT_IDX_INVALID)
        pMac->hal.halMac.MulticastRateIndex = (tANI_U16) rateIndex;
    else{
        HALLOGE(halLog( pMac, LOGE,
            FL("Illegal TPE rate index %d\n"), rateIndex ));
    }
}

/** -------------------------------------------------------------
\fn halGetDefaultBcnRateIdx
  -------------------------------------------------------------*/
void halGetBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx *pRateIndex)
{
    *pRateIndex = (tTpeRateIdx)pMac->hal.halMac.BeaconRateIndex;
}

/** -------------------------------------------------------------
\fn halGetDefaultNonBcnRateIdx
  -------------------------------------------------------------*/

void halGetNonBcnRateIdx(tpAniSirGlobal pMac, tTpeRateIdx *pRateIndex)
{
    *pRateIndex = (tTpeRateIdx)pMac->hal.halMac.NonBeaconRateIndex;
}

/** -------------------------------------------------------------
\fn halGetDefaultMulticastRateIdx
  -------------------------------------------------------------*/

void halGetDefaultMulticastRateIdx(tpAniSirGlobal pMac, tTpeRateIdx *pRateIndex)
{
    *pRateIndex = (tTpeRateIdx)pMac->hal.halMac.MulticastRateIndex;
}

/** -------------------------------------------------------------
\fn      halMsg_setLinkState
\brief   HAL shall set the corresponding RXP filter mode depending
\        on the link state mode.
\param   tpAniSirGlobal pMac
\param   tSirLinkState  state
\return  none.
  -------------------------------------------------------------*/
#if 0
void halMsg_setLinkState(tpAniSirGlobal pMac, tpLinkStateParams pStateParam)
{
    tRxpMode    rxpFilter;
    tANI_U32    bSendFwMesg = 0;
    tSirLinkState state = pStateParam->state;

    switch(state)
    {
        case eSIR_LINK_IDLE_STATE:
            halATH_handleScanStop(pMac);
            rxpFilter = eRXP_IDLE_MODE;
            bSendFwMesg = 1;
            break;

        case eSIR_LINK_LEARN_STATE:
            halATH_handleScanStart(pMac);
            rxpFilter = eRXP_LEARN_MODE;
            break;

        case eSIR_LINK_SCAN_STATE:
            halATH_handleScanStart(pMac);
            rxpFilter = eRXP_SCAN_MODE;
            break;

        case eSIR_LINK_FINISH_SCAN_STATE:
            halATH_handleScanStop(pMac);
            rxpFilter = halRxp_getRxpMode(pMac);
            HALLOG1( halLog(pMac, LOG1, FL("Restoring rxpMode to %d \n"), rxpFilter));
            if (rxpFilter == eRXP_POST_ASSOC_MODE)
            {
                state = eSIR_LINK_POSTASSOC_STATE;
            }
            break;

        case eSIR_LINK_PREASSOC_STATE:
            rxpFilter = eRXP_PRE_ASSOC_MODE;
            bSendFwMesg = 1;

            break;

        case eSIR_LINK_POSTASSOC_STATE:
            rxpFilter = eRXP_POST_ASSOC_MODE;
            break;

        case eSIR_LINK_AP_STATE:
            rxpFilter = eRXP_AP_MODE;
            break;

        case eSIR_LINK_IBSS_STATE:
            rxpFilter = eRXP_IBSS_MODE;
            break;

        case eSIR_LINK_BTAMP_PREASSOC_STATE:
            rxpFilter = eRXP_BTAMP_PREASSOC_STATE;
            break;

        case eSIR_LINK_BTAMP_POSTASSOC_STATE:
            rxpFilter = eRXP_BTAMP_POSTASSOC_STATE;
            break;

        case eSIR_LINK_BTAMP_AP_STATE:
            rxpFilter = eRXP_BTAMP_AP_STATE;
            break;

        case eSIR_LINK_BTAMP_STA_STATE:
            rxpFilter = eRXP_BTAMP_STA_STATE;
            break;

        default:
            HALLOGE( halLog(pMac, LOGE, FL("Invalid state %d specified \n"), state));
            return;
    }

    HALLOG1( halLog(pMac, LOG1, FL("Link State = %d \n"), state));
    HALLOG1( halLog(pMac, LOG1, FL("Rxp Mode =  %d \n"), rxpFilter));
    halRxp_setRxpFilterMode(pMac, rxpFilter, pStateParam->bssid);
    halATH_setLinkState(pMac, state);

    if(bSendFwMesg){
        halFW_SendConnectionStatusMesg(pMac, state);
    }

    // Free the memory alloacted by the PE for this indication message
    palFreeMemory(pMac->hHdd, (void*)pStateParam);

    return;
}
#endif

// Set the BSS specific link state, this is guided by PE here.
static void halMsg_storeBssLinkState(tpAniSirGlobal pMac, tSirLinkState state,
        tANI_U8 bssIdx)
{
    tpBssStruct bssTable = (tpBssStruct) pMac->hal.halMac.bssTable;
    HALLOGE(halLog(pMac, LOGE, FL("Store rxp Link State to %x \n"),
            state));

    assert(bssIdx < pMac->hal.halMac.maxBssId);
    bssTable[bssIdx].bssLinkState=state;

    return;
}

// Set the BSS specific link state.
void halMsg_handleBssLinkState(tpAniSirGlobal pMac, tSirLinkState state,
        tANI_U8 *bssid, tANI_U8 bssIdx)
{
    tRxpMode    rxpMode;
    tANI_U8     bSendFwMesg = 0;

    switch(state)
    {
        case eSIR_LINK_IDLE_STATE:  // comes with a valid bssidx from PE>
            halATH_handleScanStop(pMac);
            rxpMode = eRXP_IDLE_MODE;
            break;

        case eSIR_LINK_SCAN_STATE:
            rxpMode = eRXP_SCAN_MODE;
            break;

        case eSIR_LINK_PREASSOC_STATE:
            // We get this in the case of a JOIN REQ
            // from PE. We will not know this bssId to start with.
            rxpMode = eRXP_PRE_ASSOC_MODE;

            //We get this only for Infra connection and hence inform BTC
            //so that it can protect the Infra connection setup. Note: We are
            //not protecting BT-AMP association as it is ok for BT-AMP assoc
            //to fail if there is a parallel BT traffic contending for the antenna.
            bSendFwMesg = 1;
            break;

        case eSIR_LINK_POSTASSOC_STATE:
            rxpMode = eRXP_POST_ASSOC_MODE;
            break;

        case eSIR_LINK_AP_STATE:
            rxpMode = eRXP_AP_MODE;
            break;

        case eSIR_LINK_IBSS_STATE:
            rxpMode = eRXP_IBSS_MODE;
            break;

        case eSIR_LINK_BTAMP_PREASSOC_STATE:
            rxpMode = eRXP_BTAMP_PREASSOC_MODE;
            break;

        case eSIR_LINK_BTAMP_POSTASSOC_STATE:
            rxpMode = eRXP_BTAMP_POSTASSOC_MODE;
            break;

        case eSIR_LINK_BTAMP_AP_STATE:
            rxpMode = eRXP_BTAMP_AP_MODE;
            break;

        default:
            HALLOGE(halLog(pMac, LOGE, FL("Invalid state %d specified \n"),
                        state));
            return;
    }

    HALLOG1(halLog(pMac, LOG1, FL("Link State = %d \n"), state));
    HALLOG1(halLog(pMac, LOG1, FL("BSS Rxp Mode =  %d \n"), rxpMode));
    // Update the filter to the latest for this Bssid and the
    // other BSS rxp modes + the global system wide setting
    halRxp_setBssRxpFilterMode(pMac, rxpMode, bssid, bssIdx);
    if (bssIdx < pMac->hal.halMac.maxBssId)
    {
        // Valid bssIdx so, store the state.
        halMsg_storeBssLinkState(pMac, state, bssIdx);
    }

    if(bSendFwMesg) {
        halFW_SendConnectionStatusMesg(pMac, state);
    }

    return;
}

/* ----- Function to process the SIR_HAL_SET_LINL_STATE message -- */
void halMsg_ProcessSetLinkState(tpAniSirGlobal pMac,
    tpLinkStateParams pLinkStateParams)
{
    tANI_U8      bssIdx = 0;
    eHalStatus   status = eHAL_STATUS_SUCCESS;

    assert(pLinkStateParams != NULL);

    // Make sure the BSSID exists.
    if ((status = halTable_FindBssidByAddr(pMac,
        pLinkStateParams->bssid, &bssIdx)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("halTable_FindBssidByAddr() fail\n")));
        bssIdx = HAL_INVALID_KEYID_INDEX;
    }

    halMsg_handleBssLinkState(pMac, pLinkStateParams->state, pLinkStateParams->bssid, bssIdx);

    palFreeMemory( pMac->hHdd, pLinkStateParams );
}

/** ---------------------------------------------------------
\fn      halMsg_setTxPower
\brief   HAL converts txPower from milliwatts to power index
\        and then sends down a message to TPE
\param   tpAniSirGlobal      pMac
\param   tpSirSetTxPowerReq  request message
\return  eHalStatus
  -----------------------------------------------------------*/
eHalStatus halMsg_setTxPower(tpAniSirGlobal pMac, tpSirSetTxPowerReq pSetTxPowerReq)
{
    tPwrTemplateIndex  pwrTemplateIndex;
    tTpeRateIdx       index;
    t_mW               mW;
    eHalStatus         status = eHAL_STATUS_SUCCESS;

    // convert power from milliwatts to power template index
    mW = (t_mW) pSetTxPowerReq->txPower;
    status = halPhySetTxMilliWatts(pMac, mW, &pwrTemplateIndex);

    // send SIR_HAL_SET_TX_POWER_RSP to LIM
    halMsg_sendSetTxPowerRsp( pMac, (tANI_U32)status );

    if (status == eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Converting txPower from: %d mW  -> index=%d \n"), mW, pwrTemplateIndex));
    }
    else
    {
        HALLOGW( halLog(pMac, LOGW, FL("Failed to convert txPower %d mW \n"), mW));
        return status;
    }

    /* Update the Tx power for the in the Hal rate table */
    for(index = (tTpeRateIdx)MIN_LIBRA_RATE_NUM; index < (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM; index++) {
        halRate_UpdateRateTxPower(pMac, index, pwrTemplateIndex);
        // Update the cntrl/rsp rate tx power locally and in the TPE
        halRate_UpdateCtrlRspTxPower(pMac, index, pwrTemplateIndex, TRUE);
    }
    halRate_TxPwrIndexToFW(pMac, MIN_LIBRA_RATE_NUM, MAX_LIBRA_TX_RATE_NUM);
    halMacRaUpdateParamReq(pMac, RA_UPDATE_TXPWR_INFO, MIN_LIBRA_RATE_NUM << 24 | MAX_LIBRA_TX_RATE_NUM << 16 | pwrTemplateIndex << 8);

    palFreeMemory(pMac->hHdd, pSetTxPowerReq);
    return status;
}


/* ---------------------------------------------------------
\fn      halMsg_getTxPower
\brief   HAL retrieves the station id, look up the station's
\        primary rate index, gets the corresponding txPower
\        index, converts txPower to milliwatts, and
\        composes a message to send it up to HDD.
\param   tpAniSirGlobal      pMac
\param   tpSirGetTxPowerReq  request message
\return  none
  -----------------------------------------------------------*/
void halMsg_getTxPower(tpAniSirGlobal pMac, tpSirGetTxPowerReq pGetTxPowerReq)
{
    tPwrTemplateIndex  pwrTemplateIndex;
    t_mW               mWatts = 0;
    tANI_U16           staid;
    tHalMacRate        curTxRateIdx;
    tTpeRateIdx        tpeRateIndex;
    eHalStatus         status = eHAL_STATUS_SUCCESS;

    staid = pGetTxPowerReq->staid;

    // Check if station id is valid and associated.
    status = halTable_ValidateStaIndex(pMac, (tANI_U8) staid);
    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("Invalid sta index %d\n"), staid));
        goto generate_response;
    }

    // get station's primary rate index
    curTxRateIdx = halGetCurrentRate(pMac, staid);
    tpeRateIndex = halRate_halRate2TpeRate(curTxRateIdx);

    HALLOGW( halLog(pMac, LOGW, FL("staid %d:  curTxRateIndex %d, tpeRateIndex %d \n"), staid, curTxRateIdx, tpeRateIndex));

    // get the powerIndex corresponding to this rateindex
    if( halRate_getPowerIndex(pMac, tpeRateIndex, &pwrTemplateIndex) != eHAL_STATUS_SUCCESS )
    {
        HALLOGW( halLog(pMac, LOGW, FL("halRate_getPowerIndex() failed \n")));
        status = eHAL_STATUS_FAILURE;
        goto generate_response;
    }

    status = halPhyGetTxMilliWatts(pMac, pwrTemplateIndex, &mWatts);
    if (status == eHAL_STATUS_SUCCESS){
        HALLOGW( halLog(pMac, LOGW, FL("Converting txPowerTemplateIndex %d  -> %d mW \n"), pwrTemplateIndex, mWatts));
    }else{
        HALLOGW( halLog(pMac, LOGW, FL("Failed to convert txPower index %d \n"), pwrTemplateIndex));
    }
generate_response:
    // send SIR_HAL_GET_TX_POWER_RSP to LIM
    halMsg_sendGetTxPowerRsp( pMac, (tANI_U32)mWatts, status );

    palFreeMemory(pMac->hHdd, pGetTxPowerReq);
    return;
}

/* ---------------------------------------------------------
\fn      halMsg_sendSetTxPowerRsp
\brief   HAL composes a SIR_HAL_SET_TX_POWER_RSP message
\        with the status value, and sends it to LIM.
\param   tpAniSirGlobal   pMac
\param   tANI_U32         status
\return  none
  -----------------------------------------------------------*/
void halMsg_sendSetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 status)
{
    eHalStatus result = eHAL_STATUS_SUCCESS;
    tpSirSetTxPowerRsp pRsp;

    result = palAllocateMemory( pMac->hHdd, (void**) &pRsp, sizeof(tpSirSetTxPowerRsp) );
    if (eHAL_STATUS_SUCCESS != result)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory for tSirSetTxPowerRsp \n")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp) );
    pRsp->length = sizeof(tSirSetTxPowerRsp);
    pRsp->status = status;

    halMsg_GenerateRsp( pMac, SIR_HAL_SET_TX_POWER_RSP, (tANI_U16) 0, pRsp, 0 );

    return;
}

/* ---------------------------------------------------------
\fn      halMsg_sendGetTxPowerRsp
\brief   HAL composes a SIR_HAL_GET_TX_POWER_RSP message
\        with the txPower and status value, and sends it
\        to LIM.
\param   tpAniSirGlobal      pMac
\param   tANI_U32            txPower (units of milliwatts)
\param   tANI_U32            status
\return  none
  -----------------------------------------------------------*/
void halMsg_sendGetTxPowerRsp(tpAniSirGlobal pMac, tANI_U32 power, eHalStatus status)
{
    eHalStatus rc = eHAL_STATUS_SUCCESS;
    tpSirGetTxPowerRsp   pRsp;

    rc = palAllocateMemory( pMac->hHdd, (void**) &pRsp, sizeof(tSirGetTxPowerRsp) );
    if (eHAL_STATUS_SUCCESS != rc)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory for tSirGetTxPowerRsp \n")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp) );
    pRsp->length = sizeof(tSirGetTxPowerRsp);
    pRsp->power = power;
    pRsp->status = (tANI_U32)status;

    halMsg_GenerateRsp( pMac, SIR_HAL_GET_TX_POWER_RSP, (tANI_U16) 0, pRsp, 0 );

    return;
}

#ifdef WLAN_FEATURE_VOWIFI
eHalStatus halMsg_setTxPowerLimit(tpAniSirGlobal pMac, tpMaxTxPowerParams pSetMaxTxPwrReq)
{
    tANI_U8 bssIdx;
    tTpeRateIdx           rateIndex;
    eHalPhyRates          phyRateIndex;
    tPwrTemplateIndex     pwrIndex;
    bssRaParam            bssRaInfo;
    eHalStatus            status;
    t2Decimal             pwrDbm;

    if (eHAL_STATUS_SUCCESS != halTable_FindBssidByAddr(pMac, pSetMaxTxPwrReq->bssId, &bssIdx))
    {
        HALLOGE(halLog(pMac, LOGE, FL("Not a valid BSS IDX")));
        goto generate_response;
    }
    else
    {
        tANI_U8      rfBand, index, staIdx;
        tPwrTemplateIndex maxPwrIndex1, maxPwrIndex2;
        
        /* The power constraint from the higher layers are set by configuring the maxPwrIndex 
           per BSS which is shared between host and firmware. RA algorithm, when picking up the 
           power for a rate, would apply this constraint and selects the minimum of power in rate to 
           power index table and the maxPwrIndex constraint. Hence the rate/power table need not be updated 
           in the host as this table remains constant in the firmware memory irrespective of the power constraints.*/
        rfBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);
        halTable_GetTxPowerLimitIndex(pMac, rfBand, &maxPwrIndex1);
        halTable_GetBssRaConfig(pMac, rfBand, &bssRaInfo, bssIdx);
        
        //Should convert the max Tx power to power index and save it in RA BSS info
        /* Now we should get the power index by taking maxTxPower(power constraint) into consideration
         * and set this power index as the maxPwrIndex in RA BSS Info. No frames should be transmitted 
         * with the power value greater than this */
        halPhyGetMaxTxPowerIndex(pMac, pSetMaxTxPwrReq->power, &pwrIndex);
        bssRaInfo.bit.maxPwrIndex = pwrIndex;
        halTable_SaveBssConfig(pMac, rfBand, bssRaInfo, bssIdx);
        HALLOGE(halLog(pMac, LOGE, FL("Set TX Pwr Limit Req: BSSIDX = %d, Power(dBm) = %d, Pwr Index = %d"),
                                    bssIdx, pSetMaxTxPwrReq->power, bssRaInfo.bit.maxPwrIndex));

        halTable_GetTxPowerLimitIndex(pMac, rfBand, &maxPwrIndex2);

        /* Update the TPE sram table only if the max power before the setting and after 
           the setting are different */
        if (maxPwrIndex2 != maxPwrIndex1)
        {
            /* The control rsp power is common across all the BSS. Hence always maintain highest power for Cntrl/Rsp
               frames */
            for(index = (tTpeRateIdx)MIN_LIBRA_RATE_NUM; index < (tTpeRateIdx)MAX_LIBRA_TX_RATE_NUM; index++) {
                // Update the cntrl/rsp rate tx power locally and in the TPE
                halRate_UpdateCtrlRspTxPower(pMac, index, maxPwrIndex2, TRUE);
            }
        }

         /* Need  to return the power for management frames in the response */
        halGetNonBcnRateIdx(pMac,(tTpeRateIdx *) &rateIndex);
        phyRateIndex = halRate_MacRateIdxtoPhyRateIdx(pMac, rateIndex);
        if (HAL_PHY_RATE_INVALID == phyRateIndex)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Invalid Phy rate index for management frames, setting default")));
            phyRateIndex = HAL_PHY_RATE_11B_LONG_1_MBPS;
        }
        halPhyGetPowerForRate(pMac, phyRateIndex, POWER_MODE_HIGH_POWER, pSetMaxTxPwrReq->power, &pwrIndex);
        pwrIndex = (pwrIndex < bssRaInfo.bit.maxPwrIndex) ? pwrIndex : bssRaInfo.bit.maxPwrIndex;
        
        /* The power returned to the PE is used to fill the TX power in WFA TPC report in 802.11k Link measurement frames 
                and probe request frames */
        status = halPhyGetPwrFromRate2PwrTable(pMac, phyRateIndex, &pwrDbm);
        if (eHAL_STATUS_SUCCESS != status)
        {
            HALLOGP(halLog(pMac, LOGP, FL("Get power from rate2pwr table for rateindex %d failed with status %d\n"), phyRateIndex, status));
        }
        pwrDbm /= 100;      //The halPhyGetPwrFromRate2PwrTable provides pwrDbm as 1900 for 19dBm. So, divide by 100
        pSetMaxTxPwrReq->power = VOS_MIN(pSetMaxTxPwrReq->power, pwrDbm);
        HALLOGE(halLog(pMac, LOGE, FL("Set TX Pwr Limit Rsp: Mgmt Rate Index = %d, Power = %d"),
                                            rateIndex, pSetMaxTxPwrReq->power));
        //Need to update the power for the BD rates in the TPE station descriptor
        halTable_FindStaidByAddr(pMac , pSetMaxTxPwrReq->selfStaMacAddr, &staIdx);
        halTpe_UpdateDescPowerParams(pMac, staIdx, TPE_STA_BD_RATE, pwrIndex);
                
     }
generate_response:
    halMsg_GenerateRsp( pMac, SIR_HAL_SET_MAX_TX_POWER_RSP, (tANI_U16) 0, pSetMaxTxPwrReq, 0 );

    return eHAL_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_VOWIFI */

/* ---------------------------------------------------------
\fn      halMsg_sendGetNoiseRsp
\brief   HAL composes a SIR_HAL_GET_NOISE_RSP message,
\        and sends it to LIM.
\param   tpAniSirGlobal      pMac
\return  none
  -----------------------------------------------------------*/
void halMsg_sendGetNoiseRsp(tpAniSirGlobal pMac)
{
    tpSirGetNoiseRsp  pRsp=NULL;

#ifdef FIXME_GEN5
    tANI_U32          value;
    tANI_U32          noise[3];
    eHalStatus        status = eHAL_STATUS_SUCCESS;
    if (halReadRegister(pMac, AGC_RSSI0_REG, &value) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("read AGC_RSSI0_REG(0x%x) fail \n"), AGC_RSSI0_REG));
        return;
    }
    noise[0] = value & AGC_RSSI0_RSSI_MASK;
    noise[0] = (tANI_U32) HAL_CONVERT_HW_RSSI_UNIT_2_DB(noise[0], pMac->hal.halMac.topGainDb);

    if (halReadRegister(pMac, AGC_RSSI1_REG, &value) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("read AGC_RSSI1_REG(0x%x) fail \n"), AGC_RSSI1_REG));
        return;
    }
    noise[1] = value & AGC_RSSI1_RSSI_MASK;
    noise[1] = (tANI_U32) HAL_CONVERT_HW_RSSI_UNIT_2_DB(noise[1], pMac->hal.halMac.topGainDb);

    if (halReadRegister(pMac, AGC_RSSI2_REG, &value) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog(pMac, LOGW, FL("read AGC_RSSI2_REG(0x%x) fail \n"), AGC_RSSI2_REG));
        return;
    }
    noise[2] = value & AGC_RSSI2_RSSI_MASK;
    noise[2] = (tANI_U32) HAL_CONVERT_HW_RSSI_UNIT_2_DB(noise[2], pMac->hal.halMac.topGainDb);

    status = palAllocateMemory( pMac->hHdd, (void**) &pRsp, sizeof(tSirGetNoiseRsp) );
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Unable to allocate memory for tSirGetNoiseRsp \n")));
        return;
    }

    palZeroMemory( pMac, pRsp, sizeof(*pRsp) );
    pRsp->length = sizeof(tSirGetNoiseRsp);

    pRsp->noise[0] = noise[0];
    pRsp->noise[1] = noise[1];
    pRsp->noise[2] = noise[2];
#endif

    halMsg_GenerateRsp( pMac, SIR_HAL_GET_NOISE_RSP, (tANI_U16) 0, pRsp, 0 );
    return;
}

//FIXME_GEN6 : this fumction shoulg go away once PMC stops sending message down to HAL.
eHalStatus halMsg_configPowerSave(tpAniSirGlobal pMac,
                  tpSirPowerSaveCfg pPowerSaveConfig)
{
    palFreeMemory(pMac->hHdd, pPowerSaveConfig);

    return eHAL_STATUS_SUCCESS;
}

/** ------------------------------------------------------
\brief   This timer keeps track of the delta traffic for
\        the window of 100 ms
\
\note    LED functionality is also implemented in the
\        same timer and the traffic LED algorithm is
\        implemented based on the transmitted and received
\        packets to and from DXE respectively. This controls
\        the traffic LED on/off and duration.
\
\param   pMac A pointer to the tAniSirGlobal structure
\return None
\ -------------------------------------------------------- */
void halMsg_HandleTrafficActivity(tpAniSirGlobal pMac)
{
    /** Store the current tx and rx count*/
    pMac->hal.halMac.macStats.prevTxCount = pMac->hal.halMac.macStats.txCount;
    pMac->hal.halMac.macStats.prevRxCount = pMac->hal.halMac.macStats.rxCount;

   /** Since LED also needs a 100 ms timer or < we piggyback to this timer.*/
#if defined(ANI_LED_ENABLE)
    if (pMac->hal.ledParam.config.bEnable == eANI_BOOLEAN_TRUE)
    {
    halLedHandler(pMac);
    }
#endif
}

#ifndef WLAN_SOFTAP_FEATURE
/**
* @function :   halMsg_BeaconPre
* @brief :        Handles the PreBeacon Indication from S/W Mac.
*
* @param:       pMac                 The global tpAniSirGlobal object
*                    pBeaconPreMesg  BeaconPre Message Data's
*/
void
halMsg_BeaconPre( tpAniSirGlobal pMac )
{

    tBeaconGenParams *pBeaconGenParams = NULL;
    tBmuStaQueueData staQueueData = { 0 };
    tANI_U32  totalSta = 0;
    tpBssStruct t = (tpBssStruct) pMac->hal.halMac.bssTable;
    tANI_U32 staIdx = 0;
	tANI_U8  bssIdx = 0;

   //Read BTQM to get beaconGenParams
   /* 
    * In HAL, during pre-beacon indication (halMsg_BeaconPre()), we are querying BTQM queues for all STA indices 
    * to see if there is any traffic for that STA index. I understand during pre-beacon,  the purpose is to get 
    * some parameters like StaIdx, BssIndex and to provide them to PE. Looks like this code suits more for SoftAP 
    * (may be Gen4/5) implementation where HAL also queries DTIM count for that STA index and takes some action. 
    * Now I got in to this issue while doing multisession with Infra STA associated along with BT-AMP AP trying 
    * to create a BSS. When I try this, HAL tries to query STA Idx where traffic is flowing in any of the BTQM 
    * queues and finally gives incorrect STA index and BSS Index because of which we point to incorrect BSSID.
    *
    * This code holds good for VOLANS
    */
#ifdef FIXME_VOLANS
        halBmu_GetStaWqStatus(pMac, &staQueueData);
#endif

    for (staIdx = 0; staIdx < pMac->hal.memMap.maxStations; staIdx++) {
        /** Get the BSSID for the STA */
        if (halTable_GetBssIndexForSta(pMac, &bssIdx, staIdx) != eHAL_STATUS_SUCCESS)
            return;

        if ( (eSYSTEM_STA_IN_IBSS_ROLE ==  t[bssIdx].bssSystemRole) ||
	      (eSYSTEM_BTAMP_STA_ROLE ==   t[bssIdx].bssSystemRole) ||
	      (eSYSTEM_BTAMP_AP_ROLE ==  t[bssIdx].bssSystemRole) ) {

	       staQueueData.bssIdx = bssIdx;
	       totalSta ++;
           break;
        }
    }

#ifdef FIXME_VOLANS
    totalSta = staQueueData.numOfSta + staQueueData.numOfStaWithoutData;
    if( totalSta > pMac->hal.halMac.maxSta)
    {
        HALLOGW( halLog( pMac, LOGW, FL("Station Count in BeaconPre Message invalid: %d:%d\n"),
                            staQueueData.numOfSta, staQueueData.numOfStaWithoutData));
        return;
    }
#endif /* FIXME_VOLANS */

    /** Allocate the Memory for Beacon Pre Message and for Stations in PoweSave*/
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd,
                                            (void **) &pBeaconGenParams, (sizeof(*pBeaconGenParams) +
                                            sizeof(tBeaconGenStaInfo) * totalSta)))
    {
        HALLOGP( halLog( pMac, LOGP,
                        FL( "Unable to PAL allocate memory during sending beaconPreMessage\n" )));
        return;
    }
    palZeroMemory( pMac->hHdd, pBeaconGenParams, sizeof(*pBeaconGenParams) + \
                                                              (sizeof(tBeaconGenStaInfo) * totalSta));

    palCopyMemory( pMac->hHdd, pBeaconGenParams->bssId, (void *) t[bssIdx].bssId, SIR_MAC_ADDR_LENGTH );
    pBeaconGenParams->bssIdx = bssIdx;

#ifdef FIXME_VOLANS
    if (eHAL_STATUS_SUCCESS != halFillBeaconGenParams(pMac,
                                                            &staQueueData, pBeaconGenParams, totalSta))
    {
        HALLOG1( halLog( pMac, LOG1, FL( "Fill BeaconGenParams failed \n" )));

        palFreeMemory(pMac->hHdd, pBeaconGenParams);
        return; /** Dont post back the message to PE*/
    }
#endif

    if (pMac->hal.pPECallBack == NULL) /** Is PE Callback is being Registered*/
    {
        HALLOG1( halLog( pMac, LOG1, FL( "PE Callback is not being Registered \n" )));

        palFreeMemory(pMac->hHdd, pBeaconGenParams);
        return;
    }

    (pMac->hal.pPECallBack)(pMac, SIR_LIM_BEACON_GEN_IND, (void *)pBeaconGenParams);
    return;
}

#endif

#if 0
/**
 *  @brief : This is the timer Message handler to monitor PHY.
 *  @param : pMac - Mac Global Handle.
 */
static void __halMsg_PhyMonitor(tpAniSirGlobal  pMac)
{
    tANI_U32  sentVal;
    tANI_U32  abortVal;
    tANI_U32  mpiTxSentDelta;
    tANI_U32  mpiTxAbortDelta;
    eHalStatus status;

    /** Read the MPI TX SENT counter Register.*/
    halReadRegister(pMac, QWLAN_MPI_TXP_MPI_START_REG, &sentVal);

    /** Read the MPI TX ABORT counter Register.*/
    halReadRegister(pMac, QWLAN_MPI_TXP_ABORT_REG, &abortVal);

    sentVal = sentVal & QWLAN_MPI_TXP_MPI_START_COUNT_MASK;
    abortVal = abortVal & QWLAN_MPI_TXP_ABORT_COUNT_MASK;

    /** Check for counter reset.*/
    if (sentVal < pMac->hal.halMac.mpiTxSent)
    {
        mpiTxSentDelta = (QWLAN_MPI_TXP_MPI_START_COUNT_MASK - pMac->hal.halMac.mpiTxSent) + sentVal;
    }
    else
    {
        mpiTxSentDelta = sentVal - pMac->hal.halMac.mpiTxSent;
    }

    if (abortVal < pMac->hal.halMac.mpiTxAbort)
    {
        mpiTxAbortDelta = (QWLAN_MPI_TXP_ABORT_COUNT_MASK - pMac->hal.halMac.mpiTxAbort) + abortVal;
    }
    else
{
        mpiTxAbortDelta = abortVal - pMac->hal.halMac.mpiTxAbort;
    }

    pMac->hal.halMac.mpiTxSent = sentVal;
    pMac->hal.halMac.mpiTxAbort = abortVal;

    /** Avoid false triggers ...*/
    if (mpiTxSentDelta <= HAL_PHY_MONITOR_WINDOW && mpiTxAbortDelta <= HAL_PHY_MONITOR_WINDOW)
        return;

    /** We consider at the time of reading the register the last frame would'nt have been aborted
     *      since its in progress; So window plus or minus one that of mpitxSentDelta needs to be
     *      considered.
     *  When PHY hangs we would also see mpiTxSentDelta == 0 and mpiTxAbortDelta varies.
     */
    if (((mpiTxAbortDelta >= (mpiTxSentDelta-HAL_PHY_MONITOR_WINDOW)) &&
           (mpiTxAbortDelta <= (mpiTxSentDelta+HAL_PHY_MONITOR_WINDOW)))  ||
           ((mpiTxSentDelta == 0) && (mpiTxAbortDelta > HAL_PHY_MONITOR_WINDOW)))
    {
        pMac->hal.halMac.phyHangThr++;
        if (pMac->hal.halMac.phyHangThr > HAL_PHY_MONITOR_TH)
        {
            HALLOGE( halLog(pMac, LOGE, FL("PHY Not Responding, Possibly HUNG!!!\n")));
            macSysResetReq(pMac, eSIR_PHY_HANG_EXCEPTION);
            return;
        }
    }
    else
    {
        pMac->hal.halMac.phyHangThr = 0;
    }
}
#endif

/** ------------------------------------------------------
\brief   This timer message handler monitors the Chip if
         its still alive.
\
\param   pMac A pointer to the tAniSirGlobal structure
\return None
\ -------------------------------------------------------- */
void halMsg_ChipMonitorTimeout(tpAniSirGlobal pMac)
{
    halFW_HeartBeatMonitor(pMac);
    //__halMsg_PhyMonitor(pMac);
}

#ifdef FIXME_GEN6
/** -------------------------------------------------------
\brief   This API allows user to disable periodic calibration
\
\note    The periodic calibration is done based on the status
\                 of this flag.
\
\param   pMac A pointer to the tAniSirGlobal structure
\
\return  None
\ --------------------------------------------------------- */
static void
__halSetCalibrationState(tpAniSirGlobal pMac, tANI_BOOLEAN fEnable)
{
        /**
         * fEnable == eANI_BOOLEAN_TRUE: Enable Calibration
         * fEnable == eANI_BOOLEAN_FALSE: Disable Calibration
         */

        if(fEnable)
    {
                pMac->hphy.phy.phyPeriodicCalEnable = pMac->hal.prevCalibrationState;
    }
        else
        {
                pMac->hal.prevCalibrationState = pMac->hphy.phy.phyPeriodicCalEnable;
                pMac->hphy.phy.phyPeriodicCalEnable = (eANI_BOOLEAN_FALSE);
        }

        return;
}
#endif

//FIXME_GEN6 : remove this once LIM stops sending the correspondig message.
/** -------------------------------------------------------
\brief   This API allows user to halt the frame transmission
        as and when needed.

         The SoftMAC can be told to halt all transmission
        or on per station basis or on per bssid based.

\param   pMac A pointer to the tAniSirGlobal structure
\param   pBuffer A pointer to the message buffer

\return  eHAL_STATUS_SUCCESS - If successful
            eHAL_STATUS_FAILURE - Otherwise
 --------------------------------------------------------- */
eHalStatus
halMsg_FrameTransmitControlInd(tpAniSirGlobal pMac, void *pBuffer)
{
    palFreeMemory(pMac->hHdd, pBuffer);
    return eHAL_STATUS_FAILURE;
}

void halMsg_TXCompleteInd(tpAniSirGlobal pMac, tANI_U32 txCompleteSuccess)
{
    if(TX_SUCCESS != tx_timer_deactivate(&pMac->hal.txCompTimer))
    {
        HALLOGP(halLog(pMac, LOGP, FL("could not deactivate txCompTimer\n")));
    }

    if(pMac->hal.pCBackFnTxComp)
    {
        pMac->hal.pCBackFnTxComp(pMac, txCompleteSuccess);
        pMac->hal.pCBackFnTxComp = NULL; //invalidating this pointer.
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("There is no request pending for TX complete and intr received\n")));
    }
}


