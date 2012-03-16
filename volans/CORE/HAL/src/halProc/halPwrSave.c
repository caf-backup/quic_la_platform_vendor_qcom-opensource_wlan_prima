/*
* Copyright (c) 2012 Qualcomm Atheros, Inc.
* All Rights Reserved.
* Qualcomm Atheros Confidential and Proprietary.
*/

/*
 * File:        halPwrSave.c
 * Description: This file contains all the Power save related functions
 *              required for initialization, Idle Mode power save,
 *              Beacon Mode power save, Unscheduled automatic power
 *              save delivery and power save configuration.
 *
 * Copyright (c) 2008 QUALCOMM Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *
 * History:
 *
 * When       Who         What/Where/Why
 * -------------------------------------------------------------------
 * 07/21/2008 lawrie      Created the functions for IMPS, BMPS & UAPSD
 *
 *
 */
#include "palTypes.h"
#include "palApi.h"
#include "halDebug.h"

#include "halPwrSave.h"
#include "halFwApi.h"
#include "halFw.h"

#include "halRegBckup.h"
#include "halRxp.h"
#include "halAdu.h"
#include "halRateTable.h"
#include "halTLApi.h"
#include "cfgApi.h"
#include "halUtils.h"
#include "halRateAdaptation.h"

#include "wlan_qct_bal.h"
#include "rfApi.h"
#include "limApi.h"


#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "wlan_ps_wow_diag.h"
#endif

#ifdef WLAN_DBG_GPIO
#include <mach/gpio.h>
#endif

#define QWLAN_MAX_LISTEN_INTERVAL_TU 600

/* Static Functions */
static void halPS_FwRspTimeoutFunc(void* pData);
//static void halPS_HandleFwRspTimeout(tpAniSirGlobal pMac, tANI_U16 rspType);
eHalStatus halPS_RegBckupDirectRegisters(tpAniSirGlobal pMac);
eHalStatus halPS_RegBckupIndirectRegisters(tpAniSirGlobal pMac);
void halPS_StartMonitoringRegAccess(tpAniSirGlobal pMac);
void halPS_StopMonitoringRegAccess(tpAniSirGlobal pMac);

/*
 * DESCRIPTION:
 *      Power Save Initialization funtion for initializing registerlist,
 *      timers, events. mutexes.
 *
 * PARAMETERS:
 *      hHal:   (pMac)Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_Init(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tANI_U32 regValue;
#ifdef VOLANS_PHY_TX_OPT_ENABLED
	tHalRegBckup *pRegBckup = &pMac->hal.RegBckupParam;
    tANI_U32 memAddr;
#endif /* VOLANS_PHY_TX_OPT_ENABLED */

    // Zero out the memory
    palZeroMemory(pMac->hHdd, &pMac->hal.PsParam, sizeof(pMac->hal.PsParam));

    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_ACTIVE_STATE;
    pHalPwrSave->pwrSaveState.p.mutexProtect = FALSE;

    // Get the start address offset of the register reconfig table
    pHalPwrSave->regListStartAddr = pMac->hal.RegBckupParam.regListStartAddr;

    // Populate the ADU memory with the register addresses list
    status = halPS_RegBckupDirectRegisters(pMac);
     if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("Load the register list failed!!!\n")));
        return status;
     }

    // Initialize the start address for the loading the indirectly
    // accessed registers
    pHalPwrSave->indirectRegListStartAddr = pHalPwrSave->regListStartAddr +
            pHalPwrSave->regListSize;

#ifdef VOLANS_PHY_TX_OPT_ENABLED
		// Backup the PHY/RF TX registers which should be initialized
		// after coming out of power save and not as a part of ADU reinit
		/** Zero out the ADU Register Reconfiguration table memory */
		pRegBckup->phyRFTxRegListStartAddr = pMac->hal.memMap.aduPhyTxRegRecfgTbl_offset;
		if (halZeroDeviceMemory(pMac,
				pRegBckup->phyRFTxRegListStartAddr,
				pMac->hal.memMap.aduPhyTxRegRecfgTbl_size) != eHAL_STATUS_SUCCESS) {
			return eHAL_STATUS_FAILURE;
		}

		memAddr = pRegBckup->phyRFTxRegListStartAddr;
		status = halRegBckup_PhyRFTxRegisters(pMac, &memAddr);
		if(status != eHAL_STATUS_SUCCESS) {
			return status;
		}

		// Write the magic value in the next device memory location
		status = halRegBckup_WriteTableEndCmd(pMac, memAddr);
#endif /* VOLANS_PHY_TX_OPT_ENABLED */

    // Initialize the event
    vosStatus = vos_event_init(&pMac->hal.PsParam.fwRspEvent);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Event init failed - status = %d\n"),
                vosStatus));
        return eHAL_STATUS_FAILURE;
    }

    //Initialize the timer
    vosStatus = vos_timer_init(&pHalPwrSave->fwRspTimer, VOS_TIMER_TYPE_WAKE_APPS,
           halPS_FwRspTimeoutFunc, (void*)pMac);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer init failed - status = %d\n"),
                vosStatus));
        return eHAL_STATUS_FAILURE;
    }

    // Initialize the FW respone timeout value
    pHalPwrSave->fwRspTimeout = HAL_FW_RSP_TIMEOUT;

    // Initialize the call back function pointer and the data
    pHalPwrSave->psCbFunc = NULL;
    pHalPwrSave->psCbData = NULL;

    // Initialize the HW mutexes
    halMcu_ResetMutexCount(pMac, QWLAN_MCU_MUTEX_HOSTFW_SYNC_INDEX, QWLAN_HOSTFW_SYNC_MUTEX_MAX_COUNT);
    pHalPwrSave->mutexCount     = 0;
    pHalPwrSave->mutexIntrCount = 0;
#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
    pHalPwrSave->mutexTxRxCount = 0;
#endif
    pHalPwrSave->enableMutexDebug = 0;

#ifdef FEATURE_WLAN_VOLANS_1_0_PWRSAVE_WORKAROUND
    /* FIXME: Volans 1.0 Power-save workaround. Needs to be removed
     * as soon as fix is available in the hardware
     */
    halMcu_ResetMutexCount(pMac, QWLAN_MCU_MUTEX_HOSTFW_TX_SYNC_INDEX, QWLAN_HOSTFW_TX_SYNC_MUTEX_MAX_COUNT);
#endif

    /* This bit in MCU_HOST_INT_EN_REG is not mapped to interrupt
     * lines. Hence this bit is only used to enable the SIF unfreeze
     * to happen when Host accesss comes in during power save mode.
     */
    halReadRegister(pMac, QWLAN_MCU_MCU_HOST_INT_EN_REG, &regValue);
    regValue |= QWLAN_MCU_MCU_HOST_INT_EN_PMU_MCU_EXIT_SIF_FREEZE_EN_MASK;
    halWriteRegister(pMac, QWLAN_MCU_MCU_HOST_INT_EN_REG, regValue);

     /* Enable ADU ReInit method of booting. Currently it is done
      * everytime device enters and exits power-save, which is not
      * necessary
      */
     halPmu_AduReinitEnableDisable(pMac, TRUE);

    status = eHAL_STATUS_SUCCESS;

    return status;
}


/*
 * DESCRIPTION:
 *      Power Save exit function for destroying timers, events
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_Exit(tHalHandle hHal, void *arg)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;

    // Destroy the FW response event
    vosStatus = vos_event_destroy(&pHalPwrSave->fwRspEvent);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Event destroy failed - status = %d\n"),
                vosStatus));
        return eHAL_STATUS_FAILURE;
    }

    vos_timer_stop(&pHalPwrSave->fwRspTimer);
    // Destory the FW response timer
    vosStatus = vos_timer_destroy(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGP( halLog( pMac, LOGP, FL("VOS Timer destroy failed - status = %d\n"),
                vosStatus));
        return eHAL_STATUS_FAILURE;
    }

    return eHAL_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Callback function called on timer expiry of FW response message
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      VOID
 */
static void halPS_FwRspTimeoutFunc(void* pData)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pData;
    HALLOGP( tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam);

    (void)pMac;

    // Should do a LOGP here, if the response timeout occured
    HALLOGP( halLog(pMac, LOGP, FL("CRITICAL: FW response timedout for msg rsp type %d (%s)!!!\n"),
            pHalPwrSave->rspType, halUtil_getMsgString(pHalPwrSave->rspType)));

    macSysResetReq(pMac, eSIR_FW_EXCEPTION);

    // If we decide not to LOGP the below function can be used to generate
    // failure response message to PE, commenting for now
    // halPS_HandleFwRspTimeout(pMac, pHalPwrSave->rspType);
}

// If we decide not to LOGP the below function can be used to generate
// failure response message to PE, commenting for now
#if 0
static void halPS_HandleFwRspTimeout(tpAniSirGlobal pMac, tANI_U16 rspType)
{
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tANI_U16 dialogToken = pHalPwrSave->dialogToken;

    // Increment the token number so that if the FW responds later,
    // the rsponse message is not passed up
    pHalPwrSave->dialogToken++;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, rspType, dialogToken, NULL, eHAL_STATUS_FW_MSG_TIMEDOUT);

    return;
}
#endif
/*
 * DESCRIPTION:
 *      Get Function to return the current state of the Power Save
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      HAL_PWR_SAVE_ACTIVE_STATE
 *      HAL_PWR_SAVE_IMPS_STATE
 *      HAL_PWR_SAVE_BMPS_STATE
 *      HAL_PWR_SAVE_SUSPEND_BMPS_STATE
 *      HAL_PWR_SAVE_UAPSD_STATE
 */
tANI_U8 halPS_GetState(tpAniSirGlobal pMac)
{
    return (pMac->hal.PsParam.pwrSaveState.p.psState);
}

/*
 * DESCRIPTION:
 *      Power Save Configuration function for configuring all the required
 *      parameters for power save in the FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pPowerSaveConfig: PS Configuration data structure
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_Config(tpAniSirGlobal pMac, tpSirPowerSaveCfg pPowerSaveConfig)
{
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalRegBckup *pRegBckup = &pMac->hal.RegBckupParam;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    uNvTables nvTables; 
    
    status = halReadNvTable( pMac, NV_TABLE_XO_WARMUP_US, &nvTables );
    if (!HAL_STATUS_SUCCESS( status ))
    {
        VOS_TRACE(VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_ERROR, "fail to get NV_TABLE_XO_WARMUP_US\n");
        return status;
    }

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    // Beacon Filtering
    if (pPowerSaveConfig->nthBeaconFilter) {
        /* Commented since this gets configured through registry */
        /*pFwConfig->bBeaconFilterEnable = TRUE; */
        pFwConfig->ucBeaconFilterPeriod = (tANI_U8)pPowerSaveConfig->nthBeaconFilter;
    } else {
        /* Commented since this gets configured through registry */
        /* pFwConfig->bBeaconFilterEnable = FALSE; */
        pFwConfig->ucBeaconFilterPeriod = 0;
    }

    // Network wakeup
    if (pPowerSaveConfig->broadcastFrameFilter) {
        pFwConfig->bNetWakeupFilterEnable = TRUE;
        pFwConfig->ucNetWakeupFilterPeriod = pPowerSaveConfig->broadcastFrameFilter;
    } else {
        pFwConfig->bNetWakeupFilterEnable = FALSE;
        pFwConfig->ucNetWakeupFilterPeriod = 0;
    }

    // RSSI monitoring
    pFwConfig->bMinRssiAvgAlg = TRUE;
    pFwConfig->ucNumBeaconRssiAvg = pPowerSaveConfig->numBeaconPerRssiAverage;
    pFwConfig->ucRssiFilterPeriod = pPowerSaveConfig->rssiFilterPeriod;
    /* Commented since this gets configured through registry */
    /*
    if (pPowerSaveConfig->rssiFilterPeriod) {
        pFwConfig->bRssiFilterEnable = TRUE;
    } else {
    pFwConfig->bRssiFilterEnable = FALSE;
    }
    */

    // Tx Path monitoring in milliseconds.
    pFwConfig->ucBdPduEmptyMonitorMs = HAL_PWR_SAVE_FW_TX_PATH_MONITOR_TIME_MSEC;

    // Maximum PS Poll
    pFwConfig->ucMaxPsPoll = (tANI_U8)pPowerSaveConfig->maxPsPoll;

    // Maximum # of missed beacon before miss beacon intr is generated
    pFwConfig->ucMaxMissBeacon = (tANI_U8)pPowerSaveConfig->HeartBeatCount;

    // PMU sleep timeout
    pFwConfig->usPmuSleepTimeoutMsec =  HAL_PWR_SAVE_FW_PMU_SLEEP_TIMEOUT;

    // Max frame retries
    pFwConfig->ucMaxFrmRetries = HAL_PWR_SAVE_FW_FRAME_RETRIES;

    pFwConfig->ucBcastDataRecepTimeoutMs = HAL_PWR_SAVE_FW_MAX_BCAST_DATA_RECEPTION_TIME_MS;
    /* Commented since this gets configured through registry */
    /*
    pFwConfig->ucUcastDataRecepTimeoutMs = HAL_PWR_SAVE_FW_MAX_UCAST_DATA_RECEPTION_TIME_MS;
    */
    pFwConfig->ucMaxSifUnfreezeTimeoutMs = HAL_PWR_SAVE_FW_MAX_SIF_UNFREEZE_TIME_MS;
    pFwConfig->ucBtqmQueuesEmptyTimeoutMs = HAL_PWR_SAVE_FW_MAX_BTQM_QUEUES_EMPTY_TIMEOUT_MS;

    pFwConfig->ucBmpsFirstBeaconTimeoutMs = HAL_PWR_SAVE_FW_FIRST_BEACON_RECEPTION_TIMEOUT_MS;
    pFwConfig->usBmpsMinSleepTimeUs = HAL_PWR_SAVE_FW_BMPS_MINIMUM_SLEEP_TIME_US;
    if(pFwConfig->bRfXoOn)
    {
        pFwConfig->usBmpsSleepTimeOverheadsUs19_2 = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_RFXO_US_19_2;
        pFwConfig->ucRfSupplySettlingTimeClk19_2 = HAL_PWR_SAVE_FW_BMPS_RF_SETTLING_TIME_CLKS_19_2;
        /* RF Settling Time Clk value as read from CFG */
        pFwConfig->ucRfSupplySettlingTimeClk = (tANI_U16)((nvTables.xoWarmupUs*1000)/QWLAN_PMIC_SLEEPCLK_PERIOD_NS);
        pFwConfig->ucRfSupplySettlingTimeClk += ((nvTables.xoWarmupUs*1000)%QWLAN_PMIC_SLEEPCLK_PERIOD_NS)? 1 : 0;
        pFwConfig->usBmpsSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_WITHOUT_RFXO_SETTLING_US + 
					((pFwConfig->ucRfSupplySettlingTimeClk * QWLAN_PMIC_SLEEPCLK_PERIOD_NS)/1000);

        pFwConfig->usBmpsForcedSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_RFXO_US;
    }
    else
    {
        pFwConfig->usBmpsSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_US;
        pFwConfig->usBmpsForcedSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_FORCED_SLEEP_TIME_OVERHEADS_US;
    }

    pFwConfig->usBmpsModeEarlyTimeoutUs = HAL_PWR_SAVE_FW_BMPS_BEACON_MODE_EARLY_TIMEOUT_US;
    pFwConfig->ucUapsdDataRecepTimeoutMs = HAL_PWR_SAVE_FW_UAPSD_DATA_RECEPTION_TIMEOUT_MS;

    //Beacon Miss Handling
    pFwConfig->bBcnMissMLC = TRUE;
    pFwConfig->ucMaxBcnWaitTU = HAL_PWR_SAVE_BCN_MISS_WAIT_TU;
    pFwConfig->ucMinBcnWaitTU = HAL_PWR_SAVE_MIN_BCN_WAIT_TU;
    pFwConfig->uBcnMissGracePeriodUs = HAL_PWR_SAVE_BCN_MISS_GRACE_PERIOD_US;
    pFwConfig->ucNumConsBcnMiss = HAL_PWR_SAVE_MAX_CONS_BCN_MISS; 
    pFwConfig->uMaxAllowBcnDriftUs = HAL_PWR_SAVE_MAX_ALLOWED_BCN_DRIFT_US;

    // Listen Interval
    // just save it in HAL cache. It will be written to FW sys config at Enter BMPS.
    pHalPwrSave->listenInterval = pPowerSaveConfig->listenInterval;

    // Chip power down during BMPS enabled by default
    pFwConfig->bNoPwrDown = HAL_PWR_SAVE_FW_CHIP_PWR_DOWN_DISABLE;

    // update the tpc gain lut adu reinit addr
    pFwConfig->uTpcGainLutAduReinitAddr = pRegBckup->regListTPCGainLutStartAddr;

    pFwConfig->uVolRegReinitAddress = pRegBckup->volatileRegListStartAddr;
    pFwConfig->uNumVolRegCount      = pRegBckup->volatileRegListSize/(2*sizeof(tANI_U32));
    // Enable mutex protection between host and FW for register access
    pFwConfig->bMutexProtectionEnable = TRUE;

    // Enable the reference tbtt adjustment feature in FW
    pFwConfig->bRefTbttAdjustment = TRUE;

    // The initial compensation what fw can apply to the reference
    // TBTT passed on by the Host
    pFwConfig->uInitTimeComp = 0;

    pHalPwrSave->ignoreDtim = pPowerSaveConfig->ignoreDtim;

    // Release the memory
    palFreeMemory(pMac->hHdd, pPowerSaveConfig);

    // Write the configuration parameters in the memory mapped for
    // system configuration parameters
    status = halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

    return status;
}


/*
 * DESCRIPTION:
 *      Function to set RSSI thresholds.
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pThresholds: Three different RSSI threshold values
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */

eHalStatus halPS_SetRSSIThresholds(tpAniSirGlobal pMac, tpSirRSSIThresholds pThresholds)
{
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;
    eHalStatus status;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    pFwConfig->ucRssiThreshold1 = pThresholds->ucRssiThreshold1;
    pFwConfig->ucRssiThreshold2 = pThresholds->ucRssiThreshold2;
    pFwConfig->ucRssiThreshold3 = pThresholds->ucRssiThreshold3;
    pFwConfig->bRssiThres1PosNotify = pThresholds->bRssiThres1PosNotify;
    pFwConfig->bRssiThres1NegNotify = pThresholds->bRssiThres1NegNotify;
    pFwConfig->bRssiThres2PosNotify = pThresholds->bRssiThres2PosNotify;
    pFwConfig->bRssiThres2NegNotify = pThresholds->bRssiThres2NegNotify;
    pFwConfig->bRssiThres3PosNotify = pThresholds->bRssiThres3PosNotify;
    pFwConfig->bRssiThres3NegNotify = pThresholds->bRssiThres3NegNotify;

    if (IS_PWRSAVE_STATE_IN_BMPS) 
    {
        halPS_SetHostBusy(pMac, HAL_PS_BUSY_GENERIC); 
    }

    HALLOG1(halLog(pMac, LOG1, 
                FL("RSSI notification from FW thrs1=%d, thrs2=%d, thrs3-%d;  %d,%d - %d,%d - %d,%d\n"), 
                pThresholds->ucRssiThreshold1,
                pThresholds->ucRssiThreshold2,
                pThresholds->ucRssiThreshold3,
                pThresholds->bRssiThres1PosNotify,
                pThresholds->bRssiThres1NegNotify,
                pThresholds->bRssiThres2PosNotify,
                pThresholds->bRssiThres2NegNotify,
                pThresholds->bRssiThres3PosNotify,
                pThresholds->bRssiThres3NegNotify));

    // Write the configuration parameters in the memory mapped for
    // system configuration parameters
    status =  halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

    if (IS_PWRSAVE_STATE_IN_BMPS) 
    {
        halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC); 
    }

    return status;
}

static void computeRssAvg(tANI_U32 value, tRssAvgParam* rssi_param)
{
    tANI_U32 aCount = 0;
    tANI_S32 rssiAnt0 = 0, rssiAnt1 = 0, rssiAnt = 0;

    if(value & QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT0_STORE_REG0_VLD_MASK)
    {
        rssiAnt0 = ((value & QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT0_STORE_REG0_MASK)
                                >> QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT0_STORE_REG0_OFFSET);
    }
    if(value & QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT1_STORE_REG0_VLD_MASK)
    {
        rssiAnt1 = ((value & QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT1_STORE_REG0_MASK)
                          >> QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_PMU_RSSI_ANT1_STORE_REG0_OFFSET);
    }
    if ((rssiAnt0 > QWLANFW_PWRSAVE_RSSI_NOISE_FLOOR) || (rssiAnt1 > QWLANFW_PWRSAVE_RSSI_NOISE_FLOOR))
    {
        aCount++;
        rssiAnt = (rssiAnt0 > rssiAnt1) ? rssiAnt0 : rssiAnt1;
    }
    rssi_param->totRssi = rssiAnt;
    rssi_param->avgCount = aCount;
    rssi_param->isNonZero = (rssiAnt0 || rssiAnt1) ? 1 : 0;
}

/*
 * DESCRIPTION:
 *      Function to Get the RSSI value in BMPS mode.
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pRssi:  Rssi value
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_GetRssi(tpAniSirGlobal pMac, tANI_S8 *pRssi)
{
    tANI_U32 value[WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX];
    tANI_U32 i, startPtr;
    tANI_U32 reqd=WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX, max=WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX;
    tANI_S32 rssiVal=0, rssiAvgCount=0, nonZeroRSSIReg=0;
    eHalStatus halStatus;
    tRssAvgParam rssi_param;

    //make sure it is pwr save, before we start reading the PMU BMPS RSSI registers
    if (!halUtil_CurrentlyInPowerSave(pMac))
    {

        HALLOGE(halLog(pMac, LOGE, FL("This RSSI value can be fetched only in BMPS mode!\n")));
        return eHAL_STATUS_FAILURE;
    }

    if(wlan_cfgGetInt(pMac, WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE, &reqd) != eSIR_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("cfgGetInt() WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE failed \n")));
        return eHAL_STATUS_FAILURE;
    }

    // Since we are accessing the libra registers in PS, set host busy (by acquiring the mutex)
    // so that FW does not put the chip to sleep while host is accessing.
    halPS_SetHostBusy(pMac, HAL_PS_BUSY_GENERIC);

    halStatus = halReadRegister(pMac, QWLAN_PMU_RSSI_ANT_PTR_REG, &startPtr) ;
    if(eHAL_STATUS_SUCCESS != halStatus)
    	return halStatus;
    startPtr = (startPtr & QWLAN_PMU_RSSI_ANT_PTR_PMU_RSSI_ANT1_STORE_REG19_MASK)
                        >> QWLAN_PMU_RSSI_ANT_PTR_PMU_RSSI_ANT1_STORE_REG19_OFFSET;

    for(i = 0; i < WNI_CFG_NUM_BEACON_PER_RSSI_AVERAGE_STAMAX; i++)
    {
        halStatus = halReadRegister(pMac, QWLAN_PMU_PMU_RSSI_ANT_STORE_REG0_REG + (4*i), &value[i]);
        if(eHAL_STATUS_SUCCESS != halStatus)
    	    return halStatus;
    }

    if((startPtr + 1) < reqd)
    {
        for(i = 0; i <= startPtr; i++)
        {
            computeRssAvg(value[i], &rssi_param);
            rssiVal += rssi_param.totRssi;
            rssiAvgCount += rssi_param.avgCount;
            nonZeroRSSIReg += rssi_param.isNonZero;
        }
        for(i = (max + startPtr - reqd + 1); i < max; i++)
        {
            computeRssAvg(value[i], &rssi_param);
            rssiVal += rssi_param.totRssi;
            rssiAvgCount += rssi_param.avgCount;
            nonZeroRSSIReg += rssi_param.isNonZero;
        }
    }
    else
    {
        for(i = (startPtr - reqd + 1); i <= startPtr; i++)
        {
            computeRssAvg(value[i], &rssi_param);
            rssiVal += rssi_param.totRssi;
            rssiAvgCount += rssi_param.avgCount;
            nonZeroRSSIReg += rssi_param.isNonZero;
        }
    }

    if(rssiAvgCount && (nonZeroRSSIReg == reqd))
    {
        sRssiChannelOffsets  *pRssiOffset;
        eRfChannels curChan = rfGetCurChannel(pMac);
        tANI_S16 skuOffset = 0;

		VOS_ASSERT(curChan != INVALID_RF_CHANNEL);
        if(curChan >= NUM_2_4GHZ_CHANNELS)
            return eHAL_STATUS_FAILURE;         
        //apply the sku dependant offset on the base RSSI value.
        if(halGetNvTableLoc(pMac, NV_TABLE_RSSI_CHANNEL_OFFSETS, (uNvTables **)&pRssiOffset) == eHAL_STATUS_SUCCESS)
        {
            skuOffset = pRssiOffset[PHY_RX_CHAIN_0].bRssiOffset[curChan] / 100;
        }
        // The range of 7 bit RSSI is [-100dBm, +27dBm]
        *pRssi = (tANI_S8)((rssiVal/rssiAvgCount) - 100 + skuOffset);
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("No valid rssi value stored in HW PMU RSSI registers \n")));
        *pRssi = 0;
    }

    // Release the mutex
        halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC);
    return eHAL_STATUS_SUCCESS;
}

/* Low/High RSSI Indication */
/*
 * DESCRIPTION:
 *      Function to handle the RSSI notification from the FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwRssiNotification(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
#if defined FEATURE_WLAN_GEN6_ROAMING || defined WLAN_FEATURE_NEIGHBOR_ROAMING
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(tMBoxMsgHdr);
    tpSirRSSINotification pRSSINotification = (tpSirRSSINotification)pMsgBody;

    HALLOG1(halLog(pMac, LOG1, FL("RSSI notification from FW %d,%d - %d,%d - %d,%d\n"), 
                pRSSINotification->bRssiThres1PosCross,
                pRSSINotification->bRssiThres1NegCross,
                pRSSINotification->bRssiThres2PosCross,
                pRSSINotification->bRssiThres2NegCross,
                pRSSINotification->bRssiThres3PosCross,
                pRSSINotification->bRssiThres3NegCross ));
    //notify TL
    halTLRSSINotification(pMac, pRSSINotification);
#endif
    return status;
}

/*
 * DESCRIPTION:
 *      Function to set self and peer AP's MAC addresses after association is done.
 *      Only used in infrastructure mode.
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pStaMacAddr: Self STA MAC address
 *      pApMacAddr:  Peer AP MAC address
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */

eHalStatus halPS_SetPeerParams(tpAniSirGlobal pMac, tANI_U8 staIdx,
        tANI_U8 *pStaMacAddr, tANI_U8 *pApMacAddr)
{
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;
    tANI_U8 dpuIdx = 0, dpuSig = 0;

#ifdef HAL_SELF_STA_PER_BSS
    tANI_U8      selfStaIdx;
#endif

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    // FW uses staIdx for xmitting datanull frames, which should be out
    // of selfSta
#ifdef HAL_SELF_STA_PER_BSS
    halTable_GetBssSelfStaIdxForSta(pMac, staIdx, &selfStaIdx);
    pFwConfig->staIdx = selfStaIdx;
#else
    pFwConfig->staIdx = pMac->hal.halMac.selfStaId;
#endif

    pFwConfig->peerStaIdx = staIdx;

    if (staIdx != HAL_STA_INVALID_IDX) {
        pFwConfig->staMacAddrLo = *((tANI_U32 *)pStaMacAddr);
        pFwConfig->staMacAddrHi = *(tANI_U16 *)(pStaMacAddr + 4);

        pFwConfig->apMacAddrLo = *((tANI_U32 *)pApMacAddr);
        pFwConfig->apMacAddrHi = *((tANI_U16 *)(pApMacAddr + 4));

        pFwConfig->bRegulateTraffic = TRUE;
    } else {
        pFwConfig->staMacAddrLo = 0;
        pFwConfig->staMacAddrHi = 0;

        pFwConfig->apMacAddrLo = 0;
        pFwConfig->apMacAddrHi = 0;

        pFwConfig->bRegulateTraffic = FALSE;
    }

    halTable_GetStaDpuIdx(pMac, staIdx, &dpuIdx);
    halDpu_GetSignature(pMac, dpuIdx, &dpuSig);

    pFwConfig->ucDpuSig = dpuSig;
    pFwConfig->ucDpuIdx = dpuIdx;

    // Write the configuration parameters in the memory mapped for
    // system configuration parameters
    return halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

}


/*
 * DESCRIPTION:
 *      Function to perform the PS Poll frame template to be send by FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SetPsPollParam(tpAniSirGlobal pMac, tANI_U8 staIdx,
        tANI_U16 aid, tANI_U16 rateIndex, tANI_U8 txPower)
{
    Qwlanfw_SysCfgType *pFwConfig;

    pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    pFwConfig->ucStaAid = aid;
    pFwConfig->ucRateIndex = rateIndex;
    pFwConfig->ucTxPower = txPower;

    // Write the configuration parameters in the memory mapped for
    // system configuration parameters
    return halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));
}

/*
 * DESCRIPTION:
 *      Function to update the beaoon interval into the FW sys config
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      beaconInterval: beacon interaval in TUs
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SetBeaconInterval(tpAniSirGlobal pMac, tANI_U16 beaconInterval)
{
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

    pFwConfig->ucBeaconIntervalMsec = beaconInterval;

    // Write the configuration parameters in the memory mapped for
    // system configuration parameters
    return halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

}

/*
 * DESCRIPTION:
 *      Function to update the listen interval into the HAL cache.
 *      It will be written to FW sysCfg at enter BMPS
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      listenInterval:  interger value
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SetListenIntervalParam(tpAniSirGlobal pMac, tANI_U16 listenInterval)
{
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    if(!pHalPwrSave)
        return eHAL_STATUS_FAILURE;
    pHalPwrSave->listenInterval = listenInterval;
    return eHAL_STATUS_SUCCESS;
}



/*
 * DESCRIPTION:
 *      Function that performs the exit IMPS sequence
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_ExitImpsSequence(tpAniSirGlobal pMac)
{
    eHalStatus  status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
#ifndef WLAN_FW_BTQM_STA_MGMT
    tANI_U8 staId;
#endif // WLAN_FW_BTQM_STA_MGMT

#ifndef WLAN_FW_BTQM_STA_MGMT
    // Enable BTQM queues
    for( staId = 0 ; staId < pMac->hal.memMap.maxStations ; staId++ ) {
        if ((halTable_ValidateStaIndex( pMac, staId)) == eHAL_STATUS_SUCCESS) {
            if ((status = halBmu_sta_enable_disable_control( pMac, staId,
                          eBMU_ENB_TX_QUE_ENB_TRANS)) != eHAL_STATUS_SUCCESS ) {
                          HALLOGP( halLog( pMac, LOGP,
                              FL("Enabling Tx queue failed for staId %d"), staId ));
                return status;
            }
	    }
    }
#endif

    // Enable RX
    status = halRxp_enable(pMac);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Enable RX failed\n")));
        return status;
    }

    // Enable DXE engine, reset the enable bit in the DXE CSR register
    status = halDxe_EnableDisableDXE(pMac, TRUE);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Enable DXE failed\n")));
        return status;
    }

    // Resume BAL from suspend
    vosStatus = WLANBAL_Resume(pVosGCtx);
    if (vosStatus != VOS_STATUS_SUCCESS) {
        // Driver reset maybe required here, panic by LOGP
        HALLOGP( halLog(pMac, LOGP, FL("Resume BAL failed!!!")));
        return eHAL_STATUS_FAILURE;
    }

    // Set the power state to active
    pMac->hal.PsParam.pwrSaveState.p.psState = HAL_PWR_SAVE_ACTIVE_STATE;

    // Start the FW heart beat monitor, once we are back to full power.
    halFW_StartChipMonitor(pMac);

    return eHAL_STATUS_SUCCESS;
}


/*
 * DESCRIPTION:
 *      Idle Mode Power Save (IMPS) functions
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token for the message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleEnterImpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsImps *pImpsCtx = &pMac->hal.PsParam.ImpsCtx;
    Qwlanfw_EnterImpsReqType msg;
#ifndef WLAN_FW_BTQM_STA_MGMT
    tANI_U8 staId;
#endif // WLAN_FW_BTQM_STA_MGMT

    // Stop the FW heartbeat monitoring till the IMPS request is been
    // processed. Host is not supposed to access HW when in IMPS
    halFW_StopChipMonitor(pMac);

    // Set the state of power save, in IMPS requested
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_IMPS_REQUESTED;

	// Disable RX
    status = halRxp_disable(pMac, TRUE);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Disable RX failed\n")));
        goto error;
    }

	status = halDxe_EnsureDXEIdleState(pMac);
	if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Ensure DXE IDLE state failed\n")));
        goto error;
    }

    // Suspend BAL routine
    vosStatus = WLANBAL_Suspend(pVosGCtx);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        // TODO: Driver reset maybe required here
        HALLOGP( halLog( pMac, LOGP, FL("ERROR: BAL suspend failed, status = %d\n!!!"),
                vosStatus));
 		goto error;
    }

    // Disable DXE engine, reset the enable bit in the DXE CSR register
    status = halDxe_EnableDisableDXE(pMac, FALSE);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Disable DXE failed\n")));
        goto error;
    }

#ifndef WLAN_FW_BTQM_STA_MGMT
    // Disable and cleanup the BTQM queues
    for( staId = 0 ; staId < pMac->hal.memMap.maxStations ; staId++ ) {
        if ((halTable_ValidateStaIndex( pMac, staId)) == eHAL_STATUS_SUCCESS) {
            if (( status = halBmu_sta_enable_disable_control( pMac, staId,
                           eBMU_DIS_TX_QUE_DIS_TRANS_CLEANUP_QUE)) !=
			               eHAL_STATUS_SUCCESS ) {
                HALLOGE( halLog( pMac, LOGE,
                        FL("Disabling BTQM Tx queue failed for staId %d"),
                        staId ));
                // Should we enable the DXE engine again here???
                goto error;
            }
	    }
    }
#endif

    // Load the ADU memory with the indirectly accessed register list
    status = halPS_RegBckupIndirectRegisters(pMac);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Load indirect register failed\n")));
        goto error;
    }

#if 0 // Since FW is setting the ADU reinit address in PMU, host should not touch it.
      // Host provides this address in the sysConfig from where FW programs it in PMU
    // Set the ADU register re-init memory address
    halPmu_SetAduReInitAddress(pMac, pImpsCtx->aduMemAddr);
#endif

    // Set the FW system config for the re-init address location
    status = halFW_UpdateReInitRegListStartAddr(pMac, pImpsCtx->aduMemAddr);

    // Set the response to be given back to PE, in case of timeout to get the
    // response back from FW, this rspType will be send to the upper layer
    pHalPwrSave->rspType = SIR_HAL_ENTER_IMPS_RSP;
    pHalPwrSave->dialogToken = dialogToken;

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Timer Start Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        // In case of this failure how does HAL bring FW out of the IMPS state???
        goto error;
    }

    // Send the ENTER_IMPS request to firmware, and set the timer for the
    // response message to be received
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_ENTER_IMPS_REQ, dialogToken, sizeof(Qwlanfw_EnterImpsReqType), &msg, TRUE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send IMPS request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }

        goto error;
    }

    return eHAL_STATUS_SUCCESS;

error:
    // Exit out of IMPS state
    halPS_ExitImpsSequence(pMac);

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_IMPS_RSP,
            pHalPwrSave->dialogToken, NULL, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Enter IMPS response from the FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwEnterImpsRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus  status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Timer Stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog( pMac, LOG1, FL("IMPS enter resp received from FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pHalPwrSave->dialogToken) {
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pHalPwrSave->dialogToken));
        // TODO: Recovery Mechanism required here
        goto respond;

    }

    // Check the FW status of the response message, if failure then
    // exit out of IMPS by restoring back to the normal state and
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        halPS_ExitImpsSequence(pMac);
        goto respond;
    }


    // Execute the standby procedure
    halPS_ExecuteStandbyProcedure(pMac);

    /** Zero out the BTQM_TX_WQ descriptor */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.btqmTxQueue_offset,
                              pMac->hal.memMap.btqmTxQueue_size) ;


    // As FW response is success, lets proceed further and suspend chip
    vosStatus = WLANBAL_SuspendChip(pVosGCtx);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGP( halLog( pMac, LOGP, FL("ERROR: BAL suspend Chip failed, status = %d\n!!!"),
                vosStatus));
    }

    // Set the state of power save, now in IMPS
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_IMPS_STATE;
    status = eHAL_STATUS_SUCCESS;


respond:
    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_IMPS_RSP,
            pHalPwrSave->dialogToken, NULL, fwStatus);
    palFreeMemory(pMac->hHdd, pFwMsg);
    if(status != eHAL_STATUS_SUCCESS){
        /* Enter IMPS failure, now reenable ASIC interrupt */
        halIntChipEnable(pMac);
    }
    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle the Enter IMPS response from the FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_PostponeFwEnterImpsRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus  status = eHAL_STATUS_FAILURE;
    tSirMsgQ        msgQ;
    tANI_U8 *pMsg = NULL;
    tANI_U32 retStatus;
    // Since we are postponing the mesg handling, we need to copy the
    // buffer which contains the mailbox message read from device elsewhere
    // since the message itself would be freed by caller when return from here.
    retStatus = palAllocateMemory(pMac->hHdd, (void **)&pMsg,
            sizeof(Qwlanfw_EnterImpsRspType));
    if(retStatus != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE,
                FL("Allocating memory in halPS_PostponeFwEnterImpsRsp failed\n")));
        return eHAL_STATUS_FAILED_ALLOC;
    }

    // Copy the message, it would be freed by the mesg handler later.
    palCopyMemory(pMac->hHdd, pMsg, pFwMsg, sizeof(Qwlanfw_EnterImpsRspType));

    /* Disable ASIC interrupt first */
    halIntChipDisable(pMac);

    msgQ.type = SIR_HAL_POSTPONE_ENTER_IMPS_RSP;
    msgQ.reserved = 0;
    msgQ.bodyval = 0;
    msgQ.bodyptr = (void *)pMsg;

    /* Then post a message to HAL itself, HAL will suspend chip later */
    if(eSIR_SUCCESS != halPostMsgApi( pMac, &msgQ )){
        HALLOGE( halLog(pMac, LOGE, FL("Post SIR_HAL_POSTPONE_ENTER_IMPS_RSP failed \n")));
    }else
        status = eHAL_STATUS_SUCCESS;
    return status;
}

/*
 * DESCRIPTION:
 *      Function to execute the standby routine.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      deepSleep: If this is a deepSleep initiated standby procedure.
 *
 * RETURN:
 *      VOID
 */
void halPS_ExecuteStandbyProcedure( tpAniSirGlobal pMac)
{
    tANI_U32 regValue = 0;

#ifdef VOLANS_RF
    // RFAPB registers are to be used only for real chip and with RF Access Enabled
    // disable Pllen_force, IQ_DIV_MODE, En_TXLO_Mode, En_RXLO_Mode bits in rfApb modeSel1 register
    halWriteRegister(pMac, QWLAN_RFAPB_MODE_SEL1_REG, 0);
#endif //#if VOLANS_RF

#ifndef VOLANS_FPGA
    // Footer switch is toggled for both BB and RF version of the chip (not for FPGA)
    // restore pmu_rfa_tcxo_buf_en_mask under rf_pa_trsw_ctrl_reg to defaults
    // Disable mif_mem_fs_bypas in mif_mem_fs_ctrl_reg
    halReadRegister(pMac,QWLAN_PMU_MIF_MEM_FS_CTRL_REG_REG, &regValue);
    regValue &= ~QWLAN_PMU_MIF_MEM_FS_CTRL_REG_MIF_MEM_FS_BYPASS_MASK;
    halWriteRegister(pMac, QWLAN_PMU_MIF_MEM_FS_CTRL_REG_REG, regValue);
#endif //VOLANS_FPGA

    // enable pmu_gcu_sdio_aux_clk_gate_sel_reg, pmu_gcu_rosc_clk_gate_sel_reg  and ring_osc_pwr_en_sel_reg in ring_osc_ctrl_sel_reg
    regValue  = QWLAN_PMU_RING_OSC_CTRL_SEL_REG_PMU_GCU_SDIO_AUX_CLK_GATE_SEL_REG_MASK|
                QWLAN_PMU_RING_OSC_CTRL_SEL_REG_PMU_GCU_ROSC_CLK_GATE_SEL_REG_MASK |
    halWriteRegister(pMac, QWLAN_PMU_RING_OSC_CTRL_SEL_REG_REG, regValue);

	/**
	 * Volans does not have internal ring oscillator in PMU. Hence, removing
	 * ring_osc_pwr_en bit configurations from this routine
	 */

	/**
	 * en_rtx_bias under mode_sel2 reg enable is not required volans. Hence,
	 * removing it from source code
	 */

	/**
	 * Clearing PMU test debug status register. The register provides vital debug
     * information about internal PMU state machine in case of failure
	 */
    halWriteRegister(pMac, QWLAN_PMU_PMU_DEBUG_STATUS_REG_REG, 0x0);

    return;
}


/*
 * DESCRIPTION:
 *      Function to handle the Exit IMPS request from upper layer PE.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be used for the message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleExitImpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    v_CONTEXT_t pVosGCtx = vos_get_global_context(VOS_MODULE_ID_HAL, (v_VOID_t *) pMac);

    if (!(pMac->hal.PsParam.pwrSaveState.p.psState & HAL_PWR_SAVE_IMPS_STATE)) {
        HALLOGE( halLog(pMac, LOGE, FL("System not in IMPS state\n")));
        status = eHAL_STATUS_FAILURE;
        goto error;
    }

    // Set the response to be given back to PE, in case of timeout to get the
    // response back from FW, this rspType will be send to the upper layer
    pHalPwrSave->rspType = SIR_HAL_EXIT_IMPS_RSP;
    pHalPwrSave->dialogToken = dialogToken;

    // Start the timer for the FW response
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        goto error;
    }


    // Resume BAL from suspend
    vosStatus = WLANBAL_ResumeChip(pVosGCtx);
    if (vosStatus != VOS_STATUS_SUCCESS) {
        // Driver reset maybe required here, panic by LOGP
        HALLOGP( halLog(pMac, LOGP, FL("Resume BAL failed!!!")));
        return eHAL_STATUS_FAILURE;
    }

    /* Reenable ASIC interrupt */
    halIntChipEnable(pMac);

    return eHAL_STATUS_SUCCESS;

error:

    // Set the state back to active
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_ACTIVE_STATE;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_IMPS_RSP,
            pHalPwrSave->dialogToken, NULL, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Exit IMPS response from the FW.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwImpsExited(tpAniSirGlobal pMac, void* pFwMsg)
{
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus  status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(tMBoxMsgHdr);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog( pMac, LOG1, FL("IMPS exit received by FW, status = %d\n"), fwStatus));

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
        // FW failed to get the chip out of IMPS state, so HAL informs the PE
        // with a failure response and stays in the IMPS state
        goto respond;
    }

    // Perform the exit IMPS sequence
    status = halPS_ExitImpsSequence(pMac);

respond:
    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_IMPS_RSP,
            pHalPwrSave->dialogToken, NULL, fwStatus);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to compute the listen interval based on the current
 *      configured LI and the DTIM period. The purpose is to find LI
 *      which is exactly a disible by DTIM period in order to
 *      align LI with DTIM.
 *
 * PARAMETERS:
 *      dtimPeriod
 *      listenInterval
 *      power saving level (min or max)
 *      modified LI to be returned
 *
 * RETURN:
 *      void
 */
void halPS_ComputeListenInterval(tANI_U8 dtim, tANI_U16 listenIntv,
        tANI_U8 psLevel, tANI_U16 *modifiedLI)
{
    tANI_U16 remainder = 0;
    tANI_U16 gcd = 0, li = 0;

    // If listen interval is greater than or equal to the DTIM count
    // align LI to DTIM count by assigning it to DTIM count.
    if(listenIntv >= dtim) {
        *modifiedLI = dtim;
    } else {
        // Check if the DTIM is divisible by LI
        remainder = dtim%listenIntv;
        if(remainder == 0) {
            *modifiedLI = listenIntv;
        } else {
            // If not divisible, get the GCD
            gcd = halUtil_GetGCD(dtim, listenIntv);
            *modifiedLI = li = gcd;
            // Find LI closer to the DTIM and still a divisor.
            // LI=2 DTIM=6 GCD=2, but LI=3 is closest
            while(li<=listenIntv) {
                if ((dtim%li)==0) {
                    *modifiedLI = li;
                }
                li++;
            }
        }
    }
}



/*
 * DESCRIPTION:
 *      Function to update the FW system config with the PS related parameters.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dtimPeriod: DTIM period as advertised for the BSS
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_UpdateFwSysConfig(tpAniSirGlobal pMac, tpEnterBmpsParams pPeMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tANI_U16 transListenInterval = (tANI_U16)pMac->hal.transListenInterval;
    tANI_U16 maxListenInterval = (tANI_U16)pMac->hal.maxListenInterval;
    tANI_U16 bcnIntervalTU;
    tANI_U16 listenInterval = (tANI_U16)pHalPwrSave->listenInterval;
    tANI_U8 filterPeriod=0;

    halTable_GetBeaconIntervalForBss(pMac, (tANI_U8)pPeMsg->bssIdx, &bcnIntervalTU);

    // Compute the Listen interval based on DTIM period, to align
    // the LI with the DTIM period. Basically LI should be multiple of
    // DTIM. This would be done only if ignoreDtim is not set.
        halPS_ComputeListenInterval(pPeMsg->dtimPeriod, (tANI_U16)pHalPwrSave->listenInterval, 0,
                &listenInterval);

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);
        vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
        psRequest.event_subtype = WLAN_BMPS_FINAL_LI;
        psRequest.final_listen_intv = listenInterval;
        WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
    }
#endif

    pFwConfig->ucListenInterval = listenInterval;
    pFwConfig->ucDtimPeriod     = pPeMsg->dtimPeriod;
    pFwConfig->bBcMcFilterSetting     = pMac->hal.mcastBcastFilterSetting;

    /*Telescopic Beacon wakeup configuration*/
    pFwConfig->bTelescopicBcnWakeupEn = pMac->hal.teleBcnWakeupEnable;
    pFwConfig->ucTransListenInterval=0;
    pFwConfig->ucMaxListenInterval=0;

    if ((maxListenInterval <= transListenInterval) || (maxListenInterval <= listenInterval))
        maxListenInterval = 0;

    if (transListenInterval <= listenInterval) {
        transListenInterval = maxListenInterval;
        maxListenInterval = 0;   
    }

    /* From this point transLi will be less than
     * maxLI due to above nullifying on top.
     */
    if((transListenInterval * bcnIntervalTU) > QWLAN_MAX_LISTEN_INTERVAL_TU) {
        transListenInterval = 0;
        maxListenInterval = 0;   
    }

    if((maxListenInterval * bcnIntervalTU) > QWLAN_MAX_LISTEN_INTERVAL_TU) {
        maxListenInterval = 0;   
    }

    /*Transient Listen Interval and its Idle time configuration*/
    if (pMac->hal.teleBcnWakeupEnable && transListenInterval) {
        pFwConfig->ucTransListenInterval=transListenInterval;
        pFwConfig->uTransLiNumIdleBeacons = pMac->hal.uTransLiNumIdleBeacons;
    }

    if (pMac->hal.teleBcnWakeupEnable && maxListenInterval) {
        pFwConfig->ucMaxListenInterval=maxListenInterval;
        pFwConfig->uMaxLiNumIdleBeacons = pMac->hal.uMaxLiNumIdleBeacons;
    }

    HALLOGE( halLog(pMac, LOGE, FL("Updating Telescopic params %x %x %x %x %x"), 
                                   pFwConfig->bBcMcFilterSetting, pFwConfig->ucTransListenInterval,
                                   pFwConfig->uTransLiNumIdleBeacons, pFwConfig->ucMaxListenInterval,
                                   pFwConfig->uMaxLiNumIdleBeacons));

    if (pFwConfig->ucBeaconFilterPeriod) {
        filterPeriod = (tANI_U8)(((pFwConfig->ucBeaconFilterPeriod)/listenInterval)*listenInterval);
        pFwConfig->ucBeaconFilterPeriod = filterPeriod ? filterPeriod : listenInterval;
    }

    pFwConfig->ucRssiFilterPeriod = pPeMsg->rssiFilterPeriod;
    if (pFwConfig->ucRssiFilterPeriod) {
        filterPeriod = (tANI_U8)(((pFwConfig->ucRssiFilterPeriod)/listenInterval)*listenInterval);
        pFwConfig->ucRssiFilterPeriod = filterPeriod ? filterPeriod : listenInterval;
    }

    if (pMac->hal.currentRfBand == eRF_BAND_2_4_GHZ) {
        pFwConfig->uAirTimeComp = TBTT_COMPENSATION_2_4_GHZ;
    } else if (pMac->hal.currentRfBand == eRF_BAND_5_GHZ) {
        pFwConfig->uAirTimeComp = TBTT_COMPENSATION_5_GHZ;
    }

    // INFRA STA keep alive enable/disable.
    if (pMac->hal.infraStaKeepAlivePeriod) {
        pFwConfig->bStaKeepAliveEn = TRUE;
        pFwConfig->ucStaKeepAlivePeriodSecs = pMac->hal.infraStaKeepAlivePeriod;
    } else {
        pFwConfig->bStaKeepAliveEn = FALSE;
        pFwConfig->ucStaKeepAlivePeriodSecs = 0;
    }

    HALLOGE( halLog(pMac, LOGE, FL("Updating LI in FW to %d"), listenInterval));

    // Update the DPU routing WQ in FW sys config.
    pFwConfig->ucDpuRoutingWq = (tANI_U8)BMUWQ_ADU_UMA_RX;

    pFwConfig->ucNumNoDwnLinkThres = pMac->hal.dynamicPsPollValue;
    pFwConfig->ucNumBeaconRssiAvg = pPeMsg->numBeaconPerRssiAverage;
    pFwConfig->bRssiFilterEnable = pPeMsg->bRssiFilterEnable;

    status = halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(*pFwConfig));

    return status;
}

/*
 * DESCRIPTION:
 *      This function would basically apply the TSF compensation,
 *      compute the last dtim TBTT from current TBTT provided by PE and
 *      also compute the least possible DTIM TBTT which is used as reference TBTT in HW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dtimPeriod: DTIM period as advertised for the BSS
 *      dtimCount: The current DTIM count in the beacon received.
 *      pLeastDtimTbtt: Pointer to the variable where least DTIM TBTT would be filled.
 *      pLastDtimTbtt: Pointer to the variable where last DTIM TBTT would be filled.
 *
 * RETURN:
 *      void
 */
void halPS_GetRefTbtt(tpAniSirGlobal pMac, tANI_U64 tbtt, tANI_U8 bssIdx,
        tANI_U8 dtimPeriod, tANI_U8 dtimCount, tANI_U64 *pLeastDtimTbtt, tANI_U64 *pLastDtimTbtt)
{
    tANI_U64 prevDtimTbtt;

    // Apply the compensation for TBTT
    if (pMac->hal.currentRfBand == eRF_BAND_2_4_GHZ) 
    {
        if(tbtt > TBTT_COMPENSATION_2_4_GHZ)
            tbtt -= TBTT_COMPENSATION_2_4_GHZ;
        else
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,"%s: %d: ERROR TBTT Low: %x TBTT High: %x\n",
                      __FUNCTION__, __LINE__,(&(tbtt))[0], (&(tbtt))[1]);
    } else if (pMac->hal.currentRfBand == eRF_BAND_5_GHZ) 
    {
        if(tbtt > TBTT_COMPENSATION_5_GHZ)
            tbtt -= TBTT_COMPENSATION_5_GHZ;
	    else
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,"%s: %d: ERROR TBTT Low: %x TBTT High: %x\n",
                      __FUNCTION__, __LINE__,(&(tbtt))[0], (&(tbtt))[1]);
    }

    HALLOG1(halLog(pMac, LOG1, FL("After compensation TBTT Low: %x TBTT High: %x\n"),
                        (&(tbtt))[0], (&(tbtt))[1]);)

    halUtil_GetDtimTbtt(pMac, tbtt, bssIdx, dtimPeriod, dtimCount, &prevDtimTbtt);
    *pLastDtimTbtt = prevDtimTbtt;
    halUtil_GetLeastRefDtimTbtt(pMac, bssIdx, prevDtimTbtt, pLeastDtimTbtt, dtimPeriod);

    return;
}


/* Beacon Mode Power Save (BMPS) functions */

/*
 * DESCRIPTION:
 *      Function to handle the Enter BMPS request from upper layer PE.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleEnterBmpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpEnterBmpsParams pPeMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    Qwlanfw_EnterBmpsReqType msg;
    tANI_U16 listenInterval = (tANI_U16)pHalPwrSave->listenInterval;
    tANI_U64 leastDtimTbtt = 0, lastDtimTbtt = 0;

    // Do not enter BMPS if listen interval is set to 0. This shouldn't happen.
    if (listenInterval == 0)
   	{
        HALLOGE( halLog(pMac, LOGE, FL("Inavlid Listen Interval %d  do not enter BMPS"), listenInterval));
		goto error;
    }

    // Load the ADU memory with the indirectly accessed register list
    status = halPS_RegBckupIndirectRegisters(pMac);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Load indirect register failed\n")));
        goto error;
    }

    // Set the FW system config for the re-init address location
    status = halFW_UpdateReInitRegListStartAddr(pMac, pBmpsCtx->aduMemAddr);

    // Set the response to be given back to PE, in case of timeout to get the
    // response back from FW, this rspType will be send to the upper layer
    pHalPwrSave->rspType = SIR_HAL_ENTER_BMPS_RSP;
    pHalPwrSave->dialogToken = dialogToken;

    // Update the DTIM period of the bss
    halTable_SetDtimPeriod(pMac, pPeMsg->bssIdx, pPeMsg->dtimPeriod);

    HALLOGE( halLog(pMac, LOGE, FL("DtimPeriod = %d, DtimCount = %d, Current beacon TBTT = %lu"),
        pPeMsg->dtimPeriod, pPeMsg->dtimCount, pPeMsg->tbtt));

#ifdef FEATURE_WLAN_DIAG_SUPPORT
    {
        WLAN_VOS_DIAG_EVENT_DEF(psRequest, vos_event_wlan_powersave_payload_type);
        vos_mem_zero(&psRequest, sizeof(vos_event_wlan_powersave_payload_type));
        psRequest.event_subtype = WLAN_BMPS_DTIM_PERIOD;
        psRequest.dtim_period = pPeMsg->dtimPeriod;
        WLAN_VOS_DIAG_EVENT_REPORT(&psRequest, EVENT_WLAN_POWERSAVE_GENERIC);
    }
#endif

    /*Configure Apps CPU to Wakeup State*/
    halPSAppsCpuWakeupState(pMac, TRUE);

    // Update power save related parameters in the FW sys config
    halPS_UpdateFwSysConfig(pMac, pPeMsg);

    // Compute the TSF of the DTIM beacon based on the current TSF and
    // the DTIM count
    halPS_GetRefTbtt(pMac, pPeMsg->tbtt, pPeMsg->bssIdx,
                pPeMsg->dtimPeriod, pPeMsg->dtimCount, &leastDtimTbtt, &lastDtimTbtt );

    vos_mem_free((v_VOID_t*)pPeMsg);
    pPeMsg = NULL;
    HALLOGE( halLog(pMac, LOGE, FL("DTIM beacon TBTT (Last/Least) = %lu/%lu"), lastDtimTbtt, leastDtimTbtt));

    msg.hwTimestampLo   = (tANI_U32) (leastDtimTbtt & 0xffffffffull);
    msg.hwTimestampHi   = (tANI_U32) (leastDtimTbtt >> 32);
    msg.timestampLo     = (tANI_U32) (lastDtimTbtt & 0xffffffffull);
    msg.timestampHi     = (tANI_U32) (lastDtimTbtt >> 32);

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        goto error;
    }

    // Send the ENTER_BMPS request to firmware
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE, QWLANFW_HOST2FW_ENTER_BMPS_REQ,
            dialogToken, sizeof(Qwlanfw_EnterBmpsReqType), &msg, TRUE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send BMPS request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }

        goto error;
    }
    //Change the state to requested state
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_BMPS_REQUESTED;

    return eHAL_STATUS_SUCCESS;

error:
    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_BMPS_RSP,
            pHalPwrSave->dialogToken, NULL, status);
    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle Enter BMPS response from the FW.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwEnterBmpsRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus  status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog( pMac, LOG1, FL("BMPS enter resp received by FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pHalPwrSave->dialogToken) {
        HALLOGE( tHalPsBmps  *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx);
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pBmpsCtx->token));
        // TODO: Recovery Mechanism required here
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

#if 0
    // Since this message is handled in the interrupt context, set the host busy
    halPS_SetHostBusy(pMac, HAL_PS_BUSY_INTR_CONTEXT);
#endif

    // Start Monitoring register writes
    halPS_StartMonitoringRegAccess(pMac);

    // Set the state of power save, now in BMPS
    // Also set the mutex protection for graceful shutdown
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_BMPS_STATE;
    pHalPwrSave->pwrSaveState.p.mutexProtect = TRUE;

respond:
    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_BMPS_RSP,
            pHalPwrSave->dialogToken, NULL, fwStatus);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle Exit BMPS request from the upper layer PE.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleExitBmpsReq(tpAniSirGlobal pMac, tANI_U16 dialogToken,
        tpExitBmpsParams pPeMsg)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    Qwlanfw_ExitBmpsReqType msg;

#if 0
    // NOTE: if the system is in suspend BMPS state, we need to wait till BMPS is
    // resumed as BMPS could have been suspended as part of SCAN or CALIBRATION.
    // So we defer the EXIT BMPS request till BMPS state is resumed.
    //
    //
    // If system is in SUSPEND_BMPS state, resume the chip back to
    // BMPS state before sending the exit request
    if (pHalPwrSave->pwrSaveState.psState & HAL_PWR_SAVE_SUSPEND_BMPS_STATE) {
        halPS_ResumeBmps(pMac, 0);  //TODO:
        // Post the EXIT BMPS message back to HAL till the state is back to BMPS
    }
#endif

    // Check if system is in BMPS state
    if (!(pHalPwrSave->pwrSaveState.p.psState & HAL_PWR_SAVE_BMPS_STATE)) {
        HALLOGE( halLog(pMac, LOGE, FL("System not in BMPS state\n")));
        status = eHAL_STATUS_FAILURE;
        goto error;
    }

    // Set the response to be given back to PE, in case of timeout to get the
    // response back from FW, this rspType will be send to the upper layer
    pHalPwrSave->rspType = SIR_HAL_EXIT_BMPS_RSP;
    pHalPwrSave->dialogToken = dialogToken;

    // Inform FW whether to send Data Null on exiting BMPS
    msg.bExitDot11PwrSave = pPeMsg->sendDataNull;

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        goto error;
    }

    // Send the EXIT_BMPS request to firmware
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE, QWLANFW_HOST2FW_EXIT_BMPS_REQ,
            dialogToken, sizeof(Qwlanfw_ExitBmpsReqType), &msg, TRUE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send BMPS request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }

        goto error;
    }

    // Free the memory allocated in the request message
    palFreeMemory(pMac->hHdd, pPeMsg);

    return eHAL_STATUS_SUCCESS;

error:

    // Send the status back to PE in the message
    pPeMsg->status = status;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_BMPS_RSP,
            pHalPwrSave->dialogToken, pPeMsg, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Exit IMPS response from the FW.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwExitBmpsRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tpExitBmpsParams pPeMsg = NULL;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE, FL("VOS timer stop failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog( pMac, LOG1, FL("BMPS exit received from FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pHalPwrSave->dialogToken) {
        HALLOGE( tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx);
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pBmpsCtx->token));
        // TODO: Recovery Mechanism required here
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if ((fwStatus != QWLANFW_STATUS_SUCCESS) &&
        (fwStatus != QWLANFW_STATUS_BMPS_NO_REPLY_TO_NULLFRAME_FROM_AP)){
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

respond:

    // Set the power state to active, release the mutex proctection
    pHalPwrSave->pwrSaveState.p.psState = HAL_PWR_SAVE_ACTIVE_STATE;
    pHalPwrSave->pwrSaveState.p.mutexProtect = FALSE;

    // Stop Monitoring register writes
    halPS_StopMonitoringRegAccess(pMac);

    // Allocate the memory for the response message to be sent
    palAllocateMemory(pMac->hHdd, (void **)&pPeMsg, sizeof(tExitBmpsParams));
    if(pPeMsg == NULL)
	    return eHAL_STATUS_FAILURE;

    // Fill the status in the message
    pPeMsg->status = status;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_BMPS_RSP,
            pHalPwrSave->dialogToken, pPeMsg, fwStatus);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to Suspend BMPS
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *      cbFunc: Function pointer to the callback function to be called after
 *              suspend BMPS is done.
 *      data:   Pointer to the data to be passed to the callback function
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SuspendBmps(tpAniSirGlobal pMac, tANI_U16 dialogToken,
      funcHalPsCB cbFunc, void* data)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    Qwlanfw_SuspendBmpsReqType msg;
    pBmpsCtx->token = dialogToken;

    HALLOGE( halLog(pMac, LOGE, FL("Suspending FW from BMPS")));

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        return status;
    }

    pHalPwrSave->rspType = SIR_HAL_SUSPEND_BMPS;
    pHalPwrSave->dialogToken = dialogToken;

    // Store the callback function pointer and the data
    pHalPwrSave->psCbFunc = cbFunc;
    pHalPwrSave->psCbData = data;

    // Send the EXIT_BMPS request to firmware
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_SUSPEND_BMPS_REQ, dialogToken, sizeof(Qwlanfw_SuspendBmpsReqType), &msg, TRUE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send BMPS request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }
        return status;
    }
    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle the Suspend BMPS response from the FW.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwSuspendBmpsRsp(tpAniSirGlobal pMac, void *pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsBmps  *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE,
                FL("VOS Timer stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog(pMac, LOG1, FL("Suspend BMPS resp received from FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pBmpsCtx->token) {
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pBmpsCtx->token));
        return eHAL_STATUS_FAILURE;
    }

    // Check the FW status of the response message
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

    // Set the state of power save, now in BMPS
    // Also set the mutex protection for graceful shutdown
    pHalPwrSave->pwrSaveState.p.mutexProtect = FALSE;
    pHalPwrSave->pwrSaveState.p.psState |= HAL_PWR_SAVE_SUSPEND_BMPS_STATE;

    // Call the callback function
    if (pHalPwrSave->psCbFunc != NULL) {
        pHalPwrSave->psCbFunc(pMac, pHalPwrSave->psCbData,
                pBmpsCtx->token, status);
    }

    return status;
}


/*
 * DESCRIPTION:
 *      Function to Resume BMPS from the suspended state
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *      cbFunc: Function pointer to the callback function to be called after
 *              resume BMPS is done.
 *      data:   Pointer to the data to be passed to the callback function
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_ResumeBmps(tpAniSirGlobal pMac, tANI_U16 dialogToken,
      funcHalPsCB cbFunc, void* data, tANI_U8 rspReq)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    Qwlanfw_ResumeBmpsReqType msg;
    pBmpsCtx->token = dialogToken;

    HALLOGE( halLog(pMac, LOGE, FL("Resuming FW back to BMPS after suspending")));

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    if (rspReq) {
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        return eHAL_STATUS_TIMER_START_FAILED;
    }
    }

    pHalPwrSave->rspType = SIR_HAL_RESUME_BMPS;
    pHalPwrSave->dialogToken = dialogToken;

    // Store the callback function pointer and the data
    pHalPwrSave->psCbFunc = cbFunc;
    pHalPwrSave->psCbData = data;

    // Send the RESUME_BMPS request to firmware
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_RESUME_BMPS_REQ, dialogToken, sizeof(Qwlanfw_ResumeBmpsReqType), &msg, TRUE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send BMPS request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;

        if (rspReq) {
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }
        }
        return status;
    }

    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle the Resume BMPS response from FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwResumeBmpsRsp(tpAniSirGlobal pMac, void *pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE,
                FL("VOS Timer Stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog(pMac, LOG1, FL("Resume BMPS resp received by FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pBmpsCtx->token) {
        HALLOGE( halLog( pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pBmpsCtx->token));
        // TODO: Recovery Mechanism required here
        return eHAL_STATUS_FAILURE;
    }

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

    // Set the state of power save to out of suspend state
    // Also set the mutex protection for graceful shutdown
    pHalPwrSave->pwrSaveState.p.mutexProtect = TRUE;
    pHalPwrSave->pwrSaveState.p.psState &= ~HAL_PWR_SAVE_SUSPEND_BMPS_STATE;

    // Call the callback function
    if (pHalPwrSave->psCbFunc != NULL) {
        pHalPwrSave->psCbFunc(pMac, pHalPwrSave->psCbData,
                pBmpsCtx->token, status);
    }

    return status;
}

/*
 * DESCRIPTION:
 *      Function to handle the BMPS status message from FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwBmpsStatusMsg(tpAniSirGlobal pMac, void* pFwMsg)
{
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    Qwlanfw_StatusMsgType *pFwStatusMsg = (Qwlanfw_StatusMsgType*)pFwMsg;
    HALLOGE( tANI_U32 reasonCode = pFwStatusMsg->uStatus);

    // If host is  already in BMPS state when we receive this message,
    // then FW is indicating some failure state when trying to put
    // itself into BMPS. In that case, send the failure message to
    // upper layer PE.
    if (!((pHalPwrSave->pwrSaveState.p.psState & HAL_PWR_SAVE_BMPS_STATE) ||
            (pHalPwrSave->pwrSaveState.p.psState & HAL_PWR_SAVE_UAPSD_STATE))) {
        HALLOGE( halLog( pMac, LOGE,
                    FL("Host recvd FW BMPS failure msg when not in BMPS state\n")));
        return eHAL_STATUS_FAILURE;
    }

    HALLOGE( halLog( pMac, LOGE,
                FL("Host recvd FW BMPS failure msg, status = %d\n"), reasonCode));

     // Check if the FW has more info along with the status
    if(pFwStatusMsg->hdr.usMsgLen >  sizeof(Qwlanfw_StatusMsgType)) {
        StatusMsgInfo *pStatusMsg = (StatusMsgInfo *)((tANI_U8*)(pFwMsg) + sizeof(Qwlanfw_StatusMsgType));

        switch (pStatusMsg->uStatusCode) {
            case QWLANFW_STATUS_BMPS_BDPDU_NOT_IDLE_FAILURE:
                {
                    HALLOGE( BdPduNotIdleInfo *pInfo = (BdPduNotIdleInfo *)(pStatusMsg->aStatusInfo));
                    HALLOGE( halLog( pMac, LOGE,
                                FL("**BD PDU NOT IDLE FAILURE: AvailBds %x\n**"),
                                pInfo->uIdleBds));
                }
                break;
            case QWLANFW_STATUS_BMPS_SIF_FREEZE_FAILURE:
                {
                    HALLOGE( SifFreezeInfo *pInfo = (SifFreezeInfo *)(pStatusMsg->aStatusInfo));
                    HALLOGE( halLog( pMac, LOGE,
                                FL("**SIF FREEZE FAILURE: Intr %x BmuSifDataLen %x**\n"),
                                pInfo->uMacIntr, pInfo->uSifRxFifoData));
                }
                break;
            case QWLANFW_STATUS_BMPS_BTQM_QUEUES_NOT_EMPTY_FAILURE:
                HALLOGE( halLog( pMac, LOGE, FL("**Btqm Queues not Empty!!!**\n")));
                break;
        }

    }

    return eHAL_STATUS_SUCCESS;
}



/* Unscheduled Automatic Power Save Delivery (UAPSD) functions */

/*
 * DESCRIPTION:
 *      Function to handle the Enter UAPSD request from the upper layer PE.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleEnterUapsdReq(tpAniSirGlobal pMac, tANI_U16 dialogToken,
        tpUapsdParams pPeMsg)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsUapsd *pUapsdCtx = &pMac->hal.PsParam.UapsdCtx;
    tHalPsBmps  *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    Qwlanfw_EnterUapsdType msg;

    // Check if system is in BMPS state
    if (!(pHalPwrSave->pwrSaveState.p.psState & HAL_PWR_SAVE_BMPS_STATE)) {
        HALLOGE( halLog(pMac, LOGE, FL("System not in BMPS state\n")));
        status = eHAL_STATUS_FAILURE;
        goto error;
    }

    // Load the ADU memory with the indirectly accessed register list
    status = halPS_RegBckupIndirectRegisters(pMac);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Load indirect register failed\n")));
        goto error;
    }

#if 0 // Since FW is setting the ADU reinit address in PMU, host should not touch it.
      // Host provides this address in the sysConfig from where FW programs it in PMU
    // Program the ADU re-init memory address pointer in the PMU register
    halPmu_SetAduReInitAddress(pMac, pUapsdCtx->aduMemAddr);
#endif

    // Set the FW system config for the re-init address location
    status = halFW_UpdateReInitRegListStartAddr(pMac, pUapsdCtx->aduMemAddr);

    pHalPwrSave->rspType = SIR_HAL_ENTER_UAPSD_RSP;
    pHalPwrSave->dialogToken = dialogToken;

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        goto error;
    }

    // Copy the parameter from the PE to the FW message body
    msg.bBcAcDeliveryEnable = pPeMsg->bkDeliveryEnabled;
    msg.bBeAcDeliveryEnable = pPeMsg->beDeliveryEnabled;
    msg.bViAcDeliveryEnable = pPeMsg->viDeliveryEnabled;
    msg.bVoAcDeliveryEnable = pPeMsg->voDeliveryEnabled;
    msg.bBcAcTriggerEnable  = pPeMsg->bkTriggerEnabled;
    msg.bBeAcTriggerEnable  = pPeMsg->beTriggerEnabled;
    msg.bViAcTriggerEnable  = pPeMsg->viTriggerEnabled;
    msg.bVoAcTriggerEnable  = pPeMsg->voTriggerEnabled;

    // Send the ENTER_APSD request to firmware
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_ENTER_UAPSD_REQ, dialogToken, sizeof(Qwlanfw_EnterUapsdType), &msg, FALSE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send ENTER UAPSD request msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;

        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }

        goto error;
    }

    // Free the memory allocated in the request message
    palFreeMemory(pMac->hHdd, pPeMsg);

    return eHAL_STATUS_SUCCESS;

error:
#if 0 // Since FW is setting the ADU reinit address in PMU, host should not touch it.
      // Host provides this address in the sysConfig from where FW programs it in PMU
    // Program the ADU re-init memory address pointer in the PMU register
    halPmu_SetAduReInitAddress(pMac, pBmpsCtx->aduMemAddr);
#endif

    // Restore the FW system config for the re-init address location
    // pointing to BMPS
    halFW_UpdateReInitRegListStartAddr(pMac, pBmpsCtx->aduMemAddr);

    pPeMsg->status = status;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_UAPSD_RSP,
            pHalPwrSave->dialogToken, pPeMsg, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Enter UAPSD response from FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwEnterUapsdRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tpUapsdParams pPeMsg = NULL;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE,
                FL("VOS Timer Stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_FAILURE;
    }

    HALLOG1( halLog(pMac, LOG1, FL("Enter UAPSD resp received by FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pHalPwrSave->dialogToken) {
        HALLOGE( tHalPsUapsd *pUapsdCtx = &pMac->hal.PsParam.UapsdCtx);
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pUapsdCtx->token));
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
    // Set the DPU routing flag to the FW WQ, all the TX frames would be now pushed
    // from DPU to the FW-WQ (5) in UAPSD. FW would be in data path, monitoring
    // the traffic to decide when to suspend the trigger frames when there is no traffic
    // activity on the trigger enabled ACs
    pMac->hal.halMac.dpuRF = BMUWQ_FW_DPU_TX;

#ifdef WLAN_PERF
    // Increment the BD signature to refresh the fast path BD utilization
    pMac->hal.halMac.uBdSigSerialNum++;
#endif
#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

    // Set the power state to actitve, release the mutex proctection
    pHalPwrSave->pwrSaveState.p.psState |= HAL_PWR_SAVE_UAPSD_STATE;

respond:

    // Allocate the memory for the response message to be sent
    palAllocateMemory(pMac->hHdd, (void **)&pPeMsg, sizeof(tUapsdParams));
    if(pPeMsg == NULL)
	    return eHAL_STATUS_FAILURE;

    // Fill the status in the message
    pPeMsg->status = status;

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_ENTER_UAPSD_RSP,
            pHalPwrSave->dialogToken, pPeMsg, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Exit UAPSD request from the upper layer PE
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleExitUapsdReq(tpAniSirGlobal pMac, tANI_U16 dialogToken)
{
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPsUapsd *pUapsdCtx = &pMac->hal.PsParam.UapsdCtx;
    tHalPsBmps *pBmpsCtx = &pMac->hal.PsParam.BmpsCtx;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    Qwlanfw_ExitUpasdType msg;
    // Check if system is in BMPS state
    if (!(pHalPwrSave->pwrSaveState.p.psState & HAL_PWR_SAVE_UAPSD_STATE)) {
        HALLOGE( halLog(pMac, LOGE, FL("System not in UAPSD state\n")));
        goto error;
    }

    /* FIXME_GEN6: RF registers should be reloaded into the ADU reinit
     * memory after exiting from UAPSD, as RF would be powered down in BMPS
     * but in UAPSD RF is always on */
#if 0
    // Load the ADU memory with the indirectly accessed register list
    status = halPS_RegBckupIndirectRegisters(pMac);
    if (eHAL_STATUS_SUCCESS != status) {
        HALLOGP( halLog(pMac, LOGP, FL("Load indirect register failed\n")));
        goto error;
    }

    // Program the ADU re-init memory address pointer in the PMU register
    halPmu_SetAduReInitAddress(pMac, pBmpsCtx->aduMemAddr);
#endif

    // Restore the FW system config for the re-init address location
    // pointing to BMPS
    halFW_UpdateReInitRegListStartAddr(pMac, pBmpsCtx->aduMemAddr);

    pHalPwrSave->rspType = SIR_HAL_EXIT_UAPSD_RSP;
    pHalPwrSave->dialogToken = dialogToken;

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
#ifdef UAPSD_OOO_FIX
    // TODO: Suspend any TX activities.
    if((status = halTLSuspendTx(pMac, NULL)) != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("TL failed in suspending TX queue for all STA")));
        goto error;
    }
    // TODO: Send special frame to FW
    // status = halTLSend80211Frame();
#endif
#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, FL("VOS Timer start failed - status = %d\n"),
                vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        goto error;
    }

    // Send the EXIT_UAPSD request to firmware, and waits for the
    // response message to be received
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_EXIT_UAPSD_REQ, dialogToken, sizeof(Qwlanfw_ExitUpasdType), &msg, FALSE, NULL);
    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW Send Exit UAPSD Req msg failed\n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;

        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }
        goto error;
    }

    return eHAL_STATUS_SUCCESS;

error:
#if 0 // Since FW is setting the ADU reinit address in PMU, host should not touch it.
      // Host provides this address in the sysConfig from where FW programs it in PMU
    // Program the ADU re-init memory address pointer in the PMU register
    halPmu_SetAduReInitAddress(pMac, pUapsdCtx->aduMemAddr);
#endif

    // Set the FW system config for the re-init address location
    halFW_UpdateReInitRegListStartAddr(pMac, pUapsdCtx->aduMemAddr);

    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_UAPSD_RSP,
            pHalPwrSave->dialogToken, NULL, status);
    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the Exit UAPSD response from the FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwExitUapsdRsp(tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog(pMac, LOGE,
                FL("VOS Timer Stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog(pMac, LOG1, FL("Exit UAPSD resp received by FW, status = %d\n"), fwStatus));

    // Check the token :)
    if (pMbMsg->MsgSerialNum != pHalPwrSave->dialogToken) {
        HALLOGE( tHalPsUapsd *pUapsdCtx = &pMac->hal.PsParam.UapsdCtx);
        HALLOGE( halLog(pMac, LOGE, FL("Rsp Message %d is stale, Current token = %d\n"),
                pMbMsg->MsgSerialNum, pUapsdCtx->token));
        status = eHAL_STATUS_FAILURE;
        goto respond;
    }

    // Check the FW status of the response message, if failure then
    // send response back to PE
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
//        goto respond;
    }

#ifdef FEATURE_WLAN_UAPSD_FW_TRG_FRAMES
    // Restore back the DPU routing flag in the TxBD, for DPU to push the TxBDs to BTQM
    // directly instead of the FW WQ.
    pMac->hal.halMac.dpuRF = BMUWQ_BTQM_TX_MGMT;

#ifdef WLAN_PERF
    // Increment the BD signature to refresh the fast path BD utilization
    pMac->hal.halMac.uBdSigSerialNum++;
#endif

#ifdef UAPSD_OOO_FIX
    // TODO: Resume TL TX
    status = halTLResumeTx(pMac, NULL);
    // Resume the transmission in TL, NULL specifies resume all STA
    if( status  != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE, FL(" TL failed resuming Tx queue for all STAs")));
    }
#endif

#endif //FEATURE_WLAN_UAPSD_FW_TRG_FRAMES

    // Clear the UPASD bit in the power state
    pHalPwrSave->pwrSaveState.p.psState &= ~HAL_PWR_SAVE_UAPSD_STATE;

respond:
    // Send Response message back to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_EXIT_UAPSD_RSP,
            pHalPwrSave->dialogToken, NULL, status);
    return status;
}



/* Beacon Filtering */
/*
 * DESCRIPTION:
 *      Function to handle the Beacon filtering request from the upper layer PE.
 *      PE provides with the beacon filter pattern
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *      pMsg:   Pointer to the beacon filter pattern
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleAddBeaconFilter(tpAniSirGlobal pMac,
        tANI_U16 dialogToken, void *pMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tBeaconFilterMsg beaconFilter;
    tBeaconFilterIe *pFilterIe = NULL;
    Qwlanfw_AddBcnFilterMsgType *pAddBcnFilterMsg = NULL;
    Qwlanfw_BcnIeChangeFilterType *pBcnIeFilter = NULL;
    Qwlanfw_BcnIeByteType *pByte;
    tANI_U16 i, j;
    tANI_S16 k;
    tANI_U8 *pBuf, *pBuffer;
    tANI_U16 msgLen = 0;
    tANI_U16 filterLen;
    tANI_U8 duplicateIe = FALSE, elementId;

    // Copy the beacon filter parameters from the message
    palCopyMemory(pMac->hHdd, &beaconFilter, pMsg,
            sizeof(beaconFilter));

    // Allocate memory for sending the message down to FW
    palAllocateMemory(pMac->hHdd, (void**)&pBuffer,
            sizeof(Qwlanfw_AddBcnFilterMsgType) +
            (sizeof(*pFilterIe)*beaconFilter.ieNum) +
            sizeof(beaconFilter) );

    pAddBcnFilterMsg = (Qwlanfw_AddBcnFilterMsgType *)pBuffer;
    if(pAddBcnFilterMsg == NULL)
	    return eHAL_STATUS_FAILURE;

    // Initialize the pointer to point to the start of IEs in th msg
    pFilterIe = (tpBeaconFilterIe)((tANI_U8*)pMsg +
            sizeof(beaconFilter));

    // Copy the capability info, mask and the beacon Interval in to the buffer
    pAddBcnFilterMsg->usCapInfoFieldValue = beaconFilter.capabilityInfo;
    pAddBcnFilterMsg->usCapInfoFieldMask = beaconFilter.capabilityMask;
    pAddBcnFilterMsg->bcninterval = beaconFilter.beaconInterval;

    pBuf = pBuffer + sizeof(Qwlanfw_AddBcnFilterMsgType);


    for (i=0; i<beaconFilter.ieNum; i++) {
        elementId = pFilterIe[i].elementId;

        // Check for previous entry of similar EID
        if(i>0) {
            for (k=i-1; k>=0; k--) {
                if (elementId == pFilterIe[k].elementId) {
                    duplicateIe = TRUE;
                    break;
                }
            }
        }

        // If similar EID was found, move on to next entry
        if (duplicateIe) {
            duplicateIe = FALSE;
            continue;
        }

        pBcnIeFilter = (Qwlanfw_BcnIeChangeFilterType *)pBuf;
        pBcnIeFilter->ucIeElementId = elementId;
        pBcnIeFilter->usFilterLength = 0;
        filterLen = 0;

        pBuf += sizeof(Qwlanfw_BcnIeChangeFilterType);

        // Check for similar element ID
        for (j=i; j<beaconFilter.ieNum; j++) {
            if (elementId == pFilterIe[j].elementId) {
                if (!pFilterIe[j].checkIePresence) {
                    pByte = (Qwlanfw_BcnIeByteType *)pBuf;
                    pByte->ucByteOffset = pFilterIe[j].byte.offset;
                    pByte->ucByteValue = pFilterIe[j].byte.value;
                    pByte->ucBitMask = pFilterIe[j].byte.bitMask;
                    pByte->ucByteRef = pFilterIe[j].byte.ref;

                    // Increment the buffer pointer
                    pBuf += sizeof(pFilterIe[j].byte);

                    // Increment the length
                    filterLen += sizeof(pFilterIe[j].byte);
                }
            }
        }
        pBcnIeFilter->usFilterLength = filterLen;
    }

    msgLen = (tANI_U16)(pBuf - pBuffer);

    for(i = sizeof(Qwlanfw_AddBcnFilterMsgType);
        i<(sizeof(*pFilterIe)*beaconFilter.ieNum) + sizeof(beaconFilter); i++) {
       HALLOGE( halLog(  pMac, LOGE, FL("0x%02x "),  pBuffer[i] ));
    }

    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_ADD_BEACON_FILTER, dialogToken, msgLen,
            pBuffer, FALSE, NULL);

    // Free the memory
    palFreeMemory(pMac->hHdd, pBuffer);
    vos_mem_free((v_VOID_t*)pMsg);
    pMsg = NULL;

    return status;
}


/*
 * DESCRIPTION:
 *      Function to handle the request for removal of beacon filter from upper
 *      layer PE
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *      pMsg:   Pointer to Beacon filter pattern
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleRemBeaconFilter(tpAniSirGlobal pMac,
        tANI_U16 dialogToken, tANI_U8 *pucRemIeId, tANI_U8 ucIeCount)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    Qwlanfw_RemBcnFilterMsgType msg;

    if(ucIeCount > QWLANFW_MAX_BCN_FILTER || !pucRemIeId)
        return status;

    // TODO: PE does not have this feature for now implemented,
    // but the support for removing beacon filter exists between
    // HAL and FW. This function can be called whenever PE defines
    // a new message for beacon filter removal
    msg.ucNumIds = ucIeCount;
    palCopyMemory(pMac->hHdd, &msg.ucRemIeId[0], pucRemIeId, ucIeCount);

    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE,
            QWLANFW_HOST2FW_REM_BEACON_FILTER, dialogToken, sizeof(Qwlanfw_RemBcnFilterMsgType),
            &msg, FALSE, NULL);

    return status;
}

/* Low RSSI Indication */
/*
 * DESCRIPTION:
 *      Function to handle the Low RSSI indication from FW and informing to the
 *      upper layer PE regarding the same with a message
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SendLowRssiInd(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;

    //TODO: Get the status
    halMsg_GenerateRsp(pMac, SIR_HAL_LOW_RSSI_IND, 0, NULL, status);

    return status;
}

/* Miss Beacon Indication */
/*
 * DESCRIPTION:
 *      Function to handle the Missed beacons indication from FW, and informing
 *      to the upper layer PE regarding the same with a message
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SendBeaconMissInd(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;

    HALLOGE(halLog(pMac, LOGE, "HAL received Missed beacon indication from FW"));
    //TODO: Get the status
    halMsg_GenerateRsp(pMac, SIR_HAL_MISSED_BEACON_IND, 0, NULL, status);

    return status;
}


/* Mutex */
/*
 * DESCRIPTION:
 *      Function to set the host busy by acquiring a mutex. This function is
 *      called during any register access when the FW is in BMPS mode to
 *      acheive synchronization and mutual exclusion
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      ctx:    Context in which the mutex is acquired, interrupt context or
 *              normal context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_SetHostBusy(tpAniSirGlobal pMac, tANI_U8 ctx)
{
    tANI_U32 regValue = 0, curCnt = 0, retryCnt = 0;
    tANI_U32 index;
    eHalStatus mutexAcq = eHAL_STATUS_FW_PS_BUSY;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 maxCnt = 0;

#ifdef WLAN_SDIO_DUMMY_CMD53_WORKAROUND
    // Dummy write to the HW register
    palWriteRegister(pMac->hHdd, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_REG, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_DEFAULT);
#endif

    /***********************************************************************
     * Please note that the below logic is executed by 3 different contexts.
     * MC Thread, TX Thread and INTR Context. The contexts are differentiated
     * by using 3 different global variables, 1 for each context. But still 
     * the Mutex1 global variable is shared by all the 3 contexts. It is 
     * assumed that each context will acquire only one MUTEX1 variable and 
     * release it appropriately. Hence there is no spin lock used in this 
     * function. If any other changes are done here which requires accessing
     * any other global variable(s), this function should be made re-entrant
     * by adding appropriate lock at the entry and exit for both SetHostBusy
     * and ReleaseHostBusy functions.
     * ********************************************************************/

#ifdef FEATURE_WLAN_VOLANS_1_0_PWRSAVE_WORKAROUND
    /* FIXME: This change is required to address SIF issues in Power-save case.
     * The SIF is using Rate-Matching FIFO even in the SIF-Freeze case which is
     * causing DxE errors in high SD frequencies on 7x30 platform.
     */
    {
        tANI_U32 uRegValue, mutexFailCnt = 0;

        do {
            /* Acquire Mutex1 here */
            status = palReadRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_TX_SYNC_ADDR, &uRegValue);
            maxCnt = (uRegValue & QWLAN_MCU_MUTEX1_MAXCOUNT_MASK) >> QWLAN_MCU_MUTEX1_MAXCOUNT_OFFSET;
            if(maxCnt != QWLAN_HOSTFW_TX_SYNC_MUTEX_MAX_COUNT)
            {
                mutexFailCnt++;
            }
            else
            {
                curCnt = (uRegValue & QWLAN_MCU_MUTEX1_CURRENTCOUNT_MASK) >> QWLAN_MCU_MUTEX1_CURRENTCOUNT_OFFSET;
                if(curCnt)
                    break;
            }

            if(!VOS_IS_STATUS_SUCCESS(status) && vos_is_logp_in_progress(VOS_MODULE_ID_HAL, NULL)) 
            {
                VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s: LOGP in progress.", __func__);
                return VOS_STATUS_E_FAILURE;
            }

        } while(retryCnt++ < 300 );

        if (0 == curCnt)
        {
            /* LOGP if mutex is not acquired */
            VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s: MUTEX1 Not Acquired after 300 tries, regVal = %d", __func__, regValue);
            macSysResetReq(pMac, eSIR_PS_MUTEX_READ_EXCEPTION);
            return VOS_STATUS_E_FAILURE;
        }    

        /* Release the mutex for the counts whenever we read the value 0 from register */
        uRegValue = (1 << QWLAN_MCU_MUTEX1_MAXCOUNT_OFFSET);
        while (mutexFailCnt)
        {
            status = palWriteRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_TX_SYNC_ADDR, uRegValue);
            mutexFailCnt--;

            if(!VOS_IS_STATUS_SUCCESS(status) && vos_is_logp_in_progress(VOS_MODULE_ID_HAL, NULL)) 
            {
                VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s: LOGP in progress.", __func__);
                return VOS_STATUS_E_FAILURE;
            }
        }
        uRegValue = 0;

        /* Tracking the protection for device writes */
        pMac->hal.PsParam.mutexTxCount++;

        /* Once mutex1 is acquired, touch BPS_REQ bit in PMU that would unfreeze SIF for Register-FIFO acesses
         * It is necessary that host acquire the mutex here and not inside the DO-WHILE Loop. This addresses the
         * corner case in which firmware acquires both mutex counts of MUTEX1 and issues SIF_FREEZE request.
         * And SIF_FREEZE is going on. In the mean time, the host has something to tranfer and attempts to acquire
         * the mutex but won't be able to. And as part of polling, host issues SIF_UNFREEZE request while firmware
         * initiated SIF_FREEZE request is happening. In other words, SIF state is not synchronized between host
         * and firmware. Which can cause unpleasant results. With this, the host issue SIF_UNFREEZE request if-and-only-if
         * it gets hold of mutex for SIF Register-FIFO access
         */
        palWriteRegister(pMac->hHdd, QWLAN_PMU_PMU_CLIENT_IF_BPS_REQ_CTRL_REG_REG, 0x0);
    }
#endif
    retryCnt = 0;
    while((eHAL_STATUS_FW_PS_BUSY == mutexAcq)&& (retryCnt < 3)) 
    {
        status = palReadRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_SYNC_ADDR, &regValue);
        if(eHAL_STATUS_SUCCESS != status)
            break;
        maxCnt = (regValue & QWLAN_MCU_MUTEX0_MAXCOUNT_MASK) >> QWLAN_MCU_MUTEX0_MAXCOUNT_OFFSET;
        if(maxCnt != QWLAN_HOSTFW_SYNC_MUTEX_MAX_COUNT)
        {
            curCnt = 0;
        }
        else
        {
            curCnt = (regValue & QWLAN_MCU_MUTEX0_CURRENTCOUNT_MASK) >> QWLAN_MCU_MUTEX0_CURRENTCOUNT_OFFSET;
        }
        if(curCnt <= 1) 
        {
            VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "Not Acquired = %d,%d, %x", pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, regValue);
#ifdef WLAN_DBG_GPIO
            VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "Mutex register value read 0, asserting GPIO HIGH \n");
            gpio_tlmm_config(GPIO_CFG(181, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
#endif
            ++retryCnt;
            VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s Mutex register value read 0, %d times", 
                    __FUNCTION__, retryCnt);

            if(retryCnt == 2) 
            {
                VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "%s retryCnt = %d Sleeping for 200ms before next retry", 
                        __FUNCTION__, retryCnt);
                vos_sleep(200);
            }            	
        }
        else {
            mutexAcq = eHAL_STATUS_SUCCESS;
        }
    }

    if(eHAL_STATUS_FW_PS_BUSY == mutexAcq)
    {
#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
        VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "Context: %d - Host Busy Mutex is not Acquired = %d,%d, %x", ctx, 
                pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, pMac->hal.PsParam.mutexTxRxCount, regValue);
#else
        VOS_TRACE( VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_FATAL, "Context: %d - Host Busy Mutex is not Acquired = %d,%d, %x", ctx, 
                pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, regValue);
#endif
        macSysResetReq(pMac, eSIR_PS_MUTEX_READ_EXCEPTION);
    } else {
        if (retryCnt > 0) {
            regValue = 0;
            regValue =  (1 << QWLAN_MCU_MUTEX0_MAXCOUNT_OFFSET);

            // Release the mutex, acquired as a result of retries
            for (index=0; index<retryCnt; index++) {
                palWriteRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_SYNC_ADDR, regValue);
            }
        }
    }

    if (ctx == HAL_PS_BUSY_GENERIC) {
        pMac->hal.PsParam.mutexCount++;
    } 
#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
    else if (ctx == HAL_PS_BUSY_TXRX_CONTEXT)
    {
        pMac->hal.PsParam.mutexTxRxCount++;
    }
#endif
    else
    {
        pMac->hal.PsParam.mutexIntrCount++;
    }

    if (pMac->hal.PsParam.enableMutexDebug)
    {
#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
        HALLOGW(halLog(pMac, LOGE, "Acquired = %d,%d,%d,%d, %x", pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, 
                    pMac->hal.PsParam.mutexTxRxCount, pMac->hal.PsParam.mutexTxCount, regValue));
#else
        HALLOGW(halLog(pMac, LOGE, "Acquired = %d,%d, %x", pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, regValue));
#endif
    }
    return eHAL_STATUS_SUCCESS;
}


/*
 * DESCRIPTION:
 *      Function to release the host from busy by releasing the mutex, should
 *      be called after the host has finished with any register access when in
 *      BMPS state.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      ctx:    Context in which the mutex is acquired, interrupt context or
 *              normal context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_ReleaseHostBusy(tpAniSirGlobal pMac, tANI_U8 ctx)
{
    tANI_U32 regValue = 0;

    regValue =  (1 << QWLAN_MCU_MUTEX0_MAXCOUNT_OFFSET);
    palWriteRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_SYNC_ADDR, regValue);

    if (ctx == HAL_PS_BUSY_GENERIC) {
        pMac->hal.PsParam.mutexCount--;
    } 
 #ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
    else if (ctx == HAL_PS_BUSY_TXRX_CONTEXT)
    {
        pMac->hal.PsParam.mutexTxRxCount--;
    }
#endif
    else {
        pMac->hal.PsParam.mutexIntrCount--;
    }

#ifdef FEATURE_WLAN_VOLANS_1_0_PWRSAVE_WORKAROUND
    {
	    /* FIXME: Release the mutex after SIF Register-FIFO Access */

	    v_U32_t uRegValue = (1 << QWLAN_MCU_MUTEX1_MAXCOUNT_OFFSET);
	    palWriteRegister(pMac->hHdd, QWLAN_MCU_MUTEX_HOSTFW_TX_SYNC_ADDR, uRegValue);
        pMac->hal.PsParam.mutexTxCount--;
    }
#endif

    if (pMac->hal.PsParam.enableMutexDebug)
    {
#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
        HALLOGW(halLog(pMac, LOGE, "Released = %d,%d,%d, %d", pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, 
                                                    pMac->hal.PsParam.mutexTxRxCount, pMac->hal.PsParam.mutexTxCount));
#else
        HALLOGW(halLog(pMac, LOGE, "Released = %d,%d,%d", pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, pMac->hal.PsParam.mutexTxCount));
#endif
    }
    return eHAL_STATUS_SUCCESS;
}

/*
 * DESCRIPTION:
 *      Function to control the firmware behaviour of turning off the power
 *      from the chip during power save. If when set FW would cut off the power
 *      from the chip and vice versa.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      enable: True or false,
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_CtrlChipPowerDown(tpAniSirGlobal pMac, tANI_U8 enable)
{
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    pFwConfig->bNoPwrDown = enable ? 1 : 0;

    return halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr, (tANI_U8 *)pFwConfig,
            sizeof(Qwlanfw_SysCfgType));
}


/* Register Backup Routines */

/* DESCRIPTION:
 *      Routine to write all the directly addressed registers
 *      into the ADU memory.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_RegBckupDirectRegisters(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalRegBckup *pRegBckup = &pMac->hal.RegBckupParam;
    tANI_U32 memAddr = 0;

    // Get the current address of the register list in the ADU mem
    memAddr = pMac->hal.RegBckupParam.regListCurrAddr;

    pHalPwrSave->ImpsCtx.aduMemAddr = pRegBckup->regListImpsStartAddr;
    pHalPwrSave->BmpsCtx.aduMemAddr = pRegBckup->regListBmpsStartAddr;
    pHalPwrSave->UapsdCtx.aduMemAddr = pRegBckup->regListUapsdStartAddr;

    // ****** Uapsd Register list ******
    status = halRegBckup_PostBmuStartRegisters(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
            return status;
    }

    // ****** Volatile register list ******
    pRegBckup->volatileRegListStartAddr = memAddr;
    status = halRegBckup_VolatileRegisters(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
        return status;
    }
    pRegBckup->volatileRegListSize = memAddr - pRegBckup->volatileRegListStartAddr;

    // Get the size of the register list in the memory
    pHalPwrSave->ImpsCtx.regListSize = memAddr - pHalPwrSave->ImpsCtx.aduMemAddr;
    pHalPwrSave->BmpsCtx.regListSize = memAddr - pHalPwrSave->BmpsCtx.aduMemAddr;
    pHalPwrSave->UapsdCtx.regListSize = memAddr - pHalPwrSave->UapsdCtx.aduMemAddr;

    // Store the address of the end of this register list, for
    // reference to the start of the indirectly accessed register list
    pHalPwrSave->regListSize = memAddr - pHalPwrSave->regListStartAddr;

    // Update the current address
    pMac->hal.RegBckupParam.regListCurrAddr = memAddr;

    return status;
}

/* DESCRIPTION:
 *      Routine to write all the indirectly accessed registers
 *      into ADU memory.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_RegBckupIndirectRegisters(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tANI_U32 memAddr = 0;

    // Get the address from where to start backing up the indirectly
    // accessed register list
    memAddr = pHalPwrSave->indirectRegListStartAddr;

    status = halRxp_BckupRxpSearchTable(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
            return status;
    }

    status = halRate_BckupTpeRateTable(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
            return status;
    }

    status = halAdu_BckupUmaSearchTable(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
            return status;
    }

    status = halBmu_BckupBtqmStaConfig(pMac, &memAddr);
    if(status != eHAL_STATUS_SUCCESS) {
            return status;
    }

#ifdef FIXME_VOLANS
    // placeholder to re-enable the ARM clock by clearing the QWLAN_MCU_ACPU_CONTROL_MCU_MCU_ACPU_BOOT_LOOPING_EN

#endif

    // Introduce some empty space (with dummy wait commands) in
    // the register table for addition if entries using dump command
    halRegBckup_InsertOverrideRegList(pMac, &memAddr);

    // Write the magic value in the next device memory location
    status = halRegBckup_WriteTableEndCmd(pMac, memAddr);

    return status;
}

/**
  * halPS_AddWowlPatternToFw
  *
  * DESCRIPTION:
  *      Adds WOWL broadcast patterns to firmware when add msg
  *      is received from PE
  *
  * PARAMETERS:
  *      pMac:   Pointer to the global adapter context
  *      pBcastPat: Pointer to broadcast pattern and its parameters
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  *      return code from palAllocateMemory(), halFW_SendMsg()
  */

eHalStatus halPS_AddWowlPatternToFw(tpAniSirGlobal pMac, tpSirWowlAddBcastPtrn pBcastPat)
{
    tANI_U8 *pBuf;
    tANI_U8 *pWrite;
    tANI_U32 *pSwapWrite;
    tANI_U32 index;
    tANI_U8 msgLen;
    tANI_U8 remLen;
    tANI_U8 swapLen;
    eHalStatus status;
    Qwlanfw_AddMatchPtrnMsgType *pFwMsg;
    Qwlanfw_MatchPtrnType *pFwPtrn;

    // Validate input pointers
    if (NULL == pBcastPat)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Error in parameters: NULL pointer passed in\n")));
        // Can't free pBcastPat without both pMac & pBcastPat
        return eHAL_STATUS_FAILURE;
    }

    // Validate pattern size, pattern mask size, & pattern ID are in range
    if ((0 == pBcastPat->ucPatternSize) ||
        (0 == pBcastPat->ucPatternMaskSize) ||
        ((pBcastPat->ucPatternId >= SIR_WOWL_BCAST_MAX_NUM_PATTERNS) &&
         (pBcastPat->ucPatternId != QWLANFW_MAGIC_PCKT_PTTRN_ID)))
    {
        HALLOGE(halLog(pMac, LOGE, FL("Error in parameters: Pattern size (%d), mask size (%d), Id (%d)\n"),
               pBcastPat->ucPatternSize,
               pBcastPat->ucPatternMaskSize,
               pBcastPat->ucPatternId));
        palFreeMemory(pMac->hHdd, (void *)pBcastPat);
        return eHAL_STATUS_FAILURE;
    }

    // Allocate memory for sending the message to FW
    // round message length up to multiple of (sizeof(tANI_U32)) for BAL pre-swap later
    // (assumes sizeof(tANI_U32) will be '4' to get cheap modulo)
    msgLen = sizeof(Qwlanfw_AddMatchPtrnMsgType) +
             sizeof(Qwlanfw_MatchPtrnType) +
             pBcastPat->ucPatternSize +
             pBcastPat->ucPatternMaskSize;
    remLen = msgLen & 3; // % sizeof(tANI_U32)
    msgLen += (remLen ? (sizeof(tANI_U32) - remLen) : 0);
    status = palAllocateMemory(pMac->hHdd, (void**)&pBuf, (tANI_U32)msgLen);
    if (NULL == pBuf)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"), status));
        palFreeMemory(pMac->hHdd, (void *)pBcastPat);
        return status;
    }

    // Compose message - first, the FW message body
    pWrite = pBuf;
    pFwMsg = (Qwlanfw_AddMatchPtrnMsgType *)pWrite;
    pFwMsg->ucNumPtrns = 1;
    pFwMsg->bReserved = 0;
    pWrite += sizeof(Qwlanfw_AddMatchPtrnMsgType);

    // Followed by the pattern specifier
    pFwPtrn = (Qwlanfw_MatchPtrnType *)pWrite;
    pFwPtrn->usPtrnId = pBcastPat->ucPatternId;
    pFwPtrn->usPatternByteOffset = pBcastPat->ucPatternByteOffset;
    pFwPtrn->ucPatternSize = pBcastPat->ucPatternSize;
    pFwPtrn->ucPatternMaskSize = pBcastPat->ucPatternMaskSize;
    pWrite += sizeof(Qwlanfw_MatchPtrnType);

    // (Save the pointer to the message body (header is tANI_U32 aligned))
    pSwapWrite = (tANI_U32 *)pWrite;

    // Followed by the pattern
    palCopyMemory(pMac->hHdd, (void *)pWrite, (void *)pBcastPat->ucPattern, (tANI_U32)pBcastPat->ucPatternSize);
    pWrite += pBcastPat->ucPatternSize;

    // Followed by the pattern mask
    palCopyMemory(pMac->hHdd, (void *)pWrite, (void *)pBcastPat->ucPatternMask, (tANI_U32)pBcastPat->ucPatternMaskSize);

    // BAL will have SIF swap each DWORD to big endian (assumes all DWORDS are little endian)
    // Pre-swap the buffer here (assumes sizeof(tANI_U32) will be '4' to get cheap divide)
    swapLen = (pBcastPat->ucPatternSize + pBcastPat->ucPatternMaskSize) >> 2; // / sizeof(tANI_U32)
    swapLen += (remLen ? 1 : 0);
    for (index = 0; index < swapLen; index++)
    {
        pSwapWrite[index] = sirSwapU32(pSwapWrite[index]);
    }

    // Send message to FW
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE, QWLANFW_HOST2FW_ADD_PTRN, 0, msgLen, pBuf, FALSE, NULL);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Failed to send message to firmware (0x%x)\n"), status));
    }

    // Free FW data memory as it is copied to mailbox
    palFreeMemory(pMac->hHdd, (void *)pBuf);

    // There is no response required for this message
    palFreeMemory(pMac->hHdd, (void *)pBcastPat);
    return status;
}

/**
  * halPS_RemoveWowlPatternAtFw
  *
  * DESCRIPTION:
  *      Removes WOWL broadcast pattern from firmware when
  *      remove msg is received from PE
  *
  * PARAMETERS:
  *      pMac:   Pointer to the global adapter context
  *      pDelBcastPat: Pointer to pattern ID
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  *      return code from palAllocateMemory(), halFW_SendMsg()
  */

eHalStatus halPS_RemoveWowlPatternAtFw(tpAniSirGlobal pMac, tpSirWowlDelBcastPtrn pDelBcastPat)
{
    tANI_U8 *pBuf;
    tANI_U32 *pSwapWrite;
    tANI_U32 index;
    tANI_U8 msgLen;
    tANI_U8 remLen;
    tANI_U8 swapLen;
    eHalStatus status;
    Qwlanfw_RemMatchPtrnMsgType *pFwMsg;

    // Validate input pointers
    if (NULL == pDelBcastPat)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Error in parameters: NULL pointer passed in\n")));
        // Can't free pDelBcastPat without both pMac & pDelBcastPat
        return eHAL_STATUS_FAILURE;
    }

    // Validate input message
    if (pDelBcastPat->ucPatternId >= SIR_WOWL_BCAST_MAX_NUM_PATTERNS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Error in parameters: Id (%d)\n"), pDelBcastPat->ucPatternId));
        palFreeMemory(pMac->hHdd, (void *)pDelBcastPat);
        return eHAL_STATUS_FAILURE;
    }

    // Allocate memory for sending the message to FW
    // round message length up to multiple of (sizeof(tANI_U32)) for BAL pre-swap later
    // (assumes sizeof(tANI_U32) will be '4' to get cheap modulo)
    msgLen = sizeof(Qwlanfw_RemMatchPtrnMsgType);
    remLen = msgLen & 3; // % sizeof(tANI_U32)
    msgLen += (remLen ? (sizeof(tANI_U32) - remLen) : 0);
    status = palAllocateMemory(pMac->hHdd, (void**)&pBuf, (tANI_U32)msgLen);
    if (NULL == pBuf)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"), status));
        palFreeMemory(pMac->hHdd, (void *)pDelBcastPat);
        return status;
    }

    // Compose message - first, the FW message body
    pFwMsg = (Qwlanfw_RemMatchPtrnMsgType *)pBuf;
    pFwMsg->ucNumPtrns = 1;
    pFwMsg->bReserved = 0;

    // Followed by the pattern IDs
    pFwMsg->aPtrnId[0] = pDelBcastPat->ucPatternId;
    for (index = 1; index < QWLANFW_MAX_MATCH_PTRNS; index++)
    {
        pFwMsg->aPtrnId[index] = 0; // Zero out all other pattern IDs
    }

    // Save the pointer to the message body (header is tANI_U32 aligned)
    pSwapWrite = (tANI_U32 *)&(pFwMsg->aPtrnId[0]);

    // BAL will have SIF swap each DWORD to big endian (assumes all DWORDS are little endian)
    // Pre-swap the buffer here (assumes sizeof(tANI_U32) will be '4' to get cheap divide)
    swapLen = QWLANFW_MAX_MATCH_PTRNS >> 2; // / sizeof(tANI_U32)
    swapLen += (remLen ? 1 : 0);
    for (index = 0; index < swapLen; index++)
    {
        pSwapWrite[index] = sirSwapU32(pSwapWrite[index]);
    }

    // Send message to FW
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE, QWLANFW_HOST2FW_REM_PTRN, 0, msgLen, pBuf, FALSE, NULL);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Failed to send message to firmware (0x%x)\n"), status));
    }

    // Free FW data memory as it is copied to mailbox
    palFreeMemory(pMac->hHdd, (void*)pBuf);

    // There is no response required for this message
    palFreeMemory(pMac->hHdd, (void *)pDelBcastPat);
    return status;
}

/**
  * halPS_AddMagicPacketPatternToFw
  *
  * DESCRIPTION:
  *      adds the magic pattern to FW
  *
  * PARAMETERS:
  *      pMac:    Pointer to the global adapter context
  *      macAddr: Our MAC address
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  */

static
eHalStatus halPS_AddMagicPacketPatternToFw(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
    eHalStatus status;
    tpSirWowlAddBcastPtrn pBcastPat;
    v_MACADDR_t *pMacAddr;
    tANI_U8 index;

    // Allocate memory for sending the message to FW
    status = palAllocateMemory(pMac->hHdd, (void**)&pBcastPat, sizeof(*pBcastPat));
    if (NULL == pBcastPat)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"), status));
        return status;
    }

    // zero out pattern
    palFillMemory(pMac->hHdd, (void *)pBcastPat, sizeof(*pBcastPat), 0);

    // fill in header
    pBcastPat->ucPatternByteOffset = 0;
    pBcastPat->ucPatternSize = 102; // 17 * sizeof(tSirMacAddr)
    pBcastPat->ucPatternMaskSize = 13; // ceil(patternSize/8)

    // we use a reserved ID after the last of the normal patterns
    pBcastPat->ucPatternId = QWLANFW_MAGIC_PCKT_PTTRN_ID;

    // fill in pattern
    // addr 1
    pMacAddr = (v_MACADDR_t *)&(pBcastPat->ucPattern[0]);
    for (index = 0; index < VOS_MAC_ADDR_SIZE; index++)
    {
        pMacAddr->bytes[index] = 0xff;
    }
    pMacAddr++;

    // addrs 2-17
    for (index = 0; index < 16; index++)
    {
        vos_copy_macaddr(pMacAddr, (v_MACADDR_t *)macAddr);
        pMacAddr++;
    }

    // fill in mask
    // bytes 1-96
    for (index = 0; index < 12; index++)
    {
        pBcastPat->ucPatternMask[index] = 0xff;
    }

    // bytes 97-102
    pBcastPat->ucPatternMask[12] = 0x3f;

    // send the message to FW
    status = halPS_AddWowlPatternToFw(pMac, pBcastPat);
    return(status);
}

#if 0
/**
  * halPS_RemoveMagicPacketPatternAtFw
  *
  * DESCRIPTION:
  *      removes the magic pattern from FW
  *
  * PARAMETERS:
  *      pMac:    Pointer to the global adapter context
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  */

static
eHalStatus halPS_RemoveMagicPacketPatternAtFw(tpAniSirGlobal pMac)
{
    eHalStatus status;
    tpSirWowlDelBcastPtrn pDelBcastPat;

    // Allocate memory for sending the message to FW
    status = palAllocateMemory(pMac->hHdd, (void**)&pDelBcastPat, sizeof(*pDelBcastPat));
    if (NULL == pDelBcastPat)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"), status));
        return status;
    }

    // zero out pattern
    palFillMemory(pMac->hHdd, (void *)pDelBcastPat, sizeof(*pDelBcastPat), 0);

    // we use a reserved ID after the last of the normal patterns
    pDelBcastPat->ucPatternId = QWLANFW_MAGIC_PCKT_PTTRN_ID;

    // send the message to FW
    status = halPS_RemoveWowlPatternAtFw(pMac, pDelBcastPat);
    return(status);
}
#endif

/**
  * halPS_EnterWowlReq
  *
  * DESCRIPTION:
  *      Set and enable WOWL modes of operation
  *
  * PARAMETERS:
  *      pMac:   Pointer to the global adapter context
  *      dialogToken: Dialog Token to be carried in the message
  *      pWowParams: Pointer to the WOWL set parameters
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  *      return code from halFW_UpdateSystemConfig()
  */

eHalStatus halPS_EnterWowlReq(tpAniSirGlobal pMac, tANI_U16 dialogToken, tpSirHalWowlEnterParams pWowParams)
{
    tANI_U8 psState;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalFwParams *pFw;
    Qwlanfw_SysCfgType *pFwConfig;

#ifdef QWLAN_HW_MAGIC_PCKT_FILTER
    tANI_U32 magicPtrnHi;
    tANI_U32 magicPtrnLo;
    tANI_U32 wowTimers;
    tANI_U32 wowCfg;
    tANI_U32 aduCfg;
#endif  // QWLAN_HW_MAGIC_PCKT_FILTER

    // Validate input pointers
    if (NULL == pWowParams)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Error in parameters: NULL pointer passed in\n")));
        // Can't send a response without both pMac & pWowParams
        return eHAL_STATUS_FAILURE;
    }

    // Validate system state
    psState = halPS_GetState(pMac);
    if (!(HAL_PWR_SAVE_BMPS_STATE & psState))  // not BMPS mode
                                             // TODO: not associated
                                             // TODO: BT-AMP link active
                                             // TODO: IBSS links
    {
        VOS_ASSERT(0);
        status = eHAL_STATUS_FAILURE;
        goto end;
    }

#ifdef QWLAN_HW_MAGIC_PCKT_FILTER

    // Read in current ADU control and DPU Wow config values
    halReadRegister(pMac, QWLAN_ADU_CONTROL_REG, &aduCfg);
    halReadRegister(pMac, QWLAN_DPU_DPU_WOW_CONFIG_REG, &wowCfg);

    // Magic packet filtering
    if (FALSE != pWowParams->ucMagicPktEnable)
    {
        // Extract the pattern
        magicPtrnHi = (pWowParams->magicPtrn[0] << 8) |
                      (pWowParams->magicPtrn[1]);
        magicPtrnLo = (pWowParams->magicPtrn[2] << 24) |
                      (pWowParams->magicPtrn[3] << 16) |
                      (pWowParams->magicPtrn[4] << 8) |
                      (pWowParams->magicPtrn[5]);

        // Program magic pattern (6 bytes)
        halWriteRegister(pMac, QWLAN_DPU_DPU_WOW_MAGIC_PACKET_HI_REG, magicPtrnHi);
        halWriteRegister(pMac, QWLAN_DPU_DPU_WOW_MAGIC_PACKET_LO_REG, magicPtrnLo);

        // Extract the timer info
        wowTimers = ((pWowParams->ucWowMaxSleepUsec << QWLAN_DPU_DPU_WOW_TIMERS_WAKEUP_OFFSET) & QWLAN_DPU_DPU_WOW_TIMERS_WAKEUP_MASK) |
                    ((pWowParams->ucWowMaxMissedBeacons << QWLAN_DPU_DPU_WOW_TIMERS_LINK_DOWN_OFFSET) & QWLAN_DPU_DPU_WOW_TIMERS_LINK_DOWN_MASK);

        // Timer config
        halWriteRegister(pMac, QWLAN_DPU_DPU_WOW_TIMERS_REG, wowTimers);

        // Enable Wow routing in ADU control
        aduCfg |= (1 << QWLAN_ADU_CONTROL_PUSH_WQ_SELECTION_WOW_MODE_OFFSET);

        // Enable Wow in DPU Wow config with requested switches
        wowCfg &= ~(QWLAN_DPU_DPU_WOW_CONFIG_WOW_CHANSWITCH_EN_MASK |
                    QWLAN_DPU_DPU_WOW_CONFIG_WOW_DEAUTH_EN_MASK |
                    QWLAN_DPU_DPU_WOW_CONFIG_WOW_DISASS_EN_MASK |
                    QWLAN_DPU_DPU_WOW_CONFIG_WOW_MODE_MASK);
        wowCfg |= ((pWowParams->ucWowChnlSwitchRcv << QWLAN_DPU_DPU_WOW_CONFIG_WOW_CHANSWITCH_EN_OFFSET) & QWLAN_DPU_DPU_WOW_CONFIG_WOW_CHANSWITCH_EN_MASK) |
                  ((pWowParams->ucWowDeauthRcv << QWLAN_DPU_DPU_WOW_CONFIG_WOW_DEAUTH_EN_OFFSET) & QWLAN_DPU_DPU_WOW_CONFIG_WOW_DEAUTH_EN_MASK) |
                  ((pWowParams->ucWowDisassocRcv << QWLAN_DPU_DPU_WOW_CONFIG_WOW_DISASS_EN_OFFSET) & QWLAN_DPU_DPU_WOW_CONFIG_WOW_DISASS_EN_MASK) |
                  ((QWLAN_DPU_DPU_WOW_CONFIG_WOW_MODE_DEFAULT << QWLAN_DPU_DPU_WOW_CONFIG_WOW_MODE_OFFSET) & QWLAN_DPU_DPU_WOW_CONFIG_WOW_MODE_MASK) |
                  (1 << QWLAN_DPU_DPU_WOW_CONFIG_WOW_EN_OFFSET);
    }
    else
    {
        // Disable Wow routing in ADU control
        aduCfg &= ~(1 << QWLAN_ADU_CONTROL_PUSH_WQ_SELECTION_WOW_MODE_OFFSET);

        // Disable Wow in DPU Wow config
        wowCfg &= ~(1 << QWLAN_DPU_DPU_WOW_CONFIG_WOW_EN_OFFSET);
    }

    // Write out new ADU control and DPU Wow config values
    halWriteRegister(pMac, QWLAN_ADU_CONTROL_REG, aduCfg);
    halWriteRegister(pMac, QWLAN_DPU_DPU_WOW_CONFIG_REG, wowCfg);

#else  // QWLAN_HW_MAGIC_PCKT_FILTER

    // Magic packet filtering
    if (FALSE != pWowParams->ucMagicPktEnable)
    {
        // Add the magic packet pattern in FW
        status = halPS_AddMagicPacketPatternToFw(pMac, pWowParams->magicPtrn);
        if (eHAL_STATUS_SUCCESS != status)
        {
            HALLOGE(halLog(pMac, LOGE, FL("Failed to add magic pattern at FW (0x%x)\n"), status));
            goto end;
        }
    }

#endif  // QWLAN_HW_MAGIC_PCKT_FILTER

    // Enable wakeup features in FW
    pFw = &(pMac->hal.FwParam);
    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    pFwConfig->bNetWakeupFilterEnable = ((FALSE != pWowParams->ucPatternFilteringEnable) ? TRUE : FALSE);
    pFwConfig->bMagicPacketEnabled = ((FALSE != pWowParams->ucMagicPktEnable) ? TRUE : FALSE);
    pFwConfig->bMagicPacketFilterEnable = ((FALSE != pWowParams->ucMagicPktEnable) ? TRUE : FALSE);
    pFwConfig->bUcastFrmFilterEnable = ((FALSE != pWowParams->ucUcastPatternFilteringEnable) ? TRUE : FALSE);
    pFwConfig->uMgmtWoWLPassMask = HAL_PWR_SAVE_FW_WOWL_FRAMES_PASSED_TO_HOST;
    status = halFW_UpdateSystemConfig(pMac, pFw->fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Failed to update FW system config (0x%x)\n"), status));
        goto end;
    }

  end:
    // Send response message to PE
    pWowParams->status = status;
    halMsg_GenerateRsp(pMac, SIR_HAL_WOWL_ENTER_RSP, dialogToken, (void *)pWowParams, status);
    return status;
}

/**
  * halPS_ExitWowlReq
  *
  * DESCRIPTION:
  *      Clear and disable WOWL modes of operation
  *
  * PARAMETERS:
  *      pMac:   Pointer to the global adapter context
  *      dialogToken: Dialog Token to be carried in the message
  *
  * RETURN:
  *      eHAL_STATUS_SUCCESS
  *      eHAL_STATUS_FAILURE
  *      return code from halFW_UpdateSystemConfig()
  */

eHalStatus halPS_ExitWowlReq(tpAniSirGlobal pMac, tANI_U16 dialogToken)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tHalFwParams *pFw = &(pMac->hal.FwParam);
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;

#ifdef QWLAN_HW_MAGIC_PCKT_FILTER
    tANI_U32 wowCfg;
    tANI_U32 aduCfg;
#endif  // QWLAN_HW_MAGIC_PCKT_FILTER
   
    // Disable wakeup feature in FW
    pFwConfig->bNetWakeupFilterEnable = FALSE;
    pFwConfig->bMagicPacketEnabled = FALSE;
    pFwConfig->bMagicPacketFilterEnable = FALSE;
    pFwConfig->bUcastFrmFilterEnable = FALSE;
    status = halFW_UpdateSystemConfig(pMac, pFw->fwSysConfigAddr, (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Failed to update FW system config (0x%x)\n"), status));
    }

#ifdef QWLAN_HW_MAGIC_PCKT_FILTER
    // Disable WOW magic pattern mode
    halReadRegister(pMac, QWLAN_DPU_DPU_WOW_CONFIG_REG, &wowCfg);
    wowCfg &= ~(1 << QWLAN_DPU_DPU_WOW_CONFIG_WOW_EN_OFFSET);
    halWriteRegister(pMac, QWLAN_DPU_DPU_WOW_CONFIG_REG, wowCfg);

    // Disable WOW routing in ADU control
    halReadRegister(pMac, QWLAN_ADU_CONTROL_REG, &aduCfg);
    aduCfg &= ~(1 << QWLAN_ADU_CONTROL_PUSH_WQ_SELECTION_WOW_MODE_OFFSET);
    halWriteRegister(pMac, QWLAN_ADU_CONTROL_REG, aduCfg);
#endif  // QWLAN_HW_MAGIC_PCKT_FILTER

    // Send response message to PE
    halMsg_GenerateRsp(pMac, SIR_HAL_WOWL_EXIT_RSP, dialogToken, NULL, status);
    return status;
}



/*
 * halPS_RegisterWrite
 *
 * DESCRIPTION:
 *      Register write while in power save mode will be monitored, ie. any
 *      register writes will increment a count in the FW system config. FW would
 *      look into this count before backing up the all the registers in the ADU
 *      reinint memory. If the count did not change as compared to the previous
 *      run, FW would not spend time backing up all the registers.
 *
 * PARAMETERS:
 *      hHal:   Pointer to the global adapter context
 *      regAddr: Address for the register
 *      regValue: Value to be written into the register
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_RegisterWrite(tHalHandle hHal, tANI_U32 regAddr, tANI_U32 regValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;
    tANI_U32 offset = (tANI_U32)&((Qwlanfw_SysCfgType *)0)->uRegWriteCount;
    eHalStatus status;

    if ((pMac->hal.PsParam.mutexCount == 0) && (pMac->hal.PsParam.mutexIntrCount == 0)) {
        HALLOGE(halLog(pMac, LOGE, "WMutex not acquired, %d, %d for Reg %08x, Value 0x%x (Please report to HAL team, except for dump commands)", 
                    pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, regAddr, regValue));
    }

    //Write into the HW register
    status = palWriteRegister(pMac->hHdd, regAddr, regValue);
	
    if(status != eHAL_STATUS_SUCCESS) {
        HALLOGE(halLog( pMac, LOGE, FL("halPS_RegisterWrite failed\n")));
        return eHAL_STATUS_FAILURE;
    }

    //Not updating ADU reinit counter for following register.
    //This register is not in ADU backup list and hence it causes an 
    //huge overhead to update ADU reinit counter everytime there is an
    //idle bd pdu interrupt interrupt.
    if ( QWLAN_BMU_BMU_IDLE_BD_PDU_STATUS_REG == regAddr )
    {
         return eHAL_STATUS_SUCCESS;
    }

    //Increment the register write count in the FW system config
    pFwConfig->uRegWriteCount++;

    // Update the sytem config
    return halFW_UpdateSystemConfig(pMac,
            pMac->hal.FwParam.fwSysConfigAddr + offset,
            (tANI_U8*)&pFwConfig->uRegWriteCount, sizeof(tANI_U32));

}

eHalStatus halPS_RegisterRead(tHalHandle hHal, tANI_U32 regAddr, tANI_U32 *pRegValue)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

    if ((pMac->hal.PsParam.mutexCount == 0) && (pMac->hal.PsParam.mutexIntrCount == 0)) {
        HALLOGE(halLog(pMac, LOGE, "RMutex not acquired, %d, %d, %d for Reg 0x%08x (Please report to HAL team, except for dump commands)", 
                    pMac->hal.PsParam.mutexCount, pMac->hal.PsParam.mutexIntrCount, pMac->hal.PsParam.mutexTxCount, regAddr));
    }

    return palReadRegister(pMac->hHdd, regAddr, pRegValue);

}


/*
 * halPS_MemoryWrite
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *      hHal:   Pointer to the global adapter context
 *      regAddr: Address for the register
 *      regValue: Value to be written into the register
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_MemoryWrite(tHalHandle hHal, tANI_U32 dstOffset, void *pSrcBuffer, tANI_U32 numBytes)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

#ifdef WLAN_SDIO_DUMMY_CMD53_WORKAROUND
    // Dummy write to the HW register
    palWriteRegister(pMac->hHdd, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_REG, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_DEFAULT);
#endif    

    return( palWriteDeviceMemory( pMac->hHdd, dstOffset, (tANI_U8 *)pSrcBuffer, numBytes ) );
}

eHalStatus halPS_MemoryRead(tHalHandle hHal, tANI_U32 srcOffset, void *pBuffer, tANI_U32 numBytes)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hHal;

#ifdef WLAN_SDIO_DUMMY_CMD53_WORKAROUND
    // Dummy write to the HW register
    palWriteRegister(pMac->hHdd, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_REG, QWLAN_SIF_SIF_CMD53_RD_DLY_START_CFG_REG_DEFAULT);
#endif    

    return( palReadDeviceMemory( pMac->hHdd, srcOffset, pBuffer, numBytes ) );
}


/*
 * halPS_StartMonitoringRegAccess
 *
 * DESCRIPTION:
 *      Start monitoring the register writes
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      void
 */
void halPS_StartMonitoringRegAccess(tpAniSirGlobal pMac)
{
    pMac->hal.funcWriteReg = halPS_RegisterWrite;
    pMac->hal.funcReadReg  = halPS_RegisterRead;
    pMac->hal.funcWriteMem = halPS_MemoryWrite;
    pMac->hal.funcReadMem  = halPS_MemoryRead;
}

/*
 * halPS_StopMonitoringRegAccess
 *
 * DESCRIPTION:
 *      Stop monitoring the register writes
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *
 * RETURN:
 *      void
 */
void halPS_StopMonitoringRegAccess(tpAniSirGlobal pMac)
{
    pMac->hal.funcWriteReg = halNormalWriteRegister;
    pMac->hal.funcReadReg  = halNormalReadRegister;
    pMac->hal.funcWriteMem = halNormalWriteMemory;
    pMac->hal.funcReadMem  = halNormalReadMemory;
}

/*
 * halPSDataInActivityTimeout
 *
 * DESCRIPTION:
 *      Data inactivity timeout configuration.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */

void halPSDataInActivityTimeout( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
    tANI_U32 dataInActivityTimeout;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    if (cfgId == WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT) {
        if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &dataInActivityTimeout )) {
             HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration file for Data Inactivity Timeout with cfgId %d"), cfgId));
             return;
        }
        /* Data InActivity Timeout value as read from CFG */
        pFwConfig->ucUcastDataRecepTimeoutMs = (tANI_U8)dataInActivityTimeout;

        /* Update FW SysConfig with Data Inactivity Value */
        halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
                     (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
     }
     return;
}

/*
 * halPSNullDataAPProcessDelay
 *
 * DESCRIPTION:
 *      AP delay processing of Null Data configuration.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */
void halPSNullDataAPProcessDelay( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
    tANI_U32 nullDataApRespTimeout;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

    if (cfgId == WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT) {
        if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &nullDataApRespTimeout )) {
             HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration file for Null data delay Timeout with cfgId %d"), cfgId));
             return;
        }

        /* Null Data Delay Timeout value as read from CFG */
        pFwConfig->nullDataApRespTimeoutMsec = (tANI_U8)nullDataApRespTimeout;

        /* Update FW SysConfig with NULL Data AP response delay timeout Value */
        halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
                     (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
    }

    return;
}


/*
 * halPSFWHeartBeatCfg
 *
 * DESCRIPTION:
 *      Enable/Disable FW Heart Beat. This should start/stop Chip Monitor timer.
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */

void halPSFWHeartBeatCfg( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
    tANI_U32 heartBeatCfgInPs;

    if (cfgId == WNI_CFG_PS_ENABLE_HEART_BEAT) {
           if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &heartBeatCfgInPs )) {
              HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration file for Heart Beat with cfgId %d"), cfgId));
              return;
           }

           /* Start Chip Monitor if Heart Beat is enabled */
          if (heartBeatCfgInPs == ENABLE_HEART_BEAT_IN_PS) {
             halFW_StartChipMonitor(pMac);
          }

         /* Stop Chip Monitor if Heatr Beat is disabled */
         if (heartBeatCfgInPs == DISABLE_HEART_BEAT_IN_PS) {
           halFW_StopChipMonitor(pMac);
         }
    }
    return;
}

/*
 * halPSBcnFilterCfg
 *
 * DESCRIPTION:
 *      Enable/Disable Beacon Filtering .
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */

void halPSBcnFilterCfg( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
     tANI_U32 bcnFilterValueInPS;
     Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

     if (cfgId == WNI_CFG_PS_ENABLE_BCN_FILTER) {
          if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &bcnFilterValueInPS )) {
              HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration file for Beacon Filter with cfgId %d"), cfgId));
              return;
          }

          /* Beacon Filter Value as read by the CFG */
          pFwConfig->bBeaconFilterEnable = (tANI_U8)bcnFilterValueInPS;

          /* Update FW System Config with Beacon Filter value */
          halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
                               (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
      }
      return;
}

/*
 * halPSRssiMonitorCfg
 *
 * DESCRIPTION:
 *      Enable/Disable RSSI Monitoring in PS .
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */
void halPSRssiMonitorCfg( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
     tANI_U32 rssiMonitorValuePS;
     Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)pMac->hal.FwParam.pFwConfig;

     if (cfgId == WNI_CFG_PS_ENABLE_RSSI_MONITOR) {
         if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &rssiMonitorValuePS )) {
              HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration file for Heart Beat with cfgId %d"), cfgId));
              return;
         }

         /* RSSI Monitor value as read from CFG */
         pFwConfig->bRssiFilterEnable = (tANI_U8)rssiMonitorValuePS;

         /* Update FW System Config with RSSI Monitor value */
         halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
               (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
     }
     return;
}

/*
 * halPS_SetHostOffloadInFw
 *
 * DESCRIPTION:
 *      Sets host offload configuration in firmware when set message
 *      is received from PE
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pRequest: Pointer to offload request
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 *      return code from palAllocateMemory(), halFW_SendMsg()
 */

eHalStatus halPS_SetHostOffloadInFw(tpAniSirGlobal pMac, tpSirHostOffloadReq pRequest)
{
    Qwlanfw_HostOffloadReqType *pFwMsg;
    eHalStatus status;

    // Allocate memory for sending the message to FW
    status = palAllocateMemory(pMac->hHdd, (void **)&pFwMsg, sizeof(Qwlanfw_HostOffloadReqType));
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"),
                    status));
        palFreeMemory(pMac->hHdd, pRequest);
        return status;
    }

    pFwMsg->enableOrDisable = 0;

    // Compose message
    switch (pRequest->offloadType)
    {
        case SIR_IPV4_ARP_REPLY_OFFLOAD:
            pFwMsg->offloadType = HOST_OFFLOAD_IPV4_ARP_REPLY;
            switch (pRequest->enableOrDisable)
            {
                case SIR_OFFLOAD_DISABLE:
                    pFwMsg->enableOrDisable = HOST_OFFLOAD_DISABLE;
                    break;
                case SIR_OFFLOAD_ARP_AND_BCAST_FILTER_ENABLE:
                    pFwMsg->enableOrDisable |= HOST_OFFLOAD_BC_FILTER_ENABLE;
                case SIR_OFFLOAD_ENABLE:
                    pFwMsg->enableOrDisable |= HOST_OFFLOAD_ENABLE;
                    palCopyMemory(pMac->hHdd, pFwMsg->params.hostIpv4Addr,
                            pRequest->params.hostIpv4Addr, 4);
                    sirSwapU32BufIfNeeded((tANI_U32 *)pFwMsg->params.hostIpv4Addr, 1);
                    break;
                default:
                    HALLOGE(halLog(pMac, LOGE, 
                                FL("Invalid option %d for ARP offload, disabling it by default!\n"),
                                pRequest->enableOrDisable));
                    pFwMsg->enableOrDisable = HOST_OFFLOAD_DISABLE;
                    break;
            }
            break;
        case SIR_IPV6_NEIGHBOR_DISCOVERY_OFFLOAD:
            pFwMsg->offloadType = HOST_OFFLOAD_IPV6_NEIGHBOR_DISCOVERY;
            switch (pRequest->enableOrDisable)
            {
                case SIR_OFFLOAD_DISABLE:
                    pFwMsg->enableOrDisable = HOST_OFFLOAD_DISABLE;
                    break;
                case SIR_OFFLOAD_ENABLE:
                    pFwMsg->enableOrDisable = HOST_OFFLOAD_ENABLE;
                    palCopyMemory(pMac->hHdd, pFwMsg->params.hostIpv6Addr,
                            pRequest->params.hostIpv6Addr, 16);
                    sirSwapU32BufIfNeeded((tANI_U32 *)pFwMsg->params.hostIpv6Addr, 4);
            }
    }

    // Send message to FW
    status = halFW_SendMsg(pMac, HAL_MODULE_ID_PWR_SAVE, QWLANFW_HOST2FW_SET_HOST_OFFLOAD, 0,
            sizeof(Qwlanfw_HostOffloadReqType), pFwMsg, FALSE, NULL);
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("Failed to send message to firmware (0x%x)\n"), status));
    }

    // Free FW data memory as it is copied to mailbox
    palFreeMemory(pMac->hHdd, pFwMsg);

    // There is no response required for this message
    palFreeMemory(pMac->hHdd, pRequest);

    return status;
}

/*
 * halPSRfSettlingTimeClk
 *
 * DESCRIPTION:
 *      RF Supply Settling Time Clock Units
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      cfgId:  This will read from the CFG file set by HDD.
 *
 * RETURN:
 *      void
 */

 void halPSRfSettlingTimeClk( tpAniSirGlobal pMac, tANI_U32 cfgId )
 {
    tANI_U32 rfSettlingTimeUs = 0;
    Qwlanfw_SysCfgType *pFwConfig = (Qwlanfw_SysCfgType *)
    pMac->hal.FwParam.pFwConfig;

    if (cfgId == WNI_CFG_RF_SETTLING_TIME_CLK) {
        if (eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfgId, &rfSettlingTimeUs)) {
            HALLOGW( halLog(pMac, LOGW, FL("Failed to read Configuration "
            "file for Rf Supply Settling Time Clock Units with  cfgId %d"), cfgId));
            return;
        }
    /* RF Settling Time Clk value as read from CFG */
    pFwConfig->ucRfSupplySettlingTimeClk = (tANI_U16)((rfSettlingTimeUs*1000)/QWLAN_PMIC_SLEEPCLK_PERIOD_NS);
    pFwConfig->ucRfSupplySettlingTimeClk += ((rfSettlingTimeUs*1000)%QWLAN_PMIC_SLEEPCLK_PERIOD_NS)? 1 : 0;

    pFwConfig->usBmpsSleepTimeOverheadsUs = HAL_PWR_SAVE_FW_BMPS_SLEEP_TIME_OVERHEADS_WITHOUT_RFXO_SETTLING_US +
					((pFwConfig->ucRfSupplySettlingTimeClk * QWLAN_PMIC_SLEEPCLK_PERIOD_NS)/1000);

    /* Update FW SysConfig with Rf Supply Settling Time Clock Units Value */
    halFW_UpdateSystemConfig(pMac, pMac->hal.FwParam.fwSysConfigAddr,
                     (tANI_U8 *)pFwConfig, sizeof(Qwlanfw_SysCfgType));
   }

   return;
   
}

#ifdef WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS
/* Call back invoked by TX/RX thread incase it needs to acquire resource before direct chip 
 * access in power save mode */
VOS_STATUS halPS_SetHostBusyTxRx(v_PVOID_t pMacContext)
{
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(pMacContext);

    /* Need to acquire mutex only if we are in BMPS */
    if (IS_PWRSAVE_STATE_IN_BMPS)
    {
        status = halPS_SetHostBusy(pMac, HAL_PS_BUSY_TXRX_CONTEXT);
        VOS_TRACE(VOS_MODULE_ID_HAL, VOS_TRACE_LEVEL_INFO_HIGH, "Acquired Mutex in TxRx context, Cnt = %d\n", pMac->hal.PsParam.mutexTxRxCount);
    }

    if (status)
        return VOS_STATUS_E_FAILURE;

    return VOS_STATUS_SUCCESS;
}

void halPSAppsCpuWakeupState( tpAniSirGlobal pMac, tANI_U32 isAppsAwake )
{
    tANI_U32 offset = (tANI_U32)&((Qwlanfw_SysCfgType *)0)->isAppsCpuAwake;
    tANI_U32 appsCpuWakeupState = isAppsAwake;
    tHalFwParams *pFw = &pMac->hal.FwParam;
    Qwlanfw_SysCfgType *pFwConfig;

    pFwConfig = (Qwlanfw_SysCfgType *)pFw->pFwConfig;
    pFwConfig->isAppsCpuAwake = isAppsAwake;

    if (IS_PWRSAVE_STATE_IN_BMPS) 
    {
        halPS_SetHostBusy(pMac, HAL_PS_BUSY_GENERIC); 
    }

    // Update the sytem config
    halFW_UpdateSystemConfig(pMac,
                            pMac->hal.FwParam.fwSysConfigAddr + offset,
                            (tANI_U8*)&appsCpuWakeupState, sizeof(tANI_U32));

   if (IS_PWRSAVE_STATE_IN_BMPS) 
   {
       halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC); 
   }
   return;
}

/* Call back invoked by TX/RX thread to release the resource after chip 
 * access in power save mode */
VOS_STATUS halPS_ReleaseHostBusyTxRx(v_PVOID_t pMacContext)
{
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(pMacContext);

    /* Dont check for BMPS state here as it is possible that EXIT BMPS
     * might have got called after acquiring mutex in TX context. In 
     * that case, this condition check can result in loss of one mutex */

    /* If we dont have any mutex count, we cant release it. Something wrong must have happened */
    if (IS_HOST_BUSY_TXRX_CNTX)
        status = halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_TXRX_CONTEXT);

    if (status)
        return VOS_STATUS_E_FAILURE;

    return VOS_STATUS_SUCCESS;

}
#endif /* WLAN_FEATURE_PROTECT_TXRX_REG_ACCESS */
#ifdef WLAN_FEATURE_P2P
/*
 * DESCRIPTION:
 *      Function to Send Single NoA information to FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      dialogToken: Dialog Token to be carried in the message
 *      cbFunc: Function pointer to the callback function to be called after
 *              suspend BMPS is done.
 *      data:   Pointer to the data to be passed to the callback function
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_UpdateSingleNoA(tpAniSirGlobal pMac, tANI_U16 dialogToken,
      funcHalPsCB cbFunc, void* data)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsP2p *pP2pPsCtx = &pMac->hal.PsParam.P2pPsCtx;
    Qwlanfw_P2pPs fwP2pPS = {0};
    tpInitScanParams scanparams = (tpInitScanParams)data;
    pP2pPsCtx->token = dialogToken;

    HALLOGE( halLog(pMac, LOGE, FL("Sending Single NoA message to FW")));

    // Start the timer for the FW response
    // NOTE: Here we start the timer before sending the message so in case if
    // start time fails, we can simply fall through. Had we started the timer
    // after sending the message to FW and in case of failure, HAL will have to
    // send another message to FW to bring it out of that state.
    vosStatus = vos_timer_start(&pHalPwrSave->fwRspTimer, 
                                               pHalPwrSave->fwRspTimeout);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE, 
            FL("VOS Timer start failed - status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_START_FAILED;
        return status;
    }

    pHalPwrSave->rspType = SIR_HAL_P2P_UPDATE_SINGLE_NOA;
    pHalPwrSave->dialogToken = dialogToken;

    // Store the callback function pointer and the data
    pHalPwrSave->psCbFunc = cbFunc;
    pHalPwrSave->psCbData = data;

    fwP2pPS.P2pPsSelection = P2P_PS_UPDATE_SINGLE_NOA;
    /* Durations should be specified in us */
    fwP2pPS.SingleNoAData.uDuration = 
                        scanparams->scanDuration*TIME_UNIT_IN_USEC;
    fwP2pPS.IndicateFirstAbsent = 1;

    // Send the single NoA information to FW
    status = halFW_SendP2pMsg(pMac, QWLANFW_P2P_UPDATE_GO_PS, &fwP2pPS, 
                              sizeof(Qwlanfw_P2pPs));

    if (status != eHAL_STATUS_SUCCESS) {
        HALLOGE( halLog(pMac, LOGE, FL("FW send Update NOA to FW \n")));
        status = eHAL_STATUS_FW_SEND_MSG_FAILED;
        // Stop the FW response timeout timer
        vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
        if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
            HALLOGE( halLog(pMac, LOGE, 
                FL("VOS Timer stop failed, status = %d\n"), vosStatus));
            status = eHAL_STATUS_TIMER_STOP_FAILED;
        }
        return status;
    }
    return status;
}

/*
 * DESCRIPTION:
 *      Function handle NOA Start message from FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwP2pNoAStarted ( tpAniSirGlobal pMac, void* pFwMsg)
{
    eHalStatus status = eHAL_STATUS_FAILURE;
    VOS_STATUS  vosStatus = VOS_STATUS_E_FAILURE;
    tHalPwrSave *pHalPwrSave = &pMac->hal.PsParam;
    tHalPsP2p *pP2pPsCtx = &pMac->hal.PsParam.P2pPsCtx;
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    tANI_U8 fwStatus = *pMsgBody;

    // Stop the FW response timeout timer
    vosStatus = vos_timer_stop(&pHalPwrSave->fwRspTimer);
    if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
        HALLOGE( halLog( pMac, LOGE,
                FL("VOS Timer stop Failed, status = %d\n"), vosStatus));
        status = eHAL_STATUS_TIMER_STOP_FAILED;
    }

    HALLOG1( halLog(pMac, LOG1, 
             FL("resp received from FW, status = %d\n"), fwStatus));

    // Check the FW status of the response message
    if (fwStatus != QWLANFW_STATUS_SUCCESS) {
        HALLOGE( halLog( pMac, LOGE, FL("FW error status = %d"), fwStatus));
        status = eHAL_STATUS_FAILURE;
    } else {
        status = eHAL_STATUS_SUCCESS;
    }

    // Call the callback function
    if (pHalPwrSave->psCbFunc != NULL) {
        pHalPwrSave->psCbFunc(pMac, pHalPwrSave->psCbData, 
                                    pP2pPsCtx->token, status);
    }
    return status;
}

/*
 * DESCRIPTION:
 *      Function handle NOA Attribute message from FW
 *
 * PARAMETERS:
 *      pMac:   Pointer to the global adapter context
 *      pFwMsg: Pointer to the FW message
 *
 * RETURN:
 *      eHAL_STATUS_SUCCESS
 *      eHAL_STATUS_FAILURE
 */
eHalStatus halPS_HandleFwP2PNoAAttr ( tpAniSirGlobal pMac, void* pFwMsg)
{
    tMBoxMsgHdr *pMbMsg = (tMBoxMsgHdr *)pFwMsg;
    tANI_U8 *pMsgBody = (tANI_U8*)(pFwMsg) + sizeof(*pMbMsg);
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMsgQ    msg;    
    tpSirP2PNoaAttr pP2PNoaAttr = NULL;	

    status = palAllocateMemory(pMac->hHdd, (void **)&pP2PNoaAttr, sizeof(tSirP2PNoaAttr));
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE(halLog(pMac, LOGE, FL("palAllocateMemory() failed to return buffer (0x%x)\n"),
                       status));        
        return status;
    }

    palCopyMemory(pMac->hHdd, pP2PNoaAttr, pMsgBody,sizeof(tSirP2PNoaAttr));

    HALLOG1( halLog(pMac, LOG1, 
             FL("P2P noa attr received from FW, status = %d\n"), status));

    //Posting the info to PE
    // Prepare and post message to LIM Message Queue
    msg.type = SIR_HAL_P2P_NOA_ATTR_IND;
    msg.bodyval = 0;
    msg.bodyptr = pP2PNoaAttr;

    limPostMsgApi(pMac, &msg);
    
    return status;
}
#endif

