/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limTimerUtils.cc contains the utility functions
 * LIM uses for handling various timers.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "pmmApi.h"


// default value 5000 ms for background scan period when it is disabled
#define LIM_BACKGROUND_SCAN_PERIOD_DEFAULT_MS    5000
// channel Switch Timer in ticks
#define LIM_CHANNEL_SWITCH_TIMER_TICKS           1
// Lim Quite timer in ticks
#define LIM_QUIET_TIMER_TICKS                    100
// Lim Quite BSS timer inteval in ticks
#define LIM_QUIET_BSS_TIMER_TICK                 100
// Lim KeepAlive timer default (3000)ms
#define LIM_KEEPALIVE_TIMER_MS                   3000

/**
 * limCreateTimers()
 *
 *FUNCTION:
 * This function is called upon receiving
 * 1. SME_START_REQ for STA in ESS role
 * 2. SME_START_BSS_REQ for AP role & STA in IBSS role
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac - Pointer to Global MAC structure
 *
 * @return None
 */

void
limCreateTimers(tpAniSirGlobal pMac)
{
    tANI_U32 cfgValue, i;

    PELOG1(limLog(pMac, LOG1, FL("Creating Timers used by LIM module in Role %d\n"), pMac->lim.gLimSystemRole);)

    if (wlan_cfgGetInt(pMac, WNI_CFG_ACTIVE_MINIMUM_CHANNEL_TIME,    
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get MinChannelTimeout value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP, FL("could not retrieve MinChannelTimeout value\n"));
    }
    cfgValue = SYS_MS_TO_TICKS(cfgValue);

    // Create MIN/MAX channel timers and activate them later
    if (tx_timer_create(&pMac->lim.limTimers.gLimMinChannelTimer,
                        "MIN CHANNEL TIMEOUT",
                        limTimerHandler, SIR_LIM_MIN_CHANNEL_TIMEOUT,
                        cfgValue, 0,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        /// Could not start min channel timer.
        // Log error
        limLog(pMac, LOGP, FL("could not create MIN channel timer\n"));

        return;
    }
#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.limTimers.gLimMinChannelTimer, LIM_TIMER_EXPIRY_LIST);
#endif

    PELOG2(limLog(pMac, LOG2, FL("Created MinChannelTimer\n"));)

    if (wlan_cfgGetInt(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME,     
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get MAXChannelTimeout value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP,
               FL("could not retrieve MAXChannelTimeout value\n"));
    }
    cfgValue = SYS_MS_TO_TICKS(cfgValue);

    if (tx_timer_create(&pMac->lim.limTimers.gLimMaxChannelTimer,
                        "MAX CHANNEL TIMEOUT",
                        limTimerHandler, SIR_LIM_MAX_CHANNEL_TIMEOUT,
                        cfgValue, 0,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
    {
        /// Could not start max channel timer.
        // Log error
        limLog(pMac, LOGP, FL("could not create MAX channel timer\n"));

        return;
    }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.limTimers.gLimMaxChannelTimer, LIM_TIMER_EXPIRY_LIST);
#endif

    PELOG2(limLog(pMac, LOG2, FL("Created MaxChannelTimer\n"));)

    if (pMac->lim.gLimSystemRole != eLIM_AP_ROLE)
    {
        // Create Channel Switch Timer
        if (tx_timer_create(&pMac->lim.limTimers.gLimChannelSwitchTimer,
                            "CHANNEL SWITCH TIMER",
                            limChannelSwitchTimerHandler,
                            0,                         // expiration_input
                            LIM_CHANNEL_SWITCH_TIMER_TICKS,  // initial_ticks
                            0,                         // reschedule_ticks
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            limLog(pMac, LOGP, FL("failed to create Channel Switch timer\n"));
            return;
        }

        //
        // Create Quiet Timer
        // This is used on the STA to go and shut-off
        // Tx/Rx "after" the specified quiteInterval
        //
        if (tx_timer_create(&pMac->lim.limTimers.gLimQuietTimer,
                            "QUIET TIMER",
                            limQuietTimerHandler,
                            SIR_LIM_QUIET_TIMEOUT,     // expiration_input
                            LIM_QUIET_TIMER_TICKS,     // initial_ticks
                            0,                         // reschedule_ticks
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            limLog(pMac, LOGP, FL("failed to create Quiet Begin Timer\n"));
            return;
        }

        //
        // Create Quiet BSS Timer
        // After the specified quiteInterval, determined by
        // gLimQuietTimer, this timer, gLimQuietBssTimer,
        // trigger and put the STA to sleep for the specified
        // gLimQuietDuration
        //
        if (tx_timer_create(&pMac->lim.limTimers.gLimQuietBssTimer,
                            "QUIET BSS TIMER",
                            limQuietBssTimerHandler,
                            SIR_LIM_QUIET_BSS_TIMEOUT, // expiration_input
                            LIM_QUIET_BSS_TIMER_TICK,  // initial_ticks
                            0,                         // reschedule_ticks
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            limLog(pMac, LOGP, FL("failed to create Quiet Begin Timer\n"));
            return;
        }

        if (wlan_cfgGetInt(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get JoinFailureTimeout value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve JoinFailureTimeout value\n"));
        }
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        // Create Join failure timer and activate it later
        if (tx_timer_create(&pMac->lim.limTimers.gLimJoinFailureTimer,
                        "JOIN FAILURE TIMEOUT",
                        limTimerHandler, SIR_LIM_JOIN_FAIL_TIMEOUT,
                        cfgValue, 0,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not create Join failure timer.
            // Log error
            limLog(pMac, LOGP, FL("could not create Join failure timer\n"));

            return;
        }
#if defined(ANI_OS_TYPE_RTAI_LINUX)
        tx_timer_set_expiry_list(&pMac->lim.limTimers.gLimJoinFailureTimer,
                                 LIM_TIMER_EXPIRY_LIST);
#endif

        if (wlan_cfgGetInt(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get AssocFailureTimeout value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve AssocFailureTimeout value\n"));
        }
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        // Create Association failure timer and activate it later
        if (tx_timer_create(&pMac->lim.limTimers.gLimAssocFailureTimer,
                        "ASSOC FAILURE TIMEOUT",
                        limAssocFailureTimerHandler, LIM_ASSOC,
                        cfgValue, 0,
                        TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not create Assoc failure timer.
            // Log error
            limLog(pMac, LOGP,
               FL("could not create Association failure timer\n"));

            return;
        }
        if (wlan_cfgGetInt(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get ReassocFailureTimeout value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve ReassocFailureTimeout value\n"));
        }
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        // Create Association failure timer and activate it later
        if (tx_timer_create(&pMac->lim.limTimers.gLimReassocFailureTimer,
                            "REASSOC FAILURE TIMEOUT",
                            limAssocFailureTimerHandler, LIM_REASSOC,
                            cfgValue, 0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not create Reassoc failure timer.
            // Log error
            limLog(pMac, LOGP,
               FL("could not create Reassociation failure timer\n"));

            return;
        }

        if (wlan_cfgGetInt(pMac, WNI_CFG_ADDTS_RSP_TIMEOUT, &cfgValue) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("Fail to get WNI_CFG_ADDTS_RSP_TIMEOUT \n"));

        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        // Create Addts response timer and activate it later
        if (tx_timer_create(&pMac->lim.limTimers.gLimAddtsRspTimer,
                            "ADDTS RSP TIMEOUT",
                            limAddtsResponseTimerHandler,
                            SIR_LIM_ADDTS_RSP_TIMEOUT,
                            cfgValue, 0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not create Auth failure timer.
            // Log error
            limLog(pMac, LOGP, FL("could not create Addts response timer\n"));

            return;
        }

        if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get AuthFailureTimeout value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve AuthFailureTimeout value\n"));
        }
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        // Create Auth failure timer and activate it later
        if (tx_timer_create(&pMac->lim.limTimers.gLimAuthFailureTimer,
                            "AUTH FAILURE TIMEOUT",
                            limTimerHandler,
                            SIR_LIM_AUTH_FAIL_TIMEOUT,
                            cfgValue, 0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not create Auth failure timer.
            // Log error
            limLog(pMac, LOGP, FL("could not create Auth failure timer\n"));

            return;
        }
#if defined(ANI_OS_TYPE_RTAI_LINUX)
        tx_timer_set_expiry_list(&pMac->lim.limTimers.gLimAuthFailureTimer,
                                 LIM_TIMER_EXPIRY_LIST);
#endif
        if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get BEACON_INTERVAL value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve BEACON_INTERVAL value\n"));
        }
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        if (tx_timer_create(&pMac->lim.limTimers.gLimHeartBeatTimer,
                            "Heartbeat TIMEOUT",
                            limTimerHandler,
                            SIR_LIM_HEART_BEAT_TIMEOUT,
                            cfgValue,
                            0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not start Heartbeat timer.
            // Log error
            limLog(pMac, LOGP,
               FL("call to create heartbeat timer failed\n"));
        }

        if (wlan_cfgGetInt(pMac, WNI_CFG_PROBE_AFTER_HB_FAIL_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get PROBE_AFTER_HB_FAILURE
             * value from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve PROBE_AFTER_HB_FAIL_TIMEOUT value\n"));
        }

        // Change timer to reactivate it in future
        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        if (tx_timer_create(&pMac->lim.limTimers.gLimProbeAfterHBTimer,
                            "Probe after Heartbeat TIMEOUT",
                            limTimerHandler,
                            SIR_LIM_PROBE_HB_FAILURE_TIMEOUT,
                            cfgValue,
                            0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            // Could not creat wt-probe-after-HeartBeat-failure timer.
            // Log error
            limLog(pMac, LOGP,
                   FL("unable to create ProbeAfterHBTimer\n"));
        }

#if defined(ANI_OS_TYPE_RTAI_LINUX)
        tx_timer_set_expiry_list(&pMac->lim.limTimers.gLimProbeAfterHBTimer,
                                 LIM_TIMER_EXPIRY_LIST);
#endif

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        if (wlan_cfgGetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get Background scan period value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve Background scan period value\n"));
        }

        /*
         * setting period to zero means disabling background scans when associated
         * the way we do this is to set a flag indicating this and keeping
         * the timer running, since it will be used for PDU leak workarounds
         * as well as background scanning during SME idle states
         */
        if (cfgValue == 0)
        {
            cfgValue = LIM_BACKGROUND_SCAN_PERIOD_DEFAULT_MS;
            pMac->lim.gLimBackgroundScanDisable = true;
        }
        else
            pMac->lim.gLimBackgroundScanDisable = false;

        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        if (tx_timer_create(&pMac->lim.limTimers.gLimBackgroundScanTimer,
                            "Background scan TIMEOUT",
                            limTimerHandler,
                            SIR_LIM_CHANNEL_SCAN_TIMEOUT,
                            cfgValue,
                            cfgValue,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            /// Could not start background scan timer.
            // Log error
            limLog(pMac, LOGP,
               FL("call to create background scan timer failed\n"));
        }
#endif
    }


    cfgValue = SYS_MS_TO_TICKS(LIM_HASH_MISS_TIMER_MS);

    if (tx_timer_create(
                        &pMac->lim.limTimers.gLimSendDisassocFrameThresholdTimer,
                        "Disassoc throttle TIMEOUT",
                        limSendDisassocFrameThresholdHandler,
                        SIR_LIM_HASH_MISS_THRES_TIMEOUT,
                        cfgValue,
                        cfgValue,
                        TX_AUTO_ACTIVATE) != TX_SUCCESS)
    {
        /// Could not start Send Disassociate Frame Threshold timer.
        // Log error
        limLog(pMac, LOGP,
               FL("create Disassociate throttle timer failed\n"));
    }
#if defined(ANI_OS_TYPE_RTAI_LINUX)
    tx_timer_set_expiry_list(
             &pMac->lim.limTimers.gLimSendDisassocFrameThresholdTimer,
             LIM_TIMER_EXPIRY_LIST);
#endif
    PELOG1(limLog(pMac, LOG1,
           FL("Created Disassociate throttle timer \n"));)

    /**
     * Create keepalive timer and  activate it right away for AP role
     */

    if (wlan_cfgGetInt(pMac, WNI_CFG_KEEPALIVE_TIMEOUT,
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get keepalive timeout value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP,
               FL("could not retrieve keepalive timeout value\n"));
    }

    // A value of zero implies keep alive should be disabled
    if (cfgValue == 0)
    {
        cfgValue = LIM_KEEPALIVE_TIMER_MS;
        pMac->sch.keepAlive = 0;
    } else
        pMac->sch.keepAlive = 1;


    cfgValue = SYS_MS_TO_TICKS(cfgValue + SYS_TICK_DUR_MS - 1);

    if (tx_timer_create(&pMac->lim.limTimers.gLimKeepaliveTimer,
                        "KEEPALIVE_TIMEOUT",
                        limKeepaliveTmerHandler,
                        0,
                        cfgValue,
                        cfgValue,
                        (pMac->lim.gLimSystemRole == eLIM_AP_ROLE) ?
                         TX_AUTO_ACTIVATE : TX_NO_ACTIVATE)
                  != TX_SUCCESS)
    {
        // Cannot create keepalive timer.  Log error.
        limLog(pMac, LOGP, FL("Cannot create keepalive timer.\n"));
    }

    /**
     * Create all CNF_WAIT Timers upfront
     */

    if (wlan_cfgGetInt(pMac, WNI_CFG_WT_CNF_TIMEOUT,
                  &cfgValue) != eSIR_SUCCESS)
    {
        /**
         * Could not get CNF_WAIT timeout value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP,
               FL("could not retrieve CNF timeout value\n"));
    }
    cfgValue = SYS_MS_TO_TICKS(cfgValue);

    for (i=0; i<pMac->lim.maxStation; i++)
    {
        if (tx_timer_create(&pMac->lim.limTimers.gpLimCnfWaitTimer[i],
                            "CNF_MISS_TIMEOUT",
                            limCnfWaitTmerHandler,
                            (tANI_U32)i,
                            cfgValue,
                            0,
                            TX_NO_ACTIVATE) != TX_SUCCESS)
        {
            // Cannot create timer.  Log error.
            limLog(pMac, LOGP, FL("Cannot create CNF wait timer.\n"));
        }
    }

    /*
    ** Alloc and init table for the preAuth timer list
    **
    **/

    // get max number of Preauthentication
    if (wlan_cfgGetInt(pMac, WNI_CFG_MAX_NUM_PRE_AUTH,
             &cfgValue) != eSIR_SUCCESS)
    {
        /*
        ** Could not get max preauth value
        ** from CFG. Log error.
        **/
        limLog(pMac, LOGP,
               FL("could not retrieve mac preauth value\n"));
    }
#ifdef ANI_AP_SDK_OPT
    if(cfgValue > SIR_SDK_OPT_MAX_NUM_PRE_AUTH)
        cfgValue = SIR_SDK_OPT_MAX_NUM_PRE_AUTH;
#endif // ANI_AP_SDK_OPT
    pMac->lim.gLimPreAuthTimerTable.numEntry = cfgValue;
    if (palAllocateMemory(pMac->hHdd, (void **) &pMac->lim.gLimPreAuthTimerTable.pTable, 
          cfgValue*sizeof(tLimPreAuthNode)) != eHAL_STATUS_SUCCESS)
    {
        limLog(pMac, LOGP, FL("palAllocateMemory failed!\n"));
        return;
    }

    limInitPreAuthTimerTable(pMac, &pMac->lim.gLimPreAuthTimerTable);
    PELOG1(limLog(pMac, LOG1, FL("alloc and init table for preAuth timers\n"));)

#ifdef ANI_PRODUCT_TYPE_AP

    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
    {
        /**
         * Create OLBC cache aging timer
         */
        if (wlan_cfgGetInt(pMac, WNI_CFG_OLBC_DETECT_TIMEOUT,
                      &cfgValue) != eSIR_SUCCESS)
        {
            /**
             * Could not get OLBC detect timeout value
             * from CFG. Log error.
             */
            limLog(pMac, LOGP,
               FL("could not retrieve OLBD detect timeout value\n"));
        }

        cfgValue = SYS_MS_TO_TICKS(cfgValue);

        if (tx_timer_create(
                &pMac->lim.limTimers.gLimUpdateOlbcCacheTimer,
                "OLBC UPDATE CACHE TIMEOUT",
                limUpdateOlbcCacheTimerHandler,
                0,
                cfgValue,
                cfgValue,
                TX_AUTO_ACTIVATE) != TX_SUCCESS)
        {
            // Cannot create update OLBC cache timer
            // Log error
            limLog(pMac, LOGP, FL("Cannot create update OLBC cache timer\n"));
        }
    }
#endif
    pMac->lim.gLimTimersCreated = 1;
} /****** end limCreateTimers() ******/



/**
 * limTimerHandler()
 *
 *FUNCTION:
 * This function is called upon
 * 1. MIN_CHANNEL, MAX_CHANNEL timer expiration during scanning
 * 2. JOIN_FAILURE timer expiration while joining a BSS
 * 3. AUTH_FAILURE timer expiration while authenticating with a peer
 * 4. Heartbeat timer expiration on STA
 * 5. Background scan timer expiration on STA
 * 6. AID release, Pre-auth cleanup and Link monitoring timer
 *    expiration on AP
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  param - Message corresponding to the timer that expired
 *
 * @return None
 */

void
limTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tANI_U32         statusCode;
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    // Prepare and post message to LIM Message Queue

    msg.type = (tANI_U16) param;
    msg.bodyptr = NULL;
    msg.bodyval = 0;

    if ((statusCode = limPostMsgApi(pMac, &msg)) != eSIR_SUCCESS)
        limLog(pMac, LOGE,
               FL("posting message %X to LIM failed, reason=%d\n"),
               msg.type, statusCode);
} /****** end limTimerHandler() ******/


/**
 * limAddtsResponseTimerHandler()
 *
 *FUNCTION:
 * This function is called upon Addts response timer expiration on sta
 *
 *LOGIC:
 * Message SIR_LIM_ADDTS_RSP_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  param - pointer to pre-auth node
 *
 * @return None
 */

void
limAddtsResponseTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    // Prepare and post message to LIM Message Queue

    msg.type = SIR_LIM_ADDTS_RSP_TIMEOUT;
    msg.bodyval = param;
    msg.bodyptr = NULL;

    limPostMsgApi(pMac, &msg);
} /****** end limAuthResponseTimerHandler() ******/


/**
 * limAuthResponseTimerHandler()
 *
 *FUNCTION:
 * This function is called upon Auth response timer expiration on AP
 *
 *LOGIC:
 * Message SIR_LIM_AUTH_RSP_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  param - pointer to pre-auth node
 *
 * @return None
 */

void
limAuthResponseTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    // Prepare and post message to LIM Message Queue

    msg.type = SIR_LIM_AUTH_RSP_TIMEOUT;
    msg.bodyptr = NULL;
    msg.bodyval = (tANI_U32)param;

    limPostMsgApi(pMac, &msg);
} /****** end limAuthResponseTimerHandler() ******/



/**
 * limAssocFailureTimerHandler()
 *
 *FUNCTION:
 * This function is called upon Re/Assoc failure timer expiration
 * on STA
 *
 *LOGIC:
 * Message SIR_LIM_ASSOC_FAIL_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  param - Indicates whether this is assoc or reassoc
 *                 failure timeout
 * @return None
 */

void
limAssocFailureTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    // Prepare and post message to LIM Message Queue

    msg.type = SIR_LIM_ASSOC_FAIL_TIMEOUT;
    msg.bodyval = (tANI_U32)param;
    msg.bodyptr = NULL;

    limPostMsgApi(pMac, &msg);
} /****** end limAssocFailureTimerHandler() ******/


/**
 * limUpdateOlbcCacheTimerHandler()
 *
 *FUNCTION:
 * This function is called upon update olbc cache timer expiration
 * on STA
 *
 *LOGIC:
 * Message SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT is posted to gSirLimMsgQ
 * when this function is executed.
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */
#ifdef ANI_PRODUCT_TYPE_AP

void
limUpdateOlbcCacheTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    // Prepare and post message to LIM Message Queue

    msg.type = SIR_LIM_UPDATE_OLBC_CACHEL_TIMEOUT;
    msg.bodyval = 0;
    msg.bodyptr = NULL;

    limPostMsgApi(pMac, &msg);
} /****** end limUpdateOlbcCacheTimerHandler() ******/

#endif

/**
 * limDeactivateAndChangeTimer()
 *
 *FUNCTION:
 * This function is called to deactivate and change a timer
 * for future re-activation
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in limUtils.h file
 *
 * @return None
 */

void
limDeactivateAndChangeTimer(tpAniSirGlobal pMac, tANI_U32 timerId)
{
    tANI_U32    val=0, val1=0;

    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_DEACTIVATE, 0, timerId));

    switch (timerId)
    {
        case eLIM_ADDTS_RSP_TIMER:
            pMac->lim.gLimAddtsRspTimerCount++;
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimAddtsRspTimer) != TX_SUCCESS)
            {
                // Could not deactivate AddtsRsp Timer
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to deactivate AddtsRsp timer\n"));
            }
            break;

        case eLIM_MIN_CHANNEL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimMinChannelTimer)
                                         != TX_SUCCESS)
            {
                // Could not deactivate min channel timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to deactivate min channel timer\n"));
            }

            // If a background was triggered via Quiet BSS,
            // then we need to adjust the MIN and MAX channel
            // timer's accordingly to the Quiet duration that
            // was specified
            if( eLIM_QUIET_RUNNING == pMac->lim.gLimSpecMgmt.quietState &&
                pMac->lim.gLimTriggerBackgroundScanDuringQuietBss )
            {
              // gLimQuietDuration is already cached in units of
              // system ticks. No conversion is reqd...
              val = pMac->lim.gLimSpecMgmt.quietDuration;
            }
            else
            {
              val = SYS_MS_TO_TICKS(pMac->lim.gpLimMlmScanReq->minChannelTime);
            }

            if (tx_timer_change(&pMac->lim.limTimers.gLimMinChannelTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change min channel timer.
                // Log error
                limLog(pMac, LOGP, FL("Unable to change min channel timer\n"));
            }

            break;

        case eLIM_MAX_CHANNEL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimMaxChannelTimer)
                                      != TX_SUCCESS)
            {
                // Could not deactivate max channel timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to deactivate max channel timer\n"));
            }

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
            // If a background was triggered via Quiet BSS,
            // then we need to adjust the MIN and MAX channel
            // timer's accordingly to the Quiet duration that
            // was specified
            if (pMac->lim.gLimSystemRole != eLIM_AP_ROLE)
            {
                
                if( eLIM_QUIET_RUNNING == pMac->lim.gLimSpecMgmt.quietState &&
                    pMac->lim.gLimTriggerBackgroundScanDuringQuietBss )
                {
                    // gLimQuietDuration is already cached in units of
                    // system ticks. No conversion is reqd...
                    val = pMac->lim.gLimSpecMgmt.quietDuration;
                }
                else
                {
                    val = SYS_MS_TO_TICKS(pMac->lim.gpLimMlmScanReq->maxChannelTime);
                }
            }
#endif
#if defined(ANI_PRODUCT_TYPE_AP)
            if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            {    
                if (wlan_cfgGetInt(pMac, WNI_CFG_ACTIVE_MAXIMUM_CHANNEL_TIME,    
                          &val) != eSIR_SUCCESS)
                {
                    /**
                    * Could not get max channel value
                    * from CFG. Log error.
                    */
                    limLog(pMac, LOGP,
                   FL("could not retrieve max channel value\n"));
                }
                val = SYS_MS_TO_TICKS(val);
            }
#endif

            if (tx_timer_change(&pMac->lim.limTimers.gLimMaxChannelTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change max channel timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to change max channel timer\n"));
            }

            break;

        case eLIM_JOIN_FAIL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimJoinFailureTimer)
                                         != TX_SUCCESS)
            {
                /**
                 * Could not deactivate Join Failure
                 * timer. Log error.
                 */
                limLog(pMac, LOGP,
                       FL("Unable to deactivate Join Failure timer\n"));
            }

            if (wlan_cfgGetInt(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get JoinFailureTimeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve JoinFailureTimeout value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimJoinFailureTimer,
                                val, 0) != TX_SUCCESS)
            {
                /**
                 * Could not change Join Failure
                 * timer. Log error.
                 */
                limLog(pMac, LOGP,
                       FL("Unable to change Join Failure timer\n"));
            }

            break;

        case eLIM_AUTH_FAIL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimAuthFailureTimer)
                                              != TX_SUCCESS)
            {
                // Could not deactivate Auth failure timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to deactivate auth failure timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get AuthFailureTimeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimAuthFailureTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change Authentication failure timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("unable to change Auth failure timer\n"));
            }

            break;

        case eLIM_ASSOC_FAIL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimAssocFailureTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Association failure timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate Association failure timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_ASSOCIATION_FAILURE_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get AssocFailureTimeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve AssocFailureTimeout value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimAssocFailureTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change Association failure timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("unable to change Assoc failure timer\n"));
            }

            break;

        case eLIM_REASSOC_FAIL_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimReassocFailureTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Reassociation failure timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate Reassoc failure timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get ReassocFailureTimeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve ReassocFailureTimeout value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimReassocFailureTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change Reassociation failure timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to change Reassociation failure timer\n"));
            }

            break;

        case eLIM_HEART_BEAT_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimHeartBeatTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Heartbeat timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("unable to deactivate Heartbeat timer\n"));
            }

            if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get BEACON_INTERVAL value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve BEACON_INTERVAL value\n"));
            }

            if (wlan_cfgGetInt(pMac, WNI_CFG_HEART_BEAT_THRESHOLD, &val1) !=
                                                          eSIR_SUCCESS)
                limLog(pMac, LOGP,
                   FL("could not retrieve heartbeat failure value\n"));

            // Change timer to reactivate it in future
            val = SYS_MS_TO_TICKS(val * val1);

            if (tx_timer_change(&pMac->lim.limTimers.gLimHeartBeatTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change HeartBeat timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("unable to change HeartBeat timer\n"));
            }

            break;

        case eLIM_PROBE_AFTER_HB_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimProbeAfterHBTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Heartbeat timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate probeAfterHBTimer\n"));
            }

            if (wlan_cfgGetInt(pMac, WNI_CFG_PROBE_AFTER_HB_FAIL_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get PROBE_AFTER_HB_FAILURE
                 * value from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve PROBE_AFTER_HB_FAIL_TIMEOUT value\n"));
            }

            // Change timer to reactivate it in future
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimProbeAfterHBTimer,
                                val, 0) != TX_SUCCESS)
            {
                // Could not change HeartBeat timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("unable to change ProbeAfterHBTimer\n"));
            }

            break;

        case eLIM_KEEPALIVE_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimKeepaliveTimer)
                            != TX_SUCCESS)
            {
                // Could not deactivate Keepalive timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate KeepaliveTimer timer\n"));
            }

            // Change timer to reactivate it in future

            if (wlan_cfgGetInt(pMac, WNI_CFG_KEEPALIVE_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get keepalive timeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve keepalive timeout value\n"));
            }
            if (val == 0)
            {
                val = 3000;
                pMac->sch.keepAlive = 0;
            } else
                pMac->sch.keepAlive = 1;



            val = SYS_MS_TO_TICKS(val + SYS_TICK_DUR_MS - 1);

            if (tx_timer_change(&pMac->lim.limTimers.gLimKeepaliveTimer,
                                val, val) != TX_SUCCESS)
            {
                // Could not change KeepaliveTimer timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to change KeepaliveTimer timer\n"));
            }

            break;
 
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        case eLIM_BACKGROUND_SCAN_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimBackgroundScanTimer)
                            != TX_SUCCESS)
            {
                // Could not deactivate BackgroundScanTimer timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate BackgroundScanTimer timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_BACKGROUND_SCAN_PERIOD,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get Background scan period value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve Background scan period value\n"));
            }
            if (val == 0)
            {
                val = LIM_BACKGROUND_SCAN_PERIOD_DEFAULT_MS;
                pMac->lim.gLimBackgroundScanDisable = true;
            }
            else
                pMac->lim.gLimBackgroundScanDisable = false;

            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimBackgroundScanTimer,
                                val, val) != TX_SUCCESS)
            {
                // Could not change BackgroundScanTimer timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to change BackgroundScanTimer timer\n"));
            }

            break;
#endif

#ifdef ANI_PRODUCT_TYPE_AP
        case eLIM_PRE_AUTH_CLEANUP_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimPreAuthClnupTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Pre-auth cleanup timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate Pre-auth cleanup timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_PREAUTH_CLNUP_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get pre-auth cleanup value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve pre-auth cleanup value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimPreAuthClnupTimer,
                                val, val) != TX_SUCCESS)
            {
                // Could not change pre-auth cleanup timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to change pre-auth cleanup timer\n"));
            }

            break;

        case eLIM_RELEASE_AID_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimAIDreleaseTimer) !=
                                    TX_SUCCESS)
            {
                // Could not deactivate Release AID timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to deactivate Release AID timer\n"));
            }

            // Change timer to reactivate it in future
            if (wlan_cfgGetInt(pMac, WNI_CFG_RELEASE_AID_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get Release AID timer value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve Release AID timer value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gLimAIDreleaseTimer,
                                val, val) != TX_SUCCESS)
            {
                // Could not change Release AID timer.
                // Log error
                limLog(pMac, LOGP,
                   FL("unable to change Release AID timer\n"));
            }

            break;

        case eLIM_LEARN_INTERVAL_TIMER:
            {
            // Restart Learn Interval timer
            tANI_U32 learnInterval =
              pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod /
              pMac->lim.gpLimMeasReq->channelList.numChannels;

              if (tx_timer_deactivate(
                     &pMac->lim.gLimMeasParams.learnIntervalTimer) != TX_SUCCESS)
              {
                  // Could not deactivate Learn Interval timer.
                  // Log error
                  limLog(pMac, LOGP,
                         FL("Unable to deactivate Learn Interval timer\n"));
              }

              if (tx_timer_change(
                         &pMac->lim.gLimMeasParams.learnIntervalTimer,
                         SYS_MS_TO_TICKS(learnInterval), 0) != TX_SUCCESS)
              {
                  // Could not change Learn Interval timer.
                  // Log error
                  limLog(pMac, LOGP, FL("Unable to change Learn Interval timer\n"));

                  return;
              }

              limLog( pMac, LOG3,
                  FL("Setting the Learn Interval TIMER to %d ticks\n"),
                  SYS_MS_TO_TICKS(learnInterval));
            }
            break;

#endif
        case eLIM_CHANNEL_SWITCH_TIMER:
            if (tx_timer_deactivate(&pMac->lim.limTimers.gLimChannelSwitchTimer) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("tx_timer_deactivate failed!\n"));
                return;
            }
            
            if (tx_timer_change(&pMac->lim.limTimers.gLimChannelSwitchTimer,
                        pMac->lim.gLimChannelSwitch.switchTimeoutValue,
                                    0) != TX_SUCCESS)
            {
                limLog(pMac, LOGP, FL("tx_timer_change failed \n"));
                return;
            }
            break;

        case eLIM_LEARN_DURATION_TIMER:
#ifdef ANI_PRODUCT_TYPE_AP
            if (tx_timer_deactivate(&pMac->lim.gLimMeasParams.learnDurationTimer) != TX_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Could not deactivate learn duration timer\n"));
                return;
            }

            if (pMac->lim.gpLimMeasReq->measControl.longChannelScanPeriodicity &&
                                 (pMac->lim.gLimMeasParams.shortDurationCount ==
                                  pMac->lim.gpLimMeasReq->measControl.longChannelScanPeriodicity))
            {
#ifdef ANI_AP_SDK
                val = pMac->lim.gLimScanDurationConvert.longChannelScanDuration_tick;
#else
                val = SYS_MS_TO_TICKS(pMac->lim.gpLimMeasReq->measDuration.longChannelScanDuration
                                                                    + SYS_TICK_DUR_MS - 1);
                if(val > 1)
                    val--;
#endif /* ANI_AP_SDK */
                // Time to perform measurements for longer term
                if (tx_timer_change(&pMac->lim.gLimMeasParams.learnDurationTimer,
                                                   val, 0) != TX_SUCCESS)
                {
                    // Could not change Learn duration timer.
                    // Log error
                    limLog(pMac, LOGP, FL("Unable to change Learn duration timer\n"));
                    return;
                }
                pMac->lim.gLimMeasParams.shortDurationCount = 0;
            }
            else
            {
#ifdef ANI_AP_SDK
                val = pMac->lim.gLimScanDurationConvert.shortChannelScanDuration_tick;
#else
                val = SYS_MS_TO_TICKS(pMac->lim.gpLimMeasReq->measDuration.shortChannelScanDuration
                                                               + SYS_TICK_DUR_MS - 1);
                if(val > 1)
                    val--;
#endif /* ANI_AP_SDK */
                if (tx_timer_change(&pMac->lim.gLimMeasParams.learnDurationTimer,
                                                       val, 0) != TX_SUCCESS)
                {
                    // Could not change Learn duration timer.
                    // Log error
                    limLog(pMac, LOGP, FL("Unable to change Learn duration timer\n"));
                }
            }
            pMac->lim.gpLimMeasData->duration = val * SYS_TICK_DUR_MS;
#endif
            break;

        case eLIM_QUIET_BSS_TIMER:
            if (TX_SUCCESS !=
            tx_timer_deactivate(&pMac->lim.limTimers.gLimQuietBssTimer))
            {
                limLog( pMac, LOGE,
                  FL("Unable to de-activate gLimQuietBssTimer! Will attempt to activate anyway...\n"));
            }
            
            // gLimQuietDuration appears to be in units of ticks
            // Use it as is
            if (TX_SUCCESS !=
                tx_timer_change( &pMac->lim.limTimers.gLimQuietBssTimer,
                  pMac->lim.gLimSpecMgmt.quietDuration,
                  0))
            {
                limLog( pMac, LOGE,
                  FL("Unable to change gLimQuietBssTimer! Will still attempt to activate anyway...\n"));
            }
            break;

        case eLIM_QUIET_TIMER:
            if( TX_SUCCESS != tx_timer_deactivate(&pMac->lim.limTimers.gLimQuietTimer))
            {
                limLog( pMac, LOGE,
                    FL( "Unable to deactivate gLimQuietTimer! Will still attempt to re-activate anyway...\n" ));
            }
            
            // Set the NEW timeout value, in ticks
            if( TX_SUCCESS != tx_timer_change( &pMac->lim.limTimers.gLimQuietTimer,
                              SYS_MS_TO_TICKS(pMac->lim.gLimSpecMgmt.quietTimeoutValue), 0))
            {
                limLog( pMac, LOGE,
                    FL( "Unable to change gLimQuietTimer! Will still attempt to re-activate anyway...\n" ));
            }
            break;
        default:
            // Invalid timerId. Log error
            break;
    }
} /****** end limDeactivateAndChangeTimer() ******/



/**
 * limReactivateTimer()
 *
 *FUNCTION:
 * This function is called to deactivate, change and
 * activate a timer
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in limUtils.h file
 *
 * @return None
 */

void
limReactivateTimer(tpAniSirGlobal pMac, tANI_U32 timerId)
{
    if (timerId == eLIM_HEART_BEAT_TIMER)
    {
       PELOG3(limLog(pMac, LOG3, FL("Rxed Heartbeat. Count=%d\n"),
               pMac->lim.gLimRxedBeaconCntDuringHB);)
        limDeactivateAndChangeTimer(pMac, eLIM_HEART_BEAT_TIMER);
	 MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_HEART_BEAT_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimHeartBeatTimer)
                                                    != TX_SUCCESS)
        {
            /// Could not activate Heartbeat timer.
            // Log error
            limLog(pMac, LOGP,
                   FL("could not activate Heartbeat timer\n"));
        }
        limResetHBPktCount(pMac);
    }
} /****** end limReactivateTimer() ******/



/**
 * limDeactivateAndChangePerStaIdTimer()
 *
 *
 * @brief: This function is called to deactivate and change a per STA timer
 * for future re-activation
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 * @note   staId for eLIM_AUTH_RSP_TIMER is auth Node Index.
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  timerId - enum of timer to be deactivated and changed
 *                   This enum is defined in limUtils.h file
 * @param  staId   - staId
 *
 * @return None
 */

void
limDeactivateAndChangePerStaIdTimer(tpAniSirGlobal pMac, tANI_U32 timerId, tANI_U16 staId)
{
    tANI_U32    val;
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_DEACTIVATE, 0, timerId));

    switch (timerId)
    {
        case eLIM_CNF_WAIT_TIMER:

            if (tx_timer_deactivate(&pMac->lim.limTimers.gpLimCnfWaitTimer[staId])
                            != TX_SUCCESS)
            {
                tpDphHashNode pStaDs;

                pStaDs = dphGetHashEntry(pMac, staId);
                if (pStaDs)
                {
                    PELOGE(limLog(pMac, LOGE, FL("StaId %d in state "), staId);)
                    limPrintMlmState(
                            pMac,
                            LOGE,
                            (tLimMlmStates) pStaDs->mlmStaContext.mlmState);
                    // Could not deactivate cnf wait timer.
                    // Log error
                    limLog(pMac, LOGP,
                       FL("unable to deactivate CNF wait timer\n"));
                }
                else
                {   // I don't think LOGP is necessary here??
                    limLog(pMac, LOGE, FL("Deact CnfWaitTimer for sta %d: No dphContext!!\n"),
                           staId);
                }
            }

            // Change timer to reactivate it in future

            if (wlan_cfgGetInt(pMac, WNI_CFG_WT_CNF_TIMEOUT,
                          &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get cnf timeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve cnf timeout value\n"));
            }
            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pMac->lim.limTimers.gpLimCnfWaitTimer[staId],
                                val, val) != TX_SUCCESS)
            {
                // Could not change cnf timer.
                // Log error
                limLog(pMac, LOGP, FL("unable to change cnf wait timer\n"));
            }

            break;

        case eLIM_AUTH_RSP_TIMER:
        {
            tLimPreAuthNode *pAuthNode;

            pAuthNode = limGetPreAuthNodeFromIndex(pMac, &pMac->lim.gLimPreAuthTimerTable, staId);

            if (pAuthNode == NULL)
            {
                limLog(pMac, LOGP, FL("Invalid Pre Auth Index passed :%d\n"), staId);
                break;
            }
            
            if (tx_timer_deactivate(&pAuthNode->timer) != TX_SUCCESS)
            {
                // Could not deactivate auth response timer.
                // Log error
                limLog(pMac, LOGP, FL("unable to deactivate auth response timer\n"));
            }

            // Change timer to reactivate it in future

            if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_RSP_TIMEOUT, &val) != eSIR_SUCCESS)
            {
                /**
                 * Could not get auth rsp timeout value
                 * from CFG. Log error.
                 */
                limLog(pMac, LOGP,
                   FL("could not retrieve auth response timeout value\n"));
            }

            val = SYS_MS_TO_TICKS(val);

            if (tx_timer_change(&pAuthNode->timer, val, 0) != TX_SUCCESS)
            {
                // Could not change auth rsp timer.
                // Log error
                limLog(pMac, LOGP, FL("unable to change auth rsp timer\n"));
            }
        }
            break;

#if (defined(ANI_PRODUCT_TYPE_AP) ||defined(ANI_PRODUCT_TYPE_AP_SDK))
        case eLIM_LEARN_INTERVAL_TIMER:
            {
            // Restart Learn Interval timer
            tANI_U32 learnInterval =
                    pMac->lim.gpLimMeasReq->measDuration.shortTermPeriod /
                    pMac->lim.gpLimMeasReq->channelList.numChannels;

            if (tx_timer_deactivate(
                   &pMac->lim.gLimMeasParams.learnIntervalTimer) != TX_SUCCESS)
            {
                // Could not deactivate Learn Interval timer.
                // Log error
                limLog(pMac, LOGP,
                       FL("Unable to deactivate Learn Interval timer\n"));
            }

            if (tx_timer_change(
                       &pMac->lim.gLimMeasParams.learnIntervalTimer,
                       SYS_MS_TO_TICKS(learnInterval), 0) != TX_SUCCESS)
            {
                // Could not change Learn Interval timer.
                // Log error
                limLog(pMac, LOGP, FL("Unable to change Learn Interval timer\n"));

                return;
            }

            limLog( pMac, LOG3,
                FL("Setting the Learn Interval TIMER to %d ticks\n"),
                SYS_MS_TO_TICKS(learnInterval) );
            }
            break;
#endif //#if (defined(ANI_PRODUCT_TYPE_AP) ||defined(ANI_PRODUCT_TYPE_AP_SDK))

        default:
            // Invalid timerId. Log error
            break;

    }
}


/**
 * limActivateCnfTimer()
 *
 *FUNCTION:
 * This function is called to activate a per STA timer
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  StaId   - staId
 *
 * @return None
 */

void limActivateCnfTimer(tpAniSirGlobal pMac, tANI_U16 staId)
{
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_CNF_WAIT_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gpLimCnfWaitTimer[staId])
                != TX_SUCCESS)
    {
        tpDphHashNode    pStaDs;

        pStaDs = dphGetHashEntry(pMac, staId);

        if (pStaDs)
        {
            PELOGE(limLog(pMac, LOGE, FL("StaId %d in state "), staId);)
            limPrintMlmState(pMac,
                             LOGE,
                             (tLimMlmStates) pStaDs->mlmStaContext.mlmState);

            /// Could not activate cnf wait timer.
            // Log error
            limLog(pMac, LOGP,
                   FL("could not activate cnf wait timer\n"));
        }
        else
        {   // I don't think LOGP is necessary here??
            limLog(pMac, LOGE, FL("ActivateCnfTimer for sta %d: No dphContext!!\n"),
                   staId);
        }

    }
}

/**
 * limActivateAuthRspTimer()
 *
 *FUNCTION:
 * This function is called to activate a per STA timer
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  id      - id
 *
 * @return None
 */

void limActivateAuthRspTimer(tpAniSirGlobal pMac, tLimPreAuthNode *pAuthNode)
{
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_AUTH_RESP_TIMER));
    if (tx_timer_activate(&pAuthNode->timer) != TX_SUCCESS)
    {
        /// Could not activate auth rsp timer.
        // Log error
        limLog(pMac, LOGP,
               FL("could not activate auth rsp timer\n"));
    }
}


/**
 * limSendDisassocFrameThresholdHandler()
 *
 *FUNCTION:
 *        This function reloads the credit to the send disassociate frame bucket
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */

void
limSendDisassocFrameThresholdHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tANI_U32         statusCode;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.type = SIR_LIM_HASH_MISS_THRES_TIMEOUT;
    msg.bodyval = 0;
    msg.bodyptr = NULL;

    if ((statusCode = limPostMsgApi(pMac, &msg)) != eSIR_SUCCESS)
            limLog(pMac, LOGE,
        FL("posting to LIM failed, reason=%d\n"), statusCode);

}

/**
 * limAssocCnfWaitTmerHandler()
 *
 *FUNCTION:
 *        This function post a message to send a disassociate frame out.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */

void
limCnfWaitTmerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tANI_U32         statusCode;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.type = SIR_LIM_CNF_WAIT_TIMEOUT;
    msg.bodyval = (tANI_U32)param;
    msg.bodyptr = NULL;

    if ((statusCode = limPostMsgApi(pMac, &msg)) != eSIR_SUCCESS)
            limLog(pMac, LOGE,
        FL("posting to LIM failed, reason=%d\n"), statusCode);

}

/**
 * limKeepaliveTmerHandler()
 *
 *FUNCTION:
 *        This function post a message to send a NULL data frame.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * NA
 *
 * @param
 *
 * @return None
 */

void
limKeepaliveTmerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tANI_U32         statusCode;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.type = SIR_LIM_KEEPALIVE_TIMEOUT;
    msg.bodyval = (tANI_U32)param;
    msg.bodyptr = NULL;

    if ((statusCode = limPostMsgApi(pMac, &msg)) != eSIR_SUCCESS)
        limLog(pMac, LOGE,
               FL("posting to LIM failed, reason=%d\n"), statusCode);

}

void
limChannelSwitchTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    PELOG1(limLog(pMac, LOG1,
        FL("ChannelSwitch Timer expired.  Posting msg to LIM \n"));)

    msg.type = SIR_LIM_CHANNEL_SWITCH_TIMEOUT;
    msg.bodyval = (tANI_U32)param;    
    msg.bodyptr = NULL;
    
    limPostMsgApi(pMac, &msg);
}

void
limQuietTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.type = SIR_LIM_QUIET_TIMEOUT;
    msg.bodyval = (tANI_U32)param;
    msg.bodyptr = NULL;

    PELOG1(limLog(pMac, LOG1,
        FL("Post SIR_LIM_QUIET_TIMEOUT msg. \n"));)
    limPostMsgApi(pMac, &msg);
}

void
limQuietBssTimerHandler(void *pMacGlobal, tANI_U32 param)
{
    tSirMsgQ    msg;
    tpAniSirGlobal pMac = (tpAniSirGlobal)pMacGlobal;

    msg.type = SIR_LIM_QUIET_BSS_TIMEOUT;
    msg.bodyval = (tANI_U32)param;
    msg.bodyptr = NULL;
    PELOG1(limLog(pMac, LOG1,
        FL("Post SIR_LIM_QUIET_BSS_TIMEOUT msg. \n"));)
    limPostMsgApi(pMac, &msg);
}

