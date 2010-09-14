/*
 * =====================================================================================
 *
 *       Filename:  halMacBA.c
 *
 *    Description:  API's and Interfaces to support A-MPDU/BA functionality
 *
 *        Version:  1.0
 *        Created:  11/16/2006 05:11:30 PM PST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ashok Ranganath
 *        Company:  Airgo Networks, Inc.
 *
 * =====================================================================================
 */

#include "halTypes.h"
#include "halCommonApi.h"

#include "sirApi.h"
#include "sirParams.h"
#include "halDebug.h"
#include "cfgApi.h"
#include "halMacBA.h"
#include "halTimer.h"
#include "halTLApi.h"
#include "vos_types.h"
#include "vos_memory.h"

#define HAL_BA_TX_FRM_THRESHOLD 15 //Threshold (number of frames transmitted) for setting BA session


/**
 * \brief This API is called to initialize BA-related
 * global parameters. This is only invoked during HAL
 * startup.
 *
 * \sa baInit
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \return none
 *
 */
void baInit( tpAniSirGlobal pMac )
{
    // Global counter keeping track of active BA sessions
    pMac->hal.halMac.baNumActiveSessions = 0;

    // Read and cache (if necessary) the concerned CFG's
    baHandleCFG( pMac, ANI_IGNORE_CFG_ID );
}
/**
 * \brief This API is called to update BA related CFG's,
 * if and when they change.
 * This function is called by two interfaces -
 * 1) halProcessMsg - This is when HAL is notified RE:
 * any CFG change that concerns HAL
 * 2) halProcessStartEvent - When the HAL thread gets to
 * initialize itself and run
 * ASSUMPTIONS:
 * If this API is invoked with
 *   cfgId == ANI_IGNORE_CFG_ID
 * Then,
 *   this routine will traverse thru' ALL the BA
 *   related CFG's that are statically setup
 * Else,
 *   only update this "1" CFG identified by cfgId
 *
 * \sa baHandleCFG
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param cfgId ID of CFG parameter that got updated
 *
 * \return none
 *
 */
void baHandleCFG( tpAniSirGlobal pMac, tANI_U32 cfgId )
{
    tANI_U32 cfg, val, i = 0;
    tANI_U32 baActivityCheckCfgVal =0;
    tANI_U32 defaultCfgList[] = {
        WNI_CFG_BA_TIMEOUT,
        WNI_CFG_MAX_BA_BUFFERS,
        WNI_CFG_MAX_BA_SESSIONS,
        WNI_CFG_BA_THRESHOLD_HIGH,
        ANI_IGNORE_CFG_ID,
        WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT,
        WNI_CFG_BA_AUTO_SETUP
    };

    do
    {
        //
        // Determine if we have to use our own default CFG list
        // OR should we use the argument passed to us
        //
        if( ANI_IGNORE_CFG_ID == cfgId )
            cfg = defaultCfgList[i]; // "n" iterations reqd
        else
            cfg = cfgId; // Just "1" iteration reqd

        switch( cfg )
        {
            case WNI_CFG_BA_AUTO_SETUP:
                {
                    if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                    {
                        HALLOGP( halLog( pMac, LOGP,
                                    FL("Failed to Get CFG ID %d\n"), cfg ));
                        return;
                    }

                    // NOTE: Instead of reading selfSta capability get the System's HT capability
                    // from some CFG
                    //if not already setup, config is enabled and we are htCapable, 
                    //then we need to start timer.
                    if((false == pMac->hal.halMac.baAutoSetupEnabled) && (true == val)) 
                    {
                        pMac->hal.halMac.baAutoSetupEnabled = true;
                        if(tx_timer_activate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS)
                        {
                            // Could not start BA activity check timer.
                            // Log error
                            HALLOGP( halLog(pMac, LOGP, FL("Unable to activate BA activity check timer\n")));
                            return;
                        }
                    }
                    //if already setup and config is disabled, then we need to stop timer
                    //and delete all the existing BA sessions.
                    else if((true == pMac->hal.halMac.baAutoSetupEnabled) && (false == val))
                    {
                        pMac->hal.halMac.baAutoSetupEnabled = false;
                        halDeactivateAndChangeTimer(pMac, eHAL_BA_ACT_CHK_TIMER);
                        //send indication to LIM to delete all the BA sessions.
                        (pMac->hal.pPECallBack)(pMac, SIR_LIM_DEL_BA_ALL_IND, NULL);
                    }
                    else
                    {
                        HALLOGW( halLog(pMac, LOGW, FL("can't change BaActivityCheck timer, \
                                        CFG BA_AUTO_SETUP = %d, baAutoSetup already enabled = %d"),
                                    val, pMac->hal.halMac.baAutoSetupEnabled));
                    }
                }
                break;
            case WNI_CFG_BA_TIMEOUT:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                    return;
                }
                pMac->hal.halMac.baTimeout = (tANI_U16) val;
                break;

            case WNI_CFG_MAX_BA_BUFFERS:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"), cfg ));
                    return;
                }
                pMac->hal.halMac.baRxMaxAvailBuffers = (tANI_U16) val;
                break;

            case WNI_CFG_MAX_BA_SESSIONS:
                // Not cached for the time being...
#if 0
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                else
                    val *= sizeof( tRxBASessionTable ); // Total size

                // Allocate memory for the BA Session Entries
                if( eHAL_STATUS_SUCCESS == palAllocateMemory( pMac->hHdd,
                            (void **) &pMac->hal.halMac.baSessionTable,
                            val ))
                    palZeroMemory( pMac->hHdd,
                            (void *) pMac->hal.halMac.baSessionTable,
                            val );
                else
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to allocate memory [%d bytes] for the BA Session table!\n"),
                                val ));
#endif //#if 0
                break;

            case WNI_CFG_BA_THRESHOLD_HIGH:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                    return;
                }
                pMac->hal.halMac.baSetupThresholdHigh = val;
                break;

            case WNI_CFG_BA_ACTIVITY_CHECK_TIMEOUT:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                    return;
                }
                if (tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr)
                        != TX_SUCCESS)
                {
                    // Could not deactivate BA activitycheck timer.
                    // Log error
                    HALLOGP( halLog(pMac, LOGP,
                                FL("Unable to deactivate BA activity check timer\n")));
                    return;
                }

                baActivityCheckCfgVal = val;

                val = SYS_MS_TO_TICKS(val);

                if (tx_timer_change(&pMac->hal.halMac.baActivityChkTmr,
                            val, val) != TX_SUCCESS)
                {
                    // Could not change BA activity check timer.
                    // Log error
                    HALLOGP( halLog(pMac, LOGP, FL("Unable to change BA activity check timer\n")));
                    return;
                }

                if(tx_timer_activate(&pMac->hal.halMac.baActivityChkTmr)
                        != TX_SUCCESS)
                {
                    // Could not activate BA activity check timer.
                    // Log error
                    HALLOGP( halLog(pMac, LOGP, FL("Unable to activate BA activity check timer\n")));
                    return;
                }

                break;

            case WNI_CFG_MAX_MEDIUM_TIME:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                    return;
                }
                halTpe_SetAmpduTxTime(pMac, val);
                break;

            case WNI_CFG_MAX_MPDUS_IN_AMPDU:
                if( eSIR_SUCCESS != wlan_cfgGetInt( pMac, (tANI_U16) cfg, &val ))
                {
                    HALLOGP( halLog( pMac, LOGP,
                                FL("Failed to Get CFG ID %d\n"),
                                cfg ));
                    return;
                }
                halTpe_UpdateMaxMpduInAmpdu(pMac, val);
                break;


            default:
                break;
        }

        // If only "1" CFG needs an update, then return
        if( ANI_IGNORE_CFG_ID == cfgId )
            i++;
        else
            break;

    } while( ANI_IGNORE_CFG_ID != defaultCfgList[i] ); // End-Of-List?

    // DEBUG LOG the BA CFG's
    HALLOGW( halLog( pMac, LOGW,
                FL("The BA related global CFG's are: "
                    "BA Timeout - %d, "
                    "Max BA Buffers available - %d, "
                    "BA Activity check timeout - %d, "
                    "BA High Threshold - %d,\n"),
                pMac->hal.halMac.baTimeout,
                pMac->hal.halMac.baRxMaxAvailBuffers,
                baActivityCheckCfgVal,
                pMac->hal.halMac.baSetupThresholdHigh ));

}

/**
 * \brief Determines the amount of buffers available for
 * an A-MPDU/BA session and allocates a set from the
 * available pool. This set is then returned to the caller
 *
 * \sa baAllocateBuffer
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baBufferSize
 *        [IN] The buffer size requested by
 * the ADDBA initiator. If this size is 0, then the ADDBA
 * recipient should allocate and send a non-zero buffer
 * size back to the initiator
 *
 *        [OUT] The actual buffer size allocated by the
 * ADDBA recipient. This is based on the number of
 * available buffers, and not exceeding the total number
 * of available buffers on the host
 *
 * \return status Whether the request for BA buffers was
 * granted or not
 *
 */
eHalStatus baAllocateBuffer( tpAniSirGlobal pMac,
    tANI_U16 *baBufferSize )
{
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    tANI_U16 bufSize;

    if( NULL != baBufferSize )
    {
        if( 0 == pMac->hal.halMac.baRxMaxAvailBuffers )
        {
            HALLOGE( halLog( pMac, LOGE,
                        FL("No more buffers available to setup a BA session!\n")));

            //
            // FIXME_AMPDU - Try to reclaim any idle BA session that is
            // currently marked as valid
            //
            retStatus = eHAL_STATUS_BA_RX_BUFFERS_FULL;
            goto returnFailure;
        }

        if( 0 == *baBufferSize )
        {
            // Buffer Size requested is 0. This means that
            // the ADDBA recipient has to allocate and return
            // the desired buffer size
            bufSize = BA_DEFAULT_RX_BUFFER_SIZE;
        }
        else if( *baBufferSize > BA_DEFAULT_RX_BUFFER_SIZE )
            bufSize = BA_DEFAULT_RX_BUFFER_SIZE;
        else
            bufSize = *baBufferSize;

        if((pMac->hal.halMac.baRxMaxAvailBuffers - bufSize) > 0 )
        {
            *baBufferSize = bufSize;
            pMac->hal.halMac.baRxMaxAvailBuffers -= bufSize;
        }
        else
        {
            *baBufferSize = pMac->hal.halMac.baRxMaxAvailBuffers;
            pMac->hal.halMac.baRxMaxAvailBuffers = 0;
        }

        HALLOGW( halLog( pMac, LOGW,
                    FL("Allocated Buffer Size - [%d]. Available Buffer Size - [%d]\n"),
                    *baBufferSize,
                    pMac->hal.halMac.baRxMaxAvailBuffers ));
    }

returnFailure:
    return retStatus;
}

/**
 * \brief Once an existing A-MPDU/BA session is deleted,
 * in the case where this host is an ADDBA recipient,
 * the amount of buffers that were allocated for the
 * deleted BA session will be reclaimed by this API
 *
 * \sa baReleaseBuffer
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baBufferSize The buffer size that is restored
 * back to the BA buffer size pool
 *
 * \return none
 *
 */
void baReleaseBuffer( tpAniSirGlobal pMac,
    tANI_U16 baBufferSize )
{
    pMac->hal.halMac.baRxMaxAvailBuffers += baBufferSize;

    HALLOGW( halLog( pMac, LOGW,
                FL("Restored Buffer Size - %d. Available Buffer Size - [%d]\n"),
                baBufferSize,
                pMac->hal.halMac.baRxMaxAvailBuffers ));
}

/**
 * \brief A new BA Session ID is being setup. Allocate
 * a new session ID from the pool, for the specified
 * STA index and TID
 *
 * \sa baAllocateSessionID
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baStaIndex The STA index for which a new BA
 * session is being setup
 *
 * \param baTID The TID for which the BA session is
 * being setup
 *
 * \param baSessionID [OUT] The newly allocated BA Session ID
 *
 * \return retStatus Indicates whether we were able to
 * successfully allocate a new session ID or not
 *
 */
eHalStatus baAllocateSessionID( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex,
    tANI_U8 baTID,
    tANI_U16 *baSessionID )
{
    tANI_U8 i, found = 0;
    tANI_U32 maxBASessions = 0;
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    tpRxBASessionTable pBASession = pMac->hal.halMac.baSessionTable;

    // Determine the MAX allowed BA sessions
    if( eSIR_SUCCESS != wlan_cfgGetInt( pMac,
                WNI_CFG_MAX_BA_SESSIONS,
                &maxBASessions ))
        return eHAL_STATUS_FAILURE;
    else{
        if (maxBASessions > BA_MAX_SESSIONS)
            maxBASessions = BA_MAX_SESSIONS;
        HALLOGW( halLog( pMac, LOGW,
                    FL("Max BA Sessions - %d\n"),
                    maxBASessions ));
    }
    for( i = 0; i < maxBASessions; i++, pBASession++ )
    {
        if( 0 == pBASession->baValid )
        {
            found = 1;
            break;
        }
    }

    if( found )
    {
        // Paranoia...
        if( NULL != baSessionID )
        {
            // Update the BA Session parameters
            pBASession->baValid = 1;
            pBASession->baTID = baTID;
            pBASession->baStaIndex = baStaIndex;
            *baSessionID = i;

            // Update the HAL global counter with the latest
            // BA session count
            pMac->hal.halMac.baNumActiveSessions++;

            HALLOGW( halLog( pMac, LOGW,
                        FL("New BA Session [%d] - STA Index %d, TID %d. Total [%d]\n"),
                        *baSessionID,
                        baStaIndex,
                        baTID,
                        pMac->hal.halMac.baNumActiveSessions ));
        }
    }
    else
    {
        HALLOGW(  halLog( pMac, LOGW,
                    FL("Max BA Sessions reached!\n")));
        retStatus = eHAL_STATUS_BA_RX_MAX_SESSIONS_REACHED;
    }

    return retStatus;
}

/**
 * \brief Release the given BA session ID after the
 * coresponding BA session has been deleted
 *
 * \sa baReleaseSessionID
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baStaIndex The STA index for which the BA
 * session is being reclaimed
 *
 * \param baTID The TID for which the BA session was setup
 *
 * \return retStatus Indicates whether we were able to
 * successfully release the session ID or not
 *
 */
eHalStatus baReleaseSessionID( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex,
    tANI_U8 baTID )
{
    tANI_U8 i, found = 0;
    tANI_U32 maxBASessions = 0;
    eHalStatus retStatus = eHAL_STATUS_SUCCESS;
    tpRxBASessionTable pBASession = pMac->hal.halMac.baSessionTable;

    // Determine the MAX allowed BA sessions
    if( eSIR_SUCCESS != wlan_cfgGetInt( pMac,
                WNI_CFG_MAX_BA_SESSIONS,
                &maxBASessions ))
        return eHAL_STATUS_FAILURE;
    if (maxBASessions > BA_MAX_SESSIONS)
        maxBASessions = BA_MAX_SESSIONS;

    for( i = 0; i < maxBASessions; i++, pBASession++ )
    {
        if(( 1 == pBASession->baValid ) &&
                ( baStaIndex == pBASession->baStaIndex ) &&
                ( baTID == pBASession->baTID ))
        {
            found = 1;
            break;
        }
    }

    if( found )
    {

        // Notify HDD about this deletion...
        // TODO - Should we watch out for the return status?

        baDelNotifyTL( pMac, i );// Session ID

        // Update the BA Session parameters
        pBASession->baValid = 0;
        pBASession->baTID = 0;
        pBASession->baStaIndex = 0;

        // Update the HAL global counter with the latest
        // BA session count
        pMac->hal.halMac.baNumActiveSessions--;

        HALLOGW(  halLog( pMac, LOGW,
                    FL("Reclaimed BA Session %d. Total - [%d]\n"),
                    i,
                    pMac->hal.halMac.baNumActiveSessions ));

    }
    else
    {
        HALLOGW( halLog( pMac, LOGW,
                    FL("A valid BA Session ID not found for STA Index %d, TID %d!\n"),
                    baStaIndex,
                    baTID ));
        retStatus = eHAL_STATUS_BA_RX_INVALID_SESSION_ID;
    }

    return retStatus;
}

/**
 * \brief Given a HAL STA index, release ALL the resources
 * allocated for this STA related to A-MPDU/BA
 *
 * \sa baReleaseSTA
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baStaIndex The STA index for which all the BA
 * session(s) are(is) being reclaimed
 *
 * \return retStatus Indicates whether we were able to
 * successfully release the session ID or not
 *
 */
eHalStatus baReleaseSTA( tpAniSirGlobal pMac,
    tANI_U16 baStaIndex )
{
    tANI_U8 i;
    tpStaStruct pSta = &((tpStaStruct) pMac->hal.halMac.staTable)[baStaIndex];

    for( i = 0; i < STACFG_MAX_TC; i++ )
    {
        // If this TID is setup for BA...
        if( BA_SESSION_ID_INVALID != pSta->baSessionID[i] )
        {
            // Release Session ID
            baReleaseSessionID( pMac, baStaIndex, i );

            // Release Rx Buffer
            baReleaseBuffer( pMac,
                    (tANI_U16) pSta->staParam.tcCfg[i].rxBufSize );
        }
    }

    return eHAL_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
\fn baAddBASession
\brief Configure device to add a BA session.
\param     tpAniSirGlobal    pMac
\param     tpAddBAParams pAddBAParams
\return    eHalStatus - status
  -------------------------------------------------------------*/
eHalStatus baAddBASession(tpAniSirGlobal pMac,
    tpAddBAParams pAddBAParams)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCfgTrafficClass tcCfg;
    tHalCfgSta staEntry;
    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    tANI_U8 queueId;

    tTpeStaDesc tpeStaDescCfg;

#ifdef CONFIGURE_SW_TEMPLATE
    tANI_U8 barCnt;
    tANI_U8 barCfgCnt =HAL_BAR_FRM_CNT;
#endif //CONFIGURE_SW_TEMPLATE

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_ValidateStaIndex( pMac,
                                                 (tANI_U8) pAddBAParams->staIdx )))
    {
        HALLOGW( halLog( pMac, LOGW,
                    FL("Invalid STA Index %d\n"),
                    pAddBAParams->staIdx ));

        status = eHAL_STATUS_FAILURE;
        return status;
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

    HALLOGW( halLog( pMac, LOGW,
                FL(" UseBATx %d, TxCompBA %d, TxBApolicy %d, txBufSize %d, tuTxBAWaitTimeout %d, "
                    "UseBARx %d, RxCompBA %d, RxBApolicy %d, rxBufSize %d, tuRxBAWaitTimeout %d\n"),
                tcCfg.fUseBATx,
                tcCfg.fTxCompBA,
                tcCfg.fTxBApolicy,
                tcCfg.txBufSize,
                tcCfg.tuTxBAWaitTimeout,
                tcCfg.fUseBARx,
                tcCfg.fRxCompBA,
                tcCfg.fRxBApolicy,
                tcCfg.rxBufSize,
                tcCfg.tuRxBAWaitTimeout ));

    // BA recipient
    if( eBA_RECIPIENT == pAddBAParams->baDirection )
    {

        //Req to TL about this new BA session...
        pSta[pAddBAParams->staIdx].addBAReqParams[pAddBAParams->baTID].
            addBAState = eHAL_ADDBA_WT_HDD_RSP;

        if( eHAL_STATUS_SUCCESS !=
                (status = baAddReqTL( pMac,
                                      pSta[pAddBAParams->staIdx].baSessionID[pAddBAParams->baTID],
                                      pAddBAParams->baTID,
                                      pAddBAParams->staIdx,
                                      (tANI_U8)pAddBAParams->baBufferSize )))
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot tell TL about a new BA session, STA index %d, TID %d\n"),
                        pAddBAParams->staIdx, pAddBAParams->baTID ));
            return  eHAL_STATUS_FAILURE;;
        }

        pSta[pAddBAParams->staIdx].baReceipientTidBitMap |=  (1 << pAddBAParams->baTID);

    }
    // BA initiator
    else {

        // Get the station descriptor
        if ((status = halTpe_GetStaDesc(pMac, (tANI_U8)pAddBAParams->staIdx,
                        &tpeStaDescCfg)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot get TPE station descriptor, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }


        // Get the QID mapping for the TID
        if ((status = halBmu_get_qid_for_qos_tid(pMac, pAddBAParams->baTID, &queueId)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot get QID mapping for TID, STA index %d, TID %d\n"),
                        pAddBAParams->staIdx, pAddBAParams->baTID ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }

        switch (queueId) {

            case BTQM_QUEUE_TX_TID_0: //0:
                tpeStaDescCfg.ampdu_window_size_qid0 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_1: //1:
                tpeStaDescCfg.ampdu_window_size_qid1 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_2: //2:
                tpeStaDescCfg.ampdu_window_size_qid2 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_3: //3:
                tpeStaDescCfg.ampdu_window_size_qid3 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_4: //4:
                tpeStaDescCfg.ampdu_window_size_qid4 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_5: //5:
                tpeStaDescCfg.ampdu_window_size_qid5 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_6: //6:
                tpeStaDescCfg.ampdu_window_size_qid6 = tcCfg.txBufSize - 1;
                break;

            case BTQM_QUEUE_TX_TID_7: //7:
                tpeStaDescCfg.ampdu_window_size_qid7 = tcCfg.txBufSize - 1;
                break;

            default:
                break;

        }

#ifndef WLAN_SOFTAP_FEATURE
        // Set the QID as valid ampdu QID
        tpeStaDescCfg.ampdu_valid |= (1 << queueId);
#endif
        HALLOGW( halLog(pMac, LOGW, FL("tpeStaDescCfg.ampdu_valid is %d\n"), tpeStaDescCfg.ampdu_valid));

        // Save the station configuration
        if ((status = halTpe_SaveStaConfig(pMac, &tpeStaDescCfg,
                        (tANI_U8)pAddBAParams->staIdx)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot save TPE station configuration, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }

#ifndef WLAN_SOFTAP_FEATURE
        // Configure BMU to send BAR before sending first frame
        if ((status = halBmu_ConfigureToSendBAR(pMac, (tANI_U8)pAddBAParams->staIdx,
                        queueId)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW(  halLog( pMac, LOGW,
                        FL("Cannot configure BMU to send BAR before sending first frame, "
                            "STA index %d, queue ID %d\n"),
                        pAddBAParams->staIdx, queueId ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }
#endif

#ifdef CONFIGURE_SW_TEMPLATE
#ifdef BMU_FATAL_ERROR
        // Configure BMU to disable transmit
        if ((status = halBmu_sta_enable_disable_control(
                        pMac, pAddBAParams->staIdx, eBMU_ENB_TX_QUE_DONOT_ENB_TRANS)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot configure BMP to disable transmit, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }
#else

        // Disable data backoffs
        halMTU_stallBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);

#endif //BMU_FATAL_ERROR

        /* This would send Unsolicit BAR frame to Rx so as to sync up with updated SSN */

        for (barCnt = 0; barCnt < barCfgCnt; barCnt++) {
            halSendUnSolicitBARFrame(pMac, pAddBAParams->staIdx, pAddBAParams->baTID, queueId);
        }
#endif //CONFIGURE_SW_TEMPLATE

        pSta[pAddBAParams->staIdx].baInitiatorTidBitMap |=  (1 << pAddBAParams->baTID);

        // Update the station descriptor
        if ((status = halTpe_UpdateStaDesc(pMac, (tANI_U8)pAddBAParams->staIdx,
                        &tpeStaDescCfg)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot update TPE station descriptor, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }

#ifdef WLAN_SOFTAP_FEATURE
        // Send the Update BA message to FW, as FW would take care of setting the AMPDU valid bit
        // This is to take care of aggregation not happening when STA is in PS
        halFW_UpdateBAMsg(pMac, pAddBAParams->staIdx, queueId, TRUE);
#endif

#ifdef CONFIGURE_SW_TEMPLATE
#ifdef BMU_FATAL_ERROR
        // Configure BMU to enable transmit
        if ((status = halBmu_sta_enable_disable_control(
                        pMac, pAddBAParams->staIdx, eBMU_ENB_TX_QUE_ENB_TRANS)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot configure BMP to enable transmit, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }
#else

        // Enable data backoffs
        halMTU_startBackoffs(pMac, SW_MTU_STALL_DATA_BKOF_MASK);

#endif //BMU_FATAL_ERROR
#endif //CONFIGURE_SW_TEMPLATE

        // Enable BMU BA update
        if ((status = halRxp_EnableDisableBmuBaUpdate(pMac, 1)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot enable BMU BA update, STA index %d\n"),
                        pAddBAParams->staIdx ));
            status = eHAL_STATUS_FAILURE;
            goto Fail;
        }


        halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, pAddBAParams->baDialogToken, (void *) pAddBAParams, 0);
    }

Fail:
    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Failed to process the AddAB Rsp \n")));
        halMsg_GenerateRsp(pMac, SIR_HAL_ADDBA_RSP, 0, NULL, 0);
    }
    return status;
}

/** -------------------------------------------------------------
\fn baDelBASession
\brief Configure device to delete BA session.
\param     tpAniSirGlobal    pMac
\param     tpDelBAParams pDelBAParams
\return    eHalStatus - status
  -------------------------------------------------------------*/
eHalStatus baDelBASession(tpAniSirGlobal pMac,
    tpDelBAParams pDelBAParams)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tCfgTrafficClass tcCfg;
    tHalCfgSta staEntry;
    tpStaStruct pSta = (tpStaStruct)pMac->hal.halMac.staTable;
    tANI_U8 queueId;
    tRpeStaQueueInfo rpeStaQueueInfo;
    tTpeStaDesc tpeStaDescCfg = {0};
    //tANI_U8    dpuIdx;

    if( eHAL_STATUS_SUCCESS !=
            (status = halTable_ValidateStaIndex( pMac,
                                                 (tANI_U8) pDelBAParams->staIdx )))
    {
        HALLOGW( halLog( pMac, LOGW,
                    FL("Invalid STA Index %d\n"),
                    pDelBAParams->staIdx ));

        status = eHAL_STATUS_FAILURE;
        return status;
    }
    else
    {
        // Restore the "saved" STA context in HAL for this STA
        halTable_RestoreStaConfig( pMac, (tHalCfgSta *) &staEntry, (tANI_U8 ) pDelBAParams->staIdx );
    }

    // Restore the current TC settings from the saved STA config
    // This ensures that the existing TC configuration
    // for this TID does not get over-written
    palCopyMemory( pMac->hHdd,
            (void *) &tcCfg,
            (void *) &(staEntry.tcCfg[pDelBAParams->baTID]),
            sizeof( tCfgTrafficClass ));

    HALLOGW( halLog( pMac, LOGW,
                FL(" UseBATx %d, TxCompBA %d, TxBApolicy %d, txBufSize %d, tuTxBAWaitTimeout %d, "
                    "UseBARx %d, RxCompBA %d, RxBApolicy %d, rxBufSize %d, tuRxBAWaitTimeout %d\n"),
                tcCfg.fUseBATx,
                tcCfg.fTxCompBA,
                tcCfg.fTxBApolicy,
                tcCfg.txBufSize,
                tcCfg.tuTxBAWaitTimeout,
                tcCfg.fUseBARx,
                tcCfg.fRxCompBA,
                tcCfg.fRxBApolicy,
                tcCfg.rxBufSize,
                tcCfg.tuRxBAWaitTimeout ));

    // Get the QID mapping for the TID
    if ((status = halBmu_get_qid_for_qos_tid(pMac, pDelBAParams->baTID, &queueId)) != eHAL_STATUS_SUCCESS)
    {
        HALLOGW( halLog( pMac, LOGW,
                    FL("Cannot get QID mapping for TID, STA index %d, TID %d\n"),
                    pDelBAParams->staIdx, pDelBAParams->baTID ));
        return status;
    }

    // BA recipient
    if (pDelBAParams->baDirection == eBA_RECIPIENT) {

        // Get station descriptor queue info
        if ((status = halRpe_GetStaDescQueueInfo(pMac, pDelBAParams->staIdx, (tANI_U32) queueId,
                        &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot get RPE station descriptor queue info, STA index %d, queue ID %d\n"),
                        pDelBAParams->staIdx, queueId ));
            return status;
        }

        /* Following sequence should be followed while resetting RPE 
         *
         * a.	Set RPE to block frames to this particular STA id/Tid
         * b.	Flush RPE full/partial state cache
         * c.	Update the RPE descriptor depending. Memset RPE desc to zero, including all the reorder/BA bitmap.
         * d.	Unblock this STA id/Tid at RPE
         *
         */

        // Lock station descriptor and flush frames
        if ((status = halRpe_BlockAndFlushFrames(pMac, (tANI_U8)pDelBAParams->staIdx, queueId,
                        eRPE_SW_ENABLE_DROP)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot lock RPE station descriptor and flush frames, STA index %d, queue ID %d\n"),
                        pDelBAParams->staIdx, queueId ));
            return status;
        }


        // Set up station descriptor
        rpeStaQueueInfo.val = 1;
        rpeStaQueueInfo.bar = 0;
        rpeStaQueueInfo.psr = 0;
        rpeStaQueueInfo.reserved1 = 0;

        rpeStaQueueInfo.rty = 1;
        rpeStaQueueInfo.fsh = 1;
        rpeStaQueueInfo.ord = 1;
        rpeStaQueueInfo.frg = 1;
        rpeStaQueueInfo.check_2k = 0;
        rpeStaQueueInfo.ba_window_size = 0;
        rpeStaQueueInfo.reorder_window_size = 0;

        rpeStaQueueInfo.ssn_sval = 0;
        rpeStaQueueInfo.ba_ssn = 0;
        rpeStaQueueInfo.staId_queueId_BAbitmapLo = 0;
        rpeStaQueueInfo.staId_queueId_BAbitmapHi = 0;
        rpeStaQueueInfo.staId_queueId_ReorderbitmapLo = 0;
        rpeStaQueueInfo.staId_queueId_ReorderbitmapHi = 0;
        rpeStaQueueInfo.reorder_ssn = 0;
        rpeStaQueueInfo.reorder_sval = 0;
        rpeStaQueueInfo.reserved2 = 0;


        // Save station queue configuration
        if ((status = halRpe_SaveStaQueueConfig(pMac, (tANI_U8)pDelBAParams->staIdx, (tANI_U32) queueId,
                        &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot save RPE station queue configuration, STA index %d, queue ID %d\n"),
                        pDelBAParams->staIdx, queueId ));
            return status;
        }

        // Check if encryption is enabled
        // Knocked-off RCWindow disabling from this code.  If there are more than one BA session 
        // (>1 TID) used at Tx, there is no guarantee that when frames arrive at Rx side how 
        // TSC/PN would be seen by DPU. So we decided to disable RC Window Enabling.
        /* 
           if ((pSta[pDelBAParams->staIdx].encMode))
           {

        // Get the DPU index
        if ((status = halTable_GetStaDpuIdx(pMac, (tANI_U8)pDelBAParams->staIdx,
        &dpuIdx)) != eHAL_STATUS_SUCCESS)
        {
        HALLOGW( halLog( pMac, LOGW,
        FL("Cannot get the DPU index, STA index %d\n"),
        pDelBAParams->staIdx ));
        return status;
        }

        // Disable DPU RC window check
        if ((status = halDpu_DisableRCWinChk(pMac, dpuIdx, (tANI_U32) queueId)) != eHAL_STATUS_SUCCESS)
        {
        HALLOGW( halLog( pMac, LOGW,
        FL("Cannot disable RC windows check, DPU index %d, queue ID %d\n"),
        dpuIdx, queueId ));
        return status;
        }
        }
        */

        // Update station descriptor queue info
        if ((status = halRpe_UpdateStaDescQueueInfo(pMac, pDelBAParams->staIdx, (tANI_U32) queueId,
                        &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot update RPE station descriptor queue info, STA index %d, queue ID %d\n"),
                        pDelBAParams->staIdx, queueId ));
            return status;
        }

        // Unlock the station descriptor
        if ((status = halRpe_UpdateSwBlockReq(pMac, (tANI_U8)pDelBAParams->staIdx, queueId,
                        eRPE_SW_DISABLE_DROP)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot unlock RPE station descriptor, STA index %d, queue ID %d\n"),
                        pDelBAParams->staIdx, queueId ));
            return status;
        }

        pSta[pDelBAParams->staIdx].baReceipientTidBitMap &= ~(1 << pDelBAParams->baTID);
    }

    // BA initiator
    else {

        // Restore the station configuration
        if ((status = halTpe_RestoreStaConfig(pMac, &tpeStaDescCfg,
                        (tANI_U8)pDelBAParams->staIdx)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot restore TPE station configuration, STA index %d\n"),
                        pDelBAParams->staIdx ));
            return status;
        }

        // Clear the QID bit in the ampdu valid address
#ifndef WLAN_SOFTAP_FEATURE
        tpeStaDescCfg.ampdu_valid &= ~(1 << queueId);
#endif

        // Save the station configuration
        if ((status = halTpe_SaveStaConfig(pMac, &tpeStaDescCfg,
                        (tANI_U8)pDelBAParams->staIdx)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot save TPE station configuration, STA index %d\n"),
                        pDelBAParams->staIdx ));
            return status;
        }

        // Update the station descriptor
        if ((status = halTpe_UpdateStaDesc(pMac, (tANI_U8)pDelBAParams->staIdx,
                        &tpeStaDescCfg)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot update TPE station descriptor, STA index %d\n"),
                        pDelBAParams->staIdx ));
            return status;
        }
#ifdef WLAN_SOFTAP_FEATURE
        // Send the Update BA message to FW, as FW would take care of setting the AMPDU valid bit
        // This is to take care of aggregation not happening when STA is in PS
        halFW_UpdateBAMsg(pMac, pDelBAParams->staIdx, queueId, FALSE);
#endif
    }

    pSta[pDelBAParams->staIdx].baInitiatorTidBitMap &= ~(1 << pDelBAParams->baTID);

#ifndef WLAN_SOFTAP_FEATURE    
    // This bit should not be turned off as in SOFTAP mode there could be multiple station
    // with BA session established.
    if (tpeStaDescCfg.ampdu_valid == 0)
    {
        // Disable BMU BA update
        if ((status = halRxp_EnableDisableBmuBaUpdate(pMac, 0)) != eHAL_STATUS_SUCCESS)
        {
            HALLOGW( halLog( pMac, LOGW,
                        FL("Cannot disable BMU BA update, STA index %d\n"),
                        pDelBAParams->staIdx ));
            return status;
        }
    }
#endif

    return status;
}

/**
 * \brief This API is used to send an IND to HDD RE: a new
 * BA Session that has just been setup
 *
 * \sa baAddReqHdd
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baSessionID The newly allocated BA Session ID
 *
 * \param baTID The TID for which the BA session is
 * being setup
 *
 * \param baBufferSize The allocated buffer size for the new
 * BA session
 *
 * \return retStatus Indicates whether we were able to
 * successfully notify TL or not
 *
 */


eHalStatus baAddReqTL( tpAniSirGlobal pMac,
    tANI_U16 baSessionID,
    tANI_U8 baTID,
    tANI_U16 staIdx,
    tANI_U8 baBufferSize )
{
   tSirMsgQ msg;
   tpAddBAInd pAddBAInd = NULL;
   //tSirRetStatus status;
   eHalStatus retStatus = eHAL_STATUS_SUCCESS;

  if (tx_timer_deactivate(&pMac->hal.addBARspTimer) != TX_SUCCESS)
  {
    HALLOGP( halLog(pMac, LOGP,
           FL("Unable to deactivate sAddBARsp timer\n")));
    return eHAL_STATUS_FAILURE;
  }

  // Allocate message buffer
  if( eHAL_STATUS_SUCCESS == palAllocateMemory( pMac->hHdd,
        (void **) &pAddBAInd,
        sizeof( tAddBAInd )))
  {
    palZeroMemory( pMac->hHdd, pAddBAInd, sizeof( tAddBAInd ));

    pAddBAInd->mesgType = SIR_HAL_HDD_ADDBA_REQ;
    pAddBAInd->mesgLen = sizeof( tAddBAInd );

    // FIXME - Currently, number of BA sessions specified
    // during this IND message is 1
    pAddBAInd->baSession.baSessionID = baSessionID;
    pAddBAInd->baSession.baTID = baTID;
    pAddBAInd->baSession.baBufferSize = baBufferSize;
    pAddBAInd->baSession.STAID = (tANI_U8) staIdx;

    // POST message to HDD
    msg.type = SIR_HAL_HDD_ADDBA_REQ;
    msg.bodyptr = pAddBAInd;
    //start TL addBA rsp timer.
    if (tx_timer_activate(&pMac->hal.addBARspTimer) != TX_SUCCESS)
    {
        HALLOGP( halLog(pMac, LOGP, FL("Could not activate AddBA Rsp timer\n")));
        return eHAL_STATUS_FAILURE;
    }

    halTlPostMsgApi(pMac, &msg);
  }

  return retStatus;
}

/**
 * \brief This API is used to send an IND to HDD RE: a BA
 * Session that has just been deleted by MAC
 *
 * \sa baDelNotifyTL
 *
 * \param pMac The global tpAniSirGlobal object
 *
 * \param baSessionID The BA Session ID that was deleted
 *
 * \return retStatus Indicates whether we were able to
 * successfully notify HDD or not
 *
 */

eHalStatus baDelNotifyTL( tpAniSirGlobal pMac,
    tANI_U16 baSessionID )
{
tpDelBAInd pDelBAInd = NULL;
//tSirRetStatus status;
eHalStatus retStatus = eHAL_STATUS_FAILURE;
tSirMsgQ msg;
tpRxBASessionTable baSessionParams = (tpRxBASessionTable) &pMac->hal.halMac.baSessionTable[baSessionID];

  // Allocate message buffer
  if( eHAL_STATUS_SUCCESS == palAllocateMemory( pMac->hHdd,
        (void **) &pDelBAInd,
        sizeof( tDelBAInd )))
  {
    palZeroMemory( pMac->hHdd, pDelBAInd, sizeof( tDelBAInd ));

    pDelBAInd->mesgType = SIR_HAL_DELETEBA_IND;
    pDelBAInd->staIdx = (tANI_U8) baSessionParams->baStaIndex ;
    pDelBAInd->baTID = (tANI_U8) baSessionParams->baTID;
    pDelBAInd->mesgLen = sizeof( tDelBAInd );

    msg.type = SIR_HAL_DELETEBA_IND;
    msg.bodyptr = pDelBAInd;
    halTlPostMsgApi(pMac, &msg);
  }

  return retStatus;
}


/** -------------------------------------------------------------
\fn baProcessTLAddBARsp
\brief API to handle the response from TL, after adding a new BA Session
\param     tpAniSirGlobal    pMac
\param     tANI_U16 baSessionID : SessionID of the current BA session.
\return    Success or Failure.
  -------------------------------------------------------------*/
eHalStatus baProcessTLAddBARsp(tpAniSirGlobal pMac, tANI_U16 baSessionID, tANI_U16 tlWindowSize)
{
    tRxBASessionTable baSessionParams = pMac->hal.halMac.baSessionTable[baSessionID];
    eHalStatus status = eHAL_STATUS_FAILURE;
    tRpeStaQueueInfo rpeStaQueueInfo;
    tANI_U8 queueId;
    tCfgTrafficClass tcCfg;

    if (tx_timer_deactivate(&pMac->hal.addBARspTimer) != TX_SUCCESS)
    {
        /** Could not deactivate
          Log error*/
        HALLOGP( halLog(pMac, LOGP,
                    FL("Unable to deactivate addBARsp timer\n")));
        return eHAL_STATUS_FAILURE;
    }

    if (baSessionParams.baValid)
    {

        tSavedAddBAReqParamsStruct addBAReqParamsStruct;
        tpAddBAParams pAddBAParams;
        tpStaStruct pSta =  (tpStaStruct)pMac->hal.halMac.staTable;
        tHalCfgSta staEntry;


        addBAReqParamsStruct = (pSta + baSessionParams.baStaIndex)->addBAReqParams[baSessionParams.baTID];
        pAddBAParams = (tpAddBAParams)(addBAReqParamsStruct.pAddBAReqParams);
        if(pAddBAParams)
        {
            if(eHAL_ADDBA_WT_HDD_RSP == addBAReqParamsStruct.addBAState)
            {
                addBAReqParamsStruct.addBAState = eHAL_ADDBA_NORMAL;
                addBAReqParamsStruct.pAddBAReqParams = NULL;
                halTable_SetStaAddBAReqParams(pMac, pAddBAParams->staIdx,
                        pAddBAParams->baTID, addBAReqParamsStruct);

                status = eHAL_STATUS_SUCCESS;


                if( eHAL_STATUS_SUCCESS !=
                        (status = halTable_ValidateStaIndex( pMac,
                                                             (tANI_U8) pAddBAParams->staIdx )))
                {
                    HALLOGW( halLog( pMac, LOGW, FL(
                                    "Invalid STA Index %d\n"),
                                pAddBAParams->staIdx ));

                    status = eHAL_STATUS_FAILURE;
                    return status;
                }
                else
                {
                    // Restore the "saved" STA context in HAL for this STA
                    halTable_RestoreStaConfig( pMac, (tHalCfgSta *) &staEntry, (tANI_U8 ) pAddBAParams->staIdx );
                }


                // Restore the current TC settings from the saved STA config
                // This ensures that , the existing TC configuration
                // for this TID do not get over-written
                palCopyMemory( pMac->hHdd,
                        (void *) &tcCfg,
                        (void *) &(staEntry.tcCfg[pAddBAParams->baTID]),
                        sizeof( tCfgTrafficClass ));


                // Get the QID mapping for the TID
                if ((status = halBmu_get_qid_for_qos_tid(pMac, pAddBAParams->baTID, &queueId)) != eHAL_STATUS_SUCCESS)
                {
                    HALLOGW( halLog( pMac, LOGW,
                                FL("Cannot get QID mapping for TID, STA index %d, TID %d\n"),
                                pAddBAParams->staIdx, pAddBAParams->baTID ));
                    goto Fail;
                }


                if( eBA_RECIPIENT == pAddBAParams->baDirection )
                {
                    // Get station descriptor queue info
                    if ((status = halRpe_GetStaDescQueueInfo(pMac, pAddBAParams->staIdx, (tANI_U32) queueId,
                                    &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGW( halLog( pMac, LOGW,
                                    FL("Cannot get RPE station descriptor queue info, STA index %d, queue ID %d\n"),
                                    pAddBAParams->staIdx, queueId ));
                        goto Fail;
                    }


                    // Lock station descriptor and flush frames
                    if ((status = halRpe_BlockAndFlushFrames(pMac, (tANI_U8)pAddBAParams->staIdx, queueId,
                                    eRPE_SW_ENABLE_DROP)) != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGW( halLog( pMac, LOGW,
                                    FL("Cannot lock RPE station descriptor and flush frames, STA index %d, queue ID %d\n"),
                                    pAddBAParams->staIdx, queueId ));
                        goto Fail;
                    }

                    /* Adding Default values to RPE descriptor since in 
                     * IBSS, after DelBA, Cache is becoming valid
                     * which in-turn write back to stale values to RPE 
                     * casuing ping to fail when we add BA session again.
                     * Resetting to default values makes sure that stale 
                     * values of RPE from previous BA session will be 
                     * wiped-off.
                     */

                    // Set up station descriptor
                    rpeStaQueueInfo.val = 1;
                    rpeStaQueueInfo.bar = 0;
                    rpeStaQueueInfo.psr = 0;
                    rpeStaQueueInfo.reserved1 = 0;

                    rpeStaQueueInfo.rty = 0;
                    rpeStaQueueInfo.fsh = 0;
                    rpeStaQueueInfo.ord = 0;
                    rpeStaQueueInfo.frg = 0;
                    rpeStaQueueInfo.check_2k = 1;
                    rpeStaQueueInfo.ba_window_size = tcCfg.rxBufSize - 1;  /* FIXME_GEN6: BA window size should be coming from the TL response message */
                    rpeStaQueueInfo.reorder_window_size = tcCfg.rxBufSize - 1; 
                    /* FIXME_GEN6: BA window size should be coming from the TL response message */

                    rpeStaQueueInfo.ssn_sval = 0;
                    rpeStaQueueInfo.ba_ssn = pAddBAParams->baSSN;
                    rpeStaQueueInfo.staId_queueId_BAbitmapLo = 0;
                    rpeStaQueueInfo.staId_queueId_BAbitmapHi = 0;
                    rpeStaQueueInfo.staId_queueId_ReorderbitmapLo = 0;
                    rpeStaQueueInfo.staId_queueId_ReorderbitmapHi = 0;
                    rpeStaQueueInfo.reserved2 = 0;

                    /* Excerpt from RPE SPEC (section 3.8)
                       -   Currently we do not touch the reorder ssnval/ssn we leave them initialized to zero
                       -   This will make rpe not set correct opcode if the first received packet after BA session is BAR
                       -   So the correct programming should be to set  
                       Reorder SSNval = 1'b1 
                       Reorder SSN = BA SSN + BA_bitmap[0]
                       */
                    rpeStaQueueInfo.reorder_sval = 1;
                    rpeStaQueueInfo.reorder_ssn = rpeStaQueueInfo.ba_ssn + (rpeStaQueueInfo.staId_queueId_BAbitmapLo & 1);

                    // Save station queue configuration
                    if ((status = halRpe_SaveStaQueueConfig(pMac, (tANI_U8)pAddBAParams->staIdx, (tANI_U32)  queueId,
                                    &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGW( halLog( pMac, LOGW,
                                    FL("Cannot save RPE station queue configuration, STA index %d, queue ID %d\n"),
                                    pAddBAParams->staIdx, queueId ));
                        goto Fail;
                    }


                    // Check if encryption is enabled
                    // Knocked-off RCWindow Enabling from this code.  If there are more than one BA session 
                    // (>1 TID) used at Tx, there is no guarantee that when frames arrive at Rx side how 
                    // TSC/PN would be seen by DPU. So we decided to disable RC Window Enabling.
                    /*
                       if ((pSta[pAddBAParams->staIdx].encMode))
                       {

                    // Get the DPU index
                    if ((status = halTable_GetStaDpuIdx(pMac, (tANI_U8)pAddBAParams->staIdx,
                    &dpuIdx)) != eHAL_STATUS_SUCCESS)
                    {
                    HALLOGW( halLog( pMac, LOGW,
                    FL("Cannot get the DPU index, STA index %d\n"),
                    pAddBAParams->staIdx ));
                    goto Fail;
                    }

                    // Enable DPU RC window check
                    if ((status = halDpu_EnableRCWinChk(pMac, dpuIdx, (tANI_U32)  queueId)) != eHAL_STATUS_SUCCESS)
                    {
                    HALLOGW( halLog( pMac, LOGW,
                    FL("Cannot enable RC window check, DPU index %d, queue ID %d\n"),
                    dpuIdx, queueId ));
                    goto Fail;
                    }
                    }
                    */

                    // Update station descriptor queue info
                    if ((status = halRpe_UpdateStaDescQueueInfo(pMac, pAddBAParams->staIdx, (tANI_U32)  queueId,
                                    &rpeStaQueueInfo)) != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGW( halLog( pMac, LOGW,
                                    FL("Cannot update RPE station descriptor queue info, STA index %d, queue ID %d\n"),
                                    pAddBAParams->staIdx, queueId ));
                        goto Fail;
                    }

                    // Unlock the station descriptor
                    if ((status = halRpe_UpdateSwBlockReq(pMac, (tANI_U8)pAddBAParams->staIdx, queueId,
                                    eRPE_SW_DISABLE_DROP)) != eHAL_STATUS_SUCCESS)
                    {
                        HALLOGW( halLog( pMac, LOGW,
                                    FL("Cannot unlock RPE station descriptor, STA index %d, queue ID %d\n"),
                                    pAddBAParams->staIdx, queueId ));
                        goto Fail;
                    }

                }

                // Enable  RXP interface with BMU FOR BA update
                if ((status = halRxp_EnableDisableBmuBaUpdate(pMac, 1)) != eHAL_STATUS_SUCCESS)
                {
                    HALLOGW( halLog( pMac, LOGW,
                                FL("Cannot enable BMU BA update, STA index %d\n"),
                                pAddBAParams->staIdx ));
                    goto Fail;
                }
                pAddBAParams->baBufferSize = tlWindowSize;
                HALLOG1( halLog( pMac, LOG1, FL("Send eBA_RECIPIENT Response to LIM.......... \n")));
                halMsg_GenerateRsp( pMac, SIR_HAL_ADDBA_RSP, pAddBAParams->baDialogToken, (void *) pAddBAParams, 0);
            } else
                HALLOGE( halLog(pMac, LOGE, FL("We are in wrong state\n")));
        }
        else
            HALLOGE( halLog(pMac, LOGE, FL("Could not retrieve pAddBAParams for baSessionID = %d\n"),
                        baSessionID));
    }
    else
        HALLOGE( halLog(pMac, LOGE, FL("BA session does not exist with sessionID = %d\n"),
                    baSessionID));

Fail:

    if (eHAL_STATUS_SUCCESS != status)
    {
        HALLOGE( halLog(pMac, LOGE, FL("Could not process the AddAB Rsp given from HDD\n")));
        halMsg_GenerateRsp(pMac, SIR_HAL_ADDBA_RSP, 0, NULL, 0);
    }

    return status;
}


// Will remove once code is reviewed for Multi-BSS case
void halBaCheckActivity(tpAniSirGlobal pMac)
{
    tpAddBaCandidate pTemp;
    tpBaActivityInd pBaActivityInd;
    tANI_U8 curSta;
    tANI_U8 tid;
    tANI_U16 baCandidateCnt = 0;
    tpStaStruct       pStaTable = (tpStaStruct) (pMac->hal.halMac.staTable);
    tpStaStruct pSta = pStaTable;
    tANI_U16 bufSize;
    tANI_U16 sequenceNum =0;
    tANI_U8 resetNeeded = 0;
    tANI_U8 newBaCandidate = 0;

    if(pMac->hal.pPECallBack == NULL)
    {
        goto baActivityTimer;
    }

    //buffer size = candidate count + size of Candidate list
    bufSize = sizeof(tBaActivityInd) + (sizeof(tAddBaCandidate) * pMac->hal.halMac.maxSta * STACFG_MAX_TC);
    if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void **) &pBaActivityInd, bufSize))
    {
        HALLOGP(halLog(pMac, LOGP, FL("palAllocateMemory failed\n")));
        goto baActivityTimer;
    }

    palZeroMemory(pMac->hHdd, (void *) pBaActivityInd, bufSize);
    pTemp = (tpAddBaCandidate) (((tANI_U8*)pBaActivityInd) + sizeof(tBaActivityInd));

    for(curSta=0; curSta<pMac->hal.halMac.maxSta; curSta++, pSta++)
    {
        if(pSta && pSta->valid && pSta->htEnabled)
        {
            newBaCandidate = 0;
            if(STA_ENTRY_PEER != pSta->staType) // we want only the peer stations.
                continue;

            vos_mem_copy((v_VOID_t*) pTemp->staAddr, (v_VOID_t*) pSta->staAddr, sizeof( tSirMacAddr ));            
            for(tid = 0; tid < STACFG_MAX_TC; tid++)
            {
                tANI_U32    txPktCount = 0;

                halTLGetTxPktCount(pMac, curSta, tid, &txPktCount);
                pSta->framesTxed[tid] = txPktCount;

                if(  (!pSta->staParam.tcCfg[tid].fUseBATx) &&
                        (pSta->framesTxed[tid] >= pSta->framesTxedLastPoll[tid] + HAL_BA_TX_FRM_THRESHOLD))
                {

                    /* Knocked-off the code to read the sequence number from BTQM to address CR 190148,
                     * where-in in PS, since we block/unblock BTQM, F/W was running out of BD/PDUs.
                     */

                    /* Retrieve the QueueId from the Tid */
                    /* Get the sequence number from the DPU Descriptor */

                    if (eHAL_STATUS_SUCCESS != halDpu_GetSequence(pMac, pSta->dpuIndex, tid, &sequenceNum)) {
                        HALLOGE( halLog(pMac, LOGE, FL("Cannot get Sequence number from DPU Descriptor with DPU Indx %d \n"), pSta->dpuIndex));
                        baCandidateCnt = 0;
                        goto out;
                    }

                    HALLOG3( halLog(pMac, LOG3, FL("STA %d (DPU %d) TID %d seqNum is %d (0x%x)\n"), curSta, pSta->dpuIndex, tid,  sequenceNum, sequenceNum));

                    /* We read the btqm queue or the DPU descriptor to retrieve the updated sequence number */
                    /* pTemp->baInfo[tid].startingSeqNum = pSta->seqNum[tid] + 1; */
                    pTemp->baInfo[tid].startingSeqNum = sequenceNum;
                    newBaCandidate = 1; //got at least one new BA candidate for this station.
                    pTemp->baInfo[tid].fBaEnable = 1;                    

                }                      

                pSta->framesTxedLastPoll[tid] = pSta->framesTxed[tid];

            }
            baCandidateCnt += newBaCandidate; //This is the no. if stations for BA candidate.
            if(newBaCandidate) {//This station has at least one BA candidate. Need to move the pointer to next station. 
                pTemp++;
                sirCopyMacAddr(pBaActivityInd->bssId, pSta->bssId);
            }
        }
    }

out:

    if((baCandidateCnt > 0) && !resetNeeded)
    {   
        pBaActivityInd->baCandidateCnt = baCandidateCnt;    
        (void) (pMac->hal.pPECallBack)(pMac, SIR_LIM_ADD_BA_IND, pBaActivityInd);
    }
    else
    {
        palFreeMemory(pMac->hHdd, pBaActivityInd);
    }
    if(resetNeeded)
    {
        HALLOGP(halLog(pMac, LOGP, FL("Unexpected hardware error. Reset Needed\n")));
        return;
    }

baActivityTimer:


    if (tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS)
    {
        /** Could not deactivate
          Log error*/
        HALLOGP( halLog(pMac, LOGP,
                    FL("Unable to deactivate baActicity check timer\n")));
        return;
    }

    if(pMac->hal.halMac.baAutoSetupEnabled) {        
        if(tx_timer_activate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS) {
            // Could not start BA activity check timer.
            // Log error
            HALLOGP( halLog(pMac, LOGP, FL("Unable to activate BA activity check timer\n")));
            return;
        }
    }

    return;

}

/**
 * @brief : BA Timer Kick Start.
 *
 * @param pMac an instance of MAC parameters
 * @param arg1 - unused.
 * @return None.
 */

eHalStatus halStartBATimer(tpAniSirGlobal  pMac) 
{
    static int timerStatus;
    eHalStatus status = eHAL_STATUS_FAILURE;

    if (timerStatus == eHALBA_TIMER_STARTED)
    {
        /* We may need to restart BA timer in following cases */
        /*
           1. Re-Association
           2. GTK Re-Key
           3. PTK Rekey
           */
        if (tx_timer_deactivate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS)
        {
            /** Could not deactivate Log error*/
            HALLOGP(halLog(pMac, LOGP, FL("Unable to deactivate baActicity check timer\n")));
            return status;
        }
        timerStatus = eHALBA_TIMER_NOTSTARTED;
    }
    if (true == pMac->hal.halMac.baAutoSetupEnabled)
    {
        if (tx_timer_activate(&pMac->hal.halMac.baActivityChkTmr) != TX_SUCCESS)
        {
            // Could not start BA activity check timer.
            // Log error
            HALLOGP(halLog(pMac, LOGP, FL("Unable to activate BA activity check timer\n")));
            return status;
        }
        timerStatus = eHALBA_TIMER_STARTED;
    }
    else
    {
        HALLOGW(halLog(pMac, LOGW, FL("AUTO BA setup default NOT enabled from CFG!\n")));
        timerStatus = eHALBA_TIMER_NOTSTARTED;
    }

    return eHAL_STATUS_SUCCESS;
}


#ifdef CONFIGURE_SW_TEMPLATE
/**
 * @brief : Fill Frame Ctrl Info for BAR.
 *
 * @param Pointer to BAR allocated memory
 * @param arg1 - used.
 * @return None.
 */

void fillFrameCtrlInfo (tSirMacFrameCtl *pfc)
{
    pfc->protVer  = 0;
    pfc->type     = SIR_MAC_CTRL_FRAME; 
    pfc->subType  = SIR_MAC_CTRL_BAR;
    pfc->toDS     = 0;
    pfc->fromDS   = 0;
    pfc->moreFrag = 0;
    pfc->retry    = 0;
    pfc->powerMgmt= 0;
    pfc->moreData = 0;
    pfc->wep      = 0;
    pfc->order    = 0;
    return;
}


/**
 * @brief : Fill BAR Ctrl Info for BAR.
 *
 * @param Pointer to BAR allocated memory
 * @param arg1 - used.
 * @return None.
 */

void fillBARCtrlInfo (barCtrlType         *pBARCtrl, tANI_U16 baTID)
{
    pBARCtrl->barAckPolicy = 0; /* This field is used under HT delayed block ack, when set to 0 
                                   (Normal Ack) on an HT-delayed block ack session, the BAR frame will solicit 
                                   an ACK frame if correctly received by the receipient. This is the same 
                                   behaviour as under delayed block ack. If set to 1(No ack) on an HT delayed 
                                   block ack session the BAR will not solicit an ACK response
                                   */
    pBARCtrl->multiTID     = 0; /* This field is always set to 0 in the basic BAR frame.If set to 1 then
                                   this is multi-TID BAR and the format differs from the basic BAR
                                   */
    pBARCtrl->bitMap       = 1; /* If set to 1, the BAR frame solocits a BA with a compressed bitmap */
    pBARCtrl->rsvd         = 0; 
    pBARCtrl->numTID       = baTID; /* If the multi TID is not set then this field carries the TID of the 
                                       block ack session. If the multi TID field is set then this field carries
                                       the number of TID fields in the Multi TID BAR frames
                                       */
    return;
}
#endif //CONFIGURE_SW_TEMPLATE

/**
 * @brief : Get updated SSN from BTQM.
 *
 * @param Pointer to BAR allocated memory
 * @param arg1 - used.
 * @return None.
 */

eHalStatus halGetUpdatedSSN(tpAniSirGlobal pMac, tANI_U16 staIdx, tANI_U16 baTID, 
                        tANI_U16 queueId, tANI_U16 *ssn)
{
    tBmuBtqmBdInfo bmuBtqmbdInfo;
    tpStaStruct pSta = &((tpStaStruct) pMac->hal.halMac.staTable)[staIdx];
    eHalStatus status = eHAL_STATUS_FAILURE;

    tANI_U32 uTotalBd, uHeadBdIdx = 0;
    tANI_U16 sequenceNum =0;

    if(pSta && pSta->valid && pSta->htEnabled)
    {
        if(STA_ENTRY_PEER != pSta->staType) // we want only the peer stations.
            return status;

        // Read BTQM Queue to get BD index of first frame in the queue
        if ((status = halBmu_ReadBtqmQFrmInfo(pMac, (tANI_U8)staIdx,
                        (tANI_U8)queueId, &uTotalBd, &uHeadBdIdx, NULL))!= 
                eHAL_STATUS_SUCCESS) {
            HALLOGP(halLog( pMac, LOGP,FL("Cannot get BTQM queue frame info for "
                            "STA index %d, queue ID %d\n"),
                        staIdx, queueId ));
            return status;
        }

        HALLOGW(halLog( pMac, LOGW, FL("BTQM STA %d Qid %d Total % BDs Head %d\n"),
                    staIdx, queueId, uTotalBd, uHeadBdIdx ));

        if(uTotalBd) {
            /* There is at least one frame in the BTQM queue, 
             * go read the seqNum by BD index */                        

            if((status = halBmu_ReadBdInfo(pMac, uHeadBdIdx, 
                            &bmuBtqmbdInfo, FALSE))
                    != eHAL_STATUS_SUCCESS) {
                HALLOGP(halLog( pMac, LOGP, FL("Can't read info for BD %d\n"), 
                            uHeadBdIdx ));
                return status;
            }
            sequenceNum = (tANI_U16) bmuBtqmbdInfo.seqNum;
        } else {

            /* If currently no packets in the queue, 
             * then get the last assigned sequenceNum by DPU */

            /* Get the sequence number from the DPU Descriptor */

            if (eHAL_STATUS_SUCCESS != halDpu_GetSequence(pMac, 
                        pSta->dpuIndex, (tANI_U8)baTID, &sequenceNum)) {
                HALLOGW(halLog(pMac, LOGW, 
                            FL("Cannot get Sequence number from DPU"
                                "Descriptor with DPU Indx %d \n"), pSta->dpuIndex));
                return eHAL_STATUS_FAILURE;
            }
        }
    }

    *ssn = sequenceNum;
    return eHAL_STATUS_SUCCESS;
}

#ifdef CONFIGURE_SW_TEMPLATE
/**
 * @brief : Send out two BAR frames.
 *
 * @param Pointer to BAR allocated memory
 * @param arg1 - used.
 * @return None.
 */


void halSendUnSolicitBARFrame(tpAniSirGlobal pMac, tANI_U16 staIdx, 
                    tANI_U16 baTID, tANI_U16 queueId)
{
    tSwTemplate swTemplate;
    tSirMacFrameCtl      fc;
    barCtrlType          barCtrl;
    tpStaStruct       pSta = &((tpStaStruct) pMac->hal.halMac.staTable)[pMac->hal.halMac.selfStaId];
    BARFrmType    *pBARFrm;
    tTpeRateIdx rateIndex = TPE_RT_IDX_11B_RATE_LONG_PR_BASE_OFFSET;
    tANI_U32     apMacAddrHi, apMacAddrLo;
    tANI_U32     staMacAddrHi, staMacAddrLo;
    tANI_U32    addrLo;
    tANI_U32    addrHi;
    tANI_U32    alignedLen;
    tANI_U16    ssn;

    eHalStatus status = eHAL_STATUS_SUCCESS;
    static tANI_U8 swBaseTemplateInit = FALSE;

    /** Initialize SW Template base */
    if (swBaseTemplateInit == FALSE) {
        if (halTpe_InitSwTemplateBase(pMac, pMac->hal.memMap.swTemplate_offset) 
                != eHAL_STATUS_SUCCESS) {
            return ;    
        }
        swBaseTemplateInit = TRUE;
    }

    /** Zero out the SW Template memory */
    halZeroDeviceMemory(pMac, pMac->hal.memMap.swTemplate_offset, 
            sizeof (tSwTemplate) + sizeof( tANI_U32 ));

    palZeroMemory(pMac->hHdd, &swTemplate, sizeof(tSwTemplate));

    swTemplate.template_type     = SIR_MAC_CTRL_FRAME;
    swTemplate.template_sub_type     = SIR_MAC_CTRL_BAR;
    swTemplate.expected_resp_sub_type = SIR_MAC_CTRL_ACK;
    swTemplate.expected_resp_type     = SIR_MAC_CTRL_FRAME;
    swTemplate.ignore_expected_resp = 1;
    swTemplate.resp_is_expected    = 1;
    halGetNonBcnRateIdx(pMac, &rateIndex);
    swTemplate.primary_data_rate_index = rateIndex;

    swTemplate.template_len = sizeof(tSwTemplate) + 
        sizeof(tANI_U32) + SW_TEMPLATE_CRC;

    status = halWriteDeviceMemory(pMac, 
            pMac->hal.memMap.swTemplate_offset,
            (tANI_U8 *)&swTemplate, SW_TEMPLATE_HEADER_LEN);

    // Allocate buffer for Control frame 
    palAllocateMemory( pMac->hHdd, (void **) &pBARFrm,sizeof( BARFrmType ));

    palZeroMemory( pMac->hHdd, pBARFrm, sizeof( BARFrmType ));

    /* Fill the frame Control fields first */

    fillFrameCtrlInfo(&fc);

    /* Strcuture Assignment, works with this compiler */
    pBARFrm->fc = fc;

    /* Fill the BAR Control information */
    fillBARCtrlInfo (&barCtrl, baTID);

    /* Strcuture Assignment, works with this compiler */
    pBARFrm->barControl = barCtrl;

    // Get the Mac address
    staMacAddrHi = (((tANI_U32)pSta->staAddr[5]) << 8) |
        ((tANI_U32)pSta->staAddr[4]);

    staMacAddrLo = (((tANI_U32)pSta->staAddr[3]) << 24) |
        (((tANI_U32)pSta->staAddr[2]) << 16) |
        (((tANI_U32)pSta->staAddr[1]) << 8) |
        ((tANI_U32)pSta->staAddr[0]);

    addrLo = ani_cpu_to_le32(staMacAddrLo);
    addrHi = ani_cpu_to_le32(staMacAddrHi);

    memcpy(pBARFrm->txAddr, &addrLo, sizeof(tANI_U32));
    memcpy(((tANI_U8*)pBARFrm->txAddr + sizeof(tANI_U32)), &addrHi, 
            sizeof(tANI_U16));

    apMacAddrHi = (((tANI_U32)pSta->bssId[5]) << 8) |
        ((tANI_U32)pSta->bssId[4]);

    apMacAddrLo =  
        (((tANI_U32)pSta->bssId[3]) << 24) |
        (((tANI_U32)pSta->bssId[2]) << 16) |
        (((tANI_U32)pSta->bssId[1]) << 8) |
        ((tANI_U32)pSta->bssId[0]);

    addrLo = ani_cpu_to_le32(apMacAddrLo);
    addrHi = ani_cpu_to_le32(apMacAddrHi);

    memcpy(pBARFrm->rxAddr, &addrLo, sizeof(tANI_U32));
    memcpy(((tANI_U8*)pBARFrm->rxAddr + sizeof(tANI_U32)), 
            &addrHi, sizeof(tANI_U16));

    /* Get the updated sequence number from BTQM */
    status = halGetUpdatedSSN(pMac, staIdx, baTID, queueId,  &ssn);

    if (status == eHAL_STATUS_FAILURE) {
        return;
    }

    pBARFrm->ssnCtrl.seqNumLo = ssn & 0x0F;
    pBARFrm->ssnCtrl.seqNumHi = (ssn >> 4) & 0xFF;

    pBARFrm->ssnCtrl.fragNum = 0;
    pBARFrm->duration = 0;

    //FIXME: halWriteDevicememory requires lenght to be mulltiple of four and aligned to 4 byte boundry.
    alignedLen = (sizeof(BARFrmType) + 3 ) & ~3 ;

    // BAR body need to be swapped since there is another swap occurs while BAL writes
    // the BAR to Libra.

    sirSwapU32BufIfNeeded((tANI_U32*)pBARFrm, alignedLen >> 2);

    /*Trigger transmission of BAR frame*/

    status = halWriteDeviceMemory(pMac, pMac->hal.memMap.swTemplate_offset + SW_TEMPLATE_HEADER_LEN,
            (tANI_U8 *)pBARFrm, alignedLen);

    halTpe_TriggerSwTemplate(pMac);

    palFreeMemory(pMac->hHdd, pBARFrm);

    return ;
}
#endif //CONFIGURE_SW_TEMPLATE
