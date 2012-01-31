/**
 *
 *  @file:         halInNav.c
 *
 *  @brief:        Source file for the InNav functionality.
 *
 *  @author:       Vinay Sridhara
 *
 *  Copyright (C) 2010, Qualcomm, Inc. All rights reserved.
 *
 *  Change History:
 * ---------------------------------------
 * 07/27/2010  File created.
 */

#ifdef FEATURE_INNAV_SUPPORT

#include "halInNav.h"
#include "aniGlobal.h"
#include "halDebug.h"

///////////////////////////////////////////////////////////////////////
//                 INNAV MEAS API
//////////////////////////////////////////////////////////////////////

eHalStatus halEnableTLTx(tpAniSirGlobal pMac);

void halInNav_SendInNavMeasRspToLim(tpAniSirGlobal pMac);
void halInNav_HandleSwitchChannelPostMeasurements(tpAniSirGlobal pMac, void* pData, tANI_U32 status, tANI_U16 dialog_token);
void halInNav_StartInNavMeasPostSetChannel(tpAniSirGlobal pMac, void* pData, tANI_U32 status, tANI_U16 dialog_token);
void halInNav_GetRttRssiResults(tpAniSirGlobal pMac, Qwlanfw_InNavMeasRspType* pFWInNavMeasRsp);
eHalStatus halInNav_StartInNavMeas(tpAniSirGlobal pMac);

/* -------------------------------------------------------
 * FUNCTION: halInNav_SendInNavMeasRspToLim()
 *
 * NOTE:
 *   This function sends the meas response 
 *   SIR_HAL_START_INNAV_MEAS_RSP to the LIM 
 *   module. This function is called even for the failure
 * -------------------------------------------------------
 */
void halInNav_SendInNavMeasRspToLim(tpAniSirGlobal pMac)
{
    tpStartInNavMeasRsp rspParamOut = NULL;

    if(pMac->hal.innavMeasParam.pRspParam == NULL)
    {
        //This means that none of the BSSID measurements went through
        if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)(&rspParamOut), sizeof(tStartInNavMeasRsp)))
        {
            HALLOGP(halLog(pMac, LOGP, FL("failed to allocate memory while sending innav response (with failed status) to lim\n")));
            return;
        }
        rspParamOut->numBSSIDs = 0;
        rspParamOut->rspLen = sizeof(tStartInNavMeasRsp);
        //The status needs to be set to FAILURE
        //if none of the measurements were successful
        //This will be set to success even if one of 
        //the measurements to any of the BSSID was
        //successfull
        rspParamOut->status = eHAL_STATUS_FAILURE;
    }
    else
    {
        tpStartInNavMeasRsp rspParam = NULL;
        rspParam = (tpStartInNavMeasRsp)(pMac->hal.innavMeasParam.pRspParam);
        if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)(&rspParamOut), rspParam->rspLen))
        {
            HALLOGP(halLog(pMac, LOGP, FL("failed to allocate memory while sending innav response to lim\n")));
            return;
        }
        palCopyMemory(pMac->hHdd, (tANI_U8*)(rspParamOut), (tANI_U8*)(pMac->hal.innavMeasParam.pRspParam), rspParam->rspLen);
        rspParamOut->status = eHAL_STATUS_SUCCESS;
    }

    halMsg_GenerateRsp(pMac, SIR_HAL_START_INNAV_MEAS_RSP, pMac->hal.innavMeasParam.dialog_token, (void*)rspParamOut, 0);

    return;
}

/* -------------------------------------------------------
 * FUNCTION: halInNav_HandleSwitchChannelPostMeasurements()
 *
 * NOTE:
 *   Callback function that is called after the channel
 *   is switched to the required channel after the 
 *   completion of the measurements
 * -------------------------------------------------------
 */
void halInNav_HandleSwitchChannelPostMeasurements(
    tpAniSirGlobal pMac, 
    void* pData, 
    tANI_U32 status, 
    tANI_U16 dialog_token)
{
    //Restore the RXP filter appropriately
    //This was saved before the INNAV operation was started
    halRxp_setSystemRxpFilterMode(pMac, pMac->hal.innavMeasParam.operatingRxpMode, eHAL_USE_GLOBAL_AND_BSS_RXP);

    //Set the measurements active to false
    pMac->hal.innavMeasParam.isMeasInProgress = eANI_BOOLEAN_FALSE;
    pMac->hal.currentChannel = pMac->hal.innavMeasParam.operatingChannelNumber;
    pMac->hal.currentCBState = pMac->hal.innavMeasParam.operatingCBState;

    //Now send the measurement results to PE
    halInNav_SendInNavMeasRspToLim(pMac);

    //Now we can free the memory allocated for 
    //the response and the req messages
    palFreeMemory(pMac->hHdd, pMac->hal.innavMeasParam.pReqParam);
    pMac->hal.innavMeasParam.pReqParam = NULL;
    palFreeMemory(pMac->hHdd, pMac->hal.innavMeasParam.pRspParam);
    pMac->hal.innavMeasParam.pRspParam = NULL;
}

/* -------------------------------------------------------
 * FUNCTION: halInNav_FinishInNavMeasReq()
 *
 * NOTE:
 *   Cleanup function that is called after the 
 *   measurements are done. This resets the rxp filters
 *   and changes the channel back to the home channel
 * -------------------------------------------------------
 */
void halInNav_FinishInNavMeasReq(tHalHandle hHalHandle)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    eHalStatus status = eHAL_STATUS_SUCCESS;

    if(eHAL_STATUS_SUCCESS != halEnableTLTx(pMac))
    {
        HALLOGE(halLog(pMac, LOGE, FL("TL TX Failed\n")));
    }

    //Now switch back to the operating channel
    status = halPhy_ChangeChannel(
                            pMac, 
                            pMac->hal.innavMeasParam.operatingChannelNumber, 
                            PHY_SINGLE_CHANNEL_CENTERED, 
                            FALSE, 
                            halInNav_HandleSwitchChannelPostMeasurements, 
                            NULL,
                            pMac->hal.innavMeasParam.dialog_token
#ifdef WLAN_AP_STA_CONCURRENCY
                            , 0
#endif
                            );

    if(status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN)
    {
         halInNav_HandleSwitchChannelPostMeasurements(pMac, NULL, status, pMac->hal.innavMeasParam.dialog_token);
    }

    return;
}

/* -------------------------------------------------------
 * FUNCTION: halInNav_StartInNavMeasPostSetChannel()
 *
 * NOTE:
 *   Callback function that is called after the channel
 *   is switched to the required channel during the 
 *   measurements
 * -------------------------------------------------------
 */
void halInNav_StartInNavMeasPostSetChannel(tpAniSirGlobal pMac, 
    void* pData, 
    tANI_U32 status, 
    tANI_U16 dialog_token)
{
    if(pData == NULL)
    {
        HALLOGE(halLog(pMac, LOGE, FL("in halInNav_StartInNavMeasPostSetChannel pData is NULL")));
        status = eHAL_STATUS_FAILURE;
        return;
    }

    halInNav_SendStartInNavMeasMesg(pMac, pData);

    palFreeMemory(pMac->hHdd, pData);

    return;
}

/* -------------------------------------------------------
 * FUNCTION: halInNav_StartInNavMeas()
 *
 * NOTE:
 *   This function does the internals for innav in HAL. 
 *   Splits up the measurement requests according to the
 *   the bssids and sends the req to the FW
 *   
 * -------------------------------------------------------
 */
eHalStatus halInNav_StartInNavMeas(tpAniSirGlobal pMac)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    tpStartInNavMeasReq reqParam;
    tANI_U8 currentChannel;
    tpHalInNavPerBssidChannelInfo pPerBssidChannelInfo;
    tANI_U8 currentBssidIndex = 0;


    do
    {
        reqParam = pMac->hal.innavMeasParam.pReqParam;
    
        if(pMac->hal.innavMeasParam.currentBssidIndex <= (reqParam->numBSSIDs-1))
        {
            currentBssidIndex = pMac->hal.innavMeasParam.currentBssidIndex;

            if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)&pPerBssidChannelInfo, sizeof(tHalInNavPerBssidChannelInfo)))
            {
                status = eHAL_STATUS_FAILURE;
                break;
            }
            
            palCopyMemory( pMac->hHdd, pPerBssidChannelInfo->selfMacAddr, reqParam->selfMacAddr, sizeof(tSirMacAddr) );

            currentChannel = (tANI_U8)(reqParam->bssidChannelInfo[pMac->hal.innavMeasParam.currentBssidIndex].channel);

            pPerBssidChannelInfo->numInNavMeasurements = reqParam->numInNavMeasurements;
            pPerBssidChannelInfo->bssid[0] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[0];
            pPerBssidChannelInfo->bssid[1] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[1];
            pPerBssidChannelInfo->bssid[2] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[2];
            pPerBssidChannelInfo->bssid[3] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[3];
            pPerBssidChannelInfo->bssid[4] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[4];
            pPerBssidChannelInfo->bssid[5] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[5];

            //now we have fetched the req data for this channel
            //increment the currentChannelIndex
            pMac->hal.innavMeasParam.currentBssidIndex++;

            HALLOG1(halLog(pMac, LOG1, FL("Setting the channel to %d"),currentChannel));

            status = halPhy_ChangeChannel(
                            pMac, 
                            currentChannel, 
                            PHY_SINGLE_CHANNEL_CENTERED, 
                            FALSE, 
                            halInNav_StartInNavMeasPostSetChannel, 
                            pPerBssidChannelInfo,
                            pMac->hal.innavMeasParam.dialog_token
#ifdef WLAN_AP_STA_CONCURRENCY
                            , 0
#endif
                            );

            if(status == eHAL_STATUS_SET_CHAN_ALREADY_ON_REQUESTED_CHAN)
            {
                halInNav_StartInNavMeasPostSetChannel(pMac, pPerBssidChannelInfo, status, pMac->hal.innavMeasParam.dialog_token);
            }
            else
            {
                if(status != eHAL_STATUS_SUCCESS)
                {
                    palFreeMemory(pMac->hHdd, pPerBssidChannelInfo);
                    halInNav_FinishInNavMeasReq(pMac);
                    break;
                }
            }
        }
        else
        {
            //all the bssids are processed
            //now send the response back to the PE

            //In-order to get out of the FW message recv context
            //we need to post another message for the measurements
            //to be handled and pass over to the PE

            //XXX: This needs to be removed once we move to the 
            //multi-session code, since this is supposed to have been
            //fixed in LIBRA multi-session

            
            //Create the message to be passed to HAL
            /*msg.type = SIR_HAL_FINISH_INNAV_MEAS_REQ;
            msg.bodyptr = NULL;
            msg.bodyval = 0;

            rc = halPostMsgApi(pMac, &msg);
            if(rc != eSIR_SUCCESS)
            {
                status = eHAL_STATUS_FAILURE;
            }*/
            halInNav_FinishInNavMeasReq(pMac);
        }
    } while(0);

    return status;
}

/* -------------------------------------------------------
 * FUNCTION: halMsg_HandleStartInNavMeasReq()
 *
 * NOTE:
 *   HAL changes channel and then sends a 
 *   "QWLANFW_HOST2FW_INNAV_MEAS_START" to FW
 * -------------------------------------------------------
 */
void halInNav_HandleStartInNavMeasReq(tHalHandle hHalHandle, 
                            tANI_U16 dialog_token, 
                            tpStartInNavMeasReq reqParam)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    pMac->hal.innavMeasParam.pReqParam = reqParam;

    //Save the current channel state information 
    //before switching to the new channel
    pMac->hal.innavMeasParam.isMeasInProgress = eANI_BOOLEAN_TRUE;
    pMac->hal.innavMeasParam.operatingChannelNumber = pMac->hal.currentChannel;
    pMac->hal.innavMeasParam.operatingCBState = pMac->hal.currentCBState;
    pMac->hal.innavMeasParam.dialog_token = dialog_token;
    //set the bssid index to the first one
    pMac->hal.innavMeasParam.currentBssidIndex = 0;

    //First save the current global RXP mode before changing it to INNAV mode
    //This will be restored when the INNAV operation completes
    pMac->hal.innavMeasParam.operatingRxpMode = halRxp_getSystemRxpMode(pMac);
    //Set RXP filter appropriately for receiving CTS or ACK frames only
    halRxp_setSystemRxpFilterMode(pMac, eRXP_INNAV_MODE, eHAL_USE_GLOBAL_AND_BSS_RXP);

    //everything was successful till now
    //start the actual innav measurement
    halInNav_StartInNavMeas(pMac);

    return;
}

eHalStatus halInNav_SendStartInNavMeasMesg(tHalHandle hHalHandle, void* pData)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    eHalStatus status = eHAL_STATUS_SUCCESS;

    tANI_U16 dialogToken;
    tANI_U16 fwMsgLen = 0;
    tANI_U8  numBssids = 0;

    tANI_U8* pBuffer = NULL;
#ifdef HAL_SELF_STA_PER_BSS
    tANI_U8  selfStaIdx;
#endif
    
    Qwlanfw_InNavMeasReqType* pInNavMeasReq = NULL;
    Qwlanfw_InNavBssidInfoType* pInNavBssidInfo = NULL;
    tpHalInNavPerBssidChannelInfo pPerBssidInfo = NULL;

    dialogToken = 0;
    pPerBssidInfo = (tpHalInNavPerBssidChannelInfo)(pData);

    fwMsgLen = sizeof(Qwlanfw_InNavMeasReqType) + 
                sizeof(Qwlanfw_InNavBssidInfoType)*MAX_BSSIDS_IN_INNAV_MEAS_REQ;

    if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)&pBuffer, fwMsgLen))
    {
        HALLOGP( halLog(pMac, LOGP, FL("INNAV: failed to allocate memory for QWLANFW_HOST2FW_INNAV_MEAS_START\n")));
        return eHAL_STATUS_FAILURE;
    }

    //hardcode the number of bssids for measurements to 1 for now
    pInNavMeasReq = (Qwlanfw_InNavMeasReqType*)pBuffer;
    pInNavMeasReq->numBssidsInReq = MAX_BSSIDS_IN_INNAV_MEAS_REQ;

    //send the selfSTA to the FW for now. With out the association,
    //FW does not know the self sta index
    
#ifndef HAL_SELF_STA_PER_BSS
    pInNavMeasReq->selfStaIndex = (tANI_U8)pMac->hal.halMac.selfStaId;
#else
    halTable_FindStaidByAddr(pMac, pPerBssidInfo->selfMacAddr, &selfStaIdx);
    pInNavMeasReq->selfStaIndex = (tANI_U8)selfStaIdx;
#endif

    for(numBssids=0;numBssids<pInNavMeasReq->numBssidsInReq;numBssids++)
    {
        pInNavBssidInfo = (Qwlanfw_InNavBssidInfoType*)(pBuffer + sizeof(Qwlanfw_InNavMeasReqType) + 
                        numBssids*(sizeof(Qwlanfw_InNavBssidInfoType)));

        pInNavBssidInfo->bssidAddr0 = pPerBssidInfo->bssid[0];
        pInNavBssidInfo->bssidAddr1 = pPerBssidInfo->bssid[1];
        pInNavBssidInfo->bssidAddr2 = pPerBssidInfo->bssid[2];
        pInNavBssidInfo->bssidAddr3 = pPerBssidInfo->bssid[3];
        pInNavBssidInfo->bssidAddr4 = pPerBssidInfo->bssid[4];
        pInNavBssidInfo->bssidAddr5 = pPerBssidInfo->bssid[5];

        pInNavBssidInfo->numRTTRSSIMeasurements = pPerBssidInfo->numInNavMeasurements;
    }

    HALLOG1( halLog(pMac, LOG1, FL("INNAV: Sending QWLANFW_HOST2FW_INNAV_MEAS_START to FW\n")));

    status = halFW_SendMsg(pMac, HAL_MODULE_ID_BTC, QWLANFW_HOST2FW_INNAV_MEAS_START, dialogToken,
                    fwMsgLen, pInNavMeasReq, TRUE, NULL);

    HALLOG1( halLog(pMac, LOG1, FL("INNAV: Done Sending QWLANFW_HOST2FW_INNAV_MEAS_START to FW\n")));

    if(status != eHAL_STATUS_SUCCESS)
    {
        VOS_ASSERT(eANI_BOOLEAN_FALSE == pMac->hal.halMac.isFwInitialized);
    }

    palFreeMemory(pMac->hHdd, pBuffer);

    return status;
}

void halInNav_GetRttRssiResults(tpAniSirGlobal pMac, Qwlanfw_InNavMeasRspType* pFWInNavMeasRsp)
{
    //HAL Data structures
    tpStartInNavMeasReq reqParam = (tpStartInNavMeasReq)(pMac->hal.innavMeasParam.pReqParam);
    tpStartInNavMeasRsp innavRsp = NULL;
    tSirRttRssiResults* pRsltsOut = NULL;
    
    InNavMeasRsltType* pRsltsIn = NULL;

    tANI_U8 currentBssidIndex = pMac->hal.innavMeasParam.currentBssidIndex - 1;

    //Working variables
    tANI_U32 oldRspLen = 0;
    tANI_U32 newRspLen = 0;
    tANI_U32 dataCnt = 0;

    tANI_U32* pSrc  = NULL;
    tANI_U32* pDest = NULL;
    tANI_U8* buffer = NULL;

    //Now copy the data to the global data structure so that 
    //it can be passed back to the PE when all the measurements
    //are done

    HALLOG1(halLog(pMac, LOG1, FL("INNAV: getting the RTT and RSSI results from FW\n")));

    if(pFWInNavMeasRsp->numSuccessfulMeas > 0)
    {
        pSrc = (tANI_U32*)((tANI_U32)(pFWInNavMeasRsp) + sizeof(Qwlanfw_InNavMeasRspType));
        pRsltsIn = (InNavMeasRsltType*)(pSrc);

        if(pMac->hal.innavMeasParam.pRspParam == NULL)
        {
            newRspLen = sizeof(tStartInNavMeasRsp) + sizeof(tSirRttRssiTimeData)*(pFWInNavMeasRsp->numSuccessfulMeas-1);
            if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)(&(pMac->hal.innavMeasParam.pRspParam)), newRspLen))
            {
                HALLOGP(halLog(pMac, LOGP, FL("INNAV: failed to allocate memory for first innavMeasParam.pRspParam\n")));
                return;
            }
            palZeroMemory(pMac->hHdd, pMac->hal.innavMeasParam.pRspParam, newRspLen);
            
            innavRsp = (tpStartInNavMeasRsp)(pMac->hal.innavMeasParam.pRspParam);
            innavRsp->numBSSIDs = 1;
            innavRsp->rspLen = (tANI_U16)newRspLen;

            pDest = (tANI_U32*)((tANI_U32)(pMac->hal.innavMeasParam.pRspParam) + sizeof(tStartInNavMeasRsp) - sizeof(tSirRttRssiResults) );
            pRsltsOut = (tpSirRttRssiResults)(pDest);

            pRsltsOut->numSuccessfulMeasurements = (tANI_U8)pFWInNavMeasRsp->numSuccessfulMeas;
            pRsltsOut->bssid[0] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[0];
            pRsltsOut->bssid[1] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[1];
            pRsltsOut->bssid[2] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[2];
            pRsltsOut->bssid[3] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[3];
            pRsltsOut->bssid[4] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[4];
            pRsltsOut->bssid[5] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[5];

            for(dataCnt=0;dataCnt<pRsltsOut->numSuccessfulMeasurements;dataCnt++)
            {
                pRsltsOut->rttRssiTimeData[dataCnt].rssi = (tANI_U8)pRsltsIn->rssi;
                pRsltsOut->rttRssiTimeData[dataCnt].rtt = (tANI_U16)pRsltsIn->rtt;
                pRsltsOut->rttRssiTimeData[dataCnt].snr = (tANI_U16)pRsltsIn->snr;
                pRsltsOut->rttRssiTimeData[dataCnt].measurementTime = pRsltsIn->tsfLo;
                pRsltsOut->rttRssiTimeData[dataCnt].measurementTimeHi = pRsltsIn->tsfHi;
                
                pSrc = (tANI_U32*)((tANI_U32)(pSrc) + sizeof(InNavMeasRsltType));
                pRsltsIn = (InNavMeasRsltType*)(pSrc);
            }
        }
        else
        {
            oldRspLen = ((tpStartInNavMeasRsp)(pMac->hal.innavMeasParam.pRspParam))->rspLen;

            //Allocate a temporary buffer for storing the previous data
            //and copy the existing data and free the memory for the global storage
            if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)&buffer, oldRspLen))
            {
                HALLOGP(halLog(pMac, LOGP, FL("INNAV: failed to allocate memory for temp buffer while handling the innav FW response\n")));
                return;
            }
            palCopyMemory(pMac->hHdd, (tANI_U8*)(buffer), (tANI_U8*)(pMac->hal.innavMeasParam.pRspParam), oldRspLen);
            palFreeMemory(pMac->hHdd, (pMac->hal.innavMeasParam.pRspParam));

            //Compute the new length of the response data structure
            newRspLen = oldRspLen + sizeof(tSirRttRssiResults) + sizeof(tSirRttRssiTimeData)*(pFWInNavMeasRsp->numSuccessfulMeas-1);

            //Now allocate memory for the global data structure and store the previous contents back
            //also free the temporary storage
            if(eHAL_STATUS_SUCCESS != palAllocateMemory(pMac->hHdd, (void**)(&(pMac->hal.innavMeasParam.pRspParam)), newRspLen))
            {
                HALLOGP(halLog(pMac, LOGP, FL("failed to allocate memory for innavMeasParam.pRspParam\n")));
                return;
            }
            palZeroMemory(pMac->hHdd, pMac->hal.innavMeasParam.pRspParam, newRspLen);
            palCopyMemory(pMac->hHdd, (tANI_U8*)(pMac->hal.innavMeasParam.pRspParam), (tANI_U8*)buffer, oldRspLen);
            palFreeMemory(pMac->hHdd, buffer);

            innavRsp = (tpStartInNavMeasRsp)(pMac->hal.innavMeasParam.pRspParam);
            innavRsp->numBSSIDs++;
            innavRsp->rspLen = (tANI_U16)newRspLen;

            pDest = (tANI_U32*)((tANI_U32)(innavRsp) + oldRspLen);
            pRsltsOut = (tpSirRttRssiResults)(pDest);

            pRsltsOut->numSuccessfulMeasurements =(tANI_U8) pFWInNavMeasRsp->numSuccessfulMeas;
            pRsltsOut->bssid[0] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[0];
            pRsltsOut->bssid[1] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[1];
            pRsltsOut->bssid[2] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[2];
            pRsltsOut->bssid[3] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[3];
            pRsltsOut->bssid[4] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[4];
            pRsltsOut->bssid[5] = reqParam->bssidChannelInfo[currentBssidIndex].bssid[5];

            for(dataCnt=0;dataCnt<pRsltsOut->numSuccessfulMeasurements;dataCnt++)
            {
                pRsltsOut->rttRssiTimeData[dataCnt].rssi = (tANI_U8)pRsltsIn->rssi;
                pRsltsOut->rttRssiTimeData[dataCnt].rtt =(tANI_U16) pRsltsIn->rtt;
                pRsltsOut->rttRssiTimeData[dataCnt].snr =(tANI_U16) pRsltsIn->snr;
                pRsltsOut->rttRssiTimeData[dataCnt].measurementTime = pRsltsIn->tsfLo;
                pRsltsOut->rttRssiTimeData[dataCnt].measurementTimeHi = pRsltsIn->tsfHi;

                pSrc = (tANI_U32*)((tANI_U32)(pSrc) + sizeof(InNavMeasRsltType));
                pRsltsIn = (InNavMeasRsltType*)(pSrc);
            }
        }
    }

    return;
}

eHalStatus halInNav_HandleFwStartInNavMeasRsp(tHalHandle hHalHandle, void* pFwMsg)
{
    tpAniSirGlobal pMac = PMAC_STRUCT(hHalHandle);

    eHalStatus status = eHAL_STATUS_SUCCESS;

    halInNav_GetRttRssiResults(pMac, (Qwlanfw_InNavMeasRspType*)(pFwMsg));

    //do not free the firmware message here.
    //it will be freed by the message handler
    //Start the next set of measurements
    halInNav_StartInNavMeas(pMac);
    
    return status;
}

#endif //FEATURE_INNAV_SUPPORT

