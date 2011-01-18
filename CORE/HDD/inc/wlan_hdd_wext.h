#ifndef __WEXT_IW_H__
#define __WEXT_IW_H__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <linux/timer.h>

/*
 * order of parameters in addTs private ioctl
 */
#define HDD_WLAN_WMM_PARAM_HANDLE                       0
#define HDD_WLAN_WMM_PARAM_TID                          1
#define HDD_WLAN_WMM_PARAM_DIRECTION                    2
#define HDD_WLAN_WMM_PARAM_USER_PRIORITY                3
#define HDD_WLAN_WMM_PARAM_NOMINAL_MSDU_SIZE            4
#define HDD_WLAN_WMM_PARAM_MAXIMUM_MSDU_SIZE            5
#define HDD_WLAN_WMM_PARAM_MINIMUM_DATA_RATE            6
#define HDD_WLAN_WMM_PARAM_MEAN_DATA_RATE               7
#define HDD_WLAN_WMM_PARAM_PEAK_DATA_RATE               8
#define HDD_WLAN_WMM_PARAM_MAX_BURST_SIZE               9
#define HDD_WLAN_WMM_PARAM_MINIMUM_PHY_RATE            10
#define HDD_WLAN_WMM_PARAM_SURPLUS_BANDWIDTH_ALLOWANCE 11
#define HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL            12
#define HDD_WLAN_WMM_PARAM_SUSPENSION_INTERVAL         13
#define HDD_WLAN_WMM_PARAM_COUNT                       14

typedef enum
{
   HDD_WLAN_WMM_DIRECTION_UPSTREAM      = 0,
   HDD_WLAN_WMM_DIRECTION_DOWNSTREAM    = 1,
   HDD_WLAN_WMM_DIRECTION_BIDIRECTIONAL = 2,
} hdd_wlan_wmm_direction_e;

typedef enum
{
   HDD_WLAN_WMM_POWER_SAVE_LEGACY       = 0,
   HDD_WLAN_WMM_POWER_SAVE_UAPSD        = 1,
} hdd_wlan_wmm_power_save_e;

typedef enum
{
   // TSPEC/re-assoc done, async
   HDD_WLAN_WMM_STATUS_SETUP_SUCCESS = 0,
   // no need to setup TSPEC since ACM=0 and no UAPSD desired, sync + async
   HDD_WLAN_WMM_STATUS_SETUP_SUCCESS_NO_ACM_NO_UAPSD = 1,
   // no need to setup TSPEC since ACM=0 and UAPSD already exists, sync + async
   HDD_WLAN_WMM_STATUS_SETUP_SUCCESS_NO_ACM_UAPSD_EXISTING = 2,
   // TSPEC result pending, sync
   HDD_WLAN_WMM_STATUS_SETUP_PENDING = 3,
   // TSPEC/re-assoc failed, sync + async
   HDD_WLAN_WMM_STATUS_SETUP_FAILED = 4,
   // Request rejected due to invalid params, sync + async
   HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM = 5,
   // TSPEC request rejected since AP!=QAP, sync
   HDD_WLAN_WMM_STATUS_SETUP_FAILED_NO_WMM = 6,

   // TSPEC modification/re-assoc successful, async
   HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS = 7,
   // TSPEC modification a no-op since ACM=0 and no change in UAPSD, sync + async
   HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS_NO_ACM_NO_UAPSD = 8,
   // TSPEC modification a no-op since ACM=0 and requested U-APSD already exists, sync + async
   HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS_NO_ACM_UAPSD_EXISTING = 9,
   // TSPEC result pending, sync
   HDD_WLAN_WMM_STATUS_MODIFY_PENDING = 10,
   // TSPEC modification failed, prev TSPEC in effect, sync + async
   HDD_WLAN_WMM_STATUS_MODIFY_FAILED = 11,
   // TSPEC modification request rejected due to invalid params, sync + async
   HDD_WLAN_WMM_STATUS_MODIFY_FAILED_BAD_PARAM = 12,

   // TSPEC release successful, sync and also async
   HDD_WLAN_WMM_STATUS_RELEASE_SUCCESS = 13,
   // TSPEC release pending, sync
   HDD_WLAN_WMM_STATUS_RELEASE_PENDING = 14,
   // TSPEC release failed, sync + async
   HDD_WLAN_WMM_STATUS_RELEASE_FAILED = 15,
   // TSPEC release rejected due to invalid params, sync
   HDD_WLAN_WMM_STATUS_RELEASE_FAILED_BAD_PARAM = 16,
   // TSPEC modified due to the mux'ing of requests on ACs, async

   HDD_WLAN_WMM_STATUS_MODIFIED = 17,
   // TSPEC revoked by AP, async
   HDD_WLAN_WMM_STATUS_LOST = 18,
   // some internal failure like memory allocation failure, etc, sync
   HDD_WLAN_WMM_STATUS_INTERNAL_FAILURE = 19, 

   // U-APSD failed during setup but OTA setup (whether TSPEC exchnage or
   // re-assoc) was done so app should release this QoS, async
   HDD_WLAN_WMM_STATUS_SETUP_UAPSD_SET_FAILED = 20,
   // U-APSD failed during modify, but OTA setup (whether TSPEC exchnage or
   // re-assoc) was done so app should release this QoS, async
   HDD_WLAN_WMM_STATUS_MODIFY_UAPSD_SET_FAILED = 21

} hdd_wlan_wmm_status_e;

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

typedef enum
{
    DRIVER_POWER_MODE_AUTO = 0,
    DRIVER_POWER_MODE_ACTIVE = 1,
} hdd_power_mode_e;
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

   /**Scan mode*/
   tSirScanType scan_mode;
   
   /**Completion Variable*/
   struct completion completion_var;
   
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

extern int iw_set_cscan(struct net_device *dev, struct iw_request_info *info,
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

void ccmCfgSetCallback(tHalHandle halHandle, tANI_S32 result);
#endif // __WEXT_IW_H__

