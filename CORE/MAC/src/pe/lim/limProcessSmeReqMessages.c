/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 * This file limProcessSmeReqMessages.cc contains the code
 * for processing SME request messages.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "palTypes.h"
#include "wniApi.h"
#ifdef ANI_PRODUCT_TYPE_AP
#include "wniCfgAp.h"
#else
#include "wniCfgSta.h"
#endif
#include "cfgApi.h"
#include "sirApi.h"
#include "schApi.h"
#include "utilsApi.h"
#include "limTypes.h"
#include "limUtils.h"
#include "limAssocUtils.h"
#include "limSecurityUtils.h"
#include "limSerDesUtils.h"
#include "limSmeReqUtils.h"
#include "limIbssPeerMgmt.h"
#include "limAdmitControl.h"
#include "dphHashTable.h"
#include "limSendMessages.h"
#include "limApi.h"
#include "wmmApsd.h"

// SME REQ processing function templates
static void __limProcessSmeStartReq(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeSysReadyInd(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeStartBssReq(tpAniSirGlobal, tpSirMsgQ pMsg);
static void __limProcessSmeScanReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeJoinReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeReassocReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDisassocReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDisassocCnf(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeDeauthReq(tpAniSirGlobal, tANI_U32 *);
static void __limProcessSmeSetContextReq(tpAniSirGlobal, tANI_U32 *);
static tANI_BOOLEAN __limProcessSmeStopBssReq(tpAniSirGlobal, tpSirMsgQ pMsg);

#if 0
  static void __limProcessSmeAuthReq(tpAniSirGlobal, tANI_U32 *);
  static void __limProcessSmePromiscuousReq(tpAniSirGlobal, tANI_U32 *);
#endif

#ifdef ANI_PRODUCT_TYPE_AP
static void __limProcessSmeAssocCnf(tpAniSirGlobal, tANI_U32, tANI_U32 *);
#endif

#ifdef VOSS_ENABLED
extern void peRegisterTLHandle(tpAniSirGlobal pMac);
#endif

#ifdef ANI_PRODUCT_TYPE_CLIENT
#ifdef BACKGROUND_SCAN_ENABLED

// start the background scan timers if it hasn't already started
static void
__limBackgroundScanInitiate(tpAniSirGlobal pMac)
{
    if (pMac->lim.gLimBackgroundScanStarted)
        return;

    //make sure timer is created first
    if (TX_TIMER_VALID(pMac->lim.limTimers.gLimBackgroundScanTimer))
    {
        limDeactivateAndChangeTimer(pMac, eLIM_BACKGROUND_SCAN_TIMER);
	 MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_BACKGROUND_SCAN_TIMER));
        if (tx_timer_activate(&pMac->lim.limTimers.gLimBackgroundScanTimer) != TX_SUCCESS)
            limLog(pMac, LOGP, FL("could not activate background scan timer\n"));
        pMac->lim.gLimBackgroundScanStarted   = true;
        pMac->lim.gLimBackgroundScanChannelId = 0;
    }
}

#endif // BACKGROUND_SCAN_ENABLED
#endif

// determine if a fresh scan request must be issued or not
static tANI_U8
__limFreshScanReqd(
    tpAniSirGlobal pMac,
    tANI_U8             returnFreshResults)
{
    tANI_U8 retval = false;

    switch (pMac->lim.gLimSmeState)
    {
        case eLIM_SME_NORMAL_STATE:
            // check for flags only if we are anything other than ibss or ap
            if ( ! ( (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) ||
                     (pMac->lim.gLimSystemRole == eLIM_AP_ROLE) ) )
                break;
            // else fall through
        case eLIM_SME_IDLE_STATE:
        case eLIM_SME_JOIN_FAILURE_STATE:
        case eLIM_SME_LINK_EST_STATE:
            if (returnFreshResults & SIR_BG_SCAN_RETURN_FRESH_RESULTS)
                retval = true;
            break;

        default:
            break;
    }
#ifdef ANI_PRODUCT_TYPE_CLIENT
    PELOG1(limLog(pMac, LOG1, FL("FreshScanReqd: %d, bgScanInit %d, #Defd %d\n"),
           retval, pMac->lim.gLimBackgroundScanStarted, pMac->lim.gLimDeferredMsgQ.size);)
#endif
    return retval;
}

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
static tANI_BOOLEAN __limProcessSmeSwitchChlReq(tpAniSirGlobal, tpSirMsgQ pMsg);
#endif


/**
 * __limGetSmeJoinReqSizeForAlloc()
 *
 *FUNCTION:
 * This function is called in various places to get IE length
 * from tSirBssDescription structure
 * number being scanned.
 *
 *PARAMS:
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 * NA
 *
 *NOTE:
 * NA
 *
 * @param     pBssDescr
 * @return    Total IE length
 */

static tANI_U16
__limGetSmeJoinReqSizeForAlloc(tANI_U8 *pBuf)
{
    tANI_U16 len = 0;

    if (!pBuf)
        return len;

    pBuf += sizeof(tANI_U16);
    len = limGetU16( pBuf );
    return (len + sizeof( tANI_U16 ));
} /*** end __limGetSmeJoinReqSizeForAlloc() ***/


/**----------------------------------------------------------------
\fn     __limIsDeferedMsgForLearn

\brief  Has role only if 11h is enabled. Not used on STA side.
        Defers the message if SME is in learn state and brings
        the LIM back to normal mode.

\param  pMac
\param  pMsg - Pointer to message posted from SME to LIM.
\return TRUE - If defered
        FALSE - Otherwise
------------------------------------------------------------------*/
static tANI_BOOLEAN
__limIsDeferedMsgForLearn(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (limIsSystemInScanState(pMac))
    {
        if (limDeferMsg(pMac, pMsg) != TX_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("Could not defer Msg = %d\n"), pMsg->type);)
            return eANI_BOOLEAN_FALSE;
        }
        PELOG1(limLog(pMac, LOG1, FL("Defer the message, in learn mode type = %d\n"),
                                                                 pMsg->type);)

        /** Send finish scan req to HAL only if LIM is not waiting for any response
         * from HAL like init scan rsp, start scan rsp etc.
         */        
        if (GET_LIM_PROCESS_DEFD_MESGS(pMac))
            limSendHalFinishScanReq(pMac, eLIM_HAL_FINISH_LEARN_WAIT_STATE);

        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}

/**----------------------------------------------------------------
\fn     __limIsDeferedMsgForRadar

\brief  Has role only if 11h is enabled. Not used on STA side.
        Defers the message if radar is detected.

\param  pMac
\param  pMsg - Pointer to message posted from SME to LIM.
\return TRUE - If defered
        FALSE - Otherwise
------------------------------------------------------------------*/
static tANI_BOOLEAN
__limIsDeferedMsgForRadar(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    /** fRadarDetCurOperChan will be set only if we detect radar in current
     * operating channel and System Role == AP ROLE */
    if (LIM_IS_RADAR_DETECTED(pMac))
    {
        if (limDeferMsg(pMac, pMsg) != TX_SUCCESS)
        {
            PELOGE(limLog(pMac, LOGE, FL("Could not defer Msg = %d\n"), pMsg->type);)
            return eANI_BOOLEAN_FALSE;
        }
        PELOG1(limLog(pMac, LOG1, FL("Defer the message, in learn mode type = %d\n"),
                                                                 pMsg->type);)
        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}


/**
 * __limProcessSmeStartReq()
 *
 *FUNCTION:
 * This function is called to process SME_START_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeStartReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirResultCodes  retCode = eSIR_SME_SUCCESS;

    PELOG1(limLog(pMac, LOG1, FL("Received START_REQ\n"));)
    if (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE)
    {
        pMac->lim.gLimSystemRole = eLIM_STA_ROLE;
        pMac->lim.gLimSmeState   = eLIM_SME_IDLE_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
        /// By default do not return after first scan match
        pMac->lim.gLimReturnAfterFirstMatch = 0;

        /// Initialize MLM state machine
        limInitMlm(pMac);

        /// By default return unique scan results
        pMac->lim.gLimReturnUniqueResults = true;
        pMac->lim.gLimSmeScanResultLength = 0;

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
        if (((tSirSmeStartReq *) pMsgBuf)->sendNewBssInd)
        {
            /*
             * Need to indicate new BSSs found during background scanning to
             * host. Update this parameter at CFG
             */
            if (cfgSetInt(pMac, WNI_CFG_NEW_BSS_FOUND_IND, ((tSirSmeStartReq *) pMsgBuf)->sendNewBssInd)
                != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("could not set NEIGHBOR_BSS_IND at CFG\n"));
                retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            }
        }
#endif
    }
    else
    {
        /**
         * Should not have received eWNI_SME_START_REQ in states
         * other than OFFLINE. Return response to host and
         * log error
         */
        limLog(pMac, LOGE, FL("Invalid SME_START_REQ received in SME state %X\n"),
               pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
    }
    limSendSmeRsp(pMac, eWNI_SME_START_RSP, retCode);
} /*** end __limProcessSmeStartReq() ***/

/** -------------------------------------------------------------
\fn __limProcessSmeSysReadyInd
\brief handles the notification from HDD. PE just forwards this message to HAL.
\param   tpAniSirGlobal pMac
\param   tANI_U32* pMsgBuf
\return  TRUE-Posting to HAL failed, so PE will consume the buffer. 
\        FALSE-Posting to HAL successful, so HAL will consume the buffer.
  -------------------------------------------------------------*/
static tANI_BOOLEAN
__limProcessSmeSysReadyInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ msg;
    msg.type = SIR_HAL_SYS_READY_IND;
    msg.reserved = 0;
    msg.bodyptr =  pMsgBuf;
    msg.bodyval = 0;

#ifdef VOSS_ENABLED
#ifndef ANI_MANF_DIAG
	peRegisterTLHandle(pMac);
#endif
#endif
    PELOGW(limLog(pMac, LOGW, FL("sending SIR_HAL_SYS_READY_IND msg to HAL\n"));)
    MTRACE(macTraceMsgTx(pMac, 0, msg.type));

    if(eSIR_SUCCESS != halPostMsgApi(pMac, &msg))
    {
        limLog(pMac, LOGP, FL("halPostMsgApi failed\n"));
        return eANI_BOOLEAN_TRUE;
    }
    return eANI_BOOLEAN_FALSE;
}


/**
 * __limHandleSmeStartBssRequest()
 *
 *FUNCTION:
 * This function is called to process SME_START_BSS_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limHandleSmeStartBssRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16           size;
    tANI_U32           len;
    tANI_U32           val = 0;
    tSirRetStatus      retStatus;
    tSirMacChanNum     channelNumber;
    tLimMlmStartReq    *pMlmStartReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;
    tANI_U32 autoGenBssId = FALSE;//Flag Used in case of IBSS to Auto generate BSSID.
    tSirMacHTChannelWidth txWidthSet;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_START_BSS_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    PELOG1(limLog(pMac, LOG1, FL("Received START_BSS_REQ\n"));)

    if ( (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE) ||
         (pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) ||
         (pMac->lim.gLimSmeState == eLIM_SME_JOIN_FAILURE_STATE) )
    {
        size = sizeof(tSirSmeStartBssReq) + SIR_MAC_MAX_IE_LENGTH;
#ifdef ANI_PRODUCT_TYPE_AP
        size += ANI_WDS_INFO_MAX_LENGTH;
#endif
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMac->lim.gpLimStartBssReq, size))
        {
            PELOGE(limLog(pMac, LOGE, FL("palAllocateMemory failed for pMac->lim.gpLimStartBssReq\n"));)
            /// Send failure response to host
            retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
            goto end;
        }
        (void) palZeroMemory(pMac->hHdd, (void *) pMac->lim.gpLimStartBssReq, size);
        if ((limStartBssReqSerDes(pMac, pMac->lim.gpLimStartBssReq, (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
            (!limIsSmeStartBssReqValid(pMac, pMac->lim.gpLimStartBssReq)))
        {
            PELOGW(limLog(pMac, LOGW, FL("Received invalid eWNI_SME_START_BSS_REQ\n"));)
            retCode = eSIR_SME_INVALID_PARAMETERS;
            goto free;
        }

       PELOG3(limLog(pMac, LOG3,
           FL("Parsed START_BSS_REQ fields are bssType=%d, channelId=%d\n"),
           pMac->lim.gpLimStartBssReq->bssType, pMac->lim.gpLimStartBssReq->channelId);)

        if (pMac->lim.gpLimStartBssReq->channelId)
        {
            channelNumber = pMac->lim.gpLimStartBssReq->channelId;
            setupCBState( pMac, pMac->lim.gpLimStartBssReq->cbMode );
            pMac->lim.gHTSecondaryChannelOffset = limGetHTCBState(pMac->lim.gpLimStartBssReq->cbMode);
            txWidthSet = (tSirMacHTChannelWidth)limGetHTCapability(pMac, eHT_RECOMMENDED_TX_WIDTH_SET);

            /*
            * If there is a mismatch in secondaryChannelOffset being passed in the START_BSS request and
            * ChannelBonding CFG, then MAC will override the 'ChannelBonding' CFG with what is being passed
            * in StartBss Request.
            * HAL RA and PHY will go out of sync, if both these values are not consistent and will result in TXP Errors
            * when HAL RA tries to use 40Mhz rates when CB is turned off in PHY.
            */
            if(((pMac->lim.gHTSecondaryChannelOffset == eHT_SECONDARY_CHANNEL_OFFSET_NONE) &&
                (txWidthSet == eHT_CHANNEL_WIDTH_40MHZ)) ||
                ((pMac->lim.gHTSecondaryChannelOffset != eHT_SECONDARY_CHANNEL_OFFSET_NONE) &&
                (txWidthSet == eHT_CHANNEL_WIDTH_20MHZ)))
            {
                PELOGW(limLog(pMac, LOGW, FL("secondaryChannelOffset and txWidthSet don't match, resetting txWidthSet CFG\n"));)
                txWidthSet = (txWidthSet == eHT_CHANNEL_WIDTH_20MHZ) ? eHT_CHANNEL_WIDTH_40MHZ : eHT_CHANNEL_WIDTH_20MHZ;
                if (cfgSetInt(pMac, WNI_CFG_CHANNEL_BONDING_MODE, txWidthSet)
                                    != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP, FL("could not set  WNI_CFG_CHANNEL_BONDING_MODE at CFG\n"));
                    retCode = eSIR_LOGP_EXCEPTION;
                    goto free;
                }                
            }
        }
        else
        {
            PELOGW(limLog(pMac, LOGW, FL("Received invalid eWNI_SME_START_BSS_REQ\n"));)
            retCode = eSIR_SME_INVALID_PARAMETERS;
            goto free;
        }

        // Delete pre-auth list if any
        limDeletePreAuthList(pMac);

        // Delete IBSS peer BSSdescription list if any
        limIbssDelete(pMac);

        /// Initialize MLM state machine
#ifdef ANI_PRODUCT_TYPE_AP
            /* The Role is not set yet. Currently assuming the AddBss in Linux will be called by AP only.
             * This should be handled when IBSS functionality is implemented in the Linux 
             * TODO */
            pMac->lim.gLimMlmState = eLIM_MLM_IDLE_STATE;
            MTRACE(macTrace(pMac, TRACE_CODE_MLM_STATE, 0, pMac->lim.gLimMlmState));
#else
            limInitMlm(pMac);
#endif

        pMac->lim.htCapability = IS_DOT11_MODE_HT(pMac->lim.gLimDot11Mode);


        // Prepare and Issue LIM_MLM_START_REQ to MLM
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmStartReq, sizeof(tLimMlmStartReq)))
        {
            limLog(pMac, LOGP, FL("call to palAllocateMemory failed for mlmStartReq\n"));
            retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
            goto free;
        }

        (void) palZeroMemory(pMac->hHdd, (void *) pMlmStartReq, sizeof(tLimMlmStartReq));

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmStartReq->ssId,
                      (tANI_U8 *) &pMac->lim.gpLimStartBssReq->ssId,
                      pMac->lim.gpLimStartBssReq->ssId.length + 1);

        pMlmStartReq->bssType = pMac->lim.gpLimStartBssReq->bssType;

        if (pMlmStartReq->bssType == eSIR_INFRASTRUCTURE_MODE)
        {
            len = sizeof(tSirMacAddr);
            retStatus = wlan_cfgGetStr(pMac, WNI_CFG_STA_ID, (tANI_U8 *) pMlmStartReq->bssId, &len);
            if (retStatus != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrive BSSID, retStatus=%d\n"), retStatus);
        }
        else // ibss mode
        {
            pMac->lim.gLimIbssCoalescingHappened = false;
            
            if((retStatus = wlan_cfgGetInt(pMac, WNI_CFG_IBSS_AUTO_BSSID, &autoGenBssId)) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGP, FL("Could not retrieve Auto Gen BSSID, retStatus=%d\n"), retStatus);
                retCode = eSIR_LOGP_EXCEPTION;
                goto free;
            }
            if(!autoGenBssId)
            {
                if( (retStatus = wlan_cfgGetStr(pMac, WNI_CFG_BSSID, (tANI_U8 *) pMlmStartReq->bssId, &len)) != eSIR_SUCCESS)
                {
                    limLog(pMac, LOGP, FL("could not retrive BSSID, retStatus=%d\n"), retStatus);
                    retCode = eSIR_LOGP_EXCEPTION;
                    goto free;
                }

                if(pMlmStartReq->bssId[0] & 0x01)
                {
                   PELOGE(limLog(pMac, LOGE, FL("Request to start IBSS with group BSSID\n Autogenerating the BSSID\n"));)                    
                   autoGenBssId = TRUE;
                 }             
            }

            if( autoGenBssId )
            {   //if BSSID is not any uc id. then use locally generated BSSID.
                 //Autogenerate the BSSID
                limGetRandomBssid( pMac, pMlmStartReq->bssId);
                pMlmStartReq->bssId[0]= 0x02; 
            }
       }     

        pMlmStartReq->channelNumber = channelNumber;
        pMlmStartReq->cbMode = pMac->lim.gpLimStartBssReq->cbMode;

        if (wlan_cfgGetInt(pMac, WNI_CFG_BEACON_INTERVAL, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Beacon interval\n"));
        pMlmStartReq->beaconPeriod = (tANI_U16) val;

        if (wlan_cfgGetInt(pMac, WNI_CFG_DTIM_PERIOD, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve DTIM Period\n"));
        pMlmStartReq->dtimPeriod = (tANI_U8)val;

        if (wlan_cfgGetInt(pMac, WNI_CFG_CFP_PERIOD, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Beacon interval\n"));
        pMlmStartReq->cfParamSet.cfpPeriod = (tANI_U8)val;

        if (wlan_cfgGetInt(pMac, WNI_CFG_CFP_MAX_DURATION, &val) != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve CFPMaxDuration\n"));
        pMlmStartReq->cfParamSet.cfpMaxDuration = (tANI_U16) val;

        len = WNI_CFG_OPERATIONAL_RATE_SET_LEN;
        retStatus = wlan_cfgGetStr(pMac, WNI_CFG_OPERATIONAL_RATE_SET, pMlmStartReq->rateSet.rate, &len);
        if (retStatus != eSIR_SUCCESS)
            limLog(pMac, LOGP, FL("could not retrieve Oper rateset, Status=%d\n"), retStatus);
        pMlmStartReq->rateSet.numRates = (tANI_U8) len;

        // Now populate the 11n related parameters
        pMlmStartReq->nwType    = pMac->lim.gpLimStartBssReq->nwType;
        pMlmStartReq->htCapable = pMac->lim.htCapability;
        //
        // FIXME_GEN4 - Determine the appropriate defaults...
        //
        pMlmStartReq->htOperMode        = pMac->lim.gHTOperMode;
        pMlmStartReq->dualCTSProtection = pMac->lim.gHTDualCTSProtection; // Unused
        pMlmStartReq->txChannelWidthSet = pMac->lim.gHTRecommendedTxWidthSet;

        //Update the global LIM parameter, which is used to populate HT Info IEs in beacons/probe responses.
        pMac->lim.gHTSecondaryChannelOffset = limGetHTCBState(pMlmStartReq->cbMode);

        pMac->lim.gLimRFBand = limGetRFBand(channelNumber);

        // Initialize 11h Enable Flag
        pMac->lim.gLim11hEnable = 0;
        if((pMlmStartReq->bssType != eSIR_IBSS_MODE) &&
           (SIR_BAND_5_GHZ == pMac->lim.gLimRFBand) )
        {
            if (wlan_cfgGetInt(pMac, WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
            pMac->lim.gLim11hEnable = val;
        }
        
        if (!pMac->lim.gLim11hEnable)
        {
            if (cfgSetInt(pMac, WNI_CFG_LOCAL_POWER_CONSTRAINT, 0) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
        }

#ifdef ANI_PRODUCT_TYPE_AP
        PELOGE(limLog(pMac, LOGE, FL("Dot 11h is %s\n"), pMac->lim.gLim11hEnable?"Enabled":"Disabled");)
        if (pMac->lim.gLim11hEnable)
        { 
           PELOG2(limLog(pMac, LOG2, FL("Cb state = %d, SecChanOffset = %d\n"),
                   pMac->lim.gCbState, pMac->lim.gHTSecondaryChannelOffset);)
            limRadarInit(pMac);
        }
#endif

        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        pMac->lim.gLimSmeState     = eLIM_SME_WT_START_BSS_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        limPostMlmMessage(pMac, LIM_MLM_START_REQ, (tANI_U32 *) pMlmStartReq);
        return;
    }
    else
    {
        /** Should not have received eWNI_SME_START_BSS_REQ */
        limLog(pMac, LOGE, FL("Received unexpected START_BSS_REQ, in state %X\n"),
               pMac->lim.gLimSmeState);
        retCode = eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
        goto end;
    } // if (pMac->lim.gLimSmeState == eLIM_SME_OFFLINE_STATE)

free:
    palFreeMemory( pMac->hHdd, pMac->lim.gpLimStartBssReq);
    pMac->lim.gpLimStartBssReq = NULL;

end:
    limSendSmeStartBssRsp(pMac, eWNI_SME_START_BSS_RSP, retCode);
} /*** end __limHandleSmeStartBssRequest() ***/

/**--------------------------------------------------------------
\fn     __limProcessSmeStartBssReq

\brief  Wrapper for the function __limHandleSmeStartBssRequest
        This message will be defered until softmac come out of
        scan mode or if we have detected radar on the current
        operating channel.
\param  pMac
\param  pMsg

\return TRUE - If we consumed the buffer
        FALSE - If have defered the message.
 ---------------------------------------------------------------*/
static tANI_BOOLEAN
__limProcessSmeStartBssReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (__limIsDeferedMsgForLearn(pMac, pMsg) ||
            __limIsDeferedMsgForRadar(pMac, pMsg))
    {
        /**
         * If message defered, buffer is not consumed yet.
         * So return false
         */
        return eANI_BOOLEAN_FALSE;
    }

    __limHandleSmeStartBssRequest(pMac, (tANI_U32 *) pMsg->bodyptr);
    return eANI_BOOLEAN_TRUE;
}

/**
 *  limGetRandomBssid()
 *
 *  FUNCTION:This function is called to process generate the random number for bssid
 *  This function is called to process SME_SCAN_REQ message
 *  from HDD or upper layer application.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 * 1. geneartes the unique random number for bssid in ibss
 *
 *  @param  pMac      Pointer to Global MAC structure
 *  @param  *data      Pointer to  bssid  buffer
 *  @return None
 */
void limGetRandomBssid(tpAniSirGlobal pMac, tANI_U8 *data)
{
     tANI_U32 random[2] ;
     random[0] = tx_time_get();
     random[0] |= (random[0] << 15) ;
     random[1] = random[0] >> 1;
     palCopyMemory(pMac->hHdd, data, (tANI_U8*)random, sizeof(tSirMacAddr));
}

/**
 * __limProcessSmeScanReq()
 *
 *FUNCTION:
 * This function is called to process SME_SCAN_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * 1. Periodic scanning should be requesting to return unique
 *    scan results.
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeScanReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U32           len;
    tLimMlmScanReq     *pMlmScanReq;
    tpSirSmeScanReq    pScanReq;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_SCAN_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    pScanReq = (tpSirSmeScanReq) pMsgBuf;	
    PELOG1(limLog(pMac, LOG1, FL("SME SCAN REQ numChan %d min %d max %d first %d fresh %d unique %d type %d rsp %d\n"),
           pScanReq->channelList.numChannels,
           pScanReq->minChannelTime,
           pScanReq->maxChannelTime,
           pScanReq->returnAfterFirstMatch,
           pScanReq->returnFreshResults,
           pScanReq->returnUniqueResults,
           pScanReq->scanType, pMac->lim.gLimRspReqd ? 1 : 0);)

    if (!limIsSmeScanReqValid(pMac, pScanReq))
    {
        limLog(pMac, LOGW,
               FL("Received SME_SCAN_REQ with invalid parameters\n"));

        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            limSendSmeScanRsp(pMac, 8, eSIR_SME_INVALID_PARAMETERS);

        } // if (pMac->lim.gLimRspReqd)

        return;
    }

    //if scan is disabled then return as invalid scan request.
    //if scan in power save is disabled, and system is in power save mode, then ignore scan request.
    if( (pMac->lim.fScanDisabled) || (!pMac->lim.gScanInPowersave && !limIsSystemInActiveState(pMac))  )
    {
        limSendSmeScanRsp(pMac, 8, eSIR_SME_INVALID_PARAMETERS);
        return;
    }
    

    /**
     * If scan request is received in idle, joinFailed
     * states or in link established state (in STA role)
     * or in normal state (in STA-in-IBSS/AP role) with
     * 'return fresh scan results' request from HDD or
     * it is periodic background scanning request,
     * trigger fresh scan request to MLM
     */
    if (__limFreshScanReqd(pMac, pScanReq->returnFreshResults))
    {
        // Update global SME state
        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        if ((pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) ||
            (pMac->lim.gLimSmeState == eLIM_SME_JOIN_FAILURE_STATE))
            pMac->lim.gLimSmeState = eLIM_SME_WT_SCAN_STATE;
        else if (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)
            pMac->lim.gLimSmeState = eLIM_SME_NORMAL_CHANNEL_SCAN_STATE;
        else
            pMac->lim.gLimSmeState = eLIM_SME_LINK_EST_WT_SCAN_STATE;
		
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
        if (pScanReq->returnFreshResults & SIR_BG_SCAN_PURGE_RESUTLS)
        {
            // Discard previously cached scan results
            limReInitScanResults(pMac);
        }

        pMac->lim.gLim24Band11dScanDone     = 0;
        pMac->lim.gLim50Band11dScanDone     = 0;
        pMac->lim.gLimReturnAfterFirstMatch =
                                    pScanReq->returnAfterFirstMatch;

        pMac->lim.gLimReturnUniqueResults   =
              ((pScanReq->returnUniqueResults) > 0 ? true : false);

        if (pScanReq->channelList.numChannels == 0)
        {
            // Scan all channels
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmScanReq,
                                                          (sizeof(tLimMlmScanReq) + WNI_CFG_VALID_CHANNEL_LIST_LEN)))
            {
                // Log error
                limLog(pMac, LOGP,
                       FL("call to palAllocateMemory failed for mlmScanReq\n"));

                return;
            }

            // Initialize this buffer
            palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmScanReq,
              (tANI_U32)(sizeof(tLimMlmScanReq) + WNI_CFG_VALID_CHANNEL_LIST_LEN ));

            len = WNI_CFG_VALID_CHANNEL_LIST_LEN;
            if (wlan_cfgGetStr(pMac, WNI_CFG_VALID_CHANNEL_LIST,
                          pMlmScanReq->channelList.channelNumber,
                          &len) != eSIR_SUCCESS)
            {
                /**
                 * Could not get Valid channel list from CFG.
                 * Log error.
                 */
                limLog(pMac, LOGP,
                       FL("could not retrieve Valid channel list\n"));
            }
            pMlmScanReq->channelList.numChannels = (tANI_U8) len;
        }
        else
        {
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmScanReq,
                                                          (sizeof(tLimMlmScanReq) + pScanReq->channelList.numChannels)))
            {
                // Log error
                limLog(pMac, LOGP,
                       FL("call to palAllocateMemory failed for mlmScanReq\n"));

                return;
            }

            // Initialize this buffer
            palZeroMemory( pMac->hHdd, (tANI_U8 *) pMlmScanReq,
              (tANI_U32)(sizeof(tLimMlmScanReq) + pScanReq->channelList.numChannels ));

            pMlmScanReq->channelList.numChannels =
                            pScanReq->channelList.numChannels;

            palCopyMemory( pMac->hHdd, pMlmScanReq->channelList.channelNumber,
                          pScanReq->channelList.channelNumber,
                          pScanReq->channelList.numChannels);
        }

        pMlmScanReq->bssType = pScanReq->bssType;
        palCopyMemory( pMac->hHdd, pMlmScanReq->bssId,
                      pScanReq->bssId,
                      sizeof(tSirMacAddr));

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmScanReq->ssId,
                      (tANI_U8 *) &pScanReq->ssId,
                      pScanReq->ssId.length + 1);

        pMlmScanReq->scanType = pScanReq->scanType;
        pMlmScanReq->backgroundScanMode = pScanReq->backgroundScanMode;
        pMlmScanReq->minChannelTime = pScanReq->minChannelTime;
        pMlmScanReq->maxChannelTime = pScanReq->maxChannelTime;

        // Issue LIM_MLM_SCAN_REQ to MLM
        limPostMlmMessage(pMac, LIM_MLM_SCAN_REQ, (tANI_U32 *) pMlmScanReq);

    } // if ((pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) || ...
    else
    {
        /// In all other cases return 'cached' scan results
        if ((pMac->lim.gLimRspReqd) || pMac->lim.gLimReportBackgroundScanResults)
        {
            tANI_U16    scanRspLen = 8;

            pMac->lim.gLimRspReqd = false;

            if (pMac->lim.gLimSmeScanResultLength == 0)
            {
                limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS);
            }
            else
            {
                scanRspLen = sizeof(tSirSmeScanRsp) +
                             pMac->lim.gLimSmeScanResultLength -
                             sizeof(tSirBssDescription);
                limSendSmeScanRsp(pMac, scanRspLen, eSIR_SME_SUCCESS);
            }

            if (pScanReq->returnFreshResults & SIR_BG_SCAN_PURGE_RESUTLS)
            {
                // Discard previously cached scan results
                limReInitScanResults(pMac);
            }

        } // if (pMac->lim.gLimRspReqd)
    } // else ((pMac->lim.gLimSmeState == eLIM_SME_IDLE_STATE) || ...

#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
#ifdef BACKGROUND_SCAN_ENABLED
    // start background scans if needed
    // There is a bug opened against softmac. Need to enable when the bug is fixed.
    __limBackgroundScanInitiate(pMac);
#endif
#endif

} /*** end __limProcessSmeScanReq() ***/



/**
 * __limProcessSmeJoinReq()
 *
 *FUNCTION:
 * This function is called to process SME_JOIN_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeJoinReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U8            *pBuf;
    tANI_U32           len;
    tSirMacAddr        currentBssId;
    tpSirSmeJoinReq    pSmeJoinReq = NULL;
    tLimMlmJoinReq     *pMlmJoinReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;
    tANI_U32           val = 0;
    tANI_U16 nSize;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_JOIN_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    PELOG1(limLog(pMac, LOG1, FL("Received SME_JOIN_REQ\n"));)

    /**
     * Expect Join request in idle/join failure state.
     * Reassociate request is expected in link established state.
     */
    switch (pMac->lim.gLimSmeState)
    {
        case eLIM_SME_IDLE_STATE:
        case eLIM_SME_JOIN_FAILURE_STATE:
            nSize = __limGetSmeJoinReqSizeForAlloc((tANI_U8*) pMsgBuf);
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSmeJoinReq, nSize))
            {
                limLog(pMac, LOGP, FL("call to palAllocateMemory failed for pSmeJoinReq\n"));
                retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
                goto end;
            }
            (void) palZeroMemory(pMac->hHdd, (void *) pSmeJoinReq, nSize);
 
#if defined(ANI_PRODUCT_TYPE_CLIENT) || defined(ANI_AP_CLIENT_SDK)
            handleHTCapabilityandHTInfo(pMac);
#endif
            
            if ((limJoinReqSerDes(pMac, pSmeJoinReq, (tANI_U8 *)pMsgBuf) == eSIR_FAILURE) ||
                (!limIsSmeJoinReqValid(pMac, pSmeJoinReq)))
            {
                /// Received invalid eWNI_SME_JOIN_REQ
                // Log the event
                PELOGW(limLog(pMac, LOGW, FL("received SME_JOIN_REQ with invalid data\n"));)
                retCode = eSIR_SME_INVALID_PARAMETERS;
                goto end;
            }
            pMac->lim.gpLimJoinReq = pSmeJoinReq;

            // Delete IBSS peer BSSdescription list if any
            limIbssDelete(pMac);

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
            val = sizeof(tLimMlmJoinReq) + sizeof(tSirMacSSidIE) +
                  sizeof(tSirMacRateSetIE) + sizeof(tSirMacDsParamSetIE);
#else
            val = sizeof(tLimMlmJoinReq) + pMac->lim.gpLimJoinReq->bssDescription.length + 2;
#endif
            if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmJoinReq, val))
                limLog(pMac, LOGP, FL("call to palAllocateMemory failed for mlmJoinReq\n"));
            (void) palZeroMemory(pMac->hHdd, (void *) pMlmJoinReq, val);

            if (wlan_cfgGetInt(pMac, WNI_CFG_JOIN_FAILURE_TIMEOUT, (tANI_U32 *) &pMlmJoinReq->joinFailureTimeout)
                != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrieve JoinFailureTimer value\n"));

            len = WNI_CFG_OPERATIONAL_RATE_SET_LEN;
            if (wlan_cfgGetStr(pMac, WNI_CFG_OPERATIONAL_RATE_SET,
                          (tANI_U8 *) &pMlmJoinReq->operationalRateSet.rate,
                          (tANI_U32 *) &len)
                != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrieve Operational rateset\n"));
            pMlmJoinReq->operationalRateSet.numRates = (tANI_U8) len;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
            palCopyMemory( pMac->hHdd, pMlmJoinReq->bssDescription.bssId,
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].bssId,
                sizeof(tSirMacAddr));
            
            pMlmJoinReq->bssDescription.capabilityInfo = 1;
               
            pMlmJoinReq->bssDescription.aniIndicator =
                (tANI_U8) pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].wniIndicator;
            pMlmJoinReq->bssDescription.nwType =
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].nwType;

            pMlmJoinReq->bssDescription.channelId =
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].channelId;

            limCopyNeighborInfoToCfg(pMac,
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0]);

            palCopyMemory( pMac->hHdd, pMac->lim.gLimCurrentBssId,
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].bssId,
                sizeof(tSirMacAddr));

            pMac->lim.gLimCurrentChannelId =
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].channelId;

            pMac->lim.gLimCurrentBssCaps =
                pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].capabilityInfo;

            pMac->lim.gLimCurrentTitanHtCaps =
                    pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].titanHtCaps;

            palCopyMemory( pMac->hHdd,
             (tANI_U8 *) &pMac->lim.gLimCurrentSSID,
             (tANI_U8 *) &pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].ssId,
             pMac->lim.gpLimJoinReq->neighborBssList.bssList[0].ssId.length+1);
#else
            pMlmJoinReq->bssDescription.length =
               pMac->lim.gpLimJoinReq->bssDescription.length;
            palCopyMemory( pMac->hHdd,
               (tANI_U8 *) &pMlmJoinReq->bssDescription.bssId,
               (tANI_U8 *) &pMac->lim.gpLimJoinReq->bssDescription.bssId,
               pMac->lim.gpLimJoinReq->bssDescription.length + 2);

            palCopyMemory( pMac->hHdd,
               pMac->lim.gLimCurrentBssId,
               pMac->lim.gpLimJoinReq->bssDescription.bssId,
               sizeof(tSirMacAddr));

            pMac->lim.gLimCurrentChannelId =
               pMac->lim.gpLimJoinReq->bssDescription.channelId;

            pMac->lim.gLimCurrentBssCaps =
                   pMac->lim.gpLimJoinReq->bssDescription.capabilityInfo;

            pMac->lim.gLimCurrentTitanHtCaps=
                    pMac->lim.gpLimJoinReq->bssDescription.titanHtCaps;

            limExtractApCapability( pMac,
               (tANI_U8 *) pMac->lim.gpLimJoinReq->bssDescription.ieFields,
               limGetIElenFromBssDescription(&pMac->lim.gpLimJoinReq->bssDescription),
               &pMac->lim.gLimCurrentBssQosCaps,
               &pMac->lim.gLimCurrentBssPropCap,
               &pMac->lim.gLimCurrentBssUapsd);

            {
                tANI_U32 cfgLen = SIR_MAC_MAX_SSID_LENGTH;
                if (wlan_cfgGetStr(pMac, WNI_CFG_SSID, pMac->lim.gLimCurrentSSID.ssId, &cfgLen)
                    != eSIR_SUCCESS)
                    limLog(pMac, LOGP, FL("could not retrive SSID\n"));
                pMac->lim.gLimCurrentSSID.length = (tANI_U8) cfgLen;
            }

            if (pMac->lim.gLimCurrentBssUapsd)
            {
				pMac->lim.gUapsdPerAcBitmask = pMac->lim.gpLimJoinReq->uapsdPerAcBitmask;
                limLog( pMac, LOG1, FL("UAPSD flag for all AC - 0x%2x\n"), pMac->lim.gUapsdPerAcBitmask);

                // resetting the dynamic uapsd mask 
                pMac->lim.gUapsdPerAcDeliveryEnableMask = 0;
                pMac->lim.gUapsdPerAcTriggerEnableMask = 0;
            }
#endif

            pMac->lim.gLimRFBand = limGetRFBand(pMac->lim.gLimCurrentChannelId);

            // Initialize 11h Enable Flag
            if((eLIM_STA_IN_IBSS_ROLE != pMac->lim.gLimSystemRole) &&
               (SIR_BAND_5_GHZ == pMac->lim.gLimRFBand))
            {
                if (wlan_cfgGetInt(pMac, WNI_CFG_11H_ENABLED, &val) != eSIR_SUCCESS)
                    limLog(pMac, LOGP, FL("Fail to get WNI_CFG_11H_ENABLED \n"));
                pMac->lim.gLim11hEnable = val;
            }
            else
                pMac->lim.gLim11hEnable = 0;
            
            //To care of the scenario when STA transitions from IBSS to Infrastructure mode.
            pMac->lim.gLimIbssCoalescingHappened = false;

            pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
            pMac->lim.gLimSmeState     = eLIM_SME_WT_JOIN_STATE;
	     MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

            PELOG1(limLog(pMac, LOG1, FL("SME JoinReq: SSID %d.%c%c%c%c%c%c\n"),
                   pMac->lim.gLimCurrentSSID.length,
                   pMac->lim.gLimCurrentSSID.ssId[0],
                   pMac->lim.gLimCurrentSSID.ssId[1],
                   pMac->lim.gLimCurrentSSID.ssId[2],
                   pMac->lim.gLimCurrentSSID.ssId[3],
                   pMac->lim.gLimCurrentSSID.ssId[4],
                   pMac->lim.gLimCurrentSSID.ssId[5]);
           limLog(pMac, LOG1, FL("Channel %d, BSSID %x:%x:%x:%x:%x:%x\n"),
                   pMac->lim.gLimCurrentChannelId,
                   pMac->lim.gLimCurrentBssId[0],
                   pMac->lim.gLimCurrentBssId[1],
                   pMac->lim.gLimCurrentBssId[2],
                   pMac->lim.gLimCurrentBssId[3],
                   pMac->lim.gLimCurrentBssId[4],
                   pMac->lim.gLimCurrentBssId[5]);)

            // Issue LIM_MLM_JOIN_REQ to MLM
            limPostMlmMessage(pMac, LIM_MLM_JOIN_REQ, (tANI_U32 *) pMlmJoinReq);
            return;

        case eLIM_SME_LINK_EST_STATE:
            pBuf = limGetBssIdFromSmeJoinReqMsg((tANI_U8 *) pMsgBuf);

            len = sizeof(tSirMacAddr);
            if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &len) != eSIR_SUCCESS)
                limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));

            if (pBuf && ( palEqualMemory( pMac->hHdd,pBuf, currentBssId, sizeof( tSirMacAddr ) ) ) )
            {
                // Received eWNI_SME_JOIN_REQ for same
                // BSS as currently associated.
                // Log the event and send success
                PELOGW(limLog(pMac, LOGW, FL("Received SME_JOIN_REQ for currently joined BSS\n"));)
                /// Send Join success response to host
                retCode = eSIR_SME_SUCCESS;
                goto end;
            }
            // JOIN_REQ for some other BSS.
            // Fall through to default case.

        default:
            /// Should not have received eWNI_SME_JOIN_REQ
            // Log the event
            limLog(pMac, LOGE, FL("received unexpected SME_JOIN_REQ in state %X\n"),
                   pMac->lim.gLimSmeState);
            limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
            retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            goto end;
    }

end:
    if (pSmeJoinReq)
        palFreeMemory( pMac->hHdd, pSmeJoinReq);
    /// Send Join failure response to host
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_JOIN_RSP, retCode, eSIR_MAC_UNSPEC_FAILURE_STATUS);
} /*** end __limProcessSmeJoinReq() ***/


#if 0
/**
 * __limProcessSmeAuthReq()
 *
 *FUNCTION:
 * This function is called to process SME_AUTH_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeAuthReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{

    tAniAuthType       authMode;
    tLimMlmAuthReq     *pMlmAuthReq;
    tpSirSmeAuthReq    pSirSmeAuthReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;

    pSirSmeAuthReq = (tpSirSmeAuthReq) pMsgBuf;

    if (!limIsSmeAuthReqValid(pSirSmeAuthReq))
    {
        limLog(pMac, LOGW,
               FL("received invalid SME_AUTH_REQ message\n"));

        /// Send AUTH failure response to host
        retCode = eSIR_SME_INVALID_PARAMETERS;
        goto end;
    }

    PELOG1(limLog(pMac, LOG1,
           FL("RECEIVED AUTH_REQ\n"));)

    /**
     * Expect Auth request for STA in link established state
     * or STA in IBSS mode in normal state.
     */

    if ((pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE) ||
        (pMac->lim.gLimSmeState == eLIM_SME_JOIN_FAILURE_STATE) ||
        ((pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) &&
         (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
    {
        if (pSirSmeAuthReq->authType == eSIR_AUTO_SWITCH)
            authMode = eSIR_SHARED_KEY; // Try Shared Key first
        else
            authMode = pSirSmeAuthReq->authType;

        // Trigger MAC based Authentication
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmAuthReq, sizeof(tLimMlmAuthReq)))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmAuthReq\n"));
            return;
        }

        pMac->lim.gLimPreAuthType = pSirSmeAuthReq->authType;

        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        pMac->lim.gLimSmeState     = eLIM_SME_WT_PRE_AUTH_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        // Store channel specified in auth request.
        // This will be programmed later by MLM.
        pMac->lim.gLimPreAuthChannelNumber =
                                    (tSirMacChanNum)
                                    pSirSmeAuthReq->channelNumber;

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMac->lim.gLimPreAuthPeerAddr,
                      (tANI_U8 *) &pSirSmeAuthReq->peerMacAddr,
                      sizeof(tSirMacAddr));

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmAuthReq->peerMacAddr,
                      (tANI_U8 *) &pSirSmeAuthReq->peerMacAddr,
                      sizeof(tSirMacAddr));

        pMlmAuthReq->authType = authMode;

        if (wlan_cfgGetInt(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
                      (tANI_U32 *) &pMlmAuthReq->authFailureTimeout)
                            != eSIR_SUCCESS)
        {
            /**
             * Could not get AuthFailureTimeout value from CFG.
             * Log error.
             */
            limLog(pMac, LOGP,
                   FL("could not retrieve AuthFailureTimeout value\n"));
        }

        limPostMlmMessage(pMac, LIM_MLM_AUTH_REQ, (tANI_U32 *) pMlmAuthReq);
        return;
    }
    else
    {
        /// Should not have received eWNI_SME_AUTH_REQ
        // Log the event
        limLog(pMac, LOGE,
               FL("received unexpected SME_AUTH_REQ in state %X\n"),
               pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

        /// Send AUTH failure response to host
        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
        goto end;
    }

end:
    limSendSmeAuthRsp(pMac, retCode,
                      pSirSmeAuthReq->peerMacAddr,
                      pSirSmeAuthReq->authType,
                      eSIR_MAC_UNSPEC_FAILURE_STATUS );

} /*** end __limProcessSmeAuthReq() ***/
#endif


/**
 * __limProcessSmeReassocReq()
 *
 *FUNCTION:
 * This function is called to process SME_REASSOC_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeReassocReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                caps;
    tANI_U32                val;
    tpSirSmeReassocReq pReassocReq = NULL;
    tLimMlmReassocReq  *pMlmReassocReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_REASSOC_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    PELOG3(limLog(pMac, LOG3, FL("Received REASSOC_REQ\n"));)

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pReassocReq, __limGetSmeJoinReqSizeForAlloc((tANI_U8 *) pMsgBuf)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for pReassocReq\n"));

        retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }

    if ((limJoinReqSerDes(pMac, (tpSirSmeJoinReq) pReassocReq,
                          (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        (!limIsSmeJoinReqValid(pMac,
                               (tpSirSmeJoinReq) pReassocReq)))
    {
        /// Received invalid eWNI_SME_REASSOC_REQ
        // Log the event
        limLog(pMac, LOGW,
               FL("received SME_REASSOC_REQ with invalid data\n"));

        retCode = eSIR_SME_INVALID_PARAMETERS;
        goto end;
    }
    pMac->lim.gpLimReassocReq = pReassocReq;

    /**
     * Reassociate request is expected
     * in link established state only.
     */

    if (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE)
    {
        /// Should not have received eWNI_SME_REASSOC_REQ
        // Log the event
        limLog(pMac, LOGE,
               FL("received unexpected SME_REASSOC_REQ in state %X\n"),
               pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
        goto end;
    }

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
    limCopyNeighborInfoToCfg(pMac,
        pMac->lim.gpLimReassocReq->neighborBssList.bssList[0]);

    palCopyMemory( pMac->hHdd,
             pMac->lim.gLimReassocBssId,
             pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].bssId,
             sizeof(tSirMacAddr));

    pMac->lim.gLimReassocChannelId =
         pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].channelId;

    pMac->lim.gLimReassocBssCaps =
    pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].capabilityInfo;

    pMac->lim.gLimReassocTitanHtCaps = 
        pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].titanHtCaps;

    palCopyMemory( pMac->hHdd,
    (tANI_U8 *) &pMac->lim.gLimReassocSSID,
    (tANI_U8 *) &pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].ssId,
    pMac->lim.gpLimReassocReq->neighborBssList.bssList[0].ssId.length+1);
#else
    palCopyMemory( pMac->hHdd,
             pMac->lim.gLimReassocBssId,
             pMac->lim.gpLimReassocReq->bssDescription.bssId,
             sizeof(tSirMacAddr));

    pMac->lim.gLimReassocChannelId =
         pMac->lim.gpLimReassocReq->bssDescription.channelId;

    pMac->lim.gLimReassocBssCaps =
                pMac->lim.gpLimReassocReq->bssDescription.capabilityInfo;

    pMac->lim.gLimReassocTitanHtCaps =
            pMac->lim.gpLimReassocReq->bssDescription.titanHtCaps;
    
    limExtractApCapability( pMac,
              (tANI_U8 *) pMac->lim.gpLimReassocReq->bssDescription.ieFields,
              limGetIElenFromBssDescription(
              &pMac->lim.gpLimReassocReq->bssDescription),
              &pMac->lim.gLimReassocBssQosCaps,
              &pMac->lim.gLimReassocBssPropCap,
              &pMac->lim.gLimReassocBssUapsd);

    {
    tANI_U32 cfgLen = SIR_MAC_MAX_SSID_LENGTH;
    if (wlan_cfgGetStr(pMac, WNI_CFG_SSID, pMac->lim.gLimReassocSSID.ssId,
                  &cfgLen) != eSIR_SUCCESS)
    {
        /// Could not get SSID from CFG. Log error.
        limLog(pMac, LOGP, FL("could not retrive SSID\n"));
    }
    pMac->lim.gLimReassocSSID.length = (tANI_U8) cfgLen;
    }

	if (pMac->lim.gLimReassocBssUapsd)
	{
        pMac->lim.gUapsdPerAcBitmask = pMac->lim.gpLimReassocReq->uapsdPerAcBitmask;
        limLog( pMac, LOG1, FL("UAPSD flag for all AC - 0x%2x\n"), pMac->lim.gUapsdPerAcBitmask);
	}

#endif

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmReassocReq, sizeof(tLimMlmReassocReq)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmReassocReq\n"));

        retCode = eSIR_SME_RESOURCES_UNAVAILABLE;
        goto end;
    }

    palCopyMemory( pMac->hHdd, pMlmReassocReq->peerMacAddr,
                  pMac->lim.gLimReassocBssId,
                  sizeof(tSirMacAddr));

    if (wlan_cfgGetInt(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
                  (tANI_U32 *) &pMlmReassocReq->reassocFailureTimeout)
                           != eSIR_SUCCESS)
    {
        /**
         * Could not get ReassocFailureTimeout value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP,
               FL("could not retrieve ReassocFailureTimeout value\n"));
    }

    if (cfgGetCapabilityInfo(pMac, &caps) != eSIR_SUCCESS)
    {
        /**
         * Could not get Capabilities value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP,
               FL("could not retrieve Capabilities value\n"));
    }
    pMlmReassocReq->capabilityInfo = caps;

    if (wlan_cfgGetInt(pMac, WNI_CFG_LISTEN_INTERVAL, &val) != eSIR_SUCCESS)
    {
        /**
         * Could not get ListenInterval value
         * from CFG. Log error.
         */
        limLog(pMac, LOGP, FL("could not retrieve ListenInterval\n"));
    }

    /* Delete all BA sessions before Re-Assoc.
     *  BA frames are class 3 frames and the session 
     *  is lost upon disassociation and reassociation.
     */

    limDelAllBASessions(pMac);

    pMlmReassocReq->listenInterval = (tANI_U16) val;

    pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
    pMac->lim.gLimSmeState     = eLIM_SME_WT_REASSOC_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

    limPostMlmMessage(pMac,
                      LIM_MLM_REASSOC_REQ,
                      (tANI_U32 *) pMlmReassocReq);
    return;

end:
    if (pReassocReq)
        palFreeMemory( pMac->hHdd, pReassocReq);

    /// Send Reassoc failure response to host
    limSendSmeJoinReassocRsp(pMac, eWNI_SME_REASSOC_RSP,
                             retCode, eSIR_MAC_UNSPEC_FAILURE_STATUS);
} /*** end __limProcessSmeReassocReq() ***/



/**
 * __limProcessSmeDisassocReq()
 *
 *FUNCTION:
 * This function is called to process SME_DISASSOC_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeDisassocReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                disassocTrigger, reasonCode;
    tLimMlmDisassocReq *pMlmDisassocReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS;
    tSirSmeDisassocReq smeDisassocReq;

    retCode = limDisassocReqSerDes(pMac, &smeDisassocReq, (tANI_U8 *) pMsgBuf);
	
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (retCode != eSIR_FAILURE)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_REQ_EVENT, NULL, 0, smeDisassocReq.reasonCode);
#endif //FEATURE_WLAN_DIAG_SUPPORT
	
    if ( (retCode == eSIR_FAILURE) ||(!limIsSmeDisassocReqValid(pMac, &smeDisassocReq)) )
    {
        PELOGE(limLog(pMac, LOGE,
               FL("received invalid SME_DISASSOC_REQ message\n"));)

        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            retCode         = eSIR_SME_INVALID_PARAMETERS;
            disassocTrigger = eLIM_HOST_DISASSOC;
            goto sendDisassoc;
        }

        return;
    }


    PELOGE(limLog(pMac, LOGE,   FL("received DISASSOC_REQ message. Reason: %d SmeState: %d\n"), 
                                                        smeDisassocReq.reasonCode, pMac->lim.gLimSmeState);)


    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_ROLE:
            switch (pMac->lim.gLimSmeState)
            {
                case eLIM_SME_ASSOCIATED_STATE:
                case eLIM_SME_LINK_EST_STATE:
                    pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
                    pMac->lim.gLimSmeState = eLIM_SME_WT_DISASSOC_STATE;
                    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
                    break;

                case eLIM_SME_WT_DEAUTH_STATE:
                    /* PE shall still process the DISASSOC_REQ and proceed with 
                     * link tear down even if it had already sent a DEAUTH_IND to
                     * to SME. pMac->lim.gLimPrevSmeState shall remain the same as
                     * its been set when PE entered WT_DEAUTH_STATE. 
                     */					 
                    pMac->lim.gLimSmeState = eLIM_SME_WT_DISASSOC_STATE;
                    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
                    limLog(pMac, LOG1, FL("Rcvd SME_DISASSOC_REQ while in SME_WT_DEAUTH_STATE. \n"));
                    break;

                case eLIM_SME_JOIN_FAILURE_STATE: {
                    /** Return Success as we are already in Disconnected State*/
                     if (pMac->lim.gLimRspReqd) {
                        retCode = eSIR_SME_SUCCESS;  
                        disassocTrigger = eLIM_HOST_DISASSOC;
                        goto sendDisassoc;
                    }
                }break;
                default:
                    /**
                     * STA is not currently associated.
                     * Log error and send response to host
                     */
                    limLog(pMac, LOGE,
                       FL("received unexpected SME_DISASSOC_REQ in state %X\n"),
                       pMac->lim.gLimSmeState);
                    limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

                    if (pMac->lim.gLimRspReqd)
                    {
                        if (pMac->lim.gLimSmeState !=
                                                eLIM_SME_WT_ASSOC_STATE)
                                    pMac->lim.gLimRspReqd = false;

                        retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
                        disassocTrigger = eLIM_HOST_DISASSOC;
                        goto sendDisassoc;
                    }

                    return;
            }

            break;

        case eLIM_AP_ROLE:
            // Fall through
            break;

        case eLIM_STA_IN_IBSS_ROLE:
        default: // eLIM_UNKNOWN_ROLE
            limLog(pMac, LOGE,
               FL("received unexpected SME_DISASSOC_REQ for role %d\n"),
               pMac->lim.gLimSystemRole);

            retCode = eSIR_SME_UNEXPECTED_REQ_RESULT_CODE;
            disassocTrigger = eLIM_HOST_DISASSOC;
            goto sendDisassoc;
    } // end switch (pMac->lim.gLimSystemRole)

    if (smeDisassocReq.reasonCode == eLIM_LINK_MONITORING_DISASSOC)
    {
        /// Disassociation is triggered by Link Monitoring
        disassocTrigger = eLIM_LINK_MONITORING_DISASSOC;
        reasonCode      = eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON;
    }
    else
    {
        disassocTrigger = eLIM_HOST_DISASSOC;
        reasonCode      = smeDisassocReq.reasonCode;
    }

    // Trigger Disassociation frame to peer MAC entity
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDisassocReq, sizeof(tLimMlmDisassocReq)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmDisassocReq\n"));

        return;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDisassocReq->peerMacAddr,
                  (tANI_U8 *) &smeDisassocReq.peerMacAddr,
                  sizeof(tSirMacAddr));

    pMlmDisassocReq->reasonCode      = reasonCode;
    pMlmDisassocReq->disassocTrigger = disassocTrigger;
#ifdef ANI_PRODUCT_TYPE_AP
    pMlmDisassocReq->aid             = smeDisassocReq.aid;
#endif

    limPostMlmMessage(pMac,
                      LIM_MLM_DISASSOC_REQ,
                      (tANI_U32 *) pMlmDisassocReq);
    return;

sendDisassoc:
    limSendSmeDisassocNtf(pMac, smeDisassocReq.peerMacAddr,
                          retCode,
                          disassocTrigger,
#ifdef ANI_PRODUCT_TYPE_AP
                          smeDisassocReq.aid);
#else
                          1);
#endif
} /*** end __limProcessSmeDisassocReq() ***/


/** -----------------------------------------------------------------
  \brief __limProcessSmeDisassocCnf() - Process SME_DISASSOC_CNF
   
  This function is called to process SME_DISASSOC_CNF message
  from HDD or upper layer application. 
    
  \param pMac - global mac structure
  \param pStaDs - station dph hash node 
  \return none 
  \sa
  ----------------------------------------------------------------- */
static void
__limProcessSmeDisassocCnf(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeDisassocCnf  smeDisassocCnf;
    tANI_U16  aid;
    tpDphHashNode  pStaDs;
    tSirRetStatus  status = eSIR_SUCCESS;

    PELOG1(limLog(pMac, LOG1, FL("received DISASSOC_CNF message\n"));)

	status = limDisassocCnfSerDes(pMac, &smeDisassocCnf,(tANI_U8 *) pMsgBuf);

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (status != eSIR_FAILURE)
	{
        if (smeDisassocCnf.messageType == eWNI_SME_DISASSOC_CNF)
            limDiagEventReport(pMac, WLAN_PE_DIAG_DISASSOC_CNF_EVENT, NULL, (tANI_U16)smeDisassocCnf.statusCode, 0);
        else if (smeDisassocCnf.messageType ==  eWNI_SME_DEAUTH_CNF)
            limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_CNF_EVENT, NULL, (tANI_U16)smeDisassocCnf.statusCode, 0);
	}
#endif //FEATURE_WLAN_DIAG_SUPPORT

    if ((status == eSIR_FAILURE) || (!limIsSmeDisassocCnfValid(pMac, &smeDisassocCnf)))
    {
        limLog(pMac, LOGW, FL("received invalid SME_DISASSOC_CNF message\n"));
        return;
    }

    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_ROLE:
            if ((pMac->lim.gLimSmeState != eLIM_SME_IDLE_STATE) &&
                (pMac->lim.gLimSmeState != eLIM_SME_WT_DISASSOC_STATE) &&
                (pMac->lim.gLimSmeState != eLIM_SME_WT_DEAUTH_STATE))
            {
                limLog(pMac, LOGE,
                   FL("received unexp SME_DISASSOC_CNF in state %X\n"),
                   pMac->lim.gLimSmeState);
                limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
                return;
            }
            break;

        case eLIM_AP_ROLE:
            // Fall through
            return;

        case eLIM_STA_IN_IBSS_ROLE:
        default: // eLIM_UNKNOWN_ROLE
            limLog(pMac, LOGE,
               FL("received unexpected SME_DISASSOC_CNF role %d\n"),
               pMac->lim.gLimSystemRole);

            return;
    } 

    if (  (pMac->lim.gLimSmeState == eLIM_SME_WT_DISASSOC_STATE) || 
           (pMac->lim.gLimSmeState == eLIM_SME_WT_DEAUTH_STATE) )
    {       
        pStaDs = dphLookupHashEntry(pMac, smeDisassocCnf.peerMacAddr, &aid);
        if (pStaDs == NULL)
        {
            PELOGE(limLog(pMac, LOGE, FL("received DISASSOC_CNF for a STA that does not have context, addr= "));)
            limPrintMacAddr(pMac, smeDisassocCnf.peerMacAddr, LOGW);
            return;
        }
        limCleanupRxPath(pMac, pStaDs);
	}
    return;
} 


/**
 * __limProcessSmeDeauthReq()
 *
 *FUNCTION:
 * This function is called to process SME_DEAUTH_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeDeauthReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16                deauthTrigger, reasonCode;
    tLimMlmDeauthReq   *pMlmDeauthReq;
    tSirSmeDeauthReq   smeDeauthReq;
    tSirResultCodes    retCode = eSIR_SME_SUCCESS; 
    tSirRetStatus      status = eSIR_SUCCESS;

    status = limDeauthReqSerDes(pMac, &smeDeauthReq,(tANI_U8 *) pMsgBuf);

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    if (status != eSIR_FAILURE)
        limDiagEventReport(pMac, WLAN_PE_DIAG_DEAUTH_REQ_EVENT, NULL, 0, smeDeauthReq.reasonCode);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    if ((status == eSIR_FAILURE) || (!limIsSmeDeauthReqValid(pMac, &smeDeauthReq)))
    {
        PELOGE(limLog(pMac, LOGE,
               FL("received invalid SME_DEAUTH_REQ message\n"));)

        if (pMac->lim.gLimRspReqd)
        {
            pMac->lim.gLimRspReqd = false;

            retCode       = eSIR_SME_INVALID_PARAMETERS;
            deauthTrigger = eLIM_HOST_DEAUTH;
            goto sendDeauth;
        }

        return;
    }

    PELOGE(limLog(pMac, LOGE,   FL("received DEAUTH_REQ message. Reason: %d SmeState: %d \n"), 
                                                        smeDeauthReq.reasonCode, pMac->lim.gLimSmeState);)



    switch (pMac->lim.gLimSystemRole)
    {
        case eLIM_STA_ROLE:
            switch (pMac->lim.gLimSmeState)
            {
                case eLIM_SME_ASSOCIATED_STATE:
                case eLIM_SME_LINK_EST_STATE:
                case eLIM_SME_WT_ASSOC_STATE:
                case eLIM_SME_JOIN_FAILURE_STATE:
                case eLIM_SME_IDLE_STATE:
                    pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
                    pMac->lim.gLimSmeState = eLIM_SME_WT_DEAUTH_STATE;
		      MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

                    // Send Deauthentication request to MLM below

                    break;

                default:
                    /**
                     * STA is not in a state to deauthenticate with
                     * peer. Log error and send response to host.
                     */
                    limLog(pMac, LOGE,
                      FL("received unexp SME_DEAUTH_REQ in state %X\n"),
                      pMac->lim.gLimSmeState);
                    limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

                    if (pMac->lim.gLimRspReqd)
                    {
                        pMac->lim.gLimRspReqd = false;

                        retCode       = eSIR_SME_STA_NOT_AUTHENTICATED;
                        deauthTrigger = eLIM_HOST_DEAUTH;
                        goto sendDeauth;
                    }

                    return;
            }

            break;

        case eLIM_STA_IN_IBSS_ROLE:

            return;

        case eLIM_AP_ROLE:
            // Fall through

            break;

        default:
            limLog(pMac, LOGE,
               FL("received unexpected SME_DEAUTH_REQ for role %X\n"),
               pMac->lim.gLimSystemRole);

            return;
    } // end switch (pMac->lim.gLimSystemRole)

    if (smeDeauthReq.reasonCode == eLIM_LINK_MONITORING_DEAUTH)
    {
        /// Deauthentication is triggered by Link Monitoring
        PELOG1(limLog(pMac, LOG1, FL("**** Lost link with AP ****\n"));)
        deauthTrigger = eLIM_LINK_MONITORING_DEAUTH;
        reasonCode    = eSIR_MAC_UNSPEC_FAILURE_REASON;
    }
    else
    {
        deauthTrigger = eLIM_HOST_DEAUTH;
        reasonCode    = smeDeauthReq.reasonCode;
    }

    // Trigger Deauthentication frame to peer MAC entity
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDeauthReq, sizeof(tLimMlmDeauthReq)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for mlmDeauthReq\n"));

        return;
    }

    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDeauthReq->peerMacAddr,
                  (tANI_U8 *) &smeDeauthReq.peerMacAddr,
                  sizeof(tSirMacAddr));

    pMlmDeauthReq->reasonCode = reasonCode;
    pMlmDeauthReq->deauthTrigger = deauthTrigger;
#ifdef ANI_PRODUCT_TYPE_AP
    pMlmDeauthReq->aid = smeDeauthReq.aid;
#endif

    limPostMlmMessage(pMac,
                      LIM_MLM_DEAUTH_REQ,
                      (tANI_U32 *) pMlmDeauthReq);
    return;

sendDeauth:
    limSendSmeDeauthNtf(pMac, smeDeauthReq.peerMacAddr,
                        retCode,
                        deauthTrigger,
#ifdef ANI_PRODUCT_TYPE_AP
                        smeDeauthReq.aid);
#else
                        1);
#endif
} /*** end __limProcessSmeDeauthReq() ***/



/**
 * __limProcessSmeSetContextReq()
 *
 *FUNCTION:
 * This function is called to process SME_SETCONTEXT_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeSetContextReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirSmeSetContextReq  pSetContextReq;
    tLimMlmSetKeysReq      *pMlmSetKeysReq;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_SETCONTEXT_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    PELOG1(limLog(pMac, LOG1,
           FL("received SETCONTEXT_REQ message\n")););

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSetContextReq,
                                                (sizeof(tSirKeys) * SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS)))
    {
        // Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for pSetContextReq\n"));

        return;
    }

    if ((limSetContextReqSerDes(pMac,
                                pSetContextReq,
                                (tANI_U8 *) pMsgBuf) == eSIR_FAILURE) ||
        (!limIsSmeSetContextReqValid(pMac, pSetContextReq)))
    {
        limLog(pMac, LOGW,
               FL("received invalid SME_SETCONTEXT_REQ message\n"));

        limSendSmeSetContextRsp(pMac,
                                pSetContextReq->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                                pSetContextReq->aid,
#else
                                1,
#endif
                                eSIR_SME_INVALID_PARAMETERS);

        goto end;
    }

    if (((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE)) ||
        (((pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) ||
          (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)) &&
         (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
    {
        // Trigger MLM_SETKEYS_REQ
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmSetKeysReq, sizeof(tLimMlmSetKeysReq)))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmSetKeysReq\n"));

            goto end;
        }

        pMlmSetKeysReq->edType  = pSetContextReq->keyMaterial.edType;
        pMlmSetKeysReq->numKeys = pSetContextReq->keyMaterial.numKeys;

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmSetKeysReq->peerMacAddr,
                      (tANI_U8 *) &pSetContextReq->peerMacAddr,
                      sizeof(tSirMacAddr));

#ifdef ANI_PRODUCT_TYPE_AP
        pMlmSetKeysReq->aid = pSetContextReq->aid;
#endif

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmSetKeysReq->key,
                      (tANI_U8 *) &pSetContextReq->keyMaterial.key,
                      sizeof(tSirKeys) * (pMlmSetKeysReq->numKeys ? pMlmSetKeysReq->numKeys : 1));

        limPostMlmMessage(pMac,
                          LIM_MLM_SETKEYS_REQ,
                          (tANI_U32 *) pMlmSetKeysReq);
#ifdef ANI_AP_SDK
        /* For SDK acting as STA under Linux, need to consider the AP as *
         * as authenticatated.                                           */
        if ((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
           (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE))
        {
            tpDphHashNode pSta;
            pSta = dphGetHashEntry(pMac, 0);
            if (pSta)
            pSta->staAuthenticated = 1;
        }
#endif
    }
    else
    {
        limLog(pMac, LOGE,
           FL("received unexpected SME_SETCONTEXT_REQ for role %d, state=%X\n"),
           pMac->lim.gLimSystemRole,
           pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

        limSendSmeSetContextRsp(pMac,
                                pSetContextReq->peerMacAddr,
#ifdef ANI_PRODUCT_TYPE_AP
                                pSetContextReq->aid,
#else
                                1,
#endif
                                eSIR_SME_UNEXPECTED_REQ_RESULT_CODE);
    }

end:
    palFreeMemory( pMac->hHdd, pSetContextReq);
} /*** end __limProcessSmeSetContextReq() ***/

/**
 * __limProcessSmeRemoveKeyReq()
 *
 *FUNCTION:
 * This function is called to process SME_REMOVEKEY_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmeRemoveKeyReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirSmeRemoveKeyReq    pRemoveKeyReq;
    tLimMlmRemoveKeyReq     *pMlmRemoveKeyReq;

    PELOG1(limLog(pMac, LOG1,
           FL("received REMOVEKEY_REQ message\n"));)

    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pRemoveKeyReq,
                                                (sizeof(*pRemoveKeyReq))))
    {
        //Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for pRemoveKeyReq\n"));

        return;
     }

    if ((limRemoveKeyReqSerDes(pMac,
                                pRemoveKeyReq,
                                (tANI_U8 *) pMsgBuf) == eSIR_FAILURE))
    {
        limLog(pMac, LOGW,
               FL("received invalid SME_REMOVECONTEXT_REQ message\n"));

        limSendSmeRemoveKeyRsp(pMac,
                                pRemoveKeyReq->peerMacAddr,
                                eSIR_SME_INVALID_PARAMETERS);

        goto end;
    }

    if (((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) &&
         (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE)) ||
        (((pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE) ||
          (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)) &&
         (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
    {
        // Trigger MLM_REMOVEKEYS_REQ
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmRemoveKeyReq, sizeof(tLimMlmRemoveKeyReq)))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmRemoveKeysReq\n"));

            goto end;
        }

        pMlmRemoveKeyReq->edType  = (tAniEdType)pRemoveKeyReq->edType; 
        pMlmRemoveKeyReq->keyId = pRemoveKeyReq->keyId;
        pMlmRemoveKeyReq->wepType = pRemoveKeyReq->wepType;
        pMlmRemoveKeyReq->unicast = pRemoveKeyReq->unicast;

        palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmRemoveKeyReq->peerMacAddr,
                      (tANI_U8 *) &pRemoveKeyReq->peerMacAddr,
                      sizeof(tSirMacAddr));


        limPostMlmMessage(pMac,
                          LIM_MLM_REMOVEKEY_REQ,
                          (tANI_U32 *) pMlmRemoveKeyReq);
    }
    else
    {
        limLog(pMac, LOGE,
           FL("received unexpected SME_REMOVEKEY_REQ for role %d, state=%X\n"),
           pMac->lim.gLimSystemRole,
           pMac->lim.gLimSmeState);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);

        limSendSmeRemoveKeyRsp(pMac,
                                pRemoveKeyReq->peerMacAddr,
                                eSIR_SME_UNEXPECTED_REQ_RESULT_CODE);
    }

end:
    palFreeMemory( pMac->hHdd, pRemoveKeyReq);
} /*** end __limProcessSmeRemoveKeyReq() ***/



#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
/**
 * __limHandleSmeSwitchChlRequest()
 *
 *FUNCTION:
 *  This function is called to process the following SME messages
 *  received from HDD or WSM:
 *      - eWNI_SME_SWITCH_CHL_REQ
 *      - eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ
 *      - eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ
 *
 *ASSUMPTIONS:
 *
 *  eWNI_SME_SWITCH_CHL_REQ is issued only when 11h is enabled,
 *  and WSM wishes to switch its primary channel. AP shall
 *  populate the 802.11h channel switch IE in its Beacons/Probe Rsp.
 *
 *  eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ is issued only when 11h is enabled,
 *  and WSM wishes to switch both its primary channel and secondary channel.
 *  (In the case of if 11h is disabled, and WSM wants to change both
 *  primary & secondary channel, then WSM should issue a restart-BSS). AP
 *  shall populate the 802.11h channel switch IE in its Beacons/Probe Rsp.
 *
 *  eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ is issued when WSM wishes to
 *  switch/disable only its secondary channel. This can occur when 11h
 *  is enabled or disabled. AP shall populate the airgo proprietary
 *  channel switch IE in its Beacons/Probe Rsp.
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limHandleSmeSwitchChlRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpSirSmeSwitchChannelReq   pSmeMsg;
    eHalStatus                 status;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_SWITCH_CHL_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    if (pMac->lim.gLimSmeState != eLIM_SME_NORMAL_STATE ||
            pMac->lim.gLimSystemRole != eLIM_AP_ROLE ||
            pMac->lim.gLimSpecMgmt.dot11hChanSwState == eLIM_11H_CHANSW_RUNNING)
    {
        PELOGE(limLog(pMac, LOGE, "Rcvd Switch Chl Req in wrong state\n");)
        limSendSmeRsp(pMac, eWNI_SME_SWITCH_CHL_RSP, eSIR_SME_CHANNEL_SWITCH_FAIL);
        return;
    }
                
    status = palAllocateMemory( pMac->hHdd, (void **)&pSmeMsg, sizeof(tSirSmeSwitchChannelReq));
    if( eHAL_STATUS_SUCCESS != status)
    {
        PELOGE(limLog(pMac, LOGE, FL("palAllocateMemory failed, status = %d\n"), status);)
        return;
    }

    if (!limIsSmeSwitchChannelReqValid(pMac, (tANI_U8 *)pMsgBuf, pSmeMsg))
    {
        limLog(pMac, LOGE,
            FL("invalid sme message received\n"));
        palFreeMemory( pMac->hHdd, pSmeMsg);
        limSendSmeRsp(pMac, eWNI_SME_SWITCH_CHL_RSP, eSIR_SME_INVALID_PARAMETERS);
        return;
    }


    /* If we're already doing channel switching and we're in the
     * middle of counting down, then reject this channel switch msg.
     */
    if (pMac->lim.gLimChannelSwitch.state != eLIM_CHANNEL_SWITCH_IDLE)
    {
        limLog(pMac, LOGE,
            FL("channel switching is already in progress.\n"));
        palFreeMemory( pMac->hHdd, pSmeMsg);
        limSendSmeRsp(pMac, eWNI_SME_SWITCH_CHL_RSP, eSIR_SME_CHANNEL_SWITCH_DISABLED);
        return;
    }

    PELOG1(limLog(pMac, LOG1, FL("rcvd eWNI_SME_SWITCH_CHL_REQ, message type = %d\n"), pSmeMsg->messageType);)
    switch(pSmeMsg->messageType)
    {
        case eWNI_SME_SWITCH_CHL_REQ:
            pMac->lim.gLimChannelSwitch.state = eLIM_CHANNEL_SWITCH_PRIMARY_ONLY;
            break;

        case eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ:

            pMac->lim.gLimChannelSwitch.state = eLIM_CHANNEL_SWITCH_PRIMARY_AND_SECONDARY;
            break;

        case eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ:
            pMac->lim.gLimChannelSwitch.state = eLIM_CHANNEL_SWITCH_SECONDARY_ONLY;
            break;

        default:
            PELOGE(limLog(pMac, LOGE, FL("unknown message\n"));)
            palFreeMemory( pMac->hHdd, pSmeMsg);
            limSendSmeRsp(pMac, eWNI_SME_SWITCH_CHL_RSP, eSIR_SME_INVALID_PARAMETERS);
            return;
    }

    pMac->lim.gLimChannelSwitch.primaryChannel = pSmeMsg->channelId;
    pMac->lim.gLimChannelSwitch.secondarySubBand = pSmeMsg->cbMode;
    pMac->lim.gLimChannelSwitch.switchCount = computeChannelSwitchCount(pMac, pSmeMsg->dtimFactor);
    if (LIM_IS_RADAR_DETECTED(pMac))
    {
        /** Measurement timers not running */
        pMac->lim.gLimChannelSwitch.switchMode = eSIR_CHANSW_MODE_SILENT;
    }
    else
    {
        /** Stop measurement timers till channel switch */
        limStopMeasTimers(pMac);
        pMac->lim.gLimChannelSwitch.switchMode = eSIR_CHANSW_MODE_NORMAL;
    }

    PELOG1(limLog(pMac, LOG1, FL("state %d, primary %d, subband %d, count %d \n"),
           pMac->lim.gLimChannelSwitch.state,
           pMac->lim.gLimChannelSwitch.primaryChannel,
           pMac->lim.gLimChannelSwitch.secondarySubBand,
           pMac->lim.gLimChannelSwitch.switchCount);)
    palFreeMemory( pMac->hHdd, pSmeMsg);
    
    pMac->lim.gLimSpecMgmt.quietState = eLIM_QUIET_END;
    pMac->lim.gLimSpecMgmt.dot11hChanSwState = eLIM_11H_CHANSW_RUNNING;
    
    return;
} /*** end __limHandleSmeSwitchChlRequest() ***/


/**--------------------------------------------------------------
\fn     __limProcessSmeSwitchChlReq

\brief  Wrapper for the function __limHandleSmeSwitchChlRequest
        This message will be defered until softmac come out of
        scan mode.
\param  pMac
\param  pMsg

\return TRUE - If we consumed the buffer
        FALSE - If have defered the message.
 ---------------------------------------------------------------*/
static tANI_BOOLEAN
__limProcessSmeSwitchChlReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (__limIsDeferedMsgForLearn(pMac, pMsg))
    {
        /**
                * If message defered, buffer is not consumed yet.
                * So return false
                */
        return eANI_BOOLEAN_FALSE;
    }
    __limHandleSmeSwitchChlRequest(pMac, (tANI_U32 *) pMsg->bodyptr);
    return eANI_BOOLEAN_TRUE;
}
#endif


void limProcessSmeGetScanChannelInfo(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMsgQ         mmhMsg;
    tpSmeGetScanChnRsp  pSirSmeRsp;
    tANI_U16 len = 0;

   PELOG2(limLog(pMac, LOG2,
           FL("Sending message %s with number of channels %d\n"),
           limMsgStr(eWNI_SME_GET_SCANNED_CHANNEL_RSP), pMac->lim.scanChnInfo.numChnInfo);)

    len = sizeof(tSmeGetScanChnRsp) + (pMac->lim.scanChnInfo.numChnInfo - 1) * sizeof(tLimScanChn);
    if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pSirSmeRsp, len ))
    {
        /// Buffer not available. Log error
        limLog(pMac, LOGP,
               FL("call to palAllocateMemory failed for JOIN/REASSOC_RSP\n"));

        return;
    }
    palZeroMemory(pMac->hHdd, pSirSmeRsp, len);

#if defined(ANI_PRODUCT_TYPE_AP) && defined(ANI_LITTLE_BYTE_ENDIAN)
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->mesgType, eWNI_SME_GET_SCANNED_CHANNEL_RSP);
    sirStoreU16N((tANI_U8*)&pSirSmeRsp->mesgLen, len);
#else
    pSirSmeRsp->mesgType = eWNI_SME_GET_SCANNED_CHANNEL_RSP;
    pSirSmeRsp->mesgLen = len;
#endif
    if(pMac->lim.scanChnInfo.numChnInfo)
    {
        pSirSmeRsp->numChn = pMac->lim.scanChnInfo.numChnInfo;
        palCopyMemory(pMac->hHdd, pSirSmeRsp->scanChn, pMac->lim.scanChnInfo.scanChn, sizeof(tLimScanChn) * pSirSmeRsp->numChn);
    }
    //Clear the list
    limRessetScanChannelInfo(pMac);

    mmhMsg.type = eWNI_SME_GET_SCANNED_CHANNEL_RSP;
    mmhMsg.bodyptr = pSirSmeRsp;
    mmhMsg.bodyval = 0;
  
    pMac->lim.gLimRspReqd = false;
    MTRACE(macTraceMsgTx(pMac, 0, mmhMsg.type));
    limHalMmhPostMsgApi(pMac, &mmhMsg,  ePROT);
}


/**
 * __limCounterMeasures()
 *
 * FUNCTION:
 * This function is called to "implement" MIC counter measure
 * and is *temporary* only
 *
 * LOGIC: on AP, disassoc all STA associated thru TKIP,
 * we don't do the proper STA disassoc sequence since the
 * BSS will be stoped anyway
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @return None
 */

static void
__limCounterMeasures(tpAniSirGlobal pMac)
{
#ifdef ANI_PRODUCT_TYPE_AP
    tSirMacAddr mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
        limSendDisassocMgmtFrame(pMac,eSIR_MAC_MIC_FAILURE_REASON,mac);
    tx_thread_sleep(10);
#else
    (void) pMac;
#endif
};


static void
__limHandleSmeStopBssRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirSmeStopBssReq  stopBssReq;
    tSirRetStatus      status;
    tLimSmeStates      prevState;
	
#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_STOP_BSS_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    if ((limStopBssReqSerDes(pMac, &stopBssReq, (tANI_U8 *) pMsgBuf) != eSIR_SUCCESS) ||
        !limIsSmeStopBssReqValid(pMsgBuf))
    {
        PELOGW(limLog(pMac, LOGW, FL("received invalid SME_STOP_BSS_REQ message\n"));)
        /// Send Stop BSS response to host
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_INVALID_PARAMETERS);
        return;
    }

    if ((pMac->lim.gLimSmeState != eLIM_SME_NORMAL_STATE) ||
        (pMac->lim.gLimSystemRole == eLIM_STA_ROLE))
    {
        /**
         * Should not have received STOP_BSS_REQ in states
         * other than 'normal' state or on STA in Infrastructure
         * mode. Log error and return response to host.
         */
        limLog(pMac, LOGE,
           FL("received unexpected SME_STOP_BSS_REQ in state %X, for role %d\n"),
           pMac->lim.gLimSmeState, pMac->lim.gLimSystemRole);
        limPrintSmeState(pMac, LOGE, pMac->lim.gLimSmeState);
        /// Send Stop BSS response to host
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_UNEXPECTED_REQ_RESULT_CODE);
        return;
    }

    PELOGW(limLog(pMac, LOGW, FL("RECEIVED STOP_BSS_REQ with reason code=%d\n"), stopBssReq.reasonCode);)

    prevState = pMac->lim.gLimSmeState;

    pMac->lim.gLimSmeState   = eLIM_SME_IDLE_STATE;
    MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

    if (eLIM_STA_IN_IBSS_ROLE != pMac->lim.gLimSystemRole)
    {
        tSirMacAddr   bcAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if ((stopBssReq.reasonCode == eSIR_SME_MIC_COUNTER_MEASURES))
            // Send disassoc all stations associated thru TKIP
            __limCounterMeasures(pMac);
        else
            limSendDisassocMgmtFrame(pMac, eSIR_MAC_DISASSOC_LEAVING_BSS_REASON, bcAddr);
    }

    //limDelBss is also called as part of coalescing, when we send DEL BSS followed by Add Bss msg.
    pMac->lim.gLimIbssCoalescingHappened = false;
    /* send a delBss to HAL and wait for a response */
    status = limDelBss(pMac, NULL, pMac->lim.gLimBssIdx);
    if (status != eSIR_SUCCESS)
    {
        PELOGE(limLog(pMac, LOGE, FL("delBss failed for bss %d\n"), pMac->lim.gLimBssIdx);)
        pMac->lim.gLimSmeState   = prevState;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
        limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_STOP_BSS_FAILURE);
    }
}

/**--------------------------------------------------------------
\fn     __limProcessSmeStopBssReq

\brief  Wrapper for the function __limHandleSmeStopBssRequest
        This message will be defered until softmac come out of
        scan mode. Message should be handled even if we have
        detected radar in the current operating channel.
\param  pMac
\param  pMsg

\return TRUE - If we consumed the buffer
        FALSE - If have defered the message.
 ---------------------------------------------------------------*/
static tANI_BOOLEAN
__limProcessSmeStopBssReq(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    if (__limIsDeferedMsgForLearn(pMac, pMsg))
    {
        /**
         * If message defered, buffer is not consumed yet.
         * So return false
         */
        return eANI_BOOLEAN_FALSE;
    }
    __limHandleSmeStopBssRequest(pMac, (tANI_U32 *) pMsg->bodyptr);
    return eANI_BOOLEAN_TRUE;
} /*** end __limProcessSmeStopBssReq() ***/

void limProcessSmeDelBssRsp(
    tpAniSirGlobal  pMac,
    tANI_U32        body)
{
    (void) body;
    SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
    dphHashTableClassInit(pMac);
    limDeletePreAuthList(pMac);
    limIbssDelete(pMac);
    limSendSmeRsp(pMac, eWNI_SME_STOP_BSS_RSP, eSIR_SME_SUCCESS);
    return;
}

#if 0
/**
 * __limProcessSmePromiscuousReq()
 *
 *FUNCTION:
 * This function is called to process SME_PROMISCUOUS_REQ message
 * from HDD or upper layer application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */

static void
__limProcessSmePromiscuousReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{

    tANI_U32                cfg = sizeof(tSirMacAddr);
    tSirMacAddr        currentBssId;
    tLimMlmDisassocReq *pMlmDisassocReq;
    tSirMacAddr        bcAddr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    PELOG1(limLog(pMac, LOG1,
           FL("received PROMISCUOUS_REQ message\n"));)

    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, currentBssId, &cfg) !=
                                eSIR_SUCCESS)
    {
        /// Could not get BSSID from CFG. Log error.
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
    }

    if ((((pMac->lim.gLimSystemRole == eLIM_STA_ROLE) ||
         (pMac->lim.gLimSystemRole == eLIM_STA_IN_IBSS_ROLE)) &&
         ((pMac->lim.gLimSmeState == eLIM_SME_ASSOCIATED_STATE) ||
          (pMac->lim.gLimSmeState == eLIM_SME_LINK_EST_STATE))) ||
        ((pMac->lim.gLimSystemRole == eLIM_AP_ROLE) &&
         (pMac->lim.gLimSmeState == eLIM_SME_NORMAL_STATE)))
    {
        // Trigger Disassociation frame to peer MAC entity
        if( eHAL_STATUS_SUCCESS != palAllocateMemory( pMac->hHdd, (void **)&pMlmDisassocReq, sizeof(tLimMlmDisassocReq)))
        {
            // Log error
            limLog(pMac, LOGP,
                   FL("call to palAllocateMemory failed for mlmDisassocReq\n"));

            return;
        }

        if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE)
            palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMlmDisassocReq->peerMacAddr,
                          (tANI_U8 *) &bcAddr,
                          sizeof(tSirMacAddr));
        else
            palCopyMemory( pMac->hHdd, pMlmDisassocReq->peerMacAddr,
                          currentBssId,
                          sizeof(tSirMacAddr));

        pMlmDisassocReq->reasonCode      =
                                eSIR_MAC_DISASSOC_LEAVING_BSS_REASON;
        pMlmDisassocReq->disassocTrigger =
                                      eLIM_PROMISCUOUS_MODE_DISASSOC;

        pMac->lim.gLimPrevSmeState = pMac->lim.gLimSmeState;
        pMac->lim.gLimSmeState     = eLIM_SME_WT_DISASSOC_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));

        limPostMlmMessage(pMac,
                          LIM_MLM_DISASSOC_REQ,
                          (tANI_U32 *) pMlmDisassocReq);
    }
    else
    {
        // Send Promiscuous mode response to host
        limSendSmePromiscuousModeRsp(pMac);

        pMac->lim.gLimSmeState = eLIM_SME_OFFLINE_STATE;
	 MTRACE(macTrace(pMac, TRACE_CODE_SME_STATE, 0, pMac->lim.gLimSmeState));
    }

} /*** end __limProcessSmePromiscuousReq() ***/
#endif


#ifdef ANI_PRODUCT_TYPE_AP
/**
 * __limIsSmeAssocCnfValid()
 *
 *FUNCTION:
 * This function is called by limProcessLmmMessages() upon
 * receiving SME_ASSOC_CNF.
 *
 *LOGIC:
 * Message validity checks are performed in this function
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMeasReq  Pointer to Received ASSOC_CNF message
 * @return true      When received SME_ASSOC_CNF is formatted
 *                   correctly
 *         false     otherwise
 */

inline static tANI_U8
__limIsSmeAssocCnfValid(tpSirSmeAssocCnf pAssocCnf)
{
    if (limIsGroupAddr(pAssocCnf->peerMacAddr))
        return false;
    else
        return true;
} /*** end __limIsSmeAssocCnfValid() ***/


/**
 * __limProcessSmeAssocCnf()
 *
 *FUNCTION:
 * This function is called by limProcessSmeMessages() upon
 * receiving SME_ASSOC_CNF/SME_REASSOC_CNF from WSM.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 *
 * @return None
 */

static void
__limProcessSmeAssocCnf(tpAniSirGlobal pMac, tANI_U32 msgType, tANI_U32 *pMsgBuf)
{
    tSirSmeAssocCnf    assocCnf;
    tpDphHashNode      pStaDs;
    tHalBitVal cbBitVal;
    tANI_U8 tspecIdx = 0; //index in the lim tspec table.

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ASSOC_CNF_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    if ((pMac->lim.gLimSystemRole != eLIM_AP_ROLE) ||
        ((pMac->lim.gLimSmeState  != eLIM_SME_NORMAL_STATE) &&
         (pMac->lim.gLimSmeState  !=
                         eLIM_SME_NORMAL_CHANNEL_SCAN_STATE)))
    {
        limLog(pMac, LOGE,
         FL("Received unexpected message %X in state %X, in role %X\n"),
         msgType, pMac->lim.gLimSmeState, pMac->lim.gLimSystemRole);

        return;
    }

   if ((limAssocCnfSerDes(pMac, &assocCnf, (tANI_U8 *) pMsgBuf) ==
                            eSIR_FAILURE) ||
        !__limIsSmeAssocCnfValid(&assocCnf))
    {
        limLog(pMac, LOGW,
           FL("Received re/assocCnf message with invalid parameters\n"));

        return;
    }
    pStaDs = dphGetHashEntry(pMac, assocCnf.aid);


    if (pStaDs == NULL)
    {
        
        PELOG1(limLog(pMac, LOG1,
           FL("Received invalid message %X due to no STA context, for aid %d, peer "),
           msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);)
       
        /*
        ** send a DISASSOC_IND message to WSM to make sure
        ** the state in WSM and LIM is the same
        **/

        limSendSmeDisassocNtf(pMac,
            assocCnf.peerMacAddr,
            eSIR_SME_STA_NOT_ASSOCIATED,
            eLIM_PEER_ENTITY_DISASSOC,
            assocCnf.aid);

        return;
    }
    if ((pStaDs &&
         (( !palEqualMemory( pMac->hHdd,(tANI_U8 *) pStaDs->staAddr,
                     (tANI_U8 *) assocCnf.peerMacAddr,
                     sizeof(tSirMacAddr)) ) ||
          (pStaDs->mlmStaContext.mlmState !=
                          eLIM_MLM_WT_ASSOC_CNF_STATE) ||
          ((pStaDs->mlmStaContext.subType == LIM_ASSOC) &&
           (msgType != eWNI_SME_ASSOC_CNF)) ||
          ((pStaDs->mlmStaContext.subType == LIM_REASSOC) &&
           (msgType != eWNI_SME_REASSOC_CNF)))))
    {
        PELOG1(limLog(pMac, LOG1,
           FL("Received invalid message %X due to peerMacAddr mismatched or not in eLIM_MLM_WT_ASSOC_CNF_STATE state, for aid %d, peer "),
           msgType, assocCnf.aid);
        limPrintMacAddr(pMac, assocCnf.peerMacAddr, LOG1);)

        return;
    }

    /*
    ** Deactivate/delet CNF_WAIT timer since ASSOC_CNF
    ** has been received
    **/

    PELOG1(limLog(pMac, LOG1, FL("Received Cnf. Delete Timer\n"));)
    limDeactivateAndChangePerStaIdTimer(pMac, eLIM_CNF_WAIT_TIMER,
                                        pStaDs->assocId);

    if (assocCnf.statusCode == eSIR_SME_SUCCESS)
    {
        tSirMacScheduleIE schedule;



        // If STA is a TITAN-compatible device, then setup
        // the TITAN proprietary capabilities appropriately
        // in the per STA DS, as per the local TITAN Prop
        // capabilities of this AP

        // STA is allowed to be associated
        if (pStaDs->mlmStaContext.updateContext)
        {
            // Initialize the STA descriptor and TX WQs


          {
              tLimMlmStates mlmPrevState = pStaDs->mlmStaContext.mlmState;
              //we need to set the mlmState here in order differentiate in limDelSta.
              pStaDs->mlmStaContext.mlmState = eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
              if(limDelSta(pMac, pStaDs, true) != eSIR_SUCCESS)
              {
                  limLog(pMac, LOGE,
                         FL("could not DEL STA with assocId=%d staId %d\n"),
                         pStaDs->assocId, pStaDs->staIndex);

                  limRejectAssociation(pMac,
                           pStaDs->staAddr,
                           pStaDs->mlmStaContext.subType,
                           true, pStaDs->mlmStaContext.authType,
                           pStaDs->assocId, true,
                           (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);

                  //Restoring the state back.
                  pStaDs->mlmStaContext.mlmState = mlmPrevState;
                  return;
              }
              return;
            }
        }
        else
        {
            // Add STA context at HW
            if (limAddSta(pMac, pStaDs) != eSIR_SUCCESS)
            {
                limLog(pMac, LOGE,
                       FL("could not Add STA with assocId=%d\n"),
                       pStaDs->assocId);

              // delete the TS if it has already been added.
               // send the response with error status.
                if(pStaDs->qos.addtsPresent)
                {
                  limAdmitControlDeleteTS(pMac, pStaDs->assocId, &pStaDs->qos.addts.tspec.tsinfo, NULL, &tspecIdx);
                }

                limRejectAssociation(pMac,
                         pStaDs->staAddr,
                         pStaDs->mlmStaContext.subType,
                         true, pStaDs->mlmStaContext.authType,
                         pStaDs->assocId, true,
                         (tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS);

            }
            return;
        }

   } // if (assocCnf.statusCode == eSIR_SME_SUCCESS)
    else
    {
        // STA is not allowed to be associated
        limRejectAssociation(pMac, pStaDs->staAddr,
                             pStaDs->mlmStaContext.subType,
                             true, pStaDs->mlmStaContext.authType,
                             pStaDs->assocId, true,
                             assocCnf.statusCode);
    }
} /*** end __limProcessSmeAssocCnf() ***/

#endif

static void
__limProcessSmeAddtsReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpDphHashNode pStaDs;
    tSirMacAddr   peerMac;
    tANI_U32           val;
    tpSirAddtsReq pSirAddts;
    tANI_U32           timeout;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_ADDTS_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    pSirAddts = (tpSirAddtsReq) pMsgBuf;

    /* if sta
     *  - verify assoc state
     *  - send addts request to ap
     *  - wait for addts response from ap
     * if ap, just ignore with error log
     */
    PELOG1(limLog(pMac, LOG1,
           FL("Received SME_ADDTS_REQ (TSid %d, UP %d)\n"),
           pSirAddts->req.tspec.tsinfo.traffic.tsid,
           pSirAddts->req.tspec.tsinfo.traffic.userPrio);)

    if (pMac->lim.gLimSystemRole != eLIM_STA_ROLE)
    {
        PELOGE(limLog(pMac, LOGE, "AddTs received on AP - ignoring\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }

    //Ignore the request if STA is in 11B mode.
    if(pMac->lim.gLimDot11Mode == WNI_CFG_DOT11_MODE_11B)
    {
        PELOGE(limLog(pMac, LOGE, "AddTS received while Dot11Mode is 11B - ignoring\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }


    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);

    if (pStaDs == NULL)
    {
        PELOGE(limLog(pMac, LOGE, "Cannot find AP context for addts req\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }

    if ((! pStaDs->valid) ||
        (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE))
    {
        PELOGE(limLog(pMac, LOGE, "AddTs received in invalid MLM state\n");)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }

    pSirAddts->req.wsmTspecPresent = 0;
    pSirAddts->req.wmeTspecPresent = 0;
    pSirAddts->req.lleTspecPresent = 0;

    if ((pStaDs->wsmEnabled) &&
        (pSirAddts->req.tspec.tsinfo.traffic.accessPolicy != SIR_MAC_ACCESSPOLICY_EDCA))
        pSirAddts->req.wsmTspecPresent = 1;
    else if (pStaDs->wmeEnabled)
        pSirAddts->req.wmeTspecPresent = 1;
    else if (pStaDs->lleEnabled)
        pSirAddts->req.lleTspecPresent = 1;
    else
    {
        PELOGW(limLog(pMac, LOGW, FL("ADDTS_REQ ignore - qos is disabled\n"));)
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }


    // for edca, if no Access Control, ignore the request
    if ((pSirAddts->req.tspec.tsinfo.traffic.accessPolicy == SIR_MAC_ACCESSPOLICY_EDCA) &&
        (! pMac->sch.schObject.gSchEdcaParams[upToAc(pSirAddts->req.tspec.tsinfo.traffic.userPrio)].aci.acm))
    {
        limLog(pMac, LOGW, FL("AddTs with UP %d AC %d has no ACM - ignoring request\n"),
               pSirAddts->req.tspec.tsinfo.traffic.userPrio, upToAc(pSirAddts->req.tspec.tsinfo.traffic.userPrio));
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_SUCCESS, pSirAddts->req.tspec);
        return;
    }

    if ((pMac->lim.gLimSmeState != eLIM_SME_ASSOCIATED_STATE) &&
        (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE))
    {
        limLog(pMac, LOGE, "AddTs received in invalid LIMsme state (%d)\n",
              pMac->lim.gLimSmeState);
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }

    if (pMac->lim.gLimAddtsSent)
    {
        limLog(pMac, LOGE, "Addts (token %d, tsid %d, up %d) is still pending\n",
               pMac->lim.gLimAddtsReq.req.dialogToken,
               pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.tsid,
               pMac->lim.gLimAddtsReq.req.tspec.tsinfo.traffic.userPrio);
        limSendSmeAddtsRsp(pMac, pSirAddts->rspReqd, eSIR_FAILURE, pSirAddts->req.tspec);
        return;
    }

    val = sizeof(tSirMacAddr);
    if (wlan_cfgGetStr(pMac, WNI_CFG_BSSID, peerMac, &val) != eSIR_SUCCESS)
    {
        /// Could not get BSSID from CFG. Log error.
        limLog(pMac, LOGP, FL("could not retrieve BSSID\n"));
        return;
    }

    // save the addts request
    pMac->lim.gLimAddtsSent = true;
    palCopyMemory( pMac->hHdd, (tANI_U8 *) &pMac->lim.gLimAddtsReq, (tANI_U8 *) pSirAddts, sizeof(tSirAddtsReq));

    // ship out the message now
    limSendAddtsReqActionFrame(pMac, peerMac, &pSirAddts->req);
    PELOG1(limLog(pMac, LOG1, "Sent ADDTS request\n");)

    // start a timer to wait for the response
    if (pSirAddts->timeout) 
        timeout = pSirAddts->timeout;
    else if (wlan_cfgGetInt(pMac, WNI_CFG_ADDTS_RSP_TIMEOUT, &timeout) != eSIR_SUCCESS)
    {
        limLog(pMac, LOGP, FL("Unable to get Cfg param %d (Addts Rsp Timeout)\n"),
               WNI_CFG_ADDTS_RSP_TIMEOUT);
        return;
    }

    timeout = SYS_MS_TO_TICKS(timeout);
	if (tx_timer_change(&pMac->lim.limTimers.gLimAddtsRspTimer, timeout, 0) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer change failed!\n"));
        return;
    }
    pMac->lim.gLimAddtsRspTimerCount++;
    if (tx_timer_change_context(&pMac->lim.limTimers.gLimAddtsRspTimer,
                                pMac->lim.gLimAddtsRspTimerCount) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer change failed!\n"));
        return;
    }
    MTRACE(macTrace(pMac, TRACE_CODE_TIMER_ACTIVATE, 0, eLIM_ADDTS_RSP_TIMER));
    if (tx_timer_activate(&pMac->lim.limTimers.gLimAddtsRspTimer) != TX_SUCCESS)
    {
        limLog(pMac, LOGP, FL("AddtsRsp timer activation failed!\n"));
        return;
    }
    return;
}

static void
__limProcessSmeDeltsReq(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tSirMacAddr peerMacAddr;
    tANI_U8  ac;
    tSirMacTSInfo *pTsinfo;
    tpSirDeltsReq pDeltsReq = (tpSirDeltsReq) pMsgBuf;
    tpDphHashNode    pStaDs = NULL;

#ifdef FEATURE_WLAN_DIAG_SUPPORT 
    limDiagEventReport(pMac, WLAN_PE_DIAG_DELTS_REQ_EVENT, NULL, 0, 0);
#endif //FEATURE_WLAN_DIAG_SUPPORT

    //  - send delts request to ap

    //Ignore the request if STA is in 11B mode.
    if(pMac->lim.gLimDot11Mode == WNI_CFG_DOT11_MODE_11B)
    {
        PELOGE(limLog(pMac, LOGE, "DelTS received while Dot11Mode is 11B - ignoring\n");)
        limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_FAILURE);
        return;
    }

    if (eSIR_SUCCESS != limValidateDeltsReq(pMac, pDeltsReq, peerMacAddr))
    {
      PELOG1(limLog(pMac, LOG1, FL("limValidateDeltsReq failed\n"));)
      limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_FAILURE);
      return;
    }

    PELOG1(limLog(pMac, LOG1, FL("Sent DELTS request to station with assocId = %d MacAddr = %x:%x:%x:%x:%x:%x\n"),
              pDeltsReq->aid, peerMacAddr[0], peerMacAddr[1], peerMacAddr[2],
              peerMacAddr[3], peerMacAddr[4], peerMacAddr[5]);)

    limSendDeltsReqActionFrame(pMac, peerMacAddr, pDeltsReq->req.wmeTspecPresent, &pDeltsReq->req.tsinfo, &pDeltsReq->req.tspec);

    pTsinfo = pDeltsReq->req.wmeTspecPresent ? &pDeltsReq->req.tspec.tsinfo : &pDeltsReq->req.tsinfo;

    /* We've successfully send DELTS frame to AP. Update the 
     * dynamic UAPSD mask. The AC for this TSPEC to be deleted
     * is no longer trigger enabled or delivery enabled
     */
    limSetTspecUapsdMask(pMac, pTsinfo, CLEAR_UAPSD_MASK);

    /* We're deleting the TSPEC, so this particular AC is no longer
     * admitted.  PE needs to downgrade the EDCA
     * parameters(for the AC for which TS is being deleted) to the
     * next best AC for which ACM is not enabled, and send the
     * updated values to HAL. 
     */ 
    ac = upToAc(pTsinfo->traffic.userPrio);
    pMac->lim.gAcAdmitMask &= ~(1 << ac);	
    limSetActiveEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParams);

    pStaDs = dphGetHashEntry(pMac, DPH_STA_HASH_INDEX_PEER);
    if (pStaDs != NULL)
    {
        if (pStaDs->aniPeer == eANI_BOOLEAN_TRUE) 
            limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_TRUE);
        else
            limSendEdcaParams(pMac, pMac->sch.schObject.gSchEdcaParamsActive, pStaDs->bssId, eANI_BOOLEAN_FALSE);
    }
    else
    {
        limLog(pMac, LOGE, FL("Self entry missing in Hash Table \n"));
        limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_FAILURE);
    }     

    // send an sme response back
    limSendSmeDeltsRsp(pMac, pDeltsReq, eSIR_SUCCESS);
}

void
limProcessSmeAddtsRspTimeout(tpAniSirGlobal pMac, tANI_U32 param)
{
    if (pMac->lim.gLimSystemRole != eLIM_STA_ROLE)
    {
        limLog(pMac, LOGW, FL("AddtsRspTimeout in non-Sta role (%d)\n"),
               pMac->lim.gLimSystemRole);
        pMac->lim.gLimAddtsSent = false;
        return;
    }

    if ((pMac->lim.gLimSmeState != eLIM_SME_ASSOCIATED_STATE) &&
        (pMac->lim.gLimSmeState != eLIM_SME_LINK_EST_STATE))
    {
        limLog(pMac, LOGW, FL("AddtsRspTimeout in invalid SmeState %d\n"),
               pMac->lim.gLimSmeState);
        pMac->lim.gLimAddtsSent = false;
        return;
    }

    if (! pMac->lim.gLimAddtsSent)
    {
        PELOGW(limLog(pMac, LOGW, "AddtsRspTimeout but no AddtsSent\n");)
        return;
    }

    if (param != pMac->lim.gLimAddtsRspTimerCount)
    {
        limLog(pMac, LOGE, FL("Invalid AddtsRsp Timer count %d (exp %d)\n"),
               param, pMac->lim.gLimAddtsRspTimerCount);
        return;
    }

    // this a real response timeout
    pMac->lim.gLimAddtsSent = false;
    pMac->lim.gLimAddtsRspTimerCount++;

    limSendSmeAddtsRsp(pMac, true, eSIR_SME_ADDTS_RSP_TIMEOUT, pMac->lim.gLimAddtsReq.req.tspec);
}


/**
 * __limProcessSmeStatsRequest()
 *
 *FUNCTION:
 * 
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */
static void
__limProcessSmeStatsRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpAniGetStatsReq    pStatsReq;
    tSirMsgQ msgQ;

    pStatsReq = (tpAniGetStatsReq) pMsgBuf;

    switch(pStatsReq->msgType)
    {
        //Add Lim stats here. and send reqsponse.

        //HAL maintained Stats.
        case eWNI_SME_STA_STAT_REQ:
            msgQ.type = SIR_HAL_STA_STAT_REQ;
            break;
        case eWNI_SME_AGGR_STAT_REQ:
            msgQ.type = SIR_HAL_AGGR_STAT_REQ;
            break;
        case eWNI_SME_GLOBAL_STAT_REQ:
            msgQ.type = SIR_HAL_GLOBAL_STAT_REQ;
            break;
        case eWNI_SME_STAT_SUMM_REQ:
            msgQ.type = SIR_HAL_STAT_SUMM_REQ;
            break;   
        default: //Unknown request.
            PELOGE(limLog(pMac, LOGE, "Unknown Statistics request\n");)
            palFreeMemory( pMac, pMsgBuf );
            return;
    }

    if ( !pMac->lim.gLimRspReqd ) 
    {
        palFreeMemory( pMac, pMsgBuf );
        return;
    }
    else
    {
        pMac->lim.gLimRspReqd = FALSE;
    }

    msgQ.reserved = 0;
    msgQ.bodyptr = pMsgBuf;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));

    if( eSIR_SUCCESS != (halPostMsgApi( pMac, &msgQ ))){
        limLog(pMac, LOGP, "Unable to forward request\n");
        return;
    }

    return;
}

/**
 * __limProcessSmeGetStatisticsRequest()
 *
 *FUNCTION:
 * 
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return None
 */
static void
__limProcessSmeGetStatisticsRequest(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tpAniGetPEStatsReq    pPEStatsReq;
    tSirMsgQ msgQ;

    pPEStatsReq = (tpAniGetPEStatsReq) pMsgBuf;
    
    //pPEStatsReq->msgType should be eWNI_SME_GET_STATISTICS_REQ

    msgQ.type = SIR_HAL_GET_STATISTICS_REQ;    

    if ( !pMac->lim.gLimRspReqd ) 
    {
        palFreeMemory( pMac, pMsgBuf );
        return;
    }
    else
    {
        pMac->lim.gLimRspReqd = FALSE;
    }

    msgQ.reserved = 0;
    msgQ.bodyptr = pMsgBuf;
    msgQ.bodyval = 0;
    MTRACE(macTraceMsgTx(pMac, 0, msgQ.type));

    if( eSIR_SUCCESS != (halPostMsgApi( pMac, &msgQ ))){
        palFreeMemory( pMac, pMsgBuf );
        limLog(pMac, LOGP, "Unable to forward request\n");
        return;
    }

    return;
}


/** -------------------------------------------------------------
\fn limProcessSmeDelBaPeerInd
\brief handles indication message from HDD to send delete BA request
\param   tpAniSirGlobal pMac
\param   tANI_U32 pMsgBuf
\return None
-------------------------------------------------------------*/
void
limProcessSmeDelBaPeerInd(tpAniSirGlobal pMac, tANI_U32 *pMsgBuf)
{
    tANI_U16 assocId =0;
    tpSmeDelBAPeerInd pSmeDelBAPeerInd = (tpSmeDelBAPeerInd)pMsgBuf;
    tpDphHashNode pSta;

    if(NULL == pSmeDelBAPeerInd)
        return;

    limLog(pMac, LOGW, FL("called with staId = %d, tid = %d, baDirection = %d\n"), 
              pSmeDelBAPeerInd->staIdx, pSmeDelBAPeerInd->baTID, pSmeDelBAPeerInd->baDirection);

    pSta = dphLookupAssocId(pMac, pSmeDelBAPeerInd->staIdx, &assocId);    
    if( eSIR_SUCCESS != limPostMlmDelBAReq( pMac,
          pSta,
          pSmeDelBAPeerInd->baDirection,
          pSmeDelBAPeerInd->baTID,
          eSIR_MAC_UNSPEC_FAILURE_REASON))
    {
      limLog( pMac, LOGW,
          FL( "Failed to post LIM_MLM_DELBA_REQ to " ));
      if (pSta)
          limPrintMacAddr(pMac, pSta->staAddr, LOGW); 
    }
}


/**
 * limProcessSmeReqMessages()
 *
 *FUNCTION:
 * This function is called by limProcessMessageQueue(). This
 * function processes SME request messages from HDD or upper layer
 * application.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  msgType   Indicates the SME message type
 * @param  *pMsgBuf  A pointer to the SME message buffer
 * @return Boolean - TRUE - if pMsgBuf is consumed and can be freed.
 *                   FALSE - if pMsgBuf is not to be freed.
 */

tANI_BOOLEAN
limProcessSmeReqMessages(tpAniSirGlobal pMac, tpSirMsgQ pMsg)
{
    tANI_BOOLEAN bufConsumed = TRUE; //Set this flag to false within case block of any following message, that doesnt want pMsgBuf to be freed.
    tANI_U32 *pMsgBuf = pMsg->bodyptr;

    PELOG1(limLog(pMac, LOG1, FL("LIM Received SME Message %s(%d) LimSmeState:%s(%d) LimMlmState: %s(%d)\n"),
         limMsgStr(pMsg->type), pMsg->type,
         limSmeStateStr(pMac->lim.gLimSmeState), pMac->lim.gLimSmeState,
         limMlmStateStr(pMac->lim.gLimMlmState), pMac->lim.gLimMlmState );)

    switch (pMsg->type)
    {
        case eWNI_SME_START_REQ:
            __limProcessSmeStartReq(pMac, pMsgBuf);
            break;

        case eWNI_SME_SYS_READY_IND:
            bufConsumed = __limProcessSmeSysReadyInd(pMac, pMsgBuf);
            break;


        case eWNI_SME_START_BSS_REQ:
            bufConsumed = __limProcessSmeStartBssReq(pMac, pMsg);
            break;

        case eWNI_SME_SCAN_REQ:
            __limProcessSmeScanReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_JOIN_REQ:
            __limProcessSmeJoinReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_AUTH_REQ:
           // __limProcessSmeAuthReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_REASSOC_REQ:
            __limProcessSmeReassocReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_PROMISCUOUS_MODE_REQ:
            //__limProcessSmePromiscuousReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_DISASSOC_REQ:
            __limProcessSmeDisassocReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_DISASSOC_CNF:
        case eWNI_SME_DEAUTH_CNF:
            __limProcessSmeDisassocCnf(pMac, pMsgBuf);

            break;

        case eWNI_SME_DEAUTH_REQ:
            __limProcessSmeDeauthReq(pMac, pMsgBuf);

            break;

#if (WNI_POLARIS_FW_PACKAGE == ADVANCED) && defined(ANI_PRODUCT_TYPE_AP)
        case eWNI_SME_SWITCH_CHL_REQ:
        case eWNI_SME_SWITCH_CHL_CB_PRIMARY_REQ:
        case eWNI_SME_SWITCH_CHL_CB_SECONDARY_REQ:
            bufConsumed = __limProcessSmeSwitchChlReq(pMac, pMsg);
            break;
#endif


        case eWNI_SME_SETCONTEXT_REQ:
            __limProcessSmeSetContextReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_REMOVEKEY_REQ:
            __limProcessSmeRemoveKeyReq(pMac, pMsgBuf);

            break;

        case eWNI_SME_STOP_BSS_REQ:
            bufConsumed = __limProcessSmeStopBssReq(pMac, pMsg);
            break;

#ifdef ANI_PRODUCT_TYPE_AP
        case eWNI_SME_ASSOC_CNF:
        case eWNI_SME_REASSOC_CNF:
            if (pMsg->type == eWNI_SME_ASSOC_CNF)
                PELOG1(limLog(pMac, LOG1, FL("Received ASSOC_CNF message\n"));)
            else
                PELOG1(limLog(pMac, LOG1, FL("Received REASSOC_CNF message\n"));)

            __limProcessSmeAssocCnf(pMac, pMsg->type, pMsgBuf);

            break;
#endif

        case eWNI_SME_ADDTS_REQ:
            PELOG1(limLog(pMac, LOG1, FL("Received ADDTS_REQ message\n"));)
            __limProcessSmeAddtsReq(pMac, pMsgBuf);
            break;

        case eWNI_SME_DELTS_REQ:
            PELOG1(limLog(pMac, LOG1, FL("Received DELTS_REQ message\n"));)
            __limProcessSmeDeltsReq(pMac, pMsgBuf);
            break;

        case SIR_LIM_ADDTS_RSP_TIMEOUT:
            PELOG1(limLog(pMac, LOG1, FL("Received SIR_LIM_ADDTS_RSP_TIMEOUT message \n"));)
            limProcessSmeAddtsRspTimeout(pMac, pMsg->bodyval);
            break;

        case eWNI_SME_STA_STAT_REQ:
        case eWNI_SME_AGGR_STAT_REQ:
        case eWNI_SME_GLOBAL_STAT_REQ:
        case eWNI_SME_STAT_SUMM_REQ:
            __limProcessSmeStatsRequest( pMac, pMsgBuf);
            //HAL consumes pMsgBuf. It will be freed there. Set bufConsumed to false.
            bufConsumed = FALSE;
            break;
        case eWNI_SME_GET_STATISTICS_REQ:
            __limProcessSmeGetStatisticsRequest( pMac, pMsgBuf);
            //HAL consumes pMsgBuf. It will be freed there. Set bufConsumed to false.
            bufConsumed = FALSE;
            break;              
        case eWNI_SME_DEL_BA_PEER_IND:
            limProcessSmeDelBaPeerInd(pMac, pMsgBuf);
            break;
        case eWNI_SME_GET_SCANNED_CHANNEL_REQ:
            limProcessSmeGetScanChannelInfo(pMac, pMsgBuf);
            break;
        case eWNI_SME_DEFINE_QOS_REQ:
        case eWNI_SME_DELETE_QOS_REQ:
        default:
            vos_mem_free((v_VOID_t*)pMsg->bodyptr);
            pMsg->bodyptr = NULL;
            break;
    } // switch (msgType)

    return bufConsumed;
} /*** end limProcessSmeReqMessages() ***/
