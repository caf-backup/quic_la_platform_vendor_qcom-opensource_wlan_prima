/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * hal.c:    HAL thread startup file.
 * Author:   V. K. Kandarpa
 * Date:     01/29/2002
 * History:-
 * Date      Modified by            Modification Information
 * --------------------------------------------------------------------------
 *
 */

/* Standard include files */

/* Application Specific include files */
#include "halTypes.h"
#include "halHddApis.h"

#include "sirCommon.h"
#include "aniGlobal.h"
#include "limApi.h"
#include "utilsApi.h"
#include "halCommonApi.h"
#include "halInternal.h"
#include "halDebug.h"
#include "halPhyApi.h"
#include "halMTU.h"
#include "halRxp.h"
#include "halTpe.h"
#include "sirApi.h"
#include "cfgApi.h"
#include "halTimer.h"
#include "halUtils.h"
#include "halMacBA.h"
#include "macInitApi.h"
#include "halAdaptThrsh.h"
#include "halRFKill.h"
#include "halPwrSave.h"
#include "halBtc.h"
#include "vos_memory.h"
#include "vos_types.h"

#include "halTLFlush.h"
#include "halFw.h"
#include "rfApi.h"

#ifdef ANI_PRODUCT_TYPE_AP
#include "halRadar.h"
#endif

#ifdef ANI_DVT_DEBUG
#include "dvtModuleApi.h"
#endif

#include "pttModuleApi.h"
#include "pttMsgApi.h"
#include "halPhyUtil.h"

#ifdef ANI_OS_TYPE_RTAI_LINUX
#include "sysRtaiStartup.h"
#endif
#include <ani_assert.h>

#ifdef FEATURE_INNAV_SUPPORT
#include "halInNav.h"
#endif

/** Temp Measurment timer interval value 30 sec.*/
#define HAL_TEMPMEAS_TIMER_VAL_SEC          30

static void halPhy_setNwDensityAndProximity(tpAniSirGlobal pMac);
static void halSetChainPowerState(tpAniSirGlobal pMac);
extern eHalStatus halPrepareForBmpsEntry(tpAniSirGlobal pMac);
extern eHalStatus halPrepareForBmpsExit(tpAniSirGlobal pMac);

#ifdef WLAN_SOFTAP_FEATURE
static eHalStatus halHandleEnableListenModeCfg(tpAniSirGlobal pMac, tANI_U32 cfgId);
#endif
static 
eHalStatus halHandleMcastBcastFilterSetting(tpAniSirGlobal pMac, tANI_U32 cfgId);
static eHalStatus halHandleDynamicPsPollValue(tpAniSirGlobal pMac, tANI_U32 cfgId);

/* Constant Macros */
/* Redefine OFF -> __OFF, ON-> __ON to avoid redefinition on AMSS */
#define  MAX_VALID_CHAIN_STATE  4
#define  __OFF                    WNI_CFG_POWER_STATE_PER_CHAIN_OFF
#define  __ON                     WNI_CFG_POWER_STATE_PER_CHAIN_ON
#define  TX                     WNI_CFG_POWER_STATE_PER_CHAIN_TX
#define  RX                     WNI_CFG_POWER_STATE_PER_CHAIN_RX
#define  MAX_ALLOWED_BD_FOR_IDLE         5

/* Function Macros */
#define  GET_CHAIN_0(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_0_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)
#define  GET_CHAIN_1(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_1_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)
#define  GET_CHAIN_2(chain)  ((chain >> WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_2_OFFSET) & WNI_CFG_POWER_STATE_PER_CHAIN_MASK)

#define  SET_CHAIN(c0, c1, c2)  \
  ( (c0 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_0_OFFSET) |  \
    (c1 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_1_OFFSET) |  \
    (c2 << WNI_CFG_POWER_STATE_PER_CHAIN_CHAIN_2_OFFSET)    \
  )

typedef struct eChainState{
    tANI_U32         encoding;
    ePhyChainSelect  halPhyDef;
} tChainState;

static tChainState chainPwrStateTable[MAX_VALID_CHAIN_STATE] = {
 { SET_CHAIN(__OFF, __OFF, __OFF),  PHY_CHAIN_SEL_NO_RX_TX    },
 { SET_CHAIN(TX,  __OFF, __OFF),  PHY_CHAIN_SEL_T0_ON     },
 { SET_CHAIN(RX,  __OFF, __OFF),  PHY_CHAIN_SEL_R0_ON     },
 { SET_CHAIN(__ON,  __OFF, __OFF),  PHY_CHAIN_SEL_R0_T0_ON    },
};


#if 0
static tANI_BOOLEAN
__halIsChipBusy(tpAniSirGlobal pMac)
{
    tANI_U32    freeBdPduCnt;
    tANI_U32    regVal = 0;

    halReadRegister((tHalHandle) pMac, QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_REG, &regVal);
    /** Calculate the Free BDS From the Register*/
    freeBdPduCnt = ((regVal & QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_BDS_MASK) >>
                                                    QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_BDS_OFFSET);

    /** Calculate the Free PDUs from the register and sum up BDs Count*/
    freeBdPduCnt += ((regVal & QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_PDUS_MASK) >>
                                                    QWLAN_BMU_AVAILABLE_BD_PDU_AFTER_RSV_AVAILABLE_PDUS_OFFSET);

    if (freeBdPduCnt < (pMac->hal.halMac.halMaxBdPduAvail - MAX_ALLOWED_BD_FOR_IDLE))
       return eANI_BOOLEAN_TRUE;

    return eANI_BOOLEAN_FALSE;
}
#endif

tANI_BOOLEAN halIsSelfHtCapable(tpAniSirGlobal pMac)
{
    tpStaStruct pSta;
    if(NULL == pMac)
    {
        HALLOGP( halLog(pMac, LOGP, FL("pMac is NULL\n")));
        return eANI_BOOLEAN_FALSE;
    }
    pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    if(true == pSta[pMac->hal.halMac.selfStaId].htEnabled)
        return eANI_BOOLEAN_TRUE;
    else
        return eANI_BOOLEAN_FALSE;
}

/** -------------------------------------------------------------
\fn      halPhy_setNwDensityAndProximity
\brief   This function sets the network density and proximity
\        configurations. When there is a change in network
\        density setting, we need to also update the adaptive
\        threshold setting as well.
\param   tpAniSirGlobal pMac
\return  none
  -------------------------------------------------------------*/
static void halPhy_setNwDensityAndProximity(tpAniSirGlobal pMac)
{
    tANI_U32      proximity;
    tANI_U32      nwDensity;
    tANI_BOOLEAN  densityOn;

    if (wlan_cfgGetInt(pMac, WNI_CFG_PROXIMITY, (tANI_U32 *) &proximity) != eSIR_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_PROXIMITY) failed \n")));

    if (wlan_cfgGetInt(pMac, WNI_CFG_NETWORK_DENSITY, (tANI_U32 *) &nwDensity) != eSIR_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet(WNI_CFG_NETWORK_DENSITY) failed \n")));

    if (proximity == WNI_CFG_PROXIMITY_OFF)
        densityOn = eANI_BOOLEAN_TRUE;
    else
        densityOn = eANI_BOOLEAN_FALSE;

    {
        HALLOG1( halLog(pMac, LOG1, FL("nwDensity=%d, proximity=%d, densityOn=%d \n"), nwDensity, proximity, densityOn));

        if (halPhySetNwDensity( pMac, densityOn, (ePhyNwDensity)nwDensity, (ePhyNwDensity)nwDensity ) != eHAL_STATUS_SUCCESS)
            HALLOGP( halLog(pMac, LOGP, FL("halPhySetNwDensity() failed")));
    }

    //halRate_updateRateTable(pMac);

    return;
}


// -------------------------------------------------------------
/**
 * halDoCfgInit
 *
 * FUNCTION:
 *     Initiates configuration and waits till configuration is done.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @return tSirRetStatus SUCCESS or FAILURE
 */
tSirRetStatus halDoCfgInit(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    tSirMsgQ msg;

    // Initializes calibration and waits until cfg is done.
    // Send Mailbox message START to host
    msg.type = SIR_HAL_APP_SETUP_NTF;

    // Construct the message and post it
    msg.bodyptr = NULL;
    msg.bodyval = 0;

    halMmhPostMsgApi(pMac, &msg, eHI_PRI);

    // Initialize config and stats module
    wlan_cfgInit(pMac);

    // we're done for now.
    // we'll be notified via a SIR_CFG_DOWNLOAD_COMPLETE_IND when the
    // config & stats module has finish its exchange with the host.

            return rc;
        }

/** -------------------------------------------------------------
\fn halSetReadyToHandleInt
\brief      Enables interrupt and also set hal state to normal. resets if fails to enable interrupt.
\param   tpAniSirGlobal pMac
\return none
  -------------------------------------------------------------*/
static void halSetReadyToHandleInt(tpAniSirGlobal pMac)
{
    if(eHAL_STATUS_SUCCESS != halIntChipEnable((tHalHandle)pMac))
    {
        HALLOGP( halLog(pMac, LOGP, FL("halIntChipEnable failed\n")));
    }
    else
        halStateSet(pMac, eHAL_NORMAL);
}

// -------------------------------------------------------------
/**
 * halProcessStartEvent
 *
 * FUNCTION:
 *     Initializes the HW and the system.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

tSirRetStatus halProcessStartEvent(tpAniSirGlobal pMac)
{
    tSirMsgQ msg;
    tSirMbMsg *mb;
    tANI_U32 status;
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U32    cfgVal = 0;

    do
    {
        HALLOG3( halLog(pMac, LOG3, FL("Entered!\n")));
        {
            // shall be called only AFTER CFG download is finsihed
            if (halInitWmParam(pMac, NULL) != eHAL_STATUS_SUCCESS)
            {
               HALLOGW( halLog(pMac, LOGW, FL("Failed at halInitWmParam() \n")));
               rc = eSIR_FAILURE;
               break;
            }

            /** Get the RTS Threshold */
            if ( wlan_cfgGetInt(pMac, WNI_CFG_RTS_THRESHOLD, &cfgVal) != eSIR_SUCCESS) {
                HALLOGE(halLog(pMac, LOGE, FL("cfgGet WNI_CFG_RTS_THRESHOLD Failed\n")));
            }

            /** Set the Protection threshold */
            if (halTpe_SetProtectionThreshold(pMac, cfgVal) != eHAL_STATUS_SUCCESS) {
                return eSIR_FAILURE;
            }

            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
            // Init BA parameters
            baInit( pMac );
            }
            pMac->hal.halMac.nonRifsBssCount = pMac->hal.halMac.rifsBssCount= 0;

#if defined(ANI_PRODUCT_TYPE_CLIENT)
            // Initialize Adaptive Threshold related globals
            halATH_initialize(pMac);
#endif

#ifdef ANI_SUPPORT_SMPS
            halMsg_InitRxChainsReg(pMac);
#endif

            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
            halPhy_setNwDensityAndProximity(pMac);
            }

            msg.type = SIR_LIM_RESUME_ACTIVITY_NTF;
            status = limPostMsgApi(pMac, &msg);
            if (status != eSIR_SUCCESS)
            {
                // above api does not post LOGP on error
                HALLOGP( halLog(pMac, LOGP,
                       FL("Failed limPostMsgApi=0%X\n"),
                       status));
                rc = eSIR_FAILURE;
                break;
            }
            HALLOGW( halLog(pMac, LOGW, FL("limresumeactivityntf is sent from hal\n")));

#if !defined(LOOPBACK) && !defined(ANI_DVT_DEBUG)
            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
           if (halMsg_AddStaSelf(pMac) != eHAL_STATUS_SUCCESS)
           {
               HALLOGW( halLog(pMac, LOGW, FL("Failed at halMsg_AddStaSelf() \n")));
               rc = eSIR_FAILURE;
               break;
           }
            }
#endif
            if(pMac->gDriverType != eDRIVER_TYPE_MFG)
            {
           if (halRxp_addBroadcastEntry(pMac) != eHAL_STATUS_SUCCESS)
           {
               HALLOGW( halLog(pMac, LOGW, FL("Failed at halRxp_addBroadcastEntry() \n")));
               rc = eSIR_FAILURE;
               break;
           }
            }
            // Start HAL timers create
            if ((rc = halTimersCreate(pMac)) != eSIR_SUCCESS)
                break;

#if defined(ANI_LED_ENABLE)
            // Init LED.
            halInitLed(pMac);
#endif
            // Post NIC_OPERATIONAL message to HDD
            msg.type = SIR_HAL_NIC_OPER_NTF;
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&mb, 8))
            {
                rc = eSIR_MEM_ALLOC_FAILED;
                HALLOGW( halLog(pMac, LOGP,
                       FL("Alloc failed for OPER_NTF\n")));
                break;
            }
#if defined (ANI_PRODUCT_TYPE_AP) && defined (ANI_LITTLE_BYTE_ENDIAN)
            sirStoreU16N((tANI_U8*)mb, SIR_HAL_NIC_OPER_NTF);
            sirStoreU16N(((tANI_U8*)mb+2), 8);
            sirStoreU32N(((tANI_U8*)mb+4), 0);
#else
            mb->type    = SIR_HAL_NIC_OPER_NTF;
            mb->msgLen  = 8;    // len in bytes
            mb->data[0] = 0;
#endif
            msg.bodyptr = mb;
            msg.bodyval = 0;
            halMmhPostMsgApi(pMac, &msg, eHI_PRI);
            //if we have already received sys_ready then we can go to normal state.
            if(eHAL_SYS_READY == halStateGet(pMac))
                halSetReadyToHandleInt(pMac);
            else
                halStateSet(pMac, eHAL_STARTED);

            HALLOGP( halLog(pMac, LOGE,
                   FL("halProcessStartEvent: Completed, State %d!\n"),
                    halStateGet(pMac)));
#if 0
#ifndef LIBRA_RF
            {
                tANI_U32 i;

                for (i=0; i< sizeof(aLibraRFSetup)/sizeof(aLibraRFSetup[0]); i++)
                    halWriteRegister(pMac, aLibraRFSetup[i].reg, aLibraRFSetup[i].val);

            }
#endif
#endif
        }
    }
    while (0);

    return rc;

} // halProcessStartEvent


// -------------------------------------------------------------
/**
 * halProcessCfgDownloadComplete
 *
 * FUNCTION:
 *     Processing performed after configuration is done.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC Global instance
 * @return tSirRetStatus SUCCESS or FAILURE
 */
tSirRetStatus halProcessCfgDownloadComplete(tpAniSirGlobal pMac)
        {
//#ifdef WLAN_DEBUG
//     tHalRaBssInfo bssRaInfo;
//#endif
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U32 val;

    pMac->hal.halMac.macStats.periodicStats = false; //FALSE by default. Use Dump to enable.
    if ((rc = wlan_cfgGetInt(pMac, WNI_CFG_STATS_PERIOD,
                        &val)) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("CFG Failed STATS collection period\n")));
        macSysResetReq(pMac, rc);
        goto end;
    }

    pMac->hal.halMac.macStats.statTmrVal = SYS_SEC_TO_TICKS(val);
    pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
    pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);

    if(pMac->gDriverType != eDRIVER_TYPE_MFG) // Enable periodic calibration only if it is not the Manufacturing Diagnostics
    {
                      // driver build.
    rc = halConfigCalControl(pMac);
    if (rc != eSIR_SUCCESS)
    {
        HALLOGE(halLog(pMac, LOGE, FL("halConfigCalControl: CFG Failed Calibration Control\n")));
        macSysResetReq(pMac, rc);
        goto end;
    }
    //pMac->hal.trigCalFlag = (tANI_U8) val;

    }

#ifdef FEATURE_ON_CHIP_REORDERING
    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_RPE_POLLING_THRESHOLD, &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_RPE_POLLING_THRESHOLD\n")));
    }
    else
    {
      halRPE_UpdateOnChipReorderThreshold(pMac, WNI_CFG_RPE_POLLING_THRESHOLD, val);
    }

    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC0_REG , &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC0_REG\n")));
    }
    else
    {
      halRPE_UpdateOnChipReorderThreshold(pMac, WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC0_REG, val);
    }

    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC1_REG , &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC1_REG\n")));
    }
    else
    {
      halRPE_UpdateOnChipReorderThreshold(pMac, WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC1_REG, val);
    }

    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC2_REG , &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC2_REG\n")));
    }
    else
    {
      halRPE_UpdateOnChipReorderThreshold(pMac, WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC2_REG, val);
    }

    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC3_REG , &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC3_REG\n")));
    }
    else
    {
      halRPE_UpdateOnChipReorderThreshold(pMac, WNI_CFG_RPE_AGING_THRESHOLD_FOR_AC3_REG, val);
    }

    if ((rc = wlan_cfgGetInt(pMac,WNI_CFG_NO_OF_ONCHIP_REORDER_SESSIONS, &val)) != eSIR_SUCCESS)
    {
      HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_NO_OF_ONCHIP_REORDER_SESSIONS\n")));
    }
    else
    {
      pMac->hal.halMac.maxNumOfOnChipReorderSessions = val;
      if(pMac->hal.halMac.maxNumOfOnChipReorderSessions > MAX_NUM_OF_ONCHIP_REORDER_SESSIONS)
        pMac->hal.halMac.maxNumOfOnChipReorderSessions = MAX_NUM_OF_ONCHIP_REORDER_SESSIONS;
    }
#endif /* FEATURE_ON_CHIP_REORDERING */

    if ((rc = wlan_cfgGetInt(pMac, WNI_CFG_CAL_PERIOD, &val)) != eSIR_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Failed to get CFG CAL PERIOD\n")));
        macSysResetReq(pMac, rc);
        goto end;
    }
    else
    {
        if (val == 0)
        {
            pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);
        }
        else
        {
            pMac->hal.halMac.tempMeasTmrVal = SYS_MIN_TO_TICKS(val)/HAL_PHY_PERIODIC_CAL_ITER_LIMIT;
        }
    }
    halStateSet(pMac, eHAL_CFG);
end:

    /** Initialize the Firmware Heart Beat Monitor Values.*/
    pMac->hal.halMac.fwMonitorthr = 0;
    pMac->hal.halMac.fwHeartBeatPrev = 0;

    /** Initialize the Phy MPI tx counter values to detect PHY hang.*/
    pMac->hal.halMac.phyHangThr = 0;
    pMac->hal.halMac.mpiTxSent = 0;
    pMac->hal.halMac.mpiTxAbort = 0;

//    HALLOGE(halLog(pMac, LOGE,FL("###### Sizeof bssRaInfo %d\n"), sizeof(bssRaInfo)));
    // Post START event to HAL's event queue
    rc = halProcessStartEvent(pMac);

    return rc;
}

//dummy function for now to register to BAL as fatal error callback.
VOS_STATUS halFatalErrorHandler(v_PVOID_t pVosGCtx, v_U32_t errorCode)
{
    return VOS_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn halProcessSysReadyInd
\brief handles the notification from HDD forwaded by PE.
\        right now here we just enable all the interrupts.
\param   tpAniSirGlobal pMac
\return eSirRetStatus - status
  -------------------------------------------------------------*/
tSirRetStatus halProcessSysReadyInd(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    //if we have already received start event then we can enable interrupts and
    //change HAL state to normal.
    if(eHAL_STARTED == halStateGet(pMac))
        halSetReadyToHandleInt(pMac);
    else
        halStateSet(pMac, eHAL_SYS_READY);

    HALLOGW( halLog(pMac, LOGW, FL("After SYS_READY process halState = %d\n"),
          halStateGet(pMac)));
    return rc;
}

/** -------------------------------------------------------------
\fn halProcessMulticastRateChange
\brief handles the CFG change for mlticast rates.
\param  tpAniSirGlobal pMac
\param  tANI_U32 cfgId
\return eHalStatus status
  -------------------------------------------------------------*/
static
eHalStatus halProcessMulticastRateChange(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;
        eRfBandMode curBand;
        curBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);
        //if the multicast rate got changed for the current band then inform softmac.
        if(((WNI_CFG_FIXED_RATE_MULTICAST_24GHZ == cfgId) &&
            (eRF_BAND_2_4_GHZ == curBand)) ||
          ((WNI_CFG_FIXED_RATE_MULTICAST_5GHZ == cfgId) &&
            (eRF_BAND_5_GHZ == curBand)))
        {
        if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
        {
            HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
            return eHAL_STATUS_FAILURE;
        }
        else
        {
            tTpeRateIdx tpeRateIdx = halRate_cfgFixedRate2TpeRate(val);
            halSetMulticastRateIdx(pMac, tpeRateIdx);
        }
    }
    return status;
}

// -------------------------------------------------------------
/**
 * halInitStartReqHandler
 *
 * FUNCTION:
 *     Handles init start request
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

tSirRetStatus halInitStartReqHandler(tpAniSirGlobal pMac)
{
    tSirRetStatus rc = eSIR_SUCCESS;
    tANI_U8 halState = halStateGet(pMac);

    if (halState != eHAL_INIT)
    {
        HALLOGP( halLog(pMac, LOGE,
               FL("halInitStartReqHandler: INIT_START_REQ: Invalid HAL state %d\n"),
               halState));
        rc = eSIR_FAILURE;
        goto end;
    }


    /* Initializes all static HW initializations & some of the
     * global variables.
     * Enable BMU command processing
     * Enable all the master work queues
     * Enable RHP for receiving packets on the receive path.
     * Enables Interrupts
     */

#ifdef ANI_OS_TYPE_RTAI_LINUX

    // Resume all tasks
    if (tx_thread_resume(&pMac->sys.gSirMntThread) != TX_SUCCESS)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Could not resume MNT thread!\n")));
        rc = eSIR_SYS_TX_THREAD_RESUME_FAILED;
        macSysResetReq(pMac, rc);
    }
#endif
    // Initializes and performs config download
    rc = halDoCfgInit(pMac);

  end:
    return rc;
}

/** ------------------------------------------------------
\fn      halProcessMsg
\brief   This function process HAL messages
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  *pMsg - HAL message to be processed
\return  status
\ -------------------------------------------------------- */
tSirRetStatus halProcessMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg )
{
    tSirRetStatus   rc = eSIR_SUCCESS;
    tHalMsgDecision  msgStatus;
    tANI_U8 mutexAcquired = FALSE;

    // If hal state is IDLE, do not process any messages.
    // free the body pointer and return success
    if(pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
    {
    if(eHAL_IDLE == halStateGet(pMac)) {
        if(pMsg->bodyptr) {
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
        }
        return eSIR_SUCCESS;
    }
    }

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
    tANI_U32                    pttType;
    tPttMsgbuffer               *pPttMsg;
    tANI_U8                     *pReq;
    pReq = (tANI_U8*) pMsg->bodyptr;
    pPttMsg = (tPttMsgbuffer *)pReq;
    pttType = pMsg->type & HAL_MMH_MB_MSG_TYPE_MASK;

        if (pttType >= PTT_MSG_TYPES_BEGIN_30 &&  pttType <= PTT_MSG_TYPES_BEGIN_32)
        {
            pttProcessMsg(pMac, pPttMsg);
            return(rc);
        }
        //return rc;
    }
#endif

    msgStatus = halUtil_MsgDecision(pMac, pMsg, &mutexAcquired);
    if (msgStatus == eHAL_MSG_DROP) {
        /**@todo free message.*/
        return eSIR_SUCCESS;
    }

    if (msgStatus == eHAL_MSG_DEFER)
    {
      if(eHAL_STATUS_SUCCESS != halUtil_deferMsg(pMac, pMsg))
      {
        rc = eSIR_FAILURE;
      }
    }
    else
    {

        rc = halHandleMsg(pMac, pMsg);

//        halUtil_processDeferredMsgQ(pMac);

        // Release the mutex if acquired.
        if (mutexAcquired) {
            halPS_ReleaseHostBusy(pMac, HAL_PS_BUSY_GENERIC);
        }
    }

    return rc;
}


/** ------------------------------------------------------
\fn      halHandleMsg
\brief   This function process HAL messages
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  *pMsg - HAL message to be processed
\return  status
\ -------------------------------------------------------- */
tSirRetStatus halHandleMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg )
{
    tSirRetStatus   rc = eSIR_SUCCESS;
    eHalStatus      status = eHAL_STATUS_SUCCESS;
    tANI_U32        val;
    tANI_U16        dialogToken = pMsg->reserved;

#ifndef WLAN_FTM_STUB
    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
    switch (pMsg->type)
    {
           case SIR_HAL_HANDLE_FW_MBOX_RSP:
                HALLOGE( halLog(pMac, LOGE, FL("Recvd new Msg (or Rsp) from FW \n")));
                halFW_HandleFwMessages(pMac, pMsg->bodyptr);
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;
                break;

           case SIR_HAL_SEND_MSG_COMPLETE:
                HALLOGE( halLog(pMac, LOGE, FL("Recv ACK from FW for the host Msg \n")));
                halMbox_SendMsgComplete(pMac);
                break;

           case SIR_HAL_TIMER_ADC_RSSI_STATS:
                tx_timer_deactivate(&pMac->ptt.adcRssiStatsTimer);
                halPhyAdcRssiStatsCollection(pMac);
                tx_timer_activate(&pMac->ptt.adcRssiStatsTimer);
                break;
        }
        return eSIR_SUCCESS;
    }
#endif

    switch (pMsg->type)
    {
        case SIR_HAL_INIT_START_REQ:
            {
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;

                rc = halInitStartReqHandler(pMac);
            }
            break;

        case SIR_HAL_HDD_ADDBA_RSP:
            status = baProcessTLAddBARsp(pMac,
                            ((tpAddBARsp)pMsg->bodyptr)->baSessionID,
                            ((tpAddBARsp)pMsg->bodyptr)->replyWinSize
                            #ifdef FEATURE_ON_CHIP_REORDERING
                            ,eANI_BOOLEAN_FALSE
                            #endif
                            );
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;

        case SIR_TL_HAL_FLUSH_AC_REQ:
            // TL is requesting a flush operation.
            rc = halTLProcessFlushReq(pMac, pMsg);
            break;

        case SIR_HAL_SYS_READY_IND:
            {
                vos_mem_free((v_VOID_t*)pMsg->bodyptr);
                pMsg->bodyptr = NULL;

                rc = halProcessSysReadyInd(pMac);
            }
            break;

        case SIR_CFG_PARAM_UPDATE_IND:
            if (wlan_cfgGetInt(pMac, (tANI_U16) pMsg->bodyval, &val) != eSIR_SUCCESS)
            {
                HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), pMsg->bodyval));
                return eSIR_FAILURE;
            }

            switch (pMsg->bodyval)
            {

                case WNI_CFG_STA_ID:
                    if( (rc = halMsg_AddStaSelf(pMac)) != eHAL_STATUS_SUCCESS)
                        HALLOGW( halLog(pMac, LOGW, FL("halMsg_AddStaSelf() failed \n")));
                    break;

                case WNI_CFG_PACKET_CLASSIFICATION:
                    pMac->hal.halMac.frameClassifierEnabled = (tANI_U16) val;
                    break;

                case WNI_CFG_RTS_THRESHOLD:
                    if ( wlan_cfgGetInt(pMac, WNI_CFG_RTS_THRESHOLD, &val) != eSIR_SUCCESS) {
                        HALLOGE(halLog(pMac, LOGE, FL("cfgGet WNI_CFG_RTS_THRESHOLD Failed\n")));
                    }

                    if (halMsg_UpdateTpeProtectionThreshold(pMac, val) != eHAL_STATUS_SUCCESS) {
                        return eSIR_FAILURE;
                    }
                    break;

                case WNI_CFG_SHORT_RETRY_LIMIT:
                case WNI_CFG_LONG_RETRY_LIMIT:
                    if( (status = halMsg_updateRetryLimit(pMac)) != eHAL_STATUS_SUCCESS){
                        HALLOGW( halLog(pMac, LOGW, FL("halMsg_sendWmParam() failed \n")));
                    }
                    break;

                case WNI_CFG_FRAGMENTATION_THRESHOLD:
                    if ( (status = halMsg_updateFragThreshold(pMac)) != eHAL_STATUS_SUCCESS){
                        HALLOGW( halLog(pMac, LOGW, FL("halDPU_updateFragThreshold() failed \n")));
                    }
                    break;

                case WNI_CFG_CURRENT_TX_ANTENNA:
                    pMac->hal.cfgTxAntenna = val;
                    {
#ifdef CHAIN_SEL_CAPABLE
                        ePhyChainSelect chainSelect = halPhyGetChainSelect(pMac, (tANI_U8)pMac->hal.cfgTxAntenna, (tANI_U8)pMac->hal.cfgRxAntenna);
                        if (chainSelect != INVALID_PHY_CHAIN_SEL)
                        {
                            if (halPhySetChainSelect(pMac, chainSelect) != eHAL_STATUS_SUCCESS){
                                HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect() failed \n")));
                            }
                        }
                        else
                        {
                            HALLOGE( halLog(pMac, LOGE, FL("WNI_CFG_CURRENT_TX_ANTENNA: Incorrect chain selection")));
                        }
#endif
                    }
                    break;


                case WNI_CFG_CURRENT_RX_ANTENNA:
                    pMac->hal.cfgRxAntenna = val;
                    {
#ifdef CHAIN_SEL_CAPABLE
                        ePhyChainSelect chainSelect = halPhyGetChainSelect(pMac, (tANI_U8)pMac->hal.cfgTxAntenna, (tANI_U8)pMac->hal.cfgRxAntenna);

                        if (chainSelect != INVALID_PHY_CHAIN_SEL)
                        {
                            if (halPhySetChainSelect(pMac, chainSelect) != eHAL_STATUS_SUCCESS){
                                HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect() failed \n")));
                            }
                        }
                        else
                        {
                            HALLOGE( halLog(pMac, LOGE, FL("WNI_CFG_CURRENT_RX_ANTENNA: Incorrect chain selection")));
                        }
#endif
                    }
                    break;

                case WNI_CFG_LOW_GAIN_OVERRIDE:
                    HALLOGE( halLog(pMac, LOGE, FL("WNI_CFG_LOW_GAIN_OVERRIDE : %s\n"),
                            (val == 0)? "OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE" : "OPEN_LOOP_TX_LOW_GAIN_OVERRIDE"));
                    if ( (status = halPhyUpdateTxGainOverride(pMac, (val == 0) ? OPEN_LOOP_TX_HIGH_GAIN_OVERRIDE :
                                                                OPEN_LOOP_TX_LOW_GAIN_OVERRIDE)) != eHAL_STATUS_SUCCESS){
                        HALLOGE( halLog(pMac, LOGE, FL("halPhyUpdateTxGainOverride() failed \n")));
                    }
                    break;

                case WNI_CFG_POWER_STATE_PER_CHAIN:
                    halSetChainPowerState(pMac);
                    break;

               case WNI_CFG_CAL_PERIOD:
                       rc = halConfigCalPeriod(pMac);
                    break;

                case WNI_CFG_CAL_CONTROL:
                       rc = halConfigCalControl(pMac);
                    break;

                case WNI_CFG_STATS_PERIOD:
                    tx_timer_deactivate(&pMac->hal.halMac.wrapStats.statTimer);
                    pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
                    tx_timer_change(&pMac->hal.halMac.wrapStats.statTimer,
                                    pMac->hal.halMac.wrapStats.statTmrVal,
                                    pMac->hal.halMac.wrapStats.statTmrVal);
                    tx_timer_activate(&pMac->hal.halMac.wrapStats.statTimer);

#ifdef FIXME_GEN5
                    tx_timer_deactivate(&pMac->hal.halMac.macStats.statTimer);
                    pMac->hal.halMac.macStats.statTmrVal = SYS_SEC_TO_TICKS(val);
                    HALLOGW( halLog(pMac, LOGW, FL("WNI_CFG_STATS_PERIOD %d seconds\n"),
                           val));
                    tx_timer_change(&pMac->hal.halMac.macStats.statTimer,
                                    pMac->hal.halMac.macStats.statTmrVal,
                                    pMac->hal.halMac.macStats.statTmrVal);
                    tx_timer_activate(&pMac->hal.halMac.macStats.statTimer);
#endif
                    break;

                case WNI_CFG_CFP_MAX_DURATION:
                    break;

                // ----------------------------
                // Rate Adaptation related CFG
                // ----------------------------
                case WNI_CFG_DYNAMIC_THRESHOLD_ZERO:
                    HALLOG4( halLog(pMac, LOG4, FL("setDynamicThresh0: %d\n"),  val ));
                    break;

                case WNI_CFG_DYNAMIC_THRESHOLD_ONE:
                    HALLOG4( halLog( pMac, LOG4, FL("setDynamicThresh1: %d\n"),  val ));
                    break;

                case WNI_CFG_PROXIMITY:
                case WNI_CFG_NETWORK_DENSITY:
                    halPhy_setNwDensityAndProximity(pMac);
                    halATH_setAlgorithm(pMac);
                    break;

                case WNI_CFG_ADAPTIVE_THRESHOLD_ALGORITHM:
                    halATH_setAlgorithm(pMac);
                    break;

                case WNI_CFG_FIXED_RATE:
                case WNI_CFG_RETRYRATE_POLICY:
                case WNI_CFG_FORCE_POLICY_PROTECTION:
                    halMacRaCfgChange(pMac, pMsg->bodyval);
                    break;

                case WNI_CFG_BA_TIMEOUT:
                case WNI_CFG_MAX_BA_BUFFERS:
                case WNI_CFG_MAX_BA_SESSIONS:
                case WNI_CFG_BA_THRESHOLD_HIGH:
                case WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT:
                case WNI_CFG_BA_AUTO_SETUP:
                case WNI_CFG_MAX_MEDIUM_TIME:
                case WNI_CFG_MAX_MPDUS_IN_AMPDU:
                    baHandleCFG( pMac, pMsg->bodyval );
                    break;

                case WNI_CFG_FIXED_RATE_MULTICAST_24GHZ:
                case WNI_CFG_FIXED_RATE_MULTICAST_5GHZ:
                    halProcessMulticastRateChange(pMac, pMsg->bodyval);
                    break;
/* In firmware RA, this is not supported.
        case WNI_CFG_RA_PERIODICITY_TIMEOUT_IN_PS:
                     halRAHandleCfg( pMac, pMsg->bodyval);
                     break;
*/
                case WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT:
            halPSDataInActivityTimeout(pMac, pMsg->bodyval);
            break;

        case WNI_CFG_PS_ENABLE_HEART_BEAT:
            halPSFWHeartBeatCfg(pMac, pMsg->bodyval);
            break;

        case WNI_CFG_PS_ENABLE_BCN_FILTER:
            halPSBcnFilterCfg(pMac, pMsg->bodyval);
            break;

#ifdef WLAN_SOFTAP_FEATURE
        case WNI_CFG_ENABLE_PHY_AGC_LISTEN_MODE:
            halHandleEnableListenModeCfg(pMac, pMsg->bodyval);         
            break;
#endif  
                case WNI_CFG_PS_ENABLE_RSSI_MONITOR:
                     halPSRssiMonitorCfg(pMac, pMsg->bodyval);
                     break;
           
                case WNI_CFG_MCAST_BCAST_FILTER_SETTING:
                     halHandleMcastBcastFilterSetting(pMac, pMsg->bodyval);
                   break;
            
                case WNI_CFG_DYNAMIC_PS_POLL_VALUE:
                   halHandleDynamicPsPollValue(pMac, pMsg->bodyval);
                   break;
            
       
                case WNI_CFG_RF_SETTLING_TIME_CLK:
                      halPSRfSettlingTimeClk(pMac, pMsg->bodyval);
                      break;
                
               case WNI_CFG_PS_NULLDATA_AP_RESP_TIMEOUT:
                     halPSNullDataAPProcessDelay(pMac, pMsg->bodyval);
                     break;

                default:
                    HALLOGE( halLog(pMac, LOGE, FL("Cfg Id %d is not handled\n"), pMsg->bodyval));
                    break;
            }

            break;

        case SIR_CFG_DOWNLOAD_COMPLETE_IND:
            {
                rc = halProcessCfgDownloadComplete(pMac);
            }
            break;

        /*
         * Taurus related messages
         */
        case SIR_HAL_ADD_STA_REQ:
            halMsg_AddSta(pMac, pMsg->reserved, (tpAddStaParams) (pMsg->bodyptr), eANI_BOOLEAN_TRUE);
            break;

        case SIR_HAL_DELETE_STA_REQ:
            halMsg_DelSta(pMac, pMsg->reserved, (tpDeleteStaParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_STARATEINFO_REQ:
            halMsg_UpdateTxCmdTemplate(pMac, pMsg->reserved, (tpUpdateTxCmdTemplParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_ADD_BSS_REQ:
            halMsg_AddBss(pMac, pMsg->reserved, (tpAddBssParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DELETE_BSS_REQ:
            halMsg_DelBss(pMac, pMsg->reserved, (tpDeleteBssParams) (pMsg->bodyptr));
            break;

#ifdef WLAN_SOFTAP_FEATURE
        case SIR_HAL_UPDATE_UAPSD_IND:
            halMsg_UpdateUapsd(pMac, (tpUpdateUapsdParams) (pMsg->bodyptr));
            break;
#endif

        case SIR_HAL_INIT_SCAN_REQ:
            halMsg_InitScan(pMac, pMsg->reserved, (tpInitScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_START_SCAN_REQ:
            halMsg_StartScan(pMac, pMsg->reserved, (tpStartScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_END_SCAN_REQ:
            halMsg_EndScan(pMac, pMsg->reserved, (tpEndScanParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_FINISH_SCAN_REQ:
            halMsg_FinishScan(pMac, pMsg->reserved, (tpFinishScanParams)(pMsg->bodyptr));
            break;

#ifdef FEATURE_INNAV_SUPPORT
        case SIR_HAL_START_INNAV_MEAS_REQ:
            halInNav_HandleStartInNavMeasReq(pMac, pMsg->reserved, (tpStartInNavMeasReq)(pMsg->bodyptr));
            break;

        case SIR_HAL_FINISH_INNAV_MEAS_REQ:
            halInNav_FinishInNavMeasReq(pMac);
            break;
#endif

        case SIR_HAL_SET_LINK_STATE:
            halMsg_ProcessSetLinkState(pMac, (tpLinkStateParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_SEND_BEACON_REQ:
            halMsg_SendBeacon(pMac, (tpSendbeaconParams)(pMsg->bodyptr));
            break;

#ifdef WLAN_SOFTAP_FEATURE
        case SIR_HAL_UPDATE_PROBE_RSP_TEMPLATE_IND:
            halMsg_UpdateProbeRspTemplate(pMac, (tpSendProbeRespParams)(pMsg->bodyptr));
            break;
#if 0 //separate setting for ie bitmap for probeRsp. Not in use right now.           
        case SIR_HAL_UPDATE_PROBE_RSP_IE_BITMAP_IND:
            halFW_UpdateProbeRspIeBitmap(pMac, (tpUpdateProbeRspIeBitmap)(pMsg->bodyptr));
            break;
#endif            
#endif

        case SIR_HAL_INIT_CFG_REQ:
            break;

        case SIR_HAL_INIT_WM_CFG_REQ:
            halInitWmParam((tHalHandle)pMac, NULL);
            break;

        case SIR_HAL_SET_BSSKEY_REQ:
             halMsg_SetBssKey(pMac, pMsg->reserved, (tpSetBssKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_SET_STAKEY_REQ:
            halMsg_SetStaKey(pMac, pMsg->reserved, (tpSetStaKeyParams) (pMsg->bodyptr));
            break;

       case SIR_HAL_SET_STA_BCASTKEY_REQ:
                halMsg_SetStaBcastKey(pMac, pMsg->reserved, (tpSetStaKeyParams) (pMsg->bodyptr));
                break;

        case SIR_HAL_REMOVE_BSSKEY_REQ:
            halMsg_RemoveBssKey(pMac, pMsg->reserved, (tpRemoveBssKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_REMOVE_STAKEY_REQ:
            halMsg_RemoveStaKey(pMac, pMsg->reserved, (tpRemoveStaKeyParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DPU_STATS_REQ:
            halMsg_GetDpuStats(pMac, pMsg->reserved, (tpDpuStatsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_GET_DPUINFO_REQ:
            halMsg_GetDpuParams(pMac, pMsg->reserved, (tpGetDpuParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_EDCA_PROFILE_IND:
            halMsg_updateEdcaParam(pMac, (tEdcaParams *) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_BEACON_IND :
            halMsg_updateBeaconParam(pMac, (tpUpdateBeaconParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_UPDATE_CF_IND:
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;

            break;

        case SIR_HAL_CHNL_SWITCH_REQ:
            halMsg_ChannelSwitch(pMac, (tpSwitchChannelParams)(pMsg->bodyptr));
            //palFreeMemory( pMac->hHdd, (tANI_U8 *) pMsg->bodyptr );
            break;

        case SIR_HAL_SET_TX_POWER_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Set Tx Power Request \n")));
            halMsg_setTxPower(pMac, (tpSirSetTxPowerReq)pMsg->bodyptr);
            break;

        case SIR_HAL_GET_TX_POWER_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Get Tx Power Request \n")));
            halMsg_getTxPower(pMac, (tpSirGetTxPowerReq)pMsg->bodyptr);
            break;

        case SIR_HAL_SET_KEY_DONE:
            HALLOGW( halLog(pMac, LOGW, FL("Set Key Done \n")));
            halMsg_SetKeyDone(pMac, pMsg->bodyval);
            break;

       case SIR_HAL_HANDLE_FW_MBOX_RSP:
            HALLOGE( halLog(pMac, LOGE, FL("Recvd Msg (or Rsp) from FW \n")));
            halFW_HandleFwMessages(pMac, pMsg->bodyptr);
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;

       case SIR_HAL_SEND_MSG_COMPLETE:
            HALLOGE( halLog(pMac, LOGE, FL("Recv ACK from FW for the host Msg \n")));
            halMbox_SendMsgComplete(pMac);
            break;

       case SIR_HAL_GET_NOISE_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Get Noise Request \n")));
            halMsg_sendGetNoiseRsp(pMac);
            break;

// Start of Power Save related messages
        case SIR_HAL_PWR_SAVE_CFG:
            status = halPS_Config(pMac, (tpSirPowerSaveCfg)pMsg->bodyptr);
            break;

        case SIR_HAL_ENTER_IMPS_REQ:
            status = halPS_HandleEnterImpsReq(pMac, dialogToken);
            break;

        case SIR_HAL_EXIT_IMPS_REQ:
            status = halPS_HandleExitImpsReq(pMac, dialogToken);
            break;

        case SIR_HAL_POSTPONE_ENTER_IMPS_RSP:
            status = halPS_HandleFwEnterImpsRsp(pMac, pMsg->bodyptr);
            break;

        case SIR_HAL_ENTER_BMPS_REQ:
            status = halPS_HandleEnterBmpsReq(pMac, dialogToken, (tpEnterBmpsParams)pMsg->bodyptr);
        break;

        case SIR_HAL_EXIT_BMPS_REQ:
            status = halPS_HandleExitBmpsReq(pMac, dialogToken, (tpExitBmpsParams)pMsg->bodyptr);
            break;

        case SIR_HAL_SUSPEND_BMPS:
            status = halPS_SuspendBmps(pMac, dialogToken, NULL, NULL);
            break;

        case SIR_HAL_RESUME_BMPS:
            status = halPS_ResumeBmps(pMac, dialogToken, NULL, NULL, FALSE);
            break;

        case SIR_HAL_ENTER_UAPSD_REQ:
            status = halPS_HandleEnterUapsdReq(pMac, dialogToken,
                    (tpUapsdParams)pMsg->bodyptr);
            break;

        case SIR_HAL_EXIT_UAPSD_REQ:
            status = halPS_HandleExitUapsdReq(pMac, dialogToken);
            break;

        case SIR_HAL_BEACON_FILTER_IND:
            status = halPS_HandleAddBeaconFilter(pMac, dialogToken,
                    (void*)pMsg->bodyptr);
            break;

        case SIR_HAL_WOWL_ADD_BCAST_PTRN:
            status = halPS_AddWowlPatternToFw(pMac, (tpSirWowlAddBcastPtrn)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_DEL_BCAST_PTRN:
            status = halPS_RemoveWowlPatternAtFw(pMac, (tpSirWowlDelBcastPtrn)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_ENTER_REQ:
            status = halPS_EnterWowlReq(pMac, dialogToken, (tpSirHalWowlEnterParams)(pMsg->bodyptr));
            break;

        case SIR_HAL_WOWL_EXIT_REQ:
            status = halPS_ExitWowlReq(pMac, dialogToken);
            break;

// End of Power Save releated messages

        case SIR_HAL_ADD_TS_REQ:
            halMsg_AddTs(pMac, pMsg->reserved, (tpAddTsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DEL_TS_REQ:
            halMsg_DelTs(pMac, pMsg->reserved, (tpDelTsParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_REGISTER_PE_CALLBACK:
            if (pMsg->bodyptr)
            {
                halMsg_RegisterPECallback(pMac, pMsg->bodyptr);
            }
            break;

        case SIR_HAL_ADDBA_REQ:
            halMsg_AddBA(pMac, pMsg->reserved, (tpAddBAParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_DELBA_IND:
            halMsg_DelBA(pMac, pMsg->reserved, (tpDelBAParams) (pMsg->bodyptr));
            break;

        case SIR_HAL_BA_FAIL_IND:
            halMsg_BAFail(pMac, pMsg->reserved, (tpAddBARsp) (pMsg->bodyptr));
            break;

#ifdef ANI_SUPPORT_SMPS
        case SIR_HAL_SET_MIMOPS_REQ:
            halMsg_SetMimoPs(pMac, (tpSetMIMOPS)(pMsg->bodyptr));
            break;
#endif

#ifndef WLAN_SOFTAP_FEATURE
        case SIR_HAL_BEACON_PRE_IND:
            halMsg_BeaconPre(pMac);
            break;
#endif 

        case SIR_HAL_STA_STAT_REQ:
        case SIR_HAL_AGGR_STAT_REQ:
        case SIR_HAL_GLOBAL_STAT_REQ:
        case SIR_HAL_STAT_SUMM_REQ:
            halHandleStatsReq(pMac, pMsg->type, (tpAniGetStatsReq) (pMsg->bodyptr));
            break;

        case SIR_HAL_GET_STATISTICS_REQ:
            halHandlePEStatisticsReq(pMac, pMsg->type, (tpAniGetPEStatsReq) (pMsg->bodyptr));
            break;

        /** ---- This timer messages posted by MTU Timer. ---- */
        case SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND:
            halATH_adjustAdaptiveThreshold(pMac);
            break;

        /** ---- These are timer messages posted by HAL Timer. ---- */
        case SIR_HAL_TIMER_TEMP_MEAS_REQ:
            if ((pMac->hphy.phy.phyPeriodicCalEnable)
                    && (! pMac->hphy.phy.test.testDisableRfAccess))
            {
                halPerformTempMeasurement(pMac);
            }
            break;

        case SIR_HAL_TIMER_BA_ACTIVITY_REQ:
            halBaCheckActivity(pMac);
            break;

        case SIR_HAL_TIMER_PERIODIC_STATS_COLLECT_REQ:
            halMacPeriodicStatCollection(pMac);
            break;

        case SIR_HAL_TIMER_WRAP_AROUND_STATS_COLLECT_REQ:
            //halMacWrapAroundStatCollection(pMac);
            /**
                    * Periodic stat Timer does not work.
                    * So deactivating it and acitvating again
                    */
                    tx_timer_deactivate(&pMac->hal.halMac.wrapStats.statTimer);
            //pMac->hal.halMac.wrapStats.statTmrVal = SYS_SEC_TO_TICKS(val);
            //halLog(pMac, LOGW, FL("WNI_CFG_STATS_PERIOD %d seconds\n"),
            //       val);
            //tx_timer_change(&pMac->hal.halMac.wrapStats.statTimer,
            //                pMac->hal.halMac.wrapStats.statTmrVal,
            //                pMac->hal.halMac.wrapStats.statTmrVal);
            tx_timer_activate(&pMac->hal.halMac.wrapStats.statTimer);
            break;

        case SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT:
            halMsg_ChipMonitorTimeout(pMac);
            /**
            * Periodic chip monitor does not work.
            * So deactivating it and acitvating again
            */
            tx_timer_deactivate(&pMac->hal.halMac.chipMonitorTimer);
            tx_timer_activate(&pMac->hal.halMac.chipMonitorTimer);
            break;
        case SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ:
            halMsg_HandleTrafficActivity(pMac);
            break;
        /**  ---------  End of HAL Timer Messages.  ---------- */

        case SIR_HAL_DPU_MIC_ERROR:
            halDpu_MICErrorIndication(pMac);
            break;

        case SIR_HAL_TRANSMISSION_CONTROL_IND:
            halMsg_FrameTransmitControlInd(pMac, (void *) pMsg->bodyptr);
            break;

        case SIR_HAL_TX_COMPLETE_IND:
            halMsg_TXCompleteInd(pMac, pMsg->bodyval);
            break;
        case SIR_HAL_BTC_SET_CFG:
            halBtc_SetBtcCfg(pMac, (void *)pMsg->bodyptr);
            break;

        case SIR_HAL_SIGNAL_BT_EVENT:
            halBtc_SendBtEventToFW(pMac, (void *)pMsg->bodyptr);
            break;


#ifdef ANI_SUPPORT_5GHZ
        case SIR_HAL_INIT_RADAR_IND:
            halRadar_Init(pMac);
            break;
#endif

#ifdef WLAN_FEATURE_VOWIFI
        case SIR_HAL_SET_MAX_TX_POWER_REQ:
            HALLOGW( halLog(pMac, LOGW, FL("Got Set Tx Power Request \n")));
            halMsg_setTxPowerLimit(pMac, (tpMaxTxPowerParams)pMsg->bodyptr);
            break;
#endif /* WLAN_FEATURE_VOWIFI */

        case SIR_HAL_SET_HOST_OFFLOAD:
            halPS_SetHostOffloadInFw(pMac, (tpSirHostOffloadReq)pMsg->bodyptr);
            break;

        default:
            HALLOGW( halLog(pMac, LOGW, FL("Errored Type 0x%X\n"), pMsg->type));
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;
    }

    if (status != eHAL_STATUS_SUCCESS) {
        rc = eSIR_FAILURE;
    }

    HALLOG4( halLog(pMac, LOG4, FL("Success Returns!\n")));
    return rc;
} // halHandleMsg()


// --------------------------------------------------------
/**
 * halCleanup
 *
 * FUNCTION:
 *     Cleans up HAL state and timers.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 *
 * NOTE:
 *
 * @param pMac MAC parameter structure pointer
 * @return NONE
 */

void
halCleanup(tpAniSirGlobal pMac)
{
    halTimersDestroy(pMac);
    // Start up in eSYSTEM_STA_ROLE
    // This should get updated appropriately at a later stage
    halSetGlobalSystemRole(pMac, eSYSTEM_STA_ROLE);

    pMac->hal.currentChannel = 0;
    pMac->hal.currentCalReqd = (tANI_U8)NO_CALS;
    pMac->hal.currentRfBand = eRF_BAND_UNKNOWN;
    pMac->hal.currentCBState = PHY_SINGLE_CHANNEL_CENTERED;
    //resetting the halDeferMsgQ.
    //There will be memory leak here if there are messages in the queue.
    pMac->hal.halDeferMsgQ.size = 0;
    pMac->hal.halDeferMsgQ.read = 0;
    pMac->hal.halDeferMsgQ.write= 0;

    halStateSet(pMac, eHAL_IDLE);
    //halMac Wmm related initializations
    // Initializing TSPEC info table array
    palZeroMemory(pMac->hHdd, pMac->hal.halMac.tspecInfo, sizeof(tTspecTblEntry) * LIM_NUM_TSPEC_MAX);
    pMac->hal.halMac.tsActivityChkTmrStarted = 0;
    pMac->hal.halMac.frameClassifierEnabled = 0;

    //halMac BA related initializations.
    pMac->hal.halMac.baRxMaxAvailBuffers = 0;
    pMac->hal.halMac.baNumActiveSessions = 0;
    pMac->hal.halMac.baTimeout = 0;
    pMac->hal.halMac.baSetupThresholdHigh = 0;
    palZeroMemory(pMac->hHdd, pMac->hal.halMac.baSessionTable, sizeof(tRxBASessionTable) * BA_MAX_SESSIONS);
    pMac->hal.halMac.baAutoSetupEnabled = false;

    //adaptive threshold structure initialization.
    palZeroMemory(pMac->hHdd, &pMac->hal.halAdaptThresh, sizeof(tAniHalAdaptThresh));
}


/*
 * TAURUS related code.
 */
/*
 * halMsg_GenerateRsp - Generate response to PE for messages.
 */
void halMsg_GenerateRsp( tpAniSirGlobal pMac, tANI_U16 msgType, tANI_U16 dialog_token, void *pBodyptr, tANI_U32 bodyVal)
{
    tSirMsgQ msg;
    tANI_U32 status;

    msg.type        = msgType;
    msg.reserved    = dialog_token;
    msg.bodyval     = bodyVal;
    msg.bodyptr     = pBodyptr;

    status = limPostMsgApi(pMac, &msg);

    if (status != TX_SUCCESS)
        HALLOGP( halLog(pMac, LOGP, FL("Failed limPostMsgApi=0%X\n"), status));
    return;
}



/** -------------------------------------------------------------
\fn halGetDefaultAndMulticastRates
\brief reads config for defualt and multicast rates. Converts user config rate for multicast rate to softmac rate index.
\param   tpAniSirGlobal pMac
\param   eRfBandMode rfBand.
\param   tTpeRateIdx* pRateIndex
\param   tTpeRateIdx* pMcastRateIndex
\return eHalStatus - status
  -------------------------------------------------------------*/
eHalStatus halGetDefaultAndMulticastRates(tpAniSirGlobal pMac, eRfBandMode rfBand,
        tTpeRateIdx* pRateIndex, tTpeRateIdx* pMcastRateIndex)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U16 cfgMcastRateKey, cfgDefRateKey;
    tANI_U32 cfgRateIndex, rateIndex = TPE_RT_IDX_INVALID, defRate ;

        if (rfBand == eRF_BAND_2_4_GHZ ){
        cfgMcastRateKey = WNI_CFG_FIXED_RATE_MULTICAST_24GHZ;
        cfgDefRateKey = WNI_CFG_DEFAULT_RATE_INDEX_24GHZ;
        defRate = TPE_RT_IDX_11B_LONG_1_MBPS;
        }else{
        cfgMcastRateKey = WNI_CFG_FIXED_RATE_MULTICAST_5GHZ;
        cfgDefRateKey = WNI_CFG_DEFAULT_RATE_INDEX_5GHZ;
        defRate = TPE_RT_IDX_11A_6_MBPS;
        }

    // Read the multicast/broadcast rate from the CFG
    if (wlan_cfgGetInt(pMac, cfgMcastRateKey, &cfgRateIndex) == eSIR_SUCCESS) {
        rateIndex = halRate_cfgFixedRate2TpeRate(cfgRateIndex);
        if (rateIndex != TPE_RT_IDX_INVALID) {
            *pMcastRateIndex = (tTpeRateIdx)rateIndex;
        } else {
            HALLOGE(halLog(pMac, LOGE, FL("Default Mcast rate CFG is invalid rate %d !! Override to index %d. \n"), cfgRateIndex, defRate));
            *pMcastRateIndex = (tTpeRateIdx)defRate;
        }
    }
    else {
            HALLOGP( halLog(pMac, LOGP, FL("Get WNI_CFG_FIXED_RATE_MULTICAST_24GHZ failed \n")));
            return eHAL_STATUS_FAILURE;
    }

    // Read the default rate from the CFG
    if (wlan_cfgGetInt(pMac, cfgDefRateKey, &cfgRateIndex) == eSIR_SUCCESS) {
        rateIndex = halRate_cfgFixedRate2TpeRate(cfgRateIndex);
        // If from CFG the rate index given is an invalid one, override it
        if (rateIndex != TPE_RT_IDX_INVALID) {
            *pRateIndex = (tTpeRateIdx)rateIndex;
        } else {
            HALLOGE( halLog(pMac, LOGE, FL("Default rate CFG is invalid rate %d !! Override to index %d. \n"), cfgRateIndex, defRate));
            *pRateIndex = (tTpeRateIdx)defRate;
        }
    }
    else {
            return eHAL_STATUS_FAILURE;
    }
    return status;
}


eHalStatus halSetNewChannelParams(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    eRfBandMode oldRfBand;
    tMtuMode    mtuMode;

    oldRfBand = halUtil_GetRfBand(pMac, pMac->hal.currentChannel);

    if (pMac->hphy.setChanCntx.newRfBand != oldRfBand) {
        tTpeRateIdx rateIndex, mcastRateIndex;
        if ((status = halGetDefaultAndMulticastRates(pMac,
                        pMac->hphy.setChanCntx.newRfBand, &rateIndex,
                        &mcastRateIndex)) != eHAL_STATUS_SUCCESS) {
            HALLOGE( halLog(pMac, LOGE, FL("halGetDefaultAndMulticastRates failed with status code: %d\n"), status));
            return status;
        }

        halSetBcnRateIdx(pMac, rateIndex);
        halSetNonBcnRateIdx(pMac, rateIndex);
        halSetMulticastRateIdx(pMac, mcastRateIndex);
    }

    // Set the channel number to be carried in the RxBd
    halRxp_setChannel(pMac, pMac->hphy.setChanCntx.newChannel);

#ifdef FIXME_VOLANS
    //In Volans FPGA netlist 37, there is an issue related to CCA extension found. If this bit is set while BA session is to be setup, 
    //upstream traffic stopped and it seems TPE blocked. VI/HW team is working on this now. Disable CCA extension for now.
    // Configure Rxp when doing channel switch
    halRxp_setOperatingRfBand(pMac, pMac->hphy.setChanCntx.newRfBand);
#endif
    // Caching the new and band information
    pMac->hal.currentChannel = pMac->hphy.setChanCntx.newChannel;
    pMac->hal.currentRfBand  = pMac->hphy.setChanCntx.newRfBand;
    pMac->hal.currentCBState = pMac->hphy.setChanCntx.newCbState;
    pMac->hal.currentCalReqd = pMac->hphy.setChanCntx.newCalReqd;

    // Update MTU timing parameters when channel is changed and only
    // when MTU mode needs to be changed.
    mtuMode = halMTU_getMode(pMac);
    if((tMtuMode)pMac->hal.halMac.lastMtuMode != mtuMode) {
        halMTU_updateTimingParams(pMac, mtuMode);
    }
    return status;
}

/* -------------------------------------------------------
 * FUNCTION:  halPhy_ChangeChannel()
 *
 * NOTE:
 *  1) If RF band changes or invoked from halMsg_AddBss()
 *     then we need to update the default beaconRateIndex
 *     and nonBeaconRateIndex in the hal Global
 *     CFG table
 *  2) Cache this new channel and RF band info
 *  3) If we're using RF link, then:
  *      - set the new channel via halPhySetChannel()
 *       - update rateTable with new power value
 *       - update Calibration
 * -------------------------------------------------------
 */
eHalStatus halPhy_ChangeChannel(tpAniSirGlobal pMac,
        tANI_U8 newChannel, ePhyChanBondState newCbState,
        tANI_U8 calRequired, funcHalSetChanCB pFunc,
        void* pData, tANI_U16 dialog_token)
{
    eHalStatus     status;

    // Cache the channel information and the context to return
    pMac->hphy.setChanCntx.newChannel   = newChannel;
    pMac->hphy.setChanCntx.newRfBand    = halUtil_GetRfBand(pMac, newChannel);
    pMac->hphy.setChanCntx.newCbState   = newCbState;
    pMac->hphy.setChanCntx.newCalReqd   = calRequired;
    pMac->hphy.setChanCntx.pFunc        = pFunc;
    pMac->hphy.setChanCntx.pData        = pData;
    pMac->hphy.setChanCntx.dialog_token = dialog_token;

    // Check if the current channel is same as the channel requested
    if( (newChannel == pMac->hal.currentChannel) &&
        (newCbState == pMac->hal.currentCBState) &&
        (calRequired == pMac->hal.currentCalReqd)) {
        HALLOG1( halLog(pMac, LOG1,
                    FL("Channel(%d) and CB State(%d) are the same as the previous settings, so not changing.\n"),
                                              newChannel, newCbState));
        return eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN;
    }

    HALLOG1( halLog(pMac, LOG1, FL("(chId %d, band %d, cbState %d) --> (chId %d, band %d cbState %d) "),
           pMac->hal.currentChannel, pMac->hal.currentRfBand, pMac->hal.currentCBState,
                newChannel, pMac->hphy.setChanCntx.newRfBand, newCbState));

#ifdef ANI_PRODUCT_TYPE_AP
    halRadar_SetInterrupt((tHalHandle) pMac, eANI_BOOLEAN_FALSE);
#endif

        halPhyDisableAllPackets(pMac);
    if (newCbState == PHY_SINGLE_CHANNEL_CENTERED) {
            halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_FORCED_ON);
    } else {
            halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20);
    }

        HALLOGW( halLog(pMac, LOGW, FL("halPhySetChannel(channel %d, cbState %d) \n"), newChannel, newCbState));
        status = halPhySetChannel(pMac, newChannel, newCbState, calRequired);
#ifdef FW_PRESENT
    return status;
#else
    return eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN;
#endif
}


void halPhy_HandleSetChannelRsp(tHalHandle hHal,  void* pFwMsg)
{
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
    Qwlanfw_SetChannelRspType *setChanRsp = (Qwlanfw_SetChannelRspType *)pFwMsg;
    tpPhySetChanCntx pSetChanCtx = &pMac->hphy.setChanCntx;


    // Check the FW response status
    if(setChanRsp->uStatus == eHAL_STATUS_SUCCESS) {
        //convert it to phy index before saving it
        pMac->hphy.rf.curChannel = rfGetChannelIndex(pMac->hphy.setChanCntx.newChannel, pMac->hphy.setChanCntx.newCbState);

        // Set few halMac parameters for the new channel
        (void) halSetNewChannelParams(pMac);

    } else {
            HALLOGE( halLog(pMac, LOGE, FL("halPhySetChannel failed with status code: %d\n"), status));
        status = eHAL_STATUS_FAILURE;

        if (pMac->hal.currentCBState == PHY_SINGLE_CHANNEL_CENTERED) {
                halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_FORCED_ON);
        } else {
                halPhySetAgcCCAMode(pMac, PHY_CCA_ED_OR_CD_AND_CS, PHY_CCA_SEC_ED40_AND_NOR_PKTDET40_PKTDET20);
        }
        }
        //Enable the packet reception.
    (void)halPhySetRxPktsDisabled(pMac, pMac->hphy.modTypes);
    // Resume to the context of the caller
    pSetChanCtx->pFunc(pMac, pSetChanCtx->pData, status, pSetChanCtx->dialog_token);

    return;
}

void halForceSetNwType(tpAniSirGlobal pMac, tSirNwType forceNwType)
{
    if(forceNwType <= eSIR_11N_NW_TYPE){
        pMac->hal.nwType = forceNwType;
        HALLOGP( halLog( pMac, LOGE,
            FL("Forced set nwType to %d\n"),
            forceNwType ));

    }
}
/**
    @brief    : This function sends a dummy Init Scan request to the softmac followed by DATA_NULL or CTS2SELF
    @param    : pMac-The Global Structure for MAC
              setPMbit-If this is set, DATA_NULL packet is sent out with init scan request

    @return    : eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE based on message sent correctly or not

*/

eHalStatus hal_SendDummyInitScan(tpAniSirGlobal pMac, tANI_BOOLEAN setPMbit)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    tInitScanParams initParam, *initScanParam;
    tANI_U32 waitForTxComp = 0;
    tBssSystemRole systemRole;

    systemRole = halGetBssSystemRoleFromStaIdx(pMac,
            HAL_STA_INVALID_IDX);
    // FIXME : What to do for multi bss case ?
    assert (systemRole != eSYSTEM_MULTI_BSS_ROLE)

    initScanParam = &initParam;

    if (pMac->hal.scanParam.linkState == eSIR_LINK_SCAN_STATE ||
                        pMac->hal.scanParam.linkState == eSIR_LINK_LEARN_STATE)
        return eHAL_STATUS_FAILURE;

    palZeroMemory(pMac->hHdd, (void *)&initParam, sizeof(initParam));
    palCopyMemory(pMac->hHdd, (void *)&initScanParam->bssid, (void *)pSta[0].bssId, 6);


    if (systemRole == eSYSTEM_STA_ROLE)
    {
        initScanParam->scanMode = eHAL_SYS_MODE_SCAN;
        initScanParam->notifyBss = TRUE;
        initScanParam->notifyHost = FALSE;
        initScanParam->frameType = SIR_MAC_DATA_NULL;
        initScanParam->scanDuration = 30;
        initScanParam->frameLength = 0;

        if (setPMbit == 1)
        {
            HALLOG1( halLog(pMac, LOG1, FL("Appending DATA_NULL \n")));
            CreateInitScanRawFrame(pMac, &initScanParam->macMgmtHdr, eSYSTEM_STA_ROLE);
            initScanParam->frameLength = sizeof(tSirMacMgmtHdr);
        }  else
            HALLOG1( halLog(pMac, LOG1, FL("No frames appended \n")));
    }
    else if (systemRole == eSYSTEM_AP_ROLE || systemRole == eSYSTEM_STA_IN_IBSS_ROLE)
    {
        initScanParam->scanMode = eHAL_SYS_MODE_LEARN;
        initScanParam->notifyBss = TRUE;
        initScanParam->notifyHost = FALSE;
        initScanParam->frameType = SIR_MAC_CTRL_FRAME;
        initScanParam->scanDuration = 30;
        initScanParam->frameLength = 0;

        HALLOG1( halLog(pMac, LOG1, FL("Appending CTS Frame \n")));
        initScanParam->frameType = SIR_MAC_CTRL_CTS;
        CreateInitScanRawFrame(pMac, &initScanParam->macMgmtHdr, eSYSTEM_AP_ROLE);
        initScanParam->frameLength = sizeof(tSirMacMgmtHdr);
    }

    pMac->hal.scanParam.linkState = eSIR_LINK_INIT_CAL_STATE;;

    return halMsg_HandleInitScan(pMac, initScanParam, &waitForTxComp);
}

/**
    @brief    : This function sends a dummy Finish Scan request to the softmac followed by DATA_NULL
    @param    : pMac-The Global Structure for MAC
    @return    : eHAL_STATUS_SUCCESS or eHAL_STATUS_FAILURE based on message sent correctly or not


*/

void hal_SendDummyFinishScanPostSetChan(tpAniSirGlobal pMac, void* pData, tANI_U32 status, tANI_U16 dialog_token)
{
    tANI_U32 waitForTxComp = 0;
    tpFinishScanParams pFinishScanParam = (tpFinishScanParams)pData;

    pMac->hal.scanParam.linkState = eSIR_LINK_FINISH_CAL_STATE;

    halMsg_HandleFinishScan(pMac, pFinishScanParam, &waitForTxComp);

    return;
}

eHalStatus hal_SendDummyFinishScan(tpAniSirGlobal pMac)
{
    tpStaStruct pSta = (tpStaStruct) pMac->hal.halMac.staTable;
    eHalStatus    status = eHAL_STATUS_SUCCESS;
    tpFinishScanParams pFinishScanParam = NULL;
    tANI_U8    saved_channel = pMac->hal.currentChannel;
    tBssSystemRole systemRole;

    systemRole = halGetBssSystemRoleFromStaIdx(pMac,
            HAL_STA_INVALID_IDX);


    // Code is currently not called by anyone if used
    // then will need fixes for multi-bss support
    assert (systemRole != eSYSTEM_MULTI_BSS_ROLE)

    palAllocateMemory(pMac->hHdd, (void*)pFinishScanParam, sizeof(tFinishScanParams));

    palZeroMemory(pMac->hHdd, (void *)pFinishScanParam, sizeof(tFinishScanParams));
    palCopyMemory(pMac->hHdd, (void *)&pFinishScanParam->bssid, (void *)pSta[0].bssId, 6);

    if (systemRole == eSYSTEM_STA_ROLE)
    {
        pFinishScanParam->scanMode = eHAL_SYS_MODE_NORMAL;
        pFinishScanParam->notifyBss = 1;
        pFinishScanParam->notifyHost = 0;
        CreateFinishScanRawFrame(pMac, &pFinishScanParam->macMgmtHdr, eSYSTEM_STA_ROLE);
        pFinishScanParam->frameLength = sizeof(tSirMacMgmtHdr);
    }
    else if (systemRole == eSYSTEM_AP_ROLE || systemRole == eSYSTEM_STA_IN_IBSS_ROLE)
    {
        pFinishScanParam->scanMode = eHAL_SYS_MODE_NORMAL;
        pFinishScanParam->notifyBss = 0;
        pFinishScanParam->notifyHost = 0;
        pFinishScanParam->frameType = SIR_MAC_CTRL_CTS;
        pFinishScanParam->frameLength = 0;
    }

    pFinishScanParam->currentOperChannel = pMac->hal.currentChannel;
    pFinishScanParam->cbState = pMac->hphy.phy.chanBondState;

    HALLOG2( halLog( pMac, LOG2, FL("Current Channel = %d\n"),  pMac->hal.currentChannel ));

    status = halPhy_ChangeChannel(pMac, saved_channel, pFinishScanParam->cbState, TRUE, hal_SendDummyFinishScanPostSetChan, pFinishScanParam, 0);
    // If channel is already on the request channel, proceed further with
    // post set channel configuration
    if (status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN) {
        hal_SendDummyFinishScanPostSetChan(pMac, (void*)pFinishScanParam, eHAL_STATUS_SUCCESS, 0);
    }

        return status;
}


/**
    @brief    : This function will read the configuration from Advanced properties and update the
              global variable for CAL period
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE based on message sent correctly or not

*/

tSirRetStatus halConfigCalPeriod(tpAniSirGlobal pMac)
{
    tANI_U32    val;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_CAL_PERIOD, &val) != eSIR_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), WNI_CFG_CAL_PERIOD));
        return eSIR_FAILURE;
    }
#ifdef FIXME_GEN5
    tx_timer_deactivate(&pMac->hal.halMac.tempMeasTimer);

    if (val == 0)
    {
        pMac->hal.halMac.tempMeasTmrVal = SYS_SEC_TO_TICKS(HAL_TEMPMEAS_TIMER_VAL_SEC);
        HALLOGW( halLog(pMac, LOGW, FL("Invalid CAL Period Set, Setting to default period(5 min)\n")));
    }
    else
        pMac->hal.halMac.tempMeasTmrVal = SYS_MIN_TO_TICKS(val)/HAL_PHY_PERIODIC_CAL_ITER_LIMIT;

    //every 30 seconds to see if the RF synth is still locked
    HALLOG1( halLog(pMac, LOG1, FL("WNI_CFG_CAL_PERIOD %d minutes\n"), val));
    tx_timer_change(&pMac->hal.halMac.tempMeasTimer,
                    pMac->hal.halMac.tempMeasTmrVal,
                    pMac->hal.halMac.tempMeasTmrVal);
    tx_timer_activate(&pMac->hal.halMac.tempMeasTimer);
#endif
    pMac->hphy.calPeriodTicks = 0;

    return eSIR_SUCCESS;
}

/**
    @brief    : This function will read the configuration from Advanced properties and update the
              global variable for CAL Control
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE

*/

tSirRetStatus halConfigCalControl(tpAniSirGlobal pMac)
{
    tANI_U32    val;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_CAL_CONTROL, &val) != eSIR_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Failed to cfg get id %d\n"), WNI_CFG_CAL_CONTROL));
        return eSIR_FAILURE;
    }
    else
    {
        if(val == WNI_CFG_CAL_CONTROL_CAL_ON)
            pMac->hphy.phy.phyPeriodicCalEnable= eANI_BOOLEAN_TRUE;
        else
            pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
        HALLOG1( halLog(pMac, LOG1, FL("WNI_CFG_CAL_CONTROL %d (0-on/1-off)\n"), val));
    }

#ifdef ANI_BUS_TYPE_USB
    pMac->hphy.phy.phyPeriodicCalEnable = eANI_BOOLEAN_FALSE;
#endif

    return eSIR_SUCCESS;
}

/**
    @brief    : This function will perform the periodic calibration when calibration timer triggers.
              It assesses whether the CAL is needed or not and performs the CAL based on
              the current RXP mode
    @param    : pMac-The Global Structure for MAC

    @return    : eSIR_SUCCESS or eSIR_FAILURE based on message sent correctly or not

*/

tSirRetStatus halPerformTempMeasurement(tpAniSirGlobal pMac)
{
/*
    tANI_BOOLEAN    performCal = eANI_BOOLEAN_TRUE;

    if (!pMac->hphy.phy.phyPeriodicCalEnable)
    {
        pMac->hphy.phy.phyPeriodicCalNeeded = eANI_BOOLEAN_FALSE;
        return eSIR_FAILURE;
    }

    if (halUtil_CurrentlyInPowerSave(pMac))
    {
        pMac->hphy.phy.phyPeriodicCalNeeded = eANI_BOOLEAN_FALSE;
        HALLOGW( halLog(pMac, LOGW, FL("Station currently in Powersave Mode, Skipping periodic Cal...\n")));
        return eSIR_FAILURE;
    }

    if (!pMac->hphy.phy.phyPeriodicCalNeeded)
    {
        if (halPhyAssessCal(pMac, &performCal) != eHAL_STATUS_SUCCESS)
        {
            return eSIR_FAILURE;
        }
    }

    //assess calibration every 30 seconds
    if ((performCal == eANI_BOOLEAN_TRUE) && (pMac->hphy.phy.phyPeriodicCalNeeded == eANI_BOOLEAN_TRUE))
    {
        tRxpMode rxpMode;

        // Find the global rxp Mode.
        rxpMode = halRxp_getSystemRxpMode(pMac);

        HALLOG2( halLog( pMac, LOG2, FL("Current RXP Mode = %d\n"),  rxpMode ));
        if ((rxpMode == eRXP_POST_ASSOC_MODE) || (rxpMode == eRXP_AP_MODE) || (rxpMode == eRXP_IBSS_MODE))
        {
            if (hal_SendDummyInitScan(pMac, 1) != eHAL_STATUS_SUCCESS)
            {
                HALLOGE( halLog(pMac, LOGE, FL("Send Init Scan failed\n")));
                return eSIR_FAILURE;
            }
        }
        else if ((rxpMode == eRXP_PROMISCUOUS_MODE) || (rxpMode == eRXP_IDLE_MODE))
        {
            halPhyCalUpdate(pMac);
        }
        else
        {
            HALLOGW( halLog(pMac, LOGW, FL("Skipping periodic Cal since RXP is in %d mode\n"), rxpMode));
            return eSIR_SUCCESS;
        }

        HALLOG4(halLog(pMac, LOG4, FL("Periodic Cal done")));
    }
*/
    return eSIR_SUCCESS;
}

tANI_BOOLEAN halIsRadioSwitchOn(tpAniSirGlobal pMac)
{

#if 0 //FIXME_NO_VIRGO

    tANI_U32    regValue;
    eHalStatus  status = eHAL_STATUS_SUCCESS;
    tANI_BOOLEAN switchOn = FALSE;

    do
    {
        status = halReadRegister(pMac, MCU_RF_ON_OFF_CONTROL_REG, (tANI_U32 *)&regValue);
        if (status != eHAL_STATUS_SUCCESS)
        {
            break;
        }

        if ( (MCU_RF_ON_OFF_CONTROL_RD_ON_OFF_HWPIN_STAT_MASK & regValue) )
        {
            switchOn = TRUE;
        }

    } while (0);

    return( switchOn );
#else
    return TRUE;
#endif
}

tANI_U8 halStateGet(tpAniSirGlobal pMac)
{
    return pMac->hal.halState;
}

void halStateSet(tpAniSirGlobal pMac, tANI_U8 state)
{
    pMac->hal.halState = state;
}


/** --------------------------------------------
\fn      halSetChainPowerState
\brief   This function gets the CFG and calls
\        halSetChainConfig().
\param   tpAniSirGlobal  pMac
\return  none
\ ---------------------------------------------- */
static void halSetChainPowerState(tpAniSirGlobal pMac)
{
    tANI_U32  chainState;

    if (wlan_cfgGetInt(pMac, WNI_CFG_POWER_STATE_PER_CHAIN, &chainState) != eSIR_SUCCESS )
        HALLOGP( halLog(pMac, LOGP, FL("Failed to get WNI_CFG_POWER_STATE_PER_CHAIN \n")));

    pMac->hal.cfgPowerStatePerChain = chainState;
    halSetChainConfig(pMac, chainState);
    return;
}

/** ------------------------------------------------------
\fn      halSetChainConfig
\brief   This function checks if the powerStatePerChain
\        setting is a valid entry.  If so, it will call
\        halPhy to set it appropraitely.
\
\param   tpAniSirGlobal  pMac
\param   tANI_U32        powerStatePerChain
\return  none
\ -------------------------------------------------------- */
void halSetChainConfig(tpAniSirGlobal pMac, tANI_U32 powerStatePerChain)
{
    tANI_U16         chain0, chain1, chain2;
    tANI_BOOLEAN     matchFound = eANI_BOOLEAN_FALSE;
    tANI_U8          i;
    tChainState      *pChainState = NULL;

    chain0 = (tANI_U16) GET_CHAIN_0(powerStatePerChain);
    chain1 = (tANI_U16) GET_CHAIN_1(powerStatePerChain);
    chain2 = (tANI_U16) GET_CHAIN_2(powerStatePerChain);

    HALLOGW( halLog(pMac, LOGW, FL("chain0=0x%x, chain1=0x%x, chain2=0x%x \n"), chain0, chain1, chain2));

    for (i = 0;  i < MAX_VALID_CHAIN_STATE; i++)
    {
        pChainState = &chainPwrStateTable[i];
        if (powerStatePerChain == pChainState->encoding)
        {
            HALLOGW( halLog(pMac, LOGW, FL("Match Found[%d], [0x%x,  %d] \n"), i, pChainState->encoding, pChainState->halPhyDef));
            matchFound = eANI_BOOLEAN_TRUE;
            break;
        }
    }

    if (matchFound)
    {
#ifdef CHAIN_SEL_CAPABLE //no calls to this function
        if ( halPhySetChainSelect(pMac, pChainState->halPhyDef) != eHAL_STATUS_SUCCESS)
        {
            HALLOGE( halLog(pMac, LOGE, FL("halPhySetChainSelect(0x%x) failed \n"), pChainState->halPhyDef));
        }
#endif
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("Invalid powerStatePerChain 0x%x \n"), powerStatePerChain));
    }

    return;
}






tSirRetStatus halReadWscMiscCfg(tHalHandle hMac,
                                tANI_U32 *wscApConfigMethod,
                                tANI_U8 *manufacturer,
                                tANI_U8 *modelName,
                                tANI_U8 *modelNumber,
                                tANI_U8 *serialNumber,
                                tANI_U8 *devicename)

{
    tpAniSirGlobal pMac = (tpAniSirGlobal)hMac;
    tANI_U32 len;

    if (wlan_cfgGetInt(pMac, (tANI_U16) WNI_CFG_WPS_CFG_METHOD, wscApConfigMethod) != eSIR_SUCCESS)
    {
        HALLOGP( halLog( pMac, LOGP, FL("halReadWscInfoApCfgMethod: Failed to cfg get id %d\n"),  WNI_CFG_WPS_CFG_METHOD ));
        return eSIR_FAILURE;
    }

    HALLOG1( halLog( pMac, LOG1, FL("halReadWscInfoApCfgMethod: [%x]\n"),  *wscApConfigMethod ));

    len = WNI_CFG_MANUFACTURER_NAME_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_NAME, manufacturer, &len) == eSIR_SUCCESS)
    {
        manufacturer[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve manufacturer name\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MODEL_NAME_LEN - 1;     /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MODEL_NAME, modelName, &len) == eSIR_SUCCESS)
    {
        modelName[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve model name\n")));
        return eSIR_FAILURE;
    }


    len = WNI_CFG_MODEL_NUMBER_LEN - 1;   /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MODEL_NUMBER, modelNumber, &len) == eSIR_SUCCESS)
    {
        modelNumber[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve model number\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MANUFACTURER_PRODUCT_VERSION_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_PRODUCT_VERSION, serialNumber, &len) == eSIR_SUCCESS)
    {
        serialNumber[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve serial number\n")));
        return eSIR_FAILURE;
    }

    len = WNI_CFG_MANUFACTURER_PRODUCT_NAME_LEN - 1; /* excluding the null terminator */
    if (wlan_cfgGetStr(pMac, WNI_CFG_MANUFACTURER_PRODUCT_NAME, devicename, &len) == eSIR_SUCCESS)
    {
        devicename[len] = '\0';
    }
    else
    {
        HALLOGP( halLog(pMac, LOGP, FL("could not retrieve device namer\n")));
        return eSIR_FAILURE;
    }

    return eSIR_SUCCESS;
}


tANI_U32
halTlPostMsgApi(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
#ifdef VOSS_ENABLED
    return  vos_mq_post_message(VOS_MQ_ID_TL, (vos_msg_t *) pMsg);
#endif
}

static 
eHalStatus halHandleDynamicPsPollValue(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
        return eHAL_STATUS_FAILURE;
    }
    else
    {    
        pMac->hal.dynamicPsPollValue = (tANI_BOOLEAN)val;
    }
    
    return status;
}

#ifdef WLAN_SOFTAP_FEATURE
/** -------------------------------------------------------------
\fn     halHandleEnableListenModeCfg
\brief  handles the CFG change for listen mode.
\param  tpAniSirGlobal pMac
\param  tANI_U32 cfgId
\return eHalStatus status
  -------------------------------------------------------------*/
static
eHalStatus halHandleEnableListenModeCfg(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
        return eHAL_STATUS_FAILURE;
    }
    else
    {    
        pMac->hal.ghalPhyAgcListenMode = (tANI_U8)val;   
    }
    
    return status;
}

/** ------------------------------------------------------------------------
\fn     halEnableListenMode
\brief  hal API to configure listen mode (disable or enable Listen mode 
\       with EDET threshold settings).
\param  tpAniSirGlobal pMac
\param  tANI_U8 listenModeEnableParams
\return eHalStatus status
  --------------------------------------------------------------------------*/
eHalStatus halEnableListenMode(tpAniSirGlobal pMac, tANI_U8 listenModeEnableParams)
{
    eHalStatus status;
    
    if (listenModeEnableParams <= QWLAN_RFAPB_BBF_SAT5_EGY_THRES_IN_MASK) 
    {
        status = halPhyAGCEnableListenMode(pMac, listenModeEnableParams); 
    }
    else
    {
        status = halPhyAGCDisableListenMode(pMac);
    }

    return status;
}
#endif
static 
eHalStatus halHandleMcastBcastFilterSetting(tpAniSirGlobal pMac, tANI_U32 cfgId)
{
    tANI_U32 val;
    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(eSIR_SUCCESS != wlan_cfgGetInt(pMac, (tANI_U16)cfgId, &val))
    {
        HALLOGP( halLog(pMac, LOGP, FL("Get cfg id (%d) failed \n"), cfgId));
        return eHAL_STATUS_FAILURE;
    }
    else
    {    
        pMac->hal.mcastBcastFilterSetting = (tANI_BOOLEAN)val;
    }
    
    return status;
}
