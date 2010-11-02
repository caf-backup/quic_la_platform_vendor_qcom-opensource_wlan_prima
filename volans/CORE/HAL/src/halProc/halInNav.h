/**
 *
 *  @file:         halInNav.h
 *
 *  @brief:        Header file for the InNav functionality.
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

#ifndef __HAL_INNAV_H__
#define __HAL_INNAV_H__

#include "halTypes.h"
#include "halPhy.h"

#include "sirApi.h"
#include "sirParams.h"

#define MAX_BSSIDS_IN_INNAV_MEAS_REQ 1

typedef struct sHalInNavPerBssidChannelInfo
{
    tANI_U8             numInNavMeasurements;
    tSirMacAddr         bssid;
} tHalInNavPerBssidChannelInfo, *tpHalInNavPerBssidChannelInfo;

typedef struct tagHalInNavMeasParam
{
    void                *pReqParam;
    void                *pRspParam;
    tSirLinkState       linkState;
    tANI_BOOLEAN        isMeasInProgress;
    tANI_U8             operatingChannelNumber;
    ePhyChanBondState   operatingCBState;
    tANI_U8             currentBssidIndex;
    tANI_U16            dialog_token;
    tANI_U32            operatingRxpMode;
} tHalInNavMeasParam, *tpHalInNavMeasParam;

// Msg header is used from tSirMsgQ
// Msg Type = SIR_HAL_START_INNAV_MEAS_REQ
typedef struct tagStartInNavMeasReq
{

    // The return status of SIR_HAL_INIT_INNAV_MEAS_REQ is reported here
    eHalStatus                 status;

    //Request Parameters

    //Number of BSSIDs
    tANI_U8                    numBSSIDs;
    //Number of Measurements required
    tANI_U8                    numInNavMeasurements;
    //Type of measurements (RTS-CTS or FRAME-BASED)
    eSirInNavMeasurementMode   measurementMode;
    //bssid channel info for doing the measurements
    tSirBSSIDChannelInfo       bssidChannelInfo[1];

} tStartInNavMeasReq, *tpStartInNavMeasReq;

// Msg header is used from tSirMsgQ
// Msg Type = SIR_HAL_START_INNAV_MEAS_RSP
typedef struct tagStartInNavMeasRsp
{
    tANI_U8             numBSSIDs;
    tANI_U16            rspLen;
    eHalStatus          status;
    tSirRttRssiResults  rttRssiResults[1];
} tStartInNavMeasRsp, *tpStartInNavMeasRsp;

/**************************************************************
 Function declarations
 **************************************************************/
void halInNav_HandleStartInNavMeasReq(tHalHandle hHalHandle, tANI_U16 dialog_token, tpStartInNavMeasReq reqParam);
eHalStatus halInNav_HandleFwStartInNavMeasRsp(tHalHandle hHalHandle, void* pFwMsg);
eHalStatus halInNav_SendStartInNavMeasMesg(tHalHandle hHalHandle, void* pData);
void halInNav_FinishInNavMeasReq(tHalHandle hHalHandle);

#endif //__HAL_INNAV_H__

#endif //FEATURE_INNAV_SUPPORT
