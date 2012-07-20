#ifdef FEATURE_INNAV_SUPPORT

/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file measApi.h
  
    Exports and types for the Common InNav Measurement Module interfaces.
  
    Copyright (C) 2010 Qualcomm Inc.
  
 
   ========================================================================== */

#ifndef __MEAS_API_H__
#define __MEAS_API_H__
#include "sirApi.h"
#include "sirMacProtDef.h"
#include "csrLinkList.h"

typedef tANI_U8 tMeasurementBSSID[WNI_CFG_BSSID_LEN];

/*************************************************************************************************************
  MEASUREMENT REQUEST/CONFIGURATION - DATA STRUCTURES
*************************************************************************************************************/

/* The HW defines two ways in which the measurement can be performed
   One is by sending the RTS and receiving the CTS frame
   The other is by sending a PROBE_REQ and receiving an ACK frame */
typedef enum
{
    eMEAS_RTS_CTS_BASED = 1,
    eMEAS_FRAME_BASED,
} eInNavMeasurementMode;

/* Common structure for pairing the bssid and the channel
   information */
typedef struct tagMeasurementBSSIDChannelInfo
{
    tMeasurementBSSID   bssid; //bssid to be measured
    tANI_U16            channel; //Channel of which the BSSID is operating
} tMeasurementBSSIDChannelInfo;

/* Structure for defining the measurement configuration
   for the current measurement set */
typedef struct tagInNavMeasurementConfig
{
    //session Id for which measurement is active
    tANI_U8   sessionId;

    //Number of BSSIDs for measurements
    tANI_U8   numBSSIDs;

    //Number of RTT and RSSI measurements per BSSID
    tANI_U8   numInNavMeasurements;

    //Number of times to measure a given BSSID set
    tANI_U16  numSetRepetitions;

    //Time interval between the measurement sets
    tANI_U32  measurementTimeInterval;

    //Mode of measurement - RTS-CTS or Frame based
    eInNavMeasurementMode measurementMode;

    // Pointer to the list of all BSSIDs and respective channels that
    // need to be measured - counted by numBSSIDs. Also expect that this
    // list will be ordered by the increasing order of channel and "maybe"
    // by the decreasing order of RSSI values returned by the scan
    tMeasurementBSSIDChannelInfo* measBSSIDChannelInfo; 
} tInNavMeasurementConfig;

/* Structure for defining each measurement sent to the PE */
typedef struct tagInNavMeasurementRequest
{
    tANI_U8   sessionId;
    //Number of BSSIDs in the current measurement request
    tANI_U8   numBSSIDs;
    //Number of RTT and RSSI measurements needed per BSSID
    tANI_U8   numInNavMeasurements;
    //Mode of measurement
    eInNavMeasurementMode measurementMode;
    tMeasurementBSSIDChannelInfo* bssidChannelInfo;
} tInNavMeasurementRequest;

/*************************************************************************************************************
  MEASUREMENT RESPONSE - DATA STRUCTURES
*************************************************************************************************************/
typedef struct tagInNavMeasurementResponse
{
    tANI_U8             numBSSIDs; //Number of BSSIDs for which the measurement was performed
    tSirRttRssiResults  rttRssiResults[1]; //Pointer to the array of result data for each BSSID
} tInNavMeasurementResponse;

/*************************************************************************************************************/

typedef enum
{
    eMEAS_MEASUREMENT_SUCCESS=1,
    eMEAS_MEASUREMENT_FAILURE,
    eMEAS_MEASUREMENT_INVALID_MODE,
} eMeasMeasurementStatus;

/* ---------------------------------------------------------------------------
    \fn measInNavOpen
    \brief This function must be called before any API call to MEAS (InNav module)
    \return eHalStatus     
  -------------------------------------------------------------------------------*/

eHalStatus measInNavOpen(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn measInNavClose
    \brief This function must be called before closing the csr module
    \return eHalStatus     
  -------------------------------------------------------------------------------*/

eHalStatus measInNavClose(tHalHandle hHal);

/* HDD Callback function for the sme to callback when the measurement
results are available */
typedef eHalStatus (*measMeasurementCompleteCallback)(
                                           tHalHandle, 
                                           void* p2, 
                                           tANI_U32 measurementID, 
                                           eMeasMeasurementStatus status);

/* ---------------------------------------------------------------------------
    \fn measInNavMeasurementRequest
    \brief Request an innav measurement for given set of BSSIDs
    \param sessionId - Id of session to be used for measurement
    \param pMeasurementRequestID - pointer to an object to get back the request ID
    \param callback - a callback function that measurement calls upon finish
    \param pContext - a pointer passed in for the callback
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measInNavMeasurementRequest(tHalHandle, tANI_U8, tInNavMeasurementConfig *, tANI_U32 *pMeasurementRequestID, 
                            measMeasurementCompleteCallback callback, void *pContext);

/* ---------------------------------------------------------------------------
    \fn measStopTimers
    \brief This function stops all the timers related to inNav measurements
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measStopTimers(tHalHandle hHal);

/* ---------------------------------------------------------------------------
    \fn sme_MeasHandleInNavMeasRsp
    \brief This function processes the measurement response obtained from the PE
    \param pMsg - Pointer to the pSirSmeInNavMeasRsp
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus sme_MeasHandleInNavMeasRsp(tHalHandle hHal, tANI_U8*);

/* ---------------------------------------------------------------------------
    \fn measIsInNavAllowed
    \brief This function checks if InNav measurements can be performed in the 
           current driver state
    \return eHalStatus     
  -------------------------------------------------------------------------------*/
eHalStatus measIsInNavAllowed(tHalHandle hHal);

#endif //__MEAS_API_H__

#endif //FEATURE_INNAV_SUPPORT
