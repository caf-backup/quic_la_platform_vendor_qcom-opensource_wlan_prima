/**========================================================================
  
  \file  wlan_hdd_assoc.c
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
  05/06/09     Shailender     Created module. 
  ==========================================================================*/
  
#include "wlan_hdd_includes.h"
#include <aniGlobal.h>
#include "dot11f.h"
#include "wlan_nlink_common.h"
#include "wlan_btc_svc.h"
#include "wlan_hdd_power.h"
#ifdef CONFIG_CFG80211
#include <linux/ieee80211.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#include "csrInsideApi.h"
#endif

v_BOOL_t mibIsDot11DesiredBssTypeInfrastructure( hdd_adapter_t *pAdapter );

struct ether_addr 
{
    u_char  ether_addr_octet[6];
};
// These are needed to recognize WPA and RSN suite types
#define HDD_WPA_OUI_SIZE 4
v_U8_t ccpWpaOui00[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x00 };
v_U8_t ccpWpaOui01[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x01 };
v_U8_t ccpWpaOui02[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x02 };
v_U8_t ccpWpaOui03[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x03 };
v_U8_t ccpWpaOui04[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x04 };
v_U8_t ccpWpaOui05[ HDD_WPA_OUI_SIZE ] = { 0x00, 0x50, 0xf2, 0x05 };
#define HDD_RSN_OUI_SIZE 4
v_U8_t ccpRSNOui00[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x00 }; // group cipher
v_U8_t ccpRSNOui01[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x01 }; // WEP-40 or RSN
v_U8_t ccpRSNOui02[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x02 }; // TKIP or RSN-PSK
v_U8_t ccpRSNOui03[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x03 }; // Reserved
v_U8_t ccpRSNOui04[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x04 }; // AES-CCMP
v_U8_t ccpRSNOui05[ HDD_RSN_OUI_SIZE ] = { 0x00, 0x0F, 0xAC, 0x05 }; // WEP-104

static inline v_VOID_t hdd_connSetConnectionState( hdd_adapter_t *pAdapter, eConnectionState connState )
{         
   // save the new connection state 
   pAdapter->conn_info.connState = connState;
}

// returns FALSE if not connected.
// returns TRUE for the two 'connected' states (Infra Associated or IBSS Connected ).
// returns the connection state.  Can specify NULL if you dont' want to get the actual state.
static inline v_BOOL_t hdd_connGetConnectionState( hdd_adapter_t *pAdapter, 
                                    eConnectionState *pConnState ) 
{
   v_BOOL_t fConnected; 
   eConnectionState connState;
    
   // get the connection state.
   connState = pAdapter->conn_info.connState;
   // Set the fConnected return variable based on the Connected State.  
   if ( eConnectionState_Associated == connState ||
        eConnectionState_IbssConnected == connState )
   {
      fConnected = VOS_TRUE;
   }
   else 
   {
      fConnected = VOS_FALSE;
   }
    
   if ( pConnState )
   {
      *pConnState = connState;
   }
  
   return( fConnected );
}
v_BOOL_t hdd_connIsConnected( hdd_adapter_t *pAdapter )
{
   return( hdd_connGetConnectionState( pAdapter, NULL ) );
}  
v_BOOL_t hdd_connIsConnectedInfra( hdd_adapter_t *pAdapter )
{
   v_BOOL_t fConnectedInfra = FALSE;
   eConnectionState connState;
   
   if ( hdd_connGetConnectionState( pAdapter, &connState ) )
   {   
      if ( eConnectionState_Associated == connState ) 
      {
         fConnectedInfra = TRUE;
      }   
   }
   
   return( fConnectedInfra );
}
    
static inline v_BOOL_t hdd_connGetConnectedCipherAlgo( hdd_adapter_t *pAdapter, eCsrEncryptionType *pConnectedCipherAlgo )
{
    v_BOOL_t fConnected = VOS_FALSE;
    
    fConnected = hdd_connGetConnectionState( pAdapter, NULL );
  
    if ( pConnectedCipherAlgo ) 
    {
        *pConnectedCipherAlgo = pAdapter->conn_info.ucEncryptionType;
    }
    
    return( fConnected );
}
 
inline v_BOOL_t hdd_connGetConnectedBssType( hdd_adapter_t *pAdapter, eMib_dot11DesiredBssType *pConnectedBssType )
{
    v_BOOL_t fConnected = VOS_FALSE;
    
    fConnected = hdd_connGetConnectionState( pAdapter, NULL );
  
    if ( pConnectedBssType ) 
    {
        *pConnectedBssType = pAdapter->conn_info.connDot11DesiredBssType;
    }
    
    return( fConnected );
}
static inline void hdd_connSaveConnectedBssType( hdd_adapter_t *pAdapter, eCsrRoamBssType csrRoamBssType )
{
   switch( csrRoamBssType ) 
   {
      case eCSR_BSS_TYPE_INFRASTRUCTURE:
          pAdapter->conn_info.connDot11DesiredBssType = eMib_dot11DesiredBssType_infrastructure;
         break;
                     
      case eCSR_BSS_TYPE_IBSS:
      case eCSR_BSS_TYPE_START_IBSS:
          pAdapter->conn_info.connDot11DesiredBssType = eMib_dot11DesiredBssType_independent;
         break;
           
      /** We will never set the BssType to 'any' when attempting a connection 
            so CSR should never send this back to us.*/
      case eCSR_BSS_TYPE_ANY:                      
      default:
         VOS_ASSERT( 0 );
         break;      
   }                     
    
}

void hdd_connSaveConnectInfo( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, eCsrRoamBssType eBssType )
{
   eCsrEncryptionType encryptType = eCSR_ENCRYPT_TYPE_NONE;
 
   VOS_ASSERT( pRoamInfo );
   
   if ( pRoamInfo )   
   {
      // Save the BSSID for the connection...  
      if ( eCSR_BSS_TYPE_INFRASTRUCTURE == eBssType )
      {
         VOS_ASSERT( pRoamInfo->pBssDesc );
         vos_mem_copy(pAdapter->conn_info.bssId, pRoamInfo->bssid,6 );
         // Save the Station ID for this station from the 'Roam Info'.
         //For IBSS mode, staId is assigned in NEW_PEER_IND
         //For reassoc, the staID doesn't change and it may be invalid in this structure
         //so no change here.
         if( !pRoamInfo->fReassocReq )
         {
            pAdapter->conn_info.staId [0]= pRoamInfo->staId;
         }
      }
      else if ( eCSR_BSS_TYPE_IBSS == eBssType )
      {   
         vos_mem_copy(pAdapter->conn_info.bssId, pRoamInfo->bssid,sizeof(pRoamInfo->bssid) );
      }   
      else
      {
         // can't happen.  We need a valid IBSS or Infra setting in the BSSDescription
         // or we can't function.
         VOS_ASSERT( 0 );
      }
      // notify WMM
      hdd_wmm_connect(pAdapter, pRoamInfo, eBssType);

      if( !pRoamInfo->u.pConnectedProfile )
      {
         VOS_ASSERT( pRoamInfo->u.pConnectedProfile );
      }
      else
      {
          // Get Multicast Encryption Type
          encryptType =  pRoamInfo->u.pConnectedProfile->mcEncryptionType;
          pAdapter->conn_info.mcEncryptionType = encryptType;
          // Get Unicast Encrytion Type
          encryptType =  pRoamInfo->u.pConnectedProfile->EncryptionType;
          pAdapter->conn_info.ucEncryptionType = encryptType;
          pAdapter->conn_info.authType =  pRoamInfo->u.pConnectedProfile->AuthType;
          pAdapter->conn_info.operationChannel = pRoamInfo->u.pConnectedProfile->operationChannel;

          // Save the ssid for the connection
          vos_mem_copy( &pAdapter->conn_info.SSID.SSID, &pRoamInfo->u.pConnectedProfile->SSID, sizeof( tSirMacSSid ) );
       }
   }   
      
   // save the connected BssType
   hdd_connSaveConnectedBssType( pAdapter, eBssType );
   
}
static void hdd_SendAssociationEvent(struct net_device *dev,tCsrRoamInfo *pCsrRoamInfo)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    union iwreq_data wrqu;
    int we_event;
    char *msg;
    int type = -1;
 
    memset(&wrqu, '\0', sizeof(wrqu));
    wrqu.ap_addr.sa_family = ARPHRD_ETHER; 
    we_event = SIOCGIWAP;
   
    if(eConnectionState_Associated == pAdapter->conn_info.connState)/* Associated */
    {        
        memcpy(wrqu.ap_addr.sa_data, pCsrRoamInfo->pBssDesc->bssId, sizeof(pCsrRoamInfo->pBssDesc->bssId));
        type = WLAN_STA_ASSOC_DONE_IND;
        hddLog(LOG1," associated %02x:%02x:%02x:%02x:%02x:%02x\n",
                      wrqu.ap_addr.sa_data[0],
                      wrqu.ap_addr.sa_data[1],
                      wrqu.ap_addr.sa_data[2],
                      wrqu.ap_addr.sa_data[3],
                      wrqu.ap_addr.sa_data[4],
                      wrqu.ap_addr.sa_data[5]);  
    }
    else if (eConnectionState_IbssConnected == pAdapter->conn_info.connState) // IBss Associated
    {
        memcpy(wrqu.ap_addr.sa_data, pAdapter->conn_info.bssId, sizeof(pAdapter->conn_info.bssId));
        type = WLAN_STA_ASSOC_DONE_IND;
        hddLog(LOG1," IBSS new associated %02x:%02x:%02x:%02x:%02x:%02x\n",
                      pAdapter->conn_info.bssId[0],
                      pAdapter->conn_info.bssId[1],
                      pAdapter->conn_info.bssId[2],
                      pAdapter->conn_info.bssId[3],
                      pAdapter->conn_info.bssId[4],
                      pAdapter->conn_info.bssId[5]);  
    }
    else /* Not Associated */
    { 
        type = WLAN_STA_DISASSOC_DONE_IND;
        memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
    }
    msg = NULL;
    wireless_send_event(dev, we_event, &wrqu, msg);
    send_btc_nlink_msg(type, 0);
}
void hdd_connRemoveConnectInfo( hdd_adapter_t *pAdapter )
{
   // Remove staId, bssId and peerMacAddress
   pAdapter->conn_info.staId [ 0 ] = 0;
   vos_mem_zero( &pAdapter->conn_info.bssId, sizeof( v_MACADDR_t ) );
   vos_mem_zero( &pAdapter->conn_info.peerMacAddress[ 0 ], sizeof( v_MACADDR_t ) );
   // Clear all security settings
   pAdapter->conn_info.authType         = eCSR_AUTH_TYPE_OPEN_SYSTEM;
   pAdapter->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
   pAdapter->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
   vos_mem_zero( &pAdapter->conn_info.Keys, sizeof( tCsrKeys ) );
   // Set not-connected state
   pAdapter->conn_info.connDot11DesiredBssType = eCSR_BSS_TYPE_ANY;
   hdd_connSetConnectionState( pAdapter, eConnectionState_NotConnected );  
   vos_mem_zero( &pAdapter->conn_info.SSID, sizeof( tCsrSSIDInfo ) );
}
static VOS_STATUS hdd_roamDeregisterSTA( hdd_adapter_t *pAdapter, tANI_U8 staId )
{
    VOS_STATUS vosStatus;
    hdd_disconnect_tx_rx(pAdapter);
    vosStatus = WLANTL_ClearSTAClient( pAdapter->pvosContext, staId );
    if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                    "WLANTL_ClearSTAClient() failed to for staID %d.  Status= %d [0x%08lX]",
                    staId, vosStatus, vosStatus );
    }
    return( vosStatus );
}

static eHalStatus hdd_DisConnectHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                            tANI_U32 roamId, eRoamCmdStatus roamStatus, 
                                            eCsrRoamResult roamResult )
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
  
    struct net_device *dev = pAdapter->dev;
    // notify apps that we can't pass traffic anymore
    netif_tx_disable(dev);
    netif_carrier_off(dev);
    
    hdd_connSetConnectionState( pAdapter, eConnectionState_NotConnected );

    hdd_clearRoamProfileIe( pAdapter );

    
    // indicate 'disconnect' status to wpa_supplicant...
    hdd_SendAssociationEvent(dev,pRoamInfo);
#ifdef CONFIG_CFG80211
    /* indicate disconnected event to nl80211 */
    if(roamStatus != eCSR_ROAM_IBSS_LEAVE)
    {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, 
                "%s: sent disconnected event to nl80211", 
                __func__);
        cfg80211_disconnected(dev, WLAN_REASON_UNSPECIFIED, NULL, 0, GFP_KERNEL); 
    }
#endif
    

    //We should clear all sta register with TL, for now, only one.
    status = hdd_roamDeregisterSTA( pAdapter, pAdapter->conn_info.staId [0] );
    if ( !VOS_IS_STATUS_SUCCESS(status ) )
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,"hdd_roamDeregisterSTA() failed to for staID %d.  Status= %d [0x%x]",
                    pAdapter->conn_info.staId[0], status, status );
        status = eHAL_STATUS_FAILURE;
    }
    // Clear saved connection information in HDD
    hdd_connRemoveConnectInfo( pAdapter );
    //Unblock anyone waiting for disconnect to complete
    complete(&pAdapter->disconnect_comp_var);
    return( status );
}
static VOS_STATUS hdd_roamRegisterSTA( hdd_adapter_t *pAdapter,
                                       v_BOOL_t fAuthRequired,
                                       v_U8_t staId,
                                       v_U8_t ucastSig,
                                       v_U8_t bcastSig,
                                       v_MACADDR_t *pPeerMacAddress )
{
   VOS_STATUS vosStatus = VOS_STATUS_E_FAILURE;
   WLAN_STADescType staDesc;
   eCsrEncryptionType connectedCipherAlgo;
   v_BOOL_t  fConnected;
   
   // Get the Station ID from the one saved during the assocation.
   staDesc.ucSTAId = staId;
   if ( pAdapter->conn_info.connDot11DesiredBssType == eMib_dot11DesiredBssType_infrastructure)
   { 
      staDesc.wSTAType = WLAN_STA_INFRA;
      
      // grab the bssid from the connection info in the adapter structure and hand that 
      // over to TL when registering. 
      vos_mem_copy( staDesc.vSTAMACAddress.bytes, pAdapter->conn_info.bssId,sizeof(pAdapter->conn_info.bssId) ); 
   }
   else 
   {
      // for an IBSS 'connect', setup the Station Descriptor for TL.   
      staDesc.wSTAType = WLAN_STA_IBSS;
      
      // Note that for IBSS, the STA MAC address and BSSID are goign to be different where
      // in infrastructure, they are the same (BSSID is the MAC address of the AP).  So,
      // for IBSS we have a second field to pass to TL in the STA descriptor that we don't
      // pass when making an Infrastructure connection.
      vos_mem_copy( staDesc.vSTAMACAddress.bytes, pPeerMacAddress->bytes,sizeof(pPeerMacAddress->bytes) );
      vos_mem_copy( staDesc.vBSSIDforIBSS.bytes, pAdapter->conn_info.bssId,6 );
   }
      
   vos_copy_macaddr( &staDesc.vSelfMACAddress, &pAdapter->macAddressCurrent );
   // set the QoS field appropriately
   if (hdd_wmm_is_active(pAdapter))
   {
      staDesc.ucQosEnabled = 1;
   }
   else
   {
      staDesc.ucQosEnabled = 0;
   }
   fConnected = hdd_connGetConnectedCipherAlgo( pAdapter, &connectedCipherAlgo );
   if ( connectedCipherAlgo != eCSR_ENCRYPT_TYPE_NONE )
   {
      staDesc.ucProtectedFrame = 1;
   }
   else
   {
      staDesc.ucProtectedFrame = 0;
   }
  
#ifdef FEATURE_WLAN_WAPI
   hddLog(LOG1, "%s: WAPI STA Registered: %d", __FUNCTION__, pAdapter->wapi_info.fIsWapiSta);
   if (pAdapter->wapi_info.fIsWapiSta)
   {
      staDesc.ucIsWapiSta = 1;
   }
   else
   {
      staDesc.ucIsWapiSta = 0;
   }
#endif /* FEATURE_WLAN_WAPI */

   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "HDD register TL Sec_enabled= %d.\n", staDesc.ucProtectedFrame );
   // UMA is ready we inform TL not to do frame 
   // translation for WinMob 6.1
   staDesc.ucSwFrameTXXlation = 0;
   staDesc.ucSwFrameRXXlation = 1;
   staDesc.ucAddRmvLLC = 1;
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "HDD register TL QoS_enabled=%d\n", 
              staDesc.ucQosEnabled );
   // Initialize signatures and state
   staDesc.ucUcastSig  = ucastSig;
   staDesc.ucBcastSig  = bcastSig;
   staDesc.ucInitState = fAuthRequired ?
      WLANTL_STA_CONNECTED : WLANTL_STA_AUTHENTICATED;
   // Register the Station with TL...      
   vosStatus = WLANTL_RegisterSTAClient( pAdapter->pvosContext, 
                                         hdd_rx_packet_cbk, 
                                         hdd_tx_complete_cbk, 
                                         hdd_tx_fetch_packet_cbk, &staDesc );
   
   if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                 "WLANTL_RegisterSTAClient() failed to register.  Status= %d [0x%08lX]",
                 vosStatus, vosStatus );
      return vosStatus;      
   }                                            
    
   // if ( WPA ), tell TL to go to 'connected' and after keys come to the driver, 
   // then go to 'authenticated'.  For all other authentication types (those that do 
   // not require upper layer authentication) we can put TL directly into 'authenticated'
   // state.
   
   VOS_ASSERT( fConnected );
  
   if ( !fAuthRequired )
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "open/shared auth StaId= %d.  Changing TL state to AUTHENTICATED at Join time", pAdapter->conn_info.staId[ 0 ] );
   
      // Connections that do not need Upper layer auth, transition TL directly
      // to 'Authenticated' state.      
      vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_AUTHENTICATED );
  
      pAdapter->conn_info.uIsAuthenticated = VOS_TRUE;
   }                                            
   else
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                 "ULA auth StaId= %d.  Changing TL state to CONNECTED at Join time", pAdapter->conn_info.staId[ 0 ] );
   
      vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, staDesc.ucSTAId, 
                                         WLANTL_STA_CONNECTED );
      pAdapter->conn_info.uIsAuthenticated = VOS_FALSE;
   }      
   return( vosStatus );
}

static eHalStatus hdd_AssociationCompletionHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                    tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
                                                    eCsrRoamResult roamResult )
{
    struct net_device *dev = pAdapter->dev;
    VOS_STATUS vosStatus;
 
    if ( eCSR_ROAM_RESULT_ASSOCIATED == roamResult )
    {
        hdd_connSetConnectionState( pAdapter, eConnectionState_Associated );
   
        // Save the connection info from CSR...
        hdd_connSaveConnectInfo( pAdapter, pRoamInfo, eCSR_BSS_TYPE_INFRASTRUCTURE );

#ifdef FEATURE_WLAN_WAPI
      if ( pRoamInfo->u.pConnectedProfile->AuthType == eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE ||
           pRoamInfo->u.pConnectedProfile->AuthType == eCSR_AUTH_TYPE_WAPI_WAI_PSK )
      {
         pAdapter->wapi_info.fIsWapiSta = 1;
      }
      else
      {
         pAdapter->wapi_info.fIsWapiSta = 0;
      }
#endif  /* FEATURE_WLAN_WAPI */
	
        // indicate 'connect' status to userspace
        hdd_SendAssociationEvent(dev,pRoamInfo);

        // Initialize the Linkup event completion variable 
        INIT_COMPLETION(pAdapter->linkup_event_var);

        /*
          Sometimes Switching ON the Carrier is taking time to activate the device properly. Before allowing any
          packet to go up to the application, device activation has to be ensured for proper queue mapping by the
          kernel. we have registered net device notifier for device change notification. With this we will come to 
          know that the device is getting activated properly.
	*/
			
        // Enable Linkup Event Servicing which allows the net device notifier to set the linkup event variable       
        pAdapter->isLinkUpSvcNeeded = TRUE;

        // Switch on the Carrier to activate the device
        netif_carrier_on(dev);
        
        // Wait for the Link to up to ensure all the queues are set properly by the kernel
        wait_for_completion_interruptible_timeout(&pAdapter->linkup_event_var,
                                                   msecs_to_jiffies(ASSOC_LINKUP_TIMEOUT));
        
        // Disable Linkup Event Servicing - no more service required from the net device notifier call
        pAdapter->isLinkUpSvcNeeded = FALSE;
	
        //For reassoc, the station is already registered, all we need is to change the state
        //of the STA in TL.
        //If authentication is required (WPA/WPA2/DWEP), change TL to CONNECTED instead of AUTHENTICATED
        if( !pRoamInfo->fReassocReq )
        {
#ifdef CONFIG_CFG80211
            v_U8_t reqRsnIe[DOT11F_IE_RSN_MAX_LEN];
            v_U8_t rspRsnIe[DOT11F_IE_RSN_MAX_LEN];
            tANI_U32 reqRsnLength = DOT11F_IE_RSN_MAX_LEN;
            tANI_U32 rspRsnLength = DOT11F_IE_RSN_MAX_LEN;
            /* add bss_id to cfg80211 data base */
            wlan_hdd_cfg80211_update_bss_db(pAdapter, pRoamInfo);

            /* wpa supplicant expecting WPA/RSN IE in connect result */
            csrRoamGetWpaRsnReqIE(pAdapter->hHal,
                               pAdapter->sessionId,
                               &reqRsnLength,
                               reqRsnIe);

            csrRoamGetWpaRsnRspIE(pAdapter->hHal,
                               pAdapter->sessionId,
                               &rspRsnLength,
                               rspRsnIe);

            /* inform connect result to nl80211 */
            cfg80211_connect_result(dev, pRoamInfo->bssid, 
                    reqRsnIe, reqRsnLength, 
		    rspRsnIe, rspRsnLength,
                    WLAN_STATUS_SUCCESS, 
                    GFP_KERNEL); 
#endif

            // Register the Station with TL after associated...
            vosStatus = hdd_roamRegisterSTA( pAdapter,
                                             (v_BOOL_t)pRoamInfo->fAuthRequired,
                                             pAdapter->conn_info.staId[ 0 ],
                                             pRoamInfo->ucastSig,
                                             pRoamInfo->bcastSig,
                                             NULL );
        }
        else
        {
            //Reassoc successfully
            if( pRoamInfo->fAuthRequired )
            {
                vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, pAdapter->conn_info.staId[ 0 ], 
                                         WLANTL_STA_CONNECTED );
                pAdapter->conn_info.uIsAuthenticated = VOS_FALSE;
            }
            else
            {
                vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, pAdapter->conn_info.staId[ 0 ], 
                                         WLANTL_STA_AUTHENTICATED );
                pAdapter->conn_info.uIsAuthenticated = VOS_TRUE;
            }
        }
  
        if ( VOS_IS_STATUS_SUCCESS( vosStatus ) )
        {
           // perform any WMM-related association processing
           hdd_wmm_assoc(pAdapter, pRoamInfo, eCSR_BSS_TYPE_INFRASTRUCTURE);
        }
        else
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "Cannot register STA with TL.  Failed with vosStatus = %d [%08lX]",
                       vosStatus, vosStatus );
        }

        // Start the Queue
        netif_tx_start_all_queues(dev);
    }  
    else 
    {
        /*Handle all failure conditions*/	

        hdd_connSetConnectionState( pAdapter, eConnectionState_NotConnected);
   
#ifdef CONFIG_CFG80211
        /* inform association failure event to nl80211 */
        cfg80211_connect_result(dev, 
                pRoamInfo->bssid, 
                NULL, 0, NULL, 0,
                WLAN_STATUS_UNSPECIFIED_FAILURE, 
                GFP_KERNEL);
#endif
        netif_tx_disable(dev); 
        netif_carrier_off(dev);
    }
            
    return eHAL_STATUS_SUCCESS;
}

/**============================================================================
 *
  @brief roamRoamIbssIndicationHandler() - Here we update the status of the 
  Ibss when we receive information that we have started/joined an ibss session
  We always return SUCCESS.
  
  ===========================================================================*/
static eHalStatus roamRoamIbssIndicationHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
   tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
   eCsrRoamResult roamResult )
{
   switch( roamResult )
   {
      // both IBSS Started and IBSS Join should come in here.
      case eCSR_ROAM_RESULT_IBSS_STARTED:
      case eCSR_ROAM_RESULT_IBSS_JOIN_SUCCESS:
      {
         // we should have a pRoamInfo on this callback...
         VOS_ASSERT( pRoamInfo );
        
         // When IBSS Started comes from CSR, we need to move connection state to 
         // IBSS Disconnected (meaning no peers are in the IBSS).
         hdd_connSetConnectionState( pAdapter, eConnectionState_IbssDisconnected );
         break;
      }
      
      case eCSR_ROAM_RESULT_IBSS_START_FAILED:
      {
         VOS_ASSERT( pRoamInfo );
         
         break;
      }
      
      default:
         break;
   }   
   
    return( eHAL_STATUS_SUCCESS );
}

/**============================================================================
 *
  @brief roamSaveIbssStation() - Save the IBSS peer MAC address in the adapter.
  This information is passed to iwconfig later. The peer that joined
  last is passed as information to iwconfig.
  If we add HDD_MAX_NUM_IBSS_STA or less STA we return success else we 
  return FALSE.
  
  ===========================================================================*/
static int roamSaveIbssStation( hdd_adapter_t *pAdapter, v_U8_t staId, v_MACADDR_t *peerMacAddress )
{
   int fSuccess = FALSE;
   int idx = 0;
   
   for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
   {
      if ( 0 == pAdapter->conn_info.staId[ idx ] )
      {
         pAdapter->conn_info.staId[ idx ] = staId;
      
         vos_copy_macaddr( &pAdapter->conn_info.peerMacAddress[ idx ], peerMacAddress );
         
         fSuccess = TRUE;
         break;
      }
   }
   
   return( fSuccess );   
}
/**============================================================================
 *
  @brief roamRemoveIbssStation() - Remove the IBSS peer MAC address in the adapter.
  If we remove HDD_MAX_NUM_IBSS_STA or less STA we return success else we 
  return FALSE.
  
  ===========================================================================*/
static int roamRemoveIbssStation( hdd_adapter_t *pAdapter, v_U8_t staId )
{
   int fSuccess = FALSE;
   int idx = 0;
   v_U8_t  valid_idx   = 0;
   v_U8_t  del_idx   = 0;
   
   for ( idx = 0; idx < HDD_MAX_NUM_IBSS_STA; idx++ )
   {
      if ( staId == pAdapter->conn_info.staId[ idx ] )
      {
         pAdapter->conn_info.staId[ idx ] = 0;
      
         vos_zero_macaddr( &pAdapter->conn_info.peerMacAddress[ idx ] );
         
         fSuccess = TRUE;
         // Note the deleted Index, if its 0 we need special handling
         del_idx = idx;
      }
      else
      {
         if (pAdapter->conn_info.staId[idx] != 0) 
         {
            valid_idx = idx;
         }
      }
   }
   
   // Find next active staId, to have a valid sta trigger for TL.
   if (fSuccess == TRUE) 
   {
      if (del_idx == 0) 
      {
         if (pAdapter->conn_info.staId[valid_idx] != 0) 
         {
            pAdapter->conn_info.staId[0] = pAdapter->conn_info.staId[valid_idx]; 
            vos_copy_macaddr( &pAdapter->conn_info.peerMacAddress[ 0 ], 
               &pAdapter->conn_info.peerMacAddress[ valid_idx ]);
            pAdapter->conn_info.staId[valid_idx] = 0; 
            vos_zero_macaddr( &pAdapter->conn_info.peerMacAddress[ valid_idx ] );
	 }
      }
   }
   return( fSuccess );   
}

/**============================================================================
 *
  @brief roamIbssConnectHandler() : We update the status of the IBSS to 
  connected in this function.
  
  ===========================================================================*/
static eHalStatus roamIbssConnectHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo )
{
   VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "IBSS Connect Indication from SME!!!" );        
   // Set the internal connection state to show 'IBSS Connected' (IBSS with a partner stations)...
   hdd_connSetConnectionState( pAdapter, eConnectionState_IbssConnected );
     
   // Save the connection info from CSR...
   hdd_connSaveConnectInfo( pAdapter, pRoamInfo, eCSR_BSS_TYPE_IBSS );
   // Send the bssid address to the wext.
   hdd_SendAssociationEvent(pAdapter->dev, pRoamInfo);
#ifdef CONFIG_CFG80211
   /* add bss_id to cfg80211 data base */
   wlan_hdd_cfg80211_update_bss_db(pAdapter, pRoamInfo);
   /* send ibss join indication to nl80211 */
   cfg80211_ibss_joined(pAdapter->dev, &pRoamInfo->bssid[0], GFP_KERNEL);
#endif
   
   return( eHAL_STATUS_SUCCESS );
}
/**============================================================================
 *
  @brief hdd_RoamSetKeyCompleteHandler() - Update the security parameters.
  
  ===========================================================================*/
static eHalStatus hdd_RoamSetKeyCompleteHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                 tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
                                                 eCsrRoamResult roamResult )
{
   eCsrEncryptionType connectedCipherAlgo;
   v_BOOL_t fConnected   = FALSE;
   VOS_STATUS vosStatus    = VOS_STATUS_E_FAILURE;
   ENTER();
   // if ( WPA ), tell TL to go to 'authenticated' after the keys are set.
   // then go to 'authenticated'.  For all other authentication types (those that do 
   // not require upper layer authentication) we can put TL directly into 'authenticated'
   // state.
   fConnected = hdd_connGetConnectedCipherAlgo( pAdapter, &connectedCipherAlgo );
   if( fConnected )
   {
      // TODO: Considering getting a state machine in HDD later.
      // This routuine is invoked twice. 1)set PTK 2)set GTK. The folloing if statement will be
      // TRUE when setting GTK. At this time we don't handle the state in detail.
      // Related CR: 174048 - TL not in authenticated state
      if(( eCSR_ROAM_RESULT_AUTHENTICATED == roamResult ) && (pRoamInfo != NULL) && !pRoamInfo->fAuthRequired)
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_MED,
                    "Key set for StaId= %d.  Changing TL state to AUTHENTICATED", pAdapter->conn_info.staId[ 0 ] );
                    
         // Connections that do not need Upper layer authentication, transition TL 
         // to 'Authenticated' state after the keys are set.
         vosStatus = WLANTL_ChangeSTAState( pAdapter->pvosContext, pAdapter->conn_info.staId[ 0 ], 
                                            WLANTL_STA_AUTHENTICATED );
 
         pAdapter->conn_info.uIsAuthenticated = VOS_TRUE;
      }
      
      pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
   }
   else
   {
      // possible disassoc after issuing set key and waiting set key complete
      pAdapter->roam_info.roamingState = HDD_ROAM_STATE_NONE;
   }
   
   EXIT();
   return( eHAL_STATUS_SUCCESS );
}
/**============================================================================
 *
  @brief hdd_RoamMicErrorIndicationHandler() - This function indicates the Mic failure to the supplicant.
  ===========================================================================*/
static eHalStatus hdd_RoamMicErrorIndicationHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
                                                 tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                                              eCsrRoamResult roamResult )
{   
   if( eConnectionState_Associated == pAdapter->conn_info.connState &&
      TKIP_COUNTER_MEASURE_STOPED == pAdapter->pWextState->mTKIPCounterMeasures )
   {
      struct iw_michaelmicfailure msg;
      union iwreq_data wreq;
      memset(&msg, '\0', sizeof(msg));
      msg.src_addr.sa_family = ARPHRD_ETHER;
      memcpy(msg.src_addr.sa_data, pRoamInfo->u.pMICFailureInfo->taMacAddr, sizeof(pRoamInfo->u.pMICFailureInfo->taMacAddr));
      hddLog(LOG1,"MIC MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
                                    msg.src_addr.sa_data[0],
                                    msg.src_addr.sa_data[1],
                                    msg.src_addr.sa_data[2],
                                    msg.src_addr.sa_data[3],
                                    msg.src_addr.sa_data[4],
                                    msg.src_addr.sa_data[5]);
  
      if(pRoamInfo->u.pMICFailureInfo->multicast == eCSR_ROAM_RESULT_MIC_ERROR_GROUP)
         msg.flags = IW_MICFAILURE_GROUP;
      else 
         msg.flags = IW_MICFAILURE_PAIRWISE;
      memset(&wreq, 0, sizeof(wreq));
      wreq.data.length = sizeof(msg);
      wireless_send_event(pAdapter->dev, IWEVMICHAELMICFAILURE, &wreq, (char *)&msg);
#ifdef CONFIG_CFG80211
      /* inform mic failure to nl80211 */
      cfg80211_michael_mic_failure(pAdapter->dev, 
              pRoamInfo->u.pMICFailureInfo->taMacAddr,
              ((pRoamInfo->u.pMICFailureInfo->multicast == eCSR_ROAM_RESULT_MIC_ERROR_GROUP) ?
               NL80211_KEYTYPE_GROUP :
               NL80211_KEYTYPE_PAIRWISE),
              pRoamInfo->u.pMICFailureInfo->keyId, 
              pRoamInfo->u.pMICFailureInfo->TSC, 
              GFP_KERNEL);
#endif
      
   }
   
   return( eHAL_STATUS_SUCCESS );
}

/**============================================================================
 *
  @brief roamRoamConnectStatusUpdateHandler() - The Ibss connection status is 
  updated regularly here in this function.
  
  ===========================================================================*/
static eHalStatus roamRoamConnectStatusUpdateHandler( hdd_adapter_t *pAdapter, tCsrRoamInfo *pRoamInfo, 
   tANI_U32 roamId, eRoamCmdStatus roamStatus,                                                
   eCsrRoamResult roamResult )
{
   VOS_STATUS vosStatus;

   switch( roamResult )
   {
      case eCSR_ROAM_RESULT_IBSS_NEW_PEER:
      {
         VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, "IBSS New Peer indication from SME "
                    "with peerMac %2x-%2x-%2x-%2x-%2x-%2x  and  stationID= %d",
                    pRoamInfo->peerMac[0], pRoamInfo->peerMac[1], pRoamInfo->peerMac[2],
                    pRoamInfo->peerMac[3], pRoamInfo->peerMac[4], pRoamInfo->peerMac[5], 
                    pRoamInfo->staId );
         
         if ( !roamSaveIbssStation( pAdapter, pRoamInfo->staId, (v_MACADDR_t *)pRoamInfo->peerMac ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                       "New IBSS peer but we already have the max we can handle.  Can't register this one" );
            break;
         }
         // Register the Station with TL for the new peer. 
         vosStatus = hdd_roamRegisterSTA( pAdapter,
                                          pRoamInfo->fAuthRequired,
                                          pRoamInfo->staId,
                                          pRoamInfo->ucastSig,
                                          pRoamInfo->bcastSig,
                                          (v_MACADDR_t *)pRoamInfo->peerMac );
         if ( !VOS_IS_STATUS_SUCCESS( vosStatus ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "Cannot register STA with TL for IBSS.  Failed with vosStatus = %d [%08lX]",
               vosStatus, vosStatus );
         }
         
         netif_carrier_on(pAdapter->dev);
	 netif_tx_start_all_queues(pAdapter->dev);
         break;
      }
         
      case eCSR_ROAM_RESULT_IBSS_CONNECT:
      {
      
         roamIbssConnectHandler( pAdapter, pRoamInfo );
         
         break;
      }   
      case eCSR_ROAM_RESULT_IBSS_PEER_DEPARTED:
      {
         if ( !roamRemoveIbssStation( pAdapter, pRoamInfo->staId ) )
         {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_WARN,
                    "IBSS peer departed by cannot find peer in our registration table with TL" );
         }
         hdd_roamDeregisterSTA( pAdapter, pRoamInfo->staId );

         break;
      }
      case eCSR_ROAM_RESULT_IBSS_INACTIVE:
      {
         // Stop only when we are inactive
         netif_tx_disable(pAdapter->dev);
         netif_carrier_off(pAdapter->dev);
         hdd_connSetConnectionState( pAdapter, eConnectionState_NotConnected );
         
         // Send the bssid address to the wext.
         hdd_SendAssociationEvent(pAdapter->dev, pRoamInfo);
         // clean up data path
         hdd_disconnect_tx_rx(pAdapter);
         break;
      }
         
      default:
         break;
   
   }
   
   return( eHAL_STATUS_SUCCESS );
}

eHalStatus hdd_smeRoamCallback( void *pContext, tCsrRoamInfo *pRoamInfo, tANI_U32 roamId, 
                                eRoamCmdStatus roamStatus, eCsrRoamResult roamResult )
{
    eHalStatus halStatus = eHAL_STATUS_SUCCESS;
    hdd_adapter_t *pAdapter = (hdd_adapter_t *)pContext;
    hdd_wext_state_t *pWextState= pAdapter->pWextState;
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,"CSR Callback: status= %d result= %d roamID=%ld", 
                    roamStatus, roamResult, roamId ); 
    switch( roamStatus )
    {
            
        case eCSR_ROAM_SHOULD_ROAM:
           // Dont need to do anything
           break;
        case eCSR_ROAM_LOSTLINK:
        case eCSR_ROAM_DISASSOCIATED:
        case eCSR_ROAM_IBSS_LEAVE:
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "****eCSR_ROAM_DISASSOCIATED****");
            halStatus = hdd_DisConnectHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );

            /* Check if Mcast/Bcast Filters are set, if yes clear the filters here */
            if(pAdapter->hdd_mcastbcast_filter_set == TRUE) {
                  hdd_conf_mcastbcast_filter(pAdapter, FALSE);
                  pAdapter->hdd_mcastbcast_filter_set = FALSE;
            }

            break;
                    
        case eCSR_ROAM_ASSOCIATION_COMPLETION:
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH, "****eCSR_ROAM_ASSOCIATION_COMPLETION****");
            if (  (roamResult != eCSR_ROAM_RESULT_ASSOCIATED)
               && (   (pWextState->roamProfile.EncryptionType.encryptionType[0] == eCSR_ENCRYPT_TYPE_WEP40_STATICKEY) 
                   || (pWextState->roamProfile.EncryptionType.encryptionType[0] == eCSR_ENCRYPT_TYPE_WEP104_STATICKEY)
                  )
               && (eCSR_AUTH_TYPE_SHARED_KEY != pAdapter->conn_info.authType)
               )
            {
                v_U32_t roamId = 0;
                VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO_HIGH,
                        "****WEP open authentication failed, trying with shared authentication****");
                pAdapter->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
                pWextState->roamProfile.AuthType.authType[0] = pAdapter->conn_info.authType;
                halStatus = sme_RoamConnect( pAdapter->hHal, pAdapter->sessionId, &(pWextState->roamProfile), &roamId);
            }
            else
            {
                halStatus = hdd_AssociationCompletionHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            }

            break;
        case eCSR_ROAM_IBSS_IND:
            halStatus = roamRoamIbssIndicationHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;
        
        case eCSR_ROAM_CONNECT_STATUS_UPDATE:
            halStatus = roamRoamConnectStatusUpdateHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;            
        
        case eCSR_ROAM_MIC_ERROR_IND:
            halStatus = hdd_RoamMicErrorIndicationHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
            break;

        case eCSR_ROAM_SET_KEY_COMPLETE:
           halStatus = hdd_RoamSetKeyCompleteHandler( pAdapter, pRoamInfo, roamId, roamStatus, roamResult );
           break;
        default:
            break;
    }
    return( halStatus );
}
#ifdef WLAN_SOFTAP_FEATURE
eCsrAuthType 
hdd_TranslateRSNToCsrAuthType( u_int8_t auth_suite[4]) 
#else
static eCsrAuthType hdd_TranslateRSNToCsrAuthType( u_int8_t auth_suite[4]) 
#endif
{
    eCsrAuthType auth_type;
    // is the auth type supported?
    if ( memcmp(auth_suite , ccpRSNOui01, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_RSN;
    } else 
    if (memcmp(auth_suite , ccpRSNOui02, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_RSN_PSK;
    } else 
    { 
        auth_type = eCSR_AUTH_TYPE_UNKNOWN;
    }
    return auth_type;
} 
#ifdef WLAN_SOFTAP_FEATURE
eCsrAuthType 
hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]) 
#else
static eCsrAuthType hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]) 
#endif
{
    eCsrAuthType auth_type;
    // is the auth type supported?
    if ( memcmp(auth_suite , ccpWpaOui01, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_WPA;
    } else 
    if (memcmp(auth_suite , ccpWpaOui02, 4) == 0) 
    {
        auth_type = eCSR_AUTH_TYPE_WPA_PSK;
    } else 
    { 
        auth_type = eCSR_AUTH_TYPE_UNKNOWN;
    }
    hddLog(LOGE, FL("%s: auth_type: %d\n"), __FUNCTION__, auth_type);
    return auth_type;
}
#ifdef WLAN_SOFTAP_FEATURE
eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4])
#else
static eCsrEncryptionType hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4])                                    
#endif
{
    eCsrEncryptionType cipher_type;
    // is the cipher type supported?
    if ( memcmp(cipher_suite , ccpRSNOui04, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_AES;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui02, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui00, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_NONE;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui01, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    } 
    else if (memcmp(cipher_suite , ccpRSNOui05, 4) == 0) 
    {        
        cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY; 
    } 
    else 
    { 
        cipher_type = eCSR_ENCRYPT_TYPE_FAILED;
    }
    hddLog(LOGE, FL("%s: cipher_type: %d\n"), __FUNCTION__, cipher_type);
    return cipher_type;
} 
/* To find if the MAC address is NULL */
static tANI_U8 hdd_IsMACAddrNULL (tANI_U8 *macAddr, tANI_U8 length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (0x00 != (macAddr[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
} /****** end hdd_IsMACAddrNULL() ******/
#ifdef WLAN_SOFTAP_FEATURE
eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4])
#else
static eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4])                                    
#endif
{
    eCsrEncryptionType cipher_type;
    // is the cipher type supported?
    if ( memcmp(cipher_suite , ccpWpaOui04, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_AES;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui02, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_TKIP;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui00, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_NONE;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui01, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
    } else 
    if (memcmp(cipher_suite , ccpWpaOui05, 4) == 0) 
    {
        cipher_type = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY; 
    } else 
    { 
        cipher_type = eCSR_ENCRYPT_TYPE_FAILED;
    }
    hddLog(LOGE, FL("%s: cipher_type: %d\n"), __FUNCTION__, cipher_type);
    return cipher_type;
} 

static tANI_S32 hdd_ProcessGENIE(hdd_adapter_t *pAdapter, 
                struct ether_addr *pBssid, 
                eCsrEncryptionType *pEncryptType, 
                eCsrEncryptionType *mcEncryptType, 
                eCsrAuthType *pAuthType, 
                u_int16_t gen_ie_len, 
                u_int8_t *gen_ie) 
{
    tHalHandle halHandle = pAdapter->hHal;
    eHalStatus result; 
    tDot11fIERSN dot11RSNIE; 
    tDot11fIEWPA dot11WPAIE; 
    tANI_U32 i; 
    tANI_U8 *pRsnIe; 
    tANI_U16 RSNIeLen; 
    tPmkidCacheInfo PMKIDCache[4]; // Local transfer memory

    /* Clear struct of tDot11fIERSN and tDot11fIEWPA specifically setting present
       flag to 0 */
    memset( &dot11WPAIE, 0 , sizeof(tDot11fIEWPA) );
    memset( &dot11RSNIE, 0 , sizeof(tDot11fIERSN) );

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
            return -EINVAL;
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
        hddLog(LOGE, FL("%s: pairwise cipher suite count: %d\n"), 
                __FUNCTION__, dot11RSNIE.pwise_cipher_suite_count );
        hddLog(LOGE, FL("%s: authentication suite count: %d\n"), 
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
        for (i=0; i<dot11RSNIE.pmkid_count; i++) 
        {
            if ( pBssid == NULL) 
            {
                break;
            }
            if ( hdd_IsMACAddrNULL( (u_char *) pBssid , sizeof( (char *) pBssid))) 
            {
                break;
            }
            // For right now, I assume setASSOCIATE() has passed in the bssid.  
            vos_mem_copy(PMKIDCache[i].BSSID, 
                            pBssid, ETHER_ADDR_LEN);
            vos_mem_copy(PMKIDCache[i].PMKID, 
                            dot11RSNIE.pmkid[i],   
                            CSR_RSN_PMKID_SIZE);
        }  
        // Calling csrRoamSetPMKIDCache to configure the PMKIDs into the cache
        hddLog(LOGE, FL("%s: Calling csrRoamSetPMKIDCache with cache entry %ld.\n"), 
                                                                            __FUNCTION__, i );
        // Finally set the PMKSA ID Cache in CSR
        result = sme_RoamSetPMKIDCache(halHandle,pAdapter->sessionId, 
                                        PMKIDCache, 
                                        dot11RSNIE.pmkid_count );
    } else 
    if (gen_ie[0] == DOT11F_EID_WPA) 
    {         
        // Validity checks
        int invalid = FALSE;
        int wpaIe = FALSE;
        int wpsIe = FALSE;
        if (memcmp(&gen_ie[2], "\x00\x50\xf2\x04", 4) == 0) {
            wpsIe = TRUE;
            if(gen_ie_len < SIR_MAC_WSC_IE_MIN_LENGTH || gen_ie_len > SIR_MAC_WSC_IE_MAX_LENGTH) 
                invalid = TRUE;
        }
        if (memcmp(&gen_ie[2], "\x00\x50\xf2\x01", 4) == 0) {
            wpaIe = TRUE;
            if ((gen_ie_len < DOT11F_IE_WPA_MIN_LEN ) ||  
                    (gen_ie_len > DOT11F_IE_WPA_MAX_LEN))
                invalid  = TRUE;
        }
        if (invalid)    
        {
            return -EINVAL;
        }
    
        if (wpsIe) {
            /* error return code for caller to ignore
            pAuthType, pEncryptType, and mcEncryptType. 
            supplicant is supposed to call iw_set_auth() before wpsIe */
            return -EINVAL;
        }
        if (wpaIe) {
        // Skip past the EID byte and length byte - and four byte WiFi OUI  
        pRsnIe = gen_ie + 2 + 4; 
        RSNIeLen = gen_ie_len - (2 + 4); 
        // Unpack the WPA IE 
        dot11fUnpackIeWPA((tpAniSirGlobal) halHandle, 
                            pRsnIe, 
                            RSNIeLen, 
                            &dot11WPAIE);
        // Copy out the encryption and authentication types 
        hddLog(LOGE, FL("%s: WPA unicast cipher suite count: %d\n"), 
                __FUNCTION__, dot11WPAIE.unicast_cipher_count );
        hddLog(LOGE, FL("%s: WPA authentication suite count: %d\n"), 
                __FUNCTION__, dot11WPAIE.auth_suite_count);
        //dot11WPAIE.auth_suite_count
        // Just translate the FIRST one 
        *pAuthType =  hdd_TranslateWPAToCsrAuthType(dot11WPAIE.auth_suites[0]); 
        //dot11WPAIE.unicast_cipher_count 
        *pEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.unicast_ciphers[0]);                       
        //dot11WPAIE.unicast_cipher_count 
        *mcEncryptType = hdd_TranslateWPAToCsrEncryptionType(dot11WPAIE.multicast_cipher);                       
        }
    } 
    else 
    { 
        hddLog(LOGE, FL("%s: gen_ie[0]: %d\n"), __FUNCTION__, gen_ie[0]);
        return -EINVAL; 
    }
    return 0;
}
int hdd_SetGENIEToCsr( hdd_adapter_t *pAdapter)
{
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    v_U32_t status = 0;
    eCsrAuthType RSNAuthType;
    eCsrEncryptionType RSNEncryptType;
    eCsrEncryptionType mcRSNEncryptType;
    struct ether_addr   bSsid;   // MAC address of assoc peer
    // MAC address of assoc peer
    // But, this routine is only called when we are NOT associated.
    vos_mem_copy(bSsid.ether_addr_octet,
            pWextState->roamProfile.BSSIDs.bssid,
            sizeof(bSsid.ether_addr_octet));
    if (pWextState->WPARSNIE[0] == DOT11F_EID_RSN || pWextState->WPARSNIE[0] == DOT11F_EID_WPA)
    {
        //continue
    } 
    else
    {
        return 0;
    }
    // The actual processing may eventually be more extensive than this.
    // Right now, just consume any PMKIDs that are  sent in by the app.
    status = hdd_ProcessGENIE(pAdapter,
            &bSsid,   // MAC address of assoc peer
            &RSNEncryptType,
            &mcRSNEncryptType,
            &RSNAuthType,
            pWextState->WPARSNIE[1]+2,
            pWextState->WPARSNIE);
    if (status == 0)
    {
        // Now copy over all the security attributes you have parsed out
        pWextState->roamProfile.EncryptionType.numEntries = 1;
        pWextState->roamProfile.mcEncryptionType.numEntries = 1;
        
        pWextState->roamProfile.EncryptionType.encryptionType[0] = RSNEncryptType; // Use the cipher type in the RSN IE
        pWextState->roamProfile.mcEncryptionType.encryptionType[0] = mcRSNEncryptType;
        hddLog( LOG1, "%s: CSR AuthType = %d, EncryptionType = %d mcEncryptionType = %d\n", __FUNCTION__, RSNAuthType, RSNEncryptType, mcRSNEncryptType);
    }
    return 0;
}
int hdd_set_csr_auth_type ( hdd_adapter_t  *pAdapter)
{
    hdd_wext_state_t *pWextState = pAdapter->pWextState;
    tCsrRoamProfile* pRoamProfile = &(pWextState->roamProfile);
    ENTER();
    
    pRoamProfile->AuthType.numEntries = 1;
      
    switch( pAdapter->conn_info.authType)
    {
       case eCSR_AUTH_TYPE_OPEN_SYSTEM:
         
        if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_DISABLED) {           
           
           pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM ;
        }
        else if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA) {
           
            if(pWextState->authKeyMgmt == IW_AUTH_KEY_MGMT_802_1X) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA;   
            } 
            else if (pWextState->authKeyMgmt == IW_AUTH_KEY_MGMT_PSK) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA_PSK;
            } 
            else {     
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WPA_NONE;
            }    
        }
        if (pWextState->wpaVersion & IW_AUTH_WPA_VERSION_WPA2) {
           
            if(pWextState->authKeyMgmt == IW_AUTH_KEY_MGMT_802_1X) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_RSN;   
            } 
            else if (pWextState->authKeyMgmt == IW_AUTH_KEY_MGMT_PSK) {
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_RSN_PSK;
            } 
            else {             
               pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
            }    
        }
        break;     
         
       case eCSR_AUTH_TYPE_SHARED_KEY:
         
          pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;  
          break;
        default:
         
           pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_UNKNOWN;
           break;
    }
   
    hddLog( LOG1, "%s Set roam Authtype to %d\n",
            __FUNCTION__, pWextState->roamProfile.AuthType.authType[0]);
   
   EXIT();
    return 0;
}

/**---------------------------------------------------------------------------
  
  \brief iw_set_essid() - 
   This function sets the ssid received from wpa_supplicant
   to the CSR roam profile. 
   
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/

int iw_set_essid(struct net_device *dev, 
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra)
{
    v_U32_t status = 0;
    hdd_wext_state_t *pWextState;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    v_U32_t roamId;
    tCsrRoamProfile          *pRoamProfile;
    eMib_dot11DesiredBssType connectedBssType;
 
    pWextState = pAdapter->pWextState;
    
    ENTER();
  
    if(pWextState->mTKIPCounterMeasures == TKIP_COUNTER_MEASURE_STARTED) {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s :Counter measure is in progress\n", __func__);
        return -EBUSY;
    }
    if( SIR_MAC_MAX_SSID_LENGTH < wrqu->essid.length )
        return -EINVAL;
    pRoamProfile = &pWextState->roamProfile;
    if (pRoamProfile) 
    {
        if ( hdd_connGetConnectedBssType( pAdapter, &connectedBssType ) ||
             ( eMib_dot11DesiredBssType_independent == pAdapter->conn_info.connDot11DesiredBssType ))
        {
            VOS_STATUS vosStatus;
            // need to issue a disconnect to CSR.
            INIT_COMPLETION(pAdapter->disconnect_comp_var);
            vosStatus = sme_RoamDisconnect( pAdapter->hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED );

            if(VOS_STATUS_SUCCESS == vosStatus)
               wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
                     msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
        }
    }
    status = hdd_wmm_get_uapsd_mask(pAdapter,
                                    &pWextState->roamProfile.uapsd_mask);
    if (VOS_STATUS_SUCCESS != status)
    {
       pWextState->roamProfile.uapsd_mask = 0;
    }
    pWextState->roamProfile.SSIDs.numOfSSIDs = 1;
     
    pWextState->roamProfile.SSIDs.SSIDList->SSID.length = wrqu->essid.length;
   
    vos_mem_zero(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId, sizeof(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId)); 
    vos_mem_copy((void *)(pWextState->roamProfile.SSIDs.SSIDList->SSID.ssId), extra, wrqu->essid.length);
    if (IW_AUTH_WPA_VERSION_WPA == pWextState->wpaVersion ||
        IW_AUTH_WPA_VERSION_WPA2 == pWextState->wpaVersion ) {
   
        //set gen ie
        hdd_SetGENIEToCsr(pAdapter);
        //set auth
        hdd_set_csr_auth_type(pAdapter);
    }
 
#ifdef FEATURE_WLAN_WAPI
    hddLog(LOG1, "%s: Setting WAPI AUTH Type and Encryption Mode values", __FUNCTION__);
    if (pAdapter->wapi_info.nWapiMode)
    {
        switch (pAdapter->wapi_info.wapiAuthMode)
        {
            case WAPI_AUTH_MODE_PSK:
            {
                hddLog(LOG1, "%s: WAPI AUTH TYPE: PSK: %d", __FUNCTION__, pAdapter->wapi_info.wapiAuthMode);
                pRoamProfile->AuthType.numEntries = 1;
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_PSK;
                break;
            }
            case WAPI_AUTH_MODE_CERT:
            {
                hddLog(LOG1, "%s: WAPI AUTH TYPE: CERT: %d", __FUNCTION__, pAdapter->wapi_info.wapiAuthMode);
                pRoamProfile->AuthType.numEntries = 1;
                pRoamProfile->AuthType.authType[0] = eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
                break;
            }
        } // End of switch
        if ( pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_PSK ||
             pAdapter->wapi_info.wapiAuthMode == WAPI_AUTH_MODE_CERT)
        {
            hddLog(LOG1, "%s: WAPI PAIRWISE/GROUP ENCRYPTION: WPI", __FUNCTION__);
            pRoamProfile->EncryptionType.numEntries = 1;
            pRoamProfile->EncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
            pRoamProfile->mcEncryptionType.numEntries = 1;
            pRoamProfile->mcEncryptionType.encryptionType[0] = eCSR_ENCRYPT_TYPE_WPI;
        }
    }
#endif /* FEATURE_WLAN_WAPI */

    // Disable auto BMPS entry by PMC until DHCP is done
    sme_SetDHCPTillPowerActiveFlag(pAdapter->hHal, TRUE);
    
    status = sme_RoamConnect( pAdapter->hHal,pAdapter->sessionId, &(pWextState->roamProfile),&roamId);
    
    pRoamProfile->ChannelInfo.ChannelList = NULL; 
    pRoamProfile->ChannelInfo.numOfChannels = 0;
    
    EXIT(); 
    return status;
}
/**---------------------------------------------------------------------------
  
  \brief iw_get_essid() - 
   This function returns the essid to the wpa_supplicant.
   
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/
int iw_get_essid(struct net_device *dev, 
                       struct iw_request_info *info,
                       struct iw_point *dwrq, char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t *wextBuf = pAdapter->pWextState;
   ENTER();
   if((pAdapter->conn_info.connState == eConnectionState_Associated &&
     wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0) ||
      ((pAdapter->conn_info.connState == eConnectionState_IbssConnected ||
        pAdapter->conn_info.connState == eConnectionState_IbssDisconnected) &&
        wextBuf->roamProfile.SSIDs.SSIDList->SSID.length > 0))
   {
       dwrq->length = pAdapter->conn_info.SSID.SSID.length;
       memcpy(extra, pAdapter->conn_info.SSID.SSID.ssId, dwrq->length);
       dwrq->flags = 1;
   } else {
       memset(extra, 0, dwrq->length);
       dwrq->length = 0;
       dwrq->flags = 0;
   }
   EXIT();
   return 0;
}
/**---------------------------------------------------------------------------
  
  \brief iw_set_auth() - 
   This function sets the auth type received from the wpa_supplicant.
   
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/
int iw_set_auth(struct net_device *dev,struct iw_request_info *info,
                        union iwreq_data *wrqu,char *extra)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   hdd_wext_state_t *pWextState = pAdapter->pWextState;   
   tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
   eCsrEncryptionType mcEncryptionType;   
   eCsrEncryptionType ucEncryptionType;
   
   ENTER();
   switch(wrqu->param.flags & IW_AUTH_INDEX)
   {
      case IW_AUTH_WPA_VERSION:
        
         pWextState->wpaVersion = wrqu->param.value;
       
         break;
   
   case IW_AUTH_CIPHER_PAIRWISE:
   {
      if(wrqu->param.value & IW_AUTH_CIPHER_NONE) {            
         ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
      }           
      else if(wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
         ucEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
      }            
      else if(wrqu->param.value & IW_AUTH_CIPHER_CCMP) {
         ucEncryptionType = eCSR_ENCRYPT_TYPE_AES;
      }    
            
     else if(wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
           
         if( (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType) )
                /*Dynamic WEP key*/
             ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP40;     
         else
                /*Static WEP key*/
             ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;              
      }      
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP104) {
           
         if( (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
                  /*Dynamic WEP key*/
            ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP104;
         else
                /*Static WEP key*/
            ucEncryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
               
         }
         else {
           
               hddLog(LOG1,"%s value %d UNKNOWN IW_AUTH_CIPHER\n",__FUNCTION__, wrqu->param.value); 
               return -EINVAL;
         }
       
         pRoamProfile->EncryptionType.numEntries = 1;
         pRoamProfile->EncryptionType.encryptionType[0] = ucEncryptionType;
      }     
      break;
      case IW_AUTH_CIPHER_GROUP:
      {            
          if(wrqu->param.value & IW_AUTH_CIPHER_NONE) {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_TKIP) {
             mcEncryptionType = eCSR_ENCRYPT_TYPE_TKIP;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_CCMP) {              
              mcEncryptionType = eCSR_ENCRYPT_TYPE_AES;
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP40) {
           
         if((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                                            (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))  
                                            
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP40;
            
         else            
               mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP40_STATICKEY; 
      }
        
      else if(wrqu->param.value & IW_AUTH_CIPHER_WEP104) 
      {     
             /*Dynamic WEP keys won't work with shared keys*/
         if( (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) && 
                           (eCSR_AUTH_TYPE_OPEN_SYSTEM == pAdapter->conn_info.authType))
         {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP104;
         }
         else
         {
            mcEncryptionType = eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
         }
      }
      else {
           
          hddLog(LOG1,"%s value %d UNKNOWN IW_AUTH_CIPHER\n",__FUNCTION__, wrqu->param.value); 
          return -EINVAL;
       }
              
         pRoamProfile->mcEncryptionType.numEntries = 1;
         pRoamProfile->mcEncryptionType.encryptionType[0] = mcEncryptionType;
      }
      break;
   
      case IW_AUTH_80211_AUTH_ALG: 
      {
           /*Save the auth algo here and set auth type to SME Roam profile
                in the iw_set_ap_address*/
          if( wrqu->param.value & IW_AUTH_ALG_OPEN_SYSTEM)    
             pAdapter->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
          
          else if(wrqu->param.value & IW_AUTH_ALG_SHARED_KEY)
             pAdapter->conn_info.authType = eCSR_AUTH_TYPE_SHARED_KEY;
          else if(wrqu->param.value & IW_AUTH_ALG_LEAP)
            /*Not supported*/
             pAdapter->conn_info.authType = eCSR_AUTH_TYPE_UNKNOWN;
          pWextState->roamProfile.AuthType.authType[0] = pAdapter->conn_info.authType;
      }
      break;
      
      case IW_AUTH_KEY_MGMT:
      {
           /*Save the key management*/
         pWextState->authKeyMgmt = wrqu->param.value;
      }
      break;
      
      case IW_AUTH_TKIP_COUNTERMEASURES:
      {
         if(wrqu->param.value) {
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"Counter Measure started %d\n", wrqu->param.value);
            pWextState->mTKIPCounterMeasures = TKIP_COUNTER_MEASURE_STARTED;
         }  
         else {   
            hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"Counter Measure stopped=%d\n", wrqu->param.value);
            pWextState->mTKIPCounterMeasures = TKIP_COUNTER_MEASURE_STOPED;
         }  
      }   
      break;
      case IW_AUTH_DROP_UNENCRYPTED:
      case IW_AUTH_WPA_ENABLED:
      case IW_AUTH_RX_UNENCRYPTED_EAPOL:
      case IW_AUTH_ROAMING_CONTROL:
      case IW_AUTH_PRIVACY_INVOKED:         
         
      default:
         
         hddLog(LOG1,"%s called with unsupported auth type %d\n",__FUNCTION__, 
               wrqu->param.flags & IW_AUTH_INDEX);
      break;
   }
   
   EXIT();
   return 0;
}
/**---------------------------------------------------------------------------
  
  \brief iw_get_auth() - 
   This function returns the auth type to the wpa_supplicant.
   
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/
int iw_get_auth(struct net_device *dev,struct iw_request_info *info,
                         union iwreq_data *wrqu,char *extra)
{
    hdd_adapter_t* pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_wext_state_t *pWextState= pAdapter->pWextState;
    tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
    ENTER();
    switch(pRoamProfile->negotiatedAuthType)
    {
        case eCSR_AUTH_TYPE_WPA_NONE:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value =  IW_AUTH_WPA_VERSION_DISABLED;
            break;
        case eCSR_AUTH_TYPE_WPA:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value = IW_AUTH_WPA_VERSION_WPA;
            break;
        case eCSR_AUTH_TYPE_RSN:
            wrqu->param.flags = IW_AUTH_WPA_VERSION;
            wrqu->param.value =  IW_AUTH_WPA_VERSION_WPA2;
            break;
         case eCSR_AUTH_TYPE_OPEN_SYSTEM:
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_SHARED_KEY:
             wrqu->param.value =  IW_AUTH_ALG_SHARED_KEY;
             break;
         case eCSR_AUTH_TYPE_UNKNOWN:
             hddLog(LOG1,"%s called with unknown auth type",__FUNCTION__);
             wrqu->param.value =  IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_AUTOSWITCH:
             wrqu->param.value =  IW_AUTH_ALG_OPEN_SYSTEM;
             break;
         case eCSR_AUTH_TYPE_WPA_PSK:
             hddLog(LOG1,"%s called with unknown auth type",__FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
         case eCSR_AUTH_TYPE_RSN_PSK:
             hddLog(LOG1,"%s called with unknown auth type",__FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
         default:
             hddLog(LOG1,"%s called with unknown auth type",__FUNCTION__);
             wrqu->param.value = IW_AUTH_ALG_OPEN_SYSTEM;
             return -EIO;
    }
    if(((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_PAIRWISE))
    {
        switch(pRoamProfile->negotiatedUCEncryptionType)
        {
        case eCSR_ENCRYPT_TYPE_NONE:
            wrqu->param.value = IW_AUTH_CIPHER_NONE;
            break;
        case eCSR_ENCRYPT_TYPE_WEP40:
        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
            wrqu->param.value = IW_AUTH_CIPHER_WEP40;
            break;
        case eCSR_ENCRYPT_TYPE_TKIP:
            wrqu->param.value = IW_AUTH_CIPHER_TKIP;
            break;
         case eCSR_ENCRYPT_TYPE_WEP104:
         case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
             wrqu->param.value = IW_AUTH_CIPHER_WEP104;
             break;
         case eCSR_ENCRYPT_TYPE_AES:
             wrqu->param.value = IW_AUTH_CIPHER_CCMP;
             break;
         default:
             hddLog(LOG1,"%s called with unknown auth type\n",__FUNCTION__);
            return -EIO;
       }
   }

    if(((wrqu->param.flags & IW_AUTH_INDEX) == IW_AUTH_CIPHER_GROUP)) 
    {
        switch(pRoamProfile->negotiatedMCEncryptionType)
        {
        case eCSR_ENCRYPT_TYPE_NONE:
            wrqu->param.value = IW_AUTH_CIPHER_NONE;
            break;
        case eCSR_ENCRYPT_TYPE_WEP40:
        case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
            wrqu->param.value = IW_AUTH_CIPHER_WEP40;
            break;
        case eCSR_ENCRYPT_TYPE_TKIP:
            wrqu->param.value = IW_AUTH_CIPHER_TKIP;
            break;
         case eCSR_ENCRYPT_TYPE_WEP104:
         case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
             wrqu->param.value = IW_AUTH_CIPHER_WEP104;
             break;
         case eCSR_ENCRYPT_TYPE_AES:
             wrqu->param.value = IW_AUTH_CIPHER_CCMP;
             break;
         default:
             hddLog(LOG1,"%s called with unknown auth type\n",__FUNCTION__);
            return -EIO;
       }
   }

    hddLog(LOG1,"%s called with auth type %d\n",__FUNCTION__,pRoamProfile->AuthType.authType[0]);
    EXIT();
    return 0;
}
/**---------------------------------------------------------------------------
  
  \brief iw_set_ap_address() - 
   This function calls the sme_RoamConnect function to associate 
   to the AP with the specified BSSID received from the wpa_supplicant.
   
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/
int iw_set_ap_address(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    v_U8_t  *pMacAddress=NULL;
    ENTER();
    pMacAddress = (v_U8_t*) wrqu->ap_addr.sa_data;
    VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO, "%02x:%02x:%02x:%02x:%02x:%02x",pMacAddress[0],pMacAddress[1],
          pMacAddress[2],pMacAddress[3],pMacAddress[4],pMacAddress[5]);
    vos_mem_copy( pAdapter->conn_info.bssId, pMacAddress, sizeof( tCsrBssid ));
    EXIT();
   
    return 0;
}
/**---------------------------------------------------------------------------
  
  \brief iw_get_ap_address() - 
   This function returns the BSSID to the wpa_supplicant
  \param  - dev - Pointer to the net device.
              - info - Pointer to the iw_request_info.
              - wrqu - Pointer to the iwreq_data.
              - extra - Pointer to the data.        
  \return - 0 for success, non zero for failure
  
  --------------------------------------------------------------------------*/
int iw_get_ap_address(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra)
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

    ENTER();
    if ((pAdapter->conn_info.connState == eConnectionState_Associated) ||
        (eConnectionState_IbssConnected == pAdapter->conn_info.connState))
    {
        memcpy(wrqu->ap_addr.sa_data,pAdapter->conn_info.bssId,sizeof( pAdapter->conn_info.bssId ));
    }
    else
    {
        memset(wrqu->ap_addr.sa_data,0,sizeof(wrqu->ap_addr.sa_data));
    }
    EXIT();
    return 0;
}
