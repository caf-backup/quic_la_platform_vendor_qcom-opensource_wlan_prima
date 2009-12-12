#ifndef __WEXT_IW_H__
#define __WEXT_IW_H__

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <linux/timer.h>


/** Maximum Length of WPA/RSN IE */
#define MAX_WPA_RSN_IE_LEN 40

/** Maximum Number of WEP KEYS */
#define MAX_WEP_KEYS 4

/** Ether Address Length */
#define ETHER_ADDR_LEN 6

/** Enable 11d */
#define ENABLE_11D  1

/** Disable 11d */
#define DISABLE_11D 0

/* 
   refer wpa.h in wpa supplicant code for REASON_MICHAEL_MIC_FAILURE

   supplicant sets REASON_MICHAEL_MIC_FAILURE as the reason code when it sends the MLME deauth IOCTL 
   for TKIP counter measures
*/
#define HDD_REASON_MICHAEL_MIC_FAILURE 14

/* 
  * These are for TLV fields in WPS IE
  */
#define HDD_WPS_UUID_LEN                    16 
#define HDD_WPS_ELEM_VERSION                0x104a 
#define HDD_WPS_ELEM_REQUEST_TYPE           0x103a 
#define HDD_WPS_ELEM_CONFIG_METHODS         0x1008 
#define HDD_WPS_ELEM_UUID_E                 0x1047 
#define HDD_WPS_ELEM_PRIMARY_DEVICE_TYPE    0x1054 
#define HDD_WPS_ELEM_RF_BANDS               0x103c 
#define HDD_WPS_ELEM_ASSOCIATION_STATE      0x1002 
#define HDD_WPS_ELEM_CONFIGURATION_ERROR    0x1009
#define HDD_WPS_ELEM_DEVICE_PASSWORD_ID     0x1012 


typedef enum
{
    eWEXT_WPS_OFF = 0,
    eWEXT_WPS_ON = 1,
}hdd_wps_mode_e;


/* 
 * This structure contains the interface level (granularity) 
 * configuration information in support of wireless extensions. 
 */
typedef struct hdd_wext_state_s 
{
   /** The CSR "desired" Profile;	*/
   tCsrRoamProfile roamProfile; 
  
   /** The association status code */ 
   v_U32_t statusCode; 
   
   /** The scan id  */
   v_U32_t scanId; 

   /** The scan pending  */
   v_U32_t mScanPending; 
      	
   /** wpa version WPA/WPA2/None*/
   v_S31_t wpaVersion; 
   
   /**WPA or RSN IE*/
   u_int8_t WPARSNIE[MAX_WPA_RSN_IE_LEN]; 

   /**auth key mgmt */
   v_S31_t authKeyMgmt; 

    /**vos event */
   vos_event_t  vosevent;

    /* WPS turned on/off*/
   hdd_wps_mode_e wpsMode; 
	
   /**Counter measure state, Started/Stopped*/
   v_BOOL_t mTKIPCounterMeasures;  
   
}hdd_wext_state_t;

typedef struct ccp_freq_chan_map_s{
    // List of frequencies
    v_U32_t freq;
    v_U32_t chan;
}hdd_freq_chan_map_t;


extern int hdd_UnregisterWext(struct net_device *dev);
extern int hdd_register_wext(struct net_device *dev);

extern int iw_get_scan(struct net_device *dev, 
						 struct iw_request_info *info,
						 union iwreq_data *wrqu, char *extra);

extern int iw_set_scan(struct net_device *dev, struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra);

extern int iw_set_essid(struct net_device *dev, 
                        struct iw_request_info *info,
                        union iwreq_data *wrqu, char *extra);

extern int iw_get_essid(struct net_device *dev, 
                       struct iw_request_info *info,
                       struct iw_point *dwrq, char *extra);


extern int iw_set_ap_address(struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu, char *extra);

extern int iw_get_ap_address(struct net_device *dev,
                             struct iw_request_info *info,
                             union iwreq_data *wrqu, char *extra);

extern int iw_set_auth(struct net_device *dev,struct iw_request_info *info,
                        union iwreq_data *wrqu,char *extra);

extern int iw_get_auth(struct net_device *dev,struct iw_request_info *info,
                         union iwreq_data *wrqu,char *extra);


#endif // __WEXT_IW_H__

