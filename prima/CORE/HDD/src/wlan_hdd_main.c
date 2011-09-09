/*========================================================================

  \file  wlan_hdd_main.c

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
  02/24/10    Sudhir.S.Kohalli  Added to support param for SoftAP module
  06/03/10    js - Added support to hostapd driven deauth/disassoc/mic failure
  ==========================================================================*/

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
//#include <wlan_qct_driver.h>
#include <wlan_hdd_includes.h>
#ifdef ANI_BUS_TYPE_SDIO
#include <wlan_sal_misc.h>
#endif // ANI_BUS_TYPE_SDIO
#include <vos_api.h>
#include <vos_sched.h>
#include <vos_power.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#ifdef ANI_BUS_TYPE_SDIO
#include <linux/mmc/sdio_func.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
// added in 2.6.32, need to define locally if using an earlier kernel
#define dev_to_sdio_func(d)      container_of(d, struct sdio_func, dev)
#endif
#endif // ANI_BUS_TYPE_SDIO
#ifdef ANI_BUS_TYPE_PLATFORM
#include <linux/wcnss_wlan.h>
#endif //ANI_BUS_TYPE_PLATFORM
#ifdef ANI_BUS_TYPE_PCI
#include "wcnss_wlan.h"
#endif /* ANI_BUS_TYPE_PCI */
#include <wlan_hdd_tx_rx.h>
#include <palTimer.h>
#include <wniApi.h>
#include <wlan_nlink_srv.h>
#include <wlan_btc_svc.h>
#include <wlan_hdd_cfg.h>
#include <wlan_ptt_sock_svc.h>
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_misc.h>

#ifdef WLAN_BTAMP_FEATURE
#include <bap_hdd_main.h>
#include <bapInternal.h>
#endif // WLAN_BTAMP_FEATURE

#ifdef CONFIG_CFG80211
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include "wlan_hdd_cfg80211.h"
#endif
#include <linux/rtnetlink.h>
#ifdef ANI_MANF_DIAG
int wlan_hdd_ftm_start(hdd_context_t *pAdapter);
#endif
#ifdef WLAN_SOFTAP_FEATURE
#include "sapApi.h"
#include <linux/semaphore.h>
#include <wlan_hdd_hostapd.h>
#include <wlan_hdd_softap_tx_rx.h>
#endif
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "cfgApi.h"
#endif
#include "wlan_hdd_dev_pwr.h"
#ifdef WLAN_BTAMP_FEATURE
#include "bap_hdd_misc.h"
#endif
#ifdef FEATURE_WLAN_INTEGRATED_SOC
#include "wlan_qct_pal_trace.h"
#endif /* FEATURE_WLAN_INTEGRATED_SOC */

//internal function declaration
v_U16_t hdd_select_queue(struct net_device *dev,
    struct sk_buff *skb);

void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter);

static int hdd_netdev_notifier_call(struct notifier_block * nb,
                                         unsigned long state,
                                         void *ndev)
{
   struct net_device *dev = ndev;
   hdd_adapter_t *pAdapter = NULL;

   //Make sure that this callback corresponds to our device.
   if( strncmp( dev->name, "wlan", 4 ) ) 
      return NOTIFY_DONE;

#ifdef CONFIG_CFG80211
   if (!dev->ieee80211_ptr)
       return NOTIFY_DONE;
#endif

   pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   if(NULL == pAdapter)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD Adaptor Null Pointer", __func__);
      VOS_ASSERT(0);
      return NOTIFY_DONE;
   }

   hddLog(VOS_TRACE_LEVEL_INFO,"%s: New Net Device State = %lu", __func__, state);

   switch (state) {
   case NETDEV_REGISTER:
        break;

   case NETDEV_UNREGISTER:
        break;

   case NETDEV_UP:
        break;

   case NETDEV_DOWN:
        break;

   case NETDEV_CHANGE:
        if(VOS_STA_MODE == hdd_get_conparam()) 
        {
            if(TRUE == pAdapter->isLinkUpSvcNeeded)
               complete(&pAdapter->linkup_event_var);
        }
        break;

   case NETDEV_GOING_DOWN:
        break;

   default:
        break;
   }

   return NOTIFY_DONE;
}

struct notifier_block hdd_netdev_notifier = {
   .notifier_call = hdd_netdev_notifier_call,
};

/*---------------------------------------------------------------------------
 *   Function definitions
 *-------------------------------------------------------------------------*/
extern int isWDresetInProgress(void);
#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
extern void register_wlan_suspend(void);
extern void unregister_wlan_suspend(void);
void hdd_unregister_mcast_bcast_filter(hdd_context_t *pHddCtx);
void hdd_register_mcast_bcast_filter(hdd_context_t *pHddCtx);
#endif
#endif
#ifdef WLAN_SOFTAP_FEATURE
//variable to hold the insmod parameters
static int con_mode = 0;
#endif
static tVOS_CONCURRENCY_MODE concurrency_mode = 0;


#ifdef FEATURE_WLAN_INTEGRATED_SOC
/**---------------------------------------------------------------------------

  \brief hdd_wdi_trace_enable() - Configure initial WDI Trace enable

  Called immediately after the cfg.ini is read in order to configure
  the desired trace levels in the WDI.

  \param  - moduleId - module whose trace level is being configured
  \param  - bitmask - bitmask of log levels to be enabled

  \return - void

  --------------------------------------------------------------------------*/
static void hdd_wdi_trace_enable(wpt_moduleid moduleId, v_U32_t bitmask)
{
   wpt_tracelevel level;

   /* if the bitmask is the default value, then a bitmask was not
      specified in cfg.ini, so leave the logging level alone (it
      will remain at the "compiled in" default value) */
   if (CFG_WDI_TRACE_ENABLE_DEFAULT == bitmask)
   {
      return;
   }

   /* a mask was specified.  start by disabling all logging */
   wpalTraceSetLevel(moduleId, eWLAN_PAL_TRACE_LEVEL_NONE, 0);

   /* now cycle through the bitmask until all "set" bits are serviced */
   level = eWLAN_PAL_TRACE_LEVEL_FATAL;
   while (0 != bitmask)
   {
      if (bitmask & 1)
      {
         wpalTraceSetLevel(moduleId, level, 1);
      }
      level++;
      bitmask >>= 1;
   }
}
#endif /* FEATURE_WLAN_INTEGRATED_SOC */

int hdd_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
   return 0;
}

/**---------------------------------------------------------------------------

  \brief hdd_open() - HDD Open function

  This is called in response to ifconfig up

  \param  - dev Pointer to net_device structure

  \return - 0 for success non-zero for failure

  --------------------------------------------------------------------------*/
int hdd_open (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   if(pAdapter == NULL) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __FUNCTION__);
      return -1;
   }

   /* Enable TX queues only when we are connected */
   if(hdd_connIsConnected(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: Enabling Tx Queues" , __FUNCTION__);
      netif_tx_start_all_queues(dev);
   }

   return 0;
}

int hdd_mon_open (struct net_device *dev)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);

   if(pAdapter == NULL) {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
         "%s: HDD adapter context is Null", __FUNCTION__);
      return -1;
   }

   netif_start_queue(dev);

   return 0;
}
/**---------------------------------------------------------------------------

  \brief hdd_stop() - HDD stop function

  This is called in response to ifconfig down

  \param  - dev Pointer to net_device structure

  \return - 0 for success non-zero for failure

  --------------------------------------------------------------------------*/

int hdd_stop (struct net_device *dev)
{

   //Stop the Interface TX queue. netif_stop_queue should not be used when
   //transmission is being disabled anywhere other than hard_start_xmit
   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Disabling OS Tx queues",__func__);
   netif_tx_disable(dev);

   return 0;
}

/**---------------------------------------------------------------------------

  \brief hdd_release_firmware() -

   This function calls the release firmware API to free the firmware buffer.

  \param  - pFileName Pointer to the File Name.
                  pCtx - Pointer to the adapter .


  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS hdd_release_firmware(char *pFileName,v_VOID_t *pCtx)
{
   VOS_STATUS status = VOS_STATUS_SUCCESS;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;
   ENTER();


   if (!strcmp(WLAN_FW_FILE, pFileName)) {

      hddLog(VOS_TRACE_LEVEL_INFO_HIGH,"%s: Loaded firmware file is %s",__func__,pFileName);

      if(pHddCtx->fw) {
         release_firmware(pHddCtx->fw);
         pHddCtx->fw = NULL;
      }
      else
         status = VOS_STATUS_E_FAILURE;
   }
   else if (!strcmp(WLAN_NV_FILE,pFileName)) {
      if(pHddCtx->nv) {
         release_firmware(pHddCtx->nv);
         pHddCtx->nv = NULL;
      }
      else
         status = VOS_STATUS_E_FAILURE;

   }

   EXIT();
   return status;
}

/**---------------------------------------------------------------------------

  \brief hdd_request_firmware() -

   This function reads the firmware file using the request firmware
   API and returns the the firmware data and the firmware file size.

  \param  - pfileName - Pointer to the file name.
              - pCtx - Pointer to the adapter .
              - ppfw_data - Pointer to the pointer of the firmware data.
              - pSize - Pointer to the file size.

  \return - VOS_STATUS_SUCCESS for success, VOS_STATUS_E_FAILURE for failure

  --------------------------------------------------------------------------*/


VOS_STATUS hdd_request_firmware(char *pfileName,v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize)
{
   int status;
   VOS_STATUS retval = VOS_STATUS_SUCCESS;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;
   ENTER();

   if( (!strcmp(WLAN_FW_FILE, pfileName)) ) {

       status = request_firmware(&pHddCtx->fw, pfileName, pHddCtx->parent_dev);

       if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
           hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Firmware %s download failed",
                  __func__, pfileName);
           retval = VOS_STATUS_E_FAILURE;
       }

       else {
         *ppfw_data = (v_VOID_t *)pHddCtx->fw->data;
         *pSize = pHddCtx->fw->size;
          hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Firmware size = %d",__func__, *pSize);
       }
   }
   else if(!strcmp(WLAN_NV_FILE, pfileName)) {

       status = request_firmware(&pHddCtx->nv, pfileName, pHddCtx->parent_dev);

       if(status || !pHddCtx->nv || !pHddCtx->nv->data) {
           hddLog(VOS_TRACE_LEVEL_FATAL, "%s: nv %s download failed",
                  __func__, pfileName);
           retval = VOS_STATUS_E_FAILURE;
       }

       else {
         *ppfw_data = (v_VOID_t *)pHddCtx->nv->data;
         *pSize = pHddCtx->nv->size;
          hddLog(VOS_TRACE_LEVEL_ERROR,"%s: nv file size = %d",__func__, *pSize);
       }
   }

   EXIT();
   return retval;
}

/**---------------------------------------------------------------------------

  \brief hdd_get_cfg_file_size() -

   This function reads the configuration file using the request firmware
   API and returns the configuration file size.

  \param  - pCtx - Pointer to the adapter .
              - pFileName - Pointer to the file name.
              - pBufSize - Pointer to the buffer size.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize)
{
   int status;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;

   ENTER();

   status = request_firmware(&pHddCtx->fw, pFileName, pHddCtx->parent_dev);

   if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: CFG download failed",__func__);
      status = VOS_STATUS_E_FAILURE;
   }
   else {
      *pBufSize = pHddCtx->fw->size;
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: CFG size = %d",__func__, *pBufSize);
      release_firmware(pHddCtx->fw);
      pHddCtx->fw = NULL;
   }

   EXIT();
   return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief hdd_read_cfg_file() -

   This function reads the configuration file using the request firmware
   API and returns the cfg data and the buffer size of the configuration file.

  \param  - pCtx - Pointer to the adapter .
              - pFileName - Pointer to the file name.
              - pBuffer - Pointer to the data buffer.
              - pBufSize - Pointer to the buffer size.

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName,
    v_VOID_t *pBuffer, v_SIZE_t *pBufSize)
{
   int status;
   hdd_context_t *pHddCtx = (hdd_context_t*)pCtx;

   ENTER();

   status = request_firmware(&pHddCtx->fw, pFileName, pHddCtx->parent_dev);

   if(status || !pHddCtx->fw || !pHddCtx->fw->data) {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: CFG download failed",__func__);
      return VOS_STATUS_E_FAILURE;
   }
   else {
      if(*pBufSize != pHddCtx->fw->size) {
         hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Caller sets invalid CFG "
               "file size", __func__);
         release_firmware(pHddCtx->fw);
         pHddCtx->fw = NULL;
         return VOS_STATUS_E_FAILURE;
      }
      else {
         if(pBuffer) {
            vos_mem_copy(pBuffer,pHddCtx->fw->data,*pBufSize);
         }
         release_firmware(pHddCtx->fw);
         pHddCtx->fw = NULL;
      }
   }

   EXIT();

   return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief hdd_set_mac_addr_cb() -

   This function is the call back function for setting the station
   mac adrress called by ccm module to indicate the
   success/failure result.

  \param  - hHal - Pointer to the hal module.
              - result - returns the result of the set mac address.

  \return - void

  --------------------------------------------------------------------------*/
#ifndef FEATURE_WLAN_INTEGRATED_SOC
static void hdd_set_mac_addr_cb( tHalHandle hHal, tANI_S32 result )
{
  // ignore the STA_ID response for now.

  VOS_ASSERT( CCM_IS_RESULT_SUCCESS( result ) );
}
#endif


/**---------------------------------------------------------------------------

  \brief hdd_set_mac_address() -

   This function sets the user specified mac address using
   the command ifconfig wlanX hw ether <mac adress>.

  \param  - dev - Pointer to the net device.
              - addr - Pointer to the sockaddr.
  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static int hdd_set_mac_address(struct net_device *dev, void *addr)
{
   hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
   struct sockaddr *psta_mac_addr = addr;
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;

   ENTER();

   memcpy(&pAdapter->macAddressCurrent, psta_mac_addr->sa_data, ETH_ALEN);

#ifdef HDD_SESSIONIZE 
   // set the MAC address though the STA ID CFG.
   halStatus = ccmCfgSetStr( pAdapter->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pAdapter->macAddressCurrent,
                             sizeof( pAdapter->macAddressCurrent ),
                             hdd_set_mac_addr_cb, VOS_FALSE );
#endif

   memcpy(dev->dev_addr, psta_mac_addr->sa_data, ETH_ALEN);

   EXIT();
   return halStatus;
}

tANI_U8* wlan_hdd_get_intf_addr(hdd_context_t* pHddCtx)
{
   int i;
   for ( i = 0; i < VOS_MAX_CONCURRENCY_PERSONA; i++)
   {
      if( 0 == (pHddCtx->cfg_ini->intfAddrMask >> i))
         break;
   }

   if( VOS_MAX_CONCURRENCY_PERSONA == i)
      return NULL;

   pHddCtx->cfg_ini->intfAddrMask |= (1 << i);
   return &pHddCtx->cfg_ini->intfMacAddr[i].bytes[0];
}

void wlan_hdd_release_intf_addr(hdd_context_t* pHddCtx, tANI_U8* releaseAddr)
{
   int i;
   for ( i = 0; i < VOS_MAX_CONCURRENCY_PERSONA; i++)
   {
      if ( !memcmp(releaseAddr, &pHddCtx->cfg_ini->intfMacAddr[i].bytes[0], 6) )
      {
         pHddCtx->cfg_ini->intfAddrMask &= ~(1 << i);
         break;
      } 
   }
   return;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
  static struct net_device_ops wlan_drv_ops = {
      .ndo_open = hdd_open,
      .ndo_stop = hdd_stop,
      .ndo_start_xmit = hdd_hard_start_xmit,
      .ndo_tx_timeout = hdd_tx_timeout,
      .ndo_get_stats = hdd_stats,
      .ndo_do_ioctl = hdd_ioctl,
      .ndo_set_mac_address = hdd_set_mac_address,
      .ndo_select_queue    = hdd_select_queue,

 };
#ifdef CONFIG_CFG80211   
 static struct net_device_ops wlan_mon_drv_ops = {
      .ndo_open = hdd_mon_open,
      .ndo_stop = hdd_stop,
      .ndo_start_xmit = hdd_mon_hard_start_xmit,  
      .ndo_tx_timeout = hdd_tx_timeout,
      .ndo_get_stats = hdd_stats,
      .ndo_do_ioctl = hdd_ioctl,
      .ndo_set_mac_address = hdd_set_mac_address,
 };
#endif

#endif

void hdd_set_station_ops( struct net_device *pWlanDev )
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
      pWlanDev->tx_queue_len = NET_DEV_TX_QUEUE_LEN,
      pWlanDev->netdev_ops = &wlan_drv_ops;
#else
      pWlanDev->open = hdd_open;
      pWlanDev->stop = hdd_stop;
      pWlanDev->hard_start_xmit = NULL;
      pWlanDev->tx_timeout = hdd_tx_timeout;
      pWlanDev->get_stats = hdd_stats;
      pWlanDev->do_ioctl = hdd_ioctl;
      pWlanDev->tx_queue_len = NET_DEV_TX_QUEUE_LEN;
      pWlanDev->set_mac_address = hdd_set_mac_address;
#endif
}

hdd_adapter_t* hdd_alloc_station_adapter( hdd_context_t *pHddCtx, tSirMacAddr macAddr, char* name )
{
   struct net_device *pWlanDev = NULL;
   hdd_adapter_t *pAdapter = NULL;
#ifdef CONFIG_CFG80211
   /*
    * cfg80211 initialization and registration....
    */ 
   pWlanDev = alloc_netdev_mq(sizeof( hdd_adapter_t ), name, ether_setup, NUM_TX_QUEUES);
   
#else      
   //Allocate the net_device and private data (station ctx) 
   pWlanDev = alloc_etherdev_mq(sizeof( hdd_adapter_t ), NUM_TX_QUEUES);

#endif

   if(pWlanDev != NULL)
   {

      //Save the pointer to the net_device in the HDD adapter
      pAdapter = (hdd_adapter_t*) netdev_priv( pWlanDev );

#ifndef CONFIG_CFG80211
      //Init the net_device structure
      ether_setup(pWlanDev);
#endif

      vos_mem_zero( pAdapter, sizeof( hdd_adapter_t ) );

      pAdapter->dev = pWlanDev;
      pAdapter->pHddCtx = pHddCtx; 

      init_completion(&pAdapter->session_open_comp_var);
      init_completion(&pAdapter->session_close_comp_var);
      init_completion(&pAdapter->disconnect_comp_var);
      init_completion(&pAdapter->linkup_event_var);
      init_completion(&pAdapter->cancel_rem_on_chan_var);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
      init_completion(&pAdapter->offchannel_tx_event);
#endif
      init_completion(&pHddCtx->mc_sus_event_var);
      init_completion(&pHddCtx->tx_sus_event_var);

      pAdapter->isLinkUpSvcNeeded = FALSE; 
      //Init the net_device structure
      strcpy(pWlanDev->name, name);

      vos_mem_copy(pWlanDev->dev_addr, (void *)macAddr, sizeof(tSirMacAddr));
      vos_mem_copy( pAdapter->macAddressCurrent.bytes, macAddr, sizeof(tSirMacAddr));
      pWlanDev->watchdog_timeo = HDD_TX_TIMEOUT;
      pWlanDev->hard_header_len += LIBRA_HW_NEEDED_HEADROOM;

      hdd_set_station_ops( pAdapter->dev );

      pWlanDev->destructor = free_netdev;
#ifdef CONFIG_CFG80211
      pWlanDev->ieee80211_ptr = &pAdapter->wdev ;
      pAdapter->wdev.wiphy = pHddCtx->wiphy;  
      pAdapter->wdev.netdev =  pWlanDev;
#endif  
      /* set pWlanDev's parent to underlying device */
      SET_NETDEV_DEV(pWlanDev, pHddCtx->parent_dev);
   }

   return pAdapter;
}

VOS_STATUS hdd_register_interface( hdd_adapter_t *pAdapter, tANI_U8 rtnl_lock_held )
{
   struct net_device *pWlanDev = pAdapter->dev;
   //hdd_station_ctx_t *pHddStaCtx = &pAdapter->sessionCtx.station;
   //hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
   //eHalStatus halStatus = eHAL_STATUS_SUCCESS;

   if( rtnl_lock_held )
   {
      if (strchr(pWlanDev->name, '%')) {
         if( dev_alloc_name(pWlanDev, pWlanDev->name) < 0 )
         {
            hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:dev_alloc_name",__func__);
            return VOS_STATUS_E_FAILURE;            
         }
      }
      if (register_netdevice(pWlanDev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s:Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;         
      }
   }
   else
   {
      if(register_netdev(pWlanDev))
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed:register_netdev",__func__);
         return VOS_STATUS_E_FAILURE;         
      }
   }
   set_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);

   return VOS_STATUS_SUCCESS;
}

static eHalStatus hdd_smeCloseSessionCallback(void *pContext)
{
   if(pContext != NULL)
   {
      clear_bit(SME_SESSION_OPENED, &((hdd_adapter_t*)pContext)->event_flags);
      complete(&((hdd_adapter_t*)pContext)->session_close_comp_var);

      /* need to make sure all of our scheduled work has completed.
       * This callback is called from MC thread context, so it is safe to 
       * to call below flush workqueue API from here. 
       */
      flush_scheduled_work();
   }
   return eHAL_STATUS_SUCCESS;
}

VOS_STATUS hdd_init_station_mode( hdd_adapter_t *pAdapter )
{
   struct net_device *pWlanDev = pAdapter->dev;
   hdd_station_ctx_t *pHddStaCtx = &pAdapter->sessionCtx.station;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
   eHalStatus halStatus = eHAL_STATUS_SUCCESS;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;
   int rc = 0;

   INIT_COMPLETION(pAdapter->session_open_comp_var);
   //Open a SME session for future operation
   halStatus = sme_OpenSession( pHddCtx->hHal, hdd_smeRoamCallback, pAdapter,
         (tANI_U8 *)&pAdapter->macAddressCurrent, &pAdapter->sessionId );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "sme_OpenSession() failed with status code %08d [x%08lx]",
            halStatus, halStatus );
      status = VOS_STATUS_E_FAILURE;
      goto error_sme_open;
   }
   
   //Block on a completion variable. Can't wait forever though.
   rc = wait_for_completion_interruptible_timeout(
                        &pAdapter->session_open_comp_var,
                        msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
   if (!rc)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "Session is not opened within timeout period code %08d", rc );
      status = VOS_STATUS_E_FAILURE;
      goto error_sme_open;
   }

   // Register wireless extensions
   if( eHAL_STATUS_SUCCESS !=  (halStatus = hdd_register_wext(pWlanDev)))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
              "hdd_register_wext() failed with status code %08d [x%08lx]",
                                                   halStatus, halStatus );
      status = VOS_STATUS_E_FAILURE;
      goto error_register_wext;
   }
   //Safe to register the hard_start_xmit function again
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29))
   wlan_drv_ops.ndo_start_xmit = hdd_hard_start_xmit;
#else
   pWlanDev->hard_start_xmit = hdd_hard_start_xmit;
#endif

   //Set the Connection State to Not Connected
   pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;

   //Set the default operation channel
   pHddStaCtx->conn_info.operationChannel = pHddCtx->cfg_ini->OperatingChannel;

   /* Make the default Auth Type as OPEN*/
   pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

   if( VOS_STATUS_SUCCESS != ( status = hdd_init_tx_rx( pAdapter ) ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
            "hdd_init_tx_rx() failed with status code %08d [x%08lx]",
                            status, status );
      goto error_init_txrx;
   }

   set_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);

   if( VOS_STATUS_SUCCESS != ( status = hdd_wmm_adapter_init( pAdapter ) ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
            "hdd_wmm_adapter_init() failed with status code %08d [x%08lx]",
                            status, status );
      goto error_wmm_init;
   }

   set_bit(WMM_INIT_DONE, &pAdapter->event_flags);

   return VOS_STATUS_SUCCESS;

error_wmm_init:
   clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
   hdd_deinit_tx_rx(pAdapter);
error_init_txrx:
   hdd_UnregisterWext(pWlanDev);
error_register_wext:
   if(test_bit(SME_SESSION_OPENED, &pAdapter->event_flags))
   {
      INIT_COMPLETION(pAdapter->session_close_comp_var);
      if( eHAL_STATUS_SUCCESS == sme_CloseSession( pHddCtx->hHal,
                                    pAdapter->sessionId,
                                    hdd_smeCloseSessionCallback, pAdapter ) )
      {
         //Block on a completion variable. Can't wait forever though.
         wait_for_completion_interruptible_timeout(
                          &pAdapter->session_close_comp_var,
                           msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
      }
}
error_sme_open:
   return status;
}

void hdd_deinit_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   switch ( pAdapter->device_mode )
   {
      case WLAN_HDD_INFRA_STATION:
      case WLAN_HDD_P2P_CLIENT:
         {
            if(test_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags))
            {
               hdd_deinit_tx_rx( pAdapter );
               clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
            }

            if(test_bit(WMM_INIT_DONE, &pAdapter->event_flags))
            {
               hdd_wmm_adapter_close( pAdapter );
               clear_bit(WMM_INIT_DONE, &pAdapter->event_flags);
            }

            if(test_bit(SME_SESSION_OPENED, &pAdapter->event_flags)) 
            {
               INIT_COMPLETION(pAdapter->session_close_comp_var);
               if( eHAL_STATUS_SUCCESS ==
                       sme_CloseSession( pHddCtx->hHal, pAdapter->sessionId, 
                                         hdd_smeCloseSessionCallback, pAdapter ) )
               {
                  //Block on a completion variable. Can't wait forever though.
                  wait_for_completion_interruptible_timeout(&pAdapter->session_close_comp_var, 
                        msecs_to_jiffies(WLAN_WAIT_TIME_SESSIONOPENCLOSE));
               }
            }
            break;
         }

      case WLAN_HDD_SOFTAP:
      case WLAN_HDD_P2P_GO:
#ifdef WLAN_SOFTAP_FEATURE
         {
            VOS_STATUS status = VOS_STATUS_E_FAILURE;
            //Any softap specific cleanup here...
            if(test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) 
            {
               //Stop Bss.
               if(VOS_STATUS_SUCCESS == (status = 
                        WLANSAP_StopBss((WLAN_HDD_GET_CTX(pAdapter))->pvosContext)))
               {
                  hdd_hostapd_state_t *pHostapdState = 
                     WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter);

                  status = vos_wait_single_event(&pHostapdState->vosEvent, 10000);

                  if (!VOS_IS_STATUS_SUCCESS(status))
                  {  
                     VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR, 
                           ("ERROR: HDD vos wait for single_event failed!!\n"));
                     VOS_ASSERT(0);
                  }
               }

               if(status != VOS_STATUS_SUCCESS) 
               {
                  hddLog(VOS_TRACE_LEVEL_FATAL,
                        "%s:Error!!! Stoping the BSS\n",__func__);
               }
               clear_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags);
            }
            if (ccmCfgSetInt((WLAN_HDD_GET_CTX(pAdapter))->hHal,
                     WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG, 0,NULL, eANI_BOOLEAN_FALSE)
                  ==eHAL_STATUS_FAILURE)
            {
               hddLog(LOGE,
                     "Could not pass on WNI_CFG_PROBE_RSP_BCN_ADDNIE_FLAG to CCM\n");
            } 
            hdd_unregister_hostapd(pAdapter);
            hdd_set_conparam( 0 );
#ifdef CONFIG_CFG80211
            wlan_hdd_set_monitor_tx_adapter( WLAN_HDD_GET_CTX(pAdapter), NULL );
#endif
            break;
         }

      case WLAN_HDD_MONITOR:
         {
            if(test_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags))
            {
               hdd_deinit_tx_rx( pAdapter );
               clear_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
            }
#endif
            break;
         }


      default:
         break;
   }
}

void hdd_cleanup_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter, tANI_U8 rtnl_held )
{
   struct net_device *pWlanDev = pAdapter->dev;

   if(test_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags)) {
      if( rtnl_held )
      {
         unregister_netdevice(pWlanDev);
      }
      else
      {
         unregister_netdev(pWlanDev);
      }
      clear_bit(NET_DEVICE_REGISTERED, &pAdapter->event_flags);
   }

   hdd_deinit_adapter( pHddCtx, pAdapter );
}

hdd_adapter_t* hdd_open_adapter( hdd_context_t *pHddCtx, tANI_U8 session_type,
                                 char *iface_name, tSirMacAddr macAddr, 
                                 tANI_U8 rtnl_held )
{
   hdd_adapter_t *pAdapter = NULL;
   hdd_adapter_list_node_t *pHddAdapterNode = NULL;
   VOS_STATUS status = VOS_STATUS_E_FAILURE;

   hddLog(VOS_TRACE_LEVEL_INFO_HIGH, "%s iface =%s type = %d\n",__func__,iface_name,session_type);

   switch(session_type)
   {
       case WLAN_HDD_INFRA_STATION:
#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_CLIENT:
#endif
       {
           pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );

           if( NULL == pAdapter )
              return NULL;

#ifdef CONFIG_CFG80211
         pAdapter->wdev.iftype = (session_type == WLAN_HDD_INFRA_STATION) ?
                                  NL80211_IFTYPE_STATION :
                                  NL80211_IFTYPE_P2P_CLIENT;
#endif


           pAdapter->device_mode = session_type;

           status = hdd_init_station_mode( pAdapter );
           if( VOS_STATUS_SUCCESS != status )
              goto err_free_netdev;

           status = hdd_register_interface( pAdapter, rtnl_held );
           if( VOS_STATUS_SUCCESS != status )
           {
              hdd_deinit_adapter(pHddCtx, pAdapter);
              goto err_free_netdev;
           }
           //Stop the Interface TX queue.
           netif_tx_disable(pAdapter->dev);
           //netif_tx_disable(pWlanDev);
           netif_carrier_off(pAdapter->dev);

         break;
      }

#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_GO:
#endif
      case WLAN_HDD_SOFTAP:
      {
          pAdapter = hdd_wlan_create_ap_dev( pHddCtx, macAddr, (tANI_U8 *)iface_name );
          if( NULL == pAdapter )
              return NULL;

#ifdef CONFIG_CFG80211
         pAdapter->wdev.iftype = (session_type == WLAN_HDD_SOFTAP) ?
                                  NL80211_IFTYPE_AP:
                                  NL80211_IFTYPE_P2P_GO;
#endif
          pAdapter->device_mode = session_type;

          status = hdd_init_ap_mode(pAdapter);
          if( VOS_STATUS_SUCCESS != status )
             goto err_free_netdev;

          status = hdd_register_hostapd( pAdapter, rtnl_held );
          if( VOS_STATUS_SUCCESS != status )
          {
             hdd_deinit_adapter(pHddCtx, pAdapter);
             goto err_free_netdev;
          }

          netif_tx_disable(pAdapter->dev);
          netif_carrier_off(pAdapter->dev);

          hdd_set_conparam( 1 );
          break;
      }
      case WLAN_HDD_MONITOR:
      {
#ifdef CONFIG_CFG80211   
         pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );
         if( NULL == pAdapter )
           return NULL;

         pAdapter->wdev.iftype = NL80211_IFTYPE_MONITOR; 
         pAdapter->device_mode = session_type;
         status = hdd_register_interface( pAdapter, rtnl_held );
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
         pAdapter->dev->netdev_ops = &wlan_mon_drv_ops;
#else
         pAdapter->dev->open = hdd_mon_open,
         pAdapter->dev->hard_start_xmit = hdd_mon_hard_start_xmit;
#endif
         hdd_init_tx_rx( pAdapter );
         set_bit(INIT_TX_RX_SUCCESS, &pAdapter->event_flags);
         //Set adapter to be used for data tx. It will use either GO or softap.
         pAdapter->sessionCtx.monitor.pAdapterForTx = 
                           hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_SOFTAP);
#ifdef WLAN_FEATURE_P2P
         if (NULL == pAdapter->sessionCtx.monitor.pAdapterForTx)
         {
            pAdapter->sessionCtx.monitor.pAdapterForTx = 
                           hdd_get_adapter(pAdapter->pHddCtx, WLAN_HDD_P2P_GO);
         }
#endif
         
#endif
       }
       break;
#ifdef ANI_MANF_DIAG
       case WLAN_HDD_FTM:
       {
           pAdapter = hdd_alloc_station_adapter( pHddCtx, macAddr, iface_name );

           if( NULL == pAdapter )
              return NULL;
     
           pAdapter->device_mode = session_type;
           status = hdd_register_interface( pAdapter, rtnl_held );
       }
       break;
#endif
       default:
       {
           VOS_ASSERT(0);
           return NULL;
       }
   }

   switch(session_type)
   {
       case WLAN_HDD_INFRA_STATION:
              concurrency_mode |= VOS_STA;
              break;
#ifdef WLAN_FEATURE_P2P
       case WLAN_HDD_P2P_CLIENT:       
              concurrency_mode |= VOS_P2P_CLIENT;
              break;
       case WLAN_HDD_P2P_GO:
              concurrency_mode |= VOS_P2P_GO;
              break;
#endif
       case WLAN_HDD_SOFTAP:
              concurrency_mode |= VOS_SAP;
              break;
   }

   /* If there are concurrent session enable SW frame translation 
    * for all registered STA */ 
   if (vos_concurrent_sessions_running())
   {
      WLANTL_ConfigureSwFrameTXXlationForAll(pHddCtx->pvosContext, TRUE);
   }
   
   if( VOS_STATUS_SUCCESS == status )
   {
      //Add it to the hdd's session list. 
      pHddAdapterNode = vos_mem_malloc( sizeof( hdd_adapter_list_node_t ) );
      if( NULL == pHddAdapterNode )
      {
         status = VOS_STATUS_E_NOMEM;
      }
      else
      {
         pHddAdapterNode->pAdapter = pAdapter;
         status = hdd_add_adapter_back ( pHddCtx, 
                                         pHddAdapterNode );
      }
   }

   if( VOS_STATUS_SUCCESS != status )
   {
      if( NULL != pAdapter )
      {
         hdd_cleanup_adapter( pHddCtx, pAdapter, rtnl_held );
         pAdapter = NULL;   
      }
      if( NULL != pHddAdapterNode )
      {
         vos_mem_free( pHddAdapterNode );
      }
   }

   return pAdapter;

err_free_netdev:
   free_netdev(pAdapter->dev);
   return NULL;
}

VOS_STATUS hdd_close_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter,
                              tANI_U8 rtnl_held )
{
   hdd_adapter_list_node_t *pAdapterNode, *pCurrent, *pNext;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pCurrent );
   if( VOS_STATUS_SUCCESS != status )
      return status;

   while ( pCurrent->pAdapter != pAdapter )
   {
      status = hdd_get_next_adapter ( pHddCtx, pCurrent, &pNext );
      if( VOS_STATUS_SUCCESS != status )
         break;

      pCurrent = pNext;
   }
   pAdapterNode = pCurrent;
   if( VOS_STATUS_SUCCESS == status )
   {
      switch ( pAdapter->device_mode )
      {
            case WLAN_HDD_INFRA_STATION:
                   concurrency_mode &= ~VOS_STA;
                   break;
            case WLAN_HDD_P2P_CLIENT:
                   concurrency_mode &= ~VOS_P2P_CLIENT;
                   break;
            case WLAN_HDD_P2P_GO:
                   concurrency_mode &= ~VOS_P2P_GO;
                   break;
            case WLAN_HDD_SOFTAP:
                   concurrency_mode &= ~VOS_SAP;
                   break;
            default:
                   break;
      }
                             
      hdd_cleanup_adapter( pHddCtx, pAdapterNode->pAdapter, rtnl_held );
      hdd_remove_adapter( pHddCtx, pAdapterNode );
      vos_mem_free( pAdapterNode );

      /* If there is no concurrent session disable SW frame translation 
       * for all registered STA */ 
      if (!vos_concurrent_sessions_running())
      {
         WLANTL_ConfigureSwFrameTXXlationForAll(pHddCtx->pvosContext, FALSE);
      }
      return VOS_STATUS_SUCCESS;
   }

   return VOS_STATUS_E_FAILURE;
}

VOS_STATUS hdd_close_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pHddAdapterNode;
   VOS_STATUS status;

   do
   {
      status = hdd_remove_front_adapter( pHddCtx, &pHddAdapterNode );
      if( pHddAdapterNode && VOS_STATUS_SUCCESS == status )
      {
         hdd_cleanup_adapter( pHddCtx, pHddAdapterNode->pAdapter, FALSE );
         vos_mem_free( pHddAdapterNode );
      }
   }while( NULL != pHddAdapterNode && VOS_STATUS_E_EMPTY != status );
   
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_stop_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
  eHalStatus halStatus = eHAL_STATUS_SUCCESS;
  union iwreq_data wrqu;

  if( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
      (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
  {
    switch(pAdapter->device_mode)
    {
      case WLAN_HDD_INFRA_STATION:
      case WLAN_HDD_P2P_CLIENT:
         if( hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) )
         {
            halStatus = sme_RoamDisconnect(pHddCtx->hHal, pAdapter->sessionId, eCSR_DISCONNECT_REASON_UNSPECIFIED);
            //success implies disconnect command got queued up successfully
            if(halStatus == eHAL_STATUS_SUCCESS)
            {
               wait_for_completion_interruptible_timeout(&pAdapter->disconnect_comp_var,
               msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));
            }
            memset(&wrqu, '\0', sizeof(wrqu));
            wrqu.ap_addr.sa_family = ARPHRD_ETHER;
            memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
            wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
         }
         break;

      case WLAN_HDD_SOFTAP:
      case WLAN_HDD_P2P_GO:
         break;
      case WLAN_HDD_MONITOR:
         break;
      default:
         break;
    }
  }
  return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_stop_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;
      netif_tx_disable(pAdapter->dev);
      netif_carrier_off(pAdapter->dev);

      hdd_stop_adapter( pHddCtx, pAdapter );

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_reset_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t *pAdapter;

   status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;
      netif_tx_disable(pAdapter->dev);
      netif_carrier_off(pAdapter->dev);

      //Record whether STA is associated
      pAdapter->sessionCtx.station.bSendDisconnect = 
            hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) ?
                                                       VOS_TRUE : VOS_FALSE;

      hdd_deinit_tx_rx(pAdapter);
      hdd_wmm_adapter_close(pAdapter);

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_start_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      switch(pAdapter->device_mode)
      {
         case WLAN_HDD_INFRA_STATION:
         case WLAN_HDD_P2P_CLIENT:
            hdd_init_station_mode(pAdapter);
            /* Open the gates for HDD to receive Wext commands */
            pAdapter->isLinkUpSvcNeeded = FALSE; 
            pAdapter->sessionCtx.station.WextState.mScanPending = FALSE;

            //Trigger the initial scan
            hdd_wlan_initial_scan(pAdapter);

            //Indicate disconnect event to supplicant if associated previously
            if(pAdapter->sessionCtx.station.bSendDisconnect)
            {
               union iwreq_data wrqu;
               memset(&wrqu, '\0', sizeof(wrqu));
               wrqu.ap_addr.sa_family = ARPHRD_ETHER;
               memset(wrqu.ap_addr.sa_data,'\0',ETH_ALEN);
               wireless_send_event(pAdapter->dev, SIOCGIWAP, &wrqu, NULL);
               pAdapter->sessionCtx.station.bSendDisconnect = VOS_FALSE;

#ifdef CONFIG_CFG80211
               /* indicate disconnected event to nl80211 */
               cfg80211_disconnected(pAdapter->dev, WLAN_REASON_UNSPECIFIED,
                                     NULL, 0, GFP_KERNEL); 
#endif
            }
            break;

         case WLAN_HDD_SOFTAP:
         case WLAN_HDD_P2P_GO:
            /* add softap interface start code here */
            break;
         case WLAN_HDD_MONITOR:
            /* monitor interface start */
            break;
         default:
            break;
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }
   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_reconnect_all_adapters( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;
   v_U32_t roamId;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
             (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )
      {
         hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
         hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

         pHddStaCtx->conn_info.connState = eConnectionState_NotConnected;
         init_completion(&pAdapter->disconnect_comp_var);
         sme_RoamDisconnect(pHddCtx->hHal, pAdapter->sessionId,
                             eCSR_DISCONNECT_REASON_UNSPECIFIED);

         wait_for_completion_interruptible_timeout(
                                &pAdapter->disconnect_comp_var,
                                msecs_to_jiffies(WLAN_WAIT_TIME_DISCONNECT));

         pWextState->roamProfile.csrPersona = pAdapter->device_mode; 

         sme_RoamConnect(pHddCtx->hHal,
                         pAdapter->sessionId, &(pWextState->roamProfile),
                         &roamId); 
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return VOS_STATUS_SUCCESS;
}

VOS_STATUS hdd_get_front_adapter( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t** ppAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_peek_front ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t**) ppAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_get_next_adapter( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode,
                                 hdd_adapter_list_node_t** pNextAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status = hdd_list_peek_next ( &pHddCtx->hddAdapters,
                                  (hdd_list_node_t*) pAdapterNode,
                                  (hdd_list_node_t**)pNextAdapterNode );

    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_remove_adapter( hdd_context_t *pHddCtx,
                               hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_remove_node ( &pHddCtx->hddAdapters,
                                     &pAdapterNode->node );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_remove_front_adapter( hdd_context_t *pHddCtx,
                                     hdd_adapter_list_node_t** ppAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_remove_front( &pHddCtx->hddAdapters,
                   (hdd_list_node_t**) ppAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_add_adapter_back( hdd_context_t *pHddCtx,
                                 hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_insert_back ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t*) pAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

VOS_STATUS hdd_add_adapter_front( hdd_context_t *pHddCtx,
                                  hdd_adapter_list_node_t* pAdapterNode)
{
    VOS_STATUS status;
    spin_lock(&pHddCtx->hddAdapters.lock);
    status =  hdd_list_insert_front ( &pHddCtx->hddAdapters,
                   (hdd_list_node_t*) pAdapterNode );
    spin_unlock(&pHddCtx->hddAdapters.lock);
    return status;
}

hdd_adapter_t * hdd_get_adapter_by_macaddr( hdd_context_t *pHddCtx,
                                            tSirMacAddr macAddr )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && vos_mem_compare( pAdapter->macAddressCurrent.bytes,
                                       macAddr, sizeof(tSirMacAddr) ) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

hdd_adapter_t * hdd_get_adapter_by_name( hdd_context_t *pHddCtx, tANI_U8 *name )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && !strncmp( pAdapter->dev->name, (const char *)name,
          IFNAMSIZ ) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

hdd_adapter_t * hdd_get_adapter( hdd_context_t *pHddCtx, device_mode_t mode )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && (mode == pAdapter->device_mode) )
      {
         return pAdapter;
      }
      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

//Remove this function later
hdd_adapter_t * hdd_get_mon_adapter( hdd_context_t *pHddCtx )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   hdd_adapter_t *pAdapter;
   VOS_STATUS status;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( pAdapter && WLAN_HDD_MONITOR == pAdapter->device_mode )
      {
         return pAdapter;
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }

   return NULL;

} 

#ifdef CONFIG_CFG80211
/**---------------------------------------------------------------------------
  
  \brief hdd_set_monitor_tx_adapter() - 

   This API initializes the adapter to be used while transmitting on monitor
   adapter. 
   
  \param  - pHddCtx - Pointer to the HDD context.
            pAdapter - Adapter that will used for TX. This can be NULL.
  \return - None. 
  --------------------------------------------------------------------------*/
void wlan_hdd_set_monitor_tx_adapter( hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter )
{
   hdd_adapter_t *pMonAdapter;

   pMonAdapter = hdd_get_adapter( pHddCtx, WLAN_HDD_MONITOR );

   if( NULL != pMonAdapter )
   {
      pMonAdapter->sessionCtx.monitor.pAdapterForTx = pAdapter;
   }
}
#endif
/**---------------------------------------------------------------------------
  
  \brief hdd_select_queue() - 

   This API returns the operating channel of the requested device mode 
   
  \param  - pHddCtx - Pointer to the HDD context.
              - mode - Device mode for which operating channel is required
                suported modes - WLAN_HDD_INFRA_STATION, WLAN_HDD_P2P_CLIENT
                                 WLAN_HDD_SOFTAP, WLAN_HDD_P2P_GO.
  \return - channel number. "0" id the requested device is not found OR it is not connected. 
  --------------------------------------------------------------------------*/
v_U8_t hdd_get_operating_channel( hdd_context_t *pHddCtx, device_mode_t mode )
{
   hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
   VOS_STATUS status;
   hdd_adapter_t      *pAdapter;
   v_U8_t operatingChannel = 0;

   status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );

   while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
   {
      pAdapter = pAdapterNode->pAdapter;

      if( mode == pAdapter->device_mode )
      {
        switch(pAdapter->device_mode)
        {
          case WLAN_HDD_INFRA_STATION:
          case WLAN_HDD_P2P_CLIENT: 
            if( hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR( pAdapter )) )
              operatingChannel = (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.operationChannel;
            break;
          case WLAN_HDD_SOFTAP:
          case WLAN_HDD_P2P_GO:
            /*softap connection info */
            if(test_bit(SOFTAP_BSS_STARTED, &pAdapter->event_flags)) 
              operatingChannel = (WLAN_HDD_GET_AP_CTX_PTR(pAdapter))->operatingChannel;
            break;
          default:
            break;
        }

        break; //Found the device of interest. break the loop
      }

      status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
      pAdapterNode = pNext;
   }
   return operatingChannel;
}

/**---------------------------------------------------------------------------
  
  \brief hdd_select_queue() - 

   This function is registered with the Linux OS for network
   core to decide which queue to use first.
   
  \param  - dev - Pointer to the WLAN device.
              - skb - Pointer to OS packet (sk_buff).
  \return - ac, Queue Index/access category corresponding to UP in IP header 
  
  --------------------------------------------------------------------------*/
v_U16_t hdd_select_queue(struct net_device *dev,
    struct sk_buff *skb)
{
   return hdd_wmm_select_queue(dev, skb);
}


/**---------------------------------------------------------------------------

  \brief hdd_wlan_initial_scan() -

   This function triggers the initial scan

  \param  - pAdapter - Pointer to the HDD adapter.

  --------------------------------------------------------------------------*/
void hdd_wlan_initial_scan(hdd_adapter_t *pAdapter)
{
   tCsrScanRequest scanReq;
   tCsrChannelInfo channelInfo;
   eHalStatus halStatus;
   unsigned long scanId;
   hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

   vos_mem_zero(&scanReq, sizeof(tCsrScanRequest));
   vos_mem_set(&scanReq.bssid, sizeof(tCsrBssid), 0xff);
   scanReq.BSSType = eCSR_BSS_TYPE_ANY;

   if(sme_Is11dSupported(pHddCtx->hHal))
   {
      halStatus = sme_ScanGetBaseChannels( pHddCtx->hHal, &channelInfo );
      if ( HAL_STATUS_SUCCESS( halStatus ) )
      {
         scanReq.ChannelInfo.ChannelList = vos_mem_malloc(channelInfo.numOfChannels);
         if( !scanReq.ChannelInfo.ChannelList )
         {
            hddLog(VOS_TRACE_LEVEL_ERROR, "%s kmalloc failed", __func__);
            vos_mem_free(channelInfo.ChannelList);
            return;
         }
         vos_mem_copy(scanReq.ChannelInfo.ChannelList, channelInfo.ChannelList,
            channelInfo.numOfChannels);
         scanReq.ChannelInfo.numOfChannels = channelInfo.numOfChannels;
         vos_mem_free(channelInfo.ChannelList);
      }

      scanReq.scanType = eSIR_PASSIVE_SCAN;
      scanReq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
      scanReq.maxChnTime = pHddCtx->cfg_ini->nPassiveMaxChnTime;
      scanReq.minChnTime = pHddCtx->cfg_ini->nPassiveMinChnTime;
   }
   else
   {
      scanReq.scanType = eSIR_ACTIVE_SCAN;
      scanReq.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
      scanReq.maxChnTime = pHddCtx->cfg_ini->nActiveMaxChnTime;
      scanReq.minChnTime = pHddCtx->cfg_ini->nActiveMinChnTime;
   }

   halStatus = sme_ScanRequest(pHddCtx->hHal, pAdapter->sessionId, &scanReq, &scanId, NULL, NULL);
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR, "%s: sme_ScanRequest failed status code %d",
         __func__, halStatus );
   }

   if(sme_Is11dSupported(pHddCtx->hHal))
        vos_mem_free(scanReq.ChannelInfo.ChannelList);
}

/**---------------------------------------------------------------------------

  \brief hdd_full_pwr_cbk() - HDD full power callbackfunction

  This is the function invoked by SME to inform the result of a full power
  request issued by HDD

  \param  - callbackcontext - Pointer to cookie
  \param  - status - result of request

  \return - None

  --------------------------------------------------------------------------*/
void hdd_full_pwr_cbk(void *callbackContext, eHalStatus status)
{
   hdd_context_t *pHddCtx = (hdd_context_t*)callbackContext;

   hddLog(VOS_TRACE_LEVEL_ERROR,"HDD full Power callback status = %d", status);

   complete(&pHddCtx->full_pwr_comp_var);
}

/**---------------------------------------------------------------------------

  \brief hdd_wlan_exit() - HDD WLAN exit function

  This is the driver exit point (invoked during rmmod)

  \param  - pHddCtx - Pointer to the HDD Context

  \return - None

  --------------------------------------------------------------------------*/
void hdd_wlan_exit(hdd_context_t *pHddCtx)
{
   eHalStatus halStatus;
   v_CONTEXT_t pVosContext = pHddCtx->pvosContext;
   VOS_STATUS vosStatus;
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = NULL;
#endif // ANI_BUS_TYPE_SDIO
#ifdef CONFIG_CFG80211
   struct wiphy *wiphy = pHddCtx->wiphy;
#endif 

   hddLog(VOS_TRACE_LEVEL_ERROR,"In WLAN EXIT");

#ifdef CONFIG_CFG80211
#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE != hdd_get_conparam())
#endif
   {
      hdd_adapter_t* pAdapter = hdd_get_adapter(pHddCtx,
                                      WLAN_HDD_INFRA_STATION);

      if (pAdapter == NULL)
         pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_P2P_CLIENT);

      if (pAdapter != NULL)
         wlan_hdd_cfg80211_pre_voss_stop(pAdapter);
   }
#endif

#ifdef ANI_MANF_DIAG
   if (VOS_FTM_MODE == hdd_get_conparam())
   {
      wlan_hdd_ftm_close(pHddCtx);
      goto free_hdd_ctx;
   }
#endif  
   //Stop the Interface TX queue.
   //netif_tx_disable(pWlanDev);
   //netif_carrier_off(pWlanDev);

#ifdef CONFIG_HAS_EARLYSUSPEND
   // unregister suspend/resume callbacks
   if(pHddCtx->cfg_ini->nEnableSuspend)
   {
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      unregister_wlan_suspend();
#endif
   }
#endif

   //Disable IMPS/BMPS as we do not want the device to enter any power
   //save mode during shutdown
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_IDLE_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_BEACON_MODE_POWER_SAVE);
   sme_DisablePowerSave(pHddCtx->hHal, ePMC_UAPSD_MODE_POWER_SAVE);

   //Ensure that device is in full power as we will touch H/W during vos_Stop
   INIT_COMPLETION(pHddCtx->full_pwr_comp_var);
   halStatus = sme_RequestFullPower(pHddCtx->hHal, hdd_full_pwr_cbk,
         pHddCtx, eSME_FULL_PWR_NEEDED_BY_HDD);

   if(halStatus != eHAL_STATUS_SUCCESS)
   {
      if(halStatus == eHAL_STATUS_PMC_PENDING)
      {
         //Block on a completion variable. Can't wait forever though
         wait_for_completion_interruptible_timeout(&pHddCtx->full_pwr_comp_var,
               msecs_to_jiffies(1000));
      }
      else
      {
         hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Request for Full Power failed\n", __func__);
         VOS_ASSERT(0);
         return;
      }
   }

   // Unregister the Net Device Notifier
   unregister_netdevice_notifier(&hdd_netdev_notifier);

   hdd_stop_all_adapters( pHddCtx );
   hdd_close_all_adapters( pHddCtx );

   
#ifdef ANI_BUS_TYPE_SDIO
   sdio_func_dev = libra_getsdio_funcdev();

   if(sdio_func_dev == NULL)
   {
        hddLog(VOS_TRACE_LEVEL_FATAL, "%s: sdio_func_dev is NULL!",__func__);
        VOS_ASSERT(0);
        return;
   }

   sd_claim_host(sdio_func_dev);

   /* Disable SDIO IRQ since we are exiting */
   libra_enable_sdio_irq(sdio_func_dev, 0);

   sd_release_host(sdio_func_dev);
#endif // ANI_BUS_TYPE_SDIO

   //Stop all the modules
   vosStatus = vos_stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to stop VOSS",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

#ifdef ANI_BUS_TYPE_SDIO
   vosStatus = WLANBAL_Stop( pVosContext );

   hddLog(VOS_TRACE_LEVEL_ERROR,"WLAN BAL STOP\n");
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to stop BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   msleep(50);
   //Put the chip is standby before asserting deep sleep
   vosStatus = WLANBAL_SuspendChip( pVosContext );

   hddLog(VOS_TRACE_LEVEL_ERROR,"WLAN Suspend Chip\n");

   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to suspend chip ",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
   //Invoke SAL stop
   vosStatus = WLANSAL_Stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to stop SAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

#endif // ANI_BUS_TYPE_SDIO

   //Assert Deep sleep signal now to put Libra HW in lowest power state
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Vote off any PMIC voltage supplies
   vos_chipPowerDown(NULL, NULL, NULL);

   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   //Clean up HDD Nlink Service
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);
   nl_srv_exit();

   //This requires pMac access, Call this before vos_close().
#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
   hdd_unregister_mcast_bcast_filter(pHddCtx);
#endif
#endif

   //Close VOSS
   //This frees pMac(HAL) context. There should not be any call that requires pMac access after this.
   vos_close(pVosContext);
   //Close the scheduler before closing other modules.
   vosStatus = vos_sched_close( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))    {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to close VOSS Scheduler",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

#ifdef ANI_BUS_TYPE_SDIO
   vosStatus = WLANBAL_Close(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, 
            "%s: Failed to close BAL",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }
   hddLog(VOS_TRACE_LEVEL_ERROR,"Returned WLAN BAL CLOSE\n\n\n\n");
#endif // ANI_BUS_TYPE_SDIO

   //Close Watchdog
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

   /* Cancel the vote for XO Core ON. 
    * This is done here to ensure there is no race condition since MC, TX and WD threads have
    * exited at this point
    */
   hddLog(VOS_TRACE_LEVEL_WARN, "In module exit: Cancel the vote for XO Core ON"
                                    "when  WLAN is turned OFF\n");
   if (vos_chipVoteXOCore(NULL, NULL, NULL, VOS_FALSE) != VOS_STATUS_SUCCESS)
   {
       hddLog(VOS_TRACE_LEVEL_ERROR, "Could not cancel the vote for XO Core ON." 
                                        "Not returning failure."
                                        "Power consumed will be high\n");
   }  

   //Free up dynamically allocated members inside HDD Adapter
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

#ifdef ANI_MANF_DIAG
free_hdd_ctx:   
#endif
#ifdef CONFIG_CFG80211
   wiphy_unregister(wiphy) ; 
   wiphy_free(wiphy) ;
#else
   vos_mem_free( pHddCtx );
#endif
}

#ifdef FEATURE_WLAN_INTEGRATED_SOC
/**---------------------------------------------------------------------------

  \brief hdd_wlan_stop() - HDD WLAN stop function

  This is the driver stop point

  \param  - pHddCtx - Pointer to the HDD Context

  \return - None

  --------------------------------------------------------------------------*/
void hdd_wlan_stop(hdd_context_t *pHddCtx)
{
   v_CONTEXT_t pVosContext = pHddCtx->pvosContext;
   VOS_STATUS vosStatus;

   hddLog(VOS_TRACE_LEVEL_ERROR,"In WLAN STOP");

   // Unregister the Net Device Notifier
   unregister_netdevice_notifier(&hdd_netdev_notifier);

   hdd_stop_all_adapters( pHddCtx );
   hdd_close_all_adapters( pHddCtx );

   hddLog(VOS_TRACE_LEVEL_ERROR,"WLAN STOP ALL\n");

   //Stop all the modules
   vosStatus = vos_stop( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus))
   {
      VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to stop VOSS",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   //Assert Deep sleep signal now to put Libra HW in lowest power state
   vosStatus = vos_chipAssertDeepSleep( NULL, NULL, NULL );
   VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );

   //Vote off any PMIC voltage supplies
   vos_chipPowerDown(NULL, NULL, NULL);

   vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

   //Clean up HDD Nlink Service
   send_btc_nlink_msg(WLAN_MODULE_DOWN_IND, 0);
   nl_srv_exit();

   //Close the scheduler before closing other modules.
   vosStatus = vos_sched_close( pVosContext );
   if (!VOS_IS_STATUS_SUCCESS(vosStatus)) {
      VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_FATAL,
            "%s: Failed to close VOSS Scheduler",__func__);
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( vosStatus ) );
   }

   //Close VOSS
   hddLog(VOS_TRACE_LEVEL_ERROR,"Calling WLAN VOS CLOSE\n\n\n\n");
   vos_close(pVosContext);
   hddLog(VOS_TRACE_LEVEL_ERROR,"Returned WLAN VOS CLOSE\n\n\n\n");

   hddLog(VOS_TRACE_LEVEL_ERROR,"Calling WLAN BAL CLOSE\n\n\n\n");

   //Close Watchdog
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

   //Free up dynamically allocated members inside HDD Adapter
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

   vos_mem_free( pHddCtx );
}
#endif /*  FEATURE_WLAN_INTEGRATED_SOC */

/**---------------------------------------------------------------------------

  \brief hdd_update_config_from_nv() - Function to update the contents of
         the running configuration with parameters taken from NV storage

  \param  - pHddCtx - Pointer to the HDD global context

  \return - VOS_STATUS_SUCCESS if successful

  --------------------------------------------------------------------------*/
static VOS_STATUS hdd_update_config_from_nv(hdd_context_t* pHddCtx)
{
#ifndef FEATURE_WLAN_INTEGRATED_SOC
   eHalStatus halStatus;
#endif
#ifdef FIXME_PRIMA_NV
   v_BOOL_t itemIsValid = VOS_FALSE;
   VOS_STATUS status;
#endif // FIXME_PRIMA_NV

#ifdef FIXME_PRIMA_NV
//**************************TODO**************************************
   /*If the NV is valid then get the macaddress from nv else get it from qcom_cfg.ini*/
   status = vos_nv_getValidity(VNV_FIELD_IMAGE, &itemIsValid);

   if(status != VOS_STATUS_SUCCESS)
   {
       hddLog(VOS_TRACE_LEVEL_FATAL," vos_nv_getValidity() failed\n ");
       return VOS_STATUS_E_FAILURE;
   }

   if (itemIsValid == VOS_TRUE) 
   {
        hddLog(VOS_TRACE_LEVEL_INFO_HIGH," Reading the Macaddress from NV\n ");
        status = vos_nv_readMacAddress(&pAdapter->macAddressCurrent.bytes[0]);

        if(status != VOS_STATUS_SUCCESS)
        {
            hddLog(VOS_TRACE_LEVEL_FATAL," vos_nv_readMacAddress() failed\n ");
            return VOS_STATUS_E_FAILURE;
        }
   }
//**************************TODO****************************************
   else {
   //Update the new mac address based on qcom_cfg.ini
       vos_mem_copy(&pAdapter->macAddressCurrent, 
                &pAdapter->cfg_ini->staMacAddr,
                sizeof(v_MACADDR_t));
   }

   vos_mem_copy(pWlanDev->dev_addr, 
                &pAdapter->macAddressCurrent, 
                sizeof(v_MACADDR_t));
#endif // FIXME_PRIMA_NV

#ifndef FEATURE_WLAN_INTEGRATED_SOC
#if 1 /* need to fix for concurrency */
   // Set the MAC Address
   // Currently this is used by HAL to add self sta. Remove this once self sta is added as part of session open.
   halStatus = ccmCfgSetStr( pHddCtx->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pHddCtx->cfg_ini->intfMacAddr[0],
                             sizeof( pHddCtx->cfg_ini->intfMacAddr[0]),
                             hdd_set_mac_addr_cb, VOS_FALSE );

   if (!HAL_STATUS_SUCCESS( halStatus ))
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed to set MAC Address. "
          "HALStatus is %08d [x%08x]",__func__, halStatus, halStatus );
      return VOS_STATUS_E_FAILURE;
   }
#endif
#endif

   return VOS_STATUS_SUCCESS;
}

/**---------------------------------------------------------------------------

  \brief hdd_post_voss_start_config() - HDD post voss start config helper

  \param  - pAdapter - Pointer to the HDD

  \return - None

  --------------------------------------------------------------------------*/
VOS_STATUS hdd_post_voss_start_config(hdd_context_t* pHddCtx)
{
   eHalStatus halStatus;

#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
   /* In the non-integrated architecture we update the configuration from
      the INI file and from NV after vOSS has been started
   */

   // Apply the cfg.ini to cfg.dat
   if (FALSE == hdd_update_config_dat(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: config update failed",__func__ );
      return VOS_STATUS_E_FAILURE;
   }

   // Apply the NV to cfg.dat
   if (VOS_STATUS_SUCCESS != hdd_update_config_from_nv(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "%s: config update from NV failed", __func__ );
      return VOS_STATUS_E_FAILURE;
   }
#endif // FEATURE_WLAN_NON_INTEGRATED_SOC

   // Send ready indication to the HDD.  This will kick off the MAC
   // into a 'running' state and should kick off an initial scan.
   halStatus = sme_HDDReadyInd( pHddCtx->hHal );
   if ( !HAL_STATUS_SUCCESS( halStatus ) )
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%S: sme_HDDReadyInd() failed with status "
          "code %08d [x%08x]",__func__, halStatus, halStatus );
      return VOS_STATUS_E_FAILURE;
   }

   return VOS_STATUS_SUCCESS;
}

#ifdef ANI_BUS_TYPE_SDIO

#ifndef ANI_MANF_DIAG
// Routine to initialize the PMU
void wlan_hdd_enable_deepsleep(v_VOID_t * pVosContext)
{
/*-------------- Need to fix this correctly while doing Deepsleep testing
    tANI_U32 regValue = 0;

    regValue  = QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_DEEP_SLEEP_EN_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_1P23_LPM_AON_MASK_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_1P23_LPM_SW_MASK_MASK |
                QWLAN_PMU_LDO_CTRL_REG_PMU_ANA_2P3_LPM_MASK_MASK;

    WLANBAL_WriteRegister(pVosContext, QWLAN_PMU_LDO_CTRL_REG_REG, regValue);
---------------------*/

    return;
}
#endif
#endif
/**---------------------------------------------------------------------------

  \brief hdd_wlan_startup() - HDD init function

  This is the driver startup code executed once a WLAN device has been detected

  \param  - dev - Pointer to the underlying device

  \return -  0 for success -1 for failure

  --------------------------------------------------------------------------*/

int hdd_wlan_startup(struct device *dev )
{
   VOS_STATUS status;
   hdd_adapter_t *pAdapter = NULL;
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext= NULL;
#ifdef WLAN_BTAMP_FEATURE
   VOS_STATUS vStatus = VOS_STATUS_SUCCESS;
   WLANBAP_ConfigType btAmpConfig;
   hdd_config_t *pConfig;
#endif
   int ret;
#ifdef CONFIG_CFG80211
   struct wiphy *wiphy;
#endif
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = dev_to_sdio_func(dev);
#endif //ANI_BUS_TYPE_SDIO

   ENTER();
#ifdef CONFIG_CFG80211
   /*
    * cfg80211 initialization and registration....
    */
   wiphy = wlan_hdd_cfg80211_init(dev, sizeof(hdd_context_t)) ;

   if(wiphy == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: cfg80211 init failed", __func__);
      return -1;
   }

   pHddCtx = wiphy_priv(wiphy);

#else      
      
   pHddCtx = vos_mem_malloc ( sizeof( hdd_context_t ) );
   if(pHddCtx == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: cfg80211 init failed", __func__);
      return -1;
   }

#endif   
   //Initialize the adapter context to zeros.
   vos_mem_zero(pHddCtx, sizeof( hdd_context_t ));

#ifdef CONFIG_CFG80211
   pHddCtx->wiphy = wiphy;
#endif
   pHddCtx->isLoadUnloadInProgress = TRUE;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

   /*Get vos context here bcoz vos_open requires it*/
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

   //Save the Global VOSS context in adapter context for future.
   pHddCtx->pvosContext = pVosContext;

   //Save the adapter context in global context for future.
   ((VosContextType*)(pVosContext))->pHDDContext = (v_VOID_t*)pHddCtx;
   
#ifdef ANI_BUS_TYPE_SDIO
   // Set the private data for the device to our adapter.
   libra_sdio_setprivdata (sdio_func_dev, pHddCtx);
   atomic_set(&pHddCtx->sdio_claim_count, 0);
#endif // ANI_BUS_TYPE_SDIO

   pHddCtx->parent_dev = dev;

   init_completion(&pHddCtx->full_pwr_comp_var);
   init_completion(&pHddCtx->standby_comp_var);

   hdd_list_init( &pHddCtx->hddAdapters, MAX_NUMBER_OF_ADAPTERS );

   // Load all config first as TL config is needed during vos_open
   pHddCtx->cfg_ini = (hdd_config_t*) kmalloc(sizeof(hdd_config_t), GFP_KERNEL);
   if(pHddCtx->cfg_ini == NULL)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed kmalloc hdd_config_t",__func__);
      goto err_free_hdd_context;
   }

   vos_mem_zero(pHddCtx->cfg_ini, sizeof( hdd_config_t ));

   // Read and parse the qcom_cfg.ini file
   status = hdd_parse_config_ini( pHddCtx );
   if ( VOS_STATUS_SUCCESS != status )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: error parsing %s",
             __func__, WLAN_INI_FILE);
      goto err_config;
   }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   // Update WDI trace levels based upon the cfg.ini
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL,
                        pHddCtx->cfg_ini->wdiTraceEnableDAL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_CTRL,
                        pHddCtx->cfg_ini->wdiTraceEnableCTL);
   hdd_wdi_trace_enable(eWLAN_MODULE_DAL_DATA,
                        pHddCtx->cfg_ini->wdiTraceEnableDAT);
   hdd_wdi_trace_enable(eWLAN_MODULE_PAL,
                        pHddCtx->cfg_ini->wdiTraceEnablePAL);
#endif /* FEATURE_WLAN_INTEGRATED_SOC */

#ifdef ANI_MANF_DIAG 
   if(VOS_FTM_MODE == hdd_get_conparam())
   {
    if ( VOS_STATUS_SUCCESS != wlan_hdd_ftm_open(pHddCtx) )
    {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: wlan_hdd_ftm_open Failed",__func__);
      goto err_free_hdd_context;
    }
    hddLog(VOS_TRACE_LEVEL_FATAL,"%s: FTM driver loaded success fully",__func__);
    return VOS_STATUS_SUCCESS;
   }
#endif

    //Open watchdog module
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
   {
      status = vos_watchdog_open(pVosContext,
         &((VosContextType*)pVosContext)->vosWatchdog, sizeof(VosWatchdogContext));

      if(!VOS_IS_STATUS_SUCCESS( status ))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_watchdog_open failed",__func__);
         goto err_config;
      }
   }

   pHddCtx->isLogpInProgress = FALSE;
   vos_set_logp_in_progress(VOS_MODULE_ID_VOSS, FALSE);

#ifdef ANI_BUS_TYPE_SDIO
   status = WLANBAL_Open(pHddCtx->pvosContext);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAL",__func__);
      goto err_wdclose;
   }
#endif // ANI_BUS_TYPE_SDIO

   status = vos_chipVoteOnXOBuffer(NULL, NULL, NULL);
   if(!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to configure 19.2 MHz Clock", __func__);
#ifdef ANI_BUS_TYPE_SDIO
      goto err_balclose;
#else
      goto err_wdclose;
#endif
   }


#ifdef ANI_BUS_TYPE_SDIO
   status = WLANSAL_Start(pHddCtx->pvosContext);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Failed to start SAL",__func__);
      goto err_clkvote;
   }

  /* Start BAL */
  status = WLANBAL_Start(pHddCtx->pvosContext);

  if (!VOS_IS_STATUS_SUCCESS(status))
   {
     VOS_TRACE(VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
              "%s: Failed to start BAL",__func__);
     goto err_salstop;
  }
#endif // ANI_BUS_TYPE_SDIO

#ifdef MSM_PLATFORM_7x30
   /* FIXME: Volans 2.0 configuration. Reconfigure 1.3v SW supply to 1.3v. It will be configured to
    * 1.4v in vos_ChipPowerup() routine above
    */
#endif

   status = vos_open( &pVosContext, 0);
   if ( !VOS_IS_STATUS_SUCCESS( status ))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_open failed",__func__);
      goto err_balstop;   
   }

   /* Save the hal context in Adapter */
   pHddCtx->hHal = (tHalHandle)vos_get_context( VOS_MODULE_ID_SME, pVosContext );

   if ( NULL == pHddCtx->hHal )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HAL context is null",__func__);      
      goto err_vosclose;
   }

   // Set the SME configuration parameters...
   status = hdd_set_sme_config( pHddCtx );

   if ( VOS_STATUS_SUCCESS != status )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed hdd_set_sme_config",__func__); 
         goto err_vosclose;
      }

   //Initialize the WMM module
   status = hdd_wmm_init(pHddCtx);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: hdd_wmm_init failed", __FUNCTION__);
      goto err_vosclose;
   }

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   /* Vos preStart is calling */
   status = vos_preStart( pHddCtx->pvosContext );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_preStart failed",__func__);
      goto err_vosclose;
   }
#endif

#ifdef FEATURE_WLAN_INTEGRATED_SOC
   /* In the integrated architecture we update the configuration from
      the INI file and from NV before vOSS has been started so that
      the final contents are available to send down to the cCPU   */

   // Apply the cfg.ini to cfg.dat
   if (FALSE == hdd_update_config_dat(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: config update failed",__func__ );
      goto err_vosclose;
   }

   // Apply the NV to cfg.dat
   if (VOS_STATUS_SUCCESS != hdd_update_config_from_nv(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,
             "%s: config update from NV failed", __func__ );
      goto err_vosclose;
   }

   {
      eHalStatus halStatus;
      // Set the MAC Address
      // Currently this is used by HAL to add self sta. Remove this once self sta is added as part of session open.
      halStatus = cfgSetStr( pHddCtx->hHal, WNI_CFG_STA_ID,
                             (v_U8_t *)&pHddCtx->cfg_ini->intfMacAddr[0],
                             sizeof( pHddCtx->cfg_ini->intfMacAddr[0]) );
   
      if (!HAL_STATUS_SUCCESS( halStatus ))
      {
   	   hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Failed to set MAC Address. "
   			   "HALStatus is %08d [x%08x]",__func__, halStatus, halStatus );
   	   return VOS_STATUS_E_FAILURE;
      }
   }
#endif // FEATURE_WLAN_INTEGRATED_SOC

   /*Start VOSS which starts up the SME/MAC/HAL modules and everything else
     Note: Firmware image will be read and downloaded inside vos_start API */
   status = vos_start( pHddCtx->pvosContext );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: vos_start failed",__func__);
      goto err_vosclose;
   }

   status = hdd_post_voss_start_config( pHddCtx );
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_post_voss_start_config failed", 
         __func__);
      goto err_vosstop;
   }

#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE == hdd_get_conparam())
   {
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_SOFTAP, "softap.%d", 
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
   }
   else
   {
#endif
     pAdapter = hdd_open_adapter( pHddCtx, WLAN_HDD_INFRA_STATION, "wlan%d",
         wlan_hdd_get_intf_addr(pHddCtx), FALSE );
#ifdef WLAN_SOFTAP_FEATURE
   }
#endif

   if( pAdapter == NULL )
   {
     hddLog(VOS_TRACE_LEVEL_ERROR,"%s: hdd_open_adapter failed",__func__);
#ifdef ANI_BUS_TYPE_SDIO
     goto err_balstop;
#else
     goto err_clkvote;
#endif
   }
   
     
#ifdef WLAN_BTAMP_FEATURE
   vStatus = WLANBAP_Open(pVosContext);
   if(!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to open BAP",__func__);
      goto err_close_adapter;
   }

   vStatus = BSL_Init(pVosContext);
   if(!VOS_IS_STATUS_SUCCESS(vStatus))
   {
     VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
        "%s: Failed to Init BSL",__func__);
     goto err_bap_close;
   }
   vStatus = WLANBAP_Start(pVosContext);
   if (!VOS_IS_STATUS_SUCCESS(vStatus))
   {
       VOS_TRACE( VOS_MODULE_ID_VOSS, VOS_TRACE_LEVEL_ERROR,
               "%s: Failed to start TL",__func__);
       goto err_bap_close;
   }

   pConfig = pHddCtx->cfg_ini;
   btAmpConfig.ucPreferredChannel = pConfig->preferredChannel;
   status = WLANBAP_SetConfig(&btAmpConfig);

#endif //WLAN_BTAMP_FEATURE
 
#ifndef FEATURE_WLAN_NON_INTEGRATED_SOC
   /* Register with platform driver as client for Suspend/Resume */
   status = hddRegisterPmOps(pAdapter);
   if ( !VOS_IS_STATUS_SUCCESS( status ) )
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hddRegisterPmOps failed",__func__);
      goto err_vosclose;
   }
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
   // Register suspend/resume callbacks
   if(pHddCtx->cfg_ini->nEnableSuspend)
   {
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
      register_wlan_suspend();
#endif
   }
#endif

   // register net device notifier for device change notification
   ret = register_netdevice_notifier(&hdd_netdev_notifier);

   if(ret < 0)
   {
      hddLog(VOS_TRACE_LEVEL_ERROR,"%s: register_netdevice_notifier failed",__func__);
#ifdef WLAN_BTAMP_FEATURE
      goto err_bap_stop;
#else
      goto err_close_adapter;
#endif
   }

   //Initialize the nlink service
   if(nl_srv_init() != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%S: nl_srv_init failed",__func__);
      goto err_reg_netdev;
   }

   //Initialize the BTC service
   if(btc_activate_service(pHddCtx) != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: btc_activate_service failed",__func__);
      goto err_nl_srv;
   }

#ifdef PTT_SOCK_SVC_ENABLE
   //Initialize the PTT service
   if(ptt_sock_activate_svc(pHddCtx) != 0)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: ptt_sock_activate_svc failed",__func__);
      goto err_nl_srv;
   }
#endif

   //Initialize the WoWL service
   if(!hdd_init_wowl(pHddCtx))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: hdd_init_wowl failed",__func__);
      goto err_nl_srv;
   }

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
   hdd_register_mcast_bcast_filter(pHddCtx);
#endif
#endif
#ifdef CONFIG_CFG80211
#ifdef WLAN_SOFTAP_FEATURE
   if (VOS_STA_SAP_MODE != hdd_get_conparam())
#endif
   {
      wlan_hdd_cfg80211_post_voss_start(pAdapter);
   }
#endif

   //Scanning only for station mode.
   //TODO if initial scan is required on AP mode, need to remove this condition.
   //Since AP session is not yet created and hence there is no self sta, active scan will fail.
   if( pAdapter->device_mode == WLAN_HDD_INFRA_STATION )
   {
     //Trigger the initial scan
     hdd_wlan_initial_scan(pAdapter);
   }

   pHddCtx->isLoadUnloadInProgress = FALSE;

   vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, FALSE);
  
   goto success;

err_nl_srv:
   nl_srv_exit();

err_reg_netdev:
   unregister_netdevice_notifier(&hdd_netdev_notifier);


#ifdef WLAN_BTAMP_FEATURE
err_bap_stop:
  WLANBAP_Stop(pVosContext);
err_bap_close:
   WLANBAP_Close(pVosContext);
#endif

err_close_adapter:
   hdd_close_all_adapters( pHddCtx );

err_vosstop:
   vos_stop(pVosContext);

err_vosclose:    
   vos_close(pVosContext ); 

err_balstop:
#ifdef ANI_BUS_TYPE_SDIO
#ifndef ANI_MANF_DIAG
   wlan_hdd_enable_deepsleep(pHddCtx->pvosContext);
#endif

   WLANBAL_Stop(pHddCtx->pvosContext);
   WLANBAL_SuspendChip(pHddCtx->pvosContext);
#endif

#ifdef ANI_BUS_TYPE_SDIO
err_salstop:
   WLANSAL_Stop(pHddCtx->pvosContext);

#endif
err_clkvote:
    vos_chipVoteOffXOBuffer(NULL, NULL, NULL);

#ifdef ANI_BUS_TYPE_SDIO
err_balclose:
   WLANBAL_Close(pHddCtx->pvosContext);
#endif // ANI_BUS_TYPE_SDIO

err_wdclose:
   if(pHddCtx->cfg_ini->fIsLogpEnabled)
      vos_watchdog_close(pVosContext);

err_config:
   kfree(pHddCtx->cfg_ini);
   pHddCtx->cfg_ini= NULL;

err_free_hdd_context:
#ifdef CONFIG_CFG80211
   wiphy_unregister(wiphy) ; 
   wiphy_free(wiphy) ;
   //kfree(wdev) ;
#else
   vos_mem_free( pHddCtx );
#endif

   return -1;

success:
   EXIT();
   return 0;
}

/**---------------------------------------------------------------------------

  \brief hdd_module_init() - Init Function

   This is the driver entry point (invoked when module is loaded using insmod)

  \param  - None

  \return - 0 for success, non zero for failure

  --------------------------------------------------------------------------*/

static int __init hdd_module_init ( void)
{
   VOS_STATUS status;
   v_CONTEXT_t pVosContext = NULL;
#ifdef ANI_BUS_TYPE_SDIO
   struct sdio_func *sdio_func_dev = NULL;
   unsigned int attempts = 0;
#endif // ANI_BUS_TYPE_SDIO
   struct device *dev = NULL;
   int ret_status = 0;
   ENTER();

   hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Wi-Fi loading driver",__func__);

   //Power Up Libra WLAN card first if not already powered up
   status = vos_chipPowerUp(NULL,NULL,NULL);
   if (!VOS_IS_STATUS_SUCCESS(status))
   {
      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not Powered Up."
          "exiting", __func__);
      return -1;
   }

#ifdef ANI_BUS_TYPE_SDIO
   //SDIO Polling should be turned on for card detection. When using Android Wi-Fi GUI
   //users need not trigger SDIO polling explicitly. However when loading drivers via
   //command line (Adb shell), users must turn on SDIO polling prior to loading WLAN.
   do {
      sdio_func_dev = libra_getsdio_funcdev();
      if (NULL == sdio_func_dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN not detected yet.",__func__);
         attempts++;
      }
      else {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Libra WLAN detecton succeeded",__func__);
         dev = &sdio_func_dev->dev;
         break;
      }

      if(attempts == 7)
         break;
      
      msleep(250);

   }while (attempts < 7);

   //Retry to detect the card again by Powering Down the chip and Power up the chip 
   //again. This retry is done to recover from CRC Error
   if (NULL == sdio_func_dev) {

      attempts = 0;
      
      //Vote off any PMIC voltage supplies
      vos_chipPowerDown(NULL, NULL, NULL);

      msleep(1000);

      //Power Up Libra WLAN card first if not already powered up
      status = vos_chipPowerUp(NULL,NULL,NULL);
      if (!VOS_IS_STATUS_SUCCESS(status))
      {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not Powered Up."
             "exiting", __func__);
         return -1;
      }

      do {
         sdio_func_dev = libra_getsdio_funcdev();
         if (NULL == sdio_func_dev) {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN not detected yet.",__func__);
            attempts++;
         }
         else {
            hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Retry Libra WLAN detecton succeeded",__func__);
            dev = &sdio_func_dev->dev;
            break;
         }

         if(attempts == 2)
           break;
      
         msleep(1000);

      }while (attempts < 3);
   }

#endif // ANI_BUS_TYPE_SDIO

#ifdef ANI_BUS_TYPE_PCI

   dev = wcnss_wlan_get_device();

#endif // ANI_BUS_TYPE_PCI

#ifdef ANI_BUS_TYPE_PLATFORM
   dev = wcnss_wlan_get_device();
#endif // ANI_BUS_TYPE_PLATFORM


   do {
      if (NULL == dev) {
         hddLog(VOS_TRACE_LEVEL_FATAL, "%s: WLAN device not found!!",__func__);
         ret_status = -1;
         break;
   }

#ifdef MEMORY_DEBUG
      vos_mem_init();
#endif

#ifdef TIMER_MANAGER
      vos_timer_manager_init();
#endif

      /* Preopen VOSS so that it is ready to start at least SAL */
      status = vos_preOpen(&pVosContext);

   if (!VOS_IS_STATUS_SUCCESS(status))
   {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed to preOpen VOSS", __func__);
         ret_status = -1;
         break;
   }

#ifdef ANI_BUS_TYPE_SDIO
   /* Now Open SAL */
   status = WLANSAL_Open(pVosContext, 0);

   if(!VOS_IS_STATUS_SUCCESS(status))
   {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Failed to open SAL", __func__);

      /* If unable to open, cleanup and return failure */
      vos_preClose( &pVosContext );
         ret_status = -1;
         break;
   }
#endif // ANI_BUS_TYPE_SDIO

#if defined(FEATURE_WLAN_INTEGRATED_SOC) && defined(ANI_MANF_DIAG)
      if(5 == con_mode)
      {
         hdd_set_conparam(VOS_FTM_MODE);
      }
#endif /* FEATURE_WLAN_INTEGRATED_SOC && ANI_MANF_DIAG */

      // Call our main init function
      if(hdd_wlan_startup(dev)) {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: WLAN Driver Initialization failed",
          __func__);
#ifdef ANI_BUS_TYPE_SDIO
         WLANSAL_Close(pVosContext);
#endif // ANI_BUS_TYPE_SDIO
         vos_preClose( &pVosContext );
         ret_status = -1;
         break;
      }
      
      /* Cancel the vote for XO Core ON
       * This is done here for safety purposes in case we re-initialize without turning 
       * it OFF in any error scenario.
       */
      hddLog(VOS_TRACE_LEVEL_ERROR, "In module init: Ensure Force XO Core is OFF"
                                       "when  WLAN is turned ON so Core toggles"
                                       "unless we enter PS\n");
      if (vos_chipVoteXOCore(NULL, NULL, NULL, VOS_FALSE) != VOS_STATUS_SUCCESS)
      {
          hddLog(VOS_TRACE_LEVEL_ERROR, "Could not cancel XO Core ON vote. Not returning failure."
                                            "Power consumed will be high\n");
      }  
   } while (0);

   if(!VOS_IS_STATUS_SUCCESS(ret_status))
   {
      //Assert Deep sleep signal now to put Libra HW in lowest power state
      status = vos_chipAssertDeepSleep( NULL, NULL, NULL );
      VOS_ASSERT( VOS_IS_STATUS_SUCCESS( status) );

      //Vote off any PMIC voltage supplies
      vos_chipPowerDown(NULL, NULL, NULL);
#ifdef TIMER_MANAGER
     vos_timer_exit();
#endif
#ifdef MEMORY_DEBUG
      vos_mem_exit();
#endif

      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Wi-Fi driver load failure", __func__);
   }
   else
   {
      //Send WLAN UP indication to Nlink Service
      send_btc_nlink_msg(WLAN_MODULE_UP_IND, 0);

      hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Wi-Fi driver loaded", __func__);
   }
   
   

   EXIT();

   return ret_status;

}


/**---------------------------------------------------------------------------

  \brief hdd_module_exit() - Exit function

  This is the driver exit point (invoked when module is unloaded using rmmod)

  \param  - None

  \return - None

  --------------------------------------------------------------------------*/
static void __exit hdd_module_exit(void)
{
   hdd_context_t *pHddCtx = NULL;
   v_CONTEXT_t pVosContext = NULL;
   int attempts = 0;

   hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Entering module exit",__func__);

   //Get the global vos context
   pVosContext = vos_get_global_context(VOS_MODULE_ID_SYS, NULL);

   if(!pVosContext)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Global VOS context is Null", __func__);
      return;
   }

   //Get the HDD context.
   pHddCtx = (hdd_context_t *)vos_get_context(VOS_MODULE_ID_HDD, pVosContext );

   if(!pHddCtx)
   {
      hddLog(VOS_TRACE_LEVEL_FATAL,"%s: module exit called before probe",__func__);
   }
   else
   {
      while(isWDresetInProgress()){
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL, "%s:Reset in Progress by LOGP. Block rmmod for 500ms!!!",__func__);
         VOS_ASSERT(0);
         msleep(500);
         attempts++;
         if(attempts==MAX_EXIT_ATTEMPTS_DURING_LOGP)
           break;
      }

      pHddCtx->isLoadUnloadInProgress = TRUE;
      vos_set_load_unload_in_progress(VOS_MODULE_ID_VOSS, TRUE);

      //Do all the cleanup before deregistering the driver
      hdd_wlan_exit(pHddCtx);
   }

#ifdef ANI_BUS_TYPE_SDIO
   WLANSAL_Close(pVosContext);
#endif // ANI_BUS_TYPE_SDIO

   vos_preClose( &pVosContext );

#ifdef TIMER_MANAGER
   vos_timer_exit();
#endif
#ifdef MEMORY_DEBUG
   vos_mem_exit(); 
#endif

#if 0
done:   
#endif
   hddLog(VOS_TRACE_LEVEL_FATAL,"%s: Exiting module exit",__func__);
}

#if defined(WLAN_SOFTAP_FEATURE) || defined(ANI_MANF_DIAG)
/**---------------------------------------------------------------------------

  \brief hdd_get_conparam() -

  This is the driver exit point (invoked when module is unloaded using rmmod)

  \param  - None

  \return - tVOS_CON_MODE

  --------------------------------------------------------------------------*/
tVOS_CON_MODE hdd_get_conparam ( void )
{
    return (tVOS_CON_MODE)con_mode;

}
void hdd_set_conparam ( v_UINT_t newParam )
{
  con_mode = newParam;
}
/**---------------------------------------------------------------------------

  \brief hdd_softap_sta_deauth() - function

  This to take counter measure to handle deauth req from HDD

  \param  - pAdapter - Pointer to the HDD

  \param  - enable - boolean value

  \return - None

  --------------------------------------------------------------------------*/

void hdd_softap_sta_deauth(hdd_adapter_t *pAdapter, v_U8_t *pDestMacAddress)
{
    v_U8_t STAId;
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    tHalHandle hHalHandle;
#endif

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_deauth:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //Ignore request to deauth bcmc station
    if( pDestMacAddress[0] & 0x1 )
       return;

    WLANSAP_DeauthSta(pVosContext,pDestMacAddress);

    /*Get the Station ID*/
#ifdef FEATURE_WLAN_NON_INTEGRATED_SOC
    hHalHandle = (tHalHandle ) vos_get_context(VOS_MODULE_ID_HAL, pVosContext);
    if (eHAL_STATUS_SUCCESS ==
        halTable_FindStaidByAddr(hHalHandle, (tANI_U8 *)pDestMacAddress,
                                 &STAId))
    {
       hdd_softap_DeregisterSTA(pAdapter, STAId);
    }
#else
    if (VOS_STATUS_SUCCESS ==
        hdd_softap_GetStaId(pAdapter, (v_MACADDR_t *)pDestMacAddress,
                            &STAId))
    {
       hdd_softap_DeregisterSTA(pAdapter, STAId);
    }
#endif
}

/**---------------------------------------------------------------------------

  \brief hdd_softap_sta_disassoc() - function

  This to take counter measure to handle deauth req from HDD

  \param  - pAdapter - Pointer to the HDD

  \param  - enable - boolean value

  \return - None

  --------------------------------------------------------------------------*/

void hdd_softap_sta_disassoc(hdd_adapter_t *pAdapter,v_U8_t *pDestMacAddress)
{
        v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_sta_disassoc:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    //Ignore request to disassoc bcmc station
    if( pDestMacAddress[0] & 0x1 )
       return;

    WLANSAP_DisassocSta(pVosContext,pDestMacAddress);
}

void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t *pAdapter,v_BOOL_t enable)
{
    v_CONTEXT_t pVosContext = (WLAN_HDD_GET_CTX(pAdapter))->pvosContext;

    ENTER();

    hddLog( LOGE, "hdd_softap_tkip_mic_fail_counter_measure:(0x%x, false)", (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);

    WLANSAP_SetCounterMeasure(pVosContext, (v_BOOL_t)enable);
}

#endif /* WLAN_SOFTAP_FEATURE */
/**---------------------------------------------------------------------------
 *
 *   \brief hdd_get__concurrency_mode() -
 *
 *
 *       \param  - None
 *
 *         \return - CONCURRENCY MODE
 *
 *           --------------------------------------------------------------------------*/
tVOS_CONCURRENCY_MODE hdd_get_concurrency_mode ( void )
{
      return (tVOS_CONCURRENCY_MODE)concurrency_mode;

}

/* Decide whether to allow/not the apps power collapse. 
 * Allow apps power collapse if we are in connected state.
 * if not, allow only if we are in IMPS  */
v_BOOL_t hdd_is_apps_power_collapse_allowed(hdd_context_t* pHddCtx)
{
    tPmcState pmcState = pmcGetPmcState(pHddCtx->hHal);
    hdd_config_t *pConfig = pHddCtx->cfg_ini;
    hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL; 
    hdd_adapter_t *pAdapter = NULL; 
    VOS_STATUS status;

#ifdef WLAN_SOFTAP_FEATURE
    if (VOS_STA_SAP_MODE == hdd_get_conparam())
        return TRUE;
#endif

    /*loop through all adapters. TBD fix for Concurrency */
    status = hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
    while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
    {
        pAdapter = pAdapterNode->pAdapter;
        if ( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode)
          || (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) )	
        {
            if(!hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR(pAdapter)) && 
                (pConfig->fIsImpsEnabled) &&
                ((pmcState != IMPS) && 
                !(pmcState == STOPPED || pmcState == STANDBY))
              )
            {
                return FALSE;
            }
        }
        status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
        pAdapterNode = pNext;
    }
    return TRUE;
}

//Register the module init/exit functions
module_init(hdd_module_init);
module_exit(hdd_module_exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("QUALCOMM");
MODULE_DESCRIPTION("WLAN HOST DEVICE DRIVER");

#if defined(WLAN_SOFTAP_FEATURE) || defined(ANI_MANF_DIAG)
module_param(con_mode, int, 0);
#endif
