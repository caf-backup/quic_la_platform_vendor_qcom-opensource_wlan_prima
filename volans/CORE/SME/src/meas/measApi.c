#ifdef FEATURE_INNAV_SUPPORT
/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file measApi.c
  
    Implementation for the InNav Measurement interfaces.
  
    Copyright (C) 2010 Qualcomm Incorporated.
  
 
   ========================================================================== */
#include "halInternal.h"
#include "palApi.h"
#include "smeInside.h"
#include "smsDebug.h"

#include "csrSupport.h"
#include "wlan_qct_tl.h"

#include "vos_diag_core_log.h"
#include "vos_diag_core_event.h"

/* ---------------------------------------------------------------------------
    FUNCTION DECLARATIONS 
   ---------------------------------------------------------------------------*/
void measInNavMeasurementStartTimerHandler(void*);

/* ---------------------------------------------------------------------------
    \fn measInNavOpen
    \brief This function must be called before any API call to MEAS (InNav module)
    \return eHalStatus     
  -------------------------------------------------------------------------------*/

eHalStatus measInNavOpen(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    do
    {
        //initialize all the variables to null
        vos_mem_set(&(pMac->innavMeas), sizeof(tMeasInNavMeasurementStruct), 0);
        //allocate the timer for the measurement starting for each measurement set
        status = palTimerAlloc(pMac->hHdd, &pMac->innavMeas.hTimerMeasurement, measInNavMeasurementStartTimerHandler, pMac);
        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, "measInNavOpen: Cannot allocate memory for the measurement timer function\n");
            break;
        }
    } while(0);

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measInNavClose
    \brief This function must be called before closing the csr module
    \return eHalStatus     
  -------------------------------------------------------------------------------*/

eHalStatus measInNavClose(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    do
    {
        if(pMac->innavMeas.numMeasurementSetsRemaining > 0)
            status = measStopTimers(hHal);

        if(!HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGE, "measInNavClose: Failed in measInNavClose at measStopTimers\n");
            break;
        }

        if(pMac->innavMeas.pMeasurementResult != NULL)
        {
            vos_mem_free(pMac->innavMeas.pMeasurementResult);
        }
        if(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo != NULL)
        {
            vos_mem_free(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo);
        }
        //free the timer for the measurement 
        palTimerFree(pMac->hHdd, pMac->innavMeas.hTimerMeasurement);
        
        //initialize all the variables to null
        vos_mem_set(&(pMac->innavMeas), sizeof(tMeasInNavMeasurementStruct), 0);
    } while(0);

    return eHAL_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------
    \fn measStopTimers
    \brief This function stops all the timers related to inNav measurements
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measStopTimers(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    status = palTimerStop(pMac->hHdd, pMac->innavMeas.hTimerMeasurement);

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measReleaseCommand
    \brief This function removes the measCommand from the active list and 
           and frees up any memory occupied by this
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
void measReleaseInNavMeasurementReqCommand(tpAniSirGlobal pMac, tSmeCmd *pMeasCmd, eMeasMeasurementStatus measStatus)
{
    //Do the callback
    pMeasCmd->u.measCmd.callback(pMac, pMeasCmd->u.measCmd.pContext, pMeasCmd->u.measCmd.measurementID, measStatus);

    //First take this command out of the active list
    if(csrLLRemoveEntry(&pMac->sme.smeCmdActiveList, &pMeasCmd->Link, LL_ACCESS_LOCK))
    {
        //free the memory allocated for the bssidChannel list
        vos_mem_free(pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo);

        //NULL the pointer
        pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo = NULL;
        vos_mem_set(&(pMeasCmd->u.measCmd), sizeof(tMeasCmd), 0);

        //Now put this command back on the avilable command list
        smeReleaseCommand(pMac, pMeasCmd);
    }
    else
    {
        smsLog(pMac, LOGE, "INNAV:MEAS **************** measReleaseInNavMeasurementReqCommand cannot release the command\n");
    }
}

/* ---------------------------------------------------------------------------
    \fn measInNavMeasurementRequest
    \brief Request an innav measurement for given set of BSSIDs
    \param sessionId - Id of session to be used for measurement
    \param pMeasurementRequestID - pointer to an object to get back the request ID
    \param callback - a callback function that measurement calls upon finish
    \param pContext - a pointer passed in for the callback
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measInNavMeasurementRequest(tHalHandle hHal, 
                                tANI_U8 sessionId,
                                tInNavMeasurementConfig *measurementReq, 
                                tANI_U32 *pMeasurementRequestID, 
                                measMeasurementCompleteCallback callback, 
                                void *pContext)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal pMac = PMAC_STRUCT( hHal );
    tSmeCmd *pMeasCmd = NULL;

    do 
    {
        if( !CSR_IS_SESSION_VALID( pMac, sessionId ) )
        {
           status = eHAL_STATUS_FAILURE;
           break;
        }
        //This is a new measurement configuration coming in
        //so stop all the timers
        status = measStopTimers(hHal);
        if(status == eHAL_STATUS_FAILURE)
        {
            break;
        }

        //check if the number of measurement sets == 0
        //This means that we should stop the ongoing measurement set
        if(measurementReq->numSetRepetitions == 0)
        {
            if(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo != NULL)
            {
                vos_mem_free(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo);
                pMac->innavMeas.measurementConfig.measBSSIDChannelInfo = NULL;
                
                pMac->innavMeas.measurementConfig.numBSSIDs = 0;
                pMac->innavMeas.measurementConfig.numInNavMeasurements = 0;
                pMac->innavMeas.measurementConfig.numSetRepetitions = 0;
                pMac->innavMeas.measurementConfig.measurementTimeInterval = 0;
                pMac->innavMeas.measurementConfig.sessionId = sessionId;
                pMac->innavMeas.callback = NULL;
                pMac->innavMeas.pContext = NULL;
            }
            return status;
        }

        //now set all the measurement configuration for this measurement set
        pMac->innavMeas.measurementConfig.numBSSIDs = measurementReq->numBSSIDs;
        pMac->innavMeas.measurementConfig.numInNavMeasurements = measurementReq->numInNavMeasurements;
        pMac->innavMeas.measurementConfig.numSetRepetitions = measurementReq->numSetRepetitions;
        pMac->innavMeas.measurementConfig.measurementTimeInterval = measurementReq->measurementTimeInterval;
        pMac->innavMeas.measurementConfig.measurementMode = measurementReq->measurementMode;
        pMac->innavMeas.measurementConfig.sessionId = sessionId;
        pMac->innavMeas.callback = callback;
        pMac->innavMeas.pContext = pContext;
        pMac->innavMeas.measurementRequestID = *(pMeasurementRequestID);
    
        //make sure the pointer to the BSSID channel information is NULL;
        //If this is not null, then there is something remaining from the previous configuration
        //This could happen due to one of the following reasons
        //1. This is not the first InNav measurement configuration coming into SME
        //   a. And the measurement for the previous config is still running
        //      hence we need to clear the memory before reallocating this
        //      Due to this also make sure the configuration status is cleared out
        //      correctly
        //   b. The previous measurement configuration finished and some previous function did not 
        //      clear the configuration correctly. This has to be done by the timer handler. When the
        //      measurement timer handler expires, the number of remaining measurement sets will be set 
        //      to zero when it is the last measurement. When this happens the status has to be cleared out correctly.
        //   c. If the timer did not expire and a scan came in. Due to the optimization feature, the scan
        //      command when processed will have to clear the status correctly.

        if(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo != NULL)
        {
            vos_mem_free(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo);
        }

        //Based on the numBSSIDs allocate the memory for the bssid channel info structure and copy the information
        pMac->innavMeas.measurementConfig.measBSSIDChannelInfo = (tMeasurementBSSIDChannelInfo*)
                                                                    vos_mem_malloc(sizeof(tMeasurementBSSIDChannelInfo)*
                                                                        pMac->innavMeas.measurementConfig.numBSSIDs);

        vos_mem_copy((v_VOID_t*)(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo),
                             (v_VOID_t*)(measurementReq->measBSSIDChannelInfo),
                             sizeof(tMeasurementBSSIDChannelInfo)*(measurementReq->numBSSIDs));
    
        //now set the status of the measurements correctly
        pMac->innavMeas.numMeasurementSetsRemaining = measurementReq->numSetRepetitions;
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_FALSE;
    
        /*PER MEASUREMENT COMMAND INTERFACE -- BEGIN*/

        pMeasCmd = smeGetCommandBuffer(pMac);
    
        //fill up the command before posting it.
        if(pMeasCmd)
        {
            pMeasCmd->command = eSmeCommandMeas;
            pMeasCmd->u.measCmd.callback = callback;
            pMeasCmd->u.measCmd.pContext = pContext;
            pMeasCmd->u.measCmd.measurementID = pMac->innavMeas.measurementRequestID;
    
            //set the measurementRequest
            pMeasCmd->u.measCmd.measurementReq.sessionId = pMac->innavMeas.measurementConfig.sessionId;
            pMeasCmd->u.measCmd.measurementReq.numBSSIDs = pMac->innavMeas.measurementConfig.numBSSIDs;
            pMeasCmd->u.measCmd.measurementReq.numInNavMeasurements = pMac->innavMeas.measurementConfig.numInNavMeasurements;
            pMeasCmd->u.measCmd.measurementReq.measurementMode = pMac->innavMeas.measurementConfig.measurementMode;
            pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo = (tMeasurementBSSIDChannelInfo*)
                                                                    vos_mem_malloc(sizeof(tMeasurementBSSIDChannelInfo)*
                                                                        pMeasCmd->u.measCmd.measurementReq.numBSSIDs);
    
            vos_mem_copy((v_VOID_t*)(pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo), 
                             (v_VOID_t*)(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo),
                             sizeof(tMeasurementBSSIDChannelInfo)*(pMeasCmd->u.measCmd.measurementReq.numBSSIDs));
        }
        else
        {
            status = eHAL_STATUS_FAILURE;
            break;
        }
    
        //now queue this command in the sme command queue
        //Here since this is not interacting with the csr just push the command
        //into the sme queue. Also push this command with the normal priority
        smePushCommand(pMac, pMeasCmd, eANI_BOOLEAN_FALSE);

        /*PER MEASUREMENT COMMAND INTERFACE -- END*/

    } while(0);

    if(!HAL_STATUS_SUCCESS(status) && pMeasCmd)
    {
        measReleaseInNavMeasurementReqCommand(pMac, pMeasCmd, eMEAS_MEASUREMENT_FAILURE);
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_FALSE;
    }

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measInNavMeasurementStartTimerHandler
    \brief This function must be called before any API call to MEAS (InNav module)
    \return eHalStatus     
  -------------------------------------------------------------------------------*/

void measInNavMeasurementStartTimerHandler(void* pv)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(pv);
    tSmeCmd *pMeasCmd = NULL;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    
    do
    {
        /*PER MEASUREMENT COMMAND INTERFACE -- BEGIN*/

        pMeasCmd = smeGetCommandBuffer(pMac);
    
        //fill up the command before posting it.
        if(pMeasCmd)
        {
            //increment the measurement id
            pMac->innavMeas.measurementRequestID = pMac->innavMeas.nextMeasurementId++;

            pMeasCmd->command = eSmeCommandMeas;
            pMeasCmd->u.measCmd.callback = pMac->innavMeas.callback; //this is stored at the meas init
            pMeasCmd->u.measCmd.pContext = pMac->innavMeas.pContext; //this is stored at the meas init
            pMeasCmd->u.measCmd.measurementID = pMac->innavMeas.measurementRequestID;
    
            //set the measurementRequest
            pMeasCmd->u.measCmd.measurementReq.sessionId = pMac->innavMeas.measurementConfig.sessionId;
            pMeasCmd->u.measCmd.measurementReq.numBSSIDs = pMac->innavMeas.measurementConfig.numBSSIDs;
            pMeasCmd->u.measCmd.measurementReq.numInNavMeasurements = pMac->innavMeas.measurementConfig.numInNavMeasurements;
            pMeasCmd->u.measCmd.measurementReq.measurementMode = pMac->innavMeas.measurementConfig.measurementMode;
            pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo = (tMeasurementBSSIDChannelInfo*)
                                                                    vos_mem_malloc(sizeof(tMeasurementBSSIDChannelInfo)*
                                                                        pMeasCmd->u.measCmd.measurementReq.numBSSIDs);
    
            vos_mem_copy((v_VOID_t*)(pMeasCmd->u.measCmd.measurementReq.bssidChannelInfo), 
                             (v_VOID_t*)(pMac->innavMeas.measurementConfig.measBSSIDChannelInfo),
                             sizeof(tMeasurementBSSIDChannelInfo)*(pMeasCmd->u.measCmd.measurementReq.numBSSIDs));
        }
        else
        {
            status = eHAL_STATUS_FAILURE;
            break;
        }
    
        //now queue this command in the sme command queue
        //Here since this is not interacting with the csr just push the command
        //into the sme queue. Also push this command with the normal priority
        smePushCommand(pMac, pMeasCmd, eANI_BOOLEAN_FALSE);

        /*PER MEASUREMENT COMMAND INTERFACE -- END*/

    } while(0);

    if(!HAL_STATUS_SUCCESS(status) && pMeasCmd)
    {
        measReleaseInNavMeasurementReqCommand(pMac, pMeasCmd, eMEAS_MEASUREMENT_FAILURE);
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_FALSE;
    }

    return;
}

eHalStatus measStartInNavMeasTimer(tpAniSirGlobal pMac, tANI_U32 timeIntervalms)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    status = palTimerStart(pMac->hHdd, pMac->innavMeas.hTimerMeasurement, timeIntervalms*1000, eANI_BOOLEAN_FALSE);

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measSendMBInNavMeasurementReq
    \brief Request an innav measurement be passed down to PE
    \param pMac:
    \param pMeasReq: Pointer to the innav measurement request
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measSendMBInNavMeasurementReq(tpAniSirGlobal pMac, tInNavMeasurementRequest *pMeasReq)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tSirMeasInNavMeasurementReq* pMsg;
    tANI_U16 msgLen;
    tCsrRoamSession *pSession = CSR_GET_SESSION( pMac, pMeasReq->sessionId );

    smsLog(pMac, LOGW, "INNAV: entering Function %s\n", __FUNCTION__);
    
    msgLen = (tANI_U16)(sizeof(tSirMeasInNavMeasurementReq) - sizeof(tSirBSSIDChannelInfo) + 
                        (sizeof(tSirBSSIDChannelInfo)*pMeasReq->numBSSIDs));
    status = palAllocateMemory(pMac->hHdd, (void**)&pMsg, msgLen);
    if(HAL_STATUS_SUCCESS(status))
    {
        palZeroMemory(pMac->hHdd, pMsg, msgLen);
        pMsg->messageType = pal_cpu_to_be16((tANI_U16)eWNI_SME_INNAV_MEAS_REQ);
        pMsg->length = pal_cpu_to_be16(msgLen);
        palCopyMemory(pMac->hHdd, pMsg->selfMacAddr, pSession->selfMacAddr, sizeof(tSirMacAddr) );
        pMsg->numBSSIDs = pMeasReq->numBSSIDs;
        pMsg->numInNavMeasurements = pMeasReq->numInNavMeasurements;
        pMsg->measurementMode = (tANI_U32)(pMeasReq->measurementMode);
        status = palCopyMemory(pMac->hHdd, pMsg->bssidChannelInfo, pMeasReq->bssidChannelInfo, sizeof(tSirBSSIDChannelInfo)*pMsg->numBSSIDs);
        if(HAL_STATUS_SUCCESS(status))
        {
            smsLog(pMac, LOGW, "INNAV: sending message to pe%s\n", __FUNCTION__);
            status = palSendMBMessage(pMac->hHdd, pMsg);
        }
        else
        {
            palFreeMemory(pMac->hHdd, pMsg);
        }
    }

    smsLog(pMac, LOGW, "INNAV: exiting Function %s\n", __FUNCTION__);

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measProcessInNavMeasCommand
    \brief This function is called by the smeProcessCommand when the case hits
           eSmeCommandMeas
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measProcessInNavMeasCommand(tpAniSirGlobal pMac, tSmeCmd *pMeasCmd)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;

    //check if the system is in proper mode of operation for 
    //innav to be functional. Currently, concurrency is not
    //supported and the driver must be operational only as 
    //STA for innav to be functional. We return an invalid 
    //mode flag if it is operational as any one of the following
    //in any of the active sessions
    //1. AP Mode
    //2. IBSS Mode
    //3. BTAMP Mode ...

    if(eHAL_STATUS_SUCCESS == measIsInNavAllowed(pMac))
    {
        //set the status of the innavMeas.measurementActive to true
        smsLog(pMac, LOG1, "%s: INNAV Measurements allowed in the current mode\n", __FUNCTION__);
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_TRUE;
        status = measSendMBInNavMeasurementReq(pMac, &(pMeasCmd->u.measCmd.measurementReq));
    }
    else
    {
        smsLog(pMac, LOG1, "%s: INNAV Measurements not allowed in the current mode\n", __FUNCTION__);
        measReleaseInNavMeasurementReqCommand(pMac, pMeasCmd, eMEAS_MEASUREMENT_INVALID_MODE);
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_FALSE;
    }

    return status;
}

/* ---------------------------------------------------------------------------
    \fn sme_MeasHandleInNavMeasRsp
    \brief This function processes the measurement response obtained from the PE
    \param pMsg - Pointer to the pSirSmeInNavMeasRsp
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus sme_MeasHandleInNavMeasRsp(tHalHandle hHal, tANI_U8* pMsg)
{
    eHalStatus                         status = eHAL_STATUS_SUCCESS;
    tpAniSirGlobal                     pMac;
    tListElem                          *pEntry = NULL;
    tSmeCmd                            *pCommand = NULL;
    tSirMeasInNavMeasurementRsp*       pMeasRsp = NULL;
    eMeasMeasurementStatus             measStatus;

    tSirRttRssiResults*                pRslts_in = NULL;
    tSirRttRssiResults*                pRslts_out = NULL;

    pMac = PMAC_STRUCT(hHal);

    smsLog(pMac, LOG1, "%s: INNAV Entering\n", __FUNCTION__);

    do
    {
        if(pMsg == NULL)
        {
            smsLog(pMac, LOGE, "in %s msg ptr is NULL\n", __FUNCTION__);
            status = eHAL_STATUS_FAILURE;
            break;
        }
    
        pEntry = csrLLPeekHead( &pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK );
        if(pEntry)
        {
            pCommand = GET_BASE_ADDR( pEntry, tSmeCmd, Link );
            if(eSmeCommandMeas == pCommand->command)
            {
                pMeasRsp = (tSirMeasInNavMeasurementRsp*)pMsg;
                measStatus = (eSIR_SME_SUCCESS == pMeasRsp->statusCode) ? eMEAS_MEASUREMENT_SUCCESS : eMEAS_MEASUREMENT_FAILURE;

                //A success is returned when atleast one of the measurements is completed successfully

                if(measStatus == eMEAS_MEASUREMENT_SUCCESS)
                {
                    //make sure to acquire the lock before modifying the data
                    status = sme_AcquireGlobalLock(&pMac->sme);
                    if(!HAL_STATUS_SUCCESS(status))
                    {
                        break;
                    }
                    if(pMac->innavMeas.pMeasurementResult != NULL)
                    {
                        if(pMac->innavMeas.pMeasurementResult->numBSSIDs == 0)
                        {
                            sme_ReleaseGlobalLock(&pMac->sme);
                            smsLog(pMac, LOGE, "in %s msg ptr pMac->innavMeas.pMeasurementResult != NULL but #BSSIDs = 0\n", __FUNCTION__);
                            break;
                        }
                        else
                        {
                            vos_mem_free(pMac->innavMeas.pMeasurementResult);
                            pMac->innavMeas.pMeasurementResult = (tInNavMeasurementResponse*)vos_mem_malloc(
                                        pMeasRsp->length - sizeof(tSirMeasInNavMeasurementRsp) + sizeof(tInNavMeasurementResponse));
                        }
                    }
                    else
                    {
                        pMac->innavMeas.pMeasurementResult = (tInNavMeasurementResponse*)vos_mem_malloc(
                                    pMeasRsp->length - sizeof(tSirMeasInNavMeasurementRsp) + sizeof(tInNavMeasurementResponse));
                    }

                    if(pMac->innavMeas.pMeasurementResult == NULL)
                    {
                        sme_ReleaseGlobalLock(&pMac->sme);
                        smsLog(pMac, LOGE, "in %s vos_mem_malloc failed for pMac->innavMeas.pMeasurementResult\n", 
                              __FUNCTION__);
                        status = eHAL_STATUS_FAILURE;
                        break;
                    }

                    //Now copy over the data
                    pRslts_in = &pMeasRsp->rttRssiResults[0];
                    pRslts_out = &pMac->innavMeas.pMeasurementResult->rttRssiResults[0];
                    pMac->innavMeas.pMeasurementResult->numBSSIDs = pMeasRsp->numBSSIDs;
                    vos_mem_copy((v_VOID_t*)(pRslts_out), 
                                 (v_VOID_t*)(pRslts_in), 
                                 pMeasRsp->length - sizeof(tSirMeasInNavMeasurementRsp) + sizeof(tSirRttRssiResults));
                    sme_ReleaseGlobalLock(&pMac->sme);
                }
                else
                {
                    smsLog(pMac, LOGE, "in %s innav measurements failed\n", __FUNCTION__);
                }
            }
            else
            {
                smsLog(pMac, LOGE, "in %s eWNI_SME_INNAV_MEAS_RSP Received but NO INNAV:MEAS are ACTIVE ...\n",
                    __FUNCTION__);
                status = eHAL_STATUS_FAILURE;
                break;
            }
        }
        else
        {
            smsLog(pMac, LOGE, "in %s eWNI_SME_INNAV_MEAS_RSP Received but NO commands are ACTIVE ...\n",
                    __FUNCTION__);
            status = eHAL_STATUS_FAILURE;
            break;
        }
        measReleaseInNavMeasurementReqCommand(pMac, pCommand, measStatus);
        pMac->innavMeas.measurementActive = eANI_BOOLEAN_FALSE;
        pMac->innavMeas.numMeasurementSetsRemaining = pMac->innavMeas.numMeasurementSetsRemaining - 1;
        //Now also activate the restart timer
        //restart the timer only if the measurementSets remaining > 0
        if(pMac->innavMeas.numMeasurementSetsRemaining > 0)
        {
            status = measStartInNavMeasTimer(pMac, pMac->innavMeas.measurementConfig.measurementTimeInterval);
            if(status != eHAL_STATUS_SUCCESS)
            {
                smsLog(pMac, LOGE, "in %s could not start the measurement timer\n", __FUNCTION__);
                break;
            }
        }
        else
        {
            smsLog(pMac, LOG1, "no further measurements. Hence not activating the timer in INNAV %s\n", __FUNCTION__);
        }

    } while(0);

    return status;
}

/* ---------------------------------------------------------------------------
    \fn measIsInNavAllowed
    \brief This function checks if InNav measurements can be performed in the 
           current driver state
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measIsInNavAllowed(tHalHandle hHal)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tANI_U32 sessionId;

    tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

    for(sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++)
    {
        if(CSR_IS_SESSION_VALID(pMac, sessionId))
        {
            if(csrIsConnStateIbss(pMac, sessionId) || csrIsBTAMP(pMac, sessionId) 
#ifdef WLAN_SOFTAP_FEATURE
               || csrIsConnStateConnectedInfraAp(pMac, sessionId)
#endif
               )
            {
                //co-exist with IBSS or BT-AMP or Soft-AP mode is not supported
                smsLog(pMac, LOGW, "INNAV is not allowed due to IBSS|BTAMP|SAP exist in session %d\n", sessionId);
                status = eHAL_STATUS_CSR_WRONG_STATE;
                break;
            }
        }
    }

    smsLog(pMac, LOG1, "Exiting measIsInNavAllowed with status %d\n", status);

    return (status);
}

#endif /*FEATURE_INNAV_SUPPORT*/
