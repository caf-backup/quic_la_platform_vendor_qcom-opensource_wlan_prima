#ifdef FEATURE_INNAV_SUPPORT

/*================================================================================ 
    \file wlan_hdd_innav.c
  
    \brief Linux Wireless Extensions for innav measurements
  
    $Id: wlan_hdd_innav.c,v 1.34 2010/04/15 01:49:23 -- VINAY
  
    Copyright (C) Qualcomm Inc.
    
================================================================================*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>

/*---------------------------------------------------------------------------------------------

  \brief hdd_InNavMeasurementRequestCallback() - 

  The sme module calls this function once a measurement particular measurement is over in the
  given measurement set. This function also reports the results to the user space

  \param - dev  - Pointer to the net device
     - info - Pointer to the iw_innav_measurement_request
         - wrqu - Pointer to the iwreq data
     - extra - Pointer to the data

  \return - 0 for success, non zero for failure

-----------------------------------------------------------------------------------------------*/
static eHalStatus hdd_InNavMeasurementRequestCallback(tHalHandle hHal, 
        void *pContext,
        tANI_U32 measurementSetID,
        eMeasMeasurementStatus measurementStatus)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    struct net_device *dev = (struct net_device *) pContext;
    union iwreq_data wrqu;
    char buffer[IW_CUSTOM_MAX+1];

    memset(&wrqu, '\0', sizeof(wrqu));
    memset(buffer, '\0', sizeof(buffer));

    //now if the status is success, then send an event up
    //so that the application can request for the data
    //else no need to send the event up
    if(measurementStatus == eMEAS_MEASUREMENT_FAILURE)
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: INNAV-MEAS-FAILED");
        hddLog(LOGW, "%s: measurement %d failed\n", __FUNCTION__, measurementSetID);
    }
    else if(measurementStatus == eMEAS_MEASUREMENT_INVALID_MODE)
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: INNAV-MEAS-INVALID-MODE");
        hddLog(LOGW, "%s: measurement %d failed because the driver is in invalid mode (IBSS|BTAMP|AP)\n", __FUNCTION__, measurementSetID);
    }
    else
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: INNAV-MEAS-SUCCESS");
        //everything went alright
    }
    
    wrqu.data.pointer = buffer;
    wrqu.data.length = strlen(buffer);
        
    wireless_send_event(dev, IWEVCUSTOM, &wrqu, buffer);

    return status;
}

/**--------------------------------------------------------------------------------------------

  \brief iw_get_innav_measurement() - 

  This function gets the measurement response for InNav functionality. This invokes
  the respective sme functionality. Function for handling the get measurement 
  IOCTL for the innav measurement response

  \param - dev  - Pointer to the net device
         - info - Pointer to the iw_innav_measurement_request
         - wrqu - Pointer to the iwreq data
         - extra - Pointer to the data

  \return - 0 for success, non zero for failure

-----------------------------------------------------------------------------------------------*/
int iw_get_innav_measurements(
        struct net_device *dev, 
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
    eHalStatus                            status = eHAL_STATUS_SUCCESS;
    struct iw_innav_measurement_response* pHddMeasurementRsp;
    tInNavMeasurementResponse*            pSmeMeasRsp;

    hdd_adapter_t *pAdapter = (netdev_priv(dev));

    //Now we need to generate an event from here.
    //So that the application can request for 
    //the measurement data

    tSirRttRssiResults*      pRslts_in = NULL;
    unsigned char*           buf_in = NULL;
    struct iw_innav_results* pRslts_out = NULL;
    unsigned char*           buf_out = NULL;
    
    unsigned int             bssidCnt = 0;
    unsigned int             dataCnt = 0;

    do
    {
        //get the measurements from sme
        status = sme_getInNavMeasurementResult(WLAN_HDD_GET_HAL_CTX(pAdapter), &pSmeMeasRsp);
        if(status != eHAL_STATUS_SUCCESS)
        {
            hddLog(LOGE, "%s: failed in sme_getInNavMeasurementResult\n", __FUNCTION__);
            break;
        }
        else
        {
            if(pSmeMeasRsp != NULL)
            {
                pHddMeasurementRsp = (struct iw_innav_measurement_response*)(extra);

                pHddMeasurementRsp->numBSSIDs = pSmeMeasRsp->numBSSIDs;

                pRslts_out = &pHddMeasurementRsp->perBssidResultData[0];
                buf_out = (unsigned char*)(pRslts_out);

                pRslts_in = &pSmeMeasRsp->rttRssiResults[0];
                buf_in = (unsigned char*)pRslts_in;

                for(bssidCnt=0;bssidCnt<pHddMeasurementRsp->numBSSIDs;bssidCnt++)
                {
                    pRslts_out->bssid[0] = pRslts_in->bssid[0];
                    pRslts_out->bssid[1] = pRslts_in->bssid[1];
                    pRslts_out->bssid[2] = pRslts_in->bssid[2];
                    pRslts_out->bssid[3] = pRslts_in->bssid[3];
                    pRslts_out->bssid[4] = pRslts_in->bssid[4];
                    pRslts_out->bssid[5] = pRslts_in->bssid[5];

                    hddLog(LOG4, "INNAV:HDD RSP BSSID[%02d]:%02X:%02X:%02X:%02X:%02X:%02X\n",
                    bssidCnt,
                    pRslts_out->bssid[0],
                    pRslts_out->bssid[1],
                    pRslts_out->bssid[2],
                    pRslts_out->bssid[3],
                    pRslts_out->bssid[4],
                    pRslts_out->bssid[5]);

                    pRslts_out->numSuccessfulMeasurements = pRslts_in->numSuccessfulMeasurements;

                    hddLog(LOG4, "numSuccesfulMeas = %d\n",pRslts_out->numSuccessfulMeasurements);

                    for(dataCnt=0;dataCnt<pRslts_out->numSuccessfulMeasurements;dataCnt++)
                    {
                        pRslts_out->rttRssiSnrTimeData[dataCnt].rssi = 
                                            pRslts_in->rttRssiTimeData[dataCnt].rssi;
                        pRslts_out->rttRssiSnrTimeData[dataCnt].rtt = 
                                            pRslts_in->rttRssiTimeData[dataCnt].rtt;
                        pRslts_out->rttRssiSnrTimeData[dataCnt].snr = 
                                            pRslts_in->rttRssiTimeData[dataCnt].snr;
                        pRslts_out->rttRssiSnrTimeData[dataCnt].measurementTimeLo = 
                                            pRslts_in->rttRssiTimeData[dataCnt].measurementTime;
                        pRslts_out->rttRssiSnrTimeData[dataCnt].measurementTimeHi = 
                                            pRslts_in->rttRssiTimeData[dataCnt].measurementTimeHi;
                        hddLog(LOG4, "INNAV HDD bssid num %d Iteration %d RSSI = %d RTT = %d \n ",
                                           bssidCnt,dataCnt, 
                                                 pRslts_out->rttRssiSnrTimeData[dataCnt].rssi,
                                                             pRslts_out->rttRssiSnrTimeData[dataCnt].rtt);
                    }

                    buf_out += sizeof(struct iw_innav_results)
                                + sizeof(struct iw_innav_rtt_rssi_snr_data)*(pRslts_out->numSuccessfulMeasurements-1);
                    pRslts_out = (struct iw_innav_results*)(buf_out);

                    buf_in += sizeof(tSirRttRssiResults)
                                + sizeof(tSirRttRssiTimeData)*(pRslts_in->numSuccessfulMeasurements - 1);
                    pRslts_in = (tSirRttRssiResults*)(buf_in);
                }
            }
            else
            {
                hddLog(LOGE, "%s: pMeasRsp = NULL\n", __FUNCTION__);
                status = eHAL_STATUS_FAILURE;
                break;
            }
        }
    } while(0);

    return eHAL_STATUS_SUCCESS;
}

/**--------------------------------------------------------------------------------------------

  \brief iw_set_innav_measurement() - 

  This function sets the measurement configuration for InNav functionality. This invokes
  the respective sme Measurement functionality. Function for handling the set measurement 
  IOCTL for the innav measurement configuration

  \param - dev  - Pointer to the net device
     - info - Pointer to the iw_innav_measurement_request
         - wrqu - Pointer to the iwreq data
     - extra - Pointer to the data

  \return - 0 for success, non zero for failure

-----------------------------------------------------------------------------------------------*/
int iw_set_innav_measurements(
        struct net_device *dev, 
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
    v_U8_t bssidCount = 0;

    eHalStatus status = eHAL_STATUS_SUCCESS;
    struct iw_innav_measurement_request *measurementReq = NULL;
    tInNavMeasurementConfig measConfig;
    v_U32_t measurementSetID = 0;

    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

    do
    {
        if(NULL != wrqu->data.pointer)
        {
            measurementReq = (struct iw_innav_measurement_request *)wrqu->data.pointer;
        }

        if(measurementReq == NULL)
        {
            hddLog(LOGE, "in %s measurementReq == NULL\n", __FUNCTION__);
            status = eHAL_STATUS_FAILURE;
            break;
        }
    
        hddLog(LOG4, "INNAV:HDD: #bssids       %u\n", measurementReq->numBSSIDs);
        hddLog(LOG4, "INNAV:HDD: #measurements %u\n", measurementReq->numInNavMeasurements);
        hddLog(LOG4, "INNAV:HDD: #repetitions  %u\n", measurementReq->numSetRepetitions);
        hddLog(LOG4, "INNAV:HDD: time-interval %u\n", measurementReq->measurementTimeInterval);

        /* Perform the basic checks for the incoming requests */
        if(measurementReq->numBSSIDs < 1)
        {
            hddLog(LOGE, "number of bssids for measurement < 1\n");
            status = eHAL_STATUS_FAILURE;
            break;
        }
        if(measurementReq->numBSSIDs > MAX_BSSIDS_ALLOWED_FOR_MEASUREMENTS)
        {
            hddLog(LOGE, "number of bssids for measurement > 10\n");
            status = eHAL_STATUS_FAILURE;
            break;
        }
        if(measurementReq->numInNavMeasurements > MAX_MEASUREMENTS_ALLOWED_PER_BSSID)
        {
            hddLog(LOGE, "number of rtt rssi measurements > 8 ... Current FW might not be able to support it\n");
            status = eHAL_STATUS_FAILURE;
            break;
        }
        if(measurementReq->measurementMode != RTS_CTS_MODE)
        {
            hddLog(LOGE, "Mode currently not supported\n");
            status = eHAL_STATUS_FAILURE;
            break;
        }
        if(measurementReq->measurementTimeInterval < 500)
        {
            hddLog(LOGE, "The measurement time-interval is in ms, and has to be >= 500ms\n");
            status = eHAL_STATUS_FAILURE;
            break;
        }

        for(bssidCount=0;bssidCount<measurementReq->numBSSIDs;bssidCount++)
        {
            hddLog(LOG4, "INNAV:HDD BSSID[%02d]:%02X:%02X:%02X:%02X:%02X:%02X\n", 
                bssidCount,
                measurementReq->bssidChannelInfo[bssidCount].bssid[0],
                measurementReq->bssidChannelInfo[bssidCount].bssid[1],
                measurementReq->bssidChannelInfo[bssidCount].bssid[2],
                measurementReq->bssidChannelInfo[bssidCount].bssid[3],
                measurementReq->bssidChannelInfo[bssidCount].bssid[4],
                measurementReq->bssidChannelInfo[bssidCount].bssid[5]);
        }

        vos_mem_zero(&measConfig, sizeof(tInNavMeasurementRequest));
    
        //Grab all the meta data information before grabbing the channel and BSSID info
        measConfig.numBSSIDs = measurementReq->numBSSIDs;
        measConfig.numInNavMeasurements = measurementReq->numInNavMeasurements;
        measConfig.numSetRepetitions = measurementReq->numSetRepetitions;
        measConfig.measurementTimeInterval = measurementReq->measurementTimeInterval;
        measConfig.measurementMode = (tANI_U8)measurementReq->measurementMode;
    
        //Now create the memory for channel and BSSID info based on the numBSSIDs
        if(measConfig.numBSSIDs > 0)
        {
            measConfig.measBSSIDChannelInfo = (tMeasurementBSSIDChannelInfo*)
                                                vos_mem_malloc(sizeof(tMeasurementBSSIDChannelInfo) * 
                                                measConfig.numBSSIDs);
        }
    
        //Memory is created... Now fill up the information for each BSSID
        for(bssidCount=0;bssidCount<measConfig.numBSSIDs;bssidCount++)
        {
            //Check if this is the right way to copy -- CHECK
            vos_mem_copy(measConfig.measBSSIDChannelInfo+bssidCount, 
                        measurementReq->bssidChannelInfo+bssidCount, 
                        sizeof(tMeasurementBSSIDChannelInfo));
        }
    
        status = sme_InNavMeasurementRequest(WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                                pAdapter->sessionId,
                                                &measConfig, 
                                                &measurementSetID, 
                                                &hdd_InNavMeasurementRequestCallback, 
                                                dev);
    
        pwextBuf->inNavMeasurementID = measurementSetID;
        pwextBuf->inNavMeasurementInProgress = TRUE;

    } while(0);
    
    return status;
}


#endif
