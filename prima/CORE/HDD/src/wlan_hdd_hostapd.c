/**========================================================================
  
  \file  wlan_hdd_hostapd.c
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
  06/03/10    js - Added support to hostapd driven deauth/disassoc/mic failure
  ==========================================================================*/
/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
   
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <linux/semaphore.h>
#include <vos_api.h>
#include <vos_sched.h>
#include <linux/etherdevice.h>
#include <wlan_hdd_includes.h>
#include <qc_sap_ioctl.h>
#include <wlan_hdd_hostapd.h>
#include <sapApi.h>
#include <sapInternal.h>
#include <wlan_qct_tl.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <wlan_hdd_main.h>
#include <linux/netdevice.h>
#include <linux/mmc/sdio_func.h>


#define	IS_UP(_dev) \
	(((_dev)->flags & (IFF_RUNNING|IFF_UP)) == (IFF_RUNNING|IFF_UP))
#define	IS_UP_AUTO(_ic) \
	(IS_UP((_ic)->ic_dev) && (_ic)->ic_roaming == IEEE80211_ROAMING_AUTO)
#define WE_WLAN_VERSION     1
extern int iw_get_char_setnone(struct net_device *dev, struct iw_request_info *info,
                       union iwreq_data *wrqu, char *extra);
/*--------------------------------------------------------------------------- 
 *   Function definitions
 *-------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------
  
  \brief hdd_hostapd_open() - HDD Open function for hostapd interface
  
  This is called in response to ifconfig up
  
  \param  - dev Pointer to net_device structure
  
  \return - 0 for success non-zero for failure
              
  --------------------------------------------------------------------------*/
int hdd_hostapd_open (struct net_device *dev)
{
   ENTER();
   printk("%s", __func__);
   //Turn ON carrier state
   netif_carrier_on(dev);
   //Enable all Tx queues  
   netif_tx_start_all_queues(dev);
   
   EXIT();
   return 0;
}
/**---------------------------------------------------------------------------
  
  \brief hdd_hostapd_stop() - HDD stop function for hostapd interface
  
  This is called in response to ifconfig down
  
  \param  - dev Pointer to net_device structure
  
  \return - 0 for success non-zero for failure
              
  --------------------------------------------------------------------------*/
int hdd_hostapd_stop (struct net_device *dev)
{
   printk("%s", __func__);
   //Stop all tx queues
   netif_tx_disable(dev);
   
   //Turn OFF carrier state
   netif_carrier_off(dev);
   return 0;
}
/**============================================================================
  @brief hdd_hostapd_hard_start_xmit() - Function registered with the Linux OS for 
  transmitting packets. There are 2 versions of this function. One that uses
  locked queue and other that uses lockless queues. Both have been retained to
  do some performance testing
  @param skb      : [in]  pointer to OS packet (sk_buff)
  @param dev      : [in] pointer to Libra network device
  
  @return         : NET_XMIT_DROP if packets are dropped
                  : NET_XMIT_SUCCESS if packet is enqueued succesfully
  ===========================================================================*/
int hdd_hostapd_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    return 0;	
}
int hdd_hostapd_change_mtu(struct net_device *dev, int new_mtu)
{
	return 0;
}

int hdd_hostapd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
   return 0;
}

/**---------------------------------------------------------------------------
  
  \brief hdd_hostapd_set_mac_address() - 
   This function sets the user specified mac address using 
   the command ifconfig wlanX hw ether <mac adress>.
   
  \param  - dev - Pointer to the net device.
              - addr - Pointer to the sockaddr.
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/

static int hdd_hostapd_set_mac_address(struct net_device *dev, void *addr)
{
   struct sockaddr *psta_mac_addr = addr;
   ENTER();
   memcpy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);
   EXIT();
   return 0;
}
void hdd_hostapd_inactivity_timer_cb(v_PVOID_t usrDataForCallback)
{
    struct net_device *dev = (struct net_device *)usrDataForCallback;
    v_BYTE_t we_custom_event[64];
    union iwreq_data wrqu;

    /* event_name space-delimiter driver_module_name */
    /* Format of the event is "AUTO-SHUT.indication" " " "module_name" */
    char * autoShutEvent = "AUTO-SHUT.indication" " "  KBUILD_MODNAME;
    int event_len = strlen(autoShutEvent) + 1; /* For the NULL at the end */

    ENTER();

    memset(&we_custom_event, '\0', sizeof(we_custom_event));
    memcpy(&we_custom_event, autoShutEvent, event_len);

    memset(&wrqu, 0, sizeof(wrqu));
    wrqu.data.length = event_len;

    wireless_send_event(dev, IWEVCUSTOM, &wrqu, (char *)we_custom_event);    

    EXIT();
}

VOS_STATUS hdd_hostapd_SAPEventCB( tpSap_Event pSapEvent, v_PVOID_t usrDataForCallback)
{
    hdd_adapter_t *pHostapdAdapter;
    hdd_ap_ctx_t *pHddApCtx;
    hdd_hostapd_state_t *pHostapdState;
    struct net_device *dev;
    eSapHddEvent sapEvent;
    union iwreq_data wrqu;
    v_BYTE_t *we_custom_event_generic = NULL;
    int we_event = 0;
    int i = 0;
    u_int16_t staId;
    VOS_STATUS vos_status; 
    v_BOOL_t bWPSState;
    v_BOOL_t bApActive = FALSE;
    tpSap_AssocMacAddr pAssocStasArray = NULL;


    dev = (struct net_device *)usrDataForCallback;
    pHostapdAdapter = netdev_priv(dev);
    pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter); 
    pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
    sapEvent = pSapEvent->sapHddEventCode;
    memset(&wrqu, '\0', sizeof(wrqu));

    switch(sapEvent)
    {
        case eSAP_START_BSS_EVENT :
            hddLog(LOGE, FL("BSS configured status = %s, channel = %lu, bc sta Id = %d\n"),
                            pSapEvent->sapevt.sapStartBssCompleteEvent.status ? "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS",
                            pSapEvent->sapevt.sapStartBssCompleteEvent.operatingChannel,
                              pSapEvent->sapevt.sapStartBssCompleteEvent.staId);

            pHostapdState->vosStatus = pSapEvent->sapevt.sapStartBssCompleteEvent.status;
            vos_status = vos_event_set(&pHostapdState->vosEvent);
   
            if (!VOS_IS_STATUS_SUCCESS(vos_status) || pHostapdState->vosStatus)
            {     
                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: startbss event failed!!\n"));
                goto stopbss;
            }
            else
            {                
                pHddApCtx->uBCStaId = pSapEvent->sapevt.sapStartBssCompleteEvent.staId;
                //@@@ need wep logic here to set privacy bit
                hdd_softap_Register_BC_STA(pHostapdAdapter, pHddApCtx->uPrivacy);
            }
            
            if (0 != (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->nAPAutoShutOff)
            {
                // AP Inactivity timer init and start
                vos_status = vos_timer_init( &pHddApCtx->hdd_ap_inactivity_timer, VOS_TIMER_TYPE_SW, 
                                            hdd_hostapd_inactivity_timer_cb, (v_PVOID_t)dev );
                if (!VOS_IS_STATUS_SUCCESS(vos_status))
                   hddLog(LOGE, FL("Failed to init AP inactivity timer\n"));

                vos_status = vos_timer_start( &pHddApCtx->hdd_ap_inactivity_timer, (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->nAPAutoShutOff * 1000);
                if (!VOS_IS_STATUS_SUCCESS(vos_status))
                   hddLog(LOGE, FL("Failed to init AP inactivity timer\n"));

            }
            pHostapdState->bssState = BSS_START;

            return VOS_STATUS_SUCCESS;
        case eSAP_STOP_BSS_EVENT:
            hddLog(LOGE, FL("BSS stop status = %s\n"),pSapEvent->sapevt.sapStopBssCompleteEvent.status ? 
                             "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");
            goto stopbss;
        case eSAP_STA_SET_KEY_EVENT:
            //TODO: forward the message to hostapd once implementtation is done for now just print
            hddLog(LOGE, FL("SET Key: configured status = %s\n"),pSapEvent->sapevt.sapStationSetKeyCompleteEvent.status ? 
                            "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");
            return VOS_STATUS_SUCCESS;
        case eSAP_STA_DEL_KEY_EVENT:
	       //TODO: forward the message to hostapd once implementtation is done for now just print
	       hddLog(LOGE, FL("Event received %s\n"),"eSAP_STA_DEL_KEY_EVENT");
           return VOS_STATUS_SUCCESS;
    	case eSAP_STA_MIC_FAILURE_EVENT:
        {
            struct iw_michaelmicfailure msg;
            memset(&msg, '\0', sizeof(msg));
            msg.src_addr.sa_family = ARPHRD_ETHER;
            memcpy(msg.src_addr.sa_data, &pSapEvent->sapevt.sapStationMICFailureEvent.staMac, sizeof(msg.src_addr.sa_data));
            hddLog(LOG1, "MIC MAC "MAC_ADDRESS_STR"\n", MAC_ADDR_ARRAY(msg.src_addr.sa_data));
            if(pSapEvent->sapevt.sapStationMICFailureEvent.multicast == eCSR_ROAM_RESULT_MIC_ERROR_GROUP)
             msg.flags = IW_MICFAILURE_GROUP;
            else 
             msg.flags = IW_MICFAILURE_PAIRWISE;
            memset(&wrqu, 0, sizeof(wrqu));
            wrqu.data.length = sizeof(msg);
            we_event = IWEVMICHAELMICFAILURE;
            we_custom_event_generic = (v_BYTE_t *)&msg;
        }
            break;
        
        case eSAP_STA_ASSOC_EVENT:
        case eSAP_STA_REASSOC_EVENT:
            wrqu.addr.sa_family = ARPHRD_ETHER;
            memcpy(wrqu.addr.sa_data, &pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.staMac, 
                sizeof(wrqu.addr.sa_data));
            hddLog(LOG1, " associated "MAC_ADDRESS_STR"\n", MAC_ADDR_ARRAY(wrqu.addr.sa_data));
            we_event = IWEVREGISTERED;
            
            WLANSAP_Get_WPS_State((WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext, &bWPSState);
            
            if (pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.iesLen || bWPSState == eANI_BOOLEAN_TRUE )
            {
                hdd_softap_RegisterSTA( pHostapdAdapter,
                                       TRUE,
                                       pHddApCtx->uPrivacy,
                                       pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.staId,
                                       0,
                                       0,
                                       (v_MACADDR_t *)wrqu.addr.sa_data,
                                       pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.wmmEnabled);
            }
            else
            {
                hdd_softap_RegisterSTA( pHostapdAdapter,
                                       FALSE,
                                       pHddApCtx->uPrivacy,
                                       pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.staId,
                                       0,
                                       0,
                                       (v_MACADDR_t *)wrqu.addr.sa_data,
                                       pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.wmmEnabled);
            } 
            
            // Stop AP inactivity timer
            if (pHddApCtx->hdd_ap_inactivity_timer.state == VOS_TIMER_STATE_RUNNING)
            {
                vos_status = vos_timer_stop(&pHddApCtx->hdd_ap_inactivity_timer);
                if (!VOS_IS_STATUS_SUCCESS(vos_status))
                   hddLog(LOGE, FL("Failed to start AP inactivity timer\n"));
            }
#ifdef CONFIG_CFG80211
            {
                struct ieee80211_mgmt mgmt;
                v_U16_t iesLen =  pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.iesLen;
                v_U16_t size =  (24 + sizeof(mgmt.u.assoc_resp) + iesLen);
                memcpy(mgmt.da, &pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.staMac,6);
                memcpy(mgmt.sa, &pHostapdAdapter->macAddressCurrent,6);

                memcpy(mgmt.u.assoc_resp.variable,pSapEvent->sapevt.sapStationAssocReassocCompleteEvent.ies,iesLen);
                cfg80211_send_rx_assoc(dev,(const u8 *)&mgmt,size);
             }
#endif

            break;
        case eSAP_STA_DISASSOC_EVENT:
            memcpy(wrqu.addr.sa_data, &pSapEvent->sapevt.sapStationDisassocCompleteEvent.staMac,
                   sizeof(wrqu.addr.sa_data));
            hddLog(LOG1, " disassociated "MAC_ADDRESS_STR"\n", MAC_ADDR_ARRAY(wrqu.addr.sa_data));
            if (pSapEvent->sapevt.sapStationDisassocCompleteEvent.reason == eSAP_USR_INITATED_DISASSOC)
                hddLog(LOG1," User initiated disassociation");
            else
                hddLog(LOG1," MAC initiated disassociation");
            we_event = IWEVEXPIRED;
            vos_status = hdd_softap_GetStaId(pHostapdAdapter, &pSapEvent->sapevt.sapStationDisassocCompleteEvent.staMac, &staId);
            if (!VOS_IS_STATUS_SUCCESS(vos_status))
            {
                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD Failed to find sta id!!\n"));
                return VOS_STATUS_E_FAILURE;
            }
            hdd_softap_DeregisterSTA(pHostapdAdapter, staId);

            if (0 != (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->nAPAutoShutOff)
            {
                // Start AP inactivity timer if no stations associated with it
                for (i = 0; i < WLAN_MAX_STA_COUNT; i++)
                {
                    if (pHostapdAdapter->aStaInfo[i].isUsed && i != (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uBCStaId)
                    {
                        bApActive = TRUE;
                        break;
                    }
                }

                if (bApActive == FALSE)
                {
                    if (pHddApCtx->hdd_ap_inactivity_timer.state == VOS_TIMER_STATE_STOPPED)
                    {
                        vos_status = vos_timer_start(&pHddApCtx->hdd_ap_inactivity_timer, (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->nAPAutoShutOff * 1000);
                        if (!VOS_IS_STATUS_SUCCESS(vos_status))
                            hddLog(LOGE, FL("Failed to init AP inactivity timer\n"));
                    }
                    else
                        VOS_ASSERT(vos_timer_getCurrentState(&pHddApCtx->hdd_ap_inactivity_timer) == VOS_TIMER_STATE_STOPPED);
                }
            }
#ifdef CONFIG_CFG80211
            cfg80211_disconnected(dev, WLAN_REASON_UNSPECIFIED, NULL, 0, GFP_KERNEL); 
#endif
            break;
        case eSAP_WPS_PBC_PROBE_REQ_EVENT:
        {
                static const char * message ="MLMEWPSPBCPROBEREQ.indication";
                union iwreq_data wreq;
               
                down(&pHddApCtx->semWpsPBCOverlapInd);
                pHddApCtx->WPSPBCProbeReq.probeReqIELen = pSapEvent->sapevt.sapPBCProbeReqEvent.WPSPBCProbeReq.probeReqIELen;
                
                vos_mem_copy(pHddApCtx->WPSPBCProbeReq.probeReqIE, pSapEvent->sapevt.sapPBCProbeReqEvent.WPSPBCProbeReq.probeReqIE, 
                    pHddApCtx->WPSPBCProbeReq.probeReqIELen);
                     
                vos_mem_copy(pHddApCtx->WPSPBCProbeReq.peerMacAddr, pSapEvent->sapevt.sapPBCProbeReqEvent.WPSPBCProbeReq.peerMacAddr, sizeof(v_MACADDR_t));
                hddLog(LOGE, "WPS PBC probe req "MAC_ADDRESS_STR"\n", MAC_ADDR_ARRAY(pHddApCtx->WPSPBCProbeReq.peerMacAddr));
                memset(&wreq, 0, sizeof(wreq));
                wreq.data.length = strlen(message); // This is length of message
                wireless_send_event(dev, IWEVCUSTOM, &wreq, (char *)message); 
                
                return VOS_STATUS_SUCCESS;
        }
        case eSAP_ASSOC_STA_CALLBACK_EVENT:
            pAssocStasArray = pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas;
            if (pSapEvent->sapevt.sapAssocStaListEvent.noOfAssocSta != 0)
            {   // List of associated stations
                for (i = 0; i < pSapEvent->sapevt.sapAssocStaListEvent.noOfAssocSta; i++)
                {
                    hddLog(LOG1,"Associated Sta Num %d:assocId=%d, staId=%d, staMac="MAC_ADDRESS_STR,
                        i+1,
                        pAssocStasArray->assocId,
                        pAssocStasArray->staId,
                                    MAC_ADDR_ARRAY(pAssocStasArray->staMac.bytes));
                        pAssocStasArray++;             
            }
            }
            vos_mem_free(pSapEvent->sapevt.sapAssocStaListEvent.pAssocStas);// Release caller allocated memory here
            return VOS_STATUS_SUCCESS;
        default:
            hddLog(LOG1,"SAP message is not handled\n");
            goto stopbss;
            return VOS_STATUS_SUCCESS;
    }
    wireless_send_event(dev, we_event, &wrqu, (char *)we_custom_event_generic);
    return VOS_STATUS_SUCCESS;
    stopbss :
    {
        v_BYTE_t we_custom_event[64];
        char *stopBssEvent = "STOP-BSS.response";//17
        int event_len = strlen(stopBssEvent);
        memset(&we_custom_event, '\0', sizeof(we_custom_event));
        memcpy(&we_custom_event, stopBssEvent, event_len);
        memset(&wrqu, 0, sizeof(wrqu));
        wrqu.data.length = event_len;
        hddLog(LOGE, FL("BSS stop status = %s\n"),pSapEvent->sapevt.sapStopBssCompleteEvent.status ?
                         "eSAP_STATUS_FAILURE" : "eSAP_STATUS_SUCCESS");
        hdd_softap_stop_bss(pHostapdAdapter);
        we_event = IWEVCUSTOM;
        we_custom_event_generic = we_custom_event;
        wireless_send_event(dev, we_event, &wrqu, (char *)we_custom_event_generic);

        pHostapdState->bssState = BSS_STOP;
    }
    return VOS_STATUS_SUCCESS;
}
int hdd_softap_unpackIE( 
                tHalHandle halHandle,
                eCsrEncryptionType *pEncryptType, 
                eCsrEncryptionType *mcEncryptType, 
                eCsrAuthType *pAuthType, 
                u_int16_t gen_ie_len, 
                u_int8_t *gen_ie )
{
    tDot11fIERSN dot11RSNIE; 
    tDot11fIEWPA dot11WPAIE; 
 
    tANI_U8 *pRsnIe; 
    tANI_U16 RSNIeLen;
	
    if (NULL == halHandle)
    {
        hddLog(LOGE, FL("Error haHandle returned NULL\n"));
        return -EINVAL;
    }
	
	// Validity checks
    if ((gen_ie_len < VOS_MIN(DOT11F_IE_RSN_MIN_LEN, DOT11F_IE_WPA_MIN_LEN)) ||  
        (gen_ie_len > VOS_MAX(DOT11F_IE_RSN_MAX_LEN, DOT11F_IE_WPA_MAX_LEN)) ) 
        return -EINVAL;
    // Type check
    if ( gen_ie[0] ==  DOT11F_EID_RSN) 
    {         
        // Validity checks
        if ((gen_ie_len < DOT11F_IE_RSN_MIN_LEN ) ||  
            (gen_ie_len > DOT11F_IE_RSN_MAX_LEN) )
        {
            return VOS_STATUS_E_FAILURE;
        }
        // Skip past the EID byte and length byte  
        pRsnIe = gen_ie + 2; 
        RSNIeLen = gen_ie_len - 2; 
        // Unpack the RSN IE 
        dot11fUnpackIeRSN((tpAniSirGlobal) halHandle, 
                            pRsnIe, 
                            RSNIeLen, 
                            &dot11RSNIE);
        // Copy out the encryption and authentication types 
        hddLog(LOG1, FL("%s: pairwise cipher suite count: %d\n"), 
                __FUNCTION__, dot11RSNIE.pwise_cipher_suite_count );
        hddLog(LOG1, FL("%s: authentication suite count: %d\n"), 
                __FUNCTION__, dot11RSNIE.akm_suite_count);
        /*Here we have followed the apple base code, 
          but probably I suspect we can do something different*/
        //dot11RSNIE.akm_suite_count
        // Just translate the FIRST one 
        *pAuthType =  hdd_TranslateRSNToCsrAuthType(dot11RSNIE.akm_suites[0]); 
        //dot11RSNIE.pwise_cipher_suite_count 
        *pEncryptType = hdd_TranslateRSNToCsrEncryptionType(dot11RSNIE.pwise_cipher_suites[0]);                     
        //dot11RSNIE.gp_cipher_suite_count 
        *mcEncryptType = hdd_TranslateRSNToCsrEncryptionType(dot11RSNIE.gp_cipher_suite);                     
        // Set the PMKSA ID Cache for this interface
          
        // Calling csrRoamSetPMKIDCache to configure the PMKIDs into the cache
    } else 
    if (gen_ie[0] == DOT11F_EID_WPA) 
    {         
        // Validity checks
        if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN ) ||  
            (gen_ie_len > DOT11F_IE_WPA_MAX_LEN))
        {
            return VOS_STATUS_E_FAILURE;
        }
        // Skip past the EID byte and length byte - and four byte WiFi OUI  
        pRsnIe = gen_ie + 2 + 4; 
        RSNIeLen = gen_ie_len - (2 + 4); 
        // Unpack the WPA IE 
        dot11fUnpackIeWPA((tpAniSirGlobal) halHandle, 
                            pRsnIe, 
                            RSNIeLen, 
                            &dot11WPAIE);
        // Copy out the encryption and authentication types 
        hddLog(LOG1, FL("%s: WPA unicast cipher suite count: %d\n"), 
                __FUNCTION__, dot11WPAIE.unicast_cipher_count );
        hddLog(LOG1, FL("%s: WPA authentication suite count: %d\n"), 
                __FUNCTION__, dot11WPAIE.auth_suite_count);
        //dot11WPAIE.auth_suite_count
        // Just translate the FIRST one 
        *pAuthType =  hdd_TranslateWPAToCsrAuthType(dot11WPAIE.auth_suites[0]); 
        //dot11WPAIE.unicast_cipher_count 
        *pEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.unicast_ciphers[0]);                       
        //dot11WPAIE.unicast_cipher_count 
        *mcEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.multicast_cipher);                       
    } 
    else 
    { 
        hddLog(LOGE, FL("%s: gen_ie[0]: %d\n"), __FUNCTION__, gen_ie[0]);
        return VOS_STATUS_E_FAILURE; 
    }
	return VOS_STATUS_SUCCESS;
}
int
static iw_softap_setparam(struct net_device *dev, 
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
    int *value = (int *)extra;
    int sub_cmd = value[0];
    int set_value = value[1];
    eHalStatus status;
    int ret = 0; /* success */

    switch(sub_cmd)
    {

    case QCSAP_PARAM_MAX_ASSOC:
        if ((WNI_CFG_ASSOC_STA_LIMIT_STAMIN > set_value) ||
            (WNI_CFG_ASSOC_STA_LIMIT_STAMAX < set_value))
        {
            hddLog(LOGE, FL("Invalid setMaxAssoc value %d"),
                   set_value);
            ret = -EINVAL;
        }
        else
        {
            status = ccmCfgSetInt(hHal, WNI_CFG_ASSOC_STA_LIMIT,
                                  set_value, NULL, eANI_BOOLEAN_FALSE);
            if ( status != eHAL_STATUS_SUCCESS ) 
            {
                hddLog(LOGE, FL("setMaxAssoc failure, status %d"),
                       status);
                ret = -EIO;
            }
        }
        break;

    default:
        hddLog(LOGE, FL("Invalid setparam command %d value %d"),
               sub_cmd, set_value);
        ret = -EINVAL;
        break;
    }

    return ret;
}


int
static iw_softap_getparam(struct net_device *dev, 
                          struct iw_request_info *info,
                          union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pHostapdAdapter);
    int *value = (int *)extra;
    int sub_cmd = value[0];
    eHalStatus status;
    int ret = 0; /* success */

    switch (sub_cmd)
    {
    case QCSAP_PARAM_MAX_ASSOC:
        status = ccmCfgGetInt(hHal, WNI_CFG_ASSOC_STA_LIMIT, (tANI_U32 *)value);
        if (eHAL_STATUS_SUCCESS != status)
        {
            ret = -EIO;
        }
        break;

    default:
        hddLog(LOGE, FL("Invalid getparam command %d"), sub_cmd);
        ret = -EINVAL;
        break;

    }

    return ret;
}


int
static iw_softap_getchannel(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
#ifdef HDD_SESSIONIZE
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	ptSapContext  pSapCtx;
	v_CONTEXT_t pvos = pHostapdAdapter->pvosContext;
 
   //TODO - Use an API. OR get it from profile. Dont use SAP data structure here.
	pSapCtx = VOS_GET_SAP_CB(pvos);

	/** Operating channel */
    *(v_U32_t *)(wrqu->data.pointer) = pSapCtx->channel;

	wrqu->data.length = sizeof(pSapCtx->channel);
#endif

	return 0;
}

#define IS_BROADCAST_MAC(x) (((x[0] & x[1] & x[2] & x[3] & x[4] & x[5]) == 0xff) ? 1 : 0)

int
static iw_softap_getassoc_stamacaddr(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	unsigned char *pmaclist;
   hdd_station_info_t *pStaInfo = pHostapdAdapter->aStaInfo;
	int cnt = 0, len;


	pmaclist = wrqu->data.pointer + sizeof(unsigned long int);
	len = wrqu->data.length;

	while((cnt < WLAN_MAX_STA_COUNT) && (len > (sizeof(v_MACADDR_t)+1))) {
		if (TRUE == pStaInfo[cnt].isUsed) {
			
			if(!IS_BROADCAST_MAC(pStaInfo[cnt].macAddrSTA.bytes)) {
				memcpy((void *)pmaclist, (void *)&(pStaInfo[cnt].macAddrSTA), sizeof(v_MACADDR_t));
				pmaclist += sizeof(v_MACADDR_t);
				len -= sizeof(v_MACADDR_t);
			}
		}
		cnt++;
	}

	*pmaclist = '\0';

    wrqu->data.length -= len;

	*(unsigned long int *)(wrqu->data.pointer) = wrqu->data.length;

	return 0;
}

int
static iw_softap_disassoc_sta(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
	

	hdd_softap_sta_disassoc(pHostapdAdapter, (v_U8_t *)(wrqu->data.pointer));

	return 0;
}

int
static iw_softap_ap_stats(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    WLANTL_TRANSFER_STA_TYPE  statBuffer;
    char *pstatbuf;
    int len = wrqu->data.length;
    pstatbuf = wrqu->data.pointer;

    WLANSAP_GetStatistics((WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext, &statBuffer, (v_BOOL_t)wrqu->data.flags);

    len = snprintf(pstatbuf, len,
            "RUF=%d RMF=%d RBF=%d "
            "RUB=%d RMB=%d RBB=%d "
            "TUF=%d TMF=%d TBF=%d "
            "TUB=%d TMB=%d TBB=%d",
            (int)statBuffer.rxUCFcnt, (int)statBuffer.rxMCFcnt, (int)statBuffer.rxBCFcnt,
            (int)statBuffer.rxUCBcnt, (int)statBuffer.rxMCBcnt, (int)statBuffer.rxBCBcnt,
            (int)statBuffer.txUCFcnt, (int)statBuffer.txMCFcnt, (int)statBuffer.txBCFcnt,
            (int)statBuffer.txUCBcnt, (int)statBuffer.txMCBcnt, (int)statBuffer.txBCBcnt
            );

    wrqu->data.length -= len;
    return 0;
}

int
static iw_softap_commit(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    VOS_STATUS vos_status = VOS_STATUS_SUCCESS;
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    hdd_hostapd_state_t *pHostapdState;
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext; 
    tpWLAN_SAPEventCB pSapEventCallback;
    tsap_Config_t *pConfig;
    s_CommitConfig_t *pCommitConfig;
    struct qc_mac_acl_entry *acl_entry = NULL;
    v_SINT_t i = 0, num_mac = 0;
    v_U32_t status = 0;
    eCsrAuthType RSNAuthType;
    eCsrEncryptionType RSNEncryptType;
    eCsrEncryptionType mcRSNEncryptType;

    pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);
    pCommitConfig = (s_CommitConfig_t *)extra;
    
    pConfig = kmalloc(sizeof(tsap_Config_t), GFP_KERNEL);
    pConfig->beacon_int =  pCommitConfig->beacon_int;
    pConfig->channel = pCommitConfig->channel;

    /*Protection parameter to enable or disable*/
    pConfig->protEnabled = (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apProtEnabled;
    pConfig->dtim_period = pCommitConfig->dtim_period;
    switch(pCommitConfig->hw_mode )
    {
        case eQC_DOT11_MODE_11B:
        pConfig->SapHw_mode = eSAP_DOT11_MODE_11b; 
            break;
        case eQC_DOT11_MODE_11G:
        pConfig->SapHw_mode = eSAP_DOT11_MODE_11g;
            break;
       
        case eQC_DOT11_MODE_11N:
        pConfig->SapHw_mode = eSAP_DOT11_MODE_11n;
            break;
        case eQC_DOT11_MODE_11G_ONLY:
            pConfig->SapHw_mode = eSAP_DOT11_MODE_11g_ONLY;
            break;
        case eQC_DOT11_MODE_11N_ONLY:
            pConfig->SapHw_mode = eSAP_DOT11_MODE_11n_ONLY;
            break;
        default:
        pConfig->SapHw_mode = eSAP_DOT11_MODE_11n;
            break;
            
    }
  
    pConfig->ieee80211d = pCommitConfig->qcsap80211d;
    vos_mem_copy(pConfig->countryCode, pCommitConfig->countryCode, 3);
    if(pCommitConfig->authType == eQC_AUTH_TYPE_SHARED_KEY)
        pConfig->authType = eSAP_SHARED_KEY;
    else if(pCommitConfig->authType == eQC_AUTH_TYPE_OPEN_SYSTEM) 
        pConfig->authType = eSAP_OPEN_SYSTEM;
    else
        pConfig->authType = eSAP_AUTO_SWITCH;
    
    pConfig->privacy = pCommitConfig->privacy;
    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = pCommitConfig->privacy;
    pConfig->wps_state = pCommitConfig->wps_state;
    pConfig->fwdWPSPBCProbeReq  = 1; // Forward WPS PBC probe request frame up 
    pConfig->RSNWPAReqIELength = pCommitConfig->RSNWPAReqIELength;
    if(pConfig->RSNWPAReqIELength){
        pConfig->pRSNWPAReqIE = &pCommitConfig->RSNWPAReqIE[0];
        if ((pConfig->pRSNWPAReqIE[0] == DOT11F_EID_RSN) || (pConfig->pRSNWPAReqIE[0] == DOT11F_EID_WPA)){
            // The actual processing may eventually be more extensive than this.
            // Right now, just consume any PMKIDs that are  sent in by the app.
            status = hdd_softap_unpackIE( 
#if defined(FEATURE_WLAN_NON_INTEGRATED_SOC)
                                  vos_get_context( VOS_MODULE_ID_HAL, pVosContext),
#else
                                  vos_get_context( VOS_MODULE_ID_PE, pVosContext),
#endif
                                  &RSNEncryptType,
                                  &mcRSNEncryptType,
                                  &RSNAuthType,
                                  pConfig->pRSNWPAReqIE[1]+2,
                                  pConfig->pRSNWPAReqIE );
             
            if( VOS_STATUS_SUCCESS == status )
            {
                 // Now copy over all the security attributes you have parsed out
                 //TODO: Need to handle mixed mode     
                 pConfig->RSNEncryptType = RSNEncryptType; // Use the cipher type in the RSN IE
                 pConfig->mcRSNEncryptType = mcRSNEncryptType;
                 hddLog( LOG1, FL("%s: CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d\n"), 
                                  RSNAuthType, RSNEncryptType, mcRSNEncryptType);
             } 
        }
    }
    else
    {
        /* If no RSNIE, set encrypt type to NONE*/
        pConfig->RSNEncryptType = eCSR_ENCRYPT_TYPE_NONE;
        pConfig->mcRSNEncryptType =  eCSR_ENCRYPT_TYPE_NONE;
        hddLog( LOG1, FL("EncryptionType = %d mcEncryptionType = %d\n"), 
                         pConfig->RSNEncryptType, pConfig->mcRSNEncryptType);
    }

    pConfig->SSIDinfo.ssidHidden = pCommitConfig->SSIDinfo.ssidHidden; 
    pConfig->SSIDinfo.ssid.length = pCommitConfig->SSIDinfo.ssid.length;
    vos_mem_copy(pConfig->SSIDinfo.ssid.ssId, pCommitConfig->SSIDinfo.ssid.ssId, pConfig->SSIDinfo.ssid.length);
    vos_mem_copy(pConfig->self_macaddr.bytes, pHostapdAdapter->macAddressCurrent.bytes, sizeof(v_MACADDR_t));
    
    pConfig->SapMacaddr_acl = pCommitConfig->qc_macaddr_acl;
    pConfig->ht_capab = pCommitConfig->ht_capab;
    if (pCommitConfig->num_accept_mac > MAX_MAC_ADDRESS_ACCEPTED)
        num_mac = pConfig->num_accept_mac = MAX_MAC_ADDRESS_ACCEPTED;
    else
        num_mac = pConfig->num_accept_mac = pCommitConfig->num_accept_mac;
    acl_entry = pCommitConfig->accept_mac;
    for (i = 0; i < num_mac; i++)
    {
        vos_mem_copy(&pConfig->accept_mac[i], acl_entry->addr, sizeof(v_MACADDR_t));
        acl_entry++;
    }
    if (pCommitConfig->num_deny_mac > MAX_MAC_ADDRESS_DENIED)
        num_mac = pConfig->num_deny_mac = MAX_MAC_ADDRESS_DENIED;
    else
        num_mac = pConfig->num_deny_mac = pCommitConfig->num_deny_mac;
    acl_entry = pCommitConfig->deny_mac;
    for (i = 0; i < num_mac; i++)
    {
        vos_mem_copy(&pConfig->deny_mac[i], acl_entry->addr, sizeof(v_MACADDR_t));
        acl_entry++;
    }
    //Uapsd Enabled Bit
    pConfig->UapsdEnable =  (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apUapsdEnabled;
    //Enable OBSS protection
    pConfig->obssProtEnabled = (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apOBSSProtEnabled; 
    (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->apDisableIntraBssFwd = (WLAN_HDD_GET_CTX(pHostapdAdapter))->cfg_ini->apDisableIntraBssFwd;
    
    hddLog(LOGW, FL("SOftAP macaddress : "MAC_ADDRESS_STR"\n"), MAC_ADDR_ARRAY(pHostapdAdapter->macAddressCurrent.bytes));
    hddLog(LOGW,FL("ssid =%s\n"), pConfig->SSIDinfo.ssid.ssId);  
    hddLog(LOGW,FL("beaconint=%d, channel=%d\n"), (int)pConfig->beacon_int, (int)pConfig->channel);
    hddLog(LOGW,FL("hw_mode=%x\n"),  pConfig->SapHw_mode);
    hddLog(LOGW,FL("privacy=%d, authType=%d\n"), pConfig->privacy, pConfig->authType); 
    hddLog(LOGW,FL("RSN/WPALen=%d, \n"),(int)pConfig->RSNWPAReqIELength);
    hddLog(LOGW,FL("Uapsd = %d\n"),pConfig->UapsdEnable); 
    hddLog(LOGW,FL("ProtEnabled = %d, OBSSProtEnabled = %d\n"),pConfig->protEnabled, pConfig->obssProtEnabled); 
    hddLog(LOGW,FL("DisableIntraBssFwd = %d\n"),(WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->apDisableIntraBssFwd); 
            
    pSapEventCallback = hdd_hostapd_SAPEventCB;
	pConfig->persona = pHostapdAdapter->device_mode;
    if(WLANSAP_StartBss(pVosContext, pSapEventCallback, pConfig,(v_PVOID_t)dev) != VOS_STATUS_SUCCESS)
    {
           hddLog(LOGE,FL("SAP Start Bss fail\n"));
    }
    
    kfree(pConfig);
    
    hddLog(LOGE, FL("Waiting for Scan to complete(auto mode) and BSS to start"));
    vos_status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);
   
    if (!VOS_IS_STATUS_SUCCESS(vos_status))
    {  
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: HDD vos wait for single_event failed!!\n"));
       VOS_ASSERT(0);
    }
 
    pHostapdState->bCommit = TRUE;
    if(pHostapdState->vosStatus)
    {
        return -1;
    }
    else
    {
        set_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);
        WLANSAP_Update_WpsIe ( pVosContext );            
        return 0;
    }
}
static 
int iw_softap_setmlme(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    struct sQcSapreq_mlme *pmlme;
    hdd_adapter_t *pHostapdAdapter = (hdd_adapter_t*)(netdev_priv(dev));
    v_MACADDR_t destAddress;
    pmlme = (struct sQcSapreq_mlme *)(wrqu->name);
    /* NOTE: this address is not valid incase of TKIP failure, since not filled */
    vos_mem_copy(&destAddress.bytes, pmlme->im_macaddr, sizeof(v_MACADDR_t));
    switch(pmlme->im_op)
    {
        case QCSAP_MLME_AUTHORIZE:
                    hdd_softap_change_STA_state( pHostapdAdapter, &destAddress, WLANTL_STA_AUTHENTICATED);
        break;
        case QCSAP_MLME_ASSOC:
        //TODO:inform to TL after associating (not needed  as we do in sapCallback)
        break;
        case QCSAP_MLME_UNAUTHORIZE:
        //TODO: send the disassoc to station
        //hdd_softap_change_STA_state( pHostapdAdapter, pmlme->im_macaddr, WLANTL_STA_AUTHENTICATED);
        break;
        case QCSAP_MLME_DISASSOC:
            hdd_softap_sta_disassoc(pHostapdAdapter,pmlme->im_macaddr);
        break;
        case QCSAP_MLME_DEAUTH:
            hdd_softap_sta_deauth(pHostapdAdapter,pmlme->im_macaddr);
        break;
        case QCSAP_MLME_MICFAILURE:
            hdd_softap_tkip_mic_fail_counter_measure(pHostapdAdapter,pmlme->im_reason);
        break;
        default:
        break;
    }
    return 0;
}
static 
int iw_get_genie(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext; 
    eHalStatus status;
    v_U32_t length = DOT11F_IE_RSN_MAX_LEN;
    v_U8_t genIeBytes[DOT11F_IE_RSN_MAX_LEN];
    ENTER();
    hddLog(LOG1,FL("getGEN_IE ioctl\n"));
    // Actually retrieve the RSN IE from CSR.  (We previously sent it down in the CSR Roam Profile.)
    status = WLANSap_getstationIE_information(pVosContext, 
                                   &length,
                                   genIeBytes);
    wrqu->data.length = VOS_MIN((u_int16_t) length, DOT11F_IE_RSN_MAX_LEN);
    vos_mem_copy( wrqu->data.pointer, (v_VOID_t*)genIeBytes, wrqu->data.length);
    
    hddLog(LOG1,FL(" RSN IE of %d bytes returned\n"), wrqu->data.length ); 
    
   
    EXIT();
    return 0;
}
static 
int iw_get_WPSPBCProbeReqIEs(struct net_device *dev,
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));    
    sQcSapreq_WPSPBCProbeReqIES_t *pWPSPBCProbeReqIEs;
    hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
    ENTER();
        
    hddLog(LOG1,FL("get_WPSPBCProbeReqIEs ioctl\n"));
    
    pWPSPBCProbeReqIEs = (sQcSapreq_WPSPBCProbeReqIES_t *)(wrqu->data.pointer);
    pWPSPBCProbeReqIEs->probeReqIELen = pHddApCtx->WPSPBCProbeReq.probeReqIELen;
    vos_mem_copy(pWPSPBCProbeReqIEs->probeReqIE, pHddApCtx->WPSPBCProbeReq.probeReqIE, pWPSPBCProbeReqIEs->probeReqIELen);
    vos_mem_copy(pWPSPBCProbeReqIEs->macaddr, pHddApCtx->WPSPBCProbeReq.peerMacAddr, sizeof(v_MACADDR_t));
    wrqu->data.length = 12 + pWPSPBCProbeReqIEs->probeReqIELen;
    hddLog(LOGE, FL("Macaddress : "MAC_ADDRESS_STR"\n"),  MAC_ADDR_ARRAY(pWPSPBCProbeReqIEs->macaddr));
    up(&pHddApCtx->semWpsPBCOverlapInd);
    EXIT();
    return 0;
}
static int iw_set_encodeext(struct net_device *dev, 
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;    
    hdd_ap_ctx_t *pHddApCtx = WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter);
    eHalStatus halStatus= eHAL_STATUS_SUCCESS;
    struct iw_encode_ext *ext = (struct iw_encode_ext*)extra;
    v_U8_t groupmacaddr[WNI_CFG_BSSID_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int key_index;
    struct iw_point *encoding = &wrqu->encoding;
    tCsrRoamSetKey  setKey;   
//    tCsrRoamRemoveKey RemoveKey;
    int i;
    ENTER();    
   
    key_index = encoding->flags & IW_ENCODE_INDEX;
   
    if(key_index > 0) {
      
         /*Convert from 1-based to 0-based keying*/
        key_index--;
    }
    if(!ext->key_len) {
#if 0     
      /*Set the encrytion type to NONE*/
#if 0
       pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_NONE;
#endif
     
         RemoveKey.keyId = key_index;
         if(ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
              /*Key direction for group is RX only*/
             vos_mem_copy(RemoveKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
         }
         else {
             vos_mem_copy(RemoveKey.peerMac,ext->addr.sa_data,WNI_CFG_BSSID_LEN);
         }
         switch(ext->alg)
         {
           case IW_ENCODE_ALG_NONE:
              RemoveKey.encType = eCSR_ENCRYPT_TYPE_NONE;
              break;
           case IW_ENCODE_ALG_WEP:
              RemoveKey.encType = (ext->key_len== 5) ? eCSR_ENCRYPT_TYPE_WEP40:eCSR_ENCRYPT_TYPE_WEP104;
              break;
           case IW_ENCODE_ALG_TKIP:
              RemoveKey.encType = eCSR_ENCRYPT_TYPE_TKIP;
           break;
           case IW_ENCODE_ALG_CCMP:
              RemoveKey.encType = eCSR_ENCRYPT_TYPE_AES;
              break;
          default:
              RemoveKey.encType = eCSR_ENCRYPT_TYPE_NONE;
              break;
         }
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Remove key cipher_alg:%d key_len%d *pEncryptionType :%d \n",
                    __FUNCTION__,(int)ext->alg,(int)ext->key_len,RemoveKey.encType);
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%s: Peer Mac = "MAC_ADDRESS_STR"\n",
                    __FUNCTION__, MAC_ADDR_ARRAY(RemoveKey.peerMac));
          );
         halStatus = WLANSAP_DelKeySta( pVosContext, &RemoveKey);
         if ( halStatus != eHAL_STATUS_SUCCESS )
         {
             VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "[%4d] WLANSAP_DeleteKeysSta returned ERROR status= %d",
                        __LINE__, halStatus );
         }
#endif         
         return halStatus;

    }   
    
    vos_mem_zero(&setKey,sizeof(tCsrRoamSetKey));
   
    setKey.keyId = key_index;
    setKey.keyLength = ext->key_len;
   
    if(ext->key_len <= CSR_MAX_KEY_LEN) {
       vos_mem_copy(&setKey.Key[0],ext->key,ext->key_len);
    }   
   
    if(ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
      /*Key direction for group is RX only*/
       setKey.keyDirection = eSIR_RX_ONLY;
       vos_mem_copy(setKey.peerMac,groupmacaddr,WNI_CFG_BSSID_LEN);
    }
    else {   
      
       setKey.keyDirection =  eSIR_TX_RX;
       vos_mem_copy(setKey.peerMac,ext->addr.sa_data,WNI_CFG_BSSID_LEN);
    }
    if(ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
    {
       setKey.keyDirection = eSIR_TX_DEFAULT;
       vos_mem_copy(setKey.peerMac,ext->addr.sa_data,WNI_CFG_BSSID_LEN);
    }
 
    /*For supplicant pae role is zero*/
    setKey.paeRole = 0;
      
    switch(ext->alg)
    {   
       case IW_ENCODE_ALG_NONE:   
         setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
         break;
         
       case IW_ENCODE_ALG_WEP:
         setKey.encType = (ext->key_len== 5) ? eCSR_ENCRYPT_TYPE_WEP40:eCSR_ENCRYPT_TYPE_WEP104;
         pHddApCtx->uPrivacy = 1;
         printk("(%s) uPrivacy=%d", __FUNCTION__, pHddApCtx->uPrivacy);
         break;
      
       case IW_ENCODE_ALG_TKIP:
       {
          v_U8_t *pKey = &setKey.Key[0];
  
          setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;
  
          vos_mem_zero(pKey, CSR_MAX_KEY_LEN);
  
          /*Supplicant sends the 32bytes key in this order 
          
                |--------------|----------|----------|
                |   Tk1        |TX-MIC    |  RX Mic  | 
                |--------------|----------|----------|
                <---16bytes---><--8bytes--><--8bytes-->
                
                */
          /*Sme expects the 32 bytes key to be in the below order
  
                |--------------|----------|----------|
                |   Tk1        |RX-MIC    |  TX Mic  | 
                |--------------|----------|----------|
                <---16bytes---><--8bytes--><--8bytes-->
               */
          /* Copy the Temporal Key 1 (TK1) */
          vos_mem_copy(pKey,ext->key,16);
           
         /*Copy the rx mic first*/
          vos_mem_copy(&pKey[16],&ext->key[24],8); 
          
         /*Copy the tx mic */
          vos_mem_copy(&pKey[24],&ext->key[16],8); 
  
       }     
       break;
      
       case IW_ENCODE_ALG_CCMP:
          setKey.encType = eCSR_ENCRYPT_TYPE_AES;
          break;
          
       default:
          setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
          break;
    }
         
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
          ("%s:EncryptionType:%d key_len:%d, :%d, KeyId:%d \n"),__FUNCTION__, setKey.encType, setKey.keyLength,
            setKey.keyId);
    for(i=0; i< ext->key_len; i++)
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
          ("%02x"), setKey.Key[i]);    
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
          ("\n"));
    halStatus = WLANSAP_SetKeySta( pVosContext, &setKey);
    
    if ( halStatus != eHAL_STATUS_SUCCESS )
    {
       VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "[%4d] WLANSAP_SetKeySta returned ERROR status= %d", __LINE__, halStatus );
    }   
   
   return halStatus;
}
static int iw_set_mlme(struct net_device *dev,
                       struct iw_request_info *info,
                       union iwreq_data *wrqu,
                       char *extra)
{
#if 0
    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    struct iw_mlme *mlme = (struct iw_mlme *)extra;
 
    ENTER();    
   
    //reason_code is unused. By default it is set to eCSR_DISCONNECT_REASON_UNSPECIFIED
    switch (mlme->cmd) {
        case IW_MLME_DISASSOC:
        case IW_MLME_DEAUTH:
	    printk("Station disassociate");	
            if( pAdapter->conn_info.connState == eConnectionState_Associated ) 
            {
                eCsrRoamDisconnectReason reason = eCSR_DISCONNECT_REASON_UNSPECIFIED;
                
                if( mlme->reason_code == HDD_REASON_MICHAEL_MIC_FAILURE )
                    reason = eCSR_DISCONNECT_REASON_MIC_ERROR;
                
                status = sme_RoamDisconnect( pAdapter->hHal,pAdapter->sessionId, reason);
                
                //clear all the reason codes
                if (status != 0)
                {
                    hddLog(LOGE,"%s %d Command Disassociate/Deauthenticate : csrRoamDisconnect failure returned %d \n", __FUNCTION__, (int)mlme->cmd, (int)status );
                }
                
               netif_stop_queue(dev);
               netif_carrier_off(dev);
            }
            else
            {
                hddLog(LOGE,"%s %d Command Disassociate/Deauthenticate called but station is not in associated state \n", __FUNCTION__, (int)mlme->cmd );
            }
        default:
            hddLog(LOGE,"%s %d Command should be Disassociate/Deauthenticate \n", __FUNCTION__, (int)mlme->cmd );
            return -EINVAL;
    }//end of switch
    EXIT();
#endif    
    return 0;
//    return status;
}

static int iw_softap_setwpsie(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, 
        char *extra)
{
   hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
   v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;
   hdd_hostapd_state_t *pHostapdState;
   eHalStatus halStatus= eHAL_STATUS_SUCCESS;
   u_int8_t *wps_genie =  wrqu->data.pointer;
   u_int8_t *pos;
   tpSap_WPSIE pSap_WPSIe;
   u_int8_t WPSIeType;
   u_int16_t length;   
   ENTER();

   if(!wrqu->data.length)
      return 0;

   pSap_WPSIe = vos_mem_malloc(sizeof(tSap_WPSIE));
   vos_mem_zero(pSap_WPSIe, sizeof(tSap_WPSIE));
 
   hddLog(LOGE,"%s WPS IE type[0x%X] IE[0x%X], LEN[%d]\n", __FUNCTION__, wps_genie[0], wps_genie[1], wps_genie[2]);
   WPSIeType = wps_genie[0];
   if ( wps_genie[0] == eQC_WPS_BEACON_IE)
   {
      pSap_WPSIe->sapWPSIECode = eSAP_WPS_BEACON_IE; 
      wps_genie = wps_genie + 1;
      switch ( wps_genie[0] ) 
      {
         case DOT11F_EID_WPA: 
            if (wps_genie[1] < 2 + 4)
            {
               vos_mem_free(pSap_WPSIe); 
               return -EINVAL;
            }
            else if (memcmp(&wps_genie[2], "\x00\x50\xf2\x04", 4) == 0) 
            {
             hddLog (LOGE, "%s Set WPS BEACON IE(len %d)",__FUNCTION__, wps_genie[1]+2);
             pos = &wps_genie[6];
             while (((size_t)pos - (size_t)&wps_genie[6])  < (wps_genie[1] - 4) )
             {
                switch((u_int16_t)(*pos<<8) | *(pos+1))
                {
                   case HDD_WPS_ELEM_VERSION:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.Version = *pos;   
                      hddLog(LOGE, "WPS version %d\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.Version);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_VER_PRESENT;   
                      pos += 1;
                      break;
                   
                   case HDD_WPS_ELEM_WPS_STATE:
                      pos +=4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.wpsState = *pos;
                      hddLog(LOGE, "WPS State %d\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.wpsState);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_STATE_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_APSETUPLOCK:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.APSetupLocked = *pos;
                      hddLog(LOGE, "AP setup lock %d\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.APSetupLocked);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_APSETUPLOCK_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_SELECTEDREGISTRA:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.SelectedRegistra = *pos;
                      hddLog(LOGE, "Selected Registra %d\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.SelectedRegistra);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_SELECTEDREGISTRA_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_DEVICE_PASSWORD_ID:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.DevicePasswordID = (*pos<<8) | *(pos+1);
                      hddLog(LOGE, "Password ID: %x\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.DevicePasswordID);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_DEVICEPASSWORDID_PRESENT;
                      pos += 2; 
                      break;
                   case HDD_WPS_ELEM_REGISTRA_CONF_METHODS:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.SelectedRegistraCfgMethod = (*pos<<8) | *(pos+1);
                      hddLog(LOGE, "Select Registra Config Methods: %x\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.SelectedRegistraCfgMethod);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_SELECTEDREGISTRACFGMETHOD_PRESENT;
                      pos += 2; 
                      break;
                
                   case HDD_WPS_ELEM_UUID_E:
                      pos += 2; 
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSBeaconIE.UUID_E, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_UUIDE_PRESENT; 
                      pos += length;
                      break;
                   case HDD_WPS_ELEM_RF_BANDS:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.RFBand = *pos;
                      hddLog(LOGE, "RF band: %d\n", pSap_WPSIe->sapwpsie.sapWPSBeaconIE.RFBand);
                      pSap_WPSIe->sapwpsie.sapWPSBeaconIE.FieldPresent |= WPS_BEACON_RF_BANDS_PRESENT;
                      pos += 1;
                      break;
                   
                   default:
                      hddLog (LOGE, "UNKNOWN TLV in WPS IE(%x)\n", (*pos<<8 | *(pos+1)));
                      vos_mem_free(pSap_WPSIe);
                      return -EINVAL; 
                }
              }  
            }
            else { 
                 hddLog (LOGE, "%s WPS IE Mismatch%X",__FUNCTION__);
            }     
            break;
                 
         default:
            hddLog (LOGE, "%s Set UNKNOWN IE %X",__FUNCTION__, wps_genie[0]);
            vos_mem_free(pSap_WPSIe);
            return 0;
      }
    } 
    else if( wps_genie[0] == eQC_WPS_PROBE_RSP_IE)
    {
      pSap_WPSIe->sapWPSIECode = eSAP_WPS_PROBE_RSP_IE; 
      wps_genie = wps_genie + 1;
      switch ( wps_genie[0] ) 
      {
         case DOT11F_EID_WPA: 
            if (wps_genie[1] < 2 + 4)
            {
               vos_mem_free(pSap_WPSIe); 
               return -EINVAL;
            }
            else if (memcmp(&wps_genie[2], "\x00\x50\xf2\x04", 4) == 0) 
            {
             hddLog (LOGE, "%s Set WPS PROBE RSP IE(len %d)",__FUNCTION__, wps_genie[1]+2);
             pos = &wps_genie[6];
             while (((size_t)pos - (size_t)&wps_genie[6])  < (wps_genie[1] - 4) )
             {
              switch((u_int16_t)(*pos<<8) | *(pos+1))
              {
                   case HDD_WPS_ELEM_VERSION:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.Version = *pos;   
                      hddLog(LOGE, "WPS version %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.Version); 
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_VER_PRESENT;   
                      pos += 1;
                      break;
                   
                   case HDD_WPS_ELEM_WPS_STATE:
                      pos +=4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.wpsState = *pos;
                      hddLog(LOGE, "WPS State %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.wpsState);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_STATE_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_APSETUPLOCK:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.APSetupLocked = *pos;
                      hddLog(LOGE, "AP setup lock %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.APSetupLocked);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_APSETUPLOCK_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_SELECTEDREGISTRA:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SelectedRegistra = *pos;
                      hddLog(LOGE, "Selected Registra %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SelectedRegistra);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_SELECTEDREGISTRA_PRESENT;                      
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_DEVICE_PASSWORD_ID:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DevicePasswordID = (*pos<<8) | *(pos+1);
                      hddLog(LOGE, "Password ID: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DevicePasswordID);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_DEVICEPASSWORDID_PRESENT;
                      pos += 2; 
                      break;
                   case HDD_WPS_ELEM_REGISTRA_CONF_METHODS:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SelectedRegistraCfgMethod = (*pos<<8) | *(pos+1);
                      hddLog(LOGE, "Select Registra Config Methods: %x\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SelectedRegistraCfgMethod);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_SELECTEDREGISTRACFGMETHOD_PRESENT;
                      pos += 2; 
                      break;
                  case HDD_WPS_ELEM_RSP_TYPE:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ResponseType = *pos;
                      hddLog(LOGE, "Config Methods: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ResponseType);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_RESPONSETYPE_PRESENT;
                      pos += 1;
                      break;
                   case HDD_WPS_ELEM_UUID_E:
                      pos += 2; 
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.UUID_E, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_UUIDE_PRESENT;
                      pos += length;
                      break;
                   
                   case HDD_WPS_ELEM_MANUFACTURER:
                      pos += 2;
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.Manufacture.num_name = length;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.Manufacture.name, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_MANUFACTURE_PRESENT;
                      pos += length;
                      break;
 
                   case HDD_WPS_ELEM_MODEL_NAME:
                      pos += 2;
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ModelName.num_text = length;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ModelName.text, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_MODELNAME_PRESENT;
                      pos += length;
                      break;
                   case HDD_WPS_ELEM_MODEL_NUM:
                      pos += 2;
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ModelNumber.num_text = length;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ModelNumber.text, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_MODELNUMBER_PRESENT;
                      pos += length;
                      break;
                   case HDD_WPS_ELEM_SERIAL_NUM:
                      pos += 2;
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SerialNumber.num_text = length;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SerialNumber.text, pos, length);
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_SERIALNUMBER_PRESENT;
                      pos += length;
                      break;
                   case HDD_WPS_ELEM_PRIMARY_DEVICE_TYPE:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.PrimaryDeviceCategory = (*pos<<8 | *(pos+1));
                      hddLog(LOGE, "primary dev category: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.PrimaryDeviceCategory);  
                      pos += 2;
                      
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.PrimaryDeviceOUI, pos, HDD_WPS_DEVICE_OUI_LEN);
                      hddLog(LOGE, "primary dev oui: %02x, %02x, %02x, %02x\n", pos[0], pos[1], pos[2], pos[3]);
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DeviceSubCategory = (*pos<<8 | *(pos+1));
                      hddLog(LOGE, "primary dev sub category: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DeviceSubCategory);  
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_PRIMARYDEVICETYPE_PRESENT;                      
                      break;
                   case HDD_WPS_ELEM_DEVICE_NAME:
                      pos += 2;
                      length = *pos<<8 | *(pos+1);
                      pos += 2;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DeviceName.num_text = length;
                      vos_mem_copy(pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.DeviceName.text, pos, length);
                      pos += length;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_DEVICENAME_PRESENT;
                      break;
                   case HDD_WPS_ELEM_CONFIG_METHODS:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.ConfigMethod = (*pos<<8) | *(pos+1);
                      hddLog(LOGE, "Config Methods: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.SelectedRegistraCfgMethod);
                      pos += 2; 
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_CONFIGMETHODS_PRESENT;
                      break;
 
                   case HDD_WPS_ELEM_RF_BANDS:
                      pos += 4;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.RFBand = *pos;
                      hddLog(LOGE, "RF band: %d\n", pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.RFBand);
                      pos += 1;
                      pSap_WPSIe->sapwpsie.sapWPSProbeRspIE.FieldPresent |= WPS_PROBRSP_RF_BANDS_PRESENT;
                      break;
              }  // switch
            }
         } 
         else
         {
             hddLog (LOGE, "%s WPS IE Mismatch%X",__FUNCTION__);
         }
         
      } // switch
    }
    halStatus = WLANSAP_Set_WpsIe(pVosContext, pSap_WPSIe);
    pHostapdState = WLAN_HDD_GET_HOSTAP_STATE_PTR(pHostapdAdapter);
    if( pHostapdState->bCommit && WPSIeType == eQC_WPS_PROBE_RSP_IE)
    {
        //hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
        //v_CONTEXT_t pVosContext = pHostapdAdapter->pvosContext;
        WLANSAP_Update_WpsIe ( pVosContext );
    }
 
    vos_mem_free(pSap_WPSIe);   
    EXIT();
    return halStatus;
}

static int iw_softap_stopbss(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, 
        char *extra)
{
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    eHalStatus halStatus= eHAL_STATUS_SUCCESS;
    ENTER();
    if(test_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags)) {
    halStatus = WLANSAP_StopBss((WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext);
         clear_bit(SOFTAP_BSS_STARTED, &pHostapdAdapter->event_flags);
    }
    EXIT();
    return halStatus;
}
static int iw_softap_version(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, 
        char *extra)
{
#ifdef HDD_SESSIONIZE
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    int ret;
#endif
    ENTER();
    wrqu->data.flags = WE_WLAN_VERSION;
#ifdef HDD_SESSIONIZE
    //Why? Both the functions can call same internal API than doing this.
    ret = iw_get_char_setnone(pHostapdAdapter->pWlanDev, info, wrqu, extra);
    if (-EINVAL == ret)
        return -EINVAL;
#endif
    EXIT();
    return 0;
}
static int iw_set_ap_genie(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, 
        char *extra)
{
 
    hdd_adapter_t *pHostapdAdapter = (netdev_priv(dev));
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pHostapdAdapter))->pvosContext;
    eHalStatus halStatus= eHAL_STATUS_SUCCESS;
    u_int8_t *genie = wrqu->data.pointer;
    
    ENTER();
    
    if(!wrqu->data.length)
    {
        EXIT();
        return 0;
    }
    
    switch (genie[0]) 
    {
        case DOT11F_EID_WPA: 
        case DOT11F_EID_RSN:
            if((WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy == 0)
            {
                hdd_softap_Deregister_BC_STA(pHostapdAdapter);
                hdd_softap_Register_BC_STA(pHostapdAdapter, 1);
            }   
            (WLAN_HDD_GET_AP_CTX_PTR(pHostapdAdapter))->uPrivacy = 1;
            halStatus = WLANSAP_Set_WPARSNIes(pVosContext, wrqu->data.pointer, wrqu->data.length);
            break;
            
        default:
            hddLog (LOGE, "%s Set UNKNOWN IE %X",__FUNCTION__, genie[0]);
            halStatus = 0;
    }
    
    EXIT();
    return halStatus; 
}
static const iw_handler      hostapd_handler[] =
{
   (iw_handler) NULL,           /* SIOCSIWCOMMIT */
   (iw_handler) NULL,    	/* SIOCGIWNAME */
   (iw_handler) NULL,           /* SIOCSIWNWID */
   (iw_handler) NULL,           /* SIOCGIWNWID */
   (iw_handler) NULL,           /* SIOCSIWFREQ */
   (iw_handler) NULL,           /* SIOCGIWFREQ */
   (iw_handler) NULL,           /* SIOCSIWMODE */
   (iw_handler) NULL,           /* SIOCGIWMODE */
   (iw_handler) NULL,           /* SIOCSIWSENS */
   (iw_handler) NULL,           /* SIOCGIWSENS */
   (iw_handler) NULL,           /* SIOCSIWRANGE */
   (iw_handler) NULL,   	/* SIOCGIWRANGE */
   (iw_handler) NULL,           /* SIOCSIWPRIV */
   (iw_handler) NULL,           /* SIOCGIWPRIV */
   (iw_handler) NULL,           /* SIOCSIWSTATS */
   (iw_handler) NULL,           /* SIOCGIWSTATS */
   (iw_handler) NULL,           /* SIOCSIWSPY */
   (iw_handler) NULL,           /* SIOCGIWSPY */
   (iw_handler) NULL,           /* SIOCSIWTHRSPY */
   (iw_handler) NULL,           /* SIOCGIWTHRSPY */
   (iw_handler) NULL,           /* SIOCSIWAP */
   (iw_handler) NULL,   	/* SIOCGIWAP */
   (iw_handler) iw_set_mlme,    /* SIOCSIWMLME */
   (iw_handler) NULL,           /* SIOCGIWAPLIST */
   (iw_handler) NULL,           /* SIOCSIWSCAN */
   (iw_handler) NULL,           /* SIOCGIWSCAN */
   (iw_handler) NULL,      	/* SIOCSIWESSID */
   (iw_handler) NULL,      	/* SIOCGIWESSID */
   (iw_handler) NULL,           /* SIOCSIWNICKN */
   (iw_handler) NULL,           /* SIOCGIWNICKN */
   (iw_handler) NULL,           /* -- hole -- */
   (iw_handler) NULL,           /* -- hole -- */
   (iw_handler) NULL,           /* SIOCSIWRATE */
   (iw_handler) NULL,           /* SIOCGIWRATE */
   (iw_handler) NULL,           /* SIOCSIWRTS */
   (iw_handler) NULL,           /* SIOCGIWRTS */
   (iw_handler) NULL,           /* SIOCSIWFRAG */
   (iw_handler) NULL,           /* SIOCGIWFRAG */
   (iw_handler) NULL,           /* SIOCSIWTXPOW */
   (iw_handler) NULL,           /* SIOCGIWTXPOW */
   (iw_handler) NULL,           /* SIOCSIWRETRY */
   (iw_handler) NULL,           /* SIOCGIWRETRY */
   (iw_handler) NULL,           /* SIOCSIWENCODE */
   (iw_handler) NULL,           /* SIOCGIWENCODE */
   (iw_handler) NULL,           /* SIOCSIWPOWER */
   (iw_handler) NULL,           /* SIOCGIWPOWER */
   (iw_handler) NULL,           /* -- hole -- */
   (iw_handler) NULL,           /* -- hole -- */
   (iw_handler) iw_set_ap_genie,   /* SIOCSIWGENIE */
   (iw_handler) NULL,           /* SIOCGIWGENIE */
   (iw_handler) NULL,           /* SIOCSIWAUTH */
   (iw_handler) NULL,           /* SIOCGIWAUTH */
   (iw_handler) iw_set_encodeext,   /* SIOCSIWENCODEEXT */
   (iw_handler) NULL,           /* SIOCGIWENCODEEXT */
   (iw_handler) NULL,           /* SIOCSIWPMKSA */
};

#define	IW_PRIV_TYPE_OPTIE	IW_PRIV_TYPE_BYTE | QCSAP_MAX_OPT_IE
#define	IW_PRIV_TYPE_MLME \
	IW_PRIV_TYPE_BYTE | sizeof(struct ieee80211req_mlme)

static const struct iw_priv_args hostapd_private_args[] = {
  { QCSAP_IOCTL_SETPARAM,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "setparam" },
  { QCSAP_IOCTL_SETPARAM,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "" },
  { QCSAP_PARAM_MAX_ASSOC,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "setMaxAssoc" },
  { QCSAP_IOCTL_GETPARAM,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,    "getparam" },
  { QCSAP_IOCTL_GETPARAM, 0,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,    "" },
  { QCSAP_PARAM_MAX_ASSOC, 0,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,    "getMaxAssoc" },
  { QCSAP_IOCTL_COMMIT,
      IW_PRIV_TYPE_BYTE | sizeof(struct s_CommitConfig) | IW_PRIV_SIZE_FIXED, 0, "commit" },
  { QCSAP_IOCTL_SETMLME,
      IW_PRIV_TYPE_BYTE | sizeof(struct sQcSapreq_mlme)| IW_PRIV_SIZE_FIXED, 0, "setmlme" },
  { QCSAP_IOCTL_GET_STAWPAIE,
      IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 1, 0, "get_staWPAIE" },
  { QCSAP_IOCTL_SETWPAIE,
      IW_PRIV_TYPE_BYTE | QCSAP_MAX_WSC_IE | IW_PRIV_SIZE_FIXED, 0, "setwpaie" },
  { QCSAP_IOCTL_STOPBSS,
      IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED, 0, "stopbss" },
  { QCSAP_IOCTL_VERSION, 0,
      IW_PRIV_TYPE_CHAR | QCSAP_MAX_WSC_IE, "version" },
  { QCSAP_IOCTL_GET_WPS_PBC_PROBE_REQ_IES,
      IW_PRIV_TYPE_BYTE | sizeof(sQcSapreq_WPSPBCProbeReqIES_t) | IW_PRIV_SIZE_FIXED | 1, 0, "getProbeReqIEs" },
  { QCSAP_IOCTL_GET_CHANNEL, 0,
      IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | sizeof(signed long int), "getchannel" },
  { QCSAP_IOCTL_ASSOC_STA_MACADDR, 0,
      IW_PRIV_TYPE_BYTE | /*((WLAN_MAX_STA_COUNT*6)+100)*/1 , "get_assoc_stamac" },
  { QCSAP_IOCTL_DISASSOC_STA, 0,
      IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | 6 , "disassoc_sta" },
  { QCSAP_IOCTL_AP_STATS,
        IW_PRIV_TYPE_BYTE | QCSAP_MAX_WSC_IE, 0, "ap_stats" },

  { QCSAP_IOCTL_PRIV_SET_THREE_INT_GET_NONE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "" },
   /* handlers for sub-ioctl */
   {   WE_SET_WLAN_DBG,
       IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
       0, 
       "setwlandbg" },

   /* handlers for main ioctl */
   {   QCSAP_IOCTL_PRIV_SET_VAR_INT_GET_NONE,
       IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
       0, 
       "" },

   /* handlers for sub-ioctl */
   {   WE_LOG_DUMP_CMD,
       IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
       0, 
       "dump" },
};
static const iw_handler hostapd_private[] = {
   [QCSAP_IOCTL_SETPARAM - SIOCIWFIRSTPRIV] = iw_softap_setparam,  //set priv ioctl
   [QCSAP_IOCTL_GETPARAM - SIOCIWFIRSTPRIV] = iw_softap_getparam,  //get priv ioctl   
   [QCSAP_IOCTL_COMMIT   - SIOCIWFIRSTPRIV] = iw_softap_commit, //get priv ioctl   
   [QCSAP_IOCTL_SETMLME  - SIOCIWFIRSTPRIV] = iw_softap_setmlme,
   [QCSAP_IOCTL_GET_STAWPAIE - SIOCIWFIRSTPRIV] = iw_get_genie, //get station genIE
   [QCSAP_IOCTL_SETWPAIE - SIOCIWFIRSTPRIV] = iw_softap_setwpsie,
   [QCSAP_IOCTL_STOPBSS - SIOCIWFIRSTPRIV] = iw_softap_stopbss,       // stop bss
   [QCSAP_IOCTL_VERSION - SIOCIWFIRSTPRIV] = iw_softap_version,       // get driver version
   [QCSAP_IOCTL_GET_WPS_PBC_PROBE_REQ_IES - SIOCIWFIRSTPRIV] = iw_get_WPSPBCProbeReqIEs,
   [QCSAP_IOCTL_GET_CHANNEL - SIOCIWFIRSTPRIV] = iw_softap_getchannel,
   [QCSAP_IOCTL_ASSOC_STA_MACADDR - SIOCIWFIRSTPRIV] = iw_softap_getassoc_stamacaddr,
   [QCSAP_IOCTL_DISASSOC_STA - SIOCIWFIRSTPRIV] = iw_softap_disassoc_sta,
   [QCSAP_IOCTL_AP_STATS - SIOCIWFIRSTPRIV] = iw_softap_ap_stats,
   [QCSAP_IOCTL_PRIV_SET_THREE_INT_GET_NONE - SIOCIWFIRSTPRIV]  = iw_set_three_ints_getnone,   
   [QCSAP_IOCTL_PRIV_SET_VAR_INT_GET_NONE	- SIOCIWFIRSTPRIV]	 = iw_set_var_ints_getnone,
};
const struct iw_handler_def hostapd_handler_def = {
   .num_standard     = sizeof(hostapd_handler) / sizeof(hostapd_handler[0]),
   .num_private      = sizeof(hostapd_private) / sizeof(hostapd_private[0]),
   .num_private_args = sizeof(hostapd_private_args) / sizeof(hostapd_private_args[0]),
   .standard         = (iw_handler *)hostapd_handler,
   .private          = (iw_handler *)hostapd_private,
   .private_args     = hostapd_private_args,
   .get_wireless_stats = NULL,
};
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
struct net_device_ops net_ops_struct  = {
    .ndo_open = hdd_hostapd_open,
    .ndo_stop = hdd_hostapd_stop,
    .ndo_start_xmit = hdd_softap_hard_start_xmit,
    .ndo_tx_timeout = hdd_softap_tx_timeout,
    .ndo_get_stats = hdd_softap_stats,
    .ndo_set_mac_address = hdd_hostapd_set_mac_address,
    .ndo_do_ioctl = hdd_hostapd_ioctl,
    .ndo_change_mtu = hdd_hostapd_change_mtu,
    .ndo_select_queue = hdd_hostapd_select_queue,
 };
#endif

int hdd_set_hostapd(hdd_adapter_t *pAdapter)
{
    return VOS_STATUS_SUCCESS;
} 

void hdd_set_ap_ops( struct net_device *pWlanHostapdDev )
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
  pWlanHostapdDev->netdev_ops = &net_ops_struct;
#else
  pWlanHostapdDev->open = hdd_hostapd_open;
  pWlanHostapdDev->stop = hdd_hostapd_stop;
  pWlanHostapdDev->hard_start_xmit = hdd_softap_hard_start_xmit;
  pWlanHostapdDev->tx_timeout = hdd_softap_tx_timeout;
  pWlanHostapdDev->get_stats = hdd_softap_stats;
  pWlanHostapdDev->set_mac_address = hdd_hostapd_set_mac_address;
  pWlanHostapdDev->do_ioctl = hdd_hostapd_ioctl;
#endif
}

VOS_STATUS hdd_init_ap_mode( hdd_adapter_t *pAdapter )
{   
    hdd_hostapd_state_t * phostapdBuf;
    struct net_device *dev = pAdapter->dev;
    VOS_STATUS status;
    ENTER();
       // Allocate the Wireless Extensions state structure   
    phostapdBuf = WLAN_HDD_GET_HOSTAP_STATE_PTR( pAdapter );
 
    // Zero the memory.  This zeros the profile structure.
    memset(phostapdBuf, 0,sizeof(hdd_hostapd_state_t));
    
    // Set up the pointer to the Wireless Extensions state structure
    // NOP
    status = hdd_set_hostapd(pAdapter);
    if(!VOS_IS_STATUS_SUCCESS(status)) {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: hdd_set_hostapd failed!!\n"));
         return status;
    }
 
    status = vos_event_init(&phostapdBuf->vosEvent);
    if (!VOS_IS_STATUS_SUCCESS(status))
    {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, ("ERROR: Hostapd HDD vos event init failed!!\n"));
         return status;
    }
    
 
    sema_init(&(WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->semWpsPBCOverlapInd, 1);
 
     // Register as a wireless device
    dev->wireless_handlers = (struct iw_handler_def *)& hostapd_handler_def;
    
    netif_tx_disable(dev);
    netif_carrier_off(dev);
 
    //Initialize the data path module
    status = hdd_softap_init_tx_rx(pAdapter);
    if ( !VOS_IS_STATUS_SUCCESS( status ))
    {
       hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_softap_init_tx_rx failed", __FUNCTION__);
    }
    
    EXIT();
    return status;
}

hdd_adapter_t* hdd_wlan_create_ap_dev( hdd_context_t *pHddCtx, tSirMacAddr macAddr, tANI_U8 *iface_name )
{
    struct net_device *pWlanHostapdDev = NULL;
    hdd_adapter_t *pHostapdAdapter = NULL;
    v_CONTEXT_t pVosContext= NULL;

#ifdef CONFIG_CFG80211
   pWlanHostapdDev = alloc_netdev_mq(sizeof(hdd_adapter_t), iface_name, ether_setup, NUM_TX_QUEUES);
#else   
   pWlanHostapdDev = alloc_etherdev_mq(sizeof(hdd_adapter_t), NUM_TX_QUEUES);
#endif

    if(pWlanHostapdDev!=NULL)
    {
        pHostapdAdapter = netdev_priv(pWlanHostapdDev);

        //Init the net_device structure
        ether_setup(pWlanHostapdDev);

        //Initialize the adapter context to zeros.
        vos_mem_zero(pHostapdAdapter, sizeof( hdd_adapter_t ));
        pHostapdAdapter->dev = pWlanHostapdDev;
        pHostapdAdapter->pHddCtx = pHddCtx; 

        //Get the Global VOSS context.
        pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);
        //Save the adapter context in global context for future.
        ((VosContextType*)(pVosContext))->pHDDSoftAPContext = (v_VOID_t*)pHostapdAdapter;
    
        //Init the net_device structure
        strcpy(pWlanHostapdDev->name, (const char *)iface_name);

        hdd_set_ap_ops( pHostapdAdapter->dev );

        pWlanHostapdDev->tx_queue_len = NET_DEV_TX_QUEUE_LEN;
        pWlanHostapdDev->watchdog_timeo = HDD_TX_TIMEOUT;
        pWlanHostapdDev->mtu = HDD_DEFAULT_MTU;
    
        vos_mem_copy(pWlanHostapdDev->dev_addr, (void *)macAddr,sizeof(tSirMacAddr));
        vos_mem_copy(pHostapdAdapter->macAddressCurrent.bytes, (void *)macAddr, sizeof(tSirMacAddr));

        pWlanHostapdDev->destructor = free_netdev;
#ifdef CONFIG_CFG80211
        pHostapdAdapter->wdev.iftype = NL80211_IFTYPE_AP; //TODO
        pWlanHostapdDev->ieee80211_ptr = &pHostapdAdapter->wdev ;
        pHostapdAdapter->wdev.wiphy = pHddCtx->wiphy;  
        pHostapdAdapter->wdev.netdev =  pWlanHostapdDev;
#endif 
        SET_NETDEV_DEV(pWlanHostapdDev, pHddCtx->parent_dev);
    }
    return pHostapdAdapter;
}

VOS_STATUS hdd_register_hostapd( hdd_adapter_t *pAdapter, tANI_U8 rtnl_lock_held )
{
   struct net_device *dev = pAdapter->dev;
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   ENTER();
   
   hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s\n",__func__);
 
   if( rtnl_lock_held )
   {
      if (strchr(dev->name, '%')) {
         if( dev_alloc_name(dev, dev->name) < 0 )
         {
            printk(KERN_ERR"%s:Failed:dev_alloc_name",__func__);
            return VOS_STATUS_E_FAILURE;            
         }
      }
      if (register_netdevice(dev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;         
      }
   }
   else
   {
      if(register_netdev(dev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;
      }
   }
   set_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);
     
   EXIT();
   return status;
}
    
VOS_STATUS hdd_unregister_hostapd(hdd_adapter_t *pAdapter)
{
   ENTER();
   
   printk("%s", __func__);
   hdd_softap_deinit_tx_rx(pAdapter);
   pAdapter->dev->wireless_handlers = NULL;
   
   EXIT();
   return 0;
}
