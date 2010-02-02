/**========================================================================
  
  \file  wlan_hdd_scan.c

  \brief WLAN Host Device Driver implementation
               
   Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.
   
   Qualcomm Confidential and Proprietary.
  
  ========================================================================*/

/**========================================================================= 

                       EDIT HISTORY FOR FILE 
   
   
  This section contains comments describing changes made to the module. 
  Notice that changes are listed in reverse chronological order. 
   
   
  $Header:$   $DateTime: $ $Author: $ 
   
   
  when        who    what, where, why 
  --------    ---    --------------------------------------------------------
  04/5/09     Shailender     Created module. 

  ==========================================================================*/
      /* To extract the Scan results */

/* Add a stream event */

#include <wlan_qct_driver.h>
#include <wlan_hdd_includes.h>
#include <vos_api.h>
#include <palTypes.h>
#include <aniGlobal.h>
#include <dot11f.h>

#define GET_IE_LEN_IN_BSS(lenInBss) ( lenInBss + sizeof(lenInBss) - \
              ((int) OFFSET_OF( tSirBssDescription, ieFields)))

typedef struct hdd_scan_info{
    struct net_device *dev;
    struct iw_request_info *info;
    char *start;
    char *end;
} hdd_scan_info_t, *hdd_scan_info_tp;

static v_S31_t hdd_TranslateABGRateToMbpsRate(v_U8_t *pFcRate)   
{

    /** Slightly more sophisticated processing has to take place here.
              Basic rates are rounded DOWN.  HT rates are rounded UP.*/
    return ( (( ((v_S31_t) *pFcRate) & 0x007f) * 1000000) / 2);
}


static eHalStatus hdd_AddIwStreamEvent(int cmd, int length, char* data, hdd_scan_info_t *pscanInfo, char **last_event, char **current_event )
{
    struct iw_event event;

    *last_event = *current_event;
    vos_mem_zero(&event, sizeof (struct iw_event));
    event.cmd = cmd;
    event.u.data.flags = 1;
    event.u.data.length = length;
    *current_event = iwe_stream_add_point (pscanInfo->info,*current_event, pscanInfo->end,  &event, data);

    if(*last_event == *current_event)
    {      
            /* no space to add event */
        return -E2BIG; /* Error code, may be E2BIG */
    }

    return 0;
}

/**---------------------------------------------------------------------------

  \brief hdd_GetWPARSNIEs() - 

   This function extract the WPA/RSN IE from the Bss descriptor IEs fields

  \param  - ieFields - Pointer to the Bss Descriptor IEs.
              - ie_length - IE Length.
              - last_event -Points to the last event.
              - current_event - Points to the 
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/


/* Extract the WPA and/or RSN IEs */
static eHalStatus hdd_GetWPARSNIEs( v_U8_t *ieFields, v_U16_t ie_length, char **last_event, char **current_event, hdd_scan_info_t *pscanInfo )
{
    v_U8_t eid, elen, *element;
    v_U16_t tie_length=0;

    ENTER();

    element = ieFields;
    tie_length = ie_length;

    while( tie_length > 2 && element != NULL )
    {
        eid = element[0];
        elen = element[1];
        switch(eid)
        {
            case DOT11F_EID_WPA:
            case DOT11F_EID_RSN:
                if(hdd_AddIwStreamEvent( IWEVGENIE,  elen+2, (char*)element, pscanInfo, last_event, current_event ) < 0 )
                    return -E2BIG;
                break;

            default:
                break;
        }

        /* Next element */
        tie_length -= (2 + elen);
        element += 2 + elen;
    }

    return 0;
}

/**---------------------------------------------------------------------------

  \brief hdd_IndicateScanResult() - 

   This function returns the scan results to the wpa_supplicant

  \param  - scanInfo - Pointer to the scan info structure.
              - descriptor - Pointer to the Bss Descriptor.
              
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/      
#define MAX_CUSTOM_LEN 64
static eHalStatus hdd_IndicateScanResult(hdd_scan_info_t *scanInfo,
                                 tSirBssDescription *descriptor)
{
   hdd_adapter_t *pAdapter = (netdev_priv(scanInfo->dev));
   tHalHandle hHal = pAdapter->hHal;
   struct iw_event event;
   char *current_event = scanInfo->start;
   char *end = scanInfo->end;
   char *last_event;
   char *current_pad;
   v_U16_t ie_length = 0;
   v_U16_t capabilityInfo;
   char *modestr;
   int error;
   char custom[MAX_CUSTOM_LEN];
   char *p;
 
   hddLog( LOG1, "hdd_IndicateScanResult %02x:%02x:%02x:%02x:%02x:%02x\n",
          descriptor->bssId[0],
          descriptor->bssId[1],
          descriptor->bssId[2],
          descriptor->bssId[3],
          descriptor->bssId[4],
          descriptor->bssId[5]);
 
   error = 0;
   last_event = current_event;
   vos_mem_zero(&event, sizeof (event));
   
   /* BSSID */
   event.cmd = SIOCGIWAP; 
   event.u.ap_addr.sa_family = ARPHRD_ETHER;
   vos_mem_copy (event.u.ap_addr.sa_data, descriptor->bssId, 
                  sizeof (descriptor->bssId));
   current_event = iwe_stream_add_event(scanInfo->info,current_event, end, 
                   &event, IW_EV_ADDR_LEN);
 
   if (last_event == current_event)
   { 
      /* no space to add event */
      /* Error code may be E2BIG */
       hddLog( LOGE, "hdd_IndicateScanResult: no space for SIOCGIWAP \n");
       return -E2BIG; 
   }
 
   last_event = current_event;
   vos_mem_zero(&event, sizeof (struct iw_event));

 /* Protocol Name */
   event.cmd = SIOCGIWNAME; 
 
   switch (descriptor->nwType)
   {
   case eSIR_11A_NW_TYPE:
       modestr = "a";
       break;
   case eSIR_11B_NW_TYPE:
       modestr = "b";
       break;
   case eSIR_11G_NW_TYPE:
       modestr = "g";
       break;
   case eSIR_11N_NW_TYPE:
       modestr = "n";
       break;
   default:
       hddLog( LOG1, "%s: Unknown network type [%d]\n",
              __FUNCTION__, descriptor->nwType);
       modestr = "?";
       break;
   }
   snprintf(event.u.name, IFNAMSIZ, "IEEE 802.11%s", modestr);
   current_event = iwe_stream_add_event(scanInfo->info,current_event, end, 
                   &event, IW_EV_CHAR_LEN);
 
   if (last_event == current_event)
   { /* no space to add event */
       hddLog( LOG1, "hdd_IndicateScanResult: no space for SIOCGIWNAME\n");
      /* Error code, may be E2BIG */
       return -E2BIG; 
   }
 
   last_event = current_event;
   vos_mem_zero( &event, sizeof (struct iw_event));
   
   /*Freq*/
   event.cmd = SIOCGIWFREQ;
 
   event.u.freq.m = descriptor->channelId;
   event.u.freq.e = 0;
   event.u.freq.i = 0;
   current_event = iwe_stream_add_event(scanInfo->info,current_event, end,
                                        &event, IW_EV_FREQ_LEN);
 
   if (last_event == current_event)
   { /* no space to add event */
       return -E2BIG;
   }
 
   last_event = current_event;
   vos_mem_zero( &event, sizeof (struct iw_event));

   /* BSS Mode */
   event.cmd = SIOCGIWMODE;
 
   capabilityInfo = descriptor->capabilityInfo;
 
   if (SIR_MAC_GET_ESS(capabilityInfo))
   {
       event.u.mode = IW_MODE_INFRA;
   }
   else if (SIR_MAC_GET_IBSS(capabilityInfo))
   {
       event.u.mode = IW_MODE_ADHOC;
   }
   else
   {
       /* neither ESS or IBSS */
       event.u.mode = IW_MODE_AUTO;
   }
 
   current_event = iwe_stream_add_event(scanInfo->info,current_event, end, 
                                        &event, IW_EV_UINT_LEN);
 
   if (last_event == current_event)
   { /* no space to add event */
       hddLog( LOGE, "hdd_IndicateScanResult: no space for SIOCGIWMODE\n");
       return -E2BIG; 
   }
   /* To extract SSID */
   ie_length = GET_IE_LEN_IN_BSS( descriptor->length );
 
   if (ie_length > 0)
   {
       tDot11fBeaconIEs dot11BeaconIEs; 
       tDot11fIESSID *pDot11SSID;
       tDot11fIESuppRates *pDot11SuppRates;
       tDot11fIEExtSuppRates *pDot11ExtSuppRates;
       tDot11fIEHTCaps *pDot11IEHTCaps;
       int numBasicRates = 0;
       int maxNumRates = 0;
 
       pDot11IEHTCaps = NULL;
 
       dot11fUnpackBeaconIEs ((tpAniSirGlobal) 
           hHal, (tANI_U8 *) descriptor->ieFields, ie_length,  &dot11BeaconIEs);
 
       pDot11SSID = &dot11BeaconIEs.SSID; 
 
 
       if (pDot11SSID->present ) {    
          last_event = current_event;
          vos_mem_zero (&event, sizeof (struct iw_event));
 
          event.cmd = SIOCGIWESSID;
          event.u.data.flags = 1;
          event.u.data.length = pDot11SSID->num_ssid;
          current_event = iwe_stream_add_point (scanInfo->info,current_event, end, 
                  &event, (char *)pDot11SSID->ssid);
 
          if(last_event == current_event)
          { /* no space to add event */
             hddLog( LOGE, "hdd_IndicateScanResult: no space for SIOCGIWESSID\n");
             return -E2BIG; 
          }
       }

      if( hdd_GetWPARSNIEs( ( tANI_U8 *) descriptor->ieFields, ie_length, &last_event, &current_event, scanInfo )  < 0    )
      {
          hddLog( LOGE, "hdd_IndicateScanResult: no space for SIOCGIWESSID\n");
          return -E2BIG;
      }

      last_event = current_event;
      current_pad = current_event + IW_EV_LCP_LEN;
      vos_mem_zero( &event, sizeof (struct iw_event));
      
      /*Rates*/
      event.cmd = SIOCGIWRATE;


      pDot11SuppRates = &dot11BeaconIEs.SuppRates;

      if (pDot11SuppRates->present ) 
      {
          int i;

          numBasicRates = pDot11SuppRates->num_rates;;
          for (i=0; i<pDot11SuppRates->num_rates; i++) 
          {
              if (0 != (pDot11SuppRates->rates[i] & 0x7F))
              {
                  event.u.bitrate.value = hdd_TranslateABGRateToMbpsRate (
                      &pDot11SuppRates->rates[i]);

                  current_pad = iwe_stream_add_value (scanInfo->info,current_event,
                      current_pad, end, &event, IW_EV_PARAM_LEN);
              }
          }
               
      }

      pDot11ExtSuppRates = &dot11BeaconIEs.ExtSuppRates;

      if (pDot11ExtSuppRates->present ) 
      {   
          int i;

          maxNumRates = numBasicRates + pDot11ExtSuppRates->num_rates;    
          
          /* Check to make sure the total number of rates 
               doesn't exceed IW_MAX_BITRATES */
               
          maxNumRates = VOS_MIN(maxNumRates , IW_MAX_BITRATES); 
              
          for ( i=0; i< (maxNumRates - numBasicRates) ; i++ ) 
          {
              if (0 != (pDot11ExtSuppRates->rates[i] & 0x7F))
              {
                  event.u.bitrate.value = hdd_TranslateABGRateToMbpsRate (
                      &pDot11ExtSuppRates->rates[i]);

                  current_pad = iwe_stream_add_value (scanInfo->info,current_event,
                      current_pad, end, &event, IW_EV_PARAM_LEN);
              }
          }
      }
      
      
      if ((current_pad - current_event) > IW_EV_LCP_LEN) 
      {
          current_event = current_pad;
      } 
      else 
      {
          if (last_event == current_event)
          { /* no space to add event */
              hddLog( LOGE, "hdd_IndicateScanResult: no space for SIOCGIWRATE\n");
              return -E2BIG;
          }
      }
      
      last_event = current_event;
      vos_mem_zero (&event, sizeof (struct iw_event));

      
      event.cmd = SIOCGIWENCODE;
 
      if (SIR_MAC_GET_PRIVACY(capabilityInfo))
      {
         event.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
      }
      else
      {
         event.u.data.flags = IW_ENCODE_DISABLED;
      }
      event.u.data.length = 0;

      current_event = iwe_stream_add_point(scanInfo->info,current_event, end, &event, (char *)pDot11SSID->ssid);                      
             
       
      if(last_event == current_event)
      { /* no space to add event 
               Error code, may be E2BIG */
          return -E2BIG; 
      }
   }
  
   last_event = current_event;
   vos_mem_zero( &event, sizeof (struct iw_event));

    /*RSSI*/
   event.cmd = IWEVQUAL;
   event.u.qual.qual = descriptor->rssi;
   event.u.qual.noise = descriptor->sinr;
   event.u.qual.level = VOS_MIN ((descriptor->rssi + descriptor->sinr), 0);
   event.u.qual.updated = IW_QUAL_ALL_UPDATED;
  
   current_event = iwe_stream_add_event(scanInfo->info,current_event,
       end, &event, IW_EV_QUAL_LEN);
 
   if(last_event == current_event)
   { /* no space to add event */
       hddLog( LOGE, "hdd_IndicateScanResult: no space for IWEVQUAL\n");
       return -E2BIG; 
   }
 

   /* AGE */
   event.cmd = IWEVCUSTOM;
   p = custom;
   p += snprintf(p, MAX_CUSTOM_LEN, " Age: %lu",
                 vos_timer_get_system_ticks() - descriptor->nReceivedTime);
   event.u.data.length = p - custom;
   current_event = iwe_stream_add_point (scanInfo->info,current_event, end,
                                         &event, custom);
   if(last_event == current_event)
   { /* no space to add event */
      hddLog( LOGE, "hdd_IndicateScanResult: no space for IWEVCUSTOM (age)\n");
      return -E2BIG;
   }

   scanInfo->start = current_event;
   
   return 0;
}

/**---------------------------------------------------------------------------
  
  \brief hdd_ScanRequestCallback() - 

   The sme module calls this callback function once it finish the scan request
   and this function notifies the scan complete event to the wpa_supplicant.

  \param  - halHandle - Pointer to the Hal Handle.
              - pContext - Pointer to the data context.
              - scanId - Scan ID.
              - status - CSR Status.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/

static eHalStatus hdd_ScanRequestCallback(tHalHandle halHandle, void *pContext,
                         tANI_U32 scanId, eCsrScanStatus status)
{
    struct net_device *dev = (struct net_device *) pContext;
    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    hdd_wext_state_t *pwextBuf = pAdapter->pWextState;
    union iwreq_data wrqu;
    int we_event;
    char *msg;
    VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
    ENTER();

   hddLog(LOG1,"%s called with halHandle = %p, pContext = %p, scanID = %d,"
           " returned status = %d\n", __FUNCTION__, halHandle, pContext,
            (int) scanId, (int) status);

    /* Check the scanId */
    if (pwextBuf->scanId != scanId)
    {
        hddLog(LOG1,"%s called with mismatched scanId pWextState->scanId = %d "
               "scanId = %d \n", __FUNCTION__, (int) pwextBuf->scanId,
                (int) scanId);
    }

    /* Scan is no longer pending */
    pwextBuf->mScanPending = VOS_FALSE;
   
    // notify any applications that may be interested
    memset(&wrqu, '\0', sizeof(wrqu));
    we_event = SIOCGIWSCAN;
    msg = NULL;
    wireless_send_event(dev, we_event, &wrqu, msg);

    vos_status = vos_event_set(&pwextBuf->vosevent);
   
    if (!VOS_IS_STATUS_SUCCESS(vos_status))
    {    
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos_event_set failed!!\n"));
       return VOS_STATUS_E_FAILURE;
    }

    EXIT();

    return eHAL_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------
  
  \brief iw_set_scan() - 

   This function process the scan request from the wpa_supplicant
   and set the scan request to the SME  

  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/


int iw_set_scan(struct net_device *dev, struct iw_request_info *info,
                 union iwreq_data *wrqu, char *extra)
{
   VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
   hdd_adapter_t *pAdapter = (netdev_priv(dev));
   hdd_wext_state_t *pwextBuf = pAdapter->pWextState;
   tCsrScanRequest scanRequest;
   v_U32_t scanId = 0;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   struct iw_scan_req *scanReq = (struct iw_scan_req *)extra;

   ENTER();

   if(pwextBuf->mScanPending == TRUE)
   {
       hddLog(LOG1,"%s: mScanPending is TRUE\n",__func__);
       return -EBUSY;                  
   }
   
   vos_mem_zero( &scanRequest, sizeof(scanRequest));
 
   if (NULL != wrqu->data.pointer)
   {      
       /* set scanType, active or passive */
      
       if ((IW_SCAN_TYPE_ACTIVE ==  scanReq->scan_type) || (eSIR_ACTIVE_SCAN == pAdapter->pWextState->scan_mode))
       {
           scanRequest.scanType = eSIR_ACTIVE_SCAN;
       }
       else
       {
           scanRequest.scanType = eSIR_PASSIVE_SCAN;
       }
 
       /* set bssid using sockaddr from iw_scan_req */
       vos_mem_copy(scanRequest.bssid,
                       &scanReq->bssid.sa_data, sizeof(scanRequest.bssid) );
      
      if (wrqu->data.flags & IW_SCAN_THIS_ESSID)  {

          if(scanReq->essid_len) {
              scanRequest.SSIDs.numOfSSIDs = 1;
              scanRequest.SSIDs.SSIDList =( tCsrSSIDInfo *)vos_mem_malloc(sizeof(tCsrSSIDInfo));
              scanRequest.SSIDs.SSIDList->SSID.length = scanReq->essid_len;
              vos_mem_copy(scanRequest.SSIDs.SSIDList->SSID.ssId,scanReq->essid,scanReq->essid_len);
          }
      }
 
       /* set min and max channel time */
       scanRequest.minChnTime = scanReq->min_channel_time;
       scanRequest.maxChnTime = scanReq->max_channel_time;
 
   }
   else
   {
       if(pAdapter->pWextState->scan_mode == eSIR_ACTIVE_SCAN) {
           /* set the scan type to active */
           scanRequest.scanType = eSIR_ACTIVE_SCAN;
       } else {                      
           scanRequest.scanType = eSIR_PASSIVE_SCAN;
       }
 
       vos_mem_set( scanRequest.bssid, sizeof( tCsrBssid ), 0xff );
       
       /* set min and max channel time to zero */
       scanRequest.minChnTime = 0;
       scanRequest.maxChnTime = 0;
   }
   
   /* set BSSType to default type */
   scanRequest.BSSType = eCSR_BSS_TYPE_ANY;
 
   /*Scan all the channels */
   scanRequest.ChannelInfo.numOfChannels = 0;

   scanRequest.ChannelInfo.ChannelList = NULL;
 
   /* set requestType to full scan */
   scanRequest.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
   
   pwextBuf->mScanPending = TRUE;
   
   status = sme_ScanRequest( pAdapter->hHal, &scanRequest, &scanId, &hdd_ScanRequestCallback, dev ); 
      
   pwextBuf->scanId = scanId;

   vos_status = vos_wait_single_event(&pwextBuf->vosevent,3000);
   
   if (!VOS_IS_STATUS_SUCCESS(vos_status))
   {
      pwextBuf->mScanPending = FALSE;
      return VOS_STATUS_E_FAILURE;
   }
   if ((wrqu->data.flags & IW_SCAN_THIS_ESSID) && (scanReq->essid_len))
       vos_mem_free(scanRequest.SSIDs.SSIDList);

   EXIT();
   return status;
}

/**---------------------------------------------------------------------------
  
  \brief iw_set_scan() - 

   This function returns the scan results to the wpa_supplicant

  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/

  
int iw_get_scan(struct net_device *dev, 
                         struct iw_request_info *info,
                         union iwreq_data *wrqu, char *extra)
{
   hdd_adapter_t *pAdapter = (netdev_priv(dev));
   hdd_wext_state_t *pwextBuf = pAdapter->pWextState;
   tHalHandle hHal = pAdapter->hHal;
   tCsrScanResultInfo *pScanResult;
   eHalStatus status = eHAL_STATUS_SUCCESS;
   hdd_scan_info_t scanInfo;
   tScanResultHandle pResult;
 
   ENTER();

   if (TRUE == pwextBuf->mScanPending)
   {
       hddLog(LOG1,"iw_get_scan: mScanPending \n");
       return -EAGAIN;
   }
 
   scanInfo.dev = dev;
   scanInfo.start = extra;
   scanInfo.info = info;
   
   if (0 == wrqu->data.length)
   {
       scanInfo.end = extra + IW_SCAN_MAX_DATA;
   }
   else
   {
       scanInfo.end = extra + wrqu->data.length;
   }
 
   status = sme_ScanGetResult(hHal,NULL,&pResult);
 
   if (NULL == pResult)
   {
       // no scan results
       hddLog(LOG1,"iw_get_scan: NULL Scan Result \n");
       return 0;
   }
 
   pScanResult = sme_ScanResultGetFirst(hHal, pResult);
 
   while (pScanResult)
   {
       status = hdd_IndicateScanResult(&scanInfo,&pScanResult->BssDescriptor);
       if (0 != status)
       {
           break;
       }
       pScanResult = sme_ScanResultGetNext(hHal, pResult);
   }
 
   sme_ScanResultPurge(hHal, pResult); 

   EXIT();
   return status;
}
                                                 
