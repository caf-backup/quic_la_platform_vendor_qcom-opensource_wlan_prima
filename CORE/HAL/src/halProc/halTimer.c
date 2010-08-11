    /*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file halTimer.c contains the utility function HAL
 * uses for timer related functions.
 *
 * Author:        Susan Tsao
 * Date:          01/04/07
 */

#include "palTypes.h"
#include "halTimer.h"
#include "halUtils.h"
#include "cfgApi.h"
#include "halLED.h"
#include "halDebug.h"
#include "halTLApi.h"

/** Keep track of Traffic Activity for the window of 100ms.
    As this information is used by:
     - halIsLinkBusy() used for BG Scan
     - Led Activity Timer
*/
#define  HAL_TRAFFIC_ACTIVITY_MONITOR_INTERVAL_MS           100

/** Monitor Chip for once in every ten Seconds.*/
#define  HAL_CHIP_MONITOR_INTERVAL_MS                       10000

/** Wait 500 ms for Firware response message. */
#define HAL_FW_RESPONSE_MESSAGE_TIMEOUT_MS             500

/** adc rssi stat collection every 500 ms. */
#define HAL_ADC_RSSI_STATS_INTERVAL_MS                      500

/**
 *  @brief : This is the generic handler for hal timers which indentifies
 * @param param is a pointer to the tAniSirGlobal structure
 * @return NONE
 */
static void halTimerHandler(void *pMacGlobal, tANI_U32 timerInfo)
{
    tSirMsgQ msg;

    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.bodyptr = NULL;
    msg.type = (tANI_U16) timerInfo;
    msg.bodyval = 0;

        halPostMsgApi(pMac, &msg);
    }

/** -------------------------------------------------------------
\fn halTimerTxCompleteTimeoutHandler
\brief handles the condition when TxComplete wait timeout expired.
\param     tpAniSirGlobal    pMacGlobal
\param     tANI_U32 timerInfo
\return    none.
  -------------------------------------------------------------*/

static void halTimerTxCompleteTimeoutHandler(void* pMacGlobal, tANI_U32 timerInfo)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;
    if (tx_timer_deactivate(&pMac->hal.txCompTimer) != TX_SUCCESS)
    {
        /** Could not deactivate
                    Log error*/
        HALLOGE(halLog(pMac, LOGE,
               FL("Unable to deactivate txCompTimer timer\n")));
        return;
    }

    if(pMac->hal.pCBackFnTxComp)
    {
        pMac->hal.pCBackFnTxComp(pMac, 0);
        pMac->hal.pCBackFnTxComp = NULL; //invalidating the pointer to be ready to handle further requests.
    }
    else
    {
        HALLOGE(halLog(pMac, LOGE, FL("There is no request pending for TxComplete and wait timer expired\n")));
    }
}

/** -------------------------------------------------------------
\fn halTimerAddBARspTimeoutHandler
\brief handles the condition when TL addBA rsp timeout happens.
\      makes a call to HAL for delBA, which will notify TL and also sends response to LIM.
\      when the API can't retrieve the pending addBAReq parameter
\      it will only send dummy response to LIM so that LIM can
\      start processing deferred messages.
\param     tpAniSirGlobal    pMacGlobal
\param     tANI_U32 timerInfo
\return    none.
  -------------------------------------------------------------*/
static void halTimerAddBARspTimeoutHandler(void *pMacGlobal, tANI_U32 timerInfo)
{
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;
    tpAddBAParams pAddBAParams = NULL;
    tSavedAddBAReqParamsStruct staAddBAParams;

    if (eHAL_STATUS_SUCCESS == halTable_GetStaAddBAParams(pMac, &staAddBAParams))
    {
        tSavedAddBAReqParamsStruct addBAReqParamsStruct;
        tpDelBAParams pDelBAParams;

        pAddBAParams = staAddBAParams.pAddBAReqParams;
        //reset the AddBAParams to default in the STA table
        addBAReqParamsStruct.pAddBAReqParams = NULL;
        addBAReqParamsStruct.addBAState = eHAL_ADDBA_NORMAL;
        halTable_SetStaAddBAReqParams(pMac, pAddBAParams->staIdx,
            pAddBAParams->baTID, addBAReqParamsStruct);

        //Let HAL delete BA and notify softmac and HDD to do so.
        if(eHAL_STATUS_SUCCESS == palAllocateMemory(pMac->hHdd, (void**)&pDelBAParams,
                               sizeof(tDelBAParams)))
        {
            pDelBAParams->baDirection = pAddBAParams->baDirection;
            pDelBAParams->baTID = pAddBAParams->baTID;
            pDelBAParams->staIdx = pAddBAParams->staIdx;
            halMsg_DelBA(pMac, 0, pDelBAParams);

            //send resp to LIM as failure.
            pAddBAParams->status = eHAL_STATUS_FAILURE;
            halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, pAddBAParams->baDialogToken, (void *) pAddBAParams, 0);
        }
        else
        {
            HALLOGP( halLog(pMac, LOGP, FL("palAllocateMemory() failed\n")));
            return;
        }
    }
    else
    {
        HALLOGE( halLog(pMac, LOGE, FL("Could not retrieve pAddBAParams for msgId = 0\n")));
        //sending a dummy message so that LIM can start processing deferring message.
        halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, 0, NULL, 0);
    }
    return;
}

/**
 *  @brief : Creates HAL timers
 * @param pMac MAC parameter structure pointer
 * @return tSirRetStatus SUCCESS or FAILURE
 */

tSirRetStatus halTimersCreate(tpAniSirGlobal pMac)
{
    tANI_U32 val;
    if(pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
    {
        if(wlan_cfgGetInt(pMac, WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT, &val) != eSIR_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("could not get BA activity cfg value\n")));
            return eSIR_FAILURE;
        }

        val = SYS_MS_TO_TICKS(val);

        if(tx_timer_create(&pMac->hal.halMac.baActivityChkTmr, "BA Activity check timer",
                    halTimerHandler, SIR_HAL_TIMER_BA_ACTIVITY_REQ,
                    val, val, TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Unable to create BA activity check timer\n")));
            return eSIR_FAILURE;
        }

        if (tx_timer_create(&pMac->hal.halMac.wrapStats.statTimer, "WRAP_AROUND_STAT COLLECTION TIMER",
                     halTimerHandler, SIR_HAL_TIMER_WRAP_AROUND_STATS_COLLECT_REQ,
                     pMac->hal.halMac.wrapStats.statTmrVal,
                     pMac->hal.halMac.wrapStats.statTmrVal, TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP(halLog(pMac, LOGP, FL("Could not create wrap around Stat timer\n")));
            return eSIR_FAILURE;
        }

    #ifdef FIXME_GEN5
        /** Create and Activate Temperature Measure Timer: Periodic calibration
            is based on this timer and temp measurements are done only in
            open tpc temp measure timer.*/
        if (tx_timer_create(&pMac->hal.halMac.tempMeasTimer, "TEMP MEASURE TIMER",
                     halTimerHandler, SIR_HAL_TIMER_TEMP_MEAS_REQ,
                                      pMac->hal.halMac.tempMeasTmrVal,
                     pMac->hal.halMac.tempMeasTmrVal, TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create Temp Measurement timer\n")));
            return eSIR_FAILURE;
        }

        /** Start Aging timer.*/
        if (tx_timer_create(&pMac->hal.halMac.macStats.statTimer, "STAT COLLECTION TIMER",
                     halTimerHandler, SIR_HAL_TIMER_PERIODIC_STATS_COLLECT_REQ,
                     pMac->hal.halMac.macStats.statTmrVal,
                     pMac->hal.halMac.macStats.statTmrVal, TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create Stat timer\n")));
            return eSIR_FAILURE;
        }
    #endif

        if(pMac->gDriverType == eDRIVER_TYPE_PRODUCTION)
        {
            val = SYS_MS_TO_TICKS(HAL_CHIP_MONITOR_INTERVAL_MS);
            /** Create hal chip monitor timer */
            if (tx_timer_create(&pMac->hal.halMac.chipMonitorTimer, "CHIP MONITOR INTERVAL TIMER",
                         halTimerHandler, SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT,
                         val,
                         val, TX_AUTO_ACTIVATE) != TX_SUCCESS)
            {
                HALLOGP( halLog(pMac, LOGP, FL("Could not create Chip monitor interval timer\n")));
                return eSIR_FAILURE;
            }
        }
    #ifdef FIXME_GEN5
        val = SYS_MS_TO_TICKS(HAL_TRAFFIC_ACTIVITY_MONITOR_INTERVAL_MS);
        /** Create Traffic Activity Monitor timer */
        if (tx_timer_create(&pMac->hal.trafficActivityTimer, "Traffic Activity Monitor timer",
                     halTimerHandler, SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ,
                     val,
                     val, TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create Traffic Activity Monitor timer\n")));
            return eSIR_FAILURE;
        }
    #endif
        /** Create timer for waiting for TL responce */
        //Right now this is being used for addBA only. Later, may be used in other cases too.
        if (tx_timer_create(&pMac->hal.addBARspTimer, "TL resp timeout timer",
                     halTimerAddBARspTimeoutHandler, 0,
                     SYS_TICK_DUR_MS,
                     0, TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create TL AddBA resp timeout timer\n")));
            return eSIR_FAILURE;
        }

        //Timer value in sys ticks unit.
        // 2 sec more than the HAL timeout for TL TX frame.
        val = (SYS_TICKS_PER_SECOND *2) + (HAL_TL_TX_FRAME_TIMEOUT / SYS_TICK_DUR_MS);

        if (tx_timer_create(&pMac->hal.txCompTimer, "TxComplete wait timer",
                     halTimerTxCompleteTimeoutHandler, 0,
                     val, 0, TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create TxComplete timeout timer\n")));
            return eSIR_FAILURE;
        }
    }

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        val = SYS_MS_TO_TICKS(HAL_ADC_RSSI_STATS_INTERVAL_MS);
        /** Create adc rssi stat collection timer */
        if (tx_timer_create(&pMac->ptt.adcRssiStatsTimer, "ADC RSSI STAT COLLECTION TIMER",
                     halTimerHandler, SIR_HAL_TIMER_ADC_RSSI_STATS,
                     val,
                     val, TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            HALLOGP( halLog(pMac, LOGP, FL("Could not create adc rssi stat collection timer\n")));
            return eSIR_FAILURE;
        }
    }

    return eSIR_SUCCESS;
}

/**
 *  @brief : Deletes and Destroys HAL timers
 *  @param pMac MAC parameter structure pointer
 *  @return tSirRetStatus SUCCESS or FAILURE
 */

tSirRetStatus halTimersDestroy(tpAniSirGlobal pMac)
{
    /** deactivate and delete BA activity check timer.*/
    tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr);
    tx_timer_delete(&pMac->hal.halMac.baActivityChkTmr);

#ifdef FIXME_GEN5
    /** deactiviate and delete temperature measurement timer.*/
    tx_timer_deactivate(&pMac->hal.halMac.tempMeasTimer);
    tx_timer_delete(&pMac->hal.halMac.tempMeasTimer);

#endif

    /** deactiviate and delete wrap around statistic collection timer.*/
    tx_timer_deactivate(&pMac->hal.halMac.wrapStats.statTimer);
    tx_timer_delete(&pMac->hal.halMac.wrapStats.statTimer);

    if(pMac->gDriverType == eDRIVER_TYPE_MFG)
    {
        /** deactiviate and delete adc rssi stat collection timer.*/
        tx_timer_deactivate(&pMac->ptt.adcRssiStatsTimer);
        tx_timer_delete(&pMac->ptt.adcRssiStatsTimer);
    }


#ifdef FIXME_GEN5
    /** deactiviate and delete statistic collection timer.*/
    tx_timer_deactivate(&pMac->hal.halMac.macStats.statTimer);
    tx_timer_delete(&pMac->hal.halMac.macStats.statTimer);
#endif

    /** deactiviate and delete Chip monitor timer.*/
    tx_timer_deactivate(&pMac->hal.halMac.chipMonitorTimer);
    tx_timer_delete(&pMac->hal.halMac.chipMonitorTimer);

#ifdef FIXME_GEN5

    /** deactiviate and delete traffic activity timer.*/
    tx_timer_deactivate(&pMac->hal.trafficActivityTimer);
    tx_timer_delete(&pMac->hal.trafficActivityTimer);
#endif
    /** deactivate and delete SMAC AddBA resp timeout timer **/
    tx_timer_deactivate(&pMac->hal.addBARspTimer);
    tx_timer_delete(&pMac->hal.addBARspTimer);

    tx_timer_deactivate(&pMac->hal.txCompTimer);
    tx_timer_delete(&pMac->hal.txCompTimer);

    return eSIR_SUCCESS;
}


/**
 *  @brief :This function is called to deactivate and change a timer
 * for future re-activation
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in halTimer.h file
 *
 * @return None
 */

void halDeactivateAndChangeTimer(tpAniSirGlobal pMac, tANI_U32 timerId)
{
    tANI_U32    val;

    switch (timerId)
    {
        case eHAL_BA_ACT_CHK_TIMER:
            if (tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS)
            {
                /** Could not deactivate
                    Log error*/
                HALLOGP( halLog(pMac, LOGP,
                       FL("Unable to deactivate baActicity check timer\n")));
                return;
            }

            /** Change timer to reactivate it in future */

            if (wlan_cfgGetInt(pMac, WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /** Could not read from CFG. Log error.*/
                HALLOGP( halLog(pMac, LOGP,
                   FL("could not retrieve BA activity check timeout period value from CFG\n")));
                return;
            }

            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->hal.halMac.baActivityChkTmr,
                                val, val) != TX_SUCCESS)
            {
                /** Could not change
                    Log error*/
                HALLOGP( halLog(pMac, LOGP,
                   FL("unable to change Ba Acticity check timer\n")));
                return;
            }

            break;

        default:
            /** need to add other timers if we are doing deactivate and change.
                Invalid timerId. Log error */
            HALLOGW( halLog(pMac, LOGW, FL("received invalid timer timerid = %d\n"),  timerId ));
            break;
    }
}
