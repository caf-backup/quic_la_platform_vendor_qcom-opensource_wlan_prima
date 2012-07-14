
/*
 * Copyright (c) 2011 Qualcomm Atheros, Inc. 
 * All Rights Reserved. 
 * Qualcomm Atheros Confidential and Proprietary. 
 * 
 * Copyright (C) 2006 Airgo Networks, Incorporated
 * 
 * halUtils.cc: Provides all the utilities required by the driver APIs.
 * Author:  Susan Tsao
 * Date:    03/14/2007
 * History:-
 * Date     Modified by         Modification Information
 * --------------------------------------------------------------------------
 */
#include "palTypes.h"
#include "halUtils.h"
#include "halMacWmmApi.h"
#include "halStaTableApi.h"
#include "halDebug.h"
#include "halGlobal.h"       // halDeferMsgQ.size, halDeferMsgQ.write, HAL_MAX_DEFERRED_MSG_QUEUE_LEN
#include "cfgApi.h"          //wlan_cfgGetInt
#include "halFw.h"			//halUtil_DumpFwCorexLogs

#ifdef CONFIG_X86_32
//do_div() macro is required to do 64 bit division in 32 bit linux kernel. 
#include <asm/div64.h>
#endif

#define CASE_RETURN_STRING(x) case (x): return( #x );

//Static Functions
static  tSirMsgQ* halUtil_readDeferredMsgQ(tpAniSirGlobal pMac);

#if 0
static tANI_U32 aAc2BtqmQid[MAX_NUM_AC]={
   BTQM_QUEUE_TX_AC_BE, /* EDCA_AC_BE */
   BTQM_QUEUE_TX_AC_BK, /* EDCA_AC_BK */
   BTQM_QUEUE_TX_AC_VI, /* EDCA_AC_VI */
   BTQM_QUEUE_TX_AC_VO  /* EDCA_AC_VO */
};
#endif

/**
 * \brief This API returns the RF_BAND for a given channel number.
 * TODO: PE, HAL, and HALPHY should maintain a standard list
 * of channel definitions.
 *
 * \sa halUtil_GetRfBand
 *
 * \param pMac    The global tpAniSirGlobal object
 *
 * \param channel channel number
 *
 */
eRfBandMode halUtil_GetRfBand( tpAniSirGlobal pMac, tANI_U8 channel )
{
    if (channel == 0)
        return eRF_BAND_UNKNOWN;

    else if ( (channel >= 1) && (channel <= 14) )
    {
        return eRF_BAND_2_4_GHZ;
    }
    return eRF_BAND_5_GHZ;
}

/** ------------------------------------------------------
\fn      halUtil_MsgDecision
\brief   This function decides if a message needs to be
\        deferred, dropped or allowed. If we're in power
\        save, then defer all messages except if its a
\        message to exit out of power save mode.
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ        msg
\return   tHalMsgDecision eHAL_MSG_VALID, eHAL_MSG_DROP
\                         eHAL_MSG_DEFER
\ -------------------------------------------------------- */
tHalMsgDecision halUtil_MsgDecision(tpAniSirGlobal pMac, tSirMsgQ *pMsg, tANI_U8 *pMutexAcquired)
{
    if (halUtil_CurrentlyInPowerSave(pMac))
    {
        switch(pMsg->type)
        {
            case SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ:
            case SIR_HAL_TIMER_TEMP_MEAS_REQ:
            case SIR_HAL_TIMER_PERIODIC_STATS_COLLECT_REQ:
            case SIR_HAL_TIMER_WRAP_AROUND_STATS_COLLECT_REQ:
            case SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND:
                return eHAL_MSG_DROP;

            // Exclusion list of all the message for which acquiring mutex is
            // not required when in BMPS, as these messages would not be accessing
            // any register. Hence acquiring mutex for them would unnecessarily
            // wake up the chip.
            case SIR_HAL_TIMER_BA_ACTIVITY_REQ:
            case SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT:
                HALLOGW( halLog(pMac, LOGW, FL("##### In Power Save Message = %d (%s) #####"),
                        pMsg->type, halUtil_getMsgString(pMsg->type)));
                return eHAL_MSG_VALID;

            default:
                HALLOGW( halLog(pMac, LOGW, FL("##### In Power Save Message (Mutex) = %d (%s) #####"),
                        pMsg->type, halUtil_getMsgString(pMsg->type)));

                if (IS_PWRSAVE_STATE_IN_BMPS) {
                    // Acquire the mutex for all the other messages
                    halPS_SetHostBusy(pMac, HAL_PS_BUSY_GENERIC);
                    *pMutexAcquired = TRUE; 
                }
                return eHAL_MSG_VALID;
        }
    }

    return eHAL_MSG_VALID;
}



/** ------------------------------------------------------
\fn      halUtil_deferMsg
\brief   This function writes a message to the deferred
\        queue for later processing.
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ*       pMsg
\return  status
\ -------------------------------------------------------- */
eHalStatus halUtil_deferMsg(tpAniSirGlobal pMac, tSirMsgQ *pMsg)
{
    eHalStatus  status;
    tSirMsgQ    *pDeferMsg;
    tANI_U8     i;

    // check if deferred message queue is full
    if (pMac->hal.halDeferMsgQ.size >= HAL_MAX_DEFERRED_MSG_QUEUE_LEN)
    {
        pMac->hal.halDeferMsgQ.write = 0;
        pMac->hal.halDeferMsgQ.size = 0;
        pMac->hal.halDeferMsgQ.read = 0;

        // Free this msg for which we can't defer and all other msgs in the queue
        if( pMsg->bodyptr != NULL )
            palFreeMemory( pMac->hHdd, (void *) pMsg->bodyptr );

        for (i=0; i < HAL_MAX_DEFERRED_MSG_QUEUE_LEN; i++)
        {
            // Free all messages in the deferred queue
            pDeferMsg = &pMac->hal.halDeferMsgQ.deferredQueue[i];
            if (pDeferMsg->bodyptr != NULL)
                palFreeMemory( pMac->hHdd, (void *) pDeferMsg->bodyptr );
        }

        HALLOGP( halLog(pMac, LOGP, FL("HAL: Deferred Message Queue is full. resetting queue. (0x%x)\n"), pMsg->type));
        return eHAL_STATUS_FAILURE;
    }

    // write the message to the defer queue and advance the write pointer
    status = palCopyMemory(pMac->hHdd,
                           (tANI_U8 *)&pMac->hal.halDeferMsgQ.deferredQueue[pMac->hal.halDeferMsgQ.write],
                           (tANI_U8 *)pMsg,
                           sizeof(tSirMsgQ));

    if (status != eHAL_STATUS_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("palCopyMemory failed. Status %x \n"), status));
        return status;
    }

    HALLOGW( halLog(pMac, LOGW, FL("Write to HAL defer queue(size %d, write %d, msg %s (0x%x)) \n"),
           pMac->hal.halDeferMsgQ.size, pMac->hal.halDeferMsgQ.write, halUtil_getMsgString(pMsg->type), pMsg->type));

    ++pMac->hal.halDeferMsgQ.size;
    ++pMac->hal.halDeferMsgQ.write;

    // Reset the write pointer when it reaches the end of the queue.
    if (pMac->hal.halDeferMsgQ.write >= HAL_MAX_DEFERRED_MSG_QUEUE_LEN)
        pMac->hal.halDeferMsgQ.write = 0;

    return status;
}


/** ------------------------------------------------------
\fn      halUtil_readDeferredMsgQ
\brief   This function dequeues a deferred message for
\        processing.
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  msg
\return  status
\ -------------------------------------------------------- */
static tSirMsgQ* halUtil_readDeferredMsgQ(tpAniSirGlobal pMac)
{
    tSirMsgQ    *pMsg;

    // check if queue is empty
    if (pMac->hal.halDeferMsgQ.size <= 0)
        return NULL;

    // dequeue message from head of the queue
    pMsg = &pMac->hal.halDeferMsgQ.deferredQueue[pMac->hal.halDeferMsgQ.read];

    pMac->hal.halDeferMsgQ.size--;

    // advance the read pointer
    pMac->hal.halDeferMsgQ.read++;

    // Reset the read pointer when it reaches the end of queue
    if (pMac->hal.halDeferMsgQ.read >= HAL_MAX_DEFERRED_MSG_QUEUE_LEN)
        pMac->hal.halDeferMsgQ.read = 0;

    return (pMsg);
}


/** ------------------------------------------------------
\fn      halUtil_processDeferredMsgQ
\brief   This function processes the deferred messages in
\        the queue.
\param   tpAniSirGlobal  pMac
\param   tSirMsgQ  msg
\return  status
\ -------------------------------------------------------- */
void halUtil_processDeferredMsgQ(tpAniSirGlobal pMac)
{
    tSirMsgQ  msg = { 0, 0, 0 };
    tSirMsgQ  *readMsg;

    while (pMac->hal.halDeferMsgQ.size > 0)
    {
        HALLOG1( halLog(pMac, LOG1, FL("HAL defer queueSize = %d \n"), pMac->hal.halDeferMsgQ.size));
        readMsg = halUtil_readDeferredMsgQ(pMac);
        if (readMsg != NULL)
        {
            if (palCopyMemory( pMac->hHdd,
                               (tANI_U8*) &msg,
                               (tANI_U8*) readMsg,
                               sizeof(tSirMsgQ)) != eHAL_STATUS_SUCCESS )
                HALLOGP( halLog(pMac, LOGP, FL("palCopyMemory() failed \n")));

            if(halHandleMsg(pMac, &msg) != eSIR_SUCCESS )
                HALLOGE( halLog(pMac, LOGE, FL("halHandleMsg() returned error \n")));
        }
    }

    // Reset all counts for extra caution
    pMac->hal.halDeferMsgQ.size = 0;
    pMac->hal.halDeferMsgQ.write = 0;
    pMac->hal.halDeferMsgQ.read = 0;

    return;
}


/* ------------------------------------------
 * FUNCTION:  halUtil_CurrentlyInPowerSave()
 *
 * NOTE:
 *   Returns TRUE if device is currently
 *   in power save mode.
 * ------------------------------------------
 */
 tANI_BOOLEAN halUtil_CurrentlyInPowerSave( tpAniSirGlobal pMac )
{
    return (halPS_GetState(pMac)?1:0);
}


/** ------------------------------------------------------
\fn      halUtil_GetCardType
\brief   This function gets the card type from EEPROM.
\param   tpAniSirGlobal  pMac
\return  tAniCardType cardType
\ -------------------------------------------------------- */
tANI_U32 halUtil_GetCardType(tpAniSirGlobal pMac)
{
    return 0;
}

/** ------------------------------------------------------
\fn      halUtil_EndianConvert
\brief   This function converts the endian, and it expects
\        length to be multiples of 4.
\param   tpAniSirGlobal  pMac
\param   tANI_U32 pBuf
\param   tANI_U32 nLen
\return  status
\ -------------------------------------------------------- */
void halUtil_EndianConvert(tpAniSirGlobal pMac, tANI_U32 *pBuf, tANI_U32 nLen)
{
    tANI_U32 i;
    tANI_U8 *s;

    for (i=0; i<nLen/4; i++)
    {
       s = (tANI_U8 *)&pBuf[i];
       pBuf[i] = ((s[0]<<24) | (s[1]<<16) | (s[2]<<8) | s[3]);
    }
}


/** ------------------------------------------------------
\fn      halUtil_getMsgStr
\brief   This function converts the message id into string.
\param   tpAniSirGlobal  pMac
\param   tANI_U32        msgType
\return  char*
\ -------------------------------------------------------- */
#ifdef WLAN_DEBUG
char *halUtil_getMsgString(tANI_U16 msgId)
{
    switch (msgId)
    {
        CASE_RETURN_STRING( SIR_HAL_ADD_STA_REQ );;
        CASE_RETURN_STRING( SIR_HAL_ADD_STA_RSP );
        CASE_RETURN_STRING( SIR_HAL_DELETE_STA_REQ );
        CASE_RETURN_STRING( SIR_HAL_DELETE_STA_RSP );
        CASE_RETURN_STRING( SIR_HAL_ADD_BSS_REQ );
        CASE_RETURN_STRING( SIR_HAL_ADD_BSS_RSP );
        CASE_RETURN_STRING( SIR_HAL_DELETE_BSS_REQ );
        CASE_RETURN_STRING( SIR_HAL_DELETE_BSS_RSP );
        CASE_RETURN_STRING( SIR_HAL_INIT_SCAN_REQ );
        CASE_RETURN_STRING( SIR_HAL_INIT_SCAN_RSP );
        CASE_RETURN_STRING( SIR_HAL_START_SCAN_REQ );
        CASE_RETURN_STRING( SIR_HAL_START_SCAN_RSP );
        CASE_RETURN_STRING( SIR_HAL_END_SCAN_REQ );
        CASE_RETURN_STRING( SIR_HAL_END_SCAN_RSP );
        CASE_RETURN_STRING( SIR_HAL_FINISH_SCAN_REQ );
        CASE_RETURN_STRING( SIR_HAL_FINISH_SCAN_RSP );
        CASE_RETURN_STRING( SIR_HAL_SEND_BEACON_REQ );
        CASE_RETURN_STRING( SIR_HAL_SEND_BEACON_RSP );
        CASE_RETURN_STRING( SIR_HAL_INIT_CFG_REQ );
        CASE_RETURN_STRING( SIR_HAL_INIT_CFG_RSP );
        CASE_RETURN_STRING( SIR_HAL_INIT_WM_CFG_REQ );
        CASE_RETURN_STRING( SIR_HAL_INIT_WM_CFG_RSP );
        CASE_RETURN_STRING( SIR_HAL_SET_BSSKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_BSSKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_SET_STAKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_STAKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_DPU_STATS_REQ );
        CASE_RETURN_STRING( SIR_HAL_DPU_STATS_RSP );
        CASE_RETURN_STRING( SIR_HAL_GET_DPUINFO_REQ );
        CASE_RETURN_STRING( SIR_HAL_GET_DPUINFO_RSP );
        CASE_RETURN_STRING( SIR_HAL_UPDATE_EDCA_PROFILE_IND );
        CASE_RETURN_STRING( SIR_HAL_UPDATE_STARATEINFO_REQ );
        CASE_RETURN_STRING( SIR_HAL_UPDATE_STARATEINFO_RSP );
        CASE_RETURN_STRING( SIR_HAL_UPDATE_BEACON_IND );
        CASE_RETURN_STRING( SIR_HAL_UPDATE_CF_IND );
        CASE_RETURN_STRING( SIR_HAL_CHNL_SWITCH_REQ );
        CASE_RETURN_STRING( SIR_HAL_SWITCH_CHANNEL_RSP );
        CASE_RETURN_STRING( SIR_HAL_ADD_TS_REQ );
        CASE_RETURN_STRING( SIR_HAL_DEL_TS_REQ );
        CASE_RETURN_STRING( SIR_HAL_ENTER_BMPS_REQ );
        CASE_RETURN_STRING( SIR_HAL_ENTER_BMPS_RSP );
        CASE_RETURN_STRING( SIR_HAL_EXIT_BMPS_REQ );
        CASE_RETURN_STRING( SIR_HAL_EXIT_BMPS_RSP );
        CASE_RETURN_STRING( SIR_HAL_EXIT_BMPS_IND );
        CASE_RETURN_STRING( SIR_HAL_MISSED_BEACON_IND);
        CASE_RETURN_STRING( SIR_HAL_ENTER_UAPSD_REQ );
        CASE_RETURN_STRING( SIR_HAL_EXIT_UAPSD_REQ );
        CASE_RETURN_STRING( SIR_HAL_WOWL_ADD_BCAST_PTRN );
        CASE_RETURN_STRING( SIR_HAL_WOWL_DEL_BCAST_PTRN );
        CASE_RETURN_STRING( SIR_HAL_WOWL_ENTER_REQ );
        CASE_RETURN_STRING( SIR_HAL_WOWL_EXIT_REQ );
        CASE_RETURN_STRING( SIR_HAL_PWR_SAVE_CFG );
        CASE_RETURN_STRING( SIR_HAL_REGISTER_PE_CALLBACK );
        CASE_RETURN_STRING( SIR_HAL_ADDBA_REQ );
        CASE_RETURN_STRING( SIR_HAL_ADDBA_RSP );
        CASE_RETURN_STRING( SIR_HAL_DELBA_IND );
        CASE_RETURN_STRING( SIR_HAL_DELBA_REQ );
        CASE_RETURN_STRING( SIR_HAL_IBSS_STA_ADD );
        CASE_RETURN_STRING( SIR_HAL_TIMER_ADJUST_ADAPTIVE_THRESHOLD_IND );
        CASE_RETURN_STRING( SIR_HAL_SET_LINK_STATE );
        CASE_RETURN_STRING( SIR_HAL_ENTER_IMPS_REQ );
        CASE_RETURN_STRING( SIR_HAL_ENTER_IMPS_RSP );
        CASE_RETURN_STRING( SIR_HAL_POSTPONE_ENTER_IMPS_RSP);
        CASE_RETURN_STRING( SIR_HAL_EXIT_IMPS_REQ );
        CASE_RETURN_STRING( SIR_HAL_EXIT_IMPS_RSP );
        CASE_RETURN_STRING( SIR_HAL_SOFTMAC_HOSTMESG_PS_STATUS_IND );
        CASE_RETURN_STRING( SIR_HAL_STA_STAT_REQ );
        CASE_RETURN_STRING( SIR_HAL_GLOBAL_STAT_REQ );
        CASE_RETURN_STRING( SIR_HAL_AGGR_STAT_REQ );
        CASE_RETURN_STRING( SIR_HAL_STA_STAT_RSP );
        CASE_RETURN_STRING( SIR_HAL_GLOBAL_STAT_RSP );
        CASE_RETURN_STRING( SIR_HAL_AGGR_STAT_RSP );
        CASE_RETURN_STRING( SIR_HAL_STAT_SUMM_REQ );
        CASE_RETURN_STRING( SIR_HAL_STAT_SUMM_RSP );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_BSSKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_BSSKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_STAKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_STAKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_SET_STA_BCASTKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_STA_BCASTKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_STA_BCASTKEY_REQ );
        CASE_RETURN_STRING( SIR_HAL_REMOVE_STA_BCASTKEY_RSP );
        CASE_RETURN_STRING( SIR_HAL_DPU_MIC_ERROR );
        CASE_RETURN_STRING( SIR_HAL_TIMER_BA_ACTIVITY_REQ );
        CASE_RETURN_STRING( SIR_HAL_TIMER_CHIP_MONITOR_TIMEOUT );
        CASE_RETURN_STRING( SIR_HAL_TIMER_TRAFFIC_ACTIVITY_REQ );
        CASE_RETURN_STRING( SIR_HAL_TIMER_RA_COLLECT_AND_ADAPT );
        CASE_RETURN_STRING( SIR_HAL_GET_STATISTICS_REQ );
        CASE_RETURN_STRING( SIR_HAL_TIMER_WRAP_AROUND_STATS_COLLECT_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_MIMOPS_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_MIMOPS_RSP );
        CASE_RETURN_STRING( SIR_HAL_SYS_READY_IND );
        CASE_RETURN_STRING( SIR_HAL_SET_TX_POWER_REQ );
        CASE_RETURN_STRING( SIR_HAL_SET_TX_POWER_RSP );
        CASE_RETURN_STRING( SIR_HAL_GET_TX_POWER_REQ );
        CASE_RETURN_STRING( SIR_HAL_GET_TX_POWER_RSP );
        CASE_RETURN_STRING( SIR_HAL_GET_NOISE_REQ );
        CASE_RETURN_STRING( SIR_HAL_GET_NOISE_RSP );
        CASE_RETURN_STRING( SIR_HAL_BEACON_PRE_IND );
        CASE_RETURN_STRING( SIR_HAL_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_HAL_SEND_MSG_COMPLETE);
        CASE_RETURN_STRING( SIR_HAL_BTC_SET_CFG);
        CASE_RETURN_STRING( SIR_HAL_SIGNAL_BT_EVENT);
        CASE_RETURN_STRING( SIR_HAL_UPDATE_PROBE_RSP_TEMPLATE_IND);
        CASE_RETURN_STRING( SIR_HAL_TX_COMPLETE_IND);
        CASE_RETURN_STRING( SIR_CFG_PARAM_UPDATE_IND );
        CASE_RETURN_STRING( SIR_CFG_DOWNLOAD_COMPLETE_IND );
        CASE_RETURN_STRING( SIR_CFG_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_LIM_RESUME_ACTIVITY_NTF );
        CASE_RETURN_STRING( SIR_LIM_SUSPEND_ACTIVITY_REQ );
        CASE_RETURN_STRING( SIR_HAL_SUSPEND_ACTIVITY_RSP );
        CASE_RETURN_STRING( SIR_LIM_RETRY_INTERRUPT_MSG );
        CASE_RETURN_STRING( SIR_BB_XPORT_MGMT_MSG );
        CASE_RETURN_STRING( SIR_LIM_INV_KEY_INTERRUPT_MSG );
        CASE_RETURN_STRING( SIR_LIM_KEY_ID_INTERRUPT_MSG );
        CASE_RETURN_STRING( SIR_LIM_REPLAY_THRES_INTERRUPT_MSG );
        CASE_RETURN_STRING( SIR_LIM_TD_DUMMY_CALLBACK_MSG );
        CASE_RETURN_STRING( SIR_LIM_SCH_CLEAN_MSG );
        CASE_RETURN_STRING( SIR_LIM_RADAR_DETECT_IND );
        CASE_RETURN_STRING( SIR_LIM_DEL_TS_IND );
        CASE_RETURN_STRING( SIR_LIM_ADD_BA_IND );
        CASE_RETURN_STRING( SIR_LIM_DEL_BA_ALL_IND );
        CASE_RETURN_STRING( SIR_LIM_DELETE_STA_CONTEXT_IND );
        CASE_RETURN_STRING( SIR_LIM_DEL_BA_IND );
        CASE_RETURN_STRING( SIR_LIM_MIN_CHANNEL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_MAX_CHANNEL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_JOIN_FAIL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_AUTH_FAIL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_AUTH_RSP_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_ASSOC_FAIL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_REASSOC_FAIL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_HEART_BEAT_TIMEOUT );
        CASE_RETURN_STRING( SIR_HAL_GET_STATISTICS_RSP );
        CASE_RETURN_STRING( SIR_HAL_SET_KEY_DONE );
        CASE_RETURN_STRING( SIR_HAL_HANDLE_FW_MBOX_RSP);
#if (WNI_POLARIS_FW_PRODUCT == AP)
        CASE_RETURN_STRING( SIR_LIM_PREAUTH_CLNUP_TIMEOUT );
#endif
        CASE_RETURN_STRING( SIR_LIM_CHANNEL_SCAN_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_PROBE_HB_FAILURE_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_ADDTS_RSP_TIMEOUT );
#if (WNI_POLARIS_FW_PRODUCT == AP) && (WNI_POLARIS_FW_PACKAGE == ADVANCED)
        CASE_RETURN_STRING( SIR_LIM_MEASUREMENT_IND_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_LEARN_INTERVAL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_LEARN_DURATION_TIMEOUT );
#endif
        CASE_RETURN_STRING( SIR_LIM_LINK_TEST_DURATION_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_HASH_MISS_THRES_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_CNF_WAIT_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_KEEPALIVE_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_CHANNEL_SWITCH_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_QUIET_TIMEOUT );
        CASE_RETURN_STRING( SIR_LIM_QUIET_BSS_TIMEOUT );
#ifdef WMM_APSD
        CASE_RETURN_STRING( SIR_LIM_WMM_APSD_SP_START_MSG_TYPE );
        CASE_RETURN_STRING( SIR_LIM_WMM_APSD_SP_END_MSG_TYPE );
#endif
        CASE_RETURN_STRING( SIR_LIM_BEACON_GEN_IND );
        CASE_RETURN_STRING( SIR_LIM_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_SCH_CHANNEL_SWITCH_REQUEST );
        CASE_RETURN_STRING( SIR_SCH_START_SCAN_REQ );
        CASE_RETURN_STRING( SIR_SCH_START_SCAN_RSP );
        CASE_RETURN_STRING( SIR_SCH_END_SCAN_NTF );
        CASE_RETURN_STRING( SIR_SCH_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_PMM_CHANGE_PM_MODE );
        CASE_RETURN_STRING( SIR_PMM_CHANGE_IMPS_MODE );
        CASE_RETURN_STRING( SIR_MNT_RELEASE_BD );
        CASE_RETURN_STRING( SIR_MNT_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_DVT_ITC_MSG_TYPES_BEGIN );
        CASE_RETURN_STRING( SIR_DVT_MSG_TYPES_END );
        CASE_RETURN_STRING( SIR_PTT_MSG_TYPES_BEGIN );
        CASE_RETURN_STRING( SIR_PTT_MSG_TYPES_END );
#ifdef WLAN_FEATURE_P2P
        CASE_RETURN_STRING( SIR_HAL_P2P_UPDATE_SINGLE_NOA );
#endif
        CASE_RETURN_STRING( SIR_HAL_RESUME_BMPS );
        CASE_RETURN_STRING( SIR_HAL_SUSPEND_BMPS );
        CASE_RETURN_STRING( SIR_HAL_ENTER_UAPSD_RSP );
        CASE_RETURN_STRING( SIR_HAL_EXIT_UAPSD_RSP );
        default:
            return "Unknown messageId \n";
    }
}
#endif

void halUtil_getProtectionMode(tpAniSirGlobal pMac, tTpeProtPolicy *pProtPolicy)
{
    tANI_U32 cfgVal;

    if (wlan_cfgGetInt(pMac, WNI_CFG_FORCE_POLICY_PROTECTION, &cfgVal) != eSIR_SUCCESS) {
        HALLOGP( halLog(pMac, LOGP, FL("cfgGet WNI_CFG_FORCE_POLICY_PROTECTION failed \n")));
        return;
    }

    switch(cfgVal) {

        case WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE:
            *pProtPolicy = TPE_RATE_PROTECTION_NONE;
            break;

        case WNI_CFG_FORCE_POLICY_PROTECTION_AUTO:
            *pProtPolicy = TPE_RATE_PROTECTION_AUTO;
            break;

        case WNI_CFG_FORCE_POLICY_PROTECTION_RTS_ALWAYS:
            *pProtPolicy = TPE_RATE_PROTECTION_RTS;
            break;

        case WNI_CFG_FORCE_POLICY_PROTECTION_CTS:
            *pProtPolicy = TPE_RATE_PROTECTION_CTS;
            break;

        case WNI_CFG_FORCE_POLICY_PROTECTION_RTS:
            *pProtPolicy = TPE_RATE_PROTECTION_RTS;
            break;

        case WNI_CFG_FORCE_POLICY_PROTECTION_DUAL_CTS:
            *pProtPolicy = TPE_RATE_PROTECTION_DUAL_CTS;
            break;

        default:
            *pProtPolicy = TPE_RATE_PROTECTION_NONE;
            break;
    }
}


/** ------------------------------------------------------
\fn      halUtil_GetGCD
\brief   This function gets the GCD of the two given numbers
\param   num1, num2
\return  GCD
\ -------------------------------------------------------- */
tANI_U16 halUtil_GetGCD(tANI_U16 num1, tANI_U16 num2)
{
    tANI_U16 remainder = 0;
    tANI_U16 numerator = 0;
    tANI_U16 denominator = 0;

    numerator = num1;
    denominator = num2;

    while (1) {
        remainder = numerator%denominator;
        if(remainder == 0)
            break;
        numerator = denominator;
        denominator = remainder;
    }

    return denominator;
}


/* ------------------------------------------------------
\fn      halUtil_GetDtimTbtt
\brief   Get the TSF of the previous DTIM beacon based on the current
         TSF and the dtim count
\param   tpAniSirGlobal pMac, tANI_U64 tbtt,
         tANI_U16 dtimPeriod, tANI_U16 dtimCount, tANI_U64 *pDtimTbbt
\return  void
\ -------------------------------------------------------- */
#define HAL_DEFAULT_DTIM_TBTT   102400
void halUtil_GetDtimTbtt(tpAniSirGlobal pMac, tANI_U64 tbtt, tANI_U8 bssIdx,
        tANI_U8 dtimPeriod, tANI_U8 dtimCount, tANI_U64 *pDtimTbtt)
{
    tANI_U16 beaconInterval = 0;

    halTable_GetBeaconIntervalForBss(pMac, bssIdx, &beaconInterval);

    if(beaconInterval == 0)
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: %d: Beacon Interval is 0", __FUNCTION__, __LINE__);

    // Initialize to the given TBTT value
    *pDtimTbtt = tbtt;

    // If DTIM period is 1, each beacon TBTT is DTIM TBTT. Else calculate
    // the last received DTIM beacons TBTT
    if (dtimPeriod>1) {
        // If DTIM count is non-zero then we have a non DTIM tbtt so convert it
        // to the last probable DTIM TBTT
        if ((dtimCount>0) && (dtimCount<dtimPeriod)) {
            if(tbtt > ((beaconInterval * TIME_UNIT_IN_USEC) * (dtimPeriod - dtimCount)))
	        {
               *pDtimTbtt = tbtt - ((beaconInterval * TIME_UNIT_IN_USEC) * (dtimPeriod - dtimCount));
	        }
	        else
	        {
               *pDtimTbtt = HAL_DEFAULT_DTIM_TBTT * dtimPeriod;
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,"%s: %d: ERROR TBTT Low: 0x%x TBTT High: 0x%x\n",
                        __FUNCTION__, __LINE__,(&(tbtt))[0], (&(tbtt))[1]);
               VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,"%s: %d: beaconInterval: %d, dtimPeriod %d, dtimCount %d\n",
                        __FUNCTION__, __LINE__,beaconInterval, dtimPeriod, dtimCount);
	        }
        }
    }
}

void halUtil_GetLeastRefDtimTbtt(tpAniSirGlobal pMac, tANI_U8 bssIdx, tANI_U64 dtimTbtt, tANI_U64 *pRefDtimTbtt, tANI_U16 dtimPeriod)
{
    tANI_U16 beaconInterval = 0;

    halTable_GetBeaconIntervalForBss(pMac, bssIdx, &beaconInterval);

    if(beaconInterval == 0)
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s: %d: Beacon Interval is 0", __FUNCTION__, __LINE__);

    if((dtimPeriod > 0) && ( beaconInterval > 0)) 
    {	
        if(dtimTbtt == 0) 
        {
            *pRefDtimTbtt = (beaconInterval*dtimPeriod*TIME_UNIT_IN_USEC);
        }
    	else
    	{
#ifndef ANI_OS_TYPE_ANDROID
           *pRefDtimTbtt = dtimTbtt%(beaconInterval*dtimPeriod*TIME_UNIT_IN_USEC);
#else
           *pRefDtimTbtt = do_div(dtimTbtt, (beaconInterval*dtimPeriod*TIME_UNIT_IN_USEC)); 
#endif
    	}
    } else {
        *pRefDtimTbtt = dtimTbtt;
        HALLOGE(halLog(pMac, LOGE, "Error: Dtim period = %d !!!", dtimPeriod));
    }
}


/* ------------------------------------------------------
\fn      halUtil_GetRegPowerLimit
\brief   Function to get the regulatory power limit for the given channel
\param   tpAniSirGlobal pMac, tANI_U8 currChannel,
         tANI_U8 localPwrConstraint, tPowerDbm *pRegPwrLimit
\return  void
\ -------------------------------------------------------- */
void halUtil_GetRegPowerLimit(tpAniSirGlobal pMac, tANI_U8 currChannel,
        tANI_U8 localPwrConstraint, tANI_S8 *pRegPwrLimit)
{
    if(isChannelValid(currChannel)) {
        /* Get the regulatory transmit power from CFG */
        *pRegPwrLimit = cfgGetRegulatoryMaxTransmitPower(pMac, currChannel);

        /* We need to consider local power constraint for the BSS if imposed by the AP */
        *pRegPwrLimit = (*pRegPwrLimit - localPwrConstraint);
    } else {
        /* This is set intentionally to the highest possible value of INT8
         * If the current channel is invalid, dot11d power cannot be taken
         * then need to fall back to EEPROM power.
         */
        *pRegPwrLimit = HAL_MAX_TXPOWER_INVALID;
    }
}

// -------------------------------------------------------------------------------------
// FW corex log routine and header info

#define local_FEATURE_WLANFW_COREX_LOG_BUFFER_SIZE (0x1000)
#define local_COREX_LOG_NUM_FILTERS (8)

typedef struct {
   tANI_U8 nLogLevel;
   tANI_U8 nEventTypeMask;
} local_CorexLog_EventFilterType;

#define local_COREX_LOG_OVERHEAD \
  (sizeof(tANI_U32) \
   + sizeof(tANI_U32) \
   + sizeof(tANI_U32) \
   + (sizeof(local_CorexLog_EventFilterType) * local_COREX_LOG_NUM_FILTERS))

#define local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES \
   ((local_FEATURE_WLANFW_COREX_LOG_BUFFER_SIZE - local_COREX_LOG_OVERHEAD) / sizeof(tANI_U32))

typedef struct {
   volatile tANI_U32 nHaltLogging;
   volatile tANI_U32 nHeadIndex;
   volatile tANI_U32 nTailIndex;
   local_CorexLog_EventFilterType sEventFilter[local_COREX_LOG_NUM_FILTERS];
   tANI_U32 aBuffer[local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES];
} local_CorexLog_LogDescType;

typedef struct {
   tANI_U8  nCodeLo;
   tANI_U8  nCodeHi;
   tANI_U8  nModuleIndex;
   tANI_U8  nNumOfWords;

   tANI_U32 nTimestamp;
} local_CorexLog_EntryType;

// local storage
tANI_U8 local_fwCorexLog[sizeof(local_CorexLog_LogDescType)];

// FW corex log decoding routine
void halUtil_DumpFwCorexLogs(void *pData)
{
  tpAniSirGlobal pMac = (tpAniSirGlobal)pData;

  unsigned int i;
  unsigned int nHeadIndex, nTailIndex;
  unsigned int nStartIndex, nEndIndex;
  local_CorexLog_EntryType *pEntry;
  local_CorexLog_LogDescType *pCorexLog_DescP;
  unsigned int haltLogging;

  HALLOGP(halLog(pMac, LOGW, FL("[LOG] In Function halUtil_DumpFwCorexLogs\n")));
  haltLogging = 1;
  palWriteDeviceMemory(pMac->hHdd,
                       QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                       (tANI_U8 *)&haltLogging,
                       sizeof(unsigned int));

  palReadDeviceMemory(pMac->hHdd,
                      QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                      (tANI_U8 *)&local_fwCorexLog,
                      sizeof(local_CorexLog_LogDescType));

  haltLogging = 0;
  palWriteDeviceMemory(pMac->hHdd,
                       QWLANFW_MEM_FW_LOG_ADDR_OFFSET,
                       (tANI_U8 *)&haltLogging,
                       sizeof(unsigned int));

  pCorexLog_DescP = (local_CorexLog_LogDescType *)local_fwCorexLog;

  nHeadIndex = pCorexLog_DescP->nHeadIndex;
  nTailIndex = pCorexLog_DescP->nTailIndex;

  while (nHeadIndex != nTailIndex)
  {
     pEntry = (local_CorexLog_EntryType *) &(pCorexLog_DescP->aBuffer[nHeadIndex]);

     nStartIndex = nHeadIndex;
     nEndIndex = nStartIndex + pEntry->nNumOfWords + 1;
     if (nEndIndex >= local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES)
        nEndIndex -= local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES;

     if (++nHeadIndex == local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES)
        nHeadIndex = 0;

     HALLOGP(halLog(pMac, LOGE, FL("%08x : (%u-%u) %u:0x%02x%02x %u[ "),
        pCorexLog_DescP->aBuffer[nHeadIndex],
        nStartIndex,
        nEndIndex,
        pEntry->nModuleIndex,
        pEntry->nCodeHi,
        pEntry->nCodeLo,
        pEntry->nNumOfWords));

     if (++nHeadIndex == local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES)
        nHeadIndex = 0;

     for (i = 0; i < pEntry->nNumOfWords; ++i)
     {
        HALLOGE(halLog(pMac, LOGE, FL("%08x "), pCorexLog_DescP->aBuffer[nHeadIndex]));
        if (++nHeadIndex == local_FEATURE_WLANFW_COREX_LOG_BUFFER_ENTRIES)
           nHeadIndex = 0;
     }

     HALLOGE(halLog(pMac, LOGE, FL("]\r\n")));
  }

  return;
}
